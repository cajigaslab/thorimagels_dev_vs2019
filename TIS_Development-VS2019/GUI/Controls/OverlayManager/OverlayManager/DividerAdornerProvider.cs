namespace OverlayManager
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Controls.Primitives;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Shapes;

    public class DividerAdornerProvider : Adorner
    {
        #region Fields

        protected static int _imageHeight;
        protected static int _imageWidth;

        protected Type _adornedElementType;
        protected Brush _foreground;

        private VisualCollection _visualChildren;
        private List<Line> _stripeLines;
        //private List<Thumb> _debugThumbs;

        #endregion Fields

        #region Constructors

        public DividerAdornerProvider(UIElement adornedElement, Brush foreground, int imageWidth, int imageHeight)
            : base(adornedElement)
        {
            _imageHeight = imageHeight;
            _imageWidth = imageWidth;
            _foreground = foreground;
            _adornedElementType = adornedElement.GetType();
            _visualChildren = new VisualCollection(this);
            _stripeLines = new List<Line>();
            BuildVisuals();
            //adornedElement.LayoutUpdated += UpdateVisuals;
        }

        #endregion Constructors

        #region Properties

        public static int ImageHeight
        {
            get { return _imageHeight; }
            set { _imageHeight = value; }
        }

        public static int ImageWidth
        {
            get { return _imageWidth; }
            set { _imageWidth = value; }
        }

        public Brush ForeGround
        {
            get { return _foreground; }
            set { _foreground = value; }
        }

        protected override int VisualChildrenCount
        {
            get
            {
                if (null != _visualChildren)
                {
                    return _visualChildren.Count;
                }
                return 0;
            }
        }
        #endregion Properties

        #region Methods
        protected override Size ArrangeOverride(Size finalSize)
        {
            UpdateVisuals(this, EventArgs.Empty);
            return finalSize;
        }

        protected override Visual GetVisualChild(int index)
        {
            return _visualChildren?[index];
        }

        protected override void OnRender(DrawingContext drawingContext)
        { }

        private void BuildVisuals()
        {
            ROIRect rect = (this.AdornedElement as ROIRect);
            int numStripes = ((int[])(rect.Tag))[(int)ThorSharedTypes.Tag.MROI_STRIPE_COUNT];

            for (int i = 0; i < numStripes - 1; i++)
            {
                _stripeLines.Add(new Line() 
                {
                    X1 = 0,
                    X2 = 0,
                    Y1 = 0,
                    Y2 = rect.ROIHeight,
                    Stroke = _foreground, 
                    StrokeThickness = rect.StrokeThickness, 
                    //Dash array sizes are relative to the stroke thickness. This makes it so there are always 10 dashes per roi height 
                    StrokeDashArray = new DoubleCollection() { (rect.ROIHeight / rect.StrokeThickness) / 20, (rect.ROIHeight / rect.StrokeThickness) / 20 }
                });
            }

            foreach (Line line in _stripeLines)
            {
                _visualChildren.Add(line);
            }

        }

        private void UpdateVisuals(Object sender = null, EventArgs e = null)
        { 
            ROIRect tempRect = AdornedElement as ROIRect;
            int numStripes = ((int[])(tempRect.Tag))[(int)ThorSharedTypes.Tag.MROI_STRIPE_COUNT];

            double rectCenterY = tempRect.TopLeft.Y + tempRect.ROIHeight / 2;
            double lineCenterX = 0;
            double lineCenterY = 0;

            for (int i = 0; i < numStripes - 1; i++)
            {
                lineCenterX = tempRect.TopLeft.X + (tempRect.ROIWidth / numStripes) * (i + 1);
                lineCenterY = rectCenterY;

                if (_stripeLines.Count > i)
                {
                    _stripeLines[i].StrokeThickness = tempRect.StrokeThickness;
                    _stripeLines[i].StrokeDashArray = new DoubleCollection() {(tempRect.ROIHeight / tempRect.StrokeThickness)/20, (tempRect.ROIHeight / tempRect.StrokeThickness) / 20 };
                    _stripeLines[i].Y2 = tempRect.ROIHeight;
                    _stripeLines[i].Arrange(new Rect(new Point(lineCenterX, tempRect.TopLeft.Y), new Size(20, tempRect.ROIHeight)));
                }
            }
        }
        
        #endregion Methods
    }
}