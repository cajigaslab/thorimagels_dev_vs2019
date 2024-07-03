
#include "stdafx.h"
#include "Blink_SDK.h"

#ifdef __cplusplus
extern "C"
{
#endif

	class MeadowlarkPDM : ISLM
	{
	private:
		static bool _instanceFlag;
		static unique_ptr<MeadowlarkPDM> _single;

		const int  DEFAULT_TRUE_FRAMES;
		const unsigned int DEFAULT_TRANSIENT_FRAMES;
		const unsigned int MAX_TRANSIENT_FRAMES;

		static wchar_t _errMsg[MSG_SIZE];
		static MemoryStruct<unsigned char> _intermediateBuf[MAX_ARRAY_CNT]; ///<buffer for all overdrive SLM transient frames
		static Blink_SDK* _blinkSDK;
		static unsigned int _slmTimeout; ///<timeout value for overdrive SLM, can be multiple of min value
		static MemoryStruct<unsigned char> _slmTempBuf; ///<temperary buffer for calculation of transient frames

		std::string _pSlmName;///<slm device name
		long _deviceCount; ///<how many SLM being detected
		long _overDrive; ///<overdrive mode for meadowlark slm
		unsigned char* _transferBuf[2]; ///<[0]buffer for the first SLM pattern, used for calculating transient back to first.
										///<[1]buffer to hold for transiant or temp.
		size_t _transferBufSize[2];

		//params in struct:		
		long _slmFuncMode; ///<different function mode of slm
		static long _arrayOrFileID; ///<index of slm points set or bitmap buffers
		static long _bufferCount; ///<total count of pattern buffers in circulation
		static long _slmRuntimeCalculate; ///<runtime calculation of transient frames

	public:
		static MeadowlarkPDM* getInstance();

		long FindSLM(char* xml);
		long GetParam(const long paramID, double& param);
		long SetParam(const long paramID, const double param);
		long TeardownSLM();
		long GetParamBuffer(const long paramID, char* pBuffer, long size);
		long SetParamBuffer(const long paramID, char* pBuffer, long size);
		long StartSLM();
		long StopSLM();
		long UpdateSLM(long arrayID);
		long GetLastErrorMsg(wchar_t* msg, long size);

		~MeadowlarkPDM();

	private:
		MeadowlarkPDM();
		BOOL IsOverdrive() { return ((_overDrive) && (0 == _pSlmName.compare(EnumString<SLMHardware>::From(SLMHardware::PDM512))) && (NULL != _blinkSDK)) ? TRUE : FALSE; }
		void ReleaseMem();
		void ReleaseTransientBuf();
		void SetDefault();
		long SetIntermediateBuffer(unsigned char* mem, size_t size);

	};

#ifdef __cplusplus
}
#endif