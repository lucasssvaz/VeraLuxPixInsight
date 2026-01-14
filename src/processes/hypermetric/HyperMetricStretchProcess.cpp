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

#include "HyperMetricStretchProcess.h"
#include "HyperMetricStretchParameters.h"
#include "HyperMetricStretchInstance.h"
#include "HyperMetricStretchInterface.h"

#include <pcl/Console.h>

namespace pcl
{

// ----------------------------------------------------------------------------

HyperMetricStretchProcess* TheHyperMetricStretchProcess = nullptr;

// ----------------------------------------------------------------------------

HyperMetricStretchProcess::HyperMetricStretchProcess()
{
   // NOTE: Console is not available during process construction.
   // This constructor is called during module installation.
   
   TheHyperMetricStretchProcess = this;
   
   new HMSProcessingMode( this );
   new HMSSensorProfile( this );
   new HMSTargetBackground( this );
   new HMSLogD( this );
   new HMSProtectB( this );
   new HMSColorConvergence( this );
   new HMSColorStrategy( this );
   new HMSColorGrip( this );
   new HMSShadowConvergence( this );
   new HMSLinearExpansion( this );
   new HMSAdaptiveAnchor( this );
}

// ----------------------------------------------------------------------------

IsoString HyperMetricStretchProcess::Id() const
{
   return "HyperMetricStretch";
}

// ----------------------------------------------------------------------------

IsoString HyperMetricStretchProcess::Category() const
{
   return "VeraLux";
}

// ----------------------------------------------------------------------------

uint32 HyperMetricStretchProcess::Version() const
{
   return 0x150; // Version 1.5.0
}

// ----------------------------------------------------------------------------

String HyperMetricStretchProcess::Description() const
{
   return "<html>"
          "<p>VeraLux HyperMetric Stretch - Photometric Hyperbolic Stretch Engine</p>"
          "<p>A precision linear-to-nonlinear stretching engine designed to maximize sensor "
          "fidelity while managing the transition to the visible domain. Implements inverse "
          "hyperbolic sine (arcsinh) stretching with sensor-specific quantum efficiency "
          "weighting and vector color preservation.</p>"
          "<p>Original algorithm by Riccardo Paterniti (2025) - VeraLux</p>"
          "<p>PixInsight port by Lucas Saavedra Vaz (2025)</p>"
          "</html>";
}

// ----------------------------------------------------------------------------

String HyperMetricStretchProcess::IconImageSVGFile() const
{
   return "@module_icons_dir/HyperMetricStretch.svg";
}

// ----------------------------------------------------------------------------

ProcessInterface* HyperMetricStretchProcess::DefaultInterface() const
{
   return TheHyperMetricStretchInterface;
}

// ----------------------------------------------------------------------------

ProcessImplementation* HyperMetricStretchProcess::Create() const
{
   return new HyperMetricStretchInstance( this );
}

// ----------------------------------------------------------------------------

ProcessImplementation* HyperMetricStretchProcess::Clone( const ProcessImplementation& p ) const
{
   const HyperMetricStretchInstance* instPtr = dynamic_cast<const HyperMetricStretchInstance*>( &p );
   return (instPtr != nullptr) ? new HyperMetricStretchInstance( *instPtr ) : nullptr;
}

// ----------------------------------------------------------------------------

} // pcl

// ----------------------------------------------------------------------------
