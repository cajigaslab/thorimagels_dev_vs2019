namespace CaptureSetupDll.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Xml;

    using CaptureSetupDll.Model;

    using OverlayManager;

    using ThorLogging;

    using ThorSharedTypes;

    public partial class CaptureSetupViewModel : ViewModelBase
    {
        #region Properties

        public bool PersistDataNow
        {
            set
            {
                if (value)
                {
                    MVMManager.Instance.UpdateMVMXMLSettings(ref MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS]);
                    MVMManager.Instance.SaveSettings(SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS);
                }
            }
        }

        public int PersistGlobalExperimentXMLNow
        {
            set
            {
                PersistGlobalExperimentXML((GlobalExpAttribute)value);
            }
        }

        #endregion Properties

        #region Methods

        [DllImport(".\\Modules_Native\\SelectHardware.dll", EntryPoint = "UpdateAndPersistCurrentDevices")]
        public static extern int UpdateAndPersistCurrentDevices();

        public void PersistChannels()
        {
            if (ResourceManagerCS.BorrowDocMutexCS(SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS, (int)Constants.TIMEOUT_MS))
            {
                MVMManager.Instance.ReloadSettings(SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS);
                XmlDocument xmlDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS];
                XmlNodeList ndList = xmlDoc.SelectNodes("/ThorImageExperiment/Wavelengths");

                if (ndList.Count > 0)
                {
                    ndList[0].RemoveAll();
                }

                ndList = xmlDoc.SelectNodes("/ThorImageExperiment/Wavelengths");

                if (ndList.Count > 0)
                {
                    XmlManager.SetAttribute(ndList[0], xmlDoc, "nyquistExWavelengthNM", MVMManager.Instance["AreaControlViewModel", "NyquistExWavelength", (object)string.Empty].ToString());
                    XmlManager.SetAttribute(ndList[0], xmlDoc, "nyquistEmWavelengthNM", MVMManager.Instance["AreaControlViewModel", "NyquistEmWavelength", (object)string.Empty].ToString());

                    XmlElement newElement;

                    XmlNodeList waveList = this.HardwareDoc.SelectNodes("/HardwareSettings/Wavelength");

                    switch (this.LSMChannel)
                    {
                        case 0:
                            {
                                newElement = CreateWavelengthTag(waveList[0].Attributes["name"].Value, xmlDoc);
                                ndList[0].AppendChild(newElement);

                            }
                            break;
                        case 1:
                            {
                                newElement = CreateWavelengthTag(waveList[1].Attributes["name"].Value, xmlDoc);
                                ndList[0].AppendChild(newElement);
                            }
                            break;
                        case 2:
                            {
                                newElement = CreateWavelengthTag(waveList[2].Attributes["name"].Value, xmlDoc);
                                ndList[0].AppendChild(newElement);
                            }
                            break;
                        case 3:
                            {
                                newElement = CreateWavelengthTag(waveList[3].Attributes["name"].Value, xmlDoc);
                                ndList[0].AppendChild(newElement);
                            }
                            break;
                        case 4:
                            {
                                for (int i = 0; i < waveList.Count; i++)
                                {

                                    bool isEnabled = false;

                                    switch (i)
                                    {
                                        case 0: isEnabled = LSMChannelEnable0; break;
                                        case 1: isEnabled = LSMChannelEnable1; break;
                                        case 2: isEnabled = LSMChannelEnable2; break;
                                        case 3: isEnabled = LSMChannelEnable3; break;
                                    }

                                    if (isEnabled)
                                    {
                                        newElement = CreateWavelengthTag(waveList[i].Attributes["name"].Value, xmlDoc);
                                        ndList[0].AppendChild(newElement);
                                    }
                                }
                            }
                            break;
                    }

                    //Persist channel selection as an integer which will be decoded at Master loading:
                    XmlElement newElement2 = xmlDoc.CreateElement("ChannelEnable");
                    XmlAttribute ChanAttribute = xmlDoc.CreateAttribute("Set");

                    int maxChannels = this.MaxChannels;
                    bool[] bArray = new bool[maxChannels];

                    for (int i = 0; i < maxChannels; ++i)
                    {
                        switch (i)
                        {
                            case 0: bArray[0] = LSMChannelEnable0; break;
                            case 1: bArray[1] = LSMChannelEnable1; break;
                            case 2: bArray[2] = LSMChannelEnable2; break;
                            case 3: bArray[3] = LSMChannelEnable3; break;
                        }
                    }

                    ChanAttribute.Value = ConvertBoolAry2Int(bArray, maxChannels).ToString();
                    newElement2.Attributes.Append(ChanAttribute);
                    ndList[0].AppendChild(newElement2);
                }

                MVMManager.Instance.SaveSettings(SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS);
            }
            ResourceManagerCS.ReturnDocMutexCS(SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS);
        }

        /// <summary>
        /// Saves all global experiment parameters in all the modalities, or in path otherwise
        /// </summary>
        public void PersistGlobalExperimentXML(GlobalExpAttribute attType, string path = null)
        {
            //prepare all modality experiments' path:
            List<string> xmlFiles = new List<string>();

            if (null != path)
            {
                xmlFiles.Add(path);
            }
            else
            {
                //current active to be first:
                xmlFiles.Add(ResourceManagerCS.GetActiveSettingsFileString());

                foreach (string xmlFile in Directory.GetFiles(ResourceManagerCS.GetMyDocumentsThorImageFolderString() + "Modalities", "Active.xml", SearchOption.AllDirectories))
                {
                    xmlFiles.Add(xmlFile);
                }
            }

            ResourceManagerCS.BorrowDocMutexCS(SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS);
            try
            {
                //persist to all experiments:
                for (int fid = 0; fid < xmlFiles.Count; fid++)
                {
                    if (!File.Exists(xmlFiles[fid]))
                        continue;

                    XmlNodeList ndList, ndListP;
                    double dTmp = 0.0;
                    XmlDocument experimentFile = new XmlDocument();

                    //persist current active and give back afterward in case of missing properties:
                    if (0 == xmlFiles[fid].CompareTo(ResourceManagerCS.GetActiveSettingsFileString()))
                    {
                        this.ExperimentDoc.Save(xmlFiles[fid]);
                        experimentFile = this.ExperimentDoc;
                    }
                    else
                    {
                        experimentFile.Load(xmlFiles[fid]);
                    }

                    switch (attType)
                    {
                        case GlobalExpAttribute.GALVO_CALIBTATION:
                            ndList = experimentFile.SelectNodes("/ThorImageExperiment/Photobleaching");
                            //persist galvo calibration only when valid using ScaleFine:
                            if ((ndList.Count > 0) && ((double)MVMManager.Instance["AreaControlViewModel", "LSMFieldScaleXFine", (object)0.0] > 0))
                            {
                                XmlManager.SetAttribute(ndList[0], experimentFile, "calibrationDate", MVMManager.Instance["AreaControlViewModel", "LSMLastCalibrationDate", (object)0].ToString());
                                XmlManager.SetAttribute(ndList[0], experimentFile, "calibrationDateUnix", MVMManager.Instance["AreaControlViewModel", "LSMLastCalibrationDateUnix", (object)0].ToString());
                                XmlManager.SetAttribute(ndList[0], experimentFile, "fieldSize", MVMManager.Instance["AreaControlViewModel", "LSMFieldSize", (object)0].ToString());
                                XmlManager.SetAttribute(ndList[0], experimentFile, "pixelX", MVMManager.Instance["AreaControlViewModel", "LSMPixelX", (object)0].ToString());
                                XmlManager.SetAttribute(ndList[0], experimentFile, "pixelY", MVMManager.Instance["AreaControlViewModel", "LSMPixelY", (object)0].ToString());
                                XmlManager.SetAttribute(ndList[0], experimentFile, "scaleYScan", MVMManager.Instance["AreaControlViewModel", "LSMScaleYScan", (object)0].ToString());
                                XmlManager.SetAttribute(ndList[0], experimentFile, "pixelSizeUM", MVMManager.Instance["AreaControlViewModel", "LSMUMPerPixel", (object)0].ToString());
                                XmlManager.SetAttribute(ndList[0], experimentFile, "offsetX", MVMManager.Instance["AreaControlViewModel", "LSMFieldOffsetXActual", (object)0].ToString());
                                XmlManager.SetAttribute(ndList[0], experimentFile, "offsetY", MVMManager.Instance["AreaControlViewModel", "LSMFieldOffsetYActual", (object)0].ToString());
                                XmlManager.SetAttribute(ndList[0], experimentFile, "fineOffsetX", MVMManager.Instance["AreaControlViewModel", "LSMFieldOffsetXFine", (object)0].ToString());
                                XmlManager.SetAttribute(ndList[0], experimentFile, "fineOffsetY", MVMManager.Instance["AreaControlViewModel", "LSMFieldOffsetYFine", (object)0].ToString());
                                XmlManager.SetAttribute(ndList[0], experimentFile, "fineScaleX", MVMManager.Instance["AreaControlViewModel", "LSMFieldScaleXFine", (object)0].ToString());
                                XmlManager.SetAttribute(ndList[0], experimentFile, "fineScaleY", MVMManager.Instance["AreaControlViewModel", "LSMFieldScaleYFine", (object)0].ToString());
                                XmlManager.SetAttribute(ndList[0], experimentFile, "areaAngle", MVMManager.Instance["AreaControlViewModel", "LSMScanAreaAngle", (object)0].ToString());
                                XmlManager.SetAttribute(ndList[0], experimentFile, "horizontalFlip", MVMManager.Instance["AreaControlViewModel", "LSMFlipHorizontal", (object)0].ToString());
                                XmlManager.SetAttribute(ndList[0], experimentFile, "verticalFlip", MVMManager.Instance["AreaControlViewModel", "LSMFlipVerticalScan", (object)0].ToString());
                                XmlManager.SetAttribute(ndList[0], experimentFile, "powerMin", MVMManager.Instance["PowerControlViewModel", "PockelsVoltageMin0", (object)0].ToString());
                                XmlManager.SetAttribute(ndList[0], experimentFile, "powerMax", MVMManager.Instance["PowerControlViewModel", "PockelsVoltageMax0", (object)0].ToString());
                            }
                            break;
                        case GlobalExpAttribute.GALVO_BLEACH:
                            ndList = experimentFile.SelectNodes("/ThorImageExperiment/Photobleaching");

                            if (ndList.Count > 0)
                            {
                                XmlManager.SetAttribute(ndList[0], experimentFile, "bleachFrames", this.BleachFrames.ToString());
                                XmlManager.SetAttribute(ndList[0], experimentFile, "powerEnable", this.BleachPowerEnable.ToString());
                                XmlManager.SetAttribute(ndList[0], experimentFile, "powerPos", this.BleachPower.ToString());
                                XmlManager.SetAttribute(ndList[0], experimentFile, "laserEnable", this.BleachWavelengthEnable.ToString());
                                XmlManager.SetAttribute(ndList[0], experimentFile, "laserPos", this.BleachWavelength.ToString());
                                XmlManager.SetAttribute(ndList[0], experimentFile, "bleachQuery", this.BleachQuery.ToString());
                            }
                            break;
                        case GlobalExpAttribute.SLM_BLEACH:
                            //SLM share bleach frames with GG bleach:
                            ndList = experimentFile.SelectNodes("/ThorImageExperiment/Photobleaching");

                            if (ndList.Count > 0)
                            {
                                XmlManager.SetAttribute(ndList[0], experimentFile, "bleachFrames", this.BleachFrames.ToString());
                            }

                            //SLM:
                            ndList = experimentFile.SelectNodes("/ThorImageExperiment/SLM");

                            if (ndList.Count <= 0)
                            {
                                CreateXmlNode(experimentFile, "SLM");
                                ndList = experimentFile.SelectNodes("/ThorImageExperiment/SLM");
                            }
                            XmlManager.SetAttribute(ndList[0], experimentFile, "lastCalibTime", this.SLMLastCalibTime.ToString());
                            XmlManager.SetAttribute(ndList[0], experimentFile, "lastCalibTimeUnix", this.SLMLastCalibTimeUnix.ToString());
                            XmlManager.SetAttribute(ndList[0], experimentFile, "calibPower", this.SLMCalibPower.ToString());
                            XmlManager.SetAttribute(ndList[0], experimentFile, "calibDwell", this.SLMCalibDwell.ToString());
                            XmlManager.SetAttribute(ndList[0], experimentFile, "cycleDelay", this.SLMBleachDelay.ToString());
                            XmlManager.SetAttribute(ndList[0], experimentFile, "epochCount", this.EpochCount.ToString());
                            XmlManager.SetAttribute(ndList[0], experimentFile, "advanceMode", this.SLMSequenceOn ? "1" : "0");
                            XmlManager.SetAttribute(ndList[0], experimentFile, "holoGen3D", this.SLM3D ? "1" : "0");

                            ndList = experimentFile.SelectNodes("/ThorImageExperiment/SLM/Pattern");
                            for (int id = 0; id < ndList.Count; id++)
                            {
                                ndList[id].ParentNode.RemoveChild(ndList[id]);
                            }

                            for (int i = 0; i < this.SLMBleachWaveParams.Count; i++)
                            {
                                if (null == this.SLMBleachWaveParams[i])
                                    continue;

                                ndListP = experimentFile.SelectNodes("/ThorImageExperiment/SLM");
                                XmlNode node = experimentFile.CreateNode(XmlNodeType.Element, "Pattern", null);
                                ndListP[0].AppendChild(node);
                                ndList = experimentFile.SelectNodes("/ThorImageExperiment/SLM/Pattern");
                                XmlManager.SetAttribute(ndList[i], experimentFile, "name", this.SLMBleachWaveParams[i].Name.ToString());
                                XmlManager.SetAttribute(ndList[i], experimentFile, "pixelSizeUM", this.SLMBleachWaveParams[i].BleachWaveParams.UMPerPixel.ToString());
                                XmlManager.SetAttribute(ndList[i], experimentFile, "xOffsetUM", this.SLMBleachWaveParams[i].BleachWaveParams.CenterUM.X.ToString());
                                XmlManager.SetAttribute(ndList[i], experimentFile, "yOffsetUM", this.SLMBleachWaveParams[i].BleachWaveParams.CenterUM.Y.ToString());
                                XmlManager.SetAttribute(ndList[i], experimentFile, "roiWidthUM", this.SLMBleachWaveParams[i].BleachWaveParams.ROIWidthUM.ToString());
                                XmlManager.SetAttribute(ndList[i], experimentFile, "roiHeightUM", this.SLMBleachWaveParams[i].BleachWaveParams.ROIHeightUM.ToString());
                                XmlManager.SetAttribute(ndList[i], experimentFile, "pxSpacing", this.SLMBleachWaveParams[i].PixelSpacing.ToString());
                                XmlManager.SetAttribute(ndList[i], experimentFile, "durationMS", this.SLMBleachWaveParams[i].Duration.ToString());
                                XmlManager.SetAttribute(ndList[i], experimentFile, "iterations", this.SLMBleachWaveParams[i].BleachWaveParams.Iterations.ToString());
                                XmlManager.SetAttribute(ndList[i], experimentFile, "prePatIdleMS", this.SLMBleachWaveParams[i].BleachWaveParams.PrePatIdleTime.ToString());
                                XmlManager.SetAttribute(ndList[i], experimentFile, "postPatIdleMS", this.SLMBleachWaveParams[i].BleachWaveParams.PostPatIdleTime.ToString());
                                XmlManager.SetAttribute(ndList[i], experimentFile, "preIteIdleMS", this.SLMBleachWaveParams[i].BleachWaveParams.PreIdleTime.ToString());
                                XmlManager.SetAttribute(ndList[i], experimentFile, "postIteIdleMS", this.SLMBleachWaveParams[i].BleachWaveParams.PostIdleTime.ToString());
                                for (int j = 0; j < BleachCalibratePockelsVoltageMin0.Length; j++)
                                {
                                    //only persist when power is valid
                                    if (0 <= (double)this.SLMBleachWaveParams[i].BleachWaveParams.GetType().GetProperty((0 < j) ? "Power" + j : "Power").GetValue(this.SLMBleachWaveParams[i].BleachWaveParams))
                                    {
                                        XmlManager.SetAttribute(ndList[i], experimentFile, (0 < j) ? "power" + j : "power",
                                           this.SLMBleachWaveParams[i].BleachWaveParams.GetType().GetProperty((0 < j) ? "Power" + j : "Power").GetValue(this.SLMBleachWaveParams[i].BleachWaveParams).ToString());

                                        Double.TryParse(this.SLMBleachWaveParams[i].BleachWaveParams.GetType().GetProperty((0 < j) ? "MeasurePower" + j : "MeasurePower").GetValue(this.SLMBleachWaveParams[i].BleachWaveParams).ToString(), NumberStyles.Any, CultureInfo.InvariantCulture, out dTmp);
                                        XmlManager.SetAttribute(ndList[i], experimentFile, (0 < j) ? "measurePower" + j + "MW" : "measurePowerMW", dTmp.ToString());
                                        //keep measurePower at root for later retrieval after patterns cleared
                                        if (0 < dTmp) XmlManager.SetAttribute(ndListP[0], experimentFile, (0 < j) ? "measurePower" + j + "MW" : "measurePowerMW", dTmp.ToString());

                                        XmlManager.SetAttribute(ndList[i], experimentFile, (0 < j) ? "measurePower" + j + "MWPerUM2" : "measurePowerMWPerUM2",
                                            this.SLMBleachWaveParams[i].GetType().GetProperty((0 < j) ? "SLMMeasurePowerArea" + j : "SLMMeasurePowerArea").GetValue(this.SLMBleachWaveParams[i]).ToString());
                                    }
                                }
                                XmlManager.SetAttribute(ndList[i], experimentFile, "red", this.SLMBleachWaveParams[i].Red.ToString());
                                XmlManager.SetAttribute(ndList[i], experimentFile, "green", this.SLMBleachWaveParams[i].Green.ToString());
                                XmlManager.SetAttribute(ndList[i], experimentFile, "blue", this.SLMBleachWaveParams[i].Blue.ToString());
                                XmlManager.SetAttribute(ndList[i], experimentFile, "fileID", this.SLMBleachWaveParams[i].BleachWaveParams.ID.ToString());
                                XmlManager.SetAttribute(ndList[i], experimentFile, "shape", this.SLMBleachWaveParams[i].BleachWaveParams.shapeType);
                            }
                            //persist advance sequence mode, clean up sequences first
                            ndList = experimentFile.SelectNodes("/ThorImageExperiment/SLM/SequenceEpoch");
                            for (int id = 0; id < ndList.Count; id++)
                            {
                                ndList[id].ParentNode.RemoveChild(ndList[id]);
                            }
                            //create sequences
                            for (int i = 0; i < this.EpochSequence.Count; i++)
                            {
                                ndList = experimentFile.SelectNodes("/ThorImageExperiment/SLM");
                                XmlNode tempnode = experimentFile.CreateNode(XmlNodeType.Element, "SequenceEpoch", null);
                                ndList[0].AppendChild(tempnode);
                                ndList = experimentFile.SelectNodes("/ThorImageExperiment/SLM/SequenceEpoch");
                                XmlManager.SetAttribute(ndList[i], experimentFile, "sequence", SLMEpochSequence.IntArrayToString(SLMEpochSequence.ParseForIntArray(this.EpochSequence.ElementAt(i).SequenceStr)));
                                XmlManager.SetAttribute(ndList[i], experimentFile, "sequenceEpochCount", this.EpochSequence.ElementAt(i).EpochCountInt.ToString());
                            }
                            break;
                        case GlobalExpAttribute.OTM:
                            MVMManager.Instance["OTMControlViewModel", "PersistGlobalOTMCalibration"] = experimentFile;
                            break;
                        default:
                            break;
                    }
                    experimentFile.Save(xmlFiles[fid]);
                    if (fid == 0)
                    {
                        MVMManager.Instance.LoadSettings(SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS, false);
                    }

                    //give back current active:
                    if (0 == xmlFiles[fid].CompareTo(ResourceManagerCS.GetActiveSettingsFileString()))
                    {
                        this.ExperimentDoc = experimentFile;
                    }
                }
            }
            catch (Exception ex)
            {
                ThorLogging.ThorLog.Instance.TraceEvent(System.Diagnostics.TraceEventType.Verbose, 1, "PersistGlobalExperimentXML: " + ex.Message);
                MessageBox.Show("There was an error Persisting the experiment to all modalities. Some of your changes may not have been saved.");
            }
            ResourceManagerCS.ReturnDocMutexCS(SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS);
        }

        public void UpdateExpXMLSettings(ref XmlDocument xmlDoc)
        {
            try
            {
                // if no camera is available in system, do not overwrite active settings
                if ((int)ICamera.CameraType.LAST_CAMERA_TYPE == ResourceManagerCS.GetCameraType())
                    return;

                //if we are starting a new zstackpreview it means we are not swithing modalities, and we don't want to close the progress window
                if (!_startingContinuousZStackPreview)
                {
                    //stop preview before persistance, since it could switch modality afterward
                    CloseProgressWindow();
                }

                //Persist ROIs that are in the canvas
                OverlayManagerClass.Instance.PersistSaveROIs();

                bool tempSwitchCI;
                System.Globalization.CultureInfo originalCultureInfo;

                //Keep decimal dot in xml:
                tempSwitchCI = false;
                originalCultureInfo = (System.Globalization.CultureInfo)System.Threading.Thread.CurrentThread.CurrentCulture.Clone();

                if (0 == originalCultureInfo.NumberFormat.NumberDecimalSeparator.CompareTo(","))
                {
                    tempSwitchCI = true;
                    originalCultureInfo.NumberFormat.NumberDecimalSeparator = ".";
                    System.Threading.Thread.CurrentThread.CurrentCulture = originalCultureInfo;
                }

                XmlNodeList ndList;
                XmlNodeList ndListHW;
                XmlNodeList lightPathSequenceStepNdList;

                ndList = xmlDoc.SelectNodes("/ThorImageExperiment/LSM");
                if (ndList.Count > 0)
                {

                    if (LSMChannel == 4)
                    {
                        //channel is stored as a zero based index
                        //convert the enabled channels to the index value
                        int chan = (Convert.ToInt32(LSMChannelEnable0) |
                            (Convert.ToInt32(LSMChannelEnable1) << 1) |
                            (Convert.ToInt32(LSMChannelEnable2) << 2) |
                            (Convert.ToInt32(LSMChannelEnable3) << 3));
                        XmlManager.SetAttribute(ndList[0], xmlDoc, "channel", chan.ToString());
                    }
                    else
                    {
                        int ch = 0;
                        switch ((int)LSMChannel)
                        {
                            case 0: ch = 0x1; break;
                            case 1: ch = 0x2; break;
                            case 2: ch = 0x4; break;
                            case 3: ch = 0x8; break;
                        }

                        XmlManager.SetAttribute(ndList[0], xmlDoc, "channel", ch.ToString());
                    }
                }
                ndList = xmlDoc.SelectNodes("/ThorImageExperiment/ZStage");

                if (ndList.Count > 0)
                {
                    double stepSize = (double)MVMManager.Instance["ZControlViewModel", "ZScanStep", (object)0.0];

                    //ensure the sign of the step is correct
                    if ((double)MVMManager.Instance["ZControlViewModel", "ZScanStart", (object)0.0] > (double)MVMManager.Instance["ZControlViewModel", "ZScanStop", (object)0.0])
                    {
                        stepSize *= -1;
                    }

                    XmlManager.SetAttribute(ndList[0], xmlDoc, "steps", ((int)MVMManager.Instance["ZControlViewModel", "ZScanNumSteps", (object)0.0]).ToString());
                    XmlManager.SetAttribute(ndList[0], xmlDoc, "stepSizeUM", stepSize.ToString());
                    XmlManager.SetAttribute(ndList[0], xmlDoc, "setupPositionMM", ((double)MVMManager.Instance["ZControlViewModel", "ZPosition", (object)0.0]).ToString());
                    XmlManager.SetAttribute(ndList[0], xmlDoc, "startPos", ((double)MVMManager.Instance["ZControlViewModel", "ZScanStart", (object)0.0]).ToString());
                    XmlManager.SetAttribute(ndList[0], xmlDoc, "z2StageLock", ((bool)MVMManager.Instance["ZControlViewModel", "Z2StageLock", (object)0.0]) == true ? "1" : "0");
                    XmlManager.SetAttribute(ndList[0], xmlDoc, "z2StageMirror", ((bool)MVMManager.Instance["ZControlViewModel", "Z2StageMirror", (object)0.0]) == true ? "1" : "0");
                }

                ndListHW = HardwareDoc.SelectNodes("/HardwareSettings/Devices/ZStage");

                //persist the active ZStage name:
                if (ndListHW.Count > 0)
                {
                    string str = string.Empty;
                    for (int i = 0; i < ndListHW.Count; i++)
                    {
                        XmlManager.GetAttribute(ndListHW[i], this.HardwareDoc, "active", ref str);
                        if (1 == Convert.ToInt32(str))
                        {
                            XmlManager.GetAttribute(ndListHW[i], this.HardwareDoc, "dllName", ref str);
                            if (str != string.Empty)
                            {
                                XmlManager.SetAttribute(ndList[0], xmlDoc, "name", str);
                                break;
                            }
                        }
                    }
                }

                // Persist RStage Position
                ndList = xmlDoc.SelectNodes("/ThorImageExperiment/RStage");

                if (ndList.Count <= 0)
                {
                    CreateXmlNode(xmlDoc, "RStage");
                    ndList = xmlDoc.SelectNodes("/ThorImageExperiment/RStage");
                }
                if (ndList.Count > 0)
                {
                    XmlManager.SetAttribute(ndList[0], xmlDoc, "pos", ((double)MVMManager.Instance["ZControlViewModel", "RPosition", (object)0.0]).ToString());
                }

                // Persist Secondary Z Position
                ndList = xmlDoc.SelectNodes("/ThorImageExperiment/ZStage2");

                if (0 >= ndList.Count)
                {
                    CreateXmlNode(xmlDoc, "ZStage2");
                    ndList = xmlDoc.SelectNodes("/ThorImageExperiment/ZStage2");
                }
                if (0 < ndList.Count)
                {
                    XmlManager.SetAttribute(ndList[0], xmlDoc, "pos", ((double)MVMManager.Instance["ZControlViewModel", "Z2Position", (object)0.0]).ToString());
                }

                //SLM
                UpdateSLMFiles();

                ndList = xmlDoc.SelectNodes("/ThorImageExperiment/LightPath");
                if (ndList.Count <= 0)
                {
                    CreateXmlNode(xmlDoc, "LightPath");
                    ndList = xmlDoc.SelectNodes("/ThorImageExperiment/LightPath");
                }

                XmlManager.SetAttribute(ndList[0], xmlDoc, "GalvoGalvo", this.LightPathGGEnable.ToString());
                XmlManager.SetAttribute(ndList[0], xmlDoc, "GalvoResonance", this.LightPathGREnable.ToString());
                XmlManager.SetAttribute(ndList[0], xmlDoc, "Camera", this.LightPathCamEnable.ToString());
                if (-1 == this.InvertedLightPathPos)
                {
                    XmlManager.SetAttribute(ndList[0], xmlDoc, "InvertedLightPathPos", "-1");
                }
                else
                {
                    XmlManager.SetAttribute(ndList[0], xmlDoc, "InvertedLightPathPos", this.InvertedLightPathPos.ToString());
                }
                XmlManager.SetAttribute(ndList[0], xmlDoc, "NDD", this.PositionNDD.ToString());

                //////Capture Sequence
                ndList = xmlDoc.SelectNodes("/ThorImageExperiment/CaptureSequence");
                if (0 >= ndList.Count)
                {
                    ndList = xmlDoc.SelectNodes("ThorImageExperiment");
                    XmlElement elem = xmlDoc.CreateElement("CaptureSequence");
                    ndList[0].AppendChild(elem);
                    ndList = xmlDoc.SelectNodes("/ThorImageExperiment/CaptureSequence");
                }

                if (ndList.Count > 0)
                {
                    XmlManager.SetAttribute(ndList[0], xmlDoc, "enable", this.EnableSequentialCapture.ToString());
                }

                lightPathSequenceStepNdList = xmlDoc.SelectNodes("/ThorImageExperiment/CaptureSequence/LightPathSequenceStep");

                //Remove all channel steps and add those that are in capture sequence collection
                for (int i = 0; i < lightPathSequenceStepNdList.Count; i++)
                {
                    ndList[0].RemoveChild(lightPathSequenceStepNdList[i]);
                }
                for (int i = 0; i < _captureSequence.Count; i++)
                {
                    XmlNode importNode = ndList[0].OwnerDocument.ImportNode(_captureSequence[i].LightPathSequenceStepNode, true);
                    ndList[0].AppendChild(importNode);
                }
                ////End Capture Sequence

                //////(SLM) Bleach non-waveform criticals
                ndList = xmlDoc.SelectNodes("/ThorImageExperiment/Photobleaching");
                if (ndList.Count > 0)
                {
                    XmlManager.SetAttribute(ndList[0], xmlDoc, "bleachFrames", this.BleachFrames.ToString());
                }
                ndList = xmlDoc.SelectNodes("/ThorImageExperiment/SLM");
                if (ndList.Count > 0)
                {
                    XmlManager.SetAttribute(ndList[0], xmlDoc, "cycleDelay", this.SLMBleachDelay.ToString());
                    XmlManager.SetAttribute(ndList[0], xmlDoc, "advanceMode", this.SLMSequenceOn ? "1" : "0");
                    XmlManager.SetAttribute(ndList[0], xmlDoc, "holoGen3D", this.SLM3D ? "1" : "0");
                }
                ////End (SLM) Bleach non-waveform criticals

                //Set the date for the file
                string formatDate = "MM/dd/yyyy";
                string date = DateTime.Now.ToString(formatDate);

                ndList = xmlDoc.SelectNodes("/ThorImageExperiment/Date");

                if (ndList.Count > 0)
                {
                    XmlManager.SetAttribute(ndList[0], xmlDoc, "date", date);
                }
                ndList = xmlDoc.SelectNodes("/ThorImageExperiment/User");

                if (ndList.Count > 0)
                {
                    XmlManager.SetAttribute(ndList[0], xmlDoc, "name", Environment.UserName.ToString());
                }
                ndList = xmlDoc.SelectNodes("/ThorImageExperiment/Computer");

                if (ndList.Count > 0)
                {
                    XmlManager.SetAttribute(ndList[0], xmlDoc, "name", Environment.MachineName.ToString());
                }

                ndList = xmlDoc.SelectNodes("/ThorImageExperiment/Software");

                if (ndList.Count > 0)
                {
                    XmlManager.SetAttribute(ndList[0], xmlDoc, "version", System.Diagnostics.Process.GetCurrentProcess().MainModule.FileVersionInfo.FileVersion.ToString());
                }

                //Modality tag
                ndList = xmlDoc.SelectNodes("/ThorImageExperiment/Modality");
                if (ndList.Count <= 0)
                {
                    CreateXmlNode(xmlDoc, "Modality");
                    ndList = xmlDoc.SelectNodes("/ThorImageExperiment/Modality");
                }

                XmlManager.SetAttribute(ndList[0], xmlDoc, "primaryDetectorType", ResourceManagerCS.GetCameraType().ToString());
                XmlManager.SetAttribute(ndList[0], xmlDoc, "detectorLSMType", ResourceManagerCS.GetLSMType().ToString());

                //Update the channel step templates
                XmlDocument lightPathListDoc = new XmlDocument();
                string lightPathListFolder = Application.Current.Resources["LightPathListFolder"].ToString();
                //if the directory doesn't existe, create it.
                if (false == Directory.Exists(lightPathListFolder))
                {
                    Directory.CreateDirectory(lightPathListFolder);
                }

                string lightPathListFile = lightPathListFolder + "\\LightPathList.xml";

                //if the file doesnt exist or is corrupted, create it.
                if (true == Directory.Exists(lightPathListFile))
                {
                    lightPathListDoc.Load(lightPathListFile);
                    if (null == lightPathListDoc)
                    {
                        lightPathListDoc.LoadXml("<ThorImageLightPathList></ThorImageLightPathList>");
                    }
                }
                else
                {
                    lightPathListDoc.LoadXml("<ThorImageLightPathList></ThorImageLightPathList>");
                }

                ndList = lightPathListDoc.SelectNodes("/ThorImageLightPathList");
                lightPathSequenceStepNdList = lightPathListDoc.SelectNodes("/ThorImageLightPathList/LightPathSequenceStep");
                //remove all nodes before inserting the ones found in the lighpath sequence step collection
                for (int j = 0; j < lightPathSequenceStepNdList.Count; j++)
                {
                    ndList[0].RemoveChild(lightPathSequenceStepNdList[j]);
                }
                for (int j = 0; j < _lightPaths.Count; j++)
                {
                    XmlNode importNode = ndList[0].OwnerDocument.ImportNode(_lightPaths[j].LightPathSequenceStepNode, true);
                    ndList[0].AppendChild(importNode);
                }
                lightPathListDoc.Save(lightPathListFile);

                //reload the hardware doc to make sure the hardware document is updated to date:
                MVMManager.Instance.ReloadSettings(SettingsFileType.HARDWARE_SETTINGS);
                this.HardwareDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];

                ndListHW = this.HardwareDoc.SelectNodes("/HardwareSettings/Wavelength");

                if (ndListHW.Count > 0)
                {
                    for (int i = 0; i < ndListHW.Count; i++)
                    {
                        double bp = 0;
                        double wp = 255;
                        string autoTog = string.Empty;
                        string logTog = string.Empty;

                        switch (i)
                        {
                            case 0:
                                bp = BlackPoint0;
                                wp = WhitePoint0;
                                autoTog = (AutoManualTog1Checked) ? "1" : "0";
                                logTog = (LogScaleEnabled0) ? "1" : "0";
                                break;
                            case 1:
                                bp = BlackPoint1;
                                wp = WhitePoint1;
                                autoTog = (AutoManualTog2Checked) ? "1" : "0";
                                logTog = (LogScaleEnabled1) ? "1" : "0";
                                break;
                            case 2:
                                bp = BlackPoint2;
                                wp = WhitePoint2;
                                autoTog = (AutoManualTog3Checked) ? "1" : "0";
                                logTog = (LogScaleEnabled2) ? "1" : "0";
                                break;
                            case 3:
                                bp = BlackPoint3;
                                wp = WhitePoint3;
                                autoTog = (AutoManualTog4Checked) ? "1" : "0";
                                logTog = (LogScaleEnabled3) ? "1" : "0";
                                break;
                        }
                        XmlManager.SetAttribute(ndListHW[i], this.HardwareDoc, "bp", bp.ToString());
                        XmlManager.SetAttribute(ndListHW[i], this.HardwareDoc, "wp", wp.ToString());
                        XmlManager.SetAttribute(ndListHW[i], this.HardwareDoc, "AutoSta", autoTog);
                        XmlManager.SetAttribute(ndListHW[i], this.HardwareDoc, "LogSta", logTog);
                    }
                }

                MVMManager.Instance.SaveSettings(SettingsFileType.HARDWARE_SETTINGS);

                //give back CultureInfo:
                if (tempSwitchCI)
                {
                    originalCultureInfo.NumberFormat.NumberDecimalSeparator = ",";
                    System.Threading.Thread.CurrentThread.CurrentCulture = originalCultureInfo;
                }
            }
            catch (Exception ex)
            {
                ThorLogging.ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "Error Persisting the experiment settings: " + ex.Message);
                MessageBox.Show("There was an error Persisting the experiment or hardware settings. Some of your changes may not have been saved.");
                MVMManager.Instance.SaveSettings(SettingsFileType.HARDWARE_SETTINGS);
            }
        }

        #endregion Methods
    }
}