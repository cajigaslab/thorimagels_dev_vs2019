#Read the Python SDK Instructions powerpoint in the documents folder of the SDK before use.

import time

#Ensure that the Python.NET and numpy packages are installed in your Python environment
import clr
import numpy as np

#Set the current directory to the LSMGalvo python folder
import os
os.chdir(r'C:\Program Files\Thorlabs\LSMGalvo 4.3\Python')

#Import and instantiate the classes containing the functions of the devices you need
clr.AddReference('CameraFunctions')
clr.AddReference('DeviceFunctions')
from CameraFunctions import ConfocalGalvo
from DeviceFunctions import PMT2100
from ThorSharedTypes import *
LSMGalvo = ConfocalGalvo()
PMT2100 = PMT2100()

paramType = 0           
paramAvail = 0
paramReadOnly = 0
pmin = 0.0
pmax = 0.0
paramDefault = 0.0
deviceCount = 0
device = 0


                                     #Enable PMT2100 and Set Gain
paramPMT2100Device = PMT2100.FindDevices(deviceCount)[0]
if (paramPMT2100Device == 0):  
    print("FindDevices FAILED") 
elif (paramPMT2100Device == 1):
    if (PMT2100.SelectDevice(device) == 0):
        print("SelectDevice FAILED")
        
    #Enabling PMT1
    enablePMT = 1.0
    if (PMT2100.SetParam(IDevice.Params.PARAM_PMT1_ENABLE, enablePMT) == 0):
        print("SetParam (Enable PMT1) FAILED")
        
    #Setting PMT1 Gain
    gainPos2100 = 10
    if (PMT2100.SetParam(IDevice.Params.PARAM_PMT1_GAIN_POS, gainPos2100) == 0):
        print("SetParam (PMT1 Gain) FAILED")
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
    print("PMT1 Enabled")
    
    #Getting PMT1 Gain and Current Output Offset
    if (PMT2100.GetParam(IDevice.Params.PARAM_PMT1_GAIN_POS, gainPos2100)[0] == 0):
        print("GetParam: (PMT1 Gain) FAILED")
    print("PMT1 Current Gain: ", PMT2100.GetParam(IDevice.Params.PARAM_PMT1_GAIN_POS, gainPos2100)[1])
    gainOffset = 0.0
    if (PMT2100.GetParam(IDevice.Params.PARAM_PMT1_OUTPUT_OFFSET_CURRENT, gainOffset)[0] == 0):
        print("GetParam: (PMT1 Output Offset Current) FAILED")
    print("PMT1 Current Output Offset: ", PMT2100.GetParam(IDevice.Params.PARAM_PMT1_OUTPUT_OFFSET_CURRENT, gainOffset)[1])
    if (PMT2100.PostflightPosition() == 0):
        print("PostflightPosition FAILED")      
  
                                    
cameraCount = 0
camera = 0

#Locate and Select the LSMGalvo
paramLSMCamera = LSMGalvo.FindCameras(cameraCount)[0]
if (paramLSMCamera == 0):  
    print("FindCameras FAILED")
if (LSMGalvo.SelectCamera(camera) == 0):
    print("SelectCamera FAILED")
enableYGalvo = 1
if (LSMGalvo.SetParam(ICamera.Params.PARAM_LSM_GALVO_ENABLE, enableYGalvo) == 0):
    print("SetParam (Galvo Enable) FAILED ")

#Set the Pixel Resolution
pixelsX = 512.0
pixelsY = 512.0
if (LSMGalvo.SetParam(ICamera.Params.PARAM_LSM_PIXEL_X, pixelsX) == 0):
    print("SetParam (X Pixels) FAILED ")
if (LSMGalvo.SetParam(ICamera.Params.PARAM_LSM_PIXEL_Y, pixelsY) == 0):
    print("SetParam (Y Pixels) FAILED ")

#Set Field Scan Size and Zoom
fieldSize = 100
if (LSMGalvo.SetParam(ICamera.Params.PARAM_LSM_FIELD_SIZE, fieldSize) == 0):
    print("SetParam (Field Size) FAILED ")

#Set the Number of Channels to Output
channel = 1.0 #A Output
if (LSMGalvo.SetParam(ICamera.Params.PARAM_LSM_CHANNEL, channel) == 0):
    print("SetParam (Channel Number) FAILED ")

#Set the Synchronous (SWSingleFrame) Trigger Mode
triggerMode = ICamera.TriggerMode.SW_SINGLE_FRAME
if (LSMGalvo.SetParam(ICamera.Params.PARAM_TRIGGER_MODE, triggerMode) == 0):
    print("SetParam (Trigger Mode) FAILED ")

#Set Area Mode
areaMode = ICamera.LSMAreaMode.SQUARE
if (LSMGalvo.SetParam(ICamera.Params.PARAM_LSM_AREAMODE, areaMode) == 0):
    print("SetParam (Area Mode) FAILED ")

#Set Scan Mode
scanMode = ICamera.ScanMode.FORWARD_SCAN
if (LSMGalvo.SetParam(ICamera.Params.PARAM_LSM_SCANMODE, scanMode) == 0):
    print("SetParam (Scan Mode) FAILED ")
    
#Set Average Mode
averageMode = ICamera.AverageMode.AVG_MODE_NONE
if (LSMGalvo.SetParam(ICamera.Params.PARAM_LSM_AVERAGEMODE, averageMode) == 0):
    print("SetParam (Average Mode) FAILED ")
    
#Set Pockels Min Voltage
minVoltage = 0
if (LSMGalvo.SetParam(ICamera.Params.PARAM_LSM_POCKELS_MIN_VOLTAGE_0, minVoltage) == 0):
    print("SetParam (Pockels Min Voltage) FAILED ") 
    
#Set Pockels Max Voltage
maxVoltage = 3
if (LSMGalvo.SetParam(ICamera.Params.PARAM_LSM_POCKELS_MAX_VOLTAGE_0, maxVoltage) == 0):
    print("SetParam (Pockels Max Voltage) FAILED ")  
    
#Set Pockels Power Percentage
pockelsPower = 60
if (LSMGalvo.SetParam(ICamera.Params.PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_0, pockelsPower) == 0):
    print("SetParam (Pockels Power) FAILED ") 
    
#Channel Polarity Values    
POL_NEG = 0
POL_POS = 1

#Set Channel Polarity  
if (LSMGalvo.SetParam(ICamera.Params.PARAM_LSM_CHANNEL_POLARITY_1, POL_NEG) == 0):
    print("SetParam (Polarity) FAILED ")

#Create a Buffer to Copy Data Into
BYTES_PER_PIXEL = 2
totalOutputChannels = 1      #1: single channel, 4: multiple channels
imgSize = int((pixelsX*pixelsY*BYTES_PER_PIXEL*totalOutputChannels))
pDataBuffer = np.ones(imgSize, dtype = np.int16)

#Open a File for Writing
f = open('C:\\Temp\\SyncGalvo001.raw', 'wb')

#Preflight the Settings to the LSM
if (LSMGalvo.PreflightAcquisition() == 0):
    print("PreflightAcquisition FAILED ")

#Perform 5 Image Captures Synchronously
for i in range(0,5):
    
    #Set Force Updating Parameters at SetupAcquisition
    forceUpdate = 1
    if (LSMGalvo.SetParam(ICamera.Params.PARAM_LSM_FORCE_SETTINGS_UPDATE, forceUpdate) == 0):
        print("SetParam (Force Settings Update) FAILED ")
        
    #Prepare for Capture
    if (LSMGalvo.SetupAcquisition() == 0):
        print("SetupAcquisition FAILED ")      
        
    #Each Image Capture is Initiated via a Software Trigger
    if (LSMGalvo.StartAcquisition() == 0):
        print("StartAcquisition FAILED ")
    time.sleep(.6)
    STATUS_PARTIAL = 3
    status = ICamera.StatusType.STATUS_BUSY
    while (LSMGalvo.StatusAcquisition(status)[1] == ICamera.StatusType.STATUS_BUSY) or (LSMGalvo.StatusAcquisition(status)[1] == STATUS_PARTIAL):
        if (LSMGalvo.StatusAcquisition(status)[0] == 0):
            print("StatusAcquisition FAILED ")
            
    #As Each Image Becomes Available Copy it to the Allocated Buffer
    copyAcq = LSMGalvo.CopyAcquisition(pDataBuffer, imgSize)
    if (copyAcq[0] == 0):
        print("CopyAcquisition FAILED ")
    
    #Copy the Data from the Buffer into an int16 Numpy Array
    elif (copyAcq[0] == 1):
        cpDataBuffer = copyAcq[1]
        dataArray = np.empty(len(cpDataBuffer), dtype = np.int16)
        for j in range(len(cpDataBuffer)): 
            dataArray[j] = cpDataBuffer[j]
        
        #Write Data to the File
        dataArray.tofile(f)
        print("Captured Frame ", i)
        
#Stop the Acquisition
if (LSMGalvo.PostflightAcquisition() == 0):
    print("PostflightAcquisition FAILED ")
    
#Disconnect from the Driver
if (LSMGalvo.TeardownCamera() == 0):
    print("TeardownCamera FAILED ")
    
#Close the File
f.close()

#Disable the PMT2100
if (paramPMT2100Device == 1):
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