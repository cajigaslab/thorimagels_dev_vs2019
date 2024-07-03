namespace MCM6000_Control
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Globalization;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Navigation;
    using System.Windows.Shapes;
    using System.Windows.Threading;
    using System.Xml;

    public class DoubleCultureConverter : IValueConverter
    {
        #region Methods

        object IValueConverter.Convert(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            try
            {
                if (targetType == typeof(string))
                {
                    return (Double.Parse(value.ToString()).ToString());
                }
                else if (targetType == typeof(double))
                {
                    return (Double.Parse(value.ToString()));
                }
                else if (targetType == typeof(object))
                {
                    return (object)value.ToString();
                }
                else
                {
                    throw new InvalidOperationException("The target must be a string or double");
                }
            }
            catch (FormatException ex)
            {
                string str = ex.Message;
                return null;
            }
        }

        object IValueConverter.ConvertBack(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            try
            {
                if (targetType == typeof(string))
                {
                    return (Double.Parse(value.ToString())).ToString();
                }
                else if (targetType == typeof(double))
                {
                    return (Double.Parse(value.ToString()));
                }
                else if (targetType == typeof(object))
                {
                    return (object)value.ToString();
                }
                else
                {
                    throw new InvalidOperationException("The target must be a string or double");
                }
            }
            catch (FormatException ex)
            {
                string str = ex.Message;
                return null;
            }
        }

        #endregion Methods
    }

    /// <summary>
    /// Interaction logic for Window1.xaml
    /// </summary>
    public partial class Window1 : Window, INotifyPropertyChanged
    {
        #region Fields

        const int UM_TO_MM = 1000;

        private double rMax;
        private double rMin;
        private double rPos;
        private DispatcherTimer timer;
        private double xMax;
        private double xMin;
        private double xPos;
        private double yMax;
        private double yMin;
        private double yPos;
        private double zePos;
        private double zMax;
        private double zMin;
        private double zPos;
        private int _lightPathCamera;
        private int _lightPathGG;
        private int _lightPathGR;
        private double _rMoveByThreshold = 0;
        private double _rStepSize;
        private double _xMoveByThreshold = 0;
        private double _xStepSize;
        private double _yMoveByThreshold = 0;
        private double _yStepSize;
        private Visibility _zeVisibility = Visibility.Collapsed;
        private double _zMoveByThreshold = 0;
        private double _zStepSize;

        #endregion Fields

        #region Constructors

        public Window1()
        {
            InitializeComponent();
            timer = new DispatcherTimer();
            Application.Current.Exit += new ExitEventHandler(Current_Exit);
            this.Closing += new System.ComponentModel.CancelEventHandler(Window1_Closing);
        }

        #endregion Constructors

        #region Enumerations

        enum AxisType
        {
            X,
            Y,
            Z,
            R
        }

        #endregion Enumerations

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public bool EnableDeviceReading
        {
            get;
            set;
        }

        public int LightPathCamera
        {
            get
            {
                return _lightPathCamera;
            }
            set
            {
                _lightPathCamera = value;
                SetLighPath(0, _lightPathCamera);
                OnPropertyChanged("LightPathCamera");
            }
        }

        public int LightPathGG
        {
            get
            {
                return _lightPathGG;
            }
            set
            {
                _lightPathGG = value;
                SetLighPath(2, _lightPathGG);
                OnPropertyChanged("LightPathGG");
            }
        }

        public int LightPathGR
        {
            get
            {
                return _lightPathGR;
            }
            set
            {
                _lightPathGR = value;
                SetLighPath(1, _lightPathGR);
                OnPropertyChanged("LightPathGR");
            }
        }

        public double RMax
        {
            get
            {
                return this.rMax;
            }
            set
            {
                rMax = Math.Round(value, 4);
                OnPropertyChanged("RMax");
            }
        }

        public double RMin
        {
            get
            {
                return this.rMin;
            }
            set
            {
                rMin = Math.Round(value, 4);
                OnPropertyChanged("RMin");
            }
        }

        public double RPosition
        {
            get
            {
                return Math.Round(this.rPos, 4);
            }
            set
            {
                this.rPos = Math.Round(value, 4);

                OnPropertyChanged("RPosition");
            }
        }

        public bool RPosOutOfBounds
        {
            get
            {
                return ((RPosition > RMax) || (RPosition < RMin));
            }
        }

        public double RStepSize
        {
            get
            {
                return _rStepSize;
            }
            set
            {
                _rStepSize = value;
                OnPropertyChanged("RStepSize");
            }
        }

        public double XMax
        {
            get
            {
                return this.xMax;
            }
            set
            {
                xMax = Math.Round(value, 4);
                OnPropertyChanged("XMax");
            }
        }

        public double XMin
        {
            get
            {
                return this.xMin;
            }
            set
            {
                xMin = Math.Round(value, 4);
                OnPropertyChanged("XMin");
            }
        }

        public double XPosition
        {
            get
            {
                return Math.Round(this.xPos, 4);
            }
            set
            {
                this.xPos = Math.Round(value, 4);

                OnPropertyChanged("XPosition");
            }
        }

        public bool XPosOutOfBounds
        {
            get
            {
                return ((XPosition > XMax) || (XPosition < XMin));
            }
        }

        public double XStepSize
        {
            get
            {
                return _xStepSize;
            }
            set
            {
                _xStepSize = value;
                OnPropertyChanged("XStepSize");
            }
        }

        public double YMax
        {
            get
            {
                return this.yMax;
            }
            set
            {
                yMax = Math.Round(value, 4);
                OnPropertyChanged("YMax");
            }
        }

        public double YMin
        {
            get
            {
                return this.yMin;
            }
            set
            {
                yMin = Math.Round(value, 4);
                OnPropertyChanged("YMin");
            }
        }

        public double YPosition
        {
            get
            {
                return Math.Round(this.yPos, 4);
            }
            set
            {

                this.yPos = Math.Round(value, 4);

                OnPropertyChanged("YPosition");
            }
        }

        public bool YPosOutOfBounds
        {
            get
            {
                return ((YPosition > YMax) || (YPosition < YMin));
            }
        }

        public double YStepSize
        {
            get
            {
                return _yStepSize;
            }
            set
            {
                _yStepSize = value;
                OnPropertyChanged("YStepSize");
            }
        }

        public Visibility ZElevatorVis
        {
            get
            {
                return _zeVisibility;
            }
            set
            {
                _zeVisibility = value;
                OnPropertyChanged("ZElevatorVis");
            }
        }

        public double ZEPosition
        {
            get
            {
                return Math.Round(this.zePos, 4);
            }
            set
            {
                this.zePos = Math.Round(value, 4);

                OnPropertyChanged("ZEPosition");
            }
        }

        public bool ZEPosOutOfBounds
        {
            get
            {
                //In case we decide to add a Min and Max values in the settings file
                //for the Z elevator later
                //return ((ZEPosition > ZEMax) || (ZEPosition < ZEMin));
                return false;
            }
        }

        public double ZMax
        {
            get
            {
                return this.zMax;
            }
            set
            {
                zMax = Math.Round(value, 4);
                OnPropertyChanged("ZMax");
            }
        }

        public double ZMin
        {
            get
            {
                return this.zMin;
            }
            set
            {
                zMin = Math.Round(value, 4);
                OnPropertyChanged("ZMin");
            }
        }

        public double ZPosition
        {
            get
            {
                return Math.Round(this.zPos, 4);
            }
            set
            {
                this.zPos = Math.Round(value, 4);

                OnPropertyChanged("ZPosition");
            }
        }

        public bool ZPosOutOfBounds
        {
            get
            {
                return ((ZPosition > ZMax) || (ZPosition < ZMin));
            }
        }

        public double ZStepSize
        {
            get
            {
                return _zStepSize;
            }
            set
            {
                _zStepSize = value;
                OnPropertyChanged("ZStepSize");
            }
        }

        #endregion Properties

        #region Methods

        private void btnExit_Click(object sender, RoutedEventArgs e)
        {
            this.Close();
        }

        private void btnGoR_Click(object sender, RoutedEventArgs e)
        {
            if (txtRGoto.Text.Length > 0)
            {
                try
                {
                    moveToPosition(AxisType.R, Convert.ToDouble(txtRGoto.Text));
                    timer.Start();
                }
                catch (FormatException ex)
                {
                    string str = ex.Message;
                }
            }
        }

        private void btnGoX_Click(object sender, RoutedEventArgs e)
        {
            if (txtXGoto.Text.Length > 0)
            {
                try
                {
                    moveToPosition(AxisType.X, Convert.ToDouble(txtXGoto.Text));
                    timer.Start();
                }
                catch (FormatException ex)
                {
                    string str = ex.Message;
                }
            }
        }

        private void btnGoY_Click(object sender, RoutedEventArgs e)
        {
            if (txtYGoto.Text.Length > 0)
            {
                try
                {
                    moveToPosition(AxisType.Y, Convert.ToDouble(txtYGoto.Text));
                    timer.Start();
                }
                catch (FormatException ex)
                {
                    string str = ex.Message;
                }
            }
        }

        private void btnGoZ_Click(object sender, RoutedEventArgs e)
        {
            if (txtZGoto.Text.Length > 0)
            {
                try
                {
                    moveToPosition(AxisType.Z, Convert.ToDouble(txtZGoto.Text));
                    timer.Start();
                }
                catch (FormatException ex)
                {
                    string str = ex.Message;
                }
            }
        }

        private void btnRMinus_Click(object sender, RoutedEventArgs e)
        {
            if (_rMoveByThreshold >= RStepSize)
            {
                MoveByPosition(AxisType.R, (-1 * RStepSize));
            }
            else
            {
                double val = RPosition;
                val -= RStepSize;
                moveToPosition(AxisType.R, val);
            }
        }

        private void btnRPlus_Click(object sender, RoutedEventArgs e)
        {
            if (_rMoveByThreshold >= RStepSize)
            {
                MoveByPosition(AxisType.R, RStepSize);
            }
            else
            {
                double val = RPosition;
                val += RStepSize;
                moveToPosition(AxisType.R, val);
            }
        }

        private void btnSetZeroR_Click(object sender, RoutedEventArgs e)
        {
            if (rLabel.Content.ToString().Length > 0)
            {
                try
                {
                    setZero(AxisType.R, Convert.ToDouble(rLabel.Content.ToString()));
                    //moveToPosition(AxisType.R, 0);
                }
                catch (FormatException ex)
                {
                    string str = ex.Message;
                }
            }
        }

        private void btnSetZeroX_Click(object sender, RoutedEventArgs e)
        {
            if (xLabel.Content.ToString().Length > 0)
            {
                try
                {
                    setZero(AxisType.X, Convert.ToDouble(xLabel.Content.ToString()));
                    //moveToPosition(AxisType.X, 0);
                }
                catch (FormatException ex)
                {
                    string str = ex.Message;
                }
            }
        }

        private void btnSetZeroY_Click(object sender, RoutedEventArgs e)
        {
            if (yLabel.Content.ToString().Length > 0)
            {
                try
                {
                    setZero(AxisType.Y, Convert.ToDouble(yLabel.Content.ToString()));
                    //moveToPosition(AxisType.Y, 0);
                }
                catch (FormatException ex)
                {
                    string str = ex.Message;
                }
            }
        }

        private void btnSetZeroZ_Click(object sender, RoutedEventArgs e)
        {
            if (zLabel.Content.ToString().Length > 0)
            {
                try
                {
                    setZero(AxisType.Z, Convert.ToDouble(zLabel.Content.ToString()));
                    //moveToPosition(AxisType.Z, 0);
                }
                catch (FormatException ex)
                {
                    string str = ex.Message;
                }
            }
        }

        private void btnStopR_Click(object sender, RoutedEventArgs e)
        {
            int ret;
            int status = 0;//0 - busy, 1-ready, 2-error
            ret = MCM6000Functions.StatusPosition(ref status);
            if ((1 == ret) && (1 != status))
            {
                setRStop();
            }
        }

        private void btnStopX_Click(object sender, RoutedEventArgs e)
        {
            int ret;
            int status = 0;//0 - busy, 1-ready, 2-error
            ret = MCM6000Functions.StatusPosition(ref status);
            if ((1 == ret) && (1 != status))
            {
                setXStop();
            }
        }

        private void btnStopY_Click(object sender, RoutedEventArgs e)
        {
            int ret;
            int status = 0;//0 - busy, 1-ready, 2-error
            ret = MCM6000Functions.StatusPosition(ref status);
            if ((1 == ret) && (1 != status))
            {
                setYStop();
            }
        }

        private void btnStopZ_Click(object sender, RoutedEventArgs e)
        {
            int ret;
            int status = 0;//0 - busy, 1-ready, 2-error
            ret = MCM6000Functions.StatusPosition(ref status);
            if ((1 == ret) && (1 != status))
            {
                setZStop();
            }
        }

        private void btnXMinus_Click(object sender, RoutedEventArgs e)
        {
            if (_xMoveByThreshold >= XStepSize)
            {
                MoveByPosition(AxisType.X, (-1 * XStepSize));
            }
            else
            {
                double val = XPosition;
                val -= XStepSize;
                moveToPosition(AxisType.X, val);
            }
        }

        private void btnXPlus_Click(object sender, RoutedEventArgs e)
        {
            if (_xMoveByThreshold >= XStepSize)
            {
                MoveByPosition(AxisType.X, XStepSize);
            }
            else
            {
                double val = XPosition;
                val += XStepSize;
                moveToPosition(AxisType.X, val);
            }
        }

        private void btnYMinus_Click(object sender, RoutedEventArgs e)
        {
            if (_yMoveByThreshold >= YStepSize)
            {
                MoveByPosition(AxisType.Y, (-1 * YStepSize));
            }
            else
            {
                double val = YPosition;
                val -= YStepSize;
                moveToPosition(AxisType.Y, val);
            }
        }

        private void btnYPlus_Click(object sender, RoutedEventArgs e)
        {
            if (_yMoveByThreshold >= YStepSize)
            {
                MoveByPosition(AxisType.Y, YStepSize);
            }
            else
            {
                double val = YPosition;
                val += YStepSize;
                moveToPosition(AxisType.Y, val);
            }
        }

        private void btnZMinus_Click(object sender, RoutedEventArgs e)
        {
            if (_zMoveByThreshold >= ZStepSize)
            {
                MoveByPosition(AxisType.Z, (-1 * ZStepSize));
            }
            else
            {
                double val = ZPosition;
                val -= ZStepSize;
                moveToPosition(AxisType.Z, val);
            }
        }

        private void btnZPlus_Click(object sender, RoutedEventArgs e)
        {
            if (_zMoveByThreshold >= ZStepSize)
            {
                MoveByPosition(AxisType.Z, ZStepSize);
            }
            else
            {
                double val = ZPosition;
                val += ZStepSize;
                moveToPosition(AxisType.Z, val);
            }
        }

        void Current_Exit(object sender, ExitEventArgs e)
        {
            this.Close();
            Application.Current.Shutdown();
        }

        private void MoveByPosition(AxisType axisType, double pos)
        {
            switch (axisType)
            {
                case AxisType.X: MCM6000Functions.SetParam(Convert.ToInt32(DeviceParams.PARAM_X_POS_MOVE_BY), pos); break;
                case AxisType.Y: MCM6000Functions.SetParam(Convert.ToInt32(DeviceParams.PARAM_Y_POS_MOVE_BY), pos); break;
                case AxisType.Z: MCM6000Functions.SetParam(Convert.ToInt32(DeviceParams.PARAM_Z_POS_MOVE_BY), pos); break;
                case AxisType.R: MCM6000Functions.SetParam(Convert.ToInt32(DeviceParams.PARAM_R_POS_MOVE_BY), pos); break;
            }

            MCM6000Functions.PreflightPosition();
            MCM6000Functions.SetupPosition();
            MCM6000Functions.StartPosition();
            MCM6000Functions.PostflightPosition();
        }

        private void moveToPosition(AxisType axisType, double pos)
        {
            switch (axisType)
            {
                case AxisType.X: MCM6000Functions.SetParam(Convert.ToInt32(DeviceParams.PARAM_X_POS), pos); break;
                case AxisType.Y: MCM6000Functions.SetParam(Convert.ToInt32(DeviceParams.PARAM_Y_POS), pos); break;
                case AxisType.Z: MCM6000Functions.SetParam(Convert.ToInt32(DeviceParams.PARAM_Z_POS), pos); break;
                case AxisType.R: MCM6000Functions.SetParam(Convert.ToInt32(DeviceParams.PARAM_R_POS), pos); break;
            }

            MCM6000Functions.PreflightPosition();
            MCM6000Functions.SetupPosition();
            MCM6000Functions.StartPosition();
            MCM6000Functions.PostflightPosition();
        }

        private void OnPropertyChanged(string propertyName)
        {
            if (PropertyChanged != null)
            {
                this.PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }

        private void rbCoarseR_Click(object sender, RoutedEventArgs e)
        {
            double stepSire = RStepSize;
            RStepSize = stepSire * 10.0;
        }

        private void rbCoarseX_Click(object sender, RoutedEventArgs e)
        {
            double stepSixe = XStepSize;
            XStepSize = stepSixe * 10.0;
        }

        private void rbCoarseY_Click(object sender, RoutedEventArgs e)
        {
            double stepSiye = YStepSize;
            YStepSize = stepSiye * 10.0;
        }

        private void rbCoarseZ_Click(object sender, RoutedEventArgs e)
        {
            double stepSize = ZStepSize;
            ZStepSize = stepSize * 10.0;
        }

        private void rbFineR_Click(object sender, RoutedEventArgs e)
        {
            double stepSire = RStepSize;
            RStepSize = stepSire / 10.0;
        }

        private void rbFineX_Click(object sender, RoutedEventArgs e)
        {
            double stepSixe = XStepSize;
            XStepSize = stepSixe / 10.0;
        }

        private void rbFineY_Click(object sender, RoutedEventArgs e)
        {
            double stepSiye = YStepSize;
            YStepSize = stepSiye / 10.0;
        }

        private void rbFineZ_Click(object sender, RoutedEventArgs e)
        {
            double stepSize = ZStepSize;
            ZStepSize = stepSize / 10.0;
        }

        private void rSlider_DragCompleted(object sender, System.Windows.Controls.Primitives.DragCompletedEventArgs e)
        {
            this.EnableDeviceReading = true;
        }

        private void rSlider_DragStarted(object sender, System.Windows.Controls.Primitives.DragStartedEventArgs e)
        {
            this.EnableDeviceReading = false;
        }

        private void rSlider_LostMouseCapture(object sender, MouseEventArgs e)
        {
            double newVal = Math.Round(((Slider)e.Source).Value, 4);
            BindingExpression be = BindingOperations.GetBindingExpression(this.rSlider, Slider.ValueProperty);
            be.UpdateSource();
            moveToPosition(AxisType.R, RPosition);
        }

        private void rSlider_MouseWheel(object sender, MouseWheelEventArgs e)
        {
            double newVal = rSlider.Value;

            if (e.Delta > 0)
            {
                newVal += RStepSize;
            }
            else if (e.Delta < 0)
            {
                newVal -= RStepSize;
            }

            moveToPosition(AxisType.R, newVal);
            BindingExpression be = rLabel.GetBindingExpression(Label.ContentProperty);
            be.UpdateSource();

            e.Handled = true;
        }

        private void SetEnabledState(bool val)
        {
            zSlider.IsEnabled = val;
            btnGoZ.IsEnabled = val;
            btnStopZ.IsEnabled = val;
            btnSetZeroZ.IsEnabled = val;
            xSlider.IsEnabled = val;
            btnGoX.IsEnabled = val;
            btnStopX.IsEnabled = val;
            btnSetZeroX.IsEnabled = val;
            ySlider.IsEnabled = val;
            btnGoY.IsEnabled = val;
            btnStopY.IsEnabled = val;
            btnSetZeroY.IsEnabled = val;
            rSlider.IsEnabled = val;
            btnGoR.IsEnabled = val;
            btnStopR.IsEnabled = val;
            btnSetZeroR.IsEnabled = val;
            rbCamera.IsEnabled = val;
            rbGR.IsEnabled = val;
            rbGG.IsEnabled = val;
        }

        private void SetLighPath(int val, int state)
        {
            MCM6000Functions.PreflightPosition();

            switch (val)
            {
                case 0: MCM6000Functions.SetParam(Convert.ToInt32(DeviceParams.PARAM_LIGHTPATH_CAMERA), state); break;
                case 1: MCM6000Functions.SetParam(Convert.ToInt32(DeviceParams.PARAM_LIGHTPATH_GR), state); break;
                case 2: MCM6000Functions.SetParam(Convert.ToInt32(DeviceParams.PARAM_LIGHTPATH_GG), state); break;
            }
            MCM6000Functions.SetupPosition();
            MCM6000Functions.StartPosition();
        }

        private void setRRero(double r)
        {
            MCM6000Functions.PreflightPosition();
            MCM6000Functions.SetParam(Convert.ToInt32(DeviceParams.PARAM_R_ZERO), 1); // Set the current R position to hardware Rero
            MCM6000Functions.SetupPosition();
            MCM6000Functions.StartPosition();

            int ret;
            int status = 0;//0 - busy, 1-ready, 2-error
            do
            {
                ret = MCM6000Functions.StatusPosition(ref status);
            } while ((1 == ret) && (1 != status));

            MCM6000Functions.PostflightPosition();
        }

        private void setRStop()
        {
            MCM6000Functions.PreflightPosition();
            MCM6000Functions.SetParam(Convert.ToInt32(DeviceParams.PARAM_R_STOP), 1);
            MCM6000Functions.SetupPosition();
            MCM6000Functions.StartPosition();
            MCM6000Functions.GetParam(Convert.ToInt32(DeviceParams.PARAM_R_POS_CURRENT), ref rPos);
            RPosition = rPos;
        }

        private void setXStop()
        {
            MCM6000Functions.PreflightPosition();
            MCM6000Functions.SetParam(Convert.ToInt32(DeviceParams.PARAM_X_STOP), 1);
            MCM6000Functions.SetupPosition();
            MCM6000Functions.StartPosition();
            MCM6000Functions.GetParam(Convert.ToInt32(DeviceParams.PARAM_X_POS_CURRENT), ref xPos);
            XPosition = xPos;
        }

        private void setXXero(double x)
        {
            MCM6000Functions.PreflightPosition();
            MCM6000Functions.SetParam(Convert.ToInt32(DeviceParams.PARAM_X_ZERO), 1); // Set the current X position to hardware Xero
            MCM6000Functions.SetupPosition();
            MCM6000Functions.StartPosition();

            int ret;
            int status = 0;//0 - busy, 1-ready, 2-error
            do
            {
                ret = MCM6000Functions.StatusPosition(ref status);
            } while ((1 == ret) && (1 != status));

            MCM6000Functions.PostflightPosition();
        }

        private void setYStop()
        {
            MCM6000Functions.PreflightPosition();
            MCM6000Functions.SetParam(Convert.ToInt32(DeviceParams.PARAM_Y_STOP), 1);
            MCM6000Functions.SetupPosition();
            MCM6000Functions.StartPosition();
            MCM6000Functions.GetParam(Convert.ToInt32(DeviceParams.PARAM_Y_POS_CURRENT), ref yPos);
            YPosition = yPos;
        }

        private void setYYero(double y)
        {
            MCM6000Functions.PreflightPosition();
            MCM6000Functions.SetParam(Convert.ToInt32(DeviceParams.PARAM_Y_ZERO), 1); // Set the current Y position to hardware Yero
            MCM6000Functions.SetupPosition();
            MCM6000Functions.StartPosition();

            int ret;
            int status = 0;//0 - busy, 1-ready, 2-error
            do
            {
                ret = MCM6000Functions.StatusPosition(ref status);
            } while ((1 == ret) && (1 != status));

            MCM6000Functions.PostflightPosition();
        }

        private void setZero(AxisType axisType, double z)
        {
            switch (axisType)
            {
                case AxisType.X: MCM6000Functions.SetParam(Convert.ToInt32(DeviceParams.PARAM_X_ZERO), 1); break;
                case AxisType.Y: MCM6000Functions.SetParam(Convert.ToInt32(DeviceParams.PARAM_Y_ZERO), 1); break;
                case AxisType.Z: MCM6000Functions.SetParam(Convert.ToInt32(DeviceParams.PARAM_Z_ZERO), 1); break;
                case AxisType.R: MCM6000Functions.SetParam(Convert.ToInt32(DeviceParams.PARAM_R_ZERO), 1); break;
            }
            MCM6000Functions.PreflightPosition();
            MCM6000Functions.SetupPosition();
            MCM6000Functions.StartPosition();

            int ret;
            int status = 0;//0 - busy, 1-ready, 2-error
            do
            {
                ret = MCM6000Functions.StatusPosition(ref status);
            } while ((1 == ret) && (1 != status));

            MCM6000Functions.PostflightPosition();
        }

        private void setZStop()
        {
            MCM6000Functions.PreflightPosition();
            MCM6000Functions.SetParam(Convert.ToInt32(DeviceParams.PARAM_Z_STOP), 1);
            MCM6000Functions.SetupPosition();
            MCM6000Functions.StartPosition();
            MCM6000Functions.GetParam(Convert.ToInt32(DeviceParams.PARAM_Z_POS_CURRENT), ref zPos);
            ZPosition = zPos;
        }

        void timer_Tick(object sender, EventArgs e)
        {
            if (EnableDeviceReading == true)
            {
                MCM6000Functions.GetParam(Convert.ToInt32(DeviceParams.PARAM_Z_POS_CURRENT), ref zPos);
                ZPosition = zPos;

                MCM6000Functions.GetParam(Convert.ToInt32(DeviceParams.PARAM_X_POS_CURRENT), ref xPos);
                XPosition = xPos;

                MCM6000Functions.GetParam(Convert.ToInt32(DeviceParams.PARAM_Y_POS_CURRENT), ref yPos);
                YPosition = yPos;

                MCM6000Functions.GetParam(Convert.ToInt32(DeviceParams.PARAM_R_POS_CURRENT), ref rPos);
                RPosition = rPos;

                MCM6000Functions.GetParam(Convert.ToInt32(DeviceParams.PARAM_Z_ELEVATOR_POS_CURRENT), ref zePos);
                ZEPosition = zePos;

                double lpPos = 0;
                MCM6000Functions.GetParam(Convert.ToInt32(DeviceParams.PARAM_LIGHTPATH_CAMERA), ref lpPos);
                _lightPathCamera = Convert.ToInt32(lpPos);
                OnPropertyChanged("LightPathCamera");

                MCM6000Functions.GetParam(Convert.ToInt32(DeviceParams.PARAM_LIGHTPATH_GR), ref lpPos);
                _lightPathGR = Convert.ToInt32(lpPos);
                OnPropertyChanged("LightPathGR");

                MCM6000Functions.GetParam(Convert.ToInt32(DeviceParams.PARAM_LIGHTPATH_GG), ref lpPos);
                _lightPathGG = Convert.ToInt32(lpPos);
                OnPropertyChanged("LightPathGG");

                OnPropertyChanged("XPosOutOfBounds");
                OnPropertyChanged("YPosOutOfBounds");
                OnPropertyChanged("ZPosOutOfBounds");
                OnPropertyChanged("RPosOutOfBounds");
            }
        }

        void Window1_Closing(object sender, CancelEventArgs e)
        {
            Application.Current.Shutdown();
        }

        void Window1_Loaded(object sender, RoutedEventArgs e)
        {
            SetEnabledState(false);

            try
            {
                int dev = 0;
                //find the devices
                MCM6000Functions.FindDevices(ref dev);

                if (dev == 0)
                {
                    MessageBox.Show("No device found!");
                    return;
                }

                //Select the devices
                if (0 == MCM6000Functions.SelectDevice(0))
                {
                    MessageBox.Show("Could not connect to MCM6000, please make sure there isn't any other application connected to it.");
                    return;
                }

                int available, type, readOnly;
                double posMin, posMax, posDef;

                available = type = readOnly = 0;
                posMin = posMax = posDef = 0;

                MCM6000Functions.GetParamInfo(Convert.ToInt32(DeviceParams.PARAM_Z_POS), ref type, ref available, ref readOnly, ref posMin, ref posMax, ref posDef);
                ZMin = posMin;
                ZMax = posMax;
                ZStepSize = 0.01;

                MCM6000Functions.GetParamInfo(Convert.ToInt32(DeviceParams.PARAM_X_POS), ref type, ref available, ref readOnly, ref posMin, ref posMax, ref posDef);
                XMin = posMin;
                XMax = posMax;
                XStepSize = 0.01;

                MCM6000Functions.GetParamInfo(Convert.ToInt32(DeviceParams.PARAM_Y_POS), ref type, ref available, ref readOnly, ref posMin, ref posMax, ref posDef);
                YMin = posMin;
                YMax = posMax;
                YStepSize = 0.01;

                MCM6000Functions.GetParamInfo(Convert.ToInt32(DeviceParams.PARAM_R_POS), ref type, ref available, ref readOnly, ref posMin, ref posMax, ref posDef);
                RMin = posMin;
                RMax = posMax;
                RStepSize = 0.1;

                MCM6000Functions.GetParamInfo(Convert.ToInt32(DeviceParams.PARAM_Z_ELEVATOR_POS_CURRENT), ref type, ref available, ref readOnly, ref posMin, ref posMax, ref posDef);
                ZElevatorVis = (1 == available) ? Visibility.Visible : Visibility.Collapsed; ;

                EnableDeviceReading = true;

                timer.Tick += new EventHandler(timer_Tick);
                timer.Interval = new TimeSpan(0, 0, 0, 0, 250);
                timer.Start();

                SetEnabledState(true);

                string settingsFile = Environment.CurrentDirectory + "\\ThorMCM6000Settings.xml";
                XmlDocument settingsXML = new XmlDocument();
                settingsXML.Load(settingsFile.ToString());

                XmlNodeList ndList = settingsXML.SelectNodes("/MCM6000Settings/XRangeConfig");
                if (ndList.Count > 0)
                {
                    XmlNode node = ndList.Item(0);
                    _xMoveByThreshold = Convert.ToDouble(ndList.Item(0).Attributes["moveByThresholduM"].Value.ToString()) / UM_TO_MM;
                }
                ndList = settingsXML.SelectNodes("/MCM6000Settings/YRangeConfig");
                if (ndList.Count > 0)
                {
                    XmlNode node = ndList.Item(0);
                    _yMoveByThreshold = Convert.ToDouble(ndList.Item(0).Attributes["moveByThresholduM"].Value.ToString()) / UM_TO_MM;
                }
                ndList = settingsXML.SelectNodes("/MCM6000Settings/ZRangeConfig");
                if (ndList.Count > 0)
                {
                    XmlNode node = ndList.Item(0);
                    _zMoveByThreshold = Convert.ToDouble(ndList.Item(0).Attributes["moveByThresholduM"].Value.ToString()) / UM_TO_MM;
                }
                ndList = settingsXML.SelectNodes("/MCM6000Settings/RRangeConfig");
                if (ndList.Count > 0)
                {
                    XmlNode node = ndList.Item(0);
                    _rMoveByThreshold = Convert.ToDouble(ndList.Item(0).Attributes["moveByThresholduM"].Value.ToString()) / UM_TO_MM;
                }

            }
            catch (DllNotFoundException ex)
            {
                MessageBox.Show(string.Format("The dll {0} was not found! ({1})!!!!!!!!!!!!!!", MCM6000Functions.DLL_NAME, ex.Message));
            }
            catch (SEHException ex)
            {
                MessageBox.Show(string.Format("The device talking to {0} was not found! ({1})!!!!!!!!!!!!!!", MCM6000Functions.DLL_NAME, ex.Message), "Device Not Found");
            }
        }

        void Window1_Unloaded(object sender, RoutedEventArgs e)
        {
            timer.Stop();
            MCM6000Functions.TeardownDevice();
        }

        private void xSlider_DragCompleted(object sender, System.Windows.Controls.Primitives.DragCompletedEventArgs e)
        {
            this.EnableDeviceReading = true;
        }

        private void xSlider_DragStarted(object sender, System.Windows.Controls.Primitives.DragStartedEventArgs e)
        {
            this.EnableDeviceReading = false;
        }

        private void xSlider_LostMouseCapture(object sender, MouseEventArgs e)
        {
            double newVal = Math.Round(((Slider)e.Source).Value, 4);
            BindingExpression be = BindingOperations.GetBindingExpression(this.xSlider, Slider.ValueProperty);
            be.UpdateSource();
            moveToPosition(AxisType.X, XPosition);
        }

        private void xSlider_MouseWheel(object sender, MouseWheelEventArgs e)
        {
            double newVal = xSlider.Value;

            if (e.Delta > 0)
            {
                newVal += XStepSize;
            }
            else if (e.Delta < 0)
            {
                newVal -= XStepSize;
            }

            moveToPosition(AxisType.X, newVal);
            BindingExpression be = xLabel.GetBindingExpression(Label.ContentProperty);
            be.UpdateSource();

            e.Handled = true;
        }

        private void ySlider_DragCompleted(object sender, System.Windows.Controls.Primitives.DragCompletedEventArgs e)
        {
            this.EnableDeviceReading = true;
        }

        private void ySlider_DragStarted(object sender, System.Windows.Controls.Primitives.DragStartedEventArgs e)
        {
            this.EnableDeviceReading = false;
        }

        private void ySlider_LostMouseCapture(object sender, MouseEventArgs e)
        {
            double newVal = Math.Round(((Slider)e.Source).Value, 4);
            BindingExpression be = BindingOperations.GetBindingExpression(this.ySlider, Slider.ValueProperty);
            be.UpdateSource();
            moveToPosition(AxisType.Y, YPosition);
        }

        private void ySlider_MouseWheel(object sender, MouseWheelEventArgs e)
        {
            double newVal = ySlider.Value;

            if (e.Delta > 0)
            {
                newVal += YStepSize;
            }
            else if (e.Delta < 0)
            {
                newVal -= YStepSize;
            }

            moveToPosition(AxisType.Y, newVal);
            BindingExpression be = yLabel.GetBindingExpression(Label.ContentProperty);
            be.UpdateSource();

            e.Handled = true;
        }

        private void zSlider_DragCompleted(object sender, System.Windows.Controls.Primitives.DragCompletedEventArgs e)
        {
            this.EnableDeviceReading = true;
        }

        private void zSlider_DragStarted(object sender, System.Windows.Controls.Primitives.DragStartedEventArgs e)
        {
            this.EnableDeviceReading = false;
        }

        private void zSlider_LostMouseCapture(object sender, MouseEventArgs e)
        {
            double newVal = Math.Round(((Slider)e.Source).Value, 4);
            BindingExpression be = BindingOperations.GetBindingExpression(this.zSlider, Slider.ValueProperty);
            be.UpdateSource();
            moveToPosition(AxisType.Z, ZPosition);
        }

        private void zSlider_MouseWheel(object sender, MouseWheelEventArgs e)
        {
            double newVal = zSlider.Value;

            if (e.Delta > 0)
            {
                newVal += ZStepSize;
            }
            else if (e.Delta < 0)
            {
                newVal -= ZStepSize;
            }

            moveToPosition(AxisType.Z, newVal);
            BindingExpression be = zLabel.GetBindingExpression(Label.ContentProperty);
            be.UpdateSource();
            e.Handled = true;
        }

        #endregion Methods
    }
}