namespace ScanControl.ViewModel
{
    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Text.RegularExpressions;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Shapes;
    using System.Xml;

    using ChartPlotterControl;

    using ThorLogging;

    using ThorSharedTypes;

    public class TwoWayItemComparer : IComparer<string>
    {
        #region Methods

        public int Compare(string left, string right)
        {
            string leftStr = (string)left;
            string rightStr = (string)right;

            string pat = @"(.*),(.*)";
            string[] strResultL = Regex.Split(leftStr, pat);
            string[] strResultR = Regex.Split(rightStr, pat);

            int iLeft = Convert.ToInt32(strResultL[1]);
            int iRight = Convert.ToInt32(strResultR[1]);
            if (iLeft == iRight)
            {
                return 0;

            }
            else if (iLeft < iRight)
            {
                return -1;
            }
            else
            {
                return 1;
            }
        }

        #endregion Methods
    }

    /// <summary>
    /// Interaction logic for TwoWaySettings.xaml
    /// </summary>
    public partial class TwoWaySettings : Window
    {
        #region Fields

        private const string COARSE_FILE_BASE = "AlignDataCoarse";
        private const string COARSE_FILE_NAME = ".\\AlignDataCoarse.txt";
        private const int FIELD_SIZE_TABLE_LENGTH = 256;
        private const string FINE_FILE_BASE = "AlignData";
        private const string FINE_FILE_NAME = ".\\AlignData.txt";
        private const string GALVO_FILE_BASE = "AlignDataGalvo";
        private const string GALVO_FILE_NAME = ".\\AlignDataGalvo.txt";
        private const double MAX_ALIGN_VALUE = 128;
        private const double MAX_ALIGN_VALUE_COARSE = 255;
        private const double MAX_ALIGN_VALUE_GALVO = 5000;
        private const double MIN_ALIGN_VALUE = -128;
        private const double MIN_ALIGN_VALUE_COARSE = 0;
        private const double MIN_ALIGN_VALUE_GALVO = 0;

        private List<string> array;
        private string _alignFileBase;
        private string _alignFileFineBase;
        private string _alignFileFineName;
        private string _alignFileName;
        private bool _apply = false;
        private string _fileAlignmentBackup = string.Empty;
        private string _fileAlignmentFineBackup = string.Empty;
        private double _maxAlignValue;
        private double _minAlignValue;
        private double[] _xAxis = new double[FIELD_SIZE_TABLE_LENGTH];
        private double[] _yAxis = new double[FIELD_SIZE_TABLE_LENGTH];

        #endregion Fields

        #region Constructors

        public TwoWaySettings()
        {
            InitializeComponent();
            array = new List<String>();
            this.Loaded += new RoutedEventHandler(TwoWaySettings_Loaded);
            this.Closed += new EventHandler(TwoWaySettings_Closed);
        }

        #endregion Constructors

        #region Events

        public event Action<bool> EnableImaging;

        #endregion Events

        #region Properties

        public int AlignMode
        {
            get;
            set;
        }

        public bool Apply
        {
            get
            {
                return _apply;
            }
        }

        public double DwellMin
        {
            get;
            set;
        }

        public double DwellStep
        {
            get;
            set;
        }

        public double[] XAxis
        {
            get
            {
                return _xAxis;
            }
            set
            {
                if (value.Length == FIELD_SIZE_TABLE_LENGTH)
                {
                    _xAxis = value;
                }
                else
                {
                    MessageBox.Show("X axis invalid field size table length");
                }
            }
        }

        public double[] YAxis
        {
            get
            {
                return _yAxis;
            }
            set
            {
                if (value.Length == FIELD_SIZE_TABLE_LENGTH)
                {
                    _yAxis = value;
                }
                else
                {
                    MessageBox.Show("Y axis invalid field size table length");
                }
            }
        }

        #endregion Properties

        #region Methods

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "SetupCommand")]
        private static extern int LISetupCommand();

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "TeardownCommand")]
        private static extern int LITeardownCommand();

        [DllImport(".\\Modules_Native\\SelectHardware.dll", EntryPoint = "SetupCommand")]
        private static extern int SetupCommand();

        [DllImport(".\\Modules_Native\\SelectHardware.dll", EntryPoint = "TeardownCommand")]
        private static extern int TeardownCommand();

        private void btnDelete_Click(object sender, RoutedEventArgs e)
        {
            int Idx2Delete = lstRecPoints.SelectedIndex;
            if (Idx2Delete >= 0)
            {
                lstRecPoints.Items.RemoveAt(Idx2Delete);
                array.RemoveAt(Idx2Delete);
                UpdateGraph();
            }
        }

        private void btnRecord_Click(object sender, RoutedEventArgs e)
        {
            int offset = (Visibility.Visible == (Visibility)lblCoarseVisibility.Content) ? (int)lblTwoWayCoarse.Content : (int)lblTwoWay.Content;

            int lsmType = 0;
            ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_TYPE, ref lsmType);

            int selIndex = 0;

            if ((int)ICamera.LSMType.GALVO_GALVO == lsmType)
            {
                selIndex = Convert.ToInt32(lblDwellTimeIndex.Content);
            }
            else
            {
                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_FIELD_SIZE, ref selIndex);
            }

            string str2array = string.Format("{0},{1}", selIndex, offset);
            string str2lstRecPoints = string.Format("{0:0.0},{1}", XAxis[selIndex], offset);

            for (int i = 0; i < lstRecPoints.Items.Count; i++)
            {
                string itemStr = Convert.ToString(lstRecPoints.Items[i]);
                if (itemStr.StartsWith(string.Format("{0:0.0},", XAxis[selIndex])))
                {
                    lstRecPoints.Items.RemoveAt(i);
                    array.RemoveAt(i);
                }
            }
            lstRecPoints.Items.Add(str2lstRecPoints);
            array.Add(str2array);

            UpdateGraph();
        }

        private void Button_OnCancel(object sender, RoutedEventArgs e)
        {
            Close();
        }

        private void Button_OnOK(object sender, RoutedEventArgs e)
        {
            _apply = true;

            Close();
        }

        void TwoWaySettings_Closed(object sender, EventArgs e)
        {
            //disable imaging before tearing down to avoid errors
            //do at the top of the function to ensure effectivity
            if (null != EnableImaging) EnableImaging(false);

            try
            {
                bool selectHardware = false;

                if (true == _apply)
                {
                    if (null != _alignFileName && string.Empty != _alignFileName)
                    {
                        //output alignment file
                        //reselect camera
                        FileStream fs = File.Create(_alignFileName);

                        StreamWriter sw = new StreamWriter(fs);

                        for (int i = 0; i < _yAxis.Length; i++)
                        {
                            sw.WriteLine(_yAxis[i].ToString());
                        }

                        sw.Close();
                        fs.Close();
                        selectHardware = true;
                    }
                    if (null != _alignFileFineName && string.Empty != _alignFileFineName)
                    {
                        FileStream fs = File.Create(_alignFileFineName);

                        StreamWriter sw = new StreamWriter(fs);

                        for (int i = 0; i < _yAxis.Length; i++)
                        {
                            sw.WriteLine(_yAxis[i].ToString());
                        }

                        sw.Close();
                        fs.Close();
                        selectHardware = true;
                    }
                }
                else
                {
                    //reinstate the backup file if the dialog is canceled
                    if (null != _fileAlignmentBackup && string.Empty != _fileAlignmentBackup &&
                        null != _alignFileName && string.Empty != _alignFileName)
                    {
                        File.Move(_fileAlignmentBackup, _alignFileName);
                        selectHardware = true;
                    }

                    if (null != _fileAlignmentFineBackup && string.Empty != _fileAlignmentFineBackup &&
                        null != _alignFileFineName && string.Empty != _alignFileFineName)
                    {
                        File.Move(_fileAlignmentFineBackup, _alignFileFineName);
                        selectHardware = true;
                    }
                }

                if (selectHardware)
                {
                    //reselect the camera
                    //teardown LiveImageData first, this prevents the devices from reading while the devices are being disconnected
                    LITeardownCommand();
                    TeardownCommand();
                    SetupCommand();
                    LISetupCommand();
                }
            }
            catch (Exception ex)
            {
                ex.ToString();
            }

            //enable imaging after setup went through
            if (null != EnableImaging) EnableImaging(true);
        }

        void TwoWaySettings_Loaded(object sender, RoutedEventArgs e)
        {
            _apply = false;
            double dwell = DwellMin;

            for (int i = 0; i < FIELD_SIZE_TABLE_LENGTH; i++, dwell += DwellStep)
            {
                switch (AlignMode)
                {
                    case 2: _xAxis[i] = dwell; break;
                    default: _xAxis[i] = i; break;
                }
                _yAxis[i] = 0;
            }

            plot.XCoordinates = _xAxis;
            plot.YCoordinates = _yAxis;
            plot.YTitle = "Offset";

            switch (AlignMode)
            {
                case 0:
                    {
                        plot.XTitle = "Field Size";
                        _alignFileName = string.Empty;
                        _alignFileBase = string.Empty;
                        _alignFileFineName = FINE_FILE_NAME;
                        _alignFileFineBase = FINE_FILE_BASE;
                        _minAlignValue = MIN_ALIGN_VALUE;
                        _maxAlignValue = MAX_ALIGN_VALUE;
                    }
                    break;
                case 1:
                    {
                        plot.XTitle = "Field Size";
                        _alignFileName = COARSE_FILE_NAME;
                        _alignFileBase = COARSE_FILE_BASE;
                        _alignFileFineName = string.Empty;
                        _alignFileFineBase = string.Empty;
                        _minAlignValue = MIN_ALIGN_VALUE_COARSE;
                        _maxAlignValue = MAX_ALIGN_VALUE_COARSE;
                    }
                    break;
                case 2:
                    {
                        plot.XTitle = "Dwell Time";
                        _alignFileName = GALVO_FILE_NAME;
                        _alignFileBase = GALVO_FILE_BASE;
                        _alignFileFineName = string.Empty;
                        _alignFileFineBase = string.Empty;
                        _minAlignValue = MIN_ALIGN_VALUE_GALVO;
                        _maxAlignValue = MAX_ALIGN_VALUE_GALVO;
                    }
                    break;
            }

            if (File.Exists(_alignFileFineName))
            {
                _fileAlignmentFineBackup = string.Format(_alignFileFineBase + "_{0:yyyy_MM_dd_hh_mm_ss}.txt", DateTime.Now);
                File.Move(_alignFileFineName, _fileAlignmentFineBackup);
            }

            //reset the alignment table by renaming the AlignmentFile and reselecting the camera
            if (File.Exists(_alignFileName))
            {
                _fileAlignmentBackup = string.Format(_alignFileBase + "_{0:yyyy_MM_dd_hh_mm_ss}.txt", DateTime.Now);
                File.Move(_alignFileName, _fileAlignmentBackup);
            }

            //disable imaging before tearing down to avoid errors
            if (null != EnableImaging) EnableImaging(false);

            //reselect the camera. With the aligndata.txt missing the two values will repopulate with zeroes for each field size
            //teardown LiveImageData first, this prevents the devices from reading while the devices are being disconnected
            LITeardownCommand();
            TeardownCommand();
            SetupCommand();
            LISetupCommand();

            //enable imaging after setup went through
            if (null != EnableImaging) EnableImaging(true);

            plot.AnchorEnable = false;
            plot.SelectedAnchorIndex = 0;
            plot.YCoordinateMin = _minAlignValue;
            plot.YCoordinateMax = _maxAlignValue;
            plot.drawGraph();
        }

        private void UpdateGraph()
        {
            if (lstRecPoints.Items.Count < 2)
            {
                plot.drawGraph();
                return;
            }

            plot.AnchorEnable = false;

            //List<string> array = lstRecPoints.Items.OfType<string>().ToList();

            array.Sort(new TwoWayItemComparer());

            int xFirst = FIELD_SIZE_TABLE_LENGTH - 1;
            int xLast = 0;

            for (int i = 0; i < array.Count - 1; i++)
            {
                string pat = @"(.*),(.*)";
                string[] strResult = Regex.Split(array[i], pat);
                int x1 = Convert.ToInt32(strResult[1]);
                int y1 = Convert.ToInt32(strResult[2]);
                strResult = Regex.Split(array[i + 1], pat);
                int x2 = Convert.ToInt32(strResult[1]);
                int y2 = Convert.ToInt32(strResult[2]);

                xFirst = Math.Min(x1, xFirst);
                xLast = Math.Max(x2, xLast);

                for (int j = x1; j < x2; j++)
                {
                    _yAxis[j] = Math.Min(_maxAlignValue, Math.Max(_minAlignValue, Math.Round(((double)(y2 - y1) / (double)(x2 - x1)) * (j - x1) + y1)));
                }
            }

            //extrapolate the beginng of the dataset
            if (xFirst > 0)
            {
                string pat = @"(.*),(.*)";
                string[] strResult = Regex.Split(array[0], pat);
                int x1 = Convert.ToInt32(strResult[1]);
                int y1 = Convert.ToInt32(strResult[2]);
                strResult = Regex.Split(array[1], pat);
                int x2 = Convert.ToInt32(strResult[1]);
                int y2 = Convert.ToInt32(strResult[2]);

                for (int j = 0; j < x1; j++)
                {
                    _yAxis[j] = Math.Min(_maxAlignValue, Math.Max(_minAlignValue, Math.Round(((double)(y2 - y1) / (double)(x2 - x1)) * (j - x1) + y1)));
                }

            }

            //extrapolate the end of the dataset
            if (xLast < FIELD_SIZE_TABLE_LENGTH)
            {
                string pat = @"(.*),(.*)";
                string[] strResult = Regex.Split(array[array.Count - 2], pat);
                int x1 = Convert.ToInt32(strResult[1]);
                int y1 = Convert.ToInt32(strResult[2]);
                strResult = Regex.Split(array[array.Count - 1], pat);
                int x2 = Convert.ToInt32(strResult[1]);
                int y2 = Convert.ToInt32(strResult[2]);

                for (int j = x2; j < FIELD_SIZE_TABLE_LENGTH; j++)
                {
                    _yAxis[j] = Math.Min(_maxAlignValue, Math.Max(_minAlignValue, Math.Round(((double)(y2 - y1) / (double)(x2 - x1)) * (j - x1) + y1, 0)));
                }

            }
            plot.drawGraph();
        }

        #endregion Methods
    }
}