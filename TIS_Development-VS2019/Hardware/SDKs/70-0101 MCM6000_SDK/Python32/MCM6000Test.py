#Read the Python SDK Instructions powerpoint in the documents folder of the SDK before use.

import time

#Ensure that the Python.NET package is installed in your Python environment
import clr
#Set the current directory to the MCM6000 python folder
import os
os.chdir(r'C:\Program Files\Thorlabs\MCM6000 4.3\Python')

#Import and instantiate the class containing the functions of the MCM6000
clr.AddReference('DeviceFunctions')
from DeviceFunctions import MCM6000
from ThorSharedTypes import *
MCM6000 = MCM6000()

paramType = 0           
paramAvail = 0
paramReadOnly = 0
pmin = 0
pmax = 0
paramDefault = 0
deviceCount = 0
device = 0
                  #List of function returns when MCM6000 is connected and the enclosed parameters are used:

if (MCM6000.FindDevices(deviceCount)[0] == 0): 
    print("FindDevices FAILED")
if (MCM6000.SelectDevice(device) == 0):
    print("SelectDevice FAILED")
    
#Setting Current Position to be Zero for X,Y,Z,R
for i in range(0,4):
    setZero = 1
    if (i == 0):
        if (MCM6000.SetParam(IDevice.Params.PARAM_X_ZERO, setZero) == 0):
            print("SetParam (New X Zero) FAILED ")
        print("Setting Current X Position to be Zero")
    elif (i == 1):
        if (MCM6000.SetParam(IDevice.Params.PARAM_Y_ZERO, setZero) == 0):
            print("SetParam (New Y Zero) FAILED ")
        print("Setting Current Y Position to be Zero")
    elif (i == 2):
        if (MCM6000.SetParam(IDevice.Params.PARAM_Z_ZERO, setZero) == 0):
            print("SetParam (New Z Zero) FAILED ")
        print("Setting Current Z Position to be Zero")
    elif (i == 3):
        if (MCM6000.SetParam(IDevice.Params.PARAM_R_ZERO, setZero) == 0):
            print("SetParam (New R Zero) FAILED ")
        print("Setting Current R Position to be Zero")
    if (MCM6000.PreflightPosition() == 0):
        print("PreflightPosition FAILED")
    if (MCM6000.SetupPosition() == 0):
        print("SetupPosition FAILED")
    if (MCM6000.StartPosition() == 0):
        print("StartPosition FAILED")
    status = ICamera.StatusType.STATUS_READY 
    while (MCM6000.StatusPosition(status)[1] == ICamera.StatusType.STATUS_BUSY):
        if (MCM6000.StatusPosition(status)[0] == 0):
            print("StatusPosition FAILED")
    if (MCM6000.PostflightPosition() == 0):
        print("PostflightPosition FAILED")
    time.sleep(.5)

#Moving the X Stage
if (MCM6000.GetParamInfo(IDevice.Params.PARAM_X_POS, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[0] == 0):
    print("GetParamInfo (X-Value) FAILED")
xMax = MCM6000.GetParamInfo(IDevice.Params.PARAM_X_POS, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[5]
if (MCM6000.SetParam(IDevice.Params.PARAM_X_POS, xMax/10) == 0):
    print("SetParam (New X-Value) FAILED ") 
if (MCM6000.PreflightPosition() == 0):
    print("PreflightPosition FAILED")
print("Moving X Stage to ", xMax/10)
if (MCM6000.SetupPosition() == 0):
    print("SetupPosition FAILED")
if (MCM6000.StartPosition() == 0):
    print("StartPosition FAILED")
status = ICamera.StatusType.STATUS_READY
while (MCM6000.StatusPosition(status)[1] == ICamera.StatusType.STATUS_BUSY):
    if (MCM6000.StatusPosition(status)[0] == 0):
        print("StatusPosition FAILED")
if (MCM6000.PostflightPosition() == 0):
    print("PostflightPosition FAILED")
readPos = 0.0
time.sleep(1)
if (MCM6000.GetParam(IDevice.Params.PARAM_X_POS_CURRENT, readPos)[0] == 0):
    print("GetParam (Current X-Value) FAILED")
print("Current X-Value: ", MCM6000.GetParam(IDevice.Params.PARAM_X_POS_CURRENT, readPos)[1])

#Moving the Y Stage
if (MCM6000.GetParamInfo(IDevice.Params.PARAM_Y_POS, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[0] == 0):
    print("GetParamInfo (Y-Value) FAILED")
yMax = MCM6000.GetParamInfo(IDevice.Params.PARAM_Y_POS, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[5]
if (MCM6000.SetParam(IDevice.Params.PARAM_Y_POS, yMax/10) == 0):
    print("SetParam (New Y-Value) FAILED ")
if (MCM6000.PreflightPosition() == 0):
    print("PreflightPosition FAILED")
print("Moving Y Stage to ", yMax/10)
if (MCM6000.SetupPosition() == 0):
    print("SetupPosition FAILED")
if (MCM6000.StartPosition() == 0):
    print("StartPosition FAILED")
status = ICamera.StatusType.STATUS_READY
while (MCM6000.StatusPosition(status)[1] == ICamera.StatusType.STATUS_BUSY):
    if (MCM6000.StatusPosition(status)[0] == 0):
        print("StatusPosition FAILED")
if (MCM6000.PostflightPosition() == 0):
    print("PostflightPosition FAILED")
readPos = 0.0
time.sleep(1)    
if (MCM6000.GetParam(IDevice.Params.PARAM_Y_POS_CURRENT, readPos)[0] == 0):
    print("GetParam (Current Y-Value) FAILED")
print("Current Y-Value: ", MCM6000.GetParam(IDevice.Params.PARAM_Y_POS_CURRENT, readPos)[1])

#Moving the Z Stage
if (MCM6000.GetParamInfo(IDevice.Params.PARAM_Z_POS, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[0] == 0):
    print("GetParamInfo (Z-Value) FAILED")
zMax = MCM6000.GetParamInfo(IDevice.Params.PARAM_Z_POS, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[5]
if (MCM6000.SetParam(IDevice.Params.PARAM_Z_POS, zMax/10) == 0):
    print("SetParam (New Z-Value) FAILED ") 
if (MCM6000.PreflightPosition() == 0):
    print("PreflightPosition FAILED")
print("Moving Z Stage to ", zMax/10)
if (MCM6000.SetupPosition() == 0):
    print("SetupPosition FAILED")
if (MCM6000.StartPosition() == 0):
    print("StartPosition FAILED")
status = ICamera.StatusType.STATUS_READY 
while (MCM6000.StatusPosition(status)[1] == ICamera.StatusType.STATUS_BUSY):
    if (MCM6000.StatusPosition(status)[0] == 0):
        print("StatusPosition FAILED")
if (MCM6000.PostflightPosition() == 0):
    print("PostflightPosition FAILED")
readPos = 0.0
time.sleep(1)
if (MCM6000.GetParam(IDevice.Params.PARAM_Z_POS_CURRENT, readPos)[0] == 0):
    print("GetParam (Current Z-Value) FAILED")
print("Current Z-Value: ", MCM6000.GetParam(IDevice.Params.PARAM_Z_POS_CURRENT, readPos)[1])

#Moving the R Stage
if (MCM6000.GetParamInfo(IDevice.Params.PARAM_R_POS, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[0] == 0):
    print("GetParamInfo (R-Value) FAILED")
rMax = MCM6000.GetParamInfo(IDevice.Params.PARAM_R_POS, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[5]
if (MCM6000.SetParam(IDevice.Params.PARAM_R_POS, rMax/10) == 0):
    print("SetParam (New R-Value) FAILED ") 
if (MCM6000.PreflightPosition() == 0):
    print("PreflightPosition FAILED")
print("Moving R Stage to ", rMax/10)
if (MCM6000.SetupPosition() == 0):
    print("SetupPosition FAILED")
if (MCM6000.StartPosition() == 0):
    print("StartPosition FAILED")
status = ICamera.StatusType.STATUS_READY
while (MCM6000.StatusPosition(status)[1] == ICamera.StatusType.STATUS_BUSY):
    if (MCM6000.StatusPosition(status)[0] == 0):
        print("StatusPosition FAILED")
if (MCM6000.PostflightPosition() == 0):
    print("PostflightPosition FAILED")
readPos = 0.0
time.sleep(1)
if (MCM6000.GetParam(IDevice.Params.PARAM_R_POS_CURRENT, readPos)[0] == 0):
    print("GetParam (Current R-Value) FAILED")
print("Current R-Value: ", MCM6000.GetParam(IDevice.Params.PARAM_R_POS_CURRENT, readPos)[1])

#Moving Lightpath Mirrors in/out
for j in range(1, -1, -1):
    for k in range(0,3):
        if (k == 0):
            if (MCM6000.SetParam(IDevice.Params.PARAM_LIGHTPATH_GG, j) == 0):
                print("SetParam (LightPath GG) FAILED ")
        if (k == 1):
            if (MCM6000.SetParam(IDevice.Params.PARAM_LIGHTPATH_GR, j) == 0):
                print("SetParam (LightPath GR) FAILED ")
        if (k == 2):
            if (MCM6000.SetParam(IDevice.Params.PARAM_LIGHTPATH_CAMERA, j) == 0):
                print("SetParam (LightPath Camera) FAILED ")
        if (MCM6000.PreflightPosition() == 0):
            print("PreflightPosition FAILED")
        if (MCM6000.SetupPosition() == 0):
            print("SetupPosition FAILED")
        if (MCM6000.StartPosition() == 0):
            print("StartPosition FAILED")
        if (MCM6000.PostflightPosition() == 0):
            print("PostflightPosition FAILED")
        time.sleep(1)

if (MCM6000.TeardownDevice() == 0):
    print("TeardownDevice FAILED")