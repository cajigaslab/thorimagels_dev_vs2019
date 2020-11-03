namespace CaptureSetupDll.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Linq;
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

    using ThorSharedTypes;

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
            XmlDocument appDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

            //Use the visibitlity settings in application settings to setup the visibility on the controls
            //that can be hidden, such as the MCLS and Multiphoton laser
            string xPath = "/ApplicationSettings/DisplayOptions/CaptureSetup/ScannerView";
            gbPMTSettings.Visibility = GetVisibility(appDoc, xPath, "Visibility");

            xPath = "/ApplicationSettings/DisplayOptions/CaptureSetup/MCLSView";
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

            gbPockels0Settings.Header = ((ObservableCollection<StringPC>)MVMManager.Instance["PowerControlViewModel", "PowerControlName", (object)0])[0].Value;
            gbPockels1Settings.Header = ((ObservableCollection<StringPC>)MVMManager.Instance["PowerControlViewModel", "PowerControlName", (object)0])[1].Value;
            gbPockels2Settings.Header = ((ObservableCollection<StringPC>)MVMManager.Instance["PowerControlViewModel", "PowerControlName", (object)0])[2].Value;
            gbPockels3Settings.Header = ((ObservableCollection<StringPC>)MVMManager.Instance["PowerControlViewModel", "PowerControlName", (object)0])[3].Value;

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
            int pmt1Gain = 0, pmt2Gain = 0, pmt3Gain = 0, pmt4Gain = 0;
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
            int mainLaserSelection = -1;
            int laser1Enable = 0, laser2Enable = 0, laser3Enable = 0, laser4Enable = 0;
            double laser1Power = 0.0, laser1Percent = 0.0;
            double laser2Power = 0.0, laser2Percent = 0.0;
            double laser3Power = 0.0, laser3Percent = 0.0;
            double laser4Power = 0.0, laser4Percent = 0.0;
            _lightPathSequenceStep.GetLightPathSequenceStepMCLS(ref mainLaserSelection, ref laser1Enable, ref laser1Power, ref laser1Percent, ref laser2Enable, ref laser2Power, ref laser2Percent,
                ref laser3Enable, ref laser3Power, ref laser3Percent, ref laser4Enable, ref laser4Power, ref laser4Percent);
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
            int M1Enable = 0, M2Enable = 0, M3Enable = 0;
            _lightPathSequenceStep.GetLightPath(ref M1Enable, ref M2Enable, ref M3Enable);
            if ((int)ScopeType.INVERTED == _lightPathSequenceStep.SequenceStepScopeType)
            {
                lblMirror1Mark.Content = "Left";
                lblMirror2Mark.Content = "Center";
                lblMirror3Mark.Content = "Right";
                lblMirror1Pos.Content = (1 == M1Enable) ? "IN" : "";
                lblMirror2Pos.Content = (1 == M2Enable) ? "IN" : "";
                lblMirror3Pos.Content = (1 == M3Enable) ? "IN" : "";
            }
            else
            {
                lblMirror1Mark.Content = "GG";
                lblMirror2Mark.Content = "GR";
                lblMirror3Mark.Content = "CAM";
                lblMirror1Pos.Content = (1 == M1Enable) ? "IN" : "OUT";
                lblMirror2Pos.Content = (1 == M2Enable) ? "IN" : "OUT";
                lblMirror3Pos.Content = (1 == M3Enable) ? "CAM" : "PMT";
            }
        }

        #endregion Methods
    }
}