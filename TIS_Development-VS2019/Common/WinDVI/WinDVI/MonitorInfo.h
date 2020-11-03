//// Below is inspired by Paddy from New Zealand

#include "stdafx.h"
#include <vector>

class CMonitorInfoEx : public MONITORINFOEX
{
public:
    CMonitorInfoEx();

    LPCRECT GetRect() const { return &rcMonitor; }
    LPCRECT GetWorkRect() const { return &rcWork; }
    LPCTSTR DeviceName() const { return szDevice; }

	bool IsPrimary() const { return (dwFlags & MONITORINFOF_PRIMARY) ? true : false; }

    int Width() const { return rcMonitor.right - rcMonitor.left; }
    int Height() const { return rcMonitor.bottom - rcMonitor.top; }
    int WorkWidth() const { return rcWork.right - rcWork.left; }
    int WorkHeight() const { return rcWork.bottom - rcWork.top; }

	HMONITOR hwndWindow;
};


class CSysDisplays
{
public:
    CSysDisplays();

    void Update();

    int Count() const;
    const CMonitorInfoEx& Get( int i ) const;

private:
    std::vector<CMonitorInfoEx> mInfo;
};