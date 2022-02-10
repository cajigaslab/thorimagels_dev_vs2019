namespace AreaControl
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.IO;
    using System.Linq;
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

    using Microsoft.Win32;

    /// <summary>
    /// Interaction logic for NyquistCalculator.xaml
    /// </summary>
    public partial class NyquistCalculator : Window, INotifyPropertyChanged
    {
        #region Fields

        public double NYQUIST_2P_CONST = Math.Sqrt(Math.Log10(2.0) / Math.Log10(Math.E));

        private int _emissionWavelength = 488;
        private int _excitationWavelength = 488;
        private double _indexOfRefraction = 1.33;
        private double _numericalAperture = 1.0;
        private double _pinholeSizeUM = 1.0;
        private int _pixelX2P = 512;
        private int _pixelXConfocal = 512;
        private int _pixelY2P = 512;
        private int _pixelYConfocal = 512;

        #endregion Fields

        #region Constructors

        public NyquistCalculator()
        {
            InitializeComponent();
            DataContext = this;

            this.Loaded += NyquistCalculator_Loaded;
            this.KeyDown += NyquistCalculator_KeyDown;
        }

        #endregion Constructors

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public double AspectRatio
        {
            get;
            set;
        }

        public bool ConfocalSettingsValid
        {
            get
            {
                return !(LsmPixelYConfocalOutOfRange || LsmPixelXConfocalOutOfRange);
            }
        }

        public int EmissionWavelength
        {
            get
            {
                return _emissionWavelength;
            }
            set
            {
                _emissionWavelength = value;
                OnPropertyChanged("IndexOfRefraction");
                OnPropertyChanged("UMPerPixelXConfocal");
                OnPropertyChanged("PixelXConfocal");
                OnPropertyChanged("UMPerPixelYConfocal");
                OnPropertyChanged("PixelYConfocal");
                OnPropertyChanged("UMPerStepZConfocal");

                OnPropertyChanged("UMPerPixelX2P");
                OnPropertyChanged("PixelX2P");
                OnPropertyChanged("UMPerPixelY2P");
                OnPropertyChanged("PixelY2P");
                OnPropertyChanged("UMPerStepZ2P");
                RaisePixelRecalculatedEvents();

                ;
            }
        }

        public int ExcitationWavelength
        {
            get
            {

                return _excitationWavelength;
            }
            set
            {
                _excitationWavelength = value;
                OnPropertyChanged("IndexOfRefraction");
                OnPropertyChanged("UMPerPixelXConfocal");
                OnPropertyChanged("PixelXConfocal");
                OnPropertyChanged("UMPerPixelYConfocal");
                OnPropertyChanged("PixelYConfocal");
                OnPropertyChanged("UMPerStepZConfocal");

                OnPropertyChanged("UMPerPixelX2P");
                OnPropertyChanged("PixelX2P");
                OnPropertyChanged("UMPerPixelY2P");
                OnPropertyChanged("PixelY2P");
                OnPropertyChanged("UMPerStepZ2P");
                RaisePixelRecalculatedEvents();
            }
        }

        public int FieldSize
        {
            get;
            set;
        }

        public double FieldSizeCalibration
        {
            get;
            set;
        }

        public double IndexOfRefraction
        {
            get
            {
                return _indexOfRefraction;
            }
            set
            {
                _indexOfRefraction = value;
                OnPropertyChanged("IndexOfRefraction");
                OnPropertyChanged("UMPerPixelXConfocal");
                OnPropertyChanged("PixelXConfocal");
                OnPropertyChanged("UMPerPixelYConfocal");
                OnPropertyChanged("PixelYConfocal");
                OnPropertyChanged("UMPerStepZConfocal");

                OnPropertyChanged("UMPerPixelX2P");
                OnPropertyChanged("PixelX2P");
                OnPropertyChanged("UMPerPixelY2P");
                OnPropertyChanged("PixelY2P");
                OnPropertyChanged("UMPerStepZ2P");
                RaisePixelRecalculatedEvents();
            }
        }

        public bool LsmPixelXConfocalOutOfRange
        {
            get
            {
                if (PixelXConfocal > LsmPixelXMax)
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
        }

        public double LsmPixelXMax
        {
            get;
            set;
        }

        public bool LsmPixelXMultiphotonOutOfRange
        {
            get
            {
                if (PixelX2P > LsmPixelXMax)
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
        }

        public bool LsmPixelYConfocalOutOfRange
        {
            get
            {
                if (PixelYConfocal > LsmPixelYMax)
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
        }

        public double LsmPixelYMax
        {
            get;
            set;
        }

        public bool LsmPixelYMultiphotonOutOfRange
        {
            get
            {
                if (PixelY2P > LsmPixelYMax)
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
        }

        public double Magnification
        {
            get;
            set;
        }

        public int Modality
        {
            get;
            set;
        }

        public bool MultiPhotonSettingsValid
        {
            get
            {
                return !(LsmPixelYMultiphotonOutOfRange || LsmPixelXMultiphotonOutOfRange);
            }
        }

        public double NumericalAperture
        {
            get
            {
                return _numericalAperture;
            }
            set
            {
                _numericalAperture = value;
                OnPropertyChanged("IndexOfRefraction");
                OnPropertyChanged("UMPerPixelXConfocal");
                OnPropertyChanged("PixelXConfocal");
                OnPropertyChanged("UMPerPixelYConfocal");
                OnPropertyChanged("PixelYConfocal");
                OnPropertyChanged("UMPerStepZConfocal");

                OnPropertyChanged("UMPerPixelX2P");
                OnPropertyChanged("PixelX2P");
                OnPropertyChanged("UMPerPixelY2P");
                OnPropertyChanged("PixelY2P");
                OnPropertyChanged("UMPerStepZ2P");
                RaisePixelRecalculatedEvents();
            }
        }

        public double PinholeSizeUM
        {
            get
            {
                return _pinholeSizeUM;
            }
            set
            {
                _pinholeSizeUM = value;
                OnPropertyChanged("PinholeSizeUM");

                OnPropertyChanged("IndexOfRefraction");
                OnPropertyChanged("UMPerPixelXConfocal");
                OnPropertyChanged("PixelXConfocal");
                OnPropertyChanged("UMPerPixelYConfocal");
                OnPropertyChanged("PixelYConfocal");
                OnPropertyChanged("UMPerStepZConfocal");

                OnPropertyChanged("UMPerPixelX2P");
                OnPropertyChanged("PixelX2P");
                OnPropertyChanged("UMPerPixelY2P");
                OnPropertyChanged("PixelY2P");
                OnPropertyChanged("UMPerStepZ2P");
                RaisePixelRecalculatedEvents();
            }
        }

        public int PixelX2P
        {
            get
            {
                //round to a 32 boudary
                _pixelX2P = 0x7FFFFFE0 & (int)Math.Floor(0x1F + (FieldSize * FieldSizeCalibration) / (UMPerPixelX2P * Magnification));

                return _pixelX2P;
            }
        }

        public int PixelXConfocal
        {
            get
            {
                //round to a 32 boudary
                _pixelXConfocal = 0x7FFFFFE0 & (int)Math.Floor(0x1F + (FieldSize * FieldSizeCalibration) / (UMPerPixelXConfocal * Magnification));

                return _pixelXConfocal;
            }
        }

        public int PixelY2P
        {
            get
            {
                //round to a 32 boudary
                _pixelY2P = 0x7FFFFFE0 & (int)Math.Floor(0x1F + AspectRatio * (0x7FFFFFE0 & (int)Math.Floor(0x1F + (FieldSize * FieldSizeCalibration) / (UMPerPixelY2P * Magnification))));

                return _pixelY2P;
            }
        }

        public int PixelYConfocal
        {
            get
            {

                //round to a 32 boudary
                _pixelYConfocal = 0x7FFFFFE0 & (int)Math.Floor(0x1F + AspectRatio * (0x7FFFFFE0 & (int)Math.Floor(0x1F + (FieldSize * FieldSizeCalibration) / (UMPerPixelYConfocal * Magnification))));

                return _pixelYConfocal;
            }
        }

        public double UMPerPixelX2P
        {
            get
            {
                if (NumericalAperture <= 0.7)
                {
                    return Math.Round(NYQUIST_2P_CONST * 0.320 * ExcitationWavelength / 1000.0 / (Math.Sqrt(2) * NumericalAperture), 3);
                }
                else
                {
                    return Math.Round(NYQUIST_2P_CONST * 0.325 * ExcitationWavelength / 1000.0 / (Math.Sqrt(2) * Math.Pow(NumericalAperture, 0.91)), 3);
                }
            }
        }

        public double UMPerPixelXConfocal
        {
            get
            {
                return Math.Round(.51 * ExcitationWavelength / (1000 * 2 * NumericalAperture), 3);
            }
        }

        public double UMPerPixelY2P
        {
            get
            {
                if (NumericalAperture <= 0.7)
                {
                    return Math.Round(NYQUIST_2P_CONST * 0.320 * ExcitationWavelength / 1000.0 / (Math.Sqrt(2) * NumericalAperture), 3);
                }
                else
                {
                    return Math.Round(NYQUIST_2P_CONST * 0.325 * ExcitationWavelength / 1000.0 / (Math.Sqrt(2) * Math.Pow(NumericalAperture, 0.91)), 3);
                }

            }
        }

        public double UMPerPixelYConfocal
        {
            get
            {
                return Math.Round(.51 * ExcitationWavelength / (1000 * 2 * NumericalAperture), 3);
            }
        }

        public double UMPerStepZ2P
        {
            get
            {
                double termA = (NYQUIST_2P_CONST * 0.532 * ExcitationWavelength / 1000.0) / Math.Sqrt(2);
                double termB = 1 / (IndexOfRefraction - Math.Sqrt(IndexOfRefraction * IndexOfRefraction - NumericalAperture * NumericalAperture));
                return Math.Round(termA * termB, 3);
            }
        }

        public double UMPerStepZConfocal
        {
            get
            {
                const double SCANLENSFOCALLENGTH = 70;
                const double PINHOLECOLLECTORLENSFOCALLENGTH = 75;

                double pinholeProjMag = SCANLENSFOCALLENGTH / PINHOLECOLLECTORLENSFOCALLENGTH;

                //PH variable in Confocal NyquistZ formula
                double pinholeVar = _pinholeSizeUM * pinholeProjMag / Magnification;

                double termA = ((0.88 * EmissionWavelength / 1000.0) / (IndexOfRefraction - Math.Sqrt(IndexOfRefraction * IndexOfRefraction - NumericalAperture * NumericalAperture)));
                double termB = (Math.Sqrt(2) * IndexOfRefraction * pinholeVar) / NumericalAperture;
                return Math.Round(Math.Sqrt(termA * termA + termB * termB) / 2.0, 3);
            }
        }

        public string XOutOfRangeErrorText
        {
            get
            {
                return GetPixelOutOfBoundsErrorMessage(LsmPixelXMax);
            }
        }

        public string YOutOfRangeErrorText
        {
            get
            {
                return GetPixelOutOfBoundsErrorMessage(LsmPixelYMax);
            }
        }

        #endregion Properties

        #region Methods

        protected virtual void OnPropertyChanged(String propertyName)
        {
            if (System.String.IsNullOrEmpty(propertyName))
            {
                return;
            }
            if (PropertyChanged != null)
            {
                PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }

        /// <summary>
        /// Handles the enter key being pressed by moving the keyboard focus to the next element
        /// </summary>
        private static void HandleEnterKey()
        {
            TraversalRequest trNext = new TraversalRequest(FocusNavigationDirection.Next);

            UIElement keyboardFocus = (UIElement)Keyboard.FocusedElement;

            //move the focus to the next element
            if (keyboardFocus != null)
            {
                if (keyboardFocus.GetType() == typeof(TextBox))
                {
                    keyboardFocus.MoveFocus(trNext);
                }
            }
        }

        private void btnApply2P_Click(object sender, RoutedEventArgs e)
        {
            Modality = 1;
            this.DialogResult = true;
            this.Close();
        }

        private void btnApplyConfocal_Click(object sender, RoutedEventArgs e)
        {
            Modality = 0;
            this.DialogResult = true;
            this.Close();
        }

        private void btnCancel_Click(object sender, RoutedEventArgs e)
        {
            this.DialogResult = false;
            this.Close();
        }

        private void btnInfo2P_Click(object sender, RoutedEventArgs e)
        {
            NyquistInfo dlg = new NyquistInfo();

            dlg.ShowDialog();
        }

        private void btnInfoConfocal_Click(object sender, RoutedEventArgs e)
        {
            NyquistInfo dlg = new NyquistInfo();

            dlg.ShowDialog();
        }

        /// <summary>
        /// The error message displayed when the calculated resolution is higher than the maximum
        /// </summary>
        /// <param name="max"> The current maximum </param>
        /// <returns> String containing the error message </returns>
        private string GetPixelOutOfBoundsErrorMessage(double max)
        {
            return "Resolution not supported by current field size. Lower below " + max + " or increase field size";
        }

        /// <summary>
        /// Handles Key Presses
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void NyquistCalculator_KeyDown(object sender, KeyEventArgs e)
        {
            //filter for the enter or return key
            if ((e.Key == Key.Enter) || (e.Key == Key.Return))
            {
                e.Handled = true;
                HandleEnterKey();
            }
        }

        void NyquistCalculator_Loaded(object sender, RoutedEventArgs e)
        {
        }

        /// <summary>
        /// Raises the additional events that need to be raised when the calculated pixel resolution property events are raised
        /// </summary>
        private void RaisePixelRecalculatedEvents()
        {
            OnPropertyChanged("LsmPixelXConfocalOutOfRange");
            OnPropertyChanged("LsmPixelYConfocalOutOfRange");
            OnPropertyChanged("LsmPixelXMultiphotonOutOfRange");
            OnPropertyChanged("LsmPixelYMultiphotonOutOfRange");
            OnPropertyChanged("ConfocalSettingsValid");
            OnPropertyChanged("MultiPhotonSettingsValid");
        }

        #endregion Methods
    }
}