using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Globalization;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.RegularExpressions;
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
using ThorDAQConfigControl.Model;

using SplashScreen = ThorDAQConfigControl.Controls.SplashScreen;

namespace ThorDAQConfigControl.Model
{   
    public partial class ThorDAQCommandProvider
    {
        THORDAQ_STATUS LFTworkerThreadStatus = THORDAQ_STATUS.STATUS_INCOMPLETE;

        SplashScreen _LFTcpldSplash;
        int _LFTprogressPercentage = 0;
        public void LFT_UpdateCPLDProgressText(int percentage, string text)
        {
            //set our progress dialog text and value
            _LFTcpldSplash.ProgressText = string.Format("{0}%", percentage.ToString());
            _LFTcpldSplash.ProgressValue = percentage;
            _LFTcpldSplash.DisplayText = text;
        }

        SplashScreen _LFTmcuSplash;
        int _LTFmcuProgressPercentage = 0;
        /// <summary>
        /// this is the method that the UpdateProgressDelegate will execute
        /// </summary>
        /// <param name="percentage"></param>
        public void UpdateMCUProgressText(int percentage, string text)
        {
            //set our progress dialog text and value
            _LFTmcuSplash.ProgressText = string.Format("{0}%", percentage.ToString());
            _LFTmcuSplash.ProgressValue = percentage;
            _LFTmcuSplash.DisplayText = text;
        }


        #region APT Commands
        const int MAX_I2C_SLAVE_WRITE_Len = (6 + 16);
        const uint OpCodeLen = MAX_I2C_SLAVE_WRITE_Len;  // set to LONGEST possible APT cmd+data WRITE to Slave


        byte[][] MSGOpcodesRead =
        { 
            // READ opcodes (data expected from I2C slave)
            new byte[MAX_I2C_SLAVE_WRITE_Len] { 0x05, 0x00, 0x00, 0x00, 0x67, 0x1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }, // MGMSG_HW_REQ_INFO
            new byte[MAX_I2C_SLAVE_WRITE_Len] { 0x02, 0x46, 0x00, 0x00, 0x67, 0x1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }, // MGMSG_CLK_REQ_INPUT_THRESH
            new byte[MAX_I2C_SLAVE_WRITE_Len] { 0x07, 0x46, 0x00, 0x00, 0x67, 0x1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }, // MGMSG_CLK_REQ_INPUT_STATUS
//            new byte[MAX_I2C_SLAVE_WRITE_Len] { 0x05, 0x46, 0x00, 0x00, 0x67, 0x1, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }, // MGMSG_CLK_REQ_INPUT_SELECT
        };

        // These are WRITE only (I2C Master writes, no data from I2C slave)
        byte[][] MSGOpcodesWriteOnly =
        { 
            // WRITE opcodes                                                    
                                                                                // LFx, INPUT_PATH (0x2 is Narrow band)   
            new byte[MAX_I2C_SLAVE_WRITE_Len] { 0x04, 0x46, 0x00, 0x00, 0x67, 0x1, 0x2, 0x2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },  // MGMSG_CLK_SET_INPUT_SELECT (8 bytes)
                                                                                // auto_level,x,LF0.neg_thresh ... 
            new byte[MAX_I2C_SLAVE_WRITE_Len] { 0x01, 0x46, 0x00, 0x00, 0x67, 0x1, 0x3, 0x0, 0xD1, 0x7, 0xE1, 0x7, 0xD2, 0x7, 0xE2, 0x7, 0, 0, 0, 0, 0, 0 }  // MGMSG_CLK_SET_INPUT_THRESH (16 bytes)
        };

        const int MGMSG_HW_REQ_INFO = 0x0005; // LEN 18
        const int MGMSG_HW_GET_INFO = 0x0006;
        const int MGMSG_CLK_SET_INPUT_THRESH = 0x4601;
        const int MGMSG_CLK_REQ_INPUT_THRESH = 0x4602; // LEN 8
        const int MGMSG_CLK_GET_INPUT_THRESH = 0x4603;
        const int MGMSG_CLK_SET_INPUT_SELECT = 0x4604;
        const int MGMSG_CLK_REQ_INPUT_SELECT = 0x4605;
        const int MGMSG_CLK_GET_INPUT_SELECT = 0x4606;
        const int MGMSG_CLK_REQ_INPUT_STATUS = 0x4607; // LEN 23, add auto_tracking (status)
        const int MGMSG_CLK_GET_INPUT_STATUS = 0x4608;
        const int MGMSG_CLK_SET_OUTPUT_CONFIG = 0x4609;
        const int MGMSG_CLK_REQ_OUTPUT_CONFIG = 0x460A;
        const int MGMSG_CLK_GET_OUTPUT_CONFIG = 0x460B;


        #endregion



        // HEX file format source FLASH: Copied from original Xilinx FPGA flash, modified for I2C
        string s8000HexWarningMsg = "!!! WARNING !!!  This HEX firmware file loads at 0x8000, the address used by " + System.Environment.NewLine +
                            "legacy 3P firmware applications that do NOT support update through FPGA" + System.Environment.NewLine +
                            "Loading this .HEX may make your 3P card NOT updateable again" + System.Environment.NewLine +
                            "If you want to continue re-issue command with '-F' flag";
        public void APIupdateLFT_MCUFirmware(string fileName, bool bOriginal, bool bForce8000Hex)
        {
            string sFinalStatus = " ";
            // Identify the LFT_JA (3P card) I2C slave 
            uint MasterMUXChan = 0x1;
            uint SlaveMUXChan = 0xff;
            uint TargetSlaveAddr = 0x67; // CPLD address on DAC board
            UInt32 I2CbusHz = 100;  // cannot run MCU slave at 400 Hz
            XilinxI2CXfer I2Cxfer = new XilinxI2CXfer();
            bool bFlashUpdateSuccess = false;
            uint retStatus = Win32.STATUS_INCOMPLETE;
            string startBlock = string.Empty;
            string endBlock = string.Empty;
            string lastNVRAMaddr = string.Empty;
            //string startAddr_cmd;
            //string endAddr_cmd;
            // string data_sent;
            bool startBlockSet = false;
            string line;
            string sErr = "OK";
            bool bFirstPysAddrFound = false;
            uint uiStartingNVRAMphyAddr = 0xBAD0FEED;
            uint uiEndingNVRAMphyAddr = 0;
            //byte progStatus;
            Int32 fileSize_byte;
            uint BufferLen = 80;  // max data bytes expected by MCU = 64 (hardware size of NVRAM row)
            Byte[] byteBuf = new Byte[BufferLen];

            // Flash page size of SAMD21 MCU is 64 bytes
            const int FlashPageSize = 64;
            uint status;
            IntPtr UnManagedDataBuffer = IntPtr.Zero;
            UnManagedDataBuffer = Marshal.AllocHGlobal((int)BufferLen); // allocated "unmanaged" buffer
            IntPtr UnManagedOpCodeBuffer = IntPtr.Zero;
            UnManagedOpCodeBuffer = Marshal.AllocHGlobal((int)BufferLen);      // allocated "unmanaged" buffer

            ASCIIEncoding ascii = new ASCIIEncoding();  // for converting EEPROM bytes to ASCII
            System.IO.StreamReader file = new System.IO.StreamReader(fileName);

            ////////////////////////////////////////////////
            // READ MCU FW App version 
            ////////////////////////////////////////////////
            // Typically 3P firmware App is running - send command to force reset back
            // to bootloader...
            bool bI2C_SlaveRead = false;
            MasterMUXChan = 0x1;     // locate the Atmel SAMD21J18 MCU on I2C map
            SlaveMUXChan = 0xFF;     // (no slave MUX)
            TargetSlaveAddr = 0x67;  // MCU Application slave listens to this addr

            // In original code, addressing the MCU (e.g. at I2C address 0x67) caused an immediate RESET
            // FW 2.x now reports its App version number before resetting
            uint DataLen = 18;  // How many bytes of MGMSG_HW_REQ_INFO do we want?  See modified APT def in MCU
                                // definition for API commands for I2C -- all must be 22 bytes
            byte[] OpCodeBuf = new byte[22] { 0x05, 0x00, 0x00, 0x00, 0x67, 0x1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };  // MGMSG_HW_REQ_INFO
            uint OpCodeLen = (uint)OpCodeBuf.Length;
            const int TRIGcardBL_VER_LEN = 21;
            byte[] DataBytes = new byte[TRIGcardBL_VER_LEN];

            Marshal.Copy(OpCodeBuf, 0, UnManagedOpCodeBuffer, (int)OpCodeLen);
            status = ThorDAQAPIXI2CReadWrite(MasterBoardIndex, true,  // read
                TDMasterI2CMUXAddr, MasterMUXChan,
                TDSlaveI2CMUXAddr, SlaveMUXChan, TargetSlaveAddr, I2CbusHz, FlashPageSize,
                UnManagedOpCodeBuffer, ref OpCodeLen, // "OpCodeLen" IN: Opcode byte count (in buffer)
                UnManagedDataBuffer, ref DataLen); // "DataLen" IN: dataLen,  OUT: ALL Opcdoes + Data bytes transfered
                                                   // if this is 2.x FW, we'll have a good status and a version string...
                                                   // for 1.x FW, there is no returned version string
            if ((THORDAQ_STATUS)status == THORDAQ_STATUS.STATUS_SUCCESSFUL)
            {
                // decode the version string
                for (int CPPbyteIndx = 0; CPPbyteIndx < TRIGcardBL_VER_LEN; CPPbyteIndx++)
                {
                    DataBytes[CPPbyteIndx] = Marshal.ReadByte(UnManagedDataBuffer, CPPbyteIndx);
                }
                string sLFT_JAfw = ascii.GetString(DataBytes, 4, 8);
                string sfwVer = DataBytes[16].ToString() + "." + DataBytes[15].ToString() + "." + DataBytes[14].ToString();
                UpdateConsoleStatus("Current F/W Application: " + sLFT_JAfw + " Ver. " + sfwVer);

                DataLen = 1;      // no data returned because MCU will reset
                OpCodeBuf = new byte[22] { 0xA6, 0x00, 0x00, 0x00, 0x67, 0x1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };  // MGMSG_GET_UPDATE_FIRMWARE
                Marshal.Copy(OpCodeBuf, 0, UnManagedOpCodeBuffer, (int)OpCodeLen);
                status = ThorDAQAPIXI2CReadWrite(MasterBoardIndex, true, // read
                    TDMasterI2CMUXAddr, MasterMUXChan,
                    TDSlaveI2CMUXAddr, SlaveMUXChan, TargetSlaveAddr, I2CbusHz, FlashPageSize,
                    UnManagedOpCodeBuffer, ref OpCodeLen, // "OpCodeLen" IN: Opcode byte count (in buffer)
                    UnManagedDataBuffer, ref DataLen); // "DataLen" IN: dataLen,  OUT: ALL Opcdoes + Data bytes transfered
            }
            // (if above version string failed, the attempt already did the reboot [on 1.x FW with I2C slave support...])
            // if the 3P card Application did NOT reboot, it has very old FW which has to be
            // replaced with Segger J-Link or some other probe to update it.

            // It takes about a second for bootloader to start answering...
            System.Threading.Thread.Sleep(1250); // delay for 3P bootloader to startup

            ////////////////////////////////////////////
            // Start BOOTLOADER protocol...
            // Read BL version string 
            ////////////////////////////////////////////
            bI2C_SlaveRead = true;
            TargetSlaveAddr = 0x66;  // MCU BOOTLOADER slave listens to this addr
            OpCodeBuf[0] = 0x11;     // this Opcode starts the BOOTLOADER protocol
            OpCodeLen = 1;
            DataLen = TRIGcardBL_VER_LEN;  // Version string is hardcoded in BL

            Marshal.Copy(OpCodeBuf, 0, UnManagedOpCodeBuffer, (int)OpCodeLen);
            status = ThorDAQAPIXI2CReadWrite(MasterBoardIndex, bI2C_SlaveRead,
                TDMasterI2CMUXAddr, MasterMUXChan,
                TDSlaveI2CMUXAddr, SlaveMUXChan, TargetSlaveAddr, I2CbusHz, FlashPageSize,
                UnManagedOpCodeBuffer, ref OpCodeLen, // "OpCodeLen" IN: Opcode byte count (in buffer)
                UnManagedDataBuffer, ref DataLen); // "DataLen" IN: dataLen,  OUT: ALL Opcdoes + Data bytes transfered
            if (status != Win32.STATUS_SUCCESSFUL)
            {
                sErr = "Misc.";
                if (status == Win32.STATUS_I2C_INVALIDDEV)
                    sErr = "STATUS_I2C_INVALIDDEV";
                else if (status == Win32.STATUS_I2C_INVALIDMUX)
                    sErr = "STATUS_I2C_INVALIDMUX";
                else if (status == Win32.STATUS_I2C_TIMEOUT_ERR)
                    sErr = "STATUS_I2C_TIMEOUT_ERR";
            }
            else // we should have the bootloader version string
            {
                // get the version string into "managed" memory string...
                for (int CPPbyteIndx = 0; CPPbyteIndx < TRIGcardBL_VER_LEN; CPPbyteIndx++)
                {
                    DataBytes[CPPbyteIndx] = Marshal.ReadByte(UnManagedDataBuffer, CPPbyteIndx);
                }
                string sBLver = ascii.GetString(DataBytes);
                UpdateConsoleStatus("Bootloader: " + sBLver);
            }
            if(sErr != "OK")
            {
                UpdateConsoleStatus("Bootloader @I2C slave 0x66 fails to answer: cycle power on computer ");
                return;
            }

            // CONTINUE...
            BackgroundWorker splashWkr = new BackgroundWorker();
            splashWkr.WorkerSupportsCancellation = true;
            UpdateProgressDelegate update;
            _LFTmcuSplash = new SplashScreen();
            _LFTmcuSplash.DisplayText = "Please wait while loading data";
            _LFTmcuSplash.ShowInTaskbar = false;
            _LFTmcuSplash.Owner = Application.Current.MainWindow;
            _LFTmcuSplash.Show();
            _LFTmcuSplash.CancelSplashProgress += delegate (object sender, EventArgs e)
            {
                splashWkr.CancelAsync();
            };


            //////////////////////////////////
            // Send APP PROGRAM to LFT MCU's BOOTLOADER
            //////////////////////////////////
            System.Threading.Thread.Sleep(5); // delay for 3P MCU to process

            uint TotalBytesToFlash = 0; // accumulator for entire file DATA records
            _LTFmcuProgressPercentage = 0; // progress bar reset

            //get dispatcher to update the contents that was created on the UI thread:
            System.Windows.Threading.Dispatcher spDispatcher = _LFTmcuSplash.Dispatcher;

            splashWkr.RunWorkerAsync();
            // now the Delegate "working" function
            // Use the I2C slave on the LFT MCU to update App F/W
            // Adapted from USB update via Thorlabs_CPLD_Update.cs received from E. Leiser (~March 2021)
            //hereeee
            splashWkr.DoWork += delegate (object sender, DoWorkEventArgs DoWrkeVentArgs)
            {
                file.DiscardBufferedData();
                file.BaseStream.Seek(0, System.IO.SeekOrigin.Begin);
                file.BaseStream.Position = 0;
                DoWrkeVentArgs.Cancel = false;

                // We need to discover starting physical address of HEX image
                while (!file.EndOfStream && (DoWrkeVentArgs.Cancel != true))
                {
                    line = file.ReadLine();
                    RecordType recordType = GetRecordType(line);
                    // MCU wants total number of data bytes to flash
                    if (recordType == RecordType.Data)
                    {
                        TotalBytesToFlash += (uint)GetDataLength(line);
                        if (bFirstPysAddrFound == false)
                        {
                            string sAddr = GetAddress(line);
                            uiStartingNVRAMphyAddr = uint.Parse(sAddr, NumberStyles.HexNumber);
                            uiEndingNVRAMphyAddr = uiStartingNVRAMphyAddr;
                            bFirstPysAddrFound = true;
                        }
                    }
                    if (recordType == RecordType.ExtLinearAddress)
                    {
                        if (!startBlockSet)
                        {
                            startBlock = GetData(line);
                            startBlockSet = true;
                        }
                        else
                        {
                            endBlock = GetData(line);
                        }
                    }
                    else if (recordType == RecordType.Data)
                    {
                        lastNVRAMaddr = GetAddress(line);
                    }
                }
                endBlock = endBlock + lastNVRAMaddr;
                fileSize_byte = (Int32.Parse(endBlock, System.Globalization.NumberStyles.HexNumber) / 2);
                //     endAddr_cmd = "0x45" + (fileSize_byte.ToString("X4"));
                //      startAddr_cmd = "0x5341" + startBlock;

                // The new I2C_SLAVE_BOOTLOADER (I2C through FPGA) requires a Firmware APP
                // that supports command (through FPGA) to RESET to 0x0000, to start the 
                // new bootloader.  If user loads a LEGACY .HEX file -- i.e., rx_clk_rev2.hex
                // starting at 0x8000, the 3P card will NOT be updateable through FPGA software.
                // The new load address is 0x2000, so if we see file with 0x8000, we assume it's
                // legacy and issue warning
                if (uiStartingNVRAMphyAddr == 0x8000 && (bForce8000Hex == false))
                {
                    retStatus = Win32.STATUS_OVERFLOW;
                    DoWrkeVentArgs.Cancel = true;
                }
                else if (uiStartingNVRAMphyAddr < 0x2000)
                {
                    retStatus = Win32.STATUS_OVERFLOW;
                    DoWrkeVentArgs.Cancel = true;
                }

                //                WriteFlashMemoryRegister(writeBuffer, RESET_ASYNC);
                // Write Programming Length to MCU

                //  WriteFlashMemoryRegister(writeBuffer, startAddr_cmd);
                //  WriteFlashMemoryRegister(writeBuffer, endAddr_cmd);

                if (DoWrkeVentArgs.Cancel != true)
                {
                    //create a new delegate for updating our progress text
                    update = new UpdateProgressDelegate(UpdateMCUProgressText);
                    //invoke the dispatcher and pass the percentage
                    spDispatcher.BeginInvoke(update, new object[] { _LTFmcuProgressPercentage, "Sending start address & Len to 3P MCU..." });
                    System.Threading.Thread.Sleep(100);
                    //    WriteFlashMemoryRegister(writeBuffer, UNLOCK_CMD);


                    // tell MCU to expect Address
                    bI2C_SlaveRead = false;
                    OpCodeBuf[0] = 0x12; // byte 0
                    OpCodeLen = 1;
                    Marshal.Copy(OpCodeBuf, 0, UnManagedOpCodeBuffer, (int)OpCodeLen);
                    DataLen = 0;
                    status = ThorDAQAPIXI2CReadWrite(MasterBoardIndex, bI2C_SlaveRead,
                        TDMasterI2CMUXAddr, MasterMUXChan,
                        TDSlaveI2CMUXAddr, SlaveMUXChan, TargetSlaveAddr, I2CbusHz, FlashPageSize,
                        UnManagedOpCodeBuffer, ref OpCodeLen, // "OpCodeLen" IN: Opcode byte count (in buffer)
                        UnManagedDataBuffer, ref DataLen); // "DataLen" IN: dataLen,  OUT: ALL Opcdoes + Data bytes transfered

                    System.Threading.Thread.Sleep(1);

                    // Send the MCU starting NVRAM (flash) physical address
                    bI2C_SlaveRead = false;
                    OpCodeBuf[0] = (Byte)(uiStartingNVRAMphyAddr & 0xFF); // byte 0
                    OpCodeBuf[1] = (Byte)((uiStartingNVRAMphyAddr & 0xFF00) >> 8); // byte 0
                    OpCodeBuf[2] = (Byte)((uiStartingNVRAMphyAddr & 0xFF0000) >> 16); // byte 0
                    OpCodeBuf[3] = (Byte)((uiStartingNVRAMphyAddr & 0xFF000000) >> 24); // byte 0
                    OpCodeLen = 4;
                    Marshal.Copy(OpCodeBuf, 0, UnManagedOpCodeBuffer, (int)OpCodeLen);
                    DataLen = 0;

                    status = ThorDAQAPIXI2CReadWrite(MasterBoardIndex, bI2C_SlaveRead,
                        TDMasterI2CMUXAddr, MasterMUXChan,
                        TDSlaveI2CMUXAddr, SlaveMUXChan, TargetSlaveAddr, I2CbusHz, FlashPageSize,
                        UnManagedOpCodeBuffer, ref OpCodeLen, // "OpCodeLen" IN: Opcode byte count (in buffer)
                        UnManagedDataBuffer, ref DataLen); // "DataLen" IN: dataLen,  OUT: ALL Opcdoes + Data bytes transfered

                    if (status != Win32.STATUS_SUCCESSFUL)
                    {
                        sErr = "I2C ERROR";
                        if (status == Win32.STATUS_I2C_INVALIDDEV)
                            sErr = "STATUS_I2C_INVALIDDEV";
                        else if (status == Win32.STATUS_I2C_INVALIDMUX)
                            sErr = "STATUS_I2C_INVALIDMUX";
                        else if (status == Win32.STATUS_I2C_TIMEOUT_ERR)
                            sErr = "STATUS_I2C_TIMEOUT_ERR";

                        DoWrkeVentArgs.Cancel = true;
                        bFlashUpdateSuccess = false;
                        System.Threading.Thread.Sleep(1);
                    }


                    // tell MCU to expect FLASH Program Length 
                    bI2C_SlaveRead = false;
                    OpCodeBuf[0] = 0x13; // byte 0
                    OpCodeLen = 1;
                    Marshal.Copy(OpCodeBuf, 0, UnManagedOpCodeBuffer, (int)OpCodeLen);
                    DataLen = 0;
                    status = ThorDAQAPIXI2CReadWrite(MasterBoardIndex, bI2C_SlaveRead,
                        TDMasterI2CMUXAddr, MasterMUXChan,
                        TDSlaveI2CMUXAddr, SlaveMUXChan, TargetSlaveAddr, I2CbusHz, FlashPageSize,
                        UnManagedOpCodeBuffer, ref OpCodeLen, // "OpCodeLen" IN: Opcode byte count (in buffer)
                        UnManagedDataBuffer, ref DataLen); // "DataLen" IN: dataLen,  OUT: ALL Opcdoes + Data bytes transfered

                    System.Threading.Thread.Sleep(1);


                    // Send the total byte len of NVRAM flash

                    bI2C_SlaveRead = false;
                    OpCodeBuf[0] = (Byte)(TotalBytesToFlash & 0xFF); // byte 0
                    OpCodeBuf[1] = (Byte)((TotalBytesToFlash & 0xFF00) >> 8); // byte 0
                    OpCodeBuf[2] = (Byte)((TotalBytesToFlash & 0xFF0000) >> 16); // byte 0
                    OpCodeBuf[3] = (Byte)((TotalBytesToFlash & 0xFF000000) >> 24); // byte 0
                    OpCodeLen = 4;
                    Marshal.Copy(OpCodeBuf, 0, UnManagedOpCodeBuffer, (int)OpCodeLen);
                    DataLen = 0;

                    status = ThorDAQAPIXI2CReadWrite(MasterBoardIndex, bI2C_SlaveRead,
                        TDMasterI2CMUXAddr, MasterMUXChan,
                        TDSlaveI2CMUXAddr, SlaveMUXChan, TargetSlaveAddr, I2CbusHz, FlashPageSize,
                        UnManagedOpCodeBuffer, ref OpCodeLen, // "OpCodeLen" IN: Opcode byte count (in buffer)
                        UnManagedDataBuffer, ref DataLen); // "DataLen" IN: dataLen,  OUT: ALL Opcdoes + Data bytes transfered

                    System.Threading.Thread.Sleep(1);


                    // tell MCU to INITIALIZE FLASH variables (Addr. and Length & MCU init)
                    bI2C_SlaveRead = false;
                    OpCodeBuf[0] = 0x14; // byte 0
                    OpCodeLen = 1;
                    Marshal.Copy(OpCodeBuf, 0, UnManagedOpCodeBuffer, (int)OpCodeLen);
                    DataLen = 0;
                    status = ThorDAQAPIXI2CReadWrite(MasterBoardIndex, bI2C_SlaveRead,
                        TDMasterI2CMUXAddr, MasterMUXChan,
                        TDSlaveI2CMUXAddr, SlaveMUXChan, TargetSlaveAddr, I2CbusHz, FlashPageSize,
                        UnManagedOpCodeBuffer, ref OpCodeLen, // "OpCodeLen" IN: Opcode byte count (in buffer)
                        UnManagedDataBuffer, ref DataLen); // "DataLen" IN: dataLen,  OUT: ALL Opcdoes + Data bytes transfered

                    System.Threading.Thread.Sleep(1);



                    //create a new delegate for updating our progress text
                    update = new UpdateProgressDelegate(UpdateMCUProgressText);
                    //invoke the dispatcher and pass the percentage
                    spDispatcher.BeginInvoke(update, new object[] { _LTFmcuProgressPercentage, "Programming 3P card MCU Flash" });

                    //Reset sequential read pointer
                    file.DiscardBufferedData();
                    file.BaseStream.Seek(0, System.IO.SeekOrigin.Begin);
                    file.BaseStream.Position = 0;
                    Int32 TotalBytesFlashed = 0;
                    Byte[] FlashSegmentOf64 = new byte[FlashPageSize];
                    for (int indx = 0; indx < FlashPageSize; indx++) FlashSegmentOf64[indx] = 0xFF; // default byte is "erased" value                

                    int SegmentByteIndx = 0; // prepare for first 64 - byte segment
                                             // READ PROGRAM BYTES FROM HEX FILE!
                    while (!file.EndOfStream && (DoWrkeVentArgs.Cancel != true))
                    {
                        // The HEX file usually has 16-byte DATA records, but it can also give 8-byte record, followed by
                        // another contiguous 16-byte DATA record, meaning 64-byte segments are filling within the 
                        // processing of a single HEX file LINE of data
                        //                    BytesThisSegment = 0;
                        while (SegmentByteIndx < FlashPageSize && (DoWrkeVentArgs.Cancel != true))  // expect to stay in while loop until HEX data records exhausted
                        {
                            if (splashWkr.CancellationPending == true) // user abort?
                            {
                                DoWrkeVentArgs.Cancel = true;
                                bFlashUpdateSuccess = false;
                                break;
                            }
                            line = file.ReadLine(); //  grab and process a HEX file line...
                            if (GetRecordType(line) == RecordType.EOF)
                            {
                                bFlashUpdateSuccess = true;
                                // remainding segment will have 0xFF in segment buffer
                                break;
                            }
                            else if (GetRecordType(line) == RecordType.ExtSegmentAddress)
                            {
                                /*02  Extended Segment Address The data field contains a 16 - bit segment base address
                                 * (thus byte count is always 02) compatible with 80x86 real mode addressing. 
                                 * The address field(typically 0000) is ignored.
                                 * The segment address from the most recent 02 record is multiplied by 16 and added to each 
                                 * subsequent data record address to form the physical starting address for the data. 
                                 * This allows addressing up to one megabyte of address space.*/

                                string sDATA = GetData(line); // 2 bytes -- the multiple of 16
                                uint MS_AddrOffset = uint.Parse(sDATA, NumberStyles.HexNumber);
                                MS_AddrOffset *= 16;
                                // set the new MSB offset
                                uiEndingNVRAMphyAddr &= 0x0000FFFF; // clear old segment
                                uiEndingNVRAMphyAddr |= MS_AddrOffset; // add in the address extension above 0xFFFF

                            }
                            else if (GetRecordType(line) == RecordType.Data)
                            {
                                int LineDataLen = GetDataLength(line); // expect 16 or 8

                                string sADDRESS = GetAddress(line);
                                uint LS_AddrOffset = uint.Parse(sADDRESS, NumberStyles.HexNumber); // lower 16-bits of phys. addr
                                uiEndingNVRAMphyAddr &= 0xFFFF0000; // clears older LS offset
                                uiEndingNVRAMphyAddr |= LS_AddrOffset; // set new LS offset
                                string sDATA = GetData(line);
                                for (int DataByteIndx = 0; DataByteIndx < LineDataLen; DataByteIndx++)
                                {
                                    if (splashWkr.CancellationPending == true) // user abort?
                                    {
                                        DoWrkeVentArgs.Cancel = true;
                                        bFlashUpdateSuccess = false;
                                        break;
                                    }

                                    FlashSegmentOf64[SegmentByteIndx++] = byte.Parse(sDATA.Substring(DataByteIndx * 2, 2), NumberStyles.HexNumber); // 2 ASCII hex digits per byte
                                                                                                                                                    // it's possible to cross a 64-byte segment within the processing of a HEX line
                                                                                                                                                    // (e.g. 16 byte records, followed by 8 byte, the 16 again...)
                                    if (SegmentByteIndx >= FlashPageSize)
                                    {
                                        // Segment ready - WRITE the firmware bytes to MCU I2C slave NVRAM
                                        /////////////////////////
                                        bI2C_SlaveRead = false;
                                        OpCodeLen = 0;
                                        DataLen = (uint)FlashPageSize;
                                        //     Buffer.BlockCopy(FlashSegmentOf64, 0, byteBuf, 0, (int)DataLen);
                                        Marshal.Copy(FlashSegmentOf64, 0, UnManagedDataBuffer, (int)DataLen);

                                        status = ThorDAQAPIXI2CReadWrite(MasterBoardIndex, bI2C_SlaveRead,
                                            TDMasterI2CMUXAddr, MasterMUXChan,
                                            TDSlaveI2CMUXAddr, SlaveMUXChan, TargetSlaveAddr, I2CbusHz, FlashPageSize,
                                            UnManagedOpCodeBuffer, ref OpCodeLen, // "OpCodeLen" IN: Opcode byte count (in buffer)
                                            UnManagedDataBuffer, ref DataLen); // "DataLen" IN: dataLen,  OUT: ALL Opcdoes + Data bytes transfered

                                        if (status != Win32.STATUS_SUCCESSFUL)
                                        {
                                            sErr = "I2C ERROR";
                                            if (status == Win32.STATUS_I2C_INVALIDDEV)
                                                sErr = "STATUS_I2C_INVALIDDEV";
                                            else if (status == Win32.STATUS_I2C_INVALIDMUX)
                                                sErr = "STATUS_I2C_INVALIDMUX";
                                            else if (status == Win32.STATUS_I2C_TIMEOUT_ERR)
                                                sErr = "STATUS_I2C_TIMEOUT_ERR";

                                            DoWrkeVentArgs.Cancel = true;
                                            bFlashUpdateSuccess = false;
                                            break;
                                        }

                                        System.Threading.Thread.Sleep(6); // @100KHz it takes ~6 ms to send 64 bytes

                                        // tell MCU to COMMIT 64-byte Segment
                                        bI2C_SlaveRead = false;
                                        OpCodeBuf[0] = 0x15; // byte 0
                                        OpCodeLen = 1;
                                        Marshal.Copy(OpCodeBuf, 0, UnManagedOpCodeBuffer, (int)OpCodeLen);
                                        DataLen = 0;
                                        status = ThorDAQAPIXI2CReadWrite(MasterBoardIndex, bI2C_SlaveRead,
                                            TDMasterI2CMUXAddr, MasterMUXChan,
                                            TDSlaveI2CMUXAddr, SlaveMUXChan, TargetSlaveAddr, I2CbusHz, FlashPageSize,
                                            UnManagedOpCodeBuffer, ref OpCodeLen, // "OpCodeLen" IN: Opcode byte count (in buffer)
                                            UnManagedDataBuffer, ref DataLen); // "DataLen" IN: dataLen,  OUT: ALL Opcdoes + Data bytes transfered

                                        System.Threading.Thread.Sleep(1);


                                        // Query MCU for completion of page...
                                        bI2C_SlaveRead = true;
                                        OpCodeBuf[0] = 0x16;  // Query status, e.g. 0x0 OK, 0xB Busy, 0xE Error
                                        OpCodeLen = 1;
                                        Marshal.Copy(OpCodeBuf, 0, UnManagedOpCodeBuffer, (int)OpCodeLen);

                                        long lIterations = 0x64;
                                        int MCUpolledStatus;
                                        do
                                        {
                                            OpCodeLen = 1;
                                            DataLen = 1;
                                            status = ThorDAQAPIXI2CReadWrite(MasterBoardIndex, bI2C_SlaveRead,  // POLL MCU
                                                TDMasterI2CMUXAddr, MasterMUXChan,
                                                TDSlaveI2CMUXAddr, SlaveMUXChan, TargetSlaveAddr, I2CbusHz, FlashPageSize,
                                                UnManagedOpCodeBuffer, ref OpCodeLen, // "OpCodeLen" IN: Opcode byte count (in buffer)
                                                UnManagedDataBuffer, ref DataLen); // "DataLen" IN: dataLen,  OUT: ALL Opcdoes + Data bytes transfered

                                            MCUpolledStatus = Marshal.ReadByte(UnManagedDataBuffer, 0);
                                            // get MCU slave response byte...
                                            // status "0" is NOT busy (done/OK), "0x0B" is Busy, "0x0E" is fatal Error
                                            if ((status != Win32.STATUS_SUCCESSFUL) || MCUpolledStatus == 0x0E) // xmit error, or MCU err?
                                            {
                                                sErr = "I2C ERROR";
                                                if (MCUpolledStatus == 0x0E)
                                                {
                                                    sErr = "STATUS_MCU_FLASH_ERR";
                                                }
                                                else if (status != Win32.STATUS_SUCCESSFUL)
                                                {
                                                    if (status == Win32.STATUS_I2C_INVALIDDEV)
                                                        sErr = "STATUS_I2C_INVALIDDEV";
                                                    else if (status == Win32.STATUS_I2C_INVALIDMUX)
                                                        sErr = "STATUS_I2C_INVALIDMUX";
                                                    else if (status == Win32.STATUS_I2C_TIMEOUT_ERR)
                                                        sErr = "STATUS_I2C_TIMEOUT_ERR";
                                                }
                                                DoWrkeVentArgs.Cancel = true;   // force early exit
                                                bFlashUpdateSuccess = false;
                                                System.Threading.Thread.Sleep(1);
                                                break;
                                            }
                                            // no error - are we done?
                                            if (MCUpolledStatus == 0)
                                                break;
                                            System.Threading.Thread.Sleep(5);  // pause before next POLL
                                        } while (--lIterations > 0);

                                        if (lIterations <= 0)
                                        {
                                            DoWrkeVentArgs.Cancel = true;   // force early exit
                                            bFlashUpdateSuccess = false;
                                            sErr = "STATUS_I2C_TIMEOUT_ERR";
                                        }

                                        TotalBytesFlashed += FlashPageSize; // MCU expects 64 bytes at a time
                                                                            //////////////////////////
                                        SegmentByteIndx = 0;  // start new 64-byte segment
                                        for (int indx = 0; indx < FlashPageSize; indx++) FlashSegmentOf64[indx] = 0xFF; // default byte is "erased" value                
                                        _LTFmcuProgressPercentage = (int)((float)TotalBytesFlashed / (float)TotalBytesToFlash * 100);
                                        //create a new delegate for updating our progress text
                                        update = new UpdateProgressDelegate(UpdateMCUProgressText);
                                        //invoke the dispatcher and pass the percentage
                                        spDispatcher.BeginInvoke(update, new object[] { _LTFmcuProgressPercentage, "Programming 3P MCU Flash" });
                                    }
                                } // finished ALL DATA bytes in this HEX file line
                            }
                        } // 64 byte segment accumulation
                    } // END while reading file

                    // It's likely to have a partial 64-byte segment filled when the HEX DATA is all read
                    // In that case, send the final, partially filled, 64-byte segment (unfilled part 0xFF)
                    if (SegmentByteIndx > 0 && (DoWrkeVentArgs.Cancel == false))
                    {
                        // Final partial segment - write the firmware bytes to MCU I2C slave NVRAM
                        /////////////////////////
                        // Segment ready - WRITE the firmware bytes to MCU I2C slave NVRAM
                        /////////////////////////
                        bI2C_SlaveRead = false;
                        OpCodeLen = 0;
                        DataLen = (uint)FlashPageSize;
                        //Buffer.BlockCopy(FlashSegmentOf64, 0, byteBuf, 0, (int)DataLen);
                        Marshal.Copy(FlashSegmentOf64, 0, UnManagedDataBuffer, (int)DataLen);

                        status = ThorDAQAPIXI2CReadWrite(MasterBoardIndex, bI2C_SlaveRead,
                            TDMasterI2CMUXAddr, MasterMUXChan,
                            TDSlaveI2CMUXAddr, SlaveMUXChan, TargetSlaveAddr, I2CbusHz, FlashPageSize,
                            UnManagedOpCodeBuffer, ref OpCodeLen, // "OpCodeLen" IN: Opcode byte count (in buffer)
                            UnManagedDataBuffer, ref DataLen); // "DataLen" IN: dataLen,  OUT: ALL Opcdoes + Data bytes transfered

                        System.Threading.Thread.Sleep(6); // @100KHz it takes ~6 ms to send 64 bytes

                        // tell MCU to COMMIT 64-byte Segment
                        bI2C_SlaveRead = false;
                        OpCodeBuf[0] = 0x15; // byte 0
                        OpCodeLen = 1;
                        Marshal.Copy(OpCodeBuf, 0, UnManagedOpCodeBuffer, (int)OpCodeLen);
                        DataLen = 0;
                        status = ThorDAQAPIXI2CReadWrite(MasterBoardIndex, bI2C_SlaveRead,
                            TDMasterI2CMUXAddr, MasterMUXChan,
                            TDSlaveI2CMUXAddr, SlaveMUXChan, TargetSlaveAddr, I2CbusHz, FlashPageSize,
                            UnManagedOpCodeBuffer, ref OpCodeLen, // "OpCodeLen" IN: Opcode byte count (in buffer)
                            UnManagedDataBuffer, ref DataLen); // "DataLen" IN: dataLen,  OUT: ALL Opcdoes + Data bytes transfered

                        System.Threading.Thread.Sleep(1);

                        // do not POLL MCU for last page... MCU jumps to F/W App start on flash completion

                        TotalBytesFlashed += FlashPageSize; // MCU expects 64 bytes at a time
                        _LTFmcuProgressPercentage = 99;
                    }
                    retStatus = 0; // success
                    _LTFmcuProgressPercentage += 1;
                }// end of loading 64-byte segments logic
                string MCUstartAddr = string.Format("{0:X4}", uiStartingNVRAMphyAddr);
                sFinalStatus = "FLASH @ " + MCUstartAddr;
                if (retStatus == 0)
                {
                    sFinalStatus += (bFlashUpdateSuccess) ? (" SUCCESS: POWER" + Environment.NewLine + " CYCLE now!") : "Failed";
                }
                //         UpdateConsoleStatus("Update " + sStatus);
                else if (retStatus == Win32.STATUS_OVERFLOW)
                {
                    if (uiStartingNVRAMphyAddr < 0x2000)
                    {
                        sFinalStatus += "ERROR - HEX image can't be located below 0x2000 in NVRAM";
                    }
                    else
                    {
                        sFinalStatus += "ERROR " + s8000HexWarningMsg;
                    }
                }
                update = new UpdateProgressDelegate(UpdateMCUProgressText);
                spDispatcher.BeginInvoke(update, new object[] { _LTFmcuProgressPercentage, "MCU Flash Update: " + sFinalStatus });
                System.Threading.Thread.Sleep(2500);
            }; // End of DoWork DELEGATE
            splashWkr.RunWorkerCompleted += delegate (object sender, RunWorkerCompletedEventArgs e)
            {
                if (bFlashUpdateSuccess) retStatus = Win32.STATUS_SUCCESSFUL;
                file.Close();
                if (UnManagedDataBuffer != IntPtr.Zero)
                    Win32.FreeBuffer(UnManagedDataBuffer);
                _LFTmcuSplash.Close();
                // App inherits from Application, and has a Window property called MainWindow
                // and a List<Window> property called OpenWindows.
                Application.Current.MainWindow.Activate();
                _LTFmcuProgressPercentage = 0;


                if (retStatus == Win32.STATUS_OVERFLOW)
                    UpdateConsoleStatus(sFinalStatus);
            };

            return;
        }

        void APIupdateLFT_CPLDFirmware( string fileName)
        {
            // Identify the LFT_JA (3P card) I2C slave 
            const uint MasterMUXChan = 0x1;
            const uint SlaveMUXChan = 0xff;
            const uint TargetSlaveAddr = 0x67; // CPLD address on DAC board
            XilinxI2CXfer I2Cxfer = new XilinxI2CXfer();

            System.IO.StreamReader file = new System.IO.StreamReader(fileName);
            BackgroundWorker splashWkr = new BackgroundWorker();
            splashWkr.WorkerSupportsCancellation = true;
            _LFTcpldSplash = new SplashScreen();
            _LFTcpldSplash.DisplayText = "Please wait while loading data";
            _LFTcpldSplash.ShowInTaskbar = false;
            _LFTcpldSplash.Owner = Application.Current.MainWindow;
            _LFTcpldSplash.Show();
            _LFTcpldSplash.CancelSplashProgress += delegate (object sender, EventArgs e)
            {
                splashWkr.CancelAsync();
            };
            //get dispatcher to update the contents that was created on the UI thread:
            System.Windows.Threading.Dispatcher spDispatcher = _LFTcpldSplash.Dispatcher;
            splashWkr.RunWorkerAsync();

            // now the Delegate "working" function
            // Use the I2C slave on the LFT MCU to program CPLD
            // Adapted from USB update via Thorlabs_CPLD_Update.cs received from E. Leiser (~March 2021)
            splashWkr.DoWork += delegate (object sender, DoWorkEventArgs e)
            {
                THORDAQ_STATUS I2Cstatus = THORDAQ_STATUS.STATUS_INCOMPLETE;
                const int PageReadWriteLen = 16;
                const int APTcmdLen = 6;
                int dbg1 = 0;
                double PercentDone = 0;
                Int16 currentLineIndex = 1, j, k;
                string new_line;
                Int32 fuse_count;  // this is the total real fuse count = lines * 128bits
                string[] new_lines = new string[8000];
                Int16 line_count = 0;
                string temp;
                byte[,] lines_in_hex_format = new byte[8000, 16];
                string lines = "CPLD LINES\r\n";
                byte MCU_CPLD_status;
                // Set up the variables needed for the API
                uint TotalDataPlusOpcodesBytesTransfered;
                bool b_I2CReadDir = false;
                uint OpcodeByteLen, DATAtransferBufferLen;
                byte[] OpcodeBytes = new byte[APTcmdLen]; // OpCode count per APT command, plus MAX possible data 16 bytes
                byte[] DataBytes = new byte[PageReadWriteLen]; //  DATA count per Lattice MACHx02 command
                IntPtr UnManagedOpCodeBuffer = Marshal.AllocHGlobal(OpcodeBytes.Length);
                IntPtr UnManagedDataBuffer = Marshal.AllocHGlobal(DataBytes.Length);

                OpcodeByteLen = (uint)OpcodeBytes.Length;
                // MachX02 program DATA length is 16
                DATAtransferBufferLen = (uint)DataBytes.Length;

                string sFinalStatus = "ERROR";
                UpdateProgressDelegate UpdatePct;
                //////////////
                try
                {
                    System.IO.StreamReader JEDfile = new System.IO.StreamReader(fileName);

                    // read file until we reach the QF
                    // The key word QF identifies the total real fuse count of the device
                    new_line = JEDfile.ReadLine();
                    do
                    {
                        new_line = JEDfile.ReadLine();  // skipping header text at beginning of file
                    } while (!new_line.StartsWith("QF"));

                    new_line = new_line.Substring(2, new_line.Length - 3); // remove the QF and the '*' at the end
                    fuse_count = Convert.ToInt32(new_line);
                    // ------------------------------------------------------------------ 
                    _LFTprogressPercentage = (int)(PercentDone += 1);
                    //create a new delegate for updating our progress text
                    UpdatePct = new UpdateProgressDelegate(LFT_UpdateCPLDProgressText);
                    spDispatcher.BeginInvoke(UpdatePct, new object[] { _LFTprogressPercentage, "LFT CPLD Setup" });

                    System.Threading.Thread.Sleep(100);
                    // goto the L000000 line
                    new_line = JEDfile.ReadLine();
                    do
                    {
                        new_line = JEDfile.ReadLine();
                    } while (!new_line.StartsWith("L")); // this gets us past the initial "NOTE" sections
                    // ------------------------------------------------------------------ 

                    // this is where the configuration data starts
                    // read to the NOTE '*' just before next line "END CONFIG DATA* " and count how many lines we are sending
                    do
                    {
                        new_lines[line_count++] = JEDfile.ReadLine();
                    } while (!new_lines[line_count - 1].StartsWith("*"));
                    line_count--;
                    // ------------------------------------------------------------------ 
                    _LFTprogressPercentage = (int)(PercentDone += 1);
                    UpdatePct = new UpdateProgressDelegate(LFT_UpdateCPLDProgressText);
                    spDispatcher.BeginInvoke(UpdatePct, new object[] { _LFTprogressPercentage, "LFT CPLD Update Program Setup" });

                    // send command to switch MCU to CPLD programming mode...
                    // this command stops the JitterAttenuator service loop, resets CPLD, starts ERASE, which takes about 1.5 seconds
                    //
                    OpcodeBytes = new byte[APTcmdLen+16] { 0x55, 0x08, 0x00, 0x00, (byte)TargetSlaveAddr, 0x1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; // MGMSG_LATTICE_JED_UPDATE
                    OpcodeByteLen = (uint)OpcodeBytes.Length;
                    DATAtransferBufferLen = 6; // STATUS APT response
                    b_I2CReadDir = true;       // READ the STATUS in response to MGMSG_LATTICE_JED_UPDATE
                    // (same I2C function as DAC CPLD programming uses...)
                    I2Cstatus = I2Cxfer.I2CTransfer(MasterMUXChan, SlaveMUXChan, TargetSlaveAddr, b_I2CReadDir, PageReadWriteLen, OpcodeBytes, OpcodeByteLen, DataBytes, DATAtransferBufferLen, out TotalDataPlusOpcodesBytesTransfered);
                    if (I2Cstatus != THORDAQ_STATUS.STATUS_SUCCESSFUL)
                    {
                        LFTworkerThreadStatus = THORDAQ_STATUS.STATUS_INCOMPLETE;
                        spDispatcher.BeginInvoke(UpdatePct, new object[] { _LFTprogressPercentage, "LFT CPLD Update: MGMSG_LATTICE_JED_UPDATE I2C comm Error, Status: 0x" + I2Cstatus.ToString("X") });
                        System.Threading.Thread.Sleep(2000);
                        goto ExitEarly;
                    }
                    System.Threading.Thread.Sleep(1600); // longest delay is to reset CPLD and ERASE flash

                    MCU_CPLD_status = DataBytes[3];
                    // I2C slave is half duplex - can never initiate communication
                    // Send separate POLL (READ) command
                    if (MCU_CPLD_status != 0)
                    {
                        LFTworkerThreadStatus = THORDAQ_STATUS.STATUS_INCOMPLETE;
                        spDispatcher.BeginInvoke(UpdatePct, new object[] { _LFTprogressPercentage, "LFT CPLD Update ERROR: MGMSG_LATTICE_JED_UPDATE cmd Status: 0x" + MCU_CPLD_status.ToString("X")});
                        System.Threading.Thread.Sleep(2000);
                        goto ExitEarly;
                    }
                    const byte CONFIGURATION_FLASH_START = 0;
                    const byte CONFIGURATION_FLASH_PROGRAM = 1;
                    const byte WRITE_FEATURE_ROW = 2;
                    const byte WRITE_FEABITS = 3;
                    //const byte REFRESH_CPLD = 4;  // i.e. MachX02 LSC_REFRESH 0x79 00 00
                    const byte WRITE_USERCODE = 5;
                    const byte READ_CPLD_PGM_STATUS = 9;

                    // did ERASE command succeed and is MCU's CPLD ready??
                    OpcodeBytes = new byte[APTcmdLen + 16] { 0xA9, 0x00, READ_CPLD_PGM_STATUS, 0x00, (byte)TargetSlaveAddr, 0x1, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // MGMSG_READ_CPLD_PGM_STATUS
                    OpcodeByteLen = (uint)OpcodeBytes.Length;
                    DATAtransferBufferLen = 6; // READ 6 APT bytes (for single Status byte at Param2)
                    b_I2CReadDir = true;       // reading STATUS (APT format)
                    I2Cstatus = I2Cxfer.I2CTransfer(MasterMUXChan, SlaveMUXChan, TargetSlaveAddr, b_I2CReadDir, PageReadWriteLen, OpcodeBytes, OpcodeByteLen, DataBytes, DATAtransferBufferLen, out TotalDataPlusOpcodesBytesTransfered);
                    // get the status byte...
                    MCU_CPLD_status = DataBytes[3];
                    if (MCU_CPLD_status != 0)
                    {
                        // error
                        LFTworkerThreadStatus = THORDAQ_STATUS.STATUS_INCOMPLETE;
                        spDispatcher.BeginInvoke(UpdatePct, new object[] { _LFTprogressPercentage, "Update FLASH ERROR: CPLD ERASE failed, Status: 0x" + MCU_CPLD_status.ToString("X") });
                        System.Threading.Thread.Sleep(2000);
                        goto ExitEarly;
                        // error
                    }

                    // CONFIGURATION_FLASH_START
                    // send the number of lines for the configuration flash
                    byte[] JEDlineCount = BitConverter.GetBytes(line_count);
                                                                        // [2]Param1                       [4]                  [5]   [6] DATA....
                    OpcodeBytes = new byte[APTcmdLen + 16] { 0xA8, 0x00, CONFIGURATION_FLASH_START, 0x00, (byte)TargetSlaveAddr, 0x1, JEDlineCount[0], JEDlineCount[1], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // MGMSG_CPLD_REPROGRAM
                    OpcodeByteLen = (uint)OpcodeBytes.Length;
                    DATAtransferBufferLen = 0; // no READ data
                    b_I2CReadDir = false;       // WRITE line count  

                    I2Cstatus = I2Cxfer.I2CTransfer(MasterMUXChan, SlaveMUXChan, TargetSlaveAddr, b_I2CReadDir, PageReadWriteLen, OpcodeBytes, OpcodeByteLen, DataBytes, DATAtransferBufferLen, out TotalDataPlusOpcodesBytesTransfered);
                    if (I2Cstatus != THORDAQ_STATUS.STATUS_SUCCESSFUL)
                    {
                        LFTworkerThreadStatus = THORDAQ_STATUS.STATUS_INCOMPLETE;
                        spDispatcher.BeginInvoke(UpdatePct, new object[] { _LFTprogressPercentage, "Update: CONFIGURATION_FLASH_START I2C comm Error, Status: 0x" + I2Cstatus.ToString("X") });
                        System.Threading.Thread.Sleep(2000);
                        goto ExitEarly;
                    }
                    // once program mode is entered, it takes about 1.6 seconds to get READY status
                    // from CPLD command {0xFF 00 00 00} ...
                    System.Threading.Thread.Sleep(4); 

                    // MOVE to FLASH_PROGRAM phase...

                    // CONFIGURATION_FLASH_PROGRAM
                    // send line by line of the configuration flash
                    double dPctIncreasePerLine = (99.0 - _LFTprogressPercentage) / line_count;
                    double dProgressPercent = _LFTprogressPercentage;

                    for (currentLineIndex = 0; currentLineIndex < line_count; currentLineIndex++)  // for ALL .JED FLASH "programming" lines 
                    {
                        dProgressPercent += dPctIncreasePerLine;
                        _LFTprogressPercentage = (int)Math.Round(dProgressPercent);
                        UpdatePct = new UpdateProgressDelegate(LFT_UpdateCPLDProgressText);
                        spDispatcher.BeginInvoke(UpdatePct, new object[] { _LFTprogressPercentage, "CPLD Program FLASHING..." });

                        lines += currentLineIndex.ToString();
                        lines += "- ";
                        k = 0;

                        // Send programming bytes as I2C WRITE (to slave)
                        //                                                  [2]Param1                               [3]                         [4]          [5]  [6] DATA (fill in from .JED file)
                        OpcodeBytes = new byte[APTcmdLen + 16] { 0xA8, 0x00, CONFIGURATION_FLASH_PROGRAM, (byte)(currentLineIndex & 0xFF), (byte)(currentLineIndex >> 8), 0x1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // MGMSG_CPLD_REPROGRAM

                        UInt32 ProgramDataSum = 0;
                        // convert line string to binary
                        for (j = 0; j < 16; j++)
                        {
                            temp = new_lines[currentLineIndex].Substring(k, 8);
                            DataBytes[j] = Convert.ToByte(temp, 2);
                            ProgramDataSum += DataBytes[j]; // test for requirement to send this line...
                            OpcodeBytes[6 + j] = DataBytes[j]; // copy to I2C data (OpCode) buffer
                            lines_in_hex_format[currentLineIndex, j] = DataBytes[j];
                            k += 8;
                            lines += DataBytes[j].ToString("D3");
                            lines += " ";
                        }
                        lines += "\r\n";

                        // Lattic MachX02 "Programming and Configuration Usage Guide", Table 21 (pg. 41) [July 2017] -- 
                        // "It is not necessary to program any page data containing all '0' values"
                        if (ProgramDataSum == 0) continue; // skip line

                        // now WRITE the 16-byte "line" to I2C slave
                        OpcodeByteLen = (uint)OpcodeBytes.Length;
                        DATAtransferBufferLen = 0; // no READ data
                        b_I2CReadDir = false;       // WRITE 16-bytes of FLASH program data

                        I2Cstatus = I2Cxfer.I2CTransfer(MasterMUXChan, SlaveMUXChan, TargetSlaveAddr, b_I2CReadDir, PageReadWriteLen, OpcodeBytes, OpcodeByteLen, DataBytes, DATAtransferBufferLen, out TotalDataPlusOpcodesBytesTransfered);
                        if (I2Cstatus != THORDAQ_STATUS.STATUS_SUCCESSFUL)
                        {
                            LFTworkerThreadStatus = THORDAQ_STATUS.STATUS_INCOMPLETE;
                            spDispatcher.BeginInvoke(UpdatePct, new object[] { _LFTprogressPercentage, "Update: MGMSG_CPLD_REPROGRAM(CONFIGURATION_FLASH_PROGRAM) I2C comm Error, Status: 0x" + I2Cstatus.ToString("X") });
                            goto ExitEarly;
                        }

                        System.Threading.Thread.Sleep(2); // provide real-time for MCU's SPI bus to transfer data to CPLD, etc.

                        // POLL for MCU's CPLD programming status response 
                        //                System.Threading.Thread.Sleep(900);
                        for (int attempt = 0; attempt < 3; attempt++)
                        {
                            //                                                      [2]Param1       [3]Param2   [4]                [5]   [6] DATA....
                            OpcodeBytes = new byte[APTcmdLen + 16] { 0xA9, 0x00, READ_CPLD_PGM_STATUS, 0x00, (byte)TargetSlaveAddr, 0x1, (byte)(currentLineIndex & 0xFF), (byte)(currentLineIndex >> 8), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // MGMSG_READ_CPLD_PGM_STATUS
                            OpcodeByteLen = (uint)OpcodeBytes.Length;
                            DATAtransferBufferLen = 6; // READ 6 APT bytes (for single Status byte at Param2)
                            b_I2CReadDir = true;       // reading STATUS (APT format)
                            I2Cstatus = I2Cxfer.I2CTransfer(MasterMUXChan, SlaveMUXChan, TargetSlaveAddr, b_I2CReadDir, PageReadWriteLen, OpcodeBytes, OpcodeByteLen, DataBytes, DATAtransferBufferLen, out TotalDataPlusOpcodesBytesTransfered);
                            if (I2Cstatus == THORDAQ_STATUS.STATUS_SUCCESSFUL) break;
                            // failed - wait and try again
                            System.Threading.Thread.Sleep(5);
                        }
                        if (I2Cstatus != THORDAQ_STATUS.STATUS_SUCCESSFUL)
                        {
                            LFTworkerThreadStatus = THORDAQ_STATUS.STATUS_INCOMPLETE;
                            spDispatcher.BeginInvoke(UpdatePct, new object[] { _LFTprogressPercentage, "Update: MGMSG_READ_CPLD_PGM_STATUS I2C comm Error, Status: 0x" + I2Cstatus.ToString("X") });
                            System.Threading.Thread.Sleep(2000);
                            goto ExitEarly;
                        }
                        // get the status byte...
                        MCU_CPLD_status = DataBytes[3];
                        if (MCU_CPLD_status != 0)
                        {
                            // error
                            LFTworkerThreadStatus = THORDAQ_STATUS.STATUS_INCOMPLETE;
                            spDispatcher.BeginInvoke(UpdatePct, new object[] { _LFTprogressPercentage, "Update FLASH ERROR: MGMSG_READ_CPLD_PGM_STATUS: 0x" + MCU_CPLD_status.ToString("X") });
                            System.Threading.Thread.Sleep(2000);
                            goto ExitEarly;
                            // error
                        }
                    } // end of for(line count), for user's flash program

                    // debug
                    System.IO.StreamWriter filelines = new System.IO.StreamWriter(@"\temp\CPLD Lines.txt", false);
                    filelines.WriteLine(lines);
                    filelines.Close();

                    //////////////////////////////////////////////////
                    /// 
                    /// See Mach XO2 Programming and Configuration Usage Guide, TN1204, July 2017, Table 21, pg. 41 / 57
                    /// JEDEC file format
                    /// "E" Field (start of line) is JEDEC architecture FUSES - Lattice uses 1st line (following "E")
                    ///     as "Feature Row", the second line as "FEABITS"
                    /// "U" Field (start of line) is Lattice USERCODE field, e.g. 55 41 31 30 32 33 2A or "UA1023*"
                    ///     where 1023 is the ASCII USERCODE
                    int iExpectedLines = 0x55A00; // put generous limit on file line iterations
                    do
                    {
                        new_lines[0] = JEDfile.ReadLine(); // "next" line
                        if (new_lines[0].StartsWith("E")) break;
                    } while (--iExpectedLines > 0);
                    // Feature Row
                    //                                                  [2]Param1                               [4]     [5]  [6] DATA (fill in from .JED file)
                    OpcodeBytes = new byte[APTcmdLen + 16] { 0xA8, 0x00, WRITE_FEATURE_ROW, (byte)(currentLineIndex & 0xFF), (byte)(currentLineIndex >> 8), 0x1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // MGMSG_CPLD_REPROGRAM
                    for (j = 0, k = 1; j < 8; j++) // total of 64 bit (8 bytes)
                    {
                        temp = new_lines[0].Substring(k, 8); // (8 bits in a byte)
                        
                        DataBytes[j] = Convert.ToByte(temp, 2); // binary conversion, e.g. '00100000' is byte val 0x20 
                        OpcodeBytes[6 + j] = DataBytes[j]; // copy to I2C data (OpCode) buffer

                        k += 8;
                    }
                    // Send "feature row" programming bytes as I2C WRITE (to slave)
                    OpcodeByteLen = (uint)OpcodeBytes.Length;
                    DATAtransferBufferLen = 0; // no READ data
                    b_I2CReadDir = false;      // WRITING 8-bytes of FEATURE ROW program data
                    I2Cstatus = I2Cxfer.I2CTransfer(MasterMUXChan, SlaveMUXChan, TargetSlaveAddr, b_I2CReadDir, PageReadWriteLen, OpcodeBytes, OpcodeByteLen, DataBytes, DATAtransferBufferLen, out TotalDataPlusOpcodesBytesTransfered);
                    if (I2Cstatus != THORDAQ_STATUS.STATUS_SUCCESSFUL)
                    {
                        LFTworkerThreadStatus = THORDAQ_STATUS.STATUS_INCOMPLETE;
                        spDispatcher.BeginInvoke(UpdatePct, new object[] { _LFTprogressPercentage, "Update: MGMSG_CPLD_REPROGRAM(WRITE_FEATURE_ROW) I2C comm Error, Status: 0x" + I2Cstatus.ToString("X") });
                        goto ExitEarly;
                    }
                    if (I2Cstatus != THORDAQ_STATUS.STATUS_SUCCESSFUL)
                    {
                        LFTworkerThreadStatus = THORDAQ_STATUS.STATUS_INCOMPLETE;
                        spDispatcher.BeginInvoke(UpdatePct, new object[] { _LFTprogressPercentage, "Update: MGMSG_CPLD_REPROGRAM(WRITE_FEATURE_ROW) I2C comm Error, Status: 0x" + I2Cstatus.ToString("X") });
                        goto ExitEarly;
                    }

                    // POLL for status...
                    OpcodeBytes = new byte[APTcmdLen + 16] { 0xA9, 0x00, READ_CPLD_PGM_STATUS, 0x00, (byte)TargetSlaveAddr, 0x1, (byte)(currentLineIndex & 0xFF), (byte)(currentLineIndex >> 8), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // MGMSG_READ_CPLD_PGM_STATUS
                    OpcodeByteLen = (uint)OpcodeBytes.Length;
                    DATAtransferBufferLen = 6; // READ 6 APT bytes (for single Status byte at Param2)
                    b_I2CReadDir = true;       // reading STATUS (APT format)
                                               
                    I2Cstatus = I2Cxfer.I2CTransfer(MasterMUXChan, SlaveMUXChan, TargetSlaveAddr, b_I2CReadDir, PageReadWriteLen, OpcodeBytes, OpcodeByteLen, DataBytes, DATAtransferBufferLen, out TotalDataPlusOpcodesBytesTransfered);
                    if (I2Cstatus != THORDAQ_STATUS.STATUS_SUCCESSFUL)
                    {
                        LFTworkerThreadStatus = THORDAQ_STATUS.STATUS_INCOMPLETE;
                        spDispatcher.BeginInvoke(UpdatePct, new object[] { _LFTprogressPercentage, "Update: MGMSG_READ_CPLD_PGM_STATUS I2C comm Error, Status: 0x" + I2Cstatus.ToString("X") });
                        System.Threading.Thread.Sleep(2000);
                        goto ExitEarly;
                    }
                    // get the status byte...
                    MCU_CPLD_status = DataBytes[3];
                    if (MCU_CPLD_status != 0)
                    {
                        LFTworkerThreadStatus = THORDAQ_STATUS.STATUS_INCOMPLETE; // error
                        spDispatcher.BeginInvoke(UpdatePct, new object[] { _LFTprogressPercentage, "Update FLASH ERROR: MGMSG_READ_CPLD_PGM_STATUS: 0x" + MCU_CPLD_status.ToString("X") });
                        System.Threading.Thread.Sleep(2000);
                        goto ExitEarly;
                    }
                    ///////////////////////////////////////////////////////////////////////////////////////
                    // WRITE_FEABITS

                    currentLineIndex++;

                    dProgressPercent += dPctIncreasePerLine;
                    _LFTprogressPercentage = (int)Math.Round(dProgressPercent); UpdatePct = new UpdateProgressDelegate(LFT_UpdateCPLDProgressText);
                    spDispatcher.BeginInvoke(UpdatePct, new object[] { _LFTprogressPercentage, "CPLD FEA Bits FLASHING..." });

                    new_lines[0] = JEDfile.ReadLine();
                    OpcodeBytes = new byte[APTcmdLen + 16] { 0xA8, 0x00, WRITE_FEABITS, (byte)(currentLineIndex & 0xFF), (byte)(currentLineIndex >> 8), 0x1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // MGMSG_CPLD_REPROGRAM
                    // convert line string to binary
                    k = 0;
                    for (j = 0; j < 2; j++)
                    {
                        temp = new_lines[0].Substring(k, 8);
                        DataBytes[j] = Convert.ToByte(temp, 2);
                        OpcodeBytes[6 + j] = DataBytes[j]; // copy to I2C data (OpCode) buffer
                        k += 8;
                    }
                    System.Threading.Thread.Sleep(100);

                    // WRITE the 2(two) FEA bits to MCU
                    OpcodeByteLen = (uint)OpcodeBytes.Length;
                    DATAtransferBufferLen = 0; // no READ data
                    b_I2CReadDir = false;       // WRITE 16-bytes of FLASH program data

                    I2Cstatus = I2Cxfer.I2CTransfer(MasterMUXChan, SlaveMUXChan, TargetSlaveAddr, b_I2CReadDir, PageReadWriteLen, OpcodeBytes, OpcodeByteLen, DataBytes, DATAtransferBufferLen, out TotalDataPlusOpcodesBytesTransfered);
                    if (I2Cstatus != THORDAQ_STATUS.STATUS_SUCCESSFUL)
                    {
                        LFTworkerThreadStatus = THORDAQ_STATUS.STATUS_INCOMPLETE;
                        spDispatcher.BeginInvoke(UpdatePct, new object[] { _LFTprogressPercentage, "Update: MGMSG_CPLD_REPROGRAM(WRITE_FEABITS) I2C comm Error, Status: 0x" + I2Cstatus.ToString("X") });
                        goto ExitEarly;
                    }
                    System.Threading.Thread.Sleep(100);

                    // POLL for FEA bits update STATUS
                    OpcodeBytes = new byte[APTcmdLen + 16] { 0xA9, 0x00, READ_CPLD_PGM_STATUS, 0x00, (byte)TargetSlaveAddr, 0x1, (byte)(line_count & 0xFF), (byte)(line_count >> 8), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // MGMSG_READ_CPLD_PGM_STATUS
                    OpcodeByteLen = (uint)OpcodeBytes.Length;
                    DATAtransferBufferLen = 6; // READ 6 APT bytes (for single Status byte at Param2)
                    b_I2CReadDir = true;       // reading STATUS (APT format)
                    I2Cstatus = I2Cxfer.I2CTransfer(MasterMUXChan, SlaveMUXChan, TargetSlaveAddr, b_I2CReadDir, PageReadWriteLen, OpcodeBytes, OpcodeByteLen, DataBytes, DATAtransferBufferLen, out TotalDataPlusOpcodesBytesTransfered);
                    // get the status byte...
                    MCU_CPLD_status = DataBytes[3];
                    if (MCU_CPLD_status != 0)
                    {
                        LFTworkerThreadStatus = THORDAQ_STATUS.STATUS_INCOMPLETE;
                        spDispatcher.BeginInvoke(UpdatePct, new object[] { _LFTprogressPercentage, "Update FLASH ERROR: WRITE_FEABITS Status: 0x" + MCU_CPLD_status.ToString("X") });
                        System.Threading.Thread.Sleep(2000);
                        goto ExitEarly;
                    }


                    ///////////////////////////////////////////////////////////////////////////////////////
                    // WRITE_USERCODE
                    currentLineIndex++;

                    dProgressPercent += dPctIncreasePerLine;
                    _LFTprogressPercentage = (int)Math.Round(dProgressPercent); UpdatePct = new UpdateProgressDelegate(LFT_UpdateCPLDProgressText);
                    spDispatcher.BeginInvoke(UpdatePct, new object[] { _LFTprogressPercentage, "CPLD Write USERCODE..." });
                    iExpectedLines = 4; // line usually found on 2nd "do" iteration
                    do
                    {
                        new_lines[0] = JEDfile.ReadLine();
                    } while (!new_lines[0].StartsWith("U") && --iExpectedLines > 0);
                    if( iExpectedLines < 0)
                    {
                        LFTworkerThreadStatus = THORDAQ_STATUS.STATUS_INCOMPLETE;
                        spDispatcher.BeginInvoke(UpdatePct, new object[] { _LFTprogressPercentage, "Update FLASH ERROR: USERCODE not found" });
                        System.Threading.Thread.Sleep(2000);
                        goto ExitEarly;
                    }
                    // We found USERCODE line; expect it to be in ASCII form

                    OpcodeBytes = new byte[APTcmdLen + 16] { 0xA8, 0x00, WRITE_USERCODE, (byte)(currentLineIndex & 0xFF), (byte)(currentLineIndex >> 8), 0x1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // MGMSG_CPLD_REPROGRAM
                    // convert line string to binary
                    k = 2; // i.e. skip the "UA"
                    for (j = 0; j < 4; j++)
                    {
                        temp = new_lines[0].Substring(k++, 1);
                        DataBytes[j] = Convert.ToByte(temp, 16);
                        OpcodeBytes[6 + j] = DataBytes[j]; // copy to I2C data (OpCode) buffer
                    }
                    System.Threading.Thread.Sleep(100);

                    // WRITE the 2(two) FEA bits to MCU
                    OpcodeByteLen = (uint)OpcodeBytes.Length;
                    DATAtransferBufferLen = 0; // no READ data
                    b_I2CReadDir = false;       // WRITE 16-bytes of FLASH program data

                    I2Cstatus = I2Cxfer.I2CTransfer(MasterMUXChan, SlaveMUXChan, TargetSlaveAddr, b_I2CReadDir, PageReadWriteLen, OpcodeBytes, OpcodeByteLen, DataBytes, DATAtransferBufferLen, out TotalDataPlusOpcodesBytesTransfered);
                    if (I2Cstatus != THORDAQ_STATUS.STATUS_SUCCESSFUL)
                    {
                        LFTworkerThreadStatus = THORDAQ_STATUS.STATUS_INCOMPLETE;
                        spDispatcher.BeginInvoke(UpdatePct, new object[] { _LFTprogressPercentage, "Update: MGMSG_CPLD_REPROGRAM(WRITE_FEABITS) I2C comm Error, Status: 0x" + I2Cstatus.ToString("X") });
                        goto ExitEarly;
                    }
                    System.Threading.Thread.Sleep(100);

                    // POLL for FEA bits update STATUS
                    OpcodeBytes = new byte[APTcmdLen + 16] { 0xA9, 0x00, READ_CPLD_PGM_STATUS, 0x00, (byte)TargetSlaveAddr, 0x1, (byte)(line_count & 0xFF), (byte)(line_count >> 8), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // MGMSG_READ_CPLD_PGM_STATUS
                    OpcodeByteLen = (uint)OpcodeBytes.Length;
                    DATAtransferBufferLen = 6; // READ 6 APT bytes (for single Status byte at Param2)
                    b_I2CReadDir = true;       // reading STATUS (APT format)
                    I2Cstatus = I2Cxfer.I2CTransfer(MasterMUXChan, SlaveMUXChan, TargetSlaveAddr, b_I2CReadDir, PageReadWriteLen, OpcodeBytes, OpcodeByteLen, DataBytes, DATAtransferBufferLen, out TotalDataPlusOpcodesBytesTransfered);
                    // get the status byte...
                    MCU_CPLD_status = DataBytes[3];
                    if (MCU_CPLD_status != 0)
                    {
                        LFTworkerThreadStatus = THORDAQ_STATUS.STATUS_INCOMPLETE;
                        spDispatcher.BeginInvoke(UpdatePct, new object[] { _LFTprogressPercentage, "Update FLASH ERROR: WRITE_FEABITS Status: 0x" + MCU_CPLD_status.ToString("X") });
                        System.Threading.Thread.Sleep(2000);
                        goto ExitEarly;
                    }




                    // After programming done (e.g. WRITE_USERCODE)
                    // the MCU firmware sends to CPLD 0x5E (program done), then 0x26 00 00 (disable config interface),
                    // then 0xFF FF FF FF (NoOp / Device Wakeup), the 0x79 (refresh)

                    // SUCCESS!
                    LFTworkerThreadStatus = THORDAQ_STATUS.STATUS_SUCCESSFUL;
                    JEDfile.Close();
                }
                catch (Exception ex)
                {
                    sFinalStatus = sFinalStatus + ": " + ex.Message;
                    LFTworkerThreadStatus = THORDAQ_STATUS.STATUS_INCOMPLETE;
                    dbg1++;
                }
                ExitEarly:
 
                if (LFTworkerThreadStatus == THORDAQ_STATUS.STATUS_SUCCESSFUL)
                {
                    sFinalStatus = "SUCCESS " + Environment.NewLine + "POWER CYCLE Now!";
                    _LFTprogressPercentage = 100;
                }
                //invoke the dispatcher and pass the percentage
                UpdatePct = new UpdateProgressDelegate(LFT_UpdateCPLDProgressText);
                spDispatcher.BeginInvoke(UpdatePct, new object[] { _LFTprogressPercentage, "CPLD Program Update " + sFinalStatus });

                System.Threading.Thread.Sleep(1000);
            }; // End of DoWork DELEGATE
            splashWkr.RunWorkerCompleted += delegate (object sender, RunWorkerCompletedEventArgs e)
            {
                _LFTcpldSplash.Close();
                // App inherits from Application, and has a Window property called MainWindow
                // and a List<Window> property called OpenWindows.
                Application.Current.MainWindow.Activate();
                _LFTprogressPercentage = 0;
            };


            return;
        }

        string _LFT_JAfirmwareVer = string.Empty;

        // format the channel data according to MCU code
        class SynthInputConfig
        {
            public bool auto_tracking;
            public ushort neg_thresh_dac;
            public ushort pos_thresh_dac;
            public ushort adc_pulse_amplitude;
            public UInt32 frequency;
  //          public Byte synth;
        }

        // SET Config/Data in LFT mezz. card
        public List<string> APIsetLFT_JAconfig(List<string> argumentsList)
        {
            THORDAQ_STATUS status;
            const uint MasterMUXChan = 0x1;
            const uint SlaveMUXChan = 0xff;
            const uint TargetSlaveAddr = 0x67; // CPLD address on DAC board
            XilinxI2CXfer I2Cxfer = new XilinxI2CXfer();


            List<string> retStrings = new List<string> { };
            string sFuncStatus = "OK";
            bool bI2C_SlaveRead = false;
            int PageSize = 16;
            byte[] OpcodeBytes; // set to array according to PARAM
            byte[] DataBytes = new byte[1];
            uint I2CtransferLen;
            SynthInputConfig lf0 = new SynthInputConfig(), lf1 = new SynthInputConfig(); // setup defaults
            lf0.neg_thresh_dac = 2000; lf0.pos_thresh_dac = 2015; // DAC counts
            lf1.neg_thresh_dac = 2000; lf1.pos_thresh_dac = 2015;
            byte AutoTrack = 0x3; // default both LF0, LF1 to "auto-tracking"

            //            -CSIS { D | 1 | 2}
            //            Sets MGMSG_CLK_SET_INPUT_SELECT to Disable, LF0, LF1

            for (int argNum = 1; argNum < argumentsList.Count; argNum++)
            {
                // manipulate default commands based on args (if any)
                switch (argumentsList[argNum])
                {

                    //-CSIT { -LF0 NegThresh PosThresh | -LF1 NegThresh PosThresh }
                    case "-CSIT":
                        {
                            argNum++;
                            switch (argumentsList[argNum++]) 
                            {
                                //-CSIT { -LFn - a | < -m < NegThresh PosThresh >}
                                //                        Sets auto_tracking(def.), if manual set thresholds
                                case "-LF0":
                                    if (argumentsList[argNum++] == "-m") // def. is "-a"
                                    {
                                        ushort.TryParse(argumentsList[argNum++], out lf0.neg_thresh_dac);
                                        ushort.TryParse(argumentsList[argNum], out lf0.pos_thresh_dac);
                                        AutoTrack &= 0xFE; // bit 0
                                    }
                                    else
                                    {
                                        AutoTrack |= 0x01;
                                    }
                                    break;
                                case "-LF1":
                                    if (argumentsList[argNum++] == "-m") // def. is "-a"
                                    {
                                        ushort.TryParse(argumentsList[argNum++], out lf1.neg_thresh_dac);
                                        ushort.TryParse(argumentsList[argNum], out lf1.pos_thresh_dac);
                                        AutoTrack &= 0xFD; // bit 1
                                    }
                                    else
                                    {
                                        AutoTrack |= 0x02;
                                    }

                                    break;
                                default:   // LF1
                                    break;
                            }
                            argNum++;
                        }
                        break;
                    case "-CSIS":
                        argNum++; // get setting { D | 1 | 2}
                        switch (argumentsList[argNum])
                        {
                            case "d":
                            case "D":
                                MSGOpcodesWriteOnly[0][6] = 0;
                                break;
                            case "1":
                                MSGOpcodesWriteOnly[0][6] = 1;
                                break;
                            case "2":
                                MSGOpcodesWriteOnly[0][6] = 2;
                                break;
                        }
                        break;
                    default:
                        break;
                }
            }

            // set defaults
            // for Thresholds...
            MSGOpcodesWriteOnly[1][6] = AutoTrack;
            MSGOpcodesWriteOnly[1] = new byte[MAX_I2C_SLAVE_WRITE_Len] { 0x01, 0x46, 0x00, 0x00, 0x67, 0x1, AutoTrack, 0x0,
                            (byte)(lf0.neg_thresh_dac & 0xFF), (byte)(lf0.neg_thresh_dac>>8), (byte)(lf0.pos_thresh_dac & 0xFF), (byte)(lf0.pos_thresh_dac>>8),
                            (byte)(lf1.neg_thresh_dac & 0xFF), (byte)(lf1.neg_thresh_dac>>8), (byte)(lf1.pos_thresh_dac & 0xFF), (byte)(lf1.pos_thresh_dac>>8),
                            0, 0, 0, 0, 0, 0 };

            string[] SETParams = new string[] { "MGMSG_CLK_SET_INPUT_SELECT", "MGMSG_CLK_SET_INPUT_THRESH" };
            for (int i = 0; i < SETParams.Length; i++)  // for each APT SET command
            {
                System.Threading.Thread.Sleep(1);
                OpcodeBytes = MSGOpcodesWriteOnly[i];
                status = I2Cxfer.I2CTransfer(MasterMUXChan, SlaveMUXChan, TargetSlaveAddr, bI2C_SlaveRead, PageSize, OpcodeBytes, OpCodeLen, DataBytes, 0, out I2CtransferLen);

                // if this is 2.x FW, we'll have a good status and a version string...
                if ((THORDAQ_STATUS)status != THORDAQ_STATUS.STATUS_SUCCESSFUL)
                {
                    retStrings.Add("PARAM: " + SETParams[i] + " status 0x" + status.ToString("X") + "  Obsolete F/W version on card??");
                    sFuncStatus = "ERROR";
                }
                else
                {
                    // decode the data
                }

            }
                // done - return
                retStrings.Add(sFuncStatus);
            return retStrings;
        }

        // GET Config/Data from LFT mezz. card
        public List<string> APIgetLFT_JAstatus(List<string> argumentsList)
        {
            THORDAQ_STATUS status;
            const uint MasterMUXChan = 0x1;
            const uint SlaveMUXChan = 0xff;
            const uint TargetSlaveAddr = 0x67; // CPLD address on DAC board
            XilinxI2CXfer I2Cxfer = new XilinxI2CXfer();

            string sFuncStatus = "OK";
            List<string> retStrings = new List<string> { };
            bool bI2C_SlaveRead;
            int PageSize = 16;
            byte[] OpcodeBytes; // set to array according to PARAM
            byte[] DataBytes;

            // RETURN data buffers
            const int TRIGcardFW_VER_LEN = 18;
            const int TRIGcardStatus_LEN = 23;
            const int TRIGcardThresh_LEN = 8;
            byte[] FWverDataBytes = new byte[TRIGcardFW_VER_LEN];
            byte[] StatusDataBytes = new byte[TRIGcardStatus_LEN];
            byte[] ThresholdDataBytes = new byte[TRIGcardThresh_LEN];
            bI2C_SlaveRead = true;
            uint I2CtransferLen;
            ASCIIEncoding ascii = new ASCIIEncoding();  // for converting EEPROM bytes to ASCII
            string[] REQParams = new string[] { "MGMSG_HW_REQ_INFO", "MGMSG_CLK_REQ_INPUT_THRESH", "MGMSG_CLK_REQ_INPUT_STATUS" }; /*, "MGMSG_CLK_REQ_INPUT_SELECT", "MGMSG_CLK_REQ_OUTPUT_CONFIG" };*/

            string sLFT_JAfw, sfwVer, sBuf;
            SynthInputConfig lf0 = new SynthInputConfig();
            SynthInputConfig lf1 = new SynthInputConfig();

            for (int i = 0; i < REQParams.Length; i++)
            {
                System.Threading.Thread.Sleep(1);
                OpcodeBytes = MSGOpcodesRead[i];
                switch( i)  // how many MCU data bytes should we read?
                {
                    case 0:
                        DataBytes = FWverDataBytes;
                    break;
                    case 1:
                        DataBytes = ThresholdDataBytes;
                        break;
                    case 2:
                        DataBytes = StatusDataBytes;
                        break;
                    default:
                        DataBytes = FWverDataBytes;
                        break;
                }
                status = I2Cxfer.I2CTransfer(MasterMUXChan, SlaveMUXChan, TargetSlaveAddr, bI2C_SlaveRead, PageSize, OpcodeBytes, OpCodeLen, DataBytes, (uint)DataBytes.Length, out I2CtransferLen);

                // if this is 2.x FW, we'll have a good status and a version string...
                if ((THORDAQ_STATUS)status != THORDAQ_STATUS.STATUS_SUCCESSFUL)
                {
                    retStrings.Add("PARAM: " + REQParams[i] + " status 0x" + status.ToString("X") + "  Obsolete F/W version on card??");
                    sFuncStatus = "ERROR";
                }
                else
                {
                    // decode the data
                    switch(REQParams[i])
                    {
                        case "MGMSG_HW_REQ_INFO":
                            sLFT_JAfw = ascii.GetString(DataBytes, 4, 8);
                            sfwVer = DataBytes[16].ToString() + "." + DataBytes[15].ToString() + "." + DataBytes[14].ToString();
                            retStrings.Add(REQParams[i] + ": CPLD USERCODE: " + Convert.ToChar(DataBytes[0]) + "." + 
                                Convert.ToChar(DataBytes[1]) + "." + Convert.ToChar(DataBytes[2]) + "." + Convert.ToChar(DataBytes[3]) +
                                " App F/W: " + sLFT_JAfw + " Ver. " + sfwVer);
                            break;
                        case "MGMSG_CLK_REQ_INPUT_THRESH":
                            lf0.neg_thresh_dac = (ushort)(DataBytes[0] + (DataBytes[1] << 8));
                            lf0.pos_thresh_dac = (ushort)(DataBytes[2] + (DataBytes[3] << 8));
                            lf1.neg_thresh_dac = (ushort)(DataBytes[4] + (DataBytes[5] << 8));
                            lf1.pos_thresh_dac = (ushort)(DataBytes[6] + (DataBytes[7] << 8));
                            retStrings.Add(REQParams[i] + ": " + "LF0 Neg Threshold: " + lf0.neg_thresh_dac + " (DAC cnts)");
                            retStrings.Add(REQParams[i] + ": " + "LF0 Pos Threshold: " + lf0.pos_thresh_dac + " (DAC cnts)");
                            retStrings.Add(REQParams[i] + ": " + "LF1 Neg Threshold: " + lf1.neg_thresh_dac + " (DAC cnts)");
                            retStrings.Add(REQParams[i] + ": " + "LF1 Pos Threshold: " + lf1.pos_thresh_dac + " (DAC cnts)");

                            break;
                        case "MGMSG_CLK_REQ_INPUT_STATUS":
                            // parse byte structure:
                            // byte 
                            // 0    - "sticky" LOL register
                            // 1    - live LOL register ?
                            // 2-5  - 4 bytes (LSB 1st) LF0 freq
                            // 6-9  - 4 bytes (LSB 1st) LF1 freq
                            // 10-13 - 4 bytes (LSB 1st) HF0 freq
                            // 14-17 - 4 bytes (LSB 1st) HF1 freq
                            // 18-19 - 2 bytes (LSB 1st) LF0 ADC pulse amplitute
                            // 20-21 - 2 bytes (LSB 1st) LF1 ADC pulse amplitute
                            sBuf = DataBytes[0].ToString("X");
                            // auto_tracking status
                            lf0.auto_tracking = ((DataBytes[22] & 0x01) == 0x01) ? true : false;
                            lf1.auto_tracking = ((DataBytes[22] & 0x02) == 0x02) ? true : false;
                            retStrings.Add(REQParams[i] + ": " + " LF0 autoTrack " + lf0.auto_tracking);
                            retStrings.Add(REQParams[i] + ": " + " LF1 autoTrack " + lf1.auto_tracking);
                            // Si5344 jitter attentuator registers for locking status bits, e.g. LOL, OOF
                            retStrings.Add(REQParams[i] + ": " + " LOL (sticky)  0x" + DataBytes[0].ToString("X"));
                            retStrings.Add(REQParams[i] + ": " + " LOL           0x" + DataBytes[1].ToString("X"));
                            // pulse amplitude detected (ADC cnts)
                            lf0.adc_pulse_amplitude = (ushort)(DataBytes[18] | (DataBytes[19] << 8));
                            retStrings.Add(REQParams[i] + ": " + " LF0 PulseAmp  " + lf0.adc_pulse_amplitude + " (ADC cnts)");
                            lf1.adc_pulse_amplitude = (ushort)(DataBytes[20] | (DataBytes[21] << 8));
                            retStrings.Add(REQParams[i] + ": " + " LF1 PulseAmp  " + lf1.adc_pulse_amplitude + " (ADC cnts)");

                            lf0.frequency = (UInt32)DataBytes[2] | (UInt32)(DataBytes[3] << 8) | (UInt32)(DataBytes[4] << 16) | (UInt32)(DataBytes[5] << 24);
                            retStrings.Add(REQParams[i] + ": " + " LF0 Freq      " + lf0.frequency + " (Hz)");
                            lf1.frequency = (UInt32)DataBytes[6] | (UInt32)(DataBytes[7] << 8) | (UInt32)(DataBytes[8] << 16) | (UInt32)(DataBytes[9] << 24);
                            retStrings.Add(REQParams[i] + ": " + " LF1 Freq      " + lf1.frequency + " (Hz)");

                            break;
                    }
                }
            }

            // done - return
            retStrings.Add(sFuncStatus);
            return retStrings;
        }
    }
}
