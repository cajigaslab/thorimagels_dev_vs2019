namespace RemoteIPCControl
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

    /// <summary>
    /// Interaction logic for UserControl1.xaml
    /// </summary>
    public partial class RemoteIPCControlUC : UserControl
    {
        #region Fields

        public static DependencyProperty IDModeProperty = DependencyProperty.Register("IDMode", typeof(int), typeof(RemoteIPCControlUC));
        public static DependencyProperty LocalPCHostNameProperty = DependencyProperty.Register("LocalPCHostName", typeof(string), typeof(RemoteIPCControlUC));
        public static DependencyProperty LocalPCIPv4Property = DependencyProperty.Register("LocalPCIPv4", typeof(string), typeof(RemoteIPCControlUC));
        public static DependencyProperty RemoteAppNameProperty = DependencyProperty.Register("RemoteAppName", typeof(string), typeof(RemoteIPCControlUC));
        public static DependencyProperty RemoteConnectionProperty = DependencyProperty.Register("RemoteConnection", typeof(bool), typeof(RemoteIPCControlUC));
        public static DependencyProperty RemotePCHostNameProperty = DependencyProperty.Register("RemotePCHostName", typeof(string), typeof(RemoteIPCControlUC));

        #endregion Fields

        #region Constructors

        public RemoteIPCControlUC()
        {
            InitializeComponent();
        }

        #endregion Constructors

        #region Properties

        public int IDMode
        {
            get { return (int)GetValue(IDModeProperty); }
            set { SetValue(IDModeProperty, value); }
        }

        public string LocalPCHostName
        {
            get { return (string)GetValue(LocalPCHostNameProperty); }
            set { SetValue(LocalPCHostNameProperty, value); }
        }

        public string LocalPCIPv4
        {
            get { return (string)GetValue(LocalPCIPv4Property); }
            set { SetValue(LocalPCIPv4Property, value); }
        }

        public string RemoteAppName
        {
            get { return (string)GetValue(RemoteAppNameProperty); }
            set { SetValue(RemoteAppNameProperty, value); }
        }

        public bool RemoteConnection
        {
            get { return (bool)GetValue(RemoteConnectionProperty); }
            set { SetValue(RemoteConnectionProperty, value); }
        }

        public string RemotePCHostName
        {
            get { return (string)GetValue(RemotePCHostNameProperty); }
            set { SetValue(RemotePCHostNameProperty, value); }
        }

        #endregion Properties

        #region Methods

        private void ConnectionSettings_Click(object sender, RoutedEventArgs e)
        {
            EditPipeDialog editPipeDialog = new EditPipeDialog();
            editPipeDialog.DataContext = MVMManager.Instance["RemoteIPCControlViewModelBase"]; ;
            editPipeDialog.ShowDialog();
        }

        #endregion Methods
    }
}