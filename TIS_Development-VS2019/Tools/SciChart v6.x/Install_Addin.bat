@echo off

REM Installs the SciChart.UpdaterTool Visual Studio Addin

REM VS2010 Install
IF EXIST "%UserProfile%\Documents\Visual Studio 2010\" (
  echo - Installing SciChart.UpdaterTool Addin for VS2010
  IF NOT EXIST "%UserProfile%\Documents\Visual Studio 2010\Addins" (
     echo Creating Directory for Addin in "%UserProfile%\Documents\Visual Studio 2010\Addins"  
     mkdir "%UserProfile%\Documents\Visual Studio 2010\Addins"  	 
	 if ERRORLEVEL 1 goto :mkdirError
  )
  echo -- Copying .Addin file from "SciChart.UpdaterTool\UpgradeToolAddIn.AddIn" to "%UserProfile%\Documents\Visual Studio 2010\Addins\UpgradeToolAddIn.AddIn"
  echo f | xcopy "SciChart.UpdaterTool\UpgradeToolAddIn.AddIn" "%UserProfile%\Documents\Visual Studio 2010\Addins\UpgradeToolAddIn.AddIn" /S
  if ERRORLEVEL 1 goto :copyAddinError
  
  echo -- Copying Addin dir from "SciChart.UpdaterTool" to "%UserProfile%\Documents\Visual Studio 2010\Addins\SciChart.UpdaterTool"
  echo d | xcopy "SciChart.UpdaterTool" "%UserProfile%\Documents\Visual Studio 2010\Addins\SciChart.UpdaterTool" /S /Q
  if ERRORLEVEL 1 goto :copyAddinFolderError
  
  echo -- Deleting .Addin file "%UserProfile%\Documents\Visual Studio 2010\Addins\SciChart.UpdaterTool\UpgradeToolAddIn.AddIn"
  echo f | del  "%UserProfile%\Documents\Visual Studio 2010\Addins\SciChart.UpdaterTool\UpgradeToolAddIn.AddIn" /Q
  if ERRORLEVEL 1 goto :delAddinError
  
) ELSE ( 
  echo - VS2010 is not installed on this machine
)

REM VS2012 Install
IF EXIST "%UserProfile%\Documents\Visual Studio 2012\" (
  echo - Installing SciChart.UpdaterTool Addin for VS2012
  IF NOT EXIST "%UserProfile%\Documents\Visual Studio 2012\Addins" (
     echo Creating Directory for Addin in "%UserProfile%\Documents\Visual Studio 2012\Addins"  
     mkdir "%UserProfile%\Documents\Visual Studio 2012\Addins"  	 
	 if ERRORLEVEL 1 goto :mkdirError
  )
  echo -- Copying .Addin file from "SciChart.UpdaterTool\UpgradeToolAddIn.AddIn" to "%UserProfile%\Documents\Visual Studio 2012\Addins\UpgradeToolAddIn.AddIn"
  echo f | xcopy "SciChart.UpdaterTool\UpgradeToolAddIn.AddIn" "%UserProfile%\Documents\Visual Studio 2012\Addins\UpgradeToolAddIn.AddIn" /S
  if ERRORLEVEL 1 goto :copyAddinError
  
  echo -- Copying Addin dir from "SciChart.UpdaterTool" to "%UserProfile%\Documents\Visual Studio 2012\Addins\SciChart.UpdaterTool"
  echo d | xcopy "SciChart.UpdaterTool" "%UserProfile%\Documents\Visual Studio 2012\Addins\SciChart.UpdaterTool" /S /Q
  if ERRORLEVEL 1 goto :copyAddinFolderError
  
  echo -- Deleting .Addin file "%UserProfile%\Documents\Visual Studio 2012\Addins\SciChart.UpdaterTool\UpgradeToolAddIn.AddIn"
  echo f | del  "%UserProfile%\Documents\Visual Studio 2012\Addins\SciChart.UpdaterTool\UpgradeToolAddIn.AddIn" /Q
  if ERRORLEVEL 1 goto :delAddinError
  
) ELSE ( 
  echo - VS2012 is not installed on this machine
)

REM VS2013 Install
IF EXIST "%UserProfile%\Documents\Visual Studio 2013\" (
  echo - Installing SciChart.UpdaterTool Addin for VS2013
  IF NOT EXIST "%UserProfile%\Documents\Visual Studio 2013\Addins" (
     echo Creating Directory for Addin in "%UserProfile%\Documents\Visual Studio 2013\Addins"  
     mkdir "%UserProfile%\Documents\Visual Studio 2013\Addins"  	 
	 if ERRORLEVEL 1 goto :mkdirError
  )
  echo -- Copying .Addin file from "SciChart.UpdaterTool\UpgradeToolAddIn.AddIn" to "%UserProfile%\Documents\Visual Studio 2013\Addins\UpgradeToolAddIn.AddIn"
  echo f | xcopy "SciChart.UpdaterTool\UpgradeToolAddIn.AddIn" "%UserProfile%\Documents\Visual Studio 2013\Addins\UpgradeToolAddIn.AddIn" /S
  if ERRORLEVEL 1 goto :copyAddinError
  
  echo -- Copying Addin dir from "SciChart.UpdaterTool" to "%UserProfile%\Documents\Visual Studio 2013\Addins\SciChart.UpdaterTool"
  echo d | xcopy "SciChart.UpdaterTool" "%UserProfile%\Documents\Visual Studio 2013\Addins\SciChart.UpdaterTool" /S /Q
  if ERRORLEVEL 1 goto :copyAddinFolderError
  
  echo -- Deleting .Addin file "%UserProfile%\Documents\Visual Studio 2013\Addins\SciChart.UpdaterTool\UpgradeToolAddIn.AddIn"
  echo f | del  "%UserProfile%\Documents\Visual Studio 2013\Addins\SciChart.UpdaterTool\UpgradeToolAddIn.AddIn" /Q
  if ERRORLEVEL 1 goto :delAddinError
  
) ELSE ( 
  echo - VS2013 is not installed on this machine
)

REM VS2015 Install
IF EXIST "%UserProfile%\Documents\Visual Studio 2015\" (
  echo - Installing SciChart.UpdaterTool Addin for VS2015
  IF NOT EXIST "%UserProfile%\Documents\Visual Studio 2015\Addins" (
     echo Creating Directory for Addin in "%UserProfile%\Documents\Visual Studio 2015\Addins"  
     mkdir "%UserProfile%\Documents\Visual Studio 2015\Addins"  	 
	 if ERRORLEVEL 1 goto :mkdirError
  )
  echo -- Copying .Addin file from "SciChart.UpdaterTool\UpgradeToolAddIn.AddIn" to "%UserProfile%\Documents\Visual Studio 2015\Addins\UpgradeToolAddIn.AddIn"
  echo f | xcopy "SciChart.UpdaterTool\UpgradeToolAddIn.AddIn" "%UserProfile%\Documents\Visual Studio 2015\Addins\UpgradeToolAddIn.AddIn" /S
  if ERRORLEVEL 1 goto :copyAddinError
  
  echo -- Copying Addin dir from "SciChart.UpdaterTool" to "%UserProfile%\Documents\Visual Studio 2015\Addins\SciChart.UpdaterTool"
  echo d | xcopy "SciChart.UpdaterTool" "%UserProfile%\Documents\Visual Studio 2015\Addins\SciChart.UpdaterTool" /S /Q
  if ERRORLEVEL 1 goto :copyAddinFolderError
  
  echo -- Deleting .Addin file "%UserProfile%\Documents\Visual Studio 2015\Addins\SciChart.UpdaterTool\UpgradeToolAddIn.AddIn"
  echo f | del  "%UserProfile%\Documents\Visual Studio 2015\Addins\SciChart.UpdaterTool\UpgradeToolAddIn.AddIn" /Q
  if ERRORLEVEL 1 goto :delAddinError
  
) ELSE ( 
  echo - VS2015 is not installed on this machine
)

echo Success Kid! 
exit 0

:mkdirError
echo The mkdir step to create Addin directory failed
exit %ERRORLEVEL%

:copyAddinError
echo The step to copy .addin to target directory failed 
exit %ERRORLEVEL%

:copyAddinFolderError
echo The step to copy Addin Folder to target directory failed 
exit %ERRORLEVEL%

:delAddinError
echo The step to delete .addin from Addin folder failed 
exit %ERRORLEVEL%