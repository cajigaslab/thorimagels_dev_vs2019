namespace OverlayManager
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Collections.Specialized;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Runtime.Serialization.Formatters.Binary;
    using System.Security.Cryptography;
    using System.Text;
    using System.Text.RegularExpressions;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Markup;
    using System.Windows.Media;
    using System.Windows.Shapes;
    using System.Windows.Threading;
    using System.Xml;

    using MesoScan.Params;

    using ThorLogging;

    using ThorSharedTypes;

    using static System.Windows.Forms.VisualStyles.VisualStyleElement;
    using static System.Windows.Forms.VisualStyles.VisualStyleElement.StartPanel;

    public class OverlayManagerClass
    {
        #region Fields

        public static Byte ATTENUATE_VALUE = (Byte)(Byte.MaxValue / 2);
        public static Dictionary<string, Type> ROITypeDictionary = new Dictionary<string, Type>()
        {
        {"RECTANGLE", typeof(ROIRect)},
        {"CROSSHAIR", typeof(ROICrosshair)},
        {"ELLIPSE", typeof(ROIEllipse)},
        {"POLYGON", typeof(ROIPoly)},
        {"POLYLINE", typeof(Polyline)},
        {"LINE", typeof(Line)},
        {"MIXED", null}
        };

        //save RGB in int32 in order: [A-B-G-R]
        //byte sections will be used in all necessary cases:
        //Mode:SecR, Version:SecA
        public static BitVector32.Section SecR = BitVector32.CreateSection(255);
        public static BitVector32.Section SecG = BitVector32.CreateSection(255, SecR);
        public static BitVector32.Section SecB = BitVector32.CreateSection(255, SecG);
        public static BitVector32.Section SecA = BitVector32.CreateSection(255, SecB);
        public static double TOLERANCE_ZUM = 0.5;

        //new fields for handling zoom level changes
        private const double _strokeThicknessMax = 10;
        private const double _strokeThicknessMin = .1;
        private double _roiStrokeThickness = 1;
        private double _roiScaleLineStrokeThicknessMultiplier = 2;
        private double _roiScaleNumberStrokeThicknessMultiplier = .4;


        private static BitVector32 _bitVec32;
        private static OverlayManagerClass _instance = null;

        private Type _activeType;
        private AdornerProvider _adornerProvider;
        private int _colorRGB = 0;
        private Mode _currentMode = Mode.STATSONLY;
        private string _expROIsPathAndName;
        private double _fieldHeight;
        private double _fieldWidth;
        private double _imageScaleX = 1.0;
        private double _imageScaleY = 1.0;
        private bool _isObjectComplete;
        private bool _isObjectCreated;

        // private Canvas _lastCanvas = null;
        private int _lastLineIndex;
        private short[] _mask;
        private ulong _maskIndex = 0;
        private bool _objectUpdated;
        ObservableCollection<UIElement> _overlayItems = new ObservableCollection<UIElement>();
        private int _patternID = 0;
        private bool _patternSubIndexVisible = true;
        private int[] _pixelUnitSizeXY = new int[2] { (int)Constants.PIXEL_X_MIN, (int)Constants.PIXEL_X_MIN };
        private int _pixelX;
        private int _pixelXAdjusted;
        private int _pixelY;
        private int _pixelYAdjusted;
        private bool _reviewWindowOpened = false;
        private AdornerLayer _roiAdornerLayer;
        private int _roiCount;
        private List<Shape> _roiList;
        private List<Shape> _roiListBackup;
        bool _roisLoaded = false;
        private List<Shape> _roiSpec;
        private bool _save = true;
        private bool _saveEveryChange;
        private bool _saveMaskEveryTime;
        private List<Shape> _scanAreaROIList;
        private Polygon _selectedPolygon;
        private bool _showLineLength = false;
        private bool _showPolylineLength = false;
        private bool _simulatorMode = false;
        private bool _skipRedrawCenter = false;
        private PixelSizeUM _umPerPixel;
        private bool _visible = true;
        private int _wavelengthNM = 0;
        private double _zoomLevel = 1;
        private double _zRefMM = 0.0;
        private double _zUM = 0.0;

        #endregion Fields

        #region Constructors

        private OverlayManagerClass()
        {
            mROIsDisableMoveAndResize = false;
        }

        #endregion Constructors

        #region Enumerations

        public enum AdornerType
        {
            GEOMETRY_ADORNER,
            ADJUST_ADORNER,
            INDEX_ADORNER,
            DIVIDER_ADORNER
        }

        public enum Flag
        {
            GEOMETRY_ADONERS = 0,
            ADJUST_ADONERS,
            STATS,
            DISPLAY,
            SUB_PATTERN_INDEX_ADONERS,
            SAVE,
            LAST_FLAG
        }

        public enum MouseEventEnum
        {
            LEFTSINGLECLICK,
            RIGHTSINGLECLICK,
            LEFTDOUBLECLICK,
            RIGHTDOUBLECLICK,
            LEFTHOLDING,
            RIGHTHOLDING,
            LEFTMOUSEUP,
            RIGHTMOUSEUP
        }

        public enum ROIType
        {
            RECTANGLE,
            POLYGON,
            LINE,
            POLYLINE,
            RETICLE,
            ELLIPSE,
        }

        #endregion Enumerations

        #region Events

        public event Action ClearedObjectEvent;

        public event Action ClearedStatsROIsEvent;

        public event Action MaskChangedEvent;

        public event Action MaskWillChangeEvent;

        public event Action<int> mROISelectedEvent;

        public event Action mROIsUpdated;

        public event Action mROIDeletedEvent;

        public event Action<Shape> NeedOverlayItemsUpdateToActiveROIs;

        public event Action<double, double> ObjectSizeChangedEvent;

        public event Action ParamsUpdatedEvent;

        public event Action ROIListUpdated;

        public event Action<bool> UpdatingObjectEvent;

        #endregion Events

        #region Properties

        public static double DefaultStrokeThickness
        {
            get
            {
                return 1.5;
            }
        }

        public static OverlayManagerClass Instance
        {
            get
            {
                if (null == _instance)
                {
                    _instance = new OverlayManagerClass();
                }

                return _instance;
            }
        }

        public int ColorRGB
        {
            get { return _colorRGB; }
            set { _colorRGB = value; }
        }

        public Mode CurrentMode
        {
            get { return _currentMode; }
            set
            {
                DefaultProperties(value);
            }
        }

        public double FieldWidth
        {
            get
            {
                return _fieldWidth;
            }
        }

        public double ImageXScale
        {
            get => _imageScaleX;
        }

        public double ImageYScale
        {
            get => _imageScaleY;
        }

        public bool mROIsDisableMoveAndResize
        {
            get; set;
        }

        public ObservableCollection<UIElement> OverlayItems
        {
            get => _overlayItems;
        }

        public int PatternID
        {
            get { return _patternID; }
            set { _patternID = value; }
        }

        public bool PatternSubIndexVisible
        {
            get { return _patternSubIndexVisible; }
            set { _patternSubIndexVisible = value; }
        }

        public int[] PixelUnitSizeXY
        {
            get { return _pixelUnitSizeXY; }
            set { _pixelUnitSizeXY = value; }
        }

        public int PixelX
        {
            get
            {
                return _pixelX;
            }
        }

        public int PixelY
        {
            get
            {
                return _pixelY;
            }
        }

        public bool ReviewWindowOpened
        {
            get { return _reviewWindowOpened; }
            set { _reviewWindowOpened = value; }
        }

        public int ROICount
        {
            get
            {
                if (_roiList == null)
                {
                    return 0;
                }
                return _roiList.Count;
            }
        }

        public bool ROIsLoaded
        {
            get => _roisLoaded;
        }

        public bool Save
        {
            get { return _save; }
            set { _save = value; }
        }

        public bool SaveEveryChange
        {
            get { return _saveEveryChange; }
            set { _saveEveryChange = value; }
        }

        /// <summary>
        /// True to ask overlay manager only work with given properties instead of hardware MVMs
        /// </summary>
        public bool SimulatorMode
        {
            get { return _simulatorMode; }
            set { _simulatorMode = value; }
        }

        // Define sub pattern ID is starting from zero;
        // Draw first with zeroth sub ID, no need to calculate zeroth pattern.
        public bool SkipRedrawCenter
        {
            get { return _skipRedrawCenter; }
            set { _skipRedrawCenter = value; }
        }

        public int StatsROICount
        {
            get
            {
                int count = 0;
                for (int i = 0; i < _roiList?.Count; ++i)
                {
                    Mode md = (Mode)(GetTagByteSection(_roiList[i].Tag, Tag.MODE, SecR));
                    if (md == Mode.STATSONLY)
                    {
                        ++count;
                    }
                }

                return count;
            }
        }

        public PixelSizeUM UmPerPixel
        {
            get
            {
                return _umPerPixel;
            }
        }

        public bool Visible
        {
            get { return _visible; }
            set { _visible = value; }
        }

        public int WavelengthNM
        {
            get { return _wavelengthNM; }
            set { _wavelengthNM = value; }
        }

        public double ZoomLevel
        {
            get
            {
                return _zoomLevel;
            }
            set
            {
                _zoomLevel = value;
                double newStrokeThickness = DefaultStrokeThickness * 1.5 / _zoomLevel;

                if (newStrokeThickness < _strokeThicknessMin)
                {
                    newStrokeThickness = _strokeThicknessMin;
                }
                if (newStrokeThickness > _strokeThicknessMax)
                {
                    newStrokeThickness = _strokeThicknessMax;
                }

                AdornerProvider.ZoomLevel = _zoomLevel;
                _roiStrokeThickness = newStrokeThickness;

                foreach (Shape roi in _roiList)
                {
                    if (null != roi)
                    {
                        if (roi is ScaleLines)
                        {
                            roi.StrokeThickness = _roiStrokeThickness * _roiScaleLineStrokeThicknessMultiplier;
                        }
                        else if (roi is ScaleNumbers)
                        {
                            roi.StrokeThickness = _roiStrokeThickness * _roiScaleNumberStrokeThicknessMultiplier;
                        }
                        else
                        {
                            roi.StrokeThickness = _roiStrokeThickness;
                        }
                    }
                }
                UpdateVisibleROIs();
            }
        }

        public double ZRefMM
        {
            get { return _zRefMM; }
            set { _zRefMM = value; }
        }

        public double ZUM
        {
            get
            {
                //Only get from ZControlViewModel if instance is not derived from Experiment Review window
                return _reviewWindowOpened || _simulatorMode ? _zUM : (double)MVMManager.Instance["ZControlViewModel", "ZPosition", (object)0.0] * (double)Constants.UM_TO_MM;
            }
            set
            {
                _zUM = value;
            }
        }

        #endregion Properties

        #region Methods

        /// <summary>
        /// use the XamlWriter and XamlReader to clone UIElement, may not foolproof
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="source"></param>
        /// <returns></returns>
        public static T CloneUIElementByXamlWriter<T>(T source)
        {
            string cloneObj = System.Windows.Markup.XamlWriter.Save(source);
            StringReader stringReader = new StringReader(cloneObj);
            System.Xml.XmlReader xmlReader = System.Xml.XmlReader.Create(stringReader);
            T target = (T)System.Windows.Markup.XamlReader.Load(xmlReader);
            return target;
        }

        public static byte GetTagByteSection(object tag, Tag tagID, BitVector32.Section section)
        {
            int[] localTag = (int[])tag;
            if (null != localTag)
            {
                if ((0 <= tagID) && ((int)tagID < localTag.Length))
                {
                    _bitVec32 = new BitVector32(localTag[(int)tagID]);
                    return (byte)(_bitVec32[section]);
                }
            }
            return (byte)0;
        }

        public static ROICapsule LoadXamlROIs(string pathandName = "")
        {
            try
            {
                pathandName = string.IsNullOrEmpty(pathandName) ? ResourceManagerCS.GetCaptureTemplatePathString() + "\\ActiveROIs.xaml" : pathandName;
                string strXaml = String.Empty;
                using (var reader = new System.IO.StreamReader(pathandName, true))
                {
                    strXaml = reader.ReadToEnd();
                    reader.Close();
                }

                ROICapsule roiCapsule = new ROICapsule();
                StringReader sreader = new StringReader(strXaml);
                XmlTextReader xreader = new XmlTextReader(sreader);
                roiCapsule = (XamlReader.Load(xreader) as ROICapsule);
                sreader.Close();
                xreader.Close();
                return roiCapsule;
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, "LoadXamlROIs exception " + ex.Message);
                return null;
            }
        }

        //At the present only adds a corner to a Polygon ROI or Polyline ROI
        public void AddPointToObject(Point pt)
        {
            if (false == _isObjectComplete && true == _isObjectCreated)
            {
                if (_activeType == typeof(ROIPoly))
                {
                    if (false == ((ROIPoly)_roiList[_roiList.Count - 1]).Closed)
                    {
                        _selectedPolygon.Points.Add(pt);
                        ((ROIPoly)_roiList[_roiList.Count - 1]).Points.Add(pt);
                        _overlayItems.Add(GetCornerEllipse(pt));

                    }
                }
                else if (_activeType == typeof(Polyline))
                {
                    ((Polyline)_roiList[_roiList.Count - 1]).Points.Add(pt);
                    _overlayItems.Add(GetCornerEllipse(pt));
                    if (true == _showPolylineLength)
                    {
                        UpdateGeometryAdorner(_roiList.Last());
                    }
                }
                else if (_activeType == typeof(ROIPolyline))
                {
                    ((ROIPolyline)_roiList[_roiList.Count - 1]).Points.Add(pt);
                    _overlayItems.Add(GetCornerEllipse(pt));
                    if (true == _showPolylineLength)
                    {
                        UpdateGeometryAdorner(_roiList.Last());
                    }
                }
            }
        }

        public void AdjustROIListAspectRatio(double xScale, double yScale, bool reset = false)
        {
            if (xScale <= 0 || yScale <= 0)
            {
                //invalid scale
                return;
            }

            double toAdjustX = (double)(_pixelX * xScale) / ((double)_pixelXAdjusted);
            double toAdjustY = (double)(_pixelY * yScale) / ((double)_pixelYAdjusted);

            _pixelXAdjusted = (int)(_pixelX * xScale);
            _pixelYAdjusted = (int)(_pixelY * yScale);
            _imageScaleX = xScale;
            _imageScaleY = yScale;
            for (int i = 0; i < _roiList.Count; i++)
            {
                //if (true == GetTagBit(_roiList[i].Tag, Tag.FLAGS, Flag.DISPLAY))
                {
                    if (_roiList[i] is ScalableShape)
                    {
                        (_roiList[i] as ScalableShape).ApplyScaleUpdate(xScale, yScale);
                    }

                    UpdatePanningAdorner(_roiList[i]);
                    UpdateGeometryAdorner(_roiList[i]);
                    UpdateIndexAdorner(_roiList[i]);
                    UpdateDividerAdorner(_roiList[i]);
                }
            }

            ROIListUpdated?.Invoke();
            if (_currentMode == Mode.MICRO_SCANAREA)
            {
                mROIsUpdated?.Invoke();
            }
        }

        /// <summary>
        ///  Append the Mode ROIs
        /// </summary>
        /// <param name="mode"></param>
        /// <param name="patternID"></param>
        /// <param name="rois"></param>
        /// <param name="waveLength"></param>
        public void AppendModeROIs(Mode mode, int patternID, List<Shape> rois, int waveLength)
        {
            List<Shape> roiList = new List<Shape>();

            for (int i = 0; i < _roiList.Count; i++)
            {
                if (mode == (Mode)(GetTagByteSection(_roiList[i].Tag, Tag.MODE, SecR)) &&
                    (0 < patternID && patternID == ((int[])_roiList[i].Tag)[(int)Tag.PATTERN_ID]))
                {
                    roiList.Add(_roiList[i]);
                }
            }

            //find the last sub pattern id for the current patternID
            int lspid = 0;
            for (int i = 0; i < roiList.Count; ++i)
            {
                if (lspid < ((int[])roiList[i].Tag)[(int)Tag.SUB_PATTERN_ID])
                {
                    lspid = ((int[])roiList[i].Tag)[(int)Tag.SUB_PATTERN_ID];
                }
            }

            //append the rois to the passed wavelenth
            for (int i = 0; i < rois.Count; ++i)
            {
                var newRoi = CloneUIElementByXamlWriter(rois[i]);
                int[] originalTag = (int[])rois[i].Tag;
                int[] newTag = DefaultTags(_roiCount);//newRoi.Tag as int[];
                ++lspid;
                newTag[(int)Tag.SUB_PATTERN_ID] = lspid;
                newTag[(int)Tag.PATTERN_ID] = patternID;
                newTag[(int)Tag.ROI_ID] = _roiCount;
                newTag[(int)Tag.MODE] = originalTag[(int)Tag.MODE];
                newTag[(int)Tag.RGB] = originalTag[(int)Tag.RGB];
                newTag[(int)Tag.FLAGS] = originalTag[(int)Tag.FLAGS];
                newTag[(int)Tag.WAVELENGTH_NM] = waveLength;
                ++_roiCount;
                newRoi.Tag = newTag;
                newRoi.ToolTip = "ROI #" + newTag[(int)Tag.SUB_PATTERN_ID];
                newRoi.MouseLeftButtonDown += ROI_MouseDown;
                _roiList.Add(newRoi);
                _overlayItems.Add(_roiList[_roiList.Count - 1]);
                AddIndexToROI(_roiList[_roiList.Count - 1]);
            }

            ValidateROIs();
        }

        /// <summary>
        /// back up roi list, can be retrieved by RevokeROIs
        /// </summary>
        public void BackupROIs()
        {
            _roiListBackup = new List<Shape>();
            int[] tag;
            for (int i = 0; i < _roiList.Count; i++)
            {
                //no panning adoners:
                tag = SetTagBit(_roiList[i].Tag, Tag.FLAGS, Flag.ADJUST_ADONERS, false);

                //replace with new shape, since modified size
                //will be carried over when revoke:
                if (typeof(ROIRect) == _roiList[i].GetType())
                {
                    ROIRect roiRect = new ROIRect(((ROIRect)_roiList[i]).StartPoint, ((ROIRect)_roiList[i]).EndPoint, ((ROIRect)_roiList[i]).ImageScaleX, ((ROIRect)_roiList[i]).ImageScaleY);
                    roiRect.Fill = ((ROIRect)_roiList[i]).Fill;
                    roiRect.Stroke = ((ROIRect)_roiList[i]).Stroke;
                    roiRect.StrokeThickness = ((ROIRect)_roiList[i]).StrokeThickness;
                    roiRect.MouseLeftButtonDown += ROI_MouseDown;
                    roiRect.Tag = tag.Clone();
                    _roiListBackup.Add(roiRect);
                }
                else if (typeof(ROIPoly) == _roiList[i].GetType())
                {
                    ROIPoly roiPoly = new ROIPoly(((ROIPoly)_roiList[i]).Points.Clone(), _imageScaleX, _imageScaleY);
                    roiPoly.Closed = ((ROIPoly)_roiList[i]).Closed;
                    roiPoly.Fill = ((ROIPoly)_roiList[i]).Fill;
                    roiPoly.Stroke = ((ROIPoly)_roiList[i]).Stroke;
                    roiPoly.StrokeThickness = ((ROIPoly)_roiList[i]).StrokeThickness;
                    roiPoly.MouseLeftButtonDown += ROI_MouseDown;
                    roiPoly.Tag = tag.Clone();
                    _roiListBackup.Add(roiPoly);
                }
                else if (typeof(Line) == _roiList[i].GetType())
                {
                    Line roiLine = new Line();
                    roiLine.X1 = ((Line)_roiList[i]).X1;
                    roiLine.Y1 = ((Line)_roiList[i]).Y1;
                    roiLine.X2 = ((Line)_roiList[i]).X2;
                    roiLine.Y2 = ((Line)_roiList[i]).Y2;
                    roiLine.Fill = ((Line)_roiList[i]).Fill;
                    roiLine.Stroke = ((Line)_roiList[i]).Stroke;
                    roiLine.StrokeThickness = ((Line)_roiList[i]).StrokeThickness;
                    roiLine.MouseLeftButtonDown += ROI_MouseDown;
                    roiLine.Tag = tag.Clone();
                    _roiListBackup.Add(roiLine);
                }
                else if (typeof(ROILine) == _roiList[i].GetType())
                {
                    Line roiLine = new Line();
                    roiLine.X1 = ((ROILine)_roiList[i]).StartPoint.X;
                    roiLine.Y1 = ((ROILine)_roiList[i]).StartPoint.Y;
                    roiLine.X2 = ((ROILine)_roiList[i]).EndPoint.X;
                    roiLine.Y2 = ((ROILine)_roiList[i]).EndPoint.Y;
                    roiLine.Fill = ((ROILine)_roiList[i]).Fill;
                    roiLine.Stroke = ((ROILine)_roiList[i]).Stroke;
                    roiLine.StrokeThickness = ((ROILine)_roiList[i]).StrokeThickness;
                    roiLine.MouseLeftButtonDown += ROI_MouseDown;
                    roiLine.Tag = tag.Clone();
                    _roiListBackup.Add(roiLine);
                }
                else if (typeof(Reticle) == _roiList[i].GetType())
                {
                    Reticle roiReticle = new Reticle(_imageScaleX, _imageScaleY);
                    roiReticle.ImageHeight = ((Reticle)_roiList[i]).ImageHeight;
                    roiReticle.ImageWidth = ((Reticle)_roiList[i]).ImageWidth;
                    roiReticle.Fill = ((Reticle)_roiList[i]).Fill;
                    roiReticle.Stroke = ((Reticle)_roiList[i]).Stroke;
                    roiReticle.StrokeThickness = ((Reticle)_roiList[i]).StrokeThickness;
                    roiReticle.MouseLeftButtonDown += ROI_MouseDown;
                    roiReticle.Tag = tag.Clone();
                    _roiListBackup.Add(roiReticle);
                }
                else if (typeof(ScaleLines) == _roiList[i].GetType())
                {
                    ScaleLines roiScaleL = new ScaleLines();
                    roiScaleL.ScaleFieldWidth = ((ScaleLines)_roiList[i]).ScaleFieldWidth;
                    roiScaleL.ImageHeight = ((ScaleLines)_roiList[i]).ImageHeight;
                    roiScaleL.ImageWidth = ((ScaleLines)_roiList[i]).ImageWidth;
                    roiScaleL.ImageScaleX = ((ScaleLines)_roiList[i]).ImageScaleX;
                    roiScaleL.ImageScaleY = ((ScaleLines)_roiList[i]).ImageScaleY;
                    roiScaleL.Fill = ((ScaleLines)_roiList[i]).Fill;
                    roiScaleL.Stroke = ((ScaleLines)_roiList[i]).Stroke;
                    roiScaleL.StrokeThickness = ((ScaleLines)_roiList[i]).StrokeThickness;
                    roiScaleL.Tag = tag.Clone();
                    _roiListBackup.Add(roiScaleL);
                }
                else if (typeof(ScaleNumbers) == _roiList[i].GetType())
                {
                    ScaleNumbers roiScale = new ScaleNumbers();
                    roiScale.ScaleFieldWidth = ((ScaleNumbers)_roiList[i]).ScaleFieldWidth;
                    roiScale.ImageHeight = ((ScaleNumbers)_roiList[i]).ImageHeight;
                    roiScale.ImageWidth = ((ScaleNumbers)_roiList[i]).ImageWidth;
                    roiScale.ImageScaleX = ((ScaleLines)_roiList[i]).ImageScaleX;
                    roiScale.ImageScaleY = ((ScaleLines)_roiList[i]).ImageScaleY;
                    roiScale.Fill = ((ScaleNumbers)_roiList[i]).Fill;
                    roiScale.Stroke = ((ScaleNumbers)_roiList[i]).Stroke;
                    roiScale.StrokeThickness = ((ScaleNumbers)_roiList[i]).StrokeThickness;
                    roiScale.Tag = tag.Clone();
                    _roiListBackup.Add(roiScale);
                }
                else if (typeof(ROICrosshair) == _roiList[i].GetType())
                {
                    ROICrosshair roiCrosshair = new ROICrosshair(((ROICrosshair)_roiList[i]).CenterPoint, _imageScaleX, _imageScaleY);
                    roiCrosshair.Fill = ((ROICrosshair)_roiList[i]).Fill;
                    roiCrosshair.Stroke = ((ROICrosshair)_roiList[i]).Stroke;
                    roiCrosshair.StrokeThickness = ((ROICrosshair)_roiList[i]).StrokeThickness;
                    roiCrosshair.MouseLeftButtonDown += ROI_MouseDown;
                    roiCrosshair.Tag = tag.Clone();
                    _roiListBackup.Add(roiCrosshair);
                }
                else if (typeof(Polyline) == _roiList[i].GetType())
                {
                    Polyline roiPolyLine = new Polyline();
                    roiPolyLine.Points = ((Polyline)_roiList[i]).Points.Clone();
                    roiPolyLine.Fill = ((Polyline)_roiList[i]).Fill;
                    roiPolyLine.Stroke = ((Polyline)_roiList[i]).Stroke;
                    roiPolyLine.StrokeThickness = ((Polyline)_roiList[i]).StrokeThickness;
                    roiPolyLine.MouseLeftButtonDown += ROI_MouseDown;
                    roiPolyLine.Tag = tag.Clone();
                    _roiListBackup.Add(roiPolyLine);
                }
                else if (typeof(ROIPolyline) == _roiList[i].GetType())
                {
                    ROIPolyline roiPolyLine = new ROIPolyline(((ROIPolyline)_roiList[i]).Points.Clone(), _imageScaleX, _imageScaleY);
                    roiPolyLine.Fill = ((ROIPolyline)_roiList[i]).Fill;
                    roiPolyLine.Stroke = ((ROIPolyline)_roiList[i]).Stroke;
                    roiPolyLine.StrokeThickness = ((ROIPolyline)_roiList[i]).StrokeThickness;
                    roiPolyLine.MouseLeftButtonDown += ROI_MouseDown;
                    roiPolyLine.Tag = tag.Clone();
                    _roiListBackup.Add(roiPolyLine);
                }
                else if (typeof(ROIEllipse) == _roiList[i].GetType())
                {
                    ROIEllipse roiEllipse = new ROIEllipse(((ROIEllipse)_roiList[i]).StartPoint, ((ROIEllipse)_roiList[i]).EndPoint, _imageScaleX, _imageScaleY);
                    roiEllipse.Center = ((ROIEllipse)_roiList[i]).Center;
                    roiEllipse.Fill = ((ROIEllipse)_roiList[i]).Fill;
                    roiEllipse.Stroke = ((ROIEllipse)_roiList[i]).Stroke;
                    roiEllipse.StrokeThickness = ((ROIEllipse)_roiList[i]).StrokeThickness;
                    roiEllipse.MouseLeftButtonDown += ROI_MouseDown;
                    roiEllipse.Tag = tag.Clone();
                    _roiListBackup.Add(roiEllipse);
                }
            }
        }

        /// <summary>
        /// compare bleach ROIs and current ROIs via file hash.
        /// </summary>
        /// <param name="path"></param>
        /// <returns></returns>
        public bool BleachCompareROIs(string path)
        {
            string tmpPath1 = path + "\\" + "tmpROIsBl.xaml";
            string tmpPath2 = path + "\\" + "tmpROIsCr.xaml";
            string bleachPath = path + "\\" + "BleachROIs.xaml";
            if (!File.Exists(bleachPath))
            {
                return false;
            }
            try
            {
                Application.Current.Dispatcher.Invoke((Action)(() =>
                {
                    //tmpPath2: current ROIs
                    SaveROIs(tmpPath2);
                    ROICapsule tmpCapsule = LoadXamlROIs(tmpPath2);
                    BleachCreateROIs(tmpPath2, tmpCapsule);

                    //tmpPath1: bleach ROIs
                    ROICapsule roiCapsule = LoadXamlROIs(bleachPath);
                    BleachCreateROIs(tmpPath1, roiCapsule);
                }));
                if (!File.Exists(tmpPath1) || !File.Exists(tmpPath2))
                {
                    File.Delete(tmpPath1);
                    File.Delete(tmpPath2);
                    return false;
                }

                bool ret = true;
                Byte[] bleachArray, tmpArray;
                using (var md5 = MD5.Create())
                {
                    using (var stream = File.OpenRead(tmpPath1))
                    {
                        bleachArray = md5.ComputeHash(stream);
                    }
                    using (var stream = File.OpenRead(tmpPath2))
                    {
                        tmpArray = md5.ComputeHash(stream);
                    }
                }
                if ((null == bleachArray) || (null == tmpArray))
                {
                    File.Delete(tmpPath1);
                    File.Delete(tmpPath2);
                    return false;
                }
                if (bleachArray.Length != tmpArray.Length)
                {
                    ret = false;
                }
                else
                {
                    for (int i = 0; i < bleachArray.Length; i++)
                    {
                        if (bleachArray[i] != tmpArray[i])
                        {
                            ret = false;
                            break;
                        }
                    }
                }
                File.Delete(tmpPath1);
                File.Delete(tmpPath2);
                return ret;
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + "BleachCompareROIs exception " + ex.Message);
                File.Delete(tmpPath1);
                File.Delete(tmpPath2);
                return false;
            }
        }

        public void BleachSaveROIs(string path)
        {
            try
            {
                if (_roiList.Count > 0)
                {
                    List<Shape> tmpList = new List<Shape>();
                    for (int i = 0; i < _roiList.Count; i++)
                    {
                        if ((_roiList[i].ToString() != "OverlayManager.Reticle") && (true == GetTagBit(_roiList[i].Tag, Tag.FLAGS, Flag.SAVE)))
                        {
                            _roiList[i].Tag = SetTagRGB(_roiList[i]);
                            tmpList.Add(_roiList[i]);
                        }
                    }
                    if (tmpList.Count > 0)
                    {
                        ROICapsule newCapsule = new ROICapsule();
                        newCapsule.ROIs = new Shape[tmpList.Count];
                        newCapsule.ROIs = tmpList.ToArray();

                        string pathandname = path + "\\" + "BleachROIs.xaml";
                        if (!File.Exists(pathandname))
                        {
                            BleachCreateROIs(pathandname, newCapsule);
                        }
                        else
                        {
                            string tmpName = path + "\\" + "tmpROIs.xaml";
                            BleachCreateROIs(tmpName, newCapsule);
                            ROICapsule oldCapsule = LoadXamlROIs(pathandname);

                            if (oldCapsule != null)
                            {

                                XmlDocument newDoc = new XmlDocument();
                                XmlReader newReader = new XmlTextReader(tmpName);
                                newDoc.Load(newReader);
                                XmlNodeList newNodes = newDoc.DocumentElement.ChildNodes.Item(0).ChildNodes.Item(0).ChildNodes;

                                XmlDocument oldDoc = new XmlDocument();
                                XmlReader oldReader = new XmlTextReader(pathandname);
                                oldDoc.Load(oldReader);
                                XmlNodeList oldNodes = oldDoc.DocumentElement.ChildNodes.Item(0).ChildNodes.Item(0).ChildNodes;
                                int idx = 0;
                                foreach (XmlNode node in newNodes)
                                {
                                    //persist bleach params:
                                    string str = string.Empty;
                                    string nodeName = (node.Name.Contains("av:")) ? node.Name.Substring(3, node.Name.Length - 3) : node.Name;
                                    if ((idx < oldCapsule.ROIs.Length) && (oldCapsule.ROIs[idx].ToString().IndexOf(nodeName) > 0))
                                    {
                                        for (int ib = 0; ib < BleachClass.BleachAttributes.Length; ib++)
                                        {
                                            if (BleachClass.GetBleachAttribute(oldNodes[idx], oldDoc, ib, ref str))
                                            {
                                                BleachClass.SetBleachAttribute(node, newDoc, ib, str);
                                            }
                                        }
                                    }
                                    idx++;
                                }
                                oldReader.Close();
                                newReader.Close();
                                newDoc.Save(pathandname);
                            }
                            else
                            {
                                BleachCreateROIs(pathandname, newCapsule);
                            }
                            File.Delete(tmpName);
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + "BleachSaveROIs exception " + ex.Message);
            }
        }

        public void ClearAllObjects()
        {
            List<Shape> roiListKeep = new List<Shape>();
            for (int i = 0; i < _roiList.Count; i++)
            {
                Mode md = (Mode)(GetTagByteSection(_roiList[i].Tag, Tag.MODE, SecR));
                switch (_currentMode)
                {
                    case Mode.MICRO_SCANAREA:
                        if (Mode.MICRO_SCANAREA != (Mode)(GetTagByteSection(_roiList[i].Tag, Tag.MODE, SecR)))
                        {
                            roiListKeep.Add(_roiList[i]);
                        }
                        break;
                    case Mode.PATTERN_NOSTATS:
                    case Mode.PATTERN_WIDEFIELD:
                    case Mode.STATSONLY:
                        if ((Mode.PATTERN_NOSTATS == md || Mode.PATTERN_WIDEFIELD == md) &&
                            (_patternID != ((int[])_roiList[i].Tag)[(int)Tag.PATTERN_ID]))
                        {
                            roiListKeep.Add(_roiList[i]);
                        }
                        break;
                    case Mode.LAST_MODE:
                    default:
                        break;
                }
            }

            _roiList = roiListKeep;
            _mask = null;
            CreateMask();

            _isObjectComplete = false;
            _isObjectCreated = false;
            _activeType = null;
            UpdateVisibleROIs();

            _roiCount = _roiList.Count;
            _lastLineIndex = 0;
            for (int i = 0; i < _roiList.Count; i++)
            {
                if (_roiList[i] is Line)
                {
                    _lastLineIndex = i;
                }
            }
            if (true == _saveEveryChange)
            {
                SaveROIs(_expROIsPathAndName);
            }

            ClearedObjectEvent?.Invoke();

            if (_currentMode == Mode.STATSONLY)
            {
                ClearedStatsROIsEvent?.Invoke();
            }

            if (_currentMode == Mode.MICRO_SCANAREA)
            {
                mROIsUpdated?.Invoke();
            }
        }

        public void ClearNonSaveROIs()
        {
            List<Shape> roiKeep = new List<Shape>();
            for (int i = 0; i < _roiList.Count; i++)
            {
                if (true == GetTagBit((int[])_roiList[i].Tag, Tag.FLAGS, Flag.SAVE))
                {
                    roiKeep.Add(_roiList[i]);
                    //remove panning adoner, can be separate in future:
                    if (true == RemoveAdornerFromROI(_roiList[i], AdornerType.ADJUST_ADORNER))
                    {
                        int[] tag = SetTagBit(_roiList[i].Tag, Tag.FLAGS, Flag.ADJUST_ADONERS, false);
                        _roiList[i].Tag = tag;
                    }
                }
            }
            _roiList = roiKeep;

            UpdateVisibleROIs();

            if (_currentMode == Mode.MICRO_SCANAREA)
            {
                mROIsUpdated?.Invoke();
            }
        }

        public void CloseObject(Point pt)
        {
            if (false == _isObjectComplete && true == _isObjectCreated)
            {
                if (_activeType == typeof(ROIPoly))
                {
                    if (((ROIPoly)_roiList[_roiList.Count - 1]).Closed == false && 3 < ((ROIPoly)_roiList[_roiList.Count - 1]).Points.Count)
                    {
                        try
                        {
                            int points = ((ROIPoly)_roiList[_roiList.Count - 1]).Points.Count;
                            for (int i = 0; i < points; i++)
                            {
                                _overlayItems.RemoveAt(_overlayItems.Count - 1);
                            }
                            _overlayItems.Remove(_selectedPolygon);

                            ((ROIPoly)_roiList[_roiList.Count - 1]).Points.RemoveAt(points - 1);

                            ((ROIPoly)_roiList[_roiList.Count - 1]).Closed = true;
                            ObjectComplete(_roiList.Count - 1);
                            _roiCount++;

                            if (null != UpdatingObjectEvent) UpdatingObjectEvent(false); //New objects can be created at this point

                            //add index adorner:
                            AddIndexToROI(_roiList[_roiList.Count - 1]);

                            InitROIPoly();
                        }
                        catch (Exception e)
                        {
                            e.ToString();
                        }
                    }
                }
                else if (_activeType == typeof(Polyline) && 2 < ((Polyline)_roiList[_roiList.Count - 1]).Points.Count)
                {
                    int points = ((Polyline)_roiList[_roiList.Count - 1]).Points.Count;
                    for (int i = 0; i < points; i++)
                    {
                        if (_overlayItems.Count <= 0)
                        {
                            break;
                        }
                        _overlayItems.RemoveAt(_overlayItems.Count - 1);
                    }

                    ((Polyline)_roiList[_roiList.Count - 1]).Points.RemoveAt(points - 1);

                    ObjectComplete(_roiList.Count - 1);
                    _roiCount++;

                    if (null != UpdatingObjectEvent) UpdatingObjectEvent(false); //New objects can be created at this point

                    //add index adorner:
                    AddIndexToROI(_roiList[_roiList.Count - 1]);

                    InitROIPolyline();
                }
                else if (_activeType == typeof(ROIPolyline) && 2 < ((ROIPolyline)_roiList[_roiList.Count - 1]).Points.Count)
                {
                    int points = ((ROIPolyline)_roiList[_roiList.Count - 1]).Points.Count;
                    for (int i = 0; i < points; i++)
                    {
                        if (_overlayItems.Count <= 0)
                        {
                            break;
                        }
                        _overlayItems.RemoveAt(_overlayItems.Count - 1);
                    }

                    ((ROIPolyline)_roiList[_roiList.Count - 1]).Points.RemoveAt(points - 1);

                    ObjectComplete(_roiList.Count - 1);
                    _roiCount++;

                    if (null != UpdatingObjectEvent) UpdatingObjectEvent(false); //New objects can be created at this point

                    //add index adorner:
                    AddIndexToROI(_roiList[_roiList.Count - 1]);

                    InitROIPolyline();
                }

                UpdateVisibleROIs();
            }
        }

        public void CreateROICenter()
        {
            //return if sub ID is zero-based
            if (SkipRedrawCenter)
                return;

            //return if not valid shapes
            Point pt = new Point(0, 0);
            if (0 < _roiList.Count)
            {
                if (typeof(ROICrosshair) == ROITypeDictionary.FirstOrDefault(x => x.Value == _roiList[_roiList.Count - 1].GetType()).Value)
                    pt = (_roiList[_roiList.Count - 1] as ROICrosshair).CenterPoint;
                else if (typeof(ROIEllipse) == ROITypeDictionary.FirstOrDefault(x => x.Value == _roiList[_roiList.Count - 1].GetType()).Value)
                    pt = (_roiList[_roiList.Count - 1] as ROIEllipse).ROICenter;
                else
                    return;
            }

            //return if not in NOSTATS mode or already created:
            if (Mode.PATTERN_NOSTATS != _currentMode)
                return;
            for (int i = 0; i < _roiList.Count; i++)
            {
                if ((_patternID == ((int[])_roiList[i].Tag)[(int)Tag.PATTERN_ID]) && (0 == ((int[])_roiList[i].Tag)[(int)Tag.SUB_PATTERN_ID]))
                    return;
            }

            _selectedPolygon = null;
            int[] tag = DefaultTags(_roiCount);
            tag[(int)Tag.SUB_PATTERN_ID] = 0;

            _bitVec32 = new BitVector32(tag[(int)Tag.RGB]);
            if (typeof(ROICrosshair) == _roiList[_roiList.Count - 1].GetType())
            {
                ROICrosshair crosshair = new ROICrosshair()
                {
                    Stroke = (Mode.PATTERN_NOSTATS == _currentMode) ?
                        new SolidColorBrush(Color.FromArgb(Byte.MaxValue, Convert.ToByte(_bitVec32[SecR]), Convert.ToByte(_bitVec32[SecG]), Convert.ToByte(_bitVec32[SecB]))) :
                        GetROIColor((tag[(int)Tag.SUB_PATTERN_ID] - 1) % 8),
                    StrokeThickness = 1,
                    Fill = Brushes.Transparent
                };
                crosshair.CenterPoint = pt;
                crosshair.MouseLeftButtonDown += ROI_MouseDown;
                _roiList.Add(crosshair);
            }
            else if (typeof(ROIEllipse) == _roiList[_roiList.Count - 1].GetType())
            {
                ROIEllipse ellipse = new ROIEllipse(_roiList[_roiList.Count - 1] as ROIEllipse)
                {
                    Stroke = (Mode.PATTERN_NOSTATS == _currentMode) ?
                        new SolidColorBrush(Color.FromArgb(Byte.MaxValue, Convert.ToByte(_bitVec32[SecR]), Convert.ToByte(_bitVec32[SecG]), Convert.ToByte(_bitVec32[SecB]))) :
                        GetROIColor((tag[(int)Tag.SUB_PATTERN_ID] - 1) % 8),
                    StrokeThickness = _roiStrokeThickness,
                    Fill = Brushes.Transparent
                };
                ellipse.MouseLeftButtonDown += ROI_MouseDown;
                _roiList.Add(ellipse);
            }
            _roiCount++;

            _roiList[_roiList.Count - 1].ToolTip = "Center";
            _roiList[_roiList.Count - 1].Tag = tag;
            _roiList[_roiList.Count - 1].Tag = SetTagRGB(_roiList[_roiList.Count - 1]);

            if (!_simulatorMode) _overlayItems.Add(_roiList[_roiList.Count - 1]);
            ObjectComplete(_roiList.Count - 1);

            //add index adorner:
            AddIndexToROI(_roiList[_roiList.Count - 1]);
        }

        /// <summary>
        /// create ROI according to shape and range from start to end
        /// </summary>
        /// <param name="canvas"></param>
        /// <param name="shape"></param>
        /// <param name="startPt"></param>
        /// <param name="endPt"></param>
        public void CreateROIShape(Type shape, Point startPt, Point endPt)
        {
            _isObjectComplete = false;
            _isObjectCreated = false;
            _activeType = shape;
            CreateObject(startPt);
            ResizeObject(endPt, false);

            //user must close object later after adding points to object
            if (typeof(ROIPoly) != shape && typeof(ROIPolyline) != shape && typeof(Polyline) != shape)
                ObjectComplete(_roiList.Count - 1);
        }

        public void DefaultProperties(Mode mode)
        {
            _currentMode = mode;
            _visible = true;
            _colorRGB = 0;
            _patternSubIndexVisible = true;
            switch (mode)
            {
                case Mode.STATSONLY:
                case Mode.MICRO_SCANAREA:
                    _patternID = 0;
                    _save = true;
                    break;
                case Mode.PATTERN_NOSTATS:
                case Mode.PATTERN_WIDEFIELD:
                    _save = false;
                    break;
                default:
                    break;
            }
        }

        public void DeletePatternROI(int patternID = -1, Mode pMode = Mode.PATTERN_NOSTATS)
        {
            //keep other patterns and stats:
            List<Shape> localListToKeep = new List<Shape>();

            for (int i = 0; i < _roiList.Count; i++)
            {
                Mode roiMode = (Mode)GetTagByteSection(_roiList[i].Tag, Tag.MODE, SecR);
                // [patternID: -1, delete all]
                if ((pMode != roiMode) || (0 < patternID && (patternID != ((int[])_roiList[i].Tag)[(int)Tag.PATTERN_ID])))
                {
                    localListToKeep.Add(_roiList[i]);
                }
                //reassign later pattern IDs forward
                if ((pMode == roiMode) && (0 < patternID) && (patternID < ((int[])_roiList[i].Tag)[(int)Tag.PATTERN_ID]))
                {
                    int[] tag = (int[])_roiList[i].Tag;
                    tag[(int)Tag.PATTERN_ID]--;
                    _roiList[i].Tag = tag;
                }

            }
            _roiList = localListToKeep;

            UpdateVisibleROIs();
        }

        public void DeleteSelectedROIs()
        {
            _lastLineIndex = 0;
            List<Shape> roiList = new List<Shape>();

            for (int i = _roiList.Count - 1; i >= 0; i--)
            {
                if (true == GetTagBit(_roiList[i].Tag, Tag.FLAGS, Flag.ADJUST_ADONERS)) // check if ROI is selected
                {
                    // only delete ROI under edit:
                    if ((Mode.STATSONLY == (Mode)(GetTagByteSection(_roiList[i].Tag, Tag.MODE, SecR))) ||
                        (_patternID == ((int[])_roiList[i].Tag)[(int)Tag.PATTERN_ID]))
                    {
                        _overlayItems.Remove(_roiList[i]);
                        _roiList.RemoveAt(i);
                    }
                }
            }

            for (int i = 0; i < _roiList.Count; i++)
            {
                if (_roiList[i] is Line)
                {
                    _lastLineIndex = i;
                }
            }
            ReorderROIs();
            CreateMask();
            UpdateVisibleROIs();

            if (true == _saveEveryChange)
            {
                SaveROIs(_expROIsPathAndName);
            }
            ClearedObjectEvent?.Invoke();

            bool hasStatsROIs = false;
            for (int i = 0; i < _roiList.Count; i++)
            {
                Mode md = (Mode)(GetTagByteSection(_roiList[i].Tag, Tag.MODE, SecR));
                if (md == Mode.STATSONLY)
                {
                    hasStatsROIs = true;
                    break;
                }
            }

            if (!hasStatsROIs)
            {
                ClearedStatsROIsEvent?.Invoke();
            }

            if (_currentMode == Mode.MICRO_SCANAREA)
            {
                mROIsUpdated?.Invoke();
            }
        }

        public void DeselectAllROIs()
        {
            if (null == _roiList || 0 == _roiList.Count)
            {
                return;
            }
            if (true == _isObjectComplete)
            {
                for (int i = 0; i < _roiList.Count; i++)
                {
                    if (false == (_roiList[i] is Reticle) || false == (_roiList[i] is Scale))
                    {
                        DeselectROI(_roiList[i]);
                    }
                }
            }
        }

        /// <summary>
        /// dim wavelength ROIs other than target wavelength to attenuation value
        /// </summary>
        /// <param name="canvas"></param>
        /// <param name="wavelength"></param>
        /// <param name="attenuation"></param>
        public void DimWavelengthROI()
        {
            for (int i = 0; i < _roiList.Count; i++)
            {
                _roiList[i].Stroke = new SolidColorBrush(Color.FromArgb(
                    (_wavelengthNM == ((int[])_roiList[i].Tag)[(int)Tag.WAVELENGTH_NM]) ? Byte.MaxValue : ATTENUATE_VALUE,
                    ((SolidColorBrush)_roiList[i].Stroke).Color.R,
                    ((SolidColorBrush)_roiList[i].Stroke).Color.G,
                    ((SolidColorBrush)_roiList[i].Stroke).Color.B));
            }
            UpdateVisibleROIs();
        }

        public void DisplayModeROI(Mode[] mode, bool setVisible)
        {
            for (int i = 0; i < _roiList.Count; i++)
            {
                for (int j = 0; j < mode.Count(); j++)
                {
                    if (mode[j] == (Mode)(GetTagByteSection(_roiList[i].Tag, Tag.MODE, SecR)))
                    {
                        SetTagBit(_roiList[i].Tag, Tag.FLAGS, Flag.DISPLAY, setVisible);
                    }
                }
            }

            UpdateVisibleROIs();
        }

        public void DisplayOnlyPatternROIs(Mode[] mode, int patternID, int[] wavelengthsNM)
        {
            if (mode == null || wavelengthsNM == null) return;

            for (int i = 0; i < _roiList.Count; i++)
            {
                bool displaying = false;
                var roiMode = (Mode)(GetTagByteSection(_roiList[i].Tag, Tag.MODE, SecR));
                var roiWavelength = ((int[])_roiList[i].Tag)[(int)Tag.WAVELENGTH_NM];
                var roiPatternID = ((int[])_roiList[i].Tag)[(int)Tag.PATTERN_ID];
                for (int j = 0; j < mode.Length; j++)
                {
                    for (int k = 0; k < wavelengthsNM.Length; k++)
                    {
                        if (mode[j] == roiMode && roiPatternID == patternID && (wavelengthsNM[k] == roiWavelength || wavelengthsNM[k] < 0))
                        {
                            SetTagBit(_roiList[i].Tag, Tag.FLAGS, Flag.DISPLAY, true);
                            displaying = true;
                        }
                    }
                }

                if (!displaying)
                {
                    SetTagBit(_roiList[i].Tag, Tag.FLAGS, Flag.DISPLAY, false);
                }
            }

            UpdateVisibleROIs();
        }

        public void DisplayPatternROI(int patternID, bool setVisible)
        {
            for (int i = 0; i < _roiList.Count; i++)
            {
                if (0 > patternID || patternID == ((int[])_roiList[i].Tag)[(int)Tag.PATTERN_ID])
                {
                    SetTagBit(_roiList[i].Tag, Tag.FLAGS, Flag.DISPLAY, setVisible);
                }
            }

            UpdateVisibleROIs();
        }

        public void DuplicatePatternROIs(int patternID, Mode patternMode = Mode.PATTERN_NOSTATS)
        {
            //keep other patterns and stats:
            List<Shape> listToDuplicate = new List<Shape>();
            int maxPatternID = 0;
            for (int i = 0; i < _roiList.Count; i++)
            {
                Mode roiMode = (Mode)GetTagByteSection(_roiList[i].Tag, Tag.MODE, SecR);

                if ((patternMode == roiMode) && 0 < patternID && (patternID == ((int[])_roiList[i].Tag)[(int)Tag.PATTERN_ID]))
                {
                    listToDuplicate.Add(_roiList[i]);
                }
                //reassign later pattern IDs forward
                if ((patternMode == roiMode) && (maxPatternID < ((int[])_roiList[i].Tag)[(int)Tag.PATTERN_ID]))
                {
                    int[] tag = (int[])_roiList[i].Tag;
                    maxPatternID = tag[(int)Tag.PATTERN_ID];
                }
            }

            int newPatternID = maxPatternID + 1;
            List<Shape> roiList = new List<Shape>();

            //append the rois to the passed wavelenth
            for (int i = 0; i < listToDuplicate.Count; ++i)
            {
                var newRoi = CloneUIElementByXamlWriter(listToDuplicate[i]);

                int[] newTag = DefaultTags(_roiCount);//newRoi.Tag as int[];
                int[] originalTag = (int[])listToDuplicate[i].Tag;
                newTag[(int)Tag.PATTERN_ID] = newPatternID;
                newTag[(int)Tag.SUB_PATTERN_ID] = originalTag[(int)Tag.SUB_PATTERN_ID];
                newTag[(int)Tag.ROI_ID] = _roiCount;
                newTag[(int)Tag.WAVELENGTH_NM] = originalTag[(int)Tag.WAVELENGTH_NM];
                newTag[(int)Tag.MODE] = originalTag[(int)Tag.MODE];
                newTag[(int)Tag.RGB] = originalTag[(int)Tag.RGB];
                newTag[(int)Tag.FLAGS] = originalTag[(int)Tag.FLAGS];
                ++_roiCount;
                newRoi.Tag = newTag;
                newRoi.ToolTip = "ROI #" + newTag[(int)Tag.SUB_PATTERN_ID];
                newRoi.MouseLeftButtonDown += ROI_MouseDown;
                _roiList.Add(newRoi);
                _overlayItems.Add(_roiList[_roiList.Count - 1]);
                AddIndexToROI(_roiList[_roiList.Count - 1]);
            }

            ValidateROIs();
        }

        /// <summary>
        /// get current shape
        /// </summary>
        /// <returns></returns>
        public Shape GetCurrentROI()
        {
            return (0 < _roiList.Count) ? _roiList.Last() : null;
        }

        /// <summary>
        /// get current shape length if ROILine or ROIPolyline. Otherwise return 0
        /// </summary>
        /// <returns>length of current ROI if ROILine or ROIPolyline otherwise 0</returns>
        public double GetCurrentROILength()
        {

            if (GetCurrentROI() is ROILine)
            {

                ROILine r = (ROILine)GetCurrentROI();
                PixelSizeUM pixelSizeUM = ((PixelSizeUM)MVMManager.Instance["AreaControlViewModel", "PixelSizeUM", (object)null]);
                double pixelWidthUM = pixelSizeUM.PixelWidthUM;
                double pixelHeightUM = pixelSizeUM.PixelHeightUM;

                return Math.Round(Math.Sqrt(Math.Pow(pixelWidthUM * (r.EndPoint.X - r.StartPoint.X) / OverlayManagerClass.Instance.ImageXScale, 2)
                    + Math.Pow(pixelHeightUM * (r.EndPoint.Y - r.StartPoint.Y) / OverlayManagerClass.Instance.ImageYScale, 2))) + 1;

            }
            else if (GetCurrentROI() is ROIPolyline)
            {
                int polylineLength = 1;
                Application.Current.Dispatcher.Invoke((Action)delegate
                {

                    ROIPolyline polyline = (ROIPolyline)GetCurrentROI();
                    if (2 <= polyline.Points.Count)
                    {

                        int x1;
                        int x2;
                        int y1;
                        int y2;

                        for (int i = 1; i < polyline.Points.Count; i++)
                        {
                            x1 = (int)Math.Floor(polyline.Points[i - 1].X);
                            x2 = (int)Math.Floor(polyline.Points[i].X);
                            y1 = (int)Math.Floor(polyline.Points[i - 1].Y);
                            y2 = (int)Math.Floor(polyline.Points[i].Y);
                            polylineLength += (int)Math.Round(Math.Sqrt(Math.Pow(_umPerPixel.PixelWidthUM * (x2 - x1) / _imageScaleX, 2) + Math.Pow(_umPerPixel.PixelHeightUM * (y2 - y1) / _imageScaleY, 2)));
                        }
                    }
                });
                return polylineLength;
            }
            else
            {
                return 0;
            }
        }

        public void GetDuplicatedROIList(ref ObservableCollection<UIElement> duplicatedROIs)
        {
            bool skipRedrawCenterBackup = _skipRedrawCenter;
            _skipRedrawCenter = true;   //get duplicate no need to redraw
            //ValidateROIs();
            duplicatedROIs.Clear();

            for (int i = 0; i < _roiList.Count; i++)
            {
                if (true == GetTagBit(_roiList[i].Tag, Tag.FLAGS, Flag.DISPLAY))
                {
                    var duplicatedROI = CloneUIElementByXamlWriter(_roiList[i]);
                    duplicatedROI.MouseLeftButtonDown += ROI_MouseDown;

                    RemoveAdornerFromROI(duplicatedROI, AdornerType.ADJUST_ADORNER);
                    int[] tag = SetTagBit(duplicatedROI.Tag, Tag.FLAGS, Flag.ADJUST_ADONERS, false);
                    duplicatedROI.Tag = tag;

                    duplicatedROIs.Add(duplicatedROI);
                    UpdateGeometryAdorner(duplicatedROI);
                    UpdateIndexAdorner(duplicatedROI);
                    UpdateDividerAdorner(duplicatedROI);
                }
            }
            _skipRedrawCenter = skipRedrawCenterBackup;
        }

        /// <summary>
        /// get lists of shapes with the same mode, all if no pattern ID or wavelengthNM specified.
        /// </summary>
        /// <param name="mode"></param>
        /// <param name="patternID"></param>
        /// <param name="wavelengthNM"></param>
        /// <returns></returns>
        public List<Shape> GetModeROIs(Mode mode, int patternID = -1, int wavelengthNM = -1, int subID = -1)
        {
            List<Shape> roiList = new List<Shape>();

            for (int i = 0; i < _roiList.Count; i++)
            {
                if (mode == (Mode)(GetTagByteSection(_roiList[i].Tag, Tag.MODE, SecR)) &&
                    (0 > patternID || patternID == ((int[])_roiList[i].Tag)[(int)Tag.PATTERN_ID]) &&
                    (0 > wavelengthNM || wavelengthNM == ((int[])_roiList[i].Tag)[(int)Tag.WAVELENGTH_NM]) &&
                    (0 > subID || subID == ((int[])_roiList[i].Tag)[(int)Tag.SUB_PATTERN_ID]))
                {
                    roiList.Add(_roiList[i]);
                }
            }
            return roiList;
        }

        //Causing problems with UI bindings
        /*private Shape GetScaledROI(Shape roi)
        {
            Shape tmp;
            switch (roi.GetType().ToString())
            {
                case "OverlayManager.ROIEllipse":
                    tmp = new ROIEllipse((roi as ROIEllipse).StatsStartPoint, (roi as ROIEllipse).StatsEndPoint, 1, 1);
                    break;
                case "OverlayManager.ROICrosshair":
                    tmp = new ROICrosshair((roi as ROICrosshair).StatsCenterPoint, 1, 1);
                    break;
                case "OverlayManager.ROIRect":
                    tmp = new ROIRect((roi as ROIRect).StatsStartPoint, (roi as ROIRect).StatsEndPoint, 1, 1);
                    break;
                case "OverlayManager.ROIPoly":
                    tmp = new ROIPoly((roi as ROIPoly).StatsPoints, 1, 1);
                    break;
                case "OverlayManager.ROILine":
                    tmp = new ROILine((roi as ROILine).StatsStartPoint, (roi as ROILine).StatsEndPoint, 1, 1);
                    break;
                case "OverlayManager.ROIPolyline":
                    tmp = new ROIPolyline((roi as ROIPolyline).StatsPoints, 1, 1);
                    break;
                default:
                    tmp = new ROIRect();
                    break;
            }
            tmp.Tag = roi.Tag;
            return tmp;
        }*/
        /// <summary>
        /// return mode ROI sub pattern IDs.
        /// </summary>
        /// <returns></returns>
        public List<int> GetModeSubPatternIDs(Mode mode)
        {
            List<int> iList = new List<int>();
            List<Shape> roiList = GetModeROIs(mode);

            for (int i = 0; i < roiList.Count; i++)
            {
                iList.Add(((int[])roiList[i].Tag)[(int)Tag.SUB_PATTERN_ID]);
            }
            return iList;
        }

        /// <summary>
        /// Retrieve ROI centers from given shapes
        /// </summary>
        /// <param name="shapes"></param>
        /// <returns></returns>
        public List<Point> GetPatternROICenters(List<Shape> shapes)
        {
            List<Point> centers = new List<Point>();
            Point? pt = null;
            double left = double.MaxValue, right = 0, top = double.MaxValue, bottom = 0;
            for (int i = 0; i < shapes.Count; i++)
            {
                switch (shapes[i].GetType().ToString())
                {
                    case "OverlayManager.ROIEllipse":
                        pt = (shapes[i] as ROIEllipse).StatsCenterPoint;
                        break;
                    case "OverlayManager.ROICrosshair":
                        pt = (shapes[i] as ROICrosshair).StatsCenterPoint;
                        break;
                    case "OverlayManager.ROIRect":
                        pt = new Point((((OverlayManager.ROIRect)shapes[i]).StatsTopLeft.X + ((OverlayManager.ROIRect)shapes[i]).StatsBottomRight.X) / 2,
                            (((OverlayManager.ROIRect)shapes[i]).StatsTopLeft.Y + ((OverlayManager.ROIRect)shapes[i]).StatsBottomRight.Y) / 2);
                        break;
                    case "OverlayManager.ROIPoly":
                        for (int j = 0; j < ((ROIPoly)shapes[i]).StatsPoints.Count; j++)
                        {
                            left = Math.Min(left, ((ROIPoly)shapes[i]).StatsPoints[j].X);
                            right = Math.Max(right, ((ROIPoly)shapes[i]).StatsPoints[j].X);
                            top = Math.Min(top, ((ROIPoly)shapes[i]).StatsPoints[j].Y);
                            bottom = Math.Min(bottom, ((ROIPoly)shapes[i]).StatsPoints[j].Y);
                        }
                        pt = new Point((left + right) / 2, (top + bottom) / 2);
                        break;
                    case "Line":
                        pt = new Point((((Line)shapes[i]).X1 + ((Line)shapes[i]).X2) / 2, (((Line)shapes[i]).Y1 + ((Line)shapes[i]).Y2) / 2);
                        break;
                    case "OverlayManager.ROILine":
                        pt = new Point((((ROILine)shapes[i]).StatsStartPoint.X + ((ROILine)shapes[i]).StatsEndPoint.X) / 2, (((ROILine)shapes[i]).StatsStartPoint.Y + ((ROILine)shapes[i]).StatsEndPoint.Y) / 2);
                        break;
                    case "Polyline":
                        for (int j = 0; j < ((Polyline)shapes[i]).Points.Count; j++)
                        {
                            left = Math.Min(left, ((Polyline)shapes[i]).Points[j].X);
                            right = Math.Max(right, ((Polyline)shapes[i]).Points[j].X);
                            top = Math.Min(top, ((Polyline)shapes[i]).Points[j].Y);
                            bottom = Math.Min(bottom, ((Polyline)shapes[i]).Points[j].Y);
                        }
                        pt = new Point((left + right) / 2, (top + bottom) / 2);
                        break;
                    case "OverlayManager.ROIPolyline":
                        for (int j = 0; j < ((ROIPolyline)shapes[i]).Points.Count; j++)
                        {
                            left = Math.Min(left, ((ROIPolyline)shapes[i]).StatsPoints[j].X);
                            right = Math.Max(right, ((ROIPolyline)shapes[i]).StatsPoints[j].X);
                            top = Math.Min(top, ((ROIPolyline)shapes[i]).StatsPoints[j].Y);
                            bottom = Math.Min(bottom, ((ROIPolyline)shapes[i]).StatsPoints[j].Y);
                        }
                        pt = new Point((left + right) / 2, (top + bottom) / 2);
                        break;
                    default:
                        break;
                }
                if (null != pt)
                {
                    centers.Add((Point)pt);
                }
            }
            return centers;
        }

        public List<Point> GetPatternROICenters(int patternID, ref string roiType, ref Point centerPoint)
        {
            List<Point> centers = new List<Point>();
            Point? pt = null;
            roiType = string.Empty;
            double left = double.MaxValue, right = 0, top = double.MaxValue, bottom = 0;
            for (int i = 0; i < _roiList.Count; i++)
            {
                //avoid patternID == 0 for Mode.STATSONLY
                if (0 < patternID && patternID == ((int[])_roiList[i].Tag)[(int)Tag.PATTERN_ID])
                {
                    switch (_roiList[i].GetType().ToString())
                    {
                        case "OverlayManager.ROIEllipse":
                            pt = (_roiList[i] as ROIEllipse).StatsCenterPoint;
                            roiType = (string.IsNullOrEmpty(roiType) || 0 == roiType.CompareTo("Ellipse")) ? "Ellipse" : "mixed";
                            break;
                        case "OverlayManager.ROICrosshair":
                            pt = (_roiList[i] as ROICrosshair).StatsCenterPoint;
                            roiType = (string.IsNullOrEmpty(roiType) || 0 == roiType.CompareTo("Crosshair")) ? "Crosshair" : "mixed";
                            break;
                        case "OverlayManager.ROIRect":
                            pt = new Point((((OverlayManager.ROIRect)_roiList[i]).StatsTopLeft.X + ((OverlayManager.ROIRect)_roiList[i]).StatsBottomRight.X) / 2, (((OverlayManager.ROIRect)_roiList[i]).StatsTopLeft.Y + ((OverlayManager.ROIRect)_roiList[i]).StatsBottomRight.Y) / 2);
                            roiType = (string.IsNullOrEmpty(roiType) || 0 == roiType.CompareTo("Rectangle")) ? "Rectangle" : "mixed";
                            break;
                        case "OverlayManager.ROIPoly":
                            for (int j = 0; j < ((ROIPoly)_roiList[i]).StatsPoints.Count; j++)
                            {
                                left = Math.Min(left, ((ROIPoly)_roiList[i]).StatsPoints[j].X);
                                right = Math.Max(right, ((ROIPoly)_roiList[i]).StatsPoints[j].X);
                                top = Math.Min(top, ((ROIPoly)_roiList[i]).StatsPoints[j].Y);
                                bottom = Math.Min(bottom, ((ROIPoly)_roiList[i]).StatsPoints[j].Y);
                            }
                            pt = new Point((left + right) / 2, (top + bottom) / 2);
                            roiType = (string.IsNullOrEmpty(roiType) || 0 == roiType.CompareTo("Polygon")) ? "Polygon" : "mixed";
                            break;
                        case "Line":
                            pt = new Point((((Line)_roiList[i]).X1 + ((Line)_roiList[i]).X2) / 2, (((Line)_roiList[i]).Y1 + ((Line)_roiList[i]).Y2) / 2);
                            roiType = (string.IsNullOrEmpty(roiType) || 0 == roiType.CompareTo("Line")) ? "Line" : "mixed";
                            break;
                        case "Polyline":
                            for (int j = 0; j < ((Polyline)_roiList[i]).Points.Count; j++)
                            {
                                left = Math.Min(left, ((Polyline)_roiList[i]).Points[j].X);
                                right = Math.Max(right, ((Polyline)_roiList[i]).Points[j].X);
                                top = Math.Min(top, ((Polyline)_roiList[i]).Points[j].Y);
                                bottom = Math.Min(bottom, ((Polyline)_roiList[i]).Points[j].Y);
                            }
                            pt = new Point((left + right) / 2, (top + bottom) / 2);
                            roiType = (string.IsNullOrEmpty(roiType) || 0 == roiType.CompareTo("Polyline")) ? "Polyline" : "mixed";
                            break;
                        case "OverlayManager.ROILine":
                            pt = new Point((((ROILine)_roiList[i]).StatsStartPoint.X + ((ROILine)_roiList[i]).StatsEndPoint.X) / 2, (((ROILine)_roiList[i]).StatsStartPoint.Y + ((ROILine)_roiList[i]).StatsEndPoint.Y) / 2);
                            roiType = (string.IsNullOrEmpty(roiType) || 0 == roiType.CompareTo("Line")) ? "Line" : "mixed";
                            break;
                        case "OverlayManager.ROIPolyline":
                            for (int j = 0; j < ((ROIPolyline)_roiList[i]).Points.Count; j++)
                            {
                                left = Math.Min(left, ((ROIPolyline)_roiList[i]).StatsPoints[j].X);
                                right = Math.Max(right, ((ROIPolyline)_roiList[i]).StatsPoints[j].X);
                                top = Math.Min(top, ((ROIPolyline)_roiList[i]).StatsPoints[j].Y);
                                bottom = Math.Min(bottom, ((ROIPolyline)_roiList[i]).StatsPoints[j].Y);
                            }
                            pt = new Point((left + right) / 2, (top + bottom) / 2);
                            roiType = (string.IsNullOrEmpty(roiType) || 0 == roiType.CompareTo("Polyline")) ? "Polyline" : "mixed";
                            break;
                        default:
                            break;
                    }
                    if (null != pt)
                    {
                        if (0 == ((int[])_roiList[i].Tag)[(int)Tag.SUB_PATTERN_ID])
                        {
                            centerPoint = (Point)pt;
                        }
                        else
                        {
                            centers.Add((Point)pt);
                        }
                    }
                }
            }

            return centers;
        }

        //TODO update this
        /// <summary>
        /// Get patterns with z[um] as key, all if any attribute is not specified
        /// </summary>
        /// <param name="mode"></param>
        /// <param name="patternID"></param>
        /// <param name="wavelengthNM"></param>
        /// <param name="subID"></param>
        /// <returns></returns>
        public Dictionary<double, List<Shape>> GetPatternZROIs(int mode = -1, int patternID = -1, int wavelengthNM = -1, string subIDstr = "> 0")
        {
            Dictionary<double, List<Shape>> zShapes = new Dictionary<double, List<Shape>>();
            double zUM = 0.0;
            int[] limit = ParseLimit(subIDstr);

            for (int i = 0; i < _roiList.Count; i++)
            {
                if (0 > mode || (Mode)mode == (Mode)(GetTagByteSection(_roiList[i].Tag, Tag.MODE, SecR)) &&
                    //avoid patternID == 0 for Mode.STATSONLY and SUB_PATTERN_ID == 0 for pattern center
                    (0 > patternID || patternID == ((int[])_roiList[i].Tag)[(int)Tag.PATTERN_ID]) &&
                    (0 > wavelengthNM || wavelengthNM == ((int[])_roiList[i].Tag)[(int)Tag.WAVELENGTH_NM]) &&
                    (limit[0] <= ((int[])_roiList[i].Tag)[(int)Tag.SUB_PATTERN_ID] && limit[1] >= ((int[])_roiList[i].Tag)[(int)Tag.SUB_PATTERN_ID]))
                {
                    if (Double.TryParse(((int[])_roiList[i].Tag)[(int)Tag.Z_UM_INT].ToString() + "." + ((int[])_roiList[i].Tag)[(int)Tag.Z_UM_DEC].ToString(), out zUM))
                    {
                        //consider drifting of z value, keep on the same level if within tolerance
                        double foundZum = 0;
                        bool found = false;
                        foreach (KeyValuePair<double, List<Shape>> entry in zShapes)
                        {
                            if (TOLERANCE_ZUM > Math.Abs(entry.Key - zUM))
                            {
                                foundZum = entry.Key;
                                found = true;
                                break;
                            }
                        }
                        if (found)
                        {
                            zShapes[foundZum].Add(_roiList[i]);
                        }
                        else
                        {
                            List<Shape> shapeList = new List<Shape>();
                            zShapes.Add(zUM, shapeList);
                            zShapes[zUM].Add(_roiList[i]);
                        }
                    }
                }
            }
            return zShapes;
        }

        public void InitOverlayManagerClass(int pixelX, int pixelY, PixelSizeUM umPerPixel, bool saveMaskEverytime)
        {
            _pixelX = pixelX;
            _pixelY = pixelY;
            _pixelXAdjusted = pixelX;
            _pixelYAdjusted = pixelY;
            _umPerPixel = umPerPixel;
            _fieldWidth = Math.Round(pixelX * umPerPixel.PixelWidthUM);
            _fieldHeight = Math.Round(pixelY * umPerPixel.PixelHeightUM);
            //_umPerPixel = umPerPixel;
            _saveMaskEveryTime = saveMaskEverytime;
            _roiList = new List<Shape>();
            _roiListBackup = null;
            _scanAreaROIList = new List<Shape>();
            _roiSpec = new List<Shape>();
            _isObjectComplete = true;
            _isObjectCreated = false;
            _objectUpdated = false;
            _mask = new short[1];
            _roiCount = 0;
            _saveEveryChange = false;
            _expROIsPathAndName = "";
            _lastLineIndex = 0;
            _currentMode = Mode.STATSONLY;
            _patternSubIndexVisible = true;
            _visible = true;
            _save = true;
            _patternID = 0;
        }

        //Turn On or Off Reticle
        public void InitReticle(bool reticleOnOff)
        {
            InitROI();

            //Remove Any Reticle left in the canvas
            for (int i = 0; i < _roiList.Count; i++)
            {
                if (_roiList[i] is Reticle)
                {
                    DeleteSingleObject(i);
                }
            }

            //If reticle flag is on then create object
            if (true == reticleOnOff)
            {
                _activeType = typeof(Reticle);
                _isObjectCreated = false;
                _isObjectComplete = false;
                Point pt = new Point();
                if (null != UpdatingObjectEvent) UpdatingObjectEvent(true);
                CreateObject(pt);
            }
            UpdateVisibleROIS();
        }

        //set up to create ROI Rectangle (will create the ROI when there is a left click on the canvas)
        public void InitROICrosshair()
        {
            InitROI();
            _activeType = typeof(ROICrosshair);
            _isObjectCreated = false;
            _isObjectComplete = false;
            if (null != ObjectSizeChangedEvent) { ObjectSizeChangedEvent(0, 0); }
            if (null != UpdatingObjectEvent) UpdatingObjectEvent(true);
        }

        //set up to create ROI Ellipse (will create the ROI when there is a left click on the canvas)
        public void InitROIEllipse()
        {
            InitROI();
            _activeType = typeof(ROIEllipse);
            _isObjectCreated = false;
            _isObjectComplete = false;
            if (null != UpdatingObjectEvent) UpdatingObjectEvent(true);
        }

        //set up to create ROI Line (will create the ROI when there is a left click on the canvas)
        public void InitROILine()
        {
            InitROI();
            _activeType = typeof(ROILine);
            _isObjectCreated = false;
            _isObjectComplete = false;
            if (null != UpdatingObjectEvent) UpdatingObjectEvent(true);
        }

        //set up to create ROI Line (will create the ROI when there is a left click on the canvas)
        //Call this function when there is a double click
        public void InitROILineWithOptions()
        {
            LineROIOptions roiOptions = new LineROIOptions();
            roiOptions.Title = "Line Options";
            roiOptions.ShowLength = _showLineLength;
            roiOptions.ShowDialog();
            _showLineLength = roiOptions.ShowLength;
            InitROI();
            _activeType = typeof(ROILine);
            _isObjectCreated = false;
            _isObjectComplete = false;
            if (null != UpdatingObjectEvent) UpdatingObjectEvent(true);
        }

        //set up to create ROI Polygon (will create the ROI when there is a left click on the canvas)
        public void InitROIPoly()
        {
            InitROI();
            _activeType = typeof(ROIPoly);
            _isObjectCreated = false;
            _isObjectComplete = false;
            if (null != UpdatingObjectEvent) UpdatingObjectEvent(true);
        }

        //set up to create ROI Polygon (will create the ROI when there is a left click on the canvas)
        public void InitROIPolyline()
        {
            InitROI();
            _activeType = typeof(ROIPolyline);
            _isObjectCreated = false;
            _isObjectComplete = false;
            if (null != UpdatingObjectEvent) UpdatingObjectEvent(true);
        }

        //set up to create ROI Polygon (will create the ROI when there is a left click on the canvas)
        public void InitROIPolylineWithOptions()
        {
            LineROIOptions roiOptions = new LineROIOptions();
            roiOptions.Title = "Polyline Options";
            roiOptions.ShowLength = _showPolylineLength;
            roiOptions.ShowDialog();
            _showPolylineLength = roiOptions.ShowLength;
            _activeType = typeof(ROIPolyline);
            _isObjectCreated = false;
            _isObjectComplete = false;
            InitROI();
            if (null != UpdatingObjectEvent) UpdatingObjectEvent(true);
        }

        //set up to create ROI Rectangle (will create the ROI when there is a left click on the canvas)
        public void InitROIRect()
        {
            InitROI();
            _activeType = typeof(ROIRect);
            _isObjectCreated = false;
            _isObjectComplete = false;
            if (null != UpdatingObjectEvent) UpdatingObjectEvent(true);
        }

        //Turn On or Off Scale
        public void InitScale(bool scaleOnOff)
        {
            InitROI();

            //Remove Any Scale left in the canvas
            int j = -1, k = -1;
            for (int i = 0; i < _roiList.Count; i++)
            {
                if (_roiList[i] is Scale)
                {
                    if (j < 0)
                        j = i;
                    else
                        k = i;
                }
            }
            DeleteSingleObject(k);
            DeleteSingleObject(j);

            //If reticle flag is on then create object
            if (scaleOnOff)
            {
                _activeType = typeof(Scale);
                _isObjectCreated = false;
                _isObjectComplete = false;
                Point pt = new Point();
                if (null != UpdatingObjectEvent) UpdatingObjectEvent(true);
                CreateObject(pt);
            }
            UpdateVisibleROIs();
        }

        //Allows for selection of multiple ROIs after it is called
        public void InitSelectROI()
        {
            InitROI();
            _activeType = null;
            _isObjectCreated = false;
            _isObjectComplete = true;
            if (null != UpdatingObjectEvent) UpdatingObjectEvent(false);
        }

        public bool IsAdornerLayerValid()
        {
            //1 - If the overlay items collection is initialized before a reference to the CoumpoundImage is set in ImageView VM, the Visual Parent
            //Will be null and prevent any adorners from being drawn. This checks if that parent is null.
            //2 - If the overlay items are set in ImageView on image capture, the adorners might not be loaded because of the roi's being persisted on view load,
            //This method also checks if the first roi has any adorners which it should if it is valid.
            if (0 < _overlayItems?.Count)
            {
                DependencyObject parentVisual = VisualTreeHelper.GetParent(_overlayItems[0]);

                bool hasAdorners = false;
                _roiAdornerLayer = AdornerLayer.GetAdornerLayer(_overlayItems[0]);
                if (null != _roiAdornerLayer)
                {
                    Adorner[] itemAdorners = _roiAdornerLayer.GetAdorners(_overlayItems[0]);
                    hasAdorners = itemAdorners?.Length > 0;
                }

                return parentVisual != null && hasAdorners;
            }
            return false;
        }

        /// <summary>
        /// Returns if the input ROI is selected
        /// </summary>
        /// <param name="roi"></param>
        /// <returns></returns>
        public bool IsROISelected(Shape roi)
        {
            int[] tag = (int[])roi.Tag;

            if (true == GetTagBit(roi.Tag, Tag.FLAGS, Flag.ADJUST_ADONERS))
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        public bool LoadOriginalFieldAndROIs(ref int areaMode, ref int scanMode, ref int fieldSize, ref int offsetX, ref int offsetY, ref int pixelX, ref int pixelY, ref double areaAngle, ref double dwellTime, ref int interleaveScan)
        {
            ROICapsule roiCapsule = LoadXamlROIs();
            bool ret = false;
            try
            {
                if (roiCapsule != null)
                {
                    _roiList = new List<Shape>();
                    if (null != _overlayItems)
                    {
                        _overlayItems.Clear();
                    }
                    for (int i = 0; i < roiCapsule.ScanAreaROIs.Count(); i++)
                    {
                        if (null != roiCapsule.ScanAreaROIs[i])
                        {
                            int[] oldTag = (int[])roiCapsule.ScanAreaROIs[i].Tag;
                            areaMode = oldTag[0];
                            scanMode = oldTag[1];
                            fieldSize = oldTag[2];
                            offsetX = oldTag[3];
                            offsetY = oldTag[4];
                            pixelX = oldTag[5];
                            pixelY = oldTag[6];
                            areaAngle = ((double)oldTag[7]) / 100.0; //this number is multiplied by 100 before saving
                            dwellTime = ((double)oldTag[8]) / 10.0; //this number is multiplied by 10 before saving
                            interleaveScan = oldTag[9];
                            if (roiCapsule.ScanAreaROIs[i] is ROIPoly) //If the ROI is a Polygon recreate it and add it to the List of ROIs
                            {
                                ROIPoly poly = new ROIPoly()
                                {
                                    StrokeThickness = _roiStrokeThickness,
                                    UseRoundnessPercentage = true,
                                    Fill = Brushes.Transparent
                                };
                                int nPoints = (roiCapsule.ScanAreaROIs[i] as ROIPoly).Points.Count;
                                for (int j = 0; j < nPoints; j++)
                                {
                                    poly.Points.Add((roiCapsule.ScanAreaROIs[i] as ROIPoly).Points[j]);
                                }
                                poly.Closed = true;
                                _roiList.Add(poly);
                            }
                            else //if the ROI is NOT a polygon just add it to the List of ROIs
                            {
                                if (roiCapsule.ScanAreaROIs[i] is ROIPolyline)
                                    roiCapsule.ScanAreaROIs[i].Name = "Polyline" + (_roiList.Count + 1);
                                else if (roiCapsule.ScanAreaROIs[i] is ROILine)
                                    roiCapsule.ScanAreaROIs[i].Name = "Line" + (_roiList.Count + 1);
                                else if (roiCapsule.ScanAreaROIs[i] is ROIPoly)
                                    roiCapsule.ScanAreaROIs[i].Name = "Polygon" + (_roiList.Count + 1);
                                _roiList.Add(roiCapsule.ScanAreaROIs[i]);
                            }

                            //Use the ROICenter property to locate the Ellipse at its last location
                            //This is necessary because the Center property gets modified when redrawing the ROIEllipse
                            //Which happens when the ROIs are loaded.
                            if (roiCapsule.ScanAreaROIs[i] is ROIEllipse)
                            {
                                (_roiList[_roiList.Count - 1] as ROIEllipse).Center = (roiCapsule.ScanAreaROIs[i] as ROIEllipse).ROICenter;
                            }

                            if (false == roiCapsule.ScanAreaROIs[i] is Reticle || false == roiCapsule.ScanAreaROIs[i] is Scale)
                            {
                                //Only register to mouseDown event if ROI is not a Reticle or a scale
                                _roiList[i].MouseLeftButtonDown += ROI_MouseDown;
                            }

                            int[] newTag = DefaultTags(i, null);
                            newTag[(int)Tag.SUB_PATTERN_ID] = i + 1;
                            //add to canvas before add geometry:
                            if ((null != _overlayItems) && (true == GetTagBit(newTag, Tag.FLAGS, Flag.DISPLAY)))
                            {
                                _overlayItems.Add(_roiList[i]);
                            }

                            //Add properties to the last added item
                            _roiList[i].ToolTip = "ROI #" + newTag[(int)Tag.SUB_PATTERN_ID];
                            _roiList[i].Stroke = GetROIColor(i % 8, null);
                            _roiList[i].Tag = newTag;
                            _roiList[i].Tag = SetTagRGB(_roiList[i]);

                            //Carry Geometry adorner selection for line and polyline
                            if (_roiList[i] is Line || _roiList[i] is Polyline || _roiList[i] is ROILine || _roiList[i] is ROIPolyline)
                            {
                                if (10 <= oldTag.Length)
                                {
                                    if ((_showLineLength == true))
                                    {
                                        AddGeometryToROI(_roiList[i]);
                                    }
                                }
                            }

                            //Add index to ROI:
                            if (((int)Tag.LAST_TAG <= ((int[])_roiList[i].Tag).Length) && (null != _overlayItems))
                            {
                                AddIndexToROI(_roiList[i]);
                            }
                        }
                    }
                    _roiCount = _roiList.Count;
                    CreateMask();

                    ROIListUpdated?.Invoke();
                    InitSelectROI();

                    ret = true;
                }
            }
            catch (Exception e)
            {
                e.ToString();
                return false;
            }

            return ret;
        }

        //Load ROIs from file
        public void LoadROIs(string pathandFileName, ref bool reticleActive, ref bool scaleActive)
        {
            try
            {
                double loadedScaleX = 1;
                double loadedScaleY = 1;
                _roiList = new List<Shape>();
                _overlayItems = new ObservableCollection<UIElement>();
                ROICapsule roiCapsule = LoadXamlROIs(pathandFileName);

                _roiList = new List<Shape>();
                if (roiCapsule != null)
                {
                    if (null != _overlayItems)
                    {
                        _overlayItems.Clear();
                    }

                    if (null != roiCapsule.ROIspec)
                    {
                        _roiSpec = roiCapsule.ROIspec.ToList();
                    }

                    if (null != roiCapsule.ScanAreaROIs)
                    {
                        _scanAreaROIList = roiCapsule.ScanAreaROIs.ToList();
                    }

                    if (null != roiCapsule.ROIs)
                    {
                        for (int i = 0; i < roiCapsule.ROIs.Count(); i++)
                        {
                            if (null != roiCapsule.ROIs[i])
                            {
                                if (roiCapsule.ROIs[i] is ROIPoly) //If the ROI is a Polygonm recreate it and add it to the List of ROIs
                                {
                                    ROIPoly poly = new ROIPoly()
                                    {
                                        StrokeThickness = _roiStrokeThickness,
                                        UseRoundnessPercentage = true,
                                        Fill = Brushes.Transparent
                                    };
                                    int nPoints = (roiCapsule.ROIs[i] as ROIPoly).Points.Count;
                                    for (int j = 0; j < nPoints; j++)
                                    {
                                        poly.Points.Add((roiCapsule.ROIs[i] as ROIPoly).Points[j]);
                                    }
                                    poly.Closed = true;
                                    poly.Tag = roiCapsule.ROIs[i].Tag;
                                    _roiList.Add(poly);
                                }
                                else //if the ROI is NOT a polygon just add it to the List of ROIs
                                {
                                    _roiList.Add(roiCapsule.ROIs[i]);
                                }

                                //Use the ROICenter property to locate the Ellipse at its last location
                                //This is necessary because the Center property gets modified when redrawing the ROIEllipse
                                //Which happens when the ROIs are loaded.
                                if (roiCapsule.ROIs[i] is ROIEllipse)
                                {
                                    (_roiList[_roiList.Count - 1] as ROIEllipse).Center = (roiCapsule.ROIs[i] as ROIEllipse).ROICenter;
                                }

                                if (roiCapsule.ROIs[i] is Reticle)
                                {
                                    reticleActive = true;
                                }
                                else if (roiCapsule.ROIs[i] is Scale)
                                {
                                    scaleActive = true;
                                }
                                else
                                {
                                    //Only register to mouseDown event if ROI is not a Reticle or scale
                                    _roiList[i].MouseLeftButtonDown += ROI_MouseDown;
                                }

                                //Carry all tags except ROI index:
                                int[] tag = DefaultTags(i, _roiList[i].Tag);
                                //add to canvas before add geometry:
                                if ((null != _overlayItems) && (true == GetTagBit(tag, Tag.FLAGS, Flag.DISPLAY)))
                                {
                                    _overlayItems.Add(_roiList[i]);
                                }

                                if (_roiList[i] is Line)
                                {
                                    _lastLineIndex = i;
                                }

                                //Add properties to the last added item
                                UpdateToolTip(_roiList[i]);
                                _roiList[i].Tag = tag;
                                _roiList[i].Stroke = GetROIColor(i % 8, tag);

                                //Carry Geometry adorner selection for line and polyline
                                if (_roiList[i] is Line || _roiList[i] is Polyline || _roiList[i] is ROILine || _roiList[i] is ROIPolyline)
                                {
                                    if (4 <= ((int[])_roiList[i].Tag).Length)
                                    {
                                        if ((true == GetTagBit(tag, Tag.FLAGS, Flag.GEOMETRY_ADONERS)))
                                        {
                                            AddGeometryToROI(_roiList[i]);
                                        }
                                    }
                                }
                                //Add index to ROI:
                                if (((int)Tag.LAST_TAG <= ((int[])_roiList[i].Tag).Length))
                                {
                                    AddIndexToROI(_roiList[i]);
                                }
                            }
                        }
                        _roiCount = _roiList.Count;
                    }
                    foreach (Shape shape in _roiList)
                    {
                        if (shape is ScalableShape)
                        {
                            loadedScaleX = (shape as ScalableShape).ImageScaleX;
                            loadedScaleY = (shape as ScalableShape).ImageScaleY;
                            break;
                        }
                    }
                    if (loadedScaleX != _imageScaleX || loadedScaleY != _imageScaleY)
                    {
                        AdjustROIListAspectRatio(_imageScaleX, _imageScaleY);
                    }
                    _expROIsPathAndName = pathandFileName;
                    CreateMask();

                    ROIListUpdated?.Invoke();
                }
                LoadSettings();
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + "LoadROIs exception " + ex.Message);
            }
            _isObjectComplete = true;
            _roisLoaded = true;
        }

        //Collects the mouse events and calls the appropiate method
        public void MouseEvent(int meIndex, Point point, bool shiftDown)
        {
            switch ((MouseEventEnum)meIndex)
            {
                case MouseEventEnum.LEFTSINGLECLICK: // left single click
                    if (false == _isObjectCreated)
                    {
                        CreateObject(point);
                    }
                    else if (typeof(ROIPoly) == _activeType || typeof(Polyline) == _activeType || typeof(ROIPolyline) == _activeType)
                    {
                        AddPointToObject(point);
                    }
                    break;
                case MouseEventEnum.RIGHTSINGLECLICK: // right single click
                    break;
                case MouseEventEnum.LEFTDOUBLECLICK: // left double click
                    if (typeof(ROIPoly) == _activeType || typeof(Polyline) == _activeType || typeof(ROIPolyline) == _activeType)
                    {
                        CloseObject(point);
                        UpdateVisibleROIs();
                    }
                    break;
                case MouseEventEnum.RIGHTDOUBLECLICK: // right double click
                    break;
                case MouseEventEnum.LEFTHOLDING: // left holding and moving
                    ResizeObject(point, shiftDown);
                    break;
                case MouseEventEnum.RIGHTHOLDING: // right holding and moving
                    break;
                case MouseEventEnum.LEFTMOUSEUP: // left up
                    if ((typeof(ROILine) == _activeType || typeof(ROIRect) == _activeType || typeof(ROIEllipse) == _activeType || typeof(ROILine) == _activeType)
                        && true == _isObjectCreated && true == _objectUpdated)
                    {
                        ObjectComplete(_roiList.Count - 1);

                        //add index adorner:
                        AddIndexToROI(_roiList[_roiList.Count - 1]);

                        if (null != UpdatingObjectEvent) UpdatingObjectEvent(false);

                        if (typeof(Line) == _activeType)
                        {
                            InitROILine();
                        }
                        if (typeof(ROILine) == _activeType)
                        {
                            InitROILine();
                        }
                        else if (typeof(ROIRect) == _activeType)
                        {
                            InitROIRect();
                        }
                        else if (typeof(ROIEllipse) == _activeType)
                        {
                            CreateROICenter();
                            InitROIEllipse();
                        }
                        _objectUpdated = false;

                        ValidateROIs();
                        UpdateVisibleROIs();

                        if (_currentMode == Mode.MICRO_SCANAREA)
                        {
                            mROIsUpdated?.Invoke();
                        }
                    }
                    break;
                case MouseEventEnum.RIGHTMOUSEUP: // right up
                    break;
                default:
                    break;
            }
        }

        public void NewFieldFromROI(int areaMode, int scanMode, int fieldSize, int offsetX, int offsetY, int pixelX, int pixelY, double areaAngle, double dwellTime, int interleaveScan)
        {
            if (0 < _roiList.Count)
            {
                _scanAreaROIList = new List<Shape>();

                _roiList[_roiList.Count - 1].Name = "ScanAreaROI";
                _scanAreaROIList.Add(CloneUIElementByXamlWriter(_roiList[_roiList.Count - 1]));
                int angle = (int)(areaAngle * 100); //multiply angle by 100 to keep 2 decimal places of the angle
                int dtime = (int)(dwellTime * 10);
                int[] tag = { areaMode, scanMode, fieldSize, offsetX, offsetY, pixelX, pixelY, angle, dtime, interleaveScan, ((int[])_roiList[_roiList.Count - 1].Tag)[(int)Tag.FLAGS] };
                _scanAreaROIList[0].Tag = tag;
                ClearAllObjects();
                PersistSaveROIs();
            }
        }

        public void PersistLoadROIs(ref bool reticleActive, ref bool scaleActive)
        {
            string pathandName = Application.Current.Resources["TemplatesFolder"].ToString() + "\\ActiveROIs.xaml";
            LoadROIs(pathandName, ref reticleActive, ref scaleActive);
        }

        public void PersistSaveROIs(bool tabChange = false)
        {
            string pathandName = Application.Current.Resources["TemplatesFolder"].ToString() + "\\ActiveROIs.xaml";
            SaveROIs(pathandName);
            if(tabChange)
            { 
                _roisLoaded = false; 
            }
        }

        public bool QueryTheLastROIRange(ref List<Point> points, ref ROIType roiType)
        {
            if ((_roiCount > 0) && (_roiList.Count > 0))
            {
                points = new List<Point>();

                if (_roiList[_roiList.Count - 1] is ROIRect)
                {
                    ROIRect rect = _roiList[_roiList.Count - 1] as ROIRect;
                    points.Add(rect.StatsTopLeft);
                    points.Add(rect.StatsBottomRight);
                    roiType = ROIType.RECTANGLE;
                    return true;
                }
                else if (_roiList[_roiList.Count - 1] is Line)
                {
                    Line line = _roiList[_roiList.Count - 1] as Line;
                    Point p1 = new Point(line.X1, line.Y1);
                    Point p2 = new Point(line.X2, line.Y2);
                    points.Add(p1);
                    points.Add(p2);
                    roiType = ROIType.LINE;
                    return true;
                }
                else if (_roiList[_roiList.Count - 1] is Polyline)
                {
                    Polyline polyLine = _roiList[_roiList.Count - 1] as Polyline;
                    points = polyLine.Points.ToList();
                    roiType = ROIType.POLYLINE;
                    return true;
                }
                else if (_roiList[_roiList.Count - 1] is ROILine)
                {
                    ROILine line = _roiList[_roiList.Count - 1] as ROILine;
                    Point p1 = new Point(line.StatsStartPoint.X, line.StatsStartPoint.Y);
                    Point p2 = new Point(line.StatsEndPoint.X, line.StatsEndPoint.Y);
                    points.Add(p1);
                    points.Add(p2);
                    roiType = ROIType.LINE;
                    return true;
                }
                else if (_roiList[_roiList.Count - 1] is ROIPolyline)
                {
                    ROIPolyline polyLine = _roiList[_roiList.Count - 1] as ROIPolyline;
                    points = polyLine.StatsPoints.ToList();
                    roiType = ROIType.POLYLINE;
                    return true;
                }
            }
            return false;
        }

        public void ReloadPanningAdorners()
        {
            foreach (Shape overlayROI in _overlayItems)
            {
                UpdatePanningAdorner(overlayROI);
            }
        }

        public void ReorderROIs(Dictionary<int, int> oldAndNewROIPos)
        {
            var roiList = _roiList;
            _roiList = new List<Shape>();

            for (int i = 0; i < oldAndNewROIPos.Count; ++i)
            {
                if (oldAndNewROIPos.ContainsKey(i))
                {
                    if (oldAndNewROIPos[i] < roiList.Count)
                    {
                        _roiList.Add(roiList[oldAndNewROIPos[i]]);
                    }
                }
            }
            ReorderROIs();
            ValidateROIs();
        }

        /// <summary>
        /// replace current roi list with previous backup
        /// </summary>
        /// <param name="canvas"></param>
        public void RevokeROIs()
        {
            if (null != _roiListBackup)
            {
                _roiList = new List<Shape>();
                for (int i = 0; i < _roiListBackup.Count; i++)
                {
                    _roiList.Add(_roiListBackup[i]);
                }

                UpdateVisibleROIs();
            }
        }

        // save mask to raw file
        public void SaveMaskToPath(string pathAndName)
        {
            try
            {
                using (System.IO.BinaryWriter file = new System.IO.BinaryWriter(File.Open(pathAndName, FileMode.Create)))
                {
                    foreach (short item in _mask)
                    {
                        file.Write(item);
                    }
                }
            }
            catch (Exception e)
            {
                e.ToString();
            }
        }

        public void SaveROIs(string pathAndName, ROICapsule capsule = null)
        {
            if (true == _saveEveryChange)
            {
                _expROIsPathAndName = pathAndName;
            }

            try
            {
                using (FileStream f = new FileStream(pathAndName, FileMode.Create, FileAccess.Write))
                {
                    if (null != capsule)
                    {
                        XamlWriter.Save(capsule, f);
                    }
                    else
                    {
                        ROICapsule roiCapsule = new ROICapsule();
                        if (0 < _roiList.Count)
                        {
                            roiCapsule.ROIs = new Shape[_roiList.Count];
                            for (int i = 0; i < _roiList.Count; i++)
                            {
                                if (true == GetTagBit(_roiList[i].Tag, Tag.FLAGS, Flag.SAVE))
                                {
                                    _roiList[i].Tag = SetTagRGB(_roiList[i]);

                                    roiCapsule.ROIs[i] = _roiList[i];
                                }
                            }
                        }
                        if (0 < _scanAreaROIList.Count)
                        {
                            roiCapsule.ScanAreaROIs = new Shape[_scanAreaROIList.Count];
                            for (int i = 0; i < _scanAreaROIList.Count; i++)
                            {
                                roiCapsule.ScanAreaROIs[i] = _scanAreaROIList[i];
                            }
                        }
                        if (0 < _roiSpec.Count)
                        {
                            roiCapsule.ROIspec = new Shape[_roiSpec.Count];
                            for (int i = 0; i < _roiSpec.Count; i++)
                            {
                                roiCapsule.ROIspec[i] = _roiSpec[i];
                            }
                        }
                        XamlWriter.Save(roiCapsule, f);
                    }
                    f.Close();
                }
                PersistSettings();
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + "SaveROIs exception " + ex.Message);
            }
        }

        /// <summary>
        /// Scale ROIs to new boundary(pixels) based on scaleTo[PixelX, PixelY]
        /// </summary>
        /// <param name="pathandName"></param>
        /// <param name="scaleTo"></param>
        public bool ScaleROIs(string pathandName, int[] scaleTo)
        {
            try
            {
                ROICapsule roiCapsule = LoadXamlROIs(pathandName);
                if (null == roiCapsule)
                    return false;

                if (null == roiCapsule.ROIspec || null == roiCapsule.ROIspec[0])
                {
                    roiCapsule.ROIspec = new Shape[] { new Rectangle { Width = (int)Constants.DEFAULT_PIXEL_X, Height = (int)Constants.DEFAULT_PIXEL_X } };
                }
                if (null == scaleTo)
                {
                    scaleTo = new int[] { (int)((Rectangle)roiCapsule.ROIspec[0]).Width, (int)((Rectangle)roiCapsule.ROIspec[0]).Height };
                }
                if (null == roiCapsule.ROIs)
                    return false;

                for (int i = 0; i < roiCapsule.ROIs.Length; i++)
                {
                    if (null == roiCapsule.ROIs[i])
                        return false;

                    switch (roiCapsule.ROIs[i].GetType().ToString())
                    {
                        case "OverlayManager.ROIEllipse":
                            ((OverlayManager.ROIEllipse)(roiCapsule.ROIs[i])).Center =
                                new Point((((OverlayManager.ROIEllipse)(roiCapsule.ROIs[i])).ROICenter.X - ((Rectangle)roiCapsule.ROIspec[0]).Width / 2) * scaleTo[0] / ((Rectangle)roiCapsule.ROIspec[0]).Width + (scaleTo[0] / 2),
                                    (((OverlayManager.ROIEllipse)(roiCapsule.ROIs[i])).ROICenter.Y - ((Rectangle)roiCapsule.ROIspec[0]).Height / 2) * scaleTo[1] / ((Rectangle)roiCapsule.ROIspec[0]).Height + (scaleTo[1] / 2));
                            break;
                        case "OverlayManager.ROICrosshair":
                            ((OverlayManager.ROICrosshair)(roiCapsule.ROIs[i])).CenterPoint =
                                new Point((((OverlayManager.ROICrosshair)(roiCapsule.ROIs[i])).CenterPoint.X - ((Rectangle)roiCapsule.ROIspec[0]).Width / 2) * scaleTo[0] / ((Rectangle)roiCapsule.ROIspec[0]).Width + (scaleTo[0] / 2),
                                    (((OverlayManager.ROICrosshair)(roiCapsule.ROIs[i])).CenterPoint.Y - ((Rectangle)roiCapsule.ROIspec[0]).Height / 2) * scaleTo[1] / ((Rectangle)roiCapsule.ROIspec[0]).Height + (scaleTo[1] / 2));
                            break;
                    }
                }
                roiCapsule.ROIspec[0] = new Rectangle { Width = scaleTo[0], Height = scaleTo[1] };

                SaveROIs(pathandName, roiCapsule);
                return true;
            }
            catch (Exception ex)
            {
                ThorLogging.ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "ScaleROIs: " + ex.Message);
                return false;
            }
        }

        /// <summary>
        /// Scale ROIs from center of images by ratio in X and Y for given file
        /// </summary>
        /// <param name="pathandName"></param>
        /// <param name="scaleTo"></param>
        public bool ScaleROIsByRatio(string pathandName, double[] scaleRatio)
        {
            if (null == scaleRatio || (1.0 == scaleRatio[0] && 1.0 == scaleRatio[1]))
                return true;
            try
            {
                ROICapsule roiCapsule = LoadXamlROIs(pathandName);
                if (null == roiCapsule)
                    return false;

                if (null == roiCapsule.ROIspec || null == roiCapsule.ROIspec[0])
                {
                    roiCapsule.ROIspec = new Shape[] { new Rectangle { Width = (int)Constants.DEFAULT_PIXEL_X, Height = (int)Constants.DEFAULT_PIXEL_X } };
                }
                Point imgCenter = new Point(((Rectangle)roiCapsule.ROIspec[0]).Width / 2, ((Rectangle)roiCapsule.ROIspec[0]).Height / 2);
                for (int i = 0; i < roiCapsule.ROIs.Length; i++)
                {
                    if (null == roiCapsule.ROIs[i])
                        return false;

                    PivotScaleROI(roiCapsule.ROIs[i], imgCenter, scaleRatio);
                }

                SaveROIs(pathandName, roiCapsule);
                return true;
            }
            catch (Exception ex)
            {
                ThorLogging.ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "ScaleROIsByRatio: " + ex.Message);
                return false;
            }
        }

        /// <summary>
        /// Scale ROIs from point of interest (center of image) by ratio in X and Y
        /// </summary>
        /// <param name="pathandName"></param>
        /// <param name="scaleTo"></param>
        public bool PivotScaleROI(Shape roi, Point pivot, double[] scaleRatio, Mode[] patternMode = null)
        {
            if (null == scaleRatio || (1.0 == scaleRatio[0] && 1.0 == scaleRatio[1]))
                return true;
            try
            {
                //apply to all modes if null
                patternMode = (null == patternMode) ? Enum.GetValues(typeof(Mode)).Cast<Mode>().ToArray() : patternMode;

                if (null != roi && patternMode.Contains((Mode)(GetTagByteSection(roi.Tag, Tag.MODE, SecR))))
                {
                    //New (Px,Py) = (Px*Rx+Cx*(1-Rx), Py*Ry+Cy*(1-Ry))
                    switch (roi.GetType().ToString())
                    {
                        case "OverlayManager.ROIEllipse":
                            ((OverlayManager.ROIEllipse)(roi)).Center =
                                new Point((((OverlayManager.ROIEllipse)(roi)).ROICenter.X * scaleRatio[0] + pivot.X * (1.0 - scaleRatio[0])),
                                    ((OverlayManager.ROIEllipse)(roi)).ROICenter.Y * scaleRatio[1] + pivot.Y * (1.0 - scaleRatio[1]));
                            ((OverlayManager.ROIEllipse)(roi)).ROIWidth *= scaleRatio[0];
                            ((OverlayManager.ROIEllipse)(roi)).ROIHeight *= scaleRatio[1];
                            break;
                        case "OverlayManager.ROICrosshair":
                            ((OverlayManager.ROICrosshair)(roi)).CenterPoint =
                                new Point((((OverlayManager.ROICrosshair)(roi)).CenterPoint.X * scaleRatio[0] + pivot.X * (1.0 - scaleRatio[0])),
                                    ((OverlayManager.ROICrosshair)(roi)).CenterPoint.Y * scaleRatio[1] + pivot.Y * (1.0 - scaleRatio[1]));
                            break;
                        case "OverlayManager.ROIRect":
                            ((OverlayManager.ROIRect)(roi)).StartPoint =
                                new Point((((OverlayManager.ROIRect)(roi)).StartPoint.X * scaleRatio[0] + pivot.X * (1.0 - scaleRatio[0])),
                                    ((OverlayManager.ROIRect)(roi)).StartPoint.Y * scaleRatio[1] + pivot.Y * (1.0 - scaleRatio[1]));
                            ((OverlayManager.ROIRect)(roi)).EndPoint =
                               new Point((((OverlayManager.ROIRect)(roi)).EndPoint.X * scaleRatio[0] + pivot.X * (1.0 - scaleRatio[0])),
                                   ((OverlayManager.ROIRect)(roi)).EndPoint.Y * scaleRatio[1] + pivot.Y * (1.0 - scaleRatio[1]));
                            break;
                        case "OverlayManager.ROIPoly":
                            for (int i = 0; i < ((OverlayManager.ROIPoly)(roi)).Points.Count; i++)
                            {
                                ((OverlayManager.ROIPoly)(roi)).Points[i] = new Point((((OverlayManager.ROIPoly)(roi)).Points[i].X * scaleRatio[0] + pivot.X * (1.0 - scaleRatio[0])),
                                    (((OverlayManager.ROIPoly)(roi)).Points[i].Y * scaleRatio[1] + pivot.Y * (1.0 - scaleRatio[1])));
                            }
                            break;
                        case "OverlayManager.ROILine":
                            ((OverlayManager.ROILine)(roi)).StartPoint =
                                new Point((((OverlayManager.ROILine)(roi)).StartPoint.X * scaleRatio[0] + pivot.X * (1.0 - scaleRatio[0])),
                                ((OverlayManager.ROILine)(roi)).StartPoint.Y * scaleRatio[1] + pivot.Y * (1.0 - scaleRatio[1]));
                            ((OverlayManager.ROILine)(roi)).EndPoint =
                                new Point((((OverlayManager.ROILine)(roi)).EndPoint.X * scaleRatio[0] + pivot.X * (1.0 - scaleRatio[0])),
                                ((OverlayManager.ROILine)(roi)).EndPoint.Y * scaleRatio[1] + pivot.Y * (1.0 - scaleRatio[1]));
                            break;
                        case "OverlayManager.ROIPolyline":
                            for (int i = 0; i < ((OverlayManager.ROIPolyline)(roi)).Points.Count; i++)
                            {
                                ((OverlayManager.ROIPolyline)(roi)).Points[i] = new Point((((OverlayManager.ROIPolyline)(roi)).Points[i].X * scaleRatio[0] + pivot.X * (1.0 - scaleRatio[0])),
                                    (((OverlayManager.ROIPolyline)(roi)).Points[i].Y * scaleRatio[1] + pivot.Y * (1.0 - scaleRatio[1])));
                            }
                            break;
                    }
                }
                return true;
            }
            catch (Exception ex)
            {
                ThorLogging.ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "ScaleROIsByRatio: " + ex.Message);
                return false;
            }
        }

        public void SelectAllROIs()
        {
            if (null == _roiList || 0 == _roiList.Count)
            {
                return;
            }
            if (true == _isObjectComplete)
            {
                for (int i = 0; i < _roiList.Count; i++)
                {
                    if (false == (_roiList[i] is Reticle) || false == (_roiList[i] is Scale))
                    {
                        //continue to next ROI if this ROI is already selected
                        if (true == GetTagBit(_roiList[i].Tag, Tag.FLAGS, Flag.ADJUST_ADONERS))
                        {
                            continue;
                        }
                        AddPanningAdorners(_roiList[i]); // add adorner to selected ROIs
                        int[] tag = SetTagBit(_roiList[i].Tag, Tag.FLAGS, Flag.ADJUST_ADONERS, true); //mark as selected
                        _roiList[i].Tag = tag;
                    }
                }
            }
        }

        /// <summary>
        /// Selects the next ROI after the first ROI currently selected
        /// </summary>
        public void SelectNextROI()
        {
            int nextIndex = GetIndexOfNextROI(GetSelectedIndex(-1));

            DeselectAllROIs();

            SelectROIIndexWithErrorChecking(nextIndex);
        }

        /// <summary>
        /// Selects the previous ROI after the first ROI currently selected
        /// </summary>
        public void SelectPrevROI()
        {
            int nextIndex = GetIndexOfPrevROI(GetSelectedIndex(1));

            DeselectAllROIs();

            SelectROIIndexWithErrorChecking(nextIndex);
        }

        public void SelectSingleROI(Shape roi)
        {
            if (null == roi)
            {
                return;
            }
            DeselectAllROIs();
            if (true == _isObjectComplete)
            {
                if (false == (roi is Reticle) || false == (roi is Scale))
                {
                    //continue to next ROI if this ROI is already selected
                    if (true == GetTagBit(roi.Tag, Tag.FLAGS, Flag.ADJUST_ADONERS))
                    {
                        return;
                    }
                    if (!IsROISelected(roi))
                    {
                        AddPanningAdorners(roi); // add adorner to selected ROIs
                        int[] tag = SetTagBit(roi.Tag, Tag.FLAGS, Flag.ADJUST_ADONERS, true); //mark as selected
                        roi.Tag = tag;
                    }
                }
            }
        }

        public void SetOverlayItems(ObservableCollection<UIElement> overlayItems, bool duplicateROIList)
        {
            bool doExcecute = false;
            if (_overlayItems != overlayItems)
            {
                doExcecute = true;
            }
            else
            {
                doExcecute = true;
                for (int i = 0; i < _overlayItems.Count; i++)
                {
                    if (_overlayItems[i] is Shape shape)
                    {
                        for (int j = 0; j < _roiList.Count; j++)
                        {
                            if (_roiList[j] == shape)
                            {
                                doExcecute = false;
                            }
                        }
                    }
                }
            }
            if (doExcecute)
            {
                if (_overlayItems?.Count > 0 && _roiList?.Count > 0)
                {
                    if (duplicateROIList)
                    {
                        //find selected ROIs and keep a list to redraw the panning adorners
                        List<int> selectedROIs = new List<int>();
                        for (int i = 0; i < _roiList.Count; ++i)
                        {
                            if (IsROISelected(_roiList[i]))
                            {
                                selectedROIs.Add(i);
                            }
                        }

                        _roiList.Clear();

                        //add the ROIs from the
                        for (int i = 0; i < overlayItems.Count; i++)
                        {
                            //add Shape overlat items into ROILists
                            if (overlayItems[i] is Shape shape)
                            {
                                _roiList.Add(shape);
                            }
                        }

                        //make the old list reference have duplicated items off of the currently selected list
                        GetDuplicatedROIList(ref _overlayItems);

                        _overlayItems = overlayItems;
                    }
                    else
                    {
                        _overlayItems = overlayItems;
                        _roiList.Clear();

                        for (int i = 0; i < _overlayItems.Count; i++)
                        {
                            if (_overlayItems[i] is Shape shape)
                            {
                                _roiList.Add(shape);
                                UpdatePanningAdorner(shape);
                            }
                        }
                    }
                }
                else
                {
                    _overlayItems = overlayItems;

                }
            }
        }

        public void SetPatternToSaveROI(int patternID, Mode pMode = Mode.PATTERN_NOSTATS)
        {
            for (int i = 0; i < _roiList.Count; i++)
            {
                if ((pMode == (Mode)(GetTagByteSection(_roiList[i].Tag, Tag.MODE, SecR))) &&
                    (0 > patternID || patternID == ((int[])_roiList[i].Tag)[(int)Tag.PATTERN_ID]))
                {
                    int[] tag = SetTagBit(_roiList[i].Tag, Tag.FLAGS, Flag.SAVE, true);
                    _roiList[i].Tag = tag;
                }
            }
        }

        public void UpdateAdorners(Shape roi)
        {
            UpdateGeometryAdorner(roi);
            UpdateIndexAdorner(roi);
        }

        public void UpdateAspectRatio(double xScale, double yScale)
        {
            if (_pixelXAdjusted == (int)(_pixelX * xScale) && _pixelYAdjusted == (int)(_pixelY * yScale))
            {
                //no change in scale so no need to adjust ROI's
                return;
            }
            _imageScaleX = xScale;
            _imageScaleY = yScale;

            AdornerProvider.ImageScaleX = xScale;
            AdornerProvider.ImageScaleY = yScale;
            GeometryAdornerProvider.ImageScaleX = xScale;
            GeometryAdornerProvider.ImageScaleY = yScale;
            AdjustROIListAspectRatio(xScale, yScale);
            CreateMask();
        }

        public void UpdateParams(int pixelX, int pixelY, PixelSizeUM umPerPixel)
        {
            AdornerProvider.ImageWidth = pixelX;
            AdornerProvider.ImageHeight = pixelY;
            AdornerProvider.ZoomLevel = _zoomLevel;
            _umPerPixel = umPerPixel;
            GeometryAdornerProvider.ImageWidth = pixelX;
            GeometryAdornerProvider.ImageHeight = pixelY;
            if (!GeometryAdornerProvider.UMPerPixel.Compare(umPerPixel) || _pixelX != pixelX || _pixelY != pixelY)
            {
                if (!GeometryAdornerProvider.UMPerPixel.Compare(umPerPixel))
                {
                    _umPerPixel = umPerPixel;
                    _fieldWidth = Math.Round(pixelX * umPerPixel.PixelWidthUM);
                    _fieldHeight = Math.Round(pixelY * umPerPixel.PixelHeightUM);
                    GeometryAdornerProvider.UMPerPixel = umPerPixel;
                    UpdateGeometryAdorners();
                }

                if (_pixelX != pixelX || _pixelY != pixelY)
                {
                    _pixelX = pixelX;
                    _pixelY = pixelY;
                    _fieldWidth = Math.Round(pixelX * umPerPixel.PixelWidthUM);
                    _fieldHeight = Math.Round(pixelY * umPerPixel.PixelHeightUM);
                    if (0 < _pixelX && 0 < _pixelY && 0 < _roiList.Count)
                    {
                        _pixelX = pixelX;
                        _pixelY = pixelY;
                        _fieldWidth = Math.Round(pixelX * umPerPixel.PixelWidthUM);
                        _fieldHeight = Math.Round(pixelY * umPerPixel.PixelHeightUM);
                        CreateMask();
                    }

                }
                ParamsUpdatedEvent?.Invoke();
            }
        }

        public void UpdatePatternROIColor(Mode pMode = Mode.PATTERN_NOSTATS)
        {
            for (int i = 0; i < _roiList.Count; i++)
            {
                if ((pMode == _currentMode) && (_patternID == ((int[])_roiList[i].Tag)[(int)Tag.PATTERN_ID]))
                {
                    //update pattern roi color tag:
                    int[] tag = (int[])_roiList[i].Tag;
                    tag[(int)Tag.RGB] = _colorRGB;
                    _roiList[i].Tag = tag;

                    //update pattern roi color:
                    _bitVec32 = new BitVector32(_colorRGB);
                    _roiList[i].Stroke = new SolidColorBrush(Color.FromArgb((_wavelengthNM == ((int[])_roiList[i].Tag)[(int)Tag.WAVELENGTH_NM]) ? Byte.MaxValue : ATTENUATE_VALUE,
                        Convert.ToByte(_bitVec32[SecR]), Convert.ToByte(_bitVec32[SecG]), Convert.ToByte(_bitVec32[SecB])));
                }
            }

            UpdateVisibleROIs();
        }

        public void UpdatePatternROISize(double widthPx, double heightPx)
        {
            List<Shape> roiList = new List<Shape>();

            for (int i = 0; i < _roiList.Count; i++)
            {
                if ((Mode.PATTERN_NOSTATS == _currentMode) && (_patternID == ((int[])_roiList[i].Tag)[(int)Tag.PATTERN_ID]) && (typeof(ROIEllipse) == _roiList[i].GetType()))
                {
                    //determine new start and end points, each can be top-left or bottom-right corner:
                    double startX = (((ROIEllipse)_roiList[i]).StartPoint.X > ((ROIEllipse)_roiList[i]).EndPoint.X) ?
                        (((ROIEllipse)_roiList[i]).Center.X + (widthPx / 2)) : (((ROIEllipse)_roiList[i]).Center.X - (widthPx / 2));
                    double startY = (((ROIEllipse)_roiList[i]).StartPoint.Y > ((ROIEllipse)_roiList[i]).EndPoint.Y) ?
                        (((ROIEllipse)_roiList[i]).Center.Y + (heightPx / 2)) : (((ROIEllipse)_roiList[i]).Center.Y - (heightPx / 2));
                    double endX = (((ROIEllipse)_roiList[i]).StartPoint.X > ((ROIEllipse)_roiList[i]).EndPoint.X) ?
                        (((ROIEllipse)_roiList[i]).Center.X - (widthPx / 2)) : (((ROIEllipse)_roiList[i]).Center.X + (widthPx / 2));
                    double endY = (((ROIEllipse)_roiList[i]).StartPoint.Y > ((ROIEllipse)_roiList[i]).EndPoint.Y) ?
                        (((ROIEllipse)_roiList[i]).Center.Y - (heightPx / 2)) : (((ROIEllipse)_roiList[i]).Center.Y + (heightPx / 2));

                    //replace with new ellipse,
                    //since modifying size won't work in shiftdown case:
                    ROIEllipse roiReplace = new ROIEllipse(new Point(startX, startY), new Point(endX, endY), _imageScaleX, _imageScaleY);
                    roiReplace.Center = ((ROIEllipse)_roiList[i]).Center;
                    roiReplace.Fill = ((ROIEllipse)_roiList[i]).Fill;
                    roiReplace.Stroke = ((ROIEllipse)_roiList[i]).Stroke;
                    roiReplace.StrokeThickness = ((ROIEllipse)_roiList[i]).StrokeThickness;
                    roiReplace.MouseLeftButtonDown += ROI_MouseDown;
                    roiReplace.Tag = SetTagRGB(_roiList[i]);
                    roiList.Add(roiReplace);
                }
                else
                {
                    roiList.Add(_roiList[i]);
                }
            }
            _roiList.Clear();
            _roiList = roiList;
            UpdateVisibleROIs();
        }

        public void UpdateVisibleROIS()
        {
            ROIListUpdated?.Invoke();
        }

        public void UserLoadROIs(string pathAndName)
        {
            bool reticleActive = false;
            bool scaleActive = false;
            LoadROIs(pathAndName, ref reticleActive, ref scaleActive);
        }

        /// <summary>
        /// Validate drawn ROI
        /// </summary>
        /// <param name="roi"></param>
        public void ValidateROIs()
        {
            List<Shape> roiList = new List<Shape>();        //all final rois
            double top = Int32.MaxValue, left = Int32.MaxValue;
            double bottom = 0, right = 0;
            bool triggerMessageToUser = false;

            for (int i = 0; i < _roiList.Count; i++)
            {
                switch ((Mode)(GetTagByteSection(_roiList[i].Tag, Tag.MODE, SecR)))
                {
                    case Mode.MICRO_SCANAREA:
                        if (typeof(ROIRect) == _roiList[i].GetType())
                        {
                            //replace with new rectangle
                            double width = 0;
                            if (_pixelUnitSizeXY[0] >= ((ROIRect)_roiList[i]).ROIWidth)
                            {
                                width = _pixelUnitSizeXY[0];
                            }
                            else
                            {
                                int multiPlier = (int)Math.Round(((ROIRect)_roiList[i]).ROIWidth / _pixelUnitSizeXY[0]);
                                width = multiPlier * _pixelUnitSizeXY[0];
                            }

                            double height = 0;

                            if (_pixelUnitSizeXY[1] >= ((ROIRect)_roiList[i]).ROIHeight)
                            {
                                height = _pixelUnitSizeXY[1];
                            }
                            else
                            {
                                int multiPlier = (int)Math.Round(((ROIRect)_roiList[i]).ROIHeight / _pixelUnitSizeXY[1]);
                                height = multiPlier * _pixelUnitSizeXY[1];
                            }

                            bool safeToAddReplacement = true;
                            double replaceStartX = ((ROIRect)_roiList[i]).StartPoint.X;
                            double replaceStartY = ((ROIRect)_roiList[i]).StartPoint.Y;
                            int numStripes = (int)Math.Round(((ROIRect)_roiList[i]).ROIWidth / _pixelUnitSizeXY[0]);

                            if (width > _pixelX || height > _pixelY)
                            {
                                if (numStripes > 1 && _pixelUnitSizeXY[0] < _pixelX)
                                {
                                    //Try to reduce the number of stripes
                                    double newWidth = width;
                                    int numToDecrease = 0;
                                    while (newWidth > _pixelX)
                                    {
                                        numToDecrease++;
                                        newWidth = _pixelUnitSizeXY[0] * (numStripes - numToDecrease);
                                    }
                                    numStripes = numStripes - numToDecrease;
                                    width = newWidth;
                                    if (width <= 0)
                                    {
                                        safeToAddReplacement = false;
                                    }
                                }
                                else
                                {
                                    safeToAddReplacement = false;
                                    //don't add this roi back to the list. Alert user
                                }
                            }
                            else if (replaceStartX + width > _pixelX || replaceStartY + height > _pixelY)
                            {
                                double xShift = replaceStartX + width - _pixelX;
                                double yShift = replaceStartY + height - _pixelY;
                                xShift = xShift > 0 ? xShift : 0;
                                yShift = yShift > 0 ? yShift : 0;

                                replaceStartX = replaceStartX - xShift;
                                replaceStartY = replaceStartY - yShift;
                            }
                            if (safeToAddReplacement)
                            {
                                ROIRect roiReplace = new ROIRect(new Point(replaceStartX, replaceStartY), new Point(replaceStartX + width, replaceStartY + height), ((ROIRect)_roiList[i]).ImageScaleX, ((ROIRect)_roiList[i]).ImageScaleY);
                                roiReplace.Fill = ((ROIRect)_roiList[i]).Fill;
                                roiReplace.Stroke = ((ROIRect)_roiList[i]).Stroke;
                                roiReplace.StrokeThickness = ((ROIRect)_roiList[i]).StrokeThickness;
                                roiReplace.MouseLeftButtonDown += ROI_MouseDown;
                                roiReplace.Tag = SetTagRGB(_roiList[i]);
                                roiReplace.Tag = SetTagStripeCount(_roiList[i], numStripes);
                                roiList.Add(roiReplace);
                            }
                            else {
                                triggerMessageToUser = true;
                            }
                        }
                        else
                        {
                            roiList.Add(_roiList[i]);
                        }
                        break;
                    case Mode.PATTERN_NOSTATS:
                        //determine range for roi center, skip zeroth if not auto draw center
                        if (_patternID == ((int[])_roiList[i].Tag)[(int)Tag.PATTERN_ID])
                        {
                            if (0 != ((int[])_roiList[i].Tag)[(int)Tag.SUB_PATTERN_ID] || SkipRedrawCenter)
                            {
                                if (typeof(ROIEllipse) == _roiList[i].GetType())
                                {
                                    if (((ROIEllipse)_roiList[i]).TopLeft.X < left)
                                        left = ((ROIEllipse)_roiList[i]).TopLeft.X;
                                    if (((ROIEllipse)_roiList[i]).TopLeft.Y < top)
                                        top = ((ROIEllipse)_roiList[i]).TopLeft.Y;
                                    if (((ROIEllipse)_roiList[i]).BottomRight.X > right)
                                        right = ((ROIEllipse)_roiList[i]).BottomRight.X;
                                    if (((ROIEllipse)_roiList[i]).BottomRight.Y > bottom)
                                        bottom = ((ROIEllipse)_roiList[i]).BottomRight.Y;
                                }
                                else if (typeof(ROICrosshair) == _roiList[i].GetType())
                                {
                                    if (((ROICrosshair)_roiList[i]).CenterPoint.X < left)
                                        left = ((ROICrosshair)_roiList[i]).CenterPoint.X;
                                    if (((ROICrosshair)_roiList[i]).CenterPoint.Y < top)
                                        top = ((ROICrosshair)_roiList[i]).CenterPoint.Y;
                                    if (((ROICrosshair)_roiList[i]).CenterPoint.X > right)
                                        right = ((ROICrosshair)_roiList[i]).CenterPoint.X;
                                    if (((ROICrosshair)_roiList[i]).CenterPoint.Y > bottom)
                                        bottom = ((ROICrosshair)_roiList[i]).CenterPoint.Y;
                                }
                                roiList.Add(_roiList[i]);
                            }
                        }
                        else
                        {
                            roiList.Add(_roiList[i]);
                        }
                        break;
                    case Mode.PATTERN_WIDEFIELD:
                    case Mode.STATSONLY:
                    case Mode.LAST_MODE:
                        roiList.Add(_roiList[i]);
                        break;
                    default:
                        break;
                }
            }
            if (triggerMessageToUser)
            {
                mROIDeletedEvent?.Invoke();
            }
            //replace center shape when necessary
            if (top < Int32.MaxValue && !SkipRedrawCenter)
            {
                Shape roiRef = null;
                //locate SUB_PATTERN_ID == 1 for reference:
                for (int i = 0; i < _roiList.Count; i++)
                {
                    if ((_patternID == ((int[])_roiList[i].Tag)[(int)Tag.PATTERN_ID]) && (1 == ((int[])_roiList[i].Tag)[(int)Tag.SUB_PATTERN_ID]))
                    {
                        roiRef = _roiList[i];
                        break;
                    }
                }
                if (null != roiRef)
                {
                    for (int i = 0; i < _roiList.Count; i++)
                    {
                        if ((_patternID == ((int[])_roiList[i].Tag)[(int)Tag.PATTERN_ID]) && (0 == ((int[])_roiList[i].Tag)[(int)Tag.SUB_PATTERN_ID]))
                        {
                            Point newCenter = new Point((left + right) / 2, (top + bottom) / 2);
                            if (typeof(ROICrosshair) == _roiList[i].GetType())
                            {
                                Shape roiReplace = new ROICrosshair(newCenter, _imageScaleX, _imageScaleY);
                                roiReplace.Fill = ((ROICrosshair)_roiList[i]).Fill;
                                roiReplace.Stroke = ((ROICrosshair)_roiList[i]).Stroke;
                                roiReplace.StrokeThickness = ((ROICrosshair)_roiList[i]).StrokeThickness;
                                roiReplace.ToolTip = "Center";
                                roiReplace.MouseLeftButtonDown += ROI_MouseDown;
                                roiReplace.Tag = SetTagRGB(_roiList[i]);
                                roiList.Add(roiReplace);
                            }
                            else if (typeof(ROIEllipse) == _roiList[i].GetType())
                            {
                                Shape roiReplace = new ROIEllipse(
                                    new Point(newCenter.X - (roiRef as ROIEllipse).ROIWidth / 2, newCenter.Y - (roiRef as ROIEllipse).ROIHeight / 2),
                                    new Point(newCenter.X + (roiRef as ROIEllipse).ROIWidth / 2, newCenter.Y + (roiRef as ROIEllipse).ROIHeight / 2), _imageScaleX, _imageScaleY);
                                roiReplace.Fill = ((ROIEllipse)_roiList[i]).Fill;
                                roiReplace.Stroke = ((ROIEllipse)_roiList[i]).Stroke;
                                roiReplace.StrokeThickness = ((ROIEllipse)_roiList[i]).StrokeThickness;
                                roiReplace.ToolTip = "Center";
                                roiReplace.MouseLeftButtonDown += ROI_MouseDown;
                                roiReplace.Tag = SetTagRGB(_roiList[i]);
                                roiList.Add(roiReplace);
                            }
                        }
                    }
                }
            }
            _roiList.Clear();
            _roiList = roiList;
        }

        [DllImport("StatsManager.dll", EntryPoint = "SetLineProfileLine")]
        private static extern int SetLineProfileLine(int[] px, int[] py, int numberOfPoints, int lineIsActive);

        [DllImport("StatsManager.dll", EntryPoint = "SetStatsMask")]
        private static extern int SetStatsMask(short[] mask, int imgWidth, int imgHeight);

        /// <summary>
        /// Wraps an index to fit within an array bounds. If equal or above length, will wrap to 0, and if below 0, will wrap to length - 1
        /// </summary>
        /// <param name="index"> Index to wrap </param>
        /// <param name="length"> Length of array </param>
        /// <returns> The wrapped index </returns>
        private static int WrapIndex(int index, int length)
        {
            if (index >= length)
            {
                index = 0;
            }
            else if (index < 0)
            {
                index = length - 1;
            }

            return index;
        }

        private void AddDividerAdorner(Shape roi)
        {
            try
            {
                if (roi is ROIRect)
                {
                    _roiAdornerLayer = AdornerLayer.GetAdornerLayer(roi);
                    if (_roiAdornerLayer != null)
                    {
                        DividerAdornerProvider dividerAdornerProvider = new DividerAdornerProvider(roi, roi.Stroke, _pixelX, _pixelY);
                        _roiAdornerLayer.Add(dividerAdornerProvider);
                    }
                }
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "OverlayManagerClass.cs AddDividerAdorner failed, exception: " + ex.Message);
            }
        }

        private void AddGeometryToROI(Shape roi)
        {
            try
            {
                if (roi is Line || roi is Polyline || roi is ROILine || roi is ROIPolyline)
                {
                    _roiAdornerLayer = AdornerLayer.GetAdornerLayer(roi);
                    GeometryAdornerProvider LabelAdornerProvider = new GeometryAdornerProvider(roi, roi.Stroke, _pixelX, _pixelY, _umPerPixel);
                    _roiAdornerLayer.Add(LabelAdornerProvider);
                    int[] tag = SetTagBit(roi.Tag, Tag.FLAGS, Flag.GEOMETRY_ADONERS, true);
                    roi.Tag = tag;
                }
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "OverlayManagerClass.cs AddGeometryToROI failed, exception: " + ex.Message);
            }
        }

        private void AddIndexToROI(Shape roi)
        {
            if (true == GetTagBit(roi.Tag, Tag.FLAGS, Flag.SUB_PATTERN_INDEX_ADONERS))
            {
                _roiAdornerLayer = AdornerLayer.GetAdornerLayer(roi);
                if (null != _roiAdornerLayer)
                {
                    IndexAdornerProvider IndexAdornerProvider = new IndexAdornerProvider(roi, roi.Stroke, _pixelX, _pixelY);
                    IndexAdornerProvider.Index = ((int[])roi.Tag)[(int)Tag.SUB_PATTERN_ID];
                    _bitVec32 = new BitVector32(((int[])roi.Tag)[(int)Tag.RGB]);
                    IndexAdornerProvider.ForeGround = new SolidColorBrush(Color.FromArgb(Byte.MaxValue, Convert.ToByte(_bitVec32[SecR]), Convert.ToByte(_bitVec32[SecG]), Convert.ToByte(_bitVec32[SecB])));
                    _roiAdornerLayer.Add(IndexAdornerProvider);
                }
            }
        }

        private void AddPanningAdorners(Shape roi)
        {
            try
            {
                if (null == roi || false == _isObjectComplete)
                {
                    return;
                }
                if (typeof(Line) == roi.GetType() || typeof(ROIRect) == roi.GetType() ||
                    typeof(ROIPoly) == roi.GetType() || typeof(ROICrosshair) == roi.GetType() ||
                    typeof(Reticle) == roi.GetType() || typeof(Polyline) == roi.GetType() ||
                    typeof(ROIEllipse) == roi.GetType() || typeof(ROILine) == roi.GetType()
                    || typeof(ROIPolyline) == roi.GetType())
                {
                    _roiAdornerLayer = AdornerLayer.GetAdornerLayer(roi);

                    _adornerProvider = new AdornerProvider(roi, _pixelX, _pixelY);
                    if (_adornerProvider == null || _roiAdornerLayer == null)
                    {
                        return;
                    }
                    _adornerProvider.UpdateLinePosition += _adornerProvider_UpdateLinePosition;
                    _adornerProvider.UpdateNow += _adornerProvider_UpdateNow;
                    _adornerProvider.RectangleAdornerPositionUpdatedEvent += AdornerProvider_RectangleMoved;
                    _adornerProvider.DisableDrag = (Mode.MICRO_SCANAREA == _currentMode) && mROIsDisableMoveAndResize;
                    _roiAdornerLayer.Add(_adornerProvider);
                }
            }
            catch (Exception ex)
            {
                ex.ToString();
            }
        }

        private void BleachCreateROIs(string pathandname, ROICapsule capsule)
        {
            using (FileStream f = new FileStream(pathandname, FileMode.Create, FileAccess.Write))
            {
                XamlWriter.Save(capsule, f);
                f.Close();
            }
            //Prepare bleach attached properties:
            XmlDocument doc = new XmlDocument();
            XmlReader xreader = new XmlTextReader(pathandname);
            doc.Load(xreader);
            BleachClass.SetBleachNamespace(doc);
            XmlNodeList xnodes = doc.DocumentElement.ChildNodes.Item(0).ChildNodes.Item(0).ChildNodes;
            foreach (XmlNode node in xnodes)
            {
                BleachClass.SetBleachAttribute(node, doc, 0, "0");          //PreIdleTime
                BleachClass.SetBleachAttribute(node, doc, 1, "0");          //DwellTime
                BleachClass.SetBleachAttribute(node, doc, 2, "0");          //PostIdleTime
                BleachClass.SetBleachAttribute(node, doc, 3, "1");          //Itereations
                BleachClass.SetBleachAttribute(node, doc, 4, "1");          //Fill
                BleachClass.SetBleachAttribute(node, doc, 5, "0");          //PixelMode
                BleachClass.SetBleachAttribute(node, doc, 6, "0");          //Power
                BleachClass.SetBleachAttribute(node, doc, 7, "80000");      //ClkRate
                BleachClass.SetBleachAttribute(node, doc, 8, "0");          //PixelLongIdleTime
                BleachClass.SetBleachAttribute(node, doc, 9, "0");          //PrePatIdleTime
                BleachClass.SetBleachAttribute(node, doc, 10, "0");         //PostPatIdleTime
                BleachClass.SetBleachAttribute(node, doc, 11, "0");         //PreCycleIdleMS
                BleachClass.SetBleachAttribute(node, doc, 12, "0");         //PostCycleIdleMS
                BleachClass.SetBleachAttribute(node, doc, 13, "0");         //PreEpochIdleMS
                BleachClass.SetBleachAttribute(node, doc, 14, "0");         //PostEpochIdleMS
                BleachClass.SetBleachAttribute(node, doc, 15, "1");         //EpochCount

            }
            xreader.Close();
            doc.Save(pathandname);
        }

        /// <summary>
        /// Check index of ROI can be selected, limit roi selection by mode, also except Reticle.
        /// </summary>
        /// <param name="roi"></param>
        /// <returns></returns>
        private bool CheckSelectIndexOfROI(Shape roi)
        {
            if ((roi is Reticle) || (roi is Scale) ||
                (Mode.STATSONLY == _currentMode || Mode.MICRO_SCANAREA == _currentMode) && (0 != ((int[])roi.Tag)[(int)Tag.PATTERN_ID]) ||
                ((Mode.PATTERN_NOSTATS == _currentMode || Mode.PATTERN_WIDEFIELD == _currentMode) && (_patternID != ((int[])roi.Tag)[(int)Tag.PATTERN_ID])))
            {
                return false;
            }

            return true;
        }

        private void CreateMask()
        {
            if (_simulatorMode)
                return;

            List<Shape> roiList = GetStatsROI();

            _maskIndex++;
            if (null != MaskWillChangeEvent)
            {
                MaskWillChangeEvent();
            }
            int width = _pixelX;
            int height = _pixelY;
            short[] mask = new short[width * height];

            BackgroundWorker worker = new BackgroundWorker();
            string mskFile = "";
            if (0 < _roiList.Count)
            {
                //version: SecA in Tag.Mode
                if ((0 == GetTagByteSection(_roiList[0].Tag, Tag.MODE, SecA)) && (0 != _expROIsPathAndName.CompareTo("")))
                {
                    mskFile = System.IO.Directory.GetParent(_expROIsPathAndName).ToString() + "\\ROIMask.raw";
                }
            }
            worker.DoWork += (o, ea) =>
            {
                ulong wkrIndex = _maskIndex;
                if (mask.Length > 0)
                {
                    if (mskFile.CompareTo("") != 0 && File.Exists(mskFile))
                    {
                        using (var stream = File.OpenRead(mskFile))
                        {
                            var reader = new BinaryReader(stream);
                            for (int i = 0; i < width * height; i++)
                            {
                                mask[i] = reader.ReadInt16();
                            }
                        }
                    }
                    else
                    {

                        for (int i = 0; i < roiList.Count(); i++)
                        {
                            if (wkrIndex < _maskIndex)
                            {
                                ea.Cancel = true;
                                return;
                            }
                            if (roiList[i] is ROIRect)
                            {
                                RectToMask(ref mask, width, height, wkrIndex, roiList[i] as ROIRect);
                            }
                            else if (roiList[i] is ROIPoly)
                            {
                                PolyToMask(ref mask, width, height, wkrIndex, roiList[i] as ROIPoly);
                            }
                            else if (roiList[i] is ROICrosshair)
                            {
                                CrossHairToMask(ref mask, width, height, roiList[i] as ROICrosshair);
                            }
                            else if (roiList[i] is Reticle)
                            {
                                ReticleToMask(ref mask, width, height, roiList[i] as Reticle);
                            }
                            else if (roiList[i] is Line)
                            {
                                LineToMask(ref mask, width, height, wkrIndex, roiList[i] as Line);
                            }
                            else if (roiList[i] is ROILine)
                            {
                                ROILineToMask(ref mask, width, height, wkrIndex, roiList[i] as ROILine);
                            }
                            else if (roiList[i] is Polyline)
                            {
                                PolylineToMask(ref mask, width, height, wkrIndex, roiList[i] as Polyline);
                            }
                            else if (roiList[i] is ROIPolyline)
                            {
                                ROIPolylineToMask(ref mask, width, height, wkrIndex, roiList[i] as ROIPolyline);
                            }
                            else if (roiList[i] is ROIEllipse)
                            {
                                EllipseToMask(ref mask, width, height, wkrIndex, roiList[i] as ROIEllipse);
                            }
                        }
                    }

                    try
                    {
                        ea.Result = mask;

                        if (wkrIndex < _maskIndex)
                        {
                            ea.Cancel = true;
                            return;
                        }

                        SetStatsMask(mask, width, height);
                        if (true == _saveMaskEveryTime)
                        {
                            SaveMask(mask);
                        }

                        if (_lastLineIndex < _roiList.Count)
                        {
                            if (_roiList[_lastLineIndex] is Line line)
                            {
                                int[] x = new int[2];
                                int[] y = new int[2];
                                line.Dispatcher.Invoke((Action)(() =>
                                {
                                    x[0] = Convert.ToInt32(Math.Floor((line).X1));
                                    y[0] = Convert.ToInt32(Math.Floor((line).Y1));
                                    x[1] = Convert.ToInt32(Math.Floor((line).X2));
                                    y[1] = Convert.ToInt32(Math.Floor((line).Y2));
                                }));

                                SetLineProfileLine(x, y, 2, 1);
                            }
                            else if (_roiList[_lastLineIndex] is ROILine roiLine)
                            {
                                int[] x = new int[2];
                                int[] y = new int[2];
                                roiLine.Dispatcher.Invoke((Action)(() =>
                                {
                                    x[0] = Convert.ToInt32(Math.Floor((roiLine).StatsStartPoint.X));
                                    y[0] = Convert.ToInt32(Math.Floor((roiLine).StatsStartPoint.Y));
                                    x[1] = Convert.ToInt32(Math.Floor((roiLine).StatsEndPoint.X));
                                    y[1] = Convert.ToInt32(Math.Floor((roiLine).StatsEndPoint.Y));
                                }));

                                SetLineProfileLine(x, y, 2, 1);
                            }
                            else if (_roiList[_lastLineIndex] is Polyline polyline)
                            {
                                int[] x = null;
                                int[] y = null;
                                polyline.Dispatcher.Invoke((Action)(() =>
                                {
                                    x = new int[polyline.Points.Count];
                                    y = new int[polyline.Points.Count];
                                    for (int p = 0; p < polyline.Points.Count; p++)
                                    {
                                        x[p] = Convert.ToInt32(Math.Floor(polyline.Points[p].X));
                                        y[p] = Convert.ToInt32(Math.Floor(polyline.Points[p].Y));
                                    }
                                }));
                                SetLineProfileLine(x, y, x.Length, 1);
                            }
                            else if (_roiList[_lastLineIndex] is ROIPolyline roiPolyline)
                            {
                                int[] x = null;
                                int[] y = null;
                                roiPolyline.Dispatcher.Invoke((Action)(() =>
                                {
                                    x = new int[roiPolyline.StatsPoints.Count];
                                    y = new int[roiPolyline.StatsPoints.Count];
                                    for (int p = 0; p < roiPolyline.StatsPoints.Count; p++)
                                    {
                                        x[p] = Convert.ToInt32(Math.Floor(roiPolyline.StatsPoints[p].X));
                                        y[p] = Convert.ToInt32(Math.Floor(roiPolyline.StatsPoints[p].Y));
                                    }
                                }));
                                SetLineProfileLine(x, y, x.Length, 1);
                            }
                            else
                            {
                                int[] x = new int[0];
                                int[] y = new int[0];
                                SetLineProfileLine(x, y, 0, 0);
                            }
                        }
                        else
                        {
                            int[] x = new int[0];
                            int[] y = new int[0];
                            SetLineProfileLine(x, y, 0, 0);
                        }
                    }
                    catch (Exception e)
                    {
                        e.ToString();
                    }
                }
            };

            worker.RunWorkerCompleted += (o, ea) =>
            {
                try
                {
                    if (ea.Cancelled)
                    {
                        return;
                    }
                    _mask = (short[])ea.Result;
                    if (null != MaskChangedEvent) MaskChangedEvent();
                }
                catch (Exception e)
                {
                    e.ToString();
                }
            };

            worker.RunWorkerAsync();
        }

        //calls the appropiate method to create the desired Object
        private void CreateObject(Point pt)
        {
            if (false == _isObjectComplete)
            {
                if (typeof(ROIRect) == _activeType)
                {
                    CreateROIRect(pt);
                }
                else if (typeof(ROIPoly) == _activeType)
                {
                    CreateROIPoly(pt);
                }
                else if (typeof(Line) == _activeType)
                {
                    CreateROILine(pt);
                }
                else if (typeof(ROILine) == _activeType)
                {
                    CreateROILineUpdated(pt);
                }
                else if (typeof(Reticle) == _activeType)
                {
                    CreateReticle();
                }
                else if (typeof(ROICrosshair) == _activeType)
                {
                    CreateROICrosshair(pt);
                }
                else if (typeof(ROIPolyline) == _activeType)
                {
                    CreateROIPolyLineUpdated(pt);
                }
                else if (typeof(ROIEllipse) == _activeType)
                {
                    CreateROIEllipse(pt);
                }
                else if (typeof(Scale) == _activeType)
                {
                    CreateScale();
                }
            }
        }

        //create a Reticle
        private void CreateReticle()
        {
            int imgWidth = _pixelX;
            int imgHeight = _pixelY;
            _selectedPolygon = null;
            int[] tag = DefaultTags(_roiCount);
            Shape roi = new Reticle();
            tag[(int)Tag.SUB_PATTERN_ID] = GetCurrentSubPatternCount(ref roi) + 1;
            _bitVec32 = new BitVector32(tag[(int)Tag.RGB]);
            Reticle reticle = new Reticle(_imageScaleX, _imageScaleY)
            {
                Stroke = (Mode.PATTERN_NOSTATS == _currentMode || Mode.PATTERN_WIDEFIELD == _currentMode) ?
                    new SolidColorBrush(Color.FromArgb(Byte.MaxValue, Convert.ToByte(_bitVec32[SecR]), Convert.ToByte(_bitVec32[SecG]), Convert.ToByte(_bitVec32[SecB]))) :
                    GetROIColor((tag[(int)Tag.SUB_PATTERN_ID] - 1) % 8),
                StrokeThickness = _roiStrokeThickness
            };
            reticle.Fill = new SolidColorBrush(Colors.Transparent);
            reticle.ImageWidth = imgWidth;
            reticle.ImageHeight = imgHeight;
            _activeType = typeof(Reticle);

            _roiList.Add(reticle);

            _roiCount++;
            _roiList[_roiList.Count - 1].Tag = tag;
            _roiList[_roiList.Count - 1].ToolTip = "ROI #" + tag[(int)Tag.SUB_PATTERN_ID];
            if (!_simulatorMode) _overlayItems.Add(_roiList[_roiList.Count - 1]);

            ObjectComplete(_roiList.Count - 1);
            if (null != UpdatingObjectEvent) UpdatingObjectEvent(false);
            _isObjectCreated = true;
            UpdateVisibleROIs();
        }

        private void CreateROICrosshair(Point pt)
        {
            _selectedPolygon = null;
            int[] tag = DefaultTags(_roiCount);
            Shape roi = new ROICrosshair();
            tag[(int)Tag.SUB_PATTERN_ID] = GetCurrentSubPatternCount(ref roi) + 1;

            //return if previous not crosshair in NOSTATS mode:
            if ((Mode.PATTERN_NOSTATS == _currentMode) && (null == (roi as ROICrosshair)))
                return;

            _bitVec32 = new BitVector32(tag[(int)Tag.RGB]);
            ROICrosshair crosshair = new ROICrosshair()
            {
                Stroke = (Mode.PATTERN_NOSTATS == _currentMode || Mode.PATTERN_WIDEFIELD == _currentMode) ?
                    new SolidColorBrush(Color.FromArgb(Byte.MaxValue, Convert.ToByte(_bitVec32[SecR]), Convert.ToByte(_bitVec32[SecG]), Convert.ToByte(_bitVec32[SecB]))) :
                    GetROIColor((tag[(int)Tag.SUB_PATTERN_ID] - 1) % 8),
                StrokeThickness = _roiStrokeThickness,
                Fill = Brushes.Transparent,
                ImageWidth = _pixelX,
                ImageHeight = _pixelY
            };
            crosshair.CenterPoint = pt;
            crosshair.MouseLeftButtonDown += ROI_MouseDown;
            _roiList.Add(crosshair);
            _roiCount++;

            _roiList[_roiList.Count - 1].ToolTip = "ROI #" + tag[(int)Tag.SUB_PATTERN_ID];
            _roiList[_roiList.Count - 1].Tag = tag;
            _roiList[_roiList.Count - 1].Tag = SetTagRGB(_roiList[_roiList.Count - 1]);

            if (!_simulatorMode) _overlayItems.Add(_roiList[_roiList.Count - 1]);
            ObjectComplete(_roiList.Count - 1);

            //add index adorner:
            AddIndexToROI(_roiList[_roiList.Count - 1]);

            CreateROICenter();

            if (null != UpdatingObjectEvent) UpdatingObjectEvent(false);
            _isObjectCreated = true;
            InitROICrosshair();
            ValidateROIs();
            UpdateVisibleROIs();
        }

        //create an ROI Ellipse
        private void CreateROIEllipse(Point pt)
        {
            _selectedPolygon = null;
            Shape roi = new ROIEllipse();
            int[] tag = DefaultTags(_roiCount);
            tag[(int)Tag.SUB_PATTERN_ID] = GetCurrentSubPatternCount(ref roi) + 1;

            //return if previous not ellipse in NOSTATS mode:
            if ((Mode.PATTERN_NOSTATS == _currentMode) && (null == (roi as ROIEllipse)))
                return;

            ROIEllipse ellipse = new ROIEllipse();
            if ((Mode.PATTERN_NOSTATS == _currentMode) && ((SkipRedrawCenter ? 0 : 1) < tag[(int)Tag.SUB_PATTERN_ID]))
            {
                //sub-pattern index will increase only in pattern_nostats mode,
                //copy first ellipse to the rest:
                ellipse.StartPoint = new Point(pt.X - ((roi as ROIEllipse).ROIWidth / 2), pt.Y - (roi as ROIEllipse).ROIHeight / 2);
                ellipse.EndPoint = new Point(pt.X + ((roi as ROIEllipse).ROIWidth / 2), pt.Y + (roi as ROIEllipse).ROIHeight / 2);
                ellipse.Center = pt;
            }
            else
            {
                ellipse.StartPoint = pt;
                ellipse.EndPoint = pt;
                ellipse.Center = pt;
            }

            ellipse.Fill = new SolidColorBrush(Colors.Transparent);
            _bitVec32 = new BitVector32(tag[(int)Tag.RGB]);
            ellipse.Stroke = (Mode.PATTERN_NOSTATS == _currentMode || Mode.PATTERN_WIDEFIELD == _currentMode) ?
                new SolidColorBrush(Color.FromArgb(Byte.MaxValue, Convert.ToByte(_bitVec32[SecR]), Convert.ToByte(_bitVec32[SecG]), Convert.ToByte(_bitVec32[SecB]))) :
                GetROIColor((tag[(int)Tag.SUB_PATTERN_ID] - 1) % 8);
            ellipse.StrokeThickness = _roiStrokeThickness;

            ellipse.MouseLeftButtonDown += ROI_MouseDown;
            _roiList.Add(ellipse);
            _roiCount++;
            _roiList[_roiList.Count - 1].ToolTip = "ROI #" + tag[(int)Tag.SUB_PATTERN_ID];
            _roiList[_roiList.Count - 1].Tag = tag;
            _roiList[_roiList.Count - 1].Tag = SetTagRGB(_roiList[_roiList.Count - 1]);

            if (!_simulatorMode) _overlayItems.Add(_roiList[_roiList.Count - 1]);
            _isObjectCreated = true;

            //complete if ready:
            if ((Mode.PATTERN_NOSTATS == _currentMode) && ((SkipRedrawCenter ? 0 : 1) < tag[(int)Tag.SUB_PATTERN_ID]))
            {
                MouseEvent((int)MouseEventEnum.LEFTMOUSEUP, pt, false);
            }
        }

        //create an ROI Line
        private void CreateROILine(Point pt)
        {
            _selectedPolygon = null;
            int[] tag = DefaultTags(_roiCount);
            Shape roi = new Line();
            tag[(int)Tag.SUB_PATTERN_ID] = GetCurrentSubPatternCount(ref roi) + 1;
            _bitVec32 = new BitVector32(tag[(int)Tag.RGB]);
            Line line = new Line
            {
                Stroke = (Mode.PATTERN_NOSTATS == _currentMode || Mode.PATTERN_WIDEFIELD == _currentMode) ?
                    new SolidColorBrush(Color.FromArgb(Byte.MaxValue, Convert.ToByte(_bitVec32[SecR]), Convert.ToByte(_bitVec32[SecG]), Convert.ToByte(_bitVec32[SecB]))) :
                    GetROIColor((tag[(int)Tag.SUB_PATTERN_ID] - 1) % 8),
                StrokeThickness = _roiStrokeThickness,
                Name = "Line" + _roiList.Count + 1,
                X1 = pt.X,
                Y1 = pt.Y,
                X2 = pt.X + 1,
                Y2 = pt.Y + 1,
            };
            line.MouseLeftButtonDown += ROI_MouseDown;
            _roiList.Add(line);

            _roiCount++;
            _roiList[_roiList.Count - 1].ToolTip = "ROI #" + tag[(int)Tag.SUB_PATTERN_ID];
            _roiList[_roiList.Count - 1].Tag = tag;
            _roiList[_roiList.Count - 1].Tag = SetTagRGB(_roiList[_roiList.Count - 1]);

            if (!_simulatorMode) _overlayItems.Add(_roiList[_roiList.Count - 1]);
            _isObjectCreated = true;
            _lastLineIndex = _roiList.Count - 1;

            if (true == _showLineLength)
            {
                AddGeometryToROI(_roiList[_roiList.Count - 1]);
            }
        }

        private void CreateROILineUpdated(Point pt)
        {
            _selectedPolygon = null;
            int[] tag = DefaultTags(_roiCount);
            Shape roi = new ROILine();
            tag[(int)Tag.SUB_PATTERN_ID] = GetCurrentSubPatternCount(ref roi) + 1;
            _bitVec32 = new BitVector32(tag[(int)Tag.RGB]);
            ROILine line = new ROILine
            {
                Stroke = (Mode.PATTERN_NOSTATS == _currentMode || Mode.PATTERN_WIDEFIELD == _currentMode) ?
                    new SolidColorBrush(Color.FromArgb(Byte.MaxValue, Convert.ToByte(_bitVec32[SecR]), Convert.ToByte(_bitVec32[SecG]), Convert.ToByte(_bitVec32[SecB]))) :
                    GetROIColor((tag[(int)Tag.SUB_PATTERN_ID] - 1) % 8),
                StrokeThickness = _roiStrokeThickness,
                Name = "Line" + _roiList.Count + 1,
                StartPoint = pt,
                EndPoint = new Point(pt.X + 1, pt.Y + 1),
                ImageScaleX = _imageScaleX,
                ImageScaleY = _imageScaleY
            };
            line.MouseLeftButtonDown += ROI_MouseDown;
            _roiList.Add(line);

            _roiCount++;
            _roiList[_roiList.Count - 1].ToolTip = "ROI #" + tag[(int)Tag.SUB_PATTERN_ID];
            _roiList[_roiList.Count - 1].Tag = tag;
            _roiList[_roiList.Count - 1].Tag = SetTagRGB(_roiList[_roiList.Count - 1]);

            if (!_simulatorMode) _overlayItems.Add(_roiList[_roiList.Count - 1]);
            _isObjectCreated = true;
            _lastLineIndex = _roiList.Count - 1;

            if (true == _showLineLength)
            {
                AddGeometryToROI(_roiList[_roiList.Count - 1]);
            }
        }

        //create an ROI Polygon
        private void CreateROIPoly(Point pt)
        {
            _selectedPolygon = new Polygon { Stroke = Brushes.Goldenrod, StrokeThickness = 1, StrokeDashArray = new DoubleCollection { 3, 3 } };
            _selectedPolygon.Name = "Polygon";
            int[] tag = DefaultTags(_roiCount);
            Shape roi = new Polygon();
            tag[(int)Tag.SUB_PATTERN_ID] = GetCurrentSubPatternCount(ref roi) + 1;
            _bitVec32 = new BitVector32(tag[(int)Tag.RGB]);
            ROIPoly roundedPolygon = new ROIPoly
            {
                Stroke = (Mode.PATTERN_NOSTATS == _currentMode || Mode.PATTERN_WIDEFIELD == _currentMode) ?
                    new SolidColorBrush(Color.FromArgb(Byte.MaxValue, Convert.ToByte(_bitVec32[SecR]), Convert.ToByte(_bitVec32[SecG]), Convert.ToByte(_bitVec32[SecB]))) :
                    GetROIColor((tag[(int)Tag.SUB_PATTERN_ID] - 1) % 8),
                StrokeThickness = _roiStrokeThickness,
                UseRoundnessPercentage = true,
                Fill = Brushes.Transparent
            };
            roundedPolygon.MouseLeftButtonDown += ROI_MouseDown;
            _roiList.Add(roundedPolygon);

            if (!_simulatorMode)
            {
                _overlayItems.Add(_selectedPolygon);
                _overlayItems.Add(_roiList[_roiList.Count - 1]);
            }
            _roiList[_roiList.Count - 1].ToolTip = "ROI #" + tag[(int)Tag.SUB_PATTERN_ID];
            _roiList[_roiList.Count - 1].Tag = tag;
            _roiList[_roiList.Count - 1].Tag = SetTagRGB(_roiList[_roiList.Count - 1]);

            if (!_simulatorMode) _overlayItems.Add(GetCornerEllipse(pt));

            ((ROIPoly)_roiList[_roiList.Count - 1]).Points.Add(pt);

            _selectedPolygon.Points.Add(pt);

            _activeType = typeof(ROIPoly);
            _isObjectCreated = true;
        }

        //create an ROI Polyline
        private void CreateROIPolyLine(Point pt)
        {
            int[] tag = DefaultTags(_roiCount);
            Shape roi = new Polyline();
            tag[(int)Tag.SUB_PATTERN_ID] = GetCurrentSubPatternCount(ref roi) + 1;
            _bitVec32 = new BitVector32(tag[(int)Tag.RGB]);
            Polyline polyLine = new Polyline
            {
                Stroke = (Mode.PATTERN_NOSTATS == _currentMode || Mode.PATTERN_WIDEFIELD == _currentMode) ?
                    new SolidColorBrush(Color.FromArgb(Byte.MaxValue, Convert.ToByte(_bitVec32[SecR]), Convert.ToByte(_bitVec32[SecG]), Convert.ToByte(_bitVec32[SecB]))) :
                    GetROIColor((tag[(int)Tag.SUB_PATTERN_ID] - 1) % 8),
                StrokeThickness = _roiStrokeThickness
            };
            polyLine.Name = "Polyline" + _roiList.Count + 1;
            polyLine.MouseLeftButtonDown += ROI_MouseDown;
            polyLine.Points.Add(pt);
            _roiList.Add(polyLine);
            _overlayItems.Add(_roiList.Last());

            _roiList.Last().ToolTip = "ROI #" + tag[(int)Tag.SUB_PATTERN_ID];
            _roiList.Last().Tag = tag;
            _roiList.Last().Tag = SetTagRGB(_roiList.Last());

            if (!_simulatorMode) _overlayItems.Add(GetCornerEllipse(pt));

            _activeType = typeof(Polyline);
            _isObjectCreated = true;

            if (true == _showPolylineLength)
            {
                AddGeometryToROI(_roiList[_roiList.Count - 1]);
            }
        }

        private void CreateROIPolyLineUpdated(Point pt)
        {
            int[] tag = DefaultTags(_roiCount);
            Shape roi = new ROIPolyline();
            tag[(int)Tag.SUB_PATTERN_ID] = GetCurrentSubPatternCount(ref roi) + 1;
            _bitVec32 = new BitVector32(tag[(int)Tag.RGB]);
            ROIPolyline polyLine = new ROIPolyline
            {
                Stroke = (Mode.PATTERN_NOSTATS == _currentMode || Mode.PATTERN_WIDEFIELD == _currentMode) ?
                    new SolidColorBrush(Color.FromArgb(Byte.MaxValue, Convert.ToByte(_bitVec32[SecR]), Convert.ToByte(_bitVec32[SecG]), Convert.ToByte(_bitVec32[SecB]))) :
                    GetROIColor((tag[(int)Tag.SUB_PATTERN_ID] - 1) % 8),
                StrokeThickness = _roiStrokeThickness
            };
            polyLine.Name = "Polyline" + _roiList.Count + 1;
            polyLine.MouseLeftButtonDown += ROI_MouseDown;
            polyLine.Points.Add(pt);
            _roiList.Add(polyLine);
            _overlayItems.Add(_roiList.Last());

            _roiList.Last().ToolTip = "ROI #" + tag[(int)Tag.SUB_PATTERN_ID];
            _roiList.Last().Tag = tag;
            _roiList.Last().Tag = SetTagRGB(_roiList.Last());

            if (!_simulatorMode) _overlayItems.Add(GetCornerEllipse(pt));

            _activeType = typeof(ROIPolyline);
            _isObjectCreated = true;
            _lastLineIndex = _roiList.Count - 1;

            if (true == _showPolylineLength)
            {
                AddGeometryToROI(_roiList[_roiList.Count - 1]);
            }
        }

        //create an ROI Rectangle
        private void CreateROIRect(Point pt)
        {
            _selectedPolygon = null;
            int[] tag = DefaultTags(_roiCount);

            Shape roi = new ROIRect();
            tag[(int)Tag.SUB_PATTERN_ID] = GetCurrentSubPatternCount(ref roi) + 1;
            _bitVec32 = new BitVector32(tag[(int)Tag.RGB]);
            ROIRect rectangle = new ROIRect(pt, pt, _imageScaleX, _imageScaleY)
            {
                Stroke = (Mode.PATTERN_NOSTATS == _currentMode || Mode.PATTERN_WIDEFIELD == _currentMode) ?
                   new SolidColorBrush(Color.FromArgb(Byte.MaxValue, Convert.ToByte(_bitVec32[SecR]), Convert.ToByte(_bitVec32[SecG]), Convert.ToByte(_bitVec32[SecB]))) :
                   GetROIColor((tag[(int)Tag.SUB_PATTERN_ID] - 1) % 8),
                StrokeThickness = _roiStrokeThickness,
                StartPoint = pt,
                Fill = new SolidColorBrush(Colors.Transparent)
            };
            /*rectangle.ScaleX = _imageXScale;
            rectangle.ScaleY = _imageYScale;
            rectangle.StartPoint = pt;
            rectangle.EndPoint = pt;*/
            rectangle.MouseLeftButtonDown += ROI_MouseDown;
            _roiList.Add(rectangle);

            _roiCount++;
            _roiList[_roiList.Count - 1].ToolTip = "ROI #" + tag[(int)Tag.SUB_PATTERN_ID];
            _roiList[_roiList.Count - 1].Tag = tag;
            _roiList[_roiList.Count - 1].Tag = SetTagRGB(_roiList[_roiList.Count - 1]);

            if (!_simulatorMode) _overlayItems.Add(_roiList[_roiList.Count - 1]);
            _isObjectCreated = true;
        }

        //create a Scale
        private void CreateScale()
        {
            _selectedPolygon = null;
            int[] tag = DefaultTags(_roiCount);
            Shape roi = new Scale();
            tag[(int)Tag.SUB_PATTERN_ID] = GetCurrentSubPatternCount(ref roi) + 1;

            int[] tag2 = DefaultTags(_roiCount + 1);
            Shape roi2 = new Scale();
            tag2[(int)Tag.SUB_PATTERN_ID] = GetCurrentSubPatternCount(ref roi2) + 1;

            _bitVec32 = new BitVector32(tag[(int)Tag.RGB]);
            Brush scaleStroke = (Mode.PATTERN_NOSTATS == _currentMode || Mode.PATTERN_WIDEFIELD == _currentMode) ?
                    new SolidColorBrush(Color.FromArgb(Byte.MaxValue, Convert.ToByte(_bitVec32[SecR]), Convert.ToByte(_bitVec32[SecG]), Convert.ToByte(_bitVec32[SecB]))) :
                    GetROIColor((tag[(int)Tag.SUB_PATTERN_ID] - 1) % 8);
            ScaleLines scaleL = new ScaleLines()
            {
                Stroke = scaleStroke,
                StrokeThickness = _roiStrokeThickness * _roiScaleLineStrokeThicknessMultiplier
            };
            scaleL.ImageScaleX = _imageScaleX;
            scaleL.ImageScaleY = _imageScaleY;
            scaleL.ImageWidth = _pixelX;
            scaleL.ImageHeight = _pixelY;
            scaleL.ScaleFieldWidth = _fieldWidth;
            scaleL.ScaleFieldHeight = _fieldHeight;
            _roiList.Add(scaleL);

            ScaleNumbers scaleN = new ScaleNumbers()
            {
                Stroke = scaleStroke,
                StrokeThickness = _roiStrokeThickness * _roiScaleNumberStrokeThicknessMultiplier,
                Fill = scaleStroke
            };
            scaleN.ImageWidth = _pixelX;
            scaleN.ImageHeight = _pixelY;
            scaleN.ImageScaleX = _imageScaleX;
            scaleN.ImageScaleY = _imageScaleY;
            scaleN.ScaleFieldWidth = _fieldWidth;
            scaleN.ScaleFieldHeight = _fieldHeight;
            _roiList.Add(scaleN);

            _activeType = typeof(Scale);

            _roiCount += 2;
            _roiList[_roiList.Count - 1].Tag = tag2;
            _roiList[_roiList.Count - 1].ToolTip = "ROI #" + tag2[(int)Tag.SUB_PATTERN_ID];
            _overlayItems.Add(_roiList[_roiList.Count - 1]);

            _roiList[_roiList.Count - 2].Tag = tag;
            _roiList[_roiList.Count - 2].ToolTip = "ROI #" + tag[(int)Tag.SUB_PATTERN_ID];
            _overlayItems.Add(_roiList[_roiList.Count - 2]);

            ObjectComplete(_roiList.Count - 1);
            if (null != UpdatingObjectEvent) UpdatingObjectEvent(false);
            _isObjectCreated = true;
            UpdateVisibleROIs();
        }

        private void CrossHairToMask(ref short[] mask, int imgWidth, int imgHeight, ROICrosshair roi)
        {
            try
            {
                int ptX = 0;
                int ptY = 0;
                int[] tag = null;

                roi.Dispatcher.Invoke((Action)(() =>
                {
                    ptX = Convert.ToInt32(Math.Floor(roi.StatsCenterPoint.X));
                    ptY = Convert.ToInt32(Math.Floor(roi.StatsCenterPoint.Y));
                    tag = (int[])((int[])(roi.Tag)).Clone();
                }));

                if (imgHeight < ptY || imgWidth < ptX) return;
                mask[ptY * imgWidth + ptX] = Convert.ToInt16(tag[(int)Tag.SUB_PATTERN_ID]);
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + "CrossHairToMask exception " + ex.Message);
            }
        }

        private int[] DefaultTags(int index, object refTag = null)
        {
            //*** Old tags info ***//
            //int[] newTag = new int[4];
            //newTag[0] = index; // ROI Index in the list
            //newTag[1] = index; // ROI Index that includes deleted ROIS
            //newTag[2] = 0; // Flag (0 or 1) to know if the AdjustingAdorners are on or off
            //newTag[3] = 0; // Flag (0 or 1) to know if the GeometryAdorners are on or off
            //return newTag;

            //*** New tags: combine all flags into one int ***//
            int[] newTag;
            Version ver = System.Reflection.Assembly.GetExecutingAssembly().GetName().Version;
            int versionInt = ver.Major * 10 + ver.Minor;
            string[] zUMsplit = (ZUM - (ZRefMM * (double)Constants.UM_TO_MM)).ToString("F3", CultureInfo.InvariantCulture).Split('.');

            if (null != refTag)
            {
                if (4 < ((int[])refTag).Length)
                {
                    newTag = new int[(int)Tag.LAST_TAG];

                    for (int i = 0; i < ((int[])refTag).Length; i++)
                    {
                        newTag[i] = (1 == i) ? index : ((int[])refTag)[i];
                    }
                    for (int i = ((int[])refTag).Length; i < (int)Tag.LAST_TAG; i++)
                    {
                        newTag[i] = 0;
                    }
                    return newTag;
                }
                else
                {
                    //backward compatible: [for 3.0 or earlier]
                    newTag = new int[(int)Tag.LAST_TAG];
                    _bitVec32 = new BitVector32((int)Mode.STATSONLY);
                    _bitVec32[SecA] = versionInt;
                    newTag[(int)Tag.MODE] = _bitVec32.Data;                                             // Mode (0:stats only, 1: pattern without stats, in SecR), Version (in SecA)
                    newTag[(int)Tag.ROI_ID] = ((int[])refTag)[1];                                       // ROI Index that includes deleted ROIS
                    newTag[(int)Tag.PATTERN_ID] = 0;                                                    // Pattern Index, 1-based if used, 0: stats only

                    _bitVec32 = new BitVector32();
                    if (((int[])refTag).Length < 4)
                    {
                        _bitVec32[BitVector32.CreateMask(0)] = false;
                    }
                    else
                    {
                        _bitVec32[BitVector32.CreateMask(0)] = (1 == ((int[])refTag)[3]) ? true : false;// GeometryAdorners are on or off
                    }
                    _bitVec32[BitVector32.CreateMask(1)] = false;                                       // AdjustingAdorners are on or off
                    _bitVec32[BitVector32.CreateMask(2)] = true;                                        // do stats or not
                    _bitVec32[BitVector32.CreateMask(4)] = true;                                        // do display or not
                    _bitVec32[BitVector32.CreateMask(8)] = true;                                        // indexAdorners are on or off
                    _bitVec32[BitVector32.CreateMask(16)] = true;                                       // do save or not
                    newTag[(int)Tag.FLAGS] = _bitVec32.Data;

                    newTag[(int)Tag.SUB_PATTERN_ID] = index + 1;                                        // Sub-Pattern ROI Index, 1-based if used, 0 reserved for center

                    SolidColorBrush cBrush = (SolidColorBrush)GetROIColor(index % 8);
                    _bitVec32 = new BitVector32(cBrush.Color.R);
                    _bitVec32[SecG] = cBrush.Color.G;
                    _bitVec32[SecB] = cBrush.Color.B;
                    newTag[(int)Tag.RGB] = _bitVec32.Data;                                              // RGB for brush color, R starts from lower bits.
                    newTag[(int)Tag.WAVELENGTH_NM] = 0;                                                 // Wavelength of light in [nm]
                    newTag[(int)Tag.Z_UM_INT] = 0;                                                      // z integer value in [um]
                    newTag[(int)Tag.Z_UM_DEC] = 0;                                                      // z decimal value in [um]
                    newTag[(int)Tag.MROI_STRIPE_COUNT] = -1;                                            // number of field stripes. -1 if not used
                    return newTag;
                }
            }
            newTag = new int[(int)Tag.LAST_TAG];
            _bitVec32 = new BitVector32((int)_currentMode);
            _bitVec32[SecA] = versionInt;
            newTag[(int)Tag.MODE] = _bitVec32.Data;                                                     // Mode (0:stats only, 1: pattern without stats, in SecR), Version (in SecA)
            newTag[(int)Tag.ROI_ID] = index;                                                            // ROI Index that includes deleted ROIS
            newTag[(int)Tag.PATTERN_ID] = _patternID;                                                   // Pattern Index, 1-based if used, 0: stats only

            _bitVec32 = new BitVector32();
            _bitVec32[BitVector32.CreateMask(0)] = false;                                               // GeometryAdorners are on or off
            _bitVec32[BitVector32.CreateMask(1)] = false;                                               // AdjustingAdorners are on or off
            _bitVec32[BitVector32.CreateMask(2)] = (Mode.STATSONLY == _currentMode) ? true : false;     // do stats or not
            _bitVec32[BitVector32.CreateMask(4)] = _visible;                                            // do display or not
            _bitVec32[BitVector32.CreateMask(8)] = _patternSubIndexVisible;                             // indexAdorners are on or off
            _bitVec32[BitVector32.CreateMask(16)] = _save;                                              // do save or not
            newTag[(int)Tag.FLAGS] = _bitVec32.Data;                                                    // Flags (0: GeometryAdorners, 1: AdjustingAdorners, 2: stats, 3: display 4: indexAdorners 5: save)

            newTag[(int)Tag.SUB_PATTERN_ID] = 0;                                                        // Sub-Pattern ROI Index, 1-based if used, 0: center
            newTag[(int)Tag.RGB] = _colorRGB;                                                           // RGB for brush color, R starts from lower bits.
            newTag[(int)Tag.WAVELENGTH_NM] = _wavelengthNM;                                             // Wavelength of light in [nm]
            newTag[(int)Tag.Z_UM_INT] = (Mode.PATTERN_NOSTATS == _currentMode || Mode.PATTERN_WIDEFIELD == _currentMode) ? int.Parse(zUMsplit[0]) : 0;    // z integer value in [um]
            newTag[(int)Tag.Z_UM_DEC] = (Mode.PATTERN_NOSTATS == _currentMode || Mode.PATTERN_WIDEFIELD == _currentMode) ? int.Parse(zUMsplit[1]) : 0;    // z decimal value in [um]
            newTag[(int)Tag.MROI_STRIPE_COUNT] = -1;                                            // number of field stripes. -1 if not used
            return newTag;
        }

        private void DeleteSingleObject(int roiIndex)
        {
            if (_roiList.Count > 0)
            {
                try
                {
                    _overlayItems.Remove(_roiList[roiIndex]);
                    _roiList.RemoveAt(roiIndex);
                    if (_roiList.Count > 0 && roiIndex != _roiList.Count && _lastLineIndex == _roiList.Count)
                        _lastLineIndex--;

                    CreateMask();
                    if (true == _saveEveryChange)
                    {
                        SaveROIs(_expROIsPathAndName);
                    }
                }
                catch (Exception ex)
                {
                    ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + "DeleteSingleObject exception " + ex.Message);
                }
                _activeType = null;
            }
        }

        /// <summary>
        /// Deselects the input ROI, check selected if specified.
        /// </summary>
        /// <param name="roi"></param>
        private void DeselectROI(Shape roi, bool checkSelected = false)
        {
            if (checkSelected)
            {
                if (IsROISelected(roi))
                {
                    if (true == RemoveAdornerFromROI(roi, AdornerType.ADJUST_ADORNER))
                    {
                        int[] tag = SetTagBit(roi.Tag, Tag.FLAGS, Flag.ADJUST_ADONERS, false);
                        roi.Tag = tag;
                    }
                }
            }
            else
            {
                RemoveAdornerFromROI(roi, AdornerType.ADJUST_ADORNER);
                int[] tag = SetTagBit(roi.Tag, Tag.FLAGS, Flag.ADJUST_ADONERS, false);
                roi.Tag = tag;
            }
        }

        // generate a binary mask based on user defined ellipse ROI (inside ROI->1, outside ROI->0)
        private void EllipseToMask(ref short[] mask, int imgWidth, int imgHeight, ulong wkrIndex, ROIEllipse roi)
        {
            try
            {
                Rect boundingRect = new Rect();
                int[] tag = null;

                roi.Dispatcher.Invoke((Action)(() =>
                {
                    boundingRect = roi.StatsBounds;
                    tag = (int[])((int[])(roi.Tag)).Clone();
                }));

                System.Drawing.Bitmap b = new System.Drawing.Bitmap(imgWidth, imgHeight);

                int left = Convert.ToInt32(Math.Floor(boundingRect.Left));
                int top = Convert.ToInt32(Math.Floor(boundingRect.Top));
                int width = Convert.ToInt32(Math.Floor(boundingRect.Width)) + 1;
                int height = Convert.ToInt32(Math.Floor(boundingRect.Height)) + 1;

                using (System.Drawing.Graphics g = System.Drawing.Graphics.FromImage(b))
                {
                    g.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.None;
                    g.Clear(System.Drawing.Color.Black);

                    g.FillEllipse(System.Drawing.Brushes.White, left - 1, top - 1, width + 1, height + 1);
                }

                int maxY = top + height;
                if (maxY > imgHeight)
                {
                    maxY = imgHeight;
                }

                int maxX = left + width;
                if (maxX > imgWidth)
                {
                    maxX = imgWidth;
                }

                //fill Ellipse
                for (int y = top; y < maxY; y++)
                {
                    for (int x = left; x < maxX; x++)
                    {
                        if (wkrIndex < _maskIndex)
                        {
                            return;
                        }

                        if (b.GetPixel(x, y).R > 0)
                        {
                            if ((mask.Length - 1) < (y * b.Width + x))
                            {
                                continue;
                            }
                            mask[y * b.Width + x] = Convert.ToInt16(tag[(int)Tag.SUB_PATTERN_ID]);
                        }
                    }
                }

            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + "EllipseToMask exception " + ex.Message);
            }
        }

        //Creates the Ellipses that go on each corner of a polygon allowing the user to reshape the polygons
        private System.Windows.Shapes.Path GetCornerEllipse(Point point)
        {
            var geometry = new EllipseGeometry { Center = point, RadiusX = 1.5 };
            geometry.RadiusY = geometry.RadiusX;
            _bitVec32 = new BitVector32(_colorRGB);
            var path = new System.Windows.Shapes.Path
            {
                Data = geometry,
                Fill = (Mode.PATTERN_NOSTATS == _currentMode || Mode.PATTERN_WIDEFIELD == _currentMode) ?
                    new SolidColorBrush(Color.FromArgb(Byte.MaxValue, Convert.ToByte(_bitVec32[SecR]), Convert.ToByte(_bitVec32[SecG]), Convert.ToByte(_bitVec32[SecB]))) :
                    GetROIColor((((int[])(_roiList[_roiList.Count - 1].Tag))[(int)Tag.SUB_PATTERN_ID] - 1) % 8)
            };
            int[] pointAndObjectIndex = new int[2];
            return path;
        }

        /// <summary>
        /// return current sub-pattern count and first ROI.
        /// </summary>
        /// <param name="patternID"></param>
        /// <returns></returns>
        private int GetCurrentSubPatternCount(ref Shape roi)
        {
            int subPatternCount = SkipRedrawCenter ? -1 : 0;
            bool found = false;
            for (int i = 0; i < _roiList.Count; i++)
            {
                if (null != _roiList[i].Tag)
                {
                    if ((_currentMode == (Mode)(GetTagByteSection(_roiList[i].Tag, Tag.MODE, SecR))) &&
                        (_patternID == ((int[])_roiList[i].Tag)[(int)Tag.PATTERN_ID]) &&
                        (subPatternCount < ((int[])_roiList[i].Tag)[(int)Tag.SUB_PATTERN_ID]))
                    {
                        subPatternCount = ((int[])_roiList[i].Tag)[(int)Tag.SUB_PATTERN_ID];
                        if (!found)
                        {
                            roi = _roiList[i];
                            found = true;
                        }
                    }
                }
            }
            return subPatternCount;
        }

        /// <summary>
        /// Returns the index of the next ROI, wrapping around to the beginning of the list when needed
        /// </summary>
        /// <param name="index"> Index of current ROI </param>
        /// <returns> The index of the next ROI, or -1 if there are no ROI's </returns>
        private int GetIndexOfNextROI(int index)
        {
            if (_roiList.Count <= 0)
            {
                return -1;
            }

            //find next potential index:
            index = WrapIndex(index + 1, _roiList.Count);

            //start search for next valid index:
            for (int i = index; i < _roiList.Count; i++)
            {
                if (CheckSelectIndexOfROI(_roiList[i]))
                {
                    return i;
                }
            }

            //if not found, try the rest:
            for (int i = 0; i < index; i++)
            {
                if (CheckSelectIndexOfROI(_roiList[i]))
                {
                    return i;
                }
            }
            return -1;
        }

        /// <summary>
        /// Returns the index of the previous ROI, wrapping around to the end of the list when needed
        /// </summary>
        /// <param name="index"> Index of current ROI </param>
        /// <returns> The index of the previous ROI, or -1 if there are no ROI's </returns>
        private int GetIndexOfPrevROI(int index)
        {
            if (_roiList.Count <= 0)
            {
                return -1;
            }

            //find next potential index:
            index = WrapIndex(index - 1, _roiList.Count);

            //start search for next valid index:
            for (int i = index; i >= 0; i--)
            {
                if (CheckSelectIndexOfROI(_roiList[i]))
                {
                    return i;
                }
            }

            //if not found, try the rest:
            for (int i = (_roiList.Count - 1); i > index; i--)
            {
                if (CheckSelectIndexOfROI(_roiList[i]))
                {
                    return i;
                }
            }
            return -1;
        }

        private Brush GetROIColor(int colorIndex, object tag = null)
        {
            if ((null != tag) && ((int)Tag.RGB < ((int[])tag).Length))
            {
                _bitVec32 = new BitVector32(((int[])tag)[(int)Tag.RGB]);
                Color newColor = Color.FromArgb(Byte.MaxValue, Convert.ToByte(_bitVec32[SecR]), Convert.ToByte(_bitVec32[SecG]), Convert.ToByte(_bitVec32[SecB]));
                return new SolidColorBrush(newColor);
            }

            switch (colorIndex)
            {
                case 0:
                    {
                        return Brushes.Yellow;
                    }
                case 1:
                    {
                        return Brushes.Lime;
                    }
                case 2:
                    {
                        return Brushes.DodgerBlue;
                    }
                case 3:
                    {
                        return Brushes.DeepPink;
                    }
                case 4:
                    {
                        return Brushes.DarkOrange;
                    }
                case 5:
                    {
                        return Brushes.Khaki;
                    }
                case 6:
                    {
                        return Brushes.LightGreen;
                    }
                case 7:
                    {
                        return Brushes.SteelBlue;
                    }
                default:
                    {
                        return Brushes.Black;
                    }
            }
        }

        /// <summary>
        /// Returns the first selected ROI, search by ascending (1) or decending (-1) index order
        /// </summary>
        /// <param name="forwardSearch"></param>
        /// <returns>Returns -1 of no ROI selected</returns>
        private int GetSelectedIndex(int searchDirection)
        {
            if (null == _roiList || _roiList.Count == 0)
            {
                return -1;
            }
            if (true == _isObjectComplete)
            {
                switch (searchDirection)
                {
                    case 1: //search ascending index
                        for (int i = 0; i < _roiList.Count; i++)
                        {
                            Shape roi = _roiList[i];
                            if (IsROISelected(roi))
                            {
                                return i;
                            }
                        }
                        break;
                    case -1: //search decending index
                        for (int i = _roiList.Count - 1; i >= 0; i--)
                        {
                            Shape roi = _roiList[i];
                            if (IsROISelected(roi))
                            {
                                return i;
                            }
                        }
                        break;
                    default:
                        break;
                }
            }
            return -1;
        }

        /// <summary>
        /// return all ROIs to do stats.
        /// </summary>
        /// <returns></returns>
        private List<Shape> GetStatsROI()
        {
            List<Shape> roiList = new List<Shape>();
            for (int i = 0; i < _roiList.Count; i++)
            {
                if (false == GetTagBit(_roiList[i].Tag, Tag.FLAGS, Flag.STATS))
                {
                    continue;
                }
                roiList.Add(_roiList[i]);
            }
            return roiList;
        }

        private bool GetTagBit(object tag, Tag tagID, Flag location)
        {
            int[] localTag = (int[])tag;
            int byteLocation = (0 < (int)location) ? (int)Math.Pow(2, (int)location - 1) : (int)location;
            if (null != localTag)
            {
                if ((0 <= tagID) && ((int)tagID < localTag.Length))
                {
                    _bitVec32 = new BitVector32(localTag[(int)tagID]);
                    return _bitVec32[BitVector32.CreateMask(byteLocation)];
                }
            }
            return false;
        }

        int GetTagValue(object tag, Tag tagID)
        {
            int[] localTag = (int[])tag;
            if (null != localTag)
            {
                if ((0 <= tagID) && ((int)tagID < localTag.Length))
                {
                    return localTag[(int)tagID];
                }
            }
            return -1;
        }

        //Initializes the Canvas making sure no ROI Polygon was left half complete and clears the canvas if there was a reticle, redrawing the previous ROIs
        private void InitROI()
        {
            try
            {   //clean canvas if polygon or polyline was not completed
                if (_activeType == typeof(ROIPoly))
                {
                    if (false == _isObjectComplete && true == _isObjectCreated)
                    {

                        for (int i = ((ROIPoly)_roiList[_roiList.Count - 1]).Points.Count; i >= 0 + 1; i--)
                        {
                            _overlayItems.RemoveAt(_overlayItems.Count - i);
                        }
                        _overlayItems.Remove(_roiList[_roiList.Count - 1]);
                        _overlayItems.Remove(_selectedPolygon);
                        _roiList.RemoveAt(_roiList.Count - 1);
                    }
                }
                else if (_activeType == typeof(Polyline))
                {
                    if (false == _isObjectComplete && true == _isObjectCreated)
                    {

                        for (int i = ((Polyline)_roiList[_roiList.Count - 1]).Points.Count; i >= 0 + 1; i--)
                        {
                            _overlayItems.RemoveAt(_overlayItems.Count - i);
                        }
                        _overlayItems.Remove(_roiList[_roiList.Count - 1]);
                        _roiList.RemoveAt(_roiList.Count - 1);
                    }
                }
                else if (_activeType == typeof(ROIPolyline))
                {
                    if (false == _isObjectComplete && true == _isObjectCreated)
                    {

                        for (int i = ((ROIPolyline)_roiList[_roiList.Count - 1]).Points.Count; i >= 0 + 1; i--)
                        {
                            _overlayItems.RemoveAt(_overlayItems.Count - i);
                        }
                        _overlayItems.Remove(_roiList[_roiList.Count - 1]);
                        _roiList.RemoveAt(_roiList.Count - 1);
                    }
                }

                //clear adorners if a new roi is being initiated
                for (int i = 0; i < _roiList.Count; i++)
                {
                    DeselectROI(_roiList[i]);
                }
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + "InitROI exception " + ex.Message);
            }
        }

        private void LineToMask(ref short[] mask, int imgWidth, int imgHeight, ulong wkrIndex, Line roi)
        {
            try
            {
                int x1 = 0;
                int x2 = 0;
                int y1 = 0;
                int y2 = 0;
                int[] tag = null;

                roi.Dispatcher.Invoke((Action)(() =>
                {
                    x1 = Convert.ToInt32(Math.Floor(roi.X1));
                    x2 = Convert.ToInt32(Math.Floor(roi.X2));

                    y1 = Convert.ToInt32(Math.Floor(roi.Y1));
                    y2 = Convert.ToInt32(Math.Floor(roi.Y2));

                    tag = (int[])((int[])(roi.Tag)).Clone();
                }));
                SingleLineToMask(ref mask, imgWidth, imgHeight, tag, x1, x2, y1, y2, wkrIndex);
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + "LineToMask exception " + ex.Message);
            }
        }

        private void LoadSettings()
        {
            try
            {
                XmlDocument appSettingsFile = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
                if (null != appSettingsFile)
                {
                    XmlNode node = appSettingsFile.SelectSingleNode("/ApplicationSettings/DisplayOptions/OverlayManager");

                    if (node != null)
                    {
                        string str = string.Empty;

                        XmlManager.GetAttribute(node, appSettingsFile, "showLineLength", ref str);
                        _showLineLength = ("1" == str) ? true : false;
                        XmlManager.GetAttribute(node, appSettingsFile, "showPolylineLength", ref str);
                        _showPolylineLength = ("1" == str) ? true : false;
                    }
                }
            }
            catch (Exception e)
            {
                e.ToString();
            }
        }

        private void ObjectComplete(int roiIndex)
        {
            _isObjectComplete = true;
            CreateMask();
            if (true == _saveEveryChange)
            {
                SaveROIs(_expROIsPathAndName);
            }
        }

        /// <summary>
        /// Determine limit range by parsing input string
        /// </summary>
        /// <param name="input"></param>
        /// <returns></returns>
        private int[] ParseLimit(string input)
        {
            int[] limit = { 0, Int32.MaxValue };
            int subID = 0;

            //parse subIDstr for condition (< or >), expected one side only:
            if (!Int32.TryParse(input, out subID))
            {
                Match oMatch = Regex.Match(input, @"\s*([<>=]+)\s*(\d+)\s*?");
                if (oMatch.Success)
                {
                    int lVal = Convert.ToInt32(oMatch.Groups[2].Value);
                    switch (oMatch.Groups[1].Value)
                    {
                        case "<":
                            limit[1] = Math.Max(0, lVal - 1);
                            break;
                        case "<=":
                        case "=<":
                            limit[1] = Math.Max(0, lVal);
                            break;
                        case ">":
                            limit[0] = lVal + 1;
                            break;
                        case ">=":
                        case "=>":
                            limit[0] = lVal;
                            break;
                        case "==":
                            limit[0] = limit[1] = lVal;
                            break;
                    }
                }
            }
            else
            {
                limit[0] = limit[1] = subID;
            }
            return limit;
        }

        private void PersistSettings()
        {
            ResourceManagerCS.BorrowDocMutexCS(SettingsFileType.APPLICATION_SETTINGS);
            MVMManager.Instance.ReloadSettings(SettingsFileType.APPLICATION_SETTINGS);
            XmlDocument appSettingsFile = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
            if (null != appSettingsFile)
            {
                XmlNodeList ndList = appSettingsFile.SelectNodes("/ApplicationSettings/DisplayOptions/OverlayManager");
                if (0 >= ndList.Count)
                {
                    ndList = appSettingsFile.SelectNodes("/ApplicationSettings/DisplayOptions");
                    XmlManager.CreateXmlNodeWithinNode(appSettingsFile, ndList[0], "OverlayManager");
                    ndList = appSettingsFile.SelectNodes("/ApplicationSettings/DisplayOptions/OverlayManager");
                }

                if (null != ndList[0])
                {
                    string str = (true == _showLineLength) ? "1" : "0";
                    XmlManager.SetAttribute(ndList[0], appSettingsFile, "showLineLength", str);
                    str = (true == _showPolylineLength) ? "1" : "0";
                    XmlManager.SetAttribute(ndList[0], appSettingsFile, "showPolylineLength", str);
                    MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS);
                }
            }
            ResourceManagerCS.ReturnDocMutexCS(SettingsFileType.APPLICATION_SETTINGS);
        }

        private void PolylineToMask(ref short[] mask, int imgWidth, int imgHeight, ulong wkrIndex, Polyline roi)
        {
            try
            {
                int nVertices = 0;
                roi.Dispatcher.Invoke((Action)(() =>
                {
                    nVertices = roi.Points.Count;
                }));

                int[] verticesX = new int[nVertices];
                int[] verticesY = new int[nVertices];

                int[] tag = null;

                roi.Dispatcher.Invoke((Action)(() =>
                {
                    for (int i = 0; i < nVertices; i++)
                    {
                        verticesX[i] = Convert.ToInt32(Math.Floor(roi.Points[i].X));
                        verticesY[i] = Convert.ToInt32(Math.Floor(roi.Points[i].Y));
                    }
                    tag = (int[])((int[])(roi.Tag)).Clone();
                }));
                for (int i = 1; i < nVertices; i++)
                {
                    if (wkrIndex < _maskIndex)
                    {
                        return;
                    }
                    SingleLineToMask(ref mask, imgWidth, imgHeight, tag, verticesX[i - 1], verticesX[i], verticesY[i - 1], verticesY[i], wkrIndex);
                }
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + "PolylineToMask exception " + ex.Message);
            }
        }

        // generate a binary mask based on user defined polygon ROI (inside ROI->1, outside ROI->0)
        private void PolyToMask(ref short[] mask, int imgWidth, int imgHeight, ulong wkrIndex, ROIPoly roi)
        {
            try
            {
                int nVertices = 0;
                roi.Dispatcher.Invoke((Action)(() =>
                {
                    nVertices = roi.StatsPoints.Count;

                }));

                int[] verticesX = new int[nVertices];
                int[] verticesY = new int[nVertices];

                int[] tag = null;
                Rect boundingRect = new Rect();

                roi.Dispatcher.Invoke((Action)(() =>
                {
                    for (int i = 0; i < nVertices; i++)
                    {
                        verticesX[i] = Convert.ToInt32(Math.Floor(roi.StatsPoints[i].X));
                        verticesY[i] = Convert.ToInt32(Math.Floor(roi.StatsPoints[i].Y));
                    }
                    boundingRect = roi.StatsBounds;
                    tag = (int[])((int[])(roi.Tag)).Clone();
                }));

                int left = Convert.ToInt32(Math.Floor(boundingRect.Left));
                int top = Convert.ToInt32(Math.Floor(boundingRect.Top));
                int width = Convert.ToInt32(Math.Floor(boundingRect.Width));
                int height = Convert.ToInt32(Math.Floor(boundingRect.Height));

                System.Drawing.Bitmap b = new System.Drawing.Bitmap(imgWidth, imgHeight);

                using (System.Drawing.Graphics g = System.Drawing.Graphics.FromImage(b))
                {
                    g.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.None;
                    g.Clear(System.Drawing.Color.Black);

                    System.Drawing.Point[] ptArray = new System.Drawing.Point[nVertices];

                    for (int i = 0; i < nVertices; i++)
                    {
                        if (wkrIndex < _maskIndex)
                        {
                            return;
                        }
                        ptArray[i] = new System.Drawing.Point(verticesX[i], verticesY[i]);
                    }

                    g.FillPolygon(System.Drawing.Brushes.White, ptArray);
                }

                int maxY = top + height;
                if (maxY > imgHeight)
                {
                    maxY = imgHeight;
                }

                int maxX = left + width;
                if (maxX > imgWidth)
                {
                    maxX = imgWidth;
                }

                //fill inside of polygon
                for (int y = top; y < maxY; y++)
                {
                    for (int x = left; x < maxX; x++)
                    {
                        if (wkrIndex < _maskIndex)
                        {
                            return;
                        }
                        if (b.GetPixel(x, y).R > 0)
                        {
                            if ((mask.Length - 1) < (y * b.Width + x))
                            {
                                continue;
                            }
                            mask[y * b.Width + x] = Convert.ToInt16(tag[(int)Tag.SUB_PATTERN_ID]);
                        }
                    }
                }

                //fill side lines of polygon
                for (int i = 1; i < nVertices; i++)
                {
                    SingleLineToMask(ref mask, imgWidth, imgHeight, tag, verticesX[i - 1], verticesX[i], verticesY[i - 1], verticesY[i], wkrIndex);
                }
                SingleLineToMask(ref mask, imgWidth, imgHeight, tag, verticesX[0], verticesX[verticesX.Length - 1], verticesY[0], verticesY[verticesY.Length - 1], wkrIndex);
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + "PolyToMask exception " + ex.Message);
            }
        }

        private void RectToMask(ref short[] mask, int imgWidth, int imgHeight, ulong wkrIndex, ROIRect roi)
        {
            int roiTop = 0;
            int roiLeft = 0;
            int roiBottom = 0;
            int roiRight = 0;
            int[] tag = null;

            try
            {
                roi.Dispatcher.Invoke((Action)(() =>
                {
                    roiTop = Convert.ToInt32(Math.Floor(roi.StatsTopLeft.Y));
                    roiLeft = Convert.ToInt32(Math.Floor(roi.StatsTopLeft.X));
                    roiRight = Convert.ToInt32(Math.Floor(roi.StatsBottomRight.X));
                    roiBottom = Convert.ToInt32(Math.Floor(roi.StatsBottomRight.Y));
                    tag = (int[])((int[])(roi.Tag)).Clone();
                }));

                int roiHeight = Math.Abs(roiBottom - roiTop) + 1;
                int roiWidth = Math.Abs(roiRight - roiLeft) + 1;
                int offset = roiTop * imgWidth + roiLeft;

                for (int j = 0; j < roiHeight; j++)
                {
                    for (int i = 0; i < roiWidth; i++)
                    {
                        if (wkrIndex < _maskIndex)
                        {
                            return;
                        }
                        if (imgHeight * imgWidth <= (offset + i)) break;
                        if (imgWidth <= (roiLeft + i)) break;
                        mask[offset + i] = Convert.ToInt16(tag[(int)Tag.SUB_PATTERN_ID]);
                    }
                    offset += imgWidth;
                }
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + "RectToMask exception " + ex.Message);
            }
        }

        private bool RemoveAdornerFromROI(Shape roi, AdornerType adrType)
        {
            bool removed = false;
            _roiAdornerLayer = AdornerLayer.GetAdornerLayer(roi);
            if (null != _roiAdornerLayer)
            {
                Adorner[] adorners = _roiAdornerLayer.GetAdorners(roi);
                if (null != adorners)
                {
                    foreach (Adorner adorner in adorners)
                    {
                        switch (adrType)
                        {
                            case AdornerType.GEOMETRY_ADORNER:      //GeometryAdornerProvider
                                if (adorner is GeometryAdornerProvider)
                                {
                                    _roiAdornerLayer.Remove(adorner);
                                    removed = true;
                                }
                                break;
                            case AdornerType.ADJUST_ADORNER:        //AdornerProvider
                                if (adorner is AdornerProvider)
                                {
                                    _roiAdornerLayer.Remove(adorner);
                                    removed = true;
                                }
                                break;
                            case AdornerType.INDEX_ADORNER:         //IndexAdornerProvider
                                if (adorner is IndexAdornerProvider)
                                {
                                    _roiAdornerLayer.Remove(adorner);
                                    removed = true;
                                }
                                break;
                            case AdornerType.DIVIDER_ADORNER:
                                if (adorner is DividerAdornerProvider)
                                {
                                    _roiAdornerLayer.Remove(adorner);
                                    removed = true;
                                }
                                break;
                            default:
                                break;
                        }
                    }
                }
            }
            return removed;
        }

        private void ReorderROIs()
        {
            int[] tag = null;
            int[] id = new int[(int)Mode.LAST_MODE] { 1, 1, 1, 1 };
            Dictionary<int, int> patID = new Dictionary<int, int>();

            for (int i = 0; i < _roiList.Count; i++)
            {
                Mode mode = (Mode)(GetTagByteSection(_roiList[i].Tag, Tag.MODE, SecR));     //Mode: SecR of Tag.Mode
                switch (mode)
                {
                    case Mode.STATSONLY:
                    case Mode.MICRO_SCANAREA:
                        //reassign color and index:
                        _roiList[i].Stroke = GetROIColor((id[(int)mode] - 1) % 8);
                        tag = (int[])_roiList[i].Tag;
                        tag[(int)Tag.SUB_PATTERN_ID] = id[(int)mode];
                        _roiList[i].Tag = tag;
                        _roiList[i].Tag = SetTagRGB(_roiList[i]);
                        _roiList[i].ToolTip = "ROI #" + tag[(int)Tag.SUB_PATTERN_ID];
                        id[(int)mode]++;
                        break;
                    case Mode.PATTERN_NOSTATS:
                    case Mode.PATTERN_WIDEFIELD:
                        //add key of Pattern ID:
                        if (!patID.ContainsKey(((int[])_roiList[i].Tag)[(int)Tag.PATTERN_ID]))
                        {
                            patID.Add(((int[])_roiList[i].Tag)[(int)Tag.PATTERN_ID], 1);
                        }
                        //check sub-pattern ID as value:
                        if (patID[((int[])_roiList[i].Tag)[(int)Tag.PATTERN_ID]] <= ((int[])_roiList[i].Tag)[(int)Tag.SUB_PATTERN_ID])
                        {
                            tag = (int[])_roiList[i].Tag;
                            tag[(int)Tag.SUB_PATTERN_ID] = patID[((int[])_roiList[i].Tag)[(int)Tag.PATTERN_ID]];
                            _roiList[i].ToolTip = "ROI #" + tag[(int)Tag.SUB_PATTERN_ID];
                            patID[((int[])_roiList[i].Tag)[(int)Tag.PATTERN_ID]]++;
                        }
                        break;
                    default:
                        break;
                }
            }
        }

        private void ResizeObject(Point currentPoint, bool shiftDown)
        {
            if (_roiList?.Count > 0)
            {
                if (_activeType == typeof(ROIRect))
                {
                    ((ROIRect)_roiList[_roiList.Count - 1]).ShiftDown = shiftDown;
                    ((ROIRect)_roiList[_roiList.Count - 1]).EndPoint = currentPoint;
                }
                else if (typeof(ROIPoly) == _activeType)
                {
                }
                else if (typeof(ROILine) == _activeType)
                {
                    if (true == _isObjectComplete)
                    {
                        return;
                    }
                    (_roiList[_roiList.Count - 1] as ROILine).ShiftDown = shiftDown;
                    (_roiList[_roiList.Count - 1] as ROILine).EndPoint = currentPoint;
                    UpdateGeometryAdorner(_roiList.Last());
                }
                else if (typeof(Line) == _activeType)
                {
                    if (true == _isObjectComplete)
                    {
                        return;
                    }
                    if (false == shiftDown)
                    {
                        ((Line)_roiList[_roiList.Count - 1]).X2 = currentPoint.X;
                        ((Line)_roiList[_roiList.Count - 1]).Y2 = currentPoint.Y;
                    }
                    else
                    {
                        double x1 = ((Line)_roiList[_roiList.Count - 1]).X1;
                        double y1 = ((Line)_roiList[_roiList.Count - 1]).Y1;
                        if (Math.Abs(x1 - currentPoint.X) >= Math.Abs(y1 - currentPoint.Y))
                        {
                            ((Line)_roiList[_roiList.Count - 1]).X2 = currentPoint.X;
                            ((Line)_roiList[_roiList.Count - 1]).Y2 = ((Line)_roiList[_roiList.Count - 1]).Y1;
                        }
                        else
                        {
                            ((Line)_roiList[_roiList.Count - 1]).X2 = ((Line)_roiList[_roiList.Count - 1]).X1;
                            ((Line)_roiList[_roiList.Count - 1]).Y2 = currentPoint.Y; ;
                        }
                    }
                    UpdateGeometryAdorner(_roiList.Last());
                }
                else if ((typeof(ROIEllipse) == _activeType) && (typeof(ROIEllipse) == _roiList[_roiList.Count - 1].GetType()))
                {
                    ((ROIEllipse)_roiList[_roiList.Count - 1]).ShiftDown = shiftDown;

                    //allow add roi by left click only in Pattern_NoStats mode:
                    Shape tmp = new ROIEllipse();
                    if (((Mode.PATTERN_NOSTATS != _currentMode)) || (1 >= GetCurrentSubPatternCount(ref tmp)))
                    {
                        ((ROIEllipse)_roiList[_roiList.Count - 1]).EndPoint = currentPoint;

                        if (null != ObjectSizeChangedEvent && (SkipRedrawCenter ? 2 : 1) <= _roiList.Count)
                        {
                            ObjectSizeChangedEvent(((ROIEllipse)_roiList[_roiList.Count - (SkipRedrawCenter ? 2 : 1)]).ROIWidth,
                                ((ROIEllipse)_roiList[_roiList.Count - (SkipRedrawCenter ? 2 : 1)]).ROIHeight);
                        }
                    }
                    UpdateIndexAdorner(_roiList.Last());
                }
                _objectUpdated = true;
            }
        }

        private void ReticleToMask(ref short[] mask, int imgWidth, int imgHeight, Reticle roi)
        {
            try
            {
                int ptX = 0;
                int ptY = 0;
                int[] tag = null;
                roi.Dispatcher.Invoke((Action)(() =>
                {
                    ptX = Convert.ToInt32(Math.Floor(roi.CenterPoint.X));
                    ptY = Convert.ToInt32(Math.Floor(roi.CenterPoint.Y));
                    tag = (int[])((int[])(roi.Tag)).Clone();
                }));
                if (imgHeight < ptY || imgWidth < ptX) return;
                mask[ptY * imgWidth + ptX] = Convert.ToInt16(tag[(int)Tag.SUB_PATTERN_ID]);
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + "ReticleToMask exception " + ex.Message);
            }
        }

        private void ROILineToMask(ref short[] mask, int imgWidth, int imgHeight, ulong wkrIndex, ROILine roi)
        {
            try
            {
                int x1 = 0;
                int x2 = 0;
                int y1 = 0;
                int y2 = 0;
                int[] tag = null;

                roi.Dispatcher.Invoke((Action)(() =>
                {
                    x1 = Convert.ToInt32(Math.Floor(roi.StatsStartPoint.X));
                    x2 = Convert.ToInt32(Math.Floor(roi.StatsEndPoint.X));

                    y1 = Convert.ToInt32(Math.Floor(roi.StatsStartPoint.Y));
                    y2 = Convert.ToInt32(Math.Floor(roi.StatsEndPoint.Y));

                    tag = (int[])((int[])(roi.Tag)).Clone();
                }));
                SingleLineToMask(ref mask, imgWidth, imgHeight, tag, x1, x2, y1, y2, wkrIndex);
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + "LineToMask exception " + ex.Message);
            }
        }

        private void ROIPolylineToMask(ref short[] mask, int imgWidth, int imgHeight, ulong wkrIndex, ROIPolyline roi)
        {
            try
            {
                int nVertices = 0;
                roi.Dispatcher.Invoke((Action)(() =>
                {
                    nVertices = roi.StatsPoints.Count;
                }));

                int[] verticesX = new int[nVertices];
                int[] verticesY = new int[nVertices];

                int[] tag = null;

                roi.Dispatcher.Invoke((Action)(() =>
                {
                    for (int i = 0; i < nVertices; i++)
                    {
                        verticesX[i] = Convert.ToInt32(Math.Floor(roi.StatsPoints[i].X));
                        verticesY[i] = Convert.ToInt32(Math.Floor(roi.StatsPoints[i].Y));
                    }
                    tag = (int[])((int[])(roi.Tag)).Clone();
                }));
                for (int i = 1; i < nVertices; i++)
                {
                    if (wkrIndex < _maskIndex)
                    {
                        return;
                    }
                    SingleLineToMask(ref mask, imgWidth, imgHeight, tag, verticesX[i - 1], verticesX[i], verticesY[i - 1], verticesY[i], wkrIndex);
                }
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + "PolylineToMask exception " + ex.Message);
            }
        }

        void ROI_MouseDown(object sender, MouseButtonEventArgs e)
        {
            //limit user select roi:
            if ((Mode.STATSONLY == _currentMode || Mode.MICRO_SCANAREA == _currentMode) && (0 != ((int[])(sender as Shape).Tag)[(int)Tag.PATTERN_ID]) ||
                ((Mode.PATTERN_NOSTATS == _currentMode || Mode.PATTERN_WIDEFIELD == _currentMode) && (_patternID != ((int[])(sender as Shape).Tag)[(int)Tag.PATTERN_ID])))
                return;

            if (true == _isObjectComplete)
            {
                if (true == GetTagBit((sender as Shape).Tag, Tag.FLAGS, Flag.ADJUST_ADONERS))
                {
                    if (true == RemoveAdornerFromROI((sender as Shape), AdornerType.ADJUST_ADORNER))
                    {
                        int[] tag = SetTagBit((sender as Shape).Tag, Tag.FLAGS, Flag.ADJUST_ADONERS, false);
                        (sender as Shape).Tag = tag;
                    }

                    if (Mode.MICRO_SCANAREA == _currentMode)
                    {
                        mROISelectedEvent?.Invoke(0);
                    }
                }
                else
                {
                    if (Mode.MICRO_SCANAREA == _currentMode)
                    {
                        DeselectAllROIs();
                    }

                    AddPanningAdorners((sender as Shape));
                    int[] tag = SetTagBit((sender as Shape).Tag, Tag.FLAGS, Flag.ADJUST_ADONERS, true);
                    (sender as Shape).Tag = tag;

                    if (Mode.MICRO_SCANAREA == _currentMode)
                    {
                        int id = tag[(int)Tag.SUB_PATTERN_ID];
                        mROISelectedEvent?.Invoke(id);
                    }
                }

                int tagVal = GetTagValue((sender as Shape).Tag, Tag.ROI_ID);
                if (_roiList.Count <= tagVal || _roiList[tagVal] != sender)
                {
                    NeedOverlayItemsUpdateToActiveROIs?.Invoke(sender as Shape);
                }
            }
        }

        // save mask to raw file
        private void SaveMask(short[] arrayToSave)
        {
            string pathAndName;
            if (true == _saveEveryChange)
            {
                pathAndName = System.IO.Directory.GetParent(_expROIsPathAndName).ToString() + "\\ROIMask.raw";
            }
            else
            {
                pathAndName = Application.Current.Resources["TemplatesFolder"].ToString() + "\\ActiveROIMask.raw";
            }

            using (System.IO.BinaryWriter file = new System.IO.BinaryWriter(File.Open(pathAndName, FileMode.Create)))
            {
                foreach (short item in arrayToSave)
                {
                    file.Write(item);
                }
            }
        }

        /// <summary>
        /// Selects the input ROI
        /// </summary>
        /// <param name="roi"></param>
        private void SelectROI(Shape roi)
        {
            if (!IsROISelected(roi))
            {
                AddPanningAdorners(roi);
                int[] tag = SetTagBit(roi.Tag, Tag.FLAGS, Flag.ADJUST_ADONERS, true);
                roi.Tag = tag;
            }
        }

        /// <summary>
        /// Selects the ROI at the input index with in bound checking
        /// </summary>
        /// <param name="index"> The index of the ROI to be selected.</param>
        private void SelectROIIndexWithErrorChecking(int index)
        {
            if (0 <= index && index < _roiList.Count)
            {
                SelectROI(_roiList[index]);
            };
        }

        private int[] SetTagBit(object tag, Tag tagID, Flag location, bool value)
        {
            int[] localTag = new int[((int[])tag).Length];
            int byteLocation = (0 < (int)location) ? (int)Math.Pow(2, (int)location - 1) : (int)location;
            localTag = (int[])tag;
            if (null != localTag)
            {
                if ((0 <= tagID) && ((int)tagID < localTag.Length))
                {
                    BitVector32 bv = new BitVector32(localTag[(int)tagID]);
                    bv[BitVector32.CreateMask(byteLocation)] = value;
                    localTag[(int)tagID] = bv.Data;
                }
            }
            return localTag;
        }

        private int[] SetTagRGB(Shape roi)
        {
            int[] tag = (int[])roi.Tag;
            SolidColorBrush cBrush = roi.Stroke as SolidColorBrush;
            _bitVec32 = new BitVector32(cBrush.Color.R);
            _bitVec32[SecG] = cBrush.Color.G;
            _bitVec32[SecB] = cBrush.Color.B;
            tag[(int)Tag.RGB] = _bitVec32.Data;
            return tag;
        }

        private int[] SetTagStripeCount(Shape roi, int val)
        {
            int[] tag = (int[])roi.Tag;
            tag[(int)Tag.MROI_STRIPE_COUNT] = val;
            return tag;
        }

        private void SingleLineToMask(ref short[] mask, int imgWidth, int imgHeight, int[] roiTag, int x1, int x2, int y1, int y2, ulong wkrIndex)
        {
            double m = 0;
            if (x1 == x2 && y1 == y2)
            {
                mask[y1 * imgWidth + x1] = Convert.ToInt16(roiTag[(int)Tag.SUB_PATTERN_ID]);
                return;
            }
            else if (x1 == x2)
            {
                int tY1 = 0;
                int tY2 = 0;
                if (y1 < y2)
                {
                    tY1 = y1;
                    tY2 = y2;
                }
                else
                {
                    tY1 = y2;
                    tY2 = y1;
                }
                int offset = 0;
                for (int y = tY1; y <= tY2; y++)
                {
                    if (wkrIndex < _maskIndex)
                    {
                        return;
                    }
                    offset = imgWidth * y;
                    mask[offset + x1] = Convert.ToInt16(roiTag[(int)Tag.SUB_PATTERN_ID]);
                }
                return;
            }
            else
            {
                m = Convert.ToDouble((y2 - y1)) / (x2 - x1);
            }
            int roiWidth = Math.Abs(x2 - x1) + 1;
            int roiHeight = Math.Abs(y2 - y1) + 1;
            if (imgWidth < roiWidth) roiWidth = imgWidth;
            if (imgHeight < roiHeight) roiHeight = imgHeight;
            if (1 <= Math.Abs(m))
            {
                int tX1 = 0;
                int tY1 = 0;
                if (y1 < y2)
                {
                    tY1 = y1;
                    tX1 = x1;
                }
                else
                {
                    tY1 = y2;
                    tX1 = x2;
                }
                int x = 0;
                int offset = 0;
                for (int y = tY1; y < roiHeight + tY1; y++)
                {
                    if (wkrIndex < _maskIndex)
                    {
                        return;
                    }
                    x = Convert.ToInt32(Math.Round(Convert.ToDouble((y - tY1)) / m) + tX1);
                    offset = imgWidth * y;
                    if (imgHeight * imgWidth <= (offset + x)) break;
                    if (imgWidth <= (x)) continue;
                    if (imgHeight <= y) break;
                    mask[offset + x] = Convert.ToInt16(roiTag[(int)Tag.SUB_PATTERN_ID]);
                }
            }
            else
            {
                int tX1 = 0;
                int tY1 = 0;
                if (x1 < x2)
                {
                    tY1 = y1;
                    tX1 = x1;
                }
                else
                {
                    tY1 = y2;
                    tX1 = x2;
                }
                int offset = 0;
                int y = 0;
                for (int x = tX1; x < roiWidth + tX1; x++)
                {
                    if (wkrIndex < _maskIndex)
                    {
                        return;
                    }
                    y = Convert.ToInt32(m * (Convert.ToDouble(x - tX1)) + tY1);
                    offset = imgWidth * y;
                    if (imgHeight * imgWidth <= (offset + x)) continue;
                    if (imgWidth <= (x)) break;
                    if (imgHeight <= y) continue;
                    mask[offset + x] = Convert.ToInt16(roiTag[(int)Tag.SUB_PATTERN_ID]);

                }
            }
        }

        void UpdateDividerAdorner(Shape roi)
        {
            if (roi is ROIRect && ((int[])roi.Tag)[(int)Tag.MROI_STRIPE_COUNT] > -1)
            {
                RemoveAdornerFromROI(roi, AdornerType.DIVIDER_ADORNER);
                AddDividerAdorner(roi);
            }
        }

        void UpdateGeometryAdorner(Shape roi)
        {
            if ((roi is Line || roi is Polyline || roi is ROIPolyline || roi is ROILine) && (true == GetTagBit(roi.Tag, Tag.FLAGS, Flag.GEOMETRY_ADONERS)))
            {
                RemoveAdornerFromROI(roi, AdornerType.GEOMETRY_ADORNER);
                AddGeometryToROI(roi);
            }
        }

        void UpdateGeometryAdorners()
        {
            //Remove and re-Add GeometryAdornerProvider when UMPerPixel changed
            for (int i = 0; i < _roiList.Count; i++)
            {
                UpdateGeometryAdorner(_roiList[i]);
            }
        }

        void UpdateIndexAdorner(Shape roi)
        {
            if (!_simulatorMode)
            {
                RemoveAdornerFromROI(roi, AdornerType.INDEX_ADORNER);
                AddIndexToROI(roi);
            }
        }

        /// <summary>
        /// redraw panning adorner if selected
        /// </summary>
        /// <param name="roi"></param>
        void UpdatePanningAdorner(Shape roi)
        {
            if (true == GetTagBit(roi.Tag, Tag.FLAGS, Flag.ADJUST_ADONERS))
            {
                RemoveAdornerFromROI(roi, AdornerType.ADJUST_ADORNER);
                AddPanningAdorners(roi);
            }
        }

        void UpdateToolTip(Shape roi)
        {
            double zUM = 0.0;
            if ((int)Tag.Z_UM_INT < ((int[])roi.Tag).Count())
            {
                Double.TryParse(((int[])roi.Tag)[(int)Tag.Z_UM_INT].ToString() + "." + ((int[])roi.Tag)[(int)Tag.Z_UM_DEC].ToString(), out zUM);
            }
            if (Mode.PATTERN_NOSTATS == (Mode)(GetTagByteSection(((int[])roi.Tag), Tag.MODE, SecR)) ||
                Mode.PATTERN_WIDEFIELD == (Mode)(GetTagByteSection(((int[])roi.Tag), Tag.MODE, SecR)))
            {
                roi.ToolTip = ((0 == ((int[])roi.Tag)[(int)Tag.SUB_PATTERN_ID]) ? "Center" : "ROI #" + ((int[])roi.Tag)[(int)Tag.SUB_PATTERN_ID]) + "\nz=" + zUM + "um";
            }
            else
            {
                roi.ToolTip = "ROI #" + ((int[])roi.Tag)[(int)Tag.SUB_PATTERN_ID];
            }
        }

        /// <summary>
        /// update rois by visibility tags, and set stroke by color tags
        /// </summary>
        /// <param name="canvas"></param>
        private void UpdateVisibleROIs(bool fireUpdatedEvent = true)
        {
            _overlayItems.Clear();

            for (int i = 0; i < _roiList.Count; i++)
            {
                if (true == GetTagBit(_roiList[i].Tag, Tag.FLAGS, Flag.DISPLAY))
                {
                    _overlayItems.Add(_roiList[i]);
                    UpdateGeometryAdorner(_roiList[i]);
                    UpdateIndexAdorner(_roiList[i]);
                    UpdatePanningAdorner(_roiList[i]);
                    UpdateToolTip(_roiList[i]);
                    if ((Mode)(GetTagByteSection(_roiList[i].Tag, Tag.MODE, SecR)) == Mode.MICRO_SCANAREA && typeof(ROIRect) == _roiList[i].GetType())
                    {
                        UpdateDividerAdorner(_roiList[i]);
                    }
                }
            }

            if (fireUpdatedEvent)
            {
                ROIListUpdated?.Invoke();
            }
        }

        void _adornerProvider_UpdateLinePosition(int obj)
        {
            if (_lastLineIndex < _roiList.Count)
            {
                if (_roiList[_lastLineIndex] is Line line)
                {
                    int[] x = new int[2];
                    int[] y = new int[2];
                    line.Dispatcher.Invoke((Action)(() =>
                    {
                        x[0] = Convert.ToInt32(Math.Floor((line).X1));
                        y[0] = Convert.ToInt32(Math.Floor((line).Y1));
                        x[1] = Convert.ToInt32(Math.Floor((line).X2));
                        y[1] = Convert.ToInt32(Math.Floor((line).Y2));
                    }));

                    SetLineProfileLine(x, y, 2, 1);
                }
                else if (_roiList[_lastLineIndex] is Polyline polyline)
                {
                    int[] x = null;
                    int[] y = null;
                    polyline.Dispatcher.Invoke((Action)(() =>
                    {
                        x = new int[polyline.Points.Count];
                        y = new int[polyline.Points.Count];
                        for (int p = 0; p < polyline.Points.Count; p++)
                        {
                            x[p] = Convert.ToInt32(Math.Floor(polyline.Points[p].X));
                            y[p] = Convert.ToInt32(Math.Floor(polyline.Points[p].Y));
                        }
                    }));
                    SetLineProfileLine(x, y, x.Length, 1);
                }
                if (_roiList[_lastLineIndex] is ROILine roiLine)
                {
                    int[] x = new int[2];
                    int[] y = new int[2];
                    roiLine.Dispatcher.Invoke((Action)(() =>
                    {
                        x[0] = Convert.ToInt32(Math.Floor((roiLine).StatsStartPoint.X));
                        y[0] = Convert.ToInt32(Math.Floor((roiLine).StatsStartPoint.Y));
                        x[1] = Convert.ToInt32(Math.Floor((roiLine).StatsEndPoint.X));
                        y[1] = Convert.ToInt32(Math.Floor((roiLine).StatsEndPoint.Y));
                    }));

                    SetLineProfileLine(x, y, 2, 1);
                }
                else if (_roiList[_lastLineIndex] is ROIPolyline roiPolyline)
                {
                    int[] x = null;
                    int[] y = null;
                    roiPolyline.Dispatcher.Invoke((Action)(() =>
                    {
                        x = new int[roiPolyline.StatsPoints.Count];
                        y = new int[roiPolyline.StatsPoints.Count];
                        for (int p = 0; p < roiPolyline.StatsPoints.Count; p++)
                        {
                            x[p] = Convert.ToInt32(Math.Floor(roiPolyline.StatsPoints[p].X));
                            y[p] = Convert.ToInt32(Math.Floor(roiPolyline.StatsPoints[p].Y));
                        }
                    }));
                    SetLineProfileLine(x, y, x.Length, 1);
                }
                else
                {
                    int[] x = new int[0];
                    int[] y = new int[0];
                    SetLineProfileLine(x, y, 0, 0);
                }
            }
        }

        void _adornerProvider_UpdateNow(Shape roi)
        {
            CreateMask();
            UpdateGeometryAdorner(roi);
            UpdateIndexAdorner(roi);
            if (null != ObjectSizeChangedEvent)
            {
                if (typeof(ROIEllipse) == roi.GetType())
                    ObjectSizeChangedEvent((roi as ROIEllipse).ROIWidth, (roi as ROIEllipse).ROIHeight);
                else if (typeof(ROIRect) == roi.GetType())
                    ObjectSizeChangedEvent((roi as ROIRect).ROIWidth, (roi as ROIRect).ROIHeight);
            }
            //only update with non-center movement
            if (Mode.PATTERN_NOSTATS == _currentMode && _patternID == ((int[])roi.Tag)[(int)Tag.PATTERN_ID] && 0 != ((int[])roi.Tag)[(int)Tag.SUB_PATTERN_ID])
            {
                ValidateROIs();
            }

            UpdateVisibleROIs();

            if (_currentMode == Mode.MICRO_SCANAREA)
            {
                mROIsUpdated?.Invoke();
            }
        }

        void AdornerProvider_RectangleMoved(Shape roi)
        {
            Adorner[] adorners = AdornerLayer.GetAdornerLayer(roi)?.GetAdorners(roi);
            if (adorners == null)
            {
                return;
            }
            foreach (Adorner adorner in adorners)
            {
                if (adorner is DividerAdornerProvider)
                {
                    adorner.InvalidateArrange();
                }
            }
        }

        #endregion Methods
    }

    public class ROICapsule
    {
        #region Properties

        public Shape[] ROIs
        {
            get;
            set;
        }

        /// <summary>
        /// [0]RecBound[Width,Height]
        /// </summary>
        public Shape[] ROIspec
        {
            get;
            set;
        }

        public Shape[] ScanAreaROIs
        {
            get;
            set;
        }

        #endregion Properties
    }
}