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

    /// <summary>
    /// Bleach parameters for table display and building waveforms.
    /// </summary>
    public class BleachWaveParams : INotifyPropertyChanged
    {
        #region Fields

        Point _center = new Point();
        Point _centerUM = new Point();
        private int _clockRate = 0;
        private double _deltaX_Px = 0;
        private double _dwellTime = 0;
        private int _epochCount = 1;
        private int _fill = (int)FillChoice.Tornado;
        private int _iterations = 1;
        private double[] _measurePower = { 0.0 };
        private bool _pixelMode = false;
        private double _postCycleIdleMS = 0;
        private double _postEpochIdleMS = 0;
        private double _postIdleTime = 0;
        private double _postPatIdleTime = 0;
        private double _preCycleIdleMS = 0;
        private double _preEpochIdleMS = 0;
        private double _preIdleTime = 0;
        private double _prePatIdleTime = 0;
        private double _roiBottom = 0;
        private double _roiBottomUM = 0;
        private double _roiHeight = 0;
        private double _roiHeightUM = 0;
        private double _roiLeft = 0;
        private double _roiLeftUM = 0;
        private double _roiRight = 0;
        private double _roiRightUM = 0;
        private double _roiTop = 0;
        private double _roiTopUM = 0;
        private double _roiWidth = 0;
        private double _roiWidthUM = 0;
        private double _umPerPixel = 1.0;
        private double _umPerPixelRatio = 1.0;
        List<Point> _vertices = new List<Point>();
        private double _zValue = 0;

        #endregion Fields

        #region Enumerations

        public enum FillChoice
        {
            Boundary,
            Tornado,
            Raster
        }

        #endregion Enumerations

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        //Ellipse:
        public Point Center
        {
            get
            { return _center; }
            set
            {
                _center = value;
                if (0 < _umPerPixel)
                {
                    _centerUM = new Point(Math.Round(value.X * _umPerPixel / _umPerPixelRatio, 2), Math.Round(value.Y * _umPerPixel / _umPerPixelRatio, 2));
                    OnPropertyChanged("CenterUM");
                }
                OnPropertyChanged("Center");
            }
        }

        public Point CenterUM
        {
            get
            { return _centerUM; }
            set
            {
                if (0 < _umPerPixel)
                {
                    _center = new Point(value.X * _umPerPixelRatio / _umPerPixel, value.Y * _umPerPixelRatio / _umPerPixel);
                    OnPropertyChanged("Center");
                }
                _centerUM = new Point(Math.Round(value.X, 2), Math.Round(value.Y, 2));
                OnPropertyChanged("CenterUM");
            }
        }

        public int ClockRate
        {
            get { return _clockRate; }
            set
            {
                _clockRate = value;
                OnPropertyChanged("ClockRate");
            }
        }

        public double DeltaX_Px
        {
            get
            {
                return _deltaX_Px;
            }
            set
            {
                _deltaX_Px = value;
                OnPropertyChanged("DeltaX_Px");
                OnPropertyChanged("EstDuration");
            }
        }

        //[ms]
        public double DwellTime
        {
            get { return _dwellTime; }
            set
            {
                _dwellTime = value;
                OnPropertyChanged("DwellTime");
                OnPropertyChanged("EstDuration");
            }
        }

        public int EpochCount
        {
            get { return _epochCount; }
            set
            {
                _epochCount = value;
                OnPropertyChanged("EpochCount");
            }
        }

        /// <summary>
        /// Estimated duration time [ms] based on side trace fill of area
        /// </summary>
        public double EstDuration
        {
            get
            {
                if (0 >= DeltaX_Px)
                    return 0;

                //[ms], DwellTime in [us]
                const double FILL_FACTOR = 0.25;            //estimated, polygon may adjust based on DeltaX_Px
                const int SQUARE_SIDE_COUNT = 4;

                double unitArea = Math.Pow(DeltaX_Px, 2);
                double clockTime = (0.0 < ClockRate) ? WaveformBuilder.MS_TO_S / ClockRate : 0.0;
                int stepCount = 0;
                double pLength = 0, pArea = 0;

                switch (shapeType)
                {
                    case "Rectangle":
                        switch (Fill)
                        {
                            case 0: //Boundary Trace
                                stepCount = Math.Max(1, (int)((ROIWidth + ROIHeight) * 2 / DeltaX_Px));
                                break;
                            case 1: //Tornado Fill
                                stepCount = Math.Max(1, (int)(ROIWidth * ROIHeight / unitArea * SQUARE_SIDE_COUNT * FILL_FACTOR));
                                break;
                            case 2: //Raster Fill, = (Nx + 1) * Ny
                                stepCount = Math.Max(1, ((int)(ROIWidth / DeltaX_Px) + 1) * (int)(ROIHeight / DeltaX_Px));
                                break;
                            default:
                                return 0;
                        }
                        break;
                    case "Polygon":
                        if (2 > VerticeCount)
                            return 0;

                        if (1 == Fill)
                        {
                            // ***    Get area based on pixel count from binary image    *** //
                            Point rootTranslate = new Point();
                            System.Drawing.Bitmap aMap = ProcessBitmap.CreateBitmap(Vertices, true, true, 0, ref rootTranslate);
                            //aMap.SaveMap("C:/Temp/SourcePoly");
                            byte[] buffer = ProcessBitmap.LoadBinaryBitmap(aMap);

                            int imageIdxOffset = 0;
                            byte white = 0xFF;               //Color order: BGR instead of RGB
                            int stride = aMap.Width * 4;

                            for (int j = 0; j < aMap.Height; j++)
                            {
                                for (int i = 0; i < aMap.Width; i++)
                                {
                                    imageIdxOffset = j * stride + i * 4;
                                    if ((white == buffer[imageIdxOffset]) && (white == buffer[imageIdxOffset + 1]) && (white == buffer[imageIdxOffset + 2]))
                                    {
                                        pArea++;
                                    }
                                }
                            }

                            stepCount = Math.Max(1, (int)(pArea / unitArea * SQUARE_SIDE_COUNT * FILL_FACTOR));
                        }
                        else
                        {
                            for (int i = 0; i < VerticeCount; i++)
                            {
                                pLength += ((VerticeCount - 1) == i) ?
                                    (Math.Sqrt(Math.Pow((Vertices[0].X - Vertices[i].X), 2) + Math.Pow((Vertices[0].Y - Vertices[i].Y), 2))) :
                                    (Math.Sqrt(Math.Pow((Vertices[i + 1].X - Vertices[i].X), 2) + Math.Pow((Vertices[i + 1].Y - Vertices[i].Y), 2)));
                            }
                            stepCount = Math.Max(1, (int)(pLength / DeltaX_Px));
                        }
                        break;
                    case "Crosshair":
                        stepCount = 1;
                        return (double)Decimal.Round((Decimal)(stepCount * (DwellTime / WaveformBuilder.MS_TO_S)), 3);
                    case "Line":
                        stepCount = Math.Max(1, (int)(Math.Sqrt(Math.Pow(ROITop - ROIBottom, 2) + Math.Pow(ROILeft - ROIRight, 2)) / DeltaX_Px));
                        break;
                    case "Polyline":
                        for (int i = 0; i < (VerticeCount - 1); i++)
                        {
                            pLength += Math.Sqrt(Math.Pow(Vertices[i].X - Vertices[i + 1].X, 2) + Math.Pow(Vertices[i].Y - Vertices[i + 1].Y, 2));
                        }
                        stepCount = Math.Max(1, (int)(pLength / DeltaX_Px));
                        break;
                    case "Ellipse":
                        stepCount = (1 == Fill) ?
                            Math.Max(1, (int)(ROIWidth / 2 * ROIHeight / 2 * Math.PI / unitArea * SQUARE_SIDE_COUNT * FILL_FACTOR)) :
                            Math.Max(1, (int)(Math.PI / 2 * ((ROIWidth + ROIHeight) - Math.Sqrt((3 * ROIWidth + ROIHeight) * (ROIWidth + 3 * ROIHeight)))));
                        break;
                    default:
                        return 0;
                }
                return (PixelMode) ?
                    (double)Decimal.Round((Decimal)(stepCount * (PreIdleTime + DwellTime / WaveformBuilder.MS_TO_S + PostIdleTime + clockTime)), 3) :
                    (double)Decimal.Round((Decimal)(stepCount * (DwellTime / WaveformBuilder.MS_TO_S + clockTime)), 3);
            }
        }

        public int Fill
        {
            get { return _fill; }
            set
            {
                _fill = value;
                OnPropertyChanged("Fill");
                OnPropertyChanged("EstDuration");
            }
        }

        public Dictionary<int, string> FillModes
        {
            get
            {
                Dictionary<int, string> modes = new Dictionary<int, string>();
                modes.Add((int)FillChoice.Boundary, "Boundary Trace");
                switch (shapeType)
                {
                    case "Rectangle":
                        modes.Add((int)FillChoice.Tornado, "Tornado Fill");
                        modes.Add((int)FillChoice.Raster, "Raster Fill");
                        break;
                    case "Polygon":
                    case "Ellipse":
                        modes.Add((int)FillChoice.Tornado, "Tornado Fill");
                        break;
                    default:
                        _fill = (int)FillChoice.Boundary;
                        break;
                }
                return modes;
            }
        }

        public uint ID
        {
            get;
            set;
        }

        public int Iterations
        {
            get { return _iterations; }
            set { _iterations = value; }
        }

        //[sec]
        public double LongIdleTime
        {
            get;
            set;
        }

        public double[] MeasurePower
        {
            get { return _measurePower; }
            set
            {
                _measurePower = value;
                OnPropertyChanged("MeasurePower");
            }
        }

        public bool PixelMode
        {
            get { return _pixelMode; }
            set
            {
                _pixelMode = value;
                OnPropertyChanged("PixelMode");
                OnPropertyChanged("EstDuration");
            }
        }

        //[ms]
        public double PostCycleIdleMS
        {
            get { return _postCycleIdleMS; }
            set
            {
                _postCycleIdleMS = value;
                OnPropertyChanged("PostCycleIdleMS");
            }
        }

        //[ms]
        public double PostEpochIdleMS
        {
            get { return _postEpochIdleMS; }
            set
            {
                _postEpochIdleMS = value;
                OnPropertyChanged("PostEpochIdleMS");
            }
        }

        //[ms]
        public double PostIdleTime
        {
            get { return _postIdleTime; }
            set
            {
                _postIdleTime = value;
                OnPropertyChanged("PostIdleTime");
                OnPropertyChanged("EstDuration");
            }
        }

        //[ms]
        public double PostPatIdleTime
        {
            get { return _postPatIdleTime; }
            set
            {
                _postPatIdleTime = value;
                OnPropertyChanged("PostPatIdleTime");
            }
        }

        //[percent]
        public double[] Power
        {
            get;
            set;
        }

        //[ms]
        public double PreCycleIdleMS
        {
            get { return _preCycleIdleMS; }
            set
            {
                _preCycleIdleMS = value;
                OnPropertyChanged("PreCycleIdleMS");
            }
        }

        //[ms]
        public double PreEpochIdleMS
        {
            get { return _preEpochIdleMS; }
            set
            {
                _preEpochIdleMS = value;
                OnPropertyChanged("PreEpochIdleMS");
            }
        }

        //[ms]
        public double PreIdleTime
        {
            get { return _preIdleTime; }
            set
            {
                _preIdleTime = value;
                OnPropertyChanged("PreIdleTime");
                OnPropertyChanged("EstDuration");
            }
        }

        //[ms]
        public double PrePatIdleTime
        {
            get { return _prePatIdleTime; }
            set
            {
                _prePatIdleTime = value;
                OnPropertyChanged("PrePatIdleTime");
            }
        }

        //[Pixel]
        public double ROIBottom
        {
            get
            { return _roiBottom; }
            set
            {
                _roiBottom = value;
                if (0 < _umPerPixel)
                {
                    _roiBottomUM = Math.Round(value * _umPerPixel / _umPerPixelRatio, 2);
                    OnPropertyChanged("ROIBottomUM");
                }
                OnPropertyChanged("ROIBottom");
            }
        }

        //[um]
        public double ROIBottomUM
        {
            get
            { return _roiBottomUM; }
            set
            {
                if (0 < _umPerPixel)
                {
                    _roiBottom = value * _umPerPixelRatio / _umPerPixel;
                    OnPropertyChanged("ROIBottom");
                }
                _roiBottomUM = Math.Round(value, 2);
                OnPropertyChanged("ROIBottomUM");
            }
        }

        //[Pixel]
        public double ROIHeight
        {
            get
            { return _roiHeight; }
            set
            {
                _roiHeight = value;
                if (0 < _umPerPixel)
                {
                    _roiHeightUM = Math.Round(value * _umPerPixel / _umPerPixelRatio, 2);
                    OnPropertyChanged("ROIHeightUM");
                }
                OnPropertyChanged("ROIHeight");
            }
        }

        //[um]
        public double ROIHeightUM
        {
            get
            { return _roiHeightUM; }
            set
            {
                if (0 < _umPerPixel)
                {
                    _roiHeight = value * _umPerPixelRatio / _umPerPixel;
                    OnPropertyChanged("ROIHeight");
                }
                _roiHeightUM = Math.Round(value, 2);
                OnPropertyChanged("ROIHeightUM");
            }
        }

        //[Pixel]
        public double ROILeft
        {
            get
            { return _roiLeft; }
            set
            {
                _roiLeft = value;
                if (0 < _umPerPixel)
                {
                    _roiLeftUM = Math.Round(value * _umPerPixel / _umPerPixelRatio, 2);
                    OnPropertyChanged("ROILeftUM");
                }
                OnPropertyChanged("ROILeft");
            }
        }

        //[um]
        public double ROILeftUM
        {
            get
            { return _roiLeftUM; }
            set
            {
                if (0 < _umPerPixel)
                {
                    _roiLeft = value * _umPerPixelRatio / _umPerPixel;
                    OnPropertyChanged("ROILeft");
                }
                _roiLeftUM = Math.Round(value, 2);
                OnPropertyChanged("ROILeftUM");
            }
        }

        //[Pixel]
        public double ROIRight
        {
            get
            { return _roiRight; }
            set
            {
                _roiRight = value;
                if (0 < _umPerPixel)
                {
                    _roiRightUM = Math.Round(value * _umPerPixel / _umPerPixelRatio, 2);
                    OnPropertyChanged("ROIRightUM");
                }
                OnPropertyChanged("ROIRight");
            }
        }

        //[um]
        public double ROIRightUM
        {
            get
            { return _roiRightUM; }
            set
            {
                if (0 < _umPerPixel)
                {
                    _roiRight = value * _umPerPixelRatio / _umPerPixel;
                    OnPropertyChanged("ROIRight");
                }
                _roiRightUM = Math.Round(value, 2);
                OnPropertyChanged("ROIRightUM");
            }
        }

        //[Pixel]
        public double ROITop
        {
            get
            { return _roiTop; }
            set
            {
                _roiTop = value;
                if (0 < _umPerPixel)
                {
                    _roiTopUM = Math.Round(value * _umPerPixel / _umPerPixelRatio, 2);
                    OnPropertyChanged("ROITopUM");
                }
                OnPropertyChanged("ROITop");
            }
        }

        //[um]
        public double ROITopUM
        {
            get
            { return _roiTopUM; }
            set
            {
                if (0 < _umPerPixel)
                {
                    _roiTop = value * _umPerPixelRatio / _umPerPixel;
                    OnPropertyChanged("ROITop");
                }
                _roiTopUM = Math.Round(value, 2);
                OnPropertyChanged("ROITopUM");
            }
        }

        //[Pixel]
        public double ROIWidth
        {
            get
            { return _roiWidth; }
            set
            {
                _roiWidth = value;
                if (0 < _umPerPixel)
                {
                    _roiWidthUM = Math.Round(value * _umPerPixel / _umPerPixelRatio, 2);
                    OnPropertyChanged("ROIWidthUM");
                }
                OnPropertyChanged("ROIWidth");
            }
        }

        //[um]
        public double ROIWidthUM
        {
            get
            { return _roiWidthUM; }
            set
            {
                if (0 < _umPerPixel)
                {
                    _roiWidth = value * _umPerPixelRatio / _umPerPixel;
                    OnPropertyChanged("ROIWidth");
                }
                _roiWidthUM = Math.Round(value, 2);
                OnPropertyChanged("ROIWidthUM");
            }
        }

        public string shapeType
        {
            get;
            set;
        }

        public double UMPerPixel
        {
            get
            { return _umPerPixel; }
            set
            {
                _umPerPixel = value;
                OnPropertyChanged("UMPerPixel");
            }
        }

        public double UMPerPixelRatio
        {
            get
            { return _umPerPixelRatio; }
            set
            {
                _umPerPixelRatio = value;
                OnPropertyChanged("UMPerPixelRatio");
            }
        }

        public int VerticeCount
        {
            get;
            set;
        }

        //Polygon:
        public List<Point> Vertices
        {
            get
            {
                return _vertices;
            }
            set
            {
                _vertices = value;
            }
        }

        public double ZValue
        {
            get { return _zValue; }
            set
            {
                _zValue = value;
                OnPropertyChanged("ZValue");
            }
        }

        #endregion Properties

        #region Methods

        public bool CompareTo(BleachWaveParams bParamsIn)
        {
            if (this._center.X != bParamsIn._center.X)
                return false;
            if (this._center.Y != bParamsIn._center.Y)
                return false;
            if (this._iterations != bParamsIn._iterations)
                return false;
            if (this._roiHeightUM != bParamsIn._roiHeightUM)
                return false;
            if (this._roiWidthUM != bParamsIn._roiWidthUM)
                return false;
            if (this.PreIdleTime != bParamsIn.PreIdleTime)
                return false;
            if (this.DwellTime != bParamsIn.DwellTime)
                return false;
            if (this.PostIdleTime != bParamsIn.PostIdleTime)
                return false;
            if (this.Power != bParamsIn.Power)
                return false;
            if (this.PrePatIdleTime != bParamsIn.PrePatIdleTime)
                return false;
            if (this.PostPatIdleTime != bParamsIn.PostPatIdleTime)
                return false;

            return true;
        }

        public double GetPolyBottom()
        {
            if (Vertices.Count > 0)
            {
                double val = 0;
                for (int i = 0; i < Vertices.Count; i++)
                {
                    if (i == 0) { val = Vertices[0].Y; }
                    else if (Vertices[i].Y > val) { val = Vertices[i].Y; }
                }
                return val;
            }
            else
            {
                return 0;
            }
        }

        public double GetPolyLeft()
        {
            if (Vertices.Count > 0)
            {
                double val = 0;
                for (int i = 0; i < Vertices.Count; i++)
                {
                    if (i == 0) { val = Vertices[0].X; }
                    else if (Vertices[i].X < val) { val = Vertices[i].X; }
                }
                return val;
            }
            else
            {
                return 0;
            }
        }

        public double GetPolyRight()
        {
            if (Vertices.Count > 0)
            {
                double val = 0;
                for (int i = 0; i < Vertices.Count; i++)
                {
                    if (i == 0) { val = Vertices[0].X; }
                    else if (Vertices[i].X > val) { val = Vertices[i].X; }
                }
                return val;
            }
            else
            {
                return 0;
            }
        }

        public double GetPolyTop()
        {
            if (Vertices.Count > 0)
            {
                double val = 0;
                for (int i = 0; i < Vertices.Count; i++)
                {
                    if (i == 0) { val = Vertices[0].Y; }
                    else if (Vertices[i].Y < val) { val = Vertices[i].Y; }
                }
                return val;
            }
            else
            {
                return 0;
            }
        }

        public BleachWaveParams MakeCopy()
        {
            BleachWaveParams outParams = new BleachWaveParams();
            outParams._umPerPixel = this.UMPerPixel;
            outParams._umPerPixelRatio = this.UMPerPixelRatio;
            outParams._center.X = this.Center.X;
            outParams._center.Y = this.Center.Y;
            outParams._centerUM.X = this.CenterUM.X;
            outParams._centerUM.Y = this.CenterUM.Y;
            outParams.ClockRate = this.ClockRate;
            outParams.DwellTime = this.DwellTime;
            outParams.Fill = this.Fill;
            outParams.ID = this.ID;
            outParams.Iterations = this.Iterations;
            outParams.PixelMode = this.PixelMode;
            outParams.PostIdleTime = this.PostIdleTime;
            outParams.PostPatIdleTime = this.PostPatIdleTime;
            outParams.PreCycleIdleMS = this.PreCycleIdleMS;
            outParams.PostCycleIdleMS = this.PostCycleIdleMS;
            outParams.PreEpochIdleMS = this.PreEpochIdleMS;
            outParams.PostEpochIdleMS = this.PostEpochIdleMS;
            outParams.Power = this.Power;
            outParams.PreIdleTime = this.PreIdleTime;
            outParams.PrePatIdleTime = this.PrePatIdleTime;
            outParams.ROIBottom = this.ROIBottom;
            outParams.ROIBottomUM = this.ROIBottomUM;
            outParams.ROIHeight = this.ROIHeight;
            outParams.ROIHeightUM = this.ROIHeightUM;
            outParams.ROILeft = this.ROILeft;
            outParams.ROILeftUM = this.ROILeftUM;
            outParams.ROIRight = this.ROIRight;
            outParams.ROIRightUM = this.ROIRightUM;
            outParams.ROITop = this.ROITop;
            outParams.ROITopUM = this.ROITopUM;
            outParams.ROIWidth = this.ROIWidth;
            outParams.ROIWidthUM = this.ROIWidthUM;
            outParams.shapeType = this.shapeType;
            outParams.VerticeCount = this.VerticeCount;
            outParams.LongIdleTime = this.LongIdleTime;
            outParams.MeasurePower = this.MeasurePower;
            outParams.EpochCount = this.EpochCount;
            for (int i = 0; i < outParams.VerticeCount; i++)
            {
                outParams.Vertices.Add(new Point(this.Vertices[i].X, this.Vertices[i].Y));
            }
            return outParams;
        }

        protected virtual void OnPropertyChanged(string propertyName)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null) handler(this, new PropertyChangedEventArgs(propertyName));
        }

        #endregion Methods
    }
}