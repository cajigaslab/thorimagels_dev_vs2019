namespace ImageViewMVM.ViewModels
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows.Input;

    using ThorSharedTypes;

    public class ImageViewCaptureVM : ImageViewVVMBase, IMVMCapture, IViewModelActions
    {
        #region Constructors

        public ImageViewCaptureVM()
            : base(new Models.ImageViewMCapture())
        {
            _mainViewModel = "RunSampleLSViewModel";
            _imageViewM.DFLIMControlViewModelName = "DFLIMControlCaptureViewModel";
            OrthogonalViewVisibility = System.Windows.Visibility.Collapsed;
        }

        #endregion Constructors

        #region Methods

        public new void HandleViewLoaded()
        {
            _imageViewM.ResetBitmap();
            OnPropertyChanged("ImagesGrid");
            OverlayManager.OverlayManagerClass.Instance.mROIsDisableMoveAndResize = true;
            base.HandleViewLoaded();
        }

        #endregion Methods
    }
}