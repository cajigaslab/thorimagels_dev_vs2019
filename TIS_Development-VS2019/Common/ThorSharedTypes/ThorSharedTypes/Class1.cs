namespace ThorSharedTypes
{
    using System;
    using System.Collections.Concurrent;
    using System.Collections.Generic;
    using System.Collections.Specialized;
    using System.ComponentModel;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Threading.Tasks;
    using System.Xml;
    using System.Xml.Linq;

    #region Enumerations

    public enum DeviceType : uint
    {
        DEVICE_TYPE_FIRST = 0,
        SHUTTER = 0x00000001,
        BEAM_STABILIZER = 0x00000002,
        STAGE_X = 0x00000004,
        STAGE_Y = 0x00000008,
        STAGE_Z = 0x00000010,
        AUTOFOCUS = 0x00000020,
        LAMP = 0x00000040,
        FILTER_WHEEL_EM = 0x00000080,
        FILTER_WHEEL_DIC = 0x00000100,
        TURRET = 0x00000200,
        PMT1 = 0x00000400,
        PMT2 = 0x00000800,
        POWER_REG = 0x00001000,
        BEAM_EXPANDER = 0x00002000,
        LASER1 = 0x00004000,
        LASER2 = 0x00008000,
        LASER3 = 0x00010000,
        LASER4 = 0x00020000,
        PINHOLE_WHEEL = 0x00040000,
        PMT3 = 0x00080000,
        PMT4 = 0x00100000,
        SLM = 0x00200000,
        STAGE_R = 0x00400000,
        PMT_SWITCH = 0x00800000,
        EPHYS = 0x01000000,
        LIGHT_PATH = 0x02000000,
        PMT5 = 0x04000000,
        PMT6 = 0x08000000,
        STAGE_Z2 = 0x10000000,
        CONTROL_UNIT = 0x20000000,
        SPECTRUM_FILTER = 0x40000000,
        POWER_REG2 = 0x80000000,
        DEVICE_TYPE_LAST
    }

    public enum HWType : uint
    {
        Device = 0,
        Camera = 1
    }

    /// <summary>
    /// OverlayManager: 
    /// </summary>
    public enum Mode
    {
        STATSONLY = 0,
        PATTERN_NOSTATS,
        PATTERN_WIDEFIELD,
        MICRO_SCANAREA,
        LAST_MODE
    }

    public enum RangeEnum
    {
        NO_COLOR = 0,
        GREEN,
        YELLOW,
        RED
    }

    public enum SampleType
    {
        WELL6,
        WELL24,
        WELL96,
        WELL384,
        WELL1536,
        SLIDE
    }

    /// <summary>
    /// OverlayManager: MODE was attached with version at highest byte (SecA)
    /// </summary>
    public enum Tag
    {
        MODE = 0,
        ROI_ID,
        PATTERN_ID,
        FLAGS,
        SUB_PATTERN_ID,
        RGB,
        WAVELENGTH_NM,
        LAST_TAG
    }

    #endregion Enumerations

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    public struct FrameInfoStruct
    {
        public Int32 imageWidth;
        public Int32 imageHeight;
        public Int32 channels;
        public Int32 fullFrame;
        public Int32 scanAreaID;
        public Int32 bufferType;
        public UInt64 copySize;
        public Int32 numberOfPlanes;
    }

    /// <summary>
    /// Identical data structure defined in ThorSharedTypesCPP.h.
    /// clockRate, analog galvo XY interleaved, analog pockels of unit length,  
    /// digital signals for multiple lines in order: 
    /// dummy,complete output, cycle envelope, iteration envelope, pattern envelope, pattern complete trigger.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct GGalvoWaveformParams
    {
        public UInt64 clockRate;
        public UInt64 analogXYSize;
        public UInt64 analogPockelSize;
        public UInt64 digitalSize;
        public UInt64 analogZSize;
        public double stepVolt;
        public byte pockelsCount;
        public byte driverType;
        public IntPtr GalvoWaveformXY;
        public IntPtr GalvoWaveformPockel;
        public IntPtr DigBufWaveform;
        public IntPtr PiezoWaveformZ;
        public int digitalLineCnt;
        public int Scanmode;
        public int Triggermode;
        public int CycleNum;
        public IntPtr bufferHandle;
        public int lastLoaded;
        public int PreCapStatus;
    }

    /// <summary>
    /// Identical data structure defined in ThorSharedTypesCPP.h.
    /// clockRate, analog galvo XY interleaved, analog pockels of unit length,  
    /// digital signals for multiple lines in order: 
    /// dummy,complete output, cycle envelope, iteration envelope, pattern envelope, pattern complete trigger.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct ThorDAQGGWaveformParams
    {
        public UInt64 clockRate;
        public UInt64 analogXYSize;
        public UInt64 analogPockelSize;
        public UInt64 digitalSize;
        public double stepVolt;
        public byte pockelsCount;
        public byte driverType;
        public IntPtr GalvoWaveformX;
        public IntPtr GalvoWaveformY;
        public IntPtr GalvoWaveformPockel;
        public IntPtr DigBufWaveform;
        public int digitalLineCnt;
        public int Scanmode;
        public int Triggermode;
        public int CycleNum;
        public IntPtr bufferHandle;
        public int lastLoaded;
        public int PreCapStatus;
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    public struct ScanRegionStruct
    {
        public byte ScanID;
        public UInt16 RegionID;
        public UInt32 SizeX;
        public UInt32 SizeY;
        public UInt32 SizeZ;
        public UInt32 SizeT;
        public UInt32 SizeS;
        public UInt64 BufferSize;
    }

    public class DoublePC : INotifyPropertyChanged
    {
        #region Fields

        private double _Value;

        #endregion Fields

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public double Value
        {
            get { return _Value; }
            set { _Value = value; OnPropertyChanged("Value"); }
        }

        #endregion Properties

        #region Methods

        void OnPropertyChanged(string propertyName)
        {
            var handler = PropertyChanged;
            if (handler != null)
            {
                handler(this, new PropertyChangedEventArgs(propertyName));
            }
        }

        #endregion Methods
    }

    public static class ICamera
    {
        #region Enumerations

        public enum AverageMode
        {
            AVG_MODE_NONE = 0,
            AVG_MODE_CUMULATIVE = 1
        }

        public enum CameraType
        {
            CCD = 0,
            LSM = 1,
            CCD_MOSAIC = 2,
            LAST_CAMERA_TYPE
        }

        public enum CCDType
        {
            CCD_LEGACY = 0,
            CMOS = 1,
            sCMOS = 2,
            ORCA = 3,
            DCx = 4,
            CCDTYPE_LAST
        }

        public enum ConnectionStatusType
        {
            CONNECTION_WARMING_UP,
            CONNECTION_READY,
            CONNECTION_UNAVAILABLE,
            CONNECTION_ERROR_STATE
        }

        public enum DataMappingMode
        {
            FIRST_MAPPING_MODE = 0,
            POLARITY_INDEPENDENT = 0,
            POLARITY_POSITIVE = 1,
            POLARITY_NEGATIVE = 2,
            POLARITY_MIXED = 3,
            LAST_MAPPING_MODE
        }

        public enum LSMAreaMode
        {
            FIRST_AREA_MODE = 0,
            SQUARE = 0,
            RECTANGLE = 1,
            LINE_TIMELAPSE = 2,
            LINE = 3,
            POLYLINE = 4,
            LAST_AREA_MODE
        }

        public enum LSMType
        {
            GALVO_RESONANCE,
            GALVO_GALVO,
            RESONANCE_GALVO_GALVO,
            DFLIM_GALVO_GALVO,
            DFLIM_GALVO_RESONANCE,
            STIMULATE_MODULATOR,
            LSMTYPE_LAST
        }

        public enum Params
        {
            PARAM_FIRST_PARAM = 0,

            PARAM_TRIGGER_MODE = 8,

            PARAM_MULTI_FRAME_COUNT = 13,

            PARAM_CAMERA_TYPE,

            PARAM_TRIGGER_TIMEOUT_SEC,//Wait time for timeout in seconds when using hardware trigger mode

            PARAM_ENABLE_FRAME_TRIGGER_WITH_HW_TRIG,//Enable frame trigger when the hardware trigger mode is also enabled.

            PARAM_FRAME_RATE,///<Frame rate calculated by camera

            PARAM_RAW_SAVE_ENABLED_CHANNELS_ONLY,

            PARAM_LSM_TYPE,

            PARAM_CCD_TYPE,

            //confocal settings
            PARAM_LSM_PIXEL_X = 200,
            PARAM_LSM_PIXEL_Y,
            PARAM_LSM_FIELD_SIZE,
            PARAM_LSM_OFFSET_X,
            PARAM_LSM_OFFSET_Y,
            PARAM_LSM_CHANNEL,
            PARAM_LSM_ALIGNMENT,
            PARAM_LSM_INPUTRANGE1,
            PARAM_LSM_INPUTRANGE2,
            PARAM_LSM_INPUTRANGE3,
            PARAM_LSM_INPUTRANGE4,
            PARAM_LSM_CLOCKSOURCE,
            PARAM_LSM_INTERNALCLOCKRATE,
            PARAM_LSM_EXTERNALCLOCKRATE,
            PARAM_LSM_SCANMODE,
            PARAM_LSM_AVERAGEMODE,
            PARAM_LSM_AVERAGENUM,
            PARAM_LSM_AREAMODE,
            PARAM_LSM_Y_AMPLITUDE_SCALER,
            PARAM_LSM_FLYBACK_CYCLE,
            PARAM_LSM_GALVO_ENABLE,
            PARAM_LSM_APPEND_INDEX_TO_FRAME,
            PARAM_LSM_DATAMAP_MODE,
            PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_0,
            PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_0,
            PARAM_LSM_DWELL_TIME,
            PARAM_LSM_GALVO_RASTERANGLE,
            PARAM_LSM_GALVO_LINEDUTY,
            PARAM_LSM_REALTIME_DATA_AVERAGING_ENABLE,
            PARAM_LSM_CAPTURE_WITHOUT_LINE_TRIGGER,
            PARAM_LSM_TWO_WAY_ZONE_1,
            PARAM_LSM_TWO_WAY_ZONE_2,
            PARAM_LSM_TWO_WAY_ZONE_3,
            PARAM_LSM_TWO_WAY_ZONE_4,
            PARAM_LSM_TWO_WAY_ZONE_5,
            PARAM_LSM_TWO_WAY_ZONE_6,
            PARAM_LSM_TWO_WAY_ZONE_7,
            PARAM_LSM_TWO_WAY_ZONE_8,
            PARAM_LSM_TWO_WAY_ZONE_9,
            PARAM_LSM_TWO_WAY_ZONE_10,
            PARAM_LSM_TWO_WAY_ZONE_11,
            PARAM_LSM_TWO_WAY_ZONE_12,
            PARAM_LSM_TWO_WAY_ZONE_13,
            PARAM_LSM_TWO_WAY_ZONE_14,
            PARAM_LSM_TWO_WAY_ZONE_15,
            PARAM_LSM_TWO_WAY_ZONE_16,
            PARAM_LSM_TWO_WAY_ZONE_17,
            PARAM_LSM_TWO_WAY_ZONE_18,
            PARAM_LSM_TWO_WAY_ZONE_19,
            PARAM_LSM_TWO_WAY_ZONE_20,
            PARAM_LSM_TWO_WAY_ZONE_21,
            PARAM_LSM_TWO_WAY_ZONE_22,
            PARAM_LSM_TWO_WAY_ZONE_23,
            PARAM_LSM_TWO_WAY_ZONE_24,
            PARAM_LSM_TWO_WAY_ZONE_25,
            PARAM_LSM_TWO_WAY_ZONE_26,
            PARAM_LSM_TWO_WAY_ZONE_27,
            PARAM_LSM_TWO_WAY_ZONE_28,
            PARAM_LSM_TWO_WAY_ZONE_29,
            PARAM_LSM_TWO_WAY_ZONE_30,
            PARAM_LSM_TWO_WAY_ZONE_31,
            PARAM_LSM_TWO_WAY_ZONE_32,
            PARAM_LSM_TWO_WAY_ZONE_33,
            PARAM_LSM_TWO_WAY_ZONE_34,
            PARAM_LSM_TWO_WAY_ZONE_35,
            PARAM_LSM_TWO_WAY_ZONE_36,
            PARAM_LSM_TWO_WAY_ZONE_37,
            PARAM_LSM_TWO_WAY_ZONE_38,
            PARAM_LSM_TWO_WAY_ZONE_39,
            PARAM_LSM_TWO_WAY_ZONE_40,
            PARAM_LSM_TWO_WAY_ZONE_41,
            PARAM_LSM_TWO_WAY_ZONE_42,
            PARAM_LSM_TWO_WAY_ZONE_43,
            PARAM_LSM_TWO_WAY_ZONE_44,
            PARAM_LSM_TWO_WAY_ZONE_45,
            PARAM_LSM_TWO_WAY_ZONE_46,
            PARAM_LSM_TWO_WAY_ZONE_47,
            PARAM_LSM_TWO_WAY_ZONE_48,
            PARAM_LSM_TWO_WAY_ZONE_49,
            PARAM_LSM_TWO_WAY_ZONE_50,
            PARAM_LSM_TWO_WAY_ZONE_51,
            PARAM_LSM_TWO_WAY_ZONE_52,
            PARAM_LSM_TWO_WAY_ZONE_53,
            PARAM_LSM_TWO_WAY_ZONE_54,
            PARAM_LSM_TWO_WAY_ZONE_55,
            PARAM_LSM_TWO_WAY_ZONE_56,
            PARAM_LSM_TWO_WAY_ZONE_57,
            PARAM_LSM_TWO_WAY_ZONE_58,
            PARAM_LSM_TWO_WAY_ZONE_59,
            PARAM_LSM_TWO_WAY_ZONE_60,
            PARAM_LSM_TWO_WAY_ZONE_61,
            PARAM_LSM_TWO_WAY_ZONE_62,
            PARAM_LSM_TWO_WAY_ZONE_63,
            PARAM_LSM_TWO_WAY_ZONE_64,
            PARAM_LSM_TWO_WAY_ZONE_65,
            PARAM_LSM_TWO_WAY_ZONE_66,
            PARAM_LSM_TWO_WAY_ZONE_67,
            PARAM_LSM_TWO_WAY_ZONE_68,
            PARAM_LSM_TWO_WAY_ZONE_69,
            PARAM_LSM_TWO_WAY_ZONE_70,
            PARAM_LSM_TWO_WAY_ZONE_71,
            PARAM_LSM_TWO_WAY_ZONE_72,
            PARAM_LSM_TWO_WAY_ZONE_73,
            PARAM_LSM_TWO_WAY_ZONE_74,
            PARAM_LSM_TWO_WAY_ZONE_75,
            PARAM_LSM_TWO_WAY_ZONE_76,
            PARAM_LSM_TWO_WAY_ZONE_77,
            PARAM_LSM_TWO_WAY_ZONE_78,
            PARAM_LSM_TWO_WAY_ZONE_79,
            PARAM_LSM_TWO_WAY_ZONE_80,
            PARAM_LSM_TWO_WAY_ZONE_81,
            PARAM_LSM_TWO_WAY_ZONE_82,
            PARAM_LSM_TWO_WAY_ZONE_83,
            PARAM_LSM_TWO_WAY_ZONE_84,
            PARAM_LSM_TWO_WAY_ZONE_85,
            PARAM_LSM_TWO_WAY_ZONE_86,
            PARAM_LSM_TWO_WAY_ZONE_87,
            PARAM_LSM_TWO_WAY_ZONE_88,
            PARAM_LSM_TWO_WAY_ZONE_89,
            PARAM_LSM_TWO_WAY_ZONE_90,
            PARAM_LSM_TWO_WAY_ZONE_91,
            PARAM_LSM_TWO_WAY_ZONE_92,
            PARAM_LSM_TWO_WAY_ZONE_93,
            PARAM_LSM_TWO_WAY_ZONE_94,
            PARAM_LSM_TWO_WAY_ZONE_95,
            PARAM_LSM_TWO_WAY_ZONE_96,
            PARAM_LSM_TWO_WAY_ZONE_97,
            PARAM_LSM_TWO_WAY_ZONE_98,
            PARAM_LSM_TWO_WAY_ZONE_99,
            PARAM_LSM_TWO_WAY_ZONE_100,
            PARAM_LSM_TWO_WAY_ZONE_101,
            PARAM_LSM_TWO_WAY_ZONE_102,
            PARAM_LSM_TWO_WAY_ZONE_103,
            PARAM_LSM_TWO_WAY_ZONE_104,
            PARAM_LSM_TWO_WAY_ZONE_105,
            PARAM_LSM_TWO_WAY_ZONE_106,
            PARAM_LSM_TWO_WAY_ZONE_107,
            PARAM_LSM_TWO_WAY_ZONE_108,
            PARAM_LSM_TWO_WAY_ZONE_109,
            PARAM_LSM_TWO_WAY_ZONE_110,
            PARAM_LSM_TWO_WAY_ZONE_111,
            PARAM_LSM_TWO_WAY_ZONE_112,
            PARAM_LSM_TWO_WAY_ZONE_113,
            PARAM_LSM_TWO_WAY_ZONE_114,
            PARAM_LSM_TWO_WAY_ZONE_115,
            PARAM_LSM_TWO_WAY_ZONE_116,
            PARAM_LSM_TWO_WAY_ZONE_117,
            PARAM_LSM_TWO_WAY_ZONE_118,
            PARAM_LSM_TWO_WAY_ZONE_119,
            PARAM_LSM_TWO_WAY_ZONE_120,
            PARAM_LSM_TWO_WAY_ZONE_121,
            PARAM_LSM_TWO_WAY_ZONE_122,
            PARAM_LSM_TWO_WAY_ZONE_123,
            PARAM_LSM_TWO_WAY_ZONE_124,
            PARAM_LSM_TWO_WAY_ZONE_125,
            PARAM_LSM_TWO_WAY_ZONE_126,
            PARAM_LSM_TWO_WAY_ZONE_127,
            PARAM_LSM_TWO_WAY_ZONE_128,
            PARAM_LSM_TWO_WAY_ZONE_129,
            PARAM_LSM_TWO_WAY_ZONE_130,
            PARAM_LSM_TWO_WAY_ZONE_131,
            PARAM_LSM_TWO_WAY_ZONE_132,
            PARAM_LSM_TWO_WAY_ZONE_133,
            PARAM_LSM_TWO_WAY_ZONE_134,
            PARAM_LSM_TWO_WAY_ZONE_135,
            PARAM_LSM_TWO_WAY_ZONE_136,
            PARAM_LSM_TWO_WAY_ZONE_137,
            PARAM_LSM_TWO_WAY_ZONE_138,
            PARAM_LSM_TWO_WAY_ZONE_139,
            PARAM_LSM_TWO_WAY_ZONE_140,
            PARAM_LSM_TWO_WAY_ZONE_141,
            PARAM_LSM_TWO_WAY_ZONE_142,
            PARAM_LSM_TWO_WAY_ZONE_143,
            PARAM_LSM_TWO_WAY_ZONE_144,
            PARAM_LSM_TWO_WAY_ZONE_145,
            PARAM_LSM_TWO_WAY_ZONE_146,
            PARAM_LSM_TWO_WAY_ZONE_147,
            PARAM_LSM_TWO_WAY_ZONE_148,
            PARAM_LSM_TWO_WAY_ZONE_149,
            PARAM_LSM_TWO_WAY_ZONE_150,
            PARAM_LSM_TWO_WAY_ZONE_151,
            PARAM_LSM_TWO_WAY_ZONE_152,
            PARAM_LSM_TWO_WAY_ZONE_153,
            PARAM_LSM_TWO_WAY_ZONE_154,
            PARAM_LSM_TWO_WAY_ZONE_155,
            PARAM_LSM_TWO_WAY_ZONE_156,
            PARAM_LSM_TWO_WAY_ZONE_157,
            PARAM_LSM_TWO_WAY_ZONE_158,
            PARAM_LSM_TWO_WAY_ZONE_159,
            PARAM_LSM_TWO_WAY_ZONE_160,
            PARAM_LSM_TWO_WAY_ZONE_161,
            PARAM_LSM_TWO_WAY_ZONE_162,
            PARAM_LSM_TWO_WAY_ZONE_163,
            PARAM_LSM_TWO_WAY_ZONE_164,
            PARAM_LSM_TWO_WAY_ZONE_165,
            PARAM_LSM_TWO_WAY_ZONE_166,
            PARAM_LSM_TWO_WAY_ZONE_167,
            PARAM_LSM_TWO_WAY_ZONE_168,
            PARAM_LSM_TWO_WAY_ZONE_169,
            PARAM_LSM_TWO_WAY_ZONE_170,
            PARAM_LSM_TWO_WAY_ZONE_171,
            PARAM_LSM_TWO_WAY_ZONE_172,
            PARAM_LSM_TWO_WAY_ZONE_173,
            PARAM_LSM_TWO_WAY_ZONE_174,
            PARAM_LSM_TWO_WAY_ZONE_175,
            PARAM_LSM_TWO_WAY_ZONE_176,
            PARAM_LSM_TWO_WAY_ZONE_177,
            PARAM_LSM_TWO_WAY_ZONE_178,
            PARAM_LSM_TWO_WAY_ZONE_179,
            PARAM_LSM_TWO_WAY_ZONE_180,
            PARAM_LSM_TWO_WAY_ZONE_181,
            PARAM_LSM_TWO_WAY_ZONE_182,
            PARAM_LSM_TWO_WAY_ZONE_183,
            PARAM_LSM_TWO_WAY_ZONE_184,
            PARAM_LSM_TWO_WAY_ZONE_185,
            PARAM_LSM_TWO_WAY_ZONE_186,
            PARAM_LSM_TWO_WAY_ZONE_187,
            PARAM_LSM_TWO_WAY_ZONE_188,
            PARAM_LSM_TWO_WAY_ZONE_189,
            PARAM_LSM_TWO_WAY_ZONE_190,
            PARAM_LSM_TWO_WAY_ZONE_191,
            PARAM_LSM_TWO_WAY_ZONE_192,
            PARAM_LSM_TWO_WAY_ZONE_193,
            PARAM_LSM_TWO_WAY_ZONE_194,
            PARAM_LSM_TWO_WAY_ZONE_195,
            PARAM_LSM_TWO_WAY_ZONE_196,
            PARAM_LSM_TWO_WAY_ZONE_197,
            PARAM_LSM_TWO_WAY_ZONE_198,
            PARAM_LSM_TWO_WAY_ZONE_199,
            PARAM_LSM_TWO_WAY_ZONE_200,
            PARAM_LSM_TWO_WAY_ZONE_201,
            PARAM_LSM_TWO_WAY_ZONE_202,
            PARAM_LSM_TWO_WAY_ZONE_203,
            PARAM_LSM_TWO_WAY_ZONE_204,
            PARAM_LSM_TWO_WAY_ZONE_205,
            PARAM_LSM_TWO_WAY_ZONE_206,
            PARAM_LSM_TWO_WAY_ZONE_207,
            PARAM_LSM_TWO_WAY_ZONE_208,
            PARAM_LSM_TWO_WAY_ZONE_209,
            PARAM_LSM_TWO_WAY_ZONE_210,
            PARAM_LSM_TWO_WAY_ZONE_211,
            PARAM_LSM_TWO_WAY_ZONE_212,
            PARAM_LSM_TWO_WAY_ZONE_213,
            PARAM_LSM_TWO_WAY_ZONE_214,
            PARAM_LSM_TWO_WAY_ZONE_215,
            PARAM_LSM_TWO_WAY_ZONE_216,
            PARAM_LSM_TWO_WAY_ZONE_217,
            PARAM_LSM_TWO_WAY_ZONE_218,
            PARAM_LSM_TWO_WAY_ZONE_219,
            PARAM_LSM_TWO_WAY_ZONE_220,
            PARAM_LSM_TWO_WAY_ZONE_221,
            PARAM_LSM_TWO_WAY_ZONE_222,
            PARAM_LSM_TWO_WAY_ZONE_223,
            PARAM_LSM_TWO_WAY_ZONE_224,
            PARAM_LSM_TWO_WAY_ZONE_225,
            PARAM_LSM_TWO_WAY_ZONE_226,
            PARAM_LSM_TWO_WAY_ZONE_227,
            PARAM_LSM_TWO_WAY_ZONE_228,
            PARAM_LSM_TWO_WAY_ZONE_229,
            PARAM_LSM_TWO_WAY_ZONE_230,
            PARAM_LSM_TWO_WAY_ZONE_231,
            PARAM_LSM_TWO_WAY_ZONE_232,
            PARAM_LSM_TWO_WAY_ZONE_233,
            PARAM_LSM_TWO_WAY_ZONE_234,
            PARAM_LSM_TWO_WAY_ZONE_235,
            PARAM_LSM_TWO_WAY_ZONE_236,
            PARAM_LSM_TWO_WAY_ZONE_237,
            PARAM_LSM_TWO_WAY_ZONE_238,
            PARAM_LSM_TWO_WAY_ZONE_239,
            PARAM_LSM_TWO_WAY_ZONE_240,
            PARAM_LSM_TWO_WAY_ZONE_241,
            PARAM_LSM_TWO_WAY_ZONE_242,
            PARAM_LSM_TWO_WAY_ZONE_243,
            PARAM_LSM_TWO_WAY_ZONE_244,
            PARAM_LSM_TWO_WAY_ZONE_245,
            PARAM_LSM_TWO_WAY_ZONE_246,
            PARAM_LSM_TWO_WAY_ZONE_247,
            PARAM_LSM_TWO_WAY_ZONE_248,
            PARAM_LSM_TWO_WAY_ZONE_249,
            PARAM_LSM_TWO_WAY_ZONE_250,
            PARAM_LSM_TWO_WAY_ZONE_251,

            PARAM_LSM_FORCE_SETTINGS_UPDATE,

            PARAM_LSM_POCKELS_MASK,
            PARAM_LSM_POCKELS_MASK_ENABLE_0,

            PARAM_LSM_FIELD_SIZE_CALIBRATION,
            PARAM_LSM_Y_COMMUNICATION_ENABLE,
            PARAM_LSM_RESET_FLYBACK_ENABLE,

            PARAM_LSM_DMA_BUFFER_COUNT,///<Change the number of dma buffers on the fly

            PARAM_LSM_VERTICAL_SCAN_DIRECTION,
            PARAM_LSM_FINE_OFFSET_X,
            PARAM_LSM_FINE_OFFSET_Y,
            PARAM_LSM_FINE_FIELD_SIZE_SCALE_X,
            PARAM_LSM_FINE_FIELD_SIZE_SCALE_Y,
            PARAM_LSM_1X_FIELD_SIZE,

            PARAM_LSM_POCKELS_FIND_MIN_MAX_0,
            PARAM_LSM_POCKELS_FIND_MIN_MAX_1,
            PARAM_LSM_POCKELS_FIND_MIN_MAX_2,
            PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_1,
            PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_1,
            PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_2,
            PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_2,
            PARAM_LSM_POCKELS_MASK_ENABLE_1,
            PARAM_LSM_POCKELS_MASK_ENABLE_2,

            PARAM_LSM_POCKELS_MIN_VOLTAGE_0,
            PARAM_LSM_POCKELS_MIN_VOLTAGE_1,
            PARAM_LSM_POCKELS_MIN_VOLTAGE_2,
            PARAM_LSM_POCKELS_MAX_VOLTAGE_0,
            PARAM_LSM_POCKELS_MAX_VOLTAGE_1,
            PARAM_LSM_POCKELS_MAX_VOLTAGE_2,

            PARAM_LSM_POCKELS_MIN_MAX_PLOT_0,
            PARAM_LSM_POCKELS_MIN_MAX_PLOT_1,
            PARAM_LSM_POCKELS_MIN_MAX_PLOT_2,

            PARAM_LSM_POCKELS_CONNECTED_0,
            PARAM_LSM_POCKELS_CONNECTED_1,
            PARAM_LSM_POCKELS_CONNECTED_2,

            PARAM_LSM_CHANNEL_POLARITY_1,
            PARAM_LSM_CHANNEL_POLARITY_2,
            PARAM_LSM_CHANNEL_POLARITY_3,
            PARAM_LSM_CHANNEL_POLARITY_4,

            PARAM_LSM_WAVEFORM_PRECAPTURESTATUS,
            PARAM_LSM_WAVEFORM_PATH_NAME,

            PARAM_LSM_SCANAREA_ANGLE,

            PARAM_LSM_POCKELS_START_SCAN_VOLTAGE_0,
            PARAM_LSM_POCKELS_START_SCAN_VOLTAGE_1,
            PARAM_LSM_POCKELS_START_SCAN_VOLTAGE_2,
            PARAM_LSM_POCKELS_STOP_SCAN_VOLTAGE_0,
            PARAM_LSM_POCKELS_STOP_SCAN_VOLTAGE_1,
            PARAM_LSM_POCKELS_STOP_SCAN_VOLTAGE_2,

            PARAM_LSM_FLYBACK_TIME,

            PARAM_LSM_HORIZONTAL_FLIP,
            PARAM_LSM_POCKELS_LINE_0,
            PARAM_LSM_POCKELS_LINE_1,
            PARAM_LSM_POCKELS_LINE_2,
            PARAM_LSM_MINIMIZE_FLYBACK_CYCLES,

            PARAM_LSM_DWELL_TIME_STEP,

            PARAM_LSM_STOP_ACQUISITION,

            PARAM_LSM_POCKELS_MASK_WIDTH,

            PARAM_LSM_POCKELS_OUTPUT_USE_REF,

            PARAM_LSM_POCKELS_FIND_MIN_MAX_3,
            PARAM_LSM_POCKELS_POWER_LEVEL_PERCENTAGE_3,
            PARAM_LSM_POCKELS_LINE_BLANKING_PERCENTAGE_3,
            PARAM_LSM_POCKELS_MASK_ENABLE_3,
            PARAM_LSM_POCKELS_MIN_VOLTAGE_3,
            PARAM_LSM_POCKELS_MAX_VOLTAGE_3,
            PARAM_LSM_POCKELS_MIN_MAX_PLOT_3,
            PARAM_LSM_POCKELS_CONNECTED_3,
            PARAM_LSM_POCKELS_START_SCAN_VOLTAGE_3,
            PARAM_LSM_POCKELS_STOP_SCAN_VOLTAGE_3,
            PARAM_LSM_POCKELS_LINE_3,
            PARAM_LSM_POCKELS_MASK_INVERT_0,
            PARAM_LSM_POCKELS_MASK_INVERT_1,
            PARAM_LSM_POCKELS_MASK_INVERT_2,
            PARAM_LSM_POCKELS_MASK_INVERT_3,

            PARAM_LSM_POCKELS_RESPONSE_TYPE_0,
            PARAM_LSM_POCKELS_RESPONSE_TYPE_1,
            PARAM_LSM_POCKELS_RESPONSE_TYPE_2,
            PARAM_LSM_POCKELS_RESPONSE_TYPE_3,

            PARAM_LSM_PULSE_MULTIPLEXING_ENABLE,
            PARAM_LSM_PULSE_MULTIPLEXING_PHASE,

            PARAM_LSM_TWO_WAY_ZONE_FINE_1 = 700,
            PARAM_LSM_TWO_WAY_ZONE_FINE_2,
            PARAM_LSM_TWO_WAY_ZONE_FINE_3,
            PARAM_LSM_TWO_WAY_ZONE_FINE_4,
            PARAM_LSM_TWO_WAY_ZONE_FINE_5,
            PARAM_LSM_TWO_WAY_ZONE_FINE_6,
            PARAM_LSM_TWO_WAY_ZONE_FINE_7,
            PARAM_LSM_TWO_WAY_ZONE_FINE_8,
            PARAM_LSM_TWO_WAY_ZONE_FINE_9,
            PARAM_LSM_TWO_WAY_ZONE_FINE_10,
            PARAM_LSM_TWO_WAY_ZONE_FINE_11,
            PARAM_LSM_TWO_WAY_ZONE_FINE_12,
            PARAM_LSM_TWO_WAY_ZONE_FINE_13,
            PARAM_LSM_TWO_WAY_ZONE_FINE_14,
            PARAM_LSM_TWO_WAY_ZONE_FINE_15,
            PARAM_LSM_TWO_WAY_ZONE_FINE_16,
            PARAM_LSM_TWO_WAY_ZONE_FINE_17,
            PARAM_LSM_TWO_WAY_ZONE_FINE_18,
            PARAM_LSM_TWO_WAY_ZONE_FINE_19,
            PARAM_LSM_TWO_WAY_ZONE_FINE_20,
            PARAM_LSM_TWO_WAY_ZONE_FINE_21,
            PARAM_LSM_TWO_WAY_ZONE_FINE_22,
            PARAM_LSM_TWO_WAY_ZONE_FINE_23,
            PARAM_LSM_TWO_WAY_ZONE_FINE_24,
            PARAM_LSM_TWO_WAY_ZONE_FINE_25,
            PARAM_LSM_TWO_WAY_ZONE_FINE_26,
            PARAM_LSM_TWO_WAY_ZONE_FINE_27,
            PARAM_LSM_TWO_WAY_ZONE_FINE_28,
            PARAM_LSM_TWO_WAY_ZONE_FINE_29,
            PARAM_LSM_TWO_WAY_ZONE_FINE_30,
            PARAM_LSM_TWO_WAY_ZONE_FINE_31,
            PARAM_LSM_TWO_WAY_ZONE_FINE_32,
            PARAM_LSM_TWO_WAY_ZONE_FINE_33,
            PARAM_LSM_TWO_WAY_ZONE_FINE_34,
            PARAM_LSM_TWO_WAY_ZONE_FINE_35,
            PARAM_LSM_TWO_WAY_ZONE_FINE_36,
            PARAM_LSM_TWO_WAY_ZONE_FINE_37,
            PARAM_LSM_TWO_WAY_ZONE_FINE_38,
            PARAM_LSM_TWO_WAY_ZONE_FINE_39,
            PARAM_LSM_TWO_WAY_ZONE_FINE_40,
            PARAM_LSM_TWO_WAY_ZONE_FINE_41,
            PARAM_LSM_TWO_WAY_ZONE_FINE_42,
            PARAM_LSM_TWO_WAY_ZONE_FINE_43,
            PARAM_LSM_TWO_WAY_ZONE_FINE_44,
            PARAM_LSM_TWO_WAY_ZONE_FINE_45,
            PARAM_LSM_TWO_WAY_ZONE_FINE_46,
            PARAM_LSM_TWO_WAY_ZONE_FINE_47,
            PARAM_LSM_TWO_WAY_ZONE_FINE_48,
            PARAM_LSM_TWO_WAY_ZONE_FINE_49,
            PARAM_LSM_TWO_WAY_ZONE_FINE_50,
            PARAM_LSM_TWO_WAY_ZONE_FINE_51,
            PARAM_LSM_TWO_WAY_ZONE_FINE_52,
            PARAM_LSM_TWO_WAY_ZONE_FINE_53,
            PARAM_LSM_TWO_WAY_ZONE_FINE_54,
            PARAM_LSM_TWO_WAY_ZONE_FINE_55,
            PARAM_LSM_TWO_WAY_ZONE_FINE_56,
            PARAM_LSM_TWO_WAY_ZONE_FINE_57,
            PARAM_LSM_TWO_WAY_ZONE_FINE_58,
            PARAM_LSM_TWO_WAY_ZONE_FINE_59,
            PARAM_LSM_TWO_WAY_ZONE_FINE_60,
            PARAM_LSM_TWO_WAY_ZONE_FINE_61,
            PARAM_LSM_TWO_WAY_ZONE_FINE_62,
            PARAM_LSM_TWO_WAY_ZONE_FINE_63,
            PARAM_LSM_TWO_WAY_ZONE_FINE_64,
            PARAM_LSM_TWO_WAY_ZONE_FINE_65,
            PARAM_LSM_TWO_WAY_ZONE_FINE_66,
            PARAM_LSM_TWO_WAY_ZONE_FINE_67,
            PARAM_LSM_TWO_WAY_ZONE_FINE_68,
            PARAM_LSM_TWO_WAY_ZONE_FINE_69,
            PARAM_LSM_TWO_WAY_ZONE_FINE_70,
            PARAM_LSM_TWO_WAY_ZONE_FINE_71,
            PARAM_LSM_TWO_WAY_ZONE_FINE_72,
            PARAM_LSM_TWO_WAY_ZONE_FINE_73,
            PARAM_LSM_TWO_WAY_ZONE_FINE_74,
            PARAM_LSM_TWO_WAY_ZONE_FINE_75,
            PARAM_LSM_TWO_WAY_ZONE_FINE_76,
            PARAM_LSM_TWO_WAY_ZONE_FINE_77,
            PARAM_LSM_TWO_WAY_ZONE_FINE_78,
            PARAM_LSM_TWO_WAY_ZONE_FINE_79,
            PARAM_LSM_TWO_WAY_ZONE_FINE_80,
            PARAM_LSM_TWO_WAY_ZONE_FINE_81,
            PARAM_LSM_TWO_WAY_ZONE_FINE_82,
            PARAM_LSM_TWO_WAY_ZONE_FINE_83,
            PARAM_LSM_TWO_WAY_ZONE_FINE_84,
            PARAM_LSM_TWO_WAY_ZONE_FINE_85,
            PARAM_LSM_TWO_WAY_ZONE_FINE_86,
            PARAM_LSM_TWO_WAY_ZONE_FINE_87,
            PARAM_LSM_TWO_WAY_ZONE_FINE_88,
            PARAM_LSM_TWO_WAY_ZONE_FINE_89,
            PARAM_LSM_TWO_WAY_ZONE_FINE_90,
            PARAM_LSM_TWO_WAY_ZONE_FINE_91,
            PARAM_LSM_TWO_WAY_ZONE_FINE_92,
            PARAM_LSM_TWO_WAY_ZONE_FINE_93,
            PARAM_LSM_TWO_WAY_ZONE_FINE_94,
            PARAM_LSM_TWO_WAY_ZONE_FINE_95,
            PARAM_LSM_TWO_WAY_ZONE_FINE_96,
            PARAM_LSM_TWO_WAY_ZONE_FINE_97,
            PARAM_LSM_TWO_WAY_ZONE_FINE_98,
            PARAM_LSM_TWO_WAY_ZONE_FINE_99,
            PARAM_LSM_TWO_WAY_ZONE_FINE_100,
            PARAM_LSM_TWO_WAY_ZONE_FINE_101,
            PARAM_LSM_TWO_WAY_ZONE_FINE_102,
            PARAM_LSM_TWO_WAY_ZONE_FINE_103,
            PARAM_LSM_TWO_WAY_ZONE_FINE_104,
            PARAM_LSM_TWO_WAY_ZONE_FINE_105,
            PARAM_LSM_TWO_WAY_ZONE_FINE_106,
            PARAM_LSM_TWO_WAY_ZONE_FINE_107,
            PARAM_LSM_TWO_WAY_ZONE_FINE_108,
            PARAM_LSM_TWO_WAY_ZONE_FINE_109,
            PARAM_LSM_TWO_WAY_ZONE_FINE_110,
            PARAM_LSM_TWO_WAY_ZONE_FINE_111,
            PARAM_LSM_TWO_WAY_ZONE_FINE_112,
            PARAM_LSM_TWO_WAY_ZONE_FINE_113,
            PARAM_LSM_TWO_WAY_ZONE_FINE_114,
            PARAM_LSM_TWO_WAY_ZONE_FINE_115,
            PARAM_LSM_TWO_WAY_ZONE_FINE_116,
            PARAM_LSM_TWO_WAY_ZONE_FINE_117,
            PARAM_LSM_TWO_WAY_ZONE_FINE_118,
            PARAM_LSM_TWO_WAY_ZONE_FINE_119,
            PARAM_LSM_TWO_WAY_ZONE_FINE_120,
            PARAM_LSM_TWO_WAY_ZONE_FINE_121,
            PARAM_LSM_TWO_WAY_ZONE_FINE_122,
            PARAM_LSM_TWO_WAY_ZONE_FINE_123,
            PARAM_LSM_TWO_WAY_ZONE_FINE_124,
            PARAM_LSM_TWO_WAY_ZONE_FINE_125,
            PARAM_LSM_TWO_WAY_ZONE_FINE_126,
            PARAM_LSM_TWO_WAY_ZONE_FINE_127,
            PARAM_LSM_TWO_WAY_ZONE_FINE_128,
            PARAM_LSM_TWO_WAY_ZONE_FINE_129,
            PARAM_LSM_TWO_WAY_ZONE_FINE_130,
            PARAM_LSM_TWO_WAY_ZONE_FINE_131,
            PARAM_LSM_TWO_WAY_ZONE_FINE_132,
            PARAM_LSM_TWO_WAY_ZONE_FINE_133,
            PARAM_LSM_TWO_WAY_ZONE_FINE_134,
            PARAM_LSM_TWO_WAY_ZONE_FINE_135,
            PARAM_LSM_TWO_WAY_ZONE_FINE_136,
            PARAM_LSM_TWO_WAY_ZONE_FINE_137,
            PARAM_LSM_TWO_WAY_ZONE_FINE_138,
            PARAM_LSM_TWO_WAY_ZONE_FINE_139,
            PARAM_LSM_TWO_WAY_ZONE_FINE_140,
            PARAM_LSM_TWO_WAY_ZONE_FINE_141,
            PARAM_LSM_TWO_WAY_ZONE_FINE_142,
            PARAM_LSM_TWO_WAY_ZONE_FINE_143,
            PARAM_LSM_TWO_WAY_ZONE_FINE_144,
            PARAM_LSM_TWO_WAY_ZONE_FINE_145,
            PARAM_LSM_TWO_WAY_ZONE_FINE_146,
            PARAM_LSM_TWO_WAY_ZONE_FINE_147,
            PARAM_LSM_TWO_WAY_ZONE_FINE_148,
            PARAM_LSM_TWO_WAY_ZONE_FINE_149,
            PARAM_LSM_TWO_WAY_ZONE_FINE_150,
            PARAM_LSM_TWO_WAY_ZONE_FINE_151,
            PARAM_LSM_TWO_WAY_ZONE_FINE_152,
            PARAM_LSM_TWO_WAY_ZONE_FINE_153,
            PARAM_LSM_TWO_WAY_ZONE_FINE_154,
            PARAM_LSM_TWO_WAY_ZONE_FINE_155,
            PARAM_LSM_TWO_WAY_ZONE_FINE_156,
            PARAM_LSM_TWO_WAY_ZONE_FINE_157,
            PARAM_LSM_TWO_WAY_ZONE_FINE_158,
            PARAM_LSM_TWO_WAY_ZONE_FINE_159,
            PARAM_LSM_TWO_WAY_ZONE_FINE_160,
            PARAM_LSM_TWO_WAY_ZONE_FINE_161,
            PARAM_LSM_TWO_WAY_ZONE_FINE_162,
            PARAM_LSM_TWO_WAY_ZONE_FINE_163,
            PARAM_LSM_TWO_WAY_ZONE_FINE_164,
            PARAM_LSM_TWO_WAY_ZONE_FINE_165,
            PARAM_LSM_TWO_WAY_ZONE_FINE_166,
            PARAM_LSM_TWO_WAY_ZONE_FINE_167,
            PARAM_LSM_TWO_WAY_ZONE_FINE_168,
            PARAM_LSM_TWO_WAY_ZONE_FINE_169,
            PARAM_LSM_TWO_WAY_ZONE_FINE_170,
            PARAM_LSM_TWO_WAY_ZONE_FINE_171,
            PARAM_LSM_TWO_WAY_ZONE_FINE_172,
            PARAM_LSM_TWO_WAY_ZONE_FINE_173,
            PARAM_LSM_TWO_WAY_ZONE_FINE_174,
            PARAM_LSM_TWO_WAY_ZONE_FINE_175,
            PARAM_LSM_TWO_WAY_ZONE_FINE_176,
            PARAM_LSM_TWO_WAY_ZONE_FINE_177,
            PARAM_LSM_TWO_WAY_ZONE_FINE_178,
            PARAM_LSM_TWO_WAY_ZONE_FINE_179,
            PARAM_LSM_TWO_WAY_ZONE_FINE_180,
            PARAM_LSM_TWO_WAY_ZONE_FINE_181,
            PARAM_LSM_TWO_WAY_ZONE_FINE_182,
            PARAM_LSM_TWO_WAY_ZONE_FINE_183,
            PARAM_LSM_TWO_WAY_ZONE_FINE_184,
            PARAM_LSM_TWO_WAY_ZONE_FINE_185,
            PARAM_LSM_TWO_WAY_ZONE_FINE_186,
            PARAM_LSM_TWO_WAY_ZONE_FINE_187,
            PARAM_LSM_TWO_WAY_ZONE_FINE_188,
            PARAM_LSM_TWO_WAY_ZONE_FINE_189,
            PARAM_LSM_TWO_WAY_ZONE_FINE_190,
            PARAM_LSM_TWO_WAY_ZONE_FINE_191,
            PARAM_LSM_TWO_WAY_ZONE_FINE_192,
            PARAM_LSM_TWO_WAY_ZONE_FINE_193,
            PARAM_LSM_TWO_WAY_ZONE_FINE_194,
            PARAM_LSM_TWO_WAY_ZONE_FINE_195,
            PARAM_LSM_TWO_WAY_ZONE_FINE_196,
            PARAM_LSM_TWO_WAY_ZONE_FINE_197,
            PARAM_LSM_TWO_WAY_ZONE_FINE_198,
            PARAM_LSM_TWO_WAY_ZONE_FINE_199,
            PARAM_LSM_TWO_WAY_ZONE_FINE_200,
            PARAM_LSM_TWO_WAY_ZONE_FINE_201,
            PARAM_LSM_TWO_WAY_ZONE_FINE_202,
            PARAM_LSM_TWO_WAY_ZONE_FINE_203,
            PARAM_LSM_TWO_WAY_ZONE_FINE_204,
            PARAM_LSM_TWO_WAY_ZONE_FINE_205,
            PARAM_LSM_TWO_WAY_ZONE_FINE_206,
            PARAM_LSM_TWO_WAY_ZONE_FINE_207,
            PARAM_LSM_TWO_WAY_ZONE_FINE_208,
            PARAM_LSM_TWO_WAY_ZONE_FINE_209,
            PARAM_LSM_TWO_WAY_ZONE_FINE_210,
            PARAM_LSM_TWO_WAY_ZONE_FINE_211,
            PARAM_LSM_TWO_WAY_ZONE_FINE_212,
            PARAM_LSM_TWO_WAY_ZONE_FINE_213,
            PARAM_LSM_TWO_WAY_ZONE_FINE_214,
            PARAM_LSM_TWO_WAY_ZONE_FINE_215,
            PARAM_LSM_TWO_WAY_ZONE_FINE_216,
            PARAM_LSM_TWO_WAY_ZONE_FINE_217,
            PARAM_LSM_TWO_WAY_ZONE_FINE_218,
            PARAM_LSM_TWO_WAY_ZONE_FINE_219,
            PARAM_LSM_TWO_WAY_ZONE_FINE_220,
            PARAM_LSM_TWO_WAY_ZONE_FINE_221,
            PARAM_LSM_TWO_WAY_ZONE_FINE_222,
            PARAM_LSM_TWO_WAY_ZONE_FINE_223,
            PARAM_LSM_TWO_WAY_ZONE_FINE_224,
            PARAM_LSM_TWO_WAY_ZONE_FINE_225,
            PARAM_LSM_TWO_WAY_ZONE_FINE_226,
            PARAM_LSM_TWO_WAY_ZONE_FINE_227,
            PARAM_LSM_TWO_WAY_ZONE_FINE_228,
            PARAM_LSM_TWO_WAY_ZONE_FINE_229,
            PARAM_LSM_TWO_WAY_ZONE_FINE_230,
            PARAM_LSM_TWO_WAY_ZONE_FINE_231,
            PARAM_LSM_TWO_WAY_ZONE_FINE_232,
            PARAM_LSM_TWO_WAY_ZONE_FINE_233,
            PARAM_LSM_TWO_WAY_ZONE_FINE_234,
            PARAM_LSM_TWO_WAY_ZONE_FINE_235,
            PARAM_LSM_TWO_WAY_ZONE_FINE_236,
            PARAM_LSM_TWO_WAY_ZONE_FINE_237,
            PARAM_LSM_TWO_WAY_ZONE_FINE_238,
            PARAM_LSM_TWO_WAY_ZONE_FINE_239,
            PARAM_LSM_TWO_WAY_ZONE_FINE_240,
            PARAM_LSM_TWO_WAY_ZONE_FINE_241,
            PARAM_LSM_TWO_WAY_ZONE_FINE_242,
            PARAM_LSM_TWO_WAY_ZONE_FINE_243,
            PARAM_LSM_TWO_WAY_ZONE_FINE_244,
            PARAM_LSM_TWO_WAY_ZONE_FINE_245,
            PARAM_LSM_TWO_WAY_ZONE_FINE_246,
            PARAM_LSM_TWO_WAY_ZONE_FINE_247,
            PARAM_LSM_TWO_WAY_ZONE_FINE_248,
            PARAM_LSM_TWO_WAY_ZONE_FINE_249,
            PARAM_LSM_TWO_WAY_ZONE_FINE_250,
            PARAM_LSM_TWO_WAY_ZONE_FINE_251,
            PARAM_SCANNER_INIT_MODE,
            PARAM_LSM_SIM_INDEX,
            PARAM_LSM_CENTER_WITH_OFFSET,
            PARAM_LSM_DIG_OFFSET_0,
            PARAM_LSM_DIG_OFFSET_1,
            PARAM_LSM_DIG_OFFSET_2,
            PARAM_LSM_DIG_OFFSET_3,
            PARAM_LSM_INTERLEAVE_SCAN,
            PARAM_LSM_HIGHRES_OFFSET_X,///<GG mirror X high resolution offset in volt
            PARAM_LSM_HIGHRES_OFFSET_Y,///<GG mirror Y high resolution offset in volt
            PARAM_LSM_HIGHRES_OFFSET_X2,///<2nd GG mirror X high resolution offset in volt
            PARAM_LSM_HIGHRES_OFFSET_Y2,///<2nd GG mirror Y high resolution offset in volt
            PARAM_LSM_FINE_OFFSET_X2,///<2nd GG mirror X fine offset
            PARAM_LSM_FINE_OFFSET_Y2,///<2nd GG mirror Y fine offset
            PARAM_LSM_3P_ENABLE,
            PARAM_LSM_3P_ALIGN_FINE,
            PARAM_LSM_3P_ALIGN_COARSE,
            PARAM_LSM_3P_FIR_CHANNEL,
            PARAM_LSM_3P_FIR_INDEX,
            PARAM_LSM_3P_FIR_TAP_INDEX,
            PARAM_LSM_3P_FIR_TAP_VALUE,
            PARAM_LSM_QUERY_EXTERNALCLOCKRATE,
            PARAM_LSM_GG_TURNAROUNDTIME_US,
            PARAM_LSM_GG_SUPER_USER,
            PARAM_LSM_CALCULATED_MIN_DWELL,
            PARAM_LSM_TB_LINE_SCAN_TIME_MS,
            PARAM_LSM_TIME_BASED_LINE_SCAN,
            PARAM_LSM_IS_LIVE,
            PARAM_LSM_TIME_BASED_LINE_SCAN_INCREMENT_TIME_MS,
            PARAM_LSM_WAVEFORM_DRIVER_TYPE,
            PARAM_LSM_NUMBER_OF_PLANES,
            PARAM_LSM_3P_MANUAL_FIR1_CONTROL_ENABLE,
            PARAM_LSM_POWER_RAMP_ENABLE,
            PARAM_LSM_POWER_RAMP_NUM_FRAMES,
            PARAM_LSM_POWER_RAMP_NUM_FLYBACK_FRAMES,
            PARAM_LSM_POWER_RAMP_MODE,
            PARAM_LSM_POWER_RAMP_PERCENTAGE_BUFFER,

            PARAM_FIRST_CCD_PARAM = 1000,
            PARAM_BINNING_X = 1000,///<Binning X
            PARAM_BINNING_Y,///<Binning Y

            PARAM_CAPTURE_REGION_LEFT,///<coordinates for region
            PARAM_CAPTURE_REGION_RIGHT,
            PARAM_CAPTURE_REGION_TOP,
            PARAM_CAPTURE_REGION_BOTTOM,

            PARAM_GAIN,///<Camera gain
            PARAM_EM_GAIN,///<Camear EM Gain
            PARAM_EXPOSURE_TIME_MS,
            PARAM_PIXEL_SIZE,///<Pixel size in microns
            PARAM_TDI_HEIGHT,///<TDI height. Can be smaller than height returned by camera
            PARAM_LIGHT_MODE,///<anti-blooming setting

            PARAM_READOUT_SPEED_INDEX, ///<index used to retrieve the readout speed value
            PARAM_READOUT_SPEED_VALUE,///<
            PARAM_OPTICAL_BLACK_LEVEL,///<
            PARAM_COOLING_MODE, // only 0 and 1 are allowed
            PARAM_OP_MODE, //current operating mode,  (0 == NORMAL, 1 == PDX, 2 == TOE, 3 (not implemented yet) == TDI)
            PARAM_TDI_TRIGGERS,///<
            PARAM_TDI_LINESHIFTS,///<
            PARAM_TDI_TRIM_MODE, ///< (OFF/ON)
            PARAM_CONSOLE_WRITE,///<
            PARAM_CONSOLE_READ,///<
            PARAM_TDI_LINETRIM,///<
            PARAM_DETECTOR_NAME,///<
            PARAM_DETECTOR_SERIAL_NUMBER,///<
            PARAM_NIR_BOOST,///<
            PARAM_NUMBER_OF_IMAGES_TO_BUFFER,///<
            PARAM_TAP_INDEX,///<
            PARAM_TAP_VALUE,///<
            PARAM_TAP_BALANCE_MODE,///<
            PARAM_BITS_PER_PIXEL,///<Should be 12 or 14 for most CCD's
            PARAM_DROPPED_FRAMES,
            PARAM_CAMERA_AVERAGEMODE,
            PARAM_CAMERA_AVERAGENUM,
            PARAM_CAMERA_IMAGE_WIDTH,
            PARAM_CAMERA_IMAGE_HEIGHT,
            PARAM_CAMERA_IMAGE_HORIZONTAL_FLIP,
            PARAM_CAMERA_IMAGE_VERTICAL_FLIP,
            PARAM_CAMERA_IMAGE_ANGLE,
            PARAM_CAMERA_DMA_BUFFER_COUNT,
            PARAM_CAMERA_CHANNEL,
            PARAM_CAMERA_LED_AVAILABLE,
            PARAM_CAMERA_LED_ENABLE,
            PARAM_HOT_PIXEL_THRESHOLD_VALUE,
            PARAM_HOT_PIXEL_ENABLED,
            PARAM_BIN_INDEX,
            PARAM_HOT_PIXEL_INDEX,
            PARAM_DMA_BUFFER_AVAILABLE_FRAMES,
            PARAM_CAMERA_FRAME_RATE_CONTROL_ENABLED,
            PARAM_CAMERA_FRAME_RATE_CONTROL_VALUE,

            PARAM_MESO_PLATE_INFO = 1500,
            PARAM_MESO_SCAN_INFO,
            PARAM_MESO_CURVE_PARAME_A,
            PARAM_MESO_CURVE_PARAME_B,
            PARAM_MESO_STRIP_COUNT,

            PARAM_MESO_GX1_FEEDBACK_RATIO,
            PARAM_MESO_GX1_FEEDBACK_OFFSET,
            PARAM_MESO_GX2_FEEDBACK_RATIO,
            PARAM_MESO_GX2_FEEDBACK_OFFSET,
            PARAM_MESO_GY_FEEDBACK_RATIO,
            PARAM_MESO_GY_FEEDBACK_OFFSET,
            PARAM_MESO_VOICECOIL_FEEDBACK_RATIO,
            PARAM_MESO_VOICECOIL_FEEDBACK_OFFSET,
            PARAM_MESO_POS_MIN_X_VOLTAGE,
            PARAM_MESO_POS_MIN_Y_VOLTAGE,
            PARAM_MESO_POS_MIN_Z_VOLTAGE,
            PARAM_MESO_POS_MAX_X_VOLTAGE,
            PARAM_MESO_POS_MAX_Y_VOLTAGE,
            PARAM_MESO_POS_MAX_Z_VOLTAGE,

            PARAM_MESO_LAG_GALVO_Y,
            PARAM_MESO_DAMPING_GALVO_Y,
            PARAM_MESO_MAXVELOCITY_GALVO_Y,

            PARAM_MESO_EXTENDTIME_START_Y,
            PARAM_MESO_EXTENDTIME_END_Y,

            PARAM_MESO_LAG_GALVO_X1,
            PARAM_MESO_LAG_GALVO_X2,
            PARAM_MESO_DAMPING_GALVO_X,
            PARAM_MESO_MAXVELOCITY_GALVO_X,
            PARAM_MESO_EXTENDTIME_START_X,
            PARAM_MESO_EXTENDTIME_END_X,

            PARAM_MESO_LAG_VOICECOIL,
            PARAM_MESO_DAMPING_VOICECOIL,
            PARAM_MESO_MAXVELOCITY_VOICECOIL,
            PARAM_MESO_EXTENDTIME_START_VOICECOIL,
            PARAM_MESO_EXTENDTIME_END_VOICECOIL,
            PARAM_MESO_CENTER_SHIFT_X_VOICECOIL,
            PARAM_MESO_CENTER_SHIFT_Y_VOICECOIL,
            PARAM_MESO_POINTS_PER_LINE_VOICECOIL,

            PARAM_MESO_TWOWAY_ALIGNMENT_SHIFT,
            PARAM_MESO_GY_FIELD_TO_VOLTAGE,
            PARAM_MESO_RESONANT_FIELD_TO_VOLTAGE,

            PARAM_MESO_GX_FIELD_TO_VOLTAGE,
            PARAM_MESO_VOICECOIL_FIELD_TO_VOLTAGE,
            PARAM_MESO_DUTYCYCLE,
            PARAM_MESO_POCKEL_MIN_PERCENT,
            PARAM_MESO_POCKEL_ALLOW_MIN,
            PARAM_MESO_POCKEL_ALLOW_MAX,
            PARAM_POCKELS_STOP_CALIBRATION,
            PARAM_MESO_G_FOR_X2,
            PARAM_MESO_H_FOR_X2,

            PARAM_MESO_MAXPOS_X,
            PARAM_MESO_MAXPOS_Y,
            PARAM_MESO_MAXPOS_Z,
            PARAM_MESO_CAMERA_CONFIG,
            PARAM_MESO_RESONANT_AMPLITUDE,
            PARAM_MESO_EXP_PATH,
            PARAM_WAVEFORM_OUTPATH,

            PARAM_FIRST_RESEARCH_CAMERA = 6000,	///Temporary parameters for research group
            PARAM_DFLIM_ACQUISITION_MODE = 6000,
            PARAM_DFLIM_RESYNC,
            PARAM_DFLIM_COARSE_SHIFT1,
            PARAM_DFLIM_COARSE_SHIFT2,
            PARAM_DFLIM_COARSE_SHIFT3,
            PARAM_DFLIM_COARSE_SHIFT4,
            PARAM_DFLIM_FINE_SHIFT1,
            PARAM_DFLIM_FINE_SHIFT2,
            PARAM_DFLIM_FINE_SHIFT3,
            PARAM_DFLIM_FINE_SHIFT4,
            PARAM_DFLIM_QUERY_CLOCK_FREQS,
            PARAM_DFLIM_FREQ_CLOCK0,
            PARAM_DFLIM_FREQ_CLOCK1,
            PARAM_DFLIM_FREQ_CLOCK2,
            PARAM_DFLIM_FREQ_CLOCK3,
            PARAM_DFLIM_FREQ_CLOCK4,
            PARAM_DFLIM_IMPLIED_FREQ_CLOCK0,
            PARAM_DFLIM_IMPLIED_FREQ_CLOCK1,
            PARAM_DFLIM_IMPLIED_FREQ_CLOCK2,
            PARAM_DFLIM_IMPLIED_FREQ_CLOCK3,
            PARAM_DFLIM_IMPLIED_FREQ_CLOCK4,
            PARAM_DFLIM_SYNC_DELAY,
            PARAM_DFLIM_RESYNC_DELAY,
            PARAM_DFLIM_RESYNC_EVERYLINE,
            PARAM_DFLIM_SAVE_SETTINGS,
            PARAM_DFLIM_SAVE_IMAGES_ON_LIVE_MODE,
            PARAM_DFLIM_FRAME_TYPE,
            PARAM_RESEARCH_CAMERA_10,
            PARAM_RESEARCH_CAMERA_11,
            PARAM_RESEARCH_CAMERA_12,
            PARAM_RESEARCH_CAMERA_13,
            PARAM_RESEARCH_CAMERA_14,
            PARAM_RESEARCH_CAMERA_15,
            PARAM_RESEARCH_CAMERA_16,
            PARAM_RESEARCH_CAMERA_17,
            PARAM_RESEARCH_CAMERA_18,
            PARAM_RESEARCH_CAMERA_19,
            PARAM_RESEARCH_CAMERA_20,
            PARAM_RESEARCH_CAMERA_21,
            PARAM_RESEARCH_CAMERA_22,
            PARAM_RESEARCH_CAMERA_23,
            PARAM_RESEARCH_CAMERA_24,
            PARAM_RESEARCH_CAMERA_25,
            PARAM_RESEARCH_CAMERA_26,
            PARAM_RESEARCH_CAMERA_27,
            PARAM_RESEARCH_CAMERA_28,
            PARAM_RESEARCH_CAMERA_29,
            PARAM_RESEARCH_CAMERA_30,
            PARAM_RESEARCH_CAMERA_31,
            PARAM_RESEARCH_CAMERA_32,
            PARAM_RESEARCH_CAMERA_33,
            PARAM_RESEARCH_CAMERA_34,
            PARAM_RESEARCH_CAMERA_35,
            PARAM_RESEARCH_CAMERA_36,
            PARAM_RESEARCH_CAMERA_37,
            PARAM_RESEARCH_CAMERA_38,
            PARAM_RESEARCH_CAMERA_39,
            PARAM_RESEARCH_CAMERA_40,
            PARAM_RESEARCH_CAMERA_41,
            PARAM_RESEARCH_CAMERA_42,
            PARAM_RESEARCH_CAMERA_43,
            PARAM_RESEARCH_CAMERA_44,
            PARAM_RESEARCH_CAMERA_45,
            PARAM_RESEARCH_CAMERA_46,
            PARAM_RESEARCH_CAMERA_47,
            PARAM_RESEARCH_CAMERA_48,
            PARAM_RESEARCH_CAMERA_49,
            PARAM_RESEARCH_CAMERA_50,
            PARAM_RESEARCH_CAMERA_51,
            PARAM_RESEARCH_CAMERA_52,
            PARAM_RESEARCH_CAMERA_53,
            PARAM_RESEARCH_CAMERA_54,
            PARAM_RESEARCH_CAMERA_55,
            PARAM_RESEARCH_CAMERA_56,
            PARAM_RESEARCH_CAMERA_57,
            PARAM_RESEARCH_CAMERA_58,
            PARAM_RESEARCH_CAMERA_59,
            PARAM_RESEARCH_CAMERA_60,
            PARAM_RESEARCH_CAMERA_61,
            PARAM_RESEARCH_CAMERA_62,
            PARAM_RESEARCH_CAMERA_63,
            PARAM_RESEARCH_CAMERA_64,
            PARAM_RESEARCH_CAMERA_65,
            PARAM_RESEARCH_CAMERA_66,
            PARAM_RESEARCH_CAMERA_67,
            PARAM_RESEARCH_CAMERA_68,
            PARAM_RESEARCH_CAMERA_69,
            PARAM_RESEARCH_CAMERA_70,
            PARAM_RESEARCH_CAMERA_71,
            PARAM_RESEARCH_CAMERA_72,
            PARAM_RESEARCH_CAMERA_73,
            PARAM_RESEARCH_CAMERA_74,
            PARAM_RESEARCH_CAMERA_75,
            PARAM_RESEARCH_CAMERA_76,
            PARAM_RESEARCH_CAMERA_77,
            PARAM_RESEARCH_CAMERA_78,
            PARAM_RESEARCH_CAMERA_79,
            PARAM_RESEARCH_CAMERA_80,
            PARAM_RESEARCH_CAMERA_81,
            PARAM_RESEARCH_CAMERA_82,
            PARAM_RESEARCH_CAMERA_83,
            PARAM_RESEARCH_CAMERA_84,
            PARAM_RESEARCH_CAMERA_85,
            PARAM_RESEARCH_CAMERA_86,
            PARAM_RESEARCH_CAMERA_87,
            PARAM_RESEARCH_CAMERA_88,
            PARAM_RESEARCH_CAMERA_89,
            PARAM_RESEARCH_CAMERA_90,
            PARAM_RESEARCH_CAMERA_91,
            PARAM_RESEARCH_CAMERA_92,
            PARAM_RESEARCH_CAMERA_93,
            PARAM_RESEARCH_CAMERA_94,
            PARAM_RESEARCH_CAMERA_95,
            PARAM_RESEARCH_CAMERA_96,
            PARAM_RESEARCH_CAMERA_97,
            PARAM_RESEARCH_CAMERA_98,
            PARAM_RESEARCH_CAMERA_99,
            PARAM_RESEARCH_CAMERA_100,

            PARAM_LAST_PARAM
        }

        public enum ParamType
        {
            TYPE_LONG,
            TYPE_DOUBLE,
            TYPE_STRING,
            TYPE_BUFFER
        }

        public enum ScanMode
        {
            SCANMODE_FIRST = 0,
            TWO_WAY_SCAN = 0,
            FORWARD_SCAN = 1,
            BACKWARD_SCAN = 2,
            CENTER = 3,
            BLEACH_SCAN = 4,
            SCANMODE_LAST
        }

        public enum StatusType
        {
            STATUS_BUSY = 0,
            STATUS_READY,
            STATUS_ERROR
        }

        public enum TriggerMode
        {
            FIRST_TRIGGER_MODE = 0,
            SW_SINGLE_FRAME = 0,
            SW_MULTI_FRAME,
            SW_FREE_RUN_MODE,
            HW_SINGLE_FRAME,
            HW_MULTI_FRAME_TRIGGER_FIRST,
            HW_MULTI_FRAME_TRIGGER_EACH,
            HW_MULTI_FRAME_TRIGGER_EACH_BULB,
            HW_TDI_TRIGGER_MODE,
            LAST_TRIGGER_MODE
        }

        #endregion Enumerations
    }

    public static class IDevice
    {
        #region Enumerations
        public enum ThorDAQ_DBB1_DIO_SLAVE_PORTS
        {
            Resonant_scanner_line_trigger_input = 0x00,
            Extern_line_trigger_input = 0x01,
            Extern_pixel_clock_input = 0x02,
            Scan_direction_output = 0x03,
            Horizontal_line_pulse_output = 0x04,
            Pixel_integration_output = 0x05,
            Start_of_frame_output = 0x06,
            Hardware_trigger_input = 0x07,
            External_SOF_input = 0x08,
            Pixel_clock_pulse_output = 0x09,
            Digital_Output_0 = 0x0A,
            Digital_Output_1 = 0x0B,
            Digital_Output_2 = 0x0C,
            Digital_Output_3 = 0x0D,
            Digital_Output_4 = 0x0E,
            Digital_Output_5 = 0x0F,
            Digital_Output_6 = 0x10,
            Digital_Output_7 = 0x11,
            Capture_Active = 0x12,
            Aux_GPIO_0 = 0x13,
            Aux_GPIO_1 = 0x14,
            Aux_GPIO_2 = 0x15,
            Aux_GPIO_3 = 0x16,
            Aux_GPIO_4 = 0x17
        };


        public enum ConnectionStatusType
        {
            CONNECTION_WARMING_UP,
            CONNECTION_READY,
            CONNECTION_UNAVAILABLE,
            CONNECTION_ERROR_STATE
        }

        public enum DeviceSetParamType
        {
            EXECUTION_NO_WAIT = 0,
            EXECUTION_WAIT,
            NO_EXECUTION
        }

        public enum HPLSControlMode
        {
            LOCAL = 1,
            REMOTE = 2
        }

        public enum HPLSLampMode
        {
            CLOSELOOP = 1,
            OPENLOOP = 2,
            ECO = 3
        }

        public enum HPLSShutterState
        {
            CLOSED = 1,
            OPEN = 2
        }

        public enum KuriosBandwidthMode
        {
            WIDE = 2,
            MEDIUM = 4,
            NARROW = 8
        }

        public enum KuriosControlMode
        {
            MANUAL = 1,
            SEQUENCE_INT,
            SEQUENCE_EXT,
            ANALOG_INT,
            ANALOG_EXT
        }

        public enum KuriosTriggerOutSignalMode
        {
            NON_FLIPPED = 0,
            FLIPPED = 1
        }

        public enum KuriosTriggerOutTimeMode
        {
            DISABLE_BULB = 0,
            ENABLE_BULB = 1
        }

        public enum Params
        {
            PARAM_FIRST_PARAM = 0,
            PARAM_DEVICE_TYPE = 0,
            PARAM_SHUTTER_POS,
            PARAM_SHUTTER_WAIT_TIME,
            PARAM_CONNECTION_STATUS,
            PARAM_FW_EX_POS = 100,
            PARAM_FW_EM_POS = 101,
            PARAM_FW_DIC_POS = 102,  ///<Also used as getter for EPI turret position
            PARAM_SHUTTER2_POS = 103,
            PARAM_SHUTTER3_POS = 104,
            PARAM_DEVICE_SERIALNUMBER = 105,
            PARAM_DEVICE_FIRMWAREVERSION = 106,
            PARAM_DEVICE_STATUS_MESSAGE = 107,

            PARAM_X_POS = 200,
            PARAM_X_HOME = 201,
            PARAM_X_ZERO = 202,
            PARAM_X_VELOCITY = 203,
            PARAM_X_STEPS_PER_MM = 204,
            PARAM_X_ACCEL = 205,
            PARAM_X_DECEL = 206,
            PARAM_X_POS_CURRENT = 207,
            PARAM_X_JOYSTICK_VELOCITY = 208,
            PARAM_X_STOP = 209,
            PARAM_X_VELOCITY_CURRENT = 210,
            PARAM_X_WAIT_UNTIL_SETTLED = 211,
            PARAM_X_POSITIVE = 212,
            PARAM_X_INVERT = 213,
            PARAM_X_MOTOR_TYPE = 214,
            PARAM_X_ANALOG_MODE = 215,	///<added for X analog input
            PARAM_X_STAGE_TYPE = 216,	///<added for X Piezo
            PARAM_X_SERVO_MODE = 217,	///<added for X Piezo Servo mode
            PARAM_X_FAST_START_POS = 218,
            PARAM_X_FAST_STOP_POS = 219,
            PARAM_X_FAST_VOLUME_TIME = 220,
            PARAM_X_FAST_FLYBACK_TIME = 221,
            PARAM_X_OUTPUT_POCKELS_REFERENCE = 222,
            PARAM_X_STATUS = 223,
            PARAM_X_JOG = 224,
            PARAM_X_POS_MOVE_BY = 225,

            PARAM_Y_POS = 300,
            PARAM_Y_HOME = 301,
            PARAM_Y_ZERO = 302,
            PARAM_Y_VELOCITY = 303,
            PARAM_Y_STEPS_PER_MM = 304,
            PARAM_Y_ACCEL = 305,
            PARAM_Y_DECEL = 306,
            PARAM_Y_POS_CURRENT = 307,
            PARAM_Y_JOYSTICK_VELOCITY = 308,
            PARAM_Y_STOP = 309,
            PARAM_Y_VELOCITY_CURRENT = 310,
            PARAM_Y_WAIT_UNTIL_SETTLED = 311,
            PARAM_Y_POSITIVE = 312,
            PARAM_Y_INVERT = 313,
            PARAM_Y_MOTOR_TYPE = 314,
            PARAM_Y_ANALOG_MODE = 315,	///<added for Y analog input
            PARAM_Y_STAGE_TYPE = 316,	///<added for Y piezo
            PARAM_Y_SERVO_MODE = 317,	///<added for Y Piezo Servo mode
            PARAM_Y_FAST_START_POS = 318,
            PARAM_Y_FAST_STOP_POS = 319,
            PARAM_Y_FAST_VOLUME_TIME = 320,
            PARAM_Y_FAST_FLYBACK_TIME = 321,
            PARAM_Y_OUTPUT_POCKELS_REFERENCE = 322,
            PARAM_Y_STATUS = 323,
            PARAM_Y_JOG = 324,
            PARAM_Y_POS_MOVE_BY = 325,

            PARAM_Z_POS = 400,
            PARAM_Z_HOME = 401,
            PARAM_Z_ZERO = 402,
            PARAM_Z_VELOCITY = 403,
            PARAM_Z_STEPS_PER_MM = 404,
            PARAM_Z_ACCEL = 405,
            PARAM_Z_DECEL = 406,
            PARAM_Z_POS_CURRENT = 407,
            PARAM_Z_JOYSTICK_VELOCITY = 408,
            PARAM_Z_STOP = 409,
            PARAM_Z_VELOCITY_CURRENT = 410,
            PARAM_Z_ANALOG_MODE = 411,///<Enables piezo to run in single point, waveform, staircase mode etc...
            PARAM_Z_FAST_START_POS = 412,///<Start location for volume scan
            PARAM_Z_FAST_STOP_POS = 413,///<Stop location for volume scan
            PARAM_Z_FAST_VOLUME_TIME = 414,///<Determines slope of volume scan or dwell time per step in staircase mode
            PARAM_Z_FAST_FLYBACK_TIME = 415,///Used to calculate flyback waveform
            PARAM_Z_STAGE_TYPE = 416,///<0-Stepper, 1-Piezo
            PARAM_Z_POSITIVE = 417,
            PARAM_Z_OUTPUT_POCKELS_REFERENCE = 418,
            PARAM_Z_INVERT = 419,
            PARAM_Z_MOTOR_TYPE = 420,
            PARAM_Z_SERVO_MODE = 421,	///<added for Z Piezo Servo mode
            PARAM_Z_OUTPUT_POCKELS_RESPONSE_TYPE = 422,	///<PockelsResponseType
            PARAM_Z_FAST_FLYBACK_TIME_ADJUST_MS = 423,///Used to adjust flyback time [msec] to fit one volume
            PARAM_Z_FAST_VOLUME_TIME_ADJUST_MS = 424,///Used to adjust volume dwell time [msec] to fit one volume
            PARAM_Z_FAST_STEP_TIME_ADJUST_MS = 425,///Used to adjust step dwell time [msec] to fit one volume
            PARAM_Z_FAST_STEP_TIME = 426,///Step dwell time in staircase mode
            PARAM_Z_FAST_STEP_BUFFER = 427,///Step position buffers in staircase mode
            PARAM_Z_FAST_INTRA_STEP_TIME = 428,///Intra step transition time in staircase mode
            PARAM_Z_POCKELS_MIN = 429,///Min Voltage from pockels calibration
            PARAM_Z_JOG = 430,
            PARAM_Z_STATUS = 431,
            PARAM_Z_POS_MOVE_BY = 432,
            PARAM_Z_ELEVATOR_POS_CURRENT = 433,

            PARAM_Z_ENABLE_HOLDING_VOLTAGE = 499,
            PARAM_AUTOFOCUS_POS = 500,
            PARAM_LOAD_AND_EJECT = 501,
            PARAM_TURRET_POS = 502,
            PARAM_AUTOFOCUS_OFFSET = 503,
            PARAM_AUTOFOCUS_FOUND = 504,
            PARAM_TURRET_SERIALNUMBER = 505,
            PARAM_TURRET_FIRMWAREUPDATE = 506,
            PARAM_TURRET_FIRMWAREVERSION = 507,
            PARAM_TURRET_STOP = 508,
            PARAM_TURRET_HOMED = 509,
            PARAM_TURRET_COLLISION = 510,
            PARAM_LAMP_POS = 600,
            PARAM_EPI_TURRET_POS = 601,
            PARAM_EPI_TURRET_AVAILABLE = 602,

            PARAM_PMT1_GAIN_POS = 700,
            PARAM_PMT1_ENABLE,
            PARAM_PMT2_GAIN_POS,
            PARAM_PMT2_ENABLE,
            PARAM_PMT3_GAIN_POS,
            PARAM_PMT3_ENABLE,
            PARAM_PMT4_GAIN_POS,
            PARAM_PMT4_ENABLE,
            PARAM_SCANNER_ENABLE,
            PARAM_POWER_ENABLE,
            PARAM_POWER_POS,
            PARAM_POWER_HOME,
            PARAM_POWER_VELOCITY,
            PARAM_PMT1_SAFETY,
            PARAM_PMT2_SAFETY,
            PARAM_PMT3_SAFETY,
            PARAM_PMT4_SAFETY,
            PARAM_SCANNER_ZOOM_POS,
            PARAM_PMT_SWITCH_POS,
            PARAM_PMT_SWITCH_POS_CURRENT,
            PARAM_SCANNER_ALIGN_POS,
            PARAM_SCANNER_ALIGN_POS_CURRENT,
            PARAM_PMT1_BANDWIDTH_POS,
            PARAM_PMT2_BANDWIDTH_POS,
            PARAM_PMT3_BANDWIDTH_POS,
            PARAM_PMT4_BANDWIDTH_POS,

            PARAM_TURRET_POS_CURRENT,

            PARAM_POWER_POS_CURRENT,
            PARAM_POWER2_POS,
            PARAM_POWER3_POS,
            PARAM_POWER2_HOME,
            PARAM_POWER3_HOME,

            PARAM_POWER_ZERO,
            PARAM_POWER2_ZERO,
            PARAM_POWER3_ZERO,
            PARAM_POWER_ZERO_POS,
            PARAM_POWER2_ZERO_POS,
            PARAM_POWER3_ZERO_POS,
            PARAM_POWER_SERIALNUMBER,
            PARAM_POWER2_SERIALNUMBER,
            PARAM_POWER3_SERIALNUMBER,
            PARAM_POWER_RAMP_BUFFER,

            PARAM_SCANNER_INIT_MODE,
            PARAM_CONTROL_UNIT_FIRMWAREUPDATE,
            PARAM_CONTROL_UNIT_FIRMWAREVERSION,
            PARAM_CONTROL_UNIT_SERIALNUMBER,
            PARAM_SCANNER_ENABLE_ANALOG,

            PARAM_POWER2_POS_CURRENT,
            PARAM_POWER_ENCODER_POS,
            PARAM_POWER2_ENCODER_POS,

            PARAM_PMT1_GAIN_POS_CURRENT_VOLTS = 750,
            PARAM_PMT1_OUTPUT_OFFSET,
            PARAM_PMT1_OUTPUT_OFFSET_CURRENT,
            PARAM_PMT1_SERIALNUMBER,
            PARAM_PMT2_GAIN_POS_CURRENT_VOLTS,
            PARAM_PMT2_OUTPUT_OFFSET,
            PARAM_PMT2_OUTPUT_OFFSET_CURRENT,
            PARAM_PMT2_SERIALNUMBER,
            PARAM_PMT3_GAIN_POS_CURRENT_VOLTS,
            PARAM_PMT3_OUTPUT_OFFSET,
            PARAM_PMT3_OUTPUT_OFFSET_CURRENT,
            PARAM_PMT3_SERIALNUMBER,
            PARAM_PMT4_GAIN_POS_CURRENT_VOLTS,
            PARAM_PMT4_OUTPUT_OFFSET,
            PARAM_PMT4_OUTPUT_OFFSET_CURRENT,
            PARAM_PMT4_SERIALNUMBER,
            PARAM_PMT5_GAIN_POS,
            PARAM_PMT5_ENABLE,
            PARAM_PMT5_BANDWIDTH_POS,
            PARAM_PMT5_SAFETY,
            PARAM_PMT5_GAIN_POS_CURRENT_VOLTS,
            PARAM_PMT5_OUTPUT_OFFSET,
            PARAM_PMT5_OUTPUT_OFFSET_CURRENT,
            PARAM_PMT5_SERIALNUMBER,
            PARAM_PMT6_GAIN_POS,
            PARAM_PMT6_ENABLE,
            PARAM_PMT6_BANDWIDTH_POS,
            PARAM_PMT6_SAFETY,
            PARAM_PMT6_GAIN_POS_CURRENT_VOLTS,
            PARAM_PMT6_OUTPUT_OFFSET,
            PARAM_PMT6_OUTPUT_OFFSET_CURRENT,
            PARAM_PMT6_SERIALNUMBER,
            PARAM_CONNECTED_PMTS,
            PARAM_PMT1_FIRMWAREUPDATE,
            PARAM_PMT2_FIRMWAREUPDATE,
            PARAM_PMT3_FIRMWAREUPDATE,
            PARAM_PMT4_FIRMWAREUPDATE,
            PARAM_PMT5_FIRMWAREUPDATE,
            PARAM_PMT6_FIRMWAREUPDATE,
            PARAM_PMT1_TYPE,
            PARAM_PMT2_TYPE,
            PARAM_PMT3_TYPE,
            PARAM_PMT4_TYPE,
            PARAM_PMT5_TYPE,
            PARAM_PMT6_TYPE,

            PARAM_EXP_RATIO = 800,
            PARAM_MOT0_POS,
            PARAM_MOT1_POS,
            PARAM_BMEXP_MODE,
            PARAM_EXP_RATIO2,
            PARAM_EXP_WAVELENGTH,
            PARAM_EXP_WAVELENGTH2,
            PARAM_EXP_SERIALNUMBER,
            PARAM_EXP_SERIALNUMBER2,

            PARAM_LASER_POWER = 824,
            PARAM_LASER1_ENABLE = 825,
            PARAM_LASER1_POS,
            PARAM_LASER2_ENABLE,
            PARAM_LASER2_POS,
            PARAM_LASER3_ENABLE,
            PARAM_LASER3_POS,
            PARAM_LASER4_ENABLE,
            PARAM_LASER4_POS,
            PARAM_LASER1_POWER,
            PARAM_LASER2_POWER,
            PARAM_LASER3_POWER,
            PARAM_LASER4_POWER,
            PARAM_LASER1_POWER_CURRENT,
            PARAM_LASER2_POWER_CURRENT,
            PARAM_LASER3_POWER_CURRENT,
            PARAM_LASER4_POWER_CURRENT,
            PARAM_LASER1_POS_CURRENT,
            PARAM_LASER2_POS_CURRENT,
            PARAM_LASER3_POS_CURRENT,
            PARAM_LASER4_POS_CURRENT,
            PARAM_LASER1_SHUTTER_POS,
            PARAM_LASER1_SHUTTER_POS_CURRENT,
            PARAM_LASER1_SHUTTER2_POS,
            PARAM_LASER1_SHUTTER2_POS_CURRENT,
            PARAM_LASER1_SEQ,

            PARAM_PINHOLE_POS = 900,
            PARAM_PINHOLE_POS_CURRENT,
            PARAM_PINHOLE_ALIGNMENT_POS,
            PARAM_PINHOLE_ALIGNMENT_POS_CURRENT,
            PARAM_PINHOLE_ALIGNMENT_MODE,
            PARAM_PINHOLE_SEPARATION,
            PARAM_PINHOLE_NON_PINHOLE_FREQUENCY,

            PARAM_PMT1_BANDWIDTH_POS_CURRENT = 920,
            PARAM_PMT2_BANDWIDTH_POS_CURRENT,
            PARAM_PMT3_BANDWIDTH_POS_CURRENT,
            PARAM_PMT4_BANDWIDTH_POS_CURRENT,
            PARAM_PMT5_BANDWIDTH_POS_CURRENT,
            PARAM_PMT6_BANDWIDTH_POS_CURRENT,
            PARAM_PMT1_SATURATIONS,
            PARAM_PMT2_SATURATIONS,
            PARAM_PMT3_SATURATIONS,
            PARAM_PMT4_SATURATIONS,
            PARAM_PMT5_SATURATIONS,
            PARAM_PMT6_SATURATIONS,

            PARAM_STOP = 1000,

            PARAM_DECODER_INCREMENT = 1100,
            PARAM_DECODER_BLANKING_PERIOD,
            PARAM_DECODER_FRACTIONAL,
            PARAM_DECODER_TRIGGER_PULSES_PER_FRAME,
            PARAM_DECODER_DIRECTION,
            PARAM_DECODER_HOME_POSITION,
            PARAM_DECODER_FSTART,
            PARAM_DECODER_ENCODER_PER_FRAME,
            PARAM_DECODER_RUN,
            PARAM_DECODER_FRAME_COUNT,
            PARAM_DECODER_APPLY,

            PARAM_SLM_FUNC_MODE = 1130,
            PARAM_SLM_ARRAY_ID,
            PARAM_SLM_POINTS_ARRAY,
            PARAM_SLM_PIXEL_X,
            PARAM_SLM_PIXEL_Y,
            PARAM_SLM_BMP_FILENAME,
            PARAM_SLM_RESET_AFFINE,
            PARAM_SLM_BLANK,
            PARAM_SLM_TIMEOUT,
            PARAM_SLM_RUNTIME_CALC,
            PARAM_SLM_SEQ_FILENAME,
            PARAM_SLM_CALIB_Z,
            PARAM_SLM_NA,
            PARAM_SLM_WAVELENGTH,
            PARAM_SLM_WAVELENGTH_SELECT,
            PARAM_SLM_3D,
            PARAM_SLM_PHASE_DIRECT,

            PARAM_R_POS = 1200,
            PARAM_R_HOME = 1201,
            PARAM_R_ZERO = 1202,
            PARAM_R_VELOCITY = 1203,
            PARAM_R_STEPS_PER_MM = 1204,
            PARAM_R_ACCEL = 1205,
            PARAM_R_DECEL = 1206,
            PARAM_R_POS_CURRENT = 1207,
            PARAM_R_JOYSTICK_VELOCITY = 1208,
            PARAM_R_STOP = 1209,
            PARAM_R_VELOCITY_CURRENT = 1210,
            PARAM_R_STATUS = 1211,
            PARAM_R_JOG = 1212,
            PARAM_R_INVERT = 1213,
            PARAM_R_POS_MOVE_BY = 1214,

            PARAM_CONDENSER_POS = 1215,
            PARAM_CONDENSER_JOG = 1216,
            PARAM_CONDENSER_POS_CURRENT = 1217,
            PARAM_CONDENSER_STATUS = 1218,
            PARAM_CONDENSER_ZERO = 1219,
            PARAM_CONDENSER_INVERT = 1220,
            PARAM_CONDENSER_STOP = 1221,
            PARAM_CONDENSER_POS_MOVE_BY = 1222,

            PARAM_ECU_TWO_WAY_ZONE_1 = 1300,
            PARAM_ECU_TWO_WAY_ZONE_2,
            PARAM_ECU_TWO_WAY_ZONE_3,
            PARAM_ECU_TWO_WAY_ZONE_4,
            PARAM_ECU_TWO_WAY_ZONE_5,
            PARAM_ECU_TWO_WAY_ZONE_6,
            PARAM_ECU_TWO_WAY_ZONE_7,
            PARAM_ECU_TWO_WAY_ZONE_8,
            PARAM_ECU_TWO_WAY_ZONE_9,
            PARAM_ECU_TWO_WAY_ZONE_10,
            PARAM_ECU_TWO_WAY_ZONE_11,
            PARAM_ECU_TWO_WAY_ZONE_12,
            PARAM_ECU_TWO_WAY_ZONE_13,
            PARAM_ECU_TWO_WAY_ZONE_14,
            PARAM_ECU_TWO_WAY_ZONE_15,
            PARAM_ECU_TWO_WAY_ZONE_16,
            PARAM_ECU_TWO_WAY_ZONE_17,
            PARAM_ECU_TWO_WAY_ZONE_18,
            PARAM_ECU_TWO_WAY_ZONE_19,
            PARAM_ECU_TWO_WAY_ZONE_20,
            PARAM_ECU_TWO_WAY_ZONE_21,
            PARAM_ECU_TWO_WAY_ZONE_22,
            PARAM_ECU_TWO_WAY_ZONE_23,
            PARAM_ECU_TWO_WAY_ZONE_24,
            PARAM_ECU_TWO_WAY_ZONE_25,
            PARAM_ECU_TWO_WAY_ZONE_26,
            PARAM_ECU_TWO_WAY_ZONE_27,
            PARAM_ECU_TWO_WAY_ZONE_28,
            PARAM_ECU_TWO_WAY_ZONE_29,
            PARAM_ECU_TWO_WAY_ZONE_30,
            PARAM_ECU_TWO_WAY_ZONE_31,
            PARAM_ECU_TWO_WAY_ZONE_32,
            PARAM_ECU_TWO_WAY_ZONE_33,
            PARAM_ECU_TWO_WAY_ZONE_34,
            PARAM_ECU_TWO_WAY_ZONE_35,
            PARAM_ECU_TWO_WAY_ZONE_36,
            PARAM_ECU_TWO_WAY_ZONE_37,
            PARAM_ECU_TWO_WAY_ZONE_38,
            PARAM_ECU_TWO_WAY_ZONE_39,
            PARAM_ECU_TWO_WAY_ZONE_40,
            PARAM_ECU_TWO_WAY_ZONE_41,
            PARAM_ECU_TWO_WAY_ZONE_42,
            PARAM_ECU_TWO_WAY_ZONE_43,
            PARAM_ECU_TWO_WAY_ZONE_44,
            PARAM_ECU_TWO_WAY_ZONE_45,
            PARAM_ECU_TWO_WAY_ZONE_46,
            PARAM_ECU_TWO_WAY_ZONE_47,
            PARAM_ECU_TWO_WAY_ZONE_48,
            PARAM_ECU_TWO_WAY_ZONE_49,
            PARAM_ECU_TWO_WAY_ZONE_50,
            PARAM_ECU_TWO_WAY_ZONE_51,
            PARAM_ECU_TWO_WAY_ZONE_52,
            PARAM_ECU_TWO_WAY_ZONE_53,
            PARAM_ECU_TWO_WAY_ZONE_54,
            PARAM_ECU_TWO_WAY_ZONE_55,
            PARAM_ECU_TWO_WAY_ZONE_56,
            PARAM_ECU_TWO_WAY_ZONE_57,
            PARAM_ECU_TWO_WAY_ZONE_58,
            PARAM_ECU_TWO_WAY_ZONE_59,
            PARAM_ECU_TWO_WAY_ZONE_60,
            PARAM_ECU_TWO_WAY_ZONE_61,
            PARAM_ECU_TWO_WAY_ZONE_62,
            PARAM_ECU_TWO_WAY_ZONE_63,
            PARAM_ECU_TWO_WAY_ZONE_64,
            PARAM_ECU_TWO_WAY_ZONE_65,
            PARAM_ECU_TWO_WAY_ZONE_66,
            PARAM_ECU_TWO_WAY_ZONE_67,
            PARAM_ECU_TWO_WAY_ZONE_68,
            PARAM_ECU_TWO_WAY_ZONE_69,
            PARAM_ECU_TWO_WAY_ZONE_70,
            PARAM_ECU_TWO_WAY_ZONE_71,
            PARAM_ECU_TWO_WAY_ZONE_72,
            PARAM_ECU_TWO_WAY_ZONE_73,
            PARAM_ECU_TWO_WAY_ZONE_74,
            PARAM_ECU_TWO_WAY_ZONE_75,
            PARAM_ECU_TWO_WAY_ZONE_76,
            PARAM_ECU_TWO_WAY_ZONE_77,
            PARAM_ECU_TWO_WAY_ZONE_78,
            PARAM_ECU_TWO_WAY_ZONE_79,
            PARAM_ECU_TWO_WAY_ZONE_80,
            PARAM_ECU_TWO_WAY_ZONE_81,
            PARAM_ECU_TWO_WAY_ZONE_82,
            PARAM_ECU_TWO_WAY_ZONE_83,
            PARAM_ECU_TWO_WAY_ZONE_84,
            PARAM_ECU_TWO_WAY_ZONE_85,
            PARAM_ECU_TWO_WAY_ZONE_86,
            PARAM_ECU_TWO_WAY_ZONE_87,
            PARAM_ECU_TWO_WAY_ZONE_88,
            PARAM_ECU_TWO_WAY_ZONE_89,
            PARAM_ECU_TWO_WAY_ZONE_90,
            PARAM_ECU_TWO_WAY_ZONE_91,
            PARAM_ECU_TWO_WAY_ZONE_92,
            PARAM_ECU_TWO_WAY_ZONE_93,
            PARAM_ECU_TWO_WAY_ZONE_94,
            PARAM_ECU_TWO_WAY_ZONE_95,
            PARAM_ECU_TWO_WAY_ZONE_96,
            PARAM_ECU_TWO_WAY_ZONE_97,
            PARAM_ECU_TWO_WAY_ZONE_98,
            PARAM_ECU_TWO_WAY_ZONE_99,
            PARAM_ECU_TWO_WAY_ZONE_100,
            PARAM_ECU_TWO_WAY_ZONE_101,
            PARAM_ECU_TWO_WAY_ZONE_102,
            PARAM_ECU_TWO_WAY_ZONE_103,
            PARAM_ECU_TWO_WAY_ZONE_104,
            PARAM_ECU_TWO_WAY_ZONE_105,
            PARAM_ECU_TWO_WAY_ZONE_106,
            PARAM_ECU_TWO_WAY_ZONE_107,
            PARAM_ECU_TWO_WAY_ZONE_108,
            PARAM_ECU_TWO_WAY_ZONE_109,
            PARAM_ECU_TWO_WAY_ZONE_110,
            PARAM_ECU_TWO_WAY_ZONE_111,
            PARAM_ECU_TWO_WAY_ZONE_112,
            PARAM_ECU_TWO_WAY_ZONE_113,
            PARAM_ECU_TWO_WAY_ZONE_114,
            PARAM_ECU_TWO_WAY_ZONE_115,
            PARAM_ECU_TWO_WAY_ZONE_116,
            PARAM_ECU_TWO_WAY_ZONE_117,
            PARAM_ECU_TWO_WAY_ZONE_118,
            PARAM_ECU_TWO_WAY_ZONE_119,
            PARAM_ECU_TWO_WAY_ZONE_120,
            PARAM_ECU_TWO_WAY_ZONE_121,
            PARAM_ECU_TWO_WAY_ZONE_122,
            PARAM_ECU_TWO_WAY_ZONE_123,
            PARAM_ECU_TWO_WAY_ZONE_124,
            PARAM_ECU_TWO_WAY_ZONE_125,
            PARAM_ECU_TWO_WAY_ZONE_126,
            PARAM_ECU_TWO_WAY_ZONE_127,
            PARAM_ECU_TWO_WAY_ZONE_128,
            PARAM_ECU_TWO_WAY_ZONE_129,
            PARAM_ECU_TWO_WAY_ZONE_130,
            PARAM_ECU_TWO_WAY_ZONE_131,
            PARAM_ECU_TWO_WAY_ZONE_132,
            PARAM_ECU_TWO_WAY_ZONE_133,
            PARAM_ECU_TWO_WAY_ZONE_134,
            PARAM_ECU_TWO_WAY_ZONE_135,
            PARAM_ECU_TWO_WAY_ZONE_136,
            PARAM_ECU_TWO_WAY_ZONE_137,
            PARAM_ECU_TWO_WAY_ZONE_138,
            PARAM_ECU_TWO_WAY_ZONE_139,
            PARAM_ECU_TWO_WAY_ZONE_140,
            PARAM_ECU_TWO_WAY_ZONE_141,
            PARAM_ECU_TWO_WAY_ZONE_142,
            PARAM_ECU_TWO_WAY_ZONE_143,
            PARAM_ECU_TWO_WAY_ZONE_144,
            PARAM_ECU_TWO_WAY_ZONE_145,
            PARAM_ECU_TWO_WAY_ZONE_146,
            PARAM_ECU_TWO_WAY_ZONE_147,
            PARAM_ECU_TWO_WAY_ZONE_148,
            PARAM_ECU_TWO_WAY_ZONE_149,
            PARAM_ECU_TWO_WAY_ZONE_150,
            PARAM_ECU_TWO_WAY_ZONE_151,
            PARAM_ECU_TWO_WAY_ZONE_152,
            PARAM_ECU_TWO_WAY_ZONE_153,
            PARAM_ECU_TWO_WAY_ZONE_154,
            PARAM_ECU_TWO_WAY_ZONE_155,
            PARAM_ECU_TWO_WAY_ZONE_156,
            PARAM_ECU_TWO_WAY_ZONE_157,
            PARAM_ECU_TWO_WAY_ZONE_158,
            PARAM_ECU_TWO_WAY_ZONE_159,
            PARAM_ECU_TWO_WAY_ZONE_160,
            PARAM_ECU_TWO_WAY_ZONE_161,
            PARAM_ECU_TWO_WAY_ZONE_162,
            PARAM_ECU_TWO_WAY_ZONE_163,
            PARAM_ECU_TWO_WAY_ZONE_164,
            PARAM_ECU_TWO_WAY_ZONE_165,
            PARAM_ECU_TWO_WAY_ZONE_166,
            PARAM_ECU_TWO_WAY_ZONE_167,
            PARAM_ECU_TWO_WAY_ZONE_168,
            PARAM_ECU_TWO_WAY_ZONE_169,
            PARAM_ECU_TWO_WAY_ZONE_170,
            PARAM_ECU_TWO_WAY_ZONE_171,
            PARAM_ECU_TWO_WAY_ZONE_172,
            PARAM_ECU_TWO_WAY_ZONE_173,
            PARAM_ECU_TWO_WAY_ZONE_174,
            PARAM_ECU_TWO_WAY_ZONE_175,
            PARAM_ECU_TWO_WAY_ZONE_176,
            PARAM_ECU_TWO_WAY_ZONE_177,
            PARAM_ECU_TWO_WAY_ZONE_178,
            PARAM_ECU_TWO_WAY_ZONE_179,
            PARAM_ECU_TWO_WAY_ZONE_180,
            PARAM_ECU_TWO_WAY_ZONE_181,
            PARAM_ECU_TWO_WAY_ZONE_182,
            PARAM_ECU_TWO_WAY_ZONE_183,
            PARAM_ECU_TWO_WAY_ZONE_184,
            PARAM_ECU_TWO_WAY_ZONE_185,
            PARAM_ECU_TWO_WAY_ZONE_186,
            PARAM_ECU_TWO_WAY_ZONE_187,
            PARAM_ECU_TWO_WAY_ZONE_188,
            PARAM_ECU_TWO_WAY_ZONE_189,
            PARAM_ECU_TWO_WAY_ZONE_190,
            PARAM_ECU_TWO_WAY_ZONE_191,
            PARAM_ECU_TWO_WAY_ZONE_192,
            PARAM_ECU_TWO_WAY_ZONE_193,
            PARAM_ECU_TWO_WAY_ZONE_194,
            PARAM_ECU_TWO_WAY_ZONE_195,
            PARAM_ECU_TWO_WAY_ZONE_196,
            PARAM_ECU_TWO_WAY_ZONE_197,
            PARAM_ECU_TWO_WAY_ZONE_198,
            PARAM_ECU_TWO_WAY_ZONE_199,
            PARAM_ECU_TWO_WAY_ZONE_200,
            PARAM_ECU_TWO_WAY_ZONE_201,
            PARAM_ECU_TWO_WAY_ZONE_202,
            PARAM_ECU_TWO_WAY_ZONE_203,
            PARAM_ECU_TWO_WAY_ZONE_204,
            PARAM_ECU_TWO_WAY_ZONE_205,
            PARAM_ECU_TWO_WAY_ZONE_206,
            PARAM_ECU_TWO_WAY_ZONE_207,
            PARAM_ECU_TWO_WAY_ZONE_208,
            PARAM_ECU_TWO_WAY_ZONE_209,
            PARAM_ECU_TWO_WAY_ZONE_210,
            PARAM_ECU_TWO_WAY_ZONE_211,
            PARAM_ECU_TWO_WAY_ZONE_212,
            PARAM_ECU_TWO_WAY_ZONE_213,
            PARAM_ECU_TWO_WAY_ZONE_214,
            PARAM_ECU_TWO_WAY_ZONE_215,
            PARAM_ECU_TWO_WAY_ZONE_216,
            PARAM_ECU_TWO_WAY_ZONE_217,
            PARAM_ECU_TWO_WAY_ZONE_218,
            PARAM_ECU_TWO_WAY_ZONE_219,
            PARAM_ECU_TWO_WAY_ZONE_220,
            PARAM_ECU_TWO_WAY_ZONE_221,
            PARAM_ECU_TWO_WAY_ZONE_222,
            PARAM_ECU_TWO_WAY_ZONE_223,
            PARAM_ECU_TWO_WAY_ZONE_224,
            PARAM_ECU_TWO_WAY_ZONE_225,
            PARAM_ECU_TWO_WAY_ZONE_226,
            PARAM_ECU_TWO_WAY_ZONE_227,
            PARAM_ECU_TWO_WAY_ZONE_228,
            PARAM_ECU_TWO_WAY_ZONE_229,
            PARAM_ECU_TWO_WAY_ZONE_230,
            PARAM_ECU_TWO_WAY_ZONE_231,
            PARAM_ECU_TWO_WAY_ZONE_232,
            PARAM_ECU_TWO_WAY_ZONE_233,
            PARAM_ECU_TWO_WAY_ZONE_234,
            PARAM_ECU_TWO_WAY_ZONE_235,
            PARAM_ECU_TWO_WAY_ZONE_236,
            PARAM_ECU_TWO_WAY_ZONE_237,
            PARAM_ECU_TWO_WAY_ZONE_238,
            PARAM_ECU_TWO_WAY_ZONE_239,
            PARAM_ECU_TWO_WAY_ZONE_240,
            PARAM_ECU_TWO_WAY_ZONE_241,
            PARAM_ECU_TWO_WAY_ZONE_242,
            PARAM_ECU_TWO_WAY_ZONE_243,
            PARAM_ECU_TWO_WAY_ZONE_244,
            PARAM_ECU_TWO_WAY_ZONE_245,
            PARAM_ECU_TWO_WAY_ZONE_246,
            PARAM_ECU_TWO_WAY_ZONE_247,
            PARAM_ECU_TWO_WAY_ZONE_248,
            PARAM_ECU_TWO_WAY_ZONE_249,
            PARAM_ECU_TWO_WAY_ZONE_250,
            PARAM_ECU_TWO_WAY_ZONE_251,

            PARAM_EPHYS_DIG_LINE_IN_1,
            PARAM_EPHYS_DIG_LINE_IN_2,
            PARAM_EPHYS_DIG_LINE_IN_3,
            PARAM_EPHYS_DIG_LINE_IN_4,
            PARAM_EPHYS_DIG_LINE_OUT_1,
            PARAM_EPHYS_DIG_LINE_OUT_2,
            PARAM_EPHYS_DIG_LINE_OUT_3,
            PARAM_EPHYS_DIG_LINE_OUT_4,
            PARAM_EPHYS_DIG_LINE_OUT_5,
            PARAM_EPHYS_DIG_LINE_OUT_6,
            PARAM_EPHYS_DIG_LINE_OUT_7,
            PARAM_EPHYS_DIG_LINE_OUT_8,
            PARAM_EPHYS_MEASURE,
            PARAM_EPHYS_MEASURE_CONFIGURE,
            PARAM_EPHYS_MEASURE_RATE,

            PARAM_ECU_TWO_WAY_ZONE_FINE_1,
            PARAM_ECU_TWO_WAY_ZONE_FINE_2,
            PARAM_ECU_TWO_WAY_ZONE_FINE_3,
            PARAM_ECU_TWO_WAY_ZONE_FINE_4,
            PARAM_ECU_TWO_WAY_ZONE_FINE_5,
            PARAM_ECU_TWO_WAY_ZONE_FINE_6,
            PARAM_ECU_TWO_WAY_ZONE_FINE_7,
            PARAM_ECU_TWO_WAY_ZONE_FINE_8,
            PARAM_ECU_TWO_WAY_ZONE_FINE_9,
            PARAM_ECU_TWO_WAY_ZONE_FINE_10,
            PARAM_ECU_TWO_WAY_ZONE_FINE_11,
            PARAM_ECU_TWO_WAY_ZONE_FINE_12,
            PARAM_ECU_TWO_WAY_ZONE_FINE_13,
            PARAM_ECU_TWO_WAY_ZONE_FINE_14,
            PARAM_ECU_TWO_WAY_ZONE_FINE_15,
            PARAM_ECU_TWO_WAY_ZONE_FINE_16,
            PARAM_ECU_TWO_WAY_ZONE_FINE_17,
            PARAM_ECU_TWO_WAY_ZONE_FINE_18,
            PARAM_ECU_TWO_WAY_ZONE_FINE_19,
            PARAM_ECU_TWO_WAY_ZONE_FINE_20,
            PARAM_ECU_TWO_WAY_ZONE_FINE_21,
            PARAM_ECU_TWO_WAY_ZONE_FINE_22,
            PARAM_ECU_TWO_WAY_ZONE_FINE_23,
            PARAM_ECU_TWO_WAY_ZONE_FINE_24,
            PARAM_ECU_TWO_WAY_ZONE_FINE_25,
            PARAM_ECU_TWO_WAY_ZONE_FINE_26,
            PARAM_ECU_TWO_WAY_ZONE_FINE_27,
            PARAM_ECU_TWO_WAY_ZONE_FINE_28,
            PARAM_ECU_TWO_WAY_ZONE_FINE_29,
            PARAM_ECU_TWO_WAY_ZONE_FINE_30,
            PARAM_ECU_TWO_WAY_ZONE_FINE_31,
            PARAM_ECU_TWO_WAY_ZONE_FINE_32,
            PARAM_ECU_TWO_WAY_ZONE_FINE_33,
            PARAM_ECU_TWO_WAY_ZONE_FINE_34,
            PARAM_ECU_TWO_WAY_ZONE_FINE_35,
            PARAM_ECU_TWO_WAY_ZONE_FINE_36,
            PARAM_ECU_TWO_WAY_ZONE_FINE_37,
            PARAM_ECU_TWO_WAY_ZONE_FINE_38,
            PARAM_ECU_TWO_WAY_ZONE_FINE_39,
            PARAM_ECU_TWO_WAY_ZONE_FINE_40,
            PARAM_ECU_TWO_WAY_ZONE_FINE_41,
            PARAM_ECU_TWO_WAY_ZONE_FINE_42,
            PARAM_ECU_TWO_WAY_ZONE_FINE_43,
            PARAM_ECU_TWO_WAY_ZONE_FINE_44,
            PARAM_ECU_TWO_WAY_ZONE_FINE_45,
            PARAM_ECU_TWO_WAY_ZONE_FINE_46,
            PARAM_ECU_TWO_WAY_ZONE_FINE_47,
            PARAM_ECU_TWO_WAY_ZONE_FINE_48,
            PARAM_ECU_TWO_WAY_ZONE_FINE_49,
            PARAM_ECU_TWO_WAY_ZONE_FINE_50,
            PARAM_ECU_TWO_WAY_ZONE_FINE_51,
            PARAM_ECU_TWO_WAY_ZONE_FINE_52,
            PARAM_ECU_TWO_WAY_ZONE_FINE_53,
            PARAM_ECU_TWO_WAY_ZONE_FINE_54,
            PARAM_ECU_TWO_WAY_ZONE_FINE_55,
            PARAM_ECU_TWO_WAY_ZONE_FINE_56,
            PARAM_ECU_TWO_WAY_ZONE_FINE_57,
            PARAM_ECU_TWO_WAY_ZONE_FINE_58,
            PARAM_ECU_TWO_WAY_ZONE_FINE_59,
            PARAM_ECU_TWO_WAY_ZONE_FINE_60,
            PARAM_ECU_TWO_WAY_ZONE_FINE_61,
            PARAM_ECU_TWO_WAY_ZONE_FINE_62,
            PARAM_ECU_TWO_WAY_ZONE_FINE_63,
            PARAM_ECU_TWO_WAY_ZONE_FINE_64,
            PARAM_ECU_TWO_WAY_ZONE_FINE_65,
            PARAM_ECU_TWO_WAY_ZONE_FINE_66,
            PARAM_ECU_TWO_WAY_ZONE_FINE_67,
            PARAM_ECU_TWO_WAY_ZONE_FINE_68,
            PARAM_ECU_TWO_WAY_ZONE_FINE_69,
            PARAM_ECU_TWO_WAY_ZONE_FINE_70,
            PARAM_ECU_TWO_WAY_ZONE_FINE_71,
            PARAM_ECU_TWO_WAY_ZONE_FINE_72,
            PARAM_ECU_TWO_WAY_ZONE_FINE_73,
            PARAM_ECU_TWO_WAY_ZONE_FINE_74,
            PARAM_ECU_TWO_WAY_ZONE_FINE_75,
            PARAM_ECU_TWO_WAY_ZONE_FINE_76,
            PARAM_ECU_TWO_WAY_ZONE_FINE_77,
            PARAM_ECU_TWO_WAY_ZONE_FINE_78,
            PARAM_ECU_TWO_WAY_ZONE_FINE_79,
            PARAM_ECU_TWO_WAY_ZONE_FINE_80,
            PARAM_ECU_TWO_WAY_ZONE_FINE_81,
            PARAM_ECU_TWO_WAY_ZONE_FINE_82,
            PARAM_ECU_TWO_WAY_ZONE_FINE_83,
            PARAM_ECU_TWO_WAY_ZONE_FINE_84,
            PARAM_ECU_TWO_WAY_ZONE_FINE_85,
            PARAM_ECU_TWO_WAY_ZONE_FINE_86,
            PARAM_ECU_TWO_WAY_ZONE_FINE_87,
            PARAM_ECU_TWO_WAY_ZONE_FINE_88,
            PARAM_ECU_TWO_WAY_ZONE_FINE_89,
            PARAM_ECU_TWO_WAY_ZONE_FINE_90,
            PARAM_ECU_TWO_WAY_ZONE_FINE_91,
            PARAM_ECU_TWO_WAY_ZONE_FINE_92,
            PARAM_ECU_TWO_WAY_ZONE_FINE_93,
            PARAM_ECU_TWO_WAY_ZONE_FINE_94,
            PARAM_ECU_TWO_WAY_ZONE_FINE_95,
            PARAM_ECU_TWO_WAY_ZONE_FINE_96,
            PARAM_ECU_TWO_WAY_ZONE_FINE_97,
            PARAM_ECU_TWO_WAY_ZONE_FINE_98,
            PARAM_ECU_TWO_WAY_ZONE_FINE_99,
            PARAM_ECU_TWO_WAY_ZONE_FINE_100,
            PARAM_ECU_TWO_WAY_ZONE_FINE_101,
            PARAM_ECU_TWO_WAY_ZONE_FINE_102,
            PARAM_ECU_TWO_WAY_ZONE_FINE_103,
            PARAM_ECU_TWO_WAY_ZONE_FINE_104,
            PARAM_ECU_TWO_WAY_ZONE_FINE_105,
            PARAM_ECU_TWO_WAY_ZONE_FINE_106,
            PARAM_ECU_TWO_WAY_ZONE_FINE_107,
            PARAM_ECU_TWO_WAY_ZONE_FINE_108,
            PARAM_ECU_TWO_WAY_ZONE_FINE_109,
            PARAM_ECU_TWO_WAY_ZONE_FINE_110,
            PARAM_ECU_TWO_WAY_ZONE_FINE_111,
            PARAM_ECU_TWO_WAY_ZONE_FINE_112,
            PARAM_ECU_TWO_WAY_ZONE_FINE_113,
            PARAM_ECU_TWO_WAY_ZONE_FINE_114,
            PARAM_ECU_TWO_WAY_ZONE_FINE_115,
            PARAM_ECU_TWO_WAY_ZONE_FINE_116,
            PARAM_ECU_TWO_WAY_ZONE_FINE_117,
            PARAM_ECU_TWO_WAY_ZONE_FINE_118,
            PARAM_ECU_TWO_WAY_ZONE_FINE_119,
            PARAM_ECU_TWO_WAY_ZONE_FINE_120,
            PARAM_ECU_TWO_WAY_ZONE_FINE_121,
            PARAM_ECU_TWO_WAY_ZONE_FINE_122,
            PARAM_ECU_TWO_WAY_ZONE_FINE_123,
            PARAM_ECU_TWO_WAY_ZONE_FINE_124,
            PARAM_ECU_TWO_WAY_ZONE_FINE_125,
            PARAM_ECU_TWO_WAY_ZONE_FINE_126,
            PARAM_ECU_TWO_WAY_ZONE_FINE_127,
            PARAM_ECU_TWO_WAY_ZONE_FINE_128,
            PARAM_ECU_TWO_WAY_ZONE_FINE_129,
            PARAM_ECU_TWO_WAY_ZONE_FINE_130,
            PARAM_ECU_TWO_WAY_ZONE_FINE_131,
            PARAM_ECU_TWO_WAY_ZONE_FINE_132,
            PARAM_ECU_TWO_WAY_ZONE_FINE_133,
            PARAM_ECU_TWO_WAY_ZONE_FINE_134,
            PARAM_ECU_TWO_WAY_ZONE_FINE_135,
            PARAM_ECU_TWO_WAY_ZONE_FINE_136,
            PARAM_ECU_TWO_WAY_ZONE_FINE_137,
            PARAM_ECU_TWO_WAY_ZONE_FINE_138,
            PARAM_ECU_TWO_WAY_ZONE_FINE_139,
            PARAM_ECU_TWO_WAY_ZONE_FINE_140,
            PARAM_ECU_TWO_WAY_ZONE_FINE_141,
            PARAM_ECU_TWO_WAY_ZONE_FINE_142,
            PARAM_ECU_TWO_WAY_ZONE_FINE_143,
            PARAM_ECU_TWO_WAY_ZONE_FINE_144,
            PARAM_ECU_TWO_WAY_ZONE_FINE_145,
            PARAM_ECU_TWO_WAY_ZONE_FINE_146,
            PARAM_ECU_TWO_WAY_ZONE_FINE_147,
            PARAM_ECU_TWO_WAY_ZONE_FINE_148,
            PARAM_ECU_TWO_WAY_ZONE_FINE_149,
            PARAM_ECU_TWO_WAY_ZONE_FINE_150,
            PARAM_ECU_TWO_WAY_ZONE_FINE_151,
            PARAM_ECU_TWO_WAY_ZONE_FINE_152,
            PARAM_ECU_TWO_WAY_ZONE_FINE_153,
            PARAM_ECU_TWO_WAY_ZONE_FINE_154,
            PARAM_ECU_TWO_WAY_ZONE_FINE_155,
            PARAM_ECU_TWO_WAY_ZONE_FINE_156,
            PARAM_ECU_TWO_WAY_ZONE_FINE_157,
            PARAM_ECU_TWO_WAY_ZONE_FINE_158,
            PARAM_ECU_TWO_WAY_ZONE_FINE_159,
            PARAM_ECU_TWO_WAY_ZONE_FINE_160,
            PARAM_ECU_TWO_WAY_ZONE_FINE_161,
            PARAM_ECU_TWO_WAY_ZONE_FINE_162,
            PARAM_ECU_TWO_WAY_ZONE_FINE_163,
            PARAM_ECU_TWO_WAY_ZONE_FINE_164,
            PARAM_ECU_TWO_WAY_ZONE_FINE_165,
            PARAM_ECU_TWO_WAY_ZONE_FINE_166,
            PARAM_ECU_TWO_WAY_ZONE_FINE_167,
            PARAM_ECU_TWO_WAY_ZONE_FINE_168,
            PARAM_ECU_TWO_WAY_ZONE_FINE_169,
            PARAM_ECU_TWO_WAY_ZONE_FINE_170,
            PARAM_ECU_TWO_WAY_ZONE_FINE_171,
            PARAM_ECU_TWO_WAY_ZONE_FINE_172,
            PARAM_ECU_TWO_WAY_ZONE_FINE_173,
            PARAM_ECU_TWO_WAY_ZONE_FINE_174,
            PARAM_ECU_TWO_WAY_ZONE_FINE_175,
            PARAM_ECU_TWO_WAY_ZONE_FINE_176,
            PARAM_ECU_TWO_WAY_ZONE_FINE_177,
            PARAM_ECU_TWO_WAY_ZONE_FINE_178,
            PARAM_ECU_TWO_WAY_ZONE_FINE_179,
            PARAM_ECU_TWO_WAY_ZONE_FINE_180,
            PARAM_ECU_TWO_WAY_ZONE_FINE_181,
            PARAM_ECU_TWO_WAY_ZONE_FINE_182,
            PARAM_ECU_TWO_WAY_ZONE_FINE_183,
            PARAM_ECU_TWO_WAY_ZONE_FINE_184,
            PARAM_ECU_TWO_WAY_ZONE_FINE_185,
            PARAM_ECU_TWO_WAY_ZONE_FINE_186,
            PARAM_ECU_TWO_WAY_ZONE_FINE_187,
            PARAM_ECU_TWO_WAY_ZONE_FINE_188,
            PARAM_ECU_TWO_WAY_ZONE_FINE_189,
            PARAM_ECU_TWO_WAY_ZONE_FINE_190,
            PARAM_ECU_TWO_WAY_ZONE_FINE_191,
            PARAM_ECU_TWO_WAY_ZONE_FINE_192,
            PARAM_ECU_TWO_WAY_ZONE_FINE_193,
            PARAM_ECU_TWO_WAY_ZONE_FINE_194,
            PARAM_ECU_TWO_WAY_ZONE_FINE_195,
            PARAM_ECU_TWO_WAY_ZONE_FINE_196,
            PARAM_ECU_TWO_WAY_ZONE_FINE_197,
            PARAM_ECU_TWO_WAY_ZONE_FINE_198,
            PARAM_ECU_TWO_WAY_ZONE_FINE_199,
            PARAM_ECU_TWO_WAY_ZONE_FINE_200,
            PARAM_ECU_TWO_WAY_ZONE_FINE_201,
            PARAM_ECU_TWO_WAY_ZONE_FINE_202,
            PARAM_ECU_TWO_WAY_ZONE_FINE_203,
            PARAM_ECU_TWO_WAY_ZONE_FINE_204,
            PARAM_ECU_TWO_WAY_ZONE_FINE_205,
            PARAM_ECU_TWO_WAY_ZONE_FINE_206,
            PARAM_ECU_TWO_WAY_ZONE_FINE_207,
            PARAM_ECU_TWO_WAY_ZONE_FINE_208,
            PARAM_ECU_TWO_WAY_ZONE_FINE_209,
            PARAM_ECU_TWO_WAY_ZONE_FINE_210,
            PARAM_ECU_TWO_WAY_ZONE_FINE_211,
            PARAM_ECU_TWO_WAY_ZONE_FINE_212,
            PARAM_ECU_TWO_WAY_ZONE_FINE_213,
            PARAM_ECU_TWO_WAY_ZONE_FINE_214,
            PARAM_ECU_TWO_WAY_ZONE_FINE_215,
            PARAM_ECU_TWO_WAY_ZONE_FINE_216,
            PARAM_ECU_TWO_WAY_ZONE_FINE_217,
            PARAM_ECU_TWO_WAY_ZONE_FINE_218,
            PARAM_ECU_TWO_WAY_ZONE_FINE_219,
            PARAM_ECU_TWO_WAY_ZONE_FINE_220,
            PARAM_ECU_TWO_WAY_ZONE_FINE_221,
            PARAM_ECU_TWO_WAY_ZONE_FINE_222,
            PARAM_ECU_TWO_WAY_ZONE_FINE_223,
            PARAM_ECU_TWO_WAY_ZONE_FINE_224,
            PARAM_ECU_TWO_WAY_ZONE_FINE_225,
            PARAM_ECU_TWO_WAY_ZONE_FINE_226,
            PARAM_ECU_TWO_WAY_ZONE_FINE_227,
            PARAM_ECU_TWO_WAY_ZONE_FINE_228,
            PARAM_ECU_TWO_WAY_ZONE_FINE_229,
            PARAM_ECU_TWO_WAY_ZONE_FINE_230,
            PARAM_ECU_TWO_WAY_ZONE_FINE_231,
            PARAM_ECU_TWO_WAY_ZONE_FINE_232,
            PARAM_ECU_TWO_WAY_ZONE_FINE_233,
            PARAM_ECU_TWO_WAY_ZONE_FINE_234,
            PARAM_ECU_TWO_WAY_ZONE_FINE_235,
            PARAM_ECU_TWO_WAY_ZONE_FINE_236,
            PARAM_ECU_TWO_WAY_ZONE_FINE_237,
            PARAM_ECU_TWO_WAY_ZONE_FINE_238,
            PARAM_ECU_TWO_WAY_ZONE_FINE_239,
            PARAM_ECU_TWO_WAY_ZONE_FINE_240,
            PARAM_ECU_TWO_WAY_ZONE_FINE_241,
            PARAM_ECU_TWO_WAY_ZONE_FINE_242,
            PARAM_ECU_TWO_WAY_ZONE_FINE_243,
            PARAM_ECU_TWO_WAY_ZONE_FINE_244,
            PARAM_ECU_TWO_WAY_ZONE_FINE_245,
            PARAM_ECU_TWO_WAY_ZONE_FINE_246,
            PARAM_ECU_TWO_WAY_ZONE_FINE_247,
            PARAM_ECU_TWO_WAY_ZONE_FINE_248,
            PARAM_ECU_TWO_WAY_ZONE_FINE_249,
            PARAM_ECU_TWO_WAY_ZONE_FINE_250,
            PARAM_ECU_TWO_WAY_ZONE_FINE_251,

            PARAM_LAMP_INTENSITY = 1830,
            PARAM_LAMP_SHUTTERSTATE,
            PARAM_LAMP_CONTROLMODE,
            PARAM_LAMP_MODE,
            PARAM_LAMP_LAMPTEMPERATURE,
            PARAM_LAMP_LLGTEMPERATURE,
            PARAM_LAMP_LIFETIME,
            PARAM_LAMP_WARNINGCODE,
            PARAM_LAMP_ERRORCODE,
            PARAM_LAMP_LIGHTENGINE = 1839,
            PARAM_LAMP_TERMINAL = 1840,
            PARAM_LAMP1_CONNECTION,
            PARAM_LAMP2_CONNECTION,

            PARAM_LIGHTPATH_GG = 1850,
            PARAM_LIGHTPATH_GR,
            PARAM_LIGHTPATH_CAMERA,
            PARAM_LIGHTPATH_GG_SERIALNUMBER,
            PARAM_LIGHTPATH_GR_SERIALNUMBER,
            PARAM_LIGHTPATH_CAMERA_SERIALNUMBER,
            PARAM_SCOPE_TYPE,
            PARAM_LIGHTPATH_INVERTED_POS,
            PARAM_LIGHTPATH_NDD,
            PARAM_LIGHTPATH_NDD_AVAILABLE,

            PARAM_KURIOS_WAVELENGTH = 1860,
            PARAM_KURIOS_BANDWIDTHMODE,
            PARAM_KURIOS_TEMPERATURE,
            PARAM_KURIOS_TEMPERATURESTATUS,
            PARAM_KURIOS_CONTROLMODE,
            PARAM_KURIOS_GETSEQUENCE,
            PARAM_KURIOS_SETSEQUENCE,
            PARAM_KURIOS_INSERTSEQUENCE,
            PARAM_KURIOS_DELETESEQUENCE,
            PARAM_KURIOS_FASTSWITCHINGDATA,
            PARAM_KURIOS_TRIGGEROUTSIGNALMODE,
            PARAM_KURIOS_FORCETRIGGER,
            PARAM_KURIOS_TRIGGEROUTTIMEMODE,
            PARAM_KURIOS_SWITCHDELAY,

            PARAM_BEAM_STABILIZER_FIRSTPARAM = 1930,
            PARAM_BEAM_STABILIZER_REALIGNBEAM = 1930,
            PARAM_BEAM_STABILIZER_BPA_SN,
            PARAM_BEAM_STABILIZER_BPB_SN,
            PARAM_BEAM_STABILIZER_BPA_CENTER_X,
            PARAM_BEAM_STABILIZER_BPA_CENTER_Y,
            PARAM_BEAM_STABILIZER_BPA_EXPOSURE,
            PARAM_BEAM_STABILIZER_BPB_CENTER_X,
            PARAM_BEAM_STABILIZER_BPB_CENTER_Y,
            PARAM_BEAM_STABILIZER_BPB_EXPOSURE,
            PARAM_BEAM_STABILIZER_PIEZO1_POS,
            PARAM_BEAM_STABILIZER_PIEZO2_POS,
            PARAM_BEAM_STABILIZER_PIEZO3_POS,
            PARAM_BEAM_STABILIZER_PIEZO4_POS,
            PARAM_BEAM_STABILIZER_FACTORY_RESET_PIEZOS,
            PARAM_BEAM_STABILIZER_LASTPARAM = 1949,

            PARAM_FW_DIC_FIRMWAREUPDATE,
            PARAM_FW_DIC_FIRMWAREVERSION,
            PARAM_FW_DIC_SERIALNUMBER,

            PARAM_DEVICE_STATUS,

            PARAM_SHUTTER_1_STATE = 1954,
            PARAM_SHUTTER_2_STATE,
            PARAM_SHUTTER_3_STATE,
            PARAM_SHUTTER_4_STATE,
            PARAM_SHUTTER_AVAILABLE,
            PARAM_SHUTTER_SAFETY_INTERLOCK_STATE = 1959,

            PARAM_LED1_SOCKEL_ID = 2000,
            PARAM_LED2_SOCKEL_ID,
            PARAM_LED3_SOCKEL_ID,
            PARAM_LED4_SOCKEL_ID,
            PARAM_LED5_SOCKEL_ID,
            PARAM_LED6_SOCKEL_ID,

            PARAM_LED1_CONTROL_NAME,
            PARAM_LED2_CONTROL_NAME,
            PARAM_LED3_CONTROL_NAME,
            PARAM_LED4_CONTROL_NAME,
            PARAM_LED5_CONTROL_NAME,
            PARAM_LED6_CONTROL_NAME,

            PARAM_LED1_CONTROL_CUSTOM_NAME,
            PARAM_LED2_CONTROL_CUSTOM_NAME,
            PARAM_LED3_CONTROL_CUSTOM_NAME,
            PARAM_LED4_CONTROL_CUSTOM_NAME,
            PARAM_LED5_CONTROL_CUSTOM_NAME,
            PARAM_LED6_CONTROL_CUSTOM_NAME,

            PARAM_LED1_SN,
            PARAM_LED2_SN,
            PARAM_LED3_SN,
            PARAM_LED4_SN,
            PARAM_LED5_SN,
            PARAM_LED6_SN,

            PARAM_LED1_HEADS_COLOR_NAME,
            PARAM_LED2_HEADS_COLOR_NAME,
            PARAM_LED3_HEADS_COLOR_NAME,
            PARAM_LED4_HEADS_COLOR_NAME,
            PARAM_LED5_HEADS_COLOR_NAME,
            PARAM_LED6_HEADS_COLOR_NAME,

            PARAM_LED1_LIGHT_COLOR,
            PARAM_LED2_LIGHT_COLOR,
            PARAM_LED3_LIGHT_COLOR,
            PARAM_LED4_LIGHT_COLOR,
            PARAM_LED5_LIGHT_COLOR,
            PARAM_LED6_LIGHT_COLOR,

            PARAM_LED1_SPECTRUM_DATA,
            PARAM_LED2_SPECTRUM_DATA,
            PARAM_LED3_SPECTRUM_DATA,
            PARAM_LED4_SPECTRUM_DATA,
            PARAM_LED5_SPECTRUM_DATA,
            PARAM_LED6_SPECTRUM_DATA,

            PARAM_LED1_POWER_STATE,
            PARAM_LED2_POWER_STATE,
            PARAM_LED3_POWER_STATE,
            PARAM_LED4_POWER_STATE,
            PARAM_LED5_POWER_STATE,
            PARAM_LED6_POWER_STATE,

            PARAM_LED1_POWER,
            PARAM_LED2_POWER,
            PARAM_LED3_POWER,
            PARAM_LED4_POWER,
            PARAM_LED5_POWER,
            PARAM_LED6_POWER,

            PARAM_LEDS_ENABLE_DISABLE,
            PARAM_LEDS_LINEAR_VALUE,

            PARAM_LED1_CURRENT,
            PARAM_LED2_CURRENT,
            PARAM_LED3_CURRENT,
            PARAM_LED4_CURRENT,
            PARAM_LED5_CURRENT,
            PARAM_LED6_CURRENT,

            PARAM_LED1_TEMP,
            PARAM_LED2_TEMP,
            PARAM_LED3_TEMP,
            PARAM_LED4_TEMP,
            PARAM_LED5_TEMP,
            PARAM_LED6_TEMP,

            PARAM_LED1_PEAK_WAVELENGTH,
            PARAM_LED2_PEAK_WAVELENGTH,
            PARAM_LED3_PEAK_WAVELENGTH,
            PARAM_LED4_PEAK_WAVELENGTH,
            PARAM_LED5_PEAK_WAVELENGTH,
            PARAM_LED6_PEAK_WAVELENGTH,

            PARAM_LED1_NOMINAL_WAVELENGTH,
            PARAM_LED2_NOMINAL_WAVELENGTH,
            PARAM_LED3_NOMINAL_WAVELENGTH,
            PARAM_LED4_NOMINAL_WAVELENGTH,
            PARAM_LED5_NOMINAL_WAVELENGTH,
            PARAM_LED6_NOMINAL_WAVELENGTH,

            PARAM_UPDATE_TEMPERATURES,

            PARAM_TTL_LED1_IS_ACTIVE_HIGH,
            PARAM_TTL_LED2_IS_ACTIVE_HIGH,
            PARAM_TTL_LED3_IS_ACTIVE_HIGH,
            PARAM_TTL_LED4_IS_ACTIVE_HIGH,
            PARAM_TTL_LED5_IS_ACTIVE_HIGH,
            PARAM_TTL_LED6_IS_ACTIVE_HIGH,
            PARAM_TTL_ADD1_IS_ACTIVE_HIGH,
            PARAM_TTL_ADD2_IS_ACTIVE_HIGH,
            PARAM_TTL_ADD3_IS_ACTIVE_HIGH,
            PARAM_TTL_ADD4_IS_ACTIVE_HIGH,

            PARAM_TTL_LED1_DELAY_TIME,
            PARAM_TTL_LED2_DELAY_TIME,
            PARAM_TTL_LED3_DELAY_TIME,
            PARAM_TTL_LED4_DELAY_TIME,
            PARAM_TTL_LED5_DELAY_TIME,
            PARAM_TTL_LED6_DELAY_TIME,
            PARAM_TTL_ADD1_DELAY_TIME,
            PARAM_TTL_ADD2_DELAY_TIME,
            PARAM_TTL_ADD3_DELAY_TIME,
            PARAM_TTL_ADD4_DELAY_TIME,

            PARAM_TTL_LED1_DUTY_TIME,
            PARAM_TTL_LED2_DUTY_TIME,
            PARAM_TTL_LED3_DUTY_TIME,
            PARAM_TTL_LED4_DUTY_TIME,
            PARAM_TTL_LED5_DUTY_TIME,
            PARAM_TTL_LED6_DUTY_TIME,
            PARAM_TTL_ADD1_DUTY_TIME,
            PARAM_TTL_ADD2_DUTY_TIME,
            PARAM_TTL_ADD3_DUTY_TIME,
            PARAM_TTL_ADD4_DUTY_TIME,

            PARAM_TTL_LED1_IDLE_TIME,
            PARAM_TTL_LED2_IDLE_TIME,
            PARAM_TTL_LED3_IDLE_TIME,
            PARAM_TTL_LED4_IDLE_TIME,
            PARAM_TTL_LED5_IDLE_TIME,
            PARAM_TTL_LED6_IDLE_TIME,
            PARAM_TTL_ADD1_IDLE_TIME,
            PARAM_TTL_ADD2_IDLE_TIME,
            PARAM_TTL_ADD3_IDLE_TIME,
            PARAM_TTL_ADD4_IDLE_TIME,

            PARAM_TTL_LED1_CYCLE_COUNT,
            PARAM_TTL_LED2_CYCLE_COUNT,
            PARAM_TTL_LED3_CYCLE_COUNT,
            PARAM_TTL_LED4_CYCLE_COUNT,
            PARAM_TTL_LED5_CYCLE_COUNT,
            PARAM_TTL_LED6_CYCLE_COUNT,
            PARAM_TTL_ADD1_CYCLE_COUNT,
            PARAM_TTL_ADD2_CYCLE_COUNT,
            PARAM_TTL_ADD3_CYCLE_COUNT,
            PARAM_TTL_ADD4_CYCLE_COUNT,

            PARAM_EPHYS_TRIG_BUFFER,
            PARAM_WAVEFORM_OUTPATH,

            PARAM_HPD1_GAIN_VOLTS = 7000,	///Temporary parameters for research group
            PARAM_HPD2_GAIN_VOLTS,
            PARAM_HPD3_GAIN_VOLTS,
            PARAM_HPD4_GAIN_VOLTS,
            PARAM_HPD5_GAIN_VOLTS,
            PARAM_HPD6_GAIN_VOLTS,
            PARAM_PMT1_VBR_VOLTS,
            PARAM_PMT2_VBR_VOLTS,
            PARAM_PMT3_VBR_VOLTS,
            PARAM_PMT4_VBR_VOLTS,
            PARAM_PMT5_VBR_VOLTS,
            PARAM_PMT6_VBR_VOLTS,
            PARAM_RESEARCH_DEVICE_6,
            PARAM_RESEARCH_DEVICE_7,
            PARAM_RESEARCH_DEVICE_8,
            PARAM_RESEARCH_DEVICE_9,
            PARAM_RESEARCH_DEVICE_10,
            PARAM_RESEARCH_DEVICE_11,
            PARAM_RESEARCH_DEVICE_12,
            PARAM_RESEARCH_DEVICE_13,
            PARAM_RESEARCH_DEVICE_14,
            PARAM_RESEARCH_DEVICE_15,
            PARAM_RESEARCH_DEVICE_16,
            PARAM_RESEARCH_DEVICE_17,
            PARAM_RESEARCH_DEVICE_18,
            PARAM_RESEARCH_DEVICE_19,
            PARAM_RESEARCH_DEVICE_20,
            PARAM_RESEARCH_DEVICE_21,
            PARAM_RESEARCH_DEVICE_22,
            PARAM_RESEARCH_DEVICE_23,
            PARAM_RESEARCH_DEVICE_24,
            PARAM_RESEARCH_DEVICE_25,
            PARAM_RESEARCH_DEVICE_26,
            PARAM_RESEARCH_DEVICE_27,
            PARAM_RESEARCH_DEVICE_28,
            PARAM_RESEARCH_DEVICE_29,
            PARAM_RESEARCH_DEVICE_30,
            PARAM_RESEARCH_DEVICE_31,
            PARAM_RESEARCH_DEVICE_32,
            PARAM_RESEARCH_DEVICE_33,
            PARAM_RESEARCH_DEVICE_34,
            PARAM_RESEARCH_DEVICE_35,
            PARAM_RESEARCH_DEVICE_36,
            PARAM_RESEARCH_DEVICE_37,
            PARAM_RESEARCH_DEVICE_38,
            PARAM_RESEARCH_DEVICE_39,
            PARAM_RESEARCH_DEVICE_40,
            PARAM_RESEARCH_DEVICE_41,
            PARAM_RESEARCH_DEVICE_42,
            PARAM_RESEARCH_DEVICE_43,
            PARAM_RESEARCH_DEVICE_44,
            PARAM_RESEARCH_DEVICE_45,
            PARAM_RESEARCH_DEVICE_46,
            PARAM_RESEARCH_DEVICE_47,
            PARAM_RESEARCH_DEVICE_48,
            PARAM_RESEARCH_DEVICE_49,
            PARAM_RESEARCH_DEVICE_50,
            PARAM_RESEARCH_DEVICE_51,
            PARAM_RESEARCH_DEVICE_52,
            PARAM_RESEARCH_DEVICE_53,
            PARAM_RESEARCH_DEVICE_54,
            PARAM_RESEARCH_DEVICE_55,
            PARAM_RESEARCH_DEVICE_56,
            PARAM_RESEARCH_DEVICE_57,
            PARAM_RESEARCH_DEVICE_58,
            PARAM_RESEARCH_DEVICE_59,
            PARAM_RESEARCH_DEVICE_60,
            PARAM_RESEARCH_DEVICE_61,
            PARAM_RESEARCH_DEVICE_62,
            PARAM_RESEARCH_DEVICE_63,
            PARAM_RESEARCH_DEVICE_64,
            PARAM_RESEARCH_DEVICE_65,
            PARAM_RESEARCH_DEVICE_66,
            PARAM_RESEARCH_DEVICE_67,
            PARAM_RESEARCH_DEVICE_68,
            PARAM_RESEARCH_DEVICE_69,
            PARAM_RESEARCH_DEVICE_70,
            PARAM_RESEARCH_DEVICE_71,
            PARAM_RESEARCH_DEVICE_72,
            PARAM_RESEARCH_DEVICE_73,
            PARAM_RESEARCH_DEVICE_74,
            PARAM_RESEARCH_DEVICE_75,
            PARAM_RESEARCH_DEVICE_76,
            PARAM_RESEARCH_DEVICE_77,
            PARAM_RESEARCH_DEVICE_78,
            PARAM_RESEARCH_DEVICE_79,
            PARAM_RESEARCH_DEVICE_80,
            PARAM_RESEARCH_DEVICE_81,
            PARAM_RESEARCH_DEVICE_82,
            PARAM_RESEARCH_DEVICE_83,
            PARAM_RESEARCH_DEVICE_84,
            PARAM_RESEARCH_DEVICE_85,
            PARAM_RESEARCH_DEVICE_86,
            PARAM_RESEARCH_DEVICE_87,
            PARAM_RESEARCH_DEVICE_88,
            PARAM_RESEARCH_DEVICE_89,
            PARAM_RESEARCH_DEVICE_90,
            PARAM_RESEARCH_DEVICE_91,
            PARAM_RESEARCH_DEVICE_92,
            PARAM_RESEARCH_DEVICE_93,
            PARAM_RESEARCH_DEVICE_94,
            PARAM_RESEARCH_DEVICE_95,
            PARAM_RESEARCH_DEVICE_96,
            PARAM_RESEARCH_DEVICE_97,
            PARAM_RESEARCH_DEVICE_98,
            PARAM_RESEARCH_DEVICE_99,
            PARAM_RESEARCH_DEVICE_100,

            PARAM_LAST_PARAM,
        }

        public enum SLMFunctionMode
        {
            LOAD_PHASE_ONLY = 0,
            PHASE_CALIBRATION,
            SAVE_PHASE,
            LAST_FUNCTION
        }

        #endregion Enumerations
    }

    public class IntPC : INotifyPropertyChanged
    {
        #region Fields

        private int _Value;

        #endregion Fields

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public int Value
        {
            get { return _Value; }
            set { _Value = value; OnPropertyChanged("Value"); }
        }

        #endregion Properties

        #region Methods

        void OnPropertyChanged(string propertyName)
        {
            var handler = PropertyChanged;
            if (handler != null)
            {
                handler(this, new PropertyChangedEventArgs(propertyName));
            }
        }

        #endregion Methods
    }

    public static class MemoryCopyManager
    {
        #region Methods

        public static void CopyIntPtrMemory<T>(IntPtr source, T[] destination, int startIndex, int length)
            where T : struct
        {
            var gch = GCHandle.Alloc(destination, GCHandleType.Pinned);
            try
            {
                var targetPtr = Marshal.UnsafeAddrOfPinnedArrayElement(destination, startIndex);
                var rangePartitioner = Partitioner.Create(0, length, (length >> 3) + 1);
                Parallel.ForEach
                    (rangePartitioner, new ParallelOptions { MaxDegreeOfParallelism = 4 }, range =>
                    {
                        var bytesToCopy = Marshal.SizeOf(typeof(T)) * (range.Item2 - range.Item1);

                        CopyMemory(targetPtr + Marshal.SizeOf(typeof(T)) * range.Item1, source + Marshal.SizeOf(typeof(T)) * range.Item1, (UIntPtr)bytesToCopy);
                    }
                    );
            }
            finally
            {
                gch.Free();
            }
        }

        [DllImport("kernel32.dll", SetLastError = false)]
        static extern void CopyMemory(IntPtr destination, IntPtr source, UIntPtr length);

        #endregion Methods
    }

    public class StringPC : INotifyPropertyChanged
    {
        #region Fields

        private string _Value;

        #endregion Fields

        #region Constructors

        public StringPC()
        {
        }

        public StringPC(string val)
        {
            Value = val;
        }

        #endregion Constructors

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public string Value
        {
            get { return _Value; }
            set { _Value = value; OnPropertyChanged("Value"); }
        }

        #endregion Properties

        #region Methods

        void OnPropertyChanged(string propertyName)
        {
            var handler = PropertyChanged;
            if (handler != null)
            {
                handler(this, new PropertyChangedEventArgs(propertyName));
            }
        }

        #endregion Methods
    }

    public static class XmlManager
    {
        #region Methods

        /// <summary>
        /// Create a xml node
        /// </summary>
        /// <param name="doc"></param>
        /// <param name="nodeName"></param>
        public static void CreateXmlNode(XmlDocument doc, string nodeName)
        {
            XmlNode node = doc.CreateNode(XmlNodeType.Element, nodeName, null);
            doc.DocumentElement.AppendChild(node);
        }

        public static void CreateXmlNodeWithinNode(XmlDocument doc, XmlNode parentNode, string nodeName)
        {
            XmlNode node = doc.CreateNode(XmlNodeType.Element, nodeName, null);
            parentNode.AppendChild(node);
        }

        /// <summary>
        /// Get one xml attribute value
        /// </summary>
        /// <param name="node"></param>
        /// <param name="doc"></param>
        /// <param name="attrName"></param>
        /// <param name="attrValue"></param>
        /// <returns></returns>
        public static bool GetAttribute(XmlNode node, XmlDocument doc, string attrName, ref string attrValue)
        {
            bool ret;
            if ((null == node) || (null == doc))
                return false;

            if (null == node.Attributes.GetNamedItem(attrName))
            {
                ret = false;
            }
            else
            {
                attrValue = node.Attributes[attrName].Value;
                ret = true;
            }

            return ret;
        }

        public static bool ReadAttribute<T>(out T value, XmlDocument doc, string xPathToNode, string attribute, T valueOnError = default(T), int index = 0)
        {
            XmlNodeList ndList = doc.SelectNodes(xPathToNode);
            if (ndList.Count > index)
            {
                string attributeValue = "1";
                if (XmlManager.GetAttribute(ndList[index], doc, attribute, ref attributeValue))
                {
                    int iVal = 0;
                    bool bVal = false;
                    double dVal = 0;
                    byte byVal = 0;
                    UInt16 uiVal16 = 0;
                    UInt32 uiVal = 0;
                    switch (Type.GetTypeCode(typeof(T)))
                    {
                        case TypeCode.Boolean:
                            if (Boolean.TryParse(attributeValue, out bVal))
                            {
                                value = (T)Convert.ChangeType(bVal, typeof(T));
                                return true;
                            }
                            else
                            {
                                if (attributeValue == "1")
                                {
                                    value = (T)Convert.ChangeType(true, typeof(T));
                                    return true;
                                }
                                else if (attributeValue == "0")
                                {
                                    value = (T)Convert.ChangeType(false, typeof(T));
                                    return true;
                                }
                            }
                            break;
                        case TypeCode.Byte:
                            if (Byte.TryParse(attributeValue, out byVal))
                            {
                                value = (T)Convert.ChangeType(byVal, typeof(T));
                                return true;
                            }
                            break;
                        case TypeCode.Double:
                            if (Double.TryParse(attributeValue, out dVal))
                            {
                                value = (T)Convert.ChangeType(dVal, typeof(T));
                                return true;
                            }
                            break;
                        case TypeCode.Int32:
                            if (Int32.TryParse(attributeValue, out iVal))
                            {
                                value = (T)Convert.ChangeType(iVal, typeof(T));
                                return true;
                            }
                            break;
                        case TypeCode.UInt32:
                            if (UInt32.TryParse(attributeValue, out uiVal))
                            {
                                value = (T)Convert.ChangeType(uiVal, typeof(T));
                                return true;
                            }
                            break;
                        case TypeCode.UInt16:
                            if (UInt16.TryParse(attributeValue, out uiVal16))
                            {
                                value = (T)Convert.ChangeType(uiVal16, typeof(T));
                                return true;
                            }
                            break;
                        case TypeCode.String:
                            value = (T)Convert.ChangeType(attributeValue, typeof(T));
                            return true;
                        default:
                            break;
                    }
                }
            }
            value = valueOnError;
            return false;
        }

        /// <summary>
        /// Reads an parsed value from the xml document path.
        /// </summary>
        /// <param name="doc"> XmlDocument to read from </param>
        /// <param name="xPathToNode"> Path to node </param>
        /// <param name="attribute"> Attribute tag in node </param>
        /// <param name="valueOnError"> Optional parameter specifying the value to use if there
        /// <param name="index"> Optional value specifying the index if multiple instances
        /// is an error in retrieving and parsing the value from the xml doc</param>
        /// <returns> The value parsed from the xml document, or the default value if a value could not be retrieved </returns>
        public static T ReadAttribute<T>(XmlDocument doc, string xPathToNode, string attribute, T valueOnError = default(T), int index = 0)
        {
            T inValue = default(T);
            ReadAttribute<T>(out inValue, doc, xPathToNode, attribute, valueOnError, index);
            return inValue;
        }

        public static void RemoveNodeByName(string xmlFileName, string nodeName)
        {
            XDocument pDoc = XDocument.Load(xmlFileName);
            if (null != pDoc)
            {
                pDoc.Root.Elements().Where(e => e.Name == nodeName).Remove();
                pDoc.Save(xmlFileName);
            }
        }

        /// <summary>
        /// Set one xml attribute value
        /// </summary>
        /// <param name="node">The node.</param>
        /// <param name="doc">The document.</param>
        /// <param name="attrName">Name of the attribute.</param>
        /// <param name="attValue">The att value.</param>
        public static void SetAttribute(XmlNode node, XmlDocument doc, string attrName, string attValue)
        {
            if ((null == node) || (null == doc))
                return;

            XmlNode tempNode = node.Attributes[attrName];

            if (null == tempNode)
            {
                XmlAttribute attr = doc.CreateAttribute(attrName);

                attr.Value = attValue;

                node.Attributes.Append(attr);
            }
            else
            {
                node.Attributes[attrName].Value = attValue;
            }
        }

        public static XDocument ToXDocument(this XmlDocument xmlDocument)
        {
            using (var nodeReader = new XmlNodeReader(xmlDocument))
            {
                nodeReader.MoveToContent();
                return XDocument.Load(nodeReader);
            }
        }

        public static XmlDocument ToXmlDocument(this XDocument xDocument)
        {
            var xmlDocument = new XmlDocument();
            using (var xmlReader = xDocument.CreateReader())
            {
                xmlDocument.Load(xmlReader);
            }
            return xmlDocument;
        }

        #endregion Methods
    }
}