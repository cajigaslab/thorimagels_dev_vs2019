namespace DigitalOutputSwitches
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Navigation;
    using System.Windows.Shapes;
    using System.Xml;

    using ThorSharedTypes;

    /// <summary>
    /// Interaction logic for UserControl1.xaml
    /// </summary>
    public partial class DigitalOutputSwitchesControl : UserControl
    {
        #region Fields

        public static readonly DependencyProperty EnableCommandProperty =
            DependencyProperty.Register(
            "EnableCommand",
            typeof(ICommand),
            typeof(DigitalOutputSwitchesControl));
        public static readonly DependencyProperty ExperimentModeProperty =
            DependencyProperty.Register(
            "ExperimentMode",
            typeof(int),
            typeof(DigitalOutputSwitchesControl));
        public static readonly DependencyProperty PowerPercentStringProperty =
            DependencyProperty.Register(
            "PowerPercentString",
            typeof(string),
            typeof(DigitalOutputSwitchesControl));
        public static readonly DependencyProperty Switch0CommandProperty =
            DependencyProperty.Register(
            "Switch0Command",
            typeof(ICommand),
            typeof(DigitalOutputSwitchesControl));
        public static readonly DependencyProperty Switch1CommandProperty =
            DependencyProperty.Register(
            "Switch1Command",
            typeof(ICommand),
            typeof(DigitalOutputSwitchesControl));
        public static readonly DependencyProperty Switch2CommandProperty =
            DependencyProperty.Register(
            "Switch2Command",
            typeof(ICommand),
            typeof(DigitalOutputSwitchesControl));
        public static readonly DependencyProperty Switch3CommandProperty =
            DependencyProperty.Register(
            "Switch3Command",
            typeof(ICommand),
            typeof(DigitalOutputSwitchesControl));
        public static readonly DependencyProperty Switch4CommandProperty =
            DependencyProperty.Register(
            "Switch4Command",
            typeof(ICommand),
            typeof(DigitalOutputSwitchesControl));
        public static readonly DependencyProperty Switch5CommandProperty =
            DependencyProperty.Register(
            "Switch5Command",
            typeof(ICommand),
            typeof(DigitalOutputSwitchesControl));
        public static readonly DependencyProperty Switch6CommandProperty =
            DependencyProperty.Register(
            "Switch6Command",
            typeof(ICommand),
            typeof(DigitalOutputSwitchesControl));
        public static readonly DependencyProperty Switch7CommandProperty =
            DependencyProperty.Register(
            "Switch7Command",
            typeof(ICommand),
            typeof(DigitalOutputSwitchesControl));
        public static readonly DependencyProperty SwitchesIsVisibleProperty =
            DependencyProperty.Register(
            "SwitchesIsVisible",
            typeof(bool),
            typeof(DigitalOutputSwitchesControl));
        public static readonly DependencyProperty TriggerDurationMSProperty =
            DependencyProperty.Register(
            "TriggerDurationMS",
            typeof(double),
            typeof(DigitalOutputSwitchesControl));
        public static readonly DependencyProperty TriggerEdgeStringProperty =
            DependencyProperty.Register(
            "TriggerEdgeString",
            typeof(string),
            typeof(DigitalOutputSwitchesControl));
        public static readonly DependencyProperty TriggerEnableCommandProperty =
            DependencyProperty.Register(
            "TriggerEnableCommand",
            typeof(ICommand),
            typeof(DigitalOutputSwitchesControl));
        public static readonly DependencyProperty TriggerEnableProperty =
            DependencyProperty.Register(
            "TriggerEnable",
            typeof(int),
            typeof(DigitalOutputSwitchesControl));
        public static readonly DependencyProperty TriggerErrorColorProperty =
            DependencyProperty.Register(
            "TriggerErrorColor",
            typeof(string),
            typeof(DigitalOutputSwitchesControl));
        public static readonly DependencyProperty TriggerErrorProperty =
            DependencyProperty.Register(
            "TriggerError",
            typeof(string),
            typeof(DigitalOutputSwitchesControl));
        public static readonly DependencyProperty TriggerIdleMSProperty =
            DependencyProperty.Register(
            "TriggerIdleMS",
            typeof(double),
            typeof(DigitalOutputSwitchesControl));
        public static readonly DependencyProperty TriggerImagePathProperty =
            DependencyProperty.Register(
            "TriggerImagePath",
            typeof(string),
            typeof(DigitalOutputSwitchesControl));
        public static readonly DependencyProperty TriggerIsVisibleProperty =
            DependencyProperty.Register(
            "TriggerIsVisible",
            typeof(bool),
            typeof(DigitalOutputSwitchesControl));
        public static readonly DependencyProperty TriggerIterationsProperty =
            DependencyProperty.Register(
            "TriggerIterations",
            typeof(int),
            typeof(DigitalOutputSwitchesControl));
        public static readonly DependencyProperty TriggerLabelProperty =
            DependencyProperty.Register(
            "TriggerLabel",
            typeof(string),
            typeof(DigitalOutputSwitchesControl));
        public static readonly DependencyProperty TriggerModeItemsProperty =
            DependencyProperty.Register(
            "TriggerModeItems",
            typeof(ObservableCollection<StringPC>),
            typeof(DigitalOutputSwitchesControl));
        public static readonly DependencyProperty TriggerModeProperty =
            DependencyProperty.Register(
            "TriggerMode",
            typeof(int),
            typeof(DigitalOutputSwitchesControl));
        public static readonly DependencyProperty TriggerRepeatProperty =
            DependencyProperty.Register(
            "TriggerRepeat",
            typeof(int),
            typeof(DigitalOutputSwitchesControl));
        public static readonly DependencyProperty TriggerStartEdgeProperty =
            DependencyProperty.Register(
            "TriggerStartEdge",
            typeof(int),
            typeof(DigitalOutputSwitchesControl));
        public static readonly DependencyProperty TriggerStartIdleMSProperty =
            DependencyProperty.Register(
            "TriggerStartIdleMS",
            typeof(double),
            typeof(DigitalOutputSwitchesControl));
        public static readonly DependencyProperty TriggerTypeItemsProperty =
            DependencyProperty.Register(
            "TriggerTypeItems",
            typeof(ObservableCollection<StringPC>),
            typeof(DigitalOutputSwitchesControl));
        public static readonly DependencyProperty TriggerTypeProperty =
           DependencyProperty.Register(
           "TriggerType",
           typeof(int),
           typeof(DigitalOutputSwitchesControl));

        public static DependencyProperty SwitchEnableProperty =
           DependencyProperty.RegisterAttached("SwitchEnable",
           typeof(int),
           typeof(DigitalOutputSwitchesControl),
           new FrameworkPropertyMetadata(new PropertyChangedCallback(onSwitchEnableChanged)));
        public static DependencyProperty SwitchName0Property =
           DependencyProperty.RegisterAttached("SwitchName0",
           typeof(string),
           typeof(DigitalOutputSwitchesControl),
           new FrameworkPropertyMetadata(new PropertyChangedCallback(onSwitchNameChanged0)));
        public static DependencyProperty SwitchName1Property =
           DependencyProperty.RegisterAttached("SwitchName1",
           typeof(string),
           typeof(DigitalOutputSwitchesControl),
           new FrameworkPropertyMetadata(new PropertyChangedCallback(onSwitchNameChanged1)));
        public static DependencyProperty SwitchName2Property =
           DependencyProperty.RegisterAttached("SwitchName2",
           typeof(string),
           typeof(DigitalOutputSwitchesControl),
           new FrameworkPropertyMetadata(new PropertyChangedCallback(onSwitchNameChanged2)));
        public static DependencyProperty SwitchName3Property =
           DependencyProperty.RegisterAttached("SwitchName3",
           typeof(string),
           typeof(DigitalOutputSwitchesControl),
           new FrameworkPropertyMetadata(new PropertyChangedCallback(onSwitchNameChanged3)));
        public static DependencyProperty SwitchName4Property =
           DependencyProperty.RegisterAttached("SwitchName4",
           typeof(string),
           typeof(DigitalOutputSwitchesControl),
           new FrameworkPropertyMetadata(new PropertyChangedCallback(onSwitchNameChanged4)));
        public static DependencyProperty SwitchName5Property =
           DependencyProperty.RegisterAttached("SwitchName5",
           typeof(string),
           typeof(DigitalOutputSwitchesControl),
           new FrameworkPropertyMetadata(new PropertyChangedCallback(onSwitchNameChanged5)));
        public static DependencyProperty SwitchName6Property =
           DependencyProperty.RegisterAttached("SwitchName6",
           typeof(string),
           typeof(DigitalOutputSwitchesControl),
           new FrameworkPropertyMetadata(new PropertyChangedCallback(onSwitchNameChanged6)));
        public static DependencyProperty SwitchName7Property =
           DependencyProperty.RegisterAttached("SwitchName7",
           typeof(string),
           typeof(DigitalOutputSwitchesControl),
           new FrameworkPropertyMetadata(new PropertyChangedCallback(onSwitchNameChanged7)));
        public static DependencyProperty SwitchState0Property =
           DependencyProperty.RegisterAttached("SwitchState0",
           typeof(int),
           typeof(DigitalOutputSwitchesControl),
           new FrameworkPropertyMetadata(new PropertyChangedCallback(onSwitchStateChanged0)));
        public static DependencyProperty SwitchState1Property =
           DependencyProperty.RegisterAttached("SwitchState1",
           typeof(int),
           typeof(DigitalOutputSwitchesControl),
           new FrameworkPropertyMetadata(new PropertyChangedCallback(onSwitchStateChanged1)));
        public static DependencyProperty SwitchState2Property =
           DependencyProperty.RegisterAttached("SwitchState2",
           typeof(int),
           typeof(DigitalOutputSwitchesControl),
           new FrameworkPropertyMetadata(new PropertyChangedCallback(onSwitchStateChanged2)));
        public static DependencyProperty SwitchState3Property =
           DependencyProperty.RegisterAttached("SwitchState3",
           typeof(int),
           typeof(DigitalOutputSwitchesControl),
           new FrameworkPropertyMetadata(new PropertyChangedCallback(onSwitchStateChanged3)));
        public static DependencyProperty SwitchState4Property =
           DependencyProperty.RegisterAttached("SwitchState4",
           typeof(int),
           typeof(DigitalOutputSwitchesControl),
           new FrameworkPropertyMetadata(new PropertyChangedCallback(onSwitchStateChanged4)));
        public static DependencyProperty SwitchState5Property =
           DependencyProperty.RegisterAttached("SwitchState5",
           typeof(int),
           typeof(DigitalOutputSwitchesControl),
           new FrameworkPropertyMetadata(new PropertyChangedCallback(onSwitchStateChanged5)));
        public static DependencyProperty SwitchState6Property =
           DependencyProperty.RegisterAttached("SwitchState6",
           typeof(int),
           typeof(DigitalOutputSwitchesControl),
           new FrameworkPropertyMetadata(new PropertyChangedCallback(onSwitchStateChanged6)));
        public static DependencyProperty SwitchState7Property =
           DependencyProperty.RegisterAttached("SwitchState7",
           typeof(int),
           typeof(DigitalOutputSwitchesControl),
           new FrameworkPropertyMetadata(new PropertyChangedCallback(onSwitchStateChanged7)));

        private XmlDocument _commandsDocument = null;

        #endregion Fields

        #region Constructors

        public DigitalOutputSwitchesControl()
        {
            InitializeComponent();
            LayoutRoot.DataContext = this;
        }

        #endregion Constructors

        #region Properties

        public ICommand EnableCommand
        {
            get { return (ICommand)GetValue(EnableCommandProperty); }
            set { SetValue(EnableCommandProperty, value); }
        }

        public int ExperimentMode
        {
            get { return (int)GetValue(ExperimentModeProperty); }
            set { SetValue(ExperimentModeProperty, value); }
        }

        public string PowerPercentString
        {
            get { return (string)GetValue(PowerPercentStringProperty); }
            set { SetValue(PowerPercentStringProperty, value); }
        }

        //This Property is only used in the Script panel. Not in Capture Setup
        public XmlDocument SettingsDocument
        {
            get
            {
                return _commandsDocument;
            }
            set
            {
                _commandsDocument = value;

                //extract the path to the experiment file
                //and the path for the output
                if (null != _commandsDocument)
                {
                    XmlNode node = _commandsDocument.SelectSingleNode("Command/DigiSwitch");

                    if (null != node)
                    {
                        //only digital switches is configurable, digital trigger starts with capture
                        SwitchesIsVisible = true;
                        TriggerIsVisible = false;

                        SwitchName0 = ((ObservableCollection<StringPC>)MVMManager.Instance["DigitalOutputSwitchesViewModel", "SwitchName"])[0].Value;
                        if (SwitchName0 != null && SwitchName0.CompareTo("") == 0) SwitchName0 = "1";
                        SwitchName1 = ((ObservableCollection<StringPC>)MVMManager.Instance["DigitalOutputSwitchesViewModel", "SwitchName"])[1].Value;
                        if (SwitchName1 != null && SwitchName1.CompareTo("") == 0) SwitchName1 = "2";
                        SwitchName2 = ((ObservableCollection<StringPC>)MVMManager.Instance["DigitalOutputSwitchesViewModel", "SwitchName"])[2].Value;
                        if (SwitchName2 != null && SwitchName2.CompareTo("") == 0) SwitchName2 = "3";
                        SwitchName3 = ((ObservableCollection<StringPC>)MVMManager.Instance["DigitalOutputSwitchesViewModel", "SwitchName"])[3].Value;
                        if (SwitchName3 != null && SwitchName3.CompareTo("") == 0) SwitchName3 = "4";
                        SwitchName4 = ((ObservableCollection<StringPC>)MVMManager.Instance["DigitalOutputSwitchesViewModel", "SwitchName"])[4].Value;
                        if (SwitchName4 != null && SwitchName4.CompareTo("") == 0) SwitchName4 = "5";
                        SwitchName5 = ((ObservableCollection<StringPC>)MVMManager.Instance["DigitalOutputSwitchesViewModel", "SwitchName"])[5].Value;
                        if (SwitchName5 != null && SwitchName5.CompareTo("") == 0) SwitchName5 = "6";
                        SwitchName6 = ((ObservableCollection<StringPC>)MVMManager.Instance["DigitalOutputSwitchesViewModel", "SwitchName"])[6].Value;
                        if (SwitchName6 != null && SwitchName6.CompareTo("") == 0) SwitchName6 = "7";
                        SwitchName7 = ((ObservableCollection<StringPC>)MVMManager.Instance["DigitalOutputSwitchesViewModel", "SwitchName"])[7].Value;
                        if (SwitchName7 != null && SwitchName7.CompareTo("") == 0) SwitchName7 = "8";

                        string str = string.Empty;

                        if (XmlManager.GetAttribute(node, _commandsDocument, "enabled", ref str))
                        {
                            SwitchEnable = Convert.ToInt32(str);
                        }

                        if (XmlManager.GetAttribute(node, _commandsDocument, "status", ref str))
                        {
                            SwitchState0 = Convert.ToInt32(str.Substring(0, 1));
                            SwitchState1 = Convert.ToInt32(str.Substring(1, 1));
                            SwitchState2 = Convert.ToInt32(str.Substring(2, 1));
                            SwitchState3 = Convert.ToInt32(str.Substring(3, 1));
                            SwitchState4 = Convert.ToInt32(str.Substring(4, 1));
                            SwitchState5 = Convert.ToInt32(str.Substring(5, 1));
                            SwitchState6 = Convert.ToInt32(str.Substring(6, 1));
                            SwitchState7 = Convert.ToInt32(str.Substring(7, 1));
                        }
                        EnableCommand =
                        Switch0Command =
                        Switch1Command =
                        Switch2Command =
                        Switch3Command =
                        Switch4Command =
                        Switch5Command =
                        Switch6Command =
                        Switch7Command = new RelayCommand(() => PersistXmlDoc());
                    }
                }
            }
        }

        public ICommand Switch0Command
        {
            get { return (ICommand)GetValue(Switch0CommandProperty); }
            set { SetValue(Switch0CommandProperty, value); }
        }

        public ICommand Switch1Command
        {
            get { return (ICommand)GetValue(Switch1CommandProperty); }
            set { SetValue(Switch1CommandProperty, value); }
        }

        public ICommand Switch2Command
        {
            get { return (ICommand)GetValue(Switch2CommandProperty); }
            set { SetValue(Switch2CommandProperty, value); }
        }

        public ICommand Switch3Command
        {
            get { return (ICommand)GetValue(Switch3CommandProperty); }
            set { SetValue(Switch3CommandProperty, value); }
        }

        public ICommand Switch4Command
        {
            get { return (ICommand)GetValue(Switch4CommandProperty); }
            set { SetValue(Switch4CommandProperty, value); }
        }

        public ICommand Switch5Command
        {
            get { return (ICommand)GetValue(Switch5CommandProperty); }
            set { SetValue(Switch5CommandProperty, value); }
        }

        public ICommand Switch6Command
        {
            get { return (ICommand)GetValue(Switch6CommandProperty); }
            set { SetValue(Switch6CommandProperty, value); }
        }

        public ICommand Switch7Command
        {
            get { return (ICommand)GetValue(Switch7CommandProperty); }
            set { SetValue(Switch7CommandProperty, value); }
        }

        public int SwitchEnable
        {
            get { return (int)GetValue(SwitchEnableProperty); }
            set { SetValue(SwitchEnableProperty, value); }
        }

        public bool SwitchesIsVisible
        {
            get { return (bool)GetValue(SwitchesIsVisibleProperty); }
            set { SetValue(SwitchesIsVisibleProperty, value); }
        }

        public string SwitchName0
        {
            get { return (string)GetValue(SwitchName0Property); }
            set { SetValue(SwitchName0Property, value); }
        }

        public string SwitchName1
        {
            get { return (string)GetValue(SwitchName1Property); }
            set { SetValue(SwitchName1Property, value); }
        }

        public string SwitchName2
        {
            get { return (string)GetValue(SwitchName2Property); }
            set { SetValue(SwitchName2Property, value); }
        }

        public string SwitchName3
        {
            get { return (string)GetValue(SwitchName3Property); }
            set { SetValue(SwitchName3Property, value); }
        }

        public string SwitchName4
        {
            get { return (string)GetValue(SwitchName4Property); }
            set { SetValue(SwitchName4Property, value); }
        }

        public string SwitchName5
        {
            get { return (string)GetValue(SwitchName5Property); }
            set { SetValue(SwitchName5Property, value); }
        }

        public string SwitchName6
        {
            get { return (string)GetValue(SwitchName6Property); }
            set { SetValue(SwitchName6Property, value); }
        }

        public string SwitchName7
        {
            get { return (string)GetValue(SwitchName7Property); }
            set { SetValue(SwitchName7Property, value); }
        }

        public int SwitchState0
        {
            get { return (int)GetValue(SwitchState0Property); }
            set { SetValue(SwitchState0Property, value); }
        }

        public int SwitchState1
        {
            get { return (int)GetValue(SwitchState1Property); }
            set { SetValue(SwitchState1Property, value); }
        }

        public int SwitchState2
        {
            get { return (int)GetValue(SwitchState2Property); }
            set { SetValue(SwitchState2Property, value); }
        }

        public int SwitchState3
        {
            get { return (int)GetValue(SwitchState3Property); }
            set { SetValue(SwitchState3Property, value); }
        }

        public int SwitchState4
        {
            get { return (int)GetValue(SwitchState4Property); }
            set { SetValue(SwitchState4Property, value); }
        }

        public int SwitchState5
        {
            get { return (int)GetValue(SwitchState5Property); }
            set { SetValue(SwitchState5Property, value); }
        }

        public int SwitchState6
        {
            get { return (int)GetValue(SwitchState6Property); }
            set { SetValue(SwitchState6Property, value); }
        }

        public int SwitchState7
        {
            get { return (int)GetValue(SwitchState7Property); }
            set { SetValue(SwitchState7Property, value); }
        }

        public double TriggerDurationMS
        {
            get { return (double)GetValue(TriggerDurationMSProperty); }
            set { SetValue(TriggerDurationMSProperty, value); }
        }

        public string TriggerEdgeString
        {
            get { return (string)GetValue(TriggerEdgeStringProperty); }
            set { SetValue(TriggerEdgeStringProperty, value); }
        }

        public int TriggerEnable
        {
            get { return (int)GetValue(TriggerEnableProperty); }
            set { SetValue(TriggerEnableProperty, value); }
        }

        public ICommand TriggerEnableCommand
        {
            get { return (ICommand)GetValue(TriggerEnableCommandProperty); }
            set { SetValue(TriggerEnableCommandProperty, value); }
        }

        public string TriggerError
        {
            get { return (string)GetValue(TriggerErrorProperty); }
            set { SetValue(TriggerErrorProperty, value); }
        }

        public string TriggerErrorColor
        {
            get { return (string)GetValue(TriggerErrorColorProperty); }
            set { SetValue(TriggerErrorColorProperty, value); }
        }

        public double TriggerIdleMS
        {
            get { return (double)GetValue(TriggerIdleMSProperty); }
            set { SetValue(TriggerIdleMSProperty, value); }
        }

        public string TriggerImagePath
        {
            get { return (string)GetValue(TriggerImagePathProperty); }
            set { SetValue(TriggerImagePathProperty, value); }
        }

        public bool TriggerIsVisible
        {
            get { return (bool)GetValue(TriggerIsVisibleProperty); }
            set { SetValue(TriggerIsVisibleProperty, value); }
        }

        public int TriggerIterations
        {
            get { return (int)GetValue(TriggerIterationsProperty); }
            set { SetValue(TriggerIterationsProperty, value); }
        }

        public string TriggerLabel
        {
            get { return (string)GetValue(TriggerLabelProperty); }
            set { SetValue(TriggerLabelProperty, value); }
        }

        public int TriggerMode
        {
            get { return (int)GetValue(TriggerModeProperty); }
            set { SetValue(TriggerModeProperty, value); }
        }

        public ObservableCollection<StringPC> TriggerModeItems
        {
            get { return (ObservableCollection<StringPC>)GetValue(TriggerModeItemsProperty); }
            set { SetValue(TriggerModeItemsProperty, value); }
        }

        public int TriggerRepeat
        {
            get { return (int)GetValue(TriggerRepeatProperty); }
            set { SetValue(TriggerRepeatProperty, value); }
        }

        public int TriggerStartEdge
        {
            get { return (int)GetValue(TriggerStartEdgeProperty); }
            set { SetValue(TriggerStartEdgeProperty, value); }
        }

        public double TriggerStartIdleMS
        {
            get { return (double)GetValue(TriggerStartIdleMSProperty); }
            set { SetValue(TriggerStartIdleMSProperty, value); }
        }

        public int TriggerType
        {
            get { return (int)GetValue(TriggerTypeProperty); }
            set { SetValue(TriggerTypeProperty, value); }
        }

        public ObservableCollection<StringPC> TriggerTypeItems
        {
            get { return (ObservableCollection<StringPC>)GetValue(TriggerTypeItemsProperty); }
            set { SetValue(TriggerTypeItemsProperty, value); }
        }

        #endregion Properties

        #region Methods

        public static void onSwitchEnableChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            int val = Convert.ToInt32(e.NewValue);
        }

        public static void onSwitchNameChanged0(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            string val = Convert.ToString(e.NewValue);
        }

        public static void onSwitchNameChanged1(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            string val = Convert.ToString(e.NewValue);
        }

        public static void onSwitchNameChanged2(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            string val = Convert.ToString(e.NewValue);
        }

        public static void onSwitchNameChanged3(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            string val = Convert.ToString(e.NewValue);
        }

        public static void onSwitchNameChanged4(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            string val = Convert.ToString(e.NewValue);
        }

        public static void onSwitchNameChanged5(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            string val = Convert.ToString(e.NewValue);
        }

        public static void onSwitchNameChanged6(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            string val = Convert.ToString(e.NewValue);
        }

        public static void onSwitchNameChanged7(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            string val = Convert.ToString(e.NewValue);
        }

        public static void onSwitchStateChanged0(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            int val = Convert.ToInt32(e.NewValue);
        }

        public static void onSwitchStateChanged1(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            int val = Convert.ToInt32(e.NewValue);
        }

        public static void onSwitchStateChanged2(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            int val = Convert.ToInt32(e.NewValue);
        }

        public static void onSwitchStateChanged3(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            int val = Convert.ToInt32(e.NewValue);
        }

        public static void onSwitchStateChanged4(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            int val = Convert.ToInt32(e.NewValue);
        }

        public static void onSwitchStateChanged5(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            int val = Convert.ToInt32(e.NewValue);
        }

        public static void onSwitchStateChanged6(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            int val = Convert.ToInt32(e.NewValue);
        }

        public static void onSwitchStateChanged7(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            int val = Convert.ToInt32(e.NewValue);
        }

        //This Method is only used in the Script panel. Not in Capture Setup
        public void PersistXmlDoc()
        {
            string enStr = SwitchEnable.ToString();
            string staStr = SwitchState0.ToString() +
                            SwitchState1.ToString() +
                            SwitchState2.ToString() +
                            SwitchState3.ToString() +
                            SwitchState4.ToString() +
                            SwitchState5.ToString() +
                            SwitchState6.ToString() +
                            SwitchState7.ToString();

            XmlNode node = SettingsDocument.SelectSingleNode("Command/DigiSwitch");
            XmlManager.SetAttribute(node, SettingsDocument, "status", staStr);
            XmlManager.SetAttribute(node, SettingsDocument, "enabled", enStr);
        }

        #endregion Methods
    }
}