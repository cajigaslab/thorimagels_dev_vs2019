call del ".\build-release.txt"

echo %time% >> ".\build-release.txt"

call devenv /out ".\build-release.txt" /Clean "Release|x64" BuildAllSolution2012.sln 
call devenv /out ".\build-release.txt" /Rebuild "Release|x64" BuildAllSolution2012.sln 


call devenv /out ".\build-release.txt" /Clean "Release|Any CPU" BuildAllSolution2012.sln 
call devenv /out ".\build-release.txt" /Rebuild "Release|Any CPU" BuildAllSolution2012.sln 


call CopyModulesNativex64-Release.bat

echo %time% >> ".\build-release.txt"