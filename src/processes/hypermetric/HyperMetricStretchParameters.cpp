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

#include "HyperMetricStretchParameters.h"
#include "../../core/SensorProfiles.h"

namespace pcl
{

// ----------------------------------------------------------------------------

HMSProcessingMode* TheHMSProcessingModeParameter = nullptr;
HMSSensorProfile* TheHMSSensorProfileParameter = nullptr;
HMSTargetBackground* TheHMSTargetBackgroundParameter = nullptr;
HMSLogD* TheHMSLogDParameter = nullptr;
HMSProtectB* TheHMSProtectBParameter = nullptr;
HMSColorConvergence* TheHMSColorConvergenceParameter = nullptr;
HMSColorStrategy* TheHMSColorStrategyParameter = nullptr;
HMSColorGrip* TheHMSColorGripParameter = nullptr;
HMSShadowConvergence* TheHMSShadowConvergenceParameter = nullptr;
HMSLinearExpansion* TheHMSLinearExpansionParameter = nullptr;
HMSAdaptiveAnchor* TheHMSAdaptiveAnchorParameter = nullptr;

// ----------------------------------------------------------------------------

HMSProcessingMode::HMSProcessingMode( MetaProcess* P ) : MetaEnumeration( P )
{
   TheHMSProcessingModeParameter = this;
}

IsoString HMSProcessingMode::Id() const
{
   return "processingMode";
}

size_type HMSProcessingMode::NumberOfElements() const
{
   return NumberOfModes;
}

IsoString HMSProcessingMode::ElementId( size_type i ) const
{
   switch ( i )
   {
   default:
   case ReadyToUse: return "ReadyToUse";
   case Scientific: return "Scientific";
   }
}

int HMSProcessingMode::ElementValue( size_type i ) const
{
   return int( i );
}

size_type HMSProcessingMode::DefaultValueIndex() const
{
   return Default;
}

// ----------------------------------------------------------------------------

HMSSensorProfile::HMSSensorProfile( MetaProcess* P ) : MetaEnumeration( P )
{
   TheHMSSensorProfileParameter = this;
}

IsoString HMSSensorProfile::Id() const
{
   return "sensorProfile";
}

size_type HMSSensorProfile::NumberOfElements() const
{
   return 27; // Total number of sensor profiles
}

IsoString HMSSensorProfile::ElementId( size_type i ) const
{
   // Hardcoded list of all 27 sensor profile identifiers
   // This avoids dynamic array access during parameter initialization
   switch ( i )
   {
   case 0:  return "Rec709Recommended";
   case 1:  return "SonyIMX571";
   case 2:  return "SonyIMX455";
   case 3:  return "SonyIMX410";
   case 4:  return "SonyIMX269";
   case 5:  return "SonyIMX294";
   case 6:  return "SonyIMX533";
   case 7:  return "SonyIMX676";
   case 8:  return "SonyIMX585";
   case 9:  return "SonyIMX662";
   case 10: return "SonyIMX678";
   case 11: return "SonyIMX462";
   case 12: return "SonyIMX715";
   case 13: return "SonyIMX482";
   case 14: return "SonyIMX183";
   case 15: return "SonyIMX178";
   case 16: return "SonyIMX224";
   case 17: return "CanonEOSModern";
   case 18: return "CanonEOSLegacy";
   case 19: return "NikonDSLRModern";
   case 20: return "NikonDSLRLegacy";
   case 21: return "FujifilmXTrans5HR";
   case 22: return "PanasonicMN34230";
   case 23: return "ZWOSeestarS50";
   case 24: return "ZWOSeestarS30";
   case 25: return "NarrowbandHOO";
   case 26: return "NarrowbandSHO";
   default: return "Rec709Recommended";
   }
}

int HMSSensorProfile::ElementValue( size_type i ) const
{
   return int( i );
}

size_type HMSSensorProfile::DefaultValueIndex() const
{
   return 0; // Rec.709 is the default
}

// ----------------------------------------------------------------------------

HMSTargetBackground::HMSTargetBackground( MetaProcess* P ) : MetaDouble( P )
{
   TheHMSTargetBackgroundParameter = this;
}

IsoString HMSTargetBackground::Id() const
{
   return "targetBackground";
}

int HMSTargetBackground::Precision() const
{
   return 2;
}

double HMSTargetBackground::MinimumValue() const
{
   return 0.05;
}

double HMSTargetBackground::MaximumValue() const
{
   return 0.50;
}

double HMSTargetBackground::DefaultValue() const
{
   return 0.20;
}

// ----------------------------------------------------------------------------

HMSLogD::HMSLogD( MetaProcess* P ) : MetaDouble( P )
{
   TheHMSLogDParameter = this;
}

IsoString HMSLogD::Id() const
{
   return "logD";
}

int HMSLogD::Precision() const
{
   return 6;
}

double HMSLogD::MinimumValue() const
{
   return 0.0;
}

double HMSLogD::MaximumValue() const
{
   return 7.0;
}

double HMSLogD::DefaultValue() const
{
   return 2.0;
}

// ----------------------------------------------------------------------------

HMSProtectB::HMSProtectB( MetaProcess* P ) : MetaDouble( P )
{
   TheHMSProtectBParameter = this;
}

IsoString HMSProtectB::Id() const
{
   return "protectB";
}

int HMSProtectB::Precision() const
{
   return 2;
}

double HMSProtectB::MinimumValue() const
{
   return 0.1;
}

double HMSProtectB::MaximumValue() const
{
   return 15.0;
}

double HMSProtectB::DefaultValue() const
{
   return 6.0;
}

// ----------------------------------------------------------------------------

HMSColorConvergence::HMSColorConvergence( MetaProcess* P ) : MetaDouble( P )
{
   TheHMSColorConvergenceParameter = this;
}

IsoString HMSColorConvergence::Id() const
{
   return "colorConvergence";
}

int HMSColorConvergence::Precision() const
{
   return 2;
}

double HMSColorConvergence::MinimumValue() const
{
   return 1.0;
}

double HMSColorConvergence::MaximumValue() const
{
   return 10.0;
}

double HMSColorConvergence::DefaultValue() const
{
   return 3.5;
}

// ----------------------------------------------------------------------------

HMSColorStrategy::HMSColorStrategy( MetaProcess* P ) : MetaInt32( P )
{
   TheHMSColorStrategyParameter = this;
}

IsoString HMSColorStrategy::Id() const
{
   return "colorStrategy";
}

double HMSColorStrategy::MinimumValue() const
{
   return -100;
}

double HMSColorStrategy::MaximumValue() const
{
   return 100;
}

double HMSColorStrategy::DefaultValue() const
{
   return 0;
}

// ----------------------------------------------------------------------------

HMSColorGrip::HMSColorGrip( MetaProcess* P ) : MetaDouble( P )
{
   TheHMSColorGripParameter = this;
}

IsoString HMSColorGrip::Id() const
{
   return "colorGrip";
}

int HMSColorGrip::Precision() const
{
   return 2;
}

double HMSColorGrip::MinimumValue() const
{
   return 0.0;
}

double HMSColorGrip::MaximumValue() const
{
   return 1.0;
}

double HMSColorGrip::DefaultValue() const
{
   return 1.0;
}

// ----------------------------------------------------------------------------

HMSShadowConvergence::HMSShadowConvergence( MetaProcess* P ) : MetaDouble( P )
{
   TheHMSShadowConvergenceParameter = this;
}

IsoString HMSShadowConvergence::Id() const
{
   return "shadowConvergence";
}

int HMSShadowConvergence::Precision() const
{
   return 2;
}

double HMSShadowConvergence::MinimumValue() const
{
   return 0.0;
}

double HMSShadowConvergence::MaximumValue() const
{
   return 3.0;
}

double HMSShadowConvergence::DefaultValue() const
{
   return 0.0;
}

// ----------------------------------------------------------------------------

HMSLinearExpansion::HMSLinearExpansion( MetaProcess* P ) : MetaDouble( P )
{
   TheHMSLinearExpansionParameter = this;
}

IsoString HMSLinearExpansion::Id() const
{
   return "linearExpansion";
}

int HMSLinearExpansion::Precision() const
{
   return 2;
}

double HMSLinearExpansion::MinimumValue() const
{
   return 0.0;
}

double HMSLinearExpansion::MaximumValue() const
{
   return 1.0;
}

double HMSLinearExpansion::DefaultValue() const
{
   return 0.0;
}

// ----------------------------------------------------------------------------

HMSAdaptiveAnchor::HMSAdaptiveAnchor( MetaProcess* P ) : MetaBoolean( P )
{
   TheHMSAdaptiveAnchorParameter = this;
}

IsoString HMSAdaptiveAnchor::Id() const
{
   return "adaptiveAnchor";
}

bool HMSAdaptiveAnchor::DefaultValue() const
{
   return true;
}

// ----------------------------------------------------------------------------

} // pcl

// ----------------------------------------------------------------------------
// EOF HyperMetricStretchParameters.cpp - Released 2025-01-06T00:00:00Z
