namespace ThorMCM3000Test
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
        private const int PARAM_X_ACCEL = 205;
        private const int PARAM_X_DECEL = 206;
        private const int PARAM_X_HOME = 201;
        private const int PARAM_X_JOYSTICK_VELOCITY = 208;
        private const int PARAM_X_POS = 200;
        private const int PARAM_X_POS_CURRENT = 207;
        private const int PARAM_X_STEPS_PER_MM = 204;
        private const int PARAM_X_STOP = 209;
        private const int PARAM_X_VELOCITY = 203;
        private const int PARAM_X_VELOCITY_CURRENT = 210;
        private const int PARAM_X_ZERO = 202;
        private const int PARAM_Y_ACCEL = 305;
        private const int PARAM_Y_DECEL = 306;
        private const int PARAM_Y_HOME = 301;
        private const int PARAM_Y_JOYSTICK_VELOCITY = 308;
        private const int PARAM_Y_POS = 300;
        private const int PARAM_Y_POS_CURRENT = 307;
        private const int PARAM_Y_STEPS_PER_MM = 304;
        private const int PARAM_Y_STOP = 309;
        private const int PARAM_Y_VELOCITY = 303;
        private const int PARAM_Y_VELOCITY_CURRENT = 310;
        private const int PARAM_Y_ZERO = 302;
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
        private const int STAGE_X = 0x00000004;
        private const int STAGE_Y = 0x00000008;
        private const int STAGE_Z = 0x00000010;
        private const int TRUE = 1, FALSE = 0;

        #endregion Fields

        #region Methods

        [TestMethod]
        [DeploymentItem("..\\..\\..\\x64\\Debug\\ThorMCM3000.dll", "Modules_Native")]
        [DeploymentItem("..\\..\\..\\ThorMCM3000Settings.xml")]
        public void Test_X_Stage()
        {
            Assert.IsTrue(File.Exists("ThorMCM3000Settings.xml"));
            Assert.IsTrue(File.Exists(".\\Modules_Native\\ThorMCM3000.dll"));

            int paramType = 0;
            int paramAvailable = 0;
            int paramReadOnly = 0;
            double paramMin = 0.0;
            double paramMax = 0.0;
            double paramDefault = 0.0;
            double param = 0.0;

            Assert.IsTrue(SelectDevice(0) == TRUE);
            Assert.IsTrue(GetParamInfo(PARAM_DEVICE_TYPE, ref paramType, ref paramAvailable, ref paramReadOnly, ref paramMin, ref paramMax, ref paramDefault) == TRUE);

            int targetDevType = (int)(STAGE_X | STAGE_Y | STAGE_Z);

            if (((int)(paramDefault) & targetDevType) != targetDevType)
            {

            }
            else
            {
                Assert.IsTrue(GetParamInfo(PARAM_X_POS, ref paramType, ref paramAvailable, ref paramReadOnly, ref paramMin, ref paramMax, ref paramDefault) == TRUE);
                if (paramAvailable == TRUE)
                {
                    // Move to 1st  position:
                    Assert.IsTrue(SetParam(PARAM_X_POS, -paramMin / 10) == TRUE);

                    Assert.IsTrue(PreflightPosition() == TRUE);
                    Assert.IsTrue(SetupPosition() == TRUE);
                    Assert.IsTrue(StartPosition() == TRUE);

                    //Stop Motor:
                    Assert.IsTrue(SetParam(PARAM_X_STOP, TRUE) == TRUE);

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
                    Assert.IsTrue(SetParam(PARAM_X_POS, paramMin / 20) == TRUE);

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


                    // Repeat move to 1st  position:
                    Assert.IsTrue(SetParam(PARAM_X_POS, paramMin / 10) == TRUE);

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
                    Assert.IsTrue(GetParam(PARAM_X_POS_CURRENT, ref param) == TRUE);
                    Assert.IsTrue(Math.Round(param, 2) == Math.Round(paramMin / 10, 2));

                    // Move to 2nd/default/zero position:
                    Assert.IsTrue(SetParam(PARAM_X_POS, paramDefault) == TRUE);

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
                    Assert.IsTrue(GetParam(PARAM_X_POS_CURRENT, ref param) == TRUE);
                    Assert.IsTrue(Math.Round(param, 2) == Math.Round(paramDefault, 2));

                    // Move to 3rd  position:
                    Assert.IsTrue(SetParam(PARAM_X_POS, paramMax / 10) == TRUE);

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
                    Assert.IsTrue(GetParam(PARAM_X_POS_CURRENT, ref param) == TRUE);
                    Assert.IsTrue(Math.Round(param, 2) == Math.Round(paramMax / 10, 2));

                    // Set Zero position:
                    Assert.IsTrue(SetParam(PARAM_X_ZERO, TRUE) == TRUE);

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
                    Assert.IsTrue(GetParam(PARAM_X_POS_CURRENT, ref param) == TRUE);
                    Assert.IsTrue(Math.Round(param, 2) == Math.Round(0.0, 2));

                    // Move back to 2nd position:
                    Assert.IsTrue(SetParam(PARAM_X_POS, paramMin / 10) == TRUE);

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

                    // Confirm 2nd position is correct:
                    Assert.IsTrue(GetParam(PARAM_X_POS_CURRENT, ref param) == TRUE);
                    Assert.IsTrue(Math.Round(param, 2) == Math.Round(paramMin / 10, 2));

                    // Set Zero position:
                    Assert.IsTrue(SetParam(PARAM_X_ZERO, TRUE) == TRUE);

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

                    // Confirm Zero position is correct:
                    Assert.IsTrue(GetParam(PARAM_X_POS_CURRENT, ref param) == TRUE);
                    Assert.IsTrue(Math.Round(param, 2) == Math.Round(0.0, 2));

                }
            }
        }

        [TestMethod]
        [DeploymentItem("..\\..\\..\\x64\\Debug\\ThorMCM3000.dll", "Modules_Native")]
        [DeploymentItem("..\\..\\..\\ThorMCM3000Settings.xml")]
        public void Test_Y_Stage()
        {
            Assert.IsTrue(File.Exists("ThorMCM3000Settings.xml"));

            Assert.IsTrue(File.Exists(".\\Modules_Native\\ThorMCM3000.dll"));

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

            int targetDevType = (int)(STAGE_X | STAGE_Y | STAGE_Z);

            if (((int)(paramDefault) & targetDevType) != targetDevType)
            {

            }
            else
            {
                Assert.IsTrue(GetParamInfo(PARAM_Y_POS, ref paramType, ref paramAvailable, ref paramReadOnly, ref paramMin, ref paramMax, ref paramDefault) == TRUE);
                if (paramAvailable == TRUE)
                {
                    // Move to 1st  position:
                    Assert.IsTrue(SetParam(PARAM_Y_POS, paramMin / 10) == TRUE);
                    Assert.IsTrue(SetupPosition() == TRUE);
                    Assert.IsTrue(StartPosition() == TRUE);

                    //Stop Motor:
                    Assert.IsTrue(SetParam(PARAM_Y_STOP, TRUE) == TRUE);
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
                    Assert.IsTrue(SetParam(PARAM_Y_POS, paramMin / 20) == TRUE);

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

                    // Repeate Move to 1st  position:
                    Assert.IsTrue(SetParam(PARAM_Y_POS, paramMin / 10) == TRUE);

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
                    Assert.IsTrue(GetParam(PARAM_Y_POS_CURRENT, ref param) == TRUE);
                    Assert.IsTrue(Math.Round(param, 2) == Math.Round(paramMin / 10, 2));

                    // Move to 2nd/default/zero position:
                    Assert.IsTrue(SetParam(PARAM_Y_POS, paramDefault) == TRUE);

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
                    Assert.IsTrue(GetParam(PARAM_Y_POS_CURRENT, ref param) == TRUE);
                    Assert.IsTrue(Math.Round(param, 2) == Math.Round(paramDefault, 2));

                    // Move to 3rd  position:
                    Assert.IsTrue(SetParam(PARAM_Y_POS, paramMax / 10) == TRUE);

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
                    Assert.IsTrue(GetParam(PARAM_Y_POS_CURRENT, ref param) == TRUE);
                    Assert.IsTrue(Math.Round(param, 2) == Math.Round(paramMax / 10, 2));

                    // Set Zero position:
                    Assert.IsTrue(SetParam(PARAM_Y_ZERO, TRUE) == TRUE);

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
                    Assert.IsTrue(GetParam(PARAM_Y_POS_CURRENT, ref param) == TRUE);
                    Assert.IsTrue(Math.Round(param, 2) == Math.Round(0.0, 2));

                    // Move back to 2nd position:
                    Assert.IsTrue(SetParam(PARAM_Y_POS, paramMin / 10) == TRUE);

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

                    // Confirm 2nd position is correct:
                    Assert.IsTrue(GetParam(PARAM_Y_POS_CURRENT, ref param) == TRUE);
                    Assert.IsTrue(Math.Round(param, 2) == Math.Round(paramMin / 10, 2));

                    // Set Zero position:
                    Assert.IsTrue(SetParam(PARAM_Y_ZERO, TRUE) == TRUE);

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

                    // Confirm Zero position is correct:
                    Assert.IsTrue(GetParam(PARAM_Y_POS_CURRENT, ref param) == TRUE);
                    Assert.IsTrue(Math.Round(param, 2) == Math.Round(0.0, 2));
                }
            }
        }

        [TestMethod]
        [DeploymentItem("..\\..\\..\\x64\\Debug\\ThorMCM3000.dll", "Modules_Native")]
        [DeploymentItem("..\\..\\..\\ThorMCM3000Settings.xml")]
        public void Test_Z_Stage()
        {
            Assert.IsTrue(File.Exists("ThorMCM3000Settings.xml"));

            Assert.IsTrue(File.Exists(".\\Modules_Native\\ThorMCM3000.dll"));

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

            int targetDevType = (int)(STAGE_X | STAGE_Y | STAGE_Z);

            if (((int)(paramDefault) & targetDevType) != targetDevType)
            {

            }
            else
            {
                Assert.IsTrue(GetParamInfo(PARAM_Z_POS, ref paramType, ref paramAvailable, ref paramReadOnly, ref paramMin, ref paramMax, ref paramDefault) == TRUE);
                if (paramAvailable == TRUE)
                {
                    // Move to 1st  position:
                    Assert.IsTrue(SetParam(PARAM_Z_POS, paramMin / 10) == TRUE);
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
                    Assert.IsTrue(SetParam(PARAM_Z_POS, paramMin / 20) == TRUE);
                    
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
                    Assert.IsTrue(SetParam(PARAM_Z_POS, paramMin / 10) == TRUE);

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
                    Assert.IsTrue(Math.Round(param, 2) == Math.Round(paramMin / 10, 2));

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

                    // Move to 3rd  position:
                    Assert.IsTrue(SetParam(PARAM_Z_POS, paramMax / 10) == TRUE);

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
                    Assert.IsTrue(Math.Round(param, 2) == Math.Round(paramMax / 10, 2));

                    // Set Zero position:
                    Assert.IsTrue(SetParam(PARAM_Z_ZERO, TRUE) == TRUE);

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
                    Assert.IsTrue(Math.Round(param, 2) == Math.Round(0.0, 2));

                    // Move back to 2nd position:
                    Assert.IsTrue(SetParam(PARAM_Z_POS, paramMin / 10) == TRUE);

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

                    // Confirm 2nd position is correct:
                    Assert.IsTrue(GetParam(PARAM_Z_POS_CURRENT, ref param) == TRUE);
                    Assert.IsTrue(Math.Round(param, 2) == Math.Round(paramMin / 10, 2));

                    // Set Zero position:
                    Assert.IsTrue(SetParam(PARAM_Z_ZERO, TRUE) == TRUE);

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

                    // Confirm Zero position is correct:
                    Assert.IsTrue(GetParam(PARAM_Z_POS_CURRENT, ref param) == TRUE);
                    Assert.IsTrue(Math.Round(param, 2) == Math.Round(0.0, 2));
                }
            }
        }

        [DllImport(".\\Modules_Native\\ThorMCM3000.dll", EntryPoint = "GetParam", CallingConvention = CallingConvention.Cdecl)]
        private static extern int GetParam(int paramID, ref double param);

        [DllImport(".\\Modules_Native\\ThorMCM3000.dll", EntryPoint = "GetParamInfo", CallingConvention = CallingConvention.Cdecl)]
        private static extern int GetParamInfo(int paramID, ref int paramType, ref int paramAvailable, ref int paramReadOnly, ref double paramMin, ref double paramMax, ref double paramDefault);

        [DllImport(".\\Modules_Native\\ThorMCM3000.dll", EntryPoint = "PostflightPosition", CallingConvention = CallingConvention.Cdecl)]
        private static extern int PostflightPosition();

        [DllImport(".\\Modules_Native\\ThorMCM3000.dll", EntryPoint = "PreflightPosition", CallingConvention = CallingConvention.Cdecl)]
        private static extern int PreflightPosition();

        [DllImport(".\\Modules_Native\\ThorMCM3000.dll", EntryPoint = "SelectDevice", CallingConvention = CallingConvention.Cdecl)]
        private static extern int SelectDevice(int device);

        [DllImport(".\\Modules_Native\\ThorMCM3000.dll", EntryPoint = "SetParam", CallingConvention = CallingConvention.Cdecl)]
        private static extern int SetParam(int paramID, double param);

        [DllImport(".\\Modules_Native\\ThorMCM3000.dll", EntryPoint = "SetupPosition", CallingConvention = CallingConvention.Cdecl)]
        private static extern int SetupPosition();

        [DllImport(".\\Modules_Native\\ThorMCM3000.dll", EntryPoint = "StartPosition", CallingConvention = CallingConvention.Cdecl)]
        private static extern int StartPosition();

        [DllImport(".\\Modules_Native\\ThorMCM3000.dll", EntryPoint = "StatusPosition", CallingConvention = CallingConvention.Cdecl)]
        private static extern int StatusPosition(ref int status);

        #endregion Methods
    }
}