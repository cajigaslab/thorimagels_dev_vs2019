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
        private double _slmMeasurePowerArea = 0.0;

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

        public double Red
        {
            get;
            set;
        }

        public double SLMMeasurePowerArea
        {
            get { return _slmMeasurePowerArea; }
            set
            {
                _slmMeasurePowerArea = value;
                OnPropertyChanged("SLMMeasurePowerArea");
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
            if (handler != null) handler(this, new PropertyChangedEventArgs(propertyName));
        }

        void BleachWaveParams_PropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            if ((null != BleachParamsChangedEvent) &&
                ((0 == e.PropertyName.CompareTo("ROIWidthUM")) || (0 == e.PropertyName.CompareTo("ROIHeightUM")) || (0 == e.PropertyName.CompareTo("MeasurePower"))))
            {
                BleachParamsChangedEvent();
            }
        }

        #endregion Methods
    }
}