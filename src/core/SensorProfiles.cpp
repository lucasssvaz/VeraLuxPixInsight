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

#include "SensorProfiles.h"

#include <array>
#include <utility>

namespace pcl
{

// ----------------------------------------------------------------------------

/*!
 * \brief Runtime sensor profiles - FULLY AUTOMATIC!
 *
 * This array is automatically generated from the constexpr g_sensorProfileData
 * array defined in SensorProfiles.h using template metaprogramming.
 *
 * ============================================================================
 * TO ADD A NEW SENSOR: ONLY edit g_sensorProfileData in SensorProfiles.h
 *                      This file requires NO modifications!
 * ============================================================================
 */

namespace
{
   // Helper to create a std::array of SensorProfile from constexpr data
   template<size_t... I>
   auto MakeRuntimeProfiles( std::index_sequence<I...> )
      -> std::array<SensorProfile, sizeof...(I)>
   {
      return {{ SensorProfile( g_sensorProfileData[I] )... }};
   }

   // Create the runtime array automatically from the constexpr data
   // std::make_index_sequence generates 0, 1, 2, ... g_numSensorProfiles-1
   const auto s_runtimeProfiles = MakeRuntimeProfiles( 
      std::make_index_sequence<g_numSensorProfiles>{} 
   );
}

// Export pointer to the automatically-generated array
const SensorProfile* g_sensorProfiles = s_runtimeProfiles.data();

// ----------------------------------------------------------------------------

} // pcl

// ----------------------------------------------------------------------------
