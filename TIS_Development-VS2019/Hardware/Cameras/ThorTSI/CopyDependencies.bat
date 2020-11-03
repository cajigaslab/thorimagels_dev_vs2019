echo Configuration:%1 Platform:%2 OutputDir:%3 
set THOR_TSI_PLATFORM=%2
if "%2"=="x64" set THOR_TSI_PLATFORM=Win64
for %%F in (\work\TSI\API\dist\%THOR_TSI_PLATFORM%\%1\*.dll) do call %~dp0\CopySingleDependency.bat %%F %1 %2 %3
