namespace ImageViewMVM.Models
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;

    using OverlayManager;

    using ThorLogging;

    using ThorSharedTypes;

    using ImageViewMVM.Shared;

    internal class ImageViewMCaptureSetup : ImageViewMBase
    {
        #region Constructors

        internal ImageViewMCaptureSetup()
        {
            _imageReviewType = ImageViewType.CaptureSetup;
        }

        #endregion Constructors

        #region Methods

        public override void UpdateOverlayManager(int width, int height, double xScale, double yScale)
        {
            try
            {
                PixelSizeUM umPerPixel = (PixelSizeUM)MVMManager.Instance["AreaControlViewModel", "PixelSizeUM", (object)1.0];
                if ((int)ICamera.CameraType.LSM != ResourceManagerCS.GetCameraType())
                {
                    double camPixelSize = ((double)MVMManager.Instance["CameraControlViewModel", "CamPixelSizeUM", (object)1.0] * Math.Max(1, Math.Max((int)MVMManager.Instance["CameraControlViewModel", "BinY", (object)1], (int)MVMManager.Instance["CameraControlViewModel", "BinX", (object)1])));
                    umPerPixel = new PixelSizeUM(camPixelSize,camPixelSize);
                }

                bool sizeChanged = OverlayManagerClass.Instance.PixelX != width || OverlayManagerClass.Instance.PixelY != height;
                if (!OverlayManagerClass.Instance.UmPerPixel.Compare(umPerPixel) || sizeChanged)
                {
                    if (sizeChanged)
                    {
                        OnZoomChanged(new ZoomChangeEventArgs(width>height? width/ (double)OverlayManagerClass.Instance.PixelX : height/ (double)OverlayManagerClass.Instance.PixelY));
                    }

                    OverlayManagerClass.Instance.UpdateParams(width, height, umPerPixel);

                    if (true == (bool)MVMManager.Instance["ImageViewCaptureSetupVM", "IsReticleChecked"])
                    {
                        OverlayManagerClass.Instance.InitReticle(true);
                    }
                    if (true == (bool)MVMManager.Instance["ImageViewCaptureSetupVM", "IsScaleButtonChecked"])
                    {
                        OverlayManagerClass.Instance.InitScale(true);
                    }
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

        protected override void OnZoomChanged(ZoomChangeEventArgs e)
        {
            base.OnZoomChanged(e);
        }

        #endregion Methods
    }
}