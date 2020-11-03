namespace CaptureSetupDll.View
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Linq;
    using System.Runtime.InteropServices;
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
    using System.Windows.Threading;

    using CaptureSetupDll.ViewModel;

    using ThorLogging;

    using ThorSharedTypes;

    /// <summary>
    /// Interaction logic for XYControlView.xaml
    /// </summary>
    public partial class XYControlView : UserControl
    {
        #region Constructors

        public XYControlView()
        {
            InitializeComponent();

            this.Loaded += new RoutedEventHandler(XYControlView_Loaded);

            this.Unloaded += new RoutedEventHandler(XYControlView_Unloaded);

            //setting the default Stepsize value
            txtXStepSize.Text = ".0100";
            txtYStepSize.Text = ".0100";
        }

        #endregion Constructors

        #region Events

        //public event Action<double, double> XYStepSizeUpdate;

        #endregion Events

        #region Properties

        public double PrevXPosition
        {
            get; set;
        }

        public double PrevYPosition
        {
            get; set;
        }

        #endregion Properties

        #region Methods

        private void cbSelectedSampleType_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            RefreshSampleView(((ComboBox)sender).SelectedIndex);
        }

        private void Click_GoX(object sender, RoutedEventArgs e)
        {
            try
            {
                ((LiveImageViewModel)this.DataContext).XPosition = Convert.ToDouble(txtXGo.Text);
            }
            catch (FormatException ex)
            {
                string str = ex.Message;
            }
        }

        private void Click_GoY(object sender, RoutedEventArgs e)
        {
            try
            {
                ((LiveImageViewModel)this.DataContext).YPosition = Convert.ToDouble(txtYGo.Text);
            }
            catch (FormatException ex)
            {
                string str = ex.Message;
            }
        }

        private void rbCoarse_Click(object sender, RoutedEventArgs e)
        {
            double stepSize = ((LiveImageViewModel)this.DataContext).XStepSize;

            ((LiveImageViewModel)this.DataContext).XStepSize = stepSize * 10.0;

            stepSize = ((LiveImageViewModel)this.DataContext).YStepSize;

            ((LiveImageViewModel)this.DataContext).YStepSize = stepSize * 10.0;
        }

        private void rbFine_Click(object sender, RoutedEventArgs e)
        {
            const double MIN_STEP_SIZE = .0001;

            double stepSize = ((LiveImageViewModel)this.DataContext).XStepSize;

            if ((stepSize / 10.0) > MIN_STEP_SIZE)
            {
                ((LiveImageViewModel)this.DataContext).XStepSize = stepSize / 10.0;
            }
            stepSize = ((LiveImageViewModel)this.DataContext).YStepSize;

            if ((stepSize / 10.0) > MIN_STEP_SIZE)
            {
                ((LiveImageViewModel)this.DataContext).YStepSize = stepSize / 10.0;
            }
        }

        void RefreshSampleView(int index)
        {
            //unregister any previous click handlers
            sampleView.Item_Clicked -= new Action<int, int>(sampleView_Item_Clicked);

            sampleView.StartRow = 0;
            sampleView.StartColumn = 0;
            sampleView.NumberOfRows = 1;
            sampleView.NumberOfColumns = 1;
            sampleView.SampleType = index;
            sampleView.PlateControl = SampleRegionSelection.SampleRegionSelectionView.ControlType.BUTTON;
            sampleView.PlateHeight = 237;
            sampleView.PlateWidth = 345;
            sampleView.IsEnableDragCanvas = false;
            sampleView.Item_Clicked += new Action<int, int>(sampleView_Item_Clicked);

            SampleType st = (SampleType)index;

            LiveImageViewModel vm = (LiveImageViewModel)this.DataContext;

            vm.WellOffsetXMM = -1 * SampleDimensions.WellOffsetMM(st);
            vm.WellOffsetYMM = SampleDimensions.WellOffsetMM(st);
        }

        void sampleView_Item_Clicked(int row, int col)
        {
            LiveImageViewModel vm = ((LiveImageViewModel)this.DataContext);
            vm.StartRow = row;
            vm.StartColumn = col;
            vm.SelectedWellRow = row-1;
            vm.SelectedWellColumn = col-1;
            vm.MoveToWellSite(vm.SelectedWellRow, vm.SelectedWellColumn, vm.SelectedSubRow, vm.SelectedSubColumn,vm.TransOffsetXMM, vm.TransOffsetYMM);
        }

        private void XSlider_DragCompleted(object sender, System.Windows.Controls.Primitives.DragCompletedEventArgs e)
        {
            Decimal dec = new Decimal(((Slider)e.Source).Value);

            positionX.Content = Decimal.Round(dec, 4).ToString();
            BindingExpression be = positionX.GetBindingExpression(Label.ContentProperty);
            be.UpdateSource();

            ((LiveImageViewModel)this.DataContext).EnableDeviceReading = true;
        }

        private void xSlider_DragStarted(object sender, System.Windows.Controls.Primitives.DragStartedEventArgs e)
        {
            ((LiveImageViewModel)this.DataContext).EnableDeviceReading = false;
        }

        private void XSlider_MouseWheel(object sender, MouseWheelEventArgs e)
        {
            PrevXPosition = double.Parse(positionX.Content.ToString());

            double newVal = ((Slider)e.Source).Value;

            if (e.Delta > 0)
            {
                newVal += ((LiveImageViewModel)this.DataContext).XStepSize;
            }
            else if (e.Delta < 0)
            {
                newVal -= ((LiveImageViewModel)this.DataContext).XStepSize;
            }

            positionX.Content = newVal.ToString();
            BindingExpression be = positionX.GetBindingExpression(Label.ContentProperty);
            be.UpdateSource();

            e.Handled = true;
        }

        void XYControlView_Loaded(object sender, RoutedEventArgs e)
        {
            if (System.ComponentModel.DesignerProperties.GetIsInDesignMode(this) == true)
                return;

            cbSelectedSampleType.Items.Clear();

            foreach (SampleType st in Enum.GetValues(typeof(SampleType)))
            {
                cbSelectedSampleType.Items.Add(SampleNames.GetName(st));
            }

            LiveImageViewModel vm = (LiveImageViewModel)this.DataContext;

            cbSelectedSampleType.SelectedIndex = vm.SelectedSampleType;
            RefreshSampleView(vm.SelectedSampleType);

            //data validation on viewmodel start row and column
            if (vm.StartRow > SampleDimensions.Rows((SampleType)vm.SelectedSampleType))
            {
                ((LiveImageViewModel)this.DataContext).StartRow = 1;
            }
            if (vm.StartColumn > SampleDimensions.Columns((SampleType)vm.SelectedSampleType))
            {
                ((LiveImageViewModel)this.DataContext).StartColumn = 1;
            }

            //sampleView.SendButtonClick(vm.StartRow, vm.StartColumn);
        }

        void XYControlView_Unloaded(object sender, RoutedEventArgs e)
        {
            //unregister any previous click handlers
            sampleView.Item_Clicked -= new Action<int, int>(sampleView_Item_Clicked);
        }

        private void YSlider_DragCompleted(object sender, System.Windows.Controls.Primitives.DragCompletedEventArgs e)
        {
            PrevYPosition = double.Parse(positionY.Content.ToString());

            Decimal dec = new Decimal(((Slider)e.Source).Value);

            positionY.Content = Decimal.Round(dec, 4).ToString();
            BindingExpression be = positionY.GetBindingExpression(Label.ContentProperty);
            be.UpdateSource();

            ((LiveImageViewModel)this.DataContext).EnableDeviceReading = true;
        }

        private void ySlider_DragStarted(object sender, System.Windows.Controls.Primitives.DragStartedEventArgs e)
        {
            ((LiveImageViewModel)this.DataContext).EnableDeviceReading = false;
        }

        private void YSlider_MouseWheel(object sender, MouseWheelEventArgs e)
        {
            PrevYPosition = double.Parse(positionY.Content.ToString());

            double newVal = ((Slider)e.Source).Value;

            if (e.Delta > 0)
            {
                newVal += ((LiveImageViewModel)this.DataContext).YStepSize;
            }
            else if (e.Delta < 0)
            {
                newVal -= ((LiveImageViewModel)this.DataContext).YStepSize;
            }

            positionY.Content = newVal.ToString();
            BindingExpression be = positionY.GetBindingExpression(Label.ContentProperty);
            be.UpdateSource();

            e.Handled = true;
        }

        #endregion Methods
    }
}