#include "..\..\PDLL\pdll.h"

class IWinDVI
{
public:
	virtual long ChooseDVI(const wchar_t* id) = 0;
	virtual long EditBMP(int id, unsigned char* bmpBuf, BITMAPINFO bmpInfo) = 0;
	virtual long GetBMP(int id, unsigned char* bmpBuf, BITMAPINFO& bmpInfo) = 0;
	virtual void ClearBMPs() = 0;
	virtual long CreateDVIWindow(int w, int h) = 0;
	virtual long DisplayBMP(int id) = 0;
	virtual void DestroyDVIWindow() = 0;
	//virtual	long GetCurrentBufCount(long &count) = 0;
	virtual long GetLastErrorMsg(wchar_t* msg, long size) = 0;
	virtual long GetStatus(long& status) = 0;
	//virtual void StopDVI() = 0;
};

class WinDVIDLL : public PDLL, public IWinDVI
{
	//call the macro and pass your class name
	DECLARE_CLASS(WinDVIDLL)

	DECLARE_FUNCTION1(long, ChooseDVI, const wchar_t*)
	DECLARE_FUNCTION3(long, EditBMP, int, unsigned char*, BITMAPINFO)
	DECLARE_FUNCTION3(long, GetBMP, int, unsigned char*, BITMAPINFO&)
	DECLARE_FUNCTION0(void, ClearBMPs)
	DECLARE_FUNCTION2(long, CreateDVIWindow, int, int)
	DECLARE_FUNCTION1(long, DisplayBMP, int)
	DECLARE_FUNCTION0(void, DestroyDVIWindow)
	//DECLARE_FUNCTION1(long, GetCurrentBufCount, long &)
	DECLARE_FUNCTION2(long, GetLastErrorMsg, wchar_t*, long)
	DECLARE_FUNCTION1(long, GetStatus, long&)
	//DECLARE_FUNCTION0(void, StopDVI)

};