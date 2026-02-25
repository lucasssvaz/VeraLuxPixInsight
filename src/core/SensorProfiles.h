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
 * \struct SensorProfileData
 * \brief Compile-time sensor profile data (constexpr-friendly).
 *
 * This struct uses const char* instead of IsoString to allow constexpr
 * initialization, which is required to avoid static initialization order
 * issues during module installation.
 */
struct SensorProfileData
{
   const char* name;        //!< User-facing name
   const char* description; //!< Technical description
   const char* category;    //!< Category: "standard", "sensor-specific", "narrowband"
   double rWeight;          //!< Red channel weight (Quantum Efficiency)
   double gWeight;          //!< Green channel weight (Quantum Efficiency)
   double bWeight;          //!< Blue channel weight (Quantum Efficiency)
};

/*!
 * \brief Master Sensor Profiles Database v2.2 (Siril SPCC Derived)
 *
 * This constexpr array is the single source of data for all sensor profiles.
 * To add a new sensor, simply add a new entry here - no other files need updating!
 *
 * Includes:
 * - Standard Color Spaces (Rec.709)
 * - Sony Modern BSI (IMX571, IMX455, IMX410, IMX269, IMX294)
 * - Sony Medium Format (IMX533, IMX676)
 * - Sony Planetary/Guiding (IMX585, IMX662, IMX678, IMX462, IMX715, IMX482, IMX183, IMX178, IMX224)
 * - Canon DSLR (Modern, Legacy)
 * - Nikon DSLR (Modern, Legacy)
 * - Fujifilm X-Trans
 * - Panasonic MN34230
 * - Smart Telescopes (Seestar S50, S30)
 * - Narrowband (HOO, SHO)
 */
inline constexpr SensorProfileData g_sensorProfileData[] =
{
   // --- STANDARD ---
   {
      "Rec.709 (Recommended)",                    // Name
      "ITU-R BT.709 standard for sRGB/HDTV",      // Description
      "standard",                                 // Category
      0.2126, 0.7152, 0.0722                      // R, G, B weights
   },

   // --- SONY MODERN BSI (APS-C / Full Frame) ---
   {
      "Sony IMX571 (ASI2600/QHY268)",             // Name
      "Sony IMX571 26MP APS-C BSI (STARVIS)",     // Description
      "sensor-specific",                          // Category
      0.2944, 0.5021, 0.2035                      // R, G, B weights
   },
   {
      "Sony IMX455 (ASI6200/QHY600)",             // Name
      "Sony IMX455 61MP Full Frame BSI",          // Description
      "sensor-specific",                          // Category
      0.2987, 0.5001, 0.2013                      // R, G, B weights
   },
   {
      "Sony IMX410 (ASI2400)",                    // Name
      "Sony IMX410 24MP Full Frame (Large Pixels)", // Description
      "sensor-specific",                          // Category
      0.3015, 0.5050, 0.1935                      // R, G, B weights
   },
   {
      "Sony IMX269 (Altair/ToupTek)",             // Name
      "Sony IMX269 20MP 4/3\" BSI",               // Description
      "sensor-specific",                          // Category
      0.3040, 0.5010, 0.1950                      // R, G, B weights
   },
   {
      "Sony IMX294 (ASI294)",                     // Name
      "Sony IMX294 11.7MP 4/3\" BSI",             // Description
      "sensor-specific",                          // Category
      0.3068, 0.5008, 0.1925                      // R, G, B weights
   },

   // --- SONY MEDIUM FORMAT / SQUARE ---
   {
      "Sony IMX533 (ASI533)",                     // Name
      "Sony IMX533 9MP 1\" Square BSI",           // Description
      "sensor-specific",                          // Category
      0.2910, 0.5072, 0.2018                      // R, G, B weights
   },
   {
      "Sony IMX676 (ASI676)",                     // Name
      "Sony IMX676 12MP Square BSI (Starvis 2)",  // Description
      "sensor-specific",                          // Category
      0.2880, 0.5100, 0.2020                      // R, G, B weights
   },

   // --- SONY PLANETARY / GUIDING (High Sensitivity) ---
   {
      "Sony IMX585 (ASI585)",                     // Name
      "Sony IMX585 8.3MP 1/1.2\" BSI (STARVIS 2)", // Description
      "sensor-specific",                          // Category
      0.3431, 0.4822, 0.1747                      // R, G, B weights
   },
   {
      "Sony IMX662 (ASI662)",                     // Name
      "Sony IMX662 2.1MP 1/2.8\" BSI (STARVIS 2)", // Description
      "sensor-specific",                          // Category
      0.3430, 0.4821, 0.1749                      // R, G, B weights
   },
   {
      "Sony IMX678 (ASI678)",                     // Name
      "Sony IMX678 8MP BSI (STARVIS 2)",          // Description
      "sensor-specific",                          // Category
      0.3426, 0.4825, 0.1750                      // R, G, B weights
   },
   {
      "Sony IMX462 (ASI462)",                     // Name
      "Sony IMX462 2MP 1/2.8\" (High NIR)",       // Description
      "sensor-specific",                          // Category
      0.3333, 0.4866, 0.1801                      // R, G, B weights
   },
   {
      "Sony IMX715 (ASI715)",                     // Name
      "Sony IMX715 8MP (Starvis 2)",              // Description
      "sensor-specific",                          // Category
      0.3410, 0.4840, 0.1750                      // R, G, B weights
   },
   {
      "Sony IMX482 (ASI482)",                     // Name
      "Sony IMX482 2MP (Large Pixels)",           // Description
      "sensor-specific",                          // Category
      0.3150, 0.4950, 0.1900                      // R, G, B weights
   },
   {
      "Sony IMX183 (ASI183)",                     // Name
      "Sony IMX183 20MP 1\" BSI",                 // Description
      "sensor-specific",                          // Category
      0.2967, 0.4983, 0.2050                      // R, G, B weights
   },
   {
      "Sony IMX178 (ASI178)",                     // Name
      "Sony IMX178 6.4MP 1/1.8\" BSI",            // Description
      "sensor-specific",                          // Category
      0.2346, 0.5206, 0.2448                      // R, G, B weights
   },
   {
      "Sony IMX224 (ASI224)",                     // Name
      "Sony IMX224 1.27MP 1/3\" BSI",             // Description
      "sensor-specific",                          // Category
      0.3402, 0.4765, 0.1833                      // R, G, B weights
   },

   // --- CANON DSLR ---
   {
      "Canon EOS (Modern)",                       // Name
      "Canon CMOS (Digic 4/5 Era)",               // Description
      "sensor-specific",                          // Category
      0.2600, 0.5200, 0.2200                      // R, G, B weights
   },
   {
      "Canon EOS (Legacy)",                       // Name
      "Canon CMOS (Legacy Digic 2/3)",            // Description
      "sensor-specific",                          // Category
      0.2450, 0.5350, 0.2200                      // R, G, B weights
   },

   // --- NIKON DSLR ---
   {
      "Nikon DSLR (Modern)",                      // Name
      "Nikon DX/FX CMOS (Modern)",                // Description
      "sensor-specific",                          // Category
      0.2650, 0.5100, 0.2250                      // R, G, B weights
   },
   {
      "Nikon DSLR (Legacy)",                      // Name
      "Nikon CMOS (Legacy)",                      // Description
      "sensor-specific",                          // Category
      0.2500, 0.5300, 0.2200                      // R, G, B weights
   },

   // --- FUJI / OTHERS ---
   {
      "Fujifilm X-Trans 5 HR",                    // Name
      "Fujifilm X-Trans 5 (40MP)",                // Description
      "sensor-specific",                          // Category
      0.2800, 0.5100, 0.2100                      // R, G, B weights
   },
   {
      "Panasonic MN34230 (ASI1600)",              // Name
      "Panasonic MN34230 4/3\" CMOS",             // Description
      "sensor-specific",                          // Category
      0.2650, 0.5250, 0.2100                      // R, G, B weights
   },

   // --- SMART TELESCOPES ---
   {
      "ZWO Seestar S50",                          // Name
      "ZWO Seestar S50 (IMX462)",                 // Description
      "sensor-specific",                          // Category
      0.3333, 0.4866, 0.1801                      // R, G, B weights
   },
   {
      "ZWO Seestar S30",                          // Name
      "ZWO Seestar S30",                          // Description
      "sensor-specific",                          // Category
      0.2928, 0.5053, 0.2019                      // R, G, B weights
   },

   // --- NARROWBAND ---
   {
      "Narrowband HOO",                           // Name
      "Bicolor palette: Ha=Red, OIII=Green+Blue", // Description
      "narrowband",                               // Category
      0.5000, 0.2500, 0.2500                      // R, G, B weights
   },
   {
      "Narrowband SHO",                                // Name
      "Hubble palette: SII=Red, Ha=Green, OIII=Blue",  // Description
      "narrowband",                                    // Category
      0.3333, 0.3400, 0.3267                           // R, G, B weights
   },
};

/*!
 * \brief Number of sensor profiles (computed at compile time).
 */
inline constexpr size_t g_numSensorProfiles = sizeof( g_sensorProfileData ) / sizeof( g_sensorProfileData[0] );

/*!
 * \brief Default sensor profile index (Rec.709).
 */
inline constexpr size_t g_defaultSensorProfileIndex = 0;

// ----------------------------------------------------------------------------

/*!
 * \struct SensorProfile
 * \brief Runtime sensor profile with IsoString members.
 *
 * This struct is used at runtime where IsoString is needed.
 * It is initialized from the constexpr SensorProfileData.
 */
struct SensorProfile
{
   IsoString name;         //!< Profile name (user-facing)
   IsoString description;  //!< Technical description
   IsoString category;     //!< Category: "standard", "sensor-specific", "narrowband"
   double rWeight;         //!< Red channel weight (Quantum Efficiency)
   double gWeight;         //!< Green channel weight (Quantum Efficiency)
   double bWeight;         //!< Blue channel weight (Quantum Efficiency)

   /*!
    * Default constructor.
    */
   SensorProfile()
      : rWeight( 0.2126 ), gWeight( 0.7152 ), bWeight( 0.0722 )
   {
   }

   /*!
    * Constructor from constexpr SensorProfileData.
    */
   SensorProfile( const SensorProfileData& data )
      : name( data.name )
      , description( data.description )
      , category( data.category )
      , rWeight( data.rWeight )
      , gWeight( data.gWeight )
      , bWeight( data.bWeight )
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
 * \brief Runtime sensor profiles pointer (extern, initialized in SensorProfiles.cpp).
 *
 * Points to an array containing the same data as g_sensorProfileData but with
 * IsoString members for runtime use. Use like an array: g_sensorProfiles[i]
 *
 * NOTE: This is a pointer, not an array, to enable fully automatic initialization
 * from the constexpr data without any manual index listing.
 */
extern const SensorProfile* g_sensorProfiles;

// ----------------------------------------------------------------------------

} // pcl

#endif   // __SensorProfiles_h

// ----------------------------------------------------------------------------
