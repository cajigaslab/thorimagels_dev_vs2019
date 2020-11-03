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

namespace HardwareSetupDll.View
{
    /// <summary>
    /// Interaction logic for WavelengthInputDialog.xaml
    /// </summary>
    public partial class WavelengthInputDialog : Window
    {

        public WavelengthInputDialog()
        {
            InitializeComponent();

            wavelengthValue.Text = "480";
            FillColor = Color.FromRgb(0xFF, 0xFF, 0xFF);
            fluorescence.IsChecked = true;
        }

        private void Button_ClickOK(object sender, RoutedEventArgs e)
        {
            if (WavelengthName.Length == 0)
            {
                //*TODO*set focus to wavelength name
                return;
            }

            if (WavelengthValue.Length == 0)
            {
                //*TODO*set focus to wavelength value
                return;
            }

            if (fluorescence.IsChecked == true)
            {
                Fluorescence = FluorEnum.Pos1;
            }
            else
            {
                Fluorescence = FluorEnum.Pos2;
            }

            DialogResult = true;
            Close();
        }

        private void Button_ClickCancel(object sender, RoutedEventArgs e)
        {
            DialogResult = false;
            Close();
        }

        public string WavelengthName
        {
            get
            {
                return wavelengthName.Text;
            }
            set
            {
                wavelengthName.Text = value;
            }
        }
        public string WavelengthValue
        {
            get
            {
                return wavelengthValue.Text;
            }
            set
            {
                wavelengthValue.Text = value;
            }
        }

        private void colorButton_Click(object sender, RoutedEventArgs e)
        {
            Microsoft.Samples.CustomControls.ColorPickerDialog cPicker
                = new Microsoft.Samples.CustomControls.ColorPickerDialog();
            cPicker.StartingColor = Colors.Green;
            cPicker.Owner = this;

            bool? dialogResult = cPicker.ShowDialog();
            if (dialogResult != null && (bool)dialogResult == true)
            {
                FillColor = cPicker.SelectedColor;
            }

        }

        public Color FillColor
        {
            get
            {
                return (Color)GetValue(FillColorProperty);
            }
            set
            {
                SetValue(FillColorProperty, value);
            }
        }

        public static readonly DependencyProperty FillColorProperty =
     DependencyProperty.Register
     ("FillColor", typeof(Color), typeof(WavelengthInputDialog),
     new PropertyMetadata(Colors.LightGray));


        public enum FluorEnum { Pos1, Pos2};

        private FluorEnum _fluor = 0;

        public FluorEnum Fluorescence
        {
            get
            {
                return _fluor;
            }

            set
            {
                _fluor = value;
            }
        }

        public static readonly DependencyProperty FluorescenceProperty =
     DependencyProperty.Register
     ("Fluorescence", typeof(FluorEnum), typeof(WavelengthInputDialog),
     new PropertyMetadata(FluorEnum.Pos1));
    }
}
