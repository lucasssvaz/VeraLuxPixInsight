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

#include "StarComposerParameters.h"
#include "../../core/SensorProfiles.h"  // For g_sensorProfileData constexpr

namespace pcl
{

// ----------------------------------------------------------------------------
// Input Images
// ----------------------------------------------------------------------------

SCSStarmaskView* TheSCSStarmaskViewParameter = nullptr;

SCSStarmaskView::SCSStarmaskView( MetaProcess* P )
   : MetaString( P )
{
   TheSCSStarmaskViewParameter = this;
}

IsoString SCSStarmaskView::Id() const
{
   return "starmaskView";
}

size_type SCSStarmaskView::MinLength() const
{
   return 0; // Empty string allowed (no view selected)
}

String SCSStarmaskView::DefaultValue() const
{
   return String(); // Empty by default
}

// ----------------------------------------------------------------------------

SCSStarlessView* TheSCSStarlessViewParameter = nullptr;

SCSStarlessView::SCSStarlessView( MetaProcess* P )
   : MetaString( P )
{
   TheSCSStarlessViewParameter = this;
}

IsoString SCSStarlessView::Id() const
{
   return "starlessView";
}

size_type SCSStarlessView::MinLength() const
{
   return 0; // Empty string allowed (no view selected)
}

String SCSStarlessView::DefaultValue() const
{
   return String(); // Empty by default
}

// ----------------------------------------------------------------------------
// Composition Mode
// ----------------------------------------------------------------------------

SCSCompositionMode* TheSCSCompositionModeParameter = nullptr;

SCSCompositionMode::SCSCompositionMode( MetaProcess* P )
   : MetaEnumeration( P )
{
   TheSCSCompositionModeParameter = this;
}

IsoString SCSCompositionMode::Id() const
{
   return "compositionMode";
}

size_type SCSCompositionMode::NumberOfElements() const
{
   return NumberOfModes;
}

IsoString SCSCompositionMode::ElementId( size_type i ) const
{
   switch ( i )
   {
   case Screen:    return "Screen";
   case LinearAdd: return "LinearAdd";
   default:        return IsoString();
   }
}

int SCSCompositionMode::ElementValue( size_type i ) const
{
   return int( i );
}

size_type SCSCompositionMode::DefaultValueIndex() const
{
   return Default;
}

// ----------------------------------------------------------------------------
// Sensor Profile
// ----------------------------------------------------------------------------

SCSSensorProfile* TheSCSSensorProfileParameter = nullptr;

SCSSensorProfile::SCSSensorProfile( MetaProcess* P )
   : MetaEnumeration( P )
{
   TheSCSSensorProfileParameter = this;
}

IsoString SCSSensorProfile::Id() const
{
   return "sensorProfile";
}

size_type SCSSensorProfile::NumberOfElements() const
{
   // Uses constexpr g_numSensorProfiles from header - automatically updated!
   return g_numSensorProfiles;
}

IsoString SCSSensorProfile::ElementId( size_type i ) const
{
   // Prefix with 'S' to create valid identifier (S0, S1, S2...)
   return IsoString().Format( "S%zu", i );
}

int SCSSensorProfile::ElementValue( size_type i ) const
{
   return int( i );
}

size_type SCSSensorProfile::DefaultValueIndex() const
{
   return g_defaultSensorProfileIndex;
}

// ----------------------------------------------------------------------------
// VeraLux Stretch Parameters
// ----------------------------------------------------------------------------

SCSLogD* TheSCSLogDParameter = nullptr;

SCSLogD::SCSLogD( MetaProcess* P )
   : MetaDouble( P )
{
   TheSCSLogDParameter = this;
}

IsoString SCSLogD::Id() const
{
   return "logD";
}

int SCSLogD::Precision() const
{
   return 2;
}

double SCSLogD::MinimumValue() const
{
   return 1.0; // Python: s_D range 0-1000 → Log D = 1.0-21.0
}

double SCSLogD::MaximumValue() const
{
   return 21.0;
}

double SCSLogD::DefaultValue() const
{
   return 1.0; // Python default: s_D.setValue(0) → Log D = 1.0
}

// ----------------------------------------------------------------------------

SCSProfileHardness* TheSCSProfileHardnessParameter = nullptr;

SCSProfileHardness::SCSProfileHardness( MetaProcess* P )
   : MetaDouble( P )
{
   TheSCSProfileHardnessParameter = this;
}

IsoString SCSProfileHardness::Id() const
{
   return "profileHardness";
}

int SCSProfileHardness::Precision() const
{
   return 1;
}

double SCSProfileHardness::MinimumValue() const
{
   return 1.0; // Python: s_b range 10-1000 → b = 1.0-100.0
}

double SCSProfileHardness::MaximumValue() const
{
   return 100.0;
}

double SCSProfileHardness::DefaultValue() const
{
   return 50.0; // Python default: s_b.setValue(500) → b = 50.0
}

// ----------------------------------------------------------------------------

SCSAdaptiveAnchor* TheSCSAdaptiveAnchorParameter = nullptr;

SCSAdaptiveAnchor::SCSAdaptiveAnchor( MetaProcess* P )
   : MetaBoolean( P )
{
   TheSCSAdaptiveAnchorParameter = this;
}

IsoString SCSAdaptiveAnchor::Id() const
{
   return "adaptiveAnchor";
}

bool SCSAdaptiveAnchor::DefaultValue() const
{
   return true; // Python default: chk_adapt.setChecked(True)
}

// ----------------------------------------------------------------------------
// Hybrid Physics Parameters
// ----------------------------------------------------------------------------

SCSColorGrip* TheSCSColorGripParameter = nullptr;

SCSColorGrip::SCSColorGrip( MetaProcess* P )
   : MetaDouble( P )
{
   TheSCSColorGripParameter = this;
}

IsoString SCSColorGrip::Id() const
{
   return "colorGrip";
}

int SCSColorGrip::Precision() const
{
   return 2;
}

double SCSColorGrip::MinimumValue() const
{
   return 0.0; // Python: s_grip range 0-100 → grip = 0.0-1.0
}

double SCSColorGrip::MaximumValue() const
{
   return 1.0;
}

double SCSColorGrip::DefaultValue() const
{
   return 0.5; // Python default: s_grip.setValue(50) → grip = 0.5
}

// ----------------------------------------------------------------------------

SCSShadowConvergence* TheSCSShadowConvergenceParameter = nullptr;

SCSShadowConvergence::SCSShadowConvergence( MetaProcess* P )
   : MetaDouble( P )
{
   TheSCSShadowConvergenceParameter = this;
}

IsoString SCSShadowConvergence::Id() const
{
   return "shadowConvergence";
}

int SCSShadowConvergence::Precision() const
{
   return 2;
}

double SCSShadowConvergence::MinimumValue() const
{
   return 0.0; // Python: s_shad range 0-300 → shadow = 0.0-3.0
}

double SCSShadowConvergence::MaximumValue() const
{
   return 3.0;
}

double SCSShadowConvergence::DefaultValue() const
{
   return 0.0; // Python default: s_shad.setValue(0) → shadow = 0.0
}

// ----------------------------------------------------------------------------
// Star Surgery Parameters
// ----------------------------------------------------------------------------

SCSCoreRejection* TheSCSCoreRejectionParameter = nullptr;

SCSCoreRejection::SCSCoreRejection( MetaProcess* P )
   : MetaDouble( P )
{
   TheSCSCoreRejectionParameter = this;
}

IsoString SCSCoreRejection::Id() const
{
   return "coreRejection";
}

int SCSCoreRejection::Precision() const
{
   return 2;
}

double SCSCoreRejection::MinimumValue() const
{
   return 0.0; // Python: s_lsr range 0-100 → lsr = 0.0-1.0
}

double SCSCoreRejection::MaximumValue() const
{
   return 1.0;
}

double SCSCoreRejection::DefaultValue() const
{
   return 0.0; // Python default: s_lsr.setValue(0) → lsr = 0.0
}

// ----------------------------------------------------------------------------

SCSMorphReduction* TheSCSMorphReductionParameter = nullptr;

SCSMorphReduction::SCSMorphReduction( MetaProcess* P )
   : MetaDouble( P )
{
   TheSCSMorphReductionParameter = this;
}

IsoString SCSMorphReduction::Id() const
{
   return "morphReduction";
}

int SCSMorphReduction::Precision() const
{
   return 2;
}

double SCSMorphReduction::MinimumValue() const
{
   return 0.0; // Python: s_red range 0-100 → red = 0.0-1.0
}

double SCSMorphReduction::MaximumValue() const
{
   return 1.0;
}

double SCSMorphReduction::DefaultValue() const
{
   return 0.0; // Python default: s_red.setValue(0) → red = 0.0
}

// ----------------------------------------------------------------------------

SCSOpticalHealing* TheSCSOpticalHealingParameter = nullptr;

SCSOpticalHealing::SCSOpticalHealing( MetaProcess* P )
   : MetaDouble( P )
{
   TheSCSOpticalHealingParameter = this;
}

IsoString SCSOpticalHealing::Id() const
{
   return "opticalHealing";
}

int SCSOpticalHealing::Precision() const
{
   return 1;
}

double SCSOpticalHealing::MinimumValue() const
{
   return 0.0; // Python: s_heal range 0-20
}

double SCSOpticalHealing::MaximumValue() const
{
   return 20.0;
}

double SCSOpticalHealing::DefaultValue() const
{
   return 0.0; // Python default: s_heal.setValue(0)
}

// ----------------------------------------------------------------------------

} // pcl

// ----------------------------------------------------------------------------
