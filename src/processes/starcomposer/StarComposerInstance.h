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

#ifndef __StarComposerInstance_h
#define __StarComposerInstance_h

#include <pcl/ProcessImplementation.h>
#include <pcl/MetaParameter.h>

#include "../../core/SensorProfiles.h"

namespace pcl
{

// ----------------------------------------------------------------------------

class StarComposerInstance : public ProcessImplementation
{
public:

   StarComposerInstance( const MetaProcess* );
   StarComposerInstance( const StarComposerInstance& );

   void Assign( const ProcessImplementation& ) override;
   UndoFlags UndoMode( const View& ) const override;
   bool CanExecuteOn( const View&, String& whyNot ) const override;
   bool ExecuteOn( View& ) override;
   void* LockParameter( const MetaParameter*, size_type tableRow ) override;
   bool AllocateParameter( size_type sizeOrLength, const MetaParameter* p, size_type tableRow ) override;
   size_type ParameterLength( const MetaParameter* p, size_type tableRow ) const override;

   /*!
    * \brief Generate a real-time preview of the StarComposer pipeline.
    *
    * Reads both the starmask and starless views, resamples to match
    * the preview image dimensions, runs the full pipeline, and writes
    * the composed result into \a img.
    *
    * \param[in,out] img  Preview image (replaced with composed result).
    *                     Input dimensions define the output resolution.
    * \return true on success, false on failure.
    */
   bool Preview( Image& img ) const;

   // Access to sensor profile
   const SensorProfile& GetSensorProfile() const
   {
      if ( sensorProfile >= 0 && size_type( sensorProfile ) < g_numSensorProfiles )
         return g_sensorProfiles[sensorProfile];
      return g_sensorProfiles[0];
   }

private:

   // Input images (View IDs)
   String starmaskView;
   String starlessView;
   
   // Composition
   pcl_enum compositionMode;        // 0=Screen, 1=LinearAdd
   
   // Sensor profile
   pcl_enum sensorProfile;          // Index into g_sensorProfiles
   
   // VeraLux Stretch
   double   logD;                   // Star intensity (1.0-21.0)
   double   profileHardness;        // b parameter (1.0-100.0)
   pcl_bool adaptiveAnchor;         // Use morphological anchor
   
   // Hybrid Physics
   double   colorGrip;              // Vector preservation (0.0-1.0)
   double   shadowConvergence;      // Shadow noise damping (0.0-3.0)
   
   // Star Surgery
   double   coreRejection;          // LSR intensity (0.0-1.0)
   double   morphReduction;         // Erosion intensity (0.0-1.0)
   double   opticalHealing;         // Healing strength (0.0-20.0)

   friend class StarComposerProcess;
   friend class StarComposerInterface;
};

// ----------------------------------------------------------------------------

} // pcl

#endif   // __StarComposerInstance_h

// ----------------------------------------------------------------------------
