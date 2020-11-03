namespace ROIStatsChart.ViewModel
{
    using System;
    using System.Collections;
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
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Threading;
    using System.Xml;

    using Abt.Controls.SciChart;
    using Abt.Controls.SciChart.Model.DataSeries;
    using Abt.Controls.SciChart.Visuals;
    using Abt.Controls.SciChart.Visuals.RenderableSeries;

    using HDF5CS;

    using ROIStatsChart.Model;
    using ROIStatsChart.View;

    using ThorSharedTypes;

    using XMLHandle;

    public partial class ChartViewModel : ViewModelBase
    {
        #region Fields

        private List<double> _arithmeticData = new List<double>();
        private Dictionary<string, int> _arithmeticDataChartIndex = new Dictionary<string, int>();
        private Dictionary<string, string[]> _arithmeticExpressions = new Dictionary<string, string[]>();
        private List<string> _arithmeticNames = new List<string>();
        private Dictionary<string, string[]> _arithmeticRPNOperations = new Dictionary<string, string[]>();
        private ICommand _arithmeticSettings;

        #endregion Fields

        #region Properties

        public double[] ArithmeticData
        {
            get
            {
                return _arithmeticData.ToArray();
            }
        }

        public string[] ArithmeticNames
        {
            get
            {
                return _arithmeticNames.ToArray();
            }
        }

        //Command
        public ICommand ArithmeticSettings
        {
            get
            {
                if (this._arithmeticSettings == null)
                    this._arithmeticSettings = new RelayCommand(() => OpenArithmeticsSettings());

                return this._arithmeticSettings;
            }
        }

        #endregion Properties

        #region Methods

        //Save the Calculated ROI Arithmetics to the pertaining H5 file
        public void SaveROIArithmeticDataToH5()
        {
            //Save only when in Edit mode and the data is completely loaded
            if (true == _editable && true == _dataStoreLoadComplete)
            {
                try
                {
                    int seriesSize = _chartSeries[0].DataSeries.Count;
                    lock (_chartSeries)
                    {
                        string[] names = new string[_arithmeticDataChartIndex.Count];
                        double[][] data = new double[_arithmeticDataChartIndex.Count][];

                        for (int i = 0; i < _arithmeticDataChartIndex.Count; i++)
                        {
                            data[i] = new double[seriesSize];
                        }

                        for (int i = 0; i < _arithmeticDataChartIndex.Count; i++)
                        {
                            int indx = _arithmeticDataChartIndex.ElementAt(i).Value;
                            names[i] = _chartSeries[indx].DataSeries.SeriesName;
                            data[i] = ((IXyDataSeries<double, double>)_chartSeries[indx].DataSeries).YValues.ToArray();
                        }

                        string H5pathname = _path + "\\" + "ROIData.h5";
                        H5CSWrapper h5io = new H5CSWrapper(H5pathname);
                        h5io.OpenRWH5();
                        string[] grpName = { "" };
                        for (int i = 0; i < names.Length; i++)
                        {
                            string[] name = new string[1];
                            name[0] = "/" + names[i];
                            h5io.CreateGroupDatasetNames<Double>(name[0], name, 1);
                            h5io.WriteDataset<Double>(name[0], name[0], data[i], 0, (UInt32)data[i].Length);
                        }
                        h5io.CloseH5();
                        h5io.DestroyH5();
                    }
                }
                catch (Exception e)
                {
                    e.ToString();
                }
            }
        }

        //Calculate new and Recalculate previous stats with latest forlulas while or after data is being loaded from the datastore
        private void CalculateArithmeticSeriesPostMortem(List<string> newVariables)
        {
            int nOriginalArithmeticStats = _arithmeticNames.Count;
            int nChartSeries = _chartSeries.Count;

            for (int i = _chartSeries.Count - 1; i >= 0; i--)
            {
                if (ChartSeries[i].DataSeries.SeriesName.Contains("_Ar"))
                {
                    _chartSeries.RemoveAt(i);
                }
            }

            Dictionary<string, string[]> arExpressions = new Dictionary<string, string[]>();

            int nBasicStats = _chartSeries.Count;
            _arithmeticNames.Clear();
            for (int i = 0; i < _arithmeticExpressions.Count; i++)
            {
                //verify if the validity of the equation
                if (false == ValidateEquation(i))
                {
                    continue;
                }

                string name = _arithmeticExpressions.Keys.ElementAt(i);
                arExpressions.Add(_arithmeticExpressions.Keys.ElementAt(i), _arithmeticExpressions.Values.ElementAt(i));
                _arithmeticNames.Add(name);

                double[] strokeDashArray = new[] { 3.0, 3.0 };
                int indx = name.LastIndexOf("_Ar");
                int arithmetic = Convert.ToInt32(name.Substring(indx + 3, name.Length - indx - 3));

                //Add new Arithmetic Series to _chartSeries
                IXyDataSeries<double, double> ds0;
                if ((0 == FifoSize) || (!IsFifoVisible))
                {
                    ds0 = new XyDataSeries<double, double> { FifoCapacity = null, SeriesName = name };
                }
                else
                {
                    ds0 = new XyDataSeries<double, double> { FifoCapacity = FifoSize, SeriesName = name };
                }
                lock (_chartSeries)
                {

                    if (true == _skipGeometricInfo)
                    {
                        _chartSeries.Add(new ChartSeriesViewModel(ds0, new FastLineRenderableSeries() { Tag = arithmetic, StrokeDashArray = strokeDashArray, StrokeThickness = 2, IsVisible = true, SeriesColor = (Color)ChartLineProperty.GetLineColor(name, typeof(Color)) }));
                    }
                    else
                    {
                        _chartSeries.Add(new ChartSeriesViewModel(ds0, new FastLineRenderableSeries() { Tag = arithmetic, StrokeDashArray = strokeDashArray, StrokeThickness = 2, IsVisible = true, SeriesColor = (Color)ChartLineProperty.GetLineColor(name, typeof(Color)) }));
                    }
                }
                if (false == _arithmeticDataChartIndex.ContainsKey(name))
                {
                    _arithmeticDataChartIndex.Add(name, _chartSeries.Count - 1);
                }
            }

            lock (_chartSeries)
            {
                for (int i = 0; i < arExpressions.Count; i++)
                {
                    int k = nBasicStats + i;
                    for (int j = 0; j < _chartSeries[0].DataSeries.Count; j++)
                    {

                        if (true == arExpressions.ContainsKey(_chartSeries[k].DataSeries.SeriesName))
                        {
                            DoubleSeries ds = new DoubleSeries();
                            object calculation = CalculateROIArithmeticsPointPostMortem(arExpressions, j, _chartSeries[k].DataSeries.SeriesName);
                            if (null == calculation)
                            {
                                break;
                            }
                            var xy = new XYPoint();
                            xy.X = ((IXyDataSeries<double, double>)_chartSeries[0].DataSeries).XValues[j];
                            xy.Y = (double)calculation;
                            ds.Add(xy);

                            ((IXyDataSeries<double, double>)_chartSeries[k].DataSeries).Append(ds.XData, ds.YData);
                        }
                    }
                }
            }

            string measure = string.Empty, channel = string.Empty;
            int roiIndex = 0;
            string pattern = "(.*)([A|B|C|D])([0-9]+$)";
            bool updateLegend = false;

            if ((0 == StatsNames.Length) || (0 == StatsData.Length))
            {
                return;
            }

            long seriesCount = StatsData.Length / StatsNames.Length;

            if (0 == seriesCount)
            {
                return;
            }

            //get names and data:
            GetDataSeriesNames();

            //build a list of the current series names and ROIs' id:
            List<string> curNames = new List<string>();
            List<int> curROIs = new List<int>();
            List<string> curArithmetics = new List<string>();

            lock (_chartSeries)
            {
                for (int i = 0; i < _chartSeries.Count; i++)
                {
                    curNames.Add(_chartSeries[i].DataSeries.SeriesName);

                    ParseDataSeriesName(_chartSeries[i].DataSeries.SeriesName, pattern, ref measure, ref channel, ref roiIndex);
                    if (!curROIs.Contains(roiIndex))
                    { curROIs.Add(roiIndex); }
                    if (-1 < curNames[i].LastIndexOf("_Ar"))
                    {
                        curArithmetics.Add(curNames[i]);
                    }
                }
            }

            updateLegend = false;

            //set roi legend based on current series names:
            SetDataSeriesROILegend(curNames, curROIs, ref updateLegend);

            //set Arithmetics legend based on current series names:
            SetDataSeriesArithmeticsLegend(curNames, curArithmetics, ref updateLegend);

            UpdateDataSeriesSelection();
            CollectROIDataAtX(_xReviewPosition /1000);
        }

        //Calculation for each formula entered in the ROIArithmetics window
        //or loaded from the ApplicationSettings.xml
        //done while or after the calculation
        private object CalculateROIArithmeticsPointPostMortem(Dictionary<string, string[]> arExpressions, int xPos, string name)
        {
            string[] value;
            arExpressions.TryGetValue(name, out value);
            string[] rpnOperation = ExpressionToRPN.GetRPN(value);

            Stack<double> results = new Stack<double>();
            for (int j = 0; j < rpnOperation.Length; j++)
            {
                double constant;
                int nameId = -1;
                for (int k = 0; k < _chartSeries.Count; k++)
                {
                    if (true == _chartSeries[k].DataSeries.SeriesName.Equals(rpnOperation[j]))
                    {
                        nameId = k;
                        break;
                    }
                }

                //if it exists in the name dataseries then push its value to the Results Stack
                if (nameId > -1)
                {
                    results.Push(((IXyDataSeries<double, double>)_chartSeries[nameId].DataSeries).YValues[xPos]);
                }
                //if its a number then push it to the Results Stack
                else if (true == double.TryParse(rpnOperation[j], out constant))
                {
                    results.Push(constant);
                }
                else
                {
                    if (results.Count < 2)
                    {
                        return null;
                    }
                    double v1 = 0;
                    double v2 = 0;

                    //if its any of the recognized operators pop the last two items
                    //in the results stack and do the calculation, then push the
                    //result back into the results stack
                    switch (rpnOperation[j])
                    {
                        case "+":
                            v2 = results.Pop();
                            v1 = results.Pop();
                            results.Push(v1 + v2);
                            break;
                        case "-":
                            v2 = results.Pop();
                            v1 = results.Pop();
                            results.Push(v1 - v2);
                            break;
                        case "*":
                            v2 = results.Pop();
                            v1 = results.Pop();
                            results.Push(v1 * v2);
                            break;
                        case "/":
                            v2 = results.Pop();
                            v1 = results.Pop();
                            if (0.0 != v2)
                            {
                                results.Push(v1 / v2);
                            }
                            else
                            {
                                results.Push(0.0);
                            }
                            break;
                        case "^":
                            v2 = results.Pop();
                            v1 = results.Pop();
                            results.Push(Math.Pow(v1, v2));
                            break;
                    }
                }
            }

            return results.Pop();
        }

        //Calculation for each formula entered in the ROIArithmetics window
        //or loaded from the ApplicationSettings.xml
        //Done in RT
        private void CalculateROIArithmeticsRT()
        {
            _arithmeticNames.Clear();
            _arithmeticData.Clear();
            for (int i = 0; i < _arithmeticExpressions.Count; i++)
            {
                string name = _arithmeticExpressions.Keys.ElementAt(i);
                if (false == _arithmeticRPNOperations.ContainsKey(name))
                {
                    string[] value;
                    _arithmeticExpressions.TryGetValue(name, out value);
                    _arithmeticRPNOperations.Add(name, ExpressionToRPN.GetRPN(value));
                }
                string[] rpnOperation;
                _arithmeticRPNOperations.TryGetValue(name, out rpnOperation);
                Stack<double> results = new Stack<double>();
                bool noErrors = true;
                for (int j = 0; j < rpnOperation.Length; j++)
                {
                    double constant;
                    int nameId = _seriesNames.IndexOf(rpnOperation[j]);
                    //if it exists in the name dataseries then push its value to the Results Stack
                    if (nameId > -1)
                    {
                        results.Push(_seriesData[nameId]);
                    }
                    //if its a number then push it to the Results Stack
                    else if (true == double.TryParse(rpnOperation[j], out constant))
                    {
                        results.Push(constant);
                    }
                    else
                    {
                        if (results.Count < 2)
                        {
                            noErrors = false;
                            break;
                        }
                        double v1 = 0;
                        double v2 = 0;

                        //if its any of the recognized operators pop the last two items
                        //in the results stack and do the calculation, then push the
                        //result back into the results stack
                        switch (rpnOperation[j])
                        {
                            case "+":
                                v2 = results.Pop();
                                v1 = results.Pop();
                                results.Push(v1 + v2);
                                break;
                            case "-":
                                v2 = results.Pop();
                                v1 = results.Pop();
                                results.Push(v1 - v2);
                                break;
                            case "*":
                                v2 = results.Pop();
                                v1 = results.Pop();
                                results.Push(v1 * v2);
                                break;
                            case "/":
                                v2 = results.Pop();
                                v1 = results.Pop();
                                if (0.0 != v2)
                                {
                                    results.Push(v1 / v2);
                                }
                                else
                                {
                                    results.Push(0.0);
                                }
                                break;
                            case "^":
                                v2 = results.Pop();
                                v1 = results.Pop();
                                results.Push(Math.Pow(v1, v2));
                                break;
                        }
                    }
                }
                if (true == noErrors)
                {
                    _arithmeticNames.Add(name);
                    _arithmeticData.Add(results.Peek());

                    int nameId2 = _seriesNames.IndexOf(name);
                    if (-1 != nameId2)
                    {
                        _seriesData[nameId2] = results.Pop();
                    }
                    else
                    {
                        _seriesNames.Add(name);
                        _seriesData.Add(results.Pop());
                    }
                }
            }
        }

        //Load the Arithmetic Equations from ApplicationSettings.xml
        private void LoadEquationsFromFile()
        {
            if (null != _appResource && true == _editable)
            {
                XMLHandler xmlGetter = new XMLHandler(_appResource);
                XmlDocument appSettings = new XmlDocument();
                if (xmlGetter.Load(appSettings))
                {
                    XmlNode node = appSettings.SelectSingleNode("/ApplicationSettings/DisplayOptions/ROIStats/Equations");

                    if (node != null)
                    {
                        List<string> equations = new List<string>();
                        List<string> names = new List<string>();
                        for (int i = 0; i < node.ChildNodes.Count; i++)
                        {
                            string attribute = string.Empty;
                            xmlGetter.GetAttribute(node.ChildNodes[i], appSettings, "Equation", ref attribute);
                            equations.Add(attribute);

                            xmlGetter.GetAttribute(node.ChildNodes[i], appSettings, "Name", ref attribute);
                            names.Add(attribute);
                        }
                        ParseFileEquations(names, equations);
                    }
                }
            }
            else if (false == _editable)
            {
                XMLHandler xmlGetter = new XMLHandler(_path + "\\ROIArithmetics.xml");
                XmlDocument appSettings = new XmlDocument();
                if (xmlGetter.Load(appSettings))
                {
                    XmlNode node = appSettings.SelectSingleNode("/ROIArithmetics/Equations");

                    if (node != null)
                    {
                        List<string> equations = new List<string>();
                        List<string> names = new List<string>();
                        for (int i = 0; i < node.ChildNodes.Count; i++)
                        {
                            string attribute = string.Empty;
                            xmlGetter.GetAttribute(node.ChildNodes[i], appSettings, "Equation", ref attribute);
                            equations.Add(attribute);

                            xmlGetter.GetAttribute(node.ChildNodes[i], appSettings, "Name", ref attribute);
                            names.Add(attribute);
                        }
                        ParseFileEquations(names, equations);
                    }
                }
            }
        }

        //Open the ROI arithmetics window ApplicationSettings.xml
        private void OpenArithmeticsSettings()
        {
            ROIArithmetics roiArithmetics = new ROIArithmetics(this, _arithmeticExpressions);
            roiArithmetics.Editable = _editable;
            roiArithmetics.ShowDialog();
            if (true == roiArithmetics.DialogResult)
            {

                List<string> newVariables = new List<string>();
                if (roiArithmetics.ArithmeticExpressions.Count > _arithmeticExpressions.Count)
                {
                    foreach (KeyValuePair<string, string[]> expression in roiArithmetics.ArithmeticExpressions)
                    {
                        if (false == _arithmeticExpressions.ContainsKey(expression.Key))
                        {
                            newVariables.Add(expression.Key);
                        }
                    }
                }

                _arithmeticRPNOperations = new Dictionary<string, string[]>();
                _arithmeticExpressions = roiArithmetics.ArithmeticExpressions;
                if (false == _isFifoVisible && true == _editable)
                {
                    CalculateArithmeticSeriesPostMortem(newVariables);
                    PersistEquationsToFile();
                }
                else if (true == _isFifoVisible)
                {
                    PersistEquationsToFile();
                }
            }
        }

        //Parse the equations found in the ApplicationSettings.xml
        private void ParseFileEquations(List<string> equationNames, List<string> equations)
        {
            try
            {
                _arithmeticExpressions = new Dictionary<string, string[]>();

                for (int i = 0; i < equations.Count; i++)
                {
                    List<string> equation = new List<string>();
                    int indx = equations[i].IndexOf(' ');
                    if (-1 == indx)
                    {
                        equation.Add(equations[i]);
                        _arithmeticExpressions.Add(equationNames[i], equation.ToArray());
                        continue;
                    }
                    string section = equations[i].Substring(0, indx);
                    string restOfEquation = equations[i].Substring(indx + 1, equations[i].Length - indx - 1);
                    equation.Add(section);
                    do
                    {
                        indx = restOfEquation.IndexOf(' ');
                        if (-1 == indx)
                        {
                            equation.Add(restOfEquation);
                            break;
                        }
                        section = restOfEquation.Substring(0, indx);
                        restOfEquation = restOfEquation.Substring(indx + 1, restOfEquation.Length - indx - 1);
                        equation.Add(section);
                    } while (true);
                    _arithmeticExpressions.Add(equationNames[i], equation.ToArray());
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.ToString());
            }
        }

        //Save the the Equations entered in the Arithmetics Window
        private void PersistEquationsToFile()
        {
            if (true == _editable)
            {
                try
                {
                    MVMManager.Instance.LoadSettings();
                    XmlDocument ApplicationDoc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.APPLICATION_SETTINGS];
                    XmlNode roiStatsNode = ApplicationDoc.SelectSingleNode("/ApplicationSettings/DisplayOptions/ROIStats");
                    if (null == roiStatsNode)
                    {
                        XmlNode newLink = ApplicationDoc.CreateNode(XmlNodeType.Element, "ROIStats", null);
                        XmlNode NodeAdmin = ApplicationDoc.SelectSingleNode("/ApplicationSettings/DisplayOptions");
                        NodeAdmin.AppendChild(newLink.Clone());
                    }

                    XmlNode equationsNode = ApplicationDoc.SelectSingleNode("/ApplicationSettings/DisplayOptions/ROIStats/Equations");

                    if (null != equationsNode)
                    {
                        roiStatsNode.RemoveAll();
                        XmlNode newLink = ApplicationDoc.CreateNode(XmlNodeType.Element, "Equations", null);
                        XmlNode nodeAdmin = ApplicationDoc.SelectSingleNode("/ApplicationSettings/DisplayOptions/ROIStats");
                        nodeAdmin.AppendChild(newLink.Clone());
                    }
                    else
                    {
                        XmlNode newLink = ApplicationDoc.CreateNode(XmlNodeType.Element, "Equations", null);
                        XmlNode nodeAdmin = ApplicationDoc.SelectSingleNode("/ApplicationSettings/DisplayOptions/ROIStats");
                        nodeAdmin.AppendChild(newLink.Clone());
                    }

                    //XMLHandler xmlSetter = new XMLHandler(_appResource);
                    equationsNode = ApplicationDoc.SelectSingleNode("/ApplicationSettings/DisplayOptions/ROIStats/Equations");
                    for (int i = 0; i < _arithmeticExpressions.Count; i++)
                    {
                        XmlNode newLink = ApplicationDoc.CreateNode(XmlNodeType.Element, "Equation" + (i + 1).ToString(), null);

                        equationsNode.AppendChild(newLink.Clone());
                        string key = _arithmeticExpressions.Keys.ElementAt(i);

                        int indx = key.LastIndexOf("_Ar") + 3;
                        string name = key.Substring(0, indx) + (i + 1).ToString();
                        XmlManager.SetAttribute(equationsNode.ChildNodes[i], ApplicationDoc, "Name", name);

                        string[] arithmeticExpression;
                        _arithmeticExpressions.TryGetValue(key, out arithmeticExpression);
                        string equation = string.Empty;
                        for (int j = 0; j < arithmeticExpression.Length; j++)
                        {
                            equation += arithmeticExpression[j];
                            if (arithmeticExpression.Length - 1 == j)
                            {
                                break;
                            }
                            equation += " ";
                        }
                        XmlManager.SetAttribute(equationsNode.ChildNodes[i], ApplicationDoc, "Equation", equation);
                    }

                    MVMManager.Instance.SaveSettings(SettingsFileType.APPLICATION_SETTINGS);

                    equationsNode = ApplicationDoc.SelectSingleNode("/ApplicationSettings/DisplayOptions/ROIStats/Equations");
                    if (false == _isFifoVisible && null != _path)
                    {
                        XmlDocument bigDoc = new XmlDocument();
                        bigDoc.LoadXml("<ROIArithmetics></ROIArithmetics>");
                        XmlNode requestNode = bigDoc.FirstChild;
                        requestNode.AppendChild(requestNode.OwnerDocument.ImportNode(equationsNode, true));
                        bigDoc.Save(_path + "\\ROIArithmetics.xml");
                    }
                }
                catch (Exception e)
                {
                    e.ToString();
                }
            }
        }

        //Checks every field of the equation verifying that all variables
        //can be found in the chartSeries and that those field that are not
        //variables are constants or valid operators
        private bool ValidateEquation(int indx)
        {
            string[] rpn = ExpressionToRPN.GetRPN(_arithmeticExpressions.Values.ElementAt(indx));
            for (int j = 0; j < rpn.Length; j++)
            {
                double constant = 0;
                if ("+" != rpn[j] && "-" != rpn[j] && "*" != rpn[j] && "/" != rpn[j] &&
                    false == double.TryParse(rpn[j], out constant))
                {
                    int nameId = _seriesNames.IndexOf(_seriesNames.Where(x => x.Equals(rpn[j])).FirstOrDefault());
                    if (-1 == nameId)
                    {
                        return false;
                    }
                }
            }
            return true;
        }

        #endregion Methods
    }
}