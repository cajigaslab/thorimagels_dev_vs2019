namespace ScriptManagerTestApp
{
    using System;
    using System.Collections.Generic;
    using System.Configuration;
    using System.Data;
    using System.Linq;
    using System.Windows;

    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        #region Constructors

        public App()
        {
            var bootstrapper = new Bootstrapper();
            bootstrapper.Run();

            bootstrapper.ShowInitialPanel();
        }

        #endregion Constructors
    }
}