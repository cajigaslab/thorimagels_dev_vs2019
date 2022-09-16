namespace RealTimeLineChart.View
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Runtime.CompilerServices;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Animation;
    using System.Windows.Media.Imaging;
    using System.Windows.Navigation;
    using System.Windows.Shapes;

    using Microsoft.Win32;

    using RealTimeLineChart.ViewModel;

    using SciChart;
    using SciChart.Charting;
    using SciChart.Charting.ChartModifiers;
    using SciChart.Charting.Common.Extensions;
    using SciChart.Charting.Model.DataSeries;
    using SciChart.Charting.Themes;
    using SciChart.Charting.Visuals;
    using SciChart.Charting.Visuals.Annotations;
    using SciChart.Charting.Visuals.Axes;
    using SciChart.Charting.Visuals.Axes.LabelProviders;
    using SciChart.Charting.Visuals.Events;
    using SciChart.Core;
    using SciChart.Data.Model;
    using SciChart.Drawing.HighSpeedRasterizer;
    using SciChart.Drawing.VisualXcceleratorRasterizer;

    /// <summary>
    /// Interaction logic for SciChartView.xaml
    /// </summary>
    public partial class SciChartView : UserControl
    {
        #region Fields

        public long _index = 0;

        #endregion Fields

        #region Constructors

        /// <summary>
        /// Initializes a new instance of the <see cref="SciChartView"/> class.
        /// </summary>
        public SciChartView()
        {
            InitializeComponent();
            this.Loaded += SciChartView_Loaded;
            this.Unloaded += SciChartView_Unloaded;
        }

        #endregion Constructors

        #region Methods

        /// <summary>
        /// Invoked when an unhandled <see cref="E:System.Windows.Input.Mouse.MouseMove" /> attached event reaches an element in its route that is derived from this class. Implement this method to add class handling for this event.
        /// </summary>
        /// <param name="e">The <see cref="T:System.Windows.Input.MouseEventArgs" /> that contains the event data.</param>
        protected override void OnMouseMove(MouseEventArgs e)
        {
            RealTimeLineChartViewModel vm = (RealTimeLineChartViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            base.OnMouseMove(e);
            var selectedSlices = sliceModifier.VerticalLines.Where(annotation => annotation.IsSelected).ToList();

            foreach (var slice in selectedSlices)
            {
                int chosenIndex = sliceModifier.VerticalLines.IndexOf(slice);
                vm.ReviseVerticalMarker(chosenIndex, Math.Round((double)slice.X1, 6));
            }
            vm.ChangeVerticalMarkersMode(RealTimeLineChartViewModel.MarkerType.Save);
        }

        private void channelSurface_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            if (stackTimeAxisSurface.Width != e.NewSize.Width)
            {
                stackTimeAxisSurface.Width = (sender as SciChart.Charting.Visuals.SciChartSurface).ActualWidth;
            }
        }

        private void GridSplitter_MouseEnter(object sender, MouseEventArgs e)
        {
            Mouse.OverrideCursor = Cursors.SizeNS;
        }

        private void GridSplitter_MouseLeave(object sender, MouseEventArgs e)
        {
            Mouse.OverrideCursor = Cursors.Arrow;
        }

        private void NumericAxis_VisibleRangeChanged(object sender, VisibleRangeChangedEventArgs e)
        {
            RealTimeLineChartViewModel vm = (RealTimeLineChartViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            vm.XVisibleRangeStack = e.NewVisibleRange;
        }

        /// <summary>
        /// redirect mouse event from AnnotationLabel to respective VerticalLineAnnotation.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="MouseEventArgs"/> instance containing the event data.</param>
        /// <param name="slice">VerticalLineAnnotation instance for locating index.</param>
        void RedirectToVerticalLineAnnotation(object sender, MouseEventArgs e, VerticalLineAnnotation slice)
        {
            int index = this.sliceModifier.VerticalLines.IndexOf(slice);
            if (0 <= index)
                this.sliceModifier.VerticalLines[index].RaiseEvent(e);
        }

        /// <summary>
        /// Saves this instance.
        /// </summary>
        private void Save()
        {
            var saveFileDialog = new SaveFileDialog
            {
                Filter = "Png|*.png|Jpeg|*.jpeg|Bmp|*.bmp",
                InitialDirectory = Environment.GetFolderPath(Environment.SpecialFolder.Desktop)
            };

            if (saveFileDialog.ShowDialog() == true)
            {
                var exportType = (ExportType)saveFileDialog.FilterIndex - 1;
                // Saving chart to file with specified file format
                this.sciChartSurface.ExportToFile(saveFileDialog.FileName, exportType, true);
            }
        }

        private void SciChartSurface_IsVisibleChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            if (this.sciChartSurface.IsVisible)
                this.sciChartSurface.InvalidateElement();
        }

        private void sciChartSurface_MouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            RealTimeLineChartViewModel vm = (RealTimeLineChartViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            if ((int)RealTimeLineChartViewModel.ChartModes.REVIEW == vm.ChartMode && e.ChangedButton == MouseButton.Left)
            {
                this.sciChartSurface.YAxis.VisibleRange.Min = vm.YDataRange.Min;
                this.sciChartSurface.YAxis.VisibleRange.Max = vm.YDataRange.Max;

                this.sciChartSurface.XAxis.VisibleRange.Min = vm.XDataRange.Min;
                this.sciChartSurface.XAxis.VisibleRange.Max = vm.XDataRange.Max;

                vm.XVisibleRangeChartMin = (double)this.sciChartSurface.XAxis.VisibleRange.Min;
                vm.XVisibleRangeChartMax = (double)this.sciChartSurface.XAxis.VisibleRange.Max;
            }
        }

        /// <summary>
        /// Handles the Loaded event of the SciChartView control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        void SciChartView_Loaded(object sender, RoutedEventArgs e)
        {
            RealTimeLineChartViewModel vm = (RealTimeLineChartViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            vm.ChartViewSize = new double[2] { this.ActualWidth, this.ActualHeight };
            vm.EventVerticalMarkerModeChanged += vm_VerticalMarkerEvent;
            vm.EventVerticalMarkerSelectedIndexChanged += vm_VerticalMarkerSelectedEvent;
            vm.EventPrintScreen += vm_EventPrintScreen;
            vm.EventXYAxisScrollbarEnabledChanged += vm_EventScrollbar;
            vm.EventChartYAxisRangeChanged += vm_EventChartYAxisChanged;
            yAxis.VisibleRangeChanged += yAxis_VisibleRangeChanged;
            vm._visibleYAxisMin = (double)this.sciChartSurface.YAxis.VisibleRange.Min;
            vm._visibleYAxisMax = (double)this.sciChartSurface.YAxis.VisibleRange.Max;
            vm.UpdateRenderPriority += Vm_UpdateRenderPriority;
            var mode = AxisDragModes.Scale;
            yAxisRightDragmodifier.DragMode = mode;
            xAxisDragModifier.DragMode = mode;
            vm.MainChartSurface = sciChartSurface;
        }

        /// <summary>
        /// Handles the Unloaded event of the SciChartView control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        void SciChartView_Unloaded(object sender, RoutedEventArgs e)
        {
            RealTimeLineChartViewModel vm = (RealTimeLineChartViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            vm.EventVerticalMarkerModeChanged -= vm_VerticalMarkerEvent;
            vm.EventVerticalMarkerSelectedIndexChanged -= vm_VerticalMarkerSelectedEvent;
            vm.EventPrintScreen -= vm_EventPrintScreen;
            vm.EventXYAxisScrollbarEnabledChanged -= vm_EventScrollbar;
            vm.EventChartYAxisRangeChanged -= vm_EventChartYAxisChanged;
            yAxis.VisibleRangeChanged -= yAxis_VisibleRangeChanged;
        }

        /// <summary>
        /// Handles the MouseDoubleClick event of the slice control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="MouseButtonEventArgs"/> instance containing the event data.</param>
        void slice_MouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            RealTimeLineChartViewModel vm = (RealTimeLineChartViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            if ((sender as VerticalLineAnnotation).ToolTip != null)
            {
                vm.VerticalMarkerTooltipText = (sender as VerticalLineAnnotation).ToolTip.ToString();
            }
            else
            {
                vm.VerticalMarkerTooltipText = string.Empty;
            }
            string text = vm.AddCommentsOptions();
            if (text != string.Empty)
            {
                (sender as VerticalLineAnnotation).ToolTip = text;
                int chosenIndex = sliceModifier.VerticalLines.IndexOf((sender as VerticalLineAnnotation));
                vm.ReviseVerticalMarker(chosenIndex, text);
                vm.ChangeVerticalMarkersMode(RealTimeLineChartViewModel.MarkerType.Save);
            }

            if (this.Cursor != Cursors.Wait)
                Mouse.OverrideCursor = null;
            e.Handled = true;
        }

        /// <summary>
        /// Handles the MouseEnter event of the slice control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="MouseEventArgs"/> instance containing the event data.</param>
        void slice_MouseEnter(object sender, MouseEventArgs e)
        {
            //throw new NotImplementedException();
            if (this.Cursor != Cursors.Wait)
                Mouse.OverrideCursor = Cursors.SizeWE;
            e.Handled = true;
        }

        /// <summary>
        /// Handles the MouseLeave event of the slice control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="MouseEventArgs"/> instance containing the event data.</param>
        void slice_MouseLeave(object sender, MouseEventArgs e)
        {
            //throw new NotImplementedException();
            if (this.Cursor != Cursors.Wait)
                Mouse.OverrideCursor = Cursors.Arrow;
            e.Handled = true;
        }

        private void stackX_DataRangeChanged(object sender, EventArgs e)
        {
            if ((sender as NumericAxis).IsVisible && (sender as NumericAxis).DataRange != null) //TODO: check if there is data in that axis in particular
            {
                stackedViewXScrollbar.Axis = sender as NumericAxis;
            }
        }

        private void Stack_PreviewMouseWheel(object sender, MouseWheelEventArgs e)
        {
            int increment = 10;

            if (null == (ListBox)sender)
                return;

            if (null == ((ListBox)sender).Items)
                return;

            if (0 == ((ListBox)sender).Items.Count)
                return;

            if ((Keyboard.IsKeyDown(Key.RightAlt) || Keyboard.IsKeyDown(Key.LeftAlt))
                && (Keyboard.IsKeyDown(Key.LeftCtrl) || Keyboard.IsKeyDown(Key.RightCtrl)))
            {
                if (typeof(SpectralViewModel) == ((ListBox)sender).Items[0].GetType())
                {
                    foreach (SpectralViewModel item in lbAnalysisStack.Items)
                    {
                        item.Height = (0 > e.Delta) ? item.Height - increment : item.Height + increment;
                    }
                }
                else if (typeof(ChannelViewModel) == ((ListBox)sender).Items[0].GetType())
                {
                    //foreach (ChannelViewModel item in lbDataStack.Items)
                    //{
                    //    item.Height = (0 > e.Delta) ? item.Height - increment : item.Height + increment;
                    //}
                }
            }
        }

        /// <summary>
        /// VM_s the event chart y axis changed.
        /// </summary>
        /// <param name="minY">The minimum y.</param>
        /// <param name="maxY">The maximum y.</param>
        void vm_EventChartYAxisChanged(double minY, double maxY)
        {
            Dispatcher.Invoke(new Action(() =>
            {
                this.sciChartSurface.YAxis.VisibleRange.SetMinMax(minY, maxY);
                this.dummySurface.YAxis.VisibleRange.SetMinMax(minY, maxY);
            }));
        }

        /// <summary>
        /// VM_s the event print screen.
        /// </summary>
        void vm_EventPrintScreen()
        {
            Save();
        }

        /// <summary>
        /// VM_s the event scrollbar.
        /// </summary>
        /// <param name="isScrollbarEnabled">if set to <c>true</c> [is scrollbar enabled].</param>
        void vm_EventScrollbar(bool isScrollbarEnabled)
        {
            scichartMainXScrollbar.Visibility = isScrollbarEnabled ? Visibility.Visible : Visibility.Collapsed;
            scichartMainYScrollbar.Visibility = isScrollbarEnabled ? Visibility.Visible : Visibility.Collapsed;
            dummySurface.Visibility = isScrollbarEnabled ? Visibility.Visible : Visibility.Collapsed;
        }

        private void Vm_UpdateRenderPriority(RenderPriority renderPriority)
        {
            sciChartSurface.RenderPriority = renderPriority;
        }

        /// <summary>
        /// VM_s the vertical market event.
        /// </summary>
        /// <param name="markerType">Type of the marker.</param>
        void vm_VerticalMarkerEvent(int markerType, double typeValue)
        {
            Dispatcher.Invoke(new Action(() =>
            {
                RealTimeLineChartViewModel vm = (RealTimeLineChartViewModel)this.DataContext;
                if (vm == null)
                {
                    return;
                }

                RealTimeLineChartViewModel.MarkerType type = (RealTimeLineChartViewModel.MarkerType)markerType;
                switch (type)
                {
                    case RealTimeLineChartViewModel.MarkerType.Add:
                        {
                            if (typeValue > 0 && this.sciChartSurface.XAxis.VisibleRange.IsValueWithinRange(typeValue) == false)
                            {
                                MessageBoxResult messageBoxResult = MessageBoxResult.None;
                                MessageBox.Show("Position is Out of Visible Range, Would you want to continue?", "Warning", MessageBoxButton.YesNo, MessageBoxImage.Warning, messageBoxResult);
                                if (messageBoxResult == MessageBoxResult.No)
                                {
                                    return;
                                }
                            }
                            var slice = new VerticalLineAnnotation()
                            {
                                X1 = (typeValue < 0) ? this.sciChartSurface.XAxis.GetDataValue(this.sciChartSurface.ActualWidth / 2) : typeValue,
                                ShowLabel = true,
                                Stroke = new BrushConverter().ConvertFromString("#427EF6") as SolidColorBrush,
                                StrokeThickness = 2,
                                IsEditable = true,
                                LabelPlacement = LabelPlacement.Axis
                            };
                            this.sliceModifier.VerticalLines.Add(slice);
                            vm.CreateVerticalMarker(_index, Math.Round((double)slice.X1, 6));
                            slice.MouseDoubleClick += slice_MouseDoubleClick;
                            if (0 < slice.AnnotationLabels.Count)
                            {
                                slice.AnnotationLabels[0].MouseMove += new MouseEventHandler((sender, e) => RedirectToVerticalLineAnnotation(sender, e, slice));
                                slice.AnnotationLabels[0].MouseUp += new MouseButtonEventHandler((sender, e) => RedirectToVerticalLineAnnotation(sender, e, slice));
                                slice.AnnotationLabels[0].MouseDown += new MouseButtonEventHandler((sender, e) => RedirectToVerticalLineAnnotation(sender, e, slice));
                            }
                            _index++;
                        }
                        break;
                    case RealTimeLineChartViewModel.MarkerType.Delete:
                        {
                            var selectedSlices = sliceModifier.VerticalLines.Where(annotation => annotation.IsSelected).ToList();

                            foreach (var slice in selectedSlices)
                            {
                                int deleteIndex = sliceModifier.VerticalLines.IndexOf(slice);
                                this.sciChartSurface.Annotations.Remove(slice);
                                slice.MouseDoubleClick -= slice_MouseDoubleClick;
                                if (0 < slice.AnnotationLabels.Count)
                                {
                                    slice.AnnotationLabels[0].MouseMove -= new MouseEventHandler((sender, e) => RedirectToVerticalLineAnnotation(sender, e, slice));
                                    slice.AnnotationLabels[0].MouseUp -= new MouseButtonEventHandler((sender, e) => RedirectToVerticalLineAnnotation(sender, e, slice));
                                    slice.AnnotationLabels[0].MouseDown -= new MouseButtonEventHandler((sender, e) => RedirectToVerticalLineAnnotation(sender, e, slice));
                                }
                                vm.DeleteVerticalMarker(deleteIndex);
                            }
                        }
                        break;
                    case RealTimeLineChartViewModel.MarkerType.DeleteAll:
                        {
                            var selectedSlices = sliceModifier.VerticalLines.ToList();

                            foreach (var slice in selectedSlices)
                            {
                                this.sciChartSurface.Annotations.Remove(slice);
                                slice.MouseDoubleClick -= slice_MouseDoubleClick;
                                if (0 < slice.AnnotationLabels.Count)
                                {
                                    slice.AnnotationLabels[0].MouseMove -= new MouseEventHandler((sender, e) => RedirectToVerticalLineAnnotation(sender, e, slice));
                                    slice.AnnotationLabels[0].MouseUp -= new MouseButtonEventHandler((sender, e) => RedirectToVerticalLineAnnotation(sender, e, slice));
                                    slice.AnnotationLabels[0].MouseDown -= new MouseButtonEventHandler((sender, e) => RedirectToVerticalLineAnnotation(sender, e, slice));
                                }
                            }
                            vm._verticalmarker.Clear();
                            _index = 0;
                        }
                        break;
                    case RealTimeLineChartViewModel.MarkerType.HideAll:
                        {
                            var selectedSlices = sliceModifier.VerticalLines.ToList();

                            foreach (var slice in selectedSlices)
                            {
                                slice.IsHidden = true;
                                slice.ShowLabel = false;
                            }
                        }
                        break;
                    case RealTimeLineChartViewModel.MarkerType.DisplayAll:
                        {
                            var selectedSlices = sliceModifier.VerticalLines.ToList();

                            foreach (var slice in selectedSlices)
                            {
                                slice.IsHidden = false;
                                slice.ShowLabel = true;
                            }
                        }
                        break;
                    case RealTimeLineChartViewModel.MarkerType.Save:
                        {
                            if (vm._coordinationVerticalMarkers != null)
                            {
                                vm._coordinationVerticalMarkers.Clear();
                                vm._tooltipVerticalMarkers.Clear();
                            }
                            else
                            {
                                vm._coordinationVerticalMarkers = new List<double> { };
                                vm._tooltipVerticalMarkers = new List<string> { };
                            }
                            string fileName = vm.SavePath + "\\" + vm.SaveName;
                            var selectedSlices = sliceModifier.VerticalLines.ToList();
                            for (int i = 0; i < selectedSlices.Count; i++)
                            {
                                vm._coordinationVerticalMarkers.Add(Math.Round((double)selectedSlices[i].X1, 6));
                                if (selectedSlices[i].ToolTip != null)
                                {
                                    vm._tooltipVerticalMarkers.Add(selectedSlices[i].ToolTip.ToString());
                                }
                                else
                                {
                                    vm._tooltipVerticalMarkers.Add(string.Empty);
                                }
                            }
                            if ((selectedSlices.Count > 0) || (0 == typeValue)) //typeValue [0]: create file anyway, otherwise: check counts.
                            {
                                vm.CreateXMLVerticalMarkers(fileName, vm._coordinationVerticalMarkers, vm._tooltipVerticalMarkers);
                            }
                        }
                        break;
                    case RealTimeLineChartViewModel.MarkerType.Load:
                        {
                            string fileName = vm.SavePath + "\\" + vm.SaveName + "\\VerticalMarkers.xml";
                            if (vm._coordinationVerticalMarkers != null)
                            {
                                vm._coordinationVerticalMarkers.Clear();
                                vm._tooltipVerticalMarkers.Clear();
                            }
                            else
                            {
                                vm._coordinationVerticalMarkers = new List<double> { };
                                vm._tooltipVerticalMarkers = new List<string> { };
                            }
                            vm.LoadXMLVerticalMarkers(fileName, vm._coordinationVerticalMarkers, vm._tooltipVerticalMarkers);
                            _index = 0;
                            for (int i = 0; i < vm._coordinationVerticalMarkers.Count; i++)
                            {
                                var slice = new VerticalLineAnnotation()
                                {
                                    X1 = vm._coordinationVerticalMarkers[i],
                                    ShowLabel = true,
                                    Stroke = new BrushConverter().ConvertFromString("#427EF6") as SolidColorBrush,
                                    StrokeThickness = 2,
                                    IsEditable = true,
                                    LabelPlacement = LabelPlacement.Axis
                                };
                                if (vm._tooltipVerticalMarkers.Count != 0)
                                {
                                    if (vm._tooltipVerticalMarkers[i] != string.Empty)
                                    {
                                        slice.ToolTip = vm._tooltipVerticalMarkers[i];
                                    }
                                    vm.CreateVerticalMarker(_index, (double)slice.X1, vm._tooltipVerticalMarkers[i]);
                                }
                                else
                                {
                                    vm.CreateVerticalMarker(_index, (double)slice.X1);
                                }
                                this.sliceModifier.VerticalLines.Add(slice);
                                slice.MouseDoubleClick += slice_MouseDoubleClick;
                                if (0 < slice.AnnotationLabels.Count)
                                {
                                    slice.AnnotationLabels[0].MouseMove += new MouseEventHandler((sender, e) => RedirectToVerticalLineAnnotation(sender, e, slice));
                                    slice.AnnotationLabels[0].MouseUp += new MouseButtonEventHandler((sender, e) => RedirectToVerticalLineAnnotation(sender, e, slice));
                                    slice.AnnotationLabels[0].MouseDown += new MouseButtonEventHandler((sender, e) => RedirectToVerticalLineAnnotation(sender, e, slice));
                                }
                                _index++;
                            }
                        }
                        break;
                    default:
                        break;
                }
            }));
        }

        /// <summary>
        /// VM_s the vertical marker selected event.
        /// </summary>
        /// <param name="selectedIndex">Index of the selected.</param>
        void vm_VerticalMarkerSelectedEvent(int selectedIndex)
        {
            if (0 > selectedIndex)
                return;
            double diff = (double)this.sciChartSurface.XAxis.GetDataValue(this.sciChartSurface.ActualWidth) - (double)this.sciChartSurface.XAxis.GetDataValue(0);
            this.sciChartSurface.XAxis.VisibleRange.SetMinMax((double)sliceModifier.VerticalLines[selectedIndex].X1 - diff / 2, (double)sliceModifier.VerticalLines[selectedIndex].X1 + diff / 2);

            var switchOffAnimation = new DoubleAnimation
            {
                To = 0,
                Duration = TimeSpan.Zero
            };

            var switchOnAnimation = new DoubleAnimation
            {
                To = 1,
                Duration = TimeSpan.Zero,
                BeginTime = TimeSpan.FromMilliseconds(200)
            };

            var blinkStoryboard = new Storyboard
            {
                Duration = TimeSpan.FromMilliseconds(400),
                RepeatBehavior = new RepeatBehavior(5.0),
            };

            Storyboard.SetTarget(switchOffAnimation, sliceModifier.VerticalLines[selectedIndex]);
            Storyboard.SetTargetProperty(switchOffAnimation, new PropertyPath(VerticalLineAnnotation.OpacityProperty));
            blinkStoryboard.Children.Add(switchOffAnimation);

            Storyboard.SetTarget(switchOnAnimation, sliceModifier.VerticalLines[selectedIndex]);
            Storyboard.SetTargetProperty(switchOnAnimation, new PropertyPath(VerticalLineAnnotation.OpacityProperty));
            blinkStoryboard.Children.Add(switchOnAnimation);

            sliceModifier.VerticalLines[selectedIndex].BeginStoryboard(blinkStoryboard);
        }

        /// <summary>
        /// Handles the VisibleRangeChanged event of the yAxis control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="VisibleRangeChangedEventArgs"/> instance containing the event data.</param>
        void yAxis_VisibleRangeChanged(object sender, VisibleRangeChangedEventArgs e)
        {
            RealTimeLineChartViewModel vm = (RealTimeLineChartViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }

            if (vm.ChartMode == (int)RealTimeLineChartViewModel.ChartModes.REVIEW)
            {
                vm._visibleYAxisMin = (double)this.sciChartSurface.YAxis.VisibleRange.Min;
                vm._visibleYAxisMax = (double)this.sciChartSurface.YAxis.VisibleRange.Max;
                vm.VisibleYAxisMin = (double)this.sciChartSurface.YAxis.VisibleRange.Min;
                vm.VisibleYAxisMax = (double)this.sciChartSurface.YAxis.VisibleRange.Max;
                vm.XVisibleRangeChartMin = (double)this.sciChartSurface.XAxis.VisibleRange.Min;
                vm.XVisibleRangeChartMax = (double)this.sciChartSurface.XAxis.VisibleRange.Max;
            }
        }

        #endregion Methods

        #region Other

        //private void OnDirectXInitializationFailed(object sender, DXErrorEventArgs e)
        //{
        //    MessageBox.Show("DirectX Initialization Failed. "
        //                 + "Downgrading to Software. See inner exception for details: "
        //                 + e.Exception.Message);
        //    this.sciChartSurface.RenderSurface = new  HighSpeedRenderSurface();
        //}
        //private void OnDirectXRenderingFailed(object sender, DXErrorEventArgs e)
        //{
        //    MessageBox.Show("DirectX Initialization Failed. "
        //                 + "Downgrading to Software. See inner exception for details: "
        //                 + e.Exception.Message);
        //    this.sciChartSurface.RenderSurface = new HighSpeedRenderSurface();
        //}

        #endregion Other
    }
}