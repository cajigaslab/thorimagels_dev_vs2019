#Read the Python SDK Instructions powerpoint in the documents folder of the SDK before use.

import time

#Ensure that the Python.NET package is installed in your Python environment
import clr

#Set the current directory to the BCMPA python folder
import os
os.chdir(r'C:\Program Files\Thorlabs\BCM-PA 4.3\Python')

#Import and instantiate the class containing the functions of the BCMPA
clr.AddReference('DeviceFunctions')
from DeviceFunctions import BCMPA
from ThorSharedTypes import *
BCMPA = BCMPA()

paramType = 0           
paramAvail = 0
paramReadOnly = 0
pmin = 0
pmax = 0
paramDefault = 0
deviceCount = 0
device = 0

                  #List of function returns when BCMPA is connected and the enclosed parameters are used:

if (BCMPA.FindDevices(deviceCount)[0] == 0):  
    print("FindDevices FAILED")                                         
if (BCMPA.SelectDevice(device) == 0):
    print("SelectDevice FAILED")
if (BCMPA.GetParamInfo(IDevice.Params.PARAM_DEVICE_TYPE, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[0] == 0):
    print("GetParamInfo (DeviceType) FAILED ")
    
if (BCMPA.GetParamInfo(IDevice.Params.PARAM_DEVICE_TYPE, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[6] == DeviceType.POWER_REG):
    
    #Get Serial Number
    paramString = 0
    if (BCMPA.PreflightPosition() == 0):
        print("PreflightPosition FAILED")
    if (BCMPA.GetParam(IDevice.Params.PARAM_POWER_SERIALNUMBER, paramString)[0] == 0):
        print("GetParam (Serial Number) FAILED ")
    else:
        paramSerialString = BCMPA.GetParam(IDevice.Params.PARAM_POWER_SERIALNUMBER, paramString)[1]
        print("Power Serial Number:", int(paramSerialString))
        
    #Moving Power Position to 50 and then 100
    if (BCMPA.GetParamInfo(IDevice.Params.PARAM_POWER_POS, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[0] == 0):
        print("GetParamInfo (Power Position) FAILED ")
    if (BCMPA.GetParamInfo(IDevice.Params.PARAM_POWER_POS, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[2] == 1):
        powerPos = 50
        if (BCMPA.SetParam(IDevice.Params.PARAM_POWER_POS, powerPos) == 0):
            print("SetParam (Power 50 Position) FAILED ")
        if (BCMPA.PreflightPosition() == 0):
            print("PreflightPosition FAILED ")
        if (BCMPA.SetupPosition() == 0):
            print("SetupPosition FAILED ")
        if (BCMPA.StartPosition() == 0):
            print("StartPosition FAILED")
        time.sleep(1)
        print("Power Position 50 = ", BCMPA.GetParam(IDevice.Params.PARAM_POWER_POS_CURRENT, powerPos)[1])
        time.sleep(1)
        powerPos = 100
        if (BCMPA.SetParam(IDevice.Params.PARAM_POWER_POS, powerPos) == 0):
            print("SetParam (Power 100 Position) FAILED ")
        if (BCMPA.SetupPosition() == 0):
            print("SetupPosition FAILED ")
        if (BCMPA.StartPosition() == 0):
            print("StartPosition FAILED")
        time.sleep(1)
        print("Power Position 100 = ", BCMPA.GetParam(IDevice.Params.PARAM_POWER_POS_CURRENT, powerPos)[1])
        if (BCMPA.PostflightPosition() == 0):
            print("PostflightPosition FAILED")
        
    #Moving Power Position to Zero Position and then 100   
    if (BCMPA.GetParamInfo(IDevice.Params.PARAM_POWER_ZERO_POS, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[0] == 0):
        print("GetParamInfo (Power Position) FAILED ")    
    if (BCMPA.GetParamInfo(IDevice.Params.PARAM_POWER_ZERO_POS, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[2] == 1):
        powerPos = 0
        if (BCMPA.SetParam(IDevice.Params.PARAM_POWER_POS, powerPos) == 0):
            print("SetParam (Power 0 Position) FAILED ")
        if (BCMPA.PreflightPosition() == 0):
            print("PreflightPosition FAILED")
        if (BCMPA.SetupPosition() == 0):
            print("SetupPosition FAILED ")
        if (BCMPA.StartPosition() == 0):
            print("StartPosition FAILED")
        time.sleep(1)
        print("Power Position 0 = ", BCMPA.GetParam(IDevice.Params.PARAM_POWER_POS_CURRENT, powerPos)[1])
        if (BCMPA.PostflightPosition() == 0):
            print("PostflightPosition FAILED")
        time.sleep(1)
        powerPos = 100
        if (BCMPA.SetParam(IDevice.Params.PARAM_POWER_POS, powerPos) == 0):
            print("SetParam (Power 100 Position) FAILED ")
        if (BCMPA.PreflightPosition() == 0):
            print("PreflightPosition FAILED")
        if (BCMPA.SetupPosition() == 0):
            print("SetupPosition FAILED ")
        if (BCMPA.StartPosition() == 0):
            print("StartPosition FAILED")
        time.sleep(1)
        print("Power Position 100 = ", BCMPA.GetParam(IDevice.Params.PARAM_POWER_POS_CURRENT, powerPos)[1])
        if (BCMPA.PostflightPosition() == 0):
            print("PostflightPosition FAILED")
    
    #Moving Power Position to 50 and then 0
    if (BCMPA.GetParamInfo(IDevice.Params.PARAM_POWER_POS, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[0] == 0):
        print("GetParamInfo (Power Position Test) FAILED ")   
    if (BCMPA.GetParamInfo(IDevice.Params.PARAM_POWER_POS, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[2] == 1):
        powerPos = 50
        if (BCMPA.SetParam(IDevice.Params.PARAM_POWER_POS, powerPos) == 0):
            print("SetParam (Power 50 Position) FAILED ")
        if (BCMPA.PreflightPosition() == 0):
            print("PreflightPosition FAILED")
        if (BCMPA.SetupPosition() == 0):
            print("SetupPosition FAILED ")
        if (BCMPA.StartPosition() == 0):
            print("StartPosition FAILED")
        print("Power Position 50 = ", BCMPA.GetParam(IDevice.Params.PARAM_POWER_POS_CURRENT, powerPos)[1])
        time.sleep(1)
        if (BCMPA.SetParam(IDevice.Params.PARAM_POWER_POS, 0) == 0):
            print("SetParam (Power 0 Position) FAILED ")
        if (BCMPA.SetupPosition() == 0):
            print("SetupPosition FAILED ")
        if (BCMPA.StartPosition() == 0):
            print("StartPosition FAILED")
        print("Power Position 0 = ", BCMPA.GetParam(IDevice.Params.PARAM_POWER_POS_CURRENT, powerPos)[1])
        if (BCMPA.PostflightPosition() == 0):
            print("PostflightPosition FAILED")
    
if (BCMPA.TeardownDevice() == 0):
    print("TeardownDevice FAILED")