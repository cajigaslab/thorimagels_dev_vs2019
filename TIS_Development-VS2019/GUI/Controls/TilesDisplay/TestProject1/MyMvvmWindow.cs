using System;
using System.Windows;
using System.Windows.Controls;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using TilesDisplay;
using IcuTesting.WPF;

namespace TestProject1
{
    public class MyMvvmWindow : Window
    {
        public MyMvvmWindow()
        {
            AddChild(new Tiles());
        }
    }
}
