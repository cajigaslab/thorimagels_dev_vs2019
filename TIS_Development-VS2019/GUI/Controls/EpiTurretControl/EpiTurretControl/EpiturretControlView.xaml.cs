namespace EpiTurretControl
{
    using System;
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

    /// <summary>
    /// Interaction logic for EpiturretControlView.xaml
    /// </summary>
    public partial class EpiTurretControlUC : UserControl
    {
        #region Fields

        public static readonly DependencyProperty EpiPositionNameChangeCommandProperty = 
            DependencyProperty.Register(
            "EpiPositionNameChangeCommand",
            typeof(ICommand),
            typeof(EpiTurretControlUC));
        public static readonly DependencyProperty GoToEpiPositionCommandProperty = 
            DependencyProperty.Register(
            "GoToEpiPositionCommand",
            typeof(ICommand),
            typeof(EpiTurretControlUC));

        public static DependencyProperty EpiPosition1NameProperty = 
            DependencyProperty.Register("EpiPosition1Name",
            typeof(string),
            typeof(EpiTurretControlUC));
        public static DependencyProperty EpiPosition1Property = 
            DependencyProperty.Register("EpiPosition1",
            typeof(bool),
            typeof(EpiTurretControlUC));
        public static DependencyProperty EpiPosition2NameProperty = 
            DependencyProperty.Register("EpiPosition2Name",
            typeof(string),
            typeof(EpiTurretControlUC));
        public static DependencyProperty EpiPosition2Property = 
            DependencyProperty.Register("EpiPosition2",
            typeof(bool),
            typeof(EpiTurretControlUC));
        public static DependencyProperty EpiPosition3NameProperty = 
            DependencyProperty.Register("EpiPosition3Name",
            typeof(string),
            typeof(EpiTurretControlUC));
        public static DependencyProperty EpiPosition3Property = 
            DependencyProperty.Register("EpiPosition3",
            typeof(bool),
            typeof(EpiTurretControlUC));
        public static DependencyProperty EpiPosition4NameProperty = 
            DependencyProperty.Register("EpiPosition4Name",
            typeof(string),
            typeof(EpiTurretControlUC));
        public static DependencyProperty EpiPosition4Property = 
            DependencyProperty.Register("EpiPosition4",
            typeof(bool),
            typeof(EpiTurretControlUC));
        public static DependencyProperty EpiPosition5NameProperty = 
            DependencyProperty.Register("EpiPosition5Name",
            typeof(string),
            typeof(EpiTurretControlUC));
        public static DependencyProperty EpiPosition5Property = 
            DependencyProperty.Register("EpiPosition5",
            typeof(bool),
            typeof(EpiTurretControlUC));
        public static DependencyProperty EpiPosition6NameProperty = 
            DependencyProperty.Register("EpiPosition6Name",
            typeof(string),
            typeof(EpiTurretControlUC));
        public static DependencyProperty EpiPosition6Property = 
            DependencyProperty.Register("EpiPosition6",
            typeof(bool),
            typeof(EpiTurretControlUC));

        #endregion Fields

        #region Constructors

        public EpiTurretControlUC()
        {
            InitializeComponent();
        }

        #endregion Constructors

        #region Properties

        public bool EpiPosition1
        {
            get { return (bool)GetValue(EpiPosition1Property); }
            set { SetValue(EpiPosition1Property, value); }
        }

        public string EpiPosition1Name
        {
            get { return (string)GetValue(EpiPosition1NameProperty); }
            set { SetValue(EpiPosition1NameProperty, value); }
        }

        public bool EpiPosition2
        {
            get { return (bool)GetValue(EpiPosition2Property); }
            set { SetValue(EpiPosition2Property, value); }
        }

        public string EpiPosition2Name
        {
            get { return (string)GetValue(EpiPosition2NameProperty); }
            set { SetValue(EpiPosition2NameProperty, value); }
        }

        public bool EpiPosition3
        {
            get { return (bool)GetValue(EpiPosition3Property); }
            set { SetValue(EpiPosition3Property, value); }
        }

        public string EpiPosition3Name
        {
            get { return (string)GetValue(EpiPosition3NameProperty); }
            set { SetValue(EpiPosition3NameProperty, value); }
        }

        public bool EpiPosition4
        {
            get { return (bool)GetValue(EpiPosition4Property); }
            set { SetValue(EpiPosition4Property, value); }
        }

        public string EpiPosition4Name
        {
            get { return (string)GetValue(EpiPosition4NameProperty); }
            set { SetValue(EpiPosition4NameProperty, value); }
        }

        public bool EpiPosition5
        {
            get { return (bool)GetValue(EpiPosition5Property); }
            set { SetValue(EpiPosition5Property, value); }
        }

        public string EpiPosition5Name
        {
            get { return (string)GetValue(EpiPosition5NameProperty); }
            set { SetValue(EpiPosition5NameProperty, value); }
        }

        public bool EpiPosition6
        {
            get { return (bool)GetValue(EpiPosition6Property); }
            set { SetValue(EpiPosition6Property, value); }
        }

        public string EpiPosition6Name
        {
            get { return (string)GetValue(EpiPosition6NameProperty); }
            set { SetValue(EpiPosition6NameProperty, value); }
        }

        public ICommand EpiPositionNameChangeCommand
        {
            get { return (ICommand)GetValue(EpiPositionNameChangeCommandProperty); }
            set { SetValue(EpiPositionNameChangeCommandProperty, value); }
        }

        public ICommand GoToEpiPositionCommand
        {
            get { return (ICommand)GetValue(GoToEpiPositionCommandProperty); }
            set { SetValue(GoToEpiPositionCommandProperty, value); }
        }

        #endregion Properties
    }
}