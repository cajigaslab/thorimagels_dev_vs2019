public class XilinxI2CXfer
{
    public XilinxI2CXfer() { }

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
        status = (THORDAQ_STATUS)ThorDAQAPIXI2CReadWrite(MasterBoardIndex, bI2C_SlaveRead,
            TDMasterI2CMUXAddr, MasterMUXChan,
            TDSlaveI2CMUXAddr, SlaveMUXChan, TargetSlaveAddr, PageSize,
            UnManagedOpCodeBuffer, ref OpCodeLen, // "OpCodeLen" IN: Opcode byte count (in buffer)
            UnManagedDataBuffer, ref DataLen); // "DataLen" IN: dataLen,  OUT: ALL Opcdoes + Data bytes transfered

        I2CtransferLen = DataLen;

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