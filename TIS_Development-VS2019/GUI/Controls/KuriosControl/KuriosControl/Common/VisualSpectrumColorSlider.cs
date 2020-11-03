namespace KuriosControl.Common
{
    using System;
    using System.Collections.Generic;
    using System.Globalization;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Media;

    [TemplatePart(Name = "PART_TickLableCanvas", Type = typeof(Canvas))]
    public class VisualSpectrumColorSlider : Slider
    {
        #region Fields

        private List<TextBlock> listTBs = new List<TextBlock>();
        private Canvas ticklableCanvas;

        #endregion Fields

        #region Constructors

        public VisualSpectrumColorSlider()
        {
            Resources = new ResourceDictionary { Source = new Uri("/KuriosControl;component/View/VisualSpectrumSliderStyle.xaml", UriKind.Relative) };

            Style = (Style)FindResource(typeof(VisualSpectrumColorSlider));
            ApplyTemplate();
        }

        #endregion Constructors

        #region Methods

        public override void OnApplyTemplate()
        {
            base.OnApplyTemplate();

            ticklableCanvas = (Canvas)Template.FindName("PART_TickLableCanvas", this);
        }

        protected override void OnMaximumChanged(double oldMaximum, double newMaximum)
        {
            base.OnMaximumChanged(oldMaximum, newMaximum);
            AutoGenerateTicks();
            //UpdateTickLabelsUI();
        }

        protected override void OnMinimumChanged(double oldMinimum, double newMinimum)
        {
            base.OnMinimumChanged(oldMinimum, newMinimum);
            AutoGenerateTicks();
            //UpdateTickLabelsUI();
        }

        protected override void OnRenderSizeChanged(SizeChangedInfo sizeInfo)
        {
            base.OnRenderSizeChanged(sizeInfo);
            //UpdateTickLabelsUI();
        }

        private void AutoGenerateTicks()
        {
            this.Ticks.Clear();
            var strSize = MeasureString(((this.Maximum - this.Minimum) / 2).ToString());

            var dataWidth = this.Maximum - this.Minimum;
            int tickCnt = (int)(dataWidth / this.TickFrequency);
            TicksProvider ticksProvider = new TicksProvider();
            var ticks = ticksProvider.GetTicks(new Range<int>((int)this.Minimum, (int)this.Maximum), tickCnt);

            foreach (var t in ticks)
            {
                this.Ticks.Add(t);
            }
            this.listTBs.Clear();
        }

        private Size MeasureString(string candidate)
        {
            var formattedText = new FormattedText(
                candidate,
                CultureInfo.CurrentUICulture,
                FlowDirection.LeftToRight,
                new Typeface(this.FontFamily, this.FontStyle, this.FontWeight, this.FontStretch),
                this.FontSize,
                Brushes.Black,
                VisualTreeHelper.GetDpi(this).PixelsPerDip);

            return new Size(formattedText.Width, formattedText.Height);
        }

        private void UpdateTickLabelsUI()
        {
            var count = this.Ticks.Count;
            var dataWidth = this.Maximum - this.Minimum;

            if (listTBs.Count == 0)
            {
                this.ticklableCanvas.Children.Clear();
                for (int i = 0; i < count; i++)
                {
                    TextBlock tb = new TextBlock()
                    {
                        Text = Ticks[i].ToString(),
                        Foreground = Brushes.Black
                    };

                    listTBs.Add(tb);
                    this.ticklableCanvas.Children.Add(tb);
                }
            }

            var width = this.ticklableCanvas.ActualWidth;
            var height = this.ticklableCanvas.ActualHeight;
            for (int i = 0; i < listTBs.Count; i++)
            {
                if (this.Orientation == System.Windows.Controls.Orientation.Horizontal)
                {
                    Canvas.SetLeft(listTBs[i], (Ticks[i] - this.Minimum) / dataWidth * width - 10);
                }
                else
                {
                    Canvas.SetTop(listTBs[i], (Ticks[i] - this.Minimum) / dataWidth * height - 10);
                }
            }
        }

        #endregion Methods
    }
}