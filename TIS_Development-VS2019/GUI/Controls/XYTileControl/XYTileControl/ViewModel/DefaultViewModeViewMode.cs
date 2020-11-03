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

    using ThorLogging;

    using ThorSharedTypes;

    class DefaultViewModeViewMode : XYtileViewModel
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

        #endregion Fields

        #region Constructors

        /// <summary>
        /// Initializes a new instance of the <see cref="ViewModeViewModel"/> class.
        /// </summary>
        /// <param name="xyTileControl">The xy tile control.</param>
        public DefaultViewModeViewMode(XYTileDisplay xyTileControl)
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
            if (selectedColumn == 0 && newValue.Equals(false))
            {
                xytableData[selectedRow].IsEnabled = true;
            }
            XYPosition xyposition = xytableData[selectedRow];
            XYTable_DeleteItem(xytableData, selectedRow);
            XYTable_AddItem(xytableData, xyposition, selectedRow);
        }

        /// <summary>
        /// Xies the table_ select item.
        /// </summary>
        /// <param name="xytableData">The xytable data.</param>
        /// <param name="index">The xytable selected row index.</param>
        public void XYTable_SelectItem(ObservableCollection<XYPosition> xytableData, int index)
        {
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
            xyTileControl.ExpPathChanged -= xyTileControl_ExpPathChanged;
            xyTileControl.ExpPathChanged += xyTileControl_ExpPathChanged;

            xyTileControl.SampleCarrierCollection = new ObservableCollection<Carrier>();
            xyTileControl.IsControlPanelVisible = Visibility.Collapsed;
            xyTileControl.IsTileTableVisible = Visibility.Visible;
            xyTileControl.IsDataGridReadOnly = true;
            xyTileControl.Control.Width = 460;
            xyTileControl.sciChart.Visibility = Visibility.Collapsed;
            xyTileControl.XYtable.Width = 450;

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
        private bool ConvertOldExperimentSettings(XmlDocument expXML)
        {
            var sampleRootNode = expXML.SelectSingleNode("/ThorImageExperiment/Sample");
            if (sampleRootNode != null)
            {
                if (sampleRootNode.Attributes["type"] != null && sampleRootNode.Attributes["offsetXMM"] != null && sampleRootNode.Attributes["offsetYMM"] != null)
                {
                    try
                    {
                        if (Regex.IsMatch(sampleRootNode.Attributes["type"].Value, @"\d"))//make sure it's old version
                        {
                            var streamingRootNode = expXML.SelectSingleNode("/ThorImageExperiment/Streaming");
                            if (streamingRootNode != null && streamingRootNode.Attributes["rawData"] != null && streamingRootNode.Attributes["rawData"].Value != "1")
                            {
                                //convert the sample carrier settings
                                int type = Convert.ToInt32(sampleRootNode.Attributes["type"].Value);
                                xyTileControl.SampleCarrierCollection.Add(_oldCarrierCollection[type]);
                                xyTileControl.SelectedCarrierIndex = 0;

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
                                        wellRow = (Convert.ToInt32(wellNode.Attributes["startRow"].Value)) - 1;
                                        wellColumn = (Convert.ToInt32(wellNode.Attributes["startColumn"].Value)) - 1;
                                    }
                                    wellNode.Attributes.RemoveAll();
                                    xyPosition.Well = xyTileControl.TagWells(wellRow, wellColumn);

                                    //convert subimage settings
                                    var subImageNode = expXML.SelectSingleNode("/ThorImageExperiment/Sample/Wells/SubImages");
                                    if (subImageNode != null)
                                    {
                                        var lsmNode = expXML.SelectSingleNode("/ThorImageExperiment/LSM");
                                        string pixelX = string.Empty;
                                        string pixelY = string.Empty;
                                        string pixelSizeUM = string.Empty;

                                        if (XmlManager.GetAttribute(lsmNode, expXML, "pixelX", ref pixelX) && XmlManager.GetAttribute(lsmNode, expXML, "pixelY", ref pixelY) && XmlManager.GetAttribute(lsmNode, expXML, "pixelSizeUM", ref pixelSizeUM)
                                            && Convert.ToDouble(pixelX) != 0 && Convert.ToDouble(pixelSizeUM) != 0 && Convert.ToDouble(pixelY) != 0)
                                        {
                                            xyTileControl.ScanAreaWidth = Math.Round(Convert.ToDouble(pixelX) * Convert.ToDouble(pixelSizeUM)) / 1000;
                                            xyTileControl.ScanAreaHeight = Math.Round(Convert.ToDouble(pixelY) * Convert.ToDouble(pixelSizeUM)) / 1000;
                                        }
                                        else
                                        {
                                            xyTileControl.ScanAreaWidth = 0.12;
                                            xyTileControl.ScanAreaHeight = 0.12;
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
                        ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "XYTileControl, Failed to convert old experiments in Default View Mode. Exception thrown: " + ex.Message);
                    }
                    return true;
                }
            }
            return false;
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

                // Need to initialize it here since the shared one is kept in XYTileControlMVM
                xyTileControl.XYtableData = new ObservableCollection<XYPosition>();

                if (null == expXMLPath || string.Empty == expXMLPath || !File.Exists(expXMLPath))
                {
                    return false;
                }
                if (null != xyTileControl.XYtableData)
                {
                    xyTileControl.XYtableData.Clear();
                }
                xyTileControl.SampleCarrierCollection.Clear();

                XmlDocument expXML = new XmlDocument();
                expXML.Load(expXMLPath);

                //Convert Old Experiment Settings to new one.
                if (ConvertOldExperimentSettings(expXML))
                {
                    _loaded = true;
                    return true;
                }

                var lsmNode = expXML.SelectSingleNode("/ThorImageExperiment/LSM");

                string pixelX = string.Empty;
                string pixelY = string.Empty;
                string pixelSizeUM = string.Empty;

                if (XmlManager.GetAttribute(lsmNode, expXML, "pixelX", ref pixelX) && XmlManager.GetAttribute(lsmNode, expXML, "pixelY", ref pixelY) && XmlManager.GetAttribute(lsmNode, expXML, "pixelSizeUM", ref pixelSizeUM))
                {
                    xyTileControl.ScanAreaWidth = Math.Round(Convert.ToDouble(pixelX) * Convert.ToDouble(pixelSizeUM)) / 1000;
                    xyTileControl.ScanAreaHeight = Math.Round(Convert.ToDouble(pixelY) * Convert.ToDouble(pixelSizeUM)) / 1000;
                }
                else
                {
                    xyTileControl.ScanAreaWidth = 0.12;
                    xyTileControl.ScanAreaHeight = 0.12;
                }

                var sampleRootNode = expXML.SelectSingleNode("/ThorImageExperiment/Sample");

                //preset to slide
                sampleCarrierCollection.Add(_oldCarrierCollection[5]);
                xyTileControl.SelectedCarrierIndex = 0;

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
                                        if (null != xyTileControl.XYtableData)
                                        {
                                            XYTable_AddItem(xyTileControl.XYtableData, xyPosition);
                                        }
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
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "XYTileControl, Failed to load experiment settings in Default View Mode. Exception thrown: " + ex.Message);
                return false;
            }
        }

        /// <summary>
        /// Xies the tile control_ exp path changed.
        /// </summary>
        void xyTileControl_ExpPathChanged(string exp)
        {
            //Load Active.XML
            if (LoadExperimentSettings(exp, xyTileControl.SampleCarrierCollection))
            {
                if (xyTileControl.XYtableData.Count > 0)
                {
                    xyTileControl.PositionTable.IsExpanded = true;
                }
                else
                {
                    xyTileControl.PositionTable.IsExpanded = false;
                }
            }
        }

        #endregion Methods
    }
}