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

        const int DISPLAY_PANELS = 20;
        const int NUM_CHANNELS = 4;

        private bool zExpander_MouseGotFocus = false;

        //object _dataContextliveImageVM = null;
        private string _imageProcessSettingsFile;
        CaptureSetupViewModel _liveVM = null;
        string[,] _panelInfo = new string[DISPLAY_PANELS, 3] {
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/ScannerView","Scanner Control","scanBorder"},
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/AreaView","Area Control","areaBorder"},
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/PowerView","Power Control","powBorder"},
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/ZView","Z Control","zBorder"},
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/MCLSView","MCLS Control","mclsBorder"},
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
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/EpiturretControlView","EpiturretControlView","EpiturretControlBorder"}
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

            //persist global params in all modalities after loading:
            for (int i = 0; i < (int)GlobalExpAttribute.LAST; i++)
            {
                _liveVM.PersistGlobalExperimentXML((GlobalExpAttribute)i);
            }
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

                if (false == _liveVM.AutoFocus())
                {
                    ThorLog.Instance.TraceEvent(TraceEventType.Information, 1, this.GetType().Name + " Autofocus failed returning to previous Z position");

                    MVMManager.Instance["ZControlViewModel", "ZPosition"] = currentPosition;
                }

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
                    if (1 == _liveVM.EnableSequentialCapture && 1 < _liveVM.CollectionCaptureSequence.Count)
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

                if (!Directory.Exists(_liveVM.SettingsTemplatesPath))
                {
                    Directory.CreateDirectory(_liveVM.SettingsTemplatesPath);
                }

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

            if (sender != mclsExpander)
            {
                if (mclsExpander != null)
                {
                    mclsExpander.IsExpanded = false;
                }
            }

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

        private void LoadChannelSelection()
        {
            try
            {
                XmlNodeList ChannelEnable = _liveVM.ExperimentDoc.GetElementsByTagName("ChannelEnable");
                if (ChannelEnable.Count > 0)
                {
                    _liveVM.LSMChannelEnable0 = false;
                    _liveVM.LSMChannelEnable1 = false;
                    _liveVM.LSMChannelEnable2 = false;
                    _liveVM.LSMChannelEnable3 = false;

                    string ChanEnableSet = string.Empty;
                    XmlManager.GetAttribute(ChannelEnable[0], _liveVM.ExperimentDoc, "Set", ref ChanEnableSet);
                    //iterate through binary string to check channels:
                    int tmp = 0;
                    if (Int32.TryParse(ChanEnableSet, out tmp))
                    {
                        int maxChannels = _liveVM.MaxChannels;
                        string binaryString = Convert.ToString(tmp, 2).PadLeft(maxChannels, '0');

                        for (int i = 0; i < _liveVM.MaxChannels; i++)
                        {
                            if ((i == 0) && (binaryString[maxChannels - 1 - i] == '1'))
                            {
                                _liveVM.LSMChannelEnable0 = true;
                            }
                            else if ((i == 1) && (binaryString[maxChannels - 1 - i] == '1'))
                            {
                                _liveVM.LSMChannelEnable1 = true;
                            }
                            else if ((i == 2) && (binaryString[maxChannels - 1 - i] == '1'))
                            {
                                _liveVM.LSMChannelEnable2 = true;
                            }
                            else if ((i == 3) && (binaryString[maxChannels - 1 - i] == '1'))
                            {
                                _liveVM.LSMChannelEnable3 = true;
                            }
                        }
                    }
                }

                //Set the non-visible channels to disabled
                XmlDocument hwDoc = (XmlDocument)MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];
                XmlNodeList ndWLList = hwDoc.SelectNodes("/HardwareSettings/Wavelength");

                for (int i = 0; i < NUM_CHANNELS; i++)
                {
                    if (i < ndWLList.Count && i < _liveVM.MaxChannels)
                    {
                    }
                    else
                    {
                        switch (i)
                        {
                            case 0: _liveVM.LSMChannelEnable0 = false; break;
                            case 1: _liveVM.LSMChannelEnable1 = false; break;
                            case 2: _liveVM.LSMChannelEnable2 = false; break;
                            case 3: _liveVM.LSMChannelEnable3 = false; break;
                        }
                    }
                }

            }
            catch (Exception ex)
            {
                ex.ToString();
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + ex.Message);

            }
        }

        void LoadImageProcess()
        {
            XmlNodeList ndList = this._liveVM.ImageProcessDoc.SelectNodes("/ImageProcessSettings/General");
            if (ndList.Count > 0)
            {
                string str = string.Empty;

                if (XmlManager.GetAttribute(ndList[0], this._liveVM.ImageProcessDoc, "MaxRoiNum", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        _liveVM.MaxRoiNum = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], this._liveVM.ImageProcessDoc, "MinSnr", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        _liveVM.MinSnr = tmp;
                    }
                }
            }

            ndList = this._liveVM.ImageProcessDoc.SelectNodes("/ImageProcessSettings/Filter/MinArea");
            if (ndList.Count > 0)
            {
                string str = string.Empty;

                if (XmlManager.GetAttribute(ndList[0], this._liveVM.ImageProcessDoc, "active", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        _liveVM.MinAreaActive = Convert.ToBoolean(tmp & 0x1);
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], this._liveVM.ImageProcessDoc, "value", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        _liveVM.MinAreaValue = tmp;
                    }
                }
            }
        }

        void LoadLSMHardwareSettings()
        {
            string str = string.Empty;
            XmlDocument hardwareDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];

            //set visibility of controls based on number of wavelengths available to the system
            XmlNodeList ndList = hardwareDoc.SelectNodes("/HardwareSettings/Wavelength");

            _liveVM.NumChannelsAvailableForDisplay = ndList.Count;

            for (int i = 0; i < ndList.Count; i++)
            {
                if (XmlManager.GetAttribute(ndList[i], hardwareDoc, "name", ref str))
                    _liveVM.ChannelName[i].Value = str;
            }

            ndList = hardwareDoc.SelectNodes("/HardwareSettings/ImageDetectors/LSM");

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
            try
            {
                //setting initial position and size of the positionRectangle of the canvas.
                _liveVM = (CaptureSetupViewModel)this.DataContext;

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

                //Verify there is only one channel per color selected
                VerifyChannelColorSelection();

                //load hardware settings from file
                LoadLSMHardwareSettings();

                //load the magnification combo box using the hardware settings document
                XmlDataProvider dataProvider = (XmlDataProvider)this.FindResource("HardwareSettings");

                dataProvider.Document = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];

                ////loading the color channel selection before loading MVM settings:
                LoadChannelSelection();

                //load all MVMs:
                MVMManager.Instance.LoadMVMSettings();

                //loading the default color image settings
                _liveVM.LoadColorImageSettings();

                _liveVM.ConnectHandlers();

                //Load Image Process Settings
                LoadImageProcess();
            }
            catch (Exception ex)
            {
                ThorLogging.ThorLog.Instance.TraceEvent(System.Diagnostics.TraceEventType.Error, 1, "CaptureSetup MasterView load error: " + ex.Message);
                MessageBox.Show("There was an error at loading Capture Setup. Some of your properties may not have been updated.");
            }
        }

        private void selectChannelUsingConverter(int val)
        {
            switch (_liveVM.DigitizerBoardName)
            {
                case Model.CaptureSetup.DigitizerBoardNames.ATS9440:
                    {
                        _liveVM.LSMChannelEnable0 = Convert.ToBoolean(val & 0x1);
                        _liveVM.LSMChannelEnable1 = Convert.ToBoolean(val & 0x2);
                        _liveVM.LSMChannelEnable2 = Convert.ToBoolean(val & 0x4);
                        _liveVM.LSMChannelEnable3 = Convert.ToBoolean(val & 0x8);
                    }
                    break;
                case Model.CaptureSetup.DigitizerBoardNames.ATS460:
                    {
                        _liveVM.LSMChannelEnable0 = Convert.ToBoolean(val & 0x1);
                        _liveVM.LSMChannelEnable1 = Convert.ToBoolean(val & 0x2);
                        _liveVM.LSMChannelEnable2 = false;
                    }
                    break;
            }
        }

        private void SetDisplayOptions()
        {
            try
            {
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
                        string str = string.Empty;
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
                        string str = string.Empty;
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
                        _liveVM.IsChannelVisible1 = Visibility.Collapsed;
                        _liveVM.IsChannelVisible2 = Visibility.Collapsed;
                        _liveVM.IsChannelVisible3 = Visibility.Collapsed;
                        if (ResourceManagerCS.Instance.TabletModeEnabled)
                        {
                            //When in camera mode with the tablet reduce the size of the panels to 87%
                            //so the camera control fits in the screen
                            _liveVM.PanelsScale = 0.87;
                            _liveVM.IsTileDisplayButtonVisible = Visibility.Collapsed;
                        }
                    }
                    else
                    {
                        cameraControlExpander.Visibility = cameraControlBorder.Visibility = Visibility.Collapsed;
                        _liveVM.IsChannelVisible1 = Visibility.Visible;
                        _liveVM.IsChannelVisible2 = Visibility.Visible;
                        _liveVM.IsChannelVisible3 = Visibility.Visible;
                        if (ResourceManagerCS.Instance.TabletModeEnabled)
                        {
                            //When switching to joystick modality change the panels scale to 100%, each
                            // panel has it's own scaling
                            _liveVM.PanelsScale = 1;
                            _liveVM.IsTileDisplayButtonVisible = Visibility.Visible;
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

                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/BleachView");

                if (ndList.Count > 0)
                {
                    if ((int)ICamera.LSMType.LSMTYPE_LAST != ResourceManagerCS.GetBleacherType())
                    {
                        BleachBorder.Visibility = ndList[0].Attributes["Visibility"].Value.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                        if (ndList[0].Attributes["Visibility"].Value.Equals("Visible"))
                        {
                            MVMManager.Instance["CaptureOptionsControlViewModel", "BleachControlActive"] = true;
                        }
                        else
                        {
                            MVMManager.Instance["CaptureOptionsControlViewModel", "BleachControlActive"] = false;
                        }
                    }
                    else
                    {
                        BleachBorder.Visibility = System.Windows.Visibility.Collapsed;
                        MVMManager.Instance["CaptureOptionsControlViewModel", "BleachControlActive"] = false;
                    }
                    //Turn bleach panel GUI response On or OFF:
                    _liveVM.BleachBorderEnabled = (BleachBorder.Visibility == Visibility.Visible) ? true : false;
                }

                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/RunSample/FastZView");

                if (ndList.Count > 0)
                {
                    MVMManager.Instance["CaptureOptionsControlViewModel", "FastZActive"] = ndList[0].Attributes["Visibility"].Value.Equals("Visible");
                }

                //set visibility of bleach controls based on SLM device:
                ndList = _liveVM.HardwareDoc.SelectNodes("/HardwareSettings/Devices/SLM");
                BleachPanel.Children.Clear();
                _liveVM.BleachExpandHeader = string.Empty;
                string slmStr = string.Empty;
                _liveVM.SLMPanelInUse = false;
                for (int i = 0; i < ndList.Count; i++)
                {
                    if ((XmlManager.GetAttribute(ndList[i], _liveVM.HardwareDoc, "dllName", ref slmStr)) && (0 == slmStr.CompareTo("ThorSLMPDM512")))
                    {
                        _liveVM.SLMPanelInUse = (XmlManager.GetAttribute(ndList[i], _liveVM.HardwareDoc, "active", ref slmStr)) && (0 == slmStr.CompareTo("1"));
                    }
                }
                if (0 == ndList.Count)
                { _liveVM.SLMPanelInUse = false; }
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
                    string str = string.Empty;
                    if ((XmlManager.GetAttribute(ndList[i], _liveVM.HardwareDoc, "cameraName", ref slmStr)) && ((0 == slmStr.CompareTo("GGNI")) || (0 == slmStr.CompareTo("ResonanceGalvoSim")))
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
                    string str = string.Empty;
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

                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/MCLSView");
                if (ndList.Count > 0)
                {
                    mclsBorder.Visibility = ndList[0].Attributes["Visibility"].Value.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                }

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

                    string str = string.Empty;

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
                    string str = string.Empty;

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

                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/ScannerView/CoarsePanel");

                if (ndList.Count > 0)
                {
                    string str = string.Empty;

                    if (XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "Visibility", ref str))
                    {
                        MVMManager.Instance["ScanControlViewModel", "CoarsePanelVisibility"] = (str.Equals("Visible") && ((int)ICamera.LSMType.GALVO_RESONANCE == ResourceManagerCS.GetLSMType())) ? Visibility.Visible : Visibility.Collapsed;
                    }
                }

                //added
                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/ScannerView/FieldSizePanel");

                if (ndList.Count > 0)
                {
                    string str = string.Empty;

                    if (XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "Visibility", ref str))
                    {
                        MVMManager.Instance["AreaControlViewModel", "FieldSizeVisible"] = (str.Equals("Visible")) ? Visibility.Visible : Visibility.Collapsed;
                    }
                }

                //
                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/TwoWayCalibration");

                if (ndList.Count > 0)
                {
                    string str = string.Empty;

                    if (XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "Visibility", ref str))
                    {
                        MVMManager.Instance["ScanControlViewModel", "TwoWayCalibrationVisibility"] = (str.Equals("Visible")) ? Visibility.Visible : Visibility.Collapsed;
                    }
                }

                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/General/GrayscaleForSingleChannel");

                if (ndList.Count > 0)
                {
                    string str = string.Empty;

                    if (XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "value", ref str))
                    {
                        _liveVM.GrayscaleForSingleChannel = ("1" == str || Boolean.TrueString == str) ? true : false;
                    }
                }
                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/ROIStatsWindow");

                if (ndList.Count > 0)
                {
                    string str = string.Empty;

                    if (XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "display", ref str))
                    {
                        _liveVM.ROIStatsTableActive = ("1" == str || Boolean.TrueString == str) ? true : false;
                    }
                }

                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/ROIChartWindow");

                if (ndList.Count > 0)
                {
                    string str = string.Empty;

                    if (XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "display", ref str))
                    {
                        _liveVM.ROIStatsChartActive = ("1" == str || Boolean.TrueString == str) ? true : false;
                    }
                }

                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/LineProfileWindow");

                if (ndList.Count > 0)
                {
                    string str = string.Empty;

                    if (XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "display", ref str))
                    {
                        _liveVM.LineProfileActive = ("1" == str || Boolean.TrueString == str) ? true : false;
                    }
                }

                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/ScannerView/DigitalOffset");

                if (0 < ndList.Count)
                {
                    string str = string.Empty;
                    XmlManager.GetAttribute(ndList[0], _liveVM.ApplicationDoc, "Visibility", ref str);

                    MVMManager.Instance["ScanControlViewModel", "DigOffsetVisibility"] = (str.Equals("Visible")) ? Visibility.Visible : Visibility.Collapsed;
                }

                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/DigitalSwitchesView");
                if (ndList.Count > 0)
                {
                    digitalSwitchesBorder.Visibility = ndList[0].Attributes["Visibility"].Value.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;

                    for (int i = 0; i < ((ObservableCollection<StringPC>)MVMManager.Instance["DigitalOutputSwitchesViewModel", "SwitchName"]).Count; i++)
                    {
                        string str = string.Empty;
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

                ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/TwoColumnDisplay");
                if (ndList.Count > 0)
                {
                    string str = string.Empty;

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
                    string str = string.Empty;
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
                    string str = string.Empty;
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
                    string str = string.Empty;
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
                    string str = string.Empty;
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
                    string str = string.Empty;
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
                    string str = string.Empty;
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
                    string str = string.Empty;
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
                        string str = string.Empty;
                        XmlManager.GetAttribute(ndList[0], _liveVM.HardwareDoc, "calibName1", ref str);
                        MVMManager.Instance["PowerControlViewModel", "PowerRegCal1Name"] = str;
                        XmlManager.GetAttribute(ndList[0], _liveVM.HardwareDoc, "calibName2", ref str);
                        MVMManager.Instance["PowerControlViewModel", "PowerRegCal2Name"] = str;
                        XmlManager.GetAttribute(ndList[0], _liveVM.HardwareDoc, "calibName3", ref str);
                        MVMManager.Instance["PowerControlViewModel", "PowerRegCal3Name"] = str;
                        XmlManager.GetAttribute(ndList[0], _liveVM.HardwareDoc, "calibName4", ref str);
                        MVMManager.Instance["PowerControlViewModel", "PowerRegCal4Name"] = str;
                        XmlManager.GetAttribute(ndList[0], _liveVM.HardwareDoc, "calibName5", ref str);
                        MVMManager.Instance["PowerControlViewModel", "PowerRegCal5Name"] = str;
                        XmlManager.GetAttribute(ndList[0], _liveVM.HardwareDoc, "calibName6", ref str);
                        MVMManager.Instance["PowerControlViewModel", "PowerRegCal6Name"] = str;
                    }

                    ndList = _liveVM.HardwareDoc.SelectNodes("/HardwareSettings/PowerReg2");
                    if (ndList.Count > 0)
                    {
                        string str = string.Empty;
                        XmlManager.GetAttribute(ndList[0], _liveVM.HardwareDoc, "calibName1", ref str);
                        MVMManager.Instance["PowerControlViewModel", "PowerReg2CalName1"] = str;
                        XmlManager.GetAttribute(ndList[0], _liveVM.HardwareDoc, "calibName2", ref str);
                        MVMManager.Instance["PowerControlViewModel", "PowerReg2CalName2"] = str;
                        XmlManager.GetAttribute(ndList[0], _liveVM.HardwareDoc, "calibName3", ref str);
                        MVMManager.Instance["PowerControlViewModel", "PowerReg2CalName3"] = str;
                        XmlManager.GetAttribute(ndList[0], _liveVM.HardwareDoc, "calibName4", ref str);
                        MVMManager.Instance["PowerControlViewModel", "PowerReg2CalName4"] = str;
                        XmlManager.GetAttribute(ndList[0], _liveVM.HardwareDoc, "calibName5", ref str);
                        MVMManager.Instance["PowerControlViewModel", "PowerReg2CalName5"] = str;
                        XmlManager.GetAttribute(ndList[0], _liveVM.HardwareDoc, "calibName6", ref str);
                        MVMManager.Instance["PowerControlViewModel", "PowerReg2CalName6"] = str;
                    }

                    ndList = _liveVM.HardwareDoc.SelectNodes("/HardwareSettings/PowerControllers/PowerControl");
                    if (null != MVMManager.Instance["PowerControlViewModel", "PowerControlName"])
                    {
                        if (ndList.Count >= ((ObservableCollection<StringPC>)MVMManager.Instance["PowerControlViewModel", "PowerControlName"]).Count)
                        {
                            string str = string.Empty;

                            for (int i = 0; i < ((ObservableCollection<StringPC>)MVMManager.Instance["PowerControlViewModel", "PowerControlName"]).Count; i++)
                            {
                                if (XmlManager.GetAttribute(ndList[i], _liveVM.HardwareDoc, "name", ref str))
                                {
                                    ((ObservableCollection<StringPC>)MVMManager.Instance["PowerControlViewModel", "PowerControlName"])[i].Value = string.Format("{0}", str);
                                }
                                if (XmlManager.GetAttribute(ndList[i], _liveVM.HardwareDoc, "threshold", ref str))
                                {
                                    double dVal = 0;
                                    Double.TryParse(str, out dVal);
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
            _liveVM.GGLightPathModifier = mod;
            _liveVM.GGLightPathKey = key;
            ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/KeyboardOptions/GRLightPath");
            GetKeyboardProperty(ndList, ref mod, ref key);
            _liveVM.GRLightPathModifier = mod;
            _liveVM.GRLightPathKey = key;
            ndList = _liveVM.ApplicationDoc.SelectNodes("/ApplicationSettings/KeyboardOptions/CamLightPath");
            GetKeyboardProperty(ndList, ref mod, ref key);
            _liveVM.CamLightPathModifier = mod;
            _liveVM.CamLightPathKey = key;
        }

        private void Shutter_Click(object sender, RoutedEventArgs e)
        {
        }

        private void VerifyChannelColorSelection()
        {
            bool chanA = false;
            bool chanB = false;
            bool chanC = false;
            bool chanD = false;
            XmlNodeList ndList = _liveVM.HardwareDoc.SelectNodes("/HardwareSettings/ColorChannels/*");
            string str = string.Empty;
            for (int i = 0; i < ndList.Count; i++)
            {
                if (XmlManager.GetAttribute(ndList[i], _liveVM.HardwareDoc, "name", ref str))
                {
                    if (str.Contains("ChanA"))
                    {
                        if (true == chanA)
                        {
                            XmlManager.SetAttribute(ndList[i], _liveVM.HardwareDoc, "name", "None");
                        }
                        chanA = true;
                    }
                    else if (str.Contains("ChanB"))
                    {
                        if (true == chanB)
                        {
                            XmlManager.SetAttribute(ndList[i], _liveVM.HardwareDoc, "name", "None");
                        }
                        chanB = true;

                    }
                    else if (str.Contains("ChanC"))
                    {
                        if (true == chanC)
                        {
                            XmlManager.SetAttribute(ndList[i], _liveVM.HardwareDoc, "name", "None");
                        }
                        chanC = true;

                    }
                    else if (str.Contains("ChanD"))
                    {
                        if (true == chanD)
                        {
                            XmlManager.SetAttribute(ndList[i], _liveVM.HardwareDoc, "name", "None");
                        }
                        chanD = true;
                    }
                }
            }

            MVMManager.Instance.SaveSettings(SettingsFileType.HARDWARE_SETTINGS);
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