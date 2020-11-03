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
using ThorSharedTypes;

namespace SampleRegionSelectionTest
{
    /// <summary>
    /// Interaction logic for Window1.xaml
    /// </summary>
    public partial class Window1 : Window
    {
        private List<String> _liContent;
        private List<Color> _liColor;
        private SampleType _sampleType;
        private int _columnCount;
        private int _rowCount;

        public Window1()
        {  
            InitializeComponent();

            this.Loaded += new RoutedEventHandler(Window1_Loaded);

            _sampleType = SampleType.WELL96;

            _columnCount = SampleDimensions.Columns(_sampleType);
            _rowCount = SampleDimensions.Rows(_sampleType);

            sampleComboBox.Items.Add("6 well plate");
            sampleComboBox.Items.Add("24 well plate");
            sampleComboBox.Items.Add("96 well plate");
            sampleComboBox.Items.Add("384 well plate");
            sampleComboBox.Items.Add("1536 well plate");
            sampleComboBox.Items.Add("Slide");
        }

        void Window1_Loaded(object sender, RoutedEventArgs e)
        {
            BuildColor(_columnCount * _rowCount);

            BuildContent(_columnCount * _rowCount);

            BuildPlate(_sampleType);
        }

        void BuildColor(int totalWellCount)
        { 
            _liColor = new List<Color>();

            int jStep = 256 / totalWellCount;

            byte j=0;
            for (int i = 0; i < totalWellCount; i++)
            {
                _liColor.Add(Color.FromRgb(j, 0, 0));
                j+= (byte)jStep;
            }
        }

        void BuildContent(int totalWellCount)
        {
            _liContent = new List<String>();

            for (int i = 0; i < totalWellCount; i++)
            {
                _liContent.Add("Sample Text");
            }
        }

        void BuildPlate(SampleType sampleType)
        {
            mySample.PlateControl = SampleRegionSelection.SampleRegionSelectionView.ControlType.BUTTON;
            mySample.SampleType = (int)sampleType;            
            mySample.PlateHeight = 400;
            mySample.PlateWidth = 600;
            mySample.ColorList = _liColor;
            mySample.ContentList = _liContent;
            mySample.IsEnableDragCanvas = true;
            mySample.TitleVisibility = Visibility.Visible;
            mySample.Title = "Sample Text";            

            _sampleType = sampleType;
        }

        private void btnUpdate_Click(object sender, RoutedEventArgs e)
        {
            List<int> selectedWellList = mySample.WellList;

            if (selectedWellList != null && txtWellContent.Text != "")
            {
                foreach (var item in selectedWellList)
                {
                    _liContent[item - 1] = txtWellContent.Text;
                }

                BuildPlate(_sampleType);
            }           
        }

        private void sampleComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (sampleComboBox.SelectedIndex == -1)
            {
                return;
            }

            _columnCount = SampleDimensions.Columns((SampleType)(((ComboBox)e.Source).SelectedIndex));
            _rowCount = SampleDimensions.Rows((SampleType)(((ComboBox)e.Source).SelectedIndex));

            BuildColor(_columnCount * _rowCount);

            BuildContent(_columnCount * _rowCount);

            BuildPlate((SampleType)(((ComboBox)e.Source).SelectedIndex));
        }
    }
}
