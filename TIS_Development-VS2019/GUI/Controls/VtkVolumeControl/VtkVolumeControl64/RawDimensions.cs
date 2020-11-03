namespace VtkVolumeControl
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;

    /// <summary>
    /// Contains data specifying the dimensions of a raw file
    /// </summary>
    struct RawDimensions
    {
        #region Properties

        public int Channels
        {
            get; set;
        }

        public int FlybackDepth
        {
            get; set;
        }

        public int Height
        {
            get; set;
        }

        public int ImageDepth
        {
            get; set;
        }

        public int TotalDepth
        {
            get
            {
                return ImageDepth + FlybackDepth;
            }
        }

        public int Width
        {
            get; set;
        }

        #endregion Properties
    }
}