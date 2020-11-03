using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.ComponentModel;
using System.Collections.ObjectModel;

namespace CaptureSetupDll
{
    public class ExperimentTemplate : INotifyPropertyChanged
    {
        # region fields

        private string _name;
        private long _id;

        # endregion fields

        public event PropertyChangedEventHandler PropertyChanged;

        public ExperimentTemplate()
        {
        }

        public ExperimentTemplate(string name)
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

        # region properties

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

        # endregion properties
    }
    public class ExperimentTemplates : ObservableCollection<ExperimentTemplate>
    {
        public ExperimentTemplates()
            : base()
        {
        }
    }
}
