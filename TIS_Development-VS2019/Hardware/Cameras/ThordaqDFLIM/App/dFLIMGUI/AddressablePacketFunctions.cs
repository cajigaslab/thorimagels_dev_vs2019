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
    using System.Runtime.InteropServices;
    using ThorSharedTypes;

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
        
        
        
        
        public void LiveCaptureDDR3(List<String> argumentsList)
        {
            double PixelDwellUS = 1; // pixel dwell in microSecs
            if (_threadBusy == true)
            {
               return;
            }
            try
            {
                // these initial calls copy TILS behavior of early camera discovery (before TILS human interface appears)
                long CameraCount = 0;
                uint DLLstatus = Win32.FindCameras(ref CameraCount); // initializes dFLIM front end, etc.
                DLLstatus = Win32.SelectCamera(CameraCount); // sets the DAC PARK for x,y galvo

                Thread DmaReadThreads = null;
                IMAGING_CONFIG_INFO imaging_config_info;
                imaging_config_info.Channel = 0;
                imaging_config_info.FrameNum = 1;
                imaging_config_info.FrameRate = 1;
                imaging_config_info.ImageHorizontalSize = 32;
                imaging_config_info.ImageVerticalSize = 32;

                //Get Channel Information
                byte channelEnabledMASK = (byte)(int.Parse(argumentsList[1].Replace("0x", string.Empty), NumberStyles.HexNumber));
                for (int i = 0; i < _channelEnable.Length; i++)
                {
                    if ((channelEnabledMASK & BIT_MASK[i]) != 0x00)
                    {
                        _channelEnable[i] = true;
                    }
                    else
                    {
                        _channelEnable[i] = false;
                    }
                }
                imaging_config_info.Channel = (ushort)channelEnabledMASK;
                //Get Image Size Info
                string[] image_params_args_array = argumentsList[2].Split('x');
                imaging_config_info.ImageHorizontalSize = Convert.ToUInt16(image_params_args_array[0]);
                imaging_config_info.ImageVerticalSize = Convert.ToUInt16(image_params_args_array[1]);
                //Get Frame Num Info
                if(argumentsList.Count >= 4)
                {
                    PixelDwellUS = Convert.ToDouble(argumentsList[3]);
                }
                if (argumentsList.Count >= 5)  // COUNT of frames to acquire
                {
                    imaging_config_info.FrameNum = Convert.ToUInt32(argumentsList[4]);
                }
                else
                {
                    imaging_config_info.FrameNum = 1;  // count of frames
                }
                //Get FrameRate Info
                if (argumentsList.Count >= 6 )
                {
                    imaging_config_info.FrameRate = Convert.ToUInt32(argumentsList[5]);
                }
                else
                {
                    imaging_config_info.FrameRate = 1;
                }

                IntPtr unusedPtr = IntPtr.Zero;
                uint uiStatus;
                uiStatus = Win32.PreflightAcquisition(unusedPtr);
                UInt32 dataHSize = ConfigImageAcq(imaging_config_info, PixelDwellUS);  // calls upper level DLL function dFLIMSetTestUtilConfig
                UpdateConsoleStatus("dataHSize = " + dataHSize.ToString());

                //Define the buffer
                UInt32 image_buffer_size = (UInt32)(dataHSize * imaging_config_info.ImageVerticalSize); // in dFLIM, total raw frame size is single line bytes * number lines
                if (image_buffer_size > 0xE0000000)  // the 224 MB max dFLIM frame size
                {
                    return;
                }

                uiStatus = Win32.SetupAcquisition(unusedPtr);
                uiStatus = Win32.StartAcquisition(unusedPtr);

                if (uiStatus != 1) // old API returns "TRUE" on success
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
        UInt32 ConfigImageAcq(IMAGING_CONFIG_INFO imagingConfigInfo, double PixelDwellMS) 
        {

            //PACKET_GENCHK_STRUCT PacketGenChk = default(PACKET_GENCHK_STRUCT);
            //Initiation
            CL_GUI_GLOBAL_TEST_STRUCT config = default(CL_GUI_GLOBAL_TEST_STRUCT);
            config.channel = imagingConfigInfo.Channel;
            config.frameCnt = imagingConfigInfo.FrameNum;
            // if only ONE frame, mode is single frame
            config.triggerMode = (imagingConfigInfo.FrameNum == 1) ? 
                    (uint)ThorSharedTypes.ICamera.TriggerMode.SW_SINGLE_FRAME : (uint)ThorSharedTypes.ICamera.TriggerMode.SW_MULTI_FRAME;
            config.frameNumPerSec = imagingConfigInfo.FrameRate;
            config.imgHSize = imagingConfigInfo.ImageHorizontalSize;
            config.imgVSize = imagingConfigInfo.ImageVerticalSize;
            config.acquisitionMode = 0; // dFLIM = 0, diagnostic = 1 ?
            config.dwellTime = PixelDwellMS;
            //PacketGenChk.imageCtrl.channel = imagingConfigInfo.Channel;
            //PacketGenChk.imageCtrl.frameCnt = imagingConfigInfo.FrameNum; 
            //PacketGenChk.imageCtrl.frameNumPerSec = imagingConfigInfo.FrameRate;
            //PacketGenChk.imageCtrl.imgHSize = imagingConfigInfo.ImageHorizontalSize;
            //PacketGenChk.imageCtrl.imgVSize = imagingConfigInfo.ImageVerticalSize;
            //PacketGenChk.imageCtrl.defaultMode = 1;
            //PacketGenChk.dacCtrl = new DAC_CRTL_STRUCT[12];

            UInt32 dataHSize = Win32.dFLIMSetTestUtilConfig(config);

            return dataHSize;
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
                    if (Win32.ReadDDR3Mem(0, ref status, channel, buffer, ref transferSize) == Win32.STATUS_SUCCESSFUL && transferSize != 0)
                    {
                        Byte[] imageBuffer = new Byte[transferSize];
                        Marshal.Copy(buffer, imageBuffer, 0, (int)imageBuffer.Length);
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

        Byte[] DDR3imageBuffer = new Byte[0xE000000];  // max 224 MB raw frame
        IntPtr DDR3NonManagedMemBufIntPtr = IntPtr.Zero; 
        public void ImageAcquiringThreadMethod(Object param)
        {
            THREAD_PARAMS thread_params = (THREAD_PARAMS)param;
            Win32.AllocateBuffer(ref DDR3NonManagedMemBufIntPtr, 0xE000000); // buffer for the dFLIM DLL
            UInt64 numberOfFrames = 0;
            do
            {
                byte channel = 0;  // just acquire the first data channel
                //for ( channel < 4; channel++)
                {
                    // we don't care about the data, just exercising the logic
                    uint length = thread_params.MaxBufferSize;
                    uint FrameDataReturned = 0;
                    do 
                    {
                        uint lengthOutput = length;
                        // Try the CopyAcquisition upper level DLL function
                        FrameInfoStruct frameInfo = new FrameInfoStruct { };
                        frameInfo.bufferType = (int)BufferType.DFLIM_ALL; // we want type "ALL" for buffer - largest possible transfer of data.  No need for any data decode
                        // CopyAcq returns 0 if there was NO image data returned, 1 if image data is returned
                        // Run this loop continuously looking for image data to be returned by CopyAcq
                        // CopyAcq gets ALL channels which have been enabled
                        FrameDataReturned = Win32.CopyAcquisition(DDR3NonManagedMemBufIntPtr, ref frameInfo);
                        if (FrameDataReturned == 1) // pass IntPtr with largest possible 0xE00.0000 frame size
//                            if ( Win32.ThorDAQAPIReadDDR3(0, 0x0, DDR3NonManagedMemBuf, ref lengthOutput) == 0)
                        {
                            thread_params.Buffer[channel] = DDR3NonManagedMemBufIntPtr; // we will truncate largest buffer to match our BitMap processing in CL-GUI (garbage data)
                            try
                            {
                                if (_isFileSavedEnabled)
                                {
                                    System.Runtime.InteropServices.Marshal.Copy(thread_params.Buffer[channel], DDR3imageBuffer, 0, (int)lengthOutput);
                                    string filename = SettingPath + @"\Data_Channel" + (channel + 1).ToString() + "_" + numberOfFrames.ToString() + "_" + DateTime.Now.ToString("yyyy-MM-dd-hh-mm-ss-tt");
                                    File.WriteAllBytes(filename, DDR3imageBuffer);
                                }
                                else
                                {
                                    if (IsPixelDataReady == false)
                                    {
                                        System.Runtime.InteropServices.Marshal.Copy(thread_params.Buffer[channel], ImageSource, 0, (int)ImageSource.Length); // junk data, not dFLIM decoded
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
                    } while (FrameDataReturned == 0 ); // loop until CopyAcq returns data, or Flag makes us break
                }
                if (_acquisitionActiveFlag)
                {
                    numberOfFrames++;
                }
            } while (numberOfFrames < thread_params.NumberOfFrames && _acquisitionActiveFlag);

            Application.Current.Dispatcher.BeginInvoke(new Action(() =>
            {
                IntPtr unusedPtr = IntPtr.Zero;

                for (int i = 0; i < 4; i++)
                {
                    Win32.FreeBuffer(thread_params.Buffer[i]);
                }
        //        Win32.StopAcquisition(unusedPtr);
                if (_isFileSavedEnabled)
                {
                    UpdateConsoleStatus("Read Successfully");
                    _isFileSavedEnabled = false;
                }
                else
                {
                    if (_bw.IsBusy)
                    {
                        StopAcquisition();
                    }
                    UpdateConsoleStatus("LiveCapture is stopped");
                }
                _threadBusy = false;
                _acquisitionActiveFlag = true;
            }));
            Win32.FreeBuffer(DDR3NonManagedMemBufIntPtr); // free huge 224MB memory area
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
                UpdateConsoleStatus(str, 1);
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
                    UpdateConsoleStatus(str, 1);
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


        //   ARG:    0      1   2    3
        // usage: ReadDDR3 0x0 256 <-q>  (start at 0, read 256 bytes, return in "ref" rather than console)
        static public List<String> ReadDDR3(List<String> argumentsList, ref Byte[] DDR3bytes)
        {
            List<string> ReturnStrings = new List<string> { };
            ulong CardDDR3Offset = 0;
            uint ByteTransferLen; // # bytes to read
 //           bool QuietMode = false;

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
//                QuietMode = true;  // return data in list instead of printing on Console
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
     //       ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("NWL_DDR3_DMAcore", ref value);
            if ((value & 0x1) == 0) // LSB is GlobalDMAen.
            {
         //       ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("NWL_GlobalDMAIntEnable", 1);
                //                UpdateConsoleStatus("Set NWL_GlobalDMAIntEnable");
            }

            IntPtr DDR3memBuf = IntPtr.Zero; ;
            Win32.AllocateBuffer(ref DDR3memBuf, ByteTransferLen);
            uint Status = Win32.ThorDAQAPIReadDDR3(MasterBoardIndex, CardDDR3Offset, DDR3memBuf, ref transferredLen);
            if (Status != 0)
            {
                Win32.FreeBuffer(DDR3memBuf);
                ReturnStrings.Add("Error on  ThorDAQAPIReadDDR3: " + Status.ToString("X"));
                return ReturnStrings;

            }

            if (transferredLen != ByteTransferLen)
            {
                ReturnStrings.Add(string.Format("ERROR: requested: " + ByteTransferLen + "  bytes but received " + transferredLen));
                Win32.FreeBuffer(DDR3memBuf);
                return ReturnStrings;
            }
            // convert C++ DLL data to managed byte array
            Marshal.Copy(DDR3memBuf, SysHostreadBuff, 0, (int)transferredLen);
            //           if (QuietMode)
            //           {
            //               DDR3bytes = SysHostreadBuff;
            //           }
            //           else
            {
                // print to console
                foreach (string Str in format16ByteLinesOnConsole(CardDDR3Offset, SysHostreadBuff, transferredLen))
                {
                    ReturnStrings.Add(Str);
                }
            }
            ReturnStrings.Add("SUCCESS");
            Win32.FreeBuffer(DDR3memBuf);
            return ReturnStrings;
        }

        // arg0 - Program name
        // arg1 - Thordaq DDR3 starting address
        // arg2 - Bytes to write
        // arg3...argN  - Data byte values
        // hereeee
        static public List<String> WriteDDR3(List<String> argumentsList)
        {
            uint status;
            List<string> ReturnStrings = new List<string> { };
            int DataByteArgs = argumentsList.Count - 3; // data byte arg count
            ulong DDR3address;
            uint ByteTransferLen; // # bytes to read
            Byte FirstDataByte;   // must exist

            if (argumentsList[1].Contains("0x"))
            {
                DDR3address = ulong.Parse(argumentsList[1].Replace("0x", string.Empty), NumberStyles.HexNumber);
            }
            else
            {
                DDR3address = ulong.Parse(argumentsList[1]);
            }

            if (DDR3address > 0xffffffff)
            {
                ReturnStrings.Add("Invalid CardOffset address (over 0xffffFFFF ");
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

            if (ByteTransferLen > 0xffffffff)
            {
                ReturnStrings.Add("Invalid ByteTransferLen (over 0xffffFFFF "); ;
            }
            // what data to bytes to write?
            if (DataByteArgs <= 0)
            {
                ReturnStrings.Add("ERROR: need additional data byte args");
                return ReturnStrings;
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
            //            UInt64 value = 0;
            //            ThorDAQAdapters[MasterBoardIndex].FPGAregs.Read("NWL_DDR3_DMAcore", ref value);
            //            if ((value & 0x1) == 0) // LSB is GlobalDMAen.
            //            {
            //                ThorDAQAdapters[MasterBoardIndex].FPGAregs.Write("NWL_GlobalDMAIntEnable", 1);
            //                ReturnStrings.Add("Set NWL_GlobalDMAIntEnable");
            //            }


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

            IntPtr DDR3memBuf = IntPtr.Zero; ;
            Win32.AllocateBuffer(ref DDR3memBuf, ByteTransferLen);
            Marshal.Copy(DDR3writeBuff, 0, DDR3memBuf, (int)ByteTransferLen);
            status = Win32.ThorDAQAPIWriteDDR3(0, DDR3memBuf, DDR3address, ByteTransferLen);
            if (status != 0)
            {
                ReturnStrings.Add("ThorDAQAPIWriteDDR3 ERROR");
            }
            else
            {
                ReturnStrings.Add("SUCCESS");
            }
            Win32.FreeBuffer(DDR3memBuf);
            return ReturnStrings;
        }

        // format the DDR3 memory bytes read back
        // we have unlimited read size, but we should not display more than 8k of data
        static public List<string> format16ByteLinesOnConsole(ulong startingOffset, Byte[] databytes, uint len)
        {
            List<string> ReturnStrings = new List<string> { };

            ReturnStrings.Add(string.Format("Offset (h)  00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F", 1));// suppress "<<<"
            // update line at a time...
            ulong ByteArrayOffset = 0;
            string thisLinePrefix, finalLine = "";
            // LIMIT the number of lines we print to avoid hanging the process
            if(len > (8 * 1024))
            {
                finalLine = "(truncated @ 8k)";
                len = (8 * 1024);
            }
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
            if( finalLine != "") ReturnStrings.Add(finalLine);
            return ReturnStrings;
        }



        public static string ByteArrayToString(byte[] ba)
        {
            return BitConverter.ToString(ba).Replace("-", "");
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



