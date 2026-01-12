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

#include "HyperMetricStretchInterface.h"
#include "HyperMetricStretchProcess.h"
#include "HyperMetricStretchParameters.h"

#include <pcl/Console.h>
#include <pcl/ErrorHandler.h>
#include <pcl/MessageBox.h>
#include <pcl/RealTimePreview.h>

namespace pcl
{

// ----------------------------------------------------------------------------

HyperMetricStretchInterface* TheHyperMetricStretchInterface = nullptr;

// ----------------------------------------------------------------------------

HyperMetricStretchInterface::HyperMetricStretchInterface()
   : m_instance( TheHyperMetricStretchProcess )
{
   // NOTE: Console is not available during interface construction.
   // This constructor is called during module installation.
   
   TheHyperMetricStretchInterface = this;
}

// ----------------------------------------------------------------------------

HyperMetricStretchInterface::~HyperMetricStretchInterface()
{
   if ( GUI != nullptr )
      delete GUI, GUI = nullptr;
}

// ----------------------------------------------------------------------------

IsoString HyperMetricStretchInterface::Id() const
{
   return "HyperMetricStretch";
}

// ----------------------------------------------------------------------------

MetaProcess* HyperMetricStretchInterface::Process() const
{
   return TheHyperMetricStretchProcess;
}

// ----------------------------------------------------------------------------

String HyperMetricStretchInterface::IconImageSVGFile() const
{
   return "@module_icons_dir/HyperMetricStretch.svg";
}

// ----------------------------------------------------------------------------

InterfaceFeatures HyperMetricStretchInterface::Features() const
{
   return InterfaceFeature::Default | InterfaceFeature::RealTimeButton;
}

// ----------------------------------------------------------------------------

void HyperMetricStretchInterface::ApplyInstance() const
{
   m_instance.LaunchOnCurrentView();
}

// ----------------------------------------------------------------------------

void HyperMetricStretchInterface::ResetInstance()
{
   // Preserve the current processing mode when resetting
   pcl_enum currentMode = m_instance.processingMode;
   
   HyperMetricStretchInstance defaultInstance( TheHyperMetricStretchProcess );
   defaultInstance.processingMode = currentMode;
   
   ImportProcess( defaultInstance );
}

// ----------------------------------------------------------------------------

void HyperMetricStretchInterface::RealTimePreviewUpdated( bool active )
{
   if ( GUI != nullptr )
      if ( active )
         RealTimePreview::SetOwner( *this );
      else
         RealTimePreview::SetOwner( ProcessInterface::Null() );
}

// ----------------------------------------------------------------------------

bool HyperMetricStretchInterface::Launch( const MetaProcess& P, const ProcessImplementation*, 
                                           bool& dynamic, unsigned& /*flags*/ )
{
   try
   {
#ifdef DEBUG
      Console().WriteLn( "Launching HyperMetricStretch interface..." );
#endif
      
      if ( GUI == nullptr )
      {
#ifdef DEBUG
         Console().WriteLn( "Creating GUI..." );
#endif
         GUI = new GUIData( *this );
         SetWindowTitle( "VeraLux HyperMetric Stretch" );
         UpdateControls();
         
#ifdef DEBUG
         Console().WriteLn( "GUI created successfully." );
#endif
      }

      dynamic = false;
      
#ifdef DEBUG
      Console().WriteLn( "Interface launched successfully." );
#endif
      
      return &P == TheHyperMetricStretchProcess;
   }
   catch ( const std::exception& e )
   {
      Console console;
      console.CriticalLn( "<end><cbr>*** Error: Failed to launch HyperMetricStretch interface ***" );
      console.CriticalLn( String().Format( "Exception: %s", e.what() ) );
      return false;
   }
   catch ( ... )
   {
      Console().CriticalLn( "<end><cbr>*** Error: Failed to launch HyperMetricStretch interface with unknown exception ***" );
      return false;
   }
}

// ----------------------------------------------------------------------------

ProcessImplementation* HyperMetricStretchInterface::NewProcess() const
{
   return new HyperMetricStretchInstance( m_instance );
}

// ----------------------------------------------------------------------------

bool HyperMetricStretchInterface::ValidateProcess( const ProcessImplementation& p, String& whyNot ) const
{
   if ( dynamic_cast<const HyperMetricStretchInstance*>( &p ) != nullptr )
      return true;
   whyNot = "Not a HyperMetric Stretch instance.";
   return false;
}

// ----------------------------------------------------------------------------

bool HyperMetricStretchInterface::RequiresInstanceValidation() const
{
   return true;
}

// ----------------------------------------------------------------------------

bool HyperMetricStretchInterface::ImportProcess( const ProcessImplementation& p )
{
   m_instance.Assign( p );
   UpdateControls();
   UpdateRealTimePreview();
   return true;
}

// ----------------------------------------------------------------------------

bool HyperMetricStretchInterface::RequiresRealTimePreviewUpdate( const UInt16Image&, const View&, 
                                                                   const Rect&, int ) const
{
   return true;
}

// ----------------------------------------------------------------------------

bool HyperMetricStretchInterface::GenerateRealTimePreview( UInt16Image& image, const View&, 
                                                             const Rect&, int, String& info ) const
{
   // Convert to working format
   Image work;
   work.Assign( image );

   // Apply stretch
   if ( !m_instance.Preview( work ) )
      return false;

   // Convert back
   image.Assign( work );

   // Update info string
   info = String().Format( "Log D: %.2f | Bg: %.2f", m_instance.logD, m_instance.targetBackground );

   return true;
}

// ----------------------------------------------------------------------------

void HyperMetricStretchInterface::UpdateControls()
{
   // Update mode radio buttons
   GUI->ReadyToUse_RadioButton.SetChecked( m_instance.processingMode == HMSProcessingMode::ReadyToUse );
   GUI->Scientific_RadioButton.SetChecked( m_instance.processingMode == HMSProcessingMode::Scientific );

   // Update sensor profile
   GUI->SensorProfile_ComboBox.SetCurrentItem( int( m_instance.sensorProfile ) );

   // Update checkboxes
   GUI->AdaptiveAnchor_CheckBox.SetChecked( m_instance.adaptiveAnchor );

   // Update numeric controls
   GUI->TargetBg_NumericControl.SetValue( m_instance.targetBackground );
   GUI->LogD_NumericControl.SetValue( m_instance.logD );
   GUI->ProtectB_NumericControl.SetValue( m_instance.protectB );
   GUI->ColorConvergence_NumericControl.SetValue( m_instance.colorConvergence );

   // Mode-specific
   GUI->ColorStrategy_NumericControl.SetValue( m_instance.colorStrategy );
   GUI->LinearExpansion_NumericControl.SetValue( m_instance.linearExpansion );
   GUI->ColorGrip_NumericControl.SetValue( m_instance.colorGrip );
   GUI->ShadowConvergence_NumericControl.SetValue( m_instance.shadowConvergence );

   UpdateModeControls();
   UpdateSensorInfo();
   UpdateColorStrategyInfo();
}

// ----------------------------------------------------------------------------

void HyperMetricStretchInterface::UpdateModeControls()
{
   bool isReadyToUse = (m_instance.processingMode == HMSProcessingMode::ReadyToUse);

   GUI->ReadyToUse_SectionBar.SetVisible( isReadyToUse );
   GUI->ReadyToUse_Control.SetVisible( isReadyToUse );
   GUI->Scientific_SectionBar.SetVisible( !isReadyToUse );
   GUI->Scientific_Control.SetVisible( !isReadyToUse );

   SetVariableSize();
   AdjustToContents();
   SetMinWidth();
}

// ----------------------------------------------------------------------------

void HyperMetricStretchInterface::UpdateSensorInfo()
{
   const SensorProfile& profile = m_instance.GetSensorProfile();
   String info = String().Format( "R: %.4f, G: %.4f, B: %.4f",
                                  profile.rWeight, profile.gWeight, profile.bWeight );
   GUI->SensorProfile_Info.SetText( info );
}

// ----------------------------------------------------------------------------

void HyperMetricStretchInterface::UpdateColorStrategyInfo()
{
   int val = m_instance.colorStrategy;
   double grip, shadow, linearExp;
   m_instance.GetEffectiveParams( grip, shadow, linearExp );

   String txt;
   if ( val < 0 )
      txt = String().Format( "Action: Noise Cleaning (Shadow Conv: %.1f)", shadow );
   else if ( val > 0 )
      txt = String().Format( "Action: Highlight Softening (Grip: %.2f)", grip );
   else
      txt = "Balanced (Pure Vector)";

   GUI->ColorStrategy_Info.SetText( txt );
}

// ----------------------------------------------------------------------------

void HyperMetricStretchInterface::UpdateRealTimePreview()
{
   if ( IsRealTimePreviewActive() )
      RealTimePreview::Update();
}

// ----------------------------------------------------------------------------
// Event Handlers
// ----------------------------------------------------------------------------

void HyperMetricStretchInterface::e_Mode_Click( Button& sender, bool checked )
{
   if ( !checked )
      return;

   if ( sender == GUI->ReadyToUse_RadioButton )
      m_instance.processingMode = HMSProcessingMode::ReadyToUse;
   else if ( sender == GUI->Scientific_RadioButton )
      m_instance.processingMode = HMSProcessingMode::Scientific;

   UpdateModeControls();
   UpdateRealTimePreview();
}

// ----------------------------------------------------------------------------

void HyperMetricStretchInterface::e_SensorProfile_Selected( ComboBox& /*sender*/, int itemIndex )
{
   m_instance.sensorProfile = itemIndex;
   UpdateSensorInfo();
   UpdateRealTimePreview();
}

// ----------------------------------------------------------------------------

void HyperMetricStretchInterface::e_AdaptiveAnchor_Click( Button& /*sender*/, bool checked )
{
   m_instance.adaptiveAnchor = checked;
   UpdateRealTimePreview();
}

// ----------------------------------------------------------------------------

void HyperMetricStretchInterface::e_NumericControl_ValueUpdated( NumericControl& sender, double value )
{
   if ( sender == GUI->TargetBg_NumericControl )
      m_instance.targetBackground = value;
   else if ( sender == GUI->LogD_NumericControl )
      m_instance.logD = value;
   else if ( sender == GUI->ProtectB_NumericControl )
      m_instance.protectB = value;
   else if ( sender == GUI->ColorConvergence_NumericControl )
      m_instance.colorConvergence = value;
   else if ( sender == GUI->ColorStrategy_NumericControl )
   {
      m_instance.colorStrategy = int( value );
      UpdateColorStrategyInfo();
   }
   else if ( sender == GUI->LinearExpansion_NumericControl )
      m_instance.linearExpansion = value;
   else if ( sender == GUI->ColorGrip_NumericControl )
      m_instance.colorGrip = value;
   else if ( sender == GUI->ShadowConvergence_NumericControl )
      m_instance.shadowConvergence = value;

   UpdateRealTimePreview();
}

// ----------------------------------------------------------------------------

void HyperMetricStretchInterface::e_AutoCalc_Click( Button& /*sender*/, bool /*checked*/ )
{
   try
   {
      // Get current image
      ImageWindow window = ImageWindow::ActiveWindow();
      if ( window.IsNull() )
      {
         pcl::MessageBox( "No active image window.", "Auto-Calc Log D", 
                          StdIcon::Error, StdButton::Ok ).Execute();
         return;
      }
      
      View view = window.MainView();
      if ( view.IsNull() )
      {
         pcl::MessageBox( "No active view.", "Auto-Calc Log D", 
                          StdIcon::Error, StdButton::Ok ).Execute();
         return;
      }

      Console console;
      console.WriteLn( "<end><cbr>Computing optimal Log D..." );
      console.Flush();

      // Prepare luminance
      ImageVariant img = view.Image();
      Image working;
      VeraLuxEngine::NormalizeInput( working, img );

      const SensorProfile& profile = m_instance.GetSensorProfile();
      double anchor = m_instance.adaptiveAnchor ?
         VeraLuxEngine::CalculateAnchorAdaptive( working, profile ) :
         VeraLuxEngine::CalculateAnchor( working );

      Image luma;
      VeraLuxEngine::ExtractLuminance( luma, working, anchor, profile );

      // Calculate optimal Log D
      double logD = VeraLuxEngine::SolveLogD( luma, m_instance.targetBackground, m_instance.protectB );

      // Update instance and GUI
      m_instance.logD = logD;
      GUI->LogD_NumericControl.SetValue( logD );
      
      console.WriteLn( String().Format( "<end><cbr>Auto-Calc complete: Log D = %.2f", logD ) );
      
      UpdateRealTimePreview();
   }
   catch ( Exception& x )
   {
      x.Show();
   }
   catch ( ... )
   {
      try
      {
         throw;
      }
      ERROR_HANDLER
   }
}

// ----------------------------------------------------------------------------
// GUI Data Construction
// ----------------------------------------------------------------------------

HyperMetricStretchInterface::GUIData::GUIData( HyperMetricStretchInterface& w )
{
   pcl::Font fnt = w.Font();
   int labelWidth1 = fnt.Width( String( "Shadow Conv:" ) + 'M' );
   int ui4 = w.LogicalPixelsToPhysical( 4 );

   //

   Mode_SectionBar.SetTitle( "Processing Mode" );
   Mode_SectionBar.SetSection( Mode_Control );

   ReadyToUse_RadioButton.SetText( "Ready-to-Use (Aesthetic)" );
   ReadyToUse_RadioButton.SetToolTip( 
      "<p><b>Ready-to-Use Mode:</b></p>"
      "<p>Produces an aesthetic, export-ready image with automatic optimization.</p>"
      "<p>- Applies Smart Max scaling with Zero-Clipping logic to preserve star cores.<br>"
      "- Uses linked MTF to set the background level.<br>"
      "- Applies soft-clipping to reduce star blooming in highlights.</p>"
      "<p>Features a unified Color Strategy control for simplified adjustment between noise cleaning and highlight softening.</p>" );
   ReadyToUse_RadioButton.OnClick( (Button::click_event_handler)&HyperMetricStretchInterface::e_Mode_Click, w );

   Scientific_RadioButton.SetText( "Scientific (Preserve)" );
   Scientific_RadioButton.SetToolTip( 
      "<p><b>Scientific Mode:</b></p>"
      "<p>Produces a 100% mathematically consistent output that preserves absolute luminance ratios and radiometric integrity.</p>"
      "<p>- Clips only at physical saturation (1.0).<br>"
      "- Output may exceed normal range, preserving full photometric accuracy.<br>"
      "- Ideal for manual tone mapping with Curves or additional Hyperbolic stretching.</p>"
      "<p>Provides independent controls (Linear Expansion, Color Grip, Shadow Convergence) for precise manual calibration.</p>" );
   Scientific_RadioButton.OnClick( (Button::click_event_handler)&HyperMetricStretchInterface::e_Mode_Click, w );

   Mode_Sizer.SetSpacing( ui4 );
   Mode_Sizer.Add( ReadyToUse_RadioButton );
   Mode_Sizer.Add( Scientific_RadioButton );
   Mode_Sizer.AddStretch();

   Mode_Sizer.SetMargin( 6 );
   Mode_Control.SetSizer( Mode_Sizer );

   //

   Sensor_SectionBar.SetTitle( "Sensor Calibration" );
   Sensor_SectionBar.SetSection( Sensor_Control );

   SensorProfile_Label.SetText( "Sensor Profile:" );
   SensorProfile_Label.SetFixedWidth( labelWidth1 );
   SensorProfile_Label.SetTextAlignment( TextAlign::Right|TextAlign::VertCenter );

   // Populate sensor profiles
   for ( size_t i = 0; i < g_numSensorProfiles; ++i )
      SensorProfile_ComboBox.AddItem( g_sensorProfiles[i].name );
   SensorProfile_ComboBox.SetToolTip(
      "<p><b>Sensor Profile:</b></p>"
      "<p>Defines the Luminance coefficients (weights) used for the stretch. Each profile represents sensor-specific "
      "spectral response curves (Quantum Efficiency) for accurate luminance calculation.</p>"
      "<p>- <b>Rec.709 (Recommended):</b> Universal compatibility, best for general use, DSLRs, and unknown sensors.<br>"
      "- <b>Sensor-specific profiles:</b> Reduce color cross-talk and improve tonal separation for known hardware.<br>"
      "- <b>Narrowband profiles:</b> Optimized for Ha/OIII/SII channels in narrowband imaging.</p>"
      "<p>Incorrect profiles may cause color shifts in shadows or unnatural highlight transitions.</p>" );
   SensorProfile_ComboBox.OnItemSelected( (ComboBox::item_event_handler)&HyperMetricStretchInterface::e_SensorProfile_Selected, w );

   SensorProfile_Sizer.SetSpacing( ui4 );
   SensorProfile_Sizer.Add( SensorProfile_Label );
   SensorProfile_Sizer.Add( SensorProfile_ComboBox, 100 );

   SensorProfile_Info.SetText( "R: 0.2126, G: 0.7152, B: 0.0722" );
   SensorProfile_Info.SetTextAlignment( TextAlign::Left|TextAlign::VertCenter );

   SensorProfile_Info_Sizer.AddUnscaledSpacing( labelWidth1 + ui4 );
   SensorProfile_Info_Sizer.Add( SensorProfile_Info );
   SensorProfile_Info_Sizer.AddStretch();

   Sensor_Sizer.SetMargin( 6 );
   Sensor_Sizer.SetSpacing( ui4 );
   Sensor_Sizer.Add( SensorProfile_Sizer );
   Sensor_Sizer.Add( SensorProfile_Info_Sizer );

   Sensor_Control.SetSizer( Sensor_Sizer );

   //

   Stretch_SectionBar.SetTitle( "Stretch Parameters" );
   Stretch_SectionBar.SetSection( Stretch_Control );

   TargetBg_NumericControl.label.SetText( "Target Bg:" );
   TargetBg_NumericControl.label.SetFixedWidth( labelWidth1 );
   TargetBg_NumericControl.slider.SetScaledMinWidth( 250 );
   TargetBg_NumericControl.slider.SetRange( 0, 100 );
   TargetBg_NumericControl.SetReal();
   TargetBg_NumericControl.SetRange( TheHMSTargetBackgroundParameter->MinimumValue(),
                                      TheHMSTargetBackgroundParameter->MaximumValue() );
   TargetBg_NumericControl.SetPrecision( TheHMSTargetBackgroundParameter->Precision() );
   TargetBg_NumericControl.SetToolTip( 
      "<p><b>Target Background (Median):</b></p>"
      "<p>The desired median value for the background sky after stretching. Directly controls Log D calculation via the Auto-Calc solver.</p>"
      "<p>- <b>0.20:</b> Standard for good visibility (Statistical Stretch style).<br>"
      "- <b>0.12:</b> High-contrast dark skies with emphasized deep-sky structure.<br>"
      "- <b>Lower values:</b> Darker skies with maximum contrast but may reveal noise in low-SNR regions.<br>"
      "- <b>Higher values (up to 0.50):</b> Brighter, safer for noisy data but may appear flat.</p>" );
   TargetBg_NumericControl.OnValueUpdated( (NumericControl::value_event_handler)&HyperMetricStretchInterface::e_NumericControl_ValueUpdated, w );

   AdaptiveAnchor_CheckBox.SetText( "Adaptive Anchor" );
   AdaptiveAnchor_CheckBox.SetToolTip( 
      "<p><b>Adaptive Anchor:</b></p>"
      "<p>Analyzes the histogram shape to find the true signal start (black point) instead of using a fixed percentile.</p>"
      "<p><b>When enabled (recommended):</b> Maximizes dynamic range utilization and contrast. Particularly effective for well-calibrated data "
      "and images with gradients or vignetting.</p>"
      "<p><b>When disabled:</b> Uses safer fixed percentile clipping (0.001%) for more conservative black point detection. "
      "Better for uncalibrated data or poorly flat-fielded images, but may sacrifice contrast.</p>"
      "<p>Disable if you observe crushed shadows or when working with data that has strong background variations.</p>" );
   AdaptiveAnchor_CheckBox.OnClick( (Button::click_event_handler)&HyperMetricStretchInterface::e_AdaptiveAnchor_Click, w );

   AdaptiveAnchor_Sizer.AddUnscaledSpacing( labelWidth1 + ui4 );
   AdaptiveAnchor_Sizer.Add( AdaptiveAnchor_CheckBox );
   AdaptiveAnchor_Sizer.AddStretch();

   LogD_NumericControl.label.SetText( "Log D:" );
   LogD_NumericControl.label.SetFixedWidth( labelWidth1 );
   LogD_NumericControl.slider.SetScaledMinWidth( 250 );
   LogD_NumericControl.slider.SetRange( 0, 700 );
   LogD_NumericControl.SetReal();
   LogD_NumericControl.SetRange( TheHMSLogDParameter->MinimumValue(),
                                  TheHMSLogDParameter->MaximumValue() );
   LogD_NumericControl.SetPrecision( TheHMSLogDParameter->Precision() );
   LogD_NumericControl.SetToolTip( 
      "<p><b>Log D (Hyperbolic Intensity):</b></p>"
      "<p>Controls the strength of the stretch. The primary factor controlling logarithmic compression of the dynamic range.</p>"
      "<p>- <b>Lower values (&lt; 1.5):</b> Gentle stretching with minimal tonal compression, suitable for already-stretched or HDR data.<br>"
      "- <b>Moderate values (2.0-3.0):</b> Standard stretching optimal for typical deep-sky linear data, balancing midtone detail with highlight protection.<br>"
      "- <b>Higher values (3.5-5.0):</b> Aggressive stretching that maximizes faint detail visibility but risks noise amplification.<br>"
      "- <b>Very high values (&gt; 5.0):</b> Extreme compression useful for very dim nebulosity but may cause posterization.</p>"
      "<p><b>Use Auto-Calc to solve for the optimal Log D</b> that places your background at the Target Background level.</p>" );
   LogD_NumericControl.OnValueUpdated( (NumericControl::value_event_handler)&HyperMetricStretchInterface::e_NumericControl_ValueUpdated, w );

   AutoCalc_PushButton.SetText( "Auto-Calc" );
   AutoCalc_PushButton.SetToolTip( 
      "<p><b>Auto-Solver:</b></p>"
      "<p>Analyzes the image data to find the <b>Stretch Factor (Log D)</b> that places the current background median at the Target Background level.</p>"
      "<p>Uses an intelligent iterative solver with predictive feedback loop to optimize dynamic range allocation. In Ready-to-Use mode, "
      "performs 'Floating Sky Check' simulation to prevent black clipping while maximizing contrast.</p>"
      "<p>Click after adjusting Target Background, changing Adaptive Anchor, or loading a new image.</p>" );
   AutoCalc_PushButton.OnClick( (Button::click_event_handler)&HyperMetricStretchInterface::e_AutoCalc_Click, w );

   LogD_Sizer.SetSpacing( ui4 );
   LogD_Sizer.Add( LogD_NumericControl, 100 );
   LogD_Sizer.Add( AutoCalc_PushButton );

   ProtectB_NumericControl.label.SetText( "Protect b:" );
   ProtectB_NumericControl.label.SetFixedWidth( labelWidth1 );
   ProtectB_NumericControl.slider.SetScaledMinWidth( 250 );
   ProtectB_NumericControl.slider.SetRange( 0, 1500 );
   ProtectB_NumericControl.SetReal();
   ProtectB_NumericControl.SetRange( TheHMSProtectBParameter->MinimumValue(),
                                      TheHMSProtectBParameter->MaximumValue() );
   ProtectB_NumericControl.SetPrecision( TheHMSProtectBParameter->Precision() );
   ProtectB_NumericControl.SetToolTip( 
      "<p><b>Protect b (Highlight Protection / Hyperbolic Knee):</b></p>"
      "<p>Controls the knee of the Hyperbolic curve, determining the rolloff point for highlight compression.</p>"
      "<p>- <b>Lower values (1.0-3.0):</b> Apply compression earlier with strong protection of bright stars and cores. "
      "Prevents blown-out star centers in crowded fields, though may appear 'crunchy'.<br>"
      "- <b>Moderate values (4.0-8.0):</b> Balanced, natural stellar profiles with smooth core-to-halo transitions.<br>"
      "- <b>Higher values (&gt; 8.0):</b> Minimal protection with near-linear response, preserving stellar cores and highlights. "
      "Preferred for scientific photometry but risks clipping with aggressive Log D.</p>"
      "<p><b>Important:</b> Interacts strongly with Log D. Higher Log D requires higher Protect b to avoid over-compression of bright regions.</p>" );
   ProtectB_NumericControl.OnValueUpdated( (NumericControl::value_event_handler)&HyperMetricStretchInterface::e_NumericControl_ValueUpdated, w );

   ColorConvergence_NumericControl.label.SetText( "Color Conv:" );
   ColorConvergence_NumericControl.label.SetFixedWidth( labelWidth1 );
   ColorConvergence_NumericControl.slider.SetScaledMinWidth( 250 );
   ColorConvergence_NumericControl.slider.SetRange( 0, 100 );
   ColorConvergence_NumericControl.SetReal();
   ColorConvergence_NumericControl.SetRange( TheHMSColorConvergenceParameter->MinimumValue(),
                                               TheHMSColorConvergenceParameter->MaximumValue() );
   ColorConvergence_NumericControl.SetPrecision( TheHMSColorConvergenceParameter->Precision() );
   ColorConvergence_NumericControl.SetToolTip( 
      "<p><b>Star Core Recovery (White Point / Color Convergence):</b></p>"
      "<p>Controls how quickly saturated colors transition to white in highlights. Mimics the physical response of sensors/film "
      "where high-intensity regions naturally desaturate.</p>"
      "<p>- <b>Lower values (1.0-2.0):</b> Preserve color in bright regions but risk unnatural chromatic artifacts in overexposed star cores (color 'fireflies').<br>"
      "- <b>Moderate values (3.0-4.0):</b> Smooth, natural transitions to white in star centers. Recommended for most images.<br>"
      "- <b>Higher values (&gt; 5.0):</b> Faster transition to white cores, avoiding color artifacts. May appear overly bleached if Log D is too high.</p>"
      "<p>Essential for preventing false color in saturated regions, particularly in narrowband or high-dynamic-range data.</p>" );
   ColorConvergence_NumericControl.OnValueUpdated( (NumericControl::value_event_handler)&HyperMetricStretchInterface::e_NumericControl_ValueUpdated, w );

   Stretch_Sizer.SetMargin( 6 );
   Stretch_Sizer.SetSpacing( ui4 );
   Stretch_Sizer.Add( TargetBg_NumericControl );
   Stretch_Sizer.Add( AdaptiveAnchor_Sizer );
   Stretch_Sizer.Add( LogD_Sizer );
   Stretch_Sizer.Add( ProtectB_NumericControl );
   Stretch_Sizer.Add( ColorConvergence_NumericControl );

   Stretch_Control.SetSizer( Stretch_Sizer );

   //

   ReadyToUse_SectionBar.SetTitle( "Ready-to-Use Mode" );
   ReadyToUse_SectionBar.SetSection( ReadyToUse_Control );

   ColorStrategy_NumericControl.label.SetText( "Color Strategy:" );
   ColorStrategy_NumericControl.label.SetFixedWidth( labelWidth1 );
   ColorStrategy_NumericControl.slider.SetScaledMinWidth( 250 );
   ColorStrategy_NumericControl.slider.SetRange( -100, 100 );
   ColorStrategy_NumericControl.SetReal();
   ColorStrategy_NumericControl.SetRange( TheHMSColorStrategyParameter->MinimumValue(),
                                            TheHMSColorStrategyParameter->MaximumValue() );
   ColorStrategy_NumericControl.SetPrecision( 0 );
   ColorStrategy_NumericControl.SetToolTip( 
      "<p><b>Unified Color Strategy (Ready-to-Use Mode only):</b></p>"
      "<p>Single control that interpolates between noise cleaning (negative) and highlight softening (positive).</p>"
      "<p>- <b>Center (0):</b> Balanced pure VeraLux vector stretch with full color preservation across the entire tonal range. "
      "Optimal for clean, well-integrated data.<br>"
      "- <b>Left (&lt; 0, Clean Noise):</b> Increases Shadow Convergence to reduce chrominance noise and color mottling in dark regions. "
      "Blends shadows toward neutral. Effective for high-ISO/short-exposure data but may desaturate dim nebulae.<br>"
      "- <b>Right (&gt; 0, Soften Highlights):</b> Decreases Color Grip to blend bright pixels toward scalar stretch. "
      "Creates softer, less saturated star cores that prevent 'neon' artifacts in extremely bright regions.</p>"
      "<p><i>Double-click the slider to reset to center (0).</i></p>" );
   ColorStrategy_NumericControl.OnValueUpdated( (NumericControl::value_event_handler)&HyperMetricStretchInterface::e_NumericControl_ValueUpdated, w );

   ColorStrategy_Info.SetText( "Balanced (Pure Vector)" );
   ColorStrategy_Info.SetTextAlignment( TextAlign::Left|TextAlign::VertCenter );

   ColorStrategy_Info_Sizer.AddUnscaledSpacing( labelWidth1 + ui4 );
   ColorStrategy_Info_Sizer.Add( ColorStrategy_Info );
   ColorStrategy_Info_Sizer.AddStretch();

   ReadyToUse_Sizer.SetMargin( 6 );
   ReadyToUse_Sizer.SetSpacing( ui4 );
   ReadyToUse_Sizer.Add( ColorStrategy_NumericControl );
   ReadyToUse_Sizer.Add( ColorStrategy_Info_Sizer );

   ReadyToUse_Control.SetSizer( ReadyToUse_Sizer );

   //

   Scientific_SectionBar.SetTitle( "Scientific Mode" );
   Scientific_SectionBar.SetSection( Scientific_Control );

   LinearExpansion_NumericControl.label.SetText( "Linear Expan:" );
   LinearExpansion_NumericControl.label.SetFixedWidth( labelWidth1 );
   LinearExpansion_NumericControl.slider.SetScaledMinWidth( 250 );
   LinearExpansion_NumericControl.slider.SetRange( 0, 100 );
   LinearExpansion_NumericControl.SetReal();
   LinearExpansion_NumericControl.SetRange( TheHMSLinearExpansionParameter->MinimumValue(),
                                              TheHMSLinearExpansionParameter->MaximumValue() );
   LinearExpansion_NumericControl.SetPrecision( TheHMSLinearExpansionParameter->Precision() );
   LinearExpansion_NumericControl.SetToolTip( 
      "<p><b>Linear Expansion (Scientific Mode only):</b></p>"
      "<p>Post-stretch normalization that rescales the output to fill the dynamic range (0-1) using intelligent black-point and white-point detection.</p>"
      "<p>- <b>Minimum (0.0):</b> Preserves raw stretch output maintaining absolute photometric linearity. Values may exceed normal range or remain dim.<br>"
      "- <b>Low (&lt; 0.3):</b> Anchors blacks (0.001%) to remove background haze with gentle normalization. Useful for multi-stage workflows.<br>"
      "- <b>Moderate (0.3-0.7):</b> Brings output closer to full range while maintaining headroom, balancing brightness with data preservation.<br>"
      "- <b>High (&gt; 0.7):</b> Expands to the absolute physical limit using <b>Smart Max</b> logic. Maximizes visual impact while preserving star cores and rejecting hot pixels.</p>"
      "<p><b>Essential</b> for bringing Scientific mode output to visually usable levels. Increases contrast and brightness simultaneously. Does not affect Ready-to-Use mode.</p>" );
   LinearExpansion_NumericControl.OnValueUpdated( (NumericControl::value_event_handler)&HyperMetricStretchInterface::e_NumericControl_ValueUpdated, w );

   ColorGrip_NumericControl.label.SetText( "Color Grip:" );
   ColorGrip_NumericControl.label.SetFixedWidth( labelWidth1 );
   ColorGrip_NumericControl.slider.SetScaledMinWidth( 250 );
   ColorGrip_NumericControl.slider.SetRange( 0, 100 );
   ColorGrip_NumericControl.SetReal();
   ColorGrip_NumericControl.SetRange( TheHMSColorGripParameter->MinimumValue(),
                                        TheHMSColorGripParameter->MaximumValue() );
   ColorGrip_NumericControl.SetPrecision( TheHMSColorGripParameter->Precision() );
   ColorGrip_NumericControl.SetToolTip( 
      "<p><b>Color Grip (Global) - Scientific Mode only:</b></p>"
      "<p>Controls the rigor of Color Vector preservation. Fader between pure VeraLux vector stretch and traditional scalar (intensity-based) stretch.</p>"
      "<p>- <b>1.00 (Default):</b> Pure VeraLux with 100% vector lock. Maximum vividness and locked chromatic ratios. "
      "Ideal for narrowband composites or when color fidelity is paramount, though may produce 'electric' star cores in extreme cases.<br>"
      "- <b>High (0.75-0.99):</b> Mostly vector-locked with slight softening in extreme highlights, balancing saturation with natural appearance.<br>"
      "- <b>Moderate (0.30-0.74):</b> Visible transition toward scalar behavior in highlights, reducing chromatic 'pop' for more subdued stellar profiles.<br>"
      "- <b>Low (&lt; 0.30):</b> Blends significantly with standard scalar stretch. Softens star cores and relaxes saturation in highlights for conventional rendering.</p>"
      "<p>Lower values sacrifice VeraLux's unique color preservation for smoother, more traditional highlight appearance.</p>" );
   ColorGrip_NumericControl.OnValueUpdated( (NumericControl::value_event_handler)&HyperMetricStretchInterface::e_NumericControl_ValueUpdated, w );

   ShadowConvergence_NumericControl.label.SetText( "Shadow Conv:" );
   ShadowConvergence_NumericControl.label.SetFixedWidth( labelWidth1 );
   ShadowConvergence_NumericControl.slider.SetScaledMinWidth( 250 );
   ShadowConvergence_NumericControl.slider.SetRange( 0, 300 );
   ShadowConvergence_NumericControl.SetReal();
   ShadowConvergence_NumericControl.SetRange( TheHMSShadowConvergenceParameter->MinimumValue(),
                                                TheHMSShadowConvergenceParameter->MaximumValue() );
   ShadowConvergence_NumericControl.SetPrecision( TheHMSShadowConvergenceParameter->Precision() );
   ShadowConvergence_NumericControl.SetToolTip( 
      "<p><b>Shadow Convergence (Noise Reduction) - Scientific Mode only:</b></p>"
      "<p>Dampens vector preservation in deep shadows to prevent color noise bloom. Blends toward scalar stretch in dark areas, "
      "simulating sensor dark current characteristics.</p>"
      "<p>- <b>0.0 (Off):</b> Pure Vector in shadows with maximum chromatic fidelity. Exposes all color noise, banding, and hot pixels. "
      "Only recommended for pristine, low-noise data.<br>"
      "- <b>Low (0.1-0.5):</b> Gentle convergence with subtle noise suppression while retaining most shadow color. "
      "Cleans up minor artifacts without visible desaturation.<br>"
      "- <b>Moderate (0.6-1.5):</b> Balanced noise reduction suitable for typical integrated data.<br>"
      "- <b>High (&gt; 1.5):</b> Aggressive convergence where shadows become progressively neutral. "
      "Effective for high-ISO/short-exposure data or uncooled sensors but may desaturate dim nebulosity.</p>"
      "<p>Recommended for noisy images. Use sparingly on clean integrations to avoid losing faint chromatic detail in nebulae.</p>" );
   ShadowConvergence_NumericControl.OnValueUpdated( (NumericControl::value_event_handler)&HyperMetricStretchInterface::e_NumericControl_ValueUpdated, w );

   Scientific_Sizer.SetMargin( 6 );
   Scientific_Sizer.SetSpacing( ui4 );
   Scientific_Sizer.Add( LinearExpansion_NumericControl );
   Scientific_Sizer.Add( ColorGrip_NumericControl );
   Scientific_Sizer.Add( ShadowConvergence_NumericControl );

   Scientific_Control.SetSizer( Scientific_Sizer );

   //

   Global_Sizer.SetMargin( 8 );
   Global_Sizer.SetSpacing( ui4 );
   Global_Sizer.Add( Mode_SectionBar );
   Global_Sizer.Add( Mode_Control );
   Global_Sizer.Add( Sensor_SectionBar );
   Global_Sizer.Add( Sensor_Control );
   Global_Sizer.Add( Stretch_SectionBar );
   Global_Sizer.Add( Stretch_Control );
   Global_Sizer.Add( ReadyToUse_SectionBar );
   Global_Sizer.Add( ReadyToUse_Control );
   Global_Sizer.Add( Scientific_SectionBar );
   Global_Sizer.Add( Scientific_Control );

   w.SetSizer( Global_Sizer );

   // Hide Scientific mode sections initially (Ready-to-Use is default)
   Scientific_SectionBar.Hide();
   Scientific_Control.Hide();

   w.EnsureLayoutUpdated();
   w.AdjustToContents();
   w.SetMinWidth();
}

// ----------------------------------------------------------------------------

} // pcl

// ----------------------------------------------------------------------------
// EOF HyperMetricStretchInterface.cpp - Released 2025-01-06T00:00:00Z
