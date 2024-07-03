// taken from SciChart example code
namespace HistogramControl.ViewModel
{
    using System;
    using System.Collections.Generic;

    using SciChart.Charting.Numerics.TickProviders;
    using SciChart.Charting.Visuals.Axes;
    using SciChart.Data.Model;

    class ConstantTickProvider : NumericTickProvider
    {
        #region Properties

        public double LogBase
        {
            get; set;
        }

        public int MajorTickCount
        {
            get; set;
        }

        #endregion Properties

        #region Methods

        public override IList<double> GetMajorTicks(IAxisParams axis)
        {
            var visibleRange = (DoubleRange)axis.VisibleRange;
            var ticks = new double[MajorTickCount + 1];

            if(LogBase > 0)
            {
                double minLog = Math.Log(visibleRange.Min, LogBase);
                double maxLog = Math.Log(visibleRange.Max, LogBase);
                double step = (Math.Abs(maxLog) + Math.Abs(minLog)) / MajorTickCount;

                for (int i = 0; i < MajorTickCount + 1; i++)
                {
                    var nextStep = step * i + minLog;
                    ticks[i] = Math.Pow(LogBase, nextStep);
                }
            }
            else
            {
                var step = visibleRange.Diff / MajorTickCount;
                var min = visibleRange.Min;

                for (int i = 0; i < MajorTickCount + 1; i++)
                {
                    ticks[i] = min + step * i;
                }
            }

            ticks[0] = visibleRange.Min;
            ticks[MajorTickCount] = visibleRange.Max;

            return ticks;
        }

        public override IList<double> GetMinorTicks(IAxisParams axis)
        {
            return new double[0];
        }

        #endregion Methods
    }
}