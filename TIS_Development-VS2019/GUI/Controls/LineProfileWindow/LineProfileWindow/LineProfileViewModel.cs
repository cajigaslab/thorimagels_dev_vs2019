namespace LineProfileWindow.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows.Media;

    using Microsoft.Research.DynamicDataDisplay;
    using Microsoft.Research.DynamicDataDisplay.Charts;
    using Microsoft.Research.DynamicDataDisplay.DataSources;
    using Microsoft.Research.DynamicDataDisplay.PointMarkers;

    using ThorSharedTypes;

    class LineProfileViewModel : ThorSharedTypes.VMBase
    {
        #region Fields

        private bool[] _channelEnable = null;
        private Color[] _colorAssigment = null;
        private int _displayChannelIndex = 0;
        private string _horizontalAxisTitle = string.Empty;
        private int _initialChildrenCount = 0;
        private bool _isAutoScaleActive = true;
        private bool _isConversionActive = false;
        private bool _isDisplaying = true;
        LineProfileData _lineProfileData;
        private int _lineWidth = 1;
        private int _lineWidthMax = int.MaxValue;
        private int _maxChannels = 4;
        private int _numChannel = 1;
        private string _title;
        private string _verticalAxisTitle;
        //:TODO: Make Ymax dynamically read from the camera the current bitdepth
        private double _ymax = 17000; //Max pixel depth is 14bit
        private double _ymin = 0;

        #endregion Fields

        #region Constructors

        public LineProfileViewModel()
        {
        }

        #endregion Constructors

        #region Events

        public event Action<int> LineWidthChange;

        #endregion Events

        #region Properties

        public bool[] ChannelEnable
        {
            get { return _channelEnable; }
            set { _channelEnable = value; }
        }

        public Color[] ColorAssigment
        {
            get
            {
                return _colorAssigment;
            }
            set
            {
                _colorAssigment = value;
            }
        }

        public int DisplayChannelIndex
        {
            get { return _displayChannelIndex; }
            set { _displayChannelIndex = value; }
        }

        public string HorizontalAxisTitle
        {
            get
            { return _horizontalAxisTitle; }
            set
            { _horizontalAxisTitle = value; }
        }

        public int InitialChildrenCount
        {
            get { return _initialChildrenCount; }
            set { _initialChildrenCount = value; }
        }

        public bool IsAutoScaleActive
        {
            get { return _isAutoScaleActive; }
            set { _isAutoScaleActive = value; }
        }

        public bool IsConversionActive
        {
            get { return _isConversionActive; }
            set { _isConversionActive = value; }
        }

        public bool IsDisplaying
        {
            get { return _isDisplaying; }
            set { _isDisplaying = value; }
        }

        public LineProfileData LineProfileData
        {
            get { return _lineProfileData; }
            set { _lineProfileData = value; }
        }

        public int LineWidth
        {
            get
            {
                return _lineWidth;
            }
            set
            {
                _lineWidth = Math.Max(1, value);
                _lineWidth = Math.Min(_lineWidth, _lineWidthMax);
                if (_lineWidth % 2 == 0) _lineWidth--;
                LineWidthChange(_lineWidth);
                OnPropertyChanged("LineWidth");
            }
        }

        public int LineWidthMax
        {
            get { return _lineWidthMax; }
            set
            {
                _lineWidthMax = value;
                _lineWidth = (_lineWidth > _lineWidthMax) ? _lineWidthMax : _lineWidth;
            }
        }

        public string LTitle
        {
            get
            { return _title; }
            set
            { _title = value; }
        }

        public int MaxChannels
        {
            get { return _maxChannels; }
            set { _maxChannels = value; }
        }

        public int NumChannel
        {
            get { return _numChannel; }
            set { _numChannel = value; }
        }

        public string VerticalAxisTitle
        {
            get
            { return _verticalAxisTitle; }
            set
            { _verticalAxisTitle = value; }
        }

        public double YmaxValue
        {
            get { return _ymax; }
            set
            {
                if (value > _ymin)
                {
                    _ymax = value;
                }
            }
        }

        public double YminValue
        {
            get { return _ymin; }
            set
            {
                if (value < _ymax)
                {
                    _ymin = value;
                }
            }
        }

        #endregion Properties

        #region Methods

        public void SaveAs()
        {
            Microsoft.Win32.SaveFileDialog dlg = new Microsoft.Win32.SaveFileDialog();
            dlg.FileName = "LineProfileData";
            dlg.DefaultExt = ".csv";
            dlg.Filter = "CSV files (.csv)|*.csv";

            if (true == dlg.ShowDialog())
            {
                using (var fileStream = new FileStream(dlg.FileName, FileMode.Create))
                {
                    // write to just created file
                    string head = "";
                    long chEnable = LineProfileData.channelEnable;
                    if ((chEnable & 0x01) != 0) head += "Channel A,";
                    if ((chEnable & 0x02) != 0) head += "Channel B,";
                    if ((chEnable & 0x04) != 0) head += "Channel C,";
                    if ((chEnable & 0x08) != 0) head += "Channel D,";
                    byte[] h = new UTF8Encoding(true).GetBytes(head + '\n');
                    fileStream.Write(h, 0, h.Length);
                    for (int i = 0; i < LineProfileData.profileDataX.Length; i++)
                    {
                        string dataLine = "";
                        for (int j = 0; j < LineProfileData.profileDataY.Length; j++)
                        {
                            dataLine += (LineProfileData.profileDataY[j][i]).ToString() + ',';
                        }
                        byte[] d = new UTF8Encoding(true).GetBytes(dataLine + '\n');
                        fileStream.Write(d, 0, d.Length);
                    }
                }
            }
        }

        public void SetData(LineProfileData lineprofileData)
        {
            _lineProfileData = lineprofileData;
            _numChannel = 0;
            for (int i = 0; i < _channelEnable.Length; i++)
            {
                _channelEnable[i] = Convert.ToBoolean(_lineProfileData.channelEnable & (int)Math.Pow(2, i));
                _numChannel += (true == _channelEnable[i]) ? 1 : 0;
            }
        }

        #endregion Methods
    }
}