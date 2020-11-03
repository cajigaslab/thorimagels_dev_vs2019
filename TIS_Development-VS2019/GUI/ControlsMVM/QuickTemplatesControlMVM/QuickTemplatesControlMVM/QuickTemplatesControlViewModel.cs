namespace QuickTemplatesControl.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Reflection;
    using System.Runtime.CompilerServices;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Input;
    using System.Xml;

    using OverlayManager;

    using QuickTemplatesControl.Model;

    using ThorLogging;

    using ThorSharedTypes;

    public class QuickTemplatesControlViewModel : ThorSharedTypes.VMBase, ThorSharedTypes.IMVM
    {
        #region Fields

        private Dictionary<string, PropertyInfo> _properties = new Dictionary<string, PropertyInfo>();
        private List<QuickConfig> _quickConfigCollection;

        #endregion Fields

        #region Constructors

        public QuickTemplatesControlViewModel()
        {
            _quickConfigCollection = new List<QuickConfig>();
        }

        #endregion Constructors

        #region Properties

        public ICommand ActiveQuickConfigItemCommand
        {
            get
            {
                return new RelayCommandWithParam((x) =>
                {
                    var qcItem = (QuickConfig)x;
                    if (qcItem.State == ConfigurationState.LOADED || qcItem.State == ConfigurationState.ACTIVE)
                    {
                        Mouse.OverrideCursor = Cursors.Wait;
                        foreach (var item in QuickConfigCollection)
                        {
                            if (item.State == ConfigurationState.ACTIVE)
                            {
                                item.State = ConfigurationState.LOADED;
                                break;
                            }
                        }

                        try
                        {
                            Task.Factory.StartNew(() =>
                            {
                                Application.Current.Dispatcher.Invoke(() =>
                                {

                                    if (File.Exists(qcItem.FilePath))
                                    {
                                        if (qcItem.AutoStart)
                                        {
                                            // Need to call execute right after ActiveQuickConfigItem, otherwise it will overwrite qcItem with the QuickTemplate
                                            // of the new modality.
                                            ActiveQuickConfigItem(qcItem);
                                            ((ICommand)MVMManager.Instance["CaptureSetupViewModel", "CaptureNowCommand", (object)new RelayCommand(() => { })]).Execute(null);
                                        }
                                        else
                                        {
                                            ActiveQuickConfigItem(qcItem);
                                        }
                                    }
                                    else
                                    {
                                        MessageBox.Show("Template file doesn't exist: " + qcItem.FilePath, "Error");
                                    }
                                });
                            }).ContinueWith((t) =>
                            {
                                qcItem.State = ConfigurationState.ACTIVE;
                                Mouse.OverrideCursor = null;
                            }, TaskScheduler.FromCurrentSynchronizationContext());
                        }
                        catch (AggregateException)
                        {
                            Mouse.OverrideCursor = null;
                        }
                        qcItem.State = ConfigurationState.ACTIVE;
                    }

                });
            }
        }

        public ICommand DeleteQuickConfigItemCommand
        {
            get
            {
                return new RelayCommandWithParam((x) =>
                {
                    ((QuickConfig)x).Clean();
                });
            }
        }

        public ICommand OpenQuickConfigCommand
        {
            get
            {
                return new RelayCommandWithParam(x =>
                {
                    if (x is QuickConfig)
                    {
                        var captureModes = new List<CaptureModes>()
                        {
                            CaptureModes.T_AND_Z,
                            CaptureModes.STREAMING,
                            //CaptureSetup.CaptureModes.TDI
                        };
                        if (((bool)MVMManager.Instance["CaptureOptionsControlViewModel", "BleachControlActive", (object)0.0]))
                        {
                            captureModes.Add(CaptureModes.BLEACHING);
                        }
                        if (((bool)MVMManager.Instance["CaptureOptionsControlViewModel", "HyperSpectralCaptureActive", (object)0.0]))
                        {
                            captureModes.Add(CaptureModes.HYPERSPECTRAL);
                        }

                        var qtDialog = new QuickTempConfigDialog(captureModes);
                        ((QuickConfig)x).CurrentDiag = qtDialog;
                        var configItem = ((QuickConfig)x).Clone();
                        qtDialog.DataContext = configItem;

                        bool? result = qtDialog.ShowDialog();
                        if (result == true)
                        {
                            ((QuickConfig)configItem).Copy((QuickConfig)x);
                            XmlDocument ApplicationDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
                            XmlNodeList nChildren = ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/QuickTemplate/Template");

                            for (int i = 0; i < nChildren.Count; i++)
                            {
                                if (i == (configItem.Id - 1))
                                {
                                    XmlManager.SetAttribute(nChildren[i], ApplicationDoc, "FilePath", ((QuickConfig)x).FilePath);
                                    if (((QuickConfig)x).AutoStart)
                                    {
                                        XmlManager.SetAttribute(nChildren[i], ApplicationDoc, "AutoStart", "1");
                                    }
                                    else
                                    {
                                        XmlManager.SetAttribute(nChildren[i], ApplicationDoc, "AutoStart", "0");
                                    }
                                    break;
                                }
                            }

                            MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS);

                            OnPropertyChanged("QuickConfigCollection");
                        }
                    }

                });
            }
        }

        public bool QtConstructor
        {
            set
            {
                if (value)
                {
                    VMQuickTemplateConstructor();
                }
            }
        }

        public List<QuickConfig> QuickConfigCollection
        {
            get
            {
                return _quickConfigCollection;
            }
            set
            {
                _quickConfigCollection = value;
                OnPropertyChanged("QuickConfigCollection");
            }
        }

        #endregion Properties

        #region Indexers

        public object this[string propertyName, object defaultObject = null]
        {
            get
            {
                PropertyInfo myPropInfo = GetPropertyInfo(propertyName);
                return (null != myPropInfo) ? myPropInfo.GetValue(this) : null;
            }
            set
            {
                PropertyInfo myPropInfo = GetPropertyInfo(propertyName);
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
                myPropInfo = typeof(QuickTemplatesControlViewModel).GetProperty(propertyName);
                if (null != myPropInfo)
                {
                    _properties.Add(propertyName, myPropInfo);
                }
            }
            return myPropInfo;
        }

        public void LoadXMLSettings()
        {
            QtConstructor = true;
            //XmlDocument doc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS];

            //XmlNodeList ndList = doc.SelectNodes("/ThorImageExperiment/LSM");

            //if (ndList.Count > 0)
            //{
            //    string str = string.Empty;
            //}
        }

        public void OnPropertyChange(string propertyName)
        {
            if (null != GetPropertyInfo(propertyName))
            {
                OnPropertyChanged(propertyName);
            }
        }

        public void UpdateExpXMLSettings(ref XmlDocument experimentFile)
        {
            //XmlNodeList ndList = experimentFile.SelectNodes("/ThorImageExperiment/LSM");
            //XmlNodeList ndListHW = this.HardwareDoc.SelectNodes("/HardwareSettings/ImageDetectors/LSM");

            //if (ndList.Count > 0)
            //{
            //}
        }

        public void VMQuickTemplateConstructor()
        {
            string[] filePath = new string[6] { "", "", "", "", "", "" };
            string[] autoStart = new string[6] { "", "", "", "", "", "" };
            XmlDocument ApplicationDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
            XmlNodeList ndList = ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/QuickTemplate");
            QuickConfigCollection.Clear();
            if (ndList.Count > 0)
            {
                XmlNodeList nChildren;
                nChildren = ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/QuickTemplate/Template");
                if (nChildren.Count <= 0)
                {
                    for (int i = 0; i < 6; i++)
                    {
                        XmlNode n = ApplicationDoc.CreateNode(XmlNodeType.Element, "Template", null);
                        XmlAttribute aId = ApplicationDoc.CreateAttribute("ID");
                        aId.Value = i.ToString();
                        n.Attributes.Append(aId);
                        XmlAttribute aFilePath = ApplicationDoc.CreateAttribute("FilePath");
                        aFilePath.Value = "";
                        n.Attributes.Append(aFilePath);
                        XmlAttribute aAutoStart = ApplicationDoc.CreateAttribute("AutoStart");
                        aAutoStart.Value = "0";
                        n.Attributes.Append(aAutoStart);

                        ndList[0].AppendChild(n);
                        filePath[i] = "";
                    }
                    MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS);

                }
                else
                {
                    string IdValue = string.Empty;
                    string FpValue = string.Empty;
                    string AutoStartValue = string.Empty;
                    string CaptureModeValue = string.Empty;
                    for (int i = 0; i < 6; i++)
                    {
                        XmlManager.GetAttribute(nChildren[i], ApplicationDoc, "ID", ref IdValue);
                        XmlManager.GetAttribute(nChildren[i], ApplicationDoc, "FilePath", ref FpValue);
                        XmlManager.GetAttribute(nChildren[i], ApplicationDoc, "AutoStart", ref AutoStartValue);
                        filePath[Convert.ToInt32(IdValue)] = FpValue;
                        autoStart[Convert.ToInt32(IdValue)] = AutoStartValue;
                    }
                }
            }
            if (QuickConfigCollection.Count != 6)
            {
                QuickConfigCollection =
                    Enumerable.Range(1, 6).Select(x => new QuickConfig(x)
                    {
                        FilePath = "",
                        AutoStart = false,
                    }).ToList();
            }
            foreach (QuickConfig item in QuickConfigCollection)
            {
                if (0 != filePath[item.Id - 1].CompareTo(""))
                {
                    item.FilePath = filePath[item.Id - 1];
                    if (item.State == ConfigurationState.UNLOAD)
                    {
                        item.State = ConfigurationState.LOADED;
                    }
                }
                item.AutoStart = (0 == autoStart[item.Id - 1].CompareTo("1")) ? true : false;
            }
        }

        private void ActiveQuickConfigItem(QuickConfig pQuickConfig)
        {
            ((ICommand)MVMManager.Instance["CaptureSetupViewModel", "StopCommand", (object)new RelayCommand(() => { })]).Execute(null);
            MVMManager.Instance["CaptureSetupViewModel", "QuickTemplateExperimentPath"] = pQuickConfig.FilePath;
        }

        #endregion Methods
    }
}