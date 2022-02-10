
@echo off
setlocal ENABLEDELAYEDEXPANSION

set versionstring=%2
set /a "i=0"

:stringLOOP
    REM Stop when the string is empty
    if "!versionstring!" EQU "" goto END

    for /f "delims=." %%a in ("!versionstring!") do set substring=%%a
        echo !substring!
	

	for /f "tokens=1,* delims=]" %%B in ('"type %1\version.ver|find /n "!i!!i!!i!!i!""') do (
	set "line=%%C"
	if defined line (
	    call set "line=echo.%%line:!i!!i!!i!!i!=!substring!%%"
	    for /f "delims=]" %%X in ('"echo."%%line%%""') do %%~X  >>  %1\output.ver
	    ) ELSE echo.
	)
	
	set /a "i = i +1"


REM strip off the leading substring
:striploop
    set stripchar=!versionstring:~0,1!
    set versionstring=!versionstring:~1!
	
    if "!versionstring!" EQU "" goto stringloop

    if "!stripchar!" NEQ "." goto striploop

    goto stringloop
)


:END

call attrib -R "%1\version.ver"
del "%1\version.ver"
copy "%1\output.ver" "%1\version.ver"

REM Create the Debug folder of ThorImage for BuildAll Debug Any-CPU
mkdir "%1\..\GUI\Applications\ThorImageLS\ThorImage\bin\Debug\en-US"
mkdir "%1\..\GUI\Applications\ThorImageLS\ThorImage\bin\Debug\Modules\MVM"
mkdir "%1\..\GUI\Applications\ThorImageLS\ThorImage\bin\Debug\Modules_Native"
mkdir "%1\..\GUI\Applications\ThorSync\ThorSync\bin\Debug\"


endlocal