using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media;
using System.Windows.Shapes;

namespace OverlayManager
{
    public abstract class ScalableShape : Shape
    {
        public abstract double ImageScaleX { get; set; }
        public abstract double ImageScaleY { get; set; }

        public abstract void ApplyScaleUpdate(double xScale, double yScale);
    }
}
