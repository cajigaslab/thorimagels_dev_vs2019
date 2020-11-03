#pragma once


class SimDeviceXY : IDevice
{
private:
    SimDeviceXY();

	enum
	{
		PARAM_DECODER_INCREMENT_MIN = 2,
		PARAM_DECODER_INCREMENT_MAX = 63,
		PARAM_DECODER_BLANKING_PERIOD_MIN = 0,
		PARAM_DECODER_BLANKING_PERIOD_MAX = 65535,
		PARAM_DECODER_FRACTIONAL_MIN = 0,
		PARAM_DECODER_FRACTIONAL_MAX = 31,
		PARAM_DECODER_TRIGGER_PULSES_PER_FRAME_MIN=1,
		PARAM_DECODER_TRIGGER_PULSES_PER_FRAME_MAX=4095,
		PARAM_DECODER_DIRECTION_MIN=0,
		PARAM_DECODER_DIRECTION_MAX=1,
		PARAM_DECODER_HOME_POSITION_MIN=0,
		PARAM_DECODER_HOME_POSITION_MAX=1,
		PARAM_DECODER_FSTART_MIN=0,
		PARAM_DECODER_FSTART_MAX=524288,
		PARAM_DECODER_ENCODER_PER_FRAME_MIN=0,
		PARAM_DECODER_ENCODER_PER_FRAME_MAX=524288,
		PARAM_DECODER_RUN_MIN=0,
		PARAM_DECODER_RUN_MAX=2,
		PARAM_DECODER_FRAME_COUNT_MIN=1,
		PARAM_DECODER_FRAME_COUNT_MAX=65536,
	};

public:

	enum
	{
		X_HOME_MIN = 0,
		X_HOME_MAX = 0,
		X_HOME_DEFAULT = 0,

		Y_HOME_MIN = 0,
		Y_HOME_MAX = 0,
		Y_HOME_DEFAULT = 0,

		X_VELOCITY_MIN = 0,
		X_VELOCITY_MAX = 250,
		X_VELOCITY_DEFAULT = 70,
		
		Y_VELOCITY_MIN = 0,
		Y_VELOCITY_MAX = 250,
		Y_VELOCITY_DEFAULT = 70,

//		ACCELERATION_DEFAULT = 700
		ACCELERATION_DEFAULT = 137,  // This value is what the APT software uses.

		LOAD_MIN = 0,
		LOAD_MAX = 0,
		LOAD_DEFAULT = 0,

		AUTOFOCUS_MIN = 0,
		AUTOFOCUS_MAX = 1,
		AUTOFOCUS_DEFAULT = 0,

		SHUTTER_CLOSE=0,
		SHUTTER_OPEN=1,
		SHUTTER_DEFAULT=SHUTTER_CLOSE,

		SHUTTER_WAIT_TIME_MIN = 0,
		SHUTTER_WAIT_TIME_MAX = 1000,		
		SHUTTER_WAIT_TIME_DEFAULT = 0

	};

	static SimDeviceXY* getInstance();
    ~SimDeviceXY();

	long FindDevices(long &DeviceCount);
	long SelectDevice(const long Device);
	long TeardownDevice();
	long GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault);
	long SetParam(const long paramID, const double param);
	long GetParam(const long paramID, double &param);
	long SetParamString(const long paramID, wchar_t * str);
	long GetParamString(const long paramID, wchar_t * str, long size);
	long SetParamBuffer(const long paramID, char * buffer, long size);
	long GetParamBuffer(const long paramID, char * buffer, long size);
	long PreflightPosition();
	long SetupPosition();
	long StartPosition();
	long StatusPosition(long &status);
	long ReadPosition(DeviceType deviceType, double &pos);
	long PostflightPosition();
	long GetLastErrorMsg(wchar_t *msg, long size);

private:

	typedef enum _DECODER_RUN_TYPE {

		DECODER_RUN_STRIP_TDI,
		DECODER_RUN_SNAPSHOT_UNI_TDI,
		DECODER_RUN_SNAPSHOT_BI_TDI,

		DECODER_MAX_RUN_TYPES

	} DECODER_RUN_TYPE, *PDECODER_RUN_TYPE;

	typedef struct	_QDECSTATE {			// Global, static shadow of FPGA Quad Decoder internal registers

		char				qcr1;			// Shadow register of Quad-Decoder QCR1 register
		char				qcr2;			// Shadow register of Quad-Decoder QCR2 register
		char				qsr;			// Quad-Decoder QSR status register
		unsigned int		qrw_cnt;		// QRW Count 0..2,097,151

	} QDECSTATE, *PQDECSTATE;

	typedef struct _TDI_SETTINGS {			// Global, static shadow of FGPA TDI internal registers

		DECODER_RUN_TYPE	mode;			// TDI mode, 1= Strip, 2= Unidirectional Snapshot, 3= Bi-directional Snapshot, 0= off
		char				tdicr;			// Shadow register of Quad-Decoder QCR1 register
		char				tdisr;			// Quad-Decoder QSR status register
		unsigned int		pos_cnt;		// Position Count 0..2,097,151
		short unsigned int	incr_cnt;		// Increment Count 2..63, sets # whole encoder steps between TDI triggers
		short unsigned int	frac;			// Fraction multiplier 0..31, set fractional part of TDI trigger.
		short unsigned int	N;				// Number of TDI integrations 0..4095
		unsigned int		frame_start;	// Frame starting location of forward scan TDI column
		unsigned int		frame_back;		// Frame starting location of backward scan TDI column
		unsigned int		frame_step;		// Step size between frames, for Snapshot TDI mode
		short unsigned int	frame_count;	// Total number of frames in a column/strip

	} TDI_SETTINGS, *PTDI_SETTINGS;

	struct State
	{
		long pos;
		long pos_C;
		BOOL pos_B;
	};
	
	State _increment;
	State _blankingPeriod;
	State _fractional;
	State _triggerPulses;
	State _home;
	State _direction;
	State _fStart;
	State _encoderPerFrame;
	State _run;
	State _fcnt;

	TDI_SETTINGS _tdi_settings;
	QDECSTATE _qdec_settings;

	double _xPos;///x location
	volatile double _xPos_Current;
	volatile double _xPos_Last;
	BOOL _xPos_Pending;

	double _yPos;///y location
	volatile double _yPos_Current;
	volatile double _yPos_Last;
	BOOL _yPos_Pending;

	double _xVel, _xAcc;///x velocity
	volatile double _xVel_Current, _xAcc_Current;
	volatile double _xVel_Last;
	BOOL _xVel_Pending;

	double _yVel, _yAcc;///y velocity
	volatile double _yVel_Current, _yAcc_Current;
	volatile double _yVel_Last;
	BOOL _yVel_Pending;

	volatile bool _xMoveComplete;
	volatile bool _yMoveComplete;

	BOOL _xHome_Pending;///home x stage
	
	BOOL _yHome_Pending;///home y stage
	
	double _xMin;
	double _xMax;
	double _xDefault;

	double _yMin;
	double _yMax;
	double _yDefault;

	long _load;
	long _load_C;
	BOOL _load_B;
	
	long _afPos;
	long _afPos_C;
	BOOL _afPos_B;
	
	long _shutterPos;
	long _shutterPos_C;
	BOOL _shutterPos_B;
	
	long _shutterWaitTime;
	long _shutterWaitTime_C;
	BOOL _shutterWaitTime_B;

    static bool _instanceFlag;
    static auto_ptr<SimDeviceXY> _single;

	long WaitUntilSettled();
	long DecoderSetupPosition();
	long DecoderStartPosition();
	void DecoderCheckState(State& state);
};