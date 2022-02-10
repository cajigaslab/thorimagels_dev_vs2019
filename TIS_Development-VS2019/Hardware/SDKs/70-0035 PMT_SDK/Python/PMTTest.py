#Read the Python SDK Instructions powerpoint in the documents folder of the SDK before use.

import time

#Ensure that the Python.NET package is installed in your Python environment
import clr

#Set the current directory to the PMT python folder
import os
os.chdir(r'C:\Program Files\Thorlabs\PMT 4.3\Python')

#Import and instantiate the class containing the functions of the PMT
clr.AddReference('DeviceFunctions')
from DeviceFunctions import PMT
from ThorSharedTypes import *
PMT = PMT()

paramType = 0           
paramAvail = 0
paramReadOnly = 0
pmin = 0
pmax = 0
paramDefault = 0
deviceCount = 0
device = 0

                  #List of function returns when PMT is connected and the enclosed parameters are used:

if (PMT.FindDevices(deviceCount)[0] == 0): 
    print("FindDevices FAILED")
if (PMT.SelectDevice(device) == 0):
    print("SelectDevice FAILED")
enableScanner = 1
if (PMT.SetParam(IDevice.Params.PARAM_SCANNER_ENABLE, enableScanner) == 0):
    print("SetParam (Enable Scanner) FAILED ")
gainPos = 10
if (PMT.SetParam(IDevice.Params.PARAM_PMT1_GAIN_POS, gainPos) == 0):
    print("SetParam (PMT 1 Gain) FAILED ")
if (PMT.PreflightPosition() == 0):
    print("PreflightPosition FAILED")
if (PMT.SetupPosition() == 0):
    print("SetupPosition FAILED")
if (PMT.StartPosition() == 0):
    print("StartPosition FAILED")
status = ICamera.StatusType.STATUS_READY
while (PMT.StatusPosition(status)[1] == ICamera.StatusType.STATUS_BUSY):
    if (PMT.StatusPosition(status)[0] == 0):
        print("StatusPosition FAILED")
if (PMT.GetParam(IDevice.Params.PARAM_SCANNER_ENABLE, enableScanner)[0] == 1):
    if (PMT.GetParam(IDevice.Params.PARAM_SCANNER_ENABLE, enableScanner)[1] == 0):
        scannerCheck = "Scanner Disabled"
    elif (PMT.GetParam(IDevice.Params.PARAM_SCANNER_ENABLE, enableScanner)[1] == 1):
        scannerCheck = "Scanner Enabled"
    print("Scanner Enable Check:", scannerCheck)
if (PMT.GetParam(IDevice.Params.PARAM_PMT1_GAIN_POS, gainPos) == 0):
    print("GetParam (PMT1 Gain) FAILED ")
print("PMT Gain Set to", gainPos)
time.sleep(2)
disableScanner = 0
if (PMT.SetParam(IDevice.Params.PARAM_SCANNER_ENABLE, disableScanner) == 0):
    print("SetParam (Disable Scanner) FAILED ")
if (PMT.SetupPosition() == 0):
    print("SetupPosition FAILED")
if (PMT.StartPosition() == 0):
    print("StartPosition FAILED")
status = ICamera.StatusType.STATUS_READY
while (PMT.StatusPosition(status)[1] == ICamera.StatusType.STATUS_BUSY):
    if (PMT.StatusPosition(status)[0] == 0):
        print("StatusPosition FAILED")
if (PMT.GetParam(IDevice.Params.PARAM_SCANNER_ENABLE, disableScanner)[0] == 1):
    if (PMT.GetParam(IDevice.Params.PARAM_SCANNER_ENABLE, disableScanner)[1] == 0):
        scannerCheck = "Scanner Disabled"
    elif (PMT.GetParam(IDevice.Params.PARAM_SCANNER_ENABLE, disableScanner)[1] == 1):
        scannerCheck = "Scanner Enabled"
    print("Scanner Disable Check:", scannerCheck)
if (PMT.PostflightPosition() == 0):
    print("PostflightPosition FAILED")
if (PMT.TeardownDevice() == 0):
    print("TeardownDevice FAILED")