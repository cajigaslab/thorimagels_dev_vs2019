namespace ThreePhotonControl.ViewModel
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

    using ThorLogging;

    using ThorSharedTypes;

    using ThreePhotonControl.Model;

    public class ThreePhotonControlViewModel : ThorSharedTypes.VMBase, ThorSharedTypes.IMVM
    {
        #region Fields

        public double _threePhotonFrequency;

        const int NUM_CHANNELS = 4;

        private readonly ThreePhotonControlModel _threePhotonControlModel;

        Visibility _acquireDuringTurnAroundVisibility = Visibility.Collapsed;
        ICommand _ddsAmplitude0MinusCommand;
        ICommand _ddsAmplitude0PlusCommand;
        ICommand _ddsAmplitude1MinusCommand;
        ICommand _ddsAmplitude1PlusCommand;
        ICommand _ddsPhase0MinusCommand;
        ICommand _ddsPhase0PlusCommand;
        ICommand _ddsPhase1MinusCommand;
        ICommand _ddsPhase1PlusCommand;
        bool _disable3PCheckbox = true;
        Visibility _firSettingsVisibility;
        Visibility _multiplaneVisibility;
        ICommand _numberOfPlanesMinusCommand;
        ICommand _numberOfPlanesPlusCommand;
        Dictionary<string, PropertyInfo> _properties = new Dictionary<string, PropertyInfo>();
        ICommand _threePhotonMeasureFrequencyCommand;
        int _threePhotonPanelEnable = 1;
        ICommand _threePhotonPhaseCoarseMinusCommand;
        ICommand _threePhotonPhaseCoarsePlusCommand;
        Visibility _threePhotonPhaseCoarseVisibility;
        ICommand _threePhotonPhaseFineMinusCommand;
        ICommand _threePhotonPhaseFinePlusCommand;

        #endregion Fields

        #region Constructors

        public ThreePhotonControlViewModel()
        {
            this._threePhotonControlModel = new ThreePhotonControlModel();
        }

        #endregion Constructors

        #region Properties

        public int AcquireDuringTurnAround
        {
            get => _threePhotonControlModel.AcquireDuringTurnAround;
            set
            {
                _threePhotonControlModel.AcquireDuringTurnAround = value;
                OnPropertyChanged("AcquireDuringTurnAround");
            }
        }

        public Visibility AcquireDuringTurnAroundVisibility
        {
            get => _acquireDuringTurnAroundVisibility;
            set
            {
                _acquireDuringTurnAroundVisibility = value;
                OnPropertyChanged("AcquireDuringTurnAroundVisibility");
            }
        }

        public double DDSAmplitude0
        {
            get => _threePhotonControlModel.DDSAmplitude0;
            set
            {
                _threePhotonControlModel.DDSAmplitude0 = value;

                OnPropertyChanged("DDSAmplitude0");
            }
        }

        public ICommand DDSAmplitude0MinusCommand
        {
            get => _ddsAmplitude0MinusCommand ?? (_ddsAmplitude0MinusCommand = new RelayCommand(() => --DDSAmplitude0));
        }

        public ICommand DDSAmplitude0PlusCommand
        {
            get => _ddsAmplitude0PlusCommand ?? (_ddsAmplitude0PlusCommand = new RelayCommand(() => ++DDSAmplitude0));
        }

        public double DDSAmplitude1
        {
            get => _threePhotonControlModel.DDSAmplitude1;
            set
            {
                _threePhotonControlModel.DDSAmplitude1 = value;

                OnPropertyChanged("DDSAmplitude1");
            }
        }

        public ICommand DDSAmplitude1MinusCommand
        {
            get => _ddsAmplitude1MinusCommand ?? (_ddsAmplitude1MinusCommand = new RelayCommand(() => --DDSAmplitude1));
        }

        public ICommand DDSAmplitude1PlusCommand
        {
            get => _ddsAmplitude1PlusCommand ?? (_ddsAmplitude1PlusCommand = new RelayCommand(() => ++DDSAmplitude1));
        }

        public int DDSEnable
        {
            get => _threePhotonControlModel.DDSEnable;
            set
            {
                _threePhotonControlModel.DDSEnable = value;

                OnPropertyChanged("DDSEnable");
            }
        }

        public double DDSPhase0
        {
            get => _threePhotonControlModel.DDSPhase0;
            set
            {
                _threePhotonControlModel.DDSPhase0 = value;

                OnPropertyChanged("DDSPhase0");
            }
        }

        public ICommand DDSPhase0MinusCommand
        {
            get => _ddsPhase0MinusCommand ?? (_ddsPhase0MinusCommand = new RelayCommand(() => --DDSPhase0));
        }

        public ICommand DDSPhase0PlusCommand
        {
            get => _ddsPhase0PlusCommand ?? (_ddsPhase0PlusCommand = new RelayCommand(() => ++DDSPhase0));
        }

        public double DDSPhase1
        {
            get => _threePhotonControlModel.DDSPhase1;
            set
            {
                _threePhotonControlModel.DDSPhase1 = value;

                OnPropertyChanged("DDSPhase1");
            }
        }

        public ICommand DDSPhase1MinusCommand
        {
            get => _ddsPhase1MinusCommand ?? (_ddsPhase1MinusCommand = new RelayCommand(() => --DDSPhase1));
        }

        public ICommand DDSPhase1PlusCommand
        {
            get => _ddsPhase1PlusCommand ?? (_ddsPhase1PlusCommand = new RelayCommand(() => ++DDSPhase1));
        }

        public bool Disable3PCheckbox
        {
            get
            {
                return _disable3PCheckbox;
            }
            set
            {
                _disable3PCheckbox = value;
                OnPropertyChanged("Disable3PCheckbox");
            }
        }

        public int FIR1ManualControlEnable
        {
            get
            {
                return this._threePhotonControlModel.FIR1ManualControlEnable;
            }
            set
            {
                this._threePhotonControlModel.FIR1ManualControlEnable = value;
                OnPropertyChanged("FIR1ManualControlEnable");
            }
        }

        public Visibility FIRSettingsVisibility
        {
            get
            {

                return _firSettingsVisibility;
            }
            set
            {
                _firSettingsVisibility = value;
                OnPropertyChanged("FIRSettingsVisibility");

            }
        }

        public int LSMFIRFilterIndex
        {
            get
            {
                return this._threePhotonControlModel.LSMFIRFilterIndex;
            }
            set
            {
                this._threePhotonControlModel.LSMFIRFilterIndex = value;
                OnPropertyChanged("LSMFIRFilterIndex");
                OnPropertyChanged("LSMFIRFilterTapValue");
            }
        }

        public int LSMFIRFilterTapIndex
        {
            get
            {
                return this._threePhotonControlModel.LSMFIRFilterTapIndex;
            }
            set
            {
                this._threePhotonControlModel.LSMFIRFilterTapIndex = value;
                OnPropertyChanged("LSMFIRFilterTapIndex");
                OnPropertyChanged("LSMFIRFilterTapValue");
            }
        }

        public double LSMFIRFilterTapValue
        {
            get
            {
                return this._threePhotonControlModel.LSMFIRFilterTapValue;
            }
            set
            {
                this._threePhotonControlModel.LSMFIRFilterTapValue = value;
                OnPropertyChanged("LSMFIRFilterTapValue");
            }
        }

        public int LSMNumberOfPlanes
        {
            get
            {
                return this._threePhotonControlModel.LSMNumberOfPlanes;
            }
            set
            {
                if (this._threePhotonControlModel.LSMNumberOfPlanes != value)
                {
                    this._threePhotonControlModel.LSMNumberOfPlanes = value;
                    OnPropertyChanged("LSMNumberOfPlanes");
                }
                if (1 < this._threePhotonControlModel.LSMNumberOfPlanes && 1 == ThreePhotonEnable)
                {
                    MVMManager.Instance["ScanControlViewModel", "DwellTimeSliderEnabled"] = false;
                    MVMManager.Instance["ScanControlViewModel", "LSMPixelDwellTime"] = (double)MVMManager.Instance["ScanControlViewModel", "LSMPixelDwellTimeMin"];
                }
                else
                {
                    MVMManager.Instance["ScanControlViewModel", "DwellTimeSliderEnabled"] = true;
                }
            }
        }

        public int LUTOffset3P
        {
            get
            {
                return _threePhotonControlModel.LUTOffset3P;
            }
            set
            {
                _threePhotonControlModel.LUTOffset3P = value;
                OnPropertyChanged("LUTOffset3P");
            }
        }

        public Visibility MultiplaneVisibility
        {
            get
            {
                return _multiplaneVisibility;
            }
            set
            {
                _multiplaneVisibility = value;
                OnPropertyChanged("MultiplaneVisibility");
            }
        }

        public ICommand NumberOfPlanesMinusCommand
        {
            get
            {
                if (this._numberOfPlanesMinusCommand == null)
                    this._numberOfPlanesMinusCommand = new RelayCommand(() => --LSMNumberOfPlanes);

                return this._numberOfPlanesMinusCommand;
            }
        }

        public ICommand NumberOfPlanesPlusCommand
        {
            get
            {
                if (this._numberOfPlanesPlusCommand == null)
                    this._numberOfPlanesPlusCommand = new RelayCommand(() => ++LSMNumberOfPlanes);

                return this._numberOfPlanesPlusCommand;
            }
        }

        public int ThreePhotonEnable
        {
            get
            {
                return _threePhotonControlModel.ThreePhotonEnable;
            }
            set
            {
                if (value != _threePhotonControlModel.ThreePhotonEnable)
                {
                    _threePhotonControlModel.ThreePhotonEnable = value;
                    OnPropertyChanged("ThreePhotonEnable");
                }

                ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("LSMExtClockRate");
                // update the frequency, disable dwell time slider
                if (1 == value && ResourceManagerCS.Instance.IsThorDAQBoard)
                {
                    //MVMManager.Instance["ScanControlViewModel", "LSMClockSource"] = value;

                    //not a pretty way to get and set the freq but this process takes a long time
                    //this way we only do it once, and when the getter for ThreePhotonFreq is called
                    //the frequency will already be determined
                    ThreePhotonFreq = _threePhotonControlModel.ThreePhotonFreq;

                    UpdateDwellTimeForThreePhoton();
                }
                else
                {
                    MVMManager.Instance["ScanControlViewModel", "LSMPixelDwellTime"] = 0.4 + (0.2 * Math.Round(((double)MVMManager.Instance["ScanControlViewModel", "LSMPixelDwellTime"] - 0.4) / 0.2));
                }
                //else
                //{
                //    MVMManager.Instance["ScanControlViewModel", "LSMClockSource"] = value;
                //}
                //MVMManager.Instance["ScanControlViewModel", "DwellTimeSliderEnabled"] = (0 == value) ? true : false;
                LSMNumberOfPlanes = LSMNumberOfPlanes;
                ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("LSMClockSource");
                ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("InputRange");
                MVMManager.Instance["ScanControlViewModel", "UpdateDwellTime"] = true;
                ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("LSMPixelDwellTimeMaxIndex");
                ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("PulsesPerPixelVisibility");

            }
        }

        public double ThreePhotonFreq
        {
            get
            {
                return _threePhotonFrequency;
            }
            set
            {
                _threePhotonFrequency = value;
                OnPropertyChanged("ThreePhotonFreq");
            }
        }

        public ICommand ThreePhotonMeasureFrequencyCommand
        {
            get
            {
                if (this._threePhotonMeasureFrequencyCommand == null)
                    this._threePhotonMeasureFrequencyCommand = new RelayCommand(() => ThreePhotonMeasureFrequency());

                return this._threePhotonMeasureFrequencyCommand;
            }
        }

        public int ThreePhotonPanelEnable
        {
            get
            {
                return _threePhotonPanelEnable;
            }
            set
            {
                _threePhotonPanelEnable = value;
                OnPropertyChanged("ThreePhotonPanelEnable");
            }
        }

        public int ThreePhotonPhaseCoarse
        {
            get
            {
                return this._threePhotonControlModel.ThreePhotonPhaseCoarse;
            }
            set
            {
                this._threePhotonControlModel.ThreePhotonPhaseCoarse = value;
                OnPropertyChanged("ThreePhotonPhaseCoarse");
            }
        }

        public ICommand ThreePhotonPhaseCoarseMinusCommand
        {
            get
            {
                if (this._threePhotonPhaseCoarseMinusCommand == null)
                    this._threePhotonPhaseCoarseMinusCommand = new RelayCommand(() => ThreePhotonPhaseCoarseMinus());

                return this._threePhotonPhaseCoarseMinusCommand;
            }
        }

        public ICommand ThreePhotonPhaseCoarsePlusCommand
        {
            get
            {
                if (this._threePhotonPhaseCoarsePlusCommand == null)
                    this._threePhotonPhaseCoarsePlusCommand = new RelayCommand(() => ThreePhotonPhaseCoarsePlus());

                return this._threePhotonPhaseCoarsePlusCommand;
            }
        }

        public Visibility ThreePhotonPhaseCoarseVisibility
        {
            get
            {

                return _threePhotonPhaseCoarseVisibility;
            }
            set
            {
                _threePhotonPhaseCoarseVisibility = value;
                OnPropertyChanged("ThreePhotonPhaseCoarseVisibility");

            }
        }

        public int ThreePhotonPhaseFine
        {
            get
            {
                return this._threePhotonControlModel.ThreePhotonPhaseFine;
            }
            set
            {
                this._threePhotonControlModel.ThreePhotonPhaseFine = value;
                OnPropertyChanged("ThreePhotonPhaseFine");
            }
        }

        public ICommand ThreePhotonPhaseFineMinusCommand
        {
            get
            {
                if (this._threePhotonPhaseFineMinusCommand == null)
                    this._threePhotonPhaseFineMinusCommand = new RelayCommand(() => ThreePhotonPhaseFineMinus());

                return this._threePhotonPhaseFineMinusCommand;
            }
        }

        public ICommand ThreePhotonPhaseFinePlusCommand
        {
            get
            {
                if (this._threePhotonPhaseFinePlusCommand == null)
                    this._threePhotonPhaseFinePlusCommand = new RelayCommand(() => ThreePhotonPhaseFinePlus());

                return this._threePhotonPhaseFinePlusCommand;
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
                myPropInfo = typeof(ThreePhotonControlViewModel).GetProperty(propertyName);
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

            string str = string.Empty;
            int itmp = 0;
            if (ndList.Count > 0)
            {
                double dtmp = 0;
                // This is loaded in ScanControlMVM, order matters. It needs to load the status of the 3P checkbox
                // before it loads the Input Ranges.
                //if (XmlManager.GetAttribute(ndList[0], doc, "ThreePhotonEnable", ref str) && (Int32.TryParse(str, out itmp)))
                //{
                //    ThreePhotonEnable = itmp;
                //}

                //whad the wrong spelling, bring the value from the old tag ThreePhotonPhaseCourse if ThreePhotonPhaseCoarse doesn't exist
                if (XmlManager.GetAttribute(ndList[0], doc, "ThreePhotonPhaseCoarse", ref str) && (Int32.TryParse(str, out itmp)))
                {
                    ThreePhotonPhaseCoarse = itmp;
                }
                else if (XmlManager.GetAttribute(ndList[0], doc, "ThreePhotonPhaseCourse", ref str) && (Int32.TryParse(str, out itmp)))
                {
                    ThreePhotonPhaseCoarse = itmp;
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "NumberOfPlanes", ref str) && (Int32.TryParse(str, out itmp)))
                {
                    LSMNumberOfPlanes = itmp;
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "FIR1ManualControlEnable", ref str) && (Int32.TryParse(str, out itmp)))
                {
                    FIR1ManualControlEnable = itmp;
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "DDSEnable", ref str) && (Int32.TryParse(str, out itmp)))
                {
                    DDSEnable = itmp;
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "DDSPhase0", ref str) && (double.TryParse(str, out dtmp)))
                {
                    DDSPhase0 = dtmp;
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "DDSPhase1", ref str) && (double.TryParse(str, out dtmp)))
                {
                    DDSPhase1 = dtmp;
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "DDSAmplitude0", ref str) && (double.TryParse(str, out dtmp)))
                {
                    DDSAmplitude0 = dtmp;
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "DDSAmplitude1", ref str) && (double.TryParse(str, out dtmp)))
                {
                    DDSAmplitude1 = dtmp;
                }
            }

            XmlDocument appDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
            ndList = appDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/ThreePhotonView");
            if (ndList.Count > 0)
            {
                if (XmlManager.GetAttribute(ndList[0], appDoc, "PhaseCoarseVisibility", ref str))
                {
                    ThreePhotonPhaseCoarseVisibility = str.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                }
                if (XmlManager.GetAttribute(ndList[0], appDoc, "FIRSettingsVisibility", ref str))
                {
                    FIRSettingsVisibility = str.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                }
                if (XmlManager.GetAttribute(ndList[0], appDoc, "MultiplaneVisibility", ref str))
                {
                    MultiplaneVisibility = str.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                }
                if (XmlManager.GetAttribute(ndList[0], appDoc, "AcquireDuringGalvoTurnAroundVisibility", ref str))
                {
                    AcquireDuringTurnAroundVisibility = str.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                }

                 if(AcquireDuringTurnAroundVisibility != Visibility.Visible)
                {
                    AcquireDuringTurnAround = 0;
                }
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

            if (ndList.Count > 0)
            {
                XmlManager.SetAttribute(ndList[0], experimentFile, "ThreePhotonEnable", this.ThreePhotonEnable.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "ThreePhotonFreq", this.ThreePhotonFreq.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "ThreePhotonPhaseCoarse", this.ThreePhotonPhaseCoarse.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "ThreePhotonPhaseFine", this.ThreePhotonPhaseFine.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "NumberOfPlanes", this.LSMNumberOfPlanes.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "FIR1ManualControlEnable", (this.FIR1ManualControlEnable).ToString());

                XmlManager.SetAttribute(ndList[0], experimentFile, "DDSEnable", this.DDSEnable.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "DDSPhase0", this.DDSPhase0.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "DDSPhase1", this.DDSPhase1.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "DDSAmplitude0", this.DDSAmplitude0.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "DDSAmplitude1", this.DDSAmplitude1.ToString());
            }
        }

        private void ThreePhotonMeasureFrequency()
        {
            if (0 == ThreePhotonEnable)
            {
                MessageBox.Show("Enable Three Photon Mode first to measure frequency");
                return;
            }

            //not a pretty way to get and set the freq but this process takes a long time
            //this way we only do it once, and when the getter for ThreePhotonFreq is called
            //the frequency will already be determined

            MVMManager.Instance["ScanControlViewModel", "LSMQueryExternalClockRate"] = 1;

            ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("LSMExtClockRate");

            ThreePhotonFreq = this._threePhotonControlModel.ThreePhotonFreq;

            UpdateDwellTimeForThreePhoton();
        }

        private void ThreePhotonPhaseCoarseMinus()
        {
            ThreePhotonPhaseCoarse--;
        }

        private void ThreePhotonPhaseCoarsePlus()
        {
            ThreePhotonPhaseCoarse++;
        }

        private void ThreePhotonPhaseFineMinus()
        {
            ThreePhotonPhaseFine--;
        }

        private void ThreePhotonPhaseFinePlus()
        {
            ThreePhotonPhaseFine++;
        }

        private void UpdateDwellTimeForThreePhoton()
        {
            if (0 != _threePhotonFrequency)
            {
                double currentDwell = (double)MVMManager.Instance["ScanControlViewModel", "LSMPixelDwellTime"];
                double baseDwell = 1000000.0 / _threePhotonFrequency;

                double dwell = baseDwell;

                if (currentDwell > baseDwell)
                {
                    int factor = (int)Math.Round(currentDwell / baseDwell);
                    dwell = baseDwell * factor;
                }

                MVMManager.Instance["ScanControlViewModel", "LSMPixelDwellTimeThreePhotonCall"] = dwell;
            }
            else
            {
                MessageBox.Show("Check Laser Signal", "Alert", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        #endregion Methods
    }
}