namespace BesselBeamGen.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.Drawing;
    using System.IO;
    using System.Linq;
    using System.Text;
    using System.Text.RegularExpressions;
    using System.Threading.Tasks;
    using System.Windows.Forms;
    using System.Windows.Input;
    using System.Windows.Media.Imaging;
    using System.Xml.Linq;

    using ThorSharedTypes;

    /// <summary>
    /// ImageName class used to preserve image full paths and period values for sorting
    /// </summary>
    public class ImageName : IComparable
    {
        #region Constructors

        public ImageName(double period, string path)
        {
            Period = period;
            Path = path;
        }

        #endregion Constructors

        #region Properties

        public string Path
        {
            get;
            set;
        }

        public double Period
        {
            get;
            set;
        }

        #endregion Properties

        #region Methods

        public int CompareTo(object obj)
        {
            ImageName img = obj as ImageName;

            if (obj == null)
                throw new ArgumentException("obj is not an ImageName object.");

            return Period.CompareTo(img.Period);
        }

        #endregion Methods
    }

    class BesselBeamViewModel : VMBase
    {
        #region Fields

        public const string FOLDER_NAME = "BesselBeam";
        public const string SETTINGS_FILE = "BesselBeamTemplate.xml";

        const double FOCAL_LEN1 = 300000; //[um]
        const int HUNDRED_PERCENT = 100;
        const int IMAGE_SIZE = 512;
        const int MAX_PERIOD = 256;
        const int MAX_VAL = 255;
        const int NM_TO_UM = 1000;
        const double PUPLE = 15; //[um]

        private string _baseFileName = FOLDER_NAME;
        private double _d0 = 850; //[um]
        private double _d1 = 1000; //[um]
        private ICommand _generateCommand;
        private int _generateModeIndex = (int)GENERATE_MODE.SQUARE;
        private List<string> _generateModeItems = Enum.GetValues(typeof(GENERATE_MODE)).Cast<GENERATE_MODE>().Select(t => t.ToString()).ToList();
        private int _imageIdCurrent = 0;
        private int _imageIdMax = 0;
        private string _imagePath = "";
        private List<ImageName> _imagesInDir = new List<ImageName>();
        private BitmapSource _imageSource = null;
        private bool _invert = false;
        private double _lambda = 1040; //[nm]
        private string _lastError = string.Empty;
        private System.Windows.Media.Brush _lastErrorColor = System.Windows.Media.Brushes.White;
        private ERROR_STATE _lastErrorState = ERROR_STATE.NONE;
        private ICommand _openBitmapCommand;
        private string _outputPath = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments) + "\\" + FOLDER_NAME; //output folder to store generated phase masks
        private double _periodEnd = 256; //[pixels]
        private double _periodStart = 40; //[pixels]
        private double _periodStep = 1; //[pixels]
        private string _periodString = string.Empty;
        private bool _periodSweep = false;
        private int _selectedPeriodTab = 0; //[0]D1D2, [1]Period
        private ICommand _setOutputPathCommand;
        private double _valuePercent = 50; //[%] Black
        private double _valuePercentW = 50; //[%] White

        #endregion Fields

        #region Constructors

        public BesselBeamViewModel()
        {
        }

        #endregion Constructors

        #region Enumerations

        public enum ERROR_STATE
        {
            NONE, WARNING, ERROR
        }

        public enum GENERATE_MODE
        {
            SQUARE, SINUSOID, TRIANGLE, BLADE
        }

        #endregion Enumerations

        #region Properties

        public string BaseFileName
        {
            get { return _baseFileName; }
            set
            {
                if (!value.Contains('-'))
                {
                    _baseFileName = value;
                    OnPropertyChanged("BaseFileName");
                }
            }
        }

        public double D0
        {
            get { return _d0; }
            set
            {
                _d0 = value;
                OnPropertyChanged("D0");
            }
        }

        public double D1
        {
            get { return _d1; }
            set
            {
                _d1 = value;
                OnPropertyChanged("D1");
            }
        }

        public XDocument Doc
        {
            get;
            set;
        }

        public ICommand GenerateCommand
        {
            get
            {
                if (this._generateCommand == null)
                    this._generateCommand = new RelayCommand(() => Generate());

                return this._generateCommand;
            }
        }

        public int GenerateModeIndex
        {
            get { return _generateModeIndex; }
            set
            {
                _generateModeIndex = value;
                OnPropertyChanged("GenerateModeIndex");
            }
        }

        public List<string> GenerateModeItems
        {
            get { return _generateModeItems; }
            set
            {
                _generateModeItems = value;
                OnPropertyChanged("GenerateModeItems");
            }
        }

        public int ImageIdCurrent
        {
            get { return _imageIdCurrent; }
            set
            {
                if (value <= ImageIdMax)
                {
                    _imageIdCurrent = value;

                    ImagePath = Path.GetFileName(ImagesInDir.ElementAt(_imageIdCurrent - 1).Path);
                    ImageSource = File.Exists(ImagesInDir.ElementAt(_imageIdCurrent - 1).Path) ? new BitmapImage(new Uri(ImagesInDir.ElementAt(_imageIdCurrent - 1).Path)) : null;
                    OnPropertyChanged("ImageIdCurrent");
                }
            }
        }

        public int ImageIdMax
        {
            get { return _imageIdMax; }
            set
            {
                _imageIdMax = value;
                OnPropertyChanged("ImageIdMax");
            }
        }

        public string ImagePath
        {
            get { return _imagePath; }
            set
            {
                _imagePath = value;
                OnPropertyChanged("ImagePath");
            }
        }

        public List<ImageName> ImagesInDir
        {
            get { return _imagesInDir; }
            set
            {
                _imagesInDir = value;
                OnPropertyChanged("ImagesInDir");
            }
        }

        public BitmapSource ImageSource
        {
            get { return _imageSource; }
            set
            {
                _imageSource = value;
                OnPropertyChanged("ImageSource");
            }
        }

        public bool Invert
        {
            get { return _invert; }
            set
            {
                _invert = value;
                OnPropertyChanged("Invert");
            }
        }

        public double Lambda
        {
            get { return _lambda; }
            set
            {
                _lambda = value;
                OnPropertyChanged("Lambda");
            }
        }

        public string LastError
        {
            get { return _lastError; }
            set
            {
                _lastError = value;
                OnPropertyChanged("LastError");
            }
        }

        public System.Windows.Media.Brush LastErrorColor
        {
            get { return _lastErrorColor; }
            set
            {
                _lastErrorColor = value;
                OnPropertyChanged("LastErrorColor");
            }
        }

        public ERROR_STATE LastErrorState
        {
            get { return _lastErrorState; }
            set
            {
                _lastErrorState = value;
                switch (value)
                {
                    case ERROR_STATE.NONE:
                        LastErrorColor = System.Windows.Media.Brushes.White;
                        break;
                    case ERROR_STATE.WARNING:
                        LastErrorColor = System.Windows.Media.Brushes.Yellow;
                        break;
                    case ERROR_STATE.ERROR:
                        LastErrorColor = System.Windows.Media.Brushes.Red;
                        break;
                    default:
                        break;
                }
                OnPropertyChanged("LastErrorState");
            }
        }

        public ICommand OpenBitmapCommand
        {
            get
            {
                if (this._openBitmapCommand == null)
                    this._openBitmapCommand = new RelayCommand(() => OpenBitmap());

                return this._openBitmapCommand;
            }
        }

        public string OutputPath
        {
            get { return _outputPath; }
            set
            {
                _outputPath = value;
                OnPropertyChanged("OutputPath");
            }
        }

        public double PeriodEnd
        {
            get { return _periodEnd; }
            set
            {
                if (value <= MAX_PERIOD)
                {
                    _periodEnd = Math.Round(value, 3);
                    OnPropertyChanged("PeriodEnd");
                }
            }
        }

        public double PeriodStart
        {
            get { return _periodStart; }
            set
            {
                _periodStart = Math.Round(value, 3);
                OnPropertyChanged("PeriodStart");
            }
        }

        public double PeriodStep
        {
            get { return _periodStep; }
            set
            {
                _periodStep = Math.Round(value, 3);
                OnPropertyChanged("PeriodStep");
            }
        }

        public string PeriodString
        {
            get { return _periodString; }
            set
            {
                _periodString = value;
                OnPropertyChanged("PeriodString");
            }
        }

        public bool PeriodSweep
        {
            get { return _periodSweep; }
            set
            {
                _periodSweep = value;
                OnPropertyChanged("PeriodSweep");
            }
        }

        public int SelectedPeriodTab
        {
            get { return _selectedPeriodTab; }
            set
            {
                _selectedPeriodTab = value;
                OnPropertyChanged("SelectedPeriodTab");
            }
        }

        public ICommand SetOutputPathCommand
        {
            get
            {
                if (this._setOutputPathCommand == null)
                    this._setOutputPathCommand = new RelayCommand(() => SetOutputPath());

                return this._setOutputPathCommand;
            }
        }

        public double ValuePercent
        {
            get { return _valuePercent; }
            set
            {
                _valuePercent = value;
                OnPropertyChanged("ValuePercent");
            }
        }
        public double ValuePercentWhite
        {
            get { return _valuePercentW; }
            set
            {
                _valuePercentW = value;
                OnPropertyChanged("ValuePercentWhite");
            }
        }

        #endregion Properties

        #region Methods

        public static XElement GetOrCreateElement(XContainer container, string name)
        {
            var element = container.Element(name);
            if (element == null)
            {
                element = new XElement(name);
                container.Add(element);
            }
            return element;
        }

        public void LoadSettings()
        {
            try
            {
                if (!File.Exists(SETTINGS_FILE))
                {
                    XDocument doc = new XDocument(
                        new XElement("Template",
                            new XElement("BesselBeam",
                                new XAttribute("SelectedTab", SelectedPeriodTab),
                                new XElement("Parameters",
                                    new XAttribute("D0UM", D0),
                                    new XAttribute("D1UM", D1),
                                    new XAttribute("WavelengthNM", Lambda),
                                    new XAttribute("Invert", Invert),
                                    new XAttribute("ValueMax", ValuePercent),
                                    new XAttribute("ValueMaxWhite", ValuePercentWhite)
                                    ),
                                    new XElement("MaskGeneration",
                                        new XAttribute("Sweep", PeriodSweep),
                                        new XAttribute("PeriodStartPx", PeriodStart),
                                        new XAttribute("PeriodStepPx", PeriodStep),
                                        new XAttribute("PeriodEndPx", PeriodEnd),
                                        new XAttribute("GenerateMode", ((GENERATE_MODE)GenerateModeIndex).ToString())
                                        ),
                                        new XElement("FilePath",
                                            new XElement("OutputFolder", OutputPath),
                                            new XElement("FileName", BaseFileName),
                                            new XElement("DisplayImage", ImagePath)
                                            )
                                            )
                                            )
                                            );

                    doc.Save(SETTINGS_FILE);
                    Doc = doc;
                }
                else
                {
                    Doc = XDocument.Load(SETTINGS_FILE);
                    XElement root = GetOrCreateElement(Doc.Root, "BesselBeam");
                    SelectedPeriodTab = (int?)root.Attribute("SelectedTab") ?? SelectedPeriodTab;

                    XElement param = GetOrCreateElement(root, "Parameters");
                    D0 = (double?)param.Attribute("D0UM") ?? D0;
                    D1 = (double?)param.Attribute("D1UM") ?? D1;
                    Lambda = (double?)param.Attribute("WavelengthNM") ?? Lambda;
                    Invert = (bool?)param.Attribute("Invert") ?? Invert;
                    ValuePercent = (double?)param.Attribute("ValueMax") ?? ValuePercent;
                    ValuePercentWhite = (double?)param.Attribute("ValueMaxWhite") ?? ValuePercentWhite;

                    XElement gen = GetOrCreateElement(root, "MaskGeneration");
                    PeriodSweep = (bool?)gen.Attribute("Sweep") ?? PeriodSweep;
                    PeriodStart = (double?)gen.Attribute("PeriodStartPx") ?? PeriodStart;
                    PeriodStep = (double?)gen.Attribute("PeriodStepPx") ?? PeriodStep;
                    PeriodEnd = (double?)gen.Attribute("PeriodEndPx") ?? PeriodEnd;
                    GenerateModeIndex = (int)Enum.Parse(typeof(GENERATE_MODE), (string)gen.Attribute("GenerateMode") ?? ((GENERATE_MODE)GenerateModeIndex).ToString());

                    XElement paths = GetOrCreateElement(root, "FilePath");
                    OutputPath = (0 == ((string)GetOrCreateElement(paths, "OutputFolder").Value).Length) ? OutputPath : GetOrCreateElement(paths, "OutputFolder").Value;
                    BaseFileName = (0 == ((string)GetOrCreateElement(paths, "FileName").Value).Length) ? BaseFileName : GetOrCreateElement(paths, "FileName").Value;
                    //ImagePath = (0 == ((string)GetOrCreateElement(paths, "DisplayImage").Value).Length) ? ImagePath : GetOrCreateElement(paths, "DisplayImage").Value;
                }
            }
            catch (Exception ex)
            {
                LogError(ex, "Error at loading settings, please delete BesselBeamTemplate.xml and restart again.");
            }
        }

        public void UpdateSettings()
        {
            try
            {
                if (null != Doc)
                {
                    XElement root = Doc.Root.Element("BesselBeam");
                    root.SetAttributeValue("SelectedTab", SelectedPeriodTab.ToString());

                    XElement param = root.Element("Parameters");
                    param.SetAttributeValue("D0UM", D0.ToString());
                    param.SetAttributeValue("D1UM", D1.ToString());
                    param.SetAttributeValue("WavelengthNM", Lambda.ToString());
                    param.SetAttributeValue("Invert", Invert.ToString());
                    param.SetAttributeValue("ValueMax", ValuePercent.ToString());
                    param.SetAttributeValue("ValueMaxWhite", ValuePercentWhite.ToString());

                    XElement gen = root.Element("MaskGeneration");
                    gen.SetAttributeValue("Sweep", PeriodSweep.ToString());
                    gen.SetAttributeValue("PeriodStartPx", PeriodStart.ToString());
                    gen.SetAttributeValue("PeriodStepPx", PeriodStep.ToString());
                    gen.SetAttributeValue("PeriodEndPx", PeriodEnd.ToString());
                    gen.SetAttributeValue("GenerateMode", ((GENERATE_MODE)GenerateModeIndex).ToString());

                    XElement paths = root.Element("FilePath");
                    paths.SetElementValue("OutputFolder", OutputPath);
                    paths.SetElementValue("FileName", BaseFileName);
                    //paths.SetElementValue("DisplayImage", ImagePath);

                    Doc.Save(SETTINGS_FILE);
                }
            }
            catch (Exception ex)
            {
                LogError(ex, "Error at saving settings.");
            }
        }

        /// <summary>
        /// Generate phase masks, including period sweep
        /// </summary>
        private void Generate()
        {
            double period = PeriodStart;
            double lambdaUM = Lambda / NM_TO_UM;     //[um]
            PeriodString = string.Empty;
            string periodStr = string.Empty, pathAndName = string.Empty;

            try
            {

                if (0 == SelectedPeriodTab)
                {
                    PeriodSweep = false;
                    PeriodStart = (4 * FOCAL_LEN1 * lambdaUM) / (PUPLE * (D0 + D1));
                    PeriodString = "Period: " + Math.Round(PeriodStart, 2).ToString() + " Px";
                }

                //set file name:
                string modeStr = "";
                switch ((GENERATE_MODE)GenerateModeIndex)
                {
                    case GENERATE_MODE.SINUSOID:
                        modeStr = (Invert) ? "-InvSin" : "-Sin";
                        break;
                    case GENERATE_MODE.TRIANGLE:
                        modeStr = (Invert) ? "-InvTri" : "-Tri";
                        break;
                    case GENERATE_MODE.BLADE:
                        modeStr = (Invert) ? "-InvBld" : "-Bld";
                        break;
                    case GENERATE_MODE.SQUARE:
                        modeStr = (Invert) ? "-InvSqr" : "-Sqr";
                        break;
                    default:
                        break;
                }

                //generate mask:
                do
                {
                    periodStr = "-" + period.ToString("0.###") + "Px";
                    pathAndName = OutputPath + "\\" + BaseFileName + modeStr + periodStr + ".bmp";

                    Bitmap bmp = GenerateBitmap(period, pathAndName);

                    ////display image:
                    //ImagePath = Path.GetFileName(pathAndName);
                    //ImageSource = System.Windows.Interop.Imaging.CreateBitmapSourceFromHBitmap(bmp.GetHbitmap(),
                    //    IntPtr.Zero,
                    //    System.Windows.Int32Rect.Empty,
                    //    BitmapSizeOptions.FromWidthAndHeight(IMAGE_SIZE, IMAGE_SIZE));

                    if ((!PeriodSweep) || (PeriodEnd < period + PeriodStep))
                        break;

                    period += PeriodStep;
                } while (period < MAX_PERIOD);

                //done:
                PrepareImages(pathAndName);
                LastErrorState = ERROR_STATE.NONE;
                LastError = "Pattern Generated!";
            }
            catch (Exception ex)
            {
                ImagePath = "";
                LogError(ex, "Error at generating phase mask: period " + period.ToString("0.###") + "Pixels");
            }
        }

        private Bitmap GenerateBitmap(double period, string pathAndName)
        {
            try
            {
                int pixelVal = (int)((int?)(MAX_VAL * ValuePercent / HUNDRED_PERCENT) ?? ValuePercent);
                int pixelValW = (int)((int?)(MAX_VAL * ValuePercentWhite / HUNDRED_PERCENT) ?? ValuePercentWhite);
                double halfPeriod = period / 2;
                double Ks = Math.PI / period;       //doubled period after absolute value
                double slope = pixelVal / halfPeriod;   //slope for triangle and blade

                int width = IMAGE_SIZE, height = IMAGE_SIZE;
                Point center = new Point(width / 2, height / 2);
                Bitmap bmp24bit = new Bitmap(width, height, System.Drawing.Imaging.PixelFormat.Format24bppRgb);
                System.Drawing.Imaging.BitmapData bmpData = bmp24bit.LockBits(new System.Drawing.Rectangle(0, 0, bmp24bit.Width, bmp24bit.Height), System.Drawing.Imaging.ImageLockMode.WriteOnly, System.Drawing.Imaging.PixelFormat.Format24bppRgb);
                int pixelsize = System.Drawing.Image.GetPixelFormatSize(bmpData.PixelFormat) / 8;
                byte[] bytes = new byte[bmpData.Height * bmpData.Stride];

                for (int j = 0; j < height; j++)
                {
                    for (int i = 0; i < width; i++)
                    {
                        double distance = Math.Sqrt(Math.Pow(i + 1 - center.X, 2) + Math.Pow(j + 1 - center.Y, 2));     //[pixels]
                        int region = (int)(distance / halfPeriod);  //every half periods: 0,1,2,3 ...

                        switch ((GENERATE_MODE)GenerateModeIndex)
                        {
                            case GENERATE_MODE.SQUARE:
                                for (int h = 0; h < 3; h++)
                                {
                                    if (Invert)
                                    {
                                        bytes[j * bmpData.Stride + (i * pixelsize) + h] = (0 == (region % 2)) ? (byte)pixelValW : (byte)pixelVal; // added ability to set level for dark ring (Black)
                                    }
                                    else
                                    {
                                        bytes[j * bmpData.Stride + (i * pixelsize) + h] = (0 == (region % 2)) ? (byte)pixelVal : (byte)pixelValW;
                                    }
                                }
                                break;
                            case GENERATE_MODE.SINUSOID:
                                for (int h = 0; h < 3; h++)
                                {
                                    if (Invert)
                                    {
                                        bytes[j * bmpData.Stride + (i * pixelsize) + h] = (byte)(pixelVal * Math.Abs(Math.Sin(Ks * distance)));
                                    }
                                    else
                                    {
                                        bytes[j * bmpData.Stride + (i * pixelsize) + h] = (byte)(pixelVal * Math.Abs(Math.Sin(Ks * distance + (Math.PI / 2))));
                                    }
                                }
                                break;
                            case GENERATE_MODE.TRIANGLE:
                                for (int h = 0; h < 3; h++)
                                {
                                    if (Invert)
                                    {
                                        bytes[j * bmpData.Stride + (i * pixelsize) + h] = (0 == (region % 2)) ? (byte)((slope * distance) - (slope * region * halfPeriod)) : (byte)((-slope * distance) + (slope * (region + 1) * halfPeriod));
                                    }
                                    else
                                    {
                                        bytes[j * bmpData.Stride + (i * pixelsize) + h] = (0 == (region % 2)) ? (byte)((-slope * distance) + (slope * (region + 1) * halfPeriod)) : (byte)((slope * distance) - (slope * region * halfPeriod));
                                    }
                                }
                                break;
                            case GENERATE_MODE.BLADE:
                                for (int h = 0; h < 3; h++)
                                {
                                    if (Invert)
                                    {
                                        bytes[j * bmpData.Stride + (i * pixelsize) + h] = (0 == (region % 2)) ? (byte)((slope * distance) - (slope * region * halfPeriod)) : (byte)((slope * distance) - (slope * region * halfPeriod));
                                    }
                                    else
                                    {
                                        bytes[j * bmpData.Stride + (i * pixelsize) + h] = (0 == (region % 2)) ? (byte)((-slope * distance) + (slope * (region + 1) * halfPeriod)) : (byte)((-slope * distance) + (slope * (region + 1) * halfPeriod));
                                    }
                                }
                                break;
                            default:
                                break;
                        }
                    }
                }

                System.Runtime.InteropServices.Marshal.Copy(bytes, 0, bmpData.Scan0, bytes.Length);
                bmp24bit.UnlockBits(bmpData);

                //output bitmap:
                if (!Directory.Exists(Path.GetDirectoryName(pathAndName)))
                    Directory.CreateDirectory(Path.GetDirectoryName(pathAndName));

                bmp24bit.Save(pathAndName, System.Drawing.Imaging.ImageFormat.Bmp);
                return bmp24bit;
            }
            catch (Exception ex)
            {
                LogError(ex, "Error at generating bitmap.");
            }
            return null;
        }

        private void LogError(Exception ex, string defaultMessage)
        {
            LastErrorState = ERROR_STATE.ERROR;
            if (null != ex.InnerException)
                LastError = ex.InnerException.Message;
            else if (null != ex.Message)
                LastError = ex.Message + "\n" + defaultMessage;
        }

        private void OpenBitmap()
        {
            Microsoft.Win32.OpenFileDialog ofd = new Microsoft.Win32.OpenFileDialog();

            ofd.FileName = "*.bmp";
            ofd.InitialDirectory = OutputPath;
            ofd.DefaultExt = ".bmp";
            ofd.Filter = "BMP Files (*.bmp)|*.bmp";

            Nullable<bool> result = ofd.ShowDialog();
            if (true == result)
            {
                PrepareImages(ofd.FileName);
            }
            else
            {
                ImagePath = "";
            }
        }

        private bool PrepareImages(string fileName)
        {
            try
            {
                if (!File.Exists(fileName))
                    return false;

                char[] chars = { '-' };
                string baseNameWithMode = Path.GetFileName(fileName);
                baseNameWithMode = baseNameWithMode.Substring(0, baseNameWithMode.LastIndexOfAny(chars));
                string baseName = baseNameWithMode.Substring(0, baseNameWithMode.IndexOfAny(chars));

                DirectoryInfo dir = new DirectoryInfo(Path.GetDirectoryName(fileName));
                FileInfo[] filesInDir = dir.GetFiles("*" + baseNameWithMode + "*.*", SearchOption.TopDirectoryOnly);

                //use ImageName class to sort based on periods
                ImagesInDir.Clear();
                foreach (var item in filesInDir)
                {
                    ImageName imgName = new ImageName(Double.Parse(item.Name.Substring(item.Name.LastIndexOf("-") + 1, item.Name.LastIndexOf("Px") - item.Name.LastIndexOf("-") - 1)), item.FullName);
                    ImagesInDir.Add(imgName);
                }
                ImagesInDir.Sort();

                ImageIdMax = ImagesInDir.Count;
                ImageIdCurrent = ImagesInDir.FindIndex(x => x.Path.Equals(fileName, StringComparison.Ordinal)) + 1;
                return true;
            }
            catch (Exception ex)
            {
                LogError(ex, "Error at loading selected bitmap.");
            }
            return false;
        }

        private void SetOutputPath()
        {
            FolderBrowserDialog fbDialog = new FolderBrowserDialog();
            if (System.Windows.Forms.DialogResult.OK == fbDialog.ShowDialog())
            {
                OutputPath = fbDialog.SelectedPath;
            }
        }

        #endregion Methods
    }
}