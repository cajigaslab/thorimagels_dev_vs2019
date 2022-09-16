echo Y | del /Q %1\*.*
del /Q "%1\Modules_Native\*.*"

mkdir "%1\Modules_Native\"

@echo copying Native libraries:
echo F | xcopy /F/Y ..\..\..\..\Tools\HDF5IO\HDF5IO\bin\%2\*.* %1

@echo Copying Library Intel IPP
echo F | xcopy /F/Y "..\..\..\..\Tools\Intel IPP\intel64\dll\ippcore-7.0.dll" %1
echo F | xcopy /F/Y "..\..\..\..\Tools\Intel IPP\intel64\dll\libiomp5md.dll" %1
echo F | xcopy /F/Y "..\..\..\..\Tools\Intel IPP\intel64\dll\ipps-7.0.dll" %1
echo F | xcopy /F/Y "..\..\..\..\Tools\Intel IPP\intel64\dll\ippsy8-7.0.dll" %1

@echo copying Modules_Native:
echo F | xcopy /F/Y ..\..\..\..\Tools\HDF5IO\x64\%2\HDF5IO.dll %1\Modules_Native
echo F | xcopy /F/Y ..\..\..\..\Hardware\Devices\ThorRealTimeData\x64\%2\ThorRealTimeData.dll %1\Modules_Native
echo F | xcopy /F/Y ..\..\..\..\Common\ThorLogging\x64\%2\ThorLoggingUnmanaged.dll %1\Modules_Native\ThorLoggingUnmanaged.dll

echo F | xcopy /F/Y ..\..\..\..\GUI\Controls\ThemeControl\ThemeControl\bin\Debug\ThemeControl.dll %1

echo F | xcopy /F/Y ..\..\..\..\GUI\Controls\RealTimeLineChart\RealTimeLineChart\bin\Debug\RealTimeLineChart.dll %1
echo F | xcopy /F/Y ..\..\..\..\Common\ThorSharedTypes\ThorSharedTypes\bin\Debug\ThorSharedTypes.dll .\bin\Debug

@echo copying Settings:
echo F | xcopy /F/Y ..\..\..\..\Hardware\Devices\ThorRealTimeData\ThorRealTimeDataSettings.xml %1

REM @echo copying Logger:
REM echo F | xcopy /F/Y "..\..\..\..\Tools\log4cxx0.10.0\msvc9-proj\x64\Debug\log4cxx.dll" %1
REM echo F | xcopy /F/Y "..\..\..\..\Installers\ThorImageLSx64\ThorImageLSx64\ThorLogging.xml" %1
REM echo F | xcopy /F/Y ..\..\..\..\Common\ThorLogging\x64\%2\ThorLoggingUnmanaged.dll %1\Modules_Native\ThorLoggingUnmanaged.dll