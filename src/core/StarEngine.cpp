// This file is part of the VeraLux PixInsight module.
//
// Copyright (c) 2026 Lucas Saavedra Vaz (C++ Port for PixInsight)
// Copyright (c) 2025 Riccardo Paterniti (Original Python implementation)
//
// This program is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation, version 3 of the License.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along with
// this program. If not, see <https://www.gnu.org/licenses/>.
// ----------------------------------------------------------------------------
//
// Original Algorithm: VeraLux StarComposer v2.0.2
// Author: Riccardo Paterniti
// Contact: info@veralux.space
//
// ----------------------------------------------------------------------------

#include "StarEngine.h"
#include "VeraLuxEngine.h"

#include <pcl/ATrousWaveletTransform.h>
#include <pcl/Convolution.h>
#include <pcl/Math.h>
#include <pcl/MorphologicalOperator.h>
#include <pcl/MorphologicalTransformation.h>
#include <pcl/SeparableConvolution.h>
#include <pcl/StructuringElement.h>

namespace pcl
{

// ----------------------------------------------------------------------------

/*!
 * \brief Creates a 1D Gaussian separable filter for a given sigma.
 *
 * Matches OpenCV's cv2.GaussianBlur behavior: the Gaussian kernel is
 * mathematically separable, so a 2D Gaussian blur is equivalent to
 * two 1D passes. The kernel size is auto-calculated from sigma using
 * the same formula as OpenCV (when ksize=0).
 *
 * \param sigma  Standard deviation of the Gaussian kernel
 * \return       SeparableFilter suitable for SeparableConvolution
 */
static SeparableFilter MakeGaussianFilter( double sigma )
{
   // Kernel size: match OpenCV auto-calculation (ksize=0 path)
   // OpenCV: ksize = round(sigma * (3*2 + 1)) | 1, minimum 3
   int n = int( Round( sigma * 4.0 ) ) * 2 + 1;
   if ( n < 3 )
      n = 3;
   // Must be odd
   if ( (n & 1) == 0 )
      ++n;

   // Build 1D Gaussian kernel
   FVector k( n );
   int center = n / 2;
   float sum = 0;
   for ( int i = 0; i < n; ++i )
   {
      float x = float( i - center );
      k[i] = Exp( -( x * x ) / float( 2.0 * sigma * sigma ) );
      sum += k[i];
   }
   // Normalize
   for ( int i = 0; i < n; ++i )
      k[i] /= sum;

   return SeparableFilter( k, k );
}

// ----------------------------------------------------------------------------

void StarEngine::NormalizeStarInput( Image& target, const ImageVariant& source )
{
   /*
    * Python reference: VeraLuxCore.normalize_input:340-354
    * 
    * Handles normalization of star input data with NaN/Inf sanitization.
    * Converts grayscale to RGB if needed (stars should always be RGB).
    */
   
   // Handle different input formats
   if ( source.IsFloatSample() )
   {
      if ( source.BitsPerSample() == 32 )
      {
         const Image& img = static_cast<const Image&>( *source );
         target.Assign( img );
      }
      else if ( source.BitsPerSample() == 64 )
      {
         const DImage& img = static_cast<const DImage&>( *source );
         target.Assign( img );
      }
      
      // Sanitize NaN/Inf (Python: np.nan_to_num)
      for ( int c = 0; c < target.NumberOfChannels(); ++c )
      {
         Image::sample_iterator i( target, c );
         for ( ; i; ++i )
         {
            if ( !IsFinite( *i ) )
               *i = 0;
            else if ( *i < 0 )
               *i = 0;
         }
      }
      
      // Check if data needs scaling (Python lines 348-353)
      double maxVal = target.MaximumSampleValue();
      if ( maxVal > 1.0 + 1e-5 )
      {
         // Standard normalization for data > 1.0
         if ( maxVal <= 65535.0 )
            target /= 65535.0;
         else
            target /= maxVal;
      }
   }
   else if ( source.IsComplexSample() )
   {
      throw Error( "StarEngine: Complex images are not supported." );
   }
   else // Integer samples
   {
      if ( source.BitsPerSample() == 8 )
      {
         const UInt8Image& img = static_cast<const UInt8Image&>( *source );
         target.Assign( img );
         target /= 255.0;
      }
      else if ( source.BitsPerSample() == 16 )
      {
         const UInt16Image& img = static_cast<const UInt16Image&>( *source );
         target.Assign( img );
         target /= 65535.0;
      }
      else if ( source.BitsPerSample() == 32 )
      {
         const UInt32Image& img = static_cast<const UInt32Image&>( *source );
         target.Assign( img );
         target /= 4294967295.0;
      }
   }
   
   // Final clipping to [0,1] (Python: np.clip)
   target.Truncate( 0.0, 1.0 );
   
   // Convert grayscale to RGB if needed (Python lines 442-443)
   if ( target.NumberOfChannels() == 1 )
   {
      Image rgb( target.Width(), target.Height(), ColorSpace::RGB );
      const float* mono = target[0];
      
      for ( int c = 0; c < 3; ++c )
      {
         float* ch = rgb[c];
         for ( size_t i = 0, N = target.NumberOfPixels(); i < N; ++i )
            ch[i] = mono[i];
      }
      
      target.Assign( rgb );
   }
}

// ----------------------------------------------------------------------------

void StarEngine::ApplySignalConditioning( Image& img, double scaleFactor )
{
   /*
    * Python reference: process_star_pipeline:447-453
    * 
    * A. VERALUX SIGNAL CONDITIONING (Gamma 2.4)
    * img = np.power(img, 2.4)
    * 
    * B. TRANSITION SMOOTHING (Micro-Blur)
    * img_hwc = cv2.GaussianBlur(img_hwc, (0, 0), 0.5)
    */
   
   if ( img.NumberOfChannels() != 3 )
      throw Error( "StarEngine::ApplySignalConditioning: RGB image required" );
   
   // --- A. VERALUX SIGNAL CONDITIONING (Gamma 2.4) ---
   // Python line 448: img = np.power(img, 2.4)
   for ( int c = 0; c < 3; ++c )
   {
      float* ch = img[c];
      for ( size_t i = 0, N = img.NumberOfPixels(); i < N; ++i )
         ch[i] = Pow( ch[i], 2.4F );
   }
   
   // --- B. TRANSITION SMOOTHING (Micro-Blur) ---
   // Python lines 451-453: cv2.GaussianBlur(img_hwc, (0, 0), 0.5)
   // OpenCV internally uses separable Gaussian (the kernel is mathematically separable).
   // PCL's SeparableConvolution matches this exactly.
   // Sigma is scaled proportionally for preview rendering to avoid over-blurring
   // on downsampled images.
   {
      double sigma = 0.5 * scaleFactor;
      if ( sigma >= 0.1 )
      {
         SeparableFilter H = MakeGaussianFilter( sigma );
         SeparableConvolution C( H );
         C >> img;
      }
   }
}

// ----------------------------------------------------------------------------

void StarEngine::HybridStretch( Image& output, const Image& input, double anchor,
                                 double D, double b, const SensorProfile& profile,
                                 double colorGrip, double shadowConv,
                                 bool useAdaptive )
{
   /*
    * Python reference: process_star_pipeline:461-498
    * 
    * HYBRID ENGINE v2.0
    * A. Scalar Stretch (lines 463-467)
    * B. Vector Stretch (lines 470-484)
    * C. Blending (lines 489-495)
    */
   
   if ( input.NumberOfChannels() != 3 )
      throw Error( "StarEngine::HybridStretch: RGB image required" );
   
   const int w = input.Width();
   const int h = input.Height();
   const size_t N = input.NumberOfPixels();
   
   // Create anchored image (input already conditioned, just subtract anchor)
   Image img_anchored( w, h, input.ColorSpace() );
   for ( int c = 0; c < 3; ++c )
   {
      const float* src = input[c];
      float* dst = img_anchored[c];
      for ( size_t i = 0; i < N; ++i )
         dst[i] = Max( 0.0F, src[i] - float( anchor ) );
   }
   
   // --- A. SCALAR STRETCH (Python lines 463-467) ---
   // scalar[0] = VeraLuxCore.hyperbolic_stretch(img_anchored[0], D_val, b)
   // scalar[1] = VeraLuxCore.hyperbolic_stretch(img_anchored[1], D_val, b)
   // scalar[2] = VeraLuxCore.hyperbolic_stretch(img_anchored[2], D_val, b)
   
   Image scalar( w, h, input.ColorSpace() );
   for ( int c = 0; c < 3; ++c )
   {
      Image channel( w, h, ColorSpace::Gray );
      const float* src = img_anchored[c];
      float* ch = channel[0];
      for ( size_t i = 0; i < N; ++i )
         ch[i] = src[i];
      
      // Apply IHS using VeraLuxEngine (already validated)
      VeraLuxEngine::HyperbolicStretch( channel, D, b, 0.0 );
      
      // Copy to scalar
      const float* stretched = channel[0];
      float* dst = scalar[c];
      for ( size_t i = 0; i < N; ++i )
         dst[i] = stretched[i];
   }
   
   // Clip scalar to [0,1]
   scalar.Truncate( 0.0, 1.0 );
   
   // --- B. VECTOR STRETCH (Python lines 470-484) ---
   Image vector;
   
   if ( colorGrip > 0.001 )
   {
      // Extract luminance from anchored image
      Image L_anchored( w, h, ColorSpace::Gray );
      float* luma = L_anchored[0];
      
      const float* r = img_anchored[0];
      const float* g = img_anchored[1];
      const float* b_ch = img_anchored[2];
      
      double rw = profile.rWeight;
      double gw = profile.gWeight;
      double bw = profile.bWeight;
      
      for ( size_t i = 0; i < N; ++i )
         luma[i] = rw * r[i] + gw * g[i] + bw * b_ch[i];
      
      // Stretch luminance (Python line 473)
      VeraLuxEngine::HyperbolicStretch( L_anchored, D, b, 0.0 );
      L_anchored.Truncate( 0.0, 1.0 );
      
      // Calculate color ratios (Python lines 475-479)
      const float epsilon = 1e-9F;
      const float* L_str = L_anchored[0];
      
      vector.AllocateData( w, h, input.NumberOfChannels(), input.ColorSpace() );
      
      for ( size_t i = 0; i < N; ++i )
      {
         float L_safe = luma[i] + epsilon;
         float r_ratio = r[i] / L_safe;
         float g_ratio = g[i] / L_safe;
         float b_ratio = b_ch[i] / L_safe;
         
         // Reconstruct color (Python lines 481-483)
         vector[0][i] = L_str[i] * r_ratio;
         vector[1][i] = L_str[i] * g_ratio;
         vector[2][i] = L_str[i] * b_ratio;
      }
      
      vector.Truncate( 0.0, 1.0 );
   }
   else
   {
      // If grip is zero, vector = scalar
      vector.Assign( scalar );
   }
   
   // --- C. BLENDING (Python lines 489-495) ---
   output.AllocateData( w, h, input.NumberOfChannels(), input.ColorSpace() );
   
   if ( colorGrip > 0.001 )
   {
      // Calculate grip_map with shadow convergence damping
      // Python lines 490-494:
      // grip_map = np.full_like(scalar[0], grip)
      // if shadow > 0.01:
      //     L_ref = 0.2126*scalar[0] + 0.7152*scalar[1] + 0.0722*scalar[2]
      //     damping = np.power(L_ref, shadow)
      //     grip_map = grip_map * damping
      
      Image grip_map( w, h, ColorSpace::Gray );
      float* gmap = grip_map[0];
      
      for ( size_t i = 0; i < N; ++i )
         gmap[i] = colorGrip;
      
      if ( shadowConv > 0.01 )
      {
         // Compute reference luminance (Rec.709 weights)
         for ( size_t i = 0; i < N; ++i )
         {
            float L_ref = 0.2126F * scalar[0][i] + 
                          0.7152F * scalar[1][i] + 
                          0.0722F * scalar[2][i];
            float damping = Pow( L_ref, float( shadowConv ) );
            gmap[i] *= damping;
         }
      }
      
      // Blend: final = vector * grip_map + scalar * (1 - grip_map)
      // Python line 495
      for ( int c = 0; c < 3; ++c )
      {
         const float* vec = vector[c];
         const float* scal = scalar[c];
         float* out = output[c];
         
         for ( size_t i = 0; i < N; ++i )
         {
            float g = gmap[i];
            out[i] = vec[i] * g + scal[i] * (1.0F - g);
         }
      }
   }
   else
   {
      // Pure scalar mode
      output.Assign( scalar );
   }
   
   // Final clipping (Python line 499)
   output.Truncate( 0.0, 1.0 );
}

// ----------------------------------------------------------------------------

void StarEngine::ApplyLargeStructureRejection( Image& img, double intensity )
{
   /*
    * Python reference: apply_large_structure_rejection:419-437
    * 
    * Core Rejection (LSR): Removes large blobs using Difference of Gaussians.
    * Dynamic Kernel Size: scales with image to target actual structures.
    * 
    * Python lines 426-429:
    *   k_size_val = int(min(h, w) / 15.0)
    *   if k_size_val % 2 == 0: k_size_val += 1
    *   if k_size_val < 3: k_size_val = 3
    */
   
   if ( intensity <= 0.0 || img.NumberOfChannels() != 3 )
      return;
   
   const int w = img.Width();
   const int h = img.Height();
   const size_t N = img.NumberOfPixels();
   
   // Dynamic kernel size (Python lines 426-429)
   int k_size_val = int( Min( h, w ) / 15.0 );
   if ( k_size_val % 2 == 0 )
      k_size_val += 1;
   if ( k_size_val < 3 )
      k_size_val = 3;
   
   // Apply Gaussian blur (Python line 433: cv2.GaussianBlur)
   // OpenCV uses ksize with sigma = 0.3*((ksize-1)*0.5 - 1) + 0.8 when sigma=0
   // For ksize=k_size_val: sigma = 0.3*((k_size_val-1)*0.5 - 1) + 0.8
   const double sigma_lsr = 0.3 * ((k_size_val - 1) * 0.5 - 1) + 0.8;
   
   Image blurred;
   blurred.Assign( img );
   {
      SeparableFilter H = MakeGaussianFilter( sigma_lsr );
      SeparableConvolution C( H );
      C >> blurred;
   }
   
   // Compute high-pass (DoG) and blend (Python lines 434-436)
   // high_pass = np.maximum(img_hwc - low_pass, 0.0)
   // result = img_hwc * (1.0 - intensity) + high_pass * intensity
   
   for ( int c = 0; c < 3; ++c )
   {
      float* ch = img[c];
      const float* blur = blurred[c];
      
      for ( size_t i = 0; i < N; ++i )
      {
         float high_pass = Max( 0.0F, ch[i] - blur[i] );
         ch[i] = ch[i] * (1.0F - float( intensity )) + high_pass * float( intensity );
      }
   }
}

// ----------------------------------------------------------------------------

void StarEngine::ConvertRGBToYCrCb( Image& img )
{
   /*
    * OpenCV YCrCb formula (Python: cv2.COLOR_RGB2YCrCb)
    * Y = 0.299*R + 0.587*G + 0.114*B
    * Cr = (R - Y) * 0.713 + 0.5
    * Cb = (B - Y) * 0.564 + 0.5
    * 
    * Note: OpenCV uses a scaled formula, but for our purposes
    * (blur only Cr/Cb), we use the simpler:
    * Y = 0.299*R + 0.587*G + 0.114*B
    * Cr = R - Y
    * Cb = B - Y
    */
   
#ifdef SCS_USE_APPROX_YCRCB
   // OPTIMIZATION: Integer approximation (faster)
   // Not implemented yet - would use fixed-point math
#endif
   
   // DEFAULT: Float conversion (exact)
   if ( img.NumberOfChannels() != 3 )
      return;
   
   const size_t N = img.NumberOfPixels();
   float* r = img[0];
   float* g = img[1];
   float* b = img[2];
   
   // Create Y, Cr, Cb channels
   Image ycrcb( img.Width(), img.Height(), ColorSpace::RGB );
   float* y = ycrcb[0];
   float* cr = ycrcb[1];
   float* cb = ycrcb[2];
   
   for ( size_t i = 0; i < N; ++i )
   {
      float Y = 0.299F * r[i] + 0.587F * g[i] + 0.114F * b[i];
      y[i] = Y;
      cr[i] = r[i] - Y;
      cb[i] = b[i] - Y;
   }
   
   img.Assign( ycrcb );
}

// ----------------------------------------------------------------------------

void StarEngine::ConvertYCrCbToRGB( Image& img )
{
   /*
    * Inverse YCrCb conversion
    * R = Y + Cr
    * G = Y - 0.194*Cr - 0.509*Cb  (derived from weights)
    * B = Y + Cb
    * 
    * Simplified (matching our forward transform):
    * R = Y + Cr
    * B = Y + Cb
    * G = 1.0/0.587 * (Y - 0.299*R - 0.114*B)
    */
   
   if ( img.NumberOfChannels() != 3 )
      return;
   
   const size_t N = img.NumberOfPixels();
   float* y = img[0];
   float* cr = img[1];
   float* cb = img[2];
   
   // Create RGB channels
   Image rgb( img.Width(), img.Height(), ColorSpace::RGB );
   float* r = rgb[0];
   float* g = rgb[1];
   float* b = rgb[2];
   
   for ( size_t i = 0; i < N; ++i )
   {
      float Y = y[i];
      r[i] = Y + cr[i];
      b[i] = Y + cb[i];
      
      // Solve for G from: Y = 0.299*R + 0.587*G + 0.114*B
      g[i] = (Y - 0.299F * r[i] - 0.114F * b[i]) / 0.587F;
   }
   
   img.Assign( rgb );
}

// ----------------------------------------------------------------------------

void StarEngine::ApplyOpticalHealing( Image& img, double strength, double scaleFactor )
{
   /*
    * Python reference: apply_optical_healing:398-409
    * 
    * Repairs chromatic aberration by blurring chrominance channels only.
    * 
    * Python lines 401-408:
    *   ycrcb = cv2.cvtColor(img_cv, cv2.COLOR_RGB2YCrCb)
    *   y, cr, cb = cv2.split(ycrcb)
    *   ksize = int(strength * 2) + 1
    *   if ksize % 2 == 0: ksize += 1
    *   cr = cv2.GaussianBlur(cr, (ksize, ksize), 0)
    *   cb = cv2.GaussianBlur(cb, (ksize, ksize), 0)
    *   merged = cv2.merge([y, cr, cb])
    *   rgb_heal = cv2.cvtColor(merged, cv2.COLOR_YCrCb2RGB)
    */
   
   if ( strength <= 0.0 || img.NumberOfChannels() != 3 )
      return;
   
   // Calculate kernel size (Python lines 404-405), scaled for preview rendering
   int ksize = Max( 1, int( strength * 2.0 * scaleFactor ) ) | 1;
   if ( ksize < 3 )
      ksize = 3;
   
   // Convert RGB to YCrCb (Python line 401)
   ConvertRGBToYCrCb( img );
   
   // Apply Gaussian blur to Cr and Cb only (Python lines 406-407)
   // Y channel (img[0]) remains untouched.
   // OpenCV sigma from ksize when sigma=0: sigma = 0.3*((ksize-1)*0.5 - 1) + 0.8
   const double sigma_heal = 0.3 * ((ksize - 1) * 0.5 - 1) + 0.8;
   
   // Blur Cr and Cb channels (c = 1, 2)
   for ( int c = 1; c < 3; ++c )
   {
      Image channel( img.Width(), img.Height(), ColorSpace::Gray );
      float* ch = img[c];
      float* tmp = channel[0];
      size_t N = img.NumberOfPixels();
      
      for ( size_t i = 0; i < N; ++i )
         tmp[i] = ch[i];
      
      SeparableFilter H = MakeGaussianFilter( sigma_heal );
      SeparableConvolution C( H );
      C >> channel;
      
      const float* blurred = channel[0];
      for ( size_t i = 0; i < N; ++i )
         ch[i] = blurred[i];
   }
   
   // Convert back to RGB (Python line 409)
   ConvertYCrCbToRGB( img );
}

// ----------------------------------------------------------------------------

void StarEngine::ApplyStarReduction( Image& img, double intensity, double scaleFactor )
{
   /*
    * Python reference: apply_star_reduction:411-417
    * 
    * Morphological erosion with elliptical kernel.
    * 
    * Python lines 413-416:
    *   k_size = 3 if intensity < 0.5 else 5
    *   kernel = cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (k_size, k_size))
    *   eroded = cv2.erode(img_hwc, kernel, iterations=1)
    *   return (img_hwc * (1.0 - intensity) + eroded * intensity)
    */
   
   if ( intensity <= 0.0 || img.NumberOfChannels() != 3 )
      return;
   
   // Scale the blend intensity for preview rendering. A fixed 3x3 or 5x5 erosion
   // kernel affects proportionally more area on downsampled images, so we dampen
   // the blending to compensate.
   double effectiveIntensity = intensity * scaleFactor;
   if ( effectiveIntensity <= 0.0 )
      return;
   
   // Determine kernel size (Python line 413)
   const int k_size = (intensity < 0.5) ? 3 : 5;
   
   // Create elliptical (circular) structuring element
   CircularStructure structure( k_size );
   
   // Create erosion filter
   ErosionFilter erosionOp;
   
   // Apply erosion to each channel
   Image eroded( img.Width(), img.Height(), img.ColorSpace() );
   
   for ( int c = 0; c < 3; ++c )
   {
      Image channel( img.Width(), img.Height(), ColorSpace::Gray );
      float* src = img[c];
      float* ch = channel[0];
      
      for ( size_t i = 0, N = img.NumberOfPixels(); i < N; ++i )
         ch[i] = src[i];
      
      // Apply erosion using MorphologicalTransformation
      MorphologicalTransformation erosion( erosionOp, structure );
      erosion >> channel;
      
      // Copy to eroded
      const float* er = channel[0];
      float* dst = eroded[c];
      for ( size_t i = 0, N = img.NumberOfPixels(); i < N; ++i )
         dst[i] = er[i];
   }
   
   // Blend: result = img * (1 - intensity) + eroded * intensity (Python line 417)
   float blendFactor = float( effectiveIntensity );
   for ( int c = 0; c < 3; ++c )
   {
      float* ch = img[c];
      const float* er = eroded[c];
      
      for ( size_t i = 0, N = img.NumberOfPixels(); i < N; ++i )
         ch[i] = ch[i] * (1.0F - blendFactor) + er[i] * blendFactor;
   }
}

// ----------------------------------------------------------------------------

void StarEngine::ComposeImages( Image& output, const Image& starless,
                                 const Image& stars, bool useScreen )
{
   /*
    * Python reference: run_preview_logic:1268-1270
    * 
    * Screen mode (Python line 1268): final = 1.0 - (1.0 - sl) * (1.0 - st)
    * Add mode (Python line 1270): final = np.clip(sl + st, 0.0, 1.0)
    */
   
   if ( starless.NumberOfChannels() != 3 || stars.NumberOfChannels() != 3 )
      throw Error( "StarEngine::ComposeImages: RGB images required" );
   
   // Handle size mismatches (crop to minimum dimensions)
   const int min_h = Min( starless.Height(), stars.Height() );
   const int min_w = Min( starless.Width(), stars.Width() );
   
   output.AllocateData( min_w, min_h, starless.NumberOfChannels(), starless.ColorSpace() );
   
   if ( useScreen )
   {
      // Screen mode: 1 - (1-A)*(1-B)
      // Order matters for precision (Python line 1268)
      for ( int c = 0; c < 3; ++c )
      {
         const float* sl = starless[c];
         const float* st = stars[c];
         float* out = output[c];
         
         for ( int y = 0; y < min_h; ++y )
         {
            for ( int x = 0; x < min_w; ++x )
            {
               size_t idx_out = y * min_w + x;
               size_t idx_sl = y * starless.Width() + x;
               size_t idx_st = y * stars.Width() + x;
               
               float sl_val = sl[idx_sl];
               float st_val = st[idx_st];
               
               out[idx_out] = 1.0F - (1.0F - sl_val) * (1.0F - st_val);
            }
         }
      }
   }
   else
   {
      // Linear Add mode: clip(A + B, 0, 1)
      for ( int c = 0; c < 3; ++c )
      {
         const float* sl = starless[c];
         const float* st = stars[c];
         float* out = output[c];
         
         for ( int y = 0; y < min_h; ++y )
         {
            for ( int x = 0; x < min_w; ++x )
            {
               size_t idx_out = y * min_w + x;
               size_t idx_sl = y * starless.Width() + x;
               size_t idx_st = y * stars.Width() + x;
               
               float sum = sl[idx_sl] + st[idx_st];
               out[idx_out] = Min( 1.0F, Max( 0.0F, sum ) );
            }
         }
      }
   }
}

// ----------------------------------------------------------------------------

} // pcl

// ----------------------------------------------------------------------------
