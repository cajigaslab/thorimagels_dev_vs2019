namespace SequentialControl
{
    using System.Collections.ObjectModel;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Media;
    using System.Xml;

    using ThorSharedTypes;

    /// <summary>
    /// Interaction logic for SequenceStepView.xaml
    /// </summary>
    public partial class SequenceStepView : UserControl
    {
        #region Fields

        const int MAX_CHANNELS = 4;

        SequenceStep _sequenceStep = null;

        #endregion Fields

        #region Constructors

        public SequenceStepView(SequenceStep si)
        {
            InitializeComponent();
            Loaded += new RoutedEventHandler(SequenceStep_Loaded);
            _sequenceStep = si;
        }

        #endregion Constructors

        #region Methods

        public Visibility GetVisibility(XmlDocument xmlDoc, string xPath, string attName)
        {
            XmlNodeList ndList = xmlDoc.SelectNodes(xPath);
            if (ndList.Count > 0)
            {
                string tmp = string.Empty;

                if (XmlManager.GetAttribute(ndList[0], xmlDoc, attName, ref tmp))
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

        void SequenceStep_Loaded(object sender, RoutedEventArgs e)
        {
            //retrieve the application settings complete path and file name
            XmlDocument appDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

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

            xPath = "/ApplicationSettings/DisplayOptions/CaptureSetup/DigitalSwitchesView";
            gbDigitalSwitchesSettings.Visibility = GetVisibility(appDoc, xPath, "Visibility");

            xPath = "/ApplicationSettings/DisplayOptions/CaptureSetup/CameraView";
            gbCameraSettings.Visibility = GetVisibility(appDoc, xPath, "Visibility");

            xPath = "/ApplicationSettings/DisplayOptions/CaptureSetup/EpiturretControlView";
            gbEpiTurretSettings.Visibility = GetVisibility(appDoc, xPath, "Visibility");

            string tmp = string.Empty;
            XmlNodeList digitalNdList = appDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/DigitalSwitchesView");
            if (Visibility.Visible == gbDigitalSwitchesSettings.Visibility && 0 < digitalNdList.Count)
            {
                XmlManager.GetAttribute(digitalNdList[0], appDoc, "switchName1", ref tmp);
                lblDigitalSwitch1.Content = tmp;
                XmlManager.GetAttribute(digitalNdList[0], appDoc, "switchName2", ref tmp);
                lblDigitalSwitch2.Content = tmp;
                XmlManager.GetAttribute(digitalNdList[0], appDoc, "switchName3", ref tmp);
                lblDigitalSwitch3.Content = tmp;
                XmlManager.GetAttribute(digitalNdList[0], appDoc, "switchName4", ref tmp);
                lblDigitalSwitch4.Content = tmp;
                XmlManager.GetAttribute(digitalNdList[0], appDoc, "switchName5", ref tmp);
                lblDigitalSwitch5.Content = tmp;
                XmlManager.GetAttribute(digitalNdList[0], appDoc, "switchName6", ref tmp);
                lblDigitalSwitch6.Content = tmp;
                XmlManager.GetAttribute(digitalNdList[0], appDoc, "switchName7", ref tmp);
                lblDigitalSwitch7.Content = tmp;
                XmlManager.GetAttribute(digitalNdList[0], appDoc, "switchName8", ref tmp);
                lblDigitalSwitch8.Content = tmp;
            }

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

            lblSequenceStepName.Content = _sequenceStep.Name;
            //Load Channel Enable form LSM
            bool chanEnable0 = false, chanEnable1 = false, chanEnable2 = false, chanEnable3 = false;
            _sequenceStep.GetSequenceStepLSMChannel(ref chanEnable0, ref chanEnable1, ref chanEnable2, ref chanEnable3);
            btChanA.Visibility = tbGainA.Visibility = chanAColorEnable.Visibility = chanEnable0 ? Visibility.Visible : Visibility.Collapsed;
            btChanB.Visibility = tbGainB.Visibility = chanBColorEnable.Visibility = chanEnable1 ? Visibility.Visible : Visibility.Collapsed;
            btChanC.Visibility = tbGainC.Visibility = chanCColorEnable.Visibility = chanEnable2 ? Visibility.Visible : Visibility.Collapsed;
            btChanD.Visibility = tbGainD.Visibility = chanDColorEnable.Visibility = chanEnable3 ? Visibility.Visible : Visibility.Collapsed;

            //:TODO: Need to add a tag to keep track of the selection of the channel in the ViewControl.
            //btChanA.IsEnabled = chanEnable0;
            //btChanB.IsEnabled = chanEnable1;
            //btChanC.IsEnabled = chanEnable2;
            //btChanD.IsEnabled = chanEnable3;

            //Load Channel Color settings
            Color[] channelColors = new Color[MAX_CHANNELS];
            string[] channelNames = new string[MAX_CHANNELS];
            _sequenceStep.GetSequenceStepChannelColors(ref channelColors, ref channelNames);
            for (int i = 0; i < channelColors.Length; i++)
            {
                if (null != channelColors[i] && null != channelNames[i])
                {
                    switch (i)
                    {
                        case 0:
                            chanAColorBorder.Background = new SolidColorBrush(channelColors[i]);
                            chanAColorLabel.Content = channelNames[i];
                            chanAColorLabel.FontSize = (1 == channelNames[i].Length) ? 30 : (2 == channelNames[i].Length) ? 25 : 23;
                            break;
                        case 1:
                            chanBColorBorder.Background = new SolidColorBrush(channelColors[i]);
                            chanBColorLabel.Content = channelNames[i];
                            chanBColorLabel.FontSize = (1 == channelNames[i].Length) ? 30 : (2 == channelNames[i].Length) ? 25 : 23;
                            break;
                        case 2:
                            chanCColorBorder.Background = new SolidColorBrush(channelColors[i]);
                            chanCColorLabel.Content = channelNames[i];
                            chanCColorLabel.FontSize = (1 == channelNames[i].Length) ? 30 : (2 == channelNames[i].Length) ? 25 : 23;
                            break;
                        case 3:
                            chanDColorBorder.Background = new SolidColorBrush(channelColors[i]);
                            chanDColorLabel.Content = channelNames[i];
                            chanDColorLabel.FontSize = (1 == channelNames[i].Length) ? 30 : (2 == channelNames[i].Length) ? 25 : 23;
                            break;
                    }
                }
            }

            //Load PMT Settings
            double pmt1Gain = 0, pmt2Gain = 0, pmt3Gain = 0, pmt4Gain = 0;
            _sequenceStep.GetSequenceStepPMT(ref pmt1Gain, ref pmt2Gain, ref pmt3Gain, ref pmt4Gain);
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
            int laserTTL = 0, laserAnalog = 0;
            int laser1Wavelength = 0, laser2Wavelength = 0, laser3Wavelength = 0, laser4Wavelength = 0;
            _sequenceStep.GetSequenceStepMCLS(ref mainLaserSelection, ref laser1Enable, ref laser1Power, ref laser1Percent, ref laser2Enable, ref laser2Power, ref laser2Percent,
                ref laser3Enable, ref laser3Power, ref laser3Percent, ref laser4Enable, ref laser4Power, ref laser4Percent, ref laserTTL, ref laserAnalog, ref laser1Wavelength, ref laser2Wavelength, ref laser3Wavelength,
                ref laser4Wavelength);

            //Load Digital Switches settings
            int enable = 0;
            int[] switchPositions = new int[(int)Constants.MAX_SWITCHES];
            _sequenceStep.GetSequenceStepDigitalIO(ref enable, ref switchPositions);
            chkBoxDigitalSwitches.IsChecked = 1 == enable;
            btnDigitalSwitch1.IsChecked = 1 == switchPositions[0];
            btnDigitalSwitch2.IsChecked = 1 == switchPositions[1];
            btnDigitalSwitch3.IsChecked = 1 == switchPositions[2];
            btnDigitalSwitch4.IsChecked = 1 == switchPositions[3];
            btnDigitalSwitch5.IsChecked = 1 == switchPositions[4];
            btnDigitalSwitch6.IsChecked = 1 == switchPositions[5];
            btnDigitalSwitch7.IsChecked = 1 == switchPositions[6];
            btnDigitalSwitch8.IsChecked = 1 == switchPositions[7];

            //Check if wavelength has been populated or not
            int wavelengthVisibility = 0;
            if (laser1Wavelength == 0 || laser2Wavelength == 0 || laser3Wavelength == 0 || laser4Wavelength == 0)
            {
                wavelengthVisibility = 0;
            }
            else
            {
                wavelengthVisibility = 1;
            }

            //If wavelength has been populated display the ttl/analog labels and checkboxes
            if (wavelengthVisibility == 0)
            {
                ttlLabel.Visibility = Visibility.Collapsed;
                analogLabel.Visibility = Visibility.Collapsed;
                cbLaserAnalog.Visibility = Visibility.Collapsed;
                cbLaserTTL.Visibility = Visibility.Collapsed;
            }
            else
            {
                ttlLabel.Visibility = Visibility.Visible;
                analogLabel.Visibility = Visibility.Visible;
                cbLaserAnalog.Visibility = Visibility.Visible;
                cbLaserTTL.Visibility = Visibility.Visible;
            }

            //Check whether TTL/Analog modes are enabled
            cbLaserAnalog.IsChecked = (1 == laserAnalog) ? true : false;
            cbLaserTTL.IsChecked = (1 == laserTTL) ? true : false;

            //If wavelength has been populated display the wavelength as the label
            cbLaser1Enable.IsChecked = (1 == laser1Enable) ? true : false;
            if (wavelengthVisibility == 0)
            {
                cbLaser1Enable.Content = "Laser1 Enable";
            }
            else
            {
                cbLaser1Enable.Content = laser1Wavelength.ToString() + " nm";
            }

            tbLaserPowerPercent1.Text = laser1Percent.ToString();
            tbLaserPowerPercent1.IsEnabled = (1 == laser1Enable) ? true : false;

            //If wavelength has been populated display the wavelength as the label
            cbLaser2Enable.IsChecked = (1 == laser2Enable) ? true : false;
            if (wavelengthVisibility == 0)
            {
                cbLaser2Enable.Content = "Laser2 Enable";
            }
            else
            {
                cbLaser2Enable.Content = laser2Wavelength.ToString() + " nm";
            }
            tbLaserPowerPercent2.Text = laser2Percent.ToString();
            tbLaserPowerPercent2.IsEnabled = (1 == laser2Enable) ? true : false;

            //If wavelength has been populated display the wavelength as the label
            cbLaser3Enable.IsChecked = (1 == laser3Enable) ? true : false;
            if (wavelengthVisibility == 0)
            {
                cbLaser3Enable.Content = "Laser3 Enable";
            }
            else
            {
                cbLaser3Enable.Content = laser3Wavelength.ToString() + " nm";
            }
            tbLaserPowerPercent3.Text = laser3Percent.ToString();
            tbLaserPowerPercent3.IsEnabled = (1 == laser3Enable) ? true : false;

            //If wavelength has been populated display the wavelength as the label
            cbLaser4Enable.IsChecked = (1 == laser4Enable) ? true : false;
            if (wavelengthVisibility == 0)
            {
                cbLaser4Enable.Content = "Laser4 Enable";
            }
            else
            {
                cbLaser4Enable.Content = laser4Wavelength.ToString() + " nm";
            }
            tbLaserPowerPercent4.Text = laser4Percent.ToString();
            tbLaserPowerPercent4.IsEnabled = (1 == laser4Enable) ? true : false;

            //Load Multiphoton Laser Settings
            int laserPosition = 0;
            _sequenceStep.GetSequenceStepMultiphoton(ref laserPosition);
            tbMultiphotonPosition.Text = laserPosition.ToString();

            //Load PinholeWheel Settings
            int pinholePosition = 0;
            double pinholeMicrometers = 0.0, pinholeADUs = 0.0;
            _sequenceStep.GetSequenceStepPinhole(ref pinholePosition, ref pinholeMicrometers, ref pinholeADUs);
            tbPinholeSize.Text = pinholeMicrometers.ToString();
            lblADUs.Content = pinholeADUs.ToString() + "ADUs";

            //Load Pockels Settings
            const int MAX_POCKELS = 4;
            double[] pockelsPowerLevel = new double[MAX_POCKELS];
            int[] powerType = new int[MAX_POCKELS];
            string[] powerRampName = new string[MAX_POCKELS];
            _sequenceStep.GetPockels(ref pockelsPowerLevel, ref powerType, ref powerRampName);
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
            _sequenceStep.GetLightPath(ref M1Enable, ref M2Enable, ref M3Enable);
            if ((int)ScopeType.INVERTED == _sequenceStep.SequenceStepScopeType)
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

            //Load Camera Settings
            int channels = 0;
            double exposure = 0;
            _sequenceStep.GetCameraSettings(ref channels, ref exposure);
            tbExposure.Text = exposure.ToString();

            //Load Epi Turret Settings
            int epiPos = 0;
            string epiPosName = string.Empty;
            _sequenceStep.GetSequenceEpiTurretPosition(ref epiPos, ref epiPosName);
            tbEpiTurretPosition.Text = epiPos.ToString();
            if (epiPos > 0 && epiPos < 7)
            {
                switch (epiPos)
                {
                    case 1:
                        btnEpiPosition1.Background = new SolidColorBrush(Colors.SteelBlue);
                        btnEpiPosition1.BorderBrush = new SolidColorBrush(Colors.LimeGreen);
                        break;
                    case 2:
                        btnEpiPosition2.Background = new SolidColorBrush(Colors.SteelBlue);
                        btnEpiPosition2.BorderBrush = new SolidColorBrush(Colors.LimeGreen);
                        break;
                    case 3:
                        btnEpiPosition3.Background = new SolidColorBrush(Colors.SteelBlue);
                        btnEpiPosition3.BorderBrush = new SolidColorBrush(Colors.LimeGreen);
                        break;
                    case 4:
                        btnEpiPosition4.Background = new SolidColorBrush(Colors.SteelBlue);
                        btnEpiPosition4.BorderBrush = new SolidColorBrush(Colors.LimeGreen);
                        break;
                    case 5:
                        btnEpiPosition5.Background = new SolidColorBrush(Colors.SteelBlue);
                        btnEpiPosition5.BorderBrush = new SolidColorBrush(Colors.LimeGreen);
                        break;
                    case 6:
                        btnEpiPosition6.Background = new SolidColorBrush(Colors.SteelBlue);
                        btnEpiPosition6.BorderBrush = new SolidColorBrush(Colors.LimeGreen);
                        break;
                }
            }
        }

        #endregion Methods
    }
}