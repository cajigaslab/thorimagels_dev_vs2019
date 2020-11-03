using System;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;
//using System.Xml;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MaiTaiLaserTest
{
    [TestClass]
    public class UnitTest1
    {
        #region Fields
        private const int TRUE = 1, FALSE = 0;
        #endregion Fields
        #region Methods

        [TestMethod]
        [DeploymentItem("..\\..\\..\\x64\\Debug\\MaiTaiLaser.dll", "Modules_Native")]
        [DeploymentItem("..\\..\\..\\MaiTaiLaserSettings.xml")]
        public void TestMethod1()
        {
            Assert.IsTrue(File.Exists("MaiTaiLaserSettings.xml"));
            Assert.IsTrue(File.Exists(".\\Modules_Native\\MaiTaiLaser.dll"));
            int dev = 0;
            double wav = 0.0;
            double laser1Enabled = 0;
            double shutterStatus = 0;
            Assert.IsTrue(FindDevices(ref dev) == TRUE);
            Assert.IsTrue(SelectDevice(0) == TRUE);
            int status = 0;

            Assert.IsTrue(GetParam(825, ref laser1Enabled) == TRUE);
            Assert.IsTrue(SetParam(825, 1) == TRUE);
            Assert.IsTrue(SetupPosition() == TRUE);
            Assert.IsTrue(StartPosition() == TRUE);
            while (status == 0)
            {
                Assert.IsTrue(StatusPosition(ref status) == TRUE);
            }
            status = 0;
            Assert.IsTrue(GetParam(825, ref laser1Enabled) == TRUE);
            

            Assert.IsTrue(GetParam(845, ref shutterStatus) == TRUE);
            Assert.IsTrue(SetParam(845, 1) == TRUE);
            Assert.IsTrue(SetupPosition() == TRUE);
            Assert.IsTrue(StartPosition() == TRUE);
            while (status == 0)
            {
                Assert.IsTrue(StatusPosition(ref status) == TRUE);
            }
            status = 0;
            Assert.IsTrue(GetParam(845, ref shutterStatus) == TRUE);

            Assert.IsTrue(GetParam(826, ref wav) == TRUE);
            Assert.IsTrue(SetParam(826, 856) == TRUE);
            Assert.IsTrue(SetupPosition() == TRUE);
            Assert.IsTrue(StartPosition() == TRUE);
            while (status == 0)
            {
                Assert.IsTrue(StatusPosition(ref status) == TRUE);
            }
            status = 0;
            Assert.IsTrue(GetParam(826, ref wav) == TRUE);
           
            Assert.IsTrue(SetParam(826, 1200) == FALSE);
            Assert.IsTrue(SetupPosition() == FALSE);
            Assert.IsTrue(StartPosition() == FALSE);
            while (status == 0)
            {
                Assert.IsTrue(StatusPosition(ref status) == TRUE);
            }
            status = 0;
            Assert.IsTrue(GetParam(826, ref wav) == TRUE);

            Assert.IsTrue(SetParam(826, 900) == TRUE);
            Assert.IsTrue(SetupPosition() == TRUE);
            Assert.IsTrue(StartPosition() == TRUE);
            while (status == 0)
            {
                Assert.IsTrue(StatusPosition(ref status) == TRUE);
            }
            status = 0;
            Assert.IsTrue(GetParam(826, ref wav) == TRUE);

            Assert.IsTrue(SetParam(826, 0) == FALSE);
            Assert.IsTrue(SetupPosition() == FALSE);
            Assert.IsTrue(StartPosition() == FALSE);
            while (status == 0)
            {
                Assert.IsTrue(StatusPosition(ref status) == TRUE);
            }
            status = 0;
            Assert.IsTrue(GetParam(826, ref wav) == TRUE);

            Assert.IsTrue(GetParam(845, ref shutterStatus) == TRUE);
            Assert.IsTrue(SetParam(845, 0) == TRUE);
            Assert.IsTrue(SetupPosition() == TRUE);
            Assert.IsTrue(StartPosition() == TRUE);
            while (status == 0)
            {
                Assert.IsTrue(StatusPosition(ref status) == TRUE);
            }
            status = 0;
            Assert.IsTrue(GetParam(845, ref wav) == TRUE);

            Assert.IsTrue(GetParam(825, ref laser1Enabled) == TRUE);
            Assert.IsTrue(SetParam(825, 0) == TRUE);
            Assert.IsTrue(SetupPosition() == TRUE);
            Assert.IsTrue(StartPosition() == TRUE);
            while (status == 0)
            {
                Assert.IsTrue(StatusPosition(ref status) == TRUE);
            }
            status = 0;
            Assert.IsTrue(GetParam(825, ref laser1Enabled) == TRUE);

            Assert.IsTrue(TeardownDevice() == TRUE);
        }

        [DllImport(".\\Modules_Native\\MaiTaiLaser.dll", EntryPoint = "FindDevices", CallingConvention = CallingConvention.Cdecl)]
        private static extern int FindDevices(ref int Device);

        [DllImport(".\\Modules_Native\\MaiTaiLaser.dll", EntryPoint = "GetParam", CallingConvention = CallingConvention.Cdecl)]
        private static extern int GetParam(int paramID, ref double param);

        [DllImport(".\\Modules_Native\\MaiTaiLaser.dll", EntryPoint = "GetParamInfo", CallingConvention = CallingConvention.Cdecl)]
        private static extern int GetParamInfo(int paramID, ref int paramType, ref int paramAvailable, ref int paramReadOnly, ref double paramMin, ref double paramMax, ref double paramDefault);

        [DllImport(".\\Modules_Native\\MaiTaiLaser.dll", EntryPoint = "PostflightPosition", CallingConvention = CallingConvention.Cdecl)]
        private static extern int PostflightPosition();

        [DllImport(".\\Modules_Native\\MaiTaiLaser.dll", EntryPoint = "PreflightPosition", CallingConvention = CallingConvention.Cdecl)]
        private static extern int PreflightPosition();

        [DllImport(".\\Modules_Native\\MaiTaiLaser.dll", EntryPoint = "SelectDevice", CallingConvention = CallingConvention.Cdecl)]
        private static extern int SelectDevice(int device);

        [DllImport(".\\Modules_Native\\MaiTaiLaser.dll", EntryPoint = "SetParam", CallingConvention = CallingConvention.Cdecl)]
        private static extern int SetParam(int paramID, double param);

        [DllImport(".\\Modules_Native\\MaiTaiLaser.dll", EntryPoint = "SetupPosition", CallingConvention = CallingConvention.Cdecl)]
        private static extern int SetupPosition();

        [DllImport(".\\Modules_Native\\MaiTaiLaser.dll", EntryPoint = "StartPosition", CallingConvention = CallingConvention.Cdecl)]
        private static extern int StartPosition();

        [DllImport(".\\Modules_Native\\MaiTaiLaser.dll", EntryPoint = "StatusPosition", CallingConvention = CallingConvention.Cdecl)]
        private static extern int StatusPosition(ref int status);

        [DllImport(".\\Modules_Native\\MaiTaiLaser.dll", EntryPoint = "TeardownDevice", CallingConvention = CallingConvention.Cdecl)]
        private static extern int TeardownDevice();
        #endregion Methods
    }
}