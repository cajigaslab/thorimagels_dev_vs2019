using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Media;
using Telerik.Windows.Controls;

namespace ThorDAQConfigControl
{
    public class ApplicationTheme
    {
        public ApplicationTheme()
        {
        }

        public static void Init()
        {
            // load resources
            var telerikStyleFiles = new List<string>
            {
                "Telerik.Windows.Controls.xaml",
                "Telerik.Windows.Controls.Input.xaml",
                "Telerik.Windows.Controls.Navigation.xaml",
                "Telerik.Windows.Controls.Chart.xaml"
            };

            foreach (string str in telerikStyleFiles)
            {
                string uriString = string.Format("/Telerik.Windows.Themes.Fluent;component/Themes/" + str);
                ResourceDictionary item = new ResourceDictionary
                {
                    Source = new Uri(uriString, UriKind.RelativeOrAbsolute)
                };
                if (IsNewResourceDictionary(item))
                    Application.Current.Resources.MergedDictionaries.Add(item);
            }

            ResourceDictionary dict = new ResourceDictionary
            {
                Source = new Uri(string.Format("/ThorDAQConfigControl;component/ApplicationStyles.xaml"), UriKind.RelativeOrAbsolute)
            };
            Application.Current.Resources.MergedDictionaries.Add(dict);

            // font
            FluentPalette.Palette.FontSizeS = 10;
            FluentPalette.Palette.FontSize = 12;
            FluentPalette.Palette.FontSizeL = 14;
            FluentPalette.Palette.FontFamily = new FontFamily("Segoe UI");

            //dark
            FluentPalette.Palette.AccentColor = (Color)ColorConverter.ConvertFromString("#FF0086AF");
            FluentPalette.Palette.AccentFocusedColor = (Color)ColorConverter.ConvertFromString("#FF009fc4");
            FluentPalette.Palette.AccentMouseOverColor = (Color)ColorConverter.ConvertFromString("#FF00BFE8");
            FluentPalette.Palette.AccentPressedColor = (Color)ColorConverter.ConvertFromString("#FF005B70");
            FluentPalette.Palette.AlternativeColor = (Color)ColorConverter.ConvertFromString("#FF2B2B2B");
            FluentPalette.Palette.BasicColor = (Color)ColorConverter.ConvertFromString("#4CFFFFFF");
            FluentPalette.Palette.BasicSolidColor = (Color)ColorConverter.ConvertFromString("#FF4C4C4C");
            FluentPalette.Palette.ComplementaryColor = (Color)ColorConverter.ConvertFromString("#FF333333");
            FluentPalette.Palette.IconColor = (Color)ColorConverter.ConvertFromString("#CCFFFFFF");
            FluentPalette.Palette.MainColor = (Color)ColorConverter.ConvertFromString("#33FFFFFF");
            FluentPalette.Palette.MarkerColor = (Color)ColorConverter.ConvertFromString("#FFFFFFFF");
            FluentPalette.Palette.MarkerInvertedColor = (Color)ColorConverter.ConvertFromString("#FFFFFFFF");
            FluentPalette.Palette.MarkerMouseOverColor = (Color)ColorConverter.ConvertFromString("#FF000000");
            FluentPalette.Palette.MouseOverColor = (Color)ColorConverter.ConvertFromString("#4CFFFFFF");
            FluentPalette.Palette.PressedColor = (Color)ColorConverter.ConvertFromString("#26FFFFFF");
            FluentPalette.Palette.PrimaryBackgroundColor = (Color)ColorConverter.ConvertFromString("#FF0D0D0D");
            FluentPalette.Palette.PrimaryColor = (Color)ColorConverter.ConvertFromString("#66FFFFFF");
            FluentPalette.Palette.PrimaryMouseOverColor = (Color)ColorConverter.ConvertFromString("#FFFFFFFF");
            FluentPalette.Palette.ReadOnlyBackgroundColor = (Color)ColorConverter.ConvertFromString("#00FFFFFF");
            FluentPalette.Palette.ReadOnlyBorderColor = (Color)ColorConverter.ConvertFromString("#FF4C4C4C");
            FluentPalette.Palette.ValidationColor = (Color)ColorConverter.ConvertFromString("#FFE81123");
            FluentPalette.Palette.DisabledOpacity = 0.3;
            FluentPalette.Palette.InputOpacity = 1;
            FluentPalette.Palette.ReadOnlyOpacity = 0.5;
        }

        private static bool IsNewResourceDictionary(ResourceDictionary dict)
        {
            for (int i = 0; i < Application.Current.Resources.MergedDictionaries.Count; i++)
            {
                if (Application.Current.Resources.MergedDictionaries[i].Source == dict.Source)
                    return false;
            }
            return true;
        }
    }
}
