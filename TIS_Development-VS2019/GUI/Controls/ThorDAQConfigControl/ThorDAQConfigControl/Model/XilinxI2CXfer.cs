using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace ThorDAQConfigControl.Model
{
    public class XilinxI2CXfer
    {
        public UInt32 I2CbusHz;
        public XilinxI2CXfer() { I2CbusHz = 100; }

        public THORDAQ_STATUS I2CTransfer(uint MasterMUXChan, uint SlaveMUXChan, uint TargetSlaveAddr,
            bool bI2C_SlaveRead, int PageSize, byte[] OpcodeBytes, uint OpCodeLen, byte[] DataBytes, uint DataLen, out uint I2CtransferLen)
        {
            THORDAQ_STATUS status = 0;
            IntPtr UnManagedOpCodeBuffer = Marshal.AllocHGlobal((int)OpCodeLen);
            IntPtr UnManagedDataBuffer = Marshal.AllocHGlobal(PageSize);
            // Hardcoded I2C slave MUXes
            for (int ocIndx = 0; ocIndx < OpcodeBytes.Length; ocIndx++) // write OpCodes to UnManaged Mem
            {
                System.Runtime.InteropServices.Marshal.WriteByte(UnManagedOpCodeBuffer, ocIndx, OpcodeBytes[ocIndx]);
            }
            if (!bI2C_SlaveRead) // when writing, copy data bytes to DLL
            {
                for (int dIndx = 0; dIndx < DataLen; dIndx++) // write DATA bytes to UnManaged Mem
                {
                    // e.g., start of TD main board Data buffer to send, 0x54 0x44 0x30 ... ("TD0...")
                    System.Runtime.InteropServices.Marshal.WriteByte(UnManagedDataBuffer, (int)(dIndx), (byte)DataBytes[dIndx]);
                }
            }
            I2CtransferLen = DataLen;
            status = (THORDAQ_STATUS)ThorDAQCommandProvider.ThorDAQAPIXI2CReadWrite(ThorDAQCommandProvider.MasterBoardIndex, bI2C_SlaveRead,
                ThorDAQCommandProvider.TDMasterI2CMUXAddr, MasterMUXChan,
                ThorDAQCommandProvider.TDSlaveI2CMUXAddr, SlaveMUXChan, TargetSlaveAddr, I2CbusHz, PageSize,
                UnManagedOpCodeBuffer, ref OpCodeLen, // "OpCodeLen" IN: Opcode byte count (in buffer)
                UnManagedDataBuffer, ref I2CtransferLen); // IN: "DataLen" of expected READ bytes,  OUT: ALL Opcodes + Data bytes transfered


            // on READ, return the data to caller.  DataLen was the read count passed to I2C master
            if (bI2C_SlaveRead == true)
            {
                for (int CPPbyteIndx = 0; CPPbyteIndx < DataLen; CPPbyteIndx++)
                {
                    DataBytes[CPPbyteIndx] = Marshal.ReadByte(UnManagedDataBuffer, CPPbyteIndx);
                }

            }
            Marshal.FreeHGlobal(UnManagedOpCodeBuffer);
            Marshal.FreeHGlobal(UnManagedDataBuffer);
            return status;
        }
    }
}
