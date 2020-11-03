namespace ROIStatsChart.View
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Collections.Specialized;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.IO;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Text.RegularExpressions;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Shapes;
    using System.Windows.Threading;
    using System.Xml;

    using Abt.Controls.SciChart;
    using Abt.Controls.SciChart.Model.DataSeries;
    using Abt.Controls.SciChart.Visuals;
    using Abt.Controls.SciChart.Visuals.RenderableSeries;

    using ROIStatsChart.Model;
    using ROIStatsChart.ViewModel;

    using ThorSharedTypes;

    /// <summary>
    /// Interaction logic for OperandChooser.xaml
    /// </summary>
    public partial class OperandChooser : Window
    {
        #region Fields

        private ChartViewModel _vm;

        #endregion Fields

        #region Constructors

        public OperandChooser(ChartViewModel vm)
        {
            InitializeComponent();
            this.DataContext = this;
            _vm = vm;
            PopulateComboBoxes();
        }

        #endregion Constructors

        #region Properties

        public string Constant
        {
            get;
            set;
        }

        public string Operand
        {
            get;
            set;
        }

        public ChartViewModel VM
        {
            get;
            set;
        }

        #endregion Properties

        #region Methods

        private void btnCancel_Click(object sender, RoutedEventArgs e)
        {
            this.DialogResult = false;
            this.Close();
        }

        private void btnOK_Click(object sender, RoutedEventArgs e)
        {
            if (true == DataSetRB.IsChecked)
            {
                Operand = FeatureComboBox.SelectedItem.ToString() + ChannelComboBox.SelectedItem.ToString() + ROIsComboBox.SelectedItem.ToString();
            }
            else
            {
                double constant = 0.0;
                if (true == double.TryParse(Constant, out constant))
                {
                    Operand = Constant;
                }
                else
                {
                    MessageBox.Show("Invalid constant input.");
                    return;
                }
            }
            this.DialogResult = true;

            this.Close();
        }

        private void PopulateComboBoxes()
        {
            if (_vm == null)
            {
                return;
            }
            for (int i = 0; i < _vm.FeatureCheckBoxList.Count; i++)
            {
                FeatureComboBox.Items.Add(_vm.FeatureCheckBoxList[i].Name);
            }
            if (0 < _vm.FeatureCheckBoxList.Count)
            {
                FeatureComboBox.SelectedIndex = 0;
            }
            for (int i = 0; i < _vm.ChannelCheckBoxList.Count; i++)
            {
                ChannelComboBox.Items.Add(_vm.ChannelCheckBoxList[i].Name.Replace("Chan", string.Empty));
            }
            if (0 < _vm.ChannelCheckBoxList.Count)
            {
                ChannelComboBox.SelectedIndex = 0;
            }
            for (int i = 0; i < _vm.ROICheckBoxList.Count; i++)
            {
                ROIsComboBox.Items.Add(_vm.ROICheckBoxList[i].Name.Replace("ROI", string.Empty));
            }
            if (0 < _vm.ROICheckBoxList.Count)
            {
                ROIsComboBox.SelectedIndex = 0;
            }
        }

        #endregion Methods
    }
}