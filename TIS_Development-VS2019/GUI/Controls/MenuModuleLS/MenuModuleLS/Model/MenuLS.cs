using System;
using System.Windows;
using System.ComponentModel;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Runtime.InteropServices;
using System.Windows.Threading;
using System.Timers;
using System.Diagnostics;
using System.Windows.Data;
using System.Globalization;
using ThorLogging;

namespace MenuLSDll.ViewModel
{
    public class MenuLS : INotifyPropertyChanged
    {

        public event PropertyChangedEventHandler PropertyChanged;
     

        public MenuLS()
        {    
            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Created");
        }
 
        public void Start()
        {
            try
            {
            }
            catch (System.DllNotFoundException)
            {
                //MenuLSdll is missing
            }
        }
        public void Stop()
        {
            try
            {
                //Abort the execute
            }
            catch (System.DllNotFoundException)
            {
                //MenuLSdll is missing
            }
        }

        
        protected virtual void OnPropertyChanged(string propertyName)
        {
            if (this.PropertyChanged != null)
            {
                this.PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }
    }
}
