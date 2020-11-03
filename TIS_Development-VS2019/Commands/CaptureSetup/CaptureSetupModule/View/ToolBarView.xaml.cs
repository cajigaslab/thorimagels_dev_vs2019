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
    using System.Xml;

    using CaptureSetupDll.ViewModel;

    using ThorSharedTypes;

    /// <summary>
    /// Interaction logic for ToolBarView.xaml
    /// </summary>
    public partial class ToolBarView : UserControl
    {
        #region Constructors

        public ToolBarView()
        {
            InitializeComponent();
            this.Loaded += ToolBarView_Loaded;
        }

        #endregion Constructors

        #region Methods

        void ToolBarView_Loaded(object sender, RoutedEventArgs e)
        {
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;

            if (null == vm)
            {
                return;
            }

            XmlDocument appSettingsDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];

            XmlNodeList ndList;

            // if xy control is not selected, hide the tile view icon, otherwise show it
            ndList = appSettingsDoc.SelectNodes("/ApplicationSettings/DisplayOptions/CaptureSetup/XYView");
            if (ndList.Count > 0)
            {
                ((CaptureSetupViewModel)this.DataContext).XYCtrlVisible = ndList[0].Attributes["Visibility"].Value.Equals("Visible") ? true : false;

            }
        }

        #endregion Methods
    }
}