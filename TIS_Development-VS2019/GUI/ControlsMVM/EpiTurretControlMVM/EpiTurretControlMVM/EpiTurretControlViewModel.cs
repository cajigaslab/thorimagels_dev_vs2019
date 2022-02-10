namespace EpiTurretControl.ViewModel
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

    using EpiTurretControl.Model;

    using OverlayManager;

    using ThorLogging;

    using ThorSharedTypes;

    public class EpiTurretControlViewModel : ThorSharedTypes.VMBase, ThorSharedTypes.IMVM
    {
        #region Fields

        const int MAX_TURRET_POS = 6; //:TODO: Move to shared constants, also change the one in ObjectiveControlMVM

        private readonly EpiTurretControlModel _epiTurretControlModel;

        private ICommand _epiPositionNameChangeCommand;
        private ICommand _goToEpiPositionCommand;
        private Dictionary<string, PropertyInfo> _properties = new Dictionary<string, PropertyInfo>();

        #endregion Fields

        #region Constructors

        public EpiTurretControlViewModel()
        {
            this._epiTurretControlModel = new EpiTurretControlModel();
        }

        #endregion Constructors

        #region Properties

        public bool EpiPosition1
        {
            get
            {
                return (1 == EpiTurretPos);
            }
        }

        public string EpiPosition1Name
        {
            get;
            set;
        }

        public bool EpiPosition2
        {
            get
            {
                return (2 == EpiTurretPos);
            }
        }

        public string EpiPosition2Name
        {
            get;
            set;
        }

        public bool EpiPosition3
        {
            get
            {
                return (3 == EpiTurretPos);
            }
        }

        public string EpiPosition3Name
        {
            get;
            set;
        }

        public bool EpiPosition4
        {
            get
            {
                return (4 == EpiTurretPos);
            }
        }

        public string EpiPosition4Name
        {
            get;
            set;
        }

        public bool EpiPosition5
        {
            get
            {
                return (5 == EpiTurretPos);
            }
        }

        public string EpiPosition5Name
        {
            get;
            set;
        }

        public bool EpiPosition6
        {
            get
            {
                return (6 == EpiTurretPos);
            }
        }

        public string EpiPosition6Name
        {
            get;
            set;
        }

        public ICommand EpiPositionNameChangeCommand
        {
            get
            {
                if (this._epiPositionNameChangeCommand == null)
                    this._epiPositionNameChangeCommand = new RelayCommandWithParam((x) => EpiPositionNameChange(x));

                return this._epiPositionNameChangeCommand;
            }
        }

        public int EpiTurretPos
        {
            get
            {
                return _epiTurretControlModel.EpiTurretPos;
            }
            set
            {
                _epiTurretControlModel.EpiTurretPos = value;
            }
        }

        public string EpiTurretPosName
        {
            get
            {
                //Check if the current position is within a valid position
                if (_epiTurretControlModel.EpiTurretPos > 0 && _epiTurretControlModel.EpiTurretPos < 7)
                {
                    return EpiTurretPosNames[_epiTurretControlModel.EpiTurretPos - 1].Value;
                }
                return _epiTurretControlModel.EpiTurretPos.ToString();
            }
        }

        public ObservableCollection<StringPC> EpiTurretPosNames
        {
            get;
            set;
        }

        public ICommand GoToEpiPositionCommand
        {
            get
            {
                if (this._goToEpiPositionCommand == null)
                    this._goToEpiPositionCommand = new RelayCommandWithParam((x) => GoToEpiPosition(x));

                return this._goToEpiPositionCommand;
            }
        }

        public ObservableCollection<StringPC> PowerControlName
        {
            get;
            set;
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
                myPropInfo = typeof(EpiTurretControlViewModel).GetProperty(propertyName);
                if (null != myPropInfo)
                {
                    _properties.Add(propertyName, myPropInfo);
                }
            }
            return myPropInfo;
        }

        public void LoadEpiPositionNames()
        {
            try
            {
                XmlDocument applicationDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

                if (null != applicationDoc)
                {
                    XmlNodeList ndList = applicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/EpiTurretView");

                    if (ndList.Count > 0)
                    {
                        string str = string.Empty;

                        for (int i = 1; i <= MAX_TURRET_POS; i++)
                        {
                            ThorSharedTypes.XmlManager.GetAttribute(ndList[0], applicationDoc, string.Format("EpiTurretPosName{0}", i), ref str);
                            switch (i)
                            {
                                case 1:
                                    EpiPosition1Name = str;
                                    OnPropertyChanged("EpiPosition1Name");
                                    break;
                                case 2:
                                    EpiPosition2Name = str;
                                    OnPropertyChanged("EpiPosition2Name");
                                    break;
                                case 3:
                                    EpiPosition3Name = str;
                                    OnPropertyChanged("EpiPosition3Name");
                                    break;
                                case 4:
                                    EpiPosition4Name = str;
                                    OnPropertyChanged("EpiPosition4Name");
                                    break;
                                case 5:
                                    EpiPosition5Name = str;
                                    OnPropertyChanged("EpiPosition5Name");
                                    break;
                                case 6:
                                    EpiPosition6Name = str;
                                    OnPropertyChanged("EpiPosition6Name");
                                    break;
                            }
                            str = string.Empty;
                        }
                    }
                }
            }
            catch (Exception e)
            {
                e.ToString();
                //:TODO: Log issues
            }
        }

        public void LoadXMLSettings()
        {
            int iTmp = 0;
            XmlDocument doc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS];

            XmlNodeList ndList = doc.SelectNodes("/ThorImageExperiment/EPITurret");
            if (ndList.Count > 0)
            {
                string str = string.Empty;
                if (XmlManager.GetAttribute(ndList[0], doc, "pos", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iTmp))
                {
                    EpiTurretPos = iTmp - 1; // The value saved in the experiment file needs to be offset by one for the lower level
                }
            }
            LoadEpiPositionNames();
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
            try
            {
                XmlNodeList ndList = experimentFile.SelectNodes("/ThorImageExperiment/EPITurret");

                if (ndList.Count <= 0)
                {
                    ThorSharedTypes.XmlManager.CreateXmlNode(experimentFile, "EPITurret");
                    ndList = experimentFile.SelectNodes("/ThorImageExperiment/EPITurret");
                }
                XmlManager.SetAttribute(ndList[0], experimentFile, "pos", this.EpiTurretPos.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "name", EpiTurretPosName);
            }
            catch(Exception e)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 0, "Exception thrown at UpdateExpXMLSettings.\nError message: " + e.ToString());
            }
        }

        private void EpiPositionNameChange(object index)
        {
            int indexVal = Convert.ToInt32(index);
            try
            {
                XmlDocument applicationDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

                if (null != applicationDoc)
                {
                    XmlNodeList ndList = applicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/EpiTurretView");

                    if (ndList.Count > 0)
                    {
                        string str = string.Empty;
                        EpiPositionNameEditWin dlg = new EpiPositionNameEditWin();

                        ThorSharedTypes.XmlManager.GetAttribute(ndList[0], applicationDoc, string.Format("EpiTurretPosName{0}", indexVal), ref str);

                        dlg.EpiPositionName = str;
                        if (false == dlg.ShowDialog())
                        {
                            return;
                        }

                        // After name window is closed, read the last entered text
                        str = dlg.EpiPositionName;

                        switch (indexVal)
                        {
                            case 1:
                                EpiPosition1Name = str;
                                OnPropertyChanged("EpiPosition1Name");
                                break;
                            case 2:
                                EpiPosition2Name = str;
                                OnPropertyChanged("EpiPosition2Name");
                                break;
                            case 3:
                                EpiPosition3Name = str;
                                OnPropertyChanged("EpiPosition3Name");
                                break;
                            case 4:
                                EpiPosition4Name = str;
                                OnPropertyChanged("EpiPosition4Name");
                                break;
                            case 5:
                                EpiPosition5Name = str;
                                OnPropertyChanged("EpiPosition5Name");
                                break;
                            case 6:
                                EpiPosition6Name = str;
                                OnPropertyChanged("EpiPosition6Name");
                                break;
                        }

                        XmlManager.SetAttribute(ndList[0], applicationDoc, string.Format("EpiTurretPosName{0}", indexVal), str);
                        MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS);
                    }
                }
            }
            catch (Exception e)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 0, "Exception thrown at EpiPositionNameChange.\nError message: " + e.ToString());
            }
        }

        private void GoToEpiPosition(object index)
        {
            EpiTurretPos = Convert.ToInt32(index);
        }

        #endregion Methods
    }
}