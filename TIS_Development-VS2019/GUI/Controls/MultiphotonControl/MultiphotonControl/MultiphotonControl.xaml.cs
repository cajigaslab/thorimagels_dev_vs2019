namespace MultiphotonControl
{
    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Globalization;
    using System.IO;
    using System.Linq;
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

    using ThorSharedTypes;

    public class BooleanToVisibilityConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            var flag = false;
            if (value is bool)
            {
                flag = (bool)value;
            }
            else if (value is bool?)
            {
                var nullable = (bool?)value;
                flag = nullable.GetValueOrDefault();
            }
            else if (value is int)
            {
                int temp = (int)value;
                flag = (0 == temp) ? false : true;
            }
            if (parameter != null)
            {
                if (bool.Parse((string)parameter))
                {
                    flag = !flag;
                }
            }
            if (flag)
            {
                return Visibility.Visible;
            }
            else
            {
                return Visibility.Collapsed;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            var back = ((value is Visibility) && (((Visibility)value) == Visibility.Visible));
            if (parameter != null)
            {
                if ((bool)parameter)
                {
                    back = !back;
                }
            }
            return back;
        }

        #endregion Methods
    }

    /// <summary>
    /// Interaction logic for UserControl1.xaml
    /// </summary>
    public partial class MultiphotonControlUC : UserControl
    {
        #region Fields

        public static readonly DependencyProperty BeamStabilizerAvailableProperty = 
        DependencyProperty.Register(
        "BeamStabilizerAvailable",
        typeof(bool),
        typeof(MultiphotonControlUC));
        public static readonly DependencyProperty BeamStabilizerBPACentroidXProperty = 
        DependencyProperty.Register(
        "BeamStabilizerBPACentroidX",
        typeof(double),
        typeof(MultiphotonControlUC));
        public static readonly DependencyProperty BeamStabilizerBPACentroidYProperty = 
        DependencyProperty.Register(
        "BeamStabilizerBPACentroidY",
        typeof(double),
        typeof(MultiphotonControlUC));
        public static readonly DependencyProperty BeamStabilizerBPAExposureProperty = 
        DependencyProperty.Register(
        "BeamStabilizerBPAExposure",
        typeof(double),
        typeof(MultiphotonControlUC));
        public static readonly DependencyProperty BeamStabilizerBPBCentroidXProperty = 
        DependencyProperty.Register(
        "BeamStabilizerBPBCentroidX",
        typeof(double),
        typeof(MultiphotonControlUC));
        public static readonly DependencyProperty BeamStabilizerBPBCentroidYProperty = 
        DependencyProperty.Register(
        "BeamStabilizerBPBCentroidY",
        typeof(double),
        typeof(MultiphotonControlUC));
        public static readonly DependencyProperty BeamStabilizerBPBExposureProperty = 
        DependencyProperty.Register(
        "BeamStabilizerBPBExposure",
        typeof(double),
        typeof(MultiphotonControlUC));
        public static readonly DependencyProperty BeamStabilizerCentroidInRangeProperty = 
        DependencyProperty.Register(
        "BeamStabilizerCentroidInRange",
        typeof(CustomCollection<PC<int>>),
        typeof(MultiphotonControlUC));
        public static readonly DependencyProperty BeamStabilizerDataVisibilityProperty = 
        DependencyProperty.Register(
        "BeamStabilizerDataVisibility",
        typeof(Visibility),
        typeof(MultiphotonControlUC));
        public static readonly DependencyProperty BeamStabilizerExpInRangeProperty = 
        DependencyProperty.Register(
        "BeamStabilizerExpInRange",
        typeof(CustomCollection<PC<int>>),
        typeof(MultiphotonControlUC));
        public static readonly DependencyProperty BeamStabilizerPiezo1PosProperty = 
        DependencyProperty.Register(
        "BeamStabilizerPiezo1Pos",
        typeof(double),
        typeof(MultiphotonControlUC));
        public static readonly DependencyProperty BeamStabilizerPiezo2PosProperty = 
        DependencyProperty.Register(
        "BeamStabilizerPiezo2Pos",
        typeof(double),
        typeof(MultiphotonControlUC));
        public static readonly DependencyProperty BeamStabilizerPiezo3PosProperty = 
        DependencyProperty.Register(
        "BeamStabilizerPiezo3Pos",
        typeof(double),
        typeof(MultiphotonControlUC));
        public static readonly DependencyProperty BeamStabilizerPiezo4PosProperty = 
        DependencyProperty.Register(
        "BeamStabilizerPiezo4Pos",
        typeof(double),
        typeof(MultiphotonControlUC));
        public static readonly DependencyProperty BeamStabilizerPiezoInRangeProperty = 
        DependencyProperty.Register(
        "BeamStabilizerPiezoInRange",
        typeof(CustomCollection<PC<int>>),
        typeof(MultiphotonControlUC));
        public static readonly DependencyProperty Laser1FastSeqVisibilityProperty = 
        DependencyProperty.Register(
        "Laser1FastSeqVisibility",
        typeof(Visibility),
        typeof(MultiphotonControlUC));
        public static readonly DependencyProperty Laser1GoCommandProperty = 
        DependencyProperty.Register(
        "Laser1GoCommand",
        typeof(ICommand),
        typeof(MultiphotonControlUC));
        public static readonly DependencyProperty Laser1MinusCommandProperty = 
        DependencyProperty.Register(
        "Laser1MinusCommand",
        typeof(ICommand),
        typeof(MultiphotonControlUC));
        public static readonly DependencyProperty Laser1PlusCommandProperty = 
        DependencyProperty.Register(
        "Laser1PlusCommand",
        typeof(ICommand),
        typeof(MultiphotonControlUC));
        public static readonly DependencyProperty Laser1PositionGoProperty = 
        DependencyProperty.Register(
        "Laser1PositionGo",
        typeof(int),
        typeof(MultiphotonControlUC));
        public static readonly DependencyProperty Laser1PositionProperty = 
        DependencyProperty.Register(
        "Laser1Position",
        typeof(int),
        typeof(MultiphotonControlUC));
        public static readonly DependencyProperty Laser1SeqEnableProperty = 
        DependencyProperty.Register(
        "Laser1SeqEnable",
        typeof(bool),
        typeof(MultiphotonControlUC));
        public static readonly DependencyProperty Laser1SeqPos1Property = 
        DependencyProperty.Register(
        "Laser1SeqPos1",
        typeof(int),
        typeof(MultiphotonControlUC));
        public static readonly DependencyProperty Laser1SeqPos2Property = 
        DependencyProperty.Register(
        "Laser1SeqPos2",
        typeof(int),
        typeof(MultiphotonControlUC));
        public static readonly DependencyProperty LaserShutter2PositionProperty = 
        DependencyProperty.Register(
        "LaserShutter2Position",
        typeof(int),
        typeof(MultiphotonControlUC));
        public static readonly DependencyProperty LaserShutter2VisibilityProperty = 
        DependencyProperty.Register(
        "LaserShutter2Visibility",
        typeof(Visibility),
        typeof(MultiphotonControlUC));
        public static readonly DependencyProperty LaserShutterPositionProperty = 
        DependencyProperty.Register(
        "LaserShutterPosition",
        typeof(int),
        typeof(MultiphotonControlUC));
        public static readonly DependencyProperty PresetWavelengthAssignCommandProperty = 
        DependencyProperty.Register(
        "PresetWavelengthAssignCommand",
        typeof(ICommand),
        typeof(MultiphotonControlUC));
        public static readonly DependencyProperty PresetWavelengthCommandProperty = 
        DependencyProperty.Register(
        "PresetWavelengthCommand",
        typeof(ICommand),
        typeof(MultiphotonControlUC));
        public static readonly DependencyProperty PresetWavelengthNamesProperty = 
        DependencyProperty.Register(
        "PresetWavelengthNames",
        typeof(CustomCollection<PC<string>>),
        typeof(MultiphotonControlUC));
        public static readonly DependencyProperty RealignBeamCommandProperty = 
        DependencyProperty.Register(
        "RealignBeamCommand",
        typeof(ICommand),
        typeof(MultiphotonControlUC));
        public static readonly DependencyProperty ResetFactoryAlignmentCommandProperty = 
        DependencyProperty.Register(
        "ResetFactoryAlignmentCommand",
        typeof(ICommand),
        typeof(MultiphotonControlUC));
        public static readonly DependencyProperty SelectedWavelengthIndexProperty =
        DependencyProperty.Register(
        "SelectedWavelengthIndex",
        typeof(int),
        typeof(MultiphotonControlUC));
        public static readonly DependencyProperty Shutter1CloseProperty =
        DependencyProperty.Register(
        "Shutter1Close",
        typeof(bool),
        typeof(MultiphotonControlUC));
        public static readonly DependencyProperty Shutter1OpenProperty =
        DependencyProperty.Register(
        "Shutter1Open",
        typeof(bool),
        typeof(MultiphotonControlUC));


        #endregion Fields

        #region Constructors

        public MultiphotonControlUC()
        {
            InitializeComponent();
        }

        #endregion Constructors

        #region Properties

        public bool BeamStabilizerAvailable
        {
            get { return (bool)GetValue(BeamStabilizerAvailableProperty); }
            set { SetValue(BeamStabilizerAvailableProperty, value); }
        }

        public double BeamStabilizerBPACentroidX
        {
            get { return (double)GetValue(BeamStabilizerBPACentroidXProperty); }
            set { SetValue(BeamStabilizerBPACentroidXProperty, value); }
        }

        public double BeamStabilizerBPACentroidY
        {
            get { return (double)GetValue(BeamStabilizerBPACentroidYProperty); }
            set { SetValue(BeamStabilizerBPACentroidYProperty, value); }
        }

        public double BeamStabilizerBPAExposure
        {
            get { return (double)GetValue(BeamStabilizerBPAExposureProperty); }
            set { SetValue(BeamStabilizerBPAExposureProperty, value); }
        }

        public double BeamStabilizerBPBCentroidX
        {
            get { return (double)GetValue(BeamStabilizerBPBCentroidXProperty); }
            set { SetValue(BeamStabilizerBPBCentroidXProperty, value); }
        }

        public double BeamStabilizerBPBCentroidY
        {
            get { return (double)GetValue(BeamStabilizerBPBCentroidYProperty); }
            set { SetValue(BeamStabilizerBPBCentroidYProperty, value); }
        }

        public double BeamStabilizerBPBExposure
        {
            get { return (double)GetValue(BeamStabilizerBPBExposureProperty); }
            set { SetValue(BeamStabilizerBPBExposureProperty, value); }
        }

        public CustomCollection<PC<int>> BeamStabilizerCentroidInRange
        {
            get { return (CustomCollection<PC<int>>)GetValue(BeamStabilizerCentroidInRangeProperty); }
            set { SetValue(BeamStabilizerCentroidInRangeProperty, value); }
        }

        public Visibility BeamStabilizerDataVisibility
        {
            get { return (Visibility)GetValue(BeamStabilizerDataVisibilityProperty); }
            set { SetValue(BeamStabilizerDataVisibilityProperty, value); }
        }

        public CustomCollection<PC<int>> BeamStabilizerExpInRange
        {
            get { return (CustomCollection<PC<int>>)GetValue(BeamStabilizerExpInRangeProperty); }
            set { SetValue(BeamStabilizerExpInRangeProperty, value); }
        }

        public double BeamStabilizerPiezo1Pos
        {
            get { return (double)GetValue(BeamStabilizerPiezo1PosProperty); }
            set { SetValue(BeamStabilizerPiezo1PosProperty, value); }
        }

        public double BeamStabilizerPiezo2Pos
        {
            get { return (double)GetValue(BeamStabilizerPiezo2PosProperty); }
            set { SetValue(BeamStabilizerPiezo2PosProperty, value); }
        }

        public double BeamStabilizerPiezo3Pos
        {
            get { return (double)GetValue(BeamStabilizerPiezo3PosProperty); }
            set { SetValue(BeamStabilizerPiezo3PosProperty, value); }
        }

        public double BeamStabilizerPiezo4Pos
        {
            get { return (double)GetValue(BeamStabilizerPiezo4PosProperty); }
            set { SetValue(BeamStabilizerPiezo4PosProperty, value); }
        }

        public CustomCollection<PC<int>> BeamStabilizerPiezoInRange
        {
            get { return (CustomCollection<PC<int>>)GetValue(BeamStabilizerPiezoInRangeProperty); }
            set { SetValue(BeamStabilizerPiezoInRangeProperty, value); }
        }

        public Visibility Laser1FastSeqVisibility
        {
            get { return (Visibility)GetValue(Laser1FastSeqVisibilityProperty); }
            set { SetValue(Laser1FastSeqVisibilityProperty, value); }
        }

        public ICommand Laser1GoCommand
        {
            get { return (ICommand)GetValue(Laser1GoCommandProperty); }
            set { SetValue(Laser1GoCommandProperty, value); }
        }

        public ICommand Laser1MinusCommand
        {
            get { return (ICommand)GetValue(Laser1MinusCommandProperty); }
            set { SetValue(Laser1MinusCommandProperty, value); }
        }

        public ICommand Laser1PlusCommand
        {
            get { return (ICommand)GetValue(Laser1PlusCommandProperty); }
            set { SetValue(Laser1PlusCommandProperty, value); }
        }

        public int Laser1Position
        {
            get { return (int)GetValue(Laser1PositionProperty); }
            set { SetValue(Laser1PositionProperty, value); }
        }

        public int Laser1PositionGo
        {
            get { return (int)GetValue(Laser1PositionGoProperty); }
            set { SetValue(Laser1PositionGoProperty, value); }
        }

        public bool Laser1SeqEnable
        {
            get { return (bool)GetValue(Laser1SeqEnableProperty); }
            set { 
                
                if (Laser1FastSeqVisibility==Visibility.Visible)
                    SetValue(Laser1SeqEnableProperty, value); 
                else
                    SetValue(Laser1SeqEnableProperty, false);
            }
        }

        public int Laser1SeqPos1
        {
            get { return (int)GetValue(Laser1SeqPos1Property); }
            set { SetValue(Laser1SeqPos1Property, value); }
        }

        public int Laser1SeqPos2
        {
            get { return (int)GetValue(Laser1SeqPos2Property); }
            set { SetValue(Laser1SeqPos2Property, value); }
        }

        public int LaserShutter2Position
        {
            get { return (int)GetValue(LaserShutter2PositionProperty); }
            set { SetValue(LaserShutter2PositionProperty, value); }
        }

        public Visibility LaserShutter2Visibility
        {
            get { return (Visibility)GetValue(LaserShutter2VisibilityProperty); }
            set { SetValue(LaserShutter2VisibilityProperty, value); }
        }

        public int LaserShutterPosition
        {
            get { return (int)GetValue(LaserShutterPositionProperty); }
            set { SetValue(LaserShutterPositionProperty, value); }
        }

        public bool Shutter1Close
        {
            get { return ((int)GetValue(Shutter1CloseProperty)==0); }
            set { SetValue(Shutter1CloseProperty, value); }
        }


        public bool Shutter1Open
        {
            get { return ((int)GetValue(Shutter1OpenProperty) == 1); }
            set { SetValue(Shutter1OpenProperty, value); }
        }

        public ICommand PresetWavelengthAssignCommand
        {
            get { return (ICommand)GetValue(PresetWavelengthAssignCommandProperty); }
            set { SetValue(PresetWavelengthAssignCommandProperty, value); }
        }

        public ICommand PresetWavelengthCommand
        {
            get { return (ICommand)GetValue(PresetWavelengthCommandProperty); }
            set { SetValue(PresetWavelengthCommandProperty, value); }
        }

        public CustomCollection<PC<string>> PresetWavelengthNames
        {
            get { return (CustomCollection<PC<string>>)GetValue(PresetWavelengthNamesProperty); }
            set { SetValue(PresetWavelengthNamesProperty, value); }
        }

        public ICommand RealignBeamCommand
        {
            get { return (ICommand)GetValue(RealignBeamCommandProperty); }
            set { SetValue(RealignBeamCommandProperty, value); }
        }

        public ICommand ResetFactoryAlignmentCommand
        {
            get { return (ICommand)GetValue(ResetFactoryAlignmentCommandProperty); }
            set { SetValue(ResetFactoryAlignmentCommandProperty, value); }
        }

        public int SelectedWavelengthIndex
        {
            get { return (int)GetValue(SelectedWavelengthIndexProperty); }
            set { SetValue(SelectedWavelengthIndexProperty, value); }
        }

        

        #endregion Properties


        

    }
}