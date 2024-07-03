namespace CaptureSetupDll.Model
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Runtime.InteropServices;
    using System.Timers;
    using System.Windows;
    using System.Windows.Data;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Threading;
    using System.Xml;

    using GeometryUtilities;

    using ThorLogging;

    using ThorSharedTypes;

    public partial class CaptureSetup : INotifyPropertyChanged
    {
        #region Fields

        private static ReportBleachNowFinished _BleachNowFinishedCallBack;
        private static PreBleachCallback _PreBleachCallBack;

        private int _bleachFrameIndex = 0;
        private int _bleachFrames = 1;
        List<BleachWaveParams> _bleachParamList = new List<BleachWaveParams>();
        private string _bleachPath = string.Empty;
        private int _bleachPixelArrayIndex = 0;
        private List<PixelArray> _bleachPixelArrayList = new List<PixelArray>();
        private int _bleachPixelIndex = 0;
        private bool _isBleachFinished = false;
        private bool _isBleaching = false;
        private bool _isBleachStopped = true;
        private bool _isPreBleachBuilding = false;
        private double _powerShiftUS = 0.0;

        //private double _preBleachPower;
        private int _preBleachWavelength = 0;

        #endregion Fields

        #region Delegates

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void PreBleachCallback(ref int status);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void ReportBleachNowFinished();

        #endregion Delegates

        #region Properties

        public double BleachCalibrateAreaAngle
        {
            get
            {
                double dVal = 0;
                if (XmlManager.ReadAttribute<double>(out dVal, RegistrationDoc, "/ThorImageRegistration/Registrations/Registration[@Modality=" + "'" + ResourceManagerCS.GetModality() + "']/LUT[@Active='1']", "areaAngle", 0, 0))
                    return dVal;

                return XmlManager.ReadAttribute<double>(ExperimentDoc, "/ThorImageExperiment/Photobleaching", "areaAngle", 0, 0);
            }
        }

        public int BleachCalibrateFieldSize
        {
            get
            {
                int iVal = 0;
                if (XmlManager.ReadAttribute<int>(out iVal, RegistrationDoc, "/ThorImageRegistration/Registrations/Registration[@Modality=" + "'" + ResourceManagerCS.GetModality() + "']/LUT[@Active='1']", "fieldSize", 0, 0))
                    return iVal;

                return XmlManager.ReadAttribute<int>(ExperimentDoc, "/ThorImageExperiment/Photobleaching", "fieldSize", 0, 0);
            }
        }

        public double[] BleachCalibrateFineOffsetXY
        {
            get
            {
                double[] dVal = new double[2] { 0, 0 };
                if (XmlManager.ReadAttribute<double>(out dVal[0], RegistrationDoc, "/ThorImageRegistration/Registrations/Registration[@Modality=" + "'" + ResourceManagerCS.GetModality() + "']/LUT[@Active='1']", "fineOffsetX", 0, 0) &&
                    XmlManager.ReadAttribute<double>(out dVal[1], RegistrationDoc, "/ThorImageRegistration/Registrations/Registration[@Modality=" + "'" + ResourceManagerCS.GetModality() + "']/LUT[@Active='1']", "fineOffsetY", 0, 0))
                {
                    return dVal;
                }
                else if (XmlManager.ReadAttribute<double>(out dVal[0], ExperimentDoc, "/ThorImageExperiment/Photobleaching", "fineOffsetX", 0, 0) &&
                    XmlManager.ReadAttribute<double>(out dVal[1], ExperimentDoc, "/ThorImageExperiment/Photobleaching", "fineOffsetY", 0, 0))
                {
                    return dVal;
                }
                return null;
            }
        }

        public double[] BleachCalibrateFineScaleXY
        {
            get
            {
                double[] dVal = new double[2] { 0, 0 };
                if (XmlManager.ReadAttribute<double>(out dVal[0], RegistrationDoc, "/ThorImageRegistration/Registrations/Registration[@Modality=" + "'" + ResourceManagerCS.GetModality() + "']/LUT[@Active='1']", "fineScaleX", 1, 0) &&
                    XmlManager.ReadAttribute<double>(out dVal[1], RegistrationDoc, "/ThorImageRegistration/Registrations/Registration[@Modality=" + "'" + ResourceManagerCS.GetModality() + "']/LUT[@Active='1']", "fineScaleY", 1, 0))
                {
                    return dVal;
                }
                else if (XmlManager.ReadAttribute<double>(out dVal[0], ExperimentDoc, "/ThorImageExperiment/Photobleaching", "fineScaleX", 1, 0) &&
                    XmlManager.ReadAttribute<double>(out dVal[1], ExperimentDoc, "/ThorImageExperiment/Photobleaching", "fineScaleY", 1, 0))
                {
                    return dVal;
                }
                return null;
            }
        }

        public int[] BleachCalibrateFlipHV
        {
            get
            {
                int[] iVal = new int[2] { 0, 0 };
                if (XmlManager.ReadAttribute<int>(out iVal[0], RegistrationDoc, "/ThorImageRegistration/Registrations/Registration[@Modality=" + "'" + ResourceManagerCS.GetModality() + "']/LUT[@Active='1']", "horizontalFlip", 0, 0) &&
                    XmlManager.ReadAttribute<int>(out iVal[1], RegistrationDoc, "/ThorImageRegistration/Registrations/Registration[@Modality=" + "'" + ResourceManagerCS.GetModality() + "']/LUT[@Active='1']", "verticalFlip", 0, 0))
                {
                    return iVal;
                }
                else if (XmlManager.ReadAttribute<int>(out iVal[0], ExperimentDoc, "/ThorImageExperiment/Photobleaching", "horizontalFlip", 0, 0) &&
                    XmlManager.ReadAttribute<int>(out iVal[1], ExperimentDoc, "/ThorImageExperiment/Photobleaching", "verticalFlip", 0, 0))
                {
                    return iVal;
                }
                return null;
            }
        }

        public int[] BleachCalibrateOffsetXY
        {
            get
            {
                int[] iVal = new int[2] { 0, 0 };
                if (XmlManager.ReadAttribute<int>(out iVal[0], RegistrationDoc, "/ThorImageRegistration/Registrations/Registration[@Modality=" + "'" + ResourceManagerCS.GetModality() + "']/LUT[@Active='1']", "offsetX", 0, 0) &&
                    XmlManager.ReadAttribute<int>(out iVal[1], RegistrationDoc, "/ThorImageRegistration/Registrations/Registration[@Modality=" + "'" + ResourceManagerCS.GetModality() + "']/LUT[@Active='1']", "offsetY", 0, 0))
                {
                    return iVal;
                }
                else if (XmlManager.ReadAttribute<int>(out iVal[0], ExperimentDoc, "/ThorImageExperiment/Photobleaching", "offsetX", 0, 0) &&
                    XmlManager.ReadAttribute<int>(out iVal[1], ExperimentDoc, "/ThorImageExperiment/Photobleaching", "offsetY", 0, 0))
                {
                    return iVal;
                }
                return null;
            }
        }

        public int[] BleachCalibratePixelXY
        {
            get
            {
                int[] iVal = new int[2] { 0, 0 };
                if (XmlManager.ReadAttribute<int>(out iVal[0], RegistrationDoc, "/ThorImageRegistration/Registrations/Registration[@Modality=" + "'" + ResourceManagerCS.GetModality() + "']/LUT[@Active='1']", "pixelX", 0, 0) &&
                    XmlManager.ReadAttribute<int>(out iVal[1], RegistrationDoc, "/ThorImageRegistration/Registrations/Registration[@Modality=" + "'" + ResourceManagerCS.GetModality() + "']/LUT[@Active='1']", "pixelY", 0, 0))
                {
                    return iVal;
                }
                else if (XmlManager.ReadAttribute<int>(out iVal[0], ExperimentDoc, "/ThorImageExperiment/Photobleaching", "pixelX", 0, 0) &&
                    XmlManager.ReadAttribute<int>(out iVal[1], ExperimentDoc, "/ThorImageExperiment/Photobleaching", "pixelY", 0, 0))
                {
                    return iVal;
                }
                return null;
            }
        }

        public double[] BleachCalibratePockelsVoltageMax0
        {
            get
            {
                if (IsStimulator)
                {
                    //no calibration will be done in stimulator mode,
                    //get from configuration instead and only build waveform for in order of configured
                    double dVal = 0.0;
                    List<double> dList = new List<double>();
                    for (int i = 0; i < (int)Constants.MAX_GG_POCKELS_CELL_COUNT; i++)
                    {
                        ICamera.Params pwrLevel;
                        Enum.TryParse("PARAM_LSM_POCKELS_MAX_VOLTAGE_" + i, false, out pwrLevel);
                        if (1 == ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)pwrLevel, ref dVal) && dVal >= 0)
                        {
                            dList.Add(dVal);
                        }
                    }
                    return dList.ToArray();
                }
                else
                {
                    double dVal = 0;
                    if (XmlManager.ReadAttribute<double>(out dVal, RegistrationDoc, "/ThorImageRegistration/Registrations/Registration[@Modality=" + "'" + ResourceManagerCS.GetModality() + "']/LUT[@Active='1']", "powerMax", 0, 0))
                        return new double[1] { dVal };

                    return new double[1] { XmlManager.ReadAttribute<double>(ExperimentDoc, "/ThorImageExperiment/Photobleaching", "powerMax", 0, 0) };
                }
            }
        }

        public double[] BleachCalibratePockelsVoltageMin0
        {
            get
            {
                if (IsStimulator)
                {
                    //no calibration will be done in stimulator mode,
                    //get from configuration instead and only build waveform for in order of configured
                    double dVal = 0.0;
                    List<double> dList = new List<double>();
                    for (int i = 0; i < (int)Constants.MAX_GG_POCKELS_CELL_COUNT; i++)
                    {
                        ICamera.Params pwrLevel;
                        Enum.TryParse("PARAM_LSM_POCKELS_MIN_VOLTAGE_" + i, false, out pwrLevel);
                        if (1 == ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)pwrLevel, ref dVal) && dVal >= 0)
                        {
                            dList.Add(dVal);
                        }
                    }
                    return dList.ToArray();
                }
                else
                {
                    double dVal = 0;
                    if (XmlManager.ReadAttribute<double>(out dVal, RegistrationDoc, "/ThorImageRegistration/Registrations/Registration[@Modality=" + "'" + ResourceManagerCS.GetModality() + "']/LUT[@Active='1']", "powerMin", 0, 0))
                        return new double[1] { dVal };

                    return new double[1] { XmlManager.ReadAttribute<double>(ExperimentDoc, "/ThorImageExperiment/Photobleaching", "powerMin", 0, 0) };
                }
            }
        }

        public int BleachCalibrateScaleYScan
        {
            get
            {
                int iVal = 0;
                if (XmlManager.ReadAttribute<int>(out iVal, RegistrationDoc, "/ThorImageRegistration/Registrations/Registration[@Modality=" + "'" + ResourceManagerCS.GetModality() + "']/LUT[@Active='1']", "scaleYScan", 0, 0))
                    return iVal;

                return XmlManager.ReadAttribute<int>(ExperimentDoc, "/ThorImageExperiment/Photobleaching", "scaleYScan", 0, 0);
            }
        }

        public int BleachFrames
        {
            get { return _bleachFrames; }
            set { _bleachFrames = value; }
        }

        public int BleachInternalClockRate
        {
            get
            {
                int temp = 0;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_INTERNALCLOCKRATE, ref temp);
                return temp;
            }
        }

        public DateTime BleachLastRunTime
        {
            get;
            set;
        }

        public double BleachLongIdle
        {
            get;
            set;
        }

        public double[] BleachLSMOffsetXYFine
        {
            get
            {
                double[] offsetFineXY = new double[2];
                if (0 == GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_FINE_OFFSET_X, ref offsetFineXY[0]))
                    return null;
                if (0 == GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_FINE_OFFSET_Y, ref offsetFineXY[1]))
                    return null;
                return offsetFineXY;
            }
            set
            {
                SetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_FINE_OFFSET_X, value[0]);
                SetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_FINE_OFFSET_Y, value[1]);
            }
        }

        public List<BleachWaveParams> BleachParamList
        {
            get
            { return _bleachParamList; }
            set
            {
                _bleachParamList = value;
                OnPropertyChanged("BleachParamList");
            }
        }

        public double BleachPower
        {
            get;
            set;
        }

        public int BleachPowerEnable
        {
            get;
            set;
        }

        public int BleachQuery
        {
            get;
            set;
        }

        public int BleachWavelength
        {
            get;
            set;
        }

        public int BleachWavelengthEnable
        {
            get;
            set;
        }

        public bool IsBleachFinished
        {
            get
            {
                return _isBleachFinished;
            }
            set
            {
                _isBleachFinished = value;
                OnPropertyChanged("IsBleachFinished");
            }
        }

        public bool IsBleaching
        {
            get
            {
                return _isBleaching;
            }
            set
            {
                _isBleaching = value;
                OnPropertyChanged("IsBleaching");
            }
        }

        public bool IsBleachStopped
        {
            get
            {
                return _isBleachStopped;
            }
            set
            {
                _isBleachStopped = value;
                OnPropertyChanged("IsBleachStopped");
            }
        }

        public bool IsPreBleachBuilding
        {
            get
            {
                return _isPreBleachBuilding;
            }
            set
            {
                _isPreBleachBuilding = value;
                OnPropertyChanged("IsPreBleachBuilding");
            }
        }

        public double PowerShiftUS
        {
            get { return _powerShiftUS; }
            set { _powerShiftUS = value; }
        }

        public int PreBleachWavelength
        {
            get
            {
                return _preBleachWavelength;
            }
            set
            {
                _preBleachWavelength = value;
                OnPropertyChanged("PreBleachWavelength");
            }
        }

        public WaveformDriverType WaveformDriverType
        {
            get
            {
                int temp = (int)WaveformDriverType.WaveformDriver_NI;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_WAVEFORM_DRIVER_TYPE, ref temp);

                return (WaveformDriverType)temp;
            }
        }

        #endregion Properties

        #region Methods

        public void CenterScanners(int selectedCamera, bool centerWithOffset)
        {
            int centerOffset = (centerWithOffset) ? 1 : 0;
            SetCameraParamInt(selectedCamera, (int)ICamera.Params.PARAM_LSM_CENTER_WITH_OFFSET, centerOffset);
            CenterLSMScanners(selectedCamera);
        }

        public int GetBleacherFieldSizeCalibration(ref double fieldSizeCal)
        {
            return GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_FIELD_SIZE_CALIBRATION, ref fieldSizeCal);
        }

        public int GetBleachFlipScanHV(int i, ref int horizontalVertical)
        {
            int ret = 0;
            double dVal = 0;
            switch (i)
            {
                case 0:
                    ret = GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_HORIZONTAL_FLIP, ref dVal);
                    horizontalVertical = (int)dVal;
                    return ret;
                case 1:
                    ret = GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_VERTICAL_SCAN_DIRECTION, ref dVal);
                    horizontalVertical = (int)dVal;
                    return ret;
                default:
                    return 0;
            }
        }

        public int GetBleachLSMAreaAngle(ref double areaAngle)
        {
            return GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_SCANAREA_ANGLE, ref areaAngle);
        }

        public int GetBleachLSMFieldScaleXYFine(int i, ref double fieldScaleFine)
        {
            switch (i)
            {
                case 0:
                    return GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_FINE_FIELD_SIZE_SCALE_X, ref fieldScaleFine);
                case 1:
                    return GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_FINE_FIELD_SIZE_SCALE_Y, ref fieldScaleFine);
                default:
                    return 0;
            }
        }

        public int GetBleachLSMFieldSize(ref int fieldSize)
        {
            double dVal = 0;
            int ret = GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_FIELD_SIZE, ref dVal);
            fieldSize = (int)dVal;
            return ret;
        }

        public int GetBleachLSMOffsetXY(int i, ref int offset)
        {
            int ret = 0;
            double dVal = 0;
            switch (i)
            {
                case 0:
                    ret = GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_OFFSET_X, ref dVal);
                    offset = (int)dVal;
                    return ret;
                case 1:
                    ret = GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_OFFSET_Y, ref dVal);
                    offset = (int)dVal;
                    return ret;
                default:
                    return 0;
            }
        }

        public int GetBleachLSMPixelXY(int i, ref int pixelSize)
        {
            int ret = 0;
            double dVal = 0;
            switch (i)
            {
                case 0:
                    ret = GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_PIXEL_X, ref dVal);
                    pixelSize = (int)dVal;
                    return ret;
                case 1:
                    ret = GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_PIXEL_Y, ref dVal);
                    pixelSize = (int)dVal;
                    return ret;
                default:
                    return 0;
            }
        }

        public int GetBleachLSMScaleYScan(ref double scaleYScan)
        {
            return GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_Y_AMPLITUDE_SCALER, ref scaleYScan);
        }

        public bool GetIsBleaching()
        {
            return (1 == GetIsBleach()) ? true : false;
        }

        public bool LoadBleachWaveformFile(string bleachfilePathName, int CycleNum)
        {
            return ((1 == SetBleachWaveform(bleachfilePathName, CycleNum)) ? true : false);
        }

        public void ReleaseBleach()
        {
            ReleaseBleachParams();
        }

        public bool StartBleach(string WaveH5PathandName)
        {
            bool ret = true;
            //ensure the buffer is copied after the capture
            ImageDataUpdated = false;

            //prepare for callback:
            _bleachPath = Path.GetDirectoryName(WaveH5PathandName) + "\\"; //other path end with "\\"
            _isPreBleachBuilding = true;
            if (!PreBleachSetup())
            {
                return false;
            }

            //DisablePMTGains();    //let user to decide

            if (1 == BleachWavelengthEnable)
            {
                _preBleachWavelength = (int)MVMManager.Instance["MultiphotonControlViewModel", "Laser1Position", (object)0];
                MVMManager.Instance["MultiphotonControlViewModel", "Laser1Position", (object)0] = BleachWavelength;
                System.Threading.Thread.Sleep(5000);

                //user asked to stop while sleep:
                if (IsBleachStopped)
                {
                    MVMManager.Instance["MultiphotonControlViewModel", "Laser1Position", (object)0] = _preBleachWavelength;
                    System.Threading.Thread.Sleep(5000);
                    if (1 == BleachQuery)
                    {
                        MessageBox.Show("The scanner has completed the bleach. Make your manual light path adjustments now. Then press OK", "Completed Bleach", MessageBoxButton.OK);
                    }
                    IsBleachFinished = true;
                    return false;
                }
            }

            //run bleach:
            ret = (1 == LiveBleach(WaveH5PathandName, _bleachFrames)) ? true : false;

            return ret;
        }

        public void StopBleach()
        {
            StopLiveBleach();
        }

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "CenterLSMScanners")]
        private static extern bool CenterLSMScanners(int selectedCamera);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "GetIsBleach")]
        private static extern int GetIsBleach();

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "InitCallBackBleach")]
        private static extern void InitCallBackBleach(ReportBleachNowFinished reportBleachNowFinished, PreBleachCallback preBleachCallback);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "Bleach", CharSet = CharSet.Unicode)]
        private static extern int LiveBleach(string WaveH5PathandName, int cycleNum);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "ReleaseBleachParams")]
        private static extern bool ReleaseBleachParams();

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "SetBleachPowerPosition")]
        private static extern bool SetBleachPowerPosition(double pos);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "SetBleachWaveform", CharSet = CharSet.Unicode)]
        private static extern int SetBleachWaveform(string bleachfilePathName, int cycleNum);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "StopLiveBleach")]
        private static extern bool StopLiveBleach();

        private void BleachCallback(ref int status)
        {
            string filePathName = _bleachPath + "BleachWaveform.raw";

            //check calibration has been done:
            if (null == BleachCalibrateFineScaleXY)
            { return; }

            if ((0 == BleachCalibrateFieldSize) || (null == BleachCalibratePixelXY))
            { return; }

            if (0 == _bleachPixelArrayList.Count)
            {
                //not long idle time, regular galvo bleach:
                status = (int)PreCaptureStatus.PRECAPTURE_WAVEFORM_LAST_CYCLE;
                return;
            }
            else
            {
                //regenerate waveform:
                if ((BleachFrames == _bleachFrameIndex) && (_bleachPixelArrayIndex == _bleachPixelArrayList.Count))
                {
                    status = (int)PreCaptureStatus.PRECAPTURE_DONE;
                    return;
                }
                //Wait long idle time:
                if ((0 == _bleachFrameIndex) && (0 == _bleachPixelArrayIndex) && (0 == _bleachPixelIndex))
                {
                    //set initial only, no need to wait
                }
                else
                {
                    int timeWait = (int)((WaveformBuilder.MS_TO_S * BleachLongIdle) - new TimeSpan(DateTime.Now.Ticks - BleachLastRunTime.Ticks).TotalMilliseconds);
                    if (timeWait > 0)
                    {
                        System.Threading.Thread.Sleep(timeWait);
                    }
                    //check if bleaching:
                    if (!GetIsBleaching())
                    {
                        status = (int)PreCaptureStatus.PRECAPTURE_DONE;
                        IsBleaching = false;
                        return;
                    }
                }
                BleachLastRunTime = DateTime.Now;

                //build single point waveform:
                Point pt = _bleachPixelArrayList[_bleachPixelArrayIndex].GetPixel(_bleachPixelIndex);
                InitializeWaveformBuilder(BleachParamList[_bleachPixelArrayIndex].ClockRate);
                WaveformBuilder.ResetWaveform();
                WaveformBuilder.BuildTravel(pt, 0, 0, 0);
                WaveformBuilder.BuildSpot(BleachParamList[_bleachPixelArrayIndex], new double[1] { BleachParamList[_bleachPixelArrayIndex].Power });
                if (((BleachFrames - 1) == _bleachFrameIndex) && (_bleachPixelArrayIndex == _bleachPixelArrayList.Count - 1) && (_bleachPixelIndex == _bleachPixelArrayList[_bleachPixelArrayIndex].Size - 1))
                {
                    WaveformBuilder.ReturnHome(true);
                    status = (int)PreCaptureStatus.PRECAPTURE_WAVEFORM_LAST_CYCLE;
                }
                else
                {
                    WaveformBuilder.ReturnHome(false);
                    status = (int)PreCaptureStatus.PRECAPTURE_WAVEFORM_MID_CYCLE;
                }

                //save waveform:
                WaveformBuilder.SaveWaveform(filePathName, false, new bool[(int)SignalType.SIGNALTYPE_LAST] { !IsStimulator, true, true, false });

                while (!WaveformBuilder.CheckSaveState())
                {
                    System.Threading.Thread.Sleep(1);
                }

                if (!WaveformBuilder.GetSaveResult())
                {
                    status = (int)PreCaptureStatus.PRECAPTURE_DONE;
                    return;
                }

                //offset both list & pixel indexes:
                if (_bleachPixelIndex < _bleachPixelArrayList[_bleachPixelArrayIndex].Size - 1)
                {
                    _bleachPixelIndex++;
                }
                else if (_bleachPixelIndex == _bleachPixelArrayList[_bleachPixelArrayIndex].Size - 1)
                {
                    _bleachPixelIndex = 0;
                    _bleachPixelArrayIndex++;
                    //next cycle:
                    if (_bleachPixelArrayList.Count == _bleachPixelArrayIndex)
                    {
                        _bleachPixelArrayIndex = 0;
                        _bleachFrameIndex++;
                    }
                }
            }
        }

        private void BleachNowFinished()
        {
            if (true == IsBleaching)
            {

                if (1 == BleachWavelengthEnable)
                {
                    MVMManager.Instance["MultiphotonControlMVM", "Laser1Position"] = PreBleachWavelength;
                    System.Threading.Thread.Sleep(5000);

                    ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Setting Pre Bleach Wavelength " + PreBleachWavelength.ToString());
                }

                if (1 == BleachQuery)
                {
                    MessageBox.Show("The scanner has completed the bleach. Make your manual light path adjustments now. Then press OK", "Completed Bleach", MessageBoxButton.OK);
                }

                //EnablePMTGains();     //let user to decide
                IsBleaching = false;
                IsBleachFinished = true;
            }
        }

        private bool BuildBleachPixelArray(List<BleachWaveParams> bParams)
        {
            if (bParams.Count <= 0)
            { return false; }

            //check calibration has been done:
            if (null == BleachCalibrateFineScaleXY)
            { return false; }

            if ((0 == BleachCalibrateFieldSize) || (null == BleachCalibratePixelXY))
            { return false; }

            InitializeWaveformBuilder(bParams[0].ClockRate);
            foreach (BleachWaveParams bw in bParams)
            {
                PixelArray pxArray = new PixelArray();
                WaveformBuilder.ResetPixelArray();

                switch (bw.shapeType)
                {
                    case "Rectangle":
                        switch (bw.Fill)
                        {
                            case (int)BleachWaveParams.FillChoice.Raster:
                                WaveformBuilder.PixelRectTopDown(bw);
                                break;
                            default:
                                WaveformBuilder.PixelRectContour(bw);
                                break;
                        }
                        break;
                    case "Polygon":
                        WaveformBuilder.PixelPolygon(bw);
                        break;
                    case "Crosshair":
                        WaveformBuilder.PixelSpot(bw);
                        break;
                    case "Line":
                        WaveformBuilder.PixelLine(bw);
                        break;
                    case "Polyline":
                        WaveformBuilder.PixelPolyTrace(bw, false);
                        break;
                    case "Ellipse":
                        WaveformBuilder.PixelEllipse(bw);
                        break;
                    default:
                        break;
                }

                WaveformBuilder.GetPixelArray().AppendTo(ref pxArray);
                _bleachPixelArrayList.Add(pxArray);
            }

            return true;
        }

        private bool PreBleachSetup()
        {
            string bROIPath = _bleachPath + "BleachROIs.xaml";
            bool ret = true;

            //reset pixel index for callback:
            _bleachFrameIndex = _bleachPixelIndex = _bleachPixelArrayIndex = 0;
            _bleachPixelArrayList.Clear();

            if (0 == BleachLongIdle)
            {
                WaveformBuilder.ResetPixelArray();
            }
            else
            {
                ret = BuildBleachPixelArray(BleachParamList);
            }
            _isPreBleachBuilding = false;
            return ret;
        }

        #endregion Methods
    }
}