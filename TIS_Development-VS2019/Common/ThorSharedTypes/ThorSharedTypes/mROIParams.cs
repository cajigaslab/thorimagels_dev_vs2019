namespace MesoScan.Params
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Collections.Specialized;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Media;
    using System.Windows.Media.Media3D;
    using System.Xml;
    using System.Xml.Linq;

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

    public struct FULLFOVMetadata
    {
        #region Fields

        public double HeightUM;
        public int PixelHeight;
        public int PixelWidth;
        public double WidthUM;

        #endregion Fields
    }

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

        public List<ScanInfo> TemplateScans = new List<ScanInfo>();

        #endregion Fields

        #region Constructors

        public MesoParams()
        {
            ScanInfo meso = new ScanInfo();
            meso.Name = Enum.GetName(typeof(MesoScanTypes), MesoScanTypes.Meso);
            TemplateScans.Add(meso);
            ScanInfo micro = new ScanInfo();
            micro.Name = Enum.GetName(typeof(MesoScanTypes), MesoScanTypes.Micro);
            TemplateScans.Add(micro);
        }

        #endregion Constructors
    }

    public static class mROIXMLMapper
    {
        #region Methods

        public static bool MapXml2mROIParams(XmlDocument activeSettings, ICamera.LSMType lsmType, out ObservableCollection<ScanArea> mROIs, out double mROIPixelSizeXUM, out double mROIPixelSizeYUM, out double mROIStripLength, out FULLFOVMetadata fullFOVMetadata)
        {
            mROIPixelSizeXUM = 0;
            mROIPixelSizeYUM = 0;
            mROIStripLength = 1;
            mROIs = new ObservableCollection<ScanArea>();
            fullFOVMetadata = new FULLFOVMetadata();

            if (lsmType != ICamera.LSMType.RESONANCE_GALVO_GALVO)
            {
                return false;
            }
            var doc = XmlManager.ToXDocument(activeSettings);
            if (doc == null)
            {
                return false;
            }
            XElement q = doc.Root.Element("TemplateScans");
            if (q == null)
            {
                return false;
            }

            foreach (var scanInfo in q.Elements("ScanInfo"))
            {
                ScanInfo tScanInfo = new ScanInfo();
                tScanInfo.ScanID = Convert.ToInt32(scanInfo.Attribute("ScanID").Value);

                if (tScanInfo.ScanID == (int)MesoScanTypes.Meso)
                {
                    tScanInfo.Name = scanInfo.Attribute("Name").Value;
                    tScanInfo.ObjectiveType = scanInfo.Attribute("ObjectiveType").Value;
                    tScanInfo.PixelSize.X = Convert.ToDouble(scanInfo.Attribute("XPixelSize").Value);
                    tScanInfo.PixelSize.Y = Convert.ToDouble(scanInfo.Attribute("YPixelSize").Value);
                    tScanInfo.PixelSize.Z = Convert.ToDouble(scanInfo.Attribute("ZPixelSize").Value);
                    tScanInfo.iPixelType = scanInfo.Attribute("IPixelType").ToString().GetTypeCode();
                    tScanInfo.ResUnit = (ResUnit)Enum.Parse(typeof(ResUnit), scanInfo.Attribute("ResUnit").Value, true);
                    tScanInfo.TileWidth = Convert.ToInt32(scanInfo.Attribute("TileWidth").Value);
                    tScanInfo.TileHeight = Convert.ToInt32(scanInfo.Attribute("TileHeight").Value);
                    tScanInfo.TimeInterval = Convert.ToDouble(scanInfo.Attribute("TimeInterval").Value);
                    tScanInfo.SignificantBits = Convert.ToInt32(scanInfo.Attribute("SignificantBits").Value);
                    tScanInfo.HasStored = Convert.ToBoolean(scanInfo.Attribute("HasStored").Value);
                    var scanConfigure = scanInfo.Element("ScanConfigure");
                    double stripeSize = Convert.ToDouble(scanConfigure.Attribute("PhysicalFieldSize").Value);
                    var scanAreas = scanInfo.Element("ScanAreas");

                    foreach (var scanArea in scanAreas.Elements("ScanArea"))
                    {
                        fullFOVMetadata.WidthUM = Convert.ToDouble(scanArea.Attribute("PhysicalSizeX").Value);
                        fullFOVMetadata.HeightUM = fullFOVMetadata.WidthUM;
                        break;
                    }
                }
                else if (tScanInfo.ScanID == (int)MesoScanTypes.Micro)
                {
                    tScanInfo.Name = scanInfo.Attribute("Name").Value;
                    tScanInfo.ObjectiveType = scanInfo.Attribute("ObjectiveType").Value;
                    tScanInfo.PixelSize.X = Convert.ToDouble(scanInfo.Attribute("XPixelSize").Value);
                    mROIPixelSizeXUM = tScanInfo.PixelSize.X;
                    tScanInfo.PixelSize.Y = Convert.ToDouble(scanInfo.Attribute("YPixelSize").Value);
                    mROIPixelSizeYUM = tScanInfo.PixelSize.Y;
                    tScanInfo.PixelSize.Z = Convert.ToDouble(scanInfo.Attribute("ZPixelSize").Value);
                    tScanInfo.iPixelType = scanInfo.Attribute("IPixelType").ToString().GetTypeCode();
                    tScanInfo.ResUnit = (ResUnit)Enum.Parse(typeof(ResUnit), scanInfo.Attribute("ResUnit").Value, true);
                    tScanInfo.TileWidth = Convert.ToInt32(scanInfo.Attribute("TileWidth").Value);
                    tScanInfo.TileHeight = Convert.ToInt32(scanInfo.Attribute("TileHeight").Value);
                    tScanInfo.TimeInterval = Convert.ToDouble(scanInfo.Attribute("TimeInterval").Value);
                    tScanInfo.SignificantBits = Convert.ToInt32(scanInfo.Attribute("SignificantBits").Value);
                    tScanInfo.HasStored = Convert.ToBoolean(scanInfo.Attribute("HasStored").Value);
                    var scanConfigure = scanInfo.Element("ScanConfigure");
                    double stripeSize = Convert.ToDouble(scanConfigure.Attribute("PhysicalFieldSize").Value);
                    mROIStripLength = Convert.ToDouble(scanConfigure.Attribute("StripLength").Value);

                    var scanAreas = scanInfo.Element("ScanAreas");

                    fullFOVMetadata.PixelWidth = (int)Math.Floor(fullFOVMetadata.WidthUM / tScanInfo.PixelSize.X);
                    fullFOVMetadata.PixelWidth = (fullFOVMetadata.PixelWidth % 2) != 0 ? fullFOVMetadata.PixelWidth + 1 : fullFOVMetadata.PixelWidth;
                    fullFOVMetadata.PixelHeight = (int)Math.Floor(fullFOVMetadata.WidthUM / tScanInfo.PixelSize.X);
                    fullFOVMetadata.PixelHeight = (fullFOVMetadata.PixelHeight % 2) != 0 ? fullFOVMetadata.PixelHeight + 1 : fullFOVMetadata.PixelHeight;
                    foreach (var scanArea in scanAreas.Elements("ScanArea"))
                    {
                        ScanArea tScanArea = new ScanArea();
                        tScanArea.ScanAreaID = Convert.ToInt32(scanArea.Attribute("ScanAreaID").Value);
                        tScanArea.Name = scanArea.Attribute("Name").Value;
                      //  tScanArea.Color = (Color)ColorConverter.ConvertFromString(scanArea.Attribute("Color").Value);
                        tScanArea.PhysicalSizeXUM = Convert.ToDouble(scanArea.Attribute("PhysicalSizeX").Value);
                        int nStripes = (int)Math.Round(tScanArea.PhysicalSizeXUM / stripeSize);
                        tScanArea.Stripes = nStripes;
                        tScanArea.PhysicalSizeYUM = Convert.ToDouble(scanArea.Attribute("PhysicalSizeY").Value);
                        tScanArea.PhysicalSizeZ = Convert.ToDouble(scanArea.Attribute("PhysicalSizeZ").Value);
                        tScanArea.PositionXUM = Convert.ToDouble(scanArea.Attribute("PositionX").Value);
                        tScanArea.PositionYUM = Convert.ToDouble(scanArea.Attribute("PositionY").Value);
                        tScanArea.PositionZ = Convert.ToDouble(scanArea.Attribute("PositionZ").Value);
                        tScanArea.SizeXPixels = (int)Convert.ToDouble(scanArea.Attribute("SizeX").Value);
                        tScanArea.SizeYPixels = (int)Convert.ToDouble(scanArea.Attribute("SizeY").Value);
                        tScanArea.IsEnable = Convert.ToBoolean(scanArea.Attribute("IsEnable").Value);
                        var powerPoints = scanArea.Element("PowerPoints");
                        foreach (var powerPoint in powerPoints.Elements("PowerPoint"))
                        {
                            PowerPoint tPowerPoint = new PowerPoint();
                            tPowerPoint.ZPosition = Convert.ToDouble(powerPoint.Attribute("ZPosition").Value);
                            tPowerPoint.PowerPercentage0 = Convert.ToDouble(powerPoint.Attribute("PowerPercentage").Value);
                            tScanArea.PowerPoints.Add(tPowerPoint);
                            tScanArea.ZPosition = Convert.ToDouble(powerPoint.Attribute("ZPosition").Value);
                            tScanArea.Power0 = Convert.ToDouble(powerPoint.Attribute("PowerPercentage").Value);
                        }
                        if (tScanArea.IsEnable)
                        {
                            mROIs.Add(tScanArea);
                        }
                    }
                }
            }

            return mROIs?.Count > 0;
        }

        #endregion Methods
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

        public double PowerPercentage0 = 100;
        public double ZPosition = 0;

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

    public class ScanArea : VMBase
    {
        #region Fields

        public int _flyToNextAreaLineCycles;
        public List<PowerBox> _powerBoxs = new List<PowerBox>();
        public int _stripes = 1;

        int _areaFieldSize;
        bool _isEnable;
        string _name;

        //Color _color;
        //public Color Color
        //{
        //    get => _color;
        //    set => SetProperty(ref _color, value);
        //}
        double _physicalSizeXUM;
        double _physicalSizeYUM;
        double _physicalSizeZ;
        double _positionXFieldOffset;
        double _positionXUM;
        double _positionYField;
        double _positionYUM;
        double _positionZ;
        List<PowerPoint> _powerPoints;
        int _scanAreaID;
        int _sizeXPixels;
        int _sizeYPixels;

        #endregion Fields

        #region Constructors

        public ScanArea()
        {
            PowerPoints = new List<PowerPoint>();
            PowerPoint p1 = new PowerPoint();
            p1.ZPosition = 0;
            p1.PowerPercentage0 = 100;
            PowerPoints.Add(p1);
        }

        #endregion Constructors

        #region Properties

        public int AreaFieldSize
        {
            get => _areaFieldSize;
            set => SetProperty(ref _areaFieldSize, value);
        }

        public Brush Color
        {
            get =>
                GetROIColor(_scanAreaID - 1);
            set
            {
                var x = value; //dummy
            }
        }

        public int FlyToNextAreaLineCycles
        {
            get => _flyToNextAreaLineCycles; set => SetProperty(ref _flyToNextAreaLineCycles, value);
        }

        public bool IsEnable
        {
            get => _isEnable;
            set => SetProperty(ref _isEnable, value);
        }

        //public double Power1
        //{
        //    get => _scanAreaID;
        //    set => SetProperty(ref _scanAreaID, value);
        //}
        //public double Power2
        //{
        //    get => _scanAreaID;
        //    set => SetProperty(ref _scanAreaID, value);
        //}
        //public double Power3
        //{
        //    get => _scanAreaID;
        //    set => SetProperty(ref _scanAreaID, value);
        //}
        public double LeftUM
        {
            get
            {
                return _positionXUM - _physicalSizeXUM / 2 / _stripes;
            }
        }

        public string Name
        {
            get => _name;
            set => SetProperty(ref _name, value);
        }

        public double PhysicalSizeXUM
        {
            get => _physicalSizeXUM;
            set => SetProperty(ref _physicalSizeXUM, value);
        }

        public double PhysicalSizeYUM
        {
            get => _physicalSizeYUM;
            set => SetProperty(ref _physicalSizeYUM, value);
        }

        public double PhysicalSizeZ
        {
            get => _physicalSizeZ;
            set => SetProperty(ref _physicalSizeZ, value);
        }

        public double PositionXFieldOffset
        {
            get => _positionXFieldOffset;
            set => SetProperty(ref _positionXFieldOffset, value);
        }

        public double PositionXUM
        {
            get => _positionXUM;
            set
            {
                SetProperty(ref _positionXUM, value);
                OnPropertyChanged("LeftUM");
            }
        }

        public double PositionYField
        {
            get => _positionYField;
            set => SetProperty(ref _positionYField, value);
        }

        public double PositionYUM
        {
            get => _positionYUM;
            set
            {
                SetProperty(ref _positionYUM, value);
                OnPropertyChanged("TopUM");
            }
        }

        public double PositionZ
        {
            get => _positionZ;
            set => SetProperty(ref _positionZ, value);
        }

        public double Power0
        {
            get
            {
                if (PowerPoints?.Count > 0 && PowerPoints[0] != null)
                {
                    return Math.Round(PowerPoints[0].PowerPercentage0, 1);
                }
                return 0;
            }
            set
            {
                if (PowerPoints?.Count > 0 && PowerPoints[0] != null)
                {
                    PowerPoints[0].PowerPercentage0 = value;
                }
                else
                {
                    PowerPoints = new List<PowerPoint>
                    {
                        new PowerPoint() { PowerPercentage0 = value }
                    };
                }
                OnPropertyChanged();
            }
        }

        public List<PowerPoint> PowerPoints
        {
            get => _powerPoints;
            set => SetProperty(ref _powerPoints, value);
        }

        public int ScanAreaID
        {
            get => _scanAreaID;
            set => SetProperty(ref _scanAreaID, value);
        }

        public int SizeXPixels
        {
            get => _sizeXPixels; set => SetProperty(ref _sizeXPixels, value);
        }

        public int SizeYPixels
        {
            get => _sizeYPixels; set => SetProperty(ref _sizeYPixels, value);
        }

        public int Stripes
        {
            get => _stripes; set => SetProperty(ref _stripes, value);
        }

        public double TopUM
        {
            get
            {
                return _positionYUM;
            }
        }

        public double ZPosition
        {
            get
            {
                if (PowerPoints?.Count > 0 && PowerPoints[0] != null)
                {
                    return PowerPoints[0].ZPosition;
                }
                return 0;
            }
            set
            {
                if (PowerPoints?.Count > 0 && PowerPoints[0] != null)
                {
                    PowerPoints[0].ZPosition = value;
                }
                else
                {
                    PowerPoints = new List<PowerPoint>
                    {
                        new PowerPoint() { ZPosition = value }
                    };
                }
                OnPropertyChanged();
            }
        }

        #endregion Properties

        #region Methods

        private Brush GetROIColor(int colorIndex)
        {
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

        #endregion Methods

        #region Other

        //  public Color Color = (Color)ColorConverter.ConvertFromString("#00000000");
        ////  public double _currentZPosition = 0;
        //  //public bool _isActionable = false;
        //  public bool IsEnable = false;
        //  public string Name = "";
        //  public Point3D _physicalSize = new Point3D(0, 0, 0);
        //  public Point3D _position = new Point3D(0, 0, 0);
        //  //public List<PowerBox> _powerBoxs = new List<PowerBox>();
        //public List<PowerPoint> PowerPoints = new List<PowerPoint>();
        //  public long ScanAreaId = 0;
        //public Point3D _size = new Point3D(0, 0, 0);
        //public double _sizeS = 0;
        //public double _sizeT = 0;
        //public ScanArea()
        //{
        //    //PowerPoint p1 = new PowerPoint();
        //    //p1.ZPosition = 0;
        //    //p1.PowerPercentage0 = 100;
        //    ////PowerPoints.Add(p1);
        //    //PowerPoint p2 = new PowerPoint();
        //    //p2.ZPosition = 1000;
        //    //p2.PowerPercentage0 = 100;
        //    ////PowerPoints.Add(p2);
        //}

        #endregion Other
    }

    public class ScanConfigure
    {
        #region Fields

        public int _averageMode = 0;
        public double _currentPower = 10;
        public int _isLivingMode = 0;
        public int _numberOfAverageFrame = 1;
        public double _physicalFieldSize = 600;
        public int _remapShift = 0;
        public int _scanMode = 1;
        public int _stripeFieldSize = 256;
        public int _stripLength = 256;

        #endregion Fields
    }

    public class ScanInfo
    {
        #region Fields

        public bool HasStored = false;
        public TypeCode iPixelType = TypeCode.UInt32;
        public string Name = "";
        public string ObjectiveType = "";
        public Point3D PixelSize = new Point3D(0, 0, 0);
        public ResUnit ResUnit = ResUnit.Micron;
        public ObservableCollection<ScanArea> ScanAreas = new ObservableCollection<ScanArea>();
        public ScanConfigure ScanConfigure = new ScanConfigure();
        public int ScanID = 0;
        public long SignificantBits = 1;
        public long TileHeight = 0;
        public long TileWidth = 0;
        public double TimeInterval = 0;

        #endregion Fields

        #region Constructors

        public ScanInfo()
        {
            //ScanArea isa0 = new ScanArea();
            //isa0.Name = "ISA0";
            //ScanAreas.Add(isa0);
        }

        #endregion Constructors
    }
}