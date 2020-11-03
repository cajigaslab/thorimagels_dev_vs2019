namespace CaptureSetupDll.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.IO;
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

    /// <summary>
    /// Interaction logic for ZStagePresetLocationEdit.xaml
    /// </summary>
    public partial class ZStagePresetLocationEdit : Window
    {
        #region Constructors

        public ZStagePresetLocationEdit()
        {
            InitializeComponent();
            this.KeyDown += ZStagePresetLocationEdit_KeyDown;
            this.Loaded += ZStagePresetLocationEdit_Loaded;
        }

        #endregion Constructors

        #region Properties

        public string LocationName
        {
            get
            {
                return tbName.Text;
            }
            set
            {
                tbName.Text = value;
            }
        }

        #endregion Properties

        #region Methods

        private void btnCancel_Click(object sender, RoutedEventArgs e)
        {
            this.DialogResult = false;
            this.Close();
        }

        private void btnOK_Click(object sender, RoutedEventArgs e)
        {
            CloseWithConfirmation();
        }

        /// <summary>
        /// Closes this dialog and sets the result as true
        /// </summary>
        private void CloseWithConfirmation()
        {
            this.DialogResult = true;
            this.Close();
        }

        /// <summary>
        /// Focuses on the name text box and selects all text inside
        /// </summary>
        private void FocusAndSelectTextBox()
        {
            this.tbName.Focus();
            this.tbName.SelectAll();
        }

        /// <summary>
        /// Handles the enter key being pressed by moving the keyboard focus to the next element
        /// </summary>
        private void HandleEnterKey()
        {
            CloseWithConfirmation();
        }

        /// <summary>
        /// Handles Key Presses
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void ZStagePresetLocationEdit_KeyDown(object sender, KeyEventArgs e)
        {
            //filter for the enter or return key
            if ((e.Key == Key.Enter) || (e.Key == Key.Return))
            {
                e.Handled = true;
                HandleEnterKey();
            }
        }

        /// <summary>
        /// Performs additional setup routines for the UI
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void ZStagePresetLocationEdit_Loaded(object sender, RoutedEventArgs e)
        {
            FocusAndSelectTextBox();
        }

        #endregion Methods
    }
}