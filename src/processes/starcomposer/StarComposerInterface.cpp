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

#include "StarComposerInterface.h"
#include "StarComposerProcess.h"
#include "StarComposerParameters.h"

#include "../../core/SensorProfiles.h"

#include <pcl/ErrorHandler.h>
#include <pcl/MessageBox.h>
#include <pcl/RealTimePreview.h>

namespace pcl
{

// ----------------------------------------------------------------------------

StarComposerInterface* TheStarComposerInterface = nullptr;

// ----------------------------------------------------------------------------

StarComposerInterface::StarComposerInterface()
   : m_instance( TheStarComposerProcess )
{
   TheStarComposerInterface = this;
}

// ----------------------------------------------------------------------------

StarComposerInterface::~StarComposerInterface()
{
   if ( GUI != nullptr )
      delete GUI, GUI = nullptr;
}

// ----------------------------------------------------------------------------

IsoString StarComposerInterface::Id() const
{
   return "StarComposer";
}

// ----------------------------------------------------------------------------

MetaProcess* StarComposerInterface::Process() const
{
   return TheStarComposerProcess;
}

// ----------------------------------------------------------------------------

String StarComposerInterface::IconImageSVGFile() const
{
   return "@module_icons_dir/StarComposer.svg";
}

// ----------------------------------------------------------------------------

InterfaceFeatures StarComposerInterface::Features() const
{
   // View-based process with real-time preview (same pattern as HyperMetricStretch)
   // The target view is ignored - we use the starmask/starless views from the instance
   return InterfaceFeature::Default | InterfaceFeature::RealTimeButton;
}

// ----------------------------------------------------------------------------

void StarComposerInterface::ApplyInstance() const
{
   m_instance.LaunchOnCurrentView();
}

// ----------------------------------------------------------------------------

void StarComposerInterface::ResetInstance()
{
   StarComposerInstance defaultInstance( TheStarComposerProcess );
   ImportProcess( defaultInstance );
}

// ----------------------------------------------------------------------------

void StarComposerInterface::RealTimePreviewUpdated( bool active )
{
   if ( GUI != nullptr )
   {
      if ( active )
      {
         if ( m_instance.starmaskView.IsEmpty() || m_instance.starlessView.IsEmpty() )
         {
            pcl::MessageBox( "Please select both a starmask and a starless image before activating the real-time preview.",
                             "StarComposer Real-Time Preview",
                             StdIcon::Error, StdButton::Ok ).Execute();
            RealTimePreview::SetOwner( ProcessInterface::Null() );
            return;
         }
         RealTimePreview::SetOwner( *this );
      }
      else
         RealTimePreview::SetOwner( ProcessInterface::Null() );
   }
}

// ----------------------------------------------------------------------------

bool StarComposerInterface::Launch( const MetaProcess& P, const ProcessImplementation* pInstance, bool& dynamic, unsigned& /*flags*/ )
{
   if ( GUI == nullptr )
   {
      GUI = new GUIData( *this );
      SetWindowTitle( "VeraLux StarComposer" );
      UpdateControls();
   }

   dynamic = false;
   return &P == TheStarComposerProcess;
}

// ----------------------------------------------------------------------------

ProcessImplementation* StarComposerInterface::NewProcess() const
{
   return new StarComposerInstance( m_instance );
}

// ----------------------------------------------------------------------------

bool StarComposerInterface::ValidateProcess( const ProcessImplementation& p, String& whyNot ) const
{
   if ( dynamic_cast<const StarComposerInstance*>( &p ) != nullptr )
      return true;

   whyNot = "Not a StarComposer instance.";
   return false;
}

// ----------------------------------------------------------------------------

bool StarComposerInterface::RequiresInstanceValidation() const
{
   return true;
}

// ----------------------------------------------------------------------------

bool StarComposerInterface::ImportProcess( const ProcessImplementation& p )
{
   m_instance.Assign( p );
   UpdateControls();
   UpdateRealTimePreview();
   return true;
}

// ----------------------------------------------------------------------------

bool StarComposerInterface::RequiresRealTimePreviewUpdate( const UInt16Image&, const View&,
                                                            const Rect&, int ) const
{
   return true;
}

// ----------------------------------------------------------------------------

bool StarComposerInterface::GenerateRealTimePreview( UInt16Image& image, const View&,
                                                      const Rect& rect, int zoomLevel, String& info ) const
{
   // Convert incoming image to working format (we'll replace its contents)
   Image work;
   work.Assign( image );

   // Run the full StarComposer pipeline at the preview resolution
   if ( !m_instance.Preview( work ) )
      return false;

   // Convert back to UInt16
   image.Assign( work );

   // Build info string
   info = String().Format( "Log D: %.2f | Grip: %.2f | Comp: %s",
      m_instance.logD,
      m_instance.colorGrip,
      (m_instance.compositionMode == SCSCompositionMode::Screen) ? "Screen" : "Linear" );

   return true;
}

// ----------------------------------------------------------------------------

void StarComposerInterface::UpdateControls()
{
   if ( GUI == nullptr )
      return;

   UpdateViewLists();

   // Composition mode
   GUI->Screen_RadioButton.SetChecked( m_instance.compositionMode == SCSCompositionMode::Screen );
   GUI->LinearAdd_RadioButton.SetChecked( m_instance.compositionMode == SCSCompositionMode::LinearAdd );

   // Sensor profile
   GUI->SensorProfile_ComboBox.SetCurrentItem( m_instance.sensorProfile );

   // Stretch parameters
   GUI->LogD_NumericControl.SetValue( m_instance.logD );
   GUI->ProfileHardness_NumericControl.SetValue( m_instance.profileHardness );
   GUI->AdaptiveAnchor_CheckBox.SetChecked( m_instance.adaptiveAnchor );

   // Physics
   GUI->ColorGrip_NumericControl.SetValue( m_instance.colorGrip );
   GUI->ShadowConvergence_NumericControl.SetValue( m_instance.shadowConvergence );

   // Surgery
   GUI->CoreRejection_NumericControl.SetValue( m_instance.coreRejection );
   GUI->MorphReduction_NumericControl.SetValue( m_instance.morphReduction );
   GUI->OpticalHealing_NumericControl.SetValue( m_instance.opticalHealing );
}

// ----------------------------------------------------------------------------

void StarComposerInterface::UpdateViewLists()
{
   GUI->Starmask_ViewList.Regenerate( true, false );
   GUI->Starless_ViewList.Regenerate( true, false );

   // Select current views if they exist
   if ( !m_instance.starmaskView.IsEmpty() )
   {
      View v = View::ViewById( m_instance.starmaskView );
      if ( !v.IsNull() )
         GUI->Starmask_ViewList.SelectView( v );
   }

   if ( !m_instance.starlessView.IsEmpty() )
   {
      View v = View::ViewById( m_instance.starlessView );
      if ( !v.IsNull() )
         GUI->Starless_ViewList.SelectView( v );
   }
}

// ----------------------------------------------------------------------------

void StarComposerInterface::UpdateRealTimePreview()
{
   if ( IsRealTimePreviewActive() )
      RealTimePreview::Update();
}

// ----------------------------------------------------------------------------
// Event Handlers
// ----------------------------------------------------------------------------

void StarComposerInterface::e_ViewSelected( ViewList& sender, View& view )
{
   if ( sender == GUI->Starmask_ViewList )
      m_instance.starmaskView = view.IsNull() ? String() : view.FullId();
   else if ( sender == GUI->Starless_ViewList )
      m_instance.starlessView = view.IsNull() ? String() : view.FullId();

   UpdateRealTimePreview();
}

void StarComposerInterface::e_CompositionMode_Click( Button& sender, bool checked )
{
   if ( sender == GUI->Screen_RadioButton )
      m_instance.compositionMode = SCSCompositionMode::Screen;
   else if ( sender == GUI->LinearAdd_RadioButton )
      m_instance.compositionMode = SCSCompositionMode::LinearAdd;

   UpdateRealTimePreview();
}

void StarComposerInterface::e_SensorProfile_Selected( ComboBox& sender, int itemIndex )
{
   m_instance.sensorProfile = itemIndex;
   UpdateRealTimePreview();
}

void StarComposerInterface::e_NumericControl_ValueUpdated( NumericEdit& sender, double value )
{
   if ( sender == GUI->LogD_NumericControl )
      m_instance.logD = value;
   else if ( sender == GUI->ProfileHardness_NumericControl )
      m_instance.profileHardness = value;
   else if ( sender == GUI->ColorGrip_NumericControl )
      m_instance.colorGrip = value;
   else if ( sender == GUI->ShadowConvergence_NumericControl )
      m_instance.shadowConvergence = value;
   else if ( sender == GUI->CoreRejection_NumericControl )
      m_instance.coreRejection = value;
   else if ( sender == GUI->MorphReduction_NumericControl )
      m_instance.morphReduction = value;
   else if ( sender == GUI->OpticalHealing_NumericControl )
      m_instance.opticalHealing = value;

   UpdateRealTimePreview();
}

void StarComposerInterface::e_AdaptiveAnchor_Click( Button& sender, bool checked )
{
   m_instance.adaptiveAnchor = checked;
   UpdateRealTimePreview();
}

// ----------------------------------------------------------------------------
// GUI Construction
// ----------------------------------------------------------------------------

StarComposerInterface::GUIData::GUIData( StarComposerInterface& w )
{
   pcl::Font fnt = w.Font();
   int labelWidth1 = fnt.Width( String( "Morph. Reduction:" ) + 'M' );
   int ui4 = w.LogicalPixelsToPhysical( 4 );

   //

   Input_SectionBar.SetTitle( "Input Images" );
   Input_SectionBar.SetSection( Input_Control );

   Starmask_Label.SetText( "Linear Starmask:" );
   Starmask_Label.SetFixedWidth( labelWidth1 );
   Starmask_Label.SetTextAlignment( TextAlign::Right|TextAlign::VertCenter );

   Starmask_ViewList.OnViewSelected( (ViewList::view_event_handler)&StarComposerInterface::e_ViewSelected, w );

   Starmask_Sizer.SetSpacing( ui4 );
   Starmask_Sizer.Add( Starmask_Label );
   Starmask_Sizer.Add( Starmask_ViewList, 100 );

   Starless_Label.SetText( "Stretched Starless:" );
   Starless_Label.SetFixedWidth( labelWidth1 );
   Starless_Label.SetTextAlignment( TextAlign::Right|TextAlign::VertCenter );

   Starless_ViewList.OnViewSelected( (ViewList::view_event_handler)&StarComposerInterface::e_ViewSelected, w );

   Starless_Sizer.SetSpacing( ui4 );
   Starless_Sizer.Add( Starless_Label );
   Starless_Sizer.Add( Starless_ViewList, 100 );

   CompositionMode_Label.SetText( "Composition:" );
   CompositionMode_Label.SetFixedWidth( labelWidth1 );
   CompositionMode_Label.SetTextAlignment( TextAlign::Right|TextAlign::VertCenter );

   Screen_RadioButton.SetText( "Screen (Safe)" );
   Screen_RadioButton.OnClick( (Button::click_event_handler)&StarComposerInterface::e_CompositionMode_Click, w );

   LinearAdd_RadioButton.SetText( "Linear Add (Physical)" );
   LinearAdd_RadioButton.OnClick( (Button::click_event_handler)&StarComposerInterface::e_CompositionMode_Click, w );

   CompositionMode_Sizer.SetSpacing( ui4 );
   CompositionMode_Sizer.Add( CompositionMode_Label );
   CompositionMode_Sizer.Add( Screen_RadioButton );
   CompositionMode_Sizer.Add( LinearAdd_RadioButton );
   CompositionMode_Sizer.AddStretch();

   Input_Sizer.SetMargin( 6 );
   Input_Sizer.SetSpacing( ui4 );
   Input_Sizer.Add( Starmask_Sizer );
   Input_Sizer.Add( Starless_Sizer );
   Input_Sizer.Add( CompositionMode_Sizer );

   Input_Control.SetSizer( Input_Sizer );

   //

   Sensor_SectionBar.SetTitle( "Sensor Calibration" );
   Sensor_SectionBar.SetSection( Sensor_Control );

   SensorProfile_Label.SetText( "Sensor Profile:" );
   SensorProfile_Label.SetFixedWidth( labelWidth1 );
   SensorProfile_Label.SetTextAlignment( TextAlign::Right|TextAlign::VertCenter );

   for ( size_t i = 0; i < g_numSensorProfiles; ++i )
      SensorProfile_ComboBox.AddItem( g_sensorProfiles[i].name );
   SensorProfile_ComboBox.OnItemSelected( (ComboBox::item_event_handler)&StarComposerInterface::e_SensorProfile_Selected, w );

   SensorProfile_Sizer.SetSpacing( ui4 );
   SensorProfile_Sizer.Add( SensorProfile_Label );
   SensorProfile_Sizer.Add( SensorProfile_ComboBox, 100 );

   Sensor_Sizer.SetMargin( 6 );
   Sensor_Sizer.SetSpacing( ui4 );
   Sensor_Sizer.Add( SensorProfile_Sizer );

   Sensor_Control.SetSizer( Sensor_Sizer );

   //

   Stretch_SectionBar.SetTitle( "VeraLux Stretch" );
   Stretch_SectionBar.SetSection( Stretch_Control );

   LogD_NumericControl.label.SetText( "Star Intensity:" );
   LogD_NumericControl.label.SetFixedWidth( labelWidth1 );
   LogD_NumericControl.slider.SetScaledMinWidth( 250 );
   LogD_NumericControl.slider.SetRange( 0, 200 );
   LogD_NumericControl.SetReal();
   LogD_NumericControl.SetRange( TheSCSLogDParameter->MinimumValue(), TheSCSLogDParameter->MaximumValue() );
   LogD_NumericControl.SetPrecision( TheSCSLogDParameter->Precision() );
   LogD_NumericControl.OnValueUpdated( (NumericEdit::value_event_handler)&StarComposerInterface::e_NumericControl_ValueUpdated, w );

   ProfileHardness_NumericControl.label.SetText( "Profile Hardness:" );
   ProfileHardness_NumericControl.label.SetFixedWidth( labelWidth1 );
   ProfileHardness_NumericControl.slider.SetScaledMinWidth( 250 );
   ProfileHardness_NumericControl.slider.SetRange( 0, 100 );
   ProfileHardness_NumericControl.SetReal();
   ProfileHardness_NumericControl.SetRange( TheSCSProfileHardnessParameter->MinimumValue(), TheSCSProfileHardnessParameter->MaximumValue() );
   ProfileHardness_NumericControl.SetPrecision( TheSCSProfileHardnessParameter->Precision() );
   ProfileHardness_NumericControl.OnValueUpdated( (NumericEdit::value_event_handler)&StarComposerInterface::e_NumericControl_ValueUpdated, w );

   AdaptiveAnchor_CheckBox.SetText( "Adaptive Anchor" );
   AdaptiveAnchor_CheckBox.OnClick( (Button::click_event_handler)&StarComposerInterface::e_AdaptiveAnchor_Click, w );

   AdaptiveAnchor_Sizer.AddUnscaledSpacing( labelWidth1 + ui4 );
   AdaptiveAnchor_Sizer.Add( AdaptiveAnchor_CheckBox );
   AdaptiveAnchor_Sizer.AddStretch();

   Stretch_Sizer.SetMargin( 6 );
   Stretch_Sizer.SetSpacing( ui4 );
   Stretch_Sizer.Add( LogD_NumericControl );
   Stretch_Sizer.Add( ProfileHardness_NumericControl );
   Stretch_Sizer.Add( AdaptiveAnchor_Sizer );

   Stretch_Control.SetSizer( Stretch_Sizer );

   //

   Physics_SectionBar.SetTitle( "Hybrid Physics" );
   Physics_SectionBar.SetSection( Physics_Control );

   ColorGrip_NumericControl.label.SetText( "Color Grip:" );
   ColorGrip_NumericControl.label.SetFixedWidth( labelWidth1 );
   ColorGrip_NumericControl.slider.SetScaledMinWidth( 250 );
   ColorGrip_NumericControl.slider.SetRange( 0, 100 );
   ColorGrip_NumericControl.SetReal();
   ColorGrip_NumericControl.SetRange( TheSCSColorGripParameter->MinimumValue(), TheSCSColorGripParameter->MaximumValue() );
   ColorGrip_NumericControl.SetPrecision( TheSCSColorGripParameter->Precision() );
   ColorGrip_NumericControl.OnValueUpdated( (NumericEdit::value_event_handler)&StarComposerInterface::e_NumericControl_ValueUpdated, w );

   ShadowConvergence_NumericControl.label.SetText( "Shadow Conv:" );
   ShadowConvergence_NumericControl.label.SetFixedWidth( labelWidth1 );
   ShadowConvergence_NumericControl.slider.SetScaledMinWidth( 250 );
   ShadowConvergence_NumericControl.slider.SetRange( 0, 300 );
   ShadowConvergence_NumericControl.SetReal();
   ShadowConvergence_NumericControl.SetRange( TheSCSShadowConvergenceParameter->MinimumValue(), TheSCSShadowConvergenceParameter->MaximumValue() );
   ShadowConvergence_NumericControl.SetPrecision( TheSCSShadowConvergenceParameter->Precision() );
   ShadowConvergence_NumericControl.OnValueUpdated( (NumericEdit::value_event_handler)&StarComposerInterface::e_NumericControl_ValueUpdated, w );

   Physics_Sizer.SetMargin( 6 );
   Physics_Sizer.SetSpacing( ui4 );
   Physics_Sizer.Add( ColorGrip_NumericControl );
   Physics_Sizer.Add( ShadowConvergence_NumericControl );

   Physics_Control.SetSizer( Physics_Sizer );

   //

   Surgery_SectionBar.SetTitle( "Star Surgery" );
   Surgery_SectionBar.SetSection( Surgery_Control );

   CoreRejection_NumericControl.label.SetText( "Core Rejection:" );
   CoreRejection_NumericControl.label.SetFixedWidth( labelWidth1 );
   CoreRejection_NumericControl.slider.SetScaledMinWidth( 250 );
   CoreRejection_NumericControl.slider.SetRange( 0, 100 );
   CoreRejection_NumericControl.SetReal();
   CoreRejection_NumericControl.SetRange( TheSCSCoreRejectionParameter->MinimumValue(), TheSCSCoreRejectionParameter->MaximumValue() );
   CoreRejection_NumericControl.SetPrecision( TheSCSCoreRejectionParameter->Precision() );
   CoreRejection_NumericControl.OnValueUpdated( (NumericEdit::value_event_handler)&StarComposerInterface::e_NumericControl_ValueUpdated, w );

   MorphReduction_NumericControl.label.SetText( "Morph. Reduction:" );
   MorphReduction_NumericControl.label.SetFixedWidth( labelWidth1 );
   MorphReduction_NumericControl.slider.SetScaledMinWidth( 250 );
   MorphReduction_NumericControl.slider.SetRange( 0, 100 );
   MorphReduction_NumericControl.SetReal();
   MorphReduction_NumericControl.SetRange( TheSCSMorphReductionParameter->MinimumValue(), TheSCSMorphReductionParameter->MaximumValue() );
   MorphReduction_NumericControl.SetPrecision( TheSCSMorphReductionParameter->Precision() );
   MorphReduction_NumericControl.OnValueUpdated( (NumericEdit::value_event_handler)&StarComposerInterface::e_NumericControl_ValueUpdated, w );

   OpticalHealing_NumericControl.label.SetText( "Optical Healing:" );
   OpticalHealing_NumericControl.label.SetFixedWidth( labelWidth1 );
   OpticalHealing_NumericControl.slider.SetScaledMinWidth( 250 );
   OpticalHealing_NumericControl.slider.SetRange( 0, 200 );
   OpticalHealing_NumericControl.SetReal();
   OpticalHealing_NumericControl.SetRange( TheSCSOpticalHealingParameter->MinimumValue(), TheSCSOpticalHealingParameter->MaximumValue() );
   OpticalHealing_NumericControl.SetPrecision( TheSCSOpticalHealingParameter->Precision() );
   OpticalHealing_NumericControl.OnValueUpdated( (NumericEdit::value_event_handler)&StarComposerInterface::e_NumericControl_ValueUpdated, w );

   Surgery_Sizer.SetMargin( 6 );
   Surgery_Sizer.SetSpacing( ui4 );
   Surgery_Sizer.Add( CoreRejection_NumericControl );
   Surgery_Sizer.Add( MorphReduction_NumericControl );
   Surgery_Sizer.Add( OpticalHealing_NumericControl );

   Surgery_Control.SetSizer( Surgery_Sizer );

   //

   Global_Sizer.SetMargin( 8 );
   Global_Sizer.SetSpacing( ui4 );
   Global_Sizer.Add( Input_SectionBar );
   Global_Sizer.Add( Input_Control );
   Global_Sizer.Add( Sensor_SectionBar );
   Global_Sizer.Add( Sensor_Control );
   Global_Sizer.Add( Stretch_SectionBar );
   Global_Sizer.Add( Stretch_Control );
   Global_Sizer.Add( Physics_SectionBar );
   Global_Sizer.Add( Physics_Control );
   Global_Sizer.Add( Surgery_SectionBar );
   Global_Sizer.Add( Surgery_Control );

   w.SetSizer( Global_Sizer );

   w.EnsureLayoutUpdated();
   w.AdjustToContents();
   w.SetMinWidth();
}

// ----------------------------------------------------------------------------

} // pcl

// ----------------------------------------------------------------------------
