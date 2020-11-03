namespace RangeSliderWPF
{
    using System;
    using System.Reflection;
    using System.Resources;
    using System.Windows.Controls;
    using System.Windows.Controls.Primitives;
    using System.Windows.Data;

    /// <summary>
    /// A Slider which provides a way to modify the 
    /// auto tooltip text by using a format string.
    /// </summary>
    public class FormattedSlider : Slider
    {
        #region Fields

        private ToolTip _autoToolTip;
        private string _autoToolTipFormat;

        #endregion Fields

        #region Properties

        /// <summary>
        /// Gets/sets a format string used to modify the auto tooltip's content.
        /// Note: This format string must contain exactly one placeholder value,
        /// which is used to hold the tooltip's original content.
        /// </summary>        
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

        protected override void OnDecreaseLarge()
        {
            base.OnDecreaseLarge();

            BindingExpression be = this.GetBindingExpression(Slider.ValueProperty);
            be.UpdateSource();
        }

        protected override void OnIncreaseLarge()
        {
            base.OnIncreaseLarge();

            BindingExpression be = this.GetBindingExpression(Slider.ValueProperty);
            be.UpdateSource();
        }

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
                number = Math.Round(number, 2);
                content = number;
            }
            else
            {
                content = text;
            }

            this.AutoToolTip.Content = string.Format(this.AutoToolTipFormat, content);
        }

        #endregion Methods
    }
}