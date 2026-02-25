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

#include "StarComposerProcess.h"
#include "StarComposerParameters.h"
#include "StarComposerInstance.h"
#include "StarComposerInterface.h"

#include <pcl/Console.h>

namespace pcl
{

// ----------------------------------------------------------------------------

StarComposerProcess* TheStarComposerProcess = nullptr;

// ----------------------------------------------------------------------------

StarComposerProcess::StarComposerProcess()
{
   // NOTE: Console is not available during process construction.
   // This constructor is called during module installation.
   
   TheStarComposerProcess = this;
   
   // Register parameters in the same order as Instance member variables
   new SCSStarmaskView( this );
   new SCSStarlessView( this );
   new SCSCompositionMode( this );
   new SCSSensorProfile( this );
   new SCSLogD( this );
   new SCSProfileHardness( this );
   new SCSAdaptiveAnchor( this );          // Moved before ColorGrip
   new SCSColorGrip( this );
   new SCSShadowConvergence( this );
   new SCSCoreRejection( this );
   new SCSMorphReduction( this );
   new SCSOpticalHealing( this );
}

// ----------------------------------------------------------------------------

IsoString StarComposerProcess::Id() const
{
   return "StarComposer";
}

// ----------------------------------------------------------------------------

IsoString StarComposerProcess::Category() const
{
   return "VeraLux";
}

// ----------------------------------------------------------------------------

uint32 StarComposerProcess::Version() const
{
   return 0x202; // Version 2.0.2
}

// ----------------------------------------------------------------------------

String StarComposerProcess::Description() const
{
   return "<html>"
          "<p>VeraLux StarComposer - High-Fidelity Star Reconstruction Engine</p>"
          "<p>A specialized photometric reconstruction engine designed for deep-sky astrophotography. "
          "Solves the \"bloating\" and \"bleaching\" issues inherent in standard star stretching by "
          "decoupling the stellar field from the main object. Leverages the rigorous Inverse Hyperbolic "
          "Stretch (IHS) to develop linear star masks with precision, preserving true stellar color and "
          "geometry (PSF) before compositing them onto non-linear starless images.</p>"
          "<p><b>Key Features:</b></p>"
          "<ul>"
          "<li><b>Hybrid Scalar/Vector Engine:</b> Ensures stars maintain solid white cores while preserving "
          "chromatic data in halos.</li>"
          "<li><b>Star Surgery:</b> Advanced operations including Large Structure Rejection (LSR), "
          "Optical Healing for chromatic aberration, and morphological star reduction.</li>"
          "<li><b>Dual Composition Modes:</b> Screen (safe, no clipping) and Linear Add (physical light addition).</li>"
          "<li><b>True Color Pipeline:</b> Vector preservation via Color Grip with sensor-specific "
          "quantum efficiency weighting.</li>"
          "</ul>"
          "<p>Original algorithm by Riccardo Paterniti (2025) - VeraLux</p>"
          "<p>PixInsight port by Lucas Saavedra Vaz (2026)</p>"
          "</html>";
}

// ----------------------------------------------------------------------------

String StarComposerProcess::IconImageSVGFile() const
{
   return "@module_icons_dir/StarComposer.svg";
}

// ----------------------------------------------------------------------------

ProcessInterface* StarComposerProcess::DefaultInterface() const
{
   return TheStarComposerInterface;
}

// ----------------------------------------------------------------------------

ProcessImplementation* StarComposerProcess::Create() const
{
   return new StarComposerInstance( this );
}

// ----------------------------------------------------------------------------

ProcessImplementation* StarComposerProcess::Clone( const ProcessImplementation& p ) const
{
   const StarComposerInstance* instPtr = dynamic_cast<const StarComposerInstance*>( &p );
   return (instPtr != nullptr) ? new StarComposerInstance( *instPtr ) : nullptr;
}

// ----------------------------------------------------------------------------

} // pcl

// ----------------------------------------------------------------------------
