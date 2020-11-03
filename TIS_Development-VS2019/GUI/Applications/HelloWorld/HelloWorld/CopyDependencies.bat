
del /Q ".\bin\Debug\Modules\*.*"
del /Q ".\bin\Release\Modules\*.*"

del /Q ".\bin\Debug\Modules_Native\*.*"
del /Q ".\bin\Release\Modules_Native\*.*"


copy ..\HelloWorldInfastructure\bin\Debug\HelloWorldInfastructure.dll .\bin\Debug\Modules
copy ..\HelloWorldModule\bin\Debug\HelloWorldModule.dll .\bin\Debug\Modules
copy ..\ScriptModule\bin\Debug\ScriptModule.dll .\bin\Debug\Modules

copy ..\..\..\..\Commands\Acquisition\LiveImageData\LiveImageDataModule\bin\Debug\en-US\LiveImageDataModule.resources.dll .\bin\Debug\en-US\LiveImageDataModule.resources.dll

copy ..\..\..\..\Common\Sample\Debug\Sample.dll .\bin\Debug\Modules_Native
copy ..\..\..\..\Common\Sample\Release\Sample.dll .\bin\Debug\Modules_Native

copy ..\..\..\..\Common\ThorLogging\ThorLoggingUnmanaged\Debug\ThorLoggingUnmanaged.dll .\bin\Debug\Modules_Native
copy ..\..\..\..\Common\ThorLogging\ThorLoggingUnmanaged\Release\ThorLoggingUnmanaged.dll .\bin\Release\Modules_Native

copy ..\..\..\..\Hardware\Cameras\CameraManager\Debug\CameraManager.dll .\bin\Debug\
copy ..\..\..\..\Hardware\Cameras\CameraManager\Release\CameraManager.dll .\bin\Release\

copy ..\..\..\..\Hardware\Devices\DeviceManager\Debug\DeviceManager.dll .\bin\Debug\
copy ..\..\..\..\Hardware\Devices\DeviceManager\Release\DeviceManager.dll .\bin\Release\


copy ..\..\..\..\Commands\Acquisition\LiveImageData\Debug\LiveImageData.dll .\bin\Debug\Modules_Native
copy ..\..\..\..\Commands\Acquisition\LiveImageData\Release\LiveImageData.dll .\bin\Release\Modules_Native


copy ..\..\..\..\Commands\Acquisition\LiveImageData\LiveImageDataModule\bin\Debug\LiveImageDataModule.dll .\bin\Debug\Modules
copy ..\..\..\..\Commands\Acquisition\LiveImageData\LiveImageDataModule\bin\Release\LiveImageDataModule.dll .\bin\Release\Modules

REM copy ..\..\..\..\Hardware\Cameras\Simulator\Debug\Simulator.dll .\bin\Debug\Modules_Native
REM copy ..\..\..\..\Hardware\Cameras\Simulator\Release\Simulator.dll .\bin\Release\Modules_Native


REM copy .\bin\Debug\Modules_Native\Simulator.dll .\bin\Debug\Modules_Native\Simulator2.dll
REM copy .\bin\Debug\Modules_Native\Simulator.dll .\bin\Debug\Modules_Native\Simulator3.dll

copy ..\..\..\..\Hardware\Cameras\ThorDCU\Debug\ThorDCU.dll .\bin\Debug\Modules_Native
copy ..\..\..\..\Hardware\Cameras\ThorDCU\Release\ThorDCU.dll .\bin\Release\Modules_Native


copy .\bin\Debug\Modules_Native\ThorDCU.dll .\bin\Debug\Modules_Native\ThorDCU2.dll
copy .\bin\Debug\Modules_Native\ThorDCU.dll .\bin\Debug\Modules_Native\ThorDCU3.dll


REM copy ..\..\..\..\Hardware\Devices\ThorSH05\Debug\ThorSH05.dll .\bin\Debug\Modules_Native
REM copy ..\..\..\..\Hardware\Devices\ThorSH05\Release\ThorSH05.dll .\bin\Release\Modules_Native


copy ..\..\..\..\Hardware\Devices\ThorBSC\Debug\ThorBSC.dll .\bin\Debug\Modules_Native
REM copy ..\..\..\..\Hardware\Devices\ThorBSC\Release\ThorBSC.dll .\bin\Release\Modules_Native
