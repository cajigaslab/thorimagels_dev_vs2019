namespace ImageViewMVM.Models
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;

    using ThorSharedTypes;

    public class PixelDataHistogramInfo
    {
        #region Fields

        private double _blackpoint;
        private double _gamma;
        private double _whitepoint;

        #endregion Fields

        #region Constructors

        public PixelDataHistogramInfo(ImageIdentifier imageIdentifier)
        {
            DataImageIdentifier = imageIdentifier;
            Min = ushort.MaxValue;
            Max = ushort.MinValue;
            HistogramData = null;
            _whitepoint = 255;
            _blackpoint = 0;
            Palette = new byte[ushort.MaxValue + 1];
            IsWhiteBlackPointChanged = true;
            IsContinuousAutoChecked = false;
            IsAutoPressed = false;
            _gamma = 1.0;
        }

        #endregion Constructors

        #region Properties

        public double BlackPoint
        {
            get
            {
                return _blackpoint;
            }

            set
            {
                IsWhiteBlackPointChanged = true;
                _blackpoint = value;
            }
        }

        public ImageIdentifier DataImageIdentifier
        {
            get; set;
        }

        public double Gamma
        {
            get
            {
                return _gamma;
            }

            set
            {
                IsWhiteBlackPointChanged = true;
                _gamma = value;
            }
        }

        public int[] HistogramData
        {
            get; set;
        }

        public bool IsAutoPressed
        {
            get; set;
        }

        public bool IsContinuousAutoChecked
        {
            get; set;
        }

        public bool IsWhiteBlackPointChanged
        {
            get; set;
        }

        public int Max
        {
            get; set;
        }

        public int Min
        {
            get; set;
        }

        public byte[] Palette
        {
            get; set;
        }

        public double WhitePoint
        {
            get
            {
                return _whitepoint;
            }

            set
            {
                IsWhiteBlackPointChanged = true;
                _whitepoint = value;
            }
        }

        #endregion Properties
    }
}