namespace HardwareSetupUserControl
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.ComponentModel;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Shapes;
    using System.Xml;

    /// <summary>
    /// Interaction logic for SetupObjectives.xaml
    /// </summary>
    public partial class SetupObjectives : Window, INotifyPropertyChanged
    {
        #region Fields

        private const double TYPE2INCREMENT = 0.2;

        private int _beamExp;
        private int _beamExp2;
        private int _beamWavelength;
        private int _beamWavelength2;
        private XmlDocument _hardwareDoc = null;
        private double _maxExp = 2.0;
        private double _minExp = 1.0;
        private double _objectiveNA;
        private string _objectiveName;
        private int _objectiveSelected;
        private double _objecvtiveMagnification;
        private ObservableCollection<string> _objs = new ObservableCollection<string>();
        private Visibility _saveVisible;
        private int _turretPosition;
        private double _zAxisEscapeDistance;
        private int _zAxisToEscape;

        #endregion Fields

        #region Constructors

        public SetupObjectives()
        {
            InitializeComponent();
            DataContext = this;

            this.Loaded += SetupObjectives_Loaded;
        }

        #endregion Constructors

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public int BeamExp
        {
            get
            {
                return _beamExp;
            }
            set
            {
                _beamExp = value;
                OnPropertyChanged("BeamExp");
                SaveVisible = Visibility.Visible;
            }
        }

        public int BeamExp2
        {
            get
            {
                return _beamExp2;
            }
            set
            {
                _beamExp2 = value;
                OnPropertyChanged("BeamExp2");
                SaveVisible = Visibility.Visible;
            }
        }

        public int BeamWavelength
        {
            get
            {
                return _beamWavelength;
            }
            set
            {
                _beamWavelength = value;
                OnPropertyChanged("BeamWavelength");
                SaveVisible = Visibility.Visible;
            }
        }

        public int BeamWavelength2
        {
            get
            {
                return _beamWavelength2;
            }
            set
            {
                _beamWavelength2 = value;
                OnPropertyChanged("BeamWavelength2");
                SaveVisible = Visibility.Visible;
            }
        }

        public XmlDocument HardwareDoc
        {
            get
            {
                return _hardwareDoc;
            }
            set
            {
                _hardwareDoc = value;

                XmlNodeList ndList = _hardwareDoc.SelectNodes("/HardwareSettings/Objectives/Objective");

                _objs.Clear();

                for (int i = 0; i < ndList.Count; i++)
                {
                    string str = string.Empty;
                    if (GetAttribute(ndList[i], _hardwareDoc, "name", ref str))
                    {
                        _objs.Add(str);
                    }
                }

                OnPropertyChanged("Objectives");
            }
        }

        public double MaxExp
        {
            set
            {
                _maxExp = value;
            }
        }

        public double MinExp
        {
            set
            {
                _minExp = value;
            }
        }

        public double ObjectiveMagnification
        {
            get
            {
                return _objecvtiveMagnification;
            }
            set
            {
                _objecvtiveMagnification = value;
                OnPropertyChanged("ObjectiveMagnification");
                SaveVisible = Visibility.Visible;
            }
        }

        public double ObjectiveNA
        {
            get
            {
                return _objectiveNA;
            }
            set
            {
                _objectiveNA = value;
                OnPropertyChanged("ObjectiveNA");
                SaveVisible = Visibility.Visible;
            }
        }

        public string ObjectiveName
        {
            get
            {
                return _objectiveName;
            }
            set
            {
                _objectiveName = value;
                OnPropertyChanged("ObjectiveName");
                SaveVisible = Visibility.Visible;
            }
        }

        public ObservableCollection<string> Objectives
        {
            get
            {
                return _objs;
            }
            set
            {
                _objs = value;
                OnPropertyChanged("Objectives");
            }
        }

        public int ObjectiveSelected
        {
            get
            {
                return _objectiveSelected;
            }
            set
            {
                _objectiveSelected = value;
                OnPropertyChanged("ObjectiveSelected");
                LoadObjSettingsFromIndex(_objectiveSelected);
            }
        }

        public Visibility SaveVisible
        {
            get
            {
                return _saveVisible;
            }
            set
            {
                _saveVisible = value;
                OnPropertyChanged("SaveVisible");
            }
        }

        public int TurretPosition
        {
            get
            {
                return _turretPosition;
            }
            set
            {
                _turretPosition = value;
                OnPropertyChanged("TurretPosition");
                SaveVisible = Visibility.Visible;
            }
        }

        public double ZAxisEscapeDistance
        {
            get
            {
                return _zAxisEscapeDistance;
            }
            set
            {
                _zAxisEscapeDistance = value;
                OnPropertyChanged("ZAxisEscapeDistance");
                OnPropertyChanged("ZAxisEscapeDistanceUM");
                SaveVisible = Visibility.Visible;
            }
        }

        public double ZAxisEscapeDistanceUM
        {
            get
            {
                return ZAxisEscapeDistance * 1000;
            }
            set
            {
                ZAxisEscapeDistance = value / 1000;
                OnPropertyChanged("ZAxisEscapeDistanceUM");
            }
        }

        public int ZAxisToEscape
        {
            get
            {
                return _zAxisToEscape;
            }
            set
            {
                _zAxisToEscape = value;
                OnPropertyChanged("ZAxisToEscape");
                SaveVisible = Visibility.Visible;
            }
        }

        private int BeamExpType
        {
            get;
            set;
        }

        #endregion Properties

        #region Methods

        [DllImport(".\\ResourceManager.dll", EntryPoint = "GetHardwareSettingsFilePathAndName", CharSet = CharSet.Unicode)]
        public static extern int GetHardwareSettingsFilePathAndName(StringBuilder sb, int length);

        public bool GetAttribute(XmlNode node, XmlDocument doc, string attrName, ref string attrValue)
        {
            bool ret;

            if (null == node.Attributes.GetNamedItem(attrName))
            {
                ret = false;
            }
            else
            {
                attrValue = node.Attributes[attrName].Value;
                ret = true;
            }

            return ret;
        }

        public string GetHardwareSettingsFileString()
        {
            const int PATH_LENGTH = 261;
            StringBuilder sb = new StringBuilder(PATH_LENGTH);
            GetHardwareSettingsFilePathAndName(sb, PATH_LENGTH);

            return sb.ToString();
        }

        //assign the attribute value to the input node and document
        //if the attribute does not exist add it to the document
        public void SetAttribute(XmlNode node, XmlDocument doc, string attrName, string attValue)
        {
            XmlNode tempNode = node.Attributes[attrName];

            if (null == tempNode)
            {
                XmlAttribute attr = doc.CreateAttribute(attrName);

                attr.Value = attValue;

                node.Attributes.Append(attr);
            }
            else
            {
                node.Attributes[attrName].Value = attValue;
            }
        }

        protected virtual void OnPropertyChanged(String propertyName)
        {
            if (System.String.IsNullOrEmpty(propertyName))
            {
                return;
            }
            if (PropertyChanged != null)
            {
                PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }

        private int BeamExpansionPosConverter(int direction, int pos)
        {
            int retVal = 0;

            //convert from beamexp position to combo box index
            if (0 == direction)
            {
                switch (BeamExpType)
                {
                    case 0:
                    case 1:
                        {
                            retVal = pos;
                        }
                        break;
                    case 2:
                        {
                            retVal = Convert.ToInt32((pos / 10.0) - 10.0);
                            retVal = (int)((pos - _minExp * 100) / (TYPE2INCREMENT * 100));
                        }
                        break;
                }
            }
            else
            {
                //convert from combo box index to beamexp pos

                switch (BeamExpType)
                {
                    case 0:
                    case 1:
                        {
                            retVal = pos;
                        }
                        break;
                    case 2:
                        {
                            retVal = (int)(pos * TYPE2INCREMENT * 100 + _minExp * 100);
                        }
                        break;
                }
            }
            return retVal;
        }

        private void btnAdd_Click(object sender, RoutedEventArgs e)
        {
            if (null != _hardwareDoc)
            {
                XmlNodeList ndList = _hardwareDoc.SelectNodes("/HardwareSettings/Objectives/Objective");

                SaveObjectiveSettings(ndList.Count);
            }
        }

        private void btnClose_Click(object sender, RoutedEventArgs e)
        {
            this.Close();
        }

        private void btnDelete_Click(object sender, RoutedEventArgs e)
        {
            XmlNodeList ndListObj = _hardwareDoc.SelectNodes("/HardwareSettings/Objectives");
            XmlNodeList ndList = _hardwareDoc.SelectNodes("/HardwareSettings/Objectives/Objective");

            if ((ndListObj.Count > 0) && (ndList.Count > 0))
            {
                ndListObj[0].RemoveChild(ndList[ObjectiveSelected]);
            }
            _hardwareDoc.Save(GetHardwareSettingsFileString());

            int newObjectiveSelection = Math.Max(0, Math.Min(ObjectiveSelected, ndList.Count - 2));
            BuildObjectiveCollection();

            ObjectiveSelected = newObjectiveSelection;
        }

        private void btnSave_Click(object sender, RoutedEventArgs e)
        {
            SaveObjectiveSettings(ObjectiveSelected);
        }

        private void BuildObjectiveCollection()
        {
            if (null != _hardwareDoc)
            {
                Objectives.Clear();

                XmlNodeList ndList = _hardwareDoc.SelectNodes("/HardwareSettings/Objectives/Objective");
                string str = string.Empty;

                if ((ndList.Count > 0))
                {
                    for (int i = 0; i < ndList.Count; i++)
                    {
                        if (GetAttribute(ndList[i], _hardwareDoc, "name", ref str))
                        {
                            Objectives.Add(str);
                        }

                    }
                }
            }
        }

        private void LoadObjSettingsFromIndex(int index)
        {
            if (index < 0)
            {
                return;
            }

            if (null != _hardwareDoc)
            {
                XmlNodeList ndList = _hardwareDoc.SelectNodes("/HardwareSettings/Objectives/Objective");
                string str = string.Empty;

                if ((ndList.Count > 0) && (ndList.Count > index))
                {
                    if (GetAttribute(ndList[index], _hardwareDoc, "name", ref str))
                    {
                        ObjectiveName = str;
                    }
                    if (GetAttribute(ndList[index], _hardwareDoc, "mag", ref str))
                    {
                        ObjectiveMagnification = Convert.ToDouble(str);
                    }
                    if (GetAttribute(ndList[index], _hardwareDoc, "na", ref str))
                    {
                        ObjectiveNA = Convert.ToDouble(str);
                    }
                    if (GetAttribute(ndList[index], _hardwareDoc, "beamExp", ref str))
                    {
                        BeamExp = BeamExpansionPosConverter(0, Convert.ToInt32(str));
                    }
                    if (GetAttribute(ndList[index], _hardwareDoc, "beamWavelength", ref str))
                    {
                        BeamWavelength = Convert.ToInt32(str);
                    }
                    if (GetAttribute(ndList[index], _hardwareDoc, "beamExp2", ref str))
                    {
                        BeamExp2 = BeamExpansionPosConverter(0, Convert.ToInt32(str));
                    }
                    if (GetAttribute(ndList[index], _hardwareDoc, "beamWavelength2", ref str))
                    {
                        BeamWavelength2 = Convert.ToInt32(str);
                    }

                    if (GetAttribute(ndList[index], _hardwareDoc, "turretPosition", ref str))
                    {
                        //one based turret position
                        TurretPosition = Convert.ToInt32(str) - 1;
                    }

                    if (GetAttribute(ndList[index], _hardwareDoc, "zAxisToEscape", ref str))
                    {
                        ZAxisToEscape = Convert.ToInt32(str);
                    }

                    if (GetAttribute(ndList[index], _hardwareDoc, "zAxisEscapeDistance", ref str))
                    {
                        ZAxisEscapeDistance = Convert.ToDouble(str);
                    }
                }
            }

            SaveVisible = Visibility.Collapsed;
        }

        private void SaveObjectiveSettings(int index)
        {
            if (null != _hardwareDoc)
            {
                XmlNodeList ndList = _hardwareDoc.SelectNodes("/HardwareSettings/Objectives/Objective");
                string str = string.Empty;

                if ((ndList.Count > 0) && (ndList.Count > index))
                {

                    SetAttribute(ndList[index], _hardwareDoc, "name", ObjectiveName.ToString());
                    SetAttribute(ndList[index], _hardwareDoc, "na", ObjectiveNA.ToString());
                    SetAttribute(ndList[index], _hardwareDoc, "mag", ObjectiveMagnification.ToString());
                    SetAttribute(ndList[index], _hardwareDoc, "beamExp", BeamExpansionPosConverter(1, BeamExp).ToString());
                    SetAttribute(ndList[index], _hardwareDoc, "beamWavelength", BeamWavelength.ToString());
                    SetAttribute(ndList[index], _hardwareDoc, "beamExp2", BeamExpansionPosConverter(1, BeamExp2).ToString());
                    SetAttribute(ndList[index], _hardwareDoc, "beamWavelength2", BeamWavelength2.ToString());
                    SetAttribute(ndList[index], _hardwareDoc, "afScanStartMM", "0");
                    SetAttribute(ndList[index], _hardwareDoc, "afFocusOffsetMM", "0");
                    SetAttribute(ndList[index], _hardwareDoc, "afAdaptiveOffsetMM", "0");
                    //one based turret position
                    SetAttribute(ndList[index], _hardwareDoc, "turretPosition", (TurretPosition + 1).ToString());
                    SetAttribute(ndList[index], _hardwareDoc, "zAxisToEscape", ZAxisToEscape.ToString());
                    SetAttribute(ndList[index], _hardwareDoc, "zAxisEscapeDistance", ZAxisEscapeDistance.ToString());
                    _hardwareDoc.Save(GetHardwareSettingsFileString());
                }
                else if (ndList.Count == index)
                {

                    //add a new objective
                    XmlElement newElement = _hardwareDoc.CreateElement("Objective");

                    ndList = _hardwareDoc.SelectNodes("/HardwareSettings/Objectives");

                    ndList[0].AppendChild(newElement);

                    ndList = _hardwareDoc.SelectNodes("/HardwareSettings/Objectives/Objective");

                    SetAttribute(ndList[index], _hardwareDoc, "name", "New Obj");
                    SetAttribute(ndList[index], _hardwareDoc, "na", "1.0");
                    SetAttribute(ndList[index], _hardwareDoc, "mag", "20");
                    SetAttribute(ndList[index], _hardwareDoc, "beamExp", BeamExpansionPosConverter(1, 0).ToString());
                    SetAttribute(ndList[index], _hardwareDoc, "beamWavelength", "800");
                    SetAttribute(ndList[index], _hardwareDoc, "beamExp2", BeamExpansionPosConverter(1, 0).ToString());
                    SetAttribute(ndList[index], _hardwareDoc, "beamWavelength2", "800");
                    SetAttribute(ndList[index], _hardwareDoc, "afScanStartMM", "0");
                    SetAttribute(ndList[index], _hardwareDoc, "afFocusOffsetMM", "0");
                    SetAttribute(ndList[index], _hardwareDoc, "afAdaptiveOffsetMM", "0");
                    SetAttribute(ndList[index], _hardwareDoc, "turretPosition", "1");
                    SetAttribute(ndList[index], _hardwareDoc, "zAxisToEscape", "0");
                    SetAttribute(ndList[index], _hardwareDoc, "zAxisEscapeDistance", "0");

                    _hardwareDoc.Save(GetHardwareSettingsFileString());

                }
            }

            BuildObjectiveCollection();
            ObjectiveSelected = index;
            SaveVisible = Visibility.Collapsed;
        }

        void SetupObjectives_Loaded(object sender, RoutedEventArgs e)
        {
            XmlNodeList ndList = _hardwareDoc.SelectNodes("/HardwareSettings/Objectives");
            string str = string.Empty;

            if (GetAttribute(ndList[0], _hardwareDoc, "beamExpType", ref str))
            {
                BeamExpType = Convert.ToInt32(str);

                cbBeamExp.Items.Clear();
                cbBeamExp2.Items.Clear();

                switch (BeamExpType)
                {
                    case 0:
                        {
                            cbBeamExp.Items.Add("1.0");
                            cbBeamExp.Items.Add("1.5");
                            cbBeamExp.Items.Add("2.0");
                            cbBeamExp2.Items.Add("1.0");
                            cbBeamExp2.Items.Add("1.5");
                            cbBeamExp2.Items.Add("2.0");
                        }
                        break;
                    case 1: //ThorBeamExpan
                        {
                            cbBeamExp.Items.Add("1.0");
                            cbBeamExp.Items.Add("1.5");
                            cbBeamExp.Items.Add("2.0");
                            cbBeamExp.Items.Add("2.5");
                            cbBeamExp.Items.Add("2.9");
                            cbBeamExp.Items.Add("3.2");
                            cbBeamExp.Items.Add("3.5");
                            cbBeamExp2.Items.Add("1.0");
                            cbBeamExp2.Items.Add("1.5");
                            cbBeamExp2.Items.Add("2.0");
                            cbBeamExp2.Items.Add("2.5");
                            cbBeamExp2.Items.Add("2.9");
                            cbBeamExp2.Items.Add("3.2");
                            cbBeamExp2.Items.Add("3.5");
                        }
                        break;
                    case 2: //ThorVBE
                        {
                            for (double i = Math.Round(_minExp, 1); Math.Round(i, 1) <= Math.Round(_maxExp, 1); i += TYPE2INCREMENT)
                            {
                                Decimal dec = new Decimal(i);
                                cbBeamExp.Items.Add(Math.Round(dec, 1).ToString());
                                cbBeamExp2.Items.Add(Math.Round(dec, 1).ToString());
                            }
                        }
                        break;
                }
            }

            BuildObjectiveCollection();

            ObjectiveSelected = 0;
        }

        #endregion Methods
    }
}