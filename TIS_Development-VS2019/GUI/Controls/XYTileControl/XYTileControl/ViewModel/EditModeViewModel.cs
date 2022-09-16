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
    using SciChart.Drawing.HighSpeedRasterizer;
    using SciChart.Drawing.VisualXcceleratorRasterizer;

    using ThorLogging;

    using ThorSharedTypes;

    /// <summary>
    /// CaptureSetUp
    /// </summary>
    class EditModeViewModel : XYtileViewModel
    {
        #region Fields

        private ObservableCollection<WellPosition> SelectedWellsCollection = null;
        private XYTileDisplay xyTileControl;
        private bool _draggingMouse = false;
        private XYPosition _duplicatedTiles;
        private bool _gridChangedSize = false;
        private bool _isSinglePositionEnabled;
        private DateTime _lastSetTime = DateTime.Now;
        private bool _loaded;

        /// <summary>
        /// Old Carrier, which is used to convert the old settings to new one.
        /// </summary>
        private ObservableCollection<Carrier> _oldCarrierCollection = new ObservableCollection<Carrier>()
        {
            new Carrier(){Name = "6 Well Plate",Type=CarrierType.Multiwell ,Width = 127, Height = 85, Template = new WellPlateTemplate{CenterToCenterX = 36, CenterToCenterY = 36,  Col=3, Row=2, TopLeftCenterOffsetX=36, TopLeftCenterOffsetY=36, Diameter=35, Height = 0, Width = 0, Shape = WellShape.CircleWell }},
            new Carrier(){Name = "24 Well Plate",Type=CarrierType.Multiwell , Width = 127, Height = 85, Template = new WellPlateTemplate{CenterToCenterX = 18,CenterToCenterY = 18,  Col=6, Row=4, TopLeftCenterOffsetX=18, TopLeftCenterOffsetY=18,Diameter=15.5, Height = 0, Width = 0, Shape = WellShape.CircleWell }},
            new Carrier(){Name = "96 Well Plate",Type=CarrierType.Multiwell , Width = 127, Height = 85, Template = new WellPlateTemplate{CenterToCenterX = 9, CenterToCenterY = 9,  Col=12, Row=8, TopLeftCenterOffsetX=9, TopLeftCenterOffsetY=9,Diameter=6.35, Height = 0, Width = 0, Shape = WellShape.CircleWell }},
            new Carrier(){Name = "384 Well Plate",Type=CarrierType.Multiwell , Width = 127, Height = 85, Template = new WellPlateTemplate{CenterToCenterX = 4.5, CenterToCenterY = 4.5,  Col=24, Row=16, TopLeftCenterOffsetX=4.5, TopLeftCenterOffsetY=4.5,Diameter=3 , Height = 0, Width = 0, Shape = WellShape.CircleWell}},
            new Carrier(){Name = "1536 Well Plate",Type=CarrierType.Multiwell , Width = 127, Height = 85, Template = new WellPlateTemplate{CenterToCenterX = 2.25, CenterToCenterY = 2.25,  Col=48, Row=32, TopLeftCenterOffsetX=2.25, TopLeftCenterOffsetY=2.25,Diameter=1.7 , Height = 0, Width = 0, Shape = WellShape.CircleWell}},
            new Carrier(){Name = "Slide",Type=CarrierType.Slide , Width = 75, Height = 25, Template = new WellPlateTemplate{CenterToCenterX = 0,CenterToCenterY = 0,  Col=0, Row=0, TopLeftCenterOffsetX=0, TopLeftCenterOffsetY=0,Diameter=0 }},
        };

        /// <summary>
        /// The _preset sample carriers
        /// </summary>
        private ObservableCollection<Carrier> _presetSampleCarriers = new ObservableCollection<Carrier>()
        {
            new Carrier(){Name = "Slide 75 x 25 mm",Type=CarrierType.Slide , Width = 75, Height = 25, Template = new WellPlateTemplate{CenterToCenterX = 0,CenterToCenterY = 0, Col=0, Row=0, TopLeftCenterOffsetX=0, TopLeftCenterOffsetY=0,Diameter=0 , Height = 0, Width = 0, Shape = WellShape.CircleWell}},
            new Carrier(){Name = "Slide 75 x 38 mm",Type=CarrierType.Slide , Width = 75, Height = 38, Template = new WellPlateTemplate{CenterToCenterX = 0,CenterToCenterY = 0, Col=0, Row=0, TopLeftCenterOffsetX=0, TopLeftCenterOffsetY=0,Diameter=0, Height = 0, Width = 0, Shape = WellShape.CircleWell }},
            new Carrier(){Name = "Slide 76 x 26 mm",Type=CarrierType.Slide , Width = 76, Height = 26, Template = new WellPlateTemplate{CenterToCenterX = 0,CenterToCenterY = 0, Col=0, Row=0, TopLeftCenterOffsetX=0, TopLeftCenterOffsetY=0,Diameter=0 , Height = 0, Width = 0, Shape = WellShape.CircleWell}},
            new Carrier(){Name = "Slide 48 x 28 mm",Type=CarrierType.Slide , Width = 75, Height = 28, Template = new WellPlateTemplate{CenterToCenterX = 0,CenterToCenterY = 0, Col=0, Row=0, TopLeftCenterOffsetX=0, TopLeftCenterOffsetY=0,Diameter=0 , Height = 0, Width = 0, Shape = WellShape.CircleWell}},
            new Carrier(){Name = "Slide 46 x 27 mm",Type=CarrierType.Slide , Width = 46, Height = 27, Template = new WellPlateTemplate{CenterToCenterX = 0,CenterToCenterY = 0, Col=0, Row=0, TopLeftCenterOffsetX=0, TopLeftCenterOffsetY=0,Diameter=0 , Height = 0, Width = 0, Shape = WellShape.CircleWell}},
            new Carrier(){Name = "Cover Slip 20 x 20 mm",Type=CarrierType.Slide , Width = 20, Height = 20, Template = new WellPlateTemplate{CenterToCenterX = 0,CenterToCenterY = 0, Col=0, Row=0, TopLeftCenterOffsetX=0, TopLeftCenterOffsetY=0,Diameter=0 , Height = 0, Width = 0, Shape = WellShape.CircleWell}},
            new Carrier(){Name = "Cover Slip 50 x 50 mm",Type=CarrierType.Slide ,Width = 50, Height = 50, Template = new WellPlateTemplate{CenterToCenterX = 0,CenterToCenterY = 0, Col=0, Row=0, TopLeftCenterOffsetX=0, TopLeftCenterOffsetY=0,Diameter=0 , Height = 0, Width = 0, Shape = WellShape.CircleWell}},
            new Carrier(){Name = "6 Well Plate",Type=CarrierType.Multiwell ,Width = 127.5, Height = 85.5,Template = new WellPlateTemplate{CenterToCenterX = 39.24,CenterToCenterY = 39.24, Col=3, Row=2, TopLeftCenterOffsetX=24.55, TopLeftCenterOffsetY=23.05, Diameter=35 , Height = 0, Width = 0, Shape = WellShape.CircleWell}},
            new Carrier(){Name = "24 Well Plate",Type=CarrierType.Multiwell , Width = 127.5, Height = 85.5, Template = new WellPlateTemplate{CenterToCenterX = 19.3,CenterToCenterY = 19.3, Col=6, Row=4, TopLeftCenterOffsetX=17.05, TopLeftCenterOffsetY=13.67,Diameter=15.5 , Height = 0, Width = 0, Shape = WellShape.CircleWell}},
            new Carrier(){Name = "96 Well Plate",Type=CarrierType.Multiwell , Width = 127.5, Height = 85.5,Template = new WellPlateTemplate{CenterToCenterX = 8.99,CenterToCenterY = 8.99, Col=12, Row=8, TopLeftCenterOffsetX=14.38, TopLeftCenterOffsetY=11.24,Diameter=6.35, Height = 0, Width = 0, Shape = WellShape.CircleWell }},
            new Carrier(){Name = "384 Well Plate",Type=CarrierType.Multiwell , Width = 127.5, Height = 85.5, Template = new WellPlateTemplate{CenterToCenterX = 4.5,CenterToCenterY = 4.5, Col=24, Row=16, TopLeftCenterOffsetX=12.13, TopLeftCenterOffsetY=9,Diameter=3, Height = 0, Width = 0, Shape = WellShape.CircleWell }},
            new Carrier(){Name = "1536 Well Plate",Type=CarrierType.Multiwell , Width = 127.5, Height = 85.5, Template = new WellPlateTemplate{CenterToCenterX = 2.25,CenterToCenterY = 2.25, Col=48, Row=32, TopLeftCenterOffsetX=10.96, TopLeftCenterOffsetY=7.84,Diameter=1.7 , Height = 0, Width = 0, Shape = WellShape.CircleWell}},
            new Carrier(){Name = "C4SH01 4 Slide Carrier",Type=CarrierType.Multislide , Width = 127.6, Height = 85.5, Template = new WellPlateTemplate{CenterToCenterX = 27.2,CenterToCenterY = 0, Col=4, Row=1, TopLeftCenterOffsetX=23.1, TopLeftCenterOffsetY=42.75,Diameter=0 , Height = 25, Width = 75, Shape = WellShape.RectangleWell}},
            new Carrier(){Name = "MLS203P10 4 Slide Carrier",Type=CarrierType.Multislide , Width = 170.3, Height = 130.3, Template = new WellPlateTemplate{CenterToCenterX = 28.5,CenterToCenterY = 0, Col=4, Row=1, TopLeftCenterOffsetX=40.65, TopLeftCenterOffsetY=65.15,Diameter=0 , Height = 25, Width = 75, Shape = WellShape.RectangleWell}},
            new Carrier(){Name = "Clath Slide",Type=CarrierType.Multiwell , Width = 70, Height = 25, Template = new WellPlateTemplate{CenterToCenterX = 30.463,CenterToCenterY = 3.271, Col=2, Row=6, TopLeftCenterOffsetX=21.655, TopLeftCenterOffsetY=3.3425,Diameter=0 , Height = 2.141, Width = 26.226, Shape = WellShape.RectangleWell}},
            new Carrier(){Name = "2 Chamber CoverSlip",Type=CarrierType.Multiwell , Width = 46, Height = 16, Template = new WellPlateTemplate{CenterToCenterX = 25,CenterToCenterY = 0, Col=2, Row=1, TopLeftCenterOffsetX=10.05, TopLeftCenterOffsetY=8,Diameter=0 , Height = 16, Width = 21, Shape = WellShape.RectangleWell}},
            new Carrier(){Name = "8 Chamber CoverSlip",Type=CarrierType.Multiwell , Width = 52, Height = 23, Template = new WellPlateTemplate{CenterToCenterX = 12.5,CenterToCenterY = 10.45, Col=4, Row=2, TopLeftCenterOffsetX=7.25, TopLeftCenterOffsetY=6.225,Diameter=0 , Height = 8.45, Width = 10.5, Shape = WellShape.RectangleWell}},
            new Carrier(){Name = "Petri Dish 100mm",Type=CarrierType.PetriDish ,Width = 0, Height = 0, Template = new WellPlateTemplate{CenterToCenterX = 0,CenterToCenterY = 0, Col=1, Row=1, TopLeftCenterOffsetX=0, TopLeftCenterOffsetY=0, Diameter=100 , Height = 0, Width = 0, Shape = WellShape.CircleWell}},
            new Carrier(){Name = "Petri Dish 60mm",Type=CarrierType.PetriDish , Width = 0, Height = 0, Template = new WellPlateTemplate{CenterToCenterX = 0,CenterToCenterY = 0, Col=1, Row=1, TopLeftCenterOffsetX=0, TopLeftCenterOffsetY=0,Diameter=60 , Height = 0, Width = 0, Shape = WellShape.CircleWell}},
        };
        private BoxAnnotation _rangeArea;
        private BoxAnnotation _scanArea;
        private int _touchCount = 0;

        #endregion Fields

        #region Constructors

        public EditModeViewModel(XYTileDisplay xyTileControl)
        {
            this.xyTileControl = xyTileControl;
            this._loaded = false;
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
        /// Loads the sample carriers collection from ApplcationSettings.XML.
        /// </summary>
        /// <param name="applicationSettingsPath">The application settings path.</param>
        /// <param name="sampleCarrierCollection">The sample carrier collection.</param>
        public void LoadApplcationSettings(String applicationSettingsPath, ObservableCollection<Carrier> sampleCarrierCollection)
        {
            // Clear Sample Carrier List
            sampleCarrierCollection.Clear();
            // Read ApplicationSetttings.XML
            XmlDocument appSettings = new XmlDocument();
            appSettings.Load(applicationSettingsPath);
            XmlNodeList ndList = appSettings.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/XYView");
            if (ndList != null && ndList.Count > 0)
            {
                string s = string.Empty;
                XmlManager.GetAttribute(ndList[0], appSettings, "showTileGrid", ref s);
                xyTileControl.ShowGrid.IsChecked = (s.CompareTo("1") == 0) ? true : false;
            }

            ndList = appSettings.SelectNodes("/ApplicationSettings/SampleCarrier/Carrier");
            if (ndList != null && ndList.Count > 0)
            {
                string type = string.Empty;
                string name = string.Empty;
                string height = string.Empty;
                string width = string.Empty;
                string shape = string.Empty;
                string row = string.Empty;
                string col = string.Empty;
                string diameter = string.Empty;
                string centerToCenterX = string.Empty;
                string centerToCenterY = string.Empty;
                string topLeftCenterOffsetX = string.Empty;
                string topLeftCenterOffsetY = string.Empty;
                string wellHeight = string.Empty;
                string wellWidth = string.Empty;

                for (int i = 0; i < ndList.Count; i++)
                {
                    if (XmlManager.GetAttribute(ndList[i], appSettings, "Type", ref type) &&
                        XmlManager.GetAttribute(ndList[i], appSettings, "Name", ref name) &&
                        XmlManager.GetAttribute(ndList[i], appSettings, "Height", ref height) &&
                        XmlManager.GetAttribute(ndList[i], appSettings, "Width", ref width) &&
                        XmlManager.GetAttribute(ndList[i], appSettings, "WellShape", ref shape) &&
                        XmlManager.GetAttribute(ndList[i], appSettings, "Row", ref row) &&
                        XmlManager.GetAttribute(ndList[i], appSettings, "Col", ref col) &&
                        XmlManager.GetAttribute(ndList[i], appSettings, "Diameter", ref diameter) &&
                        XmlManager.GetAttribute(ndList[i], appSettings, "CenterToCenterX", ref centerToCenterX) &&
                        XmlManager.GetAttribute(ndList[i], appSettings, "CenterToCenterY", ref centerToCenterY) &&
                        XmlManager.GetAttribute(ndList[i], appSettings, "TopLeftCenterOffsetX", ref topLeftCenterOffsetX) &&
                        XmlManager.GetAttribute(ndList[i], appSettings, "TopLeftCenterOffsetY", ref topLeftCenterOffsetY) &&
                        XmlManager.GetAttribute(ndList[i], appSettings, "WellHeight", ref wellHeight) &&
                        XmlManager.GetAttribute(ndList[i], appSettings, "WellWidth", ref wellWidth))
                    {
                        shape = shape.Contains("Well") ? shape : shape + "Well";

                        Carrier carrier = new Carrier()
                        {
                            Type = (CarrierType)System.Enum.Parse(typeof(CarrierType), type),
                            Name = name,
                            Height = Convert.ToDouble(height),
                            Width = Convert.ToDouble(width),
                            Template = new WellPlateTemplate()
                            {
                                Shape = (WellShape)System.Enum.Parse(typeof(WellShape), shape),
                                Row = Convert.ToInt32(row),
                                Col = Convert.ToInt32(col),
                                Diameter = Convert.ToDouble(diameter),
                                CenterToCenterX = Convert.ToDouble(centerToCenterX),
                                CenterToCenterY = Convert.ToDouble(centerToCenterY),
                                TopLeftCenterOffsetX = Convert.ToDouble(topLeftCenterOffsetX),
                                TopLeftCenterOffsetY = Convert.ToDouble(topLeftCenterOffsetY),
                                Height = Convert.ToDouble(wellHeight),
                                Width = Convert.ToDouble(wellWidth),
                            }
                        };
                        sampleCarrierCollection.Add(carrier);
                    }
                    else
                    {
                        continue;
                    }
                }
            }

            if (sampleCarrierCollection.Count <= 0)
            {
                var sampleRootNode = appSettings.SelectSingleNode("/ApplicationSettings/SampleCarrier");
                if (sampleRootNode == null) // If SampleCarrier node is not existed, add the new node
                {
                    XmlNode rootNode = appSettings.SelectSingleNode("/ApplicationSettings");
                    XmlElement sampleCarrierNode = appSettings.CreateElement(string.Empty, "SampleCarrier".ToString(), string.Empty);
                    rootNode.AppendChild(sampleCarrierNode);
                    appSettings.Save(applicationSettingsPath);
                }
                for (int i = 0; i < _presetSampleCarriers.Count; i++) // Add presetSampleCarrier to the carrier collection
                {
                    SaveNewSampleCarrierToApplicationSettings(applicationSettingsPath, _presetSampleCarriers[i], sampleCarrierCollection);
                }
            }
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

        /// <summary>
        /// Resets the sample.
        /// </summary>
        public void ResetSample()
        {
            xyTileControl.XYtableData.Clear();
            SelectedWellsCollection.Clear();
            ResetSampleInScichart();
            ResetSampleView();
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
            // Load XY range boundary
            tempAnnotations.Add(_rangeArea);
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
                ContextMenu = (ContextMenu)xyTileControl.FindResource("ContainerContextMenu"),
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
                            Background = (SelectedWellsCollection.Contains(new WellPosition(j, i))) ? Brushes.Tomato : Brushes.AliceBlue,
                            Tag = xyTileControl.TagWells(j, i),
                            ToolTip = xyTileControl.TagWells(j, i),
                            ContextMenu = (ContextMenu)xyTileControl.FindResource("WellContextMenu"),
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
                        well.PreviewMouseDoubleClick += well_PreviewMouseDoubleClick;
                        tempAnnotations.Add(well);
                    }
                }
            }
            for (int k = 0; k < xyTileControl.XYtableData.Count; k++)
            {
                XYPosition xyPosition = xyTileControl.XYtableData[k];
                if (xyPosition.IsEnabled == false) continue;

                double overlapX = Convert.ToDouble(xyPosition.OverlapX) / 100;
                double overlapY = Convert.ToDouble(xyPosition.OverlapY) / 100;
                double col = Convert.ToDouble(xyPosition.TileCol);
                double row = Convert.ToDouble(xyPosition.TileRow);

                if (xyTileControl.ShowGrid.IsChecked == true)
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
                                ContextMenu = (ContextMenu)xyTileControl.FindResource("TileContextMenu"),
                                BorderBrush = Brushes.Ivory,
                                BorderThickness = new Thickness(1),
                                Opacity = 0.8,
                                Tag = xyPosition,
                            };
                            selectArea.PreviewMouseLeftButtonDown += selectGridArea_PreviewMouseLeftButtonDown;
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
                    selectArea.PreviewMouseLeftButtonDown += selectArea_PreviewMouseLeftButtonDown;
                    tempAnnotations.Add(selectArea);
                }
            }
            xyTileControl.sciChart.Annotations = tempAnnotations;
            // Reset TileModifer
            xyTileControl.TileModifier.Clear();
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
                if (ResourceManagerCS.Instance.TabletModeEnabled)
                {
                    // When in tablet mode set the height of the tile viewer to be 40% of its width
                    xyTileControl.sciChart.YAxis.VisibleRange.SetMinMax(((minY - yoffset) * 0.4), ((maxY + yoffset) * 0.4));
                }
            }
        }

        /// <summary>
        /// Rests the scan area view.
        /// </summary>
        public void RestScanAreaView()
        {
            if (_scanArea != null && xyTileControl.sciChart.XAxis.VisibleRange != null && xyTileControl.sciChart.YAxis.VisibleRange != null)
            {
                double offset = Math.Max(xyTileControl.ScanAreaWidth, xyTileControl.ScanAreaHeight);
                xyTileControl.sciChart.XAxis.VisibleRange.SetMinMax(Convert.ToDouble(_scanArea.X1) - 8 * offset, Convert.ToDouble(_scanArea.X2) + 8 * offset);
                xyTileControl.sciChart.YAxis.VisibleRange.SetMinMax(Convert.ToDouble(_scanArea.Y2) - 8 * offset, Convert.ToDouble(_scanArea.Y1) + 8 * offset);
                if (ResourceManagerCS.Instance.TabletModeEnabled)
                {
                    // When in tablet mode set the height of the tile viewer to be 40% of its width
                    xyTileControl.sciChart.YAxis.VisibleRange.SetMinMax(Convert.ToDouble(_scanArea.Y2) - ((8 * offset) * 0.4), Convert.ToDouble(_scanArea.Y1) + ((8 * offset) * 0.4));
                }
            }
        }

        /// <summary>
        /// Adds the new sample carrier to ApplicationSettings.XML.
        /// </summary>
        /// <param name="applicationSettingsPath">The application settings XML path.</param>
        /// <param name="carrier">The carrier.</param>
        public void SaveNewSampleCarrierToApplicationSettings(String applicationSettingsPath, Carrier carrier, ObservableCollection<Carrier> sampleCarrierCollection)
        {
            XmlDocument appSettings = new XmlDocument();
            appSettings.Load(applicationSettingsPath);
            XmlElement newCarrier;
            var node = appSettings.SelectSingleNode("/ApplicationSettings/SampleCarrier");
            if (node != null)
            {
                newCarrier = appSettings.CreateElement(string.Empty, "Carrier".ToString(), string.Empty);
                newCarrier.SetAttribute("Name", carrier.Name);
                newCarrier.SetAttribute("Type", carrier.Type.ToString());
                newCarrier.SetAttribute("Height", carrier.Height.ToString());
                newCarrier.SetAttribute("Width", carrier.Width.ToString());
                newCarrier.SetAttribute("Row", carrier.Template.Row.ToString());
                newCarrier.SetAttribute("Col", carrier.Template.Col.ToString());
                newCarrier.SetAttribute("Diameter", carrier.Template.Diameter.ToString());
                newCarrier.SetAttribute("CenterToCenterX", carrier.Template.CenterToCenterX.ToString());
                newCarrier.SetAttribute("CenterToCenterY", carrier.Template.CenterToCenterY.ToString());
                newCarrier.SetAttribute("TopLeftCenterOffsetX", carrier.Template.TopLeftCenterOffsetX.ToString());
                newCarrier.SetAttribute("TopLeftCenterOffsetY", carrier.Template.TopLeftCenterOffsetY.ToString());
                newCarrier.SetAttribute("WellShape", (carrier.Template.Shape).ToString());
                newCarrier.SetAttribute("WellWidth", carrier.Template.Width.ToString());
                newCarrier.SetAttribute("WellHeight", carrier.Template.Height.ToString());
                node.AppendChild(newCarrier);
                sampleCarrierCollection.Add(carrier); // add new Sample Carrier Template to the carrier list
                appSettings.Save(applicationSettingsPath);
            }
        }

        /// <summary>
        /// Shows the tiles grid.
        /// </summary>
        /// <param name="isEnabled">The is enabled.</param>
        public void ShowTilesGrid(bool? isEnabled)
        {
            // If user has too many tiles, create warning messagebox.
            long totalTilesNumber = 0;
            for (int k = 0; k < xyTileControl.XYtableData.Count; k++)
            {
                totalTilesNumber += Convert.ToInt32(xyTileControl.XYtableData[k].TileCol) * Convert.ToInt32(xyTileControl.XYtableData[k].TileRow);
            }
            if (totalTilesNumber >= 2000 && isEnabled == true)
            {
                if (MessageBox.Show("Showing Grid may lead to lower performance. Do you still want to continue?", "Lower Performance Warning", MessageBoxButton.YesNo, MessageBoxImage.Warning) == MessageBoxResult.No)
                {
                    xyTileControl.ShowGrid.IsChecked = false;
                    return;
                }
            }
            ResetSampleInScichart();

            XmlDocument appSettings = new XmlDocument();
            appSettings.Load(ResourceManagerCS.GetApplicationSettingsFileString());
            XmlNodeList ndList = appSettings.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/XYView");
            if (ndList != null && ndList.Count > 0)
            {
                XmlManager.SetAttribute(ndList[0], appSettings, "showTileGrid", ((xyTileControl.ShowGrid.IsChecked == true) ? 1 : 0).ToString());
            }
            appSettings.Save(ResourceManagerCS.GetApplicationSettingsFileString());
        }

        /// <summary>
        /// sorting algorithm (well plate only)
        /// even row: forward  1 2 3 4 5 6
        /// odd  row: backward 6 5 4 3 2 1
        /// </summary>
        /// <param name="xytableData">The xytable data.</param>
        /// <param name="xyPosition">The xy position.</param>
        /// <returns>The Position to insert</returns>
        public int SortXYTableData(ObservableCollection<XYPosition> xytableData, XYPosition xyPosition)
        {
            int addPosition = 0;
            WellPosition newTilesPosition = xyTileControl.GetWellsPositionFromTag(xyPosition.Well.ToString());
            for (int i = 0; i < xytableData.Count; i++)
            {
                WellPosition position = xyTileControl.GetWellsPositionFromTag(xytableData[i].Well.ToString());
                if (newTilesPosition.Row > position.Row)
                {
                    addPosition++;
                }
                else if (newTilesPosition.Row == position.Row)
                {
                    if (position.Row % 2 != 0)
                    {
                        if (position.Column > newTilesPosition.Column)
                        {
                            addPosition++;
                        }
                    }
                    else
                    {
                        if (position.Column < newTilesPosition.Column)
                        {
                            addPosition++;
                        }
                    }
                }
                else if (newTilesPosition.Row < position.Row)
                {
                    break;
                }
            }
            return addPosition;
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
            if (singleTile == true)// Add single tile
            {
                xyPosition = new XYPosition()
                {
                    Name = "Tiles",
                    IsEnabled = true,
                    X = String.Format("{0:N4}", xyTileControl.ScanAreaXPosition - xyTileControl.HomePosX),
                    Y = String.Format("{0:N4}", xyTileControl.ScanAreaYPosition - xyTileControl.HomePosY),
                    Z = String.Format("{0:N4}", Convert.ToDouble(xyTileControl.ScanAreaZPosition.ToString()) - xyTileControl.HomePosZ),
                    TileCol = "1",
                    TileRow = "1",
                    OverlapX = xyTileControl.DefaultOverlapX.ToString(),
                    OverlapY = xyTileControl.DefaultOverlapY.ToString(),
                    Well = xyTileControl.GetWellsTagByPoint(new Point(xyTileControl.ScanAreaXPosition, xyTileControl.ScanAreaYPosition), xyTileControl.CurrentCarrier)
                };
            }
            if (xytableData.Contains(xyPosition))
            {
                return;
            }
            if (index == -1)// index == -1 means no specfic position needed to insert the new tile, put to the sorting algorithm
            {
                if (xyTileControl.CurrentCarrier.Type != CarrierType.Slide)
                {
                    xytableData.Insert(SortXYTableData(xytableData, xyPosition), xyPosition);
                }
                else
                {
                    xytableData.Add(xyPosition);
                }
            }
            else
            {
                xytableData.Insert(index, xyPosition);
            }

            if (xyPosition.IsEnabled == false || _loaded == false) // if the isEnabled is false, dont need to show on the GUI
            {
                return;
            }

            if (_isSinglePositionEnabled == true)// If single position mode is enabled, always enable the latest one
            {
                var previousDisplayTiles = xytableData.Where(p => p.IsEnabled == true && !p.Equals(xyPosition)).FirstOrDefault();
                if (previousDisplayTiles != null)
                {
                    int previousDisplayTilesIndex = xytableData.IndexOf(previousDisplayTiles as XYPosition);
                    previousDisplayTiles.IsEnabled = false;
                    XYTable_DeleteItem(xytableData, previousDisplayTilesIndex);
                    XYTable_AddItem(xytableData, previousDisplayTiles, previousDisplayTilesIndex);
                }
            }

            double overlapX = Convert.ToDouble(xyPosition.OverlapX) / 100;
            double overlapY = Convert.ToDouble(xyPosition.OverlapY) / 100;
            double col = Convert.ToDouble(xyPosition.TileCol);
            double row = Convert.ToDouble(xyPosition.TileRow);

            if (xyTileControl.ShowGrid.IsChecked == true)
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
                            ContextMenu = (ContextMenu)xyTileControl.FindResource("TileContextMenu"),
                            BorderBrush = Brushes.Ivory,
                            BorderThickness = new Thickness(1),
                            Opacity = 0.8,
                            Tag = xyPosition,
                        };
                        selectArea.PreviewMouseLeftButtonDown += selectGridArea_PreviewMouseLeftButtonDown;
                        xyTileControl.sciChart.Annotations.Add(selectArea);
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
                selectArea.PreviewMouseLeftButtonDown += selectArea_PreviewMouseLeftButtonDown;
                xyTileControl.sciChart.Annotations.Add(selectArea);
            }
        }

        /// <summary>
        /// Xies the table_ delete item.
        /// </summary>
        /// <param name="xytableData">The xytable data.</param>
        /// <param name="tiledAreaIndex">Index of the tiled area.</param>
        /// <param name="tiles">The tiles.</param>
        public void XYTable_DeleteItem(ObservableCollection<XYPosition> xytableData, int tiledAreaIndex)
        {
            //the count of table items is smaller than the delete_index
            if (xytableData.Count < Math.Max(0, tiledAreaIndex))
            {
                return;
            }
            // get the total number of tiles need to be deleted
            int numOfTiles = Convert.ToInt32(xytableData[tiledAreaIndex].TileRow) * Convert.ToInt32(xytableData[tiledAreaIndex].TileCol);

            for (int i = 0; i < xyTileControl.sciChart.Annotations.Count; i++)
            {
                BoxAnnotation tiledArea = xyTileControl.sciChart.Annotations[i] as BoxAnnotation;

                if (tiledArea.Tag != null && xytableData[tiledAreaIndex].Equals(tiledArea.Tag as XYPosition))// match the tag
                {
                    if (xyTileControl.TileModifier.ResizeEnabled && tiledArea.IsSelected)
                    {
                        numOfTiles = 1;
                        xyTileControl.TileModifier.Clear();
                    }

                    if (xyTileControl.ShowGrid.IsChecked == true)
                    {
                        for (int j = 0; j < numOfTiles; j++)
                        {
                            xyTileControl.sciChart.Annotations.RemoveAt(i);//The tile in the gounp of tiles will be continous ,so tiles can be deleted one time.
                        }
                    }
                    else
                    {
                        xyTileControl.sciChart.Annotations.RemoveAt(i);
                    }
                    break;
                }
            }
            xytableData.RemoveAt(tiledAreaIndex);
        }

        /// <summary>
        /// Xies the table_ edit item.
        /// </summary>
        /// <param name="xytableData">The xytable data.</param>
        /// <param name="selectedRow">The selected row.</param>
        /// <param name="selectedColumn">The selected column.</param>
        /// <param name="newValue">The new value.</param>
        public void XYTable_EditItem(ObservableCollection<XYPosition> xytableData, int selectedRow, int selectedColumn, object newValue)
        {
            if (selectedRow >= xytableData.Count || selectedColumn >= 9)
            {
                return;
            }
            int resizeIndex = -1;
            XYPosition xyPosition = new XYPosition()
            {
                Name = xytableData[selectedRow].Name,
                IsEnabled = xytableData[selectedRow].IsEnabled,
                X = xytableData[selectedRow].X,
                Y = xytableData[selectedRow].Y,
                Z = xytableData[selectedRow].Z,
                TileCol = xytableData[selectedRow].TileCol,
                TileRow = xytableData[selectedRow].TileRow,
                OverlapX = xytableData[selectedRow].OverlapX,
                OverlapY = xytableData[selectedRow].OverlapY,
                Well = xytableData[selectedRow].Well
            };
            if (xyTileControl.TileModifier.ResizeEnabled)
            {
                BoxAnnotation box = xyTileControl.TileModifier.GetActiveTile();
                if (box != null)
                {
                    if ((box.Tag as XYPosition).Equals(xyPosition))
                    {
                        if (selectedColumn == 0)
                        {
                            TileModifier_FinishResizingEvent();
                        }
                        else
                        {
                            resizeIndex = xytableData.IndexOf(xyPosition);
                        }

                    }
                    else if (_isSinglePositionEnabled == true)
                    {
                        TileModifier_FinishResizingEvent();
                    }
                }
            }
            //0:enable  1:X  2: Name 3:Y  4:Z  5:Tile Col  6:Tile Row  7:OverlapX 8:OverlapY 9: Well
            switch (selectedColumn)
            {
                case 0:
                    {
                        xyPosition.IsEnabled = Convert.ToBoolean(newValue);
                        if (_isSinglePositionEnabled == true)
                        {
                            if (xyPosition.IsEnabled == true)
                            {
                                var previousDisplayTiles = xytableData.Where(p => p.IsEnabled == true && !p.Equals(xyPosition)).FirstOrDefault();
                                if (previousDisplayTiles != null)
                                {
                                    int previousDisplayTilesIndex = xytableData.IndexOf(previousDisplayTiles as XYPosition);
                                    previousDisplayTiles.IsEnabled = false;
                                    XYTable_DeleteItem(xytableData, previousDisplayTilesIndex);
                                    XYTable_AddItem(xytableData, previousDisplayTiles, previousDisplayTilesIndex);
                                }
                            }
                        }
                    } break;//When user change the enable setting, don't do anything.
                case 1: xyPosition.Name = newValue.ToString(); break; // name
                case 2:
                    {
                        xyPosition.X = newValue.ToString();
                        xyPosition.Well = xyTileControl.GetWellsTagByPoint(new Point(Convert.ToDouble(xyPosition.X) + xyTileControl.HomePosX, Convert.ToDouble(xyPosition.Y) + xyTileControl.HomePosY), xyTileControl.CurrentCarrier);
                    } break;
                case 3:
                    {
                        xyPosition.Y = newValue.ToString();
                        xyPosition.Well = xyTileControl.GetWellsTagByPoint(new Point(Convert.ToDouble(xyPosition.X) + xyTileControl.HomePosX, Convert.ToDouble(xyPosition.Y) + xyTileControl.HomePosY), xyTileControl.CurrentCarrier);
                    } break;
                case 4: xyPosition.Z = newValue.ToString(); break;
                case 5: xyPosition.TileCol = newValue.ToString(); break;
                case 6: xyPosition.TileRow = newValue.ToString(); break;
                case 7: xyPosition.OverlapX = newValue.ToString(); break;
                case 8: xyPosition.OverlapY = newValue.ToString(); break;
                default:
                    break;
            }

            if (resizeIndex != -1)
            {
                xytableData[resizeIndex] = xyPosition;

                double overlapX = Convert.ToDouble(xyPosition.OverlapX) / 100;
                double overlapY = Convert.ToDouble(xyPosition.OverlapY) / 100;
                double col = Convert.ToDouble(xyPosition.TileCol);
                double row = Convert.ToDouble(xyPosition.TileRow);

                BoxAnnotation box = xyTileControl.TileModifier.GetActiveTile();
                box.X1 = Convert.ToDouble(xyPosition.X) + xyTileControl.HomePosX - (col - (col - 1) * overlapX) * xyTileControl.ScanAreaWidth / 2;
                box.Y1 = Convert.ToDouble(xyPosition.Y) + xyTileControl.HomePosY + (row - (row - 1) * overlapY) * xyTileControl.ScanAreaHeight / 2;
                box.X2 = Convert.ToDouble(xyPosition.X) + xyTileControl.HomePosX + (col - (col - 1) * overlapX) * xyTileControl.ScanAreaWidth / 2;
                box.Y2 = Convert.ToDouble(xyPosition.Y) + xyTileControl.HomePosY - (row - (row - 1) * overlapY) * xyTileControl.ScanAreaHeight / 2;
                box.Tag = xyPosition;
                return;
            }
            //delete the current one
            XYTable_DeleteItem(xytableData, selectedRow);
            XYTable_AddItem(xytableData, xyPosition, selectedRow);
        }

        public void XYTable_SelectItem(ObservableCollection<XYPosition> xytableData, int index)
        {
            if (xyTileControl.sciChart.XAxis.VisibleRange != null && xyTileControl.sciChart.YAxis.VisibleRange != null && !ResourceManagerCS.Instance.TabletModeEnabled)
            {
                double overlapX = Convert.ToDouble(xytableData[index].OverlapX) / 100;
                double overlapY = Convert.ToDouble(xytableData[index].OverlapY) / 100;
                double col = Convert.ToDouble(xytableData[index].TileCol);
                double row = Convert.ToDouble(xytableData[index].TileRow);

                double offset = Math.Max(xyTileControl.ScanAreaWidth, xyTileControl.ScanAreaHeight);
                xyTileControl.sciChart.XAxis.VisibleRange.SetMinMax(Convert.ToDouble(xytableData[index].X) + xyTileControl.HomePosX - (col - (col - 1) * overlapX) * xyTileControl.ScanAreaWidth / 2 - offset / 2 * 17, Convert.ToDouble(xytableData[index].X) + xyTileControl.HomePosX - (col - (col - 1) * overlapX) * xyTileControl.ScanAreaWidth / 2 + offset / 2 * 17);
                xyTileControl.sciChart.YAxis.VisibleRange.SetMinMax(Convert.ToDouble(xytableData[index].Y) + xyTileControl.HomePosY + (row - (row - 1) * overlapY) * xyTileControl.ScanAreaHeight / 2 - offset / 2 * 17, Convert.ToDouble(xytableData[index].Y) + xyTileControl.HomePosY + (row - (row - 1) * overlapY) * xyTileControl.ScanAreaHeight / 2 + offset / 2 * 17);
            }
        }

        /// <summary>
        /// xes the ytile view model_ loaded.
        /// </summary>
        public void XYtileViewModel_Loaded()
        {
            if (_loaded) return; //Counteracts the double 'Loaded' event issue.

            //initiate parameters
            SelectedWellsCollection = new ObservableCollection<WellPosition>();
            _duplicatedTiles = new XYPosition();
            _isSinglePositionEnabled = false;
            xyTileControl.SinglePosition.IsChecked = false;
            xyTileControl.SampleCarrierCollection = new ObservableCollection<Carrier>();
            xyTileControl.IsControlPanelVisible = Visibility.Visible;
            xyTileControl.IsTileTableVisible = Visibility.Visible;
            xyTileControl.IsDataGridReadOnly = false;
            xyTileControl.Control.Width = 390;
            xyTileControl.sciChart.Width = 350;
            xyTileControl.sciChart.Height = 350;
            xyTileControl.sciChart.HorizontalAlignment = HorizontalAlignment.Center;
            xyTileControl.ZoomPanModifier.IsEnabled = true;
            xyTileControl.IsChartDragSelectModifierEnabled = true;
            xyTileControl.MouseWheelZoomModifier.IsEnabled = true;
            xyTileControl.PinchZoomModifier.IsEnabled = true;
            //register to the event
            xyTileControl.Load += xyTileControl_Load;
            xyTileControl.XPositionChanged += xyTileControl_XPositionChanged;
            xyTileControl.YPositionChanged += xyTileControl_YPositionChanged;
            xyTileControl.XYStageRangeChanged += xyTileControl_XYStageRangeChanged;
            xyTileControl.ScanAreaSizeChanged += xyTileControl_ScanAreaSizeChanged;
            xyTileControl.SelectModifier.SelectAreaEvent += SelectModifier_SelectAreaEvent;
            xyTileControl.TileModifier.FinishResizingEvent += TileModifier_FinishResizingEvent;
            xyTileControl.WellSelectedStatusChanged += xyTileControl_WellSelectedStatusChanged;
            xyTileControl.DeselectAllWellsEvent += xyTileControl_DeselectAllWellsEvent;
            xyTileControl.SinglePositionCheckedStatusChanged += xyTileControl_SinglePositionCheckedStatusChanged;
            xyTileControl.TilesPositionChanged += xyTileControl_TilesPositionChanged;
            xyTileControl.CenterTiles += xyTileControl_CenterTiles;
            xyTileControl.CopyTiles += xyTileControl_CopyTiles;
            xyTileControl.sciChart.TouchDown += sciChart_TouchDown;
            xyTileControl.sciChart.TouchUp += sciChart_TouchUp;
            xyTileControl.sciChart.TouchMove += sciChart_TouchMove;
            xyTileControl.sciChart.TouchLeave += sciChart_TouchLeave;

            xyTileControl.SelectModifier.ScanAreaWidth = xyTileControl.ScanAreaWidth;
            xyTileControl.SelectModifier.ScanAreaHeight = xyTileControl.ScanAreaHeight;
            xyTileControl.SelectModifier.LeavePanelEvent += SelectModifier_LeavePanelEvent;
            xyTileControl.SelectModifier.SelectEnabled = false;
            xyTileControl.SelectModifier.TileEnabled = false;

            if (ResourceManagerCS.Instance.TabletModeEnabled)
            {
                // When in tablet mode set the height of the tile viewer to be 40% of its width
                xyTileControl.sciChart.Height = xyTileControl.sciChart.Width * 0.4;
            }

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
            //Define the range area represent the boundary of MCM3000 moving area. Only boundary is displayed.
            _rangeArea = new BoxAnnotation()
            {
                Background = Brushes.Transparent,
                BorderBrush = Brushes.Red,
                BorderThickness = new Thickness(2),
                X1 = xyTileControl.MinX,
                X2 = xyTileControl.MaxX,
                Y1 = xyTileControl.MaxY,
                Y2 = xyTileControl.MinY,
                Tag = "Irremovable",
                Visibility = Visibility.Visible
            };
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
            //Load the Carriers defined in the Application Setting XML.
            LoadApplcationSettings(xyTileControl.ApplicationSettings, xyTileControl.SampleCarrierCollection);
            //Load Active.XML
            LoadExperimentSettings(xyTileControl.ActiveXML, xyTileControl.SampleCarrierCollection);

            _loaded = true;

            ResetSampleInScichart();
            ResetSampleView();
        }

        /// <summary>
        /// xes the ytile view model_ unloaded.
        /// </summary>
        public void XYtileViewModel_Unloaded()
        {
            //SaveActiveXMLSettings(xyTileControl.ActiveXML);

            //Clear all the well selections
            DeselectWells();

            xyTileControl.Load -= xyTileControl_Load;
            xyTileControl.SelectModifier.SelectAreaEvent -= SelectModifier_SelectAreaEvent;
            xyTileControl.TileModifier.FinishResizingEvent -= TileModifier_FinishResizingEvent;
            xyTileControl.XYStageRangeChanged -= xyTileControl_XYStageRangeChanged;
            xyTileControl.ScanAreaSizeChanged -= xyTileControl_ScanAreaSizeChanged;
            xyTileControl.XPositionChanged -= xyTileControl_XPositionChanged;
            xyTileControl.YPositionChanged -= xyTileControl_YPositionChanged;
            xyTileControl.WellSelectedStatusChanged -= xyTileControl_WellSelectedStatusChanged;
            xyTileControl.DeselectAllWellsEvent -= xyTileControl_DeselectAllWellsEvent;
            xyTileControl.SinglePositionCheckedStatusChanged -= xyTileControl_SinglePositionCheckedStatusChanged;
            xyTileControl.TilesPositionChanged -= xyTileControl_TilesPositionChanged;
            xyTileControl.CenterTiles -= xyTileControl_CenterTiles;
            xyTileControl.SelectModifier.LeavePanelEvent -= SelectModifier_LeavePanelEvent;
            xyTileControl.SelectModifier.TileEnabled = false;
            xyTileControl.SelectModifier.SelectEnabled = false;
            xyTileControl.TileEnabled.IsChecked = false;
            xyTileControl.SelectionEnabled.IsChecked = false;
            _loaded = false;
        }

        /// <summary>
        /// Converts the old experiment settings.
        /// </summary>
        /// <param name="expXMLPath">The exp XML path.</param>
        private void ConvertOldExperimentSettings(string expXMLPath)
        {
            if (expXMLPath == null)
            {
                return;
            }
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
                            int type = Convert.ToInt32(sampleRootNode.Attributes["type"].Value); //checked
                            double homeOffsetX = Convert.ToDouble(sampleRootNode.Attributes["offsetXMM"].Value) * -1;//checked
                            double homeOffsetY = Convert.ToDouble(sampleRootNode.Attributes["offsetYMM"].Value) * -1;//checked
                            sampleRootNode.Attributes.RemoveAll();
                            Carrier currentCarrier = _oldCarrierCollection[type];
                            String carrierType = (currentCarrier.Type == CarrierType.Slide) ? "Slide" : "Multiwell";
                            XmlManager.SetAttribute(sampleRootNode, expXML, "name", currentCarrier.Name);
                            XmlManager.SetAttribute(sampleRootNode, expXML, "type", carrierType);
                            XmlManager.SetAttribute(sampleRootNode, expXML, "width", currentCarrier.Width.ToString());
                            XmlManager.SetAttribute(sampleRootNode, expXML, "height", currentCarrier.Height.ToString());
                            XmlManager.SetAttribute(sampleRootNode, expXML, "row", currentCarrier.Template.Row.ToString());
                            XmlManager.SetAttribute(sampleRootNode, expXML, "column", currentCarrier.Template.Col.ToString());
                            XmlManager.SetAttribute(sampleRootNode, expXML, "diameter", currentCarrier.Template.Diameter.ToString());
                            XmlManager.SetAttribute(sampleRootNode, expXML, "centerToCenterX", currentCarrier.Template.CenterToCenterX.ToString());
                            XmlManager.SetAttribute(sampleRootNode, expXML, "centerToCenterY", currentCarrier.Template.CenterToCenterY.ToString());
                            XmlManager.SetAttribute(sampleRootNode, expXML, "topLeftCenterOffsetX", currentCarrier.Template.TopLeftCenterOffsetX.ToString());
                            XmlManager.SetAttribute(sampleRootNode, expXML, "topLeftCenterOffsetY", currentCarrier.Template.TopLeftCenterOffsetY.ToString());
                            XmlManager.SetAttribute(sampleRootNode, expXML, "WellShape", (currentCarrier.Template.Shape).ToString());
                            XmlManager.SetAttribute(sampleRootNode, expXML, "WellWidth", currentCarrier.Template.Width.ToString());
                            XmlManager.SetAttribute(sampleRootNode, expXML, "WellHeight", currentCarrier.Template.Height.ToString());

                            XmlManager.SetAttribute(sampleRootNode, expXML, "homeOffsetX", homeOffsetX.ToString());
                            XmlManager.SetAttribute(sampleRootNode, expXML, "homeOffsetY", homeOffsetY.ToString());
                            XmlManager.SetAttribute(sampleRootNode, expXML, "homeOffsetZ", xyTileControl.ScanAreaZPosition.ToString());
                            //convert well settings
                            var wellNode = expXML.SelectSingleNode("/ThorImageExperiment/Sample/Wells");
                            if (wellNode != null)
                            {
                                int wellRow = 0;
                                int wellColumn = 0;

                                string tempStr = string.Empty;

                                if (currentCarrier.Type == CarrierType.Multiwell)
                                {
                                    if (XmlManager.GetAttribute(wellNode, expXML, "startRow", ref tempStr))
                                    {
                                        wellRow = (Convert.ToInt32(tempStr)) - 1;
                                    }

                                    if (XmlManager.GetAttribute(wellNode, expXML, "startColumn", ref tempStr))
                                    {
                                        wellColumn = (Convert.ToInt32(tempStr)) - 1;
                                    }
                                }
                                wellNode.Attributes.RemoveAll();
                                XmlManager.SetAttribute(wellNode, expXML, "location", xyTileControl.TagWells(wellRow, wellColumn));
                                //convert subimage settings
                                var subImageNode = expXML.SelectSingleNode("/ThorImageExperiment/Sample/Wells/SubImages");
                                if (subImageNode != null)
                                {
                                    var lsmNode = expXML.SelectSingleNode("/ThorImageExperiment/LSM");
                                    double scanAreaWidth = 0.01;
                                    double scanAreaHeight = 0.01;
                                    string pixelX = string.Empty;
                                    string pixelY = string.Empty;
                                    string pixelSizeUM = string.Empty;

                                    if (XmlManager.GetAttribute(lsmNode, expXML, "pixelX", ref pixelX) && XmlManager.GetAttribute(lsmNode, expXML, "pixelY", ref pixelY) && XmlManager.GetAttribute(lsmNode, expXML, "pixelSizeUM", ref pixelSizeUM)
                                        && Convert.ToDouble(pixelX) != 0 && Convert.ToDouble(pixelSizeUM) != 0 && Convert.ToDouble(pixelY) != 0)
                                    {
                                        scanAreaWidth = Math.Round(Convert.ToDouble(pixelX) * Convert.ToDouble(pixelSizeUM)) / 1000;
                                        scanAreaHeight = Math.Round(Convert.ToDouble(pixelY) * Convert.ToDouble(pixelSizeUM)) / 1000;
                                    }
                                    else
                                    {
                                        scanAreaWidth = 0.12;
                                        scanAreaHeight = 0.12;
                                    }

                                    double overlapX = 0;
                                    double overlapY = 0;
                                    int rows = 0;
                                    int columns = 0;

                                    if (XmlManager.GetAttribute(subImageNode, expXML, "subOffsetXMM", ref tempStr))
                                    {
                                        overlapX = 1 - (Convert.ToDouble(tempStr) / scanAreaWidth);
                                    }
                                    if (XmlManager.GetAttribute(subImageNode, expXML, "subOffsetYMM", ref tempStr))
                                    {
                                        overlapY = 1 - (Convert.ToDouble(tempStr) / scanAreaHeight);
                                    }
                                    if (XmlManager.GetAttribute(subImageNode, expXML, "subRows", ref tempStr))
                                    {
                                        rows = Convert.ToInt32(tempStr);
                                    }
                                    if (XmlManager.GetAttribute(subImageNode, expXML, "subColumns", ref tempStr))
                                    {
                                        columns = Convert.ToInt32(tempStr);
                                    }

                                    double X = 0;
                                    double Y = 0;
                                    double Z = (Convert.ToDouble(String.Format("{0:N4}", xyTileControl.ScanAreaZPosition.ToString())));

                                    if (XmlManager.GetAttribute(subImageNode, expXML, "transOffsetXMM", ref tempStr))
                                    {
                                        X = (Convert.ToDouble((String.Format("{0:N4}", tempStr))) * -1) + (columns - (columns - 1) * (overlapX / 100)) * scanAreaWidth / 2;
                                    }
                                    if (XmlManager.GetAttribute(subImageNode, expXML, "transOffsetYMM", ref tempStr))
                                    {
                                        Y = (Convert.ToDouble((String.Format("{0:N4}", tempStr))) * -1) - (rows - (rows - 1) * (overlapY / 100)) * scanAreaHeight / 2;
                                    }
                                    subImageNode.Attributes.RemoveAll();
                                    XmlManager.SetAttribute(subImageNode, expXML, "name", "Tiles");
                                    XmlManager.SetAttribute(subImageNode, expXML, "isEnabled", "True");
                                    XmlManager.SetAttribute(subImageNode, expXML, "subRows", rows.ToString());
                                    XmlManager.SetAttribute(subImageNode, expXML, "subColumns", columns.ToString());
                                    XmlManager.SetAttribute(subImageNode, expXML, "transOffsetXMM", X.ToString());
                                    XmlManager.SetAttribute(subImageNode, expXML, "transOffsetYMM", Y.ToString());
                                    XmlManager.SetAttribute(subImageNode, expXML, "transOffsetZMM", Z.ToString());
                                    XmlManager.SetAttribute(subImageNode, expXML, "overlapX", overlapX.ToString());
                                    XmlManager.SetAttribute(subImageNode, expXML, "overlapY", overlapY.ToString());
                                }
                            }
                        }
                    }
                    catch (Exception ex)
                    {
                        ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "XYTileControl, Failed to convert old experiment settings. Exception thrown: " + ex.Message);
                    }
                }
            }
            expXML.Save(expXMLPath);
        }

        /// <summary>
        /// Deselects all wells
        /// </summary>
        private void DeselectWells()
        {
            SelectedWellsCollection.Clear();
            for (int i = 0; i < xyTileControl.sciChart.Annotations.Count; i++)
            {
                BoxAnnotation well = xyTileControl.sciChart.Annotations[i] as BoxAnnotation;
                if (well.ContextMenu != null && well.ContextMenu == (ContextMenu)xyTileControl.FindResource("WellContextMenu"))
                {
                    well.Background = Brushes.AliceBlue;
                }
            }
        }

        void GridBackgroundBox_DragEnded(object sender, EventArgs e)
        {
            _draggingMouse = false;
        }

        void GridBackgroundBox_DragStarted(object sender, EventArgs e)
        {
            _draggingMouse = true;
        }

        void GridBackgroundBox_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            //Only allow recalculation of Grid columns and rows if the size of the grid was manually changed
            if (_draggingMouse)
            {
                _gridChangedSize = true;
            }
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
                if ((null != expXMLPath && string.Empty != expXMLPath) && File.Exists(expXMLPath))
                {
                    ConvertOldExperimentSettings(expXMLPath);
                    //Load Active.XML
                    XmlDocument expXML = new XmlDocument();
                    expXML.Load(expXMLPath);
                    var sampleRootNode = expXML.SelectSingleNode("/ThorImageExperiment/Sample");

                    if (sampleRootNode != null)
                    {
                        Carrier currentCarrier = sampleCarrierCollection[0];//preset to slide

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
                            currentCarrier.Name = tempStr;
                        }
                        if (XmlManager.GetAttribute(sampleRootNode, expXML, "type", ref tempStr))
                        {
                            currentCarrier.Type = (CarrierType)System.Enum.Parse(typeof(CarrierType), tempStr);
                        }
                        if (XmlManager.GetAttribute(sampleRootNode, expXML, "width", ref tempStr))
                        {
                            currentCarrier.Width = Convert.ToDouble(tempStr);
                        }
                        if (XmlManager.GetAttribute(sampleRootNode, expXML, "height", ref tempStr))
                        {
                            currentCarrier.Height = Convert.ToDouble(tempStr);
                        }
                        if (XmlManager.GetAttribute(sampleRootNode, expXML, "row", ref tempStr))
                        {
                            currentCarrier.Template.Row = Convert.ToInt32(tempStr);
                        }
                        if (XmlManager.GetAttribute(sampleRootNode, expXML, "column", ref tempStr))
                        {
                            currentCarrier.Template.Col = Convert.ToInt32(tempStr);
                        }
                        if (XmlManager.GetAttribute(sampleRootNode, expXML, "diameter", ref tempStr))
                        {
                            currentCarrier.Template.Diameter = Convert.ToDouble(tempStr);
                        }
                        if (XmlManager.GetAttribute(sampleRootNode, expXML, "centerToCenterX", ref tempStr))
                        {
                            currentCarrier.Template.CenterToCenterX = Convert.ToDouble(tempStr);
                        }
                        if (XmlManager.GetAttribute(sampleRootNode, expXML, "centerToCenterY", ref tempStr))
                        {
                            currentCarrier.Template.CenterToCenterY = Convert.ToDouble(tempStr);
                        }
                        if (XmlManager.GetAttribute(sampleRootNode, expXML, "topLeftCenterOffsetX", ref tempStr))
                        {
                            currentCarrier.Template.TopLeftCenterOffsetX = Convert.ToDouble(tempStr);
                        }
                        if (XmlManager.GetAttribute(sampleRootNode, expXML, "topLeftCenterOffsetY", ref tempStr))
                        {
                            currentCarrier.Template.TopLeftCenterOffsetY = Convert.ToDouble(tempStr);
                        }
                        if (XmlManager.GetAttribute(sampleRootNode, expXML, "WellShape", ref tempStr))
                        {
                            tempStr = (tempStr.Contains("Well") ? tempStr : tempStr + "Well");
                            currentCarrier.Template.Shape = (WellShape)System.Enum.Parse(typeof(WellShape), tempStr);
                        }
                        if (XmlManager.GetAttribute(sampleRootNode, expXML, "WellWidth", ref tempStr))
                        {
                            currentCarrier.Template.Width = Convert.ToDouble(tempStr);
                        }
                        if (XmlManager.GetAttribute(sampleRootNode, expXML, "WellHeight", ref tempStr))
                        {
                            currentCarrier.Template.Height = Convert.ToDouble(tempStr);
                        }

                        if (XmlManager.GetAttribute(sampleRootNode, expXML, "tiltAdjustment", ref tempStr))
                        {
                            xyTileControl.TiltAdjustment = Convert.ToInt32(tempStr);
                        }

                        if (XmlManager.GetAttribute(sampleRootNode, expXML, "fPt1XMM", ref tempStr))
                        {
                            xyTileControl.FP1XYZ[0] = Convert.ToDouble(tempStr);
                        }

                        if (XmlManager.GetAttribute(sampleRootNode, expXML, "fPt1YMM", ref tempStr))
                        {
                            xyTileControl.FP1XYZ[1] = Convert.ToDouble(tempStr);
                        }

                        if (XmlManager.GetAttribute(sampleRootNode, expXML, "fPt1ZMM", ref tempStr))
                        {
                            xyTileControl.FP1XYZ[2] = Convert.ToDouble(tempStr);
                        }
                        if (XmlManager.GetAttribute(sampleRootNode, expXML, "fPt2XMM", ref tempStr))
                        {
                            xyTileControl.FP2XYZ[0] = Convert.ToDouble(tempStr);
                        }

                        if (XmlManager.GetAttribute(sampleRootNode, expXML, "fPt2YMM", ref tempStr))
                        {
                            xyTileControl.FP2XYZ[1] = Convert.ToDouble(tempStr);
                        }

                        if (XmlManager.GetAttribute(sampleRootNode, expXML, "fPt2ZMM", ref tempStr))
                        {
                            xyTileControl.FP2XYZ[2] = Convert.ToDouble(tempStr);
                        }

                        if (XmlManager.GetAttribute(sampleRootNode, expXML, "fPt3XMM", ref tempStr))
                        {
                            xyTileControl.FP3XYZ[0] = Convert.ToDouble(tempStr);
                        }

                        if (XmlManager.GetAttribute(sampleRootNode, expXML, "fPt3YMM", ref tempStr))
                        {
                            xyTileControl.FP3XYZ[1] = Convert.ToDouble(tempStr);
                        }

                        if (XmlManager.GetAttribute(sampleRootNode, expXML, "fPt3ZMM", ref tempStr))
                        {
                            xyTileControl.FP3XYZ[2] = Convert.ToDouble(tempStr);
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

                        //Search the correct carrrier index
                        bool isCarrierFound = false;
                        for (int i = 0; i < sampleCarrierCollection.Count; i++)
                        {
                            if (currentCarrier.Type == sampleCarrierCollection[i].Type &&
                                currentCarrier.Width == sampleCarrierCollection[i].Width &&
                                currentCarrier.Height == sampleCarrierCollection[i].Height &&
                                currentCarrier.Template.CenterToCenterX == sampleCarrierCollection[i].Template.CenterToCenterX &&
                                currentCarrier.Template.CenterToCenterY == sampleCarrierCollection[i].Template.CenterToCenterY &&
                                currentCarrier.Template.Col == sampleCarrierCollection[i].Template.Col &&
                                currentCarrier.Template.Row == sampleCarrierCollection[i].Template.Row &&
                                currentCarrier.Template.Diameter == sampleCarrierCollection[i].Template.Diameter &&
                                currentCarrier.Template.TopLeftCenterOffsetX == sampleCarrierCollection[i].Template.TopLeftCenterOffsetX &&
                                currentCarrier.Template.TopLeftCenterOffsetY == sampleCarrierCollection[i].Template.TopLeftCenterOffsetY &&
                                currentCarrier.Template.Width == sampleCarrierCollection[i].Template.Width &&
                                currentCarrier.Template.Height == sampleCarrierCollection[i].Template.Height)
                            {
                                xyTileControl.SelectedCarrierIndex = i; // If found, assign index to it
                                isCarrierFound = true;
                                break;
                            }
                        }
                        if (isCarrierFound == false) // If not found, save new sampleCarrier.
                        {
                            SaveNewSampleCarrierToApplicationSettings(expXMLPath, currentCarrier, sampleCarrierCollection);
                            xyTileControl.SelectedCarrierIndex = sampleCarrierCollection.Count - 1;
                        }

                        if (currentCarrier.Template.Row > 1 || currentCarrier.Template.Col > 1)
                        {
                            xyTileControl.SetStepToWellSizeVisibility = Visibility.Visible;
                        }
                        else
                        {
                            xyTileControl.SetStepToWellSizeVisibility = Visibility.Collapsed;
                        }
                        string overlapX = string.Empty;
                        string overlapY = string.Empty;

                        if (XmlManager.GetAttribute(sampleRootNode, expXML, "overlapX", ref overlapX))
                        {
                            xyTileControl.DefaultOverlapX = Convert.ToDouble(overlapX);
                        }
                        if (XmlManager.GetAttribute(sampleRootNode, expXML, "overlapY", ref overlapY))
                        {
                            xyTileControl.DefaultOverlapY = Convert.ToDouble(overlapY);
                        }

                        XmlNodeList wellList = expXML.SelectNodes("/ThorImageExperiment/Sample/Wells");

                        if (wellList != null)
                        {
                            string isEnabled = string.Empty;
                            string subRows = string.Empty;
                            string subColumns = string.Empty;
                            string transOffsetXMM = string.Empty;
                            string transOffsetYMM = string.Empty;
                            string transOffsetZMM = string.Empty;
                            string name = string.Empty;

                            string location = string.Empty;
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
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "XYTileControl, Failed to load experiment in 'edit' mode. Exception thrown: " + ex.Message);
            }
        }

        void sciChart_TouchDown(object sender, TouchEventArgs e)
        {
            _touchCount++;
            if (2 < _touchCount)
            {
                _touchCount = 2;
            }
        }

        void sciChart_TouchLeave(object sender, TouchEventArgs e)
        {
            _touchCount--;
            if (0 > _touchCount)
            {
                _touchCount = 0;
            }
        }

        void sciChart_TouchMove(object sender, TouchEventArgs e)
        {
            //If there are two fingers on the screen dissable panning
            if (2 == _touchCount)
            {
                xyTileControl.ZoomPanModifier.IsEnabled = false;
                TimeSpan ts = DateTime.Now - _lastSetTime;

                if (ts.TotalSeconds > .25)
                {
                    xyTileControl.sciChart.OnArrangeSciChart();
                    xyTileControl.sciChart.OnSciChartRendered();
                    _lastSetTime = DateTime.Now;
                }
            }
            else
            {
                xyTileControl.ZoomPanModifier.IsEnabled = true;
            }
        }

        void sciChart_TouchUp(object sender, TouchEventArgs e)
        {
        }

        void selectArea_PreviewMouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            BoxAnnotation box = sender as BoxAnnotation;
            if (box != xyTileControl.TileModifier.GetActiveTile())
            {
                if (xyTileControl.TileModifier.ResizeEnabled == true)
                {
                    BoxAnnotation tempBox = xyTileControl.TileModifier.GetActiveTile();
                    tempBox.IsEditable = false;
                    tempBox.IsSelected = false;
                    xyTileControl.TileModifier.ResizeEnabled = false;
                    TileModifier_FinishResizingEvent();
                }

                xyTileControl.TileModifier.ResizeEnabled = true;
                //If the show grid option is unchecked we need to connect SizeChanged to the box,
                // otherwise _gridChangedSize won't change to true   
                if (false == xyTileControl.ShowGrid.IsChecked)
                {
                    box.SizeChanged += GridBackgroundBox_SizeChanged;
                    box.DragEnded += GridBackgroundBox_DragEnded;
                    box.DragStarted += GridBackgroundBox_DragStarted;
                }
                box.IsEditable = true;
                box.IsSelected = true;
                xyTileControl.TileModifier.SetActiveTile(box);
            }
        }

        void selectGridArea_PreviewMouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            BoxAnnotation tiles = sender as BoxAnnotation;
            XYPosition xyPosition = (XYPosition)tiles.Tag;

            int index = xyTileControl.XYtableData.IndexOf(xyPosition);

            XYTable_DeleteItem(xyTileControl.XYtableData, index);

            xyTileControl.XYtableData.Insert(index, xyPosition);

            double overlapX = Convert.ToDouble(xyPosition.OverlapX) / 100;
            double overlapY = Convert.ToDouble(xyPosition.OverlapY) / 100;
            double col = Convert.ToDouble(xyPosition.TileCol);
            double row = Convert.ToDouble(xyPosition.TileRow);

            BoxAnnotation GridBackgroundBox = new BoxAnnotation()
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

            BoxAnnotation box = xyTileControl.TileModifier.GetActiveTile();
            if (box != null && GridBackgroundBox != box)
            {
                if (xyTileControl.TileModifier.ResizeEnabled == true)
                {
                    box.IsEditable = false;
                    box.IsSelected = false;
                    xyTileControl.TileModifier.ResizeEnabled = false;
                    TileModifier_FinishResizingEvent();
                }
            }

            Canvas.SetZIndex(GridBackgroundBox, 9999);
            xyTileControl.sciChart.Annotations.Add(GridBackgroundBox);

            GridBackgroundBox.PreviewMouseLeftButtonDown += selectArea_PreviewMouseLeftButtonDown;
            GridBackgroundBox.SizeChanged += GridBackgroundBox_SizeChanged;
            GridBackgroundBox.DragEnded += GridBackgroundBox_DragEnded;
            GridBackgroundBox.DragStarted += GridBackgroundBox_DragStarted;
            GridBackgroundBox.IsEditable = true;
            GridBackgroundBox.IsSelected = true;
            xyTileControl.TileModifier.ResizeEnabled = true;
            xyTileControl.TileModifier.SetActiveTile(GridBackgroundBox);
        }

        void SelectModifier_LeavePanelEvent()
        {
            xyTileControl.SelectionEnabled.IsChecked = false;
            xyTileControl.TileEnabled.IsChecked = false;
            xyTileControl.ZoomPanModifier.IsEnabled = true;
        }

        /// <summary>
        /// Selects the modifier_ select area event.
        /// </summary>
        /// <param name="startPoint">The start point.</param>
        /// <param name="endPoint">The end point.</param>
        void SelectModifier_SelectAreaEvent(Point startPoint, Point endPoint)
        {
            if (xyTileControl.SelectModifier.SelectEnabled == true)
            {
                SelectWells(startPoint, endPoint);
            }
            else if (xyTileControl.SelectModifier.TileEnabled == true)
            {
                TilingSelectedArea(startPoint, endPoint);
            }
            else if (xyTileControl.SelectModifier.DuplicateEnabled == true)
            {
                TilingSelectedArea(new Point(startPoint.X + xyTileControl.ScanAreaWidth / 2, startPoint.Y - xyTileControl.ScanAreaHeight / 2), new Point(endPoint.X - xyTileControl.ScanAreaWidth / 2, endPoint.Y + xyTileControl.ScanAreaHeight / 2));
            }
        }

        /// <summary>
        /// Selects the wells.
        /// </summary>
        /// <param name="startPoint">The start point.</param>
        /// <param name="endPoint">The end point.</param>
        private void SelectWells(Point startPoint, Point endPoint)
        {
            Point topLeft = new Point(Math.Min(startPoint.X, endPoint.X), Math.Max(startPoint.Y, endPoint.Y));
            Point bottomRight = new Point(Math.Max(startPoint.X, endPoint.X), Math.Min(startPoint.Y, endPoint.Y));
            if (xyTileControl.SampleCarrierCollection.Count >= 0 && xyTileControl.SelectedCarrierIndex >= 0)
            {
                for (int i = 0; i < xyTileControl.sciChart.Annotations.Count; i++)
                {
                    BoxAnnotation well = xyTileControl.sciChart.Annotations[i] as BoxAnnotation;
                    if (well.ContextMenu != null && well.ContextMenu == (ContextMenu)xyTileControl.FindResource("WellContextMenu"))
                    {
                        if (Convert.ToDouble(well.X2) < topLeft.X || Convert.ToDouble(well.X1) > bottomRight.X
                        || Convert.ToDouble(well.Y1) < bottomRight.Y || Convert.ToDouble(well.Y2) > topLeft.Y)
                        {
                            continue;
                        }
                        if (Convert.ToDouble(well.X1) <= topLeft.X && Convert.ToDouble(well.X2) >= bottomRight.X
                            && Convert.ToDouble(well.Y2) <= bottomRight.Y && Convert.ToDouble(well.Y1) >= topLeft.Y)
                        {
                            well.Background = Brushes.Tomato;
                            SelectedWellsCollection.Add(xyTileControl.GetWellsPositionFromTag(well.Tag.ToString()));
                            continue;
                        }
                        if (xyTileControl.CurrentCarrier.Template.Shape == WellShape.CircleWell)
                        {
                            double centerX = (Convert.ToDouble(well.X2) + Convert.ToDouble(well.X1)) / 2;
                            double centerY = (Convert.ToDouble(well.Y2) + Convert.ToDouble(well.Y1)) / 2;
                            double radius = (Convert.ToDouble(well.X2) - Convert.ToDouble(well.X1)) / 2;

                            for (int degree = 1; degree <= 360; degree++)
                            {
                                double angle = Math.PI * degree / 180;
                                double x = centerX + radius * Math.Cos(angle);
                                double y = centerY + radius * Math.Sin(angle);
                                if (x >= topLeft.X && x <= bottomRight.X
                                & y >= bottomRight.Y & y <= topLeft.Y)
                                {
                                    well.Background = Brushes.Tomato;
                                    SelectedWellsCollection.Add(xyTileControl.GetWellsPositionFromTag(well.Tag.ToString()));
                                    break;
                                }
                            }
                        }
                        else
                        {
                            Rect rectWell = new Rect(new Point(Convert.ToDouble(well.X1), Convert.ToDouble(well.Y1)), new Point(Convert.ToDouble(well.X2), Convert.ToDouble(well.Y2)));
                            Rect rectSelectedArea = new Rect(topLeft, bottomRight);
                            if (rectWell.IntersectsWith(rectSelectedArea))
                            {
                                well.Background = Brushes.Tomato;
                                SelectedWellsCollection.Add(xyTileControl.GetWellsPositionFromTag(well.Tag.ToString()));
                            }
                        }
                    }
                }
            }
        }

        void TileModifier_FinishResizingEvent()
        {
            BoxAnnotation box = xyTileControl.TileModifier.GetActiveTile();
            if (box == null) return;
            XYPosition xyPosition = box.Tag as XYPosition;

            string name = xyPosition.Name;

            int index = xyTileControl.XYtableData.IndexOf((XYPosition)box.Tag);

            xyTileControl.XYtableData.RemoveAt(index);

            xyTileControl.sciChart.Annotations.Remove(box);

            Point topLeft = new Point((double)box.X1, (double)box.Y1);
            Point bottomRight = new Point((double)box.X2, (double)box.Y2);
            double XSpacing = Convert.ToDouble(((XYPosition)box.Tag).OverlapX);
            double YSpacing = Convert.ToDouble(((XYPosition)box.Tag).OverlapY);
            double xSpace = XSpacing / 100.0;
            double ySpace = YSpacing / 100.0;
            double deltaX = Convert.ToDouble(Math.Abs(bottomRight.X - topLeft.X).ToString("N3"));
            double deltaY = Convert.ToDouble(Math.Abs(bottomRight.Y - topLeft.Y).ToString("N3"));
            if (deltaX < xyTileControl.ScanAreaWidth)
            {
                deltaX = xyTileControl.ScanAreaWidth;
            }
            if (deltaY < xyTileControl.ScanAreaHeight)
            {
                deltaY = xyTileControl.ScanAreaHeight;
            }
            int col = Convert.ToInt32(xyPosition.TileCol);
            int row = Convert.ToInt32(xyPosition.TileRow);
            if (xSpace >= 1)
            {
                col = 1;
            }
            else
            {
                if (_gridChangedSize) //only recalculate if grid changed size
                {
                    //need to round to avoid rounding errors when doing the ceiling which could cause adding tiles when unintended
                    if (xSpace <= 0)
                    {
                        col = Convert.ToInt32(Math.Ceiling((Math.Round(deltaX / xyTileControl.ScanAreaWidth) - xSpace) / (1.0 - xSpace)));
                    }
                    else
                    {
                        col = Convert.ToInt32(Math.Floor((Math.Round(deltaX / xyTileControl.ScanAreaWidth) - xSpace) / (1.0 - xSpace)));
                    }
                }
            }

            if (ySpace >= 1)
            {
                row = 1;
            }
            else
            {
                if (_gridChangedSize) //only recalculate if grid changed size
                {
                    //need to round to avoid rounding errors when doing the ceiling which could cause adding tiles when unintended
                    if (ySpace <= 0)
                    {
                        row = Convert.ToInt32(Math.Ceiling((Math.Round(deltaY / xyTileControl.ScanAreaHeight) - ySpace) / (1.0 - ySpace)));
                    }
                    else
                    {
                        row = Convert.ToInt32(Math.Floor((Math.Round(deltaY / xyTileControl.ScanAreaHeight) - ySpace) / (1.0 - ySpace)));
                    }
                }
            }

            xyPosition = new XYPosition()
            {
                Name = name,
                IsEnabled = true,
                X = String.Format("{0:N4}", (topLeft.X + bottomRight.X) / 2 - xyTileControl.HomePosX),
                Y = String.Format("{0:N4}", (topLeft.Y + bottomRight.Y) / 2 - xyTileControl.HomePosY),
                Z = String.Format("{0:N4}", xyTileControl.ScanAreaZPosition - xyTileControl.HomePosZ),
                TileCol = col.ToString(),
                TileRow = row.ToString(),
                OverlapX = XSpacing.ToString(),
                OverlapY = YSpacing.ToString(),
                Well = xyTileControl.GetWellsTagByPoint(new Point((topLeft.X + bottomRight.X) / 2, (topLeft.Y + bottomRight.Y) / 2), xyTileControl.CurrentCarrier)
            };
            XYTable_AddItem(xyTileControl.XYtableData, xyPosition, index);
            _gridChangedSize = false;
            xyTileControl.TileModifier.Clear();
        }

        /// <summary>
        /// Tilings the selected area.
        /// </summary>
        /// <param name="startPoint">The start point.</param>
        /// <param name="endPoint">The end point.</param>
        private void TilingSelectedArea(Point startPoint, Point endPoint)
        {
            Point topLeft = new Point(Math.Min(startPoint.X, endPoint.X), Math.Max(startPoint.Y, endPoint.Y));
            Point bottomRight = new Point(Math.Max(startPoint.X, endPoint.X), Math.Min(startPoint.Y, endPoint.Y));

            XYPosition xyPosition;

            int col = 0;
            int row = 0;
            double XSpacing = 0;
            double YSpacing = 0;

            if (xyTileControl.SelectModifier.DuplicateEnabled == true)
            {
                xyPosition = new XYPosition()
                {
                    Name = "Tiles",
                    IsEnabled = xyTileControl.DuplicatedTiles.IsEnabled,
                    X = String.Format("{0:N4}", (topLeft.X + endPoint.X) / 2 - xyTileControl.HomePosX),
                    Y = String.Format("{0:N4}", (topLeft.Y + endPoint.Y) / 2 - xyTileControl.HomePosY),
                    Z = xyTileControl.DuplicatedTiles.Z,
                    TileCol = xyTileControl.DuplicatedTiles.TileCol,
                    TileRow = xyTileControl.DuplicatedTiles.TileRow,
                    OverlapX = xyTileControl.DuplicatedTiles.OverlapX,
                    OverlapY = xyTileControl.DuplicatedTiles.OverlapY,
                    Well = xyTileControl.GetWellsTagByPoint(new Point((startPoint.X + endPoint.X) / 2, (startPoint.Y + endPoint.Y) / 2), xyTileControl.CurrentCarrier)
                };
                col = Convert.ToInt32(xyTileControl.DuplicatedTiles.TileCol);
                row = Convert.ToInt32(xyTileControl.DuplicatedTiles.TileRow);
                XSpacing = Convert.ToDouble(xyTileControl.DuplicatedTiles.OverlapX);
                YSpacing = Convert.ToDouble(xyTileControl.DuplicatedTiles.OverlapY);
            }

            else
            {
                XSpacing = xyTileControl.DefaultOverlapX;
                YSpacing = xyTileControl.DefaultOverlapY;
                double xSpace = XSpacing / 100;
                double ySpace = YSpacing / 100;
                double deltaX = Convert.ToDouble(Math.Abs(endPoint.X - startPoint.X).ToString("N3"));
                double deltaY = Convert.ToDouble(Math.Abs(endPoint.Y - startPoint.Y).ToString("N3"));
                if (deltaX < xyTileControl.ScanAreaWidth)
                {
                    deltaX = xyTileControl.ScanAreaWidth;
                }
                if (deltaY < xyTileControl.ScanAreaHeight)
                {
                    deltaY = xyTileControl.ScanAreaHeight;
                }

                if (xSpace >= 1)
                {
                    col = 1;
                }
                else
                {
                    col = Convert.ToInt32(Math.Ceiling((deltaX / xyTileControl.ScanAreaWidth - xSpace) / (1 - xSpace)));
                }

                if (ySpace >= 1)
                {
                    row = 1;
                }
                else
                {
                    row = Convert.ToInt32(Math.Ceiling((deltaY / xyTileControl.ScanAreaHeight - ySpace) / (1 - ySpace)));
                }

                xyPosition = new XYPosition()
                {
                    Name = "Tiles",
                    IsEnabled = true,
                    X = String.Format("{0:N4}", (topLeft.X + endPoint.X) / 2 - xyTileControl.HomePosX),
                    Y = String.Format("{0:N4}", (topLeft.Y + endPoint.Y) / 2 - xyTileControl.HomePosY),
                    Z = String.Format("{0:N4}", xyTileControl.ScanAreaZPosition - xyTileControl.HomePosZ),
                    TileCol = col.ToString(),
                    TileRow = row.ToString(),
                    OverlapX = XSpacing.ToString(),
                    OverlapY = YSpacing.ToString(),
                    Well = xyTileControl.GetWellsTagByPoint(new Point((startPoint.X + endPoint.X) / 2, (startPoint.Y + endPoint.Y) / 2), xyTileControl.CurrentCarrier)
                };
            }

            XYTable_AddItem(xyTileControl.XYtableData, xyPosition);
        }

        /// <summary>
        /// Handles the PreviewMouseDoubleClick event of the well control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="MouseButtonEventArgs" /> instance containing the event data.</param>
        void well_PreviewMouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            if (xyTileControl.sciChart.XAxis.VisibleRange != null && xyTileControl.sciChart.YAxis.VisibleRange != null)
            {
                BoxAnnotation well = sender as BoxAnnotation;
                if (well.Style == (Style)xyTileControl.FindResource("AnnotationEllipse"))
                {
                    xyTileControl.sciChart.XAxis.VisibleRange.SetMinMax(Convert.ToDouble(well.X1), Convert.ToDouble(well.X2));
                    xyTileControl.sciChart.YAxis.VisibleRange.SetMinMax(Convert.ToDouble(well.Y2), Convert.ToDouble(well.Y1));
                }
                else
                {
                    if (well.Height >= well.Width)
                    {
                        double XAxisLength = Convert.ToDouble(well.Y1) - (Convert.ToDouble(well.Y2));
                        double width = Convert.ToDouble(well.X2) - Convert.ToDouble(well.X1);
                        double minX = Convert.ToDouble(well.X1) - (XAxisLength - width) / 2;
                        double maxX = Convert.ToDouble(well.X2) + (XAxisLength - width) / 2;
                        xyTileControl.sciChart.XAxis.VisibleRange.SetMinMax(minX, maxX);
                        xyTileControl.sciChart.YAxis.VisibleRange.SetMinMax(Convert.ToDouble(well.Y2), Convert.ToDouble(well.Y1));
                    }
                    else
                    {
                        double YAxisLength = Convert.ToDouble(well.X2) - (Convert.ToDouble(well.X1));
                        double height = Convert.ToDouble(well.Y1) - Convert.ToDouble(well.Y2);
                        double maxY = Convert.ToDouble(well.Y1) + (YAxisLength - height) / 2;
                        double minY = Convert.ToDouble(well.Y2) - (YAxisLength - height) / 2;
                        xyTileControl.sciChart.XAxis.VisibleRange.SetMinMax(minY, maxY);
                        xyTileControl.sciChart.YAxis.VisibleRange.SetMinMax(Convert.ToDouble(well.X1), Convert.ToDouble(well.X2));
                    }
                }

            }
        }

        void xyTileControl_CenterTiles(XYPosition xyPosition)
        {
            int index = -1;
            for (int i = 0; i < xyTileControl.XYtableData.Count; i++)
            {
                if (xyTileControl.XYtableData[i].Equals(xyPosition))
                {
                    index = i;
                    break;
                }
            }
            if (index >= 0)
            {
                XYTable_EditItem(xyTileControl.XYtableData, index, 2, (xyTileControl.ScanAreaXPosition - xyTileControl.HomePosX));
                XYTable_EditItem(xyTileControl.XYtableData, index, 3, (xyTileControl.ScanAreaYPosition - xyTileControl.HomePosY));
            }
        }

        void xyTileControl_CopyTiles(XYPosition xyPosition, int copyType)
        {
            if (copyType == (int)XYTileControl.CopyDst.Selected)
            {
                if (SelectedWellsCollection.Count > 0)
                {
                    WellPosition wellPosition = xyTileControl.GetWellsPositionFromTag(xyPosition.Well);

                    for (int i = 0; i < SelectedWellsCollection.Count; i++)
                    {
                        double x = Convert.ToDouble(xyPosition.X) + (SelectedWellsCollection[i].Column - wellPosition.Column) * xyTileControl.CurrentCarrier.Template.CenterToCenterX;
                        double y = Convert.ToDouble(xyPosition.Y) - (SelectedWellsCollection[i].Row - wellPosition.Row) * xyTileControl.CurrentCarrier.Template.CenterToCenterY;

                        XYPosition position = new XYPosition()
                        {
                            Name = "Tiles",
                            IsEnabled = xyPosition.IsEnabled,
                            TileRow = xyPosition.TileRow,
                            TileCol = xyPosition.TileCol,
                            X = x.ToString(),
                            Y = y.ToString(),
                            Z = xyPosition.Z,
                            OverlapX = xyPosition.OverlapX,
                            OverlapY = xyPosition.OverlapY,
                            Well = xyTileControl.GetWellsTagByPoint(new Point(x + xyTileControl.HomePosX, y + xyTileControl.HomePosY), xyTileControl.CurrentCarrier)
                        };
                        if (x == Convert.ToDouble(xyPosition.X) && y == Convert.ToDouble(xyPosition.Y))
                        {
                            continue;
                        }
                        XYTable_AddItem(xyTileControl.XYtableData, position);
                    }
                }
            }
            else
            {
                WellPosition wellPosition = xyTileControl.GetWellsPositionFromTag(xyPosition.Well);

                for (int i = 0; i < xyTileControl.CurrentCarrier.Template.Col; i++)
                {
                    for (int j = 0; j < xyTileControl.CurrentCarrier.Template.Row; j++)
                    {
                        WellPosition well = new WellPosition(j, i);
                        double x = Convert.ToDouble(xyPosition.X) + (well.Column - wellPosition.Column) * xyTileControl.CurrentCarrier.Template.CenterToCenterX;
                        double y = Convert.ToDouble(xyPosition.Y) - (well.Row - wellPosition.Row) * xyTileControl.CurrentCarrier.Template.CenterToCenterY;

                        XYPosition position = new XYPosition()
                        {
                            Name = "Tiles",
                            IsEnabled = xyPosition.IsEnabled,
                            TileRow = xyPosition.TileRow,
                            TileCol = xyPosition.TileCol,
                            X = x.ToString(),
                            Y = y.ToString(),
                            Z = xyPosition.Z,
                            OverlapX = xyPosition.OverlapX,
                            OverlapY = xyPosition.OverlapY,
                            Well = xyTileControl.GetWellsTagByPoint(new Point(x + xyTileControl.HomePosX, y + xyTileControl.HomePosY), xyTileControl.CurrentCarrier)
                        };
                        if (x == Convert.ToDouble(xyPosition.X) && y == Convert.ToDouble(xyPosition.Y) || xyTileControl.XYtableData.Contains(position))
                        {
                            continue;
                        }
                        XYTable_AddItem(xyTileControl.XYtableData, position);
                    }
                }
            }
            ResetSampleInScichart();
        }

        /// <summary>
        /// Xies the tile control_ well selected status changed.
        /// </summary>
        /// <param name="menu">The menu.</param>
        /// <param name="status">if set to <c>true</c> [status].</param>
        void xyTileControl_DeselectAllWellsEvent()
        {
            DeselectWells();
        }

        void xyTileControl_Load()
        {
            _loaded = false;
            //Load the Carriers defined in the Application Setting XML.
            LoadApplcationSettings(xyTileControl.ApplicationSettings, xyTileControl.SampleCarrierCollection);
            //Load Active.XML
            LoadExperimentSettings(xyTileControl.ActiveXML, xyTileControl.SampleCarrierCollection);

            _loaded = true;

            ResetSampleInScichart();
            ResetSampleView();
        }

        /// <summary>
        /// Xies the tile control_ scan area size changed.
        /// </summary>
        void xyTileControl_ScanAreaSizeChanged()
        {
            xyTileControl.SelectModifier.ScanAreaWidth = xyTileControl.ScanAreaWidth;
            xyTileControl.SelectModifier.ScanAreaHeight = xyTileControl.ScanAreaHeight;
            if (_scanArea != null)
            {
                _scanArea.Y1 = xyTileControl.ScanAreaYPosition - xyTileControl.ScanAreaHeight / 2;
                _scanArea.Y2 = xyTileControl.ScanAreaYPosition + xyTileControl.ScanAreaHeight / 2;
                _scanArea.X1 = xyTileControl.ScanAreaXPosition - xyTileControl.ScanAreaWidth / 2;
                _scanArea.X2 = xyTileControl.ScanAreaXPosition + xyTileControl.ScanAreaWidth / 2;
                ResetSampleInScichart();
            }
        }

        /// <summary>
        /// Xies the tile control_ single position checked status changed.
        /// </summary>
        /// <param name="status">if set to <c>true</c> [status].</param>
        void xyTileControl_SinglePositionCheckedStatusChanged(bool status)
        {
            _isSinglePositionEnabled = status;
            if (status)
            {
                if (xyTileControl.TileModifier.ResizeEnabled)
                {
                    TileModifier_FinishResizingEvent();
                }

                int numOfEnabledPosition = 0;
                for (int i = 0; i < xyTileControl.XYtableData.Count; i++)
                {
                    if (xyTileControl.XYtableData[i].IsEnabled == true)
                    {
                        if (numOfEnabledPosition == 0)
                        {
                            numOfEnabledPosition++;
                            continue;
                        }
                        XYTable_EditItem(xyTileControl.XYtableData, i, 0, false);
                    }
                }
            }
        }

        /// <summary>
        /// Xies the tile control_ tiles position changed.(Move to the well center)
        /// </summary>
        /// <param name="xyPosition">The xy position.</param>
        void xyTileControl_TilesPositionChanged(XYPosition xyPosition)
        {
            WellPosition wellPosition = xyTileControl.GetWellsPositionFromTag(xyPosition.Well);
            double centerOfWellX = xyTileControl.CurrentCarrier.Template.CenterToCenterX * (wellPosition.Column);
            double centerOfWellY = 0 - xyTileControl.CurrentCarrier.Template.CenterToCenterY * (wellPosition.Row);
            string name = xyPosition.Name;
            for (int i = 0; i < xyTileControl.XYtableData.Count; i++)
            {
                XYPosition position = xyTileControl.XYtableData[i];
                if (xyPosition.Equals(position))
                {
                    XYTable_DeleteItem(xyTileControl.XYtableData, i);
                    xyPosition.X = (String.Format("{0:N4}", centerOfWellX));
                    xyPosition.Y = (String.Format("{0:N4}", centerOfWellY));
                    xyPosition.Name = name;
                    XYTable_AddItem(xyTileControl.XYtableData, xyPosition, i);
                    break;
                }
            }
        }

        /// <summary>
        /// Xies the tile control_ well selected status changed.
        /// </summary>
        /// <param name="menu">The menu.</param>
        /// <param name="status">if set to <c>true</c> [status].</param>
        void xyTileControl_WellSelectedStatusChanged(MenuItem menu, bool status)
        {
            BoxAnnotation well = null;
            if (menu != null)
            {
                well = ((ContextMenu)menu.Parent).PlacementTarget as BoxAnnotation;
                WellPosition wellPosition = xyTileControl.GetWellsPositionFromTag(well.Tag.ToString());
                for (int i = 0; i < SelectedWellsCollection.Count; i++)
                {
                    if (SelectedWellsCollection[i].Column == wellPosition.Column && SelectedWellsCollection[i].Row == wellPosition.Row)
                    {
                        if (status == false)
                        {
                            SelectedWellsCollection.RemoveAt(i);
                            well.Background = Brushes.AliceBlue;
                        }
                        return;
                    }
                }
                if (status == true)
                {
                    SelectedWellsCollection.Add(wellPosition);
                    well.Background = Brushes.Tomato;
                }
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
            }
        }

        /// <summary>
        /// Xies the tile control_ xy stage range changed.
        /// </summary>
        void xyTileControl_XYStageRangeChanged()
        {
            if (_rangeArea != null)
            {
                _rangeArea.X1 = xyTileControl.MinX;
                _rangeArea.X2 = xyTileControl.MaxX;
                _rangeArea.Y1 = xyTileControl.MaxY;
                _rangeArea.Y2 = xyTileControl.MinY;
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
            }
        }

        #endregion Methods
    }
}