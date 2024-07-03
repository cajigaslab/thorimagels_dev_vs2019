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
        
        SplashScreen _cpldSplash;
        /// <summary>
        /// this is the method that the UpdateProgressDelegate will execute
        /// </summary>
        /// <param name="percentage"></param>
        public void UpdateCPLDProgressText(int percentage, string text)
        {
            //set our progress dialog text and value
            _cpldSplash.ProgressText = string.Format("{0}%", percentage.ToString());
            _cpldSplash.ProgressValue = percentage;
            _cpldSplash.DisplayText = text;
        }

        private void WriteCPLDRegister(IntPtr buffer, ulong bufferLen,UInt64 address, string commmand)
        {
            IntPtr buffer1 = IntPtr.Zero;
            Win32.AllocateBuffer(ref buffer1, 1);
            const Byte I2C_BUSY_STATUS = 0x01;
            Byte busyStatus;
            do
            {
                System.Threading.Thread.Sleep(5);
                busyStatus = ReadCPLDBusyRegister(buffer1);
            } while (Convert.ToBoolean(busyStatus & I2C_BUSY_STATUS)); // busy Keep reading

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
            Win32.MemoryWrite(0, buffer, 3, address, 0, bufferLen);
            Win32.FreeBuffer(buffer1);
        }

        private Byte ReadCPLDStatusRegister(IntPtr buffer)
        {
            Win32.MemoryRead(0, buffer, 3, (UInt64)0x02c0, 0, 1);
            Byte result = System.Runtime.InteropServices.Marshal.ReadByte(buffer);
            return result;
        }

        private Byte ReadCPLDBusyRegister(IntPtr buffer)
        {
            Win32.MemoryRead(0, buffer, 3, (UInt64)0x02c1, 0, 1);
            Byte result = System.Runtime.InteropServices.Marshal.ReadByte(buffer);
            return result;
        }

        void ProgramCPLD(string fileName)
        {
            System.IO.StreamReader file = new System.IO.StreamReader(fileName);
            IntPtr buffer1 = IntPtr.Zero;
            IntPtr buffer2 = IntPtr.Zero;
            IntPtr buffer3 = IntPtr.Zero;
            IntPtr buffer4 = IntPtr.Zero;
            Win32.AllocateBuffer(ref buffer1, 1);
            Win32.AllocateBuffer(ref buffer2, 2);
            Win32.AllocateBuffer(ref buffer3, 3);
            Win32.AllocateBuffer(ref buffer4, 4);
            const Byte CPLD_BUSY_STATUS = 0x80;
            const Byte CPLD_FAIL_STATUS = 0x20;
            const Byte CPLD_DONE_STATUS = 0x01;
            Byte busyStatus;
            BackgroundWorker splashWkr = new BackgroundWorker();
            splashWkr.WorkerSupportsCancellation = true;
            _cpldSplash = new SplashScreen();
            _cpldSplash.DisplayText = "Please wait while loading data";
            _cpldSplash.ShowInTaskbar = false;
            _cpldSplash.Owner = Application.Current.MainWindow;
            _cpldSplash.Show();
            _cpldSplash.CancelSplashProgress += delegate(object sender, EventArgs e)
            {
                splashWkr.CancelAsync();
            };
            //get dispatcher to update the contents that was created on the UI thread:
            System.Windows.Threading.Dispatcher spDispatcher = _cpldSplash.Dispatcher;
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

                // address PCA9548A, channel 3, activate channel to CPLD
                WriteCPLDRegister(buffer1, 1, 0x02c8, "0x00");
                WriteCPLDRegister(buffer4, 4, 0x02c0, "0xFFFFFF08");
                WriteCPLDRegister(buffer2, 2, 0x02c4, "0x0271"); //command used to be 0x0277 but due to a change on the main board it has been updated to 0x0271
                WriteCPLDRegister(buffer1, 1, 0x02c8, "0x02");
                WriteCPLDRegister(buffer1, 1, 0x02c8, "0x03");

                
                // Enable Transparent Configuration (0x74)
                // write 3 bytes, 0x080000, to 0x74 of CPLD
                WriteCPLDRegister(buffer4, 4, 0x02c0, "0x74000008");
                WriteCPLDRegister(buffer4, 4, 0x02c4, "0x00000540");
                WriteCPLDRegister(buffer1, 1, 0x02c8, "0x02");
                WriteCPLDRegister(buffer1, 1, 0x02c8, "0x03");
                
                //create a new delegate for updating our progress text
                UpdateProgressDelegate update = new UpdateProgressDelegate(UpdateCPLDProgressText);
                //invoke the dispatcher and pass the percentage
                spDispatcher.BeginInvoke(update, new object[] { _progressPercentage, "Verify FLash" });

                //write f0, then read busy
                do
                {
                    WriteCPLDRegister(buffer4, 4, 0x02c0, "0xf0000000");
                    WriteCPLDRegister(buffer4, 4, 0x02c4, "0x000145c0");
                    WriteCPLDRegister(buffer1, 1, 0x02c8, "0x02");
                    WriteCPLDRegister(buffer1, 1, 0x02c8, "0x03");
                    System.Threading.Thread.Sleep(25);
                    busyStatus = ReadCPLDStatusRegister(buffer1);
                } while (Convert.ToBoolean(busyStatus & CPLD_BUSY_STATUS)); // busy Keep reading


                //create a new delegate for updating our progress text
                update = new UpdateProgressDelegate(UpdateCPLDProgressText);
                //invoke the dispatcher and pass the percentage
                spDispatcher.BeginInvoke(update, new object[] { _progressPercentage, "Enable transparent configuration" });
                // write 3c verify, then read 1 byte
                do
                {
                    WriteCPLDRegister(buffer4, 4, 0x02c0, "0x3c000000");
                    WriteCPLDRegister(buffer4, 4, 0x02c4, "0x0003a5c0");
                    WriteCPLDRegister(buffer1, 1, 0x02c8, "0x02");
                    WriteCPLDRegister(buffer1, 1, 0x02c8, "0x03");
                    System.Threading.Thread.Sleep(25);
                    busyStatus = ReadCPLDStatusRegister(buffer1);
                } while (Convert.ToBoolean(busyStatus & CPLD_FAIL_STATUS));

                // Erase Configuration Flash Sector (0x0E)
                // write 3 bytes, 0x040000, to 0x0E of CPLD
                WriteCPLDRegister(buffer4, 4, 0x02c0, "0x0e000004");
                WriteCPLDRegister(buffer4, 4, 0x02c4, "0x00000540");
                WriteCPLDRegister(buffer1, 1, 0x02c8, "0x02");
                WriteCPLDRegister(buffer1, 1, 0x02c8, "0x03");

                //write f0, then read busy
                do
                {
                    WriteCPLDRegister(buffer4, 4, 0x02c0, "0xf0000000");
                    WriteCPLDRegister(buffer4, 4, 0x02c4, "0x000145c0");
                    WriteCPLDRegister(buffer1, 1, 0x02c8, "0x02");
                    WriteCPLDRegister(buffer1, 1, 0x02c8, "0x03");
                    System.Threading.Thread.Sleep(25);
                    busyStatus = ReadCPLDStatusRegister(buffer1);
                } while (Convert.ToBoolean(busyStatus & CPLD_BUSY_STATUS)); // busy Keep reading


                //create a new delegate for updating our progress text
                update = new UpdateProgressDelegate(UpdateCPLDProgressText);
                //invoke the dispatcher and pass the percentage
                spDispatcher.BeginInvoke(update, new object[] { _progressPercentage, "Erasing FLash" });
                // write 3c verify, then read 1 byte
                do
                {
                    WriteCPLDRegister(buffer4, 4, 0x02c0, "0x3c000000");
                    WriteCPLDRegister(buffer4, 4, 0x02c4, "0x0003a5c0");
                    WriteCPLDRegister(buffer1, 1, 0x02c8, "0x02");
                    WriteCPLDRegister(buffer1, 1, 0x02c8, "0x03");
                    System.Threading.Thread.Sleep(25);
                    busyStatus = ReadCPLDStatusRegister(buffer1);
                } while (Convert.ToBoolean(busyStatus & CPLD_FAIL_STATUS));

                // Set Address to 0 (0x46)
                // write 3 bytes, 0x000000, to 0x46 of CPLD
                WriteCPLDRegister(buffer4, 4, 0x02c0, "0x46000000");
                WriteCPLDRegister(buffer4, 4, 0x02c4, "0x00000540");
                WriteCPLDRegister(buffer1, 1, 0x02c8, "0x02");
                WriteCPLDRegister(buffer1, 1, 0x02c8, "0x03");

                //create a new delegate for updating our progress text
                update = new UpdateProgressDelegate(UpdateCPLDProgressText);
                //invoke the dispatcher and pass the percentage
                spDispatcher.BeginInvoke(update, new object[] { _progressPercentage, "Programming FLash" });

                //Reset sequential read pointer
                file.DiscardBufferedData();
                file.BaseStream.Seek(0, System.IO.SeekOrigin.Begin);
                file.BaseStream.Position = 0;
                Int32 progress_count = 0;
                while (!file.EndOfStream  && ((float)progress_count < (float)lineCnt))
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

                    do
                    {
                        System.Threading.Thread.Sleep(5);
                        busyStatus = ReadCPLDBusyRegister(buffer1);
                    } while (Convert.ToBoolean(busyStatus & (0x01))); // busy Keep reading
                    for (int j = 0; j < 4; j++)
                    {
                        for (int i = 0; i < 4; i++)
                        {
                            //byte byteValue = byte.Parse(commmand.Substring(i * 2, 2), NumberStyles.Bin, CultureInfo.InvariantCulture);
                            string tempStr = line.Substring(j * 32 + (3 - i) * 8, 8);
                            byte byteValue = Convert.ToByte(tempStr, 2);
                            System.Runtime.InteropServices.Marshal.WriteByte(buffer4, 3 - i, byteValue);
                        }
                        UInt64 address = (UInt64)(0x02d0 + 0x0004 * j);
                        Win32.MemoryWrite(0, buffer4, 3, address, 0, 4);
                    }


                    // Write 1 Page Config Data (0x70)
                    // write 3 bytes, 0x010000, to 0x70 of CPLD. Then write 16 bytes extra data
                    WriteCPLDRegister(buffer4, 4, 0x02c0, "0x70010000");
                    WriteCPLDRegister(buffer4, 4, 0x02c4, "0x00001540");
                    WriteCPLDRegister(buffer1, 1, 0x02c8, "0x02");
                    WriteCPLDRegister(buffer1, 1, 0x02c8, "0x03");
                    // Wait for !BUSY (0xF0)
                    // write 3 bytes, 0x000000, to 0xF0 of CPLD. Then read 1 byte
                    do
                    {
                        WriteCPLDRegister(buffer4, 4, 0x02c0, "0xf0000000");
                        WriteCPLDRegister(buffer4, 4, 0x02c4, "0x000145c0");
                        WriteCPLDRegister(buffer1, 1, 0x02c8, "0x02");
                        WriteCPLDRegister(buffer1, 1, 0x02c8, "0x03");
                        System.Threading.Thread.Sleep(25);
                        busyStatus = ReadCPLDStatusRegister(buffer1);
                    } while (Convert.ToBoolean(busyStatus & CPLD_BUSY_STATUS)); // busy Keep reading

                   
                    // Then Verify !FAIL (0x3C)
                    // write 3 bytes, 0x000000, to 0x3C of CPLD. Then read 4 bytes. Output byte no.3
                    do
                    {
                        WriteCPLDRegister(buffer4, 4, 0x02c0, "0x3c000000");
                        WriteCPLDRegister(buffer4, 4, 0x02c4, "0x0003a5c0");
                        WriteCPLDRegister(buffer1, 1, 0x02c8, "0x02");
                        WriteCPLDRegister(buffer1, 1, 0x02c8, "0x03");
                        System.Threading.Thread.Sleep(25);
                        busyStatus = ReadCPLDStatusRegister(buffer1);
                    } while (Convert.ToBoolean(busyStatus & CPLD_FAIL_STATUS));

                    progress_count++;
                    _progressPercentage = (int)((float)progress_count / (float)lineCnt * 100);
                    //create a new delegate for updating our progress text
                    update = new UpdateProgressDelegate(UpdateCPLDProgressText);

                    //invoke the dispatcher and pass the percentage
                    spDispatcher.BeginInvoke(update, new object[] { _progressPercentage, "Programming FLash" });
                    
                }
                // Set Flash DONE bit (0x5E)
                // write 3 bytes, 0x000000, to 0x5E of CPLD
                WriteCPLDRegister(buffer4, 4, 0x02c0, "0x5e000000");
                WriteCPLDRegister(buffer4, 4, 0x02c4, "0x00000540");
                WriteCPLDRegister(buffer1, 1, 0x02c8, "0x02");
                WriteCPLDRegister(buffer1, 1, 0x02c8, "0x03");
                spDispatcher.BeginInvoke(update, new object[] { _progressPercentage, "Set Flash Done bit" });

                // Wait for !BUSY (0xF0)
                // write 3 bytes, 0x000000, to 0xF0 of CPLD. Then read 1 byte
                do
                {
                    WriteCPLDRegister(buffer4, 4, 0x02c0, "0xf0000000");
                    WriteCPLDRegister(buffer4, 4, 0x02c4, "0x000145c0");
                    WriteCPLDRegister(buffer1, 1, 0x02c8, "0x02");
                    WriteCPLDRegister(buffer1, 1, 0x02c8, "0x03");
                    System.Threading.Thread.Sleep(25);
                    busyStatus = ReadCPLDStatusRegister(buffer1);
                } while (Convert.ToBoolean(busyStatus & CPLD_BUSY_STATUS)); // busy Keep reading
                spDispatcher.BeginInvoke(update, new object[] { _progressPercentage, "Wait for !BUSY" });

                
                // Then Verify !FAIL (0x3C)
                // write 3 bytes, 0x000000, to 0xF0 of CPLD. Then read 4 bytes. Output byte no.3
                do
                {
                    WriteCPLDRegister(buffer4, 4, 0x02c0, "0x3c000000");
                    WriteCPLDRegister(buffer4, 4, 0x02c4, "0x0003a5c0");
                    WriteCPLDRegister(buffer1, 1, 0x02c8, "0x02");
                    WriteCPLDRegister(buffer1, 1, 0x02c8, "0x03");
                    System.Threading.Thread.Sleep(50);
                    busyStatus = ReadCPLDStatusRegister(buffer1);
                } while (Convert.ToBoolean(busyStatus & CPLD_FAIL_STATUS));
                spDispatcher.BeginInvoke(update, new object[] { _progressPercentage, "Verify !FAIL" });

                // Disable Configuration (0x26)
                // write 2 bytes, 0x0000, to 0x26 of CPLD
                WriteCPLDRegister(buffer4, 4, 0x02c0, "0x26000000");
                WriteCPLDRegister(buffer4, 4, 0x02c4, "0x00000440");
                WriteCPLDRegister(buffer1, 1, 0x02c8, "0x02");
                WriteCPLDRegister(buffer1, 1, 0x02c8, "0x03");
                spDispatcher.BeginInvoke(update, new object[] { _progressPercentage, "Disable Configuration" });

                // BYPASS (0xFF)
                // write 3 bytes, 0xffffff, to 0xff of CPLD
                WriteCPLDRegister(buffer4, 4, 0x02c0, "0xffffffff");
                WriteCPLDRegister(buffer4, 4, 0x02c4, "0x00000540");
                WriteCPLDRegister(buffer1, 1, 0x02c8, "0x02");
                WriteCPLDRegister(buffer1, 1, 0x02c8, "0x03");
                spDispatcher.BeginInvoke(update, new object[] { _progressPercentage, "BYPASS" });

                System.Threading.Thread.Sleep(1000);

                // Issue REFRESH (0x79)
                // write 2 bytes, 0x000000, to 0x79 of CPLD
                WriteCPLDRegister(buffer4, 4, 0x02c0, "0x79000000");
                WriteCPLDRegister(buffer4, 4, 0x02c4, "0x00000440");
                WriteCPLDRegister(buffer1, 1, 0x02c8, "0x02");
                WriteCPLDRegister(buffer1, 1, 0x02c8, "0x03");
                spDispatcher.BeginInvoke(update, new object[] { _progressPercentage, "Issue REFRESH" });

                //SetUserCode(0xc2)
                //WriteCPLDRegister(buffer4, 4, 0x02d0, "0x00000002");
                //WriteCPLDRegister(buffer4, 4, 0x02d4, "0x00000000");
                //WriteCPLDRegister(buffer1, 4, 0x02d8, "0x00000000");
                //WriteCPLDRegister(buffer1, 4, 0x02df, "0x00000000");

                //WriteCPLDRegister(buffer4, 4, 0x02c0, "0x20000000");
                //WriteCPLDRegister(buffer4, 4, 0x02c4, "0x00000940");
                //WriteCPLDRegister(buffer1, 1, 0x02c8, "0x02");
                //WriteCPLDRegister(buffer1, 1, 0x02c8, "0x03");

                // Wait for Trefresh > 1.9ms
                System.Threading.Thread.Sleep(1000);
                spDispatcher.BeginInvoke(update, new object[] { _progressPercentage, "Trefresh elapsed..." });

                // write to 0x3c, then verify DONE
                do
                {
                    WriteCPLDRegister(buffer4, 4, 0x02c0, "0x3c000000");
                    WriteCPLDRegister(buffer4, 4, 0x02c4, "0x0003a5c0");
                    WriteCPLDRegister(buffer1, 1, 0x02c8, "0x02");
                    WriteCPLDRegister(buffer1, 1, 0x02c8, "0x03");
                    System.Threading.Thread.Sleep(25);
                    busyStatus = ReadCPLDStatusRegister(buffer1);
                } while (!Convert.ToBoolean(busyStatus & CPLD_DONE_STATUS));
                spDispatcher.BeginInvoke(update, new object[] { _progressPercentage, "Verify DONE" });

                //create a new delegate for updating our progress text
                update = new UpdateProgressDelegate(UpdateCPLDProgressText);
                //invoke the dispatcher and pass the percentage
                spDispatcher.BeginInvoke(update, new object[] { _progressPercentage, "Update Done" });
                System.Threading.Thread.Sleep(1000);
                 
            };
            splashWkr.RunWorkerCompleted += delegate(object sender, RunWorkerCompletedEventArgs e)
            {
                _cpldSplash.Close();
                // App inherits from Application, and has a Window property called MainWindow
                // and a List<Window> property called OpenWindows.
                Application.Current.MainWindow.Activate();
                _progressPercentage = 0;
            };

            if (buffer1 != IntPtr.Zero)
            {
                Win32.FreeBuffer(buffer1);
                Win32.FreeBuffer(buffer2);
                Win32.FreeBuffer(buffer3);
                Win32.FreeBuffer(buffer4);

            }
        }
    }
}
