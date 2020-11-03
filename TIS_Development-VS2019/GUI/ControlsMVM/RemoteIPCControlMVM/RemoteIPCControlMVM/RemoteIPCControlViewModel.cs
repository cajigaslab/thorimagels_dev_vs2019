namespace RemoteIPCControl.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
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

    using Microsoft.Practices.Composite.Events;
    using Microsoft.Practices.Composite.Regions;
    using Microsoft.Practices.Composite.Wpf.Events;
    using Microsoft.Practices.Unity;

    using OverlayManager;

    using RemoteIPCControl.Model;

    using ThorLogging;

    using ThorSharedTypes;

    public class RemoteIPCControlViewModel : ThorSharedTypes.VMBase, ThorSharedTypes.IMVM
    {
        #region Fields

        private readonly RemoteIPCControlModel _RemoteIPCControlModel;

        private Dictionary<string, PropertyInfo> _properties = new Dictionary<string, PropertyInfo>();

        #endregion Fields

        #region Constructors

        public RemoteIPCControlViewModel()
        {
            this._RemoteIPCControlModel = new RemoteIPCControlModel();
            RemotePCHostName = LocalPCHostName;
        }

        #endregion Constructors

        #region Properties

        public IEventAggregator EventAggregator
        {
            get
            {
                return this._RemoteIPCControlModel.EventAggregator;
            }
            set
            {
                this._RemoteIPCControlModel.EventAggregator = value;
            }
        }

        public int IDMode
        {
            get
            {
                return this._RemoteIPCControlModel.IDMode;
            }
            set
            {
                this._RemoteIPCControlModel.IDMode = value;
                OnPropertyChanged("IDMode");
                OnPropertyChanged("RemotePCHostName");
            }
        }

        public string LocalPCHostName
        {
            get
            {
                return System.Environment.MachineName;
            }
        }

        public string LocalPCIPv4
        {
            get
            {
                return this._RemoteIPCControlModel.LocalPCIPv4;
            }
        }

        public string RemoteAppName
        {
            get
            {
                return this._RemoteIPCControlModel.RemoteAppName;
            }
            set
            {
                if ("" == value)
                {
                    return;
                }
                this._RemoteIPCControlModel.RemoteAppName = value;
                OnPropertyChanged("RemoteAppName");
            }
        }

        public bool RemoteConnection
        {
            get
            {
                return this._RemoteIPCControlModel.RemoteConnection;
            }
            set
            {
                this._RemoteIPCControlModel.RemoteConnection = value;
                OnPropertyChanged("RemoteConnection");
                ((IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("RemoteStartEnabled");
                ((IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("RemoteStartIcon");
            }
        }

        public string RemotePCHostName
        {
            get
            {
                return this._RemoteIPCControlModel.RemotePCHostName;
            }
            set
            {
                if ("" == value)
                {
                    return;
                }
                this._RemoteIPCControlModel.RemotePCHostName = value;
                OnPropertyChanged("RemotePCHostName");
            }
        }

        public string StartAcquisition
        {
            set
            {
                this._RemoteIPCControlModel.StartAcquisition = value;
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
                myPropInfo = typeof(RemoteIPCControlViewModel).GetProperty(propertyName);
                if (null != myPropInfo)
                {
                    _properties.Add(propertyName, myPropInfo);
                }
            }
            return myPropInfo;
        }

        public void LoadXMLSettings()
        {
            XmlDocument doc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS];

            XmlNodeList ndList = doc.SelectNodes("/ThorImageExperiment/LSM");

            if (ndList.Count > 0)
            {
                string str = string.Empty;
            }
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
            XmlNodeList ndList = experimentFile.SelectNodes("/ThorImageExperiment/LSM");
               // XmlNodeList ndListHW = this.HardwareDoc.SelectNodes("/HardwareSettings/ImageDetectors/LSM");

            if (ndList.Count > 0)
            {
            }
        }

        #endregion Methods
    }
}