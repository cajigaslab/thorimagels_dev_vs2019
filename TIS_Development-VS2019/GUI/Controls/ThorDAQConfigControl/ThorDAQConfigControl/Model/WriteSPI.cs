using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Text.RegularExpressions;
using System.Globalization;
using System.Threading;
using System.Drawing;
using System.IO;

namespace ThorDAQConfigControl.Model
{
    public partial class ThorDAQCommandProvider
    {
        public bool SelectSPISlaveContronller(string index) 
        {
            bool ret = true;
            IntPtr buffer = IntPtr.Zero;
            string command = string.Empty;
            int controllerIndex = Int32.Parse(index);
            Win32.AllocateBuffer(ref buffer, 4);
            switch (controllerIndex)
            {
                case 0:
                    command = "FFFFFFF7";  //clock synthesizer
                    break;
                case 1:
                    command = "FFFFFFFE";  //1 & 2 ADC Converter
                    break;
                case 2:
                    command = "FFFFFFFD";  //2 & 3 ADC Converter
                    break;
                case 3:
                    command = "FFFFFFFB";  //4 & 5 ADC Converter
                    break;
                case 4:
                    command = "FFFFFFEF";  //1 & 2 Digital Attenuator
                    break;
                case 5:
                    command = "FFFFFFDF";  //3 & 4 Digital Attenuator
                    break;
                case 6:
                    command = "FFFFFFBF";  //5 & 6 Digital Attenuator
                    break;
                default:
                    command = "FFFFFFF7";  //clock synthesizer
                    break;
            }
            for (int i = 0; i < 4; i++)
            {
                byte byteValue = byte.Parse(command.Substring(i * 2, 2), NumberStyles.HexNumber, CultureInfo.InvariantCulture);
                System.Runtime.InteropServices.Marshal.WriteByte(buffer, 3 - i, byteValue);
            }
            ret = Win32.MemoryWrite(0, buffer, 3, (UInt64)0x0228, 0, 4);
            if (buffer != IntPtr.Zero)
            {
                Win32.FreeBuffer(buffer);
            }
            return ret;
        }

        public bool writeSPIAddress(string address)
        {
            bool ret = true;

            if (address.Contains("0x")) 
            {
                address = address.Replace("0x", string.Empty);
            }

            IntPtr buffer = IntPtr.Zero;
            Win32.AllocateBuffer(ref buffer, 2);
          
            for (int i = 0; i < 2; i++)
            {
                byte byteValue = byte.Parse(address.Substring(i * 2, 2), NumberStyles.HexNumber, CultureInfo.InvariantCulture);
                System.Runtime.InteropServices.Marshal.WriteByte(buffer, 1 - i, byteValue);
            }
            ret = Win32.MemoryWrite(0, buffer, 3, (UInt64)0x0222, 0, 2);
            if (buffer != IntPtr.Zero)
            {
                Win32.FreeBuffer(buffer);
            }
            return ret;
        }

        public bool writeSPIData(string data)
        {
            bool ret = true;
            IntPtr buffer = IntPtr.Zero;
            Win32.AllocateBuffer(ref buffer, 2);

            if (data.Contains("0x"))
            {
                data = data.Replace("0x", string.Empty);
            }

            for (int i = 0; i < 2; i++)
            {
                byte byteValue = byte.Parse(data.Substring(i * 2, 2), NumberStyles.HexNumber, CultureInfo.InvariantCulture);
                System.Runtime.InteropServices.Marshal.WriteByte(buffer, 1 - i, byteValue);
            }
            ret = Win32.MemoryWrite(0, buffer, 3, (UInt64)0x0220, 0, 2);
            if (buffer != IntPtr.Zero)
            {
                Win32.FreeBuffer(buffer);
            }
            return ret;
        }

        public bool writeADCData(bool resetSPI)
        {
            bool ret = true;
            IntPtr buffer = IntPtr.Zero;
            Win32.AllocateBuffer(ref buffer, 2);
            string data = resetSPI == true ? "8044" : "8040"; 
            for (int i = 0; i < 2; i++)
            {
                byte byteValue = byte.Parse(data.Substring(i * 2, 2), NumberStyles.HexNumber, CultureInfo.InvariantCulture);
                System.Runtime.InteropServices.Marshal.WriteByte(buffer, 1 - i, byteValue);
            }
            ret = Win32.MemoryWrite(0, buffer, 3, (UInt64)0x0200, 0, 2);
            if (buffer != IntPtr.Zero)
            {
                Win32.FreeBuffer(buffer);
            }
            return ret;
        }

        public bool WriteSPIBus(List<String> argumentsList)
        {
            if (!writeADCData(false))
            {
                return false;
            }
            if (!SelectSPISlaveContronller(argumentsList[1])) 
            {
                return false;
            }
            if (!writeSPIAddress(argumentsList[2]))
            {
                return false;
            }
            if (!writeSPIData(argumentsList[3]))
            {
                return false;
            }
            if (!writeADCData(true))
            {
                return false;
            }
            if (!writeADCData(false))
            {
                return false;
            }
            return true;
        }
    }
}