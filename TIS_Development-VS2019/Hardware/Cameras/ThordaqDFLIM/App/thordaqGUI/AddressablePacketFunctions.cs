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
    using System.Threading;
    using System.Drawing;
    using System.IO;

    using NWLogic.DeviceLib;

    public partial class MainWindow : Window
    {
        readonly byte[] BIT_MASK= new byte[4]{0x01,0x02,0x04,0x08};
        private bool[] _channelEnable = new bool[4] { false, false, false, false };
        private bool[] _channelBufferReady = new bool[4]{false,false,false,false};
        private bool _threadBusy = false;
        private bool _isFileSavedEnabled = false;
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
            PACKET_GENCHK_STRUCT PacketGenChk = default(PACKET_GENCHK_STRUCT);
            //Initiation
            PacketGenChk.imageCtrl.channel = imagingConfigInfo.Channel;
            PacketGenChk.imageCtrl.frameCnt = 0;
            PacketGenChk.imageCtrl.frameNumPerSec = imagingConfigInfo.FrameRate;
            PacketGenChk.imageCtrl.imgHSize = imagingConfigInfo.ImageHorizontalSize;
            PacketGenChk.imageCtrl.imgVSize = imagingConfigInfo.ImageVerticalSize;
            PacketGenChk.imageCtrl.defaultMode = 1;
            //PacketGenChk.dacCtrl = new DAC_CRTL_STRUCT[12];

            if (Win32.SetupPacketGenerator(0, ref PacketGenChk) != Win32.STATUS_SUCCESSFUL)
            {
                ret = false;
            }
            return ret;
        }

        public void SetPacketModeAddressable()
        {
            Win32.SetPacketModeAddressable(0, false);
        }

        public void ReadChannel(List<String> argumentsList)
        {
            try
            {
                //Get Channel Information
                byte channel;
                uint transferSize;

                if (argumentsList[1].Contains("0x"))
                {
                    channel = byte.Parse(argumentsList[1].Replace("0x", string.Empty), NumberStyles.HexNumber);
                }
                else
                {
                    channel = byte.Parse(argumentsList[1]);
                }

                if (argumentsList[2].Contains("0x"))
                {
                    transferSize = uint.Parse(argumentsList[2].Replace("0x", string.Empty), NumberStyles.HexNumber);
                }
                else
                {
                    transferSize = uint.Parse(argumentsList[2]);
                }

                if (transferSize > 0xffffffff)
                {
                    return;
                }
                IntPtr buffer = IntPtr.Zero;
                try
                {
                    Win32.AllocateBuffer(ref buffer, (uint)(transferSize));
                    UInt64 status = 0;
                    if (Win32.PacketRead(0, ref status, channel, buffer, ref transferSize) == Win32.STATUS_SUCCESSFUL && transferSize != 0)
                    {
                        Byte[] imageBuffer = new Byte[transferSize];
                        System.Runtime.InteropServices.Marshal.Copy(buffer, imageBuffer, 0, (int)imageBuffer.Length);
                        //string filename = SettingPath + @"\Data_Location"+CardOffset.ToString()+"_"+ DateTime.Now.ToString("yyyy-MM-dd-hh-mm-ss-tt");
                        //File.WriteAllBytes(filename, imageBuffer);
                        UpdateConsoleStatus("Read Successfully: " + ByteArrayToString(imageBuffer));
                    }
                }
                catch (Exception ex)
                {
                    MessageBox.Show("Error" + ex.Message);
                }
                Win32.FreeBuffer(buffer);
            }
            catch (Exception e)
            {
                MessageBox.Show("Error" + e.Message);
            }
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
                                    if (IsPixelDataReady == false)
                                    {
                                        System.Runtime.InteropServices.Marshal.Copy(thread_params.Buffer[channel], ImageSource, 0, (int)lengthOutput / 2);
                                        IsPixelDataReady = true;
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
                    TransferSize = uint.Parse(argumentsList[2].Replace("0x", string.Empty), NumberStyles.HexNumber);
                }
                else
                {
                    TransferSize = uint.Parse(argumentsList[2]);
                }

                if (TransferSize > 0xffffffff)
                {
                    return;
                }
                IntPtr buffer = IntPtr.Zero;
                try
                {
                    Win32.AllocateBuffer(ref buffer, (uint)(TransferSize));
                    if (Win32.ReadDRamex(0, CardOffset, buffer, ref TransferSize) == Win32.STATUS_SUCCESSFUL)
                    {
                        Byte[] imageBuffer = new Byte[TransferSize];
                        System.Runtime.InteropServices.Marshal.Copy(buffer, imageBuffer, 0, (int)imageBuffer.Length);
                        //string filename = SettingPath + @"\Data_Location"+CardOffset.ToString()+"_"+ DateTime.Now.ToString("yyyy-MM-dd-hh-mm-ss-tt");
                        //File.WriteAllBytes(filename, imageBuffer);
                        UpdateConsoleStatus("Read Successfully: " + ByteArrayToString(imageBuffer));
                    }
                }
                catch (Exception ex)
                {
                    MessageBox.Show("Error" + ex.Message);
                }
                Win32.FreeBuffer(buffer);
            }
            catch (Exception e)
            {
                MessageBox.Show("Error" + e.Message);
            }
        }

        public static string ByteArrayToString(byte[] ba)
        {
            return BitConverter.ToString(ba).Replace("-", "");
        }

        public void WriteDram(List<String> argumentsList)
        {
            try
            {
                //Get Channel Information
                ulong CardOffset;
                uint TransferSize;
                byte[] value;
                string valueStr;
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

                //if (argumentsList[2].Contains("0x"))
                //{
                //    TransferSize = uint.Parse(argumentsList[2].Replace("0x", string.Empty), NumberStyles.HexNumber) * 2;
                //}
                //else
                //{
                //    TransferSize = uint.Parse(argumentsList[2]) * 2;
                //}

                if (argumentsList[2].Contains("0x"))
                {
                    valueStr = argumentsList[2].Replace("0x", string.Empty);
                }
                else
                {
                    valueStr = argumentsList[3];
                }

                value = ConvertHexStringToByteArray(valueStr);

                TransferSize = (uint)value.Length;

                if (TransferSize > 0xffffffff)
                {
                    return;
                }
                IntPtr buffer = IntPtr.Zero;
                Win32.AllocateBuffer(ref buffer, (uint)(TransferSize));
                for (int i = 0; i < TransferSize; i++)
                {
                    System.Runtime.InteropServices.Marshal.WriteByte(buffer, i, value[i]);
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

        public static byte[] ConvertHexStringToByteArray(string hexString)
        {
            if (hexString.Length % 2 != 0)
            {
                throw new ArgumentException(String.Format(CultureInfo.InvariantCulture, "The binary key cannot have an odd number of digits: {0}", hexString));
            }

            byte[] data = new byte[hexString.Length / 2];
            for (int index = 0; index < data.Length; index++)
            {
                string byteValue = hexString.Substring(index * 2, 2);
                data[index] = byte.Parse(byteValue, NumberStyles.HexNumber, CultureInfo.InvariantCulture);
            }

            return data;
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



