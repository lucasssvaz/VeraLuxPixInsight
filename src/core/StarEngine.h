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
// COMPILE-TIME OPTIONS:
//
// SCS_USE_FAST_BLUR: Use separable Gaussian blur instead of full convolution
//                     for signal conditioning and optical healing. Provides
//                     2-5x speedup with < 0.0001 typical error.
//
// SCS_USE_APPROX_YCRCB: Use fast integer YCrCb approximation instead of
//                        float conversion. Provides 1.5x speedup with < 0.001
//                        typical error in optical healing.
//
// Default behavior (no flags defined):
//   - Full convolution Gaussian blur (exact match to Python/OpenCV)
//   - Float YCrCb conversion (exact color space math)
//   - Exact match to Python implementation
//   - Slightly slower but mathematically identical
//
// With optimization flags defined:
//   - Separable Gaussian blur (faster)
//   - Integer YCrCb approximation (faster)
//   - < 0.001 typical error, < 0.005 worst case
//   - 2-3x overall speedup
//
// ----------------------------------------------------------------------------

#ifndef __StarEngine_h
#define __StarEngine_h

#include "SensorProfiles.h"

#include <pcl/Image.h>
#include <pcl/ImageVariant.h>

namespace pcl
{

// ----------------------------------------------------------------------------

/*!
 * \class StarEngine
 * \brief Star-specific photometric processing engine for StarComposer.
 *
 * Implements the Hybrid Scalar/Vector star reconstruction pipeline including
 * signal conditioning, dual-mode stretching, and star surgery operations.
 * All methods are static and thread-safe. This is a direct port of the
 * Python StarComposer v2.0.2 algorithm to C++/PCL.
 *
 * Key Features:
 * - Signal conditioning (Gamma 2.4 + micro-blur)
 * - Hybrid Scalar/Vector stretch engine
 * - Color Grip and Shadow Convergence blending
 * - Star Surgery operations (LSR, Optical Healing, Morphological Reduction)
 * - Dual composition modes (Screen, Linear Add)
 */
class StarEngine
{
public:

   /*!
    * \brief Normalizes star input image data to [0,1] range.
    *
    * Port of Python VeraLuxCore.normalize_input:340-354.
    * Handles various input formats: 8/16/32-bit integer and float images.
    * Sanitizes NaN/Inf values. Converts grayscale to RGB if needed.
    * Result is always a float Image in [0,1].
    *
    * Validation: Must match Python normalization exactly (< 1e-7 error).
    *
    * \param[out] target    Normalized float image (output)
    * \param[in]  source    Source image variant (any bit depth)
    */
   static void NormalizeStarInput( Image& target, const ImageVariant& source );

   /*!
    * \brief Applies signal conditioning for star processing.
    *
    * Port of Python process_star_pipeline:447-453.
    * Applies Gamma 2.4 power function followed by micro-blur (σ=0.5)
    * for transition smoothing. This is critical pre-processing for the
    * star stretch pipeline.
    *
    * Validation: Power function exact (< 1e-7), blur (< 1e-5).
    *
    * \param[in,out] img          Image to condition (modified in-place)
    * \param         scaleFactor  Ratio of current image size to full size [0,1].
    *                             Scales the micro-blur sigma for preview rendering.
    *                             Default 1.0 = full resolution (no scaling).
    */
   static void ApplySignalConditioning( Image& img, double scaleFactor = 1.0 );

   /*!
    * \brief Applies hybrid scalar/vector star stretch.
    *
    * Port of Python process_star_pipeline:461-498.
    * Implements the Hybrid Engine v2.0 with two modes:
    * - Scalar Stretch: Per-channel IHS for white cores (lines 463-467)
    * - Vector Stretch: Luminance IHS with ratio preservation (lines 470-484)
    * - Blending: Color Grip + Shadow Convergence logic (lines 489-495)
    *
    * Validation: Critical function - must match Python exactly
    * - Scalar mode (grip=0): < 1e-6 error
    * - Vector mode (grip=1): < 1e-5 error
    * - Blended mode: < 1e-5 error
    *
    * \param[out] output          Output stretched image
    * \param[in]  input           Input anchored image
    * \param[in]  anchor          Black point value
    * \param      D               Stretch factor (10^logD)
    * \param      b               Highlight protection parameter
    * \param      profile         Sensor profile for luminance weights
    * \param      colorGrip       Vector preservation [0,1]
    * \param      shadowConv      Shadow noise damping [0,3]
    * \param      useAdaptive     Whether adaptive anchor was used
    */
   static void HybridStretch( Image& output, const Image& input, double anchor,
                               double D, double b, const SensorProfile& profile,
                               double colorGrip, double shadowConv,
                               bool useAdaptive );

   /*!
    * \brief Applies Large Structure Rejection (LSR) using DoG.
    *
    * Port of Python apply_large_structure_rejection:419-437.
    * Removes large non-stellar structures (like galaxy cores) using
    * Difference of Gaussians (DoG) with dynamic kernel sizing.
    *
    * Validation: Kernel size calculation exact, blur < 0.001 error.
    *
    * \param[in,out] img        Image to process (modified in-place)
    * \param         intensity  Rejection intensity [0,1]
    */
   static void ApplyLargeStructureRejection( Image& img, double intensity );

   /*!
    * \brief Applies optical healing to fix chromatic aberration.
    *
    * Port of Python apply_optical_healing:398-409.
    * Converts RGB to YCrCb, applies Gaussian blur to Cr/Cb only
    * (chroma smoothing), then converts back to RGB. Fixes magenta/green
    * halos from chromatic aberration.
    *
    * Validation: YCrCb conversion < 0.002 error, overall < 0.002 error.
    *
    * \param[in,out] img          Image to heal (modified in-place)
    * \param         strength     Healing strength [0,20]
    * \param         scaleFactor  Ratio of current image size to full size [0,1].
    *                             Scales the blur kernel for preview rendering.
    *                             Default 1.0 = full resolution (no scaling).
    */
   static void ApplyOpticalHealing( Image& img, double strength, double scaleFactor = 1.0 );

   /*!
    * \brief Applies morphological star reduction.
    *
    * Port of Python apply_star_reduction:411-417.
    * Applies erosion with elliptical kernel, then blends with original
    * to physically shrink star diameters.
    *
    * Validation: Erosion exact match (< 1e-7), blend exact (< 1e-7).
    *
    * \param[in,out] img          Image to reduce (modified in-place)
    * \param         intensity    Reduction intensity [0,1]
    * \param         scaleFactor  Ratio of current image size to full size [0,1].
    *                             Scales the erosion intensity for preview rendering.
    *                             Default 1.0 = full resolution (no scaling).
    */
   static void ApplyStarReduction( Image& img, double intensity, double scaleFactor = 1.0 );

   /*!
    * \brief Composes stars with starless background.
    *
    * Port of Python composition logic (lines 1268-1270).
    * Supports two blending modes:
    * - Screen: 1 - (1-A)*(1-B) (safe, no clipping)
    * - Linear Add: clip(A+B, 0, 1) (physical, may clip)
    *
    * Handles size mismatches by cropping to minimum dimensions.
    *
    * Validation: Both modes must match exactly (< 1e-7 error).
    *
    * \param[out] output     Composed output image
    * \param[in]  starless   Stretched starless background
    * \param[in]  stars      Processed star image
    * \param      useScreen  True for Screen mode, false for Linear Add
    */
   static void ComposeImages( Image& output, const Image& starless,
                               const Image& stars, bool useScreen );

private:

   /*!
    * \brief Converts RGB to YCrCb color space (OpenCV formula).
    *
    * Helper for optical healing. Exact implementation:
    * Y = 0.299R + 0.587G + 0.114B
    * Cr = R - Y
    * Cb = B - Y
    *
    * \param[in,out] img    Image to convert (modified in-place)
    */
   static void ConvertRGBToYCrCb( Image& img );

   /*!
    * \brief Converts YCrCb back to RGB color space.
    *
    * Helper for optical healing. Inverse of ConvertRGBToYCrCb.
    * R = Y + Cr
    * G = Y - 0.194Cr - 0.509Cb
    * B = Y + Cb
    *
    * \param[in,out] img    Image to convert (modified in-place)
    */
   static void ConvertYCrCbToRGB( Image& img );
};

// ----------------------------------------------------------------------------

} // pcl

#endif   // __StarEngine_h

// ----------------------------------------------------------------------------
