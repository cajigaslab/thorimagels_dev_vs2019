#pragma once
#include "stdafx.h"

#define ThorErrChk(fnCall) if (200 > (error=(fnCall)) > 100) throw "fnCall";
#define ThorFnFailed(error)             ( 200 > (error) > 100 )

#ifdef __cplusplus
extern "C"
{
#endif

	/// <summary>
	/// Class PIPiezoXYZ.
	/// </summary>
	class  PIPiezoXYZ : IDevice
	{
	private:
		PIPiezoXYZ();
	public:

		/// <summary>
		/// Enum Voltage Range
		/// </summary>
		enum
		{
			/// <summary>
			/// The voltage  minimum
			/// </summary>
			VOLTAGE_MIN = 0,
			/// <summary>
			/// The voltage  maximum
			/// </summary>
			VOLTAGE_MAX = 10,
			/// <summary>
			/// The Voltage default
			/// </summary>
			VOLTAGE_DEFAULT = 0,
		};

		/// <summary>
		/// Enum Analog Mode
		/// </summary>
		enum
		{
			/// <summary>
			/// The analog mode single point
			/// </summary>
			ANALOG_MODE_SINGLE_POINT,
			/// <summary>
			/// The analog mode waveform
			/// </summary>
			ANALOG_MODE_SINGLE_WAVEFORM,
		};

		/// <summary>
		/// Enum Z device type
		/// </summary>
		enum
		{
			/// <summary>
			/// The stepper
			/// </summary>
			STEPPER = 0,
			/// <summary>
			/// The piezo
			/// </summary>
			PIEZO
		};



		static PIPiezoXYZ* getInstance();
		~PIPiezoXYZ();

		long FindDevices(long &DeviceCount);
		long SelectDevice(const long Device);
		long TeardownDevice();
		long GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault);
		long SetParam(const long paramID, const double param);
		long GetParam(const long paramID, double &param);	
		long PreflightPosition();
		long SetupPosition();
		long StartPosition();
		long StatusPosition(long &status);
		long ReadPosition(DeviceType deviceType, double &pos);
		long PostflightPosition();
		long GetLastErrorMsg(wchar_t * errMsg, long size);	
		long SetParamString(const long paramID, wchar_t * str);
		long GetParamString(const long paramID, wchar_t * str, long size);
		long SetParamBuffer(const long paramID, char * buffer, long size);
		long GetParamBuffer(const long paramID, char * buffer, long size);

		long GetControlMode(DeviceType deviceType);
		void SetControlMode(DeviceType deviceType,bool _control_mode);
		void ToggleControlMode(DeviceType deviceType);

		long GetServoMode(DeviceType deviceType);
		void SetServoMode(DeviceType deviceType,bool _servo_mode);
		void ToggleServoMode(DeviceType deviceType);



	private:

		long PIPiezoXYZ::SetAO0(double voltage);
		long PIPiezoXYZ::SetAO1(double voltage);
		long PIPiezoXYZ::SetAO2(double voltage);
		int  PIPiezoXYZ::CloseNITasks();
		long PIPiezoXYZ::BuildWaveforms();

		long PIPiezoXYZ::LoadWaveformsAndArmDAQ();



	private:

		//position of piezo x
		string _devNameX;
		string _devNameY;
		string _devNameZ;
		string _analogLineX;
		string _analogLineY;
		string _analogLineZ;

		double _xPos;
		double _xPos_C;
		double _xPos_min;
		double _xPos_max;
		double _xPos_home;
		double _xPos_target;

		double _xVol;
		double _xVol_C;
		double _xVol_min;
		double _xVol_max;
		double _xVol_home;
		double _xVol_target;
		double _xOpenloop_value;

		long _xConnection_status;
		long _xServo_status;
		long _x_analog_mode;
		long _x_analog_mode_C;
		double _xStepsize;
		double _xVelocity;

		//position of piezo y
		double _yPos;
		double _yPos_C;
		double _yPos_min;
		double _yPos_max;
		double _yPos_home;
		double _yPos_target;
		double _yOpenloop_value;

		double _yVol;
		double _yVol_C;
		double _yVol_min;
		double _yVol_max;
		double _yVol_home;
		double _yVol_target;

		long _yConnection_status;
		long _yServo_status;
		long _y_analog_mode;
		long _y_analog_mode_C;
		double _yStepsize;
		double _yVelocity;

		//position of piezo z
		double _zPos;
		double _zPos_C;
		double _zPos_min;
		double _zPos_max;
		double _zPos_home;
		double _zPos_target;
		double _zOpenloop_value;

		double _zVol;
		double _zVol_C;
		double _zVol_min;
		double _zVol_max;
		double _zVol_home;
		double _zVol_target;

		long _zConnection_status;
		long _zServo_status;
		long _z_analog_mode;
		long _z_analog_mode_C;
		double _zStepsize;
		double _zVelocity;


		//other parameters
		double _volts2mm;
		double _offsetmm;

		double* _zPockelsPowerBuffer;
		long _zPockelsPowerBufferSize;
		long _outputPockelsReference;
		long _referenceWaveformEnable;
		long _piezoDataPoints;
		long _piezoFlybackPoints;
		long _totalPoints;


		wchar_t _errMsg[256];

		double _x_fast_start_pos;
		double _x_fast_start_pos_C;

		double _x_fast_stop_pos;
		double _x_fast_stop_pos_C;

		double _x_fast_volume_time;
		double _x_fast_volume_time_C;

		double _x_fast_volume_time_min;
		double _x_fast_volume_time_max;


		double _x_fast_flyback_time;
		double _x_fast_flyback_time_C;

		double _x_fast_flyback_time_min;
		double _x_fast_flyback_time_max;

		double _y_fast_start_pos;
		double _y_fast_start_pos_C;

		double _y_fast_stop_pos;
		double _y_fast_stop_pos_C;

		double _y_fast_volume_time;
		double _y_fast_volume_time_C;

		double _y_fast_volume_time_min;
		double _y_fast_volume_time_max;


		double _y_fast_flyback_time;
		double _y_fast_flyback_time_C;

		double _y_fast_flyback_time_min;
		double _y_fast_flyback_time_max;



		double _z_fast_start_pos;
		double _z_fast_start_pos_C;

		double _z_fast_stop_pos;
		double _z_fast_stop_pos_C;

		double _z_fast_volume_time;
		double _z_fast_volume_time_C;

		double _z_fast_volume_time_min;
		double _z_fast_volume_time_max;


		double _z_fast_flyback_time;
		double _z_fast_flyback_time_C;

		double _z_fast_flyback_time_min;
		double _z_fast_flyback_time_max;

		TaskHandle _taskHandleAO0;
		TaskHandle _taskHandleCO0;

		TaskHandle _taskHandleAO1;
		TaskHandle _taskHandleCO1;



		BOOL _deviceDetected;
		long _numDevices;

		static bool _instanceFlag;

		static bool _is_xonline;		//False for analog X input
		static bool _is_yonline;		//False for analog Y input
		static bool _is_zonline;		//False for analog Z input

		static bool _is_xservo_on;	//True when Servo for X is on
		static bool _is_yservo_on;	//True when Servo for Y is on
		static bool _is_zservo_on;	//True when Servo for Z is on

		static auto_ptr<PIPiezoXYZ> _single;

	};

#ifdef __cplusplus
}
#endif