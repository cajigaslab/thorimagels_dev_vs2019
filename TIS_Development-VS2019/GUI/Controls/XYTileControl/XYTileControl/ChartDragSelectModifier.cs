namespace XYTileControl.CustomModifiers
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Diagnostics;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Shapes;
    using System.Xml;

    using Abt.Controls.SciChart.ChartModifiers;
    using Abt.Controls.SciChart.Model.DataSeries;
    using Abt.Controls.SciChart.Utility;
    using Abt.Controls.SciChart.Visuals;
    using Abt.Controls.SciChart.Visuals.Annotations;

    class ChartDragSelectModifier : ChartModifierBase
    {
        #region Fields

        public static DependencyProperty DuplicateEnabledProperty = 
        DependencyProperty.RegisterAttached("DuplicateEnabled",
        typeof(bool),
        typeof(ChartDragSelectModifier),
        null);
        public static DependencyProperty ScanAreaHeightProperty = 
        DependencyProperty.RegisterAttached("ScanAreaHeight",
        typeof(double),
        typeof(ChartDragSelectModifier),
        null);
        public static DependencyProperty ScanAreaWidthProperty = 
        DependencyProperty.RegisterAttached("ScanAreaWidth",
        typeof(double),
        typeof(ChartDragSelectModifier),
        null);
        public static DependencyProperty SelectEnabledProperty = 
        DependencyProperty.RegisterAttached("SelectEnabled",
        typeof(bool),
        typeof(ChartDragSelectModifier),
        null);
        public static DependencyProperty TileEnabledProperty = 
        DependencyProperty.RegisterAttached("TileEnabled",
        typeof(bool),
        typeof(ChartDragSelectModifier),
        null);
        public static DependencyProperty TileOverlapXProperty = 
        DependencyProperty.RegisterAttached("TileOverlapX",
        typeof(double),
        typeof(ChartDragSelectModifier),
        null);
        public static DependencyProperty TileOverlapYProperty = 
        DependencyProperty.RegisterAttached("TileOverlapY",
        typeof(double),
        typeof(ChartDragSelectModifier),
        null);

        private bool _drawSquareEnabled = false;
        private Rectangle _duplicatedRectangle;
        private Point _endPoint;
        private bool _isDragging = false;
        Point _point = new Point(0,0);
        private Rectangle _rectangle;
        private Border _sizeAnnotation;
        private Point _startPoint;

        #endregion Fields

        #region Events

        public event Action LeavePanelEvent;

        public event Action<Point, Point> SelectAreaEvent;

        #endregion Events

        #region Properties

        public bool DuplicateEnabled
        {
            get { return (bool)GetValue(DuplicateEnabledProperty); }
            set { SetValue(DuplicateEnabledProperty, value); }
        }

        public double ScanAreaHeight
        {
            get { return (double)GetValue(ScanAreaHeightProperty); }
            set { SetValue(ScanAreaHeightProperty, value); }
        }

        public double ScanAreaWidth
        {
            get { return (double)GetValue(ScanAreaWidthProperty); }
            set { SetValue(ScanAreaWidthProperty, value); }
        }

        public bool SelectEnabled
        {
            get { return (bool)GetValue(SelectEnabledProperty); }
            set { SetValue(SelectEnabledProperty, value); }
        }

        public bool TileEnabled
        {
            get { return (bool)GetValue(TileEnabledProperty); }
            set { SetValue(TileEnabledProperty, value); }
        }

        public double TileOverlapX
        {
            get { return (double)GetValue(TileOverlapXProperty); }
            set { SetValue(TileOverlapXProperty, value); }
        }

        public double TileOverlapY
        {
            get { return (double)GetValue(TileOverlapYProperty); }
            set { SetValue(TileOverlapYProperty, value); }
        }

        #endregion Properties

        #region Methods

        public void CreateDuplicatedArea(double width, double height)
        {
            _duplicatedRectangle = new Rectangle()
            {
                Fill = Brushes.RoyalBlue,
                Stroke = Brushes.Ivory,
                StrokeThickness = 2,
                Opacity = 0.8,
                Tag = new Size(width,height)
            };
        }

        public Point GetMousePosition()
        {
            return _point;
        }

        public override void OnAttached()
        {
            base.OnAttached();
            var scichart = (ParentSurface as SciChartSurface);
            var mainWindow = FindLogicalParent<UserControl>(scichart);
            mainWindow.PreviewKeyDown -=
            new KeyEventHandler(DragSelect_PreviewKeyDown);
            mainWindow.PreviewKeyDown +=
            new KeyEventHandler(DragSelect_PreviewKeyDown);
            mainWindow.KeyUp -=
            new KeyEventHandler(DragSelect_KeyUp);
            mainWindow.KeyUp +=
            new KeyEventHandler(DragSelect_KeyUp);
        }

        //public override void OnAttached()
        //{
        //    base.OnAttached();
        //}
        public override void OnMasterMouseLeave(ModifierMouseArgs e)
        {
            base.OnMasterMouseLeave(e);
            Mouse.OverrideCursor = null;
            SelectEnabled = false;
            TileEnabled = false;
            if (LeavePanelEvent != null)
            {
                LeavePanelEvent();
            }
            if (_isDragging)
            {
                ClearReticule();
                _isDragging = false;
            }
            e.Handled = true;
        }

        public override void OnModifierMouseDown(ModifierMouseArgs e)
        {
            base.OnModifierMouseDown(e);

            if (e.MouseButtons == MouseButtons.Right)
            {
                return;
            }

            if (DuplicateEnabled == true)
            {
                if (e.MouseButtons == MouseButtons.Left)
                {
                    var point = GetPointRelativeTo(e.MousePoint, ModifierSurface);
                    var xCalc = this.XAxis.GetCurrentCoordinateCalculator();
                    var yCalc = this.YAxis.GetCurrentCoordinateCalculator();
                    PerformSelection(new Point(xCalc.GetDataValue(point.X), yCalc.GetDataValue(point.Y)), new Point(xCalc.GetDataValue(point.X) + ((Size)_duplicatedRectangle.Tag).Width, yCalc.GetDataValue(point.Y) - ((Size)_duplicatedRectangle.Tag).Height));
                }
                DuplicateEnabled = false;
                ClearDuplicatedAreaPosition();
            }

            // if select mode is not enabled, return
            if (SelectEnabled == false && TileEnabled == false)
            {
                return;
            }

            // Check the mouse point was inside the ModifierSurface (the central chart area). If not, exit
            var modifierSurfaceBounds = ModifierSurface.GetBoundsRelativeTo(RootGrid);
            if (!modifierSurfaceBounds.Contains(e.MousePoint))
            {
                return;
            }

            // Translate the mouse point (which is in RootGrid coordiantes) relative to the ModifierSurface
            // This accounts for any offset due to left Y-Axis
            var ptTrans = GetPointRelativeTo(e.MousePoint, ModifierSurface);
            _startPoint = ptTrans;
            _rectangle = new Rectangle
            {
                Stroke = Brushes.LightBlue,
                StrokeThickness = 2,
                StrokeDashArray = new DoubleCollection() { 2 }
            };
            if (TileEnabled == true)
            {
                _sizeAnnotation = new Border()
                {
                    IsHitTestVisible = false,
                    CornerRadius = new CornerRadius(2, 2, 2, 2),
                    Tag = typeof(ChartDragSelectModifier),
                    Width = 100,
                    Background = Brushes.Ivory,
                    Child = new TextBlock()
                    {
                        Text = string.Format("Row: {0}, Col: {1}", "1", "1"),
                        FontSize = 12,
                        Margin = new Thickness(3),
                        Foreground = Brushes.DimGray,
                    }
                };
                if (ptTrans.X + 100 > ModifierSurface.ActualWidth)
                {
                    Canvas.SetLeft(_sizeAnnotation, ptTrans.X - 100);
                }
                else
                {
                    Canvas.SetLeft(_sizeAnnotation, ptTrans.X);
                }
                Canvas.SetTop(_sizeAnnotation, ptTrans.Y);
                ModifierSurface.Children.Add(_sizeAnnotation);
            }
            // Update the zoom recticule position
            SetReticulePosition(_rectangle, _startPoint, _startPoint);
            // Add the zoom reticule to the ModifierSurface - a canvas over the chart
            ModifierSurface.Children.Add(_rectangle);
            // Set flag that a drag has begun
            _isDragging = true;
            e.Handled = true;
        }

        public override void OnModifierMouseMove(ModifierMouseArgs e)
        {
            if (SelectEnabled == true)
            {
                if (this.Cursor != Cursors.Wait)
                {
                    Mouse.OverrideCursor = Cursors.Cross;
                }
            }
            else if (TileEnabled == true)
            {
                if (this.Cursor != Cursors.Wait)
                {
                    Mouse.OverrideCursor = Cursors.Pen;
                }
            }
            else
            {
                Mouse.OverrideCursor = null;
            }

            base.OnModifierMouseMove(e);
            e.Handled = true;

            // Translate the mouse point (which is in RootGrid coordiantes) relative to the ModifierSurface
            // This accounts for any offset due to left Y-Axis
            Point ptTrans = GetPointRelativeTo(e.MousePoint, ModifierSurface);
            var xCalc = this.XAxis.GetCurrentCoordinateCalculator();
            var yCalc = this.YAxis.GetCurrentCoordinateCalculator();
            _point.X = xCalc.GetDataValue(ptTrans.X);
            _point.Y = yCalc.GetDataValue(ptTrans.Y);

            if (DuplicateEnabled == true)
            {
                if (!ModifierSurface.Children.Contains(_duplicatedRectangle))
                {
                    ModifierSurface.Children.Add(_duplicatedRectangle);
                }
                SetDuplicatedAreaPosition(ptTrans);
                return;
            }
            if (!_isDragging)
                return;

            double deltaX = (xCalc.GetDataValue(ptTrans.X) - xCalc.GetDataValue(_startPoint.X));
            double deltaY = (yCalc.GetDataValue(ptTrans.Y) - yCalc.GetDataValue(_startPoint.Y));

            // Update the zoom recticule position
            if (_drawSquareEnabled == true && TileEnabled == true)
            {
                double delta = Math.Min(Math.Abs(deltaX), Math.Abs(deltaY));
                if (deltaX < 0)
                {
                   ptTrans.X = xCalc.GetCoordinate(xCalc.GetDataValue(_startPoint.X) - delta);
                }
                else
                {
                    ptTrans.X = xCalc.GetCoordinate(xCalc.GetDataValue(_startPoint.X) + delta);
                }

                if (deltaY < 0)
                {
                    ptTrans.Y = yCalc.GetCoordinate(yCalc.GetDataValue(_startPoint.Y) - delta);
                }
                else
                {
                    ptTrans.Y = yCalc.GetCoordinate(yCalc.GetDataValue(_startPoint.Y) + delta);
                }
            }

            SetReticulePosition(_rectangle, _startPoint, ptTrans);

            if (TileEnabled == true)
            {
                //Updata Row Col Information
                deltaX = Math.Abs(xCalc.GetDataValue(ptTrans.X) - xCalc.GetDataValue(_startPoint.X));
                deltaY = Math.Abs(yCalc.GetDataValue(ptTrans.Y) - yCalc.GetDataValue(_startPoint.Y));

                double xSpace = TileOverlapX/100 ;
                double ySpace = TileOverlapY/100 ;
                if (deltaX <  ScanAreaWidth )
                {
                    deltaX = ScanAreaWidth ;
                }
                if (deltaY < ScanAreaHeight )
                {
                    deltaY = ScanAreaHeight ;
                }
                int col, row;

                if (xSpace >= 1)
                {
                    col = 1;
                }
                else
                {
                    col = Convert.ToInt32(Math.Ceiling((deltaX / ScanAreaWidth - xSpace) / (1 - xSpace)));
                }

                if (ySpace >= 1)
                {
                    row = 1;
                }
                else
                {
                    row = Convert.ToInt32(Math.Ceiling((deltaY / ScanAreaHeight - ySpace) / (1 - ySpace)));
                }

                (_sizeAnnotation.Child as TextBlock).Text = string.Format("Row: {0}, Col: {1}", row.ToString(), col.ToString());

                if (ptTrans.X + 100 > ModifierSurface.ActualWidth)
                {
                    Canvas.SetLeft(_sizeAnnotation, ptTrans.X - 100);
                }
                else
                {
                    Canvas.SetLeft(_sizeAnnotation, ptTrans.X);
                }
                Canvas.SetTop(_sizeAnnotation, ptTrans.Y);
            }
        }

        public override void OnModifierMouseUp(ModifierMouseArgs e)
        {
            if (!_isDragging)
                return;

            base.OnModifierMouseUp(e);

            // Translate the mouse point (which is in RootGrid coordiantes) relative to the ModifierSurface
            // This accounts for any offset due to left Y-Axis
            var ptTrans = GetPointRelativeTo(e.MousePoint, ModifierSurface);

            var xCalc = this.XAxis.GetCurrentCoordinateCalculator();
            var yCalc = this.YAxis.GetCurrentCoordinateCalculator();
            double deltaX = (xCalc.GetDataValue(ptTrans.X) - xCalc.GetDataValue(_startPoint.X));
            double deltaY = (yCalc.GetDataValue(ptTrans.Y) - yCalc.GetDataValue(_startPoint.Y));

            // Update the zoom recticule position
            if (_drawSquareEnabled == true && TileEnabled == true)
            {
                double delta = Math.Min(Math.Abs(deltaX), Math.Abs(deltaY));
                if (deltaX < 0)
                {
                    ptTrans.X = xCalc.GetCoordinate(xCalc.GetDataValue(_startPoint.X) - delta);
                }
                else
                {
                    ptTrans.X = xCalc.GetCoordinate(xCalc.GetDataValue(_startPoint.X) + delta);
                }

                if (deltaY < 0)
                {
                    ptTrans.Y = yCalc.GetCoordinate(yCalc.GetDataValue(_startPoint.Y) - delta);
                }
                else
                {
                    ptTrans.Y = yCalc.GetCoordinate(yCalc.GetDataValue(_startPoint.Y) + delta);
                }
            }

            _endPoint = SetReticulePosition(_rectangle, _startPoint, ptTrans);

            //double distanceDragged = PointUtil.Distance(_startPoint, ptTrans);
            //if (distanceDragged > 10)
            // {

            PerformSelection(new Point(xCalc.GetDataValue(_startPoint.X), yCalc.GetDataValue(_startPoint.Y)), new Point(xCalc.GetDataValue(_endPoint.X), yCalc.GetDataValue(_endPoint.Y)));
            e.Handled = true;
            // }

            ClearReticule();
            _isDragging = false;
        }

        private static Point ClipToBoundaries(Rect rect, Point point)
        {
            double rightEdge = rect.Right;
            double leftEdge = rect.Left;
            double topEdge = rect.Top;
            double bottomEdge = rect.Bottom;

            point.X = point.X > rightEdge ? rightEdge : point.X;
            point.X = point.X < leftEdge ? leftEdge : point.X;
            point.Y = point.Y > bottomEdge ? bottomEdge : point.Y;
            point.Y = point.Y < topEdge ? topEdge : point.Y;

            return point;
        }

        private void ClearDuplicatedAreaPosition()
        {
            if (_duplicatedRectangle != null)
            {
                ModifierSurface.Children.Remove(_duplicatedRectangle);
            }
        }

        private void ClearReticule()
        {
            if (ModifierSurface != null && _rectangle != null)
            {
                ModifierSurface.Children.Remove(_rectangle);
                _rectangle = null;
                _isDragging = false;
            }
            if ( _sizeAnnotation != null)
            {
                ModifierSurface.Children.Remove(_sizeAnnotation);
            }
        }

        void DragSelect_KeyUp(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.LeftShift || e.Key == Key.RightShift)
            {
                _drawSquareEnabled = false;
            }
        }

        void DragSelect_PreviewKeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.LeftShift || e.Key == Key.RightShift)
            {
                _drawSquareEnabled = true;
            }
        }

        private T FindLogicalParent<T>(SciChartSurface scichart)
            where T : class
        {
            var parent = (FrameworkElement)scichart.Parent;
            while (parent != null)
            {
                var candidate = parent as T;
                if (candidate != null) return candidate;

                parent = (FrameworkElement)parent.Parent;
            }

            return null;
        }

        private void PerformSelection(Point startPoint, Point endPoint)
        {
            if (SelectAreaEvent!=null)
            {
                SelectAreaEvent(startPoint, endPoint);
            }
        }

        private void SetDuplicatedAreaPosition(Point point)
        {
            var xCalc = this.XAxis.GetCurrentCoordinateCalculator();
            var yCalc = this.YAxis.GetCurrentCoordinateCalculator();

            double a = xCalc.GetDataValue(point.X) ;
            double b = yCalc.GetDataValue(point.Y);
            double x = xCalc.GetCoordinate(xCalc.GetDataValue(point.X) + ((Size)_duplicatedRectangle.Tag).Width);
            double y = yCalc.GetCoordinate(yCalc.GetDataValue(point.Y) - ((Size)_duplicatedRectangle.Tag).Height);
            var rect = new Rect(point,new Point(x,y));
            Canvas.SetLeft(_duplicatedRectangle, rect.X);
            Canvas.SetTop(_duplicatedRectangle, rect.Y);
            _duplicatedRectangle.Width = rect.Width;
            _duplicatedRectangle.Height = rect.Height;
        }

        private Point SetReticulePosition(Rectangle rectangle, Point startPoint, Point endPoint)
        {
            var modifierRect = new Rect(0, 0, ModifierSurface.ActualWidth, ModifierSurface.ActualHeight);
            endPoint = ClipToBoundaries(modifierRect, endPoint);

            var rect = new Rect(startPoint, endPoint);
            Canvas.SetLeft(rectangle, rect.X);
            Canvas.SetTop(rectangle, rect.Y);

            //Debug.WriteLine("SetRect... x={0}, y={1}, w={2}, h={3}, IsMaster? {4}", rect.X, rect.Y, rect.Width, rect.Height, isMaster);

            rectangle.Width = rect.Width;
            rectangle.Height = rect.Height;

            return endPoint;
        }

        #endregion Methods
    }
}