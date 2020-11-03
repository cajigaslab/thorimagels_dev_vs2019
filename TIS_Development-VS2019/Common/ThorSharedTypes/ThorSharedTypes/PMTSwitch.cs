namespace ThorSharedTypes
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;

    //using System.Threading;
    using mcl_RF_Switch_ControllerNET45;

    class PMTSwitch
    {
        #region Fields

        public USB_RF_SwitchBox _sb = new USB_RF_SwitchBox();

        #endregion Fields

        #region Methods

        public bool Connect(string SerialNumber)
        {
            //Only connect if it is not currently connected to a switch box
            if (0 == _sb.Check_Connection())
            {
                //Thread.Sleep(10);
                _sb.Connect(ref SerialNumber);
            }
            return (1 == _sb.Check_Connection());
        }

        public void Disconnect()
        {
            //Only send the command if it is connected to a switch box
            if (1 == _sb.Check_Connection())
            {
                _sb.Disconnect();
            }
        }

        public void GetSwitchesStatus(int statusRet)
        {
            //Only send the command if it is connected to a switch box
            if (1 == _sb.Check_Connection())
            {
                _sb.GetSwitchesStatus(ref statusRet);
            }
        }

        public void Set_Switch(string SwitchName, int Val)
        {
            //Only send the command if it is connected to a switch box
            if (1 == _sb.Check_Connection())
            {
                _sb.Set_Switch(ref SwitchName, ref Val);
            }
        }

        public void Set_SwitchesPort(byte binVal)
        {
            //Only send the command if it is connected to a switch box
            if (1 == _sb.Check_Connection())
            {
                _sb.Set_SwitchesPort(ref binVal);
            }
        }

        #endregion Methods
    }
}