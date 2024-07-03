REM Destroy and recreate temporary folder  use arg "-r" for Release, default Debug
rmdir /q /s .\disk1
mkdir .\disk1

set buildType=Debug

REM   if %1%.==. GOTO runCmd
set buildType=Release

:runCmd
echo %buildType%

REM  Copy the dev files from Visual Studio - may have test signatures
REM  vc142.pdb symbols exist in both Debug and Release builds

copy /y ..\x64\%buildType%\TDWin10dFLIMdrv\TDWin10dFLIMdrv.sys .\disk1
copy /y ..\x64\%buildType%\TDWin10dFLIMdrv\TDWin10dFLIMdrv.inf .\disk1
copy /y ..\x64\%buildType%\TDWin10dFLIMdrv\TDWin10dFLIMdrv.cat .\disk1
copy /y ..\x64\%buildType%\*.pdb .\disk1
REM we MUST remove (default test) signature always applied in Dev cycle
signtool remove /v /s .\disk1\TDWin10dFLIMdrv.sys

REM re-sign the kernel driver .SYS file
signtool sign /v /sha1 203bdda663f0b63d93ddc9137b62a9f22a4b4aff /fd sha256 /tr http://timestamp.digicert.com /td sha256 ".\disk1\TDWin10dFLIMdrv.sys"

REM create the cabinet file "package" format required by partner.microsoft.com Portal
makecab /v3 /f ThorDAQdriverCABconfig.ddf

REM sign our .CAB file (again, all cert signatures will be replaced by MS)
signtool sign /v /sha1 203bdda663f0b63d93ddc9137b62a9f22a4b4aff /fd sha256 /tr http://timestamp.digicert.com /td sha256 ".\disk1\ThorDAQdFLIMdrv.cab"

REM sanity check signature with "kernel" policy
signtool verify /v /kp .\disk1\ThorDAQdFLIMdrv.cab

REM sanity check the .INF file; Partner Center verification will fail to sign on any infverif error
InfVerif /info .\disk1\TDWin10dFLIMdrv.inf

