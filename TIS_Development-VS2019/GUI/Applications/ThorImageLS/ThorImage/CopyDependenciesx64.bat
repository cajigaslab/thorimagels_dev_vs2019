del /Q ".\bin\Debug\Modules\MVM\*.*"

del /Q ".\bin\Debug\Modules\*.*"

del /Q ".\bin\Debug\Modules_Native\*.*"

del /Q ".\bin\Debug\Lib\*.*"

del /Q ".\bin\Debug\en-US\*.*"

del /Q ".\bin\Debug\ru-ru\*.*"

del /Q ".\bin\Debug\*.*"

mkdir ".\bin\Debug\Modules\"

mkdir ".\bin\Debug\Modules\MVM"

mkdir ".\bin\Debug\Modules_Native\"

mkdir ".\bin\Debug\Lib\"

mkdir ".\bin\Debug\en-US\"

mkdir ".\bin\Debug\ru-ru\"

@echo Copying User Folders
mkdir "%UserProfile%\Documents\ThorImageLS 4.2"
echo F | xcopy /D "..\..\..\..\Installers\ThorImageLSx64\ThorImageLSx64\*" "%UserProfile%\Documents\ThorImageLS 4.2" /s

echo F | xcopy /F ..\Library\*.* .\bin\Debug

@echo Copying Library TIFF
echo F | xcopy /F ..\..\..\..\Tools\tiff-3.9.4\bin\x64\Debug\libtiff3.dll .\bin\Debug

@echo Copying Library Intel IPP
echo F | xcopy /F "..\..\..\..\Tools\Intel IPP\intel64\dll\ippcore-7.0.dll" .\bin\Debug
echo F | xcopy /F "..\..\..\..\Tools\Intel IPP\intel64\dll\ippi-7.0.dll" .\bin\Debug
echo F | xcopy /F "..\..\..\..\Tools\Intel IPP\intel64\dll\ippiu8-7.0.dll" .\bin\Debug
echo F | xcopy /F "..\..\..\..\Tools\Intel IPP\intel64\dll\libiomp5md.dll" .\bin\Debug
echo F | xcopy /F "..\..\..\..\Tools\Intel IPP\intel64\dll\ippcvu8-7.0.dll" .\bin\Debug
echo F | xcopy /F "..\..\..\..\Tools\Intel IPP\intel64\dll\ippm-7.0.dll" .\bin\Debug
echo F | xcopy /F "..\..\..\..\Tools\Intel IPP\intel64\dll\ippmm7-7.0.dll" .\bin\Debug
echo F | xcopy /F "..\..\..\..\Tools\Intel IPP\intel64\dll\ipps-7.0.dll" .\bin\Debug
echo F | xcopy /F "..\..\..\..\Tools\Intel IPP\intel64\dll\ippsy8-7.0.dll" .\bin\Debug

@echo Copying Library WPF toolkit
echo F | xcopy /F ..\..\..\..\Tools\WPFToolkit\v3.5.40619.1\WPFToolkit.dll .\bin\Debug
echo F | xcopy /F ..\..\..\..\Tools\Colorpickerlib\bin\Debug\ColorPicker.dll .\bin\Debug

@echo Copying Library AVI
echo F | xcopy /F "..\..\..\..\Tools\AviFile\bin\Debug\AviFile.dll" .\bin\Debug

@echo Copying Library LibTiff.NET40
echo F | xcopy /F "..\..\..\..\Tools\LibTiff.Net40\Debug\BitMiracle.LibTiff.NET40.dll" .\bin\Debug
echo F | xcopy /F "..\..\..\..\Tools\LibTiff.Net40\Debug\BitMiracle.LibTiff.NET40.xml" .\bin\Debug

REM @echo Copying Library uart for Kurios
REM echo F | xcopy /F "..\..\..\..\Tools\uartlibrary\x64\uart_library_ftdi64.dll" .\bin\Debug

@echo Copying Library Gong
echo F | xcopy /F "..\..\..\..\Tools\Gong\gong-wpf-dragdrop-master\GongSolutions.Wpf.DragDrop\bin\Debug\NET4\GongSolutions.Wpf.DragDrop.dll" .\bin\Debug\Modules

@echo Copying Library MahApps.Metro
echo F | xcopy /F "..\..\..\..\Tools\MahApps.Metro\MahApps.Metro.1.5.0\lib\net45\MahApps.Metro.dll" .\bin\Debug

@echo Copying Library Mini Circuits - Switch box library built in .NET 4.5
echo F | xcopy /F "..\..\..\..\Tools\Mini-Circuits\RF switch controller\mcl_RF_Switch_ControllerNET45.dll" .\bin\Debug

@echo Copying Library Infragistics
echo F | xcopy /F "..\..\..\..\Tools\Infragistics\Infragistics.WPF.16.2.20162.2141\lib\InfragisticsWPF.dll" .\bin\Debug
echo F | xcopy /F "..\..\..\..\Tools\Infragistics\Infragistics.WPF.Controls.Editors.XamMaskedInput.16.2.20162.2141\lib\InfragisticsWPF.Controls.Editors.XamMaskedInput.dll" .\bin\Debug
echo F | xcopy /F "..\..\..\..\Tools\Infragistics\Infragistics.WPF.Controls.Editors.XamSlider.16.2.20162.2141\lib\InfragisticsWPF.Controls.Editors.XamSlider.dll" .\bin\Debug

@echo Copying FLIM fitting library
echo F | xcopy /F ..\..\..\..\Tools\FLIMFit\x64\Release\FLIMFitLibrary.dll .\bin\Debug\Lib

REM @echo Copying Library for Chrolis
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorChrolis\Driver\TL6WL\bin\TL6WL_64.dll .\bin\Debug\Lib

@echo Copying Local Libraries
echo F | xcopy /F ..\ThorImageInfastructure\bin\Debug\ThorImageInfastructure.dll .\bin\Debug\Modules

echo F | xcopy /F ..\..\..\Controls\FolderDialogControl\FolderDialogControl\bin\Debug\FolderDialogControl.dll .\bin\Debug\Modules

echo F | xcopy /F ..\..\..\Controls\ImageReview\ImageReview\bin\Debug\ImageReview.dll .\bin\Debug\Modules

echo F | xcopy /F ..\..\..\Controls\MenuModuleLS\MenuModuleLS\bin\Debug\MenuModuleLS.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\Controls\MenuModuleLS\MenuModuleLS\bin\Debug\en-US\MenuModuleLS.resources.dll .\bin\Debug\en-US\MenuModuleLS.resources.dll

echo F | xcopy /F ..\..\..\Controls\LogFileWindow\LogFileWindow\bin\Debug\LogFileWindow.dll .\bin\Debug\Modules

echo F | xcopy /F ..\..\..\..\Common\IPCModules\IPCModules\bin\Debug\IPCModules.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\Controls\ScriptManager\ScriptManager\bin\Debug\ScriptManager.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\Controls\ScriptManager\ScriptFijiProcessing\bin\Debug\ScriptFijiProcessing.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\Controls\FijiLauncher\FijiLauncher\bin\Debug\FijiLauncher.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\Controls\FijiLauncher\FijiLauncherPanel\bin\Debug\FijiLauncherPanel.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\Controls\ScriptPanels\SetScriptPath\SetScriptPath\bin\Debug\SetScriptPath.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\Controls\ScriptPanels\ForLoop\ForLoop\bin\Debug\ForLoop.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\Controls\ScriptPanels\ScriptWait\ScriptWait\bin\Debug\ScriptWait.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\Controls\ScriptPanels\ScriptXYMove\ScriptXYMove\bin\Debug\ScriptXYMove.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\Controls\ScriptPanels\ScriptZMove\ScriptZMove\bin\Debug\ScriptZMove.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\Controls\ScriptPanels\ScriptZ2Move\ScriptZ2Move\bin\Debug\ScriptZ2Move.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\ExperimentSettingsViewer\ExperimentSettingsViewer\bin\Debug\ExperimentSettingsViewer.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\ExperimentSettingsBrowser\ExperimentSettingsBrowser\bin\Debug\ExperimentSettingsBrowser.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\Validations\Validations\bin\Debug\Validations.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\OverlayManager\OverlayManager\bin\Debug\OverlayManager.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\MultiROIStats\MultiROIStats\bin\Debug\MultiROIStats.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\LineProfileWindow\LineProfileWindow\bin\Debug\LineProfileWindow.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\MessageBoxCheck\MessageBoxCheck\bin\Debug\MessageBoxCheck.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\PowerControl\PowerControl\bin\Debug\PowerControl.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\AreaControl\AreaControl\bin\Debug\AreaControl.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\ZControl\ZControl\bin\Debug\ZControl.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\ScanControl\ScanControl\bin\Debug\ScanControl.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\OTMControl\OTMControl\bin\Debug\OTMControl.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\LaserControl\LaserControl\bin\Debug\LaserControl.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\MultiphotonControl\MultiphotonControl\bin\Debug\MultiphotonControl.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\CameraControl\CameraControl\bin\Debug\CameraControl.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\LampControl\LampControl\bin\Debug\LampControl.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\PinholeControl\PinholeControl\bin\Debug\PinholeControl.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\CaptureOptionsControl\CaptureOptionsControl\bin\Debug\CaptureOptionsControl.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\QuickTemplatesControl\QuickTemplatesControl\bin\Debug\QuickTemplatesControl.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\ThreePhotonControl\ThreePhotonControl\bin\Debug\ThreePhotonControl.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\LightEngineControl\LightEngineControl\bin\Debug\LightEngineControl.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\EpiTurretControl\EpiTurretControl\bin\Debug\EpiTurretControl.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\RemoteIPCControl\RemoteIPCControl\bin\Debug\RemoteIPCControl.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\DFLIMControl\DFLIMControl\bin\Debug\DFLIMControl.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\DFLIMSetupAssistant\DFLIMSetupAssistant\bin\Debug\DFLIMSetupAssistant.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\FLIMFitting\FLIMFitting\bin\Debug\FLIMFitting.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\AutoFocusControl\AutoFocusControl\bin\Debug\AutoFocusControl.dll .\bin\Debug\Modules

@echo Copying Library MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\PowerControlMVM\PowerControlMVM\bin\Debug\PowerControlMVM.dll .\bin\Debug\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\AreaControlMVM\AreaControlMVM\bin\Debug\AreaControlMVM.dll .\bin\Debug\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\ZControlMVM\ZControlMVM\bin\Debug\ZControlMVM.dll .\bin\Debug\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\ScanControlMVM\ScanControlMVM\bin\Debug\ScanControlMVM.dll .\bin\Debug\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\OTMControlMVM\OTMControlMVM\bin\Debug\OTMControlMVM.dll .\bin\Debug\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\LaserControlMVM\LaserControlMVM\bin\Debug\LaserControlMVM.dll .\bin\Debug\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\MultiphotonControlMVM\MultiphotonControlMVM\bin\Debug\MultiphotonControlMVM.dll .\bin\Debug\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\DigitalOutputSwitchesMVM\DigitalOutputSwitchesMVM\bin\Debug\DigitalOutputSwitchesMVM.dll .\bin\Debug\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\CameraControlMVM\CameraControlMVM\bin\Debug\CameraControlMVM.dll .\bin\Debug\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\LampControlMVM\LampControlMVM\bin\Debug\LampControlMVM.dll .\bin\Debug\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\PinholeControlMVM\PinholeControlMVM\bin\Debug\PinholeControlMVM.dll .\bin\Debug\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\QuickTemplatesControlMVM\QuickTemplatesControlMVM\bin\Debug\QuickTemplatesControlMVM.dll .\bin\Debug\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\XYTileControlMVM\XYTileControlMVM\bin\Debug\XYTileControlMVM.dll .\bin\Debug\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\KuriosControlMVM\KuriosControlMVM\bin\Debug\KuriosControlMVM.dll .\bin\Debug\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\CaptureOptionsControlMVM\CaptureOptionsControlMVM\bin\Debug\CaptureOptionsControlMVM.dll .\bin\Debug\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\ThreePhotonControlMVM\ThreePhotonControlMVM\bin\Debug\ThreePhotonControlMVM.dll .\bin\Debug\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\LightEngineControlMVM\LightEngineControlMVM\bin\Debug\LightEngineControlMVM.dll .\bin\Debug\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\ObjectiveControlMVM\ObjectiveControlMVM\bin\Debug\ObjectiveControlMVM.dll .\bin\Debug\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\RemoteIPCControlMVM\RemoteIPCControlMVM\bin\Debug\RemoteIPCControlMVM.dll .\bin\Debug\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\DFLIMControlMVM\DFLIMControlMVM\bin\Debug\DFLIMControlMVM.dll .\bin\Debug\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\EpiTurretControlMVM\EpiTurretControlMVM\bin\Debug\EpiTurretControlMVM.dll .\bin\Debug\Modules\MVM
REM echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\MesoScanMVM\MesoScanMVM\bin\Debug\MesoScanMVM.dll .\bin\Debug\Modules\MVM
echo F | xcopy /F ..\..\..\..\GUI\ControlsMVM\AutoFocusControlMVM\AutoFocusControlMVM\bin\Debug\AutoFocusControlMVM.dll .\bin\Debug\Modules\MVM

echo F | xcopy /F ..\..\..\..\Common\Database\DatabaseInterface\bin\Debug\DatabaseInterface.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\..\Common\Database\DatabaseInterface\bin\Debug\System.Data.SQLite.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\..\Common\Database\DatabaseInterface\bin\Debug\x64\SQLite.Interop.dll .\bin\Debug

echo F | xcopy /F ..\..\..\..\Common\GeometryUtilities\GeometryUtilities\bin\Debug\GeometryUtilities.dll .\bin\Debug\Modules

echo F | xcopy /F ..\..\..\Controls\ROIUserControl\bin\Debug\ROIUserControl.dll .\bin\Debug\Modules

echo F | xcopy /F ..\..\..\Controls\ThemeControl\ThemeControl\bin\Debug\ThemeControl.dll .\bin\Debug\Modules

echo F | xcopy /F ..\..\..\..\Tools\DeepZoomGen\DeepZoomGen\bin\Debug\DeepZoomGen.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\..\Tools\DeepZoomGen\DeepZoomGen\bin\Debug\DeepZoomTools.dll .\bin\Debug\Modules

echo F | xcopy /F ..\..\..\..\Common\ThorSharedTypes\ThorSharedTypes\bin\Debug\ThorSharedTypes.dll .\bin\Debug
echo F | xcopy /F ..\..\..\..\Common\ThorDiskIO\x64\Debug\ThorDiskIO.dll .\bin\Debug
echo F | xcopy /F ..\..\..\..\Common\HardwareState\HardwareState\bin\Debug\HardwareState.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\..\Common\AutoFocusModule\AutoFocusModule\bin\Debug\AutoFocusModule.dll .\bin\Debug\Modules

echo F | xcopy /F ..\..\..\..\Tools\HistogramControl\HistogramControl\bin\Debug\HistogramControl.dll .\bin\Debug\Modules
echo F | xcopy /F "..\..\..\..\Tools\DynamicDataDisplay v0.3 Binaries & Samples\DynamicDataDisplay\DynamicDataDisplay.dll" .\bin\Debug
echo F | xcopy /F "..\..\..\..\Tools\DynamicDataDisplay v0.3 Binaries & Samples\DynamicDataDisplay\ru-ru\DynamicDataDisplay.resources.dll" .\bin\Debug\ru-ru

echo F | xcopy /F ..\..\..\..\Common\ThorLogging\ThorLogging.xml .\bin\Debug

REM echo F | xcopy /F ..\..\..\..\Tools\log4cxx0.10.0\msvc9-proj\x64\Debug\log4cxx.dll .\bin\Debug

REM echo F | xcopy /F ..\..\..\..\Common\Camera-TSI\dlls\Native_64_lib\logger\* .\bin\Debug

echo F | xcopy /F ..\..\..\..\Common\HelpProvider\HelpProvider\bin\Debug\HelpProvider.dll .\bin\Debug
echo F | xcopy /F ..\..\..\..\Documents\HelpLS\ThorImageLS.chm .\bin\Debug

::Commands:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

echo F | xcopy /F ..\..\..\..\Commands\General\SelectHardware\x64\Debug\SelectHardware.dll .\bin\Debug\Modules_Native
echo F | xcopy /F ..\..\..\..\Common\GeometryUtilities\x64\Debug\GeometryUtilitiesCPP.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Commands\Acquisition\LiveImageData\x64\Debug\LiveImageData.dll .\bin\Debug\Modules_Native

REM Uncomment later
echo F | xcopy /F ..\..\..\..\Commands\CaptureSetup\CaptureSetupModule\bin\Debug\CaptureSetupModule.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\..\Commands\CaptureSetup\x64\Debug\CaptureSetup.dll .\bin\Debug\Modules_Native
echo F | xcopy /F ..\..\..\..\Commands\CaptureSetup\CaptureSetupModule\bin\Debug\en-US\CaptureSetupModule.resources.dll .\bin\Debug\en-US\CaptureSetupModule.resources.dll

echo F | xcopy /F ..\..\..\Controls\RangeSliderWPF\bin\Debug\RangeSliderWPF.dll .\bin\Debug\Modules
REM Uncomment later
echo F | xcopy /F ..\..\..\..\Commands\Acquisition\RunSample\RunSampleLSModule\bin\Debug\RunSampleLSModule.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\..\Commands\Acquisition\RunSample\x64\Debug\RunSample.dll .\bin\Debug\Modules_Native
echo F | xcopy /F ..\..\..\..\Commands\Acquisition\RunSample\RunSampleLSModule\bin\Debug\en-US\RunSampleLSModule.resources.dll .\bin\Debug\en-US\RunSampleLSModule.resources.dll

echo F | xcopy /F ..\..\..\..\GUI\Controls\HardwareSetupModuleLS\HardwareSetupModuleLS\bin\Debug\HardwareSetupModuleLS.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\HardwareSetupModuleLS\HardwareSetupModuleLS\bin\Debug\en-US\HardwareSetupModuleLS.resources.dll .\bin\Debug\en-US\HardwareSetupModuleLS.resources.dll


echo F | xcopy /F ..\..\..\..\GUI\Controls\XMLViewer\XMLViewer\bin\Debug\XMLViewer.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\XMLViewer\XMLViewer\bin\Debug\en-US\XMLViewer.resources.dll .\bin\Debug\en-US\XMLViewer.resources.dll

echo F | xcopy /F ..\..\..\..\GUI\Controls\SettingsEditor\SettingsEditor\bin\Debug\SettingsEditor.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\SettingsEditor\SettingsEditor\bin\Debug\en-US\SettingsEditor.resources.dll .\bin\Debug\en-US\SettingsEditor.resources.dll

REM echo F | xcopy /F ..\..\..\..\GUI\Controls\HardwareSetupUserControl\HardwareSetupUserControl\bin\Debug\HardwareSetupUserControl.dll .\bin\Debug\Modules
REM echo F | xcopy /F ..\..\..\..\GUI\Controls\HardwareSetupUserControl\HardwareSetupUserControl\bin\Debug\en-US\HardwareSetupUserControl.resources.dll .\bin\Debug\en-US\HardwareSetupUserControl.resources.dll

echo F | xcopy /F ..\..\..\..\GUI\Controls\SampleRegionSelection\SampleRegionSelection\bin\Debug\SampleRegionSelection.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\SampleRegionSelection\SampleRegionSelection\bin\Debug\en-US\SampleRegionSelection.resources.dll .\bin\Debug\en-US\SampleRegionSelection.resources.dll

echo F | xcopy /F ..\..\..\..\GUI\Controls\ChartPlotterControl\ChartPlotterControl\bin\Debug\ChartPlotterControl.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\AboutModule\AboutModule\bin\Debug\AboutModule.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\..\Tools\WebUpdater\WebUpdater\bin\Debug\WebUpdater.exe .\bin\Debug
echo F | xcopy /F ..\..\..\..\Tools\WebUpdater\WebUpdater\bin\Debug\AppLimit.CloudComputing.SharpBox.dll .\bin\Debug
echo F | xcopy /F ..\..\..\..\Tools\WebUpdater\WebUpdater\bin\Debug\Newtonsoft.Json.Net20.dll .\bin\Debug

echo F | xcopy /F ..\..\..\..\GUI\Controls\Validations\Validations\bin\Debug\Validations.dll .\bin\Debug\Modules

@echo Copying Library ImageTilerControl / Thorlabs_IP_Library
echo F | xcopy /F ..\..\..\..\GUI\Controls\ImageTilerControl\ImageTilerControl\lib\Thorlabs_IP_Library.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\ImageTilerControl\ImageTilerControl\bin\Debug\ImageTilerControl.dll .\bin\Debug\Modules

echo F | xcopy /F ..\..\..\..\GUI\Controls\TilesDisplay\TilesDisplay\bin\Debug\TilesDisplay.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\XYTileControl\XYTileControl\bin\Debug\XYTileControl.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\VtkVolumeControl\VtkVolumeControl64\bin\Debug\VtkVolumeControl.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\VolumeControlInterface\VolumeControlInterface\bin\Debug\VolumeControlInterface.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\SpinnerProgress\SpinnerProgress\bin\Debug\SpinnerProgress.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\DigitalOutputSwitches\DigitalOutputSwitches\bin\Debug\DigitalOutputSwitches.dll .\bin\Debug\Modules

echo F | xcopy /F ..\..\..\..\GUI\Controls\JPEGer_Process\JPEGer_Process\\bin\Debug\JPEGer_Process.exe .\bin\Debug\
echo F | xcopy /F ..\..\..\..\GUI\Controls\TileBuilder\x64\Debug\TileBuilder.dll .\bin\Debug
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorDetectorSwitch\MCLWrapper\MCLWrapper\bin\Debug\mcl_RF_Switch_Controller64.dll .\bin\Debug
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorDetectorSwitch\MCLWrapper\MCLWrapper\bin\Debug\MCLWrapper.dll .\bin\Debug

echo F | xcopy /F ..\..\..\..\Tools\Gong\Debug\NET35\*.dll .\bin\Debug\Modules

echo F | xcopy /F ..\..\..\..\Tools\ActiViz-5.8.0\x64\Cosmo.dll .\bin\Debug\Lib
echo F | xcopy /F ..\..\..\..\Tools\ActiViz-5.8.0\x64\K*.* .\bin\Debug\Lib
echo F | xcopy /F ..\..\..\..\Tools\ActiViz-5.8.0\x64\V*.* .\bin\Debug\Lib
move .\bin\Debug\Lib\Kitware.mummy.Runtime.dll .\bin\Debug
move .\bin\Debug\Lib\Kitware.VTK.dll .\bin\Debug
move .\bin\Debug\Lib\Kitware.VTK.vtkFiltering.Unmanaged.dll .\bin\Debug

echo F | xcopy /F ..\..\..\..\GUI\Controls\ROIStatsChart\ROIStatsChart\bin\Debug\ROIStatsChart.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\..\GUI\Controls\ROIStatsChart\ROIStatsChart\bin\Debug\Abt.Controls.SciChart.Wpf.2.3.dll .\bin\Debug\
echo F | xcopy /F ..\..\..\..\GUI\Controls\ROIStatsChart\ROIStatsChart\bin\Debug\Abt.Controls.SciChart.Wpf.2.3.xml .\bin\Debug\

echo F | xcopy /F ..\..\..\..\GUI\Controls\XYTileControl\XYTileControl\bin\Debug\Abt.Controls.SciChart.Wpf.dll .\bin\Debug\
echo F | xcopy /F ..\..\..\..\GUI\Controls\XYTileControl\XYTileControl\bin\Debug\Abt.Controls.SciChart.Wpf.xml .\bin\Debug\

echo F | xcopy /F ..\..\..\..\GUI\Controls\KuriosControl\KuriosControl\bin\Debug\KuriosControl.dll .\bin\Debug\Modules

::ModuleNative:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::


echo F | xcopy /F ..\..\..\..\Common\ExperimentManager\x64\Debug\ExperimentManager.dll .\bin\Debug
echo F | xcopy /F ..\..\..\..\Common\ImageManager\x64\Debug\ImageManager.dll .\bin\Debug\
echo F | xcopy /F ..\..\..\..\Common\ResourceManager\x64\Debug\ResourceManager.dll .\bin\Debug\
echo F | xcopy /F ..\..\..\..\Common\RoiDataStore\x64\Debug\RoiDataStore.dll .\bin\Debug
echo F | xcopy /F ..\..\..\..\Common\ImageStoreLibrary\bin\vs2012\Debug\*.dll .\bin\Debug\Modules_Native
echo F | xcopy /F ..\..\..\..\Common\Sample\x64\Debug\Sample.dll .\bin\Debug\Modules_Native
echo F | xcopy /F ..\..\..\..\Common\HardwareCom\x64\Debug\HardwareCom.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Common\HologramGenerator\x64\Debug\HologramGenerator.dll .\bin\Debug\Modules_Native
echo F | xcopy /F ..\..\..\..\Common\StatsManager\x64\Debug\StatsManager.dll .\bin\Debug
REM echo F | xcopy /F ..\..\..\..\Common\ThorLogging\x64\Debug\ThorLoggingUnmanagedDisabled.dll .\bin\Debug\Modules_Native\ThorLoggingUnmanaged.dll
REM echo F | xcopy /F ..\..\..\..\Common\WinDVI\x64\Debug\WinDVI.dll .\bin\Debug\Modules_Native
echo F | xcopy /F ..\..\..\..\Common\ThorLogging\x64\Debug\ThorLoggingUnmanaged.dll .\bin\Debug\Modules_Native\ThorLoggingUnmanaged.dll
echo F | xcopy /F ..\..\..\..\Common\AutoFocusModule\x64\Debug\AutoFocus.dll .\bin\Debug\Modules_Native

echo F | xcopy /F ..\..\..\..\Commands\ImageAnalysis\PincushionCorrection\x64\Debug\PincushionCorrection.dll .\bin\Debug
echo F | xcopy /F ..\..\..\..\Commands\ImageAnalysis\FlatField\x64\Debug\FlatField.dll .\bin\Debug
echo F | xcopy /F ..\..\..\..\Commands\ImageAnalysis\ImageStats\x64\Debug\ImageStats.dll .\bin\Debug
echo F | xcopy /F ..\..\..\..\Commands\ImageAnalysis\LineProfile\x64\Debug\LineProfile.dll .\bin\Debug
echo F | xcopy /F ..\..\..\..\Commands\ImageAnalysis\ThorImageProcess\x64\Debug\ThorImageProcess.dll .\bin\Debug

echo F | xcopy /F ..\..\..\..\Hardware\Cameras\CameraManager\x64\Debug\CameraManager.dll .\bin\Debug\
echo F | xcopy /F ..\..\..\..\Hardware\Devices\DeviceManager\x64\Debug\DeviceManager.dll .\bin\Debug\

echo F | xcopy /F ..\..\..\..\Tools\HDF5IO\x64\Debug\HDF5IO.dll .\bin\Debug\Modules_Native
echo F | xcopy /F ..\..\..\..\Tools\HDF5IO\HDF5IO\bin\Debug\*.* .\bin\Debug


:: Simulators ::::::::::::::::::::::
echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorConfocalSimulator\x64\Debug\ThorConfocalSimulator.dll .\bin\Debug\Modules_Native
echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorConfocalGalvoSimulator\x64\Debug\ThorConfocalGalvoSimulator.dll .\bin\Debug\Modules_Native
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorZStepperSimulator\x64\Debug\ThorZStepperSimulator.dll .\bin\Debug\Modules_Native
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorPMTSimulator\x64\Debug\ThorPMTSimulator.dll .\bin\Debug\Modules_Native
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorTDCSimulator\x64\Debug\ThorTDCSimulator.dll .\bin\Debug\Modules_Native
echo F | xcopy /F ..\..\..\..\Hardware\Devices\SimDeviceXY\x64\Debug\SimDeviceXY.dll .\bin\Debug\Modules_Native
echo F | xcopy /F ..\..\..\..\Hardware\Devices\SimDeviceFW\x64\Debug\SimDeviceFW.dll .\bin\Debug\Modules_Native
rem echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorBmExpanSimulator\x64\Debug\ThorBmExpanSimulator.dll .\bin\Debug\Modules_Native
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorMCLSSimulator\x64\Debug\ThorMCLSSimulator.dll .\bin\Debug\Modules_Native
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorPinholeStepperSimulator\x64\Debug\ThorPinholeStepperSimulator.dll .\bin\Debug\Modules_Native
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorShutterSimulator\x64\Debug\ThorShutterSimulator.dll .\bin\Debug\Modules_Native
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorLightPathSimulator\x64\Debug\ThorLightPathSimulator.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorMesoScanSimulator\x64\Debug\ThorMesoScanSimulator.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorMesoScanSimulator\TorboJpeg\turbojpeg.dll .\bin\Debug\Lib
REM echo F | xcopy /F ..\..\..\..\Tools\ImageProcessLibrary.1.0.3\build\native\ImageProcessLibrary.dll .\bin\Debug\Lib

:: Devices ::::::::::::::::::::::
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\CoherentChameleon\x64\Debug\CoherentChameleon.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\MaiTaiLaser\x64\Debug\MaiTaiLaser.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorBCM\x64\Debug\ThorBCM.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorBmExpan\x64\Debug\ThorBmExpan.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorBeamStabilizer\x64\Debug\ThorBeamStabilizer.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorVBE\x64\Debug\ThorVBE.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorBScope\x64\Debug\ThorBScope.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorConfocal\x64\Debug\ThorConfocal.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorConfocalGalvo\x64\Debug\ThorConfocalGalvo.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorGGNI\x64\Debug\ThorGGNI.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorMesoScan\x64\Debug\ThorMesoScan.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorStim\x64\Debug\ThorStim.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorTSI\x64\Debug\ThorTSI.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Common\Camera-TSI\dlls\Native_64_lib\bin_CCD_mono\*.dll .\bin\Debug\Lib
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorTSI_CS\x64\Debug\ThorTSI_CS.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Common\Camera-TSI\dlls\Native_64_lib\bin_CMOS_mono\*.dll .\bin\Debug\Lib
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\Common\iDAQ\x64\Debug\iDAQ.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\DCxCamera\x64\Debug\DCxCamera.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ORCA\x64\Debug\ORCA.dll .\bin\Debug\Modules_Native

REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\thordaq\x64\Debug\thordaq.dll .\bin\debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\thordaq\x64\Debug\ThorDAQGalvoGalvo.dll .\bin\debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\thordaq\x64\Debug\ThorDAQResonantGalvo.dll .\bin\debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThordaqDFLIM\x64\Debug\ThordaqDFLIM.dll .\bin\debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThordaqDFLIM\x64\Debug\ThorDFLIMGalvoGalvo.dll .\bin\debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThordaqDFLIM\x64\Debug\ThorDAQGGDFLIMSim.dll .\bin\debug\Modules_Native

REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorBCMPA\x64\Debug\ThorBCMPA.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorBCMPA2\x64\Debug\ThorBCMPA2.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorECU\x64\Debug\ThorECU.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorElectroPhys\x64\Debug\ThorElectroPhys.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorEPiTurret\x64\Debug\ThorEpiTurret.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorDC2200\x64\Debug\ThorDC2200.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorDDR05\x64\Debug\ThorDDR05.dll .\bin\Debug\Modules_Native
echo F | xcopy /F ..\..\..\..\Hardware\Devices\DisconnectedDevice\x64\Debug\Disconnected.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorKurios\x64\Debug\ThorKurios.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorMCLS\x64\Debug\ThorMCLS.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorMCM3000\x64\Debug\ThorMCM3000.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorMCM3000Aux\x64\Debug\ThorMCM3000Aux.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorMCM6000\x64\Debug\ThorMCM6000.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorMCM6000_Condenser\x64\Debug\ThorMCM6000_Condenser.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorMLSStage\x64\Debug\ThorMLSStage.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorObjectiveChanger\x64\Debug\ThorObjectiveChanger.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorPinholeStepper\x64\Debug\ThorPinholeStepper.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorPLSZ\x64\Debug\ThorPLSZ.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorMTS25\x64\Debug\ThorMTS25.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorPMT\x64\Debug\ThorPMT.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorPMT2\x64\Debug\ThorPMT2.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorPMT2100\x64\Debug\ThorPMT2100.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorPowerControl\x64\Debug\ThorPowerControl.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorShutterDigital\x64\Debug\ThorShutterDig.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorShutterDigital2\x64\Debug\ThorShutterDig2.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorShutterDigital3\x64\Debug\ThorShutterDig3.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorShutterDigital4\x64\Debug\ThorShutterDig4.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorShutterDigital5\x64\Debug\ThorShutterDig5.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorShutterDigital6\x64\Debug\ThorShutterDig6.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorTDC\x64\Debug\ThorTDC.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorVBE\x64\Debug\ThorVBE.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorZPiezo\x64\Debug\ThorZPiezo.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorZStepper\x64\Debug\ThorZStepper.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorSLMPDM512\x64\Debug\ThorSLMPDM512.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorSLMPDM512\ThorSLMPDM512\DLL\*.* .\bin\Debug\Lib
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorTiberius\x64\Debug\ThorTiberius.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\PIPiezoXYZ\x64\Debug\PIPiezo.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\OTMLaser\x64\Debug\OTMLaser.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorChrolis\x64\Debug\ThorChrolis.dll .\bin\Debug\Modules_Native
REM echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorLSKGR\x64\Debug\ThorLSKGR.dll .\bin\Debug\Modules_Native

:: Settings Files :::::::::::::::::::::
echo F | xcopy /F ..\..\..\..\Hardware\Devices\CoherentChameleon\CoherentChameleonSettings.xml .\bin\Debug
echo F | xcopy /F ..\..\..\..\Hardware\Devices\MaiTaiLaser\MaiTaiLaserSettings.xml .\bin\Debug
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorBCM\ThorBCMSettings.xml .\bin\Debug
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorBeamStabilizer\ThorBeamStabilizerSettings.xml .\bin\Debug\
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorBmExpan\ThorBmExpanSettings.xml .\bin\Debug\
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorBScope\ThorBScopeSettings.xml .\bin\Debug
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorChrolis\ThorChrolisSettings.xml .\bin\Debug
echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorConfocal\ThorConfocalSettings.xml .\bin\Debug
echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorConfocalGalvo\ThorConfocalGalvoSettings.xml .\bin\Debug
echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorConfocalGalvoSimulator\ThorConfocalGalvoSimulatorSettings.xml .\bin\Debug 
echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorConfocalSimulator\ThorConfocalSimulatorSettings.xml .\bin\Debug
echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorGGNI\ThorGGNISettings.xml .\bin\Debug
echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorMesoScan\ThorMesoScanSettings.xml .\bin\Debug
echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThorStim\ThorStimSettings.xml .\bin\Debug
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorECU\ThorECUSettings.xml .\bin\Debug
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorEpiTurret\ThorEpiTurretSettings.xml .\bin\Debug
echo F | xcopy /F ..\..\..\..\GUI\Applications\ThorImageLS\ThorImage\ThorImageLS.ico .\bin\Debug
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorMCLS\ThorMCLSSettings.xml .\bin\Debug
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorMCM3000\ThorMCM3000Settings.xml .\bin\Debug
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorMCM3000Aux\ThorMCM3000AuxSettings.xml .\bin\Debug
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorMLSStage\ThorMLSStageSettings.xml .\bin\Debug
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorObjectiveChanger\ThorObjectiveChangerSettings.xml .\bin\Debug\
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorPinholeStepper\ThorPinholeStepperSettings.xml .\bin\Debug\
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorPMT\ThorPMTSettings.xml .\bin\Debug
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorPMT2\ThorPMT2Settings.xml .\bin\Debug
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorPMT2100\ThorPMT2100Settings.xml .\bin\Debug
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorPowerControl\PowerControlSettings.xml .\bin\Debug\
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorShutterDigital\ThorShutterDigSettings.xml .\bin\Debug\
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorShutterDigital2\ThorShutterDig2Settings.xml .\bin\Debug\
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorShutterDigital3\ThorShutterDig3Settings.xml .\bin\Debug\
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorShutterDigital4\ThorShutterDig4Settings.xml .\bin\Debug\
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorShutterDigital5\ThorShutterDig5Settings.xml .\bin\Debug\
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorShutterDigital6\ThorShutterDig6Settings.xml .\bin\Debug\
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorZPiezo\ThorZPiezoSettings.xml .\bin\Debug
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorZStepper\ZStepperSettings.xml .\bin\Debug
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorVBE\ThorVBESettings.xml .\bin\Debug
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorElectroPhys\ThorElectroPhysSettings.xml .\bin\Debug
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorBCMPA\ThorBCMPASettings.xml .\bin\Debug
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorDDR05\ThorDDR05Settings.xml .\bin\Debug
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorPLSZ\ThorPLSZSettings.xml .\bin\Debug
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorMTS25\ThorMTS25Settings.xml .\bin\Debug
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorSLMPDM512\ThorSLMPDM512Settings.xml .\bin\Debug
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorTiberius\ThorTiberiusSettings.xml .\bin\Debug
echo F | xcopy /F ..\..\..\..\Hardware\Devices\PIPiezoXYZ\PIPiezoSettings.xml .\bin\Debug
echo F | xcopy /F ..\..\..\..\Hardware\Devices\OTMLaser\OTMLaserSettings.xml .\bin\Debug
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorLSKGR\ThorLSKGRSettings.xml .\bin\Debug
echo F | xcopy /F ..\..\..\..\Hardware\Devices\ThorMCM6000\ThorMCM6000Settings.xml .\bin\Debug

echo F | xcopy /F ..\..\..\..\Hardware\Cameras\thordaq\Dll\ThorDAQResonantGalvo\ThorDAQResonantGalvoSettings.xml .\bin\Debug
echo F | xcopy /F ..\..\..\..\Hardware\Cameras\thordaq\Dll\ThorDAQGalvoGalvo\ThorDAQGalvoGalvoSettings.xml .\bin\Debug

REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThordaqDFLIM\Dll\ThorDFLIMGalvoGalvo\ThorDFLIMGalvoGalvoSettings.xml .\bin\Debug
REM echo F | xcopy /F ..\..\..\..\Hardware\Cameras\ThordaqDFLIM\Dll\ThorDAQGGDFLIMSim\fullFrameSim.bin .\bin\debug

echo F | xcopy /F ..\..\..\..\Common\ResourceManager\ResourceManager.xml .\bin\Debug\

:: Experiment Review:::::::::::::::::::


echo F | xcopy /F ..\..\ExperimentReview\ExperimentReview\bin\Debug\ExperimentReview.exe .\bin\Debug\
echo F | xcopy /F ..\..\ExperimentReview\ExperimentReview\bin\Debug\ExperimentReview.exe.config .\bin\Debug\
echo F | xcopy /F ..\..\ExperimentReview\ExperimentReview\bin\Debug\ExperimentReview.vshost.exe .\bin\Debug\

echo F | xcopy /F ..\..\ExperimentReview\ExperimentReview\bin\Debug\DynamicDataDisplay.xml .\bin\Debug\

:: Matlab Script:::::::::::::::::::::
REM echo F | xcopy /F ..\..\..\..\Common\MatlabEngine\RunTime\mclcom9_1.dll .\bin\Debug\
REM echo F | xcopy /F ..\..\..\..\Common\MatlabEngine\RunTime\mclcommain9_1.dll .\bin\Debug\
REM echo F | xcopy /F ..\..\..\..\Common\MatlabEngine\RunTime\mclmcrrt9_1.dll .\bin\Debug\
REM echo F | xcopy /F ..\..\..\..\Common\MatlabEngine\RunTime\mclxlmain9_1.dll .\bin\Debug\
REM echo F | xcopy /F ..\..\..\..\Common\MatlabEngine\x64\Debug\NativeMatlabEngine.dll .\bin\Debug\
REM echo F | xcopy /F ..\..\..\..\Common\MatlabEngine\MatlabEngine\bin\Debug\MatlabEngine.dll .\bin\Debug\Modules\
REM echo F | xcopy /F ..\..\..\Controls\ScriptPanels\ScriptMatlab\ScriptMatlab\bin\Debug\ScriptMatlab.dll .\bin\Debug\Modules\
REM echo F | xcopy /F ..\..\..\Controls\ScriptManager\ScriptRunMatlabModule\bin\Debug\ScriptRunMatlabModule.dll .\bin\Debug\Modules\