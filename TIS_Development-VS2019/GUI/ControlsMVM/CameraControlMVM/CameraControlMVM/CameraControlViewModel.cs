namespace CameraControl.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Reflection;
    using System.Runtime.CompilerServices;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Xml;

    using AutoExposureModule;

    using CameraControl.Model;

    using OverlayManager;

    using SpinnerProgress;

    using ThorLogging;

    using ThorSharedTypes;

    public class CameraControlViewModel : ThorSharedTypes.VMBase, ThorSharedTypes.IMVM
    {
        #region Fields

        private readonly CameraControlModel _cameraControlModel;

        private SpinnerProgressCancel _autoexposureSpinnerCancelControl = null;
        private SpinnerProgressWindow _autoexposureSpinnerProgressWindow = null;
        ObservableCollection<string> _binList;
        ICommand _binXMinusCommand;
        ICommand _binXPlusCommand;
        ICommand _binYMinusCommand;
        ICommand _binYPlusCommand;
        ICommand _camAddResolutionCommand;
        ICommand _camAverageNumMinusCommand;
        ICommand _camAverageNumPlusCommand;
        ObservableCollection<string> _cameraColorImageTypeList;
        ICommand _cameraImageAngleMinusCommand;
        ICommand _cameraImageAnglePlusCommand;
        ObservableCollection<string> _cameraPolarImageTypeList;
        ICommand _camExposureTimeMinusCommand;
        ICommand _camExposureTimePlusCommand;
        ICommand _camFullFrameCommand;
        ObservableCollection<string> _camReadoutSpeedList;
        ICommand _camRegionFromROICommand;
        ICommand _continuousWhiteBalanceNumFramesDecrementCommand;
        ICommand _continuousWhiteBalanceNumFramesIncrementCommand;
        ObservableCollection<string> _coolingModeList;
        ICommand _frameRateControlMinusCommand;
        ICommand _frameRateControlPlusCommand;
        ObservableCollection<string> _hotPixelLevelList;
        ICommand _hotPixelMinusCommand;
        ICommand _hotPixelPlusCommand;
        private bool _isAutoExposureRunning = false;
        private bool _liveButtonStatus = true;
        ICommand _performAutoWhiteBalanceCommand;
        private Dictionary<string, PropertyInfo> _properties = new Dictionary<string, PropertyInfo>();
        ICommand _pulseWidthMinusCommand;
        ICommand _pulseWidthPlusCommand;
        private ICommand _reconnectCameraCommand;
        private ICommand _resyncParametersCommand;
        ICommand _runAutoExposureCommand;
        ICommand _setDefaultColorGainsCommand;

        #endregion Fields

        #region Constructors

        public CameraControlViewModel()
        {
            this._cameraControlModel = new CameraControlModel();
            CamResolution = new StringPC();
            _camReadoutSpeedList = new ObservableCollection<string>();
            _binList = new ObservableCollection<string>();
            _hotPixelLevelList = new ObservableCollection<string>();
            _coolingModeList = new ObservableCollection<string>();
            _cameraColorImageTypeList = new ObservableCollection<string>();
            _cameraPolarImageTypeList = new ObservableCollection<string>();
            AutoExposureModule.Instance.AutoExposureUpdateEvent += OnAutoExposureUpdated;
        }

        #endregion Constructors

        #region Properties

        public string ActiveCameraName
        {
            get
            {
                return _cameraControlModel.ActiveCameraName;
            }
            set
            {
                _cameraControlModel.ActiveCameraName = value;
            }
        }

        public double AutoExposurePercent
        {
            get
            {
                return AutoExposureModule.Instance.TargetPercent * 100.0;
            }

            set
            {
                AutoExposureModule.Instance.TargetPercent = value / 100.0;
            }
        }

        public int BinIndex
        {
            get
            {
                return _cameraControlModel.BinIndex;
            }
            set
            {
                _cameraControlModel.BinIndex = value;
                OnPropertyChanged("BinIndex");
                OnPropertyChanged("PixelSizeUM");
                OnPropertyChanged("CamImageWidth");
                OnPropertyChanged("CamImageHeight");
                OnPropertyChanged("CameraRegionWidthUM");
                OnPropertyChanged("CameraRegionHeightUM");
                OnPropertyChanged("ExposureTimeCam");
            }
        }

        public ObservableCollection<string> BinList
        {
            get
            {
                _binList.Clear();
                if ((int)ICamera.CCDType.ORCA == ResourceManagerCS.GetCCDType())
                {
                    _binList.Add("1");
                    _binList.Add("2");
                    _binList.Add("4");
                }
                return _binList;
            }
        }

        public bool BinningOrcaType
        {
            get
            {
                return ((int)ICamera.CCDType.ORCA == ResourceManagerCS.GetCCDType());
            }
        }

        public int BinX
        {
            get
            {
                return _cameraControlModel.BinX;
            }
            set
            {
                _cameraControlModel.BinX = value;
                OnPropertyChanged("BinX");
                OnPropertyChanged("CamImageWidth");
                OnPropertyChanged("CamImageHeight"); // We need both in case the Image Angle is 90 or 270
                OnPropertyChanged("PixelSizeUM");
            }
        }

        public ICommand BinXMinusCommand
        {
            get
            {
                if (_binXMinusCommand == null)
                    _binXMinusCommand = new RelayCommand(() => BinXMinus());

                return _binXMinusCommand;
            }
        }

        public ICommand BinXPlusCommand
        {
            get
            {
                if (_binXPlusCommand == null)
                    _binXPlusCommand = new RelayCommand(() => BinXPlus());

                return _binXPlusCommand;
            }
        }

        public int BinY
        {
            get
            {
                return _cameraControlModel.BinY;
            }
            set
            {
                _cameraControlModel.BinY = value;
                OnPropertyChanged("BinY");
                OnPropertyChanged("CamImageHeight");
                OnPropertyChanged("CamImageWidth"); // We need both in case the Image Angle is 90 or 270
                OnPropertyChanged("PixelSizeUM");
            }
        }

        public ICommand BinYMinusCommand
        {
            get
            {
                if (_binYMinusCommand == null)
                    _binYMinusCommand = new RelayCommand(() => BinYMinus());

                return _binYMinusCommand;
            }
        }

        public ICommand BinYPlusCommand
        {
            get
            {
                if (_binYPlusCommand == null)
                    _binYPlusCommand = new RelayCommand(() => BinYPlus());

                return _binYPlusCommand;
            }
        }

        public double BlueGain
        {
            get
            {
                return _cameraControlModel.BlueGain;
            }
            set
            {
                _cameraControlModel.BlueGain = value;
                OnPropertyChanged("BlueGain");
            }
        }

        public int Bottom
        {
            get
            {
                return _cameraControlModel.Bottom;
            }
            set
            {
                _cameraControlModel.Bottom = value;
                OnPropertyChanged("Bottom");
                ((IMVM)MVMManager.Instance["AreaControlViewModel"]).OnPropertyChange("FieldSizeHeightUM");

                CamResolution.Value = string.Format("T: {0} L: {1} B: {2} R: {3}", Top, Left, Bottom, Right);

                if (0 != CameraImageAngle && 180 != CameraImageAngle)
                {
                    OnPropertyChanged("CamImageWidth");
                    OnPropertyChanged("CameraRegionWidthUM");
                }
                else
                {
                    OnPropertyChanged("CamImageHeight");
                    OnPropertyChanged("CameraRegionHeightUM");
                }
                ((IMVM)MVMManager.Instance["XYTileControlViewModel", this]).OnPropertyChange("ScanAreaWidth");
                ((IMVM)MVMManager.Instance["XYTileControlViewModel", this]).OnPropertyChange("ScanAreaHeight");
                OnPropertyChanged("ExposureTimeCam");
            }
        }

        public int BottomMax
        {
            get
            {
                return _cameraControlModel.BottomMax;
            }
        }

        public int BottomMin
        {
            get
            {
                return _cameraControlModel.BottomMin;
            }
        }

        public ICommand BrowseForReferenceImageCommand
        {
            get
            {
                return ((ICommand)MVMManager.Instance["ImageViewCaptureSetupVM", "BrowseForReferenceImageCommand", (object)new RelayCommand(() => { })]);
            }
        }

        public ICommand CamAddResolutionCommand
        {
            get
            {
                if (_camAddResolutionCommand == null)
                    _camAddResolutionCommand = new RelayCommand(() => CamAddResolution());

                return _camAddResolutionCommand;
            }
        }

        public int CamAverageMode
        {
            get
            {
                return _cameraControlModel.CamAverageMode;
            }
            set
            {
                _cameraControlModel.CamAverageMode = value;
                OnPropertyChanged("CamAverageMode");
            }
        }

        public int CamAverageNum
        {
            get
            {
                return _cameraControlModel.CamAverageNum;
            }
            set
            {
                _cameraControlModel.CamAverageNum = value;
                OnPropertyChanged("CamAverageNum");
            }
        }

        public ICommand CamAverageNumMinusCommand
        {
            get
            {
                if (_camAverageNumMinusCommand == null)
                    _camAverageNumMinusCommand = new RelayCommand(() => CamAverageNumMinus());

                return _camAverageNumMinusCommand;
            }
        }

        public ICommand CamAverageNumPlusCommand
        {
            get
            {
                if (_camAverageNumPlusCommand == null)
                    _camAverageNumPlusCommand = new RelayCommand(() => CamAverageNumPlus());

                return _camAverageNumPlusCommand;
            }
        }

        public int CamBitsPerPixel
        {
            get
            {
                return _cameraControlModel.CamBitsPerPixel;
            }
        }

        public int CamBlackLevel
        {
            get
            {
                return _cameraControlModel.CamBlackLevel;
            }
            set
            {
                _cameraControlModel.CamBlackLevel = value;
                OnPropertyChanged("CamBlackLevel");
            }
        }

        public int CameraColorImageTypeIndex
        {
            get
            {
                return _cameraControlModel.CamColorImageTypeIndex;
            }
            set
            {
                if (0 != value)
                {
                    // Make sure binning is set to 1 if not unprocessed
                    _cameraControlModel.BinX = 1;
                    _cameraControlModel.BinY = 1;
                    OnPropertyChanged("BinX");
                    OnPropertyChanged("BinY");
                }
                _cameraControlModel.CamColorImageTypeIndex = value;
                OnPropertyChanged("CameraColorImageTypeIndex");
                OnPropertyChanged("IsBinningAvailable");
            }
        }

        public ObservableCollection<string> CameraColorImageTypeList
        {
            get
            {
                if (ActiveCameraName.Contains("C14440") || ActiveCameraName.Contains("C13440"))
                {
                    return _cameraColorImageTypeList;// TODO: color versions of these cameras?
                }

                _cameraColorImageTypeList.Clear();
                _cameraColorImageTypeList.Add("Unprocessed");
                if (ICamera.CameraSensorType.BAYER == _cameraControlModel.CameraSensorType)
                {
                    _cameraColorImageTypeList.Add("sRGB");
                    _cameraColorImageTypeList.Add("Linear sRGB");
                }

                return _cameraColorImageTypeList;
            }
        }

        public bool CameraCSType
        {
            get
            {
                return _cameraControlModel.CameraCSType;
            }
            set
            {
                _cameraControlModel.CameraCSType = value;
                OnPropertyChanged("CameraCSType");
                OnPropertyChanged("CamReadoutSpeedList");
                OnPropertyChanged("HotPixelVis");
                OnPropertyChanged("IsGainVisible");
                OnPropertyChanged("IsGainInDecibels");
                OnPropertyChanged("IsGainContiguous");
                OnPropertyChanged("IsBlackLevelVisible");
                OnPropertyChanged("IsReadoutVisible");
                OnPropertyChanged("IsTapsVisible");
                OnPropertyChanged("ExposureTimeCam");
                OnPropertyChanged("IsCameraColorImageTypeVisible");
                OnPropertyChanged("IsCameraPolarImageTypeVisible");
                OnPropertyChanged("IsNirBoostVisible");
                OnPropertyChanged("IsColorGainsVisible");
            }
        }

        public int CameraImageAngle
        {
            get
            {
                return _cameraControlModel.CameraImageAngle;
            }
            set
            {
                _cameraControlModel.CameraImageAngle = value;
                OnPropertyChanged("CameraImageAngle");
                OnPropertyChanged("CamImageHeight");
                OnPropertyChanged("CamImageWidth");
                OnPropertyChanged("CameraRegionHeightUM");
                OnPropertyChanged("CameraRegionWidthUM");
            }
        }

        public ICommand CameraImageAngleMinusCommand
        {
            get
            {
                if (_cameraImageAngleMinusCommand == null)
                    _cameraImageAngleMinusCommand = new RelayCommand(() => CameraImageAngleMinus());

                return _cameraImageAngleMinusCommand;
            }
        }

        public ICommand CameraImageAnglePlusCommand
        {
            get
            {
                if (_cameraImageAnglePlusCommand == null)
                    _cameraImageAnglePlusCommand = new RelayCommand(() => CameraImageAnglePlus());

                return _cameraImageAnglePlusCommand;
            }
        }

        public int CameraPolarImageTypeIndex
        {
            get
            {
                return _cameraControlModel.CamPolarImageTypeIndex;
            }
            set
            {
                if ((int)ICamera.PolarizationTransformType.UNPROCESSED != value)
                {
                    // Make sure binning is set to 1 if not unprocessed
                    _cameraControlModel.BinX = 1;
                    _cameraControlModel.BinY = 1;
                    OnPropertyChanged("BinX");
                    OnPropertyChanged("BinY");
                }
                _cameraControlModel.CamPolarImageTypeIndex = value;
                OnPropertyChanged("CameraPolarImageTypeIndex");
                OnPropertyChanged("IsBinningAvailable");
            }
        }

        public ObservableCollection<string> CameraPolarImageTypeList
        {
            get
            {
                if (ActiveCameraName.Contains("C14440") || ActiveCameraName.Contains("C13440"))
                {
                    return _cameraPolarImageTypeList;// TODO: polar versions of these cameras?
                }

                _cameraPolarImageTypeList.Clear();
                _cameraPolarImageTypeList.Add("Unprocessed");
                if (ICamera.CameraSensorType.MONOCHROME_POLARIZED == _cameraControlModel.CameraSensorType)
                {
                    _cameraPolarImageTypeList.Add("Intensity");
                    _cameraPolarImageTypeList.Add("DoLP");
                    _cameraPolarImageTypeList.Add("Azimuth");
                    _cameraPolarImageTypeList.Add("Quadview");
                }

                return _cameraPolarImageTypeList;
            }
        }

        public double CameraRegionHeightUM
        {
            get
            {
                double pixelSize = (1 < BinX || 1 < BinY) ? CamPixelSizeUM * Math.Max(BinY, BinX) : CamPixelSizeUM;
                Decimal decY = new Decimal((CamImageHeight) * pixelSize);
                Decimal decX = new Decimal((CamImageWidth) * pixelSize);
                if (0 != CameraImageAngle && 180 != CameraImageAngle)
                {
                    return Convert.ToDouble(Decimal.Round(decX, 2).ToString());
                }
                return Convert.ToDouble(Decimal.Round(decY, 2).ToString());
            }
        }

        public double CameraRegionWidthUM
        {
            get
            {
                double pixelSize = (1 < BinX || 1 < BinY) ? CamPixelSizeUM * Math.Max(BinY, BinX) : CamPixelSizeUM;
                Decimal decY = new Decimal((CamImageHeight) * pixelSize);
                Decimal decX = new Decimal((CamImageWidth) * pixelSize);
                if (0 != CameraImageAngle && 180 != CameraImageAngle)
                {
                    return Convert.ToDouble(Decimal.Round(decY, 2).ToString());
                }
                return Convert.ToDouble(Decimal.Round(decX, 2).ToString());
            }
        }

        public ICommand CamExposureTimeMinusCommand
        {
            get
            {
                if (_camExposureTimeMinusCommand == null)
                    _camExposureTimeMinusCommand = new RelayCommand(() => CamExposureTimeMinus());

                return _camExposureTimeMinusCommand;
            }
        }

        public ICommand CamExposureTimePlusCommand
        {
            get
            {
                if (_camExposureTimePlusCommand == null)
                    _camExposureTimePlusCommand = new RelayCommand(() => CamExposureTimePlus());

                return _camExposureTimePlusCommand;
            }
        }

        public ICommand CamFullFrameCommand
        {
            get
            {
                if (_camFullFrameCommand == null)
                    _camFullFrameCommand = new RelayCommand(() => CamFullFrame());

                return _camFullFrameCommand;
            }
        }

        public int CamHorizontalFlip
        {
            get
            {
                return _cameraControlModel.CamHorizontalFlip;
            }
            set
            {
                _cameraControlModel.CamHorizontalFlip = value;
                OnPropertyChanged("CamHorizontalFlip");
            }
        }

        public int CamImageHeight
        {
            get
            {
                return _cameraControlModel.CamImageHeight;
            }
        }

        public int CamImageWidth
        {
            get
            {
                return _cameraControlModel.CamImageWidth;
            }
        }

        public bool CamLedAvailable
        {
            get
            {
                return _cameraControlModel.CamLedAvailable;
            }
        }

        public int CamLedEnable
        {
            get
            {
                return _cameraControlModel.CamLedEnable;
            }
            set
            {
                _cameraControlModel.CamLedEnable = value;
                OnPropertyChanged("CamLedEnable");
            }
        }

        public double CamOrcaFrameRate
        {
            get
            {
                return _cameraControlModel.CamOrcaFrameRate;
            }
            set
            {
                _cameraControlModel.CamOrcaFrameRate = value;
                OnPropertyChanged("CamOrcaFrameRate");
                OnPropertyChanged("ExposureTimeCam");
            }
        }

        public double CamPixelSizeUM
        {
            get
            {
                return Math.Round(_cameraControlModel.CamSensorPixelSizeUM / (double)MVMManager.Instance["ObjectiveControlViewModel", "TurretMagnification", (object)1.0], 3);
            }
        }

        public int CamReadoutSpeedIndex
        {
            get
            {
                return _cameraControlModel.CamReadoutSpeedIndex;
            }
            set
            {
                _cameraControlModel.CamReadoutSpeedIndex = value;
                OnPropertyChanged("CamReadoutSpeedIndex");
            }
        }

        public ObservableCollection<string> CamReadoutSpeedList
        {
            get
            {
                _camReadoutSpeedList.Clear();
                if (ActiveCameraName.Contains("CS2100"))
                {
                    _camReadoutSpeedList.Add("Low Read-Noise");
                    _camReadoutSpeedList.Add("High Frame Rate");
                }
                else if (ActiveCameraName.Contains("C14440") || ActiveCameraName.Contains("C15440"))
                {
                    _camReadoutSpeedList.Add("Ultra Quiet");
                    _camReadoutSpeedList.Add("Standard");
                    _camReadoutSpeedList.Add("Fast");
                }
                else if (ActiveCameraName.Contains("C15550"))
                {
                    _camReadoutSpeedList.Add("Ultra Quiet");
                    _camReadoutSpeedList.Add("Standard");
                }
                else if ((int)ICamera.CCDType.ORCA == ResourceManagerCS.GetCCDType())
                {
                    for (int i = _cameraControlModel.CamReadoutSpeedMin; i <= _cameraControlModel.CamReadoutSpeedMax; i++)
                    {
                        _camReadoutSpeedList.Add((i + 1).ToString());
                    }
                }
                else
                {
                    _camReadoutSpeedList.Add("20 MHZ");
                    _camReadoutSpeedList.Add("40 MHZ");
                }
                return _camReadoutSpeedList;
            }
        }

        public ICommand CamRegionFromROICommand
        {
            get
            {
                if (_camRegionFromROICommand == null)
                    _camRegionFromROICommand = new RelayCommand(() => CamRegionFromROI());

                return _camRegionFromROICommand;
            }
        }

        public StringPC CamResolution
        {
            get;
            set;
        }

        public List<string> CamResolutionPresets
        {
            get
            {
                return _cameraControlModel.GetCamResolutionPresetsFromApplicationSettings();
            }
        }

        public int CamTapBalance
        {
            get
            {
                return _cameraControlModel.CamTapBalance;
            }
            set
            {
                _cameraControlModel.CamTapBalance = value;
                OnPropertyChanged("CamTapBalance");
            }
        }

        public int CamTapIndex
        {
            get
            {
                return _cameraControlModel.CamTapIndex;
            }
            set
            {
                _cameraControlModel.CamTapIndex = value;
                OnPropertyChanged("CamTapIndex");
            }
        }

        public int CamTapindexMax
        {
            get
            {
                return _cameraControlModel.CamTapIndexMax;
            }
        }

        public int CamTapindexMin
        {
            get
            {
                return _cameraControlModel.CamTapIndexMin;
            }
        }

        public int CamVerticalFlip
        {
            get
            {
                return _cameraControlModel.CamVerticalFlip;
            }
            set
            {
                _cameraControlModel.CamVerticalFlip = value;
                OnPropertyChanged("CamVerticalFlip");
            }
        }

        public int ChannelNum
        {
            get
            {
                return _cameraControlModel.ChannelNum;
            }
        }

        public int ContinuousWhiteBalanceNumFrames
        {
            get
            {
                return _cameraControlModel.ContinuousWhiteBalanceNumFrames;
            }

            set
            {
                _cameraControlModel.ContinuousWhiteBalanceNumFrames = value;
                OnPropertyChanged("ContinuousWhiteBalanceNumFrames");
            }
        }

        public ICommand ContinuousWhiteBalanceNumFramesDecrementCommand
        {
            get
            {
                if (null == _continuousWhiteBalanceNumFramesDecrementCommand)
                {
                    _continuousWhiteBalanceNumFramesDecrementCommand = new RelayCommand(() => ContinuousWhiteBalanceNumFramesDecrement());
                }

                return _continuousWhiteBalanceNumFramesDecrementCommand;
            }
        }

        public ICommand ContinuousWhiteBalanceNumFramesIncrementCommand
        {
            get
            {
                if (null == _continuousWhiteBalanceNumFramesIncrementCommand)
                {
                    _continuousWhiteBalanceNumFramesIncrementCommand = new RelayCommand(() => ContinuousWhiteBalanceNumFramesIncrement());
                }

                return _continuousWhiteBalanceNumFramesIncrementCommand;
            }
        }

        public int CoolingModeIndex
        {
            get
            {
                return _cameraControlModel.CoolingModeIndex;
            }
            set
            {
                _cameraControlModel.CoolingModeIndex = value;
                OnPropertyChanged("CoolingModeIndex");
            }
        }

        public ObservableCollection<string> CoolingModeList
        {
            get
            {
                _hotPixelLevelList.Clear();
                if ((int)ICamera.CCDType.ORCA == ResourceManagerCS.GetCCDType())
                {
                    _hotPixelLevelList.Add("OFF");
                    _hotPixelLevelList.Add("ON");
                    _hotPixelLevelList.Add("MAX");
                }
                return _hotPixelLevelList;
            }
        }

        public Visibility CoolingVisibility
        {
            get
            {
                return ((int)ICamera.CCDType.ORCA == ResourceManagerCS.GetCCDType() && _cameraControlModel.CoolingModeAvailable) ? Visibility.Visible : Visibility.Collapsed;
            }
        }

        public bool EnableReferenceChannel
        {
            get
            {
                return _cameraControlModel.EnableReferenceChannel;
            }
            set
            {
                _cameraControlModel.EnableReferenceChannel = value;
                OnPropertyChange("EnableReferenceChannel");
            }
        }

        public double EqualExposurePulseWidth
        {
            get
            {
                return _cameraControlModel.EqualExposurePulseWidth;
            }

            set
            {
                _cameraControlModel.EqualExposurePulseWidth = value;
                OnPropertyChanged("ExposureTimeCam");
                OnPropertyChanged("EqualExposurePulseWidth");
            }
        }

        public double ExposureTimeCam
        {
            get
            {
                return _cameraControlModel.ExposureTimeCam;
            }
            set
            {
                _cameraControlModel.ExposureTimeCam = value;
                MVMManager.Instance["KuriosControlViewModel", "ExposureTimeCam", (object)1.0] = value;
                OnPropertyChanged("ExposureTimeCam");
                OnPropertyChanged("ExposureTimeMin");
                OnPropertyChanged("ExposureTimeMax");
                OnPropertyChanged("FrameRateControlValue");
                ((IMVM)MVMManager.Instance["ObjectiveControlViewModel"])?.OnPropertyChange("FramesPerSecondText");
                ((IMVM)MVMManager.Instance["CaptureSetupViewModel"])?.OnPropertyChange("CollapsedExposureTime");
            }
        }

        public double ExposureTimeCam1
        {
            get
            {
                return _cameraControlModel.ExposureTimeCam1;
            }
            set
            {
                _cameraControlModel.ExposureTimeCam1 = value;
                OnPropertyChanged("ExposureTimeCam1");
                OnPropertyChanged("ExposureTimeMin");
                OnPropertyChanged("ExposureTimeMax");
            }
        }

        public double ExposureTimeCam2
        {
            get
            {
                return _cameraControlModel.ExposureTimeCam2;
            }
            set
            {
                _cameraControlModel.ExposureTimeCam2 = value;
                OnPropertyChanged("ExposureTimeCam2");
                OnPropertyChanged("ExposureTimeMin");
                OnPropertyChanged("ExposureTimeMax");
            }
        }

        public double ExposureTimeMax
        {
            get
            {
                return _cameraControlModel.ExposureTimeMax;
            }
        }

        public double ExposureTimeMin
        {
            get
            {
                return _cameraControlModel.ExposureTimeMin;
            }
        }

        public int FrameRateControlEnabled
        {
            get
            {
                return _cameraControlModel.FrameRateControlEnabled;
            }
            set
            {
                _cameraControlModel.FrameRateControlEnabled = value;
                OnPropertyChanged("FrameRateControlEnabled");
                OnPropertyChanged("FrameRateControlValue");
                OnPropertyChanged("FrameRateControlMin");
                OnPropertyChanged("FrameRateControlMax");
            }
        }

        public double FrameRateControlMax
        {
            get
            {
                return _cameraControlModel.FrameRateControlMax;
            }
        }

        public double FrameRateControlMin
        {
            get
            {
                return _cameraControlModel.FrameRateControlMin;
            }
        }

        public ICommand FrameRateControlMinusCommand
        {
            get
            {
                if (_frameRateControlMinusCommand == null)
                    _frameRateControlMinusCommand = new RelayCommand(() => FrameRateControlMinus());

                return _frameRateControlMinusCommand;
            }
        }

        public ICommand FrameRateControlPlusCommand
        {
            get
            {
                if (_frameRateControlPlusCommand == null)
                    _frameRateControlPlusCommand = new RelayCommand(() => FrameRateControlPlus());

                return _frameRateControlPlusCommand;
            }
        }

        public double FrameRateControlValue
        {
            get
            {
                return _cameraControlModel.FrameRateControlValue;
            }
            set
            {
                // Exposure can limit the frame rate. Check if the new frame rate exceeds this limit (using miliseconds for calculation)
                if (value > (1000.0 / ExposureTimeCam))
                {
                    _cameraControlModel.FrameRateControlValue = 1000.0 / ExposureTimeCam;
                }
                else
                {
                    _cameraControlModel.FrameRateControlValue = value;
                }
                OnPropertyChanged("FrameRateControlValue");
            }
        }

        public bool FrameRateControlVisibility
        {
            get
            {
                return (FrameRateControlMax > 0);
            }
        }

        public int Gain
        {
            get
            {
                return _cameraControlModel.Gain;
            }
            set
            {
                _cameraControlModel.Gain = value;
                OnPropertyChanged("Gain");
                OnPropertyChanged("GainDecibels");
            }
        }

        public double GainDecibels
        {
            get
            {
                return _cameraControlModel.GainDecibels;
            }
            set
            {
                _cameraControlModel.GainDecibels = value;
                OnPropertyChanged("GainDecibels");
                OnPropertyChanged("Gain");
            }
        }

        public List<string> GainDiscreteOptions
        {
            get
            {
                return _cameraControlModel.GetGainDiscreteOptions();
            }
        }

        public double GreenGain
        {
            get
            {
                return _cameraControlModel.GreenGain;
            }
            set
            {
                _cameraControlModel.GreenGain = value;
                OnPropertyChanged("GreenGain");
            }
        }

        public Visibility HotPixelCBVisibility
        {
            get
            {
                return ((int)ICamera.CCDType.ORCA == ResourceManagerCS.GetCCDType() && _cameraControlModel.HotPixelAvailable) ? Visibility.Visible : Visibility.Collapsed;
            }
        }

        public int HotPixelEnabled
        {
            get
            {
                return _cameraControlModel.HotPixelEnabled;
            }
            set
            {
                _cameraControlModel.HotPixelEnabled = value;
                OnPropertyChanged("HotPixelMax");
            }
        }

        public int HotPixelLevelIndex
        {
            get
            {
                return _cameraControlModel.HotPixelLevelIndex;
            }
            set
            {
                _cameraControlModel.HotPixelLevelIndex = value;
                OnPropertyChanged("HotPixelLevelIndex");
            }
        }

        public ObservableCollection<string> HotPixelLevelList
        {
            get
            {
                _hotPixelLevelList.Clear();
                if ((int)ICamera.CCDType.ORCA == ResourceManagerCS.GetCCDType())
                {
                    _hotPixelLevelList.Add("STANDARD");
                    _hotPixelLevelList.Add("MINIMUM");
                    _hotPixelLevelList.Add("AGGRESSIVE");
                }
                return _hotPixelLevelList;
            }
        }

        public double HotPixelMax
        {
            get
            {
                return _cameraControlModel.HotPixelMax;
            }
        }

        public double HotPixelMin
        {
            get
            {
                return _cameraControlModel.HotPixelMin;
            }
        }

        public ICommand HotPixelMinusCommand
        {
            get
            {
                if (_hotPixelMinusCommand == null)
                    _hotPixelMinusCommand = new RelayCommand(() => HotPixelMinus());

                return _hotPixelMinusCommand;
            }
        }

        public ICommand HotPixelPlusCommand
        {
            get
            {
                if (_hotPixelPlusCommand == null)
                    _hotPixelPlusCommand = new RelayCommand(() => HotPixelPlus());

                return _hotPixelPlusCommand;
            }
        }

        public double HotPixelVal
        {
            get
            {
                return _cameraControlModel.HotPixelVal;
            }
            set
            {
                _cameraControlModel.HotPixelVal = value;
                OnPropertyChanged("HotPixelVal");
            }
        }

        public bool HotPixelVis
        {
            get
            {
                return (CameraCSType && 0.0 != HotPixelMax);
            }
        }

        public bool ImageStartStatusCamera
        {
            get
            {
                return _liveButtonStatus;
            }
            set
            {
                _liveButtonStatus = value;
                OnPropertyChanged("ImageStartStatusCamera");
                OnPropertyChanged("PixelSizeUM");
            }
        }

        public bool IsAutoExposureSettingEnabled
        {
            get
            {
                return !IsEqualExposurePulseEnabled;
            }
        }

        public bool IsAutoExposureSupported
        {
            get
            {
                return !_cameraControlModel.IsAutoExposureSupported;
            }
        }

        public bool IsBinningAvailable
        {
            get
            {
                if (ICamera.CameraSensorType.MONOCHROME == _cameraControlModel.CameraSensorType)
                {
                    return true;
                }
                else if (ICamera.CameraSensorType.BAYER == _cameraControlModel.CameraSensorType)
                {
                    return 0 == _cameraControlModel.CamColorImageTypeIndex;
                }
                else if (ICamera.CameraSensorType.MONOCHROME_POLARIZED == _cameraControlModel.CameraSensorType)
                {
                    return (int)ICamera.PolarizationTransformType.UNPROCESSED == _cameraControlModel.CamPolarImageTypeIndex;
                }

                return true;
            }
        }

        public bool IsBlackLevelVisible
        {
            get
            {
                return (false == ActiveCameraName.Contains("CS2100") && false == ActiveCameraName.Contains("CS135") && (int)ICamera.CCDType.ORCA != ResourceManagerCS.GetCCDType());
            }
        }

        public bool IsCameraColorImageTypeVisible
        {
            get
            {
                return ICamera.CameraSensorType.BAYER == _cameraControlModel.CameraSensorType;
            }
        }

        public bool IsCameraControlEnabled
        {
            get
            {
                return !_isAutoExposureRunning;
            }
        }

        public bool IsCameraPolarImageTypeVisible
        {
            get
            {
                return ICamera.CameraSensorType.MONOCHROME_POLARIZED == _cameraControlModel.CameraSensorType;
            }
        }

        public bool IsColorGainsVisible
        {
            get
            {
                return _cameraControlModel.IsColorGainsSupported;
            }
        }

        public bool IsContinuousWhiteBalanceEnabled
        {
            get
            {
                return _cameraControlModel.IsContinuousWhiteBalanceEnabled;
            }

            set
            {
                _cameraControlModel.IsContinuousWhiteBalanceEnabled = value;
                OnPropertyChanged("IsContinuousWhiteBalanceEnabled");
            }
        }

        public bool IsEqualExposurePulseEnabled
        {
            get
            {
                return _cameraControlModel.IsEqualExposurePulseEnabled;
            }

            set
            {
                _cameraControlModel.IsEqualExposurePulseEnabled = value;
                OnPropertyChanged("IsEqualExposurePulseEnabled");
                OnPropertyChanged("IsExposureSettingEnabled");
                OnPropertyChanged("ExposureTimeCam");
                OnPropertyChanged("IsAutoExposureSettingEnabled");
            }
        }

        public bool IsEqualExposurePulseVisible
        {
            get
            {
                return _cameraControlModel.IsEqualExposurePulseSupported;
            }
        }

        public bool IsExposureSettingEnabled
        {
            get
            {
                return !IsEqualExposurePulseEnabled;
            }
        }

        public bool IsFrameRateVisible
        {
            get
            {
                return ((int)ICamera.CCDType.ORCA == ResourceManagerCS.GetCCDType());
            }
        }

        public bool IsGainContiguous
        {
            get
            {
                ICamera.GainType gainType = _cameraControlModel.CameraGainType;
                return gainType == ICamera.GainType.CONTIGUOUS || gainType == ICamera.GainType.CONTIGUOUS_DECIBELS;
            }
        }

        public bool IsGainInDecibels
        {
            get
            {
                ICamera.GainType gainType = _cameraControlModel.CameraGainType;
                return gainType == ICamera.GainType.CONTIGUOUS_DECIBELS || gainType == ICamera.GainType.DISCRETE_DECIBELS;
            }
        }

        public bool IsGainVisible
        {
            get
            {
                return _cameraControlModel.CameraGainType != ICamera.GainType.UNSUPPORTED;
            }
        }

        public bool IsNirBoostEnabled
        {
            get
            {
                return _cameraControlModel.IsNirBoostEnabled;
            }

            set
            {
                _cameraControlModel.IsNirBoostEnabled = value;
                OnPropertyChanged("IsNirBoostEnabled");
            }
        }

        public bool IsNirBoostVisible
        {
            get
            {
                return _cameraControlModel.IsNirBoostSupported;
            }
        }

        public bool IsReadoutVisible
        {
            get
            {
                //Make it visible if the camera is not a CMOS (all CCD) or if it is the CS2100, or if the camera is an Orca and the readoutspeed range is not 0
                return (((int)ICamera.CCDType.ORCA != ResourceManagerCS.GetCCDType() && !CameraCSType) || ActiveCameraName.Contains("CS2100")) || ((int)ICamera.CCDType.ORCA == ResourceManagerCS.GetCCDType() && _cameraControlModel.CamReadoutSpeedMin != _cameraControlModel.CamReadoutSpeedMax);
            }
        }

        public bool IsTapsVisible
        {
            get
            {
                return (!CameraCSType && (int)ICamera.CCDType.ORCA != ResourceManagerCS.GetCCDType());
            }
        }

        public int Left
        {
            get
            {
                return _cameraControlModel.Left;
            }
            set
            {
                _cameraControlModel.Left = value;
                OnPropertyChanged("Left");
                ((IMVM)MVMManager.Instance["AreaControlViewModel"]).OnPropertyChange("FieldSizeWidthUM");

                CamResolution.Value = string.Format("T: {0} L: {1} B: {2} R: {3}", Top, Left, Bottom, Right);

                if (0 != CameraImageAngle && 180 != CameraImageAngle)
                {
                    OnPropertyChanged("CamImageHeight");
                    OnPropertyChanged("CameraRegionHeightUM");
                }
                else
                {
                    OnPropertyChanged("CamImageWidth");
                    OnPropertyChanged("CameraRegionWidthUM");
                }
                ((IMVM)MVMManager.Instance["XYTileControlViewModel", this]).OnPropertyChange("ScanAreaWidth");
                ((IMVM)MVMManager.Instance["XYTileControlViewModel", this]).OnPropertyChange("ScanAreaHeight");
                OnPropertyChanged("ExposureTimeCam");
            }
        }

        public int LeftMax
        {
            get
            {
                return _cameraControlModel.LeftMax;
            }
        }

        public int LeftMin
        {
            get
            {
                return _cameraControlModel.LeftMin;
            }
        }

        public int LightMode
        {
            get
            {
                return _cameraControlModel.LightMode;
            }
            set
            {
                _cameraControlModel.LightMode = value;
                OnPropertyChanged("LightMode");
            }
        }

        public int LightModeMax
        {
            get
            {
                return _cameraControlModel.LightModeMax;
            }
        }

        public int LightModeMin
        {
            get
            {
                return _cameraControlModel.LightModeMin;
            }
        }

        public bool OrcaFrameRateEnabled
        {
            get
            {
                return _cameraControlModel.OrcaFrameRateEnabled;
            }
            set
            {
                _cameraControlModel.OrcaFrameRateEnabled = value;
                OnPropertyChanged("OrcaFrameRateEnabled");
                OnPropertyChanged("ExposureTimeCam");
            }
        }

        public ICommand PerformAutoWhiteBalanceCommand
        {
            get
            {
                if (_performAutoWhiteBalanceCommand == null)
                    _performAutoWhiteBalanceCommand = new RelayCommand(() => PerformOneShotWhiteBalance());

                return _performAutoWhiteBalanceCommand;
            }
        }

        public string PixelSizeUM
        {
            get
            {
                if (1 < BinX || 1 < BinY)
                {
                    return "[" + (CamPixelSizeUM * Math.Max(BinY, BinX)).ToString() + "]";
                }
                else
                {
                    return CamPixelSizeUM.ToString();
                }
            }
        }

        public ICommand PulseWidthMinusCommand
        {
            get
            {
                if (_pulseWidthMinusCommand == null)
                    _pulseWidthMinusCommand = new RelayCommand(() => PulseWidthMinus());

                return _pulseWidthMinusCommand;
            }
        }

        public ICommand PulseWidthPlusCommand
        {
            get
            {
                if (_pulseWidthPlusCommand == null)
                    _pulseWidthPlusCommand = new RelayCommand(() => PulseWidthPlus());

                return _pulseWidthPlusCommand;
            }
        }

        public ICommand ReconnectCameraCommand
        {
            get
            {
                if (_reconnectCameraCommand == null)
                    _reconnectCameraCommand = new RelayCommand(() => ReconnectCamera());

                return _reconnectCameraCommand;
            }
        }

        public double RedGain
        {
            get
            {
                return _cameraControlModel.RedGain;
            }
            set
            {
                _cameraControlModel.RedGain = value;
                OnPropertyChanged("RedGain");
            }
        }

        public string ReferenceChannelImageName
        {
            get
            {
                return MVMManager.Instance["ImageViewCaptureSetupVM", "ReferenceChannelImageName", ""].ToString();
            }
        }

        public ICommand ResyncParametersCommand
        {
            get
            {
                if (_resyncParametersCommand == null)
                {
                    _resyncParametersCommand = new RelayCommand(() => ResyncParameters());
                }

                return _resyncParametersCommand;
            }
        }

        public int Right
        {
            get
            {
                return _cameraControlModel.Right;
            }
            set
            {
                _cameraControlModel.Right = value;
                OnPropertyChanged("Right");
                ((IMVM)MVMManager.Instance["AreaControlViewModel"]).OnPropertyChange("FieldSizeWidthUM");

                CamResolution.Value = string.Format("T: {0} L: {1} B: {2} R: {3}", Top, Left, Bottom, Right);

                if (0 != CameraImageAngle && 180 != CameraImageAngle)
                {
                    OnPropertyChanged("CamImageHeight");
                    OnPropertyChanged("CameraRegionHeightUM");
                }
                else
                {
                    OnPropertyChanged("CamImageWidth");
                    OnPropertyChanged("CameraRegionWidthUM");
                }
                ((IMVM)MVMManager.Instance["XYTileControlViewModel", this]).OnPropertyChange("ScanAreaWidth");
                ((IMVM)MVMManager.Instance["XYTileControlViewModel", this]).OnPropertyChange("ScanAreaHeight");
                OnPropertyChanged("ExposureTimeCam");
            }
        }

        public int RightMax
        {
            get
            {
                return _cameraControlModel.RightMax;
            }
        }

        public int RightMin
        {
            get
            {
                return _cameraControlModel.RightMin;
            }
        }

        public ICommand RunAutoExposureCommand
        {
            get
            {
                if (null == _runAutoExposureCommand)
                {
                    _runAutoExposureCommand = new RelayCommand(() => RunAutoExposure());
                }

                return _runAutoExposureCommand;
            }
        }

        public ICommand SetDefaultColorGainsCommand
        {
            get
            {
                if (null == _setDefaultColorGainsCommand)
                {
                    _setDefaultColorGainsCommand = new RelayCommand(() => SetDefaultColorGains());
                }

                return _setDefaultColorGainsCommand;
            }
        }

        public int Top
        {
            get
            {
                return _cameraControlModel.Top;
            }
            set
            {
                _cameraControlModel.Top = value;
                OnPropertyChanged("Top");
                ((IMVM)MVMManager.Instance["AreaControlViewModel"]).OnPropertyChange("FieldSizeHeightUM");

                CamResolution.Value = string.Format("T: {0} L: {1} B: {2} R: {3}", Top, Left, Bottom, Right);

                if (0 != CameraImageAngle && 180 != CameraImageAngle)
                {
                    OnPropertyChanged("CamImageWidth");
                    OnPropertyChanged("CameraRegionWidthUM");
                }
                else
                {
                    OnPropertyChanged("CamImageHeight");
                    OnPropertyChanged("CameraRegionHeightUM");
                }
                ((IMVM)MVMManager.Instance["XYTileControlViewModel", this]).OnPropertyChange("ScanAreaWidth");
                ((IMVM)MVMManager.Instance["XYTileControlViewModel", this]).OnPropertyChange("ScanAreaHeight");
                OnPropertyChanged("ExposureTimeCam");
            }
        }

        public int TopMax
        {
            get
            {
                return _cameraControlModel.TopMax;
            }
        }

        public int TopMin
        {
            get
            {
                return _cameraControlModel.TopMin;
            }
        }

        #endregion Properties

        #region Indexers

        public object this[string propertyName, object defaultObject = null]
        {
            get
            {
                PropertyInfo myPropInfo = typeof(CameraControlViewModel).GetProperty(propertyName);
                return (null != myPropInfo) ? myPropInfo.GetValue(this) : defaultObject;
            }
            set
            {
                PropertyInfo myPropInfo = typeof(CameraControlViewModel).GetProperty(propertyName);
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

        public string GetCameraName()
        {
            XmlDocument hardwareDoc = new XmlDocument();
            hardwareDoc.Load(ResourceManagerCS.GetHardwareSettingsFileString());
            if (null != hardwareDoc)
            {
                XmlNodeList ndListHW = hardwareDoc.SelectNodes("/HardwareSettings/ImageDetectors/Camera");
                if (ndListHW.Count > 0)
                {
                    string str = string.Empty;
                    for (int i = 0; i < ndListHW.Count; i++)
                    {
                        XmlManager.GetAttribute(ndListHW[i], hardwareDoc, "active", ref str);
                        if (1 == Convert.ToInt32(str))
                        {
                            if (XmlManager.GetAttribute(ndListHW[i], hardwareDoc, "cameraName", ref str))
                            {
                                return str;
                            }
                        }
                    }
                }
            }
            return string.Empty;
        }

        public PropertyInfo GetPropertyInfo(string propertyName)
        {
            PropertyInfo myPropInfo = null;
            if (!_properties.TryGetValue(propertyName, out myPropInfo))
            {
                myPropInfo = typeof(CameraControlViewModel).GetProperty(propertyName);
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

            XmlNodeList ndList = doc.SelectNodes("/ThorImageExperiment/Camera");

            //When loading the panel reset the index of the camera. The correct index will be loaded from active.xml later.
            CamReadoutSpeedIndex = -1;
            HotPixelLevelIndex = -1;
            CoolingModeIndex = -1;
            BinIndex = -1;

            ActiveCameraName = GetCameraName();

            CameraCSType = (ActiveCameraName.Contains("CS") || ActiveCameraName.Contains("CC")) ? true : false;

            OnPropertyChanged("CamResolutionPresets");
            //Load the visibilities of the Orca camera
            OnPropertyChanged("BinningOrcaType");
            OnPropertyChanged("BinList");
            OnPropertyChanged("HotPixelCBVisibility");
            OnPropertyChanged("HotPixelLevelList");
            OnPropertyChanged("FrameRateControlMin");
            OnPropertyChanged("FrameRateControlMax");
            OnPropertyChanged("FrameRateControlVisibility");
            OnPropertyChanged("HotPixelEnabled");
            OnPropertyChanged("HotPixelVal");
            OnPropertyChanged("IsFrameRateVisible");
            OnPropertyChanged("CoolingVisibility");
            OnPropertyChanged("CoolingModeList");
            OnPropertyChanged("PixelSizeUM");
            OnPropertyChanged("GainDiscreteOptions");

            if (ndList.Count > 0)
            {
                string str = string.Empty;

                if (XmlManager.GetAttribute(ndList[0], doc, "exposureTimeMS", ref str))
                {
                    double tmp = 0;
                    if (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmp))
                    {
                        ExposureTimeCam = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "gain", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        Gain = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "blackLevel", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        CamBlackLevel = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "lightmode", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        LightMode = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "right", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        Right = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "left", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        Left = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "bottom", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        Bottom = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "top", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        Top = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "binningX", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        BinX = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "binningY", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        BinY = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "readoutTapIndex", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        CamTapIndex = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "tapBalance", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        CamTapBalance = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "readoutSpeedIndex", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        CamReadoutSpeedIndex = tmp;
                    }
                }
                else
                {
                    CamReadoutSpeedIndex = 0;
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "averageMode", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        CamAverageMode = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "averageNum", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        CamAverageNum = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "verticalFlip", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        CamVerticalFlip = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "horizontalFlip", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        CamHorizontalFlip = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "imageAngle", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        CameraImageAngle = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "binIndex", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        BinIndex = tmp;
                    }
                }
                else
                {
                    BinIndex = 0;
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "hotPixelLevelIndex", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        HotPixelLevelIndex = tmp;
                    }
                }
                else
                {
                    HotPixelLevelIndex = 0;
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "frameRateControlEnabled", ref str))
                {
                    if (Int32.TryParse(str, out int tmp))
                    {
                        FrameRateControlEnabled = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "frameRateControlValue", ref str))
                {
                    if (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out double tmp))
                    {
                        FrameRateControlValue = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "OrcaFrameRateEnabled", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        OrcaFrameRateEnabled = Convert.ToBoolean(tmp);
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "OrcaFrameRateValue", ref str))
                {
                    double tmp = 0;
                    if (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmp))
                    {
                        CamOrcaFrameRate = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "CoolingModeIndex", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        CoolingModeIndex = tmp;
                    }
                }
                else
                {
                    CoolingModeIndex = 0;
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "colorImageTypeIndex", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        CameraColorImageTypeIndex = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "polarImageTypeIndex", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        CameraPolarImageTypeIndex = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "isNirBoostEnabled", ref str))
                {
                    bool tmp = false;
                    if (bool.TryParse(str, out tmp))
                    {
                        IsNirBoostEnabled = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "redGain", ref str))
                {
                    double tmp = 0;
                    if (double.TryParse(str, out tmp))
                    {
                        RedGain = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "greenGain", ref str))
                {
                    double tmp = 0;
                    if (double.TryParse(str, out tmp))
                    {
                        GreenGain = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "blueGain", ref str))
                {
                    double tmp = 0;
                    if (double.TryParse(str, out tmp))
                    {
                        BlueGain = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "isEepEnabled", ref str))
                {
                    bool tmp = false;
                    if (bool.TryParse(str, out tmp))
                    {
                        IsEqualExposurePulseEnabled = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "eepPulseWidth", ref str))
                {
                    double tmp = 0;
                    if (double.TryParse(str, out tmp))
                    {
                        EqualExposurePulseWidth = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "isContinuousWhiteBalanceEnabled", ref str))
                {
                    bool tmp = false;
                    if (bool.TryParse(str, out tmp))
                    {
                        IsContinuousWhiteBalanceEnabled = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "continuousWhiteBalanceNumFrames", ref str))
                {
                    int tmp = 5;
                    if (int.TryParse(str, out tmp))
                    {
                        ContinuousWhiteBalanceNumFrames = tmp;
                    }
                }
            }
        }

        public void OnPropertyChange(string propertyName)
        {
            if (null != GetPropertyInfo(propertyName))
            {
                OnPropertyChanged(propertyName);
            }
        }

        public void ReconnectCamera()
        {
            //stop the camera
            ((ICommand)MVMManager.Instance["CaptureSetupViewModel", "StopCommand", (object)new RelayCommand(() => { })]).Execute(null);

            //disconnect the camera
            CSTeardownCommand();

            //setup with the new camera
            CSSetupCommand();

            //update color and polar image type options for new camera's sensor
            OnPropertyChanged("CameraColorImageTypeList");
            OnPropertyChanged("CameraPolarImageTypeList");
        }

        public void ResyncParameters()
        {
            // only resync parameters that can be nudged
            OnPropertyChanged("ExposureTimeCam");
            OnPropertyChanged("ExposureTimeMin");
            OnPropertyChanged("ExposureTimeMax");
            OnPropertyChanged("FrameRateControlValue");
            OnPropertyChanged("FrameRateControlMin");
            OnPropertyChanged("FrameRateControlMax");
            OnPropertyChanged("CamImageWidth");
            OnPropertyChanged("CamImageHeight");
            OnPropertyChanged("CameraRegionWidthUM");
            OnPropertyChanged("CameraRegionHeightUM");
            OnPropertyChanged("RedGain");
            OnPropertyChanged("GreenGain");
            OnPropertyChanged("BlueGain");
            //OnPropertyChanged("BinX");
            //OnPropertyChanged("BinY");
        }

        public void UpdateExpXMLSettings(ref XmlDocument experimentFile)
        {
            XmlDocument hwDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];

            if ((int)ICamera.CameraType.CCD == ResourceManagerCS.GetCameraType())
            {
                XmlNodeList ndList = experimentFile.SelectNodes("/ThorImageExperiment/Camera");
                XmlNodeList ndListHW = hwDoc.SelectNodes("/HardwareSettings/ImageDetectors/Camera");
                if (ndList.Count > 0)
                {
                    XmlManager.SetAttribute(ndList[0], experimentFile, "name", ActiveCameraName);
                    XmlManager.SetAttribute(ndList[0], experimentFile, "width", this.CamImageWidth.ToString());
                    XmlManager.SetAttribute(ndList[0], experimentFile, "height", this.CamImageHeight.ToString());
                    XmlManager.SetAttribute(ndList[0], experimentFile, "pixelSizeUM", (CamPixelSizeUM * Math.Max(1, Math.Max(BinY, BinX))).ToString());
                    XmlManager.SetAttribute(ndList[0], experimentFile, "exposureTimeMS", this.ExposureTimeCam.ToString());
                    XmlManager.SetAttribute(ndList[0], experimentFile, "gain", this.Gain.ToString());
                    XmlManager.SetAttribute(ndList[0], experimentFile, "blackLevel", this.CamBlackLevel.ToString());
                    XmlManager.SetAttribute(ndList[0], experimentFile, "lightmode", this.LightMode.ToString());
                    XmlManager.SetAttribute(ndList[0], experimentFile, "left", this.Left.ToString());
                    XmlManager.SetAttribute(ndList[0], experimentFile, "right", this.Right.ToString());
                    XmlManager.SetAttribute(ndList[0], experimentFile, "top", this.Top.ToString());
                    XmlManager.SetAttribute(ndList[0], experimentFile, "bottom", this.Bottom.ToString());
                    XmlManager.SetAttribute(ndList[0], experimentFile, "binningX", this.BinX.ToString());
                    XmlManager.SetAttribute(ndList[0], experimentFile, "binningY", this.BinY.ToString());
                    XmlManager.SetAttribute(ndList[0], experimentFile, "bitsPerPixel", this.CamBitsPerPixel.ToString());
                    XmlManager.SetAttribute(ndList[0], experimentFile, "readoutTapIndex", this.CamTapIndex.ToString());
                    XmlManager.SetAttribute(ndList[0], experimentFile, "tapBalance", this.CamTapBalance.ToString());
                    XmlManager.SetAttribute(ndList[0], experimentFile, "readoutSpeedIndex", this.CamReadoutSpeedIndex.ToString());
                    XmlManager.SetAttribute(ndList[0], experimentFile, "averageMode", this.CamAverageMode.ToString());
                    XmlManager.SetAttribute(ndList[0], experimentFile, "averageNum", this.CamAverageNum.ToString());
                    XmlManager.SetAttribute(ndList[0], experimentFile, "verticalFlip", this.CamVerticalFlip.ToString());
                    XmlManager.SetAttribute(ndList[0], experimentFile, "horizontalFlip", this.CamHorizontalFlip.ToString());
                    XmlManager.SetAttribute(ndList[0], experimentFile, "imageAngle", this.CameraImageAngle.ToString());
                    XmlManager.SetAttribute(ndList[0], experimentFile, "isCSType", this.CameraCSType.ToString());
                    XmlManager.SetAttribute(ndList[0], experimentFile, "binIndex", this.BinIndex.ToString());
                    XmlManager.SetAttribute(ndList[0], experimentFile, "hotPixelLevelIndex", this.HotPixelLevelIndex.ToString());
                    XmlManager.SetAttribute(ndList[0], experimentFile, "frameRateControlEnabled", this.FrameRateControlEnabled.ToString());
                    XmlManager.SetAttribute(ndList[0], experimentFile, "frameRateControlValue", this.FrameRateControlValue.ToString());
                    XmlManager.SetAttribute(ndList[0], experimentFile, "OrcaFrameRateEnabled", Convert.ToInt32(this.OrcaFrameRateEnabled).ToString());
                    XmlManager.SetAttribute(ndList[0], experimentFile, "OrcaFrameRateValue", this.CamOrcaFrameRate.ToString());
                    XmlManager.SetAttribute(ndList[0], experimentFile, "CoolingModeIndex", this.CoolingModeIndex.ToString());
                    XmlManager.SetAttribute(ndList[0], experimentFile, "colorImageTypeIndex", this.CameraColorImageTypeIndex.ToString());
                    XmlManager.SetAttribute(ndList[0], experimentFile, "polarImageTypeIndex", this.CameraPolarImageTypeIndex.ToString());
                    XmlManager.SetAttribute(ndList[0], experimentFile, "channel", _cameraControlModel.ChannelNum.ToString());
                    XmlManager.SetAttribute(ndList[0], experimentFile, "isNirBoostEnabled", this.IsNirBoostEnabled.ToString());
                    if (_cameraControlModel.IsColorGainsSupported)
                    {
                        // only update the RGB gains if they are supported, otherwise '0' gets written in for these values
                        XmlManager.SetAttribute(ndList[0], experimentFile, "redGain", _cameraControlModel.RedGain.ToString());
                        XmlManager.SetAttribute(ndList[0], experimentFile, "greenGain", _cameraControlModel.GreenGain.ToString());
                        XmlManager.SetAttribute(ndList[0], experimentFile, "blueGain", _cameraControlModel.BlueGain.ToString());
                    }
                    XmlManager.SetAttribute(ndList[0], experimentFile, "isEepEnabled", this.IsEqualExposurePulseEnabled.ToString());
                    XmlManager.SetAttribute(ndList[0], experimentFile, "eepPulseWidth", this.EqualExposurePulseWidth.ToString());
                    XmlManager.SetAttribute(ndList[0], experimentFile, "isContinuousWhiteBalanceEnabled", this.IsContinuousWhiteBalanceEnabled.ToString());
                    XmlManager.SetAttribute(ndList[0], experimentFile, "continuousWhiteBalanceNumFrames", this.ContinuousWhiteBalanceNumFrames.ToString());

                    Decimal decX = new Decimal((this.Right - this.Left) * this.CamPixelSizeUM);
                    Decimal decY = new Decimal((this.Bottom - this.Top) * this.CamPixelSizeUM);
                    double dValX = Decimal.ToDouble(Decimal.Round(decX, 4));
                    double dValY = Decimal.ToDouble(Decimal.Round(decY, 4));
                    if (0 != CameraImageAngle && 180 != CameraImageAngle)
                    {
                        XmlManager.SetAttribute(ndList[0], experimentFile, "widthUM", dValY.ToString());
                        XmlManager.SetAttribute(ndList[0], experimentFile, "heightUM", dValX.ToString());
                    }
                    else
                    {
                        XmlManager.SetAttribute(ndList[0], experimentFile, "widthUM", dValX.ToString());
                        XmlManager.SetAttribute(ndList[0], experimentFile, "heightUM", dValY.ToString());
                    }
                }

                if ((int)ICamera.CameraType.CCD == ResourceManagerCS.GetCameraType())
                {
                    XmlNodeList wavelengthsNodeList = experimentFile.SelectNodes("/ThorImageExperiment/Wavelengths");

                    if (wavelengthsNodeList.Count > 0)
                    {
                        wavelengthsNodeList[0].RemoveAll();
                    }

                    wavelengthsNodeList = experimentFile.SelectNodes("/ThorImageExperiment/Wavelengths");

                    if (wavelengthsNodeList.Count > 0)
                    {
                        XmlNodeList waveList = hwDoc.SelectNodes("/HardwareSettings/Wavelength");
                        int cameraChannels = ChannelNum;
                        
                        XmlManager.SetAttribute(ndList[0], experimentFile, "nyquistExWavelengthNM", MVMManager.Instance["AreaControlViewModel", "NyquistExWavelength", (object)string.Empty].ToString());
                        XmlManager.SetAttribute(ndList[0], experimentFile, "nyquistEmWavelengthNM", MVMManager.Instance["AreaControlViewModel", "NyquistEmWavelength", (object)string.Empty].ToString());

                        for (int i = 0; i < waveList.Count; i++)
                        {
                            if (0 != (cameraChannels & (1 << i)))
                            {
                                XmlElement newWavelengthElement = experimentFile.CreateElement("Wavelength");
                                XmlAttribute nameAttribute = experimentFile.CreateAttribute("name");
                                XmlAttribute expAttribute = experimentFile.CreateAttribute("exposureTimeMS");
                                nameAttribute.Value = waveList[i].Attributes["name"].Value;
                                expAttribute.Value = this.ExposureTimeCam.ToString();
                                _ = newWavelengthElement.Attributes.Append(nameAttribute);
                                _ = newWavelengthElement.Attributes.Append(expAttribute);
                                _ = wavelengthsNodeList[0].AppendChild(newWavelengthElement);
                            }
                        }
                    }
                }
            }
            //Invoked at switching modalities
            ReconnectCameraCommand.Execute(null);
        }

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "SetupCommand")]
        private static extern int CSSetupCommand();

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "TeardownCommand")]
        private static extern int CSTeardownCommand();

        private void BinXMinus()
        {
            this.BinX -= 1;
        }

        private void BinXPlus()
        {
            this.BinX += 1;
        }

        private void BinYMinus()
        {
            this.BinY -= 1;
        }

        private void BinYPlus()
        {
            this.BinY += 1;
        }

        private void CamAddResolution()
        {
            //check if a similar setting already exists
            XmlDocument appDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
            XmlNodeList ndList = appDoc.SelectNodes(string.Format("/ApplicationSettings/DisplayOptions/CaptureSetup/CameraView/ResolutionPresets/Resolution[@top='{0}' and @left='{1}' and @bottom='{2}' and @right='{3}']", Top, Left, Bottom, Right));

            if (ndList.Count > 0)
            {
                return;
            }

            if (MessageBoxResult.Yes == MessageBox.Show(string.Format("Do you want to add the resolution T: {0} L: {1} B: {2} R: {3}", Top, Left, Bottom, Right), "Add Resolution", MessageBoxButton.YesNo))
            {
                ndList = appDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/CameraView/ResolutionPresets");

                if (ndList.Count > 0)
                {
                    XmlNode node = appDoc.CreateNode(XmlNodeType.Element, "Resolution", null);
                    XmlManager.SetAttribute(node, appDoc, "top", Top.ToString());
                    XmlManager.SetAttribute(node, appDoc, "left", Left.ToString());
                    XmlManager.SetAttribute(node, appDoc, "bottom", Bottom.ToString());
                    XmlManager.SetAttribute(node, appDoc, "right", Right.ToString());

                    ndList[ndList.Count - 1].AppendChild(node);

                    MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS);

                    OnPropertyChanged("CamResolutionPresets");
                }
            }
        }

        private void CamAverageNumMinus()
        {
            this.CamAverageNum -= 1;
        }

        private void CamAverageNumPlus()
        {
            this.CamAverageNum += 1;
        }

        private void CameraImageAngleMinus()
        {
            this.CameraImageAngle -= 90;
        }

        private void CameraImageAnglePlus()
        {
            this.CameraImageAngle += 90;
        }

        private void CamExposureTimeMinus()
        {
            this.ExposureTimeCam -= 1;
        }

        private void CamExposureTimePlus()
        {
            this.ExposureTimeCam += 1;
        }

        private void CamFullFrame()
        {
            this.Top = this.TopMin;
            this.Left = this.LeftMin;
            this.Bottom = this.BottomMax;
            this.Right = this.RightMax;
        }

        private void CamRegionFromROI()
        {
            List<Point> points = new List<Point>();
            OverlayManagerClass.ROIType roiType = 0;
            OverlayManagerClass.Instance.QueryTheLastROIRange(ref points, ref roiType);
            switch (roiType)
            {
                case OverlayManagerClass.ROIType.RECTANGLE:
                    if (points.Count == 2)
                    {
                        int x1, x2, y1, y2;
                        int w = CamImageWidth;
                        int h = CamImageHeight;

                        //take the image angle into consideration
                        switch (this.CameraImageAngle)
                        {
                            case 0:
                                x1 = (int)points[0].X;
                                x2 = (int)points[1].X;
                                y1 = (int)points[0].Y;
                                y2 = (int)points[1].Y;
                                break;

                            case 90:
                                x1 = w - (int)points[1].Y;
                                x2 = w - (int)points[0].Y;
                                y1 = (int)points[0].X;
                                y2 = (int)points[1].X;
                                break;

                            case 180:
                                x1 = w - (int)points[1].X;
                                x2 = w - (int)points[0].X;
                                y1 = h - (int)points[1].Y;
                                y2 = h - (int)points[0].Y;
                                break;

                            case 270:
                                x1 = (int)points[0].Y;
                                x2 = (int)points[1].Y;
                                y1 = h - (int)points[1].X;
                                y2 = h - (int)points[0].X;
                                break;

                            default:
                                x1 = (int)points[0].X;
                                x2 = (int)points[1].X;
                                y1 = (int)points[0].Y;
                                y2 = (int)points[1].Y;
                                break;
                        }

                        //After the angle is taken care of, then transform for the flip
                        if (1 == this.CamVerticalFlip && 1 == this.CamHorizontalFlip)
                        {
                            points[0] = new Point(w - x2, h - y2);
                            points[1] = new Point(w - x1, h - y1);
                            x1 = (int)points[0].X;
                            x2 = (int)points[1].X;
                            y1 = (int)points[0].Y;
                            y2 = (int)points[1].Y;
                        }
                        else if (1 == this.CamVerticalFlip)
                        {
                            points[0] = new Point(x1, h - y2);
                            points[1] = new Point(x2, h - y1);
                            x1 = (int)points[0].X;
                            x2 = (int)points[1].X;
                            y1 = (int)points[0].Y;
                            y2 = (int)points[1].Y;
                        }
                        else if (1 == this.CamHorizontalFlip)
                        {
                            points[0] = new Point(w - x2, y1);
                            points[1] = new Point(w - x1, y2);
                            x1 = (int)points[0].X;
                            x2 = (int)points[1].X;
                            y1 = (int)points[0].Y;
                            y2 = (int)points[1].Y;
                        }

                        //don't allow 0 pixel images and don't go past max width and height
                        int binningX = (BinX == 0) ? 1 : BinX;
                        int binningY = (BinY == 0) ? 1 : BinY;
                        if (x1 == x2 && x2 < this.RightMax / binningX)
                        {
                            x2 += 1;
                        }
                        else if (x1 == x2)
                        {
                            x1 -= 1;
                        }

                        if (x1 == x2 && x2 < this.RightMax / binningX)
                        {
                            x2 += 1;
                        }
                        else if (x1 == x2)
                        {
                            x1 -= 1;
                        }

                        this.Top = this.Top + y1 * binningY;
                        this.Left = this.Left + x1 * binningX;
                        this.Bottom = this.Top + (y2 - y1) * binningY;
                        this.Right = this.Left + (x2 - x1) * binningX;

                        ((ICommand)MVMManager.Instance["CaptureSetupViewModel", "ClearaAllObjectsCommand", (object)new RelayCommand(() => { })]).Execute(null);
                    }
                    break;
            }
        }

        private void ContinuousWhiteBalanceNumFramesDecrement()
        {
            this.ContinuousWhiteBalanceNumFrames -= 1;
        }

        private void ContinuousWhiteBalanceNumFramesIncrement()
        {
            this.ContinuousWhiteBalanceNumFrames += 1;
        }

        private void CreateAutoexposureSpinnerControlsIfNull()
        {
            if (null == _autoexposureSpinnerCancelControl)
            {
                _autoexposureSpinnerCancelControl = new SpinnerProgressCancel
                {
                    Width = 350,
                    Height = 390,
                    CancelButtonWidth = 150,
                    CancelButtonHeight = 40,
                    CancelButtonBackground = Brushes.Red,
                    CancelButtonForeground = Brushes.White,
                    CancelButtonContent = "Stop Auto Exposure",
                    LoadingText = "Starting Auto Exposure",
                    ProgressVisible = Visibility.Collapsed,
                };
                _autoexposureSpinnerCancelControl.CancelSplashProgress += new EventHandler(OnAutoexposureSpinnerClosed);
            }

            if (null == _autoexposureSpinnerProgressWindow)
            {
                _autoexposureSpinnerProgressWindow = new SpinnerProgressWindow();
                _autoexposureSpinnerProgressWindow = new SpinnerProgressWindow();
                _autoexposureSpinnerProgressWindow.Title = "Auto Exposure Running";
                _autoexposureSpinnerProgressWindow.ResizeMode = ResizeMode.NoResize;
                _autoexposureSpinnerProgressWindow.Width = 350;
                _autoexposureSpinnerProgressWindow.Height = 390;
                _autoexposureSpinnerProgressWindow.WindowStyle = WindowStyle.SingleBorderWindow;
                _autoexposureSpinnerProgressWindow.Background = Brushes.DimGray;
                _autoexposureSpinnerProgressWindow.AllowsTransparency = false;

                _autoexposureSpinnerProgressWindow.Content = _autoexposureSpinnerCancelControl;

                _autoexposureSpinnerProgressWindow.Owner = Application.Current.MainWindow;
                _autoexposureSpinnerProgressWindow.Left = _autoexposureSpinnerProgressWindow.Owner.Left + _autoexposureSpinnerProgressWindow.Width;
                _autoexposureSpinnerProgressWindow.Top = _autoexposureSpinnerProgressWindow.Owner.Top + ((System.Windows.Controls.Panel)_autoexposureSpinnerProgressWindow.Owner.Content).ActualHeight / 2;
                _autoexposureSpinnerProgressWindow.Closed += new EventHandler(OnAutoexposureSpinnerClosed);
            }
        }

        private void FrameRateControlMinus()
        {
            this.FrameRateControlValue -= 1;
        }

        private void FrameRateControlPlus()
        {
            this.FrameRateControlValue += 1;
        }

        private void HotPixelMinus()
        {
            this.HotPixelVal -= 1;
        }

        private void HotPixelPlus()
        {
            this.HotPixelVal += 1;
        }

        void OnAutoexposureSpinnerClosed(object sender, EventArgs args)
        {
            StopAutoExposure();
        }

        private void OnAutoExposureUpdated(object sender, AutoExposureEventArgs args)
        {
            OnPropertyChange("ExposureTimeCam");

            if (false == args.IsRunning && true == _isAutoExposureRunning)
            {
                // just stopped running
                _isAutoExposureRunning = false;
                MVMManager.Instance["CaptureSetupViewModel", "WrapPanelEnabled"] = true;
                OnPropertyChanged("IsCameraControlEnabled");
                Application.Current.Dispatcher.Invoke(new Action(() =>
                {
                    _autoexposureSpinnerProgressWindow.Close();
                    _autoexposureSpinnerProgressWindow = null;
                }));
            }
            else if (true == args.IsRunning && false == _isAutoExposureRunning)
            {
                // just started running
                _isAutoExposureRunning = true;
                Application.Current.Dispatcher.Invoke(new Action(() =>
                {
                    CreateAutoexposureSpinnerControlsIfNull();
                    _autoexposureSpinnerProgressWindow.Show();
                }));
                OnPropertyChanged("IsCameraControlEnabled");
                MVMManager.Instance["CaptureSetupViewModel", "WrapPanelEnabled"] = false; // TODO: refactoring needed in capture setup. if AreaControl or AutoFocus are used at the same time as AutoExposure, one could easily enable these buttons when it should be disabled
            }

            if (true == args.IsStable && true == _isAutoExposureRunning)
            {
                StopAutoExposure();
            }
        }

        private void PerformOneShotWhiteBalance()
        {
            _cameraControlModel.PerformOneShotWhiteBalance();
            OnPropertyChanged("RedGain");
            OnPropertyChanged("GreenGain");
            OnPropertyChanged("BlueGain");
        }

        private void PulseWidthMinus()
        {
            this.EqualExposurePulseWidth -= 1;
        }

        private void PulseWidthPlus()
        {
            this.EqualExposurePulseWidth += 1;
        }

        private void RunAutoExposure()
        {
            _cameraControlModel.StartAutoExposure();
        }

        private void SetDefaultColorGains()
        {
            _cameraControlModel.SetDefaultColorGains();
            OnPropertyChanged("RedGain");
            OnPropertyChanged("GreenGain");
            OnPropertyChanged("BlueGain");
        }

        private void StopAutoExposure()
        {
            _cameraControlModel.StopAutoExposure();
        }

        #endregion Methods
    }
}