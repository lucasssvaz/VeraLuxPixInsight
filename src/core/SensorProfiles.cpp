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

namespace pcl
{

// ----------------------------------------------------------------------------

/*!
 * \brief Sensor Profiles Database v2.2 (Siril SPCC Derived)
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
 *
 * Total: 27 profiles
 */
static const SensorProfile s_sensorProfilesData[] =
{
   // --- STANDARD ---
   SensorProfile(
      IsoString("Rec.709 (Recommended)"),
      IsoString("ITU-R BT.709 standard for sRGB/HDTV"),
      IsoString("standard"),
      0.2126, 0.7152, 0.0722
   ),

   // --- SONY MODERN BSI (APS-C / Full Frame) ---
   SensorProfile(
      IsoString("Sony IMX571 (ASI2600/QHY268)"),
      IsoString("Sony IMX571 26MP APS-C BSI (STARVIS)"),
      IsoString("sensor-specific"),
      0.2944, 0.5021, 0.2035
   ),
   SensorProfile(
      IsoString("Sony IMX455 (ASI6200/QHY600)"),
      IsoString("Sony IMX455 61MP Full Frame BSI"),
      IsoString("sensor-specific"),
      0.2987, 0.5001, 0.2013
   ),
   SensorProfile(
      IsoString("Sony IMX410 (ASI2400)"),
      IsoString("Sony IMX410 24MP Full Frame (Large Pixels)"),
      IsoString("sensor-specific"),
      0.3015, 0.5050, 0.1935
   ),
   SensorProfile(
      IsoString("Sony IMX269 (Altair/ToupTek)"),
      IsoString("Sony IMX269 20MP 4/3\" BSI"),
      IsoString("sensor-specific"),
      0.3040, 0.5010, 0.1950
   ),
   SensorProfile(
      IsoString("Sony IMX294 (ASI294)"),
      IsoString("Sony IMX294 11.7MP 4/3\" BSI"),
      IsoString("sensor-specific"),
      0.3068, 0.5008, 0.1925
   ),

   // --- SONY MEDIUM FORMAT / SQUARE ---
   SensorProfile(
      IsoString("Sony IMX533 (ASI533)"),
      IsoString("Sony IMX533 9MP 1\" Square BSI"),
      IsoString("sensor-specific"),
      0.2910, 0.5072, 0.2018
   ),
   SensorProfile(
      IsoString("Sony IMX676 (ASI676)"),
      IsoString("Sony IMX676 12MP Square BSI (Starvis 2)"),
      IsoString("sensor-specific"),
      0.2880, 0.5100, 0.2020
   ),

   // --- SONY PLANETARY / GUIDING (High Sensitivity) ---
   SensorProfile(
      IsoString("Sony IMX585 (ASI585)"),
      IsoString("Sony IMX585 8.3MP 1/1.2\" BSI (STARVIS 2)"),
      IsoString("sensor-specific"),
      0.3431, 0.4822, 0.1747
   ),
   SensorProfile(
      IsoString("Sony IMX662 (ASI662)"),
      IsoString("Sony IMX662 2.1MP 1/2.8\" BSI (STARVIS 2)"),
      IsoString("sensor-specific"),
      0.3430, 0.4821, 0.1749
   ),
   SensorProfile(
      IsoString("Sony IMX678 (ASI678)"),
      IsoString("Sony IMX678 8MP BSI (STARVIS 2)"),
      IsoString("sensor-specific"),
      0.3426, 0.4825, 0.1750
   ),
   SensorProfile(
      IsoString("Sony IMX462 (ASI462)"),
      IsoString("Sony IMX462 2MP 1/2.8\" (High NIR)"),
      IsoString("sensor-specific"),
      0.3333, 0.4866, 0.1801
   ),
   SensorProfile(
      IsoString("Sony IMX715 (ASI715)"),
      IsoString("Sony IMX715 8MP (Starvis 2)"),
      IsoString("sensor-specific"),
      0.3410, 0.4840, 0.1750
   ),
   SensorProfile(
      IsoString("Sony IMX482 (ASI482)"),
      IsoString("Sony IMX482 2MP (Large Pixels)"),
      IsoString("sensor-specific"),
      0.3150, 0.4950, 0.1900
   ),
   SensorProfile(
      IsoString("Sony IMX183 (ASI183)"),
      IsoString("Sony IMX183 20MP 1\" BSI"),
      IsoString("sensor-specific"),
      0.2967, 0.4983, 0.2050
   ),
   SensorProfile(
      IsoString("Sony IMX178 (ASI178)"),
      IsoString("Sony IMX178 6.4MP 1/1.8\" BSI"),
      IsoString("sensor-specific"),
      0.2346, 0.5206, 0.2448
   ),
   SensorProfile(
      IsoString("Sony IMX224 (ASI224)"),
      IsoString("Sony IMX224 1.27MP 1/3\" BSI"),
      IsoString("sensor-specific"),
      0.3402, 0.4765, 0.1833
   ),

   // --- CANON DSLR ---
   SensorProfile(
      IsoString("Canon EOS (Modern)"),
      IsoString("Canon CMOS (Digic 4/5 Era)"),
      IsoString("sensor-specific"),
      0.2600, 0.5200, 0.2200
   ),
   SensorProfile(
      IsoString("Canon EOS (Legacy)"),
      IsoString("Canon CMOS (Legacy Digic 2/3)"),
      IsoString("sensor-specific"),
      0.2450, 0.5350, 0.2200
   ),

   // --- NIKON DSLR ---
   SensorProfile(
      IsoString("Nikon DSLR (Modern)"),
      IsoString("Nikon DX/FX CMOS (Modern)"),
      IsoString("sensor-specific"),
      0.2650, 0.5100, 0.2250
   ),
   SensorProfile(
      IsoString("Nikon DSLR (Legacy)"),
      IsoString("Nikon CMOS (Legacy)"),
      IsoString("sensor-specific"),
      0.2500, 0.5300, 0.2200
   ),

   // --- FUJI / OTHERS ---
   SensorProfile(
      IsoString("Fujifilm X-Trans 5 HR"),
      IsoString("Fujifilm X-Trans 5 (40MP)"),
      IsoString("sensor-specific"),
      0.2800, 0.5100, 0.2100
   ),
   SensorProfile(
      IsoString("Panasonic MN34230 (ASI1600)"),
      IsoString("Panasonic MN34230 4/3\" CMOS"),
      IsoString("sensor-specific"),
      0.2650, 0.5250, 0.2100
   ),

   // --- SMART TELESCOPES ---
   SensorProfile(
      IsoString("ZWO Seestar S50"),
      IsoString("ZWO Seestar S50 (IMX462)"),
      IsoString("sensor-specific"),
      0.3333, 0.4866, 0.1801
   ),
   SensorProfile(
      IsoString("ZWO Seestar S30"),
      IsoString("ZWO Seestar S30"),
      IsoString("sensor-specific"),
      0.2928, 0.5053, 0.2019
   ),

   // --- NARROWBAND ---
   SensorProfile(
      IsoString("Narrowband HOO"),
      IsoString("Bicolor palette: Ha=Red, OIII=Green+Blue"),
      IsoString("narrowband"),
      0.5000, 0.2500, 0.2500
   ),
   SensorProfile(
      IsoString("Narrowband SHO"),
      IsoString("Hubble palette: SII=Red, Ha=Green, OIII=Blue"),
      IsoString("narrowband"),
      0.3333, 0.3400, 0.3267
   )
};

const SensorProfile* g_sensorProfiles = s_sensorProfilesData;
const size_t g_numSensorProfiles = sizeof( s_sensorProfilesData ) / sizeof( SensorProfile );

// ----------------------------------------------------------------------------

} // pcl

// ----------------------------------------------------------------------------
// EOF SensorProfiles.cpp - Released 2025-01-06T00:00:00Z
