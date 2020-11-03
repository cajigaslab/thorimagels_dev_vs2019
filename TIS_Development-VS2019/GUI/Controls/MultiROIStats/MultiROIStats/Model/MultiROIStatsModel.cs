namespace MultiROIStats.Model
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.ComponentModel;
    using System.Data;
    using System.Data.OleDb;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Text;
    using System.Text.RegularExpressions;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Data;

    class MultiROIStatsModel
    {
        #region Fields

        private bool _aInitialized = false;
        List<double[]> _arithmeticDataArray = new List<double[]>();
        List<string> _arithmeticStatNames = new List<string>();
        private DataSet _arithmeticStatsDataset = new DataSet();
        private DataTable _arithmeticStatsDataTable = new DataTable();
        List<double[]> _basicDataArray = new List<double[]>();
        List<string> _basicStatNames = new List<string>();
        private DataSet _basicStatsDataset = new DataSet();
        private DataTable _basicStatsDataTable = new DataTable();
        private bool _bInitialized = false;
        List<string> _chanLabels = new List<string>();
        List<string> _columnLabels = new List<string>();
        string[] _columnOrder = { "min", "max", "mean", "stddev", "tbar", "left", "top", "width", "height" };
        List<int> _rowLabels = new List<int>();

        #endregion Fields

        #region Constructors

        public MultiROIStatsModel()
        {
        }

        #endregion Constructors

        #region Properties

        public bool AInitialized
        {
            get
            {
                return _aInitialized;
            }
            set
            {
                _aInitialized = value;
            }
        }

        public List<double[]> ArithmeticDataArray
        {
            get
            {
                return _arithmeticDataArray;
            }
            set
            {
                _arithmeticDataArray = value;
            }
        }

        public List<string> ArithmeticStatNames
        {
            get
            {
                return _arithmeticStatNames;
            }
            set
            {
                _arithmeticStatNames = value;
            }
        }

        public DataSet ArithmeticStatsDataset
        {
            get
            {
                return _arithmeticStatsDataset;
            }

            set
            {
                _arithmeticStatsDataset = value;
            }
        }

        public DataTable ArithmeticStatsDataTable
        {
            get
            {
                return _arithmeticStatsDataTable;
            }

            set
            {
                _arithmeticStatsDataTable = value;
            }
        }

        public List<double[]> BasicDataArray
        {
            get
            {
                return _basicDataArray;
            }
            set
            {
                _basicDataArray = value;
            }
        }

        public List<string> BasicStatNames
        {
            get
            {
                return _basicStatNames;
            }
            set
            {
                _basicStatNames = value;
            }
        }

        public DataSet BasicStatsDataset
        {
            get
            {
                return _basicStatsDataset;
            }

            set
            {
                _basicStatsDataset = value;
            }
        }

        public DataTable BasicStatsDataTable
        {
            get
            {
                return _basicStatsDataTable;
            }

            set
            {
                _basicStatsDataTable = value;
            }
        }

        public bool BInitialized
        {
            get
            {
                return _bInitialized;
            }
            set
            {
                _bInitialized = value;
            }
        }

        public List<string> ChanLabels
        {
            get
            {
                return _chanLabels;
            }
            set
            {
                _chanLabels = value;
            }
        }

        public List<string> ColumnLabels
        {
            get
            {
                return _columnLabels;
            }
            set
            {
                _columnLabels = value;
            }
        }

        public string[] ColumnOrder
        {
            get
            {
                return _columnOrder;
            }
            set
            {
                _columnOrder = value;
            }
        }

        public List<int> RowLabels
        {
            get
            {
                return _rowLabels;
            }
            set
            {
                _rowLabels = value;
            }
        }

        #endregion Properties

        #region Methods

        public List<string> FindOrderLabelStartWith(List<string> strings, string subString)
        {
            List<string> sorted = new List<string>();
            List<string> tmp =  strings.Where(b => b.Contains(subString)).ToList();
            if (tmp.Count == 0)
            {
                return null;
            }
            for (int i = 0; i < _columnOrder.Length; i++)
            {
                int id = tmp.IndexOf(tmp.Where(r => r.StartsWith(_columnOrder[i])).FirstOrDefault());
                if (id > -1)
                {
                    sorted.Add(tmp[id]);
                }
            }
            return sorted;
        }

        public void RerenderArithmeticStatsDataTable()
        {
            if (1 > _arithmeticStatNames.Count)
            {
                return;
            }
            bool needResetTable = false;

            BackgroundWorker worker = new BackgroundWorker();
            worker.DoWork += (o, ea) =>
            {
                //(Re)Create table:
                lock (_arithmeticStatsDataTable)
                {
                    try
                    {
                        bool needUpdate = false;
                        if (_arithmeticStatNames.Count != _arithmeticStatsDataTable.Rows.Count)
                        {
                            needUpdate = true;
                        }
                        else
                        {
                            for (int i = 0; i < _arithmeticStatNames.Count; i++)
                            {
                                int indx = _arithmeticStatNames[i].LastIndexOf("_Ar");
                                string nm = _arithmeticStatNames[i].Substring(0, indx);
                                string num = Convert.ToInt32(_arithmeticStatNames[i].Substring(indx + 3, _arithmeticStatNames[i].Length - indx - 3)).ToString();
                                if (false == FindNameInColumnInArithmeticStatsTable(nm, 1))
                                {
                                    needUpdate = true;
                                    break;
                                }
                                if (false == FindNameInColumnInArithmeticStatsTable(num, 0))
                                {
                                    needUpdate = true;
                                    break;
                                }
                            }
                        }
                        if (true == needUpdate)
                        {
                            DataTable dt = new DataTable();
                            dt.Columns.Add("Index");
                            dt.Columns.Add("Variable");
                            dt.Columns.Add("Value");
                            int j = 0;
                            foreach (string name in _arithmeticStatNames)
                            {
                                int indx = name.LastIndexOf("_Ar");
                                string nm = name.Substring(0, indx);
                                string num = Convert.ToInt32(name.Substring(indx + 3, name.Length - indx - 3)).ToString();
                                dt.Rows.Add(num);
                                dt.Rows[j][1] = nm;
                                j++;
                            }
                            _arithmeticStatsDataTable = dt;
                            needResetTable = true;
                        }
                        for (int i = 0; i < _arithmeticStatsDataTable.Rows.Count; i++)
                        {
                            string data = _arithmeticDataArray[0][i].ToString(string.Format("F4"));
                            if (null != data)
                            {
                                string compareData = _arithmeticStatsDataTable.Rows[i][2].ToString();
                                if (false == compareData.Equals(data))
                                {
                                    _arithmeticStatsDataTable.Rows[i][2] = data;
                                }
                            }
                            else
                            {
                                _arithmeticStatsDataTable.Rows[i][2] = " ";
                            }
                        }
                    }
                    catch (Exception e)
                    {
                        needResetTable = false;
                        e.ToString();
                        return;
                    }
                }
            };

            worker.RunWorkerCompleted += (o, ea) =>
            {
                try
                {
                    if ((needResetTable) || (!_aInitialized))
                    {
                        ResetArithmeticDataSet();
                    }
                }
                catch (Exception e)
                {
                    e.ToString();
                    return;
                }
            };
            worker.RunWorkerAsync();
        }

        public void RerenderBasicStatsDataTable()
        {
            bool needResetTable = false;

            BackgroundWorker worker = new BackgroundWorker();
            worker.DoWork += (o, ea) =>
            {
                //(Re)Create table:
                lock (_basicStatsDataTable)
                {
                    try
                    {
                        bool needUpdate = false;
                        if ((ColumnLabels.Count != _basicStatsDataTable.Columns.Count) || (RowLabels.Count != _basicStatsDataTable.Rows.Count) ||
                            false == _bInitialized)
                        {
                            needUpdate = true;
                        }
                        else
                        {
                            for (int i = 0; i < RowLabels.Count; i++)
                            {
                                //look at the row label and determine if an update is needed
                                if (false == FindNameInColumnInBasicStatsTable(RowLabels[i].ToString(), 0))
                                {
                                    needUpdate = true;
                                    break;
                                }
                            }

                            for (int i = 0; i < ColumnLabels.Count; i++)
                            {
                                //look at the column label and determine if an update is needed
                                if (false == FindNameInRowInBasicStatsTable(ColumnLabels[i].ToString()))
                                {
                                    needUpdate = true;
                                    break;
                                }
                            }
                        }

                        if (true == needUpdate)
                        {
                            DataTable dt = new DataTable();
                            dt.Columns.Add("Index");

                            for (int i = 0; i < ChanLabels.Count; i++)
                            {
                                List<string> sorted = FindOrderLabelStartWith(ColumnLabels, ChanLabels[i]);
                                if (null != sorted)
                                {
                                    foreach (string name in sorted)
                                    {
                                        if (null != name)
                                        {
                                            dt.Columns.Add(name);
                                        }
                                    }
                                }
                            }
                            foreach (int name in RowLabels.OrderBy(r => r).ToList())
                            {
                                dt.Rows.Add(name);
                            }
                            _basicStatsDataTable = dt;
                            needResetTable = true;
                        }

                        //fill table with data:
                        if ((_basicStatsDataTable.Columns.Count - 1) * _basicStatsDataTable.Rows.Count != _basicDataArray[0].Count())
                        {
                            return;
                        }

                        for (int j = 1; j < _basicStatsDataTable.Columns.Count; j++)
                        {
                            for (int i = 0; i < _basicStatsDataTable.Rows.Count; i++)
                            {
                                string data = _basicDataArray[0][BasicStatNames.IndexOf(BasicStatNames.Where(x => x.Equals(_basicStatsDataTable.Columns[j].ColumnName + _basicStatsDataTable.Rows[i][0].ToString())).FirstOrDefault())].ToString(string.Format("F4"));
                                if (_basicStatsDataTable.Columns[j].ColumnName.Contains("top") ||
                                    _basicStatsDataTable.Columns[j].ColumnName.Contains("left") ||
                                    _basicStatsDataTable.Columns[j].ColumnName.Contains("width") ||
                                    _basicStatsDataTable.Columns[j].ColumnName.Contains("height") ||
                                    _basicStatsDataTable.Columns[j].ColumnName.Contains("min") ||
                                    _basicStatsDataTable.Columns[j].ColumnName.Contains("max"))
                                {
                                    int indx = data.IndexOf(".");
                                    data = data.Substring(0, indx);
                                }
                                if (null != data)
                                {
                                    string compareData = _basicStatsDataTable.Rows[i][j].ToString();
                                    if (false == compareData.Equals(data))
                                    {
                                        _basicStatsDataTable.Rows[i][j] = data;
                                    }
                                }
                                else
                                {
                                    _basicStatsDataTable.Rows[i][j] = " ";
                                }
                            }
                        }
                    }
                    catch (Exception e)
                    {
                        needResetTable = false;
                        e.ToString();
                        return;
                    }
                }
            };

            worker.RunWorkerCompleted += (o, ea) =>
            {
                try
                {
                    if ((needResetTable) || (!_bInitialized))
                    {
                        ResetBasicDataSet();
                    }
                }
                catch (Exception e)
                {
                    e.ToString();
                    return;
                }
            };
            worker.RunWorkerAsync();
        }

        public void ResetArithmeticDataSet()
        {
            try
            {
                if (_aInitialized)
                {
                    _arithmeticStatsDataset.Tables.RemoveAt(0);
                }
                _arithmeticStatsDataset.Tables.Add(_arithmeticStatsDataTable);
                _aInitialized = true;
            }
            catch (Exception e)
            {
                e.ToString();
                return;
            }
        }

        public void ResetBasicDataSet()
        {
            try
            {
                if (_bInitialized)
                {
                    _basicStatsDataset.Tables.RemoveAt(0);
                }
                _basicStatsDataset.Tables.Add(_basicStatsDataTable);
                _bInitialized = true;
            }
            catch (Exception e)
            {
                e.ToString();
                return;
            }
        }

        /// <summary>
        /// Finds the name in column in arithmetic stats table.
        /// </summary>
        /// <param name="name">The name.</param>
        /// <param name="column">The column.</param>
        /// <returns><c>true</c> if XXXX, <c>false</c> otherwise.</returns>
        private bool FindNameInColumnInArithmeticStatsTable(string name, int column)
        {
            for (int i = 0; i < _arithmeticStatsDataTable.Rows.Count; i++)
            {
                string cell = _arithmeticStatsDataTable.Rows[i][column].ToString();
                if (cell == name)
                {
                    return true;
                }
            }
            return false;
        }

        /// <summary>
        /// Finds the name in column in basic stats table.
        /// </summary>
        /// <param name="name">The name.</param>
        /// <param name="column">The column.</param>
        /// <returns><c>true</c> if XXXX, <c>false</c> otherwise.</returns>
        private bool FindNameInColumnInBasicStatsTable(string name, int column)
        {
            for (int i = 0; i < _basicStatsDataTable.Rows.Count; i++)
            {
                string cell = _basicStatsDataTable.Rows[i][column].ToString();
                if (cell == name)
                {
                    return true;
                }
            }
            return false;
        }

        /// <summary>
        /// Finds the name in row in basic stats table.
        /// </summary>
        /// <param name="name">The name.</param>
        /// <returns><c>true</c> if XXXX, <c>false</c> otherwise.</returns>
        private bool FindNameInRowInBasicStatsTable(string name)
        {
            for (int i = 0; i < _basicStatsDataTable.Columns.Count; i++)
            {
                string cell = _basicStatsDataTable.Columns[i].ToString();
                if (cell == name)
                {
                    return true;
                }
            }
            return false;
        }

        #endregion Methods
    }
}