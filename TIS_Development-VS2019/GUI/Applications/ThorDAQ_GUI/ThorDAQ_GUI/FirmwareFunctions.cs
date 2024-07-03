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
using System.IO;
using ThorLogging;
using System.Diagnostics;

using NWLogic.DeviceLib;

namespace thordaqGUI
{
    public partial class MainWindow : Window
    {
        const uint BUSY_STATUS = 0x00000100;
        const uint NO_DEV = 0x00000200;
        const uint UNKNOWN_ERR = 0x00000400;

        //  EEPROM definition: see doc "ThorDAQ PCIe Adapter Production Firmware Loads and Bring-Up Process"
        // Sec. 7; for legacy compatibility, 1st 5 digits of SN repeated at beginning
        // c - test code, x for untested, g for good, b for bad
        //        n      c   manuf     DOM      10-dig PN  6 SN   remarks 
        // TD   TD302013-x39044-0002-10/02/2018-7ITN002587-002013-annotations for card
        // ADC  ADC02013-b39042-0002-10/01/2018-7ITN002896-002013-chan D out of spec
        // DAC  DAC02013-b01822-2312-06/01/2022-7ITN002899-002013-chan A11 fails

        //                                    COMPONENT_SZ-PART_NUM_DZ-SER_NUM_SZ
        const uint SER_NUM_SZ = 5;
        const uint COMPONENT_SZ = 3;    // e.g. TD1 (main board type), LFT, DBB, AB2. DAC ...
        const uint MANUF1 = 6;
        const uint MANUF2 = 4;
        const uint DATE_SZ = 10;
        // "Legacy" length was 28
        const uint PART_NUM_SZ = 10;
        const uint AIX_SER_NUM_SZ = 6; // new format SN
        const uint REMARKS = 20;
        // total length 64 -- 4 "pages" of 16 bytes each written to EEPROM

        const uint TOT_SERIAL_SZ = COMPONENT_SZ + SER_NUM_SZ + MANUF1 + MANUF2 + DATE_SZ + PART_NUM_SZ + AIX_SER_NUM_SZ + REMARKS;
        const int WAIT_TIME = 50;
        const int NUM_ITER = 10;
        const uint BAR3 = 3;

        const string UNLOCK_CMD = "0x556E6C6B";
        const string ERASE_CMD = "0x45726173";
        const string PROG_CMD = "0x50726F67";
        const string RESET_ASYNC = "0x656E6C5A";
        const string FPGA_FLASH_PROG_CMD = "WriteMem 3 0x0038 4 ";
        const string FPGA_PROG_STATUS_CMD = "ReadMem 3 0x02 1";
        const Byte PROG_READY_STATUS = 0x40;
        const Byte FIFO_FULL_STATUS = 0x20;
        SplashScreen _splash;
        BGsplashScreen _BGsplash;
        int _progressPercentage = 0;
        #region Delegates

        public delegate void UpdateProgressDelegate(int percentage, string text);

        #endregion Delegates

        enum RecordType : int
        {
            Data = 0,
            EOF = 1,
            ExtSegmentAddress = 2,
            ExtLinearAddress = 4,
        }



        // From LeoM code...
        // Win32::Process::Memory - read and write memory of other windows' process
        private bool WriteEEPromRegister(int devInst, IntPtr buffer, UInt64 bufferLen, UInt64 address, string commmand)
        {
            commmand = commmand.Replace("0x", string.Empty);
            for (int i = 0; i < ((int)bufferLen * 2 - commmand.Length); i++)
            {
                commmand = '0' + commmand;
            }
            for (int i = 0; i < (int)bufferLen; i++)
            {
                byte byteValue = byte.Parse(commmand.Substring(i * 2, 2), NumberStyles.HexNumber, CultureInfo.InvariantCulture);
                System.Runtime.InteropServices.Marshal.WriteByte(buffer, (int)bufferLen - 1 - i, byteValue);
            }
            return Win32.MemoryWrite((uint)devInst, buffer, BAR3, address, 0, bufferLen);
        }


        private UInt32 ReadEEPromStatusRegister(int devInst, IntPtr buffer)
        {
            if (!(Win32.MemoryRead((uint)devInst, buffer, BAR3, (UInt64)0x02c0, 0, 4)))
            {
                return 0x400;// UNKNOWN_ERR;
            }

            UInt32 result = (UInt32)System.Runtime.InteropServices.Marshal.ReadInt32(buffer);
            return result;
        }



        private string ReadEEPromSerInfo(int devIndx, EEPromCMDs component)
        {
            string serialStr = string.Empty;
            IntPtr i2CBuffer = IntPtr.Zero;
            IntPtr stbuffer = IntPtr.Zero;
            UInt32 busyStatus;
            int iter1 = 0;
            bool errorCondition = false;
            Win32.AllocateBuffer(ref stbuffer, 4);
            Win32.AllocateBuffer(ref i2CBuffer, 4);

            if (component >= EEPromCMDs.INV_CMD)
            {
                serialStr = "Invalid EEPROM device: " + component;
                errorCondition = true;
                goto ReadOperationComplete;
            }

            if (component == EEPromCMDs.MB_SER)
            {
                if (!WriteEEPromRegister(devIndx, i2CBuffer, 4, 0x02C8, "0x00000002"))
                {
                    errorCondition = true;
                    goto ReadOperationComplete;
                }
                if (!WriteEEPromRegister(devIndx, i2CBuffer, 4, 0x02C0, "0x00000080"))
                {
                    errorCondition = true;
                    goto ReadOperationComplete;
                }
                if (!WriteEEPromRegister(devIndx, i2CBuffer, 4, 0x02C4, "0x00000271"))
                {
                    errorCondition = true;
                    goto ReadOperationComplete;
                }
                if (!WriteEEPromRegister(devIndx, i2CBuffer, 4, 0x02C8, "0x00000003"))
                {
                    errorCondition = true;
                    goto ReadOperationComplete;
                }

                iter1 = 0;

                do
                {
                    System.Threading.Thread.Sleep(WAIT_TIME);
                    busyStatus = ReadEEPromStatusRegister(devIndx, stbuffer);
                    if (Convert.ToBoolean(busyStatus & NO_DEV) || Convert.ToBoolean(busyStatus & UNKNOWN_ERR) || iter1 >= NUM_ITER)
                    {
                        errorCondition = true;
                        break;
                    }
                    iter1++;
                } while (Convert.ToBoolean(busyStatus & BUSY_STATUS) && iter1 < NUM_ITER);

                if (errorCondition)
                {
                    goto ReadOperationComplete;
                }

                for (int i = 0; i < (int)TOT_SERIAL_SZ; i++)
                {
                    string intvalstr = "0x" + i.ToString("X8");
                    if (!WriteEEPromRegister(devIndx, i2CBuffer, 4, 0x02C8, "0x00000002"))
                    {
                        errorCondition = true;
                        break;
                    }

                    if (!WriteEEPromRegister(devIndx, i2CBuffer, 4, 0x02C0, intvalstr))
                    {
                        errorCondition = true;
                        break;
                    }

                    if (!WriteEEPromRegister(devIndx, i2CBuffer, 4, 0x02C4, "0x000122D4"))
                    {
                        errorCondition = true;
                        break;
                    }

                    if (!WriteEEPromRegister(devIndx, i2CBuffer, 4, 0x02C8, "0x00000003"))
                    {
                        errorCondition = true;
                        break;
                    }

                    iter1 = 0;

                    do
                    {
                        System.Threading.Thread.Sleep(WAIT_TIME);
                        busyStatus = ReadEEPromStatusRegister(devIndx, stbuffer);
                        if (Convert.ToBoolean(busyStatus & NO_DEV) || Convert.ToBoolean(busyStatus & UNKNOWN_ERR) || iter1 >= NUM_ITER)
                        {
                            errorCondition = true;
                            break;
                        }
                        iter1++;
                    } while (Convert.ToBoolean(busyStatus & BUSY_STATUS) && iter1 < NUM_ITER);

                    if (!errorCondition)
                    {
                        serialStr = serialStr + ((char)busyStatus).ToString();
                    }
                    else
                    {
                        serialStr = "EEPROM ReadFail";
                        break;
                    }
                }
            }

        ReadOperationComplete:
            if (stbuffer != IntPtr.Zero)
            {
                Win32.FreeBuffer(stbuffer);
            }
            if (i2CBuffer != IntPtr.Zero)
            {
                Win32.FreeBuffer(i2CBuffer);
            }

            return serialStr;
        }


        // Discover Mezzanine cards and attached break-out boxes and their SerialNums, etc.
        public List<string> ReadFromEEProms(bool bCreateFile, string fileSpec)
        {
            var EEPromList = new List<string>();
            var fileFormatStrings = new List<string>(); // human-readable format

            BackgroundWorker splashWkr = new BackgroundWorker();
            splashWkr.WorkerSupportsCancellation = true;
            
            _BGsplash = new BGsplashScreen(splashWkr);
            _BGsplash.DisplayText = "Please wait while discovering and reading EEPROM data...";
            _BGsplash.ShowInTaskbar = false;
            _BGsplash.Owner = Application.Current.MainWindow;
            _BGsplash.Show();

            //get dispatcher to update the contents that was created on the UI thread:
            System.Windows.Threading.Dispatcher spDispatcher = _BGsplash.Dispatcher;

            // Define what will be done when we start the background task
            bool bEEPROMreadOK;
            
            splashWkr.DoWork += delegate(object sender, DoWorkEventArgs e)
            {
                if (splashWkr.CancellationPending) return;
                //create a new delegate for updating our progress text
                UpdateProgressDelegate update = new UpdateProgressDelegate(UpdateBGProgressText);
                    
                //invoke the dispatcher and pass the percentage
                _progressPercentage += 10;
                // access hardware
                string sBuf = "MB Serial: ";
                bEEPROMreadOK = ThorDAQAdapters[MasterBoardIndex].MainBrd.ReadEEPROM();
                string rBuf = ThorDAQAdapters[MasterBoardIndex].MainBrd.ProductionData; // good data or err message
                if (bEEPROMreadOK) sBuf += FormatSerStr(rBuf, false);
                else sBuf += rBuf;
                EEPromList.Add(sBuf);
                string fBuf = "MB   ";
                fileFormatStrings.Add(fBuf + rBuf);
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, sBuf);
                spDispatcher.BeginInvoke(update, new object[] { _progressPercentage, sBuf });

          
                
                //invoke the dispatcher and pass the percentage
                _progressPercentage += 10;
                // access hardware
                sBuf = "TRG3P Serial: ";
                bEEPROMreadOK = ThorDAQAdapters[MasterBoardIndex].TRG3Pcard.ReadEEPROM();
                rBuf = ThorDAQAdapters[MasterBoardIndex].TRG3Pcard.ProductionData; // good data or err message
                if (bEEPROMreadOK) sBuf += FormatSerStr(rBuf, false);
                else sBuf += rBuf;
                EEPromList.Add(sBuf);
                fBuf = "TRG3P";
                fileFormatStrings.Add(fBuf + rBuf);
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, sBuf);
                spDispatcher.BeginInvoke(update, new object[] { _progressPercentage, sBuf });
                
                //invoke the dispatcher and pass the percentage
                _progressPercentage += 10;
                // access hardware
                sBuf = "ADC Serial: ";
                bEEPROMreadOK = ThorDAQAdapters[MasterBoardIndex].ADCcard.ReadEEPROM();
                rBuf = ThorDAQAdapters[MasterBoardIndex].ADCcard.ProductionData; // good data or err message
                if (bEEPROMreadOK) sBuf += FormatSerStr(rBuf, false);
                else sBuf += rBuf;
                EEPromList.Add(sBuf);
                fBuf = "ADC  ";
                fileFormatStrings.Add(fBuf + rBuf);
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, sBuf);
                spDispatcher.BeginInvoke(update, new object[] { _progressPercentage, sBuf });

                //invoke the dispatcher and pass the percentage
                _progressPercentage += 10;
                // access hardware
                sBuf = "DBB1 Serial: ";
                bEEPROMreadOK = ThorDAQAdapters[MasterBoardIndex].DBB1box.ReadEEPROM();
                rBuf = ThorDAQAdapters[MasterBoardIndex].DBB1box.ProductionData; // good data or err message
                if (bEEPROMreadOK) sBuf += FormatSerStr(rBuf, false);
                else sBuf += rBuf;
                EEPromList.Add(sBuf);
                fBuf = "DBB1 ";
                fileFormatStrings.Add(fBuf + rBuf);
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, sBuf);
                spDispatcher.BeginInvoke(update, new object[] { _progressPercentage, sBuf });
                
                _progressPercentage += 10;
                // access hardware
                sBuf = "CPLD2 UsrCode: ";
                bEEPROMreadOK = ThorDAQAdapters[MasterBoardIndex].CPLD2dev.ReadCPLDver();
                rBuf = ThorDAQAdapters[MasterBoardIndex].CPLD2dev.ProductionData; // good data or err message
                sBuf += rBuf; // simple 32-bit code
                EEPromList.Add(sBuf);
                spDispatcher.BeginInvoke(update, new object[] { _progressPercentage, sBuf });
                // CPLD is READ only
                fBuf = "CPLD2";
                fileFormatStrings.Add(fBuf + rBuf);
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, sBuf);
                spDispatcher.BeginInvoke(update, new object[] { _progressPercentage, sBuf });
                

                //invoke the dispatcher and pass the percentage
                _progressPercentage += 10;
                // access hardware
                sBuf = "DAC Serial: ";
                bEEPROMreadOK = ThorDAQAdapters[MasterBoardIndex].DACcard.ReadEEPROM();
                rBuf = ThorDAQAdapters[MasterBoardIndex].DACcard.ProductionData; // good data or err message
                if (bEEPROMreadOK) sBuf += FormatSerStr(rBuf, false);
                else sBuf += rBuf;
                EEPromList.Add(sBuf);
                fBuf = "DAC  ";
                fileFormatStrings.Add(fBuf + rBuf);
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, sBuf);
                spDispatcher.BeginInvoke(update, new object[] { _progressPercentage, sBuf });

                //invoke the dispatcher and pass the percentage
                _progressPercentage += 10;
                // access hardware
                sBuf = "ABB1 Serial: ";
                bEEPROMreadOK = ThorDAQAdapters[MasterBoardIndex].ABB1box.ReadEEPROM();
                rBuf = ThorDAQAdapters[MasterBoardIndex].ABB1box.ProductionData; // good data or err message
                if (bEEPROMreadOK) sBuf += FormatSerStr(rBuf, false);
                else sBuf += rBuf;
                EEPromList.Add(sBuf);
                fBuf = "ABB1 ";
                fileFormatStrings.Add(fBuf + rBuf);
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, sBuf);
                spDispatcher.BeginInvoke(update, new object[] { _progressPercentage, sBuf });
                //invoke the dispatcher and pass the percentage
                _progressPercentage += 10;
                // access hardware
                sBuf = "ABB2 Serial: ";
                bEEPROMreadOK = ThorDAQAdapters[MasterBoardIndex].ABB2box.ReadEEPROM();
                rBuf = ThorDAQAdapters[MasterBoardIndex].ABB2box.ProductionData; // good data or err message
                if (bEEPROMreadOK) sBuf += FormatSerStr(rBuf, false);
                else sBuf += rBuf;
                EEPromList.Add(sBuf);
                fBuf = "ABB2 ";
                fileFormatStrings.Add(fBuf + rBuf);
                spDispatcher.BeginInvoke(update, new object[] { _progressPercentage, sBuf });
                //invoke the dispatcher and pass the percentage
                _progressPercentage += 10;
                // access hardware
                sBuf = "ABB3 Serial: ";
                bEEPROMreadOK = ThorDAQAdapters[MasterBoardIndex].ABB3box.ReadEEPROM();
                rBuf = ThorDAQAdapters[MasterBoardIndex].ABB3box.ProductionData; // good data or err message
                if (bEEPROMreadOK) sBuf += FormatSerStr(rBuf, false);
                else sBuf += rBuf;
                EEPromList.Add(sBuf);
                fBuf = "ABB3 ";
                fileFormatStrings.Add(fBuf + rBuf);
                spDispatcher.BeginInvoke(update, new object[] { _progressPercentage, sBuf });



            };
            // cause "DoWork" to run
            try
            {
                splashWkr.RunWorkerAsync();
            }
            catch (Exception ex)
            {
                Console.WriteLine("Exception Message: " + ex.Message);
            }
            finally
            {

            }

            // how do we complete background task?
            splashWkr.RunWorkerCompleted += delegate(object sender, RunWorkerCompletedEventArgs e)
            {

                _BGsplash.Close();
                _progressPercentage = 0;
                // App inherits from Application, and has a Window property called MainWindow
                // and a List<Window> property called OpenWindows.
                Application.Current.MainWindow.Activate();
                foreach( string sEEProm in EEPromList )
                    UpdateConsoleStatus(sEEProm);

                if (bCreateFile == true)
                {
                    File.WriteAllLines(fileSpec, fileFormatStrings);
                }
             
            };

            return EEPromList;
        }


        public void WriteToEEProms( List<String> EEpromStrings)
        {
            List<string> sResults = new List<string>();
            BackgroundWorker splashWkr = new BackgroundWorker();
            splashWkr.WorkerSupportsCancellation = true;

            _BGsplash = new BGsplashScreen(splashWkr);
            _BGsplash.DisplayText = "Please wait while writing EEPROM data...";
            _BGsplash.ShowInTaskbar = false;
            _BGsplash.Owner = Application.Current.MainWindow;
            _BGsplash.Show();

            //get dispatcher to update the contents that was created on the UI thread:
            System.Windows.Threading.Dispatcher spDispatcher = _BGsplash.Dispatcher;

            // Define what will be done when we start the background task
            splashWkr.DoWork += delegate(object sender, DoWorkEventArgs e)
            {
                //create a new delegate for updating our progress text
                UpdateProgressDelegate update = new UpdateProgressDelegate(UpdateBGProgressText);
                //invoke the dispatcher and pass the percentage
                // access hardware
                // discover which component the string is for...
                string sPROMcontents;
                bool bWriteSuccess;
                _progressPercentage = 0;
                foreach (string EEPROMstr in EEpromStrings)
                {
                    string eBuf = "Write ";
                    if (splashWkr.CancellationPending) break;

                    string sComponent = EEPROMstr.Substring(0, 5); // e.g. "CPLD2"
                    sPROMcontents = EEPROMstr.Substring(5, (EEPROMstr.Length - 5));
                    _progressPercentage += 10;
                    spDispatcher.BeginInvoke(update, new object[] { _progressPercentage, "Writing " + sPROMcontents });
                    switch (sComponent)
                    {
                        case "MB   ":
                            eBuf += sComponent;
                            bWriteSuccess = ThorDAQAdapters[MasterBoardIndex].MainBrd.WriteEEPROM(sPROMcontents);
                            if (bWriteSuccess) eBuf += "OK";
                            else eBuf += ThorDAQAdapters[MasterBoardIndex].MainBrd.ExceptionMessage;
                            sResults.Add(eBuf);
                            break;
                            
                        case "CPLD2":  // CPLD user code READ ONLY
                            //
                            break; 
                        case "TRG3P":
                            //TRG3Pcard
                            eBuf += sComponent;
                            bWriteSuccess = ThorDAQAdapters[MasterBoardIndex].TRG3Pcard.WriteEEPROM(sPROMcontents);
                            if (bWriteSuccess) eBuf += "OK";
                            else eBuf += ThorDAQAdapters[MasterBoardIndex].TRG3Pcard.ExceptionMessage;
                            sResults.Add(eBuf);
                            break;
                        case "ADC  ":
                            eBuf += sComponent;
                            bWriteSuccess = ThorDAQAdapters[MasterBoardIndex].ADCcard.WriteEEPROM(sPROMcontents);
                            if (bWriteSuccess) eBuf += "OK";
                            else eBuf += ThorDAQAdapters[MasterBoardIndex].ADCcard.ExceptionMessage;
                            sResults.Add(eBuf);
                            break;
                        case "DBB1 ":
                            eBuf += sComponent;
                            bWriteSuccess = ThorDAQAdapters[MasterBoardIndex].DBB1box.WriteEEPROM(sPROMcontents);
                            if (bWriteSuccess) eBuf += "OK";
                            else eBuf += ThorDAQAdapters[MasterBoardIndex].DBB1box.ExceptionMessage;
                            sResults.Add(eBuf);
                            break;
                        case "DAC  ":
                            eBuf += sComponent;
                            bWriteSuccess = ThorDAQAdapters[MasterBoardIndex].DACcard.WriteEEPROM(sPROMcontents);
                            if (bWriteSuccess) eBuf += "OK";
                            else eBuf += ThorDAQAdapters[MasterBoardIndex].DACcard.ExceptionMessage;
                            sResults.Add(eBuf);
                            break;
                        case "ABB1 ":
                            eBuf += sComponent;
                            bWriteSuccess = ThorDAQAdapters[MasterBoardIndex].ABB1box.WriteEEPROM(sPROMcontents);
                            if (bWriteSuccess) eBuf += "OK";
                            else eBuf += ThorDAQAdapters[MasterBoardIndex].ABB1box.ExceptionMessage;
                            sResults.Add(eBuf);
                            break;
                        case "ABB2 ":
                            eBuf += sComponent;
                            bWriteSuccess = ThorDAQAdapters[MasterBoardIndex].ABB2box.WriteEEPROM(sPROMcontents);
                            if (bWriteSuccess) eBuf += "OK";
                            else eBuf += ThorDAQAdapters[MasterBoardIndex].ABB2box.ExceptionMessage;
                            sResults.Add(eBuf);
                            break;
                        case "ABB3 ":
                            eBuf += sComponent;
                            bWriteSuccess = ThorDAQAdapters[MasterBoardIndex].ABB3box.WriteEEPROM(sPROMcontents);
                            if (bWriteSuccess) eBuf += "OK";
                            else eBuf += ThorDAQAdapters[MasterBoardIndex].ABB3box.ExceptionMessage;
                            sResults.Add(eBuf);
                            break;
                    }

                    System.Threading.Thread.Sleep(100);
                }
            };
            // cause "DoWork" to run
            try
            {
                splashWkr.RunWorkerAsync();
            }
            catch (Exception ex)
            {
                Console.WriteLine("Exception Message: " + ex.Message);
            }
            finally
            {

            }
            // how do we complete background task?
            splashWkr.RunWorkerCompleted += delegate(object sender, RunWorkerCompletedEventArgs e)
            {

                _BGsplash.Close();
                _progressPercentage = 0;
                // App inherits from Application, and has a Window property called MainWindow
                // and a List<Window> property called OpenWindows.
                Application.Current.MainWindow.Activate();
                foreach (string sEEProm in sResults)
                    UpdateConsoleStatus(sEEProm);
            };

            return;

        }



        public List<String> ReadAllEEProms()
        {
            string sBuf;
           // string retStr = "return String init";
            List<String> EEPromList = new List<string>();
            BackgroundWorker splashWkr = new BackgroundWorker();
            splashWkr.WorkerSupportsCancellation = true;
            _splash = new SplashScreen();
            _splash.DisplayText = "Please wait while discovering and reading EEPROM data...";
            _splash.ShowInTaskbar = false;
            _splash.Owner = Application.Current.MainWindow;
            _splash.Show();
            _splash.CancelSplashProgress += delegate(object sender, EventArgs e)
            {
                splashWkr.CancelAsync();
            };
            //get dispatcher to update the contents that was created on the UI thread:
            System.Windows.Threading.Dispatcher spDispatcher = _splash.Dispatcher;

            splashWkr.DoWork += delegate(object sender, DoWorkEventArgs e)
            {
                //create a new delegate for updating our progress text
                UpdateProgressDelegate update = new UpdateProgressDelegate(UpdateProgressText);
                //invoke the dispatcher and pass the percentage
                _progressPercentage = 10;
                string eBuf = ReadEEPromSerInfo(0, EEPromCMDs.MB_SER);
                sBuf = "MB Serial: " + FormatSerStr(eBuf, false);
                EEPromList.Add(sBuf);
                spDispatcher.BeginInvoke(update, new object[] { _progressPercentage, sBuf });
                System.Threading.Thread.Sleep(2000);

                // done
                e.Result = EEPromList;
            };

            // cause "DoWork" to run
            try
            {
                splashWkr.RunWorkerAsync(argument: EEPromList);
            }
            catch( Exception ex)
            {
                Console.WriteLine("Exception Message: " + ex.Message);
            }
            finally
            {

            }

            
            splashWkr.RunWorkerCompleted += delegate(object sender, RunWorkerCompletedEventArgs e)
            {
                
                _splash.Close();
                // App inherits from Application, and has a Window property called MainWindow
                // and a List<Window> property called OpenWindows.
                Application.Current.MainWindow.Activate();
                _progressPercentage = 0;
            };

            return EEPromList;
        }


        private void ProgramFPGAFlash(string fileName, bool bProduction)
        {
            System.IO.StreamReader file = new System.IO.StreamReader(fileName);
            string startBlock = string.Empty;
            string endBlock = string.Empty;
            string lastAddr = string.Empty;
            string startAddr_cmd;
            string endAddr_cmd;
            string data_sent;
            bool startBlockSet = false;
            string line;
            byte progStatus;
            Int32 fileSize_byte;
            IntPtr writeBuffer = IntPtr.Zero;
            IntPtr readBuffer = IntPtr.Zero;

            _progressPercentage = 0;

            if (writeBuffer == IntPtr.Zero)
            {
                Win32.AllocateBuffer(ref readBuffer, 1);
                Win32.AllocateBuffer(ref writeBuffer, 4);
            }
            BackgroundWorker splashWkr = new BackgroundWorker();
            splashWkr.WorkerSupportsCancellation = true;
            _splash = new SplashScreen();
            _splash.DisplayText = "Please wait while loading data";
            _splash.ShowInTaskbar = false;
            _splash.Owner = Application.Current.MainWindow;
            _splash.Show();
            _splash.CancelSplashProgress += delegate(object sender, EventArgs e)
            {
                splashWkr.CancelAsync();
            };
            //get dispatcher to update the contents that was created on the UI thread:
            System.Windows.Threading.Dispatcher spDispatcher = _splash.Dispatcher;
            splashWkr.RunWorkerAsync();

            // programming "golden" image or "default boot region" production image?
            splashWkr.DoWork += delegate(object sender, DoWorkEventArgs e)
            {
                WritePreFlashMemoryRegister(bProduction); // set or clear BPI FLASH MSB (golden or production address)
                while (!file.EndOfStream)
                {
                    line = file.ReadLine();
                    RecordType recordType = GetRecordType(line);
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
                        lastAddr = GetAddress(line);
                    }
                }
                endBlock = endBlock + lastAddr;
                fileSize_byte = (Int32.Parse(endBlock, System.Globalization.NumberStyles.HexNumber) / 2);
                endAddr_cmd = "0x45" + (fileSize_byte.ToString("X4"));
                startAddr_cmd = "0x5341" + startBlock;

                WriteFlashMemoryRegister(writeBuffer, RESET_ASYNC);
                System.Threading.Thread.Sleep(50);


                WriteFlashMemoryRegister(writeBuffer, startAddr_cmd);
                WriteFlashMemoryRegister(writeBuffer, endAddr_cmd);

                //create a new delegate for updating our progress text
                UpdateProgressDelegate update = new UpdateProgressDelegate(UpdateProgressText);
                //invoke the dispatcher and pass the percentage
                spDispatcher.BeginInvoke(update, new object[] { _progressPercentage, "Unlocking FLash" });
                WriteFlashMemoryRegister(writeBuffer, UNLOCK_CMD);
                do
                {
                    System.Threading.Thread.Sleep(50);
                    progStatus = ReadFlashStatusRegister(readBuffer);
                } while (!Convert.ToBoolean(progStatus & PROG_READY_STATUS));

                //create a new delegate for updating our progress text
                update = new UpdateProgressDelegate(UpdateProgressText);
                //invoke the dispatcher and pass the percentage
                spDispatcher.BeginInvoke(update, new object[] { _progressPercentage, "Erasing FLash" });
                WriteFlashMemoryRegister(writeBuffer, ERASE_CMD);// do earse
                int iDelay = 50;
                int iExpectedMillSecs = 40000;
                int iAccumlatedMS = 0;
                _progressPercentage = 0;
                do
                {
                    System.Threading.Thread.Sleep(iDelay);
                    progStatus = ReadFlashStatusRegister(readBuffer);
                    iAccumlatedMS += iDelay;
                    // update the progress bar based on EXPECTED time to completion, about 40 secs
                    _progressPercentage = (int)((float)iAccumlatedMS / (float)iExpectedMillSecs * 100);
                    if( _progressPercentage > 100)
                    {
                        _progressPercentage = 100;
                    }
                    //create a new delegate for updating our progress text
                    update = new UpdateProgressDelegate(UpdateProgressText);
                    //invoke the dispatcher and pass the percentage
                    spDispatcher.BeginInvoke(update, new object[] { _progressPercentage, "Erasing Flash..." });
                } while (!Convert.ToBoolean(progStatus & PROG_READY_STATUS));

                _progressPercentage = 0;
                //create a new delegate for updating our progress text
                update = new UpdateProgressDelegate(UpdateProgressText);
                //invoke the dispatcher and pass the percentage
                spDispatcher.BeginInvoke(update, new object[] { _progressPercentage, "Programming FLash" });
                WriteFlashMemoryRegister(writeBuffer, PROG_CMD);

                //Reset sequential read pointer
                file.DiscardBufferedData();
                file.BaseStream.Seek(0, System.IO.SeekOrigin.Begin);
                file.BaseStream.Position = 0;
                Int32 progress_count = 0;
                while (!file.EndOfStream)
                {
                    if (splashWkr.CancellationPending == true)
                    {
                        e.Cancel = true;
                        break;
                    }
                    progStatus = ReadFlashStatusRegister(readBuffer);
                    if (progStatus == 0x10)//!(Convert.ToBoolean(progStatus& FIFO_FULL_STATUS)))
                    {
                        for (int i = 0; i < 16; )
                        {
                            line = file.ReadLine();
                            if (GetRecordType(line) == RecordType.EOF)
                            {
                                e.Cancel = true;
                                break;
                            }
                            else if (GetRecordType(line) == RecordType.Data)
                            {
                                int length = GetDataLength(line);
                                for (int j = 0; j < length / 4; j++, i++)
                                {
                                    data_sent = GetData(line, 4 * j, 4);
                                    WriteFlashMemoryRegister(writeBuffer, data_sent);
                                }
                            }
                        }
                        progress_count += 32;
                        _progressPercentage = (int)((float)progress_count / (float)fileSize_byte * 100);
                        //create a new delegate for updating our progress text
                        update = new UpdateProgressDelegate(UpdateProgressText);
                        //invoke the dispatcher and pass the percentage
                        spDispatcher.BeginInvoke(update, new object[] { _progressPercentage, "Programming Flash" });
                    }
                }
            };
            splashWkr.RunWorkerCompleted += delegate(object sender, RunWorkerCompletedEventArgs e)
            {
                UpdateConsoleStatus("Updated Successfully - CYCLE POWER ON SYSTEM before running ThorDAQ!");
                _splash.Close();
                // App inherits from Application, and has a Window property called MainWindow
                // and a List<Window> property called OpenWindows.
                Application.Current.MainWindow.Activate();
                if (writeBuffer != IntPtr.Zero)
                {
                    Win32.FreeBuffer(writeBuffer);
                    Win32.FreeBuffer(readBuffer);
                }
                _progressPercentage = 0;
            };
        }

        private void WriteFlashMemoryRegister(IntPtr buffer, string commmand)
        {
            commmand = commmand.Replace("0x", string.Empty);
            for (int i = 0; i < 8 - commmand.Length; i++)
            {
                commmand = '0' + commmand;
            }
            for (int i = 0; i < 4; i++)
            {
                byte byteValue = byte.Parse(commmand.Substring(i * 2, 2), NumberStyles.HexNumber, CultureInfo.InvariantCulture);
                System.Runtime.InteropServices.Marshal.WriteByte(buffer, 3 - i, byteValue);
            }
            Win32.MemoryWrite(0, buffer, 3, (UInt64)0x0038, 0, 4);
        }

        private void WritePreFlashMemoryRegister(bool bProduction)
        {

            IntPtr buffer = IntPtr.Zero;
            Win32.AllocateBuffer(ref buffer, 1);

            if (bProduction == true)
            {
                for (int i = 0; i < 4; i++)
                {
                    System.Runtime.InteropServices.Marshal.WriteByte(buffer, 0, 0xC0);
                }
            }
            Win32.MemoryWrite(0, buffer, 3, (UInt64)0x0000, 0, 1);
            if (buffer != IntPtr.Zero)
            {
                Win32.FreeBuffer(buffer);
                Win32.FreeBuffer(buffer);
            }
        }

        private Byte ReadFlashStatusRegister(IntPtr buffer)
        {
            Win32.MemoryRead(0, buffer, 3, (UInt64)0x0004, 0, 1);
            Byte result = System.Runtime.InteropServices.Marshal.ReadByte(buffer);
            return result;
        }


        private RecordType GetRecordType(string line)
        {
            return (RecordType)new System.ComponentModel.Int32Converter().ConvertFromString(line.Substring(7, 2));
        }
        private String GetData(string line)
        {
            return line.Substring(9, 2 * GetDataLength(line));
        }

        private String GetData(string line, int startOffset, int length)
        {
            return line.Substring(9 + 2 * startOffset, 2 * length);
        }
        private String GetAddress(string line)
        {
            return line.Substring(3, 4);
        }

        private int GetDataLength(string line)
        {
            return int.Parse(line.Substring(1, 2), NumberStyles.HexNumber);
        }


        /// <summary>
        /// this is the method that the UpdateProgressDelegate will execute
        /// </summary>
        /// <param name="percentage"></param>
        public void UpdateBGProgressText(int percentage, string text)
        {
            //set our progress dialog text and value
            _BGsplash.ProgressText = string.Format("{0}%", percentage.ToString());
            _BGsplash.ProgressValue = percentage;
            _BGsplash.DisplayText = text;
        }


        /// <summary>
        /// this is the method that the UpdateProgressDelegate will execute
        /// </summary>
        /// <param name="percentage"></param>
        public void UpdateProgressText(int percentage, string text)
        {
            //set our progress dialog text and value
            _splash.ProgressText = string.Format("{0}%", percentage.ToString());
            _splash.ProgressValue = percentage;
            _splash.DisplayText = text;
        }
    }
}
