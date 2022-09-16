namespace PowerControl.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Reflection;
    using System.Runtime.CompilerServices;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Input;
    using System.Xml;

    using PowerControl.Model;

    using ThorSharedTypes;

    public class PowerControlViewModel : ThorSharedTypes.VMBase, ThorSharedTypes.IMVM
    {
        #region Fields

        private const int MAX_POWER_RAMP_DATA_POINTS = 100;
        const double MAX_STEP_SIZE = 10;
        const double MIN_STEP_SIZE = .0001;

        private readonly PowerControlModel _powerControlModel;

        private ObservableCollection<int> _bleacherPockelsList = new ObservableCollection<int>();
        private DateTime _lastPowerReg2UpdateTime;
        private DateTime _lastPowerUpdateTime = DateTime.Now;
        private Visibility _pockels0Visibility = Visibility.Collapsed;
        private Visibility _pockels1Visibility = Visibility.Collapsed;
        private Visibility _pockels2Visibility = Visibility.Collapsed;
        private Visibility _pockels3Visibility = Visibility.Collapsed;
        private ICommand _pockelsCalibrateAgainCommand;
        private bool _pockelsCalibrateAgainEnable = true;
        private KeyValuePair<XmlNodeList, XmlDocument> _pockelsNodeList = new KeyValuePair<XmlNodeList, XmlDocument>();
        private PockelsPlotWin _pockelsPlot = null;
        private double _power2StepSize = 1.0;
        private ICommand _powerCalibrateCommand;
        private ICommand _powerClearPlotCommand;
        private ICommand _powerControlNameSaveCommand;
        private double[] _powerGo = new double[PowerControlModel.MAX_POWER_CTRLS];
        private ICommand _powerMinusCommand;
        private string[] _powerMinusKey = new string[PowerControlModel.MAX_POWER_CTRLS];
        private string[] _powerMinusModifier = new string[PowerControlModel.MAX_POWER_CTRLS];
        private int[] _powerMode = new int[PowerControlModel.MAX_POWER_CTRLS];
        private PowerPlotWin _powerPlot = null;
        private ICommand _powerPlusCommand;
        private string[] _powerPlusKey = new string[PowerControlModel.MAX_POWER_CTRLS];
        private string[] _powerPlusModifier = new string[PowerControlModel.MAX_POWER_CTRLS];
        private ICommand _powerRampAddCommand;
        private ICommand _powerRampDeleteCommand;
        private ICommand _powerRampEditCommand;
        private ObservableCollection<string> _powerRampsCustom = new ObservableCollection<string>();
        private int _powerRampSelected0 = 0;
        private int _powerRampSelected1 = 0;
        private int _powerRampSelected2 = 0;
        private int _powerRampSelected3 = 0;
        private int _powerRampSelectedReg = 0;
        private int _powerRampSelectedReg2 = 0;
        private ICommand _powerRecordPlotCommand;
        private double _powerReg2 = 0.0;
        private ICommand _powerReg2CalCommand;
        private string _powerReg2CalName1;
        private string _powerReg2CalName2;
        private string _powerReg2CalName3;
        private string _powerReg2CalName4;
        private string _powerReg2CalName5;
        private string _powerReg2CalName6;
        private ICommand _powerReg2CalSaveCommand;
        private Visibility _powerReg2Visibility = Visibility.Collapsed;
        private string _powerRegCal1Name;
        private string _powerRegCal2Name;
        private string _powerRegCal3Name;
        private string _powerRegCal4Name;
        private string _powerRegCal5Name;
        private string _powerRegCal6Name;
        private ICommand _powerRegCalCommand;
        private double _powerRegCalOffset;
        private ICommand _powerRegCalSaveCommand;
        private Visibility _powerRegVisibility = Visibility.Collapsed;
        private double _powerStepSize = 1.0;
        private Dictionary<string, PropertyInfo> _properties = new Dictionary<string, PropertyInfo>();
        private int _selectedPowerTab = 0;
        private ICommand _selectPockelsMaskCommand;
        private ICommand _setBleacherPowerCommand;
        private ICommand _setBleacherPowerToZeroCommand;
        private ICommand _setPowerCommand;
        private ICommand _stepCoarseCommand;
        private ICommand _stepFineCommand;
        private ICommand _updatePockelsMaskToROIMaskCommand;

        #endregion Fields

        #region Constructors

        public PowerControlViewModel()
        {
            this._powerControlModel = new PowerControlModel();

            PowerControlName = new ObservableCollection<StringPC>();
            PockelsPowerThreshold = new ObservableCollection<DoublePC>();

            const int MAX_POWER_CONTROLS = 6;
            for (int i = 0; i < MAX_POWER_CONTROLS; i++)
            {
                PowerControlName.Add(new StringPC());
                PockelsPowerThreshold.Add(new DoublePC());
            }
        }

        #endregion Constructors

        #region Properties

        public ObservableCollection<int> BleacherPockelsList
        {
            get { return _bleacherPockelsList; }
            set
            {
                _bleacherPockelsList = value;
                OnPropertyChanged("BleacherPockelsList");
            }
        }

        public double BleacherPower0
        {
            get
            {
                return this._powerControlModel.BleacherPower0;
            }
            set
            {
                this._powerControlModel.BleacherPower0 = value;
            }
        }

        public double BleacherPowerGo
        {
            get;
            set;
        }

        public int BleacherPowerID
        {
            get;
            set;
        }

        public PockelsResponseType BleacherPowerResponse0
        {
            get { return this._powerControlModel.BleacherPowerResponse0; }
        }

        public PockelsResponseType BleacherPowerResponse1
        {
            get { return this._powerControlModel.BleacherPowerResponse1; }
        }

        public double BleachPockelsVoltageMax0
        {
            get
            {
                return this._powerControlModel.BleachPockelsVoltageMax0;
            }
        }

        public double BleachPockelsVoltageMin0
        {
            get
            {
                return this._powerControlModel.BleachPockelsVoltageMin0;
            }
        }

        public bool ClosePropertyWindows
        {
            set
            {
                if (value)
                {
                    if (null != _pockelsPlot)
                    {
                        _pockelsPlot.Close();
                    }
                    if (null != _powerPlot)
                    {
                        _powerPlot.Close();
                    }
                }
            }
        }

        public double CustomStartPower
        {
            get
            {
                if (0 < PowerRampPlotY.Length)
                {
                    return PowerRampPlotY[0];
                }
                else
                {
                    return 0.0;
                }
            }
        }

        public int EnablePockelsMask
        {
            get
            {
                return this._powerControlModel.EnablePockelsMask;
            }
            set
            {

                this._powerControlModel.EnablePockelsMask = value;
                OnPropertyChanged("EnablePockelsMask");
                OnPropertyChanged("PockelsMaskOptionsAvailable");
            }
        }

        public int IsPockelsCalibrationFailed
        {
            get;
            set;
        }

        public Visibility Pockels0Visibility
        {
            get
            {
                return _pockels0Visibility;
            }
            set
            {
                _pockels0Visibility = value;
                OnPropertyChanged("Pockels0Visibility");
            }
        }

        public Visibility Pockels1Visibility
        {
            get
            {
                return _pockels1Visibility;
            }
            set
            {
                _pockels1Visibility = value;
                OnPropertyChanged("Pockels1Visibility");
            }
        }

        public Visibility Pockels2Visibility
        {
            get
            {
                return _pockels2Visibility;
            }
            set
            {
                _pockels2Visibility = value;
                OnPropertyChanged("Pockels2Visibility");
            }
        }

        public Visibility Pockels3Visibility
        {
            get
            {
                return _pockels3Visibility;
            }
            set
            {
                _pockels3Visibility = value;
                OnPropertyChanged("Pockels3Visibility");
            }
        }

        public int PockelsActiveIndex
        {
            get;
            set;
        }

        public double PockelsActiveVoltageMax
        {
            get
            {
                double val = 0;
                switch (PockelsActiveIndex)
                {
                    case 0: val = this.PockelsVoltageMax0; break;
                    case 1: val = this.PockelsVoltageMax1; break;
                    case 2: val = this.PockelsVoltageMax2; break;
                    case 3: val = this.PockelsVoltageMax3; break;
                }
                return (IsPockelsCalibrationFailed >= 0) ? val : PockelsActiveVoltageMin;
            }
        }

        public double PockelsActiveVoltageMin
        {
            get
            {
                double val = 0;
                switch (PockelsActiveIndex)
                {
                    case 0: val = this.PockelsVoltageMin0; break;
                    case 1: val = this.PockelsVoltageMin1; break;
                    case 2: val = this.PockelsVoltageMin2; break;
                    case 3: val = this.PockelsVoltageMin3; break;
                }
                return val;
            }
        }

        public double PockelsBlankingPhaseShiftPercent
        {
            get
            {
                return _powerControlModel.PockelsBlankingPhaseShiftPercent;
            }
            set
            {
                _powerControlModel.PockelsBlankingPhaseShiftPercent = value;
                OnPropertyChanged("PockelsBlankingPhaseShiftPercent");
            }
        }

        public Visibility PockelsBlankingPhaseShiftPercentVisibility
        {
            get
            {
                return _powerControlModel.PockelsBlankingPhaseShiftPercentAvailable ? Visibility.Visible : Visibility.Collapsed;
            }
            set
            {

            }
        }

        public int PockelsBlankPercentage0
        {
            get
            {
                return this._powerControlModel.PockelsBlankPercentage0;
            }
            set
            {
                this._powerControlModel.PockelsBlankPercentage0 = value;
                OnPropertyChanged("PockelsBlankPercentage0");
            }
        }

        public int PockelsBlankPercentage1
        {
            get
            {
                return this._powerControlModel.PockelsBlankPercentage1;
            }
            set
            {
                this._powerControlModel.PockelsBlankPercentage1 = value;
                OnPropertyChanged("PockelsBlankPercentage1");
            }
        }

        public int PockelsBlankPercentage2
        {
            get
            {
                return this._powerControlModel.PockelsBlankPercentage2;
            }
            set
            {
                this._powerControlModel.PockelsBlankPercentage2 = value;
                OnPropertyChanged("PockelsBlankPercentage2");
            }
        }

        public int PockelsBlankPercentage3
        {
            get
            {
                return this._powerControlModel.PockelsBlankPercentage3;
            }
            set
            {
                this._powerControlModel.PockelsBlankPercentage3 = value;
                OnPropertyChanged("PockelsBlankPercentage3");
            }
        }

        public ICommand PockelsCalibrateAgainCommand
        {
            get
            {
                if (this._pockelsCalibrateAgainCommand == null)
                    this._pockelsCalibrateAgainCommand = new ThorSharedTypes.RelayCommand(() => PockelsCalibrate(0));

                return this._pockelsCalibrateAgainCommand;
            }
        }

        public bool PockelsCalibrateAgainEnable
        {
            get
            {
                return _pockelsCalibrateAgainEnable;
            }
            set
            {
                _pockelsCalibrateAgainEnable = value;
                if ((!value) && (null != _pockelsPlot))
                {
                    _pockelsPlot.Close();
                }
                OnPropertyChanged("PockelsCalibrateAgainEnable");
            }
        }

        public ICommand PockelsCalibrateCommand
        {
            get
            {
                if (this._powerCalibrateCommand == null)
                    this._powerCalibrateCommand = new ThorSharedTypes.RelayCommandWithParam((x) => PockelsCalibrate(x));

                return this._powerCalibrateCommand;
            }
        }

        public double PockelsDelayUS0
        {
            get
            {
                return this._powerControlModel.PockelsDelayUS0;
            }
            set
            {
                this._powerControlModel.PockelsDelayUS0 = value;
                OnPropertyChanged("PockelsDelayUS0");
            }
        }

        public double PockelsDelayUS1
        {
            get
            {
                return this._powerControlModel.PockelsDelayUS1;
            }
            set
            {
                this._powerControlModel.PockelsDelayUS1 = value;
                OnPropertyChanged("PockelsDelayUS1");
            }
        }

        public double PockelsDelayUS2
        {
            get
            {
                return this._powerControlModel.PockelsDelayUS2;
            }
            set
            {
                this._powerControlModel.PockelsDelayUS2 = value;
                OnPropertyChanged("PockelsDelayUS2");
            }
        }

        public double PockelsDelayUS3
        {
            get
            {
                return this._powerControlModel.PockelsDelayUS3;
            }
            set
            {
                this._powerControlModel.PockelsDelayUS3 = value;
                OnPropertyChanged("PockelsDelayUS3");
            }
        }

        public Visibility PockelsDelayVisibility0
        {
            get
            {
                return _powerControlModel.PockelsDelayAvailable0 ? Visibility.Visible : Visibility.Collapsed;
            }
            set
            {

            }
        }

        public Visibility PockelsDelayVisibility1
        {
            get
            {
                return _powerControlModel.PockelsDelayAvailable1 ? Visibility.Visible : Visibility.Collapsed;
            }
            set
            {

            }
        }

        public Visibility PockelsDelayVisibility2
        {
            get
            {
                return _powerControlModel.PockelsDelayAvailable2 ? Visibility.Visible : Visibility.Collapsed;
            }
            set
            {

            }
        }

        public Visibility PockelsDelayVisibility3
        {
            get
            {
                return _powerControlModel.PockelsDelayUSAvailable3 ? Visibility.Visible : Visibility.Collapsed;
            }
            set
            {

            }
        }

        public string PockelsMaskFile
        {
            get
            {
                return this._powerControlModel.PockelsMaskFile;
            }
            set
            {
                if (true == _powerControlModel.PockelsMaskOptionsAvailable)
                {
                    this._powerControlModel.PockelsMaskFile = value;
                    OnPropertyChanged("PockelsMaskFile");
                }
            }
        }

        public int PockelsMaskInvert0
        {
            get
            {
                return _powerControlModel.PockelsMaskInvert0;
            }
            set
            {
                _powerControlModel.PockelsMaskInvert0 = value;
                OnPropertyChanged("PockelsMaskInvert0");
            }
        }

        public bool PockelsMaskOptionsAvailable
        {
            get
            {
                return _powerControlModel.PockelsMaskOptionsAvailable;
            }
        }

        public KeyValuePair<XmlNodeList, XmlDocument> PockelsNodeList
        {
            get { return _pockelsNodeList; }
            set
            {
                _pockelsNodeList = value;
                UpdatePockelsNodeList();
            }
        }

        public double PockelsPowerMax
        {
            get
            {
                return this._powerControlModel.PockelsPowerMax;
            }
        }

        public double PockelsPowerMin
        {
            get
            {
                return this._powerControlModel.PockelsPowerMin;
            }
        }

        public ObservableCollection<DoublePC> PockelsPowerThreshold
        {
            get;
            set;
        }

        public double PockelsVoltageMax0
        {
            get
            {
                return Math.Round(this._powerControlModel.PockelsVoltageMax0, 3);
            }
            set
            {
                this._powerControlModel.PockelsVoltageMax0 = value;
                OnPropertyChanged("PockelsVoltageMax0");
            }
        }

        public double PockelsVoltageMax1
        {
            get
            {
                return Math.Round(this._powerControlModel.PockelsVoltageMax1, 3);
            }
            set
            {
                this._powerControlModel.PockelsVoltageMax1 = value;
                OnPropertyChanged("PockelsVoltageMax1");
            }
        }

        public double PockelsVoltageMax2
        {
            get
            {
                return Math.Round(this._powerControlModel.PockelsVoltageMax2, 3);
            }
            set
            {
                this._powerControlModel.PockelsVoltageMax2 = value;
                OnPropertyChanged("PockelsVoltageMax2");
            }
        }

        public double PockelsVoltageMax3
        {
            get
            {
                return Math.Round(this._powerControlModel.PockelsVoltageMax3, 3);
            }
            set
            {
                this._powerControlModel.PockelsVoltageMax3 = value;
                OnPropertyChanged("PockelsVoltageMax3");
            }
        }

        public double PockelsVoltageMin0
        {
            get
            {
                return Math.Round(this._powerControlModel.PockelsVoltageMin0, 3);
            }
            set
            {
                this._powerControlModel.PockelsVoltageMin0 = value;
                OnPropertyChanged("PockelsVoltageMin0");
            }
        }

        public double PockelsVoltageMin1
        {
            get
            {
                return Math.Round(this._powerControlModel.PockelsVoltageMin1, 3);
            }
            set
            {
                this._powerControlModel.PockelsVoltageMin1 = value;
                OnPropertyChanged("PockelsVoltageMin1");
            }
        }

        public double PockelsVoltageMin2
        {
            get
            {
                return Math.Round(this._powerControlModel.PockelsVoltageMin2, 3);
            }
            set
            {
                this._powerControlModel.PockelsVoltageMin2 = value;
                OnPropertyChanged("PockelsVoltageMin2");
            }
        }

        public double PockelsVoltageMin3
        {
            get
            {
                return Math.Round(this._powerControlModel.PockelsVoltageMin3, 3);
            }
            set
            {
                this._powerControlModel.PockelsVoltageMin3 = value;
                OnPropertyChanged("PockelsVoltageMin3");
            }
        }

        public double Power0
        {
            get
            {
                return this._powerControlModel.Power0;
            }
            set
            {
                this._powerControlModel.Power0 = value;
                OnPropertyChanged("Power0");
                OnPropertyChanged("PowerPositionActive");
            }
        }

        public double Power1
        {
            get
            {
                return this._powerControlModel.Power1;
            }
            set
            {
                this._powerControlModel.Power1 = value;
                OnPropertyChanged("Power1");
                OnPropertyChanged("PowerPositionActive");
            }
        }

        public double Power2
        {
            get
            {
                return this._powerControlModel.Power2;
            }
            set
            {
                this._powerControlModel.Power2 = value;
                OnPropertyChanged("Power2");
                OnPropertyChanged("PowerPositionActive");
            }
        }

        public double Power2Max
        {
            get
            {
                return this._powerControlModel.Power2Max;
            }
        }

        public double Power2Min
        {
            get
            {
                return this._powerControlModel.Power2Min;
            }
        }

        public double Power2StepSize
        {
            get
            {
                return _power2StepSize;
            }
            set
            {
                _power2StepSize = value;
                OnPropertyChanged("Power2StepSize");
            }
        }

        public double Power3
        {
            get
            {
                return this._powerControlModel.Power3;
            }
            set
            {
                this._powerControlModel.Power3 = value;
                OnPropertyChanged("Power3");
                OnPropertyChanged("PowerPositionActive");
            }
        }

        public ICommand PowerClearPlotCommand
        {
            get
            {
                if (this._powerClearPlotCommand == null)
                    this._powerClearPlotCommand = new ThorSharedTypes.RelayCommand(() => PowerClearPlot());

                return this._powerClearPlotCommand;
            }
        }

        public ObservableCollection<StringPC> PowerControlName
        {
            get;
            set;
        }

        public ICommand PowerControlNameSaveCommand
        {
            get
            {
                if (this._powerControlNameSaveCommand == null)
                    this._powerControlNameSaveCommand = new ThorSharedTypes.RelayCommandWithParam((x) => PowerControlNameSave(x));

                return this._powerControlNameSaveCommand;
            }
        }

        public double PowerGo0
        {
            get;
            set;
        }

        public double PowerGo1
        {
            get;
            set;
        }

        public double PowerGo2
        {
            get;
            set;
        }

        public double PowerGo3
        {
            get;
            set;
        }

        public double PowerGoReg
        {
            get;
            set;
        }

        public double PowerGoReg2
        {
            get
            {
                return _powerReg2;
            }
            set
            {
                _powerReg2 = value;
                OnPropertyChanged("PowerGoReg2");
            }
        }

        public double PowerMax
        {
            get
            {
                return this._powerControlModel.PowerMax;
            }
        }

        public double PowerMin
        {
            get
            {
                return this._powerControlModel.PowerMin;
            }
        }

        public ICommand PowerMinusCommand
        {
            get
            {
                if (this._powerMinusCommand == null)
                    this._powerMinusCommand = new RelayCommandWithParam((x) => PowerMinus(x));

                return this._powerMinusCommand;
            }
        }

        public string PowerMinusKey0
        {
            get { return _powerMinusKey[0]; }
            set
            {
                _powerMinusKey[0] = value;
                OnPropertyChanged("PowerMinusKey0");
            }
        }

        public string PowerMinusKey1
        {
            get { return _powerMinusKey[1]; }
            set
            {
                _powerMinusKey[1] = value;
                OnPropertyChanged("PowerMinusKey1");
            }
        }

        public string PowerMinusKey2
        {
            get { return _powerMinusKey[2]; }
            set
            {
                _powerMinusKey[2] = value;
                OnPropertyChanged("PowerMinusKey2");
            }
        }

        public string PowerMinusKey3
        {
            get { return _powerMinusKey[3]; }
            set
            {
                _powerMinusKey[3] = value;
                OnPropertyChanged("PowerMinusKey3");
            }
        }

        public string PowerMinusKeyReg
        {
            get { return _powerMinusKey[4]; }
            set
            {
                _powerMinusKey[4] = value;
                OnPropertyChanged("PowerMinusKeyReg");
            }
        }

        public string PowerMinusModifier0
        {
            get { return _powerMinusModifier[0]; }
            set
            {
                _powerMinusModifier[0] = value;
                OnPropertyChanged("PowerMinusModifier0");
            }
        }

        public string PowerMinusModifier1
        {
            get { return _powerMinusModifier[1]; }
            set
            {
                _powerMinusModifier[1] = value;
                OnPropertyChanged("PowerMinusModifier1");
            }
        }

        public string PowerMinusModifier2
        {
            get { return _powerMinusModifier[2]; }
            set
            {
                _powerMinusModifier[2] = value;
                OnPropertyChanged("PowerMinusModifier2");
            }
        }

        public string PowerMinusModifier3
        {
            get { return _powerMinusModifier[3]; }
            set
            {
                _powerMinusModifier[3] = value;
                OnPropertyChanged("PowerMinusModifier3");
            }
        }

        public string PowerMinusModifierReg
        {
            get { return _powerMinusModifier[4]; }
            set
            {
                _powerMinusModifier[4] = value;
                OnPropertyChanged("PowerMinusModifierReg");
            }
        }

        public int PowerMode0
        {
            get
            {
                return _powerMode[0];
            }
            set
            {
                _powerMode[0] = value;
                OnPropertyChanged("PowerMode0");
            }
        }

        public int PowerMode1
        {
            get
            {
                return _powerMode[1];
            }
            set
            {
                _powerMode[1] = value;
                OnPropertyChanged("PowerMode1");
            }
        }

        public int PowerMode2
        {
            get
            {
                return _powerMode[2];
            }
            set
            {
                _powerMode[2] = value;
                OnPropertyChanged("PowerMode2");
            }
        }

        public int PowerMode3
        {
            get
            {
                return _powerMode[3];
            }
            set
            {
                _powerMode[3] = value;
                OnPropertyChanged("PowerMode3");
            }
        }

        public int PowerModeReg
        {
            get
            {
                return _powerMode[4];
            }
            set
            {
                _powerMode[4] = value;
                OnPropertyChanged("PowerModeReg");
            }
        }

        public int PowerModeReg2
        {
            get
            {
                return _powerMode[5];
            }
            set
            {
                _powerMode[5] = value;
                OnPropertyChanged("PowerModeReg2");
            }
        }

        public PowerPlotWin PowerPlot
        {
            get { return _powerPlot; }
        }

        public double PowerPlotZPosition
        {
            get;
            set;
        }

        public ICommand PowerPlusCommand
        {
            get
            {
                if (this._powerPlusCommand == null)
                    this._powerPlusCommand = new RelayCommandWithParam((x) => PowerPlus(x));

                return this._powerPlusCommand;
            }
        }

        public string PowerPlusKey0
        {
            get { return _powerPlusKey[0]; }
            set
            {
                _powerPlusKey[0] = value;
                OnPropertyChanged("PowerPlusKey0");
            }
        }

        public string PowerPlusKey1
        {
            get { return _powerPlusKey[1]; }
            set
            {
                _powerPlusKey[1] = value;
                OnPropertyChanged("PowerPlusKey1");
            }
        }

        public string PowerPlusKey2
        {
            get { return _powerPlusKey[2]; }
            set
            {
                _powerPlusKey[2] = value;
                OnPropertyChanged("PowerPlusKey2");
            }
        }

        public string PowerPlusKey3
        {
            get { return _powerPlusKey[3]; }
            set
            {
                _powerPlusKey[3] = value;
                OnPropertyChanged("PowerPlusKey3");
            }
        }

        public string PowerPlusKeyReg
        {
            get { return _powerPlusKey[4]; }
            set
            {
                _powerPlusKey[4] = value;
                OnPropertyChanged("PowerPlusKeyReg");
            }
        }

        public string PowerPlusModifier0
        {
            get { return _powerPlusModifier[0]; }
            set
            {
                _powerPlusModifier[0] = value;
                OnPropertyChanged("PowerPlusModifier0");
            }
        }

        public string PowerPlusModifier1
        {
            get { return _powerPlusModifier[1]; }
            set
            {
                _powerPlusModifier[1] = value;
                OnPropertyChanged("PowerPlusModifier1");
            }
        }

        public string PowerPlusModifier2
        {
            get { return _powerPlusModifier[2]; }
            set
            {
                _powerPlusModifier[2] = value;
                OnPropertyChanged("PowerPlusModifier2");
            }
        }

        public string PowerPlusModifier3
        {
            get { return _powerPlusModifier[3]; }
            set
            {
                _powerPlusModifier[3] = value;
                OnPropertyChanged("PowerPlusModifier3");
            }
        }

        public string PowerPlusModifierReg
        {
            get { return _powerPlusModifier[4]; }
            set
            {
                _powerPlusModifier[4] = value;
                OnPropertyChanged("PowerPlusModifierReg");
            }
        }

        public double PowerPosition2Current
        {
            get
            {
                return this._powerControlModel.PowerPosition2Current;
            }
        }

        public double PowerPositionActive
        {
            get
            {
                double val = 0;

                switch (SelectedPowerTab)
                {
                    case 0: val = Power0; break;
                    case 1: val = Power1; break;
                    case 2: val = Power2; break;
                    case 3: val = Power3; break;
                    case 4: val = PowerReg; break;
                    case 5: val = PowerReg2; break;
                }

                return val;
            }
        }

        public double PowerPositionCurrent
        {
            get
            {
                return this._powerControlModel.PowerPositionCurrent;
            }
        }

        public double PowerPositionGo
        {
            get;
            set;
        }

        public ICommand PowerRampAddCommand
        {
            get
            {
                if (this._powerRampAddCommand == null)
                    this._powerRampAddCommand = new RelayCommand(PowerRampAdd);

                return this._powerRampAddCommand;
            }
        }

        public ICommand PowerRampDeleteCommand
        {
            get
            {
                if (this._powerRampDeleteCommand == null)
                    this._powerRampDeleteCommand = new RelayCommand(PowerRampDelete);

                return this._powerRampDeleteCommand;
            }
        }

        public ICommand PowerRampEditCommand
        {
            get
            {
                if (this._powerRampEditCommand == null)
                    this._powerRampEditCommand = new RelayCommand(PowerRampEdit);

                return this._powerRampEditCommand;
            }
        }

        public double[] PowerRampPlotX
        {
            get
            {
                if (SelectedRamp >= 0)
                {
                    string customRamp = Application.Current.Resources["ApplicationSettingsFolder"].ToString() + "\\powerramps\\" + PowerRampsCustom[SelectedRamp] + ".txt";

                    StreamReader fs = new StreamReader(customRamp);
                    string line;
                    int counter = 0;

                    List<double> xAxis = new List<double>();

                    try
                    {
                        while ((line = fs.ReadLine()) != null)
                        {
                            string[] split = line.Split(',');

                            if (split[1] != null)
                            {
                                xAxis.Add(Convert.ToDouble(split[0]));
                            }
                            counter++;
                        }
                    }
                    catch (Exception ex)
                    {
                        string msg = ex.Message;
                    }
                    fs.Close();
                    if (counter <= MAX_POWER_RAMP_DATA_POINTS)
                    {
                        return xAxis.ToArray();
                    }
                }

                return new double[0];
            }
        }

        public double[] PowerRampPlotY
        {
            get
            {
                if ((SelectedRamp >= 0) && (PowerRampsCustom.Count > 0))
                {
                    string customRamp = Application.Current.Resources["ApplicationSettingsFolder"].ToString() + "\\powerramps\\" + PowerRampsCustom[SelectedRamp] + ".txt";

                    StreamReader fs = new StreamReader(customRamp);
                    string line;
                    int counter = 0;

                    List<double> yAxis = new List<double>();

                    try
                    {
                        while ((line = fs.ReadLine()) != null)
                        {
                            string[] split = line.Split(',');

                            if (split[0] != null)
                            {
                                yAxis.Add(Convert.ToDouble(split[1]));
                            }
                            counter++;
                        }
                    }
                    catch (Exception ex)
                    {
                        string msg = ex.Message;
                    }
                    fs.Close();
                    if (counter <= MAX_POWER_RAMP_DATA_POINTS)
                    {
                        return yAxis.ToArray();
                    }
                }

                return new double[0];
            }
        }

        public ObservableCollection<string> PowerRampsCustom
        {
            get
            {
                return _powerRampsCustom;
            }
        }

        public int PowerRampSelected0
        {
            get
            {
                return _powerRampSelected0;
            }
            set
            {
                _powerRampSelected0 = value;
                OnPropertyChanged("PowerRampSelected0");
                PowerRampEditChanged(0);
            }
        }

        public int PowerRampSelected1
        {
            get
            {
                return _powerRampSelected1;
            }
            set
            {
                _powerRampSelected1 = value;
                OnPropertyChanged("PowerRampSelected1");
                PowerRampEditChanged(1);
            }
        }

        public int PowerRampSelected2
        {
            get
            {
                return _powerRampSelected2;
            }
            set
            {
                _powerRampSelected2 = value;
                OnPropertyChanged("PowerRampSelected2");
                PowerRampEditChanged(2);
            }
        }

        public int PowerRampSelected3
        {
            get
            {
                return _powerRampSelected3;
            }
            set
            {
                _powerRampSelected3 = value;
                OnPropertyChanged("PowerRampSelected3");
                PowerRampEditChanged(3);
            }
        }

        public int PowerRampSelectedReg
        {
            get
            {
                return _powerRampSelectedReg;
            }
            set
            {
                _powerRampSelectedReg = value;
                OnPropertyChanged("PowerRampSelectedReg");
                PowerRampEditChanged(4);
            }
        }

        public int PowerRampSelectedReg2
        {
            get
            {
                return _powerRampSelectedReg2;
            }
            set
            {
                _powerRampSelectedReg2 = value;
                OnPropertyChanged("PowerRampSelectedReg2");
                PowerRampEditChanged(5);
            }
        }

        public ICommand PowerRecordPlotCommand
        {
            get
            {
                if (this._powerRecordPlotCommand == null)
                    this._powerRecordPlotCommand = new RelayCommand(() => PowerRecordPlot());

                return this._powerRecordPlotCommand;
            }
        }

        public double PowerReg
        {
            get
            {
                return this._powerControlModel.PowerPosition;
            }
            set
            {
                TimeSpan ts = DateTime.Now - _lastPowerUpdateTime;
                if (1 < ts.TotalSeconds)
                {
                    this._powerControlModel.PowerPosition = value;
                    _lastPowerUpdateTime = DateTime.Now;
                }
                OnPropertyChanged("PowerReg");
                OnPropertyChanged("PowerPositionActive");
            }
        }

        public double PowerReg2
        {
            get
            {
                return this._powerControlModel.PowerPosition2;
            }
            set
            {
                TimeSpan ts = DateTime.Now - _lastPowerReg2UpdateTime;
                if (1 < ts.TotalSeconds)
                {
                    this._powerControlModel.PowerPosition2 = value;
                    _lastPowerReg2UpdateTime = DateTime.Now;
                }
                OnPropertyChanged("PowerReg2");
                OnPropertyChanged("PowerPositionActive");
            }
        }

        public ICommand PowerReg2CalCommand
        {
            get
            {
                if (null == this._powerReg2CalCommand)
                    this._powerReg2CalCommand = new RelayCommandWithParam((x) => PowerReg2Cal(x));

                return this._powerReg2CalCommand;
            }
        }

        public string PowerReg2CalName1
        {
            get
            {
                return _powerReg2CalName1;
            }
            set
            {
                _powerReg2CalName1 = value;
                OnPropertyChanged("PowerReg2CalName1");
            }
        }

        public string PowerReg2CalName2
        {
            get
            {
                return _powerReg2CalName2;
            }
            set
            {
                _powerReg2CalName2 = value;
                OnPropertyChanged("PowerReg2CalName2");
            }
        }

        public string PowerReg2CalName3
        {
            get
            {
                return _powerReg2CalName3;
            }
            set
            {
                _powerReg2CalName3 = value;
                OnPropertyChanged("PowerReg2CalName3");
            }
        }

        public string PowerReg2CalName4
        {
            get
            {
                return _powerReg2CalName4;
            }
            set
            {
                _powerReg2CalName4 = value;
                OnPropertyChanged("PowerReg2CalName4");
            }
        }

        public string PowerReg2CalName5
        {
            get
            {
                return _powerReg2CalName5;
            }
            set
            {
                _powerReg2CalName5 = value;
                OnPropertyChanged("PowerReg2CalName5");
            }
        }

        public string PowerReg2CalName6
        {
            get
            {
                return _powerReg2CalName6;
            }
            set
            {
                _powerReg2CalName6 = value;
                OnPropertyChanged("PowerReg2CalName6");
            }
        }

        public ICommand PowerReg2CalSaveCommand
        {
            get
            {
                if (null == this._powerReg2CalSaveCommand)
                    this._powerReg2CalSaveCommand = new RelayCommandWithParam((x) => PowerReg2CalSave(x));

                return this._powerReg2CalSaveCommand;
            }
        }

        public string PowerReg2EncoderPosition
        {
            get
            {
                return this._powerControlModel.PowerReg2EncoderPosition;
            }
        }

        public Visibility PowerReg2Visibility
        {
            get
            {
                return _powerReg2Visibility;
            }
            set
            {
                _powerReg2Visibility = value;
                OnPropertyChanged("PowerReg2Visibility");
            }
        }

        public double PowerReg2Zero
        {
            get
            {
                return _powerControlModel.PowerReg2Zero;
            }
            set
            {
                _powerControlModel.PowerReg2Zero = value;
                OnPropertyChanged("PowerReg2Zero");
            }
        }

        public string PowerRegCal1Name
        {
            get
            {
                return _powerRegCal1Name;
            }
            set
            {
                _powerRegCal1Name = value;
                OnPropertyChanged("PowerRegCal1Name");
            }
        }

        public string PowerRegCal2Name
        {
            get
            {
                return _powerRegCal2Name;
            }
            set
            {
                _powerRegCal2Name = value;
                OnPropertyChanged("PowerRegCal2Name");
            }
        }

        public string PowerRegCal3Name
        {
            get
            {
                return _powerRegCal3Name;
            }
            set
            {
                _powerRegCal3Name = value;
                OnPropertyChanged("PowerRegCal3Name");
            }
        }

        public string PowerRegCal4Name
        {
            get
            {
                return _powerRegCal4Name;
            }
            set
            {
                _powerRegCal4Name = value;
                OnPropertyChanged("PowerRegCal4Name");
            }
        }

        public string PowerRegCal5Name
        {
            get
            {
                return _powerRegCal5Name;
            }
            set
            {
                _powerRegCal5Name = value;
                OnPropertyChanged("PowerRegCal5Name");
            }
        }

        public string PowerRegCal6Name
        {
            get
            {
                return _powerRegCal6Name;
            }
            set
            {
                _powerRegCal6Name = value;
                OnPropertyChanged("PowerRegCal6Name");
            }
        }

        public ICommand PowerRegCalCommand
        {
            get
            {
                if (null == this._powerRegCalCommand)
                    this._powerRegCalCommand = new RelayCommandWithParam((x) => PowerRegCal(x));

                return this._powerRegCalCommand;
            }
        }

        public double PowerRegCalOffset
        {
            get
            {
                return _powerRegCalOffset;
            }
            set
            {
                _powerRegCalOffset = value;
            }
        }

        public ICommand PowerRegCalSaveCommand
        {
            get
            {
                if (null == this._powerRegCalSaveCommand)
                    this._powerRegCalSaveCommand = new RelayCommandWithParam((x) => PowerRegCalSave(x));

                return this._powerRegCalSaveCommand;
            }
        }

        public string PowerRegEncoderPosition
        {
            get
            {
                return this._powerControlModel.PowerRegEncoderPosition;
            }
        }

        public Visibility PowerRegVisibility
        {
            get
            {
                return _powerRegVisibility;
            }
            set
            {
                _powerRegVisibility = value;
                OnPropertyChanged("PowerRegVisibility");
            }
        }

        public double PowerRegZero
        {
            get
            {
                return _powerControlModel.PowerRegZero;
            }
            set
            {
                _powerControlModel.PowerRegZero = value;
                OnPropertyChanged("PowerRegZero");
            }
        }

        public double PowerStepSize
        {
            get
            {
                return _powerStepSize;
            }
            set
            {
                _powerStepSize = value;
                OnPropertyChanged("PowerStepSize");
            }
        }

        public bool RedrawPowerPlot
        {
            set
            {
                if (null != _powerPlot)
                {
                    _powerPlot.SetData(PowerRampPlotX, PowerRampPlotY, value);
                    PowerPlotZPosition = (double)MVMManager.Instance["ZControlViewModel", "ZPosition", (object)0.0];
                    OnPropertyChanged("PowerPlotZPosition");
                }
            }
        }

        public int SelectedPowerMode
        {
            get
            {
                int selectedPowerMode = 0;
                switch (_selectedPowerTab)
                {
                    case 0: selectedPowerMode = PowerMode0; break;
                    case 1: selectedPowerMode = PowerMode1; break;
                    case 2: selectedPowerMode = PowerMode2; break;
                    case 3: selectedPowerMode = PowerMode3; break;
                    case 4: selectedPowerMode = PowerModeReg; break;
                    case 5: selectedPowerMode = PowerModeReg2; break;
                }
                return selectedPowerMode;
            }
        }

        public int SelectedPowerTab
        {
            get
            {
                return _selectedPowerTab;
            }
            set
            {
                _selectedPowerTab = value;
                PersistSelectedPowerTab();
                OnPropertyChanged("SelectedPowerTab");
                if ((0 < SelectedPowerMode))
                {
                    PowerRampEditChanged(_selectedPowerTab); //Nothing really happens here because of previous condition
                    UpdateStartPower();
                }
            }
        }

        public int SelectedRamp
        {
            get
            {
                int rampSelected = 0;
                switch (_selectedPowerTab)
                {
                    case 0: rampSelected = PowerRampSelected0; break;
                    case 1: rampSelected = PowerRampSelected1; break;
                    case 2: rampSelected = PowerRampSelected2; break;
                    case 3: rampSelected = PowerRampSelected3; break;
                    case 4: rampSelected = PowerRampSelectedReg; break;
                    case 5: rampSelected = PowerRampSelectedReg2; break;
                }
                return rampSelected;
            }
        }

        public ICommand SelectPockelsMaskCommand
        {
            get
            {
                if (this._selectPockelsMaskCommand == null)
                    this._selectPockelsMaskCommand = new RelayCommand(() => SelectPockelsMask());

                return this._selectPockelsMaskCommand;
            }
        }

        public ICommand SetBleacherPowerCommand
        {
            get
            {
                if (this._setBleacherPowerCommand == null)
                    this._setBleacherPowerCommand = new RelayCommand(() => SetBleacherPower());

                return this._setBleacherPowerCommand;
            }
        }

        public ICommand SetBleacherPowerToZeroCommand
        {
            get
            {
                if (this._setBleacherPowerToZeroCommand == null)
                    this._setBleacherPowerToZeroCommand = new RelayCommand(() => SetBleacherPowerToZero());

                return this._setBleacherPowerToZeroCommand;
            }
        }

        public ICommand SetPowerCommand
        {
            get
            {
                if (this._setPowerCommand == null)
                    this._setPowerCommand = new RelayCommandWithParam((x) => SetPower(x));

                return this._setPowerCommand;
            }
        }

        public int ShutterPosition
        {
            get
            {
                return this._powerControlModel.ShutterPosition;
            }
            set
            {
                this._powerControlModel.ShutterPosition = value;
                OnPropertyChanged("ShutterPosition");
            }
        }

        public ICommand StepCoarseCommand
        {
            get
            {
                if (this._stepCoarseCommand == null)
                    this._stepCoarseCommand = new RelayCommandWithParam((x) => CoarseSliderStepSize(x));

                return this._stepCoarseCommand;
            }
        }

        public ICommand StepFineCommand
        {
            get
            {
                if (this._stepFineCommand == null)
                    this._stepFineCommand = new RelayCommandWithParam((x) => FineSliderStepSize(x));

                return this._stepFineCommand;
            }
        }

        public ICommand UpdatePockelsMaskToROIMaskCommand
        {
            get
            {
                if (_updatePockelsMaskToROIMaskCommand == null)
                    _updatePockelsMaskToROIMaskCommand = new RelayCommand(() => UpdatePockelsMaskToROIMask());
                return _updatePockelsMaskToROIMaskCommand;
            }
        }

        #endregion Properties

        #region Indexers

        public object this[string propertyName, object defaultObject = null]
        {
            get
            {
                PropertyInfo myPropInfo = GetPropertyInfo(propertyName);
                return (null != myPropInfo) ? myPropInfo.GetValue(this) : defaultObject;
            }
            set
            {
                PropertyInfo myPropInfo = GetPropertyInfo(propertyName);
                if (null != myPropInfo)
                {
                    myPropInfo.SetValue(this, value);
                }
            }
        }

        public object this[string propertyName, int index, object defaultObject = null]
        {
            get
            {
                PropertyInfo myPropInfo = GetPropertyInfo(propertyName);
                if (null != myPropInfo)
                {
                    if (myPropInfo.PropertyType.IsGenericTypeDefinition && typeof(CustomCollection<>) == myPropInfo.PropertyType.GetGenericTypeDefinition())
                    {
                        var collection = myPropInfo.GetValue(this, null);
                        return collection.GetType().GetProperty("Item").GetValue(collection, new object[] { index });
                    }
                    else
                    {
                        return myPropInfo.GetValue(this, null);
                    }
                }
                return defaultObject;
            }
            set
            {
                PropertyInfo myPropInfo = GetPropertyInfo(propertyName);
                if (null != myPropInfo)
                {
                    if (myPropInfo.PropertyType.IsGenericTypeDefinition && typeof(CustomCollection<>) == myPropInfo.PropertyType.GetGenericTypeDefinition())
                    {
                        var collection = myPropInfo.GetValue(this, null);
                        collection.GetType().GetProperty("Item").SetValue(collection, value, new object[] { index });
                    }
                    else
                    {
                        myPropInfo.SetValue(this, value, null);
                    }
                }
            }
        }

        #endregion Indexers

        #region Methods

        public PropertyInfo GetPropertyInfo(string propertyName)
        {
            PropertyInfo myPropInfo = null;
            if (!_properties.TryGetValue(propertyName, out myPropInfo))
            {
                myPropInfo = typeof(PowerControlViewModel).GetProperty(propertyName);
                if (null != myPropInfo)
                {
                    _properties.Add(propertyName, myPropInfo);
                }
            }
            return myPropInfo;
        }

        public void LoadXMLSettings()
        {
            XmlNodeList ndList;
            int tmp = 0;
            double tmpD = 0;

            string customPowerRampsFolder = Application.Current.Resources["ApplicationSettingsFolder"].ToString() + "\\powerramps";

            XmlDocument experimentDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS];

            if (!Directory.Exists(customPowerRampsFolder))
            {
                Directory.CreateDirectory(customPowerRampsFolder);
            }
            else
            {
                string[] ramps = Directory.GetFiles(customPowerRampsFolder, "*.txt");

                PowerRampsCustom.Clear();

                for (int i = 0; i < ramps.Length; i++)
                {
                    PowerRampsCustom.Add(Path.GetFileNameWithoutExtension(ramps[i]));
                }
            }

            ndList = experimentDoc.SelectNodes("/ThorImageExperiment/PowerRegulator");

            if (ndList.Count > 0)
            {
                string str = string.Empty;

                if (XmlManager.GetAttribute(ndList[0], experimentDoc, "offset", ref str) && (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmpD)) && (PowerRegZero != tmpD))
                {
                    PowerRegZero = tmpD;
                }

                if (XmlManager.GetAttribute(ndList[0], experimentDoc, "type", ref str) && (Int32.TryParse(str, out tmp)))
                {
                    PowerModeReg = tmp;
                }

                if (XmlManager.GetAttribute(ndList[0], experimentDoc, "sliderStepSize", ref str) && (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmpD)))
                {
                    PowerStepSize = tmpD;
                }

                if (XmlManager.GetAttribute(ndList[0], experimentDoc, "path", ref str))
                {
                    string rampName = Path.GetFileNameWithoutExtension(str);

                    for (int j = 0; j < PowerRampsCustom.Count; j++)
                    {
                        if (PowerRampsCustom[j].Equals(rampName))
                        {
                            PowerRampSelectedReg = j;
                        }
                    }
                }
                if ((1 == PowerModeReg) && (0 < PowerRampsCustom.Count) && ((PowerRampsCustom.Count - 1) >= PowerRampSelectedReg))
                {
                    //PowerReg should be updated while switch tab.
                }
                else
                {
                    if (XmlManager.GetAttribute(ndList[0], experimentDoc, "start", ref str) && (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmpD)))
                    {
                        PowerReg = tmpD;
                    }
                    PowerModeReg = 0;
                }
            }

            ndList = experimentDoc.SelectNodes("/ThorImageExperiment/PowerRegulator2");

            if (ndList.Count > 0)
            {
                string str = string.Empty;

                if (XmlManager.GetAttribute(ndList[0], experimentDoc, "offset", ref str) && (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmpD)) && (PowerReg2Zero != tmpD))
                {
                    PowerReg2Zero = tmpD;
                }

                if (XmlManager.GetAttribute(ndList[0], experimentDoc, "type", ref str) && (Int32.TryParse(str, out tmp)))
                {
                    PowerModeReg2 = tmp;
                }

                if (XmlManager.GetAttribute(ndList[0], experimentDoc, "sliderStepSize", ref str) && (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmpD)))
                {
                    Power2StepSize = tmpD;
                }

                if (XmlManager.GetAttribute(ndList[0], experimentDoc, "path", ref str))
                {
                    string rampName = Path.GetFileNameWithoutExtension(str);

                    for (int j = 0; j < PowerRampsCustom.Count; j++)
                    {
                        if (PowerRampsCustom[j].Equals(rampName))
                        {
                            PowerRampSelectedReg2 = j;
                        }
                    }
                }
                if ((1 == PowerModeReg2) && (0 < PowerRampsCustom.Count) && ((PowerRampsCustom.Count - 1) >= PowerRampSelectedReg2))
                {
                    //PowerReg2 should be updated while switch tab.
                }
                else
                {
                    if (XmlManager.GetAttribute(ndList[0], experimentDoc, "start", ref str) && (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmpD)))
                    {
                        PowerReg2 = tmpD;
                    }
                    PowerModeReg2 = 0;
                }
            }

            ndList = experimentDoc.SelectNodes("/ThorImageExperiment/Pockels");

            for (int i = 0; i < ndList.Count; i++)
            {
                string str = string.Empty;
                switch (i)
                {
                    case 0:
                        {
                            if (XmlManager.GetAttribute(ndList[i], experimentDoc, "type", ref str) && (Int32.TryParse(str, out tmp)))
                            {
                                PowerMode0 = tmp;
                            }
                            if (XmlManager.GetAttribute(ndList[i], experimentDoc, "pockelsBlankPercentage", ref str) && (Int32.TryParse(str, out tmp)))
                            {
                                PockelsBlankPercentage0 = tmp;
                            }
                            if (XmlManager.GetAttribute(ndList[i], experimentDoc, "maskEnable", ref str))
                            {
                                EnablePockelsMask = (Int32.TryParse(str, out tmp)) ? tmp : 0;
                            }
                            if (XmlManager.GetAttribute(ndList[i], experimentDoc, "maskInvert", ref str))
                            {
                                PockelsMaskInvert0 = (Int32.TryParse(str, out tmp)) ? tmp : 0;
                            }
                            if (XmlManager.GetAttribute(ndList[i], experimentDoc, "maskPath", ref str))
                            {
                                PockelsMaskFile = str;
                            }
                            if (XmlManager.GetAttribute(ndList[i], experimentDoc, "pockelsMinV", ref str) && (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmpD)))
                            {
                                PockelsVoltageMin0 = tmpD;
                            }
                            if (XmlManager.GetAttribute(ndList[i], experimentDoc, "pockelsMaxV", ref str) && (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmpD)))
                            {
                                PockelsVoltageMax0 = tmpD;
                            }
                            if (XmlManager.GetAttribute(ndList[i], experimentDoc, "path", ref str))
                            {
                                string rampName = Path.GetFileNameWithoutExtension(str);

                                for (int j = 0; j < PowerRampsCustom.Count; j++)
                                {
                                    if (PowerRampsCustom[j].Equals(rampName))
                                    {
                                        PowerRampSelected0 = j;
                                    }
                                }
                            }
                            if ((1 == PowerMode0) && (0 < PowerRampsCustom.Count) && ((PowerRampsCustom.Count - 1) >= PowerRampSelected0))
                            {
                                //Power0 should be updated while switch tab.
                            }
                            else
                            {
                                if (XmlManager.GetAttribute(ndList[i], experimentDoc, "start", ref str))
                                {
                                    if (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmpD))
                                    {
                                        Power0 = tmpD;
                                    }
                                }
                                PowerMode0 = 0;
                            }
                        }
                        break;
                    case 1:
                        {
                            if (XmlManager.GetAttribute(ndList[i], experimentDoc, "type", ref str) && (Int32.TryParse(str, out tmp)))
                            {
                                PowerMode1 = tmp;
                            }
                            if (XmlManager.GetAttribute(ndList[i], experimentDoc, "pockelsBlankPercentage", ref str) && (Int32.TryParse(str, out tmp)))
                            {
                                PockelsBlankPercentage1 = tmp;
                            }
                            if (XmlManager.GetAttribute(ndList[i], experimentDoc, "pockelsMinV", ref str) && (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmpD)))
                            {
                                PockelsVoltageMin1 = tmpD;
                            }
                            if (XmlManager.GetAttribute(ndList[i], experimentDoc, "pockelsMaxV", ref str) && (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmpD)))
                            {
                                PockelsVoltageMax1 = tmpD;
                            }
                            if (XmlManager.GetAttribute(ndList[i], experimentDoc, "path", ref str))
                            {
                                string rampName = Path.GetFileNameWithoutExtension(str);

                                for (int j = 0; j < PowerRampsCustom.Count; j++)
                                {
                                    if (PowerRampsCustom[j].Equals(rampName))
                                    {
                                        PowerRampSelected1 = j;
                                    }
                                }
                            }
                            if ((1 == PowerMode1) && (0 < PowerRampsCustom.Count) && ((PowerRampsCustom.Count - 1) >= PowerRampSelected1))
                            {
                                //Power1 should be updated while switch tab.
                            }
                            else
                            {
                                if (XmlManager.GetAttribute(ndList[i], experimentDoc, "start", ref str))
                                {
                                    if (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmpD))
                                    {
                                        Power1 = tmpD;
                                    }
                                }
                                PowerMode1 = 0;
                            }
                        }
                        break;
                    case 2:
                        {
                            if (XmlManager.GetAttribute(ndList[i], experimentDoc, "type", ref str) && (Int32.TryParse(str, out tmp)))
                            {
                                PowerMode2 = tmp;
                            }
                            if (XmlManager.GetAttribute(ndList[i], experimentDoc, "pockelsBlankPercentage", ref str) && (Int32.TryParse(str, out tmp)))
                            {
                                PockelsBlankPercentage2 = tmp;
                            }
                            if (XmlManager.GetAttribute(ndList[i], experimentDoc, "pockelsMinV", ref str) && (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmpD)))
                            {
                                PockelsVoltageMin2 = tmpD;
                            }
                            if (XmlManager.GetAttribute(ndList[i], experimentDoc, "pockelsMaxV", ref str) && (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmpD)))
                            {
                                PockelsVoltageMax2 = tmpD;
                            }
                            if (XmlManager.GetAttribute(ndList[i], experimentDoc, "path", ref str))
                            {
                                string rampName = Path.GetFileNameWithoutExtension(str);

                                for (int j = 0; j < PowerRampsCustom.Count; j++)
                                {
                                    if (PowerRampsCustom[j].Equals(rampName))
                                    {
                                        PowerRampSelected2 = j;
                                    }
                                }
                            }
                            if ((1 == PowerMode2) && (0 < PowerRampsCustom.Count) && ((PowerRampsCustom.Count - 1) >= PowerRampSelected2))
                            {
                                //Power2 should be updated while switch tab.
                            }
                            else
                            {
                                if (XmlManager.GetAttribute(ndList[i], experimentDoc, "start", ref str) && (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmpD)))
                                {
                                    Power2 = tmpD;
                                }
                                PowerMode2 = 0;
                            }
                        }
                        break;
                    case 3:
                        {
                            if (XmlManager.GetAttribute(ndList[i], experimentDoc, "type", ref str) && (Int32.TryParse(str, out tmp)))
                            {
                                PowerMode3 = tmp;
                            }
                            if (XmlManager.GetAttribute(ndList[i], experimentDoc, "pockelsBlankPercentage", ref str) && (Int32.TryParse(str, out tmp)))
                            {
                                PockelsBlankPercentage3 = tmp;
                            }
                            if (XmlManager.GetAttribute(ndList[i], experimentDoc, "pockelsMinV", ref str) && (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmpD)))
                            {
                                PockelsVoltageMin3 = tmpD;
                            }
                            if (XmlManager.GetAttribute(ndList[i], experimentDoc, "pockelsMaxV", ref str) && (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmpD)))
                            {
                                PockelsVoltageMax3 = tmpD;
                            }
                            if (XmlManager.GetAttribute(ndList[i], experimentDoc, "path", ref str))
                            {
                                string rampName = Path.GetFileNameWithoutExtension(str);

                                for (int j = 0; j < PowerRampsCustom.Count; j++)
                                {
                                    if (PowerRampsCustom[j].Equals(rampName))
                                    {
                                        PowerRampSelected3 = j;
                                    }
                                }
                            }
                            if ((1 == PowerMode3) && (0 < PowerRampsCustom.Count) && ((PowerRampsCustom.Count - 1) >= PowerRampSelected3))
                            {
                                //Power3 should be updated while switch tab.
                            }
                            else
                            {
                                if (XmlManager.GetAttribute(ndList[i], experimentDoc, "start", ref str) && (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmpD)))
                                {
                                    Power3 = tmpD;
                                }
                                PowerMode3 = 0;
                            }
                        }
                        break;
                }
            }
            OnPropertyChanged("PowerMax");
            OnPropertyChanged("PowerMin");
            OnPropertyChanged("Power2Max");
            OnPropertyChanged("Power2Min");
            OnPropertyChanged("PockelsPowerMax");
            OnPropertyChanged("PockelsPowerMin");
            OnPropertyChanged("PockelsBlankingPhaseShiftPercentVisibility");
            OnPropertyChanged("PockelsDelayVisibility0");
            OnPropertyChanged("PockelsDelayVisibility1");
            OnPropertyChanged("PockelsDelayVisibility2");
            OnPropertyChanged("PockelsDelayVisibility3");
            ResetBleacherPockelsList();
        }

        public void OnPropertyChange(string propertyName)
        {
            if (null != GetPropertyInfo(propertyName))
            {
                OnPropertyChanged(propertyName);
            }
        }

        public void ResetBleacherPockelsList()
        {
            //recreate bleach pockels list based on bleacher pockels enable
            _bleacherPockelsList.Clear();

            if (this._powerControlModel.BleacherPockelsEnable0)
            {
                _bleacherPockelsList.Add(1);
            }
            if (this._powerControlModel.BleacherPockelsEnable1)
            {
                _bleacherPockelsList.Add(2);
            }
            if (this._powerControlModel.BleacherPockelsEnable2)
            {
                _bleacherPockelsList.Add(3);
            }
            if (this._powerControlModel.BleacherPockelsEnable3)
            {
                _bleacherPockelsList.Add(4);
            }
            BleacherPowerID = 0;
            OnPropertyChanged("BleacherPowerID");
            OnPropertyChanged("BleacherPockelsList");
        }

        public void UpdateExpXMLSettings(ref XmlDocument experimentFile)
        {
            XmlNodeList ndList;

            ndList = experimentFile.SelectNodes("/ThorImageExperiment/PowerRegulator");

            if (ndList.Count > 0)
            {
                if ((int)PowerRampSelectedReg < 0)
                {
                    PowerModeReg = 0;
                }
                ndList[0].Attributes["type"].Value = PowerModeReg.ToString();

                if ((PowerRampSelectedReg < (PowerRampsCustom.Count) && (PowerRampSelectedReg >= 0)))
                {
                    XmlManager.SetAttribute(ndList[0], experimentFile, "path", Application.Current.Resources["ApplicationSettingsFolder"].ToString() + "\\powerramps\\" + PowerRampsCustom[PowerRampSelectedReg] + ".txt");
                }
                else
                {
                    XmlManager.SetAttribute(ndList[0], experimentFile, "path", "");
                }

                XmlManager.SetAttribute(ndList[0], experimentFile, "start", PowerReg.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "stop", PowerReg.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "offset", PowerRegZero.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "sliderStepSize", PowerStepSize.ToString());
            }

            ndList = experimentFile.SelectNodes("/ThorImageExperiment/PowerRegulator2");
            if (0 >= ndList.Count)
            {
                XmlManager.CreateXmlNode(experimentFile, "PowerRegulator2");
                ndList = experimentFile.SelectNodes("/ThorImageExperiment/PowerRegulator2");
                XmlManager.SetAttribute(ndList[0], experimentFile, "enable", "0");
                XmlManager.SetAttribute(ndList[0], experimentFile, "type", "0");
                XmlManager.SetAttribute(ndList[0], experimentFile, "start", "0");
                XmlManager.SetAttribute(ndList[0], experimentFile, "stop", "0");
                XmlManager.SetAttribute(ndList[0], experimentFile, "path", "C:\\");
            }

            if (ndList.Count > 0)
            {
                if (PowerRampSelectedReg2 < 0)
                {
                    PowerModeReg2 = 0;
                }
                ndList[0].Attributes["type"].Value = PowerModeReg2.ToString();

                if ((PowerRampSelectedReg2 < PowerRampsCustom.Count) && (PowerRampSelectedReg2 >= 0))
                {
                    XmlManager.SetAttribute(ndList[0], experimentFile, "path", Application.Current.Resources["ApplicationSettingsFolder"].ToString() + "\\powerramps\\" + PowerRampsCustom[PowerRampSelectedReg2] + ".txt");
                }
                else
                {
                    XmlManager.SetAttribute(ndList[0], experimentFile, "path", "");
                }

                XmlManager.SetAttribute(ndList[0], experimentFile, "start", PowerReg2.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "stop", PowerReg2.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "offset", PowerReg2Zero.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "sliderStepSize", Power2StepSize.ToString());
            }

            ndList = experimentFile.SelectNodes("/ThorImageExperiment/Pockels");
            int pcklsCount = Math.Max(0, ndList.Count);
            if (4 > pcklsCount)
            {
                for (int i = 0; i < 4 - ndList.Count; i++)
                {
                    XmlManager.CreateXmlNode(experimentFile, "Pockels");
                }
                ndList = experimentFile.SelectNodes("/ThorImageExperiment/Pockels");
            }

            PockelsNodeList = new KeyValuePair<XmlNodeList, XmlDocument>(ndList, experimentFile);
        }

        private void CoarseSliderStepSize(object index)
        {
            double stepSize = 1;
            switch (Convert.ToInt32(index))
            {
                case 0: break;
                case 1: break;
                case 2: break;
                case 3: break;
                case 4:
                    stepSize = PowerStepSize * 10;
                    PowerStepSize = (stepSize > MAX_STEP_SIZE) ? MAX_STEP_SIZE : stepSize;
                    break;
                case 5:
                    stepSize = Power2StepSize * 10;
                    Power2StepSize = (stepSize > MAX_STEP_SIZE) ? MAX_STEP_SIZE : stepSize;
                    break;
            }
        }

        private void FineSliderStepSize(object index)
        {
            double stepSize = 1;
            switch (Convert.ToInt32(index))
            {
                case 0: break;
                case 1: break;
                case 2: break;
                case 3: break;
                case 4:
                    stepSize = PowerStepSize / 10;
                    PowerStepSize = (stepSize < MIN_STEP_SIZE) ? MIN_STEP_SIZE : stepSize;
                    break;
                case 5:
                    stepSize = Power2StepSize / 10;
                    Power2StepSize = (stepSize < MIN_STEP_SIZE) ? MIN_STEP_SIZE : stepSize;
                    break;
            }
        }

        private void PersistSelectedPowerTab()
        {
            MVMManager.Instance.ReloadSettings(SettingsFileType.APPLICATION_SETTINGS);
            XmlDocument appSettings = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

            XmlNodeList ndList = appSettings.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/PockelsPowerTabs");
            if (0 < ndList.Count)
            {
                ThorSharedTypes.XmlManager.SetAttribute(ndList[0], appSettings, "selected", _selectedPowerTab.ToString());
            }
            else
            {
                XmlNode csNode = appSettings.SelectSingleNode("/ApplicationSettings/DisplayOptions/CaptureSetup");
                if (null != csNode)
                {
                    XmlNode newNode = appSettings.CreateNode(XmlNodeType.Element, "PockelsPowerTabs", null);
                    ThorSharedTypes.XmlManager.SetAttribute(newNode, appSettings, "selected", _selectedPowerTab.ToString());
                    csNode.AppendChild(newNode);
                }
            }
            MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS);
        }

        private void PockelsCalibrate(object i)
        {
            int index = Convert.ToInt32(i);

            IsPockelsCalibrationFailed = this._powerControlModel.FindPockelsMinMax(index);

            OnPropertyChanged("PockelsVoltageMin0");
            OnPropertyChanged("PockelsVoltageMin1");
            OnPropertyChanged("PockelsVoltageMin2");
            OnPropertyChanged("PockelsVoltageMin3");
            OnPropertyChanged("PockelsVoltageMax0");
            OnPropertyChanged("PockelsVoltageMax1");
            OnPropertyChanged("PockelsVoltageMax2");
            OnPropertyChanged("PockelsVoltageMax3");
            OnPropertyChanged("PockelsActiveVoltageMin");
            OnPropertyChanged("PockelsActiveVoltageMax");

            //if the plot is not yet displayed. create the window and show
            if ((_pockelsPlot == null) || (_pockelsPlot.IsLoaded == false))
            {
                _pockelsPlot = new PockelsPlotWin();
                _pockelsPlot.DataContext = this;
                _pockelsPlot.CurrentPockelsIndex = index;
                _pockelsPlot.Title = string.Format("{0} Plot", PowerControlName[index].Value);
                PockelsActiveIndex = index;
                _pockelsPlot.SetData(this._powerControlModel.GetPockelsPlotX(_pockelsPlot.CurrentPockelsIndex), this._powerControlModel.GetPockelsPlotY(_pockelsPlot.CurrentPockelsIndex), 1, 0, true);
                _pockelsPlot.Show();
            }
            else
            {
                //the plot is already created. update the data
                _pockelsPlot.SetData(this._powerControlModel.GetPockelsPlotX(_pockelsPlot.CurrentPockelsIndex), this._powerControlModel.GetPockelsPlotY(_pockelsPlot.CurrentPockelsIndex), 1, 0, true);
            }
        }

        private void PowerClearPlot()
        {
            if (SelectedRamp >= 0)
            {
                string customRamp = Application.Current.Resources["ApplicationSettingsFolder"].ToString() + "\\powerramps\\" + PowerRampsCustom[SelectedRamp] + ".txt";

                if (MessageBoxResult.Yes == MessageBox.Show((null == _powerPlot) ? new Window { Topmost = true } : _powerPlot, "Are you sure you want to clear all of the data points?", "Clear Data Points?", MessageBoxButton.YesNo, MessageBoxImage.Question, MessageBoxResult.Yes))
                {
                    try
                    {

                        File.Create(customRamp).Close();
                        System.Threading.Thread.Sleep(50);

                    }
                    catch (Exception e)
                    {
                        e.ToString();
                        System.Threading.Thread.Sleep(50);
                    }
                    PowerRampEdit();
                }
            }
        }

        private void PowerControlNameSave(object index)
        {
            MVMManager.Instance.ReloadSettings(SettingsFileType.HARDWARE_SETTINGS);

            XmlDocument hardwareDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];

            if (hardwareDoc != null)
            {
                XmlNodeList ndList = hardwareDoc.SelectNodes("/HardwareSettings/PowerControllers/PowerControl");

                if (ndList.Count > 0)
                {
                    PowerControlNameEditWin dlg = new PowerControlNameEditWin();

                    string str = string.Empty;
                    ThorSharedTypes.XmlManager.GetAttribute(ndList[Convert.ToInt32(index)], hardwareDoc, "name", ref str);

                    dlg.PowerControlName = str;

                    if (false == dlg.ShowDialog())
                    {
                        return;
                    }

                    str = dlg.PowerControlName;

                    PowerControlName[Convert.ToInt32(index)].Value = str;

                    ThorSharedTypes.XmlManager.SetAttribute(ndList[Convert.ToInt32(index)], hardwareDoc, "name", str);

                    MVMManager.Instance.SaveSettings(SettingsFileType.HARDWARE_SETTINGS);
                }

            }
        }

        private void PowerMinus(object index)
        {
            switch (Convert.ToInt32(index))
            {
                case 0: Power0 -= 1; break;
                case 1: Power1 -= 1; break;
                case 2: Power2 -= 1; break;
                case 3: Power3 -= 1; break;
                case 4: PowerReg -= PowerStepSize; break;
                case 5: PowerReg2 -= Power2StepSize; break;
            }
        }

        private void PowerPlus(object index)
        {
            switch (Convert.ToInt32(index))
            {
                case 0: Power0 += 1; break;
                case 1: Power1 += 1; break;
                case 2: Power2 += 1; break;
                case 3: Power3 += 1; break;
                case 4: PowerReg += PowerStepSize; break;
                case 5: PowerReg2 += Power2StepSize; break;
            }
        }

        private void PowerRampAdd()
        {
            PowerPlotAddWin dlg = new PowerPlotAddWin();

            if (true == dlg.ShowDialog())
            {
                foreach (char ic in Path.GetInvalidFileNameChars())
                {
                    if (dlg.PowerRampName.Contains(ic))
                    {
                        MessageBox.Show("Invalid file name. Can not contain '" + ic + "'");
                        return;
                    }
                }

                string customRampFolder = Application.Current.Resources["ApplicationSettingsFolder"].ToString() + "\\powerramps\\";

                if (0 == dlg.PowerRampName.Length)
                {
                    MessageBox.Show((null == _powerPlot) ? new Window { Topmost = true } : _powerPlot, "Power Ramp name cannot be empty.");
                    return;
                }
                if (!Directory.Exists(customRampFolder))
                {
                    Directory.CreateDirectory(customRampFolder);
                }
                if (true == File.Exists(customRampFolder + dlg.PowerRampName + ".txt"))
                {
                    MessageBox.Show((null == _powerPlot) ? new Window { Topmost = true } : _powerPlot, "Power Ramp name already exists. Choose a unique name.");
                    return;
                }

                StreamWriter fw = new StreamWriter(customRampFolder + dlg.PowerRampName + ".txt", false);

                switch (dlg.TemplateWaveform)
                {
                    case 0:
                        {
                        }
                        break;
                    case 1:
                        {
                            const double EULER_CONST = 2.71828182845904523536 * 10;

                            double b = Math.Log10(EULER_CONST);

                            for (int i = 0, j = 0; i <= 20; i++, j += 5)
                            {
                                fw.WriteLine(j.ToString() + ',' + Math.Min(100, Math.Max(0, dlg.PowerStart + Math.Pow(EULER_CONST, b * (j / 100.0)) * ((dlg.PowerStop - dlg.PowerStart) / 100.0))));
                            }
                        }
                        break;
                    case 2:
                        {
                            fw.WriteLine("0," + dlg.PowerStart);
                            fw.WriteLine("100," + dlg.PowerStop);
                        }
                        break;
                }

                fw.Close();

                string[] ramps = Directory.GetFiles(customRampFolder, "*.txt");

                const int MaxPowerTags = 6;
                string[] selectedName = new string[MaxPowerTags];

                selectedName[0] = (PowerRampSelected0 >= 0) ? PowerRampsCustom[PowerRampSelected0] : string.Empty;
                selectedName[1] = (PowerRampSelected1 >= 0) ? PowerRampsCustom[PowerRampSelected1] : string.Empty;
                selectedName[2] = (PowerRampSelected2 >= 0) ? PowerRampsCustom[PowerRampSelected2] : string.Empty;
                selectedName[3] = (PowerRampSelected3 >= 0) ? PowerRampsCustom[PowerRampSelected3] : string.Empty;
                selectedName[4] = (PowerRampSelectedReg >= 0) ? PowerRampsCustom[PowerRampSelectedReg] : string.Empty;
                selectedName[5] = (PowerRampSelectedReg2 >= 0) ? PowerRampsCustom[PowerRampSelectedReg2] : string.Empty;

                selectedName[SelectedPowerTab] = dlg.PowerRampName;

                PowerRampsCustom.Clear();

                int[] selected = new int[MaxPowerTags] { -1, -1, -1, -1, -1, -1 };

                for (int i = 0; i < ramps.Length; i++)
                {
                    PowerRampsCustom.Add(Path.GetFileNameWithoutExtension(ramps[i]));
                    for (int j = 0; j < MaxPowerTags; j++)
                    {
                        if (Path.GetFileNameWithoutExtension(ramps[i]).Equals(selectedName[j]))
                        {
                            selected[j] = i;
                        }
                    }
                }

                PowerRampSelected0 = selected[0];
                PowerRampSelected1 = selected[1];
                PowerRampSelected2 = selected[2];
                PowerRampSelected3 = selected[3];
                PowerRampSelectedReg = selected[4];
                PowerRampSelectedReg2 = selected[5];

            }
        }

        private void PowerRampDelete()
        {
            string customRampFolder = Application.Current.Resources["ApplicationSettingsFolder"].ToString() + "\\powerramps\\";

            if (PowerRampsCustom.Count <= 0)
            {
                return;
            }

            if (SelectedRamp < 0)
            {
                return;
            }

            if (MessageBoxResult.Yes == MessageBox.Show((null == _powerPlot) ? new Window { Topmost = true } : _powerPlot, string.Format("Are you sure you want to delete the power ramp {0}", PowerRampsCustom[SelectedRamp]), "Delete", MessageBoxButton.YesNo, MessageBoxImage.Question))
            {
                try
                {
                    File.Delete(customRampFolder + PowerRampsCustom[SelectedRamp] + ".txt");
                }
                catch (Exception ex)
                {
                    ex.ToString();
                }

                string[] ramps = Directory.GetFiles(customRampFolder, "*.txt");

                const int MaxPowerTags = 6;
                string[] selectedName = new string[MaxPowerTags];

                selectedName[0] = (PowerRampSelected0 >= 0) ? PowerRampsCustom[PowerRampSelected0] : string.Empty;
                selectedName[1] = (PowerRampSelected1 >= 0) ? PowerRampsCustom[PowerRampSelected1] : string.Empty;
                selectedName[2] = (PowerRampSelected2 >= 0) ? PowerRampsCustom[PowerRampSelected2] : string.Empty;
                selectedName[3] = (PowerRampSelected3 >= 0) ? PowerRampsCustom[PowerRampSelected3] : string.Empty;
                selectedName[4] = (PowerRampSelectedReg >= 0) ? PowerRampsCustom[PowerRampSelectedReg] : string.Empty;
                selectedName[5] = (PowerRampSelectedReg2 >= 0) ? PowerRampsCustom[PowerRampSelectedReg2] : string.Empty;

                PowerRampsCustom.Clear();

                int[] selected = new int[MaxPowerTags] { -1, -1, -1, -1, -1, -1 };

                for (int i = 0; i < ramps.Length; i++)
                {
                    PowerRampsCustom.Add(Path.GetFileNameWithoutExtension(ramps[i]));
                    for (int j = 0; j < MaxPowerTags; j++)
                    {
                        if (Path.GetFileNameWithoutExtension(ramps[i]).Equals(selectedName[j]))
                        {
                            selected[j] = i;
                        }
                    }

                }

                PowerRampSelected0 = selected[0];
                PowerRampSelected1 = selected[1];
                PowerRampSelected2 = selected[2];
                PowerRampSelected3 = selected[3];
                PowerRampSelectedReg = selected[4];
                PowerRampSelectedReg2 = selected[5];

                if (PowerRampsCustom.Count <= 0)
                {
                    if ((_powerPlot != null) && (_powerPlot.IsLoaded))
                    {
                        _powerPlot.Close();
                    }

                }
            }
        }

        private void PowerRampEdit()
        {
            if (PowerRampsCustom.Count <= 0)
            {
                return;
            }
            if (SelectedRamp < 0)
            {
                return;
            }

            if ((_powerPlot == null) || (_powerPlot.IsLoaded == false))
            {
                _powerPlot = new PowerPlotWin();
                _powerPlot.Closed += _powerPlot_Closed;
                _powerPlot.DataContext = this;
                _powerPlot.Title = string.Format("Power Ramp - {0}", PowerRampsCustom[SelectedRamp]);
                _powerPlot.SetData(PowerRampPlotX, PowerRampPlotY, true);
                _powerPlot.Show();
            }
            else
            {             //the plot is already created. update the data
                _powerPlot.SetData(PowerRampPlotX, PowerRampPlotY, true);
            }
        }

        private void PowerRampEditChanged(int index)
        {
            if ((_powerPlot == null) || (_powerPlot.IsLoaded == false))
            {
                //do nothing on
            }
            else
            {
                if ((SelectedPowerTab == index) && (PowerRampsCustom.Count > 0) && (SelectedRamp < PowerRampsCustom.Count) && (0 <= SelectedRamp))
                {
                    _powerPlot.Title = string.Format("Power Ramp - {0}", PowerRampsCustom[SelectedRamp]);
                    //the plot is already created. update the data
                    _powerPlot.SetData(PowerRampPlotX, PowerRampPlotY, true);
                }
            }
        }

        private void PowerRecordPlot()
        {
            double zScanStart = (double)MVMManager.Instance["ZControlViewModel", "ZScanStart", (object)0.0];
            double zScanStop = (double)MVMManager.Instance["ZControlViewModel", "ZScanStop", (object)0.0];

            double step = (zScanStop - zScanStart) / MAX_POWER_RAMP_DATA_POINTS;

            if (0 == step)
            {
                MessageBox.Show("Could not record data point. The Start and Stop positions of the Z Stack are the same. Please change the values of Start and/or Stop in Z Control.", "Z Stack range is 0", MessageBoxButton.OK, MessageBoxImage.Error);
                return;
            }

            double currentZPosition = (double)MVMManager.Instance["ZControlViewModel", "ZPosition", (object)0.0];
            double powerRampPosition = (step >= 0) ? Math.Max(currentZPosition, zScanStart) : Math.Min(currentZPosition, zScanStart);

            Decimal zRangePercentage = new Decimal(Math.Abs((powerRampPosition - zScanStart) / step));

            int loc = Math.Min(Convert.ToInt32(Decimal.Round(zRangePercentage)), MAX_POWER_RAMP_DATA_POINTS);

            if (SelectedRamp >= 0)
            {
                string customRamp = Application.Current.Resources["ApplicationSettingsFolder"].ToString() + "\\powerramps\\" + PowerRampsCustom[SelectedRamp] + ".txt";

                StreamReader fs = new StreamReader(customRamp);
                string line;
                int counter = 0;

                SortedDictionary<double, double> xyData = new SortedDictionary<double, double>();

                try
                {
                    xyData.Add(loc, PowerPositionActive);

                    while ((line = fs.ReadLine()) != null)
                    {
                        string[] split = line.Split(',');

                        if (split[0] != null)
                        {
                            if (split[1] != null)
                            {
                                if (Convert.ToInt32(split[0]) != loc)
                                {
                                    xyData.Add(Convert.ToDouble(split[0]), Convert.ToDouble(split[1]));
                                }
                            }
                        }
                        counter++;
                    }
                }
                catch (Exception ex)
                {
                    string msg = ex.Message;
                }

                fs.Close();

                if (counter <= MAX_POWER_RAMP_DATA_POINTS)
                {

                    StreamWriter fw = new StreamWriter(customRamp, false);

                    foreach (KeyValuePair<double, double> p in xyData)
                    {
                        fw.WriteLine(p.Key.ToString() + ',' + p.Value.ToString());
                    }

                    fw.Close();
                }
            }

            PowerRampEdit();
        }

        private void PowerReg2Cal(object index)
        {
            try
            {
                XmlDocument hardwareDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];

                if (null != hardwareDoc)
                {
                    XmlNodeList ndList = hardwareDoc.SelectNodes("/HardwareSettings/PowerReg2");

                    if (ndList.Count > 0)
                    {
                        string str = string.Empty;

                        switch (Convert.ToInt32(index))
                        {
                            case 1:
                                if (ThorSharedTypes.XmlManager.GetAttribute(ndList[0], hardwareDoc, "calib1", ref str))
                                {
                                    PowerReg2Zero = Convert.ToDouble(str);
                                }
                                break;
                            case 2:
                                if (ThorSharedTypes.XmlManager.GetAttribute(ndList[0], hardwareDoc, "calib2", ref str))
                                {
                                    PowerReg2Zero = Convert.ToDouble(str);
                                }
                                break;
                            case 3:
                                if (ThorSharedTypes.XmlManager.GetAttribute(ndList[0], hardwareDoc, "calib3", ref str))
                                {
                                    PowerReg2Zero = Convert.ToDouble(str);
                                }
                                break;
                            case 4:
                                if (ThorSharedTypes.XmlManager.GetAttribute(ndList[0], hardwareDoc, "calib4", ref str))
                                {
                                    PowerReg2Zero = Convert.ToDouble(str);
                                }
                                break;
                            case 5:
                                if (ThorSharedTypes.XmlManager.GetAttribute(ndList[0], hardwareDoc, "calib5", ref str))
                                {
                                    PowerReg2Zero = Convert.ToDouble(str);
                                }
                                break;
                            case 6:
                                if (ThorSharedTypes.XmlManager.GetAttribute(ndList[0], hardwareDoc, "calib6", ref str))
                                {
                                    PowerReg2Zero = Convert.ToDouble(str);
                                }
                                break;
                        }
                    }
                }
            }
            catch (Exception e)
            {
                e.ToString();
            }
        }

        private void PowerReg2CalSave(object index)
        {
            MVMManager.Instance.ReloadSettings(SettingsFileType.HARDWARE_SETTINGS);

            XmlDocument hardwareDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];

            if (null != hardwareDoc)
            {
                XmlNodeList ndList = hardwareDoc.SelectNodes("/HardwareSettings/PowerReg2");

                double prev2Zero = PowerReg2Zero;

                _powerControlModel.PowerReg2CalibrateZero();

                if (ndList.Count > 0)
                {
                    PowerRegCalEditWin dlg = new PowerRegCalEditWin();

                    string str = string.Empty;
                    string strOffset = string.Empty;

                    switch (Convert.ToInt32(index))
                    {
                        case 1: ThorSharedTypes.XmlManager.GetAttribute(ndList[0], hardwareDoc, "calibName1", ref str); ThorSharedTypes.XmlManager.GetAttribute(ndList[0], hardwareDoc, "calib1", ref strOffset); break;
                        case 2: ThorSharedTypes.XmlManager.GetAttribute(ndList[0], hardwareDoc, "calibName2", ref str); ThorSharedTypes.XmlManager.GetAttribute(ndList[0], hardwareDoc, "calib2", ref strOffset); break;
                        case 3: ThorSharedTypes.XmlManager.GetAttribute(ndList[0], hardwareDoc, "calibName3", ref str); ThorSharedTypes.XmlManager.GetAttribute(ndList[0], hardwareDoc, "calib3", ref strOffset); break;
                        case 4: ThorSharedTypes.XmlManager.GetAttribute(ndList[0], hardwareDoc, "calibName4", ref str); ThorSharedTypes.XmlManager.GetAttribute(ndList[0], hardwareDoc, "calib4", ref strOffset); break;
                        case 5: ThorSharedTypes.XmlManager.GetAttribute(ndList[0], hardwareDoc, "calibName5", ref str); ThorSharedTypes.XmlManager.GetAttribute(ndList[0], hardwareDoc, "calib5", ref strOffset); break;
                        case 6: ThorSharedTypes.XmlManager.GetAttribute(ndList[0], hardwareDoc, "calibName6", ref str); ThorSharedTypes.XmlManager.GetAttribute(ndList[0], hardwareDoc, "calib6", ref strOffset); break;
                    }

                    dlg.PowerCalName = str;
                    dlg.PowerCalOffset = PowerReg2Zero.ToString();

                    if (false == dlg.ShowDialog())
                    {
                        PowerReg2Zero = prev2Zero;
                        return;
                    }

                    OnPropertyChanged("PowerReg2Zero");

                    str = dlg.PowerCalName;

                    switch (Convert.ToInt32(index))
                    {
                        case 1: ThorSharedTypes.XmlManager.SetAttribute(ndList[0], hardwareDoc, "calibName1", str); ThorSharedTypes.XmlManager.SetAttribute(ndList[0], hardwareDoc, "calib1", PowerReg2Zero.ToString()); PowerReg2CalName1 = str; break;
                        case 2: ThorSharedTypes.XmlManager.SetAttribute(ndList[0], hardwareDoc, "calibName2", str); ThorSharedTypes.XmlManager.SetAttribute(ndList[0], hardwareDoc, "calib2", PowerReg2Zero.ToString()); PowerReg2CalName2 = str; break;
                        case 3: ThorSharedTypes.XmlManager.SetAttribute(ndList[0], hardwareDoc, "calibName3", str); ThorSharedTypes.XmlManager.SetAttribute(ndList[0], hardwareDoc, "calib3", PowerReg2Zero.ToString()); PowerReg2CalName3 = str; break;
                        case 4: ThorSharedTypes.XmlManager.SetAttribute(ndList[0], hardwareDoc, "calibName4", str); ThorSharedTypes.XmlManager.SetAttribute(ndList[0], hardwareDoc, "calib4", PowerReg2Zero.ToString()); PowerReg2CalName4 = str; break;
                        case 5: ThorSharedTypes.XmlManager.SetAttribute(ndList[0], hardwareDoc, "calibName5", str); ThorSharedTypes.XmlManager.SetAttribute(ndList[0], hardwareDoc, "calib5", PowerReg2Zero.ToString()); PowerReg2CalName5 = str; break;
                        case 6: ThorSharedTypes.XmlManager.SetAttribute(ndList[0], hardwareDoc, "calibName6", str); ThorSharedTypes.XmlManager.SetAttribute(ndList[0], hardwareDoc, "calib6", PowerReg2Zero.ToString()); PowerReg2CalName6 = str; break;
                    }

                    MVMManager.Instance.SaveSettings(SettingsFileType.HARDWARE_SETTINGS);
                }
            }
        }

        private void PowerRegCal(object index)
        {
            try
            {
                XmlDocument hardwareDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];

                if (null != hardwareDoc)
                {
                    XmlNodeList ndList = hardwareDoc.SelectNodes("/HardwareSettings/PowerReg");

                    if (ndList.Count > 0)
                    {
                        string str = string.Empty;

                        switch (Convert.ToInt32(index))
                        {
                            case 1:
                                if (XmlManager.GetAttribute(ndList[0], hardwareDoc, "calib1", ref str))
                                {
                                    PowerRegZero = Convert.ToDouble(str);
                                }
                                break;
                            case 2:
                                if (XmlManager.GetAttribute(ndList[0], hardwareDoc, "calib2", ref str))
                                {
                                    PowerRegZero = Convert.ToDouble(str);
                                }
                                break;
                            case 3:
                                if (XmlManager.GetAttribute(ndList[0], hardwareDoc, "calib3", ref str))
                                {
                                    PowerRegZero = Convert.ToDouble(str);
                                }
                                break;
                            case 4:
                                if (XmlManager.GetAttribute(ndList[0], hardwareDoc, "calib4", ref str))
                                {
                                    PowerRegZero = Convert.ToDouble(str);
                                }
                                break;
                            case 5:
                                if (XmlManager.GetAttribute(ndList[0], hardwareDoc, "calib5", ref str))
                                {
                                    PowerRegZero = Convert.ToDouble(str);
                                }
                                break;
                            case 6:
                                if (XmlManager.GetAttribute(ndList[0], hardwareDoc, "calib6", ref str))
                                {
                                    PowerRegZero = Convert.ToDouble(str);
                                }
                                break;
                        }
                    }

                }
            }
            catch (Exception e)
            {
                e.ToString();
            }
        }

        private void PowerRegCalSave(object index)
        {
            MVMManager.Instance.ReloadSettings(SettingsFileType.HARDWARE_SETTINGS);

            XmlDocument hardwareDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS];

            if (null != hardwareDoc)
            {
                XmlNodeList ndList = hardwareDoc.SelectNodes("/HardwareSettings/PowerReg");

                double prevZero = PowerRegZero;

                _powerControlModel.PowerRegCalibrateZero();

                if (ndList.Count > 0)
                {
                    PowerRegCalEditWin dlg = new PowerRegCalEditWin();

                    string str = string.Empty;
                    string strOffset = string.Empty;

                    switch (Convert.ToInt32(index))
                    {
                        case 1: ThorSharedTypes.XmlManager.GetAttribute(ndList[0], hardwareDoc, "calibName1", ref str); ThorSharedTypes.XmlManager.GetAttribute(ndList[0], hardwareDoc, "calib1", ref strOffset); break;
                        case 2: ThorSharedTypes.XmlManager.GetAttribute(ndList[0], hardwareDoc, "calibName2", ref str); ThorSharedTypes.XmlManager.GetAttribute(ndList[0], hardwareDoc, "calib2", ref strOffset); break;
                        case 3: ThorSharedTypes.XmlManager.GetAttribute(ndList[0], hardwareDoc, "calibName3", ref str); ThorSharedTypes.XmlManager.GetAttribute(ndList[0], hardwareDoc, "calib3", ref strOffset); break;
                        case 4: ThorSharedTypes.XmlManager.GetAttribute(ndList[0], hardwareDoc, "calibName4", ref str); ThorSharedTypes.XmlManager.GetAttribute(ndList[0], hardwareDoc, "calib4", ref strOffset); break;
                        case 5: ThorSharedTypes.XmlManager.GetAttribute(ndList[0], hardwareDoc, "calibName5", ref str); ThorSharedTypes.XmlManager.GetAttribute(ndList[0], hardwareDoc, "calib5", ref strOffset); break;
                        case 6: ThorSharedTypes.XmlManager.GetAttribute(ndList[0], hardwareDoc, "calibName6", ref str); ThorSharedTypes.XmlManager.GetAttribute(ndList[0], hardwareDoc, "calib6", ref strOffset); break;
                    }

                    dlg.PowerCalName = str;
                    dlg.PowerCalOffset = PowerRegZero.ToString();

                    if (false == dlg.ShowDialog())
                    {
                        PowerRegZero = prevZero;
                        return;
                    }

                    OnPropertyChanged("PowerRegZero");

                    str = dlg.PowerCalName;

                    switch (Convert.ToInt32(index))
                    {
                        case 1: ThorSharedTypes.XmlManager.SetAttribute(ndList[0], hardwareDoc, "calibName1", str); ThorSharedTypes.XmlManager.SetAttribute(ndList[0], hardwareDoc, "calib1", PowerRegZero.ToString()); PowerRegCal1Name = str; break;
                        case 2: ThorSharedTypes.XmlManager.SetAttribute(ndList[0], hardwareDoc, "calibName2", str); ThorSharedTypes.XmlManager.SetAttribute(ndList[0], hardwareDoc, "calib2", PowerRegZero.ToString()); PowerRegCal2Name = str; break;
                        case 3: ThorSharedTypes.XmlManager.SetAttribute(ndList[0], hardwareDoc, "calibName3", str); ThorSharedTypes.XmlManager.SetAttribute(ndList[0], hardwareDoc, "calib3", PowerRegZero.ToString()); PowerRegCal3Name = str; break;
                        case 4: ThorSharedTypes.XmlManager.SetAttribute(ndList[0], hardwareDoc, "calibName4", str); ThorSharedTypes.XmlManager.SetAttribute(ndList[0], hardwareDoc, "calib4", PowerRegZero.ToString()); PowerRegCal4Name = str; break;
                        case 5: ThorSharedTypes.XmlManager.SetAttribute(ndList[0], hardwareDoc, "calibName5", str); ThorSharedTypes.XmlManager.SetAttribute(ndList[0], hardwareDoc, "calib5", PowerRegZero.ToString()); PowerRegCal5Name = str; break;
                        case 6: ThorSharedTypes.XmlManager.SetAttribute(ndList[0], hardwareDoc, "calibName6", str); ThorSharedTypes.XmlManager.SetAttribute(ndList[0], hardwareDoc, "calib6", PowerRegZero.ToString()); PowerRegCal6Name = str; break;
                    }

                    MVMManager.Instance.SaveSettings(SettingsFileType.HARDWARE_SETTINGS);
                }
            }
        }

        private void SelectPockelsMask()
        {
            // Configure open file dialog box
            Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();
            dlg.FileName = "Raw"; // Default file name
            dlg.DefaultExt = ".raw"; // Default file extension
            dlg.Filter = "(*.tif, *.raw)|*.tif;*.raw"; // Filter files by extension

            // Show open file dialog box
            Nullable<bool> result = dlg.ShowDialog();

            // Process open file dialog box results
            if (result == true)
            {
                // Open document
                this.PockelsMaskFile = dlg.FileName;
            }
        }

        private void SetBleacherPower()
        {
            if ((0 < BleacherPockelsList.Count) && (0 <= BleacherPowerID))
            {
                switch (BleacherPockelsList[BleacherPowerID])
                {
                    case 1: this._powerControlModel.BleacherPower0 = BleacherPowerGo; break;
                    case 2: this._powerControlModel.BleacherPower1 = BleacherPowerGo; break;
                    case 3: this._powerControlModel.BleacherPower2 = BleacherPowerGo; break;
                    case 4: this._powerControlModel.BleacherPower3 = BleacherPowerGo; break;
                }
            }
        }

        private void SetBleacherPowerToZero()
        {
            if ((0 < BleacherPockelsList.Count) && (0 <= BleacherPowerID))
            {
                switch (BleacherPockelsList[BleacherPowerID])
                {
                    case 1: this._powerControlModel.BleacherPower0 = 0; break;
                    case 2: this._powerControlModel.BleacherPower1 = 0; break;
                    case 3: this._powerControlModel.BleacherPower2 = 0; break;
                    case 4: this._powerControlModel.BleacherPower3 = 0; break;
                }
            }
        }

        private void SetPower(object index)
        {
            switch (Convert.ToInt32(index))
            {
                case 0: Power0 = PowerGo0; break;
                case 1: Power1 = PowerGo1; break;
                case 2: Power2 = PowerGo2; break;
                case 3: Power3 = PowerGo3; break;
                case 4: PowerReg = PowerGoReg; break;
                case 5: PowerReg2 = PowerGoReg2; break;
            }
        }

        private void UpdatePockelsMaskToROIMask()
        {
            this.PockelsMaskFile = ThorSharedTypes.ResourceManagerCS.GetCaptureTemplatePathString() + "ActiveROIMask.raw";
        }

        private void UpdatePockelsNodeList()
        {
            if (null == _pockelsNodeList.Key || null == _pockelsNodeList.Value)
                return;

            for (int i = 0; i < _pockelsNodeList.Key.Count; i++)
            {
                switch (i)
                {
                    case 0:
                        if (PowerRampSelected0 < 0)
                        {
                            PowerMode0 = 0;
                        }
                        XmlManager.SetAttribute(_pockelsNodeList.Key[i], _pockelsNodeList.Value, "type", PowerMode0.ToString());
                        XmlManager.SetAttribute(_pockelsNodeList.Key[i], _pockelsNodeList.Value, "pockelsBlankPercentage", PockelsBlankPercentage0.ToString());
                        XmlManager.SetAttribute(_pockelsNodeList.Key[i], _pockelsNodeList.Value, "maskEnable", EnablePockelsMask.ToString());
                        XmlManager.SetAttribute(_pockelsNodeList.Key[i], _pockelsNodeList.Value, "maskInvert", PockelsMaskInvert0.ToString());
                        XmlManager.SetAttribute(_pockelsNodeList.Key[i], _pockelsNodeList.Value, "maskPath", PockelsMaskFile.ToString());
                        XmlManager.SetAttribute(_pockelsNodeList.Key[i], _pockelsNodeList.Value, "pockelsMinV", PockelsVoltageMin0.ToString());
                        XmlManager.SetAttribute(_pockelsNodeList.Key[i], _pockelsNodeList.Value, "pockelsMaxV", PockelsVoltageMax0.ToString());
                        XmlManager.SetAttribute(_pockelsNodeList.Key[i], _pockelsNodeList.Value, "start", Power0.ToString());
                        XmlManager.SetAttribute(_pockelsNodeList.Key[i], _pockelsNodeList.Value, "stop", Power0.ToString());

                        if ((PowerRampSelected0 < PowerRampsCustom.Count) && (PowerRampSelected0 >= 0) && (1 == PowerMode0))
                        {
                            XmlManager.SetAttribute(_pockelsNodeList.Key[i], _pockelsNodeList.Value, "path", Application.Current.Resources["ApplicationSettingsFolder"].ToString() + "\\powerramps\\" + PowerRampsCustom[PowerRampSelected0] + ".txt");
                        }
                        else
                        {
                            XmlManager.SetAttribute(_pockelsNodeList.Key[i], _pockelsNodeList.Value, "path", "");
                        }
                        break;
                    case 1:
                        if (PowerRampSelected1 < 0)
                        {
                            PowerMode1 = 0;
                        }
                        XmlManager.SetAttribute(_pockelsNodeList.Key[i], _pockelsNodeList.Value, "type", PowerMode1.ToString());
                        XmlManager.SetAttribute(_pockelsNodeList.Key[i], _pockelsNodeList.Value, "pockelsBlankPercentage", PockelsBlankPercentage1.ToString());
                        XmlManager.SetAttribute(_pockelsNodeList.Key[i], _pockelsNodeList.Value, "pockelsMinV", PockelsVoltageMin1.ToString());
                        XmlManager.SetAttribute(_pockelsNodeList.Key[i], _pockelsNodeList.Value, "pockelsMaxV", PockelsVoltageMax1.ToString());
                        XmlManager.SetAttribute(_pockelsNodeList.Key[i], _pockelsNodeList.Value, "start", Power1.ToString());
                        XmlManager.SetAttribute(_pockelsNodeList.Key[i], _pockelsNodeList.Value, "stop", Power1.ToString());

                        if ((PowerRampSelected1 < PowerRampsCustom.Count) && (PowerRampSelected1 >= 0) && (1 == PowerMode1))
                        {
                            XmlManager.SetAttribute(_pockelsNodeList.Key[i], _pockelsNodeList.Value, "path", Application.Current.Resources["ApplicationSettingsFolder"].ToString() + "\\powerramps\\" + PowerRampsCustom[PowerRampSelected1] + ".txt");
                        }
                        else
                        {
                            XmlManager.SetAttribute(_pockelsNodeList.Key[i], _pockelsNodeList.Value, "path", "");
                        }
                        break;
                    case 2:
                        if (PowerRampSelected2 < 0)
                        {
                            PowerMode2 = 0;
                        }
                        XmlManager.SetAttribute(_pockelsNodeList.Key[i], _pockelsNodeList.Value, "type", PowerMode2.ToString());
                        XmlManager.SetAttribute(_pockelsNodeList.Key[i], _pockelsNodeList.Value, "pockelsBlankPercentage", PockelsBlankPercentage2.ToString());
                        XmlManager.SetAttribute(_pockelsNodeList.Key[i], _pockelsNodeList.Value, "pockelsMinV", PockelsVoltageMin2.ToString());
                        XmlManager.SetAttribute(_pockelsNodeList.Key[i], _pockelsNodeList.Value, "pockelsMaxV", PockelsVoltageMax2.ToString());
                        XmlManager.SetAttribute(_pockelsNodeList.Key[i], _pockelsNodeList.Value, "start", Power2.ToString());
                        XmlManager.SetAttribute(_pockelsNodeList.Key[i], _pockelsNodeList.Value, "stop", Power2.ToString());

                        if ((PowerRampSelected2 < PowerRampsCustom.Count) && (PowerRampSelected2 >= 0) && (1 == PowerMode2))
                        {
                            XmlManager.SetAttribute(_pockelsNodeList.Key[i], _pockelsNodeList.Value, "path", Application.Current.Resources["ApplicationSettingsFolder"].ToString() + "\\powerramps\\" + PowerRampsCustom[PowerRampSelected2] + ".txt");
                        }
                        else
                        {
                            XmlManager.SetAttribute(_pockelsNodeList.Key[i], _pockelsNodeList.Value, "path", "");
                        }
                        break;
                    case 3:
                        if (PowerRampSelected3 < 0)
                        {
                            PowerMode3 = 0;
                        }
                        XmlManager.SetAttribute(_pockelsNodeList.Key[i], _pockelsNodeList.Value, "type", PowerMode3.ToString());
                        XmlManager.SetAttribute(_pockelsNodeList.Key[i], _pockelsNodeList.Value, "pockelsBlankPercentage", PockelsBlankPercentage3.ToString());
                        XmlManager.SetAttribute(_pockelsNodeList.Key[i], _pockelsNodeList.Value, "pockelsMinV", PockelsVoltageMin3.ToString());
                        XmlManager.SetAttribute(_pockelsNodeList.Key[i], _pockelsNodeList.Value, "pockelsMaxV", PockelsVoltageMax3.ToString());
                        XmlManager.SetAttribute(_pockelsNodeList.Key[i], _pockelsNodeList.Value, "start", Power3.ToString());
                        XmlManager.SetAttribute(_pockelsNodeList.Key[i], _pockelsNodeList.Value, "stop", Power3.ToString());

                        if ((PowerRampSelected3 < PowerRampsCustom.Count) && (PowerRampSelected3 >= 0) && (1 == PowerMode3))
                        {
                            XmlManager.SetAttribute(_pockelsNodeList.Key[i], _pockelsNodeList.Value, "path", Application.Current.Resources["ApplicationSettingsFolder"].ToString() + "\\powerramps\\" + PowerRampsCustom[PowerRampSelected3] + ".txt");
                        }
                        else
                        {
                            XmlManager.SetAttribute(_pockelsNodeList.Key[i], _pockelsNodeList.Value, "path", "");
                        }
                        break;
                }
            }
        }

        private void UpdateStartPower()
        {
            switch (_selectedPowerTab)
            {
                case 0: Power0 = CustomStartPower; break;
                case 1: Power1 = CustomStartPower; break;
                case 2: Power2 = CustomStartPower; break;
                case 3: Power3 = CustomStartPower; break;
                case 4: PowerReg = CustomStartPower; break;
                case 5: PowerReg2 = CustomStartPower; break;
            }
        }

        void _powerPlot_Closed(object sender, EventArgs e)
        {
            _powerPlot = null;
        }

        #endregion Methods
    }
}