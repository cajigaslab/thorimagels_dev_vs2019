#pragma once

#include "..\..\..\Common\thread.h"


class ThorLoggingClass
{
public:
	~ThorLoggingClass();
    static ThorLoggingClass* getInstance();
	void TraceEvent(EventType eventType,int id,LPWSTR str);

private:
    ThorLoggingClass();
	
    static bool instanceFlag;
    static ThorLoggingClass *single;
	static CritSect critSect;
	static void cleanup(void);	
};