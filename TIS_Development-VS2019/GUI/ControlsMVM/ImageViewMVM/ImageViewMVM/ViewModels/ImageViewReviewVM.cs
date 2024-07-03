namespace ImageViewMVM.ViewModels
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;

    using ThorSharedTypes;

    public class ImageViewReviewVM : ImageViewVVMBase, IMVMReview, IViewModelActions
    {
        #region Constructors

        public ImageViewReviewVM()
            : base(new Models.ImageViewMReview())
        {
            _mainViewModel = "ImageReviewViewModel";
            _imageViewM.DFLIMControlViewModelName = "DFLIMControlReviewViewModel";
        }

        #endregion Constructors

        #region Methods

        public new void HandleViewLoaded()
        {
            _imageViewM.ResetFrameDataSet();
            OverlayManager.OverlayManagerClass.Instance.mROIsDisableMoveAndResize = true;
            base.HandleViewLoaded();
        }

        #endregion Methods
    }
}