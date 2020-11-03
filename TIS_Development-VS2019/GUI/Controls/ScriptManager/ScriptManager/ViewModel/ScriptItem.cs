namespace ScriptManagerDll.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Linq;
    using System.Text;
    using System.Windows.Controls;
    using System.Xml;

    public class ScriptItem : INotifyPropertyChanged
    {
        #region Fields

        private string _description;
        private int _lineNumber;
        private string _name;
        private string _params = null;

        #endregion Fields

        #region Constructors

        public ScriptItem()
        {
            Name = string.Empty;
        }

        public ScriptItem(string str)
        {
            Name = str;
        }

        #endregion Constructors

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public string Description
        {
            get
            {
                return _description;
            }
            set
            {
                _description = value;
                OnPropertyChanged("Description");
            }
        }

        public Image Icon
        {
            get; set;
        }

        public Guid Id
        {
            get; set;
        }

        public int LineNumber
        {
            get
            {
                return _lineNumber;
            }
            set
            {
                if (_lineNumber != value)
                {
                    _lineNumber = value;
                    OnPropertyChanged("LineNumber");
                }

            }
        }

        public string Name
        {
            get { return _name; }

            set
            {
                if (_name != value)
                {
                    _name = value;
                    OnPropertyChanged("Name");
                }
            }
        }

        public string Notes
        {
            get; set;
        }

        public string Paramters
        {
            get
            {
                return _params;
            }
            set
            {
                _params = value;
                OnPropertyChanged("Parameters");
            }
        }

        #endregion Properties

        #region Methods

        public override string ToString()
        {
            return Name;
        }

        void OnPropertyChanged(string propertyName)
        {
            if (PropertyChanged != null)
                PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
        }

        #endregion Methods
    }
}