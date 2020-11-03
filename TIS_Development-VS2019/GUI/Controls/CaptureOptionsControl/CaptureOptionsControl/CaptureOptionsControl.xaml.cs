namespace CaptureOptionsControl
{
    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Globalization;
    using System.IO;
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

    using ThorSharedTypes;

    /// <summary>
    /// Interaction logic for UserControl1.xaml
    /// </summary>
    public partial class CaptureOptionsControlUC : UserControl
    {
        #region Fields

        public static readonly DependencyProperty BleachControlActiveProperty = DependencyProperty.Register("BleachControlActive", typeof(bool), typeof(CaptureOptionsControlUC));
        public static readonly DependencyProperty FastZActiveProperty = DependencyProperty.Register("FastZActive", typeof(bool), typeof(CaptureOptionsControlUC));
        public static readonly DependencyProperty BleachFramesProperty = DependencyProperty.Register("BleachFrames", typeof(int), typeof(CaptureOptionsControlUC));
        public static readonly DependencyProperty BleachingCaptureModeVisProperty = DependencyProperty.Register("BleachingCaptureModeVis", typeof(Visibility), typeof(CaptureOptionsControlUC));
        public static readonly DependencyProperty BleachPostTriggerStrProperty = DependencyProperty.Register("BleachPostTriggerStr", typeof(string), typeof(CaptureOptionsControlUC));
        public static readonly DependencyProperty BleachTriggerStrProperty = DependencyProperty.Register("BleachTriggerStr", typeof(string), typeof(CaptureOptionsControlUC));
        public static readonly DependencyProperty CaptureModeProperty = DependencyProperty.Register("CaptureMode", typeof(int), typeof(CaptureOptionsControlUC));
        public static readonly DependencyProperty DataTypeStrProperty = DependencyProperty.Register("DataTypeStr", typeof(string), typeof(CaptureOptionsControlUC));
        public static readonly DependencyProperty EnabledPMTsProperty = DependencyProperty.Register("EnabledPMTs", typeof(string), typeof(CaptureOptionsControlUC));
        public static readonly DependencyProperty FastZEnabledDisabledStrProperty = DependencyProperty.Register("FastZEnabledDisabledStr", typeof(string), typeof(CaptureOptionsControlUC));
        public static readonly DependencyProperty FiniteStreamingVisProperty = DependencyProperty.Register("FiniteStreamingVis", typeof(Visibility), typeof(CaptureOptionsControlUC));
        public static readonly DependencyProperty HyperSpectralCaptureActiveProperty = DependencyProperty.Register("HyperSpectralCaptureActive", typeof(bool), typeof(CaptureOptionsControlUC));
        public static readonly DependencyProperty HyperspectralModeVisProperty = DependencyProperty.Register("HyperspectralModeVis", typeof(Visibility), typeof(CaptureOptionsControlUC));
        public static readonly DependencyProperty KuriousSequenceStepsProperty = DependencyProperty.Register("KuriousSequenceSteps", typeof(int), typeof(CaptureOptionsControlUC));
        public static readonly DependencyProperty PostBleach1LabelProperty = DependencyProperty.Register("PostBleach1Label", typeof(string), typeof(CaptureOptionsControlUC));
        public static readonly DependencyProperty PostBleach2LabelProperty = DependencyProperty.Register("PostBleach2Label", typeof(string), typeof(CaptureOptionsControlUC));
        public static readonly DependencyProperty PostBleachFrames1Property = DependencyProperty.Register("PostBleachFrames1", typeof(int), typeof(CaptureOptionsControlUC));
        public static readonly DependencyProperty PostBleachFrames2Property = DependencyProperty.Register("PostBleachFrames2", typeof(int), typeof(CaptureOptionsControlUC));
        public static readonly DependencyProperty PostBleachInterval1Property = DependencyProperty.Register("PostBleachInterval1", typeof(double), typeof(CaptureOptionsControlUC));
        public static readonly DependencyProperty PostBleachInterval2Property = DependencyProperty.Register("PostBleachInterval2", typeof(double), typeof(CaptureOptionsControlUC));
        public static readonly DependencyProperty PostBleachMode1Property = DependencyProperty.Register("PostBleachMode1", typeof(string), typeof(CaptureOptionsControlUC));
        public static readonly DependencyProperty PostBleachMode2Property = DependencyProperty.Register("PostBleachMode2", typeof(string), typeof(CaptureOptionsControlUC));
        public static readonly DependencyProperty PreBleachFramesProperty = DependencyProperty.Register("PreBleachFrames", typeof(int), typeof(CaptureOptionsControlUC));
        public static readonly DependencyProperty PreBleachIntervalProperty = DependencyProperty.Register("PreBleachInterval", typeof(double), typeof(CaptureOptionsControlUC));
        public static readonly DependencyProperty PreBleachModeProperty = DependencyProperty.Register("PreBleachMode", typeof(string), typeof(CaptureOptionsControlUC));
        public static readonly DependencyProperty SimultaneousEnabledStrProperty = DependencyProperty.Register("SimultaneousEnabledStr", typeof(string), typeof(CaptureOptionsControlUC));
        public static readonly DependencyProperty StaircaseEnabledDisabledStrProperty = DependencyProperty.Register("StaircaseEnabledDisabledStr", typeof(string), typeof(CaptureOptionsControlUC));
        public static readonly DependencyProperty StimulusMaxFramesProperty = DependencyProperty.Register("StimulusMaxFrames", typeof(int), typeof(CaptureOptionsControlUC));
        public static readonly DependencyProperty StimulusStreamingVisProperty = DependencyProperty.Register("StimulusStreamingVis", typeof(Visibility), typeof(CaptureOptionsControlUC));
        public static readonly DependencyProperty StreamFramesProperty = DependencyProperty.Register("StreamFrames", typeof(int), typeof(CaptureOptionsControlUC));
        public static readonly DependencyProperty StreamingCaptureModeVisProperty = DependencyProperty.Register("StreamingCaptureModeVis", typeof(Visibility), typeof(CaptureOptionsControlUC));
        public static readonly DependencyProperty StreamingStorageModeStrProperty = DependencyProperty.Register("StreamingStorageModeStr", typeof(string), typeof(CaptureOptionsControlUC));
        public static readonly DependencyProperty StreamVolumesProperty = DependencyProperty.Register("StreamVolumes", typeof(int), typeof(CaptureOptionsControlUC));
        public static readonly DependencyProperty TFramesProperty = DependencyProperty.Register("TFrames", typeof(int), typeof(CaptureOptionsControlUC));
        public static readonly DependencyProperty TIntervalProperty = DependencyProperty.Register("TInterval", typeof(double), typeof(CaptureOptionsControlUC));
        public static readonly DependencyProperty TriggerModeStreamingStrProperty = DependencyProperty.Register("TriggerModeStreamingStr", typeof(string), typeof(CaptureOptionsControlUC));
        public static readonly DependencyProperty TriggerModeTimelapseStrProperty = DependencyProperty.Register("TriggerModeTimelapseStr", typeof(string), typeof(CaptureOptionsControlUC));
        public static readonly DependencyProperty TSeriesCaptureModeVisProperty = DependencyProperty.Register("TSeriesCaptureModeVis", typeof(Visibility), typeof(CaptureOptionsControlUC));
        public static readonly DependencyProperty ZFastEnableProperty = DependencyProperty.Register("ZFastEnable", typeof(bool), typeof(CaptureOptionsControlUC));

        #endregion Fields

        #region Constructors

        public CaptureOptionsControlUC()
        {
            InitializeComponent();
        }

        #endregion Constructors

        #region Properties

        public bool BleachControlActive
        {
            get { return (bool)GetValue(BleachControlActiveProperty); }
            set { SetValue(BleachControlActiveProperty, value); }
        }

        public bool FastZActive
        {
            get { return (bool)GetValue(FastZActiveProperty); }
            set { SetValue(FastZActiveProperty, value); }
        }

        public int BleachFrames
        {
            get { return (int)GetValue(BleachFramesProperty); }
            set { SetValue(BleachFramesProperty, value); }
        }

        public Visibility BleachingCaptureModeVis
        {
            get { return (Visibility)GetValue(BleachingCaptureModeVisProperty); }
            set { SetValue(BleachingCaptureModeVisProperty, value); }
        }

        public string BleachTriggerStr
        {
            get { return (string)GetValue(BleachTriggerStrProperty); }
            set { SetValue(BleachTriggerStrProperty, value); }
        }

        public string BleachPostTriggerStr
        {
            get { return (string)GetValue(BleachPostTriggerStrProperty); }
            set { SetValue(BleachPostTriggerStrProperty, value); }
        }

        public int CaptureMode
        {
            get { return (int)GetValue(CaptureModeProperty); }
            set { SetValue(CaptureModeProperty, value); }
        }

        public string DataTypeStr
        {
            get { return (string)GetValue(DataTypeStrProperty); }
            set { SetValue(DataTypeStrProperty, value); }
        }

        public string EnabledPMTs
        {
            get { return (string)GetValue(EnabledPMTsProperty); }
            set { SetValue(EnabledPMTsProperty, value); }
        }

        public string FastZEnabledDisabledStr
        {
            get { return (string)GetValue(FastZEnabledDisabledStrProperty); }
            set { SetValue(FastZEnabledDisabledStrProperty, value); }
        }

        public Visibility FiniteStreamingVis
        {
            get { return (Visibility)GetValue(FiniteStreamingVisProperty); }
            set { SetValue(FiniteStreamingVisProperty, value); }
        }

        public bool HyperSpectralCaptureActive
        {
            get { return (bool)GetValue(HyperSpectralCaptureActiveProperty); }
            set { SetValue(HyperSpectralCaptureActiveProperty, value); }
        }

        public Visibility HyperspectralModeVis
        {
            get { return (Visibility)GetValue(HyperspectralModeVisProperty); }
            set { SetValue(HyperspectralModeVisProperty, value); }
        }

        public int KuriousSequenceSteps
        {
            get { return (int)GetValue(KuriousSequenceStepsProperty); }
            set { SetValue(KuriousSequenceStepsProperty, value); }
        }

        public string PostBleach1Label
        {
            get { return (string)GetValue(PostBleach1LabelProperty); }
            set { SetValue(PostBleach1LabelProperty, value); }
        }

        public string PostBleach2Label
        {
            get { return (string)GetValue(PostBleach2LabelProperty); }
            set { SetValue(PostBleach2LabelProperty, value); }
        }

        public int PostBleachFrames1
        {
            get { return (int)GetValue(PostBleachFrames1Property); }
            set { SetValue(PostBleachFrames1Property, value); }
        }

        public int PostBleachFrames2
        {
            get { return (int)GetValue(PostBleachFrames2Property); }
            set { SetValue(PostBleachFrames2Property, value); }
        }

        public double PostBleachInterval1
        {
            get { return (double)GetValue(PostBleachInterval1Property); }
            set { SetValue(PostBleachInterval1Property, value); }
        }

        public double PostBleachInterval2
        {
            get { return (double)GetValue(PostBleachInterval2Property); }
            set { SetValue(PostBleachInterval2Property, value); }
        }

        public string PostBleachMode1
        {
            get { return (string)GetValue(PostBleachMode1Property); }
            set { SetValue(PostBleachMode1Property, value); }
        }

        public string PostBleachMode2
        {
            get { return (string)GetValue(PostBleachMode2Property); }
            set { SetValue(PostBleachMode2Property, value); }
        }

        public int PreBleachFrames
        {
            get { return (int)GetValue(PreBleachFramesProperty); }
            set { SetValue(PreBleachFramesProperty, value); }
        }

        public double PreBleachInterval
        {
            get { return (double)GetValue(PreBleachIntervalProperty); }
            set { SetValue(PreBleachIntervalProperty, value); }
        }

        public string PreBleachMode
        {
            get { return (string)GetValue(PreBleachModeProperty); }
            set { SetValue(PreBleachModeProperty, value); }
        }

        public string SimultaneousEnabledStr
        {
            get { return (string)GetValue(SimultaneousEnabledStrProperty); }
            set { SetValue(SimultaneousEnabledStrProperty, value); }
        }

        public string StaircaseEnabledDisabledStr
        {
            get { return (string)GetValue(StaircaseEnabledDisabledStrProperty); }
            set { SetValue(StaircaseEnabledDisabledStrProperty, value); }
        }

        public int StimulusMaxFrames
        {
            get { return (int)GetValue(StimulusMaxFramesProperty); }
            set { SetValue(StimulusMaxFramesProperty, value); }
        }

        public Visibility StimulusStreamingVis
        {
            get { return (Visibility)GetValue(StimulusStreamingVisProperty); }
            set { SetValue(StimulusStreamingVisProperty, value); }
        }

        public int StreamFrames
        {
            get { return (int)GetValue(StreamFramesProperty); }
            set { SetValue(StreamFramesProperty, value); }
        }

        public Visibility StreamingCaptureModeVis
        {
            get { return (Visibility)GetValue(StreamingCaptureModeVisProperty); }
            set { SetValue(StreamingCaptureModeVisProperty, value); }
        }

        public string StreamingStorageModeStr
        {
            get { return (string)GetValue(StreamingStorageModeStrProperty); }
            set { SetValue(StreamingStorageModeStrProperty, value); }
        }

        public int StreamVolumes
        {
            get { return (int)GetValue(StreamVolumesProperty); }
            set { SetValue(StreamVolumesProperty, value); }
        }

        public int TFrames
        {
            get { return (int)GetValue(TFramesProperty); }
            set { SetValue(TFramesProperty, value); }
        }

        public double TInterval
        {
            get { return (double)GetValue(TIntervalProperty); }
            set { SetValue(TIntervalProperty, value); }
        }

        public string TriggerModeStreamingStr
        {
            get { return (string)GetValue(TriggerModeStreamingStrProperty); }
            set { SetValue(TriggerModeStreamingStrProperty, value); }
        }

        public string TriggerModeTimelapseStr
        {
            get { return (string)GetValue(TriggerModeTimelapseStrProperty); }
            set { SetValue(TriggerModeTimelapseStrProperty, value); }
        }

        public Visibility TSeriesCaptureModeVis
        {
            get { return (Visibility)GetValue(TSeriesCaptureModeVisProperty); }
            set { SetValue(TSeriesCaptureModeVisProperty, value); }
        }

        public bool ZFastEnable
        {
            get { return (bool)GetValue(ZFastEnableProperty); }
            set { SetValue(ZFastEnableProperty, value); }
        }

        #endregion Properties

        #region Other

        //public static readonly DependencyProperty **REPLACE**CommandProperty =
        //      DependencyProperty.Register(
        //      "**REPLACE**Command",
        //      typeof(ICommand),
        //      typeof(DummyControlUC));
        //      public ICommand **REPLACE**
        //      {
        //          get { return (ICommand)GetValue(**REPLACE**CommandProperty); }
        //          set { SetValue(**REPLACE**CommandProperty, value); }
        //      }
        //public static readonly DependencyProperty **REPLACE**Property =
        //      DependencyProperty.Register(
        //      "**REPLACE**",
        //      typeof(int),
        //      typeof(DummyControlUC));
        //      public int **REPLACE**
        //      {
        //          get { return (int)GetValue(**REPLACE**Property); }
        //          set { SetValue(**REPLACE**Property, value); }
        //      }
        //public static DependencyProperty **REPLACE**Property =
        //DependencyProperty.RegisterAttached("**REPLACE**",
        //typeof(int),
        //typeof(DummyControlUC),
        //new FrameworkPropertyMetadata(new PropertyChangedCallback(on**REPLACE**Changed)));
        //public int **REPLACE**
        //{
        //    get { return (int)GetValue(**REPLACE**Property); }
        //    set { SetValue(**REPLACE**Property, value); }
        //}
        //public static void on**REPLACE**Changed(DependencyObject d, DependencyPropertyChangedEventArgs e)
        //{
        //}

        #endregion Other
    }
}