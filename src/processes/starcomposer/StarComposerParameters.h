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

#ifndef __StarComposerParameters_h
#define __StarComposerParameters_h

#include <pcl/MetaParameter.h>

namespace pcl
{

PCL_BEGIN_LOCAL

// ----------------------------------------------------------------------------
// Input Images
// ----------------------------------------------------------------------------

class SCSStarmaskView : public MetaString
{
public:
   SCSStarmaskView( MetaProcess* );
   
   IsoString Id() const override;
   size_type MinLength() const override;
   String DefaultValue() const override;
};

extern SCSStarmaskView* TheSCSStarmaskViewParameter;

// ----------------------------------------------------------------------------

class SCSStarlessView : public MetaString
{
public:
   SCSStarlessView( MetaProcess* );
   
   IsoString Id() const override;
   size_type MinLength() const override;
   String DefaultValue() const override;
};

extern SCSStarlessView* TheSCSStarlessViewParameter;

// ----------------------------------------------------------------------------
// Composition Mode
// ----------------------------------------------------------------------------

class SCSCompositionMode : public MetaEnumeration
{
public:
   enum { Screen,
          LinearAdd,
          NumberOfModes,
          Default = Screen };
   
   SCSCompositionMode( MetaProcess* );
   
   IsoString Id() const override;
   size_type NumberOfElements() const override;
   IsoString ElementId( size_type ) const override;
   int ElementValue( size_type ) const override;
   size_type DefaultValueIndex() const override;
};

extern SCSCompositionMode* TheSCSCompositionModeParameter;

// ----------------------------------------------------------------------------
// Sensor Profile
// ----------------------------------------------------------------------------

class SCSSensorProfile : public MetaEnumeration
{
public:
   SCSSensorProfile( MetaProcess* );
   
   IsoString Id() const override;
   size_type NumberOfElements() const override;
   IsoString ElementId( size_type ) const override;
   int ElementValue( size_type ) const override;
   size_type DefaultValueIndex() const override;
};

extern SCSSensorProfile* TheSCSSensorProfileParameter;

// ----------------------------------------------------------------------------
// VeraLux Stretch Parameters
// ----------------------------------------------------------------------------

class SCSLogD : public MetaDouble
{
public:
   SCSLogD( MetaProcess* );
   
   IsoString Id() const override;
   int Precision() const override;
   double MinimumValue() const override;
   double MaximumValue() const override;
   double DefaultValue() const override;
};

extern SCSLogD* TheSCSLogDParameter;

// ----------------------------------------------------------------------------

class SCSProfileHardness : public MetaDouble
{
public:
   SCSProfileHardness( MetaProcess* );
   
   IsoString Id() const override;
   int Precision() const override;
   double MinimumValue() const override;
   double MaximumValue() const override;
   double DefaultValue() const override;
};

extern SCSProfileHardness* TheSCSProfileHardnessParameter;

// ----------------------------------------------------------------------------

class SCSAdaptiveAnchor : public MetaBoolean
{
public:
   SCSAdaptiveAnchor( MetaProcess* );
   
   IsoString Id() const override;
   bool DefaultValue() const override;
};

extern SCSAdaptiveAnchor* TheSCSAdaptiveAnchorParameter;

// ----------------------------------------------------------------------------
// Hybrid Physics Parameters
// ----------------------------------------------------------------------------

class SCSColorGrip : public MetaDouble
{
public:
   SCSColorGrip( MetaProcess* );
   
   IsoString Id() const override;
   int Precision() const override;
   double MinimumValue() const override;
   double MaximumValue() const override;
   double DefaultValue() const override;
};

extern SCSColorGrip* TheSCSColorGripParameter;

// ----------------------------------------------------------------------------

class SCSShadowConvergence : public MetaDouble
{
public:
   SCSShadowConvergence( MetaProcess* );
   
   IsoString Id() const override;
   int Precision() const override;
   double MinimumValue() const override;
   double MaximumValue() const override;
   double DefaultValue() const override;
};

extern SCSShadowConvergence* TheSCSShadowConvergenceParameter;

// ----------------------------------------------------------------------------
// Star Surgery Parameters
// ----------------------------------------------------------------------------

class SCSCoreRejection : public MetaDouble
{
public:
   SCSCoreRejection( MetaProcess* );
   
   IsoString Id() const override;
   int Precision() const override;
   double MinimumValue() const override;
   double MaximumValue() const override;
   double DefaultValue() const override;
};

extern SCSCoreRejection* TheSCSCoreRejectionParameter;

// ----------------------------------------------------------------------------

class SCSMorphReduction : public MetaDouble
{
public:
   SCSMorphReduction( MetaProcess* );
   
   IsoString Id() const override;
   int Precision() const override;
   double MinimumValue() const override;
   double MaximumValue() const override;
   double DefaultValue() const override;
};

extern SCSMorphReduction* TheSCSMorphReductionParameter;

// ----------------------------------------------------------------------------

class SCSOpticalHealing : public MetaDouble
{
public:
   SCSOpticalHealing( MetaProcess* );
   
   IsoString Id() const override;
   int Precision() const override;
   double MinimumValue() const override;
   double MaximumValue() const override;
   double DefaultValue() const override;
};

extern SCSOpticalHealing* TheSCSOpticalHealingParameter;

// ----------------------------------------------------------------------------

PCL_END_LOCAL

} // pcl

#endif   // __StarComposerParameters_h

// ----------------------------------------------------------------------------
