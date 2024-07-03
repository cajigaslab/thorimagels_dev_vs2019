namespace HistogramControl.ViewModel
{
    using System;

    using SciChart.Charting.Visuals.Axes.LabelProviders;

    /// <summary>
    /// Remaps the data to a new line given by Slope and an Offset, and rounds to an Integer.
    /// </summary>
    class RemappedLabelProvider : NumericLabelProvider
    {
        #region Properties

        public double Offset
        {
            get; set;
        }

        public int RoundTo
        {
            get; set;
        }

        public double Slope
        {
            get; set;
        }

        #endregion Properties

        #region Methods

        /// <summary>
        /// Formats a label for the cursor, from the specified data-value passed in
        /// </summary>
        /// <param name="dataValue">The data-value to format</param>
        /// <returns>
        /// The formatted cursor label string
        /// </returns>
        public override string FormatCursorLabel(IComparable dataValue)
        {
            // Note: Implement as you wish, converting Data-Value to string
            return dataValue.ToString();

            // NOTES:
            // dataValue is always a double.
            // For a NumericAxis this is the double-representation of the data
            // For a DateTimeAxis, the conversion to DateTime is new DateTime((long)dataValue)
            // For a TimeSpanAxis the conversion to TimeSpan is new TimeSpan((long)dataValue)
            // For a CategoryDateTimeAxis, dataValue is the index to the data-series
        }

        /// <summary>
        /// Formats a label for the axis from the specified data-value passed in
        /// </summary>
        /// <param name="dataValue">The data-value to format</param>
        /// <returns>
        /// The formatted label string
        /// </returns>
        public override string FormatLabel(IComparable dataValue)
        {
            if(double.TryParse(dataValue.ToString(), out double val))
            {
                double newVal = val * Slope + Offset;
                var result = Math.Round(newVal, RoundTo);
                var roundedString = String.Format($"{{0:f{RoundTo}}} ", result);

                return roundedString;
            }

            return dataValue.ToString();

            // NOTES:
            // dataValue is always a double.
            // For a NumericAxis this is the double-representation of the data
            // For a DateTimeAxis, the conversion to DateTime is new DateTime((long)dataValue)
            // For a TimeSpanAxis the conversion to TimeSpan is new TimeSpan((long)dataValue)
            // For a CategoryDateTimeAxis, dataValue is the index to the data-series
        }

        #endregion Methods
    }
}