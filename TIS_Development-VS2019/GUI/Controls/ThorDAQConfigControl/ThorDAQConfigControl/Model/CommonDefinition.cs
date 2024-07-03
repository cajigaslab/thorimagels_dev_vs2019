using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThorDAQConfigControl.Model
{
    public static class CommonDefinition
    {

        public static bool IsPixelDataReady
        {
            get;
            set;
        }

        public static readonly string[] THORDAQ_COMMANDLINE_COMMAND =
        {
            "Exit",
            "SelectBoard",
            "ADCSampleClock_setup",
            "ADCStream_setup",
            "ScanControl_setup",
            "BuildWaveform",
            "ReadMem",
            "WriteMem",
            "GetBoardCfg",
            "GetAllFPGAregisters",
            "GetFPGAreg",
            "SetFPGAreg",
            "APIBreakoutBoxLED",
            "APIGetAI",
            "APIGetAIOConfig",
            "APIGetBOBstatus",
            "APIGetDDR3status",
            "APIGetDIOConfig",
            "APISetAIOConfig",
            "APISetDIOConfig",
            "APISetDO",
            "APIGetLFT_JAstatus",
            "APISetLFT_JAconfig",
            "APIProgrammableTrigger",
            "APIReadEEPROM",
            "APIWriteEEPROM",
            "APIUpdateLFT_AppFirmware",
            "APIUpdateBOB_CPLD",
            "APIUpdateDAC_CPLD",
            "APIUpdateLFT_CPLD",
            "DFT",
            "UpdateCentralFPGAfirmware",
            "Load",
            "DACWaveform_setup",
            "PacketRead",
            "GlobalScan",
            "ReadDRam",
            "Clear",
            "LoadScript",
            "LiveCapture",
            "StopCapture",
            "SetParkValue",
            "Sleep",
            "S2MM_DMA_setup",
//            "WriteDram",
            "WriteSPI",
            "Delay",
            "XI2Cread",
            "XI2Cwrite"
        };

        public static string HELP_MSSAGE = @"ThorDAQ commands:
    Help | <Command Below> | -?  (Many commands have extensive help documnation, simple commands don't)
    Exit
    ADCSampleClock_setup
    ADCStream_setup
    ScanControl_setup
    S2MM_DMA_setup 
    BuildWaveform
    DACWaveform_setup 
    GlobalScan 
    ReadDDR3 <DDR3_Address> <bytecount>
    ReadMem <BarNum> <Card Offset> <Length>
    LoadScript <-f filename>
    GetBoardCfg
    GetAllFPGAregisters <-f [filename]>
    GetFPGAreg <Name> 
    SetFPGAreg <Name> <value>
    APIBreakoutBoxLED <-Label|-all> <on|off|blink>  e.g. 'APIBreakoutBoxLED -DC3 blink' makes 3rd Diagnotic LED blink
    APIGetAI  
    APIReadEEPROM [-f]                        (option -f: overwrite  c:\temp\ThorDAQeeproms.txt file)
    APIWriteEEPROM                            writes contents of c:\temp\ThorDAQeeproms.txt file
    APIUpdateDAC_CPLD                         RePrograms DAC CPLD
    APIUpdateBOB_CPLD                         RePrograms 3U BreakOutBox CPLD
    APIUpdateLFT_AppFirmware [-f {filespec}]  Update LFT (e.g. 3P) mezz. card MCU F/W Application
    APIUpdateLFT_CPLD                         RePrograms LFT's (Low Freq. Trigger, e.g. 3P) CPLD
    APIGetDIO                                 Get digital input(s) or last set output(s) (3U or DBB1 BOB)
    APIGetBOBstatus                           Full Information on BreakOutBox type & connection status
    APIGetDDR3status                          Info on DDR3 image memory card on ThorDAQ board
    APIGetDIOConfig                           Get current DIO configuration MUX
    APIGetLFT_JAstatus                        Get Version & snapshot of current JA config, LFT thresholds, freq, etc.
    APIProgrammableTrigger <chan> <assert>    Chan is -1 for ImageAcqTrigger, 0-13 for WaveformControl, Assert 1 or 0                    
    APISetAIOConfig                           Set 'slow' ADC range/polarity, etc.  
    APISetDIOConfig                           Set input/output direction and output source
    APISetDO                                  Set digital output(s) (3U or DBB1 BOB)
    APISetLFT_JAconfig                        Sets config for JA (Jitter Attenuator)
    ManufTest                                 Thorlabs manufacturing test (requires custom test bench)
    ReadMem
    UpdateCentralFPGAfirmware [-g] [-f {filespec}] (options: Golden flash image area, filespec)
    SelectBoard BoardIndx (0-based board index, max 4 boards)
    SetParkValue [-c Channel -v ParkVoltage]   Channel 0 - 11, Volts -10.0 to +10.0
    WriteDDR3 <DDR3_Address> <bytecount> <data_byte [data_byte ...] >   writes 'bytecount' bytes for one 'data_byte'
    WriteMem
    WriteSPI <index> <address> <data> %index: 0 -> clock synthsizer; 1-3 -> adc converter
    XI2Cwrite
    XI2Cread
    DFT  (Discrete Fourier Transform [eng. test])

    ThorDAQ hotkeys:
    Tab: Quick find command
    Up/Down: Quick find latest command
    Ctrl+C: Quick stop threads
    Note: Data is written and data read is displayed from low to high address order";

        public static string HELP_DACWaveform_setup_MSSAGE = @"DACWaveform_setup 
Options:
  -i               DO NOT Initialize the Waveform Playback Table (for 12 Analog + 2 Digital Waveforms)
  -o DACcnts       Offset setting (def. set to 0 DAC counts)
  -c ChannelMask   e.g. 0xF for all ABB1 outputs, 0xF16 for all ABB3 + AO5 + AO2-3, 
                   default 0x3FFF (all Analog channels A0-A11 and Digital wave channels 12,13 )
  -d DOcount       Digital OUT waveforms, 0 to disable, 1 for chan 12[DO-7], 2 for chan 12 & 13[D0-15] (def. is 2)
  -f {outFileSpec} Analog wave data file, Def. 'c:\temp\Test_Wave.txt', or if NULL, dialog box for file
  -z PlaybackFreq  Def. 1000000 (in Hz)
"
;


        public static string HELP_GlobalScan_MSSAGE = @"GlobalScan: <-r> <-s> <-d WxH>
Options:
  -n  ScanningFrameCount (number of frames to capture (def. 0xFFFF inf.)
  -r  Run (set Global STOP_RUN control bit), default
  -s  Stop (possible to exit program with Global 'scan' running, but not recommended)
  -d  Width by Height dimension, e.g. '-d 32x16' means 32-pixels by 16 lines, no 'default' defined
  -T  Trigger ManufTest PFI0, e.g. D31"
 ;

        public static string HELP_APIGetSetDIOConfig_MSSAGE = @"APIGetDIOConfig APISetDIOConfig  
APIGetDIOConfig 
APISetDIOConfig <chan> <dir> {<CpySrc>} {MUX} {AUX}
  chan    - 0-based index of the BNC DIO (e.g., 2 is D2 for 3U, DIO3 for Legacy)
  dir     - 'I' for input, 'O' for Output
  CpySrc  - 0-based index of Output channel's source (def., it's own index),
            or the FPGA's five Aux_GPIO index 'n' (only legal values 0 - 4) 
            when MUX is Aux_GPIO_n
  MUX     - Enum of 'TD_DIO_MUXedSLAVE_PORTS'; defines hardware source config as 
            FPGA-type or CPLD (def. CPLD, code 48)
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
        Aux_GPIO_0 = 0x1B, 
        Aux_GPIO_1 = 0x1C,  
        Aux_GPIO_2 = 0x1D,      // 29d
        Aux_GPIO_3 = 0x1E,
        Aux_GPIO_4 = 0x1F,
        BOB3U_GPIO = 0x30  (48d)
  AUX   GPIO AUX index for FPGA (def. unused, '-1')
Sets or Returns the current DIO config - supports both Legacy DBB1 and 3U BOB 
(Break Out Box) for DIO 0 through 31 (DBB1 designated DIO1-8 for D0-7)
Outputs can be 'sourced' from other outputs, e.g. below, D2 configured as Output and
driven by the Output value of D7 (or DIO8 if DBB1)
3U BOB I/O is TTL logic levels ( ~ under 0.76 V '0',  over 1.4 V '1')
Inputs are not multiplexed
Example use: 
APISetDIOConfig  8 O 8 48   : BNC D8 (N/A to Legacy), Output, source copied itself,
                              configured and controlled by CPLD (the default, 48)
APISetDIOConfig  0 O 0 27 0 : BNC D0 (or DIO1) Output, Out value sources itself,
                              config FPGA MUX 27, AuxGPIO index 0
APISetDIOConfig  3 O 3 29 2 : BNC D3 (or DIO4) Output, config FPGA MUX 29
                              (AuxGPIO index 2)
APISetDIOConfig  2 I 2 31 4 : BNC D2 (or DIO3) Input, config FPGA MUX 41
                              (AuxGPIO index 4)
APISetDIOConfig  17 O 1     : BNC D17, Output, source copied from D1, config MUX by CPLD (def. 48)
APISetDIOConfig  4 I 4 0    : BNC D4 (or DIO5), Input, source copy N/A, config FPGA MUX 0  
                              (iResonant_scanner_line_trigger FPGA Input)
APISetDIOConfig  16 I       : BNC D16, Input, source copy N/A, config MUX by CPLD (def. 48)
APISetDIOConfig  8 I 2 23   : INVALID config - high speed  oDigital_Waveform_13 is
                              Output, not input, and cannot be configured on 'slow' 3U Panel BNC
APISetDIOConfig 99 I        : Sets all DIOs to INPUT, or O for OUTPUT (testing only), and 
                              first 5 DIOs as AUX GPIO 0-4 (MUX 27-31)
                             
"
;

        public static string HELP_APIGetSetAIOConfig_MSSAGE = @"APIGetAIOConfig APISetAIOConfig  
APIGetAIOConfig 
APISetAIOConfig BNCindx Dir Pol <newBNCorRange>
  BNCindx - 0-based index of the BNC DIO (e.g., 2 is D2 for 3U, DIO3 for Legacy)
  Dir     - 'I' for input, 'O' for Output (NOT case sensitive)
  Pol     - 'B' for bi-polar (+/- voltage), 'U' for unipolar
  O-R     - for Output Dir, required field for DMAch switch; Input, optional Full Scale range of 5 (def. is 10 VDC)
Example use: 
APISetAIOConfig 6 I B 10   : BNC AI6 (N/A to Legacy), Input, 'b'ipolar, +/- '10' VDC range
APISetAIOConfig 13 i u 5   : BNC AI13 (N/A to Legacy), Input, 'u'nipolar,  5 VDC range (neg. volts ret. 0)
APISetAIOConfig 3 o b 9    : Switch DAC waveform that was BNC AO03 to AO09 (changes TWO AOnn definitions!)
"
;


        public static string HELP_APIGetSetDIO_MSSAGE = @"APIGetDIO APISetDO  
APIGetDIO <DBNCindex> <DBNCindex> ... <DBNCindex> (D99 gets ALL)
APISetDO <DBNCindex> <value>  <DBNCindex> <value> ... 
  DBNCindex    - 0-based index of the BNC DIO (e.g., D2 is BNC 'D2' for 3U, 'DIO3' for Legacy)
  value        - 0 or 1
Example:
  APISetDO D3 1 D17 0 D0 1  -- Sets D3 and D0, clear D17
 ";

        public static string HELP_APIGetAI_MSSAGE = @"APIGetAI <-option> <BNClabel>
Options:
  -r   Raw ADC counts (12-bit, 2-s complement), def. is Volts
Example use: 
  APIGetAI         Read Volts for all AIs (def.)
  APIGetAI -r AI6  Return ADC counts read from first MAX127 channel (AI6)
"
;
        // Manufacturing Tests using the National Instruments PXIe-1071
        public static string HELP_ManufTest_MSSAGE = @"ManufTest <-t TestCode> <-a> <-u>
Options:
(NOTE: capital letters mean issue single TCP/IP CMD:, then exit - for TCP debug):
  -B  BOARDSN (Get Board serial numbers) 
  -T  1 UnitTester (REQUIRED test number & description)
  -P  POLLING
  -F  SENDFILE (Get Labview data .csv file)

  -a IPaddress of Test Computer (e.g. NIPXIe1071), (def. 10.28.10.191)
  -p IP Port number of Test Computer (def. 80)
  -r Generate .XLS format report file
  -t TestCode, 0 for all, 1 for UnitTest, 2 for Exemplar Waveform, 3 for DAC, 4 for ADC,
       5 for TRIG3P, 6 for DBB1, 7-9 for ABB1-3
  -TDboard Read/Test Texas Instruments UDC9248 Power Chips on ThorDAQ main PMbus thru TI USB Adapter
  -DACcard n Test ABBx sine waveform on channel index 'n', e.g. '-DACw 11' tests AO11 (ABB3)
  -ADCcard n Test sine waveform on PMT channel index 'n', e.g. '-ADCw 1' tests Chan B
  -ABBx      Test breakout box 'x' (e.g. -DBB1 or -ABB2 for Analog BB 2, etc)  
  -all       Do ALL available manufacturing tests
"
    //  - DACl Light LEDs on all channels(BBoxes) of DAC card, on/off cycle

    ;

        public static string HELP_S2MM_DMA_setup_MSSAGE = @"S2MM_DMA_setup: <-d WxH> ...
DMA setup of all four image channels is in three phases:  
    1. Configure the S2MM FPGA to DDR3 BRAM descriptors (both banks)
    2. Write the sample LUT (Look-Up Table)
    3. Enable the S2MM Interrupts which trigger NWA common User_Interrupt
There is one Win O/S device driver vector for all interrupts, including
NWA read to DDR3, NWA write to DDR3, and FPGA (ThorDAQ) user int.
Options:
  -d  Width by Height dimension, e.g. '-d 32x16' means 32-pixels by 16 lines; def. 32x32
  -c  Channel mask (e.g. 0x6 for channels 1&2, 0xF for chan 0-3), def. 0x1
  -a  Physical starting address of DDR3 (def. 0x0)
  -n  Number of descriptors (or 'frames') per bank (two banks), def. 3
";

        public static string HELP_ADCSampleClock_setup_MSSAGE = @"ADCSampleClock_setup:  
ADC (PMT) Sampling Clock Generation setup for 3P (low frequency) or 2P/Confocal (~80 MHz freq)
and PLL/MMCM signal selection (ENABLE/DISABLE) if using external LASER source.  Otherwise,
use internally generated (Confocal) clock source.
Options:
  -c  Confocal (Internal) Synchronized Sampling @160MHz from internal 80MHz osc. (def)
  -2  Use 2P LASER Sync (from ADC mezzanine card 'REF' connector)
  -3  Use 3P LASER Sync (from 3P mezzanine card)
";

        public static string HELP_ADCStream_setup_MSSAGE = @"SC_setup: <-d WxH> <-n NumDescriptors> <-b bank> 
ADC Streaming setup for Filters, Coefficients, Lookup Table (LUT)
Options:
  -r  Resonant-Galvo mode (def. is Galvo-Galvo mode)
  -l  Low Frequency 3P Sampling (def. is 2P/Confocal ~80MHz ref)
  -j  Configure (by FPGA) and Release JESD204B cores
  -g  Gain (attenuation) 0 = 1dB, 1 = 5dB ... 7 = 29dB (def. 0)
";

        public static string HELP_ScanControl_setup_MSSAGE = @"ScanControl_setup: 
Scan Control subsystem
Options:
  -m  Image Synchronization Master Timing Mode (0 = Res-Galvo, 1 = Galvo-Galvo, 2= External  <def. Galvo-Galvo>)
  -p  'ScanningGalvoPixelDwell', BAR3 0x0160 (Galvo-Galvo mode only) (def. 10)
  -P  'ScanningGalvoPixelDelay', BAR3 0x0168 (Galvo-Galvo mode only) (def. 1)
  -l  'ScanningIntraLineDelay',  BAR3 0x0170 (def. 12)
  -f  'ScanningIntraFrameDelay', BAR3 0x0178 (def. 25)
  -B  BiDirectional mode 'BIDIR_SCAN_MODE' (def. single direction)
";

        public static string HELP_ReadMem_MSSAGE = @"ReadMem BAR BARoffset LengthInBytes 
Usage Example:
To read 16 zeros to BAR 1, BarOffset 0x20:
   readmem 1 0x20 16 "
;

        public static string HELP_WriteMem_MSSAGE = @"WriteMem BAR# BARoffset LengthInBytes ByteString
Usage Example:
(HEX 0xNN.. format only) starting at same destination - data shows address order):
Write single value, starting at Address, to contiguous byte locations:
To write 16 zeros to BAR 1, BarOffset 0x4000:
   writemem 0 0x4000 16 0x00
Write string of bytes starting at Address, LSB to MSB from left to right:
To write 4 bytes 
   writemem 1 0x10000 4 0xFEED2189"
;

        public static string HELP_XI2CReadWrite_MSSAGE =
@"
Debugging tool to be used with I2C protocol analyzer... reports only errors, not data
Use SWUG I2C Map for all possible Slave devices
Options:
  -s Slow speed (100 Hz), def. is 400 Hz
  -o Hex OpCode bytes to WRITE prior to the READ (i.e. command bytes)
  -p Page size (i.e. for writing) - def. is 16
  -n Number of bytes we expect to READ
  -d Hex data bytes to WRITE
Examples:
XI2Cread  MasterMuxChan SlaveMuxChan TargetSlaveI2CAddress -o OpCode0 ... OpCodeN -n ByteCount
XI2Cwrite MasterMuxChan SlaveMuxChan TargetSlaveI2CAddress -o OpCode0 ... OpCodeN  
    (does NOT send I2C stoP bit)
XI2Cwrite MasterMuxChan SlaveMuxChan TargetSlaveI2CAddress -d Data0 Data1 ... DataN
    (sends I2C stoP bit)
Read/Write with AIX (Xilinx) I2C Master (see I2C Device Map in SWUG for MUX/Address config)
XI2Cread MasterMuxChan SlaveMuxChan TargetSlaveI2CAddress -o byte0 byte1 ... byteN -n ByteLen
Usage Examples:  
XI2Cwrite  0x08 0x1 0x3C -s -d 0x0 0x1   
  XI=> MasterMUX chan 0x8, SlaveMUX 0x1, DevAddr 0x3c (IS31FL3236), 100 Hz
    Reg 0 (Shutdown Cmd), Data 1 (enable
XI2Cread  0x08 0xFF 0x54 -o 0x0 -n 28  
  XI=> MasterMUX chan 0x80, No SlaveMUX, DevAddr 0x54 (TDboard), StartIndx 0x0, 
    read 28 data bytes SN @0x0 
XI2Cread  0x08 0x08 0x50 -o 0x0 -n 28  
  => MasterMUX chan 0x08, SlaveMUX 0x08, DevAddr 0x50 (ABB3), read 28 data bytes SN @0x0 
XI2Cread  0x08 0xff 0x40 -o 0xC0 0 0 0 -n 4  (Reads DAC CPLD USERCODE)
";

        public static string HELP_UpdateLFT_AppFirmware_MSSAGE = @"APIUpdateLFT_AppFirmware  <-o> <-F> <-f filename>
Update SAMD21 Firmware on 3P clock/jitter attentuation mezzanine card
Options:
  -o           Restore 'original' .HEX file included with this software package distribution (if found)
  -F           Forces a legacy firmware load @ 0x8000; FPGA download compatible load starts @ 0x2000
  -C           Update the LFT_JA (3P) App CPLD with .jed file
  -f filespec  Specify a full filespec to the file
NOTE!
  The 'legacy' bootloader which was distributed to R&D customers used the USB interface; you identify
  this bootloader version by inspecting 2 LEDs on 3P board (above USB connector), which will be OFF if 
  no bootloader exists or if legacy bootloader is running, waiting for USB command for App flash.
  The I2_SLAVE_BOOTLOADER.HEX program, when starting after power cycle, briefly lights both
  RED and GREEN LEDs (about 1 second), then rapidly flashes green (about 5x per sec)
  To install I2C_SLAVE_BOOTLOADER for first time:
  1. Connect Segger J-Link probe to 3P board (above USB connector, not to lower right of connector)
  2. Using Atmel Studio, connect to SAMD21 MCU and make sure 'fuse' BOOTPROT unprotects all NVRAM
  3. ERASE entire NVRAM chip
  4. Flash I2C_SLAVE_BOOTLOADER.HEX
  5. Run ThorDAQ-CL_GUI utility version 2.1.x.x or higher, run 'Update3PFirmware'
  6. Load CLKRX_3P or other CLKRX_xx.HEX image with I2C slave support
  
  Loading a LEGACY version of clk_rx_rev2.hex at NVRAM 0x8000 will prevent future I2C updates - a 
  warning message cautions, and -F flag allows you to overwrite
";
        
        public static string HELP_APIgetsetLFT_JA_MSSAGE = @"APIgetLFT_JAstatus:  returns ALL params
  Example:
APIgetLFT_JAstatus                     returns ALL available params, i.e.
    MGMSG_HW_REQ_INFO
    MGMSG_CLK_REQ_INPUT_THRESH
    MGMSG_CLK_REQ_INPUT_SELECT

APIsetLFT_JAconfig: <PARAM> <VALUE>    sets named param
    -CSIS {d | 1 | 2}    Sets MGMSG_CLK_SET_INPUT_SELECT to disable, LF0, LF1
    -CSIT {-LFn -a | <-m <NegThresh PosThresh>}  Sets auto_tracking (def.), if manual set thresholds 
                                                 MGMSG_CLK_SET_INPUT_THRESH for LF0 or LF1
e.g., to set both LF0 and LF1 thresholds:
    -CSIT -LF0 -m 2000 2014 -CSIT -LF1 -m 2001 2015

(see 'ThorDAQ 3P Mezzanine Card Firmware.docx')
";

        public static string HELP_APIReadWriteEEPROM_MSSAGE = @"APIReadEEPROM <-f> <-c COMP>
APIWriteEEPROM <-f filespec> [write all EEPROMs with file contents]
  where COMP is a 4-digit alphanumeric ThorDAQ component:
    TD   
    ADC
    DBB1
    DAC
    ABB1
    ABB2
    ABB3
    LFT
  and String:
    TD302025-039044-0002-10/02/2018 (ex. TD main board)
    DB002025-b30274-0010-07/01/2019 (ex. DAC card)
    TR002025-g39044-0012-01/01/2022 (ex. 3P trigger card)
    AB202025-x39044-0012-01/01/2022 (ex. ABB2 BOB)

String fields:  
    TD3:    Component code - ThorDAQ main board type TDQ 3, (1 for TDQ1, etc)
    02025:  the ThorDAQ main board Serial Num
    -0      for mezzanine cards, 'x' means untested/unknown, 'g' or 'b' means last tested good or bad on ThorDAQ S/N
    -n39044  board manufacturer batch number
    -0002    board manufacturer serial num in batch
    -10/02/2018  board date of manufacture
"
;

        public static string HELP_BuildWaveform_MSSAGE = @"BuildWaveform: <-f int> <-l int> <-h int> <-a int> <-r int> <-t float> <-s int>
Options:
  -C  DACcounts - def 65536 cnts (16-bit resolution)
  -D  DIGITAL waveform (for playback on DAC channels 12 + 13 only)
  -d  Digital waveform duty cycle, 6-96, percent of period signal is high
  -V  Volts range peak to peak - def. 20 VDC (i.e. +/- 10 VDC)
  -f  outputFileSpec (def. 'c:\temp\Test_Wave.txt', for Digital c:\temp\DAC_DOtest_wave.txt)
  -l  lowAmplitude  (def. 0 DAC cnts)
  -h  highAmplitude (def. DACcounts-1)
  -a  peak to peak amplitude (center around [DACcounts/2] DAC cnts, e.g. ~0 VDC)
  -r  frequency (def. 14 Hz)
  -t  totalPlaybackTime (def. 1.0 sec)
  -s  sample playback fequency (def. 1,000,000 sample/sec (fastest for FPGA DAC) , slowest 3,050) 
  -v  output file in units of Volts (def. DAC counts) AND in NI1071 format of single line with CSV voltage values 
      (def. output is 16-bit DAC counts, 1 sample/line )"
;
    }
}
