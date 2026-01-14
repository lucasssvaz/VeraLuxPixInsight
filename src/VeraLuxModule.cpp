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

#define MODULE_VERSION_MAJOR     0
#define MODULE_VERSION_MINOR     1
#define MODULE_VERSION_REVISION  0
#define MODULE_VERSION_BUILD     0
#define MODULE_VERSION_LANGUAGE  eng

#define MODULE_RELEASE_YEAR      2026
#define MODULE_RELEASE_MONTH     1
#define MODULE_RELEASE_DAY       6

#include "VeraLuxModule.h"
#include "processes/hypermetric/HyperMetricStretchProcess.h"
#include "processes/hypermetric/HyperMetricStretchInterface.h"

#include <pcl/Console.h>

namespace pcl
{

// ----------------------------------------------------------------------------

VeraLuxModule::VeraLuxModule()
{
}

// ----------------------------------------------------------------------------

const char* VeraLuxModule::Version() const
{
   return PCL_MODULE_VERSION( MODULE_VERSION_MAJOR,
                              MODULE_VERSION_MINOR,
                              MODULE_VERSION_REVISION,
                              MODULE_VERSION_BUILD,
                              MODULE_VERSION_LANGUAGE );
}

// ----------------------------------------------------------------------------

IsoString VeraLuxModule::Name() const
{
   return "VeraLux";
}

// ----------------------------------------------------------------------------

String VeraLuxModule::Description() const
{
   return "VeraLux port for PixInsight - C++ port of the original Siril Python implementation by Riccardo Paterniti.\n"
          "Professional photometric image processing suite with scientifically accurate algorithms.\n"
          "Currently implemented processes:\n"
          "- HyperMetric Stretch: Precision linear-to-nonlinear stretching with sensor-specific quantum efficiency weighting.";
}

// ----------------------------------------------------------------------------

String VeraLuxModule::Author() const
{
   return "Lucas Saavedra Vaz (C++ Port for PixInsight)\n"
          "Riccardo Paterniti (Original Algorithm)";
}

// ----------------------------------------------------------------------------

String VeraLuxModule::Copyright() const
{
   return "Copyright (c) 2026 Lucas Saavedra Vaz (C++ Port for PixInsight)\n"
          "Copyright (c) 2025 Riccardo Paterniti (Original Algorithm)";
}

// ----------------------------------------------------------------------------

String VeraLuxModule::TradeMarks() const
{
   return "PixInsight";
}

// ----------------------------------------------------------------------------

String VeraLuxModule::OriginalFileName() const
{
#ifdef __PCL_FREEBSD
   return "VeraLuxPixInsight-pxm.so";
#endif
#ifdef __PCL_LINUX
   return "VeraLuxPixInsight-pxm.so";
#endif
#ifdef __PCL_MACOSX
   return "VeraLuxPixInsight-pxm.dylib";
#endif
#ifdef __PCL_WINDOWS
   return "VeraLuxPixInsight-pxm.dll";
#endif
}

// ----------------------------------------------------------------------------

void VeraLuxModule::GetReleaseDate( int& year, int& month, int& day ) const
{
   year  = MODULE_RELEASE_YEAR;
   month = MODULE_RELEASE_MONTH;
   day   = MODULE_RELEASE_DAY;
}

// ----------------------------------------------------------------------------

} // pcl

// ----------------------------------------------------------------------------

PCL_MODULE_EXPORT int InstallPixInsightModule( int mode )
{
   // NOTE: Console is not available during module installation.
   // All logging must happen later during actual process execution.
   
   try
   {
      new pcl::VeraLuxModule;

      if ( mode == pcl::InstallMode::FullInstall )
      {
         new pcl::HyperMetricStretchProcess;
         new pcl::HyperMetricStretchInterface;
      }

      return 0;
   }
   catch ( ... )
   {
      // Cannot use Console here - it's not initialized yet
      // PixInsight will report the installation failure
      return -1;
   }
}

// ----------------------------------------------------------------------------
