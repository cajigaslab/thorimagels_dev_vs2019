namespace ThorPLSZTest
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
        private const int PARAM_Z_ACCEL = 405;
        private const int PARAM_Z_DECEL = 406;
        private const int PARAM_Z_HOME = 401;
        private const int PARAM_Z_JOYSTICK_VELOCITY = 408;
        private const int PARAM_Z_POS = 400;
        private const int PARAM_Z_POS_CURRENT = 407;
        private const int PARAM_Z_STEPS_PER_MM = 404;
        private const int PARAM_Z_STOP = 409;
        private const int PARAM_Z_VELOCITY = 403;
        private const int PARAM_Z_VELOCITY_CURRENT = 410;
        private const int PARAM_Z_ZERO = 402;
        private const int STAGE_Z = 0x00000010;
        private const int TRUE = 1, FALSE = 0;

        #endregion Fields

        #region Methods


        [TestMethod]
        [DeploymentItem("..\\..\\..\\x64\\Debug\\ThorPLSZ.dll", "Modules_Native")]
        [DeploymentItem("..\\..\\..\\ThorPLSZSettings.xml")]
        public void Test_Z_Stage()
        {
            Assert.IsTrue(File.Exists("ThorPLSZSettings.xml"));

            Assert.IsTrue(File.Exists(".\\Modules_Native\\ThorPLSZ.dll"));

            // Assert.IsTrue(StartPosition() == TRUE);

            int paramType = 0;
            int paramAvailable = 0;
            int paramReadOnly = 0;
            double paramMin = 0.0;
            double paramMax = 0.0;
            double paramDefault = 0.0;
            double param = 0.0;

            Assert.IsTrue(SelectDevice(0) == TRUE);
            Assert.IsTrue(GetParamInfo(PARAM_DEVICE_TYPE, ref paramType, ref paramAvailable, ref paramReadOnly, ref paramMin, ref paramMax, ref paramDefault) == TRUE);

            int targetDevType = (int)(STAGE_Z);

            if (((int)(paramDefault) & targetDevType) != targetDevType)
            {

            }
            else
            {
                Assert.IsTrue(GetParamInfo(PARAM_Z_POS, ref paramType, ref paramAvailable, ref paramReadOnly, ref paramMin, ref paramMax, ref paramDefault) == TRUE);
                if (paramAvailable == TRUE)
                {
                    double firstPos = paramMin + (paramMax - paramMin) *.10;
                    // Move to 1st  position:
                    Assert.IsTrue(SetParam(PARAM_Z_POS, firstPos) == TRUE);
                    Assert.IsTrue(PreflightPosition() == TRUE);
                    Assert.IsTrue(SetupPosition() == TRUE);
                    Assert.IsTrue(StartPosition() == TRUE);

                    //Stop Motor:
                    Assert.IsTrue(SetParam(PARAM_Z_STOP, TRUE) == TRUE);
                    Assert.IsTrue(PreflightPosition() == TRUE);
                    Assert.IsTrue(SetupPosition() == TRUE);
                    Assert.IsTrue(StartPosition() == TRUE);
                    
                    int status = 0;
                    do
                    {
                        Assert.IsTrue(StatusPosition(ref status) == TRUE);
                    }
                    while (status == 0);
                    
                    Assert.IsTrue(PostflightPosition() == TRUE);

                    // Move to half of 1st  position:
                    Assert.IsTrue(SetParam(PARAM_Z_POS, firstPos / 2.0) == TRUE);
                    
                    Assert.IsTrue(PreflightPosition() == TRUE);
                    Assert.IsTrue(SetupPosition() == TRUE);
                    Assert.IsTrue(StartPosition() == TRUE);

                    status = 0;
                    do
                    {
                        Assert.IsTrue(StatusPosition(ref status) == TRUE);
                    }
                    while (status == 0);

                    Assert.IsTrue(PostflightPosition() == TRUE);

                    // Rpeate move to 1st  position:
                    Assert.IsTrue(SetParam(PARAM_Z_POS, firstPos) == TRUE);

                    Assert.IsTrue(PreflightPosition() == TRUE);
                    Assert.IsTrue(SetupPosition() == TRUE);
                    Assert.IsTrue(StartPosition() == TRUE);

                    status = 0;
                    do
                    {
                        Assert.IsTrue(StatusPosition(ref status) == TRUE);
                    }
                    while (status == 0);

                    Assert.IsTrue(PostflightPosition() == TRUE);

                    // Confirm 1st position is correct:
                    Assert.IsTrue(GetParam(PARAM_Z_POS_CURRENT, ref param) == TRUE);
                    Assert.IsTrue(Math.Round(param, 2) == Math.Round(firstPos, 2));

                    // Move to 2nd/default/zero position:
                    Assert.IsTrue(SetParam(PARAM_Z_POS, paramDefault) == TRUE);

                    Assert.IsTrue(PreflightPosition() == TRUE);
                    Assert.IsTrue(SetupPosition() == TRUE);
                    Assert.IsTrue(StartPosition() == TRUE);

                    status = 0;
                    do
                    {
                        Assert.IsTrue(StatusPosition(ref status) == TRUE);
                    }
                    while (status == 0);

                    Assert.IsTrue(PostflightPosition() == TRUE);

                    // Confirm 2nd position is correct: 0 mm, 0 degree
                    Assert.IsTrue(GetParam(PARAM_Z_POS_CURRENT, ref param) == TRUE);
                    Assert.IsTrue(Math.Round(param, 2) == Math.Round(paramDefault, 2));

                    double thirdPos = paramMin + (paramMax - paramMin) * .80;
                    // Move to 3rd  position:
                    Assert.IsTrue(SetParam(PARAM_Z_POS, thirdPos) == TRUE);

                    Assert.IsTrue(PreflightPosition() == TRUE);
                    Assert.IsTrue(SetupPosition() == TRUE);
                    Assert.IsTrue(StartPosition() == TRUE);

                    status = 0;
                    do
                    {
                        Assert.IsTrue(StatusPosition(ref status) == TRUE);
                    }
                    while (status == 0);

                    Assert.IsTrue(PostflightPosition() == TRUE);

                    // Confirm 3rd position is correct:
                    Assert.IsTrue(GetParam(PARAM_Z_POS_CURRENT, ref param) == TRUE);
                    Assert.IsTrue(Math.Round(param, 2) == Math.Round(thirdPos, 2));


                    // Move back to 1st position:
                    Assert.IsTrue(SetParam(PARAM_Z_POS, firstPos) == TRUE);

                    Assert.IsTrue(PreflightPosition() == TRUE);
                    Assert.IsTrue(SetupPosition() == TRUE);
                    Assert.IsTrue(StartPosition() == TRUE);

                    status = 0;
                    do
                    {
                        Assert.IsTrue(StatusPosition(ref status) == TRUE);
                    }
                    while (status == 0);

                    Assert.IsTrue(PostflightPosition() == TRUE);

                    // Confirm 1st position is correct:
                    Assert.IsTrue(GetParam(PARAM_Z_POS_CURRENT, ref param) == TRUE);
                    Assert.IsTrue(Math.Round(param, 2) == Math.Round(firstPos, 2));

                    Assert.IsTrue(TeardownDevice()==TRUE);
                }
            }
        }

        [DllImport(".\\Modules_Native\\ThorPLSZ.dll", EntryPoint = "GetParam", CallingConvention = CallingConvention.Cdecl)]
        private static extern int GetParam(int paramID, ref double param);

        [DllImport(".\\Modules_Native\\ThorPLSZ.dll", EntryPoint = "GetParamInfo", CallingConvention = CallingConvention.Cdecl)]
        private static extern int GetParamInfo(int paramID, ref int paramType, ref int paramAvailable, ref int paramReadOnly, ref double paramMin, ref double paramMax, ref double paramDefault);

        [DllImport(".\\Modules_Native\\ThorPLSZ.dll", EntryPoint = "PostflightPosition", CallingConvention = CallingConvention.Cdecl)]
        private static extern int PostflightPosition();

        [DllImport(".\\Modules_Native\\ThorPLSZ.dll", EntryPoint = "PreflightPosition", CallingConvention = CallingConvention.Cdecl)]
        private static extern int PreflightPosition();

        [DllImport(".\\Modules_Native\\ThorPLSZ.dll", EntryPoint = "SelectDevice", CallingConvention = CallingConvention.Cdecl)]
        private static extern int SelectDevice(int device);

        [DllImport(".\\Modules_Native\\ThorPLSZ.dll", EntryPoint = "SetParam", CallingConvention = CallingConvention.Cdecl)]
        private static extern int SetParam(int paramID, double param);

        [DllImport(".\\Modules_Native\\ThorPLSZ.dll", EntryPoint = "SetupPosition", CallingConvention = CallingConvention.Cdecl)]
        private static extern int SetupPosition();

        [DllImport(".\\Modules_Native\\ThorPLSZ.dll", EntryPoint = "StartPosition", CallingConvention = CallingConvention.Cdecl)]
        private static extern int StartPosition();

        [DllImport(".\\Modules_Native\\ThorPLSZ.dll", EntryPoint = "StatusPosition", CallingConvention = CallingConvention.Cdecl)]
        private static extern int StatusPosition(ref int status);

        [DllImport(".\\Modules_Native\\ThorPLSZ.dll", EntryPoint = "TeardownDevice", CallingConvention = CallingConvention.Cdecl)]
        private static extern int TeardownDevice();

        #endregion Methods
    }
}