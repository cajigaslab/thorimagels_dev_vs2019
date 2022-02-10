#Read the Python SDK Instructions powerpoint in the documents folder of the SDK before use.

import time

#Ensure that the Python.NET package is installed in your Python environment
import clr

#Set the current directory to the CSE2000 python folder
import os
os.chdir(r'C:\Program Files\Thorlabs\CSE2000 4.3\Python')

#Import and instantiate the class containing the functions of the CSE2000
clr.AddReference('DeviceFunctions')
from DeviceFunctions import EpiTurret
from ThorSharedTypes import *
EpiTurret = EpiTurret()

paramType = 0           
paramAvail = 0
paramReadOnly = 0
pmin = 0
pmax = 0
paramDefault = 0
deviceCount = 0
device = 0

                  #List of function returns when Epi Turret is connected and the enclosed parameters are used:
if (EpiTurret.FindDevices(deviceCount)[0] == 0): 
    print("FindDevices FAILED")
if (EpiTurret.SelectDevice(device) == 0):
    print("SelectDevice FAILED")
if (EpiTurret.GetParamInfo(IDevice.Params.PARAM_DEVICE_TYPE, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[0] == 0):
    print("GetParamInfo (DeviceType) FAILED ")
if (EpiTurret.GetParamInfo(IDevice.Params.PARAM_DEVICE_TYPE, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[6] == DeviceType.FILTER_WHEEL_DIC):
    
    #Get Firmware Version
    if (EpiTurret.GetParamInfo(IDevice.Params.PARAM_FW_DIC_FIRMWAREVERSION, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[0] == 0):
        print("GetParamInfo (Firmware Version) FAILED ")
    if (EpiTurret.GetParamInfo(IDevice.Params.PARAM_FW_DIC_FIRMWAREVERSION, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[2] == 1):
        if (EpiTurret.PreflightPosition() == 0):
            print("PreflightPosition FAILED")
        maxStringLength = 255
        paramString = " " * maxStringLength
        if (EpiTurret.GetParamString(IDevice.Params.PARAM_FW_DIC_FIRMWAREVERSION, paramString, maxStringLength)[0] == 0):
            print("GetParamString (Firmware Version) FAILED ")
        paramFirmwareString = EpiTurret.GetParamString(IDevice.Params.PARAM_FW_DIC_FIRMWAREVERSION, paramString, maxStringLength)[1]
        print("Current Firmware Version: ", paramFirmwareString.split(' ')[0])
    
    #Get Serial Number
    if (EpiTurret.GetParamInfo(IDevice.Params.PARAM_FW_DIC_SERIALNUMBER, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[0] == 0):
        print("GetParamInfo (Serial Number) FAILED ")
    if (EpiTurret.GetParamInfo(IDevice.Params.PARAM_FW_DIC_SERIALNUMBER, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[2] == 1):
        if (EpiTurret.PreflightPosition() == 0):
            print("PreflightPosition FAILED")
        if (EpiTurret.GetParamString(IDevice.Params.PARAM_FW_DIC_SERIALNUMBER, paramString, maxStringLength)[0] == 0):
            print("GetParamString (Serial Number) FAILED ")
        paramSerialString = EpiTurret.GetParamString(IDevice.Params.PARAM_FW_DIC_SERIALNUMBER, paramString, maxStringLength)[1]
        print("Device Serial Number: ", paramSerialString.split(' ')[0])
    
    #Get Current Filter Wheel Position
    if (EpiTurret.GetParamInfo(IDevice.Params.PARAM_FW_DIC_POS, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[0] == 0):
        print("GetParamInfo (Filter Wheel Position) FAILED ")
    if (EpiTurret.GetParamInfo(IDevice.Params.PARAM_FW_DIC_POS, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[2] == 1):
        if (EpiTurret.PreflightPosition() == 0):
            print("PreflightPosition FAILED")
        paramPos = 0
        print("Current Filter Wheel Position: ", EpiTurret.GetParam(IDevice.Params.PARAM_FW_DIC_POS, paramPos)[1])
    if (EpiTurret.PostflightPosition() == 0):
        print("PostflightPosition FAILED")
if (EpiTurret.TeardownDevice() == 0):
    print("TeardownDevice FAILED")    