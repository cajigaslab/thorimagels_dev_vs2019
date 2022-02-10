#Read the Python SDK Instructions powerpoint in the documents folder of the SDK before use.

import time

#Ensure that the Python.NET and numpy packages are installed in your Python environment
import clr
import numpy as np

#Set the current directory to the LSM python folder
import os
os.chdir(r'C:\Program Files\Thorlabs\LSM 4.3\Python')

#Import and instantiate the classes containing the functions of the devices you need
clr.AddReference('CameraFunctions')
clr.AddReference('DeviceFunctions')
from CameraFunctions import Confocal
from DeviceFunctions import ECU
from DeviceFunctions import PMT2100
from DeviceFunctions import LSKGR
from ThorSharedTypes import *

LSMC = Confocal()
PMT2100 = PMT2100()
ECU = ECU()
LSKGR = LSKGR()

paramType = 0           
paramAvail = 0
paramReadOnly = 0
pmin = 0
pmax = 0
paramDefault = 0
deviceCount = 0
device = 0

                              #Enable the Resonance Scanner from the ECU or LSKGR 
paramECUDevice = ECU.FindDevices(deviceCount)[0]
paramLSKGRDevice = LSKGR.FindDevices(deviceCount)[0]
if (paramECUDevice == 0) and (paramLSKGRDevice == 0): 
    print("FindDevices (ECU/LSKGR) FAILED")    
if (paramECUDevice == 1):
    if (ECU.SelectDevice(device) == 0):
        print("SelectDevice ECU FAILED")
        
    #Enable Resonance Scanner (ECU)
    enableScanner = 1
    if (ECU.SetParam(IDevice.Params.PARAM_SCANNER_ENABLE, enableScanner) == 0):
        print("SetParam (Enable Scanner) FAILED ")
    if (ECU.PreflightPosition() == 0):
        print("PreflightPosition FAILED")
    if (ECU.SetupPosition() == 0):
        print("SetupPosition FAILED")
    if (ECU.StartPosition() == 0):
        print("StartPosition FAILED")
    status = ICamera.StatusType.STATUS_READY 
    while (ECU.StatusPosition(status)[1] == ICamera.StatusType.STATUS_BUSY):
        if (ECU.StatusPosition(status)[0] == 0):
            print("StatusPosition FAILED")
    if (ECU.PostflightPosition() == 0):
        print("PostflightPosition FAILED")
elif (paramLSKGRDevice == 1):
    if (LSKGR.SelectDevice(device) == 0):
        print("SelectDevice LSKGR FAILED")
        
    #Enable Resonance Scanner (LSKGR)
    enableScanner = 1
    if (LSKGR.SetParam(IDevice.Params.PARAM_SCANNER_ENABLE, enableScanner) == 0):
        print("SetParam (Enable Scanner) FAILED ")
    if (LSKGR.PreflightPosition() == 0):
        print("PreflightPosition FAILED")
    if (LSKGR.SetupPosition() == 0):
        print("SetupPosition FAILED")
    if (LSKGR.StartPosition() == 0):
        print("StartPosition FAILED")
    status = ICamera.StatusType.STATUS_READY 
    while (LSKGR.StatusPosition(status)[1] == ICamera.StatusType.STATUS_BUSY):
        if (LSKGR.StatusPosition(status)[0] == 0):
            print("StatusPosition FAILED")
    if (LSKGR.PostflightPosition() == 0):
        print("PostflightPosition FAILED") 
        
        
                                    #Enable PMT2100 and Set Gain
paramPMT2100Device = PMT2100.FindDevices(deviceCount)[0]
if (paramPMT2100Device == 0):  
    print("FindDevices PMT2100 FAILED") 
elif (paramPMT2100Device == 1):
    if (PMT2100.SelectDevice(device) == 0):
        print("SelectDevice PMT2100 FAILED")
        
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
    print("PMT1 Gain: ", PMT2100.GetParam(IDevice.Params.PARAM_PMT1_GAIN_POS, gainPos2100)[1])
    gainOffset = 0.0
    if (PMT2100.GetParam(IDevice.Params.PARAM_PMT1_OUTPUT_OFFSET_CURRENT, gainOffset)[0] == 0):
        print("GetParam: (PMT1 Output Offset Current) FAILED")
    print("PMT1 Current Output Offset: ", PMT2100.GetParam(IDevice.Params.PARAM_PMT1_OUTPUT_OFFSET_CURRENT, gainOffset)[1])
    if (PMT2100.PostflightPosition() == 0):
        print("PostflightPosition FAILED")      

    
cameraCount = 0
camera = 0

#Locate and Select the LSM
paramLSMCamera = LSMC.FindCameras(cameraCount)[0]
if (paramLSMCamera == 0):  
    print("FindCameras FAILED")
if (LSMC.SelectCamera(camera) == 0):
    print("SelectCamera FAILED")

#Set the Pixel Resolution
pixelsX = 512.0
pixelsY = 512.0
if (LSMC.SetParam(ICamera.Params.PARAM_LSM_PIXEL_X, pixelsX) == 0):
    print("SetParam (X Pixels) FAILED ")
if (LSMC.SetParam(ICamera.Params.PARAM_LSM_PIXEL_Y, pixelsY) == 0):
    print("SetParam (Y Pixels) FAILED ")
    
#Set Field Scan Size
fieldSize = 100
if (LSMC.SetParam(ICamera.Params.PARAM_LSM_FIELD_SIZE, fieldSize) == 0):
    print("SetParam (Field Size) FAILED ")

#Set Scanner Zoom for ECU
if (paramECUDevice == 1): 
    if (ECU.SetParam(IDevice.Params.PARAM_SCANNER_ZOOM_POS, fieldSize) == 0):
        print("SetParam ECU (Zoom Position) FAILED ")
    if (ECU.PreflightPosition() == 0):
        print("PreflightPosition FAILED")
    if (ECU.SetupPosition() == 0):
        print("SetupPosition FAILED")
    if (ECU.StartPosition() == 0):
        print("StartPosition (ECU Zoom Position) FAILED")
    status = ICamera.StatusType.STATUS_READY 
    while (ECU.StatusPosition(status)[1] == ICamera.StatusType.STATUS_BUSY):
        if (ECU.StatusPosition(status)[0] == 0):
            print("StatusPosition FAILED")
    if (ECU.PostflightPosition() == 0):
        print("PostflightPosition FAILED")
        
#Set Scanner Zoom for LSKGR
elif (paramLSKGRDevice == 1):
    if (LSKGR.SetParam(IDevice.Params.PARAM_SCANNER_ZOOM_POS, fieldSize) == 0):
        print("SetParam LSKGR (Zoom Position) FAILED ")
    if (LSKGR.PreflightPosition() == 0):
        print("PreflightPosition FAILED")
    if (LSKGR.SetupPosition() == 0):
        print("SetupPosition FAILED")
    if (LSKGR.StartPosition() == 0):
        print("StartPosition (LSKGR Zoom Position) FAILED")
    status = ICamera.StatusType.STATUS_READY 
    while (LSKGR.StatusPosition(status)[1] == ICamera.StatusType.STATUS_BUSY):
        if (LSKGR.StatusPosition(status)[0] == 0):
            print("StatusPosition FAILED")
    if (LSKGR.PostflightPosition() == 0):
        print("PostflightPosition FAILED")
        
#Set the Number of Channels to Output
channel = 1.0 #A Output
if (LSMC.SetParam(ICamera.Params.PARAM_LSM_CHANNEL, channel) == 0):
    print("SetParam (Channel Number) FAILED ")

#Set the Synchronous (SWSingleFrame) Trigger Mode
triggerMode = ICamera.TriggerMode.SW_SINGLE_FRAME
if (LSMC.SetParam(ICamera.Params.PARAM_TRIGGER_MODE, triggerMode) == 0):
    print("SetParam (Trigger Mode) FAILED ")

#Set Area Mode
areaMode = ICamera.LSMAreaMode.SQUARE
if (LSMC.SetParam(ICamera.Params.PARAM_LSM_AREAMODE, areaMode) == 0):
    print("SetParam (Area Mode) FAILED ")

#Set Scan Mode
scanMode = ICamera.ScanMode.FORWARD_SCAN
if (LSMC.SetParam(ICamera.Params.PARAM_LSM_SCANMODE, scanMode) == 0):
    print("SetParam (Scan Mode) FAILED ")
    
#Set Average Mode
averageMode = ICamera.AverageMode.AVG_MODE_NONE
if (LSMC.SetParam(ICamera.Params.PARAM_LSM_AVERAGEMODE, averageMode) == 0):
    print("SetParam (Average Mode) FAILED ")  

#Set Pockels Min Voltage
minVoltage = 0
if (LSMC.SetParam(ICamera.Params.PARAM_LSM_POCKELS_MIN_VOLTAGE_0, minVoltage) == 0):
    print("SetParam (Pockels Min Voltage) FAILED ") 
    
#Set Pockels Max Voltage
maxVoltage = 3
if (LSMC.SetParam(ICamera.Params.PARAM_LSM_POCKELS_MAX_VOLTAGE_0, maxVoltage) == 0):
    print("SetParam (Pockels Max Voltage) FAILED ")     
    
#Set Pockels Power Percentage
pockelsPower = 60
if (LSMC.SetParam(ICamera.Params.PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_0, pockelsPower) == 0):
    print("SetParam (Pockels Power) FAILED ")     

#Channel Polarity Values    
POL_NEG = 0
POL_POS = 1   
    
#Set Channel Polarity
if (LSMC.SetParam(ICamera.Params.PARAM_LSM_CHANNEL_POLARITY_1, POL_NEG) == 0):
    print("SetParam (Polarity) FAILED ")
    
#Create a Buffer to Copy Data Into
BYTES_PER_PIXEL = 2
totalOutputChannels = 1      #1: single channel, up to 4 channels
imgSize = int((pixelsX*pixelsY*BYTES_PER_PIXEL*totalOutputChannels))
pDataBuffer = np.ones(imgSize, dtype = np.int16)

#Open a File for Writing
f = open('C:\\Temp\\Sync001.raw', 'wb')

#Preflight the Settings to the LSM
if (LSMC.PreflightAcquisition() == 0):
    print("PreflightAcquisition FAILED ") 
    
#Perform 5 Image Captures Synchronously
for i in range(0,5):
    
    #Prepare for Capture
    if (LSMC.SetupAcquisition() == 0):
        print("SetupAcquisition FAILED ")
        
    #Each Image Capture is Initiated via a Software Trigger
    if (LSMC.StartAcquisition() == 0):
        print("StartAcquisition FAILED ")
    STATUS_PARTIAL = 3
    status = ICamera.StatusType.STATUS_BUSY
    while (LSMC.StatusAcquisition(status)[1] == ICamera.StatusType.STATUS_BUSY) or (LSMC.StatusAcquisition(status)[1] == STATUS_PARTIAL):
        if (LSMC.StatusAcquisition(status)[0] == 0):
            print("StatusAcquisition FAILED ")  
            
    #As Each Image Becomes Available, Copy it to the Allocated Buffer
    copyAcq = LSMC.CopyAcquisition(pDataBuffer, imgSize)
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
if (LSMC.PostflightAcquisition() == 0):
    print("PostflightAcquisition FAILED ")
    
#Disconnect from the Driver
if (LSMC.TeardownCamera() == 0):
    print("TeardownCamera FAILED ")

#Close the File
f.close()

#Disable the Resonance Scanner for ECU
disableScanner = 0
if (paramECUDevice == 1):
    if (ECU.SetParam(IDevice.Params.PARAM_SCANNER_ENABLE, disableScanner) == 0):
        print("SetParam (Disable Scanner) FAILED ")
    if (ECU.PreflightPosition() == 0):
        print("PreflightPosition FAILED")
    if (ECU.SetupPosition() == 0):
        print("SetupPosition FAILED")
    if (ECU.StartPosition() == 0):
        print("StartPosition FAILED")
    status = ICamera.StatusType.STATUS_READY 
    while (ECU.StatusPosition(status)[1] == ICamera.StatusType.STATUS_BUSY):
        if (ECU.StatusPosition(status)[0] == 0):
            print("StatusPosition FAILED")
    if (ECU.PostflightPosition() == 0):
        print("PostflightPosition FAILED")
    if (ECU.TeardownDevice() == 0):
        print("TeardownDevice FAILED")
        
#Disable Resonance Scanner for LSKGR
disableScanner = 0
if (paramLSKGRDevice == 1):
    if (LSKGR.SetParam(IDevice.Params.PARAM_SCANNER_ENABLE, disableScanner) == 0):
        print("SetParam (Disable Scanner) FAILED ")
    if (LSKGR.PreflightPosition() == 0):
        print("PreflightPosition FAILED")
    if (LSKGR.SetupPosition() == 0):
        print("SetupPosition FAILED")
    if (LSKGR.StartPosition() == 0):
        print("StartPosition FAILED")
    status = ICamera.StatusType.STATUS_READY 
    while (LSKGR.StatusPosition(status)[1] == ICamera.StatusType.STATUS_BUSY):
        if (LSKGR.StatusPosition(status)[0] == 0):
            print("StatusPosition FAILED")
    if (LSKGR.PostflightPosition() == 0):
        print("PostflightPosition FAILED")
    #Close the ECU
    if (LSKGR.TeardownDevice() == 0):
        print("TeardownDevice FAILED")

           
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