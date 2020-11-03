using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections.ObjectModel;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows;

namespace SampleRegionSelection
{   

    abstract class SampleCollectionBase
    {
        public abstract ObservableCollection<Control> CreateObservableCollection();
        public abstract Control AddControl(Color color, string name, string content, string toolTip, RoutedEventHandler eventHandler);
    }   

    class SampleCollectionButton : SampleCollectionBase
    {
        public override ObservableCollection<Control> CreateObservableCollection()
        {
            ObservableCollection<Control> collection = new ObservableCollection<Control>();

            return collection;
        }

        public override Control AddControl(Color color, string name, string content, string toolTip,RoutedEventHandler eventHandler)
        {
            Button btnWell = new Button();
            btnWell.Visibility = Visibility.Visible;
            btnWell.Name = name;           
            btnWell.Margin = new Thickness(0, 0, 0, 0);
            btnWell.Background = new SolidColorBrush(color);
            btnWell.IsHitTestVisible = true;
            btnWell.Click += eventHandler;
            btnWell.Style = null;
            return btnWell;
         }
    }  

    class SampleCollectionLabel : SampleCollectionBase
    {
        public override ObservableCollection<Control> CreateObservableCollection()
        {
            ObservableCollection<Control> collection = new ObservableCollection<Control>();

            return collection;
        }

        public override Control AddControl(Color color, string name, string content, string toolTip, RoutedEventHandler eventHandler)
        {
            Label lbl = new Label();
            lbl.Visibility = Visibility.Visible;
            lbl.Name = name;
            lbl.ToolTip = toolTip;
            lbl.VerticalAlignment = VerticalAlignment.Center;
            lbl.HorizontalAlignment = HorizontalAlignment.Center;
            lbl.Margin = new Thickness(0, 0, 0, 0);
            lbl.Content = content;
            lbl.Foreground = new SolidColorBrush(color); //white color
            return lbl;
        }
    }

    class SampleCollectionTextBlock : SampleCollectionBase
    {
        public override ObservableCollection<Control> CreateObservableCollection()
        {
            ObservableCollection<Control> collection = new ObservableCollection<Control>();

            return collection;
        }

        public override Control AddControl(Color color, string name, string content, string toolTip, RoutedEventHandler eventHandler)
        {
            Label txt = new Label();
            txt.Visibility = Visibility.Visible;
            txt.Name = name;
            txt.Content = content;                      
            txt.BorderThickness = new Thickness(0);
            txt.Margin = new Thickness(0, 0, 0, 0);
            txt.Background = new SolidColorBrush(color);
            txt.IsHitTestVisible = true;
            txt.ToolTip = toolTip;  

            return txt;
        }
    }

    class SampleRegionFactory
    {
        public SampleCollectionBase GetControl(int type)
        {
            SampleCollectionBase collection = null;
            switch (type)
            {
                case 0:
                    collection = new SampleCollectionButton();
                    break;
                case 1:
                    collection = new SampleCollectionLabel();
                    break;
                case 2:
                    collection = new SampleCollectionTextBlock();
                    break;
            }
            return collection;
        }

    }
   
}
