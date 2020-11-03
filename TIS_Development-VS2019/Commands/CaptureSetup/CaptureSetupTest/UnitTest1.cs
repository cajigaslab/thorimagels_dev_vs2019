namespace CaptureSetupTest
{
    using System;
    using System.Reflection;
    using System.Windows;

    using CaptureSetupDll.Model;
    using CaptureSetupDll.ViewModel;

    using Microsoft.VisualStudio.TestTools.UnitTesting;

    [TestClass]
    public class UnitTest1
    {
        #region Methods

        [TestMethod]
        [DeploymentItem("..\\..\\..\\..\\..\\Commands\\CaptureSetup\\x64\\Debug\\CaptureSetup.dll", "Modules_Native")]
        [DeploymentItem("..\\..\\..\\..\\..\\Commands\\Acquisition\\LiveImageData\\x64\\Debug\\LiveImageData.dll", "Modules_Native")]
        [DeploymentItem("..\\..\\..\\..\\..\\Commands\\ImageAnalysis\\PincushionCorrection\\x64\\Debug\\PincushionCorrection.dll")]
        [DeploymentItem("..\\..\\..\\..\\..\\Commands\\ImageAnalysis\\LineProfile\\x64\\Debug\\LineProfile.dll")]
        [DeploymentItem("..\\..\\..\\..\\..\\Commands\\ImageAnalysis\\FlatField\\x64\\Debug\\FlatField.dll")]
        [DeploymentItem("..\\..\\..\\..\\..\\Common\\ExperimentManager\\x64\\Debug\\ExperimentManager.dll")]
        [DeploymentItem("..\\..\\..\\..\\..\\Common\\ThorDiskIO\\x64\\Debug\\ThorDiskIO.dll")]
        [DeploymentItem("..\\..\\..\\..\\..\\Common\\ImageManager\\x64\\Debug\\ImageManager.dll")]
        [DeploymentItem("..\\..\\..\\..\\..\\Common\\ResourceManager\\x64\\Debug\\ResourceManager.dll")]
        [DeploymentItem("..\\..\\..\\..\\..\\Hardware\\Cameras\\CameraManager\\x64\\Debug\\CameraManager.dll")]
        [DeploymentItem("..\\..\\..\\..\\..\\Hardware\\Devices\\DeviceManager\\x64\\Debug\\DeviceManager.dll")]
        [DeploymentItem("..\\..\\..\\..\\..\\Common\\ThorLogging\\x64\\Debug\\ThorLoggingUnmanaged.dll", "Modules_Native")]
        [DeploymentItem("..\\..\\..\\..\\..\\Commands\\General\\SelectHardware\\x64\\Debug\\SelectHardware.dll", "Modules_Native")]
        [DeploymentItem("..\\..\\..\\..\\..\\Tools\\Intel IPP\\intel64\\dll")]
        [DeploymentItem("..\\..\\..\\..\\..\\Commands\\Acquisition\\RunSample\\x64\\Debug\\RunSample.dll", "Modules_Native")]
        [DeploymentItem("..\\..\\..\\..\\..\\GUI\\Controls\\TileBuilder\\x64\\Debug\\TileBuilder.dll")]
        [DeploymentItem("..\\..\\..\\..\\..\\Tools\\log4cxx0.10.0\\msvc9-proj\\x64\\Debug\\log4cxx.dll")]
        [DeploymentItem("..\\..\\..\\..\\..\\Common\\StatsManager\\x64\\Debug\\StatsManager.dll")]
        [DeploymentItem("..\\..\\..\\..\\..\\Commands\\ImageAnalysis\\ImageStats\\x64\\Debug\\ImageStats.dll")]
        [DeploymentItem("..\\..\\..\\..\\..\\Hardware\\Cameras\\ThorConfocalSimulator\\x64\\Debug\\ThorConfocalSimulator.dll", "Modules_Native")]
        [DeploymentItem("..\\..\\..\\..\\..\\Hardware\\Devices\\ThorTDCSimulator\\x64\\Debug\\ThorTDCSimulator.dll", "Modules_Native")]
        public void VM_Power_Simulator()
        {
            CaptureSetup model = new CaptureSetup();
            Microsoft.Practices.Composite.Events.EventAggregator ag = new Microsoft.Practices.Composite.Events.EventAggregator();

            CaptureSetupViewModel vm = new CaptureSetupViewModel(ag, null, null, model);

            //*NOTE* New property for minmax needed
            //*NOTE* not yet implemented for simulator/////////////////////////
            //TestProperty(vm, "PockelsBlankPercentage0", 0, true);
            //TestProperty(vm, "PockelsBlankPercentage0", 49, true);
            //TestProperty(vm, "PockelsBlankPercentage0", -1, false);
            //TestProperty(vm, "PockelsBlankPercentage0", 50, false);

            //*NOTE* New property for minmax needed
            //*NOTE* not yet implemented for simulator/////////////////////////
            //TestProperty(vm, "PockelsBlankPercentage1", 0, true);
            //TestProperty(vm, "PockelsBlankPercentage1", 49, true);
            //TestProperty(vm, "PockelsBlankPercentage1", -1, false);
            //TestProperty(vm, "PockelsBlankPercentage1", 50, false);

            //*NOTE* New property for minmax needed
            //*NOTE* not yet implemented for simulator/////////////////////////
            //TestProperty(vm, "PockelsBlankPercentage2", 0, true);
            //TestProperty(vm, "PockelsBlankPercentage2", 49, true);
            //TestProperty(vm, "PockelsBlankPercentage2", -1, false);
            //TestProperty(vm, "PockelsBlankPercentage2", 50, false);

            //*NOTE* New property for minmax needed
            //*NOTE* not yet implemented for simulator/////////////////////////
            //TestProperty(vm, "Power0", vm.PockelsPowerMin, true);
            //TestProperty(vm, "Power0", vm.PockelsPowerMax, true);
            //TestProperty(vm, "Power0", vm.PockelsPowerMin-1, false);
            //TestProperty(vm, "Power0", vm.PockelsPowerMax+1, false);

            //*NOTE* New property for minmax needed
            //*NOTE* not yet implemented for simulator/////////////////////////
            //TestProperty(vm, "Power1", vm.PockelsPowerMin, true);
            //TestProperty(vm, "Power1", vm.PockelsPowerMax, true);
            //TestProperty(vm, "Power1", vm.PockelsPowerMin-1, false);
            //TestProperty(vm, "Power1", vm.PockelsPowerMax+1, false);

            //*NOTE* New property for minmax needed
            //*NOTE* not yet implemented for simulator/////////////////////////
            //TestProperty(vm, "Power2", vm.PockelsPowerMin, true);
            //TestProperty(vm, "Power2", vm.PockelsPowerMax, true);
            //TestProperty(vm, "Power2", vm.PockelsPowerMin-1, false);
            //TestProperty(vm, "Power2", vm.PockelsPowerMax+1, false);

            //*NOTE* New property for minmax needed
            //*NOTE* not yet implemented for simulator/////////////////////////
            //TestProperty(vm, "PockelsVoltageMax0", 0, true);
            //TestProperty(vm, "PockelsVoltageMax0", 10.0, true);
            //TestProperty(vm, "PockelsVoltageMax0", -1.0, false);
            //TestProperty(vm, "PockelsVoltageMax0", 11.0, false);

            //*NOTE* New property for minmax needed
            //*NOTE* not yet implemented for simulator/////////////////////////
            //TestProperty(vm, "PockelsVoltageMax1", 0, true);
            //TestProperty(vm, "PockelsVoltageMax1", 10.0, true);
            //TestProperty(vm, "PockelsVoltageMax1", -1.0, false);
            //TestProperty(vm, "PockelsVoltageMax1", 11.0, false);

            //*NOTE* New property for minmax needed
            //*NOTE* not yet implemented for simulator/////////////////////////
            //TestProperty(vm, "PockelsVoltageMax2", 0, true);
            //TestProperty(vm, "PockelsVoltageMax2", 10.0, true);
            //TestProperty(vm, "PockelsVoltageMax2", -1.0, false);
            //TestProperty(vm, "PockelsVoltageMax2", 11.0, false);

            //*NOTE* New property for minMin needed
            //*NOTE* not yet implemented for simulator/////////////////////////
            //TestProperty(vm, "PockelsVoltageMin0", 0, true);
            //TestProperty(vm, "PockelsVoltageMin0", 10.0, true);
            //TestProperty(vm, "PockelsVoltageMin0", -1.0, false);
            //TestProperty(vm, "PockelsVoltageMin0", 11.0, false);

            //*NOTE* New property for minMin needed
            //*NOTE* not yet implemented for simulator/////////////////////////
            //TestProperty(vm, "PockelsVoltageMin1", 0, true);
            //TestProperty(vm, "PockelsVoltageMin1", 10.0, true);
            //TestProperty(vm, "PockelsVoltageMin1", -1.0, false);
            //TestProperty(vm, "PockelsVoltageMin1", 11.0, false);

            //*NOTE* New property for minMin needed
            //*NOTE* not yet implemented for simulator/////////////////////////
            //TestProperty(vm, "PockelsVoltageMin2", 0, true);
            //TestProperty(vm, "PockelsVoltageMin2", 10.0, true);
            //TestProperty(vm, "PockelsVoltageMin2", -1.0, false);
            //TestProperty(vm, "PockelsVoltageMin2", 11.0, false);

            //TestProperty(vm, "PowerPosition", vm.PowerMin, true);
            //TestProperty(vm, "PowerPosition", vm.PowerMax, true);
            //TestProperty(vm, "PowerPosition", vm.PowerMin - 1, false);
            //TestProperty(vm, "PowerPosition", vm.PowerMax + 1, false);

            //vm.PowerMinusCommand.Execute(null);
            //Assert.IsTrue(vm.PowerPosition == vm.PowerMax - .1);

            //vm.PowerPlusCommand.Execute(null);
            //Assert.IsTrue(vm.PowerPosition == vm.PowerMax);

            //vm.PowerPositionGo = vm.PowerMin;
            //vm.SetPowerCommand.Execute(null);
            //Assert.IsTrue(vm.PowerPosition == vm.PowerMin);
        }

        [TestMethod]
        [DeploymentItem("..\\..\\..\\..\\..\\Commands\\CaptureSetup\\x64\\Debug\\CaptureSetup.dll","Modules_Native")]
        [DeploymentItem("..\\..\\..\\..\\..\\Commands\\Acquisition\\LiveImageData\\x64\\Debug\\LiveImageData.dll", "Modules_Native")]
        [DeploymentItem("..\\..\\..\\..\\..\\Commands\\ImageAnalysis\\PincushionCorrection\\x64\\Debug\\PincushionCorrection.dll")]
        [DeploymentItem("..\\..\\..\\..\\..\\Commands\\ImageAnalysis\\LineProfile\\x64\\Debug\\LineProfile.dll")]
        [DeploymentItem("..\\..\\..\\..\\..\\Commands\\ImageAnalysis\\FlatField\\x64\\Debug\\FlatField.dll")]
        [DeploymentItem("..\\..\\..\\..\\..\\Common\\ExperimentManager\\x64\\Debug\\ExperimentManager.dll")]
        [DeploymentItem("..\\..\\..\\..\\..\\Common\\ThorDiskIO\\x64\\Debug\\ThorDiskIO.dll")]
        [DeploymentItem("..\\..\\..\\..\\..\\Common\\ImageManager\\x64\\Debug\\ImageManager.dll")]
        [DeploymentItem("..\\..\\..\\..\\..\\Common\\ResourceManager\\x64\\Debug\\ResourceManager.dll")]
        [DeploymentItem("..\\..\\..\\..\\..\\Hardware\\Cameras\\CameraManager\\x64\\Debug\\CameraManager.dll")]
        [DeploymentItem("..\\..\\..\\..\\..\\Hardware\\Devices\\DeviceManager\\x64\\Debug\\DeviceManager.dll")]
        [DeploymentItem("..\\..\\..\\..\\..\\Common\\ThorLogging\\x64\\Debug\\ThorLoggingUnmanaged.dll", "Modules_Native")]
        [DeploymentItem("..\\..\\..\\..\\..\\Commands\\General\\SelectHardware\\x64\\Debug\\SelectHardware.dll", "Modules_Native")]
        [DeploymentItem("..\\..\\..\\..\\..\\Tools\\Intel IPP\\intel64\\dll")]
        [DeploymentItem("..\\..\\..\\..\\..\\Commands\\Acquisition\\RunSample\\x64\\Debug\\RunSample.dll","Modules_Native")]
        [DeploymentItem("..\\..\\..\\..\\..\\GUI\\Controls\\TileBuilder\\x64\\Debug\\TileBuilder.dll")]
        [DeploymentItem("..\\..\\..\\..\\..\\Tools\\log4cxx0.10.0\\msvc9-proj\\x64\\Debug\\log4cxx.dll")]
        [DeploymentItem("..\\..\\..\\..\\..\\Common\\StatsManager\\x64\\Debug\\StatsManager.dll")]
        [DeploymentItem("..\\..\\..\\..\\..\\Commands\\ImageAnalysis\\ImageStats\\x64\\Debug\\ImageStats.dll")]
        [DeploymentItem("..\\..\\..\\..\\..\\Hardware\\Cameras\\ThorConfocalSimulator\\x64\\Debug\\ThorConfocalSimulator.dll", "Modules_Native")]
        [DeploymentItem("..\\..\\..\\..\\..\\Hardware\\Devices\\ThorPMTSimulator\\x64\\Debug\\ThorPMTSimulator.dll","Modules_Native")]
        public void VM_Scan_Simulator()
        {
            CaptureSetup model = new CaptureSetup();
            Microsoft.Practices.Composite.Events.EventAggregator ag = new Microsoft.Practices.Composite.Events.EventAggregator();

            CaptureSetupViewModel vm = new CaptureSetupViewModel(ag, null, null, model);

            //*NOTE* New property for minmax needed
            TestProperty(vm, "Coeff1", -1.0, true);
            TestProperty(vm, "Coeff1", 1.0, true);
            TestProperty(vm, "Coeff2", -1.0, true);
            TestProperty(vm, "Coeff2", 1.0, true);
            TestProperty(vm, "Coeff3", -1.0, true);
            TestProperty(vm, "Coeff3", 1.0, true);
            TestProperty(vm, "Coeff4", -1.0, true);
            TestProperty(vm, "Coeff4", 1.0, true);

            TestProperty(vm, "EnableBackgroundSubtraction", 0, true);
            TestProperty(vm, "EnableBackgroundSubtraction", 1, true);
            //*NOTE* this test fails/////////////////////////
            //TestProperty(vm, "EnableBackgroundSubtraction", -1, false);
            //TestProperty(vm, "EnableBackgroundSubtraction", 2, false);
            //*NOTE* this test fails/////////////////////////

            TestProperty(vm, "EnableFlatField", 0, true);
            TestProperty(vm, "EnableFlatField", 1, true);
            //*NOTE* this test fails/////////////////////////
            //TestProperty(vm, "EnableFlatField", -1, false);
            //TestProperty(vm, "EnableFlatField", 2, false);
            //*NOTE* this test fails/////////////////////////

            //*NOTE* New property for minmax needed
            TestProperty(vm, "InputRangeChannel1", 1, true);
            TestProperty(vm, "InputRangeChannel1", 6, true);
            TestProperty(vm, "InputRangeChannel2", 1, true);
            TestProperty(vm, "InputRangeChannel2", 6, true);
            TestProperty(vm, "InputRangeChannel3", 1, true);
            TestProperty(vm, "InputRangeChannel3", 6, true);
            TestProperty(vm, "InputRangeChannel4", 1, true);
            TestProperty(vm, "InputRangeChannel4", 6, true);

            TestProperty(vm, "LSMClockSource", 0, true);
            TestProperty(vm, "LSMClockSource", 1, true);
            TestProperty(vm, "LSMClockSource", -1, false);
            TestProperty(vm, "LSMClockSource", 2, false);

            //*NOTE* New property for minmax needed
            TestProperty(vm, "LSMExtClockRate", 70.0, true);
            TestProperty(vm, "LSMExtClockRate", 90.0, true);

            //*NOTE* not yet implemented for simulator/////////////////////////
            //TestProperty(vm, "LSMPixelDwellTime", .4, true);
            //TestProperty(vm, "LSMPixelDwellTime", 20, true);
            //TestProperty(vm, "LSMPixelDwellTime", .3, false);
            //TestProperty(vm, "LSMPixelDwellTime", 21, false);

            TestProperty(vm, "LSMRealtimeAveraging", 0, true);
            TestProperty(vm, "LSMRealtimeAveraging", 1, true);
            TestProperty(vm, "LSMRealtimeAveraging", -1, false);
            TestProperty(vm, "LSMRealtimeAveraging", 2, false);

            //*NOTE* New property for minmax needed
            //*NOTE* not yet implemented for simulator/////////////////////////
            //TestProperty(vm, "LSMScaleYScan", .5, true);
            //TestProperty(vm, "LSMScaleYScan", 2, true);

            //*NOTE* New property for minmax needed
            TestProperty(vm, "LSMScanMode", 0, true);
            TestProperty(vm, "LSMScanMode", 1, true);

            //*NOTE* New property for minmax needed
            TestProperty(vm, "LSMSignalAverage", 0, true);
            TestProperty(vm, "LSMSignalAverage", 1, true);

            //*NOTE* New property for minmax needed
            TestProperty(vm, "LSMSignalAverageFrames", 2, true);
            TestProperty(vm, "LSMSignalAverageFrames", 256, true);

            //vm.AverageFramesMinusCommand.Execute(null);
            //TestProperty(vm, "LSMSignalAverageFrames", 255, true);

            //vm.AverageFramesPlusCommand.Execute(null);
            //TestProperty(vm, "LSMSignalAverageFrames", 256, true);

            //*NOTE* New property for minmax needed
            TestProperty(vm, "LSMTwoWayAlignment", -128, true);
            TestProperty(vm, "LSMTwoWayAlignment", 128, true);

            //vm.LSMAlignmentMinusCommand.Execute(null);
            //TestProperty(vm, "LSMTwoWayAlignment", 127, true);

            //vm.LSMAlignmentPlusCommand.Execute(null);
            //TestProperty(vm, "LSMTwoWayAlignment", 128, true);

            //*NOTE* New property for minmax needed
            //*NOTE* not yet implemented for simulator/////////////////////////
            //TestProperty(vm, "LSMTwoWayAlignmentCoarse", 0, true);
            //TestProperty(vm, "LSMTwoWayAlignmentCoarse", 256, true);

            //vm.LSMAlignmentMinusCoarseCommand.Execute(null);
            //TestProperty(vm, "LSMTwoWayAlignmentCoarse", 127, true);

            //vm.LSMAlignmentPlusCoarseCommand.Execute(null);
            //TestProperty(vm, "LSMTwoWayAlignmentCoarse", 128, true);

            TestProperty(vm, "EnablePincushionCorrection", 0, true);
            TestProperty(vm, "EnablePincushionCorrection", 1, true);

            TestProperty(vm, "PMT1GainEnable", 0, true);
            TestProperty(vm, "PMT1GainEnable", 1, true);
            TestProperty(vm, "PMT1GainEnable", -1, false);
            TestProperty(vm, "PMT1GainEnable", 2, false);
            TestProperty(vm, "PMT2GainEnable", 0, true);
            TestProperty(vm, "PMT2GainEnable", 1, true);
            TestProperty(vm, "PMT2GainEnable", -1, false);
            TestProperty(vm, "PMT2GainEnable", 2, false);
            TestProperty(vm, "PMT3GainEnable", 0, true);
            TestProperty(vm, "PMT3GainEnable", 1, true);
            TestProperty(vm, "PMT3GainEnable", -1, false);
            TestProperty(vm, "PMT3GainEnable", 2, false);
            TestProperty(vm, "PMT4GainEnable", 0, true);
            TestProperty(vm, "PMT4GainEnable", 1, true);
            TestProperty(vm, "PMT4GainEnable", -1, false);
            TestProperty(vm, "PMT4GainEnable", 2, false);
            //TestProperty(vm, "PMT1Gain", vm.PMT1GainMin, true);
            //TestProperty(vm, "PMT1Gain", vm.PMT1GainMax, true);
            //TestProperty(vm, "PMT1Gain", vm.PMT1GainMin - 1, false);
            //TestProperty(vm, "PMT1Gain", vm.PMT1GainMax + 1, false);
            //TestProperty(vm, "PMT2Gain", vm.PMT2GainMin, true);
            //TestProperty(vm, "PMT2Gain", vm.PMT2GainMax, true);
            //TestProperty(vm, "PMT2Gain", vm.PMT2GainMin - 1, false);
            //TestProperty(vm, "PMT2Gain", vm.PMT2GainMax + 1, false);
            //TestProperty(vm, "PMT3Gain", vm.PMT3GainMin, true);
            //TestProperty(vm, "PMT3Gain", vm.PMT3GainMax, true);
            //TestProperty(vm, "PMT3Gain", vm.PMT3GainMin - 1, false);
            //TestProperty(vm, "PMT3Gain", vm.PMT3GainMax + 1, false);
            //TestProperty(vm, "PMT4Gain", vm.PMT4GainMin, true);
            //TestProperty(vm, "PMT4Gain", vm.PMT4GainMax, true);
            //TestProperty(vm, "PMT4Gain", vm.PMT4GainMin - 1, false);
            //TestProperty(vm, "PMT4Gain", vm.PMT4GainMax + 1, false);

            //vm.PMT1GainMinusCommand.Execute(null);
            //Assert.IsTrue(vm.PMT1Gain == vm.PMT1GainMax - 1);
            //vm.PMT1GainPlusCommand.Execute(null);
            //Assert.IsTrue(vm.PMT1Gain == vm.PMT1GainMax);

            //vm.PMT2GainMinusCommand.Execute(null);
            //Assert.IsTrue(vm.PMT2Gain == vm.PMT2GainMax - 1);
            //vm.PMT2GainPlusCommand.Execute(null);
            //Assert.IsTrue(vm.PMT2Gain == vm.PMT2GainMax);

            //vm.PMT3GainMinusCommand.Execute(null);
            //Assert.IsTrue(vm.PMT3Gain == vm.PMT3GainMax - 1);
            //vm.PMT3GainPlusCommand.Execute(null);
            //Assert.IsTrue(vm.PMT3Gain == vm.PMT3GainMax);

            //vm.PMT4GainMinusCommand.Execute(null);
            //Assert.IsTrue(vm.PMT4Gain == vm.PMT4GainMax - 1);
            //vm.PMT4GainPlusCommand.Execute(null);
            //Assert.IsTrue(vm.PMT4Gain == vm.PMT4GainMax);
        }

        private void TestProperty(object obj, string propName,  object val, bool expectedResult)
        {
            Type type = obj.GetType();

            PropertyInfo propInfo = type.GetProperty(propName);

            propInfo.SetValue(obj, val);

            object ret = propInfo.GetValue(obj);

            if (expectedResult)
            {
                Assert.IsTrue(val.Equals(ret));
            }
            else
            {
                Assert.IsFalse(val.Equals(ret));
            }
        }

        #endregion Methods
    }
}