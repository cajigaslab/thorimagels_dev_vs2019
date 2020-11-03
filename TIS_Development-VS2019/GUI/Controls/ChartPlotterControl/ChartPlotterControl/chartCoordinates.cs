using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.ComponentModel;

namespace ChartPlotterControl
{
    public class chartCoordinates :INotifyPropertyChanged
    {   
        int _coordinateIndex;
        double _xData;
        double _yData;
        public int coordinateIndex
        {
            
            get
            {
              return _coordinateIndex;
            }
            set { _coordinateIndex = value; }
        }
        public double xData
        {
            get { return _xData; }
            set { _xData = value; }
        }
        public double yData
        {
            get { return _yData; }
            set{_yData=value;}
        }

        #region INotifyPropertyChanged Members

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion
    }
}
