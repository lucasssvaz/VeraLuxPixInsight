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

#ifndef __HyperMetricStretchInstance_h
#define __HyperMetricStretchInstance_h

#include <pcl/ProcessImplementation.h>
#include <pcl/MetaParameter.h>

#include "../../core/SensorProfiles.h"
#include "../../core/VeraLuxEngine.h"

namespace pcl
{

// ----------------------------------------------------------------------------

class HyperMetricStretchInstance : public ProcessImplementation
{
public:

   HyperMetricStretchInstance( const MetaProcess* );
   HyperMetricStretchInstance( const HyperMetricStretchInstance& );

   void Assign( const ProcessImplementation& ) override;
   UndoFlags UndoMode( const View& ) const override;
   bool CanExecuteOn( const View&, String& whyNot ) const override;
   bool ExecuteOn( View& ) override;
   void* LockParameter( const MetaParameter*, size_type tableRow ) override;
   bool AllocateParameter( size_type sizeOrLength, const MetaParameter* p, size_type tableRow ) override;
   size_type ParameterLength( const MetaParameter* p, size_type tableRow ) const override;

   // Helper for real-time preview
   bool Preview( Image& ) const;

   // Access to sensor profile
   const SensorProfile& GetSensorProfile() const
   {
      // Map parameter index to global sensor profile array
      if ( sensorProfile >= 0 && size_type( sensorProfile ) < g_numSensorProfiles )
         return g_sensorProfiles[sensorProfile];
      // Default fallback
      return g_sensorProfiles[0];
   }

   // Calculate effective parameters based on mode
   void GetEffectiveParams( double& grip, double& shadow, double& linearExp ) const;

private:

   // Parameters
   pcl_enum processingMode;        // 0=ReadyToUse, 1=Scientific
   pcl_enum sensorProfile;         // Index into g_sensorProfiles
   double   targetBackground;      // Target median value
   double   logD;                  // Stretch intensity (log10)
   double   protectB;              // Highlight protection
   double   colorConvergence;      // Star white point power
   int32    colorStrategy;         // Unified control (-100 to +100, Ready-to-Use only)
   double   colorGrip;             // Vector preservation (0-1, Scientific only)
   double   shadowConvergence;     // Shadow noise damping (0-3, Scientific only)
   double   linearExpansion;       // Range normalization (0-1, Scientific only)
   pcl_bool adaptiveAnchor;        // Use morphological anchor

   friend class HyperMetricStretchProcess;
   friend class HyperMetricStretchInterface;
};

// ----------------------------------------------------------------------------

} // pcl

#endif   // __HyperMetricStretchInstance_h

// ----------------------------------------------------------------------------
