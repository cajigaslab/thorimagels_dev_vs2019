#Read the Python SDK Instructions powerpoint in the documents folder of the SDK before use.

import time

#Ensure that the Python.NET package is installed in your Python environment
import clr

#Set the current directory to the BScope python folder
import os
os.chdir(r'C:\Program Files\Thorlabs\BScope 4.3\Python')

#Import and instantiate the class containing the functions of the BScope
clr.AddReference('DeviceFunctions')
from DeviceFunctions import BScope
from ThorSharedTypes import *
BScope = BScope()

paramType = 0           
paramAvail = 0
paramReadOnly = 0
pmin = 0
pmax = 0
paramDefault = 0
deviceCount = 0
device = 0
                  #List of function returns when BScope is connected and the enclosed parameters are used:

if (BScope.FindDevices(deviceCount)[0] == 0):  
    print("FindDevices FAILED")
if (BScope.SelectDevice(device) == 0):
    print("SelectDevice FAILED")
    
#Setting the Current Position to be 0 in the X, Y, Z, and R Stages
for i in range(0,4):
    setZero = 1
    if (i == 0):
        if (BScope.SetParam(IDevice.Params.PARAM_X_ZERO, setZero) == 0):
            print("SetParam (New X Zero) FAILED ")
        else:
            print("Setting Current X Position to be New Zero")
    elif (i == 1):
        if (BScope.SetParam(IDevice.Params.PARAM_Y_ZERO, setZero) == 0):
            print("SetParam (New Y Zero) FAILED ")
        else:
            print("Setting Current Y Position to be New Zero")
    elif (i == 2):
        if (BScope.SetParam(IDevice.Params.PARAM_Z_ZERO, setZero) == 0):
            print("SetParam (New Z Zero) FAILED ")
        else:
            print("Setting Current Z Position to be New Zero")
    elif (i == 3):
        if (BScope.SetParam(IDevice.Params.PARAM_R_ZERO, setZero) == 0):
            print("SetParam (New R Zero) FAILED ")
        else:
            print("Setting Current R Position to be New Zero")
    if (BScope.PreflightPosition() == 0):
        print("PreflightPosition FAILED")
    if (BScope.SetupPosition() == 0):
        print("SetupPosition FAILED")
    if (BScope.StartPosition() == 0):
        print("StartPosition FAILED")
    status = ICamera.StatusType.STATUS_READY
    while (BScope.StatusPosition(status)[1] == ICamera.StatusType.STATUS_BUSY):
        if (BScope.StatusPosition(status)[0] == 0):
            print("StatusPosition FAILED")
    if (BScope.PostflightPosition() == 0):
        print("PostflightPosition FAILED")
    time.sleep(.5)
    
#Moving the X Stage
if (BScope.GetParamInfo(IDevice.Params.PARAM_X_POS, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[0] == 0):
    print("GetParamInfo (X-Value) FAILED")
xMax = BScope.GetParamInfo(IDevice.Params.PARAM_X_POS, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[5]
if (BScope.SetParam(IDevice.Params.PARAM_X_POS, xMax/10) == 0):
    print("SetParam (New X-Value) FAILED ")
if (BScope.PreflightPosition() == 0):
    print("PreflightPosition FAILED")
print("Moving X Stage to ", xMax/10)
if (BScope.SetupPosition() == 0):
    print("SetupPosition FAILED")
if (BScope.StartPosition() == 0):
    print("StartPosition FAILED")
status = ICamera.StatusType.STATUS_READY 
while (BScope.StatusPosition(status)[1] == ICamera.StatusType.STATUS_BUSY):
    if (BScope.StatusPosition(status)[0] == 0):
        print("StatusPosition FAILED")
if (BScope.PostflightPosition() == 0):
    print("PostflightPosition FAILED")
readPos = 0.0
time.sleep(1)
if (BScope.GetParam(IDevice.Params.PARAM_X_POS_CURRENT, readPos)[0] == 0):
    print("GetParam (Current X-Value) FAILED")
print("Current X-Value: ", BScope.GetParam(IDevice.Params.PARAM_X_POS_CURRENT, readPos)[1])

#Moving the Y Stage
if (BScope.GetParamInfo(IDevice.Params.PARAM_Y_POS, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[0] == 0):
    print("GetParamInfo (Y-Value) FAILED")
yMin = BScope.GetParamInfo(IDevice.Params.PARAM_Y_POS, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[4]
if (BScope.SetParam(IDevice.Params.PARAM_Y_POS, yMin/10) == 0):
    print("SetParam (New Y-Value) FAILED ") 
if (BScope.PreflightPosition() == 0):
    print("PreflightPosition FAILED")
print("Moving Y Stage to ", yMin/10)
if (BScope.SetupPosition() == 0):
    print("SetupPosition FAILED")
if (BScope.StartPosition() == 0):
    print("StartPosition FAILED")
status = ICamera.StatusType.STATUS_READY 
while (BScope.StatusPosition(status)[1] == ICamera.StatusType.STATUS_BUSY):
    if (BScope.StatusPosition(status)[0] == 0):
        print("StatusPosition FAILED")
if (BScope.PostflightPosition() == 0):
    print("PostflightPosition FAILED")
readPos = 0.0
time.sleep(1)
if (BScope.GetParam(IDevice.Params.PARAM_Y_POS_CURRENT, readPos)[0] == 0):
    print("GetParam (Current Y-Value) FAILED")
print("Current Y-Value: ", BScope.GetParam(IDevice.Params.PARAM_Y_POS_CURRENT, readPos)[1])   

#Moving the Z Stage
if (BScope.GetParamInfo(IDevice.Params.PARAM_Z_POS, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[0] == 0):
    print("GetParamInfo (Z-Value) FAILED")
zMax = BScope.GetParamInfo(IDevice.Params.PARAM_Z_POS, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[5]
if (BScope.SetParam(IDevice.Params.PARAM_Z_POS, zMax/10) == 0):
    print("SetParam (New Z-Value) FAILED ")  
if (BScope.PreflightPosition() == 0):
    print("PreflightPosition FAILED")
print("Moving Z Stage to ", zMax/10)
if (BScope.SetupPosition() == 0):
    print("SetupPosition FAILED")
if (BScope.StartPosition() == 0):
    print("StartPosition FAILED")
status = ICamera.StatusType.STATUS_READY 
while (BScope.StatusPosition(status)[1] == ICamera.StatusType.STATUS_BUSY):
    if (BScope.StatusPosition(status)[0] == 0):
        print("StatusPosition FAILED")
if (BScope.PostflightPosition() == 0):
    print("PostflightPosition FAILED")
readPos = 0.0
time.sleep(1)
if (BScope.GetParam(IDevice.Params.PARAM_Z_POS_CURRENT, readPos)[0] == 0):
    print("GetParam (Current Z-Value) FAILED")
print("Current Z-Value: ", BScope.GetParam(IDevice.Params.PARAM_Z_POS_CURRENT, readPos)[1])

#Moving the R Stage
if (BScope.GetParamInfo(IDevice.Params.PARAM_R_POS, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[0] == 0):
    print("GetParamInfo (R-Value) FAILED")
    
#The R-Value entered into SetParam must be opposite in sign to the desired rotation.
#Entering SetParam(IDevice.Params.PARAM_R_POS, -1.0) will set the R Position to 1.0 degree.
rMax = BScope.GetParamInfo(IDevice.Params.PARAM_R_POS, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[5]
if (BScope.SetParam(IDevice.Params.PARAM_R_POS, -rMax/10) == 0): #Remember to enter opposite sign of desired rotation
    print("SetParam (New R-Value) FAILED ")
if (BScope.PreflightPosition() == 0):
    print("PreflightPosition FAILED")
print("Moving R Stage to ", rMax/10)
if (BScope.SetupPosition() == 0):
    print("SetupPosition FAILED")
if (BScope.StartPosition() == 0):
    print("StartPosition FAILED")
status = ICamera.StatusType.STATUS_READY 
while (BScope.StatusPosition(status)[1] == ICamera.StatusType.STATUS_BUSY):
    if (BScope.StatusPosition(status)[0] == 0):
        print("StatusPosition FAILED")
if (BScope.PostflightPosition() == 0):
    print("PostflightPosition FAILED")
readPos = 0.0
time.sleep(1)
if (BScope.GetParam(IDevice.Params.PARAM_R_POS_CURRENT, readPos)[0] == 0):
    print("GetParam (Current R-Value) FAILED")
print("Current R-Value: ", BScope.GetParam(IDevice.Params.PARAM_R_POS_CURRENT, readPos)[1])  

#Moving Lightpath Mirrors in/out
for j in range(1, -1, -1):
    for k in range(0,3):
        if (k == 0):
            if (BScope.SetParam(IDevice.Params.PARAM_LIGHTPATH_GG, j) == 0):
                print("SetParam (LightPath GG) FAILED ")
        if (k == 1):
            if (BScope.SetParam(IDevice.Params.PARAM_LIGHTPATH_GR, j) == 0):
                print("SetParam (LightPath GR) FAILED ")
        if (k == 2):
            if (BScope.SetParam(IDevice.Params.PARAM_LIGHTPATH_CAMERA, j) == 0):
                print("SetParam (LightPath Camera) FAILED ")
        if (BScope.PreflightPosition() == 0):
            print("PreflightPosition FAILED")
        if (BScope.SetupPosition() == 0):
            print("SetupPosition FAILED")
        if (BScope.StartPosition() == 0):
            print("StartPosition FAILED")
        if (BScope.PostflightPosition() == 0):
            print("PostflightPosition FAILED")
        time.sleep(1)

if (BScope.TeardownDevice() == 0):
    print("TeardownDevice FAILED") 