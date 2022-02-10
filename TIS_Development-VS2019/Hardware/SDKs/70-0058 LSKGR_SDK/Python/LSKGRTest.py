#Read the Python SDK Instructions powerpoint in the documents folder of the SDK before use.

import time

#Ensure that the Python.NET package is installed in your Python environment
import clr

#Set the current directory to the LSKGR python folder
import os
os.chdir(r'C:\Program Files\Thorlabs\LSKGR 4.3\Python')

#Import and instantiate the class containing the functions of the LSKGR
clr.AddReference('DeviceFunctions')
from DeviceFunctions import LSKGR
from ThorSharedTypes import *
LSKGR = LSKGR()

                        #List of Function Parameters with Designated Values

paramType = 0           
paramAvail = 0
paramReadOnly = 0
pmin = 0.0
pmax = 0.0
paramDefault = 0.0
deviceCount = 0
device = 0

                  #List of function returns when LSKGR is connected and the enclosed parameters are used:
    
if (LSKGR.FindDevices(deviceCount)[0] == 0): 
    print("FindDevices FAILED")
if (LSKGR.SelectDevice(device) == 0):
    print("SelectDevice FAILED")

if (LSKGR.GetParamInfo(IDevice.Params.PARAM_DEVICE_TYPE, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[0] == 0):
    print("GetParamInfo (DeviceType) FAILED ")
if (LSKGR.GetParamInfo(IDevice.Params.PARAM_DEVICE_TYPE, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[6] == DeviceType.CONTROL_UNIT):
    
    #Get Serial Number
    maxStringLength = 255
    paramString = " " * maxStringLength
    if (LSKGR.PreflightPosition() == 0):
        print("PreflightPosition FAILED")
    if (LSKGR.GetParamString(IDevice.Params.PARAM_CONTROL_UNIT_SERIALNUMBER, paramString, maxStringLength)[0] == 0):
        print("GetParamString (Serial Number) FAILED ")
    paramSerialString = LSKGR.GetParamString(IDevice.Params.PARAM_CONTROL_UNIT_SERIALNUMBER, paramString, maxStringLength)[1]
    print("Device Serial Number:", paramSerialString.split(' ')[0])
    
    #Get Firmware Version
    maxStringLength = 255
    paramString = " " * maxStringLength
    if (LSKGR.PreflightPosition() == 0):
        print("PreflightPosition FAILED")
    if (LSKGR.GetParamString(IDevice.Params.PARAM_CONTROL_UNIT_FIRMWAREVERSION, paramString, maxStringLength)[0] == 0):
        print("GetParamString (Firmware Version) FAILED ")
    paramFirmwareString = LSKGR.GetParamString(IDevice.Params.PARAM_CONTROL_UNIT_FIRMWAREVERSION, paramString, maxStringLength)[1]
    print("Device Firmware Version:", paramFirmwareString.split(' ')[0])
    
    #Get Range for Position Parameter
    if (LSKGR.GetParamInfo(IDevice.Params.PARAM_SCANNER_ENABLE, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[0] == 0):
        print("GetParamInfo (Scanner Enable) FAILED ")
    if (LSKGR.GetParamInfo(IDevice.Params.PARAM_SCANNER_ENABLE, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[2] == 1):
            
        #Enable Resonance Scanner
        enableScanner = 1
        if (LSKGR.SetParam(IDevice.Params.PARAM_SCANNER_ENABLE, enableScanner) == 0):
            print("SetParam (Enable Scanner) FAILED ")
        if (LSKGR.PreflightPosition() == 0):
            print("PreflightPosition FAILED")
        if (LSKGR.StartPosition() == 0):
            print("StartPosition FAILED")
        if (LSKGR.PostflightPosition() == 0):
            print("PostflightPosition FAILED")
        print("Scanner Enabled")
    
    if (LSKGR.GetParamInfo(IDevice.Params.PARAM_SCANNER_ZOOM_POS, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[0] == 0):
        print("GetParamInfo (Scanner Zoom Position) FAILED ")
    if (LSKGR.GetParamInfo(IDevice.Params.PARAM_SCANNER_ZOOM_POS, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[2] == 1):
            
        #Moving Field Size Alignment to 100
        fieldSize = 100
        if (LSKGR.PreflightPosition() == 0):
            print("PreflightPosition FAILED")
        if (LSKGR.SetParam(IDevice.Params.PARAM_SCANNER_ZOOM_POS, fieldSize) == 0):
            print("SetParam (Scanner Zoom Position 100) FAILED ")
        if (LSKGR.SetupPosition() == 0):
            print("SetupPosition FAILED")
        if (LSKGR.StartPosition() == 0):
            print("StartPosition FAILED")
        if (LSKGR.PostflightPosition() == 0):
            print("PostflightPosition FAILED")
        time.sleep(.5)
               
        #Get Current Field Size (Should be 100)
        currentFieldSize = 0
        if (LSKGR.GetParam(IDevice.Params.PARAM_SCANNER_ZOOM_POS, currentFieldSize)[0] == 0):
            print("GetParam (Scanner Zoom Position) FAILED ") 
        print("Scanner Zoom Position Set to", LSKGR.GetParam(IDevice.Params.PARAM_SCANNER_ZOOM_POS, fieldSize)[1])
        
    #Disable Scanner
    if (LSKGR.GetParamInfo(IDevice.Params.PARAM_SCANNER_ENABLE, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[0] == 0):
        print("GetParamInfo (Disable Scanner) FAILED ")
    if (LSKGR.GetParamInfo(IDevice.Params.PARAM_SCANNER_ENABLE, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[2] == 1):
            disableScanner = 0
            if (LSKGR.SetParam(IDevice.Params.PARAM_SCANNER_ENABLE, disableScanner) == 0):
                print("SetParam (Disable Scanner) FAILED ")
            if (LSKGR.PreflightPosition() == 0):
                print("PreflightPosition FAILED")
            if (LSKGR.StartPosition() == 0):
                print("StartPosition FAILED")        
            if (LSKGR.PostflightPosition() == 0):
                print("PostflightPosition FAILED")
            print("Scanner Disabled")
    if (LSKGR.TeardownDevice() == 0):
        print("TeardownDevice FAILED") 