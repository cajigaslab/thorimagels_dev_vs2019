namespace HardwareSetupDll.View
{
    using System;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Reflection;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Media;
    using System.Xml;

    using FolderDialogControl;

    using HardwareSetupDll.ViewModel;

    using HardwareSetupUserControl;

    using ThorImageInfastructure;

    using ThorLogging;

    /// <summary>
    /// Interaction logic for DisplayOptions.xaml
    /// </summary>
    public partial class DisplayOptions : Window
    {
        #region Fields

        const int DISPLAY_PANELS = 22;
        private const int WINDOW_HEIGHT = 400;
        private const int WINDOW_LEFT = 0;
        private const int WINDOW_TOP = 0;
        private const int WINDOW_WIDTH = 400;

        private string _appSettings;
        private XmlDocument _doc;
        string[,] _panelInfo = new string[DISPLAY_PANELS, 2] {
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/ScannerView","Scanner Control"},
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/AreaView","Area Control"},
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/LightPathView","Light Path Control"},
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/BleachView","Stimulation Control"},
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/ZView","Z Control"},
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/PowerView","Power Control"},
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/MultiLaserControlView", "Multi Laser Power Control"},
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/MultiphotonView", "Multiphoton Control"},
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/PinholeView","Pinhole Wheel Control"},
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/XYView","Tiles Control"},
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/CaptureOptionsView","Capture Options"},
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/KuriosFilterView","Kurios Filter"},
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/CameraView","Camera Control"},
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/DigitalSwitchesView","Digital Switches Control"},
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/QuickTemplate","Quick Template"},
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/LampView","Lamp Control"},
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/ThreePhotonView","ThreePhoton Control"},
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/LightEngineView","LightEngine Control"},
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/DFLIMView","Digital FLIM Control"},
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/EpiturretControlView","Epiturret Control"},
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/AutoFocusControlView","AutoFocus Control"},
                {"/ApplicationSettings/DisplayOptions/CaptureSetup/MiniCircuitsSwitchControlView","MiniCircuits Switch Control"},
            };

        #endregion Fields

        #region Constructors

        public DisplayOptions()
        {
            InitializeComponent();

            this.Loaded += new RoutedEventHandler(DisplayOptions_Loaded);
            this.Unloaded += new RoutedEventHandler(DisplayOptions_Unloaded);
        }

        #endregion Constructors

        #region Methods

        [DllImport(".\\ResourceManager.dll", EntryPoint = "GetApplicationSettingsFilePathAndName", CharSet = CharSet.Unicode)]
        public static extern int GetApplicationSettingsFilePathAndName(StringBuilder sb, int length);

        public string GetApplicationSettingsFileString()
        {
            const int PATH_LENGTH = 261;
            StringBuilder sb = new StringBuilder(PATH_LENGTH);
            GetApplicationSettingsFilePathAndName(sb, PATH_LENGTH);

            return sb.ToString();
        }

        private void btnDown_Click(object sender, RoutedEventArgs e)
        {
            //move the selected item down one in the list
            int index = lbDisplayOrder.SelectedIndex;

            if (index < 0)
                return;

            var cb = lbDisplayOrder.Items[index];

            int newIndex = Math.Min(lbDisplayOrder.Items.Count - 1, index + 1);

            lbDisplayOrder.Items.RemoveAt(index);

            lbDisplayOrder.Items.Insert(newIndex, cb);

            lbDisplayOrder.SelectedIndex = newIndex;
        }

        private void btnUp_Click(object sender, RoutedEventArgs e)
        {
            //move the selected item up one in the list
            int index = lbDisplayOrder.SelectedIndex;

            if (index < 0)
                return;

            var cb = lbDisplayOrder.Items[index];

            lbDisplayOrder.Items.RemoveAt(index);

            int newIndex = Math.Max(0, index - 1);

            lbDisplayOrder.Items.Insert(newIndex, cb);

            lbDisplayOrder.SelectedIndex = newIndex;
        }

        private void butOk_Click(object sender, RoutedEventArgs e)
        {
            this.DialogResult = true;
            this.Close();
        }

        private void CheckBox_Click(object sender, RoutedEventArgs e)
        {
            var cb = sender as CheckBox;
            var item = cb.DataContext;
            lbDisplayOrder.SelectedItem = item;
        }

        void DisplayOptions_Loaded(object sender, RoutedEventArgs e)
        {
            object obj = GetApplicationSettingsFileString();

            if (obj != null)
            {
                _appSettings = obj.ToString();

                _doc = new XmlDocument();

                _doc.Load(_appSettings);

                XmlNodeList ndList;

                int j = 0;

                do
                {                    //reorder the items with the values from the settings file
                    for (int i = 0; i < DISPLAY_PANELS; i++)
                    {
                        string str = string.Empty;

                        ndList = _doc.SelectNodes(_panelInfo[i, 0]);

                        if (ndList.Count > 0)
                        {
                            if (GetAttribute(ndList[0], _doc, "location", ref str))
                            {
                                int loc = 0;
                                loc = int.TryParse(str, out loc) ? loc : 0;

                                if (loc == j)
                                {
                                    loc = Math.Min(lbDisplayOrder.Items.Count, loc);

                                    CheckBox cb = null;
                                    cb = new CheckBox();
                                    cb.Margin = new Thickness(5);
                                    cb.Foreground = Brushes.White;
                                    cb.Content = _panelInfo[i, 1];
                                    lbDisplayOrder.Items.Insert(Math.Max(0, Math.Min(lbDisplayOrder.Items.Count, loc)), cb);

                                    if (GetAttribute(ndList[0], _doc, "Visibility", ref str))
                                    {
                                        ((CheckBox)lbDisplayOrder.Items[loc]).IsChecked = str.Equals("Visible");
                                    }
                                    break;
                                }
                            }
                        }
                    }
                    j++;
                }
                while (j < DISPLAY_PANELS);

                ndList = _doc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/MultiPanelView");
                if (ndList.Count > 0)
                {
                    cbExpander.IsChecked = ndList[0].Attributes["Visibility"].Value.Equals("Visible");
                }

                //check if the two column option is enabled
                ndList = _doc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/TwoColumnDisplay");
                if (ndList.Count > 0)
                {
                    string str = string.Empty;

                    if (GetAttribute(ndList[0], _doc, "enable", ref str))
                    {
                        cbTwoColumnDisplay.IsChecked = (Convert.ToInt32(str) == 0) ? false : true;
                    }
                }
            }
        }

        void DisplayOptions_Unloaded(object sender, RoutedEventArgs e)
        {
            object obj = GetApplicationSettingsFileString();

            if (obj != null)
            {
                _appSettings = obj.ToString();

                _doc = new XmlDocument();

                _doc.Load(_appSettings);

                XmlNodeList ndList;

                for (int i = 0; i < lbDisplayOrder.Items.Count; i++)
                {
                    string str = string.Empty;

                    str = ((CheckBox)lbDisplayOrder.Items[i]).Content.ToString();

                    int j = 0;
                    for (j = 0; j < DISPLAY_PANELS; j++)
                    {
                        if (str.Equals(_panelInfo[j, 1]))
                        {
                            break;
                        }
                    }

                    if (j < DISPLAY_PANELS)
                    {
                        ndList = _doc.SelectNodes(_panelInfo[j, 0]);

                        if (ndList.Count > 0)
                        {
                            SetAttribute(ndList[0], _doc, "location", i.ToString());

                            SetAttribute(ndList[0], _doc, "Visibility", ((CheckBox)lbDisplayOrder.Items[i]).IsChecked == true ? Visibility.Visible.ToString() : Visibility.Collapsed.ToString());
                        }
                    }
                }
                ndList = _doc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/MultiPanelView");
                if (ndList.Count > 0)
                {
                    ndList[0].Attributes["Visibility"].Value = cbExpander.IsChecked.Value ? "Visible" : "Collapsed";
                }

                ndList = _doc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/TwoColumnDisplay");
                if (ndList.Count > 0)
                {
                    ndList[0].Attributes["enable"].Value = cbTwoColumnDisplay.IsChecked.Value ? "1" : "0";
                }
            }

            _doc.Save(_appSettings);
        }

        private bool GetAttribute(XmlNode node, XmlDocument doc, string attrName, ref string attrValue)
        {
            bool ret;

            if (null == node.Attributes.GetNamedItem(attrName))
            {
                ret = false;
            }
            else
            {
                attrValue = node.Attributes[attrName].Value;
                ret = true;
            }

            return ret;
        }

        private void ResetWindowPos(string nodeName)
        {
            try
            {

                string AppSetPath = GetApplicationSettingsFileString();

                XmlNodeList ndList;
                XmlDocument appSettingsDoc = new XmlDocument();
                appSettingsDoc.Load(AppSetPath);
                ndList = appSettingsDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/" + nodeName);
                if (ndList.Count > 0)
                {
                    XmlNode node = ndList[0];
                    SetAttribute(ndList[0], appSettingsDoc, "left", WINDOW_LEFT.ToString());
                    SetAttribute(ndList[0], appSettingsDoc, "top", WINDOW_TOP.ToString());
                    SetAttribute(ndList[0], appSettingsDoc, "reset", "1");
                }
                //save the information:
                appSettingsDoc.Save(AppSetPath);

            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + "PersistLineProfileWindowPos exception " + ex.Message);
            }
        }

        private void ResetWindowPosAndSize(string nodeName)
        {
            try
            {

                string AppSetPath = GetApplicationSettingsFileString();

                XmlNodeList ndList;
                XmlDocument appSettingsDoc = new XmlDocument();
                appSettingsDoc.Load(AppSetPath);
                ndList = appSettingsDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/" + nodeName);
                if (ndList.Count > 0)
                {
                    XmlNode node = ndList[0];
                    SetAttribute(ndList[0], appSettingsDoc, "left", WINDOW_LEFT.ToString());
                    SetAttribute(ndList[0], appSettingsDoc, "top", WINDOW_TOP.ToString());
                    SetAttribute(ndList[0], appSettingsDoc, "width", WINDOW_WIDTH.ToString());
                    SetAttribute(ndList[0], appSettingsDoc, "height", WINDOW_HEIGHT.ToString());
                    SetAttribute(ndList[0], appSettingsDoc, "reset", "1");
                }
                //save the information:
                appSettingsDoc.Save(AppSetPath);

            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + "PersistLineProfileWindowPos exception " + ex.Message);
            }
        }

        private void ResetWindows_Click(object sender, RoutedEventArgs e)
        {
            ResetWindowPosAndSize("ROIStatsWindow");
            ResetWindowPosAndSize("LineProfileWindow");
            ResetWindowPosAndSize("ROIChartWindow ");
            ResetWindowPos("EditBleachWindow");
            ResetWindowPos("PowerRampWindow");
        }

        //assign the attribute value to the input node and document
        //if the attribute does not exist add it to the document
        private void SetAttribute(XmlNode node, XmlDocument doc, string attrName, string attValue)
        {
            XmlNode tempNode = node.Attributes[attrName];

            if (null == tempNode)
            {
                XmlAttribute attr = doc.CreateAttribute(attrName);

                attr.Value = attValue;

                node.Attributes.Append(attr);
            }
            else
            {
                node.Attributes[attrName].Value = attValue;
            }
        }

        #endregion Methods
    }
}