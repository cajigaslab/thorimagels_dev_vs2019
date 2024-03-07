del /Q ".\bin\Release\Modules\MVM\*.*"

del /Q ".\bin\Release\Modules\*.*"

del /Q ".\bin\Release\Modules_Native\*.*"

del /Q ".\bin\Release\Lib\*.*"

del /Q ".\bin\Release\en-US\*.*"

del /Q ".\bin\Release\ru-ru\*.*"

del /Q ".\bin\Release\*.*"

mkdir ".\bin\Release\Modules\"

mkdir ".\bin\Release\Modules\MVM"

mkdir ".\bin\Release\Modules_Native\"

mkdir ".\bin\Release\Lib\"

mkdir ".\bin\Release\en-US\"

mkdir ".\bin\Release\ru-ru\"

@echo Copying User Folders
mkdir "%UserProfile%\Documents\ThorImageLS 4.2"
echo F | xcopy /D "..\..\..\..\Installers\ThorImageLSx64\ThorImageLSx64\*" "%UserProfile%\Documents\ThorImageLS 4.2" /s

echo F | xcopy /F ..\Library\*.* .\bin\Release

@echo Copying Library TIFF
echo F | xcopy /F ..\..\..\..\Tools\tiff-3.9.4\bin\x64\Release\libtiff3.dll .\bin\Release

@echo Copying Library Intel IPP
echo F | xcopy /F "..\..\..\..\Tools\Intel IPP\intel64\dll\ippcore-7.0.dll" .\bin\Release
echo F | xcopy /F "..\..\..\..\Tools\Intel IPP\intel64\dll\ippi-7.0.dll" .\bin\Release
echo F | xcopy /F "..\..\..\..\Tools\Intel IPP\intel64\dll\ippiu8-7.0.dll" .\bin\Release
echo F | xcopy /F "..\..\..\..\Tools\Intel IPP\intel64\dll\libiomp5md.dll" .\bin\Release
echo F | xcopy /F "..\..\..\..\Tools\Intel IPP\intel64\dll\ippcvu8-7.0.dll" .\bin\Release
echo F | xcopy /F "..\..\..\..\Tools\Intel IPP\intel64\dll\ippm-7.0.dll" .\bin\Release
echo F | xcopy /F "..\..\..\..\Tools\Intel IPP\intel64\dll\ippmm7-7.0.dll" .\bin\Release
echo F | xcopy /F "..\..\..\..\Tools\Intel IPP\intel64\dll\ipps-7.0.dll" .\bin\Release
echo F | xcopy /F "..\..\..\..\Tools\Intel IPP\intel64\dll\ippsy8-7.0.dll" .\bin\Release

@echo Copying Library WPF toolkit
echo F | xcopy /F ..\..\..\..\Tools\WPFToolkit\v3.5.40619.1\WPFToolkit.dll .\bin\Release
echo F | xcopy /F ..\..\..\..\Tools\Colorpickerlib\bin\Release\ColorPicker.dll .\bin\Release

@echo Copying Library AVI
echo F | xcopy /F "..\..\..\..\Tools\AviFile\bin\Release\AviFile.dll" .\bin\Release

@echo Copying Library LibTiff.NET40
echo F | xcopy /F "..\..\..\..\Tools\LibTiff.Net40\Debug\BitMiracle.LibTiff.NET40.dll" .\bin\Release
echo F | xcopy /F "..\..\..\..\Tools\LibTiff.Net40\Debug\BitMiracle.LibTiff.NET40.xml" .\bin\Release

@echo Copying Library Microsoft.Xaml.Behaviors
echo F | xcopy /F "..\..\..\..\Tools\Microsoft.Xaml.Behaviors\Microsoft.Xaml.Behaviors.dll" .\bin\Release
echo F | xcopy /F "..\..\..\..\Tools\Microsoft.Xaml.Behaviors\Microsoft.Xaml.Behaviors.xml" .\bin\Release

REM @echo Copying Library uart for kurios
REM echo F | xcopy /F "..\..\..\..\Tools\uartlibrary\x64\uart_library_ftdi64.dll" .\bin\Release

@echo Copying Library Gong
echo F | xcopy /F "..\..\..\..\Tools\Gong\gong-wpf-dragdrop-master\GongSolutions.Wpf.DragDrop\bin\Debug\NET4\GongSolutions.Wpf.DragDrop.dll" .\bin\Release\Modules

@echo Copying Library MahApps.Metro
echo F | xcopy /F "..\..\..\..\Tools\MahApps.Metro\MahApps.Metro.1.5.0\lib\net45\MahApps.Metro.dll" .\bin\Release

@echo Copying Library Mini Circuits - Switch box library built in .NET 4.5
echo F | xcopy /F "..\..\..\..\Tools\Mini-Circuits\RF switch controller\mcl_RF_Switch_Controller_NET45.dll" .\bin\Release

@echo Copying Library Infragistics
echo F | xcopy /F "..\..\..\..\Tools\Infragistics\Infragistics.WPF.16.2.20162.2141\lib\InfragisticsWPF.dll" .\bin\Release
echo F | xcopy /F "..\..\..\..\Tools\Infragistics\Infragistics.WPF.Controls.Editors.XamMaskedInput.16.2.20162.2141\lib\InfragisticsWPF.Controls.Editors.XamMaskedInput.dll" .\bin\Release
echo F | xcopy /F "..\..\..\..\Tools\Infragistics\Infragistics.WPF.Controls.Editors.XamSlider.16.2.20162.2141\lib\InfragisticsWPF.Controls.Editors.XamSlider.dll" .\bin\Release

@echo Copying Library Scichart
echo F | xcopy /F "..\..\..\..\Tools\SciChart v6.x\Lib\net452\SciChart.Charting.dll" .\bin\Release
echo F | xcopy /F "..\..\..\..\Tools\SciChart v6.x\Lib\net452\SciChart.Charting.DrawingTools.dll" .\bin\Release
echo F | xcopy /F "..\..\..\..\Tools\SciChart v6.x\Lib\net452\SciChart.Charting3D.dll" .\bin\Release
echo F | xcopy /F "..\..\..\..\Tools\SciChart v6.x\Lib\net452\SciChart.Core.dll" .\bin\Release
echo F | xcopy /F "..\..\..\..\Tools\SciChart v6.x\Lib\net452\SciChart.Data.dll" .\bin\Release
echo F | xcopy /F "..\..\..\..\Tools\SciChart v6.x\Lib\net452\SciChart.Drawing.dll" .\bin\Release
echo F | xcopy /F "..\..\..\..\Tools\SciChart v6.x\Lib\net452\SciChart.Drawing.DirectX.dll" .\bin\Release

@echo Copying FLIM fitting library
echo F | xcopy /F ..\..\..\..\Tools\FLIMFit\x64\Release\FLIMFitLibrary.dll .\bin\Release\Lib

REM @echo Copying Library for Chrolis
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorChrolis\Driver\TL6WL\bin\TL6WL_64.dll .\bin\Release\Lib

@echo Copying Local Libraries
echo F | xcopy /F ..\ThorImageInfastructure\bin\Release\ThorImageInfastructure.dll .\bin\Release\Modules

echo F | xcopy /F ..\..\..\Controls\FolderDialogControl\FolderDialogControl\bin\Release\FolderDialogControl.dll .\bin\Release\Modules

echo F | xcopy /F ..\..\..\Controls\ImageReview\ImageReview\bin\Release\ImageReview.dll .\bin\Release\Modules

echo F | xcopy /F ..\..\..\Controls\MenuModuleLS\MenuModuleLS\bin\Release\MenuModuleLS.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\Controls\MenuModuleLS\MenuModuleLS\bin\Release\en-US\MenuModuleLS.resources.dll .\bin\Release\en-US\MenuModuleLS.resources.dll

echo F | xcopy /F ..\..\..\Controls\LogFileWindow\LogFileWindow\bin\Release\LogFileWindow.dll .\bin\Release\Modules

echo F | xcopy /F ..\..\..\..\Common\IPCModules\IPCModules\bin\Release\IPCModules.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\Controls\ScriptManager\ScriptManager\bin\Release\ScriptManager.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\Controls\ScriptManager\ScriptFijiProcessing\bin\Release\ScriptFijiProcessing.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\Controls\FijiLauncher\FijiLauncher\bin\Release\FijiLauncher.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\Controls\FijiLauncher\FijiLauncherPanel\bin\Release\FijiLauncherPanel.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\Controls\ScriptPanels\SetScriptPath\SetScriptPath\bin\Release\SetScriptPath.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\Controls\ScriptPanels\ForLoop\ForLoop\bin\Release\ForLoop.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\Controls\ScriptPanels\ScriptWait\ScriptWait\bin\Release\ScriptWait.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\Controls\ScriptPanels\ScriptXYMove\ScriptXYMove\bin\Release\ScriptXYMove.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\Controls\ScriptPanels\ScriptZMove\ScriptZMove\bin\Release\ScriptZMove.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\Controls\ScriptPanels\ScriptZ2Move\ScriptZ2Move\bin\Release\ScriptZ2Move.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\ExperimentSettingsViewer\ExperimentSettingsViewer\bin\Release\ExperimentSettingsViewer.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\ExperimentSettingsBrowser\ExperimentSettingsBrowser\bin\Release\ExperimentSettingsBrowser.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\Validations\Validations\bin\Release\Validations.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\OverlayManager\OverlayManager\bin\Release\OverlayManager.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\MultiROIStats\MultiROIStats\bin\Release\MultiROIStats.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\LineProfileWindow\LineProfileWindow\bin\Release\LineProfileWindow.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\MessageBoxCheck\MessageBoxCheck\bin\Release\MessageBoxCheck.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\PowerControl\PowerControl\bin\Release\PowerControl.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\AreaControl\AreaControl\bin\Release\AreaControl.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\ZControl\ZControl\bin\Release\ZControl.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\ScanControl\ScanControl\bin\Release\ScanControl.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\OTMControl\OTMControl\bin\Release\OTMControl.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\LaserControl\LaserControl\bin\Release\LaserControl.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\MultiphotonControl\MultiphotonControl\bin\Release\MultiphotonControl.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\CameraControl\CameraControl\bin\Release\CameraControl.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\LampControl\LampControl\bin\Release\LampControl.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\PinholeControl\PinholeControl\bin\Release\PinholeControl.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\CaptureOptionsControl\CaptureOptionsControl\bin\Release\CaptureOptionsControl.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\QuickTemplatesControl\QuickTemplatesControl\bin\Release\QuickTemplatesControl.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\ThreePhotonControl\ThreePhotonControl\bin\Release\ThreePhotonControl.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\LightEngineControl\LightEngineControl\bin\Release\LightEngineControl.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\EpiTurretControl\EpiTurretControl\bin\Release\EpiTurretControl.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\RemoteIPCControl\RemoteIPCControl\bin\Release\RemoteIPCControl.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\DFLIMControl\DFLIMControl\bin\Release\DFLIMControl.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\DFLIMSetupAssistant\DFLIMSetupAssistant\bin\Release\DFLIMSetupAssistant.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\FLIMFitting\FLIMFitting\bin\Release\FLIMFitting.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\AutoFocusControl\AutoFocusControl\bin\Release\AutoFocusControl.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\MiniCircuitsSwitchControl\MiniCircuitsSwitchControl\bin\Release\MiniCircuitsSwitchControl.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\MultiLaserControl\MultiLaserControl\bin\Release\MultiLaserControl.dll .\bin\Release\Modules

@echo Copying Library MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\PowerControlMVM\PowerControlMVM\bin\Release\PowerControlMVM.dll .\bin\Release\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\AreaControlMVM\AreaControlMVM\bin\Release\AreaControlMVM.dll .\bin\Release\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\ZControlMVM\ZControlMVM\bin\Release\ZControlMVM.dll .\bin\Release\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\ScanControlMVM\ScanControlMVM\bin\Release\ScanControlMVM.dll .\bin\Release\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\OTMControlMVM\OTMControlMVM\bin\Release\OTMControlMVM.dll .\bin\Release\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\LaserControlMVM\LaserControlMVM\bin\Release\LaserControlMVM.dll .\bin\Release\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\MultiphotonControlMVM\MultiphotonControlMVM\bin\Release\MultiphotonControlMVM.dll .\bin\Release\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\DigitalOutputSwitchesMVM\DigitalOutputSwitchesMVM\bin\Release\DigitalOutputSwitchesMVM.dll .\bin\Release\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\CameraControlMVM\CameraControlMVM\bin\Release\CameraControlMVM.dll .\bin\Release\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\LampControlMVM\LampControlMVM\bin\Release\LampControlMVM.dll .\bin\Release\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\PinholeControlMVM\PinholeControlMVM\bin\Release\PinholeControlMVM.dll .\bin\Release\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\QuickTemplatesControlMVM\QuickTemplatesControlMVM\bin\Release\QuickTemplatesControlMVM.dll .\bin\Release\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\XYTileControlMVM\XYTileControlMVM\bin\Release\XYTileControlMVM.dll .\bin\Release\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\KuriosControlMVM\KuriosControlMVM\bin\Release\KuriosControlMVM.dll .\bin\Release\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\CaptureOptionsControlMVM\CaptureOptionsControlMVM\bin\Release\CaptureOptionsControlMVM.dll .\bin\Release\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\ThreePhotonControlMVM\ThreePhotonControlMVM\bin\Release\ThreePhotonControlMVM.dll .\bin\Release\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\LightEngineControlMVM\LightEngineControlMVM\bin\Release\LightEngineControlMVM.dll .\bin\Release\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\ObjectiveControlMVM\ObjectiveControlMVM\bin\Release\ObjectiveControlMVM.dll .\bin\Release\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\RemoteIPCControlMVM\RemoteIPCControlMVM\bin\Release\RemoteIPCControlMVM.dll .\bin\Release\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\DFLIMControlMVM\DFLIMControlMVM\bin\Release\DFLIMControlMVM.dll .\bin\Release\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\EpiTurretControlMVM\EpiTurretControlMVM\bin\Release\EpiTurretControlMVM.dll .\bin\Release\Modules\MVM
REM echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\MesoScanMVM\MesoScanMVM\bin\Release\MesoScanMVM.dll .\bin\Release\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\AutoFocusControlMVM\AutoFocusControlMVM\bin\Release\AutoFocusControlMVM.dll .\bin\Release\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\MiniCircuitsSwitchControlMVM\MiniCircuitsSwitchControlMVM\bin\Release\MiniCircuitsSwitchControlMVM.dll .\bin\Release\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\MultiLaserControlMVM\MultiLaserControlMVM\bin\Release\MultiLaserControlMVM.dll .\bin\Release\Modules\MVM

echo F | xcopy /F ..\..\..\..\Common\Database\DatabaseInterface\bin\Release\DatabaseInterface.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\..\Common\Database\DatabaseInterface\bin\Release\System.Data.SQLite.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\..\Common\Database\DatabaseInterface\bin\Release\x64\SQLite.Interop.dll .\bin\Release

echo F | xcopy /F ..\..\..\..\Common\GeometryUtilities\GeometryUtilities\bin\Release\GeometryUtilities.dll .\bin\Release\Modules

echo F | xcopy /F ..\..\..\Controls\ROIUserControl\bin\Release\ROIUserControl.dll .\bin\Release\Modules

echo F | xcopy /F ..\..\..\Controls\ThemeControl\ThemeControl\bin\Release\ThemeControl.dll .\bin\Release\Modules

echo F | xcopy /F ..\..\..\..\Tools\DeepZoomGen\DeepZoomGen\bin\Release\DeepZoomGen.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\..\Tools\DeepZoomGen\DeepZoomGen\bin\Release\DeepZoomTools.dll .\bin\Release\Modules

echo F | xcopy /F ..\..\..\..\Common\ThorSharedTypes\ThorSharedTypes\bin\Release\ThorSharedTypes.dll .\bin\Release
echo F | xcopy /F ..\..\..\..\Common\ThorDiskIO\x64\Release\ThorDiskIO.dll .\bin\Release
echo F | xcopy /F ..\..\..\..\Common\HardwareState\HardwareState\bin\Release\HardwareState.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\..\Common\AutoFocusModule\AutoFocusModule\bin\Release\AutoFocusModule.dll .\bin\Release\Modules

echo F | xcopy /F ..\..\..\..\Tools\HistogramControl\HistogramControl\bin\Release\HistogramControl.dll .\bin\Release\Modules
echo F | xcopy /F "..\..\..\..\Tools\DynamicDataDisplay v0.3 Binaries & Samples\DynamicDataDisplay\DynamicDataDisplay.dll" .\bin\Release\Modules
echo F | xcopy /F "..\..\..\..\Tools\DynamicDataDisplay v0.3 Binaries & Samples\DynamicDataDisplay\ru-ru\DynamicDataDisplay.resources.dll" .\bin\Release\ru-ru

echo F | xcopy /F ..\..\..\..\Common\ThorLogging\ThorLogging.xml .\bin\Release

REM echo F | xcopy /F ..\..\..\..\Tools\log4cxx0.10.0\msvc9-proj\x64\Release\log4cxx.dll .\bin\Release

REM echo F | xcopy /F ..\..\..\..\Common\Camera-TSI\dlls\Native_64_lib\logger\* .\bin\Release

echo F | xcopy /F ..\..\..\..\Common\HelpProvider\HelpProvider\bin\Release\HelpProvider.dll .\bin\Release
echo F | xcopy /F ..\..\..\..\Documents\HelpLS\ThorImageLS.chm .\bin\Release

::Commands:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

echo F | xcopy /F ..\..\..\..\Commands\General\SelectHardware\x64\Release\SelectHardware.dll .\bin\Release\Modules_Native
echo F | xcopy /F ..\..\..\..\Common\GeometryUtilities\x64\Release\GeometryUtilitiesCPP.dll .\bin\Release\Modules_Native

REM Uncomment later
echo F | xcopy /F ..\..\..\..\Commands\CaptureSetup\CaptureSetupModule\bin\Release\CaptureSetupModule.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\..\Commands\CaptureSetup\x64\Release\CaptureSetup.dll .\bin\Release\Modules_Native
echo F | xcopy /F ..\..\..\..\Commands\CaptureSetup\CaptureSetupModule\bin\Release\en-US\CaptureSetupModule.resources.dll .\bin\Release\en-US\CaptureSetupModule.resources.dll

echo F | xcopy /F ..\..\..\Controls\RangeSliderWPF\bin\Release\RangeSliderWPF.dll .\bin\Release\Modules
REM Uncomment later
echo F | xcopy /F ..\..\..\..\Commands\Acquisition\RunSample\RunSampleLSModule\bin\Release\RunSampleLSModule.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\..\Commands\Acquisition\RunSample\x64\Release\RunSample.dll .\bin\Release\Modules_Native
echo F | xcopy /F ..\..\..\..\Commands\Acquisition\RunSample\RunSampleLSModule\bin\Release\en-US\RunSampleLSModule.resources.dll .\bin\Release\en-US\RunSampleLSModule.resources.dll

echo F | xcopy /F ..\..\..\..\GUI\Controls\HardwareSetupModuleLS\HardwareSetupModuleLS\bin\Release\HardwareSetupModuleLS.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\HardwareSetupModuleLS\HardwareSetupModuleLS\bin\Release\en-US\HardwareSetupModuleLS.resources.dll .\bin\Release\en-US\HardwareSetupModuleLS.resources.dll


echo F | xcopy /F ..\..\..\..\GUI\Controls\XMLViewer\XMLViewer\bin\Release\XMLViewer.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\XMLViewer\XMLViewer\bin\Release\en-US\XMLViewer.resources.dll .\bin\Release\en-US\XMLViewer.resources.dll

echo F | xcopy /F ..\..\..\..\GUI\Controls\SettingsEditor\SettingsEditor\bin\Release\SettingsEditor.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\SettingsEditor\SettingsEditor\bin\Release\en-US\SettingsEditor.resources.dll .\bin\Release\en-US\SettingsEditor.resources.dll

echo F | xcopy /F ..\..\..\..\GUI\Controls\HardwareSetupUserControl\HardwareSetupUserControl\bin\Release\HardwareSetupUserControl.dll .\bin\Release\Modules
REM echo F | xcopy /F ..\..\..\..\GUI\Controls\HardwareSetupUserControl\HardwareSetupUserControl\bin\Release\en-US\HardwareSetupUserControl.resources.dll .\bin\Release\en-US\HardwareSetupUserControl.resources.dll

echo F | xcopy /F ..\..\..\..\GUI\Controls\SampleRegionSelection\SampleRegionSelection\bin\Release\SampleRegionSelection.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\SampleRegionSelection\SampleRegionSelection\bin\Release\en-US\SampleRegionSelection.resources.dll .\bin\Release\en-US\SampleRegionSelection.resources.dll

echo F | xcopy /F ..\..\..\..\GUI\Controls\ChartPlotterControl\ChartPlotterControl\bin\Release\ChartPlotterControl.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\AboutModule\AboutModule\bin\Release\AboutModule.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\..\Tools\WebUpdater\WebUpdater\bin\Release\WebUpdater.exe .\bin\Release
echo F | xcopy /F ..\..\..\..\Tools\WebUpdater\WebUpdater\bin\Release\AppLimit.CloudComputing.SharpBox.dll .\bin\Release
echo F | xcopy /F ..\..\..\..\Tools\WebUpdater\WebUpdater\bin\Release\Newtonsoft.Json.Net20.dll .\bin\Release

echo F | xcopy /F ..\..\..\..\GUI\Controls\Validations\Validations\bin\Release\Validations.dll .\bin\Release\Modules

@echo Copying Library ImageTilerControl / Thorlabs_IP_Library
echo F | xcopy /F ..\..\..\..\GUI\Controls\ImageTilerControl\ImageTilerControl\lib\Thorlabs_IP_Library.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\ImageTilerControl\ImageTilerControl\bin\Release\ImageTilerControl.dll .\bin\Release\Modules

echo F | xcopy /F ..\..\..\..\GUI\Controls\TilesDisplay\TilesDisplay\bin\Release\TilesDisplay.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\XYTileControl\XYTileControl\bin\Release\XYTileControl.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\VtkVolumeControl\VtkVolumeControl64\bin\Release\VtkVolumeControl64.dll .\bin\Release\Modules\VtkVolumeControl.dll
echo F | xcopy /F ..\..\..\..\GUI\Controls\VolumeControlInterface\VolumeControlInterface\bin\Release\VolumeControlInterface.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\SpinnerProgress\SpinnerProgress\bin\Release\SpinnerProgress.dll .\bin\Release\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\DigitalOutputSwitches\DigitalOutputSwitches\bin\Release\DigitalOutputSwitches.dll .\bin\Release\Modules

echo F | xcopy /F ..\..\..\..\GUI\Controls\JPEGer_Process\JPEGer_Process\\bin\Release\JPEGer_Process.exe .\bin\Release\
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorDetectorSwitch\MCLWrapper\MCLWrapper\bin\Release\mcl_RF_Switch_Controller64.dll .\bin\Release
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorDetectorSwitch\MCLWrapper\MCLWrapper\bin\Release\MCLWrapper.dll .\bin\Release

echo F | xcopy /F ..\..\..\..\Tools\Gong\Release\NET35\*.dll .\bin\Release\Modules

echo F | xcopy /F ..\..\..\..\Tools\ActiViz-5.8.0\x64\Cosmo.dll .\bin\Release\Lib
echo F | xcopy /F ..\..\..\..\Tools\ActiViz-5.8.0\x64\K*.* .\bin\Release\Lib
echo F | xcopy /F ..\..\..\..\Tools\ActiViz-5.8.0\x64\V*.* .\bin\Release\Lib
move .\bin\Release\Lib\Kitware.mummy.Runtime.dll .\bin\Release
move .\bin\Release\Lib\Kitware.VTK.dll .\bin\Release
move .\bin\Release\Lib\Kitware.VTK.vtkFiltering.Unmanaged.dll .\bin\Release

echo F | xcopy /F ..\..\..\..\GUI\Controls\ROIStatsChart\ROIStatsChart\bin\Release\ROIStatsChart.dll .\bin\Release\Modules

echo F | xcopy /F ..\..\..\..\GUI\Controls\KuriosControl\KuriosControl\bin\Release\KuriosControl.dll .\bin\Release\Modules

::ModuleNative:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::


echo F | xcopy /F ..\..\..\..\Common\ExperimentManager\x64\Release\ExperimentManager.dll .\bin\Release
echo F | xcopy /F ..\..\..\..\Common\ImageManager\x64\Release\ImageManager.dll .\bin\Release\
echo F | xcopy /F ..\..\..\..\Common\ResourceManager\x64\Release\ResourceManager.dll .\bin\Release\
echo F | xcopy /F ..\..\..\..\Common\RoiDataStore\x64\Release\RoiDataStore.dll .\bin\Release
echo F | xcopy /F ..\..\..\..\Common\ConvertUtilities\x64\Release\ClassicTiffConverter.dll .\bin\Release\Modules_Native
echo F | xcopy /F ..\..\..\..\Common\ConvertUtilities\Libs\classic_tiff_library\*.dll .\bin\Release\Modules_Native
echo F | xcopy /F ..\..\..\..\Common\ConvertUtilities\Libs\ImageStoreLibrary\ome_tiff_library.dll .\bin\Release\Modules_Native
echo F | xcopy /F ..\..\..\..\Common\ImageStoreLibrary\bin\vs2012\Release\*.dll .\bin\Release\Modules_Native
echo F | xcopy /F ..\..\..\..\Common\Sample\x64\Release\Sample.dll .\bin\Release\Modules_Native
echo F | xcopy /F ..\..\..\..\Common\HardwareCom\x64\Release\HardwareCom.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Common\HologramGenerator\x64\Release\HologramGenerator.dll .\bin\Release\Modules_Native
echo F | xcopy /F ..\..\..\..\Common\StatsManager\x64\Release\StatsManager.dll .\bin\Release
REM echo F | xcopy /F ..\..\..\..\Common\ThorLogging\x64\Release\ThorLoggingUnmanagedDisabled.dll .\bin\Release\Modules_Native\ThorLoggingUnmanaged.dll
REM echo F | xcopy /F ..\..\..\..\Common\WinDVI\x64\Release\WinDVI.dll .\bin\Release\Modules_Native
echo F | xcopy /F ..\..\..\..\Common\ThorLogging\x64\Release\ThorLoggingUnmanaged.dll .\bin\Release\Modules_Native\ThorLoggingUnmanaged.dll
echo F | xcopy /F ..\..\..\..\Common\AutoFocusModule\x64\Release\AutoFocus.dll .\bin\Release\Modules_Native

echo F | xcopy /F ..\..\..\..\Commands\ImageAnalysis\PincushionCorrection\x64\Release\PincushionCorrection.dll .\bin\Release
echo F | xcopy /F ..\..\..\..\Commands\ImageAnalysis\FlatField\x64\Release\FlatField.dll .\bin\Release
echo F | xcopy /F ..\..\..\..\Commands\ImageAnalysis\LineProfile\x64\Release\LineProfile.dll .\bin\Release
echo F | xcopy /F ..\..\..\..\Commands\ImageAnalysis\ThorImageProcess\x64\Release\ThorImageProcess.dll .\bin\Release

echo F | xcopy /F ..\..\..\..\Hardware\Cameras\CameraManager\x64\Release\CameraManager.dll .\bin\Release\
echo F | xcopy /F ..\..\..\..\Hardware\Devices\DeviceManager\x64\Release\DeviceManager.dll .\bin\Release\

echo F | xcopy /F ..\..\..\..\Tools\HDF5IO\x64\Release\HDF5IO.dll .\bin\Release\Modules_Native
echo F | xcopy /F ..\..\..\..\Tools\HDF5IO\HDF5IO\bin\Release\*.* .\bin\Release


:: Simulators ::::::::::::::::::::::
echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorConfocalSimulator\x64\Release\ThorConfocalSimulator.dll .\bin\Release\Modules_Native
echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorConfocalGalvoSimulator\x64\Release\ThorConfocalGalvoSimulator.dll .\bin\Release\Modules_Native
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorZStepperSimulator\x64\Release\ThorZStepperSimulator.dll .\bin\Release\Modules_Native
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorPMTSimulator\x64\Release\ThorPMTSimulator.dll .\bin\Release\Modules_Native
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorTDCSimulator\x64\Release\ThorTDCSimulator.dll .\bin\Release\Modules_Native
echo F | xcopy /F ..\..\..\..\Hardware\Devices\SimDeviceXY\x64\Release\SimDeviceXY.dll .\bin\Release\Modules_Native
echo F | xcopy /F ..\..\..\..\Hardware\Devices\SimDeviceFW\x64\Release\SimDeviceFW.dll .\bin\Release\Modules_Native
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorBmExpanSimulator\x64\Release\ThorBmExpanSimulator.dll .\bin\Release\Modules_Native
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorMCLSSimulator\x64\Release\ThorMCLSSimulator.dll .\bin\Release\Modules_Native
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorPinholeStepperSimulator\x64\Release\ThorPinholeStepperSimulator.dll .\bin\Release\Modules_Native
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorShutterSimulator\x64\Release\ThorShutterSimulator.dll .\bin\Release\Modules_Native
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorLightPathSimulator\x64\Release\ThorLightPathSimulator.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorMesoScanSimulator\x64\Release\ThorMesoScanSimulator.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorMesoScanSimulator\TorboJpeg\turbojpeg.dll .\bin\Release\Lib
REM echo F | xcopy /F ..\..\..\..\Tools\ImageProcessLibrary.1.0.3\build\native\ImageProcessLibrary.dll .\bin\Release\Lib

:: Devices ::::::::::::::::::::::
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\CoherentChameleon\x64\Release\CoherentChameleon.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\MaiTaiLaser\x64\Release\MaiTaiLaser.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorBCM\x64\Release\ThorBCM.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorBmExpan\x64\Release\ThorBmExpan.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorBeamStabilizer\x64\Release\ThorBeamStabilizer.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorVBE\x64\Release\ThorVBE.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorBScope\x64\Release\ThorBScope.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorConfocal\x64\Release\ThorConfocal.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorConfocalGalvo\x64\Release\ThorConfocalGalvo.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorGGNI\x64\Release\ThorGGNI.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorMesoScan\x64\Release\ThorMesoScan.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorStim\x64\Release\ThorStim.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorTSI\x64\Release\ThorTSI.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Common\Camera-TSI\dlls\Native_64_lib\bin_CCD_mono\*.dll .\bin\Release\Lib
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorTSI_CS\x64\Release\ThorTSI_CS.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Common\Camera-TSI\dlls\Native_64_lib\bin_CMOS_mono\*.dll .\bin\Release\Lib
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\DCxCamera\x64\Release\DCxCamera.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ORCA\x64\Release\ORCA.dll .\bin\Release\Modules_Native

REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\thordaq\x64\Release\thordaq.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\thordaq\x64\Release\ThorDAQGalvoGalvo.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\thordaq\x64\Release\ThorDAQResonantGalvo.dll .\bin\Release\Modules_Native

REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThordaqDFLIM\x64\Release\ThordaqDFLIM.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThordaqDFLIM\x64\Release\ThorDFLIMGalvoGalvo.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThordaqDFLIM\x64\Release\ThorDAQGGDFLIMSim.dll .\bin\Release\Modules_Native

REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorBCMPA\x64\Release\ThorBCMPA.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorBCMPA2\x64\Release\ThorBCMPA2.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorECU\x64\Release\ThorECU.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorElectroPhys\x64\Release\ThorElectroPhys.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorEpiTurret\x64\Release\ThorEpiTurret.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorDC2200\x64\Release\ThorDC2200.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorDDR05\x64\Release\ThorDDR05.dll .\bin\Release\Modules_Native
echo F | xcopy /F ..\..\..\..\Hardware\Devices\DisconnectedDevice\x64\Release\Disconnected.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorKurios\x64\Release\ThorKurios.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorMCLS\x64\Release\ThorMCLS.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorMCM3000\x64\Release\ThorMCM3000.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorMCM3000Aux\x64\Release\ThorMCM3000Aux.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorMCM6000\x64\Release\ThorMCM6000.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorMCM6000_Condenser\x64\Release\ThorMCM6000_Condenser.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorMCM6000_Aux\x64\Release\ThorMCM6000_Aux.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorMLSStage\x64\Release\ThorMLSStage.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorMTS25\x64\Release\ThorMTS25.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorObjectiveChanger\x64\Release\ThorObjectiveChanger.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorPinholeStepper\x64\Release\ThorPinholeStepper.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorPLSZ\x64\Release\ThorPLSZ.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorPMT\x64\Release\ThorPMT.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorPMT2\x64\Release\ThorPMT2.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorPMT2100\x64\Release\ThorPMT2100.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorPowerControl\x64\Release\ThorPowerControl.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorShutterDigital\x64\Release\ThorShutterDig.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorShutterDigital2\x64\Release\ThorShutterDig2.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorShutterDigital3\x64\Release\ThorShutterDig3.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorShutterDigital4\x64\Release\ThorShutterDig4.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorShutterDigital5\x64\Release\ThorShutterDig5.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorShutterDigital6\x64\Release\ThorShutterDig6.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorTDC\x64\Release\ThorTDC.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorVBE\x64\Release\ThorVBE.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorZPiezo\x64\Release\ThorZPiezo.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorZStepper\x64\Release\ThorZStepper.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorSLM\x64\Release\ThorSLM.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorSLMPDM512\x64\Release\ThorSLMPDM512.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorSLMPDM512\ThorSLMPDM512\DLL\*.* .\bin\Release\Lib
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorTiberius\x64\Release\ThorTiberius.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\PIPiezoXYZ\x64\Release\PIPiezo.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\OTMLaser\x64\Release\OTMLaser.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorChrolis\x64\Release\ThorChrolis.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorLSKGR\x64\Release\ThorLSKGR.dll .\bin\Release\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorDetector\x64\Release\ThorDetector.dll .\bin\Release\Modules_Native

:: Settings Files :::::::::::::::::::::
echo F | xcopy /F ..\..\..\..\Hardware\Devices\CoherentChameleon\CoherentChameleonSettings.xml .\bin\Release
echo F | xcopy /F ..\..\..\..\Hardware\Devices\MaiTaiLaser\MaiTaiLaserSettings.xml .\bin\Release
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorBCM\ThorBCMSettings.xml .\bin\Release
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorBeamStabilizer\ThorBeamStabilizerSettings.xml .\bin\Release\
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorBmExpan\ThorBmExpanSettings.xml .\bin\Release\
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorBScope\ThorBScopeSettings.xml .\bin\Release
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorChrolis\ThorChrolisSettings.xml .\bin\Release
echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorConfocal\ThorConfocalSettings.xml .\bin\Release
echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorConfocalGalvo\ThorConfocalGalvoSettings.xml .\bin\Release
echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorConfocalGalvoSimulator\ThorConfocalGalvoSimulatorSettings.xml .\bin\Release
echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorConfocalSimulator\ThorConfocalSimulatorSettings.xml .\bin\Release
echo F | xcopy /F ..\..\..\..\Installers\ThorImageLSx64\ThorImageLSx64\ThorDetectorSwitchSettings.xml .\bin\Release
echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorGGNI\ThorGGNISettings.xml .\bin\Release
echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorMesoScan\ThorMesoScanSettings.xml .\bin\Release
echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorStim\ThorStimSettings.xml .\bin\Release
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorECU\ThorECUSettings.xml .\bin\Release
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorEpiTurret\ThorEpiTurretSettings.xml .\bin\Release
echo F | xcopy /F ..\..\..\..\GUI\Applications\ThorImageLS\ThorImage\ThorImageLS.ico .\bin\Release
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorMCLS\ThorMCLSSettings.xml .\bin\Release
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorMCM3000\ThorMCM3000Settings.xml .\bin\Release
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorMCM3000Aux\ThorMCM3000AuxSettings.xml .\bin\Release
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorMLSStage\ThorMLSStageSettings.xml .\bin\Release
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorObjectiveChanger\ThorObjectiveChangerSettings.xml .\bin\Release\
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorMTS25\ThorMTS25Settings.xml .\bin\Release
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorPinholeStepper\ThorPinholeStepperSettings.xml .\bin\Release\
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorPMT\ThorPMTSettings.xml .\bin\Release
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorPMT2\ThorPMT2Settings.xml .\bin\Release
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorPMT2100\ThorPMT2100Settings.xml .\bin\Release
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorPowerControl\PowerControlSettings.xml .\bin\Release\
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorShutterDigital\ThorShutterDigSettings.xml .\bin\Release\
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorShutterDigital2\ThorShutterDig2Settings.xml .\bin\Release\
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorShutterDigital3\ThorShutterDig3Settings.xml .\bin\Release\
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorShutterDigital4\ThorShutterDig4Settings.xml .\bin\Release\
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorShutterDigital5\ThorShutterDig5Settings.xml .\bin\Release\
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorShutterDigital6\ThorShutterDig6Settings.xml .\bin\Release\
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorZPiezo\ThorZPiezoSettings.xml .\bin\Release
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorZStepper\ZStepperSettings.xml .\bin\Release
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorVBE\ThorVBESettings.xml .\bin\Release
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorElectroPhys\ThorElectroPhysSettings.xml .\bin\Release
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorBCMPA\ThorBCMPASettings.xml .\bin\Release
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorDDR05\ThorDDR05Settings.xml .\bin\Release
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorSLM\ThorSLMSettings.xml .\bin\Release
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorPLSZ\ThorPLSZSettings.xml .\bin\Release
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorTiberius\ThorTiberiusSettings.xml .\bin\Release
echo F | xcopy /F ..\..\..\..\Hardware\Devices\PIPiezoXYZ\PIPiezoSettings.xml .\bin\Release
echo F | xcopy /F ..\..\..\..\Hardware\Devices\OTMLaser\OTMLaserSettings.xml .\bin\Release
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorLSKGR\ThorLSKGRSettings.xml .\bin\Release
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorMCM6000\ThorMCM6000Settings.xml .\bin\Release
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorDetector\ThorDetectorSettings.xml .\bin\Release

echo F | xcopy /F ..\..\..\..\Hardware\Cameras\thordaq\Dll\ThorDAQResonantGalvo\ThorDAQResonantGalvoSettings.xml .\bin\Release
echo F | xcopy /F ..\..\..\..\Hardware\Cameras\thordaq\Dll\ThorDAQGalvoGalvo\ThorDAQGalvoGalvoSettings.xml .\bin\Release
echo F | xcopy /F ..\..\..\..\Hardware\Cameras\thordaq\Dll\ThorDAQIOSettings.xml .\bin\Release

REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThordaqDFLIM\Dll\ThorDFLIMGalvoGalvo\ThorDFLIMGalvoGalvoSettings.xml .\bin\Release
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThordaqDFLIM\Dll\ThorDAQGGDFLIMSim\fullFrameSim.bin .\bin\Release

echo F | xcopy /F ..\..\..\..\Common\ResourceManager\ResourceManager.xml .\bin\Release

:: Experiment Review:::::::::::::::::::


echo F | xcopy /F ..\..\ExperimentReview\ExperimentReview\bin\Release\ExperimentReview.exe .\bin\Release\
echo F | xcopy /F ..\..\ExperimentReview\ExperimentReview\bin\Release\ExperimentReview.exe.config .\bin\Release\
echo F | xcopy /F ..\..\ExperimentReview\ExperimentReview\bin\Release\ExperimentReview.vshost.exe .\bin\Release\

echo F | xcopy /F ..\..\ExperimentReview\ExperimentReview\bin\Release\DynamicDataDisplay.xml .\bin\Release\

:: Matlab Script:::::::::::::::::::::
REM echo F | xcopy /F ..\..\..\..\Common\MatlabEngine\RunTime\mclcom9_1.dll .\bin\Release\
REM echo F | xcopy /F ..\..\..\..\Common\MatlabEngine\RunTime\mclcommain9_1.dll .\bin\Release\
REM echo F | xcopy /F ..\..\..\..\Common\MatlabEngine\RunTime\mclmcrrt9_1.dll .\bin\Release\
REM echo F | xcopy /F ..\..\..\..\Common\MatlabEngine\RunTime\mclxlmain9_1.dll .\bin\Release\
REM echo F | xcopy /F ..\..\..\..\Common\MatlabEngine\x64\Release\NativeMatlabEngine.dll .\bin\Release\
REM echo F | xcopy /F ..\..\..\..\Common\MatlabEngine\MatlabEngine\bin\Release\MatlabEngine.dll .\bin\Release\Modules\
REM echo F | xcopy /F ..\..\..\Controls\ScriptPanels\ScriptMatlab\ScriptMatlab\bin\Release\ScriptMatlab.dll .\bin\Release\Modules\
REM echo F | xcopy /F ..\..\..\Controls\ScriptManager\ScriptRunMatlabModule\bin\Release\ScriptRunMatlabModule.dll .\bin\Release\Modules\