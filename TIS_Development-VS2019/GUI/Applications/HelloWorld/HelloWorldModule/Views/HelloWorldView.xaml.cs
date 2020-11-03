
using System.Windows.Controls;
using Microsoft.Practices.Composite.Modularity;
using Microsoft.Practices.Composite.Regions;
using Microsoft.Practices.Unity;

namespace HelloWorldModule
{
    /// <summary>
    /// Interaction logic for HelloWorldView.xaml
    /// </summary>
    public partial class HelloWorldView : UserControl , IHellowWorldView
    {
        private HelloWorldPresenter presenter;

        public HelloWorldView()
        {
            InitializeComponent();
        }
        
        public HelloWorldView(HelloWorldPresenter presenter) 
            : this()
        {
            this.presenter = presenter;
            presenter.View = this;
        }

        #region IHelloWorldView Members

        public void ShowDialog(string content)
        {
        }

        public string Message
        {
            set { presenter.Message = value; }
        }
            
        #endregion
    }
}
