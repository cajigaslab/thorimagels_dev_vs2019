#Read the Python SDK Instructions powerpoint in the documents folder of the SDK before use.

import time

#Ensure that the Python.NET package is installed in your Python environment
import clr

#Set the current directory to the BCM python folder
import os
os.chdir(r'C:\Program Files\Thorlabs\BCM 4.3\Python')

#Import and instantiate the class containing the functions of the BCM
clr.AddReference('DeviceFunctions')
from DeviceFunctions import BCM
from ThorSharedTypes import *
BCM = BCM()

paramType = 0           
paramAvail = 0
paramReadOnly = 0
pmin = 0
pmax = 0
paramDefault = 0
deviceCount = 0
device = 0

                  #List of function returns when BCM is connected and the enclosed parameters are used:

if (BCM.FindDevices(deviceCount)[0] == 0):  
    print("FindDevices FAILED")                                         
if (BCM.SelectDevice(device) == 0):
    print("SelectDevice FAILED")
if (BCM.GetParamInfo(IDevice.Params.PARAM_DEVICE_TYPE, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[0] == 0):
    print("GetParamInfo (DeviceType) FAILED ")
    
if (BCM.GetParamInfo(IDevice.Params.PARAM_DEVICE_TYPE, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[6] == DeviceType.LIGHT_PATH):
    
    #Get Serial Number for all Paths
    paramString = 0
    if (BCM.PreflightPosition() == 0):
        print("PreflightPosition FAILED")
    if (BCM.GetParam(IDevice.Params.PARAM_LIGHTPATH_GG_SERIALNUMBER, paramString)[0] == 0):
        print("GetParam (GG Path Serial Number) FAILED ")
    else:
        paramSerialStringGG = BCM.GetParam(IDevice.Params.PARAM_LIGHTPATH_GG_SERIALNUMBER, paramString)[1]
        print("GG Path Serial Number:", int(paramSerialStringGG))
    
    if (BCM.GetParam(IDevice.Params.PARAM_LIGHTPATH_GR_SERIALNUMBER, paramString)[0] == 0):
        print("GetParam (GR Path Serial Number) FAILED ")
    else:
        paramSerialStringGR = BCM.GetParam(IDevice.Params.PARAM_LIGHTPATH_GR_SERIALNUMBER, paramString)[1]
        print("GR Path Serial Number:", int(paramSerialStringGR))
    
    if (BCM.GetParam(IDevice.Params.PARAM_LIGHTPATH_CAMERA_SERIALNUMBER, paramString)[0] == 0):
        print("GetParam (Camera Path Serial Number) FAILED ")
    else:
        paramSerialStringCamera = BCM.GetParam(IDevice.Params.PARAM_LIGHTPATH_CAMERA_SERIALNUMBER, paramString)[1]
        print("Camera Path Serial Number:", int(paramSerialStringCamera))
    
    #Sets the Galvo-Galvo BCM to Position 1 and then Position 0
    zeroPosition = 0
    onePosition = 1
    if (BCM.GetParam(IDevice.Params.PARAM_LIGHTPATH_GG_SERIALNUMBER, paramString)[0] == 1):
        if (BCM.GetParamInfo(IDevice.Params.PARAM_LIGHTPATH_GG, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[0] == 0):
            print("GetParamInfo (LightPath GG) FAILED ")
        if (BCM.GetParamInfo(IDevice.Params.PARAM_LIGHTPATH_GG, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[2] == 1):
            if (BCM.SetParam(IDevice.Params.PARAM_LIGHTPATH_GG, onePosition) == 0):
                print("SetParam (LightPath GG 1 Position) FAILED ")
            if (BCM.StartPosition() == 0):
                print("StartPosition FAILED")
            print("Moving Galvo-Galvo BCM to Position 1")
            time.sleep(2)
            if (BCM.SetParam(IDevice.Params.PARAM_LIGHTPATH_GG, zeroPosition) == 0):
                print("SetParam (LightPath GG 0 Position) FAILED ")
            if (BCM.PreflightPosition() == 0):
                print("PreflightPosition FAILED")
            if (BCM.StartPosition() == 0):
                print("StartPosition FAILED")
            if (BCM.PostflightPosition() == 0):
                print("PostflightPosition FAILED")
            print("Moving Galvo-Galvo BCM to Position 0")
            time.sleep(2)
        
    #Sets the Galvo-Resonance BCM to Position 1 and then Position 0  
    if (BCM.GetParam(IDevice.Params.PARAM_LIGHTPATH_GR_SERIALNUMBER, paramString)[0] == 1):
        if (BCM.GetParamInfo(IDevice.Params.PARAM_LIGHTPATH_GR, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[0] == 0):
            print("GetParamInfo (LightPath GR) FAILED ")
        if (BCM.GetParamInfo(IDevice.Params.PARAM_LIGHTPATH_GR, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[2] == 1):
            if (BCM.SetParam(IDevice.Params.PARAM_LIGHTPATH_GR, onePosition) == 0):
                print("SetParam (LightPath GR 1 Position) FAILED ")
            if (BCM.StartPosition() == 0):
                print("StartPosition FAILED")
            print("Moving Galvo-Resonance BCM to Position 1")
            time.sleep(2)
            if (BCM.SetParam(IDevice.Params.PARAM_LIGHTPATH_GR, zeroPosition) == 0):
                print("SetParam (LightPath GR 0 Position) FAILED ")
            if (BCM.PreflightPosition() == 0):
                print("PreflightPosition FAILED")
            if (BCM.StartPosition() == 0):
                print("StartPosition FAILED")
            if (BCM.PostflightPosition() == 0):
                print("PostflightPosition FAILED")
            print("Moving Galvo-Resonance BCM to Position 0")
            time.sleep(1)
        
    #Sets the Camera BCM to Position 1 and then Position 0
    if (BCM.GetParam(IDevice.Params.PARAM_LIGHTPATH_CAMERA_SERIALNUMBER, paramString)[0] == 1):
        if (BCM.GetParamInfo(IDevice.Params.PARAM_LIGHTPATH_CAMERA, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[0] == 0):
            print("GetParamInfo (LightPath Camera) FAILED ")
        if (BCM.GetParamInfo(IDevice.Params.PARAM_LIGHTPATH_CAMERA, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[2] == 1):
            if (BCM.SetParam(IDevice.Params.PARAM_LIGHTPATH_CAMERA, onePosition) == 0):
                print("SetParam (LightPath Camera 1 Position) FAILED ")
            if (BCM.StartPosition() == 0):
                print("StartPosition FAILED")
            print("Moving Camera BCM to Position 1") 
            time.sleep(1)
            if (BCM.SetParam(IDevice.Params.PARAM_LIGHTPATH_CAMERA, zeroPosition) == 0):
                print("SetParam (LightPath Camera 0 Position) FAILED ")
            if (BCM.PreflightPosition() == 0):
                print("PreflightPosition FAILED")
            if (BCM.StartPosition() == 0):
                print("StartPosition FAILED")
            print("Moving Camera BCM to Position 0")
            if (BCM.PostflightPosition() == 0):
                print("PostflightPosition FAILED")        
    if (BCM.TeardownDevice() == 0):
        print("TeardownDevice FAILED")