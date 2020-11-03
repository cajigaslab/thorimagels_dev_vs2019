namespace KuriosControl.ViewModel
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
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Input;
    using System.Xml;

    using KuriosControl;
    using KuriosControl.Model;

    using ThorLogging;

    using ThorSharedTypes;

    class KuriosControlViewModel : ThorSharedTypes.VMBase, ThorSharedTypes.IMVM
    {
        #region Fields

        private KuriosControlModel _kuriosControlModel;
        private Dictionary<string, PropertyInfo> _properties = new Dictionary<string, PropertyInfo>();

        #endregion Fields

        #region Constructors

        public KuriosControlViewModel()
        {
            _kuriosControlModel = new KuriosControlModel();
        }

        #endregion Constructors

        #region Properties

        public double ExposureTimeCam
        {
            get
            {
                return (double)MVMManager.Instance["CameraControlViewModel", "ExposureTimeCam", (object)1.0];
            }
            set
            {
                
                OnPropertyChanged("ExposureTimeCam");
            }
        }

        public double ExposureTimeMax
        {
            get
            {
                double temp = (double)MVMManager.Instance["CameraControlViewModel", "ExposureTimeMax", (object)1.0];
                return temp;
            }
        }

        public double ExposureTimeMin
        {
            get
            {
                return (double)MVMManager.Instance["CameraControlViewModel", "ExposureTimeMin", (object)1.0];
            }
        }

        public int KuriosBandwidthMode
        {
            get
            {
                return _kuriosControlModel.KuriosBandwidthMode;
            }
            set
            {
                _kuriosControlModel.KuriosBandwidthMode = value;
                OnPropertyChanged("KuriosBandwidthMode");
                OnPropertyChanged("KuriosBandwidthModeIndex");
            }
        }

        public int KuriosBandwidthModeIndex
        {
            get
            {
                switch (this.KuriosBandwidthMode)
                {
                    case 2: return 0;
                    case 4: return 1;
                    case 8: return 2;
                    default: return 0;
                }
            }
            set
            {
                switch (value)
                {
                    case 0: this.KuriosBandwidthMode = 2; break;
                    case 1: this.KuriosBandwidthMode = 4; break;
                    case 2: this.KuriosBandwidthMode = 8; break;
                }
                OnPropertyChanged("KuriosBandwidthModeIndex");
            }
        }

        public int KuriosControlMode
        {
            get
            {
                return _kuriosControlModel.KuriosControlMode;
            }
            set
            {
                _kuriosControlModel.KuriosControlMode = value;
            }
        }

        public string KuriosCurrentWavelengthSequenceName
        {
            get
            {
                return _kuriosControlModel.KuriosCurrentWavelengthSequenceName;
            }
            set
            {
                _kuriosControlModel.KuriosCurrentWavelengthSequenceName = value;
                OnPropertyChanged("KuriosCurrentWavelengthSequenceName");
            }
        }

        public int KuriosStartWL
        {
            get
            {
                return _kuriosControlModel.KuriosStartWL;
            }
            set
            {
                _kuriosControlModel.KuriosStartWL = value;
                OnPropertyChanged("KuriosStartWL");
                OnPropertyChanged("KuriousSequenceSteps");
            }
        }

        public int KuriosStepSizeWL
        {
            get
            {
                return _kuriosControlModel.KuriosStepSizeWL;
            }
            set
            {
                _kuriosControlModel.KuriosStepSizeWL = value;
                OnPropertyChanged("KuriosStepSizeWL");
                OnPropertyChanged("KuriousSequenceSteps");
            }
        }

        public int KuriosStopWL
        {
            get
            {
                return _kuriosControlModel.KuriosStopWL;
            }
            set
            {
                _kuriosControlModel.KuriosStopWL = value;
                OnPropertyChanged("KuriosStopWL");
                OnPropertyChanged("KuriousSequenceSteps");
            }
        }

        public int KuriosWavelength
        {
            get
            {
                return _kuriosControlModel.KuriosWavelength;
            }
            set
            {
                _kuriosControlModel.KuriosWavelength = value;
                OnPropertyChanged("KuriosWavelength");
            }
        }

        public int KuriosWavelengthMax
        {
            get
            {
                return _kuriosControlModel.KuriosWavelengthMax;
            }
        }

        public int KuriosWavelengthMin
        {
            get
            {
                return _kuriosControlModel.KuriosWavelengthMin;
            }
        }

        public int KuriousSequenceSteps
        {
            get
            {
                return _kuriosControlModel.KuriousSequenceSteps;
            }
            set
            {
                _kuriosControlModel.KuriousSequenceSteps = value;
                OnPropertyChanged("KuriousSequenceSteps");
            }
        }

        #endregion Properties

        #region Indexers

        public object this[string propertyName, object defaultObject = null]
        {
            get
            {
                PropertyInfo myPropInfo = typeof(KuriosControlViewModel).GetProperty(propertyName);
                return (null != myPropInfo) ? myPropInfo.GetValue(this) : defaultObject;
            }
            set
            {
                PropertyInfo myPropInfo = typeof(KuriosControlViewModel).GetProperty(propertyName);
                if (null != myPropInfo)
                {
                    myPropInfo.SetValue(this, value);
                }
            }
        }

        public object this[string propertyName, int index, object defaultObject = null]
        {
            get
            {
                PropertyInfo myPropInfo = GetPropertyInfo(propertyName);
                if (null != myPropInfo)
                {
                    if (typeof(CustomCollection<>) == myPropInfo.PropertyType.GetGenericTypeDefinition())
                    {
                        var collection = myPropInfo.GetValue(this, null);
                        return collection.GetType().GetProperty("Item").GetValue(collection, new object[] { index });
                    }
                    else
                    {
                        return myPropInfo.GetValue(this, null);
                    }
                }
                return defaultObject;
            }
            set
            {
                PropertyInfo myPropInfo = GetPropertyInfo(propertyName);
                if (null != myPropInfo)
                {
                    if (typeof(CustomCollection<>) == myPropInfo.PropertyType.GetGenericTypeDefinition())
                    {
                        var collection = myPropInfo.GetValue(this, null);
                        collection.GetType().GetProperty("Item").SetValue(collection, value, new object[] { index });
                    }
                    else
                    {
                        myPropInfo.SetValue(this, value, null);
                    }
                }
            }
        }

        #endregion Indexers

        #region Methods

        public PropertyInfo GetPropertyInfo(string propertyName)
        {
            PropertyInfo myPropInfo = null;
            if (!_properties.TryGetValue(propertyName, out myPropInfo))
            {
                myPropInfo = typeof(KuriosControlViewModel).GetProperty(propertyName);
                if (null != myPropInfo)
                {
                    _properties.Add(propertyName, myPropInfo);
                }
            }
            return myPropInfo;
        }

        public void LoadXMLSettings()
        {
            XmlDocument expDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS];
            XmlNodeList ndList = expDoc.SelectNodes("/ThorImageExperiment/SpectralFilter");
            if (ndList.Count > 0)
            {
                string str = string.Empty;

                if (XmlManager.GetAttribute(ndList[0], expDoc, "wavelengthStart", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        KuriosStartWL = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], expDoc, "wavelengthStop", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        KuriosStopWL = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], expDoc, "wavelengthStepSize", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        KuriosStepSizeWL = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], expDoc, "controlMode", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        KuriosControlMode = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], expDoc, "bandwidthMode", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        KuriosBandwidthMode = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], expDoc, "path", ref str))
                {
                    KuriosCurrentWavelengthSequenceName = Path.GetFileNameWithoutExtension(str);
                }
            }
        }

        public void OnPropertyChange(string propertyName)
        {
        }

        public void UpdateExpXMLSettings(ref XmlDocument xmlDoc)
        {
            ///Spectral Filter
            XmlNodeList ndList = xmlDoc.SelectNodes("/ThorImageExperiment/SpectralFilter");
            if (ndList.Count <= 0)
            {
                CreateXmlNode(xmlDoc, "SpectralFilter");
                ndList = xmlDoc.SelectNodes("/ThorImageExperiment/SpectralFilter");
            }

            if (ndList.Count > 0)
            {
                XmlManager.SetAttribute(ndList[0], xmlDoc, "wavelengthStart", KuriosStartWL.ToString());
                XmlManager.SetAttribute(ndList[0], xmlDoc, "wavelengthStop", KuriosStopWL.ToString());
                XmlManager.SetAttribute(ndList[0], xmlDoc, "wavelengthStepSize", KuriosStepSizeWL.ToString());

                //the number of steps should always match the number of wavelengths in the link file
                XmlManager.SetAttribute(ndList[0], xmlDoc, "steps", KuriousSequenceSteps.ToString());

                XmlManager.SetAttribute(ndList[0], xmlDoc, "controlMode", KuriosControlMode.ToString());
                XmlManager.SetAttribute(ndList[0], xmlDoc, "bandwidthMode", KuriosBandwidthMode.ToString());
                XmlManager.SetAttribute(ndList[0], xmlDoc, "path", Application.Current.Resources["ApplicationSettingsFolder"].ToString() + "\\kurios\\" + KuriosCurrentWavelengthSequenceName + ".txt");
            }
        }

        private void CreateXmlNode(XmlDocument doc, string nodeName)
        {
            XmlNode node = doc.CreateNode(XmlNodeType.Element, nodeName, null);
            doc.DocumentElement.AppendChild(node);
        }

        #endregion Methods
    }
}