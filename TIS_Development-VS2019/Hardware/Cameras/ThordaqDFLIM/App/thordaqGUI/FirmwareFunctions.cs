namespace thordaqGUI
{
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

    using NWLogic.DeviceLib;

    public partial class MainWindow : Window
    {
        const string UNLOCK_CMD = "0x556E6C6B";
        const string ERASE_CMD = "0x45726173";
        const string PROG_CMD = "0x50726F67";
        const string RESET_ASYNC = "0x656E6C5A";
        const string FPGA_FLASH_PROG_CMD = "WriteMem 3 0x0038 4 ";
        const string FPGA_PROG_STATUS_CMD = "ReadMem 3 0x02 1";
        const Byte PROG_READY_STATUS = 0x40;
        const Byte FIFO_FULL_STATUS = 0x20;
        SplashScreen _splash;
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

        private void ProgramFPGAFlash(string fileName)
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
            splashWkr.DoWork += delegate(object sender, DoWorkEventArgs e)
            {
                WritePreFlashMemoryRegister(); //set bar3 0xC0
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
                do
                {
                    System.Threading.Thread.Sleep(50);
                    progStatus = ReadFlashStatusRegister(readBuffer);
                } while (!Convert.ToBoolean(progStatus & PROG_READY_STATUS));

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
                        spDispatcher.BeginInvoke(update, new object[] { _progressPercentage, "Programming FLash" });
                    }
                }
            };
            splashWkr.RunWorkerCompleted += delegate(object sender, RunWorkerCompletedEventArgs e)
            {
                UpdateConsoleStatus("Update Successfully");
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

        private void WritePreFlashMemoryRegister()
        {

            IntPtr buffer = IntPtr.Zero;
            Win32.AllocateBuffer(ref buffer, 1);

            for (int i = 0; i < 4; i++)
            {
                System.Runtime.InteropServices.Marshal.WriteByte(buffer, 0, 0xC0);
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
        public void UpdateProgressText(int percentage, string text)
        {
            //set our progress dialog text and value
            _splash.ProgressText = string.Format("{0}%", percentage.ToString());
            _splash.ProgressValue = percentage;
            _splash.DisplayText = text;
        }
    }
}
