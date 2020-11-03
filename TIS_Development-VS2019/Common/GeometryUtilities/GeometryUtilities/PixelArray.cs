namespace GeometryUtilities
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Media;
    using System.Windows.Shapes;
    using System.Xml;

    using HDF5CS;

    using OverlayManager;

    using ThorSharedTypes;

    public class PixelArray
    {
        #region Fields

        private List<Point> _orderPixel = null;

        #endregion Fields

        #region Constructors

        public PixelArray()
        {
            _orderPixel = new List<Point>();
        }

        #endregion Constructors

        #region Properties

        public double LastX
        {
            get
            {
                return _orderPixel.Last().X;
            }
        }

        public double LastY
        {
            get
            {
                return _orderPixel.Last().Y;
            }
        }

        public int Size
        {
            get
            {
                return _orderPixel.Count();
            }
        }

        #endregion Properties

        #region Methods

        public void AddPoint(double px, double py)
        {
            _orderPixel.Add(new Point(px, py));
        }

        public void AddPoint(Point pt)
        {
            _orderPixel.Add(pt);
        }

        public void AddPoints(List<Point> pts)
        {
            _orderPixel.AddRange(pts);
        }

        public void AppendTo(ref PixelArray toPxArray)
        {
            for (int i = 0; i < _orderPixel.Count; i++)
            {
                toPxArray.AddPoint(new Point(_orderPixel[i].X, _orderPixel[i].Y));
            }
        }

        public void Clear()
        {
            _orderPixel.Clear();
        }

        public Point GetPixel(int id)
        {
            return _orderPixel[id];
        }

        #endregion Methods
    }
}