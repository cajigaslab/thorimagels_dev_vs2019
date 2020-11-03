namespace XYTileControl.ViewModel
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
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Input;
    using System.Xml;

    using OverlayManager;

    using ThorLogging;

    using ThorSharedTypes;

    using XYTileControl.Model;

    public class XYTileControlViewModel : ThorSharedTypes.VMBase, ThorSharedTypes.IMVM
    {
        #region Fields

        private readonly XYTileControlModel _xYTileControlModel;

        private static ReportGoToPosition _goToPositionCallBack;

        private Carrier _currentCarrier = new Carrier();
        private double[] _defaultOverlapXY = { 0.0, 0.0 };
        private int _displayTileGrid = 0;
        private double[] _fP1XYZ = { 0.0, 0.0, 0.0 };
        private double[] _fP2XYZ = { 0.0, 0.0, 0.0 };
        private double[] _fP3XYZ = { 0.0, 0.0, 0.0 };
        private double[] _homePosXYZ = { 0.0, 0.0, 0.0 };
        private Dictionary<string, PropertyInfo> _properties = new Dictionary<string, PropertyInfo>();
        private double _sampleOffsetXMM;
        private double _sampleOffsetYMM;
        private int _sampleType = 0;
        private int _selectedSubColumn = 0;
        private int _selectedSubRow = 0;
        private int _selectedWellColumnCount = 1;
        private int _selectedWellRowCount = 1;
        private ICommand _setSampleOffsetCommand;
        private ICommand _setXZeroCommand;
        private ICommand _setYZeroCommand;
        private int _showInUM = 1;
        int _startColumn = 1;
        int _startRow = 1;
        private int _subColumns = 1;
        private double _subOffsetX = 0;
        private double _subOffsetY = 0;
        private int _subRows = 1;
        double _subSpacingXPercent = 0.0;
        double _subSpacingYPercent = 0.0;
        private int _tileControlMode = 0;
        private int _tiltAdjustment = 0;
        private double _transOffsetXMM;
        private double _transOffsetYMM;
        private double _wellOffsetXMM;
        private double _wellOffsetYMM;
        private int _xDirection = 1;
        private double _xGo;
        private string _xMinusKey;
        private string _xMinusModifier;
        private string _xPlusKey;
        private string _xPlusModifier;
        private ICommand _xPosMinusCommand;
        private ICommand _xPosPlusCommand;
        private double _xStepSize;
        private ObservableCollection<XYPosition> _xYtableData = new ObservableCollection<XYPosition>();
        private string _xZeroKey;
        private string _xZeroModifier;
        private int _yDirection = 1;
        private double _yGo;
        private string _yMinusKey;
        private string _yMinusModifier;
        private string _yPlusKey;
        private string _yPlusModifier;
        private ICommand _yPosMinusCommand;
        private ICommand _yPosPlusCommand;
        private double _yStepSize;
        private string _yZeroKey;
        private string _yZeroModifier;

        #endregion Fields

        #region Constructors

        public XYTileControlViewModel()
        {
            this._xYTileControlModel = new XYTileControlModel();

            //setting the default Stepsize value
            _xStepSize = .100;
            _yStepSize = .100;
            _yGo = 0.0;
            _xGo = 0.0;

            _goToPositionCallBack = new ReportGoToPosition(GoToPosition);
        }

        #endregion Constructors

        #region Delegates

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void ReportGoToPosition(ref double x, ref double y);

        #endregion Delegates

        #region Properties

        public String ActiveXMLPath
        {
            get
            {
                return ResourceManagerCS.GetActiveSettingsFileString();
            }
        }

        public String ApplicationSettingsPath
        {
            get
            {
                return ResourceManagerCS.GetApplicationSettingsFileString();
            }
        }

        public double ConvertRatio
        {
            get { return (1 == ShowInUM) ? (double)Constants.UM_TO_MM : 1.0; }
        }

        public Carrier CurrentCarrier
        {
            get { return _currentCarrier; }
            set
            {
                _currentCarrier = value;
                OnPropertyChanged("CurrentCarrier");
            }
        }

        public double[] DefaultOverlapXY
        {
            get { return _defaultOverlapXY; }
            set
            {
                _defaultOverlapXY = value;
                OnPropertyChanged("DefaultOverlapXY");
            }
        }

        public int DisplayTileGrid
        {
            get
            {
                return _displayTileGrid;
            }
            set
            {
                _displayTileGrid = value;
                OnPropertyChanged("DisplayTileGrid");
            }
        }

        public bool EnableXYRead
        {
            get { return _xYTileControlModel.EnableXYRead; }
        }

        public double[] FP1XYZ
        {
            get { return _fP1XYZ; }
            set
            {
                _fP1XYZ = value;
                OnPropertyChanged("FP1XYZ");
            }
        }

        public double[] FP2XYZ
        {
            get { return _fP2XYZ; }
            set
            {
                _fP2XYZ = value;
                OnPropertyChanged("FP2XYZ");
            }
        }

        public double[] FP3XYZ
        {
            get { return _fP3XYZ; }
            set
            {
                _fP3XYZ = value;
                OnPropertyChanged("FP3XYZ");
            }
        }

        public double[] HomePosXYZ
        {
            get { return _homePosXYZ; }
            set
            {
                _homePosXYZ = value;
                OnPropertyChanged("HomePosXYZ");
            }
        }

        public string LabelUnit
        {
            get { return (1 == ShowInUM) ? "[um]" : "[mm]"; }
        }

        public DateTime LastXUpdate
        {
            get { return _xYTileControlModel.LastXUpdate; }
            set { _xYTileControlModel.LastXUpdate = value; }
        }

        public DateTime LastYUpdate
        {
            get { return _xYTileControlModel.LastYUpdate; }
            set { _xYTileControlModel.LastYUpdate = value; }
        }

        public double SampleOffsetXMM
        {
            get
            {
                return _sampleOffsetXMM;
            }
            set
            {
                _sampleOffsetXMM = value;
                OnPropertyChanged("SampleOffsetXMM");

            }
        }

        public double SampleOffsetYMM
        {
            get
            {
                return _sampleOffsetYMM;
            }
            set
            {
                _sampleOffsetYMM = value;
                OnPropertyChanged("SampleOffsetYMM");

            }
        }

        public double ScanAreaHeight
        {
            get
            {
                return (double)MVMManager.Instance["CaptureSetupViewModel", "FieldSizeHeightUM", (object)1.0];
            }
        }

        public double ScanAreaWidth
        {
            get
            {
                return (double)MVMManager.Instance["CaptureSetupViewModel", "FieldSizeWidthUM", (object)1.0];
            }
        }

        public int SelectedSampleType
        {
            get
            {
                return _sampleType;
            }

            set
            {
                if (value >= 0)
                {
                    _sampleType = value;
                    OnPropertyChanged("SelectedSampleType");
                    OnPropertyChanged("SelectedSampleTypeText");

                }
            }
        }

        public string SelectedSampleTypeText
        {
            get
            {
                return SampleNames.GetName(SampleNames.GetSampleType(SelectedSampleType));
            }
        }

        public int SelectedSubColumn
        {
            get
            {
                return _selectedSubColumn;
            }
            set
            {
                _selectedSubColumn = value;
                OnPropertyChanged("SelectedSubColumn");
            }
        }

        public int SelectedSubRow
        {
            get
            {
                return _selectedSubRow;
            }
            set
            {
                _selectedSubRow = value;
                OnPropertyChanged("SelectedSubRow");
            }
        }

        public int SelectedWellColumn
        {
            get;
            set;
        }

        public int SelectedWellColumnCount
        {
            get
            {
                return _selectedWellColumnCount;
            }
            set
            {
                _selectedWellColumnCount = value;
            }
        }

        public int SelectedWellRow
        {
            get;
            set;
        }

        public int SelectedWellRowCount
        {
            get
            {
                return _selectedWellRowCount;
            }
            set
            {
                _selectedWellRowCount = value;
            }
        }

        public ICommand SetSampleOffsetCommand
        {
            get
            {
                if (this._setSampleOffsetCommand == null)
                    this._setSampleOffsetCommand = new RelayCommand(() => SetSampleOffset());

                return this._setSampleOffsetCommand;
            }
        }

        public ICommand SetXZeroCommand
        {
            get
            {
                if (this._setXZeroCommand == null)
                    this._setXZeroCommand = new RelayCommand(() => SetXZero());

                return this._setXZeroCommand;
            }
        }

        public ICommand SetYZeroCommand
        {
            get
            {
                if (this._setYZeroCommand == null)
                    this._setYZeroCommand = new RelayCommand(() => SetYZero());

                return this._setYZeroCommand;
            }
        }

        public int ShowInUM
        {
            get
            {
                return _showInUM;
            }
            set
            {
                _showInUM = value;
            }
        }

        public int StartColumn
        {
            get { return _startColumn; }
            set { _startColumn = value; }
        }

        public int StartRow
        {
            get { return _startRow; }
            set { _startRow = value; }
        }

        public int SubColumns
        {
            get
            {
                return this._subColumns;
            }
            set
            {
                if ((value <= SubColumnsMax) && (value > 0))
                {
                    this._subColumns = value;
                    OnPropertyChanged("SubColumns");

                }
            }
        }

        public int SubColumnsMax
        {
            get;
            set;
        }

        public double SubOffsetX
        {
            get
            {
                return _subOffsetX; //MMPerPixel * LSMPixelX * (1 + SubSpacingXPercent / 100.0);
            }
            set
            {
                _subOffsetX = value;
                OnPropertyChanged("SubOffsetX");

            }
        }

        public double SubOffsetY
        {
            get
            {
                return _subOffsetY; //MMPerPixel * LSMPixelY * (1 + SubSpacingYPercent / 100.0);
            }
            set
            {
                _subOffsetY = value;
                OnPropertyChanged("SubOffsetY");

            }
        }

        public int SubRows
        {
            get
            {
                return this._subRows;
            }
            set
            {
                if ((value <= SubRowsMax) && (value > 0))
                {
                    this._subRows = value;
                    OnPropertyChanged("SubRows");

                }
            }
        }

        public int SubRowsMax
        {
            get;
            set;
        }

        public double SubSpacingXPercent
        {
            get
            {
                return this._subSpacingXPercent;
            }
            set
            {
                if ((-50.0 <= value) && (50.0 >= value)) //((-99.0 <= value) && (1000.0 >= value))
                {
                    this._subSpacingXPercent = value;
                    OnPropertyChanged("SubSpacingXPercent");
                    OnPropertyChanged("SubOffsetX");

                }
            }
        }

        public double SubSpacingYPercent
        {
            get
            {
                return this._subSpacingYPercent;
            }
            set
            {
                if ((-50.0 <= value) && (50.0 >= value)) //((-99.0 <= value) && (1000.0 >= value))
                {
                    this._subSpacingYPercent = value;
                    OnPropertyChanged("SubSpacingYPercent");
                    OnPropertyChanged("SubOffsetY");

                }
            }
        }

        public int TileControlMode
        {
            get
            {
                return _tileControlMode;
            }
            set
            {
                _tileControlMode = value;
                OnPropertyChanged("TileControlMode");
            }
        }

        public int TiltAdjustment
        {
            get { return _tiltAdjustment; }
            set
            {
                _tiltAdjustment = value;
                OnPropertyChanged("TiltAdjustment");
            }
        }

        public double TransOffsetXMM
        {
            get
            {
                return this._transOffsetXMM;
            }
            set
            {
                this._transOffsetXMM = value;
                OnPropertyChanged("TransOffsetXMM");

            }
        }

        public double TransOffsetYMM
        {
            get
            {
                return this._transOffsetYMM;
            }
            set
            {
                this._transOffsetYMM = value;
                OnPropertyChanged("TransOffsetYMM");

            }
        }

        public string UpdatePositions
        {
            set
            {
                _xYTileControlModel.UpdatePositions = value;
            }
        }

        public double WellOffsetXMM
        {
            get
            {
                return _wellOffsetXMM;
            }
            set
            {
                _wellOffsetXMM = value;
                OnPropertyChanged("WellOffsetXMM");

            }
        }

        public double WellOffsetYMM
        {
            get
            {
                return _wellOffsetYMM;
            }
            set
            {
                _wellOffsetYMM = value;
                OnPropertyChanged("WellOffsetYMM");

            }
        }

        public int XDirection
        {
            get
            {
                return _xDirection;
            }
            set
            {
                _xDirection = value;
            }
        }

        public double XGo
        {
            get
            {
                return _xGo;
            }
            set
            {
                Decimal dec = new Decimal(value);
                _xGo = Decimal.ToDouble(Decimal.Round(dec, 5));
            }
        }

        public double XMax
        {
            get
            {
                return _xYTileControlModel.XMax;
            }
        }

        public double XMin
        {
            get
            {
                return _xYTileControlModel.XMin;
            }
        }

        public string XMinusKey
        {
            get { return _xMinusKey; }
            set { _xMinusKey = value; OnPropertyChanged("XMinusKey"); }
        }

        public string XMinusModifier
        {
            get { return _xMinusModifier; }
            set { _xMinusModifier = value; OnPropertyChanged("XMinusModifier"); }
        }

        public string XPlusKey
        {
            get { return _xPlusKey; }
            set { _xPlusKey = value; OnPropertyChanged("XPlusKey"); }
        }

        public string XPlusModifier
        {
            get { return _xPlusModifier; }
            set { _xPlusModifier = value; OnPropertyChanged("XPlusModifier"); }
        }

        public double XPosition
        {
            get
            {
                return _xYTileControlModel.XPosition;
            }
            set
            {
                _xYTileControlModel.XPosition = value;
                OnPropertyChanged("XPosition");
                OnPropertyChanged("XPosOutOfBounds");
            }
        }

        public ICommand XPosMinusCommand
        {
            get
            {
                if (this._xPosMinusCommand == null)
                    this._xPosMinusCommand = new RelayCommand(() => XPosMinus());

                return this._xPosMinusCommand;
            }
        }

        public bool XPosOutOfBounds
        {
            get
            {
                bool ret;
                if (_xYTileControlModel.XPosition < _xYTileControlModel.XMin || _xYTileControlModel.XPosition > _xYTileControlModel.XMax)
                    ret = true;
                else
                    ret = false;
                return ret;
            }
        }

        public ICommand XPosPlusCommand
        {
            get
            {
                if (this._xPosPlusCommand == null)
                    this._xPosPlusCommand = new RelayCommand(() => XPosPlus());

                return this._xPosPlusCommand;
            }
        }

        public double XStepSize
        {
            get
            {
                return _xStepSize;
            }
            set
            {
                Decimal dec = new Decimal(value);
                _xStepSize = Decimal.ToDouble(Decimal.Round(dec, 5));

                OnPropertyChanged("XStepSize");
            }
        }

        public double XYControlPanelScale
        {
            get
            {
                return (ResourceManagerCS.Instance.TabletModeEnabled) ? 0.85 : 1;
            }
        }

        public double XYPanelScale
        {
            get
            {
                return (ResourceManagerCS.Instance.TabletModeEnabled) ? 1.04 : 1;
            }
        }

        public ObservableCollection<XYPosition> XYtableData
        {
            get { return _xYtableData; }
            set { _xYtableData = value; }
        }

        public string XZeroKey
        {
            get { return _xZeroKey; }
            set { _xZeroKey = value; OnPropertyChanged("XZeroKey"); }
        }

        public string XZeroModifier
        {
            get { return _xZeroModifier; }
            set { _xZeroModifier = value; OnPropertyChanged("XZeroModifier"); }
        }

        /// <summary>
        /// Gets the x zero visibility.
        /// </summary>
        /// <value>The x zero visibility.</value>
        public Visibility XZeroVisibility
        {
            get
            {
                if (ResourceManagerCS.Instance.TabletModeEnabled)
                {
                    return Visibility.Visible;
                }
                XmlDocument appSettings = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

                XmlNodeList ndList = appSettings.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/XYView");
                if (ndList.Count > 0)
                {
                    string tmp = string.Empty;

                    if (XmlManager.GetAttribute(ndList[0], appSettings, "xZeroVisibility", ref tmp))
                    {
                        return tmp.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                    }
                    else
                    {
                        return Visibility.Collapsed;
                    }
                }

                return Visibility.Collapsed;
            }
        }

        public int YDirection
        {
            get
            {
                return _yDirection;
            }
            set
            {
                _yDirection = value;
            }
        }

        public double YGo
        {
            get
            {
                return _yGo;
            }
            set
            {
                Decimal dec = new Decimal(value);
                _yGo = Decimal.ToDouble(Decimal.Round(dec, 5));
            }
        }

        public double YMax
        {
            get
            {
                return _xYTileControlModel.YMax;
            }
        }

        public double YMin
        {
            get
            {
                return _xYTileControlModel.YMin;
            }
        }

        public string YMinusKey
        {
            get { return _yMinusKey; }
            set { _yMinusKey = value; OnPropertyChanged("YMinusKey"); }
        }

        public string YMinusModifier
        {
            get { return _yMinusModifier; }
            set { _yMinusModifier = value; OnPropertyChanged("YMinusModifier"); }
        }

        public string YPlusKey
        {
            get { return _yPlusKey; }
            set { _yPlusKey = value; OnPropertyChanged("YPlusKey"); }
        }

        public string YPlusModifier
        {
            get { return _yPlusModifier; }
            set { _yPlusModifier = value; OnPropertyChanged("YPlusModifier"); }
        }

        public double YPosition
        {
            get
            {
                return _xYTileControlModel.YPosition;
            }
            set
            {
                _xYTileControlModel.YPosition = value;
                OnPropertyChanged("YPosition");
                OnPropertyChanged("YPosOutOfBounds");
            }
        }

        public ICommand YPosMinusCommand
        {
            get
            {
                if (this._yPosMinusCommand == null)
                    this._yPosMinusCommand = new RelayCommand(() => YPosMinus());

                return this._yPosMinusCommand;
            }
        }

        public bool YPosOutOfBounds
        {
            get
            {
                bool ret;
                if (_xYTileControlModel.YPosition < _xYTileControlModel.YMin || _xYTileControlModel.YPosition > _xYTileControlModel.YMax)
                    ret = true;
                else
                    ret = false;
                return ret;
            }
        }

        public ICommand YPosPlusCommand
        {
            get
            {
                if (this._yPosPlusCommand == null)
                    this._yPosPlusCommand = new RelayCommand(() => YPosPlus());

                return this._yPosPlusCommand;
            }
        }

        public double YStepSize
        {
            get
            {
                return _yStepSize;
            }
            set
            {
                Decimal dec = new Decimal(value);
                _yStepSize = Decimal.ToDouble(Decimal.Round(dec, 5));

                OnPropertyChanged("YStepSize");
            }
        }

        public string YZeroKey
        {
            get { return _yZeroKey; }
            set { _yZeroKey = value; OnPropertyChanged("YZeroKey"); }
        }

        public string YZeroModifier
        {
            get { return _yZeroModifier; }
            set { _yZeroModifier = value; OnPropertyChanged("YZeroModifier"); }
        }

        /// <summary>
        /// Gets the y zero visibility.
        /// </summary>
        /// <value>The y zero visibility.</value>
        public Visibility YZeroVisibility
        {
            get
            {
                if (ResourceManagerCS.Instance.TabletModeEnabled)
                {
                    return Visibility.Visible;
                }
                XmlDocument appSettings = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

                XmlNodeList ndList = appSettings.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/XYView");
                if (ndList.Count > 0)
                {
                    string tmp = string.Empty;

                    if (XmlManager.GetAttribute(ndList[0], appSettings, "yZeroVisibility", ref tmp))
                    {
                        return tmp.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                    }
                    else
                    {
                        return Visibility.Collapsed;
                    }
                }

                return Visibility.Collapsed;
            }
        }

        #endregion Properties

        #region Indexers

        public object this[string propertyName]
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

        public object this[string propertyName, object defaultObject = null]
        {
            get
            {
                PropertyInfo myPropInfo = GetPropertyInfo(propertyName);
                return (null != myPropInfo) ? myPropInfo.GetValue(this) : defaultObject;
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
                myPropInfo = typeof(XYTileControlViewModel).GetProperty(propertyName);
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

            XmlNodeList ndList = doc.SelectNodes("/ThorImageExperiment/Sample");

            if (ndList.Count > 0)
            {
                string str = string.Empty;

                if (XmlManager.GetAttribute(ndList[0], doc, "type", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        SelectedSampleType = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "offsetXMM", ref str))
                {
                    double tmp = 0;
                    if (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmp))
                    {
                        SampleOffsetXMM = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "offsetYMM", ref str))
                {
                    double tmp = 0;
                    if (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmp))
                    {
                        SampleOffsetYMM = tmp;
                    }
                }
            }

            ndList = doc.SelectNodes("/ThorImageExperiment/Sample/Wells");

            if (ndList.Count > 0)
            {
                string str = string.Empty;

                if (XmlManager.GetAttribute(ndList[0], doc, "startRow", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        StartRow = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "startColumn", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        StartColumn = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "wellOffsetXMM", ref str))
                {
                    double tmp = 0;
                    if (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmp))
                    {
                        WellOffsetXMM = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "wellOffsetYMM", ref str))
                {
                    double tmp = 0;
                    if (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmp))
                    {
                        WellOffsetYMM = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "rows", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        SelectedWellRowCount = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "columns", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        SelectedWellColumnCount = tmp;
                    }
                }
            }

            ndList = doc.SelectNodes("/ThorImageExperiment/Sample/Wells/SubImages");

            if (ndList.Count > 0)
            {
                string str = string.Empty;
                if (XmlManager.GetAttribute(ndList[0], doc, "transOffsetXMM", ref str))
                {
                    double tmp = 0;
                    if (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmp))
                    {
                        TransOffsetXMM = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "transOffsetYMM", ref str))
                {
                    double tmp = 0;
                    if (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmp))
                    {
                        TransOffsetYMM = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "subRows", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        SubRows = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "subColumns", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        SubColumns = tmp;
                    }
                }

                if (XmlManager.GetAttribute(ndList[0], doc, "subOffsetXMM", ref str))
                {
                    SubOffsetX = (int)MVMManager.Instance["AreaControlViewModel", "LSMPixelX", (object)32] * (double)MVMManager.Instance["AreaControlViewModel", "MMPerPixel", (object)0.01];

                    if (SubOffsetX == 0)
                    {
                        SubSpacingXPercent = 0;
                    }
                    else
                    {
                        double tmp = 0;
                        if (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmp))
                        {
                            double val = tmp / SubOffsetX;
                            SubSpacingXPercent = 100.0 * Convert.ToDouble(Decimal.Round(Convert.ToDecimal(val - 1.0), 2), CultureInfo.InvariantCulture);
                        }
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "subOffsetYMM", ref str))
                {
                    SubOffsetY = (int)MVMManager.Instance["AreaControlViewModel", "LSMPixelY", (object)32] * (double)MVMManager.Instance["AreaControlViewModel", "MMPerPixel", (object)0.01];

                    if (SubOffsetY == 0)
                    {
                        SubSpacingYPercent = 0;
                    }
                    else
                    {
                        double tmp = 0;
                        if (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmp))
                        {
                            double val = tmp / SubOffsetY;
                            SubSpacingYPercent = 100.0 * Convert.ToDouble(Decimal.Round(Convert.ToDecimal(val - 1.0), 2), CultureInfo.InvariantCulture);
                        }
                    }
                }
            }

            ndList = doc.SelectNodes("/ThorImageExperiment/Sample");

            if (ndList.Count > 0)
            {
                string str = string.Empty;
                if (XmlManager.GetAttribute(ndList[0], doc, "type", ref str))
                {
                    int tmp = 0;
                    if (Int32.TryParse(str, out tmp))
                    {
                        SelectedSampleType = tmp;
                    }
                }
            }

            OnPropertyChanged("");
        }

        public bool MoveToWellSite(int row, int col, int subRow, int subCol, double transOffsetX, double transOffsetY)
        {
            bool ret;

            SubOffsetX = ((double)MVMManager.Instance["AreaControlViewModel", "MMPerPixel"] * (int)MVMManager.Instance["AreaControlViewModel", "LSMPixelX"] * (1 + SubSpacingXPercent / 100.0));
            SubOffsetY = ((double)MVMManager.Instance["AreaControlViewModel", "MMPerPixel"] * (int)MVMManager.Instance["AreaControlViewModel", "LSMPixelX"] * (1 + SubSpacingYPercent / 100.0));

            ret = GoToWellSiteAndOffset(row, col, subRow, subCol, SampleOffsetXMM, SampleOffsetYMM, WellOffsetXMM * XDirection, WellOffsetYMM * YDirection, transOffsetX * XDirection, transOffsetY * YDirection, SubOffsetX * XDirection, SubOffsetY * YDirection, _goToPositionCallBack);

            OnPropertyChanged("XPosition");
            OnPropertyChanged("XPosOutOfBounds");
            OnPropertyChanged("YPosition");
            OnPropertyChanged("YPosOutOfBounds");
            return ret;
        }

        public void OnPropertyChange(string propertyName)
        {
            if (null != GetPropertyInfo(propertyName))
            {
                OnPropertyChanged(propertyName);
            }
        }

        public void SetTransOffsetCorner1()
        {
            this.TransOffsetXMM = this.XPosition - this.SampleOffsetXMM;
            this.TransOffsetYMM = this.YPosition - this.SampleOffsetYMM;
        }

        public void SetTransOffsetCorner2()
        {
            this.TransOffsetXMM = (TransOffsetXMM < (XPosition - SampleOffsetXMM) ? TransOffsetXMM : (XPosition - SampleOffsetXMM));
            this.TransOffsetYMM = (TransOffsetYMM < (YPosition - SampleOffsetYMM) ? TransOffsetYMM : (YPosition - SampleOffsetYMM));
        }

        public void SetXZero()
        {
            _xYTileControlModel.SetXZero();

            OnPropertyChanged("XPosition");
            OnPropertyChanged("XPosOutOfBounds");
        }

        public void SetYZero()
        {
            _xYTileControlModel.SetYZero();

            OnPropertyChanged("YPosition");
            OnPropertyChanged("YPosOutOfBounds");
        }

        public void UpdateExpXMLSettings(ref XmlDocument experimentFile)
        {
            var sampleRootNode = experimentFile.SelectSingleNode("/ThorImageExperiment/Sample");
            var root = experimentFile.SelectSingleNode("/ThorImageExperiment");
            if (sampleRootNode != null)
            {
                root.RemoveChild(sampleRootNode);
            }

            XmlNode node = experimentFile.CreateElement("Sample");
            root.AppendChild(node);

            sampleRootNode = experimentFile.SelectSingleNode("/ThorImageExperiment/Sample");

            sampleRootNode.RemoveAll();// Removes all the child nodes and/or attributes of the current node.
            XmlManager.SetAttribute(sampleRootNode, experimentFile, "homeOffsetX", HomePosXYZ[0].ToString());
            XmlManager.SetAttribute(sampleRootNode, experimentFile, "homeOffsetY", HomePosXYZ[1].ToString());
            XmlManager.SetAttribute(sampleRootNode, experimentFile, "homeOffsetZ", HomePosXYZ[2].ToString());
            XmlManager.SetAttribute(sampleRootNode, experimentFile, "overlapX", DefaultOverlapXY[0].ToString());
            XmlManager.SetAttribute(sampleRootNode, experimentFile, "overlapY", DefaultOverlapXY[1].ToString());
            XmlManager.SetAttribute(sampleRootNode, experimentFile, "name", CurrentCarrier.Name);
            XmlManager.SetAttribute(sampleRootNode, experimentFile, "type", CurrentCarrier.Type.ToString());
            XmlManager.SetAttribute(sampleRootNode, experimentFile, "width", CurrentCarrier.Width.ToString());
            XmlManager.SetAttribute(sampleRootNode, experimentFile, "height", CurrentCarrier.Height.ToString());
            XmlManager.SetAttribute(sampleRootNode, experimentFile, "row", CurrentCarrier.Template.Row.ToString());
            XmlManager.SetAttribute(sampleRootNode, experimentFile, "column", CurrentCarrier.Template.Col.ToString());
            XmlManager.SetAttribute(sampleRootNode, experimentFile, "diameter", CurrentCarrier.Template.Diameter.ToString());
            XmlManager.SetAttribute(sampleRootNode, experimentFile, "centerToCenterX", CurrentCarrier.Template.CenterToCenterX.ToString());
            XmlManager.SetAttribute(sampleRootNode, experimentFile, "centerToCenterY", CurrentCarrier.Template.CenterToCenterY.ToString());
            XmlManager.SetAttribute(sampleRootNode, experimentFile, "topLeftCenterOffsetX", CurrentCarrier.Template.TopLeftCenterOffsetX.ToString());
            XmlManager.SetAttribute(sampleRootNode, experimentFile, "topLeftCenterOffsetY", CurrentCarrier.Template.TopLeftCenterOffsetY.ToString());
            XmlManager.SetAttribute(sampleRootNode, experimentFile, "WellShape", CurrentCarrier.Template.Shape.ToString());
            XmlManager.SetAttribute(sampleRootNode, experimentFile, "WellWidth", CurrentCarrier.Template.Width.ToString());
            XmlManager.SetAttribute(sampleRootNode, experimentFile, "WellHeight", CurrentCarrier.Template.Height.ToString());
            XmlManager.SetAttribute(sampleRootNode, experimentFile, "tiltAdjustment", TiltAdjustment.ToString());
            XmlManager.SetAttribute(sampleRootNode, experimentFile, "fPt1XMM", FP1XYZ[0].ToString());
            XmlManager.SetAttribute(sampleRootNode, experimentFile, "fPt1YMM", FP1XYZ[1].ToString());
            XmlManager.SetAttribute(sampleRootNode, experimentFile, "fPt1ZMM", FP1XYZ[2].ToString());
            XmlManager.SetAttribute(sampleRootNode, experimentFile, "fPt2XMM", FP2XYZ[0].ToString());
            XmlManager.SetAttribute(sampleRootNode, experimentFile, "fPt2YMM", FP2XYZ[1].ToString());
            XmlManager.SetAttribute(sampleRootNode, experimentFile, "fPt2ZMM", FP2XYZ[2].ToString());
            XmlManager.SetAttribute(sampleRootNode, experimentFile, "fPt3XMM", FP3XYZ[0].ToString());
            XmlManager.SetAttribute(sampleRootNode, experimentFile, "fPt3YMM", FP3XYZ[1].ToString());
            XmlManager.SetAttribute(sampleRootNode, experimentFile, "fPt3ZMM", FP3XYZ[2].ToString());
            XmlManager.SetAttribute(sampleRootNode, experimentFile, "DisplayTileGrid", DisplayTileGrid.ToString());
            XmlManager.SetAttribute(sampleRootNode, experimentFile, "initialStageLocationX", XPosition.ToString());
            XmlManager.SetAttribute(sampleRootNode, experimentFile, "initialStageLocationY", YPosition.ToString());

            for (int j = 0; j < XYtableData.Count; j++)
            {
                XYPosition xyPosition = XYtableData[j];
                node = null;
                XmlNodeList wellList = experimentFile.SelectNodes("/ThorImageExperiment/Sample/Wells");
                if (wellList.Count == 0)// no well is existed
                {
                    node = experimentFile.CreateElement("Wells");
                    XmlManager.SetAttribute(node, experimentFile, "location", xyPosition.Well);
                    sampleRootNode.AppendChild(node);
                }
                else
                {
                    for (int i = 0; i < wellList.Count; i++)
                    {
                        if (xyPosition.Well == wellList[i].Attributes["location"].Value) // find the node
                        {
                            node = wellList[i];
                            break;
                        }
                    }
                    if (node == null)/// no tagged well is exisred
                    {
                        node = experimentFile.CreateElement("Wells");
                        XmlManager.SetAttribute(node, experimentFile, "location", xyPosition.Well);
                        sampleRootNode.AppendChild(node);
                    }
                }
                XmlNode subImage = experimentFile.CreateElement("SubImages");
                XmlManager.SetAttribute(subImage, experimentFile, "name", xyPosition.Name);
                XmlManager.SetAttribute(subImage, experimentFile, "isEnabled", xyPosition.IsEnabled.ToString());
                XmlManager.SetAttribute(subImage, experimentFile, "subRows", xyPosition.TileRow);
                XmlManager.SetAttribute(subImage, experimentFile, "subColumns", xyPosition.TileCol);
                XmlManager.SetAttribute(subImage, experimentFile, "transOffsetXMM", xyPosition.X);
                XmlManager.SetAttribute(subImage, experimentFile, "transOffsetYMM", xyPosition.Y);
                XmlManager.SetAttribute(subImage, experimentFile, "transOffsetZMM", xyPosition.Z);
                XmlManager.SetAttribute(subImage, experimentFile, "overlapX", xyPosition.OverlapX);
                XmlManager.SetAttribute(subImage, experimentFile, "overlapY", xyPosition.OverlapY);
                node.AppendChild(subImage);
            }
        }

        [DllImport(".\\Modules_Native\\Sample.dll", EntryPoint = "CreatePlateMosaicSample")]
        private static extern bool CreatePlateMosaicSample(int startRow, int startColumn, int totalRows, int totalCols, double sampleOffsetX, double sampleOffsetY, int wellRows, int wellCols, double wellOffsetX, double wellOffsetY, int subRows, int subCols, double subOffsetX, double subOffsetY, double transOffsetX, double transOffsetY);

        private static void GoToPosition(ref double x, ref double y)
        {
            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, String.Format("Go to x:{0} y:{1}", x, y));

            ResourceManagerCS.SetDeviceParamDouble((int)SelectedHardware.SELECTED_XYSTAGE, (int)IDevice.Params.PARAM_X_POS, x, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT);
            ResourceManagerCS.SetDeviceParamDouble((int)SelectedHardware.SELECTED_XYSTAGE, (int)IDevice.Params.PARAM_Y_POS, y, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT);
        }

        [DllImport(".\\Modules_Native\\Sample.dll", EntryPoint = "GoToWellSiteAndOffset")]
        private static extern bool GoToWellSiteAndOffset(int row, int col, int subRow, int subCol, double sampleOffsetX, double sampleOffsetY, double wellOffsetX, double wellOffsetY, double transOffsetX, double transOffsetY, double subOffsetX, double subOffsetY, ReportGoToPosition reportGoToPosition);

        private void SetSampleOffset()
        {
            this.SampleOffsetXMM = this.XPosition;
            this.SampleOffsetYMM = this.YPosition;

            //zero out the translation offset
            this.TransOffsetXMM = 0.0;
            this.TransOffsetYMM = 0.0;

            this.SelectedSubRow = 0;
            this.SelectedWellColumn = 0;
            this.SelectedWellRow = 0;
            this.SelectedWellColumn = 0;

            MoveToWellSite(0, 0, 0, 0, 0, 0);
        }

        /// <summary>
        /// 
        /// </summary>
        private void XPosMinus()
        {
            _xYTileControlModel.XPosition = _xYTileControlModel.XPosition - this.XStepSize;
            OnPropertyChanged("XPosition");
            OnPropertyChanged("XPosOutOfBounds");
        }

        /// <summary>
        /// 
        /// </summary>
        private void XPosPlus()
        {
            _xYTileControlModel.XPosition = _xYTileControlModel.XPosition + this.XStepSize;
            OnPropertyChanged("XPosition");
            OnPropertyChanged("XPosOutOfBounds");
        }

        /// <summary>
        /// 
        /// </summary>
        private void YPosMinus()
        {
            _xYTileControlModel.YPosition = _xYTileControlModel.YPosition - this.YStepSize;
            OnPropertyChanged("YPosition");
            OnPropertyChanged("YPosOutOfBounds");
        }

        /// <summary>
        /// 
        /// </summary>
        private void YPosPlus()
        {
            _xYTileControlModel.YPosition = _xYTileControlModel.YPosition + this.YStepSize;
            OnPropertyChanged("YPosition");
            OnPropertyChanged("YPosOutOfBounds");
        }

        #endregion Methods

        #region Other

        //public double ZPosition
        //{
        //    get
        //    {
        //        return _xYTileControlModel.ZPosition;
        //    }
        //    set
        //    {
        //        _xYTileControlModel.ZPosition = value;
        //        OnPropertyChanged("ZPosition");
        //        OnPropertyChanged("ZPosOutOfBounds");
        //    }
        //}

        #endregion Other
    }
}