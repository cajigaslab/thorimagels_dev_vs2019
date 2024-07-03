namespace MCM6000_Control
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
    using System.Windows.Shapes;
    using System.Xml;

    /// <summary>
    /// Interaction logic for ComPortWindow.xaml
    /// </summary>
    public partial class ComPortWindow : Window
    {
        #region Fields

        public static DependencyProperty ComPortProperty =
            DependencyProperty.RegisterAttached("ComPort",
            typeof(int),
            typeof(ComPortWindow));

        #endregion Fields

        #region Constructors

        public ComPortWindow()
        {
            InitializeComponent();
            ComPort = -1;
            for (int i = 0; i < 91; i++)
                cbBoxCOM.Items.Add("COM" + i.ToString());
        }

        #endregion Constructors

        #region Properties

        public int ComPort
        {
            get
            {
                return (int)GetValue(ComPortProperty);
            }
            set
            {
                SetValue(ComPortProperty, value);
            }
        }

        #endregion Properties

        #region Methods

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                int ftdiMode = 0;
                string settingsFile = Environment.CurrentDirectory + "\\ThorMCM6000Settings.xml";
                XmlDocument settingsXML = new XmlDocument();
                settingsXML.Load(settingsFile.ToString());
                XmlNodeList ndList = settingsXML.SelectNodes("/MCM6000Settings/DeviceInfo");
                XmlNodeList ndListFTDI = settingsXML.SelectNodes("/MCM6000Settings/FTDIsettings");

                //Check if the settings is set to read from the FTDI port
                if (ndListFTDI.Count > 0)
                {
                    XmlNode node = ndListFTDI.Item(0);
                    ftdiMode = Convert.ToInt32(ndListFTDI.Item(0).Attributes["FTDIMode"].Value.ToString());
                }

                //When in FTDI mode save to the portId from the <FTDIsettings> Node
                if (1 == ftdiMode)
                {
                    XmlNode node = ndListFTDI.Item(0);
                    ndListFTDI.Item(0).Attributes["portID"].Value = ComPort.ToString();
                    settingsXML.Save(settingsFile.ToString());
                }
                else
                {
                    if (ndList.Count > 0)
                    {
                        XmlNode node = ndList.Item(0);
                        ndList.Item(0).Attributes["portId"].Value = ComPort.ToString();
                        settingsXML.Save(settingsFile.ToString());
                    }
                }
            }
            catch (System.IO.FileNotFoundException ex)
            {
                MessageBox.Show(ex.Message, "ThorMCM6000Settings.xml file doest not exist");
                this.Close();
            }
            this.Close();
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            try
            {
                string settingsFile = Environment.CurrentDirectory + "\\ThorMCM6000Settings.xml";
                XmlDocument settingsXML = new XmlDocument();
                settingsXML.Load(settingsFile.ToString());
                int ftdiMode = 0;

                XmlNodeList ndList = settingsXML.SelectNodes("/MCM6000Settings/DeviceInfo");
                XmlNodeList ndListFTDI = settingsXML.SelectNodes("/MCM6000Settings/FTDIsettings");

                //Check if the settings is set to read from the FTDI port
                if (ndListFTDI.Count > 0)
                {
                    XmlNode node = ndListFTDI.Item(0);
                    ftdiMode = Convert.ToInt32(ndListFTDI.Item(0).Attributes["FTDIMode"].Value.ToString());
                }

                //When in FTDI mode use the portId from the <FTDIsettings> Node
                if (1 == ftdiMode)
                {
                    XmlNode node = ndListFTDI.Item(0);
                    ComPort = Convert.ToInt32(ndListFTDI.Item(0).Attributes["portID"].Value.ToString());
                }
                else
                {
                    if (ndList.Count > 0)
                    {
                        XmlNode node = ndList.Item(0);
                        ComPort = Convert.ToInt32(ndList.Item(0).Attributes["portId"].Value.ToString());
                    }
                }

                if ((ComPort < 0) || (ComPort > 90))
                    throw new System.IO.FileFormatException("ThorMCM6000 settings file in wrong format");
                //MessageBox.Show(cbBoxCOM.SelectedIndex.ToString());
            }
            catch (System.IO.FileNotFoundException ex)
            {
                MessageBox.Show(ex.Message, "ThorMCM6000Settings.xml file doest not exist");
            }
        }

        #endregion Methods
    }
}