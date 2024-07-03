namespace ImageViewMVM.ViewModels
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows.Input;

    using ThorSharedTypes;

    public class ImageViewCaptureSetupVM : ImageViewVVMBase, IMVM, IViewModelActions
    {
        #region Constructors

        public ImageViewCaptureSetupVM()
            : base(new Models.ImageViewMCaptureSetup())
        {
            _mainViewModel = "CaptureSetupViewModel";
            _imageViewM.DFLIMControlViewModelName = "DFLIMControlViewModel";
            _imageViewM.AllowReferenceImage = true;
            _imageViewM.ExperimentPath = ResourceManagerCS.GetMyDocumentsThorImageFolderString() + "ZStackCache";
            HelpText = "mouse wheel = zoom, left click = pan, right click = save image";
            HelpTextVisibility = System.Windows.Visibility.Visible;
        }

        #endregion Constructors

        #region Properties

        public override bool DoneLoadingOrthogonalView
        {
            set
            {
                bool continuousZStackPreview = (bool)MVMManager.Instance["ZControlViewModel", "EnableContinuousZStackPreview", (object)false];
                //if continuous zstack is checked then continuosly update, if it has been manually stopped then don't start it again
                if (continuousZStackPreview == true && IsOrthogonalViewChecked)
                {
                    //need this flag to not allow the xml persistance to close the progress window
                    MVMManager.Instance[_mainViewModel, "StartingContinuousZStackPreview"] = true;
                    ICommand zStackPreviewCommand = (ICommand)MVMManager.Instance["ZControlViewModel", "PreviewZStackCommand", (object)null];
                    if (null != zStackPreviewCommand)
                    {
                        zStackPreviewCommand.Execute(null);
                    }
                    MVMManager.Instance[_mainViewModel, "StartingContinuousZStackPreview"] = false;
                }
                else if (!continuousZStackPreview)
                {
                    CloseProgressWindow();
                }
            }
        }

        public override int ZStepNum
        {
            get
            {
                return (int)MVMManager.Instance["ZControlViewModel", "ZScanNumSteps", (object)1];
            }
            set
            {

            }
        }

        public override double ZStepSizeUM
        {
            get
            {
                return (double)MVMManager.Instance["ZControlViewModel", "ZScanStep", (object)0];
            }
            set
            {

            }
        }

        #endregion Properties

        #region Methods

        public new void HandleViewLoaded()
        {
            _imageViewM.ResetFrameDataSet();
            OverlayManager.OverlayManagerClass.Instance.mROIsDisableMoveAndResize = false;
            base.HandleViewLoaded();
        }

        public new void HandleViewUnloaded()
        {
            base.HandleViewUnloaded();
        }

        #endregion Methods
    }
}