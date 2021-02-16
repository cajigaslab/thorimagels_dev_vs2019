namespace CaptureSetupDll.View
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Navigation;
    using System.Windows.Shapes;
    using System.Xml;

    using CaptureSetupDll.ViewModel;

    using RangeSliderWPF;

    using ThorLogging;

    using ThorSharedTypes;

    /// <summary>
    /// Interaction logic for ZControlView.xaml
    /// </summary>
    public partial class ZControlView : UserControl
    {
        #region Constructors

        public ZControlView()
        {
            InitializeComponent();
            Loaded += ZControlView_Loaded;
            Unloaded += ZControlView_Unloaded;
        }

        #endregion Constructors

        #region Methods

        private void ZControlView_Loaded(object sender, RoutedEventArgs e)
        {
            if (System.ComponentModel.DesignerProperties.GetIsInDesignMode(this) == true)  //design mode
                return;

            XmlDocument hardwareDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];
            XmlNodeList ndList = hardwareDoc.SelectNodes("/HardwareSettings/ZStage2");

            if (ndList.Count > 0)
            {
                string str = string.Empty;
                for (int i = 0; i < ((Dictionary<int, string>)MVMManager.Instance["ZControlViewModel", "LocationNames", (object)string.Empty]).Count; i++)
                {
                    if (XmlManager.GetAttribute(ndList[0], hardwareDoc, ((Dictionary<int, string>)MVMManager.Instance["ZControlViewModel", "LocationNames", (object)string.Empty])[i], ref str))
                    {
                        MVMManager.Instance["ZControlViewModel", "ZStage2LocationNames", i] = str;
                    }
                }
            }

            var appSettingsDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

            ndList = appSettingsDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/ZView");
            if (0 < ndList.Count)
            {
                string str = string.Empty;
                if (XmlManager.GetAttribute(ndList[0], appSettingsDoc, "enableContinuousZStackPreview", ref str))
                {
                    MVMManager.Instance["ZControlViewModel", "EnableContinuousZStackPreview"] = str == "1";
                }
            }
        }

        private void ZControlView_Unloaded(object sender, RoutedEventArgs e)
        {
            var appSettingsDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

            XmlNodeList ndList = appSettingsDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/ZView");

            if (ndList.Count > 0)
            {
                int val = (bool)MVMManager.Instance["ZControlViewModel", "EnableContinuousZStackPreview", (object)false] ? 1 : 0;
                XmlManager.SetAttribute(ndList[0], appSettingsDoc, "enableContinuousZStackPreview", val.ToString());

                MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS);
            }
        }

        #endregion Methods
    }
}