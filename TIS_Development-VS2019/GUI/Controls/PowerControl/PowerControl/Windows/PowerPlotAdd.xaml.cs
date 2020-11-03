namespace PowerControl
{
    using System;
    using System.Collections.Generic;
    using System.IO;
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

    using Microsoft.Win32;

    /// <summary>
    /// Interaction logic for PowerPlotAdd.xaml
    /// </summary>
    public partial class PowerPlotAddWin : Window
    {
        #region Constructors

        public PowerPlotAddWin()
        {
            InitializeComponent();
            tbStart.Text = "0";
            tbStop.Text = "50";
        }

        #endregion Constructors

        #region Properties

        public string PowerRampName
        {
            get
            {
                return tbName.Text;
            }
        }

        public double PowerStart
        {
            get
            {
                double val = 0.0;
                try
                {
                    val = Convert.ToDouble(tbStart.Text);
                }
                catch (FormatException ex)
                {
                    ex.ToString();
                }

                return val;
            }
        }

        public double PowerStop
        {
            get
            {
                double val = 0.0;
                try
                {
                    val = Convert.ToDouble(tbStop.Text);
                }
                catch (FormatException ex)
                {
                    ex.ToString();
                }

                return val;
            }
        }

        public int TemplateWaveform
        {
            get
            {
                return cbTemplate.SelectedIndex;
            }
        }

        #endregion Properties

        #region Methods

        private void btnCancel_Click(object sender, RoutedEventArgs e)
        {
            this.DialogResult = false;
            this.Close();
        }

        private void btnOK_Click(object sender, RoutedEventArgs e)
        {
            this.DialogResult = true;
            this.Close();
        }

        #endregion Methods
    }
}