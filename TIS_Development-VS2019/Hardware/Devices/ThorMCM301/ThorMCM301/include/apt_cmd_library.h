#pragma once
#ifndef APT_CMD_LIBRARY_H
#define APT_CMD_LIBRARY_H

#ifndef APT_CMD_EXPORTS
#define APT_CMD_STATIC
#endif // !APT_CMD_EXPORTS

#ifdef APT_CMD_STATIC
#define APT_CMD_API extern "C"
#else
#define APT_CMD_API extern "C" __declspec(dllexport)
#endif // !APT_CMD_STATIC

APT_CMD_API int fnAPT_DLL_InitDevice(int hDevice);
APT_CMD_API int fnAPT_DLL_RleaseDevice(int hDevice);
APT_CMD_API int fnAPT_DLL_GetMessageType(unsigned char *msg, unsigned int msgLen, int &type, unsigned int &size);
APT_CMD_API int fnAPT_DLL_SetMsgSrcDest(int hDevice, unsigned char dest1, unsigned char dest2, unsigned char src, unsigned char cid);

APT_CMD_API int fnAPT_DLL_StartCtrl(int hDevice, unsigned char* msg, bool reqinfo = true);
APT_CMD_API int fnAPT_DLL_MOD_EnableChannel(int hDevice, int en, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_HW_Req_Info(int hDevice, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_HW_Get_Info(int hDevice, char *buf, unsigned char& source, long &serial_number, char *mode_number,
	unsigned short &type, unsigned char *firmware_version, char *notes, unsigned char* empty_space, unsigned short &hw_version,
	unsigned short &mode_state, unsigned short &nchs);
APT_CMD_API int fnAPT_DLL_MOD_Req_ChanEnableState(int hDevice, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOD_Get_ChanEnableState(int hDevice, char *Msg_Data, unsigned char& source, unsigned char &chanId,
	unsigned char &enablesatus);
APT_CMD_API int fnAPT_DLL_MOD_Identify(int hDevice, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_HW_StartUpdateMsgs(int hDevice, unsigned char rate, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_HW_StopUpdateMsgs(int hDevice, unsigned char* msg);

APT_CMD_API int fnAPT_DLL_RestoreFactorySettings(int hDevice, unsigned char* msg);

#endif // !APT_CMD_LIBRARY_H