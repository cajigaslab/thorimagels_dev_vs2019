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
    using System.IO;

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
            MVMManager.CollectSingleMVM("ImageViewMVM.dll");
            MVMManager.Instance.LoadSettings();
            ImageReview imageReview = new ImageReview();
            imageReview.ExperimentReviewOpened = true;
            _vm = new ImageReviewViewModel(null, null, null, imageReview);
            ImageReviewWindow.ViewModel = _vm;
            this.Closed += new EventHandler(MainWindow_Closed);
            this.Loaded += new RoutedEventHandler(MainWindow_Loaded);
            this.ContentRendered += new EventHandler(MainWindow_Rendered);
            _vm.IncreaseViewArea += new Action<bool>(NewWindowAdjustImageViewSize);
        }

        #endregion Constructors

        #region Methods

        public void PersistWindowSettings()
        {
            try
            {
                XmlDocument erSettingsDoc = new XmlDocument();

                string experimentReviewSettingsPath = ".\\ExperimentReviewSettings.xml";

                if (File.Exists(experimentReviewSettingsPath))
                {
                    erSettingsDoc.Load(experimentReviewSettingsPath);
                    XmlNode ndList = erSettingsDoc.SelectSingleNode("/ExperimentReview/PersistedViewSize");
                    if (null != ndList)
                    {
                        XmlManager.SetAttribute(ndList, erSettingsDoc, "left", ((int)Math.Round(this.Left)).ToString());
                        XmlManager.SetAttribute(ndList, erSettingsDoc, "top", ((int)Math.Round(this.Top)).ToString());
                        XmlManager.SetAttribute(ndList, erSettingsDoc, "width", ((int)Math.Round(this.Width)).ToString());
                        XmlManager.SetAttribute(ndList, erSettingsDoc, "height", ((int)Math.Round(this.Height)).ToString());

                        erSettingsDoc.Save(experimentReviewSettingsPath);
                    }
                    else
                    { }
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
            XmlDocument erSettingsDoc = new XmlDocument();
            string experimentReviewSettingsPath = ".\\ExperimentReviewSettings.xml";


            if (File.Exists(experimentReviewSettingsPath))
            {
                erSettingsDoc.Load(experimentReviewSettingsPath);
                XmlNode node = erSettingsDoc.SelectSingleNode("/ExperimentReview/PersistedViewSize");
                if (null != node)
                {
                    if (node != null)
                    {
                        string str = string.Empty;
                        double val = 0;
                        XmlManager.GetAttribute(node, erSettingsDoc, "left", ref str);
                        if (double.TryParse(str, out val))
                        {
                            this.Left = (int)val;
                        }
                        else
                        {
                            this.Left = 0;
                        }
                        XmlManager.GetAttribute(node, erSettingsDoc, "top", ref str);
                        if (double.TryParse(str, out val))
                        {
                            this.Top = (int)val;
                        }
                        else
                        {
                            this.Top = 0;
                        }
                        XmlManager.GetAttribute(node, erSettingsDoc, "width", ref str);
                        if (double.TryParse(str, out val))
                        {
                            this.Width = (int)val;
                        }
                        else
                        {
                            this.Width = 600;
                        }
                        XmlManager.GetAttribute(node, erSettingsDoc, "height", ref str);
                        if (double.TryParse(str, out val))
                        {
                            this.Height = (int)val;
                        }
                        else
                        {
                            this.Height = 600;
                        }
                    }
                }
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

        void NewWindowAdjustImageViewSize(bool obj)
        {
            //When the viewer first loaded bring in the last experiment
            ImageReviewWindow.AdjustImageViewSize(obj);
        }

        #endregion Methods
    }
}