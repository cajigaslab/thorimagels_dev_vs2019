namespace CaptureSetupDll.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.ComponentModel;
    using System.Globalization;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Threading;
    using System.Xml;

    using ThorLogging;

    using ThorSharedTypes;

    public partial class CaptureSetupViewModel : ViewModelBase
    {
        #region Fields

        private const int MAX_CHANNEL_STEP_COUNT = 4;

        static Dictionary<string, LightPathBtns> LightPathCmds = new Dictionary<string, LightPathBtns>()
        {
        {"GG_IN", LightPathBtns.GG_IN},
        {"GG_OUT", LightPathBtns.GG_OUT},
        {"GG_Flip", LightPathBtns.GG_Flip},
        {"GR_IN", LightPathBtns.GR_IN},
        {"GR_OUT", LightPathBtns.GR_OUT},
        {"GR_Flip", LightPathBtns.GR_Flip},
        {"CAM_PMT", LightPathBtns.CAM_PMT},
        {"CAM_CAM", LightPathBtns.CAM_CAM},
        {"CAM_Flip", LightPathBtns.CAM_Flip},
        {"Left", LightPathBtns.Left},
        {"Center", LightPathBtns.Center},
        {"Right", LightPathBtns.Right},
        {"NDD_IN", LightPathBtns.NDD_IN},
        {"NDD_OUT", LightPathBtns.NDD_OUT},
        };

        private string _camLightPathKey;
        private string _camLightPathModifier;
        private ObservableCollection<LightPathSequenceStep> _captureSequence = new ObservableCollection<LightPathSequenceStep>();
        private LightPathSequenceStep _draggedLightPathSequenceStep;
        private int _enableSequentialCaptuer = 0;
        private string _ggLightPathKey;
        private string _ggLightPathModifier;
        private string _grLightPathKey;
        private string _grLightPathModifier;
        private bool _isDraggingLightPathSequenceStep = false;
        private ICommand _lightPathListAddCommand;
        private ObservableCollection<LightPathSequenceStep> _lightPaths = new ObservableCollection<LightPathSequenceStep>();
        private RelayCommandWithParam _lightPathSwitch;
        private int _previousCaptureLightPathSelectedLine = 0;
        private int _previousCaptureSequenceSelectedLine = 0;

        #endregion Fields

        #region Enumerations

        enum LightPathBtns
        {
            GG_IN = 0,
            GG_OUT = 1,
            GG_Flip = 2,
            GR_IN = 3,
            GR_OUT = 4,
            GR_Flip = 5,
            CAM_PMT = 6,
            CAM_CAM = 7,
            CAM_Flip = 8,
            Left = 9,
            Center = 10,
            Right = 11,
            NDD_IN = 12,
            NDD_OUT = 13
        }

        #endregion Enumerations

        #region Properties

        public Visibility CameraLightPathVisibility
        {
            get
            {
                return GetVisibility("/ApplicationSettings/DisplayOptions/CaptureSetup/LightPathView", "CameraLightPathVisibility");
            }
        }

        public string CamLightPathKey
        {
            get { return _camLightPathKey; }
            set { _camLightPathKey = value; OnPropertyChanged("CamLightPathKey"); }
        }

        public string CamLightPathModifier
        {
            get { return _camLightPathModifier; }
            set { _camLightPathModifier = value; OnPropertyChanged("CamLightPathModifier"); }
        }

        public ObservableCollection<LightPathSequenceStep> CollectionCaptureSequence
        {
            get
            {
                return _captureSequence;
            }

            set
            {
                _captureSequence = value;
                OnPropertyChanged("CollectionCaptureSequence");
            }
        }

        public ObservableCollection<LightPathSequenceStep> CollectionLightPaths
        {
            get
            {
                return _lightPaths;
            }

            set
            {
                _lightPaths = value;
                OnPropertyChanged("CollectionLightPaths");
            }
        }

        public string DisplayOffNDD
        {
            get
            {
                if (1 == this.PositionNDD)
                {
                    return @"/CaptureSetupModule;component/Icons/ToggleOff_up.png";
                }
                else
                {
                    return @"/CaptureSetupModule;component/Icons/ToggleOff_down.png";
                }
            }
        }

        public string DisplayOnNDD
        {
            get
            {
                if (1 == this.PositionNDD)
                {
                    return @"/CaptureSetupModule;component/Icons/ToggleOn_down.png";
                }
                else
                {
                    return @"/CaptureSetupModule;component/Icons/ToggleOn_up.png";
                }
            }
        }

        public int EnableSequentialCapture
        {
            get
            {
                return _enableSequentialCaptuer;
            }
            set
            {
                _enableSequentialCaptuer = value;
                OnPropertyChanged("EnableSequentialCapture");
            }
        }

        public string GGLightPathKey
        {
            get { return _ggLightPathKey; }
            set { _ggLightPathKey = value; OnPropertyChanged("GGLightPathKey"); }
        }

        public string GGLightPathModifier
        {
            get { return _ggLightPathModifier; }
            set { _ggLightPathModifier = value; OnPropertyChanged("GGLightPathModifier"); }
        }

        public Visibility GGLightPathVisibility
        {
            get
            {
                return GetVisibility("/ApplicationSettings/DisplayOptions/CaptureSetup/LightPathView", "GGLightPathVisibility");
            }
        }

        public string GRLightPathKey
        {
            get { return _grLightPathKey; }
            set { _grLightPathKey = value; OnPropertyChanged("GRLightPathKey"); }
        }

        public string GRLightPathModifier
        {
            get { return _grLightPathModifier; }
            set { _grLightPathModifier = value; OnPropertyChanged("GRLightPathModifier"); }
        }

        public Visibility GRLightPathVisibility
        {
            get
            {
                return GetVisibility("/ApplicationSettings/DisplayOptions/CaptureSetup/LightPathView", "GRLightPathVisibility");
            }
        }

        public int InvertedLightPathPos
        {
            get
            {
                return this._captureSetup.InvertedLightPathPos;
            }
            set
            {
                this._captureSetup.InvertedLightPathPos = value;
            }
        }

        public string InvertedLpCenterDisplay
        {
            get
            {
                if (this.InvertedLpCenterEnable)
                {
                    return @"/CaptureSetupModule;component/Icons/ToggleOn_down.png";
                }
                else
                {
                    return @"/CaptureSetupModule;component/Icons/ToggleOn_up.png";
                }
            }
        }

        public bool InvertedLpCenterEnable
        {
            get
            {
                return _captureSetup.InvertedLpCenterEnable;
            }
            set
            {
                _captureSetup.InvertedLpCenterEnable = value;
                OnPropertyChange("InvertedLpLeftDisplay");
                OnPropertyChange("InvertedLpCenterDisplay");
                OnPropertyChange("InvertedLpRightDisplay");
            }
        }

        public string InvertedLpLeftDisplay
        {
            get
            {
                if (this.InvertedLpLeftEnable)
                {
                    return @"/CaptureSetupModule;component/Icons/ToggleOn_down.png";
                }
                else
                {
                    return @"/CaptureSetupModule;component/Icons/ToggleOn_up.png";
                }
            }
        }

        public bool InvertedLpLeftEnable
        {
            get
            {
                return _captureSetup.InvertedLpLeftEnable;
            }
            set
            {
                _captureSetup.InvertedLpLeftEnable = value;
                OnPropertyChange("InvertedLpLeftDisplay");
                OnPropertyChange("InvertedLpCenterDisplay");
                OnPropertyChange("InvertedLpRightDisplay");
            }
        }

        public string InvertedLpRightDisplay
        {
            get
            {
                if (this.InvertedLpRightEnable)
                {
                    return @"/CaptureSetupModule;component/Icons/ToggleOn_down.png";
                }
                else
                {
                    return @"/CaptureSetupModule;component/Icons/ToggleOn_up.png";
                }
            }
        }

        public bool InvertedLpRightEnable
        {
            get
            {
                return _captureSetup.InvertedLpRightEnable;
            }
            set
            {
                _captureSetup.InvertedLpRightEnable = value;
                OnPropertyChange("InvertedLpLeftDisplay");
                OnPropertyChange("InvertedLpCenterDisplay");
                OnPropertyChange("InvertedLpRightDisplay");
            }
        }

        public Visibility IsInvertedScope
        {
            get
            {
                return ((int)ScopeType.INVERTED == _captureSetup.LightPathScopeType) ? Visibility.Visible : Visibility.Collapsed;
            }
        }

        public bool IsNDDAvailable
        {
            get
            {
                Visibility NDDVisibility = GetVisibility("/ApplicationSettings/DisplayOptions/CaptureSetup/LightPathView", "NDDLightPathVisibility");
                return (Visibility.Visible == NDDVisibility) ? _captureSetup.IsNDDAvailable : false;
            }
        }

        public Visibility IsTabletModeEnabled
        {
            get
            {
                return (ResourceManagerCS.Instance.TabletModeEnabled) ? Visibility.Collapsed : Visibility.Visible;
            }
        }

        public Visibility IsUprightScope
        {
            get
            {
                return ((int)ScopeType.UPRIGHT == _captureSetup.LightPathScopeType) ? Visibility.Visible : Visibility.Collapsed;
            }
        }

        public string LabelNDD
        {
            get
            {
                return GetValueString("/ApplicationSettings/DisplayOptions/CaptureSetup/InvertedLightPathView", "NddNameChange");
            }
        }

        public string LightPathCamDisplayOff
        {
            get
            {
                if (1 == this.LightPathCamEnable)
                {
                    return @"/CaptureSetupModule;component/Icons/ToggleCam_down.png";
                }
                else
                {
                    return @"/CaptureSetupModule;component/Icons/ToggleCam_up.png";
                }
            }
        }

        public string LightPathCamDisplayOn
        {
            get
            {
                if (1 == this.LightPathCamEnable)
                {
                    return @"/CaptureSetupModule;component/Icons/TogglePMT_up.png";
                }
                else
                {
                    return @"/CaptureSetupModule;component/Icons/TogglePMT_down.png";
                }
            }
        }

        public int LightPathCamEnable
        {
            get
            {
                return this._captureSetup.LightPathCamEnable;
            }
            set
            {
                this._captureSetup.LightPathCamEnable = value;
                OnPropertyChanged("LightPathCamEnable");
                OnPropertyChanged("LightPathCamDisplayOn");
                OnPropertyChanged("LightPathCamDisplayOff");
            }
        }

        public string LightPathGGDisplayOff
        {
            get
            {
                if (1 == this.LightPathGGEnable)
                {
                    return @"/CaptureSetupModule;component/Icons/ToggleOff_up.png";
                }
                else
                {
                    return @"/CaptureSetupModule;component/Icons/ToggleOff_down.png";
                }
            }
        }

        public string LightPathGGDisplayOn
        {
            get
            {
                if (1 == this.LightPathGGEnable)
                {
                    return @"/CaptureSetupModule;component/Icons/ToggleOn_down.png";
                }
                else
                {
                    return @"/CaptureSetupModule;component/Icons/ToggleOn_up.png";
                }
            }
        }

        public int LightPathGGEnable
        {
            get
            {
                return this._captureSetup.LightPathGGEnable;
            }
            set
            {
                this._captureSetup.LightPathGGEnable = value;
                OnPropertyChanged("LightPathGGEnable");
                OnPropertyChanged("LightPathGGDisplayOn");
                OnPropertyChanged("LightPathGGDisplayOff");
            }
        }

        public string LightPathGRDisplayOff
        {
            get
            {
                if (1 == this.LightPathGREnable)
                {
                    return @"/CaptureSetupModule;component/Icons/ToggleOff_up.png";
                }
                else
                {
                    return @"/CaptureSetupModule;component/Icons/ToggleOff_down.png";
                }
            }
        }

        public string LightPathGRDisplayOn
        {
            get
            {
                if (1 == this.LightPathGREnable)
                {
                    return @"/CaptureSetupModule;component/Icons/ToggleOn_down.png";
                }
                else
                {
                    return @"/CaptureSetupModule;component/Icons/ToggleOn_up.png";
                }
            }
        }

        public int LightPathGREnable
        {
            get
            {
                return this._captureSetup.LightPathGREnable;
            }
            set
            {
                this._captureSetup.LightPathGREnable = value;
                OnPropertyChanged("LightPathGREnable");
                OnPropertyChanged("LightPathGRDisplayOn");
                OnPropertyChanged("LightPathGRDisplayOff");
            }
        }

        public string LightPathLabel_1
        {
            get
            {
                string ret;
                switch (this._captureSetup.LightPathScopeType)
                {
                    case (int)ScopeType.INVERTED:
                        ret = GetValueString("/ApplicationSettings/DisplayOptions/CaptureSetup/InvertedLightPathView", "LeftNameChange");
                        break;
                    case (int)ScopeType.UPRIGHT:
                    default:
                        ret = "Galvo/Galvo";
                        break;
                }
                return ret;
            }
        }

        public string LightPathLabel_2
        {
            get
            {
                string ret;
                switch (this._captureSetup.LightPathScopeType)
                {
                    case (int)ScopeType.INVERTED:
                        ret = GetValueString("/ApplicationSettings/DisplayOptions/CaptureSetup/InvertedLightPathView", "CenterNameChange");
                        break;
                    case (int)ScopeType.UPRIGHT:
                    default:
                        ret = "Galvo/Resonance";
                        break;
                }
                return ret;
            }
        }

        public string LightPathLabel_3
        {
            get
            {
                string ret;
                switch (this._captureSetup.LightPathScopeType)
                {
                    case (int)ScopeType.INVERTED:
                        ret = GetValueString("/ApplicationSettings/DisplayOptions/CaptureSetup/InvertedLightPathView", "RightNameChange");
                        break;
                    case (int)ScopeType.UPRIGHT:
                    default:
                        ret = "Camera";
                        break;
                }
                return ret;
            }
        }

        public ICommand LightPathListAddCommand
        {
            get
            {
                if (this._lightPathListAddCommand == null)
                    this._lightPathListAddCommand = new RelayCommand(() => LightPathListAdd());

                return _lightPathListAddCommand;
            }
        }

        public RelayCommandWithParam LightPathSwitch
        {
            get
            {
                if (this._lightPathSwitch == null)
                    this._lightPathSwitch = new RelayCommandWithParam(LightPathSwitchVM);

                return this._lightPathSwitch;
            }
        }

        public Thickness MarginCenterInvertedButton
        {
            get
            {
                if (IsNDDAvailable)
                {
                    return new Thickness(20, 0, 20, 10);
                }
                else
                {
                    return new Thickness(45, 0, 45, 10);
                }
            }
        }

        public Thickness MarginLeftInvertedButton
        {
            get
            {
                if (IsNDDAvailable)
                {
                    return new Thickness(30, 0, 20, 10);
                }
                else
                {
                    return new Thickness(45, 0, 45, 10);
                }
            }
        }

        public Thickness MarginRightInvertedButton
        {
            get
            {
                if (IsNDDAvailable)
                {
                    return new Thickness(20, 0, 20, 10);
                }
                else
                {
                    return new Thickness(45, 0, 45, 10);
                }
            }
        }

        public int PositionNDD
        {
            get
            {
                return this._captureSetup.PositionNDD;
            }
            set
            {
                this._captureSetup.PositionNDD = value;
                OnPropertyChanged("PositionNDD");
                OnPropertyChanged("DisplayOnNDD");
                OnPropertyChanged("DisplayOffNDD");
            }
        }

        #endregion Properties

        #region Methods

        public void DeleteLightPathSequenceStep(int lightPathSequenceStepIndex)
        {
            string message = "Are you sure you want to delete the sequence step settings: " + _captureSequence[lightPathSequenceStepIndex].Name + "?";
            MessageBoxResult result = MessageBox.Show(message, "Delete Sequence Step Settings", MessageBoxButton.YesNo);
            if (MessageBoxResult.Yes == result)
            {
                _captureSequence.RemoveAt(lightPathSequenceStepIndex);
                ReassignCaptureSequenceLineNumbers();
            }
        }

        public void DeleteLightPathSequenceStepTemplate(int lightPathSequenceStepIndex)
        {
            string message = "Are you sure you want to delete the light path settings: " + _lightPaths[lightPathSequenceStepIndex].Name + "?";
            MessageBoxResult result = MessageBox.Show(message, "Delete Light Path Settings", MessageBoxButton.YesNo);
            if (MessageBoxResult.Yes == result)
            {
                for (int i = 0; i < _captureSequence.Count; i++)
                {
                    if (_captureSequence[i].Name == _lightPaths[lightPathSequenceStepIndex].Name)
                    {
                        _captureSequence.RemoveAt(i);
                        break;
                    }
                }

                _lightPaths.RemoveAt(lightPathSequenceStepIndex);
                ReassignLightPathListLineNumbers();
                ReassignCaptureSequenceLineNumbers();
            }
        }

        public void LoadCaptureSequenceStep(int index)
        {
            LoadLightPathSequenceStep(_captureSequence[index]);
        }

        public void LoadLightPathListtep(int index)
        {
            LoadLightPathSequenceStep(_lightPaths[index]);
        }

        public void UpdateCaptureSequenceStep(int index)
        {
            //Check the channel is not repeated in the Capture Sequence
            int chanASum = (this.LSMChannelEnable0) ? 1 : 0;
            int chanBSum = (this.LSMChannelEnable1) ? 1 : 0;
            int chanCSum = (this.LSMChannelEnable2) ? 1 : 0;
            int chanDSum = (this.LSMChannelEnable3) ? 1 : 0;
            for (int i = 0; i < _captureSequence.Count; i++)
            {
                if (index == i) continue;
                bool chanEnable0 = false, chanEnable1 = false, chanEnable2 = false, chanEnable3 = false;
                _captureSequence[i].GetLightPathSequenceStepLSMChannel(ref chanEnable0, ref chanEnable1, ref chanEnable2, ref chanEnable3);
                chanASum += (true == chanEnable0) ? 1 : 0;
                chanBSum += (true == chanEnable1) ? 1 : 0;
                chanCSum += (true == chanEnable2) ? 1 : 0;
                chanDSum += (true == chanEnable3) ? 1 : 0;
            }

            //If a channel is repeated display a message to the user and return
            if (1 < chanASum || 1 < chanBSum || 1 < chanCSum || 1 < chanDSum)
            {
                MessageBox.Show("Each channel is allowed only once per capture sequence. Make sure none of the selected channels for this capture sequence step is selected in a different capture sequence step.");
                return;
            }

            string message = "Are you sure you want to update the sequence step settings: " + _captureSequence[index].Name + "?";
            MessageBoxResult result = MessageBox.Show(message, "Update Light Path Settings", MessageBoxButton.YesNo);
            if (MessageBoxResult.Yes == result)
            {
                LightPathSequenceStep cs = CreateCurrentSettingsLightPathSequenceStep(_captureSequence[index].Name);
                _captureSequence[index] = cs;
                for (int i = 0; i < _lightPaths.Count; i++)
                {
                    if (_lightPaths[i].Name == cs.Name)
                    {
                        _lightPaths[i] = cs;
                    }
                }
                ReassignLightPathListLineNumbers();
                ReassignCaptureSequenceLineNumbers();
            }
        }

        public void UpdateLightPathList(int index)
        {
            string message = "Are you sure you want to update the Light Path settings: " + _lightPaths[index].Name + "?";
            MessageBoxResult result = MessageBox.Show(message, "Update Light Path Settings", MessageBoxButton.YesNo);
            if (MessageBoxResult.Yes == result)
            {
                LightPathSequenceStep cs = CreateCurrentSettingsLightPathSequenceStep(_lightPaths[index].Name);
                _lightPaths[index] = cs;
                for (int i = 0; i < _captureSequence.Count; i++)
                {
                    //update the sequence as well
                    if (_captureSequence[i].Name == cs.Name)
                    {
                        _captureSequence[i] = cs;
                    }
                }
                ReassignLightPathListLineNumbers();
                ReassignCaptureSequenceLineNumbers();
            }
        }

        private void ConvertBinaryToChanEnable(int binaryValue, ref bool chan0Enable, ref bool chan1Enable, ref bool chan2Enable, ref bool chan3Enable)
        {
            switch (this.DigitizerBoardName)
            {
                case Model.CaptureSetup.DigitizerBoardNames.ATS9440:
                    {
                        switch (binaryValue)
                        {
                            default:
                                {
                                    chan0Enable = Convert.ToBoolean(binaryValue & 0x1);
                                    chan1Enable = Convert.ToBoolean(binaryValue & 0x2);
                                    chan2Enable = Convert.ToBoolean(binaryValue & 0x4);
                                    chan3Enable = Convert.ToBoolean(binaryValue & 0x8);
                                }
                                break;
                        }
                    }
                    break;
                case Model.CaptureSetup.DigitizerBoardNames.ATS460:
                    {
                        switch (binaryValue)
                        {
                            default:
                                {
                                    chan0Enable = Convert.ToBoolean(binaryValue & 0x1);
                                    chan1Enable = Convert.ToBoolean(binaryValue & 0x2);
                                    chan2Enable = false;
                                    chan3Enable = false;
                                }
                                break;
                        }
                    }
                    break;
            }
        }

        private LightPathSequenceStep CreateCurrentSettingsLightPathSequenceStep(string name)
        {
            XmlDocument lightPathListDoc = new XmlDocument();
            lightPathListDoc.LoadXml("<ThorImageLightPathList></ThorImageLightPathList>");

            XmlNodeList ndList = lightPathListDoc.SelectNodes("/ThorImageLightPathList/LightPathSequenceStep");

            ndList = lightPathListDoc.SelectNodes("ThorImageLightPathList");
            if (0 >= ndList.Count)
            {
                XmlElement elem = lightPathListDoc.CreateElement("ThorImageLightPathList");
                lightPathListDoc.AppendChild(elem);
                ndList = lightPathListDoc.SelectNodes("ThorImageLightPathList");
            }

            //Create new LightPathSequenceStep Node
            {
                XmlElement elem = lightPathListDoc.CreateElement("LightPathSequenceStep");
                ndList[0].AppendChild(elem);
                ndList = lightPathListDoc.SelectNodes("/ThorImageLightPathList/LightPathSequenceStep");
            }
            //Update the name of the Capture Sequence Step in the xml file
            XmlManager.SetAttribute(ndList[ndList.Count - 1], lightPathListDoc, "name", name);

            XmlNode stepRootNode = ndList[ndList.Count - 1];

            XmlNodeList innderNdList = stepRootNode.SelectNodes("Wavelengths");

            //Clear Wavelengths node, create it if if doesn't exist
            if (0 < innderNdList.Count)
            {
                innderNdList[0].RemoveAll();
            }
            else
            {
                XmlElement elem = lightPathListDoc.CreateElement("Wavelengths");
                stepRootNode.AppendChild(elem);
            }

            innderNdList = stepRootNode.SelectNodes("Wavelengths");

            XmlManager.SetAttribute(innderNdList[0], lightPathListDoc, "nyquistExWavelengthNM", MVMManager.Instance["AreaControlViewModel", "NyquistExWavelength"].ToString());
            XmlManager.SetAttribute(innderNdList[0], lightPathListDoc, "nyquistEmWavelengthNM", MVMManager.Instance["AreaControlViewModel", "NyquistEmWavelength"].ToString());

            XmlElement newElement;

            //Add the wavelength names and exposure time for the current Capture Sequence Step
            XmlNodeList waveList = this.HardwareDoc.SelectNodes("/HardwareSettings/Wavelength");
            switch (this.LSMChannel)
            {
                case 0:
                    {
                        newElement = CreateWavelengthTag(waveList[0].Attributes["name"].Value, lightPathListDoc);
                        innderNdList[0].AppendChild(newElement);

                    }
                    break;
                case 1:
                    {
                        newElement = CreateWavelengthTag(waveList[1].Attributes["name"].Value, lightPathListDoc);
                        innderNdList[0].AppendChild(newElement);
                    }
                    break;
                case 2:
                    {
                        newElement = CreateWavelengthTag(waveList[2].Attributes["name"].Value, lightPathListDoc);
                        innderNdList[0].AppendChild(newElement);
                    }
                    break;
                case 3:
                    {
                        newElement = CreateWavelengthTag(waveList[3].Attributes["name"].Value, lightPathListDoc);
                        innderNdList[0].AppendChild(newElement);
                    }
                    break;
                case 4:
                    {
                        for (int j = 0; j < waveList.Count; j++)
                        {
                            bool isEnabled = false;

                            switch (j)
                            {
                                case 0: isEnabled = LSMChannelEnable0; break;
                                case 1: isEnabled = LSMChannelEnable1; break;
                                case 2: isEnabled = LSMChannelEnable2; break;
                                case 3: isEnabled = LSMChannelEnable3; break;
                            }

                            if (isEnabled)
                            {
                                newElement = CreateWavelengthTag(waveList[j].Attributes["name"].Value, lightPathListDoc);
                                innderNdList[0].AppendChild(newElement);
                            }
                        }
                    }
                    break;
            }

            //set the node to LSM, create it if it doesn't exist
            innderNdList = stepRootNode.SelectNodes("LSM");
            if (0 >= innderNdList.Count)
            {
                XmlElement elem = lightPathListDoc.CreateElement("LSM");
                stepRootNode.AppendChild(elem);
                innderNdList = stepRootNode.SelectNodes("LSM");
            }

            //Save the channel property for LSM in binary format
            if (this.LSMChannel == 4)
            {
                //channel is stored as a zero based index
                //convert the enabled channels to the index value
                int chanel = (Convert.ToInt32(LSMChannelEnable0) | (Convert.ToInt32(LSMChannelEnable1) << 1) | (Convert.ToInt32(LSMChannelEnable2) << 2) | (Convert.ToInt32(LSMChannelEnable3) << 3));
                XmlManager.SetAttribute(innderNdList[0], lightPathListDoc, "channel", chanel.ToString());
            }
            else
            {
                int chanel = 0;
                switch (this.LSMChannel)
                {
                    case 0: chanel = 0x1; break;
                    case 1: chanel = 0x2; break;
                    case 2: chanel = 0x4; break;
                    case 3: chanel = 0x8; break;
                }
                XmlManager.SetAttribute(innderNdList[0], lightPathListDoc, "channel", chanel.ToString());
            }

            //set the node to MCLS, create it if it doesn't exist
            innderNdList = stepRootNode.SelectNodes("MCLS");
            if (0 >= innderNdList.Count)
            {
                XmlElement elem = lightPathListDoc.CreateElement("MCLS");
                stepRootNode.AppendChild(elem);
                innderNdList = stepRootNode.SelectNodes("MCLS");
            }

            //add all the attributes for MCLS for the current Capture Sequence Step
            decimal power1percent = Power2PercentConvertion((double)MVMManager.Instance["MultiLaserControlViewModel", "Laser1Power"],
                                                            (double)MVMManager.Instance["MultiLaserControlViewModel", "Laser1Max"],
                                                            (double)MVMManager.Instance["MultiLaserControlViewModel", "Laser1Min"]);
            decimal power2percent = Power2PercentConvertion((double)MVMManager.Instance["MultiLaserControlViewModel", "Laser2Power"],
                                                            (double)MVMManager.Instance["MultiLaserControlViewModel", "Laser2Max"],
                                                            (double)MVMManager.Instance["MultiLaserControlViewModel", "Laser2Min"]);
            decimal power3percent = Power2PercentConvertion((double)MVMManager.Instance["MultiLaserControlViewModel", "Laser3Power"],
                                                            (double)MVMManager.Instance["MultiLaserControlViewModel", "Laser3Max"],
                                                            (double)MVMManager.Instance["MultiLaserControlViewModel", "Laser3Min"]);
            decimal power4percent = Power2PercentConvertion((double)MVMManager.Instance["MultiLaserControlViewModel", "Laser4Power"],
                                                            (double)MVMManager.Instance["MultiLaserControlViewModel", "Laser4Max"],
                                                            (double)MVMManager.Instance["MultiLaserControlViewModel", "Laser4Min"]);
            // SetAttribute(innderNdList[0], lightPathListDoc, "MainLaserSelection", this.MainLaserIndex.ToString());
            SetAttribute(innderNdList[0], lightPathListDoc, "enable1", MVMManager.Instance["MultiLaserControlViewModel", "Laser1Enable"].ToString());
            SetAttribute(innderNdList[0], lightPathListDoc, "power1", MVMManager.Instance["MultiLaserControlViewModel", "Laser1Power"].ToString());
            SetAttribute(innderNdList[0], lightPathListDoc, "enable2", MVMManager.Instance["MultiLaserControlViewModel", "Laser2Enable"].ToString());
            SetAttribute(innderNdList[0], lightPathListDoc, "power2", MVMManager.Instance["MultiLaserControlViewModel", "Laser2Power"].ToString());
            SetAttribute(innderNdList[0], lightPathListDoc, "enable3", MVMManager.Instance["MultiLaserControlViewModel", "Laser3Enable"].ToString());
            SetAttribute(innderNdList[0], lightPathListDoc, "power3", MVMManager.Instance["MultiLaserControlViewModel", "Laser3Power"].ToString());
            SetAttribute(innderNdList[0], lightPathListDoc, "enable4", MVMManager.Instance["MultiLaserControlViewModel", "Laser4Enable"].ToString());
            SetAttribute(innderNdList[0], lightPathListDoc, "power4", MVMManager.Instance["MultiLaserControlViewModel", "Laser4Power"].ToString());
            SetAttribute(innderNdList[0], lightPathListDoc, "power1percent", power1percent.ToString());
            SetAttribute(innderNdList[0], lightPathListDoc, "power2percent", power2percent.ToString());
            SetAttribute(innderNdList[0], lightPathListDoc, "power3percent", power3percent.ToString());
            SetAttribute(innderNdList[0], lightPathListDoc, "power4percent", power4percent.ToString());
            //Set these to set visibility of wavelength value when hovering over sequential capture Light Path
            SetAttribute(innderNdList[0], lightPathListDoc, "wavelength1", MVMManager.Instance["MultiLaserControlViewModel", "Laser1Wavelength"].ToString());
            SetAttribute(innderNdList[0], lightPathListDoc, "wavelength2", MVMManager.Instance["MultiLaserControlViewModel", "Laser2Wavelength"].ToString());
            SetAttribute(innderNdList[0], lightPathListDoc, "wavelength3", MVMManager.Instance["MultiLaserControlViewModel", "Laser3Wavelength"].ToString());
            SetAttribute(innderNdList[0], lightPathListDoc, "wavelength4", MVMManager.Instance["MultiLaserControlViewModel", "Laser4Wavelength"].ToString());

            //set the node to MultiPhotonLaser, create it if it doesn't exist
            innderNdList = stepRootNode.SelectNodes("MultiPhotonLaser");
            if (0 >= innderNdList.Count)
            {
                XmlElement elem = lightPathListDoc.CreateElement("MultiPhotonLaser");
                stepRootNode.AppendChild(elem);
                innderNdList = stepRootNode.SelectNodes("MultiPhotonLaser");
            }

            //add all the attributes for multiphoton laser for the current Capture Sequence Step
            XmlManager.SetAttribute(innderNdList[0], lightPathListDoc, "pos", MVMManager.Instance["MultiphotonControlViewModel", "Laser1Position"].ToString());

            //set the node to PinholeWheel, create it if it doesn't exist
            innderNdList = stepRootNode.SelectNodes("PinholeWheel");
            if (0 >= innderNdList.Count)
            {
                XmlElement elem = lightPathListDoc.CreateElement("PinholeWheel");
                stepRootNode.AppendChild(elem);
                innderNdList = stepRootNode.SelectNodes("PinholeWheel");
            }

            //add all the attributes for PinholeWheel for the current Capture Sequence Step
            if (innderNdList.Count > 0)
            {
                XmlManager.SetAttribute(innderNdList[0], lightPathListDoc, "position", MVMManager.Instance["PinholeControlViewModel", "PinholePosition", (object)1.0].ToString());
                string atrVal = MVMManager.Instance["PinholeControlViewModel", "PinholeSizeUM", (object)1.0].ToString();
                XmlManager.SetAttribute(innderNdList[0], lightPathListDoc, "micrometers", atrVal);
                XmlManager.SetAttribute(innderNdList[0], lightPathListDoc, "adu", MVMManager.Instance["PinholeControlViewModel", "PinholeADUs", (object)1.0].ToString());
            }

            //set the node to PMT, create it if it doesn't exist
            innderNdList = stepRootNode.SelectNodes("PMT");
            if (0 >= innderNdList.Count)
            {
                XmlElement elem = lightPathListDoc.CreateElement("PMT");
                stepRootNode.AppendChild(elem);
                innderNdList = stepRootNode.SelectNodes("PMT");
            }

            //add all the attributes for PMT for the current Capture Sequence Step
            HwVal<int> ga = (HwVal<int>)MVMManager.Instance["ScanControlViewModel", "PMTGain", 0, (object)0];
            XmlManager.SetAttribute(innderNdList[0], lightPathListDoc, "gainA", ga.Value.ToString());
            XmlManager.SetAttribute(innderNdList[0], lightPathListDoc, "enableA", (0 < ga.Value) ? "1" : "0");
            HwVal<int> gb = (HwVal<int>)MVMManager.Instance["ScanControlViewModel", "PMTGain", 1, (object)0];
            XmlManager.SetAttribute(innderNdList[0], lightPathListDoc, "gainB", gb.Value.ToString());
            XmlManager.SetAttribute(innderNdList[0], lightPathListDoc, "enableB", (0 < gb.Value) ? "1" : "0");
            HwVal<int> gc = (HwVal<int>)MVMManager.Instance["ScanControlViewModel", "PMTGain", 2, (object)0];
            XmlManager.SetAttribute(innderNdList[0], lightPathListDoc, "gainC", gc.Value.ToString());
            XmlManager.SetAttribute(innderNdList[0], lightPathListDoc, "enableC", (0 < gc.Value) ? "1" : "0");
            HwVal<int> gd = (HwVal<int>)MVMManager.Instance["ScanControlViewModel", "PMTGain", 3, (object)0];
            XmlManager.SetAttribute(innderNdList[0], lightPathListDoc, "gainD", gd.Value.ToString());
            XmlManager.SetAttribute(innderNdList[0], lightPathListDoc, "enableD", (0 < gd.Value) ? "1" : "0");

            //set the lightPath settings
            innderNdList = stepRootNode.SelectNodes("LightPath");
            if (0 >= innderNdList.Count)
            {
                XmlElement elem = lightPathListDoc.CreateElement("LightPath");
                stepRootNode.AppendChild(elem);
                innderNdList = stepRootNode.SelectNodes("LightPath");
            }

            XmlManager.SetAttribute(innderNdList[0], lightPathListDoc, "GalvoGalvo", this.LightPathGGEnable.ToString());
            XmlManager.SetAttribute(innderNdList[0], lightPathListDoc, "GalvoResonance", this.LightPathGREnable.ToString());
            XmlManager.SetAttribute(innderNdList[0], lightPathListDoc, "Camera", this.LightPathCamEnable.ToString());
            if (-1 == this.InvertedLightPathPos)
            {
                XmlManager.SetAttribute(innderNdList[0], lightPathListDoc, "InvertedLightPathPos", "-1");
            }
            else
            {
                XmlManager.SetAttribute(innderNdList[0], lightPathListDoc, "InvertedLightPathPos", this.InvertedLightPathPos.ToString());
            }
            XmlManager.SetAttribute(innderNdList[0], lightPathListDoc, "NDD", this.PositionNDD.ToString());

            //set the pockels settings
            innderNdList = stepRootNode.SelectNodes("Pockels");
            if (0 >= innderNdList.Count)
            {
                XmlElement elem1 = lightPathListDoc.CreateElement("Pockels");
                stepRootNode.AppendChild(elem1);
                XmlElement elem2 = lightPathListDoc.CreateElement("Pockels");
                stepRootNode.AppendChild(elem2);
                XmlElement elem3 = lightPathListDoc.CreateElement("Pockels");
                stepRootNode.AppendChild(elem3);
                XmlElement elem4 = lightPathListDoc.CreateElement("Pockels");
                stepRootNode.AppendChild(elem4);
                innderNdList = stepRootNode.SelectNodes("Pockels");
            }

            MVMManager.Instance["PowerControlViewModel", "PockelsNodeList"] = new KeyValuePair<XmlNodeList, XmlDocument>(innderNdList, lightPathListDoc);

            LightPathSequenceStep cs = new LightPathSequenceStep(name, stepRootNode, ndList.Count, this.DigitizerBoardName);
            return cs;
        }

        private void LightPathListAdd()
        {
            XmlDocument lightPathListDoc = new XmlDocument();
            lightPathListDoc.LoadXml("<ThorImageLightPathList></ThorImageLightPathList>");

            //Dialog to allow the user to enter a name for the current Channel Step being saved
            LightPathSequenceStepNameEdit dlg;
            bool repeatedName = false;
            do
            {
                repeatedName = false;
                string str = string.Empty;
                dlg = new LightPathSequenceStepNameEdit();
                if (false == dlg.ShowDialog())
                {
                    return;
                }

                for (int i = 0; i < _lightPaths.Count; i++)
                {
                    if (_lightPaths[i].Name == dlg.LightPathSequenceStepName)
                    {
                        MessageBox.Show("The name for the new Light Path settings must be unique. Please use a different name.");
                        repeatedName = true;
                        break;
                    }
                }
            } while (repeatedName);

            _lightPaths.Add(CreateCurrentSettingsLightPathSequenceStep(dlg.LightPathSequenceStepName));
            ReassignLightPathListLineNumbers();
        }

        private void LightPathSwitchVM(object param)
        {
            string p = (string)param;
            int t;
            switch (LightPathCmds[p])
            {
                case LightPathBtns.GG_IN:
                    LightPathGGEnable = 1;
                    break;
                case LightPathBtns.GG_OUT:
                    LightPathGGEnable = 0;
                    break;
                case LightPathBtns.GG_Flip:
                    t = LightPathGGEnable;
                    t = (t + 1) % 2;
                    LightPathGGEnable = t;
                    break;
                case LightPathBtns.GR_IN:
                    LightPathGREnable = 1;
                    break;
                case LightPathBtns.GR_OUT:
                    LightPathGREnable = 0;
                    break;
                case LightPathBtns.GR_Flip:
                    t = LightPathGREnable;
                    t = (t + 1) % 2;
                    LightPathGREnable = t;
                    break;
                case LightPathBtns.CAM_PMT:
                    LightPathCamEnable = 0;
                    break;
                case LightPathBtns.CAM_CAM:
                    LightPathCamEnable = 1;
                    break;
                case LightPathBtns.CAM_Flip:
                    t = LightPathCamEnable;
                    t = (t + 1) % 2;
                    LightPathCamEnable = t;
                    break;
                case LightPathBtns.Left:
                    InvertedLpLeftEnable = true;
                    InvertedLpCenterEnable = false;
                    InvertedLpRightEnable = false;
                    break;
                case LightPathBtns.Center:
                    InvertedLpCenterEnable = true;
                    InvertedLpRightEnable = false;
                    InvertedLpLeftEnable = false;
                    break;
                case LightPathBtns.Right:
                    InvertedLpRightEnable = true;
                    InvertedLpLeftEnable = false;
                    InvertedLpCenterEnable = false;
                    break;
                case LightPathBtns.NDD_IN:
                    PositionNDD = 1;
                    break;
                case LightPathBtns.NDD_OUT:
                    PositionNDD = 0;
                    break;
            }
        }

        private void LoadLightPathSequenceStep(LightPathSequenceStep lightPathSequenceStep)
        {
            //Load and set Channel Enable form LSM
            bool chanEnable0 = false, chanEnable1 = false, chanEnable2 = false, chanEnable3 = false;
            lightPathSequenceStep.GetLightPathSequenceStepLSMChannel(ref chanEnable0, ref chanEnable1, ref chanEnable2, ref chanEnable3);
            this.LSMChannelEnable0 = chanEnable0;
            this.LSMChannelEnable1 = chanEnable1;
            this.LSMChannelEnable2 = chanEnable2;
            this.LSMChannelEnable3 = chanEnable3;

            //Load and set MCLS Laser settings
            int mainLaserSelection = -1;
            int laser1Enable = 0, laser2Enable = 0, laser3Enable = 0, laser4Enable = 0;
            double laser1Power = 0.0, laser1Percent = 0.0;
            double laser2Power = 0.0, laser2Percent = 0.0;
            double laser3Power = 0.0, laser3Percent = 0.0;
            double laser4Power = 0.0, laser4Percent = 0.0;
            int laser1Wavelength = 0, laser2Wavelength = 0, laser3Wavelength = 0, laser4Wavelength = 0;
            lightPathSequenceStep.GetLightPathSequenceStepMCLS(ref mainLaserSelection, ref laser1Enable, ref laser1Power, ref laser1Percent, ref laser2Enable, ref laser2Power, ref laser2Percent,
                ref laser3Enable, ref laser3Power, ref laser3Percent, ref laser4Enable, ref laser4Power, ref laser4Percent, ref laser1Wavelength, ref laser2Wavelength, ref laser3Wavelength,
                ref laser4Wavelength);
            MVMManager.Instance["MultiLaserControlViewModel", "MainLaserIndex"] = mainLaserSelection;
            MVMManager.Instance["MultiLaserControlViewModel", "Laser1Enable"] = laser1Enable;
            MVMManager.Instance["MultiLaserControlViewModel", "Laser1Power"] = laser1Power;
            MVMManager.Instance["MultiLaserControlViewModel", "Laser2Enable"] = laser2Enable;
            MVMManager.Instance["MultiLaserControlViewModel", "Laser2Power"] = laser2Power;
            MVMManager.Instance["MultiLaserControlViewModel", "Laser3Enable"] = laser3Enable;
            MVMManager.Instance["MultiLaserControlViewModel", "Laser3Power"] = laser3Power;
            MVMManager.Instance["MultiLaserControlViewModel", "Laser4Enable"] = laser4Enable;
            MVMManager.Instance["MultiLaserControlViewModel", "Laser4Power"] = laser4Power;

            //Load and set Multiphoton Laser Settings
            int laserPosition = 0;
            lightPathSequenceStep.GetLightPathSequenceStepMultiphoton(ref laserPosition);
            MVMManager.Instance["MultiphotonControlViewModel", "Laser1Position"] = laserPosition;

            //Load and set PMT Settings
            int pmt1Gain = 0, pmt2Gain = 0, pmt3Gain = 0, pmt4Gain = 0;
            lightPathSequenceStep.GetLightPathSequenceStepPMT(ref pmt1Gain, ref pmt2Gain, ref pmt3Gain, ref pmt4Gain);
            ((HwVal<int>)MVMManager.Instance["ScanControlViewModel", "PMTGain", 0]).Value = pmt1Gain;
            ((HwVal<int>)MVMManager.Instance["ScanControlViewModel", "PMTGain", 1]).Value = pmt2Gain;
            ((HwVal<int>)MVMManager.Instance["ScanControlViewModel", "PMTGain", 2]).Value = pmt3Gain;
            ((HwVal<int>)MVMManager.Instance["ScanControlViewModel", "PMTGain", 3]).Value = pmt4Gain;

            //Load and set PinholeWheel Settings
            int pinholePosition = 0;
            double pinholeMicrometers = 0.0, pinholeADUs = 0.0;
            lightPathSequenceStep.GetLightPathSequenceStepPinhole(ref pinholePosition, ref pinholeMicrometers, ref pinholeADUs);
            MVMManager.Instance["PinholeControlViewModel", "PinholePosition"] = pinholePosition;

            //Load Pockels Settings
            const int MAX_POCKELS = 4;
            double[] pockelsPowerLevel = new double[MAX_POCKELS];
            int[] powerType = new int[MAX_POCKELS];
            string[] powerRampName = new string[MAX_POCKELS];
            lightPathSequenceStep.GetPockels(ref pockelsPowerLevel, ref powerType, ref powerRampName);

            //check MVM is available
            if (null == MVMManager.Instance["PowerControlViewModel", "PowerRampsCustom"])
                return;

            for (int j = 0; j < MAX_POCKELS; j++)
            {
                string str = string.Empty;
                switch (j)
                {
                    case 0:
                        {
                            MVMManager.Instance["PowerControlViewModel", "PowerMode0"] = powerType[j];

                            for (int k = 0; k < ((ObservableCollection<string>)MVMManager.Instance["PowerControlViewModel", "PowerRampsCustom"]).Count; k++)
                            {
                                if (((ObservableCollection<string>)MVMManager.Instance["PowerControlViewModel", "PowerRampsCustom"])[k].Equals(powerRampName[j]))
                                {
                                    MVMManager.Instance["PowerControlViewModel", "PowerRampSelected0"] = k;
                                }
                            }
                            if (1 == (int)MVMManager.Instance["PowerControlViewModel", "PowerMode0"] && 0 < ((ObservableCollection<string>)MVMManager.Instance["PowerControlViewModel", "PowerRampsCustom"]).Count &&
                                (((ObservableCollection<string>)MVMManager.Instance["PowerControlViewModel", "PowerRampsCustom"]).Count - 1) >= (int)MVMManager.Instance["PowerControlViewModel", "PowerRampSelected0"])
                            {
                                //power value updated by selected tab.
                            }
                            else
                            {
                                MVMManager.Instance["PowerControlViewModel", "Power0"] = pockelsPowerLevel[j];
                            }
                        }
                        break;
                    case 1:
                        {
                            MVMManager.Instance["PowerControlViewModel", "PowerMode1"] = powerType[j];

                            for (int k = 0; k < ((ObservableCollection<string>)MVMManager.Instance["PowerControlViewModel", "PowerRampsCustom"]).Count; k++)
                            {
                                if (((ObservableCollection<string>)MVMManager.Instance["PowerControlViewModel", "PowerRampsCustom"])[k].Equals(powerRampName[j]))
                                {
                                    MVMManager.Instance["PowerControlViewModel", "PowerRampSelected1"] = k;
                                }
                            }
                            if (1 == (int)MVMManager.Instance["PowerControlViewModel", "PowerMode1"] && 0 < ((ObservableCollection<string>)MVMManager.Instance["PowerControlViewModel", "PowerRampsCustom"]).Count &&
                                (((ObservableCollection<string>)MVMManager.Instance["PowerControlViewModel", "PowerRampsCustom"]).Count - 1) >= (int)MVMManager.Instance["PowerControlViewModel", "PowerRampSelected1"])
                            {
                                //power value updated by selected tab.
                            }
                            else
                            {
                                MVMManager.Instance["PowerControlViewModel", "Power1"] = pockelsPowerLevel[j];
                            }
                        }
                        break;
                    case 2:
                        {
                            MVMManager.Instance["PowerControlViewModel", "PowerMode2"] = powerType[j];

                            for (int k = 0; k < ((ObservableCollection<string>)MVMManager.Instance["PowerControlViewModel", "PowerRampsCustom"]).Count; k++)
                            {
                                if (((ObservableCollection<string>)MVMManager.Instance["PowerControlViewModel", "PowerRampsCustom"])[k].Equals(powerRampName[j]))
                                {
                                    MVMManager.Instance["PowerControlViewModel", "PowerRampSelected2"] = k;
                                }
                            }
                            if (1 == (int)MVMManager.Instance["PowerControlViewModel", "PowerMode2"] && 0 < ((ObservableCollection<string>)MVMManager.Instance["PowerControlViewModel", "PowerRampsCustom"]).Count &&
                                (((ObservableCollection<string>)MVMManager.Instance["PowerControlViewModel", "PowerRampsCustom"]).Count - 1) >= (int)MVMManager.Instance["PowerControlViewModel", "PowerRampSelected2"])
                            {
                                //power value updated by selected tab.
                            }
                            else
                            {
                                MVMManager.Instance["PowerControlViewModel", "Power2"] = pockelsPowerLevel[j];
                            }
                        }
                        break;
                    case 3:
                        {
                            MVMManager.Instance["PowerControlViewModel", "PowerMode3"] = powerType[j];

                            for (int k = 0; k < ((ObservableCollection<string>)MVMManager.Instance["PowerControlViewModel", "PowerRampsCustom"]).Count; k++)
                            {
                                if (((ObservableCollection<string>)MVMManager.Instance["PowerControlViewModel", "PowerRampsCustom"])[k].Equals(powerRampName[j]))
                                {
                                    MVMManager.Instance["PowerControlViewModel", "PowerRampSelected3"] = k;
                                }
                            }
                            if (1 == (int)MVMManager.Instance["PowerControlViewModel", "PowerMode3"] && 0 < ((ObservableCollection<string>)MVMManager.Instance["PowerControlViewModel", "PowerRampsCustom"]).Count &&
                                (((ObservableCollection<string>)MVMManager.Instance["PowerControlViewModel", "PowerRampsCustom"]).Count - 1) >= (int)MVMManager.Instance["PowerControlViewModel", "PowerRampSelected3"])
                            {
                                //power value updated by selected tab.
                            }
                            else
                            {
                                MVMManager.Instance["PowerControlViewModel", "Power3"] = pockelsPowerLevel[j];
                            }
                        }
                        break;
                }

                //update selected tab for power value
                MVMManager.Instance["PowerControlViewModel", "SelectedPowerTab"] = MVMManager.Instance["PowerControlViewModel", "SelectedPowerTab"];
            }

            //Load Light Path Settings
            int M1Enable = 0, M2Enable = 0, M3Enable = 0;
            lightPathSequenceStep.GetLightPath(ref M1Enable, ref M2Enable, ref M3Enable);
            if ((int)ScopeType.INVERTED == _captureSetup.LightPathScopeType)
            {
                this.InvertedLpLeftEnable = (M1Enable == 1);
                this.InvertedLpCenterEnable = (M2Enable == 1);
                this.InvertedLpRightEnable = (M3Enable == 1);
            }
            else
            {
                this.LightPathGGEnable = M1Enable;
                this.LightPathGREnable = M2Enable;
                this.LightPathCamEnable = M3Enable;
            }
        }

        private void ReassignCaptureSequenceLineNumbers()
        {
            for (int i = 0; i < _captureSequence.Count; i++)
            {
                _captureSequence[i].SequenceLineNumber = i + 1;
            }
        }

        private void ReassignLightPathListLineNumbers()
        {
            for (int i = 0; i < _lightPaths.Count; i++)
            {
                _lightPaths[i].LightPathLineNumber = i + 1;
            }
        }

        #endregion Methods
    }
}