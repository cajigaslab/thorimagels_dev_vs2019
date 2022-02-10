#Read the Python SDK Instructions powerpoint in the documents folder of the SDK before use.

import time

#Ensure that the Python.NET package is installed in your Python environment
import clr

#Set the current directory to the CSN210 python folder
import os
os.chdir(r'C:\Program Files\Thorlabs\CSN210 4.3\Python')

#Import and instantiate the class containing the functions of the CSN210
clr.AddReference('DeviceFunctions')
from DeviceFunctions import ObjectiveChanger
from ThorSharedTypes import *
ObjectiveChanger = ObjectiveChanger()

paramType = 0           
paramAvail = 0
paramReadOnly = 0
pmin = 0
pmax = 0
paramDefault = 0
deviceCount = 0
device = 0
                  #List of function returns when Objective Changer is connected and the enclosed parameters are used:

if (ObjectiveChanger.FindDevices(deviceCount)[0] == 0): 
    print("FindDevices FAILED")
if (ObjectiveChanger.SelectDevice(device) == 0):
    print("SelectDevice FAILED")
if (ObjectiveChanger.GetParamInfo(IDevice.Params.PARAM_DEVICE_TYPE, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[0] == 0):
    print("GetParamInfo (DeviceType) FAILED ")


if (ObjectiveChanger.GetParamInfo(IDevice.Params.PARAM_DEVICE_TYPE, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[6] == DeviceType.TURRET):
    
    #Gets Firmware Version
    if (ObjectiveChanger.PreflightPosition() == 0):
        print("PreflightPosition FAILED")
    maxStringLength = 255
    paramString = " " * maxStringLength
    if (ObjectiveChanger.GetParamString(IDevice.Params.PARAM_TURRET_FIRMWAREVERSION, paramString, maxStringLength)[0] == 0):
        print("GetParamString (Firmware Version) FAILED ")
    paramFirmwareString = ObjectiveChanger.GetParamString(IDevice.Params.PARAM_TURRET_FIRMWAREVERSION, paramString, maxStringLength)[1]
    print("Current Firmware Version:", paramFirmwareString.split(' ')[0])
    
    #Gets Serial Number
    if (ObjectiveChanger.PreflightPosition() == 0):
        print("PreflightPosition FAILED")
    maxStringLength = 255
    paramString = " " * maxStringLength
    if (ObjectiveChanger.GetParamString(IDevice.Params.PARAM_TURRET_SERIALNUMBER, paramString, maxStringLength)[0] == 0):
        print("GetParamString (Serial Number) FAILED ")
    paramSerialString = ObjectiveChanger.GetParamString(IDevice.Params.PARAM_TURRET_SERIALNUMBER, paramString, maxStringLength)[1]
    print("Device Serial Number:", paramSerialString.split(' ')[0])
    
    #Gets Starting Position
    if (ObjectiveChanger.PreflightPosition() == 0):
        print("PreflightPosition FAILED")
    if (ObjectiveChanger.GetParamInfo(IDevice.Params.PARAM_TURRET_POS_CURRENT, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[0] == 0):
        print("GetParamInfo (Turret Current Position) FAILED ")
    if (ObjectiveChanger.GetParamInfo(IDevice.Params.PARAM_TURRET_POS_CURRENT, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[2] == 1):
        if (ObjectiveChanger.PreflightPosition() == 0):
            print("PreflightPosition FAILED")
        paramPos = 0
        if (ObjectiveChanger.GetParam(IDevice.Params.PARAM_TURRET_POS_CURRENT, paramPos) == 0):    
            print("GetParam (Starting Turret Position) FAILED ")
        print('Starting Turret Position:', ObjectiveChanger.GetParam(IDevice.Params.PARAM_TURRET_POS_CURRENT, paramPos)[1])
    time.sleep(5) 
    
    #Move to Home    
    if (ObjectiveChanger.GetParamInfo(IDevice.Params.PARAM_TURRET_POS, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[0] == 0):
        print("GetParamInfo (Turret Position) FAILED ")
    if (ObjectiveChanger.GetParamInfo(IDevice.Params.PARAM_TURRET_POS, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[2] == 1):
        print("Move to Position Home")
        paramPos = 0
        if (ObjectiveChanger.SetParam(IDevice.Params.PARAM_TURRET_POS, paramPos) == 0):
            print("SetParam (Turret Position) FAILED ")
        if (ObjectiveChanger.PreflightPosition() == 0):
            print("PreflightPosition FAILED")
        if (ObjectiveChanger.SetupPosition() == 0):
            print("SetupPosition FAILED")
        if (ObjectiveChanger.StartPosition() == 0):
            print("StartPosition FAILED")
        if (ObjectiveChanger.PostflightPosition() == 0):
            print("PostflightPosition FAILED")
        time.sleep(4)
        if (ObjectiveChanger.GetParam(IDevice.Params.PARAM_TURRET_POS_CURRENT, paramPos)[0] == 0):    
            print("GetParam (Current Turret Position) FAILED ")
        print("Current Turret Position: ", ObjectiveChanger.GetParam(IDevice.Params.PARAM_TURRET_POS_CURRENT, paramPos)[1])
    
    #Move to Position 1
    if (ObjectiveChanger.GetParamInfo(IDevice.Params.PARAM_TURRET_POS, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[2] == 1):
        print("Move to Position 1")
        paramPos = 1
        if (ObjectiveChanger.SetParam(IDevice.Params.PARAM_TURRET_POS, paramPos) == 0):
            print("SetParam (Turret Position) FAILED ")
        if (ObjectiveChanger.PreflightPosition() == 0):
            print("PreflightPosition FAILED")
        if (ObjectiveChanger.SetupPosition() == 0):
            print("SetupPosition FAILED")
        if (ObjectiveChanger.StartPosition() == 0):
            print("StartPosition FAILED")
        if (ObjectiveChanger.PostflightPosition() == 0):
            print("PostflightPosition FAILED")
        time.sleep(4)
        if (ObjectiveChanger.GetParam(IDevice.Params.PARAM_TURRET_POS_CURRENT, paramPos)[0] == 0):    
            print("GetParam (Current Turret Position) FAILED ")
        print("Current Turret Position: ", ObjectiveChanger.GetParam(IDevice.Params.PARAM_TURRET_POS_CURRENT, paramPos)[1])
    
    #Move to Position 2
    if (ObjectiveChanger.GetParamInfo(IDevice.Params.PARAM_TURRET_POS, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[2] == 1):
        print("Move to Position 2")
        paramPos = 2
        if (ObjectiveChanger.SetParam(IDevice.Params.PARAM_TURRET_POS, paramPos) == 0):
            print("SetParam (Turret Position) FAILED ")
        if (ObjectiveChanger.PreflightPosition() == 0):
            print("PreflightPosition FAILED")
        if (ObjectiveChanger.SetupPosition() == 0):
            print("SetupPosition FAILED")
        if (ObjectiveChanger.StartPosition() == 0):
            print("StartPosition FAILED")
        if (ObjectiveChanger.PostflightPosition() == 0):
            print("PostflightPosition FAILED")
        time.sleep(4)
        if (ObjectiveChanger.GetParam(IDevice.Params.PARAM_TURRET_POS_CURRENT, paramPos)[0] == 0):    
            print("GetParam (Current Turret Position) FAILED ")
        print("Current Turret Position: ", ObjectiveChanger.GetParam(IDevice.Params.PARAM_TURRET_POS_CURRENT, paramPos)[1])   
    
if (ObjectiveChanger.TeardownDevice() == 0):
    print("TeardownDevice FAILED")  