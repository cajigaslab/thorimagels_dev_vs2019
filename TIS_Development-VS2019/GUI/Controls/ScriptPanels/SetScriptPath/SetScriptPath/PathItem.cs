namespace SetScriptPath
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Linq;
    using System.Text;

    public class PathItem : INotifyPropertyChanged
    {
        #region Fields

        private string _alias;
        private string _value;

        #endregion Fields

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public string Alias
        {
            get
            {
                return _alias;
            }
            set
            {
                _alias = value;
                OnPropertyChanged("Alias");
            }
        }

        public string Value
        {
            get
            {
                return _value;
            }
            set
            {
                _value = value;
                OnPropertyChanged("Value");
            }
        }

        #endregion Properties

        #region Methods

        void OnPropertyChanged(string propertyName)
        {
            if (PropertyChanged != null)
                PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
        }

        #endregion Methods
    }
}