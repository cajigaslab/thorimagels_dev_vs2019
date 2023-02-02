
//SLM.h
#include ".\PDLL\pdll.h"
//dll wrapper class using the virtual class

template <typename T>
class MemoryStruct
{
private:
	T* _memPtr = NULL;
	DWORD _bytesSize;
	BITMAPINFO _bmi;
	T _kzValue;
	int _index;			//used for location in buffer array

public:
	MemoryStruct() { _memPtr = NULL; _bytesSize = 0; _kzValue = 0; _index = -1; };

	MemoryStruct(T kz) { _memPtr = NULL; _bytesSize = 0; _kzValue = kz; _index = -1; };

	~MemoryStruct() { ReallocMemChk(0); };

	void operator=(const MemoryStruct& b) { this->ReallocMemChk(0);	if (NULL != b._memPtr) { this->_memPtr = b._memPtr; this->_bmi = b._bmi; this->_kzValue = b._kzValue; } }

	T* GetMem() { return _memPtr; }

	DWORD GetSize() { return _bytesSize; }

	BITMAPINFO GetInfo() { return _bmi; }
	void SetInfo(BITMAPINFO& bm) { _bmi = bm; }

	T GetKz() { return _kzValue; }
	void SetKz(T val) { _kzValue = val; }

	int GetIndex() { return _index; }
	void SetIndex(int val) { _index = val; }

	void ReallocMemChk(DWORD byteSize)
	{
		if (0 == byteSize) { if (NULL != _memPtr) { free(_memPtr); _memPtr = NULL; }; _bytesSize = 0; }
		else if (byteSize != _bytesSize) { T* tmp = (T*)realloc((void*)_memPtr, byteSize); if (NULL != tmp) { _memPtr = tmp; _bytesSize = byteSize; } }
	}

	void CallocMemChk(DWORD unitSize)
	{
		ReallocMemChk(0);
		if (0 != unitSize) { T* tmp = (T*)calloc(unitSize, sizeof(T)); if (NULL != tmp) { _memPtr = tmp; _bytesSize = unitSize * sizeof(T); } }
	}

};

#define MAX_ARRAY_CNT	1024

class ISLM
{
public:

	enum SLMHardware
	{
		PDM512,
		EXULUS
	};

	enum SLMParams
	{
		IS_AVAILABLE,
		ARRAY_ID,
		TIMEOUT,
		RUNTIME_CALC,
		WRITE_BUFFER,
		WRITE_FIRST_BUFFER,
		GET_CURRENT_BUFFER,
		SET_TRANSIANT_BUFFER,
		WRITE_TRANSIANT_BUFFER,
		RELEASE_TRANSIANT_BUFFER
	};

	enum SLMBlank
	{
		BLANK_ALL,
		BLANK_LEFT,
		BLANK_RIGHT
	};

	virtual long FindSLM(char* slm) = 0;///<returns the number of SLMs
	virtual long TeardownSLM() = 0;///<release SLM and its resources
	virtual long SetParam(const long paramID, const double param) = 0;
	virtual long GetParam(const long paramID, double& param) = 0;
	virtual long GetParamBuffer(const long paramID, char* pBuffer, long size) = 0;
	virtual long SetParamBuffer(const long paramID, char* pBuffer, long size) = 0;
	//virtual long SetBlank(const long paramID) = 0;///<This submits the current parameters to the Device. There may be some first time latencey in settings parameters so its best to call this outside of the shutter control for image capture
	//virtual long WriteFrame(long frameID, const unsigned char* buf) = 0;///<This submits the parameters that can chage while the Device is active. 
	virtual long StartSLM() = 0;///<Begin the capture.
	virtual long StopSLM() = 0;///<clean up if necessary
	virtual long UpdateSLM(long arrayID) = 0;///<Begin the capture.
	virtual long GetLastErrorMsg(wchar_t* msg, long size) = 0;///<Retrieve the description for the last error
};

class SLMDll : public PDLL, public ISLM
{
#pragma warning(push)
#pragma warning(disable:26495)
#pragma warning(disable:26444)
	//call the macro and pass your class name
	DECLARE_CLASS(SLMDll)
#pragma warning(pop)

	DECLARE_FUNCTION1(long, FindSLM, char*)
	DECLARE_FUNCTION0(long, TeardownSLM)
	//DECLARE_FUNCTION7(long, GetParamInfo, const long, long&, long&, long&, double&, double&, double&)
	DECLARE_FUNCTION2(long, SetParam, const long, const double)
	DECLARE_FUNCTION2(long, GetParam, const long, double&)
	//DECLARE_FUNCTION1(long, SetBlank, const long)
	//DECLARE_FUNCTION2(long, WriteFrame, long, const unsigned char*)
	DECLARE_FUNCTION0(long, StartSLM)
	DECLARE_FUNCTION0(long, StopSLM)
	DECLARE_FUNCTION1(long, UpdateSLM, long)
	DECLARE_FUNCTION2(long, GetLastErrorMsg, wchar_t*, long)
	//DECLARE_FUNCTION2(long, SetParamString, const long, wchar_t*)
	//DECLARE_FUNCTION3(long, GetParamString, const long, wchar_t*, long)
	DECLARE_FUNCTION3(long, GetParamBuffer, const long, char*, long)
	DECLARE_FUNCTION3(long, SetParamBuffer, const long, char*, long)
};