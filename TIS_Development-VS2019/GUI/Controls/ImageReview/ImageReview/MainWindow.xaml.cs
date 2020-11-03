namespace ImageReviewDll
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Net;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Text.RegularExpressions;
    using System.Threading;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Animation;
    using System.Windows.Media.Imaging;
    using System.Windows.Shapes;
    using System.Windows.Threading;
    using System.Xml;

    using DeepZoomGen;

    using FolderDialogControl;

    using ImageReviewDll.Model;
    using ImageReviewDll.View;
    using ImageReviewDll.ViewModel;

    using Microsoft.Practices.Composite.Modularity;
    using Microsoft.Practices.Composite.Regions;
    using Microsoft.Practices.Unity;
    using Microsoft.Win32;

    using ThorLogging;

    using ThorSharedTypes;

    #region Enumerations

    enum ViewTypes
    {
        ViewType2D = 0,
        ViewType3D = 1
    }

    #endregion Enumerations

    public partial class MainWindow : UserControl
    {
        #region Fields

        public ImageReviewViewModel _viewModel;

        private const string GENERATED_DIR = "/GeneratedImages";
        private const string JPEG_DIR = "/jpeg";
        private const int MAX_IMAGES_OPEN = 1;
        private const int OFFSET_FOR_RESIZE_HSCROLL = 30;
        private const int OFFSET_FOR_RESIZE_VSCROLL = 110;
        private const int PATH_LENGTH = 261;

        private static int Count = 0;

        private string prvsDisplayedImage = string.Empty;
        private List<string> _allImages = new List<string>();
        private BackgroundWorker _bwGenDeepZoom;
        private int _deletedGridIndex;
        private Grid _dynamicGrid = null;
        private DeepZoom _dz;
        private IntPtr _imageData;
        private List<string> _images = new List<string>();
        private double _imgWidth;
        private bool _isBorderMouseDown = false;
        private string _selectedFolder;
        private List<string> _selectedImages = new List<string>();

        #endregion Fields

        #region Constructors

        public MainWindow()
        {
            InitializeComponent();
            this.Loaded += new RoutedEventHandler(MainWindow_Loaded);
            this.Unloaded += new RoutedEventHandler(MainWindow_Unloaded);
            this.KeyDown += new KeyEventHandler(MainWindow_KeyDown);
            this.KeyUp += new KeyEventHandler(MainWindow_KeyUp);
            this.MasterView.PreviewMouseDown += new MouseButtonEventHandler(MasterView_PreviewMouseDown);

            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Initialized");
        }

        public MainWindow(ImageReviewViewModel imageReviewViewModel)
        {
            InitializeComponent();
            _viewModel = imageReviewViewModel;

            this.MasterView.DataContext = imageReviewViewModel;
            this.MasterView.DataContextliveImageVM = imageReviewViewModel;

            this.ImageView.DataContext = imageReviewViewModel;
            this.toolBarView.DataContext = imageReviewViewModel;
            this.DataContext = imageReviewViewModel;
            this.volumeView.DataContext = imageReviewViewModel;
            this.roiMasterView.DataContext = imageReviewViewModel;

            this.Loaded += new RoutedEventHandler(MainWindow_Loaded);
            this.Unloaded += new RoutedEventHandler(MainWindow_Unloaded);
            this.KeyDown += new KeyEventHandler(MainWindow_KeyDown);
            this.KeyUp += new KeyEventHandler(MainWindow_KeyUp);
            this.MasterView.PreviewMouseDown += new MouseButtonEventHandler(MasterView_PreviewMouseDown);
            imageReviewViewModel.IncreaseViewArea += new Action<bool>(AdjustImageViewSize);

            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Initialized");
        }

        #endregion Constructors

        #region Properties

        public ImageReviewViewModel ViewModel
        {
            set
            {
                this._viewModel = value;

                this.MasterView.DataContext = value;
                this.MasterView.DataContextliveImageVM = value;

                this.ImageView.DataContext = value;
                this.toolBarView.DataContext = value;
                this.DataContext = value;
                this.volumeView.DataContext = value;
                this.roiMasterView.DataContext = value;

                _viewModel.LoadViewModelSettingsDoc();
            }
        }

        #endregion Properties

        #region Methods

        // Called from Experiment Review application window
        public void LoadLastExperiment()
        {
            MasterView.LoadLastExperiment();
        }

        [DllImport(".\\ExperimentManager.dll", EntryPoint = "GetActiveExperimentPath", CharSet = CharSet.Unicode)]
        private static extern int GetActiveExperimentPath(StringBuilder path, int length);

        [DllImport(".\\ExperimentManager.dll", EntryPoint = "GetActiveExperimentPathAndName", CharSet = CharSet.Unicode)]
        private static extern int GetActiveExperimentPathAndName(StringBuilder path, int length);

        [DllImport(".\\ThorDiskIO.dll", EntryPoint = "ReadImage")]
        private static extern int ReadImage([MarshalAs(UnmanagedType.LPWStr)]string path, ref IntPtr outputBuffer);

        [DllImport(".\\ThorDiskIO.dll", EntryPoint = "ReadImageInfo")]
        private static extern int ReadImageInfo([MarshalAs(UnmanagedType.LPWStr)]string path, ref int width, ref int height, ref int colorChannels);

        //when the image is bigger than the dispaly area, increase the display area so the scroll bar
        // can see the entire image
        void AdjustImageViewSize(bool obj)
        {
            if (true == obj)
            {
                ImageReviewViewModel vm = ((ImageReviewViewModel)this.DataContext);
                if (null == vm)
                {
                    return;
                }
                this.ImageView.Height = Math.Max(vm.IVScrollBarHeight, Application.Current.MainWindow.ActualHeight - OFFSET_FOR_RESIZE_VSCROLL);
            }
        }

        void AdjustViewSizes()
        {
            this.scrollView.Height = Math.Max(1, Application.Current.MainWindow.ActualHeight - OFFSET_FOR_RESIZE_VSCROLL);
            this.scrollViewImage.Height = Math.Max(1, Application.Current.MainWindow.ActualHeight - OFFSET_FOR_RESIZE_VSCROLL);
            this.ImageView.Height = Math.Max(1, Application.Current.MainWindow.ActualHeight - OFFSET_FOR_RESIZE_VSCROLL);
            this.volumeView.Height = Math.Max(1, Application.Current.MainWindow.ActualHeight - OFFSET_FOR_RESIZE_VSCROLL);
            this.volumeView.Width = Math.Max(1, Application.Current.MainWindow.ActualWidth - this.MasterView.Width - this.toolBarView.Width - OFFSET_FOR_RESIZE_HSCROLL);

            const int OFFSET_FOR_TOOLBAR_RESIZE_VSCROLL = 161;
            this.toolBarView.Height = Math.Max(1, Application.Current.MainWindow.ActualHeight - OFFSET_FOR_TOOLBAR_RESIZE_VSCROLL);

            ImageReviewViewModel vm = ((ImageReviewViewModel)this.DataContext);
            if (null == vm)
            {
                return;
            }
            vm.IVHeight = this.ImageView.Height;
        }

        void border_MouseDown(object sender, MouseButtonEventArgs e)
        {
            try
            {
                _isBorderMouseDown = true;
                string currentClickedImage = ((sender as Border).Child as Image).ToolTip.ToString();

                if (_selectedImages != null && _selectedImages.Count > 0)
                {
                    if (_selectedImages[0].Equals(currentClickedImage))
                    {
                        //MessageBox.Show("Image Already Displayed!");
                        _isBorderMouseDown = false;
                        return;
                    }
                }

                Mouse.OverrideCursor = Cursors.Wait;

                if ((_dynamicGrid.Children.Count) / MAX_IMAGES_OPEN >= MAX_IMAGES_OPEN)
                {
                    Grid tempHeaderGrid = _dynamicGrid.Children[0] as Grid;
                    Button btn = tempHeaderGrid.Children[1] as Button;
                    btn_Click(btn, null);
                }

                prvsDisplayedImage = currentClickedImage;
                PlaceImageInGrid(((sender as Border).Child as Image).ToolTip.ToString());

                Border border = (Border)(sender as Border);
                border.BorderBrush = new SolidColorBrush(Colors.Yellow);
                border.BorderThickness = new Thickness(2);
                _selectedImages.Add(((sender as Border).Child as Image).ToolTip.ToString());

                if (_selectedImages.Count > MAX_IMAGES_OPEN)
                {

                    _selectedImages.RemoveAt(0);
                }
                _isBorderMouseDown = false;
                Mouse.OverrideCursor = Cursors.Arrow;
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error: border_MouseDown(Image selection changed)", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        private void btnSelectExperiment_Click(object sender, RoutedEventArgs e)
        {
            if (_viewModel == null)
            {
                return;
            }

            BrowseForFolderDialog dlg = new BrowseForFolderDialog();
            dlg.Title = "Select the Experiment Directory";
            dlg.InitialExpandedFolder = _selectedFolder;
            dlg.OKButtonText = "OK";

            string expPath = string.Empty;
            string dzFile = string.Empty;

            if (true == dlg.ShowDialog())
            {
                expPath = dlg.SelectedFolder;
                _viewModel.ExperimentFolderPath = expPath;

                dzFile = expPath + JPEG_DIR + "/DeepZoomViewTestPage.html";
                expPath = expPath + "/Experiment.xml";
                _viewModel.ExperimentXMLPath = expPath;
            }
            else
            {   //do nothing if user cancels the folder selection
                return;
            }

            _selectedFolder = dlg.SelectedFolder;

            if (File.Exists(dzFile))
            {

                UpdateWebBrowser(dzFile);

            }
            else
            {
                _bwGenDeepZoom.RunWorkerAsync();

            }
        }

        void btn_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                Button temp = (sender as Button);
                DependencyObject DP = temp.Parent;

                int index = Grid.GetColumn((DP as Grid));
                _deletedGridIndex = _dynamicGrid.Children.IndexOf(DP as Grid);

                _dynamicGrid.Children.RemoveAt(_deletedGridIndex);//remove header grid
                _dynamicGrid.Children.RemoveAt(_deletedGridIndex);//remove splitter

                ResetColumnsFromDynamicGrid(index);

                if (!_isBorderMouseDown)
                {
                    string removedImage = string.Empty;

                    foreach (UIElement ele in (DP as Grid).Children)
                    {
                        if (ele.GetType().ToString().EndsWith("TextBlock"))
                        {
                            removedImage = (ele as TextBlock).ToolTip.ToString();
                            break;
                        }
                    }

                    if (_selectedImages.Contains(removedImage))
                    {
                        _selectedImages.Remove(removedImage);
                    }

                    _allImages.Remove(removedImage);
                }

                _dynamicGrid.ColumnDefinitions.RemoveAt(_dynamicGrid.ColumnDefinitions.Count - 1);//remove column

                _dynamicGrid.UpdateLayout();
                Count--;
                prvsDisplayedImage = string.Empty;

            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error: Image Close Button_Click", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        private void butOpen_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                OpenFileDialog ofd = new OpenFileDialog();

                ofd.Multiselect = true;
                if (false == ofd.ShowDialog())
                {
                    return;
                }

                if (ofd.FileNames.Length <= 0)
                {
                    return;
                }

                Mouse.OverrideCursor = Cursors.Wait;

                var images = from t in ofd.FileNames.ToList()
                             where t.EndsWith(".tif") || t.EndsWith(".tiff")
                             select t;

                _images = new List<string>();
                foreach (string newImage in images.ToList())
                {
                    if (!_allImages.Contains(newImage))
                    {
                        int rw = 0, rh = 0, rchan = 0;
                        ReadImageInfo(newImage, ref rw, ref rh, ref rchan);
                        if (rchan == 1)
                        {
                            _images.Add(newImage);
                            _allImages.Add(newImage);
                        }
                        else
                        {
                            throw new System.FormatException("Color images are not supported!");
                        }
                    }
                }

                if (_images.Count > 0)
                {
                    //System.Threading.Thread threadOne = new System.Threading.Thread(
                    //                                           new System.Threading.ThreadStart(
                    //                                           delegate()
                    //                                           {
                    //                                               svthumbnails.Dispatcher.Invoke(
                    //                                                 System.Windows.Threading.DispatcherPriority.Normal,
                    //                                                 new Action(
                    //                                                   delegate()
                    //                                                   {
                    //                                                       CreateAllImagesThubnail();
                    //                                                   }
                    //                                               ));

                    //                                           }));
                    //threadOne.Start();

                    if (string.IsNullOrEmpty(prvsDisplayedImage))
                        prvsDisplayedImage = _images[0];
                    else
                        prvsDisplayedImage = _selectedImages[0];

                    _selectedImages = new List<string>();
                    _selectedImages.Add(_images[0]);

                    //System.Threading.Thread threadTwo = null;

                    //threadTwo = new System.Threading.Thread(
                    //                          new System.Threading.ThreadStart(
                    //                          delegate()
                    //                          {
                    //                              sv.Dispatcher.Invoke(
                    //                                System.Windows.Threading.DispatcherPriority.Normal,
                    //                                new Action(
                    //                                  delegate()
                    //                                  {
                    //                                      LoadMainFiles(_selectedImages);
                    //                                  }
                    //                              ));

                    //                          }));

                    //threadTwo.Start();
                }
                else
                {
                    MessageBox.Show("No new images to display.", "butOpen_Click", MessageBoxButton.OK, MessageBoxImage.Information);
                    Mouse.OverrideCursor = Cursors.Arrow;
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error: butOpen_Click", MessageBoxButton.OK, MessageBoxImage.Error);
                Mouse.OverrideCursor = Cursors.Arrow;
            }
        }

        private void ClearDynamicGrid()
        {
            try
            {

                _dynamicGrid.Children.Clear();
                _dynamicGrid.ColumnDefinitions.Clear();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "ClearDynamicGrid", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        private void CreateAllImagesThubnail()
        {
            if (Mouse.OverrideCursor == Cursors.Arrow)
            {
                Mouse.OverrideCursor = Cursors.Wait;
            }

            foreach (string str in _images)
            {
                try
                {
                    int rw = 0, rh = 0, rchan = 0;
                    PixelFormat pf = PixelFormats.Gray16;
                    int w = 90; int h = 80;
                    Image image = new Image();
                    ReadImageInfo(str, ref rw, ref rh, ref rchan);

                    _imageData = Marshal.AllocHGlobal(rw * rh * rchan * 2);  //_viewModel.MaxChannels

                    pf = PixelFormats.Gray16;

                    ReadImage(str, ref _imageData);
                    int rawStride = (rw * pf.BitsPerPixel + 7) / 8;
                    WriteableBitmap bitmap = new WriteableBitmap(rw, rh, 96, 96, pf, null);
                    image.Height = h; image.Width = w;
                    bitmap.WritePixels(new Int32Rect(0, 0, rw, rh), _imageData, rw * rh * rchan * 2, rawStride); //_viewModel.MaxChannels
                    image.Source = bitmap;
                    image.Stretch = Stretch.Uniform;
                    image.Margin = new Thickness(1);
                    image.ToolTip = str;
                    image.MouseDown += new MouseButtonEventHandler(image_MouseDown);
                    Border border;
                    if (_selectedImages.Contains(str))
                    {
                        border = new Border()
                        {
                            Visibility = Visibility.Visible,
                            Height = image.Height,
                            CornerRadius = new CornerRadius(2),
                            BorderThickness = new Thickness(1),
                            Background = new SolidColorBrush(Colors.Transparent),
                            BorderBrush = new SolidColorBrush(Colors.Yellow)
                        };
                    }
                    else
                    {
                        border = new Border()
                        {
                            Visibility = Visibility.Visible,
                            Height = image.Height,
                            CornerRadius = new CornerRadius(2),
                            BorderThickness = new Thickness(1),
                            Background = new SolidColorBrush(Colors.Transparent),
                            BorderBrush = new SolidColorBrush(Colors.Transparent),
                        };
                    }

                    border.Child = image;
                    border.MouseDown += new MouseButtonEventHandler(border_MouseDown);

                }
                catch (Exception ex)
                {
                    MessageBox.Show(ex.Message, "Error: Create All Images Thubnail", MessageBoxButton.OK, MessageBoxImage.Error);
                }
                finally
                {
                    Marshal.FreeHGlobal(_imageData);
                }
            }
            Mouse.OverrideCursor = Cursors.Arrow;
        }

        //        [DllImport(".\\ThorDiskIO.dll", EntryPoint = "ReadImageInfo")]
        //        private static extern int ReadImageInfo([MarshalAs(UnmanagedType.LPWStr)]string path, ref int width, ref int height, ref int colorChannels);
        private void CreateContainer()
        {
            try
            {
                _dynamicGrid = new Grid();

                _dynamicGrid.HorizontalAlignment = HorizontalAlignment.Left;
                _dynamicGrid.VerticalAlignment = VerticalAlignment.Top;
                _dynamicGrid.Background = new SolidColorBrush(Colors.Gray);

                RowDefinition gridRow1 = new RowDefinition();
                _dynamicGrid.RowDefinitions.Add(gridRow1);

                AdjustViewSizes();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "CreateContainer", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        private void CreateGridSplitter(int index)
        {
            try
            {
                GridSplitter gridSplitter = new GridSplitter();
                gridSplitter.Background = new SolidColorBrush(Colors.Yellow);
                gridSplitter.VerticalAlignment = VerticalAlignment.Stretch;
                gridSplitter.HorizontalAlignment = HorizontalAlignment.Right;
                gridSplitter.Width = 5;
                Grid.SetRow(gridSplitter, 0);
                Grid.SetColumn(gridSplitter, index);
                _dynamicGrid.Children.Add(gridSplitter);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "CreateGridSplitter", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        private Grid createHeader(string ImagePath)
        {
            try
            {
                Grid headerGrid = new Grid();

                ColumnDefinition gridCol1 = new ColumnDefinition();
                ColumnDefinition gridCol2 = new ColumnDefinition();

                RowDefinition gridrow1 = new RowDefinition();
                RowDefinition gridrow2 = new RowDefinition();

                headerGrid.ColumnDefinitions.Add(gridCol1);
                headerGrid.ColumnDefinitions.Add(gridCol2);
                headerGrid.RowDefinitions.Add(gridrow1);
                headerGrid.RowDefinitions.Add(gridrow2);

                TextBlock lbl = new TextBlock();
                lbl.FontSize = 12;
                string ImageTitle = ImagePath.Split(new char[1] { '\\' }).Last();
                lbl.Text = ImageTitle;
                lbl.HorizontalAlignment = HorizontalAlignment.Center;
                lbl.ToolTip = ImagePath;
                lbl.HorizontalAlignment = HorizontalAlignment.Right;
                lbl.VerticalAlignment = VerticalAlignment.Center;

                Button btn = new Button();
                btn.Height = 30; btn.Width = 30;
                btn.HorizontalAlignment = HorizontalAlignment.Right;
                btn.VerticalAlignment = VerticalAlignment.Top;
                btn.Click += new RoutedEventHandler(btn_Click);
                Style buttonStyle = (Style)mwUserControl.FindResource("CloseButtonStyle");
                btn.Style = buttonStyle;

                Grid.SetRow(lbl, 0);
                Grid.SetColumn(lbl, 0);
                Grid.SetRow(btn, 0);
                Grid.SetColumn(btn, 1);

                headerGrid.Children.Add(lbl);
                headerGrid.Children.Add(btn);

                return headerGrid;
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "createHeader", MessageBoxButton.OK, MessageBoxImage.Error);
                return null;
            }
        }

        private void GenerateDeepZoomData(string baseDirectory)
        {
            if (!Directory.Exists(baseDirectory + JPEG_DIR + GENERATED_DIR))
            {
                _dz = new DeepZoom();
                _dz.Source = baseDirectory + JPEG_DIR;
                _dz.Destination = baseDirectory + JPEG_DIR + GENERATED_DIR;
                _dz.Generate();
                _dz.CreateCollectionAndMetadata();

                //update the Metadata.xml
                _dz.UpdateMetadataXML(baseDirectory + JPEG_DIR);
            }
            //update the controls with the active experiment information

            XmlDocument expDoc = new XmlDocument();

            expDoc.Load(baseDirectory + "/Experiment.xml");

            XmlNodeList nodeList = expDoc.SelectNodes("/ThorImageExperiment/Sample");

            string str = string.Empty;

            int sampleType = 0;
            int wellRows = 1;
            int wellColumns = 1;
            int subImageRows = 1;
            int subImageColumns = 1;

            if (XmlManager.GetAttribute(nodeList[0], expDoc, "type", ref str))
            {
                sampleType = Convert.ToInt32(str);
            }
            nodeList = expDoc.SelectNodes("/ThorImageExperiment/Sample/Wells");

            if (XmlManager.GetAttribute(nodeList[0], expDoc, "rows", ref str))
            {
                wellRows = Convert.ToInt32(str);
            }

            if (XmlManager.GetAttribute(nodeList[0], expDoc, "columns", ref str))
            {
                wellColumns = Convert.ToInt32(str);
            }
            nodeList = expDoc.SelectNodes("/ThorImageExperiment/Sample/Wells/SubImages");
            if (XmlManager.GetAttribute(nodeList[0], expDoc, "subRows", ref str))
            {
                subImageRows = Convert.ToInt32(str);
            }

            if (XmlManager.GetAttribute(nodeList[0], expDoc, "subColumns", ref str))
            {
                subImageColumns = Convert.ToInt32(str);
            }

            File.Copy(Application.Current.Resources["ApplicationSettingsFolder"].ToString() + "\\DeepZoomView.xap", baseDirectory + JPEG_DIR + "\\DeepZoomView.xap", true);
            File.Copy(Application.Current.Resources["ApplicationSettingsFolder"].ToString() + "\\DeepZoomViewTestPage.html", baseDirectory + JPEG_DIR + "\\DeepZoomViewTestPage.html", true);

            string metaDataPath = baseDirectory + JPEG_DIR + GENERATED_DIR + "/Metadata.xml";

            XmlDocument metadataDoc = new XmlDocument();

            metadataDoc.Load(metaDataPath);

            StringWriter strw = new StringWriter();
            XmlTextWriter writer = new XmlTextWriter(strw);
            metadataDoc.WriteTo(writer);

            string metadata = System.Security.SecurityElement.Escape(strw.ToString());

            string strResult;

            using (StreamReader sr = new StreamReader(baseDirectory + JPEG_DIR + "/DeepZoomViewTestPage.html"))
            {
                string strContent = sr.ReadToEnd();

                if (strContent == null)
                {
                    return;
                }

                int startRow;
                int startColumn;

                sampleType = Convert.ToInt32(sampleType);

                startRow = 1;
                startColumn = 1;

                string pat = @"(param name=""initParams"" value="")(.*)(sampleType=)\d+";
                string patRep = "${1}" + "${2}" + "${3}" + sampleType.ToString();
                strResult = Regex.Replace(strContent, pat, patRep);

                pat = @"(param name=""initParams"" value="")(.*)(startRow=)\d+";
                patRep = "${1}" + "${2}" + "${3}" + startRow.ToString();
                strResult = Regex.Replace(strResult, pat, patRep);

                pat = @"(param name=""initParams"" value="")(.*)(startColumn=)\d+";
                patRep = "${1}" + "${2}" + "${3}" + startColumn.ToString();
                strResult = Regex.Replace(strResult, pat, patRep);

                pat = @"(param name=""initParams"" value="")(.*)(wellRows=)\d+";
                patRep = "${1}" + "${2}" + "${3}" + wellRows.ToString();
                strResult = Regex.Replace(strResult, pat, patRep);

                pat = @"(param name=""initParams"" value="")(.*)(wellColumns=)\d+";
                patRep = "${1}" + "${2}" + "${3}" + wellColumns.ToString();
                strResult = Regex.Replace(strResult, pat, patRep);

                pat = @"(param name=""initParams"" value="")(.*)(subRows=)\d+";
                patRep = "${1}" + "${2}" + "${3}" + subImageRows.ToString();
                strResult = Regex.Replace(strResult, pat, patRep);

                pat = @"(param name=""initParams"" value="")(.*)(subColumns=)\d+";
                patRep = "${1}" + "${2}" + "${3}" + subImageColumns.ToString();
                strResult = Regex.Replace(strResult, pat, patRep);

                pat = @"(param name=""initParams"" value="")(.*)(metadata=)(.*)";
                patRep = "${1}" + "${2}" + "${3}" + metadata + "${4}";
                strResult = Regex.Replace(strResult, pat, patRep);
            }

            if (strResult == null)
            {
            }

            using (StreamWriter sw = new StreamWriter(baseDirectory + JPEG_DIR + "/DeepZoomViewTestPage.html"))
            {
                sw.Write(strResult);
            }
        }

        void image_MouseDown(object sender, MouseButtonEventArgs e)
        {
        }

        private void LoadMainFiles(IEnumerable<string> selectedImages)
        {
            try
            {
                if (Mouse.OverrideCursor == Cursors.Arrow)
                {
                    Mouse.OverrideCursor = Cursors.Wait;
                }

                foreach (string str in selectedImages)
                {
                    if (_dynamicGrid.Children.Count > 0)
                    {
                        ClearDynamicGrid();
                    }
                    PlaceImageInGrid(str);
                }

                Mouse.OverrideCursor = Cursors.Arrow;
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error: Load Main Images");
            }
        }

        void MainWindow_KeyDown(object sender, KeyEventArgs e)
        {
            this.ImageView.ImageView_KeyDown(sender, e);
        }

        void MainWindow_KeyUp(object sender, KeyEventArgs e)
        {
            this.ImageView.ImageView_KeyUp(sender, e);
        }

        void MainWindow_Loaded(object sender, RoutedEventArgs e)
        {
            _viewModel.ViewType = Convert.ToInt32(ViewTypes.ViewType2D);

            Application.Current.MainWindow.SizeChanged += new SizeChangedEventHandler(MainWindow_SizeChanged);

            CreateContainer();

            AdjustViewSizes();

            _bwGenDeepZoom = new BackgroundWorker();

            _bwGenDeepZoom.WorkerReportsProgress = true;
            _bwGenDeepZoom.WorkerSupportsCancellation = true;
            _bwGenDeepZoom.DoWork += new DoWorkEventHandler(_bwGenDeepZoom_DoWork);
            _bwGenDeepZoom.ProgressChanged += new ProgressChangedEventHandler(_bwGenDeepZoom_ProgressChanged);
            _bwGenDeepZoom.RunWorkerCompleted += new RunWorkerCompletedEventHandler(_bwGenDeepZoom_RunWorkerCompleted);

            _selectedFolder = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments).ToString();

            _viewModel.SetDisplayOptions();
        }

        void MainWindow_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            AdjustViewSizes();
        }

        void MainWindow_Unloaded(object sender, RoutedEventArgs e)
        {
            if (null != Application.Current.MainWindow)
            {
                Application.Current.MainWindow.SizeChanged -= new SizeChangedEventHandler(MainWindow_SizeChanged);
            }
            _viewModel.CloseAll();
        }

        void MasterView_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            this.ImageView.SetSelectROI();
        }

        private void PlaceImageInGrid(string str)
        {
            try
            {
                int width = 0, height = 0, colorChannels = 0;
                ReadImageInfo(str, ref width, ref height, ref colorChannels);

                BitmapImage image = new BitmapImage();
                _imgWidth = width;
                _dynamicGrid.Width += _imgWidth;

                ColumnDefinition gridCol1 = new ColumnDefinition();
                _dynamicGrid.ColumnDefinitions.Add(gridCol1);

                Grid headerPanel = createHeader(str);

                ImageView iv = new ImageView();
                ImageReview model = new ImageReview();
                ImageReviewViewModel irvm = new ImageReviewViewModel(null, null, null, model);
                irvm.FileFormatMode = ImageReview.FormatMode.CUSTOM;
                irvm.ChannelEnableA = true;
                irvm.ChannelEnableB = irvm.ChannelEnableC = irvm.ChannelEnableD = false;
                iv.DataContext = irvm;
                irvm.ImagePathandName = str;
                irvm.BuildChannelPalettes();

                //setting image in header pannel grid
                headerPanel.Children.Add(iv);
                Grid.SetRow(iv, 1);
                Grid.SetColumnSpan(iv, 2);

                //seeting header panel in dynamic grid
                _dynamicGrid.Children.Add(headerPanel);
                Grid.SetRow(headerPanel, 0);
                Grid.SetColumn(headerPanel, Count);
                CreateGridSplitter(Count);
                Count++;
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error: PlaceImageInGrid", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        private void ResetColumnsFromDynamicGrid(int index)
        {
            for (int i = index; i < _dynamicGrid.ColumnDefinitions.Count - 1; i++)
            {
                _dynamicGrid.Children[_deletedGridIndex].SetValue(Grid.ColumnProperty, i);
                _dynamicGrid.Children[_deletedGridIndex + 1].SetValue(Grid.ColumnProperty, i);
                _deletedGridIndex += 2;
            }
        }

        private void scrollViewImage_ScrollChanged(object sender, ScrollChangedEventArgs e)
        {
            ImageReviewViewModel vm = ((ImageReviewViewModel)this.DataContext);
            if (null == vm)
            {
                return;
            }
            vm.ROItoolbarMargin = new Thickness(0, scrollViewImage.ContentVerticalOffset, 0, 0);
        }

        void ti_CloseTab(object sender, RoutedEventArgs e)
        {
            TabItem tabItem = e.Source as TabItem;
            if (tabItem != null)
            {
                TabControl tabControl = tabItem.Parent as TabControl;
                if (tabControl != null)
                    tabControl.Items.Remove(tabItem);
            }
        }

        private void UpdateWebBrowser(string file)
        {
        }

        void _bwGenDeepZoom_DoWork(object sender, DoWorkEventArgs e)
        {
            if (Directory.Exists(_selectedFolder))
            {
                GenerateDeepZoomData(_selectedFolder);
            }
        }

        void _bwGenDeepZoom_ProgressChanged(object sender, ProgressChangedEventArgs e)
        {
        }

        void _bwGenDeepZoom_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            string str = _selectedFolder + JPEG_DIR + "/DeepZoomViewTestPage.html";
            UpdateWebBrowser(str);
        }

        #endregion Methods
    }

    public class ViewTypeToVisibilityConverter : IValueConverter
    {
        #region Methods

        object IValueConverter.Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            Visibility val = Visibility.Collapsed;
            switch (Convert.ToInt32(value))
            {
                case 0:
                    {
                        if (0 == Convert.ToInt32(parameter))
                            val = Visibility.Visible;
                        else
                            val = Visibility.Hidden;
                    }
                    break;
                case 1:
                    {
                        if (1 == Convert.ToInt32(parameter))
                            val = Visibility.Visible;
                        else
                            val = Visibility.Hidden;
                    }
                    break;

            }
            return val;
        }

        object IValueConverter.ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            int val = 0;
            switch ((Visibility)value)
            {
                case Visibility.Visible:
                    {
                        if (0 == Convert.ToInt32(parameter))
                            val = 0;
                        else
                            val = 1;
                    }
                    break;
                case Visibility.Collapsed:
                    {
                        if (1 == Convert.ToInt32(parameter))
                            val = 0;
                        else
                            val = 1;
                    }
                    break;

            }
            return val;
        }

        #endregion Methods
    }
}