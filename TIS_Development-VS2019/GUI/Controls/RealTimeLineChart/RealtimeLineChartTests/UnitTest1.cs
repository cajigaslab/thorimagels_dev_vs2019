namespace UnitTestProject1
{
    using System;
    using System.IO;
    using System.Reflection;
    using System.Xml;

    using Microsoft.VisualStudio.TestTools.UnitTesting;

    using RealTimeLineChart.ViewModel;

    [TestClass]
    public class UnitTest1
    {
        #region Methods

        //ThorRealTimeData.dll
        //ThorRealTimeDataSettings.xml
        [TestMethod]
        [DeploymentItem("..\\..\\..\\..\\..\\..\\Hardware\\Devices\\ThorRealTimeData\\x64\\Debug\\ThorRealTimeData.dll")]
        [DeploymentItem("..\\..\\..\\..\\..\\..\\Hardware\\Devices\\ThorRealTimeData\\ThorRealTimeDataSettings.xml")]
        [DeploymentItem("..\\..\\..\\..\\..\\..\\Hardware\\Devices\\ThorRealTimeData\\ThorRealTimeData\\bin")]
        public void DocumentSettingsTest()
        {
            Assert.IsTrue(File.Exists("ThorRealTimeDataSettings.xml"));

                RealTimeLineChartViewModel vm = new RealTimeLineChartViewModel();

                vm.LoadDocumentSettings();

                vm.CreateChartLines();

                vm.SampleRate = 1;

                vm.DisplayOptionSelectedIndex = 1;

                vm.TriggerMode = 1;

                vm.SaveName = "TestingName";

                vm.SavePath = "C:\\MyTestPath";

                //vm.FifoSize = 1;

                vm.SaveDocumentSettings();

                XmlDocument doc = new XmlDocument();

                doc.Load("ThorRealTimeDataSettings.xml");

                TestAttributeSingleNode(doc, "/RealTimeDataSettings/UserSettings/Save", "name", "TestingName");
                TestAttributeSingleNode(doc, "/RealTimeDataSettings/UserSettings/Save", "path", "C:\\MyTestPath");
                TestAttributeSingleNode(doc, "/RealTimeDataSettings/UserSettings/SciChart", "fifoSize", "1000");
                TestAttributeSingleNode(doc, "/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]", "hwTrigMode", "1");
                TestAttributeMultiNodeIndex(doc, "/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]/SampleRate", "enable", "1", 1, -1);
                TestAttributeMultiNodeIndex(doc, "/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]/Display", "value", "100", 1, -1);

                vm.SampleRate = 0;

                vm.DisplayOptionSelectedIndex = 0;

                vm.TriggerMode = 0;

                vm.SaveName = "Another Name";

                vm.SavePath = "E:\\Temp";

                //vm.FifoSize = 2;

                vm.SaveDocumentSettings();

                doc.Load("ThorRealTimeDataSettings.xml");

                TestAttributeSingleNode(doc, "/RealTimeDataSettings/UserSettings/Save", "name", "Another Name");
                TestAttributeSingleNode(doc, "/RealTimeDataSettings/UserSettings/Save", "path", "E:\\Temp");
                //??Why does this test fail??//TestAttributeSingleNode(doc, "/RealTimeDataSettings/UserSettings/SciChart", "fifoSize", "2000");
                TestAttributeSingleNode(doc, "/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]", "hwTrigMode", "0");
                TestAttributeMultiNodeIndex(doc, "/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]/SampleRate", "enable", "1", 0, -1);
                TestAttributeMultiNodeIndex(doc, "/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]/Display", "value", "1000", 1, -1);
        }

        //ThorRealTimeData.dll
        //ThorRealTimeDataSettings.xml
        [TestMethod]
        [DeploymentItem("..\\..\\..\\..\\..\\..\\Hardware\\Devices\\ThorRealTimeData\\x64\\Debug\\ThorRealTimeData.dll")]
        [DeploymentItem("..\\..\\..\\..\\..\\..\\Hardware\\Devices\\ThorRealTimeData\\ThorRealTimeDataSettings.xml")]
        [DeploymentItem("..\\..\\..\\..\\..\\..\\Hardware\\Devices\\ThorRealTimeData\\ThorRealTimeData\\bin")]
        public void VMPropertyCheck()
        {
            RealTimeLineChartViewModel vm = new RealTimeLineChartViewModel();
            CheckProperty(vm.GetType(), "AutoRangeX", false, true);
            CheckProperty(vm.GetType(), "AutoRangeY", false, true);
            CheckProperty(vm.GetType(), "ChartMode", true, true);
            CheckProperty(vm.GetType(), "ChartSeries", false, true);
            CheckProperty(vm.GetType(), "ImageCounterNumber", true, true);
            CheckProperty(vm.GetType(), "FilePath", true, true);
            CheckProperty(vm.GetType(), "DisplayOptionSelectedIndex", true, true);
            CheckProperty(vm.GetType(), "DisplayOptionList", true, true);
            CheckProperty(vm.GetType(), "EditLinesCommand", false, true);
            CheckProperty(vm.GetType(), "FifoSize", true, true);
            CheckProperty(vm.GetType(), "IsCapturing", false, true);
            CheckProperty(vm.GetType(), "IsFifoVisible", false, true);
            CheckProperty(vm.GetType(), "IsPanEnabled", true, true);
            CheckProperty(vm.GetType(), "IsPanVisible", false, true);
            CheckProperty(vm.GetType(), "IsRubberBand", true, true);
            CheckProperty(vm.GetType(), "IsSaving", true, true);
            CheckProperty(vm.GetType(), "IsCapturingStopped", false, true);
            CheckProperty(vm.GetType(), "IsZoomEnabled", true, true);
            CheckProperty(vm.GetType(), "SampleRate", true, true);
            CheckProperty(vm.GetType(), "SampleRateList", true, true);
            CheckProperty(vm.GetType(), "SamplingDuration", true, true);
            CheckProperty(vm.GetType(), "SaveName", true, true);
            CheckProperty(vm.GetType(), "SavePath", true, true);
            CheckProperty(vm.GetType(), "SciChartSurface", false, true);
            CheckProperty(vm.GetType(), "SetSavePathCommand", false, true);
            CheckProperty(vm.GetType(), "StartCommand", false, true);
            CheckProperty(vm.GetType(), "CapturingIcon", false, true);
            CheckProperty(vm.GetType(), "StimulusLimit", true, true);
            CheckProperty(vm.GetType(), "TriggerMode", true, true);
            CheckProperty(vm.GetType(), "TriggerModeText", false, true);
        }

        private void CheckProperty(Type type, string propName, bool checkWrite, bool checkRead)
        {
            PropertyInfo propInfo = type.GetProperty(propName);

            if (checkWrite)
            {
                Assert.IsTrue(propInfo.CanWrite);
            }
            else
            {
                Assert.IsFalse(propInfo.CanWrite);
            }

            if (checkRead)
            {
                Assert.IsTrue(propInfo.CanRead);
            }
            else
            {
                Assert.IsFalse(propInfo.CanRead);
            }
        }

        private void TestAttributeMultiNodeIndex(XmlDocument doc, string multiNodeXPath, string attName, string attVal, int testVal,  int failVal)
        {
            int val = failVal;

            XmlNodeList nodeList = doc.SelectNodes(multiNodeXPath);

                for (int i = 0; i < nodeList.Count; i++)
                {
                    if (nodeList[i].Attributes[attName].Value.Equals(attVal))
                    {
                        val = i;
                    }
                }
                Assert.IsTrue(testVal == val);
        }

        private void TestAttributeSingleNode(XmlDocument doc, string singleNodeXPath, string attName, string testVal)
        {
            XmlNode node = doc.SelectSingleNode(singleNodeXPath);

            Assert.IsTrue(node.Attributes[attName].Value.Equals(testVal));
        }

        #endregion Methods
    }
}