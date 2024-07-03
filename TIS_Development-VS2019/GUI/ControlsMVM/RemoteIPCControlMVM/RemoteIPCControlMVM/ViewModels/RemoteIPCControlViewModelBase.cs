namespace RemoteIPCControl.ViewModels
{
    using System;
    using System.Collections.Generic;
    using System.Reflection;
    using System.Xml;

    using RemoteIPCControl.Models;

    using ThorLogging;

    using ThorSharedTypes;

    public class RemoteIPCControlViewModelBase : VMBase, IMVM
    {
        #region Fields

        private readonly RemoteIPCControlModelBase _remoteIPCControlModel;

        private Dictionary<string, PropertyInfo> _properties = new Dictionary<string, PropertyInfo>();

        bool waitingForXML = true;

        private string _thorSyncFilePath = "";

        private bool _isSaving;

        private int _tiCurrentTab = 0;

        #endregion Fields

        #region Constructors

        public RemoteIPCControlViewModelBase()
        {
            _remoteIPCControlModel = new RemoteIPCControlModelBase();
            RemotePCHostName = LocalPCHostName;
        }

        #endregion Constructors

        #region Properties

        public int IDMode
        {
            get
            {
                return _remoteIPCControlModel.IDMode;
            }
            set
            {
                if (_remoteIPCControlModel.IDMode != value && !waitingForXML)
                {
                    _remoteIPCControlModel.IDMode = value;
                    OnPropertyChanged("IDMode");
                    OnPropertyChanged("RemotePCHostName");
                }
            }
        }

        public bool IPCDownlinkFlag
        {
            get
            {
                return _remoteIPCControlModel.IPCDownlinkFlag;
            }
            set
            {
                _remoteIPCControlModel.IPCDownlinkFlag = value;
            }
        }

        public string LocalPCHostName
        {
            get
            {
                return Environment.MachineName;
            }
        }

        public string LocalPCIPv4
        {
            get
            {
                return _remoteIPCControlModel.LocalPCIPv4;
            }
        }

        public string NotifyOfSavedFile
        {
            set
            {
                _remoteIPCControlModel.NotifyOfSavedFile = value;
            }
        }

        public (ThorPipeCommand, string) ReceivedCommandThroughIPC
        {
            set
            {
                //Only receive commands if the RemoteConnection flag is on. Otherwise, only the Establish and Teardown commands are allowed. 
                if (RemoteConnection || value.Item1 == ThorPipeCommand.Establish || value.Item1 == ThorPipeCommand.TearDown)
                {
                    _remoteIPCControlModel.ReceiveFromIPCController(value.Item1, value.Item2);
                }
                if (value.Item1 == ThorPipeCommand.Establish || value.Item1 == ThorPipeCommand.TearDown)
                {
                    OnPropertyChanged("RemoteConnection");
                }
            }
        }

        public string RemoteAppName
        {
            get
            {
                return _remoteIPCControlModel.RemoteAppName;
            }
            set
            {
                if ("" == value)
                {
                    return;
                }
                _remoteIPCControlModel.RemoteAppName = value;
                OnPropertyChanged("RemoteAppName");
            }
        }

        public bool RemoteConnection
        {
            get
            {
                return _remoteIPCControlModel.RemoteConnection;
            }
            set
            {
                _remoteIPCControlModel.RemoteConnection = value;
                OnPropertyChanged("RemoteConnection");
                ((IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("RemoteStartEnabled");
                ((IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("RemoteStartIcon");
            }
        }

        public string RemotePCHostName
        {
            get
            {
                return _remoteIPCControlModel.RemotePCHostName;
            }
            set
            {
                if ("" == value)
                {
                    return;
                }
                _remoteIPCControlModel.RemotePCHostName = value;
                OnPropertyChanged("RemotePCHostName");
            }
        }

        public bool RemoteSavingStats
        {
            get
            {
                return _remoteIPCControlModel.RemoteSavingStats;
            }
            set
            {
                _remoteIPCControlModel.RemoteSavingStats = value;
            }
        }

        public int SelectedRemotePCNameIndex
        {
            get
            {
                return _remoteIPCControlModel.SelectedRemotePCNameIndex;
            }
            set
            {
                _remoteIPCControlModel.SelectedRemotePCNameIndex = value;
            }
        }

        public int SelectedTabIndex
        {
            get
            {
                return _tiCurrentTab;
            }
            set
            {
                _tiCurrentTab = value;
            }
        }

        public string ShowMostRecent
        {
            set
            {
                _remoteIPCControlModel.ShowMostRecent = value;
            }
        }

        public string StartAcquisition
        {
            set
            {
                _remoteIPCControlModel.StartAcquisition = value;
            }
        }

        public bool StopAcquisition
        {
            set
            {
                _remoteIPCControlModel.StopAcquisition = value;
            }
        }

        public string TearDownIPC
        {
            set
            {
                _remoteIPCControlModel.TearDownIPC = value;
            }
        }


        public string ThorSyncFilePath
        {
            get
            {
                return _thorSyncFilePath;
            }
            set
            {
                _thorSyncFilePath = value;
            }
        }

        public bool IsSaving
        {
            get
            {
                return _isSaving;
            }
            set
            {
                _isSaving = value;
            }
        }

        public string ThorsyncFrameSync
        {
            set
            {
                _remoteIPCControlModel.ThorsyncFrameSync = value;
            }
        }

        public ThorSyncMode ThorSyncSamplingMode
        {
            get
            {
                return _remoteIPCControlModel.ThorSyncSamplingMode;
            }
            set
            {
                _remoteIPCControlModel.ThorSyncSamplingMode = value;
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
                myPropInfo = typeof(RemoteIPCControlViewModelBase).GetProperty(propertyName);
                if (null != myPropInfo)
                {
                    _properties.Add(propertyName, myPropInfo);
                }
            }
            return myPropInfo;
        }

        public void InitIPC()
        {
            if (SelectedRemotePCNameIndex >= 0)
            {
                if (IDMode == 0)
                {
                    if (_remoteIPCControlModel.SelectRemotePCName?[SelectedRemotePCNameIndex] != "")
                    {
                        //only reconnect if the host name is different
                        if (false == _remoteIPCControlModel.SelectRemotePCName?[SelectedRemotePCNameIndex].Equals(RemotePCHostName))
                        {
                            RemotePCHostName = _remoteIPCControlModel.SelectRemotePCName?[SelectedRemotePCNameIndex];
                        }
                    }
                    else
                    {
                        RemotePCHostName = LocalPCHostName;
                    }

                }
                else if (IDMode == 1)
                {
                    if (_remoteIPCControlModel.SelectRemodePCIPAddr[SelectedRemotePCNameIndex] != "")
                    {
                        //only reconnect if the host id is different
                        if (false == _remoteIPCControlModel.SelectRemodePCIPAddr[SelectedRemotePCNameIndex].Equals(RemotePCHostName))
                        {
                            RemotePCHostName = _remoteIPCControlModel.SelectRemodePCIPAddr[SelectedRemotePCNameIndex];
                        }
                    }
                    else
                    {
                        RemotePCHostName = _remoteIPCControlModel.GetLocalIP();
                    }

                }
            }
        }

        /// <summary>
        /// Loads the remote pc host name from XML.
        /// </summary>
        public void LoadXMLSettings()
        {
            waitingForXML = true;
            XmlDocument doc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

            if (null == doc)
            {
                return;
            }

            _remoteIPCControlModel.SelectRemotePCName.Clear();
            _remoteIPCControlModel.SelectRemodePCIPAddr.Clear();

            string strTemp = string.Empty;
            XmlNodeList nodeList = doc.SelectNodes("ApplicationSettings/IPCRemoteHostPCName");
            if (nodeList.Count != 0)
            {
                foreach (XmlNode node in nodeList)
                {
                    if (XmlManager.GetAttribute(node, doc, "name", ref strTemp))
                    {
                        RemotePCHostName = strTemp;
                        string[] hostname = strTemp.Split('/');
                        for (int i = 0; i < hostname.Length; i++)
                        {
                            _remoteIPCControlModel.SelectRemotePCName.Add(hostname[i]);
                        }
                    }
                    if (XmlManager.GetAttribute(node, doc, "IP", ref strTemp))
                    {
                        string[] ipAddr = strTemp.Split('/');
                        for (int i = 0; i < ipAddr.Length; i++)
                        {
                            _remoteIPCControlModel.SelectRemodePCIPAddr.Add(ipAddr[i]);
                        }
                    }
                    if (XmlManager.GetAttribute(node, doc, "IDMode", ref strTemp))
                    {
                        IDMode = Convert.ToInt32(strTemp);
                    }
                    if (XmlManager.GetAttribute(node, doc, "activeIndex", ref strTemp))
                    {
                        SelectedRemotePCNameIndex = Convert.ToInt32(strTemp);
                    }
                    if (XmlManager.GetAttribute(node, doc, "remoteAppName", ref strTemp))
                    {
                        if (strTemp == "")
                        {
                            strTemp = "ThorSync";
                        }
                        RemoteAppName = strTemp;
                    }
                }
            }
            else
            {
                _remoteIPCControlModel.SelectRemotePCName.Add(Environment.MachineName);
                _remoteIPCControlModel.SelectRemodePCIPAddr.Add(_remoteIPCControlModel.LocalPCIPv4);
            }
            waitingForXML = false;
            InitIPC();
        }

        public void OnPropertyChange(string propertyName)
        {
            if (null != GetPropertyInfo(propertyName))
            {
                OnPropertyChanged(propertyName);
            }
        }

        /// <summary>
        /// Saves the remote pc host name to XML.
        /// </summary>
        public void UpdateExpXMLSettings(ref XmlDocument experimentFile)
        {
            //string remotePCHostNameList = String.Join("/", _remoteIPCControlModel.SelectRemotePCName);
            //string remotePCIPAddressList = String.Join("/", _remoteIPCControlModel.SelectRemodePCIPAddr);

            XmlDocument doc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

            if (null == doc)
            {
                return;
            }
            var root = doc.DocumentElement;//Get to the root node
            XmlElement node = (XmlElement)doc.SelectSingleNode("ApplicationSettings/IPCRemoteHostPCName");
            if (node == null)
            {
                XmlElement elementRoot = doc.CreateElement(string.Empty, "IPCRemoteHostPCName", string.Empty);
                XmlElement rootNode = (XmlElement)doc.SelectSingleNode("ApplicationSettings");
                rootNode.AppendChild(elementRoot);
                node = (XmlElement)doc.SelectSingleNode("ApplicationSettings/IPCRemoteHostPCName");
            }
            node.SetAttribute("name", _remoteIPCControlModel.RemotePCHostName);
            node.SetAttribute("IP", ResourceManagerCS.GetLocalIP());
            node.SetAttribute("IDMode", IDMode.ToString());
            node.SetAttribute("activeIndex", SelectedRemotePCNameIndex.ToString());
            node.SetAttribute("remoteAppName", RemoteAppName.ToString());
            MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS, true);
        }

        #endregion Methods
    }
}