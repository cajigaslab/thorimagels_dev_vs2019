namespace KuriosControl.Model
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Collections.Specialized;
    using System.ComponentModel;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;

    public class SequenceParameter : INotifyPropertyChanged
    {
        #region Fields

        private double _seqExposureStart;
        private double _seqExposureStop;
        private double _seqWavelengthStart;
        private double _seqWavelengthStep;
        private double _seqWavelengthStop;

        #endregion Fields

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public double SeqExposureStart
        {
            get { return _seqExposureStart; }
            set
            {
                _seqExposureStart = value;
                OnPropertyChanged("SeqExposureStart");
            }
        }

        public double SeqExposureStop
        {
            get { return _seqExposureStop; }
            set
            {
                _seqExposureStop = value;
                OnPropertyChanged("SeqExposureStop");
            }
        }

        public double SeqWavelengthStart
        {
            get { return _seqWavelengthStart; }
            set
            {
                _seqWavelengthStart = value;
                OnPropertyChanged("SeqWavelengthStart");
                OnPropertyChanged("StepsCount");
            }
        }

        public double SeqWavelengthStep
        {
            get { return _seqWavelengthStep; }
            set
            {
                _seqWavelengthStep = value;
                OnPropertyChanged("SeqStep");
                OnPropertyChanged("SeqWavelengthStep");
            }
        }

        public double SeqWavelengthStop
        {
            get { return _seqWavelengthStop; }
            set
            {
                _seqWavelengthStop = value;
                OnPropertyChanged("SeqWavelengthStop");
                OnPropertyChanged("StepsCount");
            }
        }

        public int StepsWavelengthCount
        {
            get
            {
                if (0 == _seqWavelengthStep) return 0;
                return (int)Math.Floor((double)Math.Abs(_seqWavelengthStop - _seqWavelengthStart) / _seqWavelengthStep) + 1;
            }
        }

        #endregion Properties

        #region Methods

        protected virtual void OnPropertyChanged(string propertyName)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null) handler(this, new PropertyChangedEventArgs(propertyName));
        }

        #endregion Methods
    }
}