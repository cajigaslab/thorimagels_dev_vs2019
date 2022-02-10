#if __LINE__
#define public
#else
namespace ThorSharedTypes
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;
#endif
    public enum SelectedHardware
    {
        SELECTED_XYSTAGE,
        SELECTED_CAMERA1,
        SELECTED_ZSTAGE,
        SELECTED_ZSTAGE2,
        SELECTED_BEAMSTABILIZER,
        SELECTED_EMISSION,
        SELECTED_DICHROIC,
        SELECTED_SHUTTER1,
        SELECTED_TURRET,
        SELECTED_BFLAMP,
        SELECTED_AUTOFOCUS,
        SELECTED_CONTROLUNIT,
        SELECTED_PMT1,
        SELECTED_PMT2,
        SELECTED_PMT3,
        SELECTED_PMT4,
        SELECTED_POWERREGULATOR,
        SELECTED_POWERREGULATOR2,
        SELECTED_BEAMEXPANDER,
        SELECTED_PINHOLEWHEEL,
        SELECTED_LASER1,
        SELECTED_LASER2,
        SELECTED_LASER3,
        SELECTED_LASER4,
        SELECTED_SLM,
        SELECTED_RSTAGE,
        SELECTED_PMTSWITCH,
        SELECTED_BLEACHINGSCANNER,
        SELECTED_EPHYS,
        SELECTED_LIGHTPATH,
        SELECTED_SPECTRUMFILTER,
        SELECTED_EPITURRET
    };

    public enum PreCaptureStatus
    {
        PRECAPTURE_BLEACHER_IDLE,					//bleacher is not running, and is available for waveform load
        PRECAPTURE_BLEACHER_ERROR,					//bleacher has error at running the waveform
        PRECAPTURE_WAVEFORM_LOADING,				//waveform file is creating/loading into bleacher		
        PRECAPTURE_WAVEFORME_DONELOAD,				//waveform file is created/loaded into bleacher
        PRECAPTURE_WAVEFORM_MID_CYCLE,		        //waveform file is to be loaded without complete cycle line
        PRECAPTURE_WAVEFORM_LAST_CYCLE,	            //allows bleacher interaction on the last cycle
        PRECAPTURE_DONE								//all tasks are done, ready to leave
    };

    // waveform digital buffer, line names in order
    public enum BLEACHSCAN_DIGITAL_LINENAME
    {
        DUMMY = 0,									//line trigger
        POCKEL_DIG,
        ACTIVE_ENVELOPE,
        CYCLE_COMPLETE,
        CYCLE_ENVELOPE,
        ITERATION_ENVELOPE,
        PATTERN_TRIGGER,
        PATTERN_COMPLETE,
        EPOCH_ENVELOPE,
        CYCLE_COMPLEMENTARY = 9,
        POCKEL_DIG_1,
        POCKEL_DIG_2,
        POCKEL_DIG_3,
        DIGITAL_LINENAME_LAST
    };

    public enum Constants
    {
        AREA_UNDER_CURVE = 2,
        GALVO_DATA_POINT_MULTIPLIER = 2,
        MAX_WIDEFIELD_WAVELENGTH_COUNT = 2,
        MAX_IMG_DIG_LINE_COUNT = 3,
        MAX_GG_POCKELS_CELL_COUNT = 4,
        DEFAULT_FILE_FORMAT_DIGITS = 4,
        MAX_FILE_FORMAT_DIGITS = 6,
        BITS_PER_BYTE = 8,
        ACTIVE_LOAD_BLKSIZE_DEFAULT = 8,
        MAX_MULTI_AREA_SCAN_COUNT = 10,
        PIXEL_X_MIN = 32,
        ACTIVE_LOAD_UNIT_SIZE = 100,
        HUNDRED_PERCENT = 100,
        GALVO_MIN_RETRACE_TIME = 200,
        TURRET_FOCALLENGTH_MAGNIFICATION_RATIO = 200,
        EPHYS_ARRAY_SIZE = 260,
        DEFAULT_PIXEL_X = 512,
        DEFAULT_GALVO_HZ = 1000,
        MS_TO_SEC = 1000,
        UM_TO_MM = 1000,
        KHZ = 1000,
        TIMEOUT_MS = 1500,
        EVENT_WAIT_TIME = 5000,                     //[ms]
        UM_PER_INCH = 25400,
        MHZ = 1000000,
        US_TO_SEC = 1000000,
        M_TO_UM = 1000000
    };

    public enum GlobalExpAttribute
    {
        GALVO_CALIBTATION = 0,
        GALVO_BLEACH = 1,
        SLM_BLEACH = 2,
        OTM = 3,
        SLM_ZREF = 4,
        LAST
    };

    public enum PockelsResponseType
    {
        SINE_RESPONSE = 0,
        LINEAR_RESPONSE = 1,
        LAST_POCKELS_RESPONSE
    };

    public enum ZStageType
    {
        STEPPER = 0,
        PIEZO
    };

    public enum BufferType
    {
        INTENSITY,
        DFLIM_IMAGE,
        DFLIM_HISTOGRAM,
        DFLIM_IMAGE_SINGLE_PHOTON,
        DFLIM_IMAGE_ARRIVAL_TIME_SUM,
        DFLIM_PHOTONS,
        DFLIM_ALL,
        DFLIM_DIAGNOSTIC
    };

    public enum ZPiezoAnalogMode
    {
        ANALOG_MODE_SINGLE_POINT = 0,
        ANALOG_MODE_SINGLE_WAVEFORM = 1,
        ANALOG_MODE_STAIRCASE_WAVEFORM = 2,
        ANALOG_MODE_LAST
    };

    public enum SettingsFileType
    {
        SETTINGS_FILE_FIRST = 0,
        ACTIVE_EXPERIMENT_SETTINGS = 0,
        APPLICATION_SETTINGS = 1,
        HARDWARE_SETTINGS = 2,
        SETTINGS_FILE_LAST
    };

    public enum CaptureModes
    {
        T_AND_Z = 0,
        STREAMING = 1,
        //TDI = 2,
        BLEACHING = 3,
        HYPERSPECTRAL = 4
    };

    public enum CaptureFile
    {
        FILE_TIFF = 0,
        FILE_RAW = 1,
        FILE_BIG_TIFF
    };

    public enum EPhysTriggerMode
    {
        NONE = 0,
        FRAME = 1,
        LINE = 2,
        ZSTACK = 3,
        CAPTURE = 4,
        STIMULATION = 5,
        MANUAL = 6,
        CUSTOM = 7,
        EPHYS_LAST_TRIGGER_MODE
    };

    public enum EPhysOutputType
    {
        DIGITAL_ONLY = 0,
        ANALOG_ONLY = 1,
        BOTH = 2,
        EPHYS_LAST_OUTPUT_TYPE
    };

    /// <summary>
    /// signal types for analog or digital lines
    /// </summary>
    public enum SignalType
    {
        ANALOG_XY = 0,
        ANALOG_POCKEL = 1,
        DIGITAL_LINES = 2,
        ANALOG_Z = 3,
        SIGNALTYPE_LAST
    };

    /// <summary>
    /// Thordaq signal types for analog or digital lines
    /// </summary>
    public enum SignalTypeThorDAQ
    {
        TDQANALOG_X = 0,
        TDQANALOG_Y = 1,
        TDQANALOG_POCKEL = 2,
        TDQDIGITAL_LINES = 3,
        TDQANALOG_Z = 4,
        TDQSIGNALTYPE_LAST
    };


    /// <summary>
    /// status type for both ICamera and IDevice
    /// </summary>
    public enum StatusType
    {
        STATUS_BUSY = 0,
        STATUS_READY = 1,
        STATUS_ERROR = 2,
        STATUS_PARTIAL = 3
    };

    public enum ExperimentModes
    {
        EXP_SETUP = 0,
        EXP_CAPTURE
    };

    public enum OutOfRangeColors
    {
        COLOR_TRANSPARENT = 0,
        COLOR_GREEN = 1,
        COLOR_YELLOW = 2,
        COLOR_RED = 3
    };

    public enum ThorPipeCommand
    {
        Establish,
        TearDown,
        AcquireInformation,
        UpdataInformation,
        FilePath,
        StartAcquiring,
        StopAcquiring,
        StartBleach,
        StopBleach,
        Receive,
        Error,
        ChangeRemotePC,
        ChangeRemoteApp
    };

    public enum ThorPipeStatus
    {
        ThorPipeStsNoError = 0,
        ThorPipeStsBusy = 1,

        ThorPipeFormatError = 10,
        ThorPipeFormatRoutingError = 11,

        ThorPipeError = 99,
    };

    public enum MesoScanTypes
    {
        Meso = 1,
        Micro = 2
    };

    public enum ScopeType
    {
        UPRIGHT = 0,
        INVERTED = 1
    };

    public enum WaveformDriverType
    {
        WaveformDriverFirst = 0,
        WaveformDriver_NI = 0,
        WaveformDriver_ThorDAQ,
        WaveformDriverLast = 1
    };

    public enum WAVEFORM_FILETYPE
    {
        H5 = 0,
        MEMORY_MAP = 1,
        CSV = 2,
        LAST_FILE_TYPE
    };

    public enum AutoFocusTypes
    {
        AF_HARDWARE,
        AF_HARDWARE_IMAGE,
        AF_IMAGE,
        AF_NONE
    };

    public enum AutoFocusStatusTypes
    {
        NOT_RUNNING,
        STOPPED,
        COARSE_AUTOFOCUS,
        FINE_AUTOFOCUS,
        HARDWARE_AUTOFOCUS
    };

    public enum ShutterState
    {
        SHUTTER_OPENED = 0,
        SHUTTER_CLOSED = 1
    };

    // ThorDAQ BreakOutBox (BOB) status defines
    public enum TD_BOBstatusDef : int // 
    {
        BOBtypeCharLen = 13, // e.g. "3U Panel BOB"
        BOB_DBB1_SN    = 29, // e.g. DB100105225601014205/10/2019 (defined field as of Oct-2021 28 chars)
        BOB_ABB1_SN = 29, // e.g. AB1...
        BOB_ABB2_SN = 29, // e.g. AB2...
        BOB_ABB3_SN = 29, // e.g. AB3...
        BOB_status =  32, // e.g. "CABLE SWAPPED", "no CPLD program", "Missing DIO/AIO", "OK"
        DAC_CPLD    = 16,  // e.g. "DACCPLD 1.0.0.1" or "DACCPLD error"
        // valid for 3U BOB only...
        BOB_CPLD = 16,     // e.g. BOB...
        BOB_ALL = BOBtypeCharLen + (4*BOB_DBB1_SN) + BOB_status + (2* DAC_CPLD)
    };
    // better to make ALL Record "CharPerBOB_xx" lengths the same
    public enum TD_BOBAODef : int // 
    {
        NumBOB_AOs = 12, // 12 legacy "fast" (via ThorDAQ mezzanine card & HDMI cables)
        CharPerBOB_AO = (12 + 1) // field width plus a NULL e.g. "AnnXDVmmMcc"
    };
    public enum TD_BOBAIDef : int // 
    {
        NumBOB_AIs = 14, // 6 legacy "fast", plus on 3U Panel 8 "slow" via I2C
        CharPerBOB_AI = (12 + 1) // field width plus a NULL
    };
    public enum TD_BOBDIODef : int // DnnXccMmmAjj, 'nn' is BNClabel index, 'X' is 'I' or 'O' Dir, 'mm' is MUX code (48 for CPLD), 'jj' is FPGA AUX_GPIO index (or -1)
    {
        NumBOB_DIOs = 32,
        CharPerBOB_DIO = (12 + 1) // field width plus a NULL
    };

    // ThorDAQ DDR3 memory card EEPROM (single byte fields unless noted), see JEDEC Serial Presence Data (SPD) for format
    public enum TD_DDR3_SPD : int
    {
        Bytes_Used = 0,   Bytes_Used_LEN = 16,  // e.g. "BytesUsed 0x92;"
        SPD_Rev    = 1,   SPD_Rev_LEN = 16,
        MFR_Year = 120,   MFR_Year_LEN = 16,
        MFR_Week = 121,   MFR_Week_LEN = 16,
        DDR3SerialNum = 122, DDR3SerialNum_LEN = 22, // e.g. "SerialNum 1234567890;" , 4 byte field
        DDR3PartNum = 128, DDR3PartNum_LEN = 27,     // e.g. "8KTF25664HZ-1G6k1;"  , 17 byte field
        DDR3_SPD_LEN = Bytes_Used_LEN + SPD_Rev_LEN + MFR_Year_LEN + MFR_Week_LEN + DDR3SerialNum_LEN + DDR3PartNum_LEN,
        SPD_FieldCount = 6  // number of fields we are decoding (there are dozens in total)
    };

    // definitions of the "Slave address" (actually DIO MUX codes) I/O which can be assigned to 
    // DIO_MUX_REG DIOx physical connections (must duplicate to SharedTypes in CL-GUI)
    // prefixes "i" and "o" imply FPGA defined "fast" I/Os configured prior to GlobalScan start,
    // controlled by or critical to FPGA/System operation at GlobalScan start time.
    // Other DIO imply "slow" DIO configured and controlled by CPLD (via I2C),
    // NOT dependent on GlobalScan status
    public enum TD_DIO_MUXedSLAVE_PORTS : int
    {
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
        // "Digital_Waveform" (13th channel) programmed to coordinate with Galvo DAC waveforms, asserted when "GlobalScan" is set
        oDigital_Waveform_0 = 0x0A,
        oDigital_Waveform_1 = 0x0B,
        oDigital_Waveform_2 = 0x0C,
        oDigital_Waveform_3 = 0x0D,
        oDigital_Waveform_4 = 0x0E,
        oDigital_Waveform_5 = 0x0F,
        oDigital_Waveform_6 = 0x10,
        oDigital_Waveform_7 = 0x11,
        // "Digital_Waveform" (14th channel) (note hardware limitations of only 8 high speed DOs total, only 6 in practice)
        oDigital_Waveform_8 = 0x12,
        oDigital_Waveform_9 = 0x13,
        oDigital_Waveform_10 = 0x14,
        oDigital_Waveform_11 = 0x15,
        oDigital_Waveform_12 = 0x16,
        oDigital_Waveform_13 = 0x17,
        oDigital_Waveform_14 = 0x18,
        oDigital_Waveform_15 = 0x19,
        oCapture_Active = 0x1A, // 26d
        Aux_GPIO_0 = 0x1B,   // General Purpose NOT related to "GlobalScan"-enable status, separate REGISTER for DIR
        Aux_GPIO_1 = 0x1C,
        Aux_GPIO_2 = 0x1D, // 29d
        Aux_GPIO_3 = 0x1E,
        Aux_GPIO_4 = 0x1F,
        BOB3U_GPIO = 0x30  // "slow" general purpose GPIO D8-D31 on 3U Panel, DIR configured and I/O value through CPLD via I2C
    };

    public enum BleachMode
    {
        BLEACH = 0,
        SLM = 1
    };

    // ThorDAQ LEDs on Breakout Boxes
    // Redefined to support both DBB1/ABBx and 3U Panel
    // for Legacy nBBx, the four (4) 9554 LED controllers have single 8-bit register for all LEDs
    // for 3U Panel, there are two (2) LS31FL LED controllers, 2 registers per single LED "channel"
    public enum BBoxLEDenum : int
    {
        D0 = 0, // i.e. DIO1 on Legacy BOB
        D1,
        D2,
        D3,
        D4,
        D5,
        D6,
        D7,     // or DIO8
        // D8 and above only on 3U Panel
        D8,
        D9,
        D10,
        D11,
        D12,
        D13,
        D14,
        D15,
        D16,
        D17,
        D18,
        D19,
        D20,
        D21,
        D22,
        D23,
        D24,
        D25,
        D26,
        D27,
        D28,
        D29,
        D30,
        D31,
        DC1, // 6 diagnostic code LEDs
        DC2,
        DC3,
        DC4,
        DC5,
        DC6,
        AO0,
        AO1,
        AO2,
        AO3,
        AO4,
        AO5,
        AO6,
        AO7,
        AO8,
        AO9,
        AO10,
        AO11,
        AI0,
        AI1,
        AI2,
        AI3,
        AI4,
        AI5,
        AI6,
        AI7,
        AI8,
        AI9,
        AI10,
        AI11,
        AI12,
        AI13,  // "last" enum value - used in logic

        ALL = 0xFF,
        all = 0xff,
    };


    // 3U Panel 
    // 12 - Analog Outputs
    // 14 - Analog Inputs 
    // 32 - Digital I/O
    // 6  - Diagnostic LEDs
    // the 'c' is for chip
    public enum TD_3UchipU1BOB_LEDs : int
    {
        // PCB Chip U1
        cDC1 = 24,   // OUT24, RENAME
        cDC2 = 23,   // OUT23
        cDC3 = 22,   // OUT22 
        cDC4 = 21,   // OUT21
        cDC5 = 20,   // OUT20
        cDC6 = 19,   // OUT19
        cAO0 = 31,  // OUT31
        cAO1 = 30,
        cAO2 = 29,
        cAO3 = 28,
        cAO4 = 27,
        cAO5 = 26,
        cAO6 = 32,
        cAO7,      // OUT33
        cAO8,
        cAO9,
        cAO10,     // OUT36
        cAO11 = 25,// OUT25
        cAI0 = 7,  // OUT7
        cAI1 = 6,
        cAI2 = 5,
        cAI3 = 4,
        cAI4 = 3,  // OUT3
        cAI5 = 14, // OUT14
        cAI6 = 8,  // OUT8
        cAI7,
        cAI8,
        cAI9,
        cAI10,
        cAI11,      // OUT11
        cAI12 = 15, // OUT15
        cAI13
    };

    public enum TD_3UchipU2BOB_LEDs : int
    {
        // 3U Panel enums follow OUTcc (cc is 1-based index) on PCB schematic
        // PCB Chip U2
        cD7 = 29,  // OUT29
        cD6,
        cD5,
        cD4,
        cD3,
        cD2,
        cD1,
        cD0,       // OUT36
        cD11 = 24, // OUT24
        cD12,
        cD13,
        cD14,
        cD15,
        cD8 = 23, // OUT23 
        cD9 = 22,
        cD10 = 21,
        cD16 = 11, // OUT11
        cD17,      // OUT12
        cD18,
        cD19,
        cD20,
        cD21,
        cD22 = 19, // OUT19
        cD23,      // OUT20
        cD24 = 3,  // OUT3
        cD25,
        cD26,
        cD27,     // OUT6
        cD28,
        cD29,
        cD30,
        cD31      // OUT10
    };

    // Legacy BOB is DEPRECATED
    public enum TD_LegacyBOB_LEDs : int
    {
        // Breakout Box LED control (Legacy - DBB1 and ABBx)
        lDIO1 = 1,  // DBB1 Legacy BOB Label enums
        lDIO2,
        lDIO3,
        lDIO4,
        lDIO5,
        lDIO6,
        lDIO7,
        lDIO8,
        lAO1,  // 9
        lAO2,
        lAO3,
        lAO4,
        lAI1,
        lAI2,
        lAO5,
        lAO6,
        lAO7,
        lAO8,
        lAI3,
        lAI4, // 20
        lAO9,
        lAO10,
        lAO11,
        lAO12,
        lAI5,
        lAI6 // 26
    };

    public enum DetectorTypes
    {
        PMT_OLD_BOOTLOADER = 0,
        PMT1000 = 1,    //BIALKALI (H10721)
        PMT2100 = 2,    //GaAsP compact (H10770PA-40)
        PMT2106 = 3,    //GaAsP with shutter (11706P-40)
        APD = 4,        //not used
        PHOTODIODE = 5, //not used
        HPD1000 = 6,    //(R11322U-40)
        SIPM100 = 7,
        PMT2110 = 8,    //Higher BW version of PMT2100. Higher speed with 180MHz
        PMT3100 = 9,    //GaAsP 3P
        GAASP_TRIP = 10,//not used yet, but should be setup if wee need software trip at any point
        PMT_TYPE_NOT_SET = 11,
        DIF_TYPE_OF_PMT = 12,
        LAST_TYPE
    };

    public enum DetectorBandwidths
    {
        BW_250kHz = 250000,
        BW_2_5MHz = 2500000,
        BW_15MHz = 15000000,
        BW_30MHz = 30000000,
        BW_80MHz = 80000000,
        BW_200MHz = 200000000,
        BW_300MHz = 300000000,
    };

#if __LINE__
#undef public
#else
}
#endif