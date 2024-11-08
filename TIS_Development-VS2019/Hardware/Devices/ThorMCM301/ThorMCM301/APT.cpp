#pragma once
#include <sstream>
#include <math.h>
#include "MCM301.h"
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

void APT::ParseApt(char* buf, int len, Mcm301Params* params)
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
	case MGMSG_MCM_HW_GET_INFO:
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

	case MGMSG_MCM_MOT_GET_LIMSWITCHPARAMS:
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

	case MGMSG_BOARD_GET_STATUSUPDATE:
		board_update(extData, extDataSize, params);
		break;

	case MGMSG_MOT_GET_DCPIDPARAMS:
		hexapod_update_pid(extData, extDataSize, params);
		break;

	case MGMSG_GET_DEVICE:
		system_tab_get_cables(extData, extDataSize, params); //Changed command
		break;

	case MGMSG_MOD_GET_SYSTEM_DIM:
		system_dim_value_req(header, 6, params);
		break;

	case MGMSG_MCM_GET_SLOT_TITLE:
		system_get_slot_title(extData, extDataSize, params);
		break;

	case MGMSG_MCM_GET_PNPSTATUS:
		stepper_get_pnp_status(extData, extDataSize, params);
		break;
	}
	free(extData);
}

void APT::hardware_info(char* data, int size, Mcm301Params* params)
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

void APT::mcm_stepper_status_update(char* data, int size, Mcm301Params* params)
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
}

void APT::usb_device_info(char* data, int size, Mcm301Params* params)
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

void APT::usb_device_mapping_in(char* data, int size, Mcm301Params* params)
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

void APT::usb_device_mapping_out(char* data, int size, Mcm301Params* params)
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

void APT::usb_device_mapping(char* data, int size, Mcm301Params* params)
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

void APT::CPLD_return_read_data(char* data, int size, Mcm301Params* params)
{
	stringstream ss;
	for (int i = 0; i < 8; i++)
		ss << hex << data[i];
	string cplRcv = ss.str();
}
void APT::stepper_get_limits_params(char* data, int size, Mcm301Params* params)
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
void APT::stepper_get_home_params(char* data, int size, Mcm301Params* params)
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
void APT::stepper_get_joystick_params(char* data, int size, Mcm301Params* params)
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
void APT::stepper_get_drive_params(char* data, int size, Mcm301Params* params)
{
	if (size != 96) return;

	byte slot = (byte)(data[0] | data[1]);
	if ((slot + CARD_ID_START_ADDRESS) == params->x_slot_id)
	{
		memcpy(params->xParams, data, size);
		//params->xConfigured = TRUE;
		//params->cardConfigured[0 + 4] = TRUE;
	}
	else if ((slot + CARD_ID_START_ADDRESS) == params->y_slot_id)
	{
		memcpy(params->yParams, data, size);
		//params->yConfigured = TRUE;
		//params->cardConfigured[1 + 4] = TRUE;
	}
	else if ((slot + CARD_ID_START_ADDRESS) == params->z_slot_id)
	{
		memcpy(params->zParams, data, size);
		//params->zConfigured = TRUE;
		//params->cardConfigured[2 + 4] = TRUE;
	}
	else if ((slot + CARD_ID_START_ADDRESS) == params->r_slot_id)
	{
		memcpy(params->rParams, data, size);
		params->rConfigured = TRUE;
	}
	else if ((slot + CARD_ID_START_ADDRESS) == params->condenser_slot_id)
	{
		memcpy(params->condenserParams, data, size);
		params->zeConfigured = TRUE;
	}

	short stage_id = (short)(data[2] | data[3] << 8);
	short axis_id = (short)(data[4] | data[5] << 8); // (not used)
	//char part_no_axis_t[16] = { '\0' };
	// Get the part number
	//Get the name of the slot
	//memcpy(params->slotName[slot], data + 6, 16); //Deprecated, replaced by slot title command

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
void APT::stepper_get_jog_params(char* data, int size, Mcm301Params* params)
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

void APT::board_update(char* data, int size, Mcm301Params* params)
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
void APT::hexapod_update_pid(char* data, int size, Mcm301Params* params)
{
	int Kp, Ki, Kd, iMax;
	memcpy(&Kp, data + 2, 4);
	memcpy(&Ki, data + 6, 4);
	memcpy(&Kd, data + 10, 4);
	memcpy(&iMax, data + 14, 4);
}

void APT::system_tab_get_cables(char* data, int size, Mcm301Params* params)
{
	byte slot = (byte)(data[0] | data[1]);
	if (/*params->x_slot_id == (CARD_ID_START_ADDRESS + slot)*/slot == 4 && !params->noDeviceFound[4])
	{
		memcpy(&(params->deviceSerialNo[4]), (data + 2), 8);

	}
	else if (/*params->y_slot_id == (CARD_ID_START_ADDRESS + slot)*/slot == 5 && !params->noDeviceFound[5])
	{
		memcpy(&(params->deviceSerialNo[5]), (data + 2), 8);
	}
	else if (/*params->z_slot_id == (CARD_ID_START_ADDRESS + slot)*/slot == 6 && !params->noDeviceFound[6])
	{
		memcpy(&(params->deviceSerialNo[6]), (data + 2), 8);
	}
}

void APT::system_dim_value_req(char* data, int size, Mcm301Params* params)
{
	double system_dim_value = data[2];
}

void APT::system_get_slot_title(char* data, int size, Mcm301Params* params) 
{
	byte slot = (byte)(data[0] | data[1]);
	memcpy(params->slotName[slot], data + 2, 16);
}

void APT::stepper_get_pnp_status(char* data, int size, Mcm301Params* params) 
{
	byte slot = (byte)(data[0] | data[1]);
	int pnpStatusFlags[9];
	int flagRaised = 0;
	//Set the values of the pnp status flags (there are only 9 right now)
	for (int i = 0; i < 8; i++) 
	{
		pnpStatusFlags[i] = ((data[2] >> i) & 0x01);
		if (pnpStatusFlags[i] == 1) 
		{
			flagRaised = 1;
		}
	}
	pnpStatusFlags[8] = (data[3] & 0x01);
	if (pnpStatusFlags[8] == 1)
	{
		flagRaised = 1;
	}

	//Assign general boolean for any error raised
	if (flagRaised)
	{
		params->pnpErrorRaised[slot] = 1;
	}
	else 
	{
		params->pnpErrorRaised[slot] = 0;
	}

	//Assign specific boolean to the Serial Number Mismatch flag
	if (pnpStatusFlags[4] == 1)
	{
		params->serialNumberMismatch[slot] = 1;
	}
	else 
	{
		params->serialNumberMismatch[slot] = 0;
	}

	//Assign specific boolean when no device is detected on slot card
	if (pnpStatusFlags[0] == 1) 
	{
		params->noDeviceFound[slot] = 1;
	}
	else
	{
		params->noDeviceFound[slot] = 0;
	}


	//pnpStatusFlags Bit Values:
	//Bit 0: No Device Detected - Set when no device detected on slot card
	//Bit 1: General One-Wire Error - Set when one-wire error is raised
	//Bit 2: Unknown One-Wire Version - Set when the connected device's presented one-wire version is not known
	//Bit 3: One-Wire Corruption - Set when connected device's one-wire checksum mismatches with calculated checksum
	//Bit 4: Serial Number Mismatch - Set when serial number checking is enabled and the connected device's serial number does not match what was saved
	//Bit 5: Device Signature Not Allowed - Set when the connected device's signature is not allowed on the slot
	//Bit 6: General Configuration Error - Set when any error occurred during the configuration step (after device detection)
	//Bit 7: Device Configuration Set Miss - Set when a device configuration set from the Device LUT could not be found
	//Bit 8: Configuration Struct Miss - Set when configuration structure required by device configuration set could not be found or was invalid
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