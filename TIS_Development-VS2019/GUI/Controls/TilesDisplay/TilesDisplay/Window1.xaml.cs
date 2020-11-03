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

namespace TilesDisplayTest
{
    /// <summary>
    /// Interaction logic for Window1.xaml
    /// </summary>
    public partial class Window1 : Window
    {
        public Window1()
        {
            InitializeComponent();
            //controlUnderTest.Mode = TilesDisplay.TilingMode.TopLeftMode;
            //controlUnderTest.changeToMode(controlUnderTest.Mode);
            controlUnderTest.XSpacing = 10;
            controlUnderTest.YSpacing = 10;
            //controlUnderTest.AspectRatio = 1;
           // controlUnderTest.colorChildControl(0, 0, Brushes.Blue);
           // controlUnderTest.CurrentImageIndex = 0;
           // controlUnderTest.CurrentImageIndex = 1;
            //controlUnderTest.CurrentImageIndex = 2;
            //controlUnderTest.CurrentImageIndex = 3;

        }

        private void OnClickFirst(object sender, RoutedEventArgs e)
        {
            controlUnderTest.TopLeft = new Point(controlUnderTest.CurrentX, controlUnderTest.CurrentY); 
        }

        private void OnClickLast(object sender, RoutedEventArgs e)
        {
            controlUnderTest.BottomRight = new Point(controlUnderTest.CurrentX, controlUnderTest.CurrentY);
            controlUnderTest.generateGrid();
        }

        /* This event handler is not needed, since function can be done in XAML
         * private void comBoxMode_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            string selected = comBoxMode.SelectedItem.ToString();
            if (controlUnderTest != null)
            {
                if (string.Equals(selected, "ListMode"))
                    controlUnderTest.Mode = TilesDisplay.TilingMode.ListMode;
                else if (string.Equals(selected, "TopLeftMode"))
                    controlUnderTest.Mode = TilesDisplay.TilingMode.TopLeftMode;
                else
                    controlUnderTest.Mode = TilesDisplay.TilingMode.TopLeftBottomRightMode;
            }
        }*/

        private void OnClickSelect(object sender, RoutedEventArgs e)
        {
            controlUnderTest.addChildControlToCanvas(controlUnderTest.CurrentX,controlUnderTest.CurrentY);
            //controlUnderTest.addChildControlToCanvas(800, 300);
            //controlUnderTest.addChildControlToCanvas(300, 700);
            //controlUnderTest.addChildControlToCanvas(400, 500);
            //MessageBox.Show(controlUnderTest.ActualHeight.ToString);
        }
        /// <summary>
        /// get the scale factor
        /// </summary>
        /// <param name="which">positive for Mode 1 and mode 2, else mode 3</param>
        /// <returns></returns>
        private double getScaleFactor(int which)
        {
            if(which > 0)
                return controlUnderTest.getGridScaleFactor();
            else
                return controlUnderTest.getCanvasScaleFactor(controlUnderTest.MaxX-controlUnderTest.MinX, 
                                                                controlUnderTest.MaxY-controlUnderTest.MinY);
        }

        private void OnClickStepNext(object sender, RoutedEventArgs e)
        {
            controlUnderTest.stepThrough();
        }

        private void OnClickStartExp(object sender, RoutedEventArgs e)
        {
            controlUnderTest.testSetup();
        }

    }
}
