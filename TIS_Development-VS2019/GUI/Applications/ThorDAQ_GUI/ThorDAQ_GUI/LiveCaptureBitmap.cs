namespace thordaqGUI
{
    using System;
    using System.Runtime.InteropServices; // DllImport
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Collections.Concurrent;
    using System.ComponentModel;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Text;
    using System.Text.RegularExpressions;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Navigation;
    using System.Windows.Shapes;
    using NWLogic.DeviceLib;

    using ThorLogging;
    using ThorSharedTypes;
    using System.Diagnostics;
    using System.Reflection;



    /// <summary>
    /// Interaction logic for LiveCaptureBitmap.xaml
    /// </summary>
    public partial class MainWindow : Window, INotifyPropertyChanged
    {
        public const int MAX_CHANNELS = 4;
        // define function pointers to bind based on ScanHead hardware DLL API
        public delegate int TDFindCameras(out long cameraCnt);
        public delegate int TDSelectCamera(long cameraCnt);
        public delegate int TDSetParam(long paramID, double value);
        public delegate int TDGetParam(long paramID, out double value);  // GetParam(const long paramID, double &param)
        public delegate int TDPreflightAcquisition(byte[] pBuffer);
        public delegate int TDPostflightAcquisition(byte[] pBuffer);
        public delegate int TDSetupAcquisition(byte[] pBuffer);
        public delegate int TDStartAcquisition(byte[] pBuffer);
        public delegate long TDStatusAcquisitionEx(ref long StatusType, ref long indexOfLastCompletedFrame);  
        public delegate int TDCopyAcquisition(IntPtr pBuffer, out FrameInfoStruct MoreFrameInfo);
        public delegate int TDTeardownCamera();

        // provide defaults for ThorDAQ functions - user app switches
        TDFindCameras       FindCameras = new TDFindCameras(ThorDAQggFindCameras);
        TDSelectCamera      SelectCamera = new TDSelectCamera(ThorDAQggSelectCamera);
        TDSetParam          SetParam = new TDSetParam(ThorDAQggSetParam);
        TDGetParam          GetParam = new TDGetParam(ThorDAQggGetParam);

        TDPreflightAcquisition PreflightAcquisition = new TDPreflightAcquisition(ThorDAQggPreflightAcquisition);
        TDPostflightAcquisition PostflightAcquisition = new TDPostflightAcquisition(ThorDAQggPostflightAcquisition);
        TDSetupAcquisition  SetupAcquisition = new TDSetupAcquisition(ThorDAQggSetupAcquisition);
        TDStartAcquisition  StartAcquisition = new TDStartAcquisition(ThorDAQggStartAcquisition);
        TDStatusAcquisitionEx  StatusAcquisitionEx = new TDStatusAcquisitionEx(ThorDAQggStatusAcquisitionEx);
        TDCopyAcquisition   CopyAcquisition = new TDCopyAcquisition(ThorDAQggCopyAcquisition);
        TDTeardownCamera TeardownCamera = new TDTeardownCamera(ThorDAQggTeardownCamera);


        // DLLs from lowest level thordaq.dll are of course independent of GalvoGalvo or ResGalvo
        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPISetDACParkValue")]
        public static extern int SetDACParkValue(uint boardNum, UInt32 channel, double ParkVolts);

        // 
        [DllImport(".\\Modules_Native\\thordaqGalvoGalvo.dll", EntryPoint = "FindCameras")]
        public static extern int GGFindCameras(out long cameraCnt);


        // GalvoGalvo and ResGalvo DLL calls
        [DllImport(".\\Modules_Native\\thordaqResonantGalvo.dll", EntryPoint = "FindCameras")]
        public static extern int ThorDAQrgFindCameras(out long cameraCnt );
        [DllImport(".\\Modules_Native\\thordaqGalvoGalvo.dll", EntryPoint = "FindCameras")]
        public static extern int ThorDAQggFindCameras(out long cameraCnt);

        [DllImport(".\\Modules_Native\\thordaqResonantGalvo.dll", EntryPoint = "SelectCamera")]
        public static extern int ThorDAQrgSelectCamera(long cameraCnt);
        [DllImport(".\\Modules_Native\\thordaqGalvoGalvo.dll", EntryPoint = "SelectCamera")]
        public static extern int ThorDAQggSelectCamera(long cameraCnt);


        [DllImport(".\\Modules_Native\\thordaqResonantGalvo.dll", EntryPoint = "SetParam")]
        public static extern int ThorDAQrgSetParam(long paramID, double value);
        [DllImport(".\\Modules_Native\\thordaqGalvoGalvo.dll", EntryPoint = "SetParam")]
        public static extern int ThorDAQggSetParam(long paramID, double value);

        [DllImport(".\\Modules_Native\\thordaqResonantGalvo.dll", EntryPoint = "GetParam")]
        public static extern int ThorDAQrgGetParam(long paramID, out double value);
        [DllImport(".\\Modules_Native\\thordaqGalvoGalvo.dll", EntryPoint = "GetParam")]
        public static extern int ThorDAQggGetParam(long paramID, out double value);


        [DllImport(".\\Modules_Native\\thordaqResonantGalvo.dll", EntryPoint = "PreflightAcquisition")]
        public static extern int ThorDAQrgPreflightAcquisition(byte[] pBuffer);  // arg not used
        [DllImport(".\\Modules_Native\\thordaqGalvoGalvo.dll", EntryPoint = "PreflightAcquisition")]
        public static extern int ThorDAQggPreflightAcquisition(byte[] pBuffer);  // arg not used

        [DllImport(".\\Modules_Native\\thordaqResonantGalvo.dll", EntryPoint = "PostflightAcquisition")]
        public static extern int ThorDAQrgPostflightAcquisition(byte[] pBuffer);  // arg not used
        [DllImport(".\\Modules_Native\\thordaqGalvoGalvo.dll", EntryPoint = "PostflightAcquisition")]
        public static extern int ThorDAQggPostflightAcquisition(byte[] pBuffer);  // arg not used

        [DllImport(".\\Modules_Native\\thordaqResonantGalvo.dll", EntryPoint = "SetupAcquisition")]
        public static extern int ThorDAQrgSetupAcquisition(byte[] pBuffer);  // arg not used
        [DllImport(".\\Modules_Native\\thordaqGalvoGalvo.dll", EntryPoint = "SetupAcquisition")]
        public static extern int ThorDAQggSetupAcquisition(byte[] pBuffer);  // arg not used

        
        [DllImport(".\\Modules_Native\\thordaqResonantGalvo.dll", EntryPoint = "StartAcquisition")]
        public static extern int ThorDAQrgStartAcquisition(byte[] pBuffer);  // arg not used - creates Capture thread
        [DllImport(".\\Modules_Native\\thordaqGalvoGalvo.dll", EntryPoint = "StartAcquisition")]
        public static extern int ThorDAQggStartAcquisition(byte[] pBuffer);  // arg not used - creates Capture thread

  
        [DllImport(".\\Modules_Native\\thordaqResonantGalvo.dll", EntryPoint = "StatusAcquisitionEx")]
        public static extern long ThorDAQrgStatusAcquisitionEx(ref long StatusType, ref long indexOfLastCompletedFrame );
        [DllImport(".\\Modules_Native\\thordaqGalvoGalvo.dll", EntryPoint = "StatusAcquisitionEx")]
        public static extern long ThorDAQggStatusAcquisitionEx(ref long StatusType, ref long indexOfLastCompletedFrame);  


        //long CThordaqResonantGalvo::CopyAcquisition(char * pDataBuffer, long &moreAvailable)
        [DllImport(".\\Modules_Native\\thordaqResonantGalvo.dll", EntryPoint = "CopyAcquisition")]
        public static extern int ThorDAQrgCopyAcquisition(IntPtr pBuffer, out FrameInfoStruct moreAvailable);  // IntPtr is array of Uint16 in DLL
        [DllImport(".\\Modules_Native\\thordaqGalvoGalvo.dll", EntryPoint = "CopyAcquisition")]
        public static extern int ThorDAQggCopyAcquisition(IntPtr pBuffer, out FrameInfoStruct moreAvailable);  // IntPtr is array of Uint16 in DLL

        [DllImport(".\\Modules_Native\\thordaqResonantGalvo.dll", EntryPoint = "TeardownCamera")]
        public static extern int ThorDAQrgTeardownCamera();
        [DllImport(".\\Modules_Native\\thordaqGalvoGalvo.dll", EntryPoint = "TeardownCamera")]
        public static extern int ThorDAQggTeardownCamera();  

        
        #region Fields
        // allocate the unmanaged memory for CPP DLL DMA transfer
        public IntPtr ADC_unmanaged_DMAbuffer;
        private byte[][] _pixelDataLUT; // from CaptureSetupImage.cs
        private static byte[][] _rawImg = new byte[MAX_CHANNELS + 1][];
        private static byte[][] _pal;
  //      private static int[][] _pixelDataHistogram;


        public static Color[][] ChannelLuts
        {
            get;
            set;
        }

        WriteableBitmap _bitmap = null;
        private BackgroundWorker _bw;
        private double _imageHeight;
        private double _imageWidth;
        Point _newOrigin;
        Point _newStart;
        private ScaleTransform _scaleTransform;
        private Point _scrollStartPoint;
        private double _currentScale;
        Matrix m = new Matrix();
        private TransformGroup _transformGroup;
        private TranslateTransform _translateTransform;
        //private string AssemExePath;

        #endregion Fields

        #region Properties



        /*
        private BitmapPalette BuildPalette()
        {
            List<Color> colors = new List<Color>();

            string chanName = "ChanA";
            if (LSMChannelEnable0)
                chanName = "ChanA";
            if (LSMChannelEnable1)
                chanName = "ChanB";
            if (LSMChannelEnable2)
                chanName = "ChanC";
            if (LSMChannelEnable3)
                chanName = "ChanD";

            XmlNodeList ndList = HardwareDoc.SelectNodes("/HardwareSettings/ColorChannels/*");

            string str = string.Empty;

            for (int i = 0; i < ndList.Count; i++)
            {
                if (XmlManager.GetAttribute(ndList[i], HardwareDoc, "name", ref str))
                {
                    if (str.Contains(chanName))
                    {
                        str = Application.Current.Resources["ApplicationSettingsFolder"].ToString() + "\\luts\\" + ndList[i].Name + ".txt";

                        if (File.Exists(str))
                        {
                            StreamReader fs = new StreamReader(str);
                            string line;
                            int counter = 0;
                            try
                            {
                                while ((line = fs.ReadLine()) != null)
                                {
                                    string[] split = line.Split(',');

                                    if (split[0] != null)
                                    {
                                        if (split[1] != null)
                                        {
                                            if (split[2] != null)
                                            {
                                                if (256 == colors.Count)
                                                {
                                                    break;
                                                }
                                                colors.Add(Color.FromRgb(Convert.ToByte(split[0]), Convert.ToByte(split[1]), Convert.ToByte(split[2])));
                                            }
                                        }
                                    }
                                    counter++;
                                }
                            }
                            catch (Exception ex)
                            {
                                string msg = ex.Message;
                            }

                            fs.Close();

                            _currentLutFile = str;
                        }
                    }
                }
            }
            return new BitmapPalette(colors);
        }
        */

        public WriteableBitmap Bitmap
        {
            get
            {
                PixelFormat _pf;
                try
                {
                    BitmapPalette _thorPalette;

                    if (_bNewBitMapRequired == true)
//                        if (_bitmap == null)
                    {

                        _pixelData = new short[BitmapWidth * BitmapHeight * _colorChannels]; // per channel, 4 channels now, 6 channels possible
                        _pixelDataByte = new byte[_pixelData.Length * 2];
                        if (!ADC_unmanaged_DMAbuffer.Equals(null))
                        {
                            Marshal.FreeHGlobal(ADC_unmanaged_DMAbuffer);
                        }
                        // allocate the DMA buffer managed memory - low level design calls for 1 image buffer size
                        // for single channel, or 4 image buffer size if 2 or more channels
                        // width * height * 2bytes frame, with 4 frames (channels) per acquisition
                        // size in bytes, 4 DMA channels of 16-bit values  
                        // the 14-bit RAW A/D is look-up table expanded to 16-bits
                        // buffer design is SINGLE channel, re-used up to four times in loop, per
                        // acquisition event
                        ADC_unmanaged_DMAbuffer = Marshal.AllocHGlobal((int)(_pixelData.Length * 2 * _colorChannels));
                        Debug.Listeners.Add(new TextWriterTraceListener(Console.Out));
                        Debug.AutoFlush = true;
                        Debug.Indent();
                        Debug.WriteLine("ADC_unmanaged_DMAbuffer allocated");

                        if (_colorChannels == 1)
                        {
                            // Palette arg is required for Indexed (PixelFormats.Indexed8) Bitmap
                            _thorPalette = BuildPaletteGrayscale(); // gives us blue image on NULL DMA values
                            _pf = PixelFormats.Indexed8;
                        }
                        else
                        {
                            LoadChannelLUTs();  // instead of reading ThorImageLS files, just create defaults
                            BuildNormalizationPalettes(); // only for RGB

                            _pf = PixelFormats.Rgb24;
                            _thorPalette = null; // BuildPalette(), takes RGB table from files
                        }

                        // initialize empty source with randomness
                        int min = 0;
                        int max = 22354;
                        Random randNum = new Random();
                        for (int i = 0; i < _pixelData.Length; i++)
                        {
                            _pixelData[i] = (short)(randNum.Next(min, max) & (ushort)0xFFFF); // limited to 16 bit numbers
                        }

                        _bitmap = new WriteableBitmap(BitmapWidth, BitmapHeight, 96, 96, _pf, _thorPalette);
                        ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, "ThorDAQcl-GUI: WriteableBitmap constructor complete");
                        _bNewBitMapRequired = false;
                    } // end of constructor
                }
                catch ( Exception ex)
                {
                    string CurrentDir = Directory.GetCurrentDirectory();
                    UpdateConsoleStatus("CodePath: " +  "  CurrentDir: " + CurrentDir);
                    Console.WriteLine("EXCEPTION: {0} ", ex.Message);
                }
                finally
                {
                }

                // which of the four channels are we displaying?

                //if (ImageSource.Length == (BitmapWidth * BitmapHeight))
                if((_pixelData != null) &&   _pixelData.Length != 0)
                {
                    byte[] pd;
                    int rawStride;

                    if (_colorChannels == 1) // 8-bit "grayscale"
                    {
                        pd = GetPixelDataByte(ADCbitwiseChanIndex);
                        rawStride = _bitmap.PixelWidth;
                    }
                    else // 24-bit palette
                    {
                        pd = GetPixelDataByteEx(true, 0);
                        rawStride = (_bitmap.PixelWidth * (int)PixelFormats.Rgb24.BitsPerPixel + 7) / 8;
                    }
                    //copy the pixel data into the _bitmap
                    _bitmap.WritePixels(new Int32Rect(0, 0, _bitmap.PixelWidth, _bitmap.PixelHeight), pd, rawStride, 0);
                    imageCanvas.InvalidateVisual();
                }
                return _bitmap;  
            }
            set 
            {
                this._bitmap = value;
            }
        }


        // From ThorImageLS ...
        public byte[] GetPixelDataByteEx(bool doColor, int channelIndex)
        {
            if (!doColor)
                return _rawImg[channelIndex];

//            IntPtr refChannIntPtr;
//            ushort[] refChannShortArray = null;
            int _dataLength = _bitmap.PixelWidth * _bitmap.PixelHeight;

            //no reset of histogram in partial frame
         //   bool resetPixelDataHistogram = (1 == ImageInfo.fullFrame) ? true : false;

            //need to rebuid the color image because a palette option is not available for RGB images
            if ((_colorChannels > 1) && (_pixelData != null) && (_dataLength * _colorChannels == _pixelData.Length))
            {
                for (int k = 0; k < MAX_CHANNELS; k++)
                {
                    if (_rawImg[k] == null || _rawImg[k].Length != 3 * _dataLength)
                        _rawImg[k] = new byte[3 * _dataLength];
                    else
                        Array.Clear(_rawImg[k], 0, _rawImg[k].Length);
                }
                Array.Clear(_pixelDataByte, 0, 3 * _dataLength);

          //      if (true == _paletteChanged)
          //      {
          //          BuildNormalizationPalettes();
          //          _paletteChanged = false;
          //      }

                // load reference channel
//                bool refToRefChann = false;

                // don't implement "reference channel" in low level test tool
/*                if (1 == (int)MVMManager.Instance["AreaControlViewModel", "EnableReferenceChannel", (object)0])
                {
                    string refChannDir = Application.Current.Resources["AppRootFolder"].ToString() + "\\ReferenceChannel.tif";
                    if (File.Exists(refChannDir))   // ref channel file existance
                    {
                        long width = 0;
                        long height = 0;
                        long colorChannels = 0;
                        if (LoadRefChannInfo(refChannDir, ref width, ref height, ref colorChannels))    // load dimention of ref image
                        {
                            if (width * height == _dataLength)
                            {
                                refChannIntPtr = Marshal.AllocHGlobal(Convert.ToInt32(width) * Convert.ToInt32(height) * 2);

                                if (LoadRefChann(refChannDir, ref refChannIntPtr))  // load ref image
                                {
                                    try
                                    {
                                        refChannShortArray = new ushort[Convert.ToInt32(width) * Convert.ToInt32(height)];
                                        MemoryCopyManager.CopyIntPtrMemory(refChannIntPtr, refChannShortArray, 0, Convert.ToInt32(width) * Convert.ToInt32(height));
                                        refToRefChann = true;
                                    }
                                    catch (Exception e)
                                    {
                                        ThorLog.Instance.TraceEvent(TraceEventType.Information, 1, e.Message);
                                    }
                                }
                                Marshal.FreeHGlobal(refChannIntPtr);
                            }
                        }
                    }
                } */

//                int shiftValue = 6; // GetBitsPerPixel() - 8;

                //in the interest of speed we are seperating the reference channel case
                //without a reference channel the logic will run faster since the
                //conditionals per pixel are removed.
/*
                if (refToRefChann)
                {
                    //factor is derived by taking CAMERA_MAX_INTENSITY_VALUE/256
                    //const double FULL_RANGE_NORMALIZATION_FACTOR = 64.0;
                    short[] rawArray = _pixelData;
                    for (int k = 0; k < MAX_CHANNELS; k++)
                    {
                        if (_lsmChannelEnable[k])
                        {
                         //   if (resetPixelDataHistogram)
                         //   {
                         //       Array.Clear(_pixelDataHistogram[k], 0, _pixelDataHistogram[k].Length);
                         //   }
                            int n = k;
                            if (3 == k)
                            {
                                rawArray = refChannShortArray;
                                n = 0;
                            }
                            for (int i = 0, j = 0; j < _dataLength; i += 3, j++)
                            {
                                ushort valRaw;
                                Color col;
                                //when the reference channel option is on. do not copy the data from channel 3
                                valRaw = rawArray[j + n * _dataLength];
                                col = ChannelLuts[k][_pal[k][valRaw]];
                                _rawImg[k][i] = col.R;
                                _rawImg[k][i + 1] = col.G;
                                _rawImg[k][i + 2] = col.B;
                                if (_pixelDataByte[i] < col.R) _pixelDataByte[i] = col.R;
                                if (_pixelDataByte[i + 1] < col.G) _pixelDataByte[i + 1] = col.G;
                                if (_pixelDataByte[i + 2] < col.B) _pixelDataByte[i + 2] = col.B;

                                //only build the histogram if the color mode is selected when full frame is ready.
                                //This will allow histograms for all of the channels to be available simultaneously
                                if (resetPixelDataHistogram)
                                {
                                    byte valRawHist = (byte)(valRaw >> shiftValue);
                                    _pixelDataHistogram[k][valRawHist]++;
                                }
                            }
                        }
                    } // k
                }
                else
 */ 
                {
                    //factor is derived by taking CAMERA_MAX_INTENSITY_VALUE/256
                    //const double FULL_RANGE_NORMALIZATION_FACTOR = 64.0;

                    for (int k = 0; k < MAX_CHANNELS; k++)
                    {
                        if (_lsmChannelEnable[k])
                        {
       //                     if (resetPixelDataHistogram)
       //                     {
       //                         Array.Clear(_pixelDataHistogram[k], 0, _pixelDataHistogram[k].Length);
       //                     }
                            var rangePartitioner = Partitioner.Create(0, _dataLength, (_dataLength >> 2) + 1);
                            Parallel.ForEach
                                (rangePartitioner, new ParallelOptions { MaxDegreeOfParallelism = 4 }, range =>
                                     {
                                         for (int q = range.Item1; q < range.Item2; q++)
                                         {
                                             int n = q * 3;

                                             //[TO DO] find out why bad buffer could occur due to latency.
                                             short valRaw = _pixelData[q + k * _dataLength];
                                             if (0 > valRaw)
                                             {
                                                 _rawImg[k][n] = _rawImg[k][n + 1] = _rawImg[k][n + 2] = 0;
                                                 _pixelDataByte[n] = _pixelDataByte[n + 1] = _pixelDataByte[n + 2] = 0;
                                             }
                                             else
                                             {
                                                 Color col = ChannelLuts[k][_pal[k][valRaw]];
                                                 _rawImg[k][n] = col.R;
                                                 _rawImg[k][n + 1] = col.G;
                                                 _rawImg[k][n + 2] = col.B;
                                                 if (_pixelDataByte[n] < col.R) _pixelDataByte[n] = col.R;
                                                 if (_pixelDataByte[n + 1] < col.G) _pixelDataByte[n + 1] = col.G;
                                                 if (_pixelDataByte[n + 2] < col.B) _pixelDataByte[n + 2] = col.B;
                                             }

                                             //only build the histogram if the color mode is selected when full frame is ready.
                                             //This will allow histograms for all of the channels to be available simultaneously
                                      //       if (resetPixelDataHistogram)
                                      //       {
                                      //           byte valRawHist = (byte)(valRaw >> shiftValue);
                                      //           _pixelDataHistogram[k][valRawHist]++;
                                      //       }
                                         }
                                     }
                                );
                        }
                    }// k
                    _rawImg[MAX_CHANNELS] = _pixelDataByte;
                }
            }

            return _pixelDataByte;
        }

        
 
        public int BitmapHeight
        {
            get;
            set;
        }

        public int BitmapWidth
        {
            get;
            set;
        }

        private int _ADCbitwiseChanIndex = 0x1; // i.e. all 4 channels selected is 0xF
        public int ADCbitwiseChanIndex
        {
            get { return _ADCbitwiseChanIndex; }
            set { _ADCbitwiseChanIndex = value; }
        }

        private int _colorChannels = 1;
        public int ColorChannels 
        {
            get { return _colorChannels; }
            set { _colorChannels = value; }
        }

        public short[] _pixelData // was "ImageSource"
        {
            get;
            set;
        }

        public byte[] _pixelDataByte
        {
            get;
            set;
        }

        public bool IsPixelDataReady
        {
            get;
            set;
        }

        #endregion Properties

        #region Methods

        protected override void OnMouseDown(MouseButtonEventArgs e)
        {
            if (image2D.ImageSource != null)
            {
                _imageWidth = image2D.ImageSource.Width;
                _imageHeight = image2D.ImageSource.Height;
            }

            if (IsMouseOver)
            {
                _scrollStartPoint = this.TranslatePoint(e.GetPosition(this), imageCanvas);
                if (image2D.ImageSource != null)
                {
                    _scrollStartPoint.X = Math.Max(0, Math.Min(_scrollStartPoint.X, image2D.ImageSource.Width - 0.001));
                    _scrollStartPoint.Y = Math.Max(0, Math.Min(_scrollStartPoint.Y, image2D.ImageSource.Height - 0.001));
                }
                _newStart = e.GetPosition(this);
                _newOrigin.X = imageCanvas.RenderTransform.Value.OffsetX;
                _newOrigin.Y = imageCanvas.RenderTransform.Value.OffsetY;
                CaptureMouse();
            }

            base.OnMouseDown(e);
        }

        protected override void OnMouseMove(MouseEventArgs e)
        {
            //if (imageCanvas.IsMouseOver)
            //{
            //    Canvas.SetLeft(PlayButton, (BitmapWidth - PlayButton.ActualWidth) / 2);
            //    Canvas.SetTop(PlayButton, (BitmapHeight - PlayButton.ActualHeight) / 2);
            //    PlayButton.Width = Math.Min(BitmapWidth, BitmapHeight) / 10;
            //    PlayButton.Height = PlayButton.Width;
            //    PlayButton.Visibility = Visibility.Visible;
            //}
            //else
            //{
            //    PlayButton.Visibility = Visibility.Hidden;
            //}
            if (IsMouseCaptured)
            {
                Point pp = new Point();
                pp = e.MouseDevice.GetPosition(this);

                Matrix m = imageCanvas.RenderTransform.Value;
                m.OffsetX = _newOrigin.X + (pp.X - _newStart.X);
                m.OffsetY = _newOrigin.Y + (pp.Y - _newStart.Y);

                imageCanvas.RenderTransform = new MatrixTransform(m);
            }

            base.OnMouseMove(e);
        }

        protected override void OnMouseUp(MouseButtonEventArgs e)
        {
            if (IsMouseCaptured)
            {
                if ((e.ChangedButton == MouseButton.Left) && (null != image2D.ImageSource))
                {
                    Point currentPoint = this.TranslatePoint(e.GetPosition(this), imageCanvas);
                    currentPoint.X = Math.Max(0, Math.Min(currentPoint.X, image2D.ImageSource.Width - 0.001));
                    currentPoint.Y = Math.Max(0, Math.Min(currentPoint.Y, image2D.ImageSource.Height - 0.001));
                }
                ReleaseMouseCapture();
            }
            InputBlock.Focus();
            base.OnMouseUp(e);
        }

        protected override void OnMouseWheel(MouseWheelEventArgs e)
        {
            if (e == null) return;

            Point p = new Point();
            p = e.MouseDevice.GetPosition(imageCanvas);

            m = imageCanvas.RenderTransform.Value;

            double nextScale = _currentScale;
            if (e.Delta > 0)
            {
                nextScale *= 1.1;
            }
            else
            {
                nextScale /= 1.1;
            }
            double scaleFactor = nextScale / _currentScale;
            m.ScaleAtPrepend(scaleFactor, scaleFactor, p.X, p.Y);
            _currentScale = nextScale;

            imageCanvas.RenderTransform = new MatrixTransform(m);

            e.Handled = true;
        }


        public void InitHandles() 
        {
            //create a background worker that will update at 30fps to udpate the bitmap image
            IsPixelDataReady = false;
            _translateTransform = new TranslateTransform();
            _scaleTransform = new ScaleTransform();
            _transformGroup = new TransformGroup();

            _transformGroup.Children.Add(_scaleTransform);
            _transformGroup.Children.Add(_translateTransform);
            imageCanvas.RenderTransform = _transformGroup;

            _currentScale = 1.0;

            _bw = new BackgroundWorker();
            _bw.WorkerReportsProgress = true;
            _bw.WorkerSupportsCancellation = true;
            _bw.DoWork += new DoWorkEventHandler(bw_DoWork);
            _bw.ProgressChanged += new ProgressChangedEventHandler(bw_ProgressChanged);
            _bw.RunWorkerCompleted += new RunWorkerCompletedEventHandler(bw_RunWorkerCompleted);        
        }

        public void ReleaseHandles()
        {
            if (_bw != null)
            {

                _bw.CancelAsync();
                _bw.DoWork -= new DoWorkEventHandler(bw_DoWork);
                _bw.ProgressChanged -= new ProgressChangedEventHandler(bw_ProgressChanged);
                _bw.RunWorkerCompleted -= new RunWorkerCompletedEventHandler(bw_RunWorkerCompleted);
            }
        }

        // copied from .\Commands\CaptureSetup\CaptureSetupModule\Model
        public  byte[] GetPixelDataByte(int chanIndex)
        {
            //   int shiftValue = GetBitsPerPixel() - 8;
            //   shiftValueResult = Math.Pow(2, shiftValue);
            double shiftValueResult = 64;
            
            // if user has de-selected ALL channels, return black
            if (chanIndex == 0) // e.g., hex mask 0x1 - 0xF all disabled
            {
                for (int i = 0; i < _pixelData.Length; i++)
                {
                    _pixelDataByte[i] = 0;
                }
                return _pixelDataByte; // black screen
            }

            if (null == _pixelData)
            {
                return _pixelDataByte; 
            }

            if (null == _pixelDataLUT)
            {
                _pixelDataLUT = new byte[MAX_CHANNELS][];

                for (int c = 0; c < MAX_CHANNELS; c++)
                {
                    _pixelDataLUT[c] = new byte[ushort.MaxValue + 1];
                }
            }

            // if there is no new image and only the GUI pixel count changed, return _pizelDataByte without updating it
       //     if (_pixelData.Length != 0) //_dataLength)
       //     {
       //         return ImageSourceByte;
       //     }
            //Build the 12/14-bit to 8-bit Lut
            for (int c = 0; c < MAX_CHANNELS; c++)
            {
                for (int i = 0; i < _pixelDataLUT[c].Length; i++)
                {
                    double val = (255.0 / (shiftValueResult * 255 /* (_whitePoint[chanIndex]*/ - 0 /*_blackPoint[chanIndex])*/)) * (i -  0 /* _blackPoint[chanIndex]*/ * shiftValueResult);
                    val = (val >= 0) ? val : 0;
                    val = (val <= 255) ? val : 255;
                    _pixelDataLUT[c][i] = (byte)Math.Round(val);
                }
            }

            for (int c = 0; c < 1 /*_colorChannels*/; c++)
            {
                for (int i = 0; i < _pixelData.Length; i++)
                {
                    byte val = _pixelDataLUT[c][( (ushort)_pixelData[i])]; // problem with bit15 2's complement extension
                    _pixelDataByte[i] = val;
                }
            }

            return _pixelDataByte;
        }

        // instead of reading these from .TXT files as in ThorImageLS,
        // just create constants.
        private void LoadChannelLUTs()
        {
            ChannelLuts = new Color[4][];
            for (int i = 0; i < 4; i++)
            {
                ChannelLuts[i] = new Color[256];
            }
            // GREEN
            for (int counter = 0; counter < 256; counter++)
            {
                ChannelLuts[0][counter] = Color.FromRgb(0, (byte)counter, 0);
            }
            // RED
            for (int counter = 0; counter < 256; counter++)
            {
                ChannelLuts[1][counter] = Color.FromRgb((byte)counter, 0,0);
            }
            // BLUE
            for (int counter = 0; counter < 256; counter++)
            {
                ChannelLuts[2][counter] = Color.FromRgb(0,0, (byte)counter);
            }
            // GRAY
            // RED
            for (int counter = 0; counter < 256; counter++)
            {
                ChannelLuts[3][counter] = Color.FromRgb((byte)counter, (byte)counter, (byte)counter);
            }
        }

        private void BuildNormalizationPalettes()
        {
            int shiftValue = 6;// GetBitsPerPixel() - 8;
            double shiftValueResult = Math.Pow(2, shiftValue);
            if (_pal == null)
            {
                _pal = new byte[4][];

                for (int r = 0; r < 4; r++)
                {
                    _pal[r] = new byte[ushort.MaxValue + 1];
                }
            
            }

            for (int j = 0; j < ColorChannels; j++)
            {
                for (int i = 0; i < ushort.MaxValue + 1; i++)
                {
                    double _whitePoint = 255.0; // was done for all 4 channels (_whitePoint[])
                    double _blackPoint = 0.0;
                    double val = (255.0 / (shiftValueResult * (_whitePoint - _blackPoint))) * (i - _blackPoint * shiftValueResult);
                    val = (val >= 0) ? val : 0;
                    val = (val <= 255) ? val : 255;

                    _pal[j][i] = (byte)val;
                }
            }
        }


        // copied from "Commands\CaputureSetup\CaptureSetupModule\ViewModel\CaptureSetupViewModelImage.cs"
        private BitmapPalette BuildPaletteGrayscale()
        {
            List<Color> colors = new List<Color>();
            for (int i = 0; i < 256; i++)
            {
                double a = 1.0;
                double b = 0;

                //   double dvalR = (a * i * (this._captureSetup.WavelengthColor.R) / 255.0) + b; // (typical)
                double dvalR = (a * i * 255.0 / 255.0) + b;
                dvalR = Math.Max(dvalR, 0);
                dvalR = Math.Min(dvalR, 255);

                double dvalG = (a * i * 255.0 / 255.0) + b;
                dvalG = Math.Max(dvalG, 0);
                dvalG = Math.Min(dvalG, 255);

                double dvalB = (a * i * 255.0 / 255.0) + b;
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
                Color color;
                color = Color.FromRgb((byte)dvalR, (byte)dvalG, (byte)dvalB);

                colors.Add(color);
            }

            return new BitmapPalette(colors);
        }

  

        public string TDSetParams()
        {
            int retStatus;
            try
            {
                retStatus = SetParam((int)ICamera.Params.PARAM_LSM_SCANMODE, 1); // one-way
            }
            catch (Exception e)
            {
                return "TDSetParams failed - DLL not found? Exception msg: " + e.Message;
            }
            retStatus = SetParam((int)ICamera.Params.PARAM_LSM_PIXEL_X, BitmapWidth);
            retStatus = SetParam((int)ICamera.Params.PARAM_LSM_PIXEL_Y, BitmapHeight);


            retStatus = SetParam((int)ICamera.Params.PARAM_LSM_NUMBER_OF_PLANES, 1);  // # 981
            retStatus = SetParam((int)ICamera.Params.PARAM_LSM_CHANNEL, 1);


            // set  gains - AFE enabled (1000b) from Slider control
            retStatus = SetParam((int)ICamera.Params.PARAM_LSM_INPUTRANGE1, _sliderADCgain[0]); // 
            retStatus = SetParam((int)ICamera.Params.PARAM_LSM_INPUTRANGE2, _sliderADCgain[1]); // 
            retStatus = SetParam((int)ICamera.Params.PARAM_LSM_INPUTRANGE3, _sliderADCgain[2]); // 
            retStatus = SetParam((int)ICamera.Params.PARAM_LSM_INPUTRANGE4, _sliderADCgain[3]); // 

            // set a reasonable PIXEL DWELL time
//            retStatus = SetParam((int)ICamera.Params.PARAM_LSM_DWELL_TIME, 1.8);
            retStatus = SetParam((int)ICamera.Params.PARAM_LSM_DWELL_TIME, 0.4);  // #225
            // set minimum averaging
            retStatus = SetParam((int)ICamera.Params.PARAM_LSM_AVERAGENUM, 2);
            // set minimum averaging
            retStatus = SetParam((int)ICamera.Params.PARAM_LSM_CLOCKSOURCE, 1);  // 1 internal, 2 external
            // set minimum averaging
            retStatus = SetParam((int)ICamera.Params.PARAM_LSM_EXTERNALCLOCKRATE, 160000000);
            // set minimum averaging
            retStatus = SetParam((int)ICamera.Params.PARAM_LSM_GG_TURNAROUNDTIME_US, 400); // #973
            retStatus = SetParam((int)ICamera.Params.PARAM_TRIGGER_MODE, 2);

            retStatus = SetParam((int)ICamera.Params.PARAM_LSM_GALVO_ENABLE, 1); // Y-axis ??
            retStatus = SetParam((int)ICamera.Params.PARAM_MULTI_FRAME_COUNT, 1024562.0); // why does ThorILS do this?

            return "OK";
        }

        public void StartAcqusition()
        {
            long cameraCnt;
            
            switch (ScanHeadMode) // bind to the correct DLL interface to ThorDAQ.dll
            {   
                case ScanHeadType.Res_Galvo:
                    FindCameras = new TDFindCameras(ThorDAQrgFindCameras);
                    SelectCamera = new TDSelectCamera(ThorDAQrgSelectCamera);
                    SetParam = new TDSetParam(ThorDAQrgSetParam);
                    GetParam = new TDGetParam(ThorDAQrgGetParam);
                    PreflightAcquisition = new TDPreflightAcquisition(ThorDAQrgPreflightAcquisition);
                    PostflightAcquisition = new TDPostflightAcquisition(ThorDAQrgPostflightAcquisition);
                    SetupAcquisition = new TDSetupAcquisition(ThorDAQrgSetupAcquisition);
                    StartAcquisition = new TDStartAcquisition(ThorDAQrgStartAcquisition);
                    StatusAcquisitionEx = new TDStatusAcquisitionEx(ThorDAQrgStatusAcquisitionEx);
                    CopyAcquisition = new TDCopyAcquisition(ThorDAQrgCopyAcquisition);
                    TeardownCamera = new TDTeardownCamera(ThorDAQrgTeardownCamera);

                    break;

                case ScanHeadType.Galvo_Galvo:
                    FindCameras = new TDFindCameras(ThorDAQggFindCameras);
                    SelectCamera = new TDSelectCamera(ThorDAQggSelectCamera);
                    SetParam = new TDSetParam(ThorDAQggSetParam);
                    GetParam = new TDGetParam(ThorDAQggGetParam);
                    PreflightAcquisition = new TDPreflightAcquisition(ThorDAQggPreflightAcquisition);
                    PostflightAcquisition = new TDPostflightAcquisition(ThorDAQggPostflightAcquisition);
                    SetupAcquisition = new TDSetupAcquisition(ThorDAQggSetupAcquisition);
                    StartAcquisition = new TDStartAcquisition(ThorDAQggStartAcquisition);
                    StatusAcquisitionEx = new TDStatusAcquisitionEx(ThorDAQggStatusAcquisitionEx);
                    CopyAcquisition = new TDCopyAcquisition(ThorDAQggCopyAcquisition);
                    TeardownCamera = new TDTeardownCamera(ThorDAQggTeardownCamera);
                    
                    break;
            }

            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, "ThorDAQcl-GUI: call FindCameras()");
            int iStatus = FindCameras(out cameraCnt);
            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, "ThorDAQcl-GUI: call SelectCamera()");
            iStatus = SelectCamera(cameraCnt);

            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, "ThorDAQcl-GUI: call SetParam()");
            string errString = TDSetParams();
            if (errString != "OK")
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, (" FATAL ABORT! ERROR: " + errString) );
                return;
            }

            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, "ThorDAQcl-GUI: call PreflightAcquisition()");
            byte[] UnusedAcqArg = null;
            int retStatus = PreflightAcquisition(UnusedAcqArg);

            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, "ThorDAQcl-GUI: call SetupAcquisition()");
            retStatus = SetupAcquisition(UnusedAcqArg);

            // SetupAcquisition() calls ConfigAcqSettings(), which calculates _frameRate (LSMPARAM 0x11)
            
            GetParam((int)ICamera.Params.PARAM_FRAME_RATE, out _frameRate);
            string sFR = String.Format("{0} fps", _frameRate.ToString("#0.0"));
        /*    FPSlabel.Content = sFR; // copy to GUI screen  */

            // START ACQUISITION
            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, "ThorDAQcl-GUI: call StartAcquisition()");
            retStatus = StartAcquisition(UnusedAcqArg);

            // spawn the thread
            _bw.RunWorkerAsync();
        }

        public void StopAcquisition()
        {
            _acquisitionActiveFlag = false;
            _bw.CancelAsync();
            System.Threading.Thread.Sleep(500);

            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, "ThorDAQcl-GUI: STOP ACQ - call PostflightAcquisition()");
            byte[] UnusedAcqArg = null;
            int retStatus = PostflightAcquisition(UnusedAcqArg);

            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, "ThorDAQcl-GUI: call TeardownCamera()");
            retStatus = TeardownCamera();

        }

        private void bw_DoWork(object sender, DoWorkEventArgs e)
        {
            FrameInfoStruct moreAvailable;
//            UInt16[] pBuffer = new UInt16[BitmapWidth * BitmapHeight *4] ; // allocate all FOUR ADDC
            BackgroundWorker worker = sender as BackgroundWorker;

            while (true)
            {
                if ((worker.CancellationPending == true))
                {
                    e.Cancel = true;
                    break;
                }
                else
                {
                    if (IsPixelDataReady == false)
                    {
                        int iStatus = CopyAcquisition(ADC_unmanaged_DMAbuffer, out moreAvailable); // unmanaged mem from CPP DLL
                        // copy from unmanaged to managed memory - the "length" arg is count of
                        // "array elements", where ImageSource array is 16-bit (ushort) elements
                        Marshal.Copy(ADC_unmanaged_DMAbuffer, _pixelData, 0, _pixelData.Length);  // 
                        IsPixelDataReady = true;
                    }

                    // Perform a time consuming operation and report progress.
                    System.Threading.Thread.Sleep(10);
                    worker.ReportProgress(1);
                }
            };
        }

        private void bw_ProgressChanged(object sender, ProgressChangedEventArgs e)
        {
            if (IsPixelDataReady == true)
            {
                OnPropertyChanged("Bitmap");
                IsPixelDataReady = false;
            }
        }
        private void bw_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            System.Threading.Thread.Sleep(2);

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

        private void saveAs_Click(object sender, RoutedEventArgs e)
        {
            byte[] imageBuffer = new byte[_pixelData.Length * 2];
            string filename = SettingPath + @"\snap" + "_" + DateTime.Now.ToString("yyyy-MM-dd-hh-mm-ss-tt");
            Buffer.BlockCopy(_pixelData, 0, imageBuffer, 0, imageBuffer.Length);
            File.WriteAllBytes(filename, imageBuffer);
            UpdateConsoleStatus("Image is saved");
        }

        #endregion Methods

        private void PlayButton_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {

        }
    }

    public sealed class BooleanToVisibilityConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            var flag = false;
            if (value is bool)
            {
                flag = (bool)value;
            }
            else if (value is bool?)
            {
                var nullable = (bool?)value;
                flag = nullable.GetValueOrDefault();
            }
            else if (value is int)
            {
                int temp = (int)value;
                flag = (0 == temp) ? false : true;
            }
            if (parameter != null)
            {
                if (bool.Parse((string)parameter))
                {
                    flag = !flag;
                }
            }
            if (flag)
            {
                return Visibility.Visible;
            }
            else
            {
                return Visibility.Collapsed;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            var back = ((value is Visibility) && (((Visibility)value) == Visibility.Visible));
            if (parameter != null)
            {
                if ((bool)parameter)
                {
                    back = !back;
                }
            }
            return back;
        }

        #endregion Methods
    }


}