#Read the Python SDK Instructions powerpoint in the documents folder of the SDK before use.

import time

#Ensure that the Python.NET package is installed in your Python environment
import clr

#Set the current directory to the MFC1 python folder
import os
os.chdir(r'C:\Program Files\Thorlabs\MFC1 4.3\Python')

#Import and instantiate the class containing the functions of the MFC1
clr.AddReference('DeviceFunctions')
from DeviceFunctions import ZStepper
from ThorSharedTypes import *
ZStepper = ZStepper()

paramType = 0           
paramAvail = 0
paramReadOnly = 0
pmin = 0
pmax = 0
paramDefault = 0
deviceCount = 0
device = 0

                  #List of function returns when Z-Stepper is connected and the enclosed parameters are used:

if (ZStepper.FindDevices(deviceCount)[0] == 0):  
    print("FindDevices FAILED")
if (ZStepper.SelectDevice(device) == 0):
    print("SelectDevice FAILED")
if (ZStepper.GetParamInfo(IDevice.Params.PARAM_Z_POS, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[0] == 0):
    print("GetParam Info (Z-Value) FAILED")
    
#Sets the Current Z Position to be 0
setZero = 1
if (ZStepper.SetParam(IDevice.Params.PARAM_Z_ZERO, setZero) == 0): 
    print("SetParam (Zero Value) FAILED")
if (ZStepper.PreflightPosition() == 0):
    print("PreflightPosition FAILED")
    
#Moves the ZStepper in increments of .002 m up to .01 m (Make sure you have the right motor settings in the .xml)
for i in range (0,6):
    target = ((2- -2) * (i))/2000.0
    if (ZStepper.SetParam(IDevice.Params.PARAM_Z_POS, target) == 0):
        print("SetParam (Z-Value) FAILED")
    if (ZStepper.SetupPosition() == 0):
        print("SetupPosition FAILED")
    if (ZStepper.StartPosition() == 0):
        print("StartPosition FAILED")
    print("Moving to Position ", target)
    status = ICamera.StatusType.STATUS_READY 
    while (ZStepper.StatusPosition(status)[1] == ICamera.StatusType.STATUS_BUSY):
        if (ZStepper.StatusPosition(status)[0] == 0):
            print("StatusPosition FAILED") 
    time.sleep(.5)
    currentZPos = 0.0
    if (ZStepper.GetParam(IDevice.Params.PARAM_Z_POS_CURRENT, currentZPos)[0] == 0):
        print("GetParam (Current Z-Value) FAILED")
    print("Current Z-Value: ", ZStepper.GetParam(IDevice.Params.PARAM_Z_POS_CURRENT, currentZPos)[1])
    
if (ZStepper.PostflightPosition() == 0):
    print("PostflightPosition FAILED")                                  
if (ZStepper.TeardownDevice() == 0):
    print("TeardownDevice FAILED")                                          