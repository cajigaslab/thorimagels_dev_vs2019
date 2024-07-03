namespace SequentialControl.Model
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Diagnostics;
    using System.IO;
    using System.Runtime.InteropServices;
    using System.Windows;
    using System.Windows.Media;
    using System.Xml;

    using ThorLogging;

    using ThorSharedTypes;

    public class SequentialControlModel
    {
        #region Fields

        private ObservableCollection<SequenceStep> _captureSequence = new ObservableCollection<SequenceStep>();
        private int _enableSequentialCapture = 0;
        private bool _isSequentialCapturing = false;
        private int _lastRemovedSequenceStep = 0;
        private int _lastRemovedTemplate = 0;
        private int _lastUpdatedTemplateStep = 0;
        private bool _sequentialStopped = true;
        private ObservableCollection<SequenceStep> _stepTemplates = new ObservableCollection<SequenceStep>();

        #endregion Fields

        #region Constructors

        /// <summary>
        /// Create a new instance of the SequentialControlModel class
        /// </summary>
        public SequentialControlModel()
        {
        }

        #endregion Constructors

        #region Properties

        public ObservableCollection<SequenceStep> CollectionCaptureSequence
        {
            get
            {
                return _captureSequence;
            }

            set
            {
                _captureSequence = value;
            }
        }

        public ObservableCollection<SequenceStep> CollectionSequences
        {
            get
            {
                return _stepTemplates;
            }

            set
            {
                _stepTemplates = value;
            }
        }

        public int EnableSequentialCapture
        {
            get
            {
                return _enableSequentialCapture;
            }
            set
            {
                _enableSequentialCapture = value;
                ReassignTemplateListLineNumbers();
            }
        }

        public bool IsSequentialCapturing
        {
            get
            {
                return _isSequentialCapturing;
            }
            set
            {
                _isSequentialCapturing = value;
            }
        }

        public int RemoveSequenceStep
        {
            get
            {
                return _lastRemovedSequenceStep;
            }
            set
            {
                _lastRemovedSequenceStep = value;
                _captureSequence.RemoveAt(_lastRemovedSequenceStep);
                ReassignCaptureSequenceLineNumbers();
            }
        }

        public int RemoveStepTemplate
        {
            get
            {
                return _lastRemovedTemplate;
            }
            set
            {
                _lastRemovedTemplate = value;

                //Before a template is removed, make sure all the sequence steps instances from it are also deleted
                bool loopAgain;
                do
                {
                    loopAgain = false;
                    for (int i = 0; i < _captureSequence.Count; i++)
                    {
                        if (_captureSequence[i].Name == _stepTemplates[_lastRemovedTemplate].Name)
                        {
                            _captureSequence.RemoveAt(i);
                            loopAgain = true;
                            break;
                        }
                    }
                }
                while (loopAgain);

                _stepTemplates.RemoveAt(_lastRemovedTemplate);
                ReassignTemplateListLineNumbers();
                ReassignCaptureSequenceLineNumbers();
            }
        }

        public bool SequentialStopped
        {
            get
            {
                return _sequentialStopped;
            }
            set
            {
                _sequentialStopped = value;
                if (_sequentialStopped)
                {
                    MVMManager.Instance["ScanControlViewModel", "EnablePMTGains"] = false;
                    StopSequentialPreview();
                }
            }
        }

        public int UpdateStepTemplate
        {
            get
            {
                return _lastUpdatedTemplateStep;
            }
            set
            {
                _lastUpdatedTemplateStep = value;

                SequenceStep cs = CreateCurrentSettingsSequenceStep(_stepTemplates[_lastUpdatedTemplateStep].Name);
                _stepTemplates[_lastUpdatedTemplateStep] = cs;
                for (int i = 0; i < _captureSequence.Count; i++)
                {
                    //update the sequence as well
                    if (_captureSequence[i].Name == cs.Name)
                    {
                        _captureSequence[i] = cs;
                    }
                }
                ReassignTemplateListLineNumbers();
                ReassignCaptureSequenceLineNumbers();
            }
        }

        public string[] UpdateTemplateName
        {
            set
            {
                foreach (SequenceStep seq in CollectionSequences)
                {
                    if (seq.Name == value?[0])
                    {
                        seq.Name = value[1];
                    }
                }
                foreach (SequenceStep seq in CollectionCaptureSequence)
                {
                    if (seq.Name == value?[0])
                    {
                        seq.Name = value[1];
                    }
                }
            }
        }

        #endregion Properties

        #region Methods

        public Decimal Power2PercentConvertion(double value, double Max, double Min)
        {
            if (Max != Min)
            {
                Decimal dec = new Decimal((value - Min) * 100 / (Max - Min));
                return dec = Decimal.Round(dec, 2);
            }
            else return new Decimal(value);
        }

        public void ReassignCaptureSequenceLineNumbers()
        {
            for (int i = 0; i < _captureSequence.Count; i++)
            {
                //Need to add 1 to the SequenceLineNumber because we display this value
                _captureSequence[i].SequenceLineNumber = i + 1;
                _captureSequence[i].CaptureSequenceWidth = (_captureSequence.Count >= 6) ? 157 : 179;
            }
        }

        public void ReassignTemplateListLineNumbers()
        {
            for (int i = 0; i < _stepTemplates.Count; i++)
            {
                _stepTemplates[i].TemplateLineNumber = i;
                if (0 == EnableSequentialCapture)
                {
                    _stepTemplates[i].StepTemplateWidth = (_stepTemplates.Count >= 6) ? 354 : 376;
                }
                else
                {
                    _stepTemplates[i].StepTemplateWidth = (_stepTemplates.Count >= 6) ? 157 : 179;
                }
            }
        }

        public void StartSequentialPreview()
        {
            //stop Live Capture
            MVMManager.Instance["CaptureSetupViewModel", "LiveCaptureProperty"] = false;

            if (false == IsSequentialCapturing)
            {
                IsSequentialCapturing = true;
                SequentialStopped = false;

                string sequentialCacheDirectory = ResourceManagerCS.GetMyDocumentsThorImageFolderString() + "SequentialCaptureCache";

                //update Active.xml
                MVMManager.Instance["CaptureSetupViewModel", "PersistDataNow"] = true;
                MVMManager.Instance["ImageViewCaptureSetupVM", "IsInSequentialMode"] = true;

                try
                {
                    ResourceManagerCS.DeleteDirectory(sequentialCacheDirectory);
                    Directory.CreateDirectory(sequentialCacheDirectory);

                    //Copy Active.xml to SequentialCaptureCache\Experiment.xml
                    string templatesFolder = ResourceManagerCS.GetCaptureTemplatePathString();
                    string srcFile = templatesFolder + "Active.xml";
                    string destFile = sequentialCacheDirectory + "\\Experiment.xml";
                    File.Copy(srcFile, destFile);
                    MVMManager.Instance["ImageViewCaptureSetupVM", "SequentialExperimentPath"] = srcFile;
                }
                catch (IOException e)
                {
                    ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, GetType().Name + " " + e.Message);
                }

                //stop the background hardware updates
                MVMManager.Instance["CaptureSetupViewModel", "BWHardware"] = false;

                MVMManager.Instance["ScanControlViewModel", "EnablePMTGains"] = false;
                MVMManager.Instance["ScanControlViewModel", "EnablePMTGains"] = true;

                CaptureSequentialPreview();

                //restart the background hardware updates
                MVMManager.Instance["CaptureSetupViewModel", "BWHardware"] = true;
                MVMManager.Instance["CaptureSetupViewModel", "PreviewProtocol"] = "SequentialPreview";
            }
        }

        public void TemplateListAdd()
        {
            XmlDocument listDoc = new XmlDocument();
            listDoc.LoadXml("<ThorImageLightPathList></ThorImageLightPathList>");

            //Dialog to allow the user to enter a name for the current Channel Step being saved
            SequenceStepNameEdit dlg;
            bool repeatedName;
            do
            {
                repeatedName = false;
                dlg = new SequenceStepNameEdit();
                if (false == dlg.ShowDialog())
                {
                    return;
                }

                for (int i = 0; i < _stepTemplates.Count; i++)
                {
                    if (_stepTemplates[i].Name == dlg.SequenceStepName)
                    {
                        MessageBox.Show("The name for the new Sequence Step template must be unique. Please use a different name.");
                        repeatedName = true;
                        break;
                    }
                }
            } while (repeatedName);

            _stepTemplates.Add(CreateCurrentSettingsSequenceStep(dlg.SequenceStepName));
            ReassignTemplateListLineNumbers();
        }

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "CaptureSequentialPreview", CharSet = CharSet.Unicode)]
        private static extern bool CaptureSequentialPreview();

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "StopSequentialPreview", CharSet = CharSet.Unicode)]
        private static extern bool StopSequentialPreview();

        private SequenceStep CreateCurrentSettingsSequenceStep(string name)
        {
            XmlDocument sequenceListDoc = new XmlDocument();
            sequenceListDoc.LoadXml("<ThorImageLightPathList></ThorImageLightPathList>");

            XmlNodeList ndList = sequenceListDoc.SelectNodes("ThorImageLightPathList");
            if (0 >= ndList.Count)
            {
                XmlElement elem = sequenceListDoc.CreateElement("ThorImageLightPathList");
                sequenceListDoc.AppendChild(elem);
                ndList = sequenceListDoc.SelectNodes("ThorImageLightPathList");
            }

            //Create new LightPathSequenceStep Node
            XmlElement temp = sequenceListDoc.CreateElement("LightPathSequenceStep");
            ndList[0].AppendChild(temp);
            ndList = sequenceListDoc.SelectNodes("/ThorImageLightPathList/LightPathSequenceStep");

            //Update the name of the Capture Sequence Step in the xml file
            XmlManager.SetAttribute(ndList[ndList.Count - 1], sequenceListDoc, "name", name);

            XmlNode stepRootNode = ndList[ndList.Count - 1];

            XmlNodeList innderNdList = stepRootNode.SelectNodes("Wavelengths");

            //Clear Wavelengths node, create it if if doesn't exist
            if (0 < innderNdList.Count)
            {
                innderNdList[0].RemoveAll();
            }
            else
            {
                XmlElement elem = sequenceListDoc.CreateElement("Wavelengths");
                stepRootNode.AppendChild(elem);
            }

            innderNdList = stepRootNode.SelectNodes("Wavelengths");

            XmlManager.SetAttribute(innderNdList[0], sequenceListDoc, "nyquistExWavelengthNM", MVMManager.Instance["AreaControlViewModel", "NyquistExWavelength"].ToString());
            XmlManager.SetAttribute(innderNdList[0], sequenceListDoc, "nyquistEmWavelengthNM", MVMManager.Instance["AreaControlViewModel", "NyquistEmWavelength"].ToString());

            XmlElement newElement;

            //Add the wavelength names and exposure time for the current Capture Sequence Step
            XmlDocument doc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];
            XmlNodeList waveList = doc.SelectNodes("/HardwareSettings/Wavelength");

            Color[] colors = (Color[])MVMManager.Instance["ImageViewCaptureSetupVM", "DefaultChannelColors"];
            string[] colornames = (string[])MVMManager.Instance["ImageViewCaptureSetupVM", "DefaultChannelColorNames"];

            int lsmChannels = 0, cameraChannels = 0;
            if (ResourceManagerCS.GetCameraType() == (int)ICamera.CameraType.LSM)
            {
                lsmChannels = (int)MVMManager.Instance["ScanControlViewModel", "LSMChannel"];
                switch (lsmChannels)
                {
                    case 1:
                        {
                            newElement = CreateWavelengthTag(waveList[0].Attributes["name"].Value, sequenceListDoc, colornames[0], colors[0], 0);
                            innderNdList[0].AppendChild(newElement);

                        }
                        break;
                    case 2:
                        {
                            newElement = CreateWavelengthTag(waveList[1].Attributes["name"].Value, sequenceListDoc, colornames[1], colors[1], 1);
                            innderNdList[0].AppendChild(newElement);
                        }
                        break;
                    case 4:
                        {
                            newElement = CreateWavelengthTag(waveList[2].Attributes["name"].Value, sequenceListDoc, colornames[2], colors[2], 2);
                            innderNdList[0].AppendChild(newElement);
                        }
                        break;
                    case 8:
                        {
                            newElement = CreateWavelengthTag(waveList[3].Attributes["name"].Value, sequenceListDoc, colornames[3], colors[3], 3);
                            innderNdList[0].AppendChild(newElement);
                        }
                        break;
                    default:
                        {
                            for (int j = 0; j < waveList.Count; j++)
                            {
                                bool isEnabled = false;

                                switch (j)
                                {
                                    case 0: isEnabled = (bool)MVMManager.Instance["ScanControlViewModel", "LSMChannelEnable0"]; break;
                                    case 1: isEnabled = (bool)MVMManager.Instance["ScanControlViewModel", "LSMChannelEnable1"]; break;
                                    case 2: isEnabled = (bool)MVMManager.Instance["ScanControlViewModel", "LSMChannelEnable2"]; break;
                                    case 3: isEnabled = (bool)MVMManager.Instance["ScanControlViewModel", "LSMChannelEnable3"]; break;
                                }

                                if (isEnabled)
                                {
                                    newElement = CreateWavelengthTag(waveList[j].Attributes["name"].Value, sequenceListDoc, colornames[j], colors[j], j);
                                    innderNdList[0].AppendChild(newElement);
                                }
                            }
                        }
                        break;
                }
            }
            else
            {
                cameraChannels = (int)MVMManager.Instance["CameraControlViewModel", "ChannelNum"];
                // NOTE: "ChannelNums" is a bit mask of active channels, not a number of channels
                for(int i = cameraChannels, c = 0; i > 0; i>>=1, ++c)
                {
                    newElement = CreateWavelengthTag(waveList[c].Attributes["name"].Value, sequenceListDoc, colornames[c], colors[c], 0);
                    innderNdList[0].AppendChild(newElement);
                }
                

            }

            //set the node to LSM, create it if it doesn't exist
            innderNdList = stepRootNode.SelectNodes("LSM");
            if (0 >= innderNdList.Count)
            {
                XmlElement elem = sequenceListDoc.CreateElement("LSM");
                stepRootNode.AppendChild(elem);
                innderNdList = stepRootNode.SelectNodes("LSM");
            }

            //Save the channel property for LSM in binary format
            XmlManager.SetAttribute(innderNdList[0], sequenceListDoc, "channel", lsmChannels.ToString());

            //set the node to Camera, create it if it doesn't exist
            innderNdList = stepRootNode.SelectNodes("Camera");
            if (0 >= innderNdList.Count)
            {
                XmlElement elem = sequenceListDoc.CreateElement("Camera");
                stepRootNode.AppendChild(elem);
                innderNdList = stepRootNode.SelectNodes("Camera");
            }

            //Save the channel property for Camera in binary format
            XmlManager.SetAttribute(innderNdList[0], sequenceListDoc, "channel", cameraChannels.ToString());

            //Save the exposure to the Camera Tag
            double exposure = (double)MVMManager.Instance["CameraControlViewModel", "ExposureTimeCam"];
            XmlManager.SetAttribute(innderNdList[0], sequenceListDoc, "exposure", exposure.ToString());

            //set the node to MCLS, create it if it doesn't exist
            innderNdList = stepRootNode.SelectNodes("MCLS");
            if (0 >= innderNdList.Count)
            {
                XmlElement elem = sequenceListDoc.CreateElement("MCLS");
                stepRootNode.AppendChild(elem);
                innderNdList = stepRootNode.SelectNodes("MCLS");
            }

            //add all the attributes for MCLS for the current Capture Sequence Step
            decimal power1percent = Power2PercentConvertion((double)MVMManager.Instance["MultiLaserControlViewModel", "Laser1Power"],
                                                            (double)MVMManager.Instance["MultiLaserControlViewModel", "Laser1Max"],
                                                            (double)MVMManager.Instance["MultiLaserControlViewModel", "Laser1Min"]);
            decimal power2percent = Power2PercentConvertion((double)MVMManager.Instance["MultiLaserControlViewModel", "Laser2Power"],
                                                            (double)MVMManager.Instance["MultiLaserControlViewModel", "Laser2Max"],
                                                            (double)MVMManager.Instance["MultiLaserControlViewModel", "Laser2Min"]);
            decimal power3percent = Power2PercentConvertion((double)MVMManager.Instance["MultiLaserControlViewModel", "Laser3Power"],
                                                            (double)MVMManager.Instance["MultiLaserControlViewModel", "Laser3Max"],
                                                            (double)MVMManager.Instance["MultiLaserControlViewModel", "Laser3Min"]);
            decimal power4percent = Power2PercentConvertion((double)MVMManager.Instance["MultiLaserControlViewModel", "Laser4Power"],
                                                            (double)MVMManager.Instance["MultiLaserControlViewModel", "Laser4Max"],
                                                            (double)MVMManager.Instance["MultiLaserControlViewModel", "Laser4Min"]);
            // SetAttribute(innderNdList[0], sequenceListDoc, "MainLaserSelection", this.MainLaserIndex.ToString());
            XmlManager.SetAttribute(innderNdList[0], sequenceListDoc, "enable1", MVMManager.Instance["MultiLaserControlViewModel", "Laser1Enable"].ToString());
            XmlManager.SetAttribute(innderNdList[0], sequenceListDoc, "power1", MVMManager.Instance["MultiLaserControlViewModel", "Laser1Power"].ToString());
            XmlManager.SetAttribute(innderNdList[0], sequenceListDoc, "enable2", MVMManager.Instance["MultiLaserControlViewModel", "Laser2Enable"].ToString());
            XmlManager.SetAttribute(innderNdList[0], sequenceListDoc, "power2", MVMManager.Instance["MultiLaserControlViewModel", "Laser2Power"].ToString());
            XmlManager.SetAttribute(innderNdList[0], sequenceListDoc, "enable3", MVMManager.Instance["MultiLaserControlViewModel", "Laser3Enable"].ToString());
            XmlManager.SetAttribute(innderNdList[0], sequenceListDoc, "power3", MVMManager.Instance["MultiLaserControlViewModel", "Laser3Power"].ToString());
            XmlManager.SetAttribute(innderNdList[0], sequenceListDoc, "enable4", MVMManager.Instance["MultiLaserControlViewModel", "Laser4Enable"].ToString());
            XmlManager.SetAttribute(innderNdList[0], sequenceListDoc, "power4", MVMManager.Instance["MultiLaserControlViewModel", "Laser4Power"].ToString());
            XmlManager.SetAttribute(innderNdList[0], sequenceListDoc, "power1percent", power1percent.ToString());
            XmlManager.SetAttribute(innderNdList[0], sequenceListDoc, "power2percent", power2percent.ToString());
            XmlManager.SetAttribute(innderNdList[0], sequenceListDoc, "power3percent", power3percent.ToString());
            XmlManager.SetAttribute(innderNdList[0], sequenceListDoc, "power4percent", power4percent.ToString());
            //Set these to set visibility of wavelength value when hovering over sequential capture steps
            XmlManager.SetAttribute(innderNdList[0], sequenceListDoc, "allttl", MVMManager.Instance["MultiLaserControlViewModel", "LaserAllTTL"].ToString());
            XmlManager.SetAttribute(innderNdList[0], sequenceListDoc, "allanalog", MVMManager.Instance["MultiLaserControlViewModel", "LaserAllAnalog"].ToString());
            XmlManager.SetAttribute(innderNdList[0], sequenceListDoc, "wavelength1", MVMManager.Instance["MultiLaserControlViewModel", "Laser1Wavelength"].ToString());
            XmlManager.SetAttribute(innderNdList[0], sequenceListDoc, "wavelength2", MVMManager.Instance["MultiLaserControlViewModel", "Laser2Wavelength"].ToString());
            XmlManager.SetAttribute(innderNdList[0], sequenceListDoc, "wavelength3", MVMManager.Instance["MultiLaserControlViewModel", "Laser3Wavelength"].ToString());
            XmlManager.SetAttribute(innderNdList[0], sequenceListDoc, "wavelength4", MVMManager.Instance["MultiLaserControlViewModel", "Laser4Wavelength"].ToString());

            //set the node to MultiPhotonLaser, create it if it doesn't exist
            innderNdList = stepRootNode.SelectNodes("MultiPhotonLaser");
            if (0 >= innderNdList.Count)
            {
                XmlElement elem = sequenceListDoc.CreateElement("MultiPhotonLaser");
                stepRootNode.AppendChild(elem);
                innderNdList = stepRootNode.SelectNodes("MultiPhotonLaser");
            }

            //add all the attributes for multiphoton laser for the current Capture Sequence Step
            XmlManager.SetAttribute(innderNdList[0], sequenceListDoc, "pos", MVMManager.Instance["MultiphotonControlViewModel", "Laser1Position"].ToString());

            //set the node to PinholeWheel, create it if it doesn't exist
            innderNdList = stepRootNode.SelectNodes("PinholeWheel");
            if (0 >= innderNdList.Count)
            {
                XmlElement elem = sequenceListDoc.CreateElement("PinholeWheel");
                stepRootNode.AppendChild(elem);
                innderNdList = stepRootNode.SelectNodes("PinholeWheel");
            }

            //add all the attributes for PinholeWheel for the current Capture Sequence Step
            if (innderNdList.Count > 0)
            {
                XmlManager.SetAttribute(innderNdList[0], sequenceListDoc, "position", MVMManager.Instance["PinholeControlViewModel", "PinholePosition", (object)1.0].ToString());
                string atrVal = MVMManager.Instance["PinholeControlViewModel", "PinholeSizeUM", (object)1.0].ToString();
                XmlManager.SetAttribute(innderNdList[0], sequenceListDoc, "micrometers", atrVal);
                XmlManager.SetAttribute(innderNdList[0], sequenceListDoc, "adu", MVMManager.Instance["PinholeControlViewModel", "PinholeADUs", (object)1.0].ToString());
            }

            //set the node to PMT, create it if it doesn't exist
            innderNdList = stepRootNode.SelectNodes("PMT");
            if (0 >= innderNdList.Count)
            {
                XmlElement elem = sequenceListDoc.CreateElement("PMT");
                stepRootNode.AppendChild(elem);
                innderNdList = stepRootNode.SelectNodes("PMT");
            }

            //add all the attributes for PMT for the current Capture Sequence Step
            HwVal<double> ga = (HwVal<double>)MVMManager.Instance["ScanControlViewModel", "PMTGain", 0, (object)0.0];
            XmlManager.SetAttribute(innderNdList[0], sequenceListDoc, "gainA", Math.Round(ga.Value, 2).ToString());
            XmlManager.SetAttribute(innderNdList[0], sequenceListDoc, "enableA", (0 < ga.Value) ? "1" : "0");
            HwVal<double> gb = (HwVal<double>)MVMManager.Instance["ScanControlViewModel", "PMTGain", 1, (object)0.0];
            XmlManager.SetAttribute(innderNdList[0], sequenceListDoc, "gainB", Math.Round(gb.Value, 2).ToString());
            XmlManager.SetAttribute(innderNdList[0], sequenceListDoc, "enableB", (0 < gb.Value) ? "1" : "0");
            HwVal<double> gc = (HwVal<double>)MVMManager.Instance["ScanControlViewModel", "PMTGain", 2, (object)0.0];
            XmlManager.SetAttribute(innderNdList[0], sequenceListDoc, "gainC", Math.Round(gc.Value, 2).ToString());
            XmlManager.SetAttribute(innderNdList[0], sequenceListDoc, "enableC", (0 < gc.Value) ? "1" : "0");
            HwVal<double> gd = (HwVal<double>)MVMManager.Instance["ScanControlViewModel", "PMTGain", 3, (object)0.0];
            XmlManager.SetAttribute(innderNdList[0], sequenceListDoc, "gainD", Math.Round(gd.Value, 2).ToString());
            XmlManager.SetAttribute(innderNdList[0], sequenceListDoc, "enableD", (0 < gd.Value) ? "1" : "0");

            //set the lightPath settings
            innderNdList = stepRootNode.SelectNodes("LightPath");
            if (0 >= innderNdList.Count)
            {
                XmlElement elem = sequenceListDoc.CreateElement("LightPath");
                stepRootNode.AppendChild(elem);
                innderNdList = stepRootNode.SelectNodes("LightPath");
            }

            XmlManager.SetAttribute(innderNdList[0], sequenceListDoc, "GalvoGalvo", MVMManager.Instance["LightPathControlViewModel", "LightPathGGEnable", (object)0].ToString());
            XmlManager.SetAttribute(innderNdList[0], sequenceListDoc, "GalvoResonance", MVMManager.Instance["LightPathControlViewModel", "LightPathGREnable", (object)0].ToString());
            XmlManager.SetAttribute(innderNdList[0], sequenceListDoc, "Camera", MVMManager.Instance["LightPathControlViewModel", "LightPathCamEnable", (object)0].ToString());
            if (-1 == (int)MVMManager.Instance["LightPathControlViewModel", "InvertedLightPathPos", (object)0])
            {
                XmlManager.SetAttribute(innderNdList[0], sequenceListDoc, "InvertedLightPathPos", "-1");
            }
            else
            {
                XmlManager.SetAttribute(innderNdList[0], sequenceListDoc, "InvertedLightPathPos", MVMManager.Instance["LightPathControlViewModel", "InvertedLightPathPos", (object)0].ToString());
            }
            XmlManager.SetAttribute(innderNdList[0], sequenceListDoc, "NDD", MVMManager.Instance["LightPathControlViewModel", "PositionNDD", (object)0].ToString());

            //set the pockels settings
            innderNdList = stepRootNode.SelectNodes("Pockels");
            if (0 >= innderNdList.Count)
            {
                XmlElement elem1 = sequenceListDoc.CreateElement("Pockels");
                stepRootNode.AppendChild(elem1);
                XmlElement elem2 = sequenceListDoc.CreateElement("Pockels");
                stepRootNode.AppendChild(elem2);
                XmlElement elem3 = sequenceListDoc.CreateElement("Pockels");
                stepRootNode.AppendChild(elem3);
                XmlElement elem4 = sequenceListDoc.CreateElement("Pockels");
                stepRootNode.AppendChild(elem4);
                innderNdList = stepRootNode.SelectNodes("Pockels");
            }

            MVMManager.Instance["PowerControlViewModel", "PockelsNodeList"] = new KeyValuePair<XmlNodeList, XmlDocument>(innderNdList, sequenceListDoc);

            //set the node to DigitalIO, create it if it doesn't exist
            innderNdList = stepRootNode.SelectNodes("DigitalIO");
            if (0 >= innderNdList.Count)
            {
                XmlElement elem = sequenceListDoc.CreateElement("DigitalIO");
                stepRootNode.AppendChild(elem);
                innderNdList = stepRootNode.SelectNodes("DigitalIO");
            }

            //add all the attributes for digital switches for the current Capture Sequence Step
            XmlManager.SetAttribute(innderNdList[0], sequenceListDoc, "enable", MVMManager.Instance["DigitalOutputSwitchesViewModel", "SwitchEnable"].ToString());
            ObservableCollection<IntPC> switchState = (ObservableCollection<IntPC>)MVMManager.Instance["DigitalOutputSwitchesViewModel", "SwitchState"];
            for (int i = 0; i < switchState.Count; i++)
            {
                XmlManager.SetAttribute(innderNdList[0], sequenceListDoc, string.Format("digOut{0}", i + 1), switchState[i].Value.ToString());
            }

            //set the EpiTurret settings
            innderNdList = stepRootNode.SelectNodes("EPITurret");
            if (0 >= innderNdList.Count)
            {
                XmlElement elem = sequenceListDoc.CreateElement("EPITurret");
                stepRootNode.AppendChild(elem);
                innderNdList = stepRootNode.SelectNodes("EPITurret");
            }

            XmlManager.SetAttribute(innderNdList[0], sequenceListDoc, "position", MVMManager.Instance["EpiTurretControlViewModel", "EpiTurretPos"].ToString());
            XmlManager.SetAttribute(innderNdList[0], sequenceListDoc, "positionName", (string)MVMManager.Instance["EpiTurretControlViewModel", "EpiTurretPosName"]);

            //Create the sequence step and return it
            SequenceStep cs = new SequenceStep(name, stepRootNode, ndList.Count, false);
            return cs;
        }

        private XmlElement CreateWavelengthTag(string name, XmlDocument doc, string colorName, Color color, int channelIndex)
        {
            //create a new XML tag for the wavelength settings
            XmlElement newElement = doc.CreateElement("Wavelength");

            XmlAttribute nameAttribute = doc.CreateAttribute("name");
            XmlAttribute expAttribute = doc.CreateAttribute("exposureTimeMS");
            XmlAttribute colorNameAttribute = doc.CreateAttribute("colorName");
            XmlAttribute channelIndexAttribute = doc.CreateAttribute("channelIndex");
            XmlAttribute colorAttribute = doc.CreateAttribute("color");

            nameAttribute.Value = name;
            expAttribute.Value = "0";
            colorAttribute.Value = color.ToString();
            channelIndexAttribute.Value = channelIndex.ToString();
            colorNameAttribute.Value = colorName;

            newElement.Attributes.Append(nameAttribute);
            newElement.Attributes.Append(expAttribute);
            newElement.Attributes.Append(colorNameAttribute);
            newElement.Attributes.Append(colorAttribute);
            newElement.Attributes.Append(channelIndexAttribute);

            return newElement;
        }

        #endregion Methods
    }
}