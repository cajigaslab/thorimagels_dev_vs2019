namespace HardwareState
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Threading.Tasks;
    using System.Xml;

    using ThorLogging;

    public class HwState
    {
        #region Fields

        const int NAME_LENGTH = 256;

        private static HwState _instance;

        #endregion Fields

        #region Constructors

        private HwState(string selectedMod)
        {
            ConfigHwModules(selectedMod);
        }

        #endregion Constructors

        #region Methods

        public static HwState GetInstance(string selectedMod)
        {
            if (_instance == null)
                _instance = new HwState(selectedMod);
            return _instance;
        }

        [DllImport(".\\ResourceManager.dll", EntryPoint = "GetModality", CharSet = CharSet.Unicode)]
        public static extern int GetModality(StringBuilder sb, int length);

        [DllImport(".\\ResourceManager.dll", EntryPoint = "GetMyDocumentsThorImageFolder", CharSet = CharSet.Unicode)]
        public static extern int GetMyDocumentsThorImageFolder(StringBuilder sb, int length);

        [DllImport(".\\ResourceManager.dll", EntryPoint = "SetModality", CharSet = CharSet.Unicode)]
        public static extern int SetModality(StringBuilder sb);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "GetCameraParamLong")]
        private static extern int GetCameraParamLong(int camId, int attr, ref int val);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "GetDeviceParamLong")]
        private static extern int GetDeviceParamLong(int devId, int attr, ref int val);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "SetCameraParamLong")]
        private static extern int SetCameraParamLong(int camId, int attr, int val);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "SetDeviceParamLong")]
        private static extern int SetDeviceParamLong(int devId, int attr, int val);

        public void ConfigPhase()
        {
        }

        private long ConfigHwModules(string selectedMod)
        {
            long ret = 0;

            StringBuilder sb = new StringBuilder(NAME_LENGTH);
            GetMyDocumentsThorImageFolder(sb, NAME_LENGTH);
            string modalitiesFolder = sb.ToString() + "Modalities";
            if (Directory.Exists(modalitiesFolder))
            {
                string[] mods = Directory.GetDirectories(modalitiesFolder);
                foreach (string mod in mods)
                {
                    //do not set if the camera is not LSM
                    if (!IsLSMActive(mod))
                    {
                        continue;
                    }

                    string dst = mod + "\\Active.xml";
                    if (File.Exists(dst))
                    {

                        string modName = Path.GetFileName(Path.GetDirectoryName(dst));
                        StringBuilder sbTemp = new StringBuilder(NAME_LENGTH);
                        sbTemp.Append(modName);
                        SetModality(sbTemp);

                        XmlDocument doc = new XmlDocument();
                        try
                        {
                            if (File.Exists(dst))
                            {
                                doc.Load(dst);
                            }
                            else
                            {
                                ThorLog.Instance.TraceEvent(System.Diagnostics.TraceEventType.Error,1,"Application failed to load " + dst + " file. ");
                                continue;
                            }

                            //send the settings to the camera
                            XmlNode node = doc.SelectSingleNode("/ThorImageExperiment/LSM");
                            if (node != null)
                            {
                                ConfigModalityCamAttr(node, (int)ThorSharedTypes.ICamera.Params.PARAM_LSM_HORIZONTAL_FLIP, "horizontalFlip");
                                ConfigModalityCamAttr(node, (int)ThorSharedTypes.ICamera.Params.PARAM_LSM_VERTICAL_SCAN_DIRECTION, "verticalFlip");
                            }
                        }
                        catch
                        {
                        }
                    }
                    else
                    {
                    }
                }

                string currentMod = modalitiesFolder + "\\" + selectedMod;

                //set the chosen modality
                SetModality(new StringBuilder(selectedMod));

                //set the camera again with the selected modality properties
                if (IsLSMActive(currentMod))
                {
                    XmlDocument docCurMod = new XmlDocument();
                    if (File.Exists(currentMod + "\\Active.xml"))
                    {
                        docCurMod.Load(currentMod + "\\Active.xml");
                    }
                    else
                    {
                        ThorLog.Instance.TraceEvent(System.Diagnostics.TraceEventType.Error,1,"Application failed to load " + currentMod + "\\Active.xml file. ");
                        return ret;
                    }
                    XmlNode curNode = docCurMod.SelectSingleNode("/ThorImageExperiment/LSM");
                    if (curNode != null)
                    {
                        ConfigModalityCamAttr(curNode, (int)ThorSharedTypes.ICamera.Params.PARAM_LSM_HORIZONTAL_FLIP, "horizontalFlip");
                        ConfigModalityCamAttr(curNode, (int)ThorSharedTypes.ICamera.Params.PARAM_LSM_VERTICAL_SCAN_DIRECTION, "verticalFlip");
                    }
                }

                //update the capturetemplates active.xml with the selected modality Active.xml
                XmlDocument docDtActive = new XmlDocument();
                if (File.Exists(sb.ToString() + "Capture Templates\\Active.xml"))
                {
                    try
                    {
                        File.Copy(currentMod + "\\Active.xml", sb.ToString() + "Capture Templates\\Active.xml", true);
                    }
                    catch (Exception ex)
                    {
                        string str = ex.Message;
                    }
                }
                else
                {
                    ThorLog.Instance.TraceEvent(System.Diagnostics.TraceEventType.Error, 1, "Application failed to load " + sb.ToString() + "Capture Templates\\Active.xml file. ");
                    return ret;
                }
            }
            return ret;
        }

        private int ConfigModalityCamAttr(XmlNode nodAct, int attrIdx, string attrName)
        {
            int val = 0;
            if (null != nodAct.Attributes.GetNamedItem(attrName))
            {
                val = Convert.ToInt32(nodAct.Attributes[attrName].Value);
            }

            return SetCameraParamLong((int)ThorSharedTypes.SelectedHardware.SELECTED_CAMERA1, attrIdx, val);
        }

        private int DupActiveFile(string src, string dst)
        {
            try
            {
                File.Copy(src, dst);
                XmlDocument doc = new XmlDocument();
                if (File.Exists(dst))
                {
                    doc.Load(dst);
                }
                else
                {
                    ThorLog.Instance.TraceEvent(System.Diagnostics.TraceEventType.Error, 1, "Application failed to load " + dst + " file. ");
                    return 0;
                }
                XmlNode modNode = doc.SelectSingleNode("/ThorImageExperiment/Modality");
                if (modNode != null)
                {
                    string modName = Path.GetFileName(Path.GetDirectoryName(dst));
                    modNode.Attributes["name"].Value = modName;
                    doc.Save(dst);
                }
            }
            catch (Exception ex)
            {
                string str = ex.Message;
            }
            return 0;
        }

        private bool IsLSMActive(string modPath)
        {
            XmlDocument hwSettings = new XmlDocument();
            if (File.Exists(modPath + "\\Application Settings\\HardwareSettings.xml"))
            {
                hwSettings.Load(modPath + "\\Application Settings\\HardwareSettings.xml");
            }
            else
            {
                ThorLog.Instance.TraceEvent(System.Diagnostics.TraceEventType.Error, 1, "Application failed to load " + modPath + "\\Application Settings\\HardwareSettings.xml" + " file. ");
                return false;
            }
            XmlNodeList hwNL = hwSettings.SelectNodes("/HardwareSettings/ImageDetectors/LSM");
            foreach (XmlNode n in hwNL)
            {
                if (null != n.Attributes.GetNamedItem("active"))
                {
                    if (n.Attributes["active"].Value.CompareTo("1") == 0)
                        return true;
                }
            }
            return false;
        }

        private int PersistModalityCamAttr(XmlNode nodAct, int attrIdx, string attrName)
        {
            int val = -1;
            GetCameraParamLong((int)ThorSharedTypes.SelectedHardware.SELECTED_CAMERA1, attrIdx, ref val);
            if (val > -1)
            {
                nodAct.Attributes[attrName].Value = val.ToString();
            }
            return val;
        }

        #endregion Methods
    }
}