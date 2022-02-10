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

    using ThorLogging;

    using ThorSharedTypes;

    /// <summary>
    /// Record Last file Information
    /// </summary>
    public struct TilePosition
    {
        #region Fields

        public string ExpPath;
        public int Index;
        public double X;
        public double Y;

        #endregion Fields

        #region Constructors

        public TilePosition(string ExpPath, int Index, double X, double Y)
        {
            this.ExpPath = ExpPath;
            this.Index = Index;
            this.X = X;
            this.Y = Y;
        }

        #endregion Constructors
    }

    /// <summary>
    /// ImageReview
    /// </summary>
    class ViewModeViewModel : XYtileViewModel
    {
        #region Fields

        private XYTileDisplay xyTileControl;
        private bool _loaded;

        /// <summary>
        /// Old Carrier, which is used to convert the old settings to new one.
        /// </summary>
        private ObservableCollection<Carrier> _oldCarrierCollection = new ObservableCollection<Carrier>()
        {
            new Carrier(){Name = "6 Well Plate",Type=CarrierType.Multiwell , Width = 127, Height = 85, Template = new WellPlateTemplate{CenterToCenterX = 36, CenterToCenterY = 36,  Col=3, Row=2, TopLeftCenterOffsetX=36, TopLeftCenterOffsetY=36, Diameter=35, Height = 0, Width = 0, Shape = WellShape.CircleWell }},
            new Carrier(){Name = "24 Well Plate",Type=CarrierType.Multiwell , Width = 127, Height = 85, Template = new WellPlateTemplate{CenterToCenterX = 18,CenterToCenterY = 18,  Col=6, Row=4, TopLeftCenterOffsetX=18, TopLeftCenterOffsetY=18,Diameter=15.5, Height = 0, Width = 0, Shape = WellShape.CircleWell }},
            new Carrier(){Name = "96 Well Plate",Type=CarrierType.Multiwell , Width = 127, Height = 85, Template = new WellPlateTemplate{CenterToCenterX = 9, CenterToCenterY = 9,  Col=12, Row=8, TopLeftCenterOffsetX=9, TopLeftCenterOffsetY=9,Diameter=6.35, Height = 0, Width = 0, Shape = WellShape.CircleWell }},
            new Carrier(){Name = "384 Well Plate",Type=CarrierType.Multiwell , Width = 127, Height = 85, Template = new WellPlateTemplate{CenterToCenterX = 4.5, CenterToCenterY = 4.5,  Col=24, Row=16, TopLeftCenterOffsetX=4.5, TopLeftCenterOffsetY=4.5,Diameter=3 , Height = 0, Width = 0, Shape = WellShape.CircleWell}},
            new Carrier(){Name = "1536 Well Plate",Type=CarrierType.Multiwell ,Width = 127, Height = 85, Template = new WellPlateTemplate{CenterToCenterX = 2.25, CenterToCenterY = 2.25,  Col=48, Row=32, TopLeftCenterOffsetX=2.25, TopLeftCenterOffsetY=2.25,Diameter=1.7 , Height = 0, Width = 0, Shape = WellShape.CircleWell}},
            new Carrier(){Name = "Slide",Type=CarrierType.Slide , Width = 75, Height = 25, Template = new WellPlateTemplate{CenterToCenterX = 0,CenterToCenterY = 0,  Col=0, Row=0, TopLeftCenterOffsetX=0, TopLeftCenterOffsetY=0,Diameter=0 }},
        };
        private BoxAnnotation _scanArea;
        private int _selectedTileIndex = 1;
        private TilePosition _tilePosition;
        private int _tilesIndex = 0;
        private int _wellIndex = 1;

        #endregion Fields

        #region Constructors

        /// <summary>
        /// Initializes a new instance of the <see cref="ViewModeViewModel"/> class.
        /// </summary>
        /// <param name="xyTileControl">The xy tile control.</param>
        public ViewModeViewModel(XYTileDisplay xyTileControl)
        {
            this.xyTileControl = xyTileControl;
            this._loaded = false;
            this._tilePosition.ExpPath = null;
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
            _selectedTileIndex++;

            if (_selectedTileIndex > GetLastEnabledTileIndex())
            {
                _selectedTileIndex = GetFirstEnabledTileIndex();
            }

            Point p1, p2;
            int newIndex;
            if (GetScanAreaForTileIndex(_selectedTileIndex, out p1, out p2, out newIndex))
            {
                _scanArea.X1 = p1.X;
                _scanArea.Y1 = p1.Y;
                _scanArea.X2 = p2.X;
                _scanArea.Y2 = p2.Y;
                xyTileControl.ChangeTilesIndex(_wellIndex, newIndex);

                _tilePosition = new TilePosition(xyTileControl.ActiveXML, newIndex, Convert.ToDouble(_scanArea.X1), Convert.ToDouble(_scanArea.Y1));
            }
        }

        /// <summary>
        /// Moves to the previous tile.
        /// </summary>
        public void MoveToPreviousTile()
        {
            _selectedTileIndex--;

            if (_selectedTileIndex < GetFirstEnabledTileIndex())
            {
                _selectedTileIndex = GetLastEnabledTileIndex();
            }

            Point p1, p2;
            int imageIndex;
            if (GetScanAreaForTileIndex(_selectedTileIndex, out p1, out p2, out imageIndex))
            {
                _scanArea.X1 = p1.X;
                _scanArea.Y1 = p1.Y;
                _scanArea.X2 = p2.X;
                _scanArea.Y2 = p2.Y;
                xyTileControl.ChangeTilesIndex(_wellIndex, imageIndex);

                _tilePosition = new TilePosition(xyTileControl.ActiveXML, _selectedTileIndex, Convert.ToDouble(_scanArea.X1), Convert.ToDouble(_scanArea.Y1));
            }
        }

        public void ResetSample()
        {
            //// not use
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
            _tilesIndex = 0;
            _scanArea = new BoxAnnotation()
            {
                Background = Brushes.RoyalBlue,
                BorderBrush = Brushes.DarkBlue,
                CornerRadius = new CornerRadius(1),
                Opacity = 0.8,
                X1 = xyTileControl.ScanAreaWidth / 2,
                X2 = xyTileControl.ScanAreaWidth / 2,
                Y1 = xyTileControl.ScanAreaHeight / 2,
                Y2 = xyTileControl.ScanAreaHeight / 2,
                Tag = "Irremovable",
                ToolTip = "Scan Area",
                IsHitTestVisible = false,
            };
            //Set the scan area to the toppest of canvas to make sure no other objects cover it.
            Canvas.SetZIndex(_scanArea, 9999);

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

            for (int k = 0; k < xyTileControl.XYtableData.Count; k++)
            {
                XYPosition xyPosition = xyTileControl.XYtableData[k];
                double overlapX = Convert.ToDouble(xyPosition.OverlapX) / 100;
                double overlapY = Convert.ToDouble(xyPosition.OverlapY) / 100;
                double col = Convert.ToDouble(xyPosition.TileCol);
                double row = Convert.ToDouble(xyPosition.TileRow);

                if (true == xyTileControl.ShowGrid.IsChecked)
                {
                    for (int i = 0; i < Convert.ToInt32(xyPosition.TileRow); i++)
                    {
                        for (int j = 0; j < Convert.ToInt32(xyPosition.TileCol); j++)
                        {
                            _tilesIndex++;
                            if (xyPosition.IsEnabled == false) continue;
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
                                Tag = _tilesIndex,
                            };
                            selectArea.PreviewMouseDown += selectArea_PreviewMouseDown;
                            tempAnnotations.Add(selectArea);
                        }
                    }
                }
                else
                {
                    if (xyPosition.IsEnabled == false) continue;
                    BoxAnnotation selectArea = new BoxAnnotation()
                    {
                        Background = Brushes.Silver,
                        X1 = Convert.ToDouble(xyPosition.X) + xyTileControl.HomePosX - (col - (col - 1) * overlapX) * xyTileControl.ScanAreaWidth / 2,
                        Y1 = Convert.ToDouble(xyPosition.Y) + xyTileControl.HomePosY + (row - (row - 1) * overlapY) * xyTileControl.ScanAreaHeight / 2,
                        X2 = Convert.ToDouble(xyPosition.X) + xyTileControl.HomePosX + (col - (col - 1) * overlapX) * xyTileControl.ScanAreaWidth / 2,
                        Y2 = Convert.ToDouble(xyPosition.Y) + xyTileControl.HomePosY - (row - (row - 1) * overlapY) * xyTileControl.ScanAreaHeight / 2,
                        BorderBrush = Brushes.Ivory,
                        BorderThickness = new Thickness(1),
                        Opacity = 0.8,
                        Tag = xyPosition,
                    };
                    //selectArea.PreviewMouseLeftButtonDown += selectArea_PreviewMouseLeftButtonDown;
                    tempAnnotations.Add(selectArea);
                }
            }
            xyTileControl.sciChart.Annotations = tempAnnotations;
        }

        public void ResetSampleView()
        {
            // not use
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
            // not use
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
                if (index == -1)
                {
                    xytableData.Add(xyPosition);
                }
                else
                {
                    xytableData.Insert(index, xyPosition);
                }
            }
        }

        public void XYTable_DeleteItem(ObservableCollection<XYPosition> xytableData, int tiledAreaIndex)
        {
            xytableData.RemoveAt(tiledAreaIndex);
        }

        public void XYTable_EditItem(ObservableCollection<XYPosition> xytableData, int selectedRow, int selectedColumn, object newValue)
        {
            //if the enable field is changed, redraw (hide unchecked areas, show checked areas).
            if (selectedColumn == 0)
            {
                ResetSampleInScichart();
                int i = 0;
                int tilesIndex = 0;
                for (; i < xyTileControl.XYtableData.Count; i++)
                {
                    if (true == xyTileControl.ShowGrid.IsChecked)
                    {
                        for (int k = 0; k < Convert.ToInt32(xyTileControl.XYtableData[i].TileRow); k++)
                        {
                            for (int j = 0; j < Convert.ToInt32(xyTileControl.XYtableData[i].TileCol); j++)
                            {
                                tilesIndex++;
                                if (!xyTileControl.XYtableData[i].IsEnabled) continue;
                                else goto BREAK;
                            }
                        }
                    }
                }
            BREAK: ;
                SetScanRrea(i);
                xyTileControl.ChangeTilesIndex(_wellIndex, tilesIndex);
                _tilePosition = new TilePosition(xyTileControl.ActiveXML, tilesIndex, Convert.ToDouble(_scanArea.X1), Convert.ToDouble(_scanArea.Y1));
                _selectedTileIndex = GetSelectedTileIndex(tilesIndex);
            }
            //Update the name in the settings file for current row
            else if (selectedColumn == 1)
            {
                xytableData[selectedRow].Name = newValue.ToString();
                XmlDocument expXML = new XmlDocument();

                expXML.Load(xyTileControl.ActiveXML);

                var wellList = expXML.SelectNodes("/ThorImageExperiment/Sample/Wells");

                if (wellList != null)
                {
                    for (int i = 0; i < wellList.Count; i++)
                    {
                        if (wellList[i].HasChildNodes)
                        {
                            for (int j = 0; j < wellList[i].ChildNodes.Count; j++)
                            {
                                if (selectedRow != i * wellList[i].ChildNodes.Count + j)
                                {
                                    continue;
                                }
                                XmlManager.SetAttribute(wellList[i].ChildNodes[j], expXML, "name", newValue.ToString());
                            }
                        }
                    }
                }
                expXML.Save(xyTileControl.ActiveXML);
            }
            //all others, do nothing
            else
            {
                XYPosition xyposition = xytableData[selectedRow];
                XYTable_DeleteItem(xytableData, selectedRow);
                XYTable_AddItem(xytableData, xyposition, selectedRow);
            }
        }

        /// <summary>
        /// Xies the table_ select item.
        /// </summary>
        /// <param name="xytableData">The xytable data.</param>
        /// <param name="index">The xytable selected row index.</param>
        public void XYTable_SelectItem(ObservableCollection<XYPosition> xytableData, int index)
        {
            if (SetScanRrea(index))
            {
                RestScanAreaView();
                int tileIndex = 1;
                for (int i = 0; i < xytableData.Count; i++)
                {
                    if (i == index)
                    {
                        xyTileControl.ChangeTilesIndex(_wellIndex, tileIndex);
                    }
                    tileIndex += Convert.ToInt32(xytableData[i].TileCol) * Convert.ToInt32(xytableData[i].TileRow);
                }
            }
        }

        /// <summary>
        /// xes the ytile view model_ loaded.
        /// </summary>
        public void XYtileViewModel_Loaded()
        {
            /* Loaded event fires twice if its a child of a TabControl.
                * Once because the TabControl seems to initiali(z|s)e everything.
                * And a second time when the Tab is switched to. */

            if (_loaded) return; ////Counteracts the double 'Loaded' event issue.

            xyTileControl.ExpPathChanged += xyTileControl_ExpPathChanged;

            xyTileControl.SampleCarrierCollection = new ObservableCollection<Carrier>();
            xyTileControl.IsControlPanelVisible = Visibility.Collapsed;
            xyTileControl.IsTileTableVisible = Visibility.Visible;
            xyTileControl.NextPreviousTileStackPanelVisible = Visibility.Visible;
            xyTileControl.IsDataGridReadOnly = true;
            xyTileControl.Control.Width = 245;
            xyTileControl.sciChart.Width = 245;
            xyTileControl.sciChart.Height = 245;
            xyTileControl.sciChart.HorizontalAlignment = HorizontalAlignment.Left;
            xyTileControl.XYtable.Width = 225;
            xyTileControl.IsChartDragSelectModifierEnabled = false;
            xyTileControl.MouseWheelZoomModifier.IsEnabled = true;
            xyTileControl.PinchZoomModifier.IsEnabled = true;
            xyTileControl.ZoomPanModifier.IsEnabled = true;

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

            _loaded = true;

            if (this.xyTileControl.ActiveXML != null)
            {
                xyTileControl_ExpPathChanged(this.xyTileControl.ActiveXML);
            }
        }

        /// <summary>
        /// xes the ytile view model_ unloaded.
        /// </summary>
        public void XYtileViewModel_Unloaded()
        {
            xyTileControl.ExpPathChanged -= xyTileControl_ExpPathChanged;
            _loaded = false;
        }

        /// <summary>
        /// Converts the old experiment settings.
        /// </summary>
        /// <param name="expXMLPath">The exp XML path.</param>
        private bool ConvertOldExperimentSettings(string expXMLPath)
        {
            XmlDocument expXML = new XmlDocument();
            expXML.Load(expXMLPath);

            var sampleRootNode = expXML.SelectSingleNode("/ThorImageExperiment/Sample");
            if (sampleRootNode != null)
            {
                if (sampleRootNode.Attributes["type"] != null && sampleRootNode.Attributes["offsetXMM"] != null && sampleRootNode.Attributes["offsetYMM"] != null)
                {
                    try
                    {
                        if (Regex.IsMatch(sampleRootNode.Attributes["type"].Value, @"\d"))//make sure it's old version
                        {

                            //convert the sample carrier settings
                            int type = Convert.ToInt32(sampleRootNode.Attributes["type"].Value);
                            xyTileControl.SampleCarrierCollection.Add(_oldCarrierCollection[type]);
                            xyTileControl.SelectedCarrierIndex = 0;

                            var streamingRootNode = expXML.SelectSingleNode("/ThorImageExperiment/Streaming");
                            if (streamingRootNode != null && streamingRootNode.Attributes["enable"] != null && streamingRootNode.Attributes["enable"].Value != "1")
                            {
                                ////Save the copy of old experiment file
                                //string oldXMLCopyPath = expXMLPath.Remove(expXMLPath.Length - 4, 4) + "_Old.xml";
                                //XmlDocument oldXMLCopy = new XmlDocument();
                                //oldXMLCopy.Load(expXMLPath);
                                //oldXMLCopy.Save(oldXMLCopyPath);
                                xyTileControl.HomePosX = Convert.ToDouble(sampleRootNode.Attributes["offsetXMM"].Value) * -1;
                                xyTileControl.HomePosY = Convert.ToDouble(sampleRootNode.Attributes["offsetYMM"].Value) * -1;
                                xyTileControl.HomePosZ = xyTileControl.ScanAreaZPosition;

                                //convert well settings
                                var wellNode = expXML.SelectSingleNode("/ThorImageExperiment/Sample/Wells");
                                if (wellNode != null)
                                {
                                    XYPosition xyPosition = new XYPosition()
                                    {
                                        IsEnabled = true,
                                        Name = "Tiles",
                                        X = "0",
                                        Y = "0",
                                        Z = "0",
                                        TileCol = "0",
                                        TileRow = "0",
                                        OverlapX = "0",
                                        OverlapY = "0",
                                        Well = "A1"
                                    };

                                    int wellRow = 1;
                                    int wellColumn = 1;
                                    if (xyTileControl.CurrentCarrier.Type == CarrierType.Multiwell)
                                    {
                                        wellRow = Convert.ToInt32(wellNode.Attributes["startRow"].Value);
                                        wellColumn = Convert.ToInt32(wellNode.Attributes["startColumn"].Value);
                                    }
                                    wellNode.Attributes.RemoveAll();
                                    xyPosition.Well = xyTileControl.TagWells(wellRow - 1, wellColumn - 1);

                                    _wellIndex = (wellRow - 1) * xyTileControl.CurrentCarrier.Template.Col + wellColumn;

                                    //convert subimage settings
                                    var subImageNode = expXML.SelectSingleNode("/ThorImageExperiment/Sample/Wells/SubImages");
                                    if (subImageNode != null)
                                    {
                                        var camNode = expXML.SelectSingleNode("/ThorImageExperiment/LSM");
                                        string pixelX = string.Empty;
                                        string pixelY = string.Empty;
                                        string pixelSizeUM = string.Empty;
                                        int cameraType = (int)ICamera.CameraType.LSM;
                                        XmlNode modalityNode = expXML.SelectSingleNode("/ThorImageExperiment/Modality");
                                        xyTileControl.ScanAreaWidth = 0.12;
                                        xyTileControl.ScanAreaHeight = 0.12;

                                        if (null != modalityNode)
                                        {
                                            string cameraTypeString = string.Empty;
                                            XmlManager.GetAttribute(modalityNode, expXML, "primaryDetectorType", ref cameraTypeString);
                                            Int32.TryParse(cameraTypeString, out cameraType);
                                        }

                                        if ((int)ICamera.CameraType.LSM == cameraType)
                                        {
                                            if (XmlManager.GetAttribute(camNode, expXML, "pixelX", ref pixelX) && XmlManager.GetAttribute(camNode, expXML, "pixelY", ref pixelY) && XmlManager.GetAttribute(camNode, expXML, "pixelSizeUM", ref pixelSizeUM)
                                                && Convert.ToDouble(pixelX) != 0 && Convert.ToDouble(pixelSizeUM) != 0 && Convert.ToDouble(pixelY) != 0)
                                            {
                                                xyTileControl.ScanAreaWidth = Math.Round(Convert.ToDouble(pixelX) * Convert.ToDouble(pixelSizeUM)) / 1000;
                                                xyTileControl.ScanAreaHeight = Math.Round(Convert.ToDouble(pixelY) * Convert.ToDouble(pixelSizeUM)) / 1000;
                                            }
                                        }
                                        else if((int)ICamera.CameraType.CCD == cameraType)
                                        {
                                            camNode = expXML.SelectSingleNode("/ThorImageExperiment/Camera");
                                            if (XmlManager.GetAttribute(camNode, expXML, "width", ref pixelX) && XmlManager.GetAttribute(camNode, expXML, "height", ref pixelY) && XmlManager.GetAttribute(camNode, expXML, "pixelSizeUM", ref pixelSizeUM)
                                                && Convert.ToDouble(pixelX) != 0 && Convert.ToDouble(pixelSizeUM) != 0 && Convert.ToDouble(pixelY) != 0)
                                            {
                                                xyTileControl.ScanAreaWidth = Math.Round(Convert.ToDouble(pixelX) * Convert.ToDouble(pixelSizeUM)) / 1000;
                                                xyTileControl.ScanAreaHeight = Math.Round(Convert.ToDouble(pixelY) * Convert.ToDouble(pixelSizeUM)) / 1000;
                                            }
                                        }

                                        string tempStr = string.Empty;

                                        if (XmlManager.GetAttribute(subImageNode, expXML, "subOffsetXMM", ref tempStr))
                                        {
                                            xyPosition.OverlapX = String.Format("{0:N3}", Convert.ToString((1 - (Convert.ToDouble(tempStr) / xyTileControl.ScanAreaWidth)) * 100));
                                        }
                                        if (XmlManager.GetAttribute(subImageNode, expXML, "subOffsetYMM", ref tempStr))
                                        {
                                            xyPosition.OverlapY = String.Format("{0:N3}", Convert.ToString((1 - (Convert.ToDouble(tempStr) / xyTileControl.ScanAreaHeight)) * 100));
                                        }
                                        if (XmlManager.GetAttribute(subImageNode, expXML, "subRows", ref tempStr))
                                        {
                                            xyPosition.TileRow = tempStr;
                                        }
                                        if (XmlManager.GetAttribute(subImageNode, expXML, "subColumns", ref tempStr))
                                        {
                                            xyPosition.TileCol = tempStr;
                                        }

                                        double X = 0;
                                        double Y = 0;
                                        double Z = (Convert.ToDouble(String.Format("{0:N4}", xyTileControl.ScanAreaZPosition.ToString())));

                                        if (XmlManager.GetAttribute(subImageNode, expXML, "transOffsetXMM", ref tempStr))
                                        {
                                            X = (Convert.ToDouble((String.Format("{0:N4}", tempStr))) * -1) + (Convert.ToInt32(xyPosition.TileCol) - (Convert.ToInt32(xyPosition.TileCol) - 1) * (Convert.ToDouble(xyPosition.OverlapX) / 100)) * xyTileControl.ScanAreaWidth / 2;
                                        }
                                        if (XmlManager.GetAttribute(subImageNode, expXML, "transOffsetYMM", ref tempStr))
                                        {
                                            Y = (Convert.ToDouble((String.Format("{0:N4}", tempStr))) * -1) - (Convert.ToInt32(xyPosition.TileRow) - (Convert.ToInt32(xyPosition.TileRow) - 1) * (Convert.ToDouble(xyPosition.OverlapY) / 100)) * xyTileControl.ScanAreaHeight / 2;
                                        }
                                    }
                                    XYTable_AddItem(xyTileControl.XYtableData, xyPosition);
                                }
                            }
                        }
                    }
                    catch (Exception ex)
                    {
                        ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "XYTileControl, Failed to convert old experiments in View Mode. Exception thrown: " + ex.Message);
                    }
                    return true;
                }
            }
            return false;
        }

        /// <summary>
        /// Gets the index of the first tile in the first enabled tile area
        /// </summary>
        /// <returns> returns the index </returns>
        private int GetFirstEnabledTileIndex()
        {
            int tilesIndex = 0;
            for (int i = 0; i < xyTileControl.XYtableData.Count; i++)
            {
                if (true == xyTileControl.ShowGrid.IsChecked)
                {
                    for (int k = 0; k < Convert.ToInt32(xyTileControl.XYtableData[i].TileRow); k++)
                    {
                        for (int j = 0; j < Convert.ToInt32(xyTileControl.XYtableData[i].TileCol); j++)
                        {
                            tilesIndex++;
                            if (!xyTileControl.XYtableData[i].IsEnabled) continue;
                            else return tilesIndex;
                        }
                    }
                }
            }
            return 0;
        }

        /// <summary>
        /// Gets the index of the last tile in the last enabled tile area
        /// </summary>
        /// <returns> returns the index </returns>
        private int GetLastEnabledTileIndex()
        {
            int tilesIndex = 0;
            int lastEnabledTileIndex = 0;
            for (int i = 0; i < xyTileControl.XYtableData.Count; i++)
            {
                if (true == xyTileControl.ShowGrid.IsChecked)
                {
                    for (int k = 0; k < Convert.ToInt32(xyTileControl.XYtableData[i].TileRow); k++)
                    {
                        for (int j = 0; j < Convert.ToInt32(xyTileControl.XYtableData[i].TileCol); j++)
                        {
                            tilesIndex++;
                            if (!xyTileControl.XYtableData[i].IsEnabled) continue;
                            else lastEnabledTileIndex = tilesIndex;
                        }
                    }
                }
            }

            return lastEnabledTileIndex;
        }

        /// <summary>
        /// Gets the area for the tile index
        /// </summary>
        /// <param name="tileIndex">the tile index.</param>
        /// <param name="p1"> point with the top left coordinates of the tile.</param>
        /// <param name="p2"> point with the bottom right coordinates of the tile.</param>
        /// <param name="imageIndex"> index of the image corresponding to the tileIndex.</param>
        private bool GetScanAreaForTileIndex(int tileIndex, out Point p1, out Point p2, out int imageIndex)
        {
            int tilesIndex = 0;
            imageIndex = 0;
            p1 = new Point();
            p2 = new Point();
            int totalRowsTiles = 0;
            for (int k = 0; k < xyTileControl.XYtableData.Count; k++)
            {
                XYPosition xyPosition = xyTileControl.XYtableData[k];

                double overlapX = Convert.ToDouble(xyPosition.OverlapX) / 100;
                double overlapY = Convert.ToDouble(xyPosition.OverlapY) / 100;
                double col = Convert.ToDouble(xyPosition.TileCol);
                double row = Convert.ToDouble(xyPosition.TileRow);

                for (int i = 0; i < Convert.ToInt32(xyPosition.TileRow); i++)
                {
                    //The tiling capture follows a snake pattern, left to right first, and then right to left
                    if (i % 2 == 0)
                    {
                        for (int j = 0; j < Convert.ToInt32(xyPosition.TileCol); j++)
                        {
                            tilesIndex++;
                            if (xyPosition.IsEnabled == false) continue;
                            if (tilesIndex == tileIndex)
                            {
                                p1.X = Convert.ToDouble(xyPosition.X) + xyTileControl.HomePosX - (col - (col - 1) * overlapX) * xyTileControl.ScanAreaWidth / 2 + j * (1 - overlapX) * xyTileControl.ScanAreaWidth;
                                p1.Y = Convert.ToDouble(xyPosition.Y) + xyTileControl.HomePosY + (row - (row - 1) * overlapY) * xyTileControl.ScanAreaHeight / 2 - i * (1 - overlapY) * xyTileControl.ScanAreaHeight;
                                p2.X = Convert.ToDouble(xyPosition.X) + xyTileControl.HomePosX - (col - (col - 1) * overlapX) * xyTileControl.ScanAreaWidth / 2 + (j + 1 - j * overlapX) * xyTileControl.ScanAreaWidth;
                                p2.Y = Convert.ToDouble(xyPosition.Y) + xyTileControl.HomePosY + (row - (row - 1) * overlapY) * xyTileControl.ScanAreaHeight / 2 - (i + 1 - i * overlapY) * xyTileControl.ScanAreaHeight;
                                imageIndex = tileIndex;
                                return true;
                            }
                        }
                    }
                    else
                    {
                        int colCount = 0;
                        for (int j = Convert.ToInt32(xyPosition.TileCol) - 1; j >= 0; j--)
                        {
                            tilesIndex++;
                            if (xyPosition.IsEnabled == false) continue;
                            if (tilesIndex == tileIndex)
                            {
                                p1.X = Convert.ToDouble(xyPosition.X) + xyTileControl.HomePosX - (col - (col - 1) * overlapX) * xyTileControl.ScanAreaWidth / 2 + j * (1 - overlapX) * xyTileControl.ScanAreaWidth;
                                p1.Y = Convert.ToDouble(xyPosition.Y) + xyTileControl.HomePosY + (row - (row - 1) * overlapY) * xyTileControl.ScanAreaHeight / 2 - i * (1 - overlapY) * xyTileControl.ScanAreaHeight;
                                p2.X = Convert.ToDouble(xyPosition.X) + xyTileControl.HomePosX - (col - (col - 1) * overlapX) * xyTileControl.ScanAreaWidth / 2 + (j + 1 - j * overlapX) * xyTileControl.ScanAreaWidth;
                                p2.Y = Convert.ToDouble(xyPosition.Y) + xyTileControl.HomePosY + (row - (row - 1) * overlapY) * xyTileControl.ScanAreaHeight / 2 - (i + 1 - i * overlapY) * xyTileControl.ScanAreaHeight;
                                imageIndex = totalRowsTiles + Convert.ToInt32(xyPosition.TileCol) - colCount;
                                return true;
                            }
                            colCount++;
                        }
                    }
                    totalRowsTiles += Convert.ToInt32(xyPosition.TileCol);
                }
            }
            return false;
        }

        /// <summary>
        /// Gets the selected index of the tile
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="MouseButtonEventArgs"/> instance containing the event data.</param>
        private int GetSelectedTileIndex(int index)
        {
            int selectedIndex = 0;
            int totalRowsTiles = 0;
            int tilesIndex = 0;
            for (int k = 0; k < xyTileControl.XYtableData.Count; k++)
            {
                XYPosition xyPosition = xyTileControl.XYtableData[k];

                double overlapX = Convert.ToDouble(xyPosition.OverlapX) / 100;
                double overlapY = Convert.ToDouble(xyPosition.OverlapY) / 100;
                double col = Convert.ToDouble(xyPosition.TileCol);
                double row = Convert.ToDouble(xyPosition.TileRow);

                for (int i = 0; i < Convert.ToInt32(xyPosition.TileRow); i++)
                {
                    //The tiling capture follows a snake pattern, left to right first, and then right to left
                    if (i % 2 == 0)
                    {
                        for (int j = 0; j < Convert.ToInt32(xyPosition.TileCol); j++)
                        {
                            tilesIndex++;
                            if (xyPosition.IsEnabled == false) continue;
                            if (tilesIndex == index)
                            {
                                selectedIndex = tilesIndex;
                                return selectedIndex;
                            }
                        }
                    }
                    else
                    {
                        int colCount = 0;
                        for (int j = Convert.ToInt32(xyPosition.TileCol) - 1; j >= 0; j--)
                        {
                            tilesIndex++;
                            if (xyPosition.IsEnabled == false) continue;
                            if (tilesIndex == index)
                            {
                                selectedIndex = totalRowsTiles + Convert.ToInt32(xyPosition.TileCol) - colCount;
                                return selectedIndex;
                            }
                            colCount++;
                        }
                    }
                    totalRowsTiles += Convert.ToInt32(xyPosition.TileCol);
                }
            }
            return selectedIndex;
        }

        /// <summary>
        /// Loads the experiment settings.
        /// </summary>
        /// <param name="expXMLPath">The exp XML path.</param>
        /// <param name="sampleCarrierCollection">The sample carrier collection.</param>
        private bool LoadExperimentSettings(String expXMLPath, ObservableCollection<Carrier> sampleCarrierCollection)
        {
            try
            {
                _loaded = false;

                if (null == expXMLPath || string.Empty == expXMLPath || !File.Exists(expXMLPath))
                {
                    return false;
                }
                _tilesIndex = 0;
                if (null != xyTileControl.XYtableData)
                {
                    xyTileControl.XYtableData.Clear();
                }
                xyTileControl.SampleCarrierCollection.Clear();

                _wellIndex = 1;
                //Convert Old Experiment Settings to new one.
                if (ConvertOldExperimentSettings(expXMLPath))
                {
                    _loaded = true;
                    return true;
                }

                XmlDocument expXML = new XmlDocument();
                expXML.Load(expXMLPath);

                var camNode = expXML.SelectSingleNode("/ThorImageExperiment/LSM");
                string pixelX = string.Empty;
                string pixelY = string.Empty;
                string pixelSizeUM = string.Empty;
                int cameraType = (int)ICamera.CameraType.LSM;
                XmlNode modalityNode = expXML.SelectSingleNode("/ThorImageExperiment/Modality");
                xyTileControl.ScanAreaWidth = 0.12;
                xyTileControl.ScanAreaHeight = 0.12;

                if (null != modalityNode)
                {
                    string cameraTypeString = string.Empty;
                    XmlManager.GetAttribute(modalityNode, expXML, "primaryDetectorType", ref cameraTypeString);
                    Int32.TryParse(cameraTypeString, out cameraType);
                }

                if ((int)ICamera.CameraType.LSM == cameraType)
                {
                    if (XmlManager.GetAttribute(camNode, expXML, "pixelX", ref pixelX) && XmlManager.GetAttribute(camNode, expXML, "pixelY", ref pixelY) && XmlManager.GetAttribute(camNode, expXML, "pixelSizeUM", ref pixelSizeUM)
                        && Convert.ToDouble(pixelX) != 0 && Convert.ToDouble(pixelSizeUM) != 0 && Convert.ToDouble(pixelY) != 0)
                    {
                        xyTileControl.ScanAreaWidth = Math.Round(Convert.ToDouble(pixelX) * Convert.ToDouble(pixelSizeUM)) / 1000;
                        xyTileControl.ScanAreaHeight = Math.Round(Convert.ToDouble(pixelY) * Convert.ToDouble(pixelSizeUM)) / 1000;
                    }
                }
                else if ((int)ICamera.CameraType.CCD == cameraType)
                {
                    camNode = expXML.SelectSingleNode("/ThorImageExperiment/Camera");
                    if (XmlManager.GetAttribute(camNode, expXML, "width", ref pixelX) && XmlManager.GetAttribute(camNode, expXML, "height", ref pixelY) && XmlManager.GetAttribute(camNode, expXML, "pixelSizeUM", ref pixelSizeUM)
                        && Convert.ToDouble(pixelX) != 0 && Convert.ToDouble(pixelSizeUM) != 0 && Convert.ToDouble(pixelY) != 0)
                    {
                        xyTileControl.ScanAreaWidth = Math.Round(Convert.ToDouble(pixelX) * Convert.ToDouble(pixelSizeUM)) / 1000;
                        xyTileControl.ScanAreaHeight = Math.Round(Convert.ToDouble(pixelY) * Convert.ToDouble(pixelSizeUM)) / 1000;
                    }
                }

                //preset to slide
                sampleCarrierCollection.Add(_oldCarrierCollection[5]);
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

                    if (XmlManager.GetAttribute(sampleRootNode, expXML, "tiltAdjustment", ref tempStr))
                    {
                        xyTileControl.TiltAdjustment = Convert.ToInt32(tempStr);
                    }

                    if (XmlManager.GetAttribute(sampleRootNode, expXML, "DisplayTileGrid", ref tempStr))
                    {
                        xyTileControl.DisplayTileGrid = Convert.ToInt32(tempStr);
                    }

                    //Update all of the properties
                    xyTileControl.OnPropertyChanged("TiltAdjustment");
                    xyTileControl.OnPropertyChanged("FocusPoint1");
                    xyTileControl.OnPropertyChanged("FocusPoint2");
                    xyTileControl.OnPropertyChanged("FocusPoint3");

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
                _loaded = true;
                return true;
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "XYTileControl, Failed to load experiment settings in View Mode. Exception thrown: " + ex.Message);
                return false;
            }
        }

        /// <summary>
        /// Handles the PreviewMouseDown event of the selectArea control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="MouseButtonEventArgs"/> instance containing the event data.</param>
        void selectArea_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            BoxAnnotation selectArea = sender as BoxAnnotation;

            _scanArea.X1 = selectArea.X1;
            _scanArea.X2 = selectArea.X2;
            _scanArea.Y1 = selectArea.Y1;
            _scanArea.Y2 = selectArea.Y2;
            int newImageIndex = Convert.ToInt32(selectArea.Tag);
            xyTileControl.ChangeTilesIndex(_wellIndex, newImageIndex);
            _tilePosition = new TilePosition(xyTileControl.ActiveXML, newImageIndex, Convert.ToDouble(_scanArea.X1), Convert.ToDouble(_scanArea.Y1));
            _selectedTileIndex = GetSelectedTileIndex(newImageIndex);
        }

        /// <summary>
        /// Sets the scan rrea.
        /// </summary>
        /// <param name="index">The xytable selected row index.</param>
        private bool SetScanRrea(int index)
        {
            if (xyTileControl.XYtableData.Count > index && index >= 0)
            {
                double x = Convert.ToDouble(xyTileControl.XYtableData[index].X);
                double y = Convert.ToDouble(xyTileControl.XYtableData[index].Y);
                double overlapX = Convert.ToDouble(xyTileControl.XYtableData[index].OverlapX) / 100;
                double overlapY = Convert.ToDouble(xyTileControl.XYtableData[index].OverlapY) / 100;
                double col = Convert.ToDouble(xyTileControl.XYtableData[index].TileCol);
                double row = Convert.ToDouble(xyTileControl.XYtableData[index].TileRow);

                _scanArea.X1 = xyTileControl.HomePosX + x - (col - (col - 1) * overlapX) * xyTileControl.ScanAreaWidth / 2;
                _scanArea.X2 = xyTileControl.HomePosX + x - (col - (col - 1) * overlapX - 2) * xyTileControl.ScanAreaWidth / 2;
                _scanArea.Y1 = xyTileControl.HomePosY + y + (row - (row - 1) * overlapY) * xyTileControl.ScanAreaHeight / 2;
                _scanArea.Y2 = xyTileControl.HomePosY + y + (row - (row - 1) * overlapY - 2) * xyTileControl.ScanAreaHeight / 2;
                return true;
            }
            return false;
        }

        /// <summary>
        /// Xies the tile control_ exp path changed.
        /// </summary>
        void xyTileControl_ExpPathChanged(string exp)
        {
            //Load Active.XML
            if (LoadExperimentSettings(exp, xyTileControl.SampleCarrierCollection))
            {
                _selectedTileIndex = 1;
                if (xyTileControl.XYtableData.Count > 0)
                {
                    xyTileControl.PositionTable.IsExpanded = true;
                }
                ResetSampleInScichart();
                if (SetScanRrea(xyTileControl.TileIndex - 1))
                {
                    RestScanAreaView();
                    xyTileControl.ChangeTilesIndex(_wellIndex, xyTileControl.TileIndex);
                    _tilePosition = new TilePosition(xyTileControl.ActiveXML, xyTileControl.TileIndex, Convert.ToDouble(_scanArea.X1), Convert.ToDouble(_scanArea.Y1));
                }
            }
        }

        #endregion Methods
    }
}