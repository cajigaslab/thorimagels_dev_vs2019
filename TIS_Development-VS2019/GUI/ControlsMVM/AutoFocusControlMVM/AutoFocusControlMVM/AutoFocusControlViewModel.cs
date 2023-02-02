namespace AutoFocusControl.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Reflection;
    using System.Runtime.CompilerServices;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Threading;
    using System.Xml;

    using AutoFocusControl.Model;

    using AutoFocusModule;

    using OverlayManager;

    using SpinnerProgress;

    using ThorLogging;

    using ThorSharedTypes;

    public class AutoFocusControlViewModel : ThorSharedTypes.VMBase, ThorSharedTypes.IMVM
    {
        #region Fields

        private readonly AutoFocusControlModel _AutoFocusControlModel;

        private static long _autoFocusRunning = 0;

        private double _absoluteStartPos = 0.0;
        private double _absoluteStopPos = 0.0;
        private bool _autoFocusButtonEnabled = true;
        private int _autoFocusType = (int)AutoFocusTypes.AF_NONE;
        private double _currentZPosition = 0;
        private double _fineAutoFocusPercentageDecrease = 0.15;
        private Dictionary<string, PropertyInfo> _properties = new Dictionary<string, PropertyInfo>();
        private int _repeats = 1;
        private SpinnerProgressCancel _spinnerUserControl = null;
        SpinnerProgressWindow _spinnerWindow = null;
        private double _startPosition = 0;
        private double _stepSizeUM = 0;
        private double _stopPositon = 0;

        #endregion Fields

        #region Constructors

        public AutoFocusControlViewModel()
        {
            this._AutoFocusControlModel = new AutoFocusControlModel();
            CreateSpinnerWindowUserControl();
        }

        #endregion Constructors

        #region Properties

        public double AbsoluteStartPosition
        {
            get
            {
                if (0 == _autoFocusRunning)
                {
                    _absoluteStartPos = StartPosition + _currentZPosition;
                    _absoluteStartPos = (_absoluteStartPos > ZMax) ? ZMax : _absoluteStartPos;
                    _absoluteStartPos = (_absoluteStartPos < ZMin) ? ZMin : _absoluteStartPos;
                }
                return _absoluteStartPos;
            }
        }

        public double AbsoluteStopPosition
        {
            get
            {
                if (0 == _autoFocusRunning)
                {
                    _absoluteStopPos = StopPosition + _currentZPosition;
                    _absoluteStopPos = (_absoluteStopPos > ZMax) ? ZMax : _absoluteStopPos;
                    _absoluteStopPos = (_absoluteStopPos < ZMin) ? ZMin : _absoluteStopPos;
                }
                return _absoluteStopPos;
            }
        }

        public bool AutoFocusButtonEnabled
        {
            get
            {
                return _autoFocusButtonEnabled;
            }
            set
            {
                _autoFocusButtonEnabled = value;
                OnPropertyChanged("AutoFocusButtonEnabled");
            }
        }

        public string AutoFocusCacheDirectory
        {
            get
            {
                return ResourceManagerCS.GetMyDocumentsThorImageFolderString() + "\\AutoFocusCache";
            }
        }

        public int AutoFocusType
        {
            get
            {
                return _autoFocusType;
            }
            set
            {
                _autoFocusType = value;
                OnPropertyChanged("AutoFocusType");
            }
        }

        public double CurrentZPosition
        {
            get
            {
                _currentZPosition = ((double)MVMManager.Instance["ZControlViewModel", "ZPosition", (object)0.0] * 1000);
                OnPropertyChanged("AbsoluteStartPosition");
                OnPropertyChanged("AbsoluteStopPosition");
                return _currentZPosition;
            }
        }

        public double FineAutoFocusPercentageDecrease
        {
            get
            {
                return _fineAutoFocusPercentageDecrease;
            }
            set
            {
                if (value > 0)
                {
                    _fineAutoFocusPercentageDecrease = value;
                }
                OnPropertyChanged("FineAutoFocusPercentageDecrease");
            }
        }

        public bool InvertZ
        {
            get
            {
                //Return the flipped boolean of ZInvert from ZControlMVM
                return !(bool)MVMManager.Instance["ZControlViewModel", "ZInvert", false];
            }
        }

        public int Repeats
        {
            get
            {
                return _repeats;
            }
            set
            {
                if (value >= 1)
                {
                    _repeats = value;
                }
                else
                {
                    _repeats = 1;
                }
                OnPropertyChanged("Repeats");
            }
        }

        public ICommand RunAutoFocusCommand
        {
            get
            {
                return new RelayCommand(() => RunAutoFocus());
            }
        }

        public double StartPosition
        {
            get
            {
                return _startPosition;
            }
            set
            {
                if (value + CurrentZPosition <= ZMin)
                {
                    _startPosition = ZMin;
                }
                else if (value + CurrentZPosition >= ZMax)
                {
                    _startPosition = ZMax;
                }
                else
                {
                    _startPosition = value;
                }
                OnPropertyChanged("StartPosition");
            }
        }

        public double StepSizeUM
        {
            get
            {
                return _stepSizeUM;
            }
            set
            {
                _stepSizeUM = value;
                OnPropertyChanged("StepSizeUM");
            }
        }

        //Value is in um
        public double StopPosition
        {
            get
            {
                return _stopPositon;
            }
            set
            {
                if (value + CurrentZPosition <= ZMin)
                {
                    _stopPositon = ZMin;
                }
                else if(value + CurrentZPosition >= ZMax)
                {
                    _stopPositon = ZMax;
                }
                else
                {
                    _stopPositon = value;
                }
                OnPropertyChanged("StopPosition");
            }
        }

        //Value is in um
        public double ZMax
        {
            get
            {
                return ((double)MVMManager.Instance["ZControlViewModel", "ZMax", (object)0.0] * 1000);
            }
        }

        public double ZMin
        {
            get
            {
                return ((double)MVMManager.Instance["ZControlViewModel", "ZMin", (object)0.0] * 1000);
            }
        }

        #endregion Properties

        #region Indexers

        public object this[string propertyName, object defaultObject = null]
        {
            get
            {
                PropertyInfo myPropInfo = GetPropertyInfo(propertyName);
                return (null != myPropInfo) ? myPropInfo.GetValue(this) : null;
            }
            set
            {
                PropertyInfo myPropInfo = GetPropertyInfo(propertyName);
                if (null != myPropInfo)
                {
                    myPropInfo.SetValue(this, value);
                }
            }
        }

        public object this[string propertyName, int index, object defaultObject = null]
        {
            get
            {
                PropertyInfo myPropInfo = GetPropertyInfo(propertyName);
                if (null != myPropInfo)
                {
                    if (typeof(CustomCollection<>) == myPropInfo.PropertyType.GetGenericTypeDefinition())
                    {
                        var collection = myPropInfo.GetValue(this, null);
                        return collection.GetType().GetProperty("Item").GetValue(collection, new object[] { index });
                    }
                    else
                    {
                        return myPropInfo.GetValue(this, null);
                    }
                }
                return defaultObject;
            }
            set
            {
                PropertyInfo myPropInfo = GetPropertyInfo(propertyName);
                if (null != myPropInfo)
                {
                    if (typeof(CustomCollection<>) == myPropInfo.PropertyType.GetGenericTypeDefinition())
                    {
                        var collection = myPropInfo.GetValue(this, null);
                        collection.GetType().GetProperty("Item").SetValue(collection, value, new object[] { index });
                    }
                    else
                    {
                        myPropInfo.SetValue(this, value, null);
                    }
                }
            }
        }

        #endregion Indexers

        #region Methods

        public void CloseProgressWindow()
        {
            if (null != this._spinnerWindow)
            {
                _autoFocusRunning = 0;
                this._spinnerWindow.Close();
                _spinnerWindow = null;
            }
        }

        public void CreateSpinnerWindowUserControl()
        {
            _spinnerUserControl = new SpinnerProgressCancel
            {
                Width = 350,
                Height = 390,
                CancelButtonWidth = 150,
                CancelButtonHeight = 40,
                CancelButtonBackground = Brushes.Red,
                CancelButtonForeground = Brushes.White,
                CancelButtonContent = "Stop Auto Focus",
                LoadingText = "Starting Auto Focus",
                ProgressVisible = Visibility.Collapsed,
            };
            _spinnerUserControl.CancelSplashProgress += new EventHandler(_spinnerWindow_Closed);
        }

        public PropertyInfo GetPropertyInfo(string propertyName)
        {
            PropertyInfo myPropInfo = null;
            if (!_properties.TryGetValue(propertyName, out myPropInfo))
            {
                myPropInfo = typeof(AutoFocusControlViewModel).GetProperty(propertyName);
                if (null != myPropInfo)
                {
                    _properties.Add(propertyName, myPropInfo);
                }
            }
            return myPropInfo;
        }

        public void LoadXMLSettings()
        {
            XmlDocument doc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS];

            XmlNodeList ndList = doc.SelectNodes("/ThorImageExperiment/Autofocus");

            string str = string.Empty;

            if (ndList.Count > 0)
            {
                if (XmlManager.GetAttribute(ndList[0], doc, "type", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        AutoFocusType = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "repeat", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        Repeats = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "stepSizeUM", ref str))
                {
                    double tmp = 0;
                    if (Double.TryParse(str, out tmp))
                    {
                        StepSizeUM = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "startPosMM", ref str))
                {
                    double tmp = 0;
                    if (Double.TryParse(str, out tmp))
                    {
                        StartPosition = tmp * 1000;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "stopPosMM", ref str))
                {
                    double tmp = 0;
                    if (Double.TryParse(str, out tmp))
                    {
                        StopPosition = tmp * 1000;
                    }
                }

                XmlDocument hardwareDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];
                XmlNodeList objList = hardwareDoc.SelectNodes("/HardwareSettings/Objectives/Objective");
                int turretPosition = (int)MVMManager.Instance["ObjectiveControlViewModel", "TurretPosition", (object)0];

                if (objList.Count > turretPosition)
                {
                    if (null == objList[turretPosition].Attributes.GetNamedItem("fineAfPercentDecrease"))
                    {
                        ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "Could not get attribute fineAfPercentDecrease from HardwareSettings file " + hardwareDoc.ToString());
                    }
                    else
                    {
                        FineAutoFocusPercentageDecrease = Convert.ToDouble(objList[turretPosition].Attributes["fineAfPercentDecrease"].Value.ToString(CultureInfo.InvariantCulture), CultureInfo.InvariantCulture);
                    }
                }
                OnPropertyChanged("ZMax");
                OnPropertyChanged("ZMin");
            }
        }

        public void OnPropertyChange(string propertyName)
        {
            if (null != GetPropertyInfo(propertyName))
            {
                OnPropertyChanged(propertyName);
            }
        }

        //Returns true if AutoFocus was stopped within 30ms
        public bool StopAutoFocus()
        {
            return (1 == AutoFocusModule.Instance.StopAutoFocus());
        }

        public void UpdateExpXMLSettings(ref XmlDocument experimentFile)
        {
            XmlNodeList ndList = experimentFile.SelectNodes("/ThorImageExperiment/Autofocus");

            if (ndList.Count > 0)
            {
                XmlManager.SetAttribute(ndList[0], experimentFile, "type", AutoFocusType.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "repeat", Repeats.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "stepSizeUM", StepSizeUM.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "startPosMM", (StartPosition / 1000).ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "stopPosMM", (StopPosition / 1000).ToString());
            }
            //Stop autofocus if user switches tabs, switches modalities, or opens Hardware Setup
            StopAutoFocus();
        }

        private void CreateProgressWindow()
        {
            if (null != _spinnerWindow)
            {
                return;
            }
            _spinnerWindow = new SpinnerProgressWindow();
            _spinnerWindow.Title = "Auto Focus Running";
            _spinnerWindow.ResizeMode = ResizeMode.NoResize;
            _spinnerWindow.Width = 350;
            _spinnerWindow.Height = 390;
            _spinnerWindow.WindowStyle = WindowStyle.SingleBorderWindow;
            _spinnerWindow.Background = Brushes.DimGray;
            _spinnerWindow.AllowsTransparency = false;

            _spinnerWindow.Content = _spinnerUserControl;

            _spinnerWindow.Owner = Application.Current.MainWindow;
            _spinnerWindow.Left = _spinnerWindow.Owner.Left + _spinnerWindow.Width;
            _spinnerWindow.Top = _spinnerWindow.Owner.Top + ((System.Windows.Controls.Panel)_spinnerWindow.Owner.Content).ActualHeight / 2;
            _spinnerWindow.Closed += new EventHandler(_spinnerWindow_Closed);

            MVMManager.Instance["CaptureSetupViewModel", "IsProgressWindowOff"] = false;
            _spinnerWindow.Show();
        }

        private void RunAutoFocus()
        {
            bool afFound = false;
            bool pixelDataReady = false;
            bool captureStatus = false;

            long autoFocusStatus = (long)AutoFocusStatusTypes.NOT_RUNNING;
            long bestContrastScore = 0;
            long currentRepeat = 0;
            double bestZPosition = 0;
            double nextZPosition = 0;
            string status = string.Empty;
            long enableImageUpdate = 1; //Inform AutoFocusModule a GUI image update is expected

            //:TODO: Verify later if we need to save the experiment file with the AutoFocusCache
            //update Active.xml
            //MVMManager.Instance["CaptureSetupViewModel", "PersistDataNow"] = true;

            try
            {
                if (Directory.Exists(AutoFocusCacheDirectory))
                {
                    ResourceManagerCS.DeleteDirectory(AutoFocusCacheDirectory);
                }

                Directory.CreateDirectory(AutoFocusCacheDirectory);
                for (int i = 1; i <= Repeats; i++)
                {
                    Directory.CreateDirectory(AutoFocusCacheDirectory + "//" + i.ToString() + "//Coarse");
                    Directory.CreateDirectory(AutoFocusCacheDirectory + "//" + i.ToString() + "//Fine");
                }

                // copy Active.xml to ZStackCacheDirectory\Experiment.xml
                //string templatesFolder = ResourceManagerCS.GetCaptureTemplatePathString();
                //string srcFile = templatesFolder + "\\Active.xml";
                //string destFile = AutoFocusCacheDirectory + "\\Experiment.xml";
                //System.IO.File.Copy(srcFile, destFile);
            }
            catch (System.IO.IOException e)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "Could not create AutoFocusCache folder\n" + this.GetType().Name + " " + e.Message);
                return;
            }

            //Stop LiveCapture
            MVMManager.Instance["CaptureSetupViewModel", "LiveCaptureProperty"] = false;

            //Set the image size to the same as Quick Find. ApplyOrRevertFastFocusParams is what Quick Find uses to set the camera params.
            MVMManager.Instance["CaptureSetupViewModel", "ApplyOrRevertFastFocusParamsMVMCall"] = false;

            //Disable the Live/Start, Snapshot, and Preview buttons
            MVMManager.Instance["CaptureSetupViewModel", "LiveStartButtonStatus"] = false;
            MVMManager.Instance["CaptureSetupViewModel", "WrapPanelEnabled"] = false;

            //Set PixelDataReady to false to prepare CaptureSetupModule's bitmap for image update
            MVMManager.Instance["CaptureSetupViewModel", "PixelDataReady"] = false;

            //Send the setup values to the correct type of auto focus before running. In other words Preflight the auto focus procedure.
            AutoFocusModule.Instance.SetupAutoFocus(AutoFocusType, Repeats, 0.1650, 1, StepSizeUM, StartPosition/1000.0, StopPosition/1000.0, 1, FineAutoFocusPercentageDecrease, enableImageUpdate);

            double magnification = (double)MVMManager.Instance["ObjectiveControlViewModel", "TurretMagnification", (object)10.0];

            // Run Autofocus through Capture Setup so we can save the images as tiffs and display them as preview
            _AutoFocusControlModel.AutoFocus(magnification, AutoFocusType, ref afFound);

            //Create a background worker that updates the status of the spinner window and it also calls onPropertyChanges on the bitmap from CaptureSetupModule
            BackgroundWorker bitmapUpdateWorker = new BackgroundWorker();
            bitmapUpdateWorker.WorkerSupportsCancellation = true;

            bitmapUpdateWorker.DoWork += (obj, eventArg) =>
            {
                do
                {
                    //Check if CaptureSetupModule has a new image buffer ready from CaptureSetup.cpp
                    pixelDataReady = (bool)MVMManager.Instance["CaptureSetupViewModel", "PixelDataReady", (object)false];
                    if (pixelDataReady)
                    {
                        ((IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("Bitmap");
                    }
                    System.Threading.Thread.Sleep(30);
                    captureStatus = (bool)MVMManager.Instance["CaptureSetupViewModel", "CaptureStatus", (object)false];
                    if (!captureStatus)
                        break;

                    //Update the text in the spinner window, read the current status from AutoFocusModule
                    if (null != _spinnerWindow)
                    {
                        Application.Current.Dispatcher.Invoke(() =>
                        {
                            if (null != _spinnerWindow && null != _spinnerUserControl)
                            {
                                AutoFocusModule.Instance.GetAutoFocusStatus(ref autoFocusStatus, ref bestContrastScore, ref bestZPosition, ref nextZPosition, ref currentRepeat);
                                bestZPosition = Math.Round(bestZPosition * 1000, 2);
                                nextZPosition = Math.Round(nextZPosition * 1000, 2);
                                switch ((AutoFocusStatusTypes)autoFocusStatus)
                                {
                                    case AutoFocusStatusTypes.NOT_RUNNING: status = "       Not Running"; break;
                                    case AutoFocusStatusTypes.COARSE_AUTOFOCUS: status = "      Coarse Auto Focus"; break;
                                    case AutoFocusStatusTypes.FINE_AUTOFOCUS: status = "        Fine Auto Focus"; break;
                                    case AutoFocusStatusTypes.STOPPED: status = "       Stopped"; break;
                                    case AutoFocusStatusTypes.HARDWARE_AUTOFOCUS: status = "      Hardware Auto Focus"; break;
                                }
                                _spinnerUserControl.LoadingText = "Status: " + status + "\nCurrent repeat:      " + currentRepeat.ToString() + "\nBest Contrast Score:    " + bestContrastScore.ToString() + "\nBest Z Position Found:    " + bestZPosition.ToString() + "\nNext Z Position:    " + nextZPosition.ToString(); ;
                            }
                        });
                    }

                    //OnPropertyChanged("CurrentZPosition");
                    _autoFocusRunning = AutoFocusModule.Instance.IsAutoFocusRunning();
                }
                while (1 == _autoFocusRunning);
                _autoFocusRunning = 0;
            };

            bitmapUpdateWorker.RunWorkerCompleted += (obj, eventArg) =>
            {
                pixelDataReady = (bool)MVMManager.Instance["CaptureSetupViewModel", "PixelDataReady", (object)false];
                if (pixelDataReady)
                {
                    ((IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("Bitmap");
                    ((IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("BitmapPresentation");
                }

                MVMManager.Instance["CaptureSetupViewModel", "LiveStartButtonStatus"] = true;
                MVMManager.Instance["CaptureSetupViewModel", "WrapPanelEnabled"] = true;
                //Set the image size back to the original size
                MVMManager.Instance["CaptureSetupViewModel", "ApplyOrRevertFastFocusParamsMVMCall"] = true;
                CloseProgressWindow();
            };

            bitmapUpdateWorker.RunWorkerAsync();
            if (bitmapUpdateWorker.IsBusy)
            {
                CreateProgressWindow();
            }
        }

        private void stopButton_Clicked(object sender, RoutedEventArgs e)
        {
            StopAutoFocus();
        }

        void _spinnerWindow_Closed(object sender, EventArgs e)
        {
            StopAutoFocus();
            MVMManager.Instance["CaptureSetupViewModel", "IsProgressWindowOff"] = true;
            _autoFocusRunning = 0;
        }

        #endregion Methods
    }
}