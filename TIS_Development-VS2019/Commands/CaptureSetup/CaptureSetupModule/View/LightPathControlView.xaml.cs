namespace CaptureSetupDll.View
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Navigation;
    using System.Windows.Shapes;

    using CaptureSetupDll.ViewModel;

    /// <summary>
    /// Interaction logic for LightPathControlView.xaml
    /// </summary>
    public partial class LightPathControlView : UserControl
    {
        #region Constructors

        public LightPathControlView()
        {
            InitializeComponent();
            if (System.ComponentModel.DesignerProperties.GetIsInDesignMode(this) == true)
                return;
        }

        #endregion Constructors

        #region Methods

        private void btApplyLightPathSequenceStepSettings_Click(object sender, RoutedEventArgs e)
        {
            Button apply = (sender as Button);
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            int lineNumber = (int)apply.CommandParameter;
            //lineNumber is 1 indexed
            vm.LoadCaptureSequenceStep(lineNumber - 1);
        }

        private void btApplyLightPathSequenceStepTemplateSettings_Click(object sender, RoutedEventArgs e)
        {
            Button apply = (sender as Button);

            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            int lineNumber = (int)apply.CommandParameter;
            //lineNumber is 1 indexed
            vm.LoadLightPathListtep(lineNumber - 1);
        }

        private void btDeleteLightPathSequenceStepTemplate_Click(object sender, RoutedEventArgs e)
        {
            Button delete = (sender as Button);
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }

            int lineNumber = (int)delete.CommandParameter;
            //lineNumber is 1 indexed
            vm.DeleteLightPathSequenceStepTemplate(lineNumber - 1);
        }

        private void btDeleteLightPathSequenceStep_Click(object sender, RoutedEventArgs e)
        {
            Button delete = (sender as Button);
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            int lineNumber = (int)delete.CommandParameter;
            //lineNumber is 1 indexed
            vm.DeleteLightPathSequenceStep(lineNumber - 1);
        }

        private void btUpdateLightPathSequenceStepSettings_Click(object sender, RoutedEventArgs e)
        {
            Button update = (sender as Button);
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            int lineNumber = (int)update.CommandParameter;
            //lineNumber is 1 indexed
            vm.UpdateCaptureSequenceStep(lineNumber - 1);
        }

        private void btUpdateLightPathSequenceStepTemplateSettings_Click(object sender, RoutedEventArgs e)
        {
            Button update = (sender as Button);
            CaptureSetupViewModel vm = (CaptureSetupViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            int lineNumber = (int)update.CommandParameter;
            //lineNumber is 1 indexed
            vm.UpdateLightPathList(lineNumber - 1);
        }

        #endregion Methods
    }
}