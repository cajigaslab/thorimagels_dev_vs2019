#include <visa.h>
#include <iostream>
enum OutputState
{
	LED_OUTPUT_OFF = 0,
	LED_OUTPUT_ON = 1
};
enum Mode
{
	MODE_CC = 1,
	MODE_CB = 2,
	MODE_PWM = 3,
	MODE_PULS = 4,
	MODE_IMOD = 5,
	MODE_EMOD = 6,
	MODE_TTL = 7
};
class ThorDC2200Visa
{
public:
	static ViStatus StatusCode;

	static bool FindAndOpenDevice(ViSession &viRM, ViSession &session, long &status);
	static bool CloseDevice(ViSession &viRM, ViSession &session);
	static bool GetIDNStr(ViSession session, char* idnStr);
	static bool GetTTLMaxCurrent(ViSession session, double &current);
	static bool GetTTLCurrent(ViSession session, double &current);
	static bool SetTTLCurrent(ViSession session, double value);
	static bool GetBrightness(ViSession session, double &brightness);
	static bool SetBrightness(ViSession session, double value);
	static bool GetMode(ViSession session, char* mode);
	static bool SetMode(ViSession session, char* mode);
	static bool SetOutputState(ViSession session, long state);
	static bool GetOutputState(ViSession session, long &state);
	static bool SetTerminal(ViSession session, long terminal);
	static bool GetTerminal(ViSession session, long &terminal);
	static bool GetLed1Connection(ViSession session, long &stat);
	static bool GetLed2Connection(ViSession session, long &stat);
};