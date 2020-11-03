namespace FLIMFitting
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Reflection;
    using System.Runtime.CompilerServices;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Threading;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Xml;

    #region Enumerations

    public enum HistogramGroupType
    {
        ChannelHistograms,
        ChannelHistogramAndROIs
    }

    #endregion Enumerations

    public class FLIMHistogramGroupData
    {
        #region Constructors

        public FLIMHistogramGroupData()
        {
            Channels = new List<int>();
            Colors = new List<SolidColorBrush>();
            Histograms = new List<uint[]>();
            HistrogramNames = new List<string>();
        }

        #endregion Constructors

        #region Properties

        public List<int> Channels
        {
            get;
            set;
        }

        public List<SolidColorBrush> Colors
        {
            get;
            set;
        }

        public string GroupName
        {
            get;
            set;
        }

        public HistogramGroupType GroupType
        {
            get;
            set;
        }

        public List<uint[]> Histograms
        {
            get;
            set;
        }

        public List<string> HistrogramNames
        {
            get;
            set;
        }

        #endregion Properties
    }
}