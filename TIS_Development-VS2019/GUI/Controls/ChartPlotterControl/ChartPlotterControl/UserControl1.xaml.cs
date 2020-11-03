/* Modification History
 * Module                           Description                                            Date
 * Optra Requirements 2011-01-04    GUI control to modify a table of data and plot        07 Jan 2011
 * Optra Requirements 2011-01-04    Changes in the array of Y- coordinate's value when    22 Feb 2011
 *                                  anchor points dragging
 */

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using Microsoft.Research.DynamicDataDisplay.DataSources;
using System.Windows.Threading;
using Microsoft.Research.DynamicDataDisplay;
using Microsoft.Research.DynamicDataDisplay.PointMarkers;
using Microsoft.Research.DynamicDataDisplay.Common;
using System.Collections.ObjectModel;

namespace ChartPlotterControl
{
    public partial class UserControl1 : UserControl
    {
        #region fields & constructor

        ObservableCollection<chartCoordinates> coordinates;
        
        EnumerableDataSource<double> xOriginalDataSource;
        EnumerableDataSource<double> yOriginalDataSource;
        CompositeDataSource _compositeDataSource;
  
        EnumerableDataSource<double> xDataSourceForAnchor;
        EnumerableDataSource<double> yDataSourceForAnchor;
        CompositeDataSource _compositeDataSourceForAnchor;

        LineAndMarker<MarkerPointsGraph> lineAndMarker = null;

        Point _ptMouseDown, _ptMouseDownDataToScreen;
        Point _ptMouseMove, _ptMouseMoveDataToScreen;
        Point _ptDataSourcePoint;

        double[] _xCoordinates;
        double[] _yCoordinates;

        double[] _anchorPointX;
        double[] _anchorPointY;

        bool _isLineGraphDrawn = false;
        bool _anchorEnable = false;
        bool _isValid = false;
        
        double _yCoordinateMax = 255;
        double _yCoordinateMin = 0;       
        int _currentAnchorIndex;

        public UserControl1()
        {
            InitializeComponent();
            _anchorPointX = new double[1];
            _anchorPointY = new double[1];
        }

        #endregion

        #region properties

        public double[] XCoordinates
        {
            get { return _xCoordinates;}
            set 
            {
                try
                {
                    _xCoordinates = value;
                    deletePreviousAnchor();
                    drawGraph();
                }
                catch (Exception ex)
                {
                    MessageBox.Show(ex.Message, "XCoordinates", MessageBoxButton.OK, MessageBoxImage.Error);
                }
            }
        }

        public double[] YCoordinates
        {
            get { return _yCoordinates; }
            set
            {
                try
                {
                    _yCoordinates = value;
                    deletePreviousAnchor();
                    drawGraph();
                }
                catch (Exception ex)
                {
                    MessageBox.Show(ex.Message, "YCoordinates", MessageBoxButton.OK, MessageBoxImage.Error);
                }
            }
        }

        public double YCoordinateMax
        {
            get { return _yCoordinateMax; }
            set
            {
                try
                {
                    _yCoordinateMax = value;
                }
                catch (Exception ex)
                {
                    MessageBox.Show(ex.Message, "yCoordinateMaxValueProperty", MessageBoxButton.OK, MessageBoxImage.Error);
                }
            }
        }

        public double YCoordinateMin
        {
            get { return _yCoordinateMin; }
            set 
            {
                try
                {
                    _yCoordinateMin = value;
                }
                catch (Exception ex)
                {
                    MessageBox.Show(ex.Message, "yCoordinateMinValueProperty", MessageBoxButton.OK, MessageBoxImage.Error);
                }
            }
        }

        public string XTitle
        {
            get
            {
                return _hAxisTitle.Content.ToString();
            }
            set
            {
                _hAxisTitle.Content = value;
            }
        }

        public string YTitle
        {
            get
            {
                return _vAxisTitle.Content.ToString();
            }
            set
            {
                _vAxisTitle.Content = value;
            }
        }

        public bool AnchorEnable
        {
            get { return _anchorEnable;}
            set { 
                _anchorEnable = value;            
            }
        }

        private int _selectedAnchorIndex = 0;
        public int SelectedAnchorIndex
        {
            get
            {
                return _selectedAnchorIndex;
            }
            set
            {
                _selectedAnchorIndex = value;

                if (_anchorEnable == true)
                {
                    if ((_xCoordinates != null) && (_yCoordinates != null))
                    {
                        _anchorPointX[0] = _xCoordinates[_selectedAnchorIndex];
                        _anchorPointY[0] = _yCoordinates[_selectedAnchorIndex];
                        drawAnchor(_selectedAnchorIndex);
                        _currentAnchorIndex = _selectedAnchorIndex;
                    }
                }
            }
        }

        #endregion

        #region event Handlers

        //private void dataList_SelectionChanged(object sender, SelectionChangedEventArgs e)
        //{
        //    try
        //    {
        //        object selectedRow = dataList.SelectedItem;                
        //        if (selectedRow == null) return;
        //        _currentAnchorIndex = (selectedRow as chartCoordinates).coordinateIndex;
        //        _anchorPointX[0] = _xCoordinates[_currentAnchorIndex];
        //        _anchorPointY[0] = _yCoordinates[_currentAnchorIndex];
        //        drawAnchor(selectedRow);
        //    }
        //    catch (Exception ex)
        //    {
        //        MessageBox.Show(ex.Message, "dataList_SelectionChanged", MessageBoxButton.OK, MessageBoxImage.Error);
        //    }
        //}
        
        //private void txtYData_TextChanged(object sender, TextChangedEventArgs e)
        //{
        //    try
        //    {
        //        ListViewItem item = dataList.ItemContainerGenerator.ContainerFromIndex(_currentAnchorIndex) as ListViewItem;
        //        if (item != null)
        //        {
        //            ContentPresenter CP = FindVisualChild<ContentPresenter>(item as DependencyObject);
        //            DataTemplate DT = CP.ContentTemplate;
        //            TextBox txtTempY = (TextBox)DT.FindName("txtYData", CP);
        //            if (txtTempY != null)
        //            {
        //                if (string.IsNullOrEmpty(txtTempY.Text))
        //                    txtTempY.Text = "0";

        //                double tempY = Convert.ToInt64(txtTempY.Text);

        //                if (tempY < _yCoordinateMin)
        //                    tempY = _yCoordinateMin;
        //                else if (tempY > _yCoordinateMax)
        //                    tempY = _yCoordinateMax;

        //                _anchorPointY[0] = tempY;                     
        //                _yCoordinates[_currentAnchorIndex] = _anchorPointY[0];                        
        //                chartCoordinates currentAnchor = new chartCoordinates()
        //                    {
        //                        coordinateIndex = _currentAnchorIndex,
        //                        xData = _anchorPointX[0],
        //                        yData = _anchorPointY[0]
        //                    };
                      
        //                drawLineGraph();
        //                drawAnchor(currentAnchor);
        //                txtTempY.Text = tempY.ToString();
        //            }
        //        }                
        //    }
        //    catch (Exception ex)
        //    {
        //        MessageBox.Show(ex.Message, "txtYData_TextChanged", MessageBoxButton.OK, MessageBoxImage.Error);
        //    }
        //}

        private void plotter_MouseDown(object sender, MouseButtonEventArgs e)
        {
            try
            {               
                if (_anchorEnable)
                {
                    if (_compositeDataSourceForAnchor != null)
                    {  
                        plotter.CaptureMouse();
                        _ptMouseDown = e.GetPosition(plotter).DataToScreen(plotter.Viewport.Transform);
                        _ptMouseDownDataToScreen = _ptMouseDown.ScreenToData(plotter.Viewport.Transform);
                        _ptDataSourcePoint = _compositeDataSourceForAnchor.GetPoints().DataToScreen(plotter.Viewport.Transform).ElementAt(0);

                        //ChartPlotter MouseDown point Bug Fixed : START

                        _isValid = validateMouseDownPoint(e.GetPosition(plotter));

                        //ChartPlotter MouseDown point Bug Fixed : END
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "plotter_MouseDown", MessageBoxButton.OK, MessageBoxImage.Error); 
            }
        }
        
        private void plotter_MouseMove(object sender, MouseEventArgs e)
        {
            try
            {          
                if (!plotter.IsMouseCaptured) return;
                {
                    //ChartPlotter MouseDown point Bug Fixed : START

                    if (!_isValid) return;

                    //ChartPlotter MouseDown point Bug Fixed : END

                    _ptMouseMove = e.GetPosition(plotter).DataToScreen(plotter.Viewport.Transform);
                    
                    _ptMouseMoveDataToScreen = _ptMouseMove.ScreenToData(plotter.Viewport.Transform);

                    double adjustmentPoint = _ptMouseMoveDataToScreen.Y - _ptMouseDownDataToScreen.Y;

                    double newYValue = _ptDataSourcePoint.Y + adjustmentPoint;
                    Point newPoint = new Point(_ptDataSourcePoint.X, newYValue);
                  
                    if (newPoint.ScreenToData(plotter.Viewport.Transform).Y < _yCoordinateMin)
                        _anchorPointY[0] = _yCoordinateMin;
                    else if (newPoint.ScreenToData(plotter.Viewport.Transform).Y > _yCoordinateMax)
                        _anchorPointY[0] = _yCoordinateMax;
                    else
                        _anchorPointY[0] = newPoint.ScreenToData(plotter.Viewport.Transform).Y;

                    _yCoordinates = ManipulateYCordinates(_anchorPointY[0], _currentAnchorIndex); 
                    drawGraph();
                   
                    chartCoordinates currentAnchor = new chartCoordinates()
                    {
                        xData = _anchorPointX[0],
                        yData = _anchorPointY[0]
                    };
                    drawAnchor(currentAnchor);
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "plotter_MouseMove", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        private void plotter_MouseUp(object sender, MouseButtonEventArgs e)
        {
            try
            {
                if (plotter.IsMouseCaptured)
                    plotter.ReleaseMouseCapture();

                //ChartPlotter MouseDown point Bug Fixed : START
                _isValid = false;
                //ChartPlotter MouseDown point Bug Fixed : END
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "plotter_MouseUp", MessageBoxButton.OK, MessageBoxImage.Error);
            }
            
        }

        #endregion

        #region Methods

        private double[] ManipulateYCordinates(double newAnchorY, int anchorIndex)
        {

            double[] newYCordinates = _yCoordinates;

            double cAnchorPoint = newAnchorY - _yCoordinates[anchorIndex];
            for (int i = 0; i < XCoordinates.Length; i++)
            {
                int a = i - anchorIndex;
                double newYCordinate = newYCordinates[i] + (cAnchorPoint / (Math.Abs(a) + 1));
                
                if (newYCordinate < _yCoordinateMin)
                    newYCordinates[i] = _yCoordinateMin;
                else if (newYCordinate > _yCoordinateMax)
                    newYCordinates[i] = _yCoordinateMax;
                else
                    newYCordinates[i] = newYCordinate;
            }
            return newYCordinates;
        }

        public ObservableCollection<chartCoordinates> getCollection()
        {
            return coordinates;
        }

        public Point getAnchor()
        {
            if (_anchorPointX != null && _anchorPointY != null)
            {
                return new Point(_anchorPointX[0], _anchorPointY[0]);
            }
            else
                return new Point(0, 0);
        }

        public void drawGraph()
        {
            bool isSet = setXYCoordinateData();
            if (isSet)
            {
                drawLineGraph();
            }
        }

        private void drawLineGraph()
        {
            try
            {
                if (_isLineGraphDrawn == true)
                {
                    IPlotterElement plotterItem1 = (IPlotterElement)plotter.Children.OfType<LineGraph>().Where(x => (x.Description.Brief == "LineGraph")).Single();
                    plotter.Children.Remove(plotterItem1);                   
                }

                xOriginalDataSource = new EnumerableDataSource<double>(_xCoordinates);
                xOriginalDataSource.SetXMapping(x => x);

                yOriginalDataSource = new EnumerableDataSource<double>(_yCoordinates);
                yOriginalDataSource.SetYMapping(y => y);

                _compositeDataSource = new CompositeDataSource(xOriginalDataSource, yOriginalDataSource);
                plotter.AddLineGraph(_compositeDataSource, Color.FromRgb(0, 255, 0), 2);

                _isLineGraphDrawn = true;
                plotter.Legend.Remove();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "drawLineGraph", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        private bool setXYCoordinateData()
        {
            try
            {
                if (_xCoordinates == null || _yCoordinates == null) return false;

                if (_xCoordinates.Length != _yCoordinates.Length)
                {
                    MessageBox.Show("Unequal array length");
                    return false;
                }

               coordinates = new ObservableCollection<chartCoordinates>();

               validateXYData();

               for (int i = 0; i < _xCoordinates.Length; i++)
               {
                    chartCoordinates chartData = new chartCoordinates()
                    {
                        coordinateIndex = i,
                        xData = _xCoordinates[i],
                        yData = _yCoordinates[i]
                    };

                    coordinates.Add(chartData);
                }
                this.DataContext = coordinates;
                return true;
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "setXYCoordinateData", MessageBoxButton.OK, MessageBoxImage.Error);
                return false;
            }
        }

        private void validateXYData()
        {
            bool isValid = false;
            string index = string.Empty;

            for (int i = 0; i < _yCoordinates.Length; i++)
            {
                if (_yCoordinates[i] > _yCoordinateMax)
                {
                    _yCoordinates[i] = _yCoordinateMax;
                    isValid = true;
                    index = index + (i + 1).ToString() + ",";
                }
                if (_yCoordinates[i] < _yCoordinateMin)
                {
                    _yCoordinates[i] = _yCoordinateMin;
                    isValid = true;
                    index = index + (i + 1).ToString() + ",";
                }
            }

            if (isValid)
            {
                MessageBox.Show("Invalid Y- coordinate data modified at index : " + index, "Validate Y-Coordinate Data");
            }
            isValid = false;              
        }

        private void drawAnchor(object selectedRow)
        {
            try
            {
                if (lineAndMarker != null) deletePreviousAnchor();
                xDataSourceForAnchor = new EnumerableDataSource<double>(_anchorPointX);
                xDataSourceForAnchor.SetXMapping(x => x);

                yDataSourceForAnchor = new EnumerableDataSource<double>(_anchorPointY);
                yDataSourceForAnchor.SetYMapping(y => y);

                _compositeDataSourceForAnchor = new CompositeDataSource(xDataSourceForAnchor, yDataSourceForAnchor);

                lineAndMarker = plotter.AddLineGraph(_compositeDataSourceForAnchor,
                                                      new Pen(Brushes.Transparent, 3),
                                                      new CirclePointMarker { Size = 10, Fill = Brushes.Red },
                                                      new PenDescription("NewPen"));


                Point p = new Point(_anchorPointX[0], _anchorPointY[0]);

                Point p1 = p.DataToScreen(plotter.Viewport.Transform);
                Point p3 = p.ScreenToData(plotter.Viewport.Transform);

                Point p4 = p1.ScreenToData(plotter.Viewport.Transform);
                Point p5 = p1.DataToScreen(plotter.Viewport.Transform);

                Point p6 = p3.DataToScreen(plotter.Viewport.Transform);
                Point p7 = p3.ScreenToData(plotter.Viewport.Transform);



            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "drawAnchor", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        private void deletePreviousAnchor()
        {
            try
            {
                if (lineAndMarker != null)
                {
                    IPlotterElement plotterItem = (IPlotterElement)plotter.Children.OfType<MarkerPointsGraph>().Where(x => (x.Description.Brief == "MarkerPointsGraph")).Single();
                    plotter.Children.Remove(plotterItem);
                    lineAndMarker = null;
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "deletePreviousAnchor", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        //ChartPlotter MouseDown point Bug Fixed : START

        private bool validateMouseDownPoint(Point p)
        {
            try
            {
                bool valid = false;
                Point temp4 = p.ScreenToViewport(plotter.Viewport.Transform);
                int currentAnchorY = Convert.ToInt32(_anchorPointY[0]);

                if (currentAnchorY < temp4.Y)
                {
                    if (temp4.Y - currentAnchorY <= 1)
                    {
                        valid = true; return valid;
                    }
                }

                if (currentAnchorY > temp4.Y)
                {
                    if (currentAnchorY - temp4.Y <= 1)
                    {
                        valid = true; return valid;
                    }
                }
                return valid;

            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "ValidateMouseDownPoint", MessageBoxButton.OK, MessageBoxImage.Error);
                return false;
            }
        }

        //ChartPlotter MouseDown point Bug Fixed : END

        private childItem FindVisualChild<childItem>(DependencyObject obj)
        where childItem : DependencyObject
        {
            try
            {
                for (int i = 0; i < VisualTreeHelper.GetChildrenCount(obj); i++)
                {
                    DependencyObject child = VisualTreeHelper.GetChild(obj, i);
                    if (child != null && child is childItem)
                        return (childItem)child;
                    else
                    {
                        childItem childOfChild = FindVisualChild<childItem>(child);
                        if (childOfChild != null)
                            return childOfChild;
                    }
                }
                return null;
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "FindVisualChild", MessageBoxButton.OK, MessageBoxImage.Error);
                return null;
            }
        }

        #endregion   

    }  
}
