namespace ImageReviewDll.View
{
    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Linq;
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

    using ImageReviewDll.ViewModel;

    /// <summary>
    /// Interaction logic for OrthoViewDispOptWin.xaml
    /// </summary>
    public partial class OrthoViewDispOptWin : Window, INotifyPropertyChanged
    {
        #region Fields

        private Color[] brushColors = new Color[] { Colors.White, Colors.Black, Colors.Green, Colors.Blue, Colors.Yellow, Colors.Red };
        private bool _setFlag = false;

        #endregion Fields

        #region Constructors

        public OrthoViewDispOptWin()
        {
            InitializeComponent();
            this.Owner = Application.Current.MainWindow;
            this.Loaded += OrthoViewDispOptWin_Loaded;
            this.Closed += OrthoViewDispOptWin_Closed;
        }

        #endregion Constructors

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public int ColorIndex
        {
            get
            {
                return lineColor.SelectedIndex;
            }
        }

        public int LineIndex
        {
            get
            {
                return lineType.SelectedIndex;
            }
        }

        public bool SetFlag
        {
            get
            {
                return _setFlag;
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
            this.Close();
        }

        private void Button_OnOK(object sender, RoutedEventArgs e)
        {
            _setFlag = true;
            this.Close();
        }

        void OrthoViewDispOptWin_Closed(object sender, EventArgs e)
        {
            Application.Current.MainWindow.Activate();
        }

        void OrthoViewDispOptWin_Loaded(object sender, RoutedEventArgs e)
        {
            //throw new NotImplementedException();

            lineType.Items.Clear();   //clear the contextmenu
            lineColor.Items.Clear();  //clear the contextmenu

            const int BITMAP_WIDTH = 256; //set the bitmap inside the item in contextmenu
            const int BITMAP_HEIGHT = 32;
            int stride = BITMAP_WIDTH / 8;
            byte[] pixels = new byte[BITMAP_HEIGHT * stride];
            //add the color
            for (int i = 0; i < brushColors.Length; i++)
            {
                List<System.Windows.Media.Color> colors = new List<System.Windows.Media.Color>();
                colors.Add(brushColors[i]);
                BitmapPalette myPalette = new BitmapPalette(colors);
                BitmapSource image = BitmapSource.Create(BITMAP_WIDTH, BITMAP_HEIGHT, 96, 96, PixelFormats.Indexed1, myPalette, pixels, stride);
                lineColor.Items.Add(new System.Windows.Controls.Image() { Source = image, Margin = new Thickness(2) });
            }
            //add the dash line to first bitmap
            byte[] pd = new byte[BITMAP_WIDTH * BITMAP_HEIGHT];
            PixelFormat pf = PixelFormats.Gray8;
            int rawStride = (BITMAP_WIDTH * pf.BitsPerPixel + 7) / 8;
            WriteableBitmap bitmap = new WriteableBitmap(BITMAP_WIDTH, BITMAP_HEIGHT, 96, 96, PixelFormats.Gray8, null);
            WriteableBitmap bitmap1 = new WriteableBitmap(BITMAP_WIDTH, BITMAP_HEIGHT, 96, 96, PixelFormats.Gray8, null);
            for (int y = 0; y < BITMAP_HEIGHT; y++)
            {
                for (int x = 0; x < BITMAP_WIDTH; x++)
                {
                    pd[x + y * BITMAP_WIDTH] = 0xFF;
                }
                if (y == BITMAP_HEIGHT / 2)
                {
                    int i = 0, j = 0;
                    for (int x = 30; x < BITMAP_WIDTH - 30; x++)
                    {
                        if (i < 15)
                        {
                            pd[x + y * BITMAP_WIDTH] = 0x00;
                            i++;
                            j = 0;
                        }
                        else if (i == 15)
                        {
                            pd[x + y * BITMAP_WIDTH] = 0xFF;
                            j++;
                            if (j == 5)
                            {
                                i = 0;
                            }
                        }

                    }
                }
            }
            bitmap.WritePixels(new Int32Rect(0, 0, BITMAP_WIDTH, BITMAP_HEIGHT), pd, rawStride, 0);
            lineType.Items.Add(new Image() { Source = bitmap, Margin = new Thickness(2) });
            //add the line to first bitmap
            for (int y = 0; y < BITMAP_HEIGHT; y++)
            {

                for (int x = 0; x < BITMAP_WIDTH; x++)
                {
                    pd[x + y * BITMAP_WIDTH] = 0xFF;
                }
                if (y == BITMAP_HEIGHT / 2)
                {
                    for (int x = 30; x < BITMAP_WIDTH - 30; x++)
                    {
                        pd[x + y * BITMAP_WIDTH] = 0x00;
                    }
                }
            }
            bitmap1.WritePixels(new Int32Rect(0, 0, BITMAP_WIDTH, BITMAP_HEIGHT), pd, rawStride, 0);
            lineType.Items.Add(new Image() { Source = bitmap1, Margin = new Thickness(2) });
            //reset to first item
            lineType.SelectedIndex = 0;
            lineColor.SelectedIndex = 0;
        }

        #endregion Methods
    }
}