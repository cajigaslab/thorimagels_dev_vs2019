namespace CaptureSetupDll
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.ComponentModel;
    using System.Linq;
    using System.Text;

    public class ExperimentTemplate : INotifyPropertyChanged
    {
        #region Fields

        private long _id;
        private string _name;

        #endregion Fields

        #region Constructors

        public ExperimentTemplate()
        {
        }

        public ExperimentTemplate(string name)
        {
            this._name = name;
        }

        #endregion Constructors

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public long Id
        {
            get { return _id; }
            set
            {
                _id = value;
                OnPropertyChanged("Id");
            }
        }

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

    public class ExperimentTemplates : ObservableCollection<ExperimentTemplate>
    {
        #region Constructors

        public ExperimentTemplates()
            : base()
        {
        }

        #endregion Constructors
    }
}