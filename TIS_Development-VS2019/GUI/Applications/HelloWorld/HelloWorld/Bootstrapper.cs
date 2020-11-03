using Microsoft.Practices.Composite.Modularity;
using Microsoft.Practices.Composite.UnityExtensions;
using System.Windows;


namespace HelloWorld
{
    class Bootstrapper : UnityBootstrapper
    {

        protected override DependencyObject CreateShell()
        {
            Shell shell = new Shell();
            shell.Show();
            return shell;
        }

        protected override IModuleEnumerator GetModuleEnumerator()
        {
            return new DirectoryLookupModuleEnumerator(@".\Modules");
        }

    }


}
