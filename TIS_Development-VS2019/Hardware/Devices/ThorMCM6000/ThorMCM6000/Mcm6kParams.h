#pragma once
#define MSG_SIZE					256
#define TOTAL_CARD_SLOTS			7
#define NUM_OF_TURRET_POS			6
#define SECTION_OF_SN				4
#define SECTION_OF_FIRMWARE_REV		3
#define DEFAULT_NM_PER_COUNT		212
#define DEFAULT_XY_NM_PER_COUNT		1777800
#define DEFAULT_Z_NM_PER_COUNT		1907100
#define RESPONSE_WAIT_TIME			20
#define RESPONSE_WAIT_TIME_FTDI		50

#define MAX_MIRRORS_PER_CARD		3
#define MIRROR_OUT					0
#define MIRROR_IN					1
#define MIRROR_UNKNOWN				2
#define GG_MIRROR_CHAN_INDEX		0
#define GR_MIRROR_CHAN_INDEX		1
#define CAM_MIRROR_CHAN_INDEX		2

#define HOST_ID  0x01
#define MOTHERBOARD_ID  0x11 // 0x11 => MCM 1, 0x12 => MCM2 .
#define CARD_ID_START_ADDRESS		0x21


struct Mcm6kParams
{
	UCHAR serialNum[SECTION_OF_SN];
	UCHAR firmwareRev[SECTION_OF_FIRMWARE_REV];
	UCHAR cpldRev;

	USHORT cardType[TOTAL_CARD_SLOTS];
	USHORT boardType;

	double slot_nm_per_count[TOTAL_CARD_SLOTS];
	double slot_counter_per_unit[TOTAL_CARD_SLOTS];

	UCHAR x_slot_id;
	UCHAR y_slot_id;
	UCHAR z_slot_id;
	UCHAR r_slot_id;
	UCHAR ze_slot_id; // ZE is used to control the Z elevator. NOTE: we don't control this on the software yet.
	UCHAR lp_slot_id; // Light Path
	UCHAR et_slot_id; // Epi Turret
	UCHAR inverted_lp_slot_id; // Inverted Light Path

	double xPositionCurrent;
	double yPositionCurrent;
	double zPositionCurrent;
	double rPositionCurrent;
	double zePositionCurrent;

	double xMax;
	double yMax;
	double zMax;
	double rMax;

	double xMin;
	double yMin;
	double zMin;
	double rMin;

	double xThreshold;
	double yThreshold;
	double zThreshold;
	double rThreshold;

	double xMoveByThreshold;
	double yMoveByThreshold;
	double zMoveByThreshold;
	double rMoveByThreshold;

	bool xInvert;
	bool yInvert;
	bool zInvert;
	bool rInvert;

	bool xPidEnable;
	bool yPidEnable;
	bool zPidEnable;
	bool rPidEnable;

	bool xPidKickoutEnable;
	bool yPidKickoutEnable;
	bool zPidKickoutEnable;
	bool rPidKickoutEnable;

	bool x_ccw_moving;
	bool x_cw_moving;
	bool y_ccw_moving;
	bool y_cw_moving;
	bool z_ccw_moving;
	bool z_cw_moving;
	bool r_ccw_moving;
	bool r_cw_moving;
	bool ze_ccw_moving;
	bool ze_cw_moving;

	byte xParams[96];
	byte yParams[96];
	byte zParams[96];
	byte rParams[96];

	long invertedLightPathPos;
	long epiTurretCurrentPos;

	long lightpathGGPosition;
	long lightpathGRPosition;
	long lightpathCameraPosition;

	long zeAvailable;
};