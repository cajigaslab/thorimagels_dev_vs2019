namespace PowerControl.Model
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

    public class PowerControlModel
    {
        #region Fields

        public const int MAX_POCKELS_COUNT = 4;
        public const int MAX_POWER_CTRLS = 6;

        string _filePockelsMask = string.Empty;
        private double _powerPosition = 0;
        private double _powerPosition2 = 0;
        private Dictionary<string, PropertyInfo> _properties = new Dictionary<string, PropertyInfo>();
        private int _shutterPosition = 0;

        #endregion Fields

        #region Constructors

        /// <summary>
        /// Create a new instance of the PowerControlModel class
        /// </summary>
        public PowerControlModel()
        {
        }

        #endregion Constructors

        #region Properties

        public bool BleacherPockelsEnable0
        {
            get
            {
                double pos = 0;
                return ((1 == ThorSharedTypes.ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_POCKELS_CONNECTED_0, ref pos)) && (1.0 == pos)) ? true : false;
            }
        }

        public bool BleacherPockelsEnable1
        {
            get
            {
                double pos = 0;
                return ((1 == ThorSharedTypes.ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_POCKELS_CONNECTED_1, ref pos)) && (1.0 == pos)) ? true : false;
            }
        }

        public bool BleacherPockelsEnable2
        {
            get
            {
                double pos = 0;
                return ((1 == ThorSharedTypes.ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_POCKELS_CONNECTED_2, ref pos)) && (1.0 == pos)) ? true : false;
            }
        }

        public bool BleacherPockelsEnable3
        {
            get
            {
                double pos = 0;
                return ((1 == ThorSharedTypes.ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_POCKELS_CONNECTED_3, ref pos)) && (1.0 == pos)) ? true : false;
            }
        }

        public double BleacherPower0
        {
            get
            {
                double pos = 0;

                ThorSharedTypes.ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_0, ref pos);

                Decimal dec = new Decimal(pos);

                pos = Decimal.ToDouble(Decimal.Round(dec, 2));

                return pos;
            }
            set
            {
                ThorSharedTypes.ResourceManagerCS.SetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_0, (double)value);
            }
        }

        public double BleacherPower1
        {
            get
            {
                double pos = 0;

                ThorSharedTypes.ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_1, ref pos);

                Decimal dec = new Decimal(pos);

                pos = Decimal.ToDouble(Decimal.Round(dec, 2));

                return pos;
            }
            set
            {
                ThorSharedTypes.ResourceManagerCS.SetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_1, (double)value);
            }
        }

        public double BleacherPower2
        {
            get
            {
                double pos = 0;

                ThorSharedTypes.ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_2, ref pos);

                Decimal dec = new Decimal(pos);

                pos = Decimal.ToDouble(Decimal.Round(dec, 2));

                return pos;
            }
            set
            {
                ThorSharedTypes.ResourceManagerCS.SetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_2, (double)value);
            }
        }

        public double BleacherPower3
        {
            get
            {
                double pos = 0;

                ThorSharedTypes.ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_3, ref pos);

                Decimal dec = new Decimal(pos);

                pos = Decimal.ToDouble(Decimal.Round(dec, 2));

                return pos;
            }
            set
            {
                ThorSharedTypes.ResourceManagerCS.SetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_3, (double)value);
            }
        }

        public PockelsResponseType BleacherPowerResponse0
        {
            get
            {
                int pos = 0;
                ThorSharedTypes.ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_POCKELS_RESPONSE_TYPE_0, ref pos);
                return (PockelsResponseType)pos;
            }
        }

        public PockelsResponseType BleacherPowerResponse1
        {
            get
            {
                int pos = 0;
                ThorSharedTypes.ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_POCKELS_RESPONSE_TYPE_1, ref pos);
                return (PockelsResponseType)pos;
            }
        }

        public double BleachPockelsVoltageMax0
        {
            get
            {
                double pos = 0;
                //if using thordaq, use the first available pockels line for bleaching
                if (WaveformDriverType == WaveformDriverType.WaveformDriver_ThorDAQ)
                {
                    for (int i =0; i < MAX_POCKELS_COUNT; ++i)
                    {
                        switch(i)
                        {
                            case 0:
                                {

                                    if (BleacherPockelsEnable0)
                                    {
                                        ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_POCKELS_MAX_VOLTAGE_0, ref pos);
                                        return pos;
                                    }
                                    break;
                                }
                            case 1:
                                {
                                    if (BleacherPockelsEnable1)
                                    {
                                        ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_POCKELS_MAX_VOLTAGE_1, ref pos);
                                        return pos;
                                    }
                                    break;
                                }
                            case 2:
                                {
                                    if (BleacherPockelsEnable2)
                                    {
                                        ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_POCKELS_MAX_VOLTAGE_2, ref pos);
                                        return pos;
                                    }
                                    break;
                                }
                            case 3:
                                {
                                    if (BleacherPockelsEnable3)
                                    {
                                        ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_POCKELS_MAX_VOLTAGE_3, ref pos);
                                        return pos;
                                    }
                                    break;
                                }
                        }
                    }
                }
                else
                {
                    ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_POCKELS_MAX_VOLTAGE_0, ref pos);
                }

                return pos;
            }
        }

        public double BleachPockelsVoltageMin0
        {
            get
            {
                double pos = 0;
                //if using thordaq, use the first available pockels line for bleaching
                if (WaveformDriverType == WaveformDriverType.WaveformDriver_ThorDAQ)
                {
                    for (int i = 0; i < MAX_POCKELS_COUNT; ++i)
                    {
                        switch (i)
                        {
                            case 0:
                                {

                                    if (BleacherPockelsEnable0)
                                    {
                                        ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_POCKELS_MIN_VOLTAGE_0, ref pos);
                                        return pos;
                                    }
                                    break;
                                }
                            case 1:
                                {
                                    if (BleacherPockelsEnable1)
                                    {
                                        ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_POCKELS_MIN_VOLTAGE_1, ref pos);
                                        return pos;
                                    }
                                    break;
                                }
                            case 2:
                                {
                                    if (BleacherPockelsEnable2)
                                    {
                                        ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_POCKELS_MIN_VOLTAGE_2, ref pos);
                                        return pos;
                                    }
                                    break;
                                }
                            case 3:
                                {
                                    if (BleacherPockelsEnable3)
                                    {
                                        ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_POCKELS_MIN_VOLTAGE_3, ref pos);
                                        return pos;
                                    }
                                    break;
                                }
                        }
                    }
                }
                else
                {
                    ThorSharedTypes.ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_POCKELS_MIN_VOLTAGE_0, ref pos);
                }

                return pos;
            }
        }

        public int EnablePockelsMask
        {
            get
            {
                int pos = 0;

                ThorSharedTypes.ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_MASK_ENABLE_0, ref pos);

                return pos;
            }
            set
            {
                ThorSharedTypes.ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_MASK_ENABLE_0, value);
            }
        }

        public double PockelsBlankingPhaseShiftPercent
        {
            get
            {
                double pos = 0;

                ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_BLANKING_PHASESHIFT_PERCENT, ref pos);

                return pos;
            }
            set
            {
                ResourceManagerCS.SetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_BLANKING_PHASESHIFT_PERCENT, value);
            }
        }

        public bool PockelsBlankingPhaseShiftPercentAvailable
        {
            get
            {
                return 1 == ResourceManagerCS.GetCameraParamAvailable((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_BLANKING_PHASESHIFT_PERCENT);
            }
        }

        public int PockelsBlankPercentage0
        {
            get
            {
                int pos = 0;

                ThorSharedTypes.ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_0, ref pos);

                return pos;
            }
            set
            {
                ThorSharedTypes.ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_0, value);
            }
        }

        public int PockelsBlankPercentage1
        {
            get
            {
                int pos = 0;

                ThorSharedTypes.ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_1, ref pos);

                return pos;
            }
            set
            {
                ThorSharedTypes.ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_1, value);
            }
        }

        public int PockelsBlankPercentage2
        {
            get
            {
                int pos = 0;

                ThorSharedTypes.ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_2, ref pos);

                return pos;
            }
            set
            {
                ThorSharedTypes.ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_2, value);
            }
        }

        public int PockelsBlankPercentage3
        {
            get
            {
                int pos = 0;

                ThorSharedTypes.ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_3, ref pos);

                return pos;
            }
            set
            {
                ThorSharedTypes.ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_3, value);
            }
        }

        public string PockelsMaskFile
        {
            get
            {
                return _filePockelsMask;
            }
            set
            {
                _filePockelsMask = value;
                SetPockelsMaskFile(_filePockelsMask);
            }
        }

        public int PockelsMaskInvert0
        {
            get
            {
                int pos = 0;

                ThorSharedTypes.ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_MASK_INVERT_0, ref pos);

                return pos;
            }
            set
            {
                ThorSharedTypes.ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_MASK_INVERT_0, value);
            }
        }

        public bool PockelsMaskOptionsAvailable
        {
            get
            {
                int val = 0;
                if (0 == ThorSharedTypes.ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_MASK_ENABLE_0, ref val))
                {
                    return false;
                }
                else
                {
                    return true;
                }
            }
        }

        public double PockelsPowerMax
        {
            get
            {
                double pMax = 0;
                double pMinPos = 0;
                double pMaxPos = 0;
                double pDefaultPos = 0;

                if (1 == ThorSharedTypes.ResourceManagerCS.GetCameraParamRangeDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_0, ref pMinPos, ref pMaxPos, ref pDefaultPos))
                {
                    pMax = pMaxPos;
                }
                return pMax;
            }
        }

        public double PockelsPowerMin
        {
            get
            {
                double pMin = 0;
                double pMinPos = 0;
                double pMaxPos = 0;
                double pDefaultPos = 0;

                if (1 == ThorSharedTypes.ResourceManagerCS.GetCameraParamRangeDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_0, ref pMinPos, ref pMaxPos, ref pDefaultPos))
                {
                    pMin = pMinPos;
                }
                return pMin;
            }
        }

        public PockelsResponseType PockelsPowerResponse0
        {
            get
            {
                int pos = 0;
                ThorSharedTypes.ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_RESPONSE_TYPE_0, ref pos);
                return (PockelsResponseType)pos;
            }
        }

        public PockelsResponseType PockelsPowerResponse1
        {
            get
            {
                int pos = 0;
                ThorSharedTypes.ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_RESPONSE_TYPE_1, ref pos);
                return (PockelsResponseType)pos;
            }
        }

        public PockelsResponseType PockelsPowerResponse2
        {
            get
            {
                int pos = 0;
                ThorSharedTypes.ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_RESPONSE_TYPE_2, ref pos);
                return (PockelsResponseType)pos;
            }
        }

        public PockelsResponseType PockelsPowerResponse3
        {
            get
            {
                int pos = 0;
                ThorSharedTypes.ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_RESPONSE_TYPE_3, ref pos);
                return (PockelsResponseType)pos;
            }
        }

        public double PockelsVoltageMax0
        {
            get
            {
                double pos = 0;

                ThorSharedTypes.ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_MAX_VOLTAGE_0, ref pos);

                return pos;
            }
            set
            {
                ThorSharedTypes.ResourceManagerCS.SetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_MAX_VOLTAGE_0, (double)value);
            }
        }

        public double PockelsVoltageMax1
        {
            get
            {
                double pos = 0;

                ThorSharedTypes.ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_MAX_VOLTAGE_1, ref pos);

                return pos;
            }
            set
            {
                ThorSharedTypes.ResourceManagerCS.SetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_MAX_VOLTAGE_1, (double)value);
            }
        }

        public double PockelsVoltageMax2
        {
            get
            {
                double pos = 0;

                ThorSharedTypes.ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_MAX_VOLTAGE_2, ref pos);

                return pos;
            }
            set
            {
                ThorSharedTypes.ResourceManagerCS.SetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_MAX_VOLTAGE_2, (double)value);
            }
        }

        public double PockelsVoltageMax3
        {
            get
            {
                double pos = 0;

                ThorSharedTypes.ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_MAX_VOLTAGE_3, ref pos);

                return pos;
            }
            set
            {
                ThorSharedTypes.ResourceManagerCS.SetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_MAX_VOLTAGE_3, (double)value);
            }
        }

        public double PockelsVoltageMin0
        {
            get
            {
                double pos = 0;

                ThorSharedTypes.ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_MIN_VOLTAGE_0, ref pos);

                return pos;
            }
            set
            {
                ThorSharedTypes.ResourceManagerCS.SetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_MIN_VOLTAGE_0, (double)value);
            }
        }

        public double PockelsVoltageMin1
        {
            get
            {
                double pos = 0;

                ThorSharedTypes.ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_MIN_VOLTAGE_1, ref pos);

                return pos;
            }
            set
            {
                ThorSharedTypes.ResourceManagerCS.SetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_MIN_VOLTAGE_1, (double)value);
            }
        }

        public double PockelsVoltageMin2
        {
            get
            {
                double pos = 0;

                ThorSharedTypes.ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_MIN_VOLTAGE_2, ref pos);

                return pos;
            }
            set
            {
                ThorSharedTypes.ResourceManagerCS.SetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_MIN_VOLTAGE_2, (double)value);
            }
        }

        public double PockelsVoltageMin3
        {
            get
            {
                double pos = 0;

                ThorSharedTypes.ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_MIN_VOLTAGE_3, ref pos);

                return pos;
            }
            set
            {
                ThorSharedTypes.ResourceManagerCS.SetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_MIN_VOLTAGE_3, (double)value);
            }
        }

        public double Power0
        {
            get
            {
                double pos = 0;

                ThorSharedTypes.ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_0, ref pos);

                Decimal dec = new Decimal(pos);

                pos = Decimal.ToDouble(Decimal.Round(dec, 2));

                return pos;
            }
            set
            {
                ThorSharedTypes.ResourceManagerCS.SetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_0, (double)value);
            }
        }

        public double Power1
        {
            get
            {
                double pos = 0;

                ThorSharedTypes.ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_1, ref pos);

                Decimal dec = new Decimal(pos);

                pos = Decimal.ToDouble(Decimal.Round(dec, 2));

                return pos;
            }
            set
            {
                ThorSharedTypes.ResourceManagerCS.SetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_1, (double)value);
            }
        }

        public double Power2
        {
            get
            {
                double pos = 0;

                ThorSharedTypes.ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_2, ref pos);

                Decimal dec = new Decimal(pos);

                pos = Decimal.ToDouble(Decimal.Round(dec, 2));

                return pos;
            }
            set
            {
                ThorSharedTypes.ResourceManagerCS.SetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_2, (double)value);
            }
        }

        public double Power2Max
        {
            get
            {
                double pMax = 0;
                double pMinPos = 0;
                double pMaxPos = 0;
                double pDefaultPos = 0;

                if (1 == ThorSharedTypes.ResourceManagerCS.GetDeviceParamRangeDouble((int)SelectedHardware.SELECTED_POWERREGULATOR2, (int)IDevice.Params.PARAM_POWER2_POS, ref pMinPos, ref pMaxPos, ref pDefaultPos))
                {
                    pMax = pMaxPos;

                }

                return pMax;
            }
        }

        public double Power2Min
        {
            get
            {
                double pMin = 0;
                double pMinPos = 0;
                double pMaxPos = 0;
                double pDefaultPos = 0;

                if (1 == ThorSharedTypes.ResourceManagerCS.GetDeviceParamRangeDouble((int)SelectedHardware.SELECTED_POWERREGULATOR2, (int)IDevice.Params.PARAM_POWER2_POS, ref pMinPos, ref pMaxPos, ref pDefaultPos))
                {
                    pMin = pMinPos;

                }

                return pMin;
            }
        }

        public double Power3
        {
            get
            {
                double pos = 0;

                ThorSharedTypes.ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_3, ref pos);

                Decimal dec = new Decimal(pos);

                pos = Decimal.ToDouble(Decimal.Round(dec, 2));

                return pos;
            }
            set
            {
                ThorSharedTypes.ResourceManagerCS.SetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_3, (double)value);
            }
        }

        public double PowerMax
        {
            get
            {
                double pMax = 0;
                double pMinPos = 0;
                double pMaxPos = 0;
                double pDefaultPos = 0;

                if (1 == ThorSharedTypes.ResourceManagerCS.GetDeviceParamRangeDouble((int)SelectedHardware.SELECTED_POWERREGULATOR, (int)IDevice.Params.PARAM_POWER_POS, ref pMinPos, ref pMaxPos, ref pDefaultPos))
                {
                    pMax = pMaxPos;

                }

                return pMax;
            }
        }

        public double PowerMin
        {
            get
            {
                double pMin = 0;
                double pMinPos = 0;
                double pMaxPos = 0;
                double pDefaultPos = 0;

                if (1 == ThorSharedTypes.ResourceManagerCS.GetDeviceParamRangeDouble((int)SelectedHardware.SELECTED_POWERREGULATOR, (int)IDevice.Params.PARAM_POWER_POS, ref pMinPos, ref pMaxPos, ref pDefaultPos))
                {
                    pMin = pMinPos;

                }

                return pMin;
            }
        }

        public double PowerPosition
        {
            get
            {
                return (double)Decimal.Round((decimal)_powerPosition, 4);
            }
            set
            {
                ThorSharedTypes.ResourceManagerCS.SetDeviceParamDouble((int)SelectedHardware.SELECTED_POWERREGULATOR, (int)IDevice.Params.PARAM_POWER_POS, value, (int)IDevice.DeviceSetParamType.EXECUTION_WAIT);
            }
        }

        public double PowerPosition2
        {
            get
            {
                return (double)Decimal.Round((decimal)_powerPosition2, 4);
            }
            set
            {
                ThorSharedTypes.ResourceManagerCS.SetDeviceParamDouble((int)SelectedHardware.SELECTED_POWERREGULATOR2, (int)IDevice.Params.PARAM_POWER2_POS, value, (int)IDevice.DeviceSetParamType.EXECUTION_WAIT);
            }
        }

        public double PowerPosition2Current
        {
            get
            {
                GetPowerPosition2(ref _powerPosition2);
                return _powerPosition2;
            }
        }

        public double PowerPositionCurrent
        {
            get
            {
                GetPowerPosition(ref _powerPosition);
                return _powerPosition;
            }
        }

        public string PowerReg2EncoderPosition
        {
            get
            {
                double val = 0;
                ThorSharedTypes.ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_POWERREGULATOR2, (int)IDevice.Params.PARAM_POWER2_ENCODER_POS, ref val);
                return val.ToString();
            }
        }

        public double PowerReg2Zero
        {
            get
            {
                double val = 0;
                ThorSharedTypes.ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_POWERREGULATOR2, (int)IDevice.Params.PARAM_POWER2_ZERO_POS, ref val);
                return val;
            }
            set
            {
                ThorSharedTypes.ResourceManagerCS.SetDeviceParamDouble((int)SelectedHardware.SELECTED_POWERREGULATOR2, (int)IDevice.Params.PARAM_POWER2_ZERO_POS, value, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT);

                ThorSharedTypes.ResourceManagerCS.SetDeviceParamDouble((int)SelectedHardware.SELECTED_POWERREGULATOR2, (int)IDevice.Params.PARAM_POWER2_POS, 0, (int)IDevice.DeviceSetParamType.EXECUTION_WAIT);
            }
        }

        public string PowerRegEncoderPosition
        {
            get
            {
                double val = 0;
                ThorSharedTypes.ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_POWERREGULATOR, (int)IDevice.Params.PARAM_POWER_ENCODER_POS, ref val);
                return val.ToString();
            }
        }

        public double PowerRegZero
        {
            get
            {
                double val = 0;
                ThorSharedTypes.ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_POWERREGULATOR, (int)IDevice.Params.PARAM_POWER_ZERO_POS, ref val);
                return val;
            }
            set
            {
                ThorSharedTypes.ResourceManagerCS.SetDeviceParamDouble((int)SelectedHardware.SELECTED_POWERREGULATOR, (int)IDevice.Params.PARAM_POWER_ZERO_POS, value, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT);

                ThorSharedTypes.ResourceManagerCS.SetDeviceParamDouble((int)SelectedHardware.SELECTED_POWERREGULATOR, (int)IDevice.Params.PARAM_POWER_POS, 0, (int)IDevice.DeviceSetParamType.EXECUTION_WAIT);
            }
        }

        public int ShutterPosition
        {
            get
            {
                return _shutterPosition;
            }
            set
            {
                if (SetShutterPosition(value))
                {
                    _shutterPosition = value;
                }
            }
        }

        public WaveformDriverType WaveformDriverType
        {
            get
            {
                int temp = (int)WaveformDriverType.WaveformDriver_NI;

                ResourceManagerCS.GetCameraParamInt((int)SelectedHardware.SELECTED_BLEACHINGSCANNER, (int)ICamera.Params.PARAM_LSM_WAVEFORM_DRIVER_TYPE, ref temp);

                return (WaveformDriverType)temp;
            }
        }

        #endregion Properties

        #region Methods

        public int FindPockelsMinMax(int index)
        {
            int ret = 0;
            switch (index)
            {
                case 0:
                    ret = (int)ThorSharedTypes.ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_FIND_MIN_MAX_0, 1);
                    break;
                case 1:
                    ret = (int)ThorSharedTypes.ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_FIND_MIN_MAX_1, 1);
                    break;
                case 2:
                    ret = (int)ThorSharedTypes.ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_FIND_MIN_MAX_2, 1);
                    break;
                case 3:
                    ret = (int)ThorSharedTypes.ResourceManagerCS.SetCameraParamInt((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_FIND_MIN_MAX_3, 1);
                    break;
            }
            return ret;
        }

        public double[] GetPockelsPlotX(int index)
        {
            const int POCKELS_VOLTAGE_STEPS = 100;
            double pockelsVoltageStart = GetPockelsScanStartVoltage(index);
            double pockelsVoltageStop = GetPockelsScanStopVoltage(index) - pockelsVoltageStart;

            double[] data = new double[POCKELS_VOLTAGE_STEPS];

            double stepSize = pockelsVoltageStop / POCKELS_VOLTAGE_STEPS;

            for (int i = 0; i < POCKELS_VOLTAGE_STEPS; i++)
            {
                data[i] = pockelsVoltageStart + i * stepSize;
            }

            return data;
        }

        public double[] GetPockelsPlotY(int index)
        {
            const int POCKELS_VOLTAGE_STEPS = 100;
            double[] data = new double[POCKELS_VOLTAGE_STEPS];

            IntPtr ptr = IntPtr.Zero;

            try
            {
                ptr = Marshal.AllocHGlobal(POCKELS_VOLTAGE_STEPS * 8);

                GetPockelsPlot(ref ptr, index);

                Marshal.Copy(ptr, data, 0, POCKELS_VOLTAGE_STEPS);
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Information, 1, ex.Message);
            }

            Marshal.FreeHGlobal(ptr);

            return data;
        }

        public PropertyInfo GetPropertyInfo(string propertyName)
        {
            PropertyInfo myPropInfo = null;
            if (!_properties.TryGetValue(propertyName, out myPropInfo))
            {
                myPropInfo = typeof(PowerControlModel).GetProperty(propertyName);
                if (null != myPropInfo)
                {
                    _properties.Add(propertyName, myPropInfo);
                }
            }
            return myPropInfo;
        }

        public void PowerReg2CalibrateZero()
        {
            PowerCalibrate2Zero();
        }

        public void PowerRegCalibrateZero()
        {
            PowerCalibrateZero();
        }

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "GetPockelsPlot")]
        private static extern bool GetPockelsPlot(ref IntPtr data, int index);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "GetPowerPosition")]
        private static extern bool GetPowerPosition(ref double pos);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "GetPowerPosition2")]
        private static extern bool GetPowerPosition2(ref double pos);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "GetPowerRange")]
        private static extern bool GetPowerRange(ref double pMin, ref double pMax, ref double pDefault);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "PowerCalibrate2Zero")]
        private static extern bool PowerCalibrate2Zero();

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "PowerCalibrateZero")]
        private static extern bool PowerCalibrateZero();

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "SetPockelsMaskFile")]
        private static extern bool SetPockelsMaskFile([MarshalAs(UnmanagedType.LPStr)] string path);

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "SetShutterPosition")]
        private static extern bool SetShutterPosition(int pos);

        private double GetPockelsScanStartVoltage(int index)
        {
            double pos = 0;
            switch (index)
            {
                case 0: ThorSharedTypes.ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_START_SCAN_VOLTAGE_0, ref pos); break;
                case 1: ThorSharedTypes.ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_START_SCAN_VOLTAGE_1, ref pos); break;
                case 2: ThorSharedTypes.ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_START_SCAN_VOLTAGE_2, ref pos); break;
                case 3: ThorSharedTypes.ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_START_SCAN_VOLTAGE_3, ref pos); break;
            }
            return pos;
        }

        private double GetPockelsScanStopVoltage(int index)
        {
            double pos = 0;
            switch (index)
            {
                case 0: ThorSharedTypes.ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_STOP_SCAN_VOLTAGE_0, ref pos); break;
                case 1: ThorSharedTypes.ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_STOP_SCAN_VOLTAGE_1, ref pos); break;
                case 2: ThorSharedTypes.ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_STOP_SCAN_VOLTAGE_2, ref pos); break;
                case 3: ThorSharedTypes.ResourceManagerCS.GetCameraParamDouble((int)SelectedHardware.SELECTED_CAMERA1, (int)ICamera.Params.PARAM_LSM_POCKELS_STOP_SCAN_VOLTAGE_3, ref pos); break;
            }
            return pos;
        }

        #endregion Methods
    }
}