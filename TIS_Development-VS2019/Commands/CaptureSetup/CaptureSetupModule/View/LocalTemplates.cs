namespace CaptureSetupDll
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.ComponentModel;
    using System.Linq;
    using System.Text;

    public class LocalTemplate : INotifyPropertyChanged
    {
        #region Fields

        private string _name;

        #endregion Fields

        #region Constructors

        public LocalTemplate()
        {
        }

        public LocalTemplate(string name)
        {
            this._name = name;
        }

        #endregion Constructors

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public string Name
        {
            get { return _name; }
            set
            {
                _name = value;
                OnPropertyChanged("Name");
            }
        }

        #endregion Properties

        #region Methods

        public override string ToString()
        {
            return _name.ToString();
        }

        protected void OnPropertyChanged(string info)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null)
            {
                handler(this, new PropertyChangedEventArgs(info));
            }
        }

        #endregion Methods
    }

    public class LocalTemplates : ObservableCollection<LocalTemplate>
    {
        #region Constructors

        public LocalTemplates()
            : base()
        {
            // Add(new LocalTemplate("Test Item 1"));
        }

        #endregion Constructors
    }
}