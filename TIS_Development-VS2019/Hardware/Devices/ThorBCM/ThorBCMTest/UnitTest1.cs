
namespace ThorBCMTest
{
    using System;
    using System.IO;
    using System.Reflection;
    using System.Runtime.InteropServices;
    using System.Xml;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    [TestClass]
    public class UnitTest1
    {
        #region Fields

        private const int PARAM_DEVICE_TYPE = 0;

        private const int LIGHT_PATH = 0x2000000;
        private const int PARAM_LIGHTPATH_GG = 1600;
        private const int PARAM_LIGHTPATH_GR = 1601;
        private const int PARAM_LIGHTPATH_CAMERA = 1602;

        private const int TRUE = 1, FALSE = 0;

        #endregion Fields

        #region Methods

        [TestMethod]
        [DeploymentItem("..\\..\\..\\x64\\Debug\\ThorBCM.dll", "Modules_Native")]
        [DeploymentItem("..\\..\\..\\ThorBCMSettings.xml")]
        public void Test_BCM_GG()
        {
            Assert.IsTrue(File.Exists("ThorBCMSettings.xml"));
            Assert.IsTrue(File.Exists(".\\Modules_Native\\ThorBCM.dll"));
            int deviceCount = 0;
            Assert.IsTrue(FindDevices(ref deviceCount) == TRUE);

            int paramType = 0;
            int paramAvailable = 0;
            int paramReadOnly = 0;
            double paramMin = 0.0;
            double paramMax = 0.0;
            double paramDefault = 0.0;

            Assert.IsTrue(SelectDevice(0) == TRUE);
            Assert.IsTrue(GetParamInfo(PARAM_DEVICE_TYPE, ref paramType, ref paramAvailable, ref paramReadOnly, ref paramMin, ref paramMax, ref paramDefault) == TRUE);

            if ((int)(paramDefault) == LIGHT_PATH)
            {
                Assert.IsTrue(GetParamInfo(PARAM_LIGHTPATH_GG, ref paramType, ref paramAvailable, ref paramReadOnly, ref paramMin, ref paramMax, ref paramDefault) == TRUE);
                if (paramAvailable == TRUE)
                {

                    // Move to 0  position:
                    Assert.IsTrue(SetParam(PARAM_LIGHTPATH_GG, 0) == TRUE);

                    Assert.IsTrue(StartPosition() == TRUE);

                    // Move to 1  position:
                    Assert.IsTrue(SetParam(PARAM_LIGHTPATH_GG, 1) == TRUE);

                    Assert.IsTrue(StartPosition() == TRUE);



                }
            }
            Assert.IsTrue(TeardownDevice() == TRUE);
        }

        [TestMethod]
        [DeploymentItem("..\\..\\..\\x64\\Debug\\ThorBCM.dll", "Modules_Native")]
        [DeploymentItem("..\\..\\..\\ThorBCMSettings.xml")]
        public void Test_BCM_GR()
        {
            Assert.IsTrue(File.Exists("ThorBCMSettings.xml"));
            Assert.IsTrue(File.Exists(".\\Modules_Native\\ThorBCM.dll"));
            int deviceCount = 0;
            Assert.IsTrue(FindDevices(ref deviceCount) == TRUE);

            int paramType = 0;
            int paramAvailable = 0;
            int paramReadOnly = 0;
            double paramMin = 0.0;
            double paramMax = 0.0;
            double paramDefault = 0.0;

            Assert.IsTrue(SelectDevice(0) == TRUE);
            Assert.IsTrue(GetParamInfo(PARAM_DEVICE_TYPE, ref paramType, ref paramAvailable, ref paramReadOnly, ref paramMin, ref paramMax, ref paramDefault) == TRUE);

            if ((int)(paramDefault) == LIGHT_PATH)
            {

                Assert.IsTrue(GetParamInfo(PARAM_LIGHTPATH_GR, ref paramType, ref paramAvailable, ref paramReadOnly, ref paramMin, ref paramMax, ref paramDefault) == TRUE);
                if (paramAvailable == TRUE)
                {

                    // Move to 0  position:
                    Assert.IsTrue(SetParam(PARAM_LIGHTPATH_GR, 0) == TRUE);

                    Assert.IsTrue(StartPosition() == TRUE);

                    // Move to 1  position:
                    Assert.IsTrue(SetParam(PARAM_LIGHTPATH_GR, 1) == TRUE);

                    Assert.IsTrue(StartPosition() == TRUE);



                }
            }
            Assert.IsTrue(TeardownDevice() == TRUE);
        }

        [TestMethod]
        [DeploymentItem("..\\..\\..\\x64\\Debug\\ThorBCM.dll", "Modules_Native")]
        [DeploymentItem("..\\..\\..\\ThorBCMSettings.xml")]
        public void Test_BCM_CAM()
        {
            Assert.IsTrue(File.Exists("ThorBCMSettings.xml"));
            Assert.IsTrue(File.Exists(".\\Modules_Native\\ThorBCM.dll"));
            int deviceCount = 0;
            Assert.IsTrue(FindDevices(ref deviceCount) == TRUE);

            int paramType = 0;
            int paramAvailable = 0;
            int paramReadOnly = 0;
            double paramMin = 0.0;
            double paramMax = 0.0;
            double paramDefault = 0.0;

            Assert.IsTrue(SelectDevice(0) == TRUE);
            Assert.IsTrue(GetParamInfo(PARAM_DEVICE_TYPE, ref paramType, ref paramAvailable, ref paramReadOnly, ref paramMin, ref paramMax, ref paramDefault) == TRUE);

            if ((int)(paramDefault) == LIGHT_PATH)
            {
                Assert.IsTrue(GetParamInfo(PARAM_LIGHTPATH_CAMERA, ref paramType, ref paramAvailable, ref paramReadOnly, ref paramMin, ref paramMax, ref paramDefault) == TRUE);
                if (paramAvailable == TRUE)
                {

                    // Move to 0  position:
                    Assert.IsTrue(SetParam(PARAM_LIGHTPATH_CAMERA, 0) == TRUE);

                    Assert.IsTrue(StartPosition() == TRUE);

                    // Move to 1  position:
                    Assert.IsTrue(SetParam(PARAM_LIGHTPATH_CAMERA, 1) == TRUE);

                    Assert.IsTrue(StartPosition() == TRUE);


                }
            }
            Assert.IsTrue(TeardownDevice() == TRUE);
        }

        [DllImport(".\\Modules_Native\\ThorBCM.dll", EntryPoint = "GetParam", CallingConvention = CallingConvention.Cdecl)]
        private static extern int GetParam(int paramID, ref double param);

        [DllImport(".\\Modules_Native\\ThorBCM.dll", EntryPoint = "GetParamInfo", CallingConvention = CallingConvention.Cdecl)]
        private static extern int GetParamInfo(int paramID, ref int paramType, ref int paramAvailable, ref int paramReadOnly, ref double paramMin, ref double paramMax, ref double paramDefault);

        [DllImport(".\\Modules_Native\\ThorBCM.dll", EntryPoint = "TeardownDevice", CallingConvention = CallingConvention.Cdecl)]
        private static extern int TeardownDevice();

        [DllImport(".\\Modules_Native\\ThorBCM.dll", EntryPoint = "SelectDevice", CallingConvention = CallingConvention.Cdecl)]
        private static extern int SelectDevice(int device);

        [DllImport(".\\Modules_Native\\ThorBCM.dll", EntryPoint = "FindDevices", CallingConvention = CallingConvention.Cdecl)]
        private static extern int FindDevices(ref int deviceCount);

        [DllImport(".\\Modules_Native\\ThorBCM.dll", EntryPoint = "SetParam", CallingConvention = CallingConvention.Cdecl)]
        private static extern int SetParam(int paramID, double param);

        [DllImport(".\\Modules_Native\\ThorBCM.dll", EntryPoint = "StartPosition", CallingConvention = CallingConvention.Cdecl)]
        private static extern int StartPosition();

        #endregion Methods
    }
}
