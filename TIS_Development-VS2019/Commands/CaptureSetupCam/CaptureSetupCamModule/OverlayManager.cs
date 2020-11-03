namespace CaptureSetupDll.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Shapes;

    using CaptureSetupDll.View;

    using Microsoft.Win32;

    public class OverlayManager
    {
        #region Fields

        public static bool _adornerProvided = false;

        private AdornerLayer layer;
        private Line line = null;
        private Rectangle rectangle = null;
        private int _activeOverlayIndex = 0;
        private Type _activeType;
        private Point _endPoint;
        private LineProfile _lineProfile = new LineProfile();
        private int _lineWidth = 1;
        private ContextMenu _overlayContextMenu = null;
        private MenuItem _overlayContextMenuItem = null;
        private ROIStats _roiInfo = new ROIStats();
        private int _saveDlgFilterIndex = 0;
        private Point _startPoint;

        #endregion Fields

        #region Enumerations

        public enum OverlayType
        {
            RECTANGLE,
            LINE
        }

        #endregion Enumerations

        #region Events

        public event Action ClosingLineProfile;

        public event Action ClosingROIStats;

        public event Action LineWidthChange;

        #endregion Events

        #region Properties

        public ROIStats ROIStatsWindow
        {
            get
            {
                if (this._roiInfo != null)
                    return this._roiInfo;
                else
                    return null;
            }
        }

        #endregion Properties

        #region Methods

        public void CloseDialog()
        {
            if (_roiInfo.IsLoaded == true)
            {
                _roiInfo.Close();
            }
        }

        public void CreateObject(OverlayType ot, ref Canvas canvas, Point pt)
        {
            switch (ot)
            {
                case OverlayType.RECTANGLE:
                    {

                        rectangle = new Rectangle();
                        line = null;
                        rectangle.Fill = new SolidColorBrush(Colors.Transparent);

                        if (_overlayContextMenu == null)
                        {
                            CreateContextMenu();
                        }
                        rectangle.ContextMenu = _overlayContextMenu;

                        rectangle.Width = 1;
                        rectangle.Height = 1;

                        Canvas.SetTop(rectangle, pt.Y);
                        Canvas.SetLeft(rectangle, pt.X);
                        rectangle.Stroke = Brushes.Yellow;
                        rectangle.StrokeThickness = 1;

                        _activeOverlayIndex = canvas.Children.Add(rectangle);

                        _startPoint = pt;

                        if (_roiInfo.IsLoaded == false)
                        {
                            _roiInfo = new ROIStats();
                            _roiInfo.Show();
                            _roiInfo.Closing += new System.ComponentModel.CancelEventHandler(RoiInfo_Closing);
                        }

                        _roiInfo.ROITop = Convert.ToInt32(pt.Y);
                        _roiInfo.ROILeft = Convert.ToInt32(pt.X);
                        _roiInfo.ROIWidth = 1;
                        _roiInfo.ROIHeight = 1;

                        _activeType = typeof(Rectangle);
                    }
                    break;
                case OverlayType.LINE:
                    {
                        line = new Line();
                        rectangle = null;

                        if (_overlayContextMenu == null)
                        {
                            CreateContextMenu();
                        }
                        line.ContextMenu = _overlayContextMenu;

                        line.X1 = pt.X;
                        line.Y1 = pt.Y;
                        line.X2 = pt.X+1;
                        line.Y2 = pt.Y+1;
                        line.Stroke = Brushes.Yellow;
                        line.StrokeThickness = 1;

                        _activeOverlayIndex = canvas.Children.Add(line);

                        if (_lineProfile.IsLoaded == false)
                        {
                            _lineProfile = new LineProfile();
                            _lineProfile.Show();
                            _lineProfile.LineWidthChange += new Action<int>(LineProfile_LineWidthChange);
                            _lineProfile.Closing += new System.ComponentModel.CancelEventHandler(LineProfile_Closing);
                        }

                        _startPoint = pt;
                        LineProfile.StartPointProperty = pt;

                        _activeType = typeof(Line);
                    }
                    break;
            }
        }

        public void ObjectResize(Point currentPoint, ref Canvas canvas)
        {
            if (typeof(Rectangle) == canvas.Children[_activeOverlayIndex].GetType())
            {
                //if the dialog is closed do not update
                if (_roiInfo.IsLoaded == false)
                {
                    return;
                }

                ((Rectangle)canvas.Children[_activeOverlayIndex]).Width = Math.Abs(_startPoint.X - currentPoint.X);
                ((Rectangle)canvas.Children[_activeOverlayIndex]).Height = Math.Abs(_startPoint.Y - currentPoint.Y);

                _roiInfo.ROIWidth = Convert.ToInt32(((Rectangle)canvas.Children[_activeOverlayIndex]).Width);
                _roiInfo.ROIHeight = Convert.ToInt32(((Rectangle)canvas.Children[_activeOverlayIndex]).Height);

                Canvas.SetTop(canvas.Children[_activeOverlayIndex], Math.Min(_startPoint.Y, currentPoint.Y));
                Canvas.SetLeft(canvas.Children[_activeOverlayIndex], Math.Min(_startPoint.X, currentPoint.X));
                Canvas.SetRight(canvas.Children[_activeOverlayIndex], Math.Max(_startPoint.X, currentPoint.X));
                Canvas.SetBottom(canvas.Children[_activeOverlayIndex], Math.Max(_startPoint.Y, currentPoint.Y));

                _roiInfo.ROITop = Convert.ToInt32(Math.Min(_startPoint.Y, currentPoint.Y));
                _roiInfo.ROILeft = Convert.ToInt32(Math.Min(_startPoint.X, currentPoint.X));
            }
            else if (typeof(Line) == canvas.Children[_activeOverlayIndex].GetType())
            {
                _endPoint = currentPoint;
                LineProfile.EndPointProperty = currentPoint;

                ((Line)canvas.Children[_activeOverlayIndex]).X2 = currentPoint.X;
                ((Line)canvas.Children[_activeOverlayIndex]).Y2 = currentPoint.Y;
            }
        }

        public void RemoveAdorners(string shape)
        {
            try
            {
                if (_adornerProvided)
                {
                    if (shape.Equals("line"))
                    {
                        if (line != null)
                        {
                            Adorner lines = layer.GetAdorners(line)[0];
                            layer.Remove(lines);
                            _overlayContextMenuItem.IsChecked = false;
                        }
                        _adornerProvided = false;
                    }
                    if (shape.Equals("rectangle"))
                    {
                        if (rectangle != null)
                        {
                            Adorner rect = layer.GetAdorners(rectangle)[0];
                            layer.Remove(rect);
                        }
                        _adornerProvided = false;
                        _overlayContextMenuItem.IsChecked = false;
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "RemoveAdorners");
            }
        }

        public void UpdateStats(int width, int height, int colorChannels)
        {
            if (_activeType == typeof(Rectangle))
            {
                //if the dialog is closed do not update
                if (_roiInfo.IsLoaded == false)
                {
                    _roiInfo.Mean = 0;
                    _roiInfo.Min = 0;
                    _roiInfo.Max = 0;
                    _roiInfo.StdDev = 0;

                    return;
                }

                if ((_roiInfo.ROIWidth <= 0) ||
                    (_roiInfo.ROIHeight <= 0))

                {
                    return;
                }

                if(((_roiInfo.ROILeft + _roiInfo.ROIWidth) > width)||
                ((_roiInfo.ROITop + _roiInfo.ROIHeight) > height))
                {
                    return;
                }

                _roiInfo.Channels = colorChannels;

                for (int c = 0; c < colorChannels; c++)
                {
                    double mean = 0, stdDev = 0;
                    int minVal = 0, maxVal = 0;

                    GetImageROIRectStats(_roiInfo.ROITop,
                                     _roiInfo.ROILeft,
                                     _roiInfo.ROIWidth,
                                     _roiInfo.ROIHeight,
                                     c,
                                     ref mean,
                                     ref minVal,
                                     ref maxVal,
                                     ref stdDev);

                    switch (c)
                    {
                        case 0:
                            {
                                _roiInfo.Mean = Math.Round(mean, 3);
                                _roiInfo.Min = minVal;
                                _roiInfo.Max = maxVal;
                                _roiInfo.StdDev = Math.Round(stdDev, 3);
                                _roiInfo.MeanR = Math.Round(mean, 3);
                                _roiInfo.MinR = minVal;
                                _roiInfo.MaxR = maxVal;
                                _roiInfo.StdDevR = Math.Round(stdDev, 3);
                            }
                            break;
                        case 1:
                            {
                                _roiInfo.MeanG = Math.Round(mean, 3);
                                _roiInfo.MinG = minVal;
                                _roiInfo.MaxG = maxVal;
                                _roiInfo.StdDevG = Math.Round(stdDev, 3);
                            }
                            break;
                        case 2:
                            {
                                _roiInfo.MeanB = Math.Round(mean, 3);
                                _roiInfo.MinB = minVal;
                                _roiInfo.MaxB = maxVal;
                                _roiInfo.StdDevB = Math.Round(stdDev, 3);
                            }
                            break;
                    }
                }
            }
            else if (_activeType == typeof(Line))
            {
                int lineWidth = _lineWidth;

                _startPoint = LineProfile.StartPointProperty;
                _endPoint = LineProfile.EndPointProperty;

                int part1 = (int)((_startPoint.X - _endPoint.X) * (_startPoint.X - _endPoint.X));
                int part2 = (int)((_startPoint.Y - _endPoint.Y) * (_startPoint.Y - _endPoint.Y));
                int length = (int)Math.Sqrt(part1 + part2);
                ushort[] returnBuffer = new ushort[length * lineWidth];

                int bitdepth = 16;

                int arraylenght = GetLineProfile(width, height, bitdepth, 0, (int)_startPoint.X, (int)_startPoint.Y, (int)_endPoint.X, (int)_endPoint.Y, lineWidth, returnBuffer);

                if (arraylenght > returnBuffer.Length)
                    arraylenght = returnBuffer.Length;

          	     int dispLen = arraylenght / lineWidth;

                ushort[] newBuffer = new ushort[arraylenght];
                for (int i = 0; i < arraylenght; i++)
                {
                    newBuffer[i] = returnBuffer[i];
                }

                if (_lineProfile.IsLoaded == false)
                {
                    return;
                }

                double[] plotX = new double[dispLen];
                double[] plotY = new double[dispLen];

                double sumVal;

                for (int i = 0; i < dispLen; i++)
                {
                    sumVal = 0;
                    for (int j = 0; j < lineWidth; j++)
                    {
                        int index = i + j * (dispLen);
                        sumVal += newBuffer[index];
                    }
                    plotX[i] = i;
                    plotY[i] = sumVal / lineWidth;
                }
                _lineProfile.DataX = plotX;
                _lineProfile.DataY = plotY;
            }
        }

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetImageROIRectStats")]
        private static extern bool GetImageROIRectStats(int roiTop, int roiLeft, int roiWidth, int roiHeight, int channel, ref double meanVal, ref int minVal, ref int maxVal, ref double stdDevVal);
        
        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetLineProfile")]
        private static extern int GetLineProfile(int imageWidth, int imageHeight, int bitdepth, int channel, int point1X, int point1Y, int point2X, int point2Y, int lineWidth, [In, Out, MarshalAs(UnmanagedType.LPArray)] ushort[] resultBuffer);

        private void AddAdorners(string shape)
        {
            try
            {
                if (shape.Equals("line"))
                {
                    if (line != null)
                    {
                        layer = AdornerLayer.GetAdornerLayer(line);
                        layer.Add(new AdornerProvider(line));
                        _adornerProvided = true;
                    }
                }
                else if (shape.Equals("rectangle"))
                {
                    if (rectangle != null)
                    {
                        layer = AdornerLayer.GetAdornerLayer(rectangle);
                        layer.Add(new AdornerProvider(rectangle));
                        _adornerProvided = true;
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "AddAdorners");
            }
        }

        private void CreateContextMenu()
        {
            _overlayContextMenu = new ContextMenu();

            _overlayContextMenuItem = new MenuItem();
            _overlayContextMenuItem.Header = "Pan Overlay";
            _overlayContextMenuItem.IsCheckable = true;
            _overlayContextMenu.Items.Add(_overlayContextMenuItem);
            _overlayContextMenuItem.Checked += new RoutedEventHandler(_overlayContectMenuItem_Checked);

            MenuItem _overlayContextRectRoi = new MenuItem();
            _overlayContextRectRoi.Header = "Create Rectangle ROI";
            _overlayContextMenu.Items.Add(_overlayContextRectRoi);
            _overlayContextRectRoi.Click += new RoutedEventHandler(_overlayContextRectRoi_Click);

            MenuItem _overlayContextLineRoi = new MenuItem();
            _overlayContextLineRoi.Header = "Create Line ROI";
            _overlayContextMenu.Items.Add(_overlayContextLineRoi);
            _overlayContextLineRoi.Click += new RoutedEventHandler(_overlayContextLineRoi_Click);

            MenuItem _overlayContextSave = new MenuItem();
            _overlayContextSave.Header = "Save As";
            _overlayContextMenu.Items.Add(_overlayContextSave);
            _overlayContextSave.Click += new RoutedEventHandler(_overlayContextSave_Click);
        }

        void LineProfile_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            if (ClosingLineProfile != null)
            {
                ClosingLineProfile();

                _adornerProvided = false;
                _overlayContextMenuItem.IsChecked = false;
            }
        }

        void LineProfile_LineWidthChange(int obj)
        {
			 if (obj < 1)
                _lineWidth = 1;
            else if (obj == 1)   
                _lineWidth = obj;
            else
            {
                if (obj % 2 == 0)           // 2->3, 4->5, 6->7; Finally, _lineWidth = 1,3,5,7,...
                    _lineWidth = obj + 1;
                else
                    _lineWidth = obj;
            }          

                LineWidthChange();
            
        }

        void RoiInfo_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            if (ClosingROIStats != null)
            {
                ClosingROIStats();
                _adornerProvided = false;
                _overlayContextMenuItem.IsChecked = false;
            }
        }

        void _overlayContectMenuItem_Checked(object sender, RoutedEventArgs e)
        {
            //MessageBox.Show("Panning on : " + _overlayContectMenuItem.IsChecked.ToString());
            if (_overlayContextMenuItem.IsChecked)
            {
                if(rectangle != null)
                    AddAdorners("rectangle");

                if (line != null)
                    AddAdorners("line");
            }
        }

        void _overlayContextLineRoi_Click(object sender, RoutedEventArgs e)
        {
            ImageView.AddAnOverlayProperty = true;
            ImageView.OverlayTypeProperty = OverlayType.LINE;

            ImageView.IsLineROIProperty = true;
            ImageView.IsRectROIProperty = false;

            if (_adornerProvided)
            {
                RemoveAdorners("line");
            }
        }

        void _overlayContextRectRoi_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                ImageView.AddAnOverlayProperty = true;
                ImageView.OverlayTypeProperty = OverlayType.RECTANGLE;
                ImageView.IsLineROIProperty = false;
                ImageView.IsRectROIProperty = true;

                if (_adornerProvided)
                {
                    RemoveAdorners("rectangle");
                }
            }
            catch (Exception ex)
            {
                string str = ex.Message;
            }
        }

        void _overlayContextSave_Click(object sender, RoutedEventArgs e)
        {
            LiveImageViewModel vm = ImageView.GetLiveImageViewModelObject();

            if (vm == null)
                return;

            // Configure save file dialog box
            SaveFileDialog dlg = new SaveFileDialog();
            dlg.Title = "Save an Image File";
            dlg.FileName = string.Format("Image_{0:yyyy-MM-dd_hh-mm-ss}", DateTime.Now);

            switch (CaptureSetupDll.Model.LiveImage.GetColorChannels())
            {
                case 1:
                    {
                        dlg.Filter = "8 Bit Tiff file (*.tif)|*.tif|16 Bit Tiff file (*.tif)|*.tif|Jpeg file (*.jpg)|*.jpg";
                    }
                    break;
                case 3:
                    {
                        dlg.Filter = "24 Bit Tiff file (*.tif)|*.tif|48 Bit Tiff file (*.tif)|*.tif|Jpeg file (*.jpg)|*.jpg";
                    }
                    break;
            }

            dlg.FilterIndex = _saveDlgFilterIndex;

            // Show save file dialog box
            Nullable<bool> result = dlg.ShowDialog();

            // Process save file dialog box results
            if (result == true && dlg.FileName != "")
            {
                // Save the image file
                string filename = dlg.FileName;
                vm.SaveImage(filename, dlg.FilterIndex);
            }

            _saveDlgFilterIndex = dlg.FilterIndex;
        }

        #endregion Methods
    }
}