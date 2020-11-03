call del ".\build.txt"

echo %time% >> ".\build.txt"

mkdir ".\..\..\GUI\Applications\ThorImageLS\ThorImage\bin\Debug\en-US"
mkdir ".\..\..\GUI\Applications\ThorImageLS\ThorImage\bin\Debug\Modules\MVM"
mkdir ".\..\..\GUI\Applications\ThorImageLS\ThorImage\bin\Debug\Modules_Native"

call devenv /out ".\build.txt" /Clean "Debug|x64" BuildAllSolution2012.sln 
call devenv /out ".\build.txt" /Rebuild "Debug|x64" BuildAllSolution2012.sln 


call devenv /out ".\build.txt" /Clean "Debug|Any CPU" BuildAllSolution2012.sln 
call devenv /out ".\build.txt" /Rebuild "Debug|Any CPU" BuildAllSolution2012.sln 
call CopyModulesNativex64.bat

echo %time% >> ".\build.txt"