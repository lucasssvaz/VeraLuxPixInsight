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

#ifndef __StarComposerInterface_h
#define __StarComposerInterface_h

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
#include <pcl/ViewList.h>

#include "StarComposerInstance.h"

namespace pcl
{

// ----------------------------------------------------------------------------

class StarComposerInterface : public ProcessInterface
{
public:

   StarComposerInterface();
   virtual ~StarComposerInterface();

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

   StarComposerInstance m_instance;

   // GUI Data
   struct GUIData
   {
      GUIData( StarComposerInterface& );

      VerticalSizer     Global_Sizer;

         // Input section
         SectionBar        Input_SectionBar;
         Control           Input_Control;
         VerticalSizer     Input_Sizer;
            HorizontalSizer   Starmask_Sizer;
               Label             Starmask_Label;
               ViewList          Starmask_ViewList;
            HorizontalSizer   Starless_Sizer;
               Label             Starless_Label;
               ViewList          Starless_ViewList;
            HorizontalSizer   CompositionMode_Sizer;
               Label             CompositionMode_Label;
               RadioButton       Screen_RadioButton;
               RadioButton       LinearAdd_RadioButton;

         // Sensor section
         SectionBar        Sensor_SectionBar;
         Control           Sensor_Control;
         VerticalSizer     Sensor_Sizer;
            HorizontalSizer   SensorProfile_Sizer;
               Label             SensorProfile_Label;
               ComboBox          SensorProfile_ComboBox;

         // Stretch parameters
         SectionBar        Stretch_SectionBar;
         Control           Stretch_Control;
         VerticalSizer     Stretch_Sizer;
            NumericControl    LogD_NumericControl;
            NumericControl    ProfileHardness_NumericControl;
            HorizontalSizer   AdaptiveAnchor_Sizer;
               CheckBox          AdaptiveAnchor_CheckBox;

         // Hybrid Physics
         SectionBar        Physics_SectionBar;
         Control           Physics_Control;
         VerticalSizer     Physics_Sizer;
            NumericControl    ColorGrip_NumericControl;
            NumericControl    ShadowConvergence_NumericControl;

         // Star Surgery
         SectionBar        Surgery_SectionBar;
         Control           Surgery_Control;
         VerticalSizer     Surgery_Sizer;
            NumericControl    CoreRejection_NumericControl;
            NumericControl    MorphReduction_NumericControl;
            NumericControl    OpticalHealing_NumericControl;
   };

   GUIData* GUI = nullptr;

   void UpdateControls();
   void UpdateViewLists();
   void UpdateRealTimePreview();

   void e_ViewSelected( ViewList& sender, View& view );
   void e_CompositionMode_Click( Button& sender, bool checked );
   void e_SensorProfile_Selected( ComboBox& sender, int itemIndex );
   void e_NumericControl_ValueUpdated( NumericEdit& sender, double value );
   void e_AdaptiveAnchor_Click( Button& sender, bool checked );

   friend struct GUIData;
};

// ----------------------------------------------------------------------------

PCL_BEGIN_LOCAL
extern StarComposerInterface* TheStarComposerInterface;
PCL_END_LOCAL

// ----------------------------------------------------------------------------

} // pcl

#endif   // __StarComposerInterface_h

// ----------------------------------------------------------------------------
