namespace ImageReviewDll.View
{
    using System;
    using System.Collections.Generic;
    using System.IO;
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
    using System.Xml;

    using FolderDialogControl;

    using ImageReviewDll.ViewModel;

    /// <summary>
    /// Interaction logic for ImageTilerView.xaml
    /// </summary>
    public partial class ImageTilerView : UserControl
    {
        #region Fields

        private const int PATH_LENGTH = 261;

        private string _selectedFolder;
        private ImageReviewViewModel _viewModel = null;

        #endregion Fields

        #region Constructors

        public ImageTilerView()
        {
            InitializeComponent();

            imageTilerControl.ImageCoordChangedEvent += new ImageTilerControl.UserControl1.ImageCoordChangedDelegate(imageTilerControl_ImageCoordChangedEvent);
        }

        #endregion Constructors

        #region Methods

        public void LoadData(string dataPath)
        {
            imageTilerControl.LoadData(dataPath);
        }

        [DllImport(".\\ExperimentManager.dll", EntryPoint = "GetActiveExperimentPath", CharSet = CharSet.Unicode)]
        private static extern int GetActiveExperimentPath(StringBuilder path, int length);

        [DllImport(".\\ExperimentManager.dll", EntryPoint = "GetActiveExperimentPathAndName", CharSet = CharSet.Unicode)]
        private static extern int GetActiveExperimentPathAndName(StringBuilder path, int length);

        private void btnSelectTDIExperiment_Click(object sender, RoutedEventArgs e)
        {
            _viewModel = (ImageReviewViewModel)this.DataContext;

            if (_viewModel == null)
            {
                return;
            }

            // disable the Image Tiler when Select Experiment dialog pops up
            imageTilerControl.IsEnabled = false;

            BrowseForFolderDialog dlg = new BrowseForFolderDialog();
            dlg.Title = "Select the Experiment Directory";
            dlg.InitialExpandedFolder = _selectedFolder;
            dlg.OKButtonText = "OK";

            string expPath = string.Empty;

            if (true == dlg.ShowDialog())
            {
                expPath = dlg.SelectedFolder;
                _viewModel.ExperimentFolderPath = expPath;

                // re-enables the Image Tiler after Select Experiment dialog dismissed
                imageTilerControl.IsEnabled = true;
            }
            else
            {
                // re-enables the Image Tiler after Select Experiment dialog dismissed
                imageTilerControl.IsEnabled = true;

                //do nothing if user cancels the folder selection
                return;
            }

            txtSelectedTDIExperiment.Text = expPath;
            _selectedFolder = expPath + "\\";

            if (Directory.Exists(_selectedFolder))
            {
                imageTilerControl.LoadData(_selectedFolder);
            }
        }

        //This is not used in ThorImage when you click Show Most Recent in the Review tab.
        //The one called from Review tab is in MasterView.xaml.cs.
        //:TODO: Check if this one is called from the standalone window
        private void btnShowMostRecent_Click(object sender, RoutedEventArgs e)
        {
            _viewModel = (ImageReviewViewModel)this.DataContext;

            if (_viewModel == null)
            {
                return;
            }

            imageTilerControl.IsEnabled = false;

            StringBuilder expPathAndName = new StringBuilder(PATH_LENGTH);
            GetActiveExperimentPathAndName(expPathAndName, PATH_LENGTH);

            if (expPathAndName.ToString().EndsWith("Experiment.xml"))
            {
                StringBuilder expPath = new StringBuilder(PATH_LENGTH);
                GetActiveExperimentPath(expPath, PATH_LENGTH);

                txtSelectedTDIExperiment.Text = expPath.ToString();
                _selectedFolder = expPath.ToString();
            }
            else
            {
                //reads experiment folder path from ApplicationSettings.xml
                XmlNode node = _viewModel.ApplicationDoc.SelectSingleNode("/ApplicationSettings/LastExperiment");
                if (node.Attributes.GetNamedItem("path") != null)
                {
                    _selectedFolder = node.Attributes["path"].Value;
                }
            }

            if (!Directory.Exists(_selectedFolder) || (_viewModel == null))
            {
                imageTilerControl.IsEnabled = true;
                return;
            }

            txtSelectedTDIExperiment.Text = _selectedFolder;

            imageTilerControl.LoadData(_selectedFolder);

            imageTilerControl.IsEnabled = true;
        }

        private void cbClickToMove_Checked(object sender, RoutedEventArgs e)
        {
            if(imageTilerControl != null)
                imageTilerControl.MoveStageOnClick = (bool)cbClickToMove.IsChecked;
        }

        private void imageTilerBorder_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            imageTilerControl.Width = imageTilerBorder.Width - 10;
            imageTilerControl.Height = imageTilerBorder.Height - 10;
        }

        void imageTilerControl_ImageCoordChangedEvent(int X, int Y)
        {
            txtMouseOverGrid.Text = "(" + X.ToString() + ", " + Y.ToString() + ")";
        }

        #endregion Methods
    }
}