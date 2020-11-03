namespace ROIStatsChart.Model
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using System.Text.RegularExpressions;
    using System.Threading.Tasks;
    using System.Windows.Media;

    public static class ChartLineProperty
    {
        #region Fields

        static Random randonGen = new Random();

        #endregion Fields

        #region Methods

        /// <summary>
        /// Return color spec based on ROI info extracted from LineName.
        /// </summary>
        /// <param name="lineName"></param>
        /// <param name="inType"></param>
        /// <returns></returns>
        public static object GetLineColor(string lineName, Type inType)
        {
            object ret = Colors.Black;
            if (-1 < lineName.LastIndexOf("_Ar") )
            {
                int indx = lineName.LastIndexOf("_Ar") + 3;
                int nLine = Convert.ToInt32(lineName.Substring(indx, lineName.Length - indx));

                switch ((nLine - 1) % Constants.MaxColorNum)
                {
                    case 0: ret = Colors.Yellow; break;
                    case 1: ret = Colors.Lime; break;
                    case 2: ret = Colors.DodgerBlue; break;
                    case 3: ret = Colors.DeepPink; break;
                    case 4: ret = Colors.DarkOrange; break;
                    case 5: ret = Colors.Khaki; break;
                    case 6: ret = Colors.LightGreen; break;
                    case 7: ret = Colors.SteelBlue; break;
                }

                switch (inType.Name)
                {
                    case "Color":
                        return ret;
                    case "Brush":
                        ret = new SolidColorBrush((Color)ret);
                        return ret;
                }
            }
            string pattern = "(.*)([a-zA-z])([0-9]{1,5})";
            Regex ex = new Regex(pattern, RegexOptions.IgnoreCase);

            Match match = ex.Match(lineName);

            //the last digits are the roi index
            int roiIndex = Convert.ToInt32(match.Groups[3].ToString());

            switch ((roiIndex - 1) % Constants.MaxColorNum)
            {
                case 0: ret = Colors.Yellow; break;
                case 1: ret = Colors.Lime; break;
                case 2: ret = Colors.DodgerBlue; break;
                case 3: ret = Colors.DeepPink; break;
                case 4: ret = Colors.DarkOrange; break;
                case 5: ret = Colors.Khaki; break;
                case 6: ret = Colors.LightGreen; break;
                case 7: ret = Colors.SteelBlue; break;
            }
            switch (inType.Name)
            {
                case "Color":
                    return ret;
                case "Brush":
                    ret = new SolidColorBrush((Color)ret);
                    break;
            }
            return ret;
        }

        public static object GetLineColor(int index, Type inType)
        {
            object ret = Colors.Black;

            switch ((index - 1) % Constants.MaxColorNum)
            {
                case 0: ret = Colors.Yellow; break;
                case 1: ret = Colors.Lime; break;
                case 2: ret = Colors.DodgerBlue; break;
                case 3: ret = Colors.DeepPink; break;
                case 4: ret = Colors.DarkOrange; break;
                case 5: ret = Colors.Khaki; break;
                case 6: ret = Colors.LightGreen; break;
                case 7: ret = Colors.SteelBlue; break;
            }
            switch (inType.Name)
            {
                case "Color":
                    return ret;
                case "Brush":
                    ret = new SolidColorBrush((Color)ret);
                    break;
            }
            return ret;
        }

        #endregion Methods
    }

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

    public class XYPoint
    {
        #region Constructors

        public XYPoint(double y, double x)
        {
            Y = y;
            X = x;
        }

        public XYPoint()
        {
        }

        #endregion Constructors

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

    static class Constants
    {
        #region Fields

        public const int MaxColorNum = 8;

        #endregion Fields
    }
}