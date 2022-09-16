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
	UCHAR condenser_slot_id; // Condenser
	UCHAR shutter_slot_id; //Shutter
	UCHAR piezo_slot_id;
	UCHAR ndd_slot_id;

	double xPositionCurrent;
	double yPositionCurrent;
	double zPositionCurrent;
	double rPositionCurrent;
	double zePositionCurrent;
	double condenserPositionCurrent;

	double xMax;
	double yMax;
	double zMax;
	double rMax;
	double condenserMax;

	double xMin;
	double yMin;
	double zMin;
	double rMin;
	double condenserMin;

	double xThreshold;
	double yThreshold;
	double zThreshold;
	double rThreshold;
	double condenserThreshold;

	double xMoveByThreshold;
	double yMoveByThreshold;
	double zMoveByThreshold;
	double rMoveByThreshold;
	double condenserMoveByThreshold;

	bool xInvert;
	bool yInvert;
	bool zInvert;
	bool rInvert;
	bool condenserInvert;

	bool xPidEnable;
	bool yPidEnable;
	bool zPidEnable;
	bool rPidEnable;
	bool condenserPidEnable;

	bool xPidKickoutEnable;
	bool yPidKickoutEnable;
	bool zPidKickoutEnable;
	bool rPidKickoutEnable;
	bool condenserPidKickoutEnable;

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
	bool condenser_ccw_moving;
	bool condenser_cw_moving;
	bool epiTurret_ccw_moving;
	bool epiTurret_cw_moving;
	bool epiTurret_homing;
	bool lightPath_ccw_moving;
	bool lightPath_cw_moving;
	bool lightPath_homing;
	bool ndd_ccw_moving;
	bool ndd_cw_moving;

	BYTE xParams[96];
	BYTE yParams[96];
	BYTE zParams[96];
	BYTE rParams[96];
	BYTE condenserParams[96];

	long invertedLightPathPos;
	long epiTurretCurrentPos;
	long nddCurrentPos;

	long lightpathGGPosition;
	long lightpathGRPosition;
	long lightpathCameraPosition;

	long xConfigured;
	long yConfigured;
	long zConfigured;
	long rConfigured;
	long lightPathConfigured;
	long epiTurretConfigured;
	long zeConfigured;
	long condenserConfigured;
	long piezoConfigured;
	long nddConfigured;
	long shutterConfigured;

	long piezoMode;

	long shuttersPositions[4];
	long safetyInterlockState; // 0 means interlock not installed or trinoc in eyepiece mode, 1 means interlock good and trinoc in camera mode

	char slotName[TOTAL_CARD_SLOTS][16];
};