using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using ThorLogging;
using ThorSharedTypes;
using System.Diagnostics;

namespace ThorDAQConfigControl.Model
{
    public partial class ThorDAQCommandProvider
    {
        public delegate void UpdateConsoleStatusDel(string input);
        UpdateConsoleStatusDel UpdateConsoleStatus;

        // Set the callback function to display the input
        public void SetUpdateConsoleStatusDelegate(UpdateConsoleStatusDel showResult)
        {
            UpdateConsoleStatus = showResult;
        }

        enum EEPromCMDs
        {
            MB_SER,
            ADC_SER,
            DAC_SER,
            CPLD_VER,
            BBA1_SER,
            BBA2_SER,
            BBA3_SER,
            BBD1_SER,
            INV_CMD,
        };


        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPIProgrammableTrigger")]
        public extern static THORDAQ_STATUS ThorDAQAPIProgrammableTrigger(UInt32 board, SByte Channel, SByte ArmASSERT); // see TDAdvancedTriggering.cpp


        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPIGetDDR3status")]
        public extern static THORDAQ_STATUS ThorDAQAPIGetDDR3status(UInt32 board, IntPtr DDR3statusString, Int32 TotalRecordSize); // DDR3status fields in SharedTypes.cs

        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPIGetBOBstatus")]
        public extern static THORDAQ_STATUS ThorDAQAPIGetBOBstatus(UInt32 board, IntPtr BOBstatusString, Int32 TotalRecordSize); // BOBstatus fields in SharedTypes.cs
        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPISetAIOConfig")]
        public extern static THORDAQ_STATUS ThorDAQAPISetAIOConfig(UInt32 board, IntPtr DIOconfig, Int32 FieldSize); // DIOconfig is single DIO definition - send our field size

        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPIGetDIOConfig")]
        public extern static THORDAQ_STATUS ThorDAQAPIGetDIOConfig(UInt32 board, IntPtr DIOconfig, Int32 RecordsSize); // DIOconfig is entire DLL char array, Max DIOs * FieldSize

        // CHAR config[32][12]  e.g. "DnnXmmMccAgg", where "n" is 0-31 (BNC label D0 through "D31", "X" is Input/Output direction "I" or "O",
        // "mm" is Index of Output source, "cc" is FPGA/CPLD MUX code, "gg" is FPGA GPIO MUX index (0-4 valid)
        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPISetDIOConfig")]
        public extern static THORDAQ_STATUS ThorDAQAPISetDIOConfig(UInt32 board, IntPtr DIOconfig, Int32 FieldSize); // DIOconfig is single DIO definition - send our field size

        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPIGetDIO")]  // gets Input or last Output setting, if DIOspec is Output
        public extern static THORDAQ_STATUS ThorDAQAPIGetDIO(UInt32 board, IntPtr DIOconfig, UInt32 FieldSize, UInt32 NumDIOs); // DIOspec (value in AUX field), DIOspec field size, Number of DIOs

        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPISetDO")]
        public extern static THORDAQ_STATUS ThorDAQAPISetDO(UInt32 board, IntPtr DIOconfigs, UInt32 FieldSize, UInt32 NumDIOs); // DIOspec (value in AUX field), DIOspec field size, Number of DIOs

        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPIGetAI")]
        public extern static uint ThorDAQAPIGetAI(UInt32 board, Int32 BNCindex, bool bVolts, ref double VoltsOrADCcounts);
        // boardNum,                  // Board to target
        // BNCindex,                  // BNC index (e.g. 6 for "AI6) on BBox for FPGA reg (legacy) or MAX127 channel
        // BiPolar,                   // True (1) if Bipolar, 0 for unipolar
        // ADCcounts                  // raw 12-bit count received from MAX127 chip

        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPIReleaseBoard")]
        public extern static uint ThorDAQAPIReleaseBoard(UInt32 board);

        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPIProgressiveScan")]
        public extern static uint ThorDAQAPIProgressiveScan(UInt32 board, bool bProgressiveScan);

        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPIXI2CReadWrite")]
        public extern static uint ThorDAQAPIXI2CReadWrite(UInt32 board,
            bool bI2C_ReadDir,
            UInt32 MasterMUXAddr, UInt32 MasterMUXChan,
            Int32 SlaveMUXAddr, UInt32 SlaveMUXChan,    // Slave Addr -1 means target on Master MUX bus
            UInt32 TargetAddr, UInt32 I2CbusHz, Int32 PageSize,
            IntPtr OpcodeBuffer, ref UInt32 OpcodeLen,
            IntPtr DataBuffer, ref UInt32 DataLen); // DataLen returns total Opcode+Data bytes written


        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPIFlashI2CSlave")]
        public extern static uint ThorDAQAPIFlashI2CSlave(UInt32 board, UInt32 MasterChan, Int32 SlaveChan, UInt32 DevAddr, UInt32 SlaveDevCmdByte, IntPtr Buffer, ref UInt32 BufByteLen);

        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPIBreakOutBoxLED")]
        public extern static uint ThorDAQAPIBreakOutBoxLED(UInt32 board, Int32 LEDenum, Byte State);  // 0 = off, 1 = on, 2 = blink

        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPIFPGAregisterWrite")]
        public extern static uint ThorDAQAPIFPGAregisterWrite(UInt32 board,
            IntPtr pRegisterFieldName,
            Int32 FldNameSize, // in bytes
            IntPtr Value);     // DLL/driver Register/Field (unManagedMem) value to write

        [DllImport(".\\Modules_Native\\thorDAQ.dll", EntryPoint = "ThorDAQAPIFPGAregisterRead")]
        public extern static uint ThorDAQAPIFPGAregisterRead(UInt32 board,
            IntPtr pRegisterFieldName,
            Int32 FldNameSize, // in bytes
            IntPtr pValue);    // DLL/driver returned value of REGISTER or FIELD    
                               //////////////////////////////////
        ///

        public tdDFT.DiscreteFourierTrans dft;

        public static TDcontroller[] ThorDAQAdapters = new TDcontroller[4];
        public static UInt16 MasterBoardIndex = 0;
        public const uint TDMasterI2CMUXAddr = 0x71;
        public const int TDSlaveI2CMUXAddr = 0x70;


        public string SettingPath;
        public bool _acquisitionActiveFlag = true;


        // first arg in register/field name to read
        // arg1 - Name
        // arg2 - Value
        public void SetFPGAreg(List<String> argumentsList)
        {
            string RegFldName = argumentsList[1];
            // value - may be in HEX
            UInt64 Value;
            if (argumentsList[2].Contains("0x"))
            {
                Value = ulong.Parse(argumentsList[2].Replace("0x", string.Empty), NumberStyles.HexNumber);
            }
            else
            {
                Value = ulong.Parse(argumentsList[2]);
            }
            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write(RegFldName, Value);
        }
        // first arg in register/field name to read
        // arg1 - Name
        public void GetFPGAreg(List<String> argumentsList)
        {
            string RegFldName = argumentsList[1];
            UInt64 Value = 0;

            bool bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read(RegFldName, ref Value);
            if (bStatus != true)
                UpdateConsoleStatus("Invalid Register/Field: " + RegFldName);
            else
                UpdateConsoleStatus(string.Format("{0}: {1} (0x{2})", RegFldName, Value, Value.ToString("X")));

        }

        // query the API DLL for all Register and Field definitions for the hardware board
        public void GetAllFPGARegisters(List<String> argumentsList)
        {
            List<string> resultList = new List<string>();
            REG_FIELD_DESC RegFldDesc = new REG_FIELD_DESC();
            uint uiStatus;
            uint TotalVariableCount = 0;
            string RegFldName = "";
            bool bCreateFile = false;
            string fileSpec = "c:\\temp\\FPGAregs.txt";
            var fileFormatStrings = new List<string>();

            // optional command line args?

            if (argumentsList.Count > 1)
            {
                if (argumentsList[1] == "-f")
                {
                    // save the Register/Field list to file
                    bCreateFile = true;
                    if (argumentsList.Count > 2)
                        fileSpec = argumentsList[2];
                }
            }
            for (int r = 0; r < 256; r++) // arbitrarily high upper bound
            {
                // first get the register
                uiStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Query(r, -1, ref RegFldName, 48, ref RegFldDesc);
                if (uiStatus != 0)
                {
                    break; // done
                }
                // we have a REGISTER
                TotalVariableCount++;
                string str = string.Format("Reg: {0}({0:x} h) Name: {1,-34} --> BAR: {2} BARoffset: 0x{3} Register Byte Len: {4}, WO:{5} RO:{6}",
                                                    r, RegFldName, RegFldDesc.BAR, RegFldDesc.BARoffset.ToString("X3"), RegFldDesc.RegisterByteSize,
                                                    RegFldDesc.bWriteOnly.ToString(), RegFldDesc.bReadOnly.ToString());
                //UpdateConsoleStatus(str, 1);
                UpdateConsoleStatus(str);
                if (bCreateFile) fileFormatStrings.Add(str);

                for (int f = 0; f < 64; f++)
                {
                    uiStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Query(r, f, ref RegFldName, 48, ref RegFldDesc);
                    if (uiStatus != 0)
                    {
                        break; // next Register
                    }
                    // we have a FIELD
                    TotalVariableCount++;
                    str = string.Format("  Fld: {0} Name: {1,-32} --> Bit Field Offset: {2} Bit Field Len: {3}", f, RegFldName, RegFldDesc.BitFieldOffset, RegFldDesc.BitFieldLen);                    
                    UpdateConsoleStatus(str);
                    //UpdateConsoleStatus(str, 1);
                    if (bCreateFile) fileFormatStrings.Add(str);
                }
            }
            UpdateConsoleStatus("Total Reg/Field variables: " + TotalVariableCount);
            if (bCreateFile == true)
            {
                File.WriteAllLines(fileSpec, fileFormatStrings);
                UpdateConsoleStatus("Register/Field descriptions written to file: " + fileSpec);
            }
        }

        // arg1  RegisterIndex
        // arg2  FieldIndex (or -1 if RegIndex only)
        // arg3  
        public void ReadBoardMemoryData(List<String> argumentsList)
        {
            List<string> resultList = new List<string>();
            IntPtr buffer = IntPtr.Zero;
            try
            {
                UInt16 value = (UInt16)Convert.ToInt32(argumentsList[1]);
                {
                    UInt32 barNum = value; // get bar number
                    int registerAddress = (int)new System.ComponentModel.Int32Converter().ConvertFromString(argumentsList[2]);
                    int transferDataLength = (int)new System.ComponentModel.Int32Converter().ConvertFromString(argumentsList[3]);
                    if (buffer == IntPtr.Zero)
                    {
                        Win32.AllocateBuffer(ref buffer, (uint)transferDataLength);
                    }
                    string sStatus = ThorDAQAdapters[MasterBoardIndex].BAR.Read(barNum, (ulong)registerAddress, buffer, (ulong)transferDataLength);
                    if (sStatus == "OK")
                    {
                        Byte[] ManagedMem = new Byte[transferDataLength];
                        Marshal.Copy(buffer, ManagedMem, 0, transferDataLength);
                        foreach (string Str in format16ByteLinesOnConsole((ulong)registerAddress, ManagedMem, (uint)transferDataLength))
                        {
                            UpdateConsoleStatus(Str);
                        }
                    }
                    else
                        UpdateConsoleStatus(sStatus);

                    if (buffer != IntPtr.Zero)
                        Win32.FreeBuffer(buffer);
                }
            }
            catch (Exception e)
            {
                if (buffer != IntPtr.Zero)
                {
                    Win32.FreeBuffer(buffer);
                }
                UpdateConsoleStatus("Readmem Error: " + e.Message);
            }
        }

        // the BBoxLEDs list can be NULL; if not, it designates which LEDs to turn ON or OFF with Scan
        public static List<string> GlobalScan(List<String> argumentsList, List<Enum> BBoxLEDs)
        {
            List<string> ReturnStrings = new List<string> { };
            bool bRun = true;
            bool bTriggerNI1071 = false;
            int argNum;
            int PixelsInLine = 0, TotalLines = 0;
            string[] FrameDimension;
            UInt64 ScanFrameCount = 0xFFFF;
            bool bHardwareTrigger = false; // default is software assertion of "scan" mode

            for (argNum = 1; argNum < argumentsList.Count; argNum++)
            {
                switch (argumentsList[argNum])
                {
                    case "-t": // use external hardware trigger (fixed)?
                        bHardwareTrigger = true;
                        break;
                    case "-T":
                        bTriggerNI1071 = true;
                        break;
                    case "-n":
                        UInt64.TryParse(argumentsList[argNum + 1], out ScanFrameCount);
                        argNum++; // next flag
                        break;

                    case "-d":  // get frame dimensions
                        FrameDimension = argumentsList[argNum + 1].Split('x');
                        Int32.TryParse(FrameDimension[0], out PixelsInLine);
                        Int32.TryParse(FrameDimension[1], out TotalLines);
                        argNum++; // next flag

                        // go ahead and set registers because there is no "default" frame dimensions required for ScanControl
                        ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("GlobalImageHSIZE", (ulong)PixelsInLine - 1);
                        ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("GlobalImageAcqHSIZE", (ulong)PixelsInLine - 1); // assume single Plane!
                        ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("GlobalImageVSIZE", (ulong)TotalLines - 1);

                        break;

                    case "-s":  // stop waveform playback
                        bRun = false;
                        break;
                    case "-r":  // start waveform playback
                        bRun = true;
                        break;

                }
            }
            // check args
            // if WxH not already set in FPGA, and not given in command arg, fatal exit
            ulong RegValue = 0;
            bool bStatus;

            // hardware trigger config?
            if(bHardwareTrigger)
            {
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ACQ_HW_TRIGGER_SEL1", 0);
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ACQ_HW_TRIGGER_SEL2", 0);
                ReturnStrings.Add(string.Format("ACQ_HW_TRIGGER_SEL1: {0}", RegValue.ToString()));
                ReturnStrings.Add(string.Format("ACQ_HW_TRIGGER_SEL2: {0}", RegValue.ToString()));

                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("imaging_trig_logic_lut_addr", 0);
                ReturnStrings.Add(string.Format("imaging_trig_logic_lut_addr: {0}", RegValue.ToString()));
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("imaging_trig_logic_lut_data", 0x3);  // trig active HIGH, RESET rising edge trig
                ReturnStrings.Add(string.Format("imaging_trig_logic_lut_data: {0}", RegValue.ToString()));
                // toggle the write strobe
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("imaging_trig_logic_lut_we", 0);
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("imaging_trig_logic_lut_we", 1);
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("imaging_trig_logic_lut_we", 0);
            }
            // count of requested frames (0xFFFF is "infinite")
            bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ScanningFrameCount", ScanFrameCount); // 1 = on 1st LineSync after Start
            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ScanningFrameCount", ref RegValue);
            ReturnStrings.Add(string.Format("ScanningFrameCount: {0}", RegValue.ToString()));


            if (bRun == false) // STOP
            {
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("GIGCR0_STOP_RUN", 0); // 0 = stop
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("GIGCR0_STOP_RUN", ref RegValue);
                ReturnStrings.Add(string.Format("GIGCR0_STOP_RUN: {0}", RegValue.ToString()));

                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("GIGCR0_LED2", 0); // 1 = on
                if (BBoxLEDs != null)
                {
                    foreach (Enum LEDenum in BBoxLEDs)
                    {
                        object enumObject = LEDenum;
                        int intValue = (int)enumObject;
                        uint uiStatus;
                        {
                            uiStatus = ThorDAQAPIBreakOutBoxLED(MasterBoardIndex, intValue, 0); // LED OFF if activated
                        }
                    }
                }
            }
            else //  START (RUN)
            {
                if (PixelsInLine == 0)
                {
                    // user wants existing HSIZE/VSIZE
                    ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("GlobalImageHSIZE", ref RegValue);
                    if (RegValue == 0) // undefined dimensions - exit
                    {
                        ReturnStrings.Add("FAIL: GlobalImageHSIZE/VSIZE not specified");
                        return ReturnStrings;
                    }
                    else
                    {
                        PixelsInLine = (int)RegValue + 1;
                        ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("GlobalImageVSIZE", ref RegValue);
                        TotalLines = (int)RegValue + 1;
                    }
                }

//                bool bMUXset = SetThorImageLS_BOBConfig(); // (Legacy or 3U) if false, DIO config may be incorrect

                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("GlobalDBB1muxReg", ref RegValue);
                ReturnStrings.Add(string.Format("GlobalDBB1muxReg: 0x{0}", RegValue.ToString("X16")));

                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("GlobalABBXmuxReg", ref RegValue);
                ReturnStrings.Add(string.Format("GlobalABBXmuxReg: 0x{0}", RegValue.ToString("X16")));


                // Image dimensions (must be set even when NOT acquiring the S2MM DDR3 image DMA
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("GlobalImageHSIZE", (ulong)(PixelsInLine - 1)); // pixels-1
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("GlobalImageAcqHSIZE", (ulong)PixelsInLine - 1); // assume single Plane!
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("GlobalImageHSIZE", ref RegValue);
                ReturnStrings.Add(string.Format("GlobalImageHSIZE: {0}", RegValue.ToString()));

                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("GlobalImageVSIZE", (ulong)(TotalLines - 1)); // lines-1
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("GlobalImageVSIZE", ref RegValue);
                ReturnStrings.Add(string.Format("GlobalImageVSIZE: {0}", RegValue.ToString()));

                // Trigger Mode (0 is "software" assertion, 1 means some software "arms" trigger, hardware signal for assertion)
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ACQ_HW_TRIGGER_MODE", 0); 
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ACQ_HW_TRIGGER_MODE", ref RegValue);
                ReturnStrings.Add(string.Format("ACQ_HW_TRIGGER_MODE: {0}", RegValue.ToString()));

                // control LEDs
                ///ulong AOchannelMaskShift = channelPlaybackMask; // use the static global configured in DACwaveform
                ///
                if (BBoxLEDs != null)
                {
                    foreach (Enum LEDenum in BBoxLEDs)
                    {
                        object enumObject = LEDenum;
                        int intValue = (int)enumObject;
                        uint uiStatus;
                        {
                            uiStatus = ThorDAQAPIBreakOutBoxLED(MasterBoardIndex, intValue, 1); // LED ON if activated
                        }
                    }
                }
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("GIGCR0_LED2", 1); // 1 = on
                if (bTriggerNI1071)
                {
                    // Configure the PFI0 trigger (e.g. D31), strictly hardware based
                    // Labview responds on rising edge, starting a sine waveform for our PMT channel
  //                  MTAPISetDIOConfig(new List<string> { "31", "o", "31", "48" });
  //                  MTAPISetDO(new List<string> { "D31", "1" });
                }
                // START the "image scan", which may or may not include both useful DAC wave output and ADC input
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("GIGCR0_STOP_RUN", 1); // 1 = start
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("GIGCR0_STOP_RUN", ref RegValue);
                ReturnStrings.Add(string.Format("GIGCR0_STOP_RUN: {0}", RegValue.ToString()));
                if (bTriggerNI1071)
                {
  //                  MTAPISetDO(new List<string> { "D31", "0" });
                }

            }  // end of RUN
            ReturnStrings.Add("SUCCESS");
            return ReturnStrings;
        }

        string FormatSerStr(string serStr)
        {
            string serStrOut = string.Empty;

            //            UpdateConsoleStatus1("Length = " + serStr.Length);

            if (serStr.Length >= (int)SER_NUM_SZ)
            {
                serStrOut = serStr.Substring(0, (int)SER_NUM_SZ) + "-";
            }

            if (serStr.Length >= (int)(SER_NUM_SZ + BATCH_NUM_SZ))
            {
                serStrOut = serStrOut + serStr.Substring((int)SER_NUM_SZ, (int)BATCH_NUM_SZ) + "-";
            }

            if (serStr.Length >= (int)(SER_NUM_SZ + BATCH_NUM_SZ + PART_NUM_SZ))
            {
                serStrOut = serStrOut + serStr.Substring((int)(SER_NUM_SZ + BATCH_NUM_SZ), (int)PART_NUM_SZ) + "-";
            }

            if (serStr.Length >= (int)(SER_NUM_SZ + BATCH_NUM_SZ + PART_NUM_SZ + DATE_SZ))
            {
                serStrOut = serStrOut + serStr.Substring((int)(SER_NUM_SZ + BATCH_NUM_SZ + PART_NUM_SZ), (int)DATE_SZ);
            }

            return serStrOut;
        }

        // API function to replace legacy I2C function
        // We define these 5-char fields in the EEPROM.txt file (for production reference)
        // 0 "TD   "
        // 1 "ADC  "
        // 2 "DBB1 "
        // OMIT CPLD on DAC because it makes no sense! (Change Legacy operation)
        // 3 "DAC  "
        // 4 "ABB1 "
        // 5 "ABB2 "
        // 6 "ABB3 "
        // 7 "LFT  "

        public string APIReadEEprom(List<String> argumentsList)
        {
            // add the "-r" arg for Read
            argumentsList.Add("-r");
            return APIReadWriteEEprom(argumentsList);
        }
        public void APIWriteEEprom(List<String> argumentsList)
        {
            // add the "-w" arg for Write
            argumentsList.Add("-w");
            argumentsList.Add("-p"); // EEPROM "page" len (def. 16)
            argumentsList.Add("16");
            APIReadWriteEEprom(argumentsList);
        }
        static string filespec = @"C:\temp\ThorDAQeeproms.txt";
        string HELP_APIReadEEprom_MSSAGE = @"APIReadEEprom: <-f> 
Options:
  -c  Specific TD component index, e.g. 0 for TDboard (def. all components)
  -f  Copies EEPROM values to " + filespec
;
        string APIReadWriteEEprom(List<String> argumentsList)
        {
            THORDAQ_STATUS status = THORDAQ_STATUS.STATUS_SUCCESSFUL;
            uint TotalDataPlusOpcodesByteCount;
            var fileFormatStrings = new List<string>(); // human-readable format
            uint MasterMUXAddr, MasterMUXChan, SlaveMUXChan, TargetSlaveAddr, OpcodeByteLen, TransferBufferLen;
            int SlaveMUXAddr, PageReadWriteLen = 16;
            byte[] OpcodeBytes = new byte[] { 0xC0, 0x0, 0x0, 0x0 }; // max opcode count by ThorDAQ definition
            byte[] DataBytes = new byte[TOT_SERIAL_SZ]; // expected EEPROM data size by ThorDAQ definition
            IntPtr UnManagedOpCodeBuffer = Marshal.AllocHGlobal(OpcodeBytes.Length);
            IntPtr UnManagedDataBuffer = Marshal.AllocHGlobal(DataBytes.Length);
            ASCIIEncoding ascii = new ASCIIEncoding();  // for converting EEPROM bytes to ASCII
            bool bI2C_SlaveRead = true, bCreateFile = false;
            int TDcomponentIndex = -1; // default, all
            ushort TDboardIndex = MasterBoardIndex; // which PCIe board? (def. global selector)
            UInt32 I2CbusHz = 400; // EEPROMs run at higher speed

            string[] EEPROMcontentsToWrite = { "TD   ", "ADC  ", "DBB1 ", "DAC  ", "ABB1 ", "ABB2 ", "ABB3 ", "LFT  " };
            string sFormattedBuf = " "; // return string

            for (int argNum = 1; argNum < argumentsList.Count; argNum++)
            {
                switch (argumentsList[argNum])
                {
                    case "-b": // [option to add] next arg PCIe Board index (def. 0)
                        TDboardIndex = (ushort)new System.ComponentModel.Int16Converter().ConvertFromString(argumentsList[++argNum]);
                        break;
                    case "-p": // [option to add] next arg as pagelen of hardware EEPROM
                        PageReadWriteLen = (Int32)new System.ComponentModel.Int32Converter().ConvertFromString(argumentsList[++argNum]);
                        break;
                    case "-c": // [option to add] single component (def. entire ThorDAQ system)
                        TDcomponentIndex = (Int32)new System.ComponentModel.Int32Converter().ConvertFromString(argumentsList[++argNum]);
                        break;

                    case "-f": // [option to add] next arg as filename
                        bCreateFile = true;
                        break;
                    case "-w": // 
                        bI2C_SlaveRead = false;
                        break;
                    default:
                        break;
                }
            }

            string[] EEpromStringsToWrite = new string[9];  // space for all TD components
            // if we are WRITING, get contents from the file...
            if (bI2C_SlaveRead == false)
            {
                EEpromStringsToWrite = File.ReadAllLines(filespec);
            }

            // Initial hardware based constants
            MasterMUXAddr = 0x71;
            SlaveMUXAddr = 0x70;
            int iBoardComponentStart, iBoardComponentEnd;
            if (TDcomponentIndex == -1) // do all components
            {
                iBoardComponentStart = 0;
                iBoardComponentEnd = 7;
            }
            else
            {
                iBoardComponentStart = TDcomponentIndex;
                iBoardComponentEnd = TDcomponentIndex;

            }

            for (int iBoardComponent = iBoardComponentStart; iBoardComponent <= iBoardComponentEnd; iBoardComponent++)
            {
                TransferBufferLen = (uint)DataBytes.Length;
                OpcodeBytes[0] = 0x0; // starting address in ALL EEPROMs (default - note CPLD command exception)
                OpcodeByteLen = 1;    // default single byte address for EEPROMs
                switch (iBoardComponent)
                {
                    case 0: // TDboard
                        MasterMUXChan = 0x80;
                        SlaveMUXChan = 0xFF;
                        TargetSlaveAddr = 0x54;
                        break;
                    case 1: // ADC
                        MasterMUXChan = 0x02;
                        SlaveMUXChan = 0xFF;
                        TargetSlaveAddr = 0x54;
                        break;
                    case 2: // DBB1
                        MasterMUXChan = 0x02;
                        SlaveMUXChan = 0xFF;
                        TargetSlaveAddr = 0x50;
                        break;
                    case 3: // DAC 
                        MasterMUXChan = 0x08;
                        SlaveMUXChan = 0x04;
                        TargetSlaveAddr = 0x54;
                        break;
                    case 4: // ABB1 
                        MasterMUXChan = 0x08;
                        SlaveMUXChan = 0x01;
                        TargetSlaveAddr = 0x50;
                        break;
                    case 5: // ABB2
                        MasterMUXChan = 0x08;
                        SlaveMUXChan = 0x02;
                        TargetSlaveAddr = 0x50;
                        break;
                    case 6: // ABB3 
                        MasterMUXChan = 0x08;
                        SlaveMUXChan = 0x08;
                        TargetSlaveAddr = 0x50;
                        break;
                    case 7: // 3P mez. card
                        MasterMUXChan = 0x01;
                        SlaveMUXChan = 0xFF;
                        TargetSlaveAddr = 0x50;
                        break;
                    default:
                        MasterMUXChan = 0x80;
                        SlaveMUXChan = 0xFF;
                        TargetSlaveAddr = 0x54;
                        break;
                }

                // write OpCode(s) to UnManaged Opcode Mem
                for (int byteIndx = 0; byteIndx < OpcodeBytes.Length; byteIndx++)
                {
                    System.Runtime.InteropServices.Marshal.WriteByte(UnManagedOpCodeBuffer, byteIndx, OpcodeBytes[byteIndx]);
                }
                if (bI2C_SlaveRead == true)
                {
                    TotalDataPlusOpcodesByteCount = TransferBufferLen;
                    status = (THORDAQ_STATUS)ThorDAQAPIXI2CReadWrite(MasterBoardIndex, bI2C_SlaveRead, MasterMUXAddr, MasterMUXChan,
                                                                                    SlaveMUXAddr, SlaveMUXChan, TargetSlaveAddr, I2CbusHz, PageReadWriteLen,
                                                                                    UnManagedOpCodeBuffer, ref OpcodeByteLen,  // OpcodeLen >0 means READ operation
                                                                                    UnManagedDataBuffer, ref TotalDataPlusOpcodesByteCount); // returns ALL bytes transfered
                }
                else  // WRITING!  contents came from file
                {
                    // now discard the 5 char prefix
                    EEpromStringsToWrite[iBoardComponent] = EEpromStringsToWrite[iBoardComponent].Remove(0, 5);
                    // discard the added format "-" chars
                    EEpromStringsToWrite[iBoardComponent] = EEpromStringsToWrite[iBoardComponent].Replace("-", "");
                    // convert ASCII chars to hex bytes
                    char[] EEPromBytes = EEpromStringsToWrite[iBoardComponent].ToCharArray();
                    // (actual EEPROM content starts at arrayIndx 5)

                    OpcodeByteLen = 1;  // i.e., starting EEPROM address of 0x0
                    for (int dEEPROMbyteIndx = 0; dEEPROMbyteIndx < TOT_SERIAL_SZ; dEEPROMbyteIndx++) // only DATA bytes
                    {
                        // e.g., start of TD main board Data buffer to send, 0x54 0x44 0x30 ... ("TD0...")
                        System.Runtime.InteropServices.Marshal.WriteByte(UnManagedDataBuffer, (int)(dEEPROMbyteIndx), (byte)EEPromBytes[dEEPROMbyteIndx]);
                    }
                    TotalDataPlusOpcodesByteCount = TransferBufferLen;  // initialized to DATA len - but RETURNS a total len transfered including required OpCodes (i.e. by PageSize)
                    status = (THORDAQ_STATUS)ThorDAQAPIXI2CReadWrite(MasterBoardIndex, bI2C_SlaveRead,
                        MasterMUXAddr, MasterMUXChan,
                        SlaveMUXAddr, SlaveMUXChan, TargetSlaveAddr, I2CbusHz, PageReadWriteLen,
                        UnManagedOpCodeBuffer, ref OpcodeByteLen,
                        UnManagedDataBuffer, ref TotalDataPlusOpcodesByteCount); // returns ALL bytes transfered
                }
                string sWriteDir = (bI2C_SlaveRead == true) ? "READ" : "WRITE";
                if (status != THORDAQ_STATUS.STATUS_SUCCESSFUL)
                {
                    UpdateConsoleStatus(" ThorDAQAPIXI2CReadWrite() I2C " + sWriteDir + " failure @M:" + MasterMUXChan.ToString("X") + " S:" + SlaveMUXChan.ToString("X") + " TargetAddr: 0x" + TargetSlaveAddr.ToString("X") +
                                        "  " + status.ToString() + " Bytes: " + TotalDataPlusOpcodesByteCount.ToString());
                    //          fileFormatStrings.Add(EEPROMcontentsToWrite[iBoardComponent] + "ERROR: I2C transfer error, status 0x" + status.ToString("X") + " ByteXferLen = " + TotalDataPlusOpcodesByteCount.ToString());
                }
                else  // format/display the EEPROM string
                {
                    // bring unmanaged buffer (the C++ passed DATA array) into C# managed...
                    for (int CPPbyteIndx = 0; CPPbyteIndx < TransferBufferLen; CPPbyteIndx++)
                    {
                        DataBytes[CPPbyteIndx] = Marshal.ReadByte(UnManagedDataBuffer, CPPbyteIndx);
                    }
                    // convert hex digits to ASCII
                    string rawBuf = ascii.GetString(DataBytes);
                    sFormattedBuf = " ";
                    if (EEPROMcontentsToWrite[iBoardComponent] != "CPLD2")
                    {
                        sFormattedBuf = EEPROMcontentsToWrite[iBoardComponent] + FormatSerStr(rawBuf);
                    }
                    // special cases... [ MOVE CPLD versions away from EEPROM functions ]
                    else if (EEPROMcontentsToWrite[iBoardComponent] == "CPLD2")
                    {
                        sFormattedBuf = EEPROMcontentsToWrite[iBoardComponent] + string.Format("Read-only Ver. 0x{0}.{1}.{2}.{3}",
                            DataBytes[0].ToString("X"), DataBytes[1].ToString("X"), DataBytes[2].ToString("X"), DataBytes[3].ToString("X"));
                    }
                    fileFormatStrings.Add(sFormattedBuf);
                }
            }
            if (bI2C_SlaveRead == true)
            {
                foreach (string Line in fileFormatStrings)
                {
                    UpdateConsoleStatus(Line);
                }
            }
            if (bCreateFile)
            {
                File.WriteAllLines(filespec, fileFormatStrings);
                UpdateConsoleStatus("EEPROM contents written to " + filespec);
            }
            Marshal.FreeHGlobal(UnManagedOpCodeBuffer);
            Marshal.FreeHGlobal(UnManagedDataBuffer);
            return sFormattedBuf;
        }


        public void APISetAIOConfig(List<String> argumentsList)
        {
            string strAIOconfig;
            int iBNClabel;
            Int32 iNum;
            int range = 10;
            int MUX_DMAchan = 0xFF; // invalid default
            string sPolarity;

            if (argumentsList.Count < 4) // at least <BNCIndex> <Dir> <Polarity> 
            {
                UpdateConsoleStatus(CommonDefinition.HELP_APIGetSetAIOConfig_MSSAGE);
                return;
            }

            string sDIR = argumentsList[2].ToUpper();
            if (sDIR == "O" || sDIR == "o") // Required arg for OUTPUT?
            {
                if (argumentsList.Count < 5) // at least <BNCIndex> <Dir> <Polarity> <MUX_DMAchan>
                {
                    UpdateConsoleStatus(CommonDefinition.HELP_APIGetSetAIOConfig_MSSAGE);
                    return;
                }
                MUX_DMAchan = (Int32)new System.ComponentModel.Int32Converter().ConvertFromString(argumentsList[4]);
            }

            sPolarity = argumentsList[3].ToUpper(); // Unipolar or Bipolar (single char)

            //       01234567890
            // e.g. "AnnXDVmmMcc", where	"n" is 0-11 (BNC label "nn", e.g. "A12" or "A03"),
            //							"X" is Input/Output direction "I" or "O", 
            //							"D" is "U"nipolar or "B"ipolar, 
            //							"V" is Volts "mm" range e.g. "10"
            //							"M" is (Output only) MUX'd DAC Waveform DMAcc [channel (00-11)] location for BNC label in FPGA's GlobalABBXmuxReg (all BOBs)            iNum = (Int32)new System.ComponentModel.Int32Converter().ConvertFromString(argumentsList[1]);
            iNum = (Int32)new System.ComponentModel.Int32Converter().ConvertFromString(argumentsList[1]);
            iBNClabel = iNum;
            strAIOconfig = String.Format("A{0,2:D2}", iNum); // 'D' plus two chars
            strAIOconfig += sDIR + sPolarity; // single DIR char

            strAIOconfig += String.Format("V{0,2:D2}", range);
            // for OUTPUT, we have 5th arg (the MUX)
            if (argumentsList.Count >= 5)  // 4th arg is Range
            {
                strAIOconfig += String.Format("M{0,2:D2}", MUX_DMAchan);
            }

            // Must send as "unmanaged" char array
            IntPtr UnManagedConfigBuffer = Marshal.AllocHGlobal((int)TD_BOBAIDef.NumBOB_AIs); // (includes NULL byte)
            //byte[] bytes = Encoding.ASCII.GetBytes(strDIOconfig);
            int Idx = 0;
            foreach (char Chr in strAIOconfig)
            {
                Marshal.WriteByte(UnManagedConfigBuffer, Idx++, (byte)Chr);
            }

            THORDAQ_STATUS status;
            status = ThorDAQAPISetAIOConfig(MasterBoardIndex, UnManagedConfigBuffer, (int)TD_BOBAIDef.CharPerBOB_AI); // DIOconfig is DLL char (e.g. [32][9] array) - send our size
            if (status != THORDAQ_STATUS.STATUS_SUCCESSFUL)
            {
                if (status == THORDAQ_STATUS.STATUS_DEVICE_NOT_EXISTS)
                    UpdateConsoleStatus(String.Format("ERROR: BOB hardware does not support BNC D{0} specified connection", iBNClabel));
                else
                    UpdateConsoleStatus("ERROR: BOB hardware device not found on I2C bus - cable problem?");
            }
            Marshal.FreeHGlobal(UnManagedConfigBuffer); // release our temp buffer
        }

        // allows for configuring entire 3U BOB 31 DIOs as intput or output
        // sets the D0-D4 to GPIO AUX MUXes 27-31
        private static bool TestDIOs(bool Output)
        {
            bool bStatus;

            List<string> retString = new List<string> { }; ;
            int i;
            int iMUX = 27;
            int iAUX = 0;
            string sIOdir = (Output == true) ? "O" : "I";
            for (i = 0; i < 32; i++, iMUX++, iAUX++)
            {
                switch (i)
                {
                    case 0:
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                        retString = APISetDIOConfig(new List<string> { "InternalCmd", i.ToString(), sIOdir, i.ToString(), iMUX.ToString(), iAUX.ToString() });
                        break;

                    // special cases of how to configure (for test) last 3 DIOs -
                    // we can pick whatever is most usable for test...
                    /*
                    iResonant_scanner_line_trigger = 0x00,
                    iExtern_line_trigger = 0x01,
                    iExtern_pixel_clock = 0x02,
                    oScan_direction = 0x03,
                    oHorizontal_line_pulse = 0x04,
                    oPixel_integration = 0x05,
                    oStart_of_frame = 0x06,
                    iFrame_hardware_trigger = 0x07,
                    iExternal_SOF = 0x08,
                    oPixel_clock_pulse = 0x09,
                    oDigital_Waveform_0 = 0x0A, (10d)
                    oDigital_Waveform_1 = 0x0B,
                    oDigital_Waveform_2 = 0x0C,
                    oDigital_Waveform_3 = 0x0D,
                    oDigital_Waveform_4 = 0x0E,
                    oDigital_Waveform_5 = 0x0F,
                    oDigital_Waveform_6 = 0x10,
                    oDigital_Waveform_7 = 0x11,
                    oDigital_Waveform_8 = 0x12,
                    oDigital_Waveform_9 = 0x13,
                    oDigital_Waveform_10 = 0x14,
                    oDigital_Waveform_11 = 0x15,
                    oDigital_Waveform_12 = 0x16,
                    oDigital_Waveform_13 = 0x17, // 23d
                    oDigital_Waveform_14 = 0x18,
                    oDigital_Waveform_15 = 0x19,
                    oCapture_Active = 0x1A, // 26d
                                    */

                        //TODO: [BGB] Don will fix this
                    //case 5:
                    //    if (Output == true)
                    //    {
                    //        iMUX = (int)TD_DIO_MUXedSLAVE_PORTS.oCapture_Active;
                    //    }
                    //    else
                    //    {
                    //        iMUX = (int)TD_DIO_MUXedSLAVE_PORTS.iExtern_line_trigger;

                    //    }
                    //    retString = APISetDIOConfig(new List<string> { "InternalCmd", i.ToString(), sIOdir, i.ToString(), iMUX.ToString() });
                    //    break;
                    //case 6:
                    //    if (Output == true)
                    //    {
                    //        iMUX = (int)TD_DIO_MUXedSLAVE_PORTS.oStart_of_frame;
                    //    }
                    //    else
                    //    {
                    //        iMUX = (int)TD_DIO_MUXedSLAVE_PORTS.iResonant_scanner_line_trigger;

                    //    }
                    //    retString = APISetDIOConfig(new List<string> { "InternalCmd", i.ToString(), sIOdir, i.ToString(), iMUX.ToString() });
                    //    break;
                    //case 7:
                    //    if (Output == true)
                    //    {
                    //        iMUX = (int)TD_DIO_MUXedSLAVE_PORTS.oHorizontal_line_pulse;
                    //    }
                    //    else
                    //    {
                    //        iMUX = (int)TD_DIO_MUXedSLAVE_PORTS.iFrame_hardware_trigger;

                    //    }
                    //    retString = APISetDIOConfig(new List<string> { "InternalCmd", i.ToString(), sIOdir, i.ToString(), iMUX.ToString() });
                    //    break;

                    default:
                        retString = APISetDIOConfig(new List<string> { "InternalCmd", i.ToString(), sIOdir, i.ToString() });
                        if (i >= 8 && (retString.Last() != "SUCCESS"))
                            retString[0] = "SUCCESS"; // ignore errors after 1st 8 DIO config successful (support Legacy DBB1)
                        break;
                }
                if (retString.Last() != "SUCCESS") break;
            }

            bStatus = (retString.Last() == "SUCCESS") ? true : false;
            return bStatus;
        }


        // in DLL, CHAR config[32][6]  e.g. "DnnXmmMxxAii", where "n" is 0-31 (BNC label D0 through "D31", "X" is
        // Input/Output direction "I" or "O", "mm" is Index of Output source, "M" is MUX code,
        //                        "Aii" is Aux GPIO index (only for TD_DIO_MUXedSLAVE_PORTS::Aux_GPIO_x codes 27-31)
        //       const int NumDIO = 32;
        //       const int CharsPerDIO = 9 + 1;  // field width plus NULL (must match ThorDAQAPIGetDIOConfig() DLL)

        public static List<string> APISetDIOConfig(List<String> argumentsList)
        {
            List<string> RetStrings = new List<string> { };
            string strDIOconfig;
            int iBNClabel;
            Int32 iNum;
            Int32 iOutputSrc = 0; // (unused for Input)
            Int32 iMUXcode = 48;  // def. CPLD configured (3U BOB)
            Int32 iAUXcode = -1;  // needed only for AUX_GPIO in FPGA

            if (argumentsList.Count < 3) // at least <Chan>  <Dir>  <CpySrc>
            {
                RetStrings.Add(CommonDefinition.HELP_APIGetSetDIOConfig_MSSAGE);
                return RetStrings;
            }

            string sDIR = argumentsList[2].ToUpper();
            if (sDIR != "I" && sDIR != "O") // Only options
            {
                RetStrings.Add(CommonDefinition.HELP_APIGetSetDIOConfig_MSSAGE);
                return RetStrings;
            }

            iNum = (Int32)new System.ComponentModel.Int32Converter().ConvertFromString(argumentsList[1]);
            iBNClabel = iNum;
            strDIOconfig = String.Format("D{0,2:D2}", iNum); // 'D' plus two chars
            strDIOconfig += sDIR; // single DIR char

            if (argumentsList.Count >= 4)  // 4th arg is CpySrc
            {
                iOutputSrc = (Int32)new System.ComponentModel.Int32Converter().ConvertFromString(argumentsList[3]);
                strDIOconfig += String.Format("{0,2:D2}", iOutputSrc);
            }
            else
            {
                strDIOconfig += String.Format("{0,2:D2}", iNum);  // CpySrc default is always the Dn index itself
            }

            if (argumentsList.Count >= 5)  // 5th arg is MUX code
            {
                iMUXcode = (Int32)new System.ComponentModel.Int32Converter().ConvertFromString(argumentsList[4]);
            }
            strDIOconfig += String.Format("M{0,2:D2}", iMUXcode); // arg or the default

            if (argumentsList.Count >= 6)  // 6th arg is AUX index for FPGA's AUX_GPIO
            {
                iAUXcode = (Int32)new System.ComponentModel.Int32Converter().ConvertFromString(argumentsList[5]);
            }
            strDIOconfig += String.Format("A{0,2:D2}", iAUXcode); // arg or the default

            // check for TEST case!!
            if (iBNClabel == 99)
            {
                bool bOutputs = (sDIR == "O") ? true : false;
                bool bStatus = TestDIOs(bOutputs);  // recursive function (calls this function with separate BNC indexes)
                if (bStatus == true) RetStrings.Add("SUCCESS");
                else RetStrings.Add("FAIL");
                return RetStrings;
            }

            // Must send as "unmanaged" char array
            IntPtr UnManagedConfigBuffer = Marshal.AllocHGlobal((int)TD_BOBDIODef.CharPerBOB_DIO); // (includes NULL byte)
            //byte[] bytes = Encoding.ASCII.GetBytes(strDIOconfig);
            int Idx = 0;
            foreach (char Chr in strDIOconfig)
            {
                Marshal.WriteByte(UnManagedConfigBuffer, Idx++, (byte)Chr);
            }

            THORDAQ_STATUS status;
            status = ThorDAQAPISetDIOConfig(MasterBoardIndex, UnManagedConfigBuffer, (int)TD_BOBDIODef.CharPerBOB_DIO); // DIOconfig is DLL char (e.g. [32][9] array) - send our size
            if (status != THORDAQ_STATUS.STATUS_SUCCESSFUL)
            {
                if (status == THORDAQ_STATUS.STATUS_DEVICE_NOT_EXISTS)
                    RetStrings.Add("ERROR: BOB hardware device not found on I2C bus - cable problem?");
                else
                    RetStrings.Add(String.Format("ERROR: BOB hardware does not support BNC D{0} specified connection", iBNClabel));
                return RetStrings;
            }
            Marshal.FreeHGlobal(UnManagedConfigBuffer); // release our temp buffer
            RetStrings.Add("SUCCESS");
            return RetStrings;
        }

        public void APIGetDDR3status(List<String> argumentsList)
        {
            foreach (string Str in GetDDR3status(new List<string> { "InternalCmd", " " }))
            {
                UpdateConsoleStatus(Str);
            }
        }

        // SharedTypes.cs is the source of the TD_DDR3_SPD (also used by DLL)
        static List<string> GetDDR3status(List<String> argumentsList)
        {
            List<string> RetStrings = new List<string> { };
            THORDAQ_STATUS status = 0;
            List<String> strDDR3status = new List<string> { };

            IntPtr UnManagedConfigBuffer = Marshal.AllocHGlobal((int)TD_DDR3_SPD.DDR3_SPD_LEN);
            for (int k = 0; k < (int)TD_DDR3_SPD.DDR3_SPD_LEN; k++)
                Marshal.WriteByte(UnManagedConfigBuffer, k, 0);

            char[] DDR3status = new char[(int)TD_DDR3_SPD.DDR3_SPD_LEN];

            status = ThorDAQAPIGetDDR3status(MasterBoardIndex, UnManagedConfigBuffer, (int)TD_DDR3_SPD.DDR3_SPD_LEN); // 
            if (status == THORDAQ_STATUS.STATUS_SUCCESSFUL)
            {
                int i;
                for (i = 0; i < (int)TD_DDR3_SPD.DDR3_SPD_LEN; i++)
                    DDR3status[i] = (char)Marshal.ReadByte(UnManagedConfigBuffer, i);
                string EntireStatus = new string(DDR3status);
                string[] DDR3fields = EntireStatus.Split(';');
                i = 0;
                foreach (string Str in DDR3fields)
                {
                    RetStrings.Add(Str);
                    //   UpdateConsoleStatus(Str);
                    if (++i >= (int)TD_DDR3_SPD.SPD_FieldCount) break; // currently only 6 fields defined.
                }
            }
            else
            {
                RetStrings.Add("Error reading I2C for DDR3 mem module EEPROM");
            };
            return RetStrings;
        }

        // make calls through new API, which in DLL detects Legacy vs. 3U Panel BOB
        static bool SetThorImageLS_BOBConfig()
        {
            bool bStatus = true;
       //     string retStatus;
            // configure minimum Global Registers needed to run
            // route the Digital (DBB1) I/O signals
            // MATCH ThorImageLS with DIO1 12   digit_output_2 (software)               IDevice.TD_DIO_MUXedSLAVE_PORTS.Digital_Output_2
            //                        DIO2 13   digit_output_3 (software)               TD_DIO_MUXedSLAVE_PORTS .Digital_Output_3
            //                        DIO3 9    pixel clock pulse output                TD_DIO_MUXedSLAVE_PORTS .Pixel_clock_pulse_output
            //                        DIO4 10   digit_output_0 (software)               TD_DIO_MUXedSLAVE_PORTS .Digital_Output_0
            //                        DIO5 0    resonant scanner line trigger input     TD_DIO_MUXedSLAVE_PORTS .Resonant_scanner_line_trigger_input
            //                        DIO6 7    external hardware frame trigger input   TD_DIO_MUXedSLAVE_PORTS .Hardware_trigger_input
            //                        DIO7 3    scan direction                          TD_DIO_MUXedSLAVE_PORTS .Start_of_frame_output [replace "3" for ManufTest]
            //                        DIO8 11   digit_output_1 (software)               IDevice.TD_DIO_MUXedSLAVE_PORTS .Digital_Output_1

            // read the current BOB config table and setup BOB hardware according to Legacy or 3U

            THORDAQ_STATUS status = 0;
            List<String> strDIOlist = new List<string> { };

            IntPtr UnManagedConfigBuffer = Marshal.AllocHGlobal((int)TD_BOBDIODef.NumBOB_DIOs * (int)TD_BOBDIODef.CharPerBOB_DIO);
            char[] DIOconfig = new char[(int)TD_BOBDIODef.CharPerBOB_DIO];

            status = ThorDAQAPIGetDIOConfig(MasterBoardIndex, UnManagedConfigBuffer, ((int)TD_BOBDIODef.NumBOB_DIOs * (int)TD_BOBDIODef.CharPerBOB_DIO)); // DIOconfig is DLL char (e.g. [32][6] array) - send our size

            if (status == THORDAQ_STATUS.STATUS_SUCCESSFUL)
            {
                int i, j;
                for (i = 0; i < (int)TD_BOBDIODef.NumBOB_DIOs; i++)
                {
                    for (j = 0; j < (int)TD_BOBDIODef.CharPerBOB_DIO; j++)
                    {
                        DIOconfig[j] = (char)Marshal.ReadByte(UnManagedConfigBuffer, (i * (int)TD_BOBDIODef.CharPerBOB_DIO + j));
                    }
                    // convert the array into strings for easier display
                    strDIOlist.Add(new string(DIOconfig));
                }
            }

            // we now have the entire CURRENT DIO config definition - write the entire configuration back,
            // one DIO at a time

            byte[] bytes;
            int iDIOsConfigured = 0;
            int k;
            for (k = 15 /*(int)TD_BOBDIODef.NumBOB_DIOs-1*/; k >= 0; k--) // for each of the 32 3U DIOs (or 8 for Legacy DIOs)
            {
                bytes = Encoding.ASCII.GetBytes(strDIOlist[k]);
                int Idx = 0;
                foreach (char Chr in strDIOlist[k])
                {
                    Marshal.WriteByte(UnManagedConfigBuffer, Idx++, (byte)Chr);
                }
                status = ThorDAQAPISetDIOConfig(MasterBoardIndex, UnManagedConfigBuffer, (int)TD_BOBDIODef.CharPerBOB_DIO); // DIOconfig is DLL char (e.g. [32][] array) - send our size
                if (status == THORDAQ_STATUS.STATUS_SUCCESSFUL)
                {
                    ++iDIOsConfigured;
                }
                Thread.Sleep(1);
            }
            bStatus = (iDIOsConfigured >= 8) ? true : false;
            return bStatus;
        }


        public static List<String> ADCStream_setup(List<String> argumentsList)
        {
            List<String> ReturnStrings = new List<string> { };
            int argCount = argumentsList.Count;
            int arg = 1;
            bool bJESDreset = false;
            UInt64 uiGain = 0; // default

            UInt64 RegValue = 0;
            bool bStatus, b3Psampling = false, bResGalvo = false;
            while (arg < argCount)
            {
                switch (argumentsList[arg])
                {
                    case "-j":
                        bJESDreset = true;
                        break;

                    case "-l":
                        b3Psampling = true;
                        break;

                    case "-r":
                        bResGalvo = true;
                        break;

                    case "-g": // must have Int arg following
                        arg++; // get number
                        UInt64.TryParse(argumentsList[arg], out uiGain);
                        break;
                    default:
                        break;
                };
                arg++;
            }


            // Check for component reset flags...
            // looking at the Xilinx FAN side of board, LEDs on top -- 2 LEDs on far left are the JESD core indicators
            // they are turned of until the cores are "reset"/enabled
            // Non-reset (i.e. after hard power cycle) cores will deliver random 16-bit garbage through the 
            // ADC stream right into the 14-bit ADC image pixel samples
            if (bJESDreset == true)
            {
                // RESET the JESD cores 
                // sequence: JESD 0, delay JESD 1, delay, JESD 0 delay
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("JESD204B_config_core1", 0);
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("JESD204B_config_core2", 0);
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADCFMCInterfaceControlReg", ref RegValue);
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADCFMCInterfaceControlReg", RegValue);
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADCFMCInterfaceControlReg", ref RegValue);
                ReturnStrings.Add(string.Format("ADCFMCInterfaceControlReg: 0x{0}", RegValue.ToString("X4")));
                Thread.Sleep(125); // ms
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("JESD204B_config_core1", 1);
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("JESD204B_config_core1", ref RegValue);
                ReturnStrings.Add(string.Format("JESD204B_config_core1: {0}", RegValue.ToString()));
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("JESD204B_config_core2", 1);
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("JESD204B_config_core2", ref RegValue);
                ReturnStrings.Add(string.Format("JESD204B_config_core2: {0}", RegValue.ToString()));
                // write entire register (to reset JESD cores)
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADCFMCInterfaceControlReg", ref RegValue);
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADCFMCInterfaceControlReg", RegValue);
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADCFMCInterfaceControlReg", ref RegValue);
                ReturnStrings.Add(string.Format("ADCFMCInterfaceControlReg: 0x{0}", RegValue.ToString("X4")));
                Thread.Sleep(125); // ms
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("JESD204B_config_core1", 0);
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("JESD204B_config_core2", 0);
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADCFMCInterfaceControlReg", ref RegValue);
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADCFMCInterfaceControlReg", RegValue);
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADCFMCInterfaceControlReg", ref RegValue);
                ReturnStrings.Add(string.Format("ADCFMCInterfaceControlReg: 0x{0}", RegValue.ToString("X4")));
                Thread.Sleep(125); // ms, copied from legacy code
            }


            if (bResGalvo == true)
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADCStream_SCAN_PER_SRCE", 1);
            else
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADCStream_SCAN_PER_SRCE", 0);

            // set the Gain(attentuation)
            string ADCgainChanx;
            for (int ch = 0; ch < 4; ch++)
            {
                ADCgainChanx = "ADCGainChan" + ch.ToString();
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write(ADCgainChanx, uiGain);
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read(ADCgainChanx, ref RegValue);
                ReturnStrings.Add(ADCgainChanx + RegValue.ToString());
            }

            // we require the Pre-filter DC offset for ThorDAQ PMTs (biased to negative voltage)
            bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADCStream_DCoffsetPreFIR", 1);
            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADCStream_DCoffsetPreFIR", ref RegValue);
            ReturnStrings.Add(string.Format("    ADCStream_DCoffsetPreFIR: {0}", RegValue));

            // no Post-filter offset
            bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADCStream_DCoffsetPostFIR", 0);
            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADCStream_DCoffsetPostFIR", ref RegValue);
            ReturnStrings.Add(string.Format("    ADCStream_DCoffsetPostFIR: {0}", RegValue));

            if (b3Psampling == true)
            {
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADCStream_LASER3P_MARKER_EN", 1);
                // must be computed according to 3P laser specifics
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADCStreamDownsampleReg", 1);
            }
            else // 2P / confocal
            {
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADCStream_LASER3P_MARKER_EN", 0);
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADCStream_REVERB_MP_EN", 0);
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADCStreamDownsampleReg", 0);
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADCStream3PReverbMPCyclesReg", 0);
            }
            // READ BACK our settings
            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADCStream_SCAN_PER_SRCE", ref RegValue);
            ReturnStrings.Add(string.Format("    ADCStream_SCAN_PER_SRCE: {0}", RegValue));
            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADCStream_LASER3P_MARKER_EN", ref RegValue);
            ReturnStrings.Add(string.Format("    ADCStream_LASER3P_MARKER_EN: {0}", RegValue));
            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADCStream_REVERB_MP_EN", ref RegValue);
            ReturnStrings.Add(string.Format("    ADCStream_REVERB_MP_EN: {0}", RegValue));
            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADCStream3PReverbMPCyclesReg", ref RegValue);
            ReturnStrings.Add(string.Format("    ADCStream3PReverbMPCyclesReg: {0}", RegValue));

            // New REVERB_MP_EN
            // NOTE: exist GG-DLL code sets 3P_Markers_RDivide even when 3P disabled
            //add 1% to the clock rate to have some margin when calculating the RDivideFactor
            double clockRate = 160000000 * 1.01 / 1000000; // assume clock at 160 MHz
            Byte rdivideFactor = 0;
            const Double RDivideLaserFreqLimit = 1.25; // MHz
            if (clockRate > RDivideLaserFreqLimit)
            {
                if ((clockRate / Math.Floor(clockRate)) <= RDivideLaserFreqLimit)
                {
                    rdivideFactor = (Byte)((Byte)Math.Floor(clockRate) - 1);
                }
                else
                {
                    rdivideFactor = (Byte)(Math.Floor(clockRate));
                }
            }

            // ADC Stream Processing
            // copy Carl's FIR logic from IOCTL
            // Two Filters, Pre and Post (p)
            // Four Channels
            // 16 coefficients per channel

            UInt64 c, f;
            UInt64 StreamControlReg = 0;

            bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADCStreamControlReg", ref StreamControlReg);
            ushort[] PreFilter_FIR_coeff = new ushort[16] { 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512 };
            ushort[] PostFilter_FIR_coeff = new ushort[16] { 0, 0, 0, 819, 819, 819, 819, 819, 819, 819, 819, 819, 819, 0, 0, 0 };
            for (c = 0; c < 4; c++)
            {
                // turn on the AnalogFrontEnd (AFE) for the channel
                string AFEbit = "ADCAFEenChan" + c.ToString();
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write(AFEbit, 0); // e.g. starts at 0x208

                UInt64 regValue = StreamControlReg | (c << 3);
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADCStreamControlReg", regValue); // 0x1C0
                // Pre FIR coeffs
                for (f = 0; f < 16; f++)
                {
                    bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADCStreamFIRcoeffReg", PreFilter_FIR_coeff[f]); // 0x1E8
                }
                // Post FIR coeffs
                regValue = StreamControlReg | ((c + 4) << 3);
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADCStreamControlReg", regValue); // 0x1C0
                for (f = 0; f < 16; f++)
                {
                    bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADCStreamFIRcoeffReg", PostFilter_FIR_coeff[f]); // 0x1E8
                }
            }
            bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADCInterface3PMarkersRDivide", rdivideFactor); // 0x230
            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADCInterface3PMarkersRDivide", ref RegValue);
            ReturnStrings.Add(string.Format("    ADCInterface3PMarkersRDivide: {0}", RegValue));

            // Downsampling factor (0 for none)...
            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADCStreamDownsampleReg", ref RegValue);
            ReturnStrings.Add(string.Format("    ADCStreamDownsampleReg: {0}", RegValue));

            ReturnStrings.Add("SUCCESS");
            return ReturnStrings;
        }



        public static List<String> ADCSampleClock_setup(List<String> argumentsList)
        {
            List<String> ReturnStrings = new List<string> { };
            int argCount = argumentsList.Count;
            int arg = 1;
            UInt64 RegValue = 0;
            bool bStatus, bLASER_sync_sampling = false;
            bool Use2P_ref = false;
            bool Use3P_ref = false;

            while (arg < argCount)
            {
                switch (argumentsList[arg])
                {
                    case "-c":
                        break;
                    case "-2":
                        Use2P_ref = true;
                        break;
                    case "-3":
                        Use3P_ref = true;
                        break;

                    default:
                        break;
                };
                arg++;
            }

            if (Use2P_ref == true || Use3P_ref == true)
                bLASER_sync_sampling = true;

            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("SamplingClockControlReg", ref RegValue);
            string sBuf = string.Format("Initial value of SamplingClockControlReg: 0x{0}", RegValue.ToString("X2"));
            ReturnStrings.Add(sBuf);
            //            UpdateConsoleStatus(string.Format("Initial value of SamplingClockControlReg: 0x{0}", RegValue.ToString("X2")));
            // select the sampling clock as constructed by the AD9517 Synthesizer (pg. 26 "Thordaq System Timing Architecture")
            // a sequence of register settings is necessary to enable or disable the FPGA's PLL/MMCM circuitry
            // and/or the internal 80MHz reference;
            // the PLL/MMCM is required for "LASER sync" mode, where either 2P or 3P LASER pulses are causing the
            // PMT signal we need to capture via ADC stream.  In "Confocal" mode, there is only an internal 
            // 80MHz clock (for 160MHz ADC sampling) input to Synthesizer, and PLL/MMCM is disabled
            if (bLASER_sync_sampling == true) // 2P/3P external LASER input required
            {
                // is the external LASER 2P (from "REF" on ADC mezzanine board) or 3P (from 3P Mezzanine board)
                // ADC FMC settings required - selects between 3p/2p or Internal 80MHz clock and PLL/MMCM enable (SWUG Pg. 39)
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("JESD204B_Sysref_sync_to_laser", 1);
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("JESD204B_Sysref_sync_to_laser", ref RegValue);
                ReturnStrings.Add(string.Format("JESD204B_Sysref_sync_to_laser: {0}", RegValue.ToString()));
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADC_GPIO1_INT_REF_EN", 0); // enable selection
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADC_GPIO1_INT_REF_EN", ref RegValue);
                ReturnStrings.Add(string.Format("ADC_GPIO1_INT_REF_EN: {0}", RegValue.ToString()));
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADC_GPIO0_INT_REF_SEL", 0); // DISABLE internal 80MHz ref
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADC_GPIO0_INT_REF_SEL", ref RegValue);
                ReturnStrings.Add(string.Format("ADC_GPIO0_INT_REF_SEL: {0}", RegValue.ToString()));
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADC_GPIO2_L_FPGA_REF_EN", 0); // LASER sync enable
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADC_GPIO2_L_FPGA_REF_EN", ref RegValue);
                ReturnStrings.Add(string.Format("ADC_GPIO2_L_FPGA_REF_EN: {0}", RegValue.ToString()));

                // SWUG Pg. 30
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("SC_PHASE_INCREMENT_MODE", 0); // not used in 'confocal'?
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("SC_PHASE_INCREMENT_MODE", ref RegValue);
                ReturnStrings.Add(string.Format("    SC_PHASE_INCREMENT_MODE: {0}", RegValue));
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("SC_FREQ_MEAS_SEL", 1); // use internal 80MHz ref
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("SC_FREQ_MEAS_SEL", ref RegValue);
                ReturnStrings.Add(string.Format("    SC_FREQ_MEAS_SEL: {0}", RegValue));
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("SC_3P_EN", 0);  // don't "source select" 3P 
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("SC_3P_EN", ref RegValue);
                ReturnStrings.Add(string.Format("    SC_3P_EN: {0}", RegValue));


            }
            else // configuration for "confocal" using internal 80MHz reference
            {
                // ADC FMC settings required - selects between 3p/2p or Internal 80MHz clock and PLL/MMCM enable (SWUG Pg. 39)
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("JESD204B_Sysref_sync_to_laser", 0);
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("JESD204B_Sysref_sync_to_laser", ref RegValue);
                ReturnStrings.Add(string.Format("JESD204B_Sysref_sync_to_laser: {0}", RegValue.ToString()));
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADC_GPIO1_INT_REF_EN", 0);
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADC_GPIO1_INT_REF_EN", ref RegValue);
                ReturnStrings.Add(string.Format("ADC_GPIO1_INT_REF_EN: {0}", RegValue.ToString()));
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADC_GPIO0_INT_REF_SEL", 1); // internal 80MHz ref
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADC_GPIO0_INT_REF_SEL", ref RegValue);
                ReturnStrings.Add(string.Format("ADC_GPIO0_INT_REF_SEL: {0}", RegValue.ToString()));
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADC_GPIO2_L_FPGA_REF_EN", 1); // LASER sync disable
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADC_GPIO2_L_FPGA_REF_EN", ref RegValue);
                ReturnStrings.Add(string.Format("ADC_GPIO2_L_FPGA_REF_EN: {0}", RegValue.ToString()));

                // SWUG Pg. 30
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("SC_PHASE_INCREMENT_MODE", 0); // not used in 'confocal'?
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("SC_PHASE_INCREMENT_MODE", ref RegValue);
                ReturnStrings.Add(string.Format("    SC_PHASE_INCREMENT_MODE: {0}", RegValue));
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("SC_FREQ_MEAS_SEL", 1); // use internal 80MHz ref
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("SC_FREQ_MEAS_SEL", ref RegValue);
                ReturnStrings.Add(string.Format("    SC_FREQ_MEAS_SEL: {0}", RegValue));
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("SC_3P_EN", 0);  // don't "source select" 3P 
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("SC_3P_EN", ref RegValue);
                ReturnStrings.Add(string.Format("    SC_3P_EN: {0}", RegValue));
                // 
                // now clear LASER_MARKEN_EN Sync. for 3P  (SWUG Pg. 34)
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADCStream_LASER3P_MARKER_EN", 0);
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADCStream_LASER3P_MARKER_EN", ref RegValue);
                ReturnStrings.Add(string.Format("    ADCStream_LASER3P_MARKER_EN: {0}", RegValue));

            }

            // All modes...
            // default setting SWUG Pg. 40 -- when would it change?
            bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADC_GPIO3_FiltAB_MODE", 2); // LASER ref input clock B/W
            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADC_GPIO3_FiltAB_MODE", ref RegValue);
            ReturnStrings.Add(string.Format("ADC_GPIO3_FiltAB_MODE: {0}", RegValue.ToString()));
            // Add final string for "SUCCESS" or "FAILURE" status of entire function
            ReturnStrings.Add("SUCCESS");
            return ReturnStrings;
        }

        public static List<string> ScanControl_setup(List<String> argumentsList)
        {
            List<string> ReturnStrings = new List<string> { };
            int argCount = argumentsList.Count;
            int arg = 1;
            UInt64 RegValue = 0;
            bool bStatus;
            UInt32 PixelDelay = 1;   // 16ns Tick_Blk (valid range 1 -> (2^24-1) (ThorImageLS constant at 1)
            UInt32 PixelDwell = 10; // 5 ns Ticks
            UInt32 ScanningIntraLineDelay = 12;  // 0x4410 from 4096x4096
            UInt32 ScanningIntraFrameDelay = 25; //  0x1d23c from 4096
            UInt32 BiDirectionMode = 0;  // single direction default
            UInt32 MasterTimingScanMode = (UInt32)TDcontroller.ImageSyncMasterTimingMode.Galvo_Galvo; // galvo-galvo

            while (arg < argCount)
            {
                switch (argumentsList[arg])
                {
                    case "-m":
                        UInt32.TryParse(argumentsList[arg + 1], out MasterTimingScanMode);
                        arg++; // next flag
                        break;
                    case "-p":
                        UInt32.TryParse(argumentsList[arg + 1], out PixelDwell);
                        arg++; // next flag
                        break;
                    case "-P":
                        UInt32.TryParse(argumentsList[arg + 1], out PixelDelay);
                        arg++; // next flag
                        break;
                    case "-l":
                        UInt32.TryParse(argumentsList[arg + 1], out ScanningIntraLineDelay);
                        arg++; // next flag
                        break;
                    case "-f":
                        UInt32.TryParse(argumentsList[arg + 1], out ScanningIntraFrameDelay);
                        arg++; // next flag
                        break;

                    case "-B":
                        UInt32.TryParse(argumentsList[arg + 1], out BiDirectionMode);
                        arg++; // next flag
                        break;

                    default:
                        ReturnStrings.Add(string.Format("Unknown option: {0}", argumentsList[arg]));
                        break;
                };
                arg++;
            }

            // APISetBOBstatus() writes default ABBX MUX values according to attached BOB
            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("GlobalABBXmuxReg", ref RegValue);
            ReturnStrings.Add(string.Format("GlobalABBXmuxReg: 0x{0}", RegValue.ToString("X16")));

            // Command line args set...
            // Global Image Sync  "GlobalImageSyncControlReg" (0x08)
            bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("BIDIR_SCAN_MODE", BiDirectionMode); // 
            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("BIDIR_SCAN_MODE", ref RegValue);
            ReturnStrings.Add(string.Format("BIDIR_SCAN_MODE: {0}", RegValue.ToString()));

            bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("HW_TRIG_MODE", 0); // GlobalImageSyncControlReg (0x08)
            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("HW_TRIG_MODE", ref RegValue);
            ReturnStrings.Add(string.Format("HW_TRIG_MODE: {0}", RegValue.ToString()));
            // set appropriate Registers for Image Sync timing mode
            if (MasterTimingScanMode == (UInt32)TDcontroller.ImageSyncMasterTimingMode.Galvo_Galvo)
            {
                // (also set in ADCStream API)
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADCStream_SCAN_PER_SRCE", 0);

                // pixel timing
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ScanningGalvoPixelDwell", PixelDwell); // 5ns tick 
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ScanningGalvoPixelDwell", ref RegValue);
                ReturnStrings.Add(string.Format("ScanningGalvoPixelDwell: {0}", RegValue.ToString()));

                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ScanningGalvoPixelDelay", PixelDelay); // 16ns tick_block 
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ScanningGalvoPixelDelay", ref RegValue);
                ReturnStrings.Add(string.Format("ScanningGalvoPixelDelay: {0}", RegValue.ToString()));
                // line timing
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ScanningIntraLineDelay", ScanningIntraLineDelay); // 2405ns tick_block match 16x16 timing diagram
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ScanningIntraLineDelay", ref RegValue);
                ReturnStrings.Add(string.Format("ScanningIntraLineDelay: {0}", RegValue.ToString()));
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ScanningIntraFrameDelay", ScanningIntraFrameDelay);
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ScanningIntraFrameDelay", ref RegValue);
                ReturnStrings.Add(string.Format("ScanningIntraFrameDelay: {0}", RegValue.ToString()));

                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("SCAN_DIR", 0); // "forward" scan
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("SCAN_DIR", ref RegValue);
                ReturnStrings.Add(string.Format("SCAN_DIR: {0}", RegValue.ToString()));

            }
            else if (MasterTimingScanMode == (UInt32)TDcontroller.ImageSyncMasterTimingMode.Resonant_Galvo)
            {
                // (also set in ADCStream API)
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADCStream_SCAN_PER_SRCE", 1);

                // note that normal Resonant line scan controller needs to "warm up" for thermal drift stability
                // so the usual circumstance is the ADPLL is locking onto the Resonant feedback signal before 
                // ThorDAQ software starts.
                // Configure settings are here for test purposes
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("APDLL_ControlReg", 0x42); // Ki(4) and Kp(2) constants
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("APDLL_ControlReg", ref RegValue);
                ReturnStrings.Add(string.Format("APDLL_ControlReg: {0}", RegValue.ToString()));

                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ScanningIntraFrameDelay", ScanningIntraFrameDelay);
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ScanningIntraFrameDelay", ref RegValue);
                ReturnStrings.Add(string.Format("ScanningIntraFrameDelay: {0}", RegValue.ToString()));

                // Set the center frequency for the ADPLL control loop
                UInt64 uFreqTarget = 7900;  // 7.9 kHz for "8 KHz" scanner - what about 12 KHz ?
                double pow32 = Math.Pow(2, 32);
                UInt64 uiCenterFreq = uFreqTarget * (UInt64)pow32 / 160000000;
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("APDLL_DCO_CenterFreq", uiCenterFreq);
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("APDLL_DCO_CenterFreq", ref RegValue);
                ReturnStrings.Add(string.Format("APDLL_DCO_CenterFreq: {0}", RegValue.ToString()));

                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("APDLL_SyncDelay", 0);
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("APDLL_PhaseOffset", 0);

                // mix it up -- GG uses "forward" scan
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("SCAN_DIR", 1); // "reverse" scan
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("SCAN_DIR", ref RegValue);
                ReturnStrings.Add(string.Format("SCAN_DIR: {0}", RegValue.ToString()));
            }
            else  // External Source
            {

            }
            // Scanning sync control bits
            bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("SRCE_SOF_MODE", 1); // 1 = on 1st LineSync after Start
            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("SRCE_SOF_MODE", ref RegValue);
            ReturnStrings.Add(string.Format("SRCE_SOF_MODE: {0}", RegValue.ToString()));
            bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("FRM_GEN_MODE", 0);
            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("FRM_GEN_MODE", ref RegValue);
            ReturnStrings.Add(string.Format("FRM_GEN_MODE: {0}", RegValue.ToString()));
            bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("TIME_SRCE_SELECT", MasterTimingScanMode);
            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("TIME_SRCE_SELECT", ref RegValue);
            ReturnStrings.Add(string.Format("TIME_SRCE_SELECT: {0}", RegValue.ToString()));

            ReturnStrings.Add("SUCCESS");
            return ReturnStrings;
        }


        public static List<string> S2MM_DMA_setup(List<String> argumentsList)
        {
            List<string> ReturnStrings = new List<string> { };
            string FunctionStatus = "SUCCESS";
            int ChannelMask = 1; // default channel 0 only (4 total)
            int DDR3startAddr = 0x0; // 0 is valid DDR3 start
            int arg = 1;
            int BRAMdescriptorCount = 3; // number of BRAM descriptors per bank (per channel)
            int argCount = argumentsList.Count;
            string[] FrameDimension;
            int PixelsInLine = 0;   // Width dimension (0 means read shadow register value)
            int TotalLines = 0;     // Height dimension (can be 1 for single line scan)
            S2MM_CONFIG s2mmConfig = new S2MM_CONFIG();
            IntPtr ADClineSamplesLUT;
            // parse args...
            while (arg < argCount)
            {
                switch (argumentsList[arg])
                {
                    case "-a":
                        if (argumentsList[arg + 1].Contains("0x"))
                        {
                            DDR3startAddr = int.Parse(argumentsList[arg + 1].Replace("0x", string.Empty), NumberStyles.HexNumber);
                        }
                        else
                        {
                            DDR3startAddr = int.Parse(argumentsList[arg + 1]);
                        }
                        arg++; // next flag
                        break;
                    case "-c": // channel mask
                        if (argumentsList[arg + 1].Contains("0x"))
                        {
                            ChannelMask = int.Parse(argumentsList[arg + 1].Replace("0x", string.Empty), NumberStyles.HexNumber);
                        }
                        else
                        {
                            ChannelMask = int.Parse(argumentsList[arg + 1]);
                        }
                        arg++; // next flag
                        break;
                    case "-n":
                        Int32.TryParse(argumentsList[arg + 1], out BRAMdescriptorCount);
                        arg++; // next flag
                        break;

                    case "-d":  // get frame dimensions
                        FrameDimension = argumentsList[arg + 1].Split('x');
                        Int32.TryParse(FrameDimension[0], out PixelsInLine);
                        Int32.TryParse(FrameDimension[1], out TotalLines);
                        arg++; // next flag
                        break;
                }
                arg++; // next flag
            }
            // variables set ...
            UInt64 Value = 0;
            if (PixelsInLine == 0) // user wants existing dimensions...
            {
                // if user hasn't already set WxH dimension, notify and exit
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("GlobalImageHSIZE", ref Value);
                if (Value == 0)
                {
                    ReturnStrings.Add("Fatal Error: Image dimensions HSIZE/VSIZE must be defined");
                    ReturnStrings.Add("FAIL");
                    return ReturnStrings;
                }
                // Value is HSIZE, the pixel (image size -1) per SWUG
                PixelsInLine = (int)(Value + 1);
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("GlobalImageVSIZE", ref Value);
                TotalLines = (int)(Value + 1);
            }
            else // user specifies (new) dimensions
            {
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("GlobalImageHSIZE", (ulong)PixelsInLine - 1);
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("GlobalImageAcqHSIZE", (ulong)PixelsInLine - 1); // assume single Plane!
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("GlobalImageVSIZE", (ulong)TotalLines - 1);
            }
            // review the settings...
            UInt64 S2MMwidth = 0, S2MMheight = 0; // temp variables
            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("GlobalImageHSIZE", ref S2MMwidth);
            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("GlobalImageAcqHSIZE", ref Value);
            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("GlobalImageVSIZE", ref S2MMheight);

            string S2MM_imageDimensions = string.Format("Image FPGA Global HSIZE x VSIZE {0}w x {1}h (GlobalImageAcqHSIZE: {2})", S2MMwidth, S2MMheight, Value);
            ReturnStrings.Add(S2MM_imageDimensions);

            s2mmConfig.HSize = PixelsInLine;
            s2mmConfig.VSize = TotalLines;
            s2mmConfig.ChannelMask = ChannelMask;
            s2mmConfig.NumberOfDescriptorsPerBank = BRAMdescriptorCount;
            s2mmConfig.DRAMstartingAddress = DDR3startAddr;

            DERIVED_TIMING DerivedTiming = new DERIVED_TIMING();
            double dFramesPerSec = 0;
            double dSecondsPerFrame = 0;
            double dMBytesPerSec = 0;

            int iStatus = ThorDAQAdapters[MasterBoardIndex].S2MMadcDMA.Init(ref s2mmConfig, ref DerivedTiming);
            if (iStatus != 0)
            {
                ReturnStrings.Add(ThorDAQAdapters[MasterBoardIndex].S2MMadcDMA.sErrMsg);
                FunctionStatus = "FAIL";
            }
            else
            {
                string Msg;
                bool bStatus;
                UInt64 RegValue = 0;
                // T_frame is in NANO seconds -- get Frames/Sec rate
                if (DerivedTiming.T_frame != 0)
                {
                    dSecondsPerFrame = (double)(DerivedTiming.T_frame * Math.Pow(10, -9));
                    dFramesPerSec = 1.0 / dSecondsPerFrame;
                    dMBytesPerSec = dFramesPerSec * (PixelsInLine * TotalLines * 2) / 1024 / 1024;
                    Msg = string.Format("  DMA rate: Frames per second {0,10:#.00}, ({1:#0.000,000} sec/frame, {2:#.000} usec/line) {3,10:#.00} MB/sec",
                        dFramesPerSec, dSecondsPerFrame, ((double)DerivedTiming.T_line / 1000.0), dMBytesPerSec);
                    ReturnStrings.Add(Msg);
                    Msg = string.Format("  Pixel to Pixel timing: {0:#0.000} micro-sec",
                        ((double)DerivedTiming.Pixel_to_Pixel_timeNS / 1000.0));
                    ReturnStrings.Add(Msg);

                    // "Scanning Period" register (for GalvoGalvo)
                    ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("TIME_SRCE_SELECT", ref RegValue);
                    if (RegValue == (UInt32)TDcontroller.ImageSyncMasterTimingMode.Galvo_Galvo)
                    {
                        bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADCStreamDownsampleReg", DerivedTiming.ADCStreamDownsamplingRate);
                        bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("ADCStreamScanningPeriodReg", DerivedTiming.ADCStreamScanningPeriodReg);
                        if (bStatus != true)
                            ReturnStrings.Add("Error write register 'ADCStreamScanningPeriodReg' -- outdated 'thordaq' DLL ?? ");

                        // LookUp Table required for DMA - for Galvo-Galvo, just linearly divide up lineADCsampleAcqBuffer
                        // NOTE!  If samples too close to START or END of line ("ADCStreamScanningPeriodReg" # of samples)
                        // symptom is ADC samples close to end of frame are NOT DMA'd!
                        // We know this from setting DDR3 DMA destination memory to constants like 0xAA,
                        // then when DMA for frame completes, "0xAA" values are present near end of frame
                        UInt32 FPGA_ADCsampleLineAcqBuffStep = (UInt16)((DerivedTiming.ADCStreamScanningPeriodReg - 1) / (ulong)PixelsInLine);
                        UInt32 nextAcqBufSamplePickoff = 1;

                        ///// TEST!!!!!!!!!!!!!!!!
                        //    FPGA_ADCsampleLineAcqBuffStep = 40;
                        //    nextAcqBufSamplePickoff = 32142;

                        short[] LUTbuffer; // for debug/copy
                        const int LUTbyteChannelSize = 4096 * 2 * 8; // (4k pixels) new 64k REVERB/MP size in bytes
                        LUTbuffer = new short[LUTbyteChannelSize / 2];
                        // NOTE!  Reduced LUT array to SINGLE channel because 4 channels exceeds Marshalling size limit
                        unsafe // required to access the fixed arrays...
                        {
                            for (int pixel = 0; pixel < PixelsInLine;
                                     pixel++, nextAcqBufSamplePickoff += FPGA_ADCsampleLineAcqBuffStep)
                            {
                                LUTbuffer[pixel] = (short)nextAcqBufSamplePickoff; // (debug)
                                //LUTbuffer[pixel] = (ushort)((ushort)LUTbuffer[pixel]  & (ushort)0xFFFF); // limit 
                            }
                        }
                        ReturnStrings.Add(string.Format("    LUTbuffer[0]: {0} LUTbuffer[{1}]: {2} (First and last LUT indexes)",
                                                            LUTbuffer[0], (ushort)(PixelsInLine) - 1, LUTbuffer[(ushort)(PixelsInLine) - 1]));

                        // Write to FPGA
                        int size = Marshal.SizeOf(LUTbuffer[0]) * LUTbuffer.Length;
                        ADClineSamplesLUT = Marshal.AllocHGlobal(size); // allocated "unmanaged" buffer
                        // copy the "managed" LUT into unmanaged space
                        Marshal.Copy(LUTbuffer, 0, ADClineSamplesLUT, LUTbuffer.Length);
                        // (the .NET managed buffer shows 32-bit sign-extended "ushorts", but only lower 16-bits sent to DLL)
                        iStatus = ThorDAQAdapters[MasterBoardIndex].S2MMadcDMA.LoadLUT(ADClineSamplesLUT);
                        if (iStatus != (int)THORDAQ_STATUS.STATUS_SUCCESSFUL)
                            ReturnStrings.Add("Error on S2MMadcDMA.LoadLUT(): can't load LUT");
                        else
                        {
                            /*                   UpdateConsoleStatus("LookUpTable (LUT) contents:");  // DEBUG LUT
                                               for (int p = 0; p < PixelsInLine; p += 8)
                                               {
                                                   UpdateConsoleStatus(string.Format("    [{0}]: {1,5} {2,5} {3,5} {4,5} {5,5} {6,5} {7,5} {8,5}", 
                     p, LUTbuffer[p], LUTbuffer[p+1],LUTbuffer[p+2],LUTbuffer[p+3],LUTbuffer[p+4],LUTbuffer[p+5],LUTbuffer[p+6],LUTbuffer[p+7]));
                                               }*/
                            ReturnStrings.Add("S2MMadcDMA.LoadLUT() successfully loaded LUT (all channels)");
                        }
                        Marshal.FreeHGlobal(ADClineSamplesLUT);
                    }
                    // ADCStreamScanningPeriodReg should be "0" if NOT GalvoGalvo mode
                    ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADCStreamDownsampleReg", ref RegValue);
                    ReturnStrings.Add(string.Format("    ADCStreamDownsamplingRate: {0} (D/S Factor = Rate+1)", RegValue));
                    ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("ADCStreamScanningPeriodReg", ref RegValue);
                    ReturnStrings.Add(string.Format("    ADCStreamScanningPeriodReg: {0} (ADC samples per line)", RegValue));

                }
                Msg = string.Format("Configured S2MM DMA (both banks), ChannelMask 0x{0} for {1} Descriptors (Frames) to DDR3 @ {2}h - {3}h",
                    ChannelMask.ToString("X"), BRAMdescriptorCount, s2mmConfig.DRAMstartingAddress.ToString("X8"),
                    s2mmConfig.DRAMendingAddress.ToString("X8"));
                ReturnStrings.Add(Msg);

                ThorDAQAdapters[MasterBoardIndex].S2MMadcDMA.IRQcontrol(true, true); // enable IRQs
                ReturnStrings.Add("Enabled DMA / user Interrupts");
            }
            ReturnStrings.Add(FunctionStatus); // SUCCESS or FAIL for entire function
            return ReturnStrings;
        }

        static ulong channelPlaybackMask = 0; // accumulated mask of enabled channels
        static List<Enum> channelPlaybackLEDs = new List<Enum> { };

        static ulong DDR3_playbackMemStart = 0x50000000;
        static bool WaveformPlaybackInitializedOnce = false; // too easy for user to miss this

        public static List<string> DACWaveform_setup(List<String> argumentsList)
        {
            List<string> ReturnStrings = new List<string> { };
            string playbackFilespec = "c:\\temp\\test_wave.txt";
            string DOWavePlaybackFilespec = "c:\\temp\\DAC_DOtest_wave.txt";

            int argNum;
            uint iChannelIndexMask = 0x3FFF; // Index 0 - 11 (for 12 'ANALOG' channels), 0x6 is channelIndex 2,1, 0x9 is 8,0; Two 'DIGITAL' channels at Index 12-13
            bool bInitFlag = true;
            bool bStatus;
            ulong RegValue = 0;
            ulong OffsetDACcntsValue = 0;
            uint PlaybackFreq = 1000000;
            string sFunctionStatus = "FAIL";
            ulong DAC_DOwaveformsToLoad = 2;  // 1 loads Index [chan] 12, 2 loads 12 & 13 (Command option adjusts)

            channelPlaybackLEDs.Clear();
            channelPlaybackMask = 0; // reset all channels - reset below per user request
                                     // route the Analog (ABBx) I/O signals
                                     // 

            for (argNum = 1; argNum < argumentsList.Count; argNum++)
            {
                switch (argumentsList[argNum])
                {
                    case "-z":
                        PlaybackFreq = uint.Parse(argumentsList[++argNum]); // (advance ARG list)
                        break;
                    case "-i":
                        bInitFlag = false;
                        break;

                    case "-o":
                        OffsetDACcntsValue = uint.Parse(argumentsList[++argNum]);
                        break;

                    case "-f":
                        if (argNum + 1 < argumentsList.Count)
                            playbackFilespec = argumentsList[++argNum]; // next arg after "-f" is filespec
                        else
                        {
                            //Create OpenFileDialog - get filename from user
                            Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();
                            dlg.DefaultExt = ".mcs";
                            dlg.Filter = "Text Files (.txt)|*.txt";
                            // Display OpenFileDialog by calling ShowDialog method
                            Nullable<bool> result = dlg.ShowDialog();
                            // Get the selected file name and display in a TextBox
                            if (result == true)
                            {
                                // Open document
                                playbackFilespec = dlg.FileName;
                            }
                        }
                        break;
                    case "-c":
                        // anticipate HEX mask
                        if (argumentsList[argNum + 1].Contains("0x"))
                        {
                            iChannelIndexMask = uint.Parse(argumentsList[++argNum].Replace("0x", string.Empty), NumberStyles.HexNumber);
                        }
                        else
                        {
                            iChannelIndexMask = uint.Parse(argumentsList[++argNum]);
                        }
                        break;
                    case "-d":
                        DAC_DOwaveformsToLoad = ulong.Parse(argumentsList[++argNum]);
                        break;
                }
            }
            // Check for component reset flags...
            if (bInitFlag == true || (WaveformPlaybackInitializedOnce == false)) // failure to init once is mass confusion
            {
                WaveformPlaybackInitializedOnce = true;
                for (ulong chIndx = 0; chIndx < (12 + DAC_DOwaveformsToLoad); chIndx++)
                {
                    string Playback_EnX = "DAC_DMA_Playback_En" + chIndx.ToString();

                    bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write(Playback_EnX, 0); // (reset)
                    ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read(Playback_EnX, ref RegValue);
                    //                    ReturnStrings.Add(string.Format("    {0}: 0x{1}", Playback_EnX, RegValue.ToString("X3")));
                }
                // reset the table
                bStatus = ThorDAQAdapters[MasterBoardIndex].DACcard.DACWave.Init(DDR3_playbackMemStart, 0); // unknown WaveformLen
                if (bStatus != true)
                    ReturnStrings.Add(ThorDAQAdapters[MasterBoardIndex].DACcard.DACWave.sErrMsg);
                else
                    ReturnStrings.Add(string.Format("DACWave.Init() @ {0}: Success", DDR3_playbackMemStart.ToString("X8")));
            }

            // Waveform sample count delays -- kludge design to attempt hardware solution to Galvo control tripping
            // because of too rapid voltage changes, such as when transitioning to park voltage to first DAC sample
            // in waveform  (DACWaveGen_SOF_DelaySysClkCnt deprecated in 5.0 FPGA )
            ///bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("DACWaveGen_SOF_DelaySysClkCnt", 399); // 1 = 5ns, 399 = 2us
            //ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("DACWaveGen_SOF_DelaySysClkCnt", ref RegValue);
            //ReturnStrings.Add(string.Format("DACWaveGen_SOF_DelaySysClkCnt: {0}", RegValue.ToString()));


            bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("DACWaveGen_EOF_DelaySampleCnt", 1);
            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("DACWaveGen_EOF_DelaySampleCnt", ref RegValue);
            ReturnStrings.Add(string.Format("DACWaveGen_EOF_DelaySampleCnt: {0}", RegValue.ToString()));

            // load the waveforms in preparation for playback
            // which waveform channels will be used (from 1 to 13, 12 DAC analog + 1 Digital Output (DBB1 BrkoutBox)
            ulong chanSelector = 1;
            ulong MaxChannelOrdinal = 0;        // HIGHEST enabled channel! FPGA "feature" is you must enable ALL channels up to the one you want
                                                // so if you only want channel 12 (Digital IO) you must enable channels 0 through 11

            // For now - no BANK SWITCHING support
            bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("DAC_Bank_Switch_EN", 0);
            UInt64 DDR3starAddr;

            for (ulong chan = 0; chan < (12 + DAC_DOwaveformsToLoad); chan++, chanSelector <<= 1)  // 1st 12 channels (Indexes) are ANALOG for BNCs A0-A11
            {
                int iStatus;
                string sFileName = (chan < 12) ? playbackFilespec : DOWavePlaybackFilespec;
                if ((chanSelector & iChannelIndexMask) != 0)
                {
                    if (chan < 12) // 0-11 are the "Analogs" to BNCs A0 - A11
                    {
                        ReturnStrings.Add("  Load 'ANALOG' DAC waveform '" + sFileName + "' in channel " + chan + " ...");
                        iStatus = ThorDAQAdapters[MasterBoardIndex].DACcard.DACWave.Load((uint)chan, sFileName, out DDR3starAddr);
                    }
                    else
                    {
                        ReturnStrings.Add("  Load 'DIGITAL' DAC waveform '" + sFileName + "' in channel " + chan + " ...");
                        iStatus = ThorDAQAdapters[MasterBoardIndex].DACcard.DACWave.Load((uint)chan, sFileName, out DDR3starAddr);
                    }
                    if (iStatus <= 0)  // error, or no DAC samples loaded?
                    {
                        ReturnStrings.Add(ThorDAQAdapters[MasterBoardIndex].DACcard.DACWave.sErrMsg);
                        sFunctionStatus = "FAILED to Load waveform file : " + sFileName;
                    }
                    else // channel waveform is loaded... update other DAC registers
                    {
                        sFunctionStatus = "SUCCESS";
                        MaxChannelOrdinal = chan + 1;
                        ReturnStrings.Add(string.Format(" {0}(0x{1} bytes) DAC samples starting @ DDR3 addr 0x{2}, Playback Freq {3} Hz", iStatus, (iStatus * 2).ToString("X"), DDR3starAddr.ToString("X8"), PlaybackFreq));
                        // read shadow reg of waveform playback table channel list start
                        string DACdescChan = "DACWavePlayDMAdescriptorChan" + chan.ToString();
                        bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read(DACdescChan, ref RegValue);
                        ReturnStrings.Add(string.Format("    {0}: 0x{1}", DACdescChan, RegValue.ToString("X16")));

                        // set the update rate - FPGA says must be >= 199 (typo fixed in later SWUG)
                        // compute the FPGA register setting...
                        UInt64 DAC_UpdateRate = (200000000 / PlaybackFreq) - 1; // 200MHz sysclock
                                                                                // set offset

                        // Set Park and Offset for DAC 
                        if (chan == 12 || (chan == 13)) // special case for Digital Outputs channels
                        {
                            String sParkChan = "DAC_Park_Chan" + chan.ToString();
                            String sOffsetChan = "DAC_Offset_Chan" + chan.ToString();
                            String sRateChan = "DAC_UpdateRate_Chan" + chan.ToString();
                            // hardware-default Park/Offset settings will make wave DO toggle @ 1MHz
                            bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write(sParkChan, 0x0);
                            bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write(sOffsetChan, 0x0);
                            bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write(sRateChan, DAC_UpdateRate);
                        }
                        else
                        {
                            string strOffset = "DAC_Park_Chan" + chan.ToString();
                            bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write(strOffset, 0);

                            strOffset = "DAC_Offset_Chan" + chan.ToString();
                            bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write(strOffset, OffsetDACcntsValue);
                            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read(strOffset, ref RegValue);
                            ReturnStrings.Add(string.Format("    {0}: {1}", strOffset, RegValue.ToString()));

                            string strUpdateRate = "DAC_UpdateRate_Chan" + chan.ToString();
                            bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write(strUpdateRate, DAC_UpdateRate);
                            if (bStatus != true)
                                ReturnStrings.Add("FAILED FPGAregs.Write() for " + strUpdateRate);
                            bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read(strUpdateRate, ref RegValue);
                            if (bStatus != true)
                                ReturnStrings.Add("FAILED FPGAregs.Read() for " + strUpdateRate);
                            ReturnStrings.Add(string.Format("    {0}: {1}", strUpdateRate, RegValue.ToString()));
                        }
                        // "enable" the DMA playback on the activated channel (bitwise)
                        channelPlaybackMask |= chanSelector;

                        string PlayBackEnXx = "DAC_DMA_Playback_En" + chan.ToString();

                        bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write(PlayBackEnXx, 1);
                        ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read(PlayBackEnXx, ref RegValue);
                        ReturnStrings.Add(string.Format("    {0}: 0x{1}", PlayBackEnXx, RegValue.ToString("X3")));

                        // (re)synchronize DAC update clock with HSync
                        string Sync_hsyncXx = "DAC_Sync_hsync" + chan.ToString();
                        bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write(Sync_hsyncXx, 0);
                        //                            bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("DAC_Sync_hsync", channelPlaybackMask);
                        ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read(Sync_hsyncXx, ref RegValue);
                        ReturnStrings.Add(string.Format("    {0}: 0x{1}", Sync_hsyncXx, RegValue.ToString("X3")));

                        // Digital waveforms are not seen unless specifically MUXed to a BNC
                        bool bMUXset = SetThorImageLS_BOBConfig(); // (Legacy or 3U) if false, DIO config may be incorrect

                        ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("GlobalDBB1muxReg", ref RegValue);
                        ReturnStrings.Add(string.Format("GlobalDBB1muxReg: 0x{0}", RegValue.ToString("X16")));

                    }
                }
            }

            // all waveform channels loaded/enabled...
            if (MaxChannelOrdinal > 0) // ANY channel selected?
            {
                // FPGA register defined as "N-1" channels set (see SWUG): i.e., to enable only 1st channel, write "0"
                // You cannot "activate" higher channel number without also activating all lower channels
                // i.e. per hardware, you cannot activate, say, channel 5 without also activating channels 0-4.
                bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("DAC_DMA_Chans_Active", MaxChannelOrdinal - 1);
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("DAC_DMA_Chans_Active", ref RegValue);
                ReturnStrings.Add(string.Format("DAC_DMA_Chans_Active: {0}", RegValue.ToString()));
            }

            // setup the RunStop Enable on the Digital Waveform channels
    //        bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("DAC_Waveplay_Per_Chan_RunStop_En12", 1);
    //        bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("DAC_Waveplay_Per_Chan_RunStop_En13", 1);
            // Go ahead and assert their Run/Stop control (expect them to start when GlobalRunStop asserted)
    //        bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("DAC_Waveplay_Per_Channel_RunStop12", 1);
    //        bStatus = ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("DAC_Waveplay_Per_Channel_RunStop13", 1);


            ReturnStrings.Add(sFunctionStatus); // last string is always final function status
            return ReturnStrings;
        }

        // Get the "live" input value for INPUT, or last written value if OUTPUT
        // Because this integrates very fast (microsecs) mezz. card DIO with slow (millisecs) I2C
        // read latency time, allow multiple args of any combination, with note to user
        // that mixing fast and slow means the function will not return until all values are
        // read.  If mixed, read SLOW values first, then FAST
        public void APIGetDIO(List<String> argumentsList)
        {
            IntPtr UnManagedConfigBuffer = IntPtr.Zero;
            List<String> strDIOlist = new List<string> { };
            char[] DIOconfig = new char[(int)TD_BOBDIODef.CharPerBOB_DIO];
            Int32 iNum;
            // allocate the MAX buffer needed...
            UnManagedConfigBuffer = Marshal.AllocHGlobal((int)TD_BOBDIODef.NumBOB_DIOs * (int)TD_BOBDIODef.CharPerBOB_DIO);

            // a legal "field" is a BNC index follow by "0" or "1" value to set
            UInt32 NumberOfBNCsToGET = 0;
            string DO_BNCindex;
            List<String> strBNCindex_Value = new List<string> { };
            if (argumentsList.Count < 2)
            {
                UpdateConsoleStatus(CommonDefinition.HELP_APIGetSetDIO_MSSAGE);
                return;
            }
            string sNum;
            int k;
            int i, j;

            try
            {
                // special case of getting them all...
                if (argumentsList[1] == "D99")
                {
                    for (k = 0; k < 8; k++)  // the first 8 must work on Legacy and 3U Panel BOB
                    {
                        DO_BNCindex = String.Format("D{0,2:D2}Xxx      ", k); // 'D' plus two chars, "xx" is default Unknown
                        NumberOfBNCsToGET++;
                        strBNCindex_Value.Add(DO_BNCindex);
                    }
                }
                else
                {
                    // process Dn one at a time...
                    for (int argNum = 1; argNum < argumentsList.Count;)
                    {
                        sNum = argumentsList[argNum++].Remove(0, 1); // i.e. omit leading "d" or "D"
                        iNum = (Int32)new System.ComponentModel.Int32Converter().ConvertFromString(sNum);
                        DO_BNCindex = String.Format("D{0,2:D2}Xxx      ", iNum); // 'D' plus two chars, "xx" is default Unknown
                                                                                 // e.g. "D17X        "
                        NumberOfBNCsToGET++;
                        strBNCindex_Value.Add(DO_BNCindex);
                    }
                }
                // we have the Output(s) to get...
                THORDAQ_STATUS status = 0;
                int Idx = 0, iDIOsCopied = 0;
                foreach (string BNCrecord in strBNCindex_Value)
                {
                    foreach (char Chr in BNCrecord)
                    {
                        Marshal.WriteByte(UnManagedConfigBuffer, Idx++, (byte)Chr);
                    }
                    Idx = ++iDIOsCopied * (int)TD_BOBDIODef.CharPerBOB_DIO;
                }
                // send the "list" of DIO read request... the 1st 8 must succeed
                status = ThorDAQAPIGetDIO(MasterBoardIndex, UnManagedConfigBuffer, (uint)TD_BOBDIODef.CharPerBOB_DIO, NumberOfBNCsToGET);
                if (status != THORDAQ_STATUS.STATUS_SUCCESSFUL)
                {
                    UpdateConsoleStatus(String.Format("ERROR: {0:X}", status));
                }
                else // print the first 8 DIOs
                {
                    for (i = 0; i < NumberOfBNCsToGET; i++)
                    {
                        for (j = 0; j < (int)TD_BOBDIODef.CharPerBOB_DIO; j++)
                        {
                            DIOconfig[j] = (char)Marshal.ReadByte(UnManagedConfigBuffer, (i * (int)TD_BOBDIODef.CharPerBOB_DIO + j));
                        }
                        // convert the array into strings for easier display
                        strDIOlist.Add(new string(DIOconfig));
                    }
                    int NumBNCsProcessed = 0;
                    for (int row = 0; row < 4 && NumBNCsProcessed < NumberOfBNCsToGET; row++)
                    {
                        string DIOline = null;
                        for (int col = 0; col < 8; col++)
                        {
                            DIOline += String.Format(" {0}", strDIOlist[row * 8 + col]);
                            if (++NumBNCsProcessed >= NumberOfBNCsToGET) break; // done
                        }
                        UpdateConsoleStatus(DIOline);
                    }
                }
                if (argumentsList[1] == "D99") // "All DIOs" ?
                {
                    strBNCindex_Value.Clear();
                    for (k = 8, NumberOfBNCsToGET = 0; k < 32; k++)  // only the 0-7 must work on Legacy and 3U Panel BOB
                    {
                        DO_BNCindex = String.Format("D{0,2:D2}Xxx      ", k); // 'D' plus two chars, "xx" is default Unknown
                        NumberOfBNCsToGET++;
                        strBNCindex_Value.Add(DO_BNCindex);
                    }
                    iDIOsCopied = 0;
                    Idx = 0;
                    foreach (string BNCrecord in strBNCindex_Value)
                    {
                        foreach (char Chr in BNCrecord)
                        {
                            Marshal.WriteByte(UnManagedConfigBuffer, Idx++, (byte)Chr);
                        }
                        Idx = ++iDIOsCopied * (int)TD_BOBDIODef.CharPerBOB_DIO;
                    }

                    // send the "list" of DIO read request...
                    status = ThorDAQAPIGetDIO(MasterBoardIndex, UnManagedConfigBuffer, (uint)TD_BOBDIODef.CharPerBOB_DIO, NumberOfBNCsToGET);
                    // these commands will fail if we're connected to Legacy BOB
                    if (status == THORDAQ_STATUS.STATUS_SUCCESSFUL)
                    {
                        strDIOlist.Clear();
                        // output the results
                        {
                            for (i = 0; i < NumberOfBNCsToGET; i++)
                            {
                                for (j = 0; j < (int)TD_BOBDIODef.CharPerBOB_DIO; j++)
                                {
                                    DIOconfig[j] = (char)Marshal.ReadByte(UnManagedConfigBuffer, (i * (int)TD_BOBDIODef.CharPerBOB_DIO + j));
                                }
                                // convert the array into strings for easier display
                                strDIOlist.Add(new string(DIOconfig));
                            }
                            int NumBNCsProcessed = 0;
                            for (int row = 0; row < 4 && NumBNCsProcessed < NumberOfBNCsToGET; row++)
                            {
                                string DIOline = null;
                                for (int col = 0; col < 8; col++)
                                {
                                    DIOline += String.Format(" {0}", strDIOlist[row * 8 + col]);
                                    if (++NumBNCsProcessed >= NumberOfBNCsToGET) break; // done
                                }
                                UpdateConsoleStatus(DIOline);
                            }
                        }

                    }

                }
            }
            catch (ArgumentException e)
            {
                UpdateConsoleStatus(String.Format("ERROR: {0}", e.Message));
                UpdateConsoleStatus(CommonDefinition.HELP_APIGetSetDIO_MSSAGE);
            }
            finally
            {
                if (UnManagedConfigBuffer != IntPtr.Zero)
                    Marshal.FreeHGlobal(UnManagedConfigBuffer); // release our temp buffer
            }
        }

        public static UInt16 devInstance; // a.k.a boardIndx (0-based)
        //Get Board configuration
        public void GetBoardConfiguration(uint boardIndex) // 0-based index of referenced board
        {
            BOARD_INFO_STRUCT BoardConfig = new BOARD_INFO_STRUCT();
            try
            {
                if (Win32.ThorDAQAPIGetBoardCfg(boardIndex, ref BoardConfig) == Win32.STATUS_SUCCESSFUL)
                {
                    UpdateConsoleStatus("Board Configuration Information:");
                    string FPGAver = "FPGA (booted) Version = 0x" + BoardConfig.UserVersion.ToString("X4") + "; PCI DevID = 0x" + BoardConfig.PCIVendorDeviceID.ToString("X4") ;
                    UpdateConsoleStatus(FPGAver);
                    ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, FPGAver);
                    // mezzanine board config?

                }
                else
                {
                    UpdateConsoleStatus("Get Board Configuration Error");
                }
            }
            catch (Exception)
            {
                throw;
            }
        }

        // SharedTypes.cs is the source of the TD_BOBstatusDef (also used by DLL)
        /* TD_BOBstatusDef
           BOBtypeCharLen = 13, // e.g. "3U Panel BOB"
            BOBhardwareRev = 4,  // e.g. "1.2"
            BOB_DDB1_SN    = 29, // e.g. DB100105225601014205/10/2019 (defined field as of Oct-2021 28 chars)
            BOB_ADB1_SN = 29, // e.g. AB1...
            BOB_ADB2_SN = 29, // e.g. AB2...
            BOB_ADB3_SN = 29, // e.g. AB3...
            BOB_status =  32, // e.g. "CABLE SWAPPED", "no CPLD program", "Missing DIO/AIO", "OK"
            DAC_CPLD    = 16,  // e.g. "DACCPLD 1.0.0.1" or "DACCPLD error"
            // valid for 3U BOB only...
            BOB_CPLD = 16     // e.g. BOB...
    */
        public bool APIGetBOBstatus(List<String> argumentsList)
        {
            THORDAQ_STATUS status = 0;
            List<String> strBOBstatus = new List<string> { };

            IntPtr UnManagedConfigBuffer = Marshal.AllocHGlobal((int)TD_BOBstatusDef.BOB_ALL);
            for (int k = 0; k < (int)TD_BOBstatusDef.BOB_ALL; k++)
                Marshal.WriteByte(UnManagedConfigBuffer, k, 0);

            char[] BOBstatus = new char[(int)TD_BOBstatusDef.BOB_ALL];

            status = ThorDAQAPIGetBOBstatus(MasterBoardIndex, UnManagedConfigBuffer, (int)TD_BOBstatusDef.BOB_ALL); // DIOconfig is DLL char (e.g. [32][6] array) - send our size
            if (status == THORDAQ_STATUS.STATUS_SUCCESSFUL)
            {
                int i, Accumlator = 0;
                for (i = 0; i < (int)TD_BOBstatusDef.BOBtypeCharLen; i++)
                {
                    BOBstatus[i] = (char)Marshal.ReadByte(UnManagedConfigBuffer, Accumlator++);
                }
                // convert the array into strings for easier display
                strBOBstatus.Add(new string(BOBstatus));
                for (i = 0; i < (int)TD_BOBstatusDef.BOB_DBB1_SN; i++)
                {
                    BOBstatus[i] = (char)Marshal.ReadByte(UnManagedConfigBuffer, Accumlator++);
                }
                strBOBstatus.Add(new string(BOBstatus));
                for (i = 0; i < (int)TD_BOBstatusDef.BOB_ABB1_SN; i++)
                {
                    BOBstatus[i] = (char)Marshal.ReadByte(UnManagedConfigBuffer, Accumlator++);
                }
                strBOBstatus.Add(new string(BOBstatus));
                for (i = 0; i < (int)TD_BOBstatusDef.BOB_ABB2_SN; i++)
                {
                    BOBstatus[i] = (char)Marshal.ReadByte(UnManagedConfigBuffer, Accumlator++);
                }
                strBOBstatus.Add(new string(BOBstatus));
                for (i = 0; i < (int)TD_BOBstatusDef.BOB_ABB3_SN; i++)
                {
                    BOBstatus[i] = (char)Marshal.ReadByte(UnManagedConfigBuffer, Accumlator++);
                }
                strBOBstatus.Add(new string(BOBstatus));
                for (i = 0; i < (int)TD_BOBstatusDef.BOB_status; i++)
                {
                    BOBstatus[i] = (char)Marshal.ReadByte(UnManagedConfigBuffer, Accumlator++);
                }
                strBOBstatus.Add(new string(BOBstatus));
                // reset buffer
                Array.Clear(BOBstatus, 0, BOBstatus.Length);
                for (i = 0; i < (int)TD_BOBstatusDef.DAC_CPLD; i++)
                {
                    BOBstatus[i] = (char)Marshal.ReadByte(UnManagedConfigBuffer, Accumlator++);
                }
                strBOBstatus.Add(new string(BOBstatus));
                for (i = 0; i < (int)TD_BOBstatusDef.BOB_CPLD; i++)
                {
                    BOBstatus[i] = (char)Marshal.ReadByte(UnManagedConfigBuffer, Accumlator++);
                }
                strBOBstatus.Add(new string(BOBstatus));
                // now read the ABBX mux (in some instances it is changed to default, sometimes not)
                UInt64 RegValue = 0;
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("GlobalABBXmuxReg", ref RegValue);
                strBOBstatus.Add(string.Format("GlobalABBXmuxReg: 0x{0}", RegValue.ToString("X16")));

                foreach (string sRec in strBOBstatus)
                {
                    if (argumentsList != null)
                        argumentsList.Add(sRec);
                    UpdateConsoleStatus(sRec);
                }
            }
            else // error on API call
            {
                UpdateConsoleStatus("ERROR: check HDMI cabling / BOB devices, status " + status.ToString());
                return false;
            }
            Marshal.FreeHGlobal(UnManagedConfigBuffer); // release our temp buffer
            return true;
        }

        // SharedTypes.cs is the source of the TD_BOBDIODef (also used by DLL)
        public void APIGetDIOConfig(List<String> argumentsList)
        {
            THORDAQ_STATUS status = 0;
            List<String> strDIOlist = new List<string> { };

            IntPtr UnManagedConfigBuffer = Marshal.AllocHGlobal((int)TD_BOBDIODef.NumBOB_DIOs * (int)TD_BOBDIODef.CharPerBOB_DIO);
            char[] DIOconfig = new char[(int)TD_BOBDIODef.CharPerBOB_DIO];

            status = ThorDAQAPIGetDIOConfig(MasterBoardIndex, UnManagedConfigBuffer, ((int)TD_BOBDIODef.NumBOB_DIOs * (int)TD_BOBDIODef.CharPerBOB_DIO)); // DIOconfig is DLL char (e.g. [32][6] array) - send our size

            if (status == THORDAQ_STATUS.STATUS_SUCCESSFUL)
            {
                int i, j;
                for (i = 0; i < (int)TD_BOBDIODef.NumBOB_DIOs; i++)
                {
                    for (j = 0; j < (int)TD_BOBDIODef.CharPerBOB_DIO; j++)
                    {
                        DIOconfig[j] = (char)Marshal.ReadByte(UnManagedConfigBuffer, (i * (int)TD_BOBDIODef.CharPerBOB_DIO + j));
                    }
                    // convert the array into strings for easier display
                    strDIOlist.Add(new string(DIOconfig));
                }

                // format as the Dx appears on 3U
                // for Legacy DBB1, expect single row populated by DLL
                for (int row = 0; row < 4; row++)
                {
                    string DIOline = null;
                    for (int col = 0; col < 8; col++)
                    {
                        DIOline += String.Format(" {0}", strDIOlist[row * 8 + col]);
                    }
                    UpdateConsoleStatus(DIOline);
                }
            }
            Marshal.FreeHGlobal(UnManagedConfigBuffer); // release our temp buffer
            return;
        }



        public void APISetDO(List<String> argumentsList)
        {
            IntPtr UnManagedConfigBuffer = IntPtr.Zero;
            // a legal "field" is a BNC index follow by "0" or "1" value to set
            UInt32 NumberOfBNCsToSet = 0;
            Int32 iNum;
            string DO_BNCandValue;
            List<String> strBNCindex_Value = new List<string> { };
            if (argumentsList.Count < 3)
            {
                UpdateConsoleStatus(CommonDefinition.HELP_APIGetSetDIO_MSSAGE);
                return;
            }
            try
            {
                // process args 2 at a time, e.g. "D3 1 D7 0", set BNC D3 to 1, next then iteration, D7 to 0
                for (int argNum = 1; argNum < argumentsList.Count;)
                {
                    string sNum = argumentsList[argNum++].Remove(0, 1); // i.e. omit leading "d" or "D"
                    iNum = (Int32)new System.ComponentModel.Int32Converter().ConvertFromString(sNum);
                    DO_BNCandValue = String.Format("D{0,2:D2}", iNum); // 'D' plus two chars
                    DO_BNCandValue += "X"; // DLL tests configured Direction of I/O (must be configured Output)
                    iNum = (Int32)new System.ComponentModel.Int32Converter().ConvertFromString(argumentsList[argNum++]);
                    DO_BNCandValue += String.Format("{0,2:D2}", iNum); // 'D' plus two chars
                    DO_BNCandValue += "   "; // pad 3 blanks (Mcc)
                    NumberOfBNCsToSet++;
                    strBNCindex_Value.Add(DO_BNCandValue);
                }
                // we have the Output(s) to set...
                THORDAQ_STATUS status = 0;
                UnManagedConfigBuffer = Marshal.AllocHGlobal((int)NumberOfBNCsToSet * (int)TD_BOBDIODef.CharPerBOB_DIO);
                int Idx = 0, iDIOsCopied = 0;
                foreach (string BNCrecord in strBNCindex_Value)
                {
                    foreach (char Chr in BNCrecord)
                    {
                        Marshal.WriteByte(UnManagedConfigBuffer, Idx++, (byte)Chr);
                    }
                    Idx = ++iDIOsCopied * (int)TD_BOBDIODef.CharPerBOB_DIO;
                }
                // send the "list" of DO commands...
                status = ThorDAQAPISetDO(MasterBoardIndex, UnManagedConfigBuffer, (uint)TD_BOBDIODef.CharPerBOB_DIO, NumberOfBNCsToSet);
                if (status != THORDAQ_STATUS.STATUS_SUCCESSFUL)
                {
                    UpdateConsoleStatus(String.Format("ERROR: {0:X}", status));
                }
            }
            catch (ArgumentException e)
            {
                UpdateConsoleStatus(String.Format("ERROR: {0}", e.Message));
                UpdateConsoleStatus(CommonDefinition.HELP_APIGetSetDIO_MSSAGE);
            }
            finally
            {
                if (UnManagedConfigBuffer != IntPtr.Zero)
                    Marshal.FreeHGlobal(UnManagedConfigBuffer); // release our temp buffer
            }
        }


        public string APIGetAI(List<String> argumentsList, bool toPrintConsole = true)
        {
            string result = "";
            uint status = 0;
            double dVolts = 99.9;
            double dADCcounts = 0;
            string AIlabel = " ";
            int iBNCindex = 0;
            bool bReturnVolts = true; // def.
            if (argumentsList.Count < 2)
            {
                AIlabel = "ALL";
            }

            for (int argNum = 1; argNum < argumentsList.Count; argNum++)
            {
                switch (argumentsList[argNum])
                {
                    case "-r":  // range
                        bReturnVolts = false;
                        break;
                    default: // get channel #
                        char[] charsToTrim = { '-', 'a', 'i', 'A', 'I' };
                        AIlabel = argumentsList[argNum].TrimStart(charsToTrim);
                        iBNCindex = (Int32)new System.ComponentModel.Int32Converter().ConvertFromString(AIlabel);
                        break;
                }
            }
            if (AIlabel == "ALL")
            {
                int idx;
                // read all possible AI channels...
                // (For now ONLY the MAX127 channels on 3U IO Panel)
                for (idx = 6; idx < 14; idx++)
                {
                    status = ThorDAQAPIGetAI(MasterBoardIndex, idx, true, ref dVolts);
                    if (status != (uint)THORDAQ_STATUS.STATUS_SUCCESSFUL)
                    {
                        result = "ERROR: status = " + status;
                        if (toPrintConsole)
                            UpdateConsoleStatus(result);
                        return result;
                    }
                    status = ThorDAQAPIGetAI(MasterBoardIndex, idx, false, ref dADCcounts);
                    int iADCcounts = (int)dADCcounts;
                    if (toPrintConsole)
                        UpdateConsoleStatus("AI" + idx + " ADCcnts: " + iADCcounts + " (0x" + iADCcounts.ToString("X") + ")  VDC: " + dVolts.ToString("0.##"));
                    result += "AI" + idx + " ADCcnts: " + iADCcounts + " (0x" + iADCcounts.ToString("X") + ")  VDC: " + dVolts.ToString("0.##") + "\n";
                }
            }
            else // read the single channel...
            {
                // what is the numerical ENUM value for the passed string (e.g. "eAI9")?
                status = ThorDAQAPIGetAI(MasterBoardIndex, iBNCindex, bReturnVolts, ref dVolts);
                if (status != (uint)THORDAQ_STATUS.STATUS_SUCCESSFUL)
                {
                    result = "ERROR: status = " + status;
                    if (toPrintConsole)
                        UpdateConsoleStatus(result);
                    return result;
                }
                if (bReturnVolts)
                {
                    if (toPrintConsole)
                        UpdateConsoleStatus("AI" + iBNCindex + " VDC: " + dVolts.ToString("0.##"));
                    result = dVolts.ToString("0.##");
                }
                else
                {
                    UInt32 ADCcnts = (UInt32)dVolts;
                    if (toPrintConsole)
                        UpdateConsoleStatus("AI" + iBNCindex + " ADCcnts: " + ADCcnts + " (0x" + ADCcnts.ToString("X") + ")"); // CNTS were returned
                    result = "AI" + iBNCindex + " ADCcnts: " + ADCcnts + " (0x" + ADCcnts.ToString("X") + ")";
                }

            }
            return result;
        }




        public bool APIbreakOutBoxLEDControl(List<String> argumentsList)
        {
            uint status;
            Byte LEDstate = 1; // on
            bool bAllLEDs = false;
            string LEDlabel = argumentsList[1].TrimStart('-');  // label must be first arg
            Int32 iterations = 1;
            string LEDstring = " "; // e.g. "AO12" instead of the enum int value for that (debug aid)

            for (int argNum = 1; argNum < argumentsList.Count; argNum++)
            {
                switch (argumentsList[argNum])
                {
                    case "-all": // turn on or off ALL the BreakoutBox LEDs
                        // test case  
                        bAllLEDs = true;

                        break;
                    case "-i":  // iterations?
                        iterations = Int32.Parse(argumentsList[argNum + 1]);
                        break;
                    case "off":
                    case "OFF":
                    case "Off":
                        LEDstate = 0;
                        break;
                    case "on":
                    case "On":
                    case "ON":
                        LEDstate = 1;
                        break;
                    case "blink":
                    case "Blink":
                    case "BLINK":
                        LEDstate = 2;
                        break;
                    default:
                        LEDstate = 0;
                        break;
                }
            }
            if (bAllLEDs)
            {
                status = ThorDAQAPIBreakOutBoxLED(MasterBoardIndex, 0xFF, LEDstate);
            }
            else // individual LED
            {
                // what is the numerical ENUM value for the passed string (e.g. "AO2")?
                // scan the entire enum range...
                Int32 LEDenum;
                for (LEDenum = 0; LEDenum <= (Int32)BBoxLEDenum.AI13; LEDenum++)
                {
                    LEDstring = Enum.GetName(typeof(BBoxLEDenum), LEDenum);
                    if (LEDlabel == LEDstring)
                        break;
                }
                // range check
                if (LEDenum > (Int32)BBoxLEDenum.AI13 || LEDenum < (Int32)BBoxLEDenum.D0)
                {
                    UpdateConsoleStatus("ERROR - invalid BBoxLEDenum " + LEDenum);
                }
                status = ThorDAQAPIBreakOutBoxLED(MasterBoardIndex, LEDenum, LEDstate);
            }
            if (status != (uint)THORDAQ_STATUS.STATUS_SUCCESSFUL)
            {
                UpdateConsoleStatus("ERROR: LED " + LEDstring + " Not found");
            }

            return status == (uint)THORDAQ_STATUS.STATUS_SUCCESSFUL;
        }

        // Breakout Box LED controls
        public void ControlBBleds(List<String> argumentsList)
        {
            bool bLEDon = true;
            bool bAllLEDs = false;
            string LEDlabel = argumentsList[1].TrimStart('-');  // label must be first arg
            Int32 iterations = 1;

            for (int argNum = 1; argNum < argumentsList.Count; argNum++)
            {
                switch (argumentsList[argNum])
                {
                    case "-all": // turn on or off ALL the BreakoutBox LEDs
                        // test case  
                        bAllLEDs = true;

                        break;
                    case "-i":  // iterations?
                        iterations = Int32.Parse(argumentsList[argNum + 1]);
                        break;
                    case "off":
                    case "OFF":
                    case "Off":
                        bLEDon = false;
                        break;
                    default:

                        break;
                }
            }
            if (bAllLEDs)
            {
                do
                {
                    if (bLEDon)
                    {
                        ThorDAQAdapters[MasterBoardIndex].DBB1box.LEDon("DIO1");
                        ThorDAQAdapters[MasterBoardIndex].DBB1box.LEDon("DIO2");
                        ThorDAQAdapters[MasterBoardIndex].DBB1box.LEDon("DIO3");
                        ThorDAQAdapters[MasterBoardIndex].DBB1box.LEDon("DIO4");
                        ThorDAQAdapters[MasterBoardIndex].DBB1box.LEDon("DIO5");
                        ThorDAQAdapters[MasterBoardIndex].DBB1box.LEDon("DIO6");
                        ThorDAQAdapters[MasterBoardIndex].DBB1box.LEDon("DIO7");
                        ThorDAQAdapters[MasterBoardIndex].DBB1box.LEDon("DIO8");

                        ThorDAQAdapters[MasterBoardIndex].ABB1box.LEDon("AO1");
                        ThorDAQAdapters[MasterBoardIndex].ABB1box.LEDon("AO2");
                        ThorDAQAdapters[MasterBoardIndex].ABB1box.LEDon("AO3");
                        ThorDAQAdapters[MasterBoardIndex].ABB1box.LEDon("AO4");
                        ThorDAQAdapters[MasterBoardIndex].ABB1box.LEDon("AI1");
                        ThorDAQAdapters[MasterBoardIndex].ABB1box.LEDon("AI2");

                        ThorDAQAdapters[MasterBoardIndex].ABB2box.LEDon("AO5");
                        ThorDAQAdapters[MasterBoardIndex].ABB2box.LEDon("AO6");
                        ThorDAQAdapters[MasterBoardIndex].ABB2box.LEDon("AO7");
                        ThorDAQAdapters[MasterBoardIndex].ABB2box.LEDon("AO8");
                        ThorDAQAdapters[MasterBoardIndex].ABB2box.LEDon("AI3");
                        ThorDAQAdapters[MasterBoardIndex].ABB2box.LEDon("AI4");

                        ThorDAQAdapters[MasterBoardIndex].ABB3box.LEDon("AO9");
                        ThorDAQAdapters[MasterBoardIndex].ABB3box.LEDon("AO10");
                        ThorDAQAdapters[MasterBoardIndex].ABB3box.LEDon("AO11");
                        ThorDAQAdapters[MasterBoardIndex].ABB3box.LEDon("AO12");
                        ThorDAQAdapters[MasterBoardIndex].ABB3box.LEDon("AI5");
                        ThorDAQAdapters[MasterBoardIndex].ABB3box.LEDon("AI6");
                    }
                    else // off
                    {
                        ThorDAQAdapters[MasterBoardIndex].DBB1box.LEDoff("DIO1");
                        ThorDAQAdapters[MasterBoardIndex].DBB1box.LEDoff("DIO2");
                        ThorDAQAdapters[MasterBoardIndex].DBB1box.LEDoff("DIO3");
                        ThorDAQAdapters[MasterBoardIndex].DBB1box.LEDoff("DIO4");
                        ThorDAQAdapters[MasterBoardIndex].DBB1box.LEDoff("DIO5");
                        ThorDAQAdapters[MasterBoardIndex].DBB1box.LEDoff("DIO6");
                        ThorDAQAdapters[MasterBoardIndex].DBB1box.LEDoff("DIO7");
                        ThorDAQAdapters[MasterBoardIndex].DBB1box.LEDoff("DIO8");

                        ThorDAQAdapters[MasterBoardIndex].ABB1box.LEDoff("AO1");
                        ThorDAQAdapters[MasterBoardIndex].ABB1box.LEDoff("AO2");
                        ThorDAQAdapters[MasterBoardIndex].ABB1box.LEDoff("AO3");
                        ThorDAQAdapters[MasterBoardIndex].ABB1box.LEDoff("AO4");
                        ThorDAQAdapters[MasterBoardIndex].ABB1box.LEDoff("AI1");
                        ThorDAQAdapters[MasterBoardIndex].ABB1box.LEDoff("AI2");

                        ThorDAQAdapters[MasterBoardIndex].ABB2box.LEDoff("AO5");
                        ThorDAQAdapters[MasterBoardIndex].ABB2box.LEDoff("AO6");
                        ThorDAQAdapters[MasterBoardIndex].ABB2box.LEDoff("AO7");
                        ThorDAQAdapters[MasterBoardIndex].ABB2box.LEDoff("AO8");
                        ThorDAQAdapters[MasterBoardIndex].ABB2box.LEDoff("AI3");
                        ThorDAQAdapters[MasterBoardIndex].ABB2box.LEDoff("AI4");

                        ThorDAQAdapters[MasterBoardIndex].ABB3box.LEDoff("AO9");
                        ThorDAQAdapters[MasterBoardIndex].ABB3box.LEDoff("AO10");
                        ThorDAQAdapters[MasterBoardIndex].ABB3box.LEDoff("AO11");
                        ThorDAQAdapters[MasterBoardIndex].ABB3box.LEDoff("AO12");
                        ThorDAQAdapters[MasterBoardIndex].ABB3box.LEDoff("AI5");
                        ThorDAQAdapters[MasterBoardIndex].ABB3box.LEDoff("AI6");
                    }
                    bLEDon = !bLEDon;
                } while (--iterations > 0);
                return;
            }
            switch (LEDlabel)
            {
                case "DIO1":
                case "DIO2":
                case "DIO3":
                case "DIO4":
                case "DIO5":
                case "DIO6":
                case "DIO7":
                case "DIO8":
                    if (bLEDon)
                        ThorDAQAdapters[MasterBoardIndex].DBB1box.LEDon(LEDlabel);
                    else
                        ThorDAQAdapters[MasterBoardIndex].DBB1box.LEDoff(LEDlabel);

                    break;
                case "AO1":
                case "AO2":
                case "AO3":
                case "AO4":
                case "AI1":
                case "AI2":
                    if (bLEDon)
                        ThorDAQAdapters[MasterBoardIndex].ABB1box.LEDon(LEDlabel);
                    else
                        ThorDAQAdapters[MasterBoardIndex].ABB1box.LEDoff(LEDlabel);

                    break;
            }
        }

        //                    requires two arguments - the 0-11 channel num, and -10.0 to +10.0 voltage
        // usage:  SetDAC_ParkValue [-c ChanNum] [-v ParkVoltage]
        public void SetDAC_ParkValue(List<String> argumentsList)
        {
            double dParkVoltage = 0;
            Int32 chan = 0;
            NumberStyles styles;

            if (argumentsList.Count < 4)
            {
                UpdateConsoleStatus("Error: insufficient command line args, [-c ChanNum] [-v ParkVoltage]");
                return;
            }

            for (int argNum = 1; argNum < argumentsList.Count; argNum++)
            {
                switch (argumentsList[argNum])
                {
                    case "-v":
                        styles = NumberStyles.Float;
                        dParkVoltage = Double.Parse(argumentsList[argNum + 1], styles);
                        if (dParkVoltage > 10.0 || dParkVoltage < -10.0)
                        {
                            UpdateConsoleStatus("Error: Park Voltage must be between +10 and -10 Volts");
                            return;
                        }
                        break;
                    case "-c": // must have next arg as filename
                        styles = NumberStyles.Integer;
                        chan = Int32.Parse(argumentsList[argNum + 1], styles);
                        if (chan > 11 || chan < 0)
                        {
                            UpdateConsoleStatus("Error: Channel must be between 0 and 11");
                            return;
                        }
                        break;

                    default:
                        break;
                }
            }
            // we have settings - call the DLL function
            var bStatus = ThorDAQAdapters[MasterBoardIndex]?.DAC_Control.SetStaticDACvoltage((UInt32)chan, dParkVoltage);
            //int iStatus = SetDACParkValue((UInt32)MasterBoardIndex, (UInt32)chan, dParkVoltage);
            if (bStatus != true)
            {
                UpdateConsoleStatus("Error: SetStaticDACvoltage() " + ThorDAQAdapters[MasterBoardIndex]?.DAC_Control?.sErrMsg);
            }
        }


        public void XI2Cwrite(List<String> argumentsList)
        {
            XI2Ctransfer(argumentsList);
        }
        public void XI2Cread(List<String> argumentsList)
        {
            XI2Ctransfer(argumentsList);
        }

        // to WRITE, the Xilinx master will receive a separate buffer of "opcodes" (if any)
        // and "data" bytes to write, to facilitate ReStart Write->Read I2C direction change
        // TxBufferLen is returned - from that number we can determine if all bytes
        // (opcode + data) were written, and if not at what byte index it failed
        // Note that hardware characteristics of I2C slaves very GREATLY
        // from relatively simply MUX slave to Complex Lattice CPLD
        // And ThorDAQ EEPROMs, for instance, can handle sequential 16-byte writes,
        // so the 28 byte write requires 2 I2C write sequences
        public void XI2Ctransfer(List<String> argumentsList)
        {
            THORDAQ_STATUS status = THORDAQ_STATUS.STATUS_SUCCESSFUL;
            UInt32 MasterMUXAddr = 0x71; // coded in hardware
            UInt32 MasterMUXChan = 0;
            Int32 SlaveMUXAddr = 0x70; // coded in hardware
            UInt32 SlaveMUXChan = 0xFF; // default NO slave MUX
            UInt32 DeviceAddr;
            UInt32 TransferBufferLen = 0; // read or write
            UInt32 OpcodeByteLen = 0; // 
            Int32 PageReadWriteLen = 16; // default for EEPROM
            bool bI2C_ReadDir = false;
            int ArgIndx = 4;  // i.e.  -o or -n;
            UInt32 I2CbusHz = 400; // DEFAULT!

            // required arg list...
            if (argumentsList.Count >= 4) // 
            {
                MasterMUXChan = (UInt32)new System.ComponentModel.UInt32Converter().ConvertFromString(argumentsList[1]);
                SlaveMUXChan = (UInt32)new System.ComponentModel.UInt32Converter().ConvertFromString(argumentsList[2]);
                DeviceAddr = (UInt32)new System.ComponentModel.UInt32Converter().ConvertFromString(argumentsList[3]);
            }
            else
            {
                UpdateConsoleStatus("Missing args: " + CommonDefinition.HELP_XI2CReadWrite_MSSAGE);
                return;
            }

            // 

            // 
            byte[] OpcodeBytes = new byte[8]; // max opcode count by ThorDAQ definition
            byte[] DataBytes = new byte[64]; // max DATA count by ThorDAQ definition
            IntPtr UnManagedOpCodeBuffer = Marshal.AllocHGlobal(OpcodeBytes.Length);
            IntPtr UnManagedDataBuffer = Marshal.AllocHGlobal(DataBytes.Length);
            uint i;
            while (ArgIndx < argumentsList.Count)
            {
                switch (argumentsList[ArgIndx])
                {
                    case "-s": // slower 100 kHz speed
                        I2CbusHz = 100;
                        break;
                    case "-p":
                        PageReadWriteLen = (Int32)new System.ComponentModel.Int32Converter().ConvertFromString(argumentsList[++ArgIndx]);
                        break;
                    case "-n":  // number of bytes we expect to READ
                        bI2C_ReadDir = true;
                        TransferBufferLen = (uint)new System.ComponentModel.UInt32Converter().ConvertFromString(argumentsList[++ArgIndx]);
                        break;
                    case "-d": // count all DATA bytes
                        bI2C_ReadDir = false;
                        ArgIndx++;
                        i = 0;
                        while (ArgIndx < argumentsList.Count)
                        {
                            // DATA includes OpCode(s)
                            DataBytes[i] = (byte)new System.ComponentModel.ByteConverter().ConvertFromString(argumentsList[ArgIndx]);
                            System.Runtime.InteropServices.Marshal.WriteByte(UnManagedDataBuffer, (int)i, DataBytes[i]);
                            ArgIndx++; i++;
                        }
                        TransferBufferLen = i;
                        break;

                    case "-o":
                        //bI2C_ReadDir = true;
                        ArgIndx++;
                        i = 0;
                        while (ArgIndx < argumentsList.Count && argumentsList[ArgIndx].Substring(0, 1) != "-")
                        {
                            OpcodeBytes[i] = (byte)new System.ComponentModel.ByteConverter().ConvertFromString(argumentsList[ArgIndx]);
                            System.Runtime.InteropServices.Marshal.WriteByte(UnManagedOpCodeBuffer, (int)i, OpcodeBytes[i]);
                            ArgIndx++; i++;
                        }
                        OpcodeByteLen = i;
                        ArgIndx--; // back up one Arg (in case -n follows -o)
                        break;

                    default:
                        break;
                };
                ArgIndx++; // next arg
            }
            status = THORDAQ_STATUS.STATUS_SUCCESSFUL;
            uint TotalBytesTransfered = 0;  // for diagnostic/error check

            if (bI2C_ReadDir == false)  // WRITE case 
            {
                status = (THORDAQ_STATUS)ThorDAQAPIXI2CReadWrite(MasterBoardIndex, bI2C_ReadDir, MasterMUXAddr, MasterMUXChan,
                                                                                SlaveMUXAddr, SlaveMUXChan, DeviceAddr, I2CbusHz, PageReadWriteLen,
                                                                                UnManagedOpCodeBuffer, ref OpcodeByteLen,  // Opcode(s) means READ operation
                                                                                UnManagedDataBuffer, ref TransferBufferLen);
                TotalBytesTransfered += TransferBufferLen;
            }
            else      // READ case
            {
                status = (THORDAQ_STATUS)ThorDAQAPIXI2CReadWrite(MasterBoardIndex, bI2C_ReadDir, MasterMUXAddr, MasterMUXChan,
                                                                            SlaveMUXAddr, SlaveMUXChan, DeviceAddr, I2CbusHz, PageReadWriteLen,
                                                                            UnManagedOpCodeBuffer, ref OpcodeByteLen,  // Opcode(s) means READ operation
                                                                            UnManagedDataBuffer, ref TransferBufferLen); // ret. value includes OpCode len
                TotalBytesTransfered += TransferBufferLen;
            }
            if (status != THORDAQ_STATUS.STATUS_SUCCESSFUL)
            {
                string sStatus = "I2C NAK - no device, check connection";
                if (status == THORDAQ_STATUS.STATUS_I2C_TIMEOUT_ERR)
                    sStatus = "I2C Timeout";
                UpdateConsoleStatus(" ThorDAQAPIXI2CReadWrite() I2C write failure " + sStatus + " [bytes transfered?:] " + TotalBytesTransfered.ToString());
            }

            Marshal.FreeHGlobal(UnManagedOpCodeBuffer);
            Marshal.FreeHGlobal(UnManagedDataBuffer);
        }


        private uint APIprogram3P_MCUappFirmware(string fileName, bool bOriginal, bool bForce8000Hex)
        {
            APIupdateLFT_MCUFirmware(fileName, bOriginal, bForce8000Hex);
            return (0);
        }


        public void APIupdateLFT_CPLD(List<String> argumentsList)
        {
            string fileSpec = "notSpecified.jed"; // (not expected)
            Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();
            dlg.DefaultExt = ".jed";
            dlg.Filter = "Xilinx JEDEC Files (.jed)|*.jed";
            // Display OpenFileDialog by calling ShowDialog method
            Nullable<bool> bResult = dlg.ShowDialog();
            // Get the selected file name and display in a TextBox

            if (bResult == true)
            {
                // Open document
                fileSpec = dlg.FileName;
            }

            APIupdateLFT_CPLDFirmware(fileSpec);
        }

        // ProgrammableTrigger added in FPGA for 5.0 and above
        public void APIProgrammableTrigger(List<String> argumentsList)
        {
            THORDAQ_STATUS status;
            // "Default" is assert ImageAcqTrigger
            SByte Chan = -1;
            SByte Assert = 1;

            for (int argNum = 1; argNum < argumentsList.Count; argNum++)
            {
                // first arg is channel, second arg BOOL to Assert/Deassert
                switch (argNum)
                {
                    case 1: //hereeee
                        Chan = (SByte)Int16.Parse(argumentsList[argNum]);
                        break;
                    case 2:
                        Assert = (SByte)Int16.Parse(argumentsList[argNum]);
                        break;

                }
            }
            status = ThorDAQAPIProgrammableTrigger(MasterBoardIndex, Chan, Assert);
            if (status != 0)
                UpdateConsoleStatus("ERROR status: " + status.ToString());
        }
        
        public void UpdateLFT_AppFirmware(List<String> argumentsList)
        {
            Nullable<bool> result;
            string fileName = null;
            bool bOriginalProgram = false;  // default is we are loading NEW flash program
            bool bForce8000Hex = false;
            bool bUpdateCPLD = false;

            for (int argNum = 1; argNum < argumentsList.Count; argNum++)
            {
                switch (argumentsList[argNum])
                {
                    case "-C":
                        bUpdateCPLD = true;
                        break;

                    case "-o":
                        bOriginalProgram = true;
                        break;
                    case "-F":
                        bForce8000Hex = true;  // forces new Bootloader to accept legacy App FW code @ 0x8000
                        break;

                    case "-f": // must have next arg as filename

                        fileName = argumentsList[argNum + 1];
                        break;
                    default:
                        break;
                }
            }
            if (fileName == null) // Dialog box for filespec
            {
                if (bOriginalProgram)
                {
                    fileName = @"c:\temp\CLKRX_3Pat2000.hex";
                }
                else
                {
                    //Create OpenFileDialog
                    string sFileExtension = ".hex";
                    string sFileFilter = "Intel HEX Files (.hex)|*.hex";
                    if (bUpdateCPLD)
                    {
                        sFileExtension = ".jed";
                        sFileFilter = "Xilinx JEDEC Files (.jed)|*.jed";
                    }

                    Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();
                    dlg.DefaultExt = sFileExtension;
                    dlg.Filter = sFileFilter;
                    // Display OpenFileDialog by calling ShowDialog method
                    result = dlg.ShowDialog();
                    // Get the selected file name and display in a TextBox

                    if (result == true)
                    {
                        // Open document
                        fileName = dlg.FileName;
                    }
                }
            }
            if (bUpdateCPLD)
            {
                APIupdateLFT_CPLDFirmware(fileName);
            }
            else
            {
                APIprogram3P_MCUappFirmware(fileName, bOriginalProgram, bForce8000Hex);
            }

        }



        //                    e.g.  arg1 arg2  arg3
        // usage:  UpdateCentralFPGAfirmware [-g] [-f <filespec>]
        public void UpdateCentralFPGAfirmware(List<String> argumentsList)
        {
            Nullable<bool> result;
            string fileName = null;
            bool bProduction = true;  // if no command line arg, assume production

            for (int argNum = 1; argNum < argumentsList.Count; argNum++)
            {
                switch (argumentsList[argNum])
                {
                    case "-g":
                        bProduction = false;
                        break;
                    case "-f": // must have next arg as filename

                        fileName = argumentsList[argNum + 1];
                        break;
                    default:
                        break;
                }
            }
            if (fileName == null) // Dialog box for filespec
            {
                //Create OpenFileDialog
                Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();
                dlg.DefaultExt = ".mcs";
                dlg.Filter = "MCS Files (.mcs)|*.mcs";
                // Display OpenFileDialog by calling ShowDialog method
                result = dlg.ShowDialog();
                // Get the selected file name and display in a TextBox

                if (result == true)
                {
                    // Open document
                    fileName = dlg.FileName;
                }
            }

            ProgramFPGAFlash(fileName, bProduction);
        }

        public void APIupdateBOB_CPLD(List<String> argumentsList)
        {
            //Create OpenFileDialog
            Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();
            dlg.DefaultExt = ".jed";
            dlg.Filter = "JED Files (.jed)|*.jed";

            // Display OpenFileDialog by calling ShowDialog method
            Nullable<bool> result = dlg.ShowDialog();
            string fileName;
            // Get the selected file name and display in a TextBox
            if (result == true)
            {
                // Open document
                fileName = dlg.FileName;

                // Identify the CPLD component on the I2C network board (on DAC mez. card)
                const uint MasterMUXChan = 0x2;
                const uint SlaveMUXChan = 0xff;
                const uint TargetSlaveAddr = 0x44; // CPLD address on BOB

                API_ProgramLatticeCPLD(fileName, MasterMUXChan, SlaveMUXChan, TargetSlaveAddr);
                dlg.Reset();
            }
        }


        public void APIUpdateDAC_CPLD()
        {
            //Create OpenFileDialog
            Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();
            dlg.DefaultExt = ".jed";
            dlg.Filter = "JED Files (.jed)|*.jed";

            // Display OpenFileDialog by calling ShowDialog method
            Nullable<bool> result = dlg.ShowDialog();

            // Get the selected file name and display in a TextBox
            if (result == true)
            {
                // Open document
                string fileName = dlg.FileName;

                // Identify the CPLD component on the I2C network board (on DAC mez. card)
                const uint MasterMUXChan = 0x8;
                const uint SlaveMUXChan = 0xff;
                const uint TargetSlaveAddr = 0x40; // CPLD address on DAC board

                API_ProgramLatticeCPLD(fileName, MasterMUXChan, SlaveMUXChan, TargetSlaveAddr);
            }
        }

        public void UpdateCPLD()
        {
            //Create OpenFileDialog
            Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();
            dlg.DefaultExt = ".jed";
            dlg.Filter = "JED Files (.jed)|*.jed";

            // Display OpenFileDialog by calling ShowDialog method
            Nullable<bool> result = dlg.ShowDialog();

            // Get the selected file name and display in a TextBox
            if (result == true)
            {
                // Open document
                string fileName = dlg.FileName;
                ProgramCPLD(fileName);
                //ProgramCPLD(fileName);
            }
        }



        public void WriteBoardMemoryData(List<String> argumentsList)
        {
            IntPtr buffer = IntPtr.Zero;

            if (argumentsList.Count < 4)
            {
                UpdateConsoleStatus(CommonDefinition.HELP_WriteMem_MSSAGE);
                return;
            }
            try
            {
                UInt16 value = (UInt16)Convert.ToInt32(argumentsList[1]);
                //    if (value <= MAX_BAR_INDEX && value >= MIN_BAR_INDEX)
                {
                    UInt32 barNum = value; // get bar number
                    int registerAddress = (int)new System.ComponentModel.Int32Converter().ConvertFromString(argumentsList[2]);
                    int NumBytesToWrite = (int)new System.ComponentModel.Int32Converter().ConvertFromString(argumentsList[3]);
                    // 2 Chars per 8-bit value, so 4 bytes means 8 chars were passed
                    if (buffer == IntPtr.Zero)
                    {
                        Win32.AllocateBuffer(ref buffer, (uint)NumBytesToWrite);
                    }
                    string buffer_hexString = argumentsList[4];
                    byte byteValue;
                    // 
                    buffer_hexString = buffer_hexString.Replace("0x", string.Empty);
                    // if there is a SINGLE hex data byte, copy that single value instead of processing string of bytes
                    if ((buffer_hexString.Length == 2))
                    {
                        byteValue = byte.Parse(buffer_hexString, NumberStyles.HexNumber, CultureInfo.InvariantCulture);
                        for (int j = 0; j < NumBytesToWrite; j++)
                            System.Runtime.InteropServices.Marshal.WriteByte(buffer, j, byteValue);
                    }
                    else
                    {
                        for (int i = 0, j = 0; j < NumBytesToWrite; i += 2, j++)
                        {
                            byteValue = byte.Parse(buffer_hexString.Substring(i, 2), NumberStyles.HexNumber, CultureInfo.InvariantCulture);
                            System.Runtime.InteropServices.Marshal.WriteByte(buffer, j, byteValue);
                        }
                    }
                    string sStatus = ThorDAQAdapters[MasterBoardIndex].BAR.Write(barNum, (ulong)registerAddress, buffer, (ulong)NumBytesToWrite);
                    if (sStatus != "OK")
                    {
                        UpdateConsoleStatus(sStatus);
                    }
                    if (buffer != IntPtr.Zero)
                        Win32.FreeBuffer(buffer);
                }
            }
            catch (Exception)
            {
                if (buffer != IntPtr.Zero)
                {
                    Win32.FreeBuffer(buffer);
                }
                UpdateConsoleStatus("Writemem Error");
            }
        }

        public void BuildWaveform(List<String> argumentsList)
        {
            List<String> DACsamples = new List<String>();
            int uiDACmaxCounts = 65536 - 1;
            int uiVoltsRange = 20;
            int lowAmplitude = 0;
            int highAmplitude = uiDACmaxCounts;
            int centeredAmplitude = -1; // amplitude centered around 32768
            int frequency = 14; // cycles/sec
            double totalPlaybackTime = 1.0; // sec
            int playbackFreq = 1000000; // samples/sec
            string fileSpec = @"c:\temp\Test_Wave.txt";
            Int32 DutyCycle = 48; // percent per period that digital signal is high
            bool bDOwave = false;
            string DOwaveFileSpec = @"c:\temp\DAC_DOtest_wave.txt";
            bool bVoltsUnits = false;
            int arg = 1;
            int argCount = argumentsList.Count;
            // parse args...
            while (arg < argCount)
            {
                switch (argumentsList[arg])
                {
                    case "-D":
                        bDOwave = true;
                        break;
                    case "-d":  // digital wave duty cycle, 6 - 96 percent
                        Int32.TryParse(argumentsList[++arg], out DutyCycle);
                     
                        break;

                    case "-C":
                        Int32.TryParse(argumentsList[arg + 1], out uiDACmaxCounts);
                        uiDACmaxCounts -= 1; // inclusive of "0" counts
                        arg++; // next flag
                        break;
                    case "-V":
                        Int32.TryParse(argumentsList[arg + 1], out uiVoltsRange);
                        arg++; // next flag
                        break;
                    case "-v":
                        bVoltsUnits = true;
                        break;
                    case "-a":
                        Int32.TryParse(argumentsList[arg + 1], out centeredAmplitude);
                        arg++; // next flag
                        break;

                    case "-l":  // low amplitude
                        Int32.TryParse(argumentsList[arg + 1], out lowAmplitude);
                        arg++; // next flag
                        break;
                    case "-h":
                        Int32.TryParse(argumentsList[arg + 1], out highAmplitude);
                        arg++; // next flag
                        break;
                    case "-f":
                        fileSpec = argumentsList[arg + 1];
                        arg++; // next flag
                        break;
                    case "-r":  // frequency
                        Int32.TryParse(argumentsList[arg + 1], out frequency);
                        arg++; // next flag
                        break;
                    case "-t":  // frequency
                        Double.TryParse(argumentsList[arg + 1], out totalPlaybackTime);
                        arg++; // next flag
                        break;
                    case "-s":  // sample-playback frequency
                        Int32.TryParse(argumentsList[arg + 1], out playbackFreq);
                        arg++; // next flag
                        break;
                }
                ++arg;
            };
            if (bVoltsUnits == true)
            {
                fileSpec += ".Volts";
            }

            ulong totalSamples = (ulong)(totalPlaybackTime * (double)playbackFreq);
            if (bDOwave)  // we do analog OR digital file in one call of this function
            {
                ulong fileSampleCount = 0;  // break when totalSamples expected are written to file
                Int64 DOSample; // two 8-bit fields per sample
                UpdateConsoleStatus("Writing file " + DOwaveFileSpec + " ...");

                int samplesPerCycle = playbackFreq / frequency;
                double totalSamplesInFile = playbackFreq * totalPlaybackTime;
                double NumberSamplesHighInPeriod = samplesPerCycle * (double)(DutyCycle) / 100.0;
                int i, DOhigh;
                // offset the assertion of bits 1-7 at beginning of period...
                do
                {
                    for (i = 0, DOhigh = (int)NumberSamplesHighInPeriod; (i < samplesPerCycle) && (fileSampleCount < totalSamples);
                         i++, DOhigh--, fileSampleCount++)
                    {
                        if(DOhigh > 0)  // DO waveform HIGH this sample
                        {
                            DOSample = 0xFFFF;
                        }
                        else // DO waveform LOW this sample
                        {
                            DOSample = 0;
                        }
                        // write sample to list
                        DACsamples.Add(string.Format("{0}", DOSample.ToString()));
                    }
                } while ((totalSamplesInFile -= samplesPerCycle) > 0 && (fileSampleCount < totalSamples));

                File.WriteAllLines(DOwaveFileSpec, DACsamples, Encoding.UTF8);
                return;
            }
            else
            {
                UpdateConsoleStatus("Writing file " + fileSpec + " ...");
            }
            double period;
            int amplitude;
            int MidPoint;
            if (centeredAmplitude != -1) // user provided "Amplitude" arg?
            {
                //                            int uiDACmaxCounts = 65536 - 1;
                //                          int uiVoltsRange = 20;
                highAmplitude = (uiDACmaxCounts + 1) / 2 + (centeredAmplitude / 2);
                lowAmplitude = (uiDACmaxCounts + 1) / 2 - (centeredAmplitude / 2);
            }
            amplitude = highAmplitude - lowAmplitude;
            MidPoint = (highAmplitude - lowAmplitude) / 2 + lowAmplitude;

            if (frequency > 0)
                period = 1.0 / (double)frequency;
            else
            {
                UpdateConsoleStatus("Param Error: frequency must be > 0");
                return;
            }

            ulong samplesPerPeriod = (ulong)(period * playbackFreq);
            ulong sampleCount = 0;
            // TEST!!!
            //samplesPerPeriod = 32;
            double PIdivisor = (2 * Math.PI) / (double)samplesPerPeriod;
            // The FPGA hardware expects the waveform to start at its lowest value (i.e. "0" DAC counts),
            // because the "Park" value is also expected to be low (like 0 DAC counts, -10 VDC)
            // "0" DAC Counts in waveform places the waveform at the "Offset" voltage setting.
            // The FPGA intends to gradually increase (in ~ 1-2 microsecs) the DAC from
            // park voltage to the "offset" voltage (defined and changeable outside the waveform defintion).
            // If the waveform does not start (and end) with "0", there will be a step
            // voltage back to the "offset" DAC value which will likely "trip" a Gavlo
            // servo controller
            double PIangleRadians = 3 * Math.PI / 2;
            Int64 ASample;
            do
            {
                for (; sampleCount < totalSamples; sampleCount++, PIangleRadians += PIdivisor)
                {
                    double SignedRatio = Math.Sin(PIangleRadians);
                    if (SignedRatio > 0)
                    {
                        double AboveDelta = (highAmplitude - MidPoint) * SignedRatio;
                        ASample = MidPoint + (int)(AboveDelta);
                    }
                    else if (SignedRatio < 0)
                    {
                        double BelowDelta = (MidPoint - lowAmplitude) * SignedRatio;
                        ASample = MidPoint + (int)(BelowDelta);
                    }
                    else
                        ASample = MidPoint;
                    // write it to list
                    DACsamples.Add(string.Format("{0}", ASample.ToString()));
                }
            } while (sampleCount < totalSamples);

            // convert to Volts?
            if (bVoltsUnits == true) // write VoltsDC list to file
            {
                System.IO.StreamWriter VoltsFile = null;
                List<String> dVoltsSamples = new List<String> { };
                double dVolts = 0.0;
                try
                {
                    using (VoltsFile = new System.IO.StreamWriter(fileSpec))
                    {
                        foreach (string sSample in DACsamples)
                        {
                            UInt16 DACcnts = UInt16.Parse(sSample);
                            // will voltage be positive or negative?

                            dVolts = (((double)uiVoltsRange) / ((double)(uiDACmaxCounts + 1)) * (DACcnts)) - ((double)uiVoltsRange / 2.0);
                            dVoltsSamples.Add(string.Format("{0}", dVolts.ToString()));


                        }
                        // now join all the Volt samples into a single line separated by ','
                        // this is the easiest format for NI Labview processing
                        String LineOfAllVoltsCSV;
                        LineOfAllVoltsCSV = String.Join(",", dVoltsSamples);
                        VoltsFile.WriteLine(LineOfAllVoltsCSV);
                    }
                }
                catch (Exception e)
                {
                    UpdateConsoleStatus(string.Format("Exception writing {0}: {1}", fileSpec, e.Message));
                }
                finally
                {
                    if (VoltsFile != null)
                    {
                        VoltsFile.Close();
                    }
                }
            }
            else  // write DAC-counts list to file
            {
                File.WriteAllLines(fileSpec, DACsamples, Encoding.UTF8);
            }
        }
    }
}
