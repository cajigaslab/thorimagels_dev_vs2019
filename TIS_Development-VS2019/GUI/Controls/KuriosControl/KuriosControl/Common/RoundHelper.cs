namespace KuriosControl.Common
{
    using System;
    using System.Diagnostics;

    internal static class RoundHelper
    {
        #region Methods

        public static int Clamp(int value, int min, int max)
        {
            return Math.Max(min, Math.Min(value, max));
        }

        internal static RoundingInfo CreateRoundedRange(double min, double max)
        {
            double delta = max - min;

            if (delta == 0)
                return new RoundingInfo { Min = min, Max = max, Log = 0 };

            int log = (int)Math.Round(Math.Log10(Math.Abs(delta))) + 1;

            double newMin = Round(min, log);
            double newMax = Round(max, log);
            if (newMin == newMax)
            {
                log--;
                newMin = Round(min, log);
                newMax = Round(max, log);
            }

            return new RoundingInfo { Min = newMin, Max = newMax, Log = log };
        }

        internal static int GetDifferenceLog(double min, double max)
        {
            return (int)Math.Log(Math.Abs(max - min));
        }

        internal static double Round(double number, int rem)
        {
            if (rem <= 0)
            {
                rem = Clamp(-rem, 0, 15);
                return Math.Round(number, rem);
            }
            else
            {
                double pow = Math.Pow(10, rem - 1);
                double val = pow * Math.Round(number / Math.Pow(10, rem - 1));
                return val;
            }
        }

        #endregion Methods
    }

    [DebuggerDisplay("{Min} - {Max}, Log = {Log}")]
    internal sealed class RoundingInfo
    {
        #region Properties

        public int Log
        {
            get; set;
        }

        public double Max
        {
            get; set;
        }

        public double Min
        {
            get; set;
        }

        #endregion Properties
    }
}