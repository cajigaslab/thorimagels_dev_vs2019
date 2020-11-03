namespace CaptureSetupDll.Model
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Timers;
    using System.Windows;
    using System.Windows.Data;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Threading;
    using System.Xml;

    using DatabaseInterface;

    using OverlayManager;

    using ThorLogging;

    using ThorSharedTypes;

    public partial class CaptureSetup : INotifyPropertyChanged
    {
        #region Fields

        private static double _framesPerSecond = 0;

        //  private static CaptureSetupCustomParams _csParams = new CaptureSetupCustomParams();
        private bool _liveStartButtonStatus;

        #endregion Fields

        #region Properties

        public CaptureSetupDll.ViewModel.CaptureSetupViewModel CaptureSetupViewModel
        {
            get;
            set;
        }

        public bool CaptureStatus
        {
            get
            {
                return GetActiveCapture();
            }
        }

        public double FramesPerSecond
        {
            get
            {
                GetFrameRate(ref _framesPerSecond);

                return _framesPerSecond;
            }
        }

        public bool LiveStartButtonStatus
        {
            get
            {
                return _liveStartButtonStatus;
            }
            set
            {
                _liveStartButtonStatus = value;
                UpdateMVMControlsStatus(value);

                if (1 == (int)MVMManager.Instance["AreaControlViewModel", "MesoMicroVisible", (object)0])
                    MVMManager.Instance["MesoScanViewModel", "IsLivingMode"] = (!value) ? (int)1 : (int)0;
            }
        }

        /// <summary>
        /// Specifies if snapshots taken are saved as a single image experiment 
        /// </summary>
        public bool SaveSnapshot
        {
            get
            {

                if (Application.Current == null)
                {
                    return false;
                }
                ApplicationDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
                XmlNode node = ApplicationDoc.SelectSingleNode("/ApplicationSettings/DisplayOptions/CaptureSetup");
                string value = String.Empty;
                if (XmlManager.GetAttribute(node, ApplicationDoc, "saveSnapshots", ref value))
                {
                    return value == "1";
                }

                return false;
            }
            set
            {
                if (Application.Current == null)
                {
                    return;
                }
                ApplicationDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
                XmlNode node = ApplicationDoc.SelectSingleNode("/ApplicationSettings/DisplayOptions/CaptureSetup");

                XmlManager.SetAttribute(node, ApplicationDoc, "saveSnapshots", ((true == value) ? 1 : 0).ToString());

                MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS);
            }
        }

        public bool SendCameraLiveStatus
        {
            get
            {
                int val = 0;
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_IS_LIVE, ref val);
                return (1 == val);
            }
            set
            {
                int val = (value) ? 1 : 0;
                ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_IS_LIVE, val);
            }
        }

        /// <summary>
        /// The base name to use for snapshot experiments
        /// </summary>
        public string SnapshotBaseName
        {
            get
            {

                string defaultName = "Snapshot";
                if (Application.Current == null)
                {
                    return defaultName;
                }

                ApplicationDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
                XmlNode node = ApplicationDoc.SelectSingleNode("/ApplicationSettings/DisplayOptions/CaptureSetup");
                string value = String.Empty;
                if (XmlManager.GetAttribute(node, ApplicationDoc, "snapshotBaseName", ref value))
                {
                    if (!String.IsNullOrEmpty(value))
                    {
                        return value;
                    }
                }

                return defaultName;
            }
            set
            {
                if (Application.Current == null)
                {
                }
                ApplicationDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
                XmlNode node = ApplicationDoc.SelectSingleNode("/ApplicationSettings/DisplayOptions/CaptureSetup");

                XmlManager.SetAttribute(node, ApplicationDoc, "snapshotBaseName", value);

                MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS);
            }
        }

        public bool SnapshotIncludeExperimentInfo
        {
            get
            {
                return GetSnapshotIncludeExperimentInfo();
            }
            set
            {
                SetSnapshotIncludeExperimentInfo((true == value) ? 1 : 0);
            }
        }

        public string SnapshotSavingPath
        {
            get
            {
                return GetSnapshotSavingPath();
            }
            set
            {
                SetSnapshotSavingPath(value);
            }
        }

        #endregion Properties

        #region Methods

        public bool AutoExposure(double exposure)
        {
            double exposureResult = 0;
            double multiplier = 0;
            if (false == AutoExposure(exposure, ref exposureResult, ref multiplier))
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, this.GetType().Name + " AutoExposure failed");
                return false;
            }

            SetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_EXPOSURE_TIME_MS, exposureResult);

            return true;
        }

        public bool AutoFocus(double magnification)
        {
            if (false == StartAutoFocus(magnification))
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, this.GetType().Name + " StartAutoFocus failed");
                return false;
            }

            return true;
        }

        /// <summary>
        /// Creates an experiment folder at the specified path, including ROI and XML files. XML files are as is, and not
        /// configured in this method
        /// </summary>
        /// <param name="experimentPath"> Output path of the experiment folder created </param>
        /// <returns> File name of the experiment folder created </returns>
        public FileName CreateSnapshotExperimentFolder(out string experimentPath)
        {
            try
            {
                //Get Current Experiment Files
                string experimentsSavePath = GetSnapshotSavingPath();

                //Create Unique Experiment Name
                FileName expName = new FileName(SnapshotBaseName);

                if (string.IsNullOrEmpty(expName.NameNumber))
                {
                    expName.Increment();
                }

                expName.MakeUnique(experimentsSavePath);
                experimentPath = experimentsSavePath + "\\" + expName.FullName;

                //Create the new experiment directory(s)
                Directory.CreateDirectory(experimentPath);

                //assign the experiment xml path
                string experimentXMLPath = experimentPath + "\\Experiment.xml";
                string activeExperimentXMLPath = ExpPath; //In Template Folder

                //overwrite the active experiment settings
                File.Copy(activeExperimentXMLPath, experimentXMLPath);

                //update experiment file
                UpdateSnapshotExperimentFile(experimentXMLPath, expName.FullName);

                //update experiment list
                try
                {
                    //Update the variables list LAST OUTPUT value
                    const string PATH_LAST_OUTPUT = "LAST OUTPUT";
                    string strVar = Application.Current.Resources["ApplicationSettingsFolder"].ToString() + "\\VariableList.xml";
                    XmlDocument varDoc = new XmlDocument();
                    varDoc.Load(strVar);
                    XmlNodeList ndList = varDoc.SelectNodes("/VariableList/Path");

                    foreach (XmlNode nd in ndList)
                    {
                        string str = nd.Attributes["name"].Value;
                        if (str.Equals(PATH_LAST_OUTPUT))
                        {
                            nd.Attributes["value"].Value = experimentPath.ToString();
                            break;
                        }
                    }
                    varDoc.Save(strVar);
                }
                catch (Exception ex)
                {
                    string str = ex.Message;
                    //error loading/saving the variables list
                    ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Unable to load/save variables list");
                }

                try
                {
                    //Update the variables list LAST OUTPUT value
                    const string PATH_LAST_OUTPUT = "LAST OUTPUT";
                    string strVar = Application.Current.Resources["ApplicationSettingsFolder"].ToString() + "\\VariableList.xml";
                    XmlDocument varDoc = new XmlDocument();
                    varDoc.Load(strVar);
                    XmlNodeList ndList = varDoc.SelectNodes("/VariableList/Path");

                    foreach (XmlNode nd in ndList)
                    {
                        string str = nd.Attributes["name"].Value;
                        if (str.Equals(PATH_LAST_OUTPUT))
                        {
                            nd.Attributes["value"].Value = experimentPath.ToString();
                            break;
                        }
                    }
                    varDoc.Save(strVar);
                }
                catch (Exception ex)
                {
                    string str = ex.Message;
                    //error loading/saving the variables list
                    ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Unable to load/save variables list");
                }

                OverlayManagerClass.Instance.SaveROIs(experimentPath + "\\ROIs.xaml");
                OverlayManagerClass.Instance.SaveMaskToPath(experimentPath + "\\ROIMask.raw");

                return expName;

            }
            catch (Exception ex)
            {
                ex.ToString();
                experimentPath = null;
                return null;
            }
        }

        /// <summary>
        /// Gets an int bitmask representing the currently enabled channels
        /// </summary>
        /// <returns> Int bitmask of currently enabled channels </returns>
        public int GetChannelEnabledBitmask()
        {
            int bitmask = 0;

            if (LSMChannelEnable0)
            {
                bitmask += 1;
            }
            if (LSMChannelEnable1)
            {
                bitmask += 2;
            }
            if (LSMChannelEnable2)
            {
                bitmask += 4;
            }
            if (LSMChannelEnable3)
            {
                bitmask += 8;
            }

            return bitmask;
        }

        public String GetUniqueSnapshotFilename()
        {
            string snapshotFilename;

            FileName expName = new FileName(SnapshotBaseName + ".tif");

            if (string.IsNullOrEmpty(expName.NameNumber))
            {
                expName.Increment();
            }
            expName.MakeUnique(SnapshotSavingPath);

            snapshotFilename = SnapshotSavingPath + "\\" + expName.FullName;

            if (false == Directory.Exists(SnapshotSavingPath))
            {
                Directory.CreateDirectory(SnapshotSavingPath);
            }

            return snapshotFilename;
        }

        /// <summary>
        /// Takes a live snapshot and save the result as an experiment
        /// </summary>
        /// <returns> Boolean if the snapshot was able to be successfully taken and saved </returns>
        public bool LiveSnapshotAndSave()
        {
            string path = "";
            if (SnapshotIncludeExperimentInfo)
            {
                FileName expName = CreateSnapshotExperimentFolder(out path);

                if (path != null)
                {
                    LiveSnapshot(path, GetChannelEnabledBitmask(), 0);
                    SaveLastExperimentInfo(path + "\\", expName.FullName);
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                if (0xF > GetChannelEnabledBitmask())
                {
                    //still take the snapshot if the experiment info is not
                    //being saved. There will be the possibility of saving
                    //without the experiment info later.
                    LiveSnapshot();
                }
                else
                {
                    //save multipage tiff for more than 3 channels:
                    string saveStr = GetUniqueSnapshotFilename();
                    LiveSnapshot(saveStr, GetChannelEnabledBitmask(), 1);
                }
                return true;
            }
        }

        /// <summary>
        /// Takes a single snapshot, and if enabled saves as an experiment
        /// </summary>
        public void Snapshot()
        {
            //ensure the buffer is copied after the capture
            _pixelDataReady = false;

            MVMManager.Instance["ScanControlViewModel", "EnablePMTGains"] = false;
            MVMManager.Instance["ScanControlViewModel", "EnablePMTGains"] = true;
            MVMManager.Instance["ScanControlViewModel", "LsmClkPnlEnabled"] = false;

            if (1 == (int)MVMManager.Instance["AreaControlViewModel", "MesoMicroVisible", (object)0])
                MVMManager.Instance["MesoScanViewModel", "SettingsPath"] = ResourceManagerCS.GetActiveSettingsFileString();

            if (SaveSnapshot)
            {
                LiveSnapshotAndSave();
            }
            else
            {
                LiveSnapshot();
            }
        }

        /// <summary>
        /// Tasks to be done after snapshot is finished.
        /// </summary>
        public void SnapshotIsDone()
        {
            MVMManager.Instance["ScanControlViewModel", "EnablePMTGains"] = false;
            MVMManager.Instance["ScanControlViewModel", "LsmClkPnlEnabled"] = true;
        }

        public void Start()
        {
            try
            {
                LiveStartButtonStatus = false;

                // event trigerred to the view model to change the status of the menu bar buttons
                UpdateMenuBarButton(false);

                //ensure the buffer is copied after the capture
                _pixelDataReady = false;

                MVMManager.Instance["ScanControlViewModel", "EnablePMTGains"] = false;
                MVMManager.Instance["ScanControlViewModel", "EnablePMTGains"] = true;

                StartLiveCapture();
            }
            catch (System.DllNotFoundException)
            {
                //CaptureSetupDll is missing
            }
        }

        public void Stop()
        {
            try
            {
                LiveStartButtonStatus = true;
                // event trigerred to the view model to change the status of the menu bar buttons
                UpdateMenuBarButton(true);
                MVMManager.Instance["ScanControlViewModel", "EnablePMTGains"] = false;
                StopLiveCapture();
                _prevTickCount = 0;
                _framesPerSecond = 0;
            }
            catch (System.DllNotFoundException)
            {
                //CaptureSetupDll is missing
            }
        }

        /// <summary>
        /// Disable some controls' availability while imaging
        /// </summary>
        /// <param name="status"></param>
        public void UpdateMVMControlsStatus(bool enable)
        {
            MVMManager.Instance["AreaControlViewModel", "ImageStartStatusArea"] = enable;
            MVMManager.Instance["CameraControlViewModel", "ImageStartStatusCamera"] = enable;
            MVMManager.Instance["ScanControlViewModel", "LsmClkPnlEnabled"] = enable;
            MVMManager.Instance["ThreePhotonControlViewModel", "Disable3PCheckbox"] = enable;
            MVMManager.Instance["DFLIMControlViewModel", "DFLIMEnableControlsThatNeedAcquistionOff"] = enable;
            ((ThorSharedTypes.IMVM)MVMManager.Instance["ZControlViewModel", this]).OnPropertyChange("PreviewButtonEnabled");
        }

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "AutoExposure")]
        private static extern bool AutoExposure(double exposure, ref double exposureResult, ref double multiplier);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "SetupCommand")]
        private static extern int CaptureSetupSetupCommand();

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "GetActiveCapture")]
        private static extern bool GetActiveCapture();

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "GetFrameRate")]
        private static extern bool GetFrameRate(ref double rate);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "LiveSnapshot")]
        private static extern bool LiveSnapshot();

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "SnapshotAndSave", CharSet = CharSet.Unicode)]
        private static extern bool LiveSnapshot(string destinationPathAndName, int enabledChannelMask, int saveMultiPage);

        private static void SetChannelFromEnable()
        {
            //update the channel value also
            int chan = (Convert.ToInt32(_lsmChannelEnable[0]) | (Convert.ToInt32(_lsmChannelEnable[1]) << 1) | (Convert.ToInt32(_lsmChannelEnable[2]) << 2) | (Convert.ToInt32(_lsmChannelEnable[3]) << 3));

            int val = 1;
            switch (chan)
            {
                case 1: val = 1; break;
                case 2: val = 2; break;
                case 4: val = 4; break;
                case 8: val = 8; break;
                default:
                    {
                        val = 0xf;
                    }
                    break;
            }

            SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_CHANNEL, val);
            SetDisplayChannels(chan);
        }

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "SetDisplayChannels")]
        private static extern bool SetDisplayChannels(int chan);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "StartAutoFocus")]
        private static extern bool StartAutoFocus(double magnification);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "StartLiveCapture")]
        private static extern bool StartLiveCapture();

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "StopLiveCapture")]
        private static extern bool StopLiveCapture();

        /// <summary>
        /// Gets the specified attribute from the given node and doc
        /// </summary>
        /// <param name="node"> The node to look in </param>
        /// <param name="doc"> The document to look in </param>
        /// <param name="attrName"> The name of the attribute </param>
        /// <param name="attrValue"> The string the attribute is currently set to </param>
        /// <returns> If the attribute was read successfully </returns>
        private bool GetAttribute(XmlNode node, XmlDocument doc, string attrName, ref string attrValue)
        {
            bool ret;

            if (null == node)
            {
                return false;
            }

            if (null == node.Attributes.GetNamedItem(attrName))
            {
                ret = false;
            }
            else
            {
                attrValue = node.Attributes[attrName].Value;
                ret = true;
            }

            return ret;
        }

        private bool GetSnapshotIncludeExperimentInfo()
        {
            string appSettingsFile = string.Empty;

            if (Application.Current == null)
            {
                return false;
            }

            ApplicationDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

            XmlNode node = ApplicationDoc.SelectSingleNode("/ApplicationSettings/DisplayOptions/CaptureSetup");
            if (node == null)
            {
                return false;
            }
            else
            {
                if (node.Attributes.GetNamedItem("saveExperimentInfo") != null)
                {
                    int val = Convert.ToInt32(node.Attributes["saveExperimentInfo"].Value);
                    return (0 == val) ? false : true;
                }
                else
                {
                    return false;
                }
            }
        }

        /// <summary>
        /// Returns the path in which experiments are saved to
        /// </summary>
        /// <returns> The path in which experiments are saved to </returns>
        private string GetSnapshotSavingPath()
        {
            if (Application.Current == null)
            {
                return string.Empty;
            }

            ApplicationDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

            XmlNode node = ApplicationDoc.SelectSingleNode("/ApplicationSettings/DisplayOptions/CaptureSetup");
            if (node == null)
            {
                return Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments).ToString();
            }
            else
            {
                if (node.Attributes.GetNamedItem("savePath") != null)
                {
                    string path = node.Attributes["savePath"].Value;
                    if (string.Empty == path)
                    {
                        path = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments).ToString();
                    }
                    return path;
                }
                else
                {
                    return Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments).ToString();
                }
            }
        }

        /// <summary>
        /// Updates the file storing the last experiment run by the system to the specified experiment path and name
        /// </summary>
        /// <param name="path"> Path to the experiment folder, including trailing file separator </param>
        /// <param name="expName"> The name of the experiment </param>
        private void SaveLastExperimentInfo(string path, string expName)
        {
            string appSettingsFolder = Application.Current.Resources["ApplicationSettingsFolder"].ToString();
            if (Directory.Exists(appSettingsFolder))
            {
                DataStore.Instance.ConnectionString = string.Format("URI=file:{0}\\{1}", appSettingsFolder, "thorDatabase.db");
                DataStore.Instance.Open();
            }
            string batchName = DataStore.Instance.GetBatchName();
            DataStore.Instance.AddExperiment(expName, path, batchName);
            ApplicationDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
            XmlNode node = ApplicationDoc.SelectSingleNode("/ApplicationSettings/LastExperiment");
            if (node == null)
            {
                node = ApplicationDoc.CreateNode(XmlNodeType.Element, "LastExperiment", null);
                ApplicationDoc.DocumentElement.AppendChild(node);
            }

            XmlManager.SetAttribute(node, ApplicationDoc, "path", path);
            MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS);
        }

        /// <summary>
        /// Sets an attribute, creating it if it does not exist
        /// </summary>
        /// <param name="node"> The node containing the attribute </param>
        /// <param name="doc"> The document containing the node </param>
        /// <param name="attrName"> The name of the attribute </param>
        /// <param name="attrValue"> The value to set it to </param>
        private void SetAttribute(XmlNode node, XmlDocument doc, string attrName, string attrValue)
        {
            if (null == node.Attributes.GetNamedItem(attrName))
            {
                XmlAttribute attr = doc.CreateAttribute(attrName);
                attr.Value = attrValue;
                node.Attributes.Append(attr);
            }

            node.Attributes[attrName].Value = attrValue;
        }

        private void SetSnapshotIncludeExperimentInfo(int enable)
        {
            if (Application.Current == null)
            {
                return;
            }
            ApplicationDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
            XmlNode node = ApplicationDoc.SelectSingleNode("/ApplicationSettings/DisplayOptions/CaptureSetup");
            if (node == null)
            {
                return;
            }
            else
            {
                XmlManager.SetAttribute(node, ApplicationDoc, "saveExperimentInfo", enable.ToString());

                MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS);
            }
        }

        private void SetSnapshotSavingPath(string str)
        {
            if (Application.Current == null)
            {
                return;
            }
            ApplicationDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
            XmlNode node = ApplicationDoc.SelectSingleNode("/ApplicationSettings/DisplayOptions/CaptureSetup");

            if (node == null)
            {
                return;
            }
            else
            {
                XmlManager.SetAttribute(node, ApplicationDoc, "savePath", str);

                MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS);
            }
        }

        /// <summary>
        /// Update the experiment file at the given path for a snapshot capture
        /// </summary>
        /// <param name="expXML"> The path to the experiment </param>
        /// <param name="experimentName"> The name of the experiment </param>
        /// <returns> true if successfully updated </returns>
        private bool UpdateSnapshotExperimentFile(string expXML, string experimentName)
        {
            XmlDocument expDoc = new XmlDocument();

            //When scripting the active.xml might be locked up when trying to load it here
            //Using a try-catch inside of a while loop allows to wait for 10ms and try
            //again for up to 3 times.
            bool loadSucceeded = false;
            int tries = 0;
            while (false == loadSucceeded)
            {
                try
                {
                    expDoc.Load(expXML);
                    loadSucceeded = true;
                }
                catch
                {
                    if (3 <= tries)
                    {
                        MessageBox.Show("SCRIPT ERROR");
                        return false;
                    }
                    tries++;
                    System.Threading.Thread.Sleep(10);
                }
            }

            //Save most parameters
            MVMManager.Instance.UpdateMVMXMLSettings(ref expDoc);

            //used to load color images.
            XmlNodeList nodeList;

            //Name
            nodeList = expDoc.SelectNodes("/ThorImageExperiment/Name");
            if (nodeList.Count > 0)
            {
                XmlManager.SetAttribute(nodeList[0], expDoc, "name", experimentName);
            }

            //Capture
            nodeList = expDoc.SelectNodes("/ThorImageExperiment/CaptureMode");
            if (nodeList.Count > 0)
            {
                XmlManager.SetAttribute(nodeList[0], expDoc, "mode", "0");
            }

            //Capture
            nodeList = expDoc.SelectNodes("/ThorImageExperiment/Timelapse");
            if (nodeList.Count > 0)
            {
                XmlManager.SetAttribute(nodeList[0], expDoc, "timepoints", "1");
                XmlManager.SetAttribute(nodeList[0], expDoc, "intervalSec", "0");
                XmlManager.SetAttribute(nodeList[0], expDoc, "triggerMode", "0");
            }
            nodeList = expDoc.SelectNodes("/ThorImageExperiment/ZStage");

            if (nodeList.Count > 0)
            {
                XmlManager.SetAttribute(nodeList[0], expDoc, "steps", "1");
                XmlManager.SetAttribute(nodeList[0], expDoc, "startPos", ((double)MVMManager.Instance["ZControlViewModel", "ZPosition", 0.0]).ToString());
                XmlManager.SetAttribute(nodeList[0], expDoc, "stopPos", ((double)MVMManager.Instance["ZControlViewModel", "ZPosition", 0.0]).ToString());
            }

            //Capture
            nodeList = expDoc.SelectNodes("/ThorImageExperiment/Streaming");
            if (nodeList.Count > 0)
            {
                XmlManager.SetAttribute(nodeList[0], expDoc, "enable", "0");
            }

            //Experiment Status
            nodeList = expDoc.SelectNodes("/ThorImageExperiment/ExperimentStatus");
            if (nodeList.Count > 0)
            {
                XmlManager.SetAttribute(nodeList[0], expDoc, "value", "Complete");
            }

            //Set the date for the file
            string formatDate = "MM/dd/yyyy HH:mm:ss";
            string date = DateTime.Now.ToString(formatDate);

            nodeList = expDoc.SelectNodes("/ThorImageExperiment/Date");

            if (nodeList.Count > 0)
            {
                XmlManager.SetAttribute(nodeList[0], expDoc, "date", date);
                XmlManager.SetAttribute(nodeList[0], expDoc, "uTime", ((int)Math.Truncate((DateTime.UtcNow.Subtract(new DateTime(1970, 1, 1))).TotalSeconds)).ToString());
            }

            //save the updated experiment file
            expDoc.Save(expXML);

            return true;
        }

        #endregion Methods

        #region Nested Types

        [StructLayout(LayoutKind.Sequential)]
        struct DoubleParam
        {
            public double val;
            public int alias;
            public int useAlias;
        }

        [StructLayout(LayoutKind.Sequential)]
        struct IntParam
        {
            public int val;
            public int alias;
            public int useAlias;
        }

        #endregion Nested Types
    }
}