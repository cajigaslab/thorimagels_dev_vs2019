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

#if __LINE__
#undef public
#else
}
#endif