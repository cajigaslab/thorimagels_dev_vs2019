namespace CaptureSetupDll
{
    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.ServiceModel;
    using System.Text;
    using System.Text.RegularExpressions;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Shapes;
    using System.Xml;

    using ThorLogging;

    /// <summary>
    /// Interaction logic for SnapshotSettings.xaml
    /// </summary>
    public partial class SnapshotSettings : Window, INotifyPropertyChanged
    {
        #region Fields

        private string[] _nameList;

        #endregion Fields

        #region Constructors

        public SnapshotSettings()
        {
            _nameList = new string[7];
            InitializeComponent();
        }

        #endregion Constructors

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public string BlueChannel
        {
            get
            {
                return _nameList[2];
            }
            set
            {
                _nameList[2] = value;
                OnPropertyChanged("BlueChannel");
            }
        }

        public string CyanChannel
        {
            get
            {
                return _nameList[3];
            }
            set
            {
                _nameList[3] = value;
                OnPropertyChanged("CyanChannel");
            }
        }

        public string GrayChannel
        {
            get
            {
                return _nameList[6];
            }
            set
            {
                _nameList[6] = value;
                OnPropertyChanged("GrayChannel");
            }
        }

        public string GreenChannel
        {
            get
            {
                return _nameList[1];
            }
            set
            {
                _nameList[1] = value;
                OnPropertyChanged("GreenChannel");
            }
        }

        public string MagentaChannel
        {
            get
            {
                return _nameList[4];
            }
            set
            {
                _nameList[4] = value;
                OnPropertyChanged("MagentaChannel");
            }
        }

        public string RedChannel
        {
            get
            {
                return _nameList[0];
            }
            set
            {
                _nameList[0] = value;
                OnPropertyChanged("RedChannel");
            }
        }

        public string YellowChannel
        {
            get
            {
                return _nameList[5];
            }
            set
            {
                _nameList[5] = value;
                OnPropertyChanged("YellowChannel");
            }
        }

        #endregion Properties

        #region Methods

        protected virtual void OnPropertyChanged(String propertyName)
        {
            if (System.String.IsNullOrEmpty(propertyName))
            {
                return;
            }
            if (PropertyChanged != null)
            {
                PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }

        private void Button_OnCancel(object sender, RoutedEventArgs e)
        {
            DialogResult = false;
            Close();
        }

        private void Button_OnOK(object sender, RoutedEventArgs e)
        {
            _nameList[0] = comboRed.Text;
            _nameList[1]= comboGreen.Text;
            _nameList[2] = comboBlue.Text;
            _nameList[6] = comboGray.Text;
            _nameList[3] = comboCyan.Text;
            _nameList[4] = comboMagenta.Text;
            _nameList[5] = comboYellow.Text;

            for (int j = 0; j < 7; j++)
            {
                for (int i = 0; i < 7; i++)
                {
                    if ((i != j)&&(_nameList[j] != "None")&&(_nameList[j].Length > 0))
                    {
                        if (_nameList[j].Equals(_nameList[i]))
                        {
                            MessageBox.Show("Choose a unique channel for each color", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                            return;
                        }
                    }
                }
            }

            DialogResult = true;
            Close();
        }

        #endregion Methods
    }
}