namespace FLIMFitting
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Linq;
    using System.Runtime.CompilerServices;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows.Input;

    /// <summary>
    /// Base class for all ViewModel classes in the application. Provides support for 
    /// property changes notification.
    /// </summary>
    public abstract class BindableBase : INotifyPropertyChanged
    {
        #region Events

        /// <summary>
        /// Raised when a property on this object has a new value.
        /// </summary>
        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Methods

        /// <summary>
        /// Warns the developer if this object does not have a public property with
        /// the specified name. This method does not exist in a Release build.
        /// </summary>
        [Conditional("DEBUG")]
        [DebuggerStepThrough]
        public void VerifyPropertyName(string propertyName)
        {
            // verify that the property name matches a real,
            // public, instance property on this object.
            // or is empty, indicating a call to refresh all
            // bindings
            if (TypeDescriptor.GetProperties(this)[propertyName] == null && propertyName != "")
            {
                Debug.Fail("Invalid property name: " + propertyName);
            }
        }

        /// <summary>
        /// Raises this object's PropertyChanged event.
        /// </summary>
        /// <param name="propertyName">The name of the property that has a new value.</param>
        protected virtual void OnPropertyChanged(string propertyName)
        {
            this.VerifyPropertyName(propertyName);

            if (this.PropertyChanged != null)
            {
                this.PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }

        protected virtual bool SetProperty<T>(
            ref T backingStore, T value,
            [CallerMemberName]string propertyName = "")
        {
            //if value didn't change
            if (EqualityComparer<T>.Default.Equals(backingStore, value))
                return false;

            backingStore = value;
            OnPropertyChanged(propertyName);
            return true;
        }

        #endregion Methods
    }

    /// <summary>
    /// A command whose sole purpose is to relay its functionality to other
    /// objects by invoking delegates. The default return value for the CanExecute
    /// method is 'true'.
    /// </summary>
    public class RelayCommand : ICommand
    {
        #region Fields

        private readonly Func<bool> canExecute;
        private readonly Action execute;

        #endregion Fields

        #region Constructors

        /// <summary>
        /// Initializes a new instance of the RelayCommand class
        /// </summary>
        /// <param name="execute">The execution logic.</param>
        public RelayCommand(Action execute)
            : this(execute, null)
        {
        }

        /// <summary>
        /// Initializes a new instance of the RelayCommand class
        /// </summary>
        /// <param name="execute">The execution logic.</param>
        /// <param name="canExecute">The execution status logic.</param>
        public RelayCommand(Action execute, Func<bool> canExecute)
        {
            if (execute == null)
                throw new ArgumentNullException("execute");

            this.execute = execute;
            this.canExecute = canExecute;
        }

        #endregion Constructors

        #region Events

        public event EventHandler CanExecuteChanged
        {
            // wire the CanExecutedChanged event only if the canExecute func
            // is defined (that improves perf when canExecute is not used)
            add
            {
                if (this.canExecute != null)
                    CommandManager.RequerySuggested += value;
            }
            remove
            {
                if (this.canExecute != null)
                    CommandManager.RequerySuggested -= value;
            }
        }

        #endregion Events

        #region Methods

        public bool CanExecute(object parameter)
        {
            return this.canExecute == null ? true : this.canExecute();
        }

        public void Execute(object parameter)
        {
            this.execute();
        }

        #endregion Methods
    }

    public class RelayCommandWithParam : ICommand
    {
        #region Fields

        private readonly Func<bool> canExecute;
        private readonly Action<object> execute;

        #endregion Fields

        #region Constructors

        /// <summary>
        /// Initializes a new instance of the RelayCommand class
        /// </summary>
        /// <param name="execute">The execution logic.</param>
        //public RelayCommand(Action execute)
        //    : this(execute, null)
        //{
        //}
        public RelayCommandWithParam(Action<object> execute)
            : this(execute, null)
        {
        }

        /// <summary>
        /// Initializes a new instance of the RelayCommand class
        /// </summary>
        /// <param name="execute">The execution logic.</param>
        /// <param name="canExecute">The execution status logic.</param>
        //public RelayCommand(Action execute, Func<bool> canExecute)
        //{
        //    if (execute == null)
        //        throw new ArgumentNullException("execute");
        //    this.execute = execute;
        //    this.canExecute = canExecute;
        //}
        public RelayCommandWithParam(Action<object> execute, Func<bool> canExecute)
        {
            if (execute == null)
                throw new ArgumentNullException("execute");

            this.execute = execute;
            this.canExecute = canExecute;
        }

        #endregion Constructors

        #region Events

        public event EventHandler CanExecuteChanged
        {
            // wire the CanExecutedChanged event only if the canExecute func
            // is defined (that improves perf when canExecute is not used)
            add
            {
                if (this.canExecute != null)
                    CommandManager.RequerySuggested += value;
            }
            remove
            {
                if (this.canExecute != null)
                    CommandManager.RequerySuggested -= value;
            }
        }

        #endregion Events

        #region Methods

        public bool CanExecute(object parameter)
        {
            return this.canExecute == null ? true : this.canExecute();
        }

        public void Execute(object parameter)
        {
            this.execute(parameter);
        }

        #endregion Methods
    }
}