namespace CaptureSetupDll.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Reflection;
    using System.Resources;
    using System.Runtime.InteropServices;
    using System.Windows;
    using System.Windows.Data;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Threading;
    using System.Xml;

    using CaptureSetupDll;
    using CaptureSetupDll.Model;

    using Microsoft.Practices.Composite.Events;
    using Microsoft.Practices.Composite.Regions;
    using Microsoft.Practices.Composite.Wpf.Events;
    using Microsoft.Practices.Unity;

    using ThorImageInfastructure;

    using ThorLogging;

    using ThorSharedTypes;

    /// <summary>
    /// ViewModel class for the LiveImage model object
    /// </summary>
    public partial class LiveImageViewModel : ViewModelBase
    {
        #region Fields

        public OverlayManager _overlayManager;

        // wrapped LiveImage object
        private readonly LiveImage _liveImage;

        private static ReportGoToPosition _goToPositionCallBack;

        private CameraConsoleDlg dlg;
        private ICommand _alignmentMinusCommand;
        private ICommand _alignmentPlusCommand;
        private XmlDocument _applicationDoc;
        private ICommand _binXMinusCommand;
        private ICommand _binXPlusCommand;
        private ICommand _binYMinusCommand;
        private ICommand _binYPlusCommand;
        WriteableBitmap _bitmap = null;
        WriteableBitmap _bitmap16 = null;
        private ICommand _blackLevelMinusCommand;
        private ICommand _blackLevelPlusCommand;
        private BackgroundWorker _bw;
        private ICommand _cameraConsoleCommand;
        private ICameraProps _camProps;
        private ICommand _captureNowCommand;
        private ICommand _centerROICommand;
        private IUnityContainer _container;
        private ICommand _decreaseXCommand;
        private ICommand _decreaseYCommand;
        private DispatcherTimer _deviceReadTimer;
        private IEventAggregator _eventAggregator;
        private XmlDocument _experimentDoc;
        private double _fieldSizeCalibration;
        private int _fineMode; //0 = course, 1 = fine
        private ICommand _goPowerCommand;
        private ICommand _goPowerStartCommand;
        private ICommand _goPowerStopCommand;
        private ICommand _goZCommand;
        private ICommand _goZScanStartCommand;
        private ICommand _goZScanStopCommand;
        private XmlDocument _hardwareDoc;
        private ICommand _increaseXCommand;
        private ICommand _increaseYCommand;
        private Visibility[] _isChannelVisible;
        private bool _isLive;
        private bool _liveSnapshotStatus;
        BitmapPalette _palette = null;
        private bool _paletteChanged;
        DispatcherTimer _pmtSafetyTimer;
        private int _powerMode;
        private int _powerStart;
        private int _powerStop;
        private IRegionManager _regionManager;
        private double _sampleOffsetXMM;
        private double _sampleOffsetYMM;
        private ICommand _saveNowCommand;
        private ICommand _selectBackgroundCommand;
        private ICommand _selectFlatFieldCommand;
        private ICommand _setPowerStartCommand;
        private ICommand _setPowerStopCommand;
        private ICommand _setSampleOffsetCommand;
        private ICommand _setZScanStartCommand;
        private ICommand _setZScanStopCommand;
        private ICommand _setZZeroCommand;
        private bool _showingPMTSafetyMessage;
        private double _sliderTickFrequency;
        private double _sliderZMax;
        private double _sliderZMin;
        private ICommand _snapshotCommand;
        private ICommand _startCommand;
        private ICommand _stopCommand;
        private ICommand _stopZCommand;
        private int _subColumns = 1;
        private int _subRows = 1;
        private double _transformX = -1;
        private double _transformY = -1;
        private double _transOffsetXMM;
        private double _transOffsetYMM;
        private string _wavelengthName;
        private double _wellOffsetXMM;
        private double _wellOffsetYMM;
        private double _xStepSize;
        private double _yStepSize;
        private double _zRangeMax;
        private double _zRangeMin;
        private int _zScanNumSteps;
        private double _zScanStart;
        private double _zScanStep;
        private double _zScanStop;
        private double _zScanThickness;
        private double _zStepSize;

        #endregion Fields

        #region Constructors

        /// <summary>
        /// Create a new instance of the LiveImageViewModel class
        /// </summary>
        /// <param name="liveImage">Wrapped liveImage object</param>
        public LiveImageViewModel(IEventAggregator eventAggregator, IRegionManager regionManager, IUnityContainer container, LiveImage liveImage,ConcreteCameraProps camProps)
        {
            this._eventAggregator = eventAggregator;
            this._regionManager = regionManager;
            this._container = container;
            this._camProps = camProps;

            if (liveImage != null)
            {
                this._liveImage = liveImage;
            }
            else
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Information, 1, this.GetType().Name + " LiveImage is null. Creating a new LiveImage object.");
                liveImage = new LiveImage();

                if (liveImage == null)
                {
                    ResourceManager rm = new ResourceManager("LiveImageDataModule.Properties.Resources", Assembly.GetExecutingAssembly());
                    ThorLog.Instance.TraceEvent(TraceEventType.Critical, 1, this.GetType().Name + " " + rm.GetString("CreateLiveImageModelFailed"));
                    throw new NullReferenceException("liveImage");
                }

                this._liveImage = liveImage;
            }

            _fineMode = 0;
            _sliderZMin = this._liveImage.ZMin;
            _sliderZMax = this._liveImage.ZMax;
            _sliderTickFrequency = .01;
            _paletteChanged = true;
            _isLive = false;

            _zScanStart = 0;
            _zScanStop = 0;
            _zScanStep = 1;
            _zScanNumSteps = 1;
            _zRangeMax = 0;
            _zRangeMin = 0;

            //setting the default Stepsize value
            _xStepSize = .100;
            _yStepSize = .100;
            _zStepSize = .0100;

            _fieldSizeCalibration = 1.0;

            _isChannelVisible = new Visibility[LiveImage.MAX_CHANNELS];

            _overlayManager = new OverlayManager();

            this._liveImage.UpdateMenuBarButton += new Action<bool>(LiveImage_UpdateMenuBarButton);

            var startLiveImageEvent = _eventAggregator.GetEvent<StartLiveImageEvent>();
            startLiveImageEvent.Subscribe(LiveImageStart);

            this._liveImage.UpdateImage+= new Action<bool>(LiveImage_Update);

            //create a background worker that will update at 30fps to udpate the bitmap image
            _bw = new BackgroundWorker();
            _bw.WorkerReportsProgress = true;
            _bw.WorkerSupportsCancellation = true;

            _showingPMTSafetyMessage=false;

            _deviceReadTimer = new DispatcherTimer();
            _deviceReadTimer.Interval = TimeSpan.FromMilliseconds(500);

            _pmtSafetyTimer = new DispatcherTimer();
            _pmtSafetyTimer.Interval = TimeSpan.FromMilliseconds(10000);
            EnableDeviceReading = true;

            _goToPositionCallBack = new ReportGoToPosition(GoToPosition);

               // CamProps = new CamPropsType();

            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Initialized");
        }

        #endregion Constructors

        #region Delegates

        //  public CamPropsType CamProps { get; set; }
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void ReportGoToPosition(ref double x, ref double y);

        #endregion Delegates

        #region Events

        //notify listeners (Ex. histogram) that the image has changed
        public event Action<bool> ImageDataChanged;

        #endregion Events

        #region Properties

        public bool EnableDeviceReading
        {
            get;set;
        }

        #endregion Properties

        #region Methods

        public bool AutoExposure(double exposure)
        {
            bool ret = this._liveImage.AutoExposure(exposure);

            OnPropertyChanged("ExposureTimeCam0");

            return ret;
        }

        /// <summary>
        /// Auto focus
        /// </summary>
        public bool AutoFocus()
        {
            if (_experimentDoc == null)
            {
                return false;
            }

            bool ret = this._liveImage.AutoFocus( Convert.ToDouble(_experimentDoc.DocumentElement["Magnification"].GetAttribute("mag")));

            //retrieve the z position after an autofocus
            OnPropertyChanged("ZPosition");

            return ret;
        }

        public WriteableBitmap Bitmap16()
        {
            short[] pd = LiveImage.GetPixelData();

            //verify pixel data is available
            if (pd == null)
            {
                return _bitmap16;
            }

            switch (LiveImage.GetColorChannels())
            {
                case 1:
                    {
                        // Define parameters used to create the BitmapSource.
                        PixelFormat pf = PixelFormats.Gray16;
                        int width = this._liveImage.DataWidth;
                        int height = this._liveImage.DataHeight;
                        int rawStride = (width * pf.BitsPerPixel + 7) / 8;

                        //create a new bitmpap when one does not exist or the size of the image changes
                        if (_bitmap16 == null)
                        {
                            _bitmap16 = new WriteableBitmap(width, height, 96, 96, pf, null);
                        }
                        else
                        {
                            if ((_bitmap16.Width != width) || (_bitmap16.Height != height) || (_bitmap16.Format != pf))
                            {
                                _bitmap16 = new WriteableBitmap(width, height, 96, 96, pf, null);
                            }
                        }

                        int w = _bitmap16.PixelWidth;
                        int h = _bitmap16.PixelHeight;
                        int widthInBytes = w;

                        if (pd.Length == (width * height))
                        {
                            //copy the pixel data into the _bitmap
                            _bitmap16.WritePixels((new Int32Rect(0, 0, w, h)), pd, rawStride, 0);
                        }

                        _paletteChanged = false;
                    }
                    break;
                default:
                    {
                        // Define parameters used to create the BitmapSource.
                        PixelFormat pf = PixelFormats.Rgb48;

                        int width = this._liveImage.DataWidth;
                        int height = this._liveImage.DataHeight;
                        int rawStride = (width * pf.BitsPerPixel + 7) / 8;

                        //create a new bitmpap when one does not exist or the size of the image changes
                        if (_bitmap16 == null)
                        {
                            _bitmap16 = new WriteableBitmap(width, height, 96, 96, pf, null);

                        }
                        else
                        {
                            if ((_bitmap16.Width != width) || (_bitmap16.Height != height) || (_bitmap16.Format != pf))
                            {
                                _bitmap16 = new WriteableBitmap(width, height, 96, 96, pf, null);
                            }
                        }
                        //calculate the raw data buffer offset index for each of the
                        //selected display channels
                        int[] dataBufferOffsetIndex = new int[LiveImage.MAX_CHANNELS];

                        int enabledChannelCount = 0;
                        for (int i = 0; i < LiveImage.MAX_CHANNELS; i++)
                        {
                            //if the channgel is enabled store the index and
                            if (LSMChannelEnable[i])
                            {
                                dataBufferOffsetIndex[enabledChannelCount] = i;
                                enabledChannelCount++;
                            }
                        }

                        int w = _bitmap16.PixelWidth;
                        int h = _bitmap16.PixelHeight;

                        if ((pd.Length / LiveImage.GetColorChannels()) == (width * height))
                        {
                            int buffSize = pd.Length / LiveImage.GetColorChannels();

                            short[] pdData = new short[buffSize * 3];

                            int i = 0;
                            for (int j = 0; j < buffSize; i+=3,j++)
                            {
                                short maxRed = 0;
                                short maxGreen = 0;
                                short maxBlue = 0;
                                pdData[i] = 0;
                                pdData[i + 1] = 0;
                                pdData[i + 2] = 0;

                                for (int k = 0; k < enabledChannelCount; k++)
                                {
                                    short valRaw = ((pd[j + dataBufferOffsetIndex[k] * buffSize]));

                                    switch ((LiveImage.ColorAssignments)GetColorAssignment(dataBufferOffsetIndex[k]))
                                    {
                                        case LiveImage.ColorAssignments.RED:
                                            {
                                                pdData[i] = maxRed = Math.Max(maxRed, valRaw);
                                            }
                                            break;
                                        case LiveImage.ColorAssignments.GREEN:
                                            {
                                                pdData[i + 1] = maxGreen = Math.Max(maxGreen, valRaw);
                                            }
                                            break;
                                        case LiveImage.ColorAssignments.BLUE:
                                            {
                                                pdData[i + 2] = maxBlue = Math.Max(maxBlue, valRaw);
                                            }
                                            break;
                                        case LiveImage.ColorAssignments.CYAN:
                                            {
                                                pdData[i + 1] = maxGreen = Math.Max(maxGreen, valRaw);
                                                pdData[i + 2] = maxBlue = Math.Max(maxBlue, valRaw);
                                            }
                                            break;
                                        case LiveImage.ColorAssignments.MAGENTA:
                                            {
                                                pdData[i] = maxRed = Math.Max(maxRed, valRaw);
                                                pdData[i + 2] = maxBlue = Math.Max(maxBlue, valRaw);
                                            }
                                            break;
                                        case LiveImage.ColorAssignments.YELLOW:
                                            {
                                                pdData[i] = maxRed = Math.Max(maxRed, valRaw);
                                                pdData[i + 1] = maxGreen = Math.Max(maxGreen, valRaw);
                                            }
                                            break;
                                        case LiveImage.ColorAssignments.GRAY:
                                            {
                                                pdData[i] = maxRed = Math.Max(maxRed, valRaw);
                                                pdData[i + 1] = maxGreen = Math.Max(maxGreen, valRaw);
                                                pdData[i + 2] = maxBlue = Math.Max(maxBlue, valRaw);
                                            }
                                            break;
                                    }
                                }
                            }

                            //copy the pixel data into the bitmap
                            _bitmap16.WritePixels(new Int32Rect(0, 0, w, h), pdData, rawStride, 0);
                        }

                        _paletteChanged = false;
                    }
                    break;
            }

            LiveImage.FinishedCopyingPixel();

            ImageDataChanged(true);

            return _bitmap16;
        }

        public void ConnectHandlers()
        {
            _bw.DoWork += new DoWorkEventHandler(bw_DoWork);
            _bw.ProgressChanged += new ProgressChangedEventHandler(bw_ProgressChanged);
            _bw.RunWorkerCompleted += new RunWorkerCompletedEventHandler(bw_RunWorkerCompleted);

            _deviceReadTimer.Tick += new EventHandler(_deviceReadTimer_Tick);
            _deviceReadTimer.Start();
            _pmtSafetyTimer.Tick += new EventHandler(_pmtSafetyTimer_Tick);
            _pmtSafetyTimer.Start();
        }

        public bool CreatePlate()
        {
            SampleType st = (SampleType)SelectedSampleType;

            if (CreatePlateMosaicSample(StartRow, StartColumn, SampleDimensions.Rows(st), SampleDimensions.Columns(st), SampleOffsetXMM, SampleOffsetYMM, SampleDimensions.Rows(st), SampleDimensions.Columns(st), SampleDimensions.WellOffsetMM(st), SampleDimensions.WellOffsetMM(st), SubRows, SubColumns, SubOffsetX, SubOffsetY, TransOffsetXMM, TransOffsetYMM) == false)
            {
                return false;
            }

            return true;
        }

        public bool GetAttribute(XmlNode node, XmlDocument doc, string attrName, ref string attrValue)
        {
            bool ret;

            if (null == node.Attributes.GetNamedItem(attrName))
            {
                ret = false;
            }
            else
            {
                attrValue = node.Attributes[attrName].Value;
                ret = true;
            }

            return ret;
        }

        public int GetColorAssignment(int index)
        {
            return LiveImage.GetColorAssignment(index);
        }

        public short[] GetData_Int16()
        {
            short[] pd = LiveImage.GetPixelData();

            return pd;
        }

        public void LiveImageStart(bool input)
        {
            if (true == input)
            {
                Start();
                _bw.RunWorkerAsync();
            }
            else
            {
                Stop();
                _bw.CancelAsync();
            }
        }

        /// <summary>
        /// load from camera the parameters that do not need to be updated frequently
        /// 
        /// </summary>
        public void LoadCameraParameters()
        {
            // load the min and max values
            OnPropertyChanged("ExposureTimeMin");
            OnPropertyChanged("ExposureTimeMax");
            OnPropertyChanged("GainMin");
            OnPropertyChanged("GainMax");
        }

        public bool MoveToWellSite(int row, int col, int subRow, int subCol, double transOffsetX, double transOffsetY)
        {
            bool ret;

            ret = GoToWellSiteAndOffset(row, col, subRow, subCol, SampleOffsetXMM, SampleOffsetYMM, WellOffsetXMM*XDirection, WellOffsetYMM*YDirection, transOffsetX*XDirection, transOffsetY*YDirection, SubOffsetX*XDirection, SubOffsetY*YDirection, _goToPositionCallBack);

            OnPropertyChanged("XPosition");
            OnPropertyChanged("YPosition");
            return ret;
        }

        public void PersistData()
        {
            // if not camera available in system, do not overwrite active settings
            if (NumAvailableCameras == 0)
            {
                return;
            }

            string templatesFolder = Application.Current.Resources["TemplatesFolder"].ToString();

            string expPath = templatesFolder + "\\Active.xml";

            XmlNodeList ndList = this.ExperimentDoc.SelectNodes("/ThorImageExperiment/ZStage");

            if (ndList.Count > 0)
            {
                double stepSize = this.ZScanStep;

                //ensure the sign of the step is correct
                if (this.ZScanStart > this.ZScanStop)
                {
                    stepSize *= -1;
                }

                ndList[0].Attributes["steps"].Value = this.ZScanNumSteps.ToString();
                ndList[0].Attributes["stepSizeUM"].Value = stepSize.ToString();
                ndList[0].Attributes["startPos"].Value = this.ZScanStart.ToString();
            }

            ndList = this.ExperimentDoc.SelectNodes("/ThorImageExperiment/LSM");

            if (ndList.Count > 0)
            {
                XmlNode node = ndList[0];
                string strNone = "None";
                ndList[0].Attributes["name"].Value = strNone;   // set the name to be None when using Camera Only
                ndList[0].Attributes["pixelX"].Value = this.LSMPixelX.ToString();
                ndList[0].Attributes["pixelY"].Value = this.LSMPixelY.ToString();
                ndList[0].Attributes["inputRange1"].Value = this.InputRangeChannel1.ToString();
                ndList[0].Attributes["inputRange2"].Value = this.InputRangeChannel2.ToString();
                if (node.Attributes.GetNamedItem("inputRange3") != null)
                {
                    node.Attributes["inputRange3"].Value = this.InputRangeChannel3.ToString();
                }
                if (node.Attributes.GetNamedItem("inputRange4") != null)
                {
                    node.Attributes["inputRange4"].Value = this.InputRangeChannel4.ToString();
                }
                ndList[0].Attributes["fieldSize"].Value = this.LSMFieldSize.ToString();

                if (this.LSMChannel == 4)
                {
                    //channel is stored as a zero based index
                    //convert the enabled channels to the index value
                    int chan = (Convert.ToInt32(LSMChannelEnable[0]) | (Convert.ToInt32(LSMChannelEnable[1]) << 1) | (Convert.ToInt32(LSMChannelEnable[2]) << 2) | (Convert.ToInt32(LSMChannelEnable[3]) << 3));
                    ndList[0].Attributes["channel"].Value = chan.ToString();
                }
                else
                {
                    int ch=0;
                    switch(this.LSMChannel)
                    {
                        case 0: ch = 0x1; break;
                        case 1: ch = 0x2; break;
                        case 2: ch = 0x4; break;
                        case 3: ch = 0x8; break;
                    }

                    ndList[0].Attributes["channel"].Value = ch.ToString();
                }

                ndList[0].Attributes["offsetX"].Value = LSMFieldOffsetXActual.ToString();
                ndList[0].Attributes["offsetY"].Value = LSMFieldOffsetYActual.ToString();
                ndList[0].Attributes["averageMode"].Value = this.LSMSignalAverage.ToString();
                ndList[0].Attributes["averageNum"].Value = this.LSMSignalAverageFrames.ToString();
                ndList[0].Attributes["scanMode"].Value = this.LSMScanMode.ToString();
                ndList[0].Attributes["twoWayAlignment"].Value = this.LSMTwoWayAlignment.ToString();
                ndList[0].Attributes["clockSource"].Value = (this.LSMClockSource + 1).ToString();
                int val = Convert.ToInt32(this.LSMExtClockRate * 1000000.0);
                ndList[0].Attributes["extClockRate"].Value = val.ToString();
            }

            ndList = this.ExperimentDoc.SelectNodes("/ThorImageExperiment/PMT");

            if (ndList.Count > 0)
            {
                ndList[0].Attributes["gainA"].Value = this.PMT1Gain.ToString();
                ndList[0].Attributes["enableA"].Value = ((LSMChannel == 0) || (LSMChannel == 4)) ? "1" : "0";
                ndList[0].Attributes["gainB"].Value = this.PMT2Gain.ToString();
                ndList[0].Attributes["enableB"].Value = ((LSMChannel == 1) || (LSMChannel == 4)) ? "1" : "0";
                ndList[0].Attributes["gainC"].Value = this.PMT3Gain.ToString();
                ndList[0].Attributes["enableC"].Value = ((LSMChannel == 2) || (LSMChannel == 4)) ? "1" : "0";
                ndList[0].Attributes["gainD"].Value = this.PMT4Gain.ToString();
                ndList[0].Attributes["enableD"].Value = ((LSMChannel == 3) || (LSMChannel == 4)) ? "1" : "0";
            }

            ndList = this.ExperimentDoc.SelectNodes("/ThorImageExperiment/PowerRegulator");

            if (ndList.Count > 0)
            {
                ndList[0].Attributes["type"].Value = this.PowerMode.ToString();

                //if the power mode is none. Set the experiment power value to the current power position.
                if (this.PowerMode == 0)
                {
                    ndList[0].Attributes["start"].Value = this.PowerPosition.ToString();
                    ndList[0].Attributes["stop"].Value = this.PowerPosition.ToString();
                }
                else
                {
                    ndList[0].Attributes["start"].Value = this.PowerStart.ToString();
                    ndList[0].Attributes["stop"].Value = this.PowerStop.ToString();
                }

                SetAttribute(ndList[0],this.ExperimentDoc,"pockelsBlankPercentage",this.PowerPockelsBlankPercentage.ToString());
            }

            ndList = this.ExperimentDoc.SelectNodes("/ThorImageExperiment/Magnification");

            if (ndList.Count > 0)
            {
                ndList[0].Attributes["mag"].Value = this.TurretMagnification.ToString();
            }

            ndList = this.ExperimentDoc.SelectNodes("/ThorImageExperiment/ImageCorrection");

            if (ndList.Count > 0)
            {
                ndList[0].Attributes["enablePincushion"].Value = this.EnablePincushionCorrection.ToString();
                ndList[0].Attributes["pinCoeff1"].Value = this.Coeff1.ToString();
                ndList[0].Attributes["pinCoeff2"].Value = this.Coeff2.ToString();
                ndList[0].Attributes["pinCoeff3"].Value = this.Coeff3.ToString();
                ndList[0].Attributes["enableBackgroundSubtraction"].Value = this.EnableBackgroundSubtraction.ToString();
                ndList[0].Attributes["pathBackgroundSubtraction"].Value = this.PathBackgroundSubtraction.ToString();
                ndList[0].Attributes["enableFlatField"].Value = this.EnableFlatField.ToString();
                ndList[0].Attributes["pathFlatField"].Value = this.PathFlatField.ToString();
            }

            ndList = this.ExperimentDoc.SelectNodes("/ThorImageExperiment/MCLS");

            if (ndList.Count > 0)
            {
                ndList[0].Attributes["enable1"].Value = this.Laser1Enable.ToString();
                ndList[0].Attributes["power1"].Value = this.Laser1Power.ToString();
                ndList[0].Attributes["enable2"].Value = this.Laser2Enable.ToString();
                ndList[0].Attributes["power2"].Value = this.Laser2Power.ToString();
                ndList[0].Attributes["enable3"].Value = this.Laser3Enable.ToString();
                ndList[0].Attributes["power3"].Value = this.Laser3Power.ToString();
                ndList[0].Attributes["enable4"].Value = this.Laser4Enable.ToString();
                ndList[0].Attributes["power4"].Value = this.Laser4Power.ToString();
            }

            ndList = this.ExperimentDoc.SelectNodes("/ThorImageExperiment/MultiPhotonLaser");

            if (ndList.Count > 0)
            {
                    XmlNode node = ndList[0];

                    if (node.Attributes.GetNamedItem("pos") != null)
                    {
                        node.Attributes["pos"].Value = this.Laser1Position.ToString();
                    }
            }

            ndList = this.ExperimentDoc.SelectNodes("/ThorImageExperiment/PinholeWheel");

            if (ndList.Count > 0)
            {
                ndList[0].Attributes["position"].Value = this.PinholePosition.ToString();
            }

            ndList = this.ExperimentDoc.SelectNodes("/ThorImageExperiment/Camera");
            XmlNodeList ndListHW = this.HardwareDoc.SelectNodes("/HardwareSettings/ImageDetectors/Camera");
            if (ndList.Count > 0)
            {
                string strCameraName = "None";
                SetAttribute(ndList[0], this.ExperimentDoc, "name", strCameraName);
                SetAttribute(ndList[0], this.ExperimentDoc, "left", this.Left.ToString());
                SetAttribute(ndList[0], this.ExperimentDoc, "right", this.Right.ToString());
                SetAttribute(ndList[0], this.ExperimentDoc, "top", this.Top.ToString());
                SetAttribute(ndList[0], this.ExperimentDoc, "bottom", this.Bottom.ToString());
                SetAttribute(ndList[0], this.ExperimentDoc, "width", this.Width.ToString());
                SetAttribute(ndList[0], this.ExperimentDoc, "height", this.Height.ToString());
                SetAttribute(ndList[0], this.ExperimentDoc, "binningX", this.BinX.ToString());
                SetAttribute(ndList[0], this.ExperimentDoc, "binningY", this.BinY.ToString());
                SetAttribute(ndList[0], this.ExperimentDoc, "gain", this.Gain.ToString());
                SetAttribute(ndList[0], this.ExperimentDoc, "lightmode", this.LightMode.ToString());
                SetAttribute(ndList[0], this.ExperimentDoc, "blacklevel", this.OpticalBlackLevel.ToString());
                SetAttribute(ndList[0], this.ExperimentDoc, "tdiWidth", this.TDIWidthMM.ToString());
                SetAttribute(ndList[0], this.ExperimentDoc, "tdiHeight", this.TDIHeightMM.ToString());
                SetAttribute(ndList[0], this.ExperimentDoc, "readoutSpeedIndex", this.ReadOutSpeedIndex.ToString());
                SetAttribute(ndList[0], this.ExperimentDoc, "bitsPerPixel", this.BitsPerPixel.ToString());
                SetAttribute(ndList[0], this.ExperimentDoc, "readoutTapIndex", this.ReadOutTapIndex.ToString());

            }
            //replace name with the active one in hardware settings:
            if (ndListHW.Count > 0)
            {
                string str = string.Empty;
                for (int i = 0; i < ndListHW.Count; i++)
                {
                    GetAttribute(ndListHW[i], this.HardwareDoc, "active", ref str);
                    if (1 == Convert.ToInt32(str))
                    {
                        GetAttribute(ndListHW[i], this.HardwareDoc, "cameraName", ref str);
                        if (str != string.Empty)
                        {
                            SetAttribute(ndList[0], this.ExperimentDoc, "name", str);
                            break;
                        }
                    }
                }
            }

            ndList = this.ExperimentDoc.SelectNodes("/ThorImageExperiment/Sample");

            if (ndList.Count > 0)
            {
                XmlNode node = ndList[0];

                if (node.Attributes.GetNamedItem("type") != null)
                {
                    int val = (int)this.SelectedSampleType;
                    node.Attributes["type"].Value = val.ToString();
                }

                if (node.Attributes.GetNamedItem("offsetXMM")!=null)
                {
                    node.Attributes["offsetXMM"].Value = this.SampleOffsetXMM.ToString();
                }

                if (node.Attributes.GetNamedItem("offsetYMM")!=null)
                {
                    node.Attributes["offsetYMM"].Value = this.SampleOffsetYMM.ToString();
                }
            }

            ndList = this.ExperimentDoc.SelectNodes("/ThorImageExperiment/Sample/Wells");

            if (ndList.Count > 0)
            {
                XmlNode node = ndList[0];

                if (node.Attributes.GetNamedItem("startRow") != null)
                {
                    //one based index for the persistence
                    node.Attributes["startRow"].Value = this.StartRow.ToString();
                }

                if (node.Attributes.GetNamedItem("startColumn") != null)
                {
                    //one based index for the persistence
                    node.Attributes["startColumn"].Value = this.StartColumn.ToString();
                }

                if (node.Attributes.GetNamedItem("rows") != null)
                {
                    node.Attributes["rows"].Value = this.SelectedWellRowCount.ToString();
                }

                if (node.Attributes.GetNamedItem("columns") != null)
                {
                    node.Attributes["columns"].Value = this.SelectedWellColumnCount.ToString();
                }

            }
            ndList = this.ExperimentDoc.SelectNodes("/ThorImageExperiment/Sample/Wells/SubImages");

            if (ndList.Count > 0)
            {
                XmlNode node = ndList[0];

                if (node.Attributes.GetNamedItem("subRows") != null)
                {
                    node.Attributes["subRows"].Value = this.SubRows.ToString();
                }

                if (node.Attributes.GetNamedItem("subColumns") != null)
                {
                    node.Attributes["subColumns"].Value = this.SubColumns.ToString();
                }

                if (node.Attributes.GetNamedItem("transOffsetXMM") != null)
                {
                    node.Attributes["transOffsetXMM"].Value = this.TransOffsetXMM.ToString();
                }

                if (node.Attributes.GetNamedItem("transOffsetYMM") != null)
                {
                    node.Attributes["transOffsetYMM"].Value = this.TransOffsetYMM.ToString();
                }

                if (node.Attributes.GetNamedItem("subOffsetXMM") != null)
                {
                    node.Attributes["subOffsetXMM"].Value = Convert.ToString(MMPerPixel * LSMPixelX * (1 + SubSpacingXPercent/100.0));
                }

                if (node.Attributes.GetNamedItem("subOffsetYMM") != null)
                {
                    node.Attributes["subOffsetYMM"].Value = Convert.ToString(MMPerPixel * LSMPixelY * (1 + SubSpacingYPercent/100.0));
                }
            }

            ndList = this.ExperimentDoc.SelectNodes("/ThorImageExperiment/Wavelengths");

            if (ndList.Count > 0)
            {
                ndList[0].RemoveAll();
            }

            ndList = this.ExperimentDoc.SelectNodes("/ThorImageExperiment/Wavelengths");

            if ((this.HardwareDoc != null) && (ndList.Count > 0))
            {
                switch (this.LSMChannel)
                {
                    case 0:
                        {
                            XmlElement newElement;

                            XmlNodeList waveList = this.HardwareDoc.SelectNodes("/HardwareSettings/Wavelength");

                            newElement = CreateTag(waveList[0].Attributes["name"].Value);

                            ndList[0].AppendChild(newElement);

                        }
                        break;
                    case 1:
                        {
                            XmlElement newElement;

                            XmlNodeList waveList = this.HardwareDoc.SelectNodes("/HardwareSettings/Wavelength");

                            newElement = CreateTag(waveList[1].Attributes["name"].Value);

                            ndList[0].AppendChild(newElement);
                        }
                        break;
                    case 2:
                        {
                            XmlElement newElement;

                            XmlNodeList waveList = this.HardwareDoc.SelectNodes("/HardwareSettings/Wavelength");

                            newElement = CreateTag(waveList[2].Attributes["name"].Value);

                            ndList[0].AppendChild(newElement);
                        }
                        break;
                    case 3:
                        {
                            XmlElement newElement;

                            XmlNodeList waveList = this.HardwareDoc.SelectNodes("/HardwareSettings/Wavelength");

                            newElement = CreateTag(waveList[3].Attributes["name"].Value);

                            ndList[0].AppendChild(newElement);
                        }
                        break;
                    case 4:
                        {
                            XmlElement newElement;

                            XmlNodeList waveList = this.HardwareDoc.SelectNodes("/HardwareSettings/Wavelength");

                            for (int i = 0; i < waveList.Count; i++)
                            {
                                if (LSMChannelEnable[i])
                                {
                                    newElement = CreateTag(waveList[i].Attributes["name"].Value);

                                    ndList[0].AppendChild(newElement);
                                }
                            }
                        }
                        break;
                }
            }

            //save the experiment information when the setup panel is unloaded
            this.ExperimentDoc.Save(expPath);
        }

        public void ReleaseHandlers()
        {
            //this._liveImage.UpdateMenuBarButton -= new Action<bool>(LiveImage_UpdateMenuBarButton);
            this._liveImage.UpdateImage -= new Action<bool>(LiveImage_Update);
            _bw.DoWork -= new DoWorkEventHandler(bw_DoWork);
            _bw.ProgressChanged -= new ProgressChangedEventHandler(bw_ProgressChanged);
            _bw.RunWorkerCompleted -= new RunWorkerCompletedEventHandler(bw_RunWorkerCompleted);
            _deviceReadTimer.Stop();
            _deviceReadTimer.Tick -= new EventHandler(_deviceReadTimer_Tick);
            _pmtSafetyTimer.Stop();
            _pmtSafetyTimer.Tick -= new EventHandler(_pmtSafetyTimer_Tick);
        }

        public void SaveImage(String filename, int filterIndex)
        {
            if (_bitmap == null)
            {
                return;
            }

            FileStream stream = new FileStream(filename, FileMode.Create);

            switch (filterIndex)
            {
                case 1:
                    {
                        //8 bit tiff image save
                        TiffBitmapEncoder encoder = new TiffBitmapEncoder();
                        encoder.Frames.Add(BitmapFrame.Create(_bitmap));
                        encoder.Save(stream);
                    }
                    break;
                case 2:
                    {
                        //16 bit tiff image save
                        Bitmap16();
                        TiffBitmapEncoder encoder = new TiffBitmapEncoder();
                        BitmapMetadata bmd = new BitmapMetadata("tiff");

                        bmd.SetQuery("/ifd/{uint=270}",CreateOMEMetadata(Convert.ToInt32(_bitmap16.Width),Convert.ToInt32(_bitmap16.Height)));
                        encoder.Frames.Add(BitmapFrame.Create(_bitmap16,null,bmd,null));
                        encoder.Save(stream);
                    }
                    break;
                case 3:
                    {
                        //8 bit jpeg image save
                        JpegBitmapEncoder jpgEncoder = new JpegBitmapEncoder();
                        jpgEncoder.Frames.Add(BitmapFrame.Create(_bitmap));
                        jpgEncoder.Save(stream);
                    }
                    break;
            }

            stream.Close();
        }

        //assign the attribute value to the input node and document
        //if the attribute does not exist add it to the document
        public void SetAttribute(XmlNode node, XmlDocument doc, string attrName, string attValue)
        {
            XmlNode tempNode = node.Attributes[attrName];

                if (null == tempNode)
                {
                    XmlAttribute attr = doc.CreateAttribute(attrName);

                    attr.Value = attValue;

                     node.Attributes.Append(attr);
                }
                else
                {
                    node.Attributes[attrName].Value = attValue;
                }
        }

        public void SetCameraFullFrame()
        {
            this.Left = 0;
            this.Top = 0;
            Width = this.CameraWidth;
            Height = this.CameraHeight;
            this.Right = Width;
            this.Bottom = Height;
        }

        public void SetColorAssignment(int index, int value)
        {
            LiveImage.SetColorAssignment(index, value);
        }

        public void UpdateUIWithCameraParameters()
        {
            OnPropertyChanged("ExposureTimeMin");
            OnPropertyChanged("ExposureTimeMax");
            OnPropertyChanged("GainMin");
            OnPropertyChanged("GainMax");
            OnPropertyChanged("ReadOutTapIndexMax");
            OnPropertyChanged("ReadOutTapIndexMin");
            OnPropertyChanged("ReadOutSpeedIndexMax");
            OnPropertyChanged("ReadOutSpeedIndexMin");
            OnPropertyChanged("Left");
            OnPropertyChanged("Right");
            OnPropertyChanged("Top");
            OnPropertyChanged("Bottom");
            OnPropertyChanged("Width");
            OnPropertyChanged("Height");
            OnPropertyChanged("BinX");
            OnPropertyChanged("BinY");
            OnPropertyChanged("Gain");
            OnPropertyChanged("LightMode");
            OnPropertyChanged("OpticalBlackLevel");
            OnPropertyChanged("TDIHeightMM");
            OnPropertyChanged("TDIWidthMM");
            OnPropertyChanged("ReadOutSpeedEntries");
            OnPropertyChanged("ReadOutSpeedIndex");
            OnPropertyChanged("ExposureTimeCam0");
            OnPropertyChanged("CoolingMode");
            OnPropertyChanged("ReadOutTapEntries");
            OnPropertyChanged("TapBalance");
            OnPropertyChanged("ReadOutTapIndex");
        }

        [DllImport(".\\Modules_Native\\Sample.dll", EntryPoint = "CreatePlateMosaicSample")]
        private static extern bool CreatePlateMosaicSample(int startRow, int startColumn, int totalRows, int totalCols, double sampleOffsetX, double sampleOffsetY, int wellRows, int wellCols, double wellOffsetX, double wellOffsetY, int subRows, int subCols, double subOffsetX, double subOffsetY, double transOffsetX, double transOffsetY);

        private static void GoToPosition(ref double x, ref double y)
        {
            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, String.Format("Go to x:{0} y:{1}", x, y));

            SetStagePosition(x, y);
        }

        [DllImport(".\\Modules_Native\\Sample.dll", EntryPoint = "GoToWellSiteAndOffset")]
        private static extern bool GoToWellSiteAndOffset(int row, int col, int subRow, int subCol, double sampleOffsetX, double sampleOffsetY, double wellOffsetX, double wellOffsetY, double transOffsetX, double transOffsetY, double subOffsetX, double subOffsetY, ReportGoToPosition reportGoToPosition);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetStagePosition")]
        private static extern bool SetStagePosition(double x, double y);

        private void AlignmentMinus()
        {
            this._liveImage.LSMTwoWayAlignment -= 1;
            OnPropertyChanged("LSMTwoWayAlignment");
        }

        private void AlignmentPlus()
        {
            this._liveImage.LSMTwoWayAlignment += 1;
            OnPropertyChanged("LSMTwoWayAlignment");
        }

        private void BinXMinus()
        {
            this._camProps.BinX -= 1;
            OnPropertyChanged("BinX");
        }

        private void BinXPlus()
        {
            this._camProps.BinX += 1;
            OnPropertyChanged("BinX");
        }

        private void BinYMinus()
        {
            this._camProps.BinY -= 1;
            OnPropertyChanged("BinY");
        }

        private void BinYPlus()
        {
            this._camProps.BinY += 1;
            OnPropertyChanged("BinY");
        }

        private void BlackLevelMinus()
        {
            this._camProps.OpticalBlackLevel -= 1;
            OnPropertyChanged("OpticalBlackLevel");
        }

        private void BlackLevelPlus()
        {
            this._camProps.OpticalBlackLevel += 1;
            OnPropertyChanged("OpticalBlackLevel");
        }

        private BitmapPalette BuildPalette(double whitePoint, double blackPoint)
        {
            List<Color> colors = new List<Color>();
            for (int i = 0; i < 256; i++)
            {
                double a = (255.0) / (whitePoint - blackPoint);
                double b = 0 - (a * blackPoint);

                double dvalR = (a * i * (this._liveImage.WavelengthColor.R) / 255.0) + b;
                dvalR = Math.Max(dvalR, 0);
                dvalR = Math.Min(dvalR, 255);

                double dvalG = (a * i * (this._liveImage.WavelengthColor.G) / 255.0) + b;
                dvalG = Math.Max(dvalG, 0);
                dvalG = Math.Min(dvalG, 255);

                double dvalB = (a * i * (this._liveImage.WavelengthColor.B) / 255.0) + b;
                dvalB = Math.Max(dvalB, 0);
                dvalB = Math.Min(dvalB, 255);

                Color color;
                color = Color.FromRgb((byte)dvalR, (byte)dvalG, (byte)dvalB);

                colors.Add(color);
            }

            return new BitmapPalette(colors);
        }

        private void bw_DoWork(object sender, DoWorkEventArgs e)
        {
            BackgroundWorker worker = sender as BackgroundWorker;

            while(true)
            {
                if ((worker.CancellationPending == true))
                {
                    e.Cancel = true;
                    break;
                }
                else
                {
                    // Perform a time consuming operation and report progress.
                    System.Threading.Thread.Sleep(30);
                    worker.ReportProgress(0);
                }
            };
        }

        private void bw_ProgressChanged(object sender, ProgressChangedEventArgs e)
        {
            if (LiveImage.IsPixelDataReady())
            {
                OnPropertyChanged("Bitmap");
                OnPropertyChanged("FramesPerSecond");

                _overlayManager.UpdateStats(this.LiveImage.DataWidth, this.LiveImage.DataHeight, LiveImage.GetColorChannels());
            }
        }

        private void bw_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            if ((e.Cancelled == true))
            {
            }

            else if (!(e.Error == null))
            {
            }

            else
            {
            }
        }

        private double CalculateFieldOffsetXDisplay(int pixelX, int pixelY, int fieldSize)
        {
            if ((pixelX != 0) || (pixelY != 0))
            {
                double xyRatio = (double)pixelY / (double)pixelX;
                return fieldSize / (Math.Sqrt(1 + (xyRatio * xyRatio)) * 2.0);
            }
            return 0;
        }

        private double CalculateFieldOffsetYDisplay(int pixelX, int pixelY, int fieldSize)
        {
            if ((pixelX != 0) || (pixelY != 0))
            {
                double xyRatio = (double)pixelY / (double)pixelX;
                return fieldSize / (Math.Sqrt(1 + 1 / (xyRatio * xyRatio)) * 2.0);
            }
            return 0;
        }

        private void CameraConsole()
        {
            dlg = new CameraConsoleDlg();

            dlg.Show();
        }

        private void CaptureNow()
        {
            PersistData();

            Command command = new Command();
            command.Message = "RunSample LS";
            command.CommandGUID = new Guid("30db4357-7508-46c9-84eb-3ca0900aa4c5");
            command.Payload = new List<string>();
            command.Payload.Add("RunImmediately");

            _eventAggregator.GetEvent<CommandShowDialogEvent>().Publish(command);
        }

        private void CenterROI()
        {
            this.LSMFieldOffsetXActual = 0;
            this.LSMFieldOffsetYActual = 0;
        }

        private string CreateOMEMetadata(int width, int height)
        {
            string tagData = "<?xml version=\"1.0\"?><OME xmlns=\"http://www.openmicroscopy.org/Schemas/OME/2008-02\" xmlns:CA=\"http://www.openmicroscopy.org/Schemas/CA/2008-02\" xmlns:STD=\"http://www.openmicroscopy.org/Schemas/STD/2008-02\" xmlns:Bin=\"http://www.openmicroscopy.org/Schemas/BinaryFile/2008-02\" xmlns:SPW=\"http://www.openmicroscopy.org/Schemas/SPW/2008-02\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://www.openmicroscopy.org/Schemas/OME/2008-02 http://www.openmicroscopy.org/Schemas/OME/2008-02/ome.xsd\">";

            tagData += "<Image><Pixels";
            tagData += " DimensionOrder=";
            tagData += "\"XYCZT\"";
            tagData += "ID=\"Pixels:0\"";
            tagData += " PhysicalSizeX=\"1.0\"";
            tagData += " PhysicalSizeY=\"1.0\"";
            tagData += " PhysicalSizeZ=\"1.0\"";
            tagData += " SizeC=";
            tagData += "\"1\"";
            tagData += " SizeT=";
            tagData += "\"1\"";
            tagData += " SizeX=";
            tagData += string.Format("\"{0}\"", width);
            tagData += " SizeY=";
            tagData += string.Format("\"{0}\"", height);
            tagData += " SizeZ=";
            tagData += "\"1\"";
            tagData += " TimeIncrement=";
            tagData += "\"1\"";
            tagData += " Type=";
            tagData += "\"uint16\"";
            tagData += ">";
            tagData += "<Channel ID=";
            tagData += "\"Channel:0:0\"";
            tagData += "SamplesPerPixel=\"1\"";
            tagData += "/>";
            tagData += "<BinData BigEndian=\"false\" Length = \"0\" xmlns=\"http://www.openmicroscopy.org/Schemas/BinaryFile/2010-06\"/>";
            tagData += "<TiffData FirstC=\"0\" FirstT=\"0\" FirstZ=\"0\" IFD=\"0\" PlaneCount=\"1\">";
            tagData += "</TiffData>";
            tagData += "</Pixels>";
            tagData += "</Image>";
            tagData += "</OME>";

            return tagData;
        }

        private XmlElement CreateTag(string name)
        {
            //create a new XML tag for the wavelength settings
            XmlElement newElement = this.ExperimentDoc.CreateElement("Wavelength");

            XmlAttribute nameAttribute = this.ExperimentDoc.CreateAttribute("name");
            XmlAttribute expAttribute = this.ExperimentDoc.CreateAttribute("exposureTimeMS");

            nameAttribute.Value = name;
            // Note: _exposureTimeCam0 should be used instead of ExposureTimeCam0, to avoid communicating with
            // camera on software exit
            expAttribute.Value = this._exposureTimeCam0.ToString(); // Only 1 camera is supported for now
                                                                    // TO DO: change this code when multiple cameras are supported.

            newElement.Attributes.Append(nameAttribute);
            newElement.Attributes.Append(expAttribute);

            return newElement;
        }

        /// <summary>
        /// 
        /// </summary>
        private void DecreaseX()
        {
            this._liveImage.XPosition = this._liveImage.XPosition - this.XStepSize;
            OnPropertyChanged("XPosition");
        }

        /// <summary>
        /// 
        /// </summary>
        private void DecreaseY()
        {
            this._liveImage.YPosition = this._liveImage.YPosition - this.YStepSize;
            OnPropertyChanged("YPosition");
        }

        private double GetPixelsPerInch()
        {
            double fieldSizeCalibration = 100.0;
            double pixelsPerInch = 96;

            //the hardware doc is valid and the camera type is LSM
            const int LSM_CAMERA_TYPE = 1;
            if ((_hardwareDoc != null) && ((int)CamType == LSM_CAMERA_TYPE))
            {

                XmlNodeList ndList = _hardwareDoc.SelectNodes("/HardwareSettings/LSM");

                if (ndList.Count > 0)
                {
                    XmlNode node = ndList[0].Attributes.GetNamedItem("fieldSizeCalibration");

                    if (node != null)
                    {
                        fieldSizeCalibration = Convert.ToDouble(node.Value);
                    }
                }

                double magnification = 10.0;

                ndList = _hardwareDoc.SelectNodes("/HardwareSettings/Objectives/Objective");

                if (ndList.Count >= TurretPosition)
                {
                    XmlNode node = ndList[TurretPosition].Attributes.GetNamedItem("mag");

                    if (node != null)
                    {
                        magnification = Convert.ToDouble(node.Value);
                    }
                }

                double umPerPixel = (LSMFieldSize * fieldSizeCalibration) / (LSMPixelX * magnification);

                const double UM_PER_INCH_CONVERSION = 25399.99999997256800000002962656;

                pixelsPerInch = UM_PER_INCH_CONVERSION / umPerPixel;
            }
            return pixelsPerInch;
        }

        private void GoPower()
        {
            OnPropertyChanged("PowerPosition");
        }

        private void GoPowerStart()
        {
            this._liveImage.PowerPosition = _powerStart;
            OnPropertyChanged("PowerPosition");
        }

        private void GoPowerStop()
        {
            this._liveImage.PowerPosition = _powerStop;
            OnPropertyChanged("PowerPosition");
        }

        /// <summary>
        /// Go to the new z position
        /// </summary>
        private void GoZ()
        {
            OnPropertyChanged("ZPosition");
        }

        private void GoZScanStart()
        {
            this._liveImage.ZPosition = _zScanStart;
            OnPropertyChanged("ZPosition");
        }

        private void GoZScanStop()
        {
            this._liveImage.ZPosition = _zScanStop;
            OnPropertyChanged("ZPosition");
        }

        /// <summary>
        /// 
        /// </summary>
        private void IncreaseX()
        {
            this._liveImage.XPosition = this._liveImage.XPosition + this.XStepSize;
            OnPropertyChanged("XPosition");
        }

        /// <summary>
        /// 
        /// </summary>
        private void IncreaseY()
        {
            this._liveImage.YPosition = this._liveImage.YPosition + this.YStepSize;
            OnPropertyChanged("YPosition");
        }

        void LiveImage_Update(bool val)
        {
        }

        void LiveImage_UpdateMenuBarButton(bool status)
        {
            OnPropertyChanged("LiveStartButtonStatus");
            OnPropertyChanged("LiveStopButtonStatus");

            bool btnStatus = status;
            Command command = new Command();
            command.CommandGUID = new Guid("1FC17C8C-960D-4f1d-902A-48C5A2032AAC");

            if (btnStatus)
            {
                command.Message = "Enable Button";
            }
            else
            {
                command.Message = "Disable Button";
            }
            //command published to change the status of the Run a Plate menu button in the Menu Control
            _eventAggregator.GetEvent<CommandShowDialogEvent>().Publish(command);
        }

        private void SaveNow()
        {
            string str = PathSaveSnapshot + "\\" + SnapshotImagePrefix + string.Format("_{0:yyyy-MM-dd_hh-mm-ss}", DateTime.Now) + ".tif";
            SaveImage(str ,2);
        }

        private void SelectBackground()
        {
            // Configure open file dialog box
            Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();
            dlg.FileName = "Tif"; // Default file name
            dlg.DefaultExt = ".tif"; // Default file extension
            dlg.Filter = "Tif files (.tif)|*.tif"; // Filter files by extension

            // Show open file dialog box
            Nullable<bool> result = dlg.ShowDialog();

            // Process open file dialog box results
            if (result == true)
            {
                // Open document
               this.PathBackgroundSubtraction = dlg.FileName;
            }
        }

        private void SelectFlatField()
        {
            // Configure open file dialog box
            Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();
            dlg.FileName = "Tif"; // Default file name
            dlg.DefaultExt = ".tif"; // Default file extension
            dlg.Filter = "Tif files (.tif)|*.tif"; // Filter files by extension

            // Show open file dialog box
            Nullable<bool> result = dlg.ShowDialog();

            // Process open file dialog box results
            if (result == true)
            {
                // Open document
                this.PathFlatField = dlg.FileName;
            }
        }

        private void SetPowerStart()
        {
            _powerStart = this._liveImage.PowerPosition;
            OnPropertyChanged("PowerStart");
        }

        private void SetPowerStop()
        {
            _powerStop = this._liveImage.PowerPosition;
            OnPropertyChanged("PowerStop");
        }

        private void SetSampleOffset()
        {
            this.SampleOffsetXMM = this.XPosition;
            this.SampleOffsetYMM = this.YPosition;

            CreatePlate();

            this.SelectedSubRow = 0;
            this.SelectedWellColumn = 0;
            this.SelectedWellRow = 0;
            this.SelectedWellColumn = 0;

            MoveToWellSite(0, 0, 0, 0, 0, 0);
        }

        private void SetZScanStart()
        {
            _zScanStart = this._liveImage.ZPosition;
            _zRangeMin = this._liveImage.ZPosition;
            OnPropertyChanged("ZRangeMin");
            OnPropertyChanged("ZScanStart");
            OnPropertyChanged("ZScanNumSteps");
            OnPropertyChanged("ZScanThickness");
        }

        private void SetZScanStop()
        {
            _zScanStop = this._liveImage.ZPosition;
            _zRangeMax = this._liveImage.ZPosition;
            OnPropertyChanged("ZRangeMax");
            OnPropertyChanged("ZScanStop");
            OnPropertyChanged("ZScanNumSteps");
            OnPropertyChanged("ZScanThickness");
        }

        private void SetZZero()
        {
            this._liveImage.SetZZero();

            OnPropertyChanged("ZPosition");
        }

        private void Snapshot()
        {
            _liveSnapshotStatus = false;

            this._liveImage.Snapshot();

            if (LiveImage.IsPixelDataReady())
            {
                OnPropertyChanged("Bitmap");
            }
        }

        /// <summary>
        /// Start live image capture
        /// </summary>
        private void Start()
        {
            if (false == _isLive)
            {
                if (false == _bw.IsBusy)
                {
                    this._liveImage.Start();
                    _bw.RunWorkerAsync();
                }
                else
                {
                    return;
                }
            }
            else
            {
                  this._liveImage.Stop();
                  _bw.CancelAsync();
                  OnPropertyChanged("FramesPerSecond");
            }

            _isLive = !_isLive;

            OnPropertyChanged("ImagePathPlay");
            OnPropertyChanged("IsLive");
        }

        /// <summary>
        /// Stop live image capture
        /// </summary>
        private void Stop()
        {
            this._liveImage.Stop();
            _bw.CancelAsync();
            _isLive = false;
            OnPropertyChanged("FramesPerSecond");
            OnPropertyChanged("ImagePathPlay");
            OnPropertyChanged("IsLive");
        }

        private void StopZ()
        {
            this._liveImage.StopZ();
        }

        private int TranslateInputRangeValue(int direction, int val)
        {
            int newVal = 1;
            //if going out
            if (direction == 0)
            {
                if (Model.LiveImage.DigitizerBoardNames.ATS9440 == DigitizerBoardName)
                {
                    switch (val)
                    {
                        case 1: newVal = 5; break;
                        case 2: newVal = 6; break;
                        case 3: newVal = 7; break;
                        case 4: newVal = 10; break;
                        case 5: newVal = 11; break;
                        case 6: newVal = 12; break;
                    }
                }
                else
                {
                    newVal = val;
                }

            }
            else
            {
                if (Model.LiveImage.DigitizerBoardNames.ATS9440 == DigitizerBoardName)
                {
                    switch (val)
                    {
                        case 5: newVal = 1; break;
                        case 6: newVal = 2; break;
                        case 7: newVal = 3; break;
                        case 10: newVal = 4; break;
                        case 11: newVal = 5; break;
                        case 12: newVal = 6; break;
                        default: newVal = 1; break;
                    }
                }
                else
                {
                    newVal = val;
                }

            }
            return newVal;
        }

        void _deviceReadTimer_Tick(object sender, EventArgs e)
        {
            if (EnableDeviceReading)
            {
                OnPropertyChanged("ZPosition");
                OnPropertyChanged("PowerPosition");
                OnPropertyChanged("XPosition");
                OnPropertyChanged("YPosition");
            }
        }

        void _pmtSafetyTimer_Tick(object sender, EventArgs e)
        {
            OnPropertyChanged("PMTSafetyStatus");

            //The safety flag for the hardware has tripped
            //Present a message to the user and disable the PMTs
            if ((false == PMTSafetyStatus) && (_showingPMTSafetyMessage == false))
            {
                _showingPMTSafetyMessage = true;
                MessageBox.Show("The PMT(s) safety has tripped. Too prevent damage to the PMT(s) please reduce the light being sent to the detectors");
                _showingPMTSafetyMessage = false;
                this.PMT1GainEnable = 0;
                this.PMT2GainEnable = 0;
                this.PMT3GainEnable = 0;
                this.PMT4GainEnable = 0;
            }
        }

        #endregion Methods
    }
}