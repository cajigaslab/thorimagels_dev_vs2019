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
using Microsoft.Practices.Composite.Regions;

namespace ScriptModule
{
    /// <summary>
    /// Interaction logic for ScriptView.xaml
    /// </summary>
    public partial class ScriptView : UserControl, IShowHelloView
    {
        private ShowHelloPresenter _presenter;

        public ScriptView()
        {
            InitializeComponent();
            this.button1.Click += new RoutedEventHandler(button1_Click);
        }

        public ScriptView(ShowHelloPresenter presenter)
            : this()
        {
            _presenter = presenter;
            _presenter.View = this;
        }

        public event EventHandler ShowHello = delegate { };

        private void button1_Click(object sender, RoutedEventArgs e)
        {
            ShowHello(this, null);
        }

        public string Message
        {
            get
            {
                return "MyMessage";

            }
        }
    }
}
