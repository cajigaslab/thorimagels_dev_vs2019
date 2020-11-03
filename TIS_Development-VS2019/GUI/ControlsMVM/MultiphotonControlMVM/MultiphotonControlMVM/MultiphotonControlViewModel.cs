namespace MultiphotonControl.ViewModel
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
    using System.Runtime.CompilerServices;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Input;
    using System.Xml;

    using MultiphotonControl.Model;

    using OverlayManager;

    using ThorLogging;

    using ThorSharedTypes;

    public class MultiphotonControlViewModel : ThorSharedTypes.VMBase, ThorSharedTypes.IMVM
    {
        #region Fields

        private readonly MultiphotonControlModel _MultiphotonControlModel;

        private Visibility _beamStabilizerDataVisibility = Visibility.Collapsed;
        private bool _isCollapsed=false;
        private Visibility _laser1FastSeqVisibility;
        private ICommand _laser1GoCommand;
        ICommand _laser1MinusCommand;
        ICommand _laser1PlusCommand;
        private int _laser1SeqEnable;
        private int _laser1SeqPos1;
        private int _laser1SeqPos2;
        private Visibility _laser1Shutter2Visibility;
        ICommand _presetWavelengthAssignCommand;
        ICommand _presetWavelengthCommand;
        private Dictionary<string, PropertyInfo> _properties = new Dictionary<string, PropertyInfo>();
        private ICommand _realignBeamCommand;
        private ICommand _resetFactoryAlignmentCommand;
        private int _selectedWavelengthIndex;
        private bool _beamStabilizerAvailable;
        private bool _shutter1open=false;
        private bool _shutter1close = true;


        #endregion Fields

        #region Constructors

        public MultiphotonControlViewModel()
        {
            this._MultiphotonControlModel = new MultiphotonControlModel();

            BeamStabilizerCentroidInRange = new CustomCollection<PC<int>>();
            BeamStabilizerExpInRange = new CustomCollection<PC<int>>();
            BeamStabilizerPiezoInRange = new CustomCollection<PC<int>>();

            const int BS_CENTROID_COUNT = 4;
            const int BS_EXPOSURE_COUNT = 2;
            const int BS_PIEZO_COUNT = 4;

            for (int i = 0; i < BS_CENTROID_COUNT; i++)
            {
                BeamStabilizerCentroidInRange.Add(new PC<int>(0));
            }
            for (int i = 0; i < BS_EXPOSURE_COUNT; i++)
            {
                BeamStabilizerExpInRange.Add(new PC<int>(0));
            }
            for (int i = 0; i < BS_PIEZO_COUNT; i++)
            {
                BeamStabilizerPiezoInRange.Add(new PC<int>(0));
            }

            const int MAX_PRESET_WAVELENGTHS = 8;

            PresetWavelengthNames = new CustomCollection<PC<string>>();
            for (int i = 0; i < MAX_PRESET_WAVELENGTHS; i++)
            {
                PresetWavelengthNames.Add(new PC<string>("0"));
            }
        }

        #endregion Constructors

        #region Enumerations

        public enum RangeEnum
        {
            NO_COLOR,
            GREEN,
            YELLOW,
            RED
        }

        #endregion Enumerations

        #region Properties

        public bool BeamStabilizerAvailable
        {
            get
            {
                if (_MultiphotonControlModel.BeamStabilizerAvailable == false)
                {
                return _MultiphotonControlModel.BeamStabilizerAvailable;
            }
                else if (_beamStabilizerAvailable == false)
                {
                    return _beamStabilizerAvailable;
        }
                else
                {
                    return true;
                }
            }
            set
            {
                _beamStabilizerAvailable = value;
                OnPropertyChanged("BeamStabilizerAvailable");
            }
        }

        public double BeamStabilizerBPACentroidX
        {
            get
            {
                BeamStabilizerCentroidInRange[0].Value = (Math.Abs(_MultiphotonControlModel.BeamStabilizerBPACentroidX) > _MultiphotonControlModel.BSDeadBand) ? (int)RangeEnum.RED : (int)RangeEnum.GREEN;
                return _MultiphotonControlModel.BeamStabilizerBPACentroidX;
            }
        }

        public double BeamStabilizerBPACentroidY
        {
            get
            {
                BeamStabilizerCentroidInRange[1].Value = (Math.Abs(_MultiphotonControlModel.BeamStabilizerBPACentroidY) > _MultiphotonControlModel.BSDeadBand) ? (int)RangeEnum.RED : (int)RangeEnum.GREEN;
                return _MultiphotonControlModel.BeamStabilizerBPACentroidY;
            }
        }

        public double BeamStabilizerBPAExposure
        {
            get
            {
                BeamStabilizerExpInRange[0].Value = (Math.Abs(_MultiphotonControlModel.BeamStabilizerBPAExposure) > _MultiphotonControlModel.BSExpLimit ||
                                                     Math.Abs(_MultiphotonControlModel.BeamStabilizerBPAExposure) < _MultiphotonControlModel.BSExpLimitMin) ? (int)RangeEnum.RED : (int)RangeEnum.GREEN;
                return _MultiphotonControlModel.BeamStabilizerBPAExposure;
            }
        }

        public double BeamStabilizerBPBCentroidX
        {
            get
            {
                BeamStabilizerCentroidInRange[2].Value = (Math.Abs(_MultiphotonControlModel.BeamStabilizerBPBCentroidX) > _MultiphotonControlModel.BSDeadBand) ? (int)RangeEnum.RED : (int)RangeEnum.GREEN;
                return _MultiphotonControlModel.BeamStabilizerBPBCentroidX;
            }
        }

        public double BeamStabilizerBPBCentroidY
        {
            get
            {
                BeamStabilizerCentroidInRange[3].Value = (Math.Abs(_MultiphotonControlModel.BeamStabilizerBPBCentroidY) > _MultiphotonControlModel.BSDeadBand) ? (int)RangeEnum.RED : (int)RangeEnum.GREEN;
                return _MultiphotonControlModel.BeamStabilizerBPBCentroidY;
            }
        }

        public double BeamStabilizerBPBExposure
        {
            get
            {
                BeamStabilizerExpInRange[1].Value = (Math.Abs(_MultiphotonControlModel.BeamStabilizerBPBExposure) > _MultiphotonControlModel.BSExpLimit ||
                                                     Math.Abs(_MultiphotonControlModel.BeamStabilizerBPBExposure) < _MultiphotonControlModel.BSExpLimitMin) ? (int)RangeEnum.RED : (int)RangeEnum.GREEN;
                return _MultiphotonControlModel.BeamStabilizerBPBExposure;
            }
        }

        public CustomCollection<PC<int>> BeamStabilizerCentroidInRange
        {
            get;
            set;
        }

        public Visibility BeamStabilizerDataVisibility
        {
            get
            {
                return _beamStabilizerDataVisibility;
            }
            set
            {
                this.BeamStabilizerEnableReadData = (Visibility.Visible == value) ? true : false;
                BeamStabilizerAvailable = this.BeamStabilizerEnableReadData;
                _beamStabilizerDataVisibility = value;

                OnPropertyChanged("BeamStabilizerDataVisibility");
            }
        }

        public bool BeamStabilizerEnableReadData
        {
            get
            {
                return _MultiphotonControlModel.BeamStabilizerEnableReadData;
            }
            set
            {
                _MultiphotonControlModel.BeamStabilizerEnableReadData = value;
            }
        }

        public CustomCollection<PC<int>> BeamStabilizerExpInRange
        {
            get;
            set;
        }

        public int BeamStabilizerPiezo1Pos
        {
            get
            {
                BeamStabilizerPiezoInRange[0].Value = (Math.Abs(_MultiphotonControlModel.BeamStabilizerPiezo1Pos) > _MultiphotonControlModel.BSPiezoLimit) ? (int)RangeEnum.RED : (int)RangeEnum.GREEN;
                return _MultiphotonControlModel.BeamStabilizerPiezo1Pos;
            }
        }

        public int BeamStabilizerPiezo2Pos
        {
            get
            {
                BeamStabilizerPiezoInRange[1].Value = (Math.Abs(_MultiphotonControlModel.BeamStabilizerPiezo2Pos) > _MultiphotonControlModel.BSPiezoLimit) ? (int)RangeEnum.RED : (int)RangeEnum.GREEN;
                return _MultiphotonControlModel.BeamStabilizerPiezo2Pos;
            }
        }

        public int BeamStabilizerPiezo3Pos
        {
            get
            {
                BeamStabilizerPiezoInRange[2].Value = (Math.Abs(_MultiphotonControlModel.BeamStabilizerPiezo3Pos) > _MultiphotonControlModel.BSPiezoLimit) ? (int)RangeEnum.RED : (int)RangeEnum.GREEN;
                return _MultiphotonControlModel.BeamStabilizerPiezo3Pos;
            }
        }

        public int BeamStabilizerPiezo4Pos
        {
            get
            {
                BeamStabilizerPiezoInRange[3].Value = (Math.Abs(_MultiphotonControlModel.BeamStabilizerPiezo4Pos) > _MultiphotonControlModel.BSPiezoLimit) ? (int)RangeEnum.RED : (int)RangeEnum.GREEN;
                return _MultiphotonControlModel.BeamStabilizerPiezo4Pos;
            }
        }

        public CustomCollection<PC<int>> BeamStabilizerPiezoInRange
        {
            get;
            set;
        }

        public bool BeamStablizerQuery
        {
            set { _MultiphotonControlModel.BeamStablizerQuery = value; }
        }


        public bool IsCollapsed
        {
            get { return _isCollapsed; }
            set { 
                    _isCollapsed = value;
                    OnPropertyChanged("IsCollapsed");
                }
        }



        public Visibility Laser1FastSeqVisibility
        {
            get
            {
                return _laser1FastSeqVisibility;
            }
            set
            {
                _laser1FastSeqVisibility = value;
                OnPropertyChanged("Laser1FastSeqVisibility");
            }
        }

        public ICommand Laser1GoCommand
        {
            get
            {
                if (this._laser1GoCommand == null)
                    this._laser1GoCommand = new RelayCommand(() => Laser1Go());

                return this._laser1GoCommand;
            }
        }

        public ICommand Laser1MinusCommand
        {
            get
            {
                if (this._laser1MinusCommand == null)
                    this._laser1MinusCommand = new RelayCommand(() => Laser1Position--);

                return this._laser1MinusCommand;
            }
        }

        public ICommand Laser1PlusCommand
        {
            get
            {
                if (this._laser1PlusCommand == null)
                    this._laser1PlusCommand = new RelayCommand(() => Laser1Position++);

                return this._laser1PlusCommand;
            }
        }

        public int Laser1Position
        {
            get
            {
                return this._MultiphotonControlModel.Laser1Position;
            }
            set
            {
                this._MultiphotonControlModel.Laser1Position = value;
                OnPropertyChanged("Laser1Position");
            }
        }

        public int Laser1PositionGo
        {
            get;
            set;
        }

        public int Laser1SeqEnable
        {
            get
            {
                return _laser1SeqEnable;
            }
            set
            {
               
                
               
                if (Laser1FastSeqVisibility==Visibility.Visible && IsCollapsed==false)
                _laser1SeqEnable = value;
                else
                    _laser1SeqEnable = 0;




                OnPropertyChanged("Laser1SeqEnable");
            }
        }

        public int Laser1SeqPos1
        {
            get
            {
                return _laser1SeqPos1;
            }
            set
            {
                _laser1SeqPos1 = value;
                OnPropertyChanged("Laser1SeqPos1");
            }
        }

        public int Laser1SeqPos2
        {
            get
            {
                return _laser1SeqPos2;
            }
            set
            {
                _laser1SeqPos2 = value;
                OnPropertyChanged("Laser1SeqPos2");
            }
        }

        public Visibility Laser1Shutter2Visibility
        {
            get
            {
                return _laser1Shutter2Visibility;
            }
            set
            {
                _laser1Shutter2Visibility = value;
                OnPropertyChanged("Laser1Shutter2Visibility");
            }
        }

        public int LaserShutter2Position
        {
            get
            {
                return this._MultiphotonControlModel.LaserShutter2Position;
            }
            set
            {
                this._MultiphotonControlModel.LaserShutter2Position = value;
                OnPropertyChanged("LaserShutter2Position");
            }
        }

        public Visibility LaserShutter2Visibility
        {
            get
            {
                return (this._MultiphotonControlModel.LaserShutter2Available) ? Visibility.Visible : Visibility.Collapsed;
            }
        }

        public int LaserShutterPosition
        {
            get
            {
                return this._MultiphotonControlModel.LaserShutterPosition;
            }
            set
            {
                this._MultiphotonControlModel.LaserShutterPosition = value;
                   
                OnPropertyChanged("LaserShutterPosition");
            }
        }

        public DateTime LastBeamStablizerUpdate
        {
            get { return _MultiphotonControlModel.LastBeamStablizerUpdate; }
            set { _MultiphotonControlModel.LastBeamStablizerUpdate = value; }
        }

        public ICommand PresetWavelengthAssignCommand
        {
            get
            {
                if (null == _presetWavelengthAssignCommand)
                {
                    _presetWavelengthAssignCommand = new RelayCommandWithParam((x) => AssignCurrentWavelength(x));
                }

                return _presetWavelengthAssignCommand;
            }
        }

        public ICommand PresetWavelengthCommand
        {
            get
            {
                if (null == _presetWavelengthCommand)
                {
                    _presetWavelengthCommand = new RelayCommandWithParam((x) => PresetWavelength(x));
                }

                return _presetWavelengthCommand;
            }
        }

        public CustomCollection<PC<string>> PresetWavelengthNames
        {
            get;
            set;
        }

        public ICommand RealignBeamCommand
        {
            get
            {
                if (null == _realignBeamCommand)
                {
                    _realignBeamCommand = new RelayCommand(() => RealignBeam());
                }

                return _realignBeamCommand;
            }
        }

        public ICommand ResetFactoryAlignmentCommand
        {
            get
            {
                if (null == _resetFactoryAlignmentCommand)
                {
                    _resetFactoryAlignmentCommand = new RelayCommand(() => ResetFactoryAlignment());
                }

                return _resetFactoryAlignmentCommand;
            }
        }


        public int SelectedWavelengthIndex
        {

            get
            {

                return _selectedWavelengthIndex;
            }
            set
            {
                _selectedWavelengthIndex = value;
                OnPropertyChanged("SelectedWavelengthIndex");
            }


        }

        public bool Shutter1Open
        {

            get
            {

                return _shutter1open;
            }
            set
            {
                _shutter1open = value;
                
                OnPropertyChanged("Shutter1Open");
                
            }


        }


        public bool Shutter1Close
        {

            get
            {

                return _shutter1close;
            }
            set
            {
                _shutter1close = value;
                
                OnPropertyChanged("Shutter1Close");
                

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
                myPropInfo = typeof(MultiphotonControlViewModel).GetProperty(propertyName);
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
            XmlNodeList ndList = doc.SelectNodes("/ThorImageExperiment/MultiPhotonLaser");

            if (ndList.Count > 0)
            {
                string str = string.Empty;
                if (XmlManager.GetAttribute(ndList[0], doc, "pos", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        Laser1Position = tmp;
                    }
                }

                str = string.Empty;
                if (XmlManager.GetAttribute(ndList[0], doc, "seqEnable", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        Laser1SeqEnable = tmp;
                    }
                }
                str = string.Empty;
                if (XmlManager.GetAttribute(ndList[0], doc, "seqPos1", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        Laser1SeqPos1 = tmp;
                    }
                }
                str = string.Empty;
                if (XmlManager.GetAttribute(ndList[0], doc, "seqPos2", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        Laser1SeqPos2 = tmp;
                    }
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

        public void PresetWavelength(object obj)
        {
            int index = Convert.ToInt32(obj);

            if (Convert.ToInt32(PresetWavelengthNames[index].Value) > 0)
            {
                Laser1Position = Convert.ToInt32(PresetWavelengthNames[index].Value);
            }

            OnPropertyChanged("Laser1Position");
        }

        public void SetSavedWavelength(int i, int val)
        {
            if (PresetWavelengthNames.Count > i)
            {
                PresetWavelengthNames[i].Value = val.ToString();
            }
            else
            {
                PresetWavelengthNames.Add(val.ToString());
            }
        }

        public void UpdateExpXMLSettings(ref XmlDocument experimentFile)
        {
            XmlNodeList ndList = experimentFile.SelectNodes("/ThorImageExperiment/MultiPhotonLaser");

            if (ndList.Count > 0)
            {
                XmlManager.SetAttribute(ndList[0], experimentFile, "pos", Laser1Position.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "seqEnable", Laser1SeqEnable.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "seqPos1", Laser1SeqPos1.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "seqPos2", Laser1SeqPos2.ToString());
            }
        }

        private void AssignCurrentWavelength(object obj)
        {
            int index = Convert.ToInt32(obj);
            if (PresetWavelengthNames.Count > index)
            {
                PresetWavelengthNames[index].Value = Laser1Position.ToString();
            }
            else
            {
                PresetWavelengthNames.Add(Laser1Position.ToString());
            }

            PersistMultiphotonWavelengths();
        }

        private void Laser1Go()
        {
            Laser1Position = Laser1PositionGo;
        }

        private void PersistMultiphotonWavelengths()
        {
            XmlDocument ApplicationDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
            XmlNodeList ndList = ApplicationDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/MultiphotonView/SavedWavelengths/SavedWavelength");
            if (ndList.Count == PresetWavelengthNames.Count)
            {
                //update XML about altered preset wavelengths
                for (int i = 0; i < PresetWavelengthNames.Count; i++)
                {
                    XmlManager.SetAttribute(ndList[i], ApplicationDoc, "wavelength", PresetWavelengthNames[i].Value.ToString());
                }
            }
            else
            {
                XmlNode CaptureSetup = ApplicationDoc.SelectSingleNode("/ApplicationSettings/DisplayOptions/CaptureSetup");
                XmlNode MultiphotonView = ApplicationDoc.SelectSingleNode("/ApplicationSettings/DisplayOptions/CaptureSetup/MultiphotonView");
                XmlNode SavedWavelengths = ApplicationDoc.SelectSingleNode("/ApplicationSettings/DisplayOptions/CaptureSetup/MultiphotonView/SavedWavelengths");
                if (null == MultiphotonView)
                {
                    MultiphotonView = ApplicationDoc.CreateNode(XmlNodeType.Element, "MultiphotonView", null);
                    XmlAttribute Visibility = ApplicationDoc.CreateAttribute("Visibility");
                    Visibility.Value = "Visible";
                    MultiphotonView.Attributes.Append(Visibility);
                    CaptureSetup.AppendChild(MultiphotonView);
                }

                if (null == SavedWavelengths)
                {
                    SavedWavelengths = ApplicationDoc.CreateNode(XmlNodeType.Element, "SavedWavelengths", null);
                    MultiphotonView.AppendChild(SavedWavelengths);
                }

                if (0 < ndList.Count)
                {
                    SavedWavelengths.RemoveAll();
                }

                for (int i = 0; i < PresetWavelengthNames.Count; i++)
                {
                    XmlNode SavedWavelength = ApplicationDoc.CreateNode(XmlNodeType.Element, "SavedWavelength", null);
                    XmlAttribute Wavelength = ApplicationDoc.CreateAttribute("Wavelength");
                    Wavelength.Value = PresetWavelengthNames[i].Value.ToString();
                    SavedWavelength.Attributes.Append(Wavelength);
                    SavedWavelengths.AppendChild(SavedWavelength);
                }
            }

            MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS);
        }

        private void RealignBeam()
        {
            _MultiphotonControlModel.RealignBeam();
        }

        private void ResetFactoryAlignment()
        {
            _MultiphotonControlModel.ResetFactoryAlignment();
        }

        #endregion Methods
    }
}