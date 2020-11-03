namespace OverlayManagerTest.Commands
{
    using System;
    using System.Collections.Generic;
    using System.Text;
    using System.Windows;
    using System.Windows.Input;

    static class Commands
    {
        #region Fields

        // command definition
        public static readonly RoutedCommand ToggleCommand = new CustomCommand("ToggleCommand", typeof(Window));

        #endregion Fields
    }
}