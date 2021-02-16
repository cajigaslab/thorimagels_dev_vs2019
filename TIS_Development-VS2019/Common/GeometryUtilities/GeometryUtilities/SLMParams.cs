namespace GeometryUtilities
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Media;
    using System.Windows.Shapes;
    using System.Xml;

    using HDF5CS;

    using OverlayManager;

    using ThorSharedTypes;

    public class SLMParams : INotifyPropertyChanged
    {
        #region Fields

        private int _pixelSpacing = 1;
        private double[] _slmMeasurePowerArea = { 0.0 };

        #endregion Fields

        #region Constructors

        public SLMParams()
        {
            BleachWaveParams = new BleachWaveParams();
            BleachWaveParams.PropertyChanged += BleachWaveParams_PropertyChanged;
        }

        public SLMParams(SLMParams slmParams)
        {
            this.Name = slmParams.Name;
            this.Duration = slmParams.Duration;
            this.SLMMeasurePowerArea = slmParams.SLMMeasurePowerArea;
            this.PixelSpacing = slmParams.PixelSpacing;
            this.Red = slmParams.Red;
            this.Green = slmParams.Green;
            this.Blue = slmParams.Blue;
            this.BleachWaveParams = slmParams.BleachWaveParams.MakeCopy();
            this.BleachWaveParams.PropertyChanged += BleachWaveParams_PropertyChanged;
        }

        #endregion Constructors

        #region Events

        public event Action BleachParamsChangedEvent;

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public BleachWaveParams BleachWaveParams
        {
            get;
            set;
        }

        public double Blue
        {
            get;
            set;
        }

        public double Duration
        {
            get;
            set;
        }

        public double Green
        {
            get;
            set;
        }

        public string Name
        {
            get;
            set;
        }

        public int PixelSpacing
        {
            get { return _pixelSpacing; }
            set { _pixelSpacing = value; }
        }

        /// <summary>
        /// Configurable property to enter power to calculate measured power density or vice versa.
        /// </summary>
        public bool PowerEntryPreferred
        {
            get
            {
                int iVal = ((int)ICamera.LSMType.STIMULATE_MODULATOR == ResourceManagerCS.GetBleacherType()) ? 0 : 1;
                string str = iVal.ToString();
                try
                {
                    MVMManager.Instance.ReloadSettings(SettingsFileType.APPLICATION_SETTINGS, true);
                    XmlDocument appDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
                    XmlNodeList nlA = appDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/BleachView");
                    if (0 < nlA.Count && !XmlManager.GetAttribute(nlA[0], appDoc, "PowerEntryPreferred", ref str))
                    {
                        XmlManager.SetAttribute(nlA[0], appDoc, "PowerEntryPreferred", iVal.ToString());
                        MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS, true);
                    }
                }
                catch (Exception ex)
                {
                    ThorLogging.ThorLog.Instance.TraceEvent(System.Diagnostics.TraceEventType.Verbose, 1, "Get PowerEntryPreferred: " + ex.Message);
                }
                return (Int32.TryParse(str, out iVal) && 1 == iVal);
            }
        }

        public double Red
        {
            get;
            set;
        }

        public double SLMMeasurePowerArea
        {
            get
            {
                if (1 <= _slmMeasurePowerArea.Length)
                    return _slmMeasurePowerArea[0];
                else
                    return 0.0;
            }
            set
            {
                if (0 >= _slmMeasurePowerArea.Length)
                {
                    _slmMeasurePowerArea = new double[1] { value };
                    OnPropertyChanged("SLMMeasurePowerArea");
                }
                else if (_slmMeasurePowerArea[0] != value)
                {
                    _slmMeasurePowerArea[0] = value;
                    OnPropertyChanged("SLMMeasurePowerArea");
                }
            }
        }

        public double SLMMeasurePowerArea1
        {
            get
            {
                if (2 <= _slmMeasurePowerArea.Length)
                    return _slmMeasurePowerArea[1];
                else
                    return 0.0;
            }
            set
            {
                if (1 >= _slmMeasurePowerArea.Length)
                {
                    double tmp = _slmMeasurePowerArea[0];
                    _slmMeasurePowerArea = new double[2] { tmp, value };
                    OnPropertyChanged("SLMMeasurePowerArea1");
                }
                else if (_slmMeasurePowerArea[1] != value)
                {
                    _slmMeasurePowerArea[1] = value;
                    OnPropertyChanged("SLMMeasurePowerArea1");
                }
            }
        }

        #endregion Properties

        #region Methods

        public bool CompareTo(SLMParams slmParams)
        {
            if (this.PixelSpacing != slmParams.PixelSpacing)
                return false;
            if (this.Duration != slmParams.Duration)
                return false;

            return (this.BleachWaveParams.CompareTo(slmParams.BleachWaveParams));
        }

        protected virtual void OnPropertyChanged(string propertyName)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null)
            {
                handler(this, new PropertyChangedEventArgs(propertyName));
                if (null != BleachParamsChangedEvent &&
                    (0 == propertyName.CompareTo("SLMMeasurePowerArea") || 0 == propertyName.CompareTo("SLMMeasurePowerArea1")))
                    BleachParamsChangedEvent();
            }
        }

        void BleachWaveParams_PropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            if ((null != BleachParamsChangedEvent) &&
                ((0 == e.PropertyName.CompareTo("ROIWidthUM")) || (0 == e.PropertyName.CompareTo("ROIHeightUM")) ||
                (0 == e.PropertyName.CompareTo("MeasurePower")) || (0 == e.PropertyName.CompareTo("MeasurePower1"))))
            {
                BleachParamsChangedEvent();
            }
        }

        #endregion Methods
    }
}