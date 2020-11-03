using System.Reflection;
using System.Resources;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;

namespace CaptureSetupDll.ViewModel
{
    /// <summary>
    /// A Slider which provides a way to modify the 
    /// auto tooltip text by using a format string.
    /// </summary>
    public class FormattedSlider : Slider
    {
        #region fields
        private ToolTip _autoToolTip;
        private string _autoToolTipFormat;
        # endregion fields

        /// <summary>
        /// Gets/sets a format string used to modify the auto tooltip's content.
        /// Note: This format string must contain exactly one placeholder value,
        /// which is used to hold the tooltip's original content.
        /// </summary>        

        # region functions
        protected override void OnThumbDragStarted(DragStartedEventArgs e)
        {
            base.OnThumbDragStarted(e);
            this.FormatAutoToolTipContent();
        }

        protected override void OnThumbDragDelta(DragDeltaEventArgs e)
        {
            base.OnThumbDragDelta(e);
            this.FormatAutoToolTipContent();
        }

        private void FormatAutoToolTipContent()
        {
            if (string.IsNullOrEmpty(this.AutoToolTipFormat))
            {
                return;
            }

            object content;

            string text = base.Value.ToString();
            double number;
            if (double.TryParse(text, out number))
            {
                content = number;
            }
            else
            {
                content = text;
            }

            this.AutoToolTip.Content = string.Format(this.AutoToolTipFormat,content);
        }

        public string AutoToolTipFormat
        {
            get { return _autoToolTipFormat; }
            set { _autoToolTipFormat = value; }
        }

        private ToolTip AutoToolTip
        {
            get
            {
                if (_autoToolTip == null)
                {
                    FieldInfo field = typeof(Slider).GetField(
                        "_autoToolTip",
                        BindingFlags.NonPublic | BindingFlags.Instance);

                    _autoToolTip = field.GetValue(this) as ToolTip;
                }

                return _autoToolTip;
            }
        }

        # endregion functions
    }
}