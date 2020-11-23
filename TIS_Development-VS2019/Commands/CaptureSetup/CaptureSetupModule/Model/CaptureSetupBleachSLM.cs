namespace CaptureSetupDll.Model
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Timers;
    using System.Windows;
    using System.Windows.Data;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Threading;
    using System.Xml;

    using GeometryUtilities;

    using ThorLogging;

    using ThorSharedTypes;

    public partial class CaptureSetup : INotifyPropertyChanged
    {
        #region Fields

        const int WAITUNIT_MS = 5000;

        private static ReportBleachSLMNowFinished _BleachNowSLMFinishedCallBack;
        private static PreBleachSLMCallback _PreBleachSLMCallBack;

        private int _currentSLMCycleID = 0;
        private int _currentSLMSequenceID = 0;
        private int _currentSLMWaveID = 0;
        private int _epochCount;
        private bool _lastInSLMCycle = false;
        private List<string> _loadedSLMFiles = new List<string>();
        private List<string> _loadedSLMPatterns = new List<string>();
        private int _loadedSLMPatternsCnt = 0;
        private List<string> _loadedSLMSequences = new List<string>();
        private double _slmBleachDelay = 0;
        private ObservableCollection<SLMParams> _slmBleachWaveParams = new ObservableCollection<GeometryUtilities.SLMParams>();
        private List<string> _slmFilesInFolder;
        private bool _slmSequenceOn = false;
        private List<string> _slmSequencesInFolder;

        #endregion Fields

        #region Delegates

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void PreBleachSLMCallback(ref int status);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void ReportBleachSLMNowFinished();

        #endregion Delegates

        #region Properties

        public int EpochCount
        {
            get
            {
                return Math.Max(1, _epochCount);
            }
            set
            {
                _epochCount = value;
            }
        }

        public bool IsStimulator
        {
            get { return ((int)ICamera.LSMType.STIMULATE_MODULATOR == ResourceManagerCS.GetBleacherType()); }
        }

        public bool SLM3D
        {
            get
            {
                int val = 0;
                ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_SLM, (int)IDevice.Params.PARAM_SLM_3D, ref val);
                return (1 == val);
            }
            set
            {
                int val = value ? 1 : 0;
                ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_SLM, (int)IDevice.Params.PARAM_SLM_3D, val, (int)IDevice.DeviceSetParamType.NO_EXECUTION);
            }
        }

        public double SLMBleachDelay
        {
            get { return _slmBleachDelay; }
            set
            {
                Decimal val = Decimal.Round((Decimal)value, 3);
                _slmBleachDelay = (double)val;
                OnPropertyChanged("SLMBleachDelay");
            }
        }

        public ObservableCollection<SLMParams> SLMBleachWaveParams
        {
            get
            { return _slmBleachWaveParams; }
            set
            {
                _slmBleachWaveParams = value;
                OnPropertyChanged("SLMBleachWaveParams");
            }
        }

        public bool SLMPhaseDirect
        {
            get
            {
                int phaseDirect = 0;
                ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_SLM, (int)IDevice.Params.PARAM_SLM_PHASE_DIRECT, ref phaseDirect);
                return (1 == phaseDirect);
            }
            set
            {
                int phaseDirect = value ? 1 : 0;
                ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_SLM, (int)IDevice.Params.PARAM_SLM_PHASE_DIRECT, phaseDirect, (int)IDevice.DeviceSetParamType.NO_EXECUTION);
            }
        }

        public int[] SLMPixelXY
        {
            get
            {
                int[] val = new int[2] { 0, 0 };
                ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_SLM, (int)IDevice.Params.PARAM_SLM_PIXEL_X, ref val[0]);
                ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_SLM, (int)IDevice.Params.PARAM_SLM_PIXEL_X, ref val[1]);
                return val;
            }
        }

        public bool SLMSelectWavelength
        {
            get
            {
                int val = -1;
                ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_SLM, (int)IDevice.Params.PARAM_SLM_WAVELENGTH_SELECT, ref val);
                return (1 == val);
            }
            set
            {
                ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_SLM, (int)IDevice.Params.PARAM_SLM_WAVELENGTH_SELECT, value ? 1 : 0, (int)IDevice.DeviceSetParamType.NO_EXECUTION);
            }
        }

        public bool SLMSequenceOn
        {
            get
            {
                return _slmSequenceOn;
            }
            set
            {
                _slmSequenceOn = value;
            }
        }

        public string[] SLMWaveBaseName
        {
            get { return new string[] { "SLMWaveform", "SLMPattern", "SLMSequence" }; }    //generic H5 waveform name, generic phase mask name
        }

        /// <summary>
        /// SLM folder path Waveforms[0], Sequences[1]
        /// </summary>
        public string[] SLMWaveformFolder
        {
            get
            {
                return new string[] {
                ResourceManagerCS.GetCaptureTemplatePathString() + "SLMWaveforms",
                ResourceManagerCS.GetCaptureTemplatePathString() + "SLMWaveforms\\SLMSequences" };
            }
        }

        public int SLMWavelengthCount
        {
            get
            {
                double[] val = new double[2] { 0, 0 };
                ResourceManagerCS.GetDeviceParamBuffer<double>((int)SelectedHardware.SELECTED_SLM, (int)IDevice.Params.PARAM_SLM_WAVELENGTH, val, (int)Constants.MAX_WIDEFIELD_WAVELENGTH_COUNT);
                return (0 < val[1] ? 2 : (0 < val[0] ? 1 : 0));
            }
        }

        public int SLMWavelengthNM
        {
            get
            {
                double[] val = new double[2] { 0, 0 };
                ResourceManagerCS.GetDeviceParamBuffer<double>((int)SelectedHardware.SELECTED_SLM, (int)IDevice.Params.PARAM_SLM_WAVELENGTH, val, (int)Constants.MAX_WIDEFIELD_WAVELENGTH_COUNT);
                return (int)val[SLMSelectWavelength ? 1 : 0];
            }
        }

        #endregion Properties

        #region Methods

        public bool CombineHolograms(string bmpPhaseName1, string bmpPhaseName2)
        {
            return ((1 == CombineHologramFiles(bmpPhaseName1, bmpPhaseName2)) ? true : false);
        }

        public void InitializeWaveformBuilder(int clockRateHz)
        {
            WaveformBuilder.ClkRate = clockRateHz;
            WaveformBuilder.Field2Volts = (double)MVMManager.Instance["AreaControlViewModel", "LSMField2Theta", (object)1.0];
            WaveformBuilder.InitializeParams(BleachCalibrateFieldSize, BleachCalibrateFineScaleXY, BleachCalibratePixelXY, BleachCalibrateOffsetXY, BleachCalibrateFineOffsetXY, BleachCalibrateScaleYScan,
                BleachCalibrateFlipHV[1], BleachCalibrateFlipHV[0], BleachCalibratePockelsVoltageMin0, WaveformDriverType);
        }

        public bool LoadSLMPatternName(int runtimeCal, int id, string bmpPatternName, bool start, int timeoutVal = 0)
        {
            return ((1 == LoadSLMPattern(runtimeCal, id, bmpPatternName, (start) ? 1 : 0, timeoutVal)) ? true : false);
        }

        public bool ResetSLMCalibration()
        {
            return ((1 == ResetAffineCalibration()) ? true : false);
        }

        public bool SaveSLMPatternName(string phaseMaskName)
        {
            return ((1 == SaveSLMPattern(phaseMaskName)) ? true : false);
        }

        public bool SLMCalibration(string bmpPatternName, float[] ptsFrom, float[] ptsTo, int size, int pixelX, int pixelY, double z)
        {
            IntPtr ptsToPtr = Marshal.AllocHGlobal(size * sizeof(float));
            IntPtr ptsFromPtr = Marshal.AllocHGlobal(size * sizeof(float));
            Marshal.Copy(ptsTo, 0, ptsToPtr, size);
            Marshal.Copy(ptsFrom, 0, ptsFromPtr, size);
            int ret = CalibrateSLM(bmpPatternName, ptsFromPtr, ptsToPtr, size, pixelX, pixelY, z);

            Marshal.FreeHGlobal(ptsToPtr);
            Marshal.FreeHGlobal(ptsFromPtr);

            return (1 == ret) ? true : false;
        }

        public bool SLMSetBlank()
        {
            return ((1 == SetSLMBlank()) ? true : false);
        }

        public bool StartSLMBleach()
        {
            bool ret = true;

            //ensure the buffer is copied after the capture
            _pixelDataReady = false;

            //before start SLM bleach:
            _currentSLMWaveID = _currentSLMCycleID = _currentSLMSequenceID = 0;
            _lastInSLMCycle = false;
            _loadedSLMPatterns.Clear();
            _loadedSLMFiles.Clear();
            _loadedSLMSequences.Clear();

            //initialize bleach before start:
            InitializeBleach();

            //try load all SLM patterns, continue even none is loaded:
            PreLoadAllSLMPatterns();

            //user asked to stop while sleep or file is not loaded:
            if (IsBleachStopped)
            {
                IsBleachFinished = true;
                return false;
            }

            //run bleach
            ret = (1 == SLMBleach()) ? true : false;

            return ret;
        }

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "CalibrateSLM", CharSet = CharSet.Unicode)]
        private static extern int CalibrateSLM(string bmpPatternName, IntPtr xyPointFrom, IntPtr xyPointTo, int size, int pixelX, int pixelY, double z);

        [DllImport(".\\Modules_Native\\HologramGenerator.dll", EntryPoint = "CombineHologramFiles", CharSet = CharSet.Unicode)]
        private static extern int CombineHologramFiles(string bmpPhaseName1, string bmpPhaseName2);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "InitCallBackBleachSLM")]
        private static extern void InitCallBackBleachSLM(ReportBleachSLMNowFinished reportBleachSLMNowFinished, PreBleachSLMCallback preBleachSLMCallback);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "InitializeBleach")]
        private static extern void InitializeBleach();

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "LoadSLMPattern", CharSet = CharSet.Unicode)]
        private static extern int LoadSLMPattern(int runtimeCal, int id, string bmpPatternName, int doStart, int timeout);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "ResetAffineCalibration")]
        private static extern int ResetAffineCalibration();

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "SaveSLMPattern", CharSet = CharSet.Unicode)]
        private static extern int SaveSLMPattern(string bmpPhaseName);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "SetSLMBlank")]
        private static extern int SetSLMBlank();

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "SLMBleach")]
        private static extern int SLMBleach();

        /// <summary>
        /// communicate with SLM bleach callback.
        /// </summary>
        /// <param name="status"></param>
        private void BleachSLMCallback(ref int status)
        {
            //user request stop:
            if (IsBleachStopped)
            {
                IsBleachFinished = true;
                status = (int)PreCaptureStatus.PRECAPTURE_DONE;
                return;
            }

            if (((0 < _currentSLMWaveID) || (0 < _currentSLMCycleID)) && (!GetIsBleaching()))
            {
                //return if not bleaching:
                status = (int)PreCaptureStatus.PRECAPTURE_DONE;
                IsBleaching = false;
                return;
            }

            //Sleep delay time:
            if (_lastInSLMCycle && 0 < SLMBleachDelay)
            {
                int timeWait = (int)(WaveformBuilder.MS_TO_S * SLMBleachDelay);
                //check status every 5 seconds
                while (timeWait > 0)
                {
                    BleachLastRunTime = DateTime.Now;
                    System.Threading.Thread.Sleep(((WAITUNIT_MS < timeWait) ? WAITUNIT_MS : timeWait));
                    //check if bleaching:
                    if (!GetIsBleaching())
                    {
                        status = (int)PreCaptureStatus.PRECAPTURE_DONE;
                        IsBleaching = false;
                        return;
                    }
                    timeWait -= (int)(new TimeSpan(DateTime.Now.Ticks - BleachLastRunTime.Ticks).TotalMilliseconds);
                }
            }

            //go through cycles: the last one if no next file to load
            if (!LoadSLMPatternAndFile())
            {
                status = (int)PreCaptureStatus.PRECAPTURE_DONE;
            }
            else if ((1 == BleachFrames) && (1 >= _slmFilesInFolder.Count))  //single waveform, single cycle
            {
                status = (int)PreCaptureStatus.PRECAPTURE_WAVEFORM_LAST_CYCLE;
            }
            else
            {
                //last if last waveform of all cycles
                status = (_lastInSLMCycle && (BleachFrames - 1) <= _currentSLMCycleID) ?
                    (int)PreCaptureStatus.PRECAPTURE_WAVEFORM_LAST_CYCLE : (int)PreCaptureStatus.PRECAPTURE_WAVEFORM_MID_CYCLE;
            }
        }

        /// <summary>
        /// complete tasks to do while done SLM bleaching.
        /// </summary>
        private void BleachSLMNowFinished()
        {
            if (true == IsBleaching)
            {
                IsBleaching = false;
                IsBleachFinished = true;
            }
        }

        /// <summary>
        /// load additional SLM pattern, waveform and sequence per cycle callback, use waveform indexes as reference.
        /// </summary>
        /// <param name="lastInCycle"></param>
        /// <returns></returns>
        bool LoadSLMPatternAndFile()
        {
            PreLoadNextSLMPattern();
            string LoadedSLMName = PreLoadNextSLM();
            LoadedSLMName = (PreLoadNextSLMSequence()) ? LoadedSLMName : string.Empty;

            //check if loaded:
            if (string.Empty == LoadedSLMName)
            {
                //no more added files located, try next cycle:
                _currentSLMWaveID = _currentSLMSequenceID = 0;
                _currentSLMCycleID++;

                _loadedSLMFiles.Clear();
                _loadedSLMSequences.Clear();

                LoadedSLMName = PreLoadNextSLM();
                LoadedSLMName = (PreLoadNextSLMSequence()) ? LoadedSLMName : string.Empty;
            }

            return (string.Empty == LoadedSLMName) ? false : true;
        }

        /// <summary>
        /// load available SLM patterns all at once, return true if loaded any.
        /// </summary>
        /// <returns></returns>
        private bool PreLoadAllSLMPatterns()
        {
            //reset counts:
            _loadedSLMPatternsCnt = 0;

            //get files in folder:
            List<string> slmPatternsInFolder = Directory.EnumerateFiles(SLMWaveformFolder[0], "*.bmp ", SearchOption.TopDirectoryOnly).ToList();

            //locate filenames to load from the list:
            for (int i = 0; i < _slmBleachWaveParams.Count; i++)
            {
                string waveFileName = SLMWaveformFolder[0] + "\\" + SLMWaveBaseName[1] + "_" + _slmBleachWaveParams[i].BleachWaveParams.ID.ToString("D" + FileName.GetDigitCounts().ToString()) + ".bmp";

                string matchingFileName = slmPatternsInFolder.FirstOrDefault(checkString => checkString.Contains(waveFileName));
                if (null != matchingFileName)
                {
                    _loadedSLMPatternsCnt++;
                    _loadedSLMPatterns.Add(waveFileName);
                }
            }

            //check if other files added:
            List<string> addedFiles = slmPatternsInFolder.Except(_loadedSLMPatterns).OrderBy(s => s).ToList();
            for (int i = 0; i < addedFiles.Count; i++)
            {
                _loadedSLMPatternsCnt++;
                _loadedSLMPatterns.Add(addedFiles[i]);
            }

            //find out maximum pattern time for SLM timeout in msec:
            int timeoutVal = 0;
            for (int i = 0; i < _slmBleachWaveParams.Count; i++)
            {
                timeoutVal = (int)Math.Max(timeoutVal, _slmBleachWaveParams[i].BleachWaveParams.PrePatIdleTime + _slmBleachWaveParams[i].BleachWaveParams.Iterations * (_slmBleachWaveParams[i].BleachWaveParams.PreIdleTime + _slmBleachWaveParams[i].Duration + _slmBleachWaveParams[i].BleachWaveParams.PostIdleTime) + _slmBleachWaveParams[i].BleachWaveParams.PostPatIdleTime);
            }

            //load to SLM:
            for (int i = 0; i < _loadedSLMPatternsCnt; i++)
            {
                //user asked to stop while loading:
                if (IsBleachStopped)
                {
                    IsBleachFinished = true;
                    StopBleach();
                    return false;
                }
                bool doStart = (i == (_loadedSLMPatternsCnt - 1)) ? true : false;
                this.LoadSLMPatternName(_slmSequenceOn ? 1 : 0, i, _loadedSLMPatterns[i], doStart, timeoutVal);
            }

            return (0 < _loadedSLMPatternsCnt) ? true : false;
        }

        /// <summary>
        /// load next SLM bleach waveform, return loaded file name and notice if it is the last available in current cycle
        /// </summary>
        /// <param name="lastInCycle"></param>
        /// <returns></returns>
        private string PreLoadNextSLM()
        {
            string LoadedSLMName = string.Empty;

            //get files in folder:
            _slmFilesInFolder = Directory.EnumerateFiles((_slmSequenceOn ? SLMWaveformFolder[1] : SLMWaveformFolder[0]), "*.raw ", SearchOption.TopDirectoryOnly).ToList();

            //locate first filename to load from the list:
            int targetCount = _slmSequenceOn ? _slmFilesInFolder.Count : _slmBleachWaveParams.Count;
            while ((string.Empty == LoadedSLMName) && (targetCount > _currentSLMWaveID))
            {
                string waveFileName = (_slmSequenceOn ? SLMWaveformFolder[1] : SLMWaveformFolder[0]) + "\\" + SLMWaveBaseName[0] + "_" +
                    (_slmSequenceOn ? (uint)(_currentSLMWaveID + 1) : _slmBleachWaveParams[_currentSLMWaveID].BleachWaveParams.ID).ToString("D" + FileName.GetDigitCounts().ToString()) + ".raw";

                //check if available in the folder, skip if just loaded:
                string matchingFileName = _slmFilesInFolder.FirstOrDefault(checkString => checkString.Contains(waveFileName));
                if (null != matchingFileName)
                {
                    if (this.LoadBleachWaveformFile(matchingFileName, 1))   //set cycle count = 1 to reload waveform everytime
                    {
                        LoadedSLMName = waveFileName;
                        _loadedSLMFiles.Add(LoadedSLMName);
                    }
                }
                _currentSLMWaveID++;
                _lastInSLMCycle = (targetCount <= _currentSLMWaveID) ? true : false;
            }

            //if not loaded from the list, check if other files added:
            if (string.Empty == LoadedSLMName)
            {
                List<string> addedFiles = _slmFilesInFolder.Except(_loadedSLMFiles).OrderBy(s => s).ToList();
                if (0 < addedFiles.Count())
                {
                    if (this.LoadBleachWaveformFile(addedFiles[0], 1))
                    {
                        LoadedSLMName = addedFiles[0];
                        _loadedSLMFiles.Add(LoadedSLMName);
                    }
                }
                _lastInSLMCycle = (1 >= addedFiles.Count()) ? true : false;
            }

            return LoadedSLMName;
        }

        private bool PreLoadNextSLMPattern()
        {
            //force to update pattern, user need to risk missing triggers:
            int currentCnt = _loadedSLMPatternsCnt;

            //get files in folder:
            List<string> slmPatternsInFolder = Directory.EnumerateFiles(SLMWaveformFolder[0], "*.bmp ", SearchOption.TopDirectoryOnly).ToList();

            //check if other files added:
            List<string> addedFiles = slmPatternsInFolder.Except(_loadedSLMPatterns).OrderBy(s => s).ToList();
            for (int i = 0; i < addedFiles.Count; i++)
            {
                if (this.LoadSLMPatternName(_slmSequenceOn ? 1 : 0, _loadedSLMPatternsCnt, addedFiles[i], true))
                {
                    _loadedSLMPatternsCnt++;
                    _loadedSLMPatterns.Add(addedFiles[i]);
                }
            }

            return (currentCnt < _loadedSLMPatternsCnt) ? true : false;
        }

        /// <summary>
        /// load next SLM Sequence, return true if loaded new and in sync with PreLoadNextSLM
        /// </summary>
        /// <returns></returns>
        private bool PreLoadNextSLMSequence()
        {
            //return if not in advance sequence mode
            if (!_slmSequenceOn)
                return true;

            string LoadedSequenceName = string.Empty;

            //get files in folder:
            _slmSequencesInFolder = Directory.EnumerateFiles(SLMWaveformFolder[1], "*.txt ", SearchOption.TopDirectoryOnly).ToList();

            //locate first filename to load from the list:
            while ((string.Empty == LoadedSequenceName) && (_slmSequencesInFolder.Count > _currentSLMSequenceID))
            {
                string seqFileName = SLMWaveformFolder[1] + "\\" + SLMWaveBaseName[2] + "_" + (_currentSLMSequenceID + 1).ToString("D" + FileName.GetDigitCounts().ToString()) + ".txt";

                //check if available in the folder, skip if just loaded:
                string matchingFileName = _slmSequencesInFolder.FirstOrDefault(checkString => checkString.Contains(seqFileName));
                if (null != matchingFileName)
                {
                    if (1 == ResourceManagerCS.SetDeviceParamString((int)SelectedHardware.SELECTED_SLM, (int)IDevice.Params.PARAM_SLM_SEQ_FILENAME, matchingFileName, (int)IDevice.DeviceSetParamType.NO_EXECUTION))
                    {
                        LoadedSequenceName = seqFileName;
                        _loadedSLMSequences.Add(LoadedSequenceName);
                    }
                }
                _currentSLMSequenceID++;
            }

            //if not loaded from the list, check if other files added:
            if (string.Empty == LoadedSequenceName)
            {
                List<string> addedFiles = _slmSequencesInFolder.Except(_loadedSLMSequences).OrderBy(s => s).ToList();
                if (0 < addedFiles.Count())
                {
                    if (1 == ResourceManagerCS.SetDeviceParamString((int)SelectedHardware.SELECTED_SLM, (int)IDevice.Params.PARAM_SLM_SEQ_FILENAME, addedFiles[0], (int)IDevice.DeviceSetParamType.NO_EXECUTION))
                    {
                        LoadedSequenceName = addedFiles[0];
                        _loadedSLMSequences.Add(LoadedSequenceName);
                    }
                }
            }
            return (string.Empty == LoadedSequenceName) ? false : true;
        }

        #endregion Methods
    }
}