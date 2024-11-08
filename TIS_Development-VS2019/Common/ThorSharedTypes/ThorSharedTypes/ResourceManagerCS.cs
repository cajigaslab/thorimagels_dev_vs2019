﻿namespace ThorSharedTypes
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

        private bool _allowSwitchBox = false;
        private PMTSwitch _pmtSwitchController = new PMTSwitch();
        private PMTSwitch _pmtSwitchController2 = new PMTSwitch();
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
                }
            }
        }

        public bool AllowSwitchBoxToWork
        {
            get => _allowSwitchBox;
            set => _allowSwitchBox = value;
        }

        public BleachMode GetBleachMode
        {
            get
            {
                BleachMode bMode = BleachMode.BLEACH;
                XmlDocument hwSettings = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];
                XmlNodeList ndListHW = hwSettings.SelectNodes("/HardwareSettings/Devices/SLM");
                for (int i = 0; i < ndListHW.Count; i++)
                {
                    string str = string.Empty;

                    if (XmlManager.GetAttribute(ndListHW[i], hwSettings, "dllName", ref str) && (str.Contains("ThorSLM")))
                    {
                        bMode = (XmlManager.GetAttribute(ndListHW[i], hwSettings, "active", ref str) && (0 == str.CompareTo("1"))) ?
                            BleachMode.SLM : BleachMode.BLEACH;
                    }
                }
                return bMode;
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

        /// <summary>
        /// Method to backup a specified directory. Checks if directory exists and creates if not. 
        /// </summary>
        /// <param name="folderPath">Path to the folder to be backed up. Path should end with '\'</param>
        /// <returns>Returns the number of errors that ocurred while copying to the backup folder or -1 for a file or folder error</returns>
        public static int BackupDirectory(string folderPath)
        {
            string allowedExtenstions = ".xml | .txt | .db";
            string excludedFiles = "ThorLogging.xml";
            int errorNum = 0;

            string backupFolderPath = folderPath + "Backup";
            if (!Directory.Exists(backupFolderPath))
            {
                try
                {
                    Directory.CreateDirectory(backupFolderPath);
                }
                catch (Exception e)
                {
                    ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "ResourceManager " + e.GetType() + "Unable to create the requested directory");
                    return -1;
                }
            }
            else
            {
                try
                {
                    Directory.Delete(backupFolderPath, true);
                }
                catch (Exception e)
                {
                    ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "ResourceManager " + e.GetType() + "Unable to delete the requested directory");
                    return -1;
                }
            }

            string[] filesInFolder = Directory.GetFiles(folderPath, ".", SearchOption.AllDirectories);
            foreach (string filePath in filesInFolder)
            {
                FileInfo info = new FileInfo(filePath);
                if (allowedExtenstions.Contains(info.Extension) && !excludedFiles.Contains(info.Name))
                {
                    string relativeFilePath = info.DirectoryName.Substring(Directory.GetParent(backupFolderPath).FullName.Length);
                    string pathToBackupFile;

                    //Relative path will include the '\' character if it is not empty
                    if (0 < relativeFilePath.Length)
                    {
                        pathToBackupFile = backupFolderPath + relativeFilePath;
                    }
                    else
                    {
                        pathToBackupFile = backupFolderPath;
                    }

                    if (!Directory.Exists(pathToBackupFile))
                    {
                        try
                        {
                            Directory.CreateDirectory(pathToBackupFile);
                        }
                        catch (Exception e)
                        {
                            ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "ResourceManager " + e.GetType() + "Unable to create the requested directory");
                            return -1;
                        }
                    }
                    try
                    {
                        File.Copy(info.FullName, pathToBackupFile + "\\" + info.Name);
                    }
                    catch (Exception e)
                    {
                        ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "ResourceManager " + e.GetType() + " Error During Backup: " + e.Message);
                        errorNum++;
                    }
                }
            }
            return errorNum;
        }

        public static bool BorrowDocMutexCS(SettingsFileType sfType, int tryTimeMSec = -1)
        {
            return BorrowDocMutex(sfType, tryTimeMSec);
        }

        public static long DateTimeToUnixTimestamp(DateTime dateTime)
        {
            return (long)(TimeZoneInfo.ConvertTimeToUtc(dateTime) -
                   new DateTime(1970, 1, 1, 0, 0, 0, 0, System.DateTimeKind.Utc)).TotalSeconds;
        }

        public static void DeleteDirectory(string target_dir)
        {
            if (!Directory.Exists(target_dir))
                return;
            try
            {
                DirectoryInfo dirInfo = new DirectoryInfo(target_dir);
                foreach (FileInfo file in dirInfo.GetFiles())
                {
                    file.Delete();
                }
                foreach (DirectoryInfo dir in dirInfo.GetDirectories())
                {
                    dir.Delete(true);
                }

                Directory.Delete(target_dir, false);
            }
            catch (System.IO.IOException ex)
            {
                ThorLogging.ThorLog.Instance.TraceEvent(System.Diagnostics.TraceEventType.Verbose, 1, "DeleteDirectory error: " + ex.Message);
            }
        }

        public static void DeleteFile(string target_file)
        {
            try
            {
                if (File.Exists(target_file))
                {
                    File.Delete(target_file);
                }
            }
            catch (Exception ex)
            {
                ThorLogging.ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, "DeleteFile '" + target_file + "': " + ex.Message);
            }
        }

        [DllImport(".\\Modules_Native\\SelectHardware.dll", EntryPoint = "DeslectCameras")]
        public static extern int DeslectCameras();

        public static void FileCopyWithExistCheck(string src, string dst, bool overwrite)
        {
            if (File.Exists(src))
            {
                if (!Directory.Exists(System.IO.Path.GetDirectoryName(dst)))
                    Directory.CreateDirectory(System.IO.Path.GetDirectoryName(dst));

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

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "GetCameraParamBuffer")]
        public static extern int GetCameraParamBuffer(long cameraSelection, long paramID, IntPtr pBuffer, long size);

        public static int GetCameraParamBuffer<T>(int deviceSelection, int paramId, T[] buf, long len)
        {
            int ret = 0;
            var gch = default(GCHandle);
            try
            {
                gch = GCHandle.Alloc(buf, GCHandleType.Pinned);
                ret = GetCameraParamBuffer(deviceSelection, paramId, gch.AddrOfPinnedObject(), len);
            }
            finally
            {
                if (gch.IsAllocated)
                    gch.Free();
            }
            return ret;
        }

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

        public static string GetRegistrationFileString()
        {
            string regFile = GetCaptureTemplatePathString() + "\\" + "Registration.xml";
            if (!File.Exists(regFile))
            {
                using (XmlWriter writer = XmlWriter.Create(regFile))
                {
                    writer.WriteStartElement("ThorImageRegistration");
                    writer.WriteElementString("Registrations","");
                    writer.Flush();
                }
            }
            return regFile;
        }

        public static string GetStartupFlag()
        {
            StringBuilder sb = new StringBuilder(256);
            GetStartupFlag(sb, 256);
            return sb.ToString();
        }

        public static string GetValueString(string xPath, string attrName)
        {
            XmlDocument appSettings = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
            string tmp = string.Empty;
            XmlNodeList ndList = appSettings.SelectNodes(xPath);
            if (ndList.Count > 0)
            {
                if (null != ndList[0].Attributes.GetNamedItem(attrName))
                {
                    tmp = ndList[0].Attributes[attrName].Value;
                }
            }
            return tmp;
        }

        public static Visibility GetVisibility(string xPath, string attrName)
        {
            XmlDocument appSettings = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

            XmlNodeList ndList = appSettings.SelectNodes(xPath);
            if (ndList.Count > 0)
            {
                string tmp = string.Empty;
                if (null != ndList[0].Attributes.GetNamedItem(attrName))
                {
                    tmp = ndList[0].Attributes[attrName].Value;
                    return tmp.Equals("Visible") ? Visibility.Visible : Visibility.Collapsed;
                }
            }
            return Visibility.Collapsed;
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

        public static bool ReloadDirectories()
        {
            return ResourceManagerReloadDirectories() == 0 ? true : false;
        }

        [DllImport(".\\ResourceManager.dll", EntryPoint = "LoadSettings")]
        public static extern int ResourceManagerLoadSettings();

        [DllImport(".\\ResourceManager.dll", EntryPoint = "ReloadDirectories")]
        public static extern int ResourceManagerReloadDirectories();

        /// <summary>
        /// Method to restore a specified directory that has been previously backed up.
        /// </summary>
        /// <param name="folderPath">Path to the folder to be backed up. Path should end with '\'</param>
        /// <returns>Returns the number of errors that ocurred while copying from the backup folder or -1 for a file or folder error</returns>
        public static int RestoreDirectory(string folderPath)
        {
            string backupFolderPath = folderPath + "Backup";
            int errorNum = 0;

            if (!Directory.Exists(backupFolderPath))
            {
                return -1;
            }

            string[] filesInBackupFolder = Directory.GetFiles(backupFolderPath, ".", SearchOption.AllDirectories);
            foreach (string filePath in filesInBackupFolder)
            {
                FileInfo info = new FileInfo(filePath);
                string relativeFilePath = info.DirectoryName.Substring(backupFolderPath.Length);
                string pathToRestoredFile;

                //Relative path will include the '\' character if it is not empty
                if (0 < relativeFilePath.Length)
                {
                    pathToRestoredFile = folderPath.Remove(folderPath.Length - 1, 1) + relativeFilePath;
                }
                else
                {
                    pathToRestoredFile = folderPath;
                }
                try
                {
                    File.Copy(info.FullName, pathToRestoredFile + "\\" + info.Name, true);
                }
                catch (Exception e)
                {
                    ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "ResourceManager " + e.GetType() + " Error during Restore: " + e.Message);
                    errorNum++;
                }
            }
            return errorNum;
        }

        public static bool ReturnDocMutexCS(SettingsFileType sfType)
        {
            return ReturnDocMutex(sfType);
        }

        public static void SafeCreateDirectory(string target_dir)
        {
            if (!Directory.Exists(target_dir))
                Directory.CreateDirectory(target_dir);
        }

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "SetCameraParamBuffer")]
        public static extern int SetCameraParamBuffer(long cameraSelection, long paramID, IntPtr pBuffer, long size);

        public static int SetCameraParamBuffer<T>(int deviceSelection, int paramId, T[] buf, long len)
        {
            int ret = 0;
            var gch = default(GCHandle);
            try
            {
                gch = GCHandle.Alloc(buf, GCHandleType.Pinned);
                ret = SetCameraParamBuffer(deviceSelection, paramId, gch.AddrOfPinnedObject(), len);
            }
            finally
            {
                if (gch.IsAllocated)
                    gch.Free();
            }
            return ret;
        }

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "SetCameraParamDouble")]
        public static extern int SetCameraParamDouble(int cameraSelection, int param, double value);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "SetCameraParamLong")]
        public static extern int SetCameraParamInt(int cameraSelection, int param, int value);

        public static int SetCameraParamString(int cameraSelection, int param, string value)
        {
            return SetCameraParamString(cameraSelection, param, new StringBuilder(value));
        }

        public static int SetDeviceParamBuffer<T>(int deviceSelection, int paramId, T[] buf, long len, int exeOrWait)
        {
            int ret = 0;
            var gch = default(GCHandle);
            try
            {
                gch = GCHandle.Alloc(buf, GCHandleType.Pinned);
                ret = SetDeviceParamBuffer(deviceSelection, paramId, gch.AddrOfPinnedObject(), len, exeOrWait);
            }
            finally
            {
                if (gch.IsAllocated)
                    gch.Free();
            }
            return ret;
        }

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "SetDeviceParamBuffer")]
        public static extern int SetDeviceParamBuffer(int deviceSelection, int paramId, byte[] buf, long len, int exeOrWait);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "SetDeviceParamBuffer")]
        public static extern int SetDeviceParamBuffer(int deviceSelection, int paramId, IntPtr buf, long len, int exeOrWait);

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
            //SaveRemotePCHostNameToXML(modality);
            MVMManager.Instance.LoadSettings();
        }


        public static void SaveRemotePCHostNameToXML(string modality)
        {
            string remotePCHostNameList = String.Join("/", System.Environment.MachineName);
            string remotePCIPAddressList = String.Join("/", ResourceManagerCS.GetLocalIP());

            XmlDocument doc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
            
            if (null == doc)
            {
                return;
            }
            var root = doc.DocumentElement;//Get to the root node
            XmlElement node = (XmlElement)doc.SelectSingleNode("ApplicationSettings/IPCRemoteHostPCName");
            if (node == null)
            {
                XmlElement elementRoot = doc.CreateElement(string.Empty, "IPCRemoteHostPCName", string.Empty);
                XmlElement rootNode = (XmlElement)doc.SelectSingleNode("ApplicationSettings");
                rootNode.AppendChild(elementRoot);
                node = (XmlElement)doc.SelectSingleNode("ApplicationSettings/IPCRemoteHostPCName");
            }
            //node.SetAttribute("name", remotePCHostNameList.ToString());
            //node.SetAttribute("IP", remotePCIPAddressList.ToString());
            //node.SetAttribute("IDMode", "0");
            //node.SetAttribute("activeIndex", "0");
            //node.SetAttribute("remoteAppName", "ThorSync");
            //doc.Save(GetMyDocumentsThorImageFolderString() + "Modalities\\" + modality + "\\Application Settings\\ApplicationSettings.xml");
        }

        public static string GetLocalIP()
        {
            if (true == System.Net.NetworkInformation.NetworkInterface.GetIsNetworkAvailable())
            {
                var host = System.Net.Dns.GetHostEntry(System.Net.Dns.GetHostName());
                foreach (var ip in host.AddressList)
                {
                    if (ip.AddressFamily == System.Net.Sockets.AddressFamily.InterNetwork)
                    {
                        return ip.ToString();
                    }
                }
            }
            return "";
        }


        /// <summary>
        /// Set modality with persisting last active xml
        /// </summary>
        /// <param name="modality"></param>
        public static void SetModalityPersistLast(string modality)
        {
            if ((int) MVMManager.Instance["CaptureSetupViewModel", "CaptureSetupModalitySwap"] == 1)
            {
                MVMManager.Instance["CaptureSetupViewModel", "ModalitySpinnerWindowShowing"] = true;
            }
            string str = string.Empty;
            string strActiveExp = GetCaptureTemplatePathString() + "Active.xml";

            if (!Directory.Exists(ResourceManagerCS.GetMyDocumentsThorImageFolderString() + "Modalities\\" + modality))
            {
                MessageBox.Show("Cannot find modality folder: " + modality + "\nCould not switch Modalities");
                return;
            }

            /// [persist last modality]

            //persist current modality as last if no history available
            _lastSavedModality = (0 >= _lastSavedModality.Length) ? modality : _lastSavedModality;

            //update settings from all mvm
            MVMManager.Instance.UpdateMVMXMLSettings(ref MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS]);
            XmlNodeList ndList = (null != MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS]) ? MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS].SelectNodes("/ThorImageExperiment/Modality") : null;

            if (null != ndList)
            {
                //update modality name
                XmlManager.SetAttribute(ndList[0], MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS], "name", _lastSavedModality);
                MVMManager.Instance.SaveSettings(SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS, true);

                //Disable lasers when swapping modalities in case TTL mode is enabled and the laser source is disconnected
                //Special case for when modality is swapped from Capture Setup. Swapping modalities from Hardware Connetions handled in MenuModuleLS
                if ((int)MVMManager.Instance["MultiLaserControlViewModel", "LaserAllTTL"] == 1 && (int)MVMManager.Instance["MultiLaserControlViewModel", "CaptureSetupModalitySwap"] == 1)
                {
                    MVMManager.Instance["MultiLaserControlViewModel", "TTLModeSaveSettings"] = 0;
                    MVMManager.Instance["MultiLaserControlViewModel", "Laser1Enable"] = 0;
                    MVMManager.Instance["MultiLaserControlViewModel", "Laser2Enable"] = 0;
                    MVMManager.Instance["MultiLaserControlViewModel", "Laser3Enable"] = 0;
                    MVMManager.Instance["MultiLaserControlViewModel", "Laser4Enable"] = 0;
                }

                //Prevents laser power levels from being set to 0 when changing from modality with Analog mode enabled to one without it
                MVMManager.Instance["MultiLaserControlViewModel", "LaserAnalogModalitySwap"] = 1;

                //back up exp settings to the modality folder:
                if (Directory.Exists(GetMyDocumentsThorImageFolderString() + "Modalities\\" + _lastSavedModality))
                {
                    string mod = GetMyDocumentsThorImageFolderString() + "Modalities\\" + _lastSavedModality;
                    FileCopyWithExistCheck(GetActiveSettingsFileString(), mod + "\\Active.xml", true);

                    ResourceManagerCS.SafeCreateDirectory(mod + "\\AlignData");

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
                        XmlNode newNl = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS].ImportNode(nl, true);
                        MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS].DocumentElement.AppendChild(newNl);
                        MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS].Save(strActiveExp);
                    }
                }
                else
                {
                    //copy if modality is mismatched with current
                    if (XmlManager.GetAttribute(ndList[0], MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS], "name", ref str) && !modality.Equals(str))
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

        public static void SetStartupFlag(string value)
        {
            SetStartupFlag(new StringBuilder(value));
        }

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "SetupCamera")]
        public static extern int SetupCamera(int cameraSelection);

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
            string sn2 = string.Empty;
            string[] serialNums;
            DisconnectSwitchBoxes();

            serialNums = _pmtSwitchController.GetSerialNumbers();
            try
            {
                if (File.Exists("ThorDetectorSwitchSettings.xml"))
                {
                    XmlDocument detetorSwitchSettings = new XmlDocument();
                    detetorSwitchSettings.Load("ThorDetectorSwitchSettings.xml");
                    XmlNodeList ndList = (null != detetorSwitchSettings) ? detetorSwitchSettings.SelectNodes("/DetectorSwitchSettings/SerialNumber") : null;
                    if (null != ndList)
                    {
                        XmlManager.GetAttribute(ndList[0], detetorSwitchSettings, "sn", ref str);
                    }
                    if (string.Empty == str)
                    {
                        XmlManager.SetAttribute(ndList[0], detetorSwitchSettings, "sn", str = serialNums[0]);
                    }
                    ret = _pmtSwitchController.Connect(str) ? 1 : 0;
                    if (1 < serialNums.Length)
                    {
                        ndList = (null != detetorSwitchSettings) ? detetorSwitchSettings.SelectNodes("/DetectorSwitchSettings/SecondSerialNumber") : null;
                        if (null != ndList)
                        {
                            XmlManager.GetAttribute(ndList[0], detetorSwitchSettings, "sn", ref sn2);
                        }
                        if (string.Empty == sn2)
                        {
                            sn2 = (str == serialNums[0]) ? serialNums[1] : serialNums[0];
                            XmlManager.SetAttribute(ndList[0], detetorSwitchSettings, "sn", sn2);
                        }
                        _pmtSwitchController2.Connect(sn2);
                    }
                    detetorSwitchSettings.Save("ThorDetectorSwitchSettings.xml");
                }
                else
                {
                    ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "Error ThorDetectorSwitchSettings.xml not found");
                }
            }
            catch (Exception e)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "Error reading ThorDetectorSwitchSettings.xml in ResourceManager -> ConnectToPMTSwitchBox. Exception: " + e.ToString());
            }
            return ret;
        }

        public void DisconnectSwitchBoxes()
        {
            _pmtSwitchController.Disconnect();
            _pmtSwitchController2.Disconnect();
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

        public bool IsSwitchBoxConnected(int selectedDetectorSwitch)
        {
            if (2 == selectedDetectorSwitch)
            {
                return _pmtSwitchController2.CheckSwitchBoxConnection();
            }
            return _pmtSwitchController.CheckSwitchBoxConnection();
        }

        public int ReadSwitchBoxPositions(int selectedDetectorSwitch)
        {
            if (2 == selectedDetectorSwitch)
            {
                return _pmtSwitchController2.GetSwitchesStatus();
            }
            return _pmtSwitchController.GetSwitchesStatus();
        }

        public void ReplaceActiveXML(string experimentPath)
        {
            string expTemplatesFldr = Path.GetDirectoryName(experimentPath);
            string templateName = Path.GetFileNameWithoutExtension(experimentPath);
            string expRoisXAML = expTemplatesFldr + "\\" + templateName + "\\ROIs.xaml";
            string expRoisMask = expTemplatesFldr + "\\" + templateName + "\\ROIMask.raw";
            string expBleachingROIsXAML = expTemplatesFldr + "\\" + templateName + "\\BleachROIs.xaml";
            string expBleachingWaveFormH5 = expTemplatesFldr + "\\" + templateName + "\\BleachWaveform.raw";
            string expSLMWaveforms = expTemplatesFldr + "\\" + templateName + "\\SLMWaveforms";

            string tempFolder = Application.Current.Resources["TemplatesFolder"].ToString();
            string pathActiveXML = tempFolder + "\\Active.xml";
            string pathActiveROIsXAML = tempFolder + "\\ActiveROIs.xaml";
            string pathActiveROIMask = tempFolder + "\\ActiveMask.xaml";
            string pathActiveBleachingROIsXAML = tempFolder + "\\BleachROIs.xaml";
            string pathActiveBleachingWaveformH5 = tempFolder + "\\BleachWaveform.raw";
            string pathActiveSLMWaveforms = tempFolder + "\\SLMWaveforms";

            if (File.Exists(experimentPath))
            {
                XmlDocument doc = new XmlDocument();
                string str = string.Empty;
                doc.Load(experimentPath);
                const string MODALITY = "ThorImageExperiment/Modality";

                XmlNode node = doc.SelectSingleNode(MODALITY);
                if (null != node)
                {
                    if (XmlManager.GetAttribute(node, doc, "name", ref str) && (!str.Equals(ResourceManagerCS.Instance.ActiveModality)))
                    {
                        //if the modality differs from the modality in the script,
                        //attempt to set the modality without persistance
                        ResourceManagerCS.SetActiveModality = str;
                    }
                }
                File.Copy(experimentPath, pathActiveXML, true);
            }
            else
            {
                MessageBox.Show("Capture: Could not find file " + experimentPath);
                return;
            }
            if (File.Exists(expRoisXAML))
            {
                File.Copy(expRoisXAML, pathActiveROIsXAML, true);
            }
            if (File.Exists(expRoisMask))
            {
                File.Copy(expRoisMask, pathActiveROIMask, true);
            }
            switch (GetBleachMode)
            {
                case BleachMode.BLEACH:
                    if (File.Exists(expBleachingROIsXAML))
                    {
                        File.Copy(expBleachingROIsXAML, pathActiveBleachingROIsXAML, true);
                    }
                    else
                    {
                        if (File.Exists(pathActiveBleachingROIsXAML))
                        {
                            File.Delete(pathActiveBleachingROIsXAML);
                        }
                    }
                    if (File.Exists(expBleachingWaveFormH5))
                    {
                        File.Copy(expBleachingWaveFormH5, pathActiveBleachingWaveformH5, true);
                    }
                    else
                    {
                        if (File.Exists(pathActiveBleachingWaveformH5))
                        {
                            File.Delete(pathActiveBleachingWaveformH5);
                        }
                    }
                    break;
                case BleachMode.SLM:
                    if (Directory.Exists(expSLMWaveforms))
                    {
                        if (Directory.Exists(pathActiveSLMWaveforms))
                        {
                            System.IO.DirectoryInfo dInfo = new DirectoryInfo(pathActiveSLMWaveforms);

                            foreach (FileInfo file in dInfo.GetFiles())
                            {
                                file.Delete();
                            }
                            foreach (DirectoryInfo folder in dInfo.GetDirectories())
                            {
                                folder.Delete(true);
                            }
                        }
                        //recreate all directories:
                        foreach (string dirPath in Directory.GetDirectories(expSLMWaveforms, "*", SearchOption.AllDirectories))
                        {
                            ResourceManagerCS.SafeCreateDirectory(dirPath.Replace(expSLMWaveforms, pathActiveSLMWaveforms));
                        }
                        //copy all files:
                        foreach (string newPath in Directory.GetFiles(expSLMWaveforms, "*.*", SearchOption.AllDirectories))
                        {
                            File.Copy(newPath, newPath.Replace(expSLMWaveforms, pathActiveSLMWaveforms), true);
                        }
                    }
                    break;
                default:
                    break;
            }
        }

        public bool ToggleSwitch(int selectedDetectorSwitch, string switchName, int value)
        {
            if (2 == selectedDetectorSwitch)
            {
                return _pmtSwitchController2.Set_Switch(switchName, value);
            }
            return _pmtSwitchController.Set_Switch(switchName, value);
        }

        public bool ToggleSwitchBox(int selectedDetectorSwitch, byte toggle)
        {
            if (2 == selectedDetectorSwitch)
            {
                return _pmtSwitchController2.Set_SwitchesPort(toggle);
            }
            return _pmtSwitchController.Set_SwitchesPort(toggle);
        }

        public void UpdatePMTSwitchBox()
        {
            byte toggle = 0; //Toggle all off by default 0000

            //only switch when changing to an LSM camera and when Capture Setup has loaded
            if ((int)ICamera.CameraType.LSM == GetCameraType() && AllowSwitchBoxToWork)
            {
                int lsmType = GetLSMType();

                //switch depending on LSM camera type
                switch ((ICamera.LSMType)lsmType)
                {
                    case ICamera.LSMType.GALVO_RESONANCE:
                        if (null == MVMManager.Instance.MVMCollection || false == (bool)MVMManager.Instance["MiniCircuitsSwitchControlViewModel", "ManualSwitchEnable", (object)false])
                        {
                            if (IsSwitchBoxConnected(1) && toggle != ReadSwitchBoxPositions(1))
                            {
                                if (false == ToggleSwitchBox(1, toggle))
                                {
                                    ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "Mini Circuits Switch: Error could not toggle Switch to configuration " + toggle.ToString());
                                }
                            }
                            if (IsSwitchBoxConnected(2) && toggle != ReadSwitchBoxPositions(2))
                            {
                                if (false == ToggleSwitchBox(2, toggle))
                                {
                                    ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "Mini Circuits Second Switch box: Error could not toggle Switch to configuration " + toggle.ToString());
                                }
                            }
                        }
                        SetDeviceParamInt((int)SelectedHardware.SELECTED_PMTSWITCH, (int)IDevice.Params.PARAM_PMT_SWITCH_POS, 0, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT);
                        break;
                    case ICamera.LSMType.GALVO_GALVO:
                        if (null == MVMManager.Instance.MVMCollection || false == (bool)MVMManager.Instance["MiniCircuitsSwitchControlViewModel", "ManualSwitchEnable", (object)false])
                        {
                            toggle = 15; //Toggle all on 1111
                            if (IsSwitchBoxConnected(1) && toggle != ReadSwitchBoxPositions(1))
                            {
                                if (false == ToggleSwitchBox(1, toggle))
                                {
                                    ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "Mini Circuits Switch: Error could not toggle Switch to configuration " + toggle.ToString());
                                }
                            }
                            if (IsSwitchBoxConnected(2) && toggle != ReadSwitchBoxPositions(2))
                            {
                                if (false == ToggleSwitchBox(2, toggle))
                                {
                                    ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "Mini Circuits Second Switch box: Error could not toggle Switch to configuration " + toggle.ToString());
                                }
                            }
                        }
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

        [DllImport(".\\ResourceManager.dll", EntryPoint = "GetStartupFlag", CharSet = CharSet.Unicode)]
        private static extern int GetStartupFlag(StringBuilder value, int length);

        [DllImport(".\\ResourceManager.dll", EntryPoint = "ReturnDocMutex")]
        private static extern bool ReturnDocMutex(SettingsFileType sfType);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "SetCameraParamString", CharSet = CharSet.Unicode)]
        private static extern int SetCameraParamString(int cameraSelection, int param, StringBuilder value);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "SetDeviceParamString", CharSet = CharSet.Unicode)]
        private static extern int SetDeviceParamString(int deviceSelection, int paramId, StringBuilder value, int exeOrWait);

        [DllImport(".\\ResourceManager.dll", EntryPoint = "SetModality", CharSet = CharSet.Unicode)]
        private static extern int SetModality(StringBuilder sb);

        [DllImport(".\\ResourceManager.dll", EntryPoint = "SetStartupFlag", CharSet = CharSet.Unicode)]
        private static extern int SetStartupFlag(StringBuilder value);

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