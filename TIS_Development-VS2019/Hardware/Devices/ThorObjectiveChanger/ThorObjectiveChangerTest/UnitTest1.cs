namespace ThorObjectiveChangerTest
{
    using System;
    using System.IO;
    using System.Reflection;
    using System.Runtime.InteropServices;
    using System.Xml;

    using Microsoft.VisualStudio.TestTools.UnitTesting;

    using ThorSharedTypes;

    [TestClass]
    public class UnitTest1
    {
        #region Fields

        private const int TRUE = 1, FALSE = 0;

        int deviceCount = 0;
        double firstPos = 1.0;
        int i = 0;
        double param = 0.0;
        int paramAvailable = 0;
        double paramDefault = 0.0;
        double paramMax = 0.0;
        double paramMin = 0.0;
        int paramReadOnly = 0;
        int paramType = 0;
        double secondPos = 2.0;
        int temp = 0;

        #endregion Fields

        #region Methods

        [TestMethod]
        [DeploymentItem("..\\..\\..\\x64\\Debug\\ThorObjectiveChanger.dll", "Modules_Native")]
        public void MoveToPosition1()
        {
            try
            {
                Assert.IsTrue(File.Exists(".\\Modules_Native\\ThorObjectiveChanger.dll"));

                Assert.IsTrue(FindDevices(ref deviceCount) == TRUE);

                // For loop selects the devices that return true when select device method is invoked.
                // Right now it can only handle one, make sure to changes this if you want it to handle more devices
                for (i = 0; i < deviceCount; i++)
                {
                    if (TRUE == SelectDevice(i))
                    {
                        temp = i;
                    }
                }

                Assert.IsTrue(SelectDevice(temp) == TRUE);
                Assert.IsTrue(GetParamInfo((int)IDevice.Params.PARAM_DEVICE_TYPE, ref paramType, ref paramAvailable, ref paramReadOnly, ref paramMin, ref paramMax, ref paramDefault) == TRUE);

                int targetDevType = (int)(DeviceType.TURRET);
                if (((int)(paramDefault) & targetDevType) != targetDevType)
                {

                }
                else
                {
                    Assert.IsTrue(GetParamInfo((int)IDevice.Params.PARAM_TURRET_POS, ref paramType, ref paramAvailable, ref paramReadOnly, ref paramMin, ref paramMax, ref paramDefault) == TRUE);
                    if (TRUE == paramAvailable)
                    {
                        int status;
                        // Move to position 1:
                        Assert.IsTrue(SetParam((int)IDevice.Params.PARAM_TURRET_POS, firstPos) == TRUE);

                        Assert.IsTrue(PreflightPosition() == TRUE);
                        Assert.IsTrue(SetupPosition() == TRUE);
                        Assert.IsTrue(StartPosition() == TRUE);

                        status = 0;
                        do
                        {
                            Assert.IsTrue(StatusPosition(ref status) == TRUE);
                        }
                        while (0 == status);

                        Assert.IsTrue(PostflightPosition() == TRUE);
                        // Confirm position 1 is correct:
                        Assert.IsTrue(GetParam((int)IDevice.Params.PARAM_TURRET_POS, ref param) == TRUE);
                        Assert.IsTrue(param == firstPos);

                        Assert.IsTrue(TeardownDevice() == TRUE);
                    }
                }
            }
            catch (AssertFailedException e)
            {
                TeardownDevice();
                Assert.Fail("Test Failed", e);
            }
        }

        [TestMethod]
        [DeploymentItem("..\\..\\..\\x64\\Debug\\ThorObjectiveChanger.dll", "Modules_Native")]
        public void MoveToPosition2()
        {
            try
            {
                Assert.IsTrue(File.Exists(".\\Modules_Native\\ThorObjectiveChanger.dll"));

                Assert.IsTrue(FindDevices(ref deviceCount) == TRUE);

                // For loop selects the devices that return true when select device method is invoked.
                // Right now it can only handle one, make sure to changes this if you want it to handle more devices
                for (i = 0; i < deviceCount; i++)
                {
                    if (TRUE == SelectDevice(i))
                    {
                        temp = i;
                    }
                }

                Assert.IsTrue(SelectDevice(temp) == TRUE);
                Assert.IsTrue(GetParamInfo((int)IDevice.Params.PARAM_DEVICE_TYPE, ref paramType, ref paramAvailable, ref paramReadOnly, ref paramMin, ref paramMax, ref paramDefault) == TRUE);

                int targetDevType = (int)(DeviceType.TURRET);
                if (((int)(paramDefault) & targetDevType) != targetDevType)
                {

                }
                else
                {
                    Assert.IsTrue(GetParamInfo((int)IDevice.Params.PARAM_TURRET_POS, ref paramType, ref paramAvailable, ref paramReadOnly, ref paramMin, ref paramMax, ref paramDefault) == TRUE);
                    if (TRUE == paramAvailable)
                    {
                        int status;
                        // Move to position 2:
                        Assert.IsTrue(SetParam((int)IDevice.Params.PARAM_TURRET_POS, secondPos) == TRUE);

                        Assert.IsTrue(PreflightPosition() == TRUE);
                        Assert.IsTrue(SetupPosition() == TRUE);
                        Assert.IsTrue(StartPosition() == TRUE);

                        status = 0;
                        do
                        {
                            Assert.IsTrue(StatusPosition(ref status) == TRUE);
                        }
                        while (0 == status);

                        Assert.IsTrue(PostflightPosition() == TRUE);
                        // Confirm position 2 is correct:
                        Assert.IsTrue(GetParam((int)IDevice.Params.PARAM_TURRET_POS, ref param) == TRUE);
                        Assert.IsTrue(param == secondPos);

                        Assert.IsTrue(TeardownDevice() == TRUE);
                    }
                }
            }
            catch (AssertFailedException e)
            {
                TeardownDevice();
                Assert.Fail("Test Failed", e);
            }
        }

        [TestMethod]
        [DeploymentItem("..\\..\\..\\x64\\Debug\\ThorObjectiveChanger.dll", "Modules_Native")]
        public void MoveToPositionMax()
        {
            try
            {
                Assert.IsTrue(File.Exists(".\\Modules_Native\\ThorObjectiveChanger.dll"));

                Assert.IsTrue(FindDevices(ref deviceCount) == TRUE);

                // For loop selects the devices that return true when select device method is invoked.
                // Right now it can only handle one, make sure to changes this if you want it to handle more devices
                for (i = 0; i < deviceCount; i++)
                {
                    if (TRUE == SelectDevice(i))
                    {
                        temp = i;
                    }
                }

                Assert.IsTrue(SelectDevice(temp) == TRUE);
                Assert.IsTrue(GetParamInfo((int)IDevice.Params.PARAM_DEVICE_TYPE, ref paramType, ref paramAvailable, ref paramReadOnly, ref paramMin, ref paramMax, ref paramDefault) == TRUE);

                int targetDevType = (int)(DeviceType.TURRET);
                if (((int)(paramDefault) & targetDevType) != targetDevType)
                {

                }
                else
                {
                    Assert.IsTrue(GetParamInfo((int)IDevice.Params.PARAM_TURRET_POS, ref paramType, ref paramAvailable, ref paramReadOnly, ref paramMin, ref paramMax, ref paramDefault) == TRUE);
                    if (TRUE == paramAvailable)
                    {
                        int status;

                        // Move to Max position:
                        Assert.IsTrue(SetParam((int)IDevice.Params.PARAM_TURRET_POS, paramMax) == TRUE);

                        Assert.IsTrue(PreflightPosition() == TRUE);
                        Assert.IsTrue(SetupPosition() == TRUE);
                        Assert.IsTrue(StartPosition() == TRUE);

                        status = 0;
                        do
                        {
                            Assert.IsTrue(StatusPosition(ref status) == TRUE);
                        }

                        while (0 == status);

                        Assert.IsTrue(PostflightPosition() == TRUE);

                        // Confirm Max position is correct:

                        Assert.IsTrue(GetParam((int)IDevice.Params.PARAM_TURRET_POS, ref param) == TRUE);
                        Assert.IsTrue(param == paramMax);

                        Assert.IsTrue(TeardownDevice() == TRUE);
                    }

                }
            }
            catch (AssertFailedException e)
            {
                TeardownDevice();
                Assert.Fail("Test Failed", e);
            }
        }

        [TestMethod]
        [DeploymentItem("..\\..\\..\\x64\\Debug\\ThorObjectiveChanger.dll", "Modules_Native")]
        public void MoveToPostionDefault()
        {
            try
            {
                Assert.IsTrue(File.Exists(".\\Modules_Native\\ThorObjectiveChanger.dll"));

                Assert.IsTrue(FindDevices(ref deviceCount) == TRUE);

                // For loop selects the devices that return true when select device method is invoked.
                // Right now it can only handle one, make sure to changes this if you want it to handle more devices
                for (i = 0; i < deviceCount; i++)
                {
                    if (TRUE == SelectDevice(i))
                    {
                        temp = i;
                    }
                }

                Assert.IsTrue(SelectDevice(temp) == TRUE, "Test Failed", TeardownDevice());
                Assert.IsTrue(GetParamInfo((int)IDevice.Params.PARAM_DEVICE_TYPE, ref paramType, ref paramAvailable, ref paramReadOnly, ref paramMin, ref paramMax, ref paramDefault) == TRUE);

                int targetDevType = (int)(DeviceType.TURRET);
                if (((int)(paramDefault) & targetDevType) != targetDevType)
                {

                }
                else
                {
                    Assert.IsTrue(GetParamInfo((int)IDevice.Params.PARAM_TURRET_POS, ref paramType, ref paramAvailable, ref paramReadOnly, ref paramMin, ref paramMax, ref paramDefault) == TRUE);
                    if (TRUE == paramAvailable)
                    {
                        int status;

                        // Move to Default position:
                        Assert.IsTrue(SetParam((int)IDevice.Params.PARAM_TURRET_POS, paramDefault) == TRUE);

                        Assert.IsTrue(PreflightPosition() == TRUE);
                        Assert.IsTrue(SetupPosition() == TRUE);
                        Assert.IsTrue(StartPosition() == TRUE);

                        status = 0;
                        do
                        {
                            Assert.IsTrue(StatusPosition(ref status) == TRUE);
                        }
                        while (0 == status);

                        Assert.IsTrue(PostflightPosition() == TRUE);

                        // Confirm Default position is correct:
                        Assert.IsTrue(GetParam((int)IDevice.Params.PARAM_TURRET_POS, ref param) == TRUE);
                        Assert.IsTrue(param == paramDefault);

                        Assert.IsTrue(TeardownDevice() == TRUE);
                    }
                }
            }
            catch (AssertFailedException e)
            {
                TeardownDevice();
                Assert.Fail("Test Failed", e);
            }
        }

        [TestMethod]
        [DeploymentItem("..\\..\\..\\x64\\Debug\\ThorObjectiveChanger.dll", "Modules_Native")]
        public void MoveToPostionMin()
        {
            try
            {
            Assert.IsTrue(File.Exists(".\\Modules_Native\\ThorObjectiveChanger.dll"));

            Assert.IsTrue(FindDevices(ref deviceCount) == TRUE);

            // For loop selects the devices that return true when select device method is invoked.
            // Right now it can only handle one, make sure to changes this if you want it to handle more devices
            for (i = 0; i < deviceCount; i++)
            {
                if (TRUE == SelectDevice(i))
                {
                    temp = i;
                }
            }

            Assert.IsTrue(SelectDevice(temp) == TRUE);
            Assert.IsTrue(GetParamInfo((int)IDevice.Params.PARAM_DEVICE_TYPE, ref paramType, ref paramAvailable, ref paramReadOnly, ref paramMin, ref paramMax, ref paramDefault) == TRUE);

            int targetDevType = (int)(DeviceType.TURRET);
            if (((int)(paramDefault) & targetDevType) != targetDevType)
            {

            }
            else
            {
                Assert.IsTrue(GetParamInfo((int)IDevice.Params.PARAM_TURRET_POS, ref paramType, ref paramAvailable, ref paramReadOnly, ref paramMin, ref paramMax, ref paramDefault) == TRUE);
                if (TRUE == paramAvailable)
                {
                    int status;

                    // Move to Min position:
                    Assert.IsTrue(SetParam((int)IDevice.Params.PARAM_TURRET_POS, paramMin) == TRUE);

                    Assert.IsTrue(PreflightPosition() == TRUE);
                    Assert.IsTrue(SetupPosition() == TRUE);
                    Assert.IsTrue(StartPosition() == TRUE);

                    status = 0;
                    do
                    {
                        Assert.IsTrue(StatusPosition(ref status) == TRUE);
                    }
                    while (0 == status);

                    Assert.IsTrue(PostflightPosition() == TRUE);

                    // Confirm Min position is correct:
                    Assert.IsTrue(GetParam((int)IDevice.Params.PARAM_TURRET_POS, ref param) == TRUE);
                    Assert.IsTrue(param == paramMin);

                    Assert.IsTrue(TeardownDevice() == TRUE);
                }
            }
            }
            catch (AssertFailedException e)
            {
                TeardownDevice();
                Assert.Fail("Test Failed", e);
            }
        }

        [DllImport(".\\Modules_Native\\ThorObjectiveChanger.dll", EntryPoint = "FindDevices", CallingConvention = CallingConvention.Cdecl)]
        private static extern int FindDevices(ref int deviceCount);

        [DllImport(".\\Modules_Native\\ThorObjectiveChanger.dll", EntryPoint = "GetParam", CallingConvention = CallingConvention.Cdecl)]
        private static extern int GetParam(int paramID, ref double param);

        [DllImport(".\\Modules_Native\\ThorObjectiveChanger.dll", EntryPoint = "GetParamInfo", CallingConvention = CallingConvention.Cdecl)]
        private static extern int GetParamInfo(int paramID, ref int paramType, ref int paramAvailable, ref int paramReadOnly, ref double paramMin, ref double paramMax, ref double paramDefault);

        [DllImport(".\\Modules_Native\\ThorObjectiveChanger.dll", EntryPoint = "PostflightPosition", CallingConvention = CallingConvention.Cdecl)]
        private static extern int PostflightPosition();

        [DllImport(".\\Modules_Native\\ThorObjectiveChanger.dll", EntryPoint = "PreflightPosition", CallingConvention = CallingConvention.Cdecl)]
        private static extern int PreflightPosition();

        [DllImport(".\\Modules_Native\\ThorObjectiveChanger.dll", EntryPoint = "SelectDevice", CallingConvention = CallingConvention.Cdecl)]
        private static extern int SelectDevice(int device);

        [DllImport(".\\Modules_Native\\ThorObjectiveChanger.dll", EntryPoint = "SetParam", CallingConvention = CallingConvention.Cdecl)]
        private static extern int SetParam(int paramID, double param);

        [DllImport(".\\Modules_Native\\ThorObjectiveChanger.dll", EntryPoint = "SetupPosition", CallingConvention = CallingConvention.Cdecl)]
        private static extern int SetupPosition();

        [DllImport(".\\Modules_Native\\ThorObjectiveChanger.dll", EntryPoint = "StartPosition", CallingConvention = CallingConvention.Cdecl)]
        private static extern int StartPosition();

        [DllImport(".\\Modules_Native\\ThorObjectiveChanger.dll", EntryPoint = "StatusPosition", CallingConvention = CallingConvention.Cdecl)]
        private static extern int StatusPosition(ref int status);

        [DllImport(".\\Modules_Native\\ThorObjectiveChanger.dll", EntryPoint = "TeardownDevice", CallingConvention = CallingConvention.Cdecl)]
        private static extern int TeardownDevice();

        #endregion Methods
    }
}