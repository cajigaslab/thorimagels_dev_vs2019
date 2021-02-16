namespace ThorSharedTypes
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.IO;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Xml;

    using ThorLogging;

    public sealed class ResourceManagerCS : INotifyPropertyChanged
    {
        #region Fields

        const int PATH_LENGTH = 261;

        private static readonly ResourceManagerCS instance = new ResourceManagerCS();

        private static string _lastSavedModality = string.Empty;
        static ObservableCollection<string> _modalities = new ObservableCollection<string>();

        private PMTSwitch _pmtSwitchController = new PMTSwitch();
        private bool _tabletMode = false;

        #endregion Fields

        #region Constructors

        private ResourceManagerCS()
        {
        }

        #endregion Constructors

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public static ResourceManagerCS Instance
        {
            get
            {
                return instance;
            }
        }

        public static ObservableCollection<string> Modalities
        {
            get
            {
                return _modalities;
            }
            set
            {
                _modalities = value;
            }
        }

        public static string SetActiveModality
        {
            get
            {
                return GetModality();
            }
            set
            {
                SetModality(value);
                Instance.OnPropertyChanged("ActiveModality");
            }
        }

        public string ActiveModality
        {
            get
            {
                return GetModality();
            }
            set
            {
                //Need to check for Null because when _modalities.clear is called it will try to set the ActiveModality to null
                if (null != value)
                {
                    SetModalityPersistLast(value);
                    OnPropertyChanged("ActiveModality");

                    //Because the HWSettings file has changed, the data structures in SelectHW must be updated
                    UpdateAndPersistCurrentDevices();

                    //The active camera might've changed with the modality, set the PMT switch box to the right position
                    UpdatePMTSwitchBox();
                }
            }
        }

        //Return true if the controlling board is ThorDAQ
        public bool IsThorDAQBoard
        {
            get
            {
                string lsmName = GetActiveLSMName();
                return lsmName.ToUpper().Contains("THORDAQ");
            }
        }

        public bool TabletModeEnabled
        {
            get
            {
                return _tabletMode;
            }
            set
            {
                _tabletMode = value;
            }
        }

        #endregion Properties

        #region Methods

        public static bool BorrowDocMutexCS(SettingsFileType sfType, int tryTimeMSec = -1)
        {
            return BorrowDocMutex(sfType, tryTimeMSec);
        }

        public static long DateTimeToUnixTimestamp(DateTime dateTime)
        {
            return (long)(TimeZoneInfo.ConvertTimeToUtc(dateTime) -
                   new DateTime(1970, 1, 1, 0, 0, 0, 0, System.DateTimeKind.Utc)).TotalSeconds;
        }

        [DllImport(".\\Modules_Native\\SelectHardware.dll", EntryPoint = "DeslectCameras")]
        public static extern int DeslectCameras();

        public static void FileCopyWithExistCheck(string src, string dst, bool overwrite)
        {
            if (File.Exists(src))
            {
                File.Copy(src, dst, overwrite);
            }
        }

        public static string GetActiveSettingsFileString()
        {
            StringBuilder sb = new StringBuilder(PATH_LENGTH);
            GetActiveSettingsFilePathAndName(sb, PATH_LENGTH);
            return sb.ToString();
        }

        public static string GetApplicationSettingsFileString()
        {
            StringBuilder sb = new StringBuilder(PATH_LENGTH);
            GetApplicationSettingsFilePathAndName(sb, PATH_LENGTH);
            return sb.ToString();
        }

        public static int GetBleacherType()
        {
            int lsmType = (int)ICamera.LSMType.LSMTYPE_LAST;

            GetCameraParamInt((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_TYPE, ref lsmType);

            return lsmType;
        }

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "GetCameraParamAvailable")]
        public static extern int GetCameraParamAvailable(int cameraSelection, int paramId);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "GetCameraParamDouble")]
        public static extern int GetCameraParamDouble(int cameraSelection, int param, ref double value);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "GetCameraParamLong")]
        public static extern int GetCameraParamInt(int cameraSelection, int param, ref int value);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "GetCameraParamRangeDouble")]
        public static extern int GetCameraParamRangeDouble(int cameraSelection, int paramId, ref double valMin, ref double valMax, ref double valDefault);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "GetCameraParamRangeLong")]
        public static extern int GetCameraParamRangeInt(int cameraSelection, int paramId, ref int valMin, ref int valMax, ref int valDefault);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "GetCameraParamReadOnly")]
        public static extern int GetCameraParamReadOnly(int cameraSelection, int paramId);

        public static string GetCameraParamString(int cameraSelction, int param)
        {
            StringBuilder sb = new StringBuilder(PATH_LENGTH);
            GetCameraParamStr(cameraSelction, param, sb, PATH_LENGTH);
            return sb.ToString();
        }

        public static int GetCameraType()
        {
            int cameraType = 0;

            GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CAMERA_TYPE, ref cameraType);

            return cameraType;
        }

        public static string GetCaptureTemplatePathString()
        {
            const int PATH_LENGTH = 261;
            StringBuilder sb = new StringBuilder(PATH_LENGTH);
            GetCaptureTemplatePath(sb, PATH_LENGTH);

            return sb.ToString();
        }

        public static int GetCCDType()
        {
            int ccdType = (int)ICamera.CCDType.CCDTYPE_LAST;

            GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_CCD_TYPE, ref ccdType);

            return ccdType;
        }

        public static string GetDeviceError(int deviceSelection)
        {
            StringBuilder sb = new StringBuilder(PATH_LENGTH);
            GetDeviceErrorMessage(deviceSelection, sb, PATH_LENGTH);
            return sb.ToString();
        }

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "GetDeviceParamAvailable")]
        public static extern int GetDeviceParamAvailable(int deviceSelection, int paramId);

        public static int GetDeviceParamBuffer<T>(int deviceSelection, int paramId, T[] buf, long len)
        {
            int ret = 0;
            var gch = default(GCHandle);
            try
            {
                gch = GCHandle.Alloc(buf, GCHandleType.Pinned);
                ret = GetDeviceParamBuffer(deviceSelection, paramId, gch.AddrOfPinnedObject(), len);
            }
            finally
            {
                if (gch.IsAllocated)
                    gch.Free();
            }
            return ret;
        }

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "GetDeviceParamBuffer")]
        public static extern int GetDeviceParamBuffer(int deviceSelection, int paramId, IntPtr buf, long len);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "GetDeviceParamDouble")]
        public static extern int GetDeviceParamDouble(int deviceSelection, int paramId, ref double param);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "GetDeviceParamLong")]
        public static extern int GetDeviceParamInt(int deviceSelection, int paramId, ref int param);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "GetDeviceParamRangeDouble")]
        public static extern int GetDeviceParamRangeDouble(int deviceSelection, int paramId, ref double valMin, ref double valMax, ref double valDefault);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "GetDeviceParamRangeLong")]
        public static extern int GetDeviceParamRangeInt(int deviceSelection, int paramId, ref int valMin, ref int valMax, ref int valDefault);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "GetDeviceParamReadOnly")]
        public static extern int GetDeviceParamReadOnly(int deviceSelection, int paramId);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "GetDeviceParamString")]
        public static extern int GetDeviceParamString(int deviceSelection, int paramId, [Out, MarshalAsAttribute(UnmanagedType.LPWStr)] StringBuilder paramString, long len);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "GetDeviceStatus")]
        public static extern int GetDeviceStatus(int deviceSelection, ref int status);

        public static string GetHardwareSettingsFileString()
        {
            StringBuilder sb = new StringBuilder(PATH_LENGTH);
            GetHardwareSettingsFilePathAndName(sb, PATH_LENGTH);
            return sb.ToString();
        }

        public static string GetLastExperimentSettingsFileString()
        {
            StringBuilder sb = new StringBuilder(PATH_LENGTH);
            GetActiveExperimentPathAndName(sb, PATH_LENGTH);
            return sb.ToString();
        }

        public static int GetLSMType()
        {
            int lsmType = (int)ICamera.LSMType.LSMTYPE_LAST;

            GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_TYPE, ref lsmType);

            return lsmType;
        }

        public static string GetModality()
        {
            StringBuilder modalityName = new StringBuilder(PATH_LENGTH);
            GetModality(modalityName, PATH_LENGTH);
            return modalityName.ToString();
        }

        public static string GetModalityApplicationSettingsFileString(string modality)
        {
            StringBuilder sb = new StringBuilder(PATH_LENGTH);
            GetModalityApplicationSettingsFilePathAndName(modality, sb, PATH_LENGTH);
            return sb.ToString();
        }

        public static string GetModalityHardwareSettingsFileString(string modality)
        {
            StringBuilder sb = new StringBuilder(PATH_LENGTH);
            GetModalityHardwareSettingsFilePathAndName(modality, sb, PATH_LENGTH);
            return sb.ToString();
        }

        public static string GetMyDocumentsThorImageFolderString()
        {
            const int PATH_LENGTH = 261;
            StringBuilder sb = new StringBuilder(PATH_LENGTH);
            GetMyDocumentsThorImageFolder(sb, PATH_LENGTH);

            return sb.ToString();
        }

        public static void LoadModalities()
        {
            string modalitiesFolder = GetMyDocumentsThorImageFolderString() + "Modalities";

            _modalities.Clear();

            if (Directory.Exists(modalitiesFolder))
            {
                string[] mods = Directory.GetDirectories(modalitiesFolder);

                foreach (string mod in mods)
                {
                    _modalities.Add(new DirectoryInfo(mod).Name);
                }
            }
            Instance.OnPropertyChanged("ActiveModality");

            //Because the HWSettings file has changed, the data structures in SelectHW must be updated
            UpdateAndPersistCurrentDevices();

            //The active camera might've changed with the modality, set the PMT switch box to the right position
            Instance.UpdatePMTSwitchBox();
        }

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "PostflightCamera")]
        public static extern int PostflightCamera(int cameraSelection);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "PreflightCamera")]
        public static extern int PreflightCamera(int cameraSelection);

        [DllImport(".\\ResourceManager.dll", EntryPoint = "LoadSettings")]
        public static extern int ResourceManagerLoadSettings();

        public static bool ReturnDocMutexCS(SettingsFileType sfType)
        {
            return ReturnDocMutex(sfType);
        }

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "SetCameraParamDouble")]
        public static extern int SetCameraParamDouble(int cameraSelection, int param, double value);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "SetCameraParamLong")]
        public static extern int SetCameraParamInt(int cameraSelection, int param, int value);

        public static int SetCameraParamString(int cameraSelection, int param, string value)
        {
            return SetCameraParamString(cameraSelection, param, new StringBuilder(value));
        }

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "SetDeviceParamBuffer")]
        public static extern int SetDeviceParamBuffer(int deviceSelection, int paramId, byte[] buf, long len, int exeOrWait);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "SetDeviceParamDouble")]
        public static extern int SetDeviceParamDouble(int deviceSelection, int paramId, double param, int exeOrWait);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "SetDeviceParamLong")]
        public static extern int SetDeviceParamInt(int deviceSelection, int paramId, int param, int exeOrWait);

        public static int SetDeviceParamString(int deviceSelection, int param, string value, int exeOrWait)
        {
            return SetDeviceParamString(deviceSelection, param, new StringBuilder(value), exeOrWait);
        }

        /// <summary>
        /// Set modality without updating last active xml
        /// </summary>
        /// <param name="modality"></param>
        public static void SetModality(string modality)
        {
            string str = string.Empty;
            string strActiveExp = GetCaptureTemplatePathString() + "Active.xml";
            XmlDocument activeDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS];
            XmlNodeList ndList = (null != activeDoc) ? activeDoc.SelectNodes("/ThorImageExperiment/Modality") : null;

            if (!Directory.Exists(ResourceManagerCS.GetMyDocumentsThorImageFolderString() + "Modalities\\" + modality))
            {
                MessageBox.Show("Cannot find modality folder: " + modality + "\nCould not switch Modalities");
                return;
            }

            /// [load next modality]
            // Check if the template being loaded is from a version before the modalities feature was implemented. Then the
            // experiment file would not have a Modality tag
            if (null != ndList)
            {
                if (0 == ndList.Count)
                {
                    str = ResourceManagerCS.GetMyDocumentsThorImageFolderString() + "Modalities\\" + modality + "\\Active.xml";
                    if (File.Exists(str))
                    {
                        XmlDocument modActiveDoc = new XmlDocument();
                        modActiveDoc.Load(str);
                        XmlNode nl = modActiveDoc.SelectSingleNode("/ThorImageExperiment/Modality");
                        if (null == nl)
                        {
                            XmlManager.CreateXmlNode(modActiveDoc, "Modality");
                            modActiveDoc.Save(str);
                            nl = modActiveDoc.SelectSingleNode("/ThorImageExperiment/Modality");
                        }
                        XmlNode newNl = activeDoc.ImportNode(nl, true);
                        activeDoc.DocumentElement.AppendChild(newNl);
                        activeDoc.Save(strActiveExp);
                    }
                }
                else
                {
                    //copy if modality is mismatched with current
                    if (null != activeDoc)
                    {
                        if (XmlManager.GetAttribute(ndList[0], activeDoc, "name", ref str) && !modality.Equals(str))
                        {
                            try
                            {
                                string mod = GetMyDocumentsThorImageFolderString() + "Modalities\\" + modality;
                                FileCopyWithExistCheck(mod + "\\Active.xml", strActiveExp, true);

                                if (Directory.Exists(mod + "\\AlignData"))
                                {
                                    foreach (string f in Directory.GetFiles(mod + "\\AlignData", "AlignData*.txt"))
                                    {
                                        FileCopyWithExistCheck(f, Directory.GetCurrentDirectory() + "\\" + Path.GetFileName(f), true);
                                    }
                                }

                                _lastSavedModality = modality;
                            }
                            catch (Exception ex)
                            {
                                ThorLogging.ThorLog.Instance.TraceEvent(System.Diagnostics.TraceEventType.Error, 1, "Copy modality error: " + ex.Message);
                            };
                        }
                    }
                }
            }
            SetModality(new StringBuilder(modality));
            MVMManager.Instance.LoadSettings();
        }

        /// <summary>
        /// Set modality with persisting last active xml
        /// </summary>
        /// <param name="modality"></param>
        public static void SetModalityPersistLast(string modality)
        {
            string str = string.Empty;
            string strActiveExp = GetCaptureTemplatePathString() + "Active.xml";
            XmlDocument activeDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS];
            XmlNodeList ndList = (null != activeDoc) ? activeDoc.SelectNodes("/ThorImageExperiment/Modality") : null;

            if (!Directory.Exists(ResourceManagerCS.GetMyDocumentsThorImageFolderString() + "Modalities\\" + modality))
            {
                MessageBox.Show("Cannot find modality folder: " + modality + "\nCould not switch Modalities");
                return;
            }

            /// [persist last modality]
            if (null != ndList)
            {
                //persist current modality as last if no history available
                _lastSavedModality = (0 >= _lastSavedModality.Length) ? modality : _lastSavedModality;

                //update settings from all mvm
                MVMManager.Instance.UpdateMVMXMLSettings(ref MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS]);

                //update modality name
                XmlManager.SetAttribute(ndList[0], activeDoc, "name", _lastSavedModality);
                MVMManager.Instance.SaveSettings(SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS);

                //back up exp settings to the modality folder:
                if (Directory.Exists(GetMyDocumentsThorImageFolderString() + "Modalities\\" + _lastSavedModality))
                {
                    string mod = GetMyDocumentsThorImageFolderString() + "Modalities\\" + _lastSavedModality;
                    FileCopyWithExistCheck(GetActiveSettingsFileString(), mod + "\\Active.xml", true);

                    if (!Directory.Exists(mod + "\\AlignData")) Directory.CreateDirectory(mod + "\\AlignData");

                    foreach (string f in Directory.GetFiles(Directory.GetCurrentDirectory(), "AlignData*.txt"))
                    {
                        FileCopyWithExistCheck(f, mod + "\\AlignData" + "\\" + Path.GetFileName(f), true);
                    }
                }

                //deselect camera and bleach scanner before switching modality
                DeslectCameras();
            }

            /// [load next modality]
            // Check if the template being loaded is from a version before the modalities feature was implemented. Then the
            // experiment file would not have a Modality tag
            if (null != ndList)
            {
                if (0 == ndList.Count)
                {
                    str = ResourceManagerCS.GetMyDocumentsThorImageFolderString() + "Modalities\\" + modality + "\\Active.xml";
                    if (File.Exists(str))
                    {
                        XmlDocument modActiveDoc = new XmlDocument();
                        modActiveDoc.Load(str);
                        XmlNode nl = modActiveDoc.SelectSingleNode("/ThorImageExperiment/Modality");
                        XmlNode newNl = activeDoc.ImportNode(nl, true);
                        activeDoc.DocumentElement.AppendChild(newNl);
                        activeDoc.Save(strActiveExp);
                    }
                }
                else
                {
                    //copy if modality is mismatched with current
                    if (XmlManager.GetAttribute(ndList[0], activeDoc, "name", ref str) && !modality.Equals(str))
                    {
                        try
                        {
                            string mod = GetMyDocumentsThorImageFolderString() + "Modalities\\" + modality;
                            FileCopyWithExistCheck(mod + "\\Active.xml", strActiveExp, true);

                            if (Directory.Exists(mod + "\\AlignData"))
                            {
                                foreach (string f in Directory.GetFiles(mod + "\\AlignData", "AlignData*.txt"))
                                {
                                    FileCopyWithExistCheck(f, Directory.GetCurrentDirectory() + "\\" + Path.GetFileName(f), true);
                                }
                            }
                        }
                        catch (Exception ex)
                        {
                            ThorLogging.ThorLog.Instance.TraceEvent(System.Diagnostics.TraceEventType.Error, 1, "Copy modality error: " + ex.Message);
                        };
                    }
                }
            }
            SetModality(new StringBuilder(modality));

            //keep last modality if not entry (ndList is null) and load from current active
            if (null != ndList)
                _lastSavedModality = GetModality();

            MVMManager.Instance.LoadSettings();
        }

        public static T StructFromByteArray<T>(byte[] array)
        {
            T str = default(T);

            int size = Marshal.SizeOf(str);
            IntPtr ptr = Marshal.AllocHGlobal(size);

            Marshal.Copy(array, 0, ptr, size);

            str = (T)Marshal.PtrToStructure(ptr, str.GetType());
            Marshal.FreeHGlobal(ptr);

            return str;
        }

        public static byte[] StructToByteArray(object obj)
        {
            int size = Marshal.SizeOf(obj);
            byte[] array = new byte[size];
            IntPtr ptr = Marshal.AllocHGlobal(size);

            Marshal.StructureToPtr(obj, ptr, true);
            Marshal.Copy(ptr, array, 0, size);
            Marshal.FreeHGlobal(ptr);

            return array;
        }

        public static DateTime ToDateTimeFromUnix(long intDate)
        {
            var timeInTicks = intDate * TimeSpan.TicksPerSecond;
            return new DateTime(1970, 1, 1, 0, 0, 0, 0).AddTicks(timeInTicks).ToLocalTime();
        }

        public int ConnectToPMTSwitchBox()
        {
            int ret = 0;
            string str = string.Empty;

            if (File.Exists("ThorDetectorSwitchSettings.xml"))
            {
                XmlDocument detetorSwitchSettings = new XmlDocument();
                detetorSwitchSettings.Load("ThorDetectorSwitchSettings.xml");
                XmlNodeList ndList = (null != detetorSwitchSettings) ? detetorSwitchSettings.SelectNodes("/DetectorSwitchSettings/SerialNumber") : null;
                if (null != ndList)
                {
                    XmlManager.GetAttribute(ndList[0], detetorSwitchSettings, "sn", ref str);
                }
            }
            ret = _pmtSwitchController.Connect(str) ? 1 : 0;
            return ret;
        }

        public string GetActiveLSMName()
        {
            XmlDocument hwDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];
            XmlNodeList ndListHW = hwDoc.SelectNodes("/HardwareSettings/ImageDetectors/LSM");
            if (ndListHW.Count > 0)
            {
                string str = string.Empty;
                for (int i = 0; i < ndListHW.Count; i++)
                {
                    XmlManager.GetAttribute(ndListHW[i], hwDoc, "active", ref str);
                    if (1 == Convert.ToInt32(str))
                    {
                        if (XmlManager.GetAttribute(ndListHW[i], hwDoc, "cameraName", ref str))
                        {
                            return str;
                        }
                    }
                }
            }
            return string.Empty;
        }

        public void UpdatePMTSwitchBox()
        {
            byte toggle = 0; //Toggle all off by default 0000

            //only switch when changing to an LSM camera
            if ((int)ICamera.CameraType.LSM == GetCameraType())
            {
                int lsmType = GetLSMType();

                //switch depending on LSM camera type
                switch ((ICamera.LSMType)lsmType)
                {
                    case ICamera.LSMType.GALVO_RESONANCE:
                        _pmtSwitchController.Set_SwitchesPort(toggle);

                        SetDeviceParamInt((int)SelectedHardware.SELECTED_PMTSWITCH, (int)IDevice.Params.PARAM_PMT_SWITCH_POS, 0, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT);

                        break;
                    case ICamera.LSMType.GALVO_GALVO:
                        toggle = 15; //Toggle all on 1111
                        _pmtSwitchController.Set_SwitchesPort(toggle);

                        SetDeviceParamInt((int)SelectedHardware.SELECTED_PMTSWITCH, (int)IDevice.Params.PARAM_PMT_SWITCH_POS, 0, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT);
                        break;
                }
            }
        }

        [DllImport(".\\ResourceManager.dll", EntryPoint = "BorrowDocMutex")]
        private static extern bool BorrowDocMutex(SettingsFileType sfType, int tryTimeMSec);

        [DllImport(".\\ExperimentManager.dll", EntryPoint = "GetActiveExperimentPathAndName", CharSet = CharSet.Unicode)]
        private static extern int GetActiveExperimentPathAndName(StringBuilder path, int length);

        [DllImport(".\\ResourceManager.dll", EntryPoint = "GetActiveSettingsFilePathAndName", CharSet = CharSet.Unicode)]
        private static extern int GetActiveSettingsFilePathAndName(StringBuilder path, int length);

        [DllImport(".\\ResourceManager.dll", EntryPoint = "GetApplicationSettingsFilePathAndName", CharSet = CharSet.Unicode)]
        private static extern int GetApplicationSettingsFilePathAndName(StringBuilder sb, int length);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "GetCameraParamString", CharSet = CharSet.Unicode)]
        private static extern int GetCameraParamStr(int cameraSelection, int param, StringBuilder str, int size);

        [DllImport(".\\ResourceManager.dll", EntryPoint = "GetCaptureTemplatePath", CharSet = CharSet.Unicode)]
        private static extern int GetCaptureTemplatePath(StringBuilder sb, int length);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "GetDeviceErrorMessage", CharSet = CharSet.Unicode)]
        private static extern int GetDeviceErrorMessage(int deviceSelection, StringBuilder errorMessage, int length);

        [DllImport(".\\ResourceManager.dll", EntryPoint = "GetHardwareSettingsFilePathAndName", CharSet = CharSet.Unicode)]
        private static extern int GetHardwareSettingsFilePathAndName(StringBuilder sb, int length);

        [DllImport(".\\ResourceManager.dll", EntryPoint = "GetModality", CharSet = CharSet.Unicode)]
        private static extern int GetModality(StringBuilder sb, int length);

        [DllImport(".\\ResourceManager.dll", EntryPoint = "GetModalityApplicationSettingsFilePathAndName", CharSet = CharSet.Unicode)]
        private static extern int GetModalityApplicationSettingsFilePathAndName(string modality, StringBuilder sb, int length);

        [DllImport(".\\ResourceManager.dll", EntryPoint = "GetModalityHardwareSettingsFilePathAndName", CharSet = CharSet.Unicode)]
        private static extern int GetModalityHardwareSettingsFilePathAndName(string modality, StringBuilder sb, int length);

        [DllImport(".\\ResourceManager.dll", EntryPoint = "GetMyDocumentsThorImageFolder", CharSet = CharSet.Unicode)]
        private static extern int GetMyDocumentsThorImageFolder(StringBuilder sb, int length);

        [DllImport(".\\ResourceManager.dll", EntryPoint = "ReturnDocMutex")]
        private static extern bool ReturnDocMutex(SettingsFileType sfType);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "SetCameraParamString", CharSet = CharSet.Unicode)]
        private static extern int SetCameraParamString(int cameraSelection, int param, StringBuilder value);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "SetDeviceParamString", CharSet = CharSet.Unicode)]
        private static extern int SetDeviceParamString(int deviceSelection, int paramId, StringBuilder value, int exeOrWait);

        [DllImport(".\\ResourceManager.dll", EntryPoint = "SetModality", CharSet = CharSet.Unicode)]
        private static extern int SetModality(StringBuilder sb);

        [DllImport(".\\Modules_Native\\SelectHardware.dll", EntryPoint = "UpdateAndPersistCurrentDevices")]
        private static extern int UpdateAndPersistCurrentDevices();

        private void OnPropertyChanged(string propertyName)
        {
            var handle = PropertyChanged;
            if (handle != null)
            {
                handle(this, new PropertyChangedEventArgs(propertyName));
            }
        }

        #endregion Methods
    }
}