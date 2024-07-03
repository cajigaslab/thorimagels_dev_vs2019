namespace ImageViewMVM.Models
{
    using System;
    using System.Collections.Concurrent;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows.Media;

    using OverlayManager;

    using ThorLogging;

    using ThorSharedTypes;

    internal class ImageViewMCapture : ImageViewMBase
    {
        #region Constructors

        internal ImageViewMCapture()
        {
            _imageReviewType = ImageViewType.Capture;
        }

        #endregion Constructors

        #region Methods

        public override void UpdateOverlayManager(int width, int height, double xScale, double yScale)
        {
            try
            {
                PixelSizeUM umPerPixel = (PixelSizeUM)MVMManager.Instance["RunSampleLSViewModel", "PixelSizeUM", (object)1.0];

                if (OverlayManagerClass.Instance.UmPerPixel.Compare(umPerPixel) || OverlayManagerClass.Instance.PixelX != width || OverlayManagerClass.Instance.PixelY != height)
                {
                    OverlayManagerClass.Instance.UpdateParams(width, height, umPerPixel);
                }

                if (OverlayManagerClass.Instance.ImageXScale != xScale || OverlayManagerClass.Instance.ImageYScale != yScale)
                {
                    OverlayManagerClass.Instance.UpdateAspectRatio(xScale, yScale);
                }
            }
            catch (Exception e)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "UpdateOverlayManager failed. Exception Message:\n" + e.Message);
                return;
            }
        }

        #endregion Methods
    }
}