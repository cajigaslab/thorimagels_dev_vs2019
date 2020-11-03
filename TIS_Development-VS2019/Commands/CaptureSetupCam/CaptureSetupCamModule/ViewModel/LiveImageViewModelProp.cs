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
    using System.Windows;
    using System.Windows.Data;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Threading;
    using System.Xml;

    using CaptureSetupDll.Model;

    using Microsoft.Practices.Composite.Events;
    using Microsoft.Practices.Composite.Regions;
    using Microsoft.Practices.Composite.Wpf.Events;
    using Microsoft.Practices.Unity;

    using ThorImageInfastructure;

    using ThorLogging;

    using ThorSharedTypes;

    public partial class LiveImageViewModel : ViewModelBase, ICameraProps
    {
        #region Fields

        private int _activeCameraID;
        private int _exposureRangeIndex;
        private double _exposureTimeCam0; // keeps the exposure time value used by the User Interface
        private double _exposureTimeSliderMax = 100;
        private int _height;
        private int _inputRangeMax;
        private int _inputRangeMin;
        private DateTime _lastPowerUpdateTime = DateTime.Now;
        private DateTime _lastZUpdateTime = DateTime.Now;
        private string _pathSaveSnapshot = string.Empty;
        private int _sampleType = 0;
        private int _selectedSubColumn = 0;
        private int _selectedSubRow = 0;
        private int _selectedWellColumnCount = 1;
        private int _selectedWellRowCount = 1;
        private string _snapshotImagePrefix = string.Empty;
        private double _subOffsetX = 0;
        private double _subOffsetY = 0;
        double _subSpacingXPercent = 0.0;
        double _subSpacingYPercent = 0.0;
        private double _tdiHeightMM = 0;
        private double _tdiWidthMM = 0;
        private int _tileControlMode = 0;
        private int _width;
        private int _xDirection = 1;
        private int _yDirection = 1;
        private int _zRangeDirection = 0;

        #endregion Fields

        #region Properties

        public int ActiveCameraID
        {
            get
            {
                return _activeCameraID;
            }
            set
            {
                // set camera to full frame if camera is switched
                if (value != _activeCameraID)
                {
                    SetCameraFullFrame();
                }

                _activeCameraID = value;
            }
        }

        public XmlDocument ApplicationDoc
        {
            get
            {
                return this._applicationDoc;
            }
            set
            {
                this._applicationDoc = value;
            }
        }

        public int BinX
        {
            get{return this._camProps.BinX;}
            set
            {
                this._camProps.BinX = value;
                OnPropertyChanged("BinX");
            }
        }

        public ICommand BinXMinusCommand
        {
            get
            {
                if (this._binXMinusCommand == null)
                    this._binXMinusCommand = new RelayCommand(() => BinXMinus());

                return this._binXMinusCommand;
            }
        }

        public ICommand BinXPlusCommand
        {
            get
            {
                if (this._binXPlusCommand == null)
                    this._binXPlusCommand = new RelayCommand(() => BinXPlus());

                return this._binXPlusCommand;
            }
        }

        public int BinY
        {
            get
            {
                return this._camProps.BinY;
            }
            set
            {
                this._camProps.BinY = value;
                OnPropertyChanged("BinY");
            }
        }

        public ICommand BinYMinusCommand
        {
            get
            {
                if (this._binYMinusCommand == null)
                    this._binYMinusCommand = new RelayCommand(() => BinYMinus());

                return this._binYMinusCommand;
            }
        }
        public int BitsPerPixel
        {
            get
            {
                return this._liveImage.BitsPerPixel;
            }
        }

        public ICommand BinYPlusCommand
        {
            get
            {
                if (this._binYPlusCommand == null)
                    this._binYPlusCommand = new RelayCommand(() => BinYPlus());

                return this._binYPlusCommand;
            }
        }

        public WriteableBitmap Bitmap
        {
            get
            {

                switch (LiveImage.GetColorChannels())
                {
                    case 1:
                        {
                            byte[] pd = LiveImage.GetPixelDataByte();

                            //verify pixel data is available
                            if (pd == null)
                            {
                                return _bitmap;
                            }

                            // Define parameters used to create the BitmapSource.
                            PixelFormat pf = PixelFormats.Indexed8;
                            int width = this._liveImage.DataWidth;
                            int height = this._liveImage.DataHeight;
                            int rawStride = (width * pf.BitsPerPixel + 7) / 8;

                            if ((_palette == null) || (true == _paletteChanged))
                            {
                                //the whitepoint blackpoint scaling is now done on the raw data retrieval
                                _palette = BuildPalette(255.0,0);
                            }

                            //create a new bitmpap when one does not exist or the size of the image changes
                            if (_bitmap == null)
                            {
                                _bitmap = new WriteableBitmap(width, height, 96, 96, pf, _palette);
                            }
                            else
                            {
                                if ((_bitmap.Width != width) || (_bitmap.Height != height) || (true == _paletteChanged) || (_bitmap.Format != pf))
                                {
                                    _bitmap = new WriteableBitmap(width, height, 96, 96, pf, _palette);
                                }
                            }

                            int w = _bitmap.PixelWidth;
                            int h = _bitmap.PixelHeight;
                            int widthInBytes = w;

                            if (pd.Length == (width * height))
                            {
                                if (GridAlignImage)
                                {
                                    byte[] buffer = new byte[width * height];
                                    for (int r = 0; r < height; r++)
                                    {
                                        for (int c = 0; c < width; c++)
                                        {
                                            if (r < height / 2)
                                            {
                                                buffer[r * width + c] = pd[(r + height / 2) * width + c];
                                            }
                                            else
                                            {
                                                buffer[r * width + c] = pd[(r - height / 2) * width + c];
                                            }
                                        }
                                    }
                                    _bitmap.WritePixels(new Int32Rect(0, 0, width, height),buffer, widthInBytes, 0);
                                }
                                else
                                {
                                    //copy the pixel data into the _bitmap
                                    _bitmap.WritePixels(new Int32Rect(0, 0, width, height), pd, widthInBytes, 0);
                                }
                            }

                            _paletteChanged = false;
                        }
                        break;
                    default:
                        {
                            byte[] pd = LiveImage.GetPixelDataByteEx(true, 0);

                            //verify pixel data is available
                            if (pd == null)
                            {
                                return _bitmap;
                            }

                            byte[] pdPal = new byte[pd.Length];

                            // Define parameters used to create the BitmapSource.
                            PixelFormat pf = PixelFormats.Rgb24;

                            int width = this._liveImage.DataWidth;
                            int height = this._liveImage.DataHeight;
                            int rawStride = (width * pf.BitsPerPixel + 7) / 8;

                            int outputBitmapWidth = width;
                            int outputBitmapHeight = height;

                            if (TileDisplay)
                            {

                                switch (LiveImage.GetColorChannels())
                                {
                                    case 2:
                                        {
                                            outputBitmapWidth *= 2;
                                            outputBitmapHeight *= 2;
                                        }
                                        break;
                                    default:
                                        {//more than 2 channels
                                            outputBitmapWidth *= 3;
                                            outputBitmapHeight *= 2;
                                        }
                                        break;
                                }
                            }

                            //create a new bitmpap when one does not exist or the size of the image changes
                            if (_bitmap == null)
                            {
                                _bitmap = new WriteableBitmap(outputBitmapWidth, outputBitmapHeight, 96, 96, pf, null);
                            }
                            else
                            {
                                if ((_bitmap.Width != outputBitmapWidth) || (_bitmap.Height != outputBitmapHeight) || (_bitmap.Format != pf))
                                {
                                    _bitmap = new WriteableBitmap(outputBitmapWidth, outputBitmapHeight, 96, 96, pf, null);
                                }
                            }

                            int w = _bitmap.PixelWidth;
                            int h = _bitmap.PixelHeight;

                            if ((pd.Length / 3) == (width * height))
                            {
                                //copy the color pixel data into the bitmap
                                _bitmap.WritePixels(new Int32Rect(0, 0, width, height), pd, rawStride, 0);

                                if (TileDisplay)
                                {
                                    switch (LiveImage.GetColorChannels())
                                    {
                                        case 2:
                                            {
                                                //channel A
                                                pd = LiveImage.GetPixelDataByteEx(false, 0);

                                                _bitmap.WritePixels(new Int32Rect(0, height, width, height), pd, rawStride, 0);

                                                //channel B
                                                pd = LiveImage.GetPixelDataByteEx(false, 1);

                                                _bitmap.WritePixels(new Int32Rect(width, height, width, height), pd, rawStride, 0);
                                            }
                                            break;

                                        default:
                                            {//more than 2 channels

                                                //channel A
                                                pd = LiveImage.GetPixelDataByteEx(false, 0);

                                                _bitmap.WritePixels(new Int32Rect(width, 0, width, height), pd, rawStride, 0);

                                                //channel B
                                                pd = LiveImage.GetPixelDataByteEx(false, 1);

                                                _bitmap.WritePixels(new Int32Rect(2 * width, 0, width, height), pd, rawStride, 0);

                                                //channel C
                                                pd = LiveImage.GetPixelDataByteEx(false, 2);

                                                _bitmap.WritePixels(new Int32Rect(0, height, width, height), pd, rawStride, 0);

                                                //channel D
                                                pd = LiveImage.GetPixelDataByteEx(false, 3);

                                                _bitmap.WritePixels(new Int32Rect(width, height, width, height), pd, rawStride, 0);
                                            }
                                            break;
                                    }
                                }
                            }

                            _paletteChanged = false;
                        }
                        break;
                }

                ImageDataChanged(true);

                LiveImage.FinishedCopyingPixel();

                return _bitmap;
            }
        }

        public ICommand BlackLevelMinusCommand
        {
            get
            {
                if (this._blackLevelMinusCommand == null)
                    this._blackLevelMinusCommand = new RelayCommand(() => BlackLevelMinus());

                return this._blackLevelMinusCommand;
            }
        }

        public ICommand BlackLevelPlusCommand
        {
            get
            {
                if (this._blackLevelPlusCommand == null)
                    this._blackLevelPlusCommand = new RelayCommand(() => BlackLevelPlus());

                return this._blackLevelPlusCommand;
            }
        }

        public double BlackPoint0
        {
            get
            {
                return this._liveImage.BlackPoint0;
            }
            set
            {
                if (value != this._liveImage.BlackPoint0)
                {
                    this._liveImage.BlackPoint0 = value;
                    _paletteChanged = true;
                    OnPropertyChanged("BlackPoint0");
                    OnPropertyChanged("Bitmap");
                }
            }
        }

        public double BlackPoint1
        {
            get
            {
                return this._liveImage.BlackPoint1;
            }
            set
            {
                if (value != this._liveImage.BlackPoint1)
                {
                    this._liveImage.BlackPoint1 = value;
                    _paletteChanged = true;
                    OnPropertyChanged("BlackPoint1");
                    OnPropertyChanged("Bitmap");
                }
            }
        }

        public double BlackPoint2
        {
            get
            {
                return this._liveImage.BlackPoint2;
            }
            set
            {
                if (value != this._liveImage.BlackPoint2)
                {
                    this._liveImage.BlackPoint2 = value;
                    _paletteChanged = true;
                    OnPropertyChanged("BlackPoint2");
                    OnPropertyChanged("Bitmap");
                }
            }
        }

        public double BlackPoint3
        {
            get
            {
                return this._liveImage.BlackPoint3;
            }
            set
            {
                if (value != this._liveImage.BlackPoint3)
                {
                    this._liveImage.BlackPoint3 = value;
                    _paletteChanged = true;
                    OnPropertyChanged("BlackPoint3");
                    OnPropertyChanged("Bitmap");
                }
            }
        }

        public int Bottom
        {
            get
            {
                return this._camProps.Bottom;
            }
            set
            {
                this._camProps.Bottom = value;
                OnPropertyChanged("Bottom");
            }
        }

        public ICommand CameraConsoleCommand
        {
            get
            {
                if (this._cameraConsoleCommand == null)
                    this._cameraConsoleCommand = new RelayCommand(() => CameraConsole());

                return this._cameraConsoleCommand;
            }
        }

        public int CameraHeight
        {
            get
            {
                return this._camProps.CameraHeight;
            }
        }

        public int CameraWidth
        {
            get
            {
                return this._camProps.CameraWidth;
            }
        }

        public CameraType CamType
        {
            get
            {
                return (CameraType)this._liveImage.CamType;
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

        public ICommand CenterROICommand
        {
            get
            {
                if (this._centerROICommand == null)
                    this._centerROICommand = new RelayCommand(() => CenterROI());

                return this._centerROICommand;
            }
        }

        public double Coeff1
        {
            get
            {
                return this._liveImage.Coeff1;
            }
            set
            {
                this._liveImage.Coeff1 = value;
                OnPropertyChanged("Coeff1");
            }
        }

        public double Coeff2
        {
            get
            {
                return this._liveImage.Coeff2;
            }
            set
            {
                this._liveImage.Coeff2 = value;
                OnPropertyChanged("Coeff2");
            }
        }

        public double Coeff3
        {
            get
            {
                return this._liveImage.Coeff3;
            }
            set
            {
                this._liveImage.Coeff3 = value;
                OnPropertyChanged("Coeff3");
            }
        }

        public double Coeff4
        {
            get
            {
                return this._liveImage.Coeff4;
            }
            set
            {
                this._liveImage.Coeff4 = value;
                OnPropertyChanged("Coeff4");
            }
        }

        public int ColorChannels
        {
            get
            {
                return LiveImage.GetColorChannels();
            }
        }

        public Guid CommandGuid
        {
            get { return this._liveImage.CommandGuid; }
        }

        public int CoolingMode
        {
            get
            {
                return this._camProps.CoolingMode;
            }
            set
            {
                this._camProps.CoolingMode = value;
                OnPropertyChanged("CoolingMode");
            }
        }

        public bool CoolingModeSupported
        {
            get
            {
                return this._camProps.CoolingModeSupported;
            }
        }

        public ICommand DecreaseXCommand
        {
            get
            {
                if (this._decreaseXCommand == null)
                    this._decreaseXCommand = new RelayCommand(() => DecreaseX());

                return this._decreaseXCommand;
            }
        }

        public ICommand DecreaseYCommand
        {
            get
            {
                if (this._decreaseYCommand == null)
                    this._decreaseYCommand = new RelayCommand(() => DecreaseY());

                return this._decreaseYCommand;
            }
        }

        public LiveImage.DigitizerBoardNames DigitizerBoardName
        {
            get
            {
                return this._liveImage.DigitizerBoardName;
            }
            set
            {
                this._liveImage.DigitizerBoardName = value;
                OnPropertyChanged("DigitizerBoardName");
            }
        }

        public int EnableBackgroundSubtraction
        {
            get
            {
                return this._liveImage.EnableBackgroundSubtraction;
            }
            set
            {
                this._liveImage.EnableBackgroundSubtraction = value;
                OnPropertyChanged("EnableBackgroundSubtraction");
            }
        }

        public int EnableFlatField
        {
            get
            {
                return this._liveImage.EnableFlatField;
            }
            set
            {
                this._liveImage.EnableFlatField = value;
                OnPropertyChanged("EnableFlatField");
            }
        }

        public int EnablePincushionCorrection
        {
            get
            {
                return this._liveImage.PincushionCorrection;
            }
            set
            {
                this._liveImage.PincushionCorrection = value;
                OnPropertyChanged("EnablePincushionCorrection");
            }
        }

        public XmlDocument ExperimentDoc
        {
            get
            {
                return this._experimentDoc;
            }
            set
            {
                this._experimentDoc = value;
            }
        }

        public int ExposureRangeIndex
        {
            get
            {
                return this._exposureRangeIndex;
            }
            set
            {
                this._exposureRangeIndex = value;

                switch (_exposureRangeIndex)
                {
                    case 0:
                        this._exposureTimeSliderMax = 10;
                        this._camProps.ExposureTimeCam0 = Math.Min(_exposureTimeSliderMax, _exposureTimeCam0);
                        break;
                    case 1:
                        this._exposureTimeSliderMax = 100;
                        this._camProps.ExposureTimeCam0 = Math.Min(_exposureTimeSliderMax, _exposureTimeCam0);
                        break;
                    case 2:
                        this._exposureTimeSliderMax = 1000;
                        this._camProps.ExposureTimeCam0 = Math.Min(_exposureTimeSliderMax, _exposureTimeCam0);
                        break;
                    case 3:
                        this._exposureTimeSliderMax = this.ExposureTimeMax;
                        this._camProps.ExposureTimeCam0 = Math.Min(_exposureTimeSliderMax, _exposureTimeCam0);
                        break;
                }
                OnPropertyChanged("ExposureTimeCam0");
                OnPropertyChanged("ExposureRangeIndex");
                OnPropertyChanged("ExposureTimeSliderMax");
            }
        }

        public double ExposureTimeCam0
        {
            get
            {
                _exposureTimeCam0 = this._camProps.ExposureTimeCam0;
                return _exposureTimeCam0;
            }
            set
            {
                //stop and restart the camera if pre-set exposure is more than 1s
                //this avoids waiting for the period of exposure time
                if ((_exposureTimeCam0 > 1000) && (_isLive))
                {
                    this._liveImage.Stop();
                    _exposureTimeCam0 = value;
                    this._camProps.ExposureTimeCam0 = value;
                    this._liveImage.Start();
                }
                //Only change the exposure time during slider dragging for fast exposure times (<100ms)
                else if (IsExposureSliderMouseCaptured == false || this._exposureRangeIndex <= 1)
                {
                    _exposureTimeCam0 = value;
                    this._camProps.ExposureTimeCam0 = value;
                }

                //change the slider range if changes are initated from textbox entry, not throught slider changes
                if (IsExposureSliderMouseCaptured == false)
                {
                    //change exposure range
                    if (_exposureTimeCam0 > 1000)
                    {
                        this._exposureRangeIndex = 3;
                        _exposureTimeSliderMax = ExposureTimeMax;
                    }
                    else if (_exposureTimeCam0 > 100)
                    {
                        this._exposureRangeIndex = 2;
                        _exposureTimeSliderMax = 1000;
                    }
                    else if (_exposureTimeCam0 > 10)
                    {
                        this._exposureRangeIndex = 1;
                        _exposureTimeSliderMax = 100;
                    }
                    else
                    {
                        this._exposureRangeIndex = 0;
                        _exposureTimeSliderMax = 10;
                    }
                }

                OnPropertyChanged("ExposureTimeCam0");
                OnPropertyChanged("ExposureRangeIndex");
                OnPropertyChanged("ExposureTimeSliderMax");
            }
        }

        public double ExposureTimeMax
        {
            get
            {
                return this._camProps.ExposureTimeMax;
            }
        }

        public double ExposureTimeMin
        {
            get
            {
                return this._camProps.ExposureTimeMin;
            }
        }

        public double ExposureTimeSliderMax
        {
            get
            {
                return _exposureTimeSliderMax;
            }
            set
            {
                this._exposureTimeSliderMax = value;
                OnPropertyChanged("ExposureTimeSliderMax");
                OnPropertyChanged("ExposureTimeMax");
            }
        }

        public int FilterPositionDic
        {
            get
            {
                return this._liveImage.FilterPositionDic;
            }
            set
            {
                this._liveImage.FilterPositionDic = value;
                OnPropertyChanged("FilterPositionDic");
            }
        }

        public int FilterPositionEm
        {
            get
            {
                return this._liveImage.FilterPositionEm;
            }
            set
            {
                this._liveImage.FilterPositionEm = value;
                OnPropertyChanged("FilterPositionEm");
            }
        }

        public int FilterPositionEx
        {
            get
            {
                return this._liveImage.FilterPositionEx;
            }
            set
            {
                this._liveImage.FilterPositionEx = value;
                OnPropertyChanged("FilterPositionEx");
            }
        }

        public int FineMode
        {
            get
            {
                return _fineMode;
            }
            set
            {

                _fineMode = value;

                if (_fineMode == 1)
                {
                    _sliderZMin = this._liveImage.ZPosition - .5;
                    _sliderZMax = this._liveImage.ZPosition + .5;
                    _sliderTickFrequency = .001;
                }
                else
                {
                    _sliderZMin = this._liveImage.ZMin;
                    _sliderZMax = this._liveImage.ZMax;
                    _sliderTickFrequency = .05;
                }
                OnPropertyChanged("ZMin");
                OnPropertyChanged("ZMax");
                OnPropertyChanged("ZSliderStepSize");
            }
        }

        public int FrameCount
        {
            get
            {
                return this._camProps.FrameCount;
            }
            set
            {
                this._camProps.FrameCount = value;
                OnPropertyChanged("FrameCount");
            }
        }

        public string FramesPerSecond
        {
            get
            {
                return String.Format("{0} fps", this._liveImage.FramesPerSecond.ToString("#0.0"));
            }
        }

        public int Gain
        {
            get
            {
                return this._camProps.Gain;
            }
            set
            {
                this._camProps.Gain = value;
                OnPropertyChanged("Gain");
            }
        }

        public int GainMax
        {
            get
            {
                return this._camProps.GainMax;
            }
        }

        public int GainMin
        {
            get
            {
                return this._camProps.GainMin;
            }
        }

        public ICommand GoPowerCommand
        {
            get
            {
                if (this._goPowerCommand == null)
                    this._goPowerCommand = new RelayCommand(() => GoPower());

                return this._goPowerCommand;
            }
        }

        public ICommand GoPowerStartCommand
        {
            get
            {
                if (this._goPowerStartCommand == null)
                    this._goPowerStartCommand = new RelayCommand(() => GoPowerStart());

                return this._goPowerStartCommand;
            }
        }

        public ICommand GoPowerStopCommand
        {
            get
            {
                if (this._goPowerStopCommand == null)
                    this._goPowerStopCommand = new RelayCommand(() => GoPowerStop());

                return this._goPowerStopCommand;
            }
        }

        public ICommand GoZCommand
        {
            get
            {
                if (this._goZCommand == null)
                    this._goZCommand = new RelayCommand(() => GoZ());

                return this._goZCommand;
            }
        }

        public ICommand GoZScanStartCommand
        {
            get
            {
                if (this._goZScanStartCommand == null)
                    this._goZScanStartCommand = new RelayCommand(() => GoZScanStart());

                return this._goZScanStartCommand;
            }
        }

        public ICommand GoZScanStopCommand
        {
            get
            {
                if (this._goZScanStopCommand == null)
                    this._goZScanStopCommand = new RelayCommand(() => GoZScanStop());

                return this._goZScanStopCommand;
            }
        }

        public bool GridAlignImage
        {
            get;
            set;
        }

        public XmlDocument HardwareDoc
        {
            get
            {
                return this._hardwareDoc;
            }
            set
            {
                this._hardwareDoc = value;
            }
        }

        public int Height
        {
            get
            {
                return _height;
            }
            set
            {
                //validate the height and restrict it to only valid values
                if (0 < value)
                {
                    int top = this.Top;
                    int newBottom = Math.Min(this.CameraHeight, top + value);
                    this._height = newBottom - top;
                    this.Bottom = newBottom;
                    OnPropertyChanged("Height");
                    OnPropertyChanged("Bottom");
                }
            }
        }

        public int[] HistogramData0
        {
            get
            {
                return this._liveImage.HistogramData0;
            }
        }

        public int[] HistogramData1
        {
            get
            {
                return this._liveImage.HistogramData1;
            }
        }

        public int[] HistogramData2
        {
            get
            {

                return this._liveImage.HistogramData2;
            }
        }

        public int[] HistogramData3
        {
            get
            {
                return this._liveImage.HistogramData3;
            }
        }

        public int ImageColorChannels
        {
            get
            {
                return this._liveImage.ImageColorChannels;
            }
        }

        public string ImagePathPlay
        {
            get
            {
                if (_isLive)
                {
                    return @"/CaptureSetupCamModule;component/Icons/Stop.png";
                }
                else
                {
                    return @"/CaptureSetupCamModule;component/Icons/Play.png";
                }
            }
        }

        public ICommand IncreaseXCommand
        {
            get
            {
                if (this._increaseXCommand == null)
                    this._increaseXCommand = new RelayCommand(() => IncreaseX());

                return this._increaseXCommand;
            }
        }

        public ICommand IncreaseYCommand
        {
            get
            {
                if (this._increaseYCommand == null)
                    this._increaseYCommand = new RelayCommand(() => IncreaseY());

                return this._increaseYCommand;
            }
        }

        public int InputRangeChannel1
        {
            get
            {
                return TranslateInputRangeValue(1, this._liveImage.InputRangeChannel1);
            }
            set
            {
                this._liveImage.InputRangeChannel1 = TranslateInputRangeValue(0, value);
                OnPropertyChanged("InputRangeChannel1");
            }
        }

        public int InputRangeChannel2
        {
            get
            {
                return TranslateInputRangeValue(1, this._liveImage.InputRangeChannel2);
            }
            set
            {
                this._liveImage.InputRangeChannel2 = TranslateInputRangeValue(0, value);
                OnPropertyChanged("InputRangeChannel2");
            }
        }

        public int InputRangeChannel3
        {
            get
            {
                return TranslateInputRangeValue(1, this._liveImage.InputRangeChannel3);
            }
            set
            {
                this._liveImage.InputRangeChannel3 = TranslateInputRangeValue(0, value);
                OnPropertyChanged("InputRangeChannel3");
            }
        }

        public int InputRangeChannel4
        {
            get
            {
                return TranslateInputRangeValue(1, this._liveImage.InputRangeChannel4);
            }
            set
            {
                this._liveImage.InputRangeChannel4 = TranslateInputRangeValue(0, value);
                OnPropertyChanged("InputRangeChannel4");
            }
        }

        public int InputRangeMax
        {
            get
            {
                return _inputRangeMax;
            }
            set
            {
                _inputRangeMax = value;
                OnPropertyChanged("InputRangeMax");
            }
        }

        public int InputRangeMin
        {
            get
            {
                return _inputRangeMin;
            }
            set
            {

                _inputRangeMin = value;
                OnPropertyChanged("InputRangeMin");
            }
        }

        public Visibility[] IsChannelVisible
        {
            get
            {
                return _isChannelVisible;
            }
            set
            {
                _isChannelVisible = value;
                OnPropertyChanged("IsChannelVisible");
            }
        }

        public bool IsDecoderPresent
        {
            get
            {
                XmlNodeList ndList = this.HardwareDoc.SelectNodes("/HardwareSettings/Devices/XYStage");
                foreach (XmlNode node in ndList)
                {
                    string sid = string.Empty;
                    GetAttribute(node, HardwareDoc, "id", ref sid);

                    string strActive = string.Empty;
                    GetAttribute(node, HardwareDoc, "active", ref strActive);

                    string dllName = string.Empty;
                    GetAttribute(node, HardwareDoc, "dllName", ref dllName);

                    if (!dllName.Contains("SimDevice") && (strActive.Equals("1")))
                    {
                        return this._liveImage.VerifyDecoder(Convert.ToInt32(sid));
                    }
                }
                return false;
            }
        }

        public bool IsExposureSliderMouseCaptured
        {
            get;
            set;
        }

        public bool IsLive
        {
            get
            {
                return _isLive;
            }
        }

        public bool IsStoredCameraInfoValid
        {
            get
            {
                string activeCamName = string.Empty;
                XmlNodeList ndList = this.HardwareDoc.SelectNodes("/HardwareSettings/ImageDetectors/Camera");

                foreach (XmlNode node in ndList)
                {
                    string active = string.Empty;
                    GetAttribute(node, this.HardwareDoc, "active", ref active);
                    if (active.Equals("1"))
                    {
                        GetAttribute(node, HardwareDoc, "cameraName", ref activeCamName);
                    }
                }

                string prevCamName = string.Empty;
                XmlNode nd = this.ExperimentDoc.SelectSingleNode("/ThorImageExperiment/Camera");
                GetAttribute(nd, this.ExperimentDoc, "name", ref prevCamName);

                if (activeCamName.Equals(prevCamName))
                    return true;
                else
                    return false;
            }
        }

        public bool IsXYStagePresent
        {
            get
            {
                XmlNodeList ndList = this.HardwareDoc.SelectNodes("/HardwareSettings/Devices/XYStage");
                foreach(XmlNode node in ndList)
                {
                    if (!node.Attributes["dllName"].Value.Contains("SimDevice"))
                    {
                        return true;
                    }
                }
                return false;
            }
        }

        public int LampPosition
        {
            get
            {
                return this._liveImage.LampPosition;
            }
            set
            {
                this._liveImage.LampPosition = value;
                OnPropertyChanged("LampPosition");
            }
        }

        public int Laser1Enable
        {
            get
            {
                return this._liveImage.Laser1Enable;
            }
            set
            {
                this._liveImage.Laser1Enable = value;
                OnPropertyChanged("Laser1Enable");
            }
        }

        public double Laser1Max
        {
            get
            {
                return this._liveImage.Laser1Max;
            }
        }

        public double Laser1Min
        {
            get
            {
                return this._liveImage.Laser1Min;
            }
        }

        public int Laser1Position
        {
            get
            {
                return this._liveImage.Laser1Position;
            }
            set
            {
                this._liveImage.Laser1Position = value;
                OnPropertyChanged("Laser1Position");
            }
        }

        public double Laser1Power
        {
            get
            {
                return this._liveImage.Laser1Power;
            }
            set
            {
                this._liveImage.Laser1Power = value;
                OnPropertyChanged("Laser1Power");
            }
        }

        public int Laser2Enable
        {
            get
            {
                return this._liveImage.Laser2Enable;
            }
            set
            {
                this._liveImage.Laser2Enable = value;
                OnPropertyChanged("Laser2Enable");
            }
        }

        public double Laser2Max
        {
            get
            {
                return this._liveImage.Laser2Max;
            }
        }

        public double Laser2Min
        {
            get
            {
                return this._liveImage.Laser2Min;
            }
        }

        public double Laser2Power
        {
            get
            {
                return this._liveImage.Laser2Power;
            }
            set
            {
                this._liveImage.Laser2Power = value;
                OnPropertyChanged("Laser2Power");
            }
        }

        public int Laser3Enable
        {
            get
            {
                return this._liveImage.Laser3Enable;
            }
            set
            {
                this._liveImage.Laser3Enable = value;
                OnPropertyChanged("Laser3Enable");
            }
        }

        public double Laser3Max
        {
            get
            {
                return this._liveImage.Laser3Max;
            }
        }

        public double Laser3Min
        {
            get
            {
                return this._liveImage.Laser3Min;
            }
        }

        public double Laser3Power
        {
            get
            {
                return this._liveImage.Laser3Power;
            }
            set
            {
                this._liveImage.Laser3Power = value;
                OnPropertyChanged("Laser3Power");
            }
        }

        public int Laser4Enable
        {
            get
            {
                return this._liveImage.Laser4Enable;
            }
            set
            {
                this._liveImage.Laser4Enable = value;
                OnPropertyChanged("Laser4Enable");
            }
        }

        public double Laser4Max
        {
            get
            {
                return this._liveImage.Laser4Max;
            }
        }

        public double Laser4Min
        {
            get
            {
                return this._liveImage.Laser4Min;
            }
        }

        public double Laser4Power
        {
            get
            {
                return this._liveImage.Laser4Power;
            }
            set
            {
                this._liveImage.Laser4Power = value;
                OnPropertyChanged("Laser4Power");
            }
        }

        public int LaserShutterPosition
        {
            get
            {
                return this.LiveImage.LaserShutterPosition;
            }
            set
            {
                this.LiveImage.LaserShutterPosition = value;
            }
        }

        public int Left
        {
            get
            {
                return this._camProps.Left;
            }
            set
            {
                this._camProps.Left = value;
                this.Width = this._width;
                OnPropertyChanged("Left");
                OnPropertyChanged("Width");
            }
        }

        public int LightMode
        {
            get
            {
                return this._camProps.LightMode;
            }
            set
            {
                this._camProps.LightMode = value;
                OnPropertyChanged("LightMode");
            }
        }

        public int LightModeMax
        {
            get
            {
                return this._camProps.LightModeMax;
            }
        }

        public int LightModeMin
        {
            get
            {
                return this._camProps.LightModeMin;
            }
        }

        /// <summary>
        /// Gets the wrapped LiveImage object
        /// </summary>
        public LiveImage LiveImage
        {
            get
            {
                return this._liveImage;
            }
        }

        public bool LiveSnapshotStatus
        {
            get
            {
                return _liveSnapshotStatus;
            }
            set
            {
                _liveSnapshotStatus = value;
            }
        }

        /// <summary>
        /// Gets or sets the left of the liveImage
        /// </summary>
        public bool LiveStartButtonStatus
        {
            get
            {
                return _liveImage.LiveStartButtonStatus;
            }
            set
            {
                this._liveImage.LiveStartButtonStatus = value;
                OnPropertyChanged("LiveStartButtonStatus");
            }
        }

        public bool LiveStopButtonStatus
        {
            get
            {
                return _liveImage.LiveStopButtonStatus;
            }
            set
            {
                this._liveImage.LiveStopButtonStatus = value;
                OnPropertyChanged("LiveStopButtonStatus");
            }
        }

        public ICommand LSMAlignmentMinusCommand
        {
            get
            {
                if (this._alignmentMinusCommand == null)
                    this._alignmentMinusCommand = new RelayCommand(() => AlignmentMinus());

                return this._alignmentMinusCommand;
            }
        }

        public ICommand LSMAlignmentPlusCommand
        {
            get
            {
                if (this._alignmentPlusCommand == null)
                    this._alignmentPlusCommand = new RelayCommand(() => AlignmentPlus());

                return this._alignmentPlusCommand;
            }
        }

        public int LSMAreaMode
        {
            get
            {
                return this._liveImage.LSMAreaMode;
            }
            set
            {

                int tempY = this._liveImage.LSMPixelY;

                //match the X and Y to ensure that the areamode set function passes
                this._liveImage.LSMPixelY = this._liveImage.LSMPixelX;

                this._liveImage.LSMAreaMode = value;

                switch ((LiveImage.AreaMode)value)
                {
                    case LiveImage.AreaMode.SQUARE:
                        {
                           this._liveImage.LSMPixelY = this._liveImage.LSMPixelX;
                        }
                        break;
                    case LiveImage.AreaMode.RECTANGLE:
                        {
                            this._liveImage.LSMPixelY = Math.Max(tempY, this._liveImage.LSMPixelXMin);
                        }
                        break;
                    case LiveImage.AreaMode.LINE:
                        {
                            this._liveImage.LSMPixelY = 1;
                        }
                        break;
                }

                OnPropertyChanged("LSMAreaMode");
                OnPropertyChanged("LSMPixelText");
                OnPropertyChanged("LSMPixelX");
                OnPropertyChanged("LSMPixelY");
                OnPropertyChanged("LSMFieldSizeDisplayX");
                OnPropertyChanged("LSMFieldSizeDisplayY");
            }
        }

        public int LSMChannel
        {
            get
            {
                return this._liveImage.LSMChannel;
            }
            set
            {
                this._liveImage.LSMChannel = value;
                OnPropertyChanged("LSMChannel");
                OnPropertyChanged("LSMPixelXMax");
                OnPropertyChanged("LSMPixelYMax");
            }
        }

        public bool[] LSMChannelEnable
        {
            get
            {
                return this._liveImage.LSMChannelEnable;
            }
            set
            {
                this._liveImage.LSMChannelEnable = value;
                OnPropertyChanged("LSMChannelEnable");
                OnPropertyChanged("LSMChannel");
            }
        }

        public int LSMClockSource
        {
            get
            {
                //binding to a check box. Convert the 1-2 values to 0-1.
                if (1 == this._liveImage.LSMClockSource)
                {
                    return 0;
                }
                else
                {
                    return 1;
                }
            }
            set
            {
                if (0 == value)
                {
                    this._liveImage.LSMClockSource = 1;
                }
                else
                {
                    this._liveImage.LSMClockSource = 2;
                }
                OnPropertyChanged("LSMClockSource");
            }
        }

        public double LSMExtClockRate
        {
            get
            {
                //convert from Hz to MHz
                double dVal = this._liveImage.LSMExtClockRate / 1000000.0;

                dVal = Math.Round(dVal, 2);
                return dVal;
            }
            set
            {
                //Convert from Mhz to Hz
                double dVal = value * 1000000.0;
                int iVal = Convert.ToInt32(dVal);
                this._liveImage.LSMExtClockRate = iVal;
                OnPropertyChanged("LSMExtClockRate");

            }
        }

        public int LSMFieldOffsetX
        {
            get
            {
                return this._liveImage.LSMFieldOffsetX;
            }
            set
            {
                this._liveImage.LSMFieldOffsetX = value;
                OnPropertyChanged("LSMFieldOffsetX");
            }
        }

        public int LSMFieldOffsetXActual
        {
            get
            {
                int val = Convert.ToInt32(this._liveImage.LSMFieldOffsetX - 128 + CalculateFieldOffsetXDisplay(this._liveImage.LSMPixelX, this._liveImage.LSMPixelY, this._liveImage.LSMFieldSize));
                return val;
            }
            set
            {
                this._liveImage.LSMFieldOffsetX = Convert.ToInt32(value + 128 - CalculateFieldOffsetXDisplay(this._liveImage.LSMPixelX, this._liveImage.LSMPixelY, this._liveImage.LSMFieldSize));
                OnPropertyChanged("LSMFieldOffsetX");
                OnPropertyChanged("LSMFieldOffsetXDisplay");
                OnPropertyChanged("LSMFieldOffsetXActual");
            }
        }

        public int LSMFieldOffsetXDisplay
        {
            get
            {
                return this._liveImage.LSMFieldOffsetX / 2;
            }
            set
            {
                this._liveImage.LSMFieldOffsetX = Convert.ToInt32(value * 2);
                OnPropertyChanged("LSMFieldOffsetX");
                OnPropertyChanged("LSMFieldOffsetXActual");
                OnPropertyChanged("LSMFieldOffsetXDisplay");
            }
        }

        public int LSMFieldOffsetY
        {
            get
            {
                return this._liveImage.LSMFieldOffsetY;
            }
            set
            {
                this._liveImage.LSMFieldOffsetY = value;
                OnPropertyChanged("LSMFieldOffsetY");
            }
        }

        public int LSMFieldOffsetYActual
        {
            get
            {
                int val = Convert.ToInt32(this._liveImage.LSMFieldOffsetY - 128 + CalculateFieldOffsetYDisplay(this._liveImage.LSMPixelX, this._liveImage.LSMPixelY, this._liveImage.LSMFieldSize));
                return val;
            }
            set
            {
                this._liveImage.LSMFieldOffsetY = Convert.ToInt32(value + 128 - CalculateFieldOffsetYDisplay(this._liveImage.LSMPixelX, this._liveImage.LSMPixelY, this._liveImage.LSMFieldSize));
                OnPropertyChanged("LSMFieldOffsetY");
                OnPropertyChanged("LSMFieldOffsetYDisplay");
                OnPropertyChanged("LSMFieldOffsetYActual");
            }
        }

        public int LSMFieldOffsetYDisplay
        {
            get
            {
                return this._liveImage.LSMFieldOffsetY / 2;
            }
            set
            {
                this._liveImage.LSMFieldOffsetY = Convert.ToInt32(value * 2);
                OnPropertyChanged("LSMFieldOffsetY");
                OnPropertyChanged("LSMFieldOffsetYActual");
                OnPropertyChanged("LSMFieldOffsetYDisplay");
            }
        }

        public int LSMFieldSize
        {
            get
            {
                return this._liveImage.LSMFieldSize;
            }
            set
            {
                //calculate the current center location of the ROI
                int centerX = Convert.ToInt32(this.LSMFieldOffsetXDisplay + this.LSMFieldSizeDisplayX / 2.0);
                int centerY = Convert.ToInt32(this.LSMFieldOffsetYDisplay + this.LSMFieldSizeDisplayY / 2.0);

                this._liveImage.LSMFieldSize = value;

                OnPropertyChanged("LSMFieldSizeDisplayX");
                OnPropertyChanged("LSMFieldSizeDisplayY");

                //using the new field size. change the offset to keep the roi center location constant
                this.LSMFieldOffsetXDisplay = Convert.ToInt32((centerX * 2 - CalculateFieldOffsetXDisplay(this._liveImage.LSMPixelX, this._liveImage.LSMPixelY, this._liveImage.LSMFieldSize))/2.0);
                OnPropertyChanged("LSMFieldOffsetXDisplay");

                this.LSMFieldOffsetYDisplay = Convert.ToInt32((centerY * 2 - CalculateFieldOffsetYDisplay(this._liveImage.LSMPixelX, this._liveImage.LSMPixelY, this._liveImage.LSMFieldSize))/2.0);
                OnPropertyChanged("LSMFieldOffsetYDisplay");

                OnPropertyChanged("LSMFieldSizeXUM");
                OnPropertyChanged("LSMFieldSizeYUM");
                OnPropertyChanged("LSMFieldSizeXMM");
                OnPropertyChanged("LSMFieldSizeYMM");
                OnPropertyChanged("LSMUMPerPixel");
                OnPropertyChanged("LSMFieldSize");

            }
        }

        public double LSMFieldSizeCalibration
        {
            get
            {
                return _fieldSizeCalibration;
            }
            set
            {
                _fieldSizeCalibration = value;
                OnPropertyChanged("LSMFieldSizeXUM");
                OnPropertyChanged("LSMFieldSizeYUM");
                OnPropertyChanged("LSMFieldSizeXMM");
                OnPropertyChanged("LSMFieldSizeYMM");
                OnPropertyChanged("LSMUMPerPixel");
            }
        }

        public int LSMFieldSizeDisplayX
        {
            get
            {
                return Math.Max(1,Convert.ToInt32(CalculateFieldOffsetXDisplay(this._liveImage.LSMPixelX, this._liveImage.LSMPixelY, this._liveImage.LSMFieldSize)));
            }
            set
            {
            }
        }

        public int LSMFieldSizeDisplayY
        {
            get
            {
                return Math.Max(1,Convert.ToInt32(CalculateFieldOffsetYDisplay(this._liveImage.LSMPixelX, this._liveImage.LSMPixelY, this._liveImage.LSMFieldSize)));
            }
            set
            {
            }
        }

        public int LSMFieldSizeMax
        {
            get
            {
                if (_hardwareDoc != null)
                {
                    XmlNodeList ndList = _hardwareDoc.SelectNodes("/HardwareSettings/LSM");

                    if (ndList.Count > 0)
                    {
                        XmlNode node = ndList[0].Attributes.GetNamedItem("fieldSizeMax");

                        if (node != null)
                        {
                            return Math.Min(Convert.ToInt32(node.Value), this._liveImage.LSMFieldSizeMax);
                        }
                    }
                }

                return this._liveImage.LSMFieldSizeMax;

            }
        }

        public int LSMFieldSizeMin
        {
            get
            {
                return this._liveImage.LSMFieldSizeMin;
            }
        }

        public double LSMFieldSizeXMM
        {
            get
            {
                double dVal = this._liveImage.LSMFieldSize * _fieldSizeCalibration / (1000.0 * TurretMagnification);

                dVal = Math.Round(dVal, 2);
                return dVal;
            }
        }

        public double LSMFieldSizeXUM
        {
            get
            {
                double dVal = this._liveImage.LSMFieldSize * _fieldSizeCalibration / TurretMagnification;

                dVal = Math.Round(dVal, 2);
                return dVal;
            }
        }

        public double LSMFieldSizeYMM
        {
            get
            {
                double dVal = this._liveImage.LSMFieldSize * _fieldSizeCalibration / (1000.0*TurretMagnification);

                dVal = Math.Round(dVal, 2);
                return dVal;
            }
        }

        public double LSMFieldSizeYUM
        {
            get
            {
                double dVal = this._liveImage.LSMFieldSize * _fieldSizeCalibration / TurretMagnification;

                dVal = Math.Round(dVal, 2);
                return dVal;
            }
        }

        public int LSMPinholeAlignmentPosition
        {
            get
            {
                return this._liveImage.LSMPinholeAlignmentPosition;
            }
            set
            {
                this._liveImage.LSMPinholeAlignmentPosition = value;
                OnPropertyChanged("LSMPinholeAlignmentPosition");
            }
        }

        public string LSMPixelText
        {
            get
            {
                string str = string.Empty;

                switch ((LiveImage.AreaMode)LSMAreaMode)
                {
                    case LiveImage.AreaMode.SQUARE:
                        {
                            str = String.Format("{0} x {1}", this._liveImage.LSMPixelX, this._liveImage.LSMPixelX);
                        }
                        break;
                    case LiveImage.AreaMode.RECTANGLE:
                        {
                            str = String.Format("{0} x {1}", this._liveImage.LSMPixelX, this._liveImage.LSMPixelY);
                        }
                        break;
                    case LiveImage.AreaMode.LINE:
                        {
                            str = String.Format("{0} x 1", this._liveImage.LSMPixelX);
                        }
                        break;
                }

                return str;
            }
        }

        public int LSMPixelX
        {
            get
            {
                return this._liveImage.LSMPixelX;
            }
            set
            {
                if (value <= LSMPixelXMax && value >= LSMPixelXMin)
                {
                    this._liveImage.LSMPixelX = value;
                    OnPropertyChanged("LSMPixelX");
                    OnPropertyChanged("LSMPixelText");
                    OnPropertyChanged("LSMPixelXMax");
                    OnPropertyChanged("LSMPixelYMax");
                    OnPropertyChanged("LSMFieldSizeXUM");
                    OnPropertyChanged("LSMFieldSizeYUM");
                    OnPropertyChanged("LSMFieldSizeXMM");
                    OnPropertyChanged("LSMFieldSizeYMM");
                    OnPropertyChanged("LSMUMPerPixel");
                    OnPropertyChanged("LSMFieldSizeDisplayX");
                    OnPropertyChanged("LSMFieldSizeDisplayY");
                }
            }
        }

        public int LSMPixelXMax
        {
            get
            {
                return this._liveImage.LSMPixelXMax;
            }
            set
            {
                this._liveImage.LSMPixelXMax = value;
                OnPropertyChanged("LSMPixelXMax");
            }
        }

        public int LSMPixelXMin
        {
            get
            {
                return this._liveImage.LSMPixelXMin;
            }
        }

        public int LSMPixelY
        {
            get
            {
                return this._liveImage.LSMPixelY;
            }
            set
            {
                value = value >> 5; // divide by 32
                value = value << 5; // multiply by 32

                this._liveImage.LSMPixelY = Math.Min(value, this._liveImage.LSMPixelYMax);

                OnPropertyChanged("LSMPixelY");
                OnPropertyChanged("LSMPixelText");
                OnPropertyChanged("LSMPixelXMax");
                OnPropertyChanged("LSMPixelYMax");
                OnPropertyChanged("LSMFieldSizeXUM");
                OnPropertyChanged("LSMFieldSizeYUM");
                OnPropertyChanged("LSMFieldSizeXMM");
                OnPropertyChanged("LSMFieldSizeYMM");
                OnPropertyChanged("LSMUMPerPixel");
                OnPropertyChanged("LSMFieldSizeDisplayX");
                OnPropertyChanged("LSMFieldSizeDisplayY");
            }
        }

        public int LSMPixelYMax
        {
            get
            {
                return this._liveImage.LSMPixelYMax;
            }
            set
            {
                this._liveImage.LSMPixelYMax = value;
                OnPropertyChanged("LSMPixelYMax");
            }
        }

        public int LSMPixelYMin
        {
            get
            {
                return this._liveImage.LSMPixelYMin;
            }
        }

        public int LSMScanMode
        {
            get
            {
                return this._liveImage.LSMScanMode;
            }
            set
            {
                this._liveImage.LSMScanMode = value;
                OnPropertyChanged("LSMScanMode");
                OnPropertyChanged("LSMPixelXMax");
                OnPropertyChanged("LSMPixelYMax");
            }
        }

        public int LSMSignalAverage
        {
            get
            {
                return this._liveImage.LSMSignalAverage;
            }
            set
            {
                this._liveImage.LSMSignalAverage = value;
                OnPropertyChanged("LSMSignalAverage");
            }
        }

        public int LSMSignalAverageFrames
        {
            get
            {
                return this._liveImage.SignalAverageFrames;
            }
            set
            {
                this._liveImage.SignalAverageFrames = value;
                OnPropertyChanged("LSMSignalAverageFrames");
            }
        }

        public int LSMTwoWayAlignment
        {
            get
            {
                return this._liveImage.LSMTwoWayAlignment;
            }
            set
            {
                this._liveImage.LSMTwoWayAlignment = value;

                OnPropertyChanged("LSMTwoWayAlignment");
            }
        }

        public double LSMUMPerPixel
        {
            get
            {
                double dVal = ((this._liveImage.LSMFieldSize * _fieldSizeCalibration) / (TurretMagnification * this._liveImage.LSMPixelX));

                dVal = Math.Round(dVal, 3);
                return dVal;
            }
        }

        public int MaxChannels
        {
            get
            {
                return LiveImage.MAX_CHANNELS;
            }
        }

        public double MMPerPixel
        {
            get
            {
                switch (CamType)
                {
                    case CameraType.LSM:
                        return (LSMFieldSize * LSMFieldSizeCalibration) / (LSMPixelX * TurretMagnification * 1000.0);
                    case CameraType.CCD:
                    case CameraType.CCD_MOSAIC:
                        return 0;
                    default:
                        return 0;
                }

            }
        }

        public int NIRBoost
        {
            get
            {
                return this._camProps.NIRBoost;
            }
            set
            {
                this._camProps.NIRBoost = value;
                OnPropertyChanged("NIRBoost");
            }
        }

        public bool NIRBoostSupported
        {
            get
            {
                return this._camProps.NIRBoostSupported;
            }
        }

        public int NumAvailableCameras
        {
            get
            {
                return this._liveImage.NumberAvailableCameras();
            }
        }

        public int NumChannelsAvailableForDisplay
        {
            get;
            set;
        }

        public int OperatingMode
        {
            get
            {
                return this._camProps.OperatingMode;
            }
            set
            {
                this._camProps.OperatingMode = value;
                OnPropertyChanged("OperatingMode");
            }
        }

        public int OpticalBlackLevel
        {
            get
            {
                return this._camProps.OpticalBlackLevel;
            }
            set
            {
                this._camProps.OpticalBlackLevel = value;
                OnPropertyChanged("OpticalBlackLevel");
            }
        }

        public int OpticalBlackLevelMax
        {
            get
            {
                return this._camProps.OpticalBlackLevelMax;
            }
        }

        public int OpticalBlackLevelMin
        {
            get
            {
                return this._camProps.OpticalBlackLevelMin;
            }
        }

        public string PathBackgroundSubtraction
        {
            get
            {
                return this._liveImage.PathBackgroundSubtraction;
            }
            set
            {
                this._liveImage.PathBackgroundSubtraction = value;
                OnPropertyChanged("PathBackgroundSubtraction");
            }
        }

        public string PathFlatField
        {
            get
            {
                return this._liveImage.PathFlatField;
            }
            set
            {
                this._liveImage.PathFlatField = value;
                OnPropertyChanged("PathFlatField");
            }
        }

        public string PathSaveSnapshot
        {
            get
            {
                return this._pathSaveSnapshot;
            }
            set
            {
                this._pathSaveSnapshot = value;
                OnPropertyChanged("PathSaveSnapshot");
            }
        }

        public int PinholeMax
        {
            get
            {
                return this._liveImage.PinholeMax;
            }
        }

        public int PinholeMin
        {
            get
            {
                return this._liveImage.PinholeMin;
            }
        }

        public int PinholePosition
        {
            get
            {
                return this._liveImage.PinholePosition;
            }
            set
            {
                this._liveImage.PinholePosition = value;
                OnPropertyChanged("PinholePosition");
            }
        }

        public double PixelSizeUM
        {
            get
            {
                return this._camProps.PixelSizeUM;
            }
        }

        public int PMT1Gain
        {
            get
            {
                return this._liveImage.PMT1Gain;
            }
            set
            {
                this._liveImage.PMT1Gain = value;
                OnPropertyChanged("PMT1Gain");
            }
        }

        public int PMT1GainEnable
        {
            get
            {
                return this._liveImage.PMT1GainEnable;
            }
            set
            {
                this._liveImage.PMT1GainEnable = value;
                OnPropertyChanged("PMT1GainEnable");
            }
        }

        public int PMT1GainMax
        {
            get
            {
                return this._liveImage.PMT1GainMax;
            }
        }

        public int PMT1GainMin
        {
            get
            {
                return this._liveImage.PMT1GainMin;
            }
        }

        public int PMT2Gain
        {
            get
            {
                return this._liveImage.PMT2Gain;
            }
            set
            {
                this._liveImage.PMT2Gain = value;
                OnPropertyChanged("PMT2Gain");
            }
        }

        public int PMT2GainEnable
        {
            get
            {
                return this._liveImage.PMT2GainEnable;
            }
            set
            {
                this._liveImage.PMT2GainEnable = value;
                OnPropertyChanged("PMT2GainEnable");
            }
        }

        public int PMT2GainMax
        {
            get
            {
                return this._liveImage.PMT2GainMax;
            }
        }

        public int PMT2GainMin
        {
            get
            {
                return this._liveImage.PMT2GainMin;
            }
        }

        public int PMT3Gain
        {
            get
            {
                return this._liveImage.PMT3Gain;
            }
            set
            {
                this._liveImage.PMT3Gain = value;
                OnPropertyChanged("PMT3Gain");
            }
        }

        public int PMT3GainEnable
        {
            get
            {
                return this._liveImage.PMT3GainEnable;
            }
            set
            {
                this._liveImage.PMT3GainEnable = value;
                OnPropertyChanged("PMT3GainEnable");
            }
        }

        public int PMT3GainMax
        {
            get
            {
                return this._liveImage.PMT3GainMax;
            }
        }

        public int PMT3GainMin
        {
            get
            {
                return this._liveImage.PMT3GainMin;
            }
        }

        public int PMT4Gain
        {
            get
            {
                return this._liveImage.PMT4Gain;
            }
            set
            {
                this._liveImage.PMT4Gain = value;
                OnPropertyChanged("PMT4Gain");
            }
        }

        public int PMT4GainEnable
        {
            get
            {
                return this._liveImage.PMT4GainEnable;
            }
            set
            {
                this._liveImage.PMT4GainEnable = value;
                OnPropertyChanged("PMT4GainEnable");
            }
        }

        public int PMT4GainMax
        {
            get
            {
                return this._liveImage.PMT4GainMax;
            }
        }

        public int PMT4GainMin
        {
            get
            {
                return this._liveImage.PMT4GainMin;
            }
        }

        public bool PMTSafetyStatus
        {
            get
            {
                return this._liveImage.PMTSafetyStatus;
            }
        }

        public int PowerCalculated
        {
            get
            {
                int val = 0;

                if (_powerMode == 1)
                {
                    double b;

                    if (_powerStart == 0)
                    {
                        b = Math.Log(_powerStop / 1.0) / (_zScanStop - _zScanStart);
                    }
                    else if ((_zScanStop - _zScanStart) == 0)
                    {
                        b = Math.Log(_powerStop / (double)_powerStart) / (.001);
                    }
                    else
                    {
                        b = Math.Log(_powerStop / (double)_powerStart) / (double)(_zScanStop - _zScanStart);
                    }

                    double a = _powerStart * Math.Pow(Math.E, -1.0 * b * _zScanStart);

                    double maxPower = Math.Max(_powerStart, _powerStop);
                    double minPower = Math.Min(_powerStart, _powerStop);

                    double result = Math.Min(maxPower, Math.Max(minPower, a * Math.Pow(Math.E, b * ZPosition)));

                    val = Convert.ToInt32(result);
                }

                return val;
            }
        }

        public int PowerMax
        {
            get
            {
                return this._liveImage.PowerMax;
            }
        }

        public int PowerMin
        {
            get
            {
                return this._liveImage.PowerMin;
            }
        }

        public int PowerMode
        {
            get
            {
                return _powerMode;
            }
            set
            {
                _powerMode = value;
                OnPropertyChanged("PowerMode");
                OnPropertyChanged("PowerCalculated");
            }
        }

        public int PowerPockelsBlankPercentage
        {
            get
            {
                return this._liveImage.PowerPockelsBlankPercentage;
            }
            set
            {
                this._liveImage.PowerPockelsBlankPercentage = value;
                OnPropertyChanged("PowerPockelsBlankPercentage");
            }
        }

        public int PowerPosition
        {
            get
            {
                return this._liveImage.PowerPosition;
            }
            set
            {
                TimeSpan ts = DateTime.Now - _lastPowerUpdateTime;

                if (ts.TotalSeconds > 1)
                {
                    this._liveImage.PowerPosition = value;
                    OnPropertyChanged("PowerPosition");
                    _lastPowerUpdateTime = DateTime.Now;
                }
            }
        }

        public int PowerStart
        {
            get
            {
                return _powerStart;
            }
            set
            {
                _powerStart = value;
                OnPropertyChanged("PowerStart");
                OnPropertyChanged("PowerCalculated");

            }
        }

        public int PowerStop
        {
            get
            {
                return _powerStop;
            }
            set
            {
                _powerStop = value;
                OnPropertyChanged("PowerStop");
                OnPropertyChanged("PowerCalculated");

            }
        }

        public CollectionView ReadOutSpeedEntries
        {
            get
            {
                return this._camProps.ReadOutSpeedEntries;
            }
        }

        public int ReadOutSpeedIndex
        {
            get
            {
                return this._camProps.ReadOutSpeedIndex;
            }
            set
            {
                this._camProps.ReadOutSpeedIndex = value;
                OnPropertyChanged("ReadOutSpeedIndex");
            }
        }

        public int ReadOutSpeedIndexMax
        {
            get
            {
                return this._camProps.ReadOutSpeedIndexMax;
            }
        }

        public int ReadOutSpeedIndexMin
        {
            get
            {
                return this._camProps.ReadOutSpeedIndexMin;
            }
        }

        public double ReadOutSpeedValue
        {
            get
            {
                return this._camProps.ReadOutSpeedValue;
            }
        }

        public CollectionView ReadOutTapEntries
        {
            get
            {
                return this._camProps.ReadOutTapEntries;
            }
        }

        public int ReadOutTapIndex
        {
            get
            {
                return this._camProps.ReadOutTapIndex;
            }
            set
            {
                this._camProps.ReadOutTapIndex = value;
                OnPropertyChanged("ReadOutTapIndex");
            }
        }

        public int ReadOutTapIndexMax
        {
            get
            {
                return this._camProps.ReadOutTapIndexMax;
            }
        }

        public int ReadOutTapIndexMin
        {
            get
            {
                return this._camProps.ReadOutTapIndexMin;
            }
        }

        public int ReadOutTapValue
        {
            get
            {
                return this._camProps.ReadOutTapValue;
            }
        }

        public int Right
        {
            get
            {
                return this._camProps.Right;
            }
            set
            {
                this._camProps.Right = value;
                OnPropertyChanged("Right");
            }
        }

        public int RollOverPointIntensity0
        {
            get
            {
                return this._liveImage.RollOverPointIntensity0;
            }
        }

        public int RollOverPointIntensity1
        {
            get
            {
                return this._liveImage.RollOverPointIntensity1;
            }
        }

        public int RollOverPointIntensity2
        {
            get
            {
                return this._liveImage.RollOverPointIntensity2;
            }
        }

        public int RollOverPointIntensity3
        {
            get
            {
                return this._liveImage.RollOverPointIntensity3;
            }
        }

        public int RollOverPointX
        {
            get
            {
                return this._liveImage.RollOverPointX;
            }
            set
            {
                int val = value;

                if (TileDisplay)
                {
                    val = val % this._liveImage.DataWidth;
                }

                this._liveImage.RollOverPointX = val;
            }
        }

        public int RollOverPointY
        {
            get
            {
                return this._liveImage.RollOverPointY;
            }
            set
            {
                int val = value;

                if (TileDisplay)
                {
                    val = val % this._liveImage.DataHeight;
                }

                this._liveImage.RollOverPointY = val;
            }
        }

        public double SampleOffsetXMM
        {
            get
            {
                return _sampleOffsetXMM;
            }
            set
            {
                _sampleOffsetXMM = value;
                OnPropertyChanged("SampleOffsetXMM");
                CreatePlate();
            }
        }

        public double SampleOffsetYMM
        {
            get
            {
                return _sampleOffsetYMM;
            }
            set
            {
                _sampleOffsetYMM = value;
                OnPropertyChanged("SampleOffsetYMM");
                CreatePlate();
            }
        }

        public ICommand SaveNowCommand
        {
            get
            {
                if (this._saveNowCommand == null)
                    this._saveNowCommand = new RelayCommand(() => SaveNow());

                return this._saveNowCommand;
            }
        }

        public ICommand SelectBackgroundCommand
        {
            get
            {
                if (this._selectBackgroundCommand == null)
                    this._selectBackgroundCommand = new RelayCommand(() => SelectBackground());

                return this._selectBackgroundCommand;
            }
        }

        public int SelectedSampleType
        {
            get
            {
                return _sampleType;
            }

            set
            {
                if (value >= 0)
                {
                    _sampleType = value;
                    OnPropertyChanged("SelectedSampleType");
                    CreatePlate();
                }
            }
        }

        public int SelectedSubColumn
        {
            get
            {
                return _selectedSubColumn;
            }
            set
            {
                _selectedSubColumn = value;
                OnPropertyChanged("SelectedSubColumn");
            }
        }

        public int SelectedSubRow
        {
            get
            {
                return _selectedSubRow;
            }
            set
            {
                _selectedSubRow = value;
                OnPropertyChanged("SelectedSubRow");
            }
        }

        public int SelectedWellColumn
        {
            get;
            set;
        }

        public int SelectedWellColumnCount
        {
            get
            {
                return _selectedWellColumnCount;
            }
            set
            {
                _selectedWellColumnCount = value;
            }
        }

        public int SelectedWellRow
        {
            get;
            set;
        }

        public int SelectedWellRowCount
        {
            get
            {
                return _selectedWellRowCount;
            }
            set
            {
                _selectedWellRowCount = value;
            }
        }

        public ICommand SelectFlatFieldCommand
        {
            get
            {
                if (this._selectFlatFieldCommand == null)
                    this._selectFlatFieldCommand = new RelayCommand(() => SelectFlatField());

                return this._selectFlatFieldCommand;
            }
        }

        public ICommand SetPowerStartCommand
        {
            get
            {
                if (this._setPowerStartCommand == null)
                    this._setPowerStartCommand = new RelayCommand(() => SetPowerStart());

                return this._setPowerStartCommand;
            }
        }

        public ICommand SetPowerStopCommand
        {
            get
            {
                if (this._setPowerStopCommand == null)
                    this._setPowerStopCommand = new RelayCommand(() => SetPowerStop());

                return this._setPowerStopCommand;
            }
        }

        public ICommand SetSampleOffsetCommand
        {
            get
            {
                if (this._setSampleOffsetCommand == null)
                    this._setSampleOffsetCommand = new RelayCommand(() => SetSampleOffset());

                return this._setSampleOffsetCommand;
            }
        }

        public ICommand SetZScanStartCommand
        {
            get
            {
                if (this._setZScanStartCommand == null)
                    this._setZScanStartCommand = new RelayCommand(() => SetZScanStart());

                return this._setZScanStartCommand;
            }
        }

        public ICommand SetZScanStopCommand
        {
            get
            {
                if (this._setZScanStopCommand == null)
                    this._setZScanStopCommand = new RelayCommand(() => SetZScanStop());

                return this._setZScanStopCommand;
            }
        }

        public ICommand SetZZeroCommand
        {
            get
            {
                if (this._setZZeroCommand == null)
                    this._setZZeroCommand = new RelayCommand(() => SetZZero());

                return this._setZZeroCommand;
            }
        }

        public int ShutterPosition
        {
            get
            {
                return this._liveImage.ShutterPosition;
            }
            set
            {
                this._liveImage.ShutterPosition = value;
                OnPropertyChanged("ShutterPosition");
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

        public string SnapshotImagePrefix
        {
            get
            {
                return this._snapshotImagePrefix;
            }
            set
            {
                this._snapshotImagePrefix = value;
                OnPropertyChanged("SnapshotImagePrefix");
            }
        }

        public int StartColumn
        {
            get;
            set;
        }

        public ICommand StartCommand
        {
            get
            {
                if (this._startCommand == null)
                    this._startCommand = new RelayCommand(() => Start());

                return this._startCommand;
            }
        }

        public int StartRow
        {
            get;
            set;
        }

        public ICommand StopCommand
        {
            get
            {
                if (this._stopCommand == null)
                    this._stopCommand = new RelayCommand(() => Stop());

                return this._stopCommand;
            }
        }

        public ICommand StopZCommand
        {
            get
            {
                if (this._stopZCommand == null)
                    this._stopZCommand = new RelayCommand(() => StopZ());

                return this._stopZCommand;
            }
        }

        public int SubColumns
        {
            get
            {
                return this._subColumns;
            }
            set
            {
                if ((value <= SubColumnsMax) && (value > 0))
                {
                    this._subColumns = value;
                    OnPropertyChanged("SubColumns");
                    CreatePlate();
                }
            }
        }

        public int SubColumnsMax
        {
            get;
            set;
        }

        public double SubOffsetX
        {
            get
            {
                return _subOffsetX; //MMPerPixel * LSMPixelX * (1 + SubSpacingXPercent / 100.0);
            }
            set
            {
                _subOffsetX = value;
                OnPropertyChanged("SubOffsetX");
                CreatePlate();
            }
        }

        public double SubOffsetY
        {
            get
            {
                return _subOffsetY; //MMPerPixel * LSMPixelY * (1 + SubSpacingYPercent / 100.0);
            }
            set
            {
                _subOffsetY = value;
                OnPropertyChanged("SubOffsetY");
                CreatePlate();
            }
        }

        public int SubRows
        {
            get
            {
                return this._subRows;
            }
            set
            {
                if ((value <= SubRowsMax) && (value > 0))
                {
                    this._subRows = value;
                    OnPropertyChanged("SubRows");
                    CreatePlate();
                }
            }
        }

        public int SubRowsMax
        {
            get;
            set;
        }

        public double SubSpacingXPercent
        {
            get
            {
                return this._subSpacingXPercent;
            }
            set
            {
                this._subSpacingXPercent = value;
                OnPropertyChanged("SubSpacingXPercent");
                OnPropertyChanged("SubOffsetX");
                CreatePlate();
            }
        }

        public double SubSpacingYPercent
        {
            get
            {
                return this._subSpacingYPercent;
            }
            set
            {
                this._subSpacingYPercent = value;
                OnPropertyChanged("SubSpacingYPercent");
                OnPropertyChanged("SubOffsetY");
                CreatePlate();
            }
        }

        public int TapBalance
        {
            get
            {
                return this._camProps.TapBalance;
            }
            set
            {
                this._camProps.TapBalance = value;
            }
        }

        public double TDIHeightMM
        {
            get
            {
                return this._tdiHeightMM;
            }
            set
            {
                this._tdiHeightMM = value;
                OnPropertyChanged("TDIHeightMM");
            }
        }

        public int TDILineShifts
        {
            get
            {
                return this._camProps.TDILineShifts;
            }
            set
            {
                this._camProps.TDILineShifts = value;
                OnPropertyChanged("TDILineShifts");
            }
        }

        public int TDILineTrim
        {
            get
            {
                return this._camProps.TDILineTrim;
            }
            set
            {
                this._camProps.TDILineTrim = value;
                OnPropertyChanged("TDILineTrim");
            }
        }

        public int TDITriggers
        {
            get
            {
                return this._camProps.TDITriggers;
            }
            set
            {
                this._camProps.TDITriggers = value;
                OnPropertyChanged("TDITriggers");
            }
        }

        public int TDITrimMode
        {
            get
            {
                return this._camProps.TDITrimMode;
            }
            set
            {
                this._camProps.TDITrimMode = value;
                OnPropertyChanged("TDITrimMode");
            }
        }

        public double TDIWidthMM
        {
            get
            {
                return this._tdiWidthMM;
            }
            set
            {
                this._tdiWidthMM = value;
                OnPropertyChanged("TDIWidthMM");
            }
        }

        public int TileControlMode
        {
            get
            {
                return _tileControlMode;
            }
            set
            {
                _tileControlMode = value;
                OnPropertyChanged("TileControlMode");
            }
        }

        public bool TileDisplay
        {
            get;
            set;
        }

        public int Top
        {
            get
            {
                return this._camProps.Top;
            }
            set
            {
                this._camProps.Top = value;
                this.Height = this._height;
                OnPropertyChanged("Top");
                OnPropertyChanged("Height");
            }
        }

        public double TransformX
        {
            get
            {
                if (_transformX < 0)
                {
                    _transformX = 100;
                }
                else
                {
                    _transformX = -100;
                }
                return _transformX;
            }
            set
            {
                if (_transformX < 0)
                {
                    _transformX = 100;
                }
                else
                {
                    _transformX = -100;
                }
            }
        }

        public double TransformY
        {
            get
            {
                if (_transformY < 0)
                {
                    _transformY = 100;
                }
                else
                {
                    _transformY = -100;
                }
                return _transformY;
            }
            set
            {
                if (_transformY < 0)
                {
                    _transformY = 100;
                }
                else
                {
                    _transformY = -100;
                }
            }
        }

        public double TransOffsetXMM
        {
            get
            {
                return this._transOffsetXMM;
            }
            set
            {
                this._transOffsetXMM = value;
                OnPropertyChanged("TransOffsetXMM");
                CreatePlate();

            }
        }

        public double TransOffsetYMM
        {
            get
            {
                return this._transOffsetYMM;
            }
            set
            {
                this._transOffsetYMM = value;
                OnPropertyChanged("TransOffsetYMM");
                CreatePlate();
            }
        }

        public string TurretBeamExpansion
        {
            get
            {
                string str = "1.0X";
                int beamExp = 0;

                if (_hardwareDoc != null)
                {
                    XmlNodeList ndList = _hardwareDoc.GetElementsByTagName("Objective");

                    if (TurretPosition <= ndList.Count)
                    {
                        beamExp = Convert.ToInt32(ndList[TurretPosition].Attributes["beamExp"].Value);
                    }
                }

                switch (beamExp)
                {
                    case 0: str = "1.0X"; break;
                    case 1: str = "1.5X"; break;
                    case 2: str = "2.0X"; break;
                    case 3: str = "2.5X"; break;
                    case 4: str = "3.0X"; break;
                    case 5: str = "3.5X"; break;
                    case 6: str = "3.8X"; break;
                }

                return str;
            }
        }

        public double TurretMagnification
        {
            get
            {
                double mag = 1.0;

                if (_hardwareDoc != null)
                {
                    XmlNodeList ndList = _hardwareDoc.GetElementsByTagName("Objective");

                    if (TurretPosition <= ndList.Count)
                    {
                        mag = Convert.ToDouble(ndList[TurretPosition].Attributes["mag"].Value);
                    }
                }

                return mag;
            }
        }

        public int TurretPosition
        {
            get
            {
                return this._liveImage.TurretPosition;
            }
            set
            {
                this._liveImage.TurretPosition = value;
                OnPropertyChanged("TurretPosition");
                OnPropertyChanged("LSMFieldSizeXUM");
                OnPropertyChanged("LSMFieldSizeYUM");
                OnPropertyChanged("LSMFieldSizeXMM");
                OnPropertyChanged("LSMFieldSizeYMM");
                OnPropertyChanged("LSMUMPerPixel");
                OnPropertyChanged("PinholePosition");
                OnPropertyChanged("TurretBeamExpansion");
            }
        }

        public Color WavelengthColor
        {
            get
            {
                return this._liveImage.WavelengthColor;
            }
            set
            {
                this._liveImage.WavelengthColor = value;
                _paletteChanged = true;
                OnPropertyChanged("WavelengthColor");
            }
        }

        public string WavelengthName
        {
            get
            {
                return _wavelengthName;
            }
            set
            {
                _wavelengthName = value;
                OnPropertyChanged("WavelengthName");
                OnPropertyChanged("WhitePoint");
                OnPropertyChanged("BlackPoint");
            }
        }

        public double WellOffsetXMM
        {
            get
            {
                return _wellOffsetXMM;
            }
            set
            {
                _wellOffsetXMM = value;
                OnPropertyChanged("WellOffsetXMM");
                CreatePlate();
            }
        }

        public double WellOffsetYMM
        {
            get
            {
                return _wellOffsetYMM;
            }
            set
            {
                _wellOffsetYMM = value;
                OnPropertyChanged("WellOffsetYMM");
                CreatePlate();
            }
        }

        public double WhitePoint0
        {
            get
            {
                return this._liveImage.WhitePoint0;
            }
            set
            {
                if (value != this._liveImage.WhitePoint0)
                {
                    this._liveImage.WhitePoint0 = value;
                    _paletteChanged = true;
                    OnPropertyChanged("WhitePoint0");
                    OnPropertyChanged("Bitmap");
                }
            }
        }

        public double WhitePoint1
        {
            get
            {
                return this._liveImage.WhitePoint1;
            }
            set
            {
                if (value != this._liveImage.WhitePoint1)
                {
                    this._liveImage.WhitePoint1 = value;
                    _paletteChanged = true;
                    OnPropertyChanged("WhitePoint1");
                    OnPropertyChanged("Bitmap");
                }
            }
        }

        public double WhitePoint2
        {
            get
            {
                return this._liveImage.WhitePoint2;
            }
            set
            {
                if (value != this._liveImage.WhitePoint2)
                {
                    this._liveImage.WhitePoint2 = value;
                    _paletteChanged = true;
                    OnPropertyChanged("WhitePoint2");
                    OnPropertyChanged("Bitmap");
                }
            }
        }

        public double WhitePoint3
        {
            get
            {
                return this._liveImage.WhitePoint3;
            }
            set
            {
                if (value != this._liveImage.WhitePoint3)
                {
                    this._liveImage.WhitePoint3 = value;
                    _paletteChanged = true;
                    OnPropertyChanged("WhitePoint3");
                    OnPropertyChanged("Bitmap");
                }
            }
        }

        public int Width
        {
            get
            {
                return _width;
            }
            set
            {
                //validate the width and restrict it to only valid values
                if (0 < value)
                {
                    int left = this.Left;
                    int newRight = Math.Min(this.CameraWidth, left + value);
                    this._width = newRight - left;
                    this.Right = newRight;
                    OnPropertyChanged("Width");
                    OnPropertyChanged("Right");
                }
            }
        }

        public int XDirection
        {
            get
            {
                return _xDirection;
            }
            set
            {
                _xDirection = value;
            }
        }

        public double XMax
        {
            get
            {
                return this._liveImage.XMax;
            }
        }

        public double XMin
        {
            get
            {
                return this._liveImage.XMin;
            }
        }

        public double XPosition
        {
            get
            {
                return this._liveImage.XPosition;
            }
            set
            {
                this._liveImage.XPosition = value;
                OnPropertyChanged("XPosition");
            }
        }

        public double XStepSize
        {
            get
            {
                return _xStepSize;
            }
            set
            {
                Decimal dec = new Decimal(value);
                _xStepSize = Decimal.ToDouble(Decimal.Round(dec, 4));

                OnPropertyChanged("XStepSize");
            }
        }

        public int YDirection
        {
            get
            {
                return _yDirection;
            }
            set
            {
                _yDirection = value;
            }
        }

        public double YMax
        {
            get
            {
                return this._liveImage.YMax;
            }
        }

        public double YMin
        {
            get
            {
                return this._liveImage.YMin;
            }
        }

        public double YPosition
        {
            get
            {
                return this._liveImage.YPosition;
            }
            set
            {
                this._liveImage.YPosition = value;
                OnPropertyChanged("YPosition");
            }
        }

        public double YStepSize
        {
            get
            {
                return _yStepSize;
            }
            set
            {
                Decimal dec = new Decimal(value);
                _yStepSize = Decimal.ToDouble(Decimal.Round(dec, 4));

                OnPropertyChanged("YStepSize");
            }
        }

        public double ZMax
        {
            get
            {
                return _sliderZMax;
            }
            set
            {
                _sliderZMax = Math.Min(value, this._liveImage.ZMax);
                OnPropertyChanged("ZMax");
                OnPropertyChanged("ZSliderStepSize");
            }
        }

        public double ZMin
        {
            get
            {
                return _sliderZMin;
            }
            set
            {
                _sliderZMin = Math.Max(value, this._liveImage.ZMin); ;
                OnPropertyChanged("ZMin");
                OnPropertyChanged("ZSliderStepSize");
            }
        }

        public double ZPosition
        {
            get
            {
                return this._liveImage.ZPosition;
            }
            set
            {
                TimeSpan ts = DateTime.Now - _lastZUpdateTime;

                if (ts.TotalSeconds > .5)
                {
                    this._liveImage.ZPosition = value;

                    OnPropertyChanged("ZPosition");
                    OnPropertyChanged("PowerCalculated");
                    _lastZUpdateTime = DateTime.Now;
                }
            }
        }

        public double ZRangeMax
        {
            get
            {
                return _zRangeMax = Double.Parse(String.Format("{0:0.###}", _zRangeMax));  // "123.456";
            }
            set
            {
                if (value <= this._liveImage.ZMin)
                {
                    _zRangeMax = this._liveImage.ZMin;
                }
                else if (value >= this._liveImage.ZMax)
                {
                    _zRangeMax = this._liveImage.ZMax;
                }
                else
                {
                    _zRangeMax = value;
                }

                if (_zRangeDirection == 0)
                {
                    _zScanStop = _zRangeMax;
                    OnPropertyChanged("ZScanStop");
                }
                else
                {
                    _zScanStart = _zRangeMax;
                    OnPropertyChanged("ZScanStart");
                }

                OnPropertyChanged("ZRangeMax");
                OnPropertyChanged("ZScanNumSteps");
                OnPropertyChanged("ZScanThickness");
                OnPropertyChanged("PowerCalculated");
            }
        }

        public double ZRangeMin
        {
            get
            {
                return _zRangeMin = Double.Parse(String.Format("{0:0.###}", _zRangeMin));  // "123.456";
            }
            set
            {
                if (value <= this._liveImage.ZMin)
                {
                    _zRangeMin = this._liveImage.ZMin;
                }
                else if (value >= this._liveImage.ZMax)
                {
                    _zRangeMin = this._liveImage.ZMax;
                }
                else
                {
                    _zRangeMin = value;
                }

                if (_zRangeDirection == 0)
                {
                    _zScanStart = _zRangeMin;
                    OnPropertyChanged("ZScanStart");
                }
                else
                {
                    _zScanStop = _zRangeMin;
                    OnPropertyChanged("ZScanStop");
                }

                OnPropertyChanged("ZRangeMin");
                OnPropertyChanged("ZScanNumSteps");
                OnPropertyChanged("ZScanThickness");
                OnPropertyChanged("PowerCalculated");
            }
        }

        public int ZScanNumSteps
        {
            get
            {
                const double UM_TO_MM = .001;
                _zScanNumSteps = (int)Math.Abs((_zScanStart - _zScanStop) / (_zScanStep * UM_TO_MM));
                return _zScanNumSteps;
            }
        }

        public double ZScanStart
        {
            get
            {

                return _zScanStart = Double.Parse(String.Format("{0:0.###}", _zScanStart));  // "123.456";
            }
            set
            {
                if (value <= this._liveImage.ZMin)
                {
                    _zScanStart = this._liveImage.ZMin;
                }
                else if (value >= this._liveImage.ZMax)
                {
                    _zScanStart = this._liveImage.ZMax;
                }
                else
                {
                    _zScanStart = value;
                }

                if (_zScanStart > _zScanStop)
                {
                    _zRangeDirection = 1;
                    _zRangeMin = _zScanStop;
                    _zRangeMax = _zScanStart;
                }
                else
                {
                    _zRangeDirection = 0;
                    _zRangeMin = _zScanStart;
                    _zRangeMax = _zScanStop;
                }

                OnPropertyChanged("ZRangeMax");
                OnPropertyChanged("ZRangeMin");

                OnPropertyChanged("ZScanStart");
                OnPropertyChanged("ZScanNumSteps");
                OnPropertyChanged("ZScanThickness");
                OnPropertyChanged("PowerCalculated");
            }
        }

        public double ZScanStep
        {
            get
            {
                return _zScanStep;
            }
            set
            {
                if (value != 0)
                {
                    _zScanStep = value;

                    OnPropertyChanged("ZScanStep");
                    OnPropertyChanged("ZScanThickness");
                    OnPropertyChanged("ZScanNumSteps");
                }
            }
        }

        public double ZScanStop
        {
            get
            {
                return _zScanStop = Double.Parse(String.Format("{0:0.###}", _zScanStop));
            }
            set
            {
                if (value <= this._liveImage.ZMin)
                {
                    _zScanStop = this._liveImage.ZMin;
                }
                else if (value >= this._liveImage.ZMax)
                {
                    _zScanStop = this._liveImage.ZMax;
                }
                else
                {
                    _zScanStop = value;
                }

                if (_zScanStop > _zScanStart)
                {
                    _zRangeDirection = 0;
                    _zRangeMin = _zScanStart;
                    _zRangeMax = _zScanStop;
                }
                else
                {
                    _zRangeDirection = 1;
                    _zRangeMin = _zScanStop;
                    _zRangeMax = _zScanStart;
                }

                OnPropertyChanged("ZRangeMax");
                OnPropertyChanged("ZRangeMin");

                OnPropertyChanged("ZScanStop");
                OnPropertyChanged("ZScanNumSteps");
                OnPropertyChanged("ZScanThickness");
                OnPropertyChanged("PowerCalculated");
            }
        }

        public double ZScanThickness
        {
            get
            {
                const double MM_TO_UM = 1000;
                double val = _zScanStart - _zScanStop;

                _zScanThickness = Math.Abs(Double.Parse(String.Format("{0:0.###}", val * MM_TO_UM)));  // "123.456";
                return _zScanThickness;
            }
        }

        public double ZSliderStepSize
        {
            get
            {
                return (ZMax - ZMin) / 20.0;
            }
        }

        public double ZStepSize
        {
            get
            {
                return _zStepSize;
            }
            set
            {
                Decimal dec = new Decimal(value);
                _zStepSize = Decimal.ToDouble(Decimal.Round(dec, 4));

                OnPropertyChanged("ZStepSize");
            }
        }

        public double ZTickFrequency
        {
            get
            {
                return _sliderTickFrequency;
            }
            set
            {
            }
        }

        #endregion Properties
    }
}