namespace RealTimeLineChart.CustomModifiers
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Shapes;
    using RealTimeLineChart.Model;
    using RealTimeLineChart.ViewModel;

    using SciChart;
    using SciChart.Charting;
    using SciChart.Charting.ChartModifiers;
    using SciChart.Charting.Common.Extensions;
    using SciChart.Charting.Model.DataSeries;
    using SciChart.Charting.Themes;
    using SciChart.Charting.Utility;
    using SciChart.Charting.Visuals;
    using SciChart.Charting.Visuals.Annotations;
    using SciChart.Charting.Visuals.Axes;
    using SciChart.Charting.Visuals.Axes.LabelProviders;
    using SciChart.Charting.Visuals.Events;
    using SciChart.Core;
    using SciChart.Core.Utility.Mouse;
    using SciChart.Drawing.HighSpeedRasterizer;
    using SciChart.Drawing.Utility;

    /// <summary>
    /// Implementation of RolloverModifier 
    ///  - Expose styles for the line, rollover points and allow them to be customized in XAML where declare
    /// </summary>
    public class ChartCanvasModifier : ChartModifierBase
    {


        #region Fields
        

        public static readonly DependencyProperty CursorBottomProperty = DependencyProperty.Register("CursorBottom",
            typeof(double),
            typeof(ChartCanvasModifier),
            new FrameworkPropertyMetadata(0.0, FrameworkPropertyMetadataOptions.BindsTwoWayByDefault, new PropertyChangedCallback(OnCursorBottomChanged)));
        public static readonly DependencyProperty CursorLeftProperty = DependencyProperty.Register("CursorLeft",
            typeof(double),
            typeof(ChartCanvasModifier),
            new FrameworkPropertyMetadata(0.0, FrameworkPropertyMetadataOptions.BindsTwoWayByDefault, new PropertyChangedCallback(OnCursorLeftChanged)));
        public static readonly DependencyProperty CursorRightProperty = DependencyProperty.Register("CursorRight",
            typeof(double),
            typeof(ChartCanvasModifier),
            new FrameworkPropertyMetadata(0.0, FrameworkPropertyMetadataOptions.BindsTwoWayByDefault, new PropertyChangedCallback(OnCursorRightChanged)));
        public static readonly DependencyProperty CursorSelectedIndexProperty = DependencyProperty.Register("CursorSelectedIndex", typeof(int), typeof(ChartCanvasModifier), new PropertyMetadata(0));
        public static readonly DependencyProperty CursorTopProperty = DependencyProperty.Register("CursorTop",
            typeof(double),
            typeof(ChartCanvasModifier),
            new FrameworkPropertyMetadata(0.0, FrameworkPropertyMetadataOptions.BindsTwoWayByDefault, new PropertyChangedCallback(OnCursorTopChanged)));
        public static readonly DependencyProperty IsDragToScaleProperty = DependencyProperty.Register("IsDragToScale", typeof(bool), typeof(ChartCanvasModifier), new PropertyMetadata(true));
        public static readonly DependencyProperty IsRollOverProperty = DependencyProperty.Register("IsRollOver", typeof(bool), typeof(ChartCanvasModifier), new PropertyMetadata(false));
        public static readonly DependencyProperty LineBrushProperty = DependencyProperty.Register("LineBrush", typeof(Brush), typeof(ChartCanvasModifier), new PropertyMetadata(new SolidColorBrush(Colors.Gray), OnLineBrushChanged));
        public static readonly DependencyProperty MeasureBorderVisibleProperty = DependencyProperty.Register("MeasureBorderVisible", typeof(bool), typeof(ChartCanvasModifier), new PropertyMetadata(true));
        public static readonly DependencyProperty MeasureCursorProperty = DependencyProperty.Register("MeasureCursor", typeof(string), typeof(ChartCanvasModifier), new PropertyMetadata("MEASURECURSOR_ALL"));
        public static readonly DependencyProperty MeasureCursorVisibleProperty = DependencyProperty.Register("MeasureCursorVisible",
            typeof(bool),
            typeof(ChartCanvasModifier),
            new FrameworkPropertyMetadata(false, FrameworkPropertyMetadataOptions.BindsTwoWayByDefault, new PropertyChangedCallback(OnMeasureCursorVisibleChanged)));
        public static readonly DependencyProperty TextForegroundProperty = DependencyProperty.Register("TextForeground", typeof(Brush), typeof(ChartCanvasModifier), new PropertyMetadata(new SolidColorBrush(Colors.White)));

        public static readonly DependencyProperty FrameCursorXProperty = DependencyProperty.Register("FrameCursorX",
            typeof(double),
            typeof(ChartCanvasModifier),
            new FrameworkPropertyMetadata(0.0, FrameworkPropertyMetadataOptions.BindsTwoWayByDefault, new PropertyChangedCallback(OnFrameCursorChanged)));
        public static readonly DependencyProperty FrameCursorVisibleProperty = DependencyProperty.Register("FrameCursorVisible",
            typeof(bool),
            typeof(ChartCanvasModifier),
            new FrameworkPropertyMetadata(false, FrameworkPropertyMetadataOptions.BindsTwoWayByDefault, new PropertyChangedCallback(OnFrameCursorVisibleChanged)));
        public static readonly DependencyProperty FrameTextForegroundProperty = DependencyProperty.Register("FrameTextForeground", typeof(Brush), typeof(ChartCanvasModifier), new PropertyMetadata(new SolidColorBrush(Colors.White)));
        
        public static readonly DependencyProperty FrameBorderVisibleProperty = DependencyProperty.Register("FrameBorderVisible", typeof(bool), typeof(ChartCanvasModifier), new PropertyMetadata(true));
        public static readonly DependencyProperty FrameCursorProperty = DependencyProperty.Register("FrameCursor", typeof(string), typeof(ChartCanvasModifier), new PropertyMetadata("FRAME_CURSOR"));

        public static Dictionary<string, MeasureCursorType> MeasureCursorTypeDictionary = new Dictionary<string, MeasureCursorType>()
        {
        {"MEASURECURSOR_ALL", MeasureCursorType.All},
        {"MEASURECURSOR_X_ONLY", MeasureCursorType.X_ONLY},
        {"MEASURECURSOR_Y_ONLY", MeasureCursorType.Y_ONLY}
        };
        public static Action<int, double> UpdateCursorTBLR;
        public static Action<bool> UpdateMeasureCursor;

        public static Action<int> UpdateFrameCursorXAction;

        public static Action<double> UpdateFrameCursorX;
        public static Action<bool> UpdateFrameCursor;

        private string[] cursorBorder = new string[4] { "cursorBorder_top", "cursorBorder_bottom", "cursorBorder_left", "cursorBorder_right" };
        private string[] _cursorName = new string[4] { "cursor_top", "cursor_bottom", "cursor_left", "cursor_right" };
        
        private Line _line;
        private Border[] _measurementBorders = new Border[4];
        private Line[] _measurementCursors = null;

        private string FrameBorder = "Frame";
        private string _frameName =  "Frame_Name" ;

        private bool _currentlyDragging = false;

        private static int _averaging;
        private Line _frameLine;
        private Border _frameBorders = new Border();// = new Border();

        private int currentFrameNumber = -1;


        #endregion Fields

        #region Constructors

        /// <summary>
        /// Initializes a new instance of the <see cref="ChartCanvasModifier"/> class.
        /// </summary>
        public ChartCanvasModifier()
        {
            this._line = new Line() { Stroke = this.LineBrush, StrokeThickness = 4, IsHitTestVisible = false, Tag = typeof(ChartCanvasModifier) };
            UpdateMeasureCursor += UpdateMeasureCursorStatus;
            UpdateCursorTBLR += RedrawCursorTBLR;

            UpdateFrameCursor += UpdateFrameCursorStatus;
            UpdateFrameCursorX += RedrawFrameCursor;
            //UPdate Frame Cursor
            UpdateFrameCursorXAction += DisplayFrameLine;

            _averaging = 1;
        }

        #endregion Constructors

        #region Enumerations

        public enum MeasureCursorType
        {
            All,
            X_ONLY,
            Y_ONLY
        }

        #endregion Enumerations

        #region Properties

        public static int Average
        {
            get
            {
                return _averaging;
            }
            set
            {
                _averaging = value; //OnPropertyChanged("Averaging");
            }
        }

        public int CurrentFrameNum
        {
            get
            {
                return currentFrameNumber;
            }
            set
            {
                currentFrameNumber = value;
            }
        }

        public double CursorFrameX
        {
            get { return (double)GetValue(FrameCursorXProperty); }
            set { SetValue(FrameCursorXProperty, value); }
        }

        public double CursorBottom
        {
            get { return (double)GetValue(CursorBottomProperty); }
            set { SetValue(CursorBottomProperty, value); }
        }

        public double CursorLeft
        {
            get { return (double)GetValue(CursorLeftProperty); }
            set { SetValue(CursorLeftProperty, value); }
        }

        public double CursorRight
        {
            get { return (double)GetValue(CursorRightProperty); }
            set { SetValue(CursorRightProperty, value); }
        }

        public int CursorSelectedIndex
        {
            get { return (int)GetValue(CursorSelectedIndexProperty); }
            set { SetValue(CursorSelectedIndexProperty, value); }
        }

        public double CursorTop
        {
            get { return (double)GetValue(CursorTopProperty); }
            set { SetValue(CursorTopProperty, value); }
        }

        public bool IsDragToScale
        {
            get { return (bool)GetValue(IsDragToScaleProperty); }
            set { SetValue(IsDragToScaleProperty, value); }
        }

        public bool IsRollOver
        {
            get { return (bool)GetValue(IsRollOverProperty); }
            set { SetValue(IsRollOverProperty, value); }
        }

        /// <summary>
        /// Gets or sets the line brush.
        /// </summary>
        /// <value>
        /// The line brush.
        /// </value>
        public Brush LineBrush
        {
            get { return (Brush)GetValue(LineBrushProperty); }
            set { SetValue(LineBrushProperty, value); }
        }

        public bool MeasureBorderVisible
        {
            get { return (bool)GetValue(MeasureBorderVisibleProperty); }
            set { SetValue(MeasureBorderVisibleProperty, value);
                SetValue(FrameBorderVisibleProperty, value);
            }
        }

        public string MeasureCursor
        {
            get { return (string)GetValue(MeasureCursorProperty); }
            set { SetValue(MeasureCursorProperty, value); }
        }

        public bool MeasureCursorVisible
        {
            get { return (bool)GetValue(MeasureCursorVisibleProperty); }
            set {
                SetValue(MeasureCursorVisibleProperty, value);
                SetValue(FrameCursorVisibleProperty, value);
            }
        }
        /// <summary>
        /// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// </summary>
        /// 
        public bool FrameBorderVisible
        {
            get { return (bool)GetValue(FrameBorderVisibleProperty); }
            set { SetValue(FrameBorderVisibleProperty, value); }
        }

        public string FrameCursor
        {
            get { return (string)GetValue(FrameCursorProperty); }
            set { SetValue(FrameCursorProperty, value); }
        }

        public bool FrameCursorVisible
        {
            get { return (bool)GetValue(FrameCursorVisibleProperty); }
            set { SetValue(FrameCursorVisibleProperty, value); }
        }

        public double FrameCursorX
        {
            get { return (double)GetValue(FrameCursorXProperty); }
            set { SetValue(FrameCursorXProperty, value); }
        }

        /// <summary>
        /// Gets or sets the text foreground.
        /// </summary>
        /// <value>
        /// The text foreground.
        /// </value>
        public Brush TextForeground
        {
            get { return (Brush)GetValue(TextForegroundProperty); }
            set { SetValue(TextForegroundProperty, value); }
        }
        #endregion Properties

        #region Methods
        /// <summary>
        /// Adds the axislabel.
        /// </summary>
        /// <param name="index">The index.</param>
        public void AddAxislabel(int index)
        {
            if (Visibility.Visible != _measurementBorders[index].Visibility)
                return;

            if (index < 2)
            {
                AxisCanvas.SetTop(_measurementBorders[index], _measurementCursors[index].Y1 - 15);
                AxisCanvas.SetLeft(_measurementBorders[index], -2);
            }
            else
            {
                AxisCanvas.SetLeft(_measurementBorders[index], _measurementCursors[index].X1 - 35);
                AxisCanvas.SetTop(_measurementBorders[index], -2);
            }
        }

        /// <summary>
        /// Displays the cursor line.
        /// </summary>
        public void DisplayCursorLine()
        {
            if (!AddMeasurementCursors() || null == this.XAxis || null == this.YAxis || false == MeasureCursorVisible)
                return;

            this.XAxis.VisibleRangeChanged += XAxis_VisibleRangeChanged;
            this.YAxis.VisibleRangeChanged += YAxis_VisibleRangeChanged;

            //top line
            _measurementCursors[0].X1 = 0;
            _measurementCursors[0].X2 = this.ModifierSurface.ActualWidth;
            _measurementCursors[0].Y1 = _measurementCursors[0].Y2 = this.ModifierSurface.ActualHeight / 2 - (this.ModifierSurface.ActualHeight / 10);
            //bottom line
            _measurementCursors[1].X1 = 0;
            _measurementCursors[1].X2 = this.ModifierSurface.ActualWidth;
            _measurementCursors[1].Y1 = _measurementCursors[1].Y2 = this.ModifierSurface.ActualHeight / 2 + (this.ModifierSurface.ActualHeight / 10);
            //left line
            _measurementCursors[2].X1 = _measurementCursors[2].X2 = this.ModifierSurface.ActualWidth / 2 - (this.ModifierSurface.ActualWidth / 10);
            _measurementCursors[2].Y1 = 0;
            _measurementCursors[2].Y2 = this.ModifierSurface.ActualHeight;
            //right line
            _measurementCursors[3].X1 = _measurementCursors[3].X2 = this.ModifierSurface.ActualWidth / 2 + (this.ModifierSurface.ActualWidth / 10);
            _measurementCursors[3].Y1 = 0;
            _measurementCursors[3].Y2 = this.ModifierSurface.ActualHeight;
            // translate the coordination
            var xCalc = this.XAxis.GetCurrentCoordinateCalculator();
            var yCalc = this.YAxis.GetCurrentCoordinateCalculator();

            switch ((MeasureCursorType)(MeasureCursorTypeDictionary[MeasureCursor]))
            {
                case MeasureCursorType.All:
                    for (int i = 0; i < _cursorName.Length; i++)
                    {
                        _measurementCursors[i].Visibility = Visibility.Visible;
                        _measurementBorders[i].Visibility = (MeasureBorderVisible) ? Visibility.Visible : Visibility.Collapsed;
                        AddAxislabel(i);
                    }
                    CursorTop = yCalc.GetDataValue(_measurementCursors[0].Y1);
                    CursorBottom = yCalc.GetDataValue(_measurementCursors[1].Y1);
                    CursorLeft = xCalc.GetDataValue(_measurementCursors[2].X1);
                    CursorRight = xCalc.GetDataValue(_measurementCursors[3].X1);
                    break;
                case MeasureCursorType.X_ONLY:
                    for (int i = 0; i < _cursorName.Length; i++)
                    {
                        if (i < 2)
                        {
                            _measurementCursors[i].Visibility = Visibility.Collapsed;
                            _measurementBorders[i].Visibility = Visibility.Collapsed;
                        }
                        else
                        {
                            _measurementCursors[i].Visibility = Visibility.Visible;
                            _measurementBorders[i].Visibility = (MeasureBorderVisible) ? Visibility.Visible : Visibility.Collapsed;
                            AddAxislabel(i);
                        }
                    }
                    CursorLeft = xCalc.GetDataValue(_measurementCursors[2].X1);
                    CursorRight = xCalc.GetDataValue(_measurementCursors[3].X1);
                    break;
                case MeasureCursorType.Y_ONLY:
                    for (int i = 0; i < _cursorName.Length; i++)
                    {
                        if (i < 2)
                        {
                            _measurementCursors[i].Visibility = Visibility.Visible;
                            _measurementBorders[i].Visibility = (MeasureBorderVisible) ? Visibility.Visible : Visibility.Collapsed;
                            AddAxislabel(i);
                        }
                        else
                        {
                            _measurementCursors[i].Visibility = Visibility.Collapsed;
                            _measurementBorders[i].Visibility = Visibility.Collapsed;
                        }
                    }
                    CursorTop = yCalc.GetDataValue(_measurementCursors[0].Y1);
                    CursorBottom = yCalc.GetDataValue(_measurementCursors[1].Y1);
                    break;
                default:
                    break;
            }
        }

        /// <summary>
        /// Displays the cursor line.
        /// </summary>
        public void DisplayFrameLine()
        {
            
            if (!AddFrameCursor() || null == this.XAxis || null == this.YAxis || false == FrameCursorVisible)
                return;

            this.XAxis.VisibleRangeChanged += XAxis_VisibleRangeChanged;
            this.YAxis.VisibleRangeChanged += YAxis_VisibleRangeChanged;

            
            //left line
            _frameLine.X1 = _frameLine.X2 = this.ModifierSurface.ActualWidth / 4 - (this.ModifierSurface.ActualWidth / 10);
            _frameLine.Y1 = 0;
            _frameLine.Y2 = this.ModifierSurface.ActualHeight;
            var xCalc = this.XAxis.GetCurrentCoordinateCalculator();
            var yCalc = this.YAxis.GetCurrentCoordinateCalculator();



            //_measCursors
            _frameLine.Visibility =  Visibility.Visible;

            _frameBorders.Visibility = (FrameBorderVisible) ? Visibility.Visible : Visibility.Collapsed;

            //if (Visibility.Visible != _frameBorders.Visibility)
            // return;

            //A
            List<double> times = RealTimeDataCapture.Instance.FrameTimes;
            TextBlock tmp = (TextBlock)_frameBorders.Child;


            if (null == times) //if times is null but the cursos should be displayed
            {
                tmp.Text = "" + FrameCursorX.ToString("0.###");
                

            }
            else
            {
                int c = 0;
                for (int x = 0; x < times.Count; x++)
                {
                    if (times[x] > FrameCursorX)
                        break;

                    c = c + 1;
                }

                tmp.Text = "" + c;
                //currentFrameNumber = c;
            }

            this.ModifierSurface.Children.Remove(this._frameLine);
            this.ModifierSurface.Children.Add(this._frameLine);

            int len = tmp.Text.Length;

            AxisCanvas.SetLeft(this._frameBorders, this._frameLine.X1 - (9 + (4 * (len - 1))));  //No  longer hardcoded could be better
            AxisCanvas.SetTop(this._frameBorders, -2);

            currentFrameNumber = -1000;
            CursorFrameX = xCalc.GetDataValue(_frameLine.X1);
        }

        public void DisplayFrameLine(int frame)
        { //!AddFrameCursor() 

            if (null == this._frameLine || null == this.YAxis || null == this.XAxis || true == _currentlyDragging )//|| true == IsDragToScale )//|| false == FrameCursorVisible)
                return;
            List<double> times = RealTimeDataCapture.Instance.FrameTimes;
            if (times == null || frame > times.Count)
                return;

            currentFrameNumber = frame;

            TextBlock tmp = (TextBlock)_frameBorders.Child;
            // Position the rollover line
            var yCalc = this.YAxis.GetCurrentCoordinateCalculator();
            var xCalc = this.XAxis.GetCurrentCoordinateCalculator();

            //if(frame - 1 < 0 || frame > times.Count())
            //{
            //    return;
            //} 
            
            this._frameLine.X1 = xCalc.GetCoordinate(times[(frame - 1) * _averaging]); //indexs start at zero, frames start counting with one
            this._frameLine.X2 = this._frameLine.X1;
            this._frameLine.Y1 = 0;
            this._frameLine.Y2 = ModifierSurface.ActualHeight;


            tmp.Text = "" + frame;


            this.ModifierSurface.Children.Remove(this._frameLine);
            this.ModifierSurface.Children.Add(this._frameLine);

            if (Visibility.Visible != this._frameLine.Visibility)
                return;

            int len = tmp.Text.Length;

            AxisCanvas.SetLeft(this._frameBorders, this._frameLine.X1 - (9 + (4 * (len - 1))));  //This is hardcoded for and the else is hardcoded for 2 digits
            AxisCanvas.SetTop(this._frameBorders, -2);

            FrameCursorX = this._frameLine.X1;
            
        }
        /// <summary>
        /// Hides the cursor line.
        /// </summary>
        public void HideFrameLine()
        {
            if (!AddFrameCursor())
                return;

            if (null != this.XAxis)
                this.XAxis.VisibleRangeChanged -= XAxis_VisibleRangeChanged;
            if (null != this.YAxis)
                this.YAxis.VisibleRangeChanged -= YAxis_VisibleRangeChanged;

            
                _frameLine.Visibility = Visibility.Collapsed;
                _frameBorders.Visibility = Visibility.Collapsed;
            

            CursorFrameX = 0;
            
        }
        /// <summary>
        /// Hides the cursor line.
        /// </summary>
        public void HideCursorLine()
        {
            if (!AddMeasurementCursors())
                return;

            if (null != this.XAxis)
                this.XAxis.VisibleRangeChanged -= XAxis_VisibleRangeChanged;
            if (null != this.YAxis)
                this.YAxis.VisibleRangeChanged -= YAxis_VisibleRangeChanged;

            for (int i = 0; i < _cursorName.Length; i++)
            {
                _measurementCursors[i].Visibility = Visibility.Collapsed;
                _measurementBorders[i].Visibility = Visibility.Collapsed;
            }

            CursorTop = CursorBottom = CursorLeft = CursorRight = 0;
        }

        /// <summary>
        /// Called when the Chart Modifier is attached to the Chart Surface
        /// </summary>
        /// <remarks></remarks>
        public override void OnAttached()
        {
            base.OnAttached();
        }

        /// <summary>
        /// Called when the Chart Modifier is detached from the Chart Surface
        /// </summary>
        /// <remarks></remarks>
        public override void OnDetached()
        {
            base.OnDetached();
        }

        /// <summary>
        /// Called when the mouse leaves the Master of current <see cref="P:Abt.Controls.SciChart.ChartModifiers.ChartModifierBase.MouseEventGroup" />
        /// </summary>
        /// <param name="e"></param>
        public override void OnMasterMouseLeave(ModifierMouseArgs e)
        {
            base.OnMasterMouseLeave(e);

            // Remove the rollover line and markers from the surface
            ClearModifierSurface();
        }

        /// <summary>
        /// Called when the Mouse is moved on the parent <see cref="T:Abt.Controls.SciChart.Visuals.SciChartSurface" />
        /// </summary>
        /// <param name="e">Arguments detailing the mouse move operation</param>
        public override void OnModifierMouseMove(ModifierMouseArgs e)
        {
            base.OnModifierMouseMove(e);
            try
            {
                //ParentSurface.XAxis.VisibleRange.SetMinMax((double)ParentSurface.XAxis.VisibleRange.Min+ 1, (double)ParentSurface.XAxis.VisibleRange.Max + 1);

                var allSeries = this.ParentSurface.RenderableSeries; // line in the legend

                // Translates the mouse point to chart area
                var pt = GetPointRelativeTo(e.MousePoint, this.ModifierSurface);

                if (IsRollOver)
                {
                    // Position the rollover line
                    _line.Y1 = 0;
                    _line.Y2 = ModifierSurface.ActualHeight;
                    _line.X1 = pt.X;
                    _line.X2 = pt.X;

                    ClearModifierSurface();

                    // Add the rollover line to the ModifierSurface,
                    // which is just a canvas over the main chart area, on top of series
                    this.ModifierSurface.Children.Add(_line);

                    // Add the rollover points to the surface
                    var hitTestResults = allSeries.Where(x => x.IsVisible == true).Select(x => x.HitTestProvider.HitTest(pt, false)).ToArray();
                    foreach (var hitTestResult in hitTestResults)
                    {
                        const int markerSize = 10;

                        //Create one ellipse per HitTestResult and position on the canvas
                        var ellipse = new Ellipse()
                        {
                            Width = markerSize,
                            Height = markerSize,
                            Fill = _line.Stroke,
                            IsHitTestVisible = false,
                            Tag = typeof(ChartCanvasModifier)
                        };

                        Canvas.SetLeft(ellipse, hitTestResult.HitTestPoint.X - markerSize * 0.5);
                        Canvas.SetTop(ellipse, hitTestResult.HitTestPoint.Y - markerSize * 0.5);

                        this.ModifierSurface.Children.Add(ellipse);

                        // Create one label per HitTestResult and position on the canvas
                        var text = new Border()
                        {
                            IsHitTestVisible = false,
                            BorderBrush = TextForeground,
                            BorderThickness = new Thickness(3),
                            Background = LineBrush,
                            CornerRadius = new CornerRadius(2, 2, 2, 2),
                            Tag = typeof(ChartCanvasModifier),
                            Width = 220,
                            Child = new TextBlock()
                            {
                                Text = string.Format("{0}: {1}, {2}: {3}", XAxis.AxisTitle, Convert.ToDouble(hitTestResult.XValue).ToString("#.000000"), hitTestResult.DataSeriesName, YAxis.FormatText(hitTestResult.YValue)),
                                FontSize = 12,
                                Margin = new Thickness(3),
                                Foreground = TextForeground,
                            }
                        };
                        if (hitTestResult.HitTestPoint.X + 220 > ModifierSurface.ActualWidth)
                        {
                            Canvas.SetLeft(text, hitTestResult.HitTestPoint.X - 220);
                        }
                        else
                        {
                            Canvas.SetLeft(text, hitTestResult.HitTestPoint.X);
                        }

                        Canvas.SetTop(text, hitTestResult.HitTestPoint.Y);

                        this.ModifierSurface.Children.Add(text);
                    }
                }

                if (false == IsDragToScale) // Updata Coordination of Meaesurement Cursors
                {
                    // Check the mouse point was inside the ModifierSurface (the central chart area). If not, exit
                    var modifierSurfaceBounds = ModifierSurface.GetBoundsRelativeTo(RootGrid);

                    // Position the rollover line
                    switch (CursorSelectedIndex)
                    {
                        case 0:
                            if (pt.Y > modifierSurfaceBounds.Bottom || pt.Y < modifierSurfaceBounds.Top)
                                return;

                            CursorTop = this.YAxis.GetCurrentCoordinateCalculator().GetDataValue(pt.Y);
                            break;
                        case 1:
                            if (pt.Y > modifierSurfaceBounds.Bottom || pt.Y < modifierSurfaceBounds.Top)
                                return;

                            CursorBottom = this.YAxis.GetCurrentCoordinateCalculator().GetDataValue(pt.Y);
                            break;
                        case 2:
                            if (pt.X < modifierSurfaceBounds.Left || pt.X > modifierSurfaceBounds.Right)
                                return;

                            CursorLeft = this.XAxis.GetCurrentCoordinateCalculator().GetDataValue(pt.X);
                            break;
                        case 3:
                            if (pt.X < modifierSurfaceBounds.Left || pt.X > modifierSurfaceBounds.Right)
                                return;

                            CursorRight = this.XAxis.GetCurrentCoordinateCalculator().GetDataValue(pt.X);
                            break;
                        default:
                            if (pt.X < modifierSurfaceBounds.Left || pt.X > modifierSurfaceBounds.Right)
                                return;

                            FrameCursorX = this.XAxis.GetCurrentCoordinateCalculator().GetDataValue(pt.X);
                            break;
                    }
                    
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
        }

        /// <summary>
        /// Called when a Mouse Button is released on the parent <see cref="T:Abt.Controls.SciChart.Visuals.SciChartSurface" />
        /// </summary>
        /// <param name="e">Arguments detailing the mouse button operation</param>
        public override void OnModifierMouseUp(ModifierMouseArgs e)
        {
            IsDragToScale = true;
            base.OnModifierMouseUp(e);
        }

        /// <summary>
        /// Called when the parent SciChartSurface is resized
        /// </summary>
        /// <param name="e">The <see cref="T:Abt.Controls.SciChart.SciChartResizedMessage" /> which contains the event arg data</param>
        public override void OnParentSurfaceResized(SciChartResizedMessage e)
        {
            base.OnParentSurfaceResized(e);
            if (true == MeasureCursorVisible && (this.ParentSurface.ModifierSurface.Children[0] as Line) != null)
            {
                var xCalc = this.XAxis.GetCurrentCoordinateCalculator();
                var yCalc = this.YAxis.GetCurrentCoordinateCalculator();

                (this.ParentSurface.ModifierSurface.Children[0] as Line).X2 = this.ModifierSurface.ActualWidth;
                (this.ParentSurface.ModifierSurface.Children[0] as Line).Y1 = (this.ParentSurface.ModifierSurface.Children[0] as Line).Y2 = yCalc.GetCoordinate(CursorTop);

                (this.ParentSurface.ModifierSurface.Children[1] as Line).X2 = this.ModifierSurface.ActualWidth;
                (this.ParentSurface.ModifierSurface.Children[1] as Line).Y1 = (this.ParentSurface.ModifierSurface.Children[1] as Line).Y2 = yCalc.GetCoordinate(CursorBottom);

                (this.ParentSurface.ModifierSurface.Children[2] as Line).Y2 = this.ModifierSurface.ActualHeight;
                (this.ParentSurface.ModifierSurface.Children[2] as Line).X1 = (this.ParentSurface.ModifierSurface.Children[2] as Line).X2 = xCalc.GetCoordinate(CursorLeft);

                (this.ParentSurface.ModifierSurface.Children[3] as Line).Y2 = this.ModifierSurface.ActualHeight;
                (this.ParentSurface.ModifierSurface.Children[3] as Line).X1 = (this.ParentSurface.ModifierSurface.Children[3] as Line).X2 = xCalc.GetCoordinate(CursorRight);

                //TODO resize frame cursor

                for (int i = 0; i < _cursorName.Length; i++)
                {
                    AddAxislabel(i);
                }
            }
        }

        private static void OnCursorBottomChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            UpdateCursorTBLR(1, (double)e.NewValue);
        }

        private static void OnCursorLeftChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            UpdateCursorTBLR(2, (double)e.NewValue);
        }

        private static void OnCursorRightChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            UpdateCursorTBLR(3, (double)e.NewValue);
        }

        private static void OnCursorSelectedIndexChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            throw new NotImplementedException();
        }

        private static void OnCursorTopChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            UpdateCursorTBLR(0, (double)e.NewValue);
        }

        public static void OnFrameCursorChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            UpdateFrameCursorX((double)e.NewValue);
        }

        /// <summary>
        /// Called when [line brush changed].
        /// </summary>
        /// <param name="d">The d.</param>
        /// <param name="e">The <see cref="DependencyPropertyChangedEventArgs"/> instance containing the event data.</param>
        private static void OnLineBrushChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            var modifier = d as ChartCanvasModifier;
            //CreateLine(modifier);
        }

        private static void OnMeasureCursorVisibleChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            UpdateMeasureCursor((bool)e.NewValue);
   
        }

        private static void OnFrameCursorVisibleChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            UpdateFrameCursor((bool)e.NewValue);
        }

        

        /// <summary>
        /// Add the measurement cursors once.
        /// </summary>
        private bool AddMeasurementCursors()
        {
            if (null == this.ModifierSurface || null == this.ParentSurface)
                return false;

            if (null == _measurementCursors)
            {
                InitMeasurementCursors();

                for (int i = 0; i < _cursorName.Length; i++)
                {
                    this.ModifierSurface.Children.Insert(i, _measurementCursors[i]);
                    if (i < 2)
                    {
                        ParentSurface.YAxis.ModifierAxisCanvas.Children.Insert(i, _measurementBorders[i]);
                    }
                    else
                    {
                        ParentSurface.XAxis.ModifierAxisCanvas.Children.Insert(i - 2, _measurementBorders[i]);
                    }
                }
            }
            return true;
        }

        /// <summary>
        /// Add the measurement cursors once.
        /// </summary>
        private bool AddFrameCursor()
        {
            if (null == this.ModifierSurface || null == this.ParentSurface)
                return false;

            if (null == _frameLine)
            {
                InitFrameCursor();

                ParentSurface.ModifierSurface.Children.Add(_frameLine);



                ParentSurface.XAxis.ModifierAxisCanvas.Children.Add( _frameBorders);
                    
                
            }
            return true;
        }


        /// <summary>
        /// Initializes the mesurement cursors.
        /// </summary>
        private void InitFrameCursor()
        {
            if (null == _frameLine)
            {

                    this._frameLine = new Line()
                    {
                        Name = _frameName,
                        Stroke = Brushes.Orange,
                        StrokeThickness = 4,
                        IsHitTestVisible = true,
                        Visibility = Visibility.Collapsed,
                    };

                    this._frameBorders = new Border()
                    {
                        Name = FrameBorder,
                        BorderBrush = Brushes.OrangeRed,
                        BorderThickness = new Thickness(2),
                        Background = Brushes.Orange,
                        CornerRadius = new CornerRadius(2, 2, 2, 2),
                        Child = new TextBlock()
                        {
                            Text = "NA",
                            FontSize = 12,
                            Margin = new Thickness(3),
                            Foreground = TextForeground,
                        },
                        Visibility = Visibility.Collapsed,
                    };
                    this._frameBorders.MouseDown += ChartBordersModifier_MouseDown;
                    this._frameBorders.MouseEnter += ChartBordersModifier_MouseEnter;
                    this._frameBorders.MouseLeave += ChartBordersModifier_MouseLeave;
            }
        }

        /// <summary>
        /// Handles the MouseDown event of the ChartBordersModifier control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="MouseButtonEventArgs"/> instance containing the event data.</param>
        private void ChartBordersModifier_MouseDown(object sender, MouseButtonEventArgs e)
        {
            _currentlyDragging = true;
            var border = sender as Border;

            string[] allCursors = new string[cursorBorder.Length + 1];

            System.Array.Copy(cursorBorder, allCursors, cursorBorder.Length);

            allCursors[allCursors.Length - 1] = FrameBorder;

            CursorSelectedIndex = Array.IndexOf(allCursors, border.Name);
            IsDragToScale = false;
            e.Handled = true;


        }

        /// <summary>
        /// Handles the MouseEnter event of the ChartBordersModifier control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="MouseEventArgs"/> instance containing the event data.</param>
        private void ChartBordersModifier_MouseEnter(object sender, MouseEventArgs e)
        {
            if (this.Cursor != Cursors.Wait)
            {
                Mouse.OverrideCursor = Cursors.Hand;
            }
            e.Handled = true;
        }

        /// <summary>
        /// Handles the MouseLeave event of the ChartBordersModifier control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="MouseEventArgs"/> instance containing the event data.</param>
        private void ChartBordersModifier_MouseLeave(object sender, MouseEventArgs e)
        {
            _currentlyDragging = false;
            if (this.Cursor != Cursors.Wait)
            {
                Mouse.OverrideCursor = Cursors.Arrow;
            }
            
            if ( RealTimeLineChartViewModel._instance.ThorImageLSConnectionStats) // ThorImageLSConnectionStats
            {
                RealTimeLineChartViewModel._instance.SendToClient("SyncFrame", currentFrameNumber + "");
            }
            
            e.Handled = true;
        }

        /// <summary>
        /// Clears the modifier surface.
        /// </summary>
        private void ClearModifierSurface()
        {
            for (int i = ParentSurface.ModifierSurface.Children.Count - 1; i >= 0; --i)
            {
                if ((System.Type)(((FrameworkElement)ParentSurface.ModifierSurface.Children[i]).Tag) == typeof(ChartCanvasModifier))
                {
                    ParentSurface.ModifierSurface.Children.RemoveAt(i);
                }
            }
            // this.ModifierSurface.Clear();
        }

        /// <summary>
        /// Initializes the mesurement cursors.
        /// </summary>
        private void InitMeasurementCursors()
        {
            if (null == _measurementCursors)
            {
                _measurementCursors = new Line[4];

                for (int i = 0; i < _cursorName.Length; i++)
                {
                    this._measurementCursors[i] = new Line()
                    {
                        Name = _cursorName[i],
                        Stroke = Brushes.LimeGreen,
                        StrokeThickness = 4,
                        IsHitTestVisible = true,
                        Visibility = Visibility.Collapsed,
                    };

                    this._measurementBorders[i] = new Border()
                    {
                        Name = cursorBorder[i],
                        BorderBrush = TextForeground,
                        BorderThickness = new Thickness(3),
                        Background = Brushes.LimeGreen,
                        CornerRadius = new CornerRadius(2, 2, 2, 2),
                        Child = new TextBlock()
                        {
                            Text = _cursorName[i],
                            FontSize = 12,
                            Margin = new Thickness(3),
                            Foreground = TextForeground,
                        },
                        Visibility = Visibility.Collapsed,
                    };
                    this._measurementBorders[i].MouseDown += ChartBordersModifier_MouseDown;
                    this._measurementBorders[i].MouseEnter += ChartBordersModifier_MouseEnter;
                    this._measurementBorders[i].MouseLeave += ChartBordersModifier_MouseLeave;
                }
            }
        }

        /// <summary>
        /// redraw measure cursor by index of Top(0), Bottom(1), Left(2), Right(3)
        /// </summary>
        /// <param name="index"></param>
        /// <param name="obj"></param>
        private void RedrawCursorTBLR(int index, double obj)
        {
            if (null == _measurementCursors || null == this.YAxis || null == this.XAxis || true == IsDragToScale || false == MeasureCursorVisible)
                return;

            // Position the rollover line
            var yCalc = this.YAxis.GetCurrentCoordinateCalculator();
            var xCalc = this.XAxis.GetCurrentCoordinateCalculator();
            switch (index)
            {
                case 0:
                    {
                        _measurementCursors[index].X1 = 0;
                        _measurementCursors[index].X2 = ModifierSurface.ActualWidth;
                        _measurementCursors[index].Y1 = yCalc.GetCoordinate(CursorTop);
                        _measurementCursors[index].Y2 = _measurementCursors[index].Y1;
                    } break;
                case 1:
                    {
                        _measurementCursors[index].X1 = 0;
                        _measurementCursors[index].X2 = ModifierSurface.ActualWidth;
                        _measurementCursors[index].Y1 = yCalc.GetCoordinate(CursorBottom);
                        _measurementCursors[index].Y2 = _measurementCursors[index].Y1;
                    } break;
                case 2:
                    {
                        _measurementCursors[index].X1 = xCalc.GetCoordinate(CursorLeft);
                        _measurementCursors[index].X2 = _measurementCursors[index].X1;
                        _measurementCursors[index].Y1 = 0;
                        _measurementCursors[index].Y2 = ModifierSurface.ActualHeight;
                    } break;
                case 3:
                    {
                        _measurementCursors[index].X1 = xCalc.GetCoordinate(CursorRight);
                        _measurementCursors[index].X2 = _measurementCursors[index].X1;
                        _measurementCursors[index].Y1 = 0;
                        _measurementCursors[index].Y2 = ModifierSurface.ActualHeight;
                    } break;
                default: break;
            }
            ParentSurface.ModifierSurface.Children.RemoveAt(index);
            this.ModifierSurface.Children.Insert(index, _measurementCursors[index]);
            AddAxislabel(index);
        }


        public void RedrawFrameCursor(double obj)
        {
            if (null == _frameLine|| null == this.YAxis || null == this.XAxis || true == IsDragToScale || false == FrameCursorVisible)
                return;

            // Position the rollover line
            var yCalc = this.YAxis.GetCurrentCoordinateCalculator();
            var xCalc = this.XAxis.GetCurrentCoordinateCalculator();

            if(xCalc.GetCoordinate(FrameCursorX) > this.XAxis.Width ) //Math.Abs(this.XAxis.Width - _frameLine.X1) > 25
            {
                return;
            }

            List<double> times = RealTimeDataCapture.Instance.FrameTimes;
            TextBlock tmp = (TextBlock)_frameBorders.Child;

            int frameCounter = 0;
            if (times == null) //if times is null but the cursos should be displayed
            {
                try
                {
                    RealTimeLineChartViewModel._instance.RefreshFrameTimes = false;
                    for (int x = 0; x < times.Count; x++)
                    {
                        if (times[x] > FrameCursorX)
                            break;

                        frameCounter = frameCounter + 1;
                    }
                }
                catch
                {
                    tmp.Text = "" + FrameCursorX.ToString("0.###");
                    RealTimeDataCapture.Instance.FrameTimes = null;
                }
            }
            
            else
            {

                for (int x = 0; x < times.Count; x++)
                {
                    if (times[x] > FrameCursorX)
                        break;

                    frameCounter = frameCounter + 1;
                }
            }
            
            if (Math.Abs(currentFrameNumber - frameCounter) > (.99) * times.Count && currentFrameNumber != -1000)
            {
                return;
            }

            //frameNumber to thorimage ipc
            if ((currentFrameNumber != frameCounter && currentFrameNumber != -1) || currentFrameNumber == -1000)
            {
                if(times != null)
                {
                    if(frameCounter <= times.Count && RealTimeLineChartViewModel._instance.ThorImageLSConnectionStats) // ThorImageLSConnectionStats
                    {
                        if (_averaging > 1)
                        {
                            RealTimeLineChartViewModel._instance.SendToClient("SyncFrame", Math.Ceiling((frameCounter / (double)_averaging)) + "");
                        }
                        else
                        {
                            RealTimeLineChartViewModel._instance.SendToClient("SyncFrame", (frameCounter) + "");
                        }
                    }

                    if (frameCounter > times.Count ) // ThorImageLSConnectionStats
                    {
                        return;
                    }
                }
                
            }

            _frameLine.X1 = xCalc.GetCoordinate(FrameCursorX);
            _frameLine.X2 = _frameLine.X1;
            _frameLine.Y1 = 0;
            _frameLine.Y2 = ModifierSurface.ActualHeight;

            if(_averaging > 1)
            {
                currentFrameNumber = ((frameCounter / _averaging) + 1);
            }
            else
            {
                currentFrameNumber = frameCounter;
            }
            
            
            this.ModifierSurface.Children.Remove(this._frameLine);
            this.ModifierSurface.Children.Add(this._frameLine);

            if (Visibility.Visible != _frameLine.Visibility)
                return;

            if (_averaging > 1)
            {
                tmp.Text = "" + Math.Ceiling((frameCounter / (double)_averaging));
            }
            else
            {
                tmp.Text = "" + frameCounter;
            }
            
            int len = tmp.Text.Length;


            AxisCanvas.SetLeft(this._frameBorders, this._frameLine.X1 - (9 + (4 * (len - 1))));  //This is hardcoded for and the else is hardcoded for 2 digits
            AxisCanvas.SetTop(this._frameBorders, -2);


        }


        /// <summary>
        /// Display or hide measure cursors
        /// </summary>
        /// <param name="obj"></param>
        private void UpdateMeasureCursorStatus(bool obj)
        {
            if (obj)
            {
                DisplayCursorLine();
            }
            else
            {
                HideCursorLine();
            }
        }

        /// <summary>
        /// Display or hide measure cursors
        /// </summary>
        /// <param name="obj"></param>
        private void UpdateFrameCursorStatus(bool obj)
        {
            if (obj)
            {
                DisplayFrameLine();
            }
            else
            {
                HideFrameLine();
            }
        }


        private void XAxis_VisibleRangeChanged(object sender, VisibleRangeChangedEventArgs e)
        {
            if (MeasureCursorVisible)
            {
                if ((Visibility.Visible != _measurementCursors[2].Visibility) || (Visibility.Visible != _measurementCursors[3].Visibility))
                    return;

                var xCalc = this.XAxis.GetCurrentCoordinateCalculator();
                CursorLeft = xCalc.GetDataValue(_measurementCursors[2].X1);
                CursorRight = xCalc.GetDataValue(_measurementCursors[3].X1);
            }

            if(FrameCursorVisible)
            {
                if ((Visibility.Visible != _frameLine.Visibility)) //If frame line is not visible
                    return;

                var xCalc = this.XAxis.GetCurrentCoordinateCalculator();
                CursorFrameX = xCalc.GetDataValue(_frameLine.X1);
            }

        }

        private void YAxis_VisibleRangeChanged(object sender, VisibleRangeChangedEventArgs e)
        {
            if (MeasureCursorVisible)
            {
                if ((Visibility.Visible != _measurementCursors[0].Visibility) || (Visibility.Visible != _measurementCursors[1].Visibility))
                    return;

                var yCalc = this.YAxis.GetCurrentCoordinateCalculator();
                CursorTop = yCalc.GetDataValue(_measurementCursors[0].Y1);
                CursorBottom = yCalc.GetDataValue(_measurementCursors[1].Y1);
            }
        }

        #endregion Methods
    }

    public class DataPointSelectionModifier : ChartModifierBase
    {
        #region Fields

        public static readonly DependencyProperty SelectionPolygonStyleProperty = DependencyProperty.Register(
            "SelectionPolygonStyle", typeof(Style), typeof(DataPointSelectionModifier), new PropertyMetadata(default(Style)));

        private Point _endPoint;
        private bool _isDragging;

        /// <summary>
        /// reticule
        /// </summary>
        private Rectangle _rectangle;
        private Point _startPoint;

        #endregion Fields

        #region Properties

        /// <summary>
        /// Gets whether the user is currently dragging the mouse
        /// </summary>
        public bool IsDragging
        {
            get { return _isDragging; }
        }

        public Style SelectionPolygonStyle
        {
            get { return (Style)GetValue(SelectionPolygonStyleProperty); }
            set { SetValue(SelectionPolygonStyleProperty, value); }
        }

        #endregion Properties

        #region Methods

        /// <summary>
        /// Called when the Chart Modifier is attached to the Chart Surface
        /// </summary>
        /// <remarks></remarks>
        public override void OnAttached()
        {
            base.OnAttached();

            ClearReticule();
        }

        /// <summary>
        /// Called when the Chart Modifier is detached from the Chart Surface
        /// </summary>
        /// <remarks></remarks>
        public override void OnDetached()
        {
            base.OnDetached();

            ClearReticule();
        }

        public override void OnModifierMouseDown(ModifierMouseArgs e)
        {
            base.OnModifierMouseDown(e);

            // Check the ExecuteOn property and if we are already dragging. If so, exit
            if (_isDragging || !MatchesExecuteOn(e.MouseButtons, ExecuteOn))
                return;

            // Check the mouse point was inside the ModifierSurface (the central chart area). If not, exit
            var modifierSurfaceBounds = ModifierSurface.GetBoundsRelativeTo(RootGrid);
            if (!modifierSurfaceBounds.Contains(e.MousePoint))
            {
                return;
            }

            // Capture the mouse, so if mouse goes out of bounds, we retain mouse events
            if (e.IsMaster)
                ModifierSurface.CaptureMouse();

            // Translate the mouse point (which is in RootGrid coordiantes) relative to the ModifierSurface
            // This accounts for any offset due to left Y-Axis
            var ptTrans = GetPointRelativeTo(e.MousePoint, ModifierSurface);

            _startPoint = ptTrans;
            _rectangle = new Rectangle
            {
                Style = SelectionPolygonStyle,
            };

            // Update the zoom recticule position
            SetReticulePosition(_rectangle, _startPoint, _startPoint, e.IsMaster);

            // Add the zoom reticule to the ModifierSurface - a canvas over the chart
            ModifierSurface.Children.Add(_rectangle);

            // Set flag that a drag has begun
            _isDragging = true;
        }

        public override void OnModifierMouseMove(ModifierMouseArgs e)
        {
            if (!_isDragging)
                return;

            base.OnModifierMouseMove(e);
            e.Handled = true;

            // Translate the mouse point (which is in RootGrid coordiantes) relative to the ModifierSurface
            // This accounts for any offset due to left Y-Axis
            var ptTrans = GetPointRelativeTo(e.MousePoint, ModifierSurface);

            // Update the zoom recticule position
            SetReticulePosition(_rectangle, _startPoint, ptTrans, e.IsMaster);
        }

        public override void OnModifierMouseUp(ModifierMouseArgs e)
        {
            if (!_isDragging)
                return;

            base.OnModifierMouseUp(e);

            // Translate the mouse point (which is in RootGrid coordiantes) relative to the ModifierSurface
            // This accounts for any offset due to left Y-Axis
            var ptTrans = GetPointRelativeTo(e.MousePoint, ModifierSurface);

            _endPoint = SetReticulePosition(_rectangle, _startPoint, ptTrans, e.IsMaster);

            double distanceDragged = PointUtil.Distance(_startPoint, ptTrans);
            if (distanceDragged > 10.0)
            {
                PerformSelection(_startPoint, _endPoint);
                e.Handled = true;
            }

            ClearReticule();
            _isDragging = false;

            if (e.IsMaster)
                ModifierSurface.ReleaseMouseCapture();
        }

        private static Point ClipToBound(Rect rect, Point point)
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

        private void ClearReticule()
        {
            if (ModifierSurface != null && _rectangle != null)
            {
                ModifierSurface.Children.Remove(_rectangle);
                _rectangle = null;
                _isDragging = false;

            }
        }

        private void PerformSelection(Point startPoint, Point endPoint)
        {
            Console.WriteLine("TODO: Perform Selection. StartPoint = {0},{1}. EndPoint = {2},{3}",
                startPoint.X, startPoint.Y, endPoint.X, endPoint.Y);
        }

        private Point SetReticulePosition(Rectangle rectangle, Point startPoint, Point endPoint, bool isMaster)
        {
            var modifierRect = new Rect(0, 0, ModifierSurface.ActualWidth, ModifierSurface.ActualHeight);
            endPoint = ClipToBound(modifierRect, endPoint);

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

    /// <summary>
    /// Implementation of MouseWheelZoomModifier 
    /// Defines custom behaviours when using the mousewheel in XAML where declare <MouseWheelZoomModifier/>
    /// </summary>
    public class MouseWheelZoomCustomModifier : MouseWheelZoomModifier
    {
        #region Fields

        public static readonly DependencyProperty IsXOnlyProperty = DependencyProperty.Register("IsXOnly", typeof(bool), typeof(MouseWheelZoomCustomModifier), new PropertyMetadata(false));

        #endregion Fields

        #region Properties

        public bool IsXOnly
        {
            get { return (bool)GetValue(IsXOnlyProperty); }
            set { SetValue(IsXOnlyProperty, value); }
        }

        #endregion Properties

        #region Methods

        public override void OnModifierMouseWheel(ModifierMouseArgs e)
        {
            switch (e.Modifier)
            {
                case MouseModifier.Ctrl:
                    this.XyDirection = XyDirection.YDirection;
                    this.ActionType = ActionType.Pan;
                    base.OnModifierMouseWheel(e);
                    break;
                case MouseModifier.Shift:
                    this.XyDirection = XyDirection.XDirection;
                    this.ActionType = ActionType.Pan;
                    base.OnModifierMouseWheel(e);
                    break;
                case MouseModifier.Alt:
                    // If zooming in
                    if (e.Delta > 0)
                    {
                        YAxis.ZoomBy(-0.1, -0.1);
                    }
                    // If zooming out
                    else if (e.Delta < 0)
                    {
                        YAxis.ZoomBy(0.1, 0.1);
                    }
                    e.Handled = true;
                    break;
                default:
                    this.XyDirection = (IsXOnly) ? XyDirection.XDirection : XyDirection.XYDirection;
                    this.ActionType = ActionType.Zoom;
                    base.OnModifierMouseWheel(e);
                    break;
            }
        }

        #endregion Methods
    }

    public class RightDoubleClickZoomExtentsModifier : ZoomExtentsModifier
    {
        #region Fields

        private Stopwatch stopwatch = new Stopwatch();

        #endregion Fields

        #region Methods

        [DllImport("user32.dll", CharSet = CharSet.Auto, ExactSpelling = true)]
        public static extern int GetDoubleClickTime();

        public override void OnModifierDoubleClick(ModifierMouseArgs e)
        {
            if (e.MouseButtons == MouseButtons.Right)
            {
                base.OnModifierDoubleClick(e);
            }
        }

        public override void OnModifierMouseDown(ModifierMouseArgs e)
        {
            if (e.MouseButtons == MouseButtons.Right && stopwatch.IsRunning)
            {
                if (stopwatch.ElapsedMilliseconds < GetDoubleClickTime())
                {
                    base.OnModifierDoubleClick(e);
                }
                stopwatch.Restart();
            }
            else if (!stopwatch.IsRunning)
            {
                stopwatch.Start();
            }
            base.OnModifierMouseDown(e);
        }

        #endregion Methods
    }

    public class RubberBandXyZoomModifierEx : RubberBandXyZoomModifier
    {
        #region Fields

        public static readonly DependencyProperty FreqSampleSecMaxProperty = DependencyProperty.Register("FreqSampleSecMax", typeof(double), typeof(RubberBandXyZoomModifierEx), new PropertyMetadata(0.0));
        public static readonly DependencyProperty FreqSampleSecMinProperty = DependencyProperty.Register("FreqSampleSecMin", typeof(double), typeof(RubberBandXyZoomModifierEx), new PropertyMetadata(0.0));
        public static readonly DependencyProperty SelectionPolygonStyleProperty = DependencyProperty.Register("SelectionPolygonStyle", typeof(Style), typeof(ChartCanvasModifier), new PropertyMetadata(default(Style)));

        public static Rectangle _rectangle = null;

        private Point _endPoint;
        private bool _isKeyDown = false; //allow consistent right mouse down event
        private Point _startPoint;

        #endregion Fields

        #region Constructors

        public RubberBandXyZoomModifierEx()
            : base()
        {
        }

        #endregion Constructors

        #region Properties

        public double FreqSampleSecMax
        {
            get { return (double)GetValue(FreqSampleSecMaxProperty); }
            set { SetValue(FreqSampleSecMaxProperty, value); }
        }

        public double FreqSampleSecMin
        {
            get { return (double)GetValue(FreqSampleSecMinProperty); }
            set { SetValue(FreqSampleSecMinProperty, value); }
        }

        public Style SelectionPolygonStyle
        {
            get { return (Style)GetValue(SelectionPolygonStyleProperty); }
            set { SetValue(SelectionPolygonStyleProperty, value); }
        }

        #endregion Properties

        #region Methods

        /// <summary>
        /// Called when the Chart Modifier is attached to the Chart Surface
        /// </summary>
        /// <remarks></remarks>
        public override void OnAttached()
        {
            base.OnAttached();
        }

        /// <summary>
        /// Called when the Chart Modifier is detached from the Chart Surface
        /// </summary>
        /// <remarks></remarks>
        public override void OnDetached()
        {
            base.OnDetached();
        }

        public override void OnModifierMouseDown(ModifierMouseArgs e)
        {
            // Check the ExecuteOn property and if we are already dragging. If so, exit
            if ((!MatchesExecuteOn(e.MouseButtons, ExecuteOn)))
            {
                ReleaseRectangle();
                return;
            }
            // Check the mouse point was inside the ModifierSurface (the central chart area). If not, exit
            var modifierSurfaceBounds = ModifierSurface.GetBoundsRelativeTo(RootGrid);
            if (!modifierSurfaceBounds.Contains(e.MousePoint))
            {
                return;
            }

            // windowing with ctrl key down
            if ((Keyboard.IsKeyDown(Key.LeftCtrl)) || (Keyboard.IsKeyDown(Key.RightCtrl)))
            {
                _isKeyDown = true;

                // Capture the mouse, so if mouse goes out of bounds, we retain mouse events
                if (e.IsMaster)
                    ModifierSurface.CaptureMouse();

                // Translate the mouse point (which is in RootGrid coordiantes) relative to the ModifierSurface
                // This accounts for any offset due to left Y-Axis
                _startPoint = GetPointRelativeTo(e.MousePoint, ModifierSurface);

                _rectangle = new Rectangle
                {
                    Style = SelectionPolygonStyle,
                };

                // Update the zoom recticule position
                SetReticulePosition(_rectangle, _startPoint, _startPoint, e.IsMaster);

                // Add the zoom reticule to the ModifierSurface - a canvas over the chart
                ModifierSurface.Children.Add(_rectangle);
            }
            else
            {
                base.OnModifierMouseDown(e);
            }
        }

        /// <summary>
        /// Called when the Mouse is moved on the parent <see cref="T:Abt.Controls.SciChart.Visuals.SciChartSurface" />
        /// </summary>
        /// <param name="e">Arguments detailing the mouse move operation</param>
        public override void OnModifierMouseMove(ModifierMouseArgs e)
        {
            /**************************added for windowing********************/
            if (_isKeyDown)
            {
                if ((!Keyboard.IsKeyDown(Key.LeftCtrl)) && (!Keyboard.IsKeyDown(Key.RightCtrl)))
                {
                    ReleaseRectangle();
                    return;
                }
                var ptTrans = GetPointRelativeTo(e.MousePoint, ModifierSurface);

                // Update the zoom recticule position
                SetReticulePosition(_rectangle, _startPoint, ptTrans, e.IsMaster);
            }
            else
            {
                base.OnModifierMouseMove(e);
            }
        }

        /// <summary>
        /// Called when a Mouse Button is released on the parent <see cref="T:Abt.Controls.SciChart.Visuals.SciChartSurface" />
        /// </summary>
        /// <param name="e">Arguments detailing the mouse button operation</param>
        public override void OnModifierMouseUp(ModifierMouseArgs e)
        {
            /********************************added for windowing*************************************/
            if (_isKeyDown)
            {
                if ((Keyboard.IsKeyDown(Key.LeftCtrl)) || (Keyboard.IsKeyDown(Key.RightCtrl)))
                {
                    // Translate the mouse point (which is in RootGrid coordiantes) relative to the ModifierSurface
                    // This accounts for any offset due to left Y-Axis
                    var ptTrans = GetPointRelativeTo(e.MousePoint, ModifierSurface);

                    _endPoint = SetReticulePosition(_rectangle, _startPoint, ptTrans, e.IsMaster);

                    double distanceDragged = PointUtil.Distance(_startPoint, ptTrans);
                    if (distanceDragged > 0.0)
                    {
                        PerformSelection(_startPoint, _endPoint);
                    }
                }
                e.Handled = true;
            }
            else
            {
                base.OnModifierMouseUp(e);
            }
            ReleaseRectangle();
        }

        private static Point ClipToBound(Rect rect, Point point)
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

        private void PerformSelection(Point startPoint, Point endPoint)
        {
            if (0 < base.ParentSurface.RenderableSeries.Count)
            {
                var renderSeries = base.ParentSurface.RenderableSeries[0];

                // We're going to convert our start/end mouse points to data values using the CoordinateCalculator API
                // Note: that RenderSeries can have different XAxis, YAxis, so we use the axes from the RenderSeries not the primary axes on the chart
                var xCalc = renderSeries.XAxis.GetCurrentCoordinateCalculator();

                // Find the bounds of the data inside the rectangle
                double leftXData = xCalc.GetDataValue(startPoint.X);
                double rightXData = xCalc.GetDataValue(endPoint.X);

                FreqSampleSecMin = Math.Min(leftXData, rightXData);
                FreqSampleSecMax = Math.Max(leftXData, rightXData);
                this.ParentSurface.InvalidateElement();
            }
        }

        private void ReleaseRectangle()
        {
            if (ModifierSurface != null && _rectangle != null)
            {
                ModifierSurface.Children.Remove(_rectangle);
                _rectangle = null;
            }
            _isKeyDown = false;
            ModifierSurface.ReleaseMouseCapture();
        }

        private Point SetReticulePosition(Rectangle rectangle, Point startPoint, Point endPoint, bool isMaster)
        {
            var modifierRect = new Rect(0, 0, ModifierSurface.ActualWidth, ModifierSurface.ActualHeight);

            endPoint = ClipToBound(modifierRect, endPoint);

            if (rectangle != null)
            {
                //draw x-range only, keep start point at top
                Point topStartPoint = new Point(startPoint.X, (double)0.0);
                Point bottomEndPoint = new Point(endPoint.X, ModifierSurface.ActualHeight);
                var rect = new Rect(topStartPoint, bottomEndPoint);
                Canvas.SetLeft(rectangle, rect.X);
                Canvas.SetTop(rectangle, rect.Y);

                rectangle.Width = rect.Width;
                rectangle.Height = rect.Height;
            }
            return endPoint;
        }

        #endregion Methods
    }
}