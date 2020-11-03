#pragma once
#include <stdio.h>
#include <windows.h>
#include <memory>
#include <math.h>
#include <direct.h>
#include "process.h"
#include <map>
#include <algorithm>
#include <vector>
#include <string>

using namespace std;

#define DEFAULTTIMEOUT					3
#define DEFAULTBAUDRATE					115200
#define BLACKMODE_BIT					0x01
#define WIDEMODE_BIT					0x02
#define MEDIEMMODE_BIT					0x04
#define NARROWMODE_BIT					0x08
#define SUPERNARROWMODE_BIT				0x10
#define SUCCESS							0
#define ERR								-1
#define ERR_DEVICENOTCONNECT			-2
#define ERR_FUNCTIONNOTEXIST			-3
#define ERR_GETSTRINGWRONG				-4
#define ERR_LOADDLLFAILED				-5
#define ERR_LOADDLLFUNCTIONFAILED		-6



typedef int(*fnUART_open)(char *sn, int nBaud, int timeout);
typedef int(*fnUART_isOpen)(int hdl);
typedef int(*fnUART_list)(char *nPort, int var);
typedef int(*fnUART_close)(int hdl);
typedef int(*fnUART_write)(int hdl, char *b, int size);
typedef int(*fnUART_read)(int hdl, char *b, int limit);
typedef int(*fnUART_set)(int hdl, char *c, int var);
typedef int(*fnUART_get)(int hdl, char *c, char *d);
typedef int(*fnUART_Req)(int hdl, char *c, char *d);
typedef void(*fnUART_timeout)(int hdl, int time);
typedef int(*fnUART_Purge)(int hdl, int flag);

typedef struct
{
	fnUART_open open;
	fnUART_isOpen isOpen;
	fnUART_list list;
	fnUART_close close;
	fnUART_write write;
	fnUART_read read;
	fnUART_set set;
	fnUART_get get;
	fnUART_Req Req;
	fnUART_timeout timeout;
	fnUART_Purge Purge;
}UART_LIBRARY;
static UART_LIBRARY fnUART_LIBRARY;

typedef enum
{
	KURIOS_SET_WAVELENGTH = 0,
	KURIOS_SET_BANDWIDTHMODE,
	KURIOS_SET_CONTROLMODE,
	KURIOS_SET_SETSEQUENCE,
	KURIOS_SET_INSERTSEQUENCE,
	KURIOS_SET_DELETESEQUENCE,
	KURIOS_SET_TRIGGEROUTSIGNALMODE,
	KURIOS_SET_FORCETRIGGER,
	KURIOS_SET_TRIGGEROUTTIMEMODE,
	KURIOS_SET_SWITCHDELAY,

	KURIOS_GET_WAVELENGTH = 100,
	KURIOS_GET_TEMPERATURESTATUS,
	KURIOS_GET_TEMPERATURE,
	KURIOS_GET_BANDWIDTHMODE,
	KURIOS_GET_CONTROLMODE,
	KURIOS_GET_ALLSEQUENCES,
	KURIOS_GET_WAVELENGTHRANGE,
	KURIOS_GET_FASTSWITCHINGDATA,
	KURIOS_GET_AVAILABLEBWMODE,
	KURIOS_GET_TRIGGEROUTSIGNALMODE,
	KURIOS_GET_TRIGGEROUTTIMEMODE,
	KURIOS_GET_SWITCHDELAY
}KURIOS_PARAMS_TYPE;

class kurios_handler
{
public:

	kurios_handler();

	~kurios_handler();

	long init();
	long List(char *nPort, long var);
	long Open(char *serialNo, long nbaud, long timeout);

	long Close();

	long SetValue(KURIOS_PARAMS_TYPE type, char* value);
	long SetValue(KURIOS_PARAMS_TYPE type, long value);
	long SetValue(KURIOS_PARAMS_TYPE type, double value);

	long GetValue(KURIOS_PARAMS_TYPE type, char*& value);
	long GetValue(KURIOS_PARAMS_TYPE type, double& value);
	long GetValue(KURIOS_PARAMS_TYPE type, long& value);
	long GetWavelengthRange(long &max, long &min);
	long GetAvailableBandwidthMode(long &max, long &min);

private:

	long ParseString(char* str);
	void Split(std::string& s, std::string& delim, std::vector< std::string >* ret);

private:

	long _nPortHandle;
	map<long, char*> _commandPhrases;

};
