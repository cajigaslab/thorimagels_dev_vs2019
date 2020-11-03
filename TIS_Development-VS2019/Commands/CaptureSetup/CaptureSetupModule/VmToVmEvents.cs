namespace CaptureSetupDll
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;

    using Microsoft.Practices.Composite.Events;
    using Microsoft.Practices.Composite.Wpf.Events;

    public class ExposureChangeEvent : CompositeWpfEvent<double>
    {
    }

    public class LoadSampleEvent : CompositeWpfEvent<bool>
    {
    }

    public class MoveXYStageEvent : CompositeWpfEvent<TranslationInfo>
    {
    }

    public class SampleInfo
    {
        #region Fields

        private double _offsetX;
        private double _offsetY;

        #endregion Fields

        #region Properties

        public double OffsetX
        {
            set
            {
                _offsetX = value;
            }
            get
            {
                return _offsetX;
            }
        }

        public double OffsetY
        {
            set
            {
                _offsetY = value;
            }
            get
            {
                return _offsetY;
               }
        }

        #endregion Properties
    }

    public class StartLiveImageEvent : CompositeWpfEvent<bool>
    {
    }

    public class TranslationInfo
    {
        #region Fields

        private double _offsetX;
        private double _offsetY;
        private double _wellOffsetX;
        private double _wellOffsetY;

        #endregion Fields

        #region Properties

        public double OffsetX
        {
            set
            {
                _offsetX = value;
            }
            get
            {
                return _offsetX;
            }
        }

        public double OffsetY
        {
            set
            {
                _offsetY = value;
            }
            get
            {
                return _offsetY;
            }
        }

        public double WellOffsetX
        {
            set
            {
                _wellOffsetX = value;
            }
            get
            {
                return _wellOffsetX;
            }
        }

        public double WellOffsetY
        {
            set
            {
                _wellOffsetY = value;
            }
            get
            {
                return _wellOffsetY;
            }
        }

        #endregion Properties
    }
}