namespace CaptureSetupDll.View
{
    using System;
    using System.Collections.Generic;
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

    using CaptureSetupDll.ViewModel;

    using ThorLogging;

    using ThorSharedTypes;

    using TilesDisplay;

    /// <summary>
    /// Interaction logic for TileControlView.xaml
    /// </summary>
    public partial class TileControlView : UserControl
    {
        #region Fields

        private const int WELL_HEIGHT = 200;
        private const int WELL_WIDTH = 200;

        private List<Point> _tileList;

        #endregion Fields

        #region Constructors

        public TileControlView()
        {
            InitializeComponent();

            this.Loaded += new RoutedEventHandler(TileControlView_Loaded);
            this.Unloaded += TileControlView_Unloaded;
        }

        #endregion Constructors

        #region Methods

        void TileControlView_Loaded(object sender, RoutedEventArgs e)
        {
            if (System.ComponentModel.DesignerProperties.GetIsInDesignMode(this) == true)  //design mode
                return;

            _tileList = new List<Point>();

            xyTileControl.XSetZero += xyTileControl_XSetZero;
            xyTileControl.YSetZero += xyTileControl_YSetZero;
        }

        void TileControlView_Unloaded(object sender, RoutedEventArgs e)
        {
            xyTileControl.XSetZero -= xyTileControl_XSetZero;
            xyTileControl.YSetZero -= xyTileControl_YSetZero;
        }

        void xyTileControl_XSetZero(object sender, EventArgs e)
        {
            ((ICommand)MVMManager.Instance["XYTileControlViewModel", "SetXZeroCommand",(object)new ThorSharedTypes.RelayCommand(new Action(()=>{ }))]).Execute(null);
        }

        void xyTileControl_YSetZero(object sender, EventArgs e)
        {
            ((ICommand)MVMManager.Instance["XYTileControlViewModel", "SetYZeroCommand", (object)new ThorSharedTypes.RelayCommand(new Action(() => { }))]).Execute(null);
        }

        #endregion Methods
    }
}