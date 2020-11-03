namespace FLIMFitting.Model
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Linq;
    using System.Windows.Media;

    public class DataReader
    {
        #region Fields

        private const int BinCountPerData = 241;
        private const int BinSize = sizeof(UInt32);
        private const int DataCount = 256;

        private static readonly Color[] _colors = new Color[] { Color.FromRgb(0xab,0x51,0xd3),Color.FromRgb(0xFF, 0xA5, 0x00),Color.FromRgb(0x2a,0xe0,0xf4),Color.FromRgb(0xbc,0xf4,0x23),Color.FromRgb(0x1d,0xe4,0x0d),
            Color.FromRgb(0x93,0x79,0x4e),Color.FromRgb(0x89,0x26,0x84),Color.FromRgb(0x37,0x7c,0x79),
            Colors.Yellow,Colors.Lime,Colors.DodgerBlue,Colors.DeepPink,Colors.DarkOrange,
            Colors.Khaki, Colors.LightGreen, Colors.SteelBlue};

        #endregion Fields

        #region Methods

        public static List<FLIMData> ReadFLIMData(string fileName)
        {
            if (!File.Exists(fileName)) return null;
            List<FLIMData> flimList = new List<FLIMData>();
            var bytes = File.ReadAllBytes(fileName);
            int count = bytes.Count() / BinSize;
            int noOfFlimData = count / DataCount;
            for (int i = 0; i < noOfFlimData; i++)
            {
                FLIMData flimData = new FLIMData();
                flimData.ID = (i + 1).ToString();
                flimData.IsSelected = true;
                flimData.RenderColor = _colors[i % _colors.Length];
                int index = i * DataCount* BinSize;
                for(int j = 0; j < BinCountPerData; j++)
                {
                    flimData.DataArray[j]= BitConverter.ToUInt32(bytes, index + j * BinSize);
                }
                flimData.NsPerPoint = (double)BitConverter.ToUInt32(bytes, index + 255 * BinSize) / (double)BitConverter.ToUInt32(bytes, index + 254 * BinSize) * 5 / 128;
                flimList.Add(flimData);
            }
            return flimList;
        }

        #endregion Methods
    }
}