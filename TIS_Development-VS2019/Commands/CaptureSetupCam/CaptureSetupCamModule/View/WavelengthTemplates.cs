using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.ComponentModel;
using System.Collections.ObjectModel;

namespace CaptureSetupDll
{
    public class WavelengthTemplate : INotifyPropertyChanged
    {
        # region fields

        private string _name;
        private double _exposure;

        # endregion fields

        public event PropertyChangedEventHandler PropertyChanged;

        public WavelengthTemplate()
        {
        }

        public WavelengthTemplate(string name)
        {
            this._name = name;
        }

        # region functions

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

        # endregion functions

        #region properties

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
        #endregion properties

    }
    public class WavelengthTemplates : ObservableCollection<WavelengthTemplate>
    {
        public WavelengthTemplates()
            : base()
        {
        }
    }
}
