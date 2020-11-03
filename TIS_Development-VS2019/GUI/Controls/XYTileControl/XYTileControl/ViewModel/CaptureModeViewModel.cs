namespace XYTileControl.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.IO;
    using System.Linq;
    using System.Text;
    using System.Text.RegularExpressions;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Controls.Primitives;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Navigation;
    using System.Windows.Shapes;
    using System.Xml;

    using Abt.Controls.SciChart;
    using Abt.Controls.SciChart.Model.DataSeries;
    using Abt.Controls.SciChart.Utility;
    using Abt.Controls.SciChart.Visuals;
    using Abt.Controls.SciChart.Visuals.Annotations;

    using ThorLogging;

    using ThorSharedTypes;

    /// <summary>
    /// Capture
    /// </summary>
    class CaptureModeViewModel : XYtileViewModel
    {
        #region Fields

        Carrier presetCarrier = new Carrier() { Name = "Slide", Type = CarrierType.Slide, Width = 75, Height = 25, Template = new WellPlateTemplate { CenterToCenterX = 0, CenterToCenterY = 0, Col = 0, Row = 0, TopLeftCenterOffsetX = 0, TopLeftCenterOffsetY = 0, Diameter = 0 } };

        // Create temporary AnnotationCollection
        XYTileDisplay xyTileControl;
        bool _loaded;
        BoxAnnotation _scanArea;

        #endregion Fields

        #region Constructors

        public CaptureModeViewModel(XYTileDisplay xyTileControl)
        {
            this.xyTileControl = xyTileControl;
        }

        #endregion Constructors

        #region Methods

        /// <summary>
        /// Get the total number of enabled tiles in all wells
        /// </summary>
        /// <returns> The total number of enabled tiles </returns>
        public int GetTotalNumberTiles()
        {
            int total = 0;

            for (int i = 0; i < xyTileControl.XYtableData.Count; i++)
            {
                if (xyTileControl.XYtableData[i].IsEnabled)
                {
                    XYPosition tile = xyTileControl.XYtableData[i];
                    total += Convert.ToInt32(tile.TileCol) * Convert.ToInt32(tile.TileRow);
                }
            }

            return total;
        }

        /// <summary>
        /// Moves to the next tile.
        /// </summary>
        public void MoveToNextTile()
        {
            //// not used
        }

        /// <summary>
        /// Moves to the previous tile.
        /// </summary>
        public void MoveToPreviousTile()
        {
            //// not used
        }

        public void ResetSample()
        {
            // not use because sample wont be changed
        }

        /// <summary>
        /// Resets the sample home location.
        /// </summary>
        public void ResetSampleHomeLocation()
        {
            ResetSampleInScichart();
        }

        /// <summary>
        /// Resets the sample in scichart.
        /// </summary>
        public void ResetSampleInScichart()
        {
            if (_loaded == false) return;
            // Create temporary AnnotationCollection
            AnnotationCollection tempAnnotations = new AnnotationCollection();
            // Load Scan Area
            tempAnnotations.Add(_scanArea);
            if (xyTileControl.CurrentCarrier == null)
            {
                xyTileControl.CurrentCarrier = xyTileControl.SampleCarrierCollection[0];
            }
            Point CarrierTopLeftAnchorPoint = new Point(xyTileControl.HomePosX - xyTileControl.CurrentCarrier.Template.TopLeftCenterOffsetX, xyTileControl.HomePosY + xyTileControl.CurrentCarrier.Template.TopLeftCenterOffsetY);
            BoxAnnotation Carrier = new BoxAnnotation
            {
                Background = Brushes.LightBlue,
                BorderBrush = Brushes.DarkBlue,
                CornerRadius = new CornerRadius(1),
                Opacity = 0.6,
                X1 = CarrierTopLeftAnchorPoint.X,
                Y1 = CarrierTopLeftAnchorPoint.Y,
                X2 = CarrierTopLeftAnchorPoint.X + xyTileControl.CurrentCarrier.Width,
                Y2 = CarrierTopLeftAnchorPoint.Y - xyTileControl.CurrentCarrier.Height,
            };
            tempAnnotations.Add(Carrier);
            //create Wells
            if (xyTileControl.CurrentCarrier.Type != CarrierType.Slide)
            {
                for (int i = 0; i < xyTileControl.CurrentCarrier.Template.Col; i++)
                {
                    for (int j = 0; j < xyTileControl.CurrentCarrier.Template.Row; j++)
                    {
                        BoxAnnotation well = new BoxAnnotation()
                        {
                            Opacity = 0.5,
                            Background = Brushes.AliceBlue,
                            Tag = xyTileControl.TagWells(j, i),
                            ToolTip = xyTileControl.TagWells(j, i),
                        };
                        if (xyTileControl.CurrentCarrier.Template.Shape == WellShape.CircleWell)
                        {
                            well.Style = (Style)xyTileControl.FindResource("AnnotationEllipse");
                            well.X1 = xyTileControl.HomePosX - xyTileControl.CurrentCarrier.Template.Diameter / 2 + i * xyTileControl.CurrentCarrier.Template.CenterToCenterX;
                            well.Y1 = xyTileControl.HomePosY + xyTileControl.CurrentCarrier.Template.Diameter / 2 - j * xyTileControl.CurrentCarrier.Template.CenterToCenterY;
                            well.X2 = xyTileControl.HomePosX + xyTileControl.CurrentCarrier.Template.Diameter / 2 + i * xyTileControl.CurrentCarrier.Template.CenterToCenterX;
                            well.Y2 = xyTileControl.HomePosY - xyTileControl.CurrentCarrier.Template.Diameter / 2 - j * xyTileControl.CurrentCarrier.Template.CenterToCenterY;
                        }
                        else if (xyTileControl.CurrentCarrier.Template.Shape == WellShape.RectangleWell)
                        {
                            double width = (xyTileControl.CurrentCarrier.Type == CarrierType.Multislide) ? xyTileControl.CurrentCarrier.Template.Height : xyTileControl.CurrentCarrier.Template.Width;
                            double height = (xyTileControl.CurrentCarrier.Type == CarrierType.Multislide) ? xyTileControl.CurrentCarrier.Template.Width : xyTileControl.CurrentCarrier.Template.Height;

                            well.X1 = xyTileControl.HomePosX - width / 2 + i * xyTileControl.CurrentCarrier.Template.CenterToCenterX;
                            well.Y1 = xyTileControl.HomePosY + height / 2 - j * xyTileControl.CurrentCarrier.Template.CenterToCenterY;
                            well.X2 = xyTileControl.HomePosX + width / 2 + i * xyTileControl.CurrentCarrier.Template.CenterToCenterX;
                            well.Y2 = xyTileControl.HomePosY - height / 2 - j * xyTileControl.CurrentCarrier.Template.CenterToCenterY;
                        }
                        tempAnnotations.Add(well);
                    }
                }
            }
            if (null != xyTileControl.XYtableData)
            {
                for (int k = 0; k < xyTileControl.XYtableData.Count; k++)
                {
                    XYPosition xyPosition = xyTileControl.XYtableData[k];
                    if (xyPosition.IsEnabled == false) continue;

                    double overlapX = Convert.ToDouble(xyPosition.OverlapX) / 100;
                    double overlapY = Convert.ToDouble(xyPosition.OverlapY) / 100;
                    double col = Convert.ToDouble(xyPosition.TileCol);
                    double row = Convert.ToDouble(xyPosition.TileRow);
                    XmlDocument appSettings = new XmlDocument();
                    appSettings.Load(ResourceManagerCS.GetApplicationSettingsFileString());
                    XmlNodeList ndList = appSettings.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/XYView");

                    if (ndList != null && ndList.Count > 0)
                    {
                        string s = string.Empty;
                        XmlManager.GetAttribute(ndList[0], appSettings, "showTileGrid", ref s);
                        xyTileControl.ShowGrid.IsChecked = (s.CompareTo("1") == 0) ? true : false;
                    }

                    if (true == xyTileControl.ShowGrid.IsChecked)
                    {
                        for (int i = 0; i < Convert.ToInt32(xyPosition.TileRow); i++)
                        {
                            for (int j = 0; j < Convert.ToInt32(xyPosition.TileCol); j++)
                            {
                                BoxAnnotation selectArea = new BoxAnnotation()
                                {
                                    Background = Brushes.Silver,
                                    X1 = Convert.ToDouble(xyPosition.X) + xyTileControl.HomePosX - (col - (col - 1) * overlapX) * xyTileControl.ScanAreaWidth / 2 + j * (1 - overlapX) * xyTileControl.ScanAreaWidth,
                                    Y1 = Convert.ToDouble(xyPosition.Y) + xyTileControl.HomePosY + (row - (row - 1) * overlapY) * xyTileControl.ScanAreaHeight / 2 - i * (1 - overlapY) * xyTileControl.ScanAreaHeight,
                                    X2 = Convert.ToDouble(xyPosition.X) + xyTileControl.HomePosX - (col - (col - 1) * overlapX) * xyTileControl.ScanAreaWidth / 2 + (j + 1 - j * overlapX) * xyTileControl.ScanAreaWidth,
                                    Y2 = Convert.ToDouble(xyPosition.Y) + xyTileControl.HomePosY + (row - (row - 1) * overlapY) * xyTileControl.ScanAreaHeight / 2 - (i + 1 - i * overlapY) * xyTileControl.ScanAreaHeight,
                                    BorderBrush = Brushes.Ivory,
                                    BorderThickness = new Thickness(1),
                                    Opacity = 0.8,
                                    Tag = xyPosition,
                                };
                                tempAnnotations.Add(selectArea);
                            }
                        }
                    }
                    else
                    {
                        BoxAnnotation selectArea = new BoxAnnotation()
                        {
                            Background = Brushes.Silver,
                            X1 = Convert.ToDouble(xyPosition.X) + xyTileControl.HomePosX - (col - (col - 1) * overlapX) * xyTileControl.ScanAreaWidth / 2,
                            Y1 = Convert.ToDouble(xyPosition.Y) + xyTileControl.HomePosY + (row - (row - 1) * overlapY) * xyTileControl.ScanAreaHeight / 2,
                            X2 = Convert.ToDouble(xyPosition.X) + xyTileControl.HomePosX + (col - (col - 1) * overlapX) * xyTileControl.ScanAreaWidth / 2,
                            Y2 = Convert.ToDouble(xyPosition.Y) + xyTileControl.HomePosY - (row - (row - 1) * overlapY) * xyTileControl.ScanAreaHeight / 2,
                            ContextMenu = (ContextMenu)xyTileControl.FindResource("TileContextMenu"),
                            BorderBrush = Brushes.Ivory,
                            BorderThickness = new Thickness(1),
                            Opacity = 0.8,
                            Tag = xyPosition,
                        };
                        //selectArea.PreviewMouseLeftButtonDown += selectArea_PreviewMouseLeftButtonDown;
                        tempAnnotations.Add(selectArea);
                    }
                }
            }
            xyTileControl.sciChart.Annotations = tempAnnotations;
        }

        /// <summary>
        /// Resets the sample view.
        /// </summary>
        public void ResetSampleView()
        {
            int numOfAnnotation = xyTileControl.sciChart.Annotations.Count;

            if (numOfAnnotation > 0 && xyTileControl.sciChart.XAxis.VisibleRange != null && xyTileControl.sciChart.YAxis.VisibleRange != null)
            {
                double minX = 0;
                double maxX = 0;
                double minY = 0;
                double maxY = 0;
                for (int i = 0, j = 0; i < numOfAnnotation; i++)
                {
                    BoxAnnotation item = xyTileControl.sciChart.Annotations[i] as BoxAnnotation;
                    if (item.Tag != null && (Convert.ToString(item.Tag)) == "Irremovable")
                    {
                        j++;
                        if (j == numOfAnnotation)// the irremovable boxannotation defined in the loaded function  = count of all annotation, which means no item in the canvas
                        {
                            return;
                        }
                        continue;
                    }
                    minX = Math.Min(minX, Convert.ToDouble(item.X1));
                    minY = Math.Min(minY, Convert.ToDouble(item.Y2));
                    maxX = Math.Max(maxX, Convert.ToDouble(item.X2));
                    maxY = Math.Max(maxY, Convert.ToDouble(item.Y1));
                }
                // To keep the displayed sharp, should keep the same zoom-scale of both X-axis and Y-axis.
                double xDiff = maxX - minX;
                double yDiff = maxY - minY;
                double xoffset = 0;
                double yoffset = 0;
                if (xDiff >= yDiff)
                {
                    xoffset = xDiff / 10;
                    yoffset = (xDiff * 6 / 5 - yDiff) / 2;
                }
                else
                {
                    yoffset = yDiff / 8;
                    xoffset = (yDiff * 6 / 5 - xDiff) / 2;
                }
                xyTileControl.sciChart.XAxis.VisibleRange.SetMinMax(minX - xoffset, maxX + xoffset);
                xyTileControl.sciChart.YAxis.VisibleRange.SetMinMax(minY - yoffset, maxY + yoffset);
            }
        }

        /// <summary>
        /// Rests the scan area view.
        /// </summary>
        public void RestScanAreaView()
        {
            if (_scanArea != null)
            {
                double offset = Math.Max(xyTileControl.ScanAreaWidth, xyTileControl.ScanAreaHeight);
                if (xyTileControl.sciChart.XAxis.VisibleRange == null)
                {
                    xyTileControl.sciChart.XAxis.VisibleRange = new DoubleRange(Convert.ToDouble(_scanArea.X1) - 8 * offset, Convert.ToDouble(_scanArea.X2) + 8 * offset);
                }
                else
                {
                    xyTileControl.sciChart.XAxis.VisibleRange.SetMinMax(Convert.ToDouble(_scanArea.X1) - 8 * offset, Convert.ToDouble(_scanArea.X2) + 8 * offset);
                }
                if (xyTileControl.sciChart.YAxis.VisibleRange == null)
                {
                    xyTileControl.sciChart.YAxis.VisibleRange = new DoubleRange(Convert.ToDouble(_scanArea.Y2) - 8 * offset, Convert.ToDouble(_scanArea.Y1) + 8 * offset);
                }
                else
                {
                    xyTileControl.sciChart.YAxis.VisibleRange.SetMinMax(Convert.ToDouble(_scanArea.Y2) - 8 * offset, Convert.ToDouble(_scanArea.Y1) + 8 * offset);
                }
            }
        }

        public void ShowTilesGrid(bool? isEnabled)
        {
            //not use
        }

        /// <summary>
        /// Xies the table_ add item.
        /// </summary>
        /// <param name="xytableData">The xytable data.</param>
        /// <param name="xyPosition">The xy position.</param>
        /// <param name="index">The index.</param>
        /// <param name="singleTile">if set to <c>true</c> [single tile].</param>
        public void XYTable_AddItem(ObservableCollection<XYPosition> xytableData, XYPosition xyPosition, int index = -1, bool singleTile = false)
        {
            if (xyPosition.IsEnabled == true)
            {
                xytableData.Add(xyPosition);
            }
        }

        public void XYTable_DeleteItem(ObservableCollection<XYPosition> xytableData, int tiledAreaIndex)
        {
            //No delete is needed in the capture.
        }

        public void XYTable_EditItem(ObservableCollection<XYPosition> xytableData, int selectedRow, int selectedColumn, object newValue)
        {
            //No editting is needed in the capture.
        }

        public void XYTable_SelectItem(ObservableCollection<XYPosition> xytableData, int index)
        {
            //no use
        }

        /// <summary>
        /// xes the ytile view model_ loaded.
        /// </summary>
        public void XYtileViewModel_Loaded()
        {
            if (_loaded == true) return;

            xyTileControl.SampleCarrierCollection = new ObservableCollection<Carrier>();
            xyTileControl.IsControlPanelVisible = Visibility.Collapsed;
            xyTileControl.IsTileTableVisible = Visibility.Collapsed;
            xyTileControl.IsDataGridReadOnly = true;
            xyTileControl.Control.Width = 200;
            xyTileControl.sciChart.Width = 200;
            xyTileControl.sciChart.Height = 200;
            xyTileControl.sciChart.HorizontalAlignment = HorizontalAlignment.Left;
            xyTileControl.IsChartDragSelectModifierEnabled = false;
            xyTileControl.XPositionChanged += xyTileControl_XPositionChanged;
            xyTileControl.YPositionChanged += xyTileControl_YPositionChanged;

            xyTileControl.ZoomPanModifier.IsEnabled = false;
            xyTileControl.MouseWheelZoomModifier.IsEnabled = false;
            xyTileControl.PinchZoomModifier.IsEnabled = false;

            //Define the data in scichart in case of the gird of scichart scales automatically without data. This dataSeries lines are transparent.
            var dataSeries0 = new XyDataSeries<double, double> { };
            var dataSeries1 = new XyDataSeries<double, double> { AcceptsUnsortedData = true };
            for (double i = -10000; i < 10000; i++)
            {
                dataSeries0.Append(i, 10000);
                dataSeries1.Append(10000, i);
            }
            xyTileControl.sciChart.RenderableSeries[0].DataSeries = dataSeries0;
            xyTileControl.sciChart.RenderableSeries[1].DataSeries = dataSeries1;

            //Define the scan Area.
            _scanArea = new BoxAnnotation()
            {
                Background = Brushes.RoyalBlue,
                BorderBrush = Brushes.DarkBlue,
                CornerRadius = new CornerRadius(1),
                Opacity = 0.8,
                X1 = xyTileControl.ScanAreaXPosition - xyTileControl.ScanAreaWidth / 2,
                X2 = xyTileControl.ScanAreaXPosition + xyTileControl.ScanAreaWidth / 2,
                Y1 = xyTileControl.ScanAreaYPosition - xyTileControl.ScanAreaHeight / 2,
                Y2 = xyTileControl.ScanAreaYPosition + xyTileControl.ScanAreaHeight / 2,
                Tag = "Irremovable",
                ToolTip = "Scan Area",
                IsHitTestVisible = false,
            };
            //Set the scan area to the toppest of canvas to make sure no other objects cover it.
            Canvas.SetZIndex(_scanArea, 9999);

            //Load Active.XML
            LoadExperimentSettings(xyTileControl.ActiveXML, xyTileControl.SampleCarrierCollection);
            _loaded = true;

            ResetSampleInScichart();
            RestScanAreaView();
        }

        /// <summary>
        /// xes the ytile view model_ unloaded.
        /// </summary>
        public void XYtileViewModel_Unloaded()
        {
            xyTileControl.XPositionChanged -= xyTileControl_XPositionChanged;
            xyTileControl.YPositionChanged -= xyTileControl_YPositionChanged;

            _loaded = false;
        }

        /// <summary>
        /// Determines whether [is scan area out of sight].
        /// </summary>
        /// <returns></returns>
        private bool IsScanAreaOutOfSight()
        {
            bool isOutside = false;
            if (_scanArea != null && xyTileControl.sciChart.XAxis.VisibleRange != null && xyTileControl.sciChart.YAxis.VisibleRange != null)
            {
                double xAxisMin = Convert.ToDouble(xyTileControl.sciChart.XAxis.VisibleRange.Min);
                double yAxisMin = Convert.ToDouble(xyTileControl.sciChart.YAxis.VisibleRange.Min);
                double xAxisMax = Convert.ToDouble(xyTileControl.sciChart.XAxis.VisibleRange.Max);
                double yAxisMax = Convert.ToDouble(xyTileControl.sciChart.YAxis.VisibleRange.Max);
                if (Convert.ToDouble(_scanArea.X2) <= xAxisMin || Convert.ToDouble(_scanArea.X1) >= xAxisMax || Convert.ToDouble(_scanArea.Y2) >= yAxisMax || Convert.ToDouble(_scanArea.Y1) <= yAxisMin)
                {
                    isOutside = true;
                }
            }
            return isOutside;
        }

        /// <summary>
        /// Loads the experiment settings.
        /// </summary>
        /// <param name="expXMLPath">The exp XML path.</param>
        /// <param name="sampleCarrierCollection">The sample carrier collection.</param>
        private void LoadExperimentSettings(String expXMLPath, ObservableCollection<Carrier> sampleCarrierCollection)
        {
            try
            {
                if (null == expXMLPath || string.Empty == expXMLPath || !File.Exists(expXMLPath))
                {
                    return;
                }
                XmlDocument expXML = new XmlDocument();
                ResourceManagerCS.BorrowDocMutexCS(SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS);
                expXML.Load(expXMLPath);
                ResourceManagerCS.ReturnDocMutexCS(SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS);

                sampleCarrierCollection.Add(presetCarrier);
                xyTileControl.SelectedCarrierIndex = 0;

                var sampleRootNode = expXML.SelectSingleNode("/ThorImageExperiment/Sample");
                if (sampleRootNode != null)
                {
                    string homeOffsetX = string.Empty;
                    string homeOffsetY = string.Empty;
                    string homeOffsetZ = string.Empty;

                    if (XmlManager.GetAttribute(sampleRootNode, expXML, "homeOffsetX", ref homeOffsetX))
                    {
                        xyTileControl.HomePosX = Convert.ToDouble(homeOffsetX);
                    }
                    if (XmlManager.GetAttribute(sampleRootNode, expXML, "homeOffsetY", ref homeOffsetY))
                    {
                        xyTileControl.HomePosY = Convert.ToDouble(homeOffsetY);
                    }
                    if (XmlManager.GetAttribute(sampleRootNode, expXML, "homeOffsetZ", ref homeOffsetZ))
                    {
                        xyTileControl.HomePosZ = Convert.ToDouble(homeOffsetZ);
                    }

                    string tempStr = string.Empty;

                    if (XmlManager.GetAttribute(sampleRootNode, expXML, "name", ref tempStr))
                    {
                        sampleCarrierCollection[0].Name = tempStr;
                    }
                    if (XmlManager.GetAttribute(sampleRootNode, expXML, "type", ref tempStr))
                    {
                        sampleCarrierCollection[0].Type = (CarrierType)System.Enum.Parse(typeof(CarrierType), tempStr);
                    }
                    if (XmlManager.GetAttribute(sampleRootNode, expXML, "width", ref tempStr))
                    {
                        sampleCarrierCollection[0].Width = Convert.ToDouble(tempStr);
                    }
                    if (XmlManager.GetAttribute(sampleRootNode, expXML, "height", ref tempStr))
                    {
                        sampleCarrierCollection[0].Height = Convert.ToDouble(tempStr);
                    }
                    if (XmlManager.GetAttribute(sampleRootNode, expXML, "row", ref tempStr))
                    {
                        sampleCarrierCollection[0].Template.Row = Convert.ToInt32(tempStr);
                    }
                    if (XmlManager.GetAttribute(sampleRootNode, expXML, "column", ref tempStr))
                    {
                        sampleCarrierCollection[0].Template.Col = Convert.ToInt32(tempStr);
                    }
                    if (XmlManager.GetAttribute(sampleRootNode, expXML, "diameter", ref tempStr))
                    {
                        sampleCarrierCollection[0].Template.Diameter = Convert.ToDouble(tempStr);
                    }
                    if (XmlManager.GetAttribute(sampleRootNode, expXML, "centerToCenterX", ref tempStr))
                    {
                        sampleCarrierCollection[0].Template.CenterToCenterX = Convert.ToDouble(tempStr);
                    }
                    if (XmlManager.GetAttribute(sampleRootNode, expXML, "centerToCenterY", ref tempStr))
                    {
                        sampleCarrierCollection[0].Template.CenterToCenterY = Convert.ToDouble(tempStr);
                    }
                    if (XmlManager.GetAttribute(sampleRootNode, expXML, "topLeftCenterOffsetX", ref tempStr))
                    {
                        sampleCarrierCollection[0].Template.TopLeftCenterOffsetX = Convert.ToDouble(tempStr);
                    }
                    if (XmlManager.GetAttribute(sampleRootNode, expXML, "topLeftCenterOffsetY", ref tempStr))
                    {
                        sampleCarrierCollection[0].Template.TopLeftCenterOffsetY = Convert.ToDouble(tempStr);
                    }
                    if (XmlManager.GetAttribute(sampleRootNode, expXML, "WellShape", ref tempStr))
                    {
                        tempStr = (tempStr.Contains("Well") ? tempStr : tempStr + "Well");
                        sampleCarrierCollection[0].Template.Shape = (WellShape)System.Enum.Parse(typeof(WellShape), tempStr);
                    }
                    if (XmlManager.GetAttribute(sampleRootNode, expXML, "WellWidth", ref tempStr))
                    {
                        sampleCarrierCollection[0].Template.Width = Convert.ToDouble(tempStr);
                    }
                    if (XmlManager.GetAttribute(sampleRootNode, expXML, "WellHeight", ref tempStr))
                    {
                        sampleCarrierCollection[0].Template.Height = Convert.ToDouble(tempStr);
                    }
                    if (XmlManager.GetAttribute(sampleRootNode, expXML, "DisplayTileGrid", ref tempStr))
                    {
                        xyTileControl.DisplayTileGrid = Convert.ToInt32(tempStr);
                    }

                    if (xyTileControl.XYtableData != null)
                        xyTileControl.XYtableData.Clear();

                    var wellList = expXML.SelectNodes("/ThorImageExperiment/Sample/Wells");

                    if (wellList != null)
                    {
                        string isEnabled = string.Empty;
                        string subRows = string.Empty;
                        string subColumns = string.Empty;
                        string transOffsetXMM = string.Empty;
                        string transOffsetYMM = string.Empty;
                        string transOffsetZMM = string.Empty;
                        string overlapX = string.Empty;
                        string overlapY = string.Empty;
                        string location = string.Empty;
                        string name = string.Empty;

                        for (int i = 0; i < wellList.Count; i++)
                        {
                            if (wellList[i].HasChildNodes)
                            {
                                for (int j = 0; j < wellList[i].ChildNodes.Count; j++)
                                {

                                    if (XmlManager.GetAttribute(wellList[i].ChildNodes[j], expXML, "name", ref name) &&
                                        XmlManager.GetAttribute(wellList[i].ChildNodes[j], expXML, "isEnabled", ref isEnabled) &&
                                        XmlManager.GetAttribute(wellList[i].ChildNodes[j], expXML, "subRows", ref subRows) &&
                                        XmlManager.GetAttribute(wellList[i].ChildNodes[j], expXML, "subColumns", ref subColumns) &&
                                        XmlManager.GetAttribute(wellList[i].ChildNodes[j], expXML, "transOffsetXMM", ref transOffsetXMM) &&
                                        XmlManager.GetAttribute(wellList[i].ChildNodes[j], expXML, "transOffsetYMM", ref transOffsetYMM) &&
                                        XmlManager.GetAttribute(wellList[i].ChildNodes[j], expXML, "transOffsetZMM", ref transOffsetZMM) &&
                                        XmlManager.GetAttribute(wellList[i].ChildNodes[j], expXML, "overlapX", ref overlapX) &&
                                        XmlManager.GetAttribute(wellList[i].ChildNodes[j], expXML, "overlapY", ref overlapY) &&
                                        XmlManager.GetAttribute(wellList[i], expXML, "location", ref location))
                                    {
                                        XYPosition xyPosition = new XYPosition()
                                        {
                                            Name = name,
                                            IsEnabled = Convert.ToBoolean(isEnabled),
                                            TileRow = subRows,
                                            TileCol = subColumns,
                                            X = transOffsetXMM,
                                            Y = transOffsetYMM,
                                            Z = transOffsetZMM,
                                            OverlapX = overlapX,
                                            OverlapY = overlapY,
                                            Well = location,
                                        };
                                        XYTable_AddItem(xyTileControl.XYtableData, xyPosition);
                                    }
                                    else
                                    {
                                        continue;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                ResourceManagerCS.ReturnDocMutexCS(SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS);
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "XYTileControl, Failed to load Tiles Sample parameters from experiment file in Capture Mode. Exception thrown: " + ex.Message);
            }
        }

        /// <summary>
        /// Xies the tile control_ x position changed.
        /// </summary>
        void xyTileControl_XPositionChanged()
        {
            if (_scanArea != null)
            {
                _scanArea.X1 = xyTileControl.ScanAreaXPosition - xyTileControl.ScanAreaWidth / 2;
                _scanArea.X2 = xyTileControl.ScanAreaXPosition + xyTileControl.ScanAreaWidth / 2;
                if (IsScanAreaOutOfSight())
                {
                    RestScanAreaView();
                }
            }
        }

        /// <summary>
        /// Xies the tile control_ y position changed.
        /// </summary>
        void xyTileControl_YPositionChanged()
        {
            if (_scanArea != null)
            {
                _scanArea.Y1 = xyTileControl.ScanAreaYPosition - xyTileControl.ScanAreaHeight / 2;
                _scanArea.Y2 = xyTileControl.ScanAreaYPosition + xyTileControl.ScanAreaHeight / 2;
                if (IsScanAreaOutOfSight())
                {
                    RestScanAreaView();
                }
            }
        }

        #endregion Methods
    }
}