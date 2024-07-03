namespace RemoteIPCControl
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
    using System.Windows.Shapes;

    /// <summary>
    /// Interaction logic for EditPipeDialog.xaml
    /// </summary>
    public partial class EditPipeDialog : Window
    {
        #region Constructors

        public EditPipeDialog()
        {
            InitializeComponent();
            this.Loaded += EditPipeDialog_Loaded;
        }

        #endregion Constructors

        #region Methods

        private void btnOK_Click(object sender, RoutedEventArgs e)
        {
            this.Close();
        }

        #endregion Methods

        #region Other

        private void DisplaySavedHostName()
        {
            //RunSampleLSViewModel vm = (RunSampleLSViewModel)this.DataContext;
            //if (vm == null)
            //{
            //    return;
            //}
            //for (int i = 0; i < lbPCName.Items.Count; i++)
            //{
            //    (lbPCName.Items[i] as ListBoxItem).Content = vm.RunSampleLS._selectRemotePCName[i];
            //}
        }
        private void DisplaySavedIPAddr()
        {
            //RunSampleLSViewModel vm = (RunSampleLSViewModel)this.DataContext;
            //if (vm == null)
            //{
            //    return;
            //}
            //for (int i = 0; i < lbPCName.Items.Count; i++)
            //{
            //    (lbPCName.Items[i] as ListBoxItem).Content = vm.RunSampleLS._selectRemodePCIPAddr[i];
            //}
        }
        void EditPipeDialog_Loaded(object sender, RoutedEventArgs e)
        {
            //RunSampleLSViewModel vm = (RunSampleLSViewModel)this.DataContext;
            //if (vm == null)
            //{
            //    return;
            //}
            //if (vm.IDMode == 0)
            //{
            //    DisplaySavedHostName();
            //}
            //else
            //{
            //    DisplaySavedIPAddr();
            //}
        }
        private void IDMode_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            //RunSampleLSViewModel vm = (RunSampleLSViewModel)this.DataContext;
            //if (vm == null)
            //{
            //    return;
            //}
            //vm.RunSampleLS.SaveRemotePCHostNameToXML();
            //if (vm.IDMode == 0)
            //{
            //    DisplaySavedHostName();
            //}
            //else
            //{
            //    DisplaySavedIPAddr();
            //}
        }
        private void MenuItem_Click(object sender, RoutedEventArgs e)
        {
            //RunSampleLSViewModel vm = (RunSampleLSViewModel)this.DataContext;
            //if (vm == null)
            //{
            //    return;
            //}
            //if (vm.IDMode == 0)
            //{
            //    vm.RunSampleLS._selectRemotePCName[vm.SelectedRemotePCNameIndex] = vm.RemotePCHostName;
            //    DisplaySavedHostName();
            //}
            //else
            //{
            //    vm.RunSampleLS._selectRemodePCIPAddr[vm.SelectedRemotePCNameIndex] = vm.RemotePCHostName;
            //    DisplaySavedIPAddr();
            //}
        }
        private void RemotePCHostName_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            //RunSampleLSViewModel vm = (RunSampleLSViewModel)this.DataContext;
            //if (vm == null)
            //{
            //    return;
            //}
            //vm.SelectedRemotePCNameIndex = lbPCName.Items.IndexOf(sender as ListBoxItem);
            //vm.RunSampleLS.SaveRemotePCHostNameToXML();
            //if (e.LeftButton == MouseButtonState.Pressed)
            //{
            //    if (vm.IDMode == 0)
            //    {
            //        if (vm.SelectedRemotePCNameIndex < lbPCName.Items.Count)
            //        {
            //            vm.RemotePCHostName = vm.RunSampleLS._selectRemotePCName[vm.SelectedRemotePCNameIndex];
            //        }
            //    }
            //    else
            //    {
            //        if (vm.SelectedRemotePCNameIndex < lbPCName.Items.Count)
            //        {
            //            vm.RemotePCHostName = vm.RunSampleLS._selectRemodePCIPAddr[vm.SelectedRemotePCNameIndex];
            //        }
            //    }
            //}
        }

        #endregion Other
    }
}