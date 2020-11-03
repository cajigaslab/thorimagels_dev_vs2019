namespace ROIStatsChart.CustomModifiers
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;

    using Abt.Controls.SciChart.ChartModifiers;

    public class RightDoubleClickZoomExtentsModifier : ZoomExtentsModifier
    {
        #region Fields

        private Stopwatch stopwatch = new Stopwatch();

        #endregion Fields

        #region Methods

        [DllImport("user32.dll", CharSet = CharSet.Auto, ExactSpelling = true)]
        public static extern int GetDoubleClickTime();

        public override void OnModifierDoubleClick(ModifierMouseArgs e)
        {
            if (e.MouseButtons == MouseButtons.Right)
            {
                base.OnModifierDoubleClick(e);
            }
        }

        public override void OnModifierMouseDown(ModifierMouseArgs e)
        {
            if (e.MouseButtons == MouseButtons.Right)
            {
                if (stopwatch.ElapsedMilliseconds < GetDoubleClickTime())
                {
                    base.OnModifierDoubleClick(e);
                }
                stopwatch.Restart();
            }
            base.OnModifierMouseDown(e);
        }

        #endregion Methods
    }
}