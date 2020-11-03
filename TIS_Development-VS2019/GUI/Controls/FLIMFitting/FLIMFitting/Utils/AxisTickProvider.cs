namespace FLIMFitting.Utils
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;

    using Abt.Controls.SciChart.Visuals.Axes;

    public class AxisTickProvider : NumericTickProvider
    {
        #region Constructors

        public AxisTickProvider(bool keepInt = false)
        {
            KeepInt = keepInt;
        }

        public AxisTickProvider()
        {
            TickCount = 3;
            KeepInt = true;
        }

        #endregion Constructors

        #region Properties

        public bool KeepInt
        {
            get; private set;
        }

        public int TickCount
        {
            get; set;
        }

        #endregion Properties

        #region Methods

        public int GetAccuracy(double min, double max)
        {
            decimal delta = (decimal)((max - min) * 1.0 / (TickCount - 1));
            var indexMax = max.ToString().IndexOf(".", StringComparison.Ordinal);
            var indexMin = min.ToString().IndexOf(".", StringComparison.Ordinal);
            var maxAxisAccuracy = 0;
            if (indexMin > 0 || indexMax > 0)
            {
                if (indexMin > 0)
                {
                    indexMin = min.ToString().Length - (indexMin + 1);
                }
                if (indexMax > 0)
                {
                    indexMax = max.ToString().Length - (indexMax + 1);
                }
                maxAxisAccuracy = Math.Max(indexMin, indexMax);
            }

            if (TickCount == 2 || delta < 0)
            {
                return maxAxisAccuracy;
            }

            var dS = delta.ToString();
            var indexDel = dS.IndexOf(".", StringComparison.Ordinal);
            if (indexDel > 0)
            {
                indexDel = dS.Length - (indexDel + 1);
            }

            if (TickCount == 3)
            {
                return indexDel;
            }

            if (delta >= 1)
            {
                if (maxAxisAccuracy < indexDel)
                {
                    return maxAxisAccuracy + 1;
                }
                else
                {
                    return maxAxisAccuracy;
                }
            }
            else
            {
                int accuracy = maxAxisAccuracy > 0 ? maxAxisAccuracy : 1;
                int index = 1;
                if (indexDel > 0)
                {
                    var r = delta;
                    for (; index <= indexDel; index++)
                    {
                        r *= 10;
                        if (r >= 1)
                        {
                            if (r % 1 != 0)
                            {
                                index++;
                            }
                            break;
                        }
                    }
                    accuracy = accuracy > index ? accuracy : index;
                }
                return accuracy;
            }
        }

        public override double[] GetMajorTicks(IAxisParams axis)
        {
            var max = axis.VisibleRange.AsDoubleRange().Max;
            var min = axis.VisibleRange.AsDoubleRange().Min;
            var count = 0;

            var delta = (max - min) * 1.0 / (TickCount - 1);

            if (!KeepInt)
            {
                count = GetAccuracy(min, max);
            }
            else
            {
                var maxInt = (int)max;
                if (maxInt < 20)
                {
                    if (IsPrime(maxInt))
                    {
                        if (maxInt <= 3)
                        {
                            TickCount = maxInt + 1;
                        }
                        else
                        {
                            TickCount = GetMinDivisors(maxInt + 1) + 1;
                        }
                    }
                    else
                    {
                        TickCount = GetMinDivisors(maxInt) + 1;
                    }
                }
                else
                {
                    TickCount = 3;
                }
            }

            var rett = new double[TickCount];
            rett[0] = KeepInt ? Math.Floor(min) : Math.Round(min, count);

            for (var index = 1; index <= (TickCount - 2); index++)
            {
                var value = min + index * delta;
                rett[index] = KeepInt ? (int)(value + 0.5) : Math.Round(value, count);
            }
            rett[TickCount - 1] = KeepInt ? Math.Ceiling(max) : Math.Round(max, count);
            return rett;
        }

        public override double[] GetMinorTicks(IAxisParams axis)
        {
            var ret = new List<double>();
            var max = axis.VisibleRange.AsDoubleRange().Max;
            var min = axis.VisibleRange.AsDoubleRange().Min;
            var count = 1;
            var minTicks = new double[3 * (TickCount - 1)];
            var delta = (max - min) / minTicks.Length;

            count = GetAccuracy(min, max);
            for (var i = 0; i < minTicks.Length; i++)
            {
                if ((i + 1) % 3 == 0)
                    continue;
                ret.Add(KeepInt ? (int)((i + 1) * delta + min + 0.5) : Math.Round((i + 1) * delta + min, count));
            }
            return ret.ToArray();
        }

        private int GetMinDivisors(int num)
        {
            int sqr = Convert.ToInt32(Math.Sqrt(num));
            for (var i = 2; i <= sqr; i++)
            {
                if (num % i == 0)
                {
                    return i;
                }
            }
            return -1;
        }

        private bool IsPrime(int num)
        {
            if (num == 1 || num == 2)
            {
                return true;
            }
            int sqr = Convert.ToInt32(Math.Sqrt(num));
            for (var i = sqr; i >= 2; i--)
            {
                if (num % i == 0)
                {
                    return false;
                }
            }
            return true;
        }

        #endregion Methods
    }
}