#include "stdafx.h"

#include "ExportedFunctions.h"
#include "ResourceManager.h"
#include <wchar.h>

DllExportFunc long GetApplicationSettingsPath(wchar_t* stringBuffer, unsigned int bufferLength)
{

	std::wstring path = ResourceManager::getInstance()->GetApplicationSettingsPath();

	//Make sure buffer is large enough to prevent overflow
	if(path.size() > bufferLength)
	{
		return false;	
	}

	//Copy into buffer
	else
	{
		wcsncpy_s(stringBuffer, bufferLength, path.c_str(), bufferLength);
	}

	return true;

}

DllExportFunc long GetActiveSettingsFilePathAndName(wchar_t* stringBuffer, unsigned int bufferLength)
{

	std::wstring path = ResourceManager::getInstance()->GetActiveSettingsFilePathAndName();

	//Make sure buffer is large enough to prevent overflow
	if(path.size() > bufferLength)
	{
		return false;	
	}

	//Copy into buffer
	else
	{
		wcsncpy_s(stringBuffer, bufferLength, path.c_str(), bufferLength);
	}

	return true;
}

DllExportFunc long GetApplicationSettingsFilePathAndName(wchar_t* stringBuffer, unsigned int bufferLength)
{

	std::wstring path = ResourceManager::getInstance()->GetApplicationSettingsFilePathAndName();

	//Make sure buffer is large enough to prevent overflow
	if(path.size() > bufferLength)
	{
		return false;	
	}

	//Copy into buffer
	else
	{
		wcsncpy_s(stringBuffer, bufferLength, path.c_str(), bufferLength);
	}

	return true;
}

DllExportFunc long GetModalityApplicationSettingsFilePathAndName(wchar_t* modality, wchar_t* stringBuffer, unsigned int bufferLength)
{
	std::wstring path = ResourceManager::getInstance()->GetModalityApplicationSettingsFilePathAndName(modality);

	//Make sure buffer is large enough to prevent overflow
	if(path.size() > bufferLength)
	{
		return false;	
	}

	//Copy into buffer
	else
	{
		wcsncpy_s(stringBuffer, bufferLength, path.c_str(), bufferLength);
	}

	return true;
}


DllExportFunc long GetCaptureTemplatePath(wchar_t* stringBuffer, unsigned int bufferLength)
{

	std::wstring path = ResourceManager::getInstance()->GetCaptureTemplatePath();

	//Make sure buffer is large enough to prevent overflow
	if(path.size() > bufferLength)
	{
		return false;	
	}

	//Copy into buffer
	else
	{
		wcsncpy_s(stringBuffer, bufferLength, path.c_str(), bufferLength);
	}

	return true;
}

DllExportFunc long GetHardwareSettingsFilePathAndName(wchar_t* stringBuffer, unsigned int bufferLength)
{

	std::wstring path = ResourceManager::getInstance()->GetHardwareSettingsFilePathAndName();

	//Make sure buffer is large enough to prevent overflow
	if(path.size() > bufferLength)
	{
		return false;	
	}

	//Copy into buffer
	else
	{
		wcsncpy_s(stringBuffer, bufferLength, path.c_str(), bufferLength);
	}

	return true;
}

DllExportFunc long GetModalityHardwareSettingsFilePathAndName(wchar_t* modality, wchar_t* stringBuffer, unsigned int bufferLength)
{
	std::wstring path = ResourceManager::getInstance()->GetModalityHardwareSettingsFilePathAndName(modality);

	//Make sure buffer is large enough to prevent overflow
	if(path.size() > bufferLength)
	{
		return false;	
	}

	//Copy into buffer
	else
	{
		wcsncpy_s(stringBuffer, bufferLength, path.c_str(), bufferLength);
	}

	return true;
}


DllExportFunc long SetModality(wchar_t* modalityName)
{
	return ResourceManager::getInstance()->SetModality(modalityName);
}

DllExportFunc long GetModality(wchar_t* stringBuffer, unsigned int bufferLength)
{

	std::wstring modality = ResourceManager::getInstance()->GetModality();

	//Make sure buffer is large enough to prevent overflow
	if(modality.size() > bufferLength)
	{
		return false;	
	}

	//Copy into buffer
	else
	{
		wcsncpy_s(stringBuffer, bufferLength, modality.c_str(), bufferLength);
	}

	return true;
}

DllExportFunc long GetApplicationPath(wchar_t* stringBuffer, unsigned int bufferLength)
{

	std::wstring path = ResourceManager::getInstance()->GetApplicationPath();

	//Make sure buffer is large enough to prevent overflow
	if(path.size() > bufferLength)
	{
		return false;	
	}

	//Copy into buffer
	else
	{
		wcsncpy_s(stringBuffer, bufferLength, path.c_str(), bufferLength);
	}

	return true;
}

DllExportFunc long SetMyDocumentsThorImageFolderPath(wchar_t* folderPath)
{
	return ResourceManager::getInstance()->SetMyDocumentsThorImageFolderPath(folderPath);
}

DllExportFunc long GetMyDocumentsThorImageFolder(wchar_t* stringBuffer, unsigned int bufferLength)
{

	std::wstring path = ResourceManager::getInstance()->GetMyDocumentsThorImageFolder();

	//Make sure buffer is large enough to prevent overflow
	if(path.size() > bufferLength)
	{
		return false;	
	}

	//Copy into buffer
	else
	{
		wcsncpy_s(stringBuffer, bufferLength, path.c_str(), bufferLength);
	}

	return true;
}

DllExportFunc long GetSettingsParamLong(int settingsFileType, wchar_t* tagName, wchar_t* attribute, long defaultValue = 0)
{
	return ResourceManager::getInstance()->GetSettingsParamLong(settingsFileType, tagName, attribute, defaultValue);
}

DllExportFunc long ReloadDirectories()
{
	return ResourceManager::getInstance()->ReloadDirectories();
}

DllExportFunc long LoadSettings()
{
	return ResourceManager::getInstance()->LoadSettings();
}

DllExportFunc long GetStartupFlag(wchar_t* stringBuffer, unsigned int bufferLength)
{
	std::wstring value = ResourceManager::getInstance()->GetStartupFlag();
	if (value.size() > bufferLength)
	{
		return false;
	}
	else 
	{
		wcsncpy_s(stringBuffer, bufferLength, value.c_str(), bufferLength);
	}

	return true;
}

DllExportFunc long SetStartupFlag(wchar_t* value)
{
	return ResourceManager::getInstance()->SetStartupFlag(value);
}

DllExportFunc bool BorrowDocMutex(long sfType, long timeMS)
{
	return ResourceManager::getInstance()->BorrowDocMutex(sfType, timeMS);
}

DllExportFunc bool ReturnDocMutex(long sfType)
{
	return ResourceManager::getInstance()->ReturnDocMutex(sfType);
}