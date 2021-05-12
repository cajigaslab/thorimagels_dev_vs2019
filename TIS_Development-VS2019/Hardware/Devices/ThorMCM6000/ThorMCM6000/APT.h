#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <iterator> 
#include "Mcm6kParams.h"

using namespace std;

const BYTE NO_LIMIT = 0x01;
const BYTE MAKES_ON_CONTACT = 0x02;
const BYTE BREAKS_ON_CONTACT = 0x03;
const BYTE MAKES_HOMING_ONLY = 0x04;
const BYTE BREAKS_HOMING_ONLY = 0x05;
const BYTE REVERSE_LIMITS = 0x80;

const BYTE HOME_TO_INDEX = 0x01;
const BYTE HOME_TO_LIMIT = 0x02;

const BYTE HOME_CW = 0x01;
const BYTE HOME_CCW = 0x02;
const BYTE HOME_CW_FIRST = 0x03;
const BYTE HOME_CCW_FIRST = 0x04;

//        #region APT
const int MGMSG_HW_REQ_INFO = 0x0005;
const int MGMSG_HW_GET_INFO = 0x0006;
const int MGMSG_MOD_SET_CHANENABLESTATE = 0x0210;
const int MGMSG_MOD_REQ_CHANENABLESTATE = 0x0211;
const int MGMSG_MOD_GET_CHANENABLESTATE = 0x0212;
const int MGMSG_MOT_SET_POSCOUNTER = 0x0410;
const int MGMSG_MOT_REQ_POSCOUNTER = 0x0411;
const int MGMSG_MOT_GET_POSCOUNTER = 0x0412;
const int MGMSG_MOT_SET_ENCCOUNTER = 0x0409;
const int MGMSG_MOT_REQ_ENCCOUNTER = 0x040A;
const int MGMSG_MOT_GET_ENCCOUNTER = 0x040B;
const int MGMSG_MOT_SET_JOGPARAMS = 0x0416;
const int MGMSG_MOT_REQ_JOGPARAMS = 0x0417;
const int MGMSG_MOT_GET_JOGPARAMS = 0x0418;
const int MGMSG_MOT_SET_HOMEPARAMS = 0x0440;
const int MGMSG_MOT_REQ_HOMEPARAMS = 0x0441;
const int MGMSG_MOT_GET_HOMEPARAMS = 0x0442;
const int MGMSG_MOT_SET_LIMSWITCHPARAMS = 0x0423;
const int MGMSG_MOT_REQ_LIMSWITCHPARAMS = 0x0424;
const int MGMSG_MOT_GET_LIMSWITCHPARAMS = 0x0425;
const int MGMSG_MOT_SET_POWERPARAMS = 0x0426;
const int MGMSG_MOT_REQ_POWERPARAMS = 0x0427;
const int MGMSG_MOT_GET_POWERPARAMS = 0x0428;
const int MGMSG_MOT_SET_GENMOVEPARAMS = 0x043A;
const int MGMSG_MOT_REQ_GENMOVEPARAMS = 0x043B;
const int MGMSG_MOT_GET_GENMOVEPARAMS = 0x043C;
const int MGMSG_MOT_MOVE_HOME = 0x0443;
const int MGMSG_MOT_MOVE_RELATIVE = 0x0448;
const int MGMSG_MOT_MOVE_ABSOLUTE = 0x0453;
const int MGMSG_MOT_MOVE_JOG = 0x046A;
const int MGMSG_MOT_MOVE_VELOCITY = 0x0457;
const int MGMSG_MOT_MOVE_STOP = 0x0465;
const int MGMSG_MOT_REQ_STATUSUPDATE = 0x0480;
const int MGMSG_MOT_GET_STATUSUPDATE = 0x0481;
const int MGMSG_MOT_SET_DCPIDPARAMS = 0x04A0;
const int MGMSG_MOT_REQ_DCPIDPARAMS = 0x04A1;
const int MGMSG_MOT_GET_DCPIDPARAMS = 0x04A2;

const int MGMSG_MOT_SET_BUTTONPARAMS = 0x04B6;
const int MGMSG_MOT_REQ_BUTTONPARAMS = 0x04B7;
const int MGMSG_MOT_GET_BUTTONPARAMS = 0x04B8;

const int MGMSG_MOT_SET_EEPROMPARAMS = 0x04B9;
const int MGMSG_MOT_REQ_EEPROMPARAMS = 0x04BA;
const int MGMSG_MOT_GET_EEPROMPARAMS = 0x04BB;

const int MGMSG_MOT_SET_SOL_CYCLEPARAMS = 0x04C3;
const int MGMSG_MOT_REQ_SOL_CYCLEPARAMS = 0x04C4;
const int MGMSG_MOT_GET_SOL_CYCLEPARAMS = 0x04C5;

const int MGMSG_MOT_SET_SOL_STATE = 0x04CB;
const int MGMSG_MOT_REQ_SOL_STATE = 0x04CC;
const int MGMSG_MOT_GET_SOL_STATE = 0x04CD;


const int MGMSG_MOT_SET_PMDJOYSTICKPARAMS = 0x04E6;
const int MGMSG_MOT_REQ_PMDJOYSTICKPARAMS = 0x04E7;
const int MGMSG_MOT_GET_PMDJOYSTICKPARAMS = 0x04E8;

const int MGMSG_MOT_SET_PMDSTAGEAXISPARAMS = 0x04F0;
const int MGMSG_MOT_REQ_PMDSTAGEAXISPARAMS = 0x04F1;
const int MGMSG_MOT_GET_PMDSTAGEAXISPARAMS = 0x04F2;
const int MGMSG_LA_SET_MAGNIFICATION = 0x0840;
const int MGMSG_LA_REQ_MAGNIFICATION = 0x0841;
const int MGMSG_LA_GET_MAGNIFICATION = 0x0842;
const int MGMSG_HS_GET_STATUSUPDATE = 0x0483;

const int MGMSG_MOT_SET_MFF_OPERPARAMS = 0x0510;
const int MGMSG_MOT_REQ_MFF_OPERPARAMS = 0x0511;
const int MGMSG_MOT_GET_MFF_OPERPARAMS = 0x0512;

// Laser Control Messages
const int MGMSG_LA_SET_PARAMS = 0x0800;
const int MGMSG_LA_REQ_PARAMS = 0x0801;
const int MGMSG_LA_GET_PARAMS = 0x0802;
const int MGMSG_LA_ENABLEOUTPUT = 0x0811;
const int MGMSG_LA_DISABLEOUTPUT = 0x0812;

//Firmware update command
const int MGMSG_GET_UPDATE_FIRMWARE = 0x00A6;
const int MGMSG_RESET_FIRMWARE_LOADCOUNT = 0x00A7;
const int MGMSG_BL_REQ_FIRMWAREVER = 0x002F;
const int MGMSG_BL_SET_FLASHPAGE = 0x00A8;
const int MGMSG_BL_GET_FIRMWAREVER = 0x0030;

//*****************************************************************
// APT
// 0x4000 to 0x4fff range are reserved for Sterling VA
//*****************************************************************
// System        
const int MGMSG_MCM_HW_REQ_INFO = 0x4000;
const int MGMSG_MCM_HW_GET_INFO = 0x4001;
const int MGMSG_CPLD_UPDATE = 0x4002;
const int MGMSG_SET_HW_REV = 0x4003;
const int MGMSG_SET_CARD_TYPE = 0x4004;
const int MGMSG_SET_CABLE = 0x4005;
const int MGMSG_REQ_CABLE = 0x4006;
const int MGMSG_GET_CABLE = 0x4007;
const int MGMSG_SET_CABLE_BOARD = 0x4008;
const int MGMSG_REQ_CABLE_BOARD = 0x4009;
const int MGMSG_GET_CABLE_BOARD = 0x400A;
const int MGMSG_RESTART_PROCESSOR = 0x400B;
const int MGMSG_ERASE_EEPROM = 0x400C;
const int MGMSG_REQ_CPLD_WR = 0x400D;
const int MGMSG_GET_CPLD_WR = 0x400E;
const int MGMSG_TASK_CONTROL = 0x400F;
const int MGMSG_BOARD_REQ_STATUSUPDATE = 0x4010;
const int MGMSG_BOARD_GET_STATUSUPDATE = 0x4011;
const int MGMSG_MOD_REQ_JOYSTICK_INFO = 0x4012;
const int MGMSG_MOD_GET_JOYSTICK_INFO = 0x4013;
const int MGMSG_MOD_SET_JOYSTICK_MAP_IN = 0x4014;
const int MGMSG_MOD_REQ_JOYSTICK_MAP_IN = 0x4015;
const int MGMSG_MOD_GET_JOYSTICKS_MAP_IN = 0x4016;
const int MGMSG_MOD_SET_JOYSTICK_MAP_OUT = 0x4017;
const int MGMSG_MOD_REQ_JOYSTICK_MAP_OUT = 0x4018;
const int MGMSG_MOD_GET_JOYSTICKS_MAP_OUT = 0x4019;
const int MGMSG_MOD_SET_SYSTEM_DIM = 0x401A;
const int MGMSG_MOD_REQ_SYSTEM_DIM = 0x401B;
const int MGMSG_MOD_GET_SYSTEM_DIM = 0x401C;
const int MGMSG_SET_STORE_POSITION = 0x4021;
const int MGMSG_REQ_STORE_POSITION = 0x4022;
const int MGMSG_GET_STORE_POSITION = 0x4023;
const int MGMSG_SET_GOTO_STORE_POSITION = 0x4024;
const int MGMSG_MCM_START_LOG = 0x4025;
const int MGMSG_MCM_POST_LOG = 0x4026;
const int MGMSG_MCM_SET_ENABLE_LOG = 0x4027;
const int MGMSG_MCM_REQ_ENABLE_LOG = 0x4028;
const int MGMSG_MCM_GET_ENABLE_LOG = 0x4029;

// Stepper
const int MGMSG_MCM_SET_SOFT_LIMITS = 0x403D;
const int MGMSG_MCM_SET_HOMEPARAMS = 0x403E;
const int MGMSG_MCM_REQ_HOMEPARAMS = 0x403F;
const int MGMSG_MCM_GET_HOMEPARAMS = 0x4040;
const int MGMSG_MCM_SET_STAGEPARAMS = 0x4041;
const int MGMSG_MCM_REQ_STAGEPARAMS = 0x4042;
const int MGMSG_MCM_GET_STAGEPARAMS = 0x4043;
const int MGMSG_MCM_REQ_STATUSUPDATE = 0x4044;
const int MGMSG_MCM_GET_STATUSUPDATE = 0x4045;
const int MGMSG_MCM_SET_ABS_LIMITS = 0x4046;
const int MGMSG_MCM_MOT_SET_LIMSWITCHPARAMS = 0x4047;
const int MGMSG_MCM_MOT_REQ_LIMSWITCHPARAMS = 0x4048;
const int MGMSG_MCM_MOT_GET_LIMSWITCHPARAMS = 0x4049;
const int MGMSG_MCM_MOT_MOVE_BY = 0x4050;	// Added for Texas TIDE autofocus
const int MGMSG_MCM_REQ_STEPPER_LOG = 0x4051;
const int MGMSG_MCM_GET_STEPPER_LOG = 0x4052;
const int MGMSG_MCM_MOT_SET_VELOCITY = 0x4053;

// Shutter
const int MGMSG_MCM_SET_SHUTTERPARAMS = 0x4064;
const int MGMSG_MCM_REQ_SHUTTERPARAMS = 0x4065;
const int MGMSG_MCM_GET_SHUTTERPARAMS = 0x4066;
const int MGMSG_MCM_REQ_INTERLOCK_STATE = 0x4067;
const int MGMSG_MCM_GET_INTERLOCK_STATE = 0x4068;

// Slider IO
const int MGMSG_MCM_SET_MIRROR_STATE = 0x4087;
const int MGMSG_MCM_REQ_MIRROR_STATE = 0x4088;
const int MGMSG_MCM_GET_MIRROR_STATE = 0x4089;
const int MGMSG_MCM_SET_MIRROR_PARAMS = 0x408A;
const int MGMSG_MCM_REQ_MIRROR_PARAMS = 0x408B;
const int MGMSG_MCM_GET_MIRROR_PARAMS = 0x408C;
const int MGMSG_MCM_SET_MIRROR_PWM_DUTY = 0x408D;
const int MGMSG_MCM_REQ_MIRROR_PWM_DUTY = 0x408E;
const int MGMSG_MCM_GET_MIRROR_PWM_DUTY = 0x408F;

// Synchronized Motion   
const int MGMSG_MCM_SET_HEX_POSE = 0x40A0;
const int MGMSG_MCM_REQ_HEX_POSE = 0x40A1;
const int MGMSG_MCM_GET_HEX_POSE = 0x40A2;
const int MGMSG_MCM_SET_SYNC_MOTION_PARAM = 0x40A3;
const int MGMSG_MCM_REQ_SYNC_MOTION_PARAM = 0x40A4;
const int MGMSG_MCM_GET_SYNC_MOTION_PARAM = 0x40A5;
const int MGMSG_MCM_SET_SYNC_MOTION_POINT = 0x40A6;

// OTM Laser
const int MGMSG_LA_DISABLEAIMING = 0x40C7;
const int MGMSG_LA_ENABLEAIMING = 0x40C8;

// Piezo
const int MGMSG_MCM_PIEZO_SET_PRAMS = 0x40DC;
const int MGMSG_MCM_PIEZO_REQ_PRAMS = 0x40DD;
const int MGMSG_MCM_PIEZO_GET_PRAMS = 0x40DE;
const int MGMSG_MCM_PIEZO_REQ_VALUES = 0x40DF;
const int MGMSG_MCM_PIEZO_GET_VALUES = 0x40E0;
//  const int MGMSG_MCM_PIEZO_REQ_LOG = 0x40E1;
const int MGMSG_MCM_PIEZO_GET_LOG = 0x40E2;
const int MGMSG_MCM_PIEZO_SET_ENABLE_PLOT = 0x40E3;
const int MGMSG_MCM_PIEZO_SET_DAC_VOLTS = 0x40E4;
const int MGMSG_MCM_PIEZO_SET_PID_PARMS = 0x40E5;
const int MGMSG_MCM_PIEZO_REQ_PID_PARMS = 0x40E6;
const int MGMSG_MCM_PIEZO_GET_PID_PARMS = 0x40E7;
const int MGMSG_MCM_PIEZO_SET_MOVE_BY = 0x40E8;
const int MGMSG_MCM_PIEZO_SET_MODE = 0x40E9;
const int MGMSG_MCM_PIEZO_REQ_MODE = 0x40EA;
const int MGMSG_MCM_PIEZO_GET_MODE = 0x40EB;

//*****************************************************************
struct MapInfo
{
	BYTE number;
	short vid;
	short pid;
	BYTE type;
	BYTE control_number;
	BYTE mode;
	BYTE slot_receiver;
	BYTE port_receiver;
};

struct MapInfoIn
{
	BYTE number;
	//public BYTE port;
	USHORT vid;
	USHORT pid;
	BYTE control_number;
	BYTE mode;
	BYTE modify_control_port;
	USHORT modify_control_ctl_num;
	BYTE destination_slot;
	BYTE destination_bit;
	BYTE destination_port;
	BYTE destination_virtual;
	BYTE modify_speed;
	BYTE revserse_dir;
	BYTE dead_band;
};

struct MapInfoOut
{
	BYTE number;
	BYTE control_number;
	BYTE type;
	USHORT vid;
	USHORT pid;
	BYTE mode;
	BYTE color_1_id;
	BYTE color_2_id;
	BYTE color_3_id;
	BYTE source_slot;
	BYTE source_bit;
	BYTE source_port;
	BYTE source_virtual;
};

#define TEMP_SAMPLES	15

const double B = 3930.0;
const double T0 = 298.0;
const double R2 = 10000.0;
const double R0 = 10000.0;
const double VCC = 3.287;
const double MAX_VAL = 4096;    // 12 bit
const double volts_per_counts = VCC / MAX_VAL;
const double vin_resistor_div = 0.083170; // R2/(R1+r2)

class APT
{
private:
	void clear_all_map_controls_for_port(BYTE _port);
	void clear_map_control(BYTE _port, BYTE number);
	void add_map(BYTE _port, MapInfo* mi);
	vector<vector<MapInfo>> mapping_data;
	vector<vector<MapInfoIn>> mapping_in_data;
	vector<vector<MapInfoOut>> mapping_out_data;

	double slot_nm_per_count[TOTAL_CARD_SLOTS];
	double slot_counts_per_unit[TOTAL_CARD_SLOTS];
	short cw_hardlimit_save[TOTAL_CARD_SLOTS];
	short ccw_hardlimit_save[TOTAL_CARD_SLOTS];
	short limit_mode_save[TOTAL_CARD_SLOTS];

	BYTE byte_to_read;
	BYTE input_control_type[8][25]; // 8 ports, max number of controls
	BYTE output_control_type[8][25]; // 8 ports, max number of controls
	int num_of_input_controls[8];
	int num_of_output_controls[8];

	double temperatures[TEMP_SAMPLES];
	int temperatures_index;
	bool temperature_ready;
	double sum_temperatures;

	double adc1;
public:


	APT();
	~APT();
	void ParseApt(char* buf, int len, Mcm6kParams* params);
	void hardware_info(char* data, int size, Mcm6kParams* params);
	void mcm_stepper_status_update(char* data, int size, Mcm6kParams* params);
	void usb_device_info(char* data, int size, Mcm6kParams* params);
	void usb_device_mapping_in(char* data, int size, Mcm6kParams* params);
	void usb_device_mapping_out(char* data, int size, Mcm6kParams* params);
	void usb_device_mapping(char* data, int size, Mcm6kParams* params);
	void CPLD_return_read_data(char* data, int size, Mcm6kParams* params);
	void stepper_get_limits_params(char* data, int size, Mcm6kParams* params);
	void stepper_get_home_params(char* data, int size, Mcm6kParams* params);
	void stepper_get_joystick_params(char* data, int size, Mcm6kParams* params);
	void stepper_get_drive_params(char* data, int size, Mcm6kParams* params);
	void stepper_get_jog_params(char* data, int size, Mcm6kParams* params);
	void servo_get_params(char* data, int size, Mcm6kParams* params);
	void servo_status_update(char* data, int size, Mcm6kParams* params);
	void board_update(char* data, int size, Mcm6kParams* params);
	void hexapod_update_xyz(char* data, int size, Mcm6kParams* params);
	void hexapod_update_pid(char* data, int size, Mcm6kParams* params);
	void shutter_get_state(char* data, int size, Mcm6kParams* params);
	void shutter_get_params(char* data, int size, Mcm6kParams* params);
	void shutter_4_get_params(char* data, int size, Mcm6kParams* params);
	void get_laser_params(char* data, int size, Mcm6kParams* params);
	void system_tab_get_cables(char* data, int size, Mcm6kParams* params);
	void system_tab_get_cables_board(char* data, int size, Mcm6kParams* params);
	void synchonized_motion_get_state(char* data, int size, Mcm6kParams* params);
	void system_dim_value_req(char* data, int size, Mcm6kParams* params);
	void mirror_position_get(char* data, int size, Mcm6kParams* params);
	void mirrors_get_params(char* data, int size, Mcm6kParams* params);
	void piezo_get_mode(char* data, int size, Mcm6kParams* params);
	void shutter_get_interlock_state(char* data, int size, Mcm6kParams* params);
};
