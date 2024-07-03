using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;

namespace ThorDAQConfigControl.Controls.Helpers
{
    public class ScrollViewerBehavior
    {
        public static int GetScrollPosition(DependencyObject obj)
        {
            return (int)obj.GetValue(ScrollPositionProperty);
        }

        public static void SetScrollPosition(DependencyObject obj, int value)
        {
            obj.SetValue(ScrollPositionProperty, value);
        }

        public static readonly DependencyProperty ScrollPositionProperty =
            DependencyProperty.RegisterAttached("ScrollPosition", typeof(int), typeof(ScrollViewerBehavior), new PropertyMetadata(0, (o, e) =>
            {
                var scrollViewer = o as ScrollViewer;
                if (scrollViewer == null)
                {
                    return;
                }
                if (e.NewValue is int)
                {
                    scrollViewer.ScrollToVerticalOffset((int)e.NewValue);
                }
            }));
    }
}
