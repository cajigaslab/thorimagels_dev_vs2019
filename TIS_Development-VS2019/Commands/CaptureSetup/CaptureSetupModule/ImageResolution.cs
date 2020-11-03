namespace CaptureSetupDll
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;

    /// <summary>
    /// Represents an image resolution defined as pixels wide by pixels tall
    /// </summary>
    public class ImageResolution : IEquatable<ImageResolution>
    {
        #region Fields

        private readonly int _xPixels;
        private readonly int _yPixels;

        #endregion Fields

        #region Constructors

        /// <summary>
        /// Construct a ImageResolution with the input parameters
        /// </summary>
        /// <param name="xPixels"></param>
        /// <param name="yPixels"></param>
        public ImageResolution(int xPixels, int yPixels)
        {
            _xPixels = xPixels;
            _yPixels = yPixels;
        }

        #endregion Constructors

        #region Properties

        /// <summary>
        /// A text representation of the resolution in the form 'width x height'
        /// </summary>
        public string Text
        {
            get
            {
                return String.Format("{0} x {1}", XPixels, YPixels);
            }
        }

        /// <summary>
        /// Width of the image in pixels
        /// </summary>
        public int XPixels
        {
            get
            {
                return _xPixels;
            }
        }

        /// <summary>
        /// Height of the image in pixels
        /// </summary>
        public int YPixels
        {
            get
            {
                return _yPixels;
            }
        }

        #endregion Properties

        #region Methods

        /// <summary>
        /// Compares the value of this object to an input ImageResolution, sorting by xpixels then ypixels
        /// </summary>
        /// <param name="other"> ImageResolution to compare with </param>
        /// <returns> 1 if greater, 0 if equal, and -1 if less </returns>
        public int CompareTo(ImageResolution other)
        {
            if (XPixels > other.XPixels)
            {
                return 1;
            }
            else if (XPixels < other.XPixels)
            {
                return -1;
            }
            else
            {
                if (YPixels > other.YPixels)
                {
                    return 1;
                }
                else if (YPixels < other.YPixels)
                {
                    return -1;
                }
                else
                {
                    return 0;
                }
            }
        }

        public bool Equals(ImageResolution other)
        {
            if ((this._xPixels == other._xPixels) && (this._yPixels == other._yPixels))
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        /// <summary>
        /// Returns if the image has a dimension that is generally invalid, such as less than or equal to 0
        /// </summary>
        /// <returns></returns>
        public bool HasInvalidPixelDimension()
        {
            return XPixels == 0 || YPixels == 0;
        }

        #endregion Methods
    }
}