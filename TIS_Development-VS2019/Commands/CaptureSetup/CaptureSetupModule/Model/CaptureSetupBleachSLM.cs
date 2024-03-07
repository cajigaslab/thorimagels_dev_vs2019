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
    using System.Threading;
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

        //private bool _afterSLMCycle = false; //if item found after a SLM repeat or cycle
        private int _currentSLMCycleID = 0;
        private int _currentSLMSequenceID = 0;
        private int _currentSLMWaveID = 0;
        private double[] _defocusParams = { 0.0, 1.33, 4.5, 0.0 }; //[0]:defocus z in [um], [1]:refractive index, [2]:effective focal length in [mm], [3]:saved defocus z in [um]
        private int _epochCount;
        private bool _lastInSLMCycle = false; //the last item in a SLM repeat or cycle
        private List<string> _loadedSLMFiles = new List<string>();
        private List<string> _loadedSLMPatterns = new List<string>();
        private int _loadedSLMPatternsCnt = 0;
        private List<string> _loadedSLMSequences = new List<string>();
        private Mutex _mDefocus = new Mutex();
        private int _runTimeCal = 0;
        private double[] _slmBleachDelay = { 0.0, 0.0 };
        private ObservableCollection<SLMParams> _slmBleachWaveParams = new ObservableCollection<GeometryUtilities.SLMParams>();
        private int _SLMCallbackCount = 0;
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

        public double DefocusSavedUM
        {
            get
            {
                ResourceManagerCS.GetDeviceParamBuffer((int)SelectedHardware.SELECTED_SLM, (int)IDevice.Params.PARAM_SLM_DEFOCUS, _defocusParams, _defocusParams.Length);
                return _defocusParams[3];
            }
            set
            {
                _defocusParams[3] = value;
                SetDefocusParam();
            }
        }

        public double DefocusUM
        {
            get
            {
                ResourceManagerCS.GetDeviceParamBuffer((int)SelectedHardware.SELECTED_SLM, (int)IDevice.Params.PARAM_SLM_DEFOCUS, _defocusParams, _defocusParams.Length);
                return _defocusParams[0];
            }
            set
            {
                _defocusParams[0] = value;
                SetDefocusParam();
            }
        }

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

        public double RefractiveIndex
        {
            get
            {
                return _defocusParams[1];
            }
            set
            {
                _defocusParams[1] = value;
                SetDefocusParam();
            }
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

        /// <summary>
        /// [0]:cycle delay, [1]:sequence delay
        /// </summary>
        public double[] SLMBleachDelay
        {
            get { return _slmBleachDelay; }
            set { _slmBleachDelay = value; }
        }

        public ObservableCollection<SLMParams> SLMBleachWaveParams
        {
            get
            { return _slmBleachWaveParams; }
            set
            { _slmBleachWaveParams = value; }
        }

        /// <summary>
        /// [0]: Bead Calibration, no burning.
        /// [1]: Calibration by burning 2D at various planes.
        /// [2]: Calibration by burning a 3D pattern. [N/A]
        /// </summary>
        public int SLMCalibByBurning
        {
            get
            {
                string strTmp = string.Empty;
                int iVal = 0;
                try
                {
                    XmlDocument appSettings = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
                    XmlNodeList ndList = appSettings.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/BleachView/BleachCalibrationTool");
                    if (null != ndList)
                    {
                        if (!(XmlManager.GetAttribute(ndList[0], appSettings, "CalibByBurning", ref strTmp) && Int32.TryParse(strTmp, out iVal)))
                        {
                            iVal = 0;
                            XmlManager.SetAttribute(ndList[0], appSettings, "CalibByBurning", iVal.ToString());
                            MVMManager.Instance.ReloadSettings(SettingsFileType.APPLICATION_SETTINGS);
                            return iVal;
                        }
                    }
                }
                catch (Exception ex)
                {
                    ThorLog.Instance.TraceEvent(System.Diagnostics.TraceEventType.Verbose, 1, "Fail to get CalibByBurning: " + ex.Message);
                }
                return iVal;
            }
        }

        public int SLMDualPatternShift
        {
            get
            {
                int val = 0;
                ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_SLM, (int)IDevice.Params.PARAM_SLM_DUAL_SHIFT_PX, ref val);
                return val;
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

        /// <summary>
        /// SLM pattern type: [0]:raw (phase to be processed), [1]:phase; aligned with SLM3D but to be extended in the future
        /// </summary>
        public List<int> SLMphaseType
        {
            get;
            set;
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

        public bool SLMSkipFitting
        {
            get
            {
                int val = -1;
                ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_SLM, (int)IDevice.Params.PARAM_SLM_SKIP_FITTING, ref val);
                return (1 == val);
            }
            set
            {
                ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_SLM, (int)IDevice.Params.PARAM_SLM_SKIP_FITTING, value ? 1 : 0, (int)IDevice.DeviceSetParamType.NO_EXECUTION);
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
                return 0 < Array.FindLastIndex(val, element => 0 >= element) ? Array.FindLastIndex(val, element => 0 >= element) : val.Length;
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

        public bool CombineHolograms(string bmpPhaseName1, string bmpPhaseName2, int shiftPx)
        {
            return ((1 == CombineHologramFiles(bmpPhaseName1, bmpPhaseName2, shiftPx)) ? true : false);
        }

        public void InitializeWaveformBuilder(int clockRateHz)
        {
            WaveformBuilder.ClkRate = clockRateHz;
            WaveformBuilder.Field2Volts = (double)MVMManager.Instance["AreaControlViewModel", "LSMField2Theta", (object)1.0];
            int calibrateFieldSize = (0 == BleachCalibrateFieldSize) ? 1 : BleachCalibrateFieldSize;
            double[] calibrateFineScaleXY = (null == BleachCalibrateFineScaleXY) ? new double[2] { 1.0, 1.0 } : BleachCalibrateFineScaleXY;
            int[] calibratePixelXY = (null == BleachCalibratePixelXY) ? new int[2] { 1, 1 } : BleachCalibratePixelXY;
            int[] calibrateOffsetXY = (null == BleachCalibrateOffsetXY) ? new int[2] { 1, 1 } : BleachCalibrateOffsetXY;
            double[] calibrateFineOffsetXY = (null == BleachCalibrateFineOffsetXY) ? new double[2] { 0.0, 0.0 } : BleachCalibrateFineOffsetXY;
            int[] calibrateFlipHV = (null == BleachCalibrateFlipHV) ? new int[2] { 0, 0 } : BleachCalibrateFlipHV;

            WaveformBuilder.InitializeParams(calibrateFieldSize, calibrateFineScaleXY, calibratePixelXY, calibrateOffsetXY, calibrateFineOffsetXY, BleachCalibrateScaleYScan,
                calibrateFlipHV[1], calibrateFlipHV[0], BleachCalibratePockelsVoltageMin0, WaveformDriverType);
        }

        public bool LoadSLMPatternName(int runtimeCal, int id, string bmpPatternName, bool start, bool phaseDirect = false, int phaseType = 0, int timeoutVal = 0)
        {
            return ((1 == LoadSLMPattern(runtimeCal, id, bmpPatternName, (start) ? 1 : 0, phaseDirect ? 1 : 0, phaseType, timeoutVal)) ? true : false);
        }

        public bool ResetSLMCalibration()
        {
            return (1 == ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_SLM, (int)IDevice.Params.PARAM_SLM_RESET_AFFINE, (int)1, (int)IDevice.DeviceSetParamType.NO_EXECUTION));
        }

        public bool SaveSLMPatternName(string phaseMaskName)
        {
            return ((1 == SaveSLMPattern(phaseMaskName)) ? true : false);
        }

        public bool SLMCalibration(string bmpPatternName, float[] ptsFrom, float[] ptsTo, int size, int slm3D)
        {
            if (!String.IsNullOrEmpty(bmpPatternName) && !File.Exists(bmpPatternName))
                return false;

            int ret = 0;
            IntPtr ptsToPtr = Marshal.AllocHGlobal(size * sizeof(float));
            IntPtr ptsFromPtr = Marshal.AllocHGlobal(size * sizeof(float));
            Marshal.Copy(ptsTo, 0, ptsToPtr, size);
            Marshal.Copy(ptsFrom, 0, ptsFromPtr, size);

            ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_SLM, (int)IDevice.Params.PARAM_SLM_3D, slm3D, (int)IDevice.DeviceSetParamType.NO_EXECUTION);

            //doing SLM calibraton and waveform gen for burning
            ret = CalibrateSLM(bmpPatternName, ptsFromPtr, ptsToPtr, size, (0 == slm3D || 2 == SLMCalibByBurning) ? 1 : 0);

            Marshal.FreeHGlobal(ptsToPtr);
            Marshal.FreeHGlobal(ptsFromPtr);

            return (1 == ret) ? true : false;
        }

        public bool SLMSetBlank(int blankMode)
        {
            return 1 == ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_SLM, blankMode, (int)1, (int)IDevice.DeviceSetParamType.NO_EXECUTION);
        }

        public bool StartSLMBleach()
        {
            bool ret = true;

            //ensure the buffer is copied after the capture
            _pixelDataReady = false;

            //before start SLM bleach:
            _currentSLMWaveID = _currentSLMCycleID = _currentSLMSequenceID = _SLMCallbackCount = 0;
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
        private static extern int CalibrateSLM(string bmpPatternName, IntPtr xyPointFrom, IntPtr xyPointTo, int size, int setBleacher);

        [DllImport(".\\Modules_Native\\HologramGenerator.dll", EntryPoint = "CombineHologramFiles", CharSet = CharSet.Unicode)]
        private static extern int CombineHologramFiles(string bmpPhaseName1, string bmpPhaseName2, int shiftPx);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "InitCallBackBleachSLM")]
        private static extern void InitCallBackBleachSLM(ReportBleachSLMNowFinished reportBleachSLMNowFinished, PreBleachSLMCallback preBleachSLMCallback);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "InitializeBleach")]
        private static extern void InitializeBleach();

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "LoadSLMPattern", CharSet = CharSet.Unicode)]
        private static extern int LoadSLMPattern(int runtimeCal, int id, string bmpPatternName, int doStart, int phaseDirect, int phaseType, int timeout);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "SaveSLMPattern", CharSet = CharSet.Unicode)]
        private static extern int SaveSLMPattern(string bmpPhaseName);

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
            bool cycleReset = false;
            PreLoadNextSLMPattern();
            string LoadedSLMName = PreLoadNextSLM();
            LoadedSLMName = (PreLoadNextSLMSequence()) ? LoadedSLMName : string.Empty;

            //check if loaded:
            if (string.Empty == LoadedSLMName)
            {
                //no more added files located, reset counters before next cycle:
                if ((BleachFrames - 1) > _currentSLMCycleID)
                {
                    _currentSLMWaveID = _currentSLMSequenceID = 0;
                    _loadedSLMFiles.Clear();

                    if (1 < _loadedSLMSequences.Count)
                    {
                        _loadedSLMSequences.Clear();
                    }

                    cycleReset = true;
                    //try search drop-in files for next cycle:
                    LoadedSLMName = PreLoadNextSLM();
                    LoadedSLMName = (PreLoadNextSLMSequence()) ? LoadedSLMName : string.Empty;
                }
                _currentSLMCycleID++;
            }

            //sleep delay time,
            //do cycle delay at cycle end and do sequence delay in advance sequence mode
            _SLMCallbackCount++;
            if (1 < _SLMCallbackCount)
            {
                if (cycleReset)
                {
                    if (string.Empty != LoadedSLMName && !SLMBleachDelayEx(SLMBleachDelay[0]))
                    {
                        return false;
                    }
                }
                else if (_slmSequenceOn)
                {
                    if (string.Empty != LoadedSLMName && !SLMBleachDelayEx(SLMBleachDelay[1]))
                    {
                        return false;
                    }
                }
            }
            return (string.Empty != LoadedSLMName || (BleachFrames - 1) <= _currentSLMCycleID);
        }

        /// <summary>
        /// reload available SLM patterns from sequence file, return true if loaded any.
        /// </summary>
        /// <returns></returns>
        private bool LoadSLMSequence(string sequenceFile)
        {
            if (null == sequenceFile)
                return false;

            bool retVal = true;
            try
            {
                switch (_runTimeCal)
                {
                    case 2:
                        //reload patterns here when extremely short pattern time
                        List<int> patternIDs = new List<int>();
                        using (TextReader reader = new StreamReader(sequenceFile))
                        {
                            string line;
                            while ((line = reader.ReadLine()) != null)
                            {
                                patternIDs.Add(Int32.Parse(line));
                            }
                        }
                        //find out maximum pattern time for SLM timeout in msec, PatternID:1-based
                        int timeoutVal = 0;
                        foreach (int i in patternIDs)
                        {
                            timeoutVal = (int)Math.Max(timeoutVal,
                                _slmBleachWaveParams[i - 1].BleachWaveParams.PrePatIdleTime + _slmBleachWaveParams[i - 1].BleachWaveParams.Iterations * (_slmBleachWaveParams[i - 1].BleachWaveParams.PreIdleTime + _slmBleachWaveParams[i - 1].Duration + _slmBleachWaveParams[i - 1].BleachWaveParams.PostIdleTime) + _slmBleachWaveParams[i - 1].BleachWaveParams.PostPatIdleTime);
                        }

                        //load to SLM
                        for (int j = 0; j < patternIDs.Count; j++)
                        {
                            string waveFileName = SLMWaveformFolder[0] + "\\" + SLMWaveBaseName[1] + "_" + _slmBleachWaveParams[patternIDs[j] - 1].BleachWaveParams.ID.ToString("D" + FileName.GetDigitCounts().ToString()) + ".bmp";
                            if (File.Exists(waveFileName))
                            {
                                bool doStart = (j == (patternIDs.Count - 1)) ? true : false;
                                retVal = this.LoadSLMPatternName(_runTimeCal, j, waveFileName, doStart, IsStimulator, _slmBleachWaveParams[patternIDs[j] - 1].PhaseType, timeoutVal);
                                if (!retVal)
                                    break;
                            }
                        }
                        break;
                    default:
                        //allow runtime load of SLM sequence
                        return (1 == ResourceManagerCS.SetDeviceParamString((int)SelectedHardware.SELECTED_SLM, (int)IDevice.Params.PARAM_SLM_SEQ_FILENAME, sequenceFile, (int)IDevice.DeviceSetParamType.NO_EXECUTION));
                }
                return retVal;
            }
            catch (Exception e)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "ReLoadSLMPatterns Error:" + e.Message);
                return false;
            }
        }

        /// <summary>
        /// load available SLM patterns all at once, return true if loaded any.
        /// </summary>
        /// <returns></returns>
        private bool PreLoadAllSLMPatterns()
        {
            bool retVal = true;

            //reset counts:
            _loadedSLMPatternsCnt = 0;

            //default pattern based on current setting,
            //but follow at edit when available: [0]:raw (phase to be processed), [1]:phase
            SLMphaseType = new List<int>();

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
                    SLMphaseType.Add(_slmBleachWaveParams[i].PhaseType);
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

            //find out maximum pattern time for SLM timeout in msec and determine run-time calculation mode:
            int timeoutVal = 0;
            _runTimeCal = _slmSequenceOn ? 1 : 0;
            for (int i = 0; i < _slmBleachWaveParams.Count; i++)
            {
                timeoutVal = (int)Math.Max(timeoutVal, _slmBleachWaveParams[i].BleachWaveParams.PrePatIdleTime + _slmBleachWaveParams[i].BleachWaveParams.Iterations * (_slmBleachWaveParams[i].BleachWaveParams.PreIdleTime + _slmBleachWaveParams[i].Duration + _slmBleachWaveParams[i].BleachWaveParams.PostIdleTime) + _slmBleachWaveParams[i].BleachWaveParams.PostPatIdleTime);
                if (_slmBleachWaveParams[i].Duration < (double)Constants.SLM_PATTERN_TIME_MIN_MS)
                    _runTimeCal = 2;
            }

            //keep slm phase type available if not from SLM param:
            if (1 > SLMphaseType.Count)
                SLMphaseType.Add(SLM3D ? 1 : 0);

            //load to SLM:
            if (2 == _runTimeCal && _slmSequenceOn)
            {
                //do load of patterns at sequence reload ...
            }
            else
            {
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
                    retVal = this.LoadSLMPatternName(_runTimeCal, i, _loadedSLMPatterns[i], doStart, IsStimulator, (i < SLMphaseType.Count ? SLMphaseType[i] : 0), timeoutVal);
                    if (!retVal)
                        break;
                }
            }

            return (0 < _loadedSLMPatternsCnt) ? retVal : false;
        }

        /// <summary>
        /// load next SLM bleach waveform, return loaded file name and notice if it is the last available in current cycle
        /// </summary>
        /// <returns>LoadedFile</returns>
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
                if (null != matchingFileName && !_loadedSLMFiles.Contains(waveFileName))
                {
                    if (this.LoadBleachWaveformFile(matchingFileName, 1))   //set cycle count = 1 to reload waveform everytime
                    {
                        LoadedSLMName = waveFileName;
                        _loadedSLMFiles.Add(LoadedSLMName);
                        //_afterSLMCycle = false;
                    }
                }
                _currentSLMWaveID++;
            }

            //flag only if there's more to load:
            _lastInSLMCycle = (0 < _slmFilesInFolder.Except(_loadedSLMFiles).OrderBy(s => s).ToList().Count()) ? false : true;

            //if not loaded from the list, check if other files added:
            if (string.Empty == LoadedSLMName)
            {
                //_afterSLMCycle = true;
                List<string> addedFiles = _slmFilesInFolder.Except(_loadedSLMFiles).OrderBy(s => s).ToList();
                if (0 < addedFiles.Count())
                {
                    if (this.LoadBleachWaveformFile(addedFiles[0], 1))
                    {
                        LoadedSLMName = addedFiles[0];
                        _loadedSLMFiles.Add(LoadedSLMName);
                    }
                }
                _lastInSLMCycle = (1 >= addedFiles.Count()) ? true : _lastInSLMCycle;
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
                if (this.LoadSLMPatternName(_runTimeCal, _loadedSLMPatternsCnt, addedFiles[i], true, IsStimulator, SLMphaseType.Last()))
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
                string loadedFileName = _loadedSLMSequences.FirstOrDefault(checkString => checkString.Contains(seqFileName));
                if (null != loadedFileName)
                {
                    LoadedSequenceName = loadedFileName;
                }
                else
                {
                    string matchingFileName = _slmSequencesInFolder.FirstOrDefault(checkString => checkString.Contains(seqFileName));
                    if (LoadSLMSequence(matchingFileName))
                    {
                        LoadedSequenceName = matchingFileName;
                        _loadedSLMSequences.Add(LoadedSequenceName);
                        //_afterSLMCycle = false;
                    }
                }
                _currentSLMSequenceID++;
            }

            //flag only if there's more to load:
            _lastInSLMCycle = (0 < _slmSequencesInFolder.Except(_loadedSLMSequences).OrderBy(s => s).ToList().Count()) ? false : true;

            //if not loaded from the list, check if other files added:
            if (string.Empty == LoadedSequenceName)
            {
                //_afterSLMCycle = true;
                List<string> addedFiles = _slmSequencesInFolder.Except(_loadedSLMSequences).OrderBy(s => s).ToList();
                if (0 < addedFiles.Count())
                {
                    if (LoadSLMSequence(addedFiles[0]))
                    {
                        LoadedSequenceName = addedFiles[0];
                        _loadedSLMSequences.Add(LoadedSequenceName);
                    }
                }
                _lastInSLMCycle = (1 >= addedFiles.Count()) ? true : _lastInSLMCycle;
            }
            return (string.Empty == LoadedSequenceName) ? false : true;
        }

        private void SetDefocusParam()
        {
            //set EffectiveFocalMM based on magnification
            double[] focalLengths = (double[])MVMManager.Instance["ObjectiveControlViewModel", "FocalLengths", (object)new double[4] { 200.0, 100.0, 50.0, 200.0 }];
            _defocusParams[2] = focalLengths[0] * focalLengths[2] * (double)Constants.TURRET_FOCALLENGTH_MAGNIFICATION_RATIO / (focalLengths[1] * focalLengths[3] * (double)MVMManager.Instance["ObjectiveControlViewModel", "TurretMagnification", (object)20.0]);

            _mDefocus.WaitOne();
            ResourceManagerCS.SetDeviceParamBuffer((int)SelectedHardware.SELECTED_SLM, (int)IDevice.Params.PARAM_SLM_DEFOCUS, _defocusParams, _defocusParams.Length, (int)IDevice.DeviceSetParamType.NO_EXECUTION);
            _mDefocus.ReleaseMutex();
        }

        /// <summary>
        /// Execute delay time between SLM repeats
        /// </summary>
        /// <returns></returns>
        private bool SLMBleachDelayEx(double delayTime)
        {
            //check status every 5 seconds
            int timeWait = (int)(WaveformBuilder.MS_TO_S * delayTime);
            while (timeWait > 0)
            {
                BleachLastRunTime = DateTime.Now;
                System.Threading.Thread.Sleep(((WAITUNIT_MS < timeWait) ? WAITUNIT_MS : timeWait));
                //check if bleaching:
                if (!GetIsBleaching())
                {
                    IsBleaching = false;
                    return false;
                }
                timeWait -= (int)(new TimeSpan(DateTime.Now.Ticks - BleachLastRunTime.Ticks).TotalMilliseconds);
            }
            return true;
        }

        #endregion Methods
    }
}