namespace FLIMFitting.Model
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.IO;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows.Data;
    using System.Windows.Media;

    using FLIMFitting.Utils;

    using SciChart;
    using SciChart.Charting;
    using SciChart.Charting.ChartModifiers;
    using SciChart.Charting.Common.Extensions;
    using SciChart.Charting.Model.DataSeries;
    using SciChart.Charting.Themes;
    using SciChart.Charting.Visuals;
    using SciChart.Charting.Visuals.Annotations;
    using SciChart.Charting.Visuals.Axes;
    using SciChart.Charting.Visuals.Axes.LabelProviders;
    using SciChart.Charting.Visuals.Events;
    using SciChart.Charting.Visuals.PointMarkers;
    using SciChart.Charting.Visuals.RenderableSeries;
    using SciChart.Core;
    using SciChart.Data.Model;
    using SciChart.Drawing.HighSpeedRasterizer;
    using SciChart.Drawing.Utility;
    using SciChart.Drawing.VisualXcceleratorRasterizer;

    public class FLIMDataGroup : BindableBase
    {
        #region Fields

        private List<IRenderableSeries> _binHistogramSeries = new List<IRenderableSeries>();
        private ObservableCollection<FLIMData> _deDataList = new ObservableCollection<FLIMData>();
        private FitParam _doubleExponentialParam = new FitParam();
        private ObservableCollection<FLIMData> _flimDataList = new ObservableCollection<FLIMData>();
        private HistogramGroupType _groupType;
        private bool _hasPreFit = false;

        //double exponential
        private bool _isDE = true;
        private bool _isMultiData = true;
        private string _name;
        private IRange _photonRange = new DoubleRange(1, 10000);
        private IRange _residualsRange = new DoubleRange(-200, 200);
        private List<IRenderableSeries> _resultLineSeries = new List<IRenderableSeries>();
        private ObservableCollection<FLIMData> _seDataList = new ObservableCollection<FLIMData>();
        private FitParam _singleExponentialParam = new FitParam
        {
            IsAmp2Fixed = true,
            IsTua2Fixed = true
        };

        #endregion Fields

        #region Constructors

        public FLIMDataGroup(string name)
        {
            BinSeries = new ObservableCollection<IRenderableSeries>();
            ResidualsSeries = new ObservableCollection<IRenderableSeries>();
            Name = name;
        }

        #endregion Constructors

        #region Properties

        public ObservableCollection<IRenderableSeries> BinSeries
        {
            get; set;
        }

        public FitParam FitParam
        {
            get
            {
                if (_isDE) return _doubleExponentialParam;
                else { return _singleExponentialParam; }
            }
        }

        public ObservableCollection<FLIMData> FlimDataList
        {
            get { return _flimDataList; }
            set { }
        }

        public HistogramGroupType GroupType
        {
            get
            {
                return _groupType;
            }
            set
            {
                if (_groupType == value) return;
                SetProperty(ref _groupType, value);
            }
        }

        public bool HasPreFit
        {
            get { return _hasPreFit; }
            set
            {
                if (_hasPreFit == value) return;
                SetProperty(ref _hasPreFit, value);
            }
        }

        public bool IsDE
        {
            get { return _isDE; }
            set
            {
                if (_isDE == value) return;
                SetProperty(ref _isDE, value);
                OnPropertyChanged("FitParam");
                if (_isDE)
                {
                    _flimDataList = _deDataList;
                    _doubleExponentialParam.Reset();
                }
                else
                {
                    _flimDataList = _seDataList;
                    _singleExponentialParam.Reset();
                }
                PreFit();
                //UpdateResult();
                OnPropertyChanged("FlimDataList");
            }
        }

        public bool IsMultiData
        {
            get { return _isMultiData; }
            set
            {
                if (_isMultiData == value) return;
                SetProperty(ref _isMultiData, value);
            }
        }

        public string Name
        {
            get { return _name; }
            set
            {
                if (_name == value) return;
                SetProperty(ref _name, value);
            }
        }

        public IRange PhotonRange
        {
            get { return _photonRange; }
            set
            {
                if (_photonRange == value) return;
                SetProperty(ref _photonRange, value);
            }
        }

        public IRange ResidualsRange
        {
            get { return _residualsRange; }
            set
            {
                if (_residualsRange == value) return;
                SetProperty(ref _residualsRange, value);
            }
        }

        public ObservableCollection<IRenderableSeries> ResidualsSeries
        {
            get; set;
        }

        #endregion Properties

        #region Methods

        public bool LoadData(string path)
        {
            var flimList = DataReader.ReadFLIMData(path);
            return LoadData(flimList);
        }

        public bool LoadData(FLIMHistogramGroupData data)
        {
            if (null == data || data.Histograms.Count <= 0 ||
                data.Histograms.Count != data.Channels.Count ||
                data.Histograms.Count != data.Colors.Count ||
                data.Histograms.Count != data.HistrogramNames.Count
                )
            {
                return false;
            }

            List<FLIMData> flimList = new List<FLIMData>();
            for (int i = 0; i < data.Histograms.Count; ++i)
            {
                FLIMData flimData = new FLIMData();
                flimData.ID = data.HistrogramNames[i];
                flimData.Channel = data.Channels[i];
                flimData.IsWholeChannelHistogramData = true;
                flimData.IsSelected = true;
                flimData.RenderColor = data.Colors[i].Color;
                flimData.DataArray = data.Histograms[i];
                flimData.NsPerPoint = 5.0 * (double)data.Histograms[i][255] / 128.0 / (double)data.Histograms[i][254];
                flimList.Add(flimData);
            }
            GroupType = data.GroupType;
            return LoadData(flimList);
        }

        public void PreFit()
        {
            var list = _flimDataList.ToList();
            FLIMFitLibWrapper.PreFit(list, (byte)FitParam.FixedFlag, (byte)FitParam.SharedFlg, IsDE);
            if (FitParam.SharedFlg > 0)
            {
                FLIMFitLibWrapper.FitMulti(list, (byte)FitParam.FixedFlag, (byte)FitParam.SharedFlg, IsDE);
            }
            FLIMFitLibWrapper.GetResult(list, (byte)FitParam.SharedFlg, IsDE);
            UpdateResult();
            HasPreFit = true;
        }

        public void Refit()
        {
            if (!HasPreFit) return;
            var list = _flimDataList.ToList();
            if (FitParam.SharedFlg > 0)
            {
                FLIMFitLibWrapper.FitMulti(list, (byte)FitParam.FixedFlag, (byte)FitParam.SharedFlg, IsDE);
            }
            else
            {
                FLIMFitLibWrapper.Fit(list, (byte)FitParam.FixedFlag, (byte)FitParam.SharedFlg, IsDE);
            }
            FLIMFitLibWrapper.GetResult(list, (byte)FitParam.SharedFlg, IsDE);
            UpdateResult();
        }

        public void Refresh()
        {
            foreach (var r in _resultLineSeries)
            {
                BinSeries.Remove(r);
            }
            ResidualsSeries.Clear();
            for (int i = 0; i < _flimDataList.Count; i++)
            {
                var residualsSerie = new XyDataSeries<double, double>() { SeriesName = string.Format("Residuals {0}", i + 1) };
                var flimData = _flimDataList[i];
                for (int j = 0; j < FLIMData.DATA_SIZE; j++)
                {
                    var t = flimData.NsPerPoint * j;
                    residualsSerie.Append(t, 0);
                }
                var binding = new Binding("IsSelected")
                {
                    Source = flimData,
                    Mode = BindingMode.TwoWay,
                };
                var fs = new FastLineRenderableSeries
                {
                    DataSeries = residualsSerie,
                    Stroke = flimData.RenderColor,
                    StrokeThickness = 2
                };
                BindingOperations.SetBinding(fs, FastLineRenderableSeries.IsVisibleProperty, binding);
                ResidualsSeries.Add(fs);
            }
        }

        private bool LoadData(List<FLIMData> flimList)
        {
            if (flimList == null || flimList.Count < 1)
            {
                return false;
            }
            HasPreFit = false;

            Dictionary<string, bool> isVisibleDictionary = new Dictionary<string, bool>();
            for (int i = 0; i < BinSeries?.Count; ++i)
            {
                if (false == isVisibleDictionary.ContainsKey(BinSeries[i].DataSeries.SeriesName))
                {
                    isVisibleDictionary.Add(BinSeries[i].DataSeries.SeriesName, BinSeries[i].IsVisible);
                }
            }

            foreach (var b in _binHistogramSeries)
            {
                BinSeries.Remove(b);
            }
            _binHistogramSeries.Clear();

            if (_deDataList.Count != flimList.Count)
            {
                _deDataList.Clear();

                for (int i = 0; i < flimList.Count; ++i)
                {
                    _deDataList.Add(flimList[i]);
                }
            }
            else
            {
                for (int i = 0; i < flimList.Count; ++i)
                {
                    _deDataList[i].ID = flimList[i].ID;
                    _deDataList[i].Channel = flimList[i].Channel;
                    _deDataList[i].IsWholeChannelHistogramData = flimList[i].IsWholeChannelHistogramData;
                    _deDataList[i].IsSelected = flimList[i].IsSelected;
                    _deDataList[i].RenderColor = flimList[i].RenderColor;
                    _deDataList[i].DataArray = flimList[i].DataArray;
                    _deDataList[i].NsPerPoint = flimList[i].NsPerPoint;
                }
            }

            _seDataList.Clear();
            foreach (var fd in flimList)
            {
                _seDataList.Add(fd.ShallowCopy());
            }

            _flimDataList = _deDataList;
            IsMultiData = _flimDataList.Count > 1;
            double maxPhoton = 0.0;
            double minPhoton = int.MaxValue;

            for (int i = 0; i < _flimDataList.Count; i++)
            {
                var serie = new XyDataSeries<double, double>() { SeriesName = string.Format("Bin {0}", i + 1) };
                var residualsSerie = new XyDataSeries<double, double>() { SeriesName = string.Format("Residuals {0}", i + 1) };
                var flimData = _flimDataList[i];
                for (int j = 0; j < FLIMData.DATA_SIZE; j++)
                {
                    var t = flimData.NsPerPoint * j;
                    serie.Append(t, flimData.DataArray[j]);
                    residualsSerie.Append(t, 0);
                }

                var binding = new Binding("IsSelected")
                {
                    Source = flimData,
                    Mode = BindingMode.TwoWay,
                };

                var renderableSeries = new XyScatterRenderableSeries
                {
                    DataSeries = serie,
                    Name = "Histogram" + i.ToString(),
                    PointMarker = new CrossPointMarker()
                    {
                        Stroke = flimData.RenderColor,
                        StrokeThickness = 2,
                        Fill = flimData.RenderColor,
                        Width = 1.5,
                        Height = 1.5,
                    }
                };

                BindingOperations.SetBinding(renderableSeries, XyScatterRenderableSeries.IsVisibleProperty, binding);
                BinSeries.Add(renderableSeries);
                _binHistogramSeries.Add(renderableSeries);

                if (maxPhoton < (double)serie.YMax)
                {
                    maxPhoton = (double)serie.YMax;
                }

                if (minPhoton > (double)serie.YMin)
                {
                    minPhoton = (double)serie.YMin;
                }
            }

            foreach (var isVisible in isVisibleDictionary)
            {
                for (int i = 0; i < BinSeries?.Count; ++i)
                {
                    if (isVisible.Key == BinSeries[i].DataSeries.SeriesName)
                    {
                        BinSeries[i].IsVisible = isVisible.Value;
                    }
                }
            }

            PhotonRange = new DoubleRange(minPhoton - minPhoton * 1.1, maxPhoton * 1.1);
            return true;
        }

        private void UpdateResult()
        {
            Dictionary<string, bool> isVisibleDictionary = new Dictionary<string, bool>();
            for (int i = 0; i < ResidualsSeries?.Count; ++i)
            {
                isVisibleDictionary.Add(ResidualsSeries[i].DataSeries.SeriesName, ResidualsSeries[i].IsVisible);
            }

            ResidualsSeries.Clear();
            foreach (var r in _resultLineSeries)
            {
                BinSeries.Remove(r);
            }
            _resultLineSeries.Clear();
            double maxResidual = 200;
            double minResidual = -200;
            for (int i = 0; i < _flimDataList.Count; i++)
            {
                var serie = new XyDataSeries<double, double>() { SeriesName = string.Format("Bin {0}", i + 1) };
                var residualsSerie = new XyDataSeries<double, double>() { SeriesName = string.Format("Residuals {0}", i + 1) };
                var flimData = _flimDataList[i];
                for (int j = 0; j < FLIMData.DATA_SIZE; j++)
                {
                    var t = flimData.NsPerPoint * j;
                    serie.Append(t, flimData.FitResultArray[j]);
                    residualsSerie.Append(t, flimData.DataArray[j] - flimData.FitResultArray[j]);
                }
                var renderableSeries = new FastLineRenderableSeries
                {
                    Stroke = flimData.RenderColor,
                    DataSeries = serie,
                    StrokeThickness = 2
                };
                var binding = new Binding("IsSelected")
                {
                    Source = flimData,
                    Mode = BindingMode.TwoWay,
                };
                BindingOperations.SetBinding(renderableSeries, FastLineRenderableSeries.IsVisibleProperty, binding);

                BinSeries.Add(renderableSeries);
                _resultLineSeries.Add(renderableSeries);

                var fs = new FastLineRenderableSeries
                {
                    DataSeries = residualsSerie,
                    Stroke = flimData.RenderColor,
                    StrokeThickness = 2
                };
                BindingOperations.SetBinding(fs, FastLineRenderableSeries.IsVisibleProperty, binding);
                ResidualsSeries.Add(fs);
                if (maxResidual < (double)residualsSerie.YMax &&
                    !double.IsInfinity((double)residualsSerie.YMax))
                {
                    maxResidual = (double)residualsSerie.YMax;
                }
                if (minResidual > (double)residualsSerie.YMin &&
                    !double.IsInfinity((double)residualsSerie.YMin))
                {
                    minResidual = (double)residualsSerie.YMin;
                }
            }

            foreach (var isVisible in isVisibleDictionary)
            {
                for (int i = 0; i < ResidualsSeries?.Count; ++i)
                {
                    if (isVisible.Key == ResidualsSeries[i].DataSeries.SeriesName)
                    {
                        ResidualsSeries[i].IsVisible = isVisible.Value;
                    }
                }
            }

            ResidualsRange = new DoubleRange(Math.Floor(minResidual), Math.Ceiling(maxResidual));
        }

        #endregion Methods
    }
}