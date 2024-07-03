namespace ImageViewControl
{
    using System.Collections.Generic;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;

    /// <summary>
    /// Interaction logic for UserControl1.xaml
    /// </summary>
    public partial class LUTComboBoxView : UserControl
    {
        #region Fields

        const int BITMAP_HEIGHT = 32;
        const int BITMAP_WIDTH = 256;

        List<Color> _gradient;
        string _labelName = string.Empty;
        byte[] _pd;
        PixelFormat _pf = PixelFormats.Indexed8;

        #endregion Fields

        #region Constructors

        public LUTComboBoxView(List<Color> colors, string label)
        {
            InitializeComponent();

            _pd = new byte[BITMAP_WIDTH * BITMAP_HEIGHT];

            for (int y = 0; y < BITMAP_HEIGHT; y++)
            {
                for (int x = 0; x < BITMAP_WIDTH; x++)
                {
                    _pd[x + y * BITMAP_WIDTH] = (byte)x;
                }
            }

            LUTColorBitmap = colors;
            LabelName = label;
        }

        #endregion Constructors

        #region Properties

        public string LabelName
        {
            get
            {
                return _labelName;
            }
            set
            {
                _labelName = value;
                lbColorName.Content = _labelName;
            }
        }

        public List<Color> LUTColorBitmap
        {
            get
            {
                return _gradient;
            }
            set
            {
                _gradient = value;
                BitmapPalette palette = new BitmapPalette(value);

                WriteableBitmap bitmap = new WriteableBitmap(BITMAP_WIDTH, BITMAP_HEIGHT, 96, 96, _pf, palette);

                bitmap.WritePixels(new Int32Rect(0, 0, BITMAP_WIDTH, BITMAP_HEIGHT), _pd, BITMAP_WIDTH, 0);

                gradientBitmap.Source = bitmap;
            }
        }

        #endregion Properties
    }
}