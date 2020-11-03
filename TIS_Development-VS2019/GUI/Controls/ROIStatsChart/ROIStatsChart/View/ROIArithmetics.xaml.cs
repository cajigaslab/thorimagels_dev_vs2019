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

    using XMLHandle;

    /// <summary>
    /// Interaction logic for ROIArithmetics.xaml
    /// </summary>
    public partial class ROIArithmetics : Window
    {
        #region Fields

        private Visibility _addDeleteCancelVisibility = Visibility.Visible;
        private Dictionary<string, string[]> _arithmeticExpressions = new Dictionary<string, string[]>();
        private bool _editable = false;
        private CustomCollection<EquationBuilder> _equations;
        private ChartViewModel _vm;

        #endregion Fields

        #region Constructors

        public ROIArithmetics(ChartViewModel vm, Dictionary<string, string[]> arithmeticExpressions)
        {
            InitializeComponent();
            _arithmeticExpressions = arithmeticExpressions;
            _vm = vm;
            _equations = new CustomCollection<EquationBuilder>();
            this.DataContext = this;
            if (0 < arithmeticExpressions.Count)
            {
                for (int i = 0; i < arithmeticExpressions.Count; i++)
                {
                    string key = arithmeticExpressions.Keys.ElementAt(i);
                    string[] arithmeticExpression;
                    _arithmeticExpressions.TryGetValue(key, out arithmeticExpression);
                    EquationBuilder eq = new EquationBuilder(key, arithmeticExpression);
                    eq.VM = _vm;
                    _equations.Add(eq);
                }
            }
            else
            {
                EquationBuilder eq = new EquationBuilder(null, null);
                eq.VM = _vm;
                eq.VariableNumber = (_equations.Count + 1).ToString();
                _equations.Add(eq);
            }
        }

        #endregion Constructors

        #region Properties

        public Visibility AddDeleteCancelVisibility
        {
            get
            {
                return _addDeleteCancelVisibility;
            }
            set
            {
                _addDeleteCancelVisibility = value;
            }
        }

        public Dictionary<string, string[]> ArithmeticExpressions
        {
            get
            {
                return _arithmeticExpressions;
            }
            set
            {
                _arithmeticExpressions = value;
            }
        }

        public bool Editable
        {
            get
            {
                return _editable;
            }

            set
            {
                _editable = value;
                if (true == Editable)
                {
                    _addDeleteCancelVisibility = Visibility.Visible;
                }
                else
                {
                    _addDeleteCancelVisibility = Visibility.Collapsed;
                }
                for (int i = 0; i < _equations.Count; i++)
                {
                    _equations[i].AddVisibility = _addDeleteCancelVisibility;
                }
            }
        }

        public CustomCollection<EquationBuilder> Equations
        {
            get
            {
                return _equations;
            }
            set
            {
                _equations = value;
            }
        }

        public ChartViewModel VM
        {
            get;
            set;
        }

        #endregion Properties

        #region Methods

        private void Add_Click(object sender, RoutedEventArgs e)
        {
            EquationBuilder eq = new EquationBuilder(null, null);
            eq.VM = _vm;
            if (0 == _equations.Count)
            {
                eq.VariableNumber = "1";
            }
            else
            {
                string numberString = _equations[_equations.Count - 1].VariableNumber.Substring(0, _equations[_equations.Count - 1].VariableNumber.Length - 2);
                int number = 0;
                if (true == int.TryParse(numberString, out number))
                {
                    eq.VariableNumber = (number + 1).ToString();
                }
                else
                {
                    eq.VariableNumber = (_equations.Count + 1).ToString();
                }
            }
            Equations.Add(eq);
        }

        private void btnCancel_Click(object sender, RoutedEventArgs e)
        {
            this.DialogResult = false;
            this.Close();
        }

        private void btnOK_Click(object sender, RoutedEventArgs e)
        {
            List<string> names = new List<string>();
            for (int i = 0; i < _equations.Count; i++)
            {
                if (1 == _equations[i].EquationArray.Length)
                {
                    double constant = 0.0;
                    string variable = _equations[i].EquationArray[0];
                    if (false == double.TryParse(variable, out constant))
                    {
                        MessageBox.Show("Only constants can be used for single operand equations.");
                        return;
                    }
                }

                for (int j = 0; j < _equations[i].EquationArray.Length; j++)
                {
                    if (string.Empty == _equations[i].EquationArray[j])
                    {
                        MessageBox.Show("All fields must be defined.");
                        return;
                    }
                }
                if (string.Empty == _equations[i].VariableName)
                {
                    MessageBox.Show("All fields must be defined.");
                    return;
                }
                if (true == names.Contains(_equations[i].VariableName))
                {
                    MessageBox.Show("All Variables (left hand side of equals sign) must be different.");
                    return;
                }

                int n = 0;
                string first = _equations[i].VariableName[0].ToString();
                if (true == int.TryParse(first, out n) || first[0] == '_')
                {
                    MessageBox.Show("Variables must start with a letter and consist only of letters, numbers, and underscores");
                    return;
                }
                Regex rgx = new Regex("^[a-zA-Z0-9_]*$");
                if (false == rgx.IsMatch(_equations[i].VariableName))
                {
                    MessageBox.Show("Variables must start with a letter and consist only of letters, numbers, and underscores");
                    return;
                }
                names.Add(_equations[i].VariableName);
            }

            _arithmeticExpressions = new Dictionary<string, string[]>();
            for (int i = 0; i < _equations.Count; i++)
            {
                _arithmeticExpressions.Add(_equations[i].CompleteVariableName, _equations[i].EquationArray);
            }

            this.DialogResult = true;

            this.Close();
        }

        private void Delete_Click(object sender, RoutedEventArgs e)
        {
            int indx = lbArithmetics.SelectedIndex;
            if (indx != -1)
            {
                _equations.RemoveAt(indx);
            }
        }

        #endregion Methods
    }
}