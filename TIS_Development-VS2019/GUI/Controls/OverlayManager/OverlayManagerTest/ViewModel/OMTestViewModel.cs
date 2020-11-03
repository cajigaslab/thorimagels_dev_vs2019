namespace OverlayManagerTest.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Threading;
    using System.Xml;
    using System.Runtime.InteropServices; 
    //using BitMiracle.LibTiff.Classic;

    using OverlayManager;

    class OMTestViewModel : ViewModelBase
    {
        #region Fields

        public List<string> _tiffFiles;
        public int _tiffFilesIndex = 0;

        private WriteableBitmap _bitmap;
        private OverlayManagerParameters _overlayManagerParams;
        private OverlayManagerClass _overlayManger;
        private DispatcherTimer _updateImageTimer;

        #endregion Fields

        #region Constructors

        public OMTestViewModel()
        {
            _overlayManagerParams = new OverlayManagerParameters();            
            _overlayManagerParams.LSMChannel = 1;
            _overlayManagerParams.ColorChannels = 1;
            _overlayManagerParams.LSMChannelEnable[0] = true;
            _overlayManagerParams.LSMChannelEnable[1] = false;
            _overlayManagerParams.LSMChannelEnable[2] = false;
            _overlayManagerParams.LSMChannelEnable[3] = false;
            _overlayManagerParams.LSMFieldSize = 120;
            _overlayManagerParams.LSMPixelX = 1024;
            _overlayManagerParams.LSMPixelY = 1024;
            _overlayManagerParams.MaxChannels = 4;
            _overlayManagerParams.NAcquireChannels = 1;
            _overlayManagerParams.NumAvailableChannels = 1;
            _overlayManagerParams.ColorAssigment = new int[4];
            _overlayManagerParams.ColorAssigment[0] = 0;
            _overlayManagerParams.ColorAssigment[1] = 1;
            _overlayManagerParams.ColorAssigment[2] = 2;
            _overlayManagerParams.ColorAssigment[3] = 3;
            _overlayManagerParams.Bitmap = null;
            _overlayManger = new OverlayManagerClass(_overlayManagerParams);
        }

        #endregion Constructors

        #region Properties

        public WriteableBitmap Bitmap
        {
            get
            {

                return _bitmap;
            }

            set
            {
                _bitmap = value;
                _overlayManagerParams.Bitmap = _bitmap;
                _overlayManger.UpdateParams();
                OnPropertyChanged("Bitmap");
            }
        }

        public OverlayManagerClass OverlayManager
        {
            get
            {
                return _overlayManger;
            }
            set
            {
                _overlayManger = value;
            }
        }

        public List<string> TiffFiles
        {
            get
            {
                return _tiffFiles;
            }
            set
            {
                _tiffFiles = value;
            }
        }

        public DispatcherTimer UpdateImageTimer
        {
            get { return _updateImageTimer; }
            set { _updateImageTimer = value; }
        }

        #endregion Properties

        #region Methods

        public void ConnectHandlers()
        {
            _updateImageTimer = new DispatcherTimer();
            _updateImageTimer.Interval = TimeSpan.FromMilliseconds(30);
            _updateImageTimer.Tick += new EventHandler(_deviceReadTimer_Tick);
        }

        public void ReleaseHandlers()
        {            
            _updateImageTimer.Tick -= new EventHandler(_deviceReadTimer_Tick);
        }

        private int GetColorChannels()
        {
            int numChannels = 1;

            return numChannels;
        }

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "LoadTIFF")]
        private extern static IntPtr LoadTIFF(string tiffFile);

        private int N = 1024;
        private WriteableBitmap TiffToBMP(string tiffFile)
        {
            int width = N;
            int height = N;
            
            byte[] buffer = new byte[N * N *2];
            Marshal.Copy(LoadTIFF(tiffFile), buffer, 0, buffer.Length);
           
            _bitmap = new WriteableBitmap(width, height, 96, 96, PixelFormats.Gray16, null);
            var pixels = new ushort[width * height];
            int offset;
            for (var y = 0; y < height; y++)
            {
                for (var x = 0; x < width; x++)
               { 
                    offset = y * height + x;

                    pixels[offset] = (ushort)(buffer[offset * 2 + 1] << 8 | buffer[offset * 2]);
                }
            }
            
            _bitmap.WritePixels(new Int32Rect(0, 0, width, height), pixels, width * 2, 0);

            return _bitmap;
        }

        private void _deviceReadTimer_Tick(object sender, EventArgs e)
        {
            if (null == _tiffFiles)
                return;
            _tiffFilesIndex++;
            if (_tiffFiles.Count > _tiffFilesIndex)
            {
                this.Bitmap = TiffToBMP(_tiffFiles[_tiffFilesIndex]);
            }
            else
            {
                _tiffFilesIndex = 0;
                this.Bitmap = TiffToBMP(_tiffFiles[_tiffFilesIndex]);
            }
        }

        #endregion Methods
    }
}