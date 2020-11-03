namespace OverlayManagerTest
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Linq;
    using System.Text;
    using System.Threading;
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

    //using OverlayManager;
    using OverlayManagerTest.Commands;
    using OverlayManagerTest.ViewModel;

    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        #region Fields

        private bool _isPlay;
        private Point _scrollStartPoint;
        private OMTestViewModel _vm;
        private bool _updatingOverlayObject;

        #endregion Fields

        #region Constructors

        public MainWindow()
        {
            InitializeComponent();
            _vm = (OMTestViewModel)imageCanvas.Resources["vm"];
            this.DataContext = _vm;
            _vm.ConnectHandlers();
            this.Unloaded +=MainWindow_Unloaded;
            _vm.OverlayManager.UpdatingObjectEvent += new Action<bool>(VM_UpdatingObject);
        }

        #endregion Constructors

        #region Enumerations

        private enum MouseEvent
        {
            LEFTSINGLECLICK,
            RIGHTSINGLECLICK,
            LEFTDOUBLECLICK,
            RIGHTDOUBLECLICK,
            LEFTHOLDING,
            RIGHTHOLDING,
            LEFTMOUSEUP,
            RIGHTMOUSEUP
        }

        #endregion Enumerations

        #region Methods

        private void VM_UpdatingObject(bool obj)
        {
            _updatingOverlayObject = obj;
        }

        protected override void OnClosed(EventArgs e)
        {
            try
            {
                _vm.ReleaseHandlers();
            }
            catch { };
            base.OnClosed(e);

            Application.Current.Shutdown();
        }

        protected override void OnMouseDoubleClick(MouseButtonEventArgs e)
        {
            Point pt = this.TranslatePoint(e.GetPosition(this), imageCanvas);

            MouseEvent me = MouseEvent.LEFTDOUBLECLICK;
            _vm.OverlayManager.MouseEvent(Convert.ToInt16(me), ref overlayCanvas, new Point(_scrollStartPoint.X, _scrollStartPoint.Y));
        }

        protected override void OnMouseDown(MouseButtonEventArgs e)
        {
            if (IsMouseOver)
            {
                MouseEvent me = new MouseEvent();
                if (e.LeftButton == MouseButtonState.Pressed)   // left click
                {
                    me = MouseEvent.LEFTSINGLECLICK;
                    _scrollStartPoint = this.TranslatePoint(e.GetPosition(this), imageCanvas);
                    _vm.OverlayManager.MouseEvent(Convert.ToInt16(me), ref overlayCanvas, new Point(_scrollStartPoint.X, _scrollStartPoint.Y));
                    CaptureMouse();
                }
                else if (e.RightButton == MouseButtonState.Pressed) // right click
                {
                    me = MouseEvent.RIGHTDOUBLECLICK;
                }

            }

            base.OnMouseDown(e);
        }

        /// <summary>
        /// If IsMouseCaptured scroll to correct position. 
        /// Where position is updated by animation timer
        /// </summary>
        protected override void OnMouseMove(MouseEventArgs e)
        {
            if (IsMouseCaptured)
            {
                if (image1.ImageSource != null)
                {
                    if (e.LeftButton == MouseButtonState.Pressed)   // left click
                    {
                        Point currentPoint = this.TranslatePoint(e.GetPosition(this), imageCanvas);
                        MouseEvent me = new MouseEvent();
                        me = MouseEvent.LEFTHOLDING;
                        currentPoint.X = Math.Max(0, Math.Min(currentPoint.X, image1.ImageSource.Width));
                        currentPoint.Y = Math.Max(0, Math.Min(currentPoint.Y, image1.ImageSource.Height));
                        _vm.OverlayManager.MouseEvent(Convert.ToInt16(me), ref overlayCanvas, new Point(currentPoint.X, currentPoint.Y));
                    }
                }
            }

            base.OnMouseMove(e);
        }

        /// <summary>
        /// Release MouseCapture if its captured
        /// </summary>
        /// <param name="e"></param>
        protected override void OnMouseUp(MouseButtonEventArgs e)
        {
            if (IsMouseCaptured)
            {
                if (null != _vm.Bitmap)
                {
                    if (e.LeftButton == MouseButtonState.Released)
                    {
                        MouseEvent me = new MouseEvent();
                        me = MouseEvent.LEFTMOUSEUP;
                        Point currentPoint = this.TranslatePoint(e.GetPosition(this), imageCanvas);
                        _vm.OverlayManager.MouseEvent(Convert.ToInt16(me), ref overlayCanvas, new Point(currentPoint.X, currentPoint.Y));
                    }
                }

                Cursor = Cursors.Arrow;
                ReleaseMouseCapture();
            }

            base.OnMouseUp(e);
        }

        private void ClearOverlayCanvas_Click(object sender, RoutedEventArgs e)
        {
            _vm.OverlayManager.ClearAllObjects(ref overlayCanvas);
        }

        private void CommandBinding_CanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = true;
        }

        private void CommandBinding_CanExecute_Close(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = true;
        }

        private void CommandBinding_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            //setting the return value
            //in the real world we probably wont have to hold a state flag "isPlay" we'll figure that out as a part of the alg
            if (e.Parameter == null)
                //the command came from menu
                ((CustomCommand)e.Command).ReturnValue = _isPlay = !_isPlay;
            else
                //the command came from toggle button
                ((CustomCommand)e.Command).ReturnValue = _isPlay = (bool)e.Parameter;

            // TODO: insert command logic here
            if (true == _isPlay)
            {
                PlayImages();
            }
            else
            {
                PauseImages();
            }

            e.Handled = true;
        }

        private void CommandBinding_Executed_Close(object sender, ExecutedRoutedEventArgs e)
        {
            Close();
        }

        private void createLineROI_Click(object sender, RoutedEventArgs e)
        {
            _vm.OverlayManager.InitROILine(ref overlayCanvas);
        }

        private void createPolyROI_Click(object sender, RoutedEventArgs e)
        {
            _vm.OverlayManager.InitROIPoly(ref overlayCanvas);
        }

        private void createRectROI_Click(object sender, RoutedEventArgs e)
        {
            _vm.OverlayManager.InitROIRect(ref overlayCanvas);
        }

        private void createReticle_Click(object sender, RoutedEventArgs e)
        {
            _vm.OverlayManager.InitReticle(ref overlayCanvas, true);
        }

        private void MainWindow_Unloaded(object sender, RoutedEventArgs e)
        {
            _vm.OverlayManager.PersistSaveROIs();
        }

        private void createCrosshairROI_Click(object sender, RoutedEventArgs e)
        {
            _vm.OverlayManager.InitROICrosshair(ref overlayCanvas);
        }

        private void PauseImages()
        {
            _vm.UpdateImageTimer.Stop();
            _vm.OverlayManager.PersistSaveROIs();
        }

        private void PlayImages()
        {
            _vm.UpdateImageTimer.Start();
            string testImageFolder = "..\\..\\..\\TestImages";

            DirectoryInfo d = new DirectoryInfo(testImageFolder);//Assuming Test is your Folder
            FileInfo[] Files = d.GetFiles("*.tif"); //Getting Text files

            List<string> tiffImage = new List<string>();

            for (int n = 0; n < Files.Length; n++)
            {
                tiffImage.Add(Files[n].Directory.ToString() + "\\" + Files[n].Name);
            }

            _vm.TiffFiles = tiffImage;
            _vm.OverlayManager.PersistLoadROIs(ref overlayCanvas);
        }

        #endregion Methods
    }
}