namespace ImageReviewDll
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Text;
    using System.Windows;
    using System.Windows.Data;
    using System.Xml;

    using ImageReviewDll.Model;
    using ImageReviewDll.ViewModel;

    using MesoScan.Params;

    using ThorSharedTypes;

    public struct ImgInfo
    {
        #region Fields

        public bool[] channelEnable;
        public byte channelEnabled;
        public byte fileChs;
        public int flybackFrames;
        public int frames;
        public int frameSize;
        public CaptureFile imageType;
        public bool isZFastEnabled;
        public int pixelX;
        public int pixelY;
        public List<ScanRegionStruct> scanAreaIDList;
        public int spectralSteps;
        public int timePoints;
        public int zSteps;
        public int zStreamFrames;

        #endregion Fields
    }

    static class ExperimentData
    {
        #region Fields

        private const int BITS_PER_PIXEL_LSM = 14;

        private static int _binX;
        private static int _binY;
        private static int _bitsPerPixel;
        private static int _cameraType = 1;
        private static CaptureModes _captureMode;
        private static int _channels = 0;
        private static int _fieldSize;
        private static int _fieldSizeX;
        private static int _fieldSizeY;
        private static ImgInfo _imageInfo;
        private static bool _ismROICapture = false;
        private static bool _isRemoteFocus;
        private static int _LSMChannel;
        private static int _lsmType = 0;
        private static double _magnification;
        private static double _mmPerPixel;
        private static FULLFOVMetadata _mROIFullFOVMetadata = new FULLFOVMetadata();
        private static double _mROIPixelSizeXUM = 1;
        private static double _mROIPixelSizeYUM = 1;
        private static ObservableCollection<ScanArea> _mROIs;
        private static double _mROIStripLength = 1;
        private static int _numberOfPlanes = 1;
        private static PixelSizeUM _pixelSizeUM = new PixelSizeUM(1.0, 1.0);
        private static double _pixelAspectRatioYScale = 1;
        private static ObservableCollection<int> _planeSequence = new ObservableCollection<int>();
        private static int _spEnd;
        private static int _spmax;
        private static int _spPace;
        private static int _spStart;
        private static int _threePhotonEnable = 0;
        private static int _tmax;
        private static MesoScanTypes _viewMode = MesoScanTypes.Meso;
        private static string[] _waveLengthNames;
        private static int _zmax;
        private static double _zStepSizeUM;
        private static int _zStreamMax;
        private static int _zStreamMode;
        private static int _onlyEnabledChannels;
        private static int _averageMode;
        private static int _imgPerAvg;

        #endregion Fields

        #region Properties

        public static int BinX
        {
            get
            {
                return _binX;
            }
        }

        public static int BinY
        {
            get
            {
                return _binY;
            }
        }

        public static int BitsPerPixel
        {
            get { return _bitsPerPixel; }
        }

        public static CaptureModes CaptureMode
        {
            get
            {
                return _captureMode;
            }
        }

        public static ImgInfo ImageInfo
        {
            get { return _imageInfo; }
        }

        public static bool IsmROICapture
        {
            get => _ismROICapture;
        }

        public static bool IsRemoteFocus
        {
            get { return _isRemoteFocus; }
        }

        public static int LSMChannel
        {
            get { return _LSMChannel; }
        }

        public static double Magnification
        {
            get { return _magnification; }
        }

        public static double MMPerPixel
        {
            get { return _mmPerPixel; }
        }

        public static FULLFOVMetadata mROIFullFOVMetadata
        {
            get => _mROIFullFOVMetadata;
        }

        public static double mROIPixelSizeXUM
        {
            get => _mROIPixelSizeXUM;
        }

        public static double mROIPixelSizeYUM
        {
            get => _mROIPixelSizeYUM;
        }

        public static ObservableCollection<ScanArea> mROIs
        {
            get => _mROIs;
        }

        public static double mROIStripLength
        {
            get => _mROIStripLength;
        }

        public static int NumberOfChannels
        {
            get { return _channels; }
        }

        public static int NumberOfPlanes
        {
            get => _numberOfPlanes;
        }

        public static PixelSizeUM PixelSizeUM
        {
            get { return _pixelSizeUM; }
        }

        public static double PixelAspectRatioYScale
        {
            get { return _pixelAspectRatioYScale; }
        }

        public static ObservableCollection<int> PlaneSequence
        {
            get { return _planeSequence; }
        }

        public static int SpEnd
        {
            get { return _spEnd; }
        }

        public static int SpMax
        {
            get { return _spmax; }
        }

        public static int SpPace
        {
            get { return _spPace; }
        }

        public static int SpStart
        {
            get { return _spStart; }
        }

        public static int TMax
        {
            get { return _tmax; }
        }

        public static MesoScanTypes ViewMode
        {
            get { return _viewMode; }
        }

        public static string[] WaveLengthNames
        {
            get { return _waveLengthNames; }
        }

        public static int ZMax
        {
            get { return _zmax; }
        }

        public static double ZStepSizeUM
        {
            get { return _zStepSizeUM; }
        }

        public static int ZStreamMax
        {
            get { return _zStreamMax; }
        }

        public static int ImgPerAvg
        {
            get { return _imgPerAvg; }
        }

        public static bool AverageMode
        {
            get { return _averageMode == 1; }
        }
        /// <summary>
        /// ZStreamMode: 0 - Z Streaming is turned off
        ///              1 - Z Streaming is turned on
        /// </summary>
        public static int ZStreamMode
        {
            get { return _zStreamMode; }
        }

        public static int OnlyEnabledChannels
        {
            get { return _onlyEnabledChannels; }
        }
        #endregion Properties

        #region Methods

        public static void Populate(XmlDocument doc, XmlDocument hwSettingsDoc)
        {
            //loading the capture mode: (//0: Z&T Series, 1: Streaming, 2: TDI, 3: Bleaching, 4: Hyperspectral)
            _captureMode = (CaptureModes)XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/CaptureMode", "mode");

            //Load the max values for the T, Z, and Sp (hyperspectral) sliders, depending on the capture mode
            switch (_captureMode)
            {
                case CaptureModes.T_AND_Z:
                    {
                        _tmax = XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/Timelapse", "timepoints");

                        if (XmlManager.ReadAttribute<Boolean>(doc, "/ThorImageExperiment/ZStage", "enable"))
                        {
                            _zmax = XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/ZStage", "steps");
                        }
                        else
                        {
                            _zmax = 1;
                        }

                        _spmax = 1;
                    }
                    break;
                case CaptureModes.STREAMING:
                    {
                        _tmax = XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/Timelapse", "timepoints");

                        if (XmlManager.ReadAttribute<Boolean>(doc, "/ThorImageExperiment/Streaming", "zFastEnable"))
                        {
                            _zmax = XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/ZStage", "steps");
                        }
                        else
                        {
                            _zmax = 1;
                        }

                        _spmax = 1;
                    }
                    break;
                case CaptureModes.BLEACHING:
                    {
                        _tmax = XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/Timelapse", "timepoints");
                        _zmax = 1;
                        _spmax = 1;
                    }
                    break;
                case CaptureModes.HYPERSPECTRAL:
                    {
                        _tmax = 1;
                        _zmax = 1;
                        _spmax = XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/SpectralFilter", "steps");
                    }
                    break;
            }

            _spStart = XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/SpectralFilter", "wavelengthStart");
            _spEnd = XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/SpectralFilter", "wavelengthStop");
            _spPace = XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/SpectralFilter", "wavelengthStepSize");

            _zStepSizeUM = XmlManager.ReadAttribute<Double>(doc, "/ThorImageExperiment/ZStage", "stepSizeUM");
            _zStreamMode = XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/ZStage", "zStreamMode");
            _onlyEnabledChannels = XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/RawData", "onlyEnabledChannels");
            _isRemoteFocus = 1 == XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/RemoteFocus", "IsRemoteFocus");
            if (_isRemoteFocus)
            {
                _planeSequence.Clear();
                string customSequence = XmlManager.ReadAttribute<string>(doc, "/ThorImageExperiment/RemoteFocus", "customSequence");
                if (string.Empty == customSequence)
                {
                    int steps = XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/RemoteFocus", "steps");
                    int startPlane = XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/RemoteFocus", "startPlane");
                    int stepSize = XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/RemoteFocus", "stepSize");
                    for (int i = 0; i < steps; i++)
                    {
                        _planeSequence.Add(startPlane + stepSize * i);
                    }
                }
                else
                {
                    string[] values = customSequence.Split(':');
                    foreach (string val in values)
                    {
                        int tmp;
                        if (Int32.TryParse(val, out tmp))
                        {
                            _planeSequence.Add(tmp);
                        }
                    }
                }
            }
            _zStreamMax = 1;
            if (CaptureModes.T_AND_Z == _captureMode && _zStreamMode > 0)
            {
                _zStreamMax = XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/ZStage", "zStreamFrames", _zStreamMax); //Default to original value of _zStreamMax
            }

            //If the value doesnt exist then set to ICamera.CameraType.LSM
            _cameraType = XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/Modality", "primaryDetectorType", (int)ICamera.CameraType.LSM);
            _lsmType = XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/Modality", "detectorLSMType", (int)ICamera.LSMType.GALVO_RESONANCE);

            switch ((ICamera.CameraType)_cameraType)
            {
                case ICamera.CameraType.CCD:
                    _LSMChannel = 1;
                    int left = XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/Camera", "left");
                    int right = XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/Camera", "right");
                    int top = XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/Camera", "top");
                    int bottom = XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/Camera", "bottom");
                    _fieldSizeX = Math.Abs(right - left);
                    _fieldSizeY = Math.Abs(bottom - top);
                    _bitsPerPixel = XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/Camera", "bitsPerPixel");
                    _pixelSizeUM.PixelHeightUM = _pixelSizeUM.PixelWidthUM = XmlManager.ReadAttribute<Double>(doc, "/ThorImageExperiment/Camera", "pixelSizeUM");
                    _binX = XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/Camera", "binningX");
                    _binY = XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/Camera", "binningY");
                    _averageMode = XmlManager.ReadAttribute<Int32>(doc, "ThorImageExperiment/Camera", "averageMode");
                    _imgPerAvg = XmlManager.ReadAttribute<Int32>(doc, "ThorImageExperiment/Camera", "averageNum");
                    _pixelAspectRatioYScale = 1;
                    break;
                case ICamera.CameraType.LSM:
                    _LSMChannel = XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/LSM", "channel");
                    _fieldSizeX = XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/LSM", "pixelX");
                    _fieldSizeY = XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/LSM", "pixelY");
                    _fieldSize = XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/LSM", "fieldSize");
                    _bitsPerPixel = BITS_PER_PIXEL_LSM;
                    _pixelSizeUM.PixelWidthUM = XmlManager.ReadAttribute<Double>(doc, "/ThorImageExperiment/LSM", "pixelWidthUM");
                    _pixelSizeUM.PixelHeightUM = XmlManager.ReadAttribute<Double>(doc, "/ThorImageExperiment/LSM", "pixelHeightUM");
                    _pixelAspectRatioYScale = XmlManager.ReadAttribute<Double>(doc, "/ThorImageExperiment/LSM", "pixelAspectRatioYScale");
                    _threePhotonEnable = XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/LSM", "ThreePhotonEnable");
                    _numberOfPlanes = _threePhotonEnable == 1 ? XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/LSM", "NumberOfPlanes") : 1;
                    _imgPerAvg = XmlManager.ReadAttribute<Int32>(doc, "ThorImageExperiment/LSM", "averageNum");
                    _averageMode = XmlManager.ReadAttribute<Int32>(doc, "ThorImageExperiment/LSM", "averageMode");
                    break;
            }

            _waveLengthNames = ReadStringAttributesMultipleNodes(doc, "/ThorImageExperiment/Wavelengths/Wavelength", "name").ToArray();
            _channels = _waveLengthNames.Count();

            _magnification = XmlManager.ReadAttribute<Double>(doc, "/ThorImageExperiment/Magnification", "mag", 1.0, 0);
            if ((_fieldSizeX != 0) && (_magnification != 0))
            {
                if (1 == _cameraType)    // LSM only
                {
                    _mmPerPixel = XmlManager.ReadAttribute<Double>(doc, "/ThorImageExperiment/LSM", "pixelSizeUM", 1.0, 0) / 1000.0;
                }
                else if (0 == _cameraType)    //Camera only
                {
                    _mmPerPixel = _pixelSizeUM.PixelWidthUM / (_magnification * 1000);
                }
            }
            
            QueryImageInfo(doc);
            
            
        }

        /// <summary>
        /// Reads an attribute from the xml document and path and parses it into a string.
        /// </summary>
        /// <param name="doc"> XmlDocument to read from </param>
        /// <param name="xPathToNode"> Path to node </param>
        /// <param name="attribute"> Attribute tag in node </param>
        /// <param name="valueOnError"> Optional parameter specifying the value to use if there
        /// is an error in retrieving and parsing the value from the xml doc</param>
        /// <returns> The value parsed from the xml document, or the default value if a value could not be retrieved </returns>
        public static List<string> ReadStringAttributesMultipleNodes(XmlDocument doc, string xPathToNode, string attribute, string valueOnError = "")
        {
            string value;
            List<string> values = new List<string>();

            int index = 0;
            while (XmlManager.ReadAttribute<String>(out value, doc, xPathToNode, attribute, valueOnError, index++))
            {
                values.Add(String.Copy(value));
            }

            return values;
        }

        private static void QueryImageInfo(XmlDocument doc)
        {
            try
            {
                //loading the scan area:
                XmlNodeList ndList = doc.SelectNodes("/ThorImageExperiment/TemplateScans/ScanInfo[@ScanID=2]/ScanAreas/ScanArea[@IsEnable='true']");
                _viewMode = (0 < ndList.Count) ? MesoScanTypes.Micro : MesoScanTypes.Meso;
                string vmodeStr = MesoScanTypes.Micro == _viewMode ? "/ThorImageExperiment/TemplateScans/ScanInfo[@ScanID=2]/ScanAreas/ScanArea" :
                    "/ThorImageExperiment/TemplateScans/ScanInfo[@ScanID=1]/ScanAreas/ScanArea[@IsEnable='true']";

                _imageInfo.channelEnabled = XmlManager.ReadAttribute<Byte>(doc, "/ThorImageExperiment/Wavelengths/ChannelEnable", "Set");
                _imageInfo.fileChs = 0;
                _imageInfo.channelEnable = new bool[ImageReview.MAX_CHANNELS];
                for (int i = 0; i < ImageReview.MAX_CHANNELS; i++)
                {
                    if ((_imageInfo.channelEnabled & (0x1 << i)) != 0)
                    {
                        _imageInfo.channelEnable[i] = true;
                        _imageInfo.fileChs++;
                    }
                    else
                    {
                        _imageInfo.channelEnable[i] = false;
                    }
                }

                //scan area image sizes:
                _imageInfo.scanAreaIDList = new List<ScanRegionStruct>();
                ScanRegionStruct sRegion = new ScanRegionStruct() { BufferSize = 0, RegionID = 0, SizeS = 0, SizeX = 0, SizeY = 0, SizeT = 0, SizeZ = 0, ScanID = (byte)_viewMode };

                switch ((ICamera.CameraType)_cameraType)
                {
                    case ICamera.CameraType.CCD:
                        _imageInfo.pixelX = XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/Camera", "width");
                        _imageInfo.pixelY = XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/Camera", "height");
                        break;
                    case ICamera.CameraType.LSM:
                        ndList = doc.SelectNodes(vmodeStr);
                        if ((int)ICamera.LSMType.RESONANCE_GALVO_GALVO == _lsmType && 0 < ndList.Count)
                        {
                            for (int i = 0; i < ndList.Count; i++)
                            {
                                sRegion.RegionID = XmlManager.ReadAttribute<UInt16>(doc, vmodeStr, "ScanAreaID", 0, i);
                                sRegion.SizeX = XmlManager.ReadAttribute<UInt32>(doc, vmodeStr, "SizeX", 0, i);
                                sRegion.SizeY = XmlManager.ReadAttribute<UInt32>(doc, vmodeStr, "SizeY", 0, i);
                                sRegion.SizeT = XmlManager.ReadAttribute<UInt32>(doc, vmodeStr, "SizeT", 0, i);
                                sRegion.SizeZ = XmlManager.ReadAttribute<UInt32>(doc, vmodeStr, "SizeZ", 0, i);
                                sRegion.SizeS = XmlManager.ReadAttribute<UInt32>(doc, vmodeStr, "SizeS", 0, i);
                                _imageInfo.scanAreaIDList.Add(sRegion);
                            }
                        }
                        _imageInfo.scanAreaIDList.Sort(delegate (ScanRegionStruct s1, ScanRegionStruct s2) { return s1.RegionID.CompareTo(s2.RegionID); });
                        _imageInfo.pixelX = XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/LSM", "pixelX");
                        _imageInfo.pixelY = XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/LSM", "pixelY");
                        break;
                    case ICamera.CameraType.LAST_CAMERA_TYPE:
                    default:
                        break;
                }

                _ismROICapture = mROIXMLMapper.MapXml2mROIParams(doc, (ICamera.LSMType)_lsmType, out _mROIs, out _mROIPixelSizeXUM, out _mROIPixelSizeYUM, out _mROIStripLength, out _mROIFullFOVMetadata);

                _imageInfo.frameSize = _imageInfo.pixelX * _imageInfo.pixelY * _numberOfPlanes * 2;

                _imageInfo.timePoints = XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/Timelapse", "timepoints");
                switch (_captureMode)
                {
                    case CaptureModes.T_AND_Z:
                        {
                            _imageInfo.imageType = CaptureFile.FILE_TIFF;
                            _imageInfo.spectralSteps = 1;

                            if (XmlManager.ReadAttribute<Boolean>(doc, "/ThorImageExperiment/ZStage", "enable"))
                            {
                                _imageInfo.zStreamFrames = XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/ZStage", "zStreamFrames");
                                _imageInfo.zSteps = XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/ZStage", "steps");

                            }
                            else
                            {
                                _imageInfo.zStreamFrames = 1;
                                _imageInfo.zSteps = 1;
                            }

                        }
                        break;
                    case CaptureModes.STREAMING:
                        {
                            _imageInfo.frames = XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/Streaming", "frames");
                            _imageInfo.imageType = (CaptureFile)XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/Streaming", "rawData");
                            if (CaptureFile.FILE_BIG_TIFF != _imageInfo.imageType) _imageInfo.scanAreaIDList.Clear();
                            _imageInfo.isZFastEnabled = XmlManager.ReadAttribute<Boolean>(doc, "/ThorImageExperiment/Streaming", "zFastEnable");
                            _imageInfo.flybackFrames = (_imageInfo.isZFastEnabled ? XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/Streaming", "flybackFrames") : 0);
                            _imageInfo.spectralSteps = 1;
                            _imageInfo.zStreamFrames = 1;
                            if (_imageInfo.isZFastEnabled)
                            {
                                _imageInfo.zSteps = XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/ZStage", "steps");

                            }
                            else
                            {
                                _imageInfo.zSteps = 1;
                            }
                        }
                        break;
                    case CaptureModes.BLEACHING:
                        {
                            _imageInfo.imageType = (CaptureFile)XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/Photobleaching", "rawOption");
                            _imageInfo.frames = XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/Photobleaching", "preBleachFrames") +
                                                XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/Photobleaching", "postBleachFrames1") +
                                                XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/Photobleaching", "postBleachFrames2");
                            _imageInfo.isZFastEnabled = false;
                            _imageInfo.zStreamFrames = 1;
                            _imageInfo.zSteps = 1;
                            _imageInfo.spectralSteps = 1;
                        }
                        break;
                    case CaptureModes.HYPERSPECTRAL:
                        {
                            _imageInfo.spectralSteps = XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/SpectralFilter", "steps");

                            _imageInfo.imageType = (CaptureFile)XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/Streaming", "rawData");
                            _imageInfo.isZFastEnabled = false;
                            _imageInfo.zStreamFrames = 1;
                            _imageInfo.zSteps = 1;
                            _imageInfo.flybackFrames = (_imageInfo.isZFastEnabled ? XmlManager.ReadAttribute<Int32>(doc, "/ThorImageExperiment/Streaming", "flybackFrames") : 0);
                        }
                        break;
                }

            }
            catch (Exception ex)
            {
                ex.ToString();
                MessageBox.Show("Corrupted Experiment.xml file");
            }
        }

        #endregion Methods
    }
}