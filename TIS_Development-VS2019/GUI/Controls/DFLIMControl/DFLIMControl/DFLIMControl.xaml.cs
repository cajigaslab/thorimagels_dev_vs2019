namespace DFLIMControl
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Collections.Specialized;
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
    using System.Windows.Navigation;
    using System.Windows.Shapes;
    using System.Windows.Threading;

    using ThorSharedTypes;

    /// <summary>
    /// Interaction logic for UserControl1.xaml
    /// </summary>
    public partial class DFLIMControlUC : UserControl
    {
        #region Fields

        public static DependencyProperty DFLIMAcquisitionModeProperty = 
             DependencyProperty.Register("DFLIMAcquisitionMode",
             typeof(int),
             typeof(DFLIMControlUC));
        public static DependencyProperty DFLIMDisplayFitCommandProperty = 
           DependencyProperty.Register("DFLIMDisplayFitCommand",
           typeof(ICommand),
           typeof(DFLIMControlUC));
        public static DependencyProperty DFLIMDisplayLifetimeImageProperty = 
             DependencyProperty.Register("DFLIMDisplayLifetimeImage",
             typeof(bool),
             typeof(DFLIMControlUC));
        public static DependencyProperty DFLIMDisplaySetupAssitantCommandProperty = 
           DependencyProperty.Register("DFLIMDisplaySetupAssitantCommand",
           typeof(ICommand),
           typeof(DFLIMControlUC));
        public static DependencyProperty DFLIMFitVisibilityProperty = 
             DependencyProperty.Register("DFLIMFitVisibility",
             typeof(Visibility),
             typeof(DFLIMControlUC));
        public static DependencyProperty DFLIMHistogramCopyCounterProperty = 
             DependencyProperty.Register("DFLIMHistogramCopyCounter",
             typeof(long),
             typeof(DFLIMControlUC));
        public static DependencyProperty DFLIMHistogramDictionaryProperty = 
             DependencyProperty.Register("DFLIMHistogramDictionary",
             typeof(Dictionary<KeyValuePair<int, SolidColorBrush>, uint[]>),
             typeof(DFLIMControlUC),
                new FrameworkPropertyMetadata(
                new PropertyChangedCallback(OnDFLIMHistogramDictionaryChanged)));
        public static DependencyProperty DFLIMHistogramMVMDataLockrProperty = 
             DependencyProperty.Register("DFLIMHistogramMVMDataLock",
             typeof(object),
             typeof(DFLIMControlUC));
        public static DependencyProperty DFLIMHWControlsVisibilityProperty = 
             DependencyProperty.Register("DFLIMHWControlsVisibility",
             typeof(Visibility),
             typeof(DFLIMControlUC));
        public static DependencyProperty DFLIMLUTHighProperty = 
           DependencyProperty.Register("DFLIMLUTHigh",
           typeof(CustomCollection<uint>),
           typeof(DFLIMControlUC));
        public static DependencyProperty DFLIMLUTLowProperty = 
             DependencyProperty.Register("DFLIMLUTLow",
             typeof(CustomCollection<uint>),
             typeof(DFLIMControlUC));
        public static DependencyProperty DFLIMReSyncCommandProperty = 
            DependencyProperty.Register("DFLIMReSyncCommand",
            typeof(ICommand),
            typeof(DFLIMControlUC));
        public static DependencyProperty DFLIMTauHighProperty = 
             DependencyProperty.Register("DFLIMTauHigh",
             typeof(CustomCollection<float>),
             typeof(DFLIMControlUC));
        public static DependencyProperty DFLIMTauLowProperty = 
             DependencyProperty.Register("DFLIMTauLow",
             typeof(CustomCollection<float>),
             typeof(DFLIMControlUC));
        public static DependencyProperty DFLIMTZeroProperty = 
             DependencyProperty.Register("DFLIMTZero",
             typeof(CustomCollection<float>),
             typeof(DFLIMControlUC));

        long _lastCopyHistogramNumber = 0;

        #endregion Fields

        #region Constructors

        public DFLIMControlUC()
        {
            InitializeComponent();
            Loaded += DFLIMControlUC_Loaded;
            Unloaded += DFLIMControlUC_Unloaded;
        }

        #endregion Constructors

        #region Properties

        public int DFLIMAcquisitionMode
        {
            get { return (int)GetValue(DFLIMAcquisitionModeProperty); }
            set { SetValue(DFLIMAcquisitionModeProperty, value); }
        }

        public ICommand DFLIMDisplayFitCommand
        {
            get { return (ICommand)GetValue(DFLIMDisplayFitCommandProperty); }
            set { SetValue(DFLIMDisplayFitCommandProperty, value); }
        }

        public bool DFLIMDisplayLifetimeImage
        {
            get { return (bool)GetValue(DFLIMDisplayLifetimeImageProperty); }
            set { SetValue(DFLIMDisplayLifetimeImageProperty, value); }
        }

        public ICommand DFLIMDisplaySetupAssitantCommand
        {
            get { return (ICommand)GetValue(DFLIMDisplaySetupAssitantCommandProperty); }
            set { SetValue(DFLIMDisplaySetupAssitantCommandProperty, value); }
        }

        public Visibility DFLIMFitVisibility
        {
            get { return (Visibility)GetValue(DFLIMFitVisibilityProperty); }
            set { SetValue(DFLIMFitVisibilityProperty, value); }
        }

        public long DFLIMHistogramCopyCounter
        {
            get { return (long)GetValue(DFLIMHistogramCopyCounterProperty); }
            set { SetValue(DFLIMHistogramCopyCounterProperty, value); }
        }

        public Dictionary<KeyValuePair<int, SolidColorBrush>, uint[]> DFLIMHistogramDictionary
        {
            get { return (Dictionary<KeyValuePair<int, SolidColorBrush>, uint[]>)GetValue(DFLIMHistogramDictionaryProperty); }
            set { SetValue(DFLIMHistogramDictionaryProperty, value); }
        }

        public object DFLIMHistogramMVMDataLock
        {
            get { return (object)GetValue(DFLIMHistogramMVMDataLockrProperty); }
            set { SetValue(DFLIMHistogramMVMDataLockrProperty, value); }
        }

        public Visibility DFLIMHWControlsVisibility
        {
            get { return (Visibility)GetValue(DFLIMHWControlsVisibilityProperty); }
            set { SetValue(DFLIMHWControlsVisibilityProperty, value); }
        }

        public CustomCollection<uint> DFLIMLUTHigh
        {
            get { return (CustomCollection<uint>)GetValue(DFLIMLUTHighProperty); }
            set { SetValue(DFLIMLUTHighProperty, value); }
        }

        public CustomCollection<uint> DFLIMLUTLow
        {
            get { return (CustomCollection<uint>)GetValue(DFLIMLUTLowProperty); }
            set { SetValue(DFLIMLUTLowProperty, value); }
        }

        public ICommand DFLIMReSyncCommand
        {
            get { return (ICommand)GetValue(DFLIMReSyncCommandProperty); }
            set { SetValue(DFLIMReSyncCommandProperty, value); }
        }

        public CustomCollection<float> DFLIMTauHigh
        {
            get { return (CustomCollection<float>)GetValue(DFLIMTauHighProperty); }
            set { SetValue(DFLIMTauHighProperty, value); }
        }

        public CustomCollection<float> DFLIMTauLow
        {
            get { return (CustomCollection<float>)GetValue(DFLIMTauLowProperty); }
            set { SetValue(DFLIMTauLowProperty, value); }
        }

        public CustomCollection<float> DFLIMTZero
        {
            get { return (CustomCollection<float>)GetValue(DFLIMTZeroProperty); }
            set { SetValue(DFLIMTZeroProperty, value); }
        }

        #endregion Properties

        #region Methods

        public static void OnDFLIMHistogramDictionaryChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            try
            {
                (d as DFLIMControlUC).DrawHistogram();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error");
            }
        }

        static int[] SubArray(int[] data, int index, int length)
        {
            int[] result = new int[length];
            Array.Copy(data, index, result, 0, length);
            return result;
        }

        void DFLIMControlUC_Loaded(object sender, RoutedEventArgs e)
        {
            if (DataContext is IViewModelActions)
            {
                (DataContext as IViewModelActions).HandleViewLoaded();
            }
        }

        void DFLIMControlUC_Unloaded(object sender, RoutedEventArgs e)
        {
            if (DataContext is IViewModelActions)
            {
                (DataContext as IViewModelActions).HandleViewUnloaded();
            }
        }

        void DrawHistogram()
        {
            if (DFLIMHistogramCopyCounter > _lastCopyHistogramNumber)
            {
                lock (DFLIMHistogramMVMDataLock)
                {
                    _lastCopyHistogramNumber = DFLIMHistogramCopyCounter;

                    var histrogram = DFLIMHistogramDictionary;
                    if (null != histrogram)
                    {
                        dflimHistogram.Plot(histrogram);
                    }
                }
            }
        }

        #endregion Methods
    }

    static class ObjectExtensions
    {
        #region Methods

        static uint[] Copy(uint[] pieces)
        {
            return pieces.Select(x =>
            {
                var handle = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(uint)));

                try
                {
                    Marshal.StructureToPtr(x, handle, false);
                    return (uint)Marshal.PtrToStructure(handle, typeof(uint));
                }
                finally
                {
                    Marshal.FreeHGlobal(handle);
                }
            }).ToArray();
        }

        #endregion Methods
    }
}