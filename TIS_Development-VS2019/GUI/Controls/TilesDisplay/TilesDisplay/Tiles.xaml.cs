namespace TilesDisplay
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
    using System.Windows.Media.Imaging;
    using System.Windows.Navigation;
    using System.Windows.Shapes;

    #region Enumerations

    public enum TilingControlType
    {
        BUTTON = 0,
        LABEL = 1
    }

    public enum TilingMode
    {
        TopLeftMode = 0,
        TopLeftBottomRightMode = 1,
        ListMode = 2
    }

    #endregion Enumerations

    /// <summary>
    /// Interaction logic for UserControl1.xaml
    /// </summary>
    public partial class Tiles : UserControl, INotifyPropertyChanged
    {
        #region Fields

        public static DependencyProperty ColumnsProperty = 
        DependencyProperty.RegisterAttached("Columns",
        typeof(int),
        typeof(Tiles),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onColumnsChanged)));
        public static DependencyProperty ControlTypeProperty = 
        DependencyProperty.RegisterAttached("ControlType",
        typeof(int),
        typeof(Tiles),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onControlTypeChanged)));
        public static DependencyProperty CurrentXProperty = 
        DependencyProperty.RegisterAttached("CurrentX",
        typeof(double),
        typeof(Tiles),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onCurrentXChanged)));
        public static DependencyProperty CurrentYProperty = 
        DependencyProperty.RegisterAttached("CurrentY",
        typeof(double),
        typeof(Tiles),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onCurrentYChanged)));
        public static DependencyProperty FieldHeightProperty = 
        DependencyProperty.RegisterAttached("FieldHeight",
        typeof(double),
        typeof(Tiles),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onFieldHeightChanged)));
        public static DependencyProperty FieldWidthProperty = 
        DependencyProperty.RegisterAttached("FieldWidth",
        typeof(double),
        typeof(Tiles),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onFieldWidthChanged)));
        public static DependencyProperty ModeProperty = 
        DependencyProperty.RegisterAttached("Mode",
        typeof(int),
        typeof(Tiles),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onModeChanged)));
        public static DependencyProperty RowsProperty = 
        DependencyProperty.RegisterAttached("Rows",
        typeof(int),
        typeof(Tiles),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onRowsChanged)));
        public static DependencyProperty XSpacingProperty = 
        DependencyProperty.RegisterAttached("XSpacing",
        typeof(double),
        typeof(Tiles),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onXSpacingChanged)));
        public static DependencyProperty YSpacingProperty = 
        DependencyProperty.RegisterAttached("YSpacing",
        typeof(double),
        typeof(Tiles),
        new FrameworkPropertyMetadata(new PropertyChangedCallback(onYSpacingChanged)));

        private static int _columns = 0;
        private static int _controlType = 0;
        private static double _currentX = 0; // this is a property that indicates where the imaging experiment is at
        private static double _currentY = 0;
        private static double _fieldHeight = 1.0;
        private static double _fieldWidth = 1.0;
        private static TilingMode _mode = TilingMode.TopLeftMode;

        // for Top Left mode and Top left bottom right mode
        private static int _rows = 0;
        private static double _xSpacing; // in percentage
        private static double _ySpacing; // in percentage

        private Point _bottomRight;

        // for List Mode
        private List<Point> _centerList = new List<Point>(); // stores the center locations for all the child controls in List Mode
        private UIElementCollection _children; // this references to the child controls corrensponding to which mode it is at
        private int _clickedButtonIndex = -1;
        private int _currentImageIndex = -1; // indicating the control index of the area under experiment
        private List<int> _finished = new List<int>(); // the control index in the grid, representing the areas finished imaging
        private double _maxX;
        private double _maxY;
        private double _minX;
        private double _minY;
        private double _scaleFactor = 1.0; // indicates the ratio between redered image size to fieldWidth, fieldHeight
        private Point _topLeft;

        #endregion Fields

        #region Constructors

        public Tiles()
        {
            InitializeComponent();
            _children = myTilesGrid.Children;
        }

        #endregion Constructors

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        public event Action<int> TileClick;

        #endregion Events

        #region Properties

        public Point BottomRight
        {
            get
            {
                return _bottomRight;
            }
            set
            {
                if (value != BottomRight)
                {
                    this._bottomRight = value;
                    OnPropertyChanged("BottomRight");
                }
            }
        }

        public int CanvasHeight
        {
            get
            {
                return (int)this.myTilesCanvas.ActualHeight;
            }
        }

        public int CanvasWidth
        {
            get
            {
                return (int)this.myTilesCanvas.ActualWidth;
            }
        }

        public int ClickedButtonIndex
        {
            get
            {
                return _clickedButtonIndex;
            }
        }

        public int Columns
        {
            get
            {
                { return (int)GetValue(ColumnsProperty); }
            }
            set
            {
                SetValue(ColumnsProperty, value);
            }
        }

        public int ColumnWidth
        {
            get
            {
                if (Columns <= 0)
                {
                    return 1;
                }
                else
                {
                    //this.UpdateLayout();
                    return (int)this.myTilesGrid.Width / Columns;
                }
            }
        }

        public int ControlType
        {
            get
            {
                return (int)GetValue(ControlTypeProperty);
            }
            set
            {
                SetValue(ControlTypeProperty, value);
            }
        }

        public int CurrentImageIndex
        {
            get
            {
                return _currentImageIndex;
            }
            set
            {
                if (value != CurrentImageIndex)
                {
                    this._currentImageIndex = value;
                    clearColor();
                    colorChildControl(this._currentImageIndex, Brushes.Green);
                    OnPropertyChanged("CurrentImageIndex");
                }
            }
        }

        public double CurrentX
        {
            get
            {
                return (double)GetValue(CurrentXProperty);
            }
            set
            {
                SetValue(CurrentXProperty, value);
            }
        }

        public double CurrentY
        {
            get
            {
                return (double)GetValue(CurrentYProperty);
            }
            set
            {
                SetValue(CurrentYProperty, value);
            }
        }

        public double FieldHeight
        {
            get
            {
                return (double)GetValue(FieldHeightProperty);
            }
            set
            {
                SetValue(FieldHeightProperty, value);
            }
        }

        public double FieldWidth
        {
            get
            {
                return (double)GetValue(FieldWidthProperty);
            }
            set
            {
                SetValue(FieldWidthProperty, value);
            }
        }

        public double MaxX
        {
            get
            {
                return _maxX;
            }
        }

        public double MaxY
        {
            get
            {
                return _maxY;
            }
        }

        public double MinX
        {
            get
            {
                return _minX;
            }
        }

        public double MinY
        {
            get
            {
                return _minY;
            }
        }

        public int Mode
        {
            get
            {
                return (int)GetValue(ModeProperty);
            }
            set
            {
                SetValue(ModeProperty, value);
            }
        }

        public int RowHeight
        {
            get
            {
                if (Rows <= 0)
                {
                    return 1;
                }
                else
                {
                    //this.UpdateLayout();
                    return (int)this.myTilesGrid.Height / Rows;
                }
            }
        }

        public int Rows
        {
            get
            {
                return (int)GetValue(RowsProperty);
            }
            set
            {
                SetValue(RowsProperty, value);
            }
        }

        public double ScaleFactor
        {
            get
            {
                return _scaleFactor;
            }
            set
            {
                if (value != ScaleFactor)
                {
                    _scaleFactor = Math.Max(.000001,value);
                    OnPropertyChanged("ScaleFactor");
                }
            }
        }

        public Point TopLeft
        {
            get
            {
                return _topLeft;
            }
            set
            {
                if (value != TopLeft)
                {
                    this._topLeft = value;
                    OnPropertyChanged("TopLeft");
                }
            }
        }

        public double XSpacing
        {
            get
            {
                return (double)GetValue(XSpacingProperty);
            }
            set
            {
                SetValue(XSpacingProperty, value);
            }
        }

        public double YSpacing
        {
            get
            {
                return (double)GetValue(YSpacingProperty);
            }
            set
            {
                SetValue(YSpacingProperty, value);
            }
        }

        #endregion Properties

        #region Methods

        public static void onColumnsChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            int post = Convert.ToInt32(e.NewValue);
            if (post > 0)
            {
                int pre = _columns;
                _columns = post;
                (d as Tiles).updateGridColumns(post - pre);
                (d as Tiles).OnPropertyChanged("ColumnWidth");
                (d as Tiles).OnPropertyChanged("ScaleFactor");
            }
        }

        public static void onControlTypeChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            try
            {
                _controlType = Convert.ToInt32(e.NewValue);
                (d as Tiles).ClearRowsAndColumns();
                (d as Tiles).addAllControls();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "ControlType changed inappropriately");
            }
        }

        public static void onCurrentXChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            try
            {
                _currentX = Convert.ToDouble(e.NewValue);
                (d as Tiles).OnPropertyChanged("CurrentX");
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "CurrentX changed inappropriately");
            }
        }

        public static void onCurrentYChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            try
            {
                _currentY = Convert.ToDouble(e.NewValue);
                (d as Tiles).OnPropertyChanged("CurrentY");
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "CurrentY changed inappropriately");
            }
        }

        public static void onFieldHeightChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            try
            {
                _fieldHeight = Convert.ToDouble(e.NewValue);
                (d as Tiles).updateGridRows(0);
                (d as Tiles).OnPropertyChanged("FieldHeight");
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Field Height changed inappropriately");
            }
        }

        public static void onFieldWidthChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            try
            {
                _fieldWidth = Convert.ToDouble(e.NewValue);
                (d as Tiles).updateGridColumns(0);
                (d as Tiles).OnPropertyChanged("FieldWidth");
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Field Width changed inappropriately");
            }
        }

        public static void onModeChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            try
            {
                _mode = (TilingMode)Convert.ToInt32(e.NewValue);
                (d as Tiles).changeToMode(_mode);
                (d as Tiles).OnPropertyChanged("Mode");
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Mode changed inappropriately");
            }
        }

        public static void onRowsChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            int post = Convert.ToInt32(e.NewValue);
            if (post > 0)
            {
                int pre = _rows;
                _rows = post;
                (d as Tiles).updateGridRows(post - pre);
                (d as Tiles).OnPropertyChanged("RowHeight");
                (d as Tiles).OnPropertyChanged("ScaleFactor");
            }
        }

        public static void onXSpacingChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            try
            {
                _xSpacing = Convert.ToInt32(e.NewValue);
                _xSpacing = Math.Min(100.0, Math.Max(-100.0, _xSpacing));
                (d as Tiles).updateGridSpacing();
                (d as Tiles).OnPropertyChanged("ColumnWidth");
                (d as Tiles).OnPropertyChanged("ScaleFactor");
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "XSpacing out of range");
            }
        }

        public static void onYSpacingChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            try
            {
                _ySpacing = Convert.ToInt32(e.NewValue);
                _ySpacing = Math.Min(100.0, Math.Max(-100.0, _ySpacing));
                (d as Tiles).updateGridSpacing();
                (d as Tiles).OnPropertyChanged("RowHeight");
                (d as Tiles).OnPropertyChanged("ScaleFactor");
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "YSpacing changed inappropriately");
            }
        }

        /// <summary>
        /// This method would be triggered by the application to add a button representation in List Mode
        /// </summary>
        /// <param name="top">distance to canvas top</param>
        /// <param name="left">distance to canvas left</param>
        public void addChildButtonToCanvas(double x, double y)
        {
            Button btn = new Button();
            btn.Style = null;
            myTilesCanvas.Children.Add(btn);
            btn.Click += btnClick;
            _centerList.Add(new Point(x, y));
            updateCanvasBounds(x, y);
            scaleAllControls();
        }

        /// <summary>
        /// Change to the specified mode
        /// </summary>
        /// <param name="tMode">the mode changes to</param>
        public void changeToMode(TilingMode tMode)
        {
            // switch between a grid and a canvas according
            // to which mode is selected
            switch (tMode)
            {
                case TilingMode.ListMode:
                    {
                        clear(myTilesGrid);
                        Panel.SetZIndex(myTilesGrid, 0);
                        Panel.SetZIndex(myTilesCanvas, 1);
                        myTilesGrid.Visibility = Visibility.Collapsed;
                        myTilesCanvas.Visibility = Visibility.Visible;
                        _children = myTilesCanvas.Children;
                    }
                    break;
                default:
                    {
                        clear(myTilesCanvas);
                        clear(myTilesGrid);
                        Panel.SetZIndex(myTilesGrid, 1);
                        Panel.SetZIndex(myTilesCanvas, 0);
                        myTilesGrid.Visibility = Visibility.Visible;
                        myTilesCanvas.Visibility = Visibility.Collapsed;
                        _children = myTilesGrid.Children;
                    }
                    break;
            }
        }

        public void clearColor()
        {
            for (int i = 0; i < _children.Count; i++)
            {
                Control control = (Control)_children[i];
                control.Background = Brushes.White;
            }
        }

        public void ClearRowsAndColumns()
        {
            clearRows();
            clearColumns();
            _rows = 0;
            _columns = 0;
        }

        /// <summary>
        /// change the specified child control's background color
        /// </summary>
        /// <param name="r">row</param>
        /// <param name="c">column</param>
        /// <param name="color">color</param>
        public void colorChildControl(int r, int c, Brush color)
        {
            int index = transToIndex(r, c);
            colorChildControl(index, color);
        }

        public void colorChildControl(int index, Brush color)
        {
            if (index >= 0 && index < _children.Count)
            {
                Control control = (Control)_children[index];
                control.Background = color;
                this.InvalidateArrange();
            }
            else
            {
            //                MessageBox.Show("Illegal arguments in method: colorChildControl. The specified element does not exist.");
            }
        }

        //double deltaX, double deltaY)
        /// <summary>
        /// redraw the grid based on the set first and last position (Mode 2)
        /// The details about generating rows and columns have to be discussed
        /// The following method for now based on the assamptiong that the start location and last location
        /// are on the center of the top left and bottom right images
        /// </summary>
        public void generateGrid()
        {
            double deltaX = Math.Abs(BottomRight.X - TopLeft.X);
            double deltaY = Math.Abs(BottomRight.Y - TopLeft.Y);
            double xSpace = XSpacing * 0.01 * FieldWidth;
            double ySpace = YSpacing * 0.01 * FieldHeight;

            int col = Convert.ToInt32(Math.Ceiling((deltaX + FieldWidth + 2 * xSpace) / (FieldWidth + 2 * xSpace)));
            int row = Convert.ToInt32(Math.Ceiling((deltaY + FieldHeight + 2 * ySpace) / (FieldHeight + 2 * ySpace)));

            string str = string.Format("Columns = {0}\nRows = {1}\nClick Yes to accept these dimensions",col,row);
            if (MessageBox.Show(str, "Calculated Mosaic dimensions", MessageBoxButton.YesNo) == MessageBoxResult.Yes)
            {
                Columns = Convert.ToInt32(Math.Ceiling((deltaX + FieldWidth + 2 * xSpace) / (FieldWidth + 2 * xSpace)));
                Rows = Convert.ToInt32(Math.Ceiling((deltaY + FieldHeight + 2 * ySpace) / (FieldHeight + 2 * ySpace)));
            }
        }

        /// <summary>
        /// Using the Canvas, it scales with the window. All the child controls have x, y locations relative
        /// to its Top and Left
        /// </summary>
        /// <param name="deltaX">The horizontal difference between the Tile with maximum value and the one with minimum</param>
        /// <param name="deltaY">The vertical difference between the Tile with maximum value and the one with minimum</param>
        /// <returns>ratio</returns>
        public double getCanvasScaleFactor(double deltaX, double deltaY)
        {
            if (FieldHeight <= 0 || FieldWidth <= 0)
            {
                return 1.0;
            }
            else
            {
                return Math.Min(CanvasHeight / (deltaY + FieldHeight),
                                CanvasWidth / (deltaX + FieldWidth));
            }
        }

        /// <summary>
        /// When using the Grid, the user control that contains the Grid, scales with the window
        /// </summary>
        /// <returns>ratio between redered sizes vs the fieldWidth, fieldHeight</returns>
        public double getGridScaleFactor()
        {
            if (FieldHeight <= 0 || FieldWidth <= 0 || Rows <= 0 || Columns <= 0)
            {
                return 1.0;
            }
            else
            {
                return Math.Min(myUC.ActualHeight / (Rows * FieldHeight * (1 + 0.01 * YSpacing)),
                                myUC.ActualWidth / (Columns * FieldWidth * (1 + 0.01 * XSpacing)));
            }
        }

        // remove the selected control from the canvas in List Mode
        public void removeControlFromCanvas(int index)
        {
            if (myTilesCanvas.Children.Count > 1 && index < myTilesCanvas.Children.Count)
            {
                myTilesCanvas.Children.RemoveAt(index);
                _centerList.RemoveAt(index);
                _minX = _centerList[0].X;
                _maxX = _centerList[0].X;
                _minY = _centerList[0].Y;
                _maxY = _centerList[0].Y;
                foreach (Point pt in _centerList)
                {
                    updateCanvasBounds(pt.X, pt.Y);
                }
                scaleAllControls();
            }
            else if (myTilesCanvas.Children.Count == 1 && index == 0)
            {
                myTilesCanvas.Children.RemoveAt(0);
            }
            else
            {
                MessageBox.Show("Illegal arguments in method: removeControlFromCanvas. No elements to remove.");
            }
        }

        /// <summary>
        /// set the content of the button using specified image
        /// </summary>
        /// <param name="btn">Button Control to edit</param>
        /// <param name="imgPath">relative path of the image</param>
        public void setChildControlImage(Button btn, string imgPath)
        {
            BitmapImage btm = new BitmapImage(new Uri(imgPath, UriKind.Relative));
            Image img = new Image();
            img.Source = btm;
            img.Stretch = Stretch.Fill;
            btn.Content = img;
        }

        public void setChildControlImage(int index, BitmapImage bmp)
        {
            if (index >= 0 && index < _children.Count)
            {
                Button btn = (Button)_children[index];
                Image img = new Image();
                img.Source = bmp;
                img.Stretch = Stretch.Uniform;
                btn.Content = img;
            }
        }

        public void stepThrough()
        {
            if (CurrentImageIndex < _children.Count)
            {
                Control control = (Control)_children[CurrentImageIndex];
                if (control.Background == Brushes.Blue)
                {
                    _finished.Add(CurrentImageIndex);
                    colorChildControl(CurrentImageIndex, Brushes.Green); // color the previous image indicator to green when done
                    CurrentImageIndex++;
                }
                colorChildControl(CurrentImageIndex, Brushes.Blue); // color the new image indicator to blue
            }
        }

        public void testSetup()
        {
            colorChildControl(0, Brushes.Blue);
        }

        /// <summary>
        /// add all the child controls (Buttons) to the grid in TopLeft and TopLeftBottomRight Mode
        /// </summary>
        private void addAllControls()
        {
            ScaleFactor = getGridScaleFactor();
            resizeContainer();
            for (int i = 0; i < Rows; i++)
            {
                for (int j = 0; j < Columns; j++)
                {
                    if ((int)TilingControlType.LABEL == ControlType)
                    {
                        addChildLabelToGrid(i, j);
                    }
                    else
                    {
                        addChildButtonToGrid(i, j);
                    }
                }
            }
        }

        /// <summary>
        ///  add single child button to the specified position (row and column)
        ///  to the grid in TopLeft and TopLeftBottomRight Mode
        /// </summary>
        /// <param name="r">specified row number to add control in</param>
        /// <param name="c">specified column number to add control in</param>
        private void addChildButtonToGrid(int r, int c)
        {
            try
            {
                Button newButton = new Button();
                newButton.Style = null;
                Grid.SetColumn(newButton, c);
                Grid.SetRow(newButton, r);
                Grid.SetZIndex(newButton, 0);
                myTilesGrid.Children.Add(newButton);

                newButton.Width = FieldWidth * ScaleFactor;
                newButton.Height = FieldHeight * ScaleFactor;
                // set image to button
                //setChildControlImage(newButton, "Resources/Forest Flowers.jpg");

                double left = XSpacing / 2 * 0.01 * newButton.Width;
                double top = YSpacing / 2 * 0.01 * newButton.Height;
                newButton.Margin = new Thickness(left, top, left, top);

                // to set the ZIndex so when mouse is over the button, that button can be completely viewed
                newButton.MouseEnter += mouseEnter;
                newButton.MouseLeave += mouseLeave;
                newButton.Click += btnClick;
            }
            catch (DivideByZeroException ex)
            {
                MessageBox.Show(ex.Message, "FieldWidth or FieldHeight value is not set");
            }
        }

        /// <summary>
        ///  add single child label to the specified position (row and column)
        ///  to the grid in TopLeft and TopLeftBottomRight Mode
        /// </summary>
        /// <param name="r">specified row number to add control in</param>
        /// <param name="c">specified column number to add control in</param>
        private void addChildLabelToGrid(int r, int c)
        {
            try
            {
                Label newLabel = new Label();
                newLabel.Style = null;
                Grid.SetColumn(newLabel, c);
                Grid.SetRow(newLabel, r);
                Grid.SetZIndex(newLabel, 0);
                myTilesGrid.Children.Add(newLabel);

                newLabel.Width = FieldWidth * ScaleFactor;
                newLabel.Height = FieldHeight * ScaleFactor;

                double left = XSpacing / 2 * 0.01 * newLabel.Width;
                double top = YSpacing / 2 * 0.01 * newLabel.Height;
                newLabel.Margin = new Thickness(left, top, left, top);

                // to set the ZIndex so when mouse is over the Label, that Label can be completely viewed
                newLabel.MouseEnter += mouseEnter;
                newLabel.MouseLeave += mouseLeave;
                newLabel.Background = Brushes.White;
            }
            catch (DivideByZeroException ex)
            {
                MessageBox.Show(ex.Message, "FieldWidth or FieldHeight value is not set");
            }
        }

        // add one column to grid
        private void addColumn()
        {
            ColumnDefinition colDef = new ColumnDefinition();
            myTilesGrid.ColumnDefinitions.Add(colDef);
        }

        // add the specified number of columns to grid
        private void addColumns(int count)
        {
            for (int i = 0; i < count; i++)
            {
                addColumn();
            }
        }

        // add one row to grid
        private void addRow()
        {
            RowDefinition rowDef = new RowDefinition();
            myTilesGrid.RowDefinitions.Add(rowDef);
        }

        // add the specified number of rows to grid
        private void addRows(int count)
        {
            for (int i = 0; i < count; i++)
            {
                addRow();
            }
        }

        // keep the clicked button index in _clickedButtonIndex variable
        private void btnClick(object sender, RoutedEventArgs e)
        {
            Button btn = sender as Button;
            _clickedButtonIndex = _children.IndexOf(btn);
            CurrentY = (int)(_clickedButtonIndex / Columns);
            CurrentX = _clickedButtonIndex - CurrentY * Columns;
            CurrentImageIndex = (0 < CurrentY) ? (int)CurrentX + (int)(Columns * CurrentY) : (int)CurrentX;
            OnPropertyChanged("ClickedButtonIndex");
            OnPropertyChanged("CurrentImageIndex");

            if (TileClick != null)
            {
                TileClick(_clickedButtonIndex);
            }
        }

        private void clear(Panel pnl)
        {
            pnl.Children.Clear();
            CurrentImageIndex = 0;
            Rows = 0;
            Columns = 0;
            clearRows();
            clearColumns();
            _clickedButtonIndex = -1;
            OnPropertyChanged("Rows");
            OnPropertyChanged("Columns");
            OnPropertyChanged("RowHeight");
            OnPropertyChanged("ColumnWidth");
            OnPropertyChanged("ClickedButtonIndex");
            UpdateLayout();
        }

        // remove all the columns from grid
        private void clearColumns()
        {
            if (myTilesGrid.ColumnDefinitions.Count > 0)
                myTilesGrid.ColumnDefinitions.Clear();
        }

        // remove all the rows from grid
        private void clearRows()
        {
            if (myTilesGrid.RowDefinitions.Count > 0)
                myTilesGrid.RowDefinitions.Clear();
        }

        private void mouseEnter(object sender, MouseEventArgs e)
        {
            Control contorl = sender as Control;
            Grid.SetZIndex(contorl, 1);
        }

        private void mouseLeave(object sender, MouseEventArgs e)
        {
            Control control = sender as Control;
            Grid.SetZIndex(control, 0);
        }

        private void OnPropertyChanged(string propertyName)
        {
            if (PropertyChanged != null)
            {
                this.PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }

        private void OnSizeChanged(object sender, SizeChangedEventArgs e)
        {
            OnPropertyChanged("RowHeight");
            OnPropertyChanged("ColumnWidth");
            updateGridSpacing();
            OnPropertyChanged("CanvasHeight");
            OnPropertyChanged("CanvasWidth");
            scaleAllControls();
        }

        // remove one column from grid
        private void removeColumn()
        {
            if (myTilesGrid.ColumnDefinitions.Count <= 0)
            {
                _columns = 0;
            }
            else
            {
                myTilesGrid.ColumnDefinitions.RemoveAt(0);
            }
        }

        // remove the specified number of columns
        private void removeColumns(int count)
        {
            for (int i = 0; i < count; i++)
            {
                removeColumn();
            }
        }

        // remove one row from grid
        private void removeRow()
        {
            if (myTilesGrid.RowDefinitions.Count <= 0)
            {
                _rows = 0;
            }
            else
            {
                myTilesGrid.RowDefinitions.RemoveAt(0);
            }
        }

        // remove the specified number of rows from grid
        private void removeRows(int count)
        {
            for (int i = 0; i < count; i++)
            {
                removeRow();
            }
        }

        /// <summary>
        /// resize the user control size to fill the window properly in TopLeft and TopLeftBottomRight Mode
        /// </summary>
        private void resizeContainer()
        {
            double h = ScaleFactor * (Rows * FieldHeight * (1 + 0.01 * YSpacing));
            double w = ScaleFactor * (Columns * FieldWidth * (1 + 0.01 * XSpacing));

            w = Math.Max(w, 10);
            h = Math.Max(h, 10);

            myTilesGrid.Width = w;
            myTilesGrid.Height = h;
        }

        /// <summary>
        /// scale the sizes of child controls and make them fit properly in the Canvas
        /// </summary>
        private void scaleAllControls()
        {
            ScaleFactor = getCanvasScaleFactor(MaxX - MinX, MaxY - MinY);
            if (ScaleFactor < 0)
                return;
            // these values indicates the length of extra space at horizontal or vertical edges,
            // these variable are used to make the UI appear in the center of Canvas
            double xExtra = CanvasWidth - (MaxX - MinX + FieldWidth) * ScaleFactor;
            double yExtra = CanvasHeight - (MaxY - MinY + FieldHeight) * ScaleFactor;

            foreach (Control control in myTilesCanvas.Children)
            {
                control.Height = FieldHeight * ScaleFactor;
                control.Width = FieldWidth * ScaleFactor;
                int index = myTilesCanvas.Children.IndexOf(control);
                int x = (int)_centerList.ElementAt(index).X;
                int y = (int)_centerList.ElementAt(index).Y;
                Canvas.SetLeft(control, (x - MinX) * ScaleFactor + xExtra / 2);
                Canvas.SetTop(control, (y - MinY) * ScaleFactor + yExtra / 2);
            }
        }

        /// <summary>
        /// interprete the row, column into element index in the children collection controls
        /// </summary>
        /// <param name="r">row number</param>
        /// <param name="c">column number</param>
        /// <returns>index</returns>
        private int transToIndex(int r, int c)
        {
            return r * Columns + c;
        }

        /// <summary>
        /// this updates the MaxX, MinX, MaxY, MinY properties when
        /// a new imaging position is selected
        /// </summary>
        /// <param name="x"></param>
        /// <param name="y"></param>
        private void updateCanvasBounds(double x, double y)
        {
            if (myTilesCanvas.Children.Count == 1)
            {
                _minX = x;
                _maxX = x;
                _minY = y;
                _maxY = y;
            }
            else
            {
                if (x < MinX)
                    _minX = x;
                if (x > MaxX)
                    _maxX = x;
                if (y < MinY)
                    _minY = y;
                if (y > MaxY)
                    _maxY = y;
            }
            OnPropertyChanged("MinX");
            OnPropertyChanged("MaxX");
            OnPropertyChanged("MinY");
            OnPropertyChanged("MaxY");
        }

        /// <summary>
        /// Update the UI when the Columns property changes
        /// </summary>
        /// <param name="diff">number of columns added or removed</param>
        private void updateGridColumns(int diff)
        {
            if (diff > 0)
                addColumns(diff);
            else
                removeColumns(Math.Abs(diff));
            myTilesGrid.Children.Clear();
            addAllControls();
        }

        /// <summary>
        /// Update the UI when the Rows property changes
        /// </summary>
        /// <param name="diff">number of rows added or removed</param>
        private void updateGridRows(int diff)
        {
            if (diff > 0)
                addRows(diff);
            else
                removeRows(Math.Abs(diff));
            myTilesGrid.Children.Clear();
            addAllControls();
        }

        /// <summary>
        ///  update the UI when the spacing is changed by client
        /// </summary>
        private void updateGridSpacing()
        {
            ScaleFactor = getGridScaleFactor();
            resizeContainer();
            foreach (Control control in myTilesGrid.Children)
            {
                control.Width = FieldWidth * ScaleFactor;
                control.Height = FieldHeight * ScaleFactor;
                double left = XSpacing / 2 * 0.01 * control.Width;
                double top = YSpacing / 2 * 0.01 * control.Height;
                control.Margin = new Thickness(left, top, left, top);
            }
        }

        #endregion Methods
    }
}