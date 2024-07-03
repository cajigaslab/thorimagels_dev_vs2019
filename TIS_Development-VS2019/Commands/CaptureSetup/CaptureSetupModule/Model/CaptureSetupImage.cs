namespace CaptureSetupDll.Model
{
    using System;
    using System.Collections.Concurrent;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Threading;
    using System.Threading.Tasks;
    using System.Timers;
    using System.Windows;
    using System.Windows.Data;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Threading;
    using System.Xml;

    using ThorLogging;

    using ThorSharedTypes;

    public partial class CaptureSetup : INotifyPropertyChanged
    {
        #region Fields

        public const int MAX_CHANNELS = 4;

        private const int DEFAULT_BITS_PER_PIXEL = 14;

        private static readonly object _dflimDiagnosticsDataLock = new object();
        private static readonly object _dflimHistogramDataLock = new object();
        static readonly FrameData _frameData = new FrameData();

        private static int _dataHeight = 0;
        private static int _dataWidth = 0;
        private static uint[] _dflimArrivalTimeSumData;
        private static double[] _dFLIMBinDurations = new double [MAX_CHANNELS];
        private static int _dflimDiagnosticsBufferLength = 128;
        private static ushort[][] _dflimDiagnosticsData;
        private static uint[][] _dflimHistogramData;
        private static bool _dflimNewHistogramData;
        private static ushort[] _dflimSinglePhotonData;
        private static ReportNewImage _imageCallBack;
        static bool _imageDataUpdated = false;
        private static bool _newDFLIMDiagnosticsData;
        private static ushort[] _pixelData;

        #endregion Fields

        #region Delegates

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void ReportNewImage(IntPtr returnArray, ref FrameInfoStruct imageInfo);

        #endregion Delegates

        #region Properties

        public static bool ImageDataUpdated
        {
            get => _imageDataUpdated;
            set => _imageDataUpdated = value;
        }

        public static FrameInfoStruct ImageInfo
        {
            get;
            set;
        }

        public int DataHeight
        {
            get => _dataHeight;
        }

        public int DataWidth
        {
            get => _dataWidth;
        }

        public int DFLIMDiagnosticsBufferLength
        {
            get
            {
                return _dflimDiagnosticsBufferLength;
            }
            set
            {
                _dflimDiagnosticsBufferLength = value;
            }
        }

        public ushort[][] DFLIMDiagnosticsData
        {
            get
            {
                return _dflimDiagnosticsData;
            }
        }

        public object DFLIMDiagnosticsDataLock
        {
            get
            {
                return _dflimDiagnosticsDataLock;
            }
        }

        public uint[][] DFLIMHistogramData
        {
            get
            {
                return _dflimHistogramData;
            }
        }

        public object DFLIMHistogramDataLock
        {
            get
            {
                return _dflimHistogramDataLock;
            }
        }

        public bool DFLIMNewHistogramData
        {
            get
            {
                return _dflimNewHistogramData;
            }
            set
            {
                _dflimNewHistogramData = value;
            }
        }

        public bool NewDFLIMDiagnosticsData
        {
            get
            {
                return _newDFLIMDiagnosticsData;
            }
            set
            {
                _newDFLIMDiagnosticsData = value;
            }
        }

        #endregion Properties

        #region Methods

        public static int GetBitsPerPixel()
        {
            int camType = ResourceManagerCS.GetCameraType();
            int val = DEFAULT_BITS_PER_PIXEL;
            if (((int)ICamera.CameraType.LSM != camType) && ((int)ICamera.CameraType.LAST_CAMERA_TYPE) != camType)
            {
                GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_BITS_PER_PIXEL, ref val);
            }
            return val;
        }

        [DllImport(".\\ThorDiskIO.dll", EntryPoint = "ReadChannelImages")]
        public static extern int ReadChannelImages([MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.LPWStr)]string[] fileNames, int size, ref IntPtr outputBuffer, int cameraWidth, int cameraHeight);

        private static BitmapPalette BuildPaletteGrayscale()
        {
            Color[] colors = new Color[256];
            for (int i = 0; i < colors.Length; i++)
            {
                double a = 1.0;
                double b = 0;

                double dvalR = (a * i * (Colors.White.R) / 255.0) + b;
                dvalR = Math.Max(dvalR, 0);
                dvalR = Math.Min(dvalR, 255);

                double dvalG = (a * i * (Colors.White.G) / 255.0) + b;
                dvalG = Math.Max(dvalG, 0);
                dvalG = Math.Min(dvalG, 255);

                double dvalB = (a * i * (Colors.White.B) / 255.0) + b;
                dvalB = Math.Max(dvalB, 0);
                dvalB = Math.Min(dvalB, 255);

                //Display Blue/Red at Min/Max pixel value for single channel:
                if (i == 0)
                {
                    dvalB = 255;
                }
                if (i == 255)
                {
                    dvalG = 0;
                    dvalB = 0;
                }
                colors[i] = Color.FromRgb((byte)dvalR, (byte)dvalG, (byte)dvalB);
            }

            return new BitmapPalette(colors);
        }

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "EnableCopyToExternalBuffer")]
        private static extern bool EnableCopyToExternalBuffer();

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "GetDisplayChannels")]
        private static extern int GetDisplayChannels();

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "GetImageDimensions")]
        private static extern bool GetImageDimensions(ref int width, ref int height);


        private static void ImageUpdate(IntPtr returnArray, ref FrameInfoStruct imageInfo)
        {
            lock (_frameData.dataLock)
            {
                //only copy to the buffer when pixel data is not being read

                //_framesPerSecond = 1000.0 / (Environment.TickCount - _prevTickCount);
                int width = imageInfo.imageWidth;
                int height = imageInfo.imageHeight;

                int planes = imageInfo.numberOfPlanes > 1 ? imageInfo.numberOfPlanes : 1;

                //width *= planes;
                height *= planes;
                var dataLength = width * height;
                var colorChannels = imageInfo.channels;
                if ((_pixelData == null) || _pixelData.Length != dataLength * colorChannels)
                {
                    _pixelData = new ushort[dataLength * colorChannels];
                }

                if ((int)ICamera.CameraType.CCD == ResourceManagerCS.GetCameraType())
                {
                    IMVM cameraControlViewModel = (IMVM)MVMManager.Instance["CameraControlViewModel"];
                    if (_dataWidth != width)
                    {
                        cameraControlViewModel?.OnPropertyChange("CamImageWidth");
                    }
                    if (_dataHeight != height)
                    {
                        cameraControlViewModel?.OnPropertyChange("CamImageHeight");
                    }

                    // TODO: does this impact SLM default display channels?
                    int activeChannels = 1;
                    for(int i = 0; i < imageInfo.channels; i++)
                    {
                        activeChannels *= 2;
                    }
                    activeChannels--;
                    _ = SetDisplayChannels((int)activeChannels);
                }

                _dataWidth = width;
                _dataHeight = height;
                //2^n  == FULL_RANGE_NORMALIZATION_FACTOR

                const int DFLIM_HISTOGRAM_BINS = 256;
                //256 items for dflimHistogram + 2 shorts per histogram item

                //DFLIM image type (including singlePhotonData, arrivalTimeSumData, and histogramData buffers)
                if ((int)ThorSharedTypes.BufferType.DFLIM_IMAGE == imageInfo.bufferType ||
                    (int)ThorSharedTypes.BufferType.DFLIM_ALL == imageInfo.bufferType)
                {

                    if ((_dflimSinglePhotonData == null) ||
                        (_dflimSinglePhotonData.Length != (dataLength * colorChannels)))
                    {
                        _dflimSinglePhotonData = new ushort[dataLength * colorChannels];
                    }
                    if ((_dflimArrivalTimeSumData == null) ||
                        (_dflimArrivalTimeSumData.Length != (dataLength * colorChannels)))
                    {
                        _dflimArrivalTimeSumData = new uint[dataLength * colorChannels];
                    }

                    const int SHORTS_PER_BIN = 2;
                    //copy out dflim histogram buffer
                    lock (_dflimHistogramDataLock)
                    {
                        if (_dflimHistogramData == null || _dflimHistogramData.Length != colorChannels)
                        {
                            _dflimHistogramData = new uint[colorChannels][];
                            for (int i = 0; i < _dflimHistogramData.Length; ++i)
                            {
                                _dflimHistogramData[i] = new uint[DFLIM_HISTOGRAM_BINS];
                            }
                        }

                        for (int i = 0; i < colorChannels; ++i)
                        {
                            MemoryCopyManager.CopyIntPtrMemory(returnArray, i * DFLIM_HISTOGRAM_BINS, _dflimHistogramData[i], 0, DFLIM_HISTOGRAM_BINS);
                        }

                        for (int k = 0; k < MAX_CHANNELS; k++)
                        {
                            int histoIndex = (colorChannels > 1) ? k : 0;

                            if (null == _dflimHistogramData || _dflimHistogramData[histoIndex].Length < 256 ||
                                0 == _dflimHistogramData[histoIndex][254] || 0 == _dflimHistogramData[histoIndex][255])
                            {
                                continue;
                            }
                            _dFLIMBinDurations[k] = 5.0 * (double)_dflimHistogramData[histoIndex][255] / 128.0 / (double)_dflimHistogramData[histoIndex][254];

                            if (colorChannels == 1)
                            {
                                break;
                            }
                        }

                        _dflimNewHistogramData = true;
                    }

                    //copy out the pixel data
                    MemoryCopyManager.CopyIntPtrMemory(returnArray, DFLIM_HISTOGRAM_BINS * SHORTS_PER_BIN * colorChannels, _pixelData, 0, dataLength * colorChannels);

                    //copy out dflim single photon data buffer
                    MemoryCopyManager.CopyIntPtrMemory(returnArray, DFLIM_HISTOGRAM_BINS * SHORTS_PER_BIN * colorChannels + dataLength * colorChannels, _dflimSinglePhotonData, 0, dataLength * colorChannels);

                    //copy out dflim arrival time sum data buffer
                    MemoryCopyManager.CopyIntPtrMemory(returnArray, DFLIM_HISTOGRAM_BINS * colorChannels + dataLength * colorChannels, _dflimArrivalTimeSumData, 0, dataLength * colorChannels);

                }
                //Dflim diagnostic mode
                else if ((int)ThorSharedTypes.BufferType.DFLIM_DIAGNOSTIC == imageInfo.bufferType)
                {
                    //copy out the pixel data
                    MemoryCopyManager.CopyIntPtrMemory(returnArray, 0, _pixelData, 0, dataLength * colorChannels);
                    //when in dflim diagnositc mode we grab the first _dflimDiagnosticsBufferLength pixels
                    //and then plot them
                    if (_dflimDiagnosticsData == null || _dflimDiagnosticsData.Length != colorChannels)
                    {
                        _dflimDiagnosticsData = new ushort[colorChannels][];
                        for (int i = 0; i < _dflimDiagnosticsData.Length; ++i)
                        {
                            _dflimDiagnosticsData[i] = new ushort[_dflimDiagnosticsBufferLength];
                        }
                    }
                    else
                    {
                        for (int i = 0; i < _dflimDiagnosticsData.Length; ++i)
                        {
                            if (_dflimDiagnosticsData[i] == null || _dflimDiagnosticsData[i].Length != _dflimDiagnosticsBufferLength)
                            {
                                _dflimDiagnosticsData[i] = new ushort[_dflimDiagnosticsBufferLength];
                            }
                        }
                    }
                    if (colorChannels > 0)
                    {
                        lock (_dflimDiagnosticsDataLock)
                        {
                            for (int i = 0; i < colorChannels; ++i)
                            {
                                int middleLineIndex = (height / 2) * width;
                                //copy out the diagnostic section
                                int copyLength = (_dflimDiagnosticsBufferLength <= ((dataLength / colorChannels) - middleLineIndex)) ? _dflimDiagnosticsBufferLength : (dataLength / colorChannels) - middleLineIndex;

                                MemoryCopyManager.CopyIntPtrMemory(returnArray, i * dataLength + middleLineIndex, _dflimDiagnosticsData[i], 0, copyLength);
                                //Array.Copy(_dataBuffer, i * _dataLength + middleLineIndex, _dflimDiagnosticsData[i], 0, copyLength);
                            }

                            _newDFLIMDiagnosticsData = true;
                        }
                    }
                }
                else
                {
                    //copy out the pixel data
                    MemoryCopyManager.CopyIntPtrMemory(returnArray, 0, _pixelData, 0, dataLength * colorChannels);
                    //Array.Copy(_dataBuffer, 0, _pixelData, 0, _dataLength * _colorChannels);
                }

                bool isSequential = (bool)MVMManager.Instance["SequentialControlViewModel", "IsSequentialCapturing"];

                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, "ImageUpdate pixeldata updated");
                ImageDataUpdated = true;

                ImageInfo = imageInfo;

                _frameData.pixelData = _pixelData;
                _frameData.bitsPerPixel = GetBitsPerPixel();
                _frameData.dFLIMArrivalTimeSumData = _dflimArrivalTimeSumData;
                _frameData.dFLIMSinglePhotonData = _dflimSinglePhotonData;
                _frameData.dFLIMBinDurations = _dFLIMBinDurations;
                _frameData.frameInfo = imageInfo;
                _frameData.averageMode = (int)MVMManager.Instance["ScanControlViewModel", "LSMSignalAverage", (object)0];
                _frameData.averageFrameCount = (int)MVMManager.Instance["ScanControlViewModel", "LSMSignalAverageFrames", (object)0];
                _frameData.contiguousChannels = false;
                _frameData.pixelSizeUM = (PixelSizeUM)MVMManager.Instance["AreaControlViewModel", "PixelSizeUM", (double)1.0];
                _frameData.channelSelection = GetDisplayChannels();
            }

            MVMManager.Instance["ImageViewCaptureSetupVM", "FrameData"] = _frameData;

            EnableCopyToExternalBuffer();
        }

        [DllImport(".\\ThorDiskIO.dll", EntryPoint = "ReadImageInfo")]
        private static extern int ReadImageInfo([MarshalAs(UnmanagedType.LPWStr)]string path, ref int width, ref int height, ref int colorChannels, ref int bitsPerPixel);

        #endregion Methods
    }
}