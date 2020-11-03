namespace ExperimentReview
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

    using ImageReviewDll.Model;
    using ImageReviewDll.ViewModel;

    using ThorSharedTypes;

    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        #region Fields

        private ImageReviewViewModel _vm = null;

        #endregion Fields

        #region Constructors

        public MainWindow()
        {
            InitializeComponent();
            ImageReview imageReview = new ImageReview();
            _vm = new ImageReviewViewModel(null, null, null, imageReview);
            ImageReviewWindow.ViewModel = _vm;
            this.Closed += new EventHandler(MainWindow_Closed);
            this.Loaded += new RoutedEventHandler(MainWindow_Loaded);
            this.ContentRendered += new EventHandler(MainWindow_Rendered);
        }

        #endregion Constructors

        #region Methods

        public void PersistWindowSettings()
        {
            try
            {
                XmlDocument appSettingsFile = _vm.ApplicationDoc;
                if (null == appSettingsFile)
                    return;

                XmlNodeList ndList = appSettingsFile.SelectNodes("/ApplicationSettings/DisplayOptions/ExperimentReview");
                if (ndList.Count <= 0)
                {
                    ndList = appSettingsFile.SelectNodes("/ApplicationSettings/DisplayOptions");
                    CreateXmlNodeWithinNode(appSettingsFile, ndList[0], "ExperimentReview");
                    ndList = appSettingsFile.SelectNodes("/ApplicationSettings/DisplayOptions/ExperimentReview");
                }

                if (ndList[0] != null)
                {
                    XmlManager.SetAttribute(ndList[0], appSettingsFile, "left", ((int)Math.Round(this.Left)).ToString());
                    XmlManager.SetAttribute(ndList[0], appSettingsFile, "top", ((int)Math.Round(this.Top)).ToString());
                    XmlManager.SetAttribute(ndList[0], appSettingsFile, "width", ((int)Math.Round(this.Width)).ToString());
                    XmlManager.SetAttribute(ndList[0], appSettingsFile, "height", ((int)Math.Round(this.Height)).ToString());

                    appSettingsFile.Save(_vm.ApplicationSettingPath);
                }
            }
            catch (Exception ex)
            {
                ex.ToString();
            }
        }

        private void CreateXmlNodeWithinNode(XmlDocument doc, XmlNode parentNode, string nodeName)
        {
            XmlNode node = doc.CreateNode(XmlNodeType.Element, nodeName, null);
            parentNode.AppendChild(node);
        }

        private bool LoadWindowSettings()
        {
            try
            {
                XmlDocument appSettingsFile = _vm.ApplicationDoc;
                if (null == appSettingsFile)
                    return false;

                XmlNode node = appSettingsFile.SelectSingleNode("/ApplicationSettings/DisplayOptions/ExperimentReview");

                if (node != null)
                {
                    string str = string.Empty;
                    double val = 0;
                    XmlManager.GetAttribute(node, appSettingsFile, "left", ref str);
                    if (double.TryParse(str, out val))
                    {
                        this.Left = (int)val;
                    }
                    else
                    {
                        this.Left = 0;
                    }
                    XmlManager.GetAttribute(node, appSettingsFile, "top", ref str);
                    if (double.TryParse(str, out val))
                    {
                        this.Top = (int)val;
                    }
                    else
                    {
                        this.Top = 0;
                    }
                    XmlManager.GetAttribute(node, appSettingsFile, "width", ref str);
                    if (double.TryParse(str, out val))
                    {
                        this.Width = (int)val;
                    }
                    else
                    {
                        this.Width = 400;
                    }
                    XmlManager.GetAttribute(node, appSettingsFile, "height", ref str);
                    if (double.TryParse(str, out val))
                    {
                        this.Height = (int)val;
                    }
                    else
                    {
                        this.Height = 400;
                    }
                }
            }
            catch (Exception e)
            {
                e.ToString();
            }
            return true;
        }

        void MainWindow_Closed(object sender, EventArgs e)
        {
            try
            {
                PersistWindowSettings();
            }
            catch
            {
                Environment.Exit(0);
            }
        }

        void MainWindow_Loaded(object sender, RoutedEventArgs e)
        {
            LoadWindowSettings();
        }

        void MainWindow_Rendered(object sender, EventArgs e)
        {
            //When the viewer first loaded bring in the last experiment
            ImageReviewWindow.LoadLastExperiment();
        }

        #endregion Methods
    }
}