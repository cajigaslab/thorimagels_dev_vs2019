namespace KuriosControl.Model
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Runtime.InteropServices;
    using System.Timers;
    using System.Windows;
    using System.Windows.Data;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Threading;
    using System.Xml;

    using ThorLogging;

    using ThorSharedTypes;

    class KuriosControlModel : ThorSharedTypes.VMBase
    {
        #region Fields

        string _kuriosCurrentWavelengthSequenceName = string.Empty;
        int _kuriosSequenceSteps = 32;
        private int _kuriosStartWL = 420;
        private int _kuriosStepSizeWL = 10;
        private int _kuriosStopWL = 730;

        #endregion Fields

        #region Constructors

        public KuriosControlModel()
        {
        }

        #endregion Constructors

        #region Properties

        public int KuriosBandwidthMode
        {
            get
            {
                int temp = 0;

                GetDeviceParamInt((int)SelectedHardware.SELECTED_SPECTRUMFILTER, (int)IDevice.Params.PARAM_KURIOS_BANDWIDTHMODE, ref temp);

                return temp;
            }
            set
            {
                StopSequence();
                SetDeviceParamInt((int)SelectedHardware.SELECTED_SPECTRUMFILTER, (int)IDevice.Params.PARAM_KURIOS_BANDWIDTHMODE, value, false);
            }
        }

        public int KuriosControlMode
        {
            get
            {
                int temp = 0;

                GetDeviceParamInt((int)SelectedHardware.SELECTED_SPECTRUMFILTER, (int)IDevice.Params.PARAM_KURIOS_CONTROLMODE, ref temp);

                return temp;
            }
            set
            {
                SetDeviceParamInt((int)SelectedHardware.SELECTED_SPECTRUMFILTER, (int)IDevice.Params.PARAM_KURIOS_CONTROLMODE, value, false);
            }
        }

        public string KuriosCurrentWavelengthSequenceName
        {
            get
            {
                return _kuriosCurrentWavelengthSequenceName;
            }
            set
            {
                _kuriosCurrentWavelengthSequenceName = value;
            }
        }

        public int KuriosStartWL
        {
            get
            {
                return _kuriosStartWL;
            }
            set
            {
                CalculateKuriosSequenceSteps();
                _kuriosStartWL = value;
            }
        }

        public int KuriosStepSizeWL
        {
            get
            {
                return _kuriosStepSizeWL;
            }
            set
            {
                CalculateKuriosSequenceSteps();
                _kuriosStepSizeWL = value;
            }
        }

        public int KuriosStopWL
        {
            get
            {
                return _kuriosStopWL;
            }
            set
            {
                CalculateKuriosSequenceSteps();
                _kuriosStopWL = value;
            }
        }

        public int KuriosWavelength
        {
            get
            {
                int temp = 0;

                GetDeviceParamInt((int)SelectedHardware.SELECTED_SPECTRUMFILTER, (int)IDevice.Params.PARAM_KURIOS_WAVELENGTH, ref temp);

                return temp;
            }
            set
            {
                int temp = 0;

                GetDeviceParamInt((int)SelectedHardware.SELECTED_SPECTRUMFILTER, (int)IDevice.Params.PARAM_KURIOS_WAVELENGTH, ref temp);
                SetDeviceParamInt((int)SelectedHardware.SELECTED_SPECTRUMFILTER, (int)IDevice.Params.PARAM_KURIOS_WAVELENGTH, value, false);
                System.Threading.Thread.Sleep(Math.Abs(temp - value) / 4);
            }
        }

        public int KuriosWavelengthMax
        {
            get
            {
                int min = 0;
                int max = 0;
                int def = 0;

                GetDeviceParamRangeInt((int)SelectedHardware.SELECTED_SPECTRUMFILTER, (int)IDevice.Params.PARAM_KURIOS_WAVELENGTH, ref min, ref max, ref def);

                return max;
            }
        }

        public int KuriosWavelengthMin
        {
            get
            {
                int min = 0;
                int max = 0;
                int def = 0;

                GetDeviceParamRangeInt((int)SelectedHardware.SELECTED_SPECTRUMFILTER, (int)IDevice.Params.PARAM_KURIOS_WAVELENGTH, ref min, ref max, ref def);

                return min;
            }
        }

        public int KuriousSequenceSteps
        {
            get
            {
                return _kuriosSequenceSteps;
            }
            set
            {
                _kuriosSequenceSteps = value;
            }
        }

        #endregion Properties

        #region Methods

        public void StopSequence()
        {
            SetDeviceParamInt((int)ThorSharedTypes.SelectedHardware.SELECTED_SPECTRUMFILTER,
                (int)ThorSharedTypes.IDevice.Params.PARAM_KURIOS_CONTROLMODE,
                (int)ThorSharedTypes.IDevice.KuriosControlMode.MANUAL,
                false);
        }

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "GetDeviceParamLong")]
        private static extern int GetDeviceParamInt(int deviceSelection, int paramId, ref int param);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "GetDeviceParamRangeLong")]
        private static extern int GetDeviceParamRangeInt(int deviceSelection, int paramId, ref int valMin, ref int valMax, ref int valDefault);

        [DllImport(".\\Modules_Native\\HardwareCom.dll", EntryPoint = "SetDeviceParamLong")]
        private static extern int SetDeviceParamInt(int deviceSelection, int paramId, int param, bool wait);

        private void CalculateKuriosSequenceSteps()
        {
            _kuriosSequenceSteps = (int)Math.Floor((double)Math.Abs(this.KuriosStopWL - this.KuriosStartWL) / this.KuriosStepSizeWL) + 1;
        }

        #endregion Methods
    }
}