namespace CaptureSetupDll.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.ComponentModel;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Security.Cryptography;
    using System.Text;
    using System.Text.RegularExpressions;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Input;
    using System.Windows.Shapes;
    using System.Xml;

    using CaptureSetupDll.View;

    using GeometryUtilities;

    using HDF5CS;

    using OverlayManager;

    using ThorLogging;

    using ThorSharedTypes;

    public partial class CaptureSetupViewModel : ViewModelBase
    {
        #region Fields

        public const string DEFAULT_SLMCALIB_XAML = "SLMCalibROIs.xaml";

        private RelayCommandWithParam _editSLMParamRelayCommand;
        private List<SLMEpochSequence> _epochSequence = new List<SLMEpochSequence>();
        private bool[] _roiToolVisible = new bool[14] { true, true, true, true, true, true, true, true, true, true, true, true, true, true };
        private ObservableCollection<SLMParams> _slmBleachCompParams = new ObservableCollection<GeometryUtilities.SLMParams>();
        private bool _slmBuildOnce = false;
        private double _slmCalibDwell = 0.0;
        private string _slmCalibFile = DEFAULT_SLMCALIB_XAML;
        private double _slmCalibPower = 0.0;
        private BleachWaveParams _slmCalibWaveParam;
        private double _slmCalibZPos = 0.0;
        private bool _slmExportChecked = false;
        private string _slmExportFileName = "SLMExport";
        private string _slmImportFilePathName = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments);
        private bool _slmImportSequences = false;
        private Int64 _slmLastCalibTimeUnix = 0;
        private bool _slmPanelAvailable = true;
        private bool _slmPanelInUse = true;
        private SLMParamEditWin _slmParamEditWin = null;
        private int _slmPatternSelectedIndex = -1;
        private bool _slmPatternsVisible = true;

        #endregion Fields

        #region Enumerations

        public enum SLMBMPSubFolder
        {
            PhaseMask,
            SLMPatternUnshifted,
            IntermediateZ
        }

        #endregion Enumerations

        #region Properties

        public double DefocusSavedUM
        {
            get
            {
                return _captureSetup.DefocusSavedUM;
            }
            set
            {
                if (_captureSetup.DefocusSavedUM != value)
                {
                    _captureSetup.DefocusSavedUM = value;
                    OnPropertyChanged("DefocusSavedUM");
                }
                OnPropertyChanged("IsDefocusUMDifferent");
            }
        }

        public double DefocusUM
        {
            get
            {
                return _captureSetup.DefocusUM;
            }
            set
            {
                if (_captureSetup.DefocusUM != value)
                {
                    _captureSetup.DefocusUM = value;
                    OnPropertyChanged("DefocusUM");
                }
                OnPropertyChanged("IsDefocusUMDifferent");
            }
        }

        public RelayCommandWithParam EditSLMParamRelayCommand
        {
            get
            {
                if (this._editSLMParamRelayCommand == null)
                    this._editSLMParamRelayCommand = new RelayCommandWithParam(EditSLMParam);

                return this._editSLMParamRelayCommand;
            }
        }

        public int EpochCount
        {
            get
            {
                return this._captureSetup.EpochCount;
            }
            set
            {
                if (this._captureSetup.EpochCount != value)
                {
                    this._captureSetup.EpochCount = value;
                    if (SLMPanelInUse)
                    {
                        EditSLMParam("SLM_BUILD");
                    }
                }
                else if (SLMPanelInUse && (!_slmBuildOnce))    //build once at first loading of SLM panel
                {
                    EditSLMParam("SLM_BUILD");
                    _slmBuildOnce = true;
                }
                OnPropertyChanged("EpochCount");
            }
        }

        public List<SLMEpochSequence> EpochSequence
        {
            get
            {
                return _epochSequence;
            }
            set
            {
                _epochSequence = value;
                OnPropertyChanged("EpochSequence");
            }
        }

        public bool IsDefocusUMDifferent
        {
            get { return (DefocusSavedUM != DefocusUM); }
        }

        public bool IsStimulator
        {
            get { return this._captureSetup.IsStimulator; }
        }

        public string LSMLastCalibrationDateForSLM
        {
            get
            {
                if (null != ExperimentDoc)
                {
                    string str = string.Empty;
                    int val = 0;
                    XmlNodeList ndList = ExperimentDoc.SelectNodes("/ThorImageExperiment/Photobleaching");
                    if (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "pixelX", ref str) && (Int32.TryParse(str, out val)) && 512 == val)
                    {
                        return (string)MVMManager.Instance["AreaControlViewModel", "LSMLastCalibrationDate", (object)string.Empty];
                    }
                }
                return "Calibration is not done or invalid.";
            }
        }

        public double RefractiveIndex
        {
            get
            {
                return _captureSetup.RefractiveIndex;
            }
            set
            {
                if (_captureSetup.RefractiveIndex != value)
                {

                    _captureSetup.RefractiveIndex = value;
                    OnPropertyChanged("RefractiveIndex");
                }
            }
        }

        /// <summary>
        /// Binding items: select[0], Line[1], Rectangle[2], Ellopse[3], Polygon[4], Polyline[5], Crosshair[6], Reticle[7], Scale[8], DeleteSingle[9], ClearAll[10], Load[11], Save[12], ChooseStats[13]
        /// </summary>
        public bool[] ROIToolVisible
        {
            get
            { return _roiToolVisible; }
            set
            {
                _roiToolVisible = value;
                OnPropertyChanged("ROIToolVisible");
            }
        }

        public bool SLM3D
        {
            get
            {
                return _captureSetup.SLM3D;
            }
            set
            {
                _captureSetup.SLM3D = value;
                OnPropertyChanged("SLM3D");
            }
        }

        public bool SLM3DBackup
        {
            get;
            set;
        }

        public string SLMActiveFolder
        {
            get
            {
                string workFolder = SLMSequenceOn ? SLMWaveformFolder[1] : SLMWaveformFolder[0];
                ResourceManagerCS.SafeCreateDirectory(workFolder);
                return workFolder;
            }
        }

        public double SLMBleachDelay
        {
            get { return this._captureSetup.SLMBleachDelay[0]; }
            set
            {
                this._captureSetup.SLMBleachDelay[0] = value;
                OnPropertyChanged("SLMBleachDelay");
            }
        }

        public bool SLMBleachNowEnabled
        {
            get
            {
                if (!IsProgressWindowOff || null != _slmParamEditWin)
                    return false;

                return (0 < Directory.EnumerateFiles(SLMActiveFolder, "*.raw ", SearchOption.TopDirectoryOnly).Count()) ? true : false;
            }
        }

        public ObservableCollection<SLMParams> SLMBleachWaveParams
        {
            get
            { return this._captureSetup.SLMBleachWaveParams; }
            set
            {
                this._captureSetup.SLMBleachWaveParams = value;
                OnPropertyChanged("SLMBleachWaveParams");
            }
        }

        public string[] SLMbmpSubFolders
        {
            get { return Enum.GetNames(typeof(SLMBMPSubFolder)).Select(x => "\\" + x.ToString()).ToArray(); }
        }

        public bool SLMCalAlert
        {
            get { return (DateTimeFormatInfo.CurrentInfo.DayNames.Length < (DateTime.Now).Subtract(SLMLastCalibTime).TotalDays) ? true : false; }
        }

        public bool SLMCalibByBurning
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
                            return 1 == iVal;
                        }
                    }
                }
                catch (Exception ex)
                {
                    ThorLog.Instance.TraceEvent(System.Diagnostics.TraceEventType.Verbose, 1, "Fail to get CalibByBurning: " + ex.Message);
                }
                return 1 == iVal;
            }
        }

        public double SLMCalibDwell
        {
            get { return _slmCalibDwell; }
            set
            {
                _slmCalibDwell = value;
                OnPropertyChanged("SLMCalibDwell");
            }
        }

        public string SLMCalibFile
        {
            get { return _slmCalibFile; }
            set
            {
                _slmCalibFile = value;
                OnPropertyChanged("SLMCalibFile");
            }
        }

        public double[] SLMCalibPatternScaleXY
        {
            get
            {
                string strTmp = string.Empty;
                double[] dVal = new double[2] { 1.0, 1.0 };
                bool doUpdate = false;
                try
                {
                    XmlDocument appSettings = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
                    XmlNodeList ndList = appSettings.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/BleachView/BleachCalibrationTool");
                    if (null != ndList)
                    {
                        if (!(XmlManager.GetAttribute(ndList[0], appSettings, "CalibPatternScaleX", ref strTmp) && Double.TryParse(strTmp, out dVal[0])))
                        {
                            dVal[0] = 1.0;
                            XmlManager.SetAttribute(ndList[0], appSettings, "CalibPatternScaleX", dVal[0].ToString());
                            doUpdate = true;
                        }
                        if (!(XmlManager.GetAttribute(ndList[0], appSettings, "CalibPatternScaleY", ref strTmp) && Double.TryParse(strTmp, out dVal[1])))
                        {
                            dVal[1] = 1.0;
                            XmlManager.SetAttribute(ndList[0], appSettings, "CalibPatternScaleY", dVal[1].ToString());
                            doUpdate = true;
                        }
                        if (doUpdate)
                            MVMManager.Instance.ReloadSettings(SettingsFileType.APPLICATION_SETTINGS);
                    }
                }
                catch (Exception ex)
                {
                    ThorLogging.ThorLog.Instance.TraceEvent(System.Diagnostics.TraceEventType.Verbose, 1, "Fail to get PatternScale: " + ex.Message);
                }
                return new double[2] { Math.Max(0.0, dVal[0]), Math.Max(0.0, dVal[1]) };
            }
        }

        public double SLMCalibPower
        {
            get { return _slmCalibPower; }
            set
            {
                _slmCalibPower = value;
                OnPropertyChanged("SLMCalibPower");
            }
        }

        public string SLMCalibROIFilePath
        {
            get { return BleachROIPath + SLMCalibFile; }
        }

        public BleachWaveParams SLMCalibWaveParam
        {
            get { return _slmCalibWaveParam; }
            set { _slmCalibWaveParam = value; }
        }

        public double SLMCalibZPos
        {
            get { return _slmCalibZPos; }
            set
            {
                _slmCalibZPos = value;
                OnPropertyChanged("SLMCalibZPos");
            }
        }

        public int SLMDualPatternShift
        {
            get
            {
                RemoveApplicationAttribute("/ApplicationSettings/DisplayOptions/CaptureSetup/BleachView", "DualPatternShiftPx");
                return _captureSetup.SLMDualPatternShift;
            }
        }

        public bool SLMExportChecked
        {
            get
            { return _slmExportChecked; }
            set
            {
                _slmExportChecked = value;
                OnPropertyChanged("SLMExportChecked");
            }
        }

        public string SLMExportFileName
        {
            get
            { return _slmExportFileName; }
            set
            {
                _slmExportFileName = value;
                OnPropertyChanged("SLMExportFileName");
            }
        }

        public string SLMImportFilePathName
        {
            get
            { return _slmImportFilePathName; }
            set
            {
                _slmImportFilePathName = value;
                OnPropertyChanged("SLMImportFilePathName");
            }
        }

        public bool SLMImportSequences
        {
            get
            { return _slmImportSequences; }
            set
            {
                _slmImportSequences = value;
                OnPropertyChanged("SLMImportSequences");
            }
        }

        public DateTime SLMLastCalibTime
        {
            get { return ResourceManagerCS.ToDateTimeFromUnix(_slmLastCalibTimeUnix); }
        }

        public Int64 SLMLastCalibTimeUnix
        {
            get { return _slmLastCalibTimeUnix; }
            set
            {
                _slmLastCalibTimeUnix = value;
                OnPropertyChanged("SLMLastCalibTimeUnix");
                OnPropertyChanged("SLMLastCalibTime");
                OnPropertyChanged("SLMCalAlert");
            }
        }

        public bool SLMPanelAvailable
        {
            get
            {
                if (!IsProgressWindowOff)
                {
                    return false;
                }
                else
                {
                    return _slmPanelAvailable;
                }
            }
            set
            {
                _slmPanelAvailable = (!IsProgressWindowOff) ? false : value;

                OnPropertyChanged("SLMPanelAvailable");
            }
        }

        public bool SLMPanelInUse
        {
            get { return _slmPanelInUse; }
            set { _slmPanelInUse = value; }
        }

        public SLMParamEditWin SLMParamEditWin
        {
            get
            { return _slmParamEditWin; }
            set
            {
                _slmParamEditWin = value;
            }
        }

        public int SLMPatternSelectedIndex
        {
            get
            { return _slmPatternSelectedIndex; }
            set
            {
                _slmPatternSelectedIndex = value;
                OnPropertyChanged("SLMPatternSelectedIndex");
            }
        }

        public bool SLMPatternsVisible
        {
            get { return _slmPatternsVisible; }
            set
            {
                _slmPatternsVisible = value;
                OverlayManagerClass.Instance.InitSelectROI(ref CaptureSetupViewModel.OverlayCanvas);
                OverlayManagerClass.Instance.DisplayModeROI(ref CaptureSetupViewModel.OverlayCanvas, new ThorSharedTypes.Mode[2] { ThorSharedTypes.Mode.PATTERN_NOSTATS, ThorSharedTypes.Mode.PATTERN_WIDEFIELD }, _slmPatternsVisible);
            }
        }

        public bool SLMPhaseDirect
        {
            get { return this._captureSetup.SLMPhaseDirect; }
            set { this._captureSetup.SLMPhaseDirect = value; }
        }

        public string SLMPreviewFileName
        {
            get { return SLMWaveformFolder[0] + "\\SLMPreview.bmp"; }
        }

        public bool SLMSelectWavelength
        {
            get { return SLMSelectWavelengthProp; }
            set
            {
                if (value != SLMSelectWavelengthProp)
                {
                    SLMSelectWavelengthProp = value;
                    //update for wavelength selection
                    OverlayManagerClass.Instance.WavelengthNM = SLMWavelengthNM;
                    OverlayManagerClass.Instance.DimWavelengthROI(ref CaptureSetupViewModel.OverlayCanvas);

                    var modes = new ThorSharedTypes.Mode[2] { ThorSharedTypes.Mode.PATTERN_NOSTATS, ThorSharedTypes.Mode.PATTERN_WIDEFIELD };

                    var wavelengths = new int[] { OverlayManagerClass.Instance.WavelengthNM };
                    OverlayManagerClass.Instance.DisplayOnlyPatternROIs(ref CaptureSetupViewModel.OverlayCanvas, modes, OverlayManagerClass.Instance.PatternID, wavelengths);
                    //update power settings
                    if (null != _slmParamEditWin)
                        _slmParamEditWin.UpdateSLMParamPower();
                }
            }
        }

        public bool SLMSelectWavelengthProp
        {
            get { return this._captureSetup.SLMSelectWavelength; }
            set
            {
                if (value != this._captureSetup.SLMSelectWavelength)
                {
                    this._captureSetup.SLMSelectWavelength = value;
                    OnPropertyChanged("SLMSelectWavelength");
                    OnPropertyChanged("SLMWavelengthNM");
                }
            }
        }

        public double SLMSequenceEpochDelay
        {
            get
            {
                return _captureSetup.SLMBleachDelay[1];
            }
            set
            {
                _captureSetup.SLMBleachDelay[1] = (double)Decimal.Round((Decimal)value, 3);
                OnPropertyChanged("SLMSequenceEpochDelay");
            }
        }

        public bool SLMSequenceOn
        {
            get
            {
                return _captureSetup.SLMSequenceOn;
            }
            set
            {
                _captureSetup.SLMSequenceOn = value;
                OnPropertyChanged("SLMSequenceOn");

                //rebuild sequences when selected
                if (_captureSetup.SLMSequenceOn)
                {
                    EditSLMParam("SLM_BUILD");
                }
                else
                {
                    OnPropertyChanged("SLMBleachNowEnabled");
                }
            }
        }

        public bool SLMSkipFitting
        {
            get { return this._captureSetup.SLMSkipFitting; }
            set { this._captureSetup.SLMSkipFitting = value; }
        }

        public string SLMViewWidth
        {
            get { return this._captureSetup.IsStimulator ? "0" : "Auto"; }
        }

        public string[] SLMWaveBaseName
        {
            get { return _captureSetup.SLMWaveBaseName; }
        }

        /// <summary>
        /// SLM folder path Waveforms[0], Sequences[1]
        /// </summary>
        public string[] SLMWaveformFolder
        {
            get
            { return this._captureSetup.SLMWaveformFolder; }
        }

        public int SLMWavelengthCount
        {
            get { return this._captureSetup.SLMWavelengthCount; }
        }

        public int SLMWavelengthNM
        {
            get { return this._captureSetup.SLMWavelengthNM; }
        }

        public double SLMZRefMM
        {
            get
            {
                string str = string.Empty;
                double dVal = 0.0;
                XmlNodeList ndList = ExperimentDoc.SelectNodes("/ThorImageExperiment/SLM");
                return (0 < ndList.Count) ? (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "zRefMM", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dVal)) ? dVal : 0.0 : 0.0;
            }
        }

        public string SLMZRefText
        {
            get
            {
                string str = string.Empty;
                XmlNodeList ndList = ExperimentDoc.SelectNodes("/ThorImageExperiment/SLM");
                return (0 >= ndList.Count || !XmlManager.GetAttribute(ndList[0], ExperimentDoc, "zRefMM", ref str)) ? "Set Ref. Z" : "Ref. Z " + (SLMZRefMM * (double)Constants.UM_TO_MM).ToString("N1") + "um";
            }
        }

        #endregion Properties

        #region Methods

        /// <summary>
        /// clear all raw files. 
        /// </summary>
        public void ClearSLMFiles(string path, string type = "all")
        {
            if (!Directory.Exists(path))
                return;

            try
            {
                IEnumerable<string> fileList;
                string[] switchStr = (0 <= type.IndexOf("all", StringComparison.OrdinalIgnoreCase)) ? new string[3] { "bmp", "txt", "raw" } : new string[1] { type };
                for (int itype = 0; itype < switchStr.Length; itype++)
                {
                    switch (switchStr[itype])
                    {
                        case "bmp":
                            fileList = Directory.EnumerateFiles(path, "*.bmp", SearchOption.TopDirectoryOnly);
                            if (0 == fileList.Count())
                                continue;

                            foreach (string bmpFile in fileList)
                            {
                                ResourceManagerCS.DeleteFile(bmpFile);
                            }

                            //iterate through subfolders
                            for (int i = 0; i < SLMbmpSubFolders.Length; i++)
                            {
                                string subFolder = path + SLMbmpSubFolders[i];
                                if (!Directory.Exists(subFolder))
                                    continue;

                                fileList = Directory.EnumerateFiles(subFolder, "*.bmp ", SearchOption.TopDirectoryOnly);
                                if (0 == fileList.Count())
                                    continue;

                                foreach (string waveFile in fileList)
                                {
                                    ResourceManagerCS.DeleteFile(waveFile);
                                }
                            }
                            break;
                        case "txt":
                            fileList = Directory.EnumerateFiles(path, "*.txt", SearchOption.TopDirectoryOnly);
                            if (0 == fileList.Count())
                                continue;

                            foreach (string txtFile in fileList)
                            {
                                ResourceManagerCS.DeleteFile(txtFile);
                            }
                            break;
                        case "raw":
                        default:
                            fileList = Directory.EnumerateFiles(path, "*.raw", SearchOption.TopDirectoryOnly);
                            if (0 == fileList.Count())
                                continue;

                            foreach (string waveFile in fileList)
                            {
                                ResourceManagerCS.DeleteFile(waveFile);
                            }

                            OnPropertyChanged("SLMBleachNowEnabled");
                            break;
                    }
                }
            }
            catch (Exception ex)
            {
                ThorLogging.ThorLog.Instance.TraceEvent(System.Diagnostics.TraceEventType.Verbose, 1, "ClearSLMFiles error: " + ex.Message);
            }
        }

        public bool CombineHolograms(string bmpPhaseName1, string bmpPhaseName2)
        {
            return _captureSetup.CombineHolograms(bmpPhaseName1, bmpPhaseName2, SLMDualPatternShift);
        }

        /// <summary>
        /// compare SLM waveform sequence files and return false if different.
        /// </summary>
        public bool CompareSLMFiles(List<SLMEpochSequence> sequences)
        {
            bool retVal = true;
            int iVal = 0, fid = 0;

            //only compare sequences
            if (!SLMSequenceOn || !Directory.Exists(SLMWaveformFolder[1]))
                return false;

            //return if no sequence to build
            if (0 >= sequences.Count())
                return true;

            //compare with sequence files
            IEnumerable<string> fileList = Directory.EnumerateFiles(SLMWaveformFolder[1], "*.txt", SearchOption.TopDirectoryOnly);
            if ((0 >= fileList.Count()) || (sequences.Count != fileList.Count()))
                retVal = false;

            try
            {
                while ((retVal) && fid < fileList.Count())
                {
                    //compare epoch count
                    if (sequences.ElementAt(fid).EpochCountInt != EpochSequence.ElementAt(fid).EpochCountInt)
                    {
                        retVal = false;
                        break;
                    }

                    //compare pattern ids with file
                    StreamReader seqTextFile = new StreamReader(fileList.ElementAt(fid));
                    List<int> readIDs = new List<int>();
                    while (!seqTextFile.EndOfStream)
                    {
                        if (Int32.TryParse(seqTextFile.ReadLine(), out iVal))
                            readIDs.Add(iVal);
                    }
                    seqTextFile.Close();
                    seqTextFile.Dispose();

                    int[] patternIDs = SLMEpochSequence.ParseForIntArray(sequences.ElementAt(fid).SequenceStr);
                    if (patternIDs.Length != readIDs.Count)
                    {
                        retVal = false;
                        break;
                    }
                    for (int j = 0; j < patternIDs.Length; j++)
                    {
                        if (patternIDs[j] != readIDs[j])
                        {
                            retVal = false;
                            break;
                        }
                    }
                    fid++;
                }
            }
            catch (Exception ex)
            {
                ThorLogging.ThorLog.Instance.TraceEvent(System.Diagnostics.TraceEventType.Verbose, 1, "CompareSLMFiles error: " + ex.Message);
                retVal = false;
            }

            //clear files if different
            if (!retVal)
            {
                ClearSLMFiles(SLMWaveformFolder[1], "txt");
                ClearSLMFiles(SLMWaveformFolder[1], "raw");
            }
            return retVal;
        }

        /// <summary>
        /// compare SLM waveform params and return false if different.
        /// </summary>
        public bool CompareSLMParams()
        {
            bool retVal = true;
            if (_slmBleachCompParams.Count != SLMBleachWaveParams.Count)
            {
                EpochSequence.Clear();          //clear sequences when delete pattern
                retVal = false;
            }
            if (retVal)
            {
                for (int i = 0; i < SLMBleachWaveParams.Count; i++)
                {
                    if (_slmBleachCompParams[i].BleachWaveParams.ID != SLMBleachWaveParams[i].BleachWaveParams.ID)
                    {
                        EpochSequence.Clear();  //clear sequences when reorder
                        retVal = false;
                        break;
                    }
                }
            }
            if (retVal)
            {
                for (int i = 0; i < SLMBleachWaveParams.Count; i++)
                {
                    if (false == _slmBleachCompParams[i].CompareTo(SLMBleachWaveParams[i]))
                    {
                        retVal = false;
                        break;
                    }
                }
            }
            if (!retVal)
            {
                ClearSLMFiles(SLMWaveformFolder[0], "raw");
                ClearSLMFiles(SLMWaveformFolder[1], "txt"); //clear sequences as well
                ClearSLMFiles(SLMWaveformFolder[1], "raw");
            }
            return retVal;
        }

        /// <summary>
        /// rearrange SLMBLeachParams's ID after deleting target ID number, similar to DeletePatternROI in OverlayManager.
        /// </summary>
        /// <param name="targetID"></param>
        /// <param name="subFolderIndex"></param>
        /// <param name="reorder"></param>
        public void DeleteSLMBleachParamsID(uint targetID, int subFolderIndex = -1, bool reorder = true)
        {
            try
            {
                string numDigits = "D" + FileName.GetDigitCounts().ToString();

                //delete pattern file:
                for (int i = 0; i < SLMBleachWaveParams.Count; i++)
                {
                    if (targetID == SLMBleachWaveParams[i].BleachWaveParams.ID)
                    {
                        ResourceManagerCS.DeleteFile(SLMWaveformFolder[0] + "\\" + SLMWaveBaseName[1] + "_" + targetID.ToString(numDigits) + ".bmp");
                        ResourceManagerCS.DeleteFile(SLMWaveformFolder[0] + "\\" + SLMWaveBaseName[1] + "_" + targetID.ToString(numDigits) + ".txt");
                        //delete files in subfolders, unshifted & intermediate Z
                        for (int j = 0; j < SLMbmpSubFolders.Length; j++)
                        {
                            if (Directory.Exists(SLMWaveformFolder[0] + SLMbmpSubFolders[j]) &&
                                (0 > subFolderIndex || j == subFolderIndex))
                            {
                                DirectoryInfo dirInfo = new DirectoryInfo(SLMWaveformFolder[0] + SLMbmpSubFolders[j]);
                                foreach (FileInfo fInfo in dirInfo.GetFiles())
                                {
                                    if (System.IO.Path.GetFileNameWithoutExtension(fInfo.Name).Contains(SLMWaveBaseName[1] + "_" + targetID.ToString(numDigits)))
                                        fInfo.Delete();
                                }
                            }
                        }
                    }
                }

                if (reorder)
                {
                    //rename pattern files in the folders:
                    ReorderSLMPatternFiles(targetID, SLMWaveformFolder[0]);
                    for (int j = 0; j < SLMbmpSubFolders.Length; j++)
                    {
                        ReorderSLMPatternFiles(targetID, SLMWaveformFolder[0] + SLMbmpSubFolders[j]);
                    }

                    //re-assign IDs:
                    for (int i = 0; i < SLMBleachWaveParams.Count; i++)
                    {
                        if (targetID <= SLMBleachWaveParams[i].BleachWaveParams.ID)
                        {
                            SLMBleachWaveParams[i].BleachWaveParams.ID--;
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                ThorLogging.ThorLog.Instance.TraceEvent(System.Diagnostics.TraceEventType.Verbose, 1, "DeleteSLMBleachParamsID error: " + ex.Message);
            }
        }

        public Point GetSLMPatternBoundROICenter(ref List<Point> pts, Point offCenter)
        {
            Point roiOffset = new Point(0, 0);
            List<Point> newPts = new List<Point>();
            double lBound = BleachLSMPixelXY[0], rBound = 0, tBound = BleachLSMPixelXY[1], bBound = 0;
            //find boundaries:
            for (int i = 0; i < pts.Count; i++)
            {
                if (lBound >= pts[i].X)
                {
                    lBound = pts[i].X;
                }
                if (rBound <= pts[i].X)
                {
                    rBound = pts[i].X;
                }
                if (tBound >= pts[i].Y)
                {
                    tBound = pts[i].Y;
                }
                if (bBound <= pts[i].Y)
                {
                    bBound = pts[i].Y;
                }
            }
            //get center of bounding box:
            if ((lBound <= rBound) && (tBound <= bBound))
            {
                Point roiCenter = new Point((rBound + lBound) / 2, (bBound + tBound) / 2);
                roiOffset.X = (offCenter.X < 0) ? (BleachLSMPixelXY[0] / 2) - roiCenter.X : (BleachLSMPixelXY[0] / 2) - offCenter.X;
                roiOffset.Y = (offCenter.Y < 0) ? (BleachLSMPixelXY[1] / 2) - roiCenter.Y : (BleachLSMPixelXY[1] / 2) - offCenter.Y;
                //offset points:
                for (int i = 0; i < pts.Count; i++)
                {
                    Point tmp = new Point(pts[i].X + roiOffset.X, pts[i].Y + roiOffset.Y);
                    newPts.Add(tmp);
                }
                pts = newPts;
            }

            return roiOffset;
        }

        public void IdleSLM()
        {
            //set bleach scanner power to zero
            MVMManager.Instance["PowerControlViewModel", "BleacherPower0"] = MVMManager.Instance["PowerControlViewModel", "BleacherPower1"] = 0.0;

            //not leaving pattern on
            SLMSetBlank();
        }

        public void InitializeWaveformBuilder(int clockRateHz)
        {
            _captureSetup.InitializeWaveformBuilder(clockRateHz);
        }

        public bool LoadSLMPatternName(int runtimeCal, int id, string bmpPatternName, bool start, bool phaseDirect = false, int timeoutVal = 0)
        {
            return _captureSetup.LoadSLMPatternName(runtimeCal, id, bmpPatternName, start, phaseDirect, timeoutVal);
        }

        public bool ResetSLMCalibration()
        {
            return _captureSetup.ResetSLMCalibration();
        }

        public bool SaveSLMPatternName(string phaseMaskName)
        {
            return _captureSetup.SaveSLMPatternName(phaseMaskName);
        }

        public bool SLMCalibration(string bmpPatternName, float[] ptsFrom, float[] ptsTo, int size)
        {
            return _captureSetup.SLMCalibration(bmpPatternName, ptsFrom, ptsTo, size, SLM3DBackup);
        }

        public bool SLMSetBlank()
        {
            return _captureSetup.SLMSetBlank();
        }

        public void UpdateSLMCompParams()
        {
            _slmBleachCompParams.Clear();

            for (int i = 0; i < SLMBleachWaveParams.Count; i++)
            {
                _slmBleachCompParams.Add(new SLMParams(SLMBleachWaveParams[i]));
            }
            OnPropertyChanged("SLMBleachNowEnabled");
        }

        public void UpdateSLMListGUI()
        {
            OnPropertyChanged("SLMBleachWaveParams");
        }

        void DuplicateSelectedSLMPattern()
        {
            if (_slmPatternSelectedIndex < 0 || _slmPatternSelectedIndex >= SLMBleachWaveParams.Count) return;
            string subFolder = string.Empty;
            string numDigits = "D" + FileName.GetDigitCounts().ToString();

            FileName compName = new FileName(SLMBleachWaveParams[_slmPatternSelectedIndex].Name, '_');
            compName.FileExtension = ".bmp";
            compName.MakeUnique(SLMWaveformFolder[0]);
            string originalPath = SLMWaveformFolder[0] + "\\" + SLMBleachWaveParams[_slmPatternSelectedIndex].Name;
            string duplicatePath = SLMWaveformFolder[0] + "\\" + compName.NameWithoutExtension;

            FileCopyWithExistCheck(originalPath + ".bmp", duplicatePath + ".bmp", true);
            FileCopyWithExistCheck(originalPath + ".txt", duplicatePath + ".txt", true);
            //delete files in subfolders
            for (int j = 0; j < SLMbmpSubFolders.Length; j++)
            {
                subFolder = SLMWaveformFolder[0] + SLMbmpSubFolders[j];

                if (Directory.Exists(subFolder))
                {
                    originalPath = subFolder + "\\" + SLMBleachWaveParams[_slmPatternSelectedIndex].Name + ".bmp";
                    duplicatePath = subFolder + "\\" + compName.NameWithoutExtension + ".bmp";

                    FileCopyWithExistCheck(originalPath + ".bmp", duplicatePath + ".bmp", true);
                }
            }

            SLMParams paramDuplicate = new SLMParams(SLMBleachWaveParams[_slmPatternSelectedIndex]);

            paramDuplicate.Name = compName.NameWithoutExtension;

            OverlayManagerClass.Instance.DuplicatePatternROIs(ref CaptureSetupViewModel.OverlayCanvas, (int)SLMBleachWaveParams[_slmPatternSelectedIndex].BleachWaveParams.ID,              //ID: 1 based
                            (IsStimulator ? ThorSharedTypes.Mode.PATTERN_WIDEFIELD : ThorSharedTypes.Mode.PATTERN_NOSTATS));
            OverlayManagerClass.Instance.BackupROIs();
            paramDuplicate.BleachWaveParams.ID = (uint)SLMBleachWaveParams.Count + 1;
            SLMBleachWaveParams.Add(paramDuplicate);

            OverlayManagerClass.Instance.PatternID = SLMBleachWaveParams.Count;
            SLMPatternSelectedIndex = SLMBleachWaveParams.Count - 1;

            //create working-directory
            ResourceManagerCS.SafeCreateDirectory(SLMWaveformFolder[0]);

            //persist slm params in all modalities:
            PersistGlobalExperimentXML(GlobalExpAttribute.SLM_BLEACH);

            if (!CompareSLMParams())
                EditSLMParam("SLM_BUILD");

            UpdateSLMCompParams();
        }

        private void EditSLMParam(object type)
        {
            //back up LSMScanMode in case SLM panel changes it
            if (ICamera.ScanMode.FORWARD_SCAN >= (ICamera.ScanMode)MVMManager.Instance["ScanControlViewModel", "LSMScanMode", (object)0])
                MVMManager.Instance["ScanControlViewModel", "LastLSMScanMode"] = (int)MVMManager.Instance["ScanControlViewModel", "LSMScanMode", (object)0];

            string str = (string)type;
            double dTmp = 0.0;
            System.Collections.Specialized.BitVector32 bitVec32;
            XmlNodeList ndList = ExperimentDoc.SelectNodes("/ThorImageExperiment/SLM");

            //keep overlay manager access to hardware mvms, property only when import
            OverlayManagerClass.Instance.SimulatorMode = false;

            //back up properties here
            SLM3DBackup = SLM3D;

            switch (SLMParamEditWin.SLMPatternTypeDictionary[str])
            {
                case SLMParamEditWin.SLMPatternType.Add:
                    if (null == _slmParamEditWin)
                    {
                        ROIToolVisible = (IsStimulator) ?
                            new bool[14] { true, true, true, true, true, true, true, false, false, true, true, true, true, false } :
                            new bool[14] { true, false, false, true, false, false, true, false, false, true, true, true, true, false };

                        _slmParamEditWin = new SLMParamEditWin(this);
                        _slmParamEditWin.Topmost = true;
                        _slmParamEditWin.PanelMode = SLMParamEditWin.SLMPanelMode.ParamEdit;
                        if (SLMBleachWaveParams.Count > 0)
                        {
                            _slmParamEditWin.SLMParamsCurrent = new SLMParams(SLMBleachWaveParams.Last());
                        }
                        else
                        {
                            _slmParamEditWin.SLMParamsCurrent = new SLMParams();
                            _slmParamEditWin.SLMParamsCurrent.Red = 255;
                            _slmParamEditWin.SLMParamsCurrent.Green = 255;
                            _slmParamEditWin.SLMParamsCurrent.Blue = 0;
                            _slmParamEditWin.SLMParamsCurrent.BleachWaveParams.Iterations = 10;
                            _slmParamEditWin.SLMParamsCurrent.Duration = 100;
                            _slmParamEditWin.SLMParamsCurrent.BleachWaveParams.Power = 0.0;
                            _slmParamEditWin.SLMParamsCurrent.BleachWaveParams.Power1 = (1 < BleachCalibratePockelsVoltageMin0.Length) ? 0.0 : -1.0;
                            _slmParamEditWin.SLMParamsCurrent.BleachWaveParams.MeasurePower = (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "measurePowerMW", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp)) ? dTmp : 0.0;
                            if (1 < BleachCalibratePockelsVoltageMin0.Length)
                                _slmParamEditWin.SLMParamsCurrent.BleachWaveParams.MeasurePower1 = (XmlManager.GetAttribute(ndList[0], ExperimentDoc, "measurePower1MW", ref str) && Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp)) ? dTmp : 0.0;
                        }
                        _slmParamEditWin.SLMParamsCurrent.BleachWaveParams.UMPerPixel = (double)MVMManager.Instance["AreaControlViewModel", "PixelSizeUM", (object)1.0];
                        _slmParamEditWin.SLMParamsCurrent.BleachWaveParams.UMPerPixelRatio = this.BleachPixelSizeUMRatio;
                        _slmParamEditWin.SLMParamsCurrent.BleachWaveParams.ROIWidthUM = 0;
                        _slmParamEditWin.SLMParamsCurrent.BleachWaveParams.ROIHeight = 0;
                        _slmParamEditWin.SLMParamsCurrent.PhaseType = (IsStimulator || SLM3D) ? 1 : 0;
                        _slmParamEditWin.UpdateSLMParamPower();
                        _slmParamEditWin.Title = "Add SLM Pattern";
                        _slmParamEditWin.SLMParamID = -1;

                        OverlayManagerClass.Instance.CurrentMode = IsStimulator ? ThorSharedTypes.Mode.PATTERN_WIDEFIELD : ThorSharedTypes.Mode.PATTERN_NOSTATS;
                        OverlayManagerClass.Instance.PatternID = SLMBleachWaveParams.Count + 1;
                        bitVec32 = new System.Collections.Specialized.BitVector32(Convert.ToByte(_slmParamEditWin.SLMParamsCurrent.Red));
                        bitVec32[OverlayManagerClass.SecG] = Convert.ToByte(_slmParamEditWin.SLMParamsCurrent.Green);
                        bitVec32[OverlayManagerClass.SecB] = Convert.ToByte(_slmParamEditWin.SLMParamsCurrent.Blue);
                        bitVec32[OverlayManagerClass.SecA] = 0; //avoid overflow int
                        OverlayManagerClass.Instance.WavelengthNM = SLMWavelengthNM;
                        OverlayManagerClass.Instance.ZRefMM = SLMZRefMM;
                        OverlayManagerClass.Instance.ColorRGB = bitVec32.Data;
                        OverlayManagerClass.Instance.BackupROIs();

                        var modes = new ThorSharedTypes.Mode[2] { ThorSharedTypes.Mode.PATTERN_NOSTATS, ThorSharedTypes.Mode.PATTERN_WIDEFIELD };
                        var wavelengths = new int[] { OverlayManagerClass.Instance.WavelengthNM };
                        OverlayManagerClass.Instance.DisplayOnlyPatternROIs(ref CaptureSetupViewModel.OverlayCanvas, modes, OverlayManagerClass.Instance.PatternID, wavelengths);

                        //create working-directory
                        ResourceManagerCS.SafeCreateDirectory(SLMWaveformFolder[0]);

                        _slmParamEditWin.DataContext = _slmParamEditWin.SLMParamsCurrent;
                        SetSLMParamEditWinPosition();
                        _slmParamEditWin.Closed += _slmParamEditWin_Closed;
                        _slmParamEditWin.Show();
                    }
                    break;
                case SLMParamEditWin.SLMPatternType.Edit:
                    if (0 == SLMBleachWaveParams.Count)
                    {
                        EditSLMParam("SLM_PATTERN_ADD");
                        return;
                    }

                    if (null == _slmParamEditWin)
                    {
                        ROIToolVisible = IsStimulator ?
                           new bool[14] { true, true, true, true, true, true, true, false, false, true, true, true, true, false } :
                           new bool[14] { true, false, false, true, false, false, true, false, false, true, true, true, true, false };

                        _slmParamEditWin = new SLMParamEditWin(this);
                        _slmParamEditWin.Topmost = true;
                        _slmParamEditWin.PanelMode = SLMParamEditWin.SLMPanelMode.ParamEdit;
                        OverlayManagerClass.Instance.CurrentMode = IsStimulator ? ThorSharedTypes.Mode.PATTERN_WIDEFIELD : ThorSharedTypes.Mode.PATTERN_NOSTATS;

                        if (SLMBleachWaveParams.Count > 0)
                        {
                            if ((0 <= _slmPatternSelectedIndex) && (_slmPatternSelectedIndex < SLMBleachWaveParams.Count))
                            {
                                _slmParamEditWin.SLMParamsCurrent = new SLMParams(SLMBleachWaveParams[_slmPatternSelectedIndex]);
                                _slmParamEditWin.SLMParamID = _slmPatternSelectedIndex;
                            }
                            else
                            {   //use the last one if not selected:
                                _slmParamEditWin.SLMParamsCurrent = new SLMParams(SLMBleachWaveParams.Last());
                                _slmPatternSelectedIndex = _slmParamEditWin.SLMParamID = SLMBleachWaveParams.Count - 1;
                            }
                            OverlayManagerClass.Instance.PatternID = (int)_slmParamEditWin.SLMParamsCurrent.BleachWaveParams.ID;     //pattern ID: 1 based
                        }
                        else
                        {
                            OverlayManagerClass.Instance.PatternID = SLMBleachWaveParams.Count + 1;     //pattern ID: 1 based
                        }

                        _slmParamEditWin.SLMParamsCurrent.BleachWaveParams.UMPerPixel = (double)MVMManager.Instance["AreaControlViewModel", "PixelSizeUM", (object)1.0];
                        _slmParamEditWin.SLMParamsCurrent.BleachWaveParams.UMPerPixelRatio = this.BleachPixelSizeUMRatio;
                        _slmParamEditWin.SLMParamsCurrent.PhaseType = (IsStimulator || SLM3D) ? 1 : 0;
                        _slmParamEditWin.Title = "Edit SLM Pattern Name: " + _slmParamEditWin.SLMParamsCurrent.Name;
                        bitVec32 = new System.Collections.Specialized.BitVector32(Convert.ToByte(_slmParamEditWin.SLMParamsCurrent.Red));
                        bitVec32[OverlayManagerClass.SecG] = Convert.ToByte(_slmParamEditWin.SLMParamsCurrent.Green);
                        bitVec32[OverlayManagerClass.SecB] = Convert.ToByte(_slmParamEditWin.SLMParamsCurrent.Blue);
                        bitVec32[OverlayManagerClass.SecA] = 0; //avoid overflow int
                        OverlayManagerClass.Instance.WavelengthNM = SLMWavelengthNM;
                        OverlayManagerClass.Instance.ZRefMM = SLMZRefMM;
                        OverlayManagerClass.Instance.ColorRGB = bitVec32.Data;
                        OverlayManagerClass.Instance.BackupROIs();

                        var modes = new ThorSharedTypes.Mode[2] { ThorSharedTypes.Mode.PATTERN_NOSTATS, ThorSharedTypes.Mode.PATTERN_WIDEFIELD };

                        var wavelengths = new int[] { OverlayManagerClass.Instance.WavelengthNM };
                        OverlayManagerClass.Instance.DisplayOnlyPatternROIs(ref CaptureSetupViewModel.OverlayCanvas, modes, OverlayManagerClass.Instance.PatternID, wavelengths);

                        _slmParamEditWin.DataContext = _slmParamEditWin.SLMParamsCurrent;
                        SetSLMParamEditWinPosition();
                        _slmParamEditWin.Closed += _slmParamEditWin_Closed;
                        _slmParamEditWin.Show();
                    }
                    break;
                case SLMParamEditWin.SLMPatternType.Delete:
                    //delete not compatible with other mode
                    if (null != _slmParamEditWin)
                        return;

                    if ((0 < SLMBleachWaveParams.Count)
                        && (0 <= _slmPatternSelectedIndex) && (SLMBleachWaveParams.Count > _slmPatternSelectedIndex))
                    {
                        OverlayManagerClass.Instance.DeletePatternROI(ref CaptureSetupViewModel.OverlayCanvas, (int)SLMBleachWaveParams[_slmPatternSelectedIndex].BleachWaveParams.ID,              //ID: 1 based
                            (IsStimulator ? ThorSharedTypes.Mode.PATTERN_WIDEFIELD : ThorSharedTypes.Mode.PATTERN_NOSTATS));
                        OverlayManagerClass.Instance.BackupROIs();
                        DeleteSLMBleachParamsID(SLMBleachWaveParams[_slmPatternSelectedIndex].BleachWaveParams.ID);

                        //delete from list at last, since selected index will be lost afterward;
                        //replace collection to update view:
                        ObservableCollection<SLMParams> slmBleachWaveParams = new ObservableCollection<SLMParams>(SLMBleachWaveParams.ToArray());
                        slmBleachWaveParams.RemoveAt(_slmPatternSelectedIndex);
                        SLMBleachWaveParams = slmBleachWaveParams;

                        if (!CompareSLMParams())
                            EditSLMParam("SLM_BUILD");
                    }

                    break;
                case SLMParamEditWin.SLMPatternType.ShowHide:
                    SLMPatternsVisible = !SLMPatternsVisible;
                    break;
                case SLMParamEditWin.SLMPatternType.Execute:
                    if (null != _slmParamEditWin)
                    {
                        _slmParamEditWin.Close();
                    }
                    StartSLMBleach();
                    break;
                case SLMParamEditWin.SLMPatternType.Calibration:
                    SLMCalibZPos = (double)MVMManager.Instance["ZControlViewModel", "ZPosition", (object)0];
                    if (null == _slmParamEditWin)
                    {
                        if (null == Bitmap)
                            return;
                        if ((int)ICamera.LSMType.LSMTYPE_LAST == ResourceManagerCS.GetBleacherType())
                        {
                            System.Windows.MessageBox.Show("Bleach Scanner has not been selected!");
                            return;
                        }
                        //stop Live Capture if GALVO_GALVO:
                        if ((int)ICamera.LSMType.GALVO_GALVO == ResourceManagerCS.GetLSMType())
                        {
                            StopLiveCapture();
                        }

                        //set overlay. Going to use a different ROI file,
                        //persist ActiveROIs first:
                        OverlayManagerClass.Instance.PersistSaveROIs();

                        //prepare calibration xaml and file path:
                        SetSLMCalibROIFilePath();

                        //display calibration ROIs:
                        if (!DisplayROI(SLMCalibROIFilePath))
                            return;

                        //blank slm before start of calibration:
                        EditSLMParam("SLM_BLANK");

                        ROIToolVisible = new bool[14] { true, false, false, false, false, false, true, false, false, true, true, true, true, false };

                        OverlayManagerClass.Instance.CurrentMode = ThorSharedTypes.Mode.PATTERN_NOSTATS;    //keep mode unchanged due to "SLMCalibROIs.xaml"
                        OverlayManagerClass.Instance.PatternID = 2;                                         //for user-selection, target is on PatterID 1
                        OverlayManagerClass.Instance.SkipRedrawCenter = SLMCalibByBurning ? false : SLM3D;  //allow start index of 1 or 0 if not calibration by burning(2D)
                        bitVec32 = new System.Collections.Specialized.BitVector32(Convert.ToByte(255));
                        bitVec32[OverlayManagerClass.SecG] = Convert.ToByte(0);
                        bitVec32[OverlayManagerClass.SecB] = Convert.ToByte(0);
                        bitVec32[OverlayManagerClass.SecA] = 0; //avoid overflow int
                        OverlayManagerClass.Instance.ColorRGB = bitVec32.Data;

                        _slmParamEditWin = new SLMParamEditWin(this);
                        _slmParamEditWin.Topmost = true;
                        _slmParamEditWin.PanelMode = SLMParamEditWin.SLMPanelMode.Calibration;
                        Dictionary<double, List<Shape>> zShapes = OverlayManagerClass.Instance.GetPatternZROIs(-1, OverlayManagerClass.Instance.PatternID - 1);
                        _slmParamEditWin.CalibratePointCount = (0 < zShapes.Count) ? zShapes[0].Count : 0;
                        SLMCalibPanel slmCalib = SLM3D ?
                            new SLMCalibPanel("", "RESET_CALIBRATION3D", "DONE", "New Calibration", "Click 'YES' will reset calibrations\nand continue.") :
                            new SLMCalibPanel("", "BURN", "DONE", "New Calibration", "Move to another clear area on the\n calibration slide then Press Yes to\n create calibration spots on the slide.");
                        _slmParamEditWin.DataContext = slmCalib;
                        SetSLMParamEditWinPosition();
                        _slmParamEditWin.Closed += _slmParamEditWin_Closed;
                        _slmParamEditWin.Show();
                    }
                    break;
                case SLMParamEditWin.SLMPatternType.Import:
                    if (null == _slmParamEditWin)
                    {
                        OverlayManagerClass.Instance.ZRefMM = SLMZRefMM;

                        _slmParamEditWin = new SLMParamEditWin(this);
                        _slmParamEditWin.Topmost = true;
                        _slmParamEditWin.PanelMode = SLMParamEditWin.SLMPanelMode.Browse;
                        SLMBrowsePanel slmBrowse = new SLMBrowsePanel("BROWSE", "IMPORT", "CANCEL",
                            SLMExportChecked.ToString(),
                            SLMImportSequences.ToString(),
                            _slmParamEditWin.SLMImportPath = SLMImportFilePathName,
                            _slmParamEditWin.SLMExportFileName = SLMExportFileName);
                        _slmParamEditWin.DataContext = slmBrowse;
                        SetSLMParamEditWinPosition();
                        _slmParamEditWin.Closed += _slmParamEditWin_Closed;
                        _slmParamEditWin.ShowDialog();
                    }
                    break;
                case SLMParamEditWin.SLMPatternType.Build:
                    if ((null == _slmParamEditWin) && !CompareSLMFiles(EpochSequence))
                    {
                        ROIToolVisible = IsStimulator ?
                           new bool[14] { true, true, true, true, true, true, true, false, false, true, true, true, true, false } :
                           new bool[14] { true, false, false, true, false, false, true, false, false, true, true, true, true, false };

                        _slmParamEditWin = new SLMParamEditWin(this);
                        _slmParamEditWin.Topmost = true;
                        _slmParamEditWin.PanelMode = SLMParamEditWin.SLMPanelMode.Build;
                        _slmParamEditWin.DataContext = this;
                        SetSLMParamEditWinPosition();
                        _slmParamEditWin.Closed += _slmParamEditWin_Closed;
                        _slmParamEditWin.Show();
                        _slmParamEditWin.BuildSequences();
                    }
                    break;
                case SLMParamEditWin.SLMPatternType.Blank:
                    SLMSetBlank();
                    break;
                case SLMParamEditWin.SLMPatternType.ConfigSequence:
                    if (null == _slmParamEditWin)
                    {
                        _slmParamEditWin = new SLMParamEditWin(this);
                        _slmParamEditWin.Topmost = true;
                        _slmParamEditWin.PanelMode = SLMParamEditWin.SLMPanelMode.Sequence;
                        _slmParamEditWin.DataContext = this;
                        SetSLMParamEditWinPosition();
                        _slmParamEditWin.Closed += _slmParamEditWin_Closed;
                        _slmParamEditWin.Show();
                        _slmParamEditWin.Activate();
                    }
                    break;
                case SLMParamEditWin.SLMPatternType.Duplicate:
                    {
                        if (null != _slmParamEditWin) return;

                        DuplicateSelectedSLMPattern();
                    }
                    break;
                case SLMParamEditWin.SLMPatternType.ZRef:
                    //set Z reference position for SLM 3D and display text on button
                    PersistGlobalExperimentXML(GlobalExpAttribute.SLM_ZREF);
                    OnPropertyChanged("SLMZRefText");
                    break;
                case SLMParamEditWin.SLMPatternType.SaveZOffset:
                    DefocusSavedUM = DefocusUM;
                    EditSLMParam("SLM_REBUILDALL"); //rebuild SLM patterns if any
                    break;
                case SLMParamEditWin.SLMPatternType.RebuildAll:
                    if (null != _slmParamEditWin) return;

                    _slmParamEditWin = new SLMParamEditWin(this);
                    _slmParamEditWin.Topmost = true;
                    _slmParamEditWin.PanelMode = SLMParamEditWin.SLMPanelMode.Build;
                    _slmParamEditWin.DataContext = this;
                    SetSLMParamEditWinPosition();
                    _slmParamEditWin.Closed += _slmParamEditWin_Closed;
                    _slmParamEditWin.Show();
                    _slmParamEditWin.ReBuildAll();
                    break;
                default:
                    break;
            }
            OnPropertyChanged("SLMBleachNowEnabled");
        }

        /// <summary>
        /// rearrange SLM pattern file names with certain type in the given directory after deleting target ID number.
        /// </summary>
        /// <param name="targetID"></param>
        /// <param name="directoryName"></param>
        /// <param name="ext"></param>
        private void ReorderSLMPatternFiles(uint targetID, string directoryName, string ext = ".all")
        {
            string numDigits = "D" + FileName.GetDigitCounts().ToString();

            string[] sType = (0 <= ext.IndexOf(".all", StringComparison.OrdinalIgnoreCase)) ? new string[2] { ".bmp", ".txt" } : new string[1] { ext };
            for (int itype = 0; itype < sType.Length; itype++)
            {
                List<string> filesInFolder = Directory.EnumerateFiles(directoryName, "*" + sType[itype], SearchOption.TopDirectoryOnly).ToList();
                for (int i = 0; i < filesInFolder.Count; i++)
                {
                    string[] nameParts = filesInFolder[i].Split('_');
                    int currentID = 0;
                    int lastPartID = (filesInFolder[i].Contains("_Z[") && 3 <= nameParts.Count()) ? 2 : 1;  //intermediate z: xxx_###_[Zxxx]um.ext, others: xxx_###.ext
                    string intStr = (filesInFolder[i].Contains("_Z[") && 3 <= nameParts.Count()) ? nameParts[nameParts.Count() - lastPartID] : nameParts[nameParts.Count() - lastPartID].Split('.')[0];

                    //expect to have default naming scheme
                    if (lastPartID + 1 > nameParts.Count())
                        continue;

                    //shift later ids forward to fill targetID
                    if (int.TryParse(intStr, out currentID) && (targetID <= currentID))
                    {
                        int newID = currentID - 1;
                        if (i < filesInFolder.Count && File.Exists(filesInFolder[i]))
                        {
                            string newName = "";
                            for (int j = 0; j < nameParts.Count() - lastPartID; j++)
                                newName += nameParts[j] + "_";

                            File.Move(filesInFolder[i], newName + newID.ToString(numDigits) + ((filesInFolder[i].Contains("_Z[") && 3 <= nameParts.Count()) ? "_" + nameParts[nameParts.Count() - 1].Substring(0, nameParts[nameParts.Count() - 1].LastIndexOf('.')) : "") + sType[itype]);
                        }
                    }
                }
            }
        }

        /// <summary>
        /// Set SLM calibration file and scale ROIs
        /// </summary>
        private void SetSLMCalibROIFilePath()
        {
            if (1.0 != SLMCalibPatternScaleXY[0] || 1.0 != SLMCalibPatternScaleXY[1])
            {
                SLMCalibFile = System.IO.Path.GetFileNameWithoutExtension(DEFAULT_SLMCALIB_XAML) + "_scale" + System.IO.Path.GetExtension(DEFAULT_SLMCALIB_XAML);
                FileCopyWithAccessCheck(BleachROIPath + DEFAULT_SLMCALIB_XAML, SLMCalibROIFilePath, true);
            }
            else
                SLMCalibFile = DEFAULT_SLMCALIB_XAML;

            //scale ROIs to current image, apply scaling-up ratio of default pattern
            OverlayManagerClass.Instance.ScaleROIs(SLMCalibROIFilePath, new int[] { ImageWidth, ImageHeight });

            //scale ROIs within current image by ratio per user request
            OverlayManagerClass.Instance.ScaleROIsByRatio(SLMCalibROIFilePath, SLMCalibPatternScaleXY);
        }

        private void SetSLMParamEditWinPosition()
        {
            if (null == _slmParamEditWin)
                return;

            XmlDocument appSettings = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

            XmlNodeList ndList = appSettings.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/EditBleachWindow");

            if (ndList.Count > 0)
            {
                string str = string.Empty;
                double val = 0;
                XmlManager.GetAttribute(ndList[0], appSettings, "left", ref str);
                if (double.TryParse(str, out val))
                {
                    _slmParamEditWin.Left = (int)val;
                }
                else
                {
                    _slmParamEditWin.Left = 0;
                }
                XmlManager.GetAttribute(ndList[0], appSettings, "top", ref str);
                if (double.TryParse(str, out val))
                {
                    _slmParamEditWin.Top = (int)val;
                }
                else
                {
                    _slmParamEditWin.Top = 0;
                }
            }
        }

        private void slmBleachWorker_DoWork(object sender, DoWorkEventArgs e)
        {
            BackgroundWorker worker = sender as BackgroundWorker;
            if (ICamera.ScanMode.FORWARD_SCAN >= (ICamera.ScanMode)MVMManager.Instance["ScanControlViewModel", "LSMScanMode", (object)0])
                MVMManager.Instance["ScanControlViewModel", "LastLSMScanMode"] = (int)MVMManager.Instance["ScanControlViewModel", "LSMScanMode", (object)0];

            if (true == worker.CancellationPending)
            {
                e.Cancel = true;
                return;
            }

            if (false == IsBleaching)
            {
                IsBleaching = true;
                IsBleachFinished = false;
                IsBleachStopped = false;

                //stop the background hardware updates
                BWHardware = false;
                SLM3DBackup = SLM3D;

                this._captureSetup.StartSLMBleach();

                //restart the background hardware updates
                BWHardware = true;
            }
        }

        private void slmBleachWorker_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            SLM3D = SLM3DBackup;
        }

        private bool StartSLMBleach()
        {
            //Return when no bleach SLM is selected:
            if ((int)ICamera.LSMType.LSMTYPE_LAST == ResourceManagerCS.GetBleacherType())
            {
                System.Windows.MessageBox.Show("Bleach Scanner has not been selected!");
                return false;
            }
            //stop Live Capture if GALVO_GALVO:
            if ((int)ICamera.LSMType.GALVO_GALVO == ResourceManagerCS.GetLSMType())
            {
                StopLiveCapture();
            }

            //locate files in the folder:
            if (!Directory.Exists(SLMSequenceOn ? SLMWaveformFolder[1] : SLMWaveformFolder[0]))
            {
                MessageBox.Show("No SLM Waveform Folder avaiable!");
                return false;
            }
            var fileList = Directory.EnumerateFiles(SLMSequenceOn ? SLMWaveformFolder[1] : SLMWaveformFolder[0], "*.raw ", SearchOption.TopDirectoryOnly);

            if (null == fileList)
            {
                MessageBox.Show("No SLM Waveforms avaiable!");
                return false;
            }

            if (0 == fileList.Count())
            {
                MessageBox.Show("No SLM Waveforms avaiable!");
                return false;
            }

            if (false == IsBleaching)
            {
                //check if previous task is done:
                if (null != _spinnerWindow)
                {
                    return false;
                }
                if (true == _slmBleachWorker.IsBusy)
                {
                    _slmBleachWorker.CancelAsync();
                    return false;
                }

                //update global params:
                PersistGlobalExperimentXML(GlobalExpAttribute.SLM_BLEACH);

                _slmBleachWorker.RunWorkerAsync();

                CreateProgressWindow();
            }
            return true;
        }

        /// <summary>
        /// remove raw and bmp files based on current SLM param list. 
        /// </summary>
        private void UpdateSLMFiles()
        {
            if (!Directory.Exists(SLMWaveformFolder[0]) || !SLMPanelInUse)
                return;

            string digits = "D" + FileName.GetDigitCounts().ToString();

            var fileList = Directory.EnumerateFiles(SLMWaveformFolder[0], "*.raw ", SearchOption.TopDirectoryOnly).Union(Directory.EnumerateFiles(SLMWaveformFolder[0], "*.bmp ", SearchOption.TopDirectoryOnly)).Union(Directory.EnumerateFiles(SLMWaveformFolder[0], "*.txt ", SearchOption.TopDirectoryOnly));
            if (0 == fileList.Count())
                return;

            List<string> waveFileName = new List<string>();
            for (int i = 0; i < SLMBleachWaveParams.Count; i++)
            {
                string name = SLMWaveformFolder[0] + "\\" + SLMWaveBaseName[0] + "_" + SLMBleachWaveParams[i].BleachWaveParams.ID.ToString(digits) + ".raw";
                string bname = SLMWaveformFolder[0] + "\\" + SLMWaveBaseName[1] + "_" + SLMBleachWaveParams[i].BleachWaveParams.ID.ToString(digits) + ".bmp";
                string tname = SLMWaveformFolder[0] + "\\" + SLMWaveBaseName[1] + "_" + SLMBleachWaveParams[i].BleachWaveParams.ID.ToString(digits) + ".txt";
                waveFileName.Add(name);
                waveFileName.Add(bname);
                waveFileName.Add(tname);

            }

            foreach (string waveFile in fileList)
            {
                if (!waveFileName.Contains(waveFile))
                {
                    ResourceManagerCS.DeleteFile(waveFile);
                }
            }

            //SLM sequence folder
            if (Directory.Exists(SLMWaveformFolder[1]))
            {
                fileList = Directory.EnumerateFiles(SLMWaveformFolder[1], "*.raw ", SearchOption.TopDirectoryOnly).Union(Directory.EnumerateFiles(SLMWaveformFolder[1], "*.txt ", SearchOption.TopDirectoryOnly));
                if (0 < fileList.Count())
                {
                    waveFileName.Clear();
                    for (int i = 1; i <= EpochSequence.Count(); i++)
                    {
                        string name = SLMWaveformFolder[1] + "\\" + SLMWaveBaseName[0] + "_" + i.ToString(digits) + ".raw";
                        string tname = SLMWaveformFolder[1] + "\\" + SLMWaveBaseName[2] + "_" + i.ToString(digits) + ".txt";
                        waveFileName.Add(name);
                        waveFileName.Add(tname);
                    }

                    foreach (string waveFile in fileList)
                    {
                        if (!waveFileName.Contains(waveFile))
                        {
                            ResourceManagerCS.DeleteFile(waveFile);
                        }
                    }
                }
            }

            //iterate through subfolders
            for (int j = 0; j < SLMbmpSubFolders.Length; j++)
            {
                string subFolder = SLMWaveformFolder[0] + SLMbmpSubFolders[j];
                if (!Directory.Exists(subFolder))
                    return;

                fileList = Directory.EnumerateFiles(subFolder, "*.bmp ", SearchOption.TopDirectoryOnly);
                if (0 == fileList.Count())
                    return;

                waveFileName.Clear();
                for (int i = 0; i < SLMBleachWaveParams.Count; i++)
                {
                    string bname = subFolder + "\\" + SLMWaveBaseName[1] + "_" + SLMBleachWaveParams[i].BleachWaveParams.ID.ToString(digits) + ".bmp";
                    string tname = subFolder + "\\" + SLMWaveBaseName[1] + "_" + SLMBleachWaveParams[i].BleachWaveParams.ID.ToString(digits) + ".txt";
                    waveFileName.Add(bname);
                    waveFileName.Add(tname);
                }
                foreach (string waveFile in fileList)
                {
                    if (!waveFileName.Contains(waveFile))
                    {
                        ResourceManagerCS.DeleteFile(waveFile);
                    }
                }
            }

            OnPropertyChanged("SLMBleachNowEnabled");
        }

        private void _slmParamEditWin_Closed(object sender, EventArgs e)
        {
            ApplicationDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
            XmlNode node = ApplicationDoc.SelectSingleNode("/ApplicationSettings/DisplayOptions/CaptureSetup/EditBleachWindow");

            if (null != node && null != _slmParamEditWin)
            {
                string str = string.Empty;

                //// Code below this point is not being reached ////
                XmlManager.SetAttribute(node, ApplicationDoc, "left", ((int)Math.Round(_slmParamEditWin.Left)).ToString());
                XmlManager.SetAttribute(node, ApplicationDoc, "top", ((int)Math.Round(_slmParamEditWin.Top)).ToString());

                MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS);
            }
            _slmParamEditWin = null;
            OverlayManagerClass.Instance.InitSelectROI(ref CaptureSetupViewModel.OverlayCanvas);
            OverlayManagerClass.Instance.DisplayModeROI(ref CaptureSetupViewModel.OverlayCanvas, new ThorSharedTypes.Mode[2] { ThorSharedTypes.Mode.PATTERN_NOSTATS, ThorSharedTypes.Mode.PATTERN_WIDEFIELD }, _slmPatternsVisible);
            OverlayManagerClass.Instance.DisplayModeROI(ref CaptureSetupViewModel.OverlayCanvas, new ThorSharedTypes.Mode[1] { ThorSharedTypes.Mode.STATSONLY }, true);
            OnPropertyChanged("SLMBleachNowEnabled");
        }

        #endregion Methods
    }

    public class SLMEpochSequence : VMBase, ICloneable
    {
        #region Fields

        private int _epochCount = 1;
        private System.Windows.Media.Brush _epochCountBrush = new System.Windows.Media.SolidColorBrush(System.Windows.Media.Colors.Transparent);
        private int _maxEpochValue = 1; //maximum allowed pattern id
        private string _sequenceStr = string.Empty;
        private System.Windows.Media.Brush _sequenceStrBrush = new System.Windows.Media.SolidColorBrush(System.Windows.Media.Colors.Transparent);

        #endregion Fields

        #region Constructors

        public SLMEpochSequence()
        {
        }

        public SLMEpochSequence(int maxEpoch)
        {
            _maxEpochValue = maxEpoch;
        }

        public SLMEpochSequence(string sequence, int epoch, int maxEpoch)
        {
            _sequenceStr = sequence;
            _epochCount = epoch;
            _maxEpochValue = maxEpoch;
        }

        #endregion Constructors

        #region Properties

        public string EpochCount
        {
            get { return (0 < _epochCount) ? _epochCount.ToString() : string.Empty; }
            set
            {
                int val;
                if (value == string.Empty)
                {
                    _epochCount = 0;
                    _epochCountBrush = new System.Windows.Media.SolidColorBrush(System.Windows.Media.Colors.Red);
                }
                else if (int.TryParse(value, out val) && 0 < val)
                {
                    _epochCount = val;
                    _epochCountBrush = new System.Windows.Media.SolidColorBrush(System.Windows.Media.Colors.Transparent);
                    OnPropertyChanged("EpochCountInt");
                    OnPropertyChanged("EpochCount");
                }
                OnPropertyChanged("EpochCountBrush");
            }
        }

        public System.Windows.Media.Brush EpochCountBrush
        {
            get { return _epochCountBrush; }
            set
            {
                _epochCountBrush = value;
                OnPropertyChanged("EpochCountBrush");
            }
        }

        public int EpochCountInt
        {
            get { return _epochCount; }
            set
            {
                _epochCount = value;
                OnPropertyChanged("EpochCountInt");
            }
        }

        public int MaxEpochValue
        {
            get { return _maxEpochValue; }
            set
            {
                if (0 < value)
                {
                    _maxEpochValue = value;
                    OnPropertyChanged("MaxEpochValue");
                }
            }
        }

        public string SequenceStr
        {
            get { return _sequenceStr; }
            set
            {
                string[] items = Regex.Split(value, ":", RegexOptions.IgnorePatternWhitespace);
                int[] seqIDs = ParseForIntArray(value, true);

                if ((seqIDs.Length == items.Length) && _maxEpochValue >= seqIDs.Max())
                {
                    _sequenceStr = value;
                    _sequenceStrBrush = (String.IsNullOrWhiteSpace(_sequenceStr)) ?
                        new System.Windows.Media.SolidColorBrush(System.Windows.Media.Colors.Red) : new System.Windows.Media.SolidColorBrush(System.Windows.Media.Colors.Transparent);
                    OnPropertyChanged("SequenceStr");
                    OnPropertyChanged("SequenceStrBrush");
                }
            }
        }

        public System.Windows.Media.Brush SequenceStrBrush
        {
            get { return _sequenceStrBrush; }
            set
            {
                _sequenceStrBrush = value;
                OnPropertyChanged("SequenceStrBrush");
            }
        }

        #endregion Properties

        #region Methods

        public static string IntArrayToString(int[] intArray, string separator = ":")
        {
            return String.Join(separator, intArray.Select(p => p.ToString()).ToArray());
        }

        public static int[] ParseForIntArray(string strToParse, bool includeSpace = false)
        {
            string[] items = Regex.Split(strToParse, ":", RegexOptions.IgnorePatternWhitespace);
            return items.Select(str =>
            {
                int val = 0;
                bool success = includeSpace ? ((string.IsNullOrWhiteSpace(str)) || int.TryParse(str, out val)) : int.TryParse(str, out val);
                return new { val, success };
            }).Where(pair => pair.success).Select(pair => pair.val).ToArray();
        }

        public object Clone()
        {
            var item = new SLMEpochSequence
            {
                MaxEpochValue = MaxEpochValue,
                SequenceStr = SequenceStr,
                EpochCount = EpochCount,
                EpochCountInt = EpochCountInt
            };
            return item;
        }

        #endregion Methods
    }
}