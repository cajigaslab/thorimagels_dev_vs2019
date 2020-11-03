#include "stdafx.h"
#include "MonitorInfo.h"


BOOL CALLBACK MonitorEnumProc( __in  HMONITOR hMonitor, __in  HDC hdcMonitor, __in  LPRECT lprcMonitor, __in  LPARAM dwData )
{
    std::vector<CMonitorInfoEx>& infoArray = *reinterpret_cast< std::vector<CMonitorInfoEx>* >( dwData );
    CMonitorInfoEx info;
    GetMonitorInfo( hMonitor, &info );
	info.hwndWindow = hMonitor;
    infoArray.push_back( info );
    return TRUE;
}

CMonitorInfoEx::CMonitorInfoEx()
{
    cbSize = sizeof(MONITORINFOEX);
}


CSysDisplays::CSysDisplays()
{
    Update();
}


void CSysDisplays::Update()
{
    mInfo.clear();
    mInfo.reserve( ::GetSystemMetrics(SM_CMONITORS) );
    EnumDisplayMonitors( NULL, NULL, MonitorEnumProc, reinterpret_cast<LPARAM>(&mInfo) );
}


int CSysDisplays::Count() const
{
    return (int)mInfo.size();
}


const CMonitorInfoEx& CSysDisplays::Get( int i ) const
{
    return mInfo[i];
}