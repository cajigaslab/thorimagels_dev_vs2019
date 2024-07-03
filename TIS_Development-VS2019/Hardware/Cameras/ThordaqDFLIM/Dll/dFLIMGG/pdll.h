////////////////////////////////////////////////////////////////////////// Class: PDll 
//
// Authors: MicHael Galkovsky // Date: April 14, 1998 
// Company: Pervasive Software 
// Purpose: Base class to wrap dynamic use of dll 
////////////////////////////////////////////////////////////////////////

#if !defined (_PDLL_H_)
#define _PDLL_H_

#include <windows.h>
#include <winbase.h>

#define FUNC_LOADED 3456

//function declarations according to the number of parameters
//define the type
//declare a variable of that type
//declare a member function by the same name as the dll function
//check for dll handle
//if this is the first call to the function then try to load it
//if not then if the function was loaded successfully make a call to it
//otherwise return a NULL cast to the return parameter.

#define DECLARE_FUNCTION0(retVal, FuncName) \
typedef retVal (WINAPIV* TYPE_##FuncName)(); \
TYPE_##FuncName m_##FuncName; \
long m_is##FuncName; \
retVal FuncName() \
{ \
if (m_dllHandle) \
{ \
if (FUNC_LOADED != m_is##FuncName) \
{\
m_##FuncName = NULL; \
m_##FuncName = (TYPE_##FuncName)GetProcAddress(m_dllHandle, #FuncName); \
m_is##FuncName = FUNC_LOADED;\
}\
if (NULL != m_##FuncName) \
return m_##FuncName(); \
else \
return (retVal)NULL; \
} \
else \
return (retVal)NULL; \
} 

#define DECLARE_FUNCTION1(retVal, FuncName, Param1) \
typedef retVal (WINAPIV* TYPE_##FuncName)(Param1); \
TYPE_##FuncName m_##FuncName; \
long m_is##FuncName;\
retVal FuncName(Param1 p1) \
{ \
if (m_dllHandle) \
{ \
if (FUNC_LOADED != m_is##FuncName) \
{\
m_##FuncName = NULL; \
m_##FuncName = (TYPE_##FuncName)GetProcAddress(m_dllHandle, #FuncName); \
m_is##FuncName = FUNC_LOADED;\
}\
if (NULL != m_##FuncName) \
return m_##FuncName(p1); \
else \
return (retVal)NULL; \
} \
else \
return (retVal)NULL; \
}


#define DECLARE_FUNCTION2(retVal, FuncName, Param1, Param2) \
typedef retVal (WINAPIV* TYPE_##FuncName)(Param1,Param2); \
TYPE_##FuncName m_##FuncName; \
long m_is##FuncName;\
retVal FuncName(Param1 p1,Param2 p2) \
{ \
if (m_dllHandle) \
{ \
if (FUNC_LOADED != m_is##FuncName) \
{\
m_##FuncName = NULL; \
m_##FuncName = (TYPE_##FuncName)GetProcAddress(m_dllHandle, #FuncName); \
m_is##FuncName = FUNC_LOADED;\
}\
if (NULL != m_##FuncName) \
return m_##FuncName(p1,p2); \
else \
return (retVal)NULL; \
} \
else \
return (retVal)NULL; \
}


#define DECLARE_FUNCTION3(retVal, FuncName, Param1, Param2, Param3) \
typedef retVal (WINAPIV* TYPE_##FuncName)(Param1,Param2, Param3); \
TYPE_##FuncName m_##FuncName; \
long m_is##FuncName;\
retVal FuncName(Param1 p1,Param2 p2, Param3 p3) \
{ \
if (m_dllHandle) \
{ \
if (FUNC_LOADED != m_is##FuncName) \
{\
m_##FuncName = NULL; \
m_##FuncName = (TYPE_##FuncName)GetProcAddress(m_dllHandle, #FuncName); \
m_is##FuncName = FUNC_LOADED;\
}\
if (NULL != m_##FuncName) \
return m_##FuncName(p1,p2,p3); \
else \
return (retVal)NULL; \
} \
else \
return (retVal)NULL; \
}

#define DECLARE_FUNCTION4(retVal, FuncName, Param1, Param2, Param3, Param4) \
typedef retVal (WINAPIV* TYPE_##FuncName)(Param1,Param2, Param3, Param4); \
TYPE_##FuncName m_##FuncName; \
long m_is##FuncName;\
retVal FuncName(Param1 p1,Param2 p2,Param3 p3, Param4 p4) \
{ \
if (m_dllHandle) \
{ \
if (FUNC_LOADED != m_is##FuncName) \
{\
m_##FuncName = NULL; \
m_##FuncName = (TYPE_##FuncName)GetProcAddress(m_dllHandle, #FuncName); \
m_is##FuncName = FUNC_LOADED;\
}\
if (NULL != m_##FuncName) \
return m_##FuncName(p1,p2,p3,p4); \
else \
return (retVal)NULL; \
} \
else \
return (retVal)NULL; \
}


#define DECLARE_FUNCTION5(retVal, FuncName, Param1, Param2, Param3, Param4, Param5) \
typedef retVal (WINAPIV* TYPE_##FuncName)(Param1,Param2, Param3, Param4, Param5); \
TYPE_##FuncName m_##FuncName; \
long m_is##FuncName;\
retVal FuncName(Param1 p1,Param2 p2,Param3 p3, Param4 p4, Param5 p5) \
{ \
if (m_dllHandle) \
{ \
if (FUNC_LOADED != m_is##FuncName) \
{\
m_##FuncName = NULL; \
m_##FuncName = (TYPE_##FuncName)GetProcAddress(m_dllHandle, #FuncName); \
m_is##FuncName = FUNC_LOADED;\
}\
if (NULL != m_##FuncName) \
return m_##FuncName(p1,p2,p3,p4,p5); \
else \
return (retVal)NULL; \
} \
else \
return (retVal)NULL; \
}

#define DECLARE_FUNCTION6(retVal, FuncName, Param1, Param2, Param3, Param4, Param5, Param6) \
typedef retVal (WINAPIV* TYPE_##FuncName)(Param1,Param2, Param3, Param4, Param5, Param6); \
TYPE_##FuncName m_##FuncName; \
long m_is##FuncName;\
retVal FuncName(Param1 p1,Param2 p2,Param3 p3, Param4 p4, Param5 p5, Param6 p6) \
{ \
if (m_dllHandle) \
{ \
if (FUNC_LOADED != m_is##FuncName) \
{\
m_##FuncName = NULL; \
m_##FuncName = (TYPE_##FuncName)GetProcAddress(m_dllHandle, #FuncName); \
m_is##FuncName = FUNC_LOADED;\
}\
if (NULL != m_##FuncName) \
return m_##FuncName(p1,p2,p3,p4,p5,p6); \
else \
return (retVal)NULL; \
} \
else \
return (retVal)NULL; \
}


#define DECLARE_FUNCTION7(retVal, FuncName, Param1, Param2, Param3, Param4, Param5, Param6, Param7) \
typedef retVal (WINAPIV* TYPE_##FuncName)(Param1,Param2, Param3, Param4, Param5, Param6, Param7); \
TYPE_##FuncName m_##FuncName; \
long m_is##FuncName;\
retVal FuncName(Param1 p1,Param2 p2,Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7) \
{ \
if (m_dllHandle) \
{ \
if (FUNC_LOADED != m_is##FuncName) \
{\
m_##FuncName = NULL; \
m_##FuncName = (TYPE_##FuncName)GetProcAddress(m_dllHandle, #FuncName); \
m_is##FuncName = FUNC_LOADED;\
}\
if (NULL != m_##FuncName) \
return m_##FuncName(p1,p2,p3,p4,p5,p6,p7); \
else \
return (retVal)NULL; \
} \
else \
return (retVal)NULL; \
}

#define DECLARE_FUNCTION8(retVal, FuncName, Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8) \
typedef retVal (WINAPIV* TYPE_##FuncName)(Param1,Param2, Param3, Param4, Param5, Param6, Param7, Param8); \
TYPE_##FuncName m_##FuncName; \
long m_is##FuncName;\
retVal FuncName(Param1 p1,Param2 p2,Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8) \
{ \
if (m_dllHandle) \
{ \
if (FUNC_LOADED != m_is##FuncName) \
{\
m_##FuncName = NULL; \
m_##FuncName = (TYPE_##FuncName)GetProcAddress(m_dllHandle, #FuncName); \
m_is##FuncName = FUNC_LOADED;\
}\
if (NULL != m_##FuncName) \
return m_##FuncName(p1,p2,p3,p4,p5,p6,p7,p8); \
else \
return (retVal)NULL; \
} \
else \
return (retVal)NULL; \
}

#define DECLARE_FUNCTION9(retVal, FuncName, Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9) \
typedef retVal (WINAPIV* TYPE_##FuncName)(Param1,Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9); \
TYPE_##FuncName m_##FuncName; \
long m_is##FuncName;\
retVal FuncName(Param1 p1,Param2 p2,Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8, Param9 p9) \
{ \
if (m_dllHandle) \
{ \
if (FUNC_LOADED != m_is##FuncName) \
{\
m_##FuncName = NULL; \
m_##FuncName = (TYPE_##FuncName)GetProcAddress(m_dllHandle, #FuncName); \
m_is##FuncName = FUNC_LOADED;\
}\
if (NULL != m_##FuncName) \
return m_##FuncName(p1,p2,p3,p4,p5,p6,p7,p8,p9); \
else \
return (retVal)NULL; \
} \
else \
return (retVal)NULL; \
}

#define DECLARE_FUNCTION10(retVal, FuncName, Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10) \
typedef retVal (WINAPIV* TYPE_##FuncName)(Param1,Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10); \
TYPE_##FuncName m_##FuncName; \
long m_is##FuncName;\
retVal FuncName(Param1 p1,Param2 p2,Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8, Param9 p9, Param10 p10) \
{ \
if (m_dllHandle) \
{ \
if (FUNC_LOADED != m_is##FuncName) \
{\
m_##FuncName = NULL; \
m_##FuncName = (TYPE_##FuncName)GetProcAddress(m_dllHandle, #FuncName); \
m_is##FuncName = FUNC_LOADED;\
}\
if (NULL != m_##FuncName) \
return m_##FuncName(p1,p2,p3,p4,p5,p6,p7,p8,p9,p10); \
else \
return (retVal)NULL; \
} \
else \
return (retVal)NULL; \
}

#define DECLARE_FUNCTION11(retVal, FuncName, Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11) \
typedef retVal (WINAPIV* TYPE_##FuncName)(Param1,Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11); \
TYPE_##FuncName m_##FuncName; \
long m_is##FuncName;\
retVal FuncName(Param1 p1,Param2 p2,Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8, Param9 p9, Param10 p10, Param11 p11) \
{ \
if (m_dllHandle) \
{ \
if (FUNC_LOADED != m_is##FuncName) \
{\
m_##FuncName = NULL; \
m_##FuncName = (TYPE_##FuncName)GetProcAddress(m_dllHandle, #FuncName); \
m_is##FuncName = FUNC_LOADED;\
}\
if (NULL != m_##FuncName) \
return m_##FuncName(p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11); \
else \
return (retVal)NULL; \
} \
else \
return (retVal)NULL; \
}

#define DECLARE_FUNCTION12(retVal, FuncName, Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12) \
typedef retVal (WINAPIV* TYPE_##FuncName)(Param1,Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12); \
TYPE_##FuncName m_##FuncName; \
long m_is##FuncName;\
retVal FuncName(Param1 p1,Param2 p2,Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8, Param9 p9, Param10 p10, Param11 p11, Param12 p12) \
{ \
if (m_dllHandle) \
{ \
if (FUNC_LOADED != m_is##FuncName) \
{\
m_##FuncName = NULL; \
m_##FuncName = (TYPE_##FuncName)GetProcAddress(m_dllHandle, #FuncName); \
m_is##FuncName = FUNC_LOADED;\
}\
if (NULL != m_##FuncName) \
return m_##FuncName(p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12); \
else \
return (retVal)NULL; \
} \
else \
return (retVal)NULL; \
}

#define DECLARE_FUNCTION13(retVal, FuncName, Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13) \
typedef retVal (WINAPIV* TYPE_##FuncName)(Param1,Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13); \
TYPE_##FuncName m_##FuncName; \
long m_is##FuncName;\
retVal FuncName(Param1 p1,Param2 p2,Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8, Param9 p9, Param10 p10, Param11 p11, Param12 p12, Param13 p13) \
{ \
if (m_dllHandle) \
{ \
if (FUNC_LOADED != m_is##FuncName) \
{\
m_##FuncName = NULL; \
m_##FuncName = (TYPE_##FuncName)GetProcAddress(m_dllHandle, #FuncName); \
m_is##FuncName = FUNC_LOADED;\
}\
if (NULL != m_##FuncName) \
return m_##FuncName(p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12,p13); \
else \
return (retVal)NULL; \
} \
else \
return (retVal)NULL; \
}

#define DECLARE_FUNCTION16(retVal, FuncName, Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15, Param16) \
typedef retVal (WINAPIV* TYPE_##FuncName)(Param1,Param2, Param3, Param4, Param5, Param6, Param7, Param8, Param9, Param10, Param11, Param12, Param13, Param14, Param15, Param16); \
TYPE_##FuncName m_##FuncName; \
long m_is##FuncName;\
retVal FuncName(Param1 p1,Param2 p2,Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8, Param9 p9, Param10 p10, Param11 p11, Param12 p12, Param13 p13, Param14 p14, Param15 p15, Param16 p16) \
{ \
if (m_dllHandle) \
{ \
if (FUNC_LOADED != m_is##FuncName) \
{\
m_##FuncName = NULL; \
m_##FuncName = (TYPE_##FuncName)GetProcAddress(m_dllHandle, #FuncName); \
m_is##FuncName = FUNC_LOADED;\
}\
if (NULL != m_##FuncName) \
return m_##FuncName(p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12,p13,p14,p15,p16); \
else \
return (retVal)NULL; \
} \
else \
return (retVal)NULL; \
}

// add macros for FUNCTION with 2, 3, 4 params

//declare constructors and LoadFunctions
#define DECLARE_CLASS(ClassName) \
public: \
ClassName (const WCHAR* name){LoadDll(name);} \
ClassName () {PDLL();}

class PDLL
{

protected:
HINSTANCE m_dllHandle;
WCHAR* m_dllName;
int m_refCount;

public:

PDLL()
{
m_dllHandle = NULL;
m_dllName = NULL;
m_refCount = 0;
}

#define MAXSTRLENGTH    255
BOOL Char2Wchar(TCHAR* pDest, char* pSrc, int nDestStrLen)
{
     int nSrcStrLen = 0;
     int nOutputBuffLen = 0;
     int retcode = 0;

     if(pDest == NULL || pSrc == NULL)
     {
          //SysDebug(MID_EXCEPTION, "Char2Wchar: Input Args NULL\n");
          return FALSE;
     }

     nSrcStrLen = static_cast<int>(strlen(pSrc));
     if(nSrcStrLen == 0)
     {  
          //SysDebug(MID_EXCEPTION, "Char2Wchar: Strlen zero\n");
          return FALSE;
     }

     nDestStrLen = nSrcStrLen;

     if (nDestStrLen > MAXSTRLENGTH - 1)
     {
          //SysDebug(MID_EXCEPTION, "Char2Wchar: Check nSrcStrLen\n");
          return FALSE;
     }
     memset(pDest,0,sizeof(TCHAR)*nDestStrLen);
     nOutputBuffLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pSrc,
     nSrcStrLen, pDest, nDestStrLen);
 
     if (nOutputBuffLen == 0)
     {
          retcode = GetLastError();
          //Sysdebug("Char2Wchar : MultiByteToWideChar returned ERROR \n",0,0);  
          return FALSE;
     }

     pDest[nOutputBuffLen] = '\0';
     return TRUE;
}

//A NULL here means the name has already been set
void LoadDll(const WCHAR* name, long showMsg = 0)
{
if (name)
SetDllName(name);

//	TCHAR dest[MAXSTRLENGTH];
	
//	Char2Wchar(dest, m_dllName, strlen(m_dllName));

//try to load
m_dllHandle = LoadLibrary(m_dllName);

if (m_dllHandle == NULL && showMsg)
{
//show warning here if needed
} 
}

bool SetDllName(const WCHAR* newName)
{
bool retVal = false;

//we allow name resets only if the current DLL handle is invalid
//once they've hooked into a DLL, the name cannot be changed
if (!m_dllHandle)
{
if (m_dllName)
{
delete []m_dllName;
m_dllName = NULL;
}

//They may be setting this null (e.g., uninitialize)
if (newName)
{
m_dllName = new WCHAR[wcslen(newName) + 1];
//make sure memory was allocated
if (m_dllName)
wcscpy_s(m_dllName, wcslen(newName)+1, newName);
else
retVal = false;
}
retVal = true;
}
return retVal;
}

virtual bool Initialize(long showMsg = 0)
{

bool retVal = false;

//Add one to our internal reference counter
m_refCount++;

if (m_refCount == 1 && m_dllName) //if this is first time, load the DLL
{
//we are assuming the name is already set
LoadDll(NULL, showMsg);
retVal = (m_dllHandle != NULL);
}
return retVal;
}

virtual void Uninitialize(void)
{
//If we're already completely unintialized, early exit
if (!m_refCount)
return;

//if this is the last time this instance has been unitialized, 
//then do a full uninitialization
m_refCount--;

if (m_refCount < 1)
{
if (m_dllHandle)
{
FreeLibrary(m_dllHandle);
m_dllHandle = NULL;
}

SetDllName(NULL); //clear out the name & free memory
}
}

~PDLL()
{
//force this to be a true uninitialize
m_refCount = 1; 
Uninitialize();

//free name
if (m_dllName)
{
delete [] m_dllName;
m_dllName = NULL;
}
}

};
#endif 
