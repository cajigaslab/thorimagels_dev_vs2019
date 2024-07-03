//namespace MesoScan.Params
//{
//    using System;
//    using System.Collections.Generic;
//    using System.Linq;
//    using System.Text;
//    using System.Threading.Tasks;
//    using System.Windows;
//    using System.Windows.Media;
//    using System.Windows.Media.Media3D;

//    using ThorSharedTypes;

//    #region Enumerations

//    public enum ResUnit
//    {
//        None, Inch, Centimeter, Millimetre, Micron, Nanometer, Picometer
//    }

//    public enum ShapeTypes
//    {
//        Rectangle = 0, Ellipse, PolyLine, Polygon
//    }

//    #endregion Enumerations

//    public class ActionMessage
//    {
//        #region Fields

//        public string _actionName; // can be create or delete or any attribute name
//        public long _scanAreaId;
//        public long _scanInfoId;
//        public double _zposition;

//        #endregion Fields

//        #region Constructors

//        public ActionMessage(string actionName = "", int scanInfoId = -1, int scanAreaId = -1, double zPosition = -1)
//        {
//            _scanInfoId = scanInfoId;
//            _scanAreaId = scanAreaId;
//            _zposition = zPosition;
//            _actionName = actionName;
//        }

//        #endregion Constructors
//    }

//    public class MesoParams
//    {
//        #region Fields

//        public List<ScanInfo> TemplateScans = new List<ScanInfo>();

//        #endregion Fields

//        #region Constructors

//        public MesoParams()
//        {
//            ScanInfo meso = new ScanInfo();
//            meso.Name = Enum.GetName(typeof(MesoScanTypes), MesoScanTypes.Meso);
//            TemplateScans.Add(meso);
//            ScanInfo micro = new ScanInfo();
//            micro.Name = Enum.GetName(typeof(MesoScanTypes), MesoScanTypes.Micro);
//            TemplateScans.Add(micro);
//        }

//        #endregion Constructors
//    }

//    public class PowerBox
//    {
//        #region Fields

//        public double _endZ = 0;
//        public double _powerPercentage = 100;
//        public Roi _powerRoi = new Roi();
//        public double _startZ = 0;

//        #endregion Fields
//    }

//    public class PowerPoint
//    {
//        #region Fields

//        public double PowerPercentage0 = 100;
//        public double ZPosition = 0;

//        #endregion Fields
//    }

//    public class Roi
//    {
//        #region Fields

//        public Rect _bound = new Rect();
//        public List<Point> _points = new List<Point>();
//        public int _roiID = 0;
//        public ShapeTypes _type = ShapeTypes.Rectangle;

//        #endregion Fields
//    }

//    public class ScanArea
//    {
//        #region Fields

//        public ScanArea()
//        {
//            PowerPoints = new List<PowerPoint>();
//            PowerPoint p1 = new PowerPoint();
//            p1.ZPosition = 0;
//            p1.PowerPercentage0 = 100;
//            PowerPoints.Add(p1);
//        }
//        public int ScanAreaID
//        {
//            get; set;
//        }

//        public string Name
//        {
//            get; set;
//        }

//        public Color Color
//        {
//            get; set;
//        }
//        public double PhysicalSizeXUM
//        {
//            get; set;
//        }

//        public double PhysicalSizeYUM
//        {
//            get; set;
//        }

//        public double PhysicalSizeZ
//        {
//            get; set;
//        }

//        public double PositionXUM
//        {
//            get; set;
//        }

//        public double PositionYUM
//        {
//            get; set;
//        }

//        public double PositionZ
//        {
//            get; set;
//        }

//        public double PositionXFieldOffset
//        {
//            get; set;
//        }

//        public double PositionYField
//        {
//            get; set;
//        }

//        public double ZPosition
//        {
//            get
//            {
//                if (PowerPoints?.Count > 0 && PowerPoints[0] != null)
//                {
//                    return PowerPoints[0].ZPosition;
//                }
//                return 0;
//            }
//            set
//            {
//                if (PowerPoints?.Count > 0 && PowerPoints[0] != null)
//                {
//                    PowerPoints[0].ZPosition = value;
//                }
//                else
//                {
//                    PowerPoints = new List<PowerPoint>
//                    {
//                        new PowerPoint() { ZPosition = value }
//                    };
//                }
//            }
//        }

//        public List<PowerBox> _powerBoxs = new List<PowerBox>();
//        public double Power0
//        {
//            get
//            {
//                if (PowerPoints?.Count > 0 && PowerPoints[0] != null)
//                {
//                    return PowerPoints[0].PowerPercentage0;
//                }
//                return 0;
//            }
//            set
//            {
//                if (PowerPoints?.Count > 0 && PowerPoints[0] != null)
//                {
//                    PowerPoints[0].PowerPercentage0 = value;
//                }
//                else
//                {
//                    PowerPoints = new List<PowerPoint>
//                    {
//                        new PowerPoint() { PowerPercentage0 = value }
//                    };
//                }
//            }
//        }

//        public double Power1
//        {
//            get; set;
//        }

//        public double Power2
//        {
//            get; set;
//        }

//        public double Power3
//        {
//            get; set;
//        }

//        public int AreaFieldSize
//        {
//            get; set;
//        }

//        public int SizeXPixels
//        {
//            get; set;
//        }

//        public int SizeYPixels
//        {
//            get; set;
//        }

//        public bool IsEnable
//        {
//            get; set;
//        }

//        public int PixelSizeX
//        {
//            get; set;
//        }

//        public int PixelSizeY
//        {
//            get; set;
//        }

//        public List<PowerPoint> PowerPoints
//        {
//            get; set;
//        }

//        public int FlyToNextAreaLineCycles
//        {
//            get; set;
//        }



//        //  public Color Color = (Color)ColorConverter.ConvertFromString("#00000000");
//        ////  public double _currentZPosition = 0;
//        //  //public bool _isActionable = false;
//        //  public bool IsEnable = false;
//        //  public string Name = "";
//        //  public Point3D _physicalSize = new Point3D(0, 0, 0);
//        //  public Point3D _position = new Point3D(0, 0, 0);
//        //  //public List<PowerBox> _powerBoxs = new List<PowerBox>();
//        //public List<PowerPoint> PowerPoints = new List<PowerPoint>();


//        //  public long ScanAreaId = 0;
//        //public Point3D _size = new Point3D(0, 0, 0);
//        //public double _sizeS = 0;
//        //public double _sizeT = 0;

//        #endregion Fields

//        #region Constructors

//        //public ScanArea()
//        //{
//        //    //PowerPoint p1 = new PowerPoint();
//        //    //p1.ZPosition = 0;
//        //    //p1.PowerPercentage0 = 100;
//        //    ////PowerPoints.Add(p1);
//        //    //PowerPoint p2 = new PowerPoint();
//        //    //p2.ZPosition = 1000;
//        //    //p2.PowerPercentage0 = 100;
//        //    ////PowerPoints.Add(p2);
//        //}

//        #endregion Constructors
//    }

//    public class ScanConfigure
//    {
//        #region Fields

//        public int _averageMode = 0;
//        public double _currentPower = 10;
//        public int _isLivingMode = 0;
//        public int _numberOfAverageFrame = 1;
//        public double _physicalFieldSize = 600;
//        public int _remapShift = 0;
//        public int _scanMode = 1;
//        public int _stripLength = 256;

//        #endregion Fields
//    }

//    public class ScanInfo
//    {
//        #region Fields

//        public bool HasStored = false;
//        public TypeCode iPixelType = TypeCode.UInt32;
//        public string Name = "";
//        public string ObjectiveType = "";
//        public Point3D PixelSize = new Point3D(0, 0, 0);
//        public ResUnit ResUnit = ResUnit.Micron;
//        public List<ScanArea> ScanAreas = new List<ScanArea>();
//        public ScanConfigure ScanConfigure = new ScanConfigure();
//        public int ScanID = 0;
//        public long SignificantBits = 1;
//        public long TileHeight = 0;
//        public long TileWidth = 0;
//        public double TimeInterval = 0;

//        #endregion Fields

//        #region Constructors

//        public ScanInfo()
//        {
//            ScanArea isa0 = new ScanArea();
//            isa0.Name = "ISA0";
//            ScanAreas.Add(isa0);
//        }

//        #endregion Constructors
//    }
//}