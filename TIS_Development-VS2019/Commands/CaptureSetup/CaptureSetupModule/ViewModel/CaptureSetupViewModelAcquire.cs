namespace CaptureSetupDll.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows.Input;
    using System.Xml;

    using CaptureSetupDll.Model;

    using ThorLogging;

    using ThorSharedTypes;

    public partial class CaptureSetupViewModel : ViewModelBase
    {
        #region Fields

        BackgroundWorker liveImageWorker;
        BackgroundWorker snapshotWorker;
        BackgroundWorker stopPreviewWorker;
        private ICommand _captureNowCommand;
        private bool _enableCapture = true;
        private bool _ffButtonStatus = true;
        private bool _isLive = false;
        private bool _isLiveSnapshot = false;
        private bool _liveSnapshotStopped = true;
        private int _preFFAreaMode;
        private int _preFFAverageMode;
        private double _preFFDwellTime;
        private int _preFFPixelX;
        private int _preFFPixelY;
        private int _preFFScanMode;
        private int _preLsmInterleaveScanMode;
        private ICommand _remoteCaptureCommand;
        private ICommand _saveNowCommand;
        private string _settingsTemplateName;
        private string _settingsTemplatesPath;
        private ICommand _snapshotCommand;
        private string _snapshotKey;
        private string _snapshotModifier;
        ICommand _snapshotSettingsCommand;
        bool _stackPreviewStopped = false;
        private ICommand _startCommand;
        private ICommand _startFastFocusCommand;
        bool _startingContinuousZStackPreview = false;
        private string _startKey;
        private string _startModifier;
        private ICommand _stopCommand;

        #endregion Fields

        #region Properties

        //Used to call ApplyOrRevertFastFocusParams from other MVMs
        public bool ApplyOrRevertFastFocusParamsMVMCall
        {
            set
            {
                ApplyOrRevertFastFocusParams(value);
            }
        }

        public string CameraSaveIcon
        {
            get
            {
                if (this._captureSetup.SaveSnapshot)
                {
                    return @"/CaptureSetupModule;component/Icons/CameraSave.png";
                }
                else
                {
                    return @"/CaptureSetupModule;component/Icons/Camera2.png";
                }
            }
        }

        public ICommand CaptureNowCommand
        {
            get
            {
                if (this._captureNowCommand == null)
                    this._captureNowCommand = new RelayCommand(() => CaptureNow());

                return this._captureNowCommand;
            }
        }

        public bool CaptureStatus
        {
            get
            {
                return this._captureSetup.CaptureStatus;
            }
        }

        public bool EnableCapture
        {
            get
            {
                return _enableCapture;
            }
            set
            {
                _enableCapture = value;
                OnPropertyChanged("EnableCapture");
            }
        }

        /// <summary>
        /// This is the enable flag for the fast focus button
        /// It checks the state of the live button and if fast focus
        /// is being used
        /// </summary>
        public bool FastFocusButtonEnable
        {
            get
            {
                if (false == _ffButtonStatus)
                {
                    //active fast focus
                    return true;
                }
                else
                {
                    //inactive fast focus
                    if (_isLive)
                    {
                        //live is active doe not enable
                        return false;
                    }
                    else
                    {
                        return true;
                    }
                }
            }
        }

        /// <summary>
        /// This reports the status of the fast focus
        /// so that other controls can enable/disable appropriately
        /// </summary>
        public bool FastFocusButtonStatus
        {
            get
            {
                return _ffButtonStatus;
            }
            set
            {
                MVMManager.Instance["PowerControlViewModel", "PockelsCalibrateAgainEnable"] = _ffButtonStatus = value;
                OnPropertyChanged("FastFocusButtonStatus");
                OnPropertyChanged("FastFocusButtonEnable");
            }
        }

        public double FramesPerSecond
        {
            get
            {
                return this._captureSetup.FramesPerSecond;
            }
        }

        public string ImagePathFastFocus
        {
            get
            {
                if ((_isLive) && (false == FastFocusButtonStatus))
                {
                    return @"/CaptureSetupModule;component/Icons/Stop.png";
                }
                else
                {
                    return @"/CaptureSetupModule;component/Icons/FastFocus.png";
                }
            }
        }

        public string ImagePathPlay
        {
            get
            {
                if ((_isLive) && (true == FastFocusButtonStatus))
                {
                    return @"/CaptureSetupModule;component/Icons/Stop.png";
                }
                else
                {
                    return @"/CaptureSetupModule;component/Icons/Play.png";
                }
            }
        }

        public bool IsLive
        {
            get
            {
                return _isLive;
            }
        }

        public bool IsLiveSnapshot
        {
            get
            {
                return _isLiveSnapshot;
            }
            set
            {
                _isLiveSnapshot = value;
                this._captureSetup.UpdateMVMControlsStatus(!value);
                OnPropertyChanged("IsLiveSnapshot");
            }
        }

        public bool LiveCaptureProperty
        {
            get { return IsLive; }
            set
            {
                LiveCapture(value);
                OnPropertyChanged("LiveCaptureProperty");
            }
        }

        public bool LiveSnapshotStopped
        {
            get
            {
                return _liveSnapshotStopped;
            }
            set
            {
                _liveSnapshotStopped = value;
                OnPropertyChanged("LiveSnapshotStopped");
            }
        }

        /// <summary>
        /// Gets or sets the left of the liveImage
        /// </summary>
        public bool LiveStartButtonStatus
        {
            get
            {
                return _captureSetup.LiveStartButtonStatus;
            }
            set
            {
                this._captureSetup.LiveStartButtonStatus = value;
                OnPropertyChanged("LiveStartButtonStatus");
            }
        }

        public string PreviewProtocol
        {
            set
            {
                switch (value)
                {
                    case "ZStackPreview":
                        if (!_bw.IsBusy)
                        {
                            ZStackCapturing((bool)MVMManager.Instance["ZControlViewModel", "IsZStackCapturing", (object)false]);
                            _bw.RunWorkerAsync();
                            CreateProgressWindow();
                            _stackPreviewStopped = false;
                        }
                        break;
                    case "ZStackPreviewStop":
                        {
                            _bw.CancelAsync(); // stop background worker when z stack captured

                            ZStackCapturing((bool)MVMManager.Instance["ZControlViewModel", "IsZStackCapturing", (object)false]);

                            if (null != ZStackCaptureFinished)
                            {
                                bool continuousZStackPreview = (bool)MVMManager.Instance["ZControlViewModel", "EnableContinuousZStackPreview", (object)false];
                                if (!IsOrthogonalViewChecked && !continuousZStackPreview)
                                {
                                    ZStackCaptureFinished(!(bool)MVMManager.Instance["ZControlViewModel", "IsZStackCapturing", (object)false]);
                                }
                            }

                            //sleep to allow the bw to stop, the background worker has a 10ms sleep in a while loop, wait 1ms longer to ensure it has passed
                            System.Threading.Thread.Sleep(11);

                            System.Windows.Application.Current.Dispatcher.BeginInvoke(System.Windows.Threading.DispatcherPriority.Normal,
                        new Action(
                            delegate ()
                            {
                                bool continuousZStackPreview = (bool)MVMManager.Instance["ZControlViewModel", "EnableContinuousZStackPreview", (object)false];

                                //if continuous zstack is checked then continuosly update
                                if (continuousZStackPreview == true && !IsOrthogonalViewChecked)
                                {
                                    CloseProgressWindow();
                                    _startingContinuousZStackPreview = true;
                                    ICommand zStackPreviewCommand = (ICommand)MVMManager.Instance["ZControlViewModel", "PreviewZStackCommand", (object)null];
                                    if (null !=  zStackPreviewCommand)
                                    {
                                        zStackPreviewCommand.Execute(null);
                                    }
                                }
                                else if (!continuousZStackPreview)
                                {
                                    CloseProgressWindow();
                                }

                                _startingContinuousZStackPreview = false;
                                if (IsOrthogonalViewChecked)
                                {
                                    UpdateOrthogonalView();
                                }
                            }
                        )
                    );
                        }
                        break;
                    default:
                        break;
                }
            }
        }

        public ICommand RemoteCaptureCommand
        {
            get
            {
                if (this._remoteCaptureCommand == null)
                    this._remoteCaptureCommand = new RelayCommand(() => RemoteCaptureNow());

                return this._remoteCaptureCommand;
            }
        }

        public bool RemoteStartEnabled
        {
            get
            {
                if ((bool)MVMManager.Instance["RemoteIPCControlViewModel", "RemoteConnection", (object)false])
                {
                    return true;
                }
                return false;
            }
        }

        public string RemoteStartIcon
        {
            get
            {
                if ((bool)MVMManager.Instance["RemoteIPCControlViewModel", "RemoteConnection", (object)false])
                {
                    return @"/CaptureSetupModule;component/Icons/Run.png";
                }
                else
                {
                    return @"/CaptureSetupModule;component/Icons/Run3.png";
                }
            }
        }

        public string SettingsTemplateName
        {
            get
            {
                return _settingsTemplateName;
            }
            set
            {
                _settingsTemplateName = value;
                OnPropertyChanged("SettingsTemplateName");
            }
        }

        public string SettingsTemplatesPath
        {
            get
            {
                return _settingsTemplatesPath;
            }
            set
            {
                _settingsTemplatesPath = value;
            }
        }

        public ICommand SnapshotCommand
        {
            get
            {
                if (this._snapshotCommand == null)
                    this._snapshotCommand = new RelayCommand(() => Snapshot());

                return this._snapshotCommand;
            }
        }

        public string SnapshotKey
        {
            get { return _snapshotKey; }
            set { _snapshotKey = value; OnPropertyChanged("SnapshotKey"); }
        }

        public string SnapshotModifier
        {
            get { return _snapshotModifier; }
            set { _snapshotModifier = value; OnPropertyChanged("SnapshotModifier"); }
        }

        public ICommand SnapshotSettingsCommand
        {
            get
            {
                if (this._snapshotSettingsCommand == null)
                    this._snapshotSettingsCommand = new RelayCommand(() => SnapshotSettings());

                return this._snapshotSettingsCommand;
            }
        }

        public ICommand StartCommand
        {
            get
            {
                if (this._startCommand == null)
                    this._startCommand = new RelayCommand(() => StartOrStopLiveCapture());

                return this._startCommand;
            }
        }

        public ICommand StartFastFocusCommand
        {
            get
            {
                if (this._startFastFocusCommand == null)
                    this._startFastFocusCommand = new RelayCommand(() => StartFastFocus());

                return this._startFastFocusCommand;
            }
        }

        public string StartKey
        {
            get { return _startKey; }
            set { _startKey = value; OnPropertyChanged("StartKey"); }
        }

        public string StartModifier
        {
            get { return _startModifier; }
            set { _startModifier = value; OnPropertyChanged("StartModifier"); }
        }

        public ICommand StopCommand
        {
            get
            {
                if (this._stopCommand == null)
                    this._stopCommand = new RelayCommand(() => StopLiveCapture());

                return this._stopCommand;
            }
        }

        #endregion Properties

        #region Methods

        public bool AutoExposure(double exposure)
        {
            bool ret = this._captureSetup.AutoExposure(exposure);

            OnPropertyChanged("ExposureTimeCam");

            return ret;
        }

        /// <summary>
        /// Auto focus
        /// </summary>
        //public bool AutoFocus()
        //{
        //    if (ExperimentDoc == null)
        //    {
        //        return false;
        //    }
        //    bool ret = this._captureSetup.AutoFocus(Convert.ToDouble(ExperimentDoc.DocumentElement["Magnification"].GetAttribute("mag"), CultureInfo.InvariantCulture));
        //    //retrieve the z position after an autofocus
        //    OnPropertyChanged("ZPosition");
        //    OnPropertyChanged("ZPosOutOfBounds");
        //    return ret;
        //}
        public void LiveCapture(bool input)
        {
            if (true == input)
            {
                StartOrStopLiveCapture();
                if (false == _bw.IsBusy)
                {
                    _bw.RunWorkerAsync();
                }
            }
            else
            {
                StopLiveCapture();
                _bw.CancelAsync();
            }
        }

        public void PrePostSettingsToCamera()
        {
            /////Update Camera Active Settings/////
            //Call camera preflight acquisition to update all the settings on the camera
            //Allowing to save the right settings into the active.xml
            ResourceManagerCS.PreflightCamera((int)SelectedHardware.SELECTED_CAMERA1);
            //After calling preflight acquisition we MUST call postflight acquisition to release all the resources
            //failing to call postflight position will cause multiple bugs, that will be hard to track
            ResourceManagerCS.PostflightCamera((int)SelectedHardware.SELECTED_CAMERA1);
            ///End update Camera Active Settings/////
        }

        /// <summary>
        /// Snapshot is executed
        /// </summary>
        private void Snapshot()
        {
            if (!_enableCapture) return;
            if (null != snapshotWorker)
            {
                if (snapshotWorker.IsBusy)
                {
                    return;
                }
            }
            if ((IsBleaching || (bool)MVMManager.Instance["ZControlViewModel", "IsZStackCapturing", (object)false]) &&
                ((int)ICamera.CameraType.LSM == ResourceManagerCS.GetCameraType()) &&
                ((int)ICamera.LSMType.GALVO_GALVO == ResourceManagerCS.GetLSMType()) &&
                ((int)ICamera.LSMType.GALVO_GALVO == ResourceManagerCS.GetBleacherType()))
            {
                //cannot snapshot while bleaching with the same lsm type:
                return;
            }
            LiveSnapshotStopped = false;
            IsLiveSnapshot = true;
            BitmapReady = true;
            snapshotWorker = new BackgroundWorker();
            snapshotWorker.WorkerSupportsCancellation = true;

            snapshotWorker.DoWork += (obj, eventArg) =>
            {
                do
                {
                    if (CaptureSetup.IsPixelDataReady())
                    {
                        OnPropertyChanged("Bitmap");
                    }
                    System.Threading.Thread.Sleep(30);
                    if (!CaptureStatus)
                        break;
                }
                while (IsLiveSnapshot);
            };

            snapshotWorker.RunWorkerCompleted += (obj, eventArg) =>
            {
                this._captureSetup.SnapshotIsDone();

                if ((int)ICamera.LSMAreaMode.LINE == (int)MVMManager.Instance["AreaControlViewModel", "LSMAreaMode", (object)0])
                {
                    if (null != DrawLineForLineScanEvent)
                    {
                        DrawLineForLineScanEvent();
                    }
                }
                if (CaptureSetup.IsPixelDataReady())
                {
                    OnPropertyChanged("Bitmap");
                    OnPropertyChanged("BitmapPresentation");
                }
                UpdateOverlayManager();
                RequestROIData();

                //user has chosen to save without the experiment info
                //save the image with the viewmodel bitmap when not more than 3 channels:
                if (this._captureSetup.SaveSnapshot && !this._captureSetup.SnapshotIncludeExperimentInfo && (0xF > this._captureSetup.GetChannelEnabledBitmask()))
                {
                    SaveNow();
                }
                IsLiveSnapshot = false;
                LiveImageCapturing(!CaptureStatus);
                ((IMVM)MVMManager.Instance["ObjectiveControlViewModel", this]).OnPropertyChange("FramesPerSecondText");
                ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("FramesPerSecondAverage");
                CloseProgressWindow();
            };

            //Snapshots don't currentlty do sequential captures
            //Change to non sequential capture, so the settings file is correct.
            //and change back after
            int seqCapture = this.EnableSequentialCapture;
            this.EnableSequentialCapture = 0;

            //first capture the image
            this._captureSetup.Snapshot();

            this.EnableSequentialCapture = seqCapture;

            snapshotWorker.RunWorkerAsync();
            if (snapshotWorker.IsBusy)
            {
                CreateProgressWindow();
            }
        }

        /// <summary>
        /// Snapshots settings dialiog is presented
        /// </summary>
        private void SnapshotSettings()
        {
            SnapSettings dlg = new SnapSettings();

            dlg.AutoSave = this._captureSetup.SaveSnapshot;
            dlg.FileName = this._captureSetup.SnapshotBaseName;
            dlg.IncludeExperimentInfo = this._captureSetup.SnapshotIncludeExperimentInfo;
            dlg.FilePath = GetExperimentSavingPath();
            if (true == dlg.ShowDialog())
            {
                this._captureSetup.SaveSnapshot = dlg.AutoSave;
                this._captureSetup.SnapshotBaseName = dlg.FileName;
                this._captureSetup.SnapshotIncludeExperimentInfo = dlg.IncludeExperimentInfo;
                this._captureSetup.SnapshotSavingPath = dlg.FilePath;
                OnPropertyChanged("CameraSaveIcon");
            }
        }

        /// <summary>
        /// Starts the fast focus.
        /// </summary>
        private void StartFastFocus()
        {
            if (false == _isLive)
            {
                //Apply Fast Focus Parameters before live capture starts
                ApplyOrRevertFastFocusParams(false);

                FastFocusButtonStatus = !FastFocusButtonStatus;
                StartOrStopLiveCapture();
                BitmapReady = true;
            }
            else
            {
                FastFocusButtonStatus = !FastFocusButtonStatus;
                StartOrStopLiveCapture();

                //Revert Fast Focus Parameters after live capture stops
                ApplyOrRevertFastFocusParams(true);
            }
        }

        /// <summary>
        /// Start live image capture
        /// </summary>
        private void StartOrStopLiveCapture()
        {
            if ((false == _isLive))
            {
                if (!_enableCapture) return;

                if ((IsBleaching || (bool)MVMManager.Instance["ZControlViewModel", "IsZStackCapturing", (object)false] || IsLiveSnapshot) &&
                    ((int)ICamera.CameraType.LSM == ResourceManagerCS.GetCameraType()) &&
                    ((int)ICamera.LSMType.GALVO_GALVO == ResourceManagerCS.GetLSMType()) &&
                    ((int)ICamera.LSMType.GALVO_GALVO == ResourceManagerCS.GetBleacherType()))
                {
                    //cannot capture while bleaching with the same lsm type:
                    return;
                }
                //do not start if already running
                if (null != liveImageWorker)
                {
                    if (liveImageWorker.IsBusy)
                    {
                        return;
                    }
                }

                this._captureSetup.Start();

                if (false == _bw.IsBusy)
                {
                    _bw.RunWorkerAsync();
                }

                if ((int)ICamera.LSMAreaMode.LINE == (int)MVMManager.Instance["AreaControlViewModel", "LSMAreaMode", (object)0] ||
                    (int)ICamera.LSMAreaMode.POLYLINE == (int)MVMManager.Instance["AreaControlViewModel", "LSMAreaMode", (object)0])
                {
                    if (null != DrawLineForLineScanEvent)
                    {
                        DrawLineForLineScanEvent();
                    }
                }

                //check capture status if live
                liveImageWorker = new BackgroundWorker();
                liveImageWorker.DoWork += (obj, eventArg) =>
                {
                    do
                    {
                        System.Threading.Thread.Sleep(30);
                        if (!CaptureStatus)
                            break;
                    }
                    while (_isLive);
                };
                liveImageWorker.RunWorkerCompleted += (obj, eventArg) =>
                {
                    LiveCapture(false);
                };
                liveImageWorker.RunWorkerAsync();
            }
            else
            {
                this._captureSetup.Stop();
                _bw.CancelAsync();
                ((IMVM)MVMManager.Instance["ObjectiveControlViewModel", this]).OnPropertyChange("FramesPerSecondText");
                ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("FramesPerSecondAverage");
            }

            _isLive = !_isLive;
            _captureSetup.SendCameraLiveStatus = _isLive;
            MVMManager.Instance["PowerControlViewModel", "PockelsCalibrateAgainEnable"] = !_isLive;

            LiveImageCapturing(IsLive);
            BitmapReady = true;
            OnPropertyChanged("ImagePathPlay");
            OnPropertyChanged("ImagePathFastFocus");
            OnPropertyChanged("IsLive");
            OnPropertyChanged("FastFocusButtonEnable");
        }

        /// <summary>
        /// Stop live image capture
        /// </summary>
        private void StopLiveCapture()
        {
            if (true == _isLive)
            {
                this._captureSetup.Stop();
                _bw.CancelAsync();
                _isLive = false;
                _captureSetup.SendCameraLiveStatus = _isLive;

                //if fast focus is active
                if (false == FastFocusButtonStatus)
                {
                    ApplyOrRevertFastFocusParams(true);
                }

                FastFocusButtonStatus = true;

                ((IMVM)MVMManager.Instance["ObjectiveControlViewModel", this]).OnPropertyChange("FramesPerSecondText");
                ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("FramesPerSecondAverage");
                OnPropertyChanged("ImagePathPlay");
                OnPropertyChanged("IsLive");
                OnPropertyChanged("ImagePathFastFocus");
            }
        }

        private void StopPreview()
        {
            if (true == IsLiveSnapshot)
            {
                LiveSnapshotStopped = true;
                if (null != stopPreviewWorker)
                {
                    if (stopPreviewWorker.IsBusy)
                    {
                        return;
                    }
                }
                stopPreviewWorker = new BackgroundWorker();
                stopPreviewWorker.DoWork += (obj, eventArg) =>
                {
                    this._captureSetup.Stop();
                    if (null != snapshotWorker)
                    {
                        while (snapshotWorker.IsBusy)
                        {
                            System.Threading.Thread.Sleep(10);
                        }
                    }
                };

                stopPreviewWorker.RunWorkerAsync();
            }

            if (true == (bool)MVMManager.Instance["ZControlViewModel", "IsZStackCapturing", (object)false])
            {
                _stackPreviewStopped = true;
                MVMManager.Instance["ZControlViewModel", "IsZCaptureStopped"] = true;
                this._captureSetup.StopZStackPreview();
                _bw.CancelAsync();
                MVMManager.Instance["ZControlViewModel", "IsZStackCapturing"] = false;
                CloseProgressWindow();
            }

            if (true == IsBleaching)
            {
                IsBleachStopped = true;
                if (null != stopPreviewWorker)
                {
                    if (stopPreviewWorker.IsBusy)
                    {
                        return;
                    }
                }
                stopPreviewWorker = new BackgroundWorker();
                stopPreviewWorker.DoWork += (obj, eventArg) =>
                {
                    StopBleach();
                    _bleachWorker.CancelAsync();
                    _slmBleachWorker.CancelAsync();
                    while ((this._captureSetup.IsPreBleachBuilding) || (!this._captureSetup.IsBleachFinished))
                    {
                        System.Threading.Thread.Sleep(10);
                    }
                };

                stopPreviewWorker.RunWorkerCompleted += (obj, eventArg) =>
                {
                    IsBleaching = false;
                    CloseProgressWindow();
                };

                stopPreviewWorker.RunWorkerAsync();
            }
        }

        #endregion Methods
    }
}