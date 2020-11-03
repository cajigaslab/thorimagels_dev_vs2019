namespace MultiROIStats.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.Data;
    using System.IO;
    using System.Linq;
    using System.Text;
    using System.Text.RegularExpressions;
    using System.Threading;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Input;
    using System.Windows.Threading;

    using Microsoft.Win32;

    using MultiROIStats.Model;

    public class MultiROIStatsViewModel : ViewModelBase
    {
        #region Fields

        private readonly MultiROIStatsModel _model;

        private bool frz;
        private object lockObject = new object();
        Visibility _arithmeticsVisibility = Visibility.Collapsed;
        int _columnCount = 0;
        bool _dataViewIsbusy = false;
        private int _dig = 5;
        private double _fieldSize;
        private List<string> _filters = new List<string>();
        int _rowCount = 0;
        private RelayCommand _saveAllCommand;
        private RelayCommand _saveCommand;
        private DataTable _statsDataTable; // ItemSource of DataGrid

        #endregion Fields

        #region Constructors

        public MultiROIStatsViewModel()
        {
            _model = new MultiROIStatsModel();
            _statsDataTable = new DataTable();
            frz = false;
        }

        #endregion Constructors

        #region Events

        public event Action SavingStats;

        #endregion Events

        #region Properties

        public DataView ArithmeticsDataset
        {
            get
            {
                if (true == _dataViewIsbusy)
                {
                    return null;
                }
                try
                {
                    _dataViewIsbusy = true;
                    if (!this._model.AInitialized)
                    {
                        _dataViewIsbusy = false;
                        return null;
                    }
                    else
                    {
                        if (null == this._model)
                        {
                            _dataViewIsbusy = false;
                            return null;
                        }
                        else if (null == this._model.ArithmeticStatsDataset)
                        {
                            _dataViewIsbusy = false;
                            return null;
                        }
                        else if (null == this._model.ArithmeticStatsDataset.Tables)
                        {
                            _dataViewIsbusy = false;
                            return null;
                        }
                        else if (null == this._model.ArithmeticStatsDataset.Tables[0])
                        {
                            _dataViewIsbusy = false;
                            return null;
                        }
                        else
                        {
                            this._model.ArithmeticStatsDataset.Tables[0].BeginLoadData();
                            DataView view = this._model.ArithmeticStatsDataset.Tables[0].DefaultView;
                            this._model.ArithmeticStatsDataset.Tables[0].EndLoadData();
                            _dataViewIsbusy = false;
                            view.Sort = "Index ASC";
                            return view;
                        }
                    }
                }
                catch
                {
                    _model.ResetArithmeticDataSet();
                    _dataViewIsbusy = false;
                    return null;
                }
            }
        }

        public List<string> ArithmeticStatNames
        {
            get
            {
                return this._model.ArithmeticStatNames;
            }
            set
            {
                this._model.ArithmeticStatNames = value;
                OnPropertyChanged("Name");
            }
        }

        public DataTable ArithmeticStatsDataTable
        {
            get
            {
                if (null == this._model)
                {
                    return null;
                }
                else if (null == this._model.ArithmeticStatsDataTable)
                {
                    return null;
                }
                else
                {
                    return this._model.ArithmeticStatsDataTable;
                }
            }
        }

        public Visibility ArithmeticsVisibility
        {
            get
            {
                return _arithmeticsVisibility;
            }
        }

        public List<string> BasicStatNames
        {
            get
            {
                return this._model.BasicStatNames;
            }
            set
            {
                this._model.BasicStatNames = value;
                OnPropertyChanged("Name");
            }
        }

        public List<string> ChanLabel
        {
            get
            {
                return this._model.ChanLabels;
            }
            set
            {
                this._model.ChanLabels = value;
                OnPropertyChanged("ChanLabel");
            }
        }

        public List<string> ColumnLabel
        {
            get
            {
                return this._model.ColumnLabels;
            }
            set
            {
                this._model.ColumnLabels = value;
                OnPropertyChanged("ColumnLabel");
            }
        }

        public double FieldSize
        {
            get
            {
                return _fieldSize;
            }
            set
            {
                _fieldSize = value;
            }
        }

        public List<int> RowLabel
        {
            get
            {
                return this._model.RowLabels;
            }
            set
            {
                this._model.RowLabels = value;
                OnPropertyChanged("RowLabel");
            }
        }

        public ICommand SaveAllCommand
        {
            get
            {
                if (_saveAllCommand == null)
                {
                    _saveAllCommand = new RelayCommand(() => OnSaveAll());
                }

                return _saveAllCommand;
            }
        }

        public ICommand SaveCommand
        {
            get
            {
                if (_saveCommand == null)
                {
                    _saveCommand = new RelayCommand(() => OnSave());
                }

                return _saveCommand;
            }
        }

        public DataView StatsDataset
        {
            get
            {
                if (true == _dataViewIsbusy)
                {
                    return null;
                }
                try
                {
                    _dataViewIsbusy = true;
                    if (!this._model.BInitialized)
                    {
                        _dataViewIsbusy = false;
                        return null;
                    }
                    else
                    {
                        if (null == this._model)
                        {
                            _dataViewIsbusy = false;
                            return null;
                        }
                        else if (null == this._model.BasicStatsDataset)
                        {
                            _dataViewIsbusy = false;
                            return null;
                        }
                        else if (null == this._model.BasicStatsDataset.Tables)
                        {
                            _dataViewIsbusy = false;
                            return null;
                        }
                        else if (null == this._model.BasicStatsDataset.Tables[0])
                        {
                            _dataViewIsbusy = false;
                            return null;
                        }
                        else
                        {
                            if (_rowCount != StatsDataTable.Rows.Count || _columnCount != StatsDataTable.Columns.Count)
                            {
                                _model.ResetBasicDataSet();
                                _rowCount = StatsDataTable.Rows.Count;
                                _columnCount = StatsDataTable.Columns.Count;
                                _dataViewIsbusy = false;
                                return null;
                            }
                            this._model.BasicStatsDataset.Tables[0].BeginLoadData();
                            DataView view = this._model.BasicStatsDataset.Tables[0].DefaultView;
                            this._model.BasicStatsDataset.Tables[0].EndLoadData();
                            _dataViewIsbusy = false;
                            return view;
                        }
                    }
                }
                catch
                {
                    _model.ResetBasicDataSet();
                    _dataViewIsbusy = false;
                    return null;
                }
            }
        }

        public DataTable StatsDataTable
        {
            get
            {
                if (null == this._model)
                {
                    return null;
                }
                else if (null == this._model.BasicStatsDataTable)
                {
                    return null;
                }
                else
                {
                    return this._model.BasicStatsDataTable;
                }
            }
        }

        #endregion Properties

        #region Methods

        public void ClearData()
        {
            BasicStatNames.Clear();
            this._model.BasicStatsDataset.Clear();
            this._model.BasicStatsDataTable.Clear();
            this._model.ArithmeticStatsDataTable.Clear();
            this._model.ArithmeticStatsDataset.Clear();
        }

        public void SetArithmeticsData(string[] statNames, double[] stats)
        {
            lock (lockObject)
            {
                try
                {
                    if (null != this.ArithmeticStatNames)
                    {
                        this.ArithmeticStatNames.Clear();
                    }
                    if (null != this._model.ArithmeticDataArray)
                    {
                        this._model.ArithmeticDataArray.Clear();
                    }

                    if (!(0 < statNames.Length) || statNames.Length != stats.Length)
                    {
                        _arithmeticsVisibility = Visibility.Collapsed;
                        OnPropertyChanged("ArithmeticsVisibility");
                        return;
                    }
                    else
                    {
                        _arithmeticsVisibility = Visibility.Visible;
                        OnPropertyChanged("ArithmeticsVisibility");
                    }

                    for (int i = 0; i < statNames.Length; i++)
                    {
                        if (null == statNames[i])
                        {
                            continue;
                        }
                        this.ArithmeticStatNames.Add(statNames[i]);
                    }

                    //double[] subStats = GetStatsSubArray(stats, this.ArithmeticStatNames.Count);
                    //statsVal = statsVal.Concat(subStats).ToArray();

                    if (stats.Length == ArithmeticStatNames.Count)
                    {
                        this._model.ArithmeticDataArray.Add(stats);
                        this._model.RerenderArithmeticStatsDataTable();
                    }
                }
                catch (Exception e)
                {
                    e.ToString();
                    return;
                }
            }
        }

        public void SetData(string[] statNames, double[] stats)
        {
            lock (lockObject)
            {
                try
                {
                    if (frz) return;

                    if (null != this.ChanLabel)
                    {
                        this.ChanLabel.Clear();
                    }
                    if (null != this.ColumnLabel)
                    {
                        this.ColumnLabel.Clear();
                        this.ColumnLabel.Add("Index");
                    }
                    if (null != this.RowLabel)
                    {
                        this.RowLabel.Clear();
                    }
                    if (null != this.BasicStatNames)
                    {
                        this.BasicStatNames.Clear();
                    }
                    if (null != this._model.BasicDataArray)
                    {
                        this._model.BasicDataArray.Clear();
                    }

                    string[] names = new string[0];
                    double[] statsVal = new double[0];

                    //double[] subStats = GetStatsSubArray(stats, statNames.Length);

                    names = names.Concat(statNames).ToArray();
                    //statsVal = statsVal.Concat(subStats).ToArray();

                    if (names.Any(s => s.Contains("tbar")))
                    {
                        if (!this._model.ColumnOrder.Any(s => s.Contains("tbar")))
                        {
                            this._model.ColumnOrder = new string[] { "min", "max", "mean", "stddev", "tbar", "left", "top", "width", "height" };
                        }
                    }
                    else
                    {
                        if (this._model.ColumnOrder.Any(s => s.Contains("tbar")))
                        {
                            this._model.ColumnOrder = new string[] { "min", "max", "mean", "stddev", "left", "top", "width", "height" };
                        }
                    }

                    string pattern = "(.*)_(.*)_(.*)";
                    Regex ex = new Regex(pattern, RegexOptions.IgnoreCase);
                    string str = string.Empty;
                    int roi;

                    for (int i = 0; i < names.Length; i++)
                    {
                        if (null == names[i])
                        {
                            continue;
                        }
                        Match match = ex.Match(names[i]);
                        if (match.Groups.Count == 4)
                        {

                            int.TryParse(match.Groups[3].ToString(), out roi);
                            if (!RowLabel.Contains(roi))
                            {
                                RowLabel.Add(roi);
                            }

                            string chan = string.Empty;
                            switch (Convert.ToInt32(match.Groups[2].ToString()))
                            {
                                case 1: chan = "A"; break;
                                case 2: chan = "B"; break;
                                case 3: chan = "C"; break;
                                case 4: chan = "D"; break;
                            }

                            str = match.Groups[1].ToString() + chan;
                            if (!ChanLabel.Contains(chan))
                            {
                                ChanLabel.Add(chan);
                            }
                            if (!ColumnLabel.Contains(str))
                            {
                                ColumnLabel.Add(str);
                            }
                            string name = str + match.Groups[3].ToString();
                            if (null == name)
                            {
                                continue;
                            }
                            this.BasicStatNames.Add(name);
                        }
                    }

                    double[] subStats = GetStatsSubArray(stats, this.BasicStatNames.Count);
                    statsVal = statsVal.Concat(subStats).ToArray();
                    ChanLabel = ChanLabel.OrderBy(x => x).ToList();

                    if (statsVal.Length == BasicStatNames.Count)
                    {
                        this._model.BasicDataArray.Add(statsVal);
                        this._model.RerenderBasicStatsDataTable();
                    }
                }
                catch (Exception e)
                {
                    e.ToString();
                    return;
                }
            }
        }

        public void UpdateDataTable()
        {
            if (true == _model.BInitialized)
            {
                OnPropertyChanged("StatsDataset");
                OnPropertyChanged("ArithmeticsDataset");
            }
        }

        private double[] GetStatsSubArray(double[] source, int length)
        {
            double[] dest = new double[length];
            Array.Copy(source, 0, dest, 0, length);
            return dest;
        }

        void OnSave()
        {
            lock (lockObject)
            {
                if (null != this._model.BasicDataArray) // reticle returns _dataArray.Count == 0;
                {
                    if (this._model.BasicDataArray.Count > 0)
                    {
                        if (SavingStats != null)
                        {
                            SavingStats();
                        }
                        string str = Application.Current.Resources["AppRootFolder"].ToString() + "\\" + "ROIStats.txt";

                        using (StreamWriter w = File.AppendText(str))
                        {
                            w.Write(string.Format("{0:yyyy-MM-dd_hh-mm-ss}", DateTime.Now));

                            w.Write(",{0}", FieldSize);

                            double width = 1;
                            double height = 1;
                            double val = 0;
                            int lastRow = _model.BasicStatsDataTable.Rows.Count - 1;
                            int i = _model.BasicStatsDataTable.Columns.IndexOf(_model.BasicStatsDataTable.Columns.Cast<DataColumn>().Where(x => x.ColumnName.StartsWith("width")).FirstOrDefault());
                            if (i >= 0)
                            {
                                Double.TryParse(_model.BasicStatsDataTable.Rows[lastRow][i].ToString(), out width);
                            }
                            i = _model.BasicStatsDataTable.Columns.IndexOf(_model.BasicStatsDataTable.Columns.Cast<DataColumn>().Where(x => x.ColumnName.StartsWith("height")).FirstOrDefault());
                            if (i >= 0)
                            {
                                Double.TryParse(_model.BasicStatsDataTable.Rows[lastRow][i].ToString(), out height);
                            }

                            double aspectRatio = Math.Round(height / width, _dig);
                            w.Write(",{0}", aspectRatio);

                            i = _model.BasicStatsDataTable.Columns.IndexOf(_model.BasicStatsDataTable.Columns.Cast<DataColumn>().Where(x => x.ColumnName.StartsWith("left")).FirstOrDefault());
                            if (i >= 0)
                            {
                                Double.TryParse(_model.BasicStatsDataTable.Rows[lastRow][i].ToString(), out val);
                                w.Write(",{0}", val);
                            }

                            i = _model.BasicStatsDataTable.Columns.IndexOf(_model.BasicStatsDataTable.Columns.Cast<DataColumn>().Where(x => x.ColumnName.StartsWith("top")).FirstOrDefault());
                            if (i >= 0)
                            {
                                Double.TryParse(_model.BasicStatsDataTable.Rows[lastRow][i].ToString(), out val);
                                w.Write(",{0}", val);
                            }
                            w.Write(",{0}", width);

                            w.Write(",{0}", height);

                            i = _model.BasicStatsDataTable.Columns.IndexOf(_model.BasicStatsDataTable.Columns.Cast<DataColumn>().Where(x => x.ColumnName.StartsWith("min")).FirstOrDefault());
                            if (i >= 0)
                            {
                                Double.TryParse(_model.BasicStatsDataTable.Rows[lastRow][i].ToString(), out val);
                                w.Write(",{0}", val);
                            }

                            i = _model.BasicStatsDataTable.Columns.IndexOf(_model.BasicStatsDataTable.Columns.Cast<DataColumn>().Where(x => x.ColumnName.StartsWith("max")).FirstOrDefault());
                            if (i >= 0)
                            {
                                Double.TryParse(_model.BasicStatsDataTable.Rows[lastRow][i].ToString(), out val);
                                w.Write(",{0}", val);
                            }

                            i = _model.BasicStatsDataTable.Columns.IndexOf(_model.BasicStatsDataTable.Columns.Cast<DataColumn>().Where(x => x.ColumnName.StartsWith("mean")).FirstOrDefault());
                            if (i >= 0)
                            {
                                Double.TryParse(_model.BasicStatsDataTable.Rows[lastRow][i].ToString(), out val);
                                w.Write(",{0}", val);
                            }

                            i = _model.BasicStatsDataTable.Columns.IndexOf(_model.BasicStatsDataTable.Columns.Cast<DataColumn>().Where(x => x.ColumnName.StartsWith("stddev")).FirstOrDefault());
                            if (i >= 0)
                            {
                                Double.TryParse(_model.BasicStatsDataTable.Rows[lastRow][i].ToString(), out val);
                                w.Write(",{0}", val);
                            }

                            i = _model.BasicStatsDataTable.Columns.IndexOf(_model.BasicStatsDataTable.Columns.Cast<DataColumn>().Where(x => x.ColumnName.StartsWith("tbar")).FirstOrDefault());
                            if (i >= 0)
                            {
                                Double.TryParse(_model.BasicStatsDataTable.Rows[lastRow][i].ToString(), out val);
                                w.Write(",{0}", val);
                            }

                            w.Write("\r\n");
                        }
                    }
                }
            }
        }

        void OnSaveAll()
        {
            lock (lockObject)
            {
                if (null != this._model.BasicDataArray && this._model.BasicDataArray.Count > 0) // reticle returns _dataArray.Count == 0;
                {
                    frz = true;
                    SaveFileDialog svFlDlg = new SaveFileDialog();
                    svFlDlg.Filter = "CSV File|*.csv|TXT File|*.txt|RAW File|*.raw";
                    svFlDlg.Title = "Save as ...";
                    svFlDlg.ShowDialog();

                    if (svFlDlg.FileName != "")
                    {
                        using (StreamWriter w = File.CreateText(svFlDlg.FileName))
                        {
                            for (int j = 0; j < _model.BasicStatsDataTable.Columns.Count; j++)
                            {
                                w.Write("{0}", _model.BasicStatsDataTable.Columns[j]);

                                if (j != (_model.BasicStatsDataTable.Columns.Count - 1))
                                {
                                    w.Write(",");
                                }
                            }
                            w.Write("\n");

                            for (int i = 0; i < _model.BasicStatsDataTable.Rows.Count; i++)
                            {
                                for (int j = 0; j < _model.BasicStatsDataTable.Columns.Count; j++)
                                {
                                    w.Write("{0}", _model.BasicStatsDataTable.Rows[i][j]);

                                    if (j != (_model.BasicStatsDataTable.Columns.Count - 1))
                                    {
                                        w.Write(",");
                                    }
                                }
                                w.Write("\n");
                            }
                        }
                    }
                    frz = false;
                }
            }
        }

        #endregion Methods
    }
}