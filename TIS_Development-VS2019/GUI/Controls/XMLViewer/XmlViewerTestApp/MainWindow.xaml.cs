namespace XmlViewerTestApp
{
    using System;
    using System.Collections.Generic;
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
    using System.Xml;

    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        #region Constructors

        public MainWindow()
        {
            InitializeComponent();
            this.Loaded += MainWindow_Loaded;
        }

        #endregion Constructors

        #region Methods

        void MainWindow_Loaded(object sender, RoutedEventArgs e)
        {
            XmlDocument doc = new XmlDocument();

            doc.Load("C:/TIS_DevX/GUI/Controls/XMLViewer/XmlViewerTestApp/TestSettings.xml");

            xmlView.XmlDocument = doc;
        }

        #endregion Methods
    }
}