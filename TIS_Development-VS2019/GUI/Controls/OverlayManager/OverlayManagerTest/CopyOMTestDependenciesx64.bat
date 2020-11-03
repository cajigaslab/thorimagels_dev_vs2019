del /Q ".\bin\Debug\Modules\*.*"
del /Q ".\bin\Release\Modules\*.*"

del /Q ".\bin\Debug\Modules_Native\*.*"
del /Q ".\bin\Release\Modules_Native\*.*"
del /Q ".\bin\Debug\Lib\*.*"

del /Q ".\bin\Debug\en-US\*.*"
del /Q ".\bin\Release\en-US\*.*"
del /Q ".\bin\Debug\ru-ru\*.*"
del /Q ".\bin\Release\ru-ru\*.*"

del /Q ".\bin\Debug\*.*"
del /Q ".\bin\Release\*.*"

mkdir ".\bin\Debug\Modules\"
mkdir ".\bin\Release\Modules\"


mkdir ".\bin\Debug\Modules_Native\"
mkdir ".\bin\Release\Modules_Native\"
mkdir ".\bin\Debug\Lib\"

mkdir ".\bin\Debug\en-US\"
mkdir ".\bin\Release\en-US\"
mkdir ".\bin\Debug\ru-ru\"
mkdir ".\bin\Release\ru-ru\"


@echo Copying Library Intel IPP
copy "..\..\..\..\Tools\Intel IPP\intel64\dll\ippcore-7.0.dll" .\bin\Debug
copy "..\..\..\..\Tools\Intel IPP\intel64\dll\ippi-7.0.dll" .\bin\Debug
copy "..\..\..\..\Tools\Intel IPP\intel64\dll\ippiu8-7.0.dll" .\bin\Debug
copy "..\..\..\..\Tools\Intel IPP\intel64\dll\libiomp5md.dll" .\bin\Debug
copy "..\..\..\..\Tools\Intel IPP\intel64\dll\libiomp5md.dll" .\bin\Debug
copy "..\..\..\..\Tools\Intel IPP\intel64\dll\ippcvu8-7.0.dll" .\bin\Debug

copy "..\..\..\..\Tools\Intel IPP\intel64\dll\ippcore-7.0.dll" .\bin\Release
copy "..\..\..\..\Tools\Intel IPP\intel64\dll\ippi-7.0.dll" .\bin\Release
copy "..\..\..\..\Tools\Intel IPP\intel64\dll\ippiu8-7.0.dll" .\bin\Release
copy "..\..\..\..\Tools\Intel IPP\intel64\dll\libiomp5md.dll" .\bin\Release
copy "..\..\..\..\Tools\Intel IPP\intel64\dll\libiomp5md.dll" .\bin\Release
copy "..\..\..\..\Tools\Intel IPP\intel64\dll\ippcvu8-7.0.dll" .\bin\Release

REM echo F | xcopy /F ..\..\..\Controls\ScriptManager\ScriptManager\bin\Debug\ScriptManager.dll .\bin\Debug\Modules
REM echo F | xcopy /F ..\..\..\Controls\ScriptManager\ScriptManager\bin\Release\ScriptManager.dll .\bin\Release\Modules

REM echo F | xcopy /F ..\OverlayManager\bin\Debug\OverlayManager.dll .\bin\Debug\Modules
REM echo F | xcopy /F ..\OverlayManager\bin\Release\OverlayManager.dll .\bin\Release\Modules

echo F | xcopy /F ..\x64\Debug\LiveImageData.dll .\bin\Debug\Modules_Native
echo F | xcopy /F ..\x64\Release\LiveImageData.dll .\bin\Release\Modules_Native

