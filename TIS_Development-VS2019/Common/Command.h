
//Command.h
#include ".\PDLL\pdll.h"
//dll wrapper class using the virtual class

class ICommand
{
public:

	enum ParamType
	{
		TYPE_LONG,
		TYPE_DOUBLE
	};

	enum Param
	{
		PARAM_FIRST_PARAM = 0,
		PARAM_VERSION = 0,
		PARAM_SHOWDIALOG,		
		PARAM_LAST_PARAM
	};

	
	enum
	{
		PARAM_VERSION_MIN = 1,
		PARAM_VERSION_MAX = 1,
		PARAM_SHOWDIALOG_MIN = 0,
		PARAM_SHOWDIALOG_MAX = 1,
	};

	enum StatusType
	{
		STATUS_BUSY=0,
		STATUS_READY,
		STATUS_ERROR
	};

	enum
	{
		MAXIMUM_PARAMS_BINARY_BUFFER = 16777216
	};

	virtual long GetCommandGUID(GUID * guid) = 0;
	virtual long SetupCommand() = 0;
	virtual long TeardownCommand() = 0;
	virtual long GetParamInfo(const long paramID, long &paramType,long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault) = 0;
	virtual long SetParam(const long paramID, const double param) = 0;
	virtual long GetParam(const long paramID, double &param) = 0;
	virtual long SetCustomParamsBinary(const char *buf) = 0;
	virtual long GetCustomParamsBinary(char *buf) = 0;
	virtual long SaveCustomParamsXML(void *fileHandle) = 0;
	virtual long LoadCustomParamXML(void *fileHandle) = 0;
	virtual long Execute() = 0;
	virtual long Stop() = 0;
	virtual long Status(long &status) = 0;///<returns the status of the capture

};

class CommandDll : public PDLL, public ICommand
{
#pragma warning(push)
#pragma warning(disable:26495)
#pragma warning(disable:26444)
	//call the macro and pass your class name
	DECLARE_CLASS(CommandDll)
#pragma warning(pop)
	
	DECLARE_FUNCTION1(long, GetCommandGUID, GUID *)
	DECLARE_FUNCTION0(long, SetupCommand)
	DECLARE_FUNCTION0(long, TeardownCommand)
	DECLARE_FUNCTION7(long, GetParamInfo, const long , long &, long&, long&, double&, double&, double &)
	DECLARE_FUNCTION2(long, SetParam, const long , const double )
	DECLARE_FUNCTION2(long, GetParam, const long , double &)
	DECLARE_FUNCTION1(long, SetCustomParamsBinary, const char *)
	DECLARE_FUNCTION1(long, GetCustomParamsBinary, char *)
	DECLARE_FUNCTION1(long, SaveCustomParamsXML, void *)
	DECLARE_FUNCTION1(long, LoadCustomParamXML, void *)
	DECLARE_FUNCTION0(long, Execute)
	DECLARE_FUNCTION0(long, Stop)
	DECLARE_FUNCTION1(long, Status,long &)
};

