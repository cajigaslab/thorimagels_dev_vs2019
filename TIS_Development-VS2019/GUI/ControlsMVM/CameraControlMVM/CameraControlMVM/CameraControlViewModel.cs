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
    using System.Xml;

    using CameraControl.Model;

    using OverlayManager;

    using ThorLogging;

    using ThorSharedTypes;

    public class CameraControlViewModel : ThorSharedTypes.VMBase, ThorSharedTypes.IMVM
    {
        #region Fields

        private readonly CameraControlModel _cameraControlModel;

        ObservableCollection<string> _binList;
        ICommand _binXMinusCommand;
        ICommand _binXPlusCommand;
        ICommand _binYMinusCommand;
        ICommand _binYPlusCommand;
        ICommand _camAddResolutionCommand;
        ICommand _camAverageNumMinusCommand;
        ICommand _camAverageNumPlusCommand;
        ICommand _cameraImageAngleMinusCommand;
        ICommand _cameraImageAnglePlusCommand;
        ICommand _camExposureTimeMinusCommand;
        ICommand _camExposureTimePlusCommand;
        ICommand _camFullFrameCommand;
        ObservableCollection<string> _camReadoutSpeedList;
        ICommand _camRegionFromROICommand;
        ObservableCollection<string> _coolingModeList;
        ICommand _frameRateControlMinusCommand;
        ICommand _frameRateControlPlusCommand;
        ObservableCollection<string> _hotPixelLevelList;
        ICommand _hotPixelMinusCommand;
        ICommand _hotPixelPlusCommand;
        private bool _liveButtonStatus = true;
        private Dictionary<string, PropertyInfo> _properties = new Dictionary<string, PropertyInfo>();
        private ICommand _reconnectCameraCommand;

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
                return ((ICommand)MVMManager.Instance["CaptureSetupViewModel", "BrowseForReferenceImageCommand", (object)new RelayCommand(() => { })]);
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
                OnPropertyChanged("IsBlackLevelVisible");
                OnPropertyChanged("IsReadoutVisible");
                OnPropertyChanged("IsTapsVisible");
                OnPropertyChanged("ExposureTimeCam");
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
                    _camReadoutSpeedList.Add("60 MS/S");
                    _camReadoutSpeedList.Add("100 MS/S");
                }
                else if (ActiveCameraName.Contains("C14440"))
                {
                    _camReadoutSpeedList.Add("Ultra Quiet");
                    _camReadoutSpeedList.Add("Standard");
                    _camReadoutSpeedList.Add("Fast");
                }
                else if (ActiveCameraName.Contains("C13440"))
                {
                    _camReadoutSpeedList.Add("1");
                    _camReadoutSpeedList.Add("2");
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
                if ((int)ICamera.CameraType.LSM != ResourceManagerCS.GetCameraType())
                {
                    //Change the visible channels in the ImageView within CaptureSetup when checked
                    if (value)
                    {
                        MVMManager.Instance["CaptureSetupViewModel", "IsChannelVisible3", Visibility.Hidden] = Visibility.Visible;
                        MVMManager.Instance["CaptureSetupViewModel", "LSMChannelEnable3", false] = true;
                    }
                    else
                    {
                        MVMManager.Instance["CaptureSetupViewModel", "LSMChannelEnable3", false] = false;
                        MVMManager.Instance["CaptureSetupViewModel", "IsChannelVisible3", Visibility.Hidden] = Visibility.Collapsed;
                    }
                    _cameraControlModel.EnableReferenceChannel = value;
                    OnPropertyChange("EnableReferenceChannel");
                }
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

        public bool IsBlackLevelVisible
        {
            get
            {
                return (false == ActiveCameraName.Contains("CS2100") && false == ActiveCameraName.Contains("CS135") && (int)ICamera.CCDType.ORCA != ResourceManagerCS.GetCCDType());
            }
        }

        public bool IsFrameRateVisible
        {
            get
            {
                return ((int)ICamera.CCDType.ORCA == ResourceManagerCS.GetCCDType());
            }
        }

        public bool IsGainVisible
        {
            get
            {
                return (false == ActiveCameraName.Contains("CS2100") && (int)ICamera.CCDType.ORCA != ResourceManagerCS.GetCCDType());
            }
        }

        public bool IsReadoutVisible
        {
            get
            {
                //Make it visible if the camera is not a CMOS (all CCD) or if it is the CS2100, or if the camera is an Orca and the CS14440 (EMCCD doesn't have readoutspeed)
                return ((!CameraCSType || ActiveCameraName.Contains("CS2100")) || ((int)ICamera.CCDType.ORCA == ResourceManagerCS.GetCCDType() && ActiveCameraName.Contains("C14440")));
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

        public ICommand ReconnectCameraCommand
        {
            get
            {
                if (_reconnectCameraCommand == null)
                    _reconnectCameraCommand = new RelayCommand(() => ReconnectCamera());

                return _reconnectCameraCommand;
            }
        }

        public string ReferenceChannelImageName
        {
            get
            {
                return MVMManager.Instance["CaptureSetupViewModel", "ReferenceChannelImageName", ""].ToString();
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

            EnableReferenceChannel = false;

            //disconnect the camera
            CSTeardownCommand();

            //setup with the new camera
            CSSetupCommand();
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
                    XmlManager.SetAttribute(ndList[0], experimentFile, "pixelSizeUM", (CamPixelSizeUM * Math.Max(1,Math.Max(BinY, BinX))).ToString());
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

        #endregion Methods
    }
}