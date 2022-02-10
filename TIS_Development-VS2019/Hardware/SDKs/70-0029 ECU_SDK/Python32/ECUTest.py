#Read the Python SDK Instructions powerpoint in the documents folder of the SDK before use.

import time

#Ensure that the Python.NET package is installed in your Python environment
import clr

#Set the current directory to the ECU python folder
import os
os.chdir(r'C:\Program Files\Thorlabs\ECU 4.3\Python')

#Import and instantiate the class containing the functions of the ECU
clr.AddReference('DeviceFunctions')
from DeviceFunctions import ECU
from ThorSharedTypes import *
ECU = ECU()

paramType = 0           
paramAvail = 0
paramReadOnly = 0
pmin = 0
pmax = 0
paramDefault = 0
deviceCount = 0
device = 0

                  #List of function returns when ECU is connected and the enclosed parameters are used:

if (ECU.FindDevices(deviceCount)[0] == 0):  
    print("FindDevices FAILED")
if (ECU.SelectDevice(device) == 0):
    print("SelectDevice FAILED")

#Enable Resonance Scanner
enableScanner = 1.0
if (ECU.SetParam(IDevice.Params.PARAM_SCANNER_ENABLE, enableScanner) == 0):
    print("SetParam (Enable Scanner) FAILED ")

#Set Scanner Zoom Position
zoomPos = 100.0
if (ECU.SetParam(IDevice.Params.PARAM_SCANNER_ZOOM_POS, zoomPos) == 0):
    print("SetParam (Scanner Zoom Position) FAILED ")

#Set PMT1 Gain
gainPos = 10
if (ECU.SetParam(IDevice.Params.PARAM_PMT1_GAIN_POS, gainPos) == 0):
    print("SetParam (PMT1 Gain) FAILED ")
if (ECU.PreflightPosition() == 0):
    print("PreflightPosition FAILED")
if (ECU.SetupPosition() == 0):
    print("SetupPosition FAILED")
if (ECU.StartPosition() == 0):
    print("StartPosition FAILED")
status = ICamera.StatusType.STATUS_READY 
while (ECU.StatusPosition(status)[1] == ICamera.StatusType.STATUS_BUSY):
    if (ECU.StatusPosition(status)[0] == 0):
        print("StatusPosition FAILED")
time.sleep(2)

#Displaying Set Parameters
if (ECU.GetParam(IDevice.Params.PARAM_SCANNER_ENABLE, enableScanner)[0] == 0):
    print("GetParam (Enable Scanner) FAILED")
if (ECU.GetParam(IDevice.Params.PARAM_SCANNER_ENABLE, enableScanner)[0] == 1):
    if (ECU.GetParam(IDevice.Params.PARAM_SCANNER_ENABLE, enableScanner)[1] == 0):
        scannerCheck = "Scanner Disabled"
    elif (ECU.GetParam(IDevice.Params.PARAM_SCANNER_ENABLE, enableScanner)[1] == 1):
        scannerCheck = "Scanner Enabled"
    print("Scanner Enable Check:", scannerCheck)
print("Zoom Position set to", zoomPos)
print("PMT Gain set to", gainPos)
time.sleep(2)

#Disable Resonance Scanner
disableScanner = 0.0
if (ECU.SetParam(IDevice.Params.PARAM_SCANNER_ENABLE, disableScanner) == 0):
    print("SetParam (Disable Scanner) FAILED ")
if (ECU.SetupPosition() == 0):
    print("SetupPosition FAILED")
if (ECU.StartPosition() == 0):
    print("StartPosition FAILED")
status = ICamera.StatusType.STATUS_READY
while (ECU.StatusPosition(status)[1] == ICamera.StatusType.STATUS_BUSY):
    if (ECU.StatusPosition(status)[0] == 0):
        print("StatusPosition FAILED")
if (ECU.GetParam(IDevice.Params.PARAM_SCANNER_ENABLE, disableScanner)[0] == 0):
    print("GetParam (Disable Scanner) FAILED")
if (ECU.GetParam(IDevice.Params.PARAM_SCANNER_ENABLE, disableScanner)[0] == 1):
    if (ECU.GetParam(IDevice.Params.PARAM_SCANNER_ENABLE, disableScanner)[1] == 0):
        scannerCheck = "Scanner Disabled"
    elif (ECU.GetParam(IDevice.Params.PARAM_SCANNER_ENABLE, disableScanner)[1] == 1):
        scannerCheck = "Scanner Enabled"
    print("Scanner Disable Check:", scannerCheck)
if (ECU.PostflightPosition() == 0):
    print("PostflightPosition FAILED")
if (ECU.TeardownDevice() == 0):
    print("TeardownDevice FAILED")