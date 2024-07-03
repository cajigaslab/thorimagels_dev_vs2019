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
        #region Fields

        SplashScreen _APIcpldSplash;
        THORDAQ_STATUS workerThreadStatus = THORDAQ_STATUS.STATUS_SUCCESSFUL;
        #endregion Fields

        #region Methods

        /// <summary>
        /// this is the method that the UpdateProgressDelegate will execute
        /// </summary>
        /// <param name="percentage"></param>
        public void API_UpdateCPLDProgressText(int percentage, string text)
        {
            //set our progress dialog text and value
            _APIcpldSplash.ProgressText = string.Format("{0}%", percentage.ToString());
            _APIcpldSplash.ProgressValue = percentage;
            _APIcpldSplash.DisplayText = text;
        }

        // See "Using User Flash Memory" (UFM) and Hardened Control Functions in MachX02, TN1246_2.4, pg 55, summary of commands
        // D. Zimmerman, 25-Mar-2021
        // Xilinx AIX I2C core master controller implementation

        void API_ProgramLatticeCPLD(string fileName, uint MasterMUXChan, uint SlaveMUXChan, uint TargetSlaveAddr)
        {
            XilinxI2CXfer I2Cxfer = new XilinxI2CXfer();

            System.IO.StreamReader file = new System.IO.StreamReader(fileName);
            BackgroundWorker splashWkr = new BackgroundWorker();
            splashWkr.WorkerSupportsCancellation = true;
            _APIcpldSplash = new SplashScreen();
            _APIcpldSplash.DisplayText = "Please wait while loading data";
            _APIcpldSplash.ShowInTaskbar = false;
            _APIcpldSplash.Owner = Application.Current.MainWindow;
            _APIcpldSplash.Show();
            _APIcpldSplash.CancelSplashProgress += delegate(object sender, EventArgs e)
            {
                splashWkr.CancelAsync();
            };
            //get dispatcher to update the contents that was created on the UI thread:
            System.Windows.Threading.Dispatcher spDispatcher = _APIcpldSplash.Dispatcher;
            splashWkr.RunWorkerAsync();
            splashWkr.DoWork += delegate(object sender, DoWorkEventArgs e)
            {
                string line;
                ulong lineCnt = 0;
                bool validLine = false;
                // get the total lines of data
                while (!file.EndOfStream)
                {
                    line = file.ReadLine();
                    if (validLine == true && line.Length != 128)
                    {
                        break;
                    }
                    if (line.Length == 128)
                    {
                        validLine = true;
                        lineCnt++;
                    }
                }

                // Set up the variables needed for the API
                THORDAQ_STATUS status = THORDAQ_STATUS.STATUS_SUCCESSFUL;
                uint TotalDataPlusOpcodesBytesTransfered;
                bool b_I2CReadDir = false;
                uint OpcodeByteLen, DATAtransferBufferLen;
                const int PageReadWriteLen = 16;
                byte[] OpcodeBytes = new byte[4]; // OpCode count per Lattice MACHx02 command
                byte[] DataBytes = new byte[PageReadWriteLen]; //  DATA count per Lattice MACHx02 command
                IntPtr UnManagedOpCodeBuffer = Marshal.AllocHGlobal(OpcodeBytes.Length);
                IntPtr UnManagedDataBuffer = Marshal.AllocHGlobal(DataBytes.Length);

                // typically CPLD needs 4-byte OpCodes
                OpcodeByteLen = (uint)OpcodeBytes.Length;
                // typical program DATA length is 16
                DATAtransferBufferLen = (uint)DataBytes.Length;


                //create a new delegate for updating our progress text
                UpdateProgressDelegate update = new UpdateProgressDelegate(API_UpdateCPLDProgressText);
                //invoke the dispatcher and pass the percentage
                spDispatcher.BeginInvoke(update, new object[] { _progressPercentage, "Setup User Flash Memory (Enable UFM Access transparent Mode)" });


                // 0x74 == ISC_ENABLE_X, Enable transparent UFM access
                OpcodeBytes = new byte[] { 0x74, 0x08, 0x00, 0x00 };
                OpcodeByteLen = 4;
                DATAtransferBufferLen = 0; // no data (only opcodes)
                b_I2CReadDir = false;      // WRITE COMMAND/OPCODES to CPLD
                status = I2Cxfer.I2CTransfer(MasterMUXChan, SlaveMUXChan, TargetSlaveAddr, b_I2CReadDir, PageReadWriteLen, OpcodeBytes, OpcodeByteLen, DataBytes, DATAtransferBufferLen, out TotalDataPlusOpcodesBytesTransfered);
                if( status != THORDAQ_STATUS.STATUS_SUCCESSFUL )
                {
                    spDispatcher.BeginInvoke(update, new object[] { _progressPercentage, "ERROR on ENABLE_X" });
                    workerThreadStatus = THORDAQ_STATUS.STATUS_INCOMPLETE;
                    System.Threading.Thread.Sleep(1500);
                    goto ExitEarly;
                }
                // now check the BUSY status
                OpcodeBytes = new byte[] { 0xF0, 0x00, 0x00, 0x00 };
                OpcodeByteLen = 4;
                b_I2CReadDir = true; // READ from CPLD
                DATAtransferBufferLen = 1; // expect one data byte

                long iterations = 256;
                do
                {
                    status = I2Cxfer.I2CTransfer(MasterMUXChan, SlaveMUXChan, TargetSlaveAddr, b_I2CReadDir, PageReadWriteLen, OpcodeBytes, OpcodeByteLen, DataBytes, DATAtransferBufferLen, out TotalDataPlusOpcodesBytesTransfered);
                    if(status != THORDAQ_STATUS.STATUS_SUCCESSFUL)
                    {
                        workerThreadStatus = status;
                        goto ExitEarly;
                    }
                    else if( --iterations == 0)
                    {
                        workerThreadStatus = status;
                        goto ExitEarly;
                    }

                } while (( DataBytes[0] != 0) && --iterations > 0);

                // 0x3C == LSC_READ_STATUS - read the 4-byte Configuration Status Register
                OpcodeBytes = new byte[] { 0x3C, 0x00, 0x00, 0x00 };
                OpcodeByteLen = 4;
                b_I2CReadDir = true; // READ from CPLD
                DATAtransferBufferLen = 4; // expect 4 bytes from CPLD
                status = I2Cxfer.I2CTransfer(MasterMUXChan, SlaveMUXChan, TargetSlaveAddr, b_I2CReadDir, PageReadWriteLen, OpcodeBytes, OpcodeByteLen, DataBytes, DATAtransferBufferLen, out TotalDataPlusOpcodesBytesTransfered);
                if (status != THORDAQ_STATUS.STATUS_SUCCESSFUL)
                {
                    workerThreadStatus = status;
                    goto ExitEarly;
                }

                // EEEEEEEEEEEEEEEEEEEEEEEE   Erasing the CPLD takes some real time EEEEEEEEEEEEEEEEEEE
                // 0x0E == ISC_ERASE,  0x04 flag is CONFIGURATION FLASH
                OpcodeBytes = new byte[] { 0x0E, 0x04, 0x00, 0x00 };
                OpcodeByteLen = 4;
                DATAtransferBufferLen = 0; // no data (only opcodes)
                b_I2CReadDir = false; // WRITE cmd to CPLD
                status = I2Cxfer.I2CTransfer(MasterMUXChan, SlaveMUXChan, TargetSlaveAddr, b_I2CReadDir, PageReadWriteLen, OpcodeBytes, OpcodeByteLen, DataBytes, DATAtransferBufferLen, out TotalDataPlusOpcodesBytesTransfered);
                if (status != THORDAQ_STATUS.STATUS_SUCCESSFUL)
                {
                    workerThreadStatus = status;
                    goto ExitEarly;
                }

                update = new UpdateProgressDelegate(API_UpdateCPLDProgressText);
                //invoke the dispatcher and pass the percentage
                spDispatcher.BeginInvoke(update, new object[] { _progressPercentage, "Erasing Flash" });


                // Poll for ERASE command complete
                // now check the BUSY status
                OpcodeBytes = new byte[] { 0xF0, 0x00, 0x00, 0x00 };
                OpcodeByteLen = 4;
                b_I2CReadDir = true; // READ from CPLD
                DATAtransferBufferLen = 1; // expect one data byte

                iterations = 100; // about 1.5 seconds; typical erase time under 700 ms
                do
                {
                    status = I2Cxfer.I2CTransfer(MasterMUXChan, SlaveMUXChan, TargetSlaveAddr, b_I2CReadDir, PageReadWriteLen, OpcodeBytes, OpcodeByteLen, DataBytes, DATAtransferBufferLen, out TotalDataPlusOpcodesBytesTransfered);
                    if (status != THORDAQ_STATUS.STATUS_SUCCESSFUL)
                    {
                        workerThreadStatus = status;
                        goto ExitEarly;
                    }
                    else if (--iterations == 0)
                    {
                        workerThreadStatus = status;
                        goto ExitEarly;
                    }
                    // ms pause between polling
                    System.Threading.Thread.Sleep(15);

                } while ((DataBytes[0] != 0) && --iterations > 0);

                // ERASE Complete

                // 0x3C == LSC_READ_STATUS - read the 4-byte Configuration Status Register
                OpcodeBytes = new byte[] { 0x3C, 0x00, 0x00, 0x00 };
                OpcodeByteLen = 4;
                b_I2CReadDir = true; // READ from CPLD
                DATAtransferBufferLen = 4; // expect 4 bytes from CPLD
                status = I2Cxfer.I2CTransfer(MasterMUXChan, SlaveMUXChan, TargetSlaveAddr, b_I2CReadDir, PageReadWriteLen, OpcodeBytes, OpcodeByteLen, DataBytes, DATAtransferBufferLen, out TotalDataPlusOpcodesBytesTransfered);
                if (status != THORDAQ_STATUS.STATUS_SUCCESSFUL)
                {
                    workerThreadStatus = status;
                    goto ExitEarly;
                }
                // ms pause between polling
                System.Threading.Thread.Sleep(15);

                // PPPPPPPPPPPPPPPPPPPPPPP   Program FLASH Contents (from file) PPPPPPPPPPPPPPPPPP

                // 0x46 == LSC_INIT_ADDRESS (to 0x0)
                OpcodeBytes = new byte[] { 0x46, 0x00, 0x00, 0x00 };
                OpcodeByteLen = 4;
                DATAtransferBufferLen = 0; // no data (only opcodes)
                b_I2CReadDir = false; // WRITE cmd to CPLD
                status = I2Cxfer.I2CTransfer(MasterMUXChan, SlaveMUXChan, TargetSlaveAddr, b_I2CReadDir, PageReadWriteLen, OpcodeBytes, OpcodeByteLen, DataBytes, DATAtransferBufferLen, out TotalDataPlusOpcodesBytesTransfered);
                if (status != THORDAQ_STATUS.STATUS_SUCCESSFUL)
                {
                    workerThreadStatus = status;
                    goto ExitEarly;
                }

                // ms pause between polling
                System.Threading.Thread.Sleep(15);

                //create a new delegate for updating our progress text
                update = new UpdateProgressDelegate(API_UpdateCPLDProgressText);
                //invoke the dispatcher and pass the percentage
                spDispatcher.BeginInvoke(update, new object[] { _progressPercentage, "Programming Flash" });

                //Reset sequential read pointer
                file.DiscardBufferedData();
                file.BaseStream.Seek(0, System.IO.SeekOrigin.Begin);
                file.BaseStream.Position = 0;
                Int32 progress_count = 0;
                
                // LLLLLLLLLLLLLLLLLLLLLL  Loop through all CPLD program 16-byte records ("lines") LLLLLLLLLLLLLLLLLLL
                while (!file.EndOfStream && ((float)progress_count < (float)lineCnt))
                {
                    if (splashWkr.CancellationPending == true)
                    {
                        e.Cancel = true;
                        break;
                    }
                    // read line
                    line = file.ReadLine();
                    if (line.Length != 128)
                    {
                        continue;
                    }
                    line = line.Replace("0x", string.Empty);

                    // load 16-byte page from Disk file...
                    for (int j = 0; j < 4; j++)
                    {
                        for (int i = 0; i < 4; i++)
                        {
                            //byte byteValue = byte.Parse(commmand.Substring(i * 2, 2), NumberStyles.Bin, CultureInfo.InvariantCulture);
                            string tempStr = line.Substring(j * 32 + (3 - i) * 8, 8);
                            byte byteValue = Convert.ToByte(tempStr, 2);
                       
                            DataBytes[j * 4 + (3 - i)] = byteValue; //  Lattice program DATA byte
                        }
                    }
                    // Write 16-byte PAGES...  4 byte Cmd-Opcode plus 16 bytes page data
                    // 0x70 == LSC_PROG_INCR_NV
                    OpcodeBytes = new byte[] { 0x70, 0x00, 0x00, 0x01 };
                    OpcodeByteLen = 4;
                    DATAtransferBufferLen = PageReadWriteLen; // CPLD program DATA 
                    b_I2CReadDir = false; // WRITE cmd to CPLD
                    status = I2Cxfer.I2CTransfer(MasterMUXChan, SlaveMUXChan, TargetSlaveAddr, b_I2CReadDir, PageReadWriteLen, OpcodeBytes, OpcodeByteLen, DataBytes, DATAtransferBufferLen, out TotalDataPlusOpcodesBytesTransfered);
                    if (status != THORDAQ_STATUS.STATUS_SUCCESSFUL)
                    {
                        workerThreadStatus = status;
                        goto ExitEarly;
                    }

                    // delay between polling (wait 200 microsecs, per Lattice flow chart)
                    System.Threading.Thread.Sleep(1);

                    // Poll for write completion with BUSY status
                    OpcodeBytes = new byte[] { 0xF0, 0x00, 0x00, 0x00 };
                    OpcodeByteLen = 4;
                    b_I2CReadDir = true; // READ from CPLD
                    DATAtransferBufferLen = 1; // expect one data byte
                    long lTimeout = 256;
                    do {
                        status = I2Cxfer.I2CTransfer(MasterMUXChan, SlaveMUXChan, TargetSlaveAddr, b_I2CReadDir, PageReadWriteLen, OpcodeBytes, OpcodeByteLen, DataBytes, DATAtransferBufferLen, out TotalDataPlusOpcodesBytesTransfered);
                        if (status != THORDAQ_STATUS.STATUS_SUCCESSFUL)
                        {
//                            UpdateConsoleStatus("error polling 16-byte flash segment: " + status.ToString("X"));
                            workerThreadStatus = status;
                            goto ExitEarly;
                        }
                    } while (DataBytes[0] != 0 && --lTimeout > 0 );

                    progress_count++;
                    _progressPercentage = (int)((float)progress_count / (float)lineCnt * 100);
                    //create a new delegate for updating our progress text
                    update = new UpdateProgressDelegate(API_UpdateCPLDProgressText);

                    //invoke the dispatcher and pass the percentage
                    spDispatcher.BeginInvoke(update, new object[] { _progressPercentage, "Programming Flash" });
                } // end of Loop writing FILE lines

                // File contents loaded to CPLD ...
                // 0x5E == LSC_PROGRAM_DONE
                OpcodeBytes = new byte[] { 0x5E, 0x00, 0x00, 0x00 };
                OpcodeByteLen = 4;
                DATAtransferBufferLen = 0; // no data (only opcodes)
                b_I2CReadDir = false; // WRITE cmd to CPLD
                status = I2Cxfer.I2CTransfer(MasterMUXChan, SlaveMUXChan, TargetSlaveAddr, b_I2CReadDir, PageReadWriteLen, OpcodeBytes, OpcodeByteLen, DataBytes, DATAtransferBufferLen, out TotalDataPlusOpcodesBytesTransfered);
                if (status != THORDAQ_STATUS.STATUS_SUCCESSFUL)
                {
                    workerThreadStatus = status;
                    goto ExitEarly;
                }
                // ms pause between polling
                System.Threading.Thread.Sleep(100);

                // now check the BUSY status
                OpcodeBytes = new byte[] { 0xF0, 0x00, 0x00, 0x00 };
                OpcodeByteLen = 4;
                b_I2CReadDir = true; // READ from CPLD
                DATAtransferBufferLen = 1; // expect one data byte
                status = I2Cxfer.I2CTransfer(MasterMUXChan, SlaveMUXChan, TargetSlaveAddr, b_I2CReadDir, PageReadWriteLen, OpcodeBytes, OpcodeByteLen, DataBytes, DATAtransferBufferLen, out TotalDataPlusOpcodesBytesTransfered);
                if (status != THORDAQ_STATUS.STATUS_SUCCESSFUL)
                {
                    workerThreadStatus = status;
                    goto ExitEarly;
                }

                // Disable programming interface
                // 0x26 ==  ISE_DISABLE
                OpcodeBytes = new byte[] { 0x26, 0x00, 0x00, 0x00 };
                OpcodeByteLen = 4;
                DATAtransferBufferLen = 0; // no data (only opcodes)
                b_I2CReadDir = false; // WRITE cmd to CPLD
                status = I2Cxfer.I2CTransfer(MasterMUXChan, SlaveMUXChan, TargetSlaveAddr, b_I2CReadDir, PageReadWriteLen, OpcodeBytes, OpcodeByteLen, DataBytes, DATAtransferBufferLen, out TotalDataPlusOpcodesBytesTransfered);
                if (status != THORDAQ_STATUS.STATUS_SUCCESSFUL)
                {
                    workerThreadStatus = status;
                    goto ExitEarly;
                }

                System.Threading.Thread.Sleep(100);

                // Wakeup device with NOOP
                // 0xFF ==  ISE_NOOP
                OpcodeBytes = new byte[] { 0xFF, 0xFF, 0xFF, 0xFF };
                OpcodeByteLen = 4;
                DATAtransferBufferLen = 0; // no data (only opcodes)
                b_I2CReadDir = false; // WRITE cmd to CPLD
                status = I2Cxfer.I2CTransfer(MasterMUXChan, SlaveMUXChan, TargetSlaveAddr, b_I2CReadDir, PageReadWriteLen, OpcodeBytes, OpcodeByteLen, DataBytes, DATAtransferBufferLen, out TotalDataPlusOpcodesBytesTransfered);
                if (status != THORDAQ_STATUS.STATUS_SUCCESSFUL)
                {
                    workerThreadStatus = status;
                    goto ExitEarly;
                }

                // Finally "refresh", force the MACHx02 to reconfigure
                // 0x79 ==  ISE_NOOP
                OpcodeBytes = new byte[] { 0x79, 0x00, 0x00 };
                OpcodeByteLen = 3;
                DATAtransferBufferLen = 0; // no data (only opcodes)
                b_I2CReadDir = false; // WRITE cmd to CPLD
                status = I2Cxfer.I2CTransfer(MasterMUXChan, SlaveMUXChan, TargetSlaveAddr, b_I2CReadDir, PageReadWriteLen, OpcodeBytes, OpcodeByteLen, DataBytes, DATAtransferBufferLen, out TotalDataPlusOpcodesBytesTransfered);
                if (status != THORDAQ_STATUS.STATUS_SUCCESSFUL)
                {
                    workerThreadStatus = status;
                    goto ExitEarly;
                }
                ExitEarly:
                //create a new delegate for updating our progress text
                update = new UpdateProgressDelegate(API_UpdateCPLDProgressText);
                //invoke the dispatcher and pass the percentage
                string sFinalStatus = "SUCCESS";
                if (workerThreadStatus != THORDAQ_STATUS.STATUS_SUCCESSFUL)
                    sFinalStatus = "ERROR";

                spDispatcher.BeginInvoke(update, new object[] { _progressPercentage, "Update " + sFinalStatus });
                System.Threading.Thread.Sleep(2000);

            };
            splashWkr.RunWorkerCompleted += delegate(object sender, RunWorkerCompletedEventArgs e)
            {
                _APIcpldSplash.Close();
                // App inherits from Application, and has a Window property called MainWindow
                // and a List<Window> property called OpenWindows.
                Application.Current.MainWindow.Activate();
                _progressPercentage = 0;
            };
        }

        #endregion Methods
    }
}