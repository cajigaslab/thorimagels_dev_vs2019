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
                                XmlManager.SetAttribute(ndList[0], experimentFile, "pixelSizeUM", ((PixelSizeUM)MVMManager.Instance["AreaControlViewModel", "LSMUMPerPixel", (object)0]).PixelWidthUM.ToString());
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
                                XmlManager.SetAttribute(ndList[0], experimentFile, "powerShiftUS", this.PowerShiftUS.ToString());
                            }
                            break;
                        case GlobalExpAttribute.SLM_BLEACH:
                            //SLM share bleach frames with GG bleach:
                            ndList = experimentFile.SelectNodes("/ThorImageExperiment/Photobleaching");

                            if (ndList.Count > 0)
                            {
                                XmlManager.SetAttribute(ndList[0], experimentFile, "bleachFrames", this.BleachFrames.ToString());
                                XmlManager.SetAttribute(ndList[0], experimentFile, "powerShiftUS", this.PowerShiftUS.ToString());
                            }

                            //SLM:
                            ndList = experimentFile.SelectNodes("/ThorImageExperiment/SLM");

                            if (ndList.Count <= 0)
                            {
                                XmlManager.CreateXmlNode(experimentFile, "SLM");
                                ndList = experimentFile.SelectNodes("/ThorImageExperiment/SLM");
                            }
                            XmlManager.SetAttribute(ndList[0], experimentFile, "lastCalibTime", this.SLMLastCalibTime.ToString());
                            XmlManager.SetAttribute(ndList[0], experimentFile, "lastCalibTimeUnix", this.SLMLastCalibTimeUnix.ToString());
                            XmlManager.SetAttribute(ndList[0], experimentFile, "calibPower", this.SLMCalibPower.ToString());
                            XmlManager.SetAttribute(ndList[0], experimentFile, "calibDwell", this.SLMCalibDwell.ToString());
                            XmlManager.SetAttribute(ndList[0], experimentFile, "cycleDelay", this.SLMBleachDelay.ToString());
                            XmlManager.SetAttribute(ndList[0], experimentFile, "epochCount", this.EpochCount.ToString());
                            XmlManager.SetAttribute(ndList[0], experimentFile, "advanceMode", this.SLMSequenceOn ? "1" : "0");
                            XmlManager.SetAttribute(ndList[0], experimentFile, "randomEpochs", this.SLMRandomEpochs ? "1" : "0");
                            XmlManager.SetAttribute(ndList[0], experimentFile, "holoGen3D", this.SLM3D ? "1" : "0");
                            XmlManager.SetAttribute(ndList[0], experimentFile, "refractiveIndex", this.RefractiveIndex.ToString());
                            XmlManager.SetAttribute(ndList[0], experimentFile, "sequenceEpochDelay", this.SLMSequenceEpochDelay.ToString());

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
                                XmlManager.SetAttribute(ndList[i], experimentFile, "phaseType", this.SLMBleachWaveParams[i].PhaseType.ToString());
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
                        case GlobalExpAttribute.SLM_ZREF:
                            ndList = experimentFile.SelectNodes("/ThorImageExperiment/SLM");
                            if (ndList.Count <= 0)
                            {
                                XmlManager.CreateXmlNode(experimentFile, "SLM");
                                ndList = experimentFile.SelectNodes("/ThorImageExperiment/SLM");
                            }
                            XmlManager.SetAttribute(ndList[0], experimentFile, "zRefMM", ((double)MVMManager.Instance["ZControlViewModel", "ZPosition", (object)0.0]).ToString());
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

                //Persist ROIs that are in the canvas, except in SLM calibration:
                if (null == _slmParamEditWin || View.SLMParamEditWin.SLMPanelMode.Calibration != _slmParamEditWin.PanelMode)
                    OverlayManagerClass.Instance.PersistSaveROIs(true);

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

                ndList = xmlDoc.SelectNodes("/ThorImageExperiment/LSM");
                if (ndList.Count > 0)
                {
                    XmlManager.SetAttribute(ndList[0], xmlDoc, "channel", LSMChannel.ToString());
                }

                //SLM
                UpdateSLMFiles();

                ///*** (SLM) Bleach non-waveform criticals or shared params ***///
                ndList = xmlDoc.SelectNodes("/ThorImageExperiment/Photobleaching");
                if (ndList.Count > 0)
                {
                    XmlManager.SetAttribute(ndList[0], xmlDoc, "bleachFrames", this.BleachFrames.ToString());
                    XmlManager.SetAttribute(ndList[0], xmlDoc, "powerShiftUS", this.PowerShiftUS.ToString());
                }
                ndList = xmlDoc.SelectNodes("/ThorImageExperiment/SLM");
                if (ndList.Count > 0)
                {
                    XmlManager.SetAttribute(ndList[0], xmlDoc, "cycleDelay", this.SLMBleachDelay.ToString());
                    XmlManager.SetAttribute(ndList[0], xmlDoc, "advanceMode", this.SLMSequenceOn ? "1" : "0");
                    XmlManager.SetAttribute(ndList[0], xmlDoc, "randomEpochs", this.SLMRandomEpochs ? "1" : "0");
                    XmlManager.SetAttribute(ndList[0], xmlDoc, "holoGen3D", this.SLM3D ? "1" : "0");
                    XmlManager.SetAttribute(ndList[0], xmlDoc, "refractiveIndex", this.RefractiveIndex.ToString());
                    XmlManager.SetAttribute(ndList[0], xmlDoc, "sequenceEpochDelay", this.SLMSequenceEpochDelay.ToString());

                    //non-global SLM properties:
                    XmlManager.SetAttribute(ndList[0], xmlDoc, "calibZoffsetUM", this.DefocusSavedUM.ToString());
                }
                ///*** End (SLM) Bleach non-waveform criticals or shared params ***///

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
                    XmlManager.CreateXmlNode(xmlDoc, "Modality");
                    ndList = xmlDoc.SelectNodes("/ThorImageExperiment/Modality");
                }

                XmlManager.SetAttribute(ndList[0], xmlDoc, "primaryDetectorType", ResourceManagerCS.GetCameraType().ToString());
                XmlManager.SetAttribute(ndList[0], xmlDoc, "detectorLSMType", ResourceManagerCS.GetLSMType().ToString());

                //reload the hardware doc to make sure the hardware document is updated to date:
                MVMManager.Instance.ReloadSettings(SettingsFileType.HARDWARE_SETTINGS);
                HardwareDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];

                ndListHW = this.HardwareDoc.SelectNodes("/HardwareSettings/Wavelength");

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