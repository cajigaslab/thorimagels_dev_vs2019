:: %1 is the version number separated by underscores e.g. 4_2_2019_11191
:: This is the second part of the setup for the SDKs, this batch file should always be called after Hardware\SDKs\UpdateBinaries.bat
set LSM-folder="70-0030 LSM_SDK"
set LSMGalvo-folder="70-0031 LSMGalvo_SDK"
set MPH16-folder="70-0034 MPH16_SDK"
set PMT-folder="70-0035 PMT_SDK"
set ECU-folder="70-0029 ECU_SDK"
set MFC1-folder="70-0051 MFC1_SDK"
set PowerControl-folder="70-0036 PowerControl_SDK"
set BScope-folder="70-0028 BScope_SDK"
set BCM-folder="70-0027 BCM_SDK"
set MCM3000-folder="70-0033 MCM3000_SDK"
set MCLS-folder="70-0032 MCLS_SDK"
set PMT2100-folder="70-0037 PMT2100_SDK"
set BCMPA-folder="70-0077 BCM-PA_SDK"
set LSKGR-folder="70-0058 LSKGR_SDK"
set CSE2000-folder="70-0062 CSE2000_SDK"
set CSN210-folder="70-0066 CSN210_SDK"
set MCM6000-folder="70-0101 MCM6000_SDK"

::BuildAllDoxygen::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
set version=%1 
:: %version:_=.% is taking away the underscores and replacing them with dots

::echo "PROJECT_NUMBER = 3.0.2016.09301" >> .\%BScope-folder%\bscopesdk.dox
copy .\%BScope-folder%\bscopesdk.dox .\%BScope-folder%\bscopesdk_tmp.dox

echo PROJECT_NAME = "BScope SDK" >> .\%BScope-folder%\bscopesdk_tmp.dox
echo OUTPUT_DIRECTORY = ".\70-0028 BScope_SDK\Documents" >> .\%BScope-folder%\bscopesdk_tmp.dox
echo INPUT = ".\70-0028 BScope_SDK" >> .\%BScope-folder%\bscopesdk_tmp.dox
echo EXAMPLE_PATH = ".\70-0028 BScope_SDK\LabVIEW\samples" >> .\%BScope-folder%\bscopesdk_tmp.dox
echo PROJECT_NUMBER = %version:_=.% >> .\%BScope-folder%\bscopesdk_tmp.dox

"C:\Program Files\doxygen\bin\doxygen.exe" ".\70-0028 BScope_SDK\bscopesdk_tmp.dox"
del .\%BScope-folder%\bscopesdk_tmp.dox


::echo "PROJECT_NUMBER = 3.0.2016.09301" >> .\%ECU-folder%\ecusdk.dox
copy .\%ECU-folder%\ecusdk.dox .\%ECU-folder%\ecusdk_tmp.dox

echo PROJECT_NAME = "ECU SDK" >> .\%ECU-folder%\ecusdk_tmp.dox
echo OUTPUT_DIRECTORY = ".\70-0029 ECU_SDK\Documents" >> .\%ECU-folder%\ecusdk_tmp.dox
echo INPUT = ".\70-0029 ECU_SDK" >> .\%ECU-folder%\ecusdk_tmp.dox
echo EXAMPLE_PATH = ".\70-0029 ECU_SDK\LabVIEW\samples" >> .\%ECU-folder%\ecusdk_tmp.dox
echo PROJECT_NUMBER = %version:_=.% >> .\%ECU-folder%\ecusdk_tmp.dox

"C:\Program Files\doxygen\bin\doxygen.exe" ".\70-0029 ECU_SDK\ecusdk_tmp.dox"
del .\%ECU-folder%\ecusdk_tmp.dox


::echo "PROJECT_NUMBER = 3.0.2016.09301" >> .\%LSM-folder%\lsmsdk.dox
copy .\%LSM-folder%\lsmsdk.dox .\%LSM-folder%\lsmsdk_tmp.dox

echo PROJECT_NAME = "LSM SDK" >> .\%LSM-folder%\lsmsdk_tmp.dox
echo OUTPUT_DIRECTORY = ".\70-0030 LSM_SDK\Documents" >> .\%LSM-folder%\lsmsdk_tmp.dox
echo INPUT = ".\70-0030 LSM_SDK" >> .\%LSM-folder%\lsmsdk_tmp.dox
echo EXAMPLE_PATH = ".\70-0030 LSM_SDK\LabVIEW\samples" >> .\%LSM-folder%\lsmsdk_tmp.dox
echo PROJECT_NUMBER = %version:_=.% >> .\%LSM-folder%\lsmsdk_tmp.dox

"C:\Program Files\doxygen\bin\doxygen.exe" ".\70-0030 LSM_SDK\lsmsdk_tmp.dox"
del .\%LSM-folder%\lsmsdk_tmp.dox


::echo "PROJECT_NUMBER = 3.0.2016.09301" >> .\%LSMGalvo-folder%\lsmgalvosdk.dox
copy .\%LSMGalvo-folder%\lsmgalvosdk.dox .\%LSMGalvo-folder%\lsmgalvosdk_tmp.dox

echo PROJECT_NAME = "LSMGalvo SDK" >> .\%LSMGalvo-folder%\lsmgalvosdk_tmp.dox
echo OUTPUT_DIRECTORY = ".\70-0031 LSMGalvo_SDK\Documents" >> .\%LSMGalvo-folder%\lsmgalvosdk_tmp.dox
echo INPUT = ".\70-0031 LSMGalvo_SDK" >> .\%LSMGalvo-folder%\lsmgalvosdk_tmp.dox
echo EXAMPLE_PATH = ".\70-0031 LSMGalvo_SDK\LabVIEW\samples" >> .\%LSMGalvo-folder%\lsmgalvosdk_tmp.dox
echo PROJECT_NUMBER = %version:_=.% >> .\%LSMGalvo-folder%\lsmgalvosdk_tmp.dox

"C:\Program Files\doxygen\bin\doxygen.exe" ".\70-0031 LSMGalvo_SDK\lsmgalvosdk_tmp.dox"
del .\%LSMGalvo-folder%\lsmgalvosdk_tmp.dox


::echo "PROJECT_NUMBER = 3.0.2016.09301" >> .\%MCM3000-folder%\mcm3000sdk.dox
copy .\%MCM3000-folder%\mcm3000sdk.dox .\%MCM3000-folder%\mcm3000sdk_tmp.dox

echo PROJECT_NAME = "MCM3000 SDK" >> .\%MCM3000-folder%\mcm3000sdk_tmp.dox
echo OUTPUT_DIRECTORY = ".\70-0033 MCM3000_SDK\Documents" >> .\%MCM3000-folder%\mcm3000sdk_tmp.dox
echo INPUT = ".\70-0033 MCM3000_SDK" >> .\%MCM3000-folder%\mcm3000sdk_tmp.dox
echo EXAMPLE_PATH = ".\70-0033 MCM3000_SDK\cpp\MCM3000PositionTest\MCM3000PositionTest" >> .\%MCM3000-folder%\mcm3000sdk_tmp.dox
echo PROJECT_NUMBER = %version:_=.% >> .\%MCM3000-folder%\mcm3000sdk_tmp.dox

"C:\Program Files\doxygen\bin\doxygen.exe" ".\70-0033 MCM3000_SDK\mcm3000sdk_tmp.dox"
del .\%MCM3000-folder%\mcm3000sdk_tmp.dox

::echo "PROJECT_NUMBER = 3.0.2016.09301" >> .\%MCM6000-folder%\mcm6000sdk.dox
copy .\%MCM6000-folder%\mcm6000sdk.dox .\%MCM6000-folder%\mcm6000sdk_tmp.dox

echo PROJECT_NAME = "MCM6000 SDK" >> .\%MCM6000-folder%\mcm6000sdk_tmp.dox
echo OUTPUT_DIRECTORY = ".\70-0101 MCM6000_SDK\Documents" >> .\%MCM6000-folder%\mcm6000sdk_tmp.dox
echo INPUT = ".\70-0101 MCM6000_SDK" >> .\%MCM6000-folder%\mcm6000sdk_tmp.dox
echo EXAMPLE_PATH = ".\70-0101 MCM6000_SDK\cpp\MCM6000PositionTest\MCM6000PositionTest" >> .\%MCM6000-folder%\mcm6000sdk_tmp.dox
echo PROJECT_NUMBER = %version:_=.% >> .\%MCM6000-folder%\mcm6000sdk_tmp.dox

"C:\Program Files\doxygen\bin\doxygen.exe" ".\70-0101 MCM6000_SDK\mcm6000sdk_tmp.dox"
del .\%MCM6000-folder%\mcm6000sdk_tmp.dox


::echo "PROJECT_NUMBER = 3.0.2016.09301" >> .\%MCLS-folder%\mclssdk.dox
copy .\%MCLS-folder%\mclssdk.dox .\%MCLS-folder%\mclssdk_tmp.dox

echo PROJECT_NAME = "MCLS SDK" >> .\%MCLS-folder%\mclssdk_tmp.dox
echo OUTPUT_DIRECTORY = ".\70-0032 MCLS_SDK\Documents" >> .\%MCLS-folder%\mclssdk_tmp.dox
echo INPUT = ".\70-0032 MCLS_SDK" >> .\%MCLS-folder%\mclssdk_tmp.dox
echo EXAMPLE_PATH = ".\70-0032 MCLS_SDK\LabVIEW\samples" >> .\%MCLS-folder%\mclssdk_tmp.dox
echo PROJECT_NUMBER = %version:_=.% >> .\%MCLS-folder%\mclssdk_tmp.dox

"C:\Program Files\doxygen\bin\doxygen.exe" ".\70-0032 MCLS_SDK\mclssdk_tmp.dox"
del .\%MCLS-folder%\mclssdk_tmp.dox


::echo "PROJECT_NUMBER = 3.0.2016.09301" >> .\%MPH16-folder%\mph16sdk.dox
copy .\%MPH16-folder%\mph16sdk.dox .\%MPH16-folder%\mph16sdk_tmp.dox

echo PROJECT_NAME = "MPH16 SDK" >> .\%MPH16-folder%\mph16sdk_tmp.dox
echo OUTPUT_DIRECTORY = ".\70-0034 MPH16_SDK\Documents" >> .\%MPH16-folder%\mph16sdk_tmp.dox
echo INPUT = ".\70-0034 MPH16_SDK" >> .\%MPH16-folder%\mph16sdk_tmp.dox
echo EXAMPLE_PATH = ".\70-0034 MPH16_SDK\LabVIEW\samples" >> .\%MPH16-folder%\mph16sdk_tmp.dox
echo PROJECT_NUMBER = %version:_=.% >> .\%MPH16-folder%\mph16sdk_tmp.dox

"C:\Program Files\doxygen\bin\doxygen.exe" ".\70-0034 MPH16_SDK\mph16sdk_tmp.dox"
del .\%MPH16-folder%\mph16sdk_tmp.dox


::echo "PROJECT_NUMBER = 3.0.2016.09301" >> .\%PMT-folder%\pmtsdk.dox
copy .\%PMT-folder%\pmtsdk.dox .\%PMT-folder%\pmtsdk_tmp.dox

echo PROJECT_NAME = "PMT SDK" >> .\%PMT-folder%\pmtsdk_tmp.dox
echo OUTPUT_DIRECTORY = ".\70-0035 PMT_SDK\Documents" >> .\%PMT-folder%\pmtsdk_tmp.dox
echo INPUT = ".\70-0035 PMT_SDK" >> .\%PMT-folder%\pmtsdk_tmp.dox
echo EXAMPLE_PATH = ".\70-0035 PMT_SDK\LabVIEW\samples" >> .\%PMT-folder%\pmtsdk_tmp.dox
echo PROJECT_NUMBER = %version:_=.% >> .\%PMT-folder%\pmtsdk_tmp.dox

"C:\Program Files\doxygen\bin\doxygen.exe" ".\70-0035 PMT_SDK\pmtsdk_tmp.dox"
del .\%PMT-folder%\pmtsdk_tmp.dox


::echo "PROJECT_NUMBER = 3.0.2016.09301" >> .\%PowerControl-folder%\powersdk.dox
copy .\%PowerControl-folder%\powersdk.dox .\%PowerControl-folder%\powersdk_tmp.dox

echo PROJECT_NAME = "PowerControl SDK" >> .\%PowerControl-folder%\powersdk_tmp.dox
echo OUTPUT_DIRECTORY = ".\70-0036 PowerControl_SDK\Documents" >> .\%PowerControl-folder%\powersdk_tmp.dox
echo INPUT = ".\70-0036 PowerControl_SDK" >> .\%PowerControl-folder%\powersdk_tmp.dox
echo EXAMPLE_PATH = ".\70-0036 PowerControl_SDK\LabVIEW\samples" >> .\%PowerControl-folder%\powersdk_tmp.dox
echo PROJECT_NUMBER = %version:_=.% >> .\%PowerControl-folder%\powersdk_tmp.dox

"C:\Program Files\doxygen\bin\doxygen.exe" ".\70-0036 PowerControl_SDK\powersdk_tmp.dox"
del .\%PowerControl-folder%\powersdk_tmp.dox


::echo "PROJECT_NUMBER = 3.0.2016.09301" >> .\%MFC1-folder%\zsteppersdk.dox
copy .\%MFC1-folder%\zsteppersdk.dox .\%MFC1-folder%\zsteppersdk_tmp.dox

echo PROJECT_NAME = "MFC1 SDK" >> .\%MFC1-folder%\zsteppersdk_tmp.dox
echo OUTPUT_DIRECTORY = ".\70-0051 MFC1_SDK\Documents" >> .\%MFC1-folder%\zsteppersdk_tmp.dox
echo INPUT = ".\70-0051 MFC1_SDK" >> .\%MFC1-folder%\zsteppersdk_tmp.dox
echo EXAMPLE_PATH = ".\70-0051 MFC1_SDK\LabVIEW\samples" >> .\%MFC1-folder%\zsteppersdk_tmp.dox
echo PROJECT_NUMBER = %version:_=.% >> .\%MFC1-folder%\zsteppersdk_tmp.dox

"C:\Program Files\doxygen\bin\doxygen.exe" ".\70-0051 MFC1_SDK\zsteppersdk_tmp.dox"
del .\%MFC1-folder%\zsteppersdk_tmp.dox


::echo "PROJECT_NUMBER = 3.0.2016.09301" >> .\%BCM-folder%\bcmsdk.dox
copy .\%BCM-folder%\bcmsdk.dox .\%BCM-folder%\bcmsdk_tmp.dox

echo PROJECT_NAME = "BCM SDK" >> .\%BCM-folder%\bcmsdk_tmp.dox
echo OUTPUT_DIRECTORY = ".\70-0027 BCM_SDK\Documents" >> .\%BCM-folder%\bcmsdk_tmp.dox
echo INPUT = ".\70-0027 BCM_SDK" >> .\%BCM-folder%\bcmsdk_tmp.dox
echo EXAMPLE_PATH = ".\70-0027 BCM_SDK\LabVIEW\samples" >> .\%BCM-folder%\bcmsdk_tmp.dox
echo PROJECT_NUMBER = %version:_=.% >> .\%BCM-folder%\bcmsdk_tmp.dox

"C:\Program Files\doxygen\bin\doxygen.exe" ".\70-0027 BCM_SDK\bcmsdk_tmp.dox"
del .\%BCM-folder%\bcmsdk_tmp.dox


::echo "PROJECT_NUMBER = 3.0.2016.09301" >> .\%PMT2100-folder%\pmt2100sdk.dox
copy .\%PMT2100-folder%\pmt2100sdk.dox .\%PMT2100-folder%\pmt2100sdk_tmp.dox

echo PROJECT_NAME = "PMT2100 SDK" >> .\%PMT2100-folder%\pmt2100sdk_tmp.dox
echo OUTPUT_DIRECTORY = ".\70-0037 PMT2100_SDK\Documents" >> .\%PMT2100-folder%\pmt2100sdk_tmp.dox
echo INPUT = ".\70-0037 PMT2100_SDK" >> .\%PMT2100-folder%\pmt2100sdk_tmp.dox
echo EXAMPLE_PATH = ".\70-0037 PMT2100_SDK\cpp\PMT2100Test\PMT2100Test" >> .\%PMT2100-folder%\pmt2100sdk_tmp.dox
echo PROJECT_NUMBER = %version:_=.% >> .\%PMT2100-folder%\pmt2100sdk_tmp.dox

"C:\Program Files\doxygen\bin\doxygen.exe" ".\70-0037 PMT2100_SDK\pmt2100sdk_tmp.dox"
del .\%PMT2100-folder%\pmt2100sdk_tmp.dox


::echo "PROJECT_NUMBER = 3.0.2016.09301" >> .\%BCMPA-folder%\bcmpasdk.dox
copy .\%BCMPA-folder%\bcmpasdk.dox .\%BCMPA-folder%\bcmpasdk_tmp.dox

echo PROJECT_NAME = "BCMPA SDK" >> .\%BCMPA-folder%\bcmpasdk_tmp.dox
echo OUTPUT_DIRECTORY = ".\70-0077 BCM-PA_SDK\Documents" >> .\%BCMPA-folder%\bcmpasdk_tmp.dox
echo INPUT = ".\70-0077 BCM-PA_SDK" >> .\%BCMPA-folder%\bcmpasdk_tmp.dox
echo EXAMPLE_PATH = ".\70-0077 BCM-PA_SDK\cpp\BCM-PAPositionTest\BCM-PAPositionTest" >> .\%BCMPA-folder%\bcmpasdk_tmp.dox
echo PROJECT_NUMBER = %version:_=.% >> .\%BCMPA-folder%\bcmpasdk_tmp.dox

"C:\Program Files\doxygen\bin\doxygen.exe" ".\70-0077 BCM-PA_SDK\bcmpasdk_tmp.dox"
del .\%BCMPA-folder%\bcmpasdk_tmp.dox


::echo "PROJECT_NUMBER = 3.0.2016.09301" >> .\%LSKGR-folder%\lskgrsdk.dox
copy .\%LSKGR-folder%\lskgrsdk.dox .\%LSKGR-folder%\lskgrsdk_tmp.dox

echo PROJECT_NAME = "LSKGR SDK" >> .\%LSKGR-folder%\lskgrsdk_tmp.dox
echo OUTPUT_DIRECTORY = ".\70-0058 LSKGR_SDK\Documents" >> .\%LSKGR-folder%\lskgrsdk_tmp.dox
echo INPUT = ".\70-0058 LSKGR_SDK" >> .\%LSKGR-folder%\lskgrsdk_tmp.dox
echo EXAMPLE_PATH = ".\70-0058 LSKGR_SDK\cpp\LSKGRPositionTest\LSKGRPositionTest" >> .\%LSKGR-folder%\lskgrsdk_tmp.dox
echo PROJECT_NUMBER = %version:_=.% >> .\%LSKGR-folder%\lskgrsdk_tmp.dox

"C:\Program Files\doxygen\bin\doxygen.exe" ".\70-0058 LSKGR_SDK\lskgrsdk_tmp.dox"
del .\%LSKGR-folder%\lskgrsdk_tmp.dox

::echo "PROJECT_NUMBER = 3.0.2016.09301" >> .\%CSE2000-folder%\cse2000sdk.dox
copy .\%CSE2000-folder%\cse2000sdk.dox .\%CSE2000-folder%\cse2000sdk_tmp.dox

echo PROJECT_NAME = "CSE2000 SDK" >> .\%CSE2000-folder%\cse2000sdk_tmp.dox
echo OUTPUT_DIRECTORY = ".\70-0062 CSE2000_SDK\Documents" >> .\%CSE2000-folder%\cse2000sdk_tmp.dox
echo INPUT = ".\70-0062 CSE2000_SDK" >> .\%CSE2000-folder%\cse2000sdk_tmp.dox
echo EXAMPLE_PATH = ".\70-0062 CSE2000_SDK\cpp\CSE2000PositionTest\CSE2000PositionTest" >> .\%CSE2000-folder%\cse2000sdk_tmp.dox
echo PROJECT_NUMBER = %version:_=.% >> .\%CSE2000-folder%\cse2000sdk_tmp.dox

"C:\Program Files\doxygen\bin\doxygen.exe" ".\70-0062 CSE2000_SDK\cse2000sdk_tmp.dox"
del .\%CSE2000-folder%\cse2000sdk_tmp.dox

::echo "PROJECT_NUMBER = 3.0.2016.09301" >> .\%CSN210-folder%\csn210sdk.dox
copy .\%CSN210-folder%\csn210sdk.dox .\%CSN210-folder%\csn210sdk_tmp.dox

echo PROJECT_NAME = "CSN210 SDK" >> .\%CSN210-folder%\csn210sdk_tmp.dox
echo OUTPUT_DIRECTORY = ".\70-0066 CSN210_SDK\Documents" >> .\%CSN210-folder%\csn210sdk_tmp.dox
echo INPUT = ".\70-0066 CSN210_SDK" >> .\%CSN210-folder%\csn210sdk_tmp.dox
echo EXAMPLE_PATH = ".\70-0066 CSN210_SDK\cpp\CSN210PositionTest\CSN210PositionTest" >> .\%CSN210-folder%\csn210sdk_tmp.dox
echo PROJECT_NUMBER = %version:_=.% >> .\%CSN210-folder%\csn210sdk_tmp.dox

"C:\Program Files\doxygen\bin\doxygen.exe" ".\70-0066 CSN210_SDK\csn210sdk_tmp.dox"
del .\%CSN210-folder%\csn210sdk_tmp.dox


::Copy All to Shipping:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

xcopy  .\%LSM-folder% .\Shipping\"70-0030 LSM_SDK_%1" /e /i /h /y
xcopy  .\%LSMGalvo-folder% .\Shipping\"70-0031 LSMGalvo_SDK_%1" /e /i /h /y
xcopy  .\%ECU-folder% .\Shipping\"70-0029 ECU_SDK_%1" /e /i /h /y
xcopy  .\%MPH16-folder% .\Shipping\"70-0034 MPH16_SDK_%1" /e /i /h /y
xcopy  .\%PMT-folder% .\Shipping\"70-0035 PMT_SDK_%1" /e /i /h /y
xcopy  .\%MFC1-folder% .\Shipping\"70-0051 MFC1_SDK_%1" /e /i /h /y
xcopy  .\%PowerControl-folder% .\Shipping\"70-0036 PowerControl_SDK_%1" /e /i /h /y
xcopy  .\%BScope-folder% .\Shipping\"70-0028 BScope_SDK_%1" /e /i /h /y
xcopy  .\%BCM-folder% .\Shipping\"70-0027 BCM_SDK_%1" /e /i /h /y
xcopy  .\%MCM3000-folder% .\Shipping\"70-0033 MCM3000_SDK_%1" /e /i /h /y
xcopy  .\%MCM6000-folder% .\Shipping\"70-0101 MCM6000_SDK_%1" /e /i /h /y
xcopy  .\%MCLS-folder% .\Shipping\"70-0032 MCLS_SDK_%1" /e /i /h /y
xcopy  .\%PMT2100-folder% .\Shipping\"70-0037 PMT2100_SDK_%1" /e /i /h /y
xcopy  .\%BCMPA-folder% .\Shipping\"70-0077 BCM-PA_SDK_%1" /e /i /h /y
xcopy  .\%LSKGR-folder% .\Shipping\"70-0058 LSKGR_SDK_%1" /e /i /h /y
xcopy  .\%CSE2000-folder% .\Shipping\"70-0062 CSE2000_SDK_%1" /e /i /h /y
xcopy  .\%CSN210-folder% .\Shipping\"70-0066 CSN210_SDK_%1" /e /i /h /y


::Delete Unnecessary files:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

attrib /S -H
del /S .\Shipping\*.ilk
del /S .\Shipping\*.pdb
REM del /S .\Shipping\*.suo
REM del /S .\Shipping\*.vssscc
REM del /S .\Shipping\*.ncb
del /S .\Shipping\*.vspscc
REM del /S .\Shipping\*.aps
REM del /S .\Shipping\BuildLog.htm
REM del /S .\Shipping\*.dep
REM del /S .\Shipping\*.manifest
REM del /S .\Shipping\*.manifest.res
del /S .\Shipping\*.obj
REM del /S .\Shipping\*.idb
del /s .\Shipping\*.dox
del /S .\Shipping\Release
REM del /S .\Shipping\Debug