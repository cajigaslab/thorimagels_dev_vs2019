namespace KuriosControl
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.ComponentModel;
    using System.Diagnostics;
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

    using KuriosControl.Common;
    using KuriosControl.Model;
    using KuriosControl.ViewModel;

    /// <summary>
    /// Interaction logic for KuriosDisplay.xaml
    /// </summary>
    public partial class KuriosDisplay : UserControl, INotifyPropertyChanged
    {
        #region Fields

        public static DependencyProperty CurrentExposureProperty = 
            DependencyProperty.RegisterAttached("CurrentExposure",
            typeof(double),
            typeof(KuriosDisplay),
            new FrameworkPropertyMetadata(new PropertyChangedCallback(onCurrentExposureChanged)));
        public static DependencyProperty ExposureMaxProperty = 
            DependencyProperty.RegisterAttached("ExposureMax",
            typeof(double),
            typeof(KuriosDisplay),
            new FrameworkPropertyMetadata(new PropertyChangedCallback(onExposureMaxChanged)));
        public static DependencyProperty ExposureMinProperty = 
            DependencyProperty.RegisterAttached("ExposureMin",
            typeof(double),
            typeof(KuriosDisplay),
            new FrameworkPropertyMetadata(new PropertyChangedCallback(onExposureMinChanged)));
        public static DependencyProperty KuriosBandwidthModeIndexProperty = 
            DependencyProperty.RegisterAttached("KuriosBandwidthModeIndex",
            typeof(int),
            typeof(KuriosDisplay),
            new FrameworkPropertyMetadata(new PropertyChangedCallback(onBandwidthModeIndexChanged)));
        public static DependencyProperty KuriosCurrentWavelengthSequenceNameProperty = 
            DependencyProperty.RegisterAttached("KuriosCurrentWavelengthSequenceName",
            typeof(string),
            typeof(KuriosDisplay),
            new FrameworkPropertyMetadata(new PropertyChangedCallback(onCurrentWavelengthSequenceChanged)));
        public static DependencyProperty KuriosStartWavelengthProperty = 
            DependencyProperty.RegisterAttached("KuriosStartWavelength",
            typeof(int),
            typeof(KuriosDisplay),
            new FrameworkPropertyMetadata(new PropertyChangedCallback(onStartWavelengthChanged)));
        public static DependencyProperty KuriosStepNumProperty = 
            DependencyProperty.RegisterAttached("KuriosStepCount",
            typeof(int),
            typeof(KuriosDisplay));
        public static DependencyProperty KuriosStepSizeProperty = 
            DependencyProperty.RegisterAttached("KuriosStepSize",
            typeof(int),
            typeof(KuriosDisplay),
            new FrameworkPropertyMetadata(new PropertyChangedCallback(onStepSizeChanged)));
        public static DependencyProperty KuriosStopWavelengthProperty = 
            DependencyProperty.RegisterAttached("KuriosStopWavelength",
            typeof(int),
            typeof(KuriosDisplay),
            new FrameworkPropertyMetadata(new PropertyChangedCallback(onStopWavelengthChanged)));
        public static DependencyProperty KuriosWavelengthMaxProperty = 
            DependencyProperty.RegisterAttached("KuriosWavelengthMax",
            typeof(int),
            typeof(KuriosDisplay),
            new FrameworkPropertyMetadata(new PropertyChangedCallback(onRangeWavelengthChanged)));
        public static DependencyProperty KuriosWavelengthMinProperty = 
            DependencyProperty.RegisterAttached("KuriosWavelengthMin",
            typeof(int),
            typeof(KuriosDisplay),
            new FrameworkPropertyMetadata(new PropertyChangedCallback(onRangeWavelengthChanged)));
        public static DependencyProperty KuriosWavelengthProperty = 
            DependencyProperty.RegisterAttached("KuriosWavelength",
            typeof(int),
            typeof(KuriosDisplay),
            new FrameworkPropertyMetadata(new PropertyChangedCallback(onWavelengthChanged)));

        private ControlViewModel _vm;

        #endregion Fields

        #region Constructors

        public KuriosDisplay()
        {
            _vm = new ControlViewModel();
            _vm.WavelengthSequencesUpdated += _vm_WavelengthSequencesUpdated;
            _vm.WavelengthStartStopUpdated += _vm_WavelengthStartStopUpdated;
            _vm.UpdateTargetWavelength += _vm_UpdateTargetWavelength;
            this.Unloaded += KuriosDisplay_Unloaded;
            InitializeComponent();
        }

        #endregion Constructors

        #region Events

        /// Raised when a property on this object has a new value.
        /// </summary>
        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public ICommand AddWavelengthSequenceCommand
        {
            get
            {
                return _vm.AddWavelengthSequenceCommand;
            }
        }

        public double CurrentExposure
        {
            get
            {
                return (double)GetValue(CurrentExposureProperty);
            }
            set
            {
                SetValue(CurrentExposureProperty, value);
            }
        }

        public string CurrentWavelengthSequence
        {
            get
            {
                return _vm.CurrentWavelengthSequence;
            }
            set
            {

                if (null != value)
                {
                    _vm.CurrentWavelengthSequence = value;
                    if (null != _vm.CurrentWavelengthSequence)
                    {
                        KuriosCurrentWavelengthSequenceName = _vm.CurrentWavelengthSequence;
                    }
                    OnPropertyChanged("CurrentWavelengthSequence");
                }
            }
        }

        public ICommand DeleteWavelengthSequenceCommand
        {
            get
            {
                return _vm.DeleteWavelengthSequenceCommand;
            }
        }

        public ICommand EditWavelengthSequenceCommand
        {
            get
            {
                return _vm.EditWavelengthSequenceCommand;
            }
        }

        public double ExposureMax
        {
            get
            {
                return (double)GetValue(ExposureMaxProperty);
            }
            set
            {
                SetValue(ExposureMaxProperty, value);
            }
        }

        public double ExposureMin
        {
            get
            {
                return (double)GetValue(ExposureMinProperty);
            }
            set
            {
                SetValue(ExposureMinProperty, value);
            }
        }

        public int KuriosBandwidthModeIndex
        {
            get
            {
                return (int)GetValue(KuriosBandwidthModeIndexProperty);
            }
            set
            {
                SetValue(KuriosBandwidthModeIndexProperty, value);
            }
        }

        public String KuriosCurrentWavelengthSequenceName
        {
            get
            {
                return (string)GetValue(KuriosCurrentWavelengthSequenceNameProperty);
            }
            set
            {
                SetValue(KuriosCurrentWavelengthSequenceNameProperty, value);
            }
        }

        public int KuriosStartWavelength
        {
            get
            {
                return (int)GetValue(KuriosStartWavelengthProperty);
            }
            set
            {
                SetValue(KuriosStartWavelengthProperty, value);
            }
        }

        public int KuriosStepCount
        {
            get
            {
                return (int)GetValue(KuriosStepNumProperty);
            }
            set
            {
                SetValue(KuriosStepNumProperty, value);
            }
        }

        public int KuriosStepSize
        {
            get
            {
                return (int)GetValue(KuriosStepSizeProperty);
            }
            set
            {
                SetValue(KuriosStepSizeProperty, value);
            }
        }

        public int KuriosStopWavelength
        {
            get
            {
                return (int)GetValue(KuriosStopWavelengthProperty);
            }
            set
            {
                SetValue(KuriosStopWavelengthProperty, value);
            }
        }

        public int KuriosWavelength
        {
            get
            {
                return (int)GetValue(KuriosWavelengthProperty);
            }
            set
            {
                SetValue(KuriosWavelengthProperty, value);
            }
        }

        public int KuriosWavelengthMax
        {
            get
            {
                return (int)GetValue(KuriosWavelengthMaxProperty);
            }
            set
            {
                SetValue(KuriosWavelengthMaxProperty, value);
            }
        }

        public int KuriosWavelengthMin
        {
            get
            {
                return (int)GetValue(KuriosWavelengthMinProperty);
            }
            set
            {
                SetValue(KuriosWavelengthMinProperty, value);
            }
        }

        public ControlViewModel ViewModel
        {
            get
            {
                return _vm;
            }
            set
            {
                _vm = value;
            }
        }

        public Range<int> WavelengthRange
        {
            get
            {
                return _vm.WavelengthRange;
            }
        }

        public ObservableCollection<string> WavelengthSequences
        {
            get
            {
                return _vm.WavelengthSequences;
            }
        }

        #endregion Properties

        #region Methods

        public static void onBandwidthModeIndexChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if (null == (d as KuriosDisplay)) return;
            (d as KuriosDisplay).UpdateBandwidthMode();
        }

        public static void onCurrentExposureChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if (null == (d as KuriosDisplay)) return;
            (d as KuriosDisplay).UpdateCurrentExposure();
        }

        public static void onCurrentWavelengthSequenceChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if (null == (d as KuriosDisplay)) return;
            (d as KuriosDisplay).UpdateCurrentWavelengthSequence();
        }

        public static void onExposureMaxChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if (null == (d as KuriosDisplay)) return;
            (d as KuriosDisplay).UpdateExposureMax();
        }

        public static void onExposureMinChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if (null == (d as KuriosDisplay)) return;
            (d as KuriosDisplay).UpdateExposureMin();
        }

        public static void onRangeWavelengthChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if (null == (d as KuriosDisplay)) return;
            (d as KuriosDisplay).UpdateWavelengthRange();
        }

        public static void onStartWavelengthChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if (null == (d as KuriosDisplay)) return;
            (d as KuriosDisplay).UpdateStartWavelength();
        }

        public static void onStepSizeChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if (null == (d as KuriosDisplay)) return;
            (d as KuriosDisplay).UpdateStepSize();
        }

        public static void onStopWavelengthChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if (null == (d as KuriosDisplay)) return;
            (d as KuriosDisplay).UpdateStopWavelength();
        }

        public static void onWavelengthChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if (null == (d as KuriosDisplay)) return;
            (d as KuriosDisplay).UpdateWavelength();
        }

        /// <summary>
        /// Called when [property changed].
        /// </summary>
        /// <param name="propertyName">Name of the property.</param>
        public void OnPropertyChanged(string propertyName)
        {
            this.VerifyPropertyName(propertyName);

            if (KuriosUserControl.PropertyChanged != null)
            {
                KuriosUserControl.PropertyChanged(KuriosUserControl, new PropertyChangedEventArgs(propertyName));
            }
        }

        /// <summary>
        /// Verifies the name of the property.
        /// </summary>
        /// <param name="propertyName">Name of the property.</param>
        public void VerifyPropertyName(string propertyName)
        {
            if (TypeDescriptor.GetProperties(KuriosUserControl)[propertyName] == null)
            {
                Debug.Fail("Invalid property name: " + propertyName);
            }
        }

        void KuriosDisplay_Unloaded(object sender, RoutedEventArgs e)
        {
            _vm.CloseWindows();
        }

        private void UpdateBandwidthMode()
        {
            switch (this.KuriosBandwidthModeIndex)
            {
                case 0: _vm.BandwidthMode = BandwidthModes.WIDE; break;
                case 1: _vm.BandwidthMode = BandwidthModes.MEDIUM; break;
                case 2: _vm.BandwidthMode  = BandwidthModes.NARROW; break;
            }
        }

        private void UpdateCurrentExposure()
        {
            _vm.CurrentExposure = this.CurrentExposure;
        }

        private void UpdateCurrentWavelengthSequence()
        {
            if (null == WavelengthSequences) return;
            for (int i = 0; i < this.WavelengthSequences.Count; i++)
            {
                if (this.WavelengthSequences[i] == this.KuriosCurrentWavelengthSequenceName)
                {
                    CurrentWavelengthSequence = this.WavelengthSequences[i];
                    break;
                }
            }
        }

        private void UpdateExposureMax()
        {
            _vm.ExposureMax = this.ExposureMax;
        }

        private void UpdateExposureMin()
        {
            _vm.ExposureMin = this.ExposureMin;
        }

        private void UpdateStartWavelength()
        {
            _vm.SequenceParameter.SeqWavelengthStart = this.KuriosStartWavelength;
            this.KuriosStepCount = _vm.SequenceParameter.StepsWavelengthCount;
            OnPropertyChanged("KuriosStepCount");
        }

        private void UpdateStepSize()
        {
            _vm.SequenceParameter.SeqWavelengthStep = this.KuriosStepSize;
            this.KuriosStepCount = _vm.SequenceParameter.StepsWavelengthCount;
            OnPropertyChanged("KuriosStepCount");
        }

        private void UpdateStopWavelength()
        {
            _vm.SequenceParameter.SeqWavelengthStop = this.KuriosStopWavelength;
            this.KuriosStepCount = _vm.SequenceParameter.StepsWavelengthCount;
            OnPropertyChanged("KuriosStepCount");
        }

        private void UpdateWavelength()
        {
            _vm.TargetWavelength = this.KuriosWavelength;
        }

        private void UpdateWavelengthRange()
        {
            _vm.WavelengthRange = new Common.Range<int>(KuriosWavelengthMin, KuriosWavelengthMax);
        }

        void _vm_UpdateTargetWavelength()
        {
            _vm.TargetWavelength = this.KuriosWavelength;
        }

        void _vm_WavelengthSequencesUpdated()
        {
            OnPropertyChanged("WavelengthSequences");
            OnPropertyChanged("CurrentWavelengthSequence");
            if (null != _vm.CurrentWavelengthSequence && null != _vm.CurrentWavelengthSequence)
            {
                this.KuriosCurrentWavelengthSequenceName = _vm.CurrentWavelengthSequence;
                linkListView.SelectedIndex = _vm.WavelengthSequences.IndexOf(_vm.CurrentWavelengthSequence);
            }
        }

        void _vm_WavelengthStartStopUpdated()
        {
            KuriosStartWavelength = Math.Max(WavelengthRange.Min, Math.Min(WavelengthRange.Max, (int)_vm.SequenceParameter.SeqWavelengthStart));
            KuriosStopWavelength = Math.Max(WavelengthRange.Min, Math.Min(WavelengthRange.Max, (int)_vm.SequenceParameter.SeqWavelengthStop));
        }

        #endregion Methods
    }
}