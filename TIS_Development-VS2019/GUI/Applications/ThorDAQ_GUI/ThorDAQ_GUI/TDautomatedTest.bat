ECHO ThorDAQ Automated Regression Test, DZimmerman
ECHO V1.0 23-Aug-19
ECHO invoke the program - if \temp\TSscript.txt exists it will be run

"C:\Users\User\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\thordaqGUI\thordaqGUI.appref-ms"

REM poll every /t seconds for the program to exit...
:pollAgain
timeout /t 2
tasklist /fi "imagename eq thordaqGUI.exe" | find ":" > nul
if ERRORLEVEL 1 goto pollAgain

REM program exited -- do we see error condition in log file?
cd \Users\User\AppData\Local\Apps\2.0
findstr /s /i /c:"thorDAQreadchannel" thorlog.log
if %errorlevel% EQU 0 goto done

REM reboot in XX seconds, expected shutdown
shutdown /r /t 2 /d p:0:0


REM end test (stop rebooting)
:done
