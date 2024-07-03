// Manufacturing Tests for TSI (Austin) for verifying ThorDAQ boards and Breakout Boxes returning from
// assembly producers.
// Copyright (c) Thorlabs Inc, Nov. 2020
// DZimmerman@thorlabs.com
// If two ThorDAQ boards are present, aim to make UUT (Unit Under Test) Board 0, golden is Board 1...

namespace thordaqGUI
{
    using System;
    using System.Windows;
    using System.ComponentModel;
    using System.Collections.Generic;
    using System.Linq;
    using tdDFT;
    using System.Net.Sockets;
    using System.Net;
    using System.Text;
    using System.Threading;
    using System.Threading.Tasks;
    using System.IO;
    using System.Globalization;
    using System.Runtime.InteropServices;
    using ThorSharedTypes;
    using NWLogic.DeviceLib;
    using System.Security.Permissions;
    using ThorLogging;
    using System.Diagnostics;
    using Excel = Microsoft.Office.Interop.Excel; // requires VS2019 "Office/Sharepoint Development" install option
    using System.Reflection;
    using TIDP.SAA;

    public partial class MainWindow : Window, INotifyPropertyChanged
    {

        // Manufacturing Tests using the National Instruments PXIe-1071
        const string HELP_ManufTest_MSSAGE = @"ManufTest <-t TestCode> <-a> <-u>
Options:
(NOTE: initial capital letters mean issue single TCP/IP CMD:, then exit - for TCP debug):
  -B  BOARDSN (Get Board serial numbers) 
  -T  1 UnitTester (REQUIRED test number & description)
  -P  POLLING
  -F  SENDFILE (Get Labview data .csv file)
General options:
  -a IPaddress of Test Computer (e.g. NIPXIe1071), (def. 10.28.10.191)
  -d Drive file path for storage of ManufTest Excel files (def. T:\ must end with \)
  -p IP Port number of Test Computer (def. 80)
  -r Generate .XLS format report file
  -t TestCode, 0 for all, 1 for UnitTest, 2 for Exemplar Waveform, 3 for DAC, 4 for ADC,
       5 for TRIG3P, 6 for DBB1, 7-9 for ABB1-3
  -I <n> Iterations of 'n' count for test
  -E Exit on first error (in Iterations case)
  -TDboard     Read/Test Texas Instruments UDC9248 Power ICs on ThorDAQ; requires TI USB Adapter
  -DACcard <n> Test ABBx sine waveform on channel index 'n', e.g. '-DACcard 11' tests AO11 (ABB3)
  -ADCcard <n> <-I c> Test sine waveform on PMT channel index 'n', e.g. '-ADCcard 1' tests Chan B
                 -I 20 Does 20 iterations (all channels, or only 'n' channel if presented)
  -DBB1 <t> <n>  Test the 32 Digital I/O ('t' is 'o' for Output, 'i' for Input, 
                 'm' for MUXing, def. is both 'oi'), n is iterations
  -ABBx <v>    Test breakout box 'x', 1 2 or 3 ('-ABB3 6' tests A6-13 ADC with +/- 6 Volts)  
  -TRIGcard <F> TDQ2-n 3P (or SAMD21 App); option F is LF1 frequency in Hz; default is to
                test range of 4 frequencies (Requires Digilent Analog Discover2 USB Device)
  -all          Do ALL available manufacturing tests
"
    ;

        // Since different components require appropriate report header formatting
        // prepare a string for writing those, and use a constant max-column setting
        const int MAXColumnsINREPORT = 16;

        // Establish globals for use with all test classes
        String AllTDSerialNumbers; // concatenated I2C available serialnumbers from all ThorDAQ components
        static ManufTestReport TestReport = null;
        static string sIPaddress; // e.g. a 32-bit dotted quad
        static string sNI1017_IPhostNameVirginia = "STL-NIPXIe1071";  // NI1017 IP hostname (Virginia)
        static string sNI1017_IPhostNameAustin = "NI1071-TX";  //  (Austin)
        static string ManufTest_DataDRIVE = @"V:\Operations\Quality\Production QC Reports\ThorDAQ Test Files\"; // default - can be overwritten by CL option; if failed default to c:\temp\

        static IPAddress NI1071_IP_Addr; // = IPAddress.Parse(sIPaddress);
        static int IP_port = 80; // pick something (e.g. HTTP) MS Win10 firewall doesn't habitually close after O/S update
                                 // i.e. \temp\ManufTest\ if no Spreadsheet report, otherwise add TDserNum_date, e.g. \temp\ManufTest\TD2019_2021-2-2
        static string ManufTest_Data_RootFilepath = ManufTest_DataDRIVE + @"ManufTestReports\"; // the hard-coded default - may be replaced by persistent file setting
        // if the "persistent MT report file" exists, use that as the default


        // Create the Digilent 2 scope/generator object, required for pulse driving LF1 on the 3P 
        // mezzanine card.
        // 2 channels, with labels "W1" and "W2" (numeric 0 and 1) are available
        // dwfercUnknownError 1 Call waiting on pending API time out.
        // dwfercApiLockTimeout 2 Call waiting on pending API time out.
        // dwfercAlreadyOpened 3 Device already opened.
        // dwfercNotSupported 4 Device not supported.
        // dwfercInvalidParameter0 0x10 Parameter 0 was invalid in last API call.
        // dwfercInvalidParameter1 0x11 Parameter 1 was invalid in last API call.
        // dwfercInvalidParameter2 0x12 Parameter 2 was invalid in last API call.
        // dwfercInvalidParameter3 0x13 Parameter 3 was invalid in last API call.
        public class Digilent2
        {
            private int _iHandle = -1;

            // CONSTRUCTOR
            public Digilent2()
            {
                // if the USB device is attached we should get a handle
                int iStatus = dwf.FDwfDeviceOpen(-1, out _iHandle);
                if (iStatus == 0)
                    _iHandle = -1; // failed Open
                return;
            }
            ~Digilent2()
            {
                if (_iHandle != -1)
                    dwf.FDwfDeviceClose(_iHandle);
            }
            public bool DigilentFound()
            {
                if (_iHandle == -1)
                    return false;
                return true;
            }
            public void CloseDigilent()
            {
                if (_iHandle != -1)
                    dwf.FDwfDeviceClose(_iHandle);
            }

            public string ConfigureWaveform(int iChan, byte WaveType, double Amplitude, double DutyCycle, double FreqInHZ)
            {
                if (_iHandle == -1)
                {
                    return "FAILED: Invalid Handle - Digilent USB device not found";
                }
                string sStatus = "SUCCESS";
                int iStatus;

                iStatus = dwf.FDwfAnalogOutNodeEnableSet(_iHandle, iChan, dwf.AnalogOutNodeCarrier, 1); // 1 is Enable
                if (iStatus == 0) // zero ("false") indicates error in the API
                {
                    dwf.FDwfGetLastErrorMsg(out sStatus);
                    return sStatus;
                }
                // Set the wave type
                iStatus = dwf.FDwfAnalogOutNodeFunctionSet(_iHandle, iChan, dwf.AnalogOutNodeCarrier, WaveType);  // e.g. dwf.funcSquare
                // and Frequency
                if (iStatus == 0)
                {
                    dwf.FDwfGetLastErrorMsg(out sStatus);
                    return sStatus;
                }
                iStatus |= dwf.FDwfAnalogOutNodeFrequencySet(_iHandle, iChan, dwf.AnalogOutNodeCarrier, FreqInHZ);
                if (iStatus == 0)
                {
                    dwf.FDwfGetLastErrorMsg(out sStatus);
                    return sStatus;
                }
                // and amplitude & offset
                iStatus |= dwf.FDwfAnalogOutNodeAmplitudeSet(_iHandle, dwf.AnalogOutNodeCarrier, iChan, Amplitude);
                if (iStatus == 0)
                {
                    dwf.FDwfGetLastErrorMsg(out sStatus);
                    return sStatus;
                }
                iStatus |= dwf.FDwfAnalogOutNodeOffsetSet(_iHandle, dwf.AnalogOutNodeCarrier, iChan, 0.0);
                if (iStatus == 0)
                {
                    dwf.FDwfGetLastErrorMsg(out sStatus);
                    return sStatus;
                }

                iStatus |= dwf.FDwfAnalogOutNodeSymmetrySet(_iHandle, iChan, dwf.AnalogOutNodeCarrier, DutyCycle);
                if (iStatus == 0)
                {
                    dwf.FDwfGetLastErrorMsg(out sStatus);
                    return sStatus;
                }

                return sStatus;
            }
            public string StartWaveform(int iChan, bool bStart)
            {
                string sStatus = "SUCCESS";
                int iStatus;

                if (_iHandle == -1)
                {
                    return "FAILED: can't start/stop - invalid handle";
                }
                // on stop (iStart == 0), the configuration is lost
                if (bStart)
                    iStatus = dwf.FDwfAnalogOutConfigure(_iHandle, iChan, 1);
                else
                    iStatus = dwf.FDwfDeviceClose(_iHandle);

                return sStatus;
            }
        }


        // HATE to do this, but Legacy architecture forces duplication of code 
        public static List<String> MTAPIGetDIO(List<String> argumentsList) // no ExeName arg[0]
        {
            THORDAQ_STATUS status = 0;
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
            if (argumentsList.Count < 1)
            {
                // error - reading nothing
                if (UnManagedConfigBuffer != IntPtr.Zero) Marshal.FreeHGlobal(UnManagedConfigBuffer); // release our temp buffer
                return strDIOlist;
            }
            string sNum;
            int k;
            int i, j;

            try
            {
                // special case of getting them all...
                if (argumentsList[0] == "D99")
                {
                    for (k = 0; k < 32; k++)  //  3U Panel BOB
                    {
                        DO_BNCindex = String.Format("D{0,2:D2}Xxx      ", k); // 'D' plus two chars, "xx" is default Unknown
                        NumberOfBNCsToGET++;
                        strBNCindex_Value.Add(DO_BNCindex);
                    }
                }
                else
                {
                    // process Dn one at a time...
                    for (int argNum = 0; argNum < argumentsList.Count;)
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
                    if (UnManagedConfigBuffer != IntPtr.Zero)
                    {
                        Marshal.FreeHGlobal(UnManagedConfigBuffer); // release our temp buffer
                        UnManagedConfigBuffer = IntPtr.Zero;
                    }
                    return strDIOlist;
                }
                else // get the DIO values
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
                }
            }
            catch (ArgumentException e)
            {
                strDIOlist.Add(String.Format("ERROR: {0}", e.Message));
            }
            finally
            {
                if (UnManagedConfigBuffer != IntPtr.Zero)
                    Marshal.FreeHGlobal(UnManagedConfigBuffer); // release our temp buffer
            }
            return strDIOlist;
        }

        public static bool MTAPISetDO(List<String> argumentsList)  // min. 2 args, e.g. "D13" "1" sets BNC D13
        {
            IntPtr UnManagedConfigBuffer = IntPtr.Zero;
            // a legal "field" is a BNC index. e.g. "D31", followed by "0" or "1" value to set
            UInt32 NumberOfBNCsToSet = 0;
            Int32 iNum;
            string DO_BNCandValue;
            List<String> strBNCindex_Value = new List<string> { };
            if (argumentsList.Count < 2)
            {
                return false;
            }
            try
            {
                // process args 2 at a time, e.g. "D3 1 D7 0", set BNC D3 to 1, next then iteration, D7 to 0
                for (int argNum = 0; argNum < argumentsList.Count;)
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
                    return false;
                }
            }
            catch
            {
                return false;
            }
            finally
            {
                if (UnManagedConfigBuffer != IntPtr.Zero)
                    Marshal.FreeHGlobal(UnManagedConfigBuffer); // release our temp buffer
            }
            return true;
        }
        // HATE to do this, but Legacy architecture forces duplication of code 
        public static List<string> MTAPISetDIOConfig(List<String> argumentsList)
        {
            List<string> RetStrings = new List<string> { };
            string strDIOconfig;
            int iBNClabel;
            Int32 iNum;
            Int32 iOutputSrc = 0; // (unused for Input)
            Int32 iMUXcode = 48;  // def. CPLD configured (3U BOB)
            Int32 iAUXcode = -1;  // needed only for AUX_GPIO in FPGA

            // e.g., arg list "31" "o" "31" "48"  (BNC 31, output, indentical source index, CPLD controlled)
            if (argumentsList.Count < 2) // 
            {
                RetStrings.Add(HELP_APIGetSetDIOConfig_MSSAGE);
                return RetStrings;
            }

            string sDIR = argumentsList[1].ToUpper();
            if (sDIR != "I" && sDIR != "O") // Only options
            {
                RetStrings.Add(HELP_APIGetSetDIOConfig_MSSAGE);
                return RetStrings;
            }

            iNum = (Int32)new System.ComponentModel.Int32Converter().ConvertFromString(argumentsList[0]);
            iBNClabel = iNum;
            strDIOconfig = String.Format("D{0,2:D2}", iNum); // 'D' plus two chars
            strDIOconfig += sDIR; // single DIR char

            if (argumentsList.Count >= 3)  // 3rd arg is CpySrc
            {
                iOutputSrc = (Int32)new System.ComponentModel.Int32Converter().ConvertFromString(argumentsList[2]);
                strDIOconfig += String.Format("{0,2:D2}", iOutputSrc);
            }
            else
            {
                strDIOconfig += String.Format("{0,2:D2}", iNum);  // CpySrc default is always the Dn index itself
            }

            if (argumentsList.Count >= 4)  // 4th arg is MUX code
            {
                iMUXcode = (Int32)new System.ComponentModel.Int32Converter().ConvertFromString(argumentsList[3]);
            }
            strDIOconfig += String.Format("M{0,2:D2}", iMUXcode); // arg or the default

            if (argumentsList.Count >= 5)  // 5th arg is AUX index for FPGA's AUX_GPIO
            {
                iAUXcode = (Int32)new System.ComponentModel.Int32Converter().ConvertFromString(argumentsList[4]);
            }
            strDIOconfig += String.Format("A{0,2:D2}", iAUXcode); // arg or the default

            // check for TEST case!!
            if (iBNClabel >= 99)
            {
                bool bOutputs = (sDIR == "O") ? true : false;
                bool bStatus = TestDIOs(bOutputs, iBNClabel);  // recursive function (calls this function with separate BNC indexes)
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


        public static IPAddress DoGetHostEntry(string hostname)
        {
            string eMsg;
            IPAddress nonExistentHostAddr = IPAddress.Parse("0.0.0.0");
            IPHostEntry host;
            try
            {
                host = Dns.GetHostEntry(hostname);
                return host.AddressList[0];
            }
            catch (Exception e)
            {
                eMsg = e.Message;

            }
            return nonExistentHostAddr;
        }
        // For REPORTs of Manufacturing Tests


        public class ManufTestReport
        {
            private string _FileSpec = null;
            private int _ReTestFailRow;
            string _FileSubDir, _FileNamePrefix;
            const string _FileNameExt = ".xlsx";
            string _FileNamePassFail = "_PASS";
            private Excel.Application _oXL;
            Excel._Workbook _oWB = null;
            Excel._Worksheet oTDboardWS = null;
            Excel._Worksheet oDACcardWS = null;
            Excel._Worksheet oADCcardWS = null;
            Excel._Worksheet oTRIGcardWS = null;
            Excel._Worksheet oDBB1WS = null; // breakout box
            Excel._Worksheet oABB1WS = null; // breakout box
            Excel._Worksheet oABB2WS = null; // breakout box  
            Excel._Worksheet oABB3WS = null; // breakout box
            public Excel._Worksheet oReTestWS = null; // quick user reference, retest list

            Excel._Worksheet _oCurrentWorksheet = null;

            // CONSTRUCTOR
            public ManufTestReport(string FileFolder, string FileNam)
            {
                if (sIPaddress == "0.0.0.0") return; // FAILED to find NI computer!
                _FileSubDir = FileFolder;
                _FileNamePrefix = FileNam;
                string PassFileSpec = _FileSubDir + _FileNamePrefix + "_PASS" + _FileNameExt;
                string FailFileSpec = _FileSubDir + _FileNamePrefix + "_FAIL" + _FileNameExt;
                bool bCreateTheFile = false;  // only create file once - creation obliterates past entries
                                              // does a "fail" FILE already exist? If so we may run a single re-test, preserving all other tests
                _FileSpec = File.Exists(FailFileSpec) == true ? FailFileSpec : PassFileSpec;
                if ((_FileSpec != FailFileSpec) && !File.Exists(PassFileSpec))  // 
                {
                    bCreateTheFile = true;
                }
                string sStatus = InitWorkBook(bCreateTheFile);
            }
            public string WriteWorkSheetRecord(string SheetName, int TestNum, string[] Row) // array 0 means Record Headers, 1...n means Record Values
            {
                int iCol;
                _oCurrentWorksheet = null;
                switch (SheetName)
                {
                    case "TDboard":
                        _oCurrentWorksheet = oTDboardWS;
                        break;
                    case "DACcard":
                        _oCurrentWorksheet = oDACcardWS;
                        break;
                    case "ADCcard":
                        _oCurrentWorksheet = oADCcardWS;
                        break;
                    case "TRIGcard":
                        _oCurrentWorksheet = oTRIGcardWS;
                        break;
                    case "DBB1":
                        _oCurrentWorksheet = oDBB1WS;
                        break;
                    case "ABB1":
                        _oCurrentWorksheet = oABB1WS;
                        break;
                    case "ABB2":
                        _oCurrentWorksheet = oABB2WS;
                        break;
                    case "ABB3":
                        _oCurrentWorksheet = oABB3WS;
                        break;
                }
                if (_oCurrentWorksheet == null)
                {
                    return "FAIL: Report Worksheet NOT FOUND";
                }
                // TestNum 0 means write the record HEADERS at Row 1
                //Add table headers going cell by cell.
                try
                {
                    for (iCol = 0; iCol < MAXColumnsINREPORT; iCol++)
                    {
                        ((Excel._Worksheet)_oCurrentWorksheet).Cells[TestNum + 1, iCol + 1] = Row[iCol]; // spreadsheet "array" starts at 1, not 0
                    }
                    // autofit the columns
                    ((Excel._Worksheet)_oCurrentWorksheet).Columns.AutoFit();

                }
                catch (Exception e)
                {
                    return ("FAIL: Exception: " + e.Message);
                }
                // save edits with default (e.g. "PASS") - when exiting CL-GUI we test for "FAIL"ed record
                ((Microsoft.Office.Interop.Excel._Workbook)_oWB).SaveAs((_FileSubDir + _FileNamePrefix + _FileNamePassFail + _FileNameExt), Excel.XlFileFormat.xlWorkbookDefault, Missing.Value, Missing.Value, Missing.Value, Missing.Value, Excel.XlSaveAsAccessMode.xlExclusive, Excel.XlSaveConflictResolution.xlLocalSessionChanges);
                return "SUCCESS";
            }

            public bool FoundAfailedTestInWorksheet()
            {
                bool bFoundFailedTest = false;
                Excel.Range currentFind;
                _ReTestFailRow = 1;

                // clear the ReTest sheet
 //               for( int i=0; i < 200; i++)
 //                   ((Excel._Worksheet)oReTestWS).Cells[i, 1] = "";

                //                _oWB.Worksheets.Count
                foreach (Excel._Worksheet ws in _oXL.Application.Worksheets)
                {
                    currentFind = null;
                    if (ws.Name == "Re-Test") continue; // obviously don't check the "Re-Test" worksheet

                    try
                    {
                        Excel.Range SheetTestRange = ws.UsedRange;

                        //                Excel.Range FailedTestRange = _oWB.Application.get_Range("A1", "Z100");
                        // You should specify all these parameters every time you call this method,
                        // since they can be overridden in the user interface. 
                        currentFind = SheetTestRange.Find("FAIL", Missing.Value,
                            Excel.XlFindLookIn.xlValues, Excel.XlLookAt.xlPart,
                            Excel.XlSearchOrder.xlByRows, Excel.XlSearchDirection.xlNext, false,
                            Missing.Value, Missing.Value);

                        if (currentFind == null) continue;

                        // now traverse the spreadsheet to denote failed tests and list each in re-test worksheet

                        object[,] TestRecords = SheetTestRange.Value;

                        int iRow, iCol, iTestIndex = 3; // for ADC card, subtraction from Spreadsheet test line
                        for (iRow = 2; iRow <= TestRecords.GetUpperBound(0); iRow++) // check every row (test) for a failure
                        {
                            iCol = 2;  // the PASS/FAIL record
                            if ((String)TestRecords[iRow, iCol] == "FAIL")
                            {
                                String sManufTestName = "ManufTest -" + ws.Name + " "; // copy the spreadsheet name we are now examining (e.g. "DACcard")
                                bFoundFailedTest = true; // first FAIL instance triggers filename to be "FAIL" not "PASS"
                                if (currentFind != null)
                                {
                          //          currentFind.Font.Color = System.Drawing.ColorTranslator.ToOle(System.Drawing.Color.Red);
                          //          currentFind.Font.Bold = true;
                                }
                                // denote the test that user should re-run for the failed item...
                                // for the tests that need to run in total (e.g. -TDboard, -TRIGcard)
                                // don't include a test index because it's invalid
                                if (ws.Name == "ADCcard" || ws.Name == "DACcard")
                                {
                                    if (ws.Name == "DACcard") iTestIndex = 4;
                                   sManufTestName = sManufTestName + (iRow - iTestIndex).ToString();
                                }
                                // write the ReTest record
                                ((Excel._Worksheet)oReTestWS).Cells[_ReTestFailRow++, 1] = sManufTestName;
                                // is there another FAIL to highlight on this worksheet?
                                currentFind = SheetTestRange.Find("FAIL", Missing.Value, Excel.XlFindLookIn.xlValues, Excel.XlLookAt.xlPart, Excel.XlSearchOrder.xlByRows, Excel.XlSearchDirection.xlNext, false, Missing.Value, Missing.Value);
                            }
                        }
                        continue; // next Spreadsheet (test)
                    }
                    catch (Exception e)  // hereeee  
                    {
                        System.Diagnostics.Debug.WriteLine(" Excel error: " + e.Message);
                    }
                    finally
                    {
                        // autofit the ReTest columns
                        ((Excel._Worksheet)oReTestWS).Columns.AutoFit();
                    }
                }  // end ForEach spreadsheet
                return bFoundFailedTest;
            }

            // create the workbook with all (empty) sheets, one sheet per component (~8 sheets)
            public string InitWorkBook(bool bCreateFile)
            {
                //string FileName = @"TD2035";
                //  _FileSpec = FileFolder + FileName;
                try
                {
                    _oXL = new Excel.Application();
                    _oXL.Visible = false;       // may confuse user if MS Excel App opens
                    _oXL.DisplayAlerts = false; // stop Excel from blocking us with user prompts
                }
                catch (Exception e)
                {
                    return ("FATAL ERR: Is MS Excel App installed?... " + e.Message);
                }
                try
                {
                    if (bCreateFile)
                    {
                        _oWB = (Excel._Workbook)(_oXL.Workbooks.Add(Missing.Value));

                        // Def. behavior: the LAST sheet added is FIRST sheet in file
                        //                        oABB3WS = (Excel._Worksheet)_oWB.ActiveSheet; 
                        oReTestWS = (Excel._Worksheet)_oWB.Worksheets.Add();
                        oReTestWS.Name = "Re-Test";

                        oABB3WS = (Excel._Worksheet)_oWB.Worksheets.Add();
                        oABB3WS.Name = "ABB3";
                        oABB2WS = (Excel._Worksheet)_oWB.Worksheets.Add();
                        oABB2WS.Name = "ABB2";
                        oABB1WS = (Excel._Worksheet)_oWB.Worksheets.Add();
                        oABB1WS.Name = "ABB1";

                        oTRIGcardWS = (Excel._Worksheet)_oWB.Worksheets.Add();
                        oTRIGcardWS.Name = "TRIGcard";

                        oDACcardWS = (Excel._Worksheet)_oWB.Worksheets.Add();
                        oDACcardWS.Name = "DACcard";

                        oDBB1WS = (Excel._Worksheet)_oWB.Worksheets.Add();
                        oDBB1WS.Name = "DBB1";

                        oADCcardWS = (Excel._Worksheet)_oWB.Worksheets.Add();
                        oADCcardWS.Name = "ADCcard";

                        oTDboardWS = (Excel.Worksheet)_oWB.Worksheets.Add();
                        oTDboardWS.Name = "TDboard"; // last Added Sheet is "Active"

                        // trying to save in older XlFileFormat 95/97 Excel format causes Exception 0x800A03EC
                        // assume PASS - "_FAIL" file generated on error
                        ((Microsoft.Office.Interop.Excel._Workbook)_oWB).SaveAs((_FileSubDir + _FileNamePrefix + _FileNamePassFail + _FileNameExt), Excel.XlFileFormat.xlWorkbookDefault, Missing.Value, Missing.Value, Missing.Value, Missing.Value, Excel.XlSaveAsAccessMode.xlExclusive, Excel.XlSaveConflictResolution.xlLocalSessionChanges);
                    }
                    else // file already exists
                    {                    //ARGS: filename, updateLinks, readonly, format (5=nothing), passwd, writeResPasswd, IgnoreReadonly, origin, delimeter,editable,notify,converter, AddToMru  
                        _oWB = _oXL.Workbooks.Open(_FileSpec, 0, false, 5, "", "", true, Microsoft.Office.Interop.Excel.XlPlatform.xlWindows, "\t", true, false, 0, true, 1, 0);

                        oTDboardWS = (Excel._Worksheet)_oWB.Worksheets["TDboard"];
                        oDACcardWS = (Excel._Worksheet)_oWB.Worksheets["DACcard"];
                        oABB1WS = (Excel._Worksheet)_oWB.Worksheets["ABB1"];
                        oABB2WS = (Excel._Worksheet)_oWB.Worksheets["ABB2"];
                        oABB3WS = (Excel._Worksheet)_oWB.Worksheets["ABB3"];
                        oTRIGcardWS = (Excel._Worksheet)_oWB.Worksheets["TRIGcard"];
                        oADCcardWS = (Excel._Worksheet)_oWB.Worksheets["ADCcard"];
                        oDBB1WS = (Excel._Worksheet)_oWB.Worksheets["DBB1"];
                        // remove and re-create the Re-Test worksheet
                        oReTestWS = (Excel._Worksheet)_oWB.Worksheets["Re-Test"];
                        if( oReTestWS != null) ((Excel.Worksheet)oReTestWS).Delete();
                        oReTestWS = (Excel._Worksheet)_oWB.Worksheets.Add();
                        oReTestWS.Name = "Re-Test";
                    }
                }
                catch (Exception e)
                {
                    StackTrace st = new StackTrace(new StackFrame(true));
                    StackFrame sf = st.GetFrame(0);
                    return (string.Format("FATAL ERR @line#{0} on Excel File Op:  ", sf.GetFileLineNumber()) + e.Message);
                }
                return "SUCCESS";
            }
            ~ManufTestReport() // Destructor
            {
                string eMsg;
                try
                {
                    if (_oWB != null)
                    {
                        if (FoundAfailedTestInWorksheet() == true)
                        {
                            _FileNamePassFail = "_FAIL";
                        }

                        _oXL.DisplayAlerts = false;
                        ((Microsoft.Office.Interop.Excel._Workbook)_oWB).SaveAs((_FileSubDir + _FileNamePrefix + _FileNamePassFail + _FileNameExt), Excel.XlFileFormat.xlWorkbookDefault, Missing.Value, Missing.Value, Missing.Value, Missing.Value, Excel.XlSaveAsAccessMode.xlExclusive, Excel.XlSaveConflictResolution.xlLocalSessionChanges);
                        ((Microsoft.Office.Interop.Excel._Workbook)_oWB).Close(false, Missing.Value, Missing.Value);
                    }
                    if (_oXL != null)
                    {
                        _oXL.Quit();


                    }
                    string sFileToRemove = (_FileNamePassFail == "_FAIL") ? "_PASS" : "_FAIL";
                    File.Delete(_FileSubDir + _FileNamePrefix + sFileToRemove + _FileNameExt); // final result - pass OR fail filename
                }
                catch (Exception e) // this is a destructor - can't report failure
                {
                    eMsg = e.Message;
                }
                finally
                {
                    eMsg = "done";
                    TestReport = null;
                }
            }
        };



        // Read the FPGA's PMbus to discover power chips and operating params like Temp, Volts, Current
        // and format into fields suitable for spreadsheet Report output
        public enum FieldName
        {
            TestNum = 0,
            PassFail,
            TestDescription,
            Units,
            Mean,
            StdDev,
            Min,
            Max,
            StdDevLimit
        };

        public class MeanStdDevHiLoLimits  // per UDC9248 chip
        {
            public byte PMbusChipAddress;
            public byte ChipRail; // not used for "Temp", and ChipAddr 54 has 3 rails, not 4
            public double[] MeanMax;
            public double[] MeanMin;
            public double[] StdDevLimit;
            private int _MaxRails = 4;
            public MeanStdDevHiLoLimits(int Rails)
            {
                _MaxRails = Rails;
                MeanMax = new double[_MaxRails]; // typically 4 rails
                MeanMin = new double[_MaxRails]; // 
                StdDevLimit = new double[_MaxRails];
            }
        };


        public class AccessFPGAregisters
        {
            private uint _boardIndx;
            public AccessFPGAregisters(uint BoardIndex)
            {
                _boardIndx = BoardIndex;
            }
            public bool Read(string sRegFldName, ref UInt64 Value)
            {
                bool bStatus = true;
                uint _BoardIndex = 0; // default (1st) board (for now)
                IntPtr NativeNameBuf = Marshal.StringToCoTaskMemAnsi(sRegFldName);
                IntPtr _UINT64_NativeBuffer = Marshal.AllocHGlobal(sizeof(UInt64) * 2); // should never fail
                uint uiStatus = ThorDAQAPIFPGAregisterRead(_BoardIndex, NativeNameBuf, sRegFldName.Length, _UINT64_NativeBuffer);
                if (uiStatus != 0)
                {
                    bStatus = false;
                }
                else
                {
                    Value = (UInt64)Marshal.ReadInt64(_UINT64_NativeBuffer);
                }
                // remember to free unmanaged mem
                Marshal.FreeCoTaskMem(NativeNameBuf);
                Marshal.FreeHGlobal(_UINT64_NativeBuffer);
                return bStatus;
            }
            public bool Write(string sRegFldName, UInt64 value)
            {
                uint _BoardIndex = 0; // default (1st) board (for now)
                int _RegFldNameBufLen = 48;
                IntPtr _NativeBuffer = Marshal.AllocHGlobal(_RegFldNameBufLen); // should never fail
                IntPtr _UINT64_NativeBuffer = Marshal.AllocHGlobal(sizeof(UInt64) * 2); // should never fail

                bool bStatus = true;
                IntPtr NativeNameBuf = Marshal.StringToCoTaskMemAnsi(sRegFldName);
                Marshal.WriteInt64(_UINT64_NativeBuffer, (long)value);
                uint uiStatus = ThorDAQAPIFPGAregisterWrite(_BoardIndex, NativeNameBuf, sRegFldName.Length, _UINT64_NativeBuffer);
                if (uiStatus != 0)
                    bStatus = false;  // e.g. Name not found
                                      // remember to free unmanaged mem
                Marshal.FreeCoTaskMem(NativeNameBuf);
                Marshal.FreeHGlobal(_NativeBuffer);
                Marshal.FreeHGlobal(_UINT64_NativeBuffer);

                return bStatus;
            }
        };



        public class StdDevAndMean // return StdDev and ref to Mean
        {

            public double ComputeStdDevAndMean(List<double> numbers, ref double Mean)
            {
                // Step 1
                var meanOfNumbers = numbers.Average();
                Mean = meanOfNumbers;
                // Step 2
                var squaredDifferences = new List<double>(numbers.Count);
                foreach (var number in numbers)
                {
                    var difference = number - meanOfNumbers;
                    var squaredDifference = Math.Pow(difference, 2);
                    squaredDifferences.Add(squaredDifference);
                }

                // Step 3
                var meanOfSquaredDifferences = squaredDifferences.Average();

                // Step 4
                var standardDeviation = Math.Sqrt(meanOfSquaredDifferences); // "population" - n is samples, not (samples-1)

                return standardDeviation;
            }
        };



        // NOTE: This test cannot utilize the NI-PXIe1071, and as of Dec 2020 requires a "USB Interface Adapter"
        // by Texas Instuments externally connected to ThorDAQ via 10-pin ribbon cable
        // The "report" format is the string array of measured values consistent with the string array of Headers
        // A report record is always displayed on Console - if "-r" Report enable, that
        // record is copied to the appropriate Worksheet in the Report Workbook file.
        public class ThorDAQmainBoardTestAndReport
        {
            private TIBusAdapters TI_USBadapter = new TIBusAdapters();
            private ushort usBoardIndex = 0; // default is single ThorDAQ
            string _TempVoltageCurrentUnits;
            byte _RailIndx; // e.g. 0,1,2 or 3
            byte _PMbusDevAddress; // e.g. 52, 53, 54
            public string[] TD_RepColHdr = new string[MAXColumnsINREPORT];    // test header ROW
            public string[] TD_ReportColValues = new string[MAXColumnsINREPORT]; // a value record
            public MeanStdDevHiLoLimits[] TempLimits = new MeanStdDevHiLoLimits[3];  // 3 UDC9248 chips
            public MeanStdDevHiLoLimits[] VoltLimits = new MeanStdDevHiLoLimits[3];  // 3 UDC9248 chips
            public MeanStdDevHiLoLimits[] CurrentLimits = new MeanStdDevHiLoLimits[3];  // 3 UDC9248 chips

            public ThorDAQmainBoardTestAndReport(ushort BoardIndex) // CONSTRUCTOR
            {
                usBoardIndex = BoardIndex; // store private variable
                TD_RepColHdr[(int)FieldName.TestNum] = "Test#"; TD_RepColHdr[(int)FieldName.PassFail] = "P/F"; TD_RepColHdr[(int)FieldName.TestDescription] = "Test Description"; TD_RepColHdr[(int)FieldName.Units] = "Units"; TD_RepColHdr[(int)FieldName.Mean] = "Avg"; TD_RepColHdr[(int)FieldName.StdDev] = "StdDev"; TD_RepColHdr[(int)FieldName.Min] = "MinLimit"; TD_RepColHdr[(int)FieldName.Max] = "MaxLimit"; TD_RepColHdr[(int)FieldName.StdDevLimit] = "StdDevLim";
                if (TestReport != null) // Global ManufTest workbook file report exists? (command option)
                {
                    TestReport.WriteWorkSheetRecord("TDboard", 0, TD_RepColHdr); // first write headers
                }
                for (int iChip = 0; iChip < 3; iChip++)
                {
                    int iRails = 4;
                    if (iChip == 2) iRails = 3; // 3 rails on PMbus Chip 54 (3rd chip)
                    TempLimits[iChip] = new MeanStdDevHiLoLimits(iRails);  // 3 UDC9248 chips, 4 rails each
                    VoltLimits[iChip] = new MeanStdDevHiLoLimits(iRails);
                    CurrentLimits[iChip] = new MeanStdDevHiLoLimits(iRails);
                }
                // fill out the Temp/Voltage/Current limits array
                SetMinMaxStdDevTolerances();
            }

            // For now hard code the PASS/FAIL metrics for the test rails
            // These values come from TSI "Production" .XML files, e.g.
            // KC705_RevE_UDC9248_addrXX_R0.XML,
            // where XX is PMbus UDC9248 chip address 52, 53, 54
            // One XML file per chip, 3 chips
            // Target voltage settings are "VOUT_COMMAND" "configuration" params
            // per configured volate rails (4 rails for 52, 53, only 3 rails for 54)
            private void SetMinMaxStdDevTolerances()
            {
                // what is the Acceptable StdDev, Min, Max for Voltage/Current on the separate rails on
                // the 3 TI UDC9248 hardware devices?
                // We may want to move these definitions to separate file, so present them in tab-like format
                TempLimits[0].PMbusChipAddress = 52;
                TempLimits[0].ChipRail = 0xFF; // not used
                TempLimits[0].MeanMax[0] = 65.0;   // 150 degF 
                TempLimits[0].MeanMin[0] = 15.0;   // 68 degF
                TempLimits[0].StdDevLimit[0] = 1.0;
                TempLimits[1].PMbusChipAddress = 53;
                TempLimits[1].ChipRail = 0xFF; // not used
                TempLimits[1].MeanMax[0] = 65.0;   // 150 degF
                TempLimits[1].MeanMin[0] = 15.0;   // 68 degF
                TempLimits[1].StdDevLimit[0] = 1.0;
                TempLimits[2].PMbusChipAddress = 54;
                TempLimits[2].ChipRail = 0xFF; // not used
                TempLimits[2].MeanMax[0] = 65.0;   // 150 degF
                TempLimits[2].MeanMin[0] = 15.0;   // 68 degF
                TempLimits[2].StdDevLimit[0] = 1.0;
                // We have total of 11 voltages and currents
                // 4 rails on Chips 52 & 53, and 3 rails on Chip 54
                // Chip 52
                VoltLimits[0].PMbusChipAddress = 52;
                VoltLimits[0].ChipRail = 0;    // Rail#1 target 1.0 VDC
                VoltLimits[0].MeanMax[0] = 1.05;   // +/- 5%
                VoltLimits[0].MeanMin[0] = 0.95;   // 
                VoltLimits[0].StdDevLimit[0] = 1.0;
                VoltLimits[0].ChipRail = 1;    // Rail#2 target 1.8 VDC
                VoltLimits[0].MeanMax[1] = 1.89;   // +/- 5% (.09)
                VoltLimits[0].MeanMin[1] = 1.71;   //  
                VoltLimits[0].StdDevLimit[1] = 1.0;
                VoltLimits[0].ChipRail = 3;    // Rail#3 target 3.3 VDC
                VoltLimits[0].MeanMax[2] = 3.47;   // +/- 5% (.17)  
                VoltLimits[0].MeanMin[2] = 3.13;
                VoltLimits[0].StdDevLimit[2] = 1.0;
                VoltLimits[0].ChipRail = 4;    // Rail#4 target 2.5 VDC
                VoltLimits[0].MeanMax[3] = 2.63;   // +/- 5% (.13)
                VoltLimits[0].MeanMin[3] = 2.37;   // 
                VoltLimits[0].StdDevLimit[3] = 1.0;
                CurrentLimits[0].PMbusChipAddress = 52;
                CurrentLimits[0].ChipRail = 0;    // Rail#1 
                CurrentLimits[0].MeanMax[0] = 16.81; // IOUT_OC_WARN_LIMIT (amps)
                CurrentLimits[0].MeanMin[0] = 1.0;
                CurrentLimits[0].StdDevLimit[0] = 1.0;
                CurrentLimits[0].ChipRail = 1;    // Rail#2 
                CurrentLimits[0].MeanMax[1] = 10.52;
                CurrentLimits[0].MeanMin[1] = 0.350;
                CurrentLimits[0].StdDevLimit[1] = 1.0;
                CurrentLimits[0].ChipRail = 3;    // Rail#3 
                CurrentLimits[0].MeanMax[2] = 10.52;
                CurrentLimits[0].MeanMin[2] = 1.50;
                CurrentLimits[0].StdDevLimit[2] = 1.0;
                CurrentLimits[0].ChipRail = 4;    // Rail#4
                CurrentLimits[0].MeanMax[3] = 8.41;
                CurrentLimits[0].MeanMin[3] = 0.150;
                CurrentLimits[0].StdDevLimit[3] = 1.0;
                // Chip 53
                VoltLimits[1].PMbusChipAddress = 53;
                VoltLimits[1].ChipRail = 0;    // Rail#1 target 2.5 VDC
                VoltLimits[1].MeanMax[0] = 2.63;   // +/- 5%
                VoltLimits[1].MeanMin[0] = 2.37;   // 
                VoltLimits[1].StdDevLimit[0] = 1.0;
                VoltLimits[1].ChipRail = 1;    // Rail#2 target 1.5 VDC
                VoltLimits[1].MeanMax[1] = 1.58;   // +/- 5% (.08)
                VoltLimits[1].MeanMin[1] = 1.42;   //  
                VoltLimits[1].StdDevLimit[1] = 1.0;
                VoltLimits[1].ChipRail = 3;    // Rail#3 target 1.0 VDC
                VoltLimits[1].MeanMax[2] = 1.05;   // +/- 5% 
                VoltLimits[1].MeanMin[2] = 0.95;   // 
                VoltLimits[1].StdDevLimit[2] = 1.0;
                VoltLimits[1].ChipRail = 4;    // Rail#4 target 1.2 VDC
                VoltLimits[1].MeanMax[3] = 1.26;   // +/- 5% (.06)
                VoltLimits[1].MeanMin[3] = 1.14;   // 
                VoltLimits[1].StdDevLimit[3] = 1.0;
                CurrentLimits[1].PMbusChipAddress = 53;
                CurrentLimits[1].ChipRail = 0;    // Rail#1 
                CurrentLimits[1].MeanMax[0] = 10.52; // IOUT_OC_WARN_LIMIT (amps)
                CurrentLimits[1].MeanMin[0] = 0.045;
                CurrentLimits[1].StdDevLimit[0] = 1.0;
                CurrentLimits[1].ChipRail = 1;    // Rail#2 
                CurrentLimits[1].MeanMax[1] = 10.52;
                CurrentLimits[1].MeanMin[1] = 0.200;
                CurrentLimits[1].StdDevLimit[1] = 1.0;
                CurrentLimits[1].ChipRail = 3;    // Rail#3 
                CurrentLimits[1].MeanMax[2] = 10.52;
                CurrentLimits[1].MeanMin[2] = 0.500;
                CurrentLimits[1].StdDevLimit[2] = 1.0;
                CurrentLimits[1].ChipRail = 4;    // Rail#4
                CurrentLimits[1].MeanMax[3] = 10.52;
                CurrentLimits[1].MeanMin[3] = 0.800;
                CurrentLimits[1].StdDevLimit[3] = 1.0;
                // Chip 54
                VoltLimits[2].PMbusChipAddress = 54;
                VoltLimits[2].ChipRail = 0;    // Rail#1 target 2.0 VDC
                VoltLimits[2].MeanMax[0] = 2.1;   // +/- 5%
                VoltLimits[2].MeanMin[0] = 1.9;   // 
                VoltLimits[2].StdDevLimit[0] = 1.0;
                VoltLimits[2].ChipRail = 1;    // Rail#2 target 1.0 VDC
                VoltLimits[2].MeanMax[1] = 1.05;   // +/- 5%
                VoltLimits[2].MeanMin[1] = 0.95;   //  
                VoltLimits[2].StdDevLimit[1] = 1.0;
                VoltLimits[2].ChipRail = 3;    // Rail#3 target 1.8 VDC
                VoltLimits[2].MeanMax[2] = 1.89;   // +/- 5% (.09)
                VoltLimits[2].MeanMin[2] = 1.71;   // 
                VoltLimits[2].StdDevLimit[2] = 1.0;
                CurrentLimits[2].PMbusChipAddress = 54;
                CurrentLimits[2].ChipRail = 0;    // Rail#1 
                CurrentLimits[2].MeanMax[0] = 10.52; // IOUT_OC_WARN_LIMIT (amps)
                CurrentLimits[2].MeanMin[0] = 0.050;
                CurrentLimits[2].StdDevLimit[0] = 1.0;
                CurrentLimits[2].ChipRail = 1;    // Rail#2 
                CurrentLimits[2].MeanMax[1] = 10.52;
                CurrentLimits[2].MeanMin[1] = 0.001;
                CurrentLimits[2].StdDevLimit[1] = 1.0;
                CurrentLimits[2].ChipRail = 3;    // Rail#3 
                CurrentLimits[2].MeanMax[2] = 8.41;
                CurrentLimits[2].MeanMin[2] = 0.0;
                CurrentLimits[2].StdDevLimit[2] = 1.0;
            }

            // LINEAR16 to double
            // In the TI 9248, exponent fixed at -12, with 16-bit unsigned matissa
            private double LINEAR16toDouble(short UnsignedVoltsMantissa)
            {
                return (UnsignedVoltsMantissa * Math.Pow(2, -12));
            }
            // PMBus conversions
            // LINEAR11 to double
            private double LINEAR11toDouble(Int16 Linear11value)
            {
                // LINEAR11 bit field, 11 LSB is "Y", 5 MSB is "N"
                // NNNNNYYY.YYYYYYYY
                Int32 Y;
                Int32 N;
                Y = N = Linear11value;
                Y &= 0x07FF;
                if ((Y & 0x400) == 0x400)
                {
                    unchecked
                    {
                        Y |= (int)0xFFFFF800;  // extend 2s compliment sign
                    }
                }
                N &= 0x0000FFFF; // only lower 16 bits are defined...
                N = N >> 11; // upper 5 bits (of 16 bit 'N' 'Y' fields)
                if ((N & 0x10) == 0x10) // negative?
                {
                    unchecked
                    {
                        N |= (int)0xFFFFFFE0; // 2s compliment
                    }
                }
                double dTemp = Y * Math.Pow(2, N);
                return dTemp;
            }
            // Compare the measured values to the limits array - if any it outside limits
            // return FAIL, else PASS
            public string PassFailLimits(string[] MeauredValues)
            {
                string bPF = "PASS"; // default to TRUE
                if (Convert.ToDouble(MeauredValues[(int)FieldName.Mean]) > Convert.ToDouble(MeauredValues[(int)FieldName.Max]))
                {
                    bPF = "FAIL";
                }
                if (Convert.ToDouble(MeauredValues[(int)FieldName.Mean]) < Convert.ToDouble(MeauredValues[(int)FieldName.Min]))
                {
                    bPF = "FAIL";
                }
                if (Convert.ToDouble(MeauredValues[(int)FieldName.StdDev]) > Convert.ToDouble(MeauredValues[(int)FieldName.StdDevLimit]))
                {
                    bPF = "FAIL";
                }
                return bPF;
            }

            // Test#1 is EEPROM serial num for this component
            public string TestEEPROMserNum()
            {
                string[] EEPROM_ReportColValues = new string[MAXColumnsINREPORT]; // a value record
                string FunctionStatus = "SUCCESS";
                string passFail = "FAIL";
                EEPROM_ReportColValues[(int)FieldName.TestNum] = "1";  // test# to perform
                EEPROM_ReportColValues[(int)FieldName.TestDescription] = "EEPROM S/N: ";

                string TDSerNumString;
                if (GetI2CeepromSerNum(usBoardIndex, 0x80, 0xFF, 0x54, 0, out TDSerNumString))
                {  // success?
                    EEPROM_ReportColValues[(int)FieldName.TestDescription] += TDSerNumString;
                    if (TDSerNumString.Substring(0, 2) == "TD")
                        passFail = "PASS";
                }
                else
                {
                    EEPROM_ReportColValues[(int)FieldName.TestDescription] += "Failed to read I2C device";
                }
                // get the FPGA firmware revision
                BOARD_INFO_STRUCT BoardConfig = new BOARD_INFO_STRUCT();
                string FPGAver = " [cannot read]";
                if (Win32.GetBoardCfg(usBoardIndex, ref BoardConfig) == Win32.STATUS_SUCCESSFUL)
                {
                    FPGAver = "FPGA Ver: 0x" + BoardConfig.UserVersion.ToString("X10");
                }
                EEPROM_ReportColValues[(int)FieldName.TestDescription] += FPGAver;
                EEPROM_ReportColValues[(int)FieldName.PassFail] = passFail;
                if (TestReport != null) // workbook exists...
                {
                    TestReport.WriteWorkSheetRecord("TDboard", 1, EEPROM_ReportColValues); // commit test record to file
                }
                return FunctionStatus;
            }

            // Get the DDR3 memory card info - read embedded EEPROM
            public string[] TestDDR3_EEPROM(int TestNum)
            {
                string[] TD_ReportColValues = new string[MAXColumnsINREPORT]; // a value record


                TD_ReportColValues[(int)FieldName.PassFail] = "FAIL"; // default
                TD_ReportColValues[(int)FieldName.TestNum] = TestNum.ToString(CultureInfo.InvariantCulture);
                // first, does the DDR module answer on the I2C bus??
                List<string> SPDfields = new List<string> { };
                SPDfields = GetDDR3status(new List<string> { "InternalCmd", " " });
                if (SPDfields[0].StartsWith("Error"))
                {
                    TD_ReportColValues[(int)FieldName.TestDescription] = SPDfields[0];
                }
                else
                {
                    TD_ReportColValues[(int)FieldName.PassFail] = "PASS";
                    for (int i = 0; i < 4; i++) // copy starting fields
                        TD_ReportColValues[(int)FieldName.TestDescription] += SPDfields[i] + "; ";
                }

                if (TestReport != null) // workbook exists...
                {
                    TestReport.WriteWorkSheetRecord("TDboard", TestNum, TD_ReportColValues); // commit test record to file
                }
                return TD_ReportColValues;
            }


            // Perform rudimentary DDR3 memory test
            public string[] TestDDR3_Memory(int TestNum, UInt64 DDR3_startAddr)
            {
                string[] TD_ReportColValues = new string[MAXColumnsINREPORT]; // a value record
                UInt32 transferredLen = 1024 * 512;
                int ByteOffsetMisMatchIndex = -1;
                Byte ByteWritten, ByteRead;
                TD_ReportColValues[(int)FieldName.TestNum] = TestNum.ToString(CultureInfo.InvariantCulture);
                TD_ReportColValues[(int)FieldName.TestDescription] = "DDR3 Mem Test, StartingAddr: 0x" + DDR3_startAddr.ToString("X") + " ";
                TD_ReportColValues[(int)FieldName.PassFail] = "PASS";

                // max Image Res, 2 byte/pixel, 4 channels, 2 DMA banks
                UInt32 MaxDDR3ReadWriteLen = 4096 * 4096 * 2 * 4 * 2;
                Byte FirstDataByte = 0xA5;

                Byte[] DDR3readBackBuff = new Byte[MaxDDR3ReadWriteLen];
                Byte[] DDR3writeBuff = new Byte[MaxDDR3ReadWriteLen];

                bool bStatus;
                // Read entire memory of interest
                bStatus = ThorDAQAdapters[MasterBoardIndex].DDR3RAM.ReadDDR3(DDR3_startAddr, ref MaxDDR3ReadWriteLen, ref DDR3writeBuff);
                if (bStatus == true)
                {
                    UInt64 DDR3Offset = 13; // pick odd boundary
                                            // now alter a portion of memory inside boundary to readback-verify...
                    for (ulong j = DDR3Offset; j < transferredLen; j++)
                    {
                        DDR3writeBuff[j] = FirstDataByte;
                    }
                    // attempt physical memory write...
                    bStatus = ThorDAQAdapters[MasterBoardIndex].DDR3RAM.WriteDDR3(DDR3_startAddr, ref MaxDDR3ReadWriteLen, DDR3writeBuff);
                    if (bStatus == true)
                    {
                        // readback what's now in memory, and compare to what we just wrote
                        bStatus = ThorDAQAdapters[MasterBoardIndex].DDR3RAM.ReadDDR3(DDR3_startAddr, ref MaxDDR3ReadWriteLen, ref DDR3readBackBuff);
                        if (bStatus == true)
                        {
                            // do BYTE comparison
                            int i;
                            for (i = 0; i < MaxDDR3ReadWriteLen; i++)
                            {
                                if (DDR3writeBuff[i] != DDR3readBackBuff[i])
                                {
                                    ByteOffsetMisMatchIndex = i;
                                    ByteWritten = DDR3writeBuff[i];
                                    ByteRead = DDR3readBackBuff[i];
                                    TD_ReportColValues[(int)FieldName.TestDescription] += " @ offset 0x" + ByteOffsetMisMatchIndex.ToString("X") +
                                        ", Wrote 0x" + ByteWritten.ToString("X") + ", Read 0x" + ByteRead.ToString("X");
                                    TD_ReportColValues[(int)FieldName.PassFail] = "FAIL";
                                    break;
                                }
                            }
                        }
                    }
                    else
                    {
                        TD_ReportColValues[(int)FieldName.TestDescription] += " Failed API call WriteDDR3() - kernel/DMA error";
                        TD_ReportColValues[(int)FieldName.PassFail] = "FAIL";
                    }

                }
                else
                {
                    TD_ReportColValues[(int)FieldName.TestDescription] += " Failed API call ReadDDR3() - kernel/DMA error";
                    TD_ReportColValues[(int)FieldName.PassFail] = "FAIL";
                }

                if (TestReport != null) // workbook exists...
                {
                    TestReport.WriteWorkSheetRecord("TDboard", TestNum, TD_ReportColValues); // commit test record to file
                }
                return TD_ReportColValues;
            }



            // Read the PMBus hardware - one UDC9248 chip at a time
            // Single Temp for all rails, voltage and current for each rail
            public string[] ReadAndTestTIDevice(int TestNum, int PMbusAddr, string Param, int RailIndx)
            {
                double StdDev, Mean = 0.0;
                StdDevAndMean stdDevAndMean = new StdDevAndMean();

                _TempVoltageCurrentUnits = Param; // first 4 chars determine type: e.g.  "Temp(oC)
                _RailIndx = (byte)RailIndx;  // "-1" means only read temperature, 0-3 is one of 4 rails
                _PMbusDevAddress = (byte)PMbusAddr;
                TD_ReportColValues[(int)FieldName.TestNum] = TestNum.ToString(CultureInfo.InvariantCulture);
                if (TI_USBadapter.Num_Adapters > 0)
                {
                    if (TI_USBadapter.Is_Attached(1))  // is the TI USB device ribbon cable attached to ThorDAQ?
                    {
                        Encoding enc8 = Encoding.UTF8;
                        var sb = new StringBuilder();
                        sb.Append(PMbusAddr + ": ");
                        var block_result = TI_USBadapter.Read_Block(1, _PMbusDevAddress, 0xFD);
                        if (block_result.Success)
                        {
                            sb.Append(block_result.Block.Hex);
                            string StrippedNull = string.Format("Power Supply: PMBusAddr:{0} TexasInst ", _PMbusDevAddress) + enc8.GetString(block_result.Bytes);
                            // Remove terminating NULL from enc8 GetString because it interferes with string Append
                            int NullEndIndx = StrippedNull.Length - 1;
                            string sASCIItestDescription = StrippedNull.Substring(0, NullEndIndx);
                            // complete the Test Description based on passed options
                            if (_RailIndx != 0xFF)
                            {
                                sASCIItestDescription += string.Format(" Rail: {0}", _RailIndx);
                            }

                            // Description complete!
                            TD_ReportColValues[(int)FieldName.TestDescription] = sASCIItestDescription;  // e.g. "Power Supply PMBus 52 TexasInst UDC9248 {Rail n} "

                            // Read the PMbus hardware
                            var read_word_result = TI_USBadapter.Read_Word(1, _PMbusDevAddress, 0x8D); // PMBus device (common) command
                                                                                                       // LINEAR11 bit field, 11 LSB is "Y", 5 MSB is "N"
                            if (read_word_result.Success != true) // make sure hardware answers
                            {
                                // process ERROR!
                                TD_ReportColValues[(int)FieldName.PassFail] = "FAIL";
                                TD_ReportColValues[(int)FieldName.TestDescription] = "Texas Instruments Chip Temperature read failed";
                            }
                            else
                            {
                                TD_ReportColValues[(int)FieldName.Units] = _TempVoltageCurrentUnits; // units of  what we'll read off PMbus...
                                                                                                     // To Compute an Avg. and StdDev.
                                List<double> TIChipReads = new List<double> { }; // empty
                                switch (_TempVoltageCurrentUnits.Substring(0, 3))
                                {
                                    case "Tem":
                                        // Load the test LIMITs for this hardware device (i.e. Chip / Rail)
                                        TD_ReportColValues[(int)FieldName.Max] = TempLimits[_PMbusDevAddress - 52].MeanMax[0].ToString("0.000");
                                        TD_ReportColValues[(int)FieldName.Min] = TempLimits[_PMbusDevAddress - 52].MeanMin[0].ToString("0.000");
                                        TD_ReportColValues[(int)FieldName.StdDevLimit] = TempLimits[_PMbusDevAddress - 52].StdDevLimit[0].ToString("0.000");
                                        // get common temperature for this device
                                        for (int Trial = 0; Trial < 10; Trial++)
                                        {
                                            Thread.Sleep(1); // wait a minimum time between hardware reads
                                            read_word_result = TI_USBadapter.Read_Word(1, _PMbusDevAddress, 0x8D); // PMBus device (common) command
                                                                                                                   // LINEAR11 bit field, 11 LSB is "Y", 5 MSB is "N"
                                            double dTemp = LINEAR11toDouble(read_word_result.Word.To_Int16());
                                            TIChipReads.Add(dTemp);
                                        }
                                        // compute Mean and Standard Deviation on set of hardware readings
                                        StdDev = stdDevAndMean.ComputeStdDevAndMean(TIChipReads, ref Mean); // return StdDev and ref to Mean)
                                        TD_ReportColValues[(int)FieldName.Mean] = string.Format("{0:F1}", Mean);
                                        TD_ReportColValues[(int)FieldName.StdDev] = string.Format("{0:F3}", StdDev);
                                        break;

                                    case "VDC":
                                        // Load the test LIMITs for this hardware device (i.e. Chip / Rail)
                                        TD_ReportColValues[(int)FieldName.Max] = VoltLimits[_PMbusDevAddress - 52].MeanMax[_RailIndx].ToString("0.000");
                                        TD_ReportColValues[(int)FieldName.Min] = VoltLimits[_PMbusDevAddress - 52].MeanMin[_RailIndx].ToString("0.000");
                                        TD_ReportColValues[(int)FieldName.StdDevLimit] = VoltLimits[_PMbusDevAddress - 52].StdDevLimit[_RailIndx].ToString("0.000");
                                        var write_result = TI_USBadapter.Write_Byte(1, _PMbusDevAddress, 0x00, _RailIndx); // select PAGE

                                        for (int Trial = 0; Trial < 10; Trial++)
                                        {
                                            Thread.Sleep(1); // wait a minimum time between hardware reads
                                            read_word_result = TI_USBadapter.Read_Word(1, _PMbusDevAddress, 0x8B); // PMBus READ_VOUT command (per PAGE)
                                            double dVolts = LINEAR16toDouble(read_word_result.Word.To_Int16());
                                            TIChipReads.Add(dVolts);
                                        }
                                        StdDev = stdDevAndMean.ComputeStdDevAndMean(TIChipReads, ref Mean); // return StdDev and ref to Mean)
                                        TD_ReportColValues[(int)FieldName.Mean] = string.Format("{0:F3}", Mean);
                                        TD_ReportColValues[(int)FieldName.StdDev] = string.Format("{0:F3}", StdDev);

                                        break;
                                    case "Amp":
                                        // Load the test LIMITs for this hardware device (i.e. Chip / Rail)
                                        TD_ReportColValues[(int)FieldName.Max] = CurrentLimits[_PMbusDevAddress - 52].MeanMax[_RailIndx].ToString("0.000");
                                        TD_ReportColValues[(int)FieldName.Min] = CurrentLimits[_PMbusDevAddress - 52].MeanMin[_RailIndx].ToString("0.000");
                                        TD_ReportColValues[(int)FieldName.StdDevLimit] = CurrentLimits[_PMbusDevAddress - 52].StdDevLimit[_RailIndx].ToString("0.000");
                                        write_result = TI_USBadapter.Write_Byte(1, _PMbusDevAddress, 0x00, _RailIndx); // select PAGE

                                        for (int Trial = 0; Trial < 10; Trial++)
                                        {
                                            Thread.Sleep(1); // wait a minimum time between hardware reads
                                            read_word_result = TI_USBadapter.Read_Word(1, _PMbusDevAddress, 0x8C); // PMBus READ_IOUT (per PAGE/PHASE)
                                            double dAmps = LINEAR11toDouble(read_word_result.Word.To_Int16());
                                            TIChipReads.Add(dAmps);
                                        }
                                        StdDev = stdDevAndMean.ComputeStdDevAndMean(TIChipReads, ref Mean); // return StdDev and ref to Mean)
                                        TD_ReportColValues[(int)FieldName.Mean] = string.Format("{0:F3}", Mean);
                                        TD_ReportColValues[(int)FieldName.StdDev] = string.Format("{0:F3}", StdDev);

                                        break;

                                }
                                /////////////  Test readings against limits!!  ////////////////////////////////
                                TD_ReportColValues[(int)FieldName.PassFail] = PassFailLimits(TD_ReportColValues);
                            }
                        }
                        else // read failed, probably disconnected TI USB Interface
                        {
                            TD_ReportColValues[(int)FieldName.PassFail] = "FAIL";
                            TD_ReportColValues[(int)FieldName.TestDescription] = "TI chip read failed - Adapter ribbon cable disconnected from gray connector?";
                        }
                    }
                }
                else
                {
                    TD_ReportColValues[(int)FieldName.PassFail] = "FAIL";
                    TD_ReportColValues[(int)FieldName.TestDescription] = "No Texas Instruments USB device detected - can't access Power Rails";
                }
                // Log this to Report Spreadsheet?
                if (TestReport != null) // workbook exists...
                {
                    TestReport.WriteWorkSheetRecord("TDboard", TestNum, TD_ReportColValues); // commit test record to file
                }

                return TD_ReportColValues; // return the entire line (record) for Console output
            }
            public string[] GetTestHeader()
            {
                return TD_RepColHdr;
            }
        };  //   END of class TIPowerRailTestReport

        public class NIPXIe1071_Controller
        {
            public string SrvResp = "oK?";
            private string _TCPclientVer = "CV1.3.0 25-May-2022"; // "this" version of the ThorDAQ CL-GUI Client to NI 1071 computer
            private string _TCPboardSerNumsSentMsg = null;
            private string _LabViewFileName;
            // Require Listening socket for receiving FILE from NIPXIe1071
            //            TcpListener _ListenFileReceiver = null; // to receive test data file from NI1071
            private string _IP_Addr;
            private int _IP_port;
            //           static string _errMsg;
            public string NI1017ServerStatusMsg = "SUCCESS";
            private string _sParamNum;             // (e.g. channel) copied from the TEST command, used to deliniate returned file
            public bool bStopSrvrFlag = false; // set to TRUE when object destroyed to kill thread
            public bool bNewFileReceived = false; // handshake between TCP listening thread and main program
                                                  // main function polls for this flag to change when
                                                  // test file is expected from NI1071
                                                  // set by Listener thread, cleared by main program


            public NIPXIe1071_Controller(string IP_Addr, int IP_port, string TestFileName)
            {
                _IP_Addr = IP_Addr;
                _IP_port = IP_port;

                // the "file path" is a global
                _LabViewFileName = TestFileName; // e.g. DAC_AO.txt, or ADC_AI.txt, which we expand to DAC_AO07 or ADC_AI3.txt

                // attempt a "CONNECT"
                string UnusedFileSpec = null;
                string sStatus = SendTCPcommand("CMD: CONNECT " + _TCPclientVer, ref SrvResp, ref UnusedFileSpec);
                SrvResp = sStatus;
            } // END of Constructor
            private string GetMyIP()
            {
                string strHostName = "";
                strHostName = System.Net.Dns.GetHostName();

                IPHostEntry ipEntry = System.Net.Dns.GetHostEntry(strHostName);

                IPAddress[] addr = ipEntry.AddressList;

                return addr[addr.Length - 1].ToString();
            }

            // receive the Labview data FILE
            // was "ControlNIPXIe1071()"
            // NIPXIe1071_Controller.SendTCPcommand
            public string SendTCPcommand(string sCommandMsg, ref string SrvResponseMsg, ref string CapturedFileName)
            {
                int iStatus = 0;
                string[] SplitMsg = sCommandMsg.Split(' '); // detect case of receive file
                string sResult = "SUCCESS";
                CapturedFileName = null; // let this function generate the entire filespec for disk
                try
                {
                    // if this is the first test command, send "BOARDSN" before proceeding to transmit "sCommandMsg"
                    if (_TCPboardSerNumsSentMsg == null && (SplitMsg[1] != "CONNECT"))
                    {
                        // Get component serial numbers
                        string AllTDSerialNumbers;
                        ReadAllTDSerialNums(MasterBoardIndex, out AllTDSerialNumbers); // Board global from Main()
                        TcpClient NIclient = new TcpClient(_IP_Addr, _IP_port);
                        _TCPboardSerNumsSentMsg = "CMD: BOARDSN " + AllTDSerialNumbers;
                        Byte[] ConnectData = System.Text.Encoding.ASCII.GetBytes(_TCPboardSerNumsSentMsg);
                        NetworkStream ConnStream = NIclient.GetStream();
                        ConnStream.Write(ConnectData, 0, ConnectData.Length);
                        Byte[] RxConnData = new Byte[256];
                        ConnStream.ReadTimeout = 500; // millisec timeout
                        Int32 RxBytes = ConnStream.Read(RxConnData, 0, RxConnData.Length);
                        string NI1071ResponseData = System.Text.Encoding.ASCII.GetString(RxConnData, 0, RxBytes);
                        ConnStream.Close();
                        NIclient.Close();

                        Thread.Sleep(500); // turnaround time for Labview DLL & .VI invocation logic
                    } // done with "CONNECT" and "BOARDSN" - only do once


                    TcpClient client = new TcpClient(_IP_Addr, _IP_port);
                    // Translate the passed message into ASCII and store it as a Byte array.

                    Byte[] data = System.Text.Encoding.ASCII.GetBytes(sCommandMsg);

                    // Get a client stream for reading and writing.
                    NetworkStream stream = client.GetStream();
                    // Send the command message to the connected TcpServer ("Listener"). 
                    stream.Write(data, 0, data.Length);
                    // String to store the response ASCII representation.
                    String SrvResponseData = String.Empty;


                    if (SplitMsg[1] == "SENDFILE" || SplitMsg[1] == "RECEIVEFILE")  // special case, where we "block" for NI1071 file send response
                    {
                        // Buffer for the NI1071 test result file (ASCII)
                        Byte[] RxData = new Byte[1024 * 1024 * 16];
                        stream.ReadTimeout = 250; // millisec timeout
                        // Read the DATA file...
                        Int32 FileBytesReceived = 0;
                        FileBytesReceived = stream.Read(RxData, 0, RxData.Length);
                        SrvResponseData = System.Text.Encoding.ASCII.GetString(RxData, 0, FileBytesReceived);

                        // append CHANNEL number to file spec
                        string[] FileNameParts = _LabViewFileName.Split('.');  // e.g. DAC_AO.txt, or ADC_AI.txt, which we expand to DAC_AO07 or ADC_AI3.txt
                        string baseFileSpec = FileNameParts[0] + _sParamNum;
                        string CompleteFileSpec = ManufTest_Data_RootFilepath + baseFileSpec + "." + FileNameParts[1];
                        // Two cases - if passed filespec is null, create one for waveforms
                        // if filespec not null, use it as is.
                        if (CapturedFileName == null)
                        {
                            FileNameParts = _LabViewFileName.Split('.');  // e.g. DAC_AO.txt, or ADC_AI.txt, which we expand to DAC_AO07 or ADC_AI3.txt
                            baseFileSpec = FileNameParts[0] + _sParamNum;
                            CompleteFileSpec = ManufTest_Data_RootFilepath + baseFileSpec + "." + FileNameParts[1];
                        }
                        else
                        {
                            CompleteFileSpec = ManufTest_Data_RootFilepath + CapturedFileName;
                        }
                        // store accumulated files to DRIVE file...
                        // ARGS: (fileSpecThisComputer, SrvResponse)
                        //
                        // File.WriteAllText():  Creates a new file, writes the specified string to the file, and then closes the file.
                        // If the target file already exists, it is overwritten.
                        System.IO.File.WriteAllText(CompleteFileSpec, SrvResponseData);
                        // if the NI1071 cannot find the file, the ASCII contents are "FILE_NOT_FOUND"...
                        if (FileBytesReceived <= 14)  // i.e. "FILE_NOT_FOUND" is 14 bytes - impossible size for captured waveform
                        {
                            sResult = "file not found on NI1071 filesystem or size less than 14 bytes";
                        }
                        else
                        {
                            CapturedFileName = CompleteFileSpec; // return to user
                            sResult += string.Format(" [NIPXIe1071 returned file @{0}:  {1} data bytes]", CompleteFileSpec, FileBytesReceived);
                        }
                    }
                    else
                    {
                        // capture the CHANNEL number (when appropriate)
                        if (SplitMsg[1] == "TEST")
                        {
                            // TEST command documentation (update in ALL places)    
                            // transfer of command test data fields to NI1071 DLL, then to params in Labview...
                            //            1         2         3
                            //  0123456789012345678901234567890
                            //  CMD: TEST n cc Test Description follows (can have spaces)
                            //  0    1    2 3  4    5 ... (split array elements)
                            // were (base10) 'n' is single test integer and 'cc' is two digit NI channel number (e.g. 0-31)
                            _sParamNum = (SplitMsg[3]);
                        }

                        // Buffer to store the response bytes.
                        Byte[] RxData = new Byte[256];
                        stream.ReadTimeout = 250; // millisec timeout
                                                  // Read the first batch of the TcpServer response bytes.
                        Int32 bytes = stream.Read(RxData, 0, RxData.Length);
                        SrvResponseData = System.Text.Encoding.ASCII.GetString(RxData, 0, bytes);
                        SrvResponseMsg = "NIPIXe1071: " + SrvResponseData;
                        // Close everything.
                    }
                    stream.Close();
                    client.Close();
                }
                catch (TimeoutException Exc)
                {
                    SrvResponseMsg = string.Format(" Timeout Exception: {0}", Exc.Message);
                    iStatus = -1;
                }
                catch (SocketException e)
                {
                    SrvResponseMsg = string.Format("Labview not Running? or wrong NI1071 IP?: {0}", e.Message);
                    iStatus = -2;
                }
                catch (Exception catchall)
                {
                    string currentFile = new System.Diagnostics.StackTrace(true).GetFrame(0).GetFileName();
                    int currentLine = new System.Diagnostics.StackTrace(true).GetFrame(0).GetFileLineNumber();
                    SrvResponseMsg = string.Format(" Exception @ Line {0} {1} ", currentLine.ToString(), catchall.Message);
                    iStatus = -3;
                }

                if (iStatus == 0)
                    return sResult;
                else
                    return " " + SrvResponseMsg;
            }
        }  // end of class NIPXIe1071_Controller

        // digital breakout box (BOB) test (for 3U BOB only!  32 total DIOs)
        public class DBB1TestAndReport
        {
            private string _sBBox = "DBB1";
            private ushort _usBoardIndex;  // we may have two ThorDAQ boards, where a separate one tests Breakout Boxes
            private AccessFPGAregisters _FPGAreg;
            private Int32 _iLabViewVItestNum;
            private String _BOBversion; // e.g. 1.0.1.2 or 2.x and above for DIO MUX support
            public bool bTestMUXing = false; // only for BOB CPLD 2.x or higher
            public string[] DBB1_RepColHdr = new string[MAXColumnsINREPORT];    // test header ROW
            public string[] DBB1_ReportColValues = new string[MAXColumnsINREPORT]; // a value record

            private string _sTestDesc;  // test EEPROM readback - must start with "AB"
                                        //   private double _dWaveRMSErr_Min = 0.0;
                                        //   private double _dWaveRMSErr_Max = 0.030;
                                        //   private double _dWaveRMSErr_StdDevLimit = 0.005;
                                        //   private const int _PixelSamples = 4096;
                                        //   private string _WaveInputFileSpec = ManufTest_Data_RootFilepath + @"TDADC_DDR3Chan.txt"; // i.e. add '0' - '3'for Chan A - D
                                        //   private AnalyzeNI1071VoltageWaveform analyzeNI1071CapturedWaveform;
            private NIPXIe1071_Controller NI_Computer;

            public DBB1TestAndReport(ushort BoardIndx)  // def. CONSTRUCTOR
            {
                _usBoardIndex = BoardIndx;
                _FPGAreg = new AccessFPGAregisters(0); // (boardIndx 0)
                NI_Computer = new NIPXIe1071_Controller(sIPaddress, IP_port, "NI1071DIOinput.txt");   // e.g. DAC_AO.txt, or ADC_AI.txt, which we expand to (e.g.) DAC_AO02 or ADC_AI07.txt
                _FPGAreg = new AccessFPGAregisters(0); // (boardIndx 0)


                DBB1_RepColHdr[(int)FieldName.TestNum] = "Test#"; DBB1_RepColHdr[(int)FieldName.PassFail] = "P/F"; DBB1_RepColHdr[(int)FieldName.TestDescription] = "Test Description"; DBB1_RepColHdr[(int)FieldName.Units] = "Units"; DBB1_RepColHdr[(int)FieldName.Mean] = "Avg"; DBB1_RepColHdr[(int)FieldName.StdDev] = "StdDev"; DBB1_RepColHdr[(int)FieldName.Min] = "MinLimit"; DBB1_RepColHdr[(int)FieldName.Max] = "MaxLimit"; DBB1_RepColHdr[(int)FieldName.StdDevLimit] = "StdDevLim";
                _sBBox = "DBB1";
                _sTestDesc = _sBBox + " Serial Number: ";
                if (TestReport != null) // Global ManufTest workbook file report exists? (command option)
                {
                    TestReport.WriteWorkSheetRecord(_sBBox, 0, DBB1_RepColHdr); // first write headers
                }

                List<string> BOBstatusStrings = APIGetBOBstatus( new List<string>{" "});
                _BOBversion = "0.0.0.0";
                int iMajorVerNum = 0;
                foreach (String str in BOBstatusStrings) 
                { 
                    if (str.Contains("BOBCPLD ")) 
                    {
                        _BOBversion = str.Substring(8);
                        try
                        {
                            iMajorVerNum = Int32.Parse(_BOBversion.Substring(0, 1)); // 
                        }
                        catch
                        {

                        }
                        if (iMajorVerNum >= 2) bTestMUXing = true;
                        break; // done with strings
                    }
                }
            }
            public string[] GetTestHeader()
            {
                return DBB1_RepColHdr;
            }

            // flash LEDs
            public string TestLEDs()
            {
                string[] LED_ReportColValues = new string[MAXColumnsINREPORT]; // a value record
                LED_ReportColValues[(int)FieldName.TestNum] = "2";  // test# to perform (EEPROM always #1)
                LED_ReportColValues[(int)FieldName.TestDescription] = "Test LED controller I2C Communication"; // 
                
                string retString = "SUCCESS";
                uint status = 0;
                for (int l = 0; l < 32; l++)
                    status |= ThorDAQAPIBreakOutBoxLED(_usBoardIndex, l, 1); // "accumulate" any error (on)
                Thread.Sleep(500);
                for (int l = 0; l < 32; l++)
                    status |= ThorDAQAPIBreakOutBoxLED(_usBoardIndex, l, 0); // off

                LED_ReportColValues[(int)FieldName.PassFail] = (status == 0) ? "PASS" : "FAIL";

                retString = string.Join(" ", LED_ReportColValues);
                if (TestReport != null) // workbook exists...
                {
                    TestReport.WriteWorkSheetRecord(_sBBox, 2, LED_ReportColValues); // commit test record to file
                }
                return retString;
            }

            // check EEPROM: from factory has all 0xFF
            public string TestEEPROMserNum()
            {
                string[] EEPROM_ReportColValues = new string[MAXColumnsINREPORT]; // a value record
                string FunctionStatus = "SUCCESS";
                string passFail = "FAIL";
                EEPROM_ReportColValues[(int)FieldName.TestNum] = "1";  // test# to perform
                EEPROM_ReportColValues[(int)FieldName.TestDescription] = _sTestDesc; // "ABBx Serial ...

                // we may test ThorDAQ on a "golden" BBox setup, or BBox packaged with card
                // of course that effects what we test for
                string DBB1SerNumString = "FailedI2Cread";
                if (GetI2CeepromSerNum(_usBoardIndex, 0x2, 0xFF, 0x50, 0, out DBB1SerNumString))
                {  // success?
                    EEPROM_ReportColValues[(int)FieldName.TestDescription] += DBB1SerNumString;
                    string sBBoxLabel = "DB"; // TBD: check field format for AB serNum  + _BBoxNum.ToString();
                    if (DBB1SerNumString.Substring(0, sBBoxLabel.Length) == sBBoxLabel)
                        passFail = "PASS";
                }
                else
                {
                    FunctionStatus = "FAIL";
                    EEPROM_ReportColValues[(int)FieldName.TestDescription] += DBB1SerNumString;
                }
                EEPROM_ReportColValues[(int)FieldName.PassFail] = passFail;
                if (TestReport != null) // workbook exists...
                {
                    TestReport.WriteWorkSheetRecord(_sBBox, 1, EEPROM_ReportColValues); // commit test record to file
                }
                FunctionStatus = string.Join(" ", EEPROM_ReportColValues);
                return FunctionStatus;
            }

            // Build the trivial DigitalOut waveform
            // "Digital Output waveform" is special case where an 8-bit value -- fed to 16-bit "sample" DAC -- is used
            // to determine the DBB1 MUX's slave "Digital_Output_n" BNC value (hi or low).  Up to 8 (or none) of these
            // MUXed DOs can be assigned to a BNC connector.  We test DO_0 and DO_7 slaves, presuming the rest
            // will work as expected
            //Volts = ((20.0 / 65536.0) * (DACcnts)) - 10.0;
            private string BuildDigitalOutputWaveform(int iType)
            {
                string fileSpec = "InvalidSettings"; // default failure
                double dLowVolts = 0.250;
                // get DAC counts for needed voltage
                ushort DACcnts = (ushort)((dLowVolts + 10.0) / (20.0 / 65536));

                if (iType == 0) // low volts
                {
                    fileSpec = @"c:\temp\DAC_DigOutWave_lowTTL.txt";  // file of base10 DAC counts
                    // Create a file to write to.
                    using (StreamWriter sw = File.CreateText(fileSpec))
                    {
                        int dutyCycle = 0;
                        for (int sample = 0; sample < 50000; sample++) // DAC rate is 1MHz, so waveform is 50ms of real time
                        {
                            if (dutyCycle++ < 10000)
                            {
                                sw.WriteLine("0"); // all Digital_Output_x de-asserted
                            }
                            else
                            {
                                sw.WriteLine("255"); // all Digital_Output_x asserted
                            }
                            if (dutyCycle >= 20000) dutyCycle = 0;
                        }
                    }
                }
                else  // hi volts
                {

                }

                return fileSpec;
            }
            // use ThorDAQAPI to configure the ThorDAQ hardware for testing DBB1
            // We use the ResGalvo mode to test the  with max 4096-pixel line because the "line"
            // is the only real-time sampling opportunity we have to capture the waveform period(s)
            // test only one channel at a time to simplify logic in NI1071 Labview program
            // INPUT:  
            // iTest - integer with test number:
            //          0 - low voltage (250mV) TTL signal test (to be read by NI1071)
            //          1 - hi volts  (2.5V)    " " "
            private string ConfigThorDAQ(int iTest)
            {
                // setup the ThorDAQ hardware...
                string FunctionStatus;
                // affirm the ThorDAQ GlobalScan is STOPped
                GlobalScan(new List<string> { "InternalCmd", "-s" }, null); // make sure ThorDAQ global scan stopped

                // Configure the BOB (3U BOB only!!  NO Legacy support) 
                // DIO BNC  FPGA MUX                In/Out  NI-6363 Channel
                // 0        0x12 Capture Active     OUT     ai12
                // 1        0x13 Aux_GPIO_0         IN      line0(do0)
                // 2        0x16 Aux_GPIO_3         IN      line1(do1)
                // 3        0x0A Digital_Out_0      OUT     ai13        dig waveform
                // 4        0x00 Res Line Trig      IN      ctr0        pulse generator
                // 5        0x07 Ext. Hardware Trig IN      line2(do2)  NI1072 triggers ThorDAQ (2nd)
                // 7        0x11 Digital_Out_7      OUT     ai14        dig waveform
                // 31       (CPLD)                  OUT     pfi0        ThorDAQ triggers NI1071 (1st)
                string[] DIOmuxFields = new string[] { "DBB1_DIO1_MUX", "DBB1_DIO2_MUX", "DBB1_DIO3_MUX", "DBB1_DIO4_MUX", "DBB1_DIO5_MUX", "DBB1_DIO6_MUX", "DBB1_DIO7_MUX", "DBB1_DIO8_MUX" };

                // reset entire AUX_GPO (output) reg
                // DEPRECATED!
                bool bStatus = _FPGAreg.Write("GlobalGPOAuxReg", 0x00);  // clear (reset) FPGA DBB1 defines

                // DEPRECATED!
                bStatus = _FPGAreg.Write("DBB1_DIO1_MUX", 0x12);
                bStatus = _FPGAreg.Write("DBB1_DIO2_MUX", 0x13);
                bStatus = _FPGAreg.Write("DBB1_DIO3_MUX", 0x16);
                bStatus = _FPGAreg.Write("DBB1_DIO4_MUX", 0x0A);
                bStatus = _FPGAreg.Write("DBB1_DIO5_MUX", 0x00);
                bStatus = _FPGAreg.Write("DBB1_DIO6_MUX", 0x07);
                bStatus = _FPGAreg.Write("DBB1_DIO7_MUX", 0x17);
                bStatus = _FPGAreg.Write("DBB1_DIO8_MUX", 0x11);

                // Configure directions on the AUX_GPIO
                bStatus = _FPGAreg.Write("GlobalGPIODirReg", 0x09);  // input mode "1", for output, "0"

                FunctionStatus = ADCSampleClock_setup(new List<string> { "InternalCmd " }).Last();
                FunctionStatus = ADCStream_setup(new List<string> { "InternalCmd ", "-r" }).Last(); // -r for Resonant
                // Res-Galvo mode is "-m 0"
                FunctionStatus = ScanControl_setup(new List<string> { "InternalCmd ", "-m", "0", "-n", "1" }).Last();
                FunctionStatus = S2MM_DMA_setup(new List<string> { "InternalCmd", "-d", "4096x4096", "-c", "0xF", "-n", "1" }).Last();   // (Image DMA not required for DAC waveforms...)

                //                ArgList = new List<string> { "InternalCmd", "-i", "-c 0xFFF", "-z 1000000", "-f", _WaveInputFileSpec }; // always 1MHz sample rate, all channels
                // create the "Digital Wavefile", which is a brain-dead simple single value file either high or low voltage (e.g. simulate TTL level)
                // and to make it even simpler, load ALL "13" channels with same file

                string DOwaveFile;
                DOwaveFile = BuildDigitalOutputWaveform(0);
                FunctionStatus = DACWaveform_setup(new List<string> { "InternalCmd", "-c", "0x1FFF", "-z", "1000000", "-f", DOwaveFile }).Last();  // need "13th channel" Digital waveform
                return FunctionStatus;
            }



            // segregate the MUX output test which we call from within the MUX input test
            // The BNC under test has been configured and set
            // Called to test a single MUXed output
            public string TestMUXedOutput(Int32 iBNCunderTest, Int32 iFPGAindex, int TestNumArg, String sDIOconfCode)  // index 0-31 (1:1 muxing is default; if BNC != FPGA, we are MUXing)
            {
                string sThisFuncStatus = "FAILED: ";
                String sStatus;
                string ServerResponse = null;
                List<string> sConcatenatedStrings = new List<string> { };
                string sNI1071OutputFileSpec = null;
                int iErrors = 0;
                String sMUX_FPGA_test_index = " FPGA index " + iFPGAindex.ToString();
                // transfer of command test data fields to NI1071 DLL, then to params in Labview...
                // (after TEST, 3 arguments delimited by space!)
                //            1         2         3
                //  01234567890123456789012345678901
                //  CMD: TEST n cc TestDescription
                // were (base10) 'n' is single test integer and 'cc' is 2 column alphanumeric NI param 
                _sTestDesc = " MUX: D" + iBNCunderTest.ToString() + " OUTPUT at FPGA's index " + iFPGAindex.ToString();
                DBB1_ReportColValues[(int)FieldName.TestDescription] = _sTestDesc;
                _iLabViewVItestNum = (int)NI1071_LabviewVItestNums.DBB1outputTest;
                // SEND COMMAND to NI1071 to measure input values seen
                string NIcommand = ("CMD: TEST " + _iLabViewVItestNum.ToString() + " " + iBNCunderTest.ToString() + " " + _sTestDesc); // send 32 bit Output value
                sStatus = NI_Computer.SendTCPcommand(NIcommand, ref ServerResponse, ref sNI1071OutputFileSpec);
                Thread.Sleep(1000);  // time for NI1071 to complete command

                // POLL (for protocol timing only - we're not waiting on data from NI1071)
                int iterations = 0;
                do
                {
                    sStatus = NI_Computer.SendTCPcommand("CMD: POLLING ", ref ServerResponse, ref sNI1071OutputFileSpec);
                    Thread.Sleep(250);  // time for NI1071 to create response
                } while (!ServerResponse.Contains("DONE") && (iterations++ < 8));  // give it 2 seconds
                if (sStatus == "SUCCESS")
                {
                    // GET THE Test Data from the NI-1071!!
                    
                    Thread.Sleep(250);
                    // test should be starting!  Can take a few millisecs or over hundred millisecs
                    // When NI program starts test it sets line HIGH, when done, LOW

                    //(IGNORE THE HANDSHAKE for this MUX test because we need all the DIO resources for MUXing
                    // handshake requires D7)
                    Thread.Sleep(1250);

                    /*List<string> DIvalue = new List<string> { };
                    int ItersForLabviewStart = 0;
                    Int32 BOBinputValueAtBNC_D7;
                    do
                    {
                        DIvalue = MTAPIGetDIO(new List<string> { "D7" });
                        BOBinputValueAtBNC_D7 = (Int32.Parse(DIvalue[0].Substring(5, 1))); // PARSE OUT from (e.g.) D03I01, BNC (D3) value is "1"

                        if (BOBinputValueAtBNC_D7 == 1) break;
                        Thread.Sleep(50);
                    } while (++ItersForLabviewStart < 61);
                    if (ItersForLabviewStart > MaxItersForLabviewStart)
                        MaxItersForLabviewStart = ItersForLabviewStart;
                    if (ItersForLabviewStart > 60)
                        return (sThisFuncStatus + " 6331 -> D7 handshake timeout"); // failed HANDSHAKE - can't confirm NI1071 start
                    */

                    // Give LabView program time to set the "Global SubVI Test Programs"
                    // (without this delay, the Labview program will return the result of LAST Test start, immediately)
                    //                        const int LabViewTestStateDelayMS = 1500; // (set by trial and error observations - we cannot dictate speed of Labview/WinOS invocations)
                    //                      Thread.Sleep(LabViewTestStateDelayMS);


                    bool bLabviewTestCompleted = false;
                    // POLL for completion (or timeout) 

                    const int SecondsToPoll = 8;  // should be longer than timeout on the Labview (sub) VI test
                    for (int t = 0; t < SecondsToPoll * 4; t++)
                    {
                        Thread.Sleep(250); // ms polling interval
                        sStatus = NI_Computer.SendTCPcommand("CMD: POLLING ", ref ServerResponse, ref sNI1071OutputFileSpec);
                        //UpdateConsoleStatus(ServerResponse);
                        if (sStatus == "SUCCESS")  // did POLLing command work?
                        {
                            // does server response have DONE, ERROR, or RESET, indicating Test has concluded?
                            if (ServerResponse.Contains("DONE") || ServerResponse.Contains("ERROR") || ServerResponse.Contains("RESET"))
                            {
                                bLabviewTestCompleted = true;
                                break;
                            }
                        }
                        else
                        {
                            // TCP/IP Comm failed in POLLING...
                            sThisFuncStatus += " on CMD: POLLING " + sStatus; // failed to send command
                        }
                    }
                    // FINISHED POLLING... 
                    // SENDFILE -- tell Server (Labview) to send acquired data file
                    // RECEIVEFILE -- tell Server to receive data file to operate on
                    List<string> slDn;
                    if (bLabviewTestCompleted == false)
                    {
                        // we have timed out while POLLING...
                        sThisFuncStatus += " FAIL: TIMEOUT ERROR on POLLING! ";
                    }
                    else  // test completed - DONE (success) or ERROR or SubVI RESET by user?
                    {
                        if (ServerResponse.Contains("DONE") == true)
                        {
                            Thread.Sleep(500);
                            // Send command to NI1071 to SEND us the data file from test...
                            sNI1071OutputFileSpec = null;
                            sStatus = NI_Computer.SendTCPcommand("CMD: SENDFILE ", ref ServerResponse, ref sNI1071OutputFileSpec);

                            if (sStatus.Contains("SUCCESS"))  // PROCEED TO TEST Digital I/O!
                            {
                                // NI1071 has returned result of reading 32 Digital Ins D0-D31...
                                // Compare NI1071 Digital Inputs with expected result
                                // // expect all 32 DIs in a single line tab separated
                                string[] NI1071data = System.IO.File.ReadAllLines(sNI1071OutputFileSpec);
                                slDn = NI1071data[0].Split('\t').ToList(); // slDn[i], where i is 0-31 
                                // we should now have the list Dn input values!
                                // Only need to test our "iBNCunderTest" of interest...
                                iErrors = (slDn[iBNCunderTest] != "1") ? iErrors++ : 0; 
                            }
                            else // failure, like a "locked" data file used by another process, etc.
                            {
                                iErrors++; // accumulate error count
                            }
                        }
                        else if (ServerResponse.Contains("ERROR") == true)
                        {
                            iErrors++; // accumulate error count
                        }
                        else if (ServerResponse.Contains("RESET") == true)
                        {
                            iErrors++; // accumulate error count
                        }
                    }

                    DBB1_ReportColValues[(int)FieldName.PassFail] = (iErrors == 0) ? "PASS" : "FAIL"; // DEFAULT is fail

                    sThisFuncStatus = string.Join(" ", DBB1_ReportColValues);
                    if (TestReport != null) // workbook exists...
                    {
                        TestReport.WriteWorkSheetRecord("DBB1", TestNumArg, DBB1_ReportColValues); // commit test record to worksheet
                    }

                    // FINALLY, reset the NI1071 to prepare for next test
                    NI_Computer.SendTCPcommand("CMD: RESET ", ref ServerResponse, ref sNI1071OutputFileSpec);

                    //List<string> ArgList = new List<string> { "InternalCmd ", "-s" };
                    //sStatus = GlobalScan(ArgList, null).Last();  // STOP!
                }
                else  // (failure)
                {
                    DBB1_ReportColValues[(int)FieldName.PassFail] = "FAIL"; // failed to send command
                }
                sThisFuncStatus = string.Join(" ", DBB1_ReportColValues);  // complete test record
                
                return sThisFuncStatus;
            }



                // TEST A SINGLE Digital Input from ThorDAQ at the BOB's BNC
                // This test sets the NI-1071 (PXIe 6363) Port 0 (32 DIOs) as OUTUPS so we can
                // test each input Dn, test that it comes on(or off), and that
                // no other BNC is effected (no cross talk, no solder bridges, etc.)
                // We send the 32-bit value of the NI-1071 DO, the read it back at our BOB BNCs 
                // through the POGO fixture
                // Normally Omit D7 and D31 because these are special PFI cutouts
            public string ProgramAndAnalyzeDBB1asInputs(Int32 iBNCunderTest, Int32 iFPGAindex)  // index 0-31 (1:1 muxing is default; if BNC != FPGA, we are MUXing)
            {
                string ServerResponse = null;
                string sThisFuncStatus = "FAILED: ";
                List<string> sConcatenatedStrings = new List<string> { };
                string sNI1071OutputFileSpec = null;
                int _testNum = 65 + iBNCunderTest;    // i.e. needed for spreadsheet lines
                bool bMUXing = false;

                // if we are MUXing, add another 31 to test numbers
                string sSetOrClear = " SET";  // if test fixture connects to BNC, and hardware works, it pulls it low, so just test "1" state
                _sTestDesc = "3U BOB INPUT BNC: D" + iBNCunderTest.ToString() + sSetOrClear;
                if (iBNCunderTest != iFPGAindex)
                {
                    bMUXing = true;
                    _sTestDesc = "MUX (Input) BNC: D" + iBNCunderTest.ToString() + sSetOrClear;
                    if ( !bTestMUXing)
                    {
                        // BOB CPLD ver. does not support MUX (or ver. string failed to report)
                        return (sThisFuncStatus + "FAILED:  MUXing not supported in BOB's CPLD version "); // failed
                    }
                    // MUX spreadsheet lines start at 96
                    _testNum = 96 + (iBNCunderTest - 8) * 16 + (iFPGAindex * 2);
                }

                // turn on LED for DIO under test (Note: SharedEnums.cs enum defines Int32 0 - 31 for D0-31)
                ThorDAQAPIBreakOutBoxLED(_usBoardIndex, iBNCunderTest, 1); // LED on
                Thread.Sleep(250);  // we have mind numbingly incomprehensible timing problems with Labview compounded by TCP/IP variability

                // the following horrible looking logic is necessary because the FPGA only give us FIVE programmable
                // DIOs, rather than the 8 we need to easily test all 8 "high speed" FPGA-controlled DIO.
                // (The "slow" DIOs controlled by the CPLD are trivial to test)

                // what are the expected states for this test?
                //UInt32 NI1071DigitalOutSetting;

                //NI1071DigitalOutSetting = (UInt32)(1 << iBNCunderTest);     // NI1071 asserts all low, BNC under test is high

                DBB1_ReportColValues[(int)FieldName.TestDescription] = _sTestDesc;
                // prepare result/Spreadsheet report record
                DBB1_ReportColValues[(int)FieldName.TestNum] = (_testNum).ToString();  // test# we are doing...
                string sStatus = "FAIL";

                // "RESET" the NI1071 
                // ThorDAQ hardware is programmed and ready...
                // make sure NI 1071 is reset
                sStatus = NI_Computer.SendTCPcommand("CMD: RESET ", ref ServerResponse, ref sNI1071OutputFileSpec);
                if (sStatus != "SUCCESS")
                {
                    return (sThisFuncStatus + " on CMD: RESET " + sStatus); // failed
                }
                List<string> SetConfStatus = new List<string> { };
                // CONFIGURE the ThorDAQ's 3U BOB for all OUTPUTs 
                //TEST!!!
                int iI2Cerrs = 0;
                string sDIOconfCode = "99";  // default D0-D3
                if (iFPGAindex > 3)
                    sDIOconfCode = "100";
                SetConfStatus = MTAPISetDIOConfig(new List<string> { sDIOconfCode, "i" }); // (we already tested DBB1 I2C bus - assume it works)
                if (!SetConfStatus.Contains("SUCCESS"))
                    iI2Cerrs++;

                if (iI2Cerrs != 0)
                    return ("FAILED: I2C comm error count:  " + iI2Cerrs.ToString()); // failed

                if (!bMUXing)
                {
                    // transfer of command test data fields to NI1071 DLL, then to params in Labview...
                    // (after TEST, 3 arguments delimited by space!)
                    //            1         2         3
                    //  01234567890123456789012345678901
                    //  CMD: TEST n cc TestDescription
                    // were (base10) 'n' is single test integer and 'cc' is 2 column alphanumeric NI param 
                    _iLabViewVItestNum = (int)NI1071_LabviewVItestNums.DBB1inputTest;
                    string NIcommand = ("CMD: TEST " + _iLabViewVItestNum.ToString() + " " + iBNCunderTest.ToString() + " " + _sTestDesc); // send 32 bit Output value
                    sStatus = NI_Computer.SendTCPcommand(NIcommand, ref ServerResponse, ref sNI1071OutputFileSpec);
                    Thread.Sleep(1000);  // time for NI1071 to complete command

                    // POLL (for protocol timing only - we're not waiting on data from NI1071)
                    int iterations = 0;
                    do
                    {
                        sStatus = NI_Computer.SendTCPcommand("CMD: POLLING ", ref ServerResponse, ref sNI1071OutputFileSpec);
                        Thread.Sleep(250);  // time for NI1071 to create response
                    } while (!ServerResponse.Contains("DONE") && (iterations++ < 8));  // give it 2 seconds
                    if (sStatus == "SUCCESS")
                    {

                        // test the result
                        int iDIO, iErrors = 0;
                        List<string> slDIs = new List<string> { };
                        for (iDIO = 0; iDIO < 31; iDIO++) // skip PFI-connected D31
                        {
                            if (iDIO == 7) continue; // skip the PFI-connected signal
                                                     // Now begin the LOOP to test each individual Digital Input...

                            // we must test for exclusion of some "high speed" FPGA DOs
                            // because we can't test all 8 at once!
                            // also, some POGO Digital BNCs are cut out for passthrough connection to NI's PFIx operation
                            // so we're can't test those here

                            if (sDIOconfCode == "99") // test D0-D3
                            {
                                if (iDIO >= 4 && iDIO < 8) continue;
                            }
                            else // testing D4-D7
                                if (iDIO < 4) continue;

                            UInt32 BOBinputValueAtBNC;
                            string sD = "D" + iDIO.ToString();
                            slDIs = MTAPIGetDIO(new List<string> { sD }); // get logic level
                            try
                            {
                                BOBinputValueAtBNC = (UInt32.Parse(slDIs[0].Substring(5, 1))); // PARSE OUT from (e.g.) D11I01, BNC (D11) value is "1"
                                                                                               // we read "xx" instead of binary value when FPGA's MUX isn't AUX GPIO
                            }
                            catch
                            {
                                BOBinputValueAtBNC = 0;  // we should distinguish between error and 0
                            }
                            // test bit value written vs. bit value read (0 or 1)
                            if (iDIO == iBNCunderTest) // should be "1"
                            {
                                if (BOBinputValueAtBNC != 0x1)
                                {
                                    iErrors++; // accumulate error count
                                }
                            }
                            else // should be "0"
                            {
                                if (BOBinputValueAtBNC != 0)
                                {
                                    iErrors++; // accumulate error count
                                }

                            }
                        }
                        // prepare test record...
                        DBB1_ReportColValues[(int)FieldName.PassFail] = (iErrors == 0) ? "PASS" : "FAIL"; // DEFAULT is fail

                        sThisFuncStatus = string.Join(" ", DBB1_ReportColValues);
                        if (TestReport != null) // workbook exists...
                        {
                            TestReport.WriteWorkSheetRecord("DBB1", _testNum, DBB1_ReportColValues); // commit test record to worksheet
                        }

                        // FINALLY, reset the NI1071 to prepare for next test
                        NI_Computer.SendTCPcommand("CMD: RESET ", ref ServerResponse, ref sNI1071OutputFileSpec);

                        //List<string> ArgList = new List<string> { "InternalCmd ", "-s" };
                        //sStatus = GlobalScan(ArgList, null).Last();  // STOP!
                    }
                    else  // (failure)
                    {
                        DBB1_ReportColValues[(int)FieldName.PassFail] = "FAIL"; // failed to send command
                    }
                    sThisFuncStatus = string.Join(" ", DBB1_ReportColValues);  // complete test record
                    sConcatenatedStrings.Add(sThisFuncStatus);
                } // end of not MUXing
                //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM  MUX MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
                else  // MUX case
                {
                    // send the individual SetDIOConfig command for the MUX (all DIO are already 1:1 inputs)
                    int iMUX, iAUXindex;
                    switch (iFPGAindex) // i.e., only 
                    {
                        case 0:
                        case 4:
                            iMUX = 27; iAUXindex = 0;
                            break;
                        case 1:
                        case 5:
                            iMUX = 28; iAUXindex = 1;
                            break;
                        case 2:
                        case 6:
                            iMUX = 29; iAUXindex = 2;
                            break;
                        case 3:
                        case 7:
                            iMUX = 30; iAUXindex = 3;
                            break;
                        default:
                            iMUX = 27; iAUXindex = 0;
                            break;
                    }
                    // MUX the BNC under test!
                    // Define the MUX for INPUT
                    SetConfStatus = MTAPISetDIOConfig(new List<string> { iBNCunderTest.ToString(), "i", iFPGAindex.ToString(), iMUX.ToString(), iAUXindex.ToString() }); // (we already tested DBB1 I2C bus - assume it works)
                    if (!SetConfStatus.Contains("SUCCESS"))
                        iI2Cerrs++;
                    // NOW, the FPGAindex we have MUXed WAS PREVIOUSLY a 1:1 INPUT, but that BNC[n] <-> FPGA[n] connection is broken;
                    // consequently we SWAP the BNC/FPGA indexes and set that BNC (with FPGA index) output to "0"
                    // (use the 5th AUX GPIO (31) which is not used in the set of 4 AUX GPIOs we work with)
                    // and that should match the expected "0" values of all BNCs
                    // EXCEPT the one under test, which should be the only BNC at "1".
                    // Define the MUX for OUTPUT (the inverse of indexes, e.g. in INPUT is D9 driving FPGA 2, then OUTPUT is D2 driven by FPGA 9
                    SetConfStatus = MTAPISetDIOConfig(new List<string> { iFPGAindex.ToString(), "o", iBNCunderTest.ToString(), "31", "4" }); // (we already tested DBB1 I2C bus - assume it works)
                    if (!SetConfStatus.Contains("SUCCESS"))
                        iI2Cerrs++;
                    // give up if CPLD config command failed
                    if (iI2Cerrs != 0)
                        return ("FAILED: I2C comm error count:  " + iI2Cerrs.ToString()); // failed

                    // TEST THE MUX in OUTPUT DIRECTION before the INPUT
                    string sMUXoutTestResult;
                    // remember - DIR and VALUE are always tracked according to BNC index; for OUTPUT we swapped
                    // indexes compared to INPUT
                    MTAPISetDO(new List<string> { "D"+ iFPGAindex.ToString(), "1" }); // Set HIGH
                    // SWITCH indexes:  iFPGAindex is now the BNC of output
                    sMUXoutTestResult = TestMUXedOutput(iFPGAindex, iBNCunderTest, _testNum++, sDIOconfCode); // increment testNum for INPUT MUX dir
                    

                    /// INPUT //////////////
                    // NOW do the MUX, INPUT direction
                    // transfer of command test data fields to NI1071 DLL, then to params in Labview...
                    // (after TEST, 3 arguments delimited by space!)
                    //            1         2         3
                    //  01234567890123456789012345678901
                    //  CMD: TEST n cc TestDescription
                    // were (base10) 'n' is single test integer and 'cc' is 2 column alphanumeric NI param 
                    _sTestDesc = " MUX: D" + iBNCunderTest.ToString() + " input at FPGA's GPIO index " + iFPGAindex.ToString() + sSetOrClear;
                    DBB1_ReportColValues[(int)FieldName.TestNum] = (_testNum).ToString();  // test# we are doing (incremented from OUTPUT dir)
                    DBB1_ReportColValues[(int)FieldName.TestDescription] = _sTestDesc;
                    _iLabViewVItestNum = (int)NI1071_LabviewVItestNums.DBB1inputTest;
                    string NIcommand = ("CMD: TEST " + _iLabViewVItestNum.ToString() + " " + iBNCunderTest.ToString() + " " + _sTestDesc); // send 32 bit Output value
                    sStatus = NI_Computer.SendTCPcommand(NIcommand, ref ServerResponse, ref sNI1071OutputFileSpec);
                    Thread.Sleep(1000);  // time for NI1071 to complete command

                    // POLL (for protocol timing only - we're not waiting on data from NI1071)
                    int iterations = 0;
                    do
                    {
                        sStatus = NI_Computer.SendTCPcommand("CMD: POLLING ", ref ServerResponse, ref sNI1071OutputFileSpec);
                        Thread.Sleep(250);  // time for NI1071 to create response
                    } while (!ServerResponse.Contains("DONE") && (iterations++ < 8));  // give it 2 seconds
                    if (sStatus == "SUCCESS")
                    {

                        // test the result
                        int iDIO, iErrors = 0;
                        String sMUX_FPGA_failure_index = " FPGAindex " + iFPGAindex.ToString();
                        string sD = "Dnn"; // the BNC "Dnn" string
                        List<string> slDIs = new List<string> { };
                        // READBACK TEST LOOP!  Read/Verify correct/expected values
                        for (iDIO = 0; iDIO < 15; iDIO++) // skip PFI-connected D7 <-> D15
                        {
                            // we must test for exclusion of some "high speed" FPGA DOs
                            // because we can't test all 8 at once!
                            // also, some POGO Digital BNCs are cut out for passthrough connection to NI's PFIx operation
                            // so we're can't test those here

                            if (sDIOconfCode == "99") // test D0-D3
                            {
                                if (iDIO >= 4 && iDIO < 8) continue;
                            }
                            else // testing D4-D7
                                if (iDIO < 4) continue;

                            UInt32 BOBinputValueAtBNC;
                            sD = "D" + iDIO.ToString();
                            slDIs = MTAPIGetDIO(new List<string> { sD }); // get logic level - input or last written output
                            try
                            {
                                BOBinputValueAtBNC = (UInt32.Parse(slDIs[0].Substring(5, 1))); // PARSE OUT from (e.g.) D11I01, BNC (D11) value is "1"
                                                                                               // we read "xx" instead of binary value when FPGA's MUX isn't AUX GPIO
                            }
                            catch
                            {
                                BOBinputValueAtBNC = 0;  // we should distinguish between error and 0 - but how?
                            }
                            // test bit value written vs. bit value read (0 or 1)
                            if ((iDIO == iBNCunderTest) || iDIO == iFPGAindex) // BNC for Input and (FPGAindex)-BNC for Output should be "1", all others "0"
                            {
                                if (BOBinputValueAtBNC != 0x1)
                                {
                                    iErrors++; //  error count
                                    break;
                                }
                            }
                            else // should be "0"
                            {
                                if (BOBinputValueAtBNC != 0)
                                {
                                    iErrors++; //  error count
                                    break;
                                }
                            }
                        } // end READBACK test loop through D8-D15
                        // prepare test record...
                        DBB1_ReportColValues[(int)FieldName.PassFail] = (iErrors == 0) ? "PASS" : "FAIL" ; // DEFAULT is fail

                        sThisFuncStatus = string.Join(" ", DBB1_ReportColValues);
                        if (TestReport != null) // workbook exists...
                        {
                            TestReport.WriteWorkSheetRecord("DBB1", _testNum, DBB1_ReportColValues); // commit test record to worksheet
                        }

                        // FINALLY, reset the NI1071 to prepare for next test
                        NI_Computer.SendTCPcommand("CMD: RESET ", ref ServerResponse, ref sNI1071OutputFileSpec);

                        //List<string> ArgList = new List<string> { "InternalCmd ", "-s" };
                        //sStatus = GlobalScan(ArgList, null).Last();  // STOP!
                    }
                    else  // (failure)
                    {
                        DBB1_ReportColValues[(int)FieldName.PassFail] = "FAIL"; // failed to send command
                    }
                    sConcatenatedStrings.Add(sMUXoutTestResult);
                    sThisFuncStatus = string.Join(" ", DBB1_ReportColValues);  // complete test record
                    sConcatenatedStrings.Add(sThisFuncStatus);


                }
                // turn LED off
                ThorDAQAPIBreakOutBoxLED(_usBoardIndex, iBNCunderTest, 0); // (0 is off)
                return string.Join(Environment.NewLine, sConcatenatedStrings);
            }



            // TEST A SINGLE Digital Output from ThorDAQ at the BOB's BNC
            // This test sets the NI-1071 (PXIe 6363) Port 0 (32 DIOs) as inputs so we can
            // test a single FPGA-written output Dn, test that it comes on(or off), and that
            // no other BNC is effected (no cross talk, no solder bridges, etc.)
            // REQUIRES NI1071 to send back a file with the 32-bit value of the DIOs
            // it has read for the test.
            // Our Lattice CPLD (when working) sources no more than 6 mA per bi-dir pin.
            // 
            // In error case of weak/failed HDMI cable 5V current supply, we could see problem,
            // but typically current problems manifest more in LED problems.
            int MaxItersForLabviewStart = 0;
            public string ProgramAndAnalyzeDBB1asOutputs(Int32 iBNCunderTest, Int32 FPGAindex)
            {
                int iErrors = 0;
                string ServerResponse = null;
                string sThisFuncStatus = "FAILED: ";
                List<string> sConcatenatedStrings = new List<string> { };
                string sNI1071OutputFileSpec = null;
                int _testNum = iBNCunderTest * 2 + 3;    // i.e. BNC 0 is test 3 for SET, test 4 for CLEAR in spreadsheet lines

                // the following horrible looking logic is necessary because the FPGA only give us FIVE programmable
                // DIOs, rather than the 8 we need to easily test all 8 "high speed" FPGA-controlled DIO.
                // (The "slow" DIOs controlled by the CPLD are trivial to test)
                // The basic approach is to ignore the state of  DO5 - DO7 when testing
                // DO0 - DO4 or D8 - D30 (if D31 is defective triggering will fail on other tests)
                // When testing DO5 - DO7, we ignore DO0 - DO4

                bool sTestingD4thruD7 = (iBNCunderTest >= 4 && iBNCunderTest <= 7) ? true : false; // if true, ignore D0-D3
                int BOBinputValueAtBNC_D7, ItersForLabviewStart = 0;

                for (int iClearOrSetBNCvalue = 0; iClearOrSetBNCvalue < 2; iClearOrSetBNCvalue++, _testNum++) // runs twice - once with BNC low, then BNC high
                {
                    // what are the expected states for this test?
                    string sUnchangedDIstate = (iClearOrSetBNCvalue == 1) ? "0" : "1";  // if we are SETTING to one, all others should remain "0" and vice versa
                    string sSetOrClear = (iClearOrSetBNCvalue == 0) ? " CLEAR" : " SET";
                    _sTestDesc = "3U BOB OUTPUT BNC: D" + iBNCunderTest.ToString() + sSetOrClear;
                    DBB1_ReportColValues[(int)FieldName.TestDescription] = _sTestDesc;
                    // prepare result/Spreadsheet report record
                    DBB1_ReportColValues[(int)FieldName.TestNum] = (_testNum).ToString();  // test# we are doing...
                    string sStatus = "FAIL";

                    // "RESET" the NI1071 
                    // ThorDAQ hardware is programmed and ready...
                    // make sure NI 1071 is reset
                    sStatus = NI_Computer.SendTCPcommand("CMD: RESET ", ref ServerResponse, ref sNI1071OutputFileSpec);
                    if (sStatus != "SUCCESS")
                    {
                        return (sThisFuncStatus + " on CMD: RESET " + sStatus); // failed
                    }

                    List<string> slI2Cstatus = new List<string> { };
                    // CONFIGURE the ThorDAQ's 3U BOB for all OUTPUTs 
                    string sDIOconfigRange = "99";
                    if (sTestingD4thruD7) //
                        sDIOconfigRange = "100";
                    slI2Cstatus = MTAPISetDIOConfig(new List<string> { sDIOconfigRange, "o" }); // (we already tested DBB1 I2C bus - assume it works)
                    if (!slI2Cstatus.Contains("SUCCESS"))
                        return (sThisFuncStatus + " Configuring Outputs across I2C"); // failed config

                    // Now begin the LOOP to test each individual Digital Input...
                    string sDn = "D" + iBNCunderTest.ToString();
                    // set all outputs to "0"
                    bool bStatus;
                    for (int i = 0; i < 31; i++)
                    {
                        string sD = "D" + i.ToString();
                        bStatus = MTAPISetDO(new List<string> { sD, sUnchangedDIstate }); // set desired static/uchanged logic level
                        if (bStatus == false)
                        {
                            return (sThisFuncStatus + " ERROR setting BOB output " + sD); // failed config
                        }
                    }
                    bStatus = MTAPISetDO(new List<string> { sDn, iClearOrSetBNCvalue.ToString() });  // alter for NI1071 test
                    if (!slI2Cstatus.Contains("SUCCESS"))
                        return (sThisFuncStatus + " Configuring D0 across I2C"); // failed config
                    // if we are CLEARing the line, the low-pass filter capacitor on our PCB may hold the line
                    // high for hundreds of millisecs - so add a delay in the case of clearing
                    // Delay also useful for Labview sync
                    Thread.Sleep(500);
                    // now IRRESPECTIVE of BNC under test, configure D7 as the Input to receive
                    // "handshake" from NI-1071 that test is in progress!  VI program asserts HI on start, LOW on complete
                    MTAPISetDIOConfig(new List<string> { "7", "i", "7", "31", "4" }); // MUX is AUX GPIO 4
                    MTAPISetDIOConfig(new List<string> { "31", "i", "31", "48" });    // Avoid PXI-6363 I/O contention


                    // Setup the NI 1071 computer, then "GlobalStart" START ThorDAQ to trigger start of test

                    // transfer of command test data fields to NI1071 DLL, then to params in Labview...
                    //            1         2         3
                    //  0123456789012345678901234567890
                    //  CMD: TEST n cc TestDescription
                    // were (base10) 'n' is single test integer and 'cc' is two digit NI channel number (e.g. 0-31)
                    _iLabViewVItestNum = (int)NI1071_LabviewVItestNums.DBB1outputTest;
                    string NIcommand = ("CMD: TEST " + _iLabViewVItestNum.ToString() + " " + iBNCunderTest.ToString() + " " + _sTestDesc);
                    sStatus = NI_Computer.SendTCPcommand(NIcommand, ref ServerResponse, ref sNI1071OutputFileSpec);
                    if (sStatus == "SUCCESS")
                    {
                        Thread.Sleep(250);
                        // test should be starting!  Can take a few millisecs or over hundred millisecs
                        // When NI program starts test it sets line HIGH, when done, LOW
                        List<string> DIvalue = new List<string> { };

                        do
                        {
                            DIvalue = MTAPIGetDIO(new List<string> { "D7" });
                            BOBinputValueAtBNC_D7 = (Int32.Parse(DIvalue[0].Substring(5, 1))); // PARSE OUT from (e.g.) D03I01, BNC (D3) value is "1"

                            if (BOBinputValueAtBNC_D7 == 1) break;
                            Thread.Sleep(50);
                        } while (++ItersForLabviewStart < 61);
                        if (ItersForLabviewStart > MaxItersForLabviewStart)
                            MaxItersForLabviewStart = ItersForLabviewStart;
                        if (ItersForLabviewStart > 60)
                            return (sThisFuncStatus + " 6331 -> D7 handshake timeout"); // failed HANDSHAKE - can't confirm NI1071 start


                        // Give LabView program time to set the "Global SubVI Test Programs"
                        // (without this delay, the Labview program will return the result of LAST Test start, immediately)
                        //                        const int LabViewTestStateDelayMS = 1500; // (set by trial and error observations - we cannot dictate speed of Labview/WinOS invocations)
                        //                      Thread.Sleep(LabViewTestStateDelayMS);


                        bool bLabviewTestCompleted = false;
                        // POLL for completion (or timeout) 

                        const int SecondsToPoll = 8;  // should be longer than timeout on the Labview (sub) VI test
                        for (int t = 0; t < SecondsToPoll * 4; t++)
                        {
                            Thread.Sleep(250); // ms polling interval
                            sStatus = NI_Computer.SendTCPcommand("CMD: POLLING ", ref ServerResponse, ref sNI1071OutputFileSpec);
                            //UpdateConsoleStatus(ServerResponse);
                            if (sStatus == "SUCCESS")  // did POLLing command work?
                            {
                                // does server response have DONE, ERROR, or RESET, indicating Test has concluded?
                                if (ServerResponse.Contains("DONE") || ServerResponse.Contains("ERROR") || ServerResponse.Contains("RESET"))
                                {
                                    bLabviewTestCompleted = true;
                                    break;
                                }
                            }
                            else
                            {
                                // TCP/IP Comm failed in POLLING...
                                sThisFuncStatus += " on CMD: POLLING " + sStatus; // failed to send command
                            }
                        }
                        // FINISHED POLLING... 
                        // SENDFILE -- tell Server (Labview) to send acquired data file
                        // RECEIVEFILE -- tell Server to receive data file to operate on

                        if (bLabviewTestCompleted == false)
                        {
                            // we have timed out while POLLING...
                            sThisFuncStatus += " FAIL: TIMEOUT ERROR on POLLING! ";
                        }
                        else  // test completed - DONE (success) or ERROR or SubVI RESET by user?
                        {
                            if (ServerResponse.Contains("DONE") == true)
                            {
                                Thread.Sleep(500);
                                // Send command to NI1071 to SEND us the data file from test...
                                sNI1071OutputFileSpec = null;
                                sStatus = NI_Computer.SendTCPcommand("CMD: SENDFILE ", ref ServerResponse, ref sNI1071OutputFileSpec);

                                if (sStatus.Contains("SUCCESS"))  // PROCEED TO TEST Digital I/O!
                                {
                                    // NI1071 has returned result of reading 32 Digital Ins D0-D31...
                                    // Compare NI1071 Digital Inputs with expected result
                                    // // expect all 32 DIs in a single line tab separated
                                    string[] NI1071data = System.IO.File.ReadAllLines(sNI1071OutputFileSpec);
                                    List<string> slDn = NI1071data[0].Split('\t').ToList(); // slDn[i], where i is 0-31 
                                    int iDIO;
                                    for (iDIO = 0; iDIO < 31; iDIO++)
                                    {
                                        if (iDIO == 7) continue; // D7 is "cut out" of Pogo for PFI use; can't test it
                                        // we must test for exclusion of some "high speed" FPGA DOs
                                        // because we can't test all 8 at once!
                                        if (sTestingD4thruD7)
                                        {
                                            if (iDIO < 4) continue;
                                        }
                                        else
                                        {
                                            if (iDIO >= 4 && iDIO <= 7) continue; // we cannot test D5 -D7 when testing D0-D4
                                        }
                                        if (iDIO == iBNCunderTest) // test this value for what we're changing (0 or 1)
                                        {
                                            if (iClearOrSetBNCvalue.ToString() != slDn[iDIO])
                                            {
                                                iErrors++; // accumulate error count
                                            }
                                        }
                                        else // test the static/unchanged value
                                        {

                                            if (slDn[iDIO] != sUnchangedDIstate)
                                            {
                                                iErrors++; // accumulate error count
                                            }
                                        }
                                    }
                                }
                                else // failure, like a "locked" data file used by another process, etc.
                                {
                                    iErrors++; // accumulate error count
                                }
                            }
                            else if (ServerResponse.Contains("ERROR") == true)
                            {
                                iErrors++; // accumulate error count
                            }
                            else if (ServerResponse.Contains("RESET") == true)
                            {
                                iErrors++; // accumulate error count
                            }
                        }
                        // prepare test record...
                        DBB1_ReportColValues[(int)FieldName.PassFail] = (iErrors == 0) ? "PASS" : "FAIL"; // DEFAULT is fail

                        sThisFuncStatus = string.Join(" ", DBB1_ReportColValues);

                        if (TestReport != null) // workbook exists...
                        {
                            TestReport.WriteWorkSheetRecord("DBB1", _testNum, DBB1_ReportColValues); // commit test record to worksheet
                        }

                        // FINALLY, reset the NI1071 to prepare for next test
                        NI_Computer.SendTCPcommand("CMD: RESET ", ref ServerResponse, ref sNI1071OutputFileSpec);

                        //List<string> ArgList = new List<string> { "InternalCmd ", "-s" };
                        //sStatus = GlobalScan(ArgList, null).Last();  // STOP!
                    }
                    else  // (failure)
                    {
                        DBB1_ReportColValues[(int)FieldName.PassFail] = "FAIL"; // failed to send command
                    }
                    sThisFuncStatus = string.Join(" ", DBB1_ReportColValues);  // complete test record
                    sConcatenatedStrings.Add(sThisFuncStatus);

                    // pause for next test
                    Thread.Sleep(500);
                } // clear/set FOR loop

                return string.Join(Environment.NewLine, sConcatenatedStrings);
            }
        }

        // Note that the MAX127 "slow" ADC runs of I2C bus on ABB3
        public class ABBxTestAndReport  // analog breakout box test 
        {
            private ushort usBoardIndex;
            private int _BBoxNum;
            private string _sBBox;
            public string[] ABBx_RepColHdr = new string[MAXColumnsINREPORT];    // test header ROW
            public string[] ABBx_ReportColValues = new string[MAXColumnsINREPORT]; // a value record
            private int _iLabViewVItestNum = (int)NI1071_LabviewVItestNums.ABBxTest; // .VI program index known to NI1071 Labview
            private string _sTestDesc;  // test EEPROM readback - must start with "AB"
            public NIPXIe1071_Controller NI_Computer;  // constructor can fail when Labview not running/answering

            public ABBxTestAndReport(ushort BoardIndex, int BBnum)  // def. CONSTRUCTOR
            {
                usBoardIndex = BoardIndex;
                NI_Computer = new NIPXIe1071_Controller(sIPaddress, IP_port, "devnull.txt");   // i.e. file not used
                if (NI_Computer.SrvResp != "SUCCESS")
                    return;

                _BBoxNum = BBnum;
                ABBx_RepColHdr[(int)FieldName.TestNum] = "Test#"; ABBx_RepColHdr[(int)FieldName.PassFail] = "P/F"; ABBx_RepColHdr[(int)FieldName.TestDescription] = "Test Description"; ABBx_RepColHdr[(int)FieldName.Units] = "Units"; ABBx_RepColHdr[(int)FieldName.Mean] = "Avg"; ABBx_RepColHdr[(int)FieldName.StdDev] = "StdDev"; ABBx_RepColHdr[(int)FieldName.Min] = "MinLimit"; ABBx_RepColHdr[(int)FieldName.Max] = "MaxLimit"; ABBx_RepColHdr[(int)FieldName.StdDevLimit] = "StdDevLim";
                _sBBox = "ABB" + _BBoxNum.ToString();
                _sTestDesc = _sBBox + " Serial Number: ";
                if (TestReport != null) // Global ManufTest workbook file report exists? (command option)
                {
                    TestReport.WriteWorkSheetRecord(_sBBox, 0, ABBx_RepColHdr); // first write headers
                }
            }
            public string[] GetTestHeader()
            {
                return ABBx_RepColHdr;
            }

            // for ABB3, test the "slow" MAX127 voltages
            // All 8 of the MAX127 ADC input BNCs are connected in parallel on the 3U BOB POGO test fixture,
            // so when we output a static voltage (integer between +/- 9 volts), all 8 of the BNCs see same voltage
            // So process is:
            // Start Test #5
            //   Set passed iNI1071_Volts as positive Voltage to NI1071 AO via TCP command
            //   Use API to read the 8 voltages AI6 - AI13 via I2C
            //   Test voltage for limits and write 8 "rows" for result
            //   Repeat with negative iNI1071_Volts voltage

            public string TestMAX127_ADC(int iNI1071_Volts)
            {
                List<string> sConcatenatedStrings = new List<string> { " " + Environment.NewLine };
                double dVoltageAccuracy = 0.005; // percentage expected of ADC
                string ServerResponse = null;
                string sThisFuncStatus = "FAILED";
                string sNI1071FileSpecNotUsed = "NULL";
                string[] MAX127chan_ReportColValues = new string[MAXColumnsINREPORT]; // a value record
                string sStatus = "SUCCESS";
                int _testNum = 2;  // starting testnum: 1 is SerNum


                // For the positive, then negative, test voltage setting...
                // CONNECT to NI-1071 and setup the static voltage...


                sStatus = NI_Computer.SendTCPcommand("CMD: RESET ", ref ServerResponse, ref sNI1071FileSpecNotUsed);
                if (sStatus != "SUCCESS")
                {
                    return (sThisFuncStatus + " " + sStatus); // failed
                }
                sThisFuncStatus = _testNum.ToString();
                // Setup the NI 1071 computer - "GlobalStart" and "trigger" not needed because we
                // merely set up a static voltage
                _sTestDesc = "ABB3's MAX127 ADCs AI6-13 all at " + iNI1071_Volts.ToString("D2") + " Volts";


                for (int iTrial = 0; iTrial < 2; iTrial++, iNI1071_Volts *= -1)
                {
                    // transfer of command test data fields to NI1071 DLL, then to params in Labview...
                    //            1         2         3
                    //  0123456789012345678901234567890
                    //  CMD: TEST n cc TestDescription
                    // were (base10) 'n' is single test integer and 'cc' is two digit NI channel number (e.g. 0-31)
                    string NIcommand = ("CMD: TEST " + _iLabViewVItestNum.ToString() + " " + iNI1071_Volts.ToString("D2") + " " + _sTestDesc);
                    sStatus = NI_Computer.SendTCPcommand(NIcommand, ref ServerResponse, ref sNI1071FileSpecNotUsed);

                    if (sStatus != "SUCCESS")
                    {
                        return (sThisFuncStatus + " " + sStatus); // failed
                    }
                    // subVI program should finish on NI1071 immediately - all it does it set the voltage, then .VI exits
                    Thread.Sleep(1500); // ms polling interval
                    sStatus = NI_Computer.SendTCPcommand("CMD: POLLING ", ref ServerResponse, ref sNI1071FileSpecNotUsed);
                    //UpdateConsoleStatus(ServerResponse);
                    if (sStatus != "SUCCESS")  // did POLLing command fail?
                    {
                        return (sThisFuncStatus + " " + sStatus); // failed
                    }
                    // NI1071 server really should be "DONE", but says RUNNING (not ERROR) when it successfully completed this test
                    if (!ServerResponse.Contains("RUNNING") && !ServerResponse.Contains("DONE") && !ServerResponse.Contains("POLLING"))
                    {
                        return ("ERROR: NI1071 failed to successfully run .VI program; voltage setting not verified ");
                    }
                    // FINALLY, reset the NI1071 to prepare for next test
                    NI_Computer.SendTCPcommand("CMD: RESET ", ref ServerResponse, ref sNI1071FileSpecNotUsed);
                    Thread.Sleep(1500); // Labview sync time - restarting test too soon can cause LV error "not in correct state"


                    // Read the I2C bus on ABB3 to get Voltages
                    int idx;
                    uint status;
                    double dVolts = 0.0;
                    double dErrPercent;
                    string sPassFail;
                    string _sMAX127TestDesc = "MAX127 ADC Read " + iNI1071_Volts.ToString() + "V:" + " BNC AI";

                    // Execute for positive and negative iNI1071_Volts
                    double dVoltsDelta = iNI1071_Volts * dVoltageAccuracy;  // e.g. taget voltage 9 Volts * % acceptable error
                    MAX127chan_ReportColValues[(int)FieldName.Max] = (iNI1071_Volts + dVoltsDelta).ToString("F2");
                    MAX127chan_ReportColValues[(int)FieldName.Min] = (iNI1071_Volts - dVoltsDelta).ToString("F2");

                    for (idx = 6; idx < 14; idx++, _testNum++)
                    {
                        status = ThorDAQAPIGetAI(MasterBoardIndex, idx, true, ref dVolts);
                        if (status != (uint)THORDAQ_STATUS.STATUS_SUCCESSFUL)
                        {
                            return ("ERROR: ThorDAQ I2C failed to get MAX127 voltage ");
                        }
                        MAX127chan_ReportColValues[(int)FieldName.TestNum] = _testNum.ToString();
                        // Test voltage range for pass/fail
                        dErrPercent = Math.Abs(iNI1071_Volts - dVolts) / dVolts;
                        sPassFail = (dErrPercent <= dVoltageAccuracy) ? "PASS" : "FAIL";  // passed?
                        MAX127chan_ReportColValues[(int)FieldName.PassFail] = sPassFail;
                        // Report the Voltages to spreadsheet...
                        MAX127chan_ReportColValues[(int)FieldName.TestDescription] = _sMAX127TestDesc + idx;
                        MAX127chan_ReportColValues[(int)FieldName.Units] = "Volts";
                        MAX127chan_ReportColValues[(int)FieldName.Mean] = dVolts.ToString("F2");


                        sThisFuncStatus = string.Join(" ", MAX127chan_ReportColValues);  // complete test record
                        sConcatenatedStrings.Add(sThisFuncStatus);
                        if (TestReport != null) // workbook exists...
                        {
                            TestReport.WriteWorkSheetRecord("ABB3", _testNum, MAX127chan_ReportColValues); // commit test record to worksheet
                        }
                    }
                }

                return string.Join(Environment.NewLine, sConcatenatedStrings);
            }



            // check EEPROM: from factory has all 0xFF
            public string TestEEPROMserNum()
            {
                string[] EEPROM_ReportColValues = new string[MAXColumnsINREPORT]; // a value record
                string FunctionStatus = "SUCCESS";
                string passFail = "FAIL";
                EEPROM_ReportColValues[(int)FieldName.TestNum] = "1";  // test# to perform
                EEPROM_ReportColValues[(int)FieldName.TestDescription] = _sTestDesc; // "ABBx Serial ...

                // we may test ThorDAQ on a "golden" BBox setup, or BBox packaged with card
                // of course that effects what we test for
                string ABBxSerNumString = "FailedI2Cread";
                bool bStatus = false;
                switch (_BBoxNum)
                {
                    case 1:
                        bStatus = GetI2CeepromSerNum(usBoardIndex, 0x8, 1, 0x50, 0, out ABBxSerNumString);
                        break;
                    case 2:
                        bStatus = GetI2CeepromSerNum(usBoardIndex, 0x8, 2, 0x50, 0, out ABBxSerNumString);
                        break;
                    case 3:
                        bStatus = GetI2CeepromSerNum(usBoardIndex, 0x8, 8, 0x50, 0, out ABBxSerNumString);
                        break;
                }
                if (bStatus)
                {  // success?
                    string sBBoxLabel = "AB" + _BBoxNum.ToString(); // i.e., 1, 2, or 3
                    EEPROM_ReportColValues[(int)FieldName.TestDescription] += ABBxSerNumString;
                    if (ABBxSerNumString.Substring(0, sBBoxLabel.Length) == sBBoxLabel)
                        passFail = "PASS";
                }
                else
                {
                    FunctionStatus = "FAIL";
                    EEPROM_ReportColValues[(int)FieldName.TestDescription] += ABBxSerNumString;
                }
                EEPROM_ReportColValues[(int)FieldName.PassFail] = passFail;
                if (TestReport != null) // workbook exists...
                {
                    TestReport.WriteWorkSheetRecord(_sBBox, 1, EEPROM_ReportColValues); // commit test record to file
                }
                return FunctionStatus;
            }
        }

        // On ThorDAQ TDQ2 and above, test for presence of the 3rd mezz card on back (do not call this test
        // for TDQ1 because card cannot exist)
        // The ThorDAQ SerialNum in EPROM has this format: 
        // There are different hardware versions of the card, and there can be different embedded firmware
        // applications on same card - we identify these by the EEPROM contents, e.g.
        // TD002095-239896-0005-06/27/2019 ---  the "-2" in ...02095-239896... is TDQn number

        public class TRIGtestAndReport
        {
            public string[] TRIG_RepColHdr = new string[MAXColumnsINREPORT];    // test header ROW
            public string[] TRIG_ReportColValues = new string[MAXColumnsINREPORT]; // a value record
            private double _dFreqTolerance = .01;  // locking is critical - 1% accuracy on frequency is sufficient
            public bool TrigCardPossible = false; // cannot exist on TDQ1
            public bool TrigCardExpected = false;  // def. is, we don't see one (e.g. TDQ2 w/ empty slot)
            private ushort usBoardIndex;
            public TRIGtestAndReport(ushort BoardIndex)  // def. CONSTRUCTOR
            {
                usBoardIndex = BoardIndex;
                TRIG_RepColHdr[(int)FieldName.TestNum] = "Test#"; TRIG_RepColHdr[(int)FieldName.PassFail] = "P/F"; TRIG_RepColHdr[(int)FieldName.TestDescription] = "Test Description"; TRIG_RepColHdr[(int)FieldName.Units] = "Units"; TRIG_RepColHdr[(int)FieldName.Mean] = "Avg"; TRIG_RepColHdr[(int)FieldName.StdDev] = "StdDev"; TRIG_RepColHdr[(int)FieldName.Min] = "MinLimit"; TRIG_RepColHdr[(int)FieldName.Max] = "MaxLimit"; TRIG_RepColHdr[(int)FieldName.StdDevLimit] = "StdDevLim";

                if (TestReport != null) // Global ManufTest workbook file report exists? (command option)
                {
                    TestReport.WriteWorkSheetRecord("TRIGcard", 0, TRIG_RepColHdr); // first write headers
                }
                // test for possibility of 3P mezz. card (depends on valid TD EEPROM string)
                string TRIGcardSerNumString;
                if (GetI2CeepromSerNum(usBoardIndex, 0x80, 0xFF, 0x54, 0, out TRIGcardSerNumString))
                {
                    int iTDQtype = Int32.Parse(TRIGcardSerNumString.Substring(2, 1));
                    // if card is TDQ1 it's impossible to have 3P slot
                    if (iTDQtype > 1)
                    {
                        TrigCardPossible = true;
                        if (iTDQtype >= 2)  // TD3 and above only?
                            TrigCardExpected = true;
                    }
                }

            }
            public string[] GetTestHeader()
            {
                return TRIG_RepColHdr;
            }

            // report the version strings from the SAMD21 embedded C application and 
            // the attached CPLD
            public string ReportTRIGcardVersions(int TestNum) // 2 for App, 3 for CPLD
            {
                string[] TrigAppVersions_ReportColValues = new string[MAXColumnsINREPORT]; // a value record
                string FunctionStatus = "ReportTRIGcardVersions";
                string passFail = "PASS";

                TrigAppVersions_ReportColValues[(int)FieldName.TestNum] = TestNum.ToString();  // test# to perform, and appropriate Desc.
                TrigAppVersions_ReportColValues[(int)FieldName.TestDescription] = (TestNum == 2) ? "Card's SAMD21 Application & Version: " : "Card's CPLD Version: ";

                // Use I2C bus to read TRIG card's App type and Version...
                // "PASS" if the I2C command is successful
                List<string> slTRIGcardStatus = APIgetLFT_JAstatus(new List<string> { "" });
                if (slTRIGcardStatus.Contains("OK") != true)
                {
                    FunctionStatus += " ERROR: FAILED to read TRIG card info across I2C bus";
                    passFail = "FAIL";
                }
                else  // parse returned data...
                {
                    if (TestNum == 2) // Application 
                    {
                        string sFW = slTRIGcardStatus[0].Substring(50, 20);
                        TrigAppVersions_ReportColValues[(int)FieldName.TestDescription] += sFW;
                    }
                    else  // CPLD
                    {
                        string sFW = slTRIGcardStatus[0].Substring(33, 8);
                        TrigAppVersions_ReportColValues[(int)FieldName.TestDescription] += sFW;
                    }
                    FunctionStatus = string.Join(" ", TrigAppVersions_ReportColValues);  // complete test record
                }

                if (TestReport != null) // workbook exists...
                {
                    TrigAppVersions_ReportColValues[(int)FieldName.PassFail] = passFail;
                    TestReport.WriteWorkSheetRecord("TRIGcard", TestNum, TrigAppVersions_ReportColValues); // commit test record to file
                }
                return FunctionStatus;
            }
            public string TestEEPROMserNum()
            {
                string[] EEPROM_ReportColValues = new string[MAXColumnsINREPORT]; // a value record
                string FunctionStatus = "SUCCESS";
                string passFail = "FAIL";
                EEPROM_ReportColValues[(int)FieldName.TestNum] = "1";  // test# to perform
                EEPROM_ReportColValues[(int)FieldName.TestDescription] = "EEPROM S/N: ";

                // FIRST, sanity check the ThorDAQ main board SerialNum to see if this Board
                // can even accomodate a back-side card (e.g. 3P)
                string TDSerNumString;
                Int32 iTDQnNum = -1;
                if (GetI2CeepromSerNum(usBoardIndex, 0x80, 0xFF, 0x54, 0, out TDSerNumString)) // I2C map to EEPROM
                {  // success?
                    try
                    {
                        iTDQnNum = Int32.Parse(TDSerNumString.Substring(2, 1));
                    }
                    catch (Exception e)
                    {
                        FunctionStatus = "FAILED to read ThorDAQ TDQn type: " + e.Message;
                    }
                }

                // proceed to test the card...
                string TRIGcardSerNumString;
                if (GetI2CeepromSerNum(usBoardIndex, 0x1, 0xFF, 0x50, 0, out TRIGcardSerNumString))
                {  // success?
                    EEPROM_ReportColValues[(int)FieldName.TestDescription] += TRIGcardSerNumString;
                    if (TRIGcardSerNumString.Substring(0, 2) == "TR")
                        passFail = "PASS";
                }
                else
                {
                    EEPROM_ReportColValues[(int)FieldName.TestDescription] += FunctionStatus;
                }
                EEPROM_ReportColValues[(int)FieldName.PassFail] = passFail;
                if (TestReport != null) // workbook exists...
                {
                    TestReport.WriteWorkSheetRecord("TRIGcard", 1, EEPROM_ReportColValues); // commit test record to file
                }
                return FunctionStatus;
            }
            // Setup the external (e.g. Digilent) function generator for a target frequency pulse (uiFreq)
            // Note that the low frequency (e.g. 3P) mezzanine card's SAMD21 runs an embedded application
            // which can be tested apart from other ThorDAQ board operation - that is, only the internal I2C
            // ThorDAQ bus master is needed to fundamentally test the Card for frequency lock
            public string SetupAndMeasureFreq(UInt32 uiFreq, int iTestIteration) // Iteration is one-based
            {
                string sThisFuncStatus;
                string sStatus;
                List<string> slConcatenatedStrings = new List<string> { };
                int _testNum = (iTestIteration - 2) * 2; // 1 is the EEPROM line, 2 App ver, 3 CPLD ver, 1st iTestIteration is 4
                TRIG_ReportColValues[(int)FieldName.TestNum] = _testNum.ToString();
                sThisFuncStatus = TRIG_ReportColValues[(int)FieldName.TestNum];  // CL-GUI return string starts with same test line num as spreadsheet


                Digilent2 SquareWaveW1 = new Digilent2(); // Labels "W1" and "W2" are channels 0 and 1
                // CONFIGURE the Digilent for the waveform (e.g. pulse or square wave) we want to drive
                // the card's LFn inputs, and start the signal 
                if (!SquareWaveW1.DigilentFound())
                {
                    sThisFuncStatus += " ERROR: Digilent Waveform Generator not found - cannot produce LF1 signal";
                    return sThisFuncStatus;
                }
                sStatus = SquareWaveW1.ConfigureWaveform(MasterBoardIndex, dwf.funcSquare, 1.7, 25.0, (double)uiFreq);
                if (sStatus != "SUCCESS")
                {
                    sThisFuncStatus += " ERROR: " + sStatus;
                    SquareWaveW1.CloseDigilent();
                    return sThisFuncStatus;
                }
                SquareWaveW1.StartWaveform(0, true);  // shutdown Waveform
                Thread.Sleep(12000); // time to start waveform and 3P to lock

                // Now use I2C bus to read TRIG card's frequency/status
                List<string> slTRIGcardStatus = APIgetLFT_JAstatus(new List<string> { "" });
                // if the command succeeded, we'll have an "OK" at the end
                SquareWaveW1.StartWaveform(0, false);  // shutdown Waveform
                if (slTRIGcardStatus.Contains("OK") != true)
                {
                    sThisFuncStatus += " ERROR: FAILED to read card status across I2C bus";
                    return sThisFuncStatus;
                }
                // now find the fields we want...
                bool bLocked = false;
                double dMeasuredFrequency = 0;
                foreach (string sLine in slTRIGcardStatus)
                {
                    if (sLine.Contains("sticky"))
                    {
                        bLocked = (sLine.Contains("0xFF")) ? false : true;  // "PASS" if bLocked is true 
                    }
                    if (sLine.Contains("LF1 Freq"))
                    {
                        string sFreq = sLine.Substring(42, 8);
                        try
                        {
                            dMeasuredFrequency = Double.Parse(sFreq);
                        }
                        catch (Exception e)
                        {
                            sThisFuncStatus += " ERROR: Cannot convert Frequency string: " + sFreq + " to integer: " + e.Message;
                        }
                    }
                }


                TRIG_ReportColValues[(int)FieldName.Mean] = dMeasuredFrequency.ToString();  // CL-GUI return string starts with same test line num as spreadsheet

                // calculate tolerance of expected locked frequency reading...
                double dFreqHigh = Math.Round(uiFreq + (_dFreqTolerance * uiFreq));
                double dFreqLow = Math.Round(uiFreq - (_dFreqTolerance * uiFreq));
                TRIG_ReportColValues[(int)FieldName.Max] = dFreqHigh.ToString();
                TRIG_ReportColValues[(int)FieldName.Min] = dFreqLow.ToString();

                TRIG_ReportColValues[(int)FieldName.Units] = "Hz";
                TRIG_ReportColValues[(int)FieldName.TestDescription] = "SAMD21 CPLD target frequency LF1: " + uiFreq.ToString();
                string sPassFail = "PASS";
                if (dMeasuredFrequency > dFreqHigh) sPassFail = "FAIL";
                if (dMeasuredFrequency < dFreqLow) sPassFail = "FAIL";
                TRIG_ReportColValues[(int)FieldName.PassFail] = sPassFail;
                sThisFuncStatus = string.Join(" ", TRIG_ReportColValues);  // complete test record
                slConcatenatedStrings.Add(sThisFuncStatus);
                if (TestReport != null) // workbook exists...
                {
                    TestReport.WriteWorkSheetRecord("TRIGcard", _testNum, TRIG_ReportColValues); // commit test record to worksheet
                }

                // Now do the "Locked" test line...
                TRIG_ReportColValues[(int)FieldName.Mean] = " ";
                TRIG_ReportColValues[(int)FieldName.Max] = " ";
                TRIG_ReportColValues[(int)FieldName.Min] = " ";
                TRIG_ReportColValues[(int)FieldName.Units] = " ";

                _testNum++; // next test line (row) ...
                TRIG_ReportColValues[(int)FieldName.TestNum] = _testNum.ToString();

                TRIG_ReportColValues[(int)FieldName.Units] = " ";
                TRIG_ReportColValues[(int)FieldName.TestDescription] = "SI5344 Jitter-Attenuator frequency LOCK: chan LF1 ";
                TRIG_ReportColValues[(int)FieldName.PassFail] = bLocked ? "PASS" : "FAIL";
                sThisFuncStatus = string.Join(" ", TRIG_ReportColValues);  // complete test record
                slConcatenatedStrings.Add(sThisFuncStatus);
                if (TestReport != null) // workbook exists...
                {
                    TestReport.WriteWorkSheetRecord("TRIGcard", _testNum, TRIG_ReportColValues); // commit test record to worksheet
                }

                return string.Join(Environment.NewLine, slConcatenatedStrings);
            }
        }

        // Test of ADC mezzanine card REQUIRES the NI-PXIe1071 external computer (running Labview program)
        // All 
        public class ADCwTestAndReport
        {
            private ushort usBoardIndex;
            public string[] ADC_RepColHdr = new string[MAXColumnsINREPORT];    // test header ROW
            public string[] ADC_ReportColValues = new string[MAXColumnsINREPORT]; // a value record
            private int _iLabViewVItestNum = (int)NI1071_LabviewVItestNums.ADCwTest; // known to NI1071 Labview program
            private int _iChanNum = 0; // NI1071 Labview channel number, e.g. "PXISlot/ao1" -- where ChanNum in CMD string is "01"
            private string _sTestDesc = "ADC/PMT 15kHz Sine +/-150mV Waveform RMS Deviation: chan ";
            private double _dWaveRMSErr_Min = 0.0;
            private double _dWaveRMSErr_Max = 0.050;
            private double _dWaveRMSErr_StdDevLimit = 0.005;
            private const int _PixelSamples = 4096;
            private string _WaveInputFileSpec = ManufTest_Data_RootFilepath + @"TDADC_DDR3Chan.txt"; // i.e. add '0' - '3'for Chan A - D
            private AnalyzeNI1071VoltageWaveform analyzeNI1071CapturedWaveform;
            public NIPXIe1071_Controller NI_Computer;  // constructor can fail when Labview not running/answering
            public ADCwTestAndReport(ushort BoardIndex)  // def. CONSTRUCTOR
            {
                usBoardIndex = BoardIndex;
                NI_Computer = new NIPXIe1071_Controller(sIPaddress, IP_port, "NIDAC_AO.txt");   // e.g. DAC_AO.txt, or ADC_AI.txt, which we expand to (e.g.) DAC_AO02 or ADC_AI7.txt
                if (NI_Computer.SrvResp != "SUCCESS")
                    return;
                ADC_RepColHdr[(int)FieldName.TestNum] = "Test#"; ADC_RepColHdr[(int)FieldName.PassFail] = "P/F"; ADC_RepColHdr[(int)FieldName.TestDescription] = "Test Description"; ADC_RepColHdr[(int)FieldName.Units] = "Units"; ADC_RepColHdr[(int)FieldName.Mean] = "Avg"; ADC_RepColHdr[(int)FieldName.StdDev] = "StdDev"; ADC_RepColHdr[(int)FieldName.Min] = "MinLimit"; ADC_RepColHdr[(int)FieldName.Max] = "MaxLimit"; ADC_RepColHdr[(int)FieldName.StdDevLimit] = "StdDevLim";

                if (TestReport != null) // Global ManufTest workbook file report exists? (command option)
                {
                    TestReport.WriteWorkSheetRecord("ADCcard", 0, ADC_RepColHdr); // first write headers
                }
                // Fill in the numerical limits for Waveform RMS deviation
                ADC_ReportColValues[(int)FieldName.Min] = _dWaveRMSErr_Min.ToString("0.000");
                ADC_ReportColValues[(int)FieldName.Max] = _dWaveRMSErr_Max.ToString("0.000");
                ADC_ReportColValues[(int)FieldName.StdDevLimit] = _dWaveRMSErr_StdDevLimit.ToString("0.000");
            }
            public string[] GetTestHeader()
            {
                return ADC_RepColHdr;
            }

            public string TestEEPROMserNum()
            {
                string[] EEPROM_ReportColValues = new string[MAXColumnsINREPORT]; // a value record
                string FunctionStatus = "SUCCESS";
                string passFail = "FAIL";
                EEPROM_ReportColValues[(int)FieldName.TestNum] = "1";  // test# to perform
                EEPROM_ReportColValues[(int)FieldName.TestDescription] = "EEPROM S/N: ";

                string ADCSerNumString;
                if (GetI2CeepromSerNum(usBoardIndex, 0x2, 0xFF, 0x54, 0, out ADCSerNumString))
                {  // success?
                    EEPROM_ReportColValues[(int)FieldName.TestDescription] += ADCSerNumString;
                    if (ADCSerNumString.Substring(0, 2) == "AD")
                        passFail = "PASS";
                }
                else
                {
                    EEPROM_ReportColValues[(int)FieldName.TestDescription] += FunctionStatus;
                }
                EEPROM_ReportColValues[(int)FieldName.PassFail] = passFail;
                if (TestReport != null) // workbook exists...
                {
                    TestReport.WriteWorkSheetRecord("ADCcard", 1, EEPROM_ReportColValues); // commit test record to file
                }
                return FunctionStatus;
            }

            // use ThorDAQAPI to configure the ThorDAQ hardware for optimum setting to capture test
            // sine waves.  We use the linear GalvoGalvo mode with max 4096-pixel line because the "line"
            // is the only real-time sampling opportunity we have to capture the waveform period(s)
            // For instance, we need around 2.5 millisec to capture 3 periods of 1240 Hz wave,
            // which means about ~ 50 ms/pixel, which suggests Dwell of about 10,000 (ticks)
            // We enable all ALL the ADC channels, loading power rails that support the functions,
            // but test only one channel at a time to simplify logic in NI1071 Labview program
            // and maximize DAC speed on NI1071 Analog Out hardware channel (Max output rate, single
            // channel on PXIe-6363 is ~2.86 MSPS)
            private string ConfigThorDAQ()
            {
                string FunctionStatus;
                // affirm the ThorDAQ GlobalScan is STOPped
                List<string> ArgList = new List<string> { "InternalCmd " };  // i.e., just arg0 means no args
                GlobalScan(new List<string> { "InternalCmd", "-s" }, null); // make sure ThorDAQ global scan stopped
                // setup the ThorDAQ hardware...
                // argument list for setup calls
                FunctionStatus = ADCSampleClock_setup(ArgList).Last();
                FunctionStatus = ADCStream_setup(new List<string> { "InternalCmd ", "-j" }).Last(); // MUST reset JESD204B once after system restart

                // NOTE!!  -e option means external hardware trigger needed to start image scan!
                FunctionStatus = ScanControl_setup(new List<string> { "InternalCmd ", "-e", "-m", "1", "-P", "5", "-p", "6", "-n", "1" }).Last();
                FunctionStatus = S2MM_DMA_setup(new List<string> { "InternalCmd", "-d", "4096x4096", "-c", "0xF", "-n", "1" }).Last();   // (Image DMA not required for DAC waveforms...)
                ArgList = new List<string> { "InternalCmd", "-c", "0xFFF", "-z", "1000000", "-f", "c:\\temp\\DACwave_1240Hz_1MHz_250ms_a65500.txt" }; // always 1MHz sample rate, all channels
                FunctionStatus = DACWaveform_setup(ArgList).Last();

                // configure the "fast" External Frame Trigger (requires "-e" option for GlobalScan)
                MTAPISetDIOConfig(new List<string> { "99", "o" }); // make all AUX_GPIO D0-D3, and 24 DIO outputs (including D31)
                // (this leaves D4-D7 as the "default" according to TILS)
                // Add the FPGA MUX output signals for Start of Frame and Start of Line
                MTAPISetDIOConfig(new List<string> { "5", "o", "5", "9" }); // oStart_of_frame = 0x06  oPixel_clock_pulse = 0x09
                MTAPISetDIOConfig(new List<string> { "6", "o", "6", "4" });  // oHorizontal_line_pulse = 0x04
                MTAPISetDIOConfig(new List<string> { "7", "i", "7", "7" }); //  Input from NI-1071 is "iFrame_hardware_trigger"=7

                return FunctionStatus;
            }

            // Test a SINGLE CHANNEL (A - D) of ThorDAQ's PMT ADC
            // EEPROM SerialNum is always test 1
            public string ProgramAndAnalyzeADCchanX(Int32 iChan)
            {
                string ServerResponse = null;
                string sThisFuncStatus;
                string sNI1071OutputFileSpec = null;
                _iChanNum = iChan;         // 0-based index
                int _testNum = iChan + 2;  // testnum 1 is SerNum

                Byte[] DDR3imageBytes = new byte[_PixelSamples * 2];
                string sDDRByteCountToRead = (_PixelSamples * 2).ToString(); // a single line...

                string sDDR3startingAddress = "0x" + ((_PixelSamples * _PixelSamples * 2) * _iChanNum).ToString("X");  // the beginning of the pixel W x H image frame
                                                                                                                       // A typical FAILURE mode is the DMA image transfer fails; in that case, if there is "left over"
                                                                                                                       // data sitting in DDR3, a failed test may "pass" because it's reading obsolete dat
                                                                                                                       // Consequently, clear the DDR3 image area before starting
                string sStatus = WriteDDR3(new List<String> { "InternalCmd", sDDR3startingAddress, sDDRByteCountToRead, "0x0" }).Last();


                string sChan;
                switch (iChan)
                {
                    case 0:
                        sChan = "A";
                        break;
                    case 1:
                        sChan = "B";
                        break;
                    case 2:
                        sChan = "C";
                        break;
                    case 3:
                        sChan = "D";
                        break;
                    default:
                        sChan = "x";
                        break;
                }

                // make sure ThorDAQ GlobalScan stopped and re-configured
                // prepare result/Spreadsheet report record
                ADC_ReportColValues[(int)FieldName.TestNum] = (_testNum).ToString();  // test# we are doing...
                ADC_ReportColValues[(int)FieldName.PassFail] = "FAIL"; // DEFAULT is fail

                sThisFuncStatus = ADC_ReportColValues[(int)FieldName.TestNum] + " chan " + sChan + ": ";
                sStatus = ConfigThorDAQ();
                if (sStatus != "SUCCESS")
                {
                    return (sThisFuncStatus + " " + sStatus); // failed
                }
                // ThorDAQ hardware is programmed and ready...
                // make sure NI 1071 is reset
                sStatus = NI_Computer.SendTCPcommand("CMD: RESET ", ref ServerResponse, ref sNI1071OutputFileSpec);
                if (sStatus != "SUCCESS")
                {
                    return (sThisFuncStatus + " " + sStatus); // failed
                }
                // Setup the NI 1071 computer, then "GlobalStart" START ThorDAQ to trigger start of test



                // transfer of command test data fields to NI1071 DLL, then to params in Labview...
                //            1         2         3
                //  0123456789012345678901234567890
                //  CMD: TEST n cc TestDescription
                // were (base10) 'n' is single test integer and 'cc' is two digit NI channel number (e.g. 0-31)
                MTAPISetDO(new List<string> { "D31", "0" });  // make sure NI1071 trigger low
                string NIcommand = ("CMD: TEST " + _iLabViewVItestNum.ToString() + " " + _iChanNum.ToString("D2") + " " + _sTestDesc);
                sStatus = NI_Computer.SendTCPcommand(NIcommand, ref ServerResponse, ref sNI1071OutputFileSpec);
                if (sStatus == "SUCCESS")
                {
                    // Give LabView program time to set the "Global SubVI Test Programs"
                    // (without this delay, the Labview program will return the result of LAST Test start, immediately)
                    const int LabViewTestStateDelayMS = 1750; // (set by trial and error observations - we cannot dictate speed of Labview/WinOS invocations)
                    Thread.Sleep(LabViewTestStateDelayMS);

                    // Use external HARDWARE TRIGGER from NI-1071 that should come into BOB's BNC D7
                    // -T option asserts D31 immediately after setting GlobalRun bit 
                    sStatus = GlobalScan(new List<string> { "InternalCmd", "-n", "4" }, null).Last();  // ARM, with line count # (smallest is 4)

                    // Toggle the NI1071 start bit
                    MTAPISetDO(new List<string> { "D31", "1" });
                    MTAPISetDO(new List<string> { "D31", "0" });
                }
                else
                {
                    return (sThisFuncStatus + " " + sStatus); // failed
                }
                bool bLabviewTestCompleted = false;
                // POLL for completion (or timeout) 


                const int SecondsToPoll = 8;  // should be longer than timeout on the Labview (sub) VI test
                for (int t = 0; t < SecondsToPoll * 4; t++)
                {
                    Thread.Sleep(250); // ms polling interval
                    sStatus = NI_Computer.SendTCPcommand("CMD: POLLING ", ref ServerResponse, ref sNI1071OutputFileSpec);
                    //UpdateConsoleStatus(ServerResponse);
                    if (sStatus != "SUCCESS")  // did POLLing command fail?
                    {
                        return (sThisFuncStatus + " " + sStatus); // failed
                    }
                    // does server response have DONE, ERROR, or RESET, indicating Test has concluded?
                    if (ServerResponse.Contains("DONE") || ServerResponse.Contains("ERROR") || ServerResponse.Contains("RESET"))
                    {
                        bLabviewTestCompleted = true;
                        break;
                    }
                }
                // FINISHED POLLING... "GlobalStart" STOP ThorDAQ 

                sStatus = GlobalScan(new List<string> { "InternalCmd ", "-s" }, null).Last();  // STOP!

                string PassFail = "PASS"; // spreadsheet variable
                sStatus = "SUCCESS"; // update on FAIL conditions
                if (bLabviewTestCompleted == false)
                {
                    // we have timed out while POLLING...
                    sThisFuncStatus += " FAIL: TIMEOUT ERROR on POLLING! ";
                }
                else  // test completed - DONE (success) or ERROR or SubVI RESET by user?
                {
                    if (ServerResponse.Contains("DONE") == true)
                    {
                        // NI1071 has played back SINE WAVE ...
                        // Fetch a copy of the "volts" waveform NI1071 used to generate the Waveform
                        Thread.Sleep(2000);
                        // Send command to NI1071 to SEND us the data file from test...
                        sStatus = NI_Computer.SendTCPcommand("CMD: RECEIVEFILE ", ref ServerResponse, ref sNI1071OutputFileSpec);


                        // NOW READ the DDR3 memory to get the ADC copy of the wave period(s)...

                        sStatus = ReadDDR3(new List<String> { "InternalCmd", sDDR3startingAddress, sDDRByteCountToRead, "-q" }, ref DDR3imageBytes).Last();
                        if (sStatus == "SUCCESS")
                        {
                            // the DDR3 image bytes should be in DDR3imageBytes array...
                            ushort[] UShortValues = new ushort[_PixelSamples];
                            for (int i = 0, j = 0; i < _PixelSamples * 2; i += 2, j++)
                            {
                                UShortValues[j] = BitConverter.ToUInt16(DDR3imageBytes, i);
                            }
                            // transform these ThorDAQ "raw" ThorDAQ 14-bit ADC counts to "voltages" consistent with ThorImageLS...??
                            string[] FileSpec = _WaveInputFileSpec.Split('.');
                            FileSpec[0] += iChan.ToString();
                            string sMeasureFileSpec = FileSpec[0] + "." + FileSpec[1];
                            using (System.IO.StreamWriter file =
                            new System.IO.StreamWriter(sMeasureFileSpec))
                            {
                                for (int i = 0; i < UShortValues.Length; i++)
                                {
                                    file.WriteLine(UShortValues[i].ToString());
                                }
                            }
                            // Compare ADC chan. Received voltage to NI-1071 DAC Wavform voltage... 
                            // ThorDAQ's PMT ADC was recorded to DDR3 memory...
                            analyzeNI1071CapturedWaveform = new AnalyzeNI1071VoltageWaveform(false, sNI1071OutputFileSpec, sMeasureFileSpec);
                            string sAnalysisMsg = analyzeNI1071CapturedWaveform.ConvertFilesToSingleLineVoltageRecords();
                            if (sAnalysisMsg == "SUCCESS")
                            {
                                // analyze wave and try to compute Root Mean Square Error...
                                int EndingIdealSample = 0;  // function returns sample that marks beginning of next period in file
                                int EndingMeasuredSample = 0;
                                double dRMSerror = 0.0;
                                double[] RMSdevIteration = new double[3];
                                for (int iteration = 0; iteration < 3; iteration++) // attempt three iterations on 3 periods in Measured sample...
                                {
                                    sAnalysisMsg = analyzeNI1071CapturedWaveform.RMSdeviation(1, ref EndingIdealSample, ref EndingMeasuredSample, ref dRMSerror);
                                    if (sAnalysisMsg != "SUCCESS")
                                    {
                                        sThisFuncStatus += " FAIL Can't compute RMS Deviation: " + sAnalysisMsg;
                                        break;
                                    }
                                    RMSdevIteration[iteration] = dRMSerror;
                                }
                                ADC_ReportColValues[(int)FieldName.TestDescription] = _sTestDesc + sChan;

                                if (sAnalysisMsg == "SUCCESS")
                                {
                                    ADC_ReportColValues[(int)FieldName.Units] = "Volts";
                                    // Calculate simple stats
                                    double dMean = 0.0;
                                    double dStdDev;
                                    List<double> RMSdevList = new List<double> { RMSdevIteration[0], RMSdevIteration[1], RMSdevIteration[2] };
                                    StdDevAndMean stdDevAndMean = new StdDevAndMean();
                                    dStdDev = stdDevAndMean.ComputeStdDevAndMean(RMSdevList, ref dMean); // return StdDev and ref to Mean)
                                    // store for Console output (& Spreadsheet report)
                                    ADC_ReportColValues[(int)FieldName.Mean] = dMean.ToString("0.00000");
                                    ADC_ReportColValues[(int)FieldName.StdDev] = dStdDev.ToString("0.00000");

                                    // Done - now determine PASS/FAIL status from metrics...
                                    if (dStdDev > _dWaveRMSErr_StdDevLimit) PassFail = "FAIL";
                                    if (dMean > _dWaveRMSErr_Max) PassFail = "FAIL";
                                    if (dStdDev < _dWaveRMSErr_Min) PassFail = "FAIL";

                                    ADC_ReportColValues[(int)FieldName.PassFail] = PassFail;

                                    sThisFuncStatus = string.Join(" ", ADC_ReportColValues);  // complete test record
                                }
                            }
                            else
                            {
                                sThisFuncStatus += " FAIL: ConvertADCcntsToVoltage() error: " + sAnalysisMsg;
                            }
                        }
                    }
                    else if (ServerResponse.Contains("ERROR") == true)
                    {
                        sThisFuncStatus += " FAIL: NI1071 Test VI program ERROR ";
                    }
                    else if (ServerResponse.Contains("RESET") == true)
                    {
                        sThisFuncStatus += " FAIL: NI1071 Test VI program RESET ";
                    }
                }
                if (TestReport != null) // workbook exists...
                {
                    TestReport.WriteWorkSheetRecord("ADCcard", _testNum, ADC_ReportColValues); // commit test record to worksheet
                }

                // FINALLY, reset the NI1071 to prepare for next test
                NI_Computer.SendTCPcommand("CMD: RESET ", ref ServerResponse, ref sNI1071OutputFileSpec);

                //if (PassFail != "SUCCESS") Debug.Assert(false);

                return sThisFuncStatus;  // Success/Data or the last failure
            }


        };
        // Test of DAC mezzanine card REQUIRES the NI-PXIe1071 external computer (running Labview program)
        // All 
        public class DACwTestAndReport  // "w"ave test
        {
            private ushort usBoardIndex;
            public string[] DAC_RepColHdr = new string[MAXColumnsINREPORT];    // test header ROW
            public string[] DAC_ReportColValues = new string[MAXColumnsINREPORT]; // a value record
            private int _iLabViewVItestNum = (int)NI1071_LabviewVItestNums.DACwTest; // known to NI1071 Labview program
            private int _iChanNum = 0; // NI1071 Labview channel number, e.g. "PXISlot/ai1" -- where ChanNum string is "01"
            private string _sTestDesc = "DAC-ABBx Waveform RMS Deviation: connector AO";
            private string _WaveInputFileSpec = @"c:\temp\DACwave_1240Hz_1MHz_250ms_a65500.txt";
            // now many periods (starting with 0 VDC neg. to pos. transition) should we analyze?
            const int _WAVE_PERIODS_IN_MEASURED_FILE_TO_ANALYZE = 4;
            private double _dRMSerror;
            private List<Enum> _LEDenumList;
            private double _dWaveRMSErr_Min = 0.0;
            private double _dWaveRMSErr_Max = 0.250;
            private double _dWaveRMSErr_StdDevLimit = 0.10;

            public NIPXIe1071_Controller NI_Computer;
            private AnalyzeNI1071VoltageWaveform analyzeNI1071CapturedWaveform;
            //private RMScomputations RMScomputing;
            public DACwTestAndReport(ushort BoardIndex)  // def. CONSTRUCTOR
            {
                usBoardIndex = BoardIndex;
                NI_Computer = new NIPXIe1071_Controller(sIPaddress, IP_port, "NIADC_AI.txt");   // e.g. DAC_AO.txt, or ADC_AI.txt, which we expand to DAC_AO07 or ADC_AI3.txt

                _LEDenumList = new List<Enum> { };
                _dRMSerror = -1; // def. invalid
                DAC_RepColHdr[(int)FieldName.TestNum] = "Test#"; DAC_RepColHdr[(int)FieldName.PassFail] = "P/F"; DAC_RepColHdr[(int)FieldName.TestDescription] = "Test Description"; DAC_RepColHdr[(int)FieldName.Units] = "Units"; DAC_RepColHdr[(int)FieldName.Mean] = "Avg"; DAC_RepColHdr[(int)FieldName.StdDev] = "StdDev"; DAC_RepColHdr[(int)FieldName.Min] = "MinLimit"; DAC_RepColHdr[(int)FieldName.Max] = "MaxLimit"; DAC_RepColHdr[(int)FieldName.StdDevLimit] = "StdDevLim";

                if (TestReport != null) // Global ManufTest workbook file report exists? (command option)
                {
                    TestReport.WriteWorkSheetRecord("DACcard", 0, DAC_RepColHdr); // first write headers
                }
                // Fill in the numerical limits for Waveform RMS deviation
                DAC_ReportColValues[(int)FieldName.Min] = _dWaveRMSErr_Min.ToString("0.000");
                DAC_ReportColValues[(int)FieldName.Max] = _dWaveRMSErr_Max.ToString("0.000");
                DAC_ReportColValues[(int)FieldName.StdDevLimit] = _dWaveRMSErr_StdDevLimit.ToString("0.000");
            }
            public string[] GetTestHeader()
            {
                return DAC_RepColHdr;
            }

            // Test#1 is EEPROM serial num for this component
            public string TestEEPROMserNum()
            {
                string[] EEPROM_ReportColValues = new string[MAXColumnsINREPORT]; // a value record
                string FunctionStatus = "SUCCESS";
                string passFail = "FAIL";
                EEPROM_ReportColValues[(int)FieldName.TestNum] = "1";  // test# to perform
                EEPROM_ReportColValues[(int)FieldName.TestDescription] = "EEPROM S/N: ";

                string DACSerNumString;
                if (GetI2CeepromSerNum(usBoardIndex, 0x8, 4, 0x54, 0, out DACSerNumString))
                {  // success?
                    EEPROM_ReportColValues[(int)FieldName.TestDescription] += DACSerNumString;
                    if (DACSerNumString.Substring(0, 2) == "DA")
                        passFail = "PASS";
                }
                else
                {
                    EEPROM_ReportColValues[(int)FieldName.TestDescription] += FunctionStatus;
                }
                EEPROM_ReportColValues[(int)FieldName.PassFail] = passFail;
                if (TestReport != null) // workbook exists...
                {
                    TestReport.WriteWorkSheetRecord("DACcard", 1, EEPROM_ReportColValues); // commit test record to file
                }
                return FunctionStatus;
            }
            // Read the DAC CPLD - legacy CPLD had binary "USERCODE", e.g. 1 or 3
            // New GIT built version has 4-digit ASCII USERCODE like 1015
            public string GetDACCPLDver()
            {
                string[] DACCPLD_ReportColValues = new string[MAXColumnsINREPORT]; // a value record
                string SNstring = "No READ value";
                int _testNum = 2;
                DACCPLD_ReportColValues[(int)FieldName.TestNum] = (_testNum).ToString();  // test# to perform
                DACCPLD_ReportColValues[(int)FieldName.TestDescription] = "CPLD Version: ";

                bool bStatus = GetI2CeepromSerNum(usBoardIndex, 0x8, 0xff, 0x40, 0xC0, out SNstring);   // read USERCODE from CPLD
                bStatus = true;
                if (bStatus == true)
                {
                    DACCPLD_ReportColValues[(int)FieldName.PassFail] = "PASS";
                    DACCPLD_ReportColValues[(int)FieldName.TestDescription] += string.Format("{0}", SNstring);
                }
                else
                    DACCPLD_ReportColValues[(int)FieldName.PassFail] = "FAIL";

                if (TestReport != null) // workbook exists...
                {
                    TestReport.WriteWorkSheetRecord("DACcard", _testNum, DACCPLD_ReportColValues); // commit test record to file
                }

                return DACCPLD_ReportColValues[(int)FieldName.TestNum] + " " +
                        DACCPLD_ReportColValues[(int)FieldName.PassFail] + " " +
                        DACCPLD_ReportColValues[(int)FieldName.TestDescription];

            }



            // use ThorDAQAPI to configure the ThorDAQ hardware to output all 12 channels
            // we are hardcoding all options for Galvo-Galvo to settings that optimize our ability to test
            // we turn all ALL the DAC channels, loading power rails that support the functions,
            // but test only one channel at a time to simplify logic in TCP session to the NI1071 Labview program
            private string ConfigThorDAQ()
            {
                string FunctionStatus;
                // affirm the ThorDAQ GlobalScan is STOPped
                GlobalScan(new List<string> { "InternalCmd", "-s" }, _LEDenumList); // make sure ThorDAQ global scan stopped
                // setup the ThorDAQ hardware...
                // argument list for setup calls
                List<string> ArgList = new List<string> { "InternalCmd " };  // i.e., just arg0 means no args
                FunctionStatus = ADCSampleClock_setup(ArgList).Last();
                FunctionStatus = ADCStream_setup(new List<string> { "InternalCmd ", "-j" }).Last();

                // -e is for "external hardware trigger" option - we use BOB's D31 output for this
                FunctionStatus = ScanControl_setup(new List<string> { "InternalCmd ", "-e" }).Last();
                // setup the Digital IO 
                MTAPISetDIOConfig(new List<string> { "99", "i" }); // Inputs generally avoid I/O contention with NI
                MTAPISetDIOConfig(new List<string> { "7", "i", "7", "0x20" }); // Input from NI-1071 feeds  DI_HW_Trigger_1	("iFrame_hardware_trigger"=7 deprecated)
                MTAPISetDIOConfig(new List<string> { "31", "o", "31" });    // D31 triggers NI
                //ArgList = new List<string> { "InternalCmd", "-d 4096x4096" };
                //FunctionStatus = S2MM_DMA_setup(new List<string> { "InternalCmd", "-d 4096x4096" }).Last();   // (Image DMA not required for DAC waveforms...)
                // always 1MHz sample rate, all channels
                FunctionStatus = DACWaveform_setup(new List<string> { "InternalCmd", "-c", "0xFFF", "-z", "1000000", "-f", _WaveInputFileSpec }).Last();

                return FunctionStatus;
            }

            // The big design advantage of single channel test is NI1071 hardware has maximum sampling speed
            // with one or two channels analyzed.  More simulaneous channels means slower sampling speed
            // ProgramAndAnalyzeABBxChannels is the process to:
            // A. Configure ThorDAQ
            // B. setup NI1071 to do the test (on requested channel)

            // and POLL for the end result...
            // DACwTestAndReport.ProgramAndAnalyzeABBxChannels
            public string ProgramAndAnalyzeABBxChannel(Int32 iChan)
            {
                string ServerResponse = null;
                string sThisFuncStatus;
                string sNI1071CapturedFileSpec = null;
                _iChanNum = iChan; // 0-based
                int _testNum = iChan + 3;  // testnum 1 is SerNum, 2 is DAC CPLD ver
                // identify the LED enum for this Channel -- e.g. AO1 - AO12
                _LEDenumList.Clear();
                string LEDlabel = "AO" + (iChan).ToString();
                BBoxLEDenum LEDenum = (BBoxLEDenum)Enum.Parse(typeof(BBoxLEDenum), LEDlabel);
                object oLEDenum = LEDenum;
                _LEDenumList.Add((Enum)oLEDenum);

                // (EEPROM SerNum always test #1)
                DAC_ReportColValues[(int)FieldName.TestNum] = (_testNum).ToString();  // test# to perform
                DAC_ReportColValues[(int)FieldName.PassFail] = "FAIL"; // DEFAULT is fail

                // make sure ThorDAQ GlobalScan stopped
                string sStatus = ConfigThorDAQ(); // includes required DIO setup

                MTAPISetDO(new List<string> { "D31", "0" });  // make sure NI1071 trigger low

                if (sStatus != "SUCCESS")
                {
                    return sStatus; // failed
                }
                // ThorDAQ hardware is programmed and ready...
                // make sure NI 1071 is reset
                sThisFuncStatus = NI_Computer.SendTCPcommand("CMD: RESET ", ref ServerResponse, ref sNI1071CapturedFileSpec);
                if (sThisFuncStatus != "SUCCESS")
                {
                    return sThisFuncStatus; // failed
                }
                // Setup the NI 1071 computer, then "GlobalStart" START ThorDAQ to trigger start of test

                // transfer of command test data fields to NI1071 DLL, then to params in Labview...
                //            1         2         3
                //  0123456789012345678901234567890
                //  CMD: TEST n cc TestDescription
                // were (base10) 'n' is single test integer and 'cc' is two digit NI channel number (e.g. 0-31)
                string NIcommand = ("CMD: TEST " + _iLabViewVItestNum.ToString() + " " + (_iChanNum).ToString("D2") + " " + _sTestDesc);
                sThisFuncStatus = NI_Computer.SendTCPcommand(NIcommand, ref ServerResponse, ref sNI1071CapturedFileSpec);
                if (sThisFuncStatus == "SUCCESS")
                {
                    // Give LabView program time to set the "Global SubVI Test Programs"
                    // (without this delay, the Labview program will return the result of LAST Test start, immediately)
                    const int LabViewTestStateDelayMS = 1250; // (set by trial and error observations - we cannot dictate speed of Labview/WinOS invocations)
                    Thread.Sleep(LabViewTestStateDelayMS);
                    // NI instrumnent waiting for SOF trigger -- time to start
                    List<string> DefArgList = new List<string> { "InternalCmd", "-d", "4096x4096" }; // define here because there's no S2MM image DMA
                    // now set D31 and trigger NI1071...
                    sStatus = GlobalScan(DefArgList, _LEDenumList).Last();  // ARM, so we're waiting for "external trigger"
                    // toggle to trigger start AND capture of DAC output wave
                    MTAPISetDO(new List<string> { "D31", "1" });  // make sure NI1071 trigger low
                    MTAPISetDO(new List<string> { "D31", "0" });  // make sure NI1071 trigger low
                }
                else
                {
                    return sThisFuncStatus; // failed
                }
                bool bLabviewTestCompleted = false;
                // POLL for completion (or timeout) 
                const int SecondsToPoll = 8;  // should be longer than timeout on the Labview (sub) VI test
                for (int t = 0; t < SecondsToPoll * 4; t++)
                {
                    Thread.Sleep(250); // ms polling interval
                    sThisFuncStatus = NI_Computer.SendTCPcommand("CMD: POLLING ", ref ServerResponse, ref sNI1071CapturedFileSpec);
                    //UpdateConsoleStatus(ServerResponse);
                    if (sThisFuncStatus != "SUCCESS")  // did POLLing command fail?
                    {
                        //UpdateConsoleStatus("...FAILED");
                        return sThisFuncStatus; // fail, quit now
                    }
                    // does server response have DONE, ERROR, or RESET, indicating Test has concluded?
                    if (ServerResponse.Contains("DONE") || ServerResponse.Contains("ERROR") || ServerResponse.Contains("RESET"))
                    {
                        bLabviewTestCompleted = true;
                        break;
                    }
                }
                // FINISHED POLLING... "GlobalStart" STOP ThorDAQ 
                List<string> ArgList = new List<string> { "InternalCmd ", "-s" };
                sStatus = GlobalScan(ArgList, _LEDenumList).Last();  // STOP!

                sStatus = "SUCCESS"; // update on FAIL conditions
                if (bLabviewTestCompleted == false)
                {
                    // we have timed out while POLLING...
                    sThisFuncStatus = "FAIL: TIMEOUT ERROR on POLLING! ";
                }
                else  // test completed - DONE (success) or ERROR or SubVI RESET by user?
                {
                    if (ServerResponse.Contains("DONE") == true)
                    {
                        // give NI1071 Win10 computer delay to flush data file for reading
                        Thread.Sleep(2000);
                        // Send command to NI1071 to SEND us the data file from test...
                        sNI1071CapturedFileSpec = null;  // make the function create our filename
                        sStatus = NI_Computer.SendTCPcommand("CMD: SENDFILE ", ref ServerResponse, ref sNI1071CapturedFileSpec);

                        if (sStatus.Substring(0, 7) != "SUCCESS")
                        {
                            sThisFuncStatus = "FAIL: " + sStatus;
                        }
                        else  // analyze the returned file
                        {
                            // Compare Received voltage to DAC Wavform voltage... 
                            analyzeNI1071CapturedWaveform = new AnalyzeNI1071VoltageWaveform(true, _WaveInputFileSpec, sNI1071CapturedFileSpec);
                            string sAnalysisMsg = analyzeNI1071CapturedWaveform.ConvertFilesToSingleLineVoltageRecords();
                            if (sAnalysisMsg == "SUCCESS")
                            {
                                // analyze wave and try to compute Root Mean Square Error...
                                int EndingIdealSample = 0;  // function returns sample that marks beginning of next period in file
                                int EndingMeasuredSample = 0;
                                double[] RMSdevIteration = new double[_WAVE_PERIODS_IN_MEASURED_FILE_TO_ANALYZE];
                                for (int iteration = 0; iteration < _WAVE_PERIODS_IN_MEASURED_FILE_TO_ANALYZE; iteration++) // expect multiple periods in Measured sample...
                                {
                                    sAnalysisMsg = analyzeNI1071CapturedWaveform.RMSdeviation(2, ref EndingIdealSample, ref EndingMeasuredSample, ref _dRMSerror);
                                    if (sAnalysisMsg != "SUCCESS")
                                    {
                                        sThisFuncStatus += "FAIL: Can't compute RMS Deviation: " + sAnalysisMsg;
                                        break;
                                    }
                                    RMSdevIteration[iteration] = _dRMSerror;
                                }
                                // prepare result/Spreadsheet report record
                                DAC_ReportColValues[(int)FieldName.TestDescription] = _sTestDesc + (iChan).ToString();
                                DAC_ReportColValues[(int)FieldName.Units] = "Volts";
                                // Calculate simple stats
                                double dMean = 0.0;
                                double dStdDev;
                                List<double> RMSdevList = new List<double> { RMSdevIteration[0], RMSdevIteration[1], RMSdevIteration[2] };
                                StdDevAndMean stdDevAndMean = new StdDevAndMean();
                                dStdDev = stdDevAndMean.ComputeStdDevAndMean(RMSdevList, ref dMean); // return StdDev and ref to Mean)
                                                                                                     // store for Console output (& Spreadsheet report)
                                DAC_ReportColValues[(int)FieldName.Mean] = dMean.ToString("0.00000");
                                DAC_ReportColValues[(int)FieldName.StdDev] = dStdDev.ToString("0.00000");

                                if (sAnalysisMsg == "SUCCESS")
                                {

                                    // Done - now determine PASS/FAIL status from metrics...
                                    string PassFail = "PASS";
                                    if (dStdDev > _dWaveRMSErr_StdDevLimit) PassFail = "FAIL";
                                    if (dMean > _dWaveRMSErr_Max) PassFail = "FAIL";
                                    if (dStdDev < _dWaveRMSErr_Min) PassFail = "FAIL";

                                    DAC_ReportColValues[(int)FieldName.PassFail] = PassFail;
                                }
                                else // failed
                                {
                                    DAC_ReportColValues[(int)FieldName.TestDescription] += sAnalysisMsg;
                                }
                                sThisFuncStatus = string.Join(" ", DAC_ReportColValues);
                            }
                            else
                            {
                                sThisFuncStatus = " ConvertDACcntsToVoltage() error: " + sAnalysisMsg;
                            }
                        }
                    }
                    else if (ServerResponse.Contains("ERROR") == true)
                    {
                        sThisFuncStatus = "FAIL: NI1071 Test VI program ERROR ";
                    }
                    else if (ServerResponse.Contains("RESET") == true)
                    {
                        sThisFuncStatus = "FAIL: NI1071 Test VI program RESET ";
                    }
                }
                // at last, update spreadsheet (if called for)
                if (TestReport != null) // workbook exists...
                {
                    TestReport.WriteWorkSheetRecord("DACcard", _testNum, DAC_ReportColValues); // ABS value line# in spreadsheet
                }

                // FINALLY, reset the NI1071 to prepare for next test
                NI_Computer.SendTCPcommand("CMD: RESET ", ref ServerResponse, ref sNI1071CapturedFileSpec);

                return sThisFuncStatus;  // Success/Data or the last failure
            }
        }

        // Function to return the ASCII string of I2C EEPROM in ThorDAQ system
        // typically returns 0xFF ASCII values when EEPROM was never programmedd
        // returns FALSE on communication error, most likely when pluggable component is not present
        // TSI defined only first 28 bytes, TIS is 64 bytes. TIS only uses first 3 chars of
        // first 28 bytes, then following that 10 bytes productNum and 6 bytes serialNum
        // In Legacy case, only first 28 bytes are defined  (shown without dashes)
        // TD3?????????????????????????7ITN002586002019???????????????????? (TIS case, 7ITN002586 partNum)
        // TD302019-209044-3240-01/28/2018????????????????????????????????? (Legacy case)
        // TIS programming definition for "Production" purposes (e.g. in TD_ProdSN.txt)
        // TD1-xxxxxxxxxx-nnnnnn       3 chars for component - 10 chars part - 6 chars SN  (no date)
        static public bool GetI2CeepromSerNum(uint BoardIndex, uint MasterChan, int SlaveChan, uint DeviceAddr, uint SlaveCommand, out string SNstring)
        {
            bool bStatus = true;
            // allocate the unmanaged C# buffer
            Encoding enc8 = Encoding.UTF8;
            uint MaxI2CdataLEN; // EEPROM DATA size to read (max possible)
            uint OpCodeLen;   // i.e., we typically send a single byte "0" with starting indx for READ
                              //            UInt32 I2CbusHz = 400; // use higher speed
            UInt32 I2CbusHz = 100; // use slowest speed
            IntPtr unManagedI2CData = Marshal.AllocHGlobal(64);
            IntPtr UnManagedI2CopCodeBuffer = Marshal.AllocHGlobal(8); // largest possible size

            SNstring = "_I2CfailedRead";

            Byte[] I2COpCodeBuf = new Byte[8] { 0, 0, 0, 0, 0, 0, 0, 0 }; // max for AIX I2C master
            Byte[] I2CbyteBuf = new Byte[64]; // max for AIX I2C master
            Byte[] ASCIIbytes = new Byte[64];

            if (SlaveCommand == 0xC0) // CPLD ver request?
            {
                I2COpCodeBuf[0] = (Byte)SlaveCommand;
                OpCodeLen = 4;
                MaxI2CdataLEN = 4;  // USERCODE returns 4 bytes
            }
            else
            {
                OpCodeLen = 1; // all EEPROMS need single start index (pick 0)
                MaxI2CdataLEN = TOT_SERIAL_SZ;  // EEPROMs return this many bytes
            }
            Marshal.Copy(I2COpCodeBuf, 0, UnManagedI2CopCodeBuffer, (int)OpCodeLen);

            uint I2CbytesXmitted = MaxI2CdataLEN; // send BufferLen -- returns with TOTAL I2C bytes
            bool bI2C_SlaveRead = true; // READing from I2C slave
            const int FlashPageSize = 16;
            uint status = ThorDAQAPIXI2CReadWrite(BoardIndex, bI2C_SlaveRead,
                TDMasterI2CMUXAddr, MasterChan,
                TDSlaveI2CMUXAddr, (uint)SlaveChan, DeviceAddr, I2CbusHz, FlashPageSize,
                UnManagedI2CopCodeBuffer, ref OpCodeLen, // "OpCodeLen" IN: Opcode byte count (in buffer)
                unManagedI2CData, ref I2CbytesXmitted); // "DataLen" IN: dataLen,  OUT: ALL Opcdoes + Data bytes transfered
            if (status == Win32.STATUS_SUCCESSFUL)
            {
                for (int i = 0; i < MaxI2CdataLEN; i++) // Len can be equal to or LESS than original BufferLen
                {
                    I2CbyteBuf[i] = Marshal.ReadByte(unManagedI2CData, i);
                }
                if (SlaveCommand == 0xC0) // CPLD ver request?
                {
                    // we must handle special case of CPLD which returns alphanumeric version chars, not a string
                    SNstring = string.Format("{0}.{1}.{2}.{3}", Convert.ToChar(I2CbyteBuf[0]), Convert.ToChar(I2CbyteBuf[1]),
                                             Convert.ToChar(I2CbyteBuf[2]), Convert.ToChar(I2CbyteBuf[3]));
                }
                else
                {
                    for (int i = 0; i < MaxI2CdataLEN; i++) // Len can be equal to or LESS than original BufferLen
                    {
                        if (I2CbyteBuf[i] >= 0x20 && I2CbyteBuf[i] <= 127) // printable?
                        {
                            ASCIIbytes[i] = I2CbyteBuf[i];
                        }
                    }
                    SNstring = enc8.GetString(ASCIIbytes);
                }
            }
            else
            {
                bStatus = false;
            }
            if (UnManagedI2CopCodeBuffer != IntPtr.Zero)
                Marshal.FreeHGlobal(UnManagedI2CopCodeBuffer);
            if (unManagedI2CData != IntPtr.Zero)
                Marshal.FreeHGlobal(unManagedI2CData);
            return bStatus;
        }


        // read ThorDAQ system serial numbers (all components with EEPROM)
        static public bool ReadAllTDSerialNums(uint TDboardIndex, out string Concatentated_SerialNums)
        {
            bool bStatus = true;
            string TDSerNumString;
            string ADCSerNumString;
            string DACSerNumString;
            string TRG3PSerNumString;
            string DBB1SerNumString;
            string ABB1SerNumString;
            string ABB2SerNumString;
            string ABB3SerNumString;
            // Hardware defined I2C map - see SWUG
            GetI2CeepromSerNum(TDboardIndex, 0x80, 0xFF, 0x54, 0, out TDSerNumString);
            GetI2CeepromSerNum(TDboardIndex, 0x2, 0xFF, 0x54, 0, out ADCSerNumString);
            GetI2CeepromSerNum(TDboardIndex, 0x2, 0xFF, 0x50, 0, out DBB1SerNumString);
            GetI2CeepromSerNum(TDboardIndex, 0x8, 4, 0x54, 0, out DACSerNumString);
            GetI2CeepromSerNum(TDboardIndex, 0x8, 1, 0x50, 0, out ABB1SerNumString);
            GetI2CeepromSerNum(TDboardIndex, 0x8, 2, 0x50, 0, out ABB2SerNumString);
            GetI2CeepromSerNum(TDboardIndex, 0x8, 8, 0x50, 0, out ABB3SerNumString);
            GetI2CeepromSerNum(TDboardIndex, 0x1, 0xFF, 0x50, 0, out TRG3PSerNumString);


            //uint uiStatus = ThorDAQAPIReadI2CDevice(MasterBoardIndex, )
            Concatentated_SerialNums = String.Join("$", TDSerNumString, ADCSerNumString, DBB1SerNumString,
                                                        DACSerNumString, ABB1SerNumString, ABB2SerNumString, ABB3SerNumString,
                                                        TRG3PSerNumString);
            return bStatus;
        }


        // in order to prevent duplicating DIO Mux Settings during testing, record
        // and re-implement the default MUX settings after each DBB1 test
        public UInt64 GetDBB1MuxDefault()
        {
            UInt64 Value = 0;
            bool bStatus = ReadFPGAreg("GlobalDBB1muxReg", ref Value);
            return Value;
        }

        public bool ReadFPGAreg(string sRegFldName, ref UInt64 Value)
        {
            bool bStatus = true;
            uint _BoardIndex = 0; // default (1st) board (for now)
            IntPtr NativeNameBuf = Marshal.StringToCoTaskMemAnsi(sRegFldName);
            IntPtr _UINT64_NativeBuffer = Marshal.AllocHGlobal(sizeof(UInt64) * 2); // should never fail
            uint uiStatus = ThorDAQAPIFPGAregisterRead(_BoardIndex, NativeNameBuf, sRegFldName.Length, _UINT64_NativeBuffer);
            if (uiStatus != 0)
            {
                bStatus = false;
            }
            else
            {
                Value = (UInt64)Marshal.ReadInt64(_UINT64_NativeBuffer);
            }
            // remember to free unmanaged mem
            Marshal.FreeCoTaskMem(NativeNameBuf);
            Marshal.FreeHGlobal(_UINT64_NativeBuffer);
            return bStatus;
        }
        public bool WriteFPGAreg(string sRegFldName, UInt64 value)
        {
            uint _BoardIndex = 0; // default (1st) board (for now)
            int _RegFldNameBufLen = 48;
            IntPtr _NativeBuffer = Marshal.AllocHGlobal(_RegFldNameBufLen); // should never fail
            IntPtr _UINT64_NativeBuffer = Marshal.AllocHGlobal(sizeof(UInt64) * 2); // should never fail

            bool bStatus = true;
            IntPtr NativeNameBuf = Marshal.StringToCoTaskMemAnsi(sRegFldName);
            Marshal.WriteInt64(_UINT64_NativeBuffer, (long)value);
            uint uiStatus = ThorDAQAPIFPGAregisterWrite(_BoardIndex, NativeNameBuf, sRegFldName.Length, _UINT64_NativeBuffer);
            if (uiStatus != 0)
                bStatus = false;  // e.g. Name not found
            // remember to free unmanaged mem
            Marshal.FreeCoTaskMem(NativeNameBuf);
            Marshal.FreeHGlobal(_NativeBuffer);
            Marshal.FreeHGlobal(_UINT64_NativeBuffer);

            return bStatus;
        }


        void ManufTest(List<String> argumentsList)
        {
            int iTestNum; // this is sent to NI1071
            int argNum;
            bool bBoardSN = false;
            bool bTestTDboard = false;
            bool bTestDACwaveforms = false;
            bool bTestADCwaveforms = false;
            bool bTestTRIGcard = false;
            bool bABB1test = false;
            bool bABB2test = false;
            bool bABB3test = false;
            bool bDBB1test = false;
            bool brunAlltests = false;
            int iAllTestIternations = 1;

            Int32 iABBxChan = 0xFF; // default - do all channels
            Int32 iPMTchan = 0xFF;  // def. all ADC channels
            Int32 iABB3volts = 9;   // optional +/- voltage arg, default
            string sDIOinOutBoth = "b"; // def. is "both" in and out
            Int32 iGenericTestIterations = 1;
            bool bBreakOnFirstError = false;
            UInt32 uiTrigCardFreq = 0;  // user may enter custom Freq for LP1 on 3P card
            bool bGenerateXLSreport = true;// false; 
            NIPXIe1071_Controller testNI_Computer;
            // we MUST have connection to the NI1071 to do this! 
            NI1071_IP_Addr = DoGetHostEntry(sNI1017_IPhostNameVirginia);
            sIPaddress = NI1071_IP_Addr.ToString();
            if (sIPaddress == "0.0.0.0")
            {
                // Virginia machine didn't answer - try Austin machine...
                NI1071_IP_Addr = DoGetHostEntry(sNI1017_IPhostNameAustin);
            }
            sIPaddress = NI1071_IP_Addr.ToString();  // these are default attempts - command line may offer IP address


            if (argumentsList.Count < 2)
            {
                UpdateConsoleStatus(HELP_ManufTest_MSSAGE);
                return;
            }
            // is there a persistent report file setting?
            if (File.Exists(@"C:\temp\MTreportFilepath.txt"))
            {
                ManufTest_DataDRIVE = 
                    System.IO.File.ReadAllText(@"C:\temp\MTreportFilepath.txt"); // the persistent file setting
                ManufTest_Data_RootFilepath = ManufTest_DataDRIVE + @"ManufTestReports\";
            }



            for (argNum = 1; argNum < argumentsList.Count; argNum++)
            {
                switch (argumentsList[argNum])
                {
                    case "-d":
                        ManufTest_DataDRIVE = argumentsList[argNum + 1] + "\\";  // replace the default
                        ManufTest_Data_RootFilepath = ManufTest_DataDRIVE + @"ManufTestReports\"; // resolve any Excel report drive option
                                                                                                  // write this new "default" to \temp text file for persistence
                        System.IO.File.WriteAllText(@"C:\temp\MTreportFilepath.txt", ManufTest_DataDRIVE);

                        argNum++;
                        break;
                    // capital letters denote one TCP command, then exit
                    case "-B":  // 
                        bBoardSN = true;
                        break;
                    case "-r":  // generate XLS report file
                        bGenerateXLSreport = true;
                        break;

                    case "-t":  // specify test code
                        iTestNum = Int32.Parse(argumentsList[argNum + 1]);
                        argNum++;
                        break;

                    case "-p":  // IP port number of NIPXIe1071 server
                        IP_port = Int32.Parse(argumentsList[argNum + 1]);
                        argNum++;
                        break;

                    case "-a":  // specify IP address
                        sIPaddress = argumentsList[argNum + 1];
                        argNum++;
                        try
                        {

                            NI1071_IP_Addr = IPAddress.Parse(sIPaddress);
                        }
                        catch (Exception e)
                        {
                            UpdateConsoleStatus(e.Message + ": " + sIPaddress);
                            return; // fatal error
                        }
                        break;

                    case "-E":
                        bBreakOnFirstError = true; // break on first iteration of Waveform error
                        break;

                    case "-I":
                        // get iteration count (next arg)
                        iGenericTestIterations = Int32.Parse(argumentsList[++argNum]);
                        break;

                    case "-TDboard": // TI power chips
                        bTestTDboard = true;
                        break;
                    case "-DACcard": // DAC mezzanine card test
                        bTestDACwaveforms = true;
                        if (((argNum + 1) < argumentsList.Count) && (argumentsList[argNum + 1].Substring(0, 1) != "-"))  // additional numeric arg?
                        {
                            // now get the channel (allows default)
                            iABBxChan = Int32.Parse(argumentsList[++argNum]);
                        }
                        break;
                    case "-ADCcard":
                        if (((argNum + 1) < argumentsList.Count) && (argumentsList[argNum + 1].Substring(0, 1) != "-"))  // additional numeric arg?
                        {
                            // now get the channel (allows default)
                            iPMTchan = Int32.Parse(argumentsList[++argNum]);
                        }
                        bTestADCwaveforms = true;
                        break;
                    case "-TRIGcard":
                        bTestTRIGcard = true;
                        if (((argNum + 1) < argumentsList.Count) && (argumentsList[argNum + 1].Substring(0, 1) != "-"))  // additional numeric arg?
                        {
                            uiTrigCardFreq = UInt32.Parse(argumentsList[++argNum]);
                        }
                        break;

                    case "-ABB1": // Analog BreakoutBox
                        bABB1test = true;
                        break;
                    case "-ABB2": // Analog BreakoutBox
                        bABB2test = true;
                        break;
                    case "-ABB3": // Analog BreakoutBox - could have "Vots" arg
                        bABB3test = true;
                        if (((argNum + 1) < argumentsList.Count) && (argumentsList[argNum + 1].Substring(0, 1) != "-"))  // additional numeric arg?
                        {
                            // now get the channel (allows default)
                            iABB3volts = Int32.Parse(argumentsList[++argNum]);
                        }

                        break;
                    case "-DBB1": // Analog BreakoutBox
                        bDBB1test = true;
                        if (((argNum + 1) < argumentsList.Count) && (argumentsList[argNum + 1].Substring(0, 1) != "-"))  // additional numeric arg?
                        {
                            // now get the channel (allows default)
                            sDIOinOutBoth = argumentsList[++argNum]; // 0 is Out, 1 is In, 0xFF is both In/Out

                        }
                        break;

                    case "-all": // run every test

                        if (sIPaddress == "0.0.0.0")
                            break; // failed to find NI1071 IP address
                        brunAlltests = true;
                        if (((argNum + 1) < argumentsList.Count) && (argumentsList[argNum + 1].Substring(0, 1) != "-"))  // additional numeric arg?
                        {
                            // now get the channel (allows default)
                            iAllTestIternations = Int32.Parse(argumentsList[++argNum]);
                        }

                        // for "all" we MUST be able to communicate with NI-1071; make sure it answers
                        testNI_Computer = new NIPXIe1071_Controller(sIPaddress, IP_port, "testIP.txt");
                        if (testNI_Computer.SrvResp != "SUCCESS")
                        {
                            UpdateConsoleStatus("NI-1071 computer at " + sIPaddress + ":" + IP_port.ToString() + ": " + testNI_Computer.SrvResp);
                            UpdateConsoleStatus("ERROR: " + testNI_Computer.SrvResp);
                            return;
                        }
                        bTestTDboard = true;
                        bTestADCwaveforms = true;
                        bTestDACwaveforms = true;
                        bABB1test = true;
                        bABB2test = true;
                        bABB3test = true;
                        bDBB1test = true;
                        bTestTRIGcard = true;  // reads ThorDAQ EEPROM S/N to detect backside mezz. slot
                        string sTDEEPROM;
                        GetI2CeepromSerNum(MasterBoardIndex, 0x80, 0xff, 0x54, 0x0, out sTDEEPROM);
                        string sMsg = "Starting ManufTest on ThorDAQ " + sTDEEPROM;
                        ThorLog.Instance.TraceEvent(TraceEventType.Information, 1, sMsg);

                        break;

                    default:
                        break;
                }
            }


            int iSrvStatus = 0;
            string ServerResponse = "Server Failed to Respond";

            // turn on LEDs for visual inspection
            // 
            APIBreakOutBoxLEDControl(new List<string> { "", "-all", "on" });
            Thread.Sleep(1000);
            APIBreakOutBoxLEDControl(new List<string> { "", "-all", "off" });

            APIGetBOBstatus(new List<string> { "" }); // identify connected BOB (if connected) - lights "Connected" for 3U


            // NOW PROCESS THE CL-GUI OPTIONS!!

            // if Report Workbook file requested, create (or open) it FIRST!
            if (bGenerateXLSreport == true)
            {
                if (TestReport == null)
                {
                    uint DUTestThorDAQbrdIndex = 0; // MainBoard - presume to be initially found (verify this with Multi-Board system)
                                                    // use ThorDAQ serial number and date in filename per spec:

                    string TDSerNumString;
                    bool bStatus = GetI2CeepromSerNum(DUTestThorDAQbrdIndex, 0x80, 0xFF, 0x54, 0, out TDSerNumString);
                    // Feature 1391, Add BOB SN to filespec
                    string BOBSerNumString;
                    bStatus = GetI2CeepromSerNum(DUTestThorDAQbrdIndex, 0x02, 0xFF, 0x50, 0, out BOBSerNumString); // DBB1 I2C mapping (Master,Slave,Address)
                    // Start with 3-letter component
                    string TDsnDateSubDir = TDSerNumString.Substring(0, 3);

                    // analyze the string for two cases - Legacy Austin TSI and Sterling TIS production case;
                    // TSI defined only first 28 bytes, TIS is 64 bytes. TIS only uses first 3 chars of
                    // first 28 bytes, then following that 10 bytes productNum and 6 bytes serialNum
                    // In Legacy case, only first 28 bytes are defined  (shown without dashes)
                    // TD3?????????????????????????7ITN002586002019???????????????????? (TIS case)
                    // TD302019-209044-3240-01/28/2018????????????????????????????????? (Legacy case)
                    
                    string sSerialNum = "000000";
                    string sBOBserialNum = "000000";
                    try
                    {
                        if (TDSerNumString.Substring(38, 1) == "?" || TDSerNumString.Substring(38, 1) == "\0")
                        {
                            sSerialNum = TDSerNumString.Substring(3, 5); // Legacy format, 4 digits SN
                            sBOBserialNum = BOBSerNumString.Substring(3, 5); // Legacy format
                        }
                        else
                        {
                            sSerialNum = TDSerNumString.Substring(38, 6); // 64-byte format, 6 digits SN
                            sBOBserialNum = BOBSerNumString.Substring(38, 6); 
                        }
                    }
                    catch (Exception ex)
                    {
                        UpdateConsoleStatus("ERROR: " + ex.Message + " EEPROM has invalid string " + TDSerNumString);
                    }

                    DateTime localDate = DateTime.Now;
                    TDsnDateSubDir = TDsnDateSubDir + sSerialNum + "_" + "BOB" + sBOBserialNum  + "_" + localDate.Year + "-" + localDate.Month + "-" + localDate.Day;
                    UpdateConsoleStatus("Excel filename " + TDsnDateSubDir);

                    // if the ROOT filepath does not exist, create it!
                    if (!Directory.Exists(ManufTest_Data_RootFilepath))
                    {
                        try
                        {
                            Directory.CreateDirectory(ManufTest_Data_RootFilepath);
                        }
                        catch(Exception e)
                        {
                            UpdateConsoleStatus("ERROR: cannot access file location " + ManufTest_Data_RootFilepath + " " + e.Message);
                            return;
                        }
                    }
                    ManufTest_Data_RootFilepath = ManufTest_Data_RootFilepath + TDsnDateSubDir + @"\";  // the FINAL TEST PATH for all (~70) data files for this ThorDAQ
                    // if the SerNum_Date filepath does not exist, create it!
                    UpdateConsoleStatus("Test filespec " + ManufTest_Data_RootFilepath);
                    if (!Directory.Exists(ManufTest_Data_RootFilepath))
                    {
                        Directory.CreateDirectory(ManufTest_Data_RootFilepath);
                    }

                    TestReport = new ManufTestReport(ManufTest_Data_RootFilepath, TDsnDateSubDir);
                    if (sIPaddress == "0.0.0.0")
                    {
                        UpdateConsoleStatus("ERROR - NI1071 hostnames not found, check Ethernet config");
                        return;
                    }
                    UpdateConsoleStatus("Configuring Spreadsheets... ");
                }
            }
            else  // just put everything in the ManufTest root folder (no SerNum_Date folder)
            {
                // if the ROOT filepath does not exist, create it!
                if (!Directory.Exists(ManufTest_Data_RootFilepath))
                {
                    Directory.CreateDirectory(ManufTest_Data_RootFilepath);
                }
            }

            while (iAllTestIternations-- > 0)
            {
                if (bBoardSN == true)
                {
                    uint DUTestThorDAQbrdIndex = 0; // we expect two Boards, Device Under Test and "Golden" board

                    ReadAllTDSerialNums(DUTestThorDAQbrdIndex, out AllTDSerialNumbers);
                    //                iSrvStatus = ControlNIPXIe1071(sIPaddress, IP_port, ("CMD: BOARDSN " + AllTDSerialNumbers), ref ServerResponse, ref NI1071_Data_Filepath);
                    UpdateConsoleStatus(ServerResponse);
                    if (iSrvStatus != 0)
                    {
                        UpdateConsoleStatus("...FAILED");
                    }
                    return; // quit now
                }

                void UpdateConsoleStatusTestRecord(string[] fields)
                {
                    string sRecord = fields[0] + " " + fields[1] + " " + fields[2] + " " + fields[3] + " " + fields[4] + " " + fields[5] + " " + fields[6] + " " + fields[7] + " " + fields[8];
                    UpdateConsoleStatus(sRecord);
                }


                if (bTestTDboard == true) // -TDboard option flah
                {
                    int PMbusDevAddr, TestNum;
                    // start TI Power Rails testing
                    ThorDAQmainBoardTestAndReport TdPowerTest = new ThorDAQmainBoardTestAndReport(MasterBoardIndex);
                    string[] TstHeader = TdPowerTest.GetTestHeader();
                    string[] TstRecord;
                    string[] TstDDR3EEPROMRec;

                    TdPowerTest.TestEEPROMserNum();

                    // loop through PMbus devices and get the temp + volts/current for all active rails
                    // bus device addresses according to hardware design
                    // DevAddr 54 has only 3 rails, not 4 like others
                    int RailMax = 4;
                    for (PMbusDevAddr = 52, TestNum = 2; PMbusDevAddr <= 54; PMbusDevAddr++)
                    {
                        if (PMbusDevAddr == 54) RailMax = 3;
                        TstRecord = TdPowerTest.ReadAndTestTIDevice(TestNum++, PMbusDevAddr, string.Format("Temp(\u00B0C)"), 0xFF); // no "rail" for temp
                        UpdateConsoleStatusTestRecord(TstRecord);
                        for (int Rail = 0; Rail < RailMax; Rail++)
                        {
                            TstRecord = TdPowerTest.ReadAndTestTIDevice(TestNum++, PMbusDevAddr, "VDC", Rail); //
                            UpdateConsoleStatusTestRecord(TstRecord);
                            TstRecord = TdPowerTest.ReadAndTestTIDevice(TestNum++, PMbusDevAddr, "Amps", Rail); //
                            UpdateConsoleStatusTestRecord(TstRecord);
                        }
                    }
                    TstDDR3EEPROMRec = TdPowerTest.TestDDR3_EEPROM(TestNum++);
                    UpdateConsoleStatusTestRecord(TstDDR3EEPROMRec);
                    TstDDR3EEPROMRec = TdPowerTest.TestDDR3_Memory(TestNum++, 0x0); // image DMA area
                    UpdateConsoleStatusTestRecord(TstDDR3EEPROMRec);
                    TstDDR3EEPROMRec = TdPowerTest.TestDDR3_Memory(TestNum++, 0x05000000); // waveform DMA area
                    UpdateConsoleStatusTestRecord(TstDDR3EEPROMRec);

               //     TestReport.FoundAfailedTestInWorksheet(); //TEST

                    if (brunAlltests == false) return; // done
                }
                if (bABB1test)
                {
                    string[] TstHeader;  // (also writes to Spreadsheet on "-r" option)
                    ABBxTestAndReport ABBtest = new ABBxTestAndReport(MasterBoardIndex, 1); // also creates TCP session to NI-1071
                    TstHeader = ABBtest.GetTestHeader(); // (also writes to Spreadsheet on "-r" option)

                    ABBtest.TestEEPROMserNum();
                    if (brunAlltests == false) return; // done
                }
                if (bABB2test)
                {
                    string[] TstHeader;  // (also writes to Spreadsheet on "-r" option)
                    ABBxTestAndReport ABBtest = new ABBxTestAndReport(MasterBoardIndex, 2); // also creates TCP session to NI-1071
                    TstHeader = ABBtest.GetTestHeader(); // (also writes to Spreadsheet on "-r" option)

                    ABBtest.TestEEPROMserNum();
                    if (brunAlltests == false) return; // done
                }
                if (bABB3test)
                {
                    string[] TstHeader;  // (also writes to Spreadsheet on "-r" option)
                    ABBxTestAndReport ABBtest = new ABBxTestAndReport(MasterBoardIndex, 3); // also creates TCP session to NI-1071
                    TstHeader = ABBtest.GetTestHeader(); // (also writes to Spreadsheet on "-r" option)

                    UpdateConsoleStatus(ABBtest.TestEEPROMserNum());
                    UpdateConsoleStatus(ABBtest.TestMAX127_ADC(iABB3volts));


                    if (brunAlltests == false) return; // done
                }
                if (bDBB1test)
                {
                    DBB1TestAndReport DBBtest = new DBB1TestAndReport(0); // also creates TCP session to NI-1071
                    DBBtest.GetTestHeader(); // (also writes to Spreadsheet on "-r" option)
                    UpdateConsoleStatus(DBBtest.TestEEPROMserNum());
                    UpdateConsoleStatus(DBBtest.TestLEDs());
                    string sStatus;
                    int thisTestIterations = iGenericTestIterations;
                    while (thisTestIterations-- > 0) // default is "1"
                    {
                        switch (sDIOinOutBoth)
                        {
                            case "o": // outputs
                                for (int iBNC = 0; iBNC < 31; iBNC++)
                                {
                                    if (iBNC == 7) continue; // skip the Pogo cut-out D7
                                    sStatus = DBBtest.ProgramAndAnalyzeDBB1asOutputs(iBNC, iBNC);
                                    UpdateConsoleStatus(sStatus); //  
                                    if (sStatus.Contains("failed")) // stop timeouts from taking minutes to complete
                                    {
                                        return;
                                    }
                                }
                                break;
                            case "i": // inputs
                                for (int iBNC = 0; iBNC < 31; iBNC++)
                                {
                                    if (iBNC != 7) // Pogo fixture cuts out D7 and D31 for PFI passthrough
                                    {
                                        sStatus = DBBtest.ProgramAndAnalyzeDBB1asInputs(iBNC, iBNC);
                                        UpdateConsoleStatus(sStatus); //  
                                        if (sStatus.Contains("failed")) // stop timeouts from taking minutes to complete
                                        {
                                            return;
                                        }
                                    }
                                }
                                break;
                            case "m":  // MUXing
                                for (int iBNC = 8; iBNC < 16; iBNC++)
                                {
                                    for(int iFPGAindex = 0; iFPGAindex < 8; iFPGAindex++)
                                    {
                                        sStatus = DBBtest.ProgramAndAnalyzeDBB1asInputs(iBNC, iFPGAindex);
                                        UpdateConsoleStatus(sStatus); //  
                                        if (!sStatus.Contains("PASS")) // stop timeouts from taking minutes to complete
                                        {
                                            return;
                                        }
                                    }
                                }
                                break;

                            default: // both
                                for (int iBNC = 0; iBNC < 31; iBNC++)
                                {
                                    if (iBNC == 7) continue; // skip the Pogo cut-out D7
                                    sStatus = DBBtest.ProgramAndAnalyzeDBB1asOutputs(iBNC, iBNC);
                                    UpdateConsoleStatus(sStatus); //  
                                    if (sStatus.Contains("failed")) // stop timeouts from taking minutes to complete
                                    {
                                        return;
                                    }
                                }
                                for (int iBNC = 0; iBNC < 31; iBNC++)
                                {
                                    if (iBNC != 7) // Pogo fixture cuts out D7 and D31 for PFI passthrough
                                        UpdateConsoleStatus(DBBtest.ProgramAndAnalyzeDBB1asInputs(iBNC, iBNC)); // turn then on one at a time
                                }
                                break;


                        } // end while
                    } // end ITERATIONS


                    if (brunAlltests == false) return; // done
                }
                // for TDQ2 (and above) check to 3P card test option
                if (bTestTRIGcard == true)
                {
                    TRIGtestAndReport TrigCardTest = new TRIGtestAndReport(MasterBoardIndex); // board index selectable by CL-GUI command
                    if (TrigCardTest.TrigCardPossible == true) // don't attempt on TDQ1
                    {
                        UpdateConsoleStatus(TrigCardTest.TestEEPROMserNum());
                        UpdateConsoleStatus(TrigCardTest.ReportTRIGcardVersions(2));
                        UpdateConsoleStatus(TrigCardTest.ReportTRIGcardVersions(3));

                        if (uiTrigCardFreq != 0)
                        {
                            UpdateConsoleStatus(TrigCardTest.SetupAndMeasureFreq(uiTrigCardFreq, 4));  // User's passed freq
                        }
                        else
                        {
                            UpdateConsoleStatus(TrigCardTest.SetupAndMeasureFreq(300000, 4));  // 300 kHz min, 5 MHz max design
                            UpdateConsoleStatus(TrigCardTest.SetupAndMeasureFreq(1000000, 5));
                            UpdateConsoleStatus(TrigCardTest.SetupAndMeasureFreq(2000000, 6));
                            UpdateConsoleStatus(TrigCardTest.SetupAndMeasureFreq(5000000, 7));
                        }
                        if (brunAlltests == false) return; // done
                    }
                    else
                    {
                        UpdateConsoleStatus("TDQ1 detected - no mezz. card possible");
                    }
                }
                if (bTestADCwaveforms == true)
                {
                    string[] TstHeader;  // (also writes to Spreadsheet on "-r" option)
                    ADCwTestAndReport ADCWaveformTest = new ADCwTestAndReport(MasterBoardIndex); // also creates TCP session to NI-1071
                    TstHeader = ADCWaveformTest.GetTestHeader(); // (also writes to Spreadsheet on "-r" option)
                    bool bRegressionTesting = false;
                    int iChanAfailures = 0;
                    int iChanBfailures = 0;
                    int iChanCfailures = 0;
                    int iChanDfailures = 0;
                    int TotalIterations = 0;
                    string TestStatus = ADCWaveformTest.TestEEPROMserNum();

                    int thisTestIterations = iGenericTestIterations;
                    if (iGenericTestIterations > 1)  // if we're doing regression, list channel failure counts
                    {
                        bRegressionTesting = true;
                        TotalIterations = iGenericTestIterations;
                    }
                    while (thisTestIterations > 0)
                    {
                        --thisTestIterations;
                        if (iPMTchan == 0xFF) // do all channels?
                        {
                            for (int iChan = 0; iChan < 4; iChan++)  // ABB1 & ABB2 channels
                            {
                                TestStatus = ADCWaveformTest.ProgramAndAnalyzeADCchanX(iChan);
                                UpdateConsoleStatus(TestStatus);
                                if (bRegressionTesting) // detect and accumulate any error on chan
                                {
                                    if (!TestStatus.Contains("PASS"))
                                    {
                                        if (TestStatus.Contains("chan A")) iChanAfailures++;
                                        if (TestStatus.Contains("chan B")) iChanBfailures++;
                                        if (TestStatus.Contains("chan C")) iChanCfailures++;
                                        if (TestStatus.Contains("chan D")) iChanDfailures++;
                                        // check flag to stop on first error
                                        if (bBreakOnFirstError == true) goto BreakOnFirstErr; // immediately escape iteration loop
                                    }
                                }
                            }
                        }
                        else
                        {
                            TestStatus = ADCWaveformTest.ProgramAndAnalyzeADCchanX(iPMTchan);
                            UpdateConsoleStatus(TestStatus);
                            if (bRegressionTesting) // detect and accumulate any error on chan
                            {
                                if (!TestStatus.Contains("PASS"))
                                {
                                    if (TestStatus.Contains("chan A")) iChanAfailures++;
                                    if (TestStatus.Contains("chan B")) iChanBfailures++;
                                    if (TestStatus.Contains("chan C")) iChanCfailures++;
                                    if (TestStatus.Contains("chan D")) iChanDfailures++;
                                    // check flag to stop on first error
                                    if (bBreakOnFirstError == true) goto BreakOnFirstErr; // immediately escape iteration loop
                                }
                            }
                        }
                    };  // end while ITERATIONS
                 BreakOnFirstErr:
                    if (bRegressionTesting && brunAlltests == false)
                    {

                        UpdateConsoleStatus("Iteration: " + (TotalIterations - thisTestIterations).ToString() + " of " + TotalIterations.ToString() + ", " +
                                                "Failures: A " + iChanAfailures.ToString() + ", B " + iChanBfailures.ToString() +
                                                        ", C " + iChanCfailures.ToString() + ", D " + iChanDfailures.ToString());
                    }

                   
                    if (brunAlltests == false) return; // done
                }

                if (bTestDACwaveforms == true) // -DACcard option flag
                {
                    string[] TstHeader;  // (also writes to Spreadsheet on "-r" option)
                    DACwTestAndReport DACWaveformTest = new DACwTestAndReport(MasterBoardIndex); // also creates TCP session to NI-1071
                    TstHeader = DACWaveformTest.GetTestHeader(); // (also writes to Spreadsheet on "-r" option)
                                                                 // Test#1 is EEPROM serial num for this component
                    DACWaveformTest.TestEEPROMserNum();
                    string sVer = DACWaveformTest.GetDACCPLDver();
                    UpdateConsoleStatus(sVer);
                    int iTotalErrs = 0;
                    int TotalIterations = iGenericTestIterations;
                    int thisTestIterations = iGenericTestIterations;
                    while (thisTestIterations-- > 0)
                    {
                        if (iABBxChan == 0xFF) // do all channels?
                        {
                            for (int iChan = 0; iChan < 12; iChan++)  // ABB1 & ABB2 channels OK Legacy ABB3 fails 8,9
                            {
                                DACWaveformTest = new DACwTestAndReport(MasterBoardIndex); // also creates TCP session to NI-1071
                                string TestStatus = DACWaveformTest.ProgramAndAnalyzeABBxChannel(iChan);
                                UpdateConsoleStatus(TestStatus);
                                if (!TestStatus.Contains("PASS")) // exit while loop on error
                                {
                                    iTotalErrs++;
                                    if (bBreakOnFirstError == true) break;
                                }
                            }
                        }
                        else // single channel
                        {
                            DACWaveformTest = new DACwTestAndReport(MasterBoardIndex); // also creates TCP session to NI-1071
                            string TestStatus = DACWaveformTest.ProgramAndAnalyzeABBxChannel(iABBxChan);
                            UpdateConsoleStatus(TestStatus);
                            if (!TestStatus.Contains("PASS")) // exit while loop on error
                            {
                                iTotalErrs++;
                                if (bBreakOnFirstError == true) break;
                            }
                        }
                    }
                }
                if (brunAlltests == false)
                {
                    return;
                }
            };
        } // manuftest exit

        // The ThorDAQ computer file is single column (sent to 1 or more DAC channels)
        // The NI PXIe-1017 file is four columns separated by commas
        long FindFirstZeroCrossingSampleNum(string VoltageDataFile)
        {
            bool AbsValueLowTrigger = false;
            // In each file - control and measured - find the sample numbers of min. ABS value
            // which should be the frist zero crossing.  Those sample numbers are to match phases
            string[] lines = System.IO.File.ReadAllLines(VoltageDataFile);
            const double ExpectedZeroCrossingLimit = 1.0; // expect a zero crossing with ABSv lower than this voltage
            double MinVolts = ExpectedZeroCrossingLimit;
            long totalSamplesToCheck = 1000000;  // @ 500KSample/sec, say 2 seconds of samples
            long sampleOrdinal = 0;
            double thisVoltage;
            long zeroCrossingSample = -1;  // def. not found
            foreach (string line in lines) // 500 KSample/sec
            {
                sampleOrdinal++;
                if (--totalSamplesToCheck <= 0)
                {

                    break; // time to give up?
                }
                if (line.Contains(","))
                {
                    string[] VoltsByChan = line.Split(',');
                    thisVoltage = double.Parse(VoltsByChan[0]);
                    thisVoltage = Math.Abs(thisVoltage);
                }
                else // single column
                {
                    thisVoltage = double.Parse(line);
                    thisVoltage = Math.Abs(thisVoltage);
                }
                if (thisVoltage < MinVolts)
                {
                    AbsValueLowTrigger = true;  // when next voltages rises about limit we'll break
                    MinVolts = thisVoltage;
                    zeroCrossingSample = sampleOrdinal;
                    // we expect to be close to zero crossing - when ABSv increases, exit loop
                }
                if (thisVoltage > ExpectedZeroCrossingLimit && AbsValueLowTrigger) break; // done!

            }

            return zeroCrossingSample;
        }

        // Analyze the ideal versus measured files...
        // for the DAC waveform generated by ThorDAQ & measured by NI1071 the "Ideal" waveform was produced by 
        // CL-GUI "BuildWaveform" function
        // for the ADC waveform generated by NI1071 and measured by ThorDAQ, the "Ideal" waveform produced by ????
        public class AnalyzeNI1071VoltageWaveform
        {
            private string _IdealFileSpec;       // what did we program hardware to do?
            private string _IdealVoltsFileSpec;  // Ideal file in single-line volts records
            private string _MeasuredFileSpec;    // what was received?
            private string _MeasuredVoltsFileSpec;  // Measured file in single-line volts records
            private bool _DACwaveform;       // if TRUE, DAC waveform (ABBx), otherwise ADC (PMT connector) waveform
            private int _IdealSamples;
            private int _MeasuredSamples;

            public AnalyzeNI1071VoltageWaveform(bool DAC, string IdealFileSpec, string MeasuredFileSpec) // ideal, measured waveforms
            {
                _MeasuredFileSpec = MeasuredFileSpec;  // expect both to be .txt text files
                _IdealFileSpec = IdealFileSpec;
                _DACwaveform = DAC;
            }
            // BuildWaveform produces 16-bit DAC counts for ThorDAQ - convert to VOLTS to compare with NI1071 measured VOLTS
            // and make both Ideal and Measure single text value line of voltage (e.g. 9.4607)
            public string ConvertFilesToSingleLineVoltageRecords()
            {
                string sMsg = "SUCCESS";
                _IdealSamples = 0;
                _MeasuredSamples = 0;
                string[] filenameSplit = _MeasuredFileSpec.Split('.'); // e.g. replace .txt with .volts
                _MeasuredVoltsFileSpec = filenameSplit[0] + ".volts";

                if (_DACwaveform) // case for ThorDAQ produced Ideal waveform
                {
                    StreamWriter outFile = null;
                    try
                    {
                        _IdealSamples = 0;
                        _MeasuredSamples = 0;

                        // ThorDAQ Buildwave produces DAC "counts" - convert to volts
                        _IdealVoltsFileSpec = ManufTest_Data_RootFilepath + "TDDAC_AOxx.volts";  // same volts Ideal File for all channels
                        outFile = File.CreateText(_IdealVoltsFileSpec);

                        string[] lines = System.IO.File.ReadAllLines(_IdealFileSpec);

                        double Volts;
                        NumberFormatInfo nfi = new CultureInfo("en-US", false).NumberFormat;
                        nfi.NumberDecimalDigits = 12;

                        foreach (string line in lines)
                        {
                            // one DAC value per line - convert to volts
                            UInt16 DACcnts = UInt16.Parse(line);
                            // will voltage be positive or negative?

                            Volts = ((20.0 / 65536.0) * (DACcnts)) - 10.0;
                            outFile.WriteLine(Volts.ToString("G"));
                            _IdealSamples++; // tally the count of samples
                        }
                        outFile.Close();

                        // now do the NI1071 file, splitting CSV voltage values in ".volts" file with one voltage / line
                        outFile = File.CreateText(_MeasuredVoltsFileSpec);
                        lines = System.IO.File.ReadAllLines(_MeasuredFileSpec);

                        string[] VoltRecordLines = new string[8096];
                        VoltRecordLines = lines[0].Split(',');  // the file is SINGLE LINE with CSV voltages
                        //
                        _MeasuredSamples = VoltRecordLines.Length;
                        // Discard ideal samples than exceed the measured samples...
                        _IdealSamples = (_IdealSamples > _MeasuredSamples) ? _MeasuredSamples : _IdealSamples;

                        foreach (string VoltRecord in VoltRecordLines)
                        {
                            outFile.WriteLine(VoltRecord);
                        }
                        outFile.Close();
                    }
                    catch (Exception e)
                    {
                        sMsg = " Excep: " + e.Message;
                    }
                    finally
                    {
                        if (outFile != null)
                            outFile.Close();
                    }
                }
                else // the ADC case -- ThorDAQ does ADC on incoming wave from NI1071 DAC
                {
                    StreamWriter outMeasuredVoltsFile = null;
                    try
                    {
                        _IdealSamples = 0;
                        _MeasuredSamples = 0;

                        // convert the ThorDAQ DDR3 file, a single ADC count per line, to one voltage / line
                        outMeasuredVoltsFile = File.CreateText(_MeasuredVoltsFileSpec);
                        string[] ADCcountLines = System.IO.File.ReadAllLines(_MeasuredFileSpec);

                        foreach (string ADCRecord in ADCcountLines)
                        {
                            double dCnts = Convert.ToDouble(ADCRecord);
                            double dVolts = (dCnts / 5461.3333) - 1.5;
                            outMeasuredVoltsFile.WriteLine(dVolts.ToString("G"));
                            _MeasuredSamples++;
                        }
                        outMeasuredVoltsFile.Close();

                        // convert the MeasuredFile ADC count records (lines) to volts (lines)
                        // ThorDAQ Buildwave produces DAC "counts" - convert to volts
                        _IdealVoltsFileSpec = ManufTest_Data_RootFilepath + "NIDAC_AOx.volts";  // same volts Ideal File for all channels
                        StreamWriter outFile = File.CreateText(_IdealVoltsFileSpec);

                        string[] DACVoltsLine = System.IO.File.ReadAllLines(_IdealFileSpec);
                        string[] VoltRecordLines = DACVoltsLine[0].Split(',');  // the file is SINGLE LINE with CSV voltages

                        foreach (string line in VoltRecordLines)
                        {
                            outFile.WriteLine(line);
                            _IdealSamples++; // tally the count of samples
                        }
                        outFile.Close();


                    }
                    catch (Exception e)
                    {
                        StackFrame CallStack = new StackFrame(1, true);
                        sMsg = " ManufTest.cs Excep @ " + CallStack.GetFileLineNumber() + e.Message;
                    }
                    finally
                    {
                        if (outMeasuredVoltsFile != null)
                            outMeasuredVoltsFile.Close();
                    }



                }
                return sMsg;
            }

            // MeasuredToIdealSampleRatio is 1 (1:1) for ThorDAQ ADC, and 2 (2:1) for NI ADC
            public string RMSdeviation(int MeasuredToIdealSampleRatio, ref int StartingIdealSample, ref int StartingMeasuredSample, ref double dRMSE)
            {
                string sReturnMsg = "SUCCESS";
                // make 128k size buffers
                const int SAMPLE_BUFF_SIZE = (128 * 1024);
                double[] dIdealValues = new double[SAMPLE_BUFF_SIZE];
                double[] dMeasuredValues = new double[SAMPLE_BUFF_SIZE];
                int[] IdealWaveZeroCrossings = new int[3];
                int[] MeasuredWaveZeroCrossings = new int[3];
                int i, m;  // Ideal and Measured Volts array indexes
                bool bLookForZeroCrossing = false;
                dRMSE = -1.0; // invalid def.
                try
                {
                    System.IO.StreamReader IdealFile =
                       new System.IO.StreamReader(_IdealVoltsFileSpec);
                    System.IO.StreamReader MeasuredFile =
                        new System.IO.StreamReader(_MeasuredVoltsFileSpec);
                    string DoubleValueLine;
                    _IdealSamples = 0;
                    _MeasuredSamples = 0;
                    // read the files into arrays of doubles and count samples...
                    while (((DoubleValueLine = IdealFile.ReadLine()) != null) && _IdealSamples < SAMPLE_BUFF_SIZE)
                    {
                        dIdealValues[_IdealSamples++] = Convert.ToDouble(DoubleValueLine);
                    }
                    IdealFile.Close();
                    while (((DoubleValueLine = MeasuredFile.ReadLine()) != null) && _MeasuredSamples < SAMPLE_BUFF_SIZE)
                    {
                        dMeasuredValues[_MeasuredSamples++] = Convert.ToDouble(DoubleValueLine);
                    }
                    MeasuredFile.Close();

                    // TO START - presume a data rate of 1 MSPS for ThorDAQ DAC Ideal WaveGeneration
                    // data rate of 2.86 MSPS for NI1071 DAC
                    // and 2MSPS for NI1071 ADC Measured Wave.  So we have two save samples measured for every one sample
                    // generated

                    // TO START - presume 1240Hz frequency of ThorDAC's DAC sine wave (formed with 1 MSPS DAC output rate)
                    // and 15kHz frequency for PXIe-6363 DAC (formed at 2.86 MSPS DAC output rate)

                    // Algorithm:
                    // A. Expect at least FIVE negative VDC as starting voltage near beginning of Measured file
                    // B. (At 1240 Hz) Expect values to increase from ~ -10 VDC to 0 VDC
                    // C. Identify a sample # at 0 VDC crossing for both files
                    // E. Start RMSE computation
                    // D. Continue until SECOND zero crossing detected
                    // E. Return result

                    if (StartingIdealSample == 0) // need to discover first wave crossing samples?
                    {                             // if non-zero, the function is NOT analyzing first wave period
                        // check for the special case of ALL ZERO ADC samples in the DDR3 memory waveform sample space
                        // This symptom is occassional seen when apparently nothing in DMA'd into the expected memory are
                        // (the test routine clears memory to 0, expecting SOMETHING to be overwritten by
                        // the DMA engine, even if it's not correct - e.g. malfuncting ADC hardware)
                        int k;
                        for (k = 0; k < 1024 * 4; k++) // check 1st 1k
                        {
                            if (dMeasuredValues[k] != -1.5)
                                break; // checking for ADC raw counts = 0, which is -1.5 volts 
                        }
                        if (k >= 1024 * 4)
                            return " suspect no DMA'd data in DDR3; raw ADC values all 0 (-1.5 volts)";

                        // A.
                        // Check for existence of ~ -10 VDC "header" on measured waveform
                        int SamplesBelow_0_VDC = 0;
                        const int CountOfNegativeVDCneeded = 5;
                        for (i = 0; i < 600; i++) // look for at least one 15kHz period, and error condition of slower freq.
                        {
                            // look for at least 10 measured values below 9.9 VDC
                            // e.g. catches flat-lined noise around 0 VDC
                            if (dMeasuredValues[i] < 0.0)
                            {
                                SamplesBelow_0_VDC++;
                                if (SamplesBelow_0_VDC >= CountOfNegativeVDCneeded) break;
                            }
                        }
                        if (SamplesBelow_0_VDC < CountOfNegativeVDCneeded)
                        {
                            return " beginning of measured waveform lacks negative voltage samples";
                        }
                        // B., C.
                        // Obviously, the higher the sine wave frequency, the fewer samples it takes to reach 0 crossing point

                        for (i = 0; i < _IdealSamples; i++)
                        {
                            if (dIdealValues[i] > 0) // crossed zero?
                            {
                                IdealWaveZeroCrossings[0] = i;  // found it - record sample index
                                StartingIdealSample = i;
                                break;
                            }
                            if ((i + 1) >= _IdealSamples)
                            {
                                return " ideal waveform missing first zero crossing";
                            }
                        }
                        // Measured is sampled twice as fast as Ideal (for NIADC)
                        // and at same rate for NIDAC 
                        // at beginning of Measured samples because we have known anomolies
                        // due to FPGA initially stuffing DAC DMA FIFOs
                        // (These anomolies not present when real ThorImageLS hardware config done)
                        for (m = IdealWaveZeroCrossings[0] - 5; m < _MeasuredSamples; m++)
                        {
                            if (dMeasuredValues[m] > 0) // crossed zero?
                            {
                                MeasuredWaveZeroCrossings[0] = m;
                                StartingMeasuredSample = m;
                                break;
                            }
                            if ((m + 1) >= _MeasuredSamples)
                            {
                                return " measured waveform missing first zero crossing";
                            }
                        }
                    }
                    else  // arguments passed for first crossing sample to analyze
                    {
                        IdealWaveZeroCrossings[0] = StartingIdealSample;
                        MeasuredWaveZeroCrossings[0] = StartingMeasuredSample;
                    }
                    // E.
                    // We have first zero crossing sample-index for both ideal and measured array
                    // Start summation of errors
                    int totalSamplesInPeriod = 0; // starting a first detected 0 VDC crossing, count of all samples until 3rd crossing
                    double RMSaccumulator = 0;
                    i = StartingIdealSample; // value just discovered, or passed into function
                    m = StartingMeasuredSample;
                    int zeroCrossings = 1;
                    for (; (i < _IdealSamples) && (m < _MeasuredSamples) && (zeroCrossings < 3); i++, m += MeasuredToIdealSampleRatio)  // measured file has 2x samples of ideal file
                    {
                        totalSamplesInPeriod++;
                        double Mvalue = dMeasuredValues[m];
                        // if needed, average the two measured values that should correspond with the single ideal value
                        if (MeasuredToIdealSampleRatio == 2)
                        {
                            Mvalue = (dMeasuredValues[m] + dMeasuredValues[m - 1]) / 2;
                        }
                        // compute RMSE
                        RMSaccumulator += Math.Pow((dIdealValues[i] - Mvalue), 2.0);

                        if (dIdealValues[i] > 0.100 || dIdealValues[i] < -0.100) bLookForZeroCrossing = true; // "arm" the crossing check
                        if (bLookForZeroCrossing)
                        {
                            switch (zeroCrossings) //  mark the sample numbers when we cross 0 VDC
                            {
                                case 1: // looking for positive to negative crossing
                                    if (dIdealValues[i] < 0.0)
                                    {
                                        IdealWaveZeroCrossings[zeroCrossings] = i;
                                        MeasuredWaveZeroCrossings[zeroCrossings] = m;
                                        zeroCrossings++; // found next zero cross
                                        bLookForZeroCrossing = false;  // re-arm when we get near amplitude peak
                                    }
                                    break;
                                case 2: // looking for negative to positive crossing
                                    if ((dIdealValues[i]) > 0.0 && (dIdealValues[i - 1] <= 0.0))
                                    {
                                        IdealWaveZeroCrossings[zeroCrossings] = i;
                                        MeasuredWaveZeroCrossings[zeroCrossings] = m;
                                        zeroCrossings++; // found next zero cross
                                        bLookForZeroCrossing = false;
                                    }
                                    break;
                                case 3:  // we are DONE - found the 3rd crossing, complete period analyzed

                                    break;
                                default:
                                    break; // do nothing
                            }
                        }
                    } // end RSME accumulator loop
                      // did we cross zero volts 3 times?
                    if (zeroCrossings == 3)
                    {
                        dRMSE = Math.Sqrt((RMSaccumulator / totalSamplesInPeriod));
                        StartingIdealSample = IdealWaveZeroCrossings[2];                // for next wave analysis (if called)
                        StartingMeasuredSample = MeasuredWaveZeroCrossings[2];
                    }
                    else
                    {
                        sReturnMsg = "FAIL: could not find 3 wave crossovers of 0 VDC";
                    }

                }
                catch (Exception e)
                {
                    sReturnMsg = "FAIL: Excep Msg " + e.Message;
                }
                return sReturnMsg;
            }  // end RMSerror

        }; // end class "AnalyzeNI1071CapturedWaveform"

        // UNIT TEST the DFT
        public void DFT(List<String> argumentsList)
        {
            string fileSpec = @"c:\temp\DFTunitTest.csv";
            bool HannWindow = false;

            for (int argNum = 1; argNum < argumentsList.Count; argNum++)
            {
                switch (argumentsList[argNum])
                {
                    case "-w": // use Hann window function on real x[n] data samples
                        // test case  
                        HannWindow = true;
                        UpdateConsoleStatus("Using Hann Windowing... ");

                        break;
                    default:
                        break;
                }
            }

            UpdateConsoleStatus("DFT unit test on file " + fileSpec);
            dft = new DiscreteFourierTrans();
            long ReadbackFileZeroCrossingSample = FindFirstZeroCrossingSampleNum(fileSpec);
            dft.ComputeDFT(fileSpec, ReadbackFileZeroCrossingSample, 0, 500000, HannWindow);
            //DiscreteFourierTrans 
        }
    }

}
