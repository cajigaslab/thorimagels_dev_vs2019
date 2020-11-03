namespace KuriosControl.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.IO;
    using System.Linq;
    using System.Text;
    using System.Windows;
    using System.Windows.Data;
    using System.Windows.Input;

    using KuriosControl.Common;
    using KuriosControl.Model;
    using KuriosControl.View;

    #region Enumerations

    public enum BandwidthModes
    {
        WIDE = 2,
        MEDIUM = 4,
        NARROW = 8
    }

    #endregion Enumerations

    public class ControlViewModel : ViewModelBase
    {
        #region Fields

        public const int MAX_SEQUENCE_DATA_POINTS = 310;

        ICommand _addWavelengthSequenceCommand;
        private BandwidthModes _bandwidthMode = BandwidthModes.WIDE;
        private ICommand _clearPlotCommand;
        double _currentExposure = 0;
        private int _currentWavelength = 420;
        private string _CurrentWavelengthSequence;
        private ICommand _editWavelengthSequenceCommand;
        double _exposureMax = 10000;
        double _exposureMin = 0;
        private bool _isEnabled = false;
        private ICommand _recordPlotCommand;
        WavelengthExposureEditWindow _sequenceEditPlot = null;
        private SequenceParameter _sequenceParameter = new SequenceParameter()
        {
            SeqWavelengthStart = 420,
            SeqWavelengthStop = 730,
            SeqWavelengthStep = 10,
            SeqExposureStart = 50
        };
        private int _targetWavelength = 420;
        private ObservableCollection<string> _wavelengthSequenceNames = new ObservableCollection<string>();

        #endregion Fields

        #region Constructors

        public ControlViewModel()
        {
            IsEnabled = true;
            LoadWavelengthSequences();
            if (null == _wavelengthSequenceNames) return;
            if (_wavelengthSequenceNames.Count > 0)
            {
                CurrentWavelengthSequence = _wavelengthSequenceNames[0];
            }
            if (null != WavelengthSequencesUpdated) WavelengthSequencesUpdated();
        }

        #endregion Constructors

        #region Events

        public event Action UpdateTargetWavelength;

        public event Action WavelengthSequencesUpdated;

        public event Action WavelengthStartStopUpdated;

        #endregion Events

        #region Properties

        public ICommand AddWavelengthSequenceCommand
        {
            get
            {
                if (_addWavelengthSequenceCommand == null)
                    _addWavelengthSequenceCommand = new RelayCommand(() => AddWavelengthSequence());

                return _addWavelengthSequenceCommand;
            }
        }

        public BandwidthModes BandwidthMode
        {
            get { return _bandwidthMode; }

            set
            {
                if (_bandwidthMode == value) return;
                _bandwidthMode = value;
                OnPropertyChanged("BandwidthMode");
            }
        }

        public ICommand ClearPlotCommand
        {
            get
            {
                if (_clearPlotCommand == null)
                    _clearPlotCommand = new RelayCommand(() => ClearPlot());

                return _clearPlotCommand;
            }
        }

        public double CurrentExposure
        {
            get
            {
                return _currentExposure;
            }
            set
            {
                _currentExposure = value;
                OnPropertyChanged("CurrentExposure");
            }
        }

        public int CurrentWavelength
        {
            get { return _currentWavelength; }

            set
            {
                if (_currentWavelength == value) return;
                _currentWavelength = value;
                OnPropertyChanged("CurrentWavelength");
            }
        }

        public string CurrentWavelengthSequence
        {
            get
            {
                return _CurrentWavelengthSequence;
            }
            set
            {
                if (value == null) return;

                _CurrentWavelengthSequence = value;
                OnPropertyChanged("CurrentWavelengthSequence");
                UpdateSequenceEditWindow();
            }
        }

        public ICommand DeleteWavelengthSequenceCommand
        {
            get
            {
                return new RelayCommand(() =>
                {
                    if (CurrentWavelengthSequence != null)
                    {
                        string directorypath = Application.Current.Resources["ApplicationSettingsFolder"].ToString() + "\\kurios\\";
                        File.Delete(directorypath + CurrentWavelengthSequence + ".txt");
                    }
                    LoadWavelengthSequences();
                    if (_wavelengthSequenceNames.Count > 0)
                    {
                        CurrentWavelengthSequence = _wavelengthSequenceNames[0];
                    }
                    if (null != WavelengthSequencesUpdated) WavelengthSequencesUpdated();
                });
            }
        }

        public ICommand EditWavelengthSequenceCommand
        {
            get
            {
                if (this._editWavelengthSequenceCommand == null)
                    this._editWavelengthSequenceCommand = new RelayCommandWithParam((x) => WavelengthSequenceEdit());

                return this._editWavelengthSequenceCommand;
            }
        }

        public double ExposureMax
        {
            get
            {
                return _exposureMax;
            }
            set
            {
                _exposureMax = value;
            }
        }

        public double ExposureMin
        {
            get
            {
                return _exposureMin;
            }
            set
            {
                _exposureMin = value;
            }
        }

        public bool IsEnabled
        {
            get { return _isEnabled; }

            set
            {
                if (_isEnabled == value) return;
                _isEnabled = value;
                OnPropertyChanged("IsEnabled");
            }
        }

        public ICommand RecordPlotCommand
        {
            get
            {
                if (_recordPlotCommand == null)
                    _recordPlotCommand = new RelayCommand(() => RecordPlot());

                return _recordPlotCommand;
            }
        }

        //public event Action stepsChanged;
        public SequenceParameter SequenceParameter
        {
            get
            {
                return _sequenceParameter;
            }
            set
            {
                _sequenceParameter = value;

            }
        }

        public int TargetWavelength
        {
            get { return _targetWavelength; }

            set
            {
                if (value > WavelengthRange.Max)
                {
                    _targetWavelength = WavelengthRange.Max;
                }
                else if (value < WavelengthRange.Min)
                {
                    _targetWavelength = WavelengthRange.Min;
                }
                else
                {
                    _targetWavelength = value;
                }
                OnPropertyChanged("TargetWavelength");
            }
        }

        public Range<int> WavelengthRange
        {
            get
            {
                return WavelengthHelper.DeviceWavelengthRange;
            }
            set
            {
                WavelengthHelper.DeviceWavelengthRange = value;
            }
        }

        public ObservableCollection<string> WavelengthSequences
        {
            get
            {
                return _wavelengthSequenceNames;
            }
            set
            {
                _wavelengthSequenceNames = value;
                if (null != WavelengthSequencesUpdated) WavelengthSequencesUpdated();
            }
        }

        #endregion Properties

        #region Methods

        public void AddWavelengthSequence()
        {
            if (SequenceParameter.SeqWavelengthStart == SequenceParameter.SeqWavelengthStop)
            {
                MessageBox.Show("Wavelength start value has to be different from stop value.");
                return;
            }
            SequenceAdd dlg = new SequenceAdd();
            dlg.WavelengthMin = this.WavelengthRange.Min;
            dlg.WavelengthMax = this.WavelengthRange.Max;
            dlg.ExposureDefault = this.CurrentExposure;
            dlg.ExposureMax = this.ExposureMax;
            dlg.ExposureMin = this.ExposureMin;
            if (true == dlg.ShowDialog())
            {
                foreach (char ic in Path.GetInvalidFileNameChars())
                {
                    if (dlg.SequenceName.Contains(ic))
                    {
                        MessageBox.Show("Invalid file name. Can not contain '" + ic + "'");
                        return;
                    }
                }
                string sequenceFolder = Application.Current.Resources["ApplicationSettingsFolder"].ToString() + "\\kurios\\";
                if (0 == dlg.SequenceName.Length)
                {
                    MessageBox.Show((null == _sequenceEditPlot) ? new Window { Topmost = true } : _sequenceEditPlot, "Sequence name cannot be empty.");
                    return;
                }
                if (!Directory.Exists(sequenceFolder))
                {
                    Directory.CreateDirectory(sequenceFolder);
                }
                if (true == File.Exists(sequenceFolder + dlg.SequenceName + ".txt"))
                {
                    MessageBox.Show((null == _sequenceEditPlot) ? new Window { Topmost = true } : _sequenceEditPlot, "Sequence Ramp name already exists. Choose a unique name.");
                    return;
                }

                StreamWriter fw = new StreamWriter(sequenceFolder + dlg.SequenceName + ".txt", false);

                fw.WriteLine((0).ToString() + "," + dlg.ExposureStart);
                fw.WriteLine((MAX_SEQUENCE_DATA_POINTS).ToString() + "," + dlg.ExposureStop);
                fw.Close();
                string[] sequences = Directory.GetFiles(sequenceFolder, "*.txt");
                WavelengthSequences.Clear();
                int selected = 0;
                LoadWavelengthSequences();
                if (null == WavelengthSequences || WavelengthSequences.Count <= 0) return;
                for (int i = 0; i < WavelengthSequences.Count; i++)
                {
                    if (WavelengthSequences[i].Equals(dlg.SequenceName))
                    {
                        selected = i;
                    }
                }

                CurrentWavelengthSequence = WavelengthSequences[selected];
                if (null != WavelengthSequencesUpdated) WavelengthSequencesUpdated();
                SequenceEdit();
            }
        }

        public void CloseWindows()
        {
            if (null != _sequenceEditPlot && true == _sequenceEditPlot.IsLoaded)
            {
                _sequenceEditPlot.Close();
                _sequenceEditPlot = null;
            }
        }

        public double[] GetWavelengthSequencePlotX(int i)
        {
            if (i >= 0)
            {
                string sequence = Application.Current.Resources["ApplicationSettingsFolder"].ToString() + "\\kurios\\" + WavelengthSequences[i] + ".txt";

                StreamReader fs = new StreamReader(sequence);
                string line;
                int counter = 0;

                List<double> xAxis = new List<double>();

                try
                {
                    while ((line = fs.ReadLine()) != null)
                    {
                        string[] split = line.Split(',');

                        if (split[1] != null)
                        {
                            xAxis.Add(Convert.ToDouble(split[0]));
                        }
                        counter++;
                    }
                }
                catch (Exception ex)
                {
                    string msg = ex.Message;
                }
                fs.Close();
                if (counter <= MAX_SEQUENCE_DATA_POINTS)
                {
                    return xAxis.ToArray();
                }
            }

            return null;
        }

        public double[] GetWavelengthSequencePlotY(int i)
        {
            if (i >= 0)
            {
                string sequence = Application.Current.Resources["ApplicationSettingsFolder"].ToString() + "\\kurios\\" + WavelengthSequences[i] + ".txt";

                StreamReader fs = new StreamReader(sequence);
                string line;
                int counter = 0;

                List<double> yAxis = new List<double>();

                try
                {
                    while ((line = fs.ReadLine()) != null)
                    {
                        string[] split = line.Split(',');

                        if (split[1] != null)
                        {
                            yAxis.Add(Convert.ToDouble(split[1]));
                        }
                        counter++;
                    }
                }
                catch (Exception ex)
                {
                    string msg = ex.Message;
                }
                fs.Close();
                if (counter <= MAX_SEQUENCE_DATA_POINTS)
                {
                    return yAxis.ToArray();
                }
            }

            return null;
        }

        private void ClearPlot()
        {
            int i = WavelengthSequences.IndexOf(CurrentWavelengthSequence);

            if (i >= 0)
            {

                if (MessageBoxResult.Yes == MessageBox.Show((null == _sequenceEditPlot) ? new Window { Topmost = true } : _sequenceEditPlot, "Are you sure you want to clear all of the data points?", "Clear Data Points?", MessageBoxButton.YesNo, MessageBoxImage.Question, MessageBoxResult.Yes))
                {
                    try
                    {
                        string directorypath = Application.Current.Resources["ApplicationSettingsFolder"].ToString() + "\\kurios\\";
                        File.Delete(directorypath + CurrentWavelengthSequence + ".txt");
                        string sequencePath = directorypath + CurrentWavelengthSequence + ".txt";

                        File.Create(sequencePath).Close();
                        System.Threading.Thread.Sleep(50);

                    }
                    catch (Exception e)
                    {
                        e.ToString();
                        System.Threading.Thread.Sleep(50);
                    }
                    SequenceEdit();
                }
            }
        }

        private void LoadWavelengthSequences()
        {
            string sequenceFolder = Application.Current.Resources["ApplicationSettingsFolder"].ToString() + "\\kurios\\";
            if (!Directory.Exists(sequenceFolder))
            {
                Directory.CreateDirectory(sequenceFolder);
            }
            WavelengthSequences = new ObservableCollection<string>();
            string[] sequences = Directory.GetFiles(sequenceFolder, "*.txt");
            for (int i = 0; i < sequences.Length; i++)
            {
                WavelengthSequences.Add(Path.GetFileNameWithoutExtension(sequences[i]));
            }
            OnPropertyChanged("WavelengthSequences");
            if (null != WavelengthSequencesUpdated) WavelengthSequencesUpdated();
        }

        private void RecordPlot()
        {
            //only continue if edit window is up
            if (null == _sequenceEditPlot || false == _sequenceEditPlot.IsLoaded) return;

            // Update the TargetWavelenght value, when starting the application or switching tabs it resets to 0
            if (null != UpdateTargetWavelength) UpdateTargetWavelength();

            // Make sure that the saved value _targetWavelength is in range
            if (TargetWavelength < WavelengthRange.Min || TargetWavelength > WavelengthRange.Max)
            {
                TargetWavelength = (int)SequenceParameter.SeqWavelengthStart;
            }

            //if the user has done a Clear All and tries to add two new points, set those new points to be the new start and stop
            if (0 == _sequenceEditPlot.DataX.Length)
            {
                SequenceParameter.SeqWavelengthStart = TargetWavelength;
                if (null != WavelengthStartStopUpdated) WavelengthStartStopUpdated();
            }
            else if (1 == _sequenceEditPlot.DataX.Length && _sequenceEditPlot.DataX[0] != TargetWavelength)
            {
                SequenceParameter.SeqWavelengthStop = TargetWavelength;
                if (null != WavelengthStartStopUpdated) WavelengthStartStopUpdated();
            }

            //Ensure the settings are within range, or move them to accomodate for current values
            if (SequenceParameter.SeqWavelengthStop > SequenceParameter.SeqWavelengthStart &&
                TargetWavelength < SequenceParameter.SeqWavelengthStart)
            {
                SequenceParameter.SeqWavelengthStart = TargetWavelength;
                if (null != WavelengthStartStopUpdated) WavelengthStartStopUpdated();
            }
            else if (SequenceParameter.SeqWavelengthStop > SequenceParameter.SeqWavelengthStart &&
                TargetWavelength > SequenceParameter.SeqWavelengthStop)
            {
                SequenceParameter.SeqWavelengthStop = TargetWavelength;
                if (null != WavelengthStartStopUpdated) WavelengthStartStopUpdated();
            }
            else if (SequenceParameter.SeqWavelengthStop <= SequenceParameter.SeqWavelengthStart &&
                TargetWavelength < SequenceParameter.SeqWavelengthStop)
            {
                SequenceParameter.SeqWavelengthStop = TargetWavelength;
                if (null != WavelengthStartStopUpdated) WavelengthStartStopUpdated();
            }
            else if (SequenceParameter.SeqWavelengthStop <= SequenceParameter.SeqWavelengthStart &&
                TargetWavelength > SequenceParameter.SeqWavelengthStart)
            {
                SequenceParameter.SeqWavelengthStart = TargetWavelength;
                if (null != WavelengthStartStopUpdated) WavelengthStartStopUpdated();
            }

            double step = (SequenceParameter.SeqWavelengthStop - SequenceParameter.SeqWavelengthStart) / MAX_SEQUENCE_DATA_POINTS;

            double currentPosition = (step >= 0) ? Math.Max(TargetWavelength, SequenceParameter.SeqWavelengthStart) : Math.Min(TargetWavelength, SequenceParameter.SeqWavelengthStart);

            if (null != CurrentWavelengthSequence)
            {
                string sequence = Application.Current.Resources["ApplicationSettingsFolder"].ToString() + "\\kurios\\" + CurrentWavelengthSequence + ".txt";

                int counter = 0;

                double[] dataX = _sequenceEditPlot.DataX;
                double[] dataY = _sequenceEditPlot.DataY;
                SortedDictionary<double, double> realData = new SortedDictionary<double, double>();
                for (int i = 0; i < dataX.Length; i++)
                {
                    realData.Add(dataX[i], dataY[i]);
                }

                //avoid inserting the same key to the dictionary, which would cause a crash
                if (realData.ContainsKey(TargetWavelength))
                {
                    realData[(double)TargetWavelength] = CurrentExposure;
                }
                else
                {
                    realData.Add(TargetWavelength, CurrentExposure);
                }

                SortedDictionary<double, double> xyData = new SortedDictionary<double, double>();
                try
                {
                    if (0 == step)
                    {
                        step = 1.0;
                    }
                    foreach (KeyValuePair<double, double> p in realData)
                    {
                        Decimal dec = new Decimal(Math.Abs((p.Key - SequenceParameter.SeqWavelengthStart) / step));

                        int loc = Math.Min(Convert.ToInt32(Decimal.Round(dec)), MAX_SEQUENCE_DATA_POINTS);
                        //avoid inserting the same key to the dictionary, which would throw an exception and all the remaining points wouldn't be added
                        if (!xyData.ContainsKey(loc))
                        {
                            xyData.Add(loc, p.Value);
                        }
                    }
                }
                catch (Exception ex)
                {
                    string msg = ex.Message;
                }

                if (counter <= MAX_SEQUENCE_DATA_POINTS)
                {

                    StreamWriter fw = new StreamWriter(sequence, false);

                    foreach (KeyValuePair<double, double> p in xyData)
                    {
                        fw.WriteLine(p.Key.ToString() + ',' + p.Value.ToString());
                    }

                    fw.Close();
                }
            }

            SequenceEdit();
        }

        private void SequenceEdit()
        {
            if (WavelengthSequences.Count <= 0)
            {
                return;
            }
            int i = WavelengthSequences.IndexOf(CurrentWavelengthSequence);
            if ((null == _sequenceEditPlot) || (false == _sequenceEditPlot.IsLoaded))
            {
                _sequenceEditPlot = new WavelengthExposureEditWindow();
                _sequenceEditPlot.Closed += _sequenceEditPlot_Closed;
                _sequenceEditPlot.DataContext = this;
                _sequenceEditPlot.Title = string.Format("Hyperspectral Sequence - {0}", CurrentWavelengthSequence);
                _sequenceEditPlot.Owner = Application.Current.MainWindow;
                _sequenceEditPlot.SetData(GetWavelengthSequencePlotX(i), GetWavelengthSequencePlotY(i), true);
                _sequenceEditPlot.Show();
            }
            else
            {
                //the plot is already created. update the data
                _sequenceEditPlot.SetData(GetWavelengthSequencePlotX(i), GetWavelengthSequencePlotY(i), true);
            }
        }

        private void UpdateSequenceEditWindow()
        {
            int i = WavelengthSequences.IndexOf(CurrentWavelengthSequence);

            //only update when edit window is being displayed
            if ((null != _sequenceEditPlot) && (false != _sequenceEditPlot.IsLoaded))
            {
                _sequenceEditPlot.Title = string.Format("Hyperspectral Sequence - {0}", CurrentWavelengthSequence);
                _sequenceEditPlot.SetData(GetWavelengthSequencePlotX(i), GetWavelengthSequencePlotY(i), true);
            }
        }

        private void WavelengthSequenceEdit()
        {
            if (WavelengthSequences.Count <= 0)
            {
                return;
            }

            if (null == _CurrentWavelengthSequence)
            {
                MessageBox.Show((null == _sequenceEditPlot) ? new Window { Topmost = true } : _sequenceEditPlot, "No Sequence selected. Please select a sequence to edit.");
            }

            int selectedIndex = WavelengthSequences.IndexOf(_CurrentWavelengthSequence);

            if ((_sequenceEditPlot == null) || (_sequenceEditPlot.IsLoaded == false))
            {
                _sequenceEditPlot = new WavelengthExposureEditWindow();
                _sequenceEditPlot.Closed += _sequenceEditPlot_Closed;
                _sequenceEditPlot.DataContext = this;
                _sequenceEditPlot.Title = string.Format("Hyperspectral Sequence - {0}", CurrentWavelengthSequence);
                _sequenceEditPlot.SetData(GetWavelengthSequencePlotX(selectedIndex), GetWavelengthSequencePlotY(selectedIndex), true);
                _sequenceEditPlot.Owner = Application.Current.MainWindow;
                _sequenceEditPlot.Show();
            }
            else
            {    //the plot is already created. update the data
                _sequenceEditPlot.SetData(GetWavelengthSequencePlotX(selectedIndex), GetWavelengthSequencePlotY(selectedIndex), true);
            }
        }

        void _sequenceEditPlot_Closed(object sender, EventArgs e)
        {
            _sequenceEditPlot = null;
        }

        #endregion Methods
    }
}