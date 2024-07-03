namespace SequentialControl
{
    using System;
    using System.Collections.ObjectModel;
    using System.ComponentModel;
    using System.Globalization;
    using System.IO;
    using System.Windows;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Xml;

    using ThorSharedTypes;

    public class SequenceStep : INotifyPropertyChanged
    {
        #region Fields

        const int MAX_CHANNELS = 4;

        private ICommand _applyStepTemplateCommand;
        private double _captureSequenceWidth = 174;
        private ICommand _changeSequenceNameCommand;
        private ICommand _deleteStepSequenceCommand;
        private ICommand _deleteStepTemplateCommand;
        private string _name = string.Empty;
        private int _sequenceLineNumber = 1;
        private double _stepTemplateWidth = 362;
        private int _templateLineNumber = 0;
        private XmlNode _templateSequenceStepNode = null;
        private ICommand _updateStepTemplateCommand;

        #endregion Fields

        #region Constructors

        public SequenceStep(string str, XmlNode sequenceStepNode, int lineNumber, bool isInCaptureSequence)
        {
            _name = str;
            _templateSequenceStepNode = sequenceStepNode;
            _templateLineNumber = lineNumber;
            _sequenceLineNumber = lineNumber + 1; //Need to add 1 to the SequenceLineNumber because we display this value
            if (isInCaptureSequence)
            {
                AddChannelIndexToName();
            }
        }

        #endregion Constructors

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public ICommand ApplyStepTemplateCommand
        {
            get => _applyStepTemplateCommand ?? (_applyStepTemplateCommand = new RelayCommand(() => LoadSequenceStep()));
        }

        public double CaptureSequenceWidth
        {
            get
            {
                return _captureSequenceWidth;
            }
            set
            {
                _captureSequenceWidth = value;
                OnPropertyChanged("CaptureSequenceWidth");
            }
        }

        public ICommand ChangeSequenceNameCommand
        {
            get => _changeSequenceNameCommand ?? (_changeSequenceNameCommand = new RelayCommand(() => { ModifySequenceName(); }));
        }

        public ICommand DeleteStepSequenceCommand
        {
            get => _deleteStepSequenceCommand ?? (_deleteStepSequenceCommand = new RelayCommand(() => DeleteSequenceStep()));
        }

        public ICommand DeleteStepTemplateCommand
        {
            get => _deleteStepTemplateCommand ?? (_deleteStepTemplateCommand = new RelayCommand(() => DeleteSequenceStepTemplate()));
        }

        public string Name
        {
            get { return _name; }

            set
            {
                if (_name != value)
                {
                    _name = value;
                    _templateSequenceStepNode.Attributes["name"].Value = _name;
                    OnPropertyChanged("Name");
                }
            }
        }

        public int SequenceLineNumber
        {
            get
            {
                return _sequenceLineNumber;
            }
            set
            {
                if (_sequenceLineNumber != value)
                {
                    ChangeSequenceStepChannelIndex(_sequenceLineNumber, value);
                    _sequenceLineNumber = value;
                    OnPropertyChanged("SequenceLineNumber");
                }
            }
        }

        public XmlNode SequenceStepNode
        {
            get
            {
                return _templateSequenceStepNode;
            }
            set
            {
                _templateSequenceStepNode = value;
            }
        }

        public SequenceStepView SequenceStepPreview
        {
            get
            {
                return new SequenceStepView(this);
            }
        }

        public long SequenceStepScopeType
        {
            get;
            set;
        }

        public Visibility SequenceStepVisibility
        {
            get
            {
                if (null != _templateSequenceStepNode)
                {
                    XmlNodeList ndList = ((int)ICamera.CameraType.LSM == ResourceManagerCS.GetCameraType()) ? _templateSequenceStepNode.SelectNodes("LSM") : _templateSequenceStepNode.SelectNodes("Camera");
                    if (0 < ndList?.Count)
                    {
                        string str = string.Empty;
                        if (GetAttribute(ndList[0], "channel", ref str))
                        {
                            if (Int32.TryParse(str, out int val))
                            {
                                if (val > 0)
                                {
                                    return Visibility.Visible;
                                }
                            }
                        }
                    }
                }
                return Visibility.Collapsed;
            }
        }

        public double StepTemplateWidth
        {
            get
            {
                return _stepTemplateWidth;
            }
            set
            {
                _stepTemplateWidth = value;
                OnPropertyChanged("StepTemplateWidth");
            }
        }

        public int TemplateLineNumber
        {
            get
            {
                return _templateLineNumber;
            }
            set
            {
                if (_templateLineNumber != value)
                {
                    _templateLineNumber = value;
                    OnPropertyChanged("TemplateLineNumber");
                }
            }
        }

        public ICommand UpdateStepTemplateCommand
        {
            get => _updateStepTemplateCommand ?? (_updateStepTemplateCommand = new RelayCommand(() => UpdateList()));
        }

        #endregion Properties

        #region Methods

        public void AddChannelIndexToName()
        {
            if (null != _templateSequenceStepNode)
            {
                XmlNodeList wavNdList = _templateSequenceStepNode.SelectNodes("Wavelengths/Wavelength");
                if (0 < wavNdList.Count)
                {
                    string str = string.Empty;
                    string chanName;
                    foreach (XmlNode wavelength in wavNdList)
                    {
                        if (GetAttribute(wavelength, "name", ref str))
                        {
                            string sequenceNum = _sequenceLineNumber.ToString();
                            int index = Math.Min(str.Length, sequenceNum.Length); //To avoid crash with two digit values
                            string currIndex = str.Substring(str.Length - index);
                            if (currIndex != sequenceNum)
                            {
                                chanName = str + sequenceNum;
                                wavelength.Attributes["name"].Value = chanName;
                            }
                        }
                    }
                }
            }
        }

        public void ChangeSequenceStepChannelIndex(int oldSequenceIndex, int newSequenceIndex)
        {
            if (null != _templateSequenceStepNode)
            {
                XmlNodeList wavNdList = _templateSequenceStepNode.SelectNodes("Wavelengths/Wavelength");
                if (0 < wavNdList.Count)
                {
                    string str = string.Empty;
                    string chanName;
                    foreach (XmlNode wavelength in wavNdList)
                    {
                        if (GetAttribute(wavelength, "name", ref str))
                        {
                            if (str.ToLower().Contains("chan"))
                            {
                                var x = str.Substring(str.ToLower().IndexOf("chan") + 4);
                                chanName = x;
                            }
                            else
                            {
                                chanName = str;
                            }
                            string oldIndexNum = oldSequenceIndex.ToString();
                            int index = Math.Min(chanName.Length, oldIndexNum.Length); //To avoid crash with two digit values
                            string currIndex = chanName.Substring(chanName.Length - index);
                            if (currIndex == oldIndexNum)
                            {
                                str = chanName.Substring(0, chanName.Length - oldIndexNum.Length) + newSequenceIndex.ToString();
                                wavelength.Attributes["name"].Value = str;
                            }
                        }
                    }
                }
            }
        }

        public void DeleteSequenceStep()
        {
            string message = "Are you sure you want to delete the sequential capture step: " + Name + "?";
            MessageBoxResult result = MessageBox.Show(message, "Delete Sequential Capture Step", MessageBoxButton.YesNo);
            if (MessageBoxResult.Yes == result)
            {
                MVMManager.Instance["SequentialControlViewModel", "RemoveSequenceStep"] = SequenceLineNumber - 1;
            }
        }

        public void DeleteSequenceStepTemplate()
        {
            string message = "Are you sure you want to delete the Sequence Step template: " + Name + "?";
            MessageBoxResult result = MessageBox.Show(message, "Delete Sequence Step Template", MessageBoxButton.YesNo);
            if (MessageBoxResult.Yes == result)
            {
                MVMManager.Instance["SequentialControlViewModel", "RemoveStepTemplate"] = TemplateLineNumber;
            }
        }

        public void GetCameraSettings(ref int channel, ref double exposure)
        {
            if (null != _templateSequenceStepNode)
            {
                XmlNodeList cameraNdList = _templateSequenceStepNode.SelectNodes("Camera");
                if (0 < cameraNdList.Count)
                {
                    string str = string.Empty;
                    if (GetAttribute(cameraNdList[0], "channel", ref str))
                    {
                        int val;
                        if (Int32.TryParse(str, out val))
                        {
                            channel = val;
                        }
                    }
                    if (GetAttribute(cameraNdList[0], "exposure", ref str))
                    {
                        double val;
                        if (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out val))
                        {
                            exposure = val;
                        }
                    }
                }
            }
        }

        public void GetLightPath(ref int Mirror1Enable, ref int Mirror2Enable, ref int Mirror3Enable)
        {
            XmlNodeList lightPathNdList = _templateSequenceStepNode.SelectNodes("LightPath");

            if (0 < lightPathNdList.Count)
            {
                string str = string.Empty;
                Mirror1Enable = 1; Mirror2Enable = 0; Mirror3Enable = 0;
                if (GetAttribute(lightPathNdList[0], "InvertedLightPathPos", ref str) && "-1" != str)
                {
                    SequenceStepScopeType = 1;
                    switch (str)
                    {
                        case "0":
                            break;
                        case "1":
                            Mirror1Enable = 0;
                            Mirror2Enable = 1;
                            break;
                        case "2":
                            Mirror3Enable = 1;
                            Mirror1Enable = 0;
                            break;
                    }
                }
                else
                {
                    SequenceStepScopeType = 0;
                    if (GetAttribute(lightPathNdList[0], "GalvoGalvo", ref str))
                    {
                        int tmp;
                        if (Int32.TryParse(str, out tmp))
                        {
                            Mirror1Enable = tmp;
                        }
                    }
                    if (GetAttribute(lightPathNdList[0], "GalvoResonance", ref str))
                    {
                        int tmp;
                        if (Int32.TryParse(str, out tmp))
                        {
                            Mirror2Enable = tmp;
                        }
                    }
                    if (GetAttribute(lightPathNdList[0], "Camera", ref str))
                    {
                        int tmp;
                        if (Int32.TryParse(str, out tmp))
                        {
                            Mirror3Enable = tmp;
                        }
                    }
                }
            }
        }

        public void GetPockels(ref double[] powerLevel, ref int[] type, ref string[] rampName)
        {
            const int MAX_POCKELS = 4;
            if (null != _templateSequenceStepNode)
            {
                powerLevel = new double[MAX_POCKELS];
                type = new int[MAX_POCKELS];
                rampName = new string[MAX_POCKELS];

                XmlNodeList pockelsNdList = _templateSequenceStepNode.SelectNodes("Pockels");

                for (int i = 0; i < Math.Min(pockelsNdList.Count, MAX_POCKELS); i++)
                {
                    string str = string.Empty;
                    {
                        if (GetAttribute(pockelsNdList[i], "start", ref str))
                        {
                            double tmp;
                            if (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmp))
                            {
                                powerLevel[i] = tmp;
                            }
                        }

                        if (GetAttribute(pockelsNdList[i], "type", ref str))
                        {
                            int tmp;
                            if (Int32.TryParse(str, out tmp))
                            {
                                type[i] = tmp;
                            }
                        }

                        if (GetAttribute(pockelsNdList[i], "path", ref str))
                        {
                            rampName[i] = Path.GetFileNameWithoutExtension(str);
                        }
                        else
                        {
                            type[i] = 0;
                        }
                    }
                }
            }
        }

        public void GetSequenceEpiTurretPosition(ref int position, ref string posName)
        {
            XmlNodeList epiTurretNdList = _templateSequenceStepNode.SelectNodes("EPITurret");

            if (0 < epiTurretNdList.Count)
            {
                string str = string.Empty;
                if (GetAttribute(epiTurretNdList[0], "position", ref str))
                {
                    int tmp;
                    if (Int32.TryParse(str, out tmp))
                    {
                        position = tmp;
                    }
                }
                if (GetAttribute(epiTurretNdList[0], "positionName", ref str))
                {
                    posName = str;
                }

            }
        }

        public void GetSequenceStepChannelColors(ref Color[] channelColor, ref string[] channelNames)
        {
            Color[] getColors = new Color[MAX_CHANNELS];
            string[] chanName = new string[MAX_CHANNELS];
            if (null != _templateSequenceStepNode)
            {
                XmlNodeList wavNdList = _templateSequenceStepNode.SelectNodes("Wavelengths/Wavelength");
                if (0 < wavNdList.Count)
                {
                    string str = string.Empty;
                    int val;
                    foreach (XmlNode wavelength in wavNdList)
                    {
                        if (GetAttribute(wavelength, "channelIndex", ref str) && Int32.TryParse(str, out val))
                        {
                            if (GetAttribute(wavelength, "color", ref str))
                            {
                                getColors[val] = (Color)ColorConverter.ConvertFromString(str);
                            }
                            if (GetAttribute(wavelength, "name", ref str))
                            {
                                if (str.ToLower().Contains("chan"))
                                {
                                    var x = str.Substring(str.ToLower().IndexOf("chan") + 4);
                                    chanName[val] = x;
                                }
                                else
                                {
                                    chanName[val] = str;
                                }
                            }
                        }
                    }
                }
            }
            channelColor = getColors;
            channelNames = chanName;
        }

        public void GetSequenceStepDigitalIO(ref int enable, ref int[] switchPositions)
        {
            if (null != _templateSequenceStepNode)
            {
                XmlNodeList digitalNdList = _templateSequenceStepNode.SelectNodes("DigitalIO");
                if (0 < digitalNdList.Count)
                {
                    string str = string.Empty;
                    if (GetAttribute(digitalNdList[0], "enable", ref str))
                    {
                        Int32.TryParse(str, out enable);
                    }
                    if (GetAttribute(digitalNdList[0], "digOut1", ref str))
                    {
                        Int32.TryParse(str, out switchPositions[0]);
                    }
                    if (GetAttribute(digitalNdList[0], "digOut2", ref str))
                    {
                        Int32.TryParse(str, out switchPositions[1]);
                    }
                    if (GetAttribute(digitalNdList[0], "digOut3", ref str))
                    {
                        Int32.TryParse(str, out switchPositions[2]);
                    }
                    if (GetAttribute(digitalNdList[0], "digOut4", ref str))
                    {
                        Int32.TryParse(str, out switchPositions[3]);
                    }
                    if (GetAttribute(digitalNdList[0], "digOut5", ref str))
                    {
                        Int32.TryParse(str, out switchPositions[4]);
                    }
                    if (GetAttribute(digitalNdList[0], "digOut6", ref str))
                    {
                        Int32.TryParse(str, out switchPositions[5]);
                    }
                    if (GetAttribute(digitalNdList[0], "digOut7", ref str))
                    {
                        Int32.TryParse(str, out switchPositions[6]);
                    }
                    if (GetAttribute(digitalNdList[0], "digOut8", ref str))
                    {
                        Int32.TryParse(str, out switchPositions[7]);
                    }
                }
            }
        }

        public void GetSequenceStepLSMChannel(ref bool chanEnable0, ref bool chanEnable1, ref bool chanEnable2, ref bool chanEnable3)
        {
            if (null != _templateSequenceStepNode)
            {
                XmlNodeList lsmNdList = (ResourceManagerCS.GetCameraType() == (int)ICamera.CameraType.LSM) ? _templateSequenceStepNode.SelectNodes("LSM") : _templateSequenceStepNode.SelectNodes("Camera");
                if (0 < lsmNdList.Count)
                {
                    string str = string.Empty;
                    if (GetAttribute(lsmNdList[0], "channel", ref str))
                    {
                        int val = 0;
                        if (Int32.TryParse(str, out val))
                        {
                            ConvertBinaryToChanEnable(val, ref chanEnable0, ref chanEnable1, ref chanEnable2, ref chanEnable3);
                        }
                    }
                }
            }
        }

        public void GetSequenceStepMCLS(ref int mainLaserSel, ref int laser1Enable, ref double laser1Power, ref double laser1PowerPercent,
            ref int laser2Enable, ref double laser2Power, ref double laser2PowerPercent,
            ref int laser3Enable, ref double laser3Power, ref double laser3PowerPercent,
            ref int laser4Enable, ref double laser4Power, ref double laser4PowerPercent, ref int laserTTL, ref int laserAnalog,
            ref int laser1Wavelength, ref int laser2Wavelength, ref int laser3Wavelength, ref int laser4Wavelength)
        {
            if (null != _templateSequenceStepNode)
            {
                XmlNodeList mclsNdList = _templateSequenceStepNode.SelectNodes("MCLS");
                if (0 < mclsNdList.Count)
                {
                    string str = string.Empty;
                    if (GetAttribute(mclsNdList[0], "MainLaserSelection", ref str))
                    {
                        Int32.TryParse(str, out mainLaserSel);
                    }
                    if (GetAttribute(mclsNdList[0], "enable1", ref str))
                    {
                        Int32.TryParse(str, out laser1Enable);
                    }
                    if (GetAttribute(mclsNdList[0], "power1", ref str))
                    {
                        double tmp = 0;
                        if (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmp))
                        {
                            laser1Power = tmp;
                        }
                    }
                    if (GetAttribute(mclsNdList[0], "power1percent", ref str))
                    {
                        Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out laser1PowerPercent);
                    }
                    if (GetAttribute(mclsNdList[0], "enable2", ref str))
                    {
                        Int32.TryParse(str, out laser2Enable);
                    }
                    if (GetAttribute(mclsNdList[0], "power2", ref str))
                    {
                        double tmp = 0;
                        if (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmp))
                        {
                            laser2Power = tmp;
                        }
                    }
                    if (GetAttribute(mclsNdList[0], "power2percent", ref str))
                    {
                        Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out laser2PowerPercent);
                    }
                    if (GetAttribute(mclsNdList[0], "enable3", ref str))
                    {
                        Int32.TryParse(str, out laser3Enable);
                    }
                    if (GetAttribute(mclsNdList[0], "power3", ref str))
                    {
                        double tmp = 0;
                        if (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmp))
                        {
                            laser3Power = tmp;
                        }
                    }
                    if (GetAttribute(mclsNdList[0], "power3percent", ref str))
                    {
                        Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out laser3PowerPercent);
                    }
                    if (GetAttribute(mclsNdList[0], "enable4", ref str))
                    {
                        Int32.TryParse(str, out laser4Enable);
                    }
                    if (GetAttribute(mclsNdList[0], "power4", ref str))
                    {
                        double tmp = 0;
                        if (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmp))
                        {
                            laser4Power = tmp;
                        }
                    }
                    if (GetAttribute(mclsNdList[0], "power4percent", ref str))
                    {
                        Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out laser4PowerPercent);
                    }
                    if (GetAttribute(mclsNdList[0], "allttl", ref str))
                    {
                        Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out laserTTL);
                    }
                    if (GetAttribute(mclsNdList[0], "allanalog", ref str))
                    {
                        Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out laserAnalog);
                    }

                    //Used to determine whether wavelength should be displayed when hovering over sequential capture step
                    if (GetAttribute(mclsNdList[0], "wavelength1", ref str))
                    {
                        Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out laser1Wavelength);
                    }
                    if (GetAttribute(mclsNdList[0], "wavelength2", ref str))
                    {
                        Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out laser2Wavelength);
                    }
                    if (GetAttribute(mclsNdList[0], "wavelength3", ref str))
                    {
                        Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out laser3Wavelength);
                    }
                    if (GetAttribute(mclsNdList[0], "wavelength4", ref str))
                    {
                        Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out laser4Wavelength);
                    }
                }
            }
        }

        public void GetSequenceStepMultiphoton(ref int laserPostion)
        {
            if (null != _templateSequenceStepNode)
            {
                XmlNodeList multiphotonNdList = _templateSequenceStepNode.SelectNodes("MultiPhotonLaser");
                if (0 < multiphotonNdList.Count)
                {
                    string str = string.Empty;
                    if (GetAttribute(multiphotonNdList[0], "pos", ref str))
                    {
                        Int32.TryParse(str, out laserPostion);
                    }
                }
            }
        }

        public void GetSequenceStepPinhole(ref int pinholePosition, ref double pinholeMicrometers, ref double pinholeADUs)
        {
            if (null != _templateSequenceStepNode)
            {
                XmlNodeList pinholeNdList = _templateSequenceStepNode.SelectNodes("PinholeWheel");
                if (0 < pinholeNdList.Count)
                {
                    string str = string.Empty;
                    if (GetAttribute(pinholeNdList[0], "position", ref str))
                    {
                        Int32.TryParse(str, out pinholePosition);
                    }

                    if (GetAttribute(pinholeNdList[0], "micrometers", ref str))
                    {
                        Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out pinholeMicrometers);
                    }

                    if (GetAttribute(pinholeNdList[0], "adu", ref str))
                    {
                        Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out pinholeADUs);
                    }
                }
            }
        }

        public void GetSequenceStepPMT(ref double pmt1Gain, ref double pmt2Gain, ref double pmt3Gain, ref double pmt4Gain)
        {
            if (null != _templateSequenceStepNode)
            {
                XmlNodeList pmtNdList = _templateSequenceStepNode.SelectNodes("PMT");
                if (0 < pmtNdList.Count)
                {
                    string str = string.Empty;
                    if (GetAttribute(pmtNdList[0], "gainA", ref str))
                    {
                        Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out pmt1Gain);
                    }
                    if (GetAttribute(pmtNdList[0], "gainB", ref str))
                    {
                        Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out pmt2Gain);
                    }
                    if (GetAttribute(pmtNdList[0], "gainC", ref str))
                    {
                        Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out pmt3Gain);
                    }
                    if (GetAttribute(pmtNdList[0], "gainD", ref str))
                    {
                        Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out pmt4Gain);
                    }
                }
            }
        }

        public void ModifySequenceName()
        {
            SequenceStepNameEdit dlg;
            bool repeatedName;

            ObservableCollection<SequenceStep> stepTemplates = (ObservableCollection<SequenceStep>)MVMManager.Instance["SequentialControlViewModel", "CollectionSequences", null];
            if (null == stepTemplates)
            {
                return;
            }

            do
            {
                repeatedName = false;
                dlg = new SequenceStepNameEdit();
                dlg.SequenceStepName = Name;
                if (false == dlg.ShowDialog())
                {
                    return;
                }

                for (int i = 0; i < stepTemplates.Count; i++)
                {
                    if (stepTemplates[i].Name == dlg.SequenceStepName)
                    {
                        MessageBox.Show("The name for the new Sequence Step template must be unique. Please use a different name.");
                        repeatedName = true;
                        break;
                    }
                }
            } while (repeatedName);

            string oldName = Name;
            Name = dlg.SequenceStepName;
            string[] nameUpdate = new string[2];
            nameUpdate[0] = oldName;
            nameUpdate[1] = dlg.SequenceStepName;
            MVMManager.Instance["SequentialControlViewModel", "UpdateTemplateName"] = nameUpdate;
        }

        public override string ToString()
        {
            return Name;
        }

        public void UpdateList()
        {
            string message = "Are you sure you want to update the Sequence Step template: " + Name + "?";
            MessageBoxResult result = MessageBox.Show(message, "Update Sequence Step Template", MessageBoxButton.YesNo);
            if (MessageBoxResult.Yes == result)
            {
                MVMManager.Instance["SequentialControlViewModel", "UpdateStepTemplate"] = TemplateLineNumber;
            }
        }

        private void ConvertBinaryToChanEnable(int binaryValue, ref bool chan0Enable, ref bool chan1Enable, ref bool chan2Enable, ref bool chan3Enable)
        {
            chan0Enable = Convert.ToBoolean(binaryValue & 0x1);
            chan1Enable = Convert.ToBoolean(binaryValue & 0x2);
            chan2Enable = Convert.ToBoolean(binaryValue & 0x4);
            chan3Enable = Convert.ToBoolean(binaryValue & 0x8);
        }

        private bool GetAttribute(XmlNode node, string attrName, ref string attrValue)
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

        private void LoadSequenceStep()
        {
            //Load and set Channel Enable form LSM
            bool chanEnable0 = false, chanEnable1 = false, chanEnable2 = false, chanEnable3 = false;
            GetSequenceStepLSMChannel(ref chanEnable0, ref chanEnable1, ref chanEnable2, ref chanEnable3);
            MVMManager.Instance["ScanControlViewModel", "LSMChannelEnable0"] = chanEnable0;
            MVMManager.Instance["ScanControlViewModel", "LSMChannelEnable1"] = chanEnable1;
            MVMManager.Instance["ScanControlViewModel", "LSMChannelEnable2"] = chanEnable2;
            MVMManager.Instance["ScanControlViewModel", "LSMChannelEnable3"] = chanEnable3;

            //Load and set MCLS Laser settings
            int mainLaserSelection = -1;
            int laser1Enable = 0, laser2Enable = 0, laser3Enable = 0, laser4Enable = 0;
            double laser1Power = 0.0, laser1Percent = 0.0;
            double laser2Power = 0.0, laser2Percent = 0.0;
            double laser3Power = 0.0, laser3Percent = 0.0;
            double laser4Power = 0.0, laser4Percent = 0.0;
            int laserTTL = 0, laserAnalog = 0;
            int laser1Wavelength = 0, laser2Wavelength = 0, laser3Wavelength = 0, laser4Wavelength = 0;
            GetSequenceStepMCLS(ref mainLaserSelection, ref laser1Enable, ref laser1Power, ref laser1Percent, ref laser2Enable, ref laser2Power, ref laser2Percent,
                ref laser3Enable, ref laser3Power, ref laser3Percent, ref laser4Enable, ref laser4Power, ref laser4Percent, ref laserTTL, ref laserAnalog, ref laser1Wavelength, ref laser2Wavelength, ref laser3Wavelength,
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
            MVMManager.Instance["MultiLaserControlViewModel", "LaserAllAnalog"] = laserAnalog;
            MVMManager.Instance["MultiLaserControlViewModel", "LaserAllTTL"] = laserTTL;

            //Load and set Multiphoton Laser Settings
            int laserPosition = 0;
            GetSequenceStepMultiphoton(ref laserPosition);
            MVMManager.Instance["MultiphotonControlViewModel", "Laser1Position"] = laserPosition;

            //Load and set PMT Settings
            double pmt1Gain = 0, pmt2Gain = 0, pmt3Gain = 0, pmt4Gain = 0;
            GetSequenceStepPMT(ref pmt1Gain, ref pmt2Gain, ref pmt3Gain, ref pmt4Gain);
            ((HwVal<double>)MVMManager.Instance["ScanControlViewModel", "PMTGain", 0]).Value = pmt1Gain;
            ((HwVal<double>)MVMManager.Instance["ScanControlViewModel", "PMTGain", 1]).Value = pmt2Gain;
            ((HwVal<double>)MVMManager.Instance["ScanControlViewModel", "PMTGain", 2]).Value = pmt3Gain;
            ((HwVal<double>)MVMManager.Instance["ScanControlViewModel", "PMTGain", 3]).Value = pmt4Gain;

            //Load and set PinholeWheel Settings
            int pinholePosition = 0;
            double pinholeMicrometers = 0.0, pinholeADUs = 0.0;
            GetSequenceStepPinhole(ref pinholePosition, ref pinholeMicrometers, ref pinholeADUs);
            MVMManager.Instance["PinholeControlViewModel", "PinholePosition"] = pinholePosition;

            //Load Pockels Settings
            const int MAX_POCKELS = 4;
            double[] pockelsPowerLevel = new double[MAX_POCKELS];
            int[] powerType = new int[MAX_POCKELS];
            string[] powerRampName = new string[MAX_POCKELS];
            GetPockels(ref pockelsPowerLevel, ref powerType, ref powerRampName);

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
            GetLightPath(ref M1Enable, ref M2Enable, ref M3Enable);
            if ((int)ScopeType.INVERTED == (int)MVMManager.Instance["LightPathControlViewModel", "LightPathScopeType"])
            {
                MVMManager.Instance["LightPathControlViewModel", "InvertedLpLeftEnable"] = (M1Enable == 1);
                MVMManager.Instance["LightPathControlViewModel", "InvertedLpCenterEnable"] = (M2Enable == 1);
                MVMManager.Instance["LightPathControlViewModel", "InvertedLpRightEnable"] = (M3Enable == 1);
            }
            else
            {
                MVMManager.Instance["LightPathControlViewModel", "LightPathGGEnable"] = M1Enable;
                MVMManager.Instance["LightPathControlViewModel", "LightPathGREnable"] = M2Enable;
                MVMManager.Instance["LightPathControlViewModel", "LightPathCamEnable"] = M3Enable;
            }

            //Load Digital Switches Settings
            int enable = 0;
            int[] switchPositions = new int[(int)Constants.MAX_SWITCHES];
            GetSequenceStepDigitalIO(ref enable, ref switchPositions);
            MVMManager.Instance["DigitalOutputSwitchesViewModel", "SwitchEnable"] = enable;
            ObservableCollection<IntPC> switchState = new ObservableCollection<IntPC>();

            for (int i = 0; i < (int)Constants.MAX_SWITCHES; i++)
            {
                switchState.Add(new IntPC());
                switchState[i].Value = switchPositions[i];
            }

            MVMManager.Instance["DigitalOutputSwitchesViewModel", "SwitchState"] = switchState;
            ((IMVM)MVMManager.Instance["DigitalOutputSwitchesViewModel", this]).OnPropertyChange("SwitchState");
            ICommand digitalSwitchCommand = (ICommand)MVMManager.Instance["DigitalOutputSwitchesViewModel", "DigitalSwitchCommand", (object)null];
            for (int i = 0; i < (int)Constants.MAX_SWITCHES; i++)
            {
                digitalSwitchCommand.Execute(i);
            }

            //Load Camera Settings
            if (ResourceManagerCS.GetCameraType() != (int)ICamera.CameraType.LSM)
            {
                int channels = 0;
                double exposure = 0;
                GetCameraSettings(ref channels, ref exposure);
                if (0 < exposure)
                {
                    MVMManager.Instance["CameraControlViewModel", "ExposureTimeCam"] = exposure;
                }
            }

            //Load Epi Turret Settings
            int epiPos = 0;
            string epiPosName = string.Empty;
            GetSequenceEpiTurretPosition(ref epiPos, ref epiPosName);
            if (epiPos > 0 && epiPos < 7)
            {
                MVMManager.Instance["EpiTurretControlViewModel", "EpiTurretPos"] = epiPos - 1;
            }

            OnPropertyChanged("SequenceStepVisibility");
        }

        private void OnPropertyChanged(string propertyName)
        {
            if (PropertyChanged != null)
                PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
        }

        #endregion Methods
    }
}