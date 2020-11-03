using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.ComponentModel;
using System.Collections.ObjectModel;

namespace CaptureSetupDll
{
    public class LocalTemplate : INotifyPropertyChanged
    {

        # region fields

        private string _name;

        # endregion fields

        public event PropertyChangedEventHandler PropertyChanged;

        public LocalTemplate()
        {
        }

        public LocalTemplate(string name)
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
    public class LocalTemplates : ObservableCollection<LocalTemplate>
    {
        public LocalTemplates()
            : base()
        {
          // Add(new LocalTemplate("Test Item 1"));
        }
    }
}
