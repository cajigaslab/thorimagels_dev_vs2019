#Read the Python SDK Instructions powerpoint in the documents folder of the SDK before use.

import time

#Ensure that the Python.NET package is installed in your Python environment
import clr

#Set the current directory to the PMT2100 python folder
import os
os.chdir(r'C:\Program Files\Thorlabs\PMT2100 4.3\Python')

#Import and instantiate the class containing the functions of the PMT2100
clr.AddReference('DeviceFunctions')
from DeviceFunctions import PMT2100
from ThorSharedTypes import *
PMT2100 = PMT2100()

paramType = 0           
paramAvail = 0
paramReadOnly = 0
pmin = 0
pmax = 0
paramDefault = 0
deviceCount = 0
device = 0

                  #List of function returns when PMT2100 is connected and the enclosed parameters are used:

if (PMT2100.FindDevices(deviceCount)[0] == 0):  
    print("FindDevices FAILED")
    
if (PMT2100.SelectDevice(device) == 0):
    print("SelectDevice FAILED")

#Print PMT1 Serial Number
maxStringLength = 255
paramString = " " * maxStringLength
if (PMT2100.GetParamString(IDevice.Params.PARAM_PMT1_SERIALNUMBER, paramString, maxStringLength)[0] == 0):
    print("GetParamString (Serial Number) FAILED ")
paramSerialNumber = PMT2100.GetParamString(IDevice.Params.PARAM_PMT1_SERIALNUMBER, paramString, maxStringLength)[1]
print("PMT 1 Serial Number:", paramSerialNumber.split(' ')[0])

#Enabling PMT1, Setting Gain and Offset
enablePMT = 1.0
if (PMT2100.SetParam(IDevice.Params.PARAM_PMT1_ENABLE, enablePMT) == 0):
    print("SetParam (Enable PMT1) FAILED")
gainPos = 10
if (PMT2100.SetParam(IDevice.Params.PARAM_PMT1_GAIN_POS, gainPos) == 0):
    print("SetParam (PMT1 Gain Pos) FAILED")
gainOffset = .75
if (PMT2100.SetParam(IDevice.Params.PARAM_PMT1_OUTPUT_OFFSET, gainOffset) == 0):
    print("SetParam (PMT1 Output Offset) FAILED")
if (PMT2100.PreflightPosition() == 0):
    print("PreflightPosition FAILED")                                                    
if (PMT2100.SetupPosition() == 0):
    print("SetupPosition FAILED")
if (PMT2100.StartPosition() == 0):
    print("StartPosition FAILED")
status = ICamera.StatusType.STATUS_BUSY
while (PMT2100.StatusPosition(status)[1] == ICamera.StatusType.STATUS_BUSY):
    if (PMT2100.StatusPosition(status)[0] == 0):
        print("StatusPosition FAILED")                   
if (PMT2100.PostflightPosition() == 0):
    print("PostflightPosition FAILED")
print("PMT1 Enabled")
time.sleep(2)

#Getting Current Gain and Output Offset
if (PMT2100.GetParam(IDevice.Params.PARAM_PMT1_GAIN_POS_CURRENT_VOLTS, gainPos)[0] == 0):
    print("GetParam: (PMT1 Gain Pos Current) FAILED")
print("PMT1 Current Gain: ", PMT2100.GetParam(IDevice.Params.PARAM_PMT1_GAIN_POS_CURRENT_VOLTS, gainPos)[1])
if (PMT2100.GetParam(IDevice.Params.PARAM_PMT1_OUTPUT_OFFSET_CURRENT, gainOffset)[0] == 0):
    print("GetParam: (PMT1 Output Offset Current) FAILED")
print("PMT1 Current Output Offset: ", PMT2100.GetParam(IDevice.Params.PARAM_PMT1_OUTPUT_OFFSET_CURRENT, gainOffset)[1])

#Disabling PMT1
disablePMT = 0.0
if (PMT2100.SetParam(IDevice.Params.PARAM_PMT1_ENABLE, disablePMT) == 0):
    print("SetParam (Disable PMT1) FAILED")
if (PMT2100.PreflightPosition() == 0):
    print("PreflightPosition FAILED")
if (PMT2100.SetupPosition() == 0):
    print("SetupPosition FAILED")
if (PMT2100.StartPosition() == 0):
    print("StartPosition FAILED")
status = ICamera.StatusType.STATUS_BUSY
while (PMT2100.StatusPosition(status)[1] == ICamera.StatusType.STATUS_BUSY):
    if (PMT2100.StatusPosition(status)[1] == 0):
        print("StatusPosition FAILED")
if (PMT2100.PostflightPosition() == 0):
    print("PostflightPosition FAILED") 
print("PMT1 Disabled")
if (PMT2100.TeardownDevice() == 0):
    print("TeardownDevice FAILED")  