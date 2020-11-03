namespace ObjectiveControl.Model
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
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

    public class ObjectiveControlModel
    {
        #region Fields

        private const int TRUE = 1, FALSE = 0;

        private int[] _changerPosition = new int[2] { 0, 0 };
        private bool _isObjectiveSwitching = false;
        private bool _magComboBoxEnabled = true;
        private ObservableCollection<string> _objectiveNames = new ObservableCollection<string>();
        private bool _objectveChangerIsDisconnected = true;

        //private string _turretBeamExpansion;
        private double _turretMag = 1.0;
        private double _turretNA = 1.0;
        private string _turretName = string.Empty;
        private int _turretPosition = 0;
        private int[] _zAxisToEscape = new int[2] { 0, 0 };
        private double[] _zEscapeDistance = new double[2] { 0.0, 0.0 };
        private bool _zStage2IsDisconnected = true;
        private bool _zStageIsDisconnected = true;

        #endregion Fields

        #region Constructors

        /// <summary>
        /// Create a new instance of the ObjectiveControlModel class
        /// </summary>
        public ObjectiveControlModel()
        {
        }

        #endregion Constructors

        #region Enumerations

        public enum ObjectiveChangerPositions
        {
            HOME = 0,
            POS1,
            POS2,
            HOMING,
            MOVING_POS1,
            MOVING_POS2,
            DISCONNECTED
        }

        #endregion Enumerations

        #region Properties

        public double BeamExp
        {
            get
            {
                double val = 0.0;

                ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_BEAMEXPANDER, (int)IDevice.Params.PARAM_EXP_RATIO, ref val);

                return val / 100; //Lower level uses value multiplied by 100
            }
        }

        public double BeamExp2
        {
            get
            {
                double val = 0.0;
                ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_BEAMEXPANDER, (int)IDevice.Params.PARAM_EXP_RATIO2, ref val);
                return val / 100; //Lower level uses value multiplied by 100
            }
        }

        public double CollisionStatus
        {
            get
            {
                double val = 0.0;
                ResourceManagerCS.GetDeviceParamDouble((int)SelectedHardware.SELECTED_TURRET, (int)IDevice.Params.PARAM_TURRET_COLLISION, ref val);
                return val;
            }
        }

        public int EpiTurretPos
        {
            get
            {
                int val = -1;
                ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_EPITURRET, (int)IDevice.Params.PARAM_FW_DIC_POS, ref val);
                return val;
            }
        }

        public bool IsObjectiveSwitching
        {
            get
            {
                return _isObjectiveSwitching;
            }
            set
            {
                _isObjectiveSwitching = value;
            }
        }

        public bool MagComboBoxEnabled
        {
            get
            {
                return _magComboBoxEnabled;
            }
            set
            {
                _magComboBoxEnabled = value;
            }
        }

        public bool MoveObjectiveReturnedFalse
        {
            get;
            set;
        }

        public double NumericalAperture
        {
            get
            {
                return _turretNA;
            }
            set
            {
                _turretNA = value;
            }

        }


        public int ObjectiveChangerStatus
        {
            get;
            set;
        }

        public ObservableCollection<string> ObjectiveNames
        {
            get
            {
                return _objectiveNames;
            }
            set
            {
                _objectiveNames = value;
            }
        }

        public bool ObjectveChangerIsDisconnected
        {
            get
            {
                return _objectveChangerIsDisconnected;
            }
            set
            {
                _objectveChangerIsDisconnected = value;
            }
        }

        //public string TurretBeamExpansion
        //{
        //    get
        //    {
        //        return _turretBeamExpansion;
        //    }
        //}
        public double TurretMagnification
        {
            get
            {
                return _turretMag;
            }
        }

        public string TurretName
        {
            get
            {
                return _turretName;
            }
        }

        public int TurretPosition
        {
            get
            {
                return _turretPosition;
            }
            set
            {
                if (0 > value || value == _turretPosition)
                {
                    return;
                }

                // Only try to move if an objective changer is selected
                if (false == ObjectveChangerIsDisconnected)
                {
                    if (false == MoveObjectiveChanger(value, _turretPosition))
                    {
                        ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, this.GetType().Name + " Objective Changer didn't move");
                    }
                    else
                    {
                        _turretPosition = value;
                    }
                }
                else
                {
                    _turretPosition = value;
                }

                GetBeamExpansion(_turretPosition);

                //Send the current selected magnification to the lower level of Capture Setup
                SetTurretPosition(_turretPosition);

                if (false == MoveBeamExpander(_turretPosition))
                {
                    ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " TurretPosition MoveBeamExpander failed");
                }
            }
        }

        public bool ZStage2IsDisconnected
        {
            get
            {
                return _zStage2IsDisconnected;
            }
            set
            {
                _zStage2IsDisconnected = value;
            }
        }

        public bool ZStageIsDisconnected
        {
            get
            {
                return _zStageIsDisconnected;
            }
            set
            {
                _zStageIsDisconnected = value;
            }
        }

        #endregion Properties

        #region Methods

        public bool GetBeamExpansion(int magnificationIndex)
        {
            string str = string.Empty;
            //int beamExp = 0;
            XmlNodeList ndList = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS].GetElementsByTagName("Objective");

            if (ndList.Count > 0)
            {
                if (magnificationIndex < ndList.Count)
                {
                    if (XmlManager.GetAttribute(ndList[magnificationIndex], MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS], "name", ref str))
                    {
                        _turretName = str;
                    }
                    if (XmlManager.GetAttribute(ndList[magnificationIndex], MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS], "mag", ref str))
                    {
                        _turretMag = Convert.ToDouble(str, CultureInfo.InvariantCulture);
                    }
                    if (XmlManager.GetAttribute(ndList[magnificationIndex], MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS], "na", ref str))
                    {
                        _turretNA = Convert.ToDouble(str, CultureInfo.InvariantCulture);
                    }


                    // This code is dormant and does not do anything, we can remove it in next version if everything works fine without it.
                    //if (XmlManager.GetAttribute(ndList[magnificationIndex], MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS], "beamExp", ref str))
                    //{
                    //    beamExp = Convert.ToInt32(str);
                    //}
                    //switch (beamExp)
                    //{
                    //    case 0: str = "1.0X"; break;
                    //    case 1: str = "1.5X"; break;
                    //    case 2: str = "2.0X"; break;
                    //    case 3: str = "2.5X"; break;
                    //    case 4: str = "3.0X"; break;
                    //    case 5: str = "3.5X"; break;
                    //    case 6: str = "3.8X"; break;
                    //}
                    //_turretBeamExpansion = str;
                    return true;
                }
            }
            return false;
        }

        public bool MoveBeamExpander(int magnificationIndex)
        {
            const int BEAMEXPANDER_MODE_AUTO = 1;
            int beamExpPos = 0;
            int beamExpPos2 = 0;
            int beamExpWavelength = 0;
            int beamExpWavelength2 = 0;
            string str = string.Empty;
            XmlNodeList ndList = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS].GetElementsByTagName("Objective");

            if (ndList.Count > 0)
            {
                if (magnificationIndex < ndList.Count)
                {
                    //Read required params from HardwareSettings
                    if (XmlManager.GetAttribute(ndList[magnificationIndex], MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS], "beamExp", ref str))
                    {
                        beamExpPos = Convert.ToInt32(str);
                    }
                    else
                    {
                        return false;
                    }
                    if (XmlManager.GetAttribute(ndList[magnificationIndex], MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS], "beamWavelength", ref str))
                    {
                        beamExpWavelength = Convert.ToInt32(str);
                    }
                    else
                    {
                        return false;
                    }
                    if (XmlManager.GetAttribute(ndList[magnificationIndex], MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS], "beamExp2", ref str))
                    {
                        beamExpPos2 = Convert.ToInt32(str);
                    }
                    else
                    {
                        return false;
                    }
                    if (XmlManager.GetAttribute(ndList[magnificationIndex], MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS], "beamWavelength2", ref str))
                    {
                        beamExpWavelength2 = Convert.ToInt32(str);
                    }
                    else
                    {
                        return false;
                    }

                    if (FALSE == ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_BEAMEXPANDER, (int)IDevice.Params.PARAM_BMEXP_MODE, BEAMEXPANDER_MODE_AUTO, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT) &&
                    TRUE == ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_BEAMEXPANDER, (int)IDevice.Params.PARAM_EXP_RATIO2, beamExpPos2, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT) &&
                    TRUE == ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_BEAMEXPANDER, (int)IDevice.Params.PARAM_EXP_WAVELENGTH, beamExpWavelength, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT) &&
                    TRUE == ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_BEAMEXPANDER, (int)IDevice.Params.PARAM_EXP_WAVELENGTH2, beamExpWavelength2, (int)IDevice.DeviceSetParamType.EXECUTION_NO_WAIT) &&
                    TRUE == ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_BEAMEXPANDER, (int)IDevice.Params.PARAM_EXP_RATIO, beamExpPos, (int)IDevice.DeviceSetParamType.EXECUTION_WAIT))
                    {
                        return true;
                    }
                }
            }
            return false;
        }

        public bool MoveObjectiveChanger(int newMagnificationIndex, int initialMagnificationIndex)
        {
            string str = string.Empty;
            int turretHomed = 0;
            int collision = 0;
            int currentChangerPosition = 0;
            double zPosCurrent = 0.0;
            int[] selectedStage = new int[2] { -1, -1 };
            XmlNodeList ndList = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS].GetElementsByTagName("Objective");

            if (ndList.Count > 0)
            {
                if (initialMagnificationIndex < ndList.Count && newMagnificationIndex < ndList.Count)
                {
                    //Read required params from HardwareSettings
                    if (XmlManager.GetAttribute(ndList[initialMagnificationIndex], MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS], "turretPosition", ref str))
                    {
                        _changerPosition[0] = Convert.ToInt32(str);
                    }
                    else
                    {
                        return false;
                    }
                    if (XmlManager.GetAttribute(ndList[initialMagnificationIndex], MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS], "zAxisToEscape", ref str))
                    {
                        _zAxisToEscape[0] = Convert.ToInt32(str);
                    }
                    else
                    {
                        return false;
                    }
                    if (XmlManager.GetAttribute(ndList[initialMagnificationIndex], MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS], "zAxisEscapeDistance", ref str))
                    {
                        _zEscapeDistance[0] = Convert.ToDouble(str, CultureInfo.InvariantCulture);
                    }
                    else
                    {
                        return false;
                    }
                    if (XmlManager.GetAttribute(ndList[newMagnificationIndex], MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS], "turretPosition", ref str))
                    {
                        _changerPosition[1] = Convert.ToInt32(str);
                    }
                    else
                    {
                        return false;
                    }
                    if (XmlManager.GetAttribute(ndList[newMagnificationIndex], MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS], "zAxisToEscape", ref str))
                    {
                        _zAxisToEscape[1] = Convert.ToInt32(str);
                    }
                    else
                    {
                        return false;
                    }
                    if (XmlManager.GetAttribute(ndList[newMagnificationIndex], MVMManager.Instance.SettingsDoc[(int)SettingsFileType.HARDWARE_SETTINGS], "zAxisEscapeDistance", ref str))
                    {
                        _zEscapeDistance[1] = Convert.ToDouble(str, CultureInfo.InvariantCulture);
                    }
                    else
                    {
                        return false;
                    }

                    if (_zAxisToEscape[0] != _zAxisToEscape[1])
                    {
                        if (MessageBox.Show("Warning, Z Escape Axis for both objectives doesn't match. Do you want to proceed?\n\nNote: If this not intended verify the Objective Setup before continuing", "Objective Change Z Stage Mismatch", MessageBoxButton.YesNo, MessageBoxImage.Warning) == MessageBoxResult.Yes)
                        {
                            // Yes, do nothing and continue
                        }
                        else
                        {
                            return false; // No, return without moving
                        }
                    }

                    //Set the selected Z Stage for each objective
                    switch (_zAxisToEscape[0])
                    {
                        case 0:
                            {
                                selectedStage[0] = 0;
                            }
                            break;
                        case 1:
                            {
                                selectedStage[0] = (int)SelectedHardware.SELECTED_ZSTAGE;
                                if (ZStageIsDisconnected)
                                {
                                    MessageBox.Show("Error, selected Z Escape Axis (Z Stage) is disconnected.\n\nIf this is intended, please change the Escape Axis to 'None' in Objective Setup.", "Z Stage Disconnected", MessageBoxButton.OK, MessageBoxImage.Error);
                                    return false;
                                }
                            }
                            break;
                        case 2:
                            {
                                selectedStage[0] = (int)SelectedHardware.SELECTED_ZSTAGE2;
                                if (ZStage2IsDisconnected)
                                {
                                    MessageBox.Show("Error, selected Z Escape Axis (Secondary Z Stage) is disconnected.\n\nIf this is intended, please change the Escape Axis to 'None' in Objective Setup.", "Z Stage 2 Disconnected", MessageBoxButton.OK, MessageBoxImage.Error);
                                    return false;
                                }
                            }
                            break;
                    }
                    switch (_zAxisToEscape[1])
                    {
                        case 0:
                            {
                                selectedStage[1] = 0;
                            }
                            break;
                        case 1:
                            {
                                selectedStage[1] = (int)SelectedHardware.SELECTED_ZSTAGE;
                                if (ZStageIsDisconnected)
                                {
                                    MessageBox.Show("Error, selected Z Escape Axis (Z Stage) is disconnected.\n\nIf this is intended, please change the Escape Axis to 'None' in Objective Setup.", "Z Stage Disconnected", MessageBoxButton.OK, MessageBoxImage.Error);
                                    return false;
                                }
                            }
                            break;
                        case 2:
                            {
                                selectedStage[1] = (int)SelectedHardware.SELECTED_ZSTAGE2;
                                if (ZStage2IsDisconnected)
                                {
                                    MessageBox.Show("Error, selected Z Escape Axis (Secondary Z Stage) is disconnected.\n\nIf this is intended, please change the Escape Axis to 'None' in Objective Setup.", "Z Stage 2 Disconnected", MessageBoxButton.OK, MessageBoxImage.Error);
                                    return false;
                                }
                            }
                            break;
                    }

                    if (TRUE == ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_TURRET, (int)IDevice.Params.PARAM_TURRET_POS_CURRENT, ref currentChangerPosition) &&
                        TRUE == ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_TURRET, (int)IDevice.Params.PARAM_TURRET_HOMED, ref turretHomed))
                    {
                        BackgroundWorker worker = new BackgroundWorker();
                        worker.DoWork += (obj, evetnArg) =>
                        {
                            ObjectiveChangerStatus = (int)OutOfRangeColors.COLOR_YELLOW;
                            MagComboBoxEnabled = false;
                            IsObjectiveSwitching = true;

                            //Step 1: Move Z Axis to escape distance only if a ZStage is selected and check if it is within the escape distance
                            if (0 != selectedStage[0])
                            {
                                if (TRUE == ResourceManagerCS.GetDeviceParamDouble(selectedStage[0], (int)IDevice.Params.PARAM_Z_POS_CURRENT, ref zPosCurrent))
                                {
                                    if (TRUE == ResourceManagerCS.SetDeviceParamDouble(selectedStage[0], (int)IDevice.Params.PARAM_Z_POS, zPosCurrent + _zEscapeDistance[0], (int)IDevice.DeviceSetParamType.EXECUTION_WAIT))
                                    {
                                        double zPosCurrentNew = 0.0;
                                        //Check if the ZStage is within 100um from the escape distance
                                        if (TRUE == ResourceManagerCS.GetDeviceParamDouble(selectedStage[0], (int)IDevice.Params.PARAM_Z_POS_CURRENT, ref zPosCurrentNew))
                                        {
                                            if (0.100 < Math.Abs(zPosCurrentNew - (zPosCurrent + _zEscapeDistance[0])))
                                            {
                                                MessageBox.Show("Z Stage didn't reach the escape distance, Objective Changer won't be moved", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                                                ObjectiveChangerStatus = (int)OutOfRangeColors.COLOR_TRANSPARENT;
                                                MagComboBoxEnabled = true;
                                                IsObjectiveSwitching = false;
                                                return;
                                            }
                                        }
                                    }
                                }
                            }

                            //Step 2: Move the Objective changer and check if it reached the position, if it didn't encounter a collision and if it is still connected
                            // Move only if the position of the device is different from the position where it has to move, or if device is not homed
                            if (currentChangerPosition != _changerPosition[1] || FALSE == turretHomed)
                            {
                                ResourceManagerCS.SetDeviceParamInt((int)SelectedHardware.SELECTED_TURRET, (int)IDevice.Params.PARAM_TURRET_POS, _changerPosition[1], (int)IDevice.DeviceSetParamType.EXECUTION_WAIT);
                                do
                                {
                                    if (FALSE == ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_TURRET, (int)IDevice.Params.PARAM_TURRET_POS_CURRENT, ref currentChangerPosition))
                                    {
                                        break;
                                    }
                                }
                                while ((int)ObjectiveChangerPositions.MOVING_POS1 == currentChangerPosition || (int)ObjectiveChangerPositions.MOVING_POS2 == currentChangerPosition); //wait while the objective changer is moving
                                //Check if the objective changer was disconnected at some point, do not move the stage to avoid any collision
                                if ((int)ObjectiveChangerPositions.DISCONNECTED == currentChangerPosition)
                                {
                                    MessageBox.Show("Objective Changer is disconnected. Please make sure the device USB cable is connected.", "Error Objective Changer is disconnected", MessageBoxButton.OK, MessageBoxImage.Error);
                                    ObjectiveChangerStatus = (int)OutOfRangeColors.COLOR_TRANSPARENT;
                                    MagComboBoxEnabled = true;
                                    IsObjectiveSwitching = false;
                                    return;
                                }
                                //Check for collision when the objective changer is done moving. Only move the ZStage back if there was no collision
                                if (TRUE == ResourceManagerCS.GetDeviceParamInt((int)SelectedHardware.SELECTED_TURRET, (int)IDevice.Params.PARAM_TURRET_COLLISION, ref collision) && TRUE == collision)
                                {
                                    MessageBox.Show("Objective Changer detected a collision. Won't move Z Stage back to return position.", "Error Collision detected", MessageBoxButton.OK, MessageBoxImage.Error);
                                    ObjectiveChangerStatus = (int)OutOfRangeColors.COLOR_TRANSPARENT;
                                    MagComboBoxEnabled = true;
                                    IsObjectiveSwitching = false;
                                    return;
                                }
                                //Make sure the new position matches the position it was supposed to reach
                                if (_changerPosition[1] != currentChangerPosition)
                                {
                                    MessageBox.Show("Objective Changer didn't reach position: " + _changerPosition[1] + ". Won't move Z Stage back to return position. Check the USB connection or the path of the Objective Changer.", "Error Objective Changer not at position" + _changerPosition[1], MessageBoxButton.OK, MessageBoxImage.Error);
                                    ObjectiveChangerStatus = (int)OutOfRangeColors.COLOR_TRANSPARENT;
                                    MagComboBoxEnabled = true;
                                    IsObjectiveSwitching = false;
                                    return;
                                }

                                //Step 3: Move the selected Z Stage of the new objectve to the invert of it's escape distance.
                                if (0 != selectedStage[1]) // Try to move only if a stage is selected
                                {
                                    if (0 == _zEscapeDistance[0] && 0 < _zEscapeDistance[1]) // Check if the initial objective had an escape distance before moving back down.
                                    {
                                        if (MessageBox.Show("Warning, Z Stage didn't move an escape distance away and now it is trying to move back -" + _zEscapeDistance[1] + "[mm]. Do you want to proceed by moving the stage back?", "Initial escape distance is zero", MessageBoxButton.YesNo, MessageBoxImage.Warning) == MessageBoxResult.Yes)
                                        {
                                            // Yes, do nothing and continue
                                        }
                                        else
                                        {
                                            ObjectiveChangerStatus = (int)OutOfRangeColors.COLOR_TRANSPARENT;
                                            MagComboBoxEnabled = true;
                                            IsObjectiveSwitching = false;
                                            return; // No, return without moving back down.
                                        }
                                    }
                                    if (TRUE == ResourceManagerCS.GetDeviceParamDouble(selectedStage[1], (int)IDevice.Params.PARAM_Z_POS_CURRENT, ref zPosCurrent) &&
                                       ((int)SelectedHardware.SELECTED_ZSTAGE == selectedStage[1] || (int)SelectedHardware.SELECTED_ZSTAGE2 == selectedStage[1]))
                                    {
                                        double returnDistance = zPosCurrent + (-1.0 * _zEscapeDistance[1]);
                                        ResourceManagerCS.SetDeviceParamDouble(selectedStage[1], (int)IDevice.Params.PARAM_Z_POS, returnDistance, (int)IDevice.DeviceSetParamType.EXECUTION_WAIT);
                                    }
                                }
                            }
                            ObjectiveChangerStatus = (int)OutOfRangeColors.COLOR_TRANSPARENT;
                            MagComboBoxEnabled = true;
                            IsObjectiveSwitching = false;
                        };
                        worker.RunWorkerAsync();
                    }
                }
                else
                {
                    ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, this.GetType().Name + " Magnification index is out of bounds from the list.");
                }
            }
            else
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, this.GetType().Name + " Could not read Objective settings from Hardware Setup");
            }
            return true;
        }

        public void SetInitialTurretPosition(int position)
        {
            _turretPosition = position;
            GetBeamExpansion(_turretPosition);

            if (false == MoveBeamExpander(_turretPosition))
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " TurretPosition MoveBeamExpander failed");
            }
        }

        [DllImport(".\\Modules_Native\\CaptureSetup.dll", EntryPoint = "SetTurretPosition")]
        private static extern bool SetTurretPosition(int pos);

        #endregion Methods
    }
}