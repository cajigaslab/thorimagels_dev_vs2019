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
    /// Interaction logic for Equation.xaml
    /// </summary>
    public partial class EquationBuilder : UserControl, INotifyPropertyChanged
    {
        #region Fields

        private static string OPS = "+-*/";

        private Visibility _addVisibility = Visibility.Visible;
        private CustomCollection<Control> _equation;
        private string _variableName = string.Empty;
        private string _variableNumber = string.Empty;

        #endregion Fields

        #region Constructors

        public EquationBuilder(string equationName, string[] equationArray)
        {
            InitializeComponent();
            this.DataContext = this;
            _equation = new CustomCollection<Control>();
            if (null != equationArray && null != equationName)
            {
                for (int i = 0; i < equationArray.Length; i++)
                {
                    switch (equationArray[i])
                    {
                        case "+":
                        case "-":
                        case "*":
                        case "/":
                            {
                                InsertOperatorCombobox(i);
                                (_equation[i] as ComboBox).SelectedIndex = OPS.IndexOf(equationArray[i]);
                                break;
                            }
                        case "(":
                            {
                                InsertLeftParenthesis(i);
                                break;
                            }
                        case ")":
                            {
                                InsertRightParenthesis(i);
                                break;
                            }
                        default:
                            {
                                InsertOperandCombobox(i);
                                (_equation[i] as ComboBox).Items[0] = equationArray[i];
                                break;
                            }
                    }
                }
                int indx = equationName.LastIndexOf("_Ar");
                VariableNumber = equationName.Substring(indx + 3, equationName.Length - indx - 3);
                VariableName = equationName.Substring(0, indx);
            }
            else
            {
                _variableName = string.Empty;
                _variableNumber = string.Empty;
                InsertOperandCombobox(0);
                InsertOperatorCombobox(1);
                InsertOperandCombobox(2);
            }
        }

        #endregion Constructors

        #region Events

        /// <summary>
        /// Raised when a property on this object has a new value.
        /// </summary>
        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public Visibility AddVisibility
        {
            get
            {
                return _addVisibility;
            }
            set
            {
                _addVisibility = value;
                OnPropertyChanged("AddVisibility");
            }
        }

        public string CompleteVariableName
        {
            get
            {
                return (_variableName + "_Ar" + _variableNumber);
            }
        }

        public CustomCollection<Control> Equation
        {
            get
            {
                return _equation;
            }
            set
            {
                _equation = value;
            }
        }

        public string[] EquationArray
        {
            get
            {
                string[] equationArray = new string[_equation.Count];
                for (int i = 0; i < _equation.Count; i++)
                {
                    if (_equation[i] is Label)
                    {
                        equationArray[i] = (_equation[i] as Label).Content.ToString();
                    }
                    else if (_equation[i] is ComboBox)
                    {
                        equationArray[i] = (_equation[i] as ComboBox).SelectedItem.ToString();
                    }
                }
                return equationArray;
            }
        }

        public string VariableName
        {
            get
            {
                return _variableName;
            }
            set
            {
                _variableName = value;
            }
        }

        public string VariableNumber
        {
            get
            {
                return _variableNumber + ". ";
            }
            set
            {

                _variableNumber =  value;
            }
        }

        public ChartViewModel VM
        {
            get;
            set;
        }

        #endregion Properties

        #region Methods

        /// <summary>
        /// Warns the developer if this object does not have a public property with
        /// the specified name. This method does not exist in a Release build.
        /// </summary>
        [Conditional("DEBUG")]
        [DebuggerStepThrough]
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

        private void Add_Click(object sender, RoutedEventArgs e)
        {
            InsertOperatorCombobox(_equation.Count);
            InsertOperandCombobox(_equation.Count);
        }

        private void InsertLeftParenthesis(int indx)
        {
            Thickness margin = new Thickness(1,3,0,3);
            Label leftParenthesis = new Label();
            leftParenthesis.Margin = margin;
            leftParenthesis.FontSize = 25;
            leftParenthesis.Content = "(";
            _equation.Insert(indx, leftParenthesis);
        }

        private void InsertOperandCombobox(int indx)
        {
            Thickness margin = new Thickness(0, 3, 0, 3);
            ComboBox operand = new ComboBox();
            operand.Width = 75;
            operand.MinWidth = 75;
            operand.Margin = margin;
            operand.Items.Add(string.Empty);
            operand.Items.Add("Define");
            operand.Items.Add("Complex");
            operand.SelectedIndex = 0;
            operand.Tag = indx;
            operand.SelectionChanged += operand_SelectionChanged;
            _equation.Insert(indx, operand);
        }

        private void InsertOperatorCombobox(int indx)
        {
            Thickness margin = new Thickness(3);
            ComboBox mathOperators = new ComboBox();
            mathOperators.Width = 35;
            mathOperators.MinWidth = 35;
            mathOperators.Margin = margin;
            mathOperators.Items.Add("+");
            mathOperators.Items.Add("-");
            mathOperators.Items.Add("*");
            mathOperators.Items.Add("/");
            mathOperators.Items.Add("Delete");
            mathOperators.SelectedIndex = 0;
            mathOperators.Tag = indx;
            mathOperators.SelectionChanged += mathOperators_SelectionChanged;
            _equation.Insert(indx, mathOperators);
        }

        private void InsertRightParenthesis(int indx)
        {
            Thickness margin = new Thickness(0, 3, 1, 3);
            Label rightParenthesis = new Label();
            rightParenthesis.Margin = margin;
            rightParenthesis.FontSize = 25;
            rightParenthesis.Content = ")";
            _equation.Insert(indx, rightParenthesis);
        }

        void mathOperators_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            ComboBox tmp = sender as ComboBox;
            string selectedItem = tmp.SelectedItem.ToString();
            if ("Delete" == selectedItem)
            {
                int tag= (int)tmp.Tag;
                if (false == _equation[tag + 1] is ComboBox)
                {
                    if (_equation[tag + 1] is Label)
                    {
                        string tmplbl1 = (_equation[tag + 1] as Label).Content.ToString();
                        if ("(" == tmplbl1)
                        {
                            int openVsClosed = 0;
                            for (int i = tag + 1; i < _equation.Count;)
                            {
                                if (_equation[i] is Label)
                                {
                                    tmplbl1 = (_equation[i] as Label).Content.ToString();
                                    if (")" == tmplbl1)
                                    {
                                        openVsClosed--;
                                    }
                                    else if ("(" == tmplbl1)
                                    {
                                        openVsClosed++;
                                    }

                                    if (0 == openVsClosed)
                                    {
                                        _equation.RemoveAt(i);
                                        break;
                                    }
                                }
                                _equation.RemoveAt(i);
                            }
                        }
                    }
                }
                else
                {
                    _equation.RemoveAt(tag + 1);
                }
                _equation.Remove(sender);
                if (tag-1 > 0 && _equation[tag - 2] is Label)
                {
                    string tmplbl1 = (_equation[tag - 2] as Label).Content.ToString();
                    if ("(" == tmplbl1)
                    {
                        _equation.RemoveAt(tag - 2);
                        for (int i = tag - 1; i < _equation.Count; i++)
                        {
                            if (_equation[i] is Label)
                            {
                                string tmplbl2 = (_equation[i] as Label).Content.ToString();
                                if (")" == tmplbl2)
                                {
                                    _equation.RemoveAt(i);
                                    break;
                                }
                            }
                        }
                    }
                }

                ReTag();
            }
        }

        private void operand_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            ComboBox tmp = sender as ComboBox;
            string selectedItem = string.Empty;
            if (null != tmp.SelectedItem)
            {
                selectedItem = tmp.SelectedItem.ToString();
            }

            if ("Define" == selectedItem)
            {
                OperandChooser operandChooser = new OperandChooser(VM);
                operandChooser.ShowDialog();
                if (true == operandChooser.DialogResult)
                {
                    (sender as ComboBox).Items[0] = operandChooser.Operand;
                }
            }
            else if ("Complex" == selectedItem)
            {
                InsertRightParenthesis((int)tmp.Tag + 1);
                InsertOperandCombobox((int)tmp.Tag + 1);
                InsertOperatorCombobox((int)tmp.Tag + 1);
                InsertLeftParenthesis((int)tmp.Tag);
                ReTag();
            }
            (sender as ComboBox).SelectedIndex = 0;
        }

        private void ReTag()
        {
            for (int i = 0; i < _equation.Count; i++)
            {
                _equation[i].Tag = i;
            }
        }

        #endregion Methods
    }
}