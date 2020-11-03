namespace CaptureSetupDll.View
{
    using System;
    using System.Collections.Generic;
    using System.Collections.Specialized;
    using System.ComponentModel;
    using System.Data;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Text.RegularExpressions;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Markup;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Navigation;
    using System.Windows.Shapes;
    using System.Xml;

    using CaptureSetupDll.Model;
    using CaptureSetupDll.ViewModel;

    using GeometryUtilities;

    using HDF5CS;

    using OverlayManager;

    using ThorLogging;

    using ThorSharedTypes;

    /// <summary>
    /// Interaction logic for EditBleachWaveform.xaml
    /// </summary>
    public partial class EditBleachWaveform : Window
    {
        #region Fields

        public const double BYTES_TO_MEGABYTES = 1048576.0;
        public const float INV_DIRECTION = -1;
        public const double KILOBYTES_TO_GIGABYTES = 1048576.0;
        public const double MEGABYTES_TO_GIGABYTES = 1024.0;

        BackgroundWorker waveformBkWkr;
        ROICapsule _roiCapsule;
        string _roiPathandName;

        #endregion Fields

        #region Constructors

        public EditBleachWaveform()
        {
            InitializeComponent();

            ActionEnableItems = new Action<bool>(EnableItems);
            waveformBkWkr = new BackgroundWorker();
            waveformBkWkr.WorkerReportsProgress = false;
            waveformBkWkr.WorkerSupportsCancellation = true;

            this.Loaded += new RoutedEventHandler(EditBleachWaveform_Loaded);
            this.Unloaded += new RoutedEventHandler(EditBleachWaveform_Unloaded);
        }

        #endregion Constructors

        #region Enumerations

        enum ClockRate
        {
            HIGH = 1540000,
            INTER_HIGH = 500000,
            MEDIUM = 250000,
            INTER_LOW = 80000,
            LOW = 20000
        }

        #endregion Enumerations

        #region Delegates

        private delegate string waveformBkWkrGetStatus();

        private delegate void waveformBkWkrSetStatus();

        private delegate bool waveformBkWkrStateChecker();

        #endregion Delegates

        #region Events

        public event Action<bool> ActionEnableItems;

        #endregion Events

        #region Properties

        public static Decimal MemorySize
        {
            get
            {
                ulong val = 0;
                MEMORYSTATUSEX msex = new MEMORYSTATUSEX();
                if (GlobalMemoryStatusEx(msex))
                {
                    val = msex.ullAvailPhys / (ulong)BYTES_TO_MEGABYTES;
                }

                return new Decimal(val / MEGABYTES_TO_GIGABYTES);
            }
        }

        public double bleachPowerVal
        {
            get;
            set;
        }

        public double[] FieldScaleFine
        {
            get;
            set;
        }

        public bool H5Saved
        {
            get;
            set;
        }

        public Decimal RequiredSize
        {
            get;
            set;
        }

        public bool Result
        {
            get;
            set;
        }

        public ROICapsule RoiCapsule
        {
            get
            { return _roiCapsule; }
            set
            { _roiCapsule = value; }
        }

        public string ROIPath
        {
            get;
            set;
        }

        public string RoiPathandName
        {
            get
            { return _roiPathandName; }
            set
            { _roiPathandName = value; }
        }

        public string Status
        {
            get
            {
                return (string)this.Dispatcher.Invoke((waveformBkWkrGetStatus)delegate
                {
                    return lblStatus.Content.ToString();
                });
            }
            set
            {
                this.Dispatcher.Invoke((waveformBkWkrSetStatus)delegate
                {
                    lblStatus.Content = value;
                });
            }
        }

        public CaptureSetupViewModel vm
        {
            get;
            set;
        }

        #endregion Properties

        #region Methods

        [return: MarshalAs(UnmanagedType.Bool)]
        [DllImport("kernel32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        static extern bool GlobalMemoryStatusEx([In, Out] MEMORYSTATUSEX lpBuffer);

        private void btnGen_Click(object sender, RoutedEventArgs e)
        {
            Status = String.Format("RAM ~{0} Gb available.\n", Decimal.Round(MemorySize, 2));
            if (WaveformBuilder.ClkRate == 0)
            {
                Status += String.Format("Please select clock rate.");
                return;
            }
            if (true == this.chkPxDen.IsChecked)
            {
                WaveformBuilder.ClkRate = (int)(vm.BleachLSMPixelXY[0] * (int)WaveformBuilder.MS_TO_S / vm.PixelDensity);
            }

            bleachPowerVal = (1 == vm.BleachPowerEnable) ?
                WaveformBuilder.GetPockelsPowerValue(vm.BleachPower, vm.BleachCalibratePockelsVoltageMin0, vm.BleachCalibratePockelsVoltageMax0, (PockelsResponseType)MVMManager.Instance["PowerControlViewModel", "BleacherPowerResponse0", 0]) :
                WaveformBuilder.GetPockelsPowerValue((double)MVMManager.Instance["PowerControlViewModel", "BleacherPower0", 0], vm.BleachCalibratePockelsVoltageMin0, vm.BleachCalibratePockelsVoltageMax0, (PockelsResponseType)MVMManager.Instance["PowerControlViewModel", "BleacherPowerResponse0", 0]);

            if (false == UpdateBleachWaveParams(WaveformBuilder.ClkRate))
            { return; }

            H5Saved = false;
            if (!waveformBkWkr.IsBusy)
            {
                Status += String.Format("Generating Waveform ...\n");
                this.CanvasSpinProgress.Visibility = Visibility.Visible;
                ActionEnableItems(false);
                waveformBkWkr.DoWork += new DoWorkEventHandler(waveformBkWkr_DoWork);
                waveformBkWkr.RunWorkerCompleted += new RunWorkerCompletedEventHandler(waveformBkWkr_RunWorkerCompleted);
                waveformBkWkr.RunWorkerAsync();
            }
        }

        private void btnOk_Click(object sender, RoutedEventArgs e)
        {
            waveformBkWkrStateChecker stillWorking = () => { return waveformBkWkr.IsBusy; };
            if (waveformBkWkr.IsBusy)
            {
                Result = false;
                waveformBkWkr.CancelAsync();
                Status += String.Format("Terminating task, please wait ...");
                while ((bool)this.Dispatcher.Invoke(stillWorking, null)) System.Threading.Thread.Sleep(50);
            }
            //delete file if terminated:
            if ((false == Result) || (vm.PixelUnitMode))
            {
                vm.DeleteFile(ROIPath + "BleachWaveform.raw");
            }
            vm.EditBleachResult = Result;
            this.Close();
        }

        /// <summary>
        /// X,Y,Pockel are all with the same length under input clock rate.
        /// </summary>
        /// <param name="clkRate"></param>
        /// <returns></returns>
        private bool BuildWaveform(int clkRate)
        {
            bool ret = true;

            //check calibration has been done:
            if (null == vm.BleachCalibrateFineScaleXY)
            { return false; }

            if ((0 == vm.BleachCalibrateFieldSize) || (null == vm.BleachCalibratePixelXY))
            { return false; }

            //***   Cycle Start    ***//
            vm.InitializeWaveformBuilder(clkRate);
            WaveformBuilder.ResetWaveform();

            bool firstEpoch = false, firstPattern = false, firstIteration = false, lastIteration = false, cycleBegin = false, cycleEnd = false, lastEpoch = false, lastPattern = false;
            byte patnTrig = (byte)0, epochTrig = (byte)0, cycleTrig = (byte)0;

            if (!vm.PixelUnitMode)
            {
                for (int iEpoch = 0; iEpoch < vm.BleachEpoches; iEpoch++)
                {
                    //First epoch:
                    firstEpoch = (0 == iEpoch) ? true : false;
                    lastEpoch = (vm.BleachEpoches - 1 == iEpoch) ? true : false;

                    foreach (BleachWaveParams bw in vm.BleachParamList)
                    {
                        firstPattern = (bw == vm.BleachParamList[0]) ? true : false;
                        lastPattern = (bw == vm.BleachParamList[vm.BleachParamList.Count - 1]) ? true : false;

                        //Each iteration including PreIdle, Dwell, PostIdle:
                        for (int it = 0; it < bw.Iterations; it++)
                        {
                            //First or last iteration / pattern:
                            firstIteration = (0 == it) ? true : false;
                            cycleBegin = (firstEpoch && firstPattern && firstIteration) ? true : false;
                            cycleTrig = (cycleBegin) ? (byte)0 : (byte)1;
                            epochTrig = (firstPattern && firstIteration) ? (byte)0 : (byte)1;
                            patnTrig = (firstIteration) ? (byte)0 : (byte)1;

                            //Transition to object's start location:
                            if ((bw.shapeType == "Polygon") || (bw.shapeType == "Polyline"))
                            {
                                WaveformBuilder.BuildTravel(bw.Vertices[0], cycleTrig, epochTrig, patnTrig);
                            }
                            else if (bw.shapeType == "Ellipse")
                            {
                                WaveformBuilder.BuildTravel(new Point(bw.Center.X + (bw.ROIWidth / 2), bw.Center.Y), cycleTrig, epochTrig, patnTrig);
                            }
                            else
                            {
                                WaveformBuilder.BuildTravel(new Point(bw.ROILeft, bw.ROITop), cycleTrig, epochTrig, patnTrig);
                            }

                            //pattern pre-idle:
                            if (firstIteration)
                                WaveformBuilder.BuildPrePatIdle(bw, cycleBegin, firstPattern);

                            //PreIdle:
                            if (!bw.PixelMode)
                                WaveformBuilder.BuildPreIdle(bw);

                            //Dwell:
                            switch (bw.shapeType)
                            {
                                case "Rectangle":
                                    //Dwell during fill:
                                    Status = String.Format("Building Rectangle ...\n");
                                    switch (bw.Fill)
                                    {
                                        case (int)BleachWaveParams.FillChoice.Raster:
                                            WaveformBuilder.BuildRectTopDown(bw, bleachPowerVal);
                                            break;
                                        default:
                                            WaveformBuilder.BuildRectContour(bw, bleachPowerVal);
                                            break;
                                    }
                                    //volt.AddRange(tmpvolt);
                                    break;
                                case "Polygon":
                                    //Dwell duing contour fill:
                                    Status = String.Format("Building Polygon ...\n");
                                    WaveformBuilder.BuildPolygon(bw, bleachPowerVal);
                                    break;
                                case "Crosshair":
                                    //Dwell at location:
                                    Status = String.Format("Building Crosshair ...\n");
                                    WaveformBuilder.BuildSpot(bw, bleachPowerVal);
                                    break;
                                case "Line":
                                    //Dwell during transition:
                                    Status = String.Format("Building Line ...\n");
                                    WaveformBuilder.BuildLine(new Point(bw.ROIRight, bw.ROIBottom), bw, bleachPowerVal);
                                    break;
                                case "Polyline":
                                    //Dwell boundary trace:
                                    Status = String.Format("Building PolyLine ...\n");
                                    WaveformBuilder.BuildPolyTrace(bw, false, bleachPowerVal);
                                    break;
                                case "Ellipse":
                                    //Dwell during contour fill:
                                    Status = String.Format("Building Ellipse ...\n");
                                    WaveformBuilder.BuildEllipse(bw, bleachPowerVal);
                                    break;
                                default:
                                    break;
                            }

                            lastIteration = ((bw.Iterations - 1) == it) ? true : false;
                            cycleEnd = (lastEpoch && lastIteration && lastPattern) ? true : false;

                            //PostIdle:
                            if (!bw.PixelMode)
                                WaveformBuilder.BuildPostIdle(bw);

                            //pattern post-idle:
                            if (lastIteration)
                                WaveformBuilder.BuildPostPatIdle(bw, lastIteration && lastPattern, cycleEnd);

                            //NI limit data length per channel or stop request:
                            if ((WaveformBuilder.GetWaveform().Count > Int32.MaxValue) || (true == waveformBkWkr.CancellationPending))
                            {
                                ret = false;
                                break;
                            }
                        }
                        if (ret == false)
                        { break; }
                    }
                }
                //Return to start position & signal cycle completed:
                WaveformBuilder.ReturnHome(true);

                //***   Cycle End    ***//

                //Verify memory availability:
                RequiredSize = (Decimal)(WaveformBuilder.GetWaveform().Count * 4 * sizeof(double));

                //false if it requires more than memory:
                if ((ret == false) || (WaveformBuilder.GetWaveform().Count > Int32.MaxValue))   //|| (((double)RequiredSize / KILOBYTES_TO_GIGABYTES) > ((double)MemorySize))
                {
                    vm.DeleteFile(ROIPath + "BleachWaveform.raw");
                    return false;
                }
            }
            return true;
        }

        private void cbClkRate_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            switch (this.cbClkRate.SelectedIndex)
            {
                case 0:
                    WaveformBuilder.ClkRate = (int)ClockRate.HIGH;
                    break;
                case 1:
                    WaveformBuilder.ClkRate = (int)ClockRate.INTER_HIGH;
                    break;
                case 2:
                    WaveformBuilder.ClkRate = (int)ClockRate.MEDIUM;
                    break;
                case 3:
                    WaveformBuilder.ClkRate = (int)ClockRate.INTER_LOW;
                    break;
                case 4:
                    WaveformBuilder.ClkRate = (int)ClockRate.LOW;
                    break;
                default:
                    Status += String.Format("Please select clock rate.");
                    return;
            }
        }

        void EditBleachWaveform_Loaded(object sender, RoutedEventArgs e)
        {
            this.CanvasSpinProgress.Visibility = Visibility.Collapsed;

            PrepareEditBleach();

            //set flag so not delete file at exit:
            Result = true;
        }

        void EditBleachWaveform_Unloaded(object sender, RoutedEventArgs e)
        {
            this.Loaded -= new RoutedEventHandler(EditBleachWaveform_Loaded);
            this.Unloaded -= new RoutedEventHandler(EditBleachWaveform_Unloaded);
        }

        private void EnableItems(bool enable)
        {
            cbClkRate.IsEnabled = enable;
            dgLines.IsEnabled = enable;
            btnGen.IsEnabled = enable;
            txPxDen.IsEnabled = enable;
            ckGenIdv.IsEnabled = enable;
            //chkPxDen.IsEnabled = enable;
            if (true == enable)
            {
                //txPxDen.IsEnabled = (bool)chkPxDen.IsChecked;
                txLongIdle.IsEnabled = (bool)ckGenIdv.IsChecked;
            }
        }

        private void PixelDensity_Checked(object sender, RoutedEventArgs e)
        {
            ///disabled:
            //this.lblClkRate.Visibility = (true == chkPxDen.IsChecked) ? Visibility.Visible : Visibility.Collapsed;
            //this.cbClkRate.Visibility = (true == chkPxDen.IsChecked) ? Visibility.Collapsed : Visibility.Visible;
            //this.txPxDen.IsEnabled = (true == chkPxDen.IsChecked) ? true : false;
            //BindingExpression be = this.txPxDen.GetBindingExpression(TextBox.TextProperty);
            //be.UpdateSource();
        }

        private void PrepareEditBleach()
        {
            if (vm.BleachParamList.Count <= 0)
            {
                return;
            }

            foreach (BleachWaveParams bwparams in vm.BleachParamList)
            {
                if (0.0 < bwparams.LongIdleTime)
                {
                    vm.PixelUnitMode = (0.0 < bwparams.LongIdleTime) ? true : false;
                    vm.PixelLongIdleTime = (vm.PixelUnitMode) ? bwparams.LongIdleTime : 0.0;
                }
                vm.PreCycleIdleTime = bwparams.PreCycleIdleMS;
                vm.PostCycleIdleTime = bwparams.PostCycleIdleMS;
                vm.PreEpochIdleTime = bwparams.PreEpochIdleMS;
                vm.PostEpochIdleTime = bwparams.PostEpochIdleMS;
                vm.BleachEpoches = bwparams.EpochCount;
            }

            //clock rate:
            switch (WaveformBuilder.ClkRate)
            {
                //case (int)ClockRate.HIGH:
                //    this.chkPxDen.IsChecked = false;
                //    this.cbClkRate.SelectedIndex = 0;
                //    break;
                //case (int)ClockRate.INTER_HIGH:
                //    this.chkPxDen.IsChecked = false;
                //    this.cbClkRate.SelectedIndex = 1;
                //    break;
                //case (int)ClockRate.MEDIUM:
                //    this.chkPxDen.IsChecked = false;
                //    this.cbClkRate.SelectedIndex = 2;
                //    break;
                //case (int)ClockRate.INTER_LOW:
                //    this.chkPxDen.IsChecked = false;
                //    this.cbClkRate.SelectedIndex = 3;
                //    break;
                //case (int)ClockRate.LOW:
                //    this.chkPxDen.IsChecked = false;
                //    this.cbClkRate.SelectedIndex = 4;
                //    break;
                default:
                    this.chkPxDen.IsChecked = true;
                    vm.PixelDensity = (int)(vm.BleachLSMPixelXY[0] * WaveformBuilder.MS_TO_S / WaveformBuilder.ClkRate);
                    BindingExpression be = this.lblClkRate.GetBindingExpression(Label.ContentProperty);
                    be.UpdateSource();
                    //ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, "No bleach clock rate is available");
                    break;
            }
            //Memory Info:
            Status = String.Format("RAM ~{0} Gb available.\n", Decimal.Round(MemorySize, 2));
        }

        private bool UpdateBleachWaveParams(int clkRate)
        {
            //update local bleach param list:

            for (int idx = 0; idx < vm.BleachParamList.Count; idx++)
            {
                if (vm.BleachParamList[idx].DwellTime < WaveformBuilder.MinDwellTime)
                {
                    Status += String.Format("Minimum dwell time is {0}us, increase dwell time or change clock rate.\n", Decimal.Round((decimal)WaveformBuilder.MinDwellTime, 2));
                    return false;
                }
                vm.BleachParamList[idx].ClockRate = clkRate;
                vm.BleachParamList[idx].LongIdleTime = (vm.PixelUnitMode) ? vm.PixelLongIdleTime : 0.0;
                vm.BleachParamList[idx].Power = bleachPowerVal;
                vm.BleachParamList[idx].PreCycleIdleMS = vm.PreCycleIdleTime;
                vm.BleachParamList[idx].PostCycleIdleMS = vm.PostCycleIdleTime;
                vm.BleachParamList[idx].PreEpochIdleMS = vm.PreEpochIdleTime;
                vm.BleachParamList[idx].PostEpochIdleMS = vm.PostEpochIdleTime;
                vm.BleachParamList[idx].EpochCount = vm.BleachEpoches;
            }
            return true;
        }

        void waveformBkWkr_DoWork(object sender, DoWorkEventArgs e)
        {
            BackgroundWorker worker = sender as BackgroundWorker;
            bool built = false;
            if (worker.CancellationPending != true)
            {
                //Try build waveform:
                built = BuildWaveform(WaveformBuilder.ClkRate);
            }
            //Save to H5:
            if (!vm.PixelUnitMode)
            {
                if ((WaveformBuilder.GetWaveform().Count <= 0) || (false == built))
                {
                    return;
                }
                Status = String.Format("Saving Waveform ...\n");

                string filePathName = ROIPath + "BleachWaveform.raw";

                WaveformBuilder.SaveWaveform(filePathName, true);

                while (!WaveformBuilder.CheckSaveState())
                {
                    System.Threading.Thread.Sleep(50);

                    if (true == waveformBkWkr.CancellationPending)
                    {
                        WaveformBuilder.StopSave();
                    }
                }

                H5Saved = WaveformBuilder.GetSaveResult();
            }
            //Persist Params:
            if ((vm.PixelUnitMode && vm.PixelLongIdleTime > 0) || (H5Saved))
            {
                if (!vm.SaveBleachWaveParams(_roiPathandName))
                {
                    Status = String.Format("Error at saving bleach ROIs.\n ");
                    Result = false;
                }
            }
        }

        void waveformBkWkr_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            ActionEnableItems(true);
            this.CanvasSpinProgress.Visibility = Visibility.Collapsed;
            if (e.Cancelled == true)
            {
                Status = String.Format("Cancelled Waveform Generation.\n ");
                Result = false;
            }
            else
            {
                Status = String.Format("RAM ~{0} Gb available.\n ", Decimal.Round(MemorySize, 2));
                //Status += String.Format(", and RAM ~{0} Mb Required.\n", Decimal.Round((Decimal)((double)RequiredSize / MEGABYTES_TO_GIGABYTES), 0));
                if (Double.IsNaN(WaveformBuilder.DeltaX_Px))
                {
                    Status += String.Format("Invalid Galvo calibration settings; Waveform preparation: FAILED.\n");
                    Result = false;
                }
                else
                {
                    if (vm.PixelUnitMode)
                    {
                        if (vm.PixelLongIdleTime > 0)
                        {
                            Status += String.Format("Unit step size is {0} pixels; Total time per cycle is {1} ms.\nWaveform preparation: SUCCESS.\n",
                                Decimal.Round((Decimal)WaveformBuilder.DeltaX_Px, 1), Decimal.Round((Decimal)((double)RequiredSize * WaveformBuilder.MS_TO_S / 4 / sizeof(double) / WaveformBuilder.ClkRate), 2));
                            Result = true;
                        }
                        else
                        {
                            Status += String.Format("Invalid time setting for long term idle; Waveform preparation: FAILED.\n");
                            Result = false;
                        }
                    }
                    else
                    {
                        if ((WaveformBuilder.GetWaveform().Count > Int32.MaxValue)) //(((double)RequiredSize / KILOBYTES_TO_GIGABYTES) > ((double)MemorySize)) ||
                        {
                            Status += String.Format("Waveform generation: FAILED due to limited memory space.\n");
                            Result = false;
                        }
                        else
                        {
                            //Save to H5:
                            if (H5Saved)
                            {
                                Status += String.Format("Unit step size is {0} pixels; Total time per cycle is {1} ms.\nWaveform generation: SUCCESS.\n",
                                    Decimal.Round((Decimal)WaveformBuilder.DeltaX_Px, 1), Decimal.Round((Decimal)((double)RequiredSize * WaveformBuilder.MS_TO_S / 4 / sizeof(double) / WaveformBuilder.ClkRate), 3));
                                Result = true;
                            }
                            else
                            {
                                Status += String.Format("Waveform file is NOT updated.\n");
                                Result = false;
                            }
                        }
                    }
                }
            }

            waveformBkWkr.DoWork -= new DoWorkEventHandler(waveformBkWkr_DoWork);
            waveformBkWkr.RunWorkerCompleted -= new RunWorkerCompletedEventHandler(waveformBkWkr_RunWorkerCompleted);
        }

        #endregion Methods

        #region Nested Types

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Auto)]
        private class MEMORYSTATUSEX
        {
            public uint dwLength;
            public uint dwMemoryLoad;
            public ulong ullTotalPhys;
            public ulong ullAvailPhys;
            public ulong ullTotalPageFile;
            public ulong ullAvailPageFile;
            public ulong ullTotalVirtual;
            public ulong ullAvailVirtual;
            public ulong ullAvailExtendedVirtual;

            #region Constructors

            public MEMORYSTATUSEX()
            {
                this.dwLength = (uint)Marshal.SizeOf(typeof(MEMORYSTATUSEX));
            }

            #endregion Constructors
        }

        #endregion Nested Types
    }
}