namespace ROIStatsChart.CustomModifiers
{
    using System;
    using System.Linq;
    using System.Text;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Media;
    using System.Windows.Shapes;

    using ROIStatsChart.Model;

    using SciChart.Charting;
    using SciChart.Charting.ChartModifiers;
    using SciChart.Charting.Visuals.Axes.LabelProviders;
    using SciChart.Core.Utility.Mouse;

    /// <summary>
    /// Implementation of RolloverModifier 
    ///  - Expose styles for the line, rollover points and allow them to be customized in XAML where declare <ChartRolloverModifier/>
    /// </summary>
    public class ChartRolloverModifier : ChartModifierBase
    {
        #region Fields

        public static readonly DependencyProperty LineBrushProperty = DependencyProperty.Register("LineBrush", typeof(Brush), typeof(ChartRolloverModifier), new PropertyMetadata(new SolidColorBrush(Colors.Gray), OnLineBrushChanged));
        public static readonly DependencyProperty TextForegroundProperty = DependencyProperty.Register("TextForeground", typeof(Brush), typeof(ChartRolloverModifier), new PropertyMetadata(new SolidColorBrush(Colors.White)));

        private Line _line;

        #endregion Fields

        #region Constructors

        public ChartRolloverModifier()
        {
            CreateLine(this);
        }

        #endregion Constructors

        #region Properties

        public Brush LineBrush
        {
            get { return (Brush)GetValue(LineBrushProperty); }
            set { SetValue(LineBrushProperty, value); }
        }

        public Brush TextForeground
        {
            get { return (Brush)GetValue(TextForegroundProperty); }
            set { SetValue(TextForegroundProperty, value); }
        }

        #endregion Properties

        #region Methods

        public override void OnMasterMouseLeave(ModifierMouseArgs e)
        {
            base.OnMasterMouseLeave(e);

            // Remove the rollover line and markers from the surface
            ClearModifierSurface();
        }

        public override void OnModifierMouseMove(ModifierMouseArgs e)
        {
            base.OnModifierMouseMove(e);

            var allSeries = this.ParentSurface.RenderableSeries;

            // Translates the mouse point to chart area
            var pt = GetPointRelativeTo(e.MousePoint, this.ModifierSurface);

            // Add the rollover points to the surface
            var hitTestResults = allSeries.Where(x => x.IsVisible == true).Select(x => x.HitTestProvider.HitTest(pt, false)).ToArray();

            if (0 < hitTestResults.Count())
            {
                // Position the rollover line
                _line.Y1 = 0;
                _line.Y2 = ModifierSurface.ActualHeight;
                _line.X1 = hitTestResults[0].HitTestPoint.X;
                _line.X2 = hitTestResults[0].HitTestPoint.X;

                ClearModifierSurface();

                // Add the rollover line to the ModifierSurface,
                // which is just a canvas over the main chart area, on top of series
                this.ModifierSurface.Children.Add(_line);
            }

            foreach (var hitTestResult in hitTestResults)
            {
                if ((null == hitTestResult.DataSeriesName) || (null == hitTestResult.XValue))
                {
                    break;
                }
                const int markerSize = 10;

                // Create one ellipse per HitTestResult and position on the canvas
                var ellipse = new Ellipse()
                {
                    Width = markerSize,
                    Height = markerSize,
                    Fill = _line.Stroke,
                    IsHitTestVisible = false,
                    Tag = typeof(ChartRolloverModifier)
                };

                Canvas.SetLeft(ellipse, hitTestResult.HitTestPoint.X - markerSize * 0.5);
                Canvas.SetTop(ellipse, hitTestResult.HitTestPoint.Y - markerSize * 0.5);

                this.ModifierSurface.Children.Add(ellipse);

                if (true == ROIStatsChart.ViewModel.ChartViewModel.StatsCursorEnabled)
                {
                    string name;
                    int indx = hitTestResult.DataSeriesName.LastIndexOf("_Ar");
                    if (-1 < indx)
                    {
                        name = hitTestResult.DataSeriesName.Substring(0, indx);
                    }
                    else
                    {
                        name = hitTestResult.DataSeriesName;
                    }

                    // Create one label per HitTestResult and position on the canvas
                    var text = new Border()
                    {
                        IsHitTestVisible = false,
                        BorderBrush = TextForeground,
                        BorderThickness = new Thickness(3),
                        Background = LineBrush,
                        CornerRadius = new CornerRadius(2, 2, 2, 2),
                        Tag = typeof(ChartRolloverModifier),
                        Child = new TextBlock()
                        {
                            Text = string.Format("{0}: {1}, {2}: {3}", XAxis.AxisTitle, (Convert.ToDouble(hitTestResult.XValue) / 1000.0).ToString("#.000"), name, YAxis.FormatText(hitTestResult.YValue)),
                            FontSize = 12,
                            Margin = new Thickness(3),
                            Foreground = (Brush)ChartLineProperty.GetLineColor(hitTestResult.DataSeriesName, typeof(Brush)),
                        }
                    };

                    Canvas.SetLeft(text, hitTestResult.HitTestPoint.X + 5);
                    Canvas.SetTop(text, hitTestResult.HitTestPoint.Y - 5);

                    this.ModifierSurface.Children.Add(text);
                }
            }
        }

        private static void CreateLine(ChartRolloverModifier modifier)
        {
            modifier._line = new Line() { Stroke = modifier.LineBrush, StrokeThickness = 4, IsHitTestVisible = false, Tag = typeof(ChartRolloverModifier) };
        }

        private static void OnLineBrushChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            var modifier = d as ChartRolloverModifier;
            CreateLine(modifier);
        }

        private void ClearModifierSurface()
        {
            for (int i = ParentSurface.ModifierSurface.Children.Count - 1; i >= 0; --i)
            {
                if ((System.Type)(((FrameworkElement)ParentSurface.ModifierSurface.Children[i]).Tag) == typeof(ChartRolloverModifier))
                {
                    ParentSurface.ModifierSurface.Children.RemoveAt(i);
                }
            }
        }

        #endregion Methods
    }

    // To create a LabelProvider for a NumericAxis or Log Axis, inherit NumericLabelProvider
    public class CustomNumericLabelProvider : NumericLabelProvider
    {
        #region Methods

        public override string FormatLabel(IComparable dataValue)
        {
            // Note: Implement as you wish, converting Data-Value to string
            return (((double)dataValue) / 1000.0).ToString();
        }

        #endregion Methods
    }

    /// <summary>
    /// Implementation of MouseWheelZoomModifier 
    /// Defines custom behaviours when using the mousewheel in XAML where declare <MouseWheelZoomModifier/>
    /// </summary>
    public class MouseWheelZoomCustomModifier : MouseWheelZoomModifier
    {
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
                    this.XyDirection = XyDirection.XYDirection;
                    this.ActionType = ActionType.Zoom;
                    base.OnModifierMouseWheel(e);
                    break;
            }
        }

        #endregion Methods
    }
}