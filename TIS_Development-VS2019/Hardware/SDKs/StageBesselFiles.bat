:: %1 is the version number separated by underscores e.g. 4_2_2019_11191
:: This is the second part of the setup for the Bessel Beam Installer, this batch file should always be called after UpdateBesselBinaries.bat
set BesselBeam-folder="70-0123 BesselBeam_App"

::Copy All to Shipping:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

xcopy  .\%BesselBeam-folder% .\Shipping\"70-0123 BesselBeam_App_%1" /e /i /h /y

::Delete Unnecessary files:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

attrib /S -H
del /S .\Shipping\*.ilk
del /S .\Shipping\*.pdb
del /S .\Shipping\*.vspscc
del /S .\Shipping\*.obj
del /s .\Shipping\*.dox
del /S .\Shipping\Release