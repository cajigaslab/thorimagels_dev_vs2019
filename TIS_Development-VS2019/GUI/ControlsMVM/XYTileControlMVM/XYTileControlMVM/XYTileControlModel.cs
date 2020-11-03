namespace XYTileControl.Model
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Reflection;
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

    public class XYTileControlModel
    {
        #region Fields

        private bool _enableXYRead = true;
        private DateTime _lastXUpdate = DateTime.Now;
        private DateTime _lastYUpdate = DateTime.Now;
        private double _xPosition;
        private double _yPosition;

        #endregion Fields

        #region Constructors

        /// <summary>
        /// Create a new instance of the XYTileControlModel class
        /// </summary>
        public XYTileControlModel()
        {
        }

        #endregion Constructors

        #region Properties

        public bool EnableXYRead
        {
            get { return _enableXYRead; }
        }

        public DateTime LastXUpdate
        {
            get { return _lastXUpdate; }
            set { _lastXUpdate = value; }
        }

        public DateTime LastYUpdate
        {
            get { return _lastYUpdate; }
            set { _lastYUpdate = value; }
        }

        public string UpdatePositions
        {
            set
            {
                switch (value)
                {
                    case "X":
                        if (GetXPosition(ref _xPosition))
                        {
                            Decimal dec = new Decimal(_xPosition);

                            _xPosition = Decimal.ToDouble(Decimal.Round(dec, 4));
                        }
                        break;
                    case "Y":
                        if (GetYPosition(ref _yPosition))
                        {
                            Decimal dec = new Decimal(_yPosition);

                            _yPosition = Decimal.ToDouble(Decimal.Round(dec, 4));
                        }
                        break;
                }
            }
        }

        public double XMax
        {
            get
            {
                double xMax = 0;
                double xMinPos = 0;
                double xMaxPos = 0;
                double xDefaultPos = 0;

                if (1 == ResourceManagerCS.GetDeviceParamRangeDouble((int)SelectedHardware.SELECTED_XYSTAGE, (int)IDevice.Params.PARAM_X_POS, ref xMinPos, ref xMaxPos, ref xDefaultPos))
                {
                    xMax = xMaxPos;
                }

                return xMax;
            }
        }

        public double XMin
        {
            get
            {
                double xMin = 0;
                double xMinPos = 0;
                double xMaxPos = 0;
                double xDefaultPos = 0;

                if (1 == ResourceManagerCS.GetDeviceParamRangeDouble((int)SelectedHardware.SELECTED_XYSTAGE, (int)IDevice.Params.PARAM_X_POS, ref xMinPos, ref xMaxPos, ref xDefaultPos))
                {
                    xMin = xMinPos;

                }

                return xMin;
            }
        }

        public double XPosition
        {
            get
            {
                return _xPosition;
            }
            set
            {
                //don't read while positioning
                _enableXYRead = false;
                double temp = value;

                if (1 == ResourceManagerCS.SetDeviceParamDouble((int)SelectedHardware.SELECTED_XYSTAGE, (int)IDevice.Params.PARAM_X_POS, temp, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT))
                {
                    temp = value;
                }
                _enableXYRead = true;
            }
        }

        public double YMax
        {
            get
            {
                double yMax = 0;
                double yMinPos = 0;
                double yMaxPos = 0;
                double yDefaultPos = 0;

                if (1 == ResourceManagerCS.GetDeviceParamRangeDouble((int)SelectedHardware.SELECTED_XYSTAGE, (int)IDevice.Params.PARAM_Y_POS, ref yMinPos, ref yMaxPos, ref yDefaultPos))
                {
                    yMax = yMaxPos;
                }

                return yMax;
            }
        }

        public double YMin
        {
            get
            {
                double yMin = 0;
                double yMinPos = 0;
                double yMaxPos = 0;
                double yDefaultPos = 0;

                if (1 == ResourceManagerCS.GetDeviceParamRangeDouble((int)SelectedHardware.SELECTED_XYSTAGE, (int)IDevice.Params.PARAM_Y_POS, ref yMinPos, ref yMaxPos, ref yDefaultPos))
                {
                    yMin = yMinPos;
                }

                return yMin;
            }
        }

        public double YPosition
        {
            get
            {
                return _yPosition;
            }
            set
            {
                //don't read while positioning
                _enableXYRead = false;
                double temp = value;

                if (1 == ResourceManagerCS.SetDeviceParamDouble((int)SelectedHardware.SELECTED_XYSTAGE, (int)IDevice.Params.PARAM_Y_POS, temp, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT))
                {
                    temp = value;
                }
                _enableXYRead = true;
            }
        }

        #endregion Properties

        #region Methods

        public int SetXZero()
        {
            return ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_XYSTAGE, (int)IDevice.Params.PARAM_X_ZERO, 1, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT);
        }

        public int SetYZero()
        {
            return ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_XYSTAGE, (int)IDevice.Params.PARAM_Y_ZERO, 1, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT);
        }

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "CaptureTiles")]
        private static extern bool CaptureTiles();

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "GetXPosition")]
        private static extern bool GetXPosition(ref double pos);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "GetYPosition")]
        private static extern bool GetYPosition(ref double pos);

        #endregion Methods
    }
}