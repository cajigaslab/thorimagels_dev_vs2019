namespace LineProfileWindow
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Navigation;
    using System.Windows.Shapes;
    using System.Xml;

    using LineProfileWindow.ViewModel;

    using ThorLogging;

    public struct LineProfileData
    {
        #region Fields

        public long channelEnable;
        public int LengthPerChannel;
        public double PixeltoµmConversionFactor;

        public double[] profileDataX;

        public  double[][] profileDataY;

        #endregion Fields
    }

    /// <summary>
    /// Interaction logic for LineProfile.xaml
    /// </summary>
    public partial class LineProfile : Window
    {
        #region Fields

        LineProfileViewModel _vm;

        #endregion Fields

        #region Constructors

        public LineProfile(Color[] colorAssignment, int maxChannels)
        {
            InitializeComponent();

            _vm = new LineProfileViewModel(colorAssignment);
            _vm.MaxChannels = maxChannels;
            this.DataContext = _vm;
            _vm.LineWidthChange += _vm_LineWidthChange;
            try
            {
                // Make this window the topmost within the app
                this.Owner = Application.Current.MainWindow;
            }
            catch (Exception e)
            {
                e.ToString();
            }
            this.Closed += LineProfile_Closed;
        }

        #endregion Constructors

        #region Events

        public event Action<int> LineWidthChange;

        #endregion Events

        #region Properties

        public bool AutoScaleOption
        {
            get
            {
                return _vm.IsAutoScaleActive;
            }
            set
            {
                _vm.IsAutoScaleActive = value;
            }
        }

        public Color[] ColorAssigment
        {
            get
            {
                return _vm.ColorAssigment;
            }
            set
            {
                _vm.ColorAssigment = value;
            }
        }

        public bool ConversionActiveOption
        {
            get
            {
                return _vm.IsConversionActive;
            }
            set
            {
                _vm.IsConversionActive = value;
            }
        }

        public int LineWidth
        {
            get
            {
                return _vm.LineWidth;
            }
            set
            {
                _vm.LineWidth = Math.Max(1, value);
            }
        }

        public int LineWidthMax
        {
            get
            {
                return _vm.LineWidthMax;
            }
            set
            {
                _vm.LineWidthMax = value;
            }
        }

        public double MaximumYVal
        {
            get
            {
                return _vm.YmaxValue;
            }
            set
            {
                _vm.YmaxValue = value;
            }
        }

        public double MinimumYVal
        {
            get
            {
                return _vm.YminValue;
            }
            set
            {
                _vm.YminValue = value;
            }
        }

        #endregion Properties

        #region Methods

        

        public void SetData(LineProfileData lineprofileData)
        {
            _vm.SetData(lineprofileData);
        }

        void LineProfile_Closed(object sender, EventArgs e)
        {
            try
            {
                Application.Current.MainWindow.Activate();
            }
            catch (Exception ex)
            {
                ex.ToString();
            }
        }

        private void SaveAs_Click(object sender, RoutedEventArgs e)
        {
            _vm.SaveAs();
        }

        private void _vm_LineWidthChange(int obj)
        {
            LineWidthChange(obj);
        }

        #endregion Methods
    }
}