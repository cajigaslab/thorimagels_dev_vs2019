namespace CaptureSetupDll.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.Collections.Specialized;
    using System.ComponentModel;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Security.Cryptography;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Input;
    using System.Xml;

    using CaptureSetupDll.View;

    using GeometryUtilities;

    using HDF5CS;

    using OverlayManager;

    using ThorLogging;

    using ThorSharedTypes;

    public partial class CaptureSetupViewModel : ViewModelBase
    {
        #region Fields

        private int _bleachEpoches = 1;
        private double _bleachLSMUMPerPixel = 0;
        private ICommand _BleachNowCommand;
        private bool _bleachNowEnable = false;
        private string _bleachNowKey;
        private string _bleachNowModifier;

        //private bool _SpotScanIsEnabled;
        private Byte[] _bROIByteArray;
        private ICommand _centerScannersCommand;
        private string _clockRateByPixel = string.Empty;
        private ICommand _displayBleachROICommand;
        private EditBleachWaveform _editBleach = null;
        private ICommand _extractBleachROICommand;
        private ICommand _genWaveformCommand;
        private ICommand _imagingCenterScannersCommand;
        private bool _isROIExtracted = false;
        private int _pixelDensity = 1;
        private double _pixelLongIdleTime = 0;
        private bool _pixelUnitMode = false;
        private double _postCycleIdleTime = 0;
        private double _postEpochIdleTime = 0;
        private double _preCycleIdleTime = 0;
        private double _preEpochIdleTime = 0;

        #endregion Fields

        #region Events

        //notify 2D view that the bleach is finished
        public event Action<bool> BleachFinished;

        public event Action BleachWaveformGeneratedEvent;

        //public event Action BleachROICheck;
        //notify listeners (Ex. ImageView) that RecROI need to be updated
        public event Action<string> ROIUpdateRequested;

        #endregion Events

        #region Properties

        public double BleachCalibrateAreaAngle
        {
            get
            {
                return _captureSetup.BleachCalibrateAreaAngle;
            }
        }

        public int BleachCalibrateFieldSize
        {
            get
            {
                return _captureSetup.BleachCalibrateFieldSize;
            }
        }

        public double[] BleachCalibrateFineOffsetXY
        {
            get
            {
                return _captureSetup.BleachCalibrateFineOffsetXY;
            }
        }

        public double[] BleachCalibrateFineScaleXY
        {
            get
            {
                return _captureSetup.BleachCalibrateFineScaleXY;
            }
        }

        public int[] BleachCalibrateFlipHV
        {
            get
            {
                return _captureSetup.BleachCalibrateFlipHV;
            }
        }

        public int[] BleachCalibrateOffsetXY
        {
            get
            {
                return _captureSetup.BleachCalibrateOffsetXY;
            }
        }

        public int[] BleachCalibratePixelXY
        {
            get
            {
                return _captureSetup.BleachCalibratePixelXY;
            }
        }

        public double[] BleachCalibratePockelsVoltageMax0
        {
            get
            {
                return _captureSetup.BleachCalibratePockelsVoltageMax0;
            }
        }

        public double[] BleachCalibratePockelsVoltageMin0
        {
            get
            {
                return _captureSetup.BleachCalibratePockelsVoltageMin0;
            }
        }

        public int BleachCalibrateScaleYScan
        {
            get
            {
                return _captureSetup.BleachCalibrateScaleYScan;
            }
        }

        public int BleachEpoches
        {
            get
            {
                return _bleachEpoches;
            }
            set
            {
                _bleachEpoches = value;
                OnPropertyChanged("BleachEpoches");
            }
        }

        public string BleachExpandHeader
        {
            get;
            set;
        }

        public int[] BleachFlipScanHV
        {
            get
            {
                int[] val = new int[2];
                if ((0 == _captureSetup.GetBleachFlipScanHV(0, ref val[0])) || (0 == _captureSetup.GetBleachFlipScanHV(1, ref val[1])))
                {
                    return null;
                }
                return val;
            }
        }

        public int BleachFrames
        {
            get
            {
                return this._captureSetup.BleachFrames;
            }
            set
            {
                this._captureSetup.BleachFrames = value;
                OnPropertyChanged("BleachFrames");
            }
        }

        public int BleachInternalClockRate
        {
            get { return this._captureSetup.BleachInternalClockRate; }
        }

        public double BleachLSMAreaAngle
        {
            get
            {
                double val = 0;
                _captureSetup.GetBleachLSMAreaAngle(ref val);
                return val;
            }
        }

        public double[] BleachLSMFieldScaleXYFine
        {
            get
            {
                double[] val = new double[2];
                if ((0 == _captureSetup.GetBleachLSMFieldScaleXYFine(0, ref val[0])) || (0 == _captureSetup.GetBleachLSMFieldScaleXYFine(1, ref val[1])))
                {
                    return null;
                }
                return val;
            }
        }

        public int BleachLSMFieldSize
        {
            get
            {
                int val = 0;
                _captureSetup.GetBleachLSMFieldSize(ref val);
                return val;
            }
        }

        public int[] BleachLSMOffsetXY
        {
            get
            {
                int[] val = new int[2];
                if ((0 == _captureSetup.GetBleachLSMOffsetXY(0, ref val[0])) || (0 == _captureSetup.GetBleachLSMOffsetXY(1, ref val[1])))
                {
                    return null;
                }
                return val;
            }
        }

        public double[] BleachLSMOffsetXYFine
        {
            get
            {
                return _captureSetup.BleachLSMOffsetXYFine;
            }
            set
            {
                _captureSetup.BleachLSMOffsetXYFine = value;
            }
        }

        public int[] BleachLSMPixelXY
        {
            get
            {
                if (IsStimulator)
                {
                    return _captureSetup.SLMPixelXY;
                }
                else
                {
                    int[] val = new int[2];
                    if ((0 == _captureSetup.GetBleachLSMPixelXY(0, ref val[0])) || (0 == _captureSetup.GetBleachLSMPixelXY(1, ref val[1])))
                    {
                        return null;
                    }
                    return val;
                }
            }
        }

        public double BleachLSMScaleYScan
        {
            get
            {
                double val = 0;
                _captureSetup.GetBleachLSMScaleYScan(ref val);
                Decimal dec = new Decimal(val / 100.0);
                return Convert.ToDouble(Decimal.Round(dec, 2).ToString());
            }
        }

        public double BleachLSMUMPerPixel
        {
            get { return _bleachLSMUMPerPixel; }
            set { _bleachLSMUMPerPixel = value; }
        }

        public ICommand BleachNowCommand
        {
            get
            {
                if (this._BleachNowCommand == null)
                    this._BleachNowCommand = new RelayCommand(() => StartBleach());

                return this._BleachNowCommand;
            }
        }

        public bool BleachNowEnable
        {
            get
            {
                if (!IsProgressWindowOff)
                    return false;

                return _bleachNowEnable;
            }
            set
            {
                _bleachNowEnable = value;
                OnPropertyChanged("BleachNowEnable");
            }
        }

        public string BleachNowKey
        {
            get { return _bleachNowKey; }
            set { _bleachNowKey = value; OnPropertyChanged("BleachNowKey"); }
        }

        public string BleachNowModifier
        {
            get { return _bleachNowModifier; }
            set { _bleachNowModifier = value; OnPropertyChanged("BleachNowModifier"); }
        }

        public List<BleachWaveParams> BleachParamList
        {
            get
            { return _captureSetup.BleachParamList; }
            set
            {
                _captureSetup.BleachParamList = value;
            }
        }

        public double BleachPixelSizeUMRatio
        {
            get
            {
                return (0 < _bleachLSMUMPerPixel) ? ((double)MVMManager.Instance["AreaControlViewModel", "PixelSizeUM", (double)1.0] / _bleachLSMUMPerPixel) : 1.0;
            }
        }

        public double BleachPower
        {
            get
            {
                return this._captureSetup.BleachPower;
            }
            set
            {
                this._captureSetup.BleachPower = value;
                OnPropertyChanged("BleachPower");
            }
        }

        public int BleachPowerEnable
        {
            get
            {
                return this._captureSetup.BleachPowerEnable;
            }
            set
            {
                this._captureSetup.BleachPowerEnable = value;
                OnPropertyChanged("BleachPowerEnable");
            }
        }

        public int BleachQuery
        {
            get
            {
                return this._captureSetup.BleachQuery;
            }
            set
            {
                this._captureSetup.BleachQuery = value;
                OnPropertyChanged("BleachQuery");
            }
        }

        public string BleachROIPath
        {
            get
            {
                return ResourceManagerCS.GetCaptureTemplatePathString();
            }
        }

        public int BleachWavelength
        {
            get
            {
                return this._captureSetup.BleachWavelength;
            }
            set
            {
                this._captureSetup.BleachWavelength = value;
                OnPropertyChanged("BleachWavelength");
            }
        }

        public int BleachWavelengthEnable
        {
            get
            {
                return this._captureSetup.BleachWavelengthEnable;
            }
            set
            {
                this._captureSetup.BleachWavelengthEnable = value;
                OnPropertyChanged("BleachWavelengthEnable");
            }
        }

        public Byte[] BROIByteArray
        {
            get
            {
                return _bROIByteArray;
            }
            set
            {
                _bROIByteArray = value;
            }
        }

        public ICommand CenterScannersCommand
        {
            get
            {
                if (this._centerScannersCommand == null)
                    this._centerScannersCommand = new RelayCommandWithParam((x) => CenterScanners(x));

                return this._centerScannersCommand;
            }
        }

        public string ClockRateByPixel
        {
            get
            { return _clockRateByPixel; }
            set
            {
                _clockRateByPixel = value;
                OnPropertyChanged("ClockRateByPixel");
            }
        }

        public ICommand DisplayBleachROICommand
        {
            get
            {
                if (this._displayBleachROICommand == null)
                    this._displayBleachROICommand = new RelayCommand(() => DisplayBleachROI());

                return this._displayBleachROICommand;
            }
        }

        public bool EditBleachResult
        {
            get;
            set;
        }

        public ICommand ExtractBleachROICommand
        {
            get
            {
                if (this._extractBleachROICommand == null)
                    this._extractBleachROICommand = new RelayCommand(() => ExtractBleachROI());

                return this._extractBleachROICommand;
            }
        }

        public ICommand GenWaveformCommand
        {
            get
            {
                if (this._genWaveformCommand == null)
                    this._genWaveformCommand = new RelayCommand(() => GenerateWaveform());

                return this._genWaveformCommand;
            }
        }

        public ICommand ImagingCenterScannersCommand
        {
            get
            {
                if (this._imagingCenterScannersCommand == null)
                    this._imagingCenterScannersCommand = new RelayCommandWithParam((x) => ImagingCenterScanners(x));

                return this._imagingCenterScannersCommand;
            }
        }

        public bool IsBleachFinished
        {
            get
            {
                return this._captureSetup.IsBleachFinished;
            }
            set
            {
                this._captureSetup.IsBleachFinished = value;
                OnPropertyChanged("IsBleachFinished");
            }
        }

        public bool IsBleaching
        {
            get
            {
                return this._captureSetup.IsBleaching;
            }
            set
            {
                this._captureSetup.IsBleaching = value;
                OnPropertyChanged("IsBleaching");
            }
        }

        public bool IsBleachStopped
        {
            get
            {
                return this._captureSetup.IsBleachStopped;
            }
            set
            {
                this._captureSetup.IsBleachStopped = value;
                OnPropertyChanged("IsBleachStopped");
            }
        }

        public bool IsROIExtracted
        {
            get
            {
                return _isROIExtracted;
            }
            set
            {
                _isROIExtracted = value;
                OnPropertyChanged("IsROIExtracted");
            }
        }

        public int PixelDensity
        {
            get
            { return _pixelDensity; }
            set
            {
                double rate = (null == this.BleachLSMPixelXY) ? 1.0 / value : (double)this.BleachLSMPixelXY[0] / value;
                if ((rate >= 20) && (rate <= 1540))
                {
                    for (int i = 0; i < BleachParamList.Count; i++)
                    {
                        BleachParamList[i].ClockRate = (int)rate * (int)WaveformBuilder.MS_TO_S;
                        BleachParamList[i].DeltaX_Px = value;
                    }
                    _clockRateByPixel = rate.ToString(string.Format("#.##")) + "KHz";
                    _pixelDensity = value;
                    OnPropertyChanged("PixelDensity");
                    OnPropertyChanged("ClockRateByPixel");
                    OnPropertyChanged("BleachParamList");
                }
            }
        }

        public double PixelLongIdleTime
        {
            get
            {
                return _pixelLongIdleTime;
            }
            set
            {
                Decimal val = Decimal.Round((Decimal)value, 3);
                _pixelLongIdleTime = (double)val;
                OnPropertyChanged("PixelLongIdleTime");
            }
        }

        public bool PixelUnitMode
        {
            get
            {
                return _pixelUnitMode;
            }
            set
            {
                _pixelUnitMode = value;
                OnPropertyChanged("PixelUnitMode");
            }
        }

        public double PostCycleIdleTime
        {
            get
            {
                return _postCycleIdleTime;
            }
            set
            {
                Decimal val = Decimal.Round((Decimal)value, 3);
                _postCycleIdleTime = (double)val;
                OnPropertyChanged("PostCycleIdleTime");
            }
        }

        public double PostEpochIdleTime
        {
            get
            {
                return _postEpochIdleTime;
            }
            set
            {
                Decimal val = Decimal.Round((Decimal)value, 3);
                _postEpochIdleTime = (double)val;
                OnPropertyChanged("PostEpochIdleTime");
            }
        }

        public double PreCycleIdleTime
        {
            get
            {
                return _preCycleIdleTime;
            }
            set
            {
                Decimal val = Decimal.Round((Decimal)value, 3);
                _preCycleIdleTime = (double)val;
                OnPropertyChanged("PreCycleIdleTime");
            }
        }

        public double PreEpochIdleTime
        {
            get
            {
                return _preEpochIdleTime;
            }
            set
            {
                Decimal val = Decimal.Round((Decimal)value, 3);
                _preEpochIdleTime = (double)val;
                OnPropertyChanged("PreEpochIdleTime");
            }
        }

        #endregion Properties

        #region Methods

        public void ClearBleachFiles()
        {
            try
            {
                string tempFolder = ResourceManagerCS.GetCaptureTemplatePathString();
                string pathActiveBleachingROIsXAML = tempFolder + "BleachROIs.xaml";
                string pathActiveBleachingWaveFormH5 = tempFolder + "BleachWaveform.raw";

                DeleteFile(pathActiveBleachingROIsXAML);

                DeleteFile(pathActiveBleachingWaveFormH5);
            }
            catch (Exception e)
            {
                ThorLog.Instance.TraceEvent(System.Diagnostics.TraceEventType.Verbose, 1, e.ToString());
            }
        }

        public int GetLSMBleacherFieldSizeCalibration(ref double calibration)
        {
            return this._captureSetup.GetBleacherFieldSizeCalibration(ref calibration);
        }

        public void ReleaseBleachWaveform()
        {
            this._captureSetup.ReleaseBleach();
            BROIByteArray = null;
        }

        public bool SaveBleachWaveParams(string filePathName)
        {
            bool retVal = true;
            //Keep decimal dot in xml:
            bool tempSwitchCI = false;
            System.Globalization.CultureInfo originalCultureInfo = (System.Globalization.CultureInfo)System.Threading.Thread.CurrentThread.CurrentCulture.Clone();

            if (0 == originalCultureInfo.NumberFormat.NumberDecimalSeparator.CompareTo(","))
            {
                tempSwitchCI = true;
                originalCultureInfo.NumberFormat.NumberDecimalSeparator = ".";
                System.Threading.Thread.CurrentThread.CurrentCulture = originalCultureInfo;
            }

            try
            {
                if (0 >= BleachParamList.Count)
                    return false;

                //save attached properties:
                XmlDocument doc = new XmlDocument();
                XmlTextReader reader = new XmlTextReader(filePathName);
                doc.Load(reader);
                XmlNodeList xnodes = doc.DocumentElement.ChildNodes.Item(0).ChildNodes.Item(0).ChildNodes;

                //persist ROI file,
                //screen for Mode(0: STATS_ONLY) bleach ROIs:
                int idx = 0; string str = string.Empty;
                for (int i = 0; i < xnodes.Count; i++)
                {
                    int val;
                    Int32.TryParse(xnodes[i].ChildNodes[0].ChildNodes[0].ChildNodes[(int)ThorSharedTypes.Tag.MODE].ChildNodes[0].Value, out val);
                    BitVector32 bVec = new BitVector32(val);
                    if ((int)ThorSharedTypes.Mode.STATSONLY == bVec[BitVector32.CreateSection(255)])
                    {
                        BleachClass.SetBleachAttribute(xnodes[i], doc, 0, BleachParamList[idx].PreIdleTime.ToString());
                        BleachClass.SetBleachAttribute(xnodes[i], doc, 1, BleachParamList[idx].DwellTime.ToString());
                        BleachClass.SetBleachAttribute(xnodes[i], doc, 2, BleachParamList[idx].PostIdleTime.ToString());
                        BleachClass.SetBleachAttribute(xnodes[i], doc, 3, BleachParamList[idx].Iterations.ToString());
                        BleachClass.SetBleachAttribute(xnodes[i], doc, 4, BleachParamList[idx].Fill.ToString());
                        str = (BleachParamList[idx].PixelMode) ? "1" : "0";
                        BleachClass.SetBleachAttribute(xnodes[i], doc, 5, str);
                        BleachClass.SetBleachAttribute(xnodes[i], doc, 6, BleachParamList[idx].Power.ToString());           //single pockel bleach, may extend
                        BleachClass.SetBleachAttribute(xnodes[i], doc, 7, BleachParamList[idx].ClockRate.ToString());
                        BleachClass.SetBleachAttribute(xnodes[i], doc, 8, BleachParamList[idx].LongIdleTime.ToString());
                        BleachClass.SetBleachAttribute(xnodes[i], doc, 9, BleachParamList[idx].PrePatIdleTime.ToString());
                        BleachClass.SetBleachAttribute(xnodes[i], doc, 10, BleachParamList[idx].PostPatIdleTime.ToString());
                        BleachClass.SetBleachAttribute(xnodes[i], doc, 11, BleachParamList[idx].PreCycleIdleMS.ToString());
                        BleachClass.SetBleachAttribute(xnodes[i], doc, 12, BleachParamList[idx].PostCycleIdleMS.ToString());
                        BleachClass.SetBleachAttribute(xnodes[i], doc, 13, BleachParamList[idx].PreEpochIdleMS.ToString());
                        BleachClass.SetBleachAttribute(xnodes[i], doc, 14, BleachParamList[idx].PostEpochIdleMS.ToString());
                        BleachClass.SetBleachAttribute(xnodes[i], doc, 15, BleachParamList[idx].EpochCount.ToString());
                        idx++;
                    }
                }

                reader.Close();
                doc.Save(filePathName);
            }
            catch (Exception ex)
            {
                retVal = false;
                ThorLogging.ThorLog.Instance.TraceEvent(System.Diagnostics.TraceEventType.Verbose, 1, "SaveBleachWaveParams error: " + ex.Message);
            }
            //give back CultureInfo:
            if (tempSwitchCI)
            {
                originalCultureInfo.NumberFormat.NumberDecimalSeparator = ",";
                System.Threading.Thread.CurrentThread.CurrentCulture = originalCultureInfo;
            }
            return retVal;
        }

        public void StopBleach()
        {
            this._captureSetup.StopBleach();
        }

        private void bleachWorker_DoWork(object sender, DoWorkEventArgs e)
        {
            BackgroundWorker worker = sender as BackgroundWorker;
            if (ICamera.ScanMode.FORWARD_SCAN >= (ICamera.ScanMode)MVMManager.Instance["ScanControlViewModel", "LSMScanMode", (object)0])
                MVMManager.Instance["ScanControlViewModel", "LastLSMScanMode"] = (int)MVMManager.Instance["ScanControlViewModel", "LSMScanMode", (object)0];

            if (true == worker.CancellationPending)
            {
                e.Cancel = true;
                return;
            }
            if (false == IsBleaching)
            {
                IsBleaching = true;
                IsBleachFinished = false;
                IsBleachStopped = false;
                string PathandName = BleachROIPath + "BleachWaveform.raw";

                //stop the background hardware updates
                BWHardware = false;

                //execute bleach:
                this._captureSetup.StartBleach(PathandName);

                //restart the background hardware updates
                BWHardware = true;
            }
        }

        private void CenterScanners(object type)
        {
            switch ((SelectedHardware)type)
            {
                case SelectedHardware.SELECTED_CAMERA1:
                    MVMManager.Instance["AreaControlViewModel", "RSInitMode"] = (int)0;
                    _captureSetup.CenterScanners((int)SelectedHardware.SELECTED_CAMERA1, false);
                    break;
                case SelectedHardware.SELECTED_BLEACHINGSCANNER:
                    _captureSetup.CenterScanners((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, false);
                    break;
            }
        }

        private bool CompareROIFile()
        {
            bool ret = true;
            string ROIpath = BleachROIPath + "BLeachROIs.xaml";

            //false if doing long term idle:
            XmlDocument doc = new XmlDocument();
            XmlTextReader reader = new XmlTextReader(ROIpath);
            doc.Load(reader);
            XmlNodeList xnodes = doc.DocumentElement.ChildNodes.Item(0).ChildNodes.Item(0).ChildNodes;
            string str = string.Empty;
            if (BleachClass.GetBleachAttribute(xnodes[0], doc, 8, ref str))
            {
                if (Convert.ToDouble(str, CultureInfo.CurrentCulture) > 0)
                {
                    return false;
                }
            }

            if (!File.Exists(ROIpath))
            {
                return false;
            }
            Byte[] newByteArray;
            using (var md5 = MD5.Create())
            {
                using (var stream = File.OpenRead(ROIpath))
                {
                    newByteArray = md5.ComputeHash(stream);
                }
            }
            if (null == BROIByteArray)
            {
                BROIByteArray = newByteArray;
                return false;
            }

            if (BROIByteArray.Length != newByteArray.Length)
            {
                ret = false;
            }
            else
            {
                for (int i = 0; i < BROIByteArray.Length; i++)
                {
                    if (BROIByteArray[i] != newByteArray[i])
                    {
                        ret = false;
                        break;
                    }
                }
            }
            BROIByteArray = newByteArray;
            return ret;
        }

        private void DisplayBleachROI()
        {
            if (!File.Exists(BleachROIPath + "BleachROIs.xaml"))
            {
                return;
            }
            bool requestAccept = true;
            if (!IsROIExtracted)
            {
                if (MessageBoxResult.No == MessageBox.Show("Do you want to replace current ROIs?", "Update ROIs?", MessageBoxButton.YesNo, MessageBoxImage.Question, MessageBoxResult.Yes, MessageBoxOptions.DefaultDesktopOnly))
                {
                    requestAccept = false;
                }
            }
            if (requestAccept)
            {
                ROIUpdateRequested(BleachROIPath + "BleachROIs.xaml");
            }
        }

        private void ExtractBleachROI()
        {
            OverlayManagerClass.Instance.BleachSaveROIs(this.BleachROIPath);
            ROIUpdateRequested(BleachROIPath + "BleachROIs.xaml");
        }

        private void GenerateWaveform()
        {
            ExtractBleachROI();
            double[] FieldScaleFine = { 0, 0 };
            if ((Bitmap == null) || (0 == _captureSetup.GetBleachLSMFieldScaleXYFine(0, ref FieldScaleFine[0])) || (0 == _captureSetup.GetBleachLSMFieldScaleXYFine(1, ref FieldScaleFine[1])))
            {
                return;
            }

            if (((_editBleach == null) || (_editBleach.IsLoaded == false)))
            {
                _editBleach = new EditBleachWaveform();
                _editBleach.DataContext = this;
                _editBleach.vm = this;
                _editBleach.ROIPath = this.BleachROIPath;
                _editBleach.FieldScaleFine = FieldScaleFine;

                _editBleach.RoiPathandName = BleachROIPath + "BleachROIs.xaml";
                _editBleach.RoiCapsule = OverlayManagerClass.LoadXamlROIs(_editBleach.RoiPathandName);
                if (null == _editBleach.RoiCapsule)
                    return;
                if (_editBleach.RoiCapsule.ROIs.Length > 0)
                {
                    BleachParamList = WaveformBuilder.LoadBleachWaveParams(_editBleach.RoiPathandName, _editBleach.RoiCapsule, (double)MVMManager.Instance["AreaControlViewModel", "PixelSizeUM", (double)1.0], BleachPixelSizeUMRatio, (int)MVMManager.Instance["CameraControlViewModel", "BinX", (object)1], (int)MVMManager.Instance["CameraControlViewModel", "BinY", (object)1]);
                    if (0 == BleachParamList.Count)
                    { return; }
                    if (!ValidateParams())
                    {
                        MessageBox.Show("A Galvo Galvo area calibration is required before proceeding.");
                        return;
                    }
                    InitializeWaveformBuilder(BleachParamList[0].ClockRate);

                    //update Delta_Px after initialization for display:
                    foreach (BleachWaveParams bParam in BleachParamList)
                    {
                        bParam.DeltaX_Px = WaveformBuilder.DeltaX_Px;
                    }
                }

                XmlDocument appSettings = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

                XmlNodeList ndList = appSettings.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/EditBleachWindow");

                if (ndList.Count > 0)
                {
                    string str = string.Empty;
                    double val = 0;
                    XmlManager.GetAttribute(ndList[0], appSettings, "left", ref str);
                    if (double.TryParse(str, out val))
                    {
                        _editBleach.Left = (int)val;
                    }
                    else
                    {
                        _editBleach.Left = 0;
                    }
                    XmlManager.GetAttribute(ndList[0], appSettings, "top", ref str);
                    if (double.TryParse(str, out val))
                    {
                        _editBleach.Top = (int)val;
                    }
                    else
                    {
                        _editBleach.Top = 0;
                    }
                    XmlManager.GetAttribute(ndList[0], appSettings, "width", ref str);
                    if (double.TryParse(str, out val))
                    {
                        _editBleach.Width = (int)val;
                    }
                    else
                    {
                        _editBleach.Width = 400;
                    }
                    XmlManager.GetAttribute(ndList[0], appSettings, "height", ref str);
                    if (double.TryParse(str, out val))
                    {
                        _editBleach.Height = (int)val;
                    }
                    else
                    {
                        _editBleach.Height = 400;
                    }
                }

                _editBleach.Closed += _editBleach_Closed;
                _editBleach.Show();
            }
            if (null != BleachWaveformGeneratedEvent)
            {
                BleachWaveformGeneratedEvent();
            }
        }

        private void ImagingCenterScanners(object type)
        {
            switch ((SelectedHardware)type)
            {
                case SelectedHardware.SELECTED_BLEACHINGSCANNER:
                    if (null != BleachCalibrateFineOffsetXY)
                    {
                        BleachLSMOffsetXYFine = BleachCalibrateFineOffsetXY;
                        _captureSetup.CenterScanners((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, true);
                    }
                    break;
            }
        }

        private bool PreBleachCheck()
        {
            //determine if long time idle:
            _captureSetup.BleachLongIdle = 0;

            string PathandName = BleachROIPath + "BleachROIs.xaml";
            if (File.Exists(PathandName))
            {
                string str = string.Empty;
                XmlDocument doc = new XmlDocument();
                XmlTextReader reader = new XmlTextReader(PathandName);
                doc.Load(reader);
                XmlNodeList xnodes = doc.DocumentElement.ChildNodes.Item(0).ChildNodes.Item(0).ChildNodes;
                for (int i = 0; i < xnodes.Count; i++)
                {
                    int val;
                    Int32.TryParse(xnodes[i].ChildNodes[0].ChildNodes[0].ChildNodes[(int)ThorSharedTypes.Tag.MODE].ChildNodes[0].Value, out val);
                    BitVector32 bVec = new BitVector32(val);
                    if ((int)ThorSharedTypes.Mode.STATSONLY == bVec[BitVector32.CreateSection(255)])
                    {
                        BleachClass.GetBleachAttribute(xnodes[i], doc, 8, ref str);
                        _captureSetup.BleachLongIdle = Convert.ToDouble(str, CultureInfo.InvariantCulture);
                        break;
                    }
                }
                if (_captureSetup.BleachLongIdle > 0)
                {
                    ROICapsule roiCapsule = OverlayManagerClass.LoadXamlROIs(PathandName);
                    if ((roiCapsule == null) || (roiCapsule.ROIs.Length == 0))
                    {
                        return false;
                    }
                    BleachParamList = WaveformBuilder.LoadBleachWaveParams(PathandName, roiCapsule, (double)MVMManager.Instance["AreaControlViewModel", "PixelSizeUM", (double)1.0], (double)MVMManager.Instance["AreaControlViewModel", "PixelSizeUM", (double)1.0] / this.BleachLSMUMPerPixel, (int)MVMManager.Instance["CameraControlViewModel", "BinX", (object)1], (int)MVMManager.Instance["CameraControlViewModel", "BinY", (object)1]);
                }
            }
            else
            {
                return false;
            }

            //waveform file required if not long time idle:
            PathandName = BleachROIPath + "BleachWaveform.raw";
            if ((_captureSetup.BleachLongIdle == 0) && (!File.Exists(PathandName)))
            {
                return false;
            }

            return true;
        }

        private bool StartBleach()
        {
            //Return when no bleach scanner is selected:
            if ((int)ICamera.LSMType.LSMTYPE_LAST == ResourceManagerCS.GetBleacherType())
            {
                System.Windows.MessageBox.Show("Bleach Scanner has not been selected!");
                return false;
            }
            if (BleachFrames <= 0)
            {
                return false;
            }
            ////Display ROI to bleach:
            //DisplayBleachROI();

            //stop Live Capture:
            if ((int)ICamera.LSMType.GALVO_GALVO == ResourceManagerCS.GetLSMType())  //GALVO_GALVO: 1, GALVO_RESONANCE: 0
            {
                StopLiveCapture();
            }
            if (false == IsBleaching)
            {
                //move flag settings to background bleachworker:
                //IsBleaching = true;
                //IsBleachStopped = false;

                if (1 == BleachQuery)
                {
                    //power control will position while the use is being queried
                    MessageBox.Show("The scanner is now ready to bleach. Make your manual light path adjustments now. Then press OK", "Ready to Bleach", MessageBoxButton.OK);
                }

                //check bleach files:
                if ((!PreBleachCheck()) || (null != _spinnerWindow))
                {
                    return false;
                }
                //check if previous task is done:
                if (true == _bleachWorker.IsBusy)
                {
                    _bleachWorker.CancelAsync();
                    return false;
                }
                //update global params:
                PersistGlobalExperimentXML(GlobalExpAttribute.GALVO_BLEACH);

                _bleachWorker.RunWorkerAsync();

                CreateProgressWindow();
            }
            return true;
        }

        private bool ValidateParams()
        {
            bool ret = false;
            if (_captureSetup.BleachCalibrateFieldSize >= 0
                && _captureSetup.BleachCalibrateFineScaleXY != null
                && _captureSetup.BleachCalibratePixelXY != null
                && _captureSetup.BleachCalibrateOffsetXY != null
                && _captureSetup.BleachCalibrateFineOffsetXY != null)
            {
                ret = true;
            }

            return ret;
        }

        void _editBleach_Closed(object sender, EventArgs e)
        {
            if (true == EditBleachResult)
            {
                BleachNowEnable = true;

                //persist params in all modalities:
                PersistGlobalExperimentXML(GlobalExpAttribute.GALVO_BLEACH);
            }

            ApplicationDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
            XmlNode node = ApplicationDoc.SelectSingleNode("/ApplicationSettings/DisplayOptions/CaptureSetup/EditBleachWindow");

            if (node != null)
            {
                string str = string.Empty;

                //// Code below this point is not being reached ////
                XmlManager.SetAttribute(node, ApplicationDoc, "left", ((int)Math.Round(_editBleach.Left)).ToString());
                XmlManager.SetAttribute(node, ApplicationDoc, "top", ((int)Math.Round(_editBleach.Top)).ToString());
                XmlManager.SetAttribute(node, ApplicationDoc, "width", ((int)Math.Round(_editBleach.Width)).ToString());
                XmlManager.SetAttribute(node, ApplicationDoc, "height", ((int)Math.Round(_editBleach.Height)).ToString());

                MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS);
            }

            _editBleach = null;
        }

        #endregion Methods
    }
}