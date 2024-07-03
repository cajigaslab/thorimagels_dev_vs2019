namespace FLIMFitting.ViewModels
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Diagnostics;
    using System.IO;
    using System.Linq;
    using System.Text;
    using System.Threading;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Data;
    using System.Windows.Input;
    using System.Windows.Media;

    using FLIMFitting;
    using FLIMFitting.Model;

    using Microsoft.Win32;

    using ThorLogging;

    public partial class FLIMFittingViewModel : BindableBase
    {
        #region Fields

        private static readonly object _dflimHistogramGroupsDataLock = new object();

        private bool _alwaysAutoFit = false;
        private bool _autoFitOnce = false;
        private int _circShift;
        List<FLIMHistogramGroupData> _dflimHistogramGroups;
        private string _filePath;
        private ObservableCollection<FLIMDataGroup> _flimDataGroups = new ObservableCollection<FLIMDataGroup>();
        private Thread _flimHistogramsUpdateThread;
        private bool _newGroupDataSet = false;
        private bool _runFLIMHistogramsUpdateThread = true;
        private FLIMDataGroup _selectedGroup;

        #endregion Fields

        #region Constructors

        public FLIMFittingViewModel()
        {
            LoadFileCommand = new RelayCommand(LoadFileCommandExecute);
            RefreshCommand = new RelayCommand(RefreshCommandExecute);
            PrefitAllCommand = new RelayCommand(PrefitAllCommandExecute);
            RefitAllCommand = new RelayCommand(RefitAllCommandExecute);
            PrefitGroupCommand = new RelayCommand(PrefitGroupCommandExecute);
            RefitGroupCommand = new RelayCommand(RefitGroupCommandExecute);
            UpdateTZeroCommand = new RelayCommand(UpdateTZeroCommandExecute);
            _flimHistogramsUpdateThread = new Thread(FLIMHistogramsUpdate);

            System.Windows.Application.Current.Exit += Current_Exit;
        }

        #endregion Constructors

        #region Events

        public event Action<Dictionary<int, double>> UpdateTZero;

        #endregion Events

        #region Properties

        public bool AlwaysAutoFit
        {
            get
            {
                return _alwaysAutoFit;
            }
            set
            {
                SetProperty(ref _alwaysAutoFit, value);

            }
        }

        public bool AutoFitOnce
        {
            get
            {
                return _autoFitOnce;
            }
            set
            {
                SetProperty(ref _autoFitOnce, value);
            }
        }

        public int CircShift
        {
            get { return _circShift; }
            set
            {
                if (_circShift == value) return;
                SetProperty(ref _circShift, value);
            }
        }

        public string FilePath
        {
            get { return _filePath; }
            set { SetProperty(ref _filePath, value); }
        }

        public ObservableCollection<FLIMDataGroup> FlimDataGroups
        {
            get { return _flimDataGroups; }
            set
            {
                if (_flimDataGroups == value) return;
                SetProperty(ref _flimDataGroups, value);
            }
        }

        public List<FLIMHistogramGroupData> FLIMHistogramGroups
        {
            set
            {
                lock (_dflimHistogramGroupsDataLock)
                {
                    _dflimHistogramGroups = value;

                    _newGroupDataSet = true;
                }
            }
        }

        public ICommand LoadFileCommand
        {
            get; set;
        }

        public ICommand PrefitAllCommand
        {
            get; set;
        }

        public ICommand PrefitGroupCommand
        {
            get; set;
        }

        public ICommand RefitAllCommand
        {
            get; set;
        }

        public ICommand RefitGroupCommand
        {
            get; set;
        }

        public ICommand RefreshCommand
        {
            get; set;
        }

        public FLIMDataGroup SelectedGroup
        {
            get { return _selectedGroup; }
            set
            {
                if (_selectedGroup == value) return;
                SetProperty(ref _selectedGroup, value);
            }
        }

        public ICommand UpdateTZeroCommand
        {
            get;
            set;
        }

        #endregion Properties

        #region Methods

        public void StartFlimHistogramUpdateThread()
        {
            _runFLIMHistogramsUpdateThread = true;
            _flimHistogramsUpdateThread.Start();
        }

        public void StopFlimHistogramUpdateThread()
        {
            _runFLIMHistogramsUpdateThread = false;
            _flimHistogramsUpdateThread.Abort();
        }

        void Current_Exit(object sender, ExitEventArgs e)
        {
            StopFlimHistogramUpdateThread();
        }

        void FLIMHistogramsUpdate()
        {
            do
            {
                if (_newGroupDataSet)
                {
                    lock (_dflimHistogramGroupsDataLock)
                    {
                        try
                        {
                            if (null != _dflimHistogramGroups && _dflimHistogramGroups.Count > 0)
                            {
                                bool groupSelected = false;
                                Application.Current.Dispatcher.Invoke(() =>
                                {
                                    for (int i = FlimDataGroups.Count - 1; i >= 0; --i)
                                    {
                                        var existingGroup = _dflimHistogramGroups.FirstOrDefault(c => c.GroupName == FlimDataGroups[i].Name);
                                        if (null == existingGroup)
                                        {
                                            FlimDataGroups.RemoveAt(i);
                                        }
                                    }
                                    foreach (var hgroup in _dflimHistogramGroups)
                                    {
                                        var group = FlimDataGroups.FirstOrDefault(c => c.Name == hgroup.GroupName);
                                        if (null == group)
                                        {
                                            group = new FLIMDataGroup(hgroup.GroupName);
                                            group.LoadData(hgroup);
                                            FlimDataGroups.Add(group);
                                        }
                                        else
                                        {
                                            group.LoadData(hgroup);
                                        }
                                        if (SelectedGroup == group)
                                        {
                                            groupSelected = true;
                                        }
                                    }

                                    if (!groupSelected)
                                    {
                                        SelectedGroup = FlimDataGroups[0];
                                    }

                                    if (AutoFitOnce || AlwaysAutoFit)
                                    {
                                        PrefitAllCommandExecute();

                                        //Turn off the updateTZero when autofit once is set to true for now
                                        //if (AutoFitOnce)
                                        //{
                                        //    UpdateTZeroCommandExecute();
                                        //}
                                        AutoFitOnce = false;
                                    }
                                });
                                _newGroupDataSet = false;
                            }
                        }
                        catch (Exception ex)
                        {
                            ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, this.GetType().Name + " FLIMHistogramsUpdate " + ex.ToString());
                            ex.ToString();
                        }
                    }
                }

                Thread.Sleep(100);
            } while (_runFLIMHistogramsUpdateThread);
        }

        private void LoadFileCommandExecute()
        {
            var dlg = new OpenFileDialog();
            dlg.Filter = "Flim data files (*.bin)|*.bin";
            if (!string.IsNullOrEmpty(FilePath))
            {
                if (Directory.Exists(Path.GetDirectoryName(FilePath)))
                    dlg.InitialDirectory = Path.GetDirectoryName(FilePath);
            }

            if (dlg.ShowDialog(Application.Current.MainWindow) == true)
            {
                FilePath = dlg.FileName;
            }
            FlimDataGroups.Clear();
            var name = Path.GetFileNameWithoutExtension(FilePath);
            var group = new FLIMDataGroup(name);
            if (!group.LoadData(FilePath))
            {
                MessageBox.Show("load failed");
                return;
            }
            FlimDataGroups.Add(group);
            SelectedGroup = group;

            //group = new FLIMDataGroup("ChB");
            //group.LoadData(FilePath);
            //FlimDataGroups.Add(group);

            //group = new FLIMDataGroup("ChC");
            //group.LoadData(FilePath);
            //FlimDataGroups.Add(group);

            //group = new FLIMDataGroup("ChD");
            //group.LoadData(FilePath);
            //FlimDataGroups.Add(group);
        }

        private void PrefitAllCommandExecute()
        {
            foreach (var g in FlimDataGroups)
            {
                g.PreFit();
            }
        }

        private void PrefitGroupCommandExecute()
        {
            if (SelectedGroup != null)
                SelectedGroup.PreFit();
        }

        private void RefitAllCommandExecute()
        {
            foreach (var g in FlimDataGroups)
            {
                g.Refit();
            }
        }

        private void RefitGroupCommandExecute()
        {
            if (SelectedGroup != null)
                SelectedGroup.Refit();
        }

        private void RefreshCommandExecute()
        {
            foreach (var g in FlimDataGroups)
            {
                g.Refresh();
            }
        }

        private void UpdateTZeroCommandExecute()
        {
            var tZeroDictionary = new Dictionary<int, double>();

            if (null == FlimDataGroups || FlimDataGroups.Count <= 0)
            {
                return;
            }

            foreach (var group in FlimDataGroups)
            {
                if (null == group.FlimDataList || group.FlimDataList.Count <= 0)
                {
                    continue;
                }
                foreach (var flimData in group.FlimDataList)
                {
                    if (flimData.IsWholeChannelHistogramData)
                    {
                        tZeroDictionary.Add(flimData.Channel, flimData.TZero);
                        if (HistogramGroupType.ChannelHistogramAndROIs == group.GroupType)
                        {
                            break;
                        }
                    }
                }
            }

            if (null != UpdateTZero)
            {
                UpdateTZero(tZeroDictionary);
            }
        }

        #endregion Methods
    }
}