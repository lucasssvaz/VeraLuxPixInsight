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

#ifndef __HyperMetricStretchInterface_h
#define __HyperMetricStretchInterface_h

#include <pcl/CheckBox.h>
#include <pcl/ComboBox.h>
#include <pcl/Control.h>
#include <pcl/Label.h>
#include <pcl/NumericControl.h>
#include <pcl/ProcessInterface.h>
#include <pcl/PushButton.h>
#include <pcl/RadioButton.h>
#include <pcl/SectionBar.h>
#include <pcl/Sizer.h>
#include <pcl/SpinBox.h>

#include "HyperMetricStretchInstance.h"

namespace pcl
{

// ----------------------------------------------------------------------------

class HyperMetricStretchInterface : public ProcessInterface
{
public:

   HyperMetricStretchInterface();
   virtual ~HyperMetricStretchInterface();

   IsoString Id() const override;
   MetaProcess* Process() const override;
   String IconImageSVGFile() const override;
   InterfaceFeatures Features() const override;
   void ApplyInstance() const override;
   void ResetInstance() override;
   void RealTimePreviewUpdated( bool active ) override;
   bool Launch( const MetaProcess&, const ProcessImplementation*, bool& dynamic, unsigned& flags ) override;
   ProcessImplementation* NewProcess() const override;
   bool ValidateProcess( const ProcessImplementation&, String& whyNot ) const override;
   bool RequiresInstanceValidation() const override;
   bool ImportProcess( const ProcessImplementation& ) override;
   bool RequiresRealTimePreviewUpdate( const UInt16Image&, const View&, const Rect&, int zoomLevel ) const override;
   bool GenerateRealTimePreview( UInt16Image&, const View&, const Rect&, int zoomLevel, String& info ) const override;

private:

   HyperMetricStretchInstance m_instance;

   // GUI Data
   struct GUIData
   {
      GUIData( HyperMetricStretchInterface& );

      VerticalSizer     Global_Sizer;

         // Mode section
         SectionBar        Mode_SectionBar;
         Control           Mode_Control;
         HorizontalSizer   Mode_Sizer;
            RadioButton       ReadyToUse_RadioButton;
            RadioButton       Scientific_RadioButton;

         // Sensor section
         SectionBar        Sensor_SectionBar;
         Control           Sensor_Control;
         VerticalSizer     Sensor_Sizer;
            HorizontalSizer   SensorProfile_Sizer;
               Label             SensorProfile_Label;
               ComboBox          SensorProfile_ComboBox;
            HorizontalSizer   SensorProfile_Info_Sizer;
               Label             SensorProfile_Info;

         // Stretch parameters
         SectionBar        Stretch_SectionBar;
         Control           Stretch_Control;
         VerticalSizer     Stretch_Sizer;
            NumericControl    TargetBg_NumericControl;
            HorizontalSizer   AdaptiveAnchor_Sizer;
               CheckBox          AdaptiveAnchor_CheckBox;
            HorizontalSizer   LogD_Sizer;
               NumericControl    LogD_NumericControl;
               PushButton        AutoCalc_PushButton;
            NumericControl    ProtectB_NumericControl;
            NumericControl    ColorConvergence_NumericControl;

         // Ready-to-Use mode controls
         SectionBar        ReadyToUse_SectionBar;
         Control           ReadyToUse_Control;
         VerticalSizer     ReadyToUse_Sizer;
            NumericControl    ColorStrategy_NumericControl;
            HorizontalSizer   ColorStrategy_Info_Sizer;
               Label             ColorStrategy_Info;

         // Scientific mode controls
         SectionBar        Scientific_SectionBar;
         Control           Scientific_Control;
         VerticalSizer     Scientific_Sizer;
            NumericControl    LinearExpansion_NumericControl;
            NumericControl    ColorGrip_NumericControl;
            NumericControl    ShadowConvergence_NumericControl;
   };

   GUIData* GUI = nullptr;

   void UpdateControls();
   void UpdateModeControls();
   void UpdateSensorInfo();
   void UpdateColorStrategyInfo();
   void UpdateRealTimePreview();

   void e_Mode_Click( Button& sender, bool checked );
   void e_SensorProfile_Selected( ComboBox& sender, int itemIndex );
   void e_AdaptiveAnchor_Click( Button& sender, bool checked );
   void e_NumericControl_ValueUpdated( NumericEdit& sender, double value );
   void e_AutoCalc_Click( Button& sender, bool checked );

   friend struct GUIData;
};

// ----------------------------------------------------------------------------

PCL_BEGIN_LOCAL
extern HyperMetricStretchInterface* TheHyperMetricStretchInterface;
PCL_END_LOCAL

// ----------------------------------------------------------------------------

} // pcl

#endif   // __HyperMetricStretchInterface_h

// ----------------------------------------------------------------------------
