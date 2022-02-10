#region Header

// The following code is inspired by the work of Josh Smith
// http://joshsmithonwpf.wordpress.com/

#endregion Header

namespace SLM_Control.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.IO;
    using System.Windows.Input;
    using System.Windows.Media.Imaging;
    using System.Xml.Linq;

    using SLM_Control.Model;

    using ThorSharedTypes;

    public class RelayCommandWithParam : ICommand
    {
        #region Fields

        private readonly Func<bool> canExecute;
        private readonly Action<object> execute;

        #endregion Fields

        #region Constructors

        /// <summary>
        /// Initializes a new instance of the RelayCommand class
        /// </summary>
        /// <param name="execute">The execution logic.</param>
        //public RelayCommand(Action execute)
        //    : this(execute, null)
        //{
        //}
        public RelayCommandWithParam(Action<object> execute)
            : this(execute, null)
        {
        }

        /// <summary>
        /// Initializes a new instance of the RelayCommand class
        /// </summary>
        /// <param name="execute">The execution logic.</param>
        /// <param name="canExecute">The execution status logic.</param>
        //public RelayCommand(Action execute, Func<bool> canExecute)
        //{
        //    if (execute == null)
        //        throw new ArgumentNullException("execute");
        //    this.execute = execute;
        //    this.canExecute = canExecute;
        //}
        public RelayCommandWithParam(Action<object> execute, Func<bool> canExecute)
        {
            if (execute == null)
                throw new ArgumentNullException("execute");

            this.execute = execute;
            this.canExecute = canExecute;
        }

        #endregion Constructors

        #region Events

        public event EventHandler CanExecuteChanged
        {
            // wire the CanExecutedChanged event only if the canExecute func
            // is defined (that improves perf when canExecute is not used)
            add
            {
                if (this.canExecute != null)
                    CommandManager.RequerySuggested += value;
            }
            remove
            {
                if (this.canExecute != null)
                    CommandManager.RequerySuggested -= value;
            }
        }

        #endregion Events

        #region Methods

        public bool CanExecute(object parameter)
        {
            return this.canExecute == null ? true : this.canExecute();
        }

        public void Execute(object parameter)
        {
            this.execute(parameter);
        }

        #endregion Methods
    }

    public class SLMViewModel : ViewModelBase
    {
        #region Fields

        public const string FOLDER_NAME = "SLMPattern";
        public const string NAME_EXT = "-phase";
        public const string SETTINGS_FILE = "SLMControlSettings.xml";
        public const double WIDTH_LIMIT = 512.0;

        public static Dictionary<string, SLMFuncType> SLMDictionary = new Dictionary<string, SLMFuncType>()
        {
        {"CONNECT", SLMFuncType.Connect},
        {"LOAD_MASK", SLMFuncType.Load},
        {"PHASE_GEN", SLMFuncType.PhaseGen},
        {"APPLY_PHASE", SLMFuncType.ApplyPhase},
        {"BLANK", SLMFuncType.Blank}
        };

        private BackgroundWorker slmBuilder;
        private string _baseFileName = "";
        private string _connectText = "CONNECT";
        private double _imageHeight = 0;
        private string _imagePath = string.Empty;
        private BitmapSource _imageSource = null;
        private double _imageWidth = 0;
        private bool _isConnected = false;
        private double _magnification = 1.0;
        private string _outputPath = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments) + "\\" + FOLDER_NAME; //output folder to store generated phase masks
        private double _pixelsizeUM = 1.0;
        private int _slm3D = 0;
        private bool _slmPanelAvailable = true;
        private RelayCommandWithParam _slmRelayCommand = null;
        private string _statusMessage = string.Empty;

        #endregion Fields

        #region Constructors

        public SLMViewModel()
        {
            slmBuilder = new BackgroundWorker();
            slmBuilder.WorkerReportsProgress = false;
            slmBuilder.WorkerSupportsCancellation = true;
        }

        #endregion Constructors

        #region Enumerations

        public enum SLMFuncType
        {
            Connect,
            Load,
            PhaseGen,
            ApplyPhase,
            Blank
        }

        #endregion Enumerations

        #region Delegates

        private delegate bool SLMWorkerStateChecker();

        #endregion Delegates

        #region Properties

        public string BaseFileName
        {
            get { return _baseFileName; }
            set
            {
                _baseFileName = value;
                OnPropertyChanged("BaseFileName");
            }
        }

        public string ConnectText
        {
            get
            {
                return _connectText;
            }
        }

        public XDocument Doc
        {
            get;
            set;
        }

        public bool ImageDisplay
        {
            get { return (_imageSource == null) ? false : true; }
        }

        public double ImageHeight
        {
            get { return _imageHeight; }
            set
            {
                _imageHeight = value;
                OnPropertyChanged("ImageHeight");
            }
        }

        public string ImagePath
        {
            get
            {
                return _imagePath;
            }
            set
            {
                _imagePath = value;
                OnPropertyChanged("ImagePath");
            }
        }

        public BitmapSource ImageSource
        {
            get { return _imageSource; }
            set
            {
                _imageSource = value;
                OnPropertyChanged("ImageSource");
                OnPropertyChanged("ImageDisplay");
            }
        }

        public double ImageWidth
        {
            get { return _imageWidth; }
            set
            {
                _imageWidth = value;
                OnPropertyChanged("ImageWidth");
            }
        }

        public bool IsConnected
        {
            get
            {
                return _isConnected;
            }
            set
            {
                _isConnected = value;
                OnPropertyChanged("IsConnected");
                _connectText = _isConnected ? "DISCONNECT" : "CONNECT";
                OnPropertyChanged("ConnectText");
            }
        }

        public double Magnification
        {
            get { return _magnification; }
            set
            {
                _magnification = value;
                OnPropertyChanged("Magnification");
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

        public double PixelsizeUM
        {
            get { return _pixelsizeUM; }
            set
            {
                _pixelsizeUM = value;
                OnPropertyChanged("PixelsizeUM");
            }
        }

        public int SLM3D
        {
            get { return _slm3D; }
            set
            {
                _slm3D = value;
                OnPropertyChanged("SLM3D");
            }
        }

        public bool SLMPanelAvailable
        {
            get
            {
                return _slmPanelAvailable;
            }
            set
            {
                _slmPanelAvailable = value;
                OnPropertyChanged("SLMPanelAvailable");
            }
        }

        public RelayCommandWithParam SLMRelayCommand
        {
            get
            {
                if (this._slmRelayCommand == null)
                    this._slmRelayCommand = new RelayCommandWithParam(SLMCommands);

                return this._slmRelayCommand;
            }
        }

        public bool SLMSelectWavelength
        {
            get
            {
                double dVal = -1;
                SLMDeviceFuncs.GetParam((int)DeviceParams.PARAM_SLM_WAVELENGTH_SELECT, ref dVal);
                return (1 == (int)dVal);
            }
            set
            {
                SLMDeviceFuncs.SetParam((int)DeviceParams.PARAM_SLM_WAVELENGTH_SELECT, value ? 1.0 : 0.0);
                OnPropertyChanged("SLMSelectWavelength");
                OnPropertyChanged("SLMWavelengthNM");
            }
        }

        public int SLMWavelengthNM
        {
            get
            {
                double[] val = new double[2] { 0, 0 };

                if (SLMDeviceFuncs.GetParamAvailable(Convert.ToInt32(DeviceParams.PARAM_SLM_WAVELENGTH)))
                    SLMDeviceFuncs.GetParamBuffer<double>((int)DeviceParams.PARAM_SLM_WAVELENGTH, val, (int)Constants.MAX_WIDEFIELD_WAVELENGTH_COUNT);

                return (int)val[SLMSelectWavelength ? 1 : 0];
            }
        }

        public string StatusMessage
        {
            get
            {
                return _statusMessage;
            }
            set
            {
                _statusMessage = value;
                OnPropertyChanged("StatusMessage");
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
                            new XElement("SLMControl",
                                new XAttribute("PixelsizeUM", PixelsizeUM),
                                new XAttribute("Magnification", Magnification),
                                new XElement("Parameters",
                                    new XAttribute("SLM3D", SLM3D),
                                    new XAttribute("SelectWavelength", (int)(SLMSelectWavelength ? 1 : 0))
                                    ),
                                    new XElement("FilePath",
                                        new XElement("OutputFolder", OutputPath),
                                        new XElement("FileName", BaseFileName)
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
                    XElement root = GetOrCreateElement(Doc.Root, "SLMControl");
                    PixelsizeUM = (double?)root.Attribute("PixelsizeUM") ?? PixelsizeUM;
                    Magnification = (double?)root.Attribute("Magnification") ?? Magnification;

                    XElement param = GetOrCreateElement(root, "Parameters");
                    SLM3D = (int?)param.Attribute("SLM3D") ?? SLM3D;
                    SLMSelectWavelength = (1 == ((int?)param.Attribute("SelectWavelength") ?? (SLMSelectWavelength ? 1 : 0)));

                    XElement paths = GetOrCreateElement(root, "FilePath");
                    OutputPath = (0 == ((string)GetOrCreateElement(paths, "OutputFolder").Value).Length) ? OutputPath : GetOrCreateElement(paths, "OutputFolder").Value;
                    BaseFileName = (0 == ((string)GetOrCreateElement(paths, "FileName").Value).Length) ? BaseFileName : GetOrCreateElement(paths, "FileName").Value;
                }
            }
            catch (Exception ex)
            {
                string err = ex.Message;
                StatusMessage = String.Format("Error at loading settings,\nplease delete SLM_ControlSettings.xml and restart again.");
            }
        }

        public void SLMCommands(object obj)
        {
            int dev = 0;
            string str = (string)obj;
            switch (SLMDictionary[str])
            {
                case SLMFuncType.Load:
                    Microsoft.Win32.OpenFileDialog ofd = new Microsoft.Win32.OpenFileDialog();

                    ofd.FileName = "*.bmp";
                    ofd.InitialDirectory = OutputPath;
                    ofd.DefaultExt = ".bmp";
                    ofd.Filter = "BMP Files (*.bmp)|*.bmp";

                    Nullable<bool> result = ofd.ShowDialog();
                    if (true == result)
                        PrepareImages(ofd.FileName);

                    break;
                case SLMFuncType.PhaseGen:
                    if (String.IsNullOrEmpty(ImagePath))
                    {
                        StatusMessage = String.Format("Please load an image first.");
                        return;
                    }
                    if (!File.Exists(ImagePath))
                    {
                        StatusMessage = String.Format("Loaded image is not valid.");
                        return;
                    }

                    if (slmBuilder.IsBusy)
                    {
                        slmBuilder.CancelAsync();
                    }
                    else
                    {
                        if (Path.GetFileNameWithoutExtension(ImagePath).Contains(NAME_EXT))
                        {
                            StatusMessage = String.Format("The selected file contains keyword: " + NAME_EXT + ",\nplease select different file name.");
                        }
                        else
                        {
                            SLMPanelAvailable = false;
                            slmBuilder.DoWork += new DoWorkEventHandler(SLMBuilder_DoWork);
                            slmBuilder.RunWorkerCompleted += new RunWorkerCompletedEventHandler(SLMBuilder_RunWorkerCompleted);
                            slmBuilder.RunWorkerAsync(argument: SetPhaseFileName(ImagePath));
                        }
                    }
                    break;
                case SLMFuncType.ApplyPhase:
                    if (null != ImageSource && !string.IsNullOrEmpty(ImagePath))
                    {
                        SLMDeviceFuncs.SetParam((int)DeviceParams.PARAM_SLM_RUNTIME_CALC, (double)0.0);
                        SLMDeviceFuncs.SetParam((int)DeviceParams.PARAM_SLM_FUNC_MODE, (double)SLMFunctionMode.LOAD_PHASE_ONLY);
                        SLMDeviceFuncs.SetParam((int)DeviceParams.PARAM_SLM_ARRAY_ID, (double)0);
                        SLMDeviceFuncs.SetParamString((int)DeviceParams.PARAM_SLM_BMP_FILENAME, ImagePath);
                        SLMDeviceFuncs.SetParam((int)DeviceParams.PARAM_SLM_PHASE_DIRECT, (double)1.0);
                        SLMDeviceFuncs.SetParam((int)DeviceParams.PARAM_SLM_TIMEOUT, (double)0.0);
                        SLMDeviceFuncs.PreflightPosition();
                        SLMDeviceFuncs.SetupPosition();
                        SLMDeviceFuncs.StartPosition();
                    }
                    break;
                case SLMFuncType.Blank:
                    SLMDeviceFuncs.PostflightPosition();
                    SLMDeviceFuncs.SetParam((int)DeviceParams.PARAM_SLM_BLANK, (double)1.0);
                    break;
                case SLMFuncType.Connect:
                    try
                    {
                        if (!IsConnected)
                        {
                            if (0 == SLMDeviceFuncs.FindDevices(ref dev))
                            {
                                StatusMessage = String.Format("Unable to connect.\nMake sure the device is powered and the port settings are correct.");
                                IsConnected = false;
                            }

                            SLMDeviceFuncs.SelectDevice(0);
                            IsConnected = true;
                        }
                        else
                        {
                            SLMDeviceFuncs.PostflightPosition();
                            SLMDeviceFuncs.TeardownDevice();
                            IsConnected = false;
                        }
                        StatusMessage = string.Empty;
                        ImageSource = null;
                    }
                    catch (Exception ex)
                    {
                        if (ex is BadImageFormatException || ex is DllNotFoundException)
                        {
                            StatusMessage = String.Format("Dll not found.");
                        }
                    }
                    break;
            }
        }

        public void UpdateSettings()
        {
            try
            {
                if (null != Doc)
                {
                    XElement root = Doc.Root.Element("SLMControl");
                    root.SetAttributeValue("PixelsizeUM", PixelsizeUM.ToString());
                    root.SetAttributeValue("Magnification", Magnification.ToString());
                    XElement param = root.Element("Parameters");
                    param.SetAttributeValue("SLM3D", SLM3D.ToString());
                    param.SetAttributeValue("SelectWavelength", (SLMSelectWavelength ? 1 : 0).ToString());
                    XElement paths = root.Element("FilePath");
                    paths.SetElementValue("OutputFolder", OutputPath);
                    paths.SetElementValue("FileName", BaseFileName);
                    Doc.Save(SETTINGS_FILE);
                }
            }
            catch (Exception)
            {
                StatusMessage = String.Format("Error at saving settings.");
            }
        }

        private void PrepareImages(string fileName)
        {
            try
            {
                if (File.Exists(fileName))
                {
                    BaseFileName = Path.GetFileName(fileName);
                    ImageSource = File.Exists(fileName) ? new BitmapImage(new Uri(fileName)) : null;
                    ImageWidth = Math.Min(WIDTH_LIMIT, ImageSource.PixelWidth);
                    ImageHeight = ImageWidth * ImageSource.PixelHeight / ImageSource.PixelWidth;
                    StatusMessage = String.Format("Image Loaded: " + BaseFileName);
                    ImagePath = fileName;
                }
            }
            catch (Exception ex)
            {
                StatusMessage = String.Format("Error at loading selected bitmap:" + ex);
            }
        }

        /// <summary>
        /// set up phase mask file name, create new if not able to remove last
        /// </summary>
        /// <param name="inputPath"></param>
        /// <returns></returns>
        private string SetPhaseFileName(string inputPath)
        {
            FileName imgPhase = new FileName(Path.GetFileNameWithoutExtension(inputPath) + NAME_EXT);
            imgPhase.FileExtension = ".bmp";
            imgPhase.MakeUnique(Path.GetDirectoryName(inputPath));
            return Path.GetDirectoryName(inputPath) + "\\" + imgPhase.FullName;
        }

        private void SLMBuilder_DoWork(object sender, DoWorkEventArgs e)
        {
            SLMDeviceFuncs.SetParam((int)DeviceParams.PARAM_SLM_FUNC_MODE, (double)SLMFunctionMode.SAVE_PHASE);
            SLMDeviceFuncs.SetParam((int)DeviceParams.PARAM_SLM_ARRAY_ID, (double)0);
            SLMDeviceFuncs.SetParam((int)DeviceParams.PARAM_SLM_PHASE_DIRECT, (double)1.0);

            // set pixel size
            string imgPhasePath = (string)e.Argument;
            System.Drawing.Bitmap bmp = new System.Drawing.Bitmap(ImagePath);
            float dpi = (float)((double)Constants.UM_PER_INCH / Magnification / PixelsizeUM);
            bmp.SetResolution(dpi, dpi);                    //dots per inch
            bmp.Save(imgPhasePath, System.Drawing.Imaging.ImageFormat.Bmp);

            // save phase mask
            SLMDeviceFuncs.SetParamString((int)DeviceParams.PARAM_SLM_BMP_FILENAME, imgPhasePath);
            SLMDeviceFuncs.PreflightPosition();
            SLMDeviceFuncs.SetupPosition();

            e.Result = imgPhasePath;
        }

        private void SLMBuilder_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            if (e.Cancelled == true)
            {
                StatusMessage = String.Format("Worker is cancelled.");
            }
            else if (e.Error != null)
            {
                StatusMessage = String.Format("Error: " + e.Error.Message);
            }
            else
            {
                PrepareImages(e.Result as string);
            }

            slmBuilder.DoWork -= new DoWorkEventHandler(SLMBuilder_DoWork);
            slmBuilder.RunWorkerCompleted -= new RunWorkerCompletedEventHandler(SLMBuilder_RunWorkerCompleted);
            SLMPanelAvailable = true;
        }

        #endregion Methods
    }

    /// <summary>
    /// Base class for all ViewModel classes in the application. Provides support for 
    /// property changes notification.
    /// </summary>
    public abstract class ViewModelBase : INotifyPropertyChanged
    {
        #region Events

        /// <summary>
        /// Raised when a property on this object has a new value.
        /// </summary>
        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Methods

        /// <summary>
        /// Warns the developer if this object does not have a public property with
        /// the specified name. This method does not exist in a Release build.
        /// </summary>
        [Conditional("DEBUG")]
        [DebuggerStepThrough]
        public void VerifyPropertyName(string propertyName)
        {
            // verify that the property name matches a real,
            // public, instance property on this object.
            // or is empty, indicating a call to refresh all
            // bindings
            if (TypeDescriptor.GetProperties(this)[propertyName] == null && propertyName != "")
            {
                Debug.Fail("Invalid property name: " + propertyName);
            }
        }

        /// <summary>
        /// Raises this object's PropertyChanged event.
        /// </summary>
        /// <param name="propertyName">The name of the property that has a new value.</param>
        protected virtual void OnPropertyChanged(string propertyName)
        {
            this.VerifyPropertyName(propertyName);

            if (this.PropertyChanged != null)
            {
                this.PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }

        #endregion Methods
    }
}