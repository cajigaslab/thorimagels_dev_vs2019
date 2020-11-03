namespace CustomMessageBox
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;
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
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class CustomMessageBox : Window
    {
        #region Constructors

        /// <summary>
        /// Construct a custom message box
        /// </summary>
        /// <param name="message"> The message to be displayed </param>
        /// <param name="title"> The title of this message box window </param>
        /// <param name="checkBoxText"> The text to be put next to the check box </param>
        /// <param name="okButtonText"> The confirmation button text </param>
        /// <param name="cancelButtonText"> The cancel button text </param>
        public CustomMessageBox(string message, string title = "", string checkBoxText = "Don't ask again", string okButtonText = "OK", string cancelButtonText = "Cancel")
        {
            InitializeComponent();
            SetupMessageBox(message, title, checkBoxText, okButtonText, cancelButtonText);
        }

        #endregion Constructors

        #region Properties

        public int ButtonFlag
        {
            get;
            set;
        }

        /// <summary>
        /// Is the check box checked
        /// </summary>
        public bool CheckBoxChecked
        {
            get
            {
                return checkBox.IsChecked.GetValueOrDefault(false);
            }
        }

        #endregion Properties

        #region Methods

        /// <summary>
        /// Sets up this message box with the proper values
        /// </summary>
        /// <param name="message"> The message to be displayed </param>
        /// <param name="title"> The title of this message box window </param>
        /// <param name="checkBoxText"> The text to be put next to the check box </param>
        /// <param name="okButtonText"> The confirmation button text </param>
        /// <param name="cancelButtonText"> The cancel button text </param>
        public void SetupMessageBox(string message, string title, string checkBoxText, string okButtonText, string cancelButtonText)
        {
            this.message.Text = message;
            this.okayButton.Content = okButtonText;
            this.cancelButton.Content = cancelButtonText;
            this.checkBox.Content = checkBoxText;
            this.Title = title;
            ButtonFlag = 0;
        }

        /// <summary>
        /// Sets the dialog result to false and exits
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void cancelButton_Click(object sender, RoutedEventArgs e)
        {
            DialogResult = false;
            ButtonFlag = -1;
            this.Close();
        }

        /// <summary>
        /// Sets the dialog result to true and exits
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void okayButton_Click(object sender, RoutedEventArgs e)
        {
            DialogResult = true;
            ButtonFlag = 1;
            this.Close();
        }

        #endregion Methods
    }
}