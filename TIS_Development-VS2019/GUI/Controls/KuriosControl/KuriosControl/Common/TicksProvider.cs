namespace KuriosControl.Common
{
    using System;
    using System.Collections.Generic;

    internal class TicksProvider
    {
        #region Fields

        private double minStep = 0.0;

        #endregion Fields

        #region Properties

        public double MinStep
        {
            get { return minStep; }
            set
            {
                if (minStep != value)
                {
                    minStep = value;
                }
            }
        }

        #endregion Properties

        #region Methods

        public List<double> GetTicks(Range<int> range, int ticksCount)
        {
            double start = range.Min;
            double finish = range.Max;

            double delta = finish - start;

            int log = (int)Math.Round(Math.Log10(delta));

            double newStart = RoundHelper.Round(start, log);
            double newFinish = RoundHelper.Round(finish, log);

            if (newStart == newFinish)
            {
                log--;
                newStart = RoundHelper.Round(start, log);
                newFinish = RoundHelper.Round(finish, log);
            }

            double unroundedStep = (newFinish - newStart) / ticksCount;
            int stepLog = log;
            double step = RoundHelper.Round(unroundedStep, stepLog);
            if (step == 0)
            {
                stepLog--;
                step = RoundHelper.Round(unroundedStep, stepLog);
                if (step == 0)
                {
                    step = unroundedStep;
                }
            }

            if (step < minStep)
                step = minStep;

            List<double> ticks = new List<double>();
            if (step != 0.0)
            {
                ticks = CreateTicks(start, finish, step);

                if (ticks.Count > 0)
                {
                    //if (ticks[0] > range.Min)
                    //{
                    //    ticks.Insert(0, range.Min);
                    //}
                    //else
                    ticks[0] = range.Min;

                    //if (ticks[ticks.Count - 1] < range.Max)
                    //{
                    //    ticks.Add(range.Max);
                    //}
                    //else
                    {
                        ticks[ticks.Count - 1] = range.Max;
                    }
                }
            }

            return ticks;
        }

        private static List<double> CreateTicks(double start, double finish, double step)
        {
            double x = step * Math.Floor(start / step);

            if (x == x + step)
            {
                return new List<double>();
            }

            List<double> ticks = new List<double>();

            while (x <= finish + step * 0.5)
            {
                ticks.Add(x);
                x += step;
            }
            return ticks;
        }

        #endregion Methods
    }
}