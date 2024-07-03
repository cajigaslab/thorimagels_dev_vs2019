namespace CaptureSetupDll.View
{
    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Reflection;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Text.RegularExpressions;
    using System.Threading;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Xml;

    using CaptureSetupDll.Model;
    using CaptureSetupDll.View;
    using CaptureSetupDll.ViewModel;

    using ExperimentSettingsBrowser;

    using FolderDialogControl;

    using HelpProvider;

    using Microsoft.Practices.Composite.Events;
    using Microsoft.Practices.Composite.Wpf.Events;
    using Microsoft.Win32;

    using ThorLogging;

    using ThorSharedTypes;

    /// <summary>
    /// Interaction logic for MasterView.xaml
    /// </summary>
    public partial class MasterView : UserControl
    {
        #region Fields

        const int DISPLAY_PANELS = 23;
        const int NUM_CHANNELS = 4;

        private bool zExpander_MouseGotFocus = false;

        //object _dataContextliveImageVM = null;
        private string _imageProcessSettingsFile;
        CaptureSetupViewModel _liveVM = null;
        bool _loaded = false;
        string[,] _panelInfo = new string[DISPLAY_PANELS, 3] {
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/ScannerView","Scanner Control","scanBorder"},
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/AreaView","Area Control","areaBorder"},
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/PowerView","Power Control","powBorder"},
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/ZView","Z Control","zBorder"},
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/MultiLaserControlView","MultiLaserControlView","MultiLaserControlBorder"},
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/MultiphotonView", "Multiphoton Control","multiphotonBorder"},
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/LightPathView","Light Path Control","LightPathBorder"},
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/BleachView","Stimulation Control","BleachBorder"},
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/PinholeView","Pinhole Wheel Control","pinholeBorder"},
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/XYView","XY Control","tilesControlBorder"},
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/CaptureOptionsView","Capture Options","captureOptionsBorder"},
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/KuriosFilterView","Kurios Filter","kuriosControlBorder"},
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/CameraView","Camera Control","cameraControlBorder"},
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/DigitalSwitchesView","Digital Switches","digitalSwitchesBorder"},
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/QuickTemplate","Quick Template","quickTemplateBorder"},
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/LampView","Lamp","LampBorder"},
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/ThreePhotonView","ThreePhotonControlView","ThreePhotonControlBorder"},
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/DFLIMView","DFLIMViewControl","DFLIMControlBorder"},
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/LightEngineView","LightEngineControlView","LightEngineControlBorder"},
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/EpiturretControlView","EpiturretControlView","EpiturretControlBorder"},
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/AutoFocusControlView","AutoFocusControlView","AutoFocusControlBorder"},
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/MiniCircuitsSwitchControlView","MiniCircuitsSwitchControlView","MiniCircuitsSwitchControlBorder"},
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/SequentialControlView","Sequential Control","sequentialControlBorder"}
            };
        Dictionary<int, string> _powerTabVisibility = new Dictionary<int, string>
         {
            {0, "Pockels0Visibility"},
            {1, "Pockels1Visibility"},
            {2, "Pockels2Visibility"},
            {3, "Pockels3Visibility"},
            {4, "PowerRegVisibility"},
            {5, "PowerReg2Visibility"},
         };

        #endregion Fields

        #region Constructors

        public MasterView()
        {
            InitializeComponent();

            if (System.ComponentModel.DesignerProperties.GetIsInDesignMode(this) == true)  //design mode
                return;

            this.Loaded += new RoutedEventHandler(MasterView_Loaded);
            //this.MouseWheel += new MouseWheelEventHandler(zExpander_MouseWheel);
            //this.MouseDown += new MouseButtonEventHandler(zExpander_GotFocus);

            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Initialized");
            this.KeyDown += MasterView_KeyDown;
            this.Unloaded += new RoutedEventHandler(MasterView_Unloaded);
        }

        #endregion Constructors

        #region Methods

        [DllImport(".\\Modules_Native\\SelectHardware.dll", EntryPoint = "UpdateAndPersistCurrentDevices")]
        public static extern int UpdateAndPersistCurrentDevices();

        /// <summary>
        /// Find the child element of a panel by name. Unnamed children cannot be found this way.
        /// </summary>
        /// <param name="parent"> The panel to look in </param>
        /// <param name="childName"> The name of the child element </param>
        /// <returns> The framework element, or null if no matching element was found </returns>
        public FrameworkElement FindChildByName(Panel parent, String childName)
        {
            foreach (var child in parent.Children.OfType<FrameworkElement>())
            {
                if (childName == child.Name)
                {
                    return child;
                }
            }
            return null;
        }

        public void MasterView_Unloaded(object sender, RoutedEventArgs e)
        {
            if (null == _liveVM)
            {
                return;
            }

            //*** DO NOT PersistApplicationSettings in MasterView Unload event, ***//
            //*** because it will overrule changes made from Settings.          ***//

            //There is the possibility that a setting was changed and the camera did not verify the
            //setting via a capture. In this case use preflight/postflight to push down the gui values
            if (false == _liveVM.IsLive)
            {
                _liveVM.PrePostSettingsToCamera();
            }
            else
            {
                //stopping the live capture
                _liveVM.LiveCapture(false);
            }
            //stop preview:
            _liveVM.CloseProgressWindow();

            _liveVM.ReleaseHandlers();

            _liveVM.ReleaseBleachWaveform();

            _loaded = false;
        }

        public void OnLoadExperiment(object sender, RoutedEventArgs e)
        {
            //stopping the live capture
            _liveVM.LiveCapture(false);

            string expSettingsFile = null;

            ExperimentSettingsBrowserWindow settingsDlg = new ExperimentSettingsBrowserWindow();
            settingsDlg.Title = "Settings Browser";
            settingsDlg.BrowserType = ExperimentSettingsBrowserWindow.BrowserTypeEnum.SETTINGS;
            settingsDlg.Owner = Application.Current.MainWindow;
            settingsDlg.BrowseExperimentPath = _liveVM.GetExperimentSavingPath();

            if (true == settingsDlg.ShowDialog())
            {
                expSettingsFile = settingsDlg.ExperimentSettingsPath;
                if (File.Exists(expSettingsFile))
                {
                    UpdateExperimentInfo(expSettingsFile);
                }
            }
        }

        /// <summary>
        /// Removes the visible children from a panel
        /// </summary>
        /// <param name="parent"> The parent panel to remove children from </param>
        public void RemoveVisiblePanels(Panel parent)
        {
            List<FrameworkElement> collapsedPanels = new List<FrameworkElement>();
            foreach (FrameworkElement panel in parent.Children)
            {
                if (panel.Visibility == System.Windows.Visibility.Collapsed)
                {
                    collapsedPanels.Add(panel);
                }
            }

            parent.Children.Clear();
            collapsedPanels.ForEach(panel => parent.Children.Add(panel as UIElement));
        }

        public void SetPath(string pathIn)
        {
            path.Text = pathIn;
        }

        public void UpdateExperimentInfo(string expSettingsFile)
        {
            _liveVM.IgnoreLineProfileGeneration = true;

            var originalSettingsTemplateName = Path.GetFileNameWithoutExtension(expSettingsFile);
            string str = string.Empty;
            if (!File.Exists(expSettingsFile))
            {
                MessageBox.Show("Could not find file: " + expSettingsFile);
                return;
            }
            _liveVM.SettingsTemplateName = originalSettingsTemplateName;

            //Set the Holding modality to the modality in the experiment template that you are about
            // to load.

            //Copies Photobleaching tag from original active XML to prevent overwriting when loading a template
            string tempFolder = ResourceManagerCS.GetCaptureTemplatePathString();
            string pathActiveXML = tempFolder + "Active.xml";
            XmlDocument originalActive = new XmlDocument();
            XmlNode oldNode;

            originalActive.Load(pathActiveXML);
            oldNode = originalActive.SelectSingleNode("/ThorImageExperiment/Photobleaching");

            XmlDocument expSettings = new XmlDocument();
            expSettings.Load(expSettingsFile);
            XmlNodeList nodeList = expSettings.SelectNodes("/ThorImageExperiment/Modality");
            if (null != nodeList && ThorSharedTypes.XmlManager.GetAttribute(nodeList[0], expSettings, "name", ref str) && ResourceManagerCS.Instance.ActiveModality != str)
            {
                ResourceManagerCS.Instance.ActiveModality = str;
                //Call Setup Command from CaptureSetup.cpp in case the camera type was changed after loading a template
                ((ICommand)MVMManager.Instance["CameraControlViewModel", "ReconnectCameraCommand"]).Execute(null);
            }

            //copy the experiment file as the active experiment
            //and reload the panel
            this.MasterView_Unloaded(null, null);

            _liveVM.ReplaceActiveSettings(expSettingsFile);

            this.MasterView_Loaded(null, null);

            if (TilesControlView.xyTileControl != null)
            {
                TilesControlView.xyTileControl.XYTileDisplay_Load();
            }

            //persist global params except for the GG calibration Photobleaching tag in all modalities after loading:
            for (int i = 2; i < (int)GlobalExpAttribute.LAST; i++)
            {
                _liveVM.PersistGlobalExperimentXML((GlobalExpAttribute)i);
            }

            //Resets the photobleaching tag values in Active xml to old values and reloads Active
            XmlDocument newActive = new XmlDocument();
            XmlNode newNode;
            XmlNode newChild;
            newActive.Load(pathActiveXML);

            //Overwriting of Photobleaching tag
            newNode = newActive.SelectSingleNode("/ThorImageExperiment");
            newChild = newNode.SelectSingleNode("/ThorImageExperiment/Photobleaching");
            newNode.ReplaceChild(newActive.ImportNode(oldNode, true), newChild);

            newActive.Save(pathActiveXML);
            //Update Capture Setup View Model with modified active
            _liveVM.ExperimentDoc = newActive;
            MVMManager.Instance.LoadSettings(SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS);
        }

        public void UpdateHardwareListView()
        {
            XmlDataProvider dataProvider = (XmlDataProvider)this.FindResource("HardwareSettings");

            dataProvider.Document = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];

            using (dataProvider.DeferRefresh())
            {
                MVMManager.Instance.SaveSettings(SettingsFileType.HARDWARE_SETTINGS);
            }
        }

        protected override void OnPreviewMouseDown(MouseButtonEventArgs e)
        {
            base.OnPreviewMouseDown(e);
            zExpander_MouseGotFocus = false;
        }

        //:TODO: Check if we need this later
        void AFScanStart_Update(string scanstart, string focusoffset)
        {
            XmlNodeList ndList = _liveVM.HardwareDoc.DocumentElement["Objectives"].GetElementsByTagName("Objective");

            //turret position is 1 based index
            int objectiveIndex = (int)MVMManager.Instance["ObjectiveControlViewModel", "TurretPosition", (object)0];

            if ((objectiveIndex > 0) && (objectiveIndex < ndList.Count))
            {
                ndList.Item(objectiveIndex).Attributes["afScanStartMM"].Value = scanstart;
                ndList.Item(objectiveIndex).Attributes["afFocusOffsetMM"].Value = focusoffset;
            }

            MVMManager.Instance.SaveSettings(SettingsFileType.HARDWARE_SETTINGS);
        }

        /// <summary>
        /// Rearranges all visible display panels into the left control panel, in the order
        /// that was specified in application settings
        /// </summary>
        private void ArrangeDisplayPanelsSingleColumn()
        {
            List<FrameworkElement> visibleDisplayPanels = GetVisibleDisplayPanelsInOrder();

            ClearVisibleDisplayPanelsFromDisplay();

            //Add back to panel
            visibleDisplayPanels.ForEach(panel => leftStackPanel.Children.Add(panel as UIElement));
        }

        /// <summary>
        /// Rearranges all visible display panels into both the left and right control panels, 
        /// in the order that was specified in application settings. Panels are arranged so 
        /// the 1st panel is top left and the 2nd panel is top right, continuing to alternate
        /// for the remaining visible panels.
        /// </summary>
        private void ArrangeDisplayPanelsTwoColumns()
        {
            List<FrameworkElement> visibleDisplayPanels = GetVisibleDisplayPanelsInOrder();
            List<FrameworkElement> leftPanels = visibleDisplayPanels.Where((panel, index) => index % 2 == 0).ToList(); //Get even panels
            List<FrameworkElement> rightPanels = visibleDisplayPanels.Where((panel, index) => (index + 1) % 2 == 0).ToList(); //Get odd panels

            //Clear Columns
            ClearVisibleDisplayPanelsFromDisplay();

            //Add to panels
            leftPanels.ForEach(panel => leftStackPanel.Children.Add(panel as UIElement));
            rightPanels.ForEach(panel => rightStackPanel.Children.Add(panel as UIElement));
        }

        void AutoExposure_Update(double exposure)
        {
        }

        void AutoFocus_Update(bool val)
        {
            try
            {
                XmlNodeList ndList = _liveVM.HardwareDoc.DocumentElement["Objectives"].GetElementsByTagName("Objective");

                double currentPosition = (double)MVMManager.Instance["ZControlViewModel", "ZPosition", (object)0.0];

                if ((int)MVMManager.Instance["ObjectiveControlViewModel", "TurretPosition", (object)0] < ndList.Count)
                {
                    double tmp = 0;
                    if (Double.TryParse(ndList.Item((int)MVMManager.Instance["ObjectiveControlViewModel", "TurretPosition", (object)0]).Attributes["afScanStartMM"].Value, NumberStyles.Any, CultureInfo.InvariantCulture, out tmp))
                    {
                        MVMManager.Instance["ZControlViewModel", "ZPosition"] = tmp;
                    }
                }

                //if (false == _liveVM.AutoFocus())
                //{
                //    ThorLog.Instance.TraceEvent(TraceEventType.Information, 1, this.GetType().Name + " Autofocus failed returning to previous Z position");

                //    MVMManager.Instance["ZControlViewModel", "ZPosition"] = currentPosition;
                //}

                MVMManager.Instance.SaveSettings(SettingsFileType.HARDWARE_SETTINGS);
            }
            catch (Exception ex)
            {
                ex.ToString();
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + ex.Message);

            }
        }

        private void btnSaveSettings_Click(object sender, RoutedEventArgs e)
        {
            if (_liveVM.SettingsTemplateName == null || _liveVM.SettingsTemplateName == String.Empty)
            {
                MessageBox.Show("Settings Template Name is empty! Please enter a valid Name.");
                return;
            }
            if (_liveVM.SettingsTemplateName.Contains("\\") || _liveVM.SettingsTemplateName.Contains("/") || _liveVM.SettingsTemplateName.StartsWith(" "))
            {
                MessageBox.Show("Settings Template Name has invalid characters or It starts with Whitespace, Please enter a valid Name.");
                return;
            }
            else
            {
                XmlDocument expDoc = _liveVM.ExperimentDoc;

                if (1 == ((int)MVMManager.Instance["CaptureOptionsControlViewModel", "CaptureMode", (object)0.0]) || 3 == ((int)MVMManager.Instance["CaptureOptionsControlViewModel", "CaptureMode", (object)0.0]))
                {
                    //=== Streaming and bleaching Captures are not compatible with Capture Sequence Mode ===
                    if (1 == (int)MVMManager.Instance["SequentialControlViewModel", "EnableSequentialCapture", (object)0])//&& 1 < _liveVM.CollectionCaptureSequence.Count)
                    {
                        MessageBox.Show("\"Sequential Capture Mode\" is incompatible with Streaming and Bleaching capture modes. Please clear your channel sequence options or change the experiment type and try again.");
                        return;
                    }

                    //=== Streaming and bleaching Captures are not compatible with Tiles Capture ===
                    if (TilesControlView.xyTileControl.TotalTiles > 1)
                    {
                        MessageBox.Show("\"Tiles Mode\" is incompatible with Streaming and Bleaching capture modes when there is more than one tile defined. Please clear or deselect the tiles and try again.");
                        return;
                    }
                }

                ResourceManagerCS.SafeCreateDirectory(_liveVM.SettingsTemplatesPath);

                //There is the possibility that a setting was changed and the camera did not verify the
                //setting via a capture. In this case use preflight/postflight to push down the gui values
                if (false == _liveVM.IsLive)
                {
                    _liveVM.PrePostSettingsToCamera();
                }

                string expSettingsPath = _liveVM.SettingsTemplatesPath + "\\" + _liveVM.SettingsTemplateName + ".xml";

                if (File.Exists(_liveVM.SettingsTemplatesPath + "\\" + _liveVM.SettingsTemplateName + ".xml"))
                {
                    MessageBoxResult m = MessageBox.Show("Do you want to replace the " + _liveVM.SettingsTemplateName + " Settings Template with the current settings?", "Update Template?", MessageBoxButton.YesNo);
                    if (m == MessageBoxResult.Yes)
                    {
                        MVMManager.Instance.UpdateMVMXMLSettings(ref MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS]);
                        MVMManager.Instance.SaveSettings(SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS);
                        if (false == _liveVM.CreateTemplateSettings(expSettingsPath))
                        {
                            return;
                        }
                    }
                    else
                    {
                        return;
                    }
                }
                else
                {
                    MVMManager.Instance.UpdateMVMXMLSettings(ref MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS]);
                    MVMManager.Instance.SaveSettings(SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS);
                    if (false == _liveVM.CreateTemplateSettings(expSettingsPath))
                    {
                        return;
                    }
                    MessageBox.Show("Template " + _liveVM.SettingsTemplateName + " successfully created");
                }
                string[] reloadMVMs = new string[] { "LightEngineControlViewModel" };
                MVMManager.Instance.LoadMVMSettings(reloadMVMs);
            }
        }

        private void Button_Click_Add_Wavelength(object sender, RoutedEventArgs e)
        {
        }

        private void Button_Click_Remove_Wavelength(object sender, RoutedEventArgs e)
        {
        }

        /// <summary>
        /// Clears the visible display panels from the main display, leaving only the visibility collapsed ones, which because
        /// they take up no space do not matter in ordering or layout.
        /// </summary>
        private void ClearVisibleDisplayPanelsFromDisplay()
        {
            RemoveVisiblePanels(leftStackPanel);
            RemoveVisiblePanels(rightStackPanel);
        }

        private void Expander_Expanded(object sender, RoutedEventArgs e)
        {
            bool bExpand = false;

            if (null == _liveVM)
            {
                return;
            }

            XmlDocument doc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

            XmlNodeList ndList = doc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/MultiPanelView");
            if (ndList.Count > 0)
            {
                if (ndList[0].Attributes["Visibility"].Value.Equals("Visible"))
                    bExpand = true;
            }

            if (bExpand)
                return;

            if (sender != scanExpander)
            {
                if (scanExpander != null)
                {
                    scanExpander.IsExpanded = false;
                }
            }
            if (sender != LightPathExpander)
            {
                if (LightPathExpander != null)
                {
                    LightPathExpander.IsExpanded = false;
                }
            }
            if (sender != BleachExpander)
            {
                if (BleachExpander != null)
                {
                    BleachExpander.IsExpanded = false;
                }
            }
            if (sender != zExpander)
            {
                if (zExpander != null)
                {
                    zExpander.IsExpanded = false;
                }
            }
            if (sender != powExpander)
            {
                if (powExpander != null)
                {
                    powExpander.IsExpanded = false;
                }
            }

            //if (sender != mclsExpander)
            //{
            //    if (mclsExpander != null)
            //    {
            //        mclsExpander.IsExpanded = false;
            //    }
            //}

            if (sender != multiphotonExpander)
            {
                if (multiphotonExpander != null)
                {
                    multiphotonExpander.IsExpanded = false;
                }
            }

            if (sender != pinholeExpander)
            {
                if (pinholeExpander != null)
                {
                    pinholeExpander.IsExpanded = false;
                }
            }

            if (sender != tilesControlExpander)
            {
                if (tilesControlExpander != null)
                {
                    tilesControlExpander.IsExpanded = false;
                }
            }

            if (sender != areaExpander)
            {
                if (areaExpander != null)
                {
                    areaExpander.IsExpanded = false;
                }
            }

            if (sender != captureOptionExpander)
            {
                if (captureOptionExpander != null)
                {
                    captureOptionExpander.IsExpanded = false;
                }
            }

            if (sender != kuriosFilterExpander)
            {
                if (kuriosFilterExpander != null)
                {
                    kuriosFilterExpander.IsExpanded = false;
                }
            }

            if (sender != cameraControlExpander)
            {
                if (cameraControlExpander != null)
                {
                    cameraControlExpander.IsExpanded = false;
                }
            }

            if (sender != digitalSwitchesExpander)
            {
                if (digitalSwitchesExpander != null)
                {
                    digitalSwitchesExpander.IsExpanded = false;
                }
            }

            if (sender != quickTemplateExpander)
            {
                if (quickTemplateExpander != null)
                {
                    quickTemplateExpander.IsExpanded = false;
                }
            }

            if (sender != LampControlExpander)
            {
                if (LampControlExpander != null)
                {
                    LampControlExpander.IsExpanded = false;
                }
            }

            if (sender != ThreePhotonControlExpander)
            {
                if (ThreePhotonControlExpander != null)
                {
                    ThreePhotonControlExpander.IsExpanded = false;
                }
            }

            if (sender != DFLIMControlExpander)
            {
                if (DFLIMControlExpander != null)
                {
                    DFLIMControlExpander.IsExpanded = false;
                }
            }

            if (sender != LightEngineControlExpander)
            {
                if (LightEngineControlExpander != null)
                {
                    LightEngineControlExpander.IsExpanded = false;
                }
            }

            if (sender != EpiturretControlExpander)
            {
                if (EpiturretControlExpander != null)
                {
                    EpiturretControlExpander.IsExpanded = false;
                }
            }

            if (sender != multiLaserExpander)
            {
                if (multiLaserExpander != null)
                {
                    multiLaserExpander.IsExpanded = false;
                }
            }
            if (sender != AutoFocusControlExpander)
            {
                if (AutoFocusControlExpander != null)
                {
                    AutoFocusControlExpander.IsExpanded = false;
                }
            }

            if (sender != MiniCircuitsSwitchControlExpander)
            {
                if (MiniCircuitsSwitchControlExpander != null)
                {
                    MiniCircuitsSwitchControlExpander.IsExpanded = false;
                }
            }
            if (sender != sequentialControlExpander)
                if (sequentialControlExpander != null)
                    sequentialControlExpander.IsExpanded = false;
        }

        private void Exposure_Update(double exposureTime)
        {
        }

        private void GetKeyboardProperty(XmlNodeList nd, ref string modifier, ref string key)
        {
            string strCtrl = string.Empty;
            string strAlt = string.Empty;
            string strShift = string.Empty;
            string str = string.Empty;
            modifier = string.Empty;
            key = string.Empty;

            if (nd.Count > 0)
            {
                XmlManager.GetAttribute(nd[0], _liveVM.ApplicationDoc, "ctrl", ref strCtrl);
                XmlManager.GetAttribute(nd[0], _liveVM.ApplicationDoc, "alt", ref strAlt);
                XmlManager.GetAttribute(nd[0], _liveVM.ApplicationDoc, "shift", ref strShift);
                XmlManager.GetAttribute(nd[0], _liveVM.ApplicationDoc, "key", ref str);

                int tmp = 0;
                if (Int32.TryParse(strCtrl, out tmp))
                {
                    if (1 == tmp)
                    {
                        modifier += "Ctrl";
                    }
                }

                if (Int32.TryParse(strAlt, out tmp))
                {
                    if (1 == tmp)
                    {
                        if (modifier.Length == 0)
                        {
                            modifier += "Alt";
                        }
                        else
                        {
                            modifier += "+Alt";
                        }
                    }
                }

                if (Int32.TryParse(strShift, out tmp))
                {
                    if (1 == tmp)
                    {
                        if (modifier.Length == 0)
                        {
                            modifier += "Shift";
                        }
                        else
                        {
                            modifier += "+Shift";
                        }
                    }
                }

                key = str;
            }
        }

        /// <summary>
        /// Put all available display panels that will be visible into a list ordered 
        /// according to application settings.
        /// </summary>
        /// <returns> A list of framework elements in display order from top to bottom </returns>
        private List<FrameworkElement> GetVisibleDisplayPanelsInOrder()
        {
            _liveVM.ApplicationDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
            FrameworkElement[] panels = new FrameworkElement[DISPLAY_PANELS];
            for (int i = 0; i < DISPLAY_PANELS; i++)
            {
                string str = string.Empty;
                XmlNodeList ndList = _liveVM.ApplicationDoc.SelectNodes(_panelInfo[i, 0]);

                if (ndList.Count > 0)
                {
                    if (XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "location", ref str))
                    {
                        int loc = 0;
                        loc = int.TryParse(str, out loc) ? loc : 0;
                        if (loc >= 0 && loc < DISPLAY_PANELS)
                        {
                            FrameworkElement panel = null;

                            //Try to find panel in left or right column
                            panel = FindChildByName(leftStackPanel, _panelInfo[i, 2]); //Right Column
                            panel = (panel == null ? FindChildByName(rightStackPanel, _panelInfo[i, 2]) : panel); //Check Left Column if not found

                            if (panel != null && panel.Visibility != System.Windows.Visibility.Collapsed)
                            {
                                panels[loc] = panel;
                            }
                        }
                    }
                }
            }

            List<FrameworkElement> validPanels = panels.ToList<FrameworkElement>();
            validPanels.RemoveAll(item => item == null);
            return validPanels;
        }

        void LoadLSMHardwareSettings()
        {
            string str = string.Empty;
            XmlDocument hardwareDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];

            XmlNodeList ndList = hardwareDoc.SelectNodes("/HardwareSettings/ImageDetectors/LSM");

            //if a LSM and Camera are not in the list of available image detectors
            //disable the panel
            if (0 == ndList.Count)
            {
                ndList = hardwareDoc.SelectNodes("/HardwareSettings/ImageDetectors/Camera");

                masterGrid.IsEnabled = (0 == ndList.Count) ? false : true;
            }
            else
            {
                masterGrid.IsEnabled = true;
            }
        }

        private void magComboBox_PreviewMouseWheel(object sender, MouseWheelEventArgs e)
        {
            e.Handled = true;
        }

        void MasterView_KeyDown(object sender, KeyEventArgs e)
        {
            //filter for the enter or return key
            if ((e.Key == Key.Enter) || (e.Key == Key.Return))
            {
                e.Handled = true;
                TraversalRequest trNext = new TraversalRequest(FocusNavigationDirection.Next);

                UIElement keyboardFocus = (UIElement)Keyboard.FocusedElement;

                //move the focus to the next element
                if (keyboardFocus != null)
                {
                    if (keyboardFocus.GetType() == typeof(TextBox))
                    {
                        keyboardFocus.MoveFocus(trNext);
                    }
                }
            }
        }

        void MasterView_Loaded(object sender, RoutedEventArgs e)
        {
            if (_loaded)
            {
                return;
            }
            try
            {
                //setting initial position and size of the positionRectangle of the canvas.
                _liveVM = (CaptureSetupViewModel)this.DataContext;

                //Once Capture Setup loads, the switchbox is allowed to start changing
                ResourceManagerCS.Instance.AllowSwitchBoxToWork = true;

                string imageProcessSettings = Application.Current.Resources["ImageProcessSettingsFile"].ToString();
                _imageProcessSettingsFile = imageProcessSettings;
                if (!File.Exists(_imageProcessSettingsFile))
                {
                    ThorLog.Instance.TraceEvent(TraceEventType.Information, 1, this.GetType().Name + " Unable to load image processing setttings");
                }

                _liveVM.HardwareDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];

                _liveVM.SettingsTemplatesPath = ResourceManagerCS.GetCaptureTemplatePathString() + "Template Favorites";

                _liveVM.ApplicationDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

                XmlDocument imageProcessSettingsDoc = new XmlDocument();
                imageProcessSettingsDoc.Load(_imageProcessSettingsFile);
                _liveVM.ImageProcessDoc = imageProcessSettingsDoc;
                //Load and adjust tems from application settings
                SetDisplayOptions();

                //Load key shortcut options from application settings
                SetKeyboardOptions();

                //The name of the scanner is a one way property.
                //force the name of scanner to update
                BindingExpression bexp = scanExpander.GetBindingExpression(Expander.HeaderProperty);
                bexp.UpdateTarget();

                _liveVM.ExperimentDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS];

                //load hardware settings from file
                LoadLSMHardwareSettings();

                //load the magnification combo box using the hardware settings document
                XmlDataProvider dataProvider = (XmlDataProvider)this.FindResource("HardwareSettings");

                dataProvider.Document = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];

                //load all MVMs:
                MVMManager.Instance.LoadMVMSettings();

                _liveVM.ConnectHandlers();

                //hide SLM patterns if SLM panel is not in use
                if (!_liveVM.SLMPanelInUse)
                    _liveVM.SLMPatternsVisible = false;
                ResourceManagerCS.BackupDirectory(ResourceManagerCS.GetMyDocumentsThorImageFolderString());
                //reloading of the ROIs will result in a stats recalculation
                //if the modality changes and the stats are run on the size of the previous
                //image detector there is a possiblity of a buffer overrun. To ensure the buffers
                //are the correct size call the SetupCaptureBuffers to reallocate according to the active image detector
                CaptureSetupViewModel.SetupCaptureBuffers();
                _liveVM.ModalitySpinnerWindowShowing = false;

            }
            catch (Exception ex)
            {
                _liveVM.ModalitySpinnerWindowShowing = false;
                ThorLogging.ThorLog.Instance.TraceEvent(System.Diagnostics.TraceEventType.Error, 1, "CaptureSetup MasterView load error: " + ex.Message);
                MessageBox.Show("There was an error loading Capture Setup. Some of your properties may not have been updated.");
            }

            _loaded = true;
        }

        private void SetDisplayOptions()
        {
            try
            {
                string str = string.Empty;
                double dVal = 0.0;
                //Load and adjust visible items from application settings
                XmlNodeList ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/ScannerView");

                if (ndList.Count > 0)
                {
                    scanBorder.Visibility = ndList[0].Attributes["Visibility"].Value.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                }

                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/QuickTemplate");
                if (ndList.Count > 0)
                {
                    quickTemplateBorder.Visibility = ndList[0].Attributes["Visibility"].Value.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                }

                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/AreaView");
                if (ndList.Count > 0)
                {
                    areaBorder.Visibility = ndList[0].Attributes["Visibility"].Value.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                }

                ndList = _liveVM.HardwareDoc.SelectNodes("/HardwareSettings/ImageDetectors/LSM");
                if (ndList.Count > 0)
                {
                    bool lsmActive = false;
                    for (int i = 0; i < ndList.Count; i++)
                    {
                        if (XmlManager.GetAttribute(ndList[i], _liveVM.HardwareDoc, "active", ref str))
                        {
                            if ("1" == str)
                            {
                                lsmActive = true;
                            }
                        }
                    }
                    if (true == lsmActive && Visibility.Visible == areaBorder.Visibility)
                    {
                        areaBorder.Visibility = Visibility.Visible;
                    }
                    else
                    {
                        areaBorder.Visibility = Visibility.Collapsed;
                    }

                    if (true == lsmActive && Visibility.Visible == scanBorder.Visibility)
                    {
                        scanBorder.Visibility = Visibility.Visible;
                    }
                    else
                    {
                        scanBorder.Visibility = Visibility.Collapsed;
                    }
                }
                else
                {
                    areaBorder.Visibility = Visibility.Collapsed;
                    scanBorder.Visibility = Visibility.Collapsed;
                }

                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/CameraView");
                if (ndList.Count > 0)
                {
                    cameraControlBorder.Visibility = ndList[0].Attributes["Visibility"].Value.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                }

                ndList = _liveVM.HardwareDoc.SelectNodes("/HardwareSettings/ImageDetectors/Camera");
                bool ccdActive = false;
                if (ndList.Count > 0)
                {
                    for (int i = 0; i < ndList.Count; i++)
                    {
                        if (XmlManager.GetAttribute(ndList[i], _liveVM.HardwareDoc, "active", ref str))
                        {
                            if ("1" == str)
                            {
                                ccdActive = true;
                            }
                        }
                    }

                    if (true == ccdActive && Visibility.Visible == cameraControlBorder.Visibility)
                    {
                        cameraControlExpander.Visibility = cameraControlBorder.Visibility = Visibility.Visible;
                        MVMManager.Instance["ImageViewCaptureSetupVM", "IsChannelVisible1"] = Visibility.Collapsed;
                        MVMManager.Instance["ImageViewCaptureSetupVM", "IsChannelVisible2"] = Visibility.Collapsed;
                        MVMManager.Instance["ImageViewCaptureSetupVM", "IsChannelVisible3"] = Visibility.Collapsed;

                        if (ResourceManagerCS.Instance.TabletModeEnabled)
                        {
                            //When in camera mode with the tablet reduce the size of the panels to 87%
                            //so the camera control fits in the screen
                            _liveVM.PanelsScale = 0.87;
                            MVMManager.Instance["ImageViewCaptureSetupVM", "IsTileDisplayButtonVisible"] = Visibility.Collapsed;
                        }
                    }
                    else
                    {
                        cameraControlExpander.Visibility = cameraControlBorder.Visibility = Visibility.Collapsed;

                        MVMManager.Instance["ImageViewCaptureSetupVM", "IsChannelVisible1"] = Visibility.Visible;
                        MVMManager.Instance["ImageViewCaptureSetupVM", "IsChannelVisible2"] = Visibility.Visible;
                        MVMManager.Instance["ImageViewCaptureSetupVM", "IsChannelVisible3"] = Visibility.Visible;

                        if (ResourceManagerCS.Instance.TabletModeEnabled)
                        {
                            //When switching to joystick modality change the panels scale to 100%, each
                            // panel has it's own scaling
                            _liveVM.PanelsScale = 1;

                            MVMManager.Instance["ImageViewCaptureSetupVM", "IsTileDisplayButtonVisible"] = Visibility.Visible;
                        }
                    }
                }
                else
                {
                    cameraControlExpander.Visibility = cameraControlBorder.Visibility = Visibility.Collapsed;
                }

                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/LightPathView");

                if (ndList.Count > 0)
                {
                    LightPathBorder.Visibility = ndList[0].Attributes["Visibility"].Value.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                }

                int selectControlView = 0;  //[0]: BleachControlView, [1]: SLMControlView
                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/BleachView");
                if (ndList.Count > 0)
                {
                    if ((int)ICamera.LSMType.LSMTYPE_LAST != ResourceManagerCS.GetBleacherType())
                    {
                        BleachBorder.Visibility = ndList[0].Attributes["Visibility"].Value.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                        MVMManager.Instance["CaptureOptionsControlViewModel", "BleachControlActive"] = ndList[0].Attributes["Visibility"].Value.Equals("Visible");
                    }
                    else
                    {
                        BleachBorder.Visibility = System.Windows.Visibility.Collapsed;
                        MVMManager.Instance["CaptureOptionsControlViewModel", "BleachControlActive"] = false;
                    }
                    if (!XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "SelectControlView", ref str))
                    {
                        XmlManager.SetAttribute(ndList[0], _liveVM.ApplicationDoc, "SelectControlView", "0");
                        MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS);
                    }
                    else
                        Int32.TryParse(str, out selectControlView);
                }

                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/RunSample/FastZView");

                if (ndList.Count > 0)
                {
                    MVMManager.Instance["CaptureOptionsControlViewModel", "FastZActive"] = ndList[0].Attributes["Visibility"].Value.Equals("Visible");
                }

                //set visibility of bleach controls based on SLM device or settings:
                ndList = _liveVM.HardwareDoc.SelectNodes("/HardwareSettings/Devices/SLM");
                BleachPanel.Children.Clear();
                _liveVM.BleachExpandHeader = string.Empty;
                _liveVM.SLMPanelInUse = false;
                for (int i = 0; i < ndList.Count; i++)
                {
                    if (XmlManager.GetAttribute(ndList[i], _liveVM.HardwareDoc, "dllName", ref str) && str.Contains("ThorSLM"))
                    {
                        _liveVM.SLMPanelInUse = XmlManager.GetAttribute(ndList[i], _liveVM.HardwareDoc, "active", ref str) && 0 == str.CompareTo("1");
                    }
                }
                if (1 == selectControlView)
                { _liveVM.SLMPanelInUse = true; }
                if (_liveVM.SLMPanelInUse)
                {
                    BleachPanel.Children.Add(new SLMControlView(_liveVM));
                    _liveVM.BleachExpandHeader = "SLM ";
                }
                else
                {
                    BleachPanel.Children.Add(new BleachControlView(_liveVM));
                    _liveVM.BleachExpandHeader = "Stimulation ";
                }

                //set visibility of OTM control
                MVMManager.Instance["OTMControlViewModel", "ViewModelIsLoad"] = false;
                ndList = _liveVM.HardwareDoc.SelectNodes("/HardwareSettings/ImageDetectors/LSM");
                for (int i = 0; i < ndList.Count; i++)
                {
                    if ((XmlManager.GetAttribute(ndList[i], _liveVM.HardwareDoc, "cameraName", ref str)) && ((0 == str.CompareTo("GGNI")) || (0 == str.CompareTo("ResonanceGalvoSim")))
                        && ((XmlManager.GetAttribute(ndList[i], _liveVM.HardwareDoc, "activation", ref str)) && (0 == str.CompareTo("1"))))
                    {
                        ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/BleachView/OTMView");
                        if (0 < ndList.Count)
                        {
                            if ((XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "Visibility", ref str)) && (0 == str.CompareTo("Visible")))
                            {
                                MVMManager.Instance["OTMControlViewModel", "ViewModelIsLoad"] = true;
                                if ((XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "Exclusive", ref str)) && (0 == str.CompareTo("1")))
                                {
                                    BleachPanel.Children.Clear();
                                    BleachPanel.Children.Add(new OTMControlView((IMVM)MVMManager.Instance["OTMControlViewModel", _liveVM]));
                                    _liveVM.BleachExpandHeader = "OTM ";
                                }
                                else
                                {
                                    BleachPanel.Children.Add(new OTMControlView((IMVM)MVMManager.Instance["OTMControlViewModel", _liveVM]));
                                    _liveVM.BleachExpandHeader += "& OTM ";
                                }
                            }
                        }
                    }
                }
                _liveVM.BleachExpandHeader += "Control";

                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/ZView");
                if (ndList.Count > 0)
                {
                    if (XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "Visibility", ref str))
                    {
                        zBorder.Visibility = str.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                    }
                    if (XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "Invert", ref str))
                    {
                        MVMManager.Instance["ZControlViewModel", "ZInvert"] = ("1" == str || Boolean.TrueString == str) ? true : false;
                    }
                    if (XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "Invert2", ref str))
                    {
                        MVMManager.Instance["ZControlViewModel", "Z2Invert"] = ("1" == str || Boolean.TrueString == str) ? true : false;
                    }
                    if (XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "Z1InvertPlusMinusButton", ref str))
                    {
                        MVMManager.Instance["ZControlViewModel", "ZInvertUpDown"] = ("1" == str || Boolean.TrueString == str) ? 1 : 0;
                    }
                    if (XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "Z2InvertPlusMinusButton", ref str))
                    {
                        MVMManager.Instance["ZControlViewModel", "ZInvertUpDown2"] = ("1" == str || Boolean.TrueString == str) ? 1 : 0;
                    }
                    if (XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "InvertLimitsZ1", ref str))
                    {
                        MVMManager.Instance["ZControlViewModel", "ZInvertLimits"] = ("1" == str || Boolean.TrueString == str) ? true : false;
                    }
                    if (XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "InvertLimitsZ2", ref str))
                    {
                        MVMManager.Instance["ZControlViewModel", "Z2InvertLimits"] = ("1" == str || Boolean.TrueString == str) ? true : false;
                    }
                }

                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/PowerView");
                if (ndList.Count > 0)
                {
                    powBorder.Visibility = ndList[0].Attributes["Visibility"].Value.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                }

                //ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/MCLSView");
                //if (ndList.Count > 0)
                //{
                //    mclsBorder.Visibility = ndList[0].Attributes["Visibility"].Value.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                //}

                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/MultiphotonView");
                if (ndList.Count > 0)
                {
                    multiphotonBorder.Visibility = ndList[0].Attributes["Visibility"].Value.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;

                    if (multiphotonBorder.Visibility != Visibility.Visible)
                    {
                        MVMManager.Instance["MultiphotonControlViewModel", "IsCollapsed"] = true;
                    }
                    else
                    {
                        MVMManager.Instance["MultiphotonControlViewModel", "IsCollapsed"] = false;
                    }

                    if (XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "mpFastSeqVisibility", ref str))
                    {
                        MVMManager.Instance["MultiphotonControlViewModel", "Laser1FastSeqVisibility"] = str.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                    }
                    else
                    {
                        MVMManager.Instance["MultiphotonControlViewModel", "Laser1FastSeqVisibility"] = Visibility.Collapsed;
                    }

                    if (XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "shutter2Visibility", ref str))
                    {
                        MVMManager.Instance["MultiphotonControlViewModel", "Laser1Shutter2Visibility"] = str.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                        _liveVM.CollapsedShutter2Visibility = str.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                    }
                    else
                    {
                        MVMManager.Instance["MultiphotonControlViewModel", "Laser1Shutter2Visibility"] = Visibility.Collapsed;
                    }

                    if (XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "BeamStabilizerDataVisibility", ref str))
                    {
                        MVMManager.Instance["MultiphotonControlViewModel", "BeamStabilizerDataVisibility"] = str.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                    }

                    // load initial saved wavelengths for multiphoton laser
                    ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/MultiphotonView/SavedWavelengths");
                    if (ndList.Count > 0)
                    {
                        ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/MultiphotonView/SavedWavelengths/SavedWavelength");
                        if (ndList.Count > 0)
                        {
                            for (int i = 0; i < ndList.Count; i++)
                            {
                                string strWavelength = string.Empty;

                                if (XmlManager.GetAttribute(ndList[i], _liveVM.ApplicationDoc, "wavelength", ref strWavelength))
                                {
                                    int wavelength = 0;
                                    if (true == Int32.TryParse(strWavelength, out wavelength))
                                    {
                                        ((CustomCollection<PC<string>>)MVMManager.Instance["MultiphotonControlViewModel", "PresetWavelengthNames"])[i].Value = wavelength.ToString();
                                    }
                                }
                            }

                        }
                    }
                    else //create default saved wavelengths
                    {
                        XmlNode ndWavelengths = _liveVM.ApplicationDoc.CreateElement("SavedWavelengths");

                        ndList[0].AppendChild(ndWavelengths);

                        // create saved wavelength preset tags in XML file
                        for (int i = 0; i < (int)((CustomCollection<PC<string>>)MVMManager.Instance["MultiphotonControlViewModel", "PresetWavelengths"]).Count; i++)
                        {
                            XmlNode temp = _liveVM.ApplicationDoc.CreateElement("SavedWavelength");

                            ndWavelengths.AppendChild(temp);

                            XmlManager.SetAttribute(temp, _liveVM.ApplicationDoc, "wavelength", "0");
                        }
                    }
                }

                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/PinholeView");
                if (ndList.Count > 0)
                {
                    pinholeBorder.Visibility = ndList[0].Attributes["Visibility"].Value.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                }

                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/KuriosFilterView");
                if (ndList.Count > 0)
                {
                    kuriosControlBorder.Visibility = ndList[0].Attributes["Visibility"].Value.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                }

                bool kuriosActive = false;
                ndList = _liveVM.HardwareDoc.SelectNodes("/HardwareSettings/Devices/SpectrumFilter");
                if (ndList.Count > 0)
                {
                    for (int i = 0; i < ndList.Count; i++)
                    {
                        string strActive = string.Empty;
                        XmlManager.GetAttribute(ndList[i], _liveVM.HardwareDoc, "active", ref strActive);

                        string strName = string.Empty;
                        XmlManager.GetAttribute(ndList[i], _liveVM.HardwareDoc, "dllName", ref strName);

                        if ("1" == strActive && "ThorKurios" == strName)
                        {
                            MVMManager.Instance["CaptureOptionsControlViewModel", "HyperSpectralCaptureActive"] = kuriosActive = true;
                        }
                    }

                    if (true == kuriosActive && Visibility.Visible == kuriosControlBorder.Visibility)
                    {
                        kuriosControlBorder.Visibility = Visibility.Visible;
                    }
                    else
                    {
                        kuriosControlBorder.Visibility = Visibility.Collapsed;
                    }
                }
                else
                {
                    kuriosControlBorder.Visibility = Visibility.Collapsed;
                }

                MVMManager.Instance["CaptureOptionsControlViewModel", "HyperSpectralCaptureActive"] = ccdActive && kuriosActive;

                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/XYView");

                if (ndList.Count > 0)
                {
                    if (XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "Visibility", ref str))
                    {
                        tilesControlBorder.Visibility = str.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                    }

                    if (XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "SubColumnsMax", ref str))
                    {
                        int convertion = 0;
                        if (true == Int32.TryParse(str, out convertion))
                        {
                            MVMManager.Instance["XYTileControlViewModel", "SubColumnsMax", (object)1] = convertion;
                        }
                    }

                    if (XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "SubRowsMax", ref str))
                    {
                        int convertion = 0;
                        if (true == Int32.TryParse(str, out convertion))
                        {
                            MVMManager.Instance["XYTileControlViewModel", "SubRowsMax", (object)1] = convertion;
                        }
                    }

                    if (XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "showInUM", ref str))
                    {
                        int convertion = 0;
                        if (true == Int32.TryParse(str, out convertion))
                        {
                            MVMManager.Instance["XYTileControlViewModel", "ShowInUM", (object)1] = convertion;
                        }
                    }
                }

                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/CaptureOptionsView");
                if (ndList.Count > 0)
                {
                    captureOptionsBorder.Visibility = ndList[0].Attributes["Visibility"].Value.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                }

                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/ScannerView/TwoWayAlignmentPanel");

                if (ndList.Count > 0)
                {
                    if (XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "Visibility", ref str))
                    {
                        MVMManager.Instance["ScanControlViewModel", "TwoWayAlignmentPanelVisibility"] = (str.Equals("Visible")) ? Visibility.Visible : Visibility.Collapsed;
                    }
                }

                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/ScannerView/CoarsePanel");

                if (ndList.Count > 0)
                {
                    if (XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "Visibility", ref str))
                    {
                        MVMManager.Instance["ScanControlViewModel", "CoarsePanelVisibility"] = (str.Equals("Visible") && ((int)ICamera.LSMType.GALVO_RESONANCE == ResourceManagerCS.GetLSMType() || (int)ICamera.LSMType.RESONANCE_GALVO_GALVO == ResourceManagerCS.GetLSMType())) ? Visibility.Visible : Visibility.Collapsed;
                    }
                }

                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/ScannerView/FieldSizePanel");

                if (ndList.Count > 0)
                {
                    if (XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "Visibility", ref str))
                    {
                        MVMManager.Instance["AreaControlViewModel", "FieldSizeVisible"] = (str.Equals("Visible")) ? Visibility.Visible : Visibility.Collapsed;
                    }
                }

                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/TwoWayCalibration");

                if (ndList.Count > 0)
                {
                    if (XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "Visibility", ref str))
                    {
                        MVMManager.Instance["ScanControlViewModel", "TwoWayCalibrationVisibility"] = (str.Equals("Visible")) ? Visibility.Visible : Visibility.Collapsed;
                    }
                }

                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/ROIStatsWindow");

                if (ndList.Count > 0)
                {
                    if (XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "display", ref str))
                    {
                        _liveVM.ROIStatsTableActive = ("1" == str || Boolean.TrueString == str) ? true : false;
                    }
                }

                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/ROIChartWindow");

                if (ndList.Count > 0)
                {
                    if (XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "display", ref str))
                    {
                        _liveVM.ROIStatsChartActive = ("1" == str || Boolean.TrueString == str) ? true : false;
                    }
                }

                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/LineProfileWindow");

                if (ndList.Count > 0)
                {
                    if (XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "display", ref str))
                    {
                        _liveVM.LineProfileActive = ("1" == str || Boolean.TrueString == str) ? true : false;
                    }
                }

                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/ScannerView/DigitalOffset");

                if (0 < ndList.Count)
                {
                    XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "Visibility", ref str);

                    MVMManager.Instance["ScanControlViewModel", "DigOffsetVisibility"] = (str.Equals("Visible")) ? Visibility.Visible : Visibility.Collapsed;
                }

                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/DigitalSwitchesView");
                if (ndList.Count > 0)
                {
                    digitalSwitchesBorder.Visibility = ndList[0].Attributes["Visibility"].Value.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;

                    for (int i = 0; i < ((ObservableCollection<StringPC>)MVMManager.Instance["DigitalOutputSwitchesViewModel", "SwitchName"]).Count; i++)
                    {
                        if (XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, string.Format("switchName{0}", i + 1), ref str))
                        {
                            ((ObservableCollection<StringPC>)MVMManager.Instance["DigitalOutputSwitchesViewModel", "SwitchName"])[i].Value = str;
                        }
                    }
                }

                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/LampView");
                if (ndList.Count > 0)
                {
                    LampBorder.Visibility = ndList[0].Attributes["Visibility"].Value.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                }

                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/ThreePhotonView");
                if (ndList.Count > 0)
                {
                    ThreePhotonControlBorder.Visibility = ndList[0].Attributes["Visibility"].Value.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                }
                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/LightEngineView");
                if (ndList.Count > 0)
                {
                    LightEngineControlBorder.Visibility = ndList[0].Attributes["Visibility"].Value.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                }
                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/EpiturretControlView");
                if (ndList.Count > 0)
                {
                    EpiturretControlBorder.Visibility = ndList[0].Attributes["Visibility"].Value.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                }
                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/DFLIMView");
                if (ndList.Count > 0)
                {
                    DFLIMControlBorder.Visibility = ndList[0].Attributes["Visibility"].Value.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                }
                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/AutoFocusControlView");
                if (ndList.Count > 0)
                {
                    AutoFocusControlBorder.Visibility = ndList[0].Attributes["Visibility"].Value.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                }
                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/MiniCircuitsSwitchControlView");
                if (ndList.Count > 0)
                {
                    MiniCircuitsSwitchControlBorder.Visibility = ndList[0].Attributes["Visibility"].Value.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                }
                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/MultiLaserControlView");
                if (ndList.Count > 0)
                {
                    MultiLaserControlBorder.Visibility = ndList[0].Attributes["Visibility"].Value.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                }
                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/SequentialControlView");
                if (ndList.Count > 0)
                {
                    sequentialControlBorder.Visibility = ndList[0].Attributes["Visibility"].Value.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                }

                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/TwoColumnDisplay");
                if (ndList.Count > 0)
                {
                    XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "enable", ref str);

                    if (str == "0")
                    {
                        _liveVM.CaptureSetupControlPanelWidth = _liveVM.WrapPanelWidth;
                        rightStackPanel.Visibility = System.Windows.Visibility.Collapsed;
                        ArrangeDisplayPanelsSingleColumn();
                    }
                    else
                    {
                        _liveVM.CaptureSetupControlPanelWidth = _liveVM.WrapPanelWidth * 2;
                        rightStackPanel.Visibility = System.Windows.Visibility.Visible;
                        ArrangeDisplayPanelsTwoColumns();
                    }
                }
                //Only display Live Capture Controls if ScanControl or CameraControl are Visible
                _liveVM.ImagerViewVis = ((Visibility.Collapsed == scanBorder.Visibility && Visibility.Collapsed == cameraControlBorder.Visibility) && ResourceManagerCS.Instance.TabletModeEnabled) ? Visibility.Collapsed : Visibility.Visible;
                if (Visibility.Collapsed == _liveVM.ImagerViewVis)
                {
                    if (ResourceManagerCS.Instance.TabletModeEnabled)
                    {
                        _liveVM.WrapPanelWidth = Double.NaN;
                        _liveVM.CaptureSetupControlPanelWidth = Double.NaN;
                    }
                }

                bool showTab = false;
                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/Pockels1");
                if (ndList.Count > 0)
                {
                    if (XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "Visibility", ref str))
                    {
                        if (str.Equals("Visible"))
                        {
                            showTab = true;
                            PowerControlView.pockels0.Visibility = Visibility.Visible;
                        }
                        else
                        {
                            PowerControlView.pockels0.Visibility = Visibility.Collapsed;

                            //select the power regulator if the pockels is unavailable
                            MVMManager.Instance["PowerControlViewModel", "SelectedPowerTab"] = 3;
                        }
                    }

                    ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/Pockels1/Calibration");
                    if (ndList.Count > 0)
                    {
                        if (XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "Visibility", ref str))
                        {
                            PowerControlView.pcPockels0.PockelsCalibrationVisibility = str.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                        }
                    }
                }
                else
                {
                    PowerControlView.pockels0.Visibility = Visibility.Collapsed;
                    //select the power regulator if the pockels is unavailable
                    MVMManager.Instance["PowerControlViewModel", "SelectedPowerTab"] = 3;
                }

                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/Pockels2");
                if (ndList.Count > 0)
                {
                    if (XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "Visibility", ref str))
                    {
                        showTab = true;
                        PowerControlView.pockels1.Visibility = str.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                    }

                    ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/Pockels2/Calibration");
                    if (ndList.Count > 0)
                    {
                        if (XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "Visibility", ref str))
                        {
                            PowerControlView.pcPockels1.PockelsCalibrationVisibility = str.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                        }
                    }
                }
                else
                {
                    PowerControlView.pockels1.Visibility = Visibility.Collapsed;
                }

                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/Pockels3");
                if (ndList.Count > 0)
                {
                    if (XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "Visibility", ref str))
                    {
                        showTab = true;
                        PowerControlView.pockels2.Visibility = str.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                    }

                    ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/Pockels3/Calibration");
                    if (ndList.Count > 0)
                    {
                        if (XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "Visibility", ref str))
                        {
                            PowerControlView.pcPockels2.PockelsCalibrationVisibility = str.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                        }
                    }
                }
                else
                {
                    PowerControlView.pockels2.Visibility = Visibility.Collapsed;
                }

                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/Pockels4");
                if (ndList.Count > 0)
                {
                    if (XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "Visibility", ref str))
                    {
                        showTab = true;
                        PowerControlView.pockels3.Visibility = str.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                    }

                    ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/Pockels4/Calibration");
                    if (ndList.Count > 0)
                    {
                        if (XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "Visibility", ref str))
                        {
                            PowerControlView.pcPockels3.PockelsCalibrationVisibility = str.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                        }
                    }
                }
                else
                {
                    PowerControlView.pockels3.Visibility = Visibility.Collapsed;
                }

                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/PowerReg");
                if (ndList.Count > 0)
                {
                    if (XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "Visibility", ref str))
                    {
                        showTab = true;
                        PowerControlView.powerReg.Visibility = str.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                    }

                    ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/PowerReg/Calibration");
                    if (ndList.Count > 0)
                    {
                        if (XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "Visibility", ref str))
                        {
                            PowerControlView.pcPowerReg.PowerRegCalibrationVisibility = str.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                        }
                    }
                    ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/PowerReg/EncoderPosition");
                    if (ndList.Count > 0)
                    {
                        if (XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "Visibility", ref str))
                        {
                            PowerControlView.pcPowerReg.EncoderPositionVisibility = str.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                        }
                    }
                }
                else
                {
                    PowerControlView.powerReg.Visibility = Visibility.Collapsed;
                }

                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/PowerReg2");
                if (ndList.Count > 0)
                {
                    if (XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "Visibility", ref str))
                    {
                        showTab = true;
                        PowerControlView.powerReg2.Visibility = str.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                    }

                    ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/PowerReg2/Calibration");
                    if (ndList.Count > 0)
                    {
                        if (XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "Visibility", ref str))
                        {
                            PowerControlView.pcPowerReg2.PowerRegCalibrationVisibility = str.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                        }
                    }
                    ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/PowerReg2/EncoderPosition");
                    if (ndList.Count > 0)
                    {
                        if (XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "Visibility", ref str))
                        {
                            PowerControlView.pcPowerReg2.EncoderPositionVisibility = str.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                        }
                    }
                }
                else
                {
                    PowerControlView.powerReg2.Visibility = Visibility.Collapsed;
                }

                MVMManager.Instance["PowerControlViewModel", "Pockels0Visibility"] = _liveVM.CollapsedPockels0Visibility = PowerControlView.pockels0.Visibility;
                MVMManager.Instance["PowerControlViewModel", "Pockels1Visibility"] = _liveVM.CollapsedPockels1Visibility = PowerControlView.pockels1.Visibility;
                MVMManager.Instance["PowerControlViewModel", "Pockels2Visibility"] = _liveVM.CollapsedPockels2Visibility = PowerControlView.pockels2.Visibility;
                MVMManager.Instance["PowerControlViewModel", "Pockels3Visibility"] = _liveVM.CollapsedPockels3Visibility = PowerControlView.pockels3.Visibility;
                MVMManager.Instance["PowerControlViewModel", "PowerRegVisibility"] = _liveVM.CollapsedPowerRegVisibility = PowerControlView.powerReg.Visibility;
                MVMManager.Instance["PowerControlViewModel", "PowerReg2Visibility"] = _liveVM.CollapsedPowerReg2Visibility = PowerControlView.powerReg2.Visibility;

                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/PockelsPowerTabs");
                //maintain the tab for the panel
                int sel = 0;
                if (0 < ndList.Count)
                {
                    if (XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "selected", ref str))
                    {
                        Int32.TryParse(str, out sel);
                    }
                }
                //make sure the tab is visible before setting
                //it to the selected tab
                if (showTab)
                {
                    bool tabVisible = false;
                    tabVisible = ((Visibility)MVMManager.Instance["PowerControlViewModel", _powerTabVisibility[sel], Visibility.Collapsed]).Equals(Visibility.Visible) ? true : false;

                    //selection does not match the visible tabs
                    //move the selection to the first visible tab
                    if (false == tabVisible)
                    {
                        for (int i = 0; i < _powerTabVisibility.Count; i++)
                        {
                            if (null != MVMManager.Instance["PowerControlViewModel", _powerTabVisibility[i]])
                            {
                                if (((Visibility)MVMManager.Instance["PowerControlViewModel", _powerTabVisibility[i]]).Equals(Visibility.Visible))
                                {
                                    sel = i;
                                    break;
                                }
                            }
                            if (3 < i)  //powerReg
                            {
                                sel = 4;
                                break;
                            }
                        }
                    }
                }

                MVMManager.Instance["PowerControlViewModel", "SelectedPowerTab"] = PowerControlView.tabPower.SelectedIndex = sel;

                PowerControlView.tabPower.Visibility = (showTab) ? Visibility.Visible : Visibility.Collapsed;

                if (_liveVM.HardwareDoc != null)
                {
                    ndList = _liveVM.HardwareDoc.SelectNodes("/HardwareSettings/PowerReg");
                    if (ndList.Count > 0)
                    {
                        if (XmlManager.GetAttribute(ndList[0], _liveVM.HardwareDoc, "calibName1", ref str))
                            MVMManager.Instance["PowerControlViewModel", "PowerRegCal1Name"] = str;
                        if (XmlManager.GetAttribute(ndList[0], _liveVM.HardwareDoc, "calibName2", ref str))
                            MVMManager.Instance["PowerControlViewModel", "PowerRegCal2Name"] = str;
                        if (XmlManager.GetAttribute(ndList[0], _liveVM.HardwareDoc, "calibName3", ref str))
                            MVMManager.Instance["PowerControlViewModel", "PowerRegCal3Name"] = str;
                        if (XmlManager.GetAttribute(ndList[0], _liveVM.HardwareDoc, "calibName4", ref str))
                            MVMManager.Instance["PowerControlViewModel", "PowerRegCal4Name"] = str;
                        if (XmlManager.GetAttribute(ndList[0], _liveVM.HardwareDoc, "calibName5", ref str))
                            MVMManager.Instance["PowerControlViewModel", "PowerRegCal5Name"] = str;
                        if (XmlManager.GetAttribute(ndList[0], _liveVM.HardwareDoc, "calibName6", ref str))
                            MVMManager.Instance["PowerControlViewModel", "PowerRegCal6Name"] = str;
                    }

                    ndList = _liveVM.HardwareDoc.SelectNodes("/HardwareSettings/PowerReg2");
                    if (ndList.Count > 0)
                    {
                        if (XmlManager.GetAttribute(ndList[0], _liveVM.HardwareDoc, "calibName1", ref str))
                            MVMManager.Instance["PowerControlViewModel", "PowerReg2CalName1"] = str;
                        if (XmlManager.GetAttribute(ndList[0], _liveVM.HardwareDoc, "calibName2", ref str))
                            MVMManager.Instance["PowerControlViewModel", "PowerReg2CalName2"] = str;
                        if (XmlManager.GetAttribute(ndList[0], _liveVM.HardwareDoc, "calibName3", ref str))
                            MVMManager.Instance["PowerControlViewModel", "PowerReg2CalName3"] = str;
                        if (XmlManager.GetAttribute(ndList[0], _liveVM.HardwareDoc, "calibName4", ref str))
                            MVMManager.Instance["PowerControlViewModel", "PowerReg2CalName4"] = str;
                        if (XmlManager.GetAttribute(ndList[0], _liveVM.HardwareDoc, "calibName5", ref str))
                            MVMManager.Instance["PowerControlViewModel", "PowerReg2CalName5"] = str;
                        if (XmlManager.GetAttribute(ndList[0], _liveVM.HardwareDoc, "calibName6", ref str))
                            MVMManager.Instance["PowerControlViewModel", "PowerReg2CalName6"] = str;
                    }

                    ndList = _liveVM.HardwareDoc.SelectNodes("/HardwareSettings/PowerControllers/PowerControl");
                    if (null != MVMManager.Instance["PowerControlViewModel", "PowerControlName"])
                    {
                        if (ndList.Count >= ((ObservableCollection<StringPC>)MVMManager.Instance["PowerControlViewModel", "PowerControlName"]).Count)
                        {
                            for (int i = 0; i < ((ObservableCollection<StringPC>)MVMManager.Instance["PowerControlViewModel", "PowerControlName"]).Count; i++)
                            {
                                if (XmlManager.GetAttribute(ndList[i], _liveVM.HardwareDoc, "name", ref str))
                                {
                                    ((ObservableCollection<StringPC>)MVMManager.Instance["PowerControlViewModel", "PowerControlName"])[i].Value = string.Format("{0}", str);
                                }
                                if (XmlManager.GetAttribute(ndList[i], _liveVM.HardwareDoc, "threshold", ref str) && Double.TryParse(str, out dVal))
                                {
                                    //the gui is inverted from the true threshold value
                                    //use 1 - val
                                    ((ObservableCollection<DoublePC>)MVMManager.Instance["PowerControlViewModel", "PockelsPowerThreshold"])[i].Value = 1.0 - dVal;
                                }
                            }
                        }
                    }
                    _liveVM.CollapsedPowerControlName = ((ObservableCollection<StringPC>)MVMManager.Instance["PowerControlViewModel", "PowerControlName"]);
                }
            }
            catch (Exception ex)
            {
                ex.ToString();
            }
        }

        private void SetKeyboardOptions()
        {
            XmlNodeList ndList;
            string str = string.Empty;
            string mod = string.Empty;
            string key = string.Empty;

            //Load and adjust visible items from application settings
            _liveVM.ApplicationDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
            ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/KeyboardOptions/ZPlus");
            GetKeyboardProperty(ndList, ref mod, ref key);
            MVMManager.Instance["ZControlViewModel", "ZPosPlusModifier"] = mod;
            MVMManager.Instance["ZControlViewModel", "ZPosPlusKey"] = key;
            ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/KeyboardOptions/ZMinus");
            GetKeyboardProperty(ndList, ref mod, ref key);
            MVMManager.Instance["ZControlViewModel", "ZPosMinusModifier"] = mod;
            MVMManager.Instance["ZControlViewModel", "ZPosMinusKey"] = key;
            ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/KeyboardOptions/ZZero");
            GetKeyboardProperty(ndList, ref mod, ref key);
            MVMManager.Instance["ZControlViewModel", "ZZeroModifier"] = mod;
            MVMManager.Instance["ZControlViewModel", "ZZeroKey"] = key;
            ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/KeyboardOptions/ZStart");
            GetKeyboardProperty(ndList, ref mod, ref key);
            MVMManager.Instance["ZControlViewModel", "ZStartModifier"] = mod;
            MVMManager.Instance["ZControlViewModel", "ZStartKey"] = key;
            ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/KeyboardOptions/ZStop");
            GetKeyboardProperty(ndList, ref mod, ref key);
            MVMManager.Instance["ZControlViewModel", "ZStopModifier"] = mod;
            MVMManager.Instance["ZControlViewModel", "ZStopKey"] = key;
            ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/KeyboardOptions/Z2Plus");
            GetKeyboardProperty(ndList, ref mod, ref key);
            MVMManager.Instance["ZControlViewModel", "Z2PosPlusModifier"] = mod;
            MVMManager.Instance["ZControlViewModel", "Z2PosPlusKey"] = key;
            ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/KeyboardOptions/Z2Minus");
            GetKeyboardProperty(ndList, ref mod, ref key);
            MVMManager.Instance["ZControlViewModel", "Z2PosMinusModifier"] = mod;
            MVMManager.Instance["ZControlViewModel", "Z2PosMinusKey"] = key;
            ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/KeyboardOptions/Z2Zero");
            GetKeyboardProperty(ndList, ref mod, ref key);
            MVMManager.Instance["ZControlViewModel", "Z2ZeroModifier"] = mod;
            MVMManager.Instance["ZControlViewModel", "Z2ZeroKey"] = key;
            ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/KeyboardOptions/XPlus");
            GetKeyboardProperty(ndList, ref mod, ref key);
            MVMManager.Instance["XYTileControlViewModel", "XPlusModifier", (object)string.Empty] = mod;
            MVMManager.Instance["XYTileControlViewModel", "XPlusKey", (object)string.Empty] = key;
            ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/KeyboardOptions/XMinus");
            GetKeyboardProperty(ndList, ref mod, ref key);
            MVMManager.Instance["XYTileControlViewModel", "XMinusModifier", (object)string.Empty] = mod;
            MVMManager.Instance["XYTileControlViewModel", "XMinusKey", (object)string.Empty] = key;
            ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/KeyboardOptions/XZero");
            GetKeyboardProperty(ndList, ref mod, ref key);
            MVMManager.Instance["XYTileControlViewModel", "XZeroModifier", (object)string.Empty] = mod;
            MVMManager.Instance["XYTileControlViewModel", "XZeroKey", (object)string.Empty] = key;
            ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/KeyboardOptions/YPlus");
            GetKeyboardProperty(ndList, ref mod, ref key);
            MVMManager.Instance["XYTileControlViewModel", "YPlusModifier", (object)string.Empty] = mod;
            MVMManager.Instance["XYTileControlViewModel", "YPlusKey", (object)string.Empty] = key;
            ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/KeyboardOptions/YMinus");
            GetKeyboardProperty(ndList, ref mod, ref key);
            MVMManager.Instance["XYTileControlViewModel", "YMinusModifier", (object)string.Empty] = mod;
            MVMManager.Instance["XYTileControlViewModel", "YMinusKey", (object)string.Empty] = key;
            ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/KeyboardOptions/YZero");
            GetKeyboardProperty(ndList, ref mod, ref key);
            MVMManager.Instance["XYTileControlViewModel", "YZeroModifier", (object)string.Empty] = mod;
            MVMManager.Instance["XYTileControlViewModel", "YZeroKey", (object)string.Empty] = key;
            ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/KeyboardOptions/PowerPlus0");
            GetKeyboardProperty(ndList, ref mod, ref key);
            MVMManager.Instance["PowerControlViewModel", "PowerPlusModifier0"] = mod;
            MVMManager.Instance["PowerControlViewModel", "PowerPlusKey0"] = key;
            ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/KeyboardOptions/PowerMinus0");
            GetKeyboardProperty(ndList, ref mod, ref key);
            MVMManager.Instance["PowerControlViewModel", "PowerMinusModifier0"] = mod;
            MVMManager.Instance["PowerControlViewModel", "PowerMinusKey0"] = key;
            ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/KeyboardOptions/PowerPlus1");
            GetKeyboardProperty(ndList, ref mod, ref key);
            MVMManager.Instance["PowerControlViewModel", "PowerPlusModifier1"] = mod;
            MVMManager.Instance["PowerControlViewModel", "PowerPlusKey1"] = key;
            ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/KeyboardOptions/PowerMinus1");
            GetKeyboardProperty(ndList, ref mod, ref key);
            MVMManager.Instance["PowerControlViewModel", "PowerMinusModifier1"] = mod;
            MVMManager.Instance["PowerControlViewModel", "PowerMinusKey1"] = key;
            ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/KeyboardOptions/PowerPlus2");
            GetKeyboardProperty(ndList, ref mod, ref key);
            MVMManager.Instance["PowerControlViewModel", "PowerPlusModifier2"] = mod;
            MVMManager.Instance["PowerControlViewModel", "PowerPlusKey2"] = key;
            ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/KeyboardOptions/PowerMinus2");
            GetKeyboardProperty(ndList, ref mod, ref key);
            MVMManager.Instance["PowerControlViewModel", "PowerMinusModifier2"] = mod;
            MVMManager.Instance["PowerControlViewModel", "PowerMinusKey2"] = key;
            ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/KeyboardOptions/PowerPlus3");
            GetKeyboardProperty(ndList, ref mod, ref key);
            MVMManager.Instance["PowerControlViewModel", "PowerPlusModifier3"] = mod;
            MVMManager.Instance["PowerControlViewModel", "PowerPlusKey3"] = key;
            ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/KeyboardOptions/PowerMinus3");
            GetKeyboardProperty(ndList, ref mod, ref key);
            MVMManager.Instance["PowerControlViewModel", "PowerMinusModifier3"] = mod;
            MVMManager.Instance["PowerControlViewModel", "PowerMinusKey3"] = key;
            ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/KeyboardOptions/PowerPlusReg");
            GetKeyboardProperty(ndList, ref mod, ref key);
            MVMManager.Instance["PowerControlViewModel", "PowerPlusModifierReg"] = mod;
            MVMManager.Instance["PowerControlViewModel", "PowerPlusKeyReg"] = key;
            ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/KeyboardOptions/PowerMinusReg");
            GetKeyboardProperty(ndList, ref mod, ref key);
            MVMManager.Instance["PowerControlViewModel", "PowerMinusModifierReg"] = mod;
            MVMManager.Instance["PowerControlViewModel", "PowerMinusKeyReg"] = key;
            ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/KeyboardOptions/Start");
            GetKeyboardProperty(ndList, ref mod, ref key);
            _liveVM.StartModifier = mod;
            _liveVM.StartKey = key;
            ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/KeyboardOptions/Snapshot");
            GetKeyboardProperty(ndList, ref mod, ref key);
            _liveVM.SnapshotModifier = mod;
            _liveVM.SnapshotKey = key;
            ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/KeyboardOptions/BleachNow");
            GetKeyboardProperty(ndList, ref mod, ref key);
            _liveVM.BleachNowModifier = mod;
            _liveVM.BleachNowKey = key;
            ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/KeyboardOptions/GGLightPath");
            GetKeyboardProperty(ndList, ref mod, ref key);
            MVMManager.Instance["LightPathControlViewModel", "GGLightPathModifier"] = mod;
            MVMManager.Instance["LightPathControlViewModel", "GGLightPathKey"] = key;
            ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/KeyboardOptions/GRLightPath");
            GetKeyboardProperty(ndList, ref mod, ref key);
            MVMManager.Instance["LightPathControlViewModel", "GRLightPathModifier"] = mod;
            MVMManager.Instance["LightPathControlViewModel", "GRLightPathKey"] = key;
            ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/KeyboardOptions/CamLightPath");
            GetKeyboardProperty(ndList, ref mod, ref key);
            MVMManager.Instance["LightPathControlViewModel", "CamLightPathModifier"] = mod;
            MVMManager.Instance["LightPathControlViewModel", "CamLightPathKey"] = key;
        }

        private void Shutter_Click(object sender, RoutedEventArgs e)
        {
        }

        private void zExpander_GotFocus(object sender, MouseButtonEventArgs e)
        {
            zExpander_MouseGotFocus = true;
        }

        private void zExpander_LostMouseCapture(object sender, MouseEventArgs e)
        {
            zExpander_MouseGotFocus = true;
        }

        private void zExpander_MouseWheel(object sender, MouseWheelEventArgs e)
        {
            if (zExpander_MouseGotFocus)
            {
                //disable scroll function at ScrollViewer:
                e.Handled = true;
            }
        }

        #endregion Methods
    }
}