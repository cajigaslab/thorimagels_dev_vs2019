mkdir "%1\Tools\BuildAllSolution2012\Release\"
mkdir "%1\Tools\BuildAllSolution2012\Release\en-US"
mkdir "%1\Tools\BuildAllSolution2012\Release\x64"

REM Copy all .dll files found in this branch
for /R %1\GUI\Applications\ThorImageLS\ThorImage\bin\Release %%f in (*.dll) do copy %%f "%1\Tools\BuildAllSolution2012\Release\"

copy %1\GUI\Applications\ThorImageLS\ThorImage\bin\Release\SQLite.Interop.dll "%1\Tools\BuildAllSolution2012\Release\x64\"

for /R %1\GUI\Applications\ThorImageLS\ThorImage\bin\Release\en-US %%f in (*.resources.dll) do copy %%f "%1\Tools\BuildAllSolution2012\Release\en-US\"

copy  %1\Common\MatlabEngine\MatlabEngine\bin\Release\MatlabEngine.dll "%1\Tools\BuildAllSolution2012\Release\"
copy  %1\GUI\Controls\ScriptPanels\ScriptMatlab\ScriptMatlab\bin\Release\ScriptMatlab.dll "%1\Tools\BuildAllSolution2012\Release\"
copy  %1\GUI\Controls\ScriptManager\ScriptRunMatlabModule\bin\Release\ScriptRunMatlabModule.dll "%1\Tools\BuildAllSolution2012\Release\"

REM Copy all .exe files 
for /R %1 %%f in (\Release\*.exe) do copy %%f "%1\Tools\BuildAllSolution2012\Release\"
for /R %1 %%f in (\Release\*.exe.config) do copy %%f "%1\Tools\BuildAllSolution2012\Release\"

copy %1\Tools\XMLTools\XMLUpdate\x64\Release\XMLUpdate.exe "%1\Tools\BuildAllSolution2012\x64\Release\"

copy  %1\GUI\Controls\HardwareSetupUserControl\HardwareSetupUserControl\bin\Release\HardwareSetupUserControl.dll "%1\Tools\BuildAllSolution2012\Release\"
copy  %1\Common\HelpProvider\HelpProvider\bin\Release\HelpProvider.dll "%1\Tools\BuildAllSolution2012\Release\"
copy  %1\GUI\Controls\ThemeControl\ThemeControl\bin\Release\ThemeControl.dll "%1\Tools\BuildAllSolution2012\Release\"
copy  %1\Common\ThorLogging\ThorLogging\bin\Release\ThorLogging.dll "%1\Tools\BuildAllSolution2012\Release\"
copy  %1\Common\ThorSharedTypes\ThorSharedTypes\bin\Release\ThorSharedTypes.dll "%1\Tools\BuildAllSolution2012\Release\"

copy  %1\Hardware\SDKs\CameraFunctions\CameraFunctions\bin\Release\CameraFunctions.dll "%1\Tools\BuildAllSolution2012\Release\"
copy  %1\Hardware\SDKs\DeviceFunctions\DeviceFunctions\bin\Release\DeviceFunctions.dll "%1\Tools\BuildAllSolution2012\Release\"