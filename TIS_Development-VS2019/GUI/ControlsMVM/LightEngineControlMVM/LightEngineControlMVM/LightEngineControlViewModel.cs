namespace LightEngineControl.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Reflection;
    using System.Runtime.CompilerServices;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Xml;

    using LightEngineControl.Model;

    using OverlayManager;

    using ThorLogging;

    using ThorSharedTypes;

    public class LightEngineControlViewModel : ThorSharedTypes.VMBase, ThorSharedTypes.IMVM
    {
        #region Fields

        private readonly LightEngineControlModel _lightEngineControlModel = new LightEngineControlModel();
        private readonly Dictionary<String, PropertyInfo> _properties = new Dictionary<String, PropertyInfo>();

        private SolidColorBrush _lED1LightColor = default(SolidColorBrush);
        private ICommand _led1MinusCommand;
        private ICommand _led1PlusCommand;
        private bool _led1PowerState = false;
        private SolidColorBrush _lED2LightColor = default(SolidColorBrush);
        private ICommand _led2MinusCommand;
        private ICommand _led2PlusCommand;
        private bool _led2PowerState = false;
        private SolidColorBrush _lED3LightColor = default(SolidColorBrush);
        private ICommand _led3MinusCommand;
        private ICommand _led3PlusCommand;
        private bool _led3PowerState = false;
        private SolidColorBrush _lED4LightColor = default(SolidColorBrush);
        private ICommand _led4MinusCommand;
        private ICommand _led4PlusCommand;
        private bool _led4PowerState = false;
        private SolidColorBrush _lED5LightColor = default(SolidColorBrush);
        private ICommand _led5MinusCommand;
        private ICommand _led5PlusCommand;
        private bool _led5PowerState = false;
        private SolidColorBrush _lED6LightColor = default(SolidColorBrush);
        private ICommand _led6MinusCommand;
        private ICommand _led6PlusCommand;
        private bool _led6PowerState = false;
        private Int32 _linearModeSettingsSelectedIdx = 0;
        private ObservableCollection<String> _linearModeSettingsSource = new ObservableCollection<String>() { "I", "II", "III" };
        private Visibility _masterControlVis = Visibility.Collapsed;
        private ICommand _masterPowerMinusCommand;
        private ICommand _masterPowerPlusCommand;
        private Boolean _showTemperatures = false;
        private Thickness _externalModeMargin = new Thickness(0, 0, 0, 0);
        private String _temperatureUnit = "ºC";

        #endregion Fields

        #region Constructors

        public LightEngineControlViewModel()
        {
            //this._lightEngineControlModel = new LightEngineControlModel();
        }

        #endregion Constructors

        #region Properties

        public Boolean EnableDisableLEDs
        {
            get
            {
                return this._lightEngineControlModel.EnableDisableLEDs;
            }
            set
            {
                this._lightEngineControlModel.EnableDisableLEDs = value;
                OnPropertyChanged("EnableDisableLEDs");
            }
        }

        public String LED1ControlName
        {
            get
            {
                return this._lightEngineControlModel.LED1ControlName;
            }
            set
            {
                if (!this._lightEngineControlModel.LED1ControlName.Equals(value))
                {
                    this._lightEngineControlModel.LED1ControlName = value;
                    this.OnPropertyChange();
                }
            }
        }

        public SolidColorBrush LED1LightColor
        {
            get
            {
                int colorCode = _lightEngineControlModel.LED1LightColor;
                var redValue = colorCode / 65536;
                var greenValue = (colorCode - redValue * 65536) / 256;
                var blueValue = colorCode - redValue * 65536 - greenValue * 256;
                // If the power is not enabled set the background of the color opacity to be lower to 38% (100% is 255)
                _lED1LightColor = (LED1PowerState) ? new SolidColorBrush(Color.FromArgb(255, Convert.ToByte(redValue), Convert.ToByte(greenValue), Convert.ToByte(blueValue))) : new SolidColorBrush(Color.FromArgb(97, Convert.ToByte(redValue), Convert.ToByte(greenValue), Convert.ToByte(blueValue)));
                return _lED1LightColor;
            }
            set
            {
            }
        }

        public String LED1PeakToolTip
        {
            get
            {
                return "Peak Wavelength: " + this._lightEngineControlModel.LED1PeakToolTip;
            }
        }

        public Double LED1Power
        {
            get
            {
                return this._lightEngineControlModel.LED1Power;
            }
            set
            {
                if (!this._lightEngineControlModel.LED1Power.Equals(value))
                {
                    this._lightEngineControlModel.LED1Power = value;
                    this.OnPropertyChange();
                }
            }
        }

        public ICommand LED1PowerMinusCommand
        {
            get
            {
                if (this._led1MinusCommand == null)
                    this._led1MinusCommand = new RelayCommand(() => LEDMinus(1));

                return this._led1MinusCommand;
            }
        }

        public ICommand LED1PowerPlusCommand
        {
            get
            {
                if (this._led1PlusCommand == null)
                    this._led1PlusCommand = new RelayCommand(() => LEDPlus(1));

                return this._led1PlusCommand;
            }
        }

        public Boolean LED1PowerState
        {
            get
            {
                //return this._lightEngineControlModel.LED1PowerState;
                return _led1PowerState;
            }
            set
            {
                _led1PowerState = value;
                if (!this._lightEngineControlModel.LED1PowerState.Equals(_led1PowerState))
                {
                    this._lightEngineControlModel.LED1PowerState = _led1PowerState;
                }
                OnPropertyChanged("LED1PowerState");
                OnPropertyChanged("LED1LightColor");
            }
        }

        public Int32 LED1SockelID
        {
            get
            {
                return this._lightEngineControlModel.LED1SockelID;
            }
            set
            {
                if (!this._lightEngineControlModel.LED1SockelID.Equals(value))
                {
                    this._lightEngineControlModel.LED1SockelID = value;
                    this.OnPropertyChange();
                }
            }
        }

        public Double LED1Temperature
        {
            get
            {
                return this._lightEngineControlModel.LED1Temperature;
            }
            set
            {
                if (!this._lightEngineControlModel.LED1Temperature.Equals(value))
                {
                    this._lightEngineControlModel.LED1Temperature = value;
                    this.OnPropertyChange();
                }
            }
        }

        public String LED2ControlName
        {
            get
            {
                return this._lightEngineControlModel.LED2ControlName;
            }
            set
            {
                if (!this._lightEngineControlModel.LED2ControlName.Equals(value))
                {
                    this._lightEngineControlModel.LED2ControlName = value;
                    this.OnPropertyChange();
                }
            }
        }

        public SolidColorBrush LED2LightColor
        {
            get
            {
                int colorCode = _lightEngineControlModel.LED2LightColor;
                var redValue = colorCode / 65536;
                var greenValue = (colorCode - redValue * 65536) / 256;
                var blueValue = colorCode - redValue * 65536 - greenValue * 256;
                // If the power is not enabled set the background of the color opacity to be lower to 38% (100% is 255)
                _lED2LightColor = (LED2PowerState) ? new SolidColorBrush(Color.FromArgb(255, Convert.ToByte(redValue), Convert.ToByte(greenValue), Convert.ToByte(blueValue))) : new SolidColorBrush(Color.FromArgb(97, Convert.ToByte(redValue), Convert.ToByte(greenValue), Convert.ToByte(blueValue)));
                return _lED2LightColor;
            }
            set
            {
            }
        }

        public String LED2PeakToolTip
        {
            get
            {
                return "Peak Wavelength: " + this._lightEngineControlModel.LED2PeakToolTip;
            }
        }

        public Double LED2Power
        {
            get
            {
                return this._lightEngineControlModel.LED2Power;
            }
            set
            {
                if (!this._lightEngineControlModel.LED2Power.Equals(value))
                {
                    this._lightEngineControlModel.LED2Power = value;
                    this.OnPropertyChange();
                }
            }
        }

        public ICommand LED2PowerMinusCommand
        {
            get
            {
                if (this._led2MinusCommand == null)
                    this._led2MinusCommand = new RelayCommand(() => LEDMinus(2));

                return this._led2MinusCommand;
            }
        }

        public ICommand LED2PowerPlusCommand
        {
            get
            {
                if (this._led2PlusCommand == null)
                    this._led2PlusCommand = new RelayCommand(() => LEDPlus(2));

                return this._led2PlusCommand;
            }
        }

        public Boolean LED2PowerState
        {
            get
            {
                //return this._lightEngineControlModel.LED2PowerState;
                return _led2PowerState;
            }
            set
            {
                _led2PowerState = value;
                if (!this._lightEngineControlModel.LED2PowerState.Equals(_led2PowerState))
                {
                    this._lightEngineControlModel.LED2PowerState = _led2PowerState;
                }
                OnPropertyChanged("LED2PowerState");
                OnPropertyChanged("LED2LightColor");
            }
        }

        public Int32 LED2SockelID
        {
            get
            {
                return this._lightEngineControlModel.LED2SockelID;
            }
            set
            {
                if (!this._lightEngineControlModel.LED2SockelID.Equals(value))
                {
                    this._lightEngineControlModel.LED2SockelID = value;
                    this.OnPropertyChange();
                }
            }
        }

        public Double LED2Temperature
        {
            get
            {
                return this._lightEngineControlModel.LED2Temperature;
            }
            set
            {
                if (!this._lightEngineControlModel.LED2Temperature.Equals(value))
                {
                    this._lightEngineControlModel.LED2Temperature = value;
                    this.OnPropertyChange();
                }
            }
        }

        public String LED3ControlName
        {
            get
            {
                return this._lightEngineControlModel.LED3ControlName;
            }
            set
            {
                if (!this._lightEngineControlModel.LED3ControlName.Equals(value))
                {
                    this._lightEngineControlModel.LED3ControlName = value;
                    this.OnPropertyChange();
                }
            }
        }

        public SolidColorBrush LED3LightColor
        {
            get
            {
                int colorCode = _lightEngineControlModel.LED3LightColor;
                var redValue = colorCode / 65536;
                var greenValue = (colorCode - redValue * 65536) / 256;
                var blueValue = colorCode - redValue * 65536 - greenValue * 256;
                // If the power is not enabled set the background of the color opacity to be lower to 38% (100% is 255)
                _lED3LightColor = (LED3PowerState) ? new SolidColorBrush(Color.FromArgb(255, Convert.ToByte(redValue), Convert.ToByte(greenValue), Convert.ToByte(blueValue))) : new SolidColorBrush(Color.FromArgb(97, Convert.ToByte(redValue), Convert.ToByte(greenValue), Convert.ToByte(blueValue)));
                return _lED3LightColor;
            }
            set
            {
            }
        }

        public String LED3PeakToolTip
        {
            get
            {
                return "Peak Wavelength: " + this._lightEngineControlModel.LED3PeakToolTip;
            }
        }

        public Double LED3Power
        {
            get
            {
                return this._lightEngineControlModel.LED3Power;
            }
            set
            {
                if (!this._lightEngineControlModel.LED3Power.Equals(value))
                {
                    this._lightEngineControlModel.LED3Power = value;
                    this.OnPropertyChange();
                }
            }
        }

        public ICommand LED3PowerMinusCommand
        {
            get
            {
                if (this._led3MinusCommand == null)
                    this._led3MinusCommand = new RelayCommand(() => LEDMinus(3));

                return this._led3MinusCommand;
            }
        }

        public ICommand LED3PowerPlusCommand
        {
            get
            {
                if (this._led3PlusCommand == null)
                    this._led3PlusCommand = new RelayCommand(() => LEDPlus(3));

                return this._led3PlusCommand;
            }
        }

        public Boolean LED3PowerState
        {
            get
            {
                //return this._lightEngineControlModel.LED3PowerState;
                return _led3PowerState;
            }
            set
            {
                _led3PowerState = value;
                if (!this._lightEngineControlModel.LED3PowerState.Equals(_led3PowerState))
                {
                    this._lightEngineControlModel.LED3PowerState = _led3PowerState;
                }
                OnPropertyChanged("LED3PowerState");
                OnPropertyChanged("LED3LightColor");
            }
        }

        public Int32 LED3SockelID
        {
            get
            {
                return this._lightEngineControlModel.LED3SockelID;
            }
            set
            {
                if (!this._lightEngineControlModel.LED3SockelID.Equals(value))
                {
                    this._lightEngineControlModel.LED3SockelID = value;
                    this.OnPropertyChange();
                }
            }
        }

        public Double LED3Temperature
        {
            get
            {
                return this._lightEngineControlModel.LED3Temperature;
            }
            set
            {
                if (!this._lightEngineControlModel.LED3Temperature.Equals(value))
                {
                    this._lightEngineControlModel.LED3Temperature = value;
                    this.OnPropertyChange();
                }
            }
        }

        public String LED4ControlName
        {
            get
            {
                return this._lightEngineControlModel.LED4ControlName;
            }
            set
            {
                if (!this._lightEngineControlModel.LED4ControlName.Equals(value))
                {
                    this._lightEngineControlModel.LED4ControlName = value;
                    this.OnPropertyChange();
                }
            }
        }

        public SolidColorBrush LED4LightColor
        {
            get
            {
                int colorCode = _lightEngineControlModel.LED4LightColor;
                var redValue = colorCode / 65536;
                var greenValue = (colorCode - redValue * 65536) / 256;
                var blueValue = colorCode - redValue * 65536 - greenValue * 256;
                // If the power is not enabled set the background of the color opacity to be lower to 38% (100% is 255)
                _lED4LightColor = (LED4PowerState) ? new SolidColorBrush(Color.FromArgb(255, Convert.ToByte(redValue), Convert.ToByte(greenValue), Convert.ToByte(blueValue))) : new SolidColorBrush(Color.FromArgb(97, Convert.ToByte(redValue), Convert.ToByte(greenValue), Convert.ToByte(blueValue)));
                return _lED4LightColor;
            }
            set
            {
            }
        }

        public String LED4PeakToolTip
        {
            get
            {
                return "Peak Wavelength: " + this._lightEngineControlModel.LED4PeakToolTip;
            }
        }

        public Double LED4Power
        {
            get
            {
                return this._lightEngineControlModel.LED4Power;
            }
            set
            {
                if (!this._lightEngineControlModel.LED4Power.Equals(value))
                {
                    this._lightEngineControlModel.LED4Power = value;
                    this.OnPropertyChange();
                }
            }
        }

        public ICommand LED4PowerMinusCommand
        {
            get
            {
                if (this._led4MinusCommand == null)
                    this._led4MinusCommand = new RelayCommand(() => LEDMinus(4));

                return this._led4MinusCommand;
            }
        }

        public ICommand LED4PowerPlusCommand
        {
            get
            {
                if (this._led4PlusCommand == null)
                    this._led4PlusCommand = new RelayCommand(() => LEDPlus(4));

                return this._led4PlusCommand;
            }
        }

        public Boolean LED4PowerState
        {
            get
            {
                //return this._lightEngineControlModel.LED4PowerState;
                return _led4PowerState;
            }
            set
            {
                _led4PowerState = value;
                if (!this._lightEngineControlModel.LED4PowerState.Equals(_led4PowerState))
                {
                    this._lightEngineControlModel.LED4PowerState = _led4PowerState;
                }
                OnPropertyChanged("LED4PowerState");
                OnPropertyChanged("LED4LightColor");
            }
        }

        public Int32 LED4SockelID
        {
            get
            {
                return this._lightEngineControlModel.LED4SockelID;
            }
            set
            {
                if (!this._lightEngineControlModel.LED4SockelID.Equals(value))
                {
                    this._lightEngineControlModel.LED4SockelID = value;
                    this.OnPropertyChange();
                }
            }
        }

        public Double LED4Temperature
        {
            get
            {
                return this._lightEngineControlModel.LED4Temperature;
            }
            set
            {
                if (!this._lightEngineControlModel.LED4Temperature.Equals(value))
                {
                    this._lightEngineControlModel.LED4Temperature = value;
                    this.OnPropertyChange();
                }
            }
        }

        public String LED5ControlName
        {
            get
            {
                return this._lightEngineControlModel.LED5ControlName;
            }
            set
            {
                if (!this._lightEngineControlModel.LED5ControlName.Equals(value))
                {
                    this._lightEngineControlModel.LED5ControlName = value;
                    this.OnPropertyChange();
                }
            }
        }

        public SolidColorBrush LED5LightColor
        {
            get
            {
                int colorCode = _lightEngineControlModel.LED5LightColor;
                var redValue = colorCode / 65536;
                var greenValue = (colorCode - redValue * 65536) / 256;
                var blueValue = colorCode - redValue * 65536 - greenValue * 256;
                // If the power is not enabled set the background of the color opacity to be lower to 38% (100% is 255)
                _lED5LightColor = (LED5PowerState) ? new SolidColorBrush(Color.FromArgb(255, Convert.ToByte(redValue), Convert.ToByte(greenValue), Convert.ToByte(blueValue))) : new SolidColorBrush(Color.FromArgb(97, Convert.ToByte(redValue), Convert.ToByte(greenValue), Convert.ToByte(blueValue)));
                return _lED5LightColor;
            }
            set
            {
            }
        }

        public String LED5PeakToolTip
        {
            get
            {
                return "Peak Wavelength: " + this._lightEngineControlModel.LED5PeakToolTip;
            }
        }

        public Double LED5Power
        {
            get
            {
                return this._lightEngineControlModel.LED5Power;
            }
            set
            {
                if (!this._lightEngineControlModel.LED5Power.Equals(value))
                {
                    this._lightEngineControlModel.LED5Power = value;
                    this.OnPropertyChange();
                }
            }
        }

        public ICommand LED5PowerMinusCommand
        {
            get
            {
                if (this._led5MinusCommand == null)
                    this._led5MinusCommand = new RelayCommand(() => LEDMinus(5));

                return this._led5MinusCommand;
            }
        }

        public ICommand LED5PowerPlusCommand
        {
            get
            {
                if (this._led5PlusCommand == null)
                    this._led5PlusCommand = new RelayCommand(() => LEDPlus(5));

                return this._led5PlusCommand;
            }
        }

        public Boolean LED5PowerState
        {
            get
            {
                //return this._lightEngineControlModel.LED5PowerState;
                return _led5PowerState;
            }
            set
            {
                _led5PowerState = value;
                if (!this._lightEngineControlModel.LED5PowerState.Equals(_led5PowerState))
                {
                    this._lightEngineControlModel.LED5PowerState = _led5PowerState;
                }
                OnPropertyChanged("LED5PowerState");
                OnPropertyChanged("LED5LightColor");
            }
        }
        
        public Boolean ExternalMode1
        {
            get
            {
                return this._lightEngineControlModel.ExternalMode1;
            }
            set
            {
                /*_led5PowerState = value;
                if (!this._lightEngineControlModel.LED5PowerState.Equals(_led5PowerState))
                {
                    this._lightEngineControlModel.LED5PowerState = _led5PowerState;
                }*/
                this._lightEngineControlModel.ExternalMode1 = value;
                OnPropertyChanged("ExternalMode1");
            }
        }
        public Boolean ExternalMode2
        {
            get
            {
                return this._lightEngineControlModel.ExternalMode2;
            }
            set
            {
                /*_led5PowerState = value;
                if (!this._lightEngineControlModel.LED5PowerState.Equals(_led5PowerState))
                {
                    this._lightEngineControlModel.LED5PowerState = _led5PowerState;
                }*/
                this._lightEngineControlModel.ExternalMode2 = value;
                OnPropertyChanged("ExternalMode2");
            }
        }
        public Boolean ExternalMode3
        {
            get
            {
                return this._lightEngineControlModel.ExternalMode3;
            }
            set
            {
                /*_led5PowerState = value;
                if (!this._lightEngineControlModel.LED5PowerState.Equals(_led5PowerState))
                {
                    this._lightEngineControlModel.LED5PowerState = _led5PowerState;
                }*/
                this._lightEngineControlModel.ExternalMode3 = value;
                OnPropertyChanged("ExternalMode3");
            }
        }
        public Boolean ExternalMode4
        {
            get
            {
                return this._lightEngineControlModel.ExternalMode4;
            }
            set
            {
                /*_led5PowerState = value;
                if (!this._lightEngineControlModel.LED5PowerState.Equals(_led5PowerState))
                {
                    this._lightEngineControlModel.LED5PowerState = _led5PowerState;
                }*/
                this._lightEngineControlModel.ExternalMode4 = value;
                OnPropertyChanged("ExternalMode4");
            }
        }
        public Boolean ExternalMode5
        {
            get
            {
                return this._lightEngineControlModel.ExternalMode5;
            }
            set
            {
                /*_led5PowerState = value;
                if (!this._lightEngineControlModel.LED5PowerState.Equals(_led5PowerState))
                {
                    this._lightEngineControlModel.LED5PowerState = _led5PowerState;
                }*/
                this._lightEngineControlModel.ExternalMode5 = value;
                OnPropertyChanged("ExternalMode5");
            }
        }
        
        public Boolean ExternalMode6
        {
            get
            {
                return this._lightEngineControlModel.ExternalMode6;
            }
            set
            {
                /*_led5PowerState = value;
                if (!this._lightEngineControlModel.LED5PowerState.Equals(_led5PowerState))
                {
                    this._lightEngineControlModel.LED5PowerState = _led5PowerState;
                }*/
                this._lightEngineControlModel.ExternalMode6 = value;
                OnPropertyChanged("ExternalMode6");
            }
        }

        public Int32 LED5SockelID
        {
            get
            {
                return this._lightEngineControlModel.LED5SockelID;
            }
            set
            {
                if (!this._lightEngineControlModel.LED5SockelID.Equals(value))
                {
                    this._lightEngineControlModel.LED5SockelID = value;
                    this.OnPropertyChange();
                }
            }
        }

        public Double LED5Temperature
        {
            get
            {
                return this._lightEngineControlModel.LED5Temperature;
            }
            set
            {
                if (!this._lightEngineControlModel.LED5Temperature.Equals(value))
                {
                    this._lightEngineControlModel.LED5Temperature = value;
                    this.OnPropertyChange();
                }
            }
        }

        public String LED6ControlName
        {
            get
            {
                return this._lightEngineControlModel.LED6ControlName;
            }
            set
            {
                if (!this._lightEngineControlModel.LED6ControlName.Equals(value))
                {
                    this._lightEngineControlModel.LED6ControlName = value;
                    this.OnPropertyChange();
                }
            }
        }

        public SolidColorBrush LED6LightColor
        {
            get
            {
                int colorCode = _lightEngineControlModel.LED6LightColor;
                var redValue = colorCode / 65536;
                var greenValue = (colorCode - redValue * 65536) / 256;
                var blueValue = colorCode - redValue * 65536 - greenValue * 256;
                // If the power is not enabled set the background of the color opacity to be lower to 38% (100% is 255)
                _lED6LightColor = (LED6PowerState) ? new SolidColorBrush(Color.FromArgb(255, Convert.ToByte(redValue), Convert.ToByte(greenValue), Convert.ToByte(blueValue))) : new SolidColorBrush(Color.FromArgb(97, Convert.ToByte(redValue), Convert.ToByte(greenValue), Convert.ToByte(blueValue)));
                return _lED6LightColor;
            }
            set
            {
            }
        }

        public String LED6PeakToolTip
        {
            get
            {
                return "Peak Wavelength: " + this._lightEngineControlModel.LED6PeakToolTip;
            }
        }

        public Double LED6Power
        {
            get
            {
                return this._lightEngineControlModel.LED6Power;
            }
            set
            {
                if (!this._lightEngineControlModel.LED6Power.Equals(value))
                {
                    this._lightEngineControlModel.LED6Power = value;
                    this.OnPropertyChange();
                }
            }
        }

        public ICommand LED6PowerMinusCommand
        {
            get
            {
                if (this._led6MinusCommand == null)
                    this._led6MinusCommand = new RelayCommand(() => LEDMinus(6));

                return this._led6MinusCommand;
            }
        }

        public ICommand LED6PowerPlusCommand
        {
            get
            {
                if (this._led6PlusCommand == null)
                    this._led6PlusCommand = new RelayCommand(() => LEDPlus(6));

                return this._led6PlusCommand;
            }
        }

        public Boolean LED6PowerState
        {
            get
            {
                //return this._lightEngineControlModel.LED6PowerState;
                return _led6PowerState;
            }
            set
            {
                _led6PowerState = value;
                if (!this._lightEngineControlModel.LED6PowerState.Equals(_led6PowerState))
                {
                    this._lightEngineControlModel.LED6PowerState = _led6PowerState;
                }
                OnPropertyChanged("LED6PowerState");
                OnPropertyChanged("LED6LightColor");
            }
        }

        public Int32 LED6SockelID
        {
            get
            {
                return this._lightEngineControlModel.LED6SockelID;
            }
            set
            {
                if (!this._lightEngineControlModel.LED6SockelID.Equals(value))
                {
                    this._lightEngineControlModel.LED6SockelID = value;
                    this.OnPropertyChange();
                }
            }
        }

        public Double LED6Temperature
        {
            get
            {
                return this._lightEngineControlModel.LED6Temperature;
            }
            set
            {
                if (!this._lightEngineControlModel.LED6Temperature.Equals(value))
                {
                    this._lightEngineControlModel.LED6Temperature = value;
                    this.OnPropertyChange();
                }
            }
        }

        /// <summary>
        /// Selected Fast Setting
        /// </summary>
        /// <remarks>
        /// View only Property
        /// </remarks>
        public Int32 LinearModeSettingsSelectedIdx
        {
            get
            {
                return this._linearModeSettingsSelectedIdx;
            }
            set
            {
                if (!this._linearModeSettingsSelectedIdx.Equals(value))
                {
                    this._linearModeSettingsSelectedIdx = value;
                    this.OnPropertyChange();
                }
            }
        }

        public ObservableCollection<String> LinearModeSettingsSource
        {
            get
            {
                return this._linearModeSettingsSource;
            }
            set
            {
                if (!this._linearModeSettingsSource.Equals(value))
                {
                    this._linearModeSettingsSource = value;
                    this.OnPropertyChange();
                }
            }
        }

        public Double MasterBrightness
        {
            get
            {
                return this._lightEngineControlModel.MasterBrightness;
            }
            set
            {
                if (!this._lightEngineControlModel.MasterBrightness.Equals(value))
                {
                    this._lightEngineControlModel.MasterBrightness = value;
                    this.OnPropertyChange();
                }
            }
        }

        public Visibility MasterControlVis
        {
            get
            {
                return _masterControlVis;
            }
            set
            {
                _masterControlVis = value;
                OnPropertyChanged("MasterControlVis");
            }
        }

        public ICommand MasterPowerMinusCommand
        {
            get
            {
                if (this._masterPowerMinusCommand == null)
                    this._masterPowerMinusCommand = new RelayCommand(() => LEDMinus(0));

                return this._masterPowerMinusCommand;
            }
        }

        public ICommand MasterPowerPlusCommand
        {
            get
            {
                if (this._masterPowerPlusCommand == null)
                    this._masterPowerPlusCommand = new RelayCommand(() => LEDPlus(0));

                return this._masterPowerPlusCommand;
            }
        }

        public Boolean ShowTemperatures
        {
            get
            {
                return this._showTemperatures;
            }
            set
            {
                if (!this._showTemperatures.Equals(value))
                {
                    this._showTemperatures = value;
                    this.OnPropertyChange();
                }
                if (value) ExternalModeMargin = new Thickness(0, 0, 33, 0);
                else ExternalModeMargin = new Thickness(0, 0, 0, 0);
            }
        }
        
        public Thickness ExternalModeMargin
        {
            get
            {
                return this._externalModeMargin;
            }
            set
            {
                if (!this._externalModeMargin.Equals(value))
                {
                    this._externalModeMargin = value;
                    this.OnPropertyChange();
                }
            }
        }

        public String TemperatureUnit
        {
            get
            {
                return this._temperatureUnit;
            }
            set
            {
                if (!this._temperatureUnit.Equals(value))
                {
                    this._temperatureUnit = value;
                    this.OnPropertyChange();
                }
            }
        }

        public bool UpdateTemperatures
        {
            get
            {
                return _lightEngineControlModel.UpdateTemperatures;
            }
        }

        #endregion Properties

        #region Indexers

        public Object this[String propertyName, Object defaultObject = null]
        {
            get
            {
                PropertyInfo myPropInfo = this.GetPropertyInfo(propertyName);
                return myPropInfo.GetValue(this);
            }
            set
            {
                PropertyInfo myPropInfo = this.GetPropertyInfo(propertyName);
                myPropInfo.SetValue(this, value);
            }
        }

        public Object this[String propertyName, Int32 index, Object defaultObject = null]
        {
            get
            {
                PropertyInfo myPropInfo = this.GetPropertyInfo(propertyName);
                if (null != myPropInfo)
                {
                    if (typeof(CustomCollection<>) == myPropInfo.PropertyType.GetGenericTypeDefinition())
                    {
                        var collection = myPropInfo.GetValue(this, null);
                        return collection.GetType().GetProperty("Item").GetValue(collection, new Object[] { index });
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
                PropertyInfo myPropInfo = this.GetPropertyInfo(propertyName);
                if (null != myPropInfo)
                {
                    if (typeof(CustomCollection<>) == myPropInfo.PropertyType.GetGenericTypeDefinition())
                    {
                        var collection = myPropInfo.GetValue(this, null);
                        collection.GetType().GetProperty("Item").SetValue(collection, value, new Object[] { index });
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

        /// <summary>
        /// Returns the Property Info of the specified Property
        /// </summary>
        /// <param name="propertyName">Name of the Property<seealso cref="String"/></param>
        /// <returns><see cref="PropertyInfo"/></returns>
        public PropertyInfo GetPropertyInfo(String propertyName)
        {
            PropertyInfo myPropInfo = null;
            if (!this._properties.TryGetValue(propertyName, out myPropInfo))
            {
                myPropInfo = typeof(LightEngineControlViewModel).GetProperty(propertyName);
                if (null != myPropInfo)
                {
                    this._properties.Add(propertyName, myPropInfo);
                }
            }
            return myPropInfo;
        }

        public void LoadXMLSettings()
        {
            XmlDocument expDoc = MVMManager.Instance.SettingsDoc[(Int32)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS];

            XmlDocument appDoc = MVMManager.Instance.SettingsDoc[(Int32)SettingsFileType.APPLICATION_SETTINGS];

            XmlNodeList ndList = expDoc.SelectNodes("/ThorImageExperiment/LAMP");

            XmlNodeList appNdList = appDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/LightEngineView");

            //Do not enable the LEDs when the hardware configuration window is open, check the flag EnableDeviceQuery for this
            bool deviceQueryEnabled = (bool)MVMManager.Instance["CaptureSetupViewModel", "EnableDeviceQuery", (object)true];

            if (appNdList.Count > 0)
            {
                string str = string.Empty;

                if (XmlManager.GetAttribute(appNdList[0], appDoc, "MainControlVisibility", ref str))
                {
                    MasterControlVis = (str.Equals("Visible")) ? Visibility.Visible : Visibility.Collapsed;
                }
            }

            if (ndList.Count > 0)
            {
                string str = string.Empty;
                if (XmlManager.GetAttribute(ndList[0], expDoc, "mainPower", ref str))
                {
                    double tmp = 0;
                    if (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmp))
                    {
                        MasterBrightness = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], expDoc, "led1enable", ref str) && deviceQueryEnabled)
                {
                    LED1PowerState = (str == "1" || str == Boolean.TrueString);
                }
                if (XmlManager.GetAttribute(ndList[0], expDoc, "led1power", ref str))
                {
                    double tmp = 0;
                    if (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmp))
                    {
                        LED1Power = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], expDoc, "led2enable", ref str) && deviceQueryEnabled)
                {
                    LED2PowerState = (str == "1" || str == Boolean.TrueString);
                }
                if (XmlManager.GetAttribute(ndList[0], expDoc, "led2power", ref str))
                {
                    double tmp = 0;
                    if (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmp))
                    {
                        LED2Power = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], expDoc, "led3enable", ref str) && deviceQueryEnabled)
                {
                    LED3PowerState = (str == "1" || str == Boolean.TrueString);
                }
                if (XmlManager.GetAttribute(ndList[0], expDoc, "led3power", ref str))
                {
                    double tmp = 0;
                    if (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmp))
                    {
                        LED3Power = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], expDoc, "led4enable", ref str) && deviceQueryEnabled)
                {
                    LED4PowerState = (str == "1" || str == Boolean.TrueString);
                }
                if (XmlManager.GetAttribute(ndList[0], expDoc, "led4power", ref str))
                {
                    double tmp = 0;
                    if (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmp))
                    {
                        LED4Power = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], expDoc, "led5enable", ref str) && deviceQueryEnabled)
                {
                    LED5PowerState = (str == "1" || str == Boolean.TrueString);
                }
                if (XmlManager.GetAttribute(ndList[0], expDoc, "led5power", ref str))
                {
                    double tmp = 0;
                    if (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmp))
                    {
                        LED5Power = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], expDoc, "led6enable", ref str) && deviceQueryEnabled)
                {
                    LED6PowerState = (str == "1" || str == Boolean.TrueString);
                }
                if (XmlManager.GetAttribute(ndList[0], expDoc, "led6power", ref str))
                {
                    double tmp = 0;
                    if (Double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out tmp))
                    {
                        LED6Power = tmp;
                    }
                }
                if (XmlManager.GetAttribute(ndList[0], expDoc, "displayTemperatures", ref str))
                {
                    ShowTemperatures = (str == "1" || str == Boolean.TrueString);
                }
            }
            OnPropertyChanged("LED1ControlName");
            OnPropertyChanged("LED2ControlName");
            OnPropertyChanged("LED3ControlName");
            OnPropertyChanged("LED4ControlName");
            OnPropertyChanged("LED5ControlName");
            OnPropertyChanged("LED6ControlName");
        }

        /// <summary>
        /// Raises Property Changed Event
        /// </summary>
        /// <param name="propertyName">Name of the Property<seealso cref="String"/></param>
        public void OnPropertyChange([CallerMemberName]String propertyName = null)
        {
            if (null != this.GetPropertyInfo(propertyName))
            {
                this.OnPropertyChanged(propertyName);
            }
        }

        public void UpdateExpXMLSettings(ref XmlDocument experimentFile)
        {
            XmlNodeList ndList = experimentFile.SelectNodes("/ThorImageExperiment/LAMP");
            // XmlNodeList ndListHW = this.HardwareDoc.SelectNodes("/HardwareSettings/ImageDetectors/LSM");

            if (ndList.Count > 0)
            {
                XmlManager.SetAttribute(ndList[0], experimentFile, "led1enable", LED1PowerState ? "1" : "0");
                XmlManager.SetAttribute(ndList[0], experimentFile, "led1power", LED1Power.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "led2enable", LED2PowerState ? "1" : "0");
                XmlManager.SetAttribute(ndList[0], experimentFile, "led2power", LED2Power.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "led3enable", LED3PowerState ? "1" : "0");
                XmlManager.SetAttribute(ndList[0], experimentFile, "led3power", LED3Power.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "led4enable", LED4PowerState ? "1" : "0");
                XmlManager.SetAttribute(ndList[0], experimentFile, "led4power", LED4Power.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "led5enable", LED5PowerState ? "1" : "0");
                XmlManager.SetAttribute(ndList[0], experimentFile, "led5power", LED5Power.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "led6enable", LED6PowerState ? "1" : "0");
                XmlManager.SetAttribute(ndList[0], experimentFile, "led6power", LED6Power.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "displayTemperatures", ShowTemperatures.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "mainPower", MasterBrightness.ToString());
            }
            EnableDisableLEDs = false;
        }

        private void LEDMinus(int ledIndex)
        {
            switch (ledIndex)
            {
                case 0:
                    {
                        MasterBrightness -= 1;
                        OnPropertyChanged("MasterBrightness");
                    }
                    break;
                case 1:
                    {
                        LED1Power -= 1;
                        OnPropertyChanged("LED1Power");
                    }
                    break;
                case 2:
                    {
                        LED2Power -= 1;
                        OnPropertyChanged("LED2Power");
                    }
                    break;
                case 3:
                    {
                        LED3Power -= 1;
                        OnPropertyChanged("LED3Power");
                    }
                    break;
                case 4:
                    {
                        LED4Power -= 1;
                        OnPropertyChanged("LED4Power");
                    }
                    break;
                case 5:
                    {
                        LED5Power -= 1;
                        OnPropertyChanged("LED5Power");
                    }
                    break;
                case 6:
                    {
                        LED6Power -= 1;
                        OnPropertyChanged("LED6Power");
                    }
                    break;
            }
        }

        private void LEDPlus(int ledIndex)
        {
            switch (ledIndex)
            {
                case 0:
                    {
                        MasterBrightness += 1;
                        OnPropertyChanged("MasterBrightness");
                    }
                    break;
                case 1:
                    {
                        LED1Power += 1;
                        OnPropertyChanged("LED1Power");
                    }
                    break;
                case 2:
                    {
                        LED2Power += 1;
                        OnPropertyChanged("LED2Power");
                    }
                    break;
                case 3:
                    {
                        LED3Power += 1;
                        OnPropertyChanged("LED3Power");
                    }
                    break;
                case 4:
                    {
                        LED4Power += 1;
                        OnPropertyChanged("LED4Power");
                    }
                    break;
                case 5:
                    {
                        LED5Power += 1;
                        OnPropertyChanged("LED5Power");
                    }
                    break;
                case 6:
                    {
                        LED6Power += 1;
                        OnPropertyChanged("LED6Power");
                    }
                    break;
            }
        }

        #endregion Methods
    }
}