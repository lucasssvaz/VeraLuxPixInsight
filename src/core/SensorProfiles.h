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

#ifndef __SensorProfiles_h
#define __SensorProfiles_h

#include <pcl/String.h>

namespace pcl
{

// ----------------------------------------------------------------------------

/*!
 * \struct SensorProfile
 * \brief Sensor quantum efficiency profile for photometric luminance extraction.
 *
 * Defines the RGB weighting coefficients for a specific camera sensor or
 * standard color space. These weights represent the quantum efficiency or
 * spectral response of the sensor and are used to compute photometrically
 * accurate luminance values.
 *
 * Database derived from SPCC (Spectrophotometric Color Calibration) data
 * and sensor specifications. Version 2.2 (2025).
 */
struct SensorProfile
{
   IsoString name;         //!< Profile name (user-facing)
   IsoString description;  //!< Technical description
   IsoString category;     //!< Category: "standard", "sensor-specific", "narrowband"
   double rWeight;      //!< Red channel weight (Quantum Efficiency)
   double gWeight;      //!< Green channel weight (Quantum Efficiency)
   double bWeight;      //!< Blue channel weight (Quantum Efficiency)

   /*!
    * Default constructor.
    */
   SensorProfile()
      : rWeight( 0.2126 ), gWeight( 0.7152 ), bWeight( 0.0722 )
   {
   }

   /*!
    * Constructor with all fields.
    */
   SensorProfile( const IsoString& n, const IsoString& d, const IsoString& c,
                  double r, double g, double b )
      : name( n ), description( d ), category( c )
      , rWeight( r ), gWeight( g ), bWeight( b )
   {
   }
};

// ----------------------------------------------------------------------------

/*!
 * \brief Global sensor profiles database (30+ profiles).
 * 
 * Includes standard color spaces (Rec.709), specific camera sensors
 * (Sony IMX, Canon, Nikon, Panasonic), smart telescopes (Seestar),
 * and narrowband profiles (HOO, SHO).
 */
extern const SensorProfile* g_sensorProfiles;

/*!
 * \brief Number of sensor profiles in the database.
 */
extern const size_t g_numSensorProfiles;

/*!
 * \brief Default sensor profile index (Rec.709).
 */
constexpr size_t g_defaultSensorProfileIndex = 0;

// ----------------------------------------------------------------------------

} // pcl

#endif   // __SensorProfiles_h

// ----------------------------------------------------------------------------
// EOF SensorProfiles.h - Released 2025-01-06T00:00:00Z
