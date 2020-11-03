namespace MultiphotonControl.Model
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

    public class MultiphotonControlModel
    {
        #region Fields

        double _beamStabilizerBPACentroidX = 0.0;
        double _beamStabilizerBPACentroidY = 0.0;
        double _beamStabilizerBPAExposure = 0.0;
        double _beamStabilizerBPBCentroidX = 0.0;
        double _beamStabilizerBPBCentroidY = 0.0;
        double _beamStabilizerBPBExposure = 0.0;
        bool _beamStabilizerEnableReadData = false;
        int _beamStabilizerPiezo1Pos = 0;
        int _beamStabilizerPiezo2Pos = 0;
        int _beamStabilizerPiezo3Pos = 0;
        int _beamStabilizerPiezo4Pos = 0;
        int _currentLaser1Position = 0;
        bool _isCurLaser1PosDirt;
        int _laser1shutterPosition = 0;
        private DateTime _lastBeamStablizerUpdate = DateTime.Now;

        #endregion Fields

        #region Constructors

        /// <summary>
        /// Create a new instance of the MultiphotonControlModel class
        /// </summary>
        public MultiphotonControlModel()
        {
        }

        #endregion Constructors

        #region Properties

        public bool BeamStabilizerAvailable
        {
            get
            {
                int val = 0;
                if (1 == ThorSharedTypes.ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_BEAMSTABILIZER, (int)IDevice.Params.PARAM_BEAM_STABILIZER_REALIGNBEAM, ref val))
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
        }

        public double BeamStabilizerBPACentroidX
        {
            get
            {
                return _beamStabilizerBPACentroidX;
            }
        }

        public double BeamStabilizerBPACentroidY
        {
            get
            {
                return _beamStabilizerBPACentroidY;
            }
        }

        public double BeamStabilizerBPAExposure
        {
            get
            {
                return _beamStabilizerBPAExposure;
            }
        }

        public double BeamStabilizerBPBCentroidX
        {
            get
            {
                return _beamStabilizerBPBCentroidX;
            }
        }

        public double BeamStabilizerBPBCentroidY
        {
            get
            {
                return _beamStabilizerBPBCentroidY;
            }
        }

        public double BeamStabilizerBPBExposure
        {
            get
            {
                return _beamStabilizerBPBExposure;
            }
        }

        public bool BeamStabilizerEnableReadData
        {
            get
            {
                return _beamStabilizerEnableReadData;
            }
            set
            {
                _beamStabilizerEnableReadData = value;
            }
        }

        public int BeamStabilizerPiezo1Pos
        {
            get
            {
                return _beamStabilizerPiezo1Pos;
            }
        }

        public int BeamStabilizerPiezo2Pos
        {
            get
            {
                return _beamStabilizerPiezo2Pos;
            }
        }

        public int BeamStabilizerPiezo3Pos
        {
            get
            {
                return _beamStabilizerPiezo3Pos;
            }
        }

        public int BeamStabilizerPiezo4Pos
        {
            get
            {
                return _beamStabilizerPiezo4Pos;
            }
        }

        public bool BeamStablizerQuery
        {
            set
            {
                if ((value) && (_beamStabilizerEnableReadData))
                {
                    TimeSpan ts = DateTime.Now - _lastBeamStablizerUpdate;
                    if (ts.TotalSeconds > 0.05)
                    {
                        ReadBeamStabilizer();
                        _lastBeamStablizerUpdate = DateTime.Now;
                    }
                }
            }
        }

        public double BSDeadBand
        {
            get;
            set;
        }

        public double BSExpLimit
        {
            get;
            set;
        }

        public double BSExpLimitMin
        {
            get;
            set;
        }

        public double BSPiezoLimit
        {
            get;
            set;
        }

        public int Laser1Position
        {
            get
            {
                int temp = 0;

                if (_isCurLaser1PosDirt)
                {
                    ThorSharedTypes.ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_LASER1, (int)IDevice.Params.PARAM_LASER1_POS, ref temp);

                    _currentLaser1Position = temp;
                    _isCurLaser1PosDirt = false;
                }
                else
                {
                    temp = _currentLaser1Position;
                }

                return temp;
            }
            set
            {
                _isCurLaser1PosDirt = true;

                ThorSharedTypes.ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_LASER1, (int)IDevice.Params.PARAM_LASER1_POS, value, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT);
            }
        }

        public bool LaserShutter2Available
        {
            get
            {
                int val = 0;
                if (1 == ThorSharedTypes.ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_LASER1, (int)IDevice.Params.PARAM_LASER1_SHUTTER2_POS_CURRENT, ref val))
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
        }

        public int LaserShutter2Position
        {
            get
            {
                int val = 0;
                ThorSharedTypes.ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_LASER1, (int)IDevice.Params.PARAM_LASER1_SHUTTER2_POS_CURRENT, ref val);
                return val;
            }
            set
            {
                ThorSharedTypes.ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_LASER1, (int)IDevice.Params.PARAM_LASER1_SHUTTER2_POS, value, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT);
            }
        }

        public int LaserShutterPosition
        {
            get
            {
                int val = 0;
                ThorSharedTypes.ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_LASER1, (int)IDevice.Params.PARAM_LASER1_SHUTTER_POS_CURRENT, ref val);
                return val;
            }
            set
            {
                ThorSharedTypes.ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_LASER1, (int)IDevice.Params.PARAM_LASER1_SHUTTER_POS, value, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT);
            }
        }

        public DateTime LastBeamStablizerUpdate
        {
            get { return _lastBeamStablizerUpdate; }
            set { _lastBeamStablizerUpdate = value; }
        }

        #endregion Properties

        #region Methods

        public void RealignBeam()
        {
            ThorSharedTypes.ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_BEAMSTABILIZER, (int)IDevice.Params.PARAM_BEAM_STABILIZER_REALIGNBEAM, 1, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT);
        }

        public void ResetFactoryAlignment()
        {
            ThorSharedTypes.ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_BEAMSTABILIZER, (int)IDevice.Params.PARAM_BEAM_STABILIZER_FACTORY_RESET_PIEZOS, 1, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT);
        }

        private void ReadBeamStabilizer()
        {
            double valMin, valMax, valDef;
            valMin = valMax = valDef = 0;

            ThorSharedTypes.ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_BEAMSTABILIZER, (int)IDevice.Params.PARAM_BEAM_STABILIZER_BPA_CENTER_X, ref _beamStabilizerBPACentroidX);

            ThorSharedTypes.ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_BEAMSTABILIZER, (int)IDevice.Params.PARAM_BEAM_STABILIZER_BPA_CENTER_Y, ref _beamStabilizerBPACentroidY);

            ThorSharedTypes.ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_BEAMSTABILIZER, (int)IDevice.Params.PARAM_BEAM_STABILIZER_BPA_EXPOSURE, ref _beamStabilizerBPAExposure);

            ThorSharedTypes.ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_BEAMSTABILIZER, (int)IDevice.Params.PARAM_BEAM_STABILIZER_BPB_CENTER_X, ref _beamStabilizerBPBCentroidX);

            ThorSharedTypes.ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_BEAMSTABILIZER, (int)IDevice.Params.PARAM_BEAM_STABILIZER_BPB_CENTER_Y, ref _beamStabilizerBPBCentroidY);

            ThorSharedTypes.ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_BEAMSTABILIZER, (int)IDevice.Params.PARAM_BEAM_STABILIZER_BPB_EXPOSURE, ref _beamStabilizerBPBExposure);

            ThorSharedTypes.ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_BEAMSTABILIZER, (int)IDevice.Params.PARAM_BEAM_STABILIZER_PIEZO1_POS, ref _beamStabilizerPiezo1Pos);

            ThorSharedTypes.ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_BEAMSTABILIZER, (int)IDevice.Params.PARAM_BEAM_STABILIZER_PIEZO2_POS, ref _beamStabilizerPiezo2Pos);

            ThorSharedTypes.ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_BEAMSTABILIZER, (int)IDevice.Params.PARAM_BEAM_STABILIZER_PIEZO3_POS, ref _beamStabilizerPiezo3Pos);

            ThorSharedTypes.ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_BEAMSTABILIZER, (int)IDevice.Params.PARAM_BEAM_STABILIZER_PIEZO4_POS, ref _beamStabilizerPiezo4Pos);

            //GetDeviceParamInt((int)SelectedHardware.SELECTED_LASER1, (int)IDevice.Params.PARAM_LASER1_SHUTTER_POS_CURRENT, ref laser1shutterPosition);
            if (_laser1shutterPosition != LaserShutterPosition)
            {
                _laser1shutterPosition = LaserShutterPosition;

                MVMManager.Instance["MultiphotonControlViewModel", "Shutter1Open"] = (_laser1shutterPosition == 1) ? true : false;
                MVMManager.Instance["MultiphotonControlViewModel", "Shutter1Close"] = (_laser1shutterPosition == 0) ? true : false;

            }
            //retrieve the limits for the BeamStabilizer params
            ThorSharedTypes.ResourceManagerCS.GetDeviceParamRangeDouble((int)SelectedHardware.SELECTED_BEAMSTABILIZER, (int)IDevice.Params.PARAM_BEAM_STABILIZER_BPA_CENTER_X, ref valMin, ref valMax, ref valDef);
            BSDeadBand = valMax;
            ThorSharedTypes.ResourceManagerCS.GetDeviceParamRangeDouble((int)SelectedHardware.SELECTED_BEAMSTABILIZER, (int)IDevice.Params.PARAM_BEAM_STABILIZER_BPB_EXPOSURE, ref valMin, ref valMax, ref valDef);
            BSExpLimit = valMax;
            BSExpLimitMin = valMin;
            ThorSharedTypes.ResourceManagerCS.GetDeviceParamRangeDouble((int)SelectedHardware.SELECTED_BEAMSTABILIZER, (int)IDevice.Params.PARAM_BEAM_STABILIZER_PIEZO1_POS, ref valMin, ref valMax, ref valDef);
            BSPiezoLimit = valMax;
        }

        #endregion Methods
    }
}