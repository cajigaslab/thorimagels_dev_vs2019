namespace SampleRegionSelection
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
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
    using SampleRegionSelection;
    using ThorSharedTypes;
    using System.Text.RegularExpressions;
   
    /// <summary>
    /// Interaction logic for UserControl1.xaml
    /// </summary>
    public partial class SampleRegionSelectionView : UserControl, INotifyPropertyChanged
    {
        #region Fields

        bool mouseDown = false; // Set to 'true' when mouse is held down.
        Point mouseDownPos; // The point where the mouse button was clicked down.
        private string _buttonName;
        private string _textBoxName;
        private ObservableCollection<Control> _collection;
        private ObservableCollection<Control> _collectionTopLabel;
        private ObservableCollection<Control> _collectionLeftLabel;
        private int _columnCount;
        private string _labelLeftName;
        private string _labelName;
        private string _title;
        private int _leftLabelCount;
        private int _rowCount;
        private ThorSharedTypes.SampleType _sampleType;
        private int _topLabelCount;
        private int _startRow;
        private int _startColumn;
        private int _numberOfRows;
        private int _numberOfCols;
        private ControlType _controlType;
        private int _plateWidth;
        private int _plateHeight;
        SampleRegionFactory _factory;
        private List<Color> _colorList;
        private List<String> _contentList;

        public event Action<int, int> Item_Clicked;

        int changeCount = 0;
        public List<String> _OriginalcontentList;

        private List<int> _wellList;

        #endregion Fields

        public enum ControlType
        {
            BUTTON,
            LABEL,
            TEXTBOX                      
        }

        #region Constructors

        public SampleRegionSelectionView()
        {
           InitializeComponent();

           //inorder to invoke the bindings
           base.DataContext = this;

           _buttonName = "Button";
           _labelName = "TopLabel";
           _labelLeftName = "LeftLabel";
           _textBoxName = "TextBox";
           TitleVisibility = Visibility.Collapsed;
           _factory = new SampleRegionFactory();
        }

        #endregion Constructors

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public Visibility TitleVisibility { get; set; }

        public string Title
        {
            get
            {
                return _title;
            }
            set
            {
                _title = value;
                OnPropertyChanged("Title");
            }
        }

        public bool IsEnableDragCanvas { get; set; }

        public int ColumnCount
        {
            get
            {
                return _columnCount;
            }
        }

        public int LeftLabelCount
        {
            get
            {
                return _leftLabelCount;
            }
            set
            {
                _leftLabelCount = value;
                OnPropertyChanged("LeftLabelCount");
            }
        }

        public ObservableCollection<Control> ObjectCollection
        {
            get
            {
                if (_collection == null)
                {
                    CreateCollection();                   
                }
                else
                {
                    _collection.Clear();

                    CreateCollection();                  
                }

                return _collection;
            }
            set
            {
                _collection = value;
                OnPropertyChanged("ObjectCollection");
            }
        }

        public ObservableCollection<Control> ObjectLeftLabelCollection
        {
            get
            {
                if ( _collectionLeftLabel == null)
                {
                    CreateLeftLabelCollection();
                }
                else
                {                   
                    _collectionLeftLabel.Clear();

                    CreateLeftLabelCollection();
                }

                return _collectionLeftLabel; 
            }
            set { _collectionLeftLabel = value;
            OnPropertyChanged("ObjectLeftLabelCollection"); 
          
            }
        }

        public ObservableCollection<Control> ObjectTopLabelCollection
        {
            get {

                if (_collectionTopLabel == null)
                {
                    CreateTopLabelCollection();
                }
                else
                {
                    _collectionTopLabel.Clear();

                    CreateTopLabelCollection();
                }

                return _collectionTopLabel;            
            
            }
            set { _collectionTopLabel = value;
            OnPropertyChanged("ObjectTopLabelCollection");
            }
        }    

        public int RowCount
        {
            get
            {
                return _rowCount;
            }
        }

        public ControlType PlateControl
        {
            get
            {
                return _controlType;

            }
            set
            {
                _controlType = value;
                OnPropertyChanged("PlateControl");
            }
        }

        public int PlateHeight
        {
            get
            {

                return _plateHeight;
            }
            set
            {
                _plateHeight = value;
                OnPropertyChanged("PlateHeight");
            }
        }

        public int PlateWidth
        {
            get
            {
                return _plateWidth;
            }
            set
            {
                _plateWidth = value;
                OnPropertyChanged("PlateWidth");
            }
        } 

        public int SampleType
        {
            get
            {               
                return (int)_sampleType;
            }
            set
            {
                _sampleType = (ThorSharedTypes.SampleType)value;

                switch (_sampleType)
                {
                    case ThorSharedTypes.SampleType.WELL1536:
                        {
                            _columnCount = ThorSharedTypes.SampleDimensions.Columns(_sampleType);
                            _rowCount = ThorSharedTypes.SampleDimensions.Rows(_sampleType);
                            _topLabelCount = 24;
                            _leftLabelCount = 15;
                            _sampleType = ThorSharedTypes.SampleType.WELL1536;
                        }
                        break;
                    default:
                        {
                            _columnCount = ThorSharedTypes.SampleDimensions.Columns(_sampleType);
                            _rowCount = ThorSharedTypes.SampleDimensions.Rows(_sampleType);
                            _topLabelCount = ThorSharedTypes.SampleDimensions.Columns(_sampleType);
                            _leftLabelCount = ThorSharedTypes.SampleDimensions.Rows(_sampleType);
                        }
                        break;
                }
  
                OnPropertyChanged("PlateControl");  
                OnPropertyChanged("RowCount");
                OnPropertyChanged("ColumnCount");
                OnPropertyChanged("LeftLabelCount");
                OnPropertyChanged("TopLabelCount");
                OnPropertyChanged("StartRow");
                OnPropertyChanged("StartColumn");
                OnPropertyChanged("NumberOfRows");
                OnPropertyChanged("NumberOfColumns");
                OnPropertyChanged("ObjectTopLabelCollection");
                OnPropertyChanged("ObjectLeftLabelCollection");
                OnPropertyChanged("ObjectCollection");        
                OnPropertyChanged("PlateHeight");
                OnPropertyChanged("PlateWidth");
                OnPropertyChanged("SampleType");  
            }
        }

        public int TopLabelCount
        {
            get
            {
                return _topLabelCount;
            }
            set
            {
                _topLabelCount = value;
                OnPropertyChanged("TopLabelCount");
            }
        }

        public int StartRow
        {
            get
            {

                return _startRow;
            }
            set
            {
                _startRow = value;
                OnPropertyChanged("StartRow");
            }
        }      

        public int StartColumn
        {
            get
            {

                return _startColumn;
            }
            set
            {
                _startColumn = value;
                OnPropertyChanged("StartColumn");
            }
        }

        public int NumberOfRows
        {
            get
            {

                return _numberOfRows;
            }
            set
            {
                _numberOfRows = value;
                OnPropertyChanged("NumberOfRows");
            }
        }

        public int NumberOfColumns
        {
            get
            {
                return _numberOfCols;
            }
            set
            {
                _numberOfCols = value;
                OnPropertyChanged("NumberOfColumns");
            }
        }

        public List<Color> ColorList
        {
            get
            {
                return _colorList;
            }
            set
            {
                _colorList = value;

                OnPropertyChanged("ObjectTopLabelCollection");
                OnPropertyChanged("ObjectLeftLabelCollection");
                OnPropertyChanged("ObjectCollection");
            }
        }       

        public List<String> ContentList
        {
            get
            {
                return _contentList;
            }
            set
            {
                _contentList = value;

                if (changeCount == 0)
                {
                    _OriginalcontentList = _contentList;
                    changeCount++;
                }

                OnPropertyChanged("ObjectTopLabelCollection");
                OnPropertyChanged("ObjectLeftLabelCollection");
                OnPropertyChanged("ObjectCollection");
            }
        }

        public List<int> WellList
        {
            get
            {
                return _wellList;
            }
            set
            {
                _wellList = value;              
            }
        }

        #endregion Properties

        private int _indexOfLastButtonClick=-1;
        #region Methods

        void BtnWell_Click(object sender, RoutedEventArgs e)
        {
            if((-1 != _indexOfLastButtonClick)&&(_indexOfLastButtonClick < _collection.Count))
            {
                if (_collection[_indexOfLastButtonClick].GetType().Equals(typeof(Button)))
                {
                    ((Button)_collection[_indexOfLastButtonClick]).Background = Brushes.White;
                }
            }
            //the user of the control has register and event handler
            if (Item_Clicked != null)
            {
                string pat = @"Button(.*)";
                string[] strResult = Regex.Split(((Button)sender).Name, pat);

                if(strResult.Length >= 1)
                {
                    int index = Convert.ToInt32(strResult[1].ToString());

                    int row = (index - 1) / ColumnCount;

                    int col = (index - (row * ColumnCount));

                    Item_Clicked(row + 1, col);

                    ((Button)sender).Background = Brushes.Green;

                    StartRow = row+1;
                    StartColumn = col;

                    //zero based index of button clicked
                    _indexOfLastButtonClick = index-1;
                }
            }
        }


        public void SendButtonClick(int row, int column)
        {
            if ((-1 != _indexOfLastButtonClick) && (_indexOfLastButtonClick < _collection.Count))
            {
                if (_collection[_indexOfLastButtonClick].GetType().Equals(typeof(Button)))
                {
                    ((Button)_collection[_indexOfLastButtonClick]).Background = Brushes.White;
                }
            }


            int index = (column - 1) + (row-1)*ColumnCount;

            if (_collection != null)
            {
                if ((index >= 0) && (_collection.Count > index))
                {
                    ((Button)_collection[index]).Background = Brushes.Green;

                    Item_Clicked(row, column);

                    StartRow = row;
                    StartColumn = column;

                    //zero based index of button clicked
                    _indexOfLastButtonClick = index;

                }
            }
        }

        void CreateCollection()
        {
            int index = 0;

            //rowlist and colist calculation used to retain the previous selected wells.
            List<int> rowList = new List<int>();
            int startRow = _startRow;

            while (rowList.Count != _numberOfRows)
            {
                rowList.Add(startRow++);
            }

            List<int> colList = new List<int>();
            int startCol = _startColumn;

            while (colList.Count != _numberOfCols)
            {
                colList.Add(startCol++);
            }
            
            //determine which plate control to build
            if (PlateControl == ControlType.TEXTBOX)
            {
                SampleCollectionBase textObj = _factory.GetControl((int)ControlType.TEXTBOX);
                _collection = textObj.CreateObservableCollection();

                int totalCount = RowCount * ColumnCount;

                //creating the Text controls
                for (int row = 0; row < _rowCount; row++)
                {
                    for (int col = 0; col < _columnCount; col++)
                    {
                        index++;
                        string content = ContentList[index - 1];
                        string tooltip = content;
                        string txtName = _textBoxName + Convert.ToString(index);

                        if (ColorList != null)
                        {
                            _collection.Add(textObj.AddControl(ColorList[index - 1], txtName, content, tooltip,null));
                        }
                        else
                        {
                            _collection.Add(textObj.AddControl(Color.FromRgb(255, 255, 255), txtName, content, tooltip,null));
                        }                        
                    }
                }
            }
            else 
            {
                SampleCollectionBase buttonObj = _factory.GetControl((int)ControlType.BUTTON);
                _collection = buttonObj.CreateObservableCollection();

                int totalCount = RowCount * ColumnCount;

                //creating the WELL buttons
                for (int row = 0; row < _rowCount; row++)
                {
                    for (int col = 0; col < _columnCount; col++)
                    {
                        index++;
                        string content = null;
                        string tooltip = "WELL " + SampleNames.WellName(_sampleType, index - 1);
                        string buttonName = _buttonName + Convert.ToString(index);

                        _collection.Add(buttonObj.AddControl(Color.FromRgb(255, 255, 255), buttonName, content, tooltip, BtnWell_Click));
                    }
                }
            }            
        }

        void CreateLeftLabelCollection()
        {
            SampleCollectionBase labelObj = _factory.GetControl((int)ControlType.LABEL);           
            _collectionLeftLabel = labelObj.CreateObservableCollection();

            int index = 0;

            //creating the plate labels
            //Checking for 1536 well plate
            if ((_columnCount == 48) && (_rowCount == 32))
            {
                string content = "";
                string tooltip = "";
                string labelName = "";                       

                //creating left labels

                for (index = 0; index < 8; index++)
                {
                    if (index < 26)
                    {
                        char p = Convert.ToChar((Convert.ToInt32('A') + index));
                        content = p.ToString();
                    }

                    tooltip = "";
                    labelName = _labelLeftName + Convert.ToString(index);
                    _collectionLeftLabel.Add(labelObj.AddControl(Color.FromRgb(255, 255, 255), labelName, content, tooltip,null));

                    if (index < 7)
                    {
                        content = "_";
                        tooltip = "";
                        labelName = "lblLeftSeperator" + Convert.ToString(index);
                        _collectionLeftLabel.Add(labelObj.AddControl(Color.FromRgb(255, 255, 255), labelName, content, tooltip,null));
                    }
                }
            }
            else
            {
                //creating left labels
                int alpha = 0;
                for (index = 0; index < _rowCount; index++)
                {
                    //label content and tooltip generation to support all the existing WELL types
                    string tooltip = null;
                    string content = null;

                    if (index < 26)
                    {
                        char p = Convert.ToChar((Convert.ToInt32('A') + index));
                        content = p.ToString();
                        tooltip = "ROW " + p;
                    }
                    else if (index >= 26 && index < 52)
                    {
                        string p = "A" + Convert.ToChar((Convert.ToInt32('a') + alpha));
                        alpha++;
                        content = p;
                        tooltip = "ROW " + p;
                    }
                    else if (index >= 52 && index < 78)
                    {
                        string p = "B" + Convert.ToChar((Convert.ToInt32('a') + alpha));
                        alpha++;
                        content = p;
                        tooltip = "ROW " + p;
                    }
                    else if (index >= 78 && index < 104)
                    {
                        string p = "C" + Convert.ToChar((Convert.ToInt32('a') + alpha));
                        alpha++;
                        content = p;
                        tooltip = "ROW " + p;
                    }

                    string lableName = _labelLeftName + Convert.ToString(index);
                    _collectionLeftLabel.Add(labelObj.AddControl(Color.FromRgb(255, 255, 255), lableName, content, tooltip,null));
                }
            }
        }

        void CreateTopLabelCollection()
        {
            SampleCollectionBase labelObj = _factory.GetControl((int)ControlType.LABEL);
            _collectionTopLabel = labelObj.CreateObservableCollection();
            
            int index = 0;

            //creating the plate labels
            //Checking for 1536 well plate
            if ((_columnCount == 48) && (_rowCount == 32))
            {
                string content = " ";
                string tooltip = "";
                string labelName = "";
                _collectionTopLabel.Add(labelObj.AddControl(Color.FromRgb(255, 255, 255), labelName, content, tooltip,null));

                //creating top labels
                for (index = 0; index < _columnCount; index += 4)
                {
                    content = (index + 4).ToString();//stepping by 4 for the names
                    tooltip = "";
                    labelName = _labelName + Convert.ToString(index);
                    _collectionTopLabel.Add(labelObj.AddControl(Color.FromRgb(255, 255, 255), labelName, content, tooltip,null));

                    //generating label seperators
                    //don't add the last sperator to the collection
                    if (index < 44)
                    {
                        content = "|";
                        tooltip = "";
                        labelName = "lblTopSeperator" + Convert.ToString(index);
                        _collectionTopLabel.Add(labelObj.AddControl(Color.FromRgb(255, 255, 255), labelName, content, tooltip,null));
                    }
                }                
            }
            else
            {
                //creating top labels
                for (index = 0; index < _columnCount; index++)
                {
                    string content = null;
                    if (_columnCount != 48)
                    {
                        content = (index + 1).ToString();
                    }

                    string tooltip = "COLUMN " + (index + 1);
                    string lableName = _labelName + Convert.ToString(index);
                    _collectionTopLabel.Add(labelObj.AddControl(Color.FromRgb(255, 255, 255), lableName, content, tooltip,null));
                }                
            }
        }

        private void Grid_MouseDown(object sender, MouseButtonEventArgs e)
        {
            if (IsEnableDragCanvas)
            {
                // Capture and track the mouse.
                mouseDown = true;
                mouseDownPos = e.GetPosition(theGrid);
                theGrid.CaptureMouse();

                // Initial placement of the drag selection box.
                Canvas.SetLeft(selectionBox, mouseDownPos.X);
                Canvas.SetTop(selectionBox, mouseDownPos.Y);
                selectionBox.Width = 0;
                selectionBox.Height = 0;

                // Make the drag selection box visible.
                selectionBox.Visibility = Visibility.Visible;

                e.Handled = true;
            }
        }

        private void Grid_MouseMove(object sender, MouseEventArgs e)
        {
            if (IsEnableDragCanvas)
            {

                if (mouseDown)
                {
                    // When the mouse is held down, reposition the drag selection box.

                    Point mousePos = e.GetPosition(theGrid);

                    if (mouseDownPos.X < mousePos.X)
                    {
                        Canvas.SetLeft(selectionBox, mouseDownPos.X);
                        selectionBox.Width = mousePos.X - mouseDownPos.X;
                    }
                    else
                    {
                        Canvas.SetLeft(selectionBox, mousePos.X);
                        selectionBox.Width = mouseDownPos.X - mousePos.X;
                    }

                    if (mouseDownPos.Y < mousePos.Y)
                    {
                        Canvas.SetTop(selectionBox, mouseDownPos.Y);
                        selectionBox.Height = mousePos.Y - mouseDownPos.Y;
                    }
                    else
                    {
                        Canvas.SetTop(selectionBox, mousePos.Y);
                        selectionBox.Height = mouseDownPos.Y - mousePos.Y;
                    }
                }
            }
        }

        private void Grid_MouseUp(object sender, MouseButtonEventArgs e)
        {
            if (IsEnableDragCanvas)
            {

                // Release the mouse capture and stop tracking it.
                mouseDown = false;
                theGrid.ReleaseMouseCapture();

                // Make the drag selection box invisible
                selectionBox.Visibility = Visibility.Hidden;

                Point mouseUpPos = e.GetPosition(theGrid);

                Rect rubberBand = new Rect(mouseDownPos, mouseUpPos);

                _wellList = new List<int>();

                if (PlateControl == ControlType.TEXTBOX)
                {
                    foreach (Label item in this.items.Items)
                    {
                        Rect itemRect = VisualTreeHelper.GetDescendantBounds(item);
                        Rect itemBounds = item.TransformToAncestor(elementCanvas).TransformBounds(itemRect);

                        if (rubberBand.IntersectsWith(itemBounds))
                        {
                            _wellList.Add(int.Parse(item.Name.Substring(7)));

                            if (item.Background != Brushes.LightBlue)
                            {
                                item.Background = Brushes.LightBlue;
                            }
                        }
                        else
                        {
                            if (item.Background != Brushes.White)
                            {
                                item.Background = Brushes.White;
                            }
                        }
                    }
                }
                else
                {
                    foreach (Button item in this.items.Items)
                    {
                        Rect itemRect = VisualTreeHelper.GetDescendantBounds(item);
                        Rect itemBounds = item.TransformToAncestor(elementCanvas).TransformBounds(itemRect);

                        if (rubberBand.IntersectsWith(itemBounds))
                        {
                            _wellList.Add(int.Parse(item.Name.Substring(6)));

                            if (item.Background != Brushes.LightBlue)
                            {
                                item.Background = Brushes.LightBlue;
                            }
                        }
                        else
                        {
                            if (item.Background != Brushes.White)
                            {
                                item.Background = Brushes.White;
                            }
                        }
                    }
                }


                _wellList.Sort();
                int wellCount = _wellList.Count;
                int j = 0;

                //checking if the entire plate is selected
                if (wellCount == (_rowCount * _columnCount))
                {
                    _numberOfCols = _columnCount;
                    _numberOfRows = _rowCount;
                }
                else
                {
                    //calculating the number of rows and cols
                    for (int i = 0; i < (wellCount - 1); i++)
                    {
                        if (_wellList[i] == (_wellList[i + 1] - 1))
                        {
                            j++;

                            //check for entire is col is selected
                            if (j == (_columnCount - 1))
                            {
                                break;
                            }
                            continue;
                        }
                        else
                        {
                            break;
                        }
                    }

                    _numberOfCols = j + 1;
                    _numberOfRows = wellCount / _numberOfCols;
                }

                //calculating the start row and column
                int k = 0;
                int columnIncrementer = _columnCount;

                for (int i = 0; i < (_rowCount - 1); i++)
                {
                    if (_wellList[0] > columnIncrementer)
                    {
                        k++;
                        columnIncrementer = _columnCount * (i + 2);
                    }
                    else
                    {
                        break;
                    }
                }

                _startRow = k + 1;
                _startColumn = _wellList[0] - _columnCount * (_startRow - 1);
            }
        }

        private void OnPropertyChanged(string propertyName)
        {
            if (PropertyChanged != null)
            {
                this.PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }

        #endregion Methods
    }
}