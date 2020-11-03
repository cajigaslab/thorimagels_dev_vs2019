namespace OverlayManagerTest.Commands
{
    using System;
    using System.Windows.Input;

    public class CustomCommand : RoutedCommand
    {
        #region Fields

        private object returnValue;

        #endregion Fields

        #region Constructors

        public CustomCommand(string Name, Type OnerType)
            : base(Name, OnerType)
        {
        }

        #endregion Constructors

        #region Delegates

        public delegate void ReturnValueChangedHandler(object sender, EventArgs e);

        #endregion Delegates

        #region Events

        //an event for command sources on return value change
        public event ReturnValueChangedHandler ReturnValueChanged;

        #endregion Events

        #region Properties

        //the return value
        public object ReturnValue
        {
            get
            {
                return returnValue;
            }
            set
            {
                returnValue = value;
                ReturnValueChanged(this, new EventArgs());
            }
        }

        #endregion Properties
    }
}