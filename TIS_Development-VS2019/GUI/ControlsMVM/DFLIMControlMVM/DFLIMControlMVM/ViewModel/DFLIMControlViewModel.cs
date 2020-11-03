namespace DFLIMControl.ViewModel
{
    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Collections.Specialized;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Reflection;
    using System.Runtime.CompilerServices;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Threading;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Xml;

    using DFLIMControl.Model;

    using DFLIMSetupAssistant;

    using ThorLogging;

    using ThorSharedTypes;

    public class DFLIMControlViewModel : DFLIMControlViewModelBase, IViewModelActions
    {
        #region Fields

        const long NUM_CHANNELS = 4;

        private static readonly object _roiHistogramsDataLock = new object();
        private static readonly object _roiHistogramsUpdateFunctionLock = new object();

        private readonly int[] _currentCoarseShift = new int[NUM_CHANNELS];
        private readonly int[] _currentFineShift = new int[NUM_CHANNELS];
        private readonly DFLIMControlModel _DFLIMControlModel;

        private static ReportDFLIMROIHistograms _dFLIMROIHistogramsCallback;

        private int _currentResyncDelay = 0;
        private int _currentSyncDelay = 0;
        private Thread _dataUpdateThread;
        private ICommand _DFLIMDisplaySetupAssitantCommand;
        private bool _dflimEnableControlsThatNeedAcquistionOff = true;
        private ICommand _DFLIMQueryClockFrequenciesCommand;
        private ICommand _DFLIMReSyncCommand;
        private ICommand _DFLIMRevertSetupAssistantSettingChangesCommand;
        private ICommand _DFLIMSaveSettingsCommand;
        private SetupAssistant _dflimSetupAssistant = null;
        private bool _extractedStartupSettings = false;
        private bool _newROIHistogramData = false;
        private Dictionary<string, PropertyInfo> _properties = new Dictionary<string, PropertyInfo>();
        private ROIHistogramsDataUpdateStruct _roiHistogramsDataUpdateStruct = new ROIHistogramsDataUpdateStruct();
        private bool _runDFLIMDataUpdateThread = true;

        #endregion Fields

        #region Constructors

        public DFLIMControlViewModel()
            : base()
        {
            _mainViewModelName = "CaptureSetupViewModel";
            _DFLIMControlModel = new DFLIMControlModel();

            System.Windows.Application.Current.Exit += Current_Exit;
            _dataUpdateThread = new Thread(DFLIMDataUpdate);
            _dataUpdateThread.Start();

            //TODO: remove HPD Gain if not needed
            HPDGain = new CustomCollection<HwVal<int>>();

            for (int i = 0; i < NUM_CHANNELS; i++)
            {

                HwVal<int> hpdGain = new HwVal<int>(i,
                                                    (int)Enum.Parse(typeof(SelectedHardware), string.Format("SELECTED_PMT{0}", i + 1)),
                                                    (int)Enum.Parse(typeof(IDevice.Params), string.Format("PARAM_HPD{0}_GAIN_VOLTS", i + 1)),
                                                    (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT);
                hpdGain.AdditionalSetLogic = (x, y) => SetGainAdditionalLogic(x, y);
                HPDGain.Add(hpdGain);
            }

            DFLIMHWControlsVisibility = Visibility.Visible;
            DFLIMFitVisibility = Visibility.Visible;
            _dFLIMROIHistogramsCallback = new ReportDFLIMROIHistograms(DFLIMROIHistogramsUpdate);

            _roiHistogramsDataUpdateStruct.roiHistograms = null;
        }

        #endregion Constructors

        #region Delegates

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        private delegate void ReportDFLIMROIHistograms(IntPtr roiHistograms, uint length, uint rois, uint tNum, int channelEnableBinary, uint numChannel);

        #endregion Delegates

        #region Properties

        public new bool ClosePropertyWindows
        {
            set
            {
                if (value)
                {
                    if (null != _dflimSetupAssistant)
                    {
                        _dflimSetupAssistant.Close();
                    }
                }

                base.ClosePropertyWindows = value;
            }
        }

        public int DFLIMAcquisitionMode
        {
            get
            {
                return _DFLIMControlModel.DFLIMAcquisitionMode;
            }
            set
            {
                _DFLIMControlModel.DFLIMAcquisitionMode = value;
                if (null != _dflimSetupAssistant)
                {
                    _dflimSetupAssistant.ShowHistrogram = 0 == value;
                    _dflimSetupAssistant.ShowDiagnostics = 1 == value;
                    OnPropertyChanged("DFLIMHistogramViewVisibility");
                    OnPropertyChanged("DFLIMDiagnosticsViewVisibility");
                }
                OnPropertyChanged("DFLIMAcquisitionMode");
            }
        }

        public int DFLIMCoarseShift1
        {
            get
            {
                return _DFLIMControlModel.DFLIMCoarseShift1;
            }
            set
            {
                _DFLIMControlModel.DFLIMCoarseShift1 = value;
                OnPropertyChanged("DFLIMCoarseShift1");
            }
        }

        public int DFLIMCoarseShift2
        {
            get
            {
                return _DFLIMControlModel.DFLIMCoarseShift2;
            }
            set
            {
                _DFLIMControlModel.DFLIMCoarseShift2 = value;
                OnPropertyChanged("DFLIMCoarseShift2");
            }
        }

        public int DFLIMCoarseShift3
        {
            get
            {
                return _DFLIMControlModel.DFLIMCoarseShift3;
            }
            set
            {
                _DFLIMControlModel.DFLIMCoarseShift3 = value;
                OnPropertyChanged("DFLIMCoarseShift3");
            }
        }

        public int DFLIMCoarseShift4
        {
            get
            {
                return _DFLIMControlModel.DFLIMCoarseShift4;
            }
            set
            {
                _DFLIMControlModel.DFLIMCoarseShift4 = value;
                OnPropertyChanged("DFLIMCoarseShift4");
            }
        }

        public Visibility DFLIMDiagnosticsViewVisibility
        {
            get
            {
                return DFLIMAcquisitionMode == 1 ? Visibility.Visible : Visibility.Collapsed;
            }
        }

        public ICommand DFLIMDisplaySetupAssitantCommand
        {
            get
            {
                if (_DFLIMDisplaySetupAssitantCommand == null)
                {
                    _DFLIMDisplaySetupAssitantCommand = new RelayCommand(() => DFLIMDisplaySetupAssitant());
                }

                return _DFLIMDisplaySetupAssitantCommand;
            }
        }

        public bool DFLIMEnableControlsThatNeedAcquistionOff
        {
            get
            {
                return _dflimEnableControlsThatNeedAcquistionOff;
            }
            set
            {
                _dflimEnableControlsThatNeedAcquistionOff = value;
                OnPropertyChanged("DFLIMEnableControlsThatNeedAcquistionOff");
            }
        }

        public int DFLIMFineShift1
        {
            get
            {
                return _DFLIMControlModel.DFLIMFineShift1;
            }
            set
            {
                _DFLIMControlModel.DFLIMFineShift1 = value;
                OnPropertyChanged("DFLIMFineShift1");
            }
        }

        public int DFLIMFineShift2
        {
            get
            {
                return _DFLIMControlModel.DFLIMFineShift2;
            }
            set
            {
                _DFLIMControlModel.DFLIMFineShift2 = value;
                OnPropertyChanged("DFLIMFineShift2");
            }
        }

        public int DFLIMFineShift3
        {
            get
            {
                return _DFLIMControlModel.DFLIMFineShift3;
            }
            set
            {
                _DFLIMControlModel.DFLIMFineShift3 = value;
                OnPropertyChanged("DFLIMFineShift3");
            }
        }

        public int DFLIMFineShift4
        {
            get
            {
                return _DFLIMControlModel.DFLIMFineShift4;
            }
            set
            {
                _DFLIMControlModel.DFLIMFineShift4 = value;
                OnPropertyChanged("DFLIMFineShift4");
            }
        }

        public double DFLIMFreqClock1
        {
            get
            {
                return _DFLIMControlModel.DFLIMFreqClock1;
            }
        }

        public double DFLIMFreqClock2
        {
            get
            {
                return _DFLIMControlModel.DFLIMFreqClock2;
            }
        }

        public double DFLIMFreqClock3
        {
            get
            {
                return _DFLIMControlModel.DFLIMFreqClock3;
            }
        }

        public double DFLIMFreqClock4
        {
            get
            {
                return _DFLIMControlModel.DFLIMFreqClock4;
            }
        }

        public Visibility DFLIMHistogramViewVisibility
        {
            get
            {
                return DFLIMAcquisitionMode == 0 ? Visibility.Visible : Visibility.Collapsed;
            }
        }

        public double DFLIMImpliedFreqClock1
        {
            get
            {
                return _DFLIMControlModel.DFLIMImpliedFreqClock1;
            }
        }

        public double DFLIMImpliedFreqClock2
        {
            get
            {
                return _DFLIMControlModel.DFLIMImpliedFreqClock2;
            }
        }

        public double DFLIMImpliedFreqClock3
        {
            get
            {
                return _DFLIMControlModel.DFLIMImpliedFreqClock3;
            }
        }

        public double DFLIMImpliedFreqClock4
        {
            get
            {
                return _DFLIMControlModel.DFLIMImpliedFreqClock4;
            }
        }

        public ICommand DFLIMQueryClockFrequenciesCommand
        {
            get
            {
                if (_DFLIMQueryClockFrequenciesCommand == null)
                {
                    _DFLIMQueryClockFrequenciesCommand = new RelayCommand(() => DFLIMQueryClockFrequencies());
                }

                return _DFLIMQueryClockFrequenciesCommand;
            }
        }

        public ICommand DFLIMReSyncCommand
        {
            get
            {
                if (_DFLIMReSyncCommand == null)
                {
                    _DFLIMReSyncCommand = new RelayCommand(() => DFLIMReSync());
                }

                return _DFLIMReSyncCommand;
            }
        }

        public int DFLIMResyncDelay
        {
            get
            {
                return _DFLIMControlModel.DFLIMResyncDelay;
            }
            set
            {
                _DFLIMControlModel.DFLIMResyncDelay = value;
                OnPropertyChanged("DFLIMResyncDelay");
            }
        }

        public int DFLIMResyncEveryLine
        {
            get
            {
                return _DFLIMControlModel.DFLIMResyncEveryLine;
            }
            set
            {
                _DFLIMControlModel.DFLIMResyncEveryLine = value;
                OnPropertyChanged("DFLIMResyncEveryLine");
            }
        }

        public ICommand DFLIMRevertSetupAssistantSettingChangesCommand
        {
            get
            {
                if (_DFLIMRevertSetupAssistantSettingChangesCommand == null)
                {
                    _DFLIMRevertSetupAssistantSettingChangesCommand = new RelayCommand(() => DFLIMRevertSetupAssistantSettingChanges());
                }

                return _DFLIMRevertSetupAssistantSettingChangesCommand;
            }
        }

        public int DFLIMSaveImagesOnLiveMode
        {
            get
            {
                return _DFLIMControlModel.DFLIMSaveImagesOnLiveMode;
            }
            set
            {
                _DFLIMControlModel.DFLIMSaveImagesOnLiveMode = value;
                OnPropertyChanged("DFLIMSaveImagesOnLiveMode");
            }
        }

        public int DFLIMSaveSettings
        {
            set
            {
                _DFLIMControlModel.DFLIMSaveSettings = value;
            }
        }

        public ICommand DFLIMSaveSettingsCommand
        {
            get
            {
                if (_DFLIMSaveSettingsCommand == null)
                {
                    _DFLIMSaveSettingsCommand = new RelayCommand(() => ExecuteDFLIMSaveSettings());
                }

                return _DFLIMSaveSettingsCommand;
            }
        }

        public int DFLIMSyncDelay
        {
            get
            {
                return _DFLIMControlModel.DFLIMSyncDelay;
            }
            set
            {
                _DFLIMControlModel.DFLIMSyncDelay = value;
                OnPropertyChanged("DFLIMSyncDelay");
            }
        }

        public int HPD1Gain
        {
            get
            {
                var gain = ((CustomCollection<HwVal<int>>)MVMManager.Instance["ScanControlViewModel", "PMTGain"]);
                return gain[0].Value;
            }
            set
            {
                var gain = ((CustomCollection<HwVal<int>>)MVMManager.Instance["ScanControlViewModel", "PMTGain"]);
                gain[0].Value = value;
                MVMManager.Instance["ScanControlViewModel", "PMTGain"] = gain;
                OnPropertyChanged("HPD1Gain");
            }
        }

        public int HPD2Gain
        {
            get
            {
                var gain = ((CustomCollection<HwVal<int>>)MVMManager.Instance["ScanControlViewModel", "PMTGain"]);
                return gain[1].Value;
            }
            set
            {
                var gain = ((CustomCollection<HwVal<int>>)MVMManager.Instance["ScanControlViewModel", "PMTGain"]);
                gain[1].Value = value;
                MVMManager.Instance["ScanControlViewModel", "PMTGain"] = gain;
                OnPropertyChanged("HPD2Gain");
            }
        }

        public int HPD3Gain
        {
            get
            {
                var gain = ((CustomCollection<HwVal<int>>)MVMManager.Instance["ScanControlViewModel", "PMTGain"]);
                return gain[2].Value;
            }
            set
            {
                var gain = ((CustomCollection<HwVal<int>>)MVMManager.Instance["ScanControlViewModel", "PMTGain"]);
                gain[2].Value = value;
                MVMManager.Instance["ScanControlViewModel", "PMTGain"] = gain;
                OnPropertyChanged("HPD3Gain");
            }
        }

        public int HPD4Gain
        {
            get
            {
                var gain = ((CustomCollection<HwVal<int>>)MVMManager.Instance["ScanControlViewModel", "PMTGain"]);
                return gain[3].Value;
            }
            set
            {
                var gain = ((CustomCollection<HwVal<int>>)MVMManager.Instance["ScanControlViewModel", "PMTGain"]);
                gain[3].Value = value;
                MVMManager.Instance["ScanControlViewModel", "PMTGain"] = gain;
                OnPropertyChanged("HPD4Gain");
            }
        }

        public CustomCollection<HwVal<int>> HPDGain
        {
            get;
            set;
        }

        #endregion Properties

        #region Methods

        [DllImport(".\\StatsManager.dll", EntryPoint = "GetNumROI")]
        public static extern int GetNumROI();

        public void HandleViewLoaded()
        {
            InitCallBackDFLIMROIHistogramsPush(_dFLIMROIHistogramsCallback);
            _dataUpdateThread = new Thread(DFLIMDataUpdate);
            _dataUpdateThread.Start();
        }

        public void HandleViewUnloaded()
        {
            InitCallBackDFLIMROIHistogramsPush(null);
            _dataUpdateThread.Abort();
        }

        [DllImport(".\\StatsManager.dll", EntryPoint = "InitCallBackDFLIMROIHistogramsPush")]
        private static extern void InitCallBackDFLIMROIHistogramsPush(ReportDFLIMROIHistograms reportDFLIMROIHistograms);

        void Current_Exit(object sender, ExitEventArgs e)
        {
            _runDFLIMDataUpdateThread = false;
        }

        void DFLIMDataUpdate()
        {
            do
            {
                try
                {
                    if (0 == DFLIMAcquisitionMode)
                    {
                        if (0 == GetNumROI())
                        {
                            CopyHistogram();
                        }
                        else
                        {
                            DFLIMUpdateFlimFit();
                        }
                    }
                    else if (1 == DFLIMAcquisitionMode)
                    {
                        CopyDiagnostics();
                    }

                    if (_dflimSetupAssistant != null)
                    {
                        Application.Current.Dispatcher.Invoke(() =>
                        {
                            if (_dflimSetupAssistant.IsLoaded)
                            {
                                if (0 == DFLIMAcquisitionMode && HistogramDataReady)
                                {
                                    _dflimSetupAssistant.PlotHistogram(DFLIMHistogramDictionary);
                                    HistogramDataReady = false;
                                }
                                else if (1 == DFLIMAcquisitionMode && DiagnosticsDataReady)
                                {
                                    _dflimSetupAssistant.PlotDiagnostics(DFLIMDiagnosticsDataDictionary);
                                    DiagnosticsDataReady = false;
                                }
                            }
                        });
                    }
                }
                catch (Exception ex)
                {
                    ex.ToString();
                }
                Thread.Sleep(100);
            } while (_runDFLIMDataUpdateThread);
        }

        private void DFLIMDisplaySetupAssitant()
        {
            if (null == _dflimSetupAssistant)
            {
                _dflimSetupAssistant = new SetupAssistant();
                _dflimSetupAssistant.DataContext = this;
            }

            if (false == _extractedStartupSettings)
            {
                _currentFineShift[0] = DFLIMFineShift1;
                _currentFineShift[1] = DFLIMFineShift2;
                _currentFineShift[2] = DFLIMFineShift3;
                _currentFineShift[3] = DFLIMFineShift4;

                _currentCoarseShift[0] = DFLIMCoarseShift1;
                _currentCoarseShift[1] = DFLIMCoarseShift2;
                _currentCoarseShift[2] = DFLIMCoarseShift3;
                _currentCoarseShift[3] = DFLIMCoarseShift4;

                _currentSyncDelay = DFLIMSyncDelay;
                _currentResyncDelay = DFLIMResyncDelay;

                _extractedStartupSettings = true;
            }

            DFLIMDiagnosticsBufferLengthSelectedIndex = 1;
            _dflimSetupAssistant.Closed += _dflimSetupAssistant_Closed;
            _dflimSetupAssistant.ShowHistrogram = DFLIMAcquisitionMode == 0;
            _dflimSetupAssistant.ShowDiagnostics = DFLIMAcquisitionMode == 1;
            _dflimSetupAssistant.Show();

            DFLIMQueryClockFrequencies();
        }

        private void DFLIMQueryClockFrequencies()
        {
            _DFLIMControlModel.DFLIMQueryClockFrequencies = 1;

            OnPropertyChanged("DFLIMFreqClock1");
            OnPropertyChanged("DFLIMFreqClock2");
            OnPropertyChanged("DFLIMFreqClock3");
            OnPropertyChanged("DFLIMFreqClock4");
            OnPropertyChanged("DFLIMImpliedFreqClock1");
            OnPropertyChanged("DFLIMImpliedFreqClock2");
            OnPropertyChanged("DFLIMImpliedFreqClock3");
            OnPropertyChanged("DFLIMImpliedFreqClock4");
        }

        private void DFLIMReSync()
        {
            _DFLIMControlModel.DFLIMReSync = 1;
        }

        private void DFLIMRevertSetupAssistantSettingChanges()
        {
            _currentFineShift[0] = DFLIMFineShift1 = _currentFineShift[0];
            _currentFineShift[1] = DFLIMFineShift2 = _currentFineShift[1];
            _currentFineShift[2] = DFLIMFineShift3 = _currentFineShift[2];
            _currentFineShift[3] = DFLIMFineShift4;

            _currentCoarseShift[0] = DFLIMCoarseShift1;
            _currentCoarseShift[1] = DFLIMCoarseShift2;
            _currentCoarseShift[2] = DFLIMCoarseShift3;
            _currentCoarseShift[3] = DFLIMCoarseShift4;

            _currentSyncDelay = DFLIMSyncDelay;
            _currentResyncDelay = DFLIMResyncDelay;
        }

        private void DFLIMROIHistogramsUpdate(IntPtr roiHistogramsPtr, uint length, uint roiNum, uint tNum, int channelEnableBinary, uint numChannel)
        {
            lock (_roiHistogramsDataLock)
            {
                try
                {
                    if (_roiHistogramsDataUpdateStruct.roiHistograms == null || _roiHistogramsDataUpdateStruct.roiHistograms.Length != length)
                    {
                        _roiHistogramsDataUpdateStruct.roiHistograms = new uint[length];
                    }

                    MemoryCopyManager.CopyIntPtrMemory(roiHistogramsPtr, _roiHistogramsDataUpdateStruct.roiHistograms, 0, _roiHistogramsDataUpdateStruct.roiHistograms.Length);

                    _roiHistogramsDataUpdateStruct.roiNum = roiNum;
                    _roiHistogramsDataUpdateStruct.tNum = tNum;
                    _roiHistogramsDataUpdateStruct.channelEnableBinary = channelEnableBinary;
                    _roiHistogramsDataUpdateStruct.numChannel = numChannel;

                    _newROIHistogramData = true;
                }
                catch (Exception ex)
                {
                    ex.ToString();
                }
            }
        }

        void DFLIMTauHigh_CollectionChanged(object sender, System.Collections.Specialized.NotifyCollectionChangedEventArgs e)
        {
            if (DFLIMDisplayLifetimeImage)
            {
                MVMManager.Instance["CaptureSetupViewModel", "RebuildBitmap"] = true;
                ((ThorSharedTypes.IMVM)MVMManager.Instance["CaptureSetupViewModel", this]).OnPropertyChange("Bitmap");
            }
        }

        private void DFLIMUpdateFlimFit()
        {
            lock (_roiHistogramsUpdateFunctionLock)
            {
                if (string.IsNullOrWhiteSpace(_mainViewModelName))
                {
                    return;
                }
                if (null != MVMManager.Instance[_mainViewModelName])
                {
                    try
                    {
                        if (_newROIHistogramData)
                        {
                            _newROIHistogramData = false;
                            var roiHistogramsDataUpdateStruct = new ROIHistogramsDataUpdateStruct();

                            lock (_roiHistogramsDataLock)
                            {
                                roiHistogramsDataUpdateStruct.roiHistograms = new uint[_roiHistogramsDataUpdateStruct.roiHistograms.Length];
                                Array.Copy(_roiHistogramsDataUpdateStruct.roiHistograms, 0, roiHistogramsDataUpdateStruct.roiHistograms, 0, _roiHistogramsDataUpdateStruct.roiHistograms.Length);
                                roiHistogramsDataUpdateStruct.channelEnableBinary = _roiHistogramsDataUpdateStruct.channelEnableBinary;
                                roiHistogramsDataUpdateStruct.numChannel = _roiHistogramsDataUpdateStruct.numChannel;
                                roiHistogramsDataUpdateStruct.roiNum = _roiHistogramsDataUpdateStruct.roiNum;
                                roiHistogramsDataUpdateStruct.tNum = _roiHistogramsDataUpdateStruct.tNum;
                            }

                            uint[][] dflimHistogramData = new uint[roiHistogramsDataUpdateStruct.numChannel][];

                            for (int i = 0; i < dflimHistogramData.Length; ++i)
                            {
                                int offset = (int)((roiHistogramsDataUpdateStruct.roiNum + 1) * roiHistogramsDataUpdateStruct.tNum * i);
                                dflimHistogramData[i] = new uint[roiHistogramsDataUpdateStruct.tNum];
                                Array.Copy(roiHistogramsDataUpdateStruct.roiHistograms, offset, dflimHistogramData[i], 0, roiHistogramsDataUpdateStruct.tNum);
                            }

                            var lsmChannelEnable = roiHistogramsDataUpdateStruct.channelEnableBinary;

                            Brush[] lsmChannelColor = (Brush[])MVMManager.Instance[_mainViewModelName, "LSMChannelColor"];

                            List<int> channels = new List<int>();
                            for (int i = 0; i < NUM_CHANNELS; ++i)
                            {
                                if (((lsmChannelEnable >> i) & 0x1) > 0)
                                {
                                    channels.Add(i);
                                }
                            }

                            //FLIMFitting.FLIMHistogramGroupData histogramGroupData = null;
                            lock (DFLIMHistogramMVMDataLock)
                            {
                                OnPropertyChanged("DFLIMHistogramMVMDataLock");
                                _dflimHistogramDictionary = new Dictionary<KeyValuePair<int, SolidColorBrush>, uint[]>();
                                for (int i = 0; i < dflimHistogramData.Length; ++i)
                                {
                                    if (NUM_CHANNELS == dflimHistogramData.Length)
                                    {
                                        if (((lsmChannelEnable >> i) & 0x1) > 0)
                                        {
                                            var channel = new KeyValuePair<int, SolidColorBrush>(i, (SolidColorBrush)lsmChannelColor[i]);
                                            _dflimHistogramDictionary.Add(channel, (dflimHistogramData[i]));
                                        }
                                    }
                                    else
                                    {

                                        var channel = new KeyValuePair<int, SolidColorBrush>(channels[i], (SolidColorBrush)lsmChannelColor[channels[i]]);
                                        _dflimHistogramDictionary.Add(channel, (dflimHistogramData[i]));
                                    }
                                }
                                ++_histogramCopyCounter;
                                _histogramDataReady = true;
                            }
                            OnPropertyChanged("DFLIMHistogramMVMDataLock");
                            OnPropertyChanged("DFLIMHistogramCopyCounter");
                            OnPropertyChanged("DFLIMHistogramDictionary");

                            try
                            {
                                if (null != _flimFittingWindow)
                                {
                                    _flimFittingWindow.FLIMHistogramGroups = PrepareFLIMFitROIHistogramData(roiHistogramsDataUpdateStruct);
                                }
                            }
                            catch (Exception ex)
                            {
                                ex.ToString();
                            }
                        }
                    }
                    catch (Exception ex)
                    {
                        ex.ToString();
                    }
                }
            }
        }

        private void ExecuteDFLIMSaveSettings()
        {
            var result = MessageBox.Show("Do you wish to permanently save these Shift and Sync settings?", "", MessageBoxButton.OKCancel);
            if (MessageBoxResult.OK == result)
            {
                DFLIMSaveSettings = 1;

                _currentFineShift[0] = DFLIMFineShift1;
                _currentFineShift[1] = DFLIMFineShift2;
                _currentFineShift[2] = DFLIMFineShift3;
                _currentFineShift[3] = DFLIMFineShift4;

                _currentCoarseShift[0] = DFLIMCoarseShift1;
                _currentCoarseShift[1] = DFLIMCoarseShift2;
                _currentCoarseShift[2] = DFLIMCoarseShift3;
                _currentCoarseShift[3] = DFLIMCoarseShift4;

                _currentSyncDelay = DFLIMSyncDelay;
                _currentResyncDelay = DFLIMResyncDelay;
            }
        }

        private Brush GetROIColor(int colorIndex)
        {
            switch (colorIndex)
            {
                case 0:
                    {
                        return Brushes.Yellow;
                    }
                case 1:
                    {
                        return Brushes.Lime;
                    }
                case 2:
                    {
                        return Brushes.DodgerBlue;
                    }
                case 3:
                    {
                        return Brushes.DeepPink;
                    }
                case 4:
                    {
                        return Brushes.DarkOrange;
                    }
                case 5:
                    {
                        return Brushes.Khaki;
                    }
                case 6:
                    {
                        return Brushes.LightGreen;
                    }
                case 7:
                    {
                        return Brushes.SteelBlue;
                    }
                default:
                    {
                        return Brushes.Black;
                    }
            }
        }

        private List<FLIMFitting.FLIMHistogramGroupData> PrepareFLIMFitROIHistogramData(ROIHistogramsDataUpdateStruct roiHistogramsDataUpdateStruct)
        {
            try
            {
                if (roiHistogramsDataUpdateStruct.roiHistograms.Length <= 0)
                {
                    return null;
                }

                var lsmChannelEnable = roiHistogramsDataUpdateStruct.channelEnableBinary;

                var histoGroups = new List<FLIMFitting.FLIMHistogramGroupData>();

                int i = 0;
                Brush[] lsmChannelColor = (Brush[])MVMManager.Instance[_mainViewModelName, "LSMChannelColor"];
                for (int c = 0; c < NUM_CHANNELS; ++c)
                {
                    if (((lsmChannelEnable >> c) & 0x1) > 0)
                    {
                        var histogramGroupData = new FLIMFitting.FLIMHistogramGroupData();
                        histogramGroupData.GroupType = FLIMFitting.HistogramGroupType.ChannelHistogramAndROIs;

                        switch (c)
                        {
                            case 0: histogramGroupData.GroupName = "Channel A"; histogramGroupData.HistrogramNames.Add("A"); break;
                            case 1: histogramGroupData.GroupName = "Channel B"; histogramGroupData.HistrogramNames.Add("B"); break;
                            case 2: histogramGroupData.GroupName = "Channel C"; histogramGroupData.HistrogramNames.Add("C"); break;
                            case 3: histogramGroupData.GroupName = "Channel D"; histogramGroupData.HistrogramNames.Add("D"); break;
                            case 4: histogramGroupData.GroupName = "Channel E"; histogramGroupData.HistrogramNames.Add("E"); break;
                            case 5: histogramGroupData.GroupName = "Channel F"; histogramGroupData.HistrogramNames.Add("F"); break;
                            default: histogramGroupData.GroupName = string.Empty; break;
                        }
                        histogramGroupData.Channels.Add(c);

                        int offsetC = (int)((roiHistogramsDataUpdateStruct.roiNum + 1) * roiHistogramsDataUpdateStruct.tNum * i);

                        uint[] channelHistogram = new uint[roiHistogramsDataUpdateStruct.tNum];
                        Array.Copy(roiHistogramsDataUpdateStruct.roiHistograms, offsetC, channelHistogram, 0, roiHistogramsDataUpdateStruct.tNum);
                        histogramGroupData.Colors.Add((SolidColorBrush)lsmChannelColor[c]);
                        histogramGroupData.Histograms.Add(channelHistogram);
                        for (int r = 1; r <= roiHistogramsDataUpdateStruct.roiNum; ++r)
                        {
                            histogramGroupData.Channels.Add(c);
                            histogramGroupData.Colors.Add((SolidColorBrush)GetROIColor((r - 1) % 8));
                            uint[] roiHistogram = new uint[roiHistogramsDataUpdateStruct.tNum];
                            int offsetR = (int)((roiHistogramsDataUpdateStruct.roiNum + 1) * roiHistogramsDataUpdateStruct.tNum * i + r * roiHistogramsDataUpdateStruct.tNum);
                            Array.Copy(roiHistogramsDataUpdateStruct.roiHistograms, offsetR, roiHistogram, 0, roiHistogramsDataUpdateStruct.tNum);
                            histogramGroupData.Histograms.Add(roiHistogram);

                            histogramGroupData.HistrogramNames.Add("R" + (r).ToString());
                        }
                        histoGroups.Add(histogramGroupData);

                        ++i;
                    }
                }

                return histoGroups;
            }
            catch (Exception ex)
            {
                ex.ToString();
                return null;
            }
        }

        private void SetGainAdditionalLogic(int index, int val)
        {
            if (0 > val)
            {
                HPDGain[index].Value = 0;
                return;
            }
            if (8500 < val)
            {
                HPDGain[index].Value = 8500;
                return;
            }

            if (true == (bool)MVMManager.Instance["CaptureSetupViewModel", "IsLive", (object)false])
            {
                var PMTGainEnable = ((CustomCollection<HwVal<int>>)MVMManager.Instance["ScanControlViewModel", "PMTGainEnable"]);
                PMTGainEnable[index].Value = (0 == val) ? 0 : 1;
                MVMManager.Instance["ScanControlViewModel", "PMTGainEnable"] = PMTGainEnable;
            }
        }

        void _dflimSetupAssistant_Closed(object sender, EventArgs e)
        {
            _dflimSetupAssistant = null;

            if (0 != DFLIMAcquisitionMode || 0 != DFLIMResyncEveryLine)
            {
                //stop the current capture
                ((ICommand)MVMManager.Instance["CaptureSetupViewModel", "StopCommand"]).Execute(null);

                //Ensure acquisition mode is DFLIM after closing setup assistant
                DFLIMAcquisitionMode = 0;

                DFLIMResyncEveryLine = 0;
            }
        }

        #endregion Methods

        #region Nested Types

        private struct ROIHistogramsDataUpdateStruct
        {
            #region Fields

            public int channelEnableBinary;
            public uint numChannel;
            public uint[] roiHistograms;
            public uint roiNum;
            public uint tNum;

            #endregion Fields
        }

        #endregion Nested Types
    }

    public static class ObjectExtensions
    {
        #region Methods

        public static uint[] Copy(uint[] pieces)
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

        public static ushort[] Copy(ushort[] pieces)
        {
            return pieces.Select(x =>
            {
                var handle = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(ushort)));

                try
                {
                    Marshal.StructureToPtr(x, handle, false);
                    return (ushort)Marshal.PtrToStructure(handle, typeof(ushort));
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