#pragma once

#if defined(RESOURCE_MANAGER)
#define DllExport_ResourceManager __declspec(dllexport)
#else
//    definitions used when using DLL
#define DllExport_ResourceManager __declspec(dllimport)
#endif

#pragma warning( push )
#pragma warning( disable : 4251 )

using namespace std;

class DllExport_ResourceManager ResourceManager
{
private:
	static wstring _appDir;
	static wstring _appSettingsDir;
	static wstring _captureTemplateDir;
	static wstring _MyDocumentsThorImageDir;
	static wstring _zStackCacheDir;
	static wstring _activeSettingsFile;
	static wstring _appSettingsFile;
	static wstring _hwSettingsFile;
	static wstring _modality;
	static const wstring MODALITY_TAG_NAME;

	ResourceManager();
	static bool instanceFlag;
	static auto_ptr<ResourceManager> _single;

	static void LocateDirectories();

public:
	wstring GetApplicationPath();

	wstring GetApplicationSettingsPath();

	wstring GetCaptureTemplatePath();

	wstring GetMyDocumentsThorImageFolder();

	wstring GetZStackCachePath();

	wstring GetActiveSettingsFilePathAndName(); 

	wstring GetApplicationSettingsFilePathAndName();

	//retrieve application settings filepath under given modality name
	wstring GetModalityApplicationSettingsFilePathAndName(wchar_t* modality);

	wstring GetHardwareSettingsFilePathAndName();

	//retrieve hardware settings filepath under given modality name
	wstring GetModalityHardwareSettingsFilePathAndName(wchar_t* modality);

	wstring GetModality();
	long SetModality(wchar_t* modalityName);

	//force resource manager to reload settings from current modality
	static long LoadSettings();

	//retrieve a long type attribute from one defined node in settings
	long GetSettingsParamLong(int settingsFileType, wchar_t* tagName, wchar_t* attribute, long defaultValue = 0);

	//access control of settings documents
	bool BorrowDocMutex(long sfType, long timeMS = -1);

	//return control of settings documents
	bool ReturnDocMutex(long sfType);

	~ResourceManager();
	static ResourceManager* getInstance();

};

#pragma warning( push )