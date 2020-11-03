REM CopySingleDependency {Source Dependency} {Debug|Release} {Win32|x64} {OutputDir}
echo CopySingleDependency %1 %2 %3 %4

:EXCEPTIONS
if "%~nx1"=="simcam.dll" goto SKIP

:COPY_DEPENDENCY

REM Copy DLL/EXE
echo copy /Y %1 ..\ThorTSI\Lib\%3\%2
copy /Y %1 ..\ThorTSI\Lib\%3\%2
echo copy /Y %1 %4
copy /Y %1 %4

REM Copy Symbols
echo copy /Y %~dpn1.pdb ..\ThorTSI\Lib\%3\%2
copy /Y %~dpn1.pdb ..\ThorTSI\Lib\%3\%2
echo copy /Y %~dpn1.pdb %4
copy /Y %~dpn1.pdb %4

goto DONE

:SKIP
echo Skipping %~nx1
goto DONE

:DONE
