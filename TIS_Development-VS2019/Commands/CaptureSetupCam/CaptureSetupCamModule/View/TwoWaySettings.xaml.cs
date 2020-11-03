namespace CaptureSetupDll
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
    using System.ServiceModel;
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
    /// Interaction logic for SnapshotSettings.xaml
    /// </summary>
    public partial class TwoWaySettings : Window
    {
        #region Fields

        private const int FIELD_SIZE_TABLE_LENGTH = 256;
        private const double MAX_TWO_VALUE = 128;
        private const double MIN_TWO_VALUE = -128;

        private bool _apply = false;
        private string _fileAlignmentBackup = string.Empty;
        private double[] _xAxis = new double[FIELD_SIZE_TABLE_LENGTH];
        private double[] _yAxis = new double[FIELD_SIZE_TABLE_LENGTH];

        #endregion Fields

        #region Constructors

        public TwoWaySettings()
        {
            InitializeComponent();

            this.Loaded += new RoutedEventHandler(TwoWaySettings_Loaded);
            this.Closed += new EventHandler(TwoWaySettings_Closed);
        }

        #endregion Constructors

        #region Properties

        public int CurrentOffset
        {
            get;
            set;
        }

        public int SelectedIndex
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

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetupCommand")]
        private static extern int LISetupCommand();

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "TeardownCommand")]
        private static extern int LITeardownCommand();

        [DllImport(".\\Modules_Native\\SelectHardware.dll", EntryPoint = "SetupCommand")]
        private static extern int SetupCommand();

        [DllImport(".\\Modules_Native\\SelectHardware.dll", EntryPoint = "TeardownCommand")]
        private static extern int TeardownCommand();

        private void btnDelete_Click(object sender, RoutedEventArgs e)
        {
            if (lstRecPoints.SelectedIndex >= 0)
            {
                lstRecPoints.Items.RemoveAt(lstRecPoints.SelectedIndex);
                UpdateGraph();
            }
        }

        private void btnRecord_Click(object sender, RoutedEventArgs e)
        {
            string str = string.Format("{0},{1}",SelectedIndex,CurrentOffset);

            for (int i = 0; i < lstRecPoints.Items.Count; i++ )
            {
                string itemStr = Convert.ToString(lstRecPoints.Items[i]);
                if (itemStr.StartsWith(string.Format("{0},", SelectedIndex)))
                {
                    lstRecPoints.Items.RemoveAt(i);
                }
            }

            lstRecPoints.Items.Add(str);

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
            bool selectHardware = false;

            if (true == _apply)
            {
                //output alignment file
                //reselect camera
                FileStream fs = File.Create(".\\AlignData.txt");

                StreamWriter sw = new StreamWriter(fs);

                for (int i = 0; i < _yAxis.Length; i++)
                {
                    sw.WriteLine(_yAxis[i].ToString());
                }

                sw.Close();
                fs.Close();

                selectHardware = true;
            }
            else
            {
                //reinstate the backup file if the dialog is canceled
                if (_fileAlignmentBackup != string.Empty)
                {
                    File.Move(_fileAlignmentBackup, ".\\AlignData.txt");
                    selectHardware = true;
                }
            }

            if (selectHardware)
            {
                //reselect the camera
                TeardownCommand();
                SetupCommand();
                LITeardownCommand();
                LISetupCommand();
            }
        }

        void TwoWaySettings_Loaded(object sender, RoutedEventArgs e)
        {
            plot.XCoordinates = _xAxis;
            plot.YCoordinates = _yAxis;
            plot.XTitle = "Field Size";
            plot.YTitle = "Offset";

            for (int i = 0; i < FIELD_SIZE_TABLE_LENGTH; i++)
            {
                _xAxis[i] = i;
                _yAxis[i] = 0;
            }

            //reset the alignment table by renaming the AlignmentFile and reselecting the camera
            if (File.Exists(".\\AlignData.txt"))
            {
                _fileAlignmentBackup = string.Format("AlignData_{0:yyyy_MM_dd_hh_mm_ss}.txt", DateTime.Now);
                File.Move(".\\AlignData.txt", _fileAlignmentBackup);
            }

            //reselect the camera. With the aligndata.txt missing the two values will repopulate with zeroes for each field size
            TeardownCommand();
            SetupCommand();
            LITeardownCommand();
            LISetupCommand();

            plot.AnchorEnable = false;
            plot.SelectedAnchorIndex = 0;
            plot.YCoordinateMin = MIN_TWO_VALUE;
            plot.YCoordinateMax = MAX_TWO_VALUE;
            plot.drawGraph();
        }

        private void UpdateGraph()
        {
            if (lstRecPoints.Items.Count < 2)
            {
                return;
            }

            plot.AnchorEnable = false;

            List<string> array = lstRecPoints.Items.OfType<string>().ToList();

            array.Sort(new TwoWayItemComparer());

            int xFirst=FIELD_SIZE_TABLE_LENGTH-1;
            int xLast=0;

            for (int i = 0; i < array.Count-1; i++)
            {
                string pat = @"(.*),(.*)";
                string[] strResult = Regex.Split(array[i], pat);
                int x1 = Convert.ToInt32(strResult[1]);
                int y1 = Convert.ToInt32(strResult[2]);
                strResult = Regex.Split(array[i+1],pat);
                int x2 = Convert.ToInt32(strResult[1]);
                int y2 = Convert.ToInt32(strResult[2]);

                xFirst = Math.Min(x1,xFirst);
                xLast = Math.Max(x2,xLast);

                for(int j=x1; j<x2; j++)
                {
                    _yAxis[j] = Math.Min(MAX_TWO_VALUE,Math.Max(MIN_TWO_VALUE,Math.Round(((double)(y2 - y1) / (double)(x2 - x1)) * (j-x1) + y1)));
                }
            }

            //extrapolate the beginng of the dataset
            if(xFirst > 0)
            {
                string pat = @"(.*),(.*)";
                string[] strResult = Regex.Split(array[0], pat);
                int x1 = Convert.ToInt32(strResult[1]);
                int y1 = Convert.ToInt32(strResult[2]);
                strResult = Regex.Split(array[1],pat);
                int x2 = Convert.ToInt32(strResult[1]);
                int y2 = Convert.ToInt32(strResult[2]);

                for (int j = 0; j < x1; j++)
                {
                    _yAxis[j] = Math.Min(MAX_TWO_VALUE,Math.Max(MIN_TWO_VALUE,Math.Round(((double)(y2 - y1) / (double)(x2 - x1)) * (j - x1) + y1)));
                }

            }

            //extrapolate the end of the dataset
            if(xLast < FIELD_SIZE_TABLE_LENGTH)
            {
                string pat = @"(.*),(.*)";
                string[] strResult = Regex.Split(array[array.Count-2], pat);
                int x1 = Convert.ToInt32(strResult[1]);
                int y1 = Convert.ToInt32(strResult[2]);
                strResult = Regex.Split(array[array.Count - 1], pat);
                int x2 = Convert.ToInt32(strResult[1]);
                int y2 = Convert.ToInt32(strResult[2]);

                for (int j = x2; j < FIELD_SIZE_TABLE_LENGTH; j++)
                {
                    _yAxis[j] = Math.Min(MAX_TWO_VALUE,Math.Max(MIN_TWO_VALUE,Math.Round(((double)(y2 - y1) / (double)(x2 - x1)) * (j - x1) + y1,0)));
                }

            }
            plot.drawGraph();
        }

        #endregion Methods
    }
}