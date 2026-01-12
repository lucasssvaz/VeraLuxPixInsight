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

#include "HyperMetricStretchInstance.h"
#include "HyperMetricStretchParameters.h"

#include <pcl/AutoViewLock.h>
#include <pcl/Console.h>
#include <pcl/StandardStatus.h>
#include <pcl/View.h>

namespace pcl
{

// ----------------------------------------------------------------------------

HyperMetricStretchInstance::HyperMetricStretchInstance( const MetaProcess* m )
   : ProcessImplementation( m )
   , processingMode( HMSProcessingMode::Default )
   , sensorProfile( 0 ) // Use Rec.709 as default
   , targetBackground( 0.20 )
   , logD( 2.0 )
   , protectB( 6.0 )
   , colorConvergence( 3.5 )
   , colorStrategy( 0 )
   , colorGrip( 1.0 )
   , shadowConvergence( 0.0 )
   , linearExpansion( 0.0 )
   , adaptiveAnchor( true )
{
}

// ----------------------------------------------------------------------------

HyperMetricStretchInstance::HyperMetricStretchInstance( const HyperMetricStretchInstance& x )
   : ProcessImplementation( x )
{
   Assign( x );
}

// ----------------------------------------------------------------------------

void HyperMetricStretchInstance::Assign( const ProcessImplementation& p )
{
   const HyperMetricStretchInstance* x = dynamic_cast<const HyperMetricStretchInstance*>( &p );
   if ( x != nullptr )
   {
      processingMode = x->processingMode;
      sensorProfile = x->sensorProfile;
      targetBackground = x->targetBackground;
      logD = x->logD;
      protectB = x->protectB;
      colorConvergence = x->colorConvergence;
      colorStrategy = x->colorStrategy;
      colorGrip = x->colorGrip;
      shadowConvergence = x->shadowConvergence;
      linearExpansion = x->linearExpansion;
      adaptiveAnchor = x->adaptiveAnchor;
   }
}

// ----------------------------------------------------------------------------

UndoFlags HyperMetricStretchInstance::UndoMode( const View& ) const
{
   return UndoFlag::PixelData;
}

// ----------------------------------------------------------------------------

bool HyperMetricStretchInstance::CanExecuteOn( const View& view, String& whyNot ) const
{
   if ( view.Image().IsComplexSample() )
   {
      whyNot = "HyperMetric Stretch cannot be executed on complex images.";
      return false;
   }

   return true;
}

// ----------------------------------------------------------------------------

void HyperMetricStretchInstance::GetEffectiveParams( double& grip, double& shadow, double& linearExp ) const
{
   if ( processingMode == HMSProcessingMode::ReadyToUse )
   {
      // Ready-to-Use mode: derive from colorStrategy
      int val = colorStrategy;
      if ( val < 0 )
      {
         // Left: Increase Shadow Convergence, Grip stays 1.0
         shadow = (Abs( val ) / 100.0) * 3.0;
         grip = 1.0;
      }
      else
      {
         // Right: Decrease Grip, Shadow stays 0.0
         grip = 1.0 - ((val / 100.0) * 0.6); // Max reduction to 0.4
         shadow = 0.0;
      }
      linearExp = 0.0; // Always off in Ready-to-Use
   }
   else
   {
      // Scientific mode: use explicit parameters
      grip = colorGrip;
      shadow = shadowConvergence;
      linearExp = linearExpansion;
   }
}

// ----------------------------------------------------------------------------

bool HyperMetricStretchInstance::ExecuteOn( View& view )
{
   AutoViewLock lock( view );

   ImageVariant image = view.Image();

   if ( image.IsComplexSample() )
      return false;

   StandardStatus status;
   image.SetStatusCallback( &status );

   Console console;
   console.EnableAbort();

   // Get effective parameters
   double grip, shadow, linearExp;
   GetEffectiveParams( grip, shadow, linearExp );

   const SensorProfile& profile = GetSensorProfile();
   double D = Pow10( logD );

   try
   {
      // Step 1: Normalize input
      console.WriteLn( "<end><cbr>VeraLux HyperMetric Stretch" );
      console.WriteLn( String().Format( "Mode: %s | Sensor: %s",
                       (processingMode == HMSProcessingMode::ReadyToUse) ? "Ready-to-Use" : "Scientific",
                       profile.name.c_str() ) );

      Image working;
      VeraLuxEngine::NormalizeInput( working, image );

      // Step 2: Calculate anchor
      double anchor;
      if ( adaptiveAnchor )
      {
         console.WriteLn( "Calculating adaptive anchor (morphological)..." );
         anchor = VeraLuxEngine::CalculateAnchorAdaptive( working, profile );
      }
      else
      {
         console.WriteLn( "Calculating anchor (statistical)..." );
         anchor = VeraLuxEngine::CalculateAnchor( working );
      }
      console.WriteLn( String().Format( "Anchor: %.6f", anchor ) );

      // Step 3: Extract luminance
      console.WriteLn( "Extracting photometric luminance..." );
      Image luma;
      VeraLuxEngine::ExtractLuminance( luma, working, anchor, profile );

      // Step 4: Apply hyperbolic stretch
      console.WriteLn( String().Format( "Applying hyperbolic stretch (Log D=%.2f, b=%.2f)...", logD, protectB ) );
      VeraLuxEngine::HyperbolicStretch( luma, D, protectB );

      // Step 5: Linear expansion (Scientific mode only)
      if ( processingMode == HMSProcessingMode::Scientific && linearExp > 0.001 )
      {
         console.WriteLn( String().Format( "Applying linear expansion (%.2f)...", linearExp ) );
         LinearExpansionStats stats;
         VeraLuxEngine::ApplyLinearExpansion( luma, float( linearExp ), &stats );

         if ( stats.pctHigh >= 0.01 )
            console.WarningLn( String().Format( "  Warning: %.3f%% of pixels clamped at high end", stats.pctHigh ) );
      }

      // Step 6: Reconstruct color with vector preservation
      console.WriteLn( "Reconstructing color (vector preservation)..." );

      // Need anchored RGB for color reconstruction
      Image anchoredRGB;
      if ( working.NumberOfChannels() == 3 )
      {
         anchoredRGB.AllocateData( working.Width(), working.Height(), 3 );
         float anchorF = float( anchor );
         for ( int c = 0; c < 3; ++c )
         {
            const float* src = working[c];
            float* dst = anchoredRGB[c];
            size_t N = working.NumberOfPixels();
            for ( size_t i = 0; i < N; ++i )
               dst[i] = Max( 0.0f, src[i] - anchorF );
         }
      }
      else
      {
         anchoredRGB.Assign( working );
         anchoredRGB.Truncate( float( anchor ), 1.0f );
         anchoredRGB -= anchor;
      }

      VeraLuxEngine::ReconstructColor( working, luma, anchoredRGB,
                                        colorConvergence, grip, shadow, D, protectB );

      // Step 7: Output scaling (Ready-to-Use mode only)
      if ( processingMode == HMSProcessingMode::ReadyToUse )
      {
         console.WriteLn( "Applying adaptive output scaling..." );
         VeraLuxEngine::AdaptiveOutputScaling( working, profile, targetBackground );

         console.WriteLn( "Applying soft-clipping..." );
         VeraLuxEngine::ApplyReadyToUseSoftClip( working, 0.98, 2.0 );
      }

      // Step 8: Write back
      console.WriteLn( "Writing result..." );
      image.CopyImage( working );

      console.WriteLn( "<end><cbr>Done." );
      return true;
   }
   catch ( ProcessAborted& )
   {
      console.NoteLn( "<end><cbr>* Process aborted by user." );
      throw;
   }
   catch ( const std::exception& e )
   {
      console.CriticalLn( "<end><cbr>*** Error: HyperMetric Stretch failed ***" );
      console.CriticalLn( String().Format( "Exception: %s", e.what() ) );
      throw;
   }
   catch ( ... )
   {
      console.CriticalLn( "<end><cbr>*** Error: HyperMetric Stretch failed with unknown exception ***" );
      throw;
   }
}

// ----------------------------------------------------------------------------

bool HyperMetricStretchInstance::Preview( Image& img ) const
{
   // Simplified version for real-time preview (no console output)
   double grip, shadow, linearExp;
   GetEffectiveParams( grip, shadow, linearExp );

   const SensorProfile& profile = GetSensorProfile();
   double D = Pow10( logD );

   try
   {
      // Normalize (match Python and ExecuteOn behavior)
      Image working;
      ImageVariant source( &img );
      VeraLuxEngine::NormalizeInput( working, source );

      // Anchor
      double anchor = adaptiveAnchor ?
         VeraLuxEngine::CalculateAnchorAdaptive( working, profile ) :
         VeraLuxEngine::CalculateAnchor( working );

      // Luminance
      Image luma;
      VeraLuxEngine::ExtractLuminance( luma, working, anchor, profile );

      // Stretch
      VeraLuxEngine::HyperbolicStretch( luma, D, protectB );

      // Linear expansion (Scientific only)
      if ( processingMode == HMSProcessingMode::Scientific && linearExp > 0.001 )
         VeraLuxEngine::ApplyLinearExpansion( luma, float( linearExp ) );

      // Color reconstruction
      Image anchoredRGB;
      if ( working.NumberOfChannels() == 3 )
      {
         anchoredRGB.AllocateData( working.Width(), working.Height(), 3 );
         float anchorF = float( anchor );
         for ( int c = 0; c < 3; ++c )
         {
            const float* src = working[c];
            float* dst = anchoredRGB[c];
            size_t N = working.NumberOfPixels();
            for ( size_t i = 0; i < N; ++i )
               dst[i] = Max( 0.0f, src[i] - anchorF );
         }
      }
      else
      {
         anchoredRGB.Assign( working );
         anchoredRGB.Truncate( float( anchor ), 1.0f );
         anchoredRGB -= anchor;
      }

      VeraLuxEngine::ReconstructColor( working, luma, anchoredRGB,
                                        colorConvergence, grip, shadow, D, protectB );

      // Output scaling (Ready-to-Use only)
      if ( processingMode == HMSProcessingMode::ReadyToUse )
      {
         VeraLuxEngine::AdaptiveOutputScaling( working, profile, targetBackground );
         VeraLuxEngine::ApplyReadyToUseSoftClip( working, 0.98, 2.0 );
      }

      // Copy back
      img.Assign( working );
      return true;
   }
   catch ( const std::exception& e )
   {
#ifdef DEBUG
      Console console;
      console.WarningLn( "<end><cbr>* Preview failed *" );
      console.WarningLn( String().Format( "Exception: %s", e.what() ) );
#endif
      return false;
   }
   catch ( ... )
   {
#ifdef DEBUG
      Console().WarningLn( "<end><cbr>* Preview failed with unknown exception *" );
#endif
      return false;
   }
}

// ----------------------------------------------------------------------------

void* HyperMetricStretchInstance::LockParameter( const MetaParameter* p, size_type /*tableRow*/ )
{
   if ( p == TheHMSProcessingModeParameter )
      return &processingMode;
   if ( p == TheHMSSensorProfileParameter )
      return &sensorProfile;
   if ( p == TheHMSTargetBackgroundParameter )
      return &targetBackground;
   if ( p == TheHMSLogDParameter )
      return &logD;
   if ( p == TheHMSProtectBParameter )
      return &protectB;
   if ( p == TheHMSColorConvergenceParameter )
      return &colorConvergence;
   if ( p == TheHMSColorStrategyParameter )
      return &colorStrategy;
   if ( p == TheHMSColorGripParameter )
      return &colorGrip;
   if ( p == TheHMSShadowConvergenceParameter )
      return &shadowConvergence;
   if ( p == TheHMSLinearExpansionParameter )
      return &linearExpansion;
   if ( p == TheHMSAdaptiveAnchorParameter )
      return &adaptiveAnchor;

   return nullptr;
}

// ----------------------------------------------------------------------------

bool HyperMetricStretchInstance::AllocateParameter( size_type /*sizeOrLength*/, const MetaParameter* /*p*/, size_type /*tableRow*/ )
{
   return true;
}

// ----------------------------------------------------------------------------

size_type HyperMetricStretchInstance::ParameterLength( const MetaParameter* /*p*/, size_type /*tableRow*/ ) const
{
   return 0;
}

// ----------------------------------------------------------------------------

} // pcl

// ----------------------------------------------------------------------------
// EOF HyperMetricStretchInstance.cpp - Released 2025-01-06T00:00:00Z
