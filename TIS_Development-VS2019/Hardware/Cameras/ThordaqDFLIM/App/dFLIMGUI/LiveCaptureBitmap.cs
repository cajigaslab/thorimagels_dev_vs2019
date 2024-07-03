namespace thordaqGUI
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
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

    /// <summary>
    /// Interaction logic for LiveCaptureBitmap.xaml
    /// </summary>
    public partial class MainWindow : Window, INotifyPropertyChanged
    {
        #region Fields

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


        #endregion Fields

        #region Properties


        public WriteableBitmap Bitmap
        {
            get
            {
                //if (_bitmap == null)
                //{
                //    _bitmap = new WriteableBitmap(BitmapWidth, BitmapHeight, 96, 96, PixelFormats.Indexed8, null);
                //}
                //if (ImageSource.Length == (BitmapWidth * BitmapHeight))
                //{
                //    ImageSourceByte = GetPixelDataByte(ImageSource);
                //    //copy the pixel data into the _bitmap
                //    _bitmap.WritePixels(new Int32Rect(0, 0, _bitmap.PixelWidth, _bitmap.PixelHeight), ImageSourceByte, _bitmap.PixelWidth, 0);
                //}
                return _bitmap;
            }
            set 
            {
                this._bitmap = value;
            }
        }

        public byte[] GetPixelDataByte(short[] pixelData)
        {
            double diff = (double)byte.MaxValue / (ImageSource.Max() - ImageSource.Min());
            double min = ImageSource.Min();
            for (int i = 0; i < ImageSource.Length; i++)
            {
                ImageSourceByte[i] = (byte)(((double)ImageSource[i] - min) * diff);
            }
            return ImageSourceByte;
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

        public short[] ImageSource
        {
            get;
            set;
        }

        public byte[] ImageSourceByte
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
            _bw.CancelAsync();
            _bw.DoWork -= new DoWorkEventHandler(bw_DoWork);
            _bw.ProgressChanged -= new ProgressChangedEventHandler(bw_ProgressChanged);
            _bw.RunWorkerCompleted -= new RunWorkerCompletedEventHandler(bw_RunWorkerCompleted);
        }

        public void StartAcqusition()
        {
            _bw.RunWorkerAsync();
        }

        public void StopAcquisition()
        {
            IntPtr unusedPtr = IntPtr.Zero;
            _acquisitionActiveFlag = false;
            Win32.PostflightAcquisition(unusedPtr);
            _bw.CancelAsync();
            System.Threading.Thread.Sleep(500);

        }

        private void bw_DoWork(object sender, DoWorkEventArgs e)
        {
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
                    // Perform a time consuming operation and report progress.
                    System.Threading.Thread.Sleep(20);
                    worker.ReportProgress(0);
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
            if ((e.Cancelled == true))
            {
            }

            else if (!(GetError(e) == null))
            {
            }

            else
            {
            }
        }

        private static Exception GetError(RunWorkerCompletedEventArgs e)
        {
            return e.Error;
        }

        private void saveAs_Click(object sender, RoutedEventArgs e)
        {
            byte[] imageBuffer = new byte[ImageSource.Length * 2];
            string filename = SettingPath + @"\snap" + "_" + DateTime.Now.ToString("yyyy-MM-dd-hh-mm-ss-tt");
            Buffer.BlockCopy(ImageSource, 0, imageBuffer, 0, imageBuffer.Length);
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