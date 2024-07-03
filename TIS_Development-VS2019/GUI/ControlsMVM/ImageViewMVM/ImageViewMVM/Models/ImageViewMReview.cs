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

    internal class ImageViewMReview : ImageViewMBase
    {

        #region Constructors

        internal ImageViewMReview()
        {
            _imageReviewType = ImageViewType.Review;
        }

        #endregion Constructors

        #region Properties

        public override int mROIPriorityIndex
        {
            get => _mROIPriorityIndex;
            set
            {
                _mROIPriorityIndex = value;
                if (_mROISpatialDisplaybleEnable)
                {
                    _redisplayAllROIs = true;
                }
            }
        }

        public override bool mROISpatialDisplaybleEnable
        {
            get => _mROISpatialDisplaybleEnable;
            set
            {
                if (_mROISpatialDisplaybleEnable != value)
                {
                    _mROISpatialDisplaybleEnable = value;
                    _redisplayAllROIs = true;
                }
            }
        }

        #endregion Properties

        #region Methods

        public override void UpdateOverlayManager(int width, int height, double xScale, double yScale)
        {
            try
            {
                PixelSizeUM umPerPixel = (PixelSizeUM)MVMManager.Instance["ImageReviewViewModel", "PixelSizeUM", (object)1.0];
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

        #region Other

        //public override FrameData FrameData
        //{
        //    get => _frameData;
        //    set {
        //        if (value == null)
        //        {
        //            return;
        //        }
        //        _frameData = value;
        //        if (_bitsPerPixel != _frameData.bitsPerPixel)
        //        {
        //            _bitsPerPixel = _frameData.bitsPerPixel;
        //        }
        //       _ = BuildBitmapsTask2();
        //    }
        //}
        //protected override void BuildBitmapsTask()
        //{
        //    do
        //    {
        //        bool wpbpChanged = DidHistogramUpdate();
        //        if (((wpbpChanged || _palletChanged || _chanEnableChanged) && _imagesGrid != null))
        //        {
        //            AutoEnhanceIfEnabled();
        //            bool updatePixelData = _updatePixelData || wpbpChanged;
        //            _chanEnableChanged = false;
        //            _palletChanged = false;
        //            Create24BitPixelDataByteRawAndComposite(updatePixelData);
        //            _updateBitmap = true;
        //        }
        //        if (_updateBitmap)
        //        {
        //            if (1 == _lastFrameInfo.isMROI && _mROISpatialDisplaybleEnable)
        //            {
        //                CreateXYmROISpatialBitmap();
        //            }
        //            else
        //            {
        //                CreateXYBitmap();
        //            }
        //            _updateBitmap = false;
        //        }
        //    }
        //    while (_runBitmapBuildingThread);
        //}

        #endregion Other
    }
}