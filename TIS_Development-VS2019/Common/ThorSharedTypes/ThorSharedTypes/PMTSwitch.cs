namespace ThorSharedTypes
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;

    //using System.Threading;
    using mcl_RF_Switch_Controller_NET45;

    class PMTSwitch
    {
        #region Fields

        public USB_RF_SwitchBox _sb = new USB_RF_SwitchBox();

        private string _serialNumber = string.Empty;

        #endregion Fields

        #region Methods

        public bool CheckSwitchBoxConnection()
        {
            return (1 == _sb.Check_Connection());
        }

        public bool Connect(string SerialNumber)
        {
            //Only connect if it is not currently connected to a switch box
            if (0 == _sb.Check_Connection())
            {
                _sb.Connect(ref SerialNumber);
                if (1 == _sb.Check_Connection())
                {
                    _serialNumber = SerialNumber;
                    return true;
                }
            }
            return false;
        }

        public void Disconnect()
        {
            //Only send the command if it is connected to a switch box
            if (1 == _sb.Check_Connection())
            {
                _sb.Disconnect();
                _serialNumber = string.Empty;
                System.Threading.Thread.Sleep(10);
            }
        }

        public string[] GetSerialNumbers()
        {
            string serialNums = string.Empty;
            _sb.Get_Available_SN_List(ref serialNums);
            return serialNums.Split(' ');
        }

        public int GetSwitchesStatus()
        {
            int statusRet = 0;
            //Only send the command if it is connected to a switch box
            if (1 == _sb.Check_Connection())
            {
                _sb.GetSwitchesStatus(ref statusRet);
            }
            return statusRet;
        }

        public bool Set_Switch(string SwitchName, int Val)
        {
            //Only send the command if it is connected to a switch box
            if (1 == _sb.Check_Connection())
            {
                return (1 == _sb.Set_Switch(ref SwitchName, ref Val));
            }
            return false;
        }

        public bool Set_SwitchesPort(byte binVal)
        {
            //Only send the command if it is connected to a switch box
            if (1 == _sb.Check_Connection())
            {
                return (1 == _sb.Set_SwitchesPort(ref binVal));
            }
            return false;
        }

        #endregion Methods
    }
}