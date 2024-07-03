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

REM we MUST remove (default test) signature always applied in Dev cycle
signtool remove /v /s .\disk1\tdNWLdriver.sys

REM re-sign the kernel driver .SYS file
signtool sign /v /ac "C:\Program Files (x86)\Windows Kits\10\CrossCertificates\DigiCert High Assurance EV Root CA.crt" /sha1 CEB93BF9A0021C9CFCB71EFB3B86A0A031E91BEE /fd sha256 /tr http://timestamp.digicert.com /td sha256 ".\disk1\tdNWLdriver.sys"

REM create the cabinet file "package" format required by partner.microsoft.com Portal
makecab /v3 /f ThorDAQdriverCABconfig.ddf

REM sign our .CAB file (again, all cert signatures will be replaced by MS)
signtool sign /v /ac "C:\Program Files (x86)\Windows Kits\10\CrossCertificates\DigiCert High Assurance EV Root CA.crt" /sha1 CEB93BF9A0021C9CFCB71EFB3B86A0A031E91BEE /fd sha256 /tr http://timestamp.digicert.com /td sha256 ".\disk1\ThorDAQDrv.cab"

REM sanity check signature with "kernel" policy
signtool verify /v /kp .\disk1\ThorDAQDrv.cab

REM sanity check the .INF file; Partner Center verification will fail to sign on any infverif error
InfVerif /info .\disk1\tdDMADriver.inf

