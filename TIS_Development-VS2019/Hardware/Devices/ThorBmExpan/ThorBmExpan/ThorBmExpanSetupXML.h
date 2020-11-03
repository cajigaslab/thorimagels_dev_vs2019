#pragma once
#define NUM_EXP_RATIO 7

class ThorBmExpanXML
{
private:
	ticpp::Document * _xmlObj;
	wchar_t _currentPathAndFile[_MAX_PATH];	

public:
	 
	 enum {NUM_CONNECTION_ATTRIBUTES = 6};
	 enum {NUM_EXP_POS = NUM_EXP_RATIO};

	 static const char * const CONNECTION;
	 static const char * const CONNECTION_ATTR[NUM_CONNECTION_ATTRIBUTES];
	 static const char * const EXP_RATIOS;
	 static const char * const EXP_RATIOS_ATTR[NUM_EXP_POS];
	 static const char * const MOT0_POS;
	 static const char * const MOT0_POS_ATTR[NUM_EXP_POS];
	 static const char * const MOT1_POS;
	 static const char * const MOT1_POS_ATTR[NUM_EXP_POS];

	ThorBmExpanXML();
	~ThorBmExpanXML();

	long GetConnection(long &portID0,long &portID1, long &baudRate0, long &baudRate1, long &address0, long &address1);
	long GetExpRatios(long &Ratio0, long &Ratio1, long &Ratio2, long &Ratio3, long &Ratio4, long &Ratio5, long &Ratio6);
	long GetMOT0_Pos(long &Pos0, long &Pos1, long &Pos2, long &Pos3, long &Pos4, long &Pos5, long &Pos6);
	long GetMOT1_Pos(long &Pos0, long &Pos1, long &Pos2, long &Pos3, long &Pos4, long &Pos5, long &Pos6);
	long OpenConfigFile();

};