
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
        readonly byte[] BIT_MASK= new byte[4]{0x01,0x02,0x04,0x08};
        private bool[] _channelEnable = new bool[4] { false, false, false, false };
        private bool[] _channelBufferReady = new bool[4]{false,false,false,false};
        private bool _threadBusy = false;
        public bool _isFileSavedEnabled = false;
        THREAD_PARAMS thread_params;
        //Command Format: PacketRead <Card Offset> <HSize_VSize_FrameRate> <Number>
        public void ReadAddressablePacket(List<String> argumentsList)
        {
            if (_threadBusy == true)
            {
               return;
            }
            try
            {
                Thread DmaReadThreads = null;
                IMAGING_CONFIG_INFO imaging_config_info;
                imaging_config_info.Channel = 0;
                imaging_config_info.FrameNum = 1;
                imaging_config_info.FrameRate = 1;
                imaging_config_info.ImageHorizontalSize = 32;
                imaging_config_info.ImageVerticalSize = 32;

                //Get Channel Information
                byte channelEnabledInfo = (byte)(int.Parse(argumentsList[1].Replace("0x", string.Empty), NumberStyles.HexNumber));
                for (int i = 0; i < _channelEnable.Length; i++)
                {
                    if ((channelEnabledInfo & BIT_MASK[i]) != 0x00)
                    {
                        _channelEnable[i] = true;
                    }
                    else
                    {
                        _channelEnable[i] = false;
                    }
                }
                imaging_config_info.Channel = (ushort)channelEnabledInfo;
                //Get Image Size Info
                string[] image_params_args_array = argumentsList[2].Split('_');
                imaging_config_info.ImageHorizontalSize = Convert.ToUInt16(image_params_args_array[0]);
                imaging_config_info.ImageVerticalSize = Convert.ToUInt16(image_params_args_array[1]);
                //Get Frame Num Info
                if (argumentsList.Count > 3)
                {
                    imaging_config_info.FrameNum = Convert.ToUInt32(argumentsList[3]);
                }
                else
                {
                    imaging_config_info.FrameNum = 1;
                }
                //Get FrameRate Info
                if (argumentsList.Count > 4)
                {
                    imaging_config_info.FrameRate = Convert.ToUInt32(argumentsList[4]);
                }
                else
                {
                    imaging_config_info.FrameRate = 1;
                }

                //Define the buffer
                UInt32 image_buffer_size = (UInt32)(imaging_config_info.ImageHorizontalSize * imaging_config_info.ImageVerticalSize * 2);
                if (image_buffer_size > 0x00000000ffffffff)
                {
                    return;
                }
                if (ConfigPacketGenChk(Win32.PACKET_GEN_ENABLE, imaging_config_info) == false)
                {
                    return;
                }
                if (Win32.StartAcquisition( 0 ) != Win32.STATUS_SUCCESSFUL)
                {
                    return;
                }
                System.Threading.Thread.Sleep(200);
                thread_params = new THREAD_PARAMS();
                
                thread_params.NumberOfFrames = imaging_config_info.FrameNum;
                thread_params.MaxBufferSize = image_buffer_size;
                thread_params.Buffer = new IntPtr[4];
                for (int i = 0; i < 4; i++)
                {
                    Win32.AllocateBuffer(ref thread_params.Buffer[i], (uint)image_buffer_size);
                }
                thread_params.UserStatus = 0;

                if (DmaReadThreads == null)
                {
                    DmaReadThreads = new Thread(ImageAcquiringThreadMethod);
                }
                DmaReadThreads.Start(thread_params);
                _threadBusy = true;

                //while (_threadBusy)
                //{
                    
                //}
                //for (int i = 0; i < 4; i++)
                //{
                //    Win32.FreeBuffer(thread_params.Buffer[i]);
                //}

                //IntPtr buffer = IntPtr.Zero;
                //Win32.AllocateBuffer(ref buffer, 1);
                //System.Runtime.InteropServices.Marshal.WriteByte(buffer, 0);
                //Win32.MemoryWrite(0, buffer, 3, (UInt64)0, 0, (UInt64)1);
                //Win32.FreeBuffer(buffer);

                //Win32.StopAcquisition(0);
                //UpdateConsoleStatus("Read Successfully");
                //DisplayFolder(SettingPath);
                //_acquisitionActiveFlag = true;
            }
            catch (Exception e)
            {
                string s = e.Message;
            }
        }

        /*+F*************************************************************************
        * Function:
        *   ConfigPacketGenChk
        *
        * Description:
        *   Configures the hardware reference designs Packet Generator / Checker
        *-F*************************************************************************/
        bool ConfigPacketGenChk(byte mode, IMAGING_CONFIG_INFO imagingConfigInfo) 
        {
            bool ret = true;
            IMAGING_CONFIGURATION_STRUCT imaging_config = default(IMAGING_CONFIGURATION_STRUCT);
            //Initiation
            imaging_config.imageCtrl.channel = imagingConfigInfo.Channel;
            imaging_config.imageCtrl.frameCnt = 0;
            imaging_config.imageCtrl.frameNumPerSec = imagingConfigInfo.FrameRate;
            imaging_config.imageCtrl.imgHSize = imagingConfigInfo.ImageHorizontalSize;
            imaging_config.imageCtrl.imgVSize = imagingConfigInfo.ImageVerticalSize;
            imaging_config.imageCtrl.defaultMode = 1;
            //PacketGenChk.dacCtrl = new DAC_CRTL_STRUCT[12];

            if (Win32.SetupPacketGenerator(0, ref imaging_config) != Win32.STATUS_SUCCESSFUL)
            {
                ret = false;
            }
            return ret;
        }

        public void ImageAcquiringThreadMethod(Object param)
        {
            THREAD_PARAMS thread_params = (THREAD_PARAMS)param;
            Byte[] imageBuffer = new Byte[thread_params.MaxBufferSize];            
            UInt64 numberOfFrames = 0;
            do
            {
                for (byte channel = 0; channel < 4; channel++)
                {
                    if (_channelEnable[channel] == false)
                    {
                        continue;
                    }
                    uint length = thread_params.MaxBufferSize;
                    do
                    {
                        uint lengthOutput = length;
                        if (Win32.PacketRead(0, ref thread_params.UserStatus, channel, thread_params.Buffer[channel], ref lengthOutput) == Win32.STATUS_SUCCESSFUL && lengthOutput != 0)
                        {
                            try
                            {
                                if (_isFileSavedEnabled)
                                {
                                    System.Runtime.InteropServices.Marshal.Copy(thread_params.Buffer[channel], imageBuffer, 0, (int)lengthOutput);
                                    string filename = SettingPath + @"\Data_Channel" + (channel + 1).ToString() + "_" + numberOfFrames.ToString() + "_" + DateTime.Now.ToString("yyyy-MM-dd-hh-mm-ss-tt");
                                    File.WriteAllBytes(filename, imageBuffer);
                                }
                                else
                                {
                                    if (CommonDefinition.IsPixelDataReady == false)
                                    {
                                        // System.Runtime.InteropServices.Marshal.Copy(thread_params.Buffer[channel], ImageSource, 0, (int)lengthOutput / 2);
                                        CommonDefinition.IsPixelDataReady = true;
                                    }
                                } 
                            }
                            catch (Exception e) 
                            {
                                MessageBox.Show("Error" + e.Message);
                            }
                            break;
                        }
                        else if (_acquisitionActiveFlag == false)
                        {
                            break;
                        }
                    } while (true);
                }
                if (_acquisitionActiveFlag)
                {
                    numberOfFrames++;
                }
            } while (numberOfFrames < thread_params.NumberOfFrames && _acquisitionActiveFlag);

            Application.Current.Dispatcher.BeginInvoke(new Action(() =>
            {
                for (int i = 0; i < 4; i++)
                {
                    Win32.FreeBuffer(thread_params.Buffer[i]);
                }
                Win32.StopAcquisition(0);
                if (_isFileSavedEnabled)
                {
                    UpdateConsoleStatus("Read Successfully");
                    _isFileSavedEnabled = false;
                }
                else
                {
                    UpdateConsoleStatus("LiveCapture is stopped");
                }
                _threadBusy = false;
                _acquisitionActiveFlag = true;
            }));
        }


        void CreateThumbnail(string filename, BitmapSource image5)
        {
            if (filename != string.Empty)
            {
                using (FileStream stream5 = new FileStream(filename, FileMode.Create))
                {
                    PngBitmapEncoder encoder5 = new PngBitmapEncoder();
                    encoder5.Frames.Add(BitmapFrame.Create(image5));
                    encoder5.Save(stream5);
                    stream5.Close();
                }
            }
        }

        // arg0 - Program name
        // arg1 - Thordaq DDR3 starting address
        // arg2 - Bytes to write
        // arg3...argN  - Data byte values
        // 
        public void WriteDDR3(List<String> argumentsList)
        {
            int DataByteArgs = argumentsList.Count - 3; // data byte arg count
            ulong CardOffset;
            uint ByteTransferLen; // # bytes to read
            Byte FirstDataByte;   // must exist

            if (argumentsList[1].Contains("0x"))
            {
                CardOffset = ulong.Parse(argumentsList[1].Replace("0x", string.Empty), NumberStyles.HexNumber);
            }
            else
            {
                CardOffset = ulong.Parse(argumentsList[1]);
            }

            if (CardOffset > 0xffffffff)
            {
                return;
            }

            if (argumentsList[2].Contains("0x"))
            {
                ByteTransferLen = uint.Parse(argumentsList[2].Replace("0x", string.Empty), NumberStyles.HexNumber);
            }
            else
            {
                ByteTransferLen = uint.Parse(argumentsList[2]);
            }

            if (ByteTransferLen > 0xffffffff)
            {
                return;
            }
            // what data to bytes to write?
            if (DataByteArgs <= 0)
            {
                UpdateConsoleStatus("ERROR: need additional data byte args");
                return;
            }
            // grab first data byte - it may be copied for entire length
            if (argumentsList[3].Contains("0x"))
            {
                FirstDataByte = Byte.Parse(argumentsList[3].Replace("0x", string.Empty), NumberStyles.HexNumber);
            }
            else
            {
                FirstDataByte = Byte.Parse(argumentsList[3]);
            }

            // Make sure NWL GlobalDMA int NOT disabled
            UInt64 value = 0;
            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("NWL_DDR3_DMAcore", ref value);
            if ((value & 0x1) == 0) // LSB is GlobalDMAen.
            {
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("NWL_GlobalDMAIntEnable", 1);
                UpdateConsoleStatus("Set NWL_GlobalDMAIntEnable");
            }


            uint transferredLen = ByteTransferLen; // we can check whether hardware DMA'd entire length
            Byte[] DDR3writeBuff = new Byte[ByteTransferLen];
            int i, argNum;

            if (DataByteArgs == 1) // repeat data byte for ByteTransferLen
            {
                for (int j = 0; j < ByteTransferLen; j++)
                {
                    DDR3writeBuff[j] = FirstDataByte;
                }
            }
            else // use the set of passed bytes...
            {
                for (i = 0, argNum = 3; DataByteArgs > 0 && (i < ByteTransferLen); --DataByteArgs, ++argNum, i++)
                {
                    if (argumentsList[argNum].Contains("0x"))
                        DDR3writeBuff[i] = Byte.Parse(argumentsList[argNum].Replace("0x", string.Empty), NumberStyles.HexNumber);
                    else
                        DDR3writeBuff[i] = Byte.Parse(argumentsList[argNum]);
                }
                ByteTransferLen = (uint)i; // use command line number of bytes entered
            }

            bool bStatus;
//            bStatus = ThorDAQAdapters[MasterBoardIndex].DDR3RAM.SetupPacketMode();
//           if( bStatus == false)
//                UpdateConsoleStatus(ThorDAQAdapters[MasterBoardIndex].DDR3RAM.sErrMsg);

            bStatus = ThorDAQAdapters[MasterBoardIndex].DDR3RAM.WriteDDR3(CardOffset, ref transferredLen, DDR3writeBuff);
            if( bStatus == false )
                UpdateConsoleStatus(ThorDAQAdapters[MasterBoardIndex].DDR3RAM.sErrMsg);

//            ThorDAQAdapters[MasterBoardIndex].DDR3RAM.ShutdownPacketMode();
        }


        public void ReadBARmem(List<String> argumentsList)
        {

        }
        public void WriteBARmem(List<String> argumentsList)
        {

        }

        //   ARG:    0      1   2    3
        // usage: ReadDDR3 0x0 256 <-q>  (start at 0, read 256 bytes, return in "ref" rather than console)
        public static List<String> ReadDDR3(List<String> argumentsList, ref Byte[] DDR3bytes) 
        {
            List<string> ReturnStrings = new List<string> { };
            ulong CardDDR3Offset = 0;
            uint ByteTransferLen; // # bytes to read
            bool QuietMode = false;

            if (argumentsList[1].Contains("0x"))
            {
                CardDDR3Offset = ulong.Parse(argumentsList[1].Replace("0x", string.Empty), NumberStyles.HexNumber);
            }
            else
            {
                CardDDR3Offset = ulong.Parse(argumentsList[1]);
            }

            if (CardDDR3Offset > 0xffffffff)
            {
                ReturnStrings.Add(string.Format("CardDDR3Offset {0} out of range", CardDDR3Offset));
                return ReturnStrings;
            }

            if (argumentsList[2].Contains("0x"))
            {
                ByteTransferLen = uint.Parse(argumentsList[2].Replace("0x", string.Empty), NumberStyles.HexNumber);
            }
            else
            {
                ByteTransferLen = uint.Parse(argumentsList[2]);
            }
            if (argumentsList.Count >= 4 && argumentsList[3] == "-q")
            {
                QuietMode = true;  // return data in list instead of printing on Console
            }

            if (ByteTransferLen > 0xffffffff)
            {
                ReturnStrings.Add(string.Format("ByteTransferLen {0} out of range", ByteTransferLen));
                return ReturnStrings;
            }

            uint transferredLen = ByteTransferLen; // we can check whether hardware DMA'd entire length
            Byte[] SysHostreadBuff = new Byte[ByteTransferLen];

            // Make sure NWL GlobalDMA int NOT disabled
            UInt64 value = 0;
            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("NWL_DDR3_DMAcore", ref value);
            if ((value & 0x1) == 0) // LSB is GlobalDMAen.
            {
                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("NWL_GlobalDMAIntEnable", 1);
                //                UpdateConsoleStatus("Set NWL_GlobalDMAIntEnable");
            }

            bool bStatus = ThorDAQAdapters[MasterBoardIndex].DDR3RAM.ReadDDR3(CardDDR3Offset, ref transferredLen, ref SysHostreadBuff);
            if (bStatus == false)
            {
                ReturnStrings.Add(string.Format("Error reading DDR3RAM: {0}", ThorDAQAdapters[MasterBoardIndex].DDR3RAM.sErrMsg));
                return ReturnStrings;

            }

            if (transferredLen != ByteTransferLen)
            {
                ReturnStrings.Add(string.Format("ERROR: requested: " + ByteTransferLen + "  bytes but received " + transferredLen));
                return ReturnStrings;
            }
            if (QuietMode) 
            {
                DDR3bytes = SysHostreadBuff;
            }
            else
            {
                // print to console
                foreach( string Str in format16ByteLinesOnConsole(CardDDR3Offset, SysHostreadBuff, transferredLen))
                {
                    ReturnStrings.Add(Str);
                }
            }
            ReturnStrings.Add("SUCCESS");
            return ReturnStrings;
        }

        // format the DDR3 memory bytes read back
        // 
        static public List<string> format16ByteLinesOnConsole(ulong startingOffset, Byte[] databytes, uint len)
        {
            List<string> ReturnStrings = new List<string> { };

            ReturnStrings.Add(string.Format("Offset (h)  00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F", 1));// suppress "<<<"
            // update line at a time...
            ulong ByteArrayOffset = 0;
            string thisLinePrefix; 
            int FullLines = (int)len / 16;
            ulong PartialLine = (ulong)len % 16;
            while (FullLines > 0 || PartialLine > 0)
            {
                thisLinePrefix = string.Format("{0}    ", startingOffset.ToString("X8"));
                if (FullLines == 0 && PartialLine > 0) // LAST line, 1 - 15 values
                {
                    for (ulong j = ByteArrayOffset; j < ByteArrayOffset + PartialLine; j++)
                    {
                        thisLinePrefix += string.Format("{0} ", databytes[j].ToString("X2"));
                    }
                    PartialLine = 0; // DONE
                }
                else // a full line of 16 values...
                {
                    for (ulong j = 0; j < 16; j++, ByteArrayOffset++)
                        thisLinePrefix += string.Format("{0} ", databytes[ByteArrayOffset].ToString("X2"));
                    
                }
                ReturnStrings.Add(string.Format(thisLinePrefix, 1)); // suppress "<<<"
                // next line...
                startingOffset += 16;
                FullLines--;
            }
            return ReturnStrings;
        }

        public void ReadDram(List<String> argumentsList)
        {
            try
            {
                //Get Channel Information
                ulong CardOffset;
                uint TransferSize;

                if (argumentsList[1].Contains("0x"))
                {
                    CardOffset = ulong.Parse(argumentsList[1].Replace("0x", string.Empty), NumberStyles.HexNumber);
                }
                else
                {
                    CardOffset = ulong.Parse(argumentsList[1]);
                }

                if (CardOffset > 0xffffffff)
                {
                    return;
                }

                if (argumentsList[2].Contains("0x"))
                {
                    TransferSize = uint.Parse(argumentsList[2].Replace("0x", string.Empty), NumberStyles.HexNumber) *2;
                }
                else
                {
                    TransferSize = uint.Parse(argumentsList[2]) * 2;
                }

                if (TransferSize > 0xffffffff)
                {
                    return;
                }

                IntPtr buffer = IntPtr.Zero;
                Win32.AllocateBuffer(ref buffer, (uint)(TransferSize));

                uint uiStatus = Win32.ReadDRamex(0, CardOffset, buffer, ref TransferSize);
                
                
                if (uiStatus == Win32.STATUS_SUCCESSFUL)
                {
                    Byte[] imageBuffer = new Byte[TransferSize];
                    System.Runtime.InteropServices.Marshal.Copy(buffer, imageBuffer, 0, (int)imageBuffer.Length);
                    string filename = SettingPath + @"\Data_Location" + CardOffset.ToString() + "_" + DateTime.Now.ToString("yyyy-MM-dd-hh-mm-ss-tt") + ".HEX";
                    File.WriteAllBytes(filename, imageBuffer);
                    UpdateConsoleStatus("Read Successfully");
                }
                else // error from NWL function
                {
                    string ErrString = " ";
                    if (uiStatus == 9) // initial error when DMA not established
                    {
                        ErrString = "  DMA is not setup - system may BSOD";
                    }
                    UpdateConsoleStatus("Win32.ReadDRamex() error: " + uiStatus + ErrString);
                }
                Win32.FreeBuffer(buffer);
            }
            catch (Exception e)
            {
                MessageBox.Show("Error" + e.Message);
            }
        }


        public void WriteDram(List<String> argumentsList)
        {
            try
            {
                //Get Channel Information
                ulong CardOffset;
                uint TransferSize;

                if (argumentsList[1].Contains("0x"))
                {
                    CardOffset = ulong.Parse(argumentsList[1].Replace("0x", string.Empty), NumberStyles.HexNumber);
                }
                else
                {
                    CardOffset = ulong.Parse(argumentsList[1]);
                }

                if (CardOffset > 0xffffffff)
                {
                    return;
                }

                if (argumentsList[2].Contains("0x"))
                {
                    TransferSize = uint.Parse(argumentsList[2].Replace("0x", string.Empty), NumberStyles.HexNumber) * 2;
                }
                else
                {
                    TransferSize = uint.Parse(argumentsList[2]) * 2;
                }

                if (TransferSize > 0xffffffff)
                {
                    return;
                }
                IntPtr buffer = IntPtr.Zero;
                Win32.AllocateBuffer(ref buffer, (uint)(TransferSize));
                for (int i = 0; i < TransferSize; i++)
                {
                    System.Runtime.InteropServices.Marshal.WriteByte(buffer, i,0x04);
                }
                if (Win32.WriteDRamex(0, CardOffset, buffer, TransferSize) == Win32.STATUS_SUCCESSFUL)
                {
                    //Byte[] imageBuffer = new Byte[TransferSize];
                    //System.Runtime.InteropServices.Marshal.Copy(buffer, imageBuffer, 0, (int)imageBuffer.Length);
                    //string filename = SettingPath + @"\Data_Location" + CardOffset.ToString() + "_" + DateTime.Now.ToString("yyyy-MM-dd-hh-mm-ss-tt");
                    //File.WriteAllBytes(filename, imageBuffer);
                    UpdateConsoleStatus("Write Successfully");
                }
                Win32.FreeBuffer(buffer);
            }
            catch (Exception e)
            {
                MessageBox.Show("Error" + e.Message);
            }
        }

        public void WriteMezzanineSynth(List<String> argumentsList)
        {
            try
            {
                //Get Channel Information
                ulong CardOffset;
                uint data;

                if (argumentsList[1].Contains("0x"))
                {
                    CardOffset = ulong.Parse(argumentsList[1].Replace("0x", string.Empty), NumberStyles.HexNumber);
                }
                else
                {
                    CardOffset = ulong.Parse(argumentsList[1]);
                }

                if (argumentsList[2].Contains("0x"))
                {
                    data = uint.Parse(argumentsList[2].Replace("0x", string.Empty), NumberStyles.HexNumber);
                }
                else
                {
                    data = uint.Parse(argumentsList[2]);
                }
            }
            catch (Exception e)
            {
                MessageBox.Show("Error" + e.Message);
            }
        }
    }
    

    public struct IMAGING_CONFIG_INFO
    {
        ///Channel Info
        public ushort Channel; 
        /// <summary>Image Horizontal Size value</summary>
        public ushort ImageHorizontalSize;
        /// <summary>Image Vertical Size value</summary>
        public ushort ImageVerticalSize;
        /// <summary>Frame Rate</summary>
        public uint FrameRate;
        /// <summary>Frames to capture</summary>
        public uint FrameNum;
    }

    public struct THREAD_PARAMS 
    {
        public UInt32 NumberOfFrames;
        public UInt32 MaxBufferSize;
        public UInt64 UserStatus;
        public IntPtr[] Buffer;
    }

    

}



