namespace CaptureSetupDll.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Windows.Data;

    using ThorSharedTypes;

    #region Enumerations

    public enum CameraType
    {
        CCD = 0,
        LSM = 1,
        CCD_MOSAIC = 2
    }

    #endregion Enumerations

    public interface ICameraProps
    {
        #region Properties

        int BinX
        {
            get;
            set;
        }

        int BinY
        {
            get;
            set;
        }

        int Bottom
        {
            get;
            set;
        }

        int CameraHeight
        {
            get;
        }

        int CameraWidth
        {
            get;
        }

        bool CoolingModeSupported
        {
            get;
        }

        int CoolingMode
        {
            get;
            set;
        }

        double ExposureTimeCam0
        {
            get;
            set;
        }

        double ExposureTimeMax
        {
            get;
        }

        double ExposureTimeMin
        {
            get;
        }

        int FrameCount
        {
            get;
            set;
        }

        int Gain
        {
            get;
            set;
        }

        int GainMax
        {
            get;
        }

        int GainMin
        {
            get;
        }

        int Left
        {
            get;
            set;
        }

        int LightMode
        {
            get;
             set;
        }

        int LightModeMax
        {
            get;
        }

        int LightModeMin
        {
            get;
        }

        bool NIRBoostSupported
        {
            get;
        }

        int NIRBoost
        {
            get;
            set;
        }

        int OperatingMode
        {
            get;
            set;
        }

        int OpticalBlackLevel
        {
            get;
            set;
        }

        int OpticalBlackLevelMax
        {
            get;
        }

        int OpticalBlackLevelMin
        {
            get;
        }

        double PixelSizeUM
        {
            get;
        }

        CollectionView ReadOutSpeedEntries
        {
            get;
        }

        int ReadOutSpeedIndex
        {
            get;
            set;
        }

        int ReadOutSpeedIndexMax
        {
            get;
        }

        int ReadOutSpeedIndexMin
        {
            get;
        }

        double ReadOutSpeedValue
        {
            get;
        }

        CollectionView ReadOutTapEntries
        {
            get;
        }

        int ReadOutTapIndex
        {
            get;
            set;
        }

        int ReadOutTapIndexMax
        {
            get;
        }

        int ReadOutTapIndexMin
        {
            get;
        }

        int ReadOutTapValue
        {
            get;
        }

        int Right
        {
            get;
            set;
        }

        int TapBalance
        {
            get;
            set;
        }

        int TDILineShifts
        {
            get;
            set;
        }

        int TDILineTrim
        {
            get;
            set;
        }

        int TDITriggers
        {
            get;
            set;
        }

        int TDITrimMode
        {
            get;
            set;
        }

        int Top
        {
            get;set;
        }

        #endregion Properties
    }

    public class ConcreteCameraProps : ICameraProps
    {
        #region Fields

        private const int UNINITIALIZED = -1;

        private double _exposureTimeCam0;
        private double _exposureTimeMax = UNINITIALIZED;
        private double _exposureTimeMin = UNINITIALIZED;
        private int _gain;
        private int _gainMax = UNINITIALIZED;
        private int _gainMin = UNINITIALIZED;
        private int _opticalBlackLevel;
        private int _opticalBlackLevelMax = UNINITIALIZED;
        private int _opticalBlackLevelMin = UNINITIALIZED;
        private int _readoutSpeedIndex;
        private int _readoutSpeedIndexMax = UNINITIALIZED;
        private int _readoutSpeedIndexMin = UNINITIALIZED;
        private IList<double> _readoutSpeedList;
        private IList<int> _readoutTapList;

        #endregion Fields

        #region Properties

        public int BinX
        {
            get
            {
                int binningX = 0;
                GetCameraParameterValueInt((int)ICamera.Params.PARAM_BINNING_X, ref binningX);
                return binningX;
            }
            set
            {
                SetCameraParameterValueDouble((int)ICamera.Params.PARAM_BINNING_X,(double)value);
            }
        }

        public int BinY
        {
            get
            {
                int binningY = 0;
                GetCameraParameterValueInt((int)ICamera.Params.PARAM_BINNING_Y, ref binningY);
                return binningY;
            }
            set
            {
                SetCameraParameterValueDouble((int)ICamera.Params.PARAM_BINNING_Y,(double)value);
            }
        }

        public int Bottom
        {
            get
            {
                int bottom = 0;
                GetCameraParameterValueInt((int)ICamera.Params.PARAM_CAPTURE_REGION_BOTTOM, ref bottom);
                return bottom;
            }
            set
            {
                SetCameraParameterValueDouble((int)ICamera.Params.PARAM_CAPTURE_REGION_BOTTOM,(double)value);
            }
        }

        public int CameraHeight
        {
            get
            {
                int height = 0;
                GetCameraHeight(ref height);
                return height;
            }
        }

        public int CameraWidth
        {
            get
            {
                int width = 0;
                GetCameraWidth(ref width);
                return width;
            }
        }


        public bool CoolingModeSupported
        {
            get
            {
                int coolingModeMax = UNINITIALIZED;
                int coolingModeMin = UNINITIALIZED;
                int coolingModeDef = UNINITIALIZED;

                GetCameraParameterRangeInt((int)ICamera.Params.PARAM_COOLING_MODE, ref coolingModeMin, ref coolingModeMax, ref coolingModeDef);

                return coolingModeMin != coolingModeMax;
            }
        }

        public int CoolingMode
        {
            get
            {
                int coolingMode = 0;
                GetCameraParameterValueInt((int)ICamera.Params.PARAM_COOLING_MODE, ref coolingMode);
                return coolingMode;
            }
            set
            {
                SetCameraParameterValueDouble((int)ICamera.Params.PARAM_COOLING_MODE, (double)value);
            }
        }

        public double ExposureTimeCam0
        {
            get
            {
                double expoTime = 0;

                // read min and max from camera if they are not initialized
                GetCameraParameterRangeDouble((int)ICamera.Params.PARAM_EXPOSURE_TIME_MS, ref _exposureTimeMin, ref _exposureTimeMax, ref expoTime);

                GetCameraParameterValueDouble((int)ICamera.Params.PARAM_EXPOSURE_TIME_MS, ref expoTime);

                if((expoTime >= _exposureTimeMin) && (expoTime <= _exposureTimeMax))
                {
                    _exposureTimeCam0 = expoTime;
                }

                return _exposureTimeCam0;
            }
            set
            {
                if ((value >= _exposureTimeMin) && (value <= _exposureTimeMax))
                {
                    SetCameraParameterValueDouble((int)ICamera.Params.PARAM_EXPOSURE_TIME_MS, value);
                    _exposureTimeCam0 = value;
                }
            }
        }

        public double ExposureTimeMax
        {
            get
            {
                // set the maximux exposure time only once
                if(_exposureTimeMax == (double) UNINITIALIZED)
                {
                    double exMin = 0;
                    double exDefault = 0;
                    GetCameraParameterRangeDouble((int)ICamera.Params.PARAM_EXPOSURE_TIME_MS, ref exMin, ref _exposureTimeMax, ref exDefault);
                }

                return _exposureTimeMax;
            }
        }

        public double ExposureTimeMin
        {
            get
            {
                if (_exposureTimeMin == (double) UNINITIALIZED)
                {
                    double exMax = 0;
                    double exDefault = 0;
                    GetCameraParameterRangeDouble((int)ICamera.Params.PARAM_EXPOSURE_TIME_MS, ref _exposureTimeMin, ref exMax, ref exDefault);
                }

                return _exposureTimeMin;
            }
        }

        public int FrameCount
        {
            get
            {
                int frameCount = 0;
                GetCameraParameterValueInt((int)ICamera.Params.PARAM_MULTI_FRAME_COUNT, ref frameCount);
                return frameCount;
            }
            set
            {
                SetCameraParameterValueDouble((int)ICamera.Params.PARAM_MULTI_FRAME_COUNT,(double)value);
            }
        }

        public int Gain
        {
            get
            {
                int gain = 0;

                // read min max from camera if they are not initialized
                GetCameraParameterRangeInt((int)ICamera.Params.PARAM_GAIN, ref _gainMin, ref _gainMax, ref gain);

                GetCameraParameterValueInt((int)ICamera.Params.PARAM_GAIN, ref gain);

                if ((gain >= _gainMin) && (gain <= _gainMax))
                {
                    _gain = gain;
                }

                return _gain;
            }
            set
            {
                if ((value >= _gainMin) && (value <= _gainMax))
                {
                    SetCameraParameterValueDouble((int)ICamera.Params.PARAM_GAIN, (double)value);
                    _gain = value;
                }
            }
        }

        public int GainMax
        {
            get
            {
                if(_gainMax == UNINITIALIZED)
                {
                    int exMin = 0;
                    int exDefault = 0;
                    GetCameraParameterRangeInt((int)ICamera.Params.PARAM_GAIN, ref exMin, ref _gainMax, ref exDefault);
                }
                return _gainMax;
            }
        }

        public int GainMin
        {
            get
            {
                if (_gainMin == UNINITIALIZED)
                {
                    int exMax = 0;
                    int exDefault = 0;
                    GetCameraParameterRangeInt((int)ICamera.Params.PARAM_GAIN, ref _gainMin, ref exMax, ref exDefault);
                }
                return _gainMin;
            }
        }

        public int Left
        {
            get
            {
                int left = 0;
                GetCameraParameterValueInt((int)ICamera.Params.PARAM_CAPTURE_REGION_LEFT, ref left);
                return left;
            }
            set
            {
                SetCameraParameterValueDouble((int)ICamera.Params.PARAM_CAPTURE_REGION_LEFT,(double)value);
            }
        }

        public int LightMode
        {
            get
            {
                int lightMode = 0;
                GetCameraParameterValueInt((int)ICamera.Params.PARAM_LIGHT_MODE, ref lightMode);
                return lightMode;
            }
            set
            {
                SetCameraParameterValueDouble((int)ICamera.Params.PARAM_LIGHT_MODE,(double)value);
            }
        }

        public int LightModeMax
        {
            get
            {
                int exMin = 0;
                int exMax = 0;
                int exDefault = 0;

                if (GetCameraParameterRangeInt((int)ICamera.Params.PARAM_LIGHT_MODE, ref exMin, ref exMax, ref exDefault))
                {
                }
                return exMax;
            }
        }

        public int LightModeMin
        {
            get
            {
                int exMin = 0;
                int exMax = 0;
                int exDefault = 0;

                if (GetCameraParameterRangeInt((int)ICamera.Params.PARAM_LIGHT_MODE, ref exMin, ref exMax, ref exDefault))
                {
                }
                return exMin;
            }
        }

        public bool NIRBoostSupported
        {
            get
            {
                int boostMin = UNINITIALIZED;
                int boostMax = UNINITIALIZED;
                int boostDef = UNINITIALIZED;

                GetCameraParameterRangeInt((int)ICamera.Params.PARAM_NIR_BOOST, ref boostMin, ref boostMax, ref boostDef);

                return boostMin != boostMax;
            }
        }

        public int NIRBoost
        {
            get
            {
                int boost = 0;
                GetCameraParameterValueInt((int)ICamera.Params.PARAM_NIR_BOOST, ref boost);
                return boost;
            }
            set
            {
                SetCameraParameterValueDouble((int)ICamera.Params.PARAM_NIR_BOOST, (double)value);
            }
        }

        public int OperatingMode
        {
            get
            {
                int opMode = 0;
                GetCameraParameterValueInt((int)ICamera.Params.PARAM_OP_MODE, ref opMode);
                return opMode;
            }
            set
            {
                SetCameraParameterValueDouble((int)ICamera.Params.PARAM_OP_MODE,(double)value);
            }
        }

        public int OpticalBlackLevel
        {
            get
            {
                int opticalBlack = 0;

                // read the min max values from camera if they have not been initialized
                GetCameraParameterRangeInt((int)ICamera.Params.PARAM_OPTICAL_BLACK_LEVEL, ref _opticalBlackLevelMin, ref _opticalBlackLevelMax, ref opticalBlack);

                GetCameraParameterValueInt((int)ICamera.Params.PARAM_OPTICAL_BLACK_LEVEL, ref opticalBlack);

                if ((opticalBlack >= _opticalBlackLevelMin) && (opticalBlack <= _opticalBlackLevelMax))
                {
                    _opticalBlackLevel = opticalBlack;
                }

                return _opticalBlackLevel;
            }
            set
            {
                if ((value >= _opticalBlackLevelMin) && (value <= _opticalBlackLevelMax))
                {
                    SetCameraParameterValueDouble((int)ICamera.Params.PARAM_OPTICAL_BLACK_LEVEL, (double)value);
                    _opticalBlackLevel = value;
                }
            }
        }

        public int OpticalBlackLevelMax
        {
            get
            {
                if (_opticalBlackLevelMax == UNINITIALIZED)
                {
                    int blackMin = 0;
                    int blackDefault = 0;
                    GetCameraParameterRangeInt((int)ICamera.Params.PARAM_OPTICAL_BLACK_LEVEL, ref blackMin, ref _opticalBlackLevelMax, ref blackDefault);
                }
                return _opticalBlackLevelMax;
            }
        }

        public int OpticalBlackLevelMin
        {
            get
            {
                if (_opticalBlackLevelMin == UNINITIALIZED)
                {
                    int blackMax = 0;
                    int blackDefault = 0;
                    GetCameraParameterRangeInt((int)ICamera.Params.PARAM_OPTICAL_BLACK_LEVEL, ref _opticalBlackLevelMin, ref blackMax, ref blackDefault);
                }
                return _opticalBlackLevelMin;
            }
        }

        public double PixelSizeUM
        {
            get
            {
                double pixSize = 0;
                GetCameraParameterValueDouble((int)ICamera.Params.PARAM_PIXEL_SIZE, ref pixSize);
                return pixSize;
            }
        }

        public CollectionView ReadOutSpeedEntries
        {
            get
            {
                //build the collection which the Readout Speed ComboBox binds to
                if (_readoutSpeedList == null)
                {
                    _readoutSpeedList = new List<double>();
                }
                else
                {
                    _readoutSpeedList.Clear();
                }

                int iDef = 0;
                GetCameraParameterRangeInt((int)ICamera.Params.PARAM_READOUT_SPEED_INDEX, ref _readoutSpeedIndexMin, ref _readoutSpeedIndexMax, ref iDef);
                
                for (int i = _readoutSpeedIndexMin; i <= _readoutSpeedIndexMax; i++)
                {
                    ReadOutSpeedIndex = i;
                    _readoutSpeedList.Add(ReadOutSpeedValue / 1000000); //from Hz to MHz
                }

                return new CollectionView(_readoutSpeedList);
            }
        }

        public int ReadOutSpeedIndex
        {
            get
            {
                int readOutSpeedIndex = 0;

                GetCameraParameterRangeInt((int)ICamera.Params.PARAM_READOUT_SPEED_INDEX, ref _readoutSpeedIndexMin, ref _readoutSpeedIndexMax, ref readOutSpeedIndex);

                GetCameraParameterValueInt((int)ICamera.Params.PARAM_READOUT_SPEED_INDEX,ref readOutSpeedIndex);

                // if the value read back from the camera is out of range, it means the camera is disconnected or dead, return the value in UI
                if ((readOutSpeedIndex >= _readoutSpeedIndexMin) && (readOutSpeedIndex <= _readoutSpeedIndexMax))
                {
                    _readoutSpeedIndex = readOutSpeedIndex;
                }

                return _readoutSpeedIndex;
            }
            set
            {
                if ((value >= _readoutSpeedIndexMin) || (value <= _readoutSpeedIndexMax))
                {
                    SetCameraParameterValueDouble((int)ICamera.Params.PARAM_READOUT_SPEED_INDEX, (double)value);
                    _readoutSpeedIndex = value;
                }
            }
        }

        public int ReadOutSpeedIndexMax
        {
            get
            {
                if (_readoutSpeedIndexMax == UNINITIALIZED)
                {
                    int indexMin = 0;
                    int indexDefault = 0;
                    GetCameraParameterRangeInt((int)ICamera.Params.PARAM_TAP_INDEX, ref indexMin, ref _readoutSpeedIndexMax, ref indexDefault);
                }
                return _readoutSpeedIndexMax;
            }
        }

        public int ReadOutSpeedIndexMin
        {
            get
            {
                if (_readoutSpeedIndexMin == UNINITIALIZED)
                {
                    int indexMax = 0;
                    int indexDefault = 0;
                    GetCameraParameterRangeInt((int)ICamera.Params.PARAM_TAP_INDEX, ref _readoutSpeedIndexMin, ref indexMax, ref indexDefault);
                }
                return _readoutSpeedIndexMin;
            }
        }

        public double ReadOutSpeedValue
        {
            get
            {
                double readOutSpeedValue = 0;
                GetCameraParameterValueDouble((int)ICamera.Params.PARAM_READOUT_SPEED_VALUE,ref readOutSpeedValue);
                return readOutSpeedValue;
            }
        }

        public CollectionView ReadOutTapEntries
        {
            get
            {
                //build the collection which the Readout Tap ComboBox binds to
                if (_readoutTapList == null)
                {
                    _readoutTapList = new List<int>();
                }
                else
                {
                    _readoutTapList.Clear();
                }

                int iMin = 0, iMax = 0, iDefault = 0;
                GetCameraParameterRangeInt((int)ICamera.Params.PARAM_TAP_INDEX, ref iMin, ref iMax, ref iDefault);
                for (int i = iMin; i <= iMax; i++)
                {
                    ReadOutTapIndex = i;
                    _readoutTapList.Add(ReadOutTapValue);
                }
                return new CollectionView(_readoutTapList);
            }
        }

        public int ReadOutTapIndex
        {
            get
            {
                int readOutTapIndex = 0;
                GetCameraParameterValueInt((int)ICamera.Params.PARAM_TAP_INDEX, ref readOutTapIndex);
                return readOutTapIndex;
            }
            set
            {
                SetCameraParameterValueInt((int)ICamera.Params.PARAM_TAP_INDEX, value);
            }
        }

        public int ReadOutTapIndexMax
        {
            get
            {
                int indexMin = 0;
                int indexMax = 0;
                int indexDefault = 0;
                GetCameraParameterRangeInt((int)ICamera.Params.PARAM_TAP_INDEX, ref indexMin, ref indexMax, ref indexDefault);
                return indexMax;
            }
        }

        public int ReadOutTapIndexMin
        {
            get
            {
                int indexMin = 0;
                int indexMax = 0;
                int indexDefault = 0;
                GetCameraParameterRangeInt((int)ICamera.Params.PARAM_TAP_INDEX, ref indexMin, ref indexMax, ref indexDefault);
                return indexMin;
            }
        }

        public int ReadOutTapValue
        {
            get
            {
                int readOutTapValue = 0;
                GetCameraParameterValueInt((int)ICamera.Params.PARAM_TAP_VALUE, ref readOutTapValue);
                return readOutTapValue;
            }
        }

        public int Right
        {
            get
            {
                int right = 0;
                GetCameraParameterValueInt((int)ICamera.Params.PARAM_CAPTURE_REGION_RIGHT, ref right);
                return right;
            }
            set
            {
                SetCameraParameterValueDouble((int)ICamera.Params.PARAM_CAPTURE_REGION_RIGHT,(double)value);
            }
        }

        public int TapBalance
        {
            get
            {
                int tapMode = 0;
                GetCameraParameterValueInt((int)ICamera.Params.PARAM_TAP_BALANCE_MODE, ref tapMode);
                return tapMode;
            }
            set
            {
                SetCameraParameterValueInt((int)ICamera.Params.PARAM_TAP_BALANCE_MODE, value);
            }
        }

        public int TDILineShifts
        {
            get
            {
                int tdiLineShifts = 0;
                GetCameraParameterValueInt((int)ICamera.Params.PARAM_TDI_LINESHIFTS, ref tdiLineShifts);
                return tdiLineShifts;
            }
            set
            {
                SetCameraParameterValueDouble((int)ICamera.Params.PARAM_TDI_LINESHIFTS,(double)value);
            }
        }

        public int TDILineTrim
        {
            get
            {
                int tdiLineTrim = 0;
                GetCameraParameterValueInt((int)ICamera.Params.PARAM_TDI_LINETRIM, ref tdiLineTrim);
                return tdiLineTrim;
            }
            set
            {
                SetCameraParameterValueDouble((int)ICamera.Params.PARAM_TDI_LINETRIM, (double)value);
            }
        }

        public int TDITriggers
        {
            get
            {
                int tdiTriggers = 0;
                GetCameraParameterValueInt((int)ICamera.Params.PARAM_TDI_TRIGGERS, ref tdiTriggers);
                return tdiTriggers;
            }
            set
            {
                SetCameraParameterValueDouble((int)ICamera.Params.PARAM_TDI_TRIGGERS,(double)value);
            }
        }

        public int TDITrimMode
        {
            get
            {
                int tdiTrimMode = 0;
                GetCameraParameterValueInt((int)ICamera.Params.PARAM_TDI_TRIM_MODE, ref tdiTrimMode);
                return tdiTrimMode;
            }
            set
            {
                SetCameraParameterValueDouble((int)ICamera.Params.PARAM_TDI_TRIM_MODE,(double)value);
            }
        }

        public int Top
        {
            get
            {
                int top = 0;
                GetCameraParameterValueInt((int)ICamera.Params.PARAM_CAPTURE_REGION_TOP, ref top);
                return top;
            }
            set
            {
                SetCameraParameterValueDouble((int)ICamera.Params.PARAM_CAPTURE_REGION_TOP,(double)value);
            }
        }

        #endregion Properties

        #region Methods

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetCameraHeight")]
        private static extern bool GetCameraHeight(ref int height);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetCameraParameterRangeDouble")]
        private static extern bool GetCameraParameterRangeDouble(int param, ref double valMin,ref double valMax,ref double valDefault);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetCameraParameterRangeInt")]
        private static extern bool GetCameraParameterRangeInt(int param, ref int paramMin, ref int paramMax, ref int paramDefault);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetCameraParameterValueDouble")]
        private static extern bool GetCameraParameterValueDouble(int param, ref double value);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetCameraParameterValueInt")]
        private static extern bool GetCameraParameterValueInt(int param, ref int paramValue);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetCameraWidth")]
        private static extern bool GetCameraWidth(ref int width);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetCameraParameterValueDouble")]
        private static extern bool SetCameraParameterValueDouble(int param, double paramValue);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetCameraParameterValueInt")]
        private static extern bool SetCameraParameterValueInt(int param, int paramValue);

        #endregion Methods
    }
}