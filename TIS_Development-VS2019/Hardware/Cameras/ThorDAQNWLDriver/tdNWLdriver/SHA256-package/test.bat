REM Destroy and recreate temporary folder  use arg "-r" for Release, default Debug
rmdir /q /s .\disk1
mkdir .\disk1

set buildType=Debug

if %1%.==. GOTO runCmd
set buildType=Release

:runCmd
echo %buildType%

REM  Copy the dev files from Visual Studio - may have test signatures

copy /y ..\..\..\tdNWLdriver\x64\%buildType%\tdNWLdriver.pdb .\disk1
copy /y ..\..\..\tdNWLdriver\x64\%buildType%\tdNWLdriver.sys .\disk1
copy /y ..\..\..\tdNWLdriver\x64\%buildType%\tdDMADriver.inf .\disk1
copy /y ..\..\..\tdNWLdriver\x64\%buildType%\tdNWLdriver\tdDMADx64.cat   .\disk1