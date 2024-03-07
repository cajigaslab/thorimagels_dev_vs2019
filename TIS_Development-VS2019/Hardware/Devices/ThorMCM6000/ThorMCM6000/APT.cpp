#pragma once
#include <sstream>
#include <math.h>
#include "MCM6000.h"
using namespace std;

APT::APT()
{
	temperatures_index = 0;
	temperature_ready = false;
	sum_temperatures = 0;
}

APT::~APT()
{
}

void APT::ParseApt(char* buf, int len, Mcm6kParams* params)
{
	char header[6];
	char* extData = NULL;
	int  extDataSize = 0;
	memcpy(header, buf, 6);
	long size = sizeof(buf);
	if ((header[4] & 0x80) != 0)
	{
		extDataSize = (USHORT)(header[3] << 8) + (USHORT)header[2];
		if (extDataSize > (len - 6)) return; // error
		extData = (char*)malloc(extDataSize * sizeof(char));
		memcpy(extData, (buf + 6), extDataSize);
	}
	USHORT cmd = 0;
	memcpy(&cmd, header, sizeof(USHORT));
	switch (cmd)
	{
	case MGMSG_HW_GET_INFO: //MGMSG_MCM_HW_GET_INFO: TODO: Panchy might eventually change this back to _MCM_
		hardware_info(extData, extDataSize, params);
		break;
	case MGMSG_MCM_GET_STATUSUPDATE:
		mcm_stepper_status_update(extData, extDataSize, params);
		break;

	case MGMSG_MOD_GET_JOYSTICK_INFO:
		usb_device_info(extData, extDataSize, params);
		break;

	case MGMSG_MOD_GET_JOYSTICKS_MAP_IN:
		usb_device_mapping_in(extData, extDataSize, params); //IM
		break;

	case MGMSG_MOD_GET_JOYSTICKS_MAP_OUT:
		usb_device_mapping_out(extData, extDataSize, params); //IM
		break;

	case MGMSG_GET_CPLD_WR:
		CPLD_return_read_data(extData, extDataSize, params);
		break;

	case MGMSG_MOT_GET_LIMSWITCHPARAMS:
		stepper_get_limits_params(extData, extDataSize, params);
		break;

	case MGMSG_MCM_GET_HOMEPARAMS:
		stepper_get_home_params(extData, extDataSize, params);
		break;

	case MGMSG_MCM_GET_STAGEPARAMS:
		stepper_get_drive_params(extData, extDataSize, params);
		break;

	case MGMSG_MOT_GET_JOGPARAMS:
		stepper_get_jog_params(extData, extDataSize, params);
		break;

	case MGMSG_MOT_GET_MFF_OPERPARAMS:
		servo_get_params(extData, extDataSize, params);
		break;

	case MGMSG_MOT_GET_BUTTONPARAMS:
		servo_status_update(extData, extDataSize, params);
		break;

	case MGMSG_BOARD_GET_STATUSUPDATE:
		board_update(extData, extDataSize, params);
		break;

	case MGMSG_MCM_GET_HEX_POSE:
		hexapod_update_xyz(extData, extDataSize, params);
		break;

	case MGMSG_MOT_GET_DCPIDPARAMS:
		hexapod_update_pid(extData, extDataSize, params);
		break;

	case MGMSG_MOT_GET_SOL_STATE:
		shutter_get_state(header, 6, params);
		break;

	case MGMSG_MCM_GET_SHUTTERPARAMS:
		if (params->cardType[extData[0]] == (USHORT)Shutter_4_type || params->cardType[extData[0]] == (USHORT)Shutter_4_type_REV6)
			shutter_4_get_params(extData, extDataSize, params);
		else
			shutter_get_params(extData, extDataSize, params);
		break;

	case MGMSG_LA_GET_PARAMS:
		get_laser_params(extData, extDataSize, params);
		break;

	case MGMSG_GET_CABLE:
		system_tab_get_cables(extData, extDataSize, params);
		break;

	case MGMSG_GET_CABLE_BOARD:
		system_tab_get_cables_board(extData, extDataSize, params);
		break;

	case MGMSG_MCM_GET_SYNC_MOTION_PARAM:
		synchonized_motion_get_state(extData, extDataSize, params);
		break;

	case MGMSG_MOD_GET_SYSTEM_DIM:
		system_dim_value_req(header, 6, params);
		break;

	case MGMSG_MCM_GET_MIRROR_STATE:
		mirror_position_get(header, 6, params);
		break;

	case MGMSG_MCM_GET_MIRROR_PARAMS:
		mirrors_get_params(header, 6, params);
		break;

	case MGMSG_MCM_PIEZO_GET_MODE:
		piezo_get_mode(header, 6, params);
		break;
	case MGMSG_MCM_GET_INTERLOCK_STATE:
		shutter_get_interlock_state(header, 6, params);
		break;
	}
	free(extData);
}

void APT::hardware_info(char* data, int size, Mcm6kParams* params)
{
	// Check if data is null, there have been cases where data was null. 
	// Should be better with the Sleep in Close(). Will keep it just in case
	if (NULL != data)
	{
		memcpy(params->serialNum, data, 4);
		// firmware revision              
		memcpy(params->firmwareRev, (data + 14), 3);// minor, interim, major
		// CPLD revision
		params->cpldRev = data[17];

		memcpy(params->cardType, data + 62, TOTAL_CARD_SLOTS * sizeof(USHORT));
		memcpy(&params->boardType, data + 76, sizeof(USHORT));
	}
}

void APT::mcm_stepper_status_update(char* data, int size, Mcm6kParams* params)
{
	double step, enc;

	byte _slot = data[0];

	// step counts 
	step = data[2] | data[3] << 8 | data[4] << 16 | data[5] << 24;
	double nm_per_step = (slot_nm_per_count[_slot] / (slot_counts_per_unit[_slot]) * 100000);
	step = step * nm_per_step / 1e3;

	// encoder counts 
	enc = static_cast<unsigned char>(data[6]) | static_cast<unsigned char>(data[7]) << 8 | static_cast<unsigned char>(data[8]) << 16 | static_cast<unsigned char>(data[9]) << 24;
	enc = enc * slot_nm_per_count[_slot] / 1e6;		//Convert to um

	byte chan_enable = data[13];

	if (params->inverted_lp_slot_id == (CARD_ID_START_ADDRESS + _slot))
	{
		// homing
		params->lightPath_homing = ((data[11] & (1 << 1)) > 0);
		// ccw moving
		params->lightPath_ccw_moving = (data[10] & 0x20) > 0;
		// cw moving
		params->lightPath_cw_moving = (data[10] & 0x10) > 0;
		params->invertedLightPathPos = data[14];
	}
	if (params->et_slot_id == (CARD_ID_START_ADDRESS + _slot))
	{
		// homing
		params->epiTurret_homing = ((data[11] & (1 << 1)) > 0);
		// ccw moving
		params->epiTurret_ccw_moving = (data[10] & 0x20) > 0;
		// cw moving
		params->epiTurret_cw_moving = (data[10] & 0x10) > 0;
		params->epiTurretCurrentPos = data[14];
	}
	if (params->ndd_slot_id == (CARD_ID_START_ADDRESS + _slot))
	{
		// ccw moving
		params->ndd_ccw_moving = (data[10] & 0x20) > 0;
		// cw moving
		params->ndd_cw_moving = (data[10] & 0x10) > 0;
		params->nddCurrentPos = data[14];
	}
	if (params->x_slot_id == (CARD_ID_START_ADDRESS + _slot))
	{
		// ccw moving
		params->x_ccw_moving = (data[10] & 0x20) > 0;
		// cw moving
		params->x_cw_moving = (data[10] & 0x10) > 0;
		params->xPositionCurrent = enc;
	}
	if (params->y_slot_id == (CARD_ID_START_ADDRESS + _slot))
	{
		// ccw moving
		params->y_ccw_moving = (data[10] & 0x20) > 0;
		// cw moving
		params->y_cw_moving = (data[10] & 0x10) > 0;
		params->yPositionCurrent = enc;
	}
	if (params->z_slot_id == (CARD_ID_START_ADDRESS + _slot))
	{
		// ccw moving
		params->z_ccw_moving = (data[10] & 0x20) > 0;
		// cw moving
		params->z_cw_moving = (data[10] & 0x10) > 0;
		params->zPositionCurrent = enc;
	}
	if (params->r_slot_id == (CARD_ID_START_ADDRESS + _slot))
	{
		// ccw moving
		params->r_ccw_moving = (data[10] & 0x20) > 0;
		// cw moving
		params->r_cw_moving = (data[10] & 0x10) > 0;
		params->rPositionCurrent = enc;
	}
	if (params->ze_slot_id == (CARD_ID_START_ADDRESS + _slot))
	{
		// ccw moving
		params->ze_ccw_moving = (data[10] & 0x20) > 0;
		// cw moving
		params->ze_cw_moving = (data[10] & 0x10) > 0;
		params->zePositionCurrent = enc;
	}
	if (params->condenser_slot_id == (CARD_ID_START_ADDRESS + _slot))
	{
		// ccw moving
		params->condenser_ccw_moving = (data[10] & 0x20) > 0;
		// cw moving
		params->condenser_cw_moving = (data[10] & 0x10) > 0;
		params->condenserPositionCurrent = enc;
	}
	if (params->aux_slot_id == (CARD_ID_START_ADDRESS + _slot))
	{
		// ccw moving
		params->aux_ccw_moving = (data[10] & 0x20) > 0;
		// cw moving
		params->aux_cw_moving = (data[10] & 0x10) > 0;
		params->auxPositionCurrent = enc;
	}
}

void APT::usb_device_info(char* data, int size, Mcm6kParams* params)
{
	int VID, PID;
	string speed_txt;
	string device_type;
	string port_or_control_string;
	byte port = 0;

	// add a mapping list for each port
	if (mapping_data.size() == 0)
	{
		for (int i = 0; i < 8; i++)
		{
			(mapping_data).push_back(*(new vector<MapInfo>()));
		}
	}

	byte_to_read = 0;

	/* Port*/
	port = (byte)(data[byte_to_read++] - 1);

	/* VID */
	VID = data[byte_to_read] | data[byte_to_read + 1] << 8;
	byte_to_read += 2;

	/* PID */
	PID = data[byte_to_read] | data[byte_to_read + 1] << 8;
	byte_to_read += 2;

	/* speed and hub */
	int speed = (data[byte_to_read] >> 1) & 0x01;

	if (speed == 1)
	{
		speed_txt = " | Lowspeed";
	}
	else
	{
		speed_txt = " | Highspeed";
	}

	int is_hub = data[byte_to_read++] & 0x01;

	if (is_hub == 1)
	{
		device_type = "(HUB) | ";
		port_or_control_string = "Ports";
	}
	else
	{
		device_type = "(Device) | ";
		port_or_control_string = "Controls";
	}

	num_of_input_controls[port] = data[byte_to_read++];
	num_of_output_controls[port] = data[byte_to_read++];
}

void APT::usb_device_mapping_in(char* data, int size, Mcm6kParams* params)
{
	if (data[14] == 0xff) return;
	/* Port*/
	byte _port = (byte)(data[0]);
	byte _control_num = (byte)(data[1]);

	MapInfoIn* mi = new MapInfoIn();
	mi->number = (byte)mapping_in_data[_port].size();

	mi->control_number = _control_num;
	mi->vid = (USHORT)(data[2] + (data[3] << 8));
	mi->pid = (USHORT)(data[4] + (data[5] << 8));
	mi->modify_control_port = data[6];
	mi->modify_control_ctl_num = (USHORT)(data[7] + (data[8] << 8));
	mi->destination_slot = data[9];
	mi->destination_bit = data[10];
	mi->destination_port = data[11];
	mi->destination_virtual = data[12];
	mi->modify_speed = data[13];
	mi->revserse_dir = data[14];
	mi->dead_band = data[15];
	mi->mode = data[16];

	byte i = (byte)mapping_in_data[_port].size();

	MapInfoIn* tM = new	MapInfoIn(*mi);
	tM->number = i;
	// add the mapping info to the list
	mapping_in_data[_port].push_back(*tM);

	(mapping_in_data[_port][mi->number].modify_control_port);
	(mapping_in_data[_port][mi->number].modify_control_ctl_num);
	(mapping_in_data[_port][mi->number].modify_speed);
	(mapping_in_data[_port][mi->number].destination_slot);
	(mapping_in_data[_port][mi->number].destination_bit);
	(mapping_in_data[_port][mi->number].destination_port);
	(mapping_in_data[_port][mi->number].destination_virtual);
	(mapping_in_data[_port][mi->number].dead_band);

	string rev_dir;

	if (mapping_in_data[_port][mi->number].revserse_dir == 1)
		rev_dir = "(R) ";
	else
		rev_dir = "";

	(mapping_in_data[_port][mi->number].mode);
}

void APT::usb_device_mapping_out(char* data, int size, Mcm6kParams* params)
{
	if (data[2] == 0xff) return;
	/* Port*/
	byte _port = (byte)(data[0]);
	byte _control_num = (byte)(data[1]);

	MapInfoOut* mi = new MapInfoOut();
	mi->number = (byte)mapping_out_data[_port].size();

	mi->control_number = _control_num;
	mi->type = data[2];
	mi->vid = (USHORT)(data[3] + (data[4] << 8));
	mi->pid = (USHORT)(data[5] + (data[6] << 8));
	mi->mode = data[7];
	mi->color_1_id = data[8];
	mi->color_2_id = data[9];
	mi->color_3_id = data[10];
	mi->source_slot = data[11];
	mi->source_bit = data[12];
	mi->source_port = data[13];
	mi->source_virtual = data[14];

	byte i = (byte)mapping_out_data[_port].size();
	MapInfoOut* tM = new MapInfoOut(*mi);
	tM->number = i;
	// add the mapping info to the list
	mapping_out_data[_port].push_back(*tM);

}

void APT::usb_device_mapping(char* data, int size, Mcm6kParams* params)
{
	byte_to_read = 0;

	if (size < 3) return;

	/* Port*/
	byte _port = (byte)(data[byte_to_read++]);

	/* ID not use only used in uC for eeprom*/
	//    byte id = (byte)(ext_data[byte_to_read++]);

	byte length = (byte)(data[byte_to_read++]);
	if (length < 9) return;

	clear_all_map_controls_for_port(_port);

	int num_of_controls = (length - 2) / 9;    // each control has 9 bytes

	MapInfo* mi = new MapInfo();

	for (int i = 0; i < num_of_controls; i++)
	{
		mi->number = (byte)i;

		mi->control_number = data[byte_to_read++];
		mi->vid = (short)(data[byte_to_read++] + (data[byte_to_read++] << 8));
		mi->pid = (short)(data[byte_to_read++] + (data[byte_to_read++] << 8));
		mi->type = data[byte_to_read++];
		mi->mode = data[byte_to_read++];
		mi->slot_receiver = data[byte_to_read++];
		mi->port_receiver = data[byte_to_read++];
		add_map(_port, mi);
	}
}

void APT::CPLD_return_read_data(char* data, int size, Mcm6kParams* params)
{
	stringstream ss;
	for (int i = 0; i < 8; i++)
		ss << hex << data[i];
	string cplRcv = ss.str();
}
void APT::stepper_get_limits_params(char* data, int size, Mcm6kParams* params)
{
	byte slot = (byte)(data[0] | data[1]);

	// CW Hardlimit
	short cw_h_lim = (short)(data[2] | data[3] << 8);
	cw_hardlimit_save[slot] = cw_h_lim;

	// CCW Hardlimit
	short ccw_h_lim = (short)(data[4] | data[5] << 8);
	ccw_hardlimit_save[slot] = ccw_h_lim;

	// CW Softlimit
	int cw_s_l = (data[6] | data[7] << 8 | data[8] << 16 | data[9] << 24);

	// CCW Softlimit
	int ccw_s_l = (data[10] | data[11] << 8 | data[12] << 16 | data[13] << 24);

	// Limit Mode
	short lim_mode = (short)(data[14] | data[15] << 8);
	limit_mode_save[slot] = lim_mode;


	// cw swap
	if ((cw_h_lim & 0x80) == 0x80)
	{

	}

	// cw limit 
	cw_h_lim &= 0x7F;
	//cb_cw_hardlimit.SelectedIndex = cw_h_lim - 1;

	// ccw swap
	if ((ccw_h_lim & 0x80) == 0x80)
	{
	}

	//limit_mode.SelectedIndex = lim_mode - 1;

	// ccw limit 
	ccw_h_lim &= 0x7F;
	//cb_ccw_hardlimit.SelectedIndex = ccw_h_lim - 1;
}
void APT::stepper_get_home_params(char* data, int size, Mcm6kParams* params)
{
	byte slot = (byte)(data[0] | data[1]);

	// Home Dir
	short home_dir = (short)(data[2] | data[3] << 8);

	// Limit Switch
	short limit_switch = (short)(data[4] | data[5] << 8);

	// Home Velocity
	int home_vel = (data[6] | data[7] << 8 | data[8] << 16 | data[9] << 24);

	// Offset Distance
	double offset = (data[10] | data[11] << 8 | data[12] << 16 | data[13] << 24);
	offset *= slot_nm_per_count[slot];
	offset /= 1e3;
}
void APT::stepper_get_joystick_params(char* data, int size, Mcm6kParams* params)
{
	byte slot = (byte)(data[0] | data[1]);

	// JSGearLowMaxVel
	int JSGearLowMaxVel = (data[2] | data[3] << 8 | data[4] << 16 | data[5] << 24);

	// JSGearHighMaxVel
	int JSGearHighMaxVel = (data[6] | data[7] << 8 | data[8] << 16 | data[9] << 24);

	// JSGearHighLowAccn (not used)
	int JSGearHighLowAccn = (data[10] | data[11] << 8 | data[12] << 16 | data[13] << 24);

	// JSGearHighHighAccn (not used)
	int JSGearHighHighAccn = (data[14] | data[15] << 8 | data[16] << 16 | data[17] << 24);

	// DirSense
	short DirSense = (short)(data[18] | data[19] << 8);
	if (DirSense == 2) // DIRSENSE_NEG = 2
	{
	}
}
void APT::stepper_get_drive_params(char* data, int size, Mcm6kParams* params)
{
	if (size != 96) return;

	byte slot = (byte)(data[0] | data[1]);
	if ((slot + CARD_ID_START_ADDRESS) == params->x_slot_id)
		memcpy(params->xParams, data, size);
	else if ((slot + CARD_ID_START_ADDRESS) == params->y_slot_id)
		memcpy(params->yParams, data, size);
	else if ((slot + CARD_ID_START_ADDRESS) == params->z_slot_id)
		memcpy(params->zParams, data, size);
	else if ((slot + CARD_ID_START_ADDRESS) == params->r_slot_id)
		memcpy(params->rParams, data, size);
	else if ((slot + CARD_ID_START_ADDRESS) == params->condenser_slot_id)
		memcpy(params->condenserParams, data, size);
	else if ((slot + CARD_ID_START_ADDRESS) == params->aux_slot_id)
		memcpy(params->auxParams, data, size);

	short stage_id = (short)(data[2] | data[3] << 8);
	short axis_id = (short)(data[4] | data[5] << 8); // (not used)
	//char part_no_axis_t[16] = { '\0' };
	// Get the part number
	//Get the name of the slot
	memcpy(params->slotName[slot], data + 6, 16);

	string part_no_string(params->slotName[slot]);

	int axis_serial_no = (data[22] | data[23] << 8 | data[24] << 16 | data[25] << 24);
	int counts_per_unit_t;
	memcpy(&counts_per_unit_t, data + 26, 4);

	slot_counts_per_unit[slot] = (counts_per_unit_t * 100 + 0.5) / 100.0;

	int min_pos = (data[30] | data[31] << 8 | data[32] << 16 | data[33] << 24); // (not used)
	int max_pos = (data[34] | data[35] << 8 | data[36] << 16 | data[37] << 24); // (not used)

	// Stepper_drive_params
	int acc = (data[38] | data[39] << 8 | data[40] << 16 | data[41] << 24);
	int dec = (data[42] | data[43] << 8 | data[44] << 16 | data[45] << 24);
	int max_speed = (data[46] | data[47] << 8 | data[48] << 16 | data[49] << 24);
	short min_speed = (short)(data[50] | data[51] << 8);
	short fs_spd = (short)(data[52] | data[53] << 8);
	byte kval_hold = data[54];
	byte kval_run = data[55];
	byte kval_acc = data[56];
	byte kval_dec = data[57];
	short int_speed = (short)(data[58] | data[59] << 8);
	byte stall_th = data[60];
	byte st_slp = data[61];
	byte fn_slp_acc = data[62];
	byte fn_slp_dec = data[63];
	byte ocd_th = data[64];
	byte step_mode = data[65];
	short config = (short)(data[66] | data[67] << 8);
	float t;
	memcpy(&t, data + 68, 4);
	slot_nm_per_count[slot] = ((int)(t * 10000 + 0.5)) / 10000.0;

	params->slot_counter_per_unit[slot] = slot_counts_per_unit[slot];
	params->slot_nm_per_count[slot] = slot_nm_per_count[slot];

	/*Flags_save*/
	byte flags = data[72];

	/*collision_threshold */
	byte collision_threshold = data[73];

}
void APT::stepper_get_jog_params(char* data, int size, Mcm6kParams* params)
{
	byte slot = (byte)(data[0] | data[1]);

	// jog_mode
	short jog_mode = (short)(data[2] | data[3] << 8);

	// step_size
	int step_size = (data[4] | data[5] << 8 | data[6] << 16 | data[7] << 24);

	// min_vel
	int min_vel = (data[8] | data[9] << 8 | data[10] << 16 | data[11] << 24);

	// acc
	int acc = (data[12] | data[13] << 8 | data[14] << 16 | data[15] << 24);

	// max_vel
	int max_vel = (data[16] | data[17] << 8 | data[18] << 16 | data[19] << 24);

	// stop_mode
	short stop_mode = (short)(data[20] | data[21] << 8);

	double temp;
	// Step size is in encoder counts so we need to convert from um to encoder counts
	temp = (step_size * slot_nm_per_count[slot]) / 1000;
	temp = ((int)(temp * 100 + 0.5)) / 100.0;
}
void APT::servo_get_params(char* data, int size, Mcm6kParams* params)
{
	byte slot = (byte)(data[0] | data[1]);

	// lTransitTime
	int lTransitTime = (data[2] | data[3] << 8 | data[4] << 16 | data[5] << 24);

	// pwm
	int pwm = (data[14] | data[15] << 8 | data[16] << 16 | data[17] << 24);
}
void APT::servo_status_update(char* data, int size, Mcm6kParams* params)
{
	byte _slot = data[0];
	// pos 
	byte position = data[4];
}
void APT::board_update(char* data, int size, Mcm6kParams* params)
{
	// temperature
	short temperature = (short)(data[0] + (data[1] << 8));

	double temperature_c_avg = 0;

	if (temperatures_index < TEMP_SAMPLES)
	{
		temperatures[temperatures_index++] = temperature;
	}
	else
	{
		temperatures_index = 0;
		temperature_ready = true;
	}

	// get the average
	if (temperature_ready)
	{
		sum_temperatures = 0;
		for (int i = 0; i < TEMP_SAMPLES; i++)
		{
			sum_temperatures += temperatures[i];
		}
		temperature_c_avg = sum_temperatures / TEMP_SAMPLES;
	}
	else
	{
		temperature_c_avg = temperature;
	}

	adc1 = (temperature_c_avg * VCC) / 4096.0;

	// in Fahrenheit 
	double temperature_f = (((T0 * B) / (log(R2 * (VCC - adc1) / (R0 * adc1)) * T0 + B)) - 273) * 1.8 + 32;
	double temperature_c = (temperature_f - 32) * 5 / 9;

	double temperature_v = temperature * volts_per_counts; // voltage at resistor divider 

	// Vin monitor
	short vin_monitor = (short)(data[2] + (data[3] << 8));

	double vm = vin_monitor * volts_per_counts;

	double vin = (double)vm / 0.083170;

	// CPU Temperature
	// The output voltage VT = 0.72V at 27C and the temperature slope dVT/dT = 2.33 mV/C
	double cpu_temperature = (short)(data[4] + (data[5] << 8));
	double cpu_temp1 = ((cpu_temperature * 3.3) / 4095) * 1000;
	double cpu_temp_c = ((cpu_temp1 - 720.00) * (100.00 / 233)) + 27.000;
	double cpu_temp_f = (cpu_temp_c * 1.8) + 32;
}
void APT::hexapod_update_xyz(char* data, int size, Mcm6kParams* params)
{
	float temp;
	double x, y, z, theta, phi, psi;
	memcpy(&temp, data + 2, 4);
	x = ((int)(temp * 1000 + 0.5)) / 1000.0;
	memcpy(&temp, data + 6, 4);
	y = ((int)(temp * 1000 + 0.5)) / 1000.0;
	memcpy(&temp, data + 10, 4);
	z = ((int)(temp * 1000 + 0.5)) / 1000.0;
	memcpy(&temp, data + 14, 4);
	theta = ((int)(temp * 1000 + 0.5)) / 1000.0;
	memcpy(&temp, data + 18, 4);
	phi = ((int)(temp * 1000 + 0.5)) / 1000.0;
	memcpy(&temp, data + 22, 4);
	psi = ((int)(temp * 1000 + 0.5)) / 1000.0;
}
void APT::hexapod_update_pid(char* data, int size, Mcm6kParams* params)
{
	int Kp, Ki, Kd, iMax;
	memcpy(&Kp, data + 2, 4);
	memcpy(&Ki, data + 6, 4);
	memcpy(&Kd, data + 10, 4);
	memcpy(&iMax, data + 14, 4);
}
void APT::shutter_get_state(char* data, int size, Mcm6kParams* params)
{
	//Save the shutter position to the first one if the type is Shutter_type
	int channel = 0;
	if (params->cardType[data[5] - CARD_ID_START_ADDRESS] == (USHORT)Shutter_4_type || params->cardType[data[5] - CARD_ID_START_ADDRESS] == (USHORT)Shutter_4_type_REV6)
	{
		byte slot = (byte)(data[5] - CARD_ID_START_ADDRESS);
		byte chan_id = (byte)(data[2] + 1);
		byte state = data[3];
		channel = chan_id - 1;
	}
	else
	{
		byte slot = data[2];
		byte state = data[3];
	}
	byte state = data[3];
	params->shuttersPositions[channel] = state;
}
void APT::shutter_4_get_params(char* data, int size, Mcm6kParams* params)
{
	byte chan_id = data[1];
	byte shutter_initial_state = data[2];
	byte shutter_mode = data[3];
	byte external_trigger_mode = data[4];
	byte on_time = data[5];
	int duty_cycle_pulse = (data[6] | data[7] << 8 | data[8] << 16 | data[9] << 24);
	int duty_cycle_hold = (data[10] | data[11] << 8 | data[12] << 16 | data[13] << 24);
}
void APT::shutter_get_params(char* data, int size, Mcm6kParams* params)
{
	byte slot = data[0];

	byte shutter_initial_state = data[2];
	byte shutter_mode = data[3];
	byte external_trigger_mode = data[4];
	byte on_time = data[5];
	int duty_cycle_pulse = (data[6] | data[7] << 8 | data[8] << 16 | data[9] << 24);
	int duty_cycle_hold = (data[10] | data[11] << 8 | data[12] << 16 | data[13] << 24);

	if (shutter_initial_state == SHUTTER_CLOSED)
	{
	}

}
void APT::get_laser_params(char* data, int size, Mcm6kParams* params)
{
	byte MsgID = data[0];
	byte _slot = data[1];
	int textbox_num = 0;
	int label_num = 0;

	if (MsgID == 1)
	{
		// The Laser power setpoint (0 to 32767 -> 0% to 100% power).
		short laser_power = (short)(data[2] | (data[3] << 8));
		double laser_power_f = int(laser_power / 32767.0 * 100 + 0.5);

	}
	if (MsgID == 12)
	{
		float temp;
		memcpy(&temp, data + 2, 4);
		double laser_temperature = (int(temp * 10000 + 0.5)) / 10000.0;
		double laser_temperature_f = (laser_temperature * 9) / 5 + 32;

		memcpy(&temp, data + 6, 4);
		double laser_power = (int(temp * 10000 + 0.5)) / 10000.0;
	}
}
void APT::system_tab_get_cables(char* data, int size, Mcm6kParams* params)
{
	byte slot = data[0];
	int cable_type = data[2] + (data[3] << 8);
}
void APT::system_tab_get_cables_board(char* data, int size, Mcm6kParams* params)
{
	byte slot = data[0];
	int cable_type = data[2] + (data[3] << 8);
}
void APT::synchonized_motion_get_state(char* data, int size, Mcm6kParams* params)
{
	// get the syncronized motion type for the combo box 
	int sm_type = data[2];

	// get the synchonized motion slot used for X axis from the combo box 
	byte x_slot = data[3];

	// get the synchonized motion slot used for Y axis from the combo box 
	byte y_slot = data[4];

	// get the synchonized motion slot used for Z axis from the combo box 
	byte z_slot = data[5];

	int synchonizedMotionDelta_t = data[6] + (data[7] << 8) + (data[8] << 16) + (data[9] << 24);
}

void APT::system_dim_value_req(char* data, int size, Mcm6kParams* params)
{
	double system_dim_value = data[2];
}

void APT::mirror_position_get(char* data, int size, Mcm6kParams* params)
{
	byte slot = (byte)(data[5] - CARD_ID_START_ADDRESS);
	byte chan_id = (byte)(data[2] + 1);
	byte state = data[3];
	switch (chan_id - 1)
	{
	case GG_MIRROR_CHAN_INDEX: // Response from GG mirror
	{
		if (MIRROR_OUT == state)
		{
			params->lightpathGGPosition = 0;
		}
		else if (MIRROR_IN == state)
		{
			params->lightpathGGPosition = 1;
		}
		else
		{
			params->lightpathGGPosition = 2;
		}
		break;
	}
	case GR_MIRROR_CHAN_INDEX: // Response from GR mirror
	{
		if (MIRROR_OUT == state)
		{
			params->lightpathGRPosition = 0;
		}
		else if (MIRROR_IN == state)
		{
			params->lightpathGRPosition = 1;
		}
		else
		{
			params->lightpathGRPosition = 2;
		}
		break;
	}
	case CAM_MIRROR_CHAN_INDEX: // Response from Camera mirror
	{
		if (MIRROR_OUT == state)
		{
			params->lightpathCameraPosition = 0;
		}
		else if (MIRROR_IN == state)
		{
			params->lightpathCameraPosition = 1;
		}
		else
		{
			params->lightpathCameraPosition = 2;
		}
		break;
	}
	}
}

void APT::shutter_get_interlock_state(char* data, int size, Mcm6kParams* params)
{
	byte slot = (byte)(data[5] - CARD_ID_START_ADDRESS);
	int interlockState = data[2];

	params->safetyInterlockState = interlockState;
}

void APT::piezo_get_mode(char* data, int size, Mcm6kParams* params)
{
	int piezo_mode = data[2];

	params->piezoMode = piezo_mode;
}

void APT::mirrors_get_params(char* data, int size, Mcm6kParams* params)
{
	byte chan_id = data[0];
	byte mirror_initial_state = data[2];
	byte mirror_mode = data[3];
	byte pwm_low_val = data[4];
	byte pwm_high_val = data[5];
}

void APT::clear_all_map_controls_for_port(byte _port)
{
	int number_of_controls = (int)mapping_data[_port].size();
	byte number;

	for (int i = 0; i < number_of_controls; i++)
	{
		// find the control number
		number = ((mapping_data)[_port])[0].number;
		clear_map_control(_port, number);
	}
}

void APT::clear_map_control(byte _port, byte number)
{
	// get the list item that matches the clear button name number
	byte num = 0;

	for (num = 0; num < mapping_data[_port].size(); num++)
	{
		if (mapping_data[_port][num].number == number)
			break;
	}

}

void APT::add_map(byte _port, MapInfo* mi)
{
	byte i = (byte)mapping_data[_port].size();

	// add the mapping info to the list
	MapInfo* newMi = new MapInfo(*mi);
	newMi->number = i;
	mapping_data[_port].push_back(*newMi);
}