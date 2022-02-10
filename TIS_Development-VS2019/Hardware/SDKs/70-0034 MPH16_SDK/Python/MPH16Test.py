#Read the Python SDK Instructions powerpoint in the documents folder of the SDK before use.

import time

#Ensure that the Python.NET package is installed in your Python environment
import clr

#Set the current directory to the MPH16 python folder
import os
os.chdir(r'C:\Program Files\Thorlabs\MPH16 4.3\Python')

#Import and instantiate the class containing the functions of the MPH16
clr.AddReference('DeviceFunctions')
from DeviceFunctions import PinholeStepper
from ThorSharedTypes import *
PinholeStepper = PinholeStepper()

paramType = 0           
paramAvail = 0
paramReadOnly = 0
pmin = 0
pmax = 0
paramDefault = 0
deviceCount = 0
device = 0

                  #List of function returns when Pinhole Stepper is connected and the enclosed parameters are used:

if (PinholeStepper.FindDevices(deviceCount)[0] == 0):  
    print("FindDevices FAILED")                                         
if (PinholeStepper.SelectDevice(device) == 0):
    print("SelectDevice FAILED")

#Setting Starting Position to 0
zeroPos = 0
if (PinholeStepper.SetParam(IDevice.Params.PARAM_PINHOLE_POS, zeroPos) == 0):
    print("SetParam (Pinhole Start Position) FAILED ")
    
#Setting Alignment/Alignment Mode
alignmentPos = 3600
if (PinholeStepper.SetParam(IDevice.Params.PARAM_PINHOLE_ALIGNMENT_POS, alignmentPos) == 0):
    print("SetParam (Pinhole Alignment Position) FAILED ")
alignmentMode = 0
if (PinholeStepper.SetParam(IDevice.Params.PARAM_PINHOLE_ALIGNMENT_MODE, alignmentMode) == 0):
    print("SetParam (Pinhole Alignment Mode) FAILED ")
if (PinholeStepper.PreflightPosition() == 0):
    print("PreflightPosition FAILED")    

#Moves the Pinhole Stepper 1 position down from 15 until 0.
for i in range (0,16):
    if (PinholeStepper.SetParam(IDevice.Params.PARAM_PINHOLE_POS, 15-i) == 0):
        print("SetParam (Pinhole Position) FAILED ")
    if (PinholeStepper.SetupPosition() == 0):
            print("SetupPosition FAILED")
    if (PinholeStepper.StartPosition() == 0):
            print("StartPosition FAILED")  
    status = ICamera.StatusType.STATUS_READY
    while (PinholeStepper.StatusPosition(status)[1] == ICamera.StatusType.STATUS_BUSY):
        if (PinholeStepper.StatusPosition(status)[0] == 0):
            print("StatusPosition FAILED")
    currentPos = 0
    if (PinholeStepper.GetParam(IDevice.Params.PARAM_PINHOLE_POS_CURRENT, currentPos)[0] == 0):
        print("GetParam (Current Pinhole Position) FAILED ")
    print("Current Pinhole Position: ", PinholeStepper.GetParam(IDevice.Params.PARAM_PINHOLE_POS_CURRENT, currentPos)[1]) 
    time.sleep(.5)
    
if (PinholeStepper.PostflightPosition() == 0):
    print("PostflightPosition FAILED")                                  
if (PinholeStepper.TeardownDevice() == 0):
    print("TeardownDevice FAILED")  