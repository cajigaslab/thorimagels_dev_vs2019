namespace MCM6000_Control
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Reflection;
    using System.Resources;
    using System.Text;
    using System.Windows.Controls;
    using System.Windows.Controls.Primitives;

    class FormattedSlider : Slider
    {
        #region Fields

        private ToolTip _autoToolTip;
        private string _autoToolTipFormat;

        #endregion Fields

        #region Properties

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

        #endregion Properties

        #region Methods

        protected override void OnThumbDragDelta(DragDeltaEventArgs e)
        {
            base.OnThumbDragDelta(e);
            this.FormatAutoToolTipContent();
        }

        protected override void OnThumbDragStarted(DragStartedEventArgs e)
        {
            base.OnThumbDragStarted(e);
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

            this.AutoToolTip.Content = string.Format(this.AutoToolTipFormat, content);
        }

        #endregion Methods

        #region Other

        /// <summary>
        /// Gets/sets a format string used to modify the auto tooltip's content.
        /// Note: This format string must contain exactly one placeholder value,
        /// which is used to hold the tooltip's original content.
        /// </summary>        

        #endregion Other
    }
}