using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ImageViewMVM.Shared
{
    public class ZoomChangeEventArgs
    {
        public ZoomChangeEventArgs(double zoomFactorChange)
        {
            ZoomFactorChange = zoomFactorChange;
        }

        public double ZoomFactorChange { get; }
    }
}
