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

        bool _disable3PCheckbox = true;
        Visibility _firSettingsVisibility;
        Dictionary<string, PropertyInfo> _properties = new Dictionary<string, PropertyInfo>();
        ICommand _threePhotonMeasureFrequencyCommand;
        int _threePhotonPanelEnable = 1;
        ICommand _threePhotonPhaseCourseMinusCommand;
        ICommand _threePhotonPhaseCoursePlusCommand;
        Visibility _threePhotonPhaseCourseVisibility;
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
                    if (LSMSelectedPlaneIndex >= this._threePhotonControlModel.LSMNumberOfPlanes)
                    {
                        LSMSelectedPlaneIndex = 0;
                    }
                    OnPropertyChanged("LSMNumberOfPlanes");
                }
            }
        }

        public int LSMSelectedPlaneIndex
        {
            get
            {
                return this._threePhotonControlModel.LSMSelectedPlaneIndex;
            }
            set
            {
                this._threePhotonControlModel.LSMSelectedPlaneIndex = value;
                OnPropertyChanged("LSMSelectedPlaneIndex");
            }
        }

        public int ThreePhotonEnable
        {
            get
            {
                return this._threePhotonControlModel.ThreePhotonEnable;
            }
            set
            {
                if (value != this._threePhotonControlModel.ThreePhotonEnable)
                {
                    this._threePhotonControlModel.ThreePhotonEnable = value;
                    OnPropertyChanged("ThreePhotonEnable");
                    ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("LSMExtClockRate");
                    // update the frequency, disable dwell time slider
                    if (1 == value && ResourceManagerCS.Instance.IsThorDAQBoard)
                    {
                        //MVMManager.Instance["ScanControlViewModel", "LSMClockSource"] = value;

                        //not a pretty way to get and set the freq but this process takes a long time
                        //this way we only do it once, and when the getter for ThreePhotonFreq is called
                        //the frequency will already be determined
                        ThreePhotonFreq = this._threePhotonControlModel.ThreePhotonFreq;

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
                    ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("LSMClockSource");
                    ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("InputRange");
                    MVMManager.Instance["ScanControlViewModel", "UpdateDwellTime"] = true;
                    ((IMVM)MVMManager.Instance["ScanControlViewModel", this]).OnPropertyChange("LSMPixelDwellTimeMaxIndex");
                }
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

        public int ThreePhotonPhaseCourse
        {
            get
            {
                return this._threePhotonControlModel.ThreePhotonPhaseCourse;
            }
            set
            {
                this._threePhotonControlModel.ThreePhotonPhaseCourse = value;
                OnPropertyChanged("ThreePhotonPhaseCourse");
            }
        }

        public ICommand ThreePhotonPhaseCourseMinusCommand
        {
            get
            {
                if (this._threePhotonPhaseCourseMinusCommand == null)
                    this._threePhotonPhaseCourseMinusCommand = new RelayCommand(() => ThreePhotonPhaseCourseMinus());

                return this._threePhotonPhaseCourseMinusCommand;
            }
        }

        public ICommand ThreePhotonPhaseCoursePlusCommand
        {
            get
            {
                if (this._threePhotonPhaseCoursePlusCommand == null)
                    this._threePhotonPhaseCoursePlusCommand = new RelayCommand(() => ThreePhotonPhaseCoursePlus());

                return this._threePhotonPhaseCoursePlusCommand;
            }
        }

        public Visibility ThreePhotonPhaseCourseVisibility
        {
            get
            {

                return _threePhotonPhaseCourseVisibility;
            }
            set
            {
                _threePhotonPhaseCourseVisibility = value;
                OnPropertyChanged("ThreePhotonPhaseCourseVisibility");

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
                // This is loaded in ScanControlMVM, order matters. It needs to load the status of the 3P checkbox
                // before it loads the Input Ranges.
                //if (XmlManager.GetAttribute(ndList[0], doc, "ThreePhotonEnable", ref str) && (Int32.TryParse(str, out itmp)))
                //{
                //    ThreePhotonEnable = itmp;
                //}
                if (XmlManager.GetAttribute(ndList[0], doc, "ThreePhotonPhaseCourse", ref str) && (Int32.TryParse(str, out itmp)))
                {
                    ThreePhotonPhaseCourse = itmp;
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "NumberOfPlanes", ref str) && (Int32.TryParse(str, out itmp)))
                {
                    LSMNumberOfPlanes = itmp;
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "SelectedPlane", ref str) && (Int32.TryParse(str, out itmp)))
                {
                    LSMSelectedPlaneIndex = itmp - 1;
                }
            }

            XmlDocument appDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
            ndList = appDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/ThreePhotonView");
            if (ndList.Count > 0)
            {
                if (XmlManager.GetAttribute(ndList[0], appDoc, "PhaseCoarseVisibility", ref str))
                {
                    ThreePhotonPhaseCourseVisibility = str.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                }
                if (XmlManager.GetAttribute(ndList[0], appDoc, "FIRSettingsVisibility", ref str))
                {
                    FIRSettingsVisibility = str.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
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
                XmlManager.SetAttribute(ndList[0], experimentFile, "ThreePhotonPhaseCourse", this.ThreePhotonPhaseCourse.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "ThreePhotonPhaseFine", this.ThreePhotonPhaseFine.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "NumberOfPlanes", this.LSMNumberOfPlanes.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "SelectedPlane", (this.LSMSelectedPlaneIndex + 1).ToString());
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

        private void ThreePhotonPhaseCourseMinus()
        {
            ThreePhotonPhaseCourse--;
            OnPropertyChanged("ThreePhotonPhaseCourse");
        }

        private void ThreePhotonPhaseCoursePlus()
        {
            ThreePhotonPhaseCourse++;
            OnPropertyChanged("ThreePhotonPhaseCourse");
        }

        private void ThreePhotonPhaseFineMinus()
        {
            ThreePhotonPhaseFine--;
            OnPropertyChanged("ThreePhotonPhaseFine");
        }

        private void ThreePhotonPhaseFinePlus()
        {
            ThreePhotonPhaseFine++;
            OnPropertyChanged("ThreePhotonPhaseFine");
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