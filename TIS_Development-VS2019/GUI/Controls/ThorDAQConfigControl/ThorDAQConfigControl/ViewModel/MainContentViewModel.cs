
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Management;
using System.Windows;
using System.Windows.Input;
using System.Windows.Threading;
using Telerik.Windows.Data;
using ThorDAQConfigControl.Controls;
using ThorDAQConfigControl.Model;
using ThorSharedTypes;

namespace ThorDAQConfigControl.ViewModel
{
    public class MainContentViewModel : ViewModelBase
    {
        private DispatcherTimer updateBobStatusTimer;
        private const int TimerInterval1S = 1000;

        public ThorDAQCommandProvider commandProvider;
        public AnalogIOSimulator AIOSimulator;
        public DigitalIOSimulator DIOSimulator;
        private DispatcherTimer aiTimer; // get input value in analog input tab
        private const int TimerInterval = 200;

        public List<BOBItem> AOBobList, AIBobList, DCBobList;
        public int AICount = 14;
        public int AOCount = 12;
        public int DCCount = 32;

        private ICommand startStopAICommand;
        public ICommand StartStopAICommand
        {
            get
            {
                if (this.startStopAICommand == null)
                    this.startStopAICommand = new RelayCommand(() => OnStartStopAI());

                return this.startStopAICommand;
            }
        }

        private ICommand setAnalogOutputCommand;
        public ICommand SetAnalogOutputCommand
        {
            get
            {
                if (this.setAnalogOutputCommand == null)
                    this.setAnalogOutputCommand = new RelayCommand(() => OnSetAnalogOutput());

                return this.setAnalogOutputCommand;
            }
        }

        #region Properties

        private ObservableCollection<string> consoleOutput = new ObservableCollection<string>();
        public ObservableCollection<string> ConsoleOutput
        {
            get => consoleOutput;
            set
            {
                consoleOutput = value;
                OnPropertyChanged("ConsoleOutput");
            }
        }

        // Buttons
        private float parkValue = 0;
        public float ParkValue
        {
            get => parkValue;
            set
            {
                if (value == parkValue)
                    return;

                parkValue = value;
                OnPropertyChanged("ParkValue");
            }
        }

        // Test Panel
        #region 1. Analog Input

        private int aiChannelIndex = 0;
        public int AIChannelIndex
        {
            get => aiChannelIndex;
            set
            {
                if (value == aiChannelIndex)
                    return;

                aiChannelIndex = value;
                OnPropertyChanged("AIChannelIndex");

                // Clear the chart
                if ((AIDataPoints != null) && (AIDataPoints.Count > 100))
                {
                    AIDataPoints.Clear();
                    AIDataPoints.ResumeNotifications();
                }
            }
        }

        // 0 - On Demand, 1-Finite, 2-Continuous
        private int aiModeIndex = 0;
        public int AIModeIndex
        {
            get => aiModeIndex;
            set
            {
                if (value == aiModeIndex)
                    return;

                aiModeIndex = value;
                OnPropertyChanged("AIModeIndex");
                OnPropertyChanged("IsFiniteOrContinuous");
                OnPropertyChanged("ShowFiniteProcessBar");
            }
        }

        public bool IsFiniteOrContinuous
        {
            get => AIModeIndex == 1 || AIModeIndex == 2;
        }

        public bool ShowFiniteProcessBar
        {
            get => IsFiniteOrContinuous && IsSimulating;
        }

        private int aiFiniteModePercent;
        public int AIFiniteModePercent
        {
            get => aiFiniteModePercent;
            set
            {
                if (aiFiniteModePercent == value) return;

                aiFiniteModePercent = value;
                OnPropertyChanged("AIFiniteModePercent");
            }
        }

        private double aiMaxLimit = 0;
        public double AIMaxLimit
        {
            get => aiMaxLimit;
            set
            {
                if (value == aiMaxLimit)
                    return;

                aiMaxLimit = value;
                OnPropertyChanged("AIMaxLimit");
            }
        }

        private double aiMinLimit = 0;
        public double AIMinLimit
        {
            get => aiMinLimit;
            set
            {
                if (value == aiMinLimit)
                    return;

                aiMinLimit = value;
                OnPropertyChanged("AIMinLimit");
            }
        }

        private int aiRate = 0;
        public int AIRate
        {
            get => aiRate;
            set
            {
                if (value == aiRate)
                    return;

                aiRate = value;
                OnPropertyChanged("AIRate");
            }
        }

        private int aiSamplesToRead = 0;
        public int AISamplesToRead
        {
            get => aiSamplesToRead;
            set
            {
                if (value == aiSamplesToRead)
                    return;

                aiSamplesToRead = value;
                OnPropertyChanged("AISamplesToRead");
            }
        }

        private bool isSimulating = false;
        public bool IsSimulating
        {
            get => isSimulating;
            set
            {
                if (value == isSimulating)
                    return;

                isSimulating = value;
                OnPropertyChanged("IsSimulating");
                OnPropertyChanged("ShowFiniteProcessBar");
            }
        }

        private int tempRound;
        private List<LiveChartPoint> tempDataPoints;

        private RadObservableCollection<LiveChartPoint> aiDataPoints;
        public RadObservableCollection<LiveChartPoint> AIDataPoints
        {
            get => aiDataPoints;
            set
            {
                if (aiDataPoints == value)
                    return;

                aiDataPoints = value;
                OnPropertyChanged("AIDataPoints");
            }
        }

        private double axisXStartValue;
        public double AxisXStartValue
        {
            get => axisXStartValue;
            set
            {
                if (value == axisXStartValue)
                    return;

                axisXStartValue = value;
                OnPropertyChanged("AxisXStartValue");
            }
        }

        private double axisXEndValue;
        public double AxisXEndValue
        {
            get => axisXEndValue;
            set
            {
                if (value == axisXEndValue)
                    return;

                axisXEndValue = value;
                OnPropertyChanged("AxisXEndValue");
            }
        }

        /*
        private List<ScatterDataPoint> aiDataPoints;
        public List<ScatterDataPoint> AIDataPoints
        {
            get => aiDataPoints;
            set
            {
                if (aiDataPoints == value) 
                    return;

                OnPropertyChanged("aiDataPoints, value);
            }
        }
        */

        private double analogChartValue;
        public double AnalogChartValue
        {
            get => analogChartValue;
            set
            {
                if (value == analogChartValue)
                    return;

                analogChartValue = value;
                OnPropertyChanged("AnalogChartValue");
            }
        }
        #endregion

        #region 2. Analog Output

        private int aoChannelIndex = 0;
        public int AOChannelIndex
        {
            get => aoChannelIndex;
            set
            {
                if (value == aoChannelIndex)
                    return;

                aoChannelIndex = value;
                OnPropertyChanged("AOChannelIndex");
            }
        }

        // 0 - Voltage DC,
        private int aoModeIndex = 0;
        public int AOModeIndex
        {
            get => aoModeIndex;
            set
            {
                if (value == aoModeIndex)
                    return;

                aoModeIndex = value;
                OnPropertyChanged("AOModeIndex");
            }
        }

        // 0 - Default,
        private int aoTransferIndex = 0;
        public int AOTransferIndex
        {
            get => aoTransferIndex;
            set
            {
                if (value == aoTransferIndex)
                    return;

                aoTransferIndex = value;
                OnPropertyChanged("AOTransferIndex");
            }
        }

        private double aoMaxLimit = 0;
        public double AOMaxLimit
        {
            get => aoMaxLimit;
            set
            {
                if (value == aoMaxLimit)
                    return;

                aoMaxLimit = value;
                OnPropertyChanged("AOMaxLimit");
                OnPropertyChanged("AOTickFrequency");
            }
        }

        private double aoMinLimit = 0;
        public double AOMinLimit
        {
            get => aoMinLimit;
            set
            {
                if (value == aoMinLimit)
                    return;

                aoMinLimit = value;
                OnPropertyChanged("AOMinLimit");
                OnPropertyChanged("AOTickFrequency");
            }
        }

        private int aoRate = 0;
        public int AORate
        {
            get => aoRate;
            set
            {
                if (value == aoRate)
                    return;

                aoRate = value;
                OnPropertyChanged("AORate");
            }
        }

        private double aoValue = 0;
        public double AOValue
        {
            get => aoValue;
            set
            {
                if (value == aoValue)
                    return;

                aoValue = value;
                OnPropertyChanged("AOValue");
            }
        }

        public double AOTickFrequency
        {
            get => (AOMaxLimit - AOMinLimit) / 10;
        }
        #endregion

        #region 3. Digital I/O
        private int diChannelIndex = 0;
        public int DIChannelIndex
        {
            get => diChannelIndex;
            set
            {
                if (value == diChannelIndex)
                    return;

                diChannelIndex = value;
                OnPropertyChanged("DIChannelIndex");
            }
        }

        #endregion

        // Bob Panel
        public bool IsBobPanelShown = false;

        private bool isBobConnected = false;
        public bool IsBobConnected
        {
            get => isBobConnected;
            set
            {
                if (value == isBobConnected)
                    return;

                isBobConnected = value;
                OnPropertyChanged("IsBobConnected");
            }
        }

        private bool isScanEnabled = false;
        public bool IsScanEnabled
        {
            get => isScanEnabled;
            set
            {
                if (value == isScanEnabled)
                    return;

                isScanEnabled = value;
                OnPropertyChanged("IsScanEnabled");
            }
        }

        private bool isDC0 = false;
        public bool IsDC0
        {
            get => isDC0;
            set
            {
                if (value == isDC0)
                    return;

                isDC0 = value;
                OnPropertyChanged("IsDC0");
            }
        }

        private bool isDC1 = false;
        public bool IsDC1
        {
            get => isDC1;
            set
            {
                if (value == isDC1)
                    return;

                isDC1 = value;
                OnPropertyChanged("IsDC1");
            }
        }

        private bool isDC2 = false;
        public bool IsDC2
        {
            get => isDC2;
            set
            {
                if (value == isDC2)
                    return;

                isDC2 = value;
                OnPropertyChanged("IsDC2");
            }
        }

        private bool isDC3 = false;
        public bool IsDC3
        {
            get => isDC3;
            set
            {
                if (value == isDC3)
                    return;

                isDC3 = value;
                OnPropertyChanged("IsDC3");
            }
        }

        #endregion

        private static MainContentViewModel instance;
        public static MainContentViewModel GetInstance()
        {
            if (instance == null)
                instance = new MainContentViewModel();
            return instance;
        }

        public MainContentViewModel()
        {
            ConsoleOutput.Add("<<< ThorDAQ CL-GUI Console  \n<<< Copyright 2016-2021 Thorlabs Imaging Systems Research Group");

            InitTestPanel();
            InitBobPanel();

            updateBobStatusTimer = new DispatcherTimer();
            updateBobStatusTimer.Interval = TimeSpan.FromMilliseconds(TimerInterval1S);
            updateBobStatusTimer.Tick += OnUpdateBobStatusTimer;
            //updateBobStatusTimer.Start();

            IsSimulating = false;
            aiTimer = new DispatcherTimer();
            aiTimer.Interval = TimeSpan.FromMilliseconds(TimerInterval);
            aiTimer.Tick += OnAITimer;
        }

        ~MainContentViewModel()
        {
            if (IsSimulating)
            {
                AIOSimulator.StopSimulating();
                IsSimulating = false;
                aiTimer.Stop();
            }
        }

        public void SetCommandProvider(ThorDAQCommandProvider cmdProvider)
        {
            commandProvider = cmdProvider;
        }

        private void InitTestPanel()
        {
            AIOSimulator = new AnalogIOSimulator();
            DIOSimulator = new DigitalIOSimulator();

            // Analog Input
            AIChannelIndex = 0;
            AIModeIndex = 0;
            AIMaxLimit = 10;
            AIMinLimit = -10;
            AIRate = 1000;
            AISamplesToRead = 1000;
            // Analog Output
            AOMaxLimit = 10;
            AOMinLimit = -10;
            AORate = 1000;
            // Digital IO
            DIChannelIndex = 0;
            // Counter IO

            tempDataPoints = new List<LiveChartPoint>();
            AIDataPoints = new RadObservableCollection<LiveChartPoint>();

            // just for test

            //var tempList = new List<ScatterDataPoint>();
            //for (int i = 0; i < 100; i++)
            //    AIDataPoints.Add(new ScatterDataPoint() { XValue = i, YValue = 8*i/100 });
            //AIDataPoints = tempList;

        }

        private void InitBobPanel()
        {
            AOBobList = new List<BOBItem>();
            AIBobList = new List<BOBItem>();
            DCBobList = new List<BOBItem>();

            IsBobConnected = true; // test
            IsScanEnabled = false;
            IsDC0 = true;
            IsDC1 = false;
            IsDC2 = false;
            IsDC3 = false;
        }

        private void OnUpdateBobStatusTimer(object sender, EventArgs e)
        {
            if (!IsBobPanelShown)
                return;

            // No way to get real time led status
            /*
            List<string> bobStatusList = new List<string>();
            var ret = commandProvider.APIGetBOBstatus(bobStatusList); // The api is not fit
            if (ret)
            {
                // Update BOB status panel
                if (bobStatusList != null && bobStatusList.Count > 6)
                {
                    var status = bobStatusList[5];
                    char[] statusArray = status.ToCharArray();
                    for (int i = 0; i < statusArray.Length; i++)
                    {
                        if (statusArray[i] == '0')
                            DCBobList[i].TurnOff();
                        else
                            DCBobList[2].TurnOn();
                    }
                }
            }
            */
        }

        int pointCount = 0;
        double currentValue = 0;
        // Get new data and add to display
        private void OnAITimer(object sender, EventArgs e)
        {
            switch (AIModeIndex)
            {
                case 0: // On Demand
                    if (GetAIValue(AIChannelIndex, ref currentValue))
                    {
                        AnalogChartValue = Math.Round(currentValue, 2);
                        AIDataPoints.SuspendNotifications();

                        // Only show latest 100 points
                        if (AIDataPoints.Count > 100)
                        {
                            AIDataPoints.RemoveAt(0);
                            AxisXStartValue = AIDataPoints[0].XValue;
                            AxisXEndValue = AxisXStartValue + 100;
                        }

                        AIDataPoints.Add(new LiveChartPoint() { XValue = pointCount++, YValue = AnalogChartValue });
                        AIDataPoints.ResumeNotifications();
                    }
                    break;
                case 1: // Finite
                    if (GetAIValue(AIChannelIndex, ref currentValue))
                    {
                        AnalogChartValue = Math.Round(currentValue, 2);
                        tempDataPoints.Add(new LiveChartPoint() { XValue = pointCount++, YValue = AnalogChartValue });
                        AIFiniteModePercent = pointCount * 100 / AISamplesToRead;
                    }
                    // Only show data after reaching the AISamplesToRead
                    if (pointCount >= AISamplesToRead)
                    {
                        AIDataPoints.SuspendNotifications();
                        AIDataPoints.AddRange(tempDataPoints);
                        AIDataPoints.ResumeNotifications();
                        tempDataPoints.Clear();
                        aiTimer.Stop();
                        IsSimulating = false;
                    }
                    break;
                case 2: // Continuous
                    var startXValue = tempRound * AISamplesToRead;
                    if (GetAIValue(AIChannelIndex, ref currentValue))
                    {
                        AnalogChartValue = Math.Round(currentValue, 2);
                        tempDataPoints.Add(new LiveChartPoint() { XValue = startXValue + (pointCount++), YValue = AnalogChartValue });
                        AIFiniteModePercent = pointCount * 100 / AISamplesToRead;
                    }
                    if (pointCount == AISamplesToRead)
                    {
                        AxisXStartValue = startXValue;
                        AxisXEndValue = startXValue + AISamplesToRead;
                        AIDataPoints.SuspendNotifications();
                        AIDataPoints.Clear();
                        AIDataPoints.AddRange(tempDataPoints);
                        AIDataPoints.ResumeNotifications();
                        tempDataPoints.Clear();
                        tempRound++;
                        pointCount = 0;
                    }
                    break;
            }
        }

        private void OnStartStopAI()
        {
            if (AIMinLimit >= AIMaxLimit)
            {
                MessageBox.Show("Invalid Min/Max input limit");
                return;
            }

            if (IsSimulating)
            {
                aiTimer.Stop();
                tempDataPoints.Clear();
                tempRound = 0;
            }
            else
            {
                pointCount = 0;
                AIDataPoints.Clear();
                AIDataPoints.ResumeNotifications();
                AxisXStartValue = 0;

                if (AIModeIndex == 0) // On Demand
                {
                    AxisXEndValue = 100;
                    aiTimer.Interval = TimeSpan.FromMilliseconds(TimerInterval);
                }
                else // Finite & Continuous
                {
                    AxisXEndValue = AISamplesToRead;
                    aiTimer.Interval = TimeSpan.FromMilliseconds(1000 / AIRate);
                }
                aiTimer.Start();
            }
            IsSimulating = !IsSimulating;
        }

        public bool GetAIValue(int index, ref double value)
        {
            var result = commandProvider.APIGetAI(new List<string> { "apigetai", "-" + index.ToString()}, false);

            if (double.TryParse(result, out double val))
            {
                value = val;
                return true;
            }
            else
                return false;
        }

        private void OnSetAnalogOutput()
        {
            SetAOValue(AOValue);
        }

        public void SetAOValue(double? value)
        {
            if (value != null)
                SetAOValue(AOChannelIndex, (double)value);
        }

        public void SetAOValue(int index, double value)
        {
            commandProvider.SetDAC_ParkValue(new List<string> { "SetParkValue", "-c", index.ToString(), "-v", Math.Round(value, 1) + "" });
        }

        // query Windows O/S for existence of ThorDAQ board(s)
        public int CountOfThorDAQboards()
        {
            int iRetCode = 0; // no boards

            // Get the installed kernel device driver date stamp (from .INF, not from driver code)
            // This stamp is typically done from Visual Studio build, stampinf step, where base .INF file variable has "Driver = " (null)
            // and this stamp is far more reliable indicating any change to driver (including packaging)
            ManagementObjectSearcher objSearcher = new ManagementObjectSearcher("Select * from Win32_PnPSignedDriver");
            ManagementObjectCollection objCollection = objSearcher.Get();

            foreach (ManagementObject obj in objCollection)
            {
                string Manuf = String.Format("{0}", obj["Manufacturer"]);
                if (Manuf.StartsWith("ThorLabs") || Manuf.StartsWith("Thorlabs")) // older Win7 driver is "Thorlabs"
                {
                    string ThorDAQdevice = String.Format("{0}", obj["DeviceName"]);    // e.g. ThorDAQ-2586
                    if (ThorDAQdevice.StartsWith("ThorDAQ"))
                    {
                        iRetCode++;
                        string sBuf = String.Format("{0}", obj["DriverDate"]);
                        string pDate = String.Format("{1}/{2}/{0}", sBuf.Substring(0, 4), sBuf.Substring(4, 2), sBuf.Substring(6, 2));
                        string info = String.Format("{0}, Driver Date: {1}  Version: {2}", obj["DeviceName"], pDate, obj["DriverVersion"]);
                        ConsoleOutput.Add("<<< " + info);
                    }
                }
            }
            return iRetCode;
        }
    }
}