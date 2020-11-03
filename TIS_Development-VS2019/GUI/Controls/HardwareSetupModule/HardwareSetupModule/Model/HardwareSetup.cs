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
using System.IO;
using ThorLogging;

namespace HardwareSetupDll.ViewModel
{
    public class HardwareSetup : INotifyPropertyChanged
    {
        public event Action<string> Update;

        public event PropertyChangedEventHandler PropertyChanged;
     
        public HardwareSetup()
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
                //HardwareSetupdll is missing
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
                //HardwareSetupdll is missing
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
