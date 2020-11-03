namespace CaptureSetupDll.View
{
    using System;
    using System.Collections.Generic;
    using System.Globalization;
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

    using CaptureSetupDll.Model;
    using CaptureSetupDll.ViewModel;

    using ThorSharedTypes;

    /// <summary>
    /// Interaction logic for AreaControlView.xaml
    /// </summary>
    public partial class AreaControlView : UserControl
    {
        #region Constructors

        public AreaControlView()
        {
            InitializeComponent();
        }

        #endregion Constructors
    }
}