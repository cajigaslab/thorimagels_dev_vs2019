namespace FLIMFitting.Model
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows.Media;

    using FLIMFitting;

    public class FLIMData : BindableBase
    {
        #region Fields

        public static int DATA_SIZE = 241;
        public static int PARAM_SIZE = 7;

        public uint[] DataArray = new uint[DATA_SIZE];
        public double[] FitResultArray = new double[DATA_SIZE];
        public double _amp2;
        public int _channel;
        public double _chi2;
        public double _gaussW;
        public double _nsPerPoint;
        public double _scattr;
        public double _tau1;
        public double _tau2;
        public double _tau8;
        public double _tZero;

        private double _amp1;
        private string _id;
        private bool _isSelected;

        #endregion Fields

        #region Properties

        public double Amp1
        {
            get { return _amp1; }
            set
            {
                if (IsNotEqual(_amp1, value))
                    SetProperty(ref _amp1, value);
            }
        }

        public double Amp2
        {
            get { return _amp2; }
            set
            {
                if (IsNotEqual(_amp2, value))
                    SetProperty(ref _amp2, value);
            }
        }

        public int Channel
        {
            get
            {
                return _channel;
            }
            set
            {
                SetProperty(ref _channel, value);
            }
        }

        public double Chi2
        {
            get { return _chi2; }
            set
            {
                if (IsNotEqual(_chi2, value))
                    SetProperty(ref _chi2, value);
            }
        }

        public double GaussW
        {
            get { return _gaussW; }
            set
            {
                if (IsNotEqual(_gaussW, value))
                    SetProperty(ref _gaussW, value);
            }
        }

        public string ID
        {
            get { return _id; }
            set { SetProperty(ref _id, value); }
        }

        public bool IsSelected
        {
            get { return _isSelected; }
            set { SetProperty(ref _isSelected,value); }
        }

        public bool IsWholeChannelHistogramData
        {
            get;
            set;
        }

        public double NsPerPoint
        {
            get { return _nsPerPoint; }
            set
            {
                if (IsNotEqual(_nsPerPoint, value))
                    SetProperty(ref _nsPerPoint, value);
            }
        }

        public Color RenderColor
        {
            set; get;
        }

        public double Scattr
        {
            get { return _scattr; }
            set
            {
                if (IsNotEqual(_scattr, value))
                    SetProperty(ref _scattr, value);
            }
        }

        public double Tau1
        {
            get { return _tau1; }
            set
            {
                if (IsNotEqual(_tau1, value))
                    SetProperty(ref _tau1, value);
            }
        }

        public double Tau2
        {
            get { return _tau2; }
            set
            {
                if (IsNotEqual(_tau2, value))
                    SetProperty(ref _tau2, value);
            }
        }

        public double Tau8
        {
            get { return _tau8; }
            set
            {
                if (IsNotEqual(_tau8, value))
                    SetProperty(ref _tau8, value);
            }
        }

        public double TZero
        {
            get { return _tZero; }
            set
            {
                if (IsNotEqual(_tZero, value))
                    SetProperty(ref _tZero, value);
            }
        }

        #endregion Properties

        #region Methods

        public FLIMData ShallowCopy()
        {
            // the DataArray will use the same;
            var data = (FLIMData)this.MemberwiseClone();
            data.FitResultArray = new double[DATA_SIZE];
            return data;
        }

        private bool IsNotEqual(double v1, double v2)
        {
            return Math.Abs((v1 - v2)) > 0.000001;
        }

        #endregion Methods
    }
}