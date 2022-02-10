namespace CaptureSetupDll.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Reflection;
    using System.Resources;
    using System.Runtime.InteropServices;
    using System.Windows;
    using System.Windows.Data;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Threading;
    using System.Xml;

    using CaptureSetupDll.Model;

    public class LightPathSequenceStep : INotifyPropertyChanged
    {
        #region Fields

        CaptureSetup.DigitizerBoardNames _digitizerBoardName = CaptureSetup.DigitizerBoardNames.ATS9440;
        private int _lightPathLineNumber = 0;
        private XmlNode _lightPathSequenceStepNode = null;
        private string _name = string.Empty;
        private int _sequenceLineNumber = 0;

        #endregion Fields

        #region Constructors

        public LightPathSequenceStep(string str, XmlNode lightPathSequenceStepNode, int lineNumber, CaptureSetup.DigitizerBoardNames digitizerBoardName)
        {
            _name = str;
            _lightPathSequenceStepNode = lightPathSequenceStepNode;
            _lightPathLineNumber = lineNumber;
            _sequenceLineNumber = lineNumber;
            _digitizerBoardName = digitizerBoardName;
        }

        #endregion Constructors

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public int LightPathLineNumber
        {
            get
            {
                return _lightPathLineNumber;
            }
            set
            {
                if (_lightPathLineNumber != value)
                {
                    _lightPathLineNumber = value;
                    OnPropertyChanged("LightPathLineNumber");
                }
            }
        }

        public XmlNode LightPathSequenceStepNode
        {
            get
            {
                return _lightPathSequenceStepNode;
            }
            set
            {
                _lightPathSequenceStepNode = value;
            }
        }

        public LightPathSequenceStepView LightPathSequenceStepPreview
        {
            get
            {
                return new LightPathSequenceStepView(this);
            }
        }

        public string Name
        {
            get { return _name; }

            set
            {
                if (_name != value)
                {
                    _name = value;
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
                    _sequenceLineNumber = value;
                    OnPropertyChanged("SequenceLineNumber");
                }
            }
        }

        public long SequenceStepScopeType
        {
            get;
            set;
        }

        #endregion Properties

        #region Methods

        public void GetLightPath(ref int Mirror1Enable, ref int Mirror2Enable, ref int Mirror3Enable)
        {
            XmlNodeList lightPathNdList = _lightPathSequenceStepNode.SelectNodes("LightPath");

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

        public void GetLightPathSequenceStepLSMChannel(ref bool chanEnable0, ref bool chanEnable1, ref bool chanEnable2, ref bool chanEnable3)
        {
            if (null != _lightPathSequenceStepNode)
            {
                XmlNodeList lsmNdList = _lightPathSequenceStepNode.SelectNodes("LSM");
                if (0 < lsmNdList.Count)
                {
                    string str = string.Empty;
                    if (GetAttribute(lsmNdList[0], "channel", ref str))
                    {
                        int val = 0;
                        if (Int32.TryParse(str, out val))
                        {
                            ConvertBinaryToChanEnable(val, _digitizerBoardName, ref chanEnable0, ref chanEnable1, ref chanEnable2, ref chanEnable3);
                        }
                    }
                }
            }
        }

        public void GetLightPathSequenceStepMCLS(ref int mainLaserSel, ref int laser1Enable, ref double laser1Power, ref double laser1PowerPercent,
            ref int laser2Enable, ref double laser2Power, ref double laser2PowerPercent,
            ref int laser3Enable, ref double laser3Power, ref double laser3PowerPercent,
            ref int laser4Enable, ref double laser4Power, ref double laser4PowerPercent,
            ref int laser1Wavelength, ref int laser2Wavelength, ref int laser3Wavelength, ref int laser4Wavelength)
        {
            if (null != _lightPathSequenceStepNode)
            {
                XmlNodeList mclsNdList = _lightPathSequenceStepNode.SelectNodes("MCLS");
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

                    //Used to determine whether wavelength should be displayed when hovering over sequential capture Light Path
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

        public void GetLightPathSequenceStepMultiphoton(ref int laserPostion)
        {
            if (null != _lightPathSequenceStepNode)
            {
                XmlNodeList multiphotonNdList = _lightPathSequenceStepNode.SelectNodes("MultiPhotonLaser");
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

        public void GetLightPathSequenceStepPinhole(ref int pinholePosition, ref double pinholeMicrometers, ref double pinholeADUs)
        {
            if (null != _lightPathSequenceStepNode)
            {
                XmlNodeList pinholeNdList = _lightPathSequenceStepNode.SelectNodes("PinholeWheel");
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

        public void GetLightPathSequenceStepPMT(ref int pmt1Gain, ref int pmt2Gain, ref int pmt3Gain, ref int pmt4Gain)
        {
            if (null != _lightPathSequenceStepNode)
            {
                XmlNodeList pmtNdList = _lightPathSequenceStepNode.SelectNodes("PMT");
                if (0 < pmtNdList.Count)
                {
                    string str = string.Empty;
                    if (GetAttribute(pmtNdList[0], "gainA", ref str))
                    {
                        Int32.TryParse(str, out pmt1Gain);
                    }
                    if (GetAttribute(pmtNdList[0], "gainB", ref str))
                    {
                        Int32.TryParse(str, out pmt2Gain);
                    }
                    if (GetAttribute(pmtNdList[0], "gainC", ref str))
                    {
                        Int32.TryParse(str, out pmt3Gain);
                    }
                    if (GetAttribute(pmtNdList[0], "gainD", ref str))
                    {
                        Int32.TryParse(str, out pmt4Gain);
                    }
                }
            }
        }

        public void GetPockels(ref double[] powerLevel, ref int[] type, ref string[] rampName)
        {
            const int MAX_POCKELS = 4;
            if (null != _lightPathSequenceStepNode)
            {
                powerLevel = new double[MAX_POCKELS];
                type = new int[MAX_POCKELS];
                rampName = new string[MAX_POCKELS];

                XmlNodeList pockelsNdList = _lightPathSequenceStepNode.SelectNodes("Pockels");

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

        public override string ToString()
        {
            return Name;
        }

        private void ConvertBinaryToChanEnable(int binaryValue, CaptureSetup.DigitizerBoardNames digitizerBoardName, ref bool chan0Enable, ref bool chan1Enable, ref bool chan2Enable, ref bool chan3Enable)
        {
            switch (digitizerBoardName)
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

        private void OnPropertyChanged(string propertyName)
        {
            if (PropertyChanged != null)
                PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
        }

        #endregion Methods
    }
}