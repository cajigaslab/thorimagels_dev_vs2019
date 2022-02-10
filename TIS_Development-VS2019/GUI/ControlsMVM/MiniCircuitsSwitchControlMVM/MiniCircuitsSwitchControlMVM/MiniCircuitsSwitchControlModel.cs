namespace MiniCircuitsSwitchControl.Model
{
    using System.Diagnostics;

    using ThorLogging;

    using ThorSharedTypes;

    public class MiniCircuitsSwitchControlModel
    {
        #region Fields

        private const int FIRST_BIT_ENABLED = 1;
        private const int FIRST_SWITCH_BOX_INDEX = 1;
        private const int FOURTH_BIT_ENABLED = 8;
        private const int SECOND_BIT_ENABLED = 2;
        private const int SECOND_SWITCH_BOX_INDEX = 2;
        private const int THIRD_BIT_ENABLED = 4;

        private bool _manualSwitchEnabled = false;

        #endregion Fields

        #region Constructors

        /// <summary>
        /// Create a new instance of the MiniCircuitsSwitchControlModel class
        /// </summary>
        public MiniCircuitsSwitchControlModel()
        {
        }

        #endregion Constructors

        #region Properties

        public int A1SwitchPosition
        {
            get
            {
                int val;
                val = ResourceManagerCS.Instance.ReadSwitchBoxPositions(FIRST_SWITCH_BOX_INDEX);
                if (FIRST_BIT_ENABLED == (FIRST_BIT_ENABLED & val))
                {
                    return 1;
                }
                return 0;
            }
            set
            {
                if (value != A1SwitchPosition)
                {
                    if (false == ResourceManagerCS.Instance.ToggleSwitch(FIRST_SWITCH_BOX_INDEX, "A", value))
                    {
                        ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "Mini Circuits Switch: Error could not toggle Switch A1 to " + value.ToString());
                    }
                }
            }
        }

        public int A2SwitchPosition
        {
            get
            {
                int val;
                val = ResourceManagerCS.Instance.ReadSwitchBoxPositions(SECOND_SWITCH_BOX_INDEX);
                if (FIRST_BIT_ENABLED == (FIRST_BIT_ENABLED & val))
                {
                    return 1;
                }
                return 0;
            }
            set
            {
                if (value != A2SwitchPosition)
                {
                    if (false == ResourceManagerCS.Instance.ToggleSwitch(SECOND_SWITCH_BOX_INDEX, "A", value))
                    {
                        ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "Mini Circuits Switch: Error could not toggle Switch A2 to " + value.ToString());
                    }
                }
            }
        }

        public int B1SwitchPosition
        {
            get
            {
                int val;
                val = ResourceManagerCS.Instance.ReadSwitchBoxPositions(FIRST_SWITCH_BOX_INDEX);
                if (SECOND_BIT_ENABLED == (SECOND_BIT_ENABLED & val))
                {
                    return 1;
                }
                return 0;
            }
            set
            {
                if (value != B1SwitchPosition)
                {
                    if (false == ResourceManagerCS.Instance.ToggleSwitch(FIRST_SWITCH_BOX_INDEX, "B", value))
                    {
                        ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "Mini Circuits Switch: Error could not toggle Switch B1 to " + value.ToString());
                    }
                }
            }
        }

        public int B2SwitchPosition
        {
            get
            {
                int val;
                val = ResourceManagerCS.Instance.ReadSwitchBoxPositions(SECOND_SWITCH_BOX_INDEX);
                if (SECOND_BIT_ENABLED == (SECOND_BIT_ENABLED & val))
                {
                    return 1;
                }
                return 0;
            }
            set
            {
                if (value != B2SwitchPosition)
                {
                    if (false == ResourceManagerCS.Instance.ToggleSwitch(SECOND_SWITCH_BOX_INDEX, "B", value))
                    {
                        ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "Mini Circuits Switch: Error could not toggle Switch B2 to " + value.ToString());
                    }
                }
            }
        }

        public int C1SwitchPosition
        {
            get
            {
                int val;
                val = ResourceManagerCS.Instance.ReadSwitchBoxPositions(FIRST_SWITCH_BOX_INDEX);
                if (THIRD_BIT_ENABLED == (THIRD_BIT_ENABLED & val))
                {
                    return 1;
                }
                return 0;
            }
            set
            {
                if (value != C1SwitchPosition)
                {
                    if (false == ResourceManagerCS.Instance.ToggleSwitch(FIRST_SWITCH_BOX_INDEX, "C", value))
                    {
                        ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "Mini Circuits Switch: Error could not toggle Switch C1 to " + value.ToString());
                    }
                }
            }
        }

        public int C2SwitchPosition
        {
            get
            {
                int val;
                val = ResourceManagerCS.Instance.ReadSwitchBoxPositions(SECOND_SWITCH_BOX_INDEX);
                if (THIRD_BIT_ENABLED == (THIRD_BIT_ENABLED & val))
                {
                    return 1;
                }
                return 0;
            }
            set
            {
                if (value != C2SwitchPosition)
                {
                    if (false == ResourceManagerCS.Instance.ToggleSwitch(SECOND_SWITCH_BOX_INDEX, "C", value))
                    {
                        ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "Mini Circuits Switch: Error could not toggle Switch C2 to " + value.ToString());
                    }
                }
            }
        }

        public int D1SwitchPosition
        {
            get
            {
                int val;
                val = ResourceManagerCS.Instance.ReadSwitchBoxPositions(FIRST_SWITCH_BOX_INDEX);
                if (FOURTH_BIT_ENABLED == (FOURTH_BIT_ENABLED & val))
                {
                    return 1;
                }
                return 0;
            }
            set
            {
                if (value != D1SwitchPosition)
                {
                    if (false == ResourceManagerCS.Instance.ToggleSwitch(FIRST_SWITCH_BOX_INDEX, "D", value))
                    {
                        ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "Mini Circuits Switch: Error could not toggle Switch D1 to " + value.ToString());
                    }
                }
            }
        }

        public int D2SwitchPosition
        {
            get
            {
                int val;
                val = ResourceManagerCS.Instance.ReadSwitchBoxPositions(SECOND_SWITCH_BOX_INDEX);
                if (FOURTH_BIT_ENABLED == (FOURTH_BIT_ENABLED & val))
                {
                    return 1;
                }
                return 0;
            }
            set
            {
                if (value != D2SwitchPosition)
                {
                    if (false == ResourceManagerCS.Instance.ToggleSwitch(SECOND_SWITCH_BOX_INDEX, "D", value))
                    {
                        ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "Mini Circuits Switch: Error could not toggle Switch D2 to " + value.ToString());
                    }
                }
            }
        }

        public bool FirstSwitchBoxAvailable
        {
            get
            {
                return ResourceManagerCS.Instance.IsSwitchBoxConnected(FIRST_SWITCH_BOX_INDEX);
            }
        }

        public bool ManualSwitchEnable
        {
            get
            {
                return _manualSwitchEnabled;
            }
            set
            {
                _manualSwitchEnabled = value;
            }
        }

        public bool SecondSwitchBoxAvailable
        {
            get
            {
                return ResourceManagerCS.Instance.IsSwitchBoxConnected(SECOND_SWITCH_BOX_INDEX);
            }
        }

        #endregion Properties
    }
}