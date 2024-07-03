
@REM Loop through all bat file args and check if any are debug or release
@REM No Default build configuration to prevent command line args from causing errors with folders

@set BUILD_CONFIG=Default

@for %%x in (%*) do @(
    @if %%~x==Debug (@set BUILD_CONFIG=Debug)
    @if %%~x==Release (@set BUILD_CONFIG=Release)
)

@if "%BUILD_CONFIG%" == "Default" (
echo no configuration passed. Please enter arg [Debug] or [Release]
echo ... ending build
exit /b
)

del /Q ".\bin\%BUILD_CONFIG%\Modules\MVM\*.*"

del /Q ".\bin\%BUILD_CONFIG%\Modules\*.*"

del /Q ".\bin\%BUILD_CONFIG%\Modules_Native\*.*"

del /Q ".\bin\%BUILD_CONFIG%\Lib\*.*"

del /Q ".\bin\%BUILD_CONFIG%\en-US\*.*"

del /Q ".\bin\%BUILD_CONFIG%\ru-ru\*.*"

del /Q ".\bin\%BUILD_CONFIG%\*.*"

mkdir ".\bin\%BUILD_CONFIG%\Modules\"

mkdir ".\bin\%BUILD_CONFIG%\Modules\MVM"

mkdir ".\bin\%BUILD_CONFIG%\Modules_Native\"

mkdir ".\bin\%BUILD_CONFIG%\Lib\"

mkdir ".\bin\%BUILD_CONFIG%\en-US\"

mkdir ".\bin\%BUILD_CONFIG%\ru-ru\"

@if not exist %UserProfile%"\Documents\ThorImageLS 4.4\" ( 
 echo Documents folder does not exist... creating folder
 mkdir "%UserProfile%\Documents\ThorImageLS 4.4"
 mkdir "%UserProfile%\Documents\ThorImageLS 4.4\Application Settings"
 mkdir "%UserProfile%\Documents\ThorImageLS 4.4\Capture Templates"
 mkdir "%UserProfile%\Documents\ThorImageLS 4.4\Matlab Scripts"
 mkdir "%UserProfile%\Documents\ThorImageLS 4.4\Modalities"
 echo Copying User Folders
 echo F | xcopy /D "..\..\..\..\Installers\ThorImageLSx64\ThorImageLSx64\Application Settings\*" "%UserProfile%\Documents\ThorImageLS 4.4\Application Settings" /s
 echo F | xcopy /D "..\..\..\..\Installers\ThorImageLSx64\ThorImageLSx64\Capture Templates\*" "%UserProfile%\Documents\ThorImageLS 4.4\Capture Templates" /s
 echo F | xcopy /D "..\..\..\..\Installers\ThorImageLSx64\ThorImageLSx64\Matlab Scripts\*" "%UserProfile%\Documents\ThorImageLS 4.4\Matlab Scripts" /s
 echo F | xcopy /D "..\..\..\..\Installers\ThorImageLSx64\ThorImageLSx64\Modalities\*" "%UserProfile%\Documents\ThorImageLS 4.4\Modalities" /s

) else (
 REM folder already exists. No need to copy
)

echo F | xcopy /F ..\Library\*.* .\bin\%BUILD_CONFIG%

@echo Copying Library TIFF
echo F | xcopy /F ..\..\..\..\Tools\tiff-3.9.4\bin\x64\%BUILD_CONFIG%\libtiff3.dll .\bin\%BUILD_CONFIG%

@echo Copying Library Intel IPP
echo F | xcopy /F "..\..\..\..\Tools\Intel IPP\intel64\dll\ippcore-7.0.dll" .\bin\%BUILD_CONFIG%
echo F | xcopy /F "..\..\..\..\Tools\Intel IPP\intel64\dll\ippi-7.0.dll" .\bin\%BUILD_CONFIG%
echo F | xcopy /F "..\..\..\..\Tools\Intel IPP\intel64\dll\ippiu8-7.0.dll" .\bin\%BUILD_CONFIG%
echo F | xcopy /F "..\..\..\..\Tools\Intel IPP\intel64\dll\libiomp5md.dll" .\bin\%BUILD_CONFIG%
echo F | xcopy /F "..\..\..\..\Tools\Intel IPP\intel64\dll\ippcvu8-7.0.dll" .\bin\%BUILD_CONFIG%
echo F | xcopy /F "..\..\..\..\Tools\Intel IPP\intel64\dll\ippm-7.0.dll" .\bin\%BUILD_CONFIG%
echo F | xcopy /F "..\..\..\..\Tools\Intel IPP\intel64\dll\ippmm7-7.0.dll" .\bin\%BUILD_CONFIG%
echo F | xcopy /F "..\..\..\..\Tools\Intel IPP\intel64\dll\ipps-7.0.dll" .\bin\%BUILD_CONFIG%
echo F | xcopy /F "..\..\..\..\Tools\Intel IPP\intel64\dll\ippsy8-7.0.dll" .\bin\%BUILD_CONFIG%

@echo Copying Library WPF toolkit
echo F | xcopy /F ..\..\..\..\Tools\WPFToolkit\v3.5.40619.1\WPFToolkit.dll .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Tools\Colorpickerlib\bin\%BUILD_CONFIG%\ColorPicker.dll .\bin\%BUILD_CONFIG%

@echo Copying Library AVI
echo F | xcopy /F "..\..\..\..\Tools\AviFile\bin\%BUILD_CONFIG%\AviFile.dll" .\bin\%BUILD_CONFIG%

@echo Copying Library LibTiff.NET40
echo F | xcopy /F "..\..\..\..\Tools\LibTiff.Net40\%BUILD_CONFIG%\BitMiracle.LibTiff.NET40.dll" .\bin\%BUILD_CONFIG%
echo F | xcopy /F "..\..\..\..\Tools\LibTiff.Net40\%BUILD_CONFIG%\BitMiracle.LibTiff.NET40.xml" .\bin\%BUILD_CONFIG%

@echo Copying Library Microsoft.Xaml.Behaviors
echo F | xcopy /F "..\..\..\..\Tools\Microsoft.Xaml.Behaviors\Microsoft.Xaml.Behaviors.dll" .\bin\%BUILD_CONFIG%
echo F | xcopy /F "..\..\..\..\Tools\Microsoft.Xaml.Behaviors\Microsoft.Xaml.Behaviors.xml" .\bin\%BUILD_CONFIG%

REM @echo Copying Library uart for Kurios
REM echo F | xcopy /F "..\..\..\..\Tools\uartlibrary\x64\uart_library_ftdi64.dll" .\bin\%BUILD_CONFIG%

@echo Copying Library Gong
echo F | xcopy /F "..\..\..\..\Tools\Gong\gong-wpf-dragdrop-master\GongSolutions.Wpf.DragDrop\bin\%BUILD_CONFIG%\NET4\GongSolutions.Wpf.DragDrop.dll" .\bin\%BUILD_CONFIG%\Modules

@echo Copying Library MahApps.Metro
echo F | xcopy /F "..\..\..\..\Tools\MahApps.Metro\MahApps.Metro.1.5.0\lib\net45\MahApps.Metro.dll" .\bin\%BUILD_CONFIG%

@echo Copying Library Mini Circuits - Switch box library built in .NET 4.5
echo F | xcopy /F "..\..\..\..\Tools\Mini-Circuits\RF switch controller\mcl_RF_Switch_Controller_NET45.dll" .\bin\%BUILD_CONFIG%

@echo Copying Library Infragistics
echo F | xcopy /F "..\..\..\..\Tools\Infragistics\Infragistics.WPF.16.2.20162.2141\lib\InfragisticsWPF.dll" .\bin\%BUILD_CONFIG%
echo F | xcopy /F "..\..\..\..\Tools\Infragistics\Infragistics.WPF.Controls.Editors.XamMaskedInput.16.2.20162.2141\lib\InfragisticsWPF.Controls.Editors.XamMaskedInput.dll" .\bin\%BUILD_CONFIG%
echo F | xcopy /F "..\..\..\..\Tools\Infragistics\Infragistics.WPF.Controls.Editors.XamSlider.16.2.20162.2141\lib\InfragisticsWPF.Controls.Editors.XamSlider.dll" .\bin\%BUILD_CONFIG%

@echo Copying Library Scichart
echo F | xcopy /F "..\..\..\..\Tools\SciChart v8.x\Lib\net462\SciChart.Charting.dll" .\bin\%BUILD_CONFIG%
echo F | xcopy /F "..\..\..\..\Tools\SciChart v8.x\Lib\net462\SciChart.Charting.DrawingTools.dll" .\bin\%BUILD_CONFIG%
echo F | xcopy /F "..\..\..\..\Tools\SciChart v8.x\Lib\net462\SciChart.Charting3D.dll" .\bin\%BUILD_CONFIG%
echo F | xcopy /F "..\..\..\..\Tools\SciChart v8.x\Lib\net462\SciChart.Core.dll" .\bin\%BUILD_CONFIG%
echo F | xcopy /F "..\..\..\..\Tools\SciChart v8.x\Lib\net462\SciChart.Data.dll" .\bin\%BUILD_CONFIG%
echo F | xcopy /F "..\..\..\..\Tools\SciChart v8.x\Lib\net462\SciChart.Drawing.dll" .\bin\%BUILD_CONFIG%
echo F | xcopy /F "..\..\..\..\Tools\SciChart v8.x\Lib\net462\SciChart.Drawing.DirectX.dll" .\bin\%BUILD_CONFIG%

@echo Copying FLIM fitting library
echo F | xcopy /F ..\..\..\..\Tools\FLIMFit\x64\%BUILD_CONFIG%\FLIMFitLibrary.dll .\bin\%BUILD_CONFIG%\Lib

REM @echo Copying Library for Chrolis
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorChrolis\Driver\TL6WL\bin\TL6WL_64.dll .\bin\%BUILD_CONFIG%\Lib

REM @echo Copying Curve fitting library
echo F | xcopy /F ..\..\..\..\Tools\CurveFitting\bin\%BUILD_CONFIG%\CurveFitting.dll .\bin\%BUILD_CONFIG%\Modules

@echo Copying Local Libraries
echo F | xcopy /F ..\ThorImageInfastructure\bin\%BUILD_CONFIG%\ThorImageInfastructure.dll .\bin\%BUILD_CONFIG%\Modules

echo F | xcopy /F ..\..\..\Controls\FolderDialogControl\FolderDialogControl\bin\%BUILD_CONFIG%\FolderDialogControl.dll .\bin\%BUILD_CONFIG%\Modules

echo F | xcopy /F ..\..\..\Controls\ImageReview\ImageReview\bin\%BUILD_CONFIG%\ImageReview.dll .\bin\%BUILD_CONFIG%\Modules

echo F | xcopy /F ..\..\..\Controls\MenuModuleLS\MenuModuleLS\bin\%BUILD_CONFIG%\MenuModuleLS.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\Controls\MenuModuleLS\MenuModuleLS\bin\%BUILD_CONFIG%\en-US\MenuModuleLS.resources.dll .\bin\%BUILD_CONFIG%\en-US\MenuModuleLS.resources.dll

echo F | xcopy /F ..\..\..\Controls\LogFileWindow\LogFileWindow\bin\%BUILD_CONFIG%\LogFileWindow.dll .\bin\%BUILD_CONFIG%\Modules

echo F | xcopy /F ..\..\..\..\Common\IPCModules\IPCModules\bin\%BUILD_CONFIG%\IPCModules.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\Controls\ScriptManager\ScriptManager\bin\%BUILD_CONFIG%\ScriptManager.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\Controls\ScriptManager\ScriptFijiProcessing\bin\%BUILD_CONFIG%\ScriptFijiProcessing.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\Controls\FijiLauncher\FijiLauncher\bin\%BUILD_CONFIG%\FijiLauncher.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\Controls\FijiLauncher\FijiLauncherPanel\bin\%BUILD_CONFIG%\FijiLauncherPanel.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\Controls\ScriptPanels\SetScriptPath\SetScriptPath\bin\%BUILD_CONFIG%\SetScriptPath.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\Controls\ScriptPanels\ForLoop\ForLoop\bin\%BUILD_CONFIG%\ForLoop.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\Controls\ScriptPanels\ScriptWait\ScriptWait\bin\%BUILD_CONFIG%\ScriptWait.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\Controls\ScriptPanels\ScriptXYMove\ScriptXYMove\bin\%BUILD_CONFIG%\ScriptXYMove.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\Controls\ScriptPanels\ScriptZMove\ScriptZMove\bin\%BUILD_CONFIG%\ScriptZMove.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\Controls\ScriptPanels\ScriptZ2Move\ScriptZ2Move\bin\%BUILD_CONFIG%\ScriptZ2Move.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\ExperimentSettingsViewer\ExperimentSettingsViewer\bin\%BUILD_CONFIG%\ExperimentSettingsViewer.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\ExperimentSettingsBrowser\ExperimentSettingsBrowser\bin\%BUILD_CONFIG%\ExperimentSettingsBrowser.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\Validations\Validations\bin\%BUILD_CONFIG%\Validations.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\OverlayManager\OverlayManager\bin\%BUILD_CONFIG%\OverlayManager.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\MultiROIStats\MultiROIStats\bin\%BUILD_CONFIG%\MultiROIStats.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\LineProfileWindow\LineProfileWindow\bin\%BUILD_CONFIG%\LineProfileWindow.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\MessageBoxCheck\MessageBoxCheck\bin\%BUILD_CONFIG%\MessageBoxCheck.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\PowerControl\PowerControl\bin\%BUILD_CONFIG%\PowerControl.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\AreaControl\AreaControl\bin\%BUILD_CONFIG%\AreaControl.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\ZControl\ZControl\bin\%BUILD_CONFIG%\ZControl.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\ScanControl\ScanControl\bin\%BUILD_CONFIG%\ScanControl.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\OTMControl\OTMControl\bin\%BUILD_CONFIG%\OTMControl.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\SLMControl\SLMControl\bin\%BUILD_CONFIG%\SLMControl.dll .\bin\%BUILD_CONFIG%\Modules
REM echo F | xcopy /F ..\..\..\..\GUI\Controls\LaserControl\LaserControl\bin\%BUILD_CONFIG%\LaserControl.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\MultiphotonControl\MultiphotonControl\bin\%BUILD_CONFIG%\MultiphotonControl.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\CameraControl\CameraControl\bin\%BUILD_CONFIG%\CameraControl.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\LampControl\LampControl\bin\%BUILD_CONFIG%\LampControl.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\PinholeControl\PinholeControl\bin\%BUILD_CONFIG%\PinholeControl.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\CaptureOptionsControl\CaptureOptionsControl\bin\%BUILD_CONFIG%\CaptureOptionsControl.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\QuickTemplatesControl\QuickTemplatesControl\bin\%BUILD_CONFIG%\QuickTemplatesControl.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\ThreePhotonControl\ThreePhotonControl\bin\%BUILD_CONFIG%\ThreePhotonControl.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\LightEngineControl\LightEngineControl\bin\%BUILD_CONFIG%\LightEngineControl.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\EpiTurretControl\EpiTurretControl\bin\%BUILD_CONFIG%\EpiTurretControl.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\RemoteIPCControl\RemoteIPCControl\bin\%BUILD_CONFIG%\RemoteIPCControl.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\DFLIMControl\DFLIMControl\bin\%BUILD_CONFIG%\DFLIMControl.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\DFLIMSetupAssistant\DFLIMSetupAssistant\bin\%BUILD_CONFIG%\DFLIMSetupAssistant.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\FLIMFitting\FLIMFitting\bin\%BUILD_CONFIG%\FLIMFitting.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\AutoFocusControl\AutoFocusControl\bin\%BUILD_CONFIG%\AutoFocusControl.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\MiniCircuitsSwitchControl\MiniCircuitsSwitchControl\bin\%BUILD_CONFIG%\MiniCircuitsSwitchControl.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\MultiLaserControl\MultiLaserControl\bin\%BUILD_CONFIG%\MultiLaserControl.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\ImageViewControl\ImageViewControl\bin\%BUILD_CONFIG%\ImageViewControl.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\SuffixedTextBoxControl\SuffixedTextBoxControl\bin\%BUILD_CONFIG%\SuffixedTextBoxControl.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\TextBoxWithFocusButtonControl\bin\%BUILD_CONFIG%\TextBoxWithFocusButtonControl.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\UpDownButtonsControl\bin\%BUILD_CONFIG%\UpDownButtonsControl.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\LightPathControl\LightPathControl\bin\%BUILD_CONFIG%\LightPathControl.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\SequentialControl\SequentialControl\bin\%BUILD_CONFIG%\SequentialControl.dll .\bin\%BUILD_CONFIG%\Modules

@echo Copying Library MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\PowerControlMVM\PowerControlMVM\bin\%BUILD_CONFIG%\PowerControlMVM.dll .\bin\%BUILD_CONFIG%\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\AreaControlMVM\AreaControlMVM\bin\%BUILD_CONFIG%\AreaControlMVM.dll .\bin\%BUILD_CONFIG%\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\ZControlMVM\ZControlMVM\bin\%BUILD_CONFIG%\ZControlMVM.dll .\bin\%BUILD_CONFIG%\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\ScanControlMVM\ScanControlMVM\bin\%BUILD_CONFIG%\ScanControlMVM.dll .\bin\%BUILD_CONFIG%\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\OTMControlMVM\OTMControlMVM\bin\%BUILD_CONFIG%\OTMControlMVM.dll .\bin\%BUILD_CONFIG%\Modules\MVM
REM echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\LaserControlMVM\LaserControlMVM\bin\%BUILD_CONFIG%\LaserControlMVM.dll .\bin\%BUILD_CONFIG%\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\MultiphotonControlMVM\MultiphotonControlMVM\bin\%BUILD_CONFIG%\MultiphotonControlMVM.dll .\bin\%BUILD_CONFIG%\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\DigitalOutputSwitchesMVM\DigitalOutputSwitchesMVM\bin\%BUILD_CONFIG%\DigitalOutputSwitchesMVM.dll .\bin\%BUILD_CONFIG%\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\CameraControlMVM\CameraControlMVM\bin\%BUILD_CONFIG%\CameraControlMVM.dll .\bin\%BUILD_CONFIG%\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\LampControlMVM\LampControlMVM\bin\%BUILD_CONFIG%\LampControlMVM.dll .\bin\%BUILD_CONFIG%\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\PinholeControlMVM\PinholeControlMVM\bin\%BUILD_CONFIG%\PinholeControlMVM.dll .\bin\%BUILD_CONFIG%\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\QuickTemplatesControlMVM\QuickTemplatesControlMVM\bin\%BUILD_CONFIG%\QuickTemplatesControlMVM.dll .\bin\%BUILD_CONFIG%\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\XYTileControlMVM\XYTileControlMVM\bin\%BUILD_CONFIG%\XYTileControlMVM.dll .\bin\%BUILD_CONFIG%\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\KuriosControlMVM\KuriosControlMVM\bin\%BUILD_CONFIG%\KuriosControlMVM.dll .\bin\%BUILD_CONFIG%\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\CaptureOptionsControlMVM\CaptureOptionsControlMVM\bin\%BUILD_CONFIG%\CaptureOptionsControlMVM.dll .\bin\%BUILD_CONFIG%\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\ThreePhotonControlMVM\ThreePhotonControlMVM\bin\%BUILD_CONFIG%\ThreePhotonControlMVM.dll .\bin\%BUILD_CONFIG%\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\LightEngineControlMVM\LightEngineControlMVM\bin\%BUILD_CONFIG%\LightEngineControlMVM.dll .\bin\%BUILD_CONFIG%\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\ObjectiveControlMVM\ObjectiveControlMVM\bin\%BUILD_CONFIG%\ObjectiveControlMVM.dll .\bin\%BUILD_CONFIG%\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\RemoteIPCControlMVM\RemoteIPCControlMVM\bin\%BUILD_CONFIG%\RemoteIPCControlMVM.dll .\bin\%BUILD_CONFIG%\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\DFLIMControlMVM\DFLIMControlMVM\bin\%BUILD_CONFIG%\DFLIMControlMVM.dll .\bin\%BUILD_CONFIG%\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\EpiTurretControlMVM\EpiTurretControlMVM\bin\%BUILD_CONFIG%\EpiTurretControlMVM.dll .\bin\%BUILD_CONFIG%\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\MesoScanMVM\MesoScanMVM\bin\%BUILD_CONFIG%\MesoScanMVM.dll .\bin\%BUILD_CONFIG%\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\AutoFocusControlMVM\AutoFocusControlMVM\bin\%BUILD_CONFIG%\AutoFocusControlMVM.dll .\bin\%BUILD_CONFIG%\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\MiniCircuitsSwitchControlMVM\MiniCircuitsSwitchControlMVM\bin\%BUILD_CONFIG%\MiniCircuitsSwitchControlMVM.dll .\bin\%BUILD_CONFIG%\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\MultiLaserControlMVM\MultiLaserControlMVM\bin\%BUILD_CONFIG%\MultiLaserControlMVM.dll .\bin\%BUILD_CONFIG%\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\ImageViewMVM\ImageViewMVM\bin\%BUILD_CONFIG%\ImageViewMVM.dll .\bin\%BUILD_CONFIG%\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\LightPathControlMVM\LightPathControlMVM\bin\%BUILD_CONFIG%\LightPathControlMVM.dll .\bin\%BUILD_CONFIG%\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\SequentialControlMVM\SequentialControlMVM\bin\%BUILD_CONFIG%\SequentialControlMVM.dll .\bin\%BUILD_CONFIG%\Modules\MVM

echo F | xcopy /F ..\..\..\..\GUI\Controls\ImageViewControl\ImageViewControl\bin\%BUILD_CONFIG%\System.Windows.Interactivity.dll .\bin\%BUILD_CONFIG%

echo F | xcopy /F ..\..\..\..\Common\Database\DatabaseInterface\bin\%BUILD_CONFIG%\DatabaseInterface.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\Common\Database\DatabaseInterface\bin\%BUILD_CONFIG%\System.Data.SQLite.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\Common\Database\DatabaseInterface\bin\%BUILD_CONFIG%\x64\SQLite.Interop.dll .\bin\%BUILD_CONFIG%

echo F | xcopy /F ..\..\..\..\Common\GeometryUtilities\GeometryUtilities\bin\%BUILD_CONFIG%\GeometryUtilities.dll .\bin\%BUILD_CONFIG%\Modules

echo F | xcopy /F ..\..\..\Controls\ROIUserControl\bin\%BUILD_CONFIG%\ROIUserControl.dll .\bin\%BUILD_CONFIG%\Modules

echo F | xcopy /F ..\..\..\Controls\ThemeControl\ThemeControl\bin\%BUILD_CONFIG%\ThemeControl.dll .\bin\%BUILD_CONFIG%\Modules

echo F | xcopy /F ..\..\..\..\Tools\DeepZoomGen\DeepZoomGen\bin\%BUILD_CONFIG%\DeepZoomGen.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\Tools\DeepZoomGen\DeepZoomGen\bin\%BUILD_CONFIG%\DeepZoomTools.dll .\bin\%BUILD_CONFIG%\Modules

echo F | xcopy /F ..\..\..\..\Common\ThorSharedTypes\ThorSharedTypes\bin\%BUILD_CONFIG%\ThorSharedTypes.dll .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Common\ThorDiskIO\x64\%BUILD_CONFIG%\ThorDiskIO.dll .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Common\HardwareState\HardwareState\bin\%BUILD_CONFIG%\HardwareState.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\Common\AutoFocusModule\AutoFocusModule\bin\%BUILD_CONFIG%\AutoFocusModule.dll .\bin\%BUILD_CONFIG%\Modules

echo F | xcopy /F ..\..\..\..\GUI\Controls\HistogramControl\HistogramControl\bin\%BUILD_CONFIG%\HistogramControl.dll .\bin\%BUILD_CONFIG%\Modules

echo F | xcopy /F ..\..\..\..\Common\ThorLogging\ThorLogging.xml .\bin\%BUILD_CONFIG%

REM echo F | xcopy /F ..\..\..\..\Tools\log4cxx0.10.0\msvc9-proj\x64\%BUILD_CONFIG%\log4cxx.dll .\bin\%BUILD_CONFIG%

REM echo F | xcopy /F ..\..\..\..\Common\Camera-TSI\dlls\Native_64_lib\logger\* .\bin\%BUILD_CONFIG%

echo F | xcopy /F ..\..\..\..\Common\HelpProvider\HelpProvider\bin\%BUILD_CONFIG%\HelpProvider.dll .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Documents\HelpLS\ThorImageLS.chm .\bin\%BUILD_CONFIG%

::Commands:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

echo F | xcopy /F ..\..\..\..\Commands\General\SelectHardware\x64\%BUILD_CONFIG%\SelectHardware.dll .\bin\%BUILD_CONFIG%\Modules_Native
echo F | xcopy /F ..\..\..\..\Common\GeometryUtilities\x64\%BUILD_CONFIG%\GeometryUtilitiesCPP.dll .\bin\%BUILD_CONFIG%\Modules_Native

REM Uncomment later
echo F | xcopy /F ..\..\..\..\Commands\CaptureSetup\CaptureSetupModule\bin\%BUILD_CONFIG%\CaptureSetupModule.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\Commands\CaptureSetup\x64\%BUILD_CONFIG%\CaptureSetup.dll .\bin\%BUILD_CONFIG%\Modules_Native
echo F | xcopy /F ..\..\..\..\Commands\CaptureSetup\CaptureSetupModule\bin\%BUILD_CONFIG%\en-US\CaptureSetupModule.resources.dll .\bin\%BUILD_CONFIG%\en-US\CaptureSetupModule.resources.dll

echo F | xcopy /F ..\..\..\Controls\RangeSliderWPF\bin\%BUILD_CONFIG%\RangeSliderWPF.dll .\bin\%BUILD_CONFIG%\Modules
REM Uncomment later
echo F | xcopy /F ..\..\..\..\Commands\Acquisition\RunSample\RunSampleLSModule\bin\%BUILD_CONFIG%\RunSampleLSModule.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\Commands\Acquisition\RunSample\x64\%BUILD_CONFIG%\RunSample.dll .\bin\%BUILD_CONFIG%\Modules_Native
echo F | xcopy /F ..\..\..\..\Commands\Acquisition\RunSample\RunSampleLSModule\bin\%BUILD_CONFIG%\en-US\RunSampleLSModule.resources.dll .\bin\%BUILD_CONFIG%\en-US\RunSampleLSModule.resources.dll

echo F | xcopy /F ..\..\..\..\GUI\Controls\HardwareSetupModuleLS\HardwareSetupModuleLS\bin\%BUILD_CONFIG%\HardwareSetupModuleLS.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\HardwareSetupModuleLS\HardwareSetupModuleLS\bin\%BUILD_CONFIG%\en-US\HardwareSetupModuleLS.resources.dll .\bin\%BUILD_CONFIG%\en-US\HardwareSetupModuleLS.resources.dll


echo F | xcopy /F ..\..\..\..\GUI\Controls\XMLViewer\XMLViewer\bin\%BUILD_CONFIG%\XMLViewer.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\XMLViewer\XMLViewer\bin\%BUILD_CONFIG%\en-US\XMLViewer.resources.dll .\bin\%BUILD_CONFIG%\en-US\XMLViewer.resources.dll

echo F | xcopy /F ..\..\..\..\GUI\Controls\SettingsEditor\SettingsEditor\bin\%BUILD_CONFIG%\SettingsEditor.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\SettingsEditor\SettingsEditor\bin\%BUILD_CONFIG%\en-US\SettingsEditor.resources.dll .\bin\%BUILD_CONFIG%\en-US\SettingsEditor.resources.dll

echo F | xcopy /F ..\..\..\..\GUI\Controls\HardwareSetupUserControl\HardwareSetupUserControl\bin\%BUILD_CONFIG%\HardwareSetupUserControl.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\HardwareSetupUserControl\HardwareSetupUserControl\bin\%BUILD_CONFIG%\en-US\HardwareSetupUserControl.resources.dll .\bin\%BUILD_CONFIG%\en-US\HardwareSetupUserControl.resources.dll

echo F | xcopy /F ..\..\..\..\GUI\Controls\SampleRegionSelection\SampleRegionSelection\bin\%BUILD_CONFIG%\SampleRegionSelection.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\SampleRegionSelection\SampleRegionSelection\bin\%BUILD_CONFIG%\en-US\SampleRegionSelection.resources.dll .\bin\%BUILD_CONFIG%\en-US\SampleRegionSelection.resources.dll

echo F | xcopy /F ..\..\..\..\GUI\Controls\AboutModule\AboutModule\bin\%BUILD_CONFIG%\AboutModule.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\Tools\WebUpdater\WebUpdater\bin\%BUILD_CONFIG%\WebUpdater.exe .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Tools\WebUpdater\WebUpdater\bin\%BUILD_CONFIG%\AppLimit.CloudComputing.SharpBox.dll .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Tools\WebUpdater\WebUpdater\bin\%BUILD_CONFIG%\Newtonsoft.Json.Net20.dll .\bin\%BUILD_CONFIG%

echo F | xcopy /F ..\..\..\..\GUI\Controls\Validations\Validations\bin\%BUILD_CONFIG%\Validations.dll .\bin\%BUILD_CONFIG%\Modules

@echo Copying Library ImageTilerControl / Thorlabs_IP_Library
echo F | xcopy /F ..\..\..\..\GUI\Controls\ImageTilerControl\ImageTilerControl\lib\Thorlabs_IP_Library.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\ImageTilerControl\ImageTilerControl\bin\%BUILD_CONFIG%\ImageTilerControl.dll .\bin\%BUILD_CONFIG%\Modules

echo F | xcopy /F ..\..\..\..\GUI\Controls\TilesDisplay\TilesDisplay\bin\%BUILD_CONFIG%\TilesDisplay.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\XYTileControl\XYTileControl\bin\%BUILD_CONFIG%\XYTileControl.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\VtkVolumeControl\VtkVolumeControl64\bin\%BUILD_CONFIG%\VtkVolumeControl.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\VolumeControlInterface\VolumeControlInterface\bin\%BUILD_CONFIG%\VolumeControlInterface.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\SpinnerProgress\SpinnerProgress\bin\%BUILD_CONFIG%\SpinnerProgress.dll .\bin\%BUILD_CONFIG%\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\DigitalOutputSwitches\DigitalOutputSwitches\bin\%BUILD_CONFIG%\DigitalOutputSwitches.dll .\bin\%BUILD_CONFIG%\Modules

echo F | xcopy /F ..\..\..\..\GUI\Controls\JPEGer_Process\JPEGer_Process\\bin\%BUILD_CONFIG%\JPEGer_Process.exe .\bin\%BUILD_CONFIG%\
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorDetectorSwitch\MCLWrapper\MCLWrapper\bin\%BUILD_CONFIG%\mcl_RF_Switch_Controller64.dll .\bin\%BUILD_CONFIG%
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorDetectorSwitch\MCLWrapper\MCLWrapper\bin\%BUILD_CONFIG%\MCLWrapper.dll .\bin\%BUILD_CONFIG%

echo F | xcopy /F ..\..\..\..\Tools\ActiViz-9.x\x64\K*.* .\bin\%BUILD_CONFIG%\Lib
echo F | xcopy /F ..\..\..\..\Tools\ActiViz-9.x\x64\V*.* .\bin\%BUILD_CONFIG%\Lib
move .\bin\%BUILD_CONFIG%\Lib\Kitware.mummy.Runtime.dll .\bin\%BUILD_CONFIG%
move .\bin\%BUILD_CONFIG%\Lib\Kitware.VTK.dll .\bin\%BUILD_CONFIG%
move .\bin\%BUILD_CONFIG%\Lib\Kitware.VTK.vtkFiltering.Unmanaged.dll .\bin\%BUILD_CONFIG%

echo F | xcopy /F ..\..\..\..\GUI\Controls\ROIStatsChart\ROIStatsChart\bin\%BUILD_CONFIG%\ROIStatsChart.dll .\bin\%BUILD_CONFIG%\Modules

echo F | xcopy /F ..\..\..\..\GUI\Controls\KuriosControl\KuriosControl\bin\%BUILD_CONFIG%\KuriosControl.dll .\bin\%BUILD_CONFIG%\Modules

::ModulesNative:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

echo F | xcopy /F ..\..\..\..\Common\ExperimentManager\x64\%BUILD_CONFIG%\ExperimentManager.dll .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Common\ImageManager\x64\%BUILD_CONFIG%\ImageManager.dll .\bin\%BUILD_CONFIG%\
echo F | xcopy /F ..\..\..\..\Common\ResourceManager\x64\%BUILD_CONFIG%\ResourceManager.dll .\bin\%BUILD_CONFIG%\
echo F | xcopy /F ..\..\..\..\Common\RoiDataStore\x64\%BUILD_CONFIG%\RoiDataStore.dll .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Common\ConvertUtilities\x64\%BUILD_CONFIG%\ClassicTiffConverter.dll .\bin\%BUILD_CONFIG%\Modules_Native
echo F | xcopy /F ..\..\..\..\Common\ConvertUtilities\Libs\classic_tiff_library\*.dll .\bin\%BUILD_CONFIG%\Modules_Native
echo F | xcopy /F ..\..\..\..\Common\ConvertUtilities\Libs\ImageStoreLibrary\ome_tiff_library.dll .\bin\%BUILD_CONFIG%\Modules_Native
echo F | xcopy /F ..\..\..\..\Common\ImageStoreLibrary\bin\vs2012\%BUILD_CONFIG%\*.dll .\bin\%BUILD_CONFIG%\Modules_Native
echo F | xcopy /F ..\..\..\..\Common\Sample\x64\%BUILD_CONFIG%\Sample.dll .\bin\%BUILD_CONFIG%\Modules_Native
echo F | xcopy /F ..\..\..\..\Common\HardwareCom\x64\%BUILD_CONFIG%\HardwareCom.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Common\HologramGenerator\x64\%BUILD_CONFIG%\HologramGenerator.dll .\bin\%BUILD_CONFIG%\Modules_Native
echo F | xcopy /F ..\..\..\..\Common\StatsManager\x64\%BUILD_CONFIG%\StatsManager.dll .\bin\%BUILD_CONFIG%
REM echo F | xcopy /F ..\..\..\..\Common\ThorLogging\x64\%BUILD_CONFIG%\ThorLoggingUnmanagedDisabled.dll .\bin\%BUILD_CONFIG%\Modules_Native\ThorLoggingUnmanaged.dll
REM echo F | xcopy /F ..\..\..\..\Common\WinDVI\x64\%BUILD_CONFIG%\WinDVI.dll .\bin\%BUILD_CONFIG%\Modules_Native
echo F | xcopy /F ..\..\..\..\Common\ThorLogging\x64\%BUILD_CONFIG%\ThorLoggingUnmanaged.dll .\bin\%BUILD_CONFIG%\Modules_Native\ThorLoggingUnmanaged.dll
echo F | xcopy /F ..\..\..\..\Common\AutoFocusModule\x64\%BUILD_CONFIG%\AutoFocus.dll .\bin\%BUILD_CONFIG%\Modules_Native
echo F | xcopy /F ..\..\..\..\Common\AutoExposureModule\x64\%BUILD_CONFIG%\AutoExposure.dll .\bin\%BUILD_CONFIG%\Modules_Native
echo F | xcopy /F ..\..\..\..\Common\AutoExposureModule\AutoExposureModule\bin\%BUILD_CONFIG%\AutoExposureModule.dll .\bin\%BUILD_CONFIG%\Modules

echo F | xcopy /F ..\..\..\..\Commands\ImageAnalysis\PincushionCorrection\x64\%BUILD_CONFIG%\PincushionCorrection.dll .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Commands\ImageAnalysis\FlatField\x64\%BUILD_CONFIG%\FlatField.dll .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Commands\ImageAnalysis\LineProfile\x64\%BUILD_CONFIG%\LineProfile.dll .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Commands\ImageAnalysis\ThorImageProcess\x64\%BUILD_CONFIG%\ThorImageProcess.dll .\bin\%BUILD_CONFIG%

echo F | xcopy /F ..\..\..\..\Hardware\Cameras\CameraManager\x64\%BUILD_CONFIG%\CameraManager.dll .\bin\%BUILD_CONFIG%\
echo F | xcopy /F ..\..\..\..\Hardware\Devices\DeviceManager\x64\%BUILD_CONFIG%\DeviceManager.dll .\bin\%BUILD_CONFIG%\

echo F | xcopy /F ..\..\..\..\Tools\HDF5IO\x64\%BUILD_CONFIG%\HDF5IO.dll .\bin\%BUILD_CONFIG%\Modules_Native
echo F | xcopy /F ..\..\..\..\Tools\HDF5IO\HDF5IO\bin\%BUILD_CONFIG%\*.* .\bin\%BUILD_CONFIG%


:: Simulators ::::::::::::::::::::::
echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorConfocalSimulator\x64\%BUILD_CONFIG%\ThorConfocalSimulator.dll .\bin\%BUILD_CONFIG%\Modules_Native
echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorConfocalGalvoSimulator\x64\%BUILD_CONFIG%\ThorConfocalGalvoSimulator.dll .\bin\%BUILD_CONFIG%\Modules_Native
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorZStepperSimulator\x64\%BUILD_CONFIG%\ThorZStepperSimulator.dll .\bin\%BUILD_CONFIG%\Modules_Native
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorPMTSimulator\x64\%BUILD_CONFIG%\ThorPMTSimulator.dll .\bin\%BUILD_CONFIG%\Modules_Native
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorTDCSimulator\x64\%BUILD_CONFIG%\ThorTDCSimulator.dll .\bin\%BUILD_CONFIG%\Modules_Native
echo F | xcopy /F ..\..\..\..\Hardware\Devices\SimDeviceXY\x64\%BUILD_CONFIG%\SimDeviceXY.dll .\bin\%BUILD_CONFIG%\Modules_Native
echo F | xcopy /F ..\..\..\..\Hardware\Devices\SimDeviceFW\x64\%BUILD_CONFIG%\SimDeviceFW.dll .\bin\%BUILD_CONFIG%\Modules_Native
rem echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorBmExpanSimulator\x64\%BUILD_CONFIG%\ThorBmExpanSimulator.dll .\bin\%BUILD_CONFIG%\Modules_Native
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorMCLSSimulator\x64\%BUILD_CONFIG%\ThorMCLSSimulator.dll .\bin\%BUILD_CONFIG%\Modules_Native
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorPinholeStepperSimulator\x64\%BUILD_CONFIG%\ThorPinholeStepperSimulator.dll .\bin\%BUILD_CONFIG%\Modules_Native
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorShutterSimulator\x64\%BUILD_CONFIG%\ThorShutterSimulator.dll .\bin\%BUILD_CONFIG%\Modules_Native
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorLightPathSimulator\x64\%BUILD_CONFIG%\ThorLightPathSimulator.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorMesoScanSimulator\x64\%BUILD_CONFIG%\ThorMesoScanSimulator.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorMesoScanSimulator\TorboJpeg\turbojpeg.dll .\bin\%BUILD_CONFIG%\Lib
REM echo F | xcopy /F ..\..\..\..\Tools\ImageProcessLibrary.1.0.3\build\native\ImageProcessLibrary.dll .\bin\%BUILD_CONFIG%\Lib

::TSI Libraries::::::::::::::::
set INCLUDE_TSI_DLLS=FALSE
if %INCLUDE_TSI_DLLS%==TRUE (
echo F | xcopy /F ..\..\..\..\Common\Camera-TSI\dlls\Native_64_lib\*.dll .\bin\%BUILD_CONFIG%\Lib
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\DCxCamera\x64\%BUILD_CONFIG%\DCxCamera.dll .\bin\%BUILD_CONFIG%\Modules_Native
echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorTSI\x64\%BUILD_CONFIG%\ThorTSI.dll .\bin\%BUILD_CONFIG%\Modules_Native
echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorTSI_CS\x64\%BUILD_CONFIG%\ThorTSI_CS.dll .\bin\%BUILD_CONFIG%\Modules_Native
)

:: Devices ::::::::::::::::::::::::
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\CoherentChameleon\x64\%BUILD_CONFIG%\CoherentChameleon.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\MaiTaiLaser\x64\%BUILD_CONFIG%\MaiTaiLaser.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorBCM\x64\%BUILD_CONFIG%\ThorBCM.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorBmExpan\x64\%BUILD_CONFIG%\ThorBmExpan.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorBeamStabilizer\x64\%BUILD_CONFIG%\ThorBeamStabilizer.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorVBE\x64\%BUILD_CONFIG%\ThorVBE.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorBScope\x64\%BUILD_CONFIG%\ThorBScope.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorConfocal\x64\%BUILD_CONFIG%\ThorConfocal.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorConfocalGalvo\x64\%BUILD_CONFIG%\ThorConfocalGalvo.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorGGNI\x64\%BUILD_CONFIG%\ThorGGNI.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorMesoScan\x64\%BUILD_CONFIG%\ThorMesoScan.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorStim\x64\%BUILD_CONFIG%\ThorStim.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ORCA\x64\%BUILD_CONFIG%\ORCA.dll .\bin\%BUILD_CONFIG%\Modules_Native

REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\thordaq\x64\%BUILD_CONFIG%\thordaq.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\thordaq\x64\%BUILD_CONFIG%\ThorDAQGalvoGalvo.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\thordaq\x64\%BUILD_CONFIG%\ThorDAQResonantGalvo.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThordaqDFLIM\x64\%BUILD_CONFIG%\ThordaqDFLIM.dll .\bin\%BUILD_CONFIG%\Modules_Native
echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThordaqDFLIM\x64\%BUILD_CONFIG%\dFLIM_4002.dll .\bin\%BUILD_CONFIG%\Modules_Native
echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThordaqDFLIM\x64\%BUILD_CONFIG%\dFLIMGG.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThordaqDFLIM\x64\%BUILD_CONFIG%\ThorDFLIMGalvoGalvo.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThordaqDFLIM\x64\%BUILD_CONFIG%\ThorDAQGGDFLIMSim.dll .\bin\%BUILD_CONFIG%\Modules_Native

REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorBCMPA\x64\%BUILD_CONFIG%\ThorBCMPA.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorBCMPA2\x64\%BUILD_CONFIG%\ThorBCMPA2.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorECU\x64\%BUILD_CONFIG%\ThorECU.dll .\bin\%BUILD_CONFIG%\Modules_Native
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorElectroPhys\x64\%BUILD_CONFIG%\ThorElectroPhys.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorEPiTurret\x64\%BUILD_CONFIG%\ThorEpiTurret.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorDC2200\x64\%BUILD_CONFIG%\ThorDC2200.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorDDR05\x64\%BUILD_CONFIG%\ThorDDR05.dll .\bin\%BUILD_CONFIG%\Modules_Native
echo F | xcopy /F ..\..\..\..\Hardware\Devices\DisconnectedDevice\x64\%BUILD_CONFIG%\Disconnected.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorKurios\x64\%BUILD_CONFIG%\ThorKurios.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorMCLS\x64\%BUILD_CONFIG%\ThorMCLS.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorMCM3000\x64\%BUILD_CONFIG%\ThorMCM3000.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorMCM3000Aux\x64\%BUILD_CONFIG%\ThorMCM3000Aux.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorMCM6000\x64\%BUILD_CONFIG%\ThorMCM6000.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorMCM6000_Condenser\x64\%BUILD_CONFIG%\ThorMCM6000_Condenser.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorMCM301\x64\%BUILD_CONFIG%\ThorMCM301.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorMLSStage\x64\%BUILD_CONFIG%\ThorMLSStage.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorObjectiveChanger\x64\%BUILD_CONFIG%\ThorObjectiveChanger.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorPinholeStepper\x64\%BUILD_CONFIG%\ThorPinholeStepper.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorPLSZ\x64\%BUILD_CONFIG%\ThorPLSZ.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorMTS25\x64\%BUILD_CONFIG%\ThorMTS25.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorPMT\x64\%BUILD_CONFIG%\ThorPMT.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorPMT2\x64\%BUILD_CONFIG%\ThorPMT2.dll .\bin\%BUILD_CONFIG%\Modules_Native
 echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorPMT2100\x64\%BUILD_CONFIG%\ThorPMT2100.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorPowerControl\x64\%BUILD_CONFIG%\ThorPowerControl.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorPrelude\x64\%BUILD_CONFIG%\ThorPrelude.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorShutterDigital\x64\%BUILD_CONFIG%\ThorShutterDig.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorShutterDigital2\x64\%BUILD_CONFIG%\ThorShutterDig2.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorShutterDigital3\x64\%BUILD_CONFIG%\ThorShutterDig3.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorShutterDigital4\x64\%BUILD_CONFIG%\ThorShutterDig4.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorShutterDigital5\x64\%BUILD_CONFIG%\ThorShutterDig5.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorShutterDigital6\x64\%BUILD_CONFIG%\ThorShutterDig6.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorTDC\x64\%BUILD_CONFIG%\ThorTDC.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorVBE\x64\%BUILD_CONFIG%\ThorVBE.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorZPiezo\x64\%BUILD_CONFIG%\ThorZPiezo.dll .\bin\%BUILD_CONFIG%\Modules_Native
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorDAQZ\x64\%BUILD_CONFIG%\ThorDAQZ.dll .\bin\%BUILD_CONFIG%\Modules_Native

REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorZStepper\x64\%BUILD_CONFIG%\ThorZStepper.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorSLM\x64\%BUILD_CONFIG%\ThorSLM.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorSLMPDM512\x64\%BUILD_CONFIG%\ThorSLMPDM512.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorSLMPDM512\ThorSLMPDM512\DLL\*.* .\bin\%BUILD_CONFIG%\Lib
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorSLMPDM1024\x64\%BUILD_CONFIG%\ThorSLMPDM1024.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorSLMPDM1024\ThorSLMPDM1024\DLL\*.* .\bin\%BUILD_CONFIG%\Lib
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorTiberius\x64\%BUILD_CONFIG%\ThorTiberius.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\PIPiezoXYZ\x64\%BUILD_CONFIG%\PIPiezo.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\OTMLaser\x64\%BUILD_CONFIG%\OTMLaser.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorChrolis\x64\%BUILD_CONFIG%\ThorChrolis.dll .\bin\%BUILD_CONFIG%\Modules_Native
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorLSKGR\x64\%BUILD_CONFIG%\ThorLSKGR.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorDetector\x64\%BUILD_CONFIG%\ThorDetector.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\TopticaiChrome\x64\%BUILD_CONFIG%\TopticaiChrome.dll .\bin\%BUILD_CONFIG%\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorRemoteFocusNI\x64\%BUILD_CONFIG%\ThorRemoteFocusNI.dll .\bin\%BUILD_CONFIG%\Modules_Native
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorDAQRemoteFocus\x64\%BUILD_CONFIG%\ThorDAQRemoteFocus.dll .\bin\%BUILD_CONFIG%\Modules_Native

:: Settings Files :::::::::::::::::::::
echo F | xcopy /F ..\..\..\..\Hardware\Devices\CoherentChameleon\CoherentChameleonSettings.xml .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Hardware\Devices\MaiTaiLaser\MaiTaiLaserSettings.xml .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorBCM\ThorBCMSettings.xml .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorBeamStabilizer\ThorBeamStabilizerSettings.xml .\bin\%BUILD_CONFIG%\
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorBmExpan\ThorBmExpanSettings.xml .\bin\%BUILD_CONFIG%\
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorBScope\ThorBScopeSettings.xml .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorChrolis\ThorChrolisSettings.xml .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorConfocal\ThorConfocalSettings.xml .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorConfocalGalvo\ThorConfocalGalvoSettings.xml .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorConfocalGalvoSimulator\ThorConfocalGalvoSimulatorSettings.xml .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorConfocalSimulator\ThorConfocalSimulatorSettings.xml .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Installers\ThorImageLSx64\ThorImageLSx64\ThorDetectorSwitchSettings.xml .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorGGNI\ThorGGNISettings.xml .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorMesoScan\ThorMesoScanSettings.xml .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorStim\ThorStimSettings.xml .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorECU\ThorECUSettings.xml .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorEpiTurret\ThorEpiTurretSettings.xml .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\GUI\Applications\ThorImageLS\ThorImage\ThorImageLS.ico .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorMCLS\ThorMCLSSettings.xml .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorMCM3000\ThorMCM3000Settings.xml .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorMCM3000Aux\ThorMCM3000AuxSettings.xml .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorMLSStage\ThorMLSStageSettings.xml .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorObjectiveChanger\ThorObjectiveChangerSettings.xml .\bin\%BUILD_CONFIG%\
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorPinholeStepper\ThorPinholeStepperSettings.xml .\bin\%BUILD_CONFIG%\
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorPMT\ThorPMTSettings.xml .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorPMT2\ThorPMT2Settings.xml .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorPMT2100\ThorPMT2100Settings.xml .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorPowerControl\PowerControlSettings.xml .\bin\%BUILD_CONFIG%\
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorPrelude\ThorPreludeSettings.xml .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorShutterDigital\ThorShutterDigSettings.xml .\bin\%BUILD_CONFIG%\
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorShutterDigital2\ThorShutterDig2Settings.xml .\bin\%BUILD_CONFIG%\
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorShutterDigital3\ThorShutterDig3Settings.xml .\bin\%BUILD_CONFIG%\
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorShutterDigital4\ThorShutterDig4Settings.xml .\bin\%BUILD_CONFIG%\
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorShutterDigital5\ThorShutterDig5Settings.xml .\bin\%BUILD_CONFIG%\
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorShutterDigital6\ThorShutterDig6Settings.xml .\bin\%BUILD_CONFIG%\
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorZPiezo\ThorZPiezoSettings.xml .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorDAQZ\ThorDAQZSettings.xml .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorZStepper\ZStepperSettings.xml .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorVBE\ThorVBESettings.xml .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorElectroPhys\ThorElectroPhysSettings.xml .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorBCMPA\ThorBCMPASettings.xml .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorDDR05\ThorDDR05Settings.xml .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorPLSZ\ThorPLSZSettings.xml .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorMTS25\ThorMTS25Settings.xml .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorSLM\ThorSLMSettings.xml .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorTiberius\ThorTiberiusSettings.xml .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Hardware\Devices\PIPiezoXYZ\PIPiezoSettings.xml .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Hardware\Devices\OTMLaser\OTMLaserSettings.xml .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorLSKGR\ThorLSKGRSettings.xml .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorMCM6000\ThorMCM6000Settings.xml .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorMCM301\ThorMCM301Settings.xml .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorDetector\ThorDetectorSettings.xml .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Hardware\Devices\TopticaiChrome\TopticaiChromeSettings.xml .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorRemoteFocusNI\ThorRemoteFocusNISettings.xml .\bin\%BUILD_CONFIG%

echo F | xcopy /F ..\..\..\..\Hardware\Cameras\thordaq\Dll\ThorDAQResonantGalvo\ThorDAQResonantGalvoSettings.xml .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Hardware\Cameras\thordaq\Dll\ThorDAQGalvoGalvo\ThorDAQGalvoGalvoSettings.xml .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Hardware\Cameras\thordaq\Dll\ThorDAQIOSettings.xml .\bin\%BUILD_CONFIG%
echo F | xcopy /F ..\..\..\..\Hardware\Cameras\thordaq\lib\thordaq\ThorDAQSettings.xml .\bin\%BUILD_CONFIG%

echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThordaqDFLIM\Dll\ThorDFLIMGalvoGalvo\ThorDFLIMGalvoGalvoSettings.xml .\bin\%BUILD_CONFIG%
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThordaqDFLIM\Dll\ThorDAQGGDFLIMSim\fullFrameSim.bin .\bin\%BUILD_CONFIG%

echo F | xcopy /F ..\..\..\..\Common\ResourceManager\ResourceManager.xml .\bin\%BUILD_CONFIG%\

:: Experiment Review:::::::::::::::::::
echo F | xcopy /F ..\..\ExperimentReview\ExperimentReview\bin\%BUILD_CONFIG%\ExperimentReview.exe .\bin\%BUILD_CONFIG%\
echo F | xcopy /F ..\..\ExperimentReview\ExperimentReview\bin\%BUILD_CONFIG%\ExperimentReview.exe.config .\bin\%BUILD_CONFIG%\
echo F | xcopy /F ..\..\ExperimentReview\ExperimentReview\bin\%BUILD_CONFIG%\ExperimentReview.vshost.exe .\bin\%BUILD_CONFIG%\
echo F | xcopy /F ..\..\ExperimentReview\ExperimentReview\ExperimentReviewSettings.xml .\bin\%BUILD_CONFIG%\

:: ThorDAQ Config Tool
echo F | xcopy /F ..\..\..\Controls\ThorDAQConfigControl\Tools\Telerik\* .\bin\%BUILD_CONFIG%\
echo F | xcopy /F ..\..\..\Controls\ThorDAQConfigControl\ThorDAQConfigControl\bin\%BUILD_CONFIG%\ThorDAQConfigControl.dll .\bin\%BUILD_CONFIG%\
echo F | xcopy /F ..\..\ThorDAQConfigTool\ThorDAQConfigTool\bin\%BUILD_CONFIG%\ThorDAQConfigTool.exe .\bin\%BUILD_CONFIG%\
echo F | xcopy /F ..\..\..\..\GUI\Controls\ThorDAQConfigControl\ThorDAQConfigControl\ThorDAQNewSettings.xml .\bin\%BUILD_CONFIG%\
echo F | xcopy /F ..\..\..\..\GUI\Controls\ThorDAQConfigControl\Tools\TIDP.SAA2.dll .\bin\%BUILD_CONFIG%\

:: Matlab Script:::::::::::::::::::::
REM echo F | xcopy /F ..\..\..\..\Common\MatlabEngine\RunTime\mclcom9_1.dll .\bin\%BUILD_CONFIG%\
REM echo F | xcopy /F ..\..\..\..\Common\MatlabEngine\RunTime\mclcommain9_1.dll .\bin\%BUILD_CONFIG%\
REM echo F | xcopy /F ..\..\..\..\Common\MatlabEngine\RunTime\mclmcrrt9_1.dll .\bin\%BUILD_CONFIG%\
REM echo F | xcopy /F ..\..\..\..\Common\MatlabEngine\RunTime\mclxlmain9_1.dll .\bin\%BUILD_CONFIG%\
REM echo F | xcopy /F ..\..\..\..\Common\MatlabEngine\x64\%BUILD_CONFIG%\NativeMatlabEngine.dll .\bin\%BUILD_CONFIG%\
echo F | xcopy /F ..\..\..\..\Common\MatlabEngine\MatlabEngine\bin\%BUILD_CONFIG%\MatlabEngine.dll .\bin\%BUILD_CONFIG%\Modules\
echo F | xcopy /F ..\..\..\Controls\ScriptPanels\ScriptMatlab\ScriptMatlab\bin\%BUILD_CONFIG%\ScriptMatlab.dll .\bin\%BUILD_CONFIG%\Modules\
echo F | xcopy /F ..\..\..\Controls\ScriptManager\ScriptRunMatlabModule\bin\%BUILD_CONFIG%\ScriptRunMatlabModule.dll .\bin\%BUILD_CONFIG%\Modules\