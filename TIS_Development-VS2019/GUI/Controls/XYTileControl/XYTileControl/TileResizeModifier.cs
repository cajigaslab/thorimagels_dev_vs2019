namespace XYTileControl.CustomModifiers
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Diagnostics;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Shapes;
    using System.Xml;

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
    using SciChart.Core;
    using SciChart.Core.Utility.Mouse;
    using SciChart.Drawing.HighSpeedRasterizer;
    using SciChart.Drawing.VisualXcceleratorRasterizer;

    class TileResizeModifier : ChartModifierBase
    {
        #region Fields

        public static DependencyProperty ResizeEnabledProperty = 
        DependencyProperty.RegisterAttached("ResizeEnabled",
        typeof(bool),
        typeof(ChartDragSelectModifier),
        null);

        private BoxAnnotation _resizedTile = null;

        #endregion Fields

        #region Events

        public event Action FinishResizingEvent;

        #endregion Events

        #region Properties

        public bool ResizeEnabled
        {
            get { return (bool)GetValue(ResizeEnabledProperty); }
            set { SetValue(ResizeEnabledProperty, value); }
        }

        #endregion Properties

        #region Methods

        public void Clear()
        {
            if (_resizedTile!=null)
            {
                _resizedTile = null;
            }
            ResizeEnabled = false;
        }

        public BoxAnnotation GetActiveTile()
        {
            return _resizedTile;
        }

        public override void OnModifierMouseUp(ModifierMouseArgs e)
        {
            if (ResizeEnabled && _resizedTile!=null)
            {
                var ptTrans = GetPointRelativeTo(e.MousePoint, ModifierSurface);
                var xCalc = this.XAxis.GetCurrentCoordinateCalculator();
                var yCalc = this.YAxis.GetCurrentCoordinateCalculator();

                double x = xCalc.GetCoordinate(((double)_resizedTile.X1 + (double)_resizedTile.X2) / 2.0);
                double y = yCalc.GetCoordinate(((double)_resizedTile.Y1 + (double)_resizedTile.Y2) / 2.0);

                if (Math.Abs(ptTrans.X - x) > _resizedTile.Width / 2.0 || Math.Abs(ptTrans.Y - y) > _resizedTile.Height / 2.0)
                {
                    _resizedTile.IsSelected = false;
                    _resizedTile.IsEditable = false;
                    ResizeEnabled = false;
                    if (FinishResizingEvent != null)
                    {
                        FinishResizingEvent();
                        return;
                    }
                }
            }
        }

        public void SetActiveTile(BoxAnnotation resizedTile)
        {
            _resizedTile = resizedTile;
        }

        #endregion Methods
    }
}