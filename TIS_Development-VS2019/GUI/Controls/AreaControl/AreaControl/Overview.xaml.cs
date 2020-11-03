namespace AreaControl
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
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
    using System.Windows.Shapes;

    using ThorLogging;

    using ThorSharedTypes;

    /// <summary>
    /// Interaction logic for Window1.xaml
    /// </summary>
    public partial class MesoOverview : Window
    {
        #region Constructors

        public MesoOverview()
        {
            InitializeComponent();
            this.Loaded += MesoOverview_Loaded;
        }

        #endregion Constructors

        #region Properties

        public static string MesoOverviewPathAndName
        {
            get { return ResourceManagerCS.GetCaptureTemplatePathString() + "MesoView.bmp"; }
        }

        public Canvas OverlayCanvas
        {
            get { return this.overlayCanvas; }
            set { this.overlayCanvas = value; }
        }

        #endregion Properties

        #region Methods

        public bool PrepareOverview()
        {
            try
            {
                //this.hide();
                if (!File.Exists(MesoOverviewPathAndName))
                    return false;

                this.overviewImage.ImageSource = new BitmapImage(new Uri(MesoOverviewPathAndName, UriKind.Relative));
                this.Top = this.Left = 0.0;
                this.Width = this.overviewImage.ImageSource.Width;
                this.Height = this.overviewImage.ImageSource.Height;

                //this.InvalidateMeasure();
                //this.UpdateLayout();
                //this.Measure(new Size(this.Width, this.Height));
                //this.Arrange(new Rect(0, 0, this.DesiredSize.Width, this.DesiredSize.Height));

                //this.Show();
                //this.Activate();
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Information, 1, ex.Message);
                return false;
            }
            return true;
        }

        void MesoOverview_Loaded(object sender, RoutedEventArgs e)
        {
            PrepareOverview();
        }

        #endregion Methods
    }
}