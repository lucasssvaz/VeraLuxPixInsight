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

#ifndef __HyperMetricStretchParameters_h
#define __HyperMetricStretchParameters_h

#include <pcl/MetaParameter.h>

namespace pcl
{

PCL_BEGIN_LOCAL

// ----------------------------------------------------------------------------

class HMSProcessingMode : public MetaEnumeration
{
public:
   enum { ReadyToUse,
          Scientific,
          NumberOfModes,
          Default = ReadyToUse };

   HMSProcessingMode( MetaProcess* );

   IsoString Id() const override;
   size_type NumberOfElements() const override;
   IsoString ElementId( size_type ) const override;
   int ElementValue( size_type ) const override;
   size_type DefaultValueIndex() const override;
};

extern HMSProcessingMode* TheHMSProcessingModeParameter;

// ----------------------------------------------------------------------------

class HMSSensorProfile : public MetaEnumeration
{
public:
   HMSSensorProfile( MetaProcess* );

   IsoString Id() const override;
   size_type NumberOfElements() const override;
   IsoString ElementId( size_type ) const override;
   int ElementValue( size_type ) const override;
   size_type DefaultValueIndex() const override;
};

extern HMSSensorProfile* TheHMSSensorProfileParameter;

// ----------------------------------------------------------------------------

class HMSTargetBackground : public MetaDouble
{
public:
   HMSTargetBackground( MetaProcess* );

   IsoString Id() const override;
   int Precision() const override;
   double MinimumValue() const override;
   double MaximumValue() const override;
   double DefaultValue() const override;
};

extern HMSTargetBackground* TheHMSTargetBackgroundParameter;

// ----------------------------------------------------------------------------

class HMSLogD : public MetaDouble
{
public:
   HMSLogD( MetaProcess* );

   IsoString Id() const override;
   int Precision() const override;
   double MinimumValue() const override;
   double MaximumValue() const override;
   double DefaultValue() const override;
};

extern HMSLogD* TheHMSLogDParameter;

// ----------------------------------------------------------------------------

class HMSProtectB : public MetaDouble
{
public:
   HMSProtectB( MetaProcess* );

   IsoString Id() const override;
   int Precision() const override;
   double MinimumValue() const override;
   double MaximumValue() const override;
   double DefaultValue() const override;
};

extern HMSProtectB* TheHMSProtectBParameter;

// ----------------------------------------------------------------------------

class HMSColorConvergence : public MetaDouble
{
public:
   HMSColorConvergence( MetaProcess* );

   IsoString Id() const override;
   int Precision() const override;
   double MinimumValue() const override;
   double MaximumValue() const override;
   double DefaultValue() const override;
};

extern HMSColorConvergence* TheHMSColorConvergenceParameter;

// ----------------------------------------------------------------------------

class HMSColorStrategy : public MetaInt32
{
public:
   HMSColorStrategy( MetaProcess* );

   IsoString Id() const override;
   double MinimumValue() const override;
   double MaximumValue() const override;
   double DefaultValue() const override;
};

extern HMSColorStrategy* TheHMSColorStrategyParameter;

// ----------------------------------------------------------------------------

class HMSColorGrip : public MetaDouble
{
public:
   HMSColorGrip( MetaProcess* );

   IsoString Id() const override;
   int Precision() const override;
   double MinimumValue() const override;
   double MaximumValue() const override;
   double DefaultValue() const override;
};

extern HMSColorGrip* TheHMSColorGripParameter;

// ----------------------------------------------------------------------------

class HMSShadowConvergence : public MetaDouble
{
public:
   HMSShadowConvergence( MetaProcess* );

   IsoString Id() const override;
   int Precision() const override;
   double MinimumValue() const override;
   double MaximumValue() const override;
   double DefaultValue() const override;
};

extern HMSShadowConvergence* TheHMSShadowConvergenceParameter;

// ----------------------------------------------------------------------------

class HMSLinearExpansion : public MetaDouble
{
public:
   HMSLinearExpansion( MetaProcess* );

   IsoString Id() const override;
   int Precision() const override;
   double MinimumValue() const override;
   double MaximumValue() const override;
   double DefaultValue() const override;
};

extern HMSLinearExpansion* TheHMSLinearExpansionParameter;

// ----------------------------------------------------------------------------

class HMSAdaptiveAnchor : public MetaBoolean
{
public:
   HMSAdaptiveAnchor( MetaProcess* );

   IsoString Id() const override;
   bool DefaultValue() const override;
};

extern HMSAdaptiveAnchor* TheHMSAdaptiveAnchorParameter;

// ----------------------------------------------------------------------------

PCL_END_LOCAL

} // pcl

#endif   // __HyperMetricStretchParameters_h

// ----------------------------------------------------------------------------
