namespace SpinnerProgress
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Linq;
    using System.Text;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Animation;
    using System.Windows.Media.Imaging;
    using System.Windows.Navigation;
    using System.Windows.Shapes;

    /// <summary>
    /// Interaction logic for SpinnerProgress.xaml
    /// </summary>
    public partial class SpinnerProgressWindow : Window
    {
        #region Constructors

        public SpinnerProgressWindow()
        {
            InitializeComponent();
        }

        #endregion Constructors

        #region Properties

        public string ProgressText
        {
            set
            {
                this.lblspinner.Content = value;
            }
        }

        #endregion Properties
    }
}