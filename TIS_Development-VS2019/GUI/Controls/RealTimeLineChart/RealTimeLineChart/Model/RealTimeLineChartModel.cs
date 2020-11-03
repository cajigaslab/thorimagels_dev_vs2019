namespace RealTimeLineChart.Model
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.IO.Pipes;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Threading;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Navigation;
    using System.Windows.Shapes;

    using Abt.Controls.SciChart.Model.DataSeries;

    /// <summary>
    /// A data-structure to contain a list of X,Y double-precision points
    /// </summary>
    public class DoubleSeries : List<XYPoint>
    {
        #region Constructors

        public DoubleSeries()
        {
        }

        public DoubleSeries(int capacity)
            : base(capacity)
        {
        }

        #endregion Constructors

        #region Properties

        public IList<double> XData
        {
            get { return this.Select(x => x.X).ToArray(); }
        }

        public IList<double> YData
        {
            get { return this.Select(x => x.Y).ToArray(); }
        }

        #endregion Properties
    }

    public partial class RealTimeLineChart
    {
        #region Constructors

        public RealTimeLineChart()
        {
        }

        #endregion Constructors
    }

    public class XYPoint
    {
        #region Properties

        public double X
        {
            get;
            set;
        }

        public double Y
        {
            get;
            set;
        }

        #endregion Properties
    }
}