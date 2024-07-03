namespace HistogramControl.ViewModel
{
    using System.Xml;

    using ThorSharedTypes;

    public class HistogramChannelSettings
    {
        #region Properties

        public double BottomPercentileReduction
        {
            get; set;
        }

        public bool ContinuousAuto
        {
            get; set;
        }

        public double Gamma
        {
            get; set;
        }

        public string ImageIdentifierKey
        {
            get; set;
        }

        public bool Log
        {
            get; set;
        }

        public int NumGridLines
        {
            get; set;
        }

        public double ThresholdBP
        {
            get; set;
        }

        public double ThresholdWP
        {
            get; set;
        }

        public double TopPercentileReduction
        {
            get; set;
        }

        public string UnitSymbol
        {
            get; set;
        }

        #endregion Properties

        #region Methods

        public static HistogramChannelSettings MakeFromXmlNode(ref XmlDocument xmlDoc, XmlNode xmlNode)
        {
            var settings = new HistogramChannelSettings();

            string imageIdentifierRef = "";
            string bpRef = "";
            string wpRef = "";
            string gammaRef = "";
            string autoStaRef = "";
            string logStaRef = "";
            string autoTopPercentileRef = "";
            string autoBottomPercentileRef = "";
            string unitSymbolRef = "";
            string numGridLinesRef = "";

            _ = XmlManager.GetAttribute(xmlNode, xmlDoc, "identifier", ref imageIdentifierRef);
            _ = XmlManager.GetAttribute(xmlNode, xmlDoc, "bp", ref bpRef);
            _ = XmlManager.GetAttribute(xmlNode, xmlDoc, "wp", ref wpRef);
            _ = XmlManager.GetAttribute(xmlNode, xmlDoc, "gamma", ref gammaRef);
            _ = XmlManager.GetAttribute(xmlNode, xmlDoc, "AutoSta", ref autoStaRef);
            _ = XmlManager.GetAttribute(xmlNode, xmlDoc, "LogSta", ref logStaRef);
            _ = XmlManager.GetAttribute(xmlNode, xmlDoc, "AutoTopPercentile", ref autoTopPercentileRef);
            _ = XmlManager.GetAttribute(xmlNode, xmlDoc, "AutoBottomPercentile", ref autoBottomPercentileRef);
            _ = XmlManager.GetAttribute(xmlNode, xmlDoc, "UnitSymbol", ref unitSymbolRef);
            _ = XmlManager.GetAttribute(xmlNode, xmlDoc, "NumGridLines", ref numGridLinesRef);

            settings.ImageIdentifierKey = imageIdentifierRef;
            if (double.TryParse(bpRef, out double bp))
            {
                settings.ThresholdBP = bp;
            }
            if (double.TryParse(wpRef, out double wp))
            {
                settings.ThresholdWP = wp;
            }
            if (double.TryParse(gammaRef, out double gamma))
            {
                settings.Gamma = gamma;
            }
            if (bool.TryParse(autoStaRef, out bool autoSta))
            {
                settings.ContinuousAuto = autoSta;
            }
            if (bool.TryParse(logStaRef, out bool logSta))
            {
                settings.Log = logSta;
            }
            if (double.TryParse(autoTopPercentileRef, out double topPercentileReduction))
            {
                settings.TopPercentileReduction = topPercentileReduction;
            }
            if (double.TryParse(autoBottomPercentileRef, out double bottomPercentileReduction))
            {
                settings.BottomPercentileReduction = bottomPercentileReduction;
            }
            settings.UnitSymbol = unitSymbolRef;
            if(int.TryParse(numGridLinesRef, out int numGridLines))
            {
                settings.NumGridLines = numGridLines;
            }

            return settings;
        }

        public void SaveToXmlNode(ref XmlDocument xmlDoc, ref XmlNode xmlNode)
        {
            XmlManager.SetAttribute(xmlNode, xmlDoc, "identifier", ImageIdentifierKey);
            XmlManager.SetAttribute(xmlNode, xmlDoc, "bp", ThresholdBP.ToString());
            XmlManager.SetAttribute(xmlNode, xmlDoc, "wp", ThresholdWP.ToString());
            XmlManager.SetAttribute(xmlNode, xmlDoc, "gamma", Gamma.ToString());
            XmlManager.SetAttribute(xmlNode, xmlDoc, "AutoSta", ContinuousAuto.ToString());
            XmlManager.SetAttribute(xmlNode, xmlDoc, "LogSta", Log.ToString());
            XmlManager.SetAttribute(xmlNode, xmlDoc, "AutoTopPercentile", TopPercentileReduction.ToString());
            XmlManager.SetAttribute(xmlNode, xmlDoc, "AutoBottomPercentile", BottomPercentileReduction.ToString());
            XmlManager.SetAttribute(xmlNode, xmlDoc, "UnitSymbol", UnitSymbol);
            XmlManager.SetAttribute(xmlNode, xmlDoc, "NumGridLines", NumGridLines.ToString());
        }

        #endregion Methods
    }
}