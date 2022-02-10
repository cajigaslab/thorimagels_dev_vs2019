#Read the Python SDK Instructions powerpoint in the documents folder of the SDK before use.

import time

#Ensure that the Python.NET package is installed in your Python environment
import clr

#Set the current directory to the MCLS python folder
import os
os.chdir(r'C:\Program Files\Thorlabs\MCLS 4.3\Python')

#Import and instantiate the class containing the functions of the MCLS
clr.AddReference('DeviceFunctions')
from DeviceFunctions import MCLS
from ThorSharedTypes import *
MCLS = MCLS()

paramType = 0           
paramAvail = 0
paramReadOnly = 0
pmin = 0
pmax = 0
paramDefault = 0
deviceCount = 0
device = 0

                  #List of function returns when MCLS is connected and the enclosed parameters are used:

if (MCLS.FindDevices(deviceCount)[0] == 0): 
    print("FindDevices FAILED")
if (MCLS.SelectDevice(device) == 0):
    print("SelectDevice FAILED")
    
#Enable Laser1
enableLaser = 1
if (MCLS.SetParam(IDevice.Params.PARAM_LASER1_ENABLE, enableLaser) == 0):
    print("SetParam (Laser Enabled) FAILED ")
if (MCLS.PreflightPosition() == 0):
    print("PreflightPosition FAILED") 
if (MCLS.SetupPosition() == 0):
    print("SetupPosition FAILED")
if (MCLS.StartPosition() == 0):
    print("StartPosition FAILED")
status = ICamera.StatusType.STATUS_READY
while (MCLS.StatusPosition(status)[1] == ICamera.StatusType.STATUS_BUSY):
    if (MCLS.StatusPosition(status)[0] == 0):
        print("StatusPosition FAILED")
if (MCLS.PostflightPosition() == 0):
    print("PostflightPosition FAILED")
time.sleep(1)

#Set Power for Laser1
if (MCLS.GetParamInfo(IDevice.Params.PARAM_LASER1_POWER, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[0] == 0):
    print("GetParamInfo (Laser1 Power) FAILED ")
max1 = MCLS.GetParamInfo(IDevice.Params.PARAM_LASER1_POWER, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[5]
min1 = MCLS.GetParamInfo(IDevice.Params.PARAM_LASER1_POWER, paramType, paramAvail, paramReadOnly, pmin, pmax, paramDefault)[4]
print("Max Laser Power:", max1)
print("Min Laser Power:", min1)
laserPower = (((max1 - min1)/10) + min1)
if (MCLS.SetParam(IDevice.Params.PARAM_LASER1_POWER, laserPower) == 0):
    print("SetParam (Laser1 Power) FAILED ")
if (MCLS.PreflightPosition() == 0):
    print("PreflightPosition FAILED")
if (MCLS.SetupPosition() == 0):
    print("SetupPosition FAILED")
if (MCLS.StartPosition() == 0):
    print("StartPosition FAILED")
status = ICamera.StatusType.STATUS_READY
while (MCLS.StatusPosition(status)[1] == ICamera.StatusType.STATUS_BUSY):
    if (MCLS.StatusPosition(status)[0] == 0):
        print("StatusPosition FAILED")
if (MCLS.PostflightPosition() == 0):
    print("PostflightPosition FAILED")

#Read Power for Laser1
if (MCLS.GetParam(IDevice.Params.PARAM_LASER1_POWER_CURRENT, laserPower)[0] == 0):
    print("GetParam (Laser1 Power) FAILED ")
print("Laser Power: ", MCLS.GetParam(IDevice.Params.PARAM_LASER1_POWER_CURRENT, laserPower)[1], "Tested - will be on for 3 seconds")
time.sleep(3)

#Disable Laser1
disableLaser = 0
if (MCLS.SetParam(IDevice.Params.PARAM_LASER1_ENABLE, disableLaser) == 0):
    print("SetParam (Laser Disabled) FAILED ")
if (MCLS.PreflightPosition() == 0):
    print("PreflightPosition FAILED")
if (MCLS.SetupPosition() == 0):
    print("SetupPosition FAILED")
if (MCLS.StartPosition() == 0):
    print("StartPosition FAILED")
status = ICamera.StatusType.STATUS_READY
while (MCLS.StatusPosition(status)[1] == ICamera.StatusType.STATUS_BUSY):
    if (MCLS.StatusPosition(status)[0] == 0):
        print("StatusPosition FAILED")
if (MCLS.PostflightPosition() == 0):
    print("PostflightPosition FAILED")
print("Turn Off Laser")

if (MCLS.TeardownDevice() == 0):
    print("TeardownDevice FAILED")