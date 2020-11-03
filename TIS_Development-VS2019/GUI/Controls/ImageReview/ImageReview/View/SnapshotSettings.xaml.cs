namespace ImageReviewDll
{
    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Runtime.InteropServices;
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

        private string[] _luts;

        #endregion Fields

        #region Constructors

        public SnapshotSettings()
        {
            InitializeComponent();
            this.Loaded += SnapshotSettings_Loaded;
        }

        #endregion Constructors

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public bool GrayscaleForSingleChannel
        {
            get;
            set;
        }

        public XmlDocument HardwareDoc
        {
            get;
            set;
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
            //if ((comboChanA.SelectedIndex == comboChanB.SelectedIndex) ||
            //    (comboChanA.SelectedIndex == comboChanC.SelectedIndex) ||
            //    (comboChanA.SelectedIndex == comboChanD.SelectedIndex) ||
            //    (comboChanB.SelectedIndex == comboChanC.SelectedIndex) ||
            //    (comboChanB.SelectedIndex == comboChanD.SelectedIndex) ||
            //    (comboChanC.SelectedIndex == comboChanD.SelectedIndex))
            //{
            //    MessageBox.Show("Choose a unique color for each channel", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
            //    return;
            //}

            for (int i = 0; i < _luts.Length; i++)
            {
                XmlNodeList ndList = HardwareDoc.SelectNodes("/HardwareSettings/ColorChannels/" + System.IO.Path.GetFileNameWithoutExtension(_luts[i]));

                if (ndList.Count > 0)
                {
                    ndList[0].Attributes["name"].Value = string.Empty;

                    if (i == comboChanA.SelectedIndex)
                    {
                        ndList[0].Attributes["name"].Value += "ChanA";
                    }
                    if (i == comboChanB.SelectedIndex)
                    {
                        ndList[0].Attributes["name"].Value += "ChanB";
                    }
                    if (i == comboChanC.SelectedIndex)
                    {
                        ndList[0].Attributes["name"].Value += "ChanC";
                    }
                    if (i == comboChanD.SelectedIndex)
                    {
                        ndList[0].Attributes["name"].Value += "ChanD";
                    }
                    if (ndList[0].Attributes["name"].Value == string.Empty)
                    {
                        ndList[0].Attributes["name"].Value = "None";
                    }
                }
                else
                {
                    XmlElement newElement = HardwareDoc.CreateElement(System.IO.Path.GetFileNameWithoutExtension(_luts[i]));
                    XmlAttribute newAttribute = HardwareDoc.CreateAttribute("name");
                    newAttribute.Value = string.Empty;
                    if (i == comboChanA.SelectedIndex)
                    {
                        newAttribute.Value += "ChanA";
                    }
                    if (i == comboChanB.SelectedIndex)
                    {
                        newAttribute.Value += "ChanB";
                    }
                    if (i == comboChanC.SelectedIndex)
                    {
                        newAttribute.Value += "ChanC";
                    }
                    if (i == comboChanD.SelectedIndex)
                    {
                        newAttribute.Value += "ChanD";
                    }
                    if (newAttribute.Value == string.Empty)
                    {
                        newAttribute.Value = "None";
                    }

                    newElement.Attributes.Append(newAttribute);

                    ndList = HardwareDoc.SelectNodes("/HardwareSettings/ColorChannels");

                    ndList[0].AppendChild(newElement);

                }
            }

            GrayscaleForSingleChannel = cbGraySingleChan.IsChecked.Value;

            DialogResult = true;
            Close();
        }

        private StackPanel CreateSPForBitmap(int i, WriteableBitmap bitmap)
        {
            StackPanel sp = new StackPanel();
            sp.Orientation = Orientation.Horizontal;
            sp.Margin = new Thickness(2);
            sp.Children.Add(new Image() { Source = bitmap });
            sp.Children.Add(new Label() { Content = System.IO.Path.GetFileNameWithoutExtension(_luts[i]), MinWidth = 90 });
            return sp;
        }

        void SnapshotSettings_Loaded(object sender, RoutedEventArgs e)
        {
            string str = Application.Current.Resources["ApplicationSettingsFolder"].ToString() + "\\luts\\";

            if (!Directory.Exists(str))
            {
                return;
            }

            _luts = Directory.GetFiles(str, "*.txt");

            for (int i = 0; i < _luts.Length; i++)
            {
                string p = System.IO.Path.GetDirectoryName(_luts[i]) + "\\";
                string f = System.IO.Path.GetFileName(_luts[i]);
                string[] tokens = f.Split(' ');
                string nName = "";
                for (int j = 0; j < tokens.Length; j++)
                {
                    nName += tokens[j];
                }
                File.Move(p + f, p + nName);
                _luts[i] = p + nName;
            }

            comboChanA.Items.Clear();
            comboChanB.Items.Clear();
            comboChanC.Items.Clear();
            comboChanD.Items.Clear();

            const int BITMAP_WIDTH = 256;
            const int BITMAP_HEIGHT = 32;

            byte[] pd = new byte[BITMAP_WIDTH * BITMAP_HEIGHT];

            for (int y = 0; y < BITMAP_HEIGHT; y++)
            {
                for (int x = 0; x < BITMAP_WIDTH; x++)
                {
                    pd[x + y * BITMAP_WIDTH] = (byte)x;
                }
            }

            for (int i = 0; i < _luts.Length; i++)
            {
                PixelFormat pf = PixelFormats.Indexed8;

                List<Color> colors = new List<Color>();

                StreamReader fs = new StreamReader(_luts[i]);
                string line;
                int counter = 0;
                try
                {
                    while ((line = fs.ReadLine()) != null)
                    {
                        string[] split = line.Split(',');

                        if (split[0] != null)
                        {
                            if (split[1] != null)
                            {
                                if (split[2] != null)
                                {
                                    colors.Add(Color.FromRgb(Convert.ToByte(split[0]), Convert.ToByte(split[1]), Convert.ToByte(split[2])));
                                }
                            }
                        }
                        counter++;
                    }
                }
                catch (Exception ex)
                {
                    string msg = ex.Message;
                }

                fs.Close();

                //if the length of the file matches the bitmap size
                if (BITMAP_WIDTH == counter)
                {
                    BitmapPalette palette = new BitmapPalette(colors);

                    WriteableBitmap bitmap = new WriteableBitmap(BITMAP_WIDTH, BITMAP_HEIGHT, 96, 96, pf, palette);

                    bitmap.WritePixels(new Int32Rect(0, 0, BITMAP_WIDTH, BITMAP_HEIGHT), pd, BITMAP_WIDTH, 0);

                    comboChanA.Items.Add(CreateSPForBitmap(i, bitmap));
                    comboChanB.Items.Add(CreateSPForBitmap(i, bitmap));
                    comboChanC.Items.Add(CreateSPForBitmap(i, bitmap));
                    comboChanD.Items.Add(CreateSPForBitmap(i, bitmap));
                }
            }

            comboChanA.SelectedIndex = 0;
            comboChanB.SelectedIndex = 0;
            comboChanC.SelectedIndex = 0;
            comboChanD.SelectedIndex = 0;

            for (int i = 0; i < _luts.Length; i++)
            {
                XmlNodeList ndList = HardwareDoc.SelectNodes("/HardwareSettings/ColorChannels/" + System.IO.Path.GetFileNameWithoutExtension(_luts[i]));

                if (ndList.Count > 0)
                {
                    if (ndList[0].Attributes["name"].Value.Contains("ChanA"))
                    {
                        comboChanA.SelectedIndex = i;
                    }
                    if (ndList[0].Attributes["name"].Value.Contains("ChanB"))
                    {
                        comboChanB.SelectedIndex = i;
                    }
                    if (ndList[0].Attributes["name"].Value.Contains("ChanC"))
                    {
                        comboChanC.SelectedIndex = i;
                    }
                    if (ndList[0].Attributes["name"].Value.Contains("ChanD"))
                    {
                        comboChanD.SelectedIndex = i;
                    }
                }
            }
            cbGraySingleChan.IsChecked = GrayscaleForSingleChannel;
        }

        #endregion Methods
    }
}