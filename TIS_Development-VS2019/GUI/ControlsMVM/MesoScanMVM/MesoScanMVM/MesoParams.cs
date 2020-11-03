namespace MesoScan.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Media;
    using System.Windows.Media.Media3D;

    using ThorSharedTypes;

    #region Enumerations

    public enum ResUnit
    {
        None, Inch, Centimeter, Millimetre, Micron, Nanometer, Picometer
    }

    public enum ShapeTypes
    {
        Rectangle = 0, Ellipse, PolyLine, Polygon
    }

    #endregion Enumerations

    public class ActionMessage
    {
        #region Fields

        public string _actionName; // can be create or delete or any attribute name
        public long _scanAreaId;
        public long _scanInfoId;
        public double _zposition;

        #endregion Fields

        #region Constructors

        public ActionMessage(string actionName = "", int scanInfoId = -1, int scanAreaId = -1, double zPosition = -1)
        {
            _scanInfoId = scanInfoId;
            _scanAreaId = scanAreaId;
            _zposition = zPosition;
            _actionName = actionName;
        }

        #endregion Constructors
    }

    public class MesoParams
    {
        #region Fields

        public List<ScanInfo> _templateScans = new List<ScanInfo>();

        #endregion Fields

        #region Constructors

        public MesoParams()
        {
            ScanInfo meso = new ScanInfo();
            meso._name = Enum.GetName(typeof(MesoScanTypes), MesoScanTypes.Meso);
            _templateScans.Add(meso);
            ScanInfo micro = new ScanInfo();
            micro._name = Enum.GetName(typeof(MesoScanTypes), MesoScanTypes.Micro);
            _templateScans.Add(micro);
        }

        #endregion Constructors
    }

    public class PowerBox
    {
        #region Fields

        public double _endZ = 0;
        public double _powerPercentage = 100;
        public Roi _powerRoi = new Roi();
        public double _startZ = 0;

        #endregion Fields
    }

    public class PowerPoint
    {
        #region Fields

        public double _powerPercentage = 100;
        public double _zPosition = 0;

        #endregion Fields
    }

    public class Roi
    {
        #region Fields

        public Rect _bound = new Rect();
        public List<Point> _points = new List<Point>();
        public int _roiID = 0;
        public ShapeTypes _type = ShapeTypes.Rectangle;

        #endregion Fields
    }

    public class ScanArea
    {
        #region Fields

        public Color _color = (Color)ColorConverter.ConvertFromString("#00000000");
        public double _currentZPosition = 0;
        public bool _isActionable = false;
        public bool _isEnable = false;
        public string _name = "";
        public Point3D _physicalSize = new Point3D(0, 0, 0);
        public Point3D _position = new Point3D(0, 0, 0);
        public List<PowerBox> _powerBoxs = new List<PowerBox>();
        public List<PowerPoint> _powerPoints = new List<PowerPoint>();
        public long _scanAreaId = 0;
        public Point3D _size = new Point3D(0, 0, 0);
        public double _sizeS = 0;
        public double _sizeT = 0;

        #endregion Fields

        #region Constructors

        public ScanArea()
        {
            PowerPoint p1 = new PowerPoint();
            p1._zPosition = 0;
            p1._powerPercentage = 100;
            _powerPoints.Add(p1);
            PowerPoint p2 = new PowerPoint();
            p2._zPosition = 1000;
            p2._powerPercentage = 100;
            _powerPoints.Add(p2);
        }

        #endregion Constructors
    }

    public class ScanConfigure
    {
        #region Fields

        public int _averageMode = 0;
        public int _currentPower = 10;
        public int _isLivingMode = 0;
        public int _numberOfAverageFrame = 1;
        public int _physicalFieldSize = 600;
        public int _remapShift = 0;
        public int _scanMode = 1;
        public int _stripLength = 256;

        #endregion Fields
    }

    public class ScanInfo
    {
        #region Fields

        public bool _hasStored = false;
        public TypeCode _iPixelType = TypeCode.UInt32;
        public string _name = "";
        public string _objectiveType = "";
        public Point3D _pixelSize = new Point3D(0, 0, 0);
        public ResUnit _resUnit = ResUnit.Micron;
        public List<ScanArea> _scanAreas = new List<ScanArea>();
        public ScanConfigure _scanConfigure = new ScanConfigure();
        public int _scanId = 0;
        public long _significantBits = 1;
        public long _tileHeight = 0;
        public long _tileWidth = 0;
        public double _timeInterval = 0;

        #endregion Fields

        #region Constructors

        public ScanInfo()
        {
            ScanArea isa0 = new ScanArea();
            isa0._name = "ISA0";
            _scanAreas.Add(isa0);
        }

        #endregion Constructors
    }
}