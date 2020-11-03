namespace HardwareSetupUserControl
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.ComponentModel;
    using System.IO;
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
    /// Interaction logic for AddModality.xaml
    /// </summary>
    public partial class DisplayedDevices : Window, INotifyPropertyChanged
    {
        #region Fields

        private bool _beamExpander;
        private bool _beamStabilizer;
        private bool _bleachingScanner;
        private bool _bleachingSLM;
        private bool _controlUnit;
        private bool _epiTurret;
        private bool _imageDetector;
        private bool _lamp;
        private bool _laserSource;
        private bool _lightPath;
        private bool _objectiveTurret;
        private bool _pinholeWheel;
        private bool _pMT1;
        private bool _pMT2;
        private bool _pMT3;
        private bool _pMT4;
        private bool _powerRegulator;
        private bool _powerRegulator2;
        private bool _secondaryZStage;
        private bool _shutter;
        private bool _spectrumFilter;
        private bool _xYStage;
        private bool _zStage;

        #endregion Fields

        #region Constructors

        public DisplayedDevices()
        {
            InitializeComponent();
            DataContext = this;
        }

        #endregion Constructors

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public bool BeamExpander
        {
            get
            {
                return _beamExpander;
            }
            set
            {
                _beamExpander = value;
                OnPropertyChanged("BeamExpander");
            }
        }

        public bool BeamStabilizer
        {
            get
            {
                return _beamStabilizer;
            }
            set
            {
                _beamStabilizer = value;
                OnPropertyChanged("BeamStabilizer");
            }
        }

        public bool BleachingScanner
        {
            get
            {
                return _bleachingScanner;
            }
            set
            {
                _bleachingScanner = value;
                OnPropertyChanged("BleachingScanner");
            }
        }

        public bool BleachingSLM
        {
            get
            {
                return _bleachingSLM;
            }
            set
            {
                _bleachingSLM = value;
                OnPropertyChanged("BleachingSLM");
            }
        }

        public bool ControlUnit
        {
            get
            {
                return _controlUnit;
            }
            set
            {
                _controlUnit = value;
                OnPropertyChanged("ControlUnit");
            }
        }

        public bool EpiTurret
        {
            get
            {
                return _epiTurret;
            }
            set
            {
                _epiTurret = value;
                OnPropertyChanged("EpiTurret");
            }
        }

        public bool ImageDetector
        {
            get
            {
                return _imageDetector;
            }
            set
            {
                _imageDetector = value;
                OnPropertyChanged("ImageDetector");
            }
        }

        public bool Lamp
        {
            get
            {
                return _lamp;
            }
            set
            {
                _lamp = value;
                OnPropertyChanged("Lamp");
            }
        }

        public bool LaserSource
        {
            get
            {
                return _laserSource;
            }
            set
            {
                _laserSource = value;
                OnPropertyChanged("LaserSource");
            }
        }

        public bool LightPath
        {
            get
            {
                return _lightPath;
            }
            set
            {
                _lightPath = value;
                OnPropertyChanged("LightPath");
            }
        }

        public bool ObjectiveTurret
        {
            get
            {
                return _objectiveTurret;
            }
            set
            {
                _objectiveTurret = value;
                OnPropertyChanged("ObjectiveTurret");
            }
        }

        public bool PinholeWheel
        {
            get
            {
                return _pinholeWheel;
            }
            set
            {
                _pinholeWheel = value;
                OnPropertyChanged("PinholeWheel");
            }
        }

        public bool PMT1
        {
            get
            {
                return _pMT1;
            }
            set
            {
                _pMT1 = value;
                OnPropertyChanged("PMT1");
            }
        }

        public bool PMT2
        {
            get
            {
                return _pMT2;
            }
            set
            {
                _pMT2 = value;
                OnPropertyChanged("PMT2");
            }
        }

        public bool PMT3
        {
            get
            {
                return _pMT3;
            }
            set
            {
                _pMT3 = value;
                OnPropertyChanged("PMT3");
            }
        }

        public bool PMT4
        {
            get
            {
                return _pMT4;
            }
            set
            {
                _pMT4 = value;
                OnPropertyChanged("PMT4");
            }
        }

        public bool PowerRegulator
        {
            get
            {
                return _powerRegulator;
            }
            set
            {
                _powerRegulator = value;
                OnPropertyChanged("PowerRegulator");
            }
        }

        public bool PowerRegulator2
        {
            get
            {
                return _powerRegulator2;
            }
            set
            {
                _powerRegulator2 = value;
                OnPropertyChanged("PowerRegulator2");
            }
        }

        public bool SecondaryZStage
        {
            get
            {
                return _secondaryZStage;
            }
            set
            {
                _secondaryZStage = value;
                OnPropertyChanged("SecondaryZStage");
            }
        }

        public bool Shutter
        {
            get
            {
                return _shutter;
            }
            set
            {
                _shutter = value;
                OnPropertyChanged("Shutter");
            }
        }

        public bool SpectrumFilter
        {
            get
            {
                return _spectrumFilter;
            }
            set
            {
                _spectrumFilter = value;
                OnPropertyChanged("SpectrumFilter");
            }
        }

        public bool XYStage
        {
            get
            {
                return _xYStage;
            }
            set
            {
                _xYStage = value;
                OnPropertyChanged("XYStage");
            }
        }

        public bool ZStage
        {
            get
            {
                return _zStage;
            }
            set
            {
                _zStage = value;
                OnPropertyChanged("ZStage");
            }
        }

        #endregion Properties

        #region Methods

        [DllImport(".\\ResourceManager.dll", EntryPoint = "GetMyDocumentsThorImageFolder", CharSet = CharSet.Unicode)]
        public static extern int GetMyDocumentsThorImageFolder(StringBuilder sb, int length);

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

        public string GetMyDocumentsThorImageFolderString()
        {
            const int PATH_LENGTH = 261;
            StringBuilder sb = new StringBuilder(PATH_LENGTH);
            GetMyDocumentsThorImageFolder(sb, PATH_LENGTH);

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

        private void btnCancel_Click(object sender, RoutedEventArgs e)
        {
            this.DialogResult = false;
            this.Close();
        }

        private void btnOK_Click(object sender, RoutedEventArgs e)
        {
            this.DialogResult = true;
            this.Close();
        }

        void Copy(string sourceDir, string targetDir)
        {
            Directory.CreateDirectory(targetDir);

            foreach(var file in Directory.GetFiles(sourceDir))
                File.Copy(file, System.IO.Path.Combine(targetDir, System.IO.Path.GetFileName(file)));

            foreach(var directory in Directory.GetDirectories(sourceDir))
                Copy(directory, System.IO.Path.Combine(targetDir, System.IO.Path.GetFileName(directory)));
        }

        #endregion Methods
    }
}