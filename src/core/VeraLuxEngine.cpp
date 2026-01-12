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

#include "VeraLuxEngine.h"

#include <pcl/ImageStatistics.h>
#include <pcl/Math.h>

#include <algorithm>
#include <vector>

namespace pcl
{

// ----------------------------------------------------------------------------

namespace
{
   /*
    * Python reference: VeraLuxCore.calculate_anchor / calculate_anchor_adaptive
    * - Percentile is computed on a subsample (stride), with NumPy's default
    *   linear interpolation between adjacent ranks.
    */
   static double PercentileFromSorted( const std::vector<float>& sorted, double pct )
   {
      if ( sorted.empty() )
         return 0.0;

      pct = Max( 0.0, Min( pct, 100.0 ) );
      if ( sorted.size() == 1 )
         return sorted[0];

      const double pos = (pct/100.0) * double( sorted.size() - 1 );
      const size_t i0 = size_t( Floor( pos ) );
      const size_t i1 = Min( i0 + 1, sorted.size() - 1 );
      const double f = pos - double( i0 );

      const double v0 = sorted[i0];
      const double v1 = sorted[i1];
      return v0 + f * (v1 - v0);
   }

   static double PercentileInPlace( std::vector<float>& sample, double pct )
   {
      if ( sample.empty() )
         return 0.0;

      std::sort( sample.begin(), sample.end() );
      return PercentileFromSorted( sample, pct );
   }

   static double SubsamplePercentile( const float* data, size_t count, size_t stride, double pct )
   {
      if ( data == nullptr || count == 0 )
         return 0.0;

      stride = Max( size_t( 1 ), stride );
      std::vector<float> sample;
      sample.reserve( count/stride + 1 );
      for ( size_t i = 0; i < count; i += stride )
         sample.push_back( data[i] );

      return PercentileInPlace( sample, pct );
   }

   /*
    * Approximate NumPy:
    *   np.convolve(hist, np.ones(50)/50, mode='same')
    * using an explicit 50-wide box filter with zero padding.
    */
   static void SmoothHistogramBox50( std::vector<double>& out,
                                     const std::vector<uint64>& hist )
   {
      const int bins = int( hist.size() );
      out.assign( bins, 0.0 );
      if ( bins <= 0 )
         return;

      const int window = 50;
      const int half = window/2; // 25

      for ( int i = 0; i < bins; ++i )
      {
         uint64 sum = 0;
         // window spans [i-half, i-half+window-1] i.e. 50 samples
         for ( int k = 0; k < window; ++k )
         {
            int j = i - half + k;
            if ( j >= 0 && j < bins )
               sum += hist[j];
         }
         out[i] = double( sum ) / double( window );
      }
   }
} // namespace

void VeraLuxEngine::NormalizeInput( Image& target, const ImageVariant& source )
{
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
      
      // Check if data is in [0,1] or needs scaling
      double maxVal = target.MaximumSampleValue();
      if ( maxVal > 1.1 )
      {
         // Assume 16-bit range
         if ( maxVal < 100000.0 )
            target /= 65535.0;
         else
            target /= 4294967295.0;
      }
   }
   else if ( source.IsComplexSample() )
   {
      throw Error( "Complex images are not supported." );
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
   
   // Sanitize NaN/Inf
   for ( int c = 0; c < target.NumberOfChannels(); ++c )
   {
      Image::sample_iterator i( target, c );
      for ( ; i; ++i )
      {
         if ( !IsFinite( *i ) || *i < 0 )
            *i = 0;
      }
   }
   
   target.Truncate( 0.0, 1.0 );
}

// ----------------------------------------------------------------------------

double VeraLuxEngine::CalculateAnchor( const Image& img )
{
   /*
    * Python reference:
    *   stride = max(1, data_norm.size // 500000)   # RGB
    *   floor  = np.percentile(channel.flatten()[::stride], 0.5)
    *   anchor = max(0.0, min(floors) - 0.00025)
    *
    *   stride = max(1, data_norm.size // 200000)   # mono
    *   floor  = np.percentile(data_norm.flatten()[::stride], 0.5)
    *   anchor = max(0.0, floor - 0.00025)
    */
   const int nChannels = img.NumberOfChannels();
   const size_t nPixels = img.NumberOfPixels();
   const size_t totalSize = nPixels * size_t( Max( 1, nChannels ) );

   if ( nChannels == 3 )
   {
      const size_t stride = Max( size_t( 1 ), totalSize / 500000 );
      double minFloor = 1.0;

      for ( int c = 0; c < 3; ++c )
      {
         const float* ch = img[c];
         const double floor = SubsamplePercentile( ch, nPixels, stride, 0.5 );
         minFloor = Min( minFloor, floor );
      }

      return Max( 0.0, minFloor - 0.00025 );
   }

   // Mono (and any non-RGB): treat as single-channel, as in Python's mono branches.
   const size_t stride = Max( size_t( 1 ), totalSize / 200000 );
   const float* ch = img[0];
   const double floor = SubsamplePercentile( ch, nPixels, stride, 0.5 );
   return Max( 0.0, floor - 0.00025 );
}

// ----------------------------------------------------------------------------

double VeraLuxEngine::CalculateAnchorAdaptive( const Image& img, 
                                                 const SensorProfile& profile )
{
   // Build luminance sample for histogram analysis
   Image luma;
   
   if ( img.NumberOfChannels() == 3 )
   {
      // Extract sensor-weighted luminance
      luma.AllocateData( img.Width(), img.Height(), 1 );
      
      const float* r = img[0];
      const float* g = img[1];
      const float* b = img[2];
      float* l = luma[0];
      size_t N = img.NumberOfPixels();
      
      double rw = profile.rWeight;
      double gw = profile.gWeight;
      double bw = profile.bWeight;
      
      for ( size_t i = 0; i < N; ++i )
         l[i] = rw * r[i] + gw * g[i] + bw * b[i];
   }
   else
   {
      // Mono: use directly
      luma.Assign( img );
   }
   
   // Build histogram (65536 bins for precision) on a subsample (Python stride logic).
   const int bins = 65536;
   std::vector<uint64> hist( bins, 0 );

   const float* data = luma[0];
   const size_t N = luma.NumberOfPixels();
   const size_t stride = Max( size_t( 1 ), N / 2000000 );

   // Keep the subsample for percentile fallback (matches Python).
   std::vector<float> sample;
   sample.reserve( N/stride + 1 );

   for ( size_t i = 0; i < N; i += stride )
   {
      float v = data[i];
      // Keep within histogram range [0,1] (Python uses range=(0,1)).
      v = Max( 0.0f, Min( v, 1.0f ) );
      sample.push_back( v );

      // Avoid overflow bin==bins for v==1 by nudging into last bin.
      float vv = Min( v, 0.999999f );
      int bin = int( vv * bins );
      if ( bin < 0 ) bin = 0;
      if ( bin >= bins ) bin = bins - 1;
      hist[bin]++;
   }

   // Smooth histogram exactly like NumPy convolution with zero padding.
   std::vector<double> histSmooth;
   SmoothHistogramBox50( histSmooth, hist );

   // Peak search (Python "search_start" behavior).
   int searchStart = 100;
   if ( searchStart >= bins )
      searchStart = 0;

   double maxBefore = 0;
   for ( int i = 0; i < Min( bins, 100 ); ++i )
      maxBefore = Max( maxBefore, histSmooth[i] );

   if ( maxBefore > 0 )
      searchStart = 0;

   int peakIdx = searchStart;
   double peakVal = histSmooth[peakIdx];
   for ( int i = searchStart + 1; i < bins; ++i )
   {
      if ( histSmooth[i] > peakVal )
      {
         peakVal = histSmooth[i];
         peakIdx = i;
      }
   }

   const double targetVal = peakVal * 0.06;

   // Python: candidates = where(left_side < target_val); anchor_idx = candidates[-1]
   int anchorIdx = -1;
   for ( int i = 0; i < peakIdx; ++i )
      if ( histSmooth[i] < targetVal )
         anchorIdx = i;

   double anchor;
   if ( anchorIdx >= 0 )
   {
      // Python uses bin_edges[anchor_idx]; with uniform bins in [0,1], this is anchorIdx/bins.
      anchor = double( anchorIdx ) / double( bins );
   }
   else
   {
      // Fallback: np.percentile(sample, 0.5)
      anchor = PercentileInPlace( sample, 0.5 );
   }

   return Max( 0.0, anchor );
}

// ----------------------------------------------------------------------------

void VeraLuxEngine::ExtractLuminance( Image& luma, const Image& rgb,
                                       double anchor, const SensorProfile& profile )
{
   if ( rgb.NumberOfChannels() == 3 )
   {
      // RGB: extract weighted luminance
      luma.AllocateData( rgb.Width(), rgb.Height(), 1 );
      
      const float* r = rgb[0];
      const float* g = rgb[1];
      const float* b = rgb[2];
      float* l = luma[0];
      size_t N = rgb.NumberOfPixels();
      
      double rw = profile.rWeight;
      double gw = profile.gWeight;
      double bw = profile.bWeight;
      float anchorF = float( anchor );
      
      for ( size_t i = 0; i < N; ++i )
      {
         float ra = Max( 0.0f, r[i] - anchorF );
         float ga = Max( 0.0f, g[i] - anchorF );
         float ba = Max( 0.0f, b[i] - anchorF );
         l[i] = rw * ra + gw * ga + bw * ba;
      }
   }
   else
   {
      // Mono: just subtract anchor
      luma.Assign( rgb );
      luma.Truncate( float( anchor ), 1.0f );
      luma -= anchor;
   }
}

// ----------------------------------------------------------------------------

void VeraLuxEngine::HyperbolicStretch( Image& target, double D, double b, double SP )
{
   // arcsinh(D*(x-SP)+b) normalized
   D = Max( D, 0.1 );
   b = Max( b, 0.1 );
   
   double term2 = ArcSinh( b );
   double normFactor = ArcSinh( D * (1.0 - SP) + b ) - term2;
   if ( normFactor == 0 )
      normFactor = 1e-6;
   
   // Apply to all channels
   for ( int c = 0; c < target.NumberOfChannels(); ++c )
   {
      Image::sample_iterator i( target, c );
      for ( ; i; ++i )
      {
         double val = *i;
         double term1 = ArcSinh( D * (val - SP) + b );
         *i = float( (term1 - term2) / normFactor );
      }
   }
   
   target.Truncate( 0.0, 1.0 );
}

// ----------------------------------------------------------------------------

double VeraLuxEngine::SolveLogD( const Image& luma, double targetMedian, double bVal )
{
   // Binary search for Log D
   ImageStatistics stats;
   stats.DisableVariance();
   stats.DisableExtremes();
   stats.DisableMean();
   stats << luma;
   
   double medianIn = stats.Median();
   if ( medianIn < 1e-9 )
      return 2.0;
   
   double lowLog = 0.0;
   double highLog = 7.0;
   double bestLogD = 2.0;
   
   for ( int iter = 0; iter < 40; ++iter )
   {
      double midLog = (lowLog + highLog) / 2.0;
      double midD = Pow10( midLog );
      
      // Simulate stretch on median value
      double term1 = ArcSinh( midD * medianIn + bVal );
      double term2 = ArcSinh( bVal );
      double normFactor = ArcSinh( midD + bVal ) - term2;
      if ( normFactor == 0 )
         normFactor = 1e-6;
      
      double testVal = (term1 - term2) / normFactor;
      
      if ( Abs( testVal - targetMedian ) < 0.0001 )
      {
         bestLogD = midLog;
         break;
      }
      
      if ( testVal < targetMedian )
         lowLog = midLog;
      else
         highLog = midLog;
   }
   
   return bestLogD;
}

// ----------------------------------------------------------------------------

void VeraLuxEngine::ApplyMTF( Image& target, double m )
{
   // Midtone transfer function
   double m1 = m - 1.0;
   double m2 = 2.0 * m - 1.0;
   
   for ( int c = 0; c < target.NumberOfChannels(); ++c )
   {
      Image::sample_iterator i( target, c );
      for ( ; i; ++i )
      {
         double v = *i;
         double term1 = m1 * v;
         double term2 = m2 * v - m;
         
         if ( term2 != 0 )
            *i = float( term1 / term2 );
         else
            *i = 0;
      }
   }
   
   target.Truncate( 0.0, 1.0 );
}

// ----------------------------------------------------------------------------

void VeraLuxEngine::ApplyLinearExpansion( Image& target, float factor,
                                           LinearExpansionStats* diagnostics )
{
   if ( factor <= 0.001f )
   {
      if ( diagnostics )
      {
         diagnostics->pctLow = 0.0;
         diagnostics->pctHigh = 0.0;
         diagnostics->low = 0.0;
         diagnostics->high = 0.0;
      }
      return;
   }
   
   factor = Max( 0.0f, Min( factor, 1.0f ) );
   
   // Analyze maximum (Smart Max logic for hot pixel rejection)
   double absMax = target.MaximumSampleValue();
   bool useAbsoluteMax = false;
   
   if ( absMax > 0.001 )
   {
      // Find max pixel location
      Point maxPos;
      target.LocateMaximumSampleValue( maxPos );
      
      // Check 3x3 neighborhood
      int y0 = Max( 0, maxPos.y - 1 );
      int y1 = Min( int( target.Height() ), maxPos.y + 2 );
      int x0 = Max( 0, maxPos.x - 1 );
      int x1 = Min( int( target.Width() ), maxPos.x + 2 );
      
      double maxNeighbor = 0;
      for ( int y = y0; y < y1; ++y )
      {
         for ( int x = x0; x < x1; ++x )
         {
            double val = target( x, y );
            if ( val < absMax )
               maxNeighbor = Max( maxNeighbor, val );
         }
      }
      
      // If bright neighbors exist, it's a real star
      if ( maxNeighbor >= absMax * 0.20 )
         useAbsoluteMax = true;
   }
   
   // Calculate bounds
   double low, high;
   
#ifdef HMS_USE_MAD
   // Fast MAD approximation (10-100x faster, < 0.001 typical error)
   ImageStatistics stats;
   stats.DisableVariance();
   stats.DisableExtremes();
   stats.DisableMean();
   stats << target;
   
   // Low bound: MAD approximation of 0.001 percentile
   low = Max( 0.0, stats.Median() - 3.5 * stats.MAD() );
   
   // High bound: use absolute max or MAD approximation of 99.999 percentile
   if ( useAbsoluteMax )
      high = absMax;
   else
      high = Min( 1.0, stats.Median() + 4.0 * stats.MAD() );
#else
   // Exact percentiles (matches Python implementation exactly)
   // Build subsample for percentile calculation (matching Python stride logic)
   const size_t stride = Max( size_t( 1 ), target.NumberOfPixels() / 500000 );
   std::vector<float> sample;
   sample.reserve( target.NumberOfPixels() * target.NumberOfChannels() / stride + 1 );
   
   // Collect all channels into sample (matching Python behavior)
   for ( int c = 0; c < target.NumberOfChannels(); ++c )
   {
      const float* ch = target[c];
      size_t N = target.NumberOfPixels();
      for ( size_t i = 0; i < N; i += stride )
         sample.push_back( ch[i] );
   }
   
   // Low bound: exact 0.001 percentile
   low = PercentileInPlace( sample, 0.001 );
   
   // High bound: use absolute max or exact 99.999 percentile
   if ( useAbsoluteMax )
      high = absMax;
   else
   {
      // Reuse sample for high percentile
      std::sort( sample.begin(), sample.end() );
      high = PercentileFromSorted( sample, 99.999 );
   }
#endif
   
   if ( high <= low )
   {
      if ( diagnostics )
      {
         diagnostics->pctLow = 0.0;
         diagnostics->pctHigh = 0.0;
         diagnostics->low = low;
         diagnostics->high = high;
      }
      return;
   }
   
   // Calculate diagnostics if requested
   if ( diagnostics )
   {
      size_t totalPixels = target.NumberOfPixels() * target.NumberOfChannels();
      size_t countLow = 0, countHigh = 0;
      
      for ( int c = 0; c < target.NumberOfChannels(); ++c )
      {
         Image::const_sample_iterator i( target, c );
         for ( ; i; ++i )
         {
            if ( *i <= low ) countLow++;
            if ( *i >= high ) countHigh++;
         }
      }
      
      diagnostics->pctLow = double( countLow ) * 100.0 / totalPixels;
      diagnostics->pctHigh = double( countHigh ) * 100.0 / totalPixels;
      diagnostics->low = low;
      diagnostics->high = high;
   }
   
   // Apply expansion
   double range = high - low;
   float factorInv = 1.0f - factor;
   
   for ( int c = 0; c < target.NumberOfChannels(); ++c )
   {
      Image::sample_iterator i( target, c );
      for ( ; i; ++i )
      {
         float original = *i;
         double temp = (double( original ) - low) / range;
         float normalized = float( Max( 0.0, Min( temp, 1.0 ) ) );
         *i = original * factorInv + normalized * factor;
      }
   }
}

// ----------------------------------------------------------------------------

double VeraLuxEngine::EstimateStarPressure( const Image& luma )
{
   if ( luma.IsEmpty() )
      return 0.0;
   
   // Subsample for performance
   size_t N = luma.NumberOfPixels();
   size_t stride = Max( size_t( 1 ), N / 300000 );
   
   std::vector<float> sample;
   sample.reserve( N / stride );
   
   const float* data = luma[0];
   for ( size_t i = 0; i < N; i += stride )
   {
      if ( data[i] > 1e-7f )
         sample.push_back( data[i] );
   }
   
   if ( sample.size() < 100 )
      return 0.0;
   
   // Sort for percentile calculation
   std::sort( sample.begin(), sample.end() );
   
   size_t idx999 = size_t( sample.size() * 0.999 );
   size_t idx9999 = size_t( sample.size() * 0.9999 );
   
   double p999 = sample[idx999];
   double p9999 = sample[idx9999];
   
   // Fraction in extreme tail
   size_t countBright = 0;
   for ( float v : sample )
      if ( v > p999 )
         countBright++;
   
   double brightFrac = double( countBright ) / sample.size();
   
   // Normalize
   double pTerm = Max( 0.0, Min( (p9999 / (p999 + 1e-9) - 1.0) / 4.0, 1.0 ) );
   double fTerm = Max( 0.0, Min( brightFrac * 200.0, 1.0 ) );
   
   double starPressure = 0.7 * pTerm + 0.3 * fTerm;
   return Max( 0.0, Min( starPressure, 1.0 ) );
}

// ----------------------------------------------------------------------------

void VeraLuxEngine::AdaptiveOutputScaling( Image& target,
                                             const SensorProfile& profile,
                                             double targetBg )
{
   // Extract luminance for analysis
   Image luma;
   if ( target.NumberOfChannels() == 3 )
   {
      luma.AllocateData( target.Width(), target.Height(), 1 );
      
      const float* r = target[0];
      const float* g = target[1];
      const float* b = target[2];
      float* l = luma[0];
      size_t N = target.NumberOfPixels();
      
      for ( size_t i = 0; i < N; ++i )
         l[i] = profile.rWeight * r[i] + profile.gWeight * g[i] + profile.bWeight * b[i];
   }
   else
   {
      luma.Assign( target );
   }
   
   // Calculate statistics
   ImageStatistics stats;
   stats << luma;
   
   double medianL = stats.Median();
   double stdL = stats.StdDev();
   double minL = stats.Minimum();
   
   // Global floor (2.7 sigma clip)
   double globalFloor = Max( minL, medianL - 2.7 * stdL );
   const double PEDESTAL = 0.001;
   
   // Analyze max (Smart Max logic)
   double absMax = stats.Maximum();
   bool validPhysicalMax = true;
   
   if ( absMax > 0.001 )
   {
      Point maxPos;
      luma.LocateMaximumSampleValue( maxPos );
      
      int y0 = Max( 0, maxPos.y - 1 );
      int y1 = Min( int( luma.Height() ), maxPos.y + 2 );
      int x0 = Max( 0, maxPos.x - 1 );
      int x1 = Min( int( luma.Width() ), maxPos.x + 2 );
      
      double maxNeighbor = 0;
      for ( int y = y0; y < y1; ++y )
      {
         for ( int x = x0; x < x1; ++x )
         {
            double val = luma( x, y );
            if ( val < absMax )
               maxNeighbor = Max( maxNeighbor, val );
         }
      }
      
      if ( maxNeighbor < absMax * 0.20 )
         validPhysicalMax = false;
   }
   
   // Calculate soft ceiling (99th percentile)
   double softCeil;
   
#ifdef HMS_USE_MAD
   // Fast standard deviation approximation (10-100x faster, < 0.005 typical error)
   softCeil = medianL + 3.0 * stdL;
   softCeil = Max( globalFloor + 1e-6, Min( softCeil, 1.0 ) );
#else
   // Exact 99th percentile (matches Python RTU_SOFT_CEIL_PERCENTILE = 99.0)
   if ( target.NumberOfChannels() == 3 )
   {
      // RGB: calculate per-channel and take max (matching Python)
      size_t N = target.NumberOfPixels();
      const size_t stride = Max( size_t( 1 ), N / 500000 );
      
      std::vector<float> sampleR, sampleG, sampleB;
      sampleR.reserve( N / stride + 1 );
      sampleG.reserve( N / stride + 1 );
      sampleB.reserve( N / stride + 1 );
      
      // Subsample each channel
      const float* r = target[0];
      const float* g = target[1];
      const float* b = target[2];
      
      for ( size_t i = 0; i < N; i += stride )
      {
         sampleR.push_back( r[i] );
         sampleG.push_back( g[i] );
         sampleB.push_back( b[i] );
      }
      
      double p99R = PercentileInPlace( sampleR, 99.0 );
      double p99G = PercentileInPlace( sampleG, 99.0 );
      double p99B = PercentileInPlace( sampleB, 99.0 );
      
      softCeil = Max( p99R, Max( p99G, p99B ) );
   }
   else
   {
      // Mono: single channel percentile
      size_t N = luma.NumberOfPixels();
      const size_t stride = Max( size_t( 1 ), N / 200000 );
      
      std::vector<float> sample;
      sample.reserve( N / stride + 1 );
      
      const float* data = luma[0];
      
      for ( size_t i = 0; i < N; i += stride )
         sample.push_back( data[i] );
      
      softCeil = PercentileInPlace( sample, 99.0 );
   }
   
   softCeil = Max( globalFloor + 1e-6, Min( softCeil, 1.0 ) );
#endif
   
   if ( absMax <= softCeil )
      absMax = softCeil + 1e-6;
   
   // Scale factors
   double scaleContrast = (0.98 - PEDESTAL) / (softCeil - globalFloor + 1e-9);
   double finalScale;
   
   if ( validPhysicalMax )
   {
      double scalePhysicalLimit = (1.0 - PEDESTAL) / (absMax - globalFloor + 1e-9);
      finalScale = Min( scaleContrast, scalePhysicalLimit );
   }
   else
   {
      finalScale = scaleContrast;
   }
   
   // Apply scaling
   for ( int c = 0; c < target.NumberOfChannels(); ++c )
   {
      Image::sample_iterator i( target, c );
      for ( ; i; ++i )
      {
         double val = *i;
         double scaled = (val - globalFloor) * finalScale + PEDESTAL;
         *i = float( Max( 0.0, Min( scaled, 1.0 ) ) );
      }
   }
   
   // Recalculate luminance for MTF
   if ( target.NumberOfChannels() == 3 )
   {
      const float* r = target[0];
      const float* g = target[1];
      const float* b = target[2];
      float* l = luma[0];
      size_t N = target.NumberOfPixels();
      
      for ( size_t i = 0; i < N; ++i )
         l[i] = profile.rWeight * r[i] + profile.gWeight * g[i] + profile.bWeight * b[i];
   }
   else
   {
      luma.Assign( target );
   }
   
   stats << luma;
   double currentBg = stats.Median();
   
   // Apply MTF if needed
   if ( currentBg > 0.0 && currentBg < 1.0 && Abs( currentBg - targetBg ) > 1e-3 )
   {
      double m = (currentBg * (targetBg - 1.0)) / (currentBg * (2.0 * targetBg - 1.0) - targetBg);
      ApplyMTF( target, m );
   }
}

// ----------------------------------------------------------------------------

void VeraLuxEngine::ApplyReadyToUseSoftClip( Image& target, double threshold, double rolloff )
{
   float threshF = float( threshold );
   float rangeInv = float( 1.0 / (1.0 - threshold + 1e-9) );
   
   for ( int c = 0; c < target.NumberOfChannels(); ++c )
   {
      Image::sample_iterator i( target, c );
      for ( ; i; ++i )
      {
         if ( *i > threshF )
         {
            float t = Max( 0.0f, Min( (*i - threshF) * rangeInv, 1.0f ) );
            float soft = 1.0f - Pow( 1.0f - t, float( rolloff ) );
            *i = threshF + (1.0f - threshF) * soft;
         }
      }
   }
   
   target.Truncate( 0.0, 1.0 );
}

// ----------------------------------------------------------------------------

void VeraLuxEngine::ReconstructColor( Image& rgb, const Image& luma,
                                       const Image& originalRGB,
                                       double colorConvergence,
                                       double colorGrip,
                                       double shadowConvergence,
                                       double D, double b )
{
   if ( rgb.NumberOfChannels() != 3 )
   {
      // Mono: just copy luminance
      rgb.Assign( luma );
      return;
   }
   
   size_t N = rgb.NumberOfPixels();
   const float epsilon = 1e-9f;
   
   // Extract ratios from original RGB (anchored)
   std::vector<float> rRatio( N ), gRatio( N ), bRatio( N );
   
   const float* origR = originalRGB[0];
   const float* origG = originalRGB[1];
   const float* origB = originalRGB[2];
   
   for ( size_t i = 0; i < N; ++i )
   {
      float L = origR[i] + origG[i] + origB[i] + epsilon;
      rRatio[i] = origR[i] / L;
      gRatio[i] = origG[i] / L;
      bRatio[i] = origB[i] / L;
   }
   
   // Apply stretched luminance with color convergence
   const float* L_str = luma[0];
   float* outR = rgb[0];
   float* outG = rgb[1];
   float* outB = rgb[2];
   
   for ( size_t i = 0; i < N; ++i )
   {
      float L = L_str[i];
      
      // Color convergence (white point)
      float k = Pow( L, float( colorConvergence ) );
      float rFinal = rRatio[i] * (1.0f - k) + 1.0f * k;
      float gFinal = gRatio[i] * (1.0f - k) + 1.0f * k;
      float bFinal = bRatio[i] * (1.0f - k) + 1.0f * k;
      
      outR[i] = L * rFinal;
      outG[i] = L * gFinal;
      outB[i] = L * bFinal;
   }
   
   // Hybrid blending (if needed)
   bool needsHybrid = (colorGrip < 1.0) || (shadowConvergence > 0.01);
   
   if ( needsHybrid )
   {
      // Compute scalar stretch of original RGB
      Image scalar;
      scalar.Assign( originalRGB );
      HyperbolicStretch( scalar, D, b );
      
      // Blend based on grip and shadow convergence
      for ( size_t i = 0; i < N; ++i )
      {
         float L = L_str[i];
         float gripMap = float( colorGrip );
         
         if ( shadowConvergence > 0.01 )
         {
            float damping = Pow( L, float( shadowConvergence ) );
            gripMap *= damping;
         }
         
         float gripInv = 1.0f - gripMap;
         
         outR[i] = outR[i] * gripMap + scalar[0][i] * gripInv;
         outG[i] = outG[i] * gripMap + scalar[1][i] * gripInv;
         outB[i] = outB[i] * gripMap + scalar[2][i] * gripInv;
      }
   }
   
   // Apply pedestal and truncate
   rgb *= 0.995f;
   rgb += 0.005f;
   rgb.Truncate( 0.0, 1.0 );
}

// ----------------------------------------------------------------------------

} // pcl

// ----------------------------------------------------------------------------
// EOF VeraLuxEngine.cpp - Released 2025-01-06T00:00:00Z
