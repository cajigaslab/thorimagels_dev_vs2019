namespace XYTileControl
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Shapes;
    using System.Xml;

    using ThorSharedTypes;

    /// <summary>
    /// Interaction logic for SampleCarrierTemplate.xaml
    /// </summary>
    public partial class SampleCarrierTemplate : Window, INotifyPropertyChanged
    {
        #region Fields

        public double _carrierHeight = 0.0;
        public string _carrierName = String.Empty;
        public double _carrierWidth = 0.0;
        public double _wellCenterToCenterDistanceX = 0.0;
        public double _wellCenterToCenterDistanceY = 0.0;
        public int _wellColumn = 0;
        public double _wellDiameter = 0.0;
        public double _wellOffsetX = 0.0;
        public double _wellOffsetY = 0.0;
        public int _wellRow = 0;

        private const int CANVAS_HEIGHT = 600;
        private const int CANVAS_WIDTH = 1000;
        private const int RowAndColumnMaxNum = 100;
        private const double WellSizeMaxNum = 200;

        int _carrierTypeIndex = 0;
        private bool _isPetriDishChosen = false;
        private bool _isSlideChosen = false;
        bool _updateCarrierInCanvas = false;
        double _wellHeight = 0;
        int _wellShapeIndex = 0;
        double _wellWidth = 0;
        private XYTileDisplay _xy;

        #endregion Fields

        #region Constructors

        public SampleCarrierTemplate()
        {
            InitializeComponent();
            this.Loaded += SampleCarrierTemplate_Loaded;
            this.Unloaded += SampleCarrierTemplate_Unloaded;
        }

        #endregion Constructors

        #region Events

        /// <summary>
        /// Raised when a property on this object has a new value.
        /// </summary>
        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public double CarrierHeight
        {
            get { return _carrierHeight; }
            set
            {
                if (_carrierHeight == value || value < 0)
                {
                    return;
                }
                _carrierHeight = value;
                OnPropertyChanged("CarrierHeight");
                _xy.SampleCarrierCollection[SampleCarrier.SelectedIndex].Height = _carrierHeight;
                UpdateCarrierDisplayedInCanvas();
            }
        }

        public string CarrierName
        {
            get { return _carrierName; }
            set
            {
                if (_carrierName == value)
                {
                    return;
                }
                _carrierName = value;
                OnPropertyChanged("CarrierName");
                _xy.SampleCarrierCollection[SampleCarrier.SelectedIndex].Name = value;
                ((SampleCarrier.Items[SampleCarrier.SelectedIndex] as Border).Child as TextBlock).Text = value;
            }
        }

        public int CarrierTypeIndex
        {
            get { return _carrierTypeIndex; }
            set
            {
                if (_carrierTypeIndex == value)
                {
                    return;
                }
                _carrierTypeIndex = value;
                IsSlideChosen = (_carrierTypeIndex == 0) ? true : false;
                OnPropertyChanged("CarrierTypeIndex");
                _xy.SampleCarrierCollection[SampleCarrier.SelectedIndex].Type = (CarrierType)_carrierTypeIndex;
                if (_xy.SampleCarrierCollection[SampleCarrier.SelectedIndex].Template.Row > 1 ||
                    _xy.SampleCarrierCollection[SampleCarrier.SelectedIndex].Template.Col > 1)
                {
                    _xy.SetStepToWellSizeVisibility = Visibility.Visible;
                }
                else
                {
                    _xy.SetStepToWellSizeVisibility = Visibility.Collapsed;
                }

                if (CarrierType.Slide == (CarrierType)_carrierTypeIndex)
                {
                    //Visibility == hidden to keep the spacing
                    this.spExtendedPlateSettings.Visibility = Visibility.Hidden;
                }
                else
                {
                    this.spExtendedPlateSettings.Visibility = Visibility.Visible;
                }

                UpdateCarrierDisplayedInCanvas();
            }
        }

        public double CarrierWidth
        {
            get { return _carrierWidth; }
            set
            {
                if (_carrierWidth == value || value < 0)
                {
                    return;
                }
                _carrierWidth = value;
                OnPropertyChanged("CarrierWidth");
                _xy.SampleCarrierCollection[SampleCarrier.SelectedIndex].Width = _carrierWidth;
                UpdateCarrierDisplayedInCanvas();
            }
        }

        public bool IsPetriDishChosen
        {
            get
            {
                return _isPetriDishChosen;
            }
            set
            {
                if (value == true)
                {
                    ShapeComboBox.IsEnabled = false;
                }
                _isPetriDishChosen = value;
                OnPropertyChanged("IsPetriDishChosen");
            }
        }

        public bool IsSlideChosen
        {
            get
            {
                return _isSlideChosen;
            }
            set
            {
                _isSlideChosen = value;
                OnPropertyChanged("IsSlideChosen");
            }
        }

        public double WellCenterToCenterDistanceX
        {
            get { return _wellCenterToCenterDistanceX; }
            set
            {
                if (_wellCenterToCenterDistanceX == value || value < 0 || value > WellSizeMaxNum)
                {
                    if ((value < _xy.SampleCarrierCollection[SampleCarrier.SelectedIndex].Template.Diameter && WellShapeIndex == (int)WellShape.CircleWell) || (value < _xy.SampleCarrierCollection[SampleCarrier.SelectedIndex].Template.Width && WellShapeIndex == (int)WellShape.RectangleWell && _xy.SampleCarrierCollection[SampleCarrier.SelectedIndex].Type != CarrierType.Multislide) || (value < _xy.SampleCarrierCollection[SampleCarrier.SelectedIndex].Template.Height && WellShapeIndex == (int)WellShape.RectangleWell && _xy.SampleCarrierCollection[SampleCarrier.SelectedIndex].Type == CarrierType.Multislide))
                    {
                        return;
                    }

                }
                _wellCenterToCenterDistanceX = value;
                OnPropertyChanged("WellCenterToCenterDistanceX");

                _xy.SampleCarrierCollection[SampleCarrier.SelectedIndex].Template.CenterToCenterX = _wellCenterToCenterDistanceX;
                UpdateCarrierDisplayedInCanvas();
            }
        }

        public double WellCenterToCenterDistanceY
        {
            get { return _wellCenterToCenterDistanceY; }
            set
            {
                if ((value < _xy.SampleCarrierCollection[SampleCarrier.SelectedIndex].Template.Diameter && WellShapeIndex == (int)WellShape.CircleWell) || (value < _xy.SampleCarrierCollection[SampleCarrier.SelectedIndex].Template.Width && WellShapeIndex == (int)WellShape.RectangleWell && _xy.SampleCarrierCollection[SampleCarrier.SelectedIndex].Type == CarrierType.Multislide) || (value < _xy.SampleCarrierCollection[SampleCarrier.SelectedIndex].Template.Height && WellShapeIndex == (int)WellShape.RectangleWell && _xy.SampleCarrierCollection[SampleCarrier.SelectedIndex].Type != CarrierType.Multislide))
                {
                    return;
                }
                _wellCenterToCenterDistanceY = value;
                OnPropertyChanged("WellCenterToCenterDistanceY");

                _xy.SampleCarrierCollection[SampleCarrier.SelectedIndex].Template.CenterToCenterY = _wellCenterToCenterDistanceY;
                UpdateCarrierDisplayedInCanvas();
            }
        }

        public int WellColumn
        {
            get
            {
                return _wellColumn;
            }
            set
            {
                if (_wellColumn == value || value < 0 || value > RowAndColumnMaxNum)
                {
                    return;
                }
                _wellColumn = value;
                OnPropertyChanged("WellColumn");
                _xy.SampleCarrierCollection[SampleCarrier.SelectedIndex].Template.Col = _wellColumn;
                UpdateCarrierDisplayedInCanvas();
            }
        }

        public double WellDiameter
        {
            get
            {
                return _wellDiameter;
            }
            set
            {
                if (_wellDiameter == value || value < 0 || value > WellSizeMaxNum || (IsPetriDishChosen == false && value > _xy.SampleCarrierCollection[SampleCarrier.SelectedIndex].Template.CenterToCenterX))
                {
                    return;
                }
                _wellDiameter = value;
                OnPropertyChanged("WellDiameter");
                _xy.SampleCarrierCollection[SampleCarrier.SelectedIndex].Template.Diameter = _wellDiameter;
                UpdateCarrierDisplayedInCanvas();
            }
        }

        public double WellHeight
        {
            get { return _wellHeight; }
            set
            {
                if (_wellHeight == value || value < 0)
                {
                    return;
                }
                _wellHeight = value;
                OnPropertyChanged("WellHeight");
                _xy.SampleCarrierCollection[SampleCarrier.SelectedIndex].Template.Height = _wellHeight;
                UpdateCarrierDisplayedInCanvas();
            }
        }

        public double WellOffsetX
        {
            get { return _wellOffsetX; }
            set
            {
                if (_wellOffsetX == value || value < 0 || value > WellSizeMaxNum)
                {
                    return;
                }
                _wellOffsetX = value;
                OnPropertyChanged("WellOffsetX");
                _xy.SampleCarrierCollection[SampleCarrier.SelectedIndex].Template.TopLeftCenterOffsetX = _wellOffsetX;
                UpdateCarrierDisplayedInCanvas();
            }
        }

        public double WellOffsetY
        {
            get { return _wellOffsetY; }
            set
            {
                if (_wellOffsetY == value || value < 0 || value > WellSizeMaxNum)
                {
                    return;
                }
                _wellOffsetY = value;
                OnPropertyChanged("WellOffsetY");
                _xy.SampleCarrierCollection[SampleCarrier.SelectedIndex].Template.TopLeftCenterOffsetY = _wellOffsetY;
                UpdateCarrierDisplayedInCanvas();
            }
        }

        public int WellRow
        {
            get
            {
                return _wellRow;
            }
            set
            {
                if (_wellRow == value || value < 0 || value > RowAndColumnMaxNum)
                {
                    return;
                }
                _wellRow = value;
                OnPropertyChanged("WellRow");
                _xy.SampleCarrierCollection[SampleCarrier.SelectedIndex].Template.Row = _wellRow;
                UpdateCarrierDisplayedInCanvas();
            }
        }

        public int WellShapeIndex
        {
            get { return _wellShapeIndex; }
            set
            {
                if (_wellShapeIndex == value)
                {
                    return;
                }
                _wellShapeIndex = value;
                OnPropertyChanged("WellShapeIndex");
                _xy.SampleCarrierCollection[SampleCarrier.SelectedIndex].Template.Shape = (WellShape)_wellShapeIndex;
                UpdateCarrierDisplayedInCanvas();
            }
        }

        public double WellWidth
        {
            get { return _wellWidth; }
            set
            {
                if (_wellWidth == value || value < 0)
                {
                    return;
                }
                _wellWidth = value;
                OnPropertyChanged("WellWidth");
                _xy.SampleCarrierCollection[SampleCarrier.SelectedIndex].Template.Width = _wellWidth;
                UpdateCarrierDisplayedInCanvas();
            }
        }

        #endregion Properties

        #region Methods

        public void AddNewSampleCarrier()
        {
            XYTileDisplay xy = (XYTileDisplay)this.DataContext;
            xy.SampleCarrierCollection.Add(new Carrier() { Name = "NewSlide", Type = CarrierType.Slide, Width = 0, Height = 0, Template = new WellPlateTemplate { CenterToCenterX = 0, CenterToCenterY = 0, Col = 0, Row = 0, TopLeftCenterOffsetX = 0, TopLeftCenterOffsetY = 0, Diameter = 0, Width = 0, Shape = WellShape.CircleWell, Height = 0 } });
        }

        public void DeleteSampleCarrier()
        {
            XYTileDisplay xy = (XYTileDisplay)this.DataContext;
            xy.SampleCarrierCollection.RemoveAt(SampleCarrier.SelectedIndex);
        }

        /// <summary>
        /// Warns the developer if this object does not have a public property with
        /// the specified name. This method does not exist in a Release build.
        /// </summary>
        public void VerifyPropertyName(string propertyName)
        {
            // verify that the property name matches a real,
            // public, instance property on this object.
            if (TypeDescriptor.GetProperties(this)[propertyName] == null)
            {
                Debug.Fail("Invalid property name: " + propertyName);
            }
        }

        /// <summary>
        /// Raises this object's PropertyChanged event.
        /// </summary>
        /// <param name="propertyName">The name of the property that has a new value.</param>
        protected virtual void OnPropertyChanged(string propertyName)
        {
            this.VerifyPropertyName(propertyName);

            if (this.PropertyChanged != null)
            {
                this.PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }

        private void Add_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            XYTileDisplay xy = (XYTileDisplay)this.DataContext;
            AddNewSampleCarrier();
            UpdateSampleCarrierListViewItem();
            SampleCarrier.SelectedIndex = SampleCarrier.Items.Count - 1;
        }

        private void Cancel_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            this.Close();
        }

        private void Delete_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            XYTileDisplay xy = (XYTileDisplay)this.DataContext;
            DeleteSampleCarrier();
            int index = SampleCarrier.SelectedIndex;
            UpdateSampleCarrierListViewItem();
            if (SampleCarrier.Items.Count > 0)
            {
                if (index < SampleCarrier.Items.Count)
                {
                    SampleCarrier.SelectedIndex = index;
                }
                else
                {
                    SampleCarrier.SelectedIndex = --index;
                }
            }
        }

        private void Exit_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            if (SampleCarrier.SelectedIndex < 0)
            {
                MessageBox.Show("Please add sample carrier Template, no carrier is chosen", "Sample Carrier Container Empty Alert", MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }
            _xy.SelectedCarrierIndex = SampleCarrier.SelectedIndex;
            this.Close();
        }

        private void SampleCarrierSelectionColorChanged()
        {
            for (int i = 0; i < SampleCarrier.Items.Count; i++)
            {
                if (i == SampleCarrier.SelectedIndex)
                {
                    (SampleCarrier.Items[i] as Border).Style = (Style)FindResource("BorderStyleSelected");
                }
                else
                {
                    (SampleCarrier.Items[i] as Border).Style = (Style)FindResource("BorderStyle");
                }
            }
        }

        void SampleCarrierTemplate_KeyDown(object sender, KeyEventArgs e)
        {
            //filter for the enter or return key
            if ((e.Key == Key.Enter) || (e.Key == Key.Return))
            {
                e.Handled = true;
                TraversalRequest trNext = new TraversalRequest(FocusNavigationDirection.Next);

                UIElement keyboardFocus = (UIElement)Keyboard.FocusedElement;

                //move the focus to the next element
                if (keyboardFocus != null)
                {
                    if (keyboardFocus.GetType() == typeof(TextBox))
                    {
                        keyboardFocus.MoveFocus(trNext);
                    }
                }
            }
        }

        void SampleCarrierTemplate_Loaded(object sender, RoutedEventArgs e)
        {
            if (_xy == null)
            {
                _xy = (XYTileDisplay)this.DataContext;
            }
            UpdateSampleCarrierListViewItem();
            if (SampleCarrier.Items.Count > 0)
            {
                SampleCarrier.SelectedIndex = _xy.SelectedCarrierIndex;
            }
            this.KeyDown -= SampleCarrierTemplate_KeyDown;
            this.KeyDown += SampleCarrierTemplate_KeyDown;
        }

        void SampleCarrierTemplate_Unloaded(object sender, RoutedEventArgs e)
        {
            if (SampleCarrier.SelectedIndex >= 0)
            {
                SaveCarrierSettings(_xy.ApplicationSettings);
            }
            this.KeyDown -= SampleCarrierTemplate_KeyDown;
            _xy = null;
        }

        private void SampleCarrier_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (SampleCarrier.SelectedIndex < 0)
            {
                return;
            }
            SampleCarrierSelectionColorChanged();
            UpdataCarrierTemplateDimensions();
            UpdateCarrierDisplayedInCanvas();
        }

        private void SaveCarrierSettings(string filePath)
        {
            XmlDocument appSettings = new XmlDocument();
            appSettings.Load(filePath);
            XmlNode node = appSettings.SelectSingleNode("/ApplicationSettings/SampleCarrier");
            var root = appSettings.SelectSingleNode("/ApplicationSettings");
            if (node != null)
            {
                root.RemoveChild(node);
            }
            node = appSettings.CreateElement("SampleCarrier");
            root.AppendChild(node);
            appSettings.Save(filePath);
            node = appSettings.SelectSingleNode("/ApplicationSettings/SampleCarrier");
            for (int i = 0; i < _xy.SampleCarrierCollection.Count; i++)
            {
                Carrier carrier = _xy.SampleCarrierCollection[i];
                XmlElement newCarrier = appSettings.CreateElement(string.Empty, "Carrier".ToString(), string.Empty);
                newCarrier.SetAttribute("Name", carrier.Name);
                newCarrier.SetAttribute("Type", (carrier.Type).ToString());
                newCarrier.SetAttribute("Height", carrier.Height.ToString());
                newCarrier.SetAttribute("Width", carrier.Width.ToString());

                newCarrier.SetAttribute("WellShape", (carrier.Template.Shape).ToString());
                newCarrier.SetAttribute("WellWidth", carrier.Template.Width.ToString());
                newCarrier.SetAttribute("WellHeight", carrier.Template.Height.ToString());
                newCarrier.SetAttribute("Row", carrier.Template.Row.ToString());
                newCarrier.SetAttribute("Col", carrier.Template.Col.ToString());
                newCarrier.SetAttribute("Diameter", carrier.Template.Diameter.ToString());
                newCarrier.SetAttribute("CenterToCenterX", carrier.Template.CenterToCenterX.ToString());
                newCarrier.SetAttribute("CenterToCenterY", carrier.Template.CenterToCenterY.ToString());
                newCarrier.SetAttribute("TopLeftCenterOffsetX", carrier.Template.TopLeftCenterOffsetX.ToString());
                newCarrier.SetAttribute("TopLeftCenterOffsetY", carrier.Template.TopLeftCenterOffsetY.ToString());
                node.AppendChild(newCarrier);
            }
            appSettings.Save(filePath);
        }

        private void UpdataCarrierTemplateDimensions()
        {
            if (SampleCarrier.SelectedIndex >= 0)
            {
                _updateCarrierInCanvas = false;
                Carrier carrier = _xy.SampleCarrierCollection[SampleCarrier.SelectedIndex];
                CarrierName = carrier.Name;
                CarrierWidth = carrier.Width;
                CarrierHeight = carrier.Height;
                CarrierTypeIndex = (int)carrier.Type;
                IsSlideChosen = (_carrierTypeIndex == (int)CarrierType.Slide) ? true : false;
                IsPetriDishChosen = (_carrierTypeIndex == (int)CarrierType.PetriDish) ? true : false;
                WellRow = carrier.Template.Row;
                WellColumn = carrier.Template.Col;
                WellDiameter = carrier.Template.Diameter;
                WellCenterToCenterDistanceX = carrier.Template.CenterToCenterX;
                WellCenterToCenterDistanceY = carrier.Template.CenterToCenterY;
                WellOffsetX = carrier.Template.TopLeftCenterOffsetX;
                WellOffsetY = carrier.Template.TopLeftCenterOffsetY;
                WellShapeIndex = (int)carrier.Template.Shape;
                WellHeight = carrier.Template.Height;
                WellWidth = carrier.Template.Width;
                _updateCarrierInCanvas = true;
            }
        }

        void UpdateCarrierDisplayedInCanvas()
        {
            if (_updateCarrierInCanvas == false)
            {
                return;
            }
            //reset
            CarrierCanvas.Children.Clear();
            //draw the panel
            if (_xy.SampleCarrierCollection.Count <= 0 || SampleCarrier.SelectedIndex < 0)
            {
                return;
            }
            Carrier carrier = _xy.SampleCarrierCollection[SampleCarrier.SelectedIndex];

            Rectangle rect = new Rectangle
            {
                Fill = Brushes.LightBlue,
                StrokeThickness = 2,
                Width = carrier.Width * 4,
                Height = carrier.Height * 4
            };
            Canvas.SetLeft(rect, CANVAS_WIDTH / 2 - carrier.Width * 2);
            Canvas.SetTop(rect, CANVAS_HEIGHT / 2 - carrier.Height * 2);
            CarrierCanvas.Children.Add(rect);

            if (carrier.Type != CarrierType.Slide) // draw wells
            {
                for (int i = 0; i < carrier.Template.Row; i++)
                {
                    for (int j = 0; j < carrier.Template.Col; j++)
                    {
                        if (WellShapeIndex == (int)WellShape.CircleWell)
                        {
                            Ellipse well = new Ellipse()
                            {
                                StrokeThickness = 2,
                                Stroke = Brushes.Gray,
                                Fill = Brushes.DarkGray,
                                Width = carrier.Template.Diameter * 4,
                                Height = carrier.Template.Diameter * 4,
                            };
                            Canvas.SetLeft(well, CANVAS_WIDTH / 2 - carrier.Width * 2 + (carrier.Template.TopLeftCenterOffsetX - carrier.Template.Diameter / 2) * 4 + j * carrier.Template.CenterToCenterX * 4);
                            Canvas.SetTop(well, CANVAS_HEIGHT / 2 - carrier.Height * 2 + (carrier.Template.TopLeftCenterOffsetY - carrier.Template.Diameter / 2) * 4 + i * carrier.Template.CenterToCenterY * 4);
                            CarrierCanvas.Children.Add(well);
                        }
                        else if (WellShapeIndex == (int)WellShape.RectangleWell)
                        {
                            double width = 0;
                            double height = 0;
                            if (carrier.Type != CarrierType.Multislide)
                            {
                                width = carrier.Template.Width;
                                height = carrier.Template.Height;
                            }
                            else
                            {
                                width = carrier.Template.Height;
                                height = carrier.Template.Width;
                            }
                            Rectangle well = new Rectangle
                            {
                                Stroke = Brushes.Gray,
                                Fill = Brushes.DarkGray,
                                StrokeThickness = 2,
                                Width = width * 4,
                                Height = height * 4
                            };
                            Canvas.SetLeft(well, CANVAS_WIDTH / 2 - carrier.Width * 2 + (carrier.Template.TopLeftCenterOffsetX - width / 2) * 4 + j * carrier.Template.CenterToCenterX * 4);
                            Canvas.SetTop(well, CANVAS_HEIGHT / 2 - carrier.Height * 2 + (carrier.Template.TopLeftCenterOffsetY - height / 2) * 4 + i * carrier.Template.CenterToCenterY * 4);
                            CarrierCanvas.Children.Add(well);
                        }
                    }
                }
            }
        }

        private void UpdateSampleCarrierListViewItem()
        {
            SampleCarrier.Items.Clear();
            for (int i = 0; i < _xy.SampleCarrierCollection.Count; i++)
            {
                Border carrier = new Border()
                {
                    CornerRadius = new CornerRadius(3),
                    Style = (Style)FindResource("BorderStyle"),
                    Width = 190,
                    Child = new TextBlock()
                    {
                        Text = (_xy.SampleCarrierCollection[i] as Carrier).Name,
                        FontSize = 12,
                        Margin = new Thickness(3),
                        Foreground = Brushes.Ivory,
                    }
                };
                SampleCarrier.Items.Add(carrier);
            }
        }

        #endregion Methods
    }
}