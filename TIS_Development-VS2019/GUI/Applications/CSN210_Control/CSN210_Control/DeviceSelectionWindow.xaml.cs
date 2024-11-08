﻿namespace CSN210_Control
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
    /// Interaction logic for ComPortWindow.xaml
    /// </summary>
    public partial class DeviceSelectionWindow : Window
    {
        #region Constructors

        public DeviceSelectionWindow()
        {
            InitializeComponent();
            this.Loaded += new RoutedEventHandler(WindowView_Loaded);
        }

        #endregion Constructors

        #region Methods

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            this.DialogResult = true;
            this.Close();
        }

        void WindowView_Loaded(object sender, RoutedEventArgs e)
        {
            SerialListBox.SelectedIndex = 0;
            var listBoxItem = (ListBoxItem)SerialListBox.ItemContainerGenerator.ContainerFromItem(SerialListBox.SelectedItem);
            listBoxItem.Focus();
        }

        #endregion Methods
    }
}