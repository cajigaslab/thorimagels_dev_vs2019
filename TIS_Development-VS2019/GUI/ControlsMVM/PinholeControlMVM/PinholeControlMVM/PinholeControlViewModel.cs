namespace PinholeControl.ViewModel
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

    using OverlayManager;

    using PinholeControl.Model;

    using ThorLogging;

    using ThorSharedTypes;

    public class PinholeControlViewModel : ThorSharedTypes.VMBase, ThorSharedTypes.IMVM
    {
        #region Fields

        private readonly PinholeControlModel _PinholeControlModel;

        private ObservableCollection<string> _comboBoxItemsList;
        private bool _isValueNegative;
        private int _pinholeIndex;
        private string _pinholeValue;
        private Dictionary<string, PropertyInfo> _properties = new Dictionary<string, PropertyInfo>();
        private bool _switchSizeWithADU;
        private string _txtPinholeAlignment;

        #endregion Fields

        #region Constructors

        public PinholeControlViewModel()
        {
            this._PinholeControlModel = new PinholeControlModel();
            _comboBoxItemsList = new ObservableCollection<string>();
        }

        #endregion Constructors

        #region Properties

        public ObservableCollection<string> ComboBoxItemsList
        {
            get
            {
                _comboBoxItemsList.Clear();
                if (IsSettingsEnabled)
                {
                    for (int i = 0; i < 16; i++)
                    {
                        string valueInADU = ConvertIndexToADU(i).ToString() + " ADUs";
                        _comboBoxItemsList.Add(valueInADU);
                    }
                }
                else
                {
                    _comboBoxItemsList.Add("25 um");
                    _comboBoxItemsList.Add("30 um");
                    _comboBoxItemsList.Add("35 um");
                    _comboBoxItemsList.Add("40 um");
                    _comboBoxItemsList.Add("45 um");
                    _comboBoxItemsList.Add("50 um");
                    _comboBoxItemsList.Add("60 um");
                    _comboBoxItemsList.Add("70 um");
                    _comboBoxItemsList.Add("80 um");
                    _comboBoxItemsList.Add("90 um");
                    _comboBoxItemsList.Add("100 um");
                    _comboBoxItemsList.Add("125 um");
                    _comboBoxItemsList.Add("200 um");
                    _comboBoxItemsList.Add("300 um");
                    _comboBoxItemsList.Add("1 mm");
                    _comboBoxItemsList.Add("2 mm");
                }
                return _comboBoxItemsList;
            }
        }

        public bool IsSettingsEnabled
        {
            get
            {
                return _switchSizeWithADU;
            }
            set
            {
                _switchSizeWithADU = value;
                OnPropertyChanged("IsSettingsEnabled");
                int pinholePos = (int)MVMManager.Instance["PinholeControlViewModel", "PinholePosition", (object)0];
                PinholePosition = -1;
                OnPropertyChanged("ComboBoxItemsList");
                PinholePosition = pinholePos;
            }
        }

        public ICommand LSMPinholeAlignmentMinusCommand
        {
            get
            {
                return new RelayCommand(() => butMinus_Click());
            }
        }

        public ICommand LSMPinholeAlignmentPlusCommand
        {
            get
            {
                return new RelayCommand(() => butPlus_Click());
            }
        }

        public int LSMPinholeAlignmentPosition
        {
            get
            {
                return GetAligmentPos(_PinholeControlModel.LSMPinholeAlignmentPosition);
            }
            set
            {
                _PinholeControlModel.LSMPinholeAlignmentPosition = value;
            }
        }

        public ICommand LSMPinholeAlignmentSetCommand
        {
            get
            {
                return new RelayCommand(() => btnSetPinhole_Click());
            }
        }

        public int LSMSavePinholeAlignmentPosition
        {
            get
            {
                return GetAligmentPos(_PinholeControlModel.LSMSavePinholeAlignmentPosition);
            }
            set
            {
                _PinholeControlModel.LSMSavePinholeAlignmentPosition = SetAligmentPos(value);
            }
        }

        public double PinholeADUs
        {
            get
            {
                return ConvertIndexToADU(_PinholeControlModel.PinholePosition);
            }
        }

        public string PinholeADUsString
        {
            get
            {
                if (IsSettingsEnabled)
                {
                    _pinholeValue = PinholeMicroMeters.ToString() + " um";
                    return PinholeMicroMeters.ToString() + " um";
                }
                else
                {
                    _pinholeValue = Convert.ToString(PinholeADUs) + " ADUs";
                    return Convert.ToString(PinholeADUs) + " ADUs";
                }
            }
        }

        public int PinholeMax
        {
            get
            {
                return _PinholeControlModel.PinholeMax;
            }
        }

        public double PinholeMicroMeters
        {
            get
            {
                return PinholeSize;
            }
        }

        public int PinholeMin
        {
            get
            {
                return _PinholeControlModel.PinholeMin;
            }
        }

        public int PinholePosition
        {
            get
            {
                if (_isValueNegative)
                {
                    return -1;
                }
                else
                {
                    return _PinholeControlModel.PinholePosition;
                }
            }
            set
            {
                if (value == -1)
                {
                    _isValueNegative = true;
                }
                else
                {
                    _isValueNegative = false;
                    _PinholeControlModel.PinholePosition = _pinholeIndex = value;
                }
                OnPropertyChanged("PinholePosition");
                OnPropertyChanged("PinholeSize");
                ((IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("CollapsedPinholeSize");
                ((IMVM)MVMManager.Instance["ZControlViewModel", this]).OnPropertyChange("ZSectionThickness");
                OnPropertyChanged("LSMPinholeAlignmentPosition");
                OnPropertyChanged("PinholeADUs");
                OnPropertyChanged("PinholeADUsString");
            }
        }

        public double PinholeSize
        {
            get
            {
                return PinholeSizeUM;
            }
        }

        public double PinholeSizeUM
        {
            get
            {
                double pinholeSize = 0;

                switch (PinholePosition)
                {
                    case 0: pinholeSize = 25; break;
                    case 1: pinholeSize = 30; break;
                    case 2: pinholeSize = 35; break;
                    case 3: pinholeSize = 40; break;
                    case 4: pinholeSize = 45; break;
                    case 5: pinholeSize = 50; break;
                    case 6: pinholeSize = 60; break;
                    case 7: pinholeSize = 70; break;
                    case 8: pinholeSize = 80; break;
                    case 9: pinholeSize = 90; break;
                    case 10: pinholeSize = 100; break;
                    case 11: pinholeSize = 125; break;
                    case 12: pinholeSize = 200; break;
                    case 13: pinholeSize = 300; break;
                    case 14: pinholeSize = 1000; break;
                    case 15: pinholeSize = 2000; break;
                    default: pinholeSize = 0; break;
                }

                return pinholeSize;
            }
        }

        public string TxtPinholeAlignment
        {
            get
            {
                return _txtPinholeAlignment;
            }
            set
            {
                _txtPinholeAlignment = value;
                OnPropertyChange("TxtPinholeAlignment");
            }
        }

        public ICommand UpdatePinholePosTxtCommand
        {
            get
            {
                return new RelayCommand(() => UpdatePinholePosTxt());
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

        public static double ConvertIndexToADU(int index)
        {
            XmlDocument hardwareDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];
            XmlNodeList objList = hardwareDoc.SelectNodes("/HardwareSettings/Objectives/Objective");
            int turretPosition = (int)MVMManager.Instance["ObjectiveControlViewModel", "TurretPosition", (object)0];

            double au = 0;
            if (objList.Count > turretPosition)
            {
                double mag = Convert.ToDouble(objList[turretPosition].Attributes["mag"].Value.ToString(CultureInfo.InvariantCulture), CultureInfo.InvariantCulture);
                double na = Convert.ToDouble(objList[turretPosition].Attributes["na"].Value.ToString(CultureInfo.InvariantCulture), CultureInfo.InvariantCulture);

                const double SCANLENS_MAG = 1.071428571;
                const double WAVELENGTH_UM = .488;
                au = ((2 * 0.61 * WAVELENGTH_UM * SCANLENS_MAG * mag) / na);
            }

            if (au <= 0)
            {
                return -1;
            }

            double pinholeSize = 0;
            switch (index)
            {
                case 0: pinholeSize = 25; break;
                case 1: pinholeSize = 30; break;
                case 2: pinholeSize = 35; break;
                case 3: pinholeSize = 40; break;
                case 4: pinholeSize = 45; break;
                case 5: pinholeSize = 50; break;
                case 6: pinholeSize = 60; break;
                case 7: pinholeSize = 70; break;
                case 8: pinholeSize = 80; break;
                case 9: pinholeSize = 90; break;
                case 10: pinholeSize = 100; break;
                case 11: pinholeSize = 125; break;
                case 12: pinholeSize = 200; break;
                case 13: pinholeSize = 300; break;
                case 14: pinholeSize = 1000; break;
                case 15: pinholeSize = 2000; break;
                default: pinholeSize = 0; break;
            }

            double aduCalc = Math.Round(pinholeSize / au, 3);
            return aduCalc;
        }

        public PropertyInfo GetPropertyInfo(string propertyName)
        {
            PropertyInfo myPropInfo = null;
            if (!_properties.TryGetValue(propertyName, out myPropInfo))
            {
                myPropInfo = typeof(PinholeControlViewModel).GetProperty(propertyName);
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

            XmlNodeList ndList = doc.SelectNodes("/ThorImageExperiment/PinholeWheel");

            string str = string.Empty;

            if (ndList.Count > 0)
            {
                if (XmlManager.GetAttribute(ndList[0], doc, "position", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        PinholePosition = tmp;
                    }
                }
            }

            XmlDocument applicationDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
            XmlNodeList objList = applicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/PinholeView");

            if (objList.Count > 0)
            {
                str = string.Empty;
                IsSettingsEnabled = (XmlManager.GetAttribute(objList[0], applicationDoc, "SwitchSizeWithADU", ref str)) ? (str == "1" || str == Boolean.TrueString) : true;
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
            XmlNodeList ndList = experimentFile.SelectNodes("/ThorImageExperiment/PinholeWheel");

            if (ndList.Count > 0)
            {
                ndList[0].Attributes["position"].Value = this.PinholePosition.ToString();
                XmlManager.SetAttribute(ndList[0], experimentFile, "micrometers", this.PinholeMicroMeters.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "adu", PinholeADUs.ToString());
            }
        }

        private void btnSetPinhole_Click()
        {
            try
            {
                LSMSavePinholeAlignmentPosition = Convert.ToInt32(TxtPinholeAlignment);
                PinholePosition = _pinholeIndex; //Go back to Pinhole Position (traceback)
            }
            catch (FormatException ex)
            {
                string msg = ex.Message;
            }
        }

        private void butMinus_Click()
        {
            try
            {
                int val = Convert.ToInt32(TxtPinholeAlignment);
                val = Math.Max(val - 1, 0);
                TxtPinholeAlignment = val.ToString();
                LSMPinholeAlignmentPosition = val;
            }
            catch (FormatException ex)
            {
                string msg = ex.Message;
            }
        }

        private void butPlus_Click()
        {
            try
            {
                int val = Convert.ToInt32(TxtPinholeAlignment);
                val = Math.Min(val + 1, 10000);
                TxtPinholeAlignment = val.ToString();
                LSMPinholeAlignmentPosition = val;
            }
            catch (FormatException ex)
            {
                string msg = ex.Message;
            }
        }

        /// <summary>
        /// Get the Alighment position depends on the Pinhole Position by converting from first position (Read Side)
        /// </summary>
        ///
        /// <param name="lsmPinholeAlignmentPosition">The first position(the position at 25nm)</param>
        ///
        /// <exception>NONE</exception>
        int GetAligmentPos(int lsmPinholeAlignmentPosition)
        {
            switch (PinholePosition)
            {
                case 0:
                case 1:
                case 2:
                case 3:
                    {
                        lsmPinholeAlignmentPosition = lsmPinholeAlignmentPosition + PinholePosition * 500;
                        lsmPinholeAlignmentPosition = (lsmPinholeAlignmentPosition > 100000) ? lsmPinholeAlignmentPosition - 10000 : lsmPinholeAlignmentPosition;
                    }
                    break;
                case 4:
                case 5:
                case 6:
                case 7:
                    {
                        lsmPinholeAlignmentPosition = lsmPinholeAlignmentPosition + (PinholePosition * 500 + 500);
                        lsmPinholeAlignmentPosition = (lsmPinholeAlignmentPosition > 10000) ? lsmPinholeAlignmentPosition - 10000 : lsmPinholeAlignmentPosition;
                    }
                    break;
                case 8:
                case 9:
                case 10:
                case 11:
                    {
                        lsmPinholeAlignmentPosition = lsmPinholeAlignmentPosition + (PinholePosition * 500 + 1000);
                        lsmPinholeAlignmentPosition = (lsmPinholeAlignmentPosition > 10000) ? lsmPinholeAlignmentPosition - 10000 : lsmPinholeAlignmentPosition;
                    }
                    break;
                case 12:
                case 13:
                case 14:
                case 15:
                    {
                        lsmPinholeAlignmentPosition = lsmPinholeAlignmentPosition + (PinholePosition * 500 + 1500);
                        lsmPinholeAlignmentPosition = (lsmPinholeAlignmentPosition > 10000) ? lsmPinholeAlignmentPosition - 10000 : lsmPinholeAlignmentPosition;
                    }
                    break;
            }
            return (lsmPinholeAlignmentPosition);
        }

        /// <summary>
        /// Get the Alighment position depends on the Pinhole Position by converting from first position (Save Side)
        /// </summary>
        ///
        /// <param name="lsmPinholeAlignmentPosition">The first position(the position at 25nm)</param>
        ///
        /// <exception>NONE</exception>
        int SetAligmentPos(int lsmPinholeAlignmentPosition)
        {
            switch (PinholePosition)
            {
                case 0:
                case 1:
                case 2:
                case 3:
                    {
                        lsmPinholeAlignmentPosition = lsmPinholeAlignmentPosition - PinholePosition * 500;
                        lsmPinholeAlignmentPosition = (lsmPinholeAlignmentPosition < 0) ? lsmPinholeAlignmentPosition + 10000 : lsmPinholeAlignmentPosition;
                    }
                    break;
                case 4:
                case 5:
                case 6:
                case 7:
                    {
                        lsmPinholeAlignmentPosition = lsmPinholeAlignmentPosition - (PinholePosition * 500 + 500);
                        lsmPinholeAlignmentPosition = (lsmPinholeAlignmentPosition < 0) ? lsmPinholeAlignmentPosition + 10000 : lsmPinholeAlignmentPosition;
                    }
                    break;
                case 8:
                case 9:
                case 10:
                case 11:
                    {
                        lsmPinholeAlignmentPosition = lsmPinholeAlignmentPosition - (PinholePosition * 500 + 1000);
                        lsmPinholeAlignmentPosition = (lsmPinholeAlignmentPosition < 0) ? lsmPinholeAlignmentPosition + 10000 : lsmPinholeAlignmentPosition;
                    }
                    break;
                case 12:
                case 13:
                case 14:
                case 15:
                    {
                        lsmPinholeAlignmentPosition = lsmPinholeAlignmentPosition - (PinholePosition * 500 + 1500);
                        lsmPinholeAlignmentPosition = (lsmPinholeAlignmentPosition < 0) ? lsmPinholeAlignmentPosition + 10000 : lsmPinholeAlignmentPosition;
                    }
                    break;
            }
            return (lsmPinholeAlignmentPosition);
        }

        void UpdatePinholePosTxt()
        {
            TxtPinholeAlignment = LSMPinholeAlignmentPosition.ToString();
        }

        #endregion Methods
    }
}