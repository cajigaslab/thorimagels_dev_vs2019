del /Q ".\bin\Debug\Modules_Native\*.*"

del /Q ".\bin\Debug\*.*"

mkdir ".\bin\Debug\Modules_Native\"

echo F | xcopy /F .\Modules_Native\*.* .\bin\Debug\Modules_Native

rem echo F | xcopy /F ..\Library\*.* .\bin\Debug

rem echo F | xcopy /F ..\..\..\..\GUI\Controls\SpinnerProgress\SpinnerProgress\bin\Debug\SpinnerProgress.dll .\bin\Debug\Modules
echo F | xcopy /F ..\..\..\..\Common\ThorLogging\ThorLogging.xml .\bin\Debug
rem echo F | xcopy /F ..\..\..\..\Common\ThorLogging\x64\Debug\ThorLoggingUnmanaged.dll .\bin\Debug\Modules_Native
rem echo F | xcopy /F ..\..\..\..\Common\ThorSharedTypes\ThorSharedTypes\bin\Debug\ThorSharedTypes.dll .\bin\Debug

rem echo F | xcopy /F ..\..\..\..\Hardware\Cameras\thordaq\x64\Debug\thordaq.dll .\bin\debug\Modules_Native
rem echo F | xcopy /F ..\..\..\..\Hardware\Cameras\thordaq\x64\Debug\ThorDAQGalvoGalvo.dll .\bin\debug\Modules_Native
rem echo F | xcopy /F ..\..\..\..\Hardware\Cameras\thordaq\x64\Debug\ThorDAQResonantGalvo.dll .\bin\debug\Modules_Native

rem echo F | xcopy /F ..\..\..\..\Hardware\Cameras\thordaq\Dll\ThorDAQResonantGalvo\ThorDAQResonantGalvoSettings.xml .\bin\Debug
rem echo F | xcopy /F ..\..\..\..\Hardware\Cameras\thordaq\Dll\ThorDAQGalvoGalvo\ThorDAQGalvoGalvoSettings.xml .\bin\Debug

echo F | xcopy /F .\ThorDAQSettings.xml .\bin\Debug