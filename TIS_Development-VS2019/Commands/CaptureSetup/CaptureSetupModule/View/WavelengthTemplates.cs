namespace CaptureSetupDll
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.ComponentModel;
    using System.Linq;
    using System.Text;

    public class WavelengthTemplate : INotifyPropertyChanged
    {
        #region Fields

        private double _exposure;
        private string _name;

        #endregion Fields

        #region Constructors

        public WavelengthTemplate()
        {
        }

        public WavelengthTemplate(string name)
        {
            this._name = name;
        }

        #endregion Constructors

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public double Exposure
        {
            get { return _exposure; }
            set
            {
                _exposure = value;
                OnPropertyChanged("Exposure");
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

    public class WavelengthTemplates : ObservableCollection<WavelengthTemplate>
    {
        #region Constructors

        public WavelengthTemplates()
            : base()
        {
        }

        #endregion Constructors
    }
}