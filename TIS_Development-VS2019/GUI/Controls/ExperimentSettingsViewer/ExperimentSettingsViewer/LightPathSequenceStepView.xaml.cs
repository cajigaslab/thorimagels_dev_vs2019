namespace ExperimentSettingsViewer
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Threading.Tasks;
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

    /// <summary>
    /// Interaction logic for LightPathSequenceStep.xaml
    /// </summary>
    public partial class LightPathSequenceStepView : UserControl
    {
        #region Fields

        LightPathSequenceStep _lightPathSequenceStep = null;

        #endregion Fields

        #region Constructors

        public LightPathSequenceStepView(LightPathSequenceStep si)
        {
            InitializeComponent();
            Loaded += new RoutedEventHandler(LightPathSequenceStep_Loaded);
            _lightPathSequenceStep = si;
        }

        #endregion Constructors

        #region Methods

        [DllImport(".\\ResourceManager.dll", EntryPoint = "GetApplicationSettingsFilePathAndName", CharSet = CharSet.Unicode)]
        public static extern int GetApplicationSettingsFilePathAndName(StringBuilder sb, int length);

        [DllImport(".\\ResourceManager.dll", EntryPoint = "GetHardwareSettingsFilePathAndName", CharSet = CharSet.Unicode)]
        public static extern int GetHardwareSettingsFilePathAndName(StringBuilder sb, int length);

        public string GetApplicationSettingsFileString()
        {
            const int PATH_LENGTH = 261;
            StringBuilder sb = new StringBuilder(PATH_LENGTH);
            GetApplicationSettingsFilePathAndName(sb, PATH_LENGTH);

            return sb.ToString();
        }

        public bool GetAttribute(XmlNode node, XmlDocument doc, string attrName, ref string attrValue)
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

        public string GetHardwareSettingsFileString()
        {
            const int PATH_LENGTH = 261;
            StringBuilder sb = new StringBuilder(PATH_LENGTH);
            GetHardwareSettingsFilePathAndName(sb, PATH_LENGTH);

            return sb.ToString();
        }

        public Visibility GetVisibility(XmlDocument xmlDoc, string xPath, string attName)
        {
            XmlNodeList ndList = xmlDoc.SelectNodes(xPath);
            if (ndList.Count > 0)
            {
                string tmp = string.Empty;

                if (GetAttribute(ndList[0], xmlDoc, attName, ref tmp))
                {
                    return tmp.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                }
                else
                {
                    return Visibility.Collapsed;
                }
            }
            return Visibility.Collapsed;
        }

        void LightPathSequenceStep_Loaded(object sender, RoutedEventArgs e)
        {
            //retrieve the application settings complete path and file name
            string appSettings = GetApplicationSettingsFileString();

            XmlDocument appDoc = new XmlDocument();
            appDoc.Load(appSettings);

            //Use the visibitlity settings in application settings to setup the visibility on the controls
            //that can be hidden, such as the MCLS and Multiphoton laser
            string xPath = "/ApplicationSettings/DisplayOptions/CaptureSetup/ScannerView";
            gbPMTSettings.Visibility = GetVisibility(appDoc, xPath, "Visibility");

            xPath = "/ApplicationSettings/DisplayOptions/CaptureSetup/MultiLaserControlView";
            gbMCLSSettings.Visibility = GetVisibility(appDoc, xPath, "Visibility");

            xPath = "/ApplicationSettings/DisplayOptions/CaptureSetup/MultiphotonView";
            gbMultiphotonSettings.Visibility = GetVisibility(appDoc, xPath, "Visibility");

            xPath = "/ApplicationSettings/DisplayOptions/CaptureSetup/PinholeView";
            gbPinholeSettings.Visibility = GetVisibility(appDoc, xPath, "Visibility");

            xPath = "/ApplicationSettings/DisplayOptions/CaptureSetup/PowerView";
            if (Visibility.Visible == GetVisibility(appDoc, xPath, "Visibility"))
            {

                xPath = "/ApplicationSettings/DisplayOptions/CaptureSetup/Pockels1";
                gbPockels0Settings.Visibility = GetVisibility(appDoc, xPath, "Visibility");

                xPath = "/ApplicationSettings/DisplayOptions/CaptureSetup/Pockels2";
                gbPockels1Settings.Visibility = GetVisibility(appDoc, xPath, "Visibility");

                xPath = "/ApplicationSettings/DisplayOptions/CaptureSetup/Pockels3";
                gbPockels2Settings.Visibility = GetVisibility(appDoc, xPath, "Visibility");

                xPath = "/ApplicationSettings/DisplayOptions/CaptureSetup/Pockels4";
                gbPockels3Settings.Visibility = GetVisibility(appDoc, xPath, "Visibility");
            }
            else
            {
                gbPockels0Settings.Visibility = Visibility.Collapsed;
                gbPockels1Settings.Visibility = Visibility.Collapsed;
                gbPockels2Settings.Visibility = Visibility.Collapsed;
                gbPockels3Settings.Visibility = Visibility.Collapsed;
            }

            xPath = "/ApplicationSettings/DisplayOptions/CaptureSetup/LightPathView";
            spGG.Visibility = GetVisibility(appDoc, xPath, "GGLightPathVisibility");
            spGR.Visibility = GetVisibility(appDoc, xPath, "GRLightPathVisibility");
            spCAM.Visibility = GetVisibility(appDoc, xPath, "CameraLightPathVisibility");

            if ((Visibility.Visible == GetVisibility(appDoc, xPath, "Visibility")) && (Visibility.Visible == spGG.Visibility || Visibility.Visible == spGR.Visibility || Visibility.Visible == spCAM.Visibility))
            {
                gbLightPathSettings.Visibility = Visibility.Visible;
            }
            else
            {
                gbLightPathSettings.Visibility = Visibility.Collapsed;
            }

            lblLightPathSequenceStepName.Content = _lightPathSequenceStep.Name;
            //Load Channel Enable form LSM
            bool chanEnable0 = false, chanEnable1 = false, chanEnable2 = false, chanEnable3 = false;
            _lightPathSequenceStep.GetLightPathSequenceStepLSMChannel(ref chanEnable0, ref chanEnable1, ref chanEnable2, ref chanEnable3);
            btChanA.IsEnabled = chanEnable0;
            btChanB.IsEnabled = chanEnable1;
            btChanC.IsEnabled = chanEnable2;
            btChanD.IsEnabled = chanEnable3;

            //Load PMT Settings
            double pmt1Gain = 0, pmt2Gain = 0, pmt3Gain = 0, pmt4Gain = 0;
            _lightPathSequenceStep.GetLightPathSequenceStepPMT(ref pmt1Gain, ref pmt2Gain, ref pmt3Gain, ref pmt4Gain);
            tbGainA.Text = pmt1Gain.ToString();
            tbGainB.Text = pmt2Gain.ToString();
            tbGainC.Text = pmt3Gain.ToString();
            tbGainD.Text = pmt4Gain.ToString();
            tbGainA.IsEnabled = chanEnable0;
            tbGainB.IsEnabled = chanEnable1;
            tbGainC.IsEnabled = chanEnable2;
            tbGainD.IsEnabled = chanEnable3;

            //Load MCLS Laser settings
            int laser1Enable = 0, laser2Enable = 0, laser3Enable = 0, laser4Enable = 0, laserAllAnalog = 0, laserAllTTL = 0, laser1Wavelength = 0, laser2Wavelength = 0, laser3Wavelength = 0, laser4Wavelength = 0;
            double laser1Power = 0.0, laser1Percent = 0.0, laser2Power = 0.0, laser2Percent = 0.0, laser3Power = 0.0, laser3Percent = 0.0, laser4Power = 0.0, laser4Percent = 0.0;
            _lightPathSequenceStep.GetLightPathSequenceStepMCLS(ref laser1Enable, ref laser1Power, ref laser1Percent, ref laser2Enable, ref laser2Power, ref laser2Percent,
                ref laser3Enable, ref laser3Power, ref laser3Percent, ref laser4Enable, ref laser4Power, ref laser4Percent, ref laserAllAnalog, ref laserAllTTL, ref laser1Wavelength,
                ref laser2Wavelength, ref laser3Wavelength, ref laser4Wavelength);
            cbLaser1Enable.IsChecked = (1 == laser1Enable) ? true : false;
            tbLaserPowerPercent1.Text = laser1Percent.ToString();
            tbLaserPowerPercent1.IsEnabled = (1 == laser1Enable) ? true : false;
            cbLaser2Enable.IsChecked = (1 == laser2Enable) ? true : false;
            tbLaserPowerPercent2.Text = laser2Percent.ToString();
            tbLaserPowerPercent2.IsEnabled = (1 == laser2Enable) ? true : false;
            cbLaser3Enable.IsChecked = (1 == laser3Enable) ? true : false;
            tbLaserPowerPercent3.Text = laser3Percent.ToString();
            tbLaserPowerPercent3.IsEnabled = (1 == laser3Enable) ? true : false;
            cbLaser4Enable.IsChecked = (1 == laser4Enable) ? true : false;
            tbLaserPowerPercent4.Text = laser4Percent.ToString();
            tbLaserPowerPercent4.IsEnabled = (1 == laser4Enable) ? true : false;

            //Populate the label for each laser based off of whether the wavelength can be queried (MCLS vs Toptica)
            if (laser1Wavelength == 0)
            {
                cbLaser1Enable.Content = "Laser1";
            }
            else
            {
                cbLaser1Enable.Content = laser1Wavelength.ToString() + " nm";
            }
            if (laser2Wavelength == 0)
            {
                cbLaser2Enable.Content = "Laser2";
            }
            else
            {
                cbLaser2Enable.Content = laser2Wavelength.ToString() + " nm";
            }
            if (laser3Wavelength == 0)
            {
                cbLaser3Enable.Content = "Laser3";
            }
            else
            {
                cbLaser3Enable.Content = laser3Wavelength.ToString() + " nm";
            }
            if (laser4Wavelength == 0)
            {
                cbLaser4Enable.Content = "Laser4";
            }
            else
            {
                cbLaser4Enable.Content = laser4Wavelength.ToString() + " nm";
            }

            //Only make TTL and Analog checkboxes visible for Toptica, read values from light path
            if (laser1Wavelength == 0 && laser2Wavelength == 0 && laser3Wavelength == 0 && laser4Wavelength == 0)
            {
                cbLaserAllAnalog.Visibility = Visibility.Collapsed;
                cbLaserAllTTL.Visibility = Visibility.Collapsed;
            }
            else
            {
                cbLaserAllAnalog.Visibility = Visibility.Visible;
                cbLaserAllTTL.Visibility = Visibility.Visible;
                cbLaserAllTTL.IsChecked = (1 == laserAllTTL) ? true : false;
                cbLaserAllAnalog.IsChecked = (1 == laserAllAnalog) ? true : false;
            }

            //Load Multiphoton Laser Settings
            int laserPosition = 0;
            _lightPathSequenceStep.GetLightPathSequenceStepMultiphoton(ref laserPosition);
            tbMultiphotonPosition.Text = laserPosition.ToString();

            //Load PinholeWheel Settings
            int pinholePosition = 0;
            double pinholeMicrometers = 0.0, pinholeADUs = 0.0;
            _lightPathSequenceStep.GetLightPathSequenceStepPinhole(ref pinholePosition, ref pinholeMicrometers, ref pinholeADUs);
            tbPinholeSize.Text = pinholeMicrometers.ToString();
            lblADUs.Content = pinholeADUs.ToString() + "ADUs";

            //Load Pockels Settings
            const int MAX_POCKELS = 4;
            double[] pockelsPowerLevel = new double[MAX_POCKELS];
            int[] powerType = new int[MAX_POCKELS];
            string[] powerRampName = new string[MAX_POCKELS];
            _lightPathSequenceStep.GetPockels(ref pockelsPowerLevel, ref powerType, ref powerRampName);
            for (int i = 0; i < pockelsPowerLevel.Length; i++)
            {
                switch (i)
                {
                    case 0:
                        tbP0level.Text = pockelsPowerLevel[i].ToString();
                        tbP0CustomRampingName.Text = powerRampName[i];
                        spPowerLevel0.Visibility = (0 == powerType[i]) ? Visibility.Visible : Visibility.Collapsed;
                        spCustomPower0.Visibility = (1 == powerType[i]) ? Visibility.Visible : Visibility.Collapsed;
                        break;
                    case 1:
                        tbP1level.Text = pockelsPowerLevel[i].ToString();
                        tbP1CustomRampingName.Text = powerRampName[i];
                        spPowerLevel1.Visibility = (0 == powerType[i]) ? Visibility.Visible : Visibility.Collapsed;
                        spCustomPower1.Visibility = (1 == powerType[i]) ? Visibility.Visible : Visibility.Collapsed;
                        break;
                    case 2:
                        tbP2level.Text = pockelsPowerLevel[i].ToString();
                        tbP2CustomRampingName.Text = powerRampName[i];
                        spPowerLevel2.Visibility = (0 == powerType[i]) ? Visibility.Visible : Visibility.Collapsed;
                        spCustomPower2.Visibility = (1 == powerType[i]) ? Visibility.Visible : Visibility.Collapsed;
                        break;
                    case 3:
                        tbP3level.Text = pockelsPowerLevel[i].ToString();
                        tbP3CustomRampingName.Text = powerRampName[i];
                        spPowerLevel3.Visibility = (0 == powerType[i]) ? Visibility.Visible : Visibility.Collapsed;
                        spCustomPower3.Visibility = (1 == powerType[i]) ? Visibility.Visible : Visibility.Collapsed;
                        break;
                }
            }

            //Load Light Path Settings
            int GGEnable = 0, GREnable = 0, CamEnable = 0;
            _lightPathSequenceStep.GetLightPath(ref GGEnable, ref GREnable, ref CamEnable);
            lblGGMirrorPos.Content = (1 == GGEnable) ? "IN" : "OUT";
            lblGRMirrorPos.Content = (1 == GREnable) ? "IN" : "OUT";
            lblCamMirrorPos.Content = (1 == CamEnable) ? "CAM" : "PMT";
        }

        #endregion Methods
    }
}