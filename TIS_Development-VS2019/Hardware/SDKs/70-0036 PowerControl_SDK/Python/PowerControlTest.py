#Read the Python SDK Instructions powerpoint in the documents folder of the SDK before use.

import time

#Ensure that the Python.NET package is installed in your Python environment
import clr

#Set the current directory to the PowerControl python folder
import os
os.chdir(r'C:\Program Files\Thorlabs\PowerControl 4.3\Python')

#Import and instantiate the class containing the functions of the PowerControl
clr.AddReference('DeviceFunctions')
from DeviceFunctions import PowerControl
from ThorSharedTypes import *
PowerControl = PowerControl()

paramType = 0         
paramAvail = 0
paramReadOnly = 0
pmin = 0
pmax = 0
paramDefault = 0
deviceCount = 0
device = 0

                  #List of function returns when Power Control is connected and the enclosed parameters are used:
    
if (PowerControl.FindDevices(deviceCount)[0] == 0): 
    print("FindDevices FAILED")
if (PowerControl.SelectDevice(device) == 0):
    print("SelectDevice FAILED")
if (PowerControl.PreflightPosition() == 0):
    print("PreflightPosition FAILED")
powerPos = 0.0
if (PowerControl.SetParam(IDevice.Params.PARAM_POWER_POS, powerPos) == 0):
    print("SetParam (Zero Power) FAILED ")
if (PowerControl.SetupPosition() == 0):
    print("SetupPosition FAILED")
if (PowerControl.StartPosition() == 0):
    print("StartPosition FAILED")
if (PowerControl.PostflightPosition() == 0):
    print("PostflightPosition FAILED")
for i in range(100, -10, -10):
    if (PowerControl.PreflightPosition() == 0):
        print("PreflightPosition FAILED")
    if (PowerControl.SetParam(IDevice.Params.PARAM_POWER_POS, i) == 1):
        print("SetParam (Power Position",i,") FAILED ")
    if (PowerControl.SetupPosition() == 0):
        print("SetupPosition FAILED")
    if (PowerControl.StartPosition() == 0):
        print("StartPosition FAILED")  
    time.sleep(.5)
    readPower = 0.0
    if (PowerControl.GetParam(IDevice.Params.PARAM_POWER_POS_CURRENT, readPower)[0] == 0):
        print("GetParam (Current Power Position) FAILED ")
    print("Current Power Position: ", PowerControl.GetParam(IDevice.Params.PARAM_POWER_POS_CURRENT, readPower)[1])
if (PowerControl.PreflightPosition() == 0):
    print("PreflightPosition FAILED")
homeParam = 0
if (PowerControl.SetParam(IDevice.Params.PARAM_POWER_HOME, homeParam) == 0):
    print("SetParam (Homed) FAILED ")
if (PowerControl.SetupPosition() == 0):
    print("SetupPosition FAILED")
if (PowerControl.StartPosition() == 0):
    print("StartPosition FAILED")
if (PowerControl.PostflightPosition() == 0):
    print("PostflightPosition FAILED")
print("Power Homed")
if (PowerControl.TeardownDevice() == 0):
    print("TeardownDevice FAILED")