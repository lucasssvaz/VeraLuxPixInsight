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
// COMPILE-TIME OPTIONS:
//
// HMS_USE_MAD: Use MAD (Median Absolute Deviation) approximations instead of
//              exact percentiles for bounds calculation in Linear Expansion
//              and Adaptive Output Scaling. Provides 10-100x performance gain
//              with < 0.001 typical error.
//
// Default behavior (HMS_USE_MAD not defined):
//   - Linear Expansion: exact 0.001 and 99.999 percentiles
//   - Adaptive Scaling: exact 99th percentile
//   - Exact match to Python implementation
//   - Slightly slower but mathematically identical
//
// With HMS_USE_MAD defined:
//   - Linear Expansion: MAD approximation (median ± 3.5σ / ± 4σ)
//   - Adaptive Scaling: stddev approximation (median ± 3σ)
//   - 10-100x faster
//   - < 0.001 typical error, < 0.005 worst case
//
// ----------------------------------------------------------------------------

#ifndef __VeraLuxEngine_h
#define __VeraLuxEngine_h

#include "SensorProfiles.h"

#include <pcl/Image.h>
#include <pcl/ImageVariant.h>

namespace pcl
{

// ----------------------------------------------------------------------------

/*!
 * \struct LinearExpansionStats
 * \brief Diagnostic statistics from linear expansion operation.
 *
 * Contains information about the clamping applied during linear expansion,
 * useful for warning the user about potential data loss.
 */
struct LinearExpansionStats
{
   double pctLow  = 0.0;  //!< Percentage of pixels clamped to zero
   double pctHigh = 0.0;  //!< Percentage of pixels clamped to one
   double low     = 0.0;  //!< Lower bound value
   double high    = 0.0;  //!< Upper bound value
};

// ----------------------------------------------------------------------------

/*!
 * \class VeraLuxEngine
 * \brief Core photometric hyperbolic stretch engine.
 *
 * Implements the mathematical foundation of the VeraLux HyperMetric Stretch
 * algorithm. All methods are static and thread-safe. This is a direct port
 * of the Python VeraLuxCore class to C++/PCL.
 *
 * Key Features:
 * - Inverse hyperbolic sine (arcsinh) based stretching
 * - Sensor-specific photometric luminance extraction
 * - Adaptive black point detection (morphological vs percentile)
 * - Hot pixel rejection with "Smart Max" logic
 * - Dual processing modes (Ready-to-Use vs Scientific)
 */
class VeraLuxEngine
{
public:

   /*!
    * \brief Normalizes input image data to [0,1] range.
    *
    * Handles various input formats: 8/16/32-bit integer and float images.
    * Sanitizes NaN/Inf values. Result is always a float Image in [0,1].
    *
    * \param[out] target    Normalized float image (output)
    * \param[in]  source    Source image variant (any bit depth)
    */
   static void NormalizeInput( Image& target, const ImageVariant& source );

   /*!
    * \brief Calculates black point using statistical percentile method.
    *
    * Uses simple percentile-based estimation (0.5th percentile minus offset).
    * Fast but less accurate on images with gradients or vignetting.
    *
    * \param img    Input normalized image
    * \return       Anchor value (black point)
    */
   static double CalculateAnchor( const Image& img );

   /*!
    * \brief Calculates black point using adaptive morphological method.
    *
    * Analyzes histogram shape to find the true signal start. Uses sensor-
    * weighted luminance for RGB images. More accurate than percentile method.
    *
    * \param img       Input normalized image
    * \param profile   Sensor profile for luminance weighting
    * \return          Anchor value (black point)
    */
   static double CalculateAnchorAdaptive( const Image& img, 
                                           const SensorProfile& profile );

   /*!
    * \brief Extracts sensor-weighted luminance from RGB image.
    *
    * Computes photometrically accurate luminance using quantum efficiency
    * weights from the sensor profile. Handles both RGB and mono images.
    * Applies anchor subtraction.
    *
    * \param[out] luma      Output luminance image
    * \param[in]  rgb       Input RGB image (or mono)
    * \param      anchor    Black point to subtract
    * \param      profile   Sensor profile for weights
    */
   static void ExtractLuminance( Image& luma, const Image& rgb, 
                                  double anchor, const SensorProfile& profile );

   /*!
    * \brief Applies inverse hyperbolic sine stretch.
    *
    * Core stretch function: arcsinh(D*(x-SP)+b) normalized to [0,1].
    * D controls intensity, b controls highlight protection (knee point).
    *
    * \param[in,out] target    Image to stretch (modified in-place)
    * \param         D         Stretch factor (10^logD)
    * \param         b         Highlight protection parameter
    * \param         SP        Shadow protection (default 0.0)
    */
   static void HyperbolicStretch( Image& target, double D, double b, 
                                   double SP = 0.0 );

   /*!
    * \brief Binary search solver for optimal Log D parameter.
    *
    * Finds the Log D value that places the luminance median at the target
    * background level. Uses iterative bisection for precision.
    *
    * \param luma           Input luminance image
    * \param targetMedian   Desired median value
    * \param bVal           Highlight protection parameter
    * \return               Optimal Log D value
    */
   static double SolveLogD( const Image& luma, double targetMedian, 
                             double bVal );

   /*!
    * \brief Applies Midtone Transfer Function (MTF).
    *
    * Adjusts background brightness using PixInsight's standard MTF formula.
    * Used in Ready-to-Use mode to reach target background.
    *
    * \param[in,out] target    Image to transform
    * \param         m         MTF parameter
    */
   static void ApplyMTF( Image& target, double m );

   /*!
    * \brief Applies smart linear expansion with hot pixel rejection.
    *
    * Normalizes data to fill [0,1] range. Uses "Smart Max" logic: preserves
    * star cores (absolute max) but rejects isolated hot pixels (percentile
    * fallback). Low bound is always percentile-based.
    *
    * \param[in,out] target        Image to expand
    * \param         factor        Expansion amount [0,1]
    * \param[out]    diagnostics   Optional clipping statistics
    */
   static void ApplyLinearExpansion( Image& target, float factor,
                                      LinearExpansionStats* diagnostics = nullptr );

   /*!
    * \brief Estimates global star pressure metric.
    *
    * Statistical measure of stellar dominance in the image. Returns a
    * normalized value in [0,1] where 0 = no stars, 1 = extreme stellar
    * concentration. Used by auto-solver for adaptation.
    *
    * \param luma    Input luminance image
    * \return        Star pressure [0,1]
    */
   static double EstimateStarPressure( const Image& luma );

   /*!
    * \brief Applies Ready-to-Use mode adaptive output scaling.
    *
    * Performs intelligent range expansion with black/white point anchoring.
    * Uses "Smart Max" logic to preserve bright stars while rejecting hot
    * pixels. Then applies MTF to reach target background.
    *
    * \param[in,out] target      Image to scale
    * \param         profile     Sensor profile for luminance calculation
    * \param         targetBg    Target background level
    */
   static void AdaptiveOutputScaling( Image& target, 
                                       const SensorProfile& profile,
                                       double targetBg );

   /*!
    * \brief Applies soft-clipping to highlights (Ready-to-Use mode).
    *
    * Smooth roll-off above threshold to prevent hard clipping of star cores.
    * Uses power function for natural transition.
    *
    * \param[in,out] target       Image to clip
    * \param         threshold    Clipping threshold (e.g., 0.98)
    * \param         rolloff      Roll-off power (e.g., 2.0)
    */
   static void ApplyReadyToUseSoftClip( Image& target, double threshold, 
                                         double rolloff );

   /*!
    * \brief Reconstructs RGB from stretched luminance using vector preservation.
    *
    * Maintains original color ratios (vector color) while applying the
    * luminance stretch. Implements color convergence (white point physics)
    * and optional hybrid blending (color grip, shadow convergence).
    *
    * \param[in,out] rgb                 RGB image to reconstruct
    * \param         luma                Stretched luminance
    * \param         originalRGB         Original anchored RGB
    * \param         colorConvergence    Star white point power
    * \param         colorGrip           Vector preservation [0,1]
    * \param         shadowConvergence   Shadow noise damping power
    * \param         D                   Stretch factor (for scalar blend)
    * \param         b                   Highlight protection (for scalar blend)
    */
   static void ReconstructColor( Image& rgb, const Image& luma,
                                  const Image& originalRGB,
                                  double colorConvergence,
                                  double colorGrip,
                                  double shadowConvergence,
                                  double D, double b );
};

// ----------------------------------------------------------------------------

} // pcl

#endif   // __VeraLuxEngine_h

// ----------------------------------------------------------------------------
