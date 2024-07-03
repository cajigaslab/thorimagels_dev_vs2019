#pragma once
#define MSG_SIZE					256
#define TOTAL_CARD_SLOTS			7
#define SECTION_OF_SN				4
#define SECTION_OF_FIRMWARE_REV		3
#define DEFAULT_NM_PER_COUNT		212
#define DEFAULT_XY_NM_PER_COUNT		1777800
#define DEFAULT_Z_NM_PER_COUNT		1907100
#define RESPONSE_WAIT_TIME			20
#define RESPONSE_WAIT_TIME_FTDI		50

#define HOST_ID  0x01
#define MOTHERBOARD_ID  0x11 // 0x11 => MCM 1, 0x12 => MCM2 .
#define CARD_ID_START_ADDRESS		0x21


struct Mcm301Params
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
	UCHAR condenser_slot_id; // Condenser

	uint64_t deviceSerialNo[TOTAL_CARD_SLOTS];

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

	BYTE xParams[96];
	BYTE yParams[96];
	BYTE zParams[96];
	BYTE rParams[96];
	BYTE condenserParams[96];

	long xConfigured;
	long yConfigured;
	long zConfigured;
	long rConfigured;
	long zeConfigured;
	long condenserConfigured;

	long cardConfigured[TOTAL_CARD_SLOTS];

	long pnpErrorRaised[TOTAL_CARD_SLOTS];
	long serialNumberMismatch[TOTAL_CARD_SLOTS];
	long noDeviceFound[TOTAL_CARD_SLOTS];

	char slotName[TOTAL_CARD_SLOTS][16];
};