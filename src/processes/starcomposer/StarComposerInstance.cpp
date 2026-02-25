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

#include "StarComposerInstance.h"
#include "StarComposerParameters.h"

#include "../../core/VeraLuxEngine.h"
#include "../../core/StarEngine.h"

#include <pcl/AutoViewLock.h>
#include <pcl/Console.h>
#include <pcl/ImageWindow.h>
#include <pcl/PixelInterpolation.h>
#include <pcl/Resample.h>
#include <pcl/StandardStatus.h>
#include <pcl/View.h>

namespace pcl
{

// ----------------------------------------------------------------------------

StarComposerInstance::StarComposerInstance( const MetaProcess* m )
   : ProcessImplementation( m )
   , starmaskView()
   , starlessView()
   , compositionMode( SCSCompositionMode::Default )
   , sensorProfile( g_defaultSensorProfileIndex )
   , logD( 1.0 )
   , profileHardness( 50.0 )
   , adaptiveAnchor( true )
   , colorGrip( 0.5 )
   , shadowConvergence( 0.0 )
   , coreRejection( 0.0 )
   , morphReduction( 0.0 )
   , opticalHealing( 0.0 )
{
}

// ----------------------------------------------------------------------------

StarComposerInstance::StarComposerInstance( const StarComposerInstance& x )
   : ProcessImplementation( x )
{
   Assign( x );
}

// ----------------------------------------------------------------------------

void StarComposerInstance::Assign( const ProcessImplementation& p )
{
   const StarComposerInstance* x = dynamic_cast<const StarComposerInstance*>( &p );
   if ( x != nullptr )
   {
      starmaskView = x->starmaskView;
      starlessView = x->starlessView;
      compositionMode = x->compositionMode;
      sensorProfile = x->sensorProfile;
      logD = x->logD;
      profileHardness = x->profileHardness;
      adaptiveAnchor = x->adaptiveAnchor;
      colorGrip = x->colorGrip;
      shadowConvergence = x->shadowConvergence;
      coreRejection = x->coreRejection;
      morphReduction = x->morphReduction;
      opticalHealing = x->opticalHealing;
   }
}

// ----------------------------------------------------------------------------

UndoFlags StarComposerInstance::UndoMode( const View& ) const
{
   // We don't modify the target view - we create a new output window.
   return UndoFlag::DefaultMode;
}

// ----------------------------------------------------------------------------

bool StarComposerInstance::CanExecuteOn( const View&, String& whyNot ) const
{
   // Ignore the target view - we use the starmask/starless views from the instance
   if ( starmaskView.IsEmpty() )
   {
      whyNot = "No starmask view specified. Please select a linear starmask image.";
      return false;
   }

   if ( starlessView.IsEmpty() )
   {
      whyNot = "No starless view specified. Please select a stretched starless image.";
      return false;
   }

   View starmaskV = View::ViewById( starmaskView );
   if ( starmaskV.IsNull() )
   {
      whyNot = "Starmask view not found: " + starmaskView;
      return false;
   }

   View starlessV = View::ViewById( starlessView );
   if ( starlessV.IsNull() )
   {
      whyNot = "Starless view not found: " + starlessView;
      return false;
   }

   return true;
}

// ----------------------------------------------------------------------------

bool StarComposerInstance::ExecuteOn( View& )
{
   /*
    * Python reference: process_star_pipeline:439-506 and process_full_resolution:1390-1465
    * 
    * Complete StarComposer pipeline:
    * 1. Load input images
    * 2. Normalize and condition starmask
    * 3. Calculate anchor
    * 4. Apply hybrid star stretch
    * 5. Apply star surgery operations
    * 6. Compose with starless background
    * 7. Create new output image window
    */

   Console console;
   console.EnableAbort();

   try
   {
      // Step 1: Load input views
      console.WriteLn( "<end><cbr>VeraLux StarComposer v2.0.2" );
      console.WriteLn( "High-Fidelity Star Reconstruction Engine" );
      console.WriteLn();

      // Load starmask view
      View starmaskV = View::ViewById( starmaskView );
      if ( starmaskV.IsNull() )
         throw Error( "Starmask view not found: " + starmaskView );

      // Load starless view
      View starlessV = View::ViewById( starlessView );
      if ( starlessV.IsNull() )
         throw Error( "Starless view not found: " + starlessView );

      console.WriteLn( String().Format( "Starmask: %s", starmaskView.c_str() ) );
      console.WriteLn( String().Format( "Starless: %s", starlessView.c_str() ) );
      console.WriteLn( String().Format( "Composition Mode: %s",
                       (compositionMode == SCSCompositionMode::Screen) ? "Screen" : "Linear Add" ) );
      console.WriteLn();

      const SensorProfile& profile = GetSensorProfile();
      console.WriteLn( String().Format( "Sensor Profile: %s", profile.name.c_str() ) );

      // Step 2: Get starmask and normalize
      console.WriteLn( "Loading starmask..." );
      AutoViewLock maskLock( starmaskV, false );
      Image starmask;
      StarEngine::NormalizeStarInput( starmask, starmaskV.Image() );

      if ( starmask.NumberOfChannels() != 3 )
         throw Error( "Starmask must be an RGB image or grayscale (will be converted to RGB)." );

      console.WriteLn( String().Format( "Starmask dimensions: %dx%d",
                       starmask.Width(), starmask.Height() ) );

      // Step 3: Signal conditioning
      console.WriteLn( "Applying signal conditioning (Gamma 2.4 + micro-blur)..." );
      StarEngine::ApplySignalConditioning( starmask );

      // Step 4: Calculate anchor
      double anchor;
      if ( adaptiveAnchor )
      {
         console.WriteLn( "Calculating adaptive anchor (morphological)..." );
         anchor = VeraLuxEngine::CalculateAnchorAdaptive( starmask, profile );
      }
      else
      {
         console.WriteLn( "Calculating anchor (statistical)..." );
         anchor = VeraLuxEngine::CalculateAnchor( starmask );
      }
      console.WriteLn( String().Format( "Anchor: %.6f", anchor ) );

      // Step 5: Apply hybrid star stretch
      console.WriteLn( String().Format( "Applying hybrid star stretch (Log D=%.2f, b=%.2f)...",
                       logD, profileHardness ) );
      console.WriteLn( String().Format( "  Color Grip: %.2f, Shadow Convergence: %.2f",
                       colorGrip, shadowConvergence ) );

      double D = Pow10( logD );
      Image stars;
      StarEngine::HybridStretch( stars, starmask, anchor, D, profileHardness,
                                 profile, colorGrip, shadowConvergence, adaptiveAnchor );

      // Step 6: Star surgery operations (if enabled)
      bool surgeryApplied = false;

      if ( coreRejection > 0.001 )
      {
         console.WriteLn( String().Format( "Applying Large Structure Rejection (LSR): %.2f",
                          coreRejection ) );
         StarEngine::ApplyLargeStructureRejection( stars, coreRejection );
         surgeryApplied = true;
      }

      if ( opticalHealing > 0.001 )
      {
         console.WriteLn( String().Format( "Applying Optical Healing: %.1f", opticalHealing ) );
         StarEngine::ApplyOpticalHealing( stars, opticalHealing );
         surgeryApplied = true;
      }

      if ( morphReduction > 0.001 )
      {
         console.WriteLn( String().Format( "Applying Morphological Reduction: %.2f",
                          morphReduction ) );
         StarEngine::ApplyStarReduction( stars, morphReduction );
         surgeryApplied = true;
      }

      if ( surgeryApplied )
         console.WriteLn( "Star surgery operations completed." );

      // Step 7: Get starless background
      console.WriteLn( "Loading starless background..." );
      AutoViewLock bgLock( starlessV, false );
      Image starless;
      VeraLuxEngine::NormalizeInput( starless, starlessV.Image() );

      if ( starless.NumberOfChannels() != 3 )
         throw Error( "Starless image must be an RGB image." );

      console.WriteLn( String().Format( "Starless dimensions: %dx%d",
                       starless.Width(), starless.Height() ) );

      // Step 8: Compose
      console.WriteLn( "Composing stars with starless background..." );
      Image result;
      bool useScreen = (compositionMode == SCSCompositionMode::Screen);
      StarEngine::ComposeImages( result, starless, stars, useScreen );

      console.WriteLn( String().Format( "Result dimensions: %dx%d",
                       result.Width(), result.Height() ) );

      // Step 9: Create new output image window
      console.WriteLn( "Creating output image..." );

      ImageWindow outputWindow( result.Width(), result.Height(), 3,
                                32, true, true, true, "StarComposer_result" );
      if ( outputWindow.IsNull() )
         throw Error( "Unable to create output image window." );

      outputWindow.MainView().Image().CopyImage( result );
      outputWindow.Show();

      console.WriteLn( "<end><cbr>VeraLux StarComposer: Processing completed successfully." );

      return true;
   }
   catch ( ProcessAborted& )
   {
      throw;
   }
   catch ( ... )
   {
      throw;
   }
}

// ----------------------------------------------------------------------------

bool StarComposerInstance::Preview( Image& img ) const
{
   /*
    * Real-time preview of the StarComposer pipeline.
    *
    * Strategy:
    * 1. Record the target dimensions from the incoming preview image.
    * 2. Read both starmask and starless views.
    * 3. Resample both to match the preview dimensions (fast on small images).
    * 4. Run the full pipeline on the small preview-sized images.
    * 5. Output must match the original preview dimensions exactly.
    */
   try
   {
      // Validate inputs
      if ( starmaskView.IsEmpty() || starlessView.IsEmpty() )
         return false;

      View starmaskV = View::ViewById( starmaskView );
      View starlessV = View::ViewById( starlessView );
      if ( starmaskV.IsNull() || starlessV.IsNull() )
         return false;

      // Target dimensions must match the preview image exactly
      const int previewW = img.Width();
      const int previewH = img.Height();
      if ( previewW < 1 || previewH < 1 )
         return false;

      // Read and normalize starmask
      Image starmask;
      StarEngine::NormalizeStarInput( starmask, starmaskV.Image() );
      if ( starmask.NumberOfChannels() != 3 )
         return false;

      // Read and normalize starless
      Image starless;
      VeraLuxEngine::NormalizeInput( starless, starlessV.Image() );
      if ( starless.NumberOfChannels() != 3 )
         return false;

      // Compute scale factor before downsampling so blur kernels and morphological
      // operations are proportionally reduced for the preview resolution.
      const double scaleFactor = double( Min( previewW, previewH ) )
                               / double( Min( starmask.Width(), starmask.Height() ) );

      // Resample both inputs to preview dimensions using PCL Resample
      if ( starmask.Width() != previewW || starmask.Height() != previewH )
      {
         BilinearPixelInterpolation interp;
         Resample R( interp );
         R.SetMode( ResizeMode::AbsolutePixels );
         R.SetSizes( previewW, previewH );
         R >> starmask;
      }

      if ( starless.Width() != previewW || starless.Height() != previewH )
      {
         BilinearPixelInterpolation interp;
         Resample R( interp );
         R.SetMode( ResizeMode::AbsolutePixels );
         R.SetSizes( previewW, previewH );
         R >> starless;
      }

      // Pipeline Step 1: Signal conditioning (sigma scaled for preview)
      StarEngine::ApplySignalConditioning( starmask, scaleFactor );

      // Pipeline Step 2: Calculate anchor
      const SensorProfile& profile = GetSensorProfile();
      double anchor;
      if ( adaptiveAnchor )
         anchor = VeraLuxEngine::CalculateAnchorAdaptive( starmask, profile );
      else
         anchor = VeraLuxEngine::CalculateAnchor( starmask );

      // Pipeline Step 3: Hybrid star stretch
      double D = Pow10( logD );
      Image stars;
      StarEngine::HybridStretch( stars, starmask, anchor, D, profileHardness,
                                 profile, colorGrip, shadowConvergence, adaptiveAnchor );

      // Pipeline Step 4: Star surgery (kernels/intensity scaled for preview)
      if ( coreRejection > 0.001 )
         StarEngine::ApplyLargeStructureRejection( stars, coreRejection );
      if ( opticalHealing > 0.001 )
         StarEngine::ApplyOpticalHealing( stars, opticalHealing, scaleFactor );
      if ( morphReduction > 0.001 )
         StarEngine::ApplyStarReduction( stars, morphReduction, scaleFactor );

      // Pipeline Step 5: Compose
      Image result;
      bool useScreen = (compositionMode == SCSCompositionMode::Screen);
      StarEngine::ComposeImages( result, starless, stars, useScreen );

      // Write result back (dimensions should already match preview)
      img.Assign( result );
      return true;
   }
   catch ( ... )
   {
      return false;
   }
}

// ----------------------------------------------------------------------------

void* StarComposerInstance::LockParameter( const MetaParameter* p, size_type /*tableRow*/ )
{
   if ( p == TheSCSStarmaskViewParameter )
      return starmaskView.Begin();
   if ( p == TheSCSStarlessViewParameter )
      return starlessView.Begin();
   if ( p == TheSCSCompositionModeParameter )
      return &compositionMode;
   if ( p == TheSCSSensorProfileParameter )
      return &sensorProfile;
   if ( p == TheSCSLogDParameter )
      return &logD;
   if ( p == TheSCSProfileHardnessParameter )
      return &profileHardness;
   if ( p == TheSCSAdaptiveAnchorParameter )
      return &adaptiveAnchor;
   if ( p == TheSCSColorGripParameter )
      return &colorGrip;
   if ( p == TheSCSShadowConvergenceParameter )
      return &shadowConvergence;
   if ( p == TheSCSCoreRejectionParameter )
      return &coreRejection;
   if ( p == TheSCSMorphReductionParameter )
      return &morphReduction;
   if ( p == TheSCSOpticalHealingParameter )
      return &opticalHealing;

   return nullptr;
}

// ----------------------------------------------------------------------------

bool StarComposerInstance::AllocateParameter( size_type sizeOrLength, const MetaParameter* p, size_type /*tableRow*/ )
{
   if ( p == TheSCSStarmaskViewParameter )
   {
      starmaskView.Clear();
      if ( sizeOrLength > 0 )
         starmaskView.SetLength( sizeOrLength );
      return true;
   }

   if ( p == TheSCSStarlessViewParameter )
   {
      starlessView.Clear();
      if ( sizeOrLength > 0 )
         starlessView.SetLength( sizeOrLength );
      return true;
   }

   return true;
}

// ----------------------------------------------------------------------------

size_type StarComposerInstance::ParameterLength( const MetaParameter* p, size_type /*tableRow*/ ) const
{
   if ( p == TheSCSStarmaskViewParameter )
      return starmaskView.Length();
   if ( p == TheSCSStarlessViewParameter )
      return starlessView.Length();

   return 0;
}

// ----------------------------------------------------------------------------

} // pcl

// ----------------------------------------------------------------------------
