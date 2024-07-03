/// <summary>
/// The ThorDetector_Control namespace.
/// </summary>
namespace ThorDetector_Control
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.ComponentModel;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Threading;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Navigation;
    using System.Windows.Shapes;
    using System.Windows.Threading;
    using System.Xml;

    /// <summary>
    /// Interaction logic for Window1.xaml
    /// </summary>
    public partial class Window1 : Window, INotifyPropertyChanged
    {
        #region Fields

        /// <summary>
        /// The false
        /// </summary>
        private const int FALSE = 0;

        /// <summary>
        /// The number of detectors
        /// </summary>
        const int NUM_DETECTORS = 6;

        /// <summary>
        /// The true
        /// </summary>
        private const int TRUE = 1;

        /// <summary>
        /// The timer
        /// </summary>
        private DispatcherTimer timer;

        /// <summary>
        /// List of detector bandwidths
        /// </summary>
        private ObservableCollection<ObservableCollection<string>> _bandwidthList = new ObservableCollection<ObservableCollection<string>>();

        ///<summary>
        /// List of bandwidths
        /// </summary>
        private string[] _bandwidthTags = { "250 kHz", "1 MHz", "2.5 MHz", "15 MHz", "30 MHz", "80 MHz", "200MHz", "300 MHz" };

        /// <summary>
        /// Bandwidth to string map
        /// </summary>
        private Dictionary<int, string> _bandwidthToStringMap = new Dictionary<int, string>();

        /// <summary>
        /// The _COMPort list
        /// </summary>
        int[] _COMPortList = new int[6];

        /// <summary>
        /// The _detector1 connected
        /// </summary>
        bool _detector1Connected = false;

        /// <summary>
        /// The _detector1gain
        /// </summary>
        private int _detector1gain;

        ///<summary>
        ///The _detector1gain offset step size
        ///</summary>
        double _detector1OffsetStepSize = 0;

        /// <summary>
        /// The _detector1 on
        /// </summary>
        private int _detector1On = 0;

        /// <summary>
        /// The _detector1gain offset
        /// </summary>
        double _detector1OutputOffset = 0;

        /// <summary>
        /// The _detector1 trip flag
        /// </summary>
        int _detector1Tripped = 1;

        /// <summary>
        /// The _detector2 connected
        /// </summary>
        bool _detector2Connected = false;

        /// <summary>
        /// The _detector2gain
        /// </summary>
        private int _detector2gain;

        ///<summary>
        ///The _detector2gain offset step size
        ///</summary>
        double _detector2OffsetStepSize = 0;

        /// <summary>
        /// The _detector2 on
        /// </summary>
        private int _detector2On = 0;

        /// <summary>
        /// The _detector2gain offset
        /// </summary>
        double _detector2OutputOffset = 0;

        /// <summary>
        /// The _detector2 trip flag
        /// </summary>
        int _detector2Tripped = 1;

        /// <summary>
        /// The _detector3 connected
        /// </summary>
        bool _detector3Connected = false;

        /// <summary>
        /// The _detector3gain
        /// </summary>
        private int _detector3gain;

        ///<summary>
        ///The _detector3gain offset step size
        ///</summary>
        double _detector3OffsetStepSize = 0;

        /// <summary>
        /// The _detector3 on
        /// </summary>
        private int _detector3On = 0;

        /// <summary>
        /// The _detector3gain offset
        /// </summary>
        double _detector3OutputOffset = 0;

        /// <summary>
        /// The _detector3 trip flag
        /// </summary>
        int _detector3Tripped = 1;

        /// <summary>
        /// The _detector4 connected
        /// </summary>
        bool _detector4Connected = false;

        /// <summary>
        /// The _detector4gain
        /// </summary>
        private int _detector4gain;

        ///<summary>
        ///The _detector4gain offset step size
        ///</summary>
        double _detector4OffsetStepSize = 0;

        /// <summary>
        /// The _detector4 on
        /// </summary>
        private int _detector4On = 0;

        /// <summary>
        /// The _detector4gain offset
        /// </summary>
        double _detector4OutputOffset = 0;

        /// <summary>
        /// The _detector4 trip flag
        /// </summary>
        int _detector4Tripped = 1;

        /// <summary>
        /// The _detector5 connected
        /// </summary>
        bool _detector5Connected = false;

        /// <summary>
        /// The _detector5gain
        /// </summary>
        private int _detector5gain;

        ///<summary>
        ///The _detector5gain offset step size
        ///</summary>
        double _detector5OffsetStepSize = 0;

        /// <summary>
        /// The _detector5 on
        /// </summary>
        private int _detector5On = 0;

        /// <summary>
        /// The _detector5gain offset
        /// </summary>
        double _detector5OutputOffset = 0;

        /// <summary>
        /// The _detector5 trip flag
        /// </summary>
        int _detector5Tripped = 1;

        /// <summary>
        /// The _detector6 connected
        /// </summary>
        bool _detector6Connected = false;

        /// <summary>
        /// The _detector6gain
        /// </summary>
        private int _detector6gain;

        ///<summary>
        ///The _detector6gain offset step size
        ///</summary>
        double _detector6OffsetStepSize = 0;

        /// <summary>
        /// The _detector6 on
        /// </summary>
        private int _detector6On = 0;

        /// <summary>
        /// The _detector6gain offset
        /// </summary>
        double _detector6OutputOffset = 0;

        /// <summary>
        /// The _detector6 trip flag
        /// </summary>
        int _detector6Tripped = 1;

        /// <summary>
        /// The _detectorS active
        /// </summary>
        private bool _detectorsActive = false;

        /// <summary>
        /// List of detector types
        /// </summary>
        List<string> _detectorTypes = new List<string> { "PMT Old Bootloader", "PMT1000", "PMT2100", "PMT2106", "APD", "Photodiode", "HPD1000", "SIPM100", "PMT2110", "PMT3100" };

        /// <summary>
        /// The _devices
        /// </summary>
        int _devices;

        /// <summary>
        /// The _initialized
        /// </summary>
        bool _initialized = false;

        /// <summary>
        /// The _serialNumber list
        /// </summary>
        string[] _serialNumberList = new string[6];

        /// <summary>
        /// String to bandwidth map
        /// </summary>
        private Dictionary<string, int> _stringToBandwidthMap = new Dictionary<string, int>();

        #endregion Fields

        #region Constructors

        /// <summary>
        /// Initializes a new instance of the <see cref="Window1"/> class.
        /// </summary>
        public Window1()
        {
            InitializeComponent();
            timer = new DispatcherTimer();
            this.KeyDown += Window1_KeyDown;
            this.Closing += new System.ComponentModel.CancelEventHandler(Window1_Closing);
            Application.Current.Exit += new ExitEventHandler(Current_Exit);
        }

        #endregion Constructors

        #region Events

        /// <summary>
        /// Occurs when a property value changes.
        /// </summary>
        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        /// <summary>
        /// Gets or sets the list of detector bandwidths
        /// </summary>
        public ObservableCollection<ObservableCollection<string>> BandwidthList
        {
            get
            {
                return _bandwidthList;
            }
            set
            {
                _bandwidthList = value;
                OnPropertyChanged("BandwidthList");
            }
        }

        /// <summary>
        /// Gets or sets the detector1 bandwidth.
        /// </summary>
        /// <value>The detector1 bandwidth.</value>
        public string Detector1Bandwidth
        {
            get
            {
                double val = 0;
                GetParameter(Convert.ToInt32(DeviceParams.PARAM_PMT1_BANDWIDTH_POS_CURRENT), ref val);
                return ConvertBandwidthToString(Convert.ToInt32(val));
            }
            set
            {
                ExecutePosition(Convert.ToInt32(DeviceParams.PARAM_PMT1_BANDWIDTH_POS), ConvertStringToBandwidth(value));
                OnPropertyChanged("Detector1Bandwidth");
            }
        }

        /// <summary>
        /// Gets the detector1 detector type.
        /// </summary>
        /// <value>The detector1 detector type.</value>
        public string Detector1DetectorType
        {
            get
            {
                double detectorType = 0;
                GetParameter(Convert.ToInt32(DeviceParams.PARAM_PMT1_DETECTOR_TYPE), ref detectorType);
                switch (detectorType)
                {
                    case 0:
                        return "[" + _detectorTypes[0] + "]";     //PMT Old Bootloader
                    case 1:
                        return "[" + _detectorTypes[1] + "]";     //PMT1000
                    case 2:
                        return "[" + _detectorTypes[2] + "]";     //PMT2100
                    case 3:
                        return "[" + _detectorTypes[3] + "]";     //PMT2106
                    case 4:
                        return "[" + _detectorTypes[4] + "]";     //APD
                    case 5:
                        return "[" + _detectorTypes[5] + "]";     //Photodiode
                    case 6:
                        return "[" + _detectorTypes[6] + "]";     //HPD1000
                    case 7:
                        return "[" + _detectorTypes[7] + "]";     //SIPM100
                    case 8:
                        return "[" + _detectorTypes[8] + "]";     //PMT2110
                    case 9:
                        return "[" + _detectorTypes[9] + "]";     //PMT3100
                    default:
                        return "[NA]";                            //Detector type not available
                }
            }
        }

        /// <summary>
        /// Gets or sets the detector1 gain.
        /// </summary>
        /// <value>The detector1 gain.</value>
        public int Detector1Gain
        {
            get
            {
                return _detector1gain;
            }
            set
            {
                if (value > Detector1GainMax)
                    _detector1gain = Detector1GainMax;
                else if (value < Detector1GainMin)
                    _detector1gain = Detector1GainMin;
                else
                    _detector1gain = value;

                if (0 == value)
                {
                    if (_detectorsActive)
                    {
                        ExecutePosition(Convert.ToInt32(DeviceParams.PARAM_PMT1_ENABLE), 0);
                    }
                }
                else if (0 < _detector1gain)
                {
                    if (_detectorsActive)
                    {
                        ExecutePosition(Convert.ToInt32(DeviceParams.PARAM_PMT1_ENABLE), _detector1On);
                    }
                }

                ExecutePosition(Convert.ToInt32(DeviceParams.PARAM_PMT1_GAIN_POS), _detector1gain);
                OnPropertyChanged("Detector1Gain");
                OnPropertyChanged("Detector1GainVolts");
            }
        }

        /// <summary>
        /// Gets or sets the detector1 gain maximum.
        /// </summary>
        /// <value>The detector1 gain maximum.</value>
        public int Detector1GainMax
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the detector1 gain minimum.
        /// </summary>
        /// <value>The detector1 gain minimum.</value>
        public int Detector1GainMin
        {
            get;
            set;
        }

        /// <summary>
        /// Gets the detector1 gain volts.
        /// </summary>
        /// <value>The detector1 gain volts.</value>
        public double Detector1GainVolts
        {
            get
            {
                double gain = 0;
                GetParameter(Convert.ToInt32(DeviceParams.PARAM_PMT1_GAIN_POS_CURRENT_VOLTS), ref gain);
                return Math.Round(gain, 3);
            }
        }

        /// <summary>
        /// Gets the detector1 offset step size.
        /// </summary>
        /// <value>The detector1 gain offset step size.</value>
        public double Detector1OffsetStepSize
        {
            get
            {
                double offset = 0;
                if (TRUE == GetParameter(Convert.ToInt32(DeviceParams.PARAM_PMT1_OFFSET_STEP_SIZE), ref offset))
                {
                    _detector1OffsetStepSize = offset;
                }
                return _detector1OffsetStepSize;
            }
        }

        /// <summary>
        /// Gets or sets the detector1 on.
        /// </summary>
        /// <value>The detector1 on.</value>
        public int Detector1On
        {
            get
            {
                return _detector1On;
            }
            set
            {
                _detector1On = value;
                if ((_detectorsActive || FALSE == _detector1On) && 0 < _detector1gain)
                {
                    ExecutePosition(Convert.ToInt32(DeviceParams.PARAM_PMT1_ENABLE), _detector1On);
                }
                OnPropertyChanged("Detector1On");
            }
        }

        /// <summary>
        /// Gets or sets the detector1 gain offset.
        /// </summary>
        /// <value>The detector1 gain offset.</value>
        public double Detector1OutputOffset
        {
            get
            {
                double OutputOffset = 0;
                if (TRUE == GetParameter(Convert.ToInt32(DeviceParams.PARAM_PMT1_OUTPUT_OFFSET_CURRENT), ref OutputOffset))
                {
                    _detector1OutputOffset = OutputOffset;
                }
                return Math.Round(_detector1OutputOffset, 3);
            }
            set
            {
                if (value > Detector1OutputOffsetMax)
                    _detector1OutputOffset = Detector1OutputOffsetMax;
                else if (value < Detector1OutputOffsetMin)
                    _detector1OutputOffset = Detector1OutputOffsetMin;
                else
                    _detector1OutputOffset = Math.Round(value, 3);
                ExecutePosition(Convert.ToInt32(DeviceParams.PARAM_PMT1_OUTPUT_OFFSET), _detector1OutputOffset);
                OnPropertyChanged("Detector1OutputOffset");
            }
        }

        /// <summary>
        /// Gets or sets the detector1 gain offset maximum.
        /// </summary>
        /// <value>The detector1 gain offset maximum.</value>
        public double Detector1OutputOffsetMax
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the detector1 gain offset minimum.
        /// </summary>
        /// <value>The detector1 gain offset minimum.</value>
        public double Detector1OutputOffsetMin
        {
            get;
            set;
        }

        /// <summary>
        /// Gets the detector1 serial number.
        /// </summary>
        /// <value>The detector1 serial number.</value>
        public string Detector1SerialNumber
        {
            get
            {
                string sn = string.Empty;
                if (TRUE == GetParameterString(Convert.ToInt32(DeviceParams.PARAM_PMT1_SERIALNUMBER), ref sn))
                {
                    return sn;
                }
                else
                {
                    return "NA";
                }
            }
        }

        /// <summary>
        /// Gets the detector1 tripped.
        /// </summary>
        /// <value>The detector1 tripped.</value>        
        public int Detector1Tripped
        {
            get
            {
                return _detector1Tripped;
            }
        }

        /// <summary>
        /// Gets or sets the detector2 bandwidth.
        /// </summary>
        /// <value>The detector2 bandwidth.</value>
        public string Detector2Bandwidth
        {
            get
            {
                double val = 0;
                GetParameter(Convert.ToInt32(DeviceParams.PARAM_PMT2_BANDWIDTH_POS_CURRENT), ref val);
                return ConvertBandwidthToString(Convert.ToInt32(val));
            }
            set
            {
                ExecutePosition(Convert.ToInt32(DeviceParams.PARAM_PMT2_BANDWIDTH_POS), ConvertStringToBandwidth(value));
                OnPropertyChanged("Detector2Bandwidth");
            }
        }

        /// <summary>
        /// Gets the detector2 detector type.
        /// </summary>
        /// <value>The detector2 detector type.</value>
        public string Detector2DetectorType
        {
            get
            {
                double detectorType = 0;
                GetParameter(Convert.ToInt32(DeviceParams.PARAM_PMT2_DETECTOR_TYPE), ref detectorType);
                switch (detectorType)
                {
                    case 0:
                        return "[" + _detectorTypes[0] + "]";     //PMT Old Bootloader
                    case 1:
                        return "[" + _detectorTypes[1] + "]";     //PMT1000
                    case 2:
                        return "[" + _detectorTypes[2] + "]";     //PMT2100
                    case 3:
                        return "[" + _detectorTypes[3] + "]";     //PMT2106
                    case 4:
                        return "[" + _detectorTypes[4] + "]";     //APD
                    case 5:
                        return "[" + _detectorTypes[5] + "]";     //Photodiode
                    case 6:
                        return "[" + _detectorTypes[6] + "]";     //HPD1000
                    case 7:
                        return "[" + _detectorTypes[7] + "]";     //SIPM100
                    case 8:
                        return "[" + _detectorTypes[8] + "]";     //PMT2110
                    case 9:
                        return "[" + _detectorTypes[9] + "]";     //PMT3100
                    default:
                        return "[NA]";                            //Detector type not available
                }
            }
        }

        /// <summary>
        /// Gets or sets the detector2 gain.
        /// </summary>
        /// <value>The detector2 gain.</value>
        public int Detector2Gain
        {
            get
            {
                return _detector2gain;
            }
            set
            {
                if (value > Detector2GainMax)
                    _detector2gain = Detector2GainMax;
                else if (value < Detector2GainMin)
                    _detector2gain = Detector2GainMin;
                else
                    _detector2gain = value;

                if (0 == value)
                {
                    if (_detectorsActive)
                    {
                        ExecutePosition(Convert.ToInt32(DeviceParams.PARAM_PMT2_ENABLE), 0);
                    }
                }
                else if (0 < _detector2gain)
                {
                    if (_detectorsActive)
                    {
                        ExecutePosition(Convert.ToInt32(DeviceParams.PARAM_PMT2_ENABLE), _detector2On);
                    }
                }

                ExecutePosition(Convert.ToInt32(DeviceParams.PARAM_PMT2_GAIN_POS), _detector2gain);
                OnPropertyChanged("Detector2Gain");
                OnPropertyChanged("Detector2GainVolts");
            }
        }

        /// <summary>
        /// Gets or sets the detector2 gain maximum.
        /// </summary>
        /// <value>The detector2 gain maximum.</value>
        public int Detector2GainMax
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the detector2 gain minimum.
        /// </summary>
        /// <value>The detector2 gain minimum.</value>
        public int Detector2GainMin
        {
            get;
            set;
        }

        /// <summary>
        /// Gets the detector2 gain volts.
        /// </summary>
        /// <value>The detector2 gain volts.</value>
        public double Detector2GainVolts
        {
            get
            {
                double gain = 0;
                GetParameter(Convert.ToInt32(DeviceParams.PARAM_PMT2_GAIN_POS_CURRENT_VOLTS), ref gain);
                return Math.Round(gain, 3);
            }
        }

        /// <summary>
        /// Gets the detector2 offset step size.
        /// </summary>
        /// <value>The detector2 gain offset step size.</value>
        public double Detector2OffsetStepSize
        {
            get
            {
                double offset = 0;
                if (TRUE == GetParameter(Convert.ToInt32(DeviceParams.PARAM_PMT2_OFFSET_STEP_SIZE), ref offset))
                {
                    _detector2OffsetStepSize = offset;
                }
                return _detector2OffsetStepSize;
            }
        }

        /// <summary>
        /// Gets or sets the detector2 on.
        /// </summary>
        /// <value>The detector2 on.</value>
        public int Detector2On
        {
            get
            {
                return _detector2On;
            }
            set
            {
                _detector2On = value;
                if ((_detectorsActive || FALSE == _detector2On) && 0 < _detector2gain)
                {
                    ExecutePosition(Convert.ToInt32(DeviceParams.PARAM_PMT2_ENABLE), _detector2On);
                }
                OnPropertyChanged("Detector2On");
            }
        }

        /// <summary>
        /// Gets or sets the detector2 gain offset.
        /// </summary>
        /// <value>The detector2 gain offset.</value>
        public double Detector2OutputOffset
        {
            get
            {
                double OutputOffset = 0;
                if (TRUE == GetParameter(Convert.ToInt32(DeviceParams.PARAM_PMT2_OUTPUT_OFFSET_CURRENT), ref OutputOffset))
                {
                    _detector2OutputOffset = OutputOffset;
                }
                return Math.Round(_detector2OutputOffset, 3);
            }
            set
            {
                if (value > Detector2OutputOffsetMax)
                    _detector2OutputOffset = Detector2OutputOffsetMax;
                else if (value < Detector2OutputOffsetMin)
                    _detector2OutputOffset = Detector2OutputOffsetMin;
                else
                    _detector2OutputOffset = Math.Round(value, 3);
                ExecutePosition(Convert.ToInt32(DeviceParams.PARAM_PMT2_OUTPUT_OFFSET), _detector2OutputOffset);
                OnPropertyChanged("Detector2OutputOffset");
            }
        }

        /// <summary>
        /// Gets or sets the detector2 gain offset maximum.
        /// </summary>
        /// <value>The detector2 gain offset maximum.</value>
        public double Detector2OutputOffsetMax
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the detector2 gain offset minimum.
        /// </summary>
        /// <value>The detector2 gain offset minimum.</value>
        public double Detector2OutputOffsetMin
        {
            get;
            set;
        }

        /// <summary>
        /// Gets the detector2 serial number.
        /// </summary>
        /// <value>The detector2 serial number.</value>
        public string Detector2SerialNumber
        {
            get
            {
                string sn = string.Empty;
                if (TRUE == GetParameterString(Convert.ToInt32(DeviceParams.PARAM_PMT2_SERIALNUMBER), ref sn))
                {
                    return sn;
                }
                else
                {
                    return "NA";
                }
            }
        }

        /// <summary>
        /// Gets the detector2 tripped.
        /// </summary>
        /// <value>The detector2 tripped.</value>        
        public int Detector2Tripped
        {
            get
            {
                return _detector2Tripped;
            }
        }

        /// <summary>
        /// Gets or sets the detector3 bandwidth.
        /// </summary>
        /// <value>The detector3 bandwidth.</value>
        public string Detector3Bandwidth
        {
            get
            {
                double val = 0;
                GetParameter(Convert.ToInt32(DeviceParams.PARAM_PMT3_BANDWIDTH_POS_CURRENT), ref val);
                return ConvertBandwidthToString(Convert.ToInt32(val));
            }
            set
            {
                ExecutePosition(Convert.ToInt32(DeviceParams.PARAM_PMT3_BANDWIDTH_POS), ConvertStringToBandwidth(value));
                OnPropertyChanged("Detector3Bandwidth");
            }
        }

        /// <summary>
        /// Gets the detector3 detector type.
        /// </summary>
        /// <value>The detector3 detector type.</value>
        public string Detector3DetectorType
        {
            get
            {
                double detectorType = 0;
                GetParameter(Convert.ToInt32(DeviceParams.PARAM_PMT3_DETECTOR_TYPE), ref detectorType);
                switch (detectorType)
                {
                    case 0:
                        return "[" + _detectorTypes[0] + "]";     //PMT Old Bootloader
                    case 1:
                        return "[" + _detectorTypes[1] + "]";     //PMT1000
                    case 2:
                        return "[" + _detectorTypes[2] + "]";     //PMT2100
                    case 3:
                        return "[" + _detectorTypes[3] + "]";     //PMT2106
                    case 4:
                        return "[" + _detectorTypes[4] + "]";     //APD
                    case 5:
                        return "[" + _detectorTypes[5] + "]";     //Photodiode
                    case 6:
                        return "[" + _detectorTypes[6] + "]";     //HPD1000
                    case 7:
                        return "[" + _detectorTypes[7] + "]";     //SIPM100
                    case 8:
                        return "[" + _detectorTypes[8] + "]";     //PMT2110
                    case 9:
                        return "[" + _detectorTypes[9] + "]";     //PMT3100
                    default:
                        return "[NA]";                            //Detector type not available
                }
            }
        }

        /// <summary>
        /// Gets or sets the detector3 gain.
        /// </summary>
        /// <value>The detector3 gain.</value>
        public int Detector3Gain
        {
            get
            {
                return _detector3gain;
            }
            set
            {
                if (value > Detector3GainMax)
                    _detector3gain = Detector3GainMax;
                else if (value < Detector3GainMin)
                    _detector3gain = Detector3GainMin;
                else
                    _detector3gain = value;

                if (0 == value)
                {
                    if (_detectorsActive)
                    {
                        ExecutePosition(Convert.ToInt32(DeviceParams.PARAM_PMT3_ENABLE), 0);
                    }
                }
                else if (0 < _detector3gain)
                {
                    if (_detectorsActive)
                    {
                        ExecutePosition(Convert.ToInt32(DeviceParams.PARAM_PMT3_ENABLE), _detector3On);
                    }
                }

                ExecutePosition(Convert.ToInt32(DeviceParams.PARAM_PMT3_GAIN_POS), _detector3gain);
                OnPropertyChanged("Detector3Gain");
                OnPropertyChanged("Detector3GainVolts");
            }
        }

        /// <summary>
        /// Gets or sets the detector3 gain maximum.
        /// </summary>
        /// <value>The detector3 gain maximum.</value>
        public int Detector3GainMax
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the detector3 gain minimum.
        /// </summary>
        /// <value>The detector3 gain minimum.</value>
        public int Detector3GainMin
        {
            get;
            set;
        }

        /// <summary>
        /// Gets the detector3 gain volts.
        /// </summary>
        /// <value>The detector3 gain volts.</value>
        public double Detector3GainVolts
        {
            get
            {
                double gain = 0;
                GetParameter(Convert.ToInt32(DeviceParams.PARAM_PMT3_GAIN_POS_CURRENT_VOLTS), ref gain);
                return Math.Round(gain, 3);
            }
        }

        /// <summary>
        /// Gets the detector3 offset step size.
        /// </summary>
        /// <value>The detector3 gain offset step size.</value>
        public double Detector3OffsetStepSize
        {
            get
            {
                double offset = 0;
                if (TRUE == GetParameter(Convert.ToInt32(DeviceParams.PARAM_PMT3_OFFSET_STEP_SIZE), ref offset))
                {
                    _detector3OffsetStepSize = offset;
                }
                return _detector3OffsetStepSize;
            }
        }

        /// <summary>
        /// Gets or sets the detector3 on.
        /// </summary>
        /// <value>The detector3 on.</value>
        public int Detector3On
        {
            get
            {
                return _detector3On;
            }
            set
            {
                _detector3On = value;
                if ((_detectorsActive || FALSE == _detector3On) && 0 < _detector3gain)
                {
                    ExecutePosition(Convert.ToInt32(DeviceParams.PARAM_PMT3_ENABLE), _detector3On);
                }
                OnPropertyChanged("Detector3On");
            }
        }

        /// <summary>
        /// Gets or sets the detector3 gain offset.
        /// </summary>
        /// <value>The detector3 gain offset.</value>
        public double Detector3OutputOffset
        {
            get
            {
                double OutputOffset = 0;
                if (TRUE == GetParameter(Convert.ToInt32(DeviceParams.PARAM_PMT3_OUTPUT_OFFSET_CURRENT), ref OutputOffset))
                {
                    _detector3OutputOffset = OutputOffset;
                }
                return Math.Round(_detector3OutputOffset, 3);
            }
            set
            {
                if (value > Detector3OutputOffsetMax)
                    _detector3OutputOffset = Detector3OutputOffsetMax;
                else if (value < Detector3OutputOffsetMin)
                    _detector3OutputOffset = Detector3OutputOffsetMin;
                else
                    _detector3OutputOffset = Math.Round(value, 3);
                ExecutePosition(Convert.ToInt32(DeviceParams.PARAM_PMT3_OUTPUT_OFFSET), _detector3OutputOffset);
                OnPropertyChanged("Detector3OutputOffset");
            }
        }

        /// <summary>
        /// Gets or sets the detector3 gain offset maximum.
        /// </summary>
        /// <value>The detector3 gain offset maximum.</value>
        public double Detector3OutputOffsetMax
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the detector3 gain offset minimum.
        /// </summary>
        /// <value>The detector3 gain offset minimum.</value>
        public double Detector3OutputOffsetMin
        {
            get;
            set;
        }

        /// <summary>
        /// Gets the detector3 serial number.
        /// </summary>
        /// <value>The detector3 serial number.</value>
        public string Detector3SerialNumber
        {
            get
            {
                string sn = string.Empty;
                if (TRUE == GetParameterString(Convert.ToInt32(DeviceParams.PARAM_PMT3_SERIALNUMBER), ref sn))
                {
                    return sn;
                }
                else
                {
                    return "NA";
                }
            }
        }

        /// <summary>
        /// Gets the detector3 tripped.
        /// </summary>
        /// <value>The detector3 tripped.</value>        
        public int Detector3Tripped
        {
            get
            {
                return _detector3Tripped;
            }
        }

        /// <summary>
        /// Gets or sets the detector4 bandwidth.
        /// </summary>
        /// <value>The detector4 bandwidth.</value>
        public string Detector4Bandwidth
        {
            get
            {
                double val = 0;
                GetParameter(Convert.ToInt32(DeviceParams.PARAM_PMT4_BANDWIDTH_POS_CURRENT), ref val);
                return ConvertBandwidthToString(Convert.ToInt32(val));
            }
            set
            {
                ExecutePosition(Convert.ToInt32(DeviceParams.PARAM_PMT4_BANDWIDTH_POS), ConvertStringToBandwidth(value));
                OnPropertyChanged("Detector4Bandwidth");
            }
        }

        /// <summary>
        /// Gets the detector4 detector type.
        /// </summary>
        /// <value>The detector4 detector type.</value>
        public string Detector4DetectorType
        {
            get
            {
                double detectorType = 0;
                GetParameter(Convert.ToInt32(DeviceParams.PARAM_PMT4_DETECTOR_TYPE), ref detectorType);
                switch (detectorType)
                {
                    case 0:
                        return "[" + _detectorTypes[0] + "]";     //PMT Old Bootloader
                    case 1:
                        return "[" + _detectorTypes[1] + "]";     //PMT1000
                    case 2:
                        return "[" + _detectorTypes[2] + "]";     //PMT2100
                    case 3:
                        return "[" + _detectorTypes[3] + "]";     //PMT2106
                    case 4:
                        return "[" + _detectorTypes[4] + "]";     //APD
                    case 5:
                        return "[" + _detectorTypes[5] + "]";     //Photodiode
                    case 6:
                        return "[" + _detectorTypes[6] + "]";     //HPD1000
                    case 7:
                        return "[" + _detectorTypes[7] + "]";     //SIPM100
                    case 8:
                        return "[" + _detectorTypes[8] + "]";     //PMT2110
                    case 9:
                        return "[" + _detectorTypes[9] + "]";     //PMT3100
                    default:
                        return "[NA]";                            //Detector type not available
                }
            }
        }

        /// <summary>
        /// Gets or sets the detector4 gain.
        /// </summary>
        /// <value>The detector4 gain.</value>
        public int Detector4Gain
        {
            get
            {
                return _detector4gain;
            }
            set
            {
                if (value > Detector4GainMax)
                    _detector4gain = Detector4GainMax;
                else if (value < Detector4GainMin)
                    _detector4gain = Detector4GainMin;
                else
                    _detector4gain = value;

                if (0 == value)
                {
                    if (_detectorsActive)
                    {
                        ExecutePosition(Convert.ToInt32(DeviceParams.PARAM_PMT4_ENABLE), 0);
                    }
                }
                else if (0 < _detector4gain)
                {
                    if (_detectorsActive)
                    {
                        ExecutePosition(Convert.ToInt32(DeviceParams.PARAM_PMT4_ENABLE), _detector4On);
                    }
                }

                ExecutePosition(Convert.ToInt32(DeviceParams.PARAM_PMT4_GAIN_POS), _detector4gain);
                OnPropertyChanged("Detector4Gain");
                OnPropertyChanged("Detector4GainVolts");
            }
        }

        /// <summary>
        /// Gets or sets the detector4 gain maximum.
        /// </summary>
        /// <value>The detector4 gain maximum.</value>
        public int Detector4GainMax
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the detector4 gain minimum.
        /// </summary>
        /// <value>The detector4 gain minimum.</value>
        public int Detector4GainMin
        {
            get;
            set;
        }

        /// <summary>
        /// Gets the detector4 gain volts.
        /// </summary>
        /// <value>The detector4 gain volts.</value>
        public double Detector4GainVolts
        {
            get
            {
                double gain = 0;
                GetParameter(Convert.ToInt32(DeviceParams.PARAM_PMT4_GAIN_POS_CURRENT_VOLTS), ref gain);
                return Math.Round(gain, 3);
            }
        }

        /// <summary>
        /// Gets the detector4 offset step size.
        /// </summary>
        /// <value>The detector4 gain offset step size.</value>
        public double Detector4OffsetStepSize
        {
            get
            {
                double offset = 0;
                if (TRUE == GetParameter(Convert.ToInt32(DeviceParams.PARAM_PMT4_OFFSET_STEP_SIZE), ref offset))
                {
                    _detector4OffsetStepSize = offset;
                }
                return _detector4OffsetStepSize;
            }
        }

        /// <summary>
        /// Gets or sets the detector4 on.
        /// </summary>
        /// <value>The detector4 on.</value>
        public int Detector4On
        {
            get
            {
                return _detector4On;
            }
            set
            {
                _detector4On = value;
                if ((_detectorsActive || FALSE == _detector4On) && 0 < _detector4gain)
                {
                    ExecutePosition(Convert.ToInt32(DeviceParams.PARAM_PMT4_ENABLE), _detector4On);
                }
                OnPropertyChanged("Detector4On");
            }
        }

        /// <summary>
        /// Gets or sets the detector4 gain offset.
        /// </summary>
        /// <value>The detector4 gain offset.</value>
        public double Detector4OutputOffset
        {
            get
            {
                double OutputOffset = 0;
                if (TRUE == GetParameter(Convert.ToInt32(DeviceParams.PARAM_PMT4_OUTPUT_OFFSET_CURRENT), ref OutputOffset))
                {
                    _detector4OutputOffset = OutputOffset;
                }
                return Math.Round(_detector4OutputOffset, 3);
            }
            set
            {
                if (value > Detector4OutputOffsetMax)
                    _detector4OutputOffset = Detector4OutputOffsetMax;
                else if (value < Detector4OutputOffsetMin)
                    _detector4OutputOffset = Detector4OutputOffsetMin;
                else
                    _detector4OutputOffset = Math.Round(value, 3);
                ExecutePosition(Convert.ToInt32(DeviceParams.PARAM_PMT4_OUTPUT_OFFSET), _detector4OutputOffset);
                OnPropertyChanged("Detector4OutputOffset");
            }
        }

        /// <summary>
        /// Gets or sets the detector4 gain offset maximum.
        /// </summary>
        /// <value>The detector4 gain offset maximum.</value>
        public double Detector4OutputOffsetMax
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the detector4 gain offset minimum.
        /// </summary>
        /// <value>The detector4 gain offset minimum.</value>
        public double Detector4OutputOffsetMin
        {
            get;
            set;
        }

        /// <summary>
        /// Gets the detector4 serial number.
        /// </summary>
        /// <value>The detector4 serial number.</value>
        public string Detector4SerialNumber
        {
            get
            {
                string sn = string.Empty;
                if (TRUE == GetParameterString(Convert.ToInt32(DeviceParams.PARAM_PMT4_SERIALNUMBER), ref sn))
                {
                    return sn;
                }
                else
                {
                    return "NA";
                }
            }
        }

        /// <summary>
        /// Gets the detector4 tripped.
        /// </summary>
        /// <value>The detector4 tripped.</value>        
        public int Detector4Tripped
        {
            get
            {
                return _detector4Tripped;
            }
        }

        /// <summary>
        /// Gets or sets the detector5 bandwidth.
        /// </summary>
        /// <value>The detector5 bandwidth.</value>
        public string Detector5Bandwidth
        {
            get
            {
                double val = 0;
                GetParameter(Convert.ToInt32(DeviceParams.PARAM_PMT5_BANDWIDTH_POS_CURRENT), ref val);
                return ConvertBandwidthToString(Convert.ToInt32(val));
            }
            set
            {
                ExecutePosition(Convert.ToInt32(DeviceParams.PARAM_PMT5_BANDWIDTH_POS), ConvertStringToBandwidth(value));
                OnPropertyChanged("Detector5Bandwidth");
            }
        }

        /// <summary>
        /// Gets the detector5 detector type.
        /// </summary>
        /// <value>The detector5 detector type.</value>
        public string Detector5DetectorType
        {
            get
            {
                double detectorType = 0;
                GetParameter(Convert.ToInt32(DeviceParams.PARAM_PMT5_DETECTOR_TYPE), ref detectorType);
                switch (detectorType)
                {
                    case 0:
                        return "[" + _detectorTypes[0] + "]";     //PMT Old Bootloader
                    case 1:
                        return "[" + _detectorTypes[1] + "]";     //PMT1000
                    case 2:
                        return "[" + _detectorTypes[2] + "]";     //PMT2100
                    case 3:
                        return "[" + _detectorTypes[3] + "]";     //PMT2106
                    case 4:
                        return "[" + _detectorTypes[4] + "]";     //APD
                    case 5:
                        return "[" + _detectorTypes[5] + "]";     //Photodiode
                    case 6:
                        return "[" + _detectorTypes[6] + "]";     //HPD1000
                    case 7:
                        return "[" + _detectorTypes[7] + "]";     //SIPM100
                    case 8:
                        return "[" + _detectorTypes[8] + "]";     //PMT2110
                    case 9:
                        return "[" + _detectorTypes[9] + "]";     //PMT3100
                    default:
                        return "[NA]";                            //Detector type not available
                }
            }
        }

        /// <summary>
        /// Gets or sets the detector5 gain.
        /// </summary>
        /// <value>The detector5 gain.</value>
        public int Detector5Gain
        {
            get
            {
                return _detector5gain;
            }
            set
            {
                if (value > Detector5GainMax)
                    _detector5gain = Detector5GainMax;
                else if (value < Detector5GainMin)
                    _detector5gain = Detector5GainMin;
                else
                    _detector5gain = value;

                if (0 == value)
                {
                    if (_detectorsActive)
                    {
                        ExecutePosition(Convert.ToInt32(DeviceParams.PARAM_PMT5_ENABLE), 0);
                    }
                }
                else if (0 < _detector5gain)
                {
                    if (_detectorsActive)
                    {
                        ExecutePosition(Convert.ToInt32(DeviceParams.PARAM_PMT5_ENABLE), _detector5On);
                    }
                }

                ExecutePosition(Convert.ToInt32(DeviceParams.PARAM_PMT5_GAIN_POS), _detector5gain);
                OnPropertyChanged("Detector5Gain");
                OnPropertyChanged("Detector5GainVolts");
            }
        }

        /// <summary>
        /// Gets or sets the detector5 gain maximum.
        /// </summary>
        /// <value>The detector5 gain maximum.</value>
        public int Detector5GainMax
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the detector5 gain minimum.
        /// </summary>
        /// <value>The detector5 gain minimum.</value>
        public int Detector5GainMin
        {
            get;
            set;
        }

        /// <summary>
        /// Gets the detector5 gain volts.
        /// </summary>
        /// <value>The detector5 gain volts.</value>
        public double Detector5GainVolts
        {
            get
            {
                double gain = 0;
                GetParameter(Convert.ToInt32(DeviceParams.PARAM_PMT5_GAIN_POS_CURRENT_VOLTS), ref gain);
                return Math.Round(gain, 3);
            }
        }

        /// <summary>
        /// Gets the detector5 offset step size.
        /// </summary>
        /// <value>The detector5 gain offset step size.</value>
        public double Detector5OffsetStepSize
        {
            get
            {
                double offset = 0;
                if (TRUE == GetParameter(Convert.ToInt32(DeviceParams.PARAM_PMT5_OFFSET_STEP_SIZE), ref offset))
                {
                    _detector5OffsetStepSize = offset;
                }
                return _detector5OffsetStepSize;
            }
        }

        /// <summary>
        /// Gets or sets the detector5 on.
        /// </summary>
        /// <value>The detector5 on.</value>
        public int Detector5On
        {
            get
            {
                return _detector5On;
            }
            set
            {
                _detector5On = value;
                if ((_detectorsActive || FALSE == _detector5On) && 0 < _detector5gain)
                {
                    ExecutePosition(Convert.ToInt32(DeviceParams.PARAM_PMT5_ENABLE), _detector5On);
                }
                OnPropertyChanged("Detector5On");
            }
        }

        /// <summary>
        /// Gets or sets the detector5 gain offset.
        /// </summary>
        /// <value>The detector5 gain offset.</value>
        public double Detector5OutputOffset
        {
            get
            {
                double OutputOffset = 0;
                if (TRUE == GetParameter(Convert.ToInt32(DeviceParams.PARAM_PMT5_OUTPUT_OFFSET_CURRENT), ref OutputOffset))
                {
                    _detector5OutputOffset = OutputOffset;
                }
                return Math.Round(_detector5OutputOffset, 3);
            }
            set
            {
                if (value > Detector5OutputOffsetMax)
                    _detector5OutputOffset = Detector5OutputOffsetMax;
                else if (value < Detector5OutputOffsetMin)
                    _detector5OutputOffset = Detector5OutputOffsetMin;
                else
                    _detector5OutputOffset = Math.Round(value, 3);
                ExecutePosition(Convert.ToInt32(DeviceParams.PARAM_PMT5_OUTPUT_OFFSET), _detector5OutputOffset);
                OnPropertyChanged("Detector5OutputOffset");
            }
        }

        /// <summary>
        /// Gets or sets the detector5 gain offset maximum.
        /// </summary>
        /// <value>The detector5 gain offset maximum.</value>
        public double Detector5OutputOffsetMax
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the detector5 gain offset minimum.
        /// </summary>
        /// <value>The detector5 gain offset minimum.</value>
        public double Detector5OutputOffsetMin
        {
            get;
            set;
        }

        /// <summary>
        /// Gets the detector5 serial number.
        /// </summary>
        /// <value>The detector5 serial number.</value>
        public string Detector5SerialNumber
        {
            get
            {
                string sn = string.Empty;
                if (TRUE == GetParameterString(Convert.ToInt32(DeviceParams.PARAM_PMT5_SERIALNUMBER), ref sn))
                {
                    return sn;
                }
                else
                {
                    return "NA";
                }
            }
        }

        /// <summary>
        /// Gets the detector5 tripped.
        /// </summary>
        /// <value>The detector5 tripped.</value>        
        public int Detector5Tripped
        {
            get
            {
                return _detector5Tripped;
            }
        }

        /// <summary>
        /// Gets or sets the detector6 bandwidth.
        /// </summary>
        /// <value>The detector6 bandwidth.</value>
        public string Detector6Bandwidth
        {
            get
            {
                double val = 0;
                GetParameter(Convert.ToInt32(DeviceParams.PARAM_PMT6_BANDWIDTH_POS_CURRENT), ref val);
                return ConvertBandwidthToString(Convert.ToInt32(val));
            }
            set
            {
                ExecutePosition(Convert.ToInt32(DeviceParams.PARAM_PMT6_BANDWIDTH_POS), ConvertStringToBandwidth(value));
                OnPropertyChanged("Detector6Bandwidth");
            }
        }

        /// <summary>
        /// Gets the detector6 detector type.
        /// </summary>
        /// <value>The detector6 detector type.</value>
        public string Detector6DetectorType
        {
            get
            {
                double detectorType = 0;
                GetParameter(Convert.ToInt32(DeviceParams.PARAM_PMT6_DETECTOR_TYPE), ref detectorType);
                switch (detectorType)
                {
                    case 0:
                        return "[" + _detectorTypes[0] + "]";     //PMT Old Bootloader
                    case 1:
                        return "[" + _detectorTypes[1] + "]";     //PMT1000
                    case 2:
                        return "[" + _detectorTypes[2] + "]";     //PMT2100
                    case 3:
                        return "[" + _detectorTypes[3] + "]";     //PMT2106
                    case 4:
                        return "[" + _detectorTypes[4] + "]";     //APD
                    case 5:
                        return "[" + _detectorTypes[5] + "]";     //Photodiode
                    case 6:
                        return "[" + _detectorTypes[6] + "]";     //HPD1000
                    case 7:
                        return "[" + _detectorTypes[7] + "]";     //SIPM100
                    case 8:
                        return "[" + _detectorTypes[8] + "]";     //PMT2110
                    case 9:
                        return "[" + _detectorTypes[9] + "]";     //PMT3100
                    default:
                        return "[NA]";                            //Detector type not available
                }
            }
        }

        /// <summary>
        /// Gets or sets the detector6 gain.
        /// </summary>
        /// <value>The detector6 gain.</value>
        public int Detector6Gain
        {
            get
            {
                return _detector6gain;
            }
            set
            {
                if (value > Detector6GainMax)
                    _detector6gain = Detector6GainMax;
                else if (value < Detector6GainMin)
                    _detector6gain = Detector6GainMin;
                else
                    _detector6gain = value;

                if (0 == value)
                {
                    if (_detectorsActive)
                    {
                        ExecutePosition(Convert.ToInt32(DeviceParams.PARAM_PMT6_ENABLE), 0);
                    }
                }
                else if (0 < _detector6gain)
                {
                    if (_detectorsActive)
                    {
                        ExecutePosition(Convert.ToInt32(DeviceParams.PARAM_PMT6_ENABLE), _detector6On);
                    }
                }

                ExecutePosition(Convert.ToInt32(DeviceParams.PARAM_PMT6_GAIN_POS), _detector6gain);
                OnPropertyChanged("Detector6Gain");
                OnPropertyChanged("Detector6GainVolts");
            }
        }

        /// <summary>
        /// Gets or sets the detector6 gain maximum.
        /// </summary>
        /// <value>The detector6 gain maximum.</value>
        public int Detector6GainMax
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the detector6 gain minimum.
        /// </summary>
        /// <value>The detector6 gain minimum.</value>
        public int Detector6GainMin
        {
            get;
            set;
        }

        /// <summary>
        /// Gets the detector6 gain volts.
        /// </summary>
        /// <value>The detector6 gain volts.</value>
        public double Detector6GainVolts
        {
            get
            {
                double gain = 0;
                GetParameter(Convert.ToInt32(DeviceParams.PARAM_PMT6_GAIN_POS_CURRENT_VOLTS), ref gain);
                return Math.Round(gain, 3);
            }
        }

        /// <summary>
        /// Gets the detector6 offset step size.
        /// </summary>
        /// <value>The detector6 gain offset step size.</value>
        public double Detector6OffsetStepSize
        {
            get
            {
                double offset = 0;
                if (TRUE == GetParameter(Convert.ToInt32(DeviceParams.PARAM_PMT6_OFFSET_STEP_SIZE), ref offset))
                {
                    _detector6OffsetStepSize = offset;
                }
                return _detector6OffsetStepSize;
            }
        }

        /// <summary>
        /// Gets or sets the detector6 on.
        /// </summary>
        /// <value>The detector6 on.</value>
        public int Detector6On
        {
            get
            {
                return _detector6On;
            }
            set
            {
                _detector6On = value;
                if ((_detectorsActive || FALSE == _detector6On) && 0 < _detector6gain)
                {
                    ExecutePosition(Convert.ToInt32(DeviceParams.PARAM_PMT6_ENABLE), _detector6On);
                }
                OnPropertyChanged("Detector6On");
            }
        }

        /// <summary>
        /// Gets or sets the detector6 gain offset.
        /// </summary>
        /// <value>The detector6 gain offset.</value>
        public double Detector6OutputOffset
        {
            get
            {
                double OutputOffset = 0;
                if (TRUE == GetParameter(Convert.ToInt32(DeviceParams.PARAM_PMT6_OUTPUT_OFFSET_CURRENT), ref OutputOffset))
                {
                    _detector6OutputOffset = OutputOffset;
                }
                return Math.Round(_detector6OutputOffset, 3);
            }
            set
            {
                if (value > Detector6OutputOffsetMax)
                    _detector6OutputOffset = Detector6OutputOffsetMax;
                else if (value < Detector6OutputOffsetMin)
                    _detector6OutputOffset = Detector6OutputOffsetMin;
                else
                    _detector6OutputOffset = Math.Round(value, 3);
                ExecutePosition(Convert.ToInt32(DeviceParams.PARAM_PMT6_OUTPUT_OFFSET), _detector6OutputOffset);
                OnPropertyChanged("Detector6OutputOffset");
            }
        }

        /// <summary>
        /// Gets or sets the detector6 gain offset maximum.
        /// </summary>
        /// <value>The detector6 gain offset maximum.</value>
        public double Detector6OutputOffsetMax
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the detector6 gain offset minimum.
        /// </summary>
        /// <value>The detector6 gain offset minimum.</value>
        public double Detector6OutputOffsetMin
        {
            get;
            set;
        }

        /// <summary>
        /// Gets the detector6 serial number.
        /// </summary>
        /// <value>The detector6 serial number.</value>
        public string Detector6SerialNumber
        {
            get
            {
                string sn = string.Empty;
                if (TRUE == GetParameterString(Convert.ToInt32(DeviceParams.PARAM_PMT6_SERIALNUMBER), ref sn))
                {
                    return sn;
                }
                else
                {
                    return "NA";
                }
            }
        }

        /// <summary>
        /// Gets the detector6 tripped.
        /// </summary>
        /// <value>The detector6 tripped.</value>        
        public int Detector6Tripped
        {
            get
            {
                return _detector6Tripped;
            }
        }

        /// <summary>
        /// Gets the image path play.
        /// </summary>
        /// <value>The image path play.</value>
        public string ImagePathPlay
        {
            get
            {
                if (_detectorsActive)
                {
                    return @"/ThorDetector_Control;component/Stop.png";
                }
                else
                {
                    return @"/ThorDetector_Control;component/Play.png";
                }
            }
        }

        /// <summary>
        /// Gets the tripped color1.
        /// </summary>
        /// <value>The tripped color1.</value>
        public Brush TrippedColor1
        {
            get
            {
                return new SolidColorBrush((1 == Detector1Tripped && _detectorsActive && 1 == _detector1On && 0 < _detector1gain) ? Colors.Green : Colors.Red);
            }
        }

        /// <summary>
        /// Gets the tripped color2.
        /// </summary>
        /// <value>The tripped color2.</value>
        public Brush TrippedColor2
        {
            get
            {
                return new SolidColorBrush((1 == Detector2Tripped && _detectorsActive && 1 == _detector2On && 0 < _detector2gain) ? Colors.Green : Colors.Red);
            }
        }

        /// <summary>
        /// Gets the tripped color3.
        /// </summary>
        /// <value>The tripped color3.</value>
        public Brush TrippedColor3
        {
            get
            {
                return new SolidColorBrush((1 == Detector3Tripped && _detectorsActive && 1 == _detector3On && 0 < _detector3gain) ? Colors.Green : Colors.Red);
            }
        }

        /// <summary>
        /// Gets the tripped color4.
        /// </summary>
        /// <value>The tripped color4.</value>
        public Brush TrippedColor4
        {
            get
            {
                return new SolidColorBrush((1 == Detector4Tripped && _detectorsActive && 1 == _detector4On && 0 < _detector4gain) ? Colors.Green : Colors.Red);
            }
        }

        /// <summary>
        /// Gets the tripped color5.
        /// </summary>
        /// <value>The tripped color5.</value>
        public Brush TrippedColor5
        {
            get
            {
                return new SolidColorBrush((1 == Detector5Tripped && _detectorsActive && 1 == _detector5On && 0 < _detector5gain) ? Colors.Green : Colors.Red);
            }
        }

        /// <summary>
        /// Gets the tripped color6.
        /// </summary>
        /// <value>The tripped color6.</value>
        public Brush TrippedColor6
        {
            get
            {
                return new SolidColorBrush((1 == Detector6Tripped && _detectorsActive && 1 == _detector6On && 0 < _detector6gain) ? Colors.Green : Colors.Red);
            }
        }

        /// <summary>
        /// Gets the tripped text1.
        /// </summary>
        /// <value>The tripped text1.</value>
        public string TrippedText1
        {
            get
            {
                if (0 == Detector1Tripped)
                {
                    return "TRIP";
                }
                else if (_detectorsActive && 1 == _detector1On && 0 != _detector1gain)
                {
                    return "ON";
                }
                else
                {
                    return "OFF";
                }
            }
        }

        /// <summary>
        /// Gets the tripped text2.
        /// </summary>
        /// <value>The tripped text2.</value>
        public string TrippedText2
        {
            get
            {
                if (0 == Detector2Tripped)
                {
                    return "TRIP";
                }
                else if (_detectorsActive && 1 == _detector2On && 0 != _detector2gain)
                {
                    return "ON";
                }
                else
                {
                    return "OFF";
                }
            }
        }

        /// <summary>
        /// Gets the tripped text3.
        /// </summary>
        /// <value>The tripped text3.</value>
        public string TrippedText3
        {
            get
            {
                if (0 == Detector3Tripped)
                {
                    return "TRIP";
                }
                else if (_detectorsActive && 1 == _detector3On && 0 != _detector3gain)
                {
                    return "ON";
                }
                else
                {
                    return "OFF";
                }
            }
        }

        /// <summary>
        /// Gets the tripped text4.
        /// </summary>
        /// <value>The tripped text4.</value>
        public string TrippedText4
        {
            get
            {
                if (0 == Detector4Tripped)
                {
                    return "TRIP";
                }
                else if (_detectorsActive && 1 == _detector4On && 0 != _detector4gain)
                {
                    return "ON";
                }
                else
                {
                    return "OFF";
                }
            }
        }

        /// <summary>
        /// Gets the tripped text5.
        /// </summary>
        /// <value>The tripped text5.</value>
        public string TrippedText5
        {
            get
            {
                if (0 == Detector5Tripped)
                {
                    return "TRIP";
                }
                else if (_detectorsActive && 1 == _detector5On && 0 != _detector5gain)
                {
                    return "ON";
                }
                else
                {
                    return "OFF";
                }
            }
        }

        /// <summary>
        /// Gets the tripped text6.
        /// </summary>
        /// <value>The tripped text6.</value>
        public string TrippedText6
        {
            get
            {
                if (0 == Detector6Tripped)
                {
                    return "TRIP";
                }
                else if (_detectorsActive && 1 == _detector6On && 0 != _detector6gain)
                {
                    return "ON";
                }
                else
                {
                    return "OFF";
                }
            }
        }

        /// <summary>
        /// Gets the tripped vis1.
        /// </summary>
        /// <value>The tripped vis1.</value>
        public Visibility TrippedVis1
        {
            get
            {
                return (1 == Detector1Tripped) ? Visibility.Hidden : Visibility.Visible;
            }
        }

        /// <summary>
        /// Gets the tripped vis2.
        /// </summary>
        /// <value>The tripped vis2.</value>
        public Visibility TrippedVis2
        {
            get
            {
                return (1 == Detector2Tripped) ? Visibility.Hidden : Visibility.Visible;
            }
        }

        /// <summary>
        /// Gets the tripped vis3.
        /// </summary>
        /// <value>The tripped vis3.</value>
        public Visibility TrippedVis3
        {
            get
            {
                return (1 == Detector3Tripped) ? Visibility.Hidden : Visibility.Visible;
            }
        }

        /// <summary>
        /// Gets the tripped vis4.
        /// </summary>
        /// <value>The tripped vis4.</value>
        public Visibility TrippedVis4
        {
            get
            {
                return (1 == Detector4Tripped) ? Visibility.Hidden : Visibility.Visible;
            }
        }

        /// <summary>
        /// Gets the tripped vis5.
        /// </summary>
        /// <value>The tripped vis5.</value>
        public Visibility TrippedVis5
        {
            get
            {
                return (1 == Detector5Tripped) ? Visibility.Hidden : Visibility.Visible;
            }
        }

        /// <summary>
        /// Gets the tripped vis6.
        /// </summary>
        /// <value>The tripped vis6.</value>
        public Visibility TrippedVis6
        {
            get
            {
                return (1 == Detector6Tripped) ? Visibility.Hidden : Visibility.Visible;
            }
        }

        #endregion Properties

        #region Methods

        /// <summary>
        /// Handles the Click event of the btnDetector1Minus control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnDetector1Minus_Click(object sender, RoutedEventArgs e)
        {
            Detector1Gain -= 1;
        }

        /// <summary>
        /// Handles the Click event of the btnDetector1OffsetMinus control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnDetector1OffsetMinus_Click(object sender, RoutedEventArgs e)
        {
            Detector1OutputOffset -= Detector1OffsetStepSize;
        }

        /// <summary>
        /// Handles the Click event of the btnDetector1OffsetPlus control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnDetector1OffsetPlus_Click(object sender, RoutedEventArgs e)
        {
            Detector1OutputOffset += Detector1OffsetStepSize;
        }

        /// <summary>
        /// Handles the Click event of the btnDetector1Plus control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnDetector1Plus_Click(object sender, RoutedEventArgs e)
        {
            Detector1Gain += 1;
        }

        /// <summary>
        /// Handles the Click event of the btnDetector2Minus control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnDetector2Minus_Click(object sender, RoutedEventArgs e)
        {
            Detector2Gain -= 1;
        }

        /// <summary>
        /// Handles the Click event of the btnDetector2OffsetMinus control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnDetector2OffsetMinus_Click(object sender, RoutedEventArgs e)
        {
            Detector2OutputOffset -= Detector2OffsetStepSize;
        }

        /// <summary>
        /// Handles the Click event of the btnDetector2OffsetPlus control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnDetector2OffsetPlus_Click(object sender, RoutedEventArgs e)
        {
            Detector2OutputOffset += Detector2OffsetStepSize;
        }

        /// <summary>
        /// Handles the Click event of the btnDetector2Plus control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnDetector2Plus_Click(object sender, RoutedEventArgs e)
        {
            Detector2Gain += 1;
        }

        /// <summary>
        /// Handles the Click event of the btnDetector3Minus control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnDetector3Minus_Click(object sender, RoutedEventArgs e)
        {
            Detector3Gain -= 1;
        }

        /// <summary>
        /// Handles the Click event of the btnDetector3OffsetMinus control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnDetector3OffsetMinus_Click(object sender, RoutedEventArgs e)
        {
            Detector3OutputOffset -= Detector3OffsetStepSize;
        }

        /// <summary>
        /// Handles the Click event of the btnDetector3OffsetPlus control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnDetector3OffsetPlus_Click(object sender, RoutedEventArgs e)
        {
            Detector3OutputOffset += Detector3OffsetStepSize;
        }

        /// <summary>
        /// Handles the Click event of the btnDetector3Plus control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnDetector3Plus_Click(object sender, RoutedEventArgs e)
        {
            Detector3Gain += 1;
        }

        /// <summary>
        /// Handles the Click event of the btnDetector4Minus control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnDetector4Minus_Click(object sender, RoutedEventArgs e)
        {
            Detector4Gain -= 1;
        }

        /// <summary>
        /// Handles the Click event of the btnDetector4OffsetMinus control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnDetector4OffsetMinus_Click(object sender, RoutedEventArgs e)
        {
            Detector4OutputOffset -= Detector4OffsetStepSize;
        }

        /// <summary>
        /// Handles the Click event of the btnDetector4OffsetPlus control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnDetector4OffsetPlus_Click(object sender, RoutedEventArgs e)
        {
            Detector4OutputOffset += Detector4OffsetStepSize;
        }

        /// <summary>
        /// Handles the Click event of the btnDetector4Plus control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnDetector4Plus_Click(object sender, RoutedEventArgs e)
        {
            Detector4Gain += 1;
        }

        /// <summary>
        /// Handles the Click event of the btnDetector5Minus control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnDetector5Minus_Click(object sender, RoutedEventArgs e)
        {
            Detector5Gain -= 1;
        }

        /// <summary>
        /// Handles the Click event of the btnDetector5OffsetMinus control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnDetector5OffsetMinus_Click(object sender, RoutedEventArgs e)
        {
            Detector5OutputOffset -= Detector5OffsetStepSize;
        }

        /// <summary>
        /// Handles the Click event of the btnDetector5OffsetPlus control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnDetector5OffsetPlus_Click(object sender, RoutedEventArgs e)
        {
            Detector5OutputOffset += Detector5OffsetStepSize;
        }

        /// <summary>
        /// Handles the Click event of the btnDetector5Plus control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnDetector5Plus_Click(object sender, RoutedEventArgs e)
        {
            Detector5Gain += 1;
        }

        /// <summary>
        /// Handles the Click event of the btnDetector6Minus control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnDetector6Minus_Click(object sender, RoutedEventArgs e)
        {
            Detector6Gain -= 1;
        }

        /// <summary>
        /// Handles the Click event of the btnDetector6OffsetMinus control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnDetector6OffsetMinus_Click(object sender, RoutedEventArgs e)
        {
            Detector6OutputOffset -= Detector6OffsetStepSize;
        }

        /// <summary>
        /// Handles the Click event of the btnDetector6OffsetPlus control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnDetector6OffsetPlus_Click(object sender, RoutedEventArgs e)
        {
            Detector6OutputOffset += Detector6OffsetStepSize;
        }

        /// <summary>
        /// Handles the Click event of the btnDetector6Plus control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnDetector6Plus_Click(object sender, RoutedEventArgs e)
        {
            Detector6Gain += 1;
        }

        /// <summary>
        /// Handles the Click event of the btnExit control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnExit_Click(object sender, RoutedEventArgs e)
        {
            this.Close();
        }

        /// <summary>
        /// Handles the Click event of the btnReset1 control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnReset1_Click(object sender, RoutedEventArgs e)
        {
            Detector1On = 0;
            Detector1On = 1;
        }

        /// <summary>
        /// Handles the Click event of the btnReset2 control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnReset2_Click(object sender, RoutedEventArgs e)
        {
            Detector2On = 0;
            Detector2On = 1;
        }

        /// <summary>
        /// Handles the Click event of the btnReset3 control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnReset3_Click(object sender, RoutedEventArgs e)
        {
            Detector3On = 0;
            Detector3On = 1;
        }

        /// <summary>
        /// Handles the Click event of the btnReset4 control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnReset4_Click(object sender, RoutedEventArgs e)
        {
            Detector4On = 0;
            Detector4On = 1;
        }

        /// <summary>
        /// Handles the Click event of the btnReset5 control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnReset5_Click(object sender, RoutedEventArgs e)
        {
            Detector5On = 0;
            Detector5On = 1;
        }

        /// <summary>
        /// Handles the Click event of the btnReset6 control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnReset6_Click(object sender, RoutedEventArgs e)
        {
            Detector6On = 0;
            Detector6On = 1;
        }

        /// <summary>
        /// Handles the Click event of the btnUpdateFirmware control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnUpdateFirmware_Click(object sender, RoutedEventArgs e)
        {
            if (_devices != 0)
            {
                if (0 != ((uint)_devices & (uint)DeviceType.PMT1))
                {
                    COMPortManager.selectedDetectorIndex = 1;
                    double firmwareVersion1 = 0; //Eventual parameter for passing firmware version when updating firmware (needs to be set properly)
                    if (MessageBox.Show("Do you want to update Detector " + COMPortManager.selectedDetectorIndex, "Firmware Update", MessageBoxButton.YesNo) == MessageBoxResult.Yes)
                    {
                        if (UpdateDetectorFirmware(COMPortManager.selectedDetectorIndex, firmwareVersion1, Detector1DetectorType))
                        {
                            COMPortManager.ewh.WaitOne();
                        }
                    }

                }
                if (0 != ((uint)_devices & (uint)DeviceType.PMT2))
                {
                    COMPortManager.selectedDetectorIndex = 2;
                    double firmwareVersion2 = 0; //Eventual parameter for passing firmware version when updating firmware (needs to be set properly)
                    if (MessageBox.Show("Do you want to update Detector " + COMPortManager.selectedDetectorIndex, "Firmware Update", MessageBoxButton.YesNo) == MessageBoxResult.Yes)
                    {
                        if (UpdateDetectorFirmware(COMPortManager.selectedDetectorIndex, firmwareVersion2, Detector2DetectorType))
                        {
                            COMPortManager.ewh.WaitOne();
                        }
                    }

                }
                if (0 != ((uint)_devices & (uint)DeviceType.PMT3))
                {
                    COMPortManager.selectedDetectorIndex = 3;
                    double firmwareVersion3 = 0; //Eventual parameter for passing firmware version when updating firmware (needs to be set properly)
                    if (MessageBox.Show("Do you want to update Detector " + COMPortManager.selectedDetectorIndex, "Firmware Update", MessageBoxButton.YesNo) == MessageBoxResult.Yes)
                    {
                        if (UpdateDetectorFirmware(COMPortManager.selectedDetectorIndex, firmwareVersion3, Detector3DetectorType))
                        {
                            COMPortManager.ewh.WaitOne();
                        }
                    }

                }
                if (0 != ((uint)_devices & (uint)DeviceType.PMT4))
                {
                    COMPortManager.selectedDetectorIndex = 4;
                    double firmwareVersion4 = 0; //Eventual parameter for passing firmware version when updating firmware (needs to be set properly)
                    if (MessageBox.Show("Do you want to update Detector " + COMPortManager.selectedDetectorIndex, "Firmware Update", MessageBoxButton.YesNo) == MessageBoxResult.Yes)
                    {
                        if (UpdateDetectorFirmware(COMPortManager.selectedDetectorIndex, firmwareVersion4, Detector4DetectorType))
                        {
                            COMPortManager.ewh.WaitOne();
                        }
                    }

                }
                if (0 != ((uint)_devices & (uint)DeviceType.PMT5))
                {
                    COMPortManager.selectedDetectorIndex = 5;
                    double firmwareVersion5 = 0; //Eventual parameter for passing firmware version when updating firmware (needs to be set properly)
                    if (MessageBox.Show("Do you want to update Detector " + COMPortManager.selectedDetectorIndex, "Firmware Update", MessageBoxButton.YesNo) == MessageBoxResult.Yes)
                    {
                        if (UpdateDetectorFirmware(COMPortManager.selectedDetectorIndex, firmwareVersion5, Detector5DetectorType))
                        {
                            COMPortManager.ewh.WaitOne();
                        }
                    }

                }
                if (0 != ((uint)_devices & (uint)DeviceType.PMT6))
                {
                    COMPortManager.selectedDetectorIndex = 6;
                    double firmwareVersion6 = 0; //Eventual parameter for passing firmware version when updating firmware (needs to be set properly)
                    if (MessageBox.Show("Do you want to update Detector " + COMPortManager.selectedDetectorIndex, "Firmware Update", MessageBoxButton.YesNo) == MessageBoxResult.Yes)
                    {
                        if (UpdateDetectorFirmware(COMPortManager.selectedDetectorIndex, firmwareVersion6, Detector6DetectorType))
                        {
                            COMPortManager.ewh.WaitOne();
                        }
                    }

                }
                // Query and save
            }
            else
            {
                COMPortManager.selectedDetectorIndex = 1;
                double firmwareVersion1 = 0; //Eventual parameter for passing firmware version when updating firmware (needs to be set properly)
                if (MessageBox.Show("Do you want to update Detector", "Firmware Update", MessageBoxButton.YesNo) == MessageBoxResult.Yes)
                {
                    if (UpdateDetectorFirmware(COMPortManager.selectedDetectorIndex, firmwareVersion1, Detector1DetectorType))
                    {
                        COMPortManager.ewh.WaitOne();
                    }
                }
            }
        }

        /// <summary>
        /// Handles the Click event of the btnUpdateSerial1 control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnUpdateSerial1_Click(object sender, RoutedEventArgs e)
        {
            string settingsFile = Environment.CurrentDirectory + "\\ThorDetectorSettings.xml";
            XmlDocument settingsXML = new XmlDocument();
            settingsXML.Load(settingsFile);
            XmlNodeList ndList = settingsXML.SelectNodes("/ThorDetectorSettings/Detector1");
            if (ndList.Count > 0)
            {
                SetAttribute(ndList[0], settingsXML, "serialNumber", Detector1SerialNumber);
            }
            settingsXML.Save(settingsFile);
        }

        /// <summary>
        /// Handles the Click event of the btnUpdateSerial2 control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnUpdateSerial2_Click(object sender, RoutedEventArgs e)
        {
            string settingsFile = Environment.CurrentDirectory + "\\ThorDetectorSettings.xml";
            XmlDocument settingsXML = new XmlDocument();
            settingsXML.Load(settingsFile);
            XmlNodeList ndList = settingsXML.SelectNodes("/ThorDetectorSettings/Detector2");
            if (ndList.Count > 0)
            {
                SetAttribute(ndList[0], settingsXML, "serialNumber", Detector2SerialNumber);
            }
            settingsXML.Save(settingsFile);
        }

        /// <summary>
        /// Handles the Click event of the btnUpdateSerial3 control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnUpdateSerial3_Click(object sender, RoutedEventArgs e)
        {
            string settingsFile = Environment.CurrentDirectory + "\\ThorDetectorSettings.xml";
            XmlDocument settingsXML = new XmlDocument();
            settingsXML.Load(settingsFile);
            XmlNodeList ndList = settingsXML.SelectNodes("/ThorDetectorSettings/Detector3");
            if (ndList.Count > 0)
            {
                SetAttribute(ndList[0], settingsXML, "serialNumber", Detector3SerialNumber);
            }
            settingsXML.Save(settingsFile);
        }

        /// <summary>
        /// Handles the Click event of the btnUpdateSerial4 control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnUpdateSerial4_Click(object sender, RoutedEventArgs e)
        {
            string settingsFile = Environment.CurrentDirectory + "\\ThorDetectorSettings.xml";
            XmlDocument settingsXML = new XmlDocument();
            settingsXML.Load(settingsFile);
            XmlNodeList ndList = settingsXML.SelectNodes("/ThorDetectorSettings/Detector4");
            if (ndList.Count > 0)
            {
                SetAttribute(ndList[0], settingsXML, "serialNumber", Detector4SerialNumber);
            }
            settingsXML.Save(settingsFile);
        }

        /// <summary>
        /// Handles the Click event of the btnUpdateSerial5 control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnUpdateSerial5_Click(object sender, RoutedEventArgs e)
        {
            string settingsFile = Environment.CurrentDirectory + "\\ThorDetectorSettings.xml";
            XmlDocument settingsXML = new XmlDocument();
            settingsXML.Load(settingsFile);
            XmlNodeList ndList = settingsXML.SelectNodes("/ThorDetectorSettings/Detector5");
            if (ndList.Count > 0)
            {
                SetAttribute(ndList[0], settingsXML, "serialNumber", Detector5SerialNumber);
            }
            settingsXML.Save(settingsFile);
        }

        /// <summary>
        /// Handles the Click event of the btnUpdateSerial6 control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnUpdateSerial6_Click(object sender, RoutedEventArgs e)
        {
            string settingsFile = Environment.CurrentDirectory + "\\ThorDetectorSettings.xml";
            XmlDocument settingsXML = new XmlDocument();
            settingsXML.Load(settingsFile);
            XmlNodeList ndList = settingsXML.SelectNodes("/ThorDetectorSettings/Detector6");
            if (ndList.Count > 0)
            {
                SetAttribute(ndList[0], settingsXML, "serialNumber", Detector6SerialNumber);
            }
            settingsXML.Save(settingsFile);
        }

        /// <summary>
        /// Converts the bandwidth to string.
        /// </summary>
        /// <param name="bandwidth">The bandwidth.</param>
        /// <returns>System.Int32.</returns>
        private string ConvertBandwidthToString(int bandwidth)
        {
            switch (bandwidth)
            {
                case 300000000:
                    return "300 MHz";   //300MHz
                case 200000000:
                    return "200 MHz";   //200MHz
                case 80000000:
                    return "80 MHz";    //80MHz
                case 30000000:
                    return "30 MHz";    //30MHz
                case 15000000:
                    return "15 MHz";    //15MHz
                case 2500000:
                    return "2.5 MHz";   //2.5MHz
                case 1000000:
                    return "1 MHz";     //1MHz
                case 250000:
                    return "250 kHz";   //250kHz
                default:
                    return "80 MHz";    //80MHz
            }
        }

        /// <summary>
        /// Converts the string to bandwidth.
        /// </summary>
        /// <param name="index">The index.</param>
        /// <returns>System.Int32.</returns>
        private int ConvertStringToBandwidth(string bandwidth)
        {
            switch (bandwidth)
            {
                case "300 MHz":
                    return 300000000;    //300MHz
                case "200 MHz":
                    return 200000000;    //200MHz
                case "80 MHz":
                    return 80000000;     //80MHz
                case "30 MHz":
                    return 30000000;     //30MHz
                case "15 MHz":
                    return 15000000;     //15MHz
                case "2.5 MHz":
                    return 2500000;      //2.5MHz
                case "1 MHz":
                    return 1000000;      //1MHz
                case "250 kHz":
                    return 250000;       //250kHz
                default:
                    return 80000000;     //80MHz
            }
        }

        /// <summary>
        /// Handles the Exit event of the Current control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="ExitEventArgs"/> instance containing the event data.</param>
        void Current_Exit(object sender, ExitEventArgs e)
        {
            TearDown();
            this.Close();
        }

        /// <summary>
        /// Executes the position.
        /// </summary>
        /// <param name="param">The parameter.</param>
        /// <param name="pos">The position.</param>
        private void ExecutePosition(int param, double pos)
        {
            ThorDetectorFunctions.SetParam(param, pos);
            ThorDetectorFunctions.PreflightPosition();
            ThorDetectorFunctions.SetupPosition();
            ThorDetectorFunctions.StartPosition();
            ThorDetectorFunctions.PostflightPosition();
        }

        void GenerateBandwidthList(int paramID, int paramID2, int index)
        {
            ObservableCollection<string> tempList = new ObservableCollection<string>();
            int j = 0;
            if (_stringToBandwidthMap.Count == 0)
            {
                foreach (DetectorBandwidths k in DetectorBandwidths.GetValues(typeof(DetectorBandwidths)))
                {
                    _stringToBandwidthMap.Add(_bandwidthTags[j], (int)k);
                    _bandwidthToStringMap.Add((int)k, _bandwidthTags[j]);
                    j++;
                }
            }
            foreach (DetectorBandwidths k in DetectorBandwidths.GetValues(typeof(DetectorBandwidths)))
            {
                if (_stringToBandwidthMap.Count == 0)
                {
                    _stringToBandwidthMap.Add(_bandwidthTags[j], (int)k);
                    _bandwidthToStringMap.Add((int)k, _bandwidthTags[j]);
                    j++;
                }
                //Try to set the bandwidth value to the PMT
                double tempBandwidth = 0;
                ExecutePosition(Convert.ToInt32(paramID), (int)k);
                Thread.Sleep(50);
                ThorDetectorFunctions.GetParam(Convert.ToInt32(paramID2), ref tempBandwidth);
                //If the PMT supports that bandwidth it will return the bandwidth or the closest one it can get to
                if ((int)k == tempBandwidth)
                {
                    if (k != DetectorBandwidths.BW_1MHz) // We need to ignore the 1MHz option, it is never used and it is not very stable according to Panchy
                    {
                        tempList.Add(_bandwidthToStringMap[(int)k]);
                    }
                }
            }
            //Only add to the list the first time the application boots up (clicking refresh button modifies list)
            if (_bandwidthList.Count <= 6)
            {
                _bandwidthList.Add(tempList);
            }
            else
            {
                _bandwidthList[index] = tempList;
            }
        }

        //get the attribute value from the input node and document
        //if the attribute does not exist return false
        /// <summary>
        /// Gets the attribute.
        /// </summary>
        /// <param name="node">The node.</param>
        /// <param name="doc">The document.</param>
        /// <param name="attrName">Name of the attribute.</param>
        /// <param name="attrValue">The attribute value.</param>
        /// <returns><c>true</c> if XXXX, <c>false</c> otherwise.</returns>
        private bool GetAttribute(XmlNode node, XmlDocument doc, string attrName, ref string attrValue)
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

        /// <summary>
        /// Gets the desired parameter from the device dll.
        /// </summary>
        /// <param name="paramId">The parameter identifier.</param>
        /// <param name="param">The parameter.</param>
        /// <returns>System.Int32.</returns>
        int GetParameter(int paramId, ref double param)
        {
            if (false == _initialized) return FALSE;
            if (TRUE == ThorDetectorFunctions.GetParam(paramId, ref param))
            {
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }

        /// <summary>
        /// Gets the desired parameter from the device dll.
        /// </summary>
        /// <param name="paramId">The parameter identifier.</param>
        /// <param name="paramString">The parameter string.</param>
        /// <returns>System.Int32.</returns>
        int GetParameterString(int paramId, ref string paramString)
        {
            if (false == _initialized) return FALSE;
            const int LENGTH = 255;
            StringBuilder paramSB = new StringBuilder(LENGTH);
            if (TRUE == ThorDetectorFunctions.GetParamString(paramId, paramSB, LENGTH))
            {
                paramString = paramSB.ToString();
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }

        /// <summary>
        /// Converts a Hexadecimal string to Ascii
        /// </summary>
        /// <param name="hexString">The Hexadecimal string.</param>
        /// <returns>System.string.</returns>
        private string HexString2Ascii(string hexString)
        {
            StringBuilder sb = new StringBuilder();
            for (int i = 0; i <= hexString.Length - 2; i += 2)
            {
                sb.Append(Convert.ToString(Convert.ToChar(Int64.Parse(hexString.Substring(i, 2), System.Globalization.NumberStyles.HexNumber))));
            }
            return sb.ToString();
        }

        /// <summary>
        /// Initializes all the views and properties of this window
        /// </summary>
        void Initialize()
        {
            try
            {
                if (false == InitializeConnectionsAndViews())
                {
                    return;
                }

                int paramType = 0, paramAvailable = 0, paramReadOnly = 0;
                double paramMin = 0, paramMax = 0, paramDefault = 0;

                ThorDetectorFunctions.GetParamInfo(Convert.ToInt32(DeviceParams.PARAM_PMT1_GAIN_POS), ref paramType, ref paramAvailable, ref paramReadOnly, ref paramMin, ref paramMax, ref paramDefault);
                Detector1GainMin = (int)paramMin;
                Detector1GainMax = (int)paramMax;

                ThorDetectorFunctions.GetParamInfo(Convert.ToInt32(DeviceParams.PARAM_PMT2_GAIN_POS), ref paramType, ref paramAvailable, ref paramReadOnly, ref paramMin, ref paramMax, ref paramDefault);
                Detector2GainMin = (int)paramMin;
                Detector2GainMax = (int)paramMax;

                ThorDetectorFunctions.GetParamInfo(Convert.ToInt32(DeviceParams.PARAM_PMT3_GAIN_POS), ref paramType, ref paramAvailable, ref paramReadOnly, ref paramMin, ref paramMax, ref paramDefault);
                Detector3GainMin = (int)paramMin;
                Detector3GainMax = (int)paramMax;

                ThorDetectorFunctions.GetParamInfo(Convert.ToInt32(DeviceParams.PARAM_PMT4_GAIN_POS), ref paramType, ref paramAvailable, ref paramReadOnly, ref paramMin, ref paramMax, ref paramDefault);
                Detector4GainMin = (int)paramMin;
                Detector4GainMax = (int)paramMax;

                ThorDetectorFunctions.GetParamInfo(Convert.ToInt32(DeviceParams.PARAM_PMT5_GAIN_POS), ref paramType, ref paramAvailable, ref paramReadOnly, ref paramMin, ref paramMax, ref paramDefault);
                Detector5GainMin = (int)paramMin;
                Detector5GainMax = (int)paramMax;

                ThorDetectorFunctions.GetParamInfo(Convert.ToInt32(DeviceParams.PARAM_PMT6_GAIN_POS), ref paramType, ref paramAvailable, ref paramReadOnly, ref paramMin, ref paramMax, ref paramDefault);
                Detector6GainMin = (int)paramMin;
                Detector6GainMax = (int)paramMax;

                ThorDetectorFunctions.GetParamInfo(Convert.ToInt32(DeviceParams.PARAM_PMT1_OUTPUT_OFFSET), ref paramType, ref paramAvailable, ref paramReadOnly, ref paramMin, ref paramMax, ref paramDefault);
                Detector1OutputOffsetMin = paramMin;
                Detector1OutputOffsetMax = paramMax;

                ThorDetectorFunctions.GetParamInfo(Convert.ToInt32(DeviceParams.PARAM_PMT2_OUTPUT_OFFSET), ref paramType, ref paramAvailable, ref paramReadOnly, ref paramMin, ref paramMax, ref paramDefault);
                Detector2OutputOffsetMin = paramMin;
                Detector2OutputOffsetMax = paramMax;

                ThorDetectorFunctions.GetParamInfo(Convert.ToInt32(DeviceParams.PARAM_PMT3_OUTPUT_OFFSET), ref paramType, ref paramAvailable, ref paramReadOnly, ref paramMin, ref paramMax, ref paramDefault);
                Detector3OutputOffsetMin = paramMin;
                Detector3OutputOffsetMax = paramMax;

                ThorDetectorFunctions.GetParamInfo(Convert.ToInt32(DeviceParams.PARAM_PMT4_OUTPUT_OFFSET), ref paramType, ref paramAvailable, ref paramReadOnly, ref paramMin, ref paramMax, ref paramDefault);
                Detector4OutputOffsetMin = paramMin;
                Detector4OutputOffsetMax = paramMax;

                ThorDetectorFunctions.GetParamInfo(Convert.ToInt32(DeviceParams.PARAM_PMT5_OUTPUT_OFFSET), ref paramType, ref paramAvailable, ref paramReadOnly, ref paramMin, ref paramMax, ref paramDefault);
                Detector5OutputOffsetMin = paramMin;
                Detector5OutputOffsetMax = paramMax;

                ThorDetectorFunctions.GetParamInfo(Convert.ToInt32(DeviceParams.PARAM_PMT6_OUTPUT_OFFSET), ref paramType, ref paramAvailable, ref paramReadOnly, ref paramMin, ref paramMax, ref paramDefault);
                Detector6OutputOffsetMin = paramMin;
                Detector6OutputOffsetMax = paramMax;
                OnPropertyChanged("Detector1GainMin");
                OnPropertyChanged("Detector1GainMax");
                OnPropertyChanged("Detector2GainMin");
                OnPropertyChanged("Detector2GainMax");
                OnPropertyChanged("Detector3GainMin");
                OnPropertyChanged("Detector3GainMax");
                OnPropertyChanged("Detector4GainMin");
                OnPropertyChanged("Detector4GainMax");
                OnPropertyChanged("Detector5GainMin");
                OnPropertyChanged("Detector5GainMax");
                OnPropertyChanged("Detector6GainMin");
                OnPropertyChanged("Detector6GainMax");

                OnPropertyChanged("Detector1OutputOffsetMin");
                OnPropertyChanged("Detector1OutputOffsetMax");
                OnPropertyChanged("Detector2OutputOffsetMin");
                OnPropertyChanged("Detector2OutputOffsetMax");
                OnPropertyChanged("Detector3OutputOffsetMin");
                OnPropertyChanged("Detector3OutputOffsetMax");
                OnPropertyChanged("Detector4OutputOffsetMin");
                OnPropertyChanged("Detector4OutputOffsetMax");
                OnPropertyChanged("Detector5OutputOffsetMin");
                OnPropertyChanged("Detector5OutputOffsetMax");
                OnPropertyChanged("Detector6OutputOffsetMin");
                OnPropertyChanged("Detector6OutputOffsetMax");

                double gain = 0;
                ThorDetectorFunctions.GetParam(Convert.ToInt32(DeviceParams.PARAM_PMT1_GAIN_POS), ref gain);
                Detector1Gain = (int)gain;
                gain = 0;
                ThorDetectorFunctions.GetParam(Convert.ToInt32(DeviceParams.PARAM_PMT2_GAIN_POS), ref gain);
                Detector2Gain = (int)gain;
                gain = 0;
                ThorDetectorFunctions.GetParam(Convert.ToInt32(DeviceParams.PARAM_PMT3_GAIN_POS), ref gain);
                Detector3Gain = (int)gain;
                gain = 0;
                ThorDetectorFunctions.GetParam(Convert.ToInt32(DeviceParams.PARAM_PMT4_GAIN_POS), ref gain);
                Detector4Gain = (int)gain;
                gain = 0;
                ThorDetectorFunctions.GetParam(Convert.ToInt32(DeviceParams.PARAM_PMT5_GAIN_POS), ref gain);
                Detector5Gain = (int)gain;
                gain = 0;
                ThorDetectorFunctions.GetParam(Convert.ToInt32(DeviceParams.PARAM_PMT6_GAIN_POS), ref gain);
                Detector6Gain = (int)gain;

                Detector1On = FALSE;
                Detector2On = FALSE;
                Detector3On = FALSE;
                Detector4On = FALSE;
                Detector5On = FALSE;
                Detector6On = FALSE;

                OnPropertyChanged("Detector1OutputOffset");
                OnPropertyChanged("Detector2OutputOffset");
                OnPropertyChanged("Detector3OutputOffset");
                OnPropertyChanged("Detector4OutputOffset");
                OnPropertyChanged("Detector5OutputOffset");
                OnPropertyChanged("Detector6OutputOffset");

                OnPropertyChanged("Detector1Bandwidth");
                OnPropertyChanged("Detector2Bandwidth");
                OnPropertyChanged("Detector3Bandwidth");
                OnPropertyChanged("Detector4Bandwidth");
                OnPropertyChanged("Detector5Bandwidth");
                OnPropertyChanged("Detector6Bandwidth");

                if (true == _detector1Connected)
                {
                    double pos = 1;
                    GetParameter(Convert.ToInt32(DeviceParams.PARAM_PMT1_SAFETY), ref pos);
                    _detector1Tripped = (int)pos;
                    OnPropertyChanged("Detector1Tripped");
                    OnPropertyChanged("TrippedText1");
                    OnPropertyChanged("TrippedColor1");
                    OnPropertyChanged("TrippedVis1");
                    OnPropertyChanged("Detector1SerialNumber");
                    OnPropertyChanged("Detector1DetectorType");
                }
                if (true == _detector2Connected)
                {
                    double pos = 1;
                    GetParameter(Convert.ToInt32(DeviceParams.PARAM_PMT2_SAFETY), ref pos);
                    _detector2Tripped = (int)pos;
                    OnPropertyChanged("Detector2Tripped");
                    OnPropertyChanged("TrippedText2");
                    OnPropertyChanged("TrippedColor2");
                    OnPropertyChanged("TrippedVis2");
                    OnPropertyChanged("Detector2SerialNumber");
                    OnPropertyChanged("Detector2DetectorType");
                }
                if (true == _detector3Connected)
                {
                    double pos = 1;
                    GetParameter(Convert.ToInt32(DeviceParams.PARAM_PMT3_SAFETY), ref pos);
                    _detector3Tripped = (int)pos;
                    OnPropertyChanged("Detector3Tripped");
                    OnPropertyChanged("TrippedText3");
                    OnPropertyChanged("TrippedColor3");
                    OnPropertyChanged("TrippedVis3");
                    OnPropertyChanged("Detector3SerialNumber");
                    OnPropertyChanged("Detector3DetectorType");
                }
                if (true == _detector4Connected)
                {
                    double pos = 1;
                    GetParameter(Convert.ToInt32(DeviceParams.PARAM_PMT4_SAFETY), ref pos);
                    _detector4Tripped = (int)pos;
                    OnPropertyChanged("Detector4Tripped");
                    OnPropertyChanged("TrippedText4");
                    OnPropertyChanged("TrippedColor4");
                    OnPropertyChanged("TrippedVis4");
                    OnPropertyChanged("Detector4SerialNumber");
                    OnPropertyChanged("Detector4DetectorType");
                }
                if (true == _detector5Connected)
                {
                    double pos = 1;
                    GetParameter(Convert.ToInt32(DeviceParams.PARAM_PMT5_SAFETY), ref pos);
                    _detector5Tripped = (int)pos;
                    OnPropertyChanged("Detector5Tripped");
                    OnPropertyChanged("TrippedText5");
                    OnPropertyChanged("TrippedColor5");
                    OnPropertyChanged("TrippedVis5");
                    OnPropertyChanged("Detector5SerialNumber");
                    OnPropertyChanged("Detector5DetectorType");
                }
                if (true == _detector6Connected)
                {
                    double pos = 1;
                    GetParameter(Convert.ToInt32(DeviceParams.PARAM_PMT6_SAFETY), ref pos);
                    _detector6Tripped = (int)pos;
                    OnPropertyChanged("Detector6Tripped");
                    OnPropertyChanged("TrippedText6");
                    OnPropertyChanged("TrippedColor6");
                    OnPropertyChanged("TrippedVis6");
                    OnPropertyChanged("Detector6SerialNumber");
                    OnPropertyChanged("Detector6DetectorType");
                }

                timer.Tick += new EventHandler(timer_Tick);
                timer.Interval = new TimeSpan(0, 0, 0, 0, 500);
                timer.Start();
            }
            catch (DllNotFoundException ex)
            {
                MessageBox.Show(string.Format("The dll {0} was not found! ({1})!!!!!!!!!!!!!!", ThorDetectorFunctions.DLL_NAME, ex.Message));
            }
            catch (SEHException ex)
            {
                MessageBox.Show(string.Format("The device talking to {0} was not found! ({1})!!!!!!!!!!!!!!", ThorDetectorFunctions.DLL_NAME, ex.Message), "Device Not Found");
            }
        }

        /// <summary>
        /// Initializes the connections and views.
        /// </summary>
        /// <returns><c>true</c> if XXXX, <c>false</c> otherwise.</returns>
        bool InitializeConnectionsAndViews()
        {
            _initialized = false;
            int devices = 0;
            _devices = 0;
            //find the devices
            if (FALSE == ThorDetectorFunctions.FindDevices(ref devices))
            {
                devices = 0;
            }

            if (devices == 0)
            {
                Detector1.Visibility = Visibility.Collapsed;
                Detector2.Visibility = Visibility.Collapsed;
                Detector3.Visibility = Visibility.Collapsed;
                Detector4.Visibility = Visibility.Collapsed;
                Detector5.Visibility = Visibility.Collapsed;
                Detector6.Visibility = Visibility.Collapsed;
                _detector1Connected = false;
                _detector2Connected = false;
                _detector3Connected = false;
                _detector4Connected = false;
                _detector5Connected = false;
                _detector6Connected = false;
                MessageBox.Show("No device found!");
                return false;
            }

            //Select the devices
            if (FALSE == ThorDetectorFunctions.SelectDevice(0))
            {
                Detector1.Visibility = Visibility.Collapsed;
                Detector2.Visibility = Visibility.Collapsed;
                Detector3.Visibility = Visibility.Collapsed;
                Detector4.Visibility = Visibility.Collapsed;
                Detector5.Visibility = Visibility.Collapsed;
                Detector6.Visibility = Visibility.Collapsed;
                MessageBox.Show("Unable to establish connection!");
                return false;
            }
            _initialized = true;

            //Gets the bytes of the connected devices
            double nDevices = 0;
            GetParameter((int)DeviceParams.PARAM_CONNECTED_PMTS, ref nDevices);
            _devices = (int)nDevices;

            uint detector1Active = (uint)_devices & (uint)DeviceType.PMT1;
            _detector1Connected = (0 != detector1Active) ? true : false;
            //Update the bandwidth combobox based on the device connected (fill with placeholder if device is not connected)
            int index = 0;
            if (_detector1Connected)
            {
                GenerateBandwidthList((int)DeviceParams.PARAM_PMT1_BANDWIDTH_POS, (int)DeviceParams.PARAM_PMT1_BANDWIDTH_POS_CURRENT, index);
            }
            else
            {
                BandwidthList.Add(new ObservableCollection<string>());
            }
            uint detector2Active = (uint)_devices & (uint)DeviceType.PMT2;
            _detector2Connected = (0 != detector2Active) ? true : false;
            if (_detector2Connected)
            {
                index = 1;
                GenerateBandwidthList((int)DeviceParams.PARAM_PMT2_BANDWIDTH_POS, (int)DeviceParams.PARAM_PMT2_BANDWIDTH_POS_CURRENT, index);
            }
            else
            {
                BandwidthList.Add(new ObservableCollection<string>());
            }
            uint detector3Active = (uint)_devices & (uint)DeviceType.PMT3;
            _detector3Connected = (0 != detector3Active) ? true : false;
            if (_detector3Connected)
            {
                index = 2;
                GenerateBandwidthList((int)DeviceParams.PARAM_PMT3_BANDWIDTH_POS, (int)DeviceParams.PARAM_PMT3_BANDWIDTH_POS_CURRENT, index);
            }
            else
            {
                BandwidthList.Add(new ObservableCollection<string>());
            }
            uint detector4Active = (uint)_devices & (uint)DeviceType.PMT4;
            _detector4Connected = (0 != detector4Active) ? true : false;
            if (_detector4Connected)
            {
                index = 3;
                GenerateBandwidthList((int)DeviceParams.PARAM_PMT4_BANDWIDTH_POS, (int)DeviceParams.PARAM_PMT4_BANDWIDTH_POS_CURRENT, index);
            }
            else
            {
                BandwidthList.Add(new ObservableCollection<string>());
            }
            uint detector5Active = (uint)_devices & (uint)DeviceType.PMT5;
            _detector5Connected = (0 != detector5Active) ? true : false;
            if (_detector5Connected)
            {
                index = 4;
                GenerateBandwidthList((int)DeviceParams.PARAM_PMT5_BANDWIDTH_POS, (int)DeviceParams.PARAM_PMT5_BANDWIDTH_POS_CURRENT, index);
            }
            else
            {
                BandwidthList.Add(new ObservableCollection<string>());
            }
            uint detector6Active = (uint)_devices & (uint)DeviceType.PMT6;
            _detector6Connected = (0 != detector6Active) ? true : false;
            if (_detector6Connected)
            {
                index = 5;
                GenerateBandwidthList((int)DeviceParams.PARAM_PMT6_BANDWIDTH_POS, (int)DeviceParams.PARAM_PMT6_BANDWIDTH_POS_CURRENT, index);
            }
            else
            {
                BandwidthList.Add(new ObservableCollection<string>());
            }

            Detector1.Visibility = (0 != detector1Active) ? Visibility.Visible : Visibility.Collapsed;
            Detector2.Visibility = (0 != detector2Active) ? Visibility.Visible : Visibility.Collapsed;
            Detector3.Visibility = (0 != detector3Active) ? Visibility.Visible : Visibility.Collapsed;
            Detector4.Visibility = (0 != detector4Active) ? Visibility.Visible : Visibility.Collapsed;
            Detector5.Visibility = (0 != detector5Active) ? Visibility.Visible : Visibility.Collapsed;
            Detector6.Visibility = (0 != detector6Active) ? Visibility.Visible : Visibility.Collapsed;

            return true;
        }

        /// <summary>
        /// Called when [property changed].
        /// </summary>
        /// <param name="propertyName">Name of the property.</param>
        private void OnPropertyChanged(string propertyName)
        {
            if (PropertyChanged != null)
            {
                this.PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }

        /// <summary>
        /// Handles the Click event of the refresh control.
        /// Here the connections and views get refreshed
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void refresh_Click(object sender, RoutedEventArgs e)
        {
            ThorDetectorFunctions.TeardownDevice();
            System.Threading.Thread.Sleep(100);
            Initialize();
        }

        /// <summary>
        /// assign the attribute value to the input node and document
        /// if the attribute does not exist add it to the document
        /// </summary>
        /// <param name="node">The node.</param>
        /// <param name="doc">The document.</param>
        /// <param name="attrName">Name of the attribute.</param>
        /// <param name="attValue">The att value.</param>
        private void SetAttribute(XmlNode node, XmlDocument doc, string attrName, string attValue)
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

        /// <summary>
        /// Sets the Detector to enabled or disabled
        /// </summary>
        private void SetDetector()
        {
            if (_detectorsActive)
            {
                if (0 < _detector1gain && TRUE == _detector1On)
                {
                    ThorDetectorFunctions.SetParam(Convert.ToInt32(DeviceParams.PARAM_PMT1_ENABLE), _detector1On);
                }
                if (0 < _detector2gain && TRUE == _detector2On)
                {
                    ThorDetectorFunctions.SetParam(Convert.ToInt32(DeviceParams.PARAM_PMT2_ENABLE), _detector2On);
                }
                if (0 < _detector3gain && TRUE == _detector3On)
                {
                    ThorDetectorFunctions.SetParam(Convert.ToInt32(DeviceParams.PARAM_PMT3_ENABLE), _detector3On);
                }
                if (0 < _detector4gain && TRUE == _detector4On)
                {
                    ThorDetectorFunctions.SetParam(Convert.ToInt32(DeviceParams.PARAM_PMT4_ENABLE), _detector4On);
                }
                if (0 < _detector5gain && TRUE == _detector5On)
                {
                    ThorDetectorFunctions.SetParam(Convert.ToInt32(DeviceParams.PARAM_PMT5_ENABLE), _detector5On);
                }
                if (0 < _detector6gain && TRUE == _detector6On)
                {
                    ThorDetectorFunctions.SetParam(Convert.ToInt32(DeviceParams.PARAM_PMT6_ENABLE), _detector6On);
                }
            }
            else
            {
                ThorDetectorFunctions.SetParam(Convert.ToInt32(DeviceParams.PARAM_PMT1_ENABLE), FALSE);
                ThorDetectorFunctions.SetParam(Convert.ToInt32(DeviceParams.PARAM_PMT2_ENABLE), FALSE);
                ThorDetectorFunctions.SetParam(Convert.ToInt32(DeviceParams.PARAM_PMT3_ENABLE), FALSE);
                ThorDetectorFunctions.SetParam(Convert.ToInt32(DeviceParams.PARAM_PMT4_ENABLE), FALSE);
                ThorDetectorFunctions.SetParam(Convert.ToInt32(DeviceParams.PARAM_PMT5_ENABLE), FALSE);
                ThorDetectorFunctions.SetParam(Convert.ToInt32(DeviceParams.PARAM_PMT6_ENABLE), FALSE);
            }
            ThorDetectorFunctions.PreflightPosition();
            ThorDetectorFunctions.SetupPosition();
            ThorDetectorFunctions.StartPosition();
            ThorDetectorFunctions.PostflightPosition();
        }

        /// <summary>
        /// Handles the Click event of the StartDetectors control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void StartDetectors_Click(object sender, RoutedEventArgs e)
        {
            _detectorsActive = !_detectorsActive;
            SetDetector();
            OnPropertyChanged("ImagePathPlay");
        }

        /// <summary>
        /// Desables all detectors and tears down the devices
        /// </summary>
        private void TearDown()
        {
            //Turn Detectors off
            ThorDetectorFunctions.SetParam(Convert.ToInt32(DeviceParams.PARAM_PMT1_ENABLE), 0);
            ThorDetectorFunctions.SetParam(Convert.ToInt32(DeviceParams.PARAM_PMT2_ENABLE), 0);
            ThorDetectorFunctions.SetParam(Convert.ToInt32(DeviceParams.PARAM_PMT3_ENABLE), 0);
            ThorDetectorFunctions.SetParam(Convert.ToInt32(DeviceParams.PARAM_PMT4_ENABLE), 0);
            ThorDetectorFunctions.SetParam(Convert.ToInt32(DeviceParams.PARAM_PMT5_ENABLE), 0);
            ThorDetectorFunctions.SetParam(Convert.ToInt32(DeviceParams.PARAM_PMT6_ENABLE), 0);

            ThorDetectorFunctions.PreflightPosition();
            ThorDetectorFunctions.SetupPosition();
            ThorDetectorFunctions.StartPosition();
            ThorDetectorFunctions.PostflightPosition();
            if (null != timer) timer.Stop();
            ThorDetectorFunctions.TeardownDevice();
        }

        /// <summary>
        /// Handles the Tick event of the timer control.
        /// Here we call OnPropertyChanged to update the view of the desired properties
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="EventArgs"/> instance containing the event data.</param>
        void timer_Tick(object sender, EventArgs e)
        {
            if (true == _detector1Connected)
            {
                double pos = 1;
                GetParameter(Convert.ToInt32(DeviceParams.PARAM_PMT1_SAFETY), ref pos);
                _detector1Tripped = (int)pos;
                OnPropertyChanged("Detector1Tripped");
                OnPropertyChanged("TrippedText1");
                OnPropertyChanged("TrippedColor1");
                OnPropertyChanged("TrippedVis1");
            }
            if (true == _detector2Connected)
            {
                double pos = 1;
                GetParameter(Convert.ToInt32(DeviceParams.PARAM_PMT2_SAFETY), ref pos);
                _detector2Tripped = (int)pos;
                OnPropertyChanged("Detector2Tripped");
                OnPropertyChanged("TrippedText2");
                OnPropertyChanged("TrippedColor2");
                OnPropertyChanged("TrippedVis2");
            }
            if (true == _detector3Connected)
            {
                double pos = 1;
                GetParameter(Convert.ToInt32(DeviceParams.PARAM_PMT3_SAFETY), ref pos);
                _detector3Tripped = (int)pos;
                OnPropertyChanged("Detector3Tripped");
                OnPropertyChanged("TrippedText3");
                OnPropertyChanged("TrippedColor3");
                OnPropertyChanged("TrippedVis3");
            }
            if (true == _detector4Connected)
            {
                double pos = 1;
                GetParameter(Convert.ToInt32(DeviceParams.PARAM_PMT4_SAFETY), ref pos);
                _detector4Tripped = (int)pos;
                OnPropertyChanged("Detector4Tripped");
                OnPropertyChanged("TrippedText4");
                OnPropertyChanged("TrippedColor4");
                OnPropertyChanged("TrippedVis4");
            }
            if (true == _detector5Connected)
            {
                double pos = 1;
                GetParameter(Convert.ToInt32(DeviceParams.PARAM_PMT5_SAFETY), ref pos);
                _detector5Tripped = (int)pos;
                OnPropertyChanged("Detector5Tripped");
                OnPropertyChanged("TrippedText5");
                OnPropertyChanged("TrippedColor5");
                OnPropertyChanged("TrippedVis5");
            }
            if (true == _detector6Connected)
            {
                double pos = 1;
                GetParameter(Convert.ToInt32(DeviceParams.PARAM_PMT6_SAFETY), ref pos);
                _detector6Tripped = (int)pos;
                OnPropertyChanged("Detector6Tripped");
                OnPropertyChanged("TrippedText6");
                OnPropertyChanged("TrippedColor6");
                OnPropertyChanged("TrippedVis6");
            }
        }

        private bool UpdateDetectorFirmware(int devIndex, double firmwareVersion, string detectorType)
        {
            if (_devices != 0)
            {
                string para = "Save" + devIndex;
                ThorDetectorFunctions.SetParamString(Convert.ToInt32(DeviceParams.PARAM_CONNECTED_PMTS), para);

                //Variables that might be necessary for checking specific cases when updating device firmware
                COMPortManager.type = string.Empty;
                COMPortManager.firmwareNum = string.Empty;
                COMPortManager.bootloaderNum = string.Empty;
            }
            string comPort = string.Empty;
            string fileName = string.Empty;
            string firmwareFile = string.Empty;
            Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();
            dlg.Title = "Select .hex firmware file using this Dialog Box";
            dlg.DefaultExt = ".hex";
            dlg.Filter = "HEX Files|*.hex";
            dlg.AddExtension = true;
            dlg.CheckFileExists = true;
            dlg.CheckPathExists = true;
            bool result = (dlg.ShowDialog() == true) ? true : false;
            if (true == result)
            {
                firmwareFile = dlg.FileName.ToString();
                fileName = dlg.SafeFileName.ToString();
                MessageBoxResult messageResult = MessageBox.Show("Are you sure " + fileName + " is the correct Firmware file for this device?", "Confirmation", MessageBoxButton.YesNo);
                Mouse.OverrideCursor = Cursors.Wait;
                if (MessageBoxResult.No == messageResult || MessageBoxResult.Cancel == messageResult || MessageBoxResult.None == messageResult)
                {
                    Mouse.OverrideCursor = null;
                    return false;
                }
                /*if (Convert.ToString(COMPortManager.firmwareNum) != null)
                {

                    string jumpFile = System.IO.Path.GetDirectoryName(dlg.FileName) + @"\JUMP.hex";
                    if (File.Exists(jumpFile))
                    {
                    }
                    // If FW < 2.x and Jump.hex File does not exist.
                    else
                    {
                        MessageBox.Show("Jump.hex File required but not found.  Please Contact Customer Service.  Update Cancelled!", "Firmware File Check");
                        return false;
                    }
                }*/
                ExecutePosition((int)DeviceParams.PARAM_PMT1_FIRMWAREUPDATE + devIndex - 1, 1);

                //TearDown();
                COMPortManager.comPorts.Clear();
                // configure com port for firmware loading
                ComPortWindow CompWin = new ComPortWindow();
                CompWin.Title = "Thorlabs Update Com Port Selection";
                CompWin.detectorLabel.Text = "Detector" + devIndex + " COM Port:";
                CompWin.ShowDialog();
                if (true == CompWin.DialogResult)
                {
                    if (CompWin.cbDCOM.SelectedIndex > 0)
                        COMPortManager.comPorts.Add(new ComPort(CompWin.cbDCOM.SelectedValue.ToString(), CompWin.cbDCOM.SelectedValue.ToString()));

                    //Allow some time for the devices to load as comports
                    System.Threading.Thread.Sleep(3500);
                    System.IO.Ports.SerialPort port = new System.IO.Ports.SerialPort(CompWin.cbDCOM.SelectedValue.ToString(), 115200);
                    comPort = CompWin.cbDCOM.SelectedValue.ToString();
                    try
                    {
                        port.Open();

                        System.Threading.Thread.Sleep(1500);

                        port.Dispose();
                        comPort = CompWin.cbDCOM.SelectedValue.ToString();
                        UpdateFirmwareWindow.UpdateFirmware updateFirmwareWindow = new UpdateFirmwareWindow.UpdateFirmware(comPort, firmwareFile, fileName);
                        try
                        {
                            if (true == updateFirmwareWindow.ShowDialog())
                            {
                                System.Threading.Thread.Sleep(3500);
                                Initialize();
                            }
                            else
                            {
                                MessageBox.Show("No device detected. Please make sure the device is connected and try again.");
                                return false;
                            }
                        }
                        catch (Exception ex)
                        {
                            MessageBox.Show("Firmware update was unsuccessful");
                            ex.ToString();
                        }
                    }
                    catch (Exception ex)
                    {
                        MessageBox.Show("Com Port is not valid.  Please select correct Com Port and try again.");
                        ex.ToString();
                        return false;
                    }
                }
                // User closes window without selecting Com port
                else
                {
                    return false;
                }
            }
            return result;
        }

        /// <summary>
        /// Handles the Closing event of the Window1 control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="CancelEventArgs"/> instance containing the event data.</param>
        void Window1_Closing(object sender, CancelEventArgs e)
        {
            Application.Current.Shutdown();
        }

        /// <summary>
        /// Handles the KeyDown event of the Window1 control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="KeyEventArgs"/> instance containing the event data.</param>
        void Window1_KeyDown(object sender, KeyEventArgs e)
        {
            //filter for the enter or return key
            if ((e.Key == Key.Enter) || (e.Key == Key.Return))
            {
                e.Handled = true;
                TraversalRequest trNext = new TraversalRequest(FocusNavigationDirection.Next);

                UIElement keyboardFocus = (UIElement)Keyboard.FocusedElement;

                //move the focus to the next element
                if (keyboardFocus != null)
                {
                    if (keyboardFocus.GetType() == typeof(TextBox))
                    {
                        keyboardFocus.MoveFocus(trNext);
                    }
                }
            }
        }

        /// <summary>
        /// Handles the Loaded event of the Window1 control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        void Window1_Loaded(object sender, RoutedEventArgs e)
        {
            Initialize();
        }

        #endregion Methods
    }
}