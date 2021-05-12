
#include "stdafx.h"
#include <mutex>
#include "ResourceManager.h"

auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
auto_ptr<CommandDll> shwDll(new CommandDll(L".\\Modules_Native\\SelectHardware.dll"));
std::timed_mutex hSettingsDocMutex[(int)SettingsFileType::SETTINGS_FILE_LAST];

wchar_t message[MAX_PATH];

long CreateTag(ticpp::Document *obj, string tag)
{
	// make sure the top level root element exist
	ticpp::Element *configObj = obj->FirstChildElement(false);

	if ( configObj == NULL )
	{
		return FALSE;
	}
	else
	{
		//get the attribute value for the specified attribute name
		ticpp::Element * element = new ticpp::Element(tag);

		configObj->LinkEndChild(element);

		return TRUE;
	}
}

long SetAttribute(ticpp::Document *obj, string tagName, string attribute, string attributeValue)
{
	try
	{
		if(NULL == obj)
			return FALSE;

		// make sure the top level root element exist
		ticpp::Element *configObj = obj->FirstChildElement(false);

		if ( configObj == NULL )
			return FALSE;

		// iterate over to get the particular tag element specified as a parameter(tagName)
		ticpp::Iterator<ticpp::Element> child(configObj->FirstChildElement(tagName), tagName);
		//get the attribute value for the specified attribute name
		child->SetAttribute(attribute, attributeValue);
	}
	catch(ticpp::Exception ex)
	{
		//char buf[512];
		//WCHAR wbuf[512];
		//const char* msg = ex.what();
		//StringCbPrintfA(buf,512, "%s=> Tag: %s Attribute: %s", msg, tagName.c_str(), attribute.c_str());
		//MultiByteToWideChar(0, 0, buf, -1, wbuf, 512);  
		//logDll->TLTraceEvent(VERBOSE_EVENT, 1, wbuf);
		return FALSE;
	}
	return TRUE;
}

long GetAttribute(ticpp::Document *obj, string tagName, string attribute, string &attributeValue)
{
	try
	{
		if(NULL == obj)
			return FALSE;

		// make sure the top level root element exist
		ticpp::Element *configObj = obj->FirstChildElement(false);

		if ( configObj == NULL )
			return FALSE;

		// iterate over to get the particular tag element specified as a parameter(tagName)
		ticpp::Iterator<ticpp::Element> child(configObj->FirstChildElement(tagName), tagName);
		//get the attribute value for the specified attribute name
		child->GetAttribute(attribute, &attributeValue);
	}
	catch(ticpp::Exception ex)
	{
		//char buf[512];
		//WCHAR wbuf[512];
		//const char* msg = ex.what();
		//StringCbPrintfA(buf,512, "%s=> Tag: %s Attribute: %s", msg, tagName.c_str(), attribute.c_str());
		//MultiByteToWideChar(0, 0, buf, -1, wbuf, 512);  
		//logDll->TLTraceEvent(VERBOSE_EVENT, 1, wbuf);
		return FALSE;
	}
	return TRUE;
}

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
	if(true == instanceFlag)
	{
		const string file = ".\\ResourceManager.xml";
		auto_ptr<ticpp::Document> obj(new ticpp::Document(file));

		try
		{
			obj->LoadFile();
		}
		catch(ticpp::Exception ex)
		{
			StringCbPrintfW(message,MAX_PATH,L"ResourceManager Locate Directories Failed");

			logDll->TLTraceEvent(ERROR_EVENT,1,message);	
		}

		//Do not save modality if it is empty
		if(_modality.length() > 0)
		{
			//get parent tag
			ticpp::Element *configObj = obj->FirstChildElement(false);

			ticpp::Element * modalityElement = configObj->FirstChildElement("Modality",false);

			if(NULL == modalityElement)
			{
				CreateTag(obj.get(),"Modality");
			}

			string strModality = ConvertWStringToString(_modality);

			if(SetAttribute(obj.get(), "Modality", "value", strModality))
			{
			}
			
			obj->SaveFile();
		}
	}
	instanceFlag = false;
}

bool ResourceManager::instanceFlag = false;
wstring ResourceManager::_appDir;
wstring ResourceManager::_appSettingsDir;
wstring ResourceManager::_captureTemplateDir;
wstring ResourceManager::_MyDocumentsThorImageDir;
wstring ResourceManager::_zStackCacheDir;
wstring ResourceManager::_activeSettingsFile;
wstring ResourceManager::_appSettingsFile;
wstring ResourceManager::_hwSettingsFile;
wstring ResourceManager::_modality;
wstring ResourceManager::_autoFocusCacheDir;
const wstring ResourceManager::MODALITY_TAG_NAME = L"Modality";

auto_ptr<ResourceManager> ResourceManager::_single(new ResourceManager());
auto_ptr<ticpp::Document> _settingsDoc[(int)SettingsFileType::SETTINGS_FILE_LAST];

ResourceManager* ResourceManager::getInstance()
{
	if(! instanceFlag)
	{
		try
		{
			_single.reset(new ResourceManager());

			wsprintf(message,L"ResourceManager Created");
			logDll->TLTraceEvent(VERBOSE_EVENT,1,message);

			LocateDirectories();
		}
		catch(...)
		{
			//critically low on resources
			//do not proceed with application
			throw;
		}
		instanceFlag = true;
	}
	return _single.get();
}

long ResourceManager::LoadSettings()
{
	try
	{
		//not loading active experiment since it will be replaced by one from modality folder,
		//here only loading application and hardware settings to be shared among cpp level.
		_settingsDoc[(int)SettingsFileType::APPLICATION_SETTINGS].reset(new ticpp::Document(WStringToString(_appSettingsFile)));
		_settingsDoc[(int)SettingsFileType::APPLICATION_SETTINGS]->LoadFile();

		_settingsDoc[(int)SettingsFileType::HARDWARE_SETTINGS].reset(new ticpp::Document(WStringToString(_hwSettingsFile)));
		_settingsDoc[(int)SettingsFileType::HARDWARE_SETTINGS]->LoadFile();
	}
	catch(ticpp::Exception ex)
	{
		StringCbPrintfW(message,MAX_PATH,L"ResourceManager load settings Failed");
		logDll->TLTraceEvent(ERROR_EVENT,1,message);	
		return FALSE;
	}
	return TRUE;
}

void ResourceManager::LocateDirectories()
{
	const string file = ".\\ResourceManager.xml";
	auto_ptr<ticpp::Document> obj(new ticpp::Document(file));

	try
	{
		obj->LoadFile();
	}
	catch(ticpp::Exception ex)
	{
		StringCbPrintfW(message,MAX_PATH,L"ResourceManager Locate Directories Failed");

		logDll->TLTraceEvent(ERROR_EVENT,1,message);	
	}

	string str;

	if(!GetAttribute(obj.get(), "DocumentsFolder", "value", str))
	{
	}

	wstring thorImageFolder = L"\\" + StringToWString(str) + L"\\"; 
	wstring modalityFolder;

	string strModality;

	if(GetAttribute(obj.get(), "Modality", "value", strModality))
	{
		modalityFolder = L"\\Modalities\\" + StringToWString(strModality) + L"\\";
		_modality = StringToWString(strModality);
	}

	wchar_t buffer[_MAX_PATH];
	GetModuleFileName( NULL, buffer, _MAX_PATH );

	_appDir = wstring(buffer);

	if(SUCCEEDED(SHGetFolderPath(NULL,CSIDL_PERSONAL|CSIDL_FLAG_CREATE,NULL,0,buffer))) 
	{
		_MyDocumentsThorImageDir = wstring(buffer);
		_MyDocumentsThorImageDir += thorImageFolder;

		_appSettingsDir = wstring(_MyDocumentsThorImageDir);
		_appSettingsDir += wstring(L"Application Settings\\");

		wstring tmp = wstring(_MyDocumentsThorImageDir) +  modalityFolder;
		DWORD ftyp = GetFileAttributes(tmp.c_str());
		if(INVALID_FILE_ATTRIBUTES == ftyp || _modality.compare(L"") == 0)
		{
			tmp = wstring(_MyDocumentsThorImageDir) + L"Modalities\\";
			ftyp = GetFileAttributes(tmp.c_str());
			if(INVALID_FILE_ATTRIBUTES == ftyp)
			{
				modalityFolder = L"";
				_modality = L"";
			}
			else
			{
				tmp = tmp + L"*";
				LPWIN32_FIND_DATA ffd =  new WIN32_FIND_DATA();
				HANDLE hf = FindFirstFile(tmp.c_str(), ffd);
				if(INVALID_HANDLE_VALUE == hf)
				{
					modalityFolder = L"";
					_modality = L"";
				}
				else
				{
					BOOL r;

					while(wstring(ffd->cFileName).compare(L".") == 0 || wstring(ffd->cFileName).compare(L"..") == 0)
					{
						r = FindNextFile(hf, ffd);
						// if no modality is found in the modalities folder, break out of the while loop
						if(!r)		
						{
							break;
						} 
						else 
						{
						}

					}
					if(r != 0 && ffd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					{
						modalityFolder = L"\\Modalities\\" + wstring(ffd->cFileName) + L"\\";
						_modality = wstring(ffd->cFileName);

					}
					else
					{
						modalityFolder = L"";
						_modality = L"";
					}
				}
				FindClose(hf);
				delete ffd;
			}
		}

		_captureTemplateDir = wstring(_MyDocumentsThorImageDir);
		_captureTemplateDir += wstring(L"Capture Templates\\"); 

		_zStackCacheDir = wstring(_MyDocumentsThorImageDir);
		_zStackCacheDir += wstring(L"ZStackCache\\");

		_activeSettingsFile = _captureTemplateDir + wstring(L"Active.xml");

		_appSettingsFile = wstring(_MyDocumentsThorImageDir);
		_appSettingsFile += modalityFolder;
		_appSettingsFile += wstring(L"Application Settings\\ApplicationSettings.xml");

		_hwSettingsFile = wstring(_MyDocumentsThorImageDir);
		_hwSettingsFile += modalityFolder;
		_hwSettingsFile += wstring(L"Application Settings\\HardwareSettings.xml");

		_autoFocusCacheDir = wstring(_MyDocumentsThorImageDir);
		_autoFocusCacheDir += wstring(L"AutoFocusCache\\");

		LoadSettings();
	}
}

wstring ResourceManager::GetActiveSettingsFilePathAndName()
{
	return _activeSettingsFile;
}

wstring ResourceManager::GetApplicationPath()
{
	return _appDir;
}

wstring ResourceManager::GetApplicationSettingsPath()
{
	return _appSettingsDir;
}

wstring ResourceManager::GetCaptureTemplatePath()
{
	return _captureTemplateDir;
}

wstring ResourceManager::GetMyDocumentsThorImageFolder()
{
	return _MyDocumentsThorImageDir;
}

wstring ResourceManager::GetZStackCachePath()
{
	return _zStackCacheDir;
}

wstring ResourceManager::GetAutoFocusCachePath()
{
	return _autoFocusCacheDir;
}

wstring ResourceManager::GetApplicationSettingsFilePathAndName()
{
	return _appSettingsFile;
}

wstring ResourceManager::GetModalityApplicationSettingsFilePathAndName(wchar_t* modality)
{
	wstring temp;
	temp = wstring(_MyDocumentsThorImageDir);
	temp +=  L"Modalities\\";
	temp += modality;
	temp += L"\\";

	DWORD ftyp = GetFileAttributes(temp.c_str());

	if (ftyp == INVALID_FILE_ATTRIBUTES)
	{
		//something is wrong with your path, return from current modality
		return _appSettingsFile;  
	}
	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
	{
		//found directory, return target settings without affecting current modality
		return temp + wstring(L"Application Settings\\ApplicationSettings.xml");
	}
	return _appSettingsFile;
}

wstring ResourceManager::GetHardwareSettingsFilePathAndName()
{
	return _hwSettingsFile;
}

wstring ResourceManager::GetModalityHardwareSettingsFilePathAndName(wchar_t* modality)
{
	wstring temp;
	temp = wstring(_MyDocumentsThorImageDir);
	temp +=  L"Modalities\\";
	temp += modality;
	temp += L"\\";

	DWORD ftyp = GetFileAttributes(temp.c_str());

	if (ftyp == INVALID_FILE_ATTRIBUTES)
	{
		//something is wrong with your path, return from current modality
		return _hwSettingsFile;  
	}
	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
	{
		//found directory, return target settings without affecting current modality
		return temp + wstring(L"Application Settings\\HardwareSettings.xml");
	}
	return _hwSettingsFile;
}

long ResourceManager::SetModality(wchar_t* modalityName)
{
	wstring temp;
	wstring modality = modalityName;
	if(0 == wcslen(modalityName))
	{
		modality = GetModality();
	}

	temp = wstring(_MyDocumentsThorImageDir);
	temp +=  L"Modalities\\";
	temp += modality;
	temp += L"\\";

	DWORD ftyp = GetFileAttributes(temp.c_str());

	if (ftyp == INVALID_FILE_ATTRIBUTES)
	{
		//something is wrong with your path!
		return FALSE;  
	}
	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
	{
		// this is a directory!
		_appSettingsFile = temp + wstring(L"Application Settings\\ApplicationSettings.xml");
		_appSettingsDir = temp + wstring(L"Application Settings\\"); 
		_hwSettingsFile = temp + wstring(L"Application Settings\\HardwareSettings.xml");
		_modality = modality;

		shwDll->LoadCustomParamXML(NULL);

		return TRUE;
	}

	return FALSE;
}

wstring ResourceManager::GetModality()
{
	return _modality;
}

long ResourceManager::GetSettingsParamLong(int settingsFileType, wchar_t* tagName, wchar_t* attribute, long defaultValue)
{
	long ret = defaultValue;
	std::string strValue;
	if(GetAttribute(_settingsDoc[(int)settingsFileType].get(), WStringToString(tagName), WStringToString(attribute), strValue))
	{
		ret = atoi(strValue.c_str());
	}
	return ret;
}

bool ResourceManager::BorrowDocMutex(long sfType, long timeMS)
{
	if (0 > timeMS)
	{
		hSettingsDocMutex[sfType].lock();
		return true;
	}
	else
	{
		return hSettingsDocMutex[sfType].try_lock_for(std::chrono::milliseconds(timeMS));
	}
}

bool ResourceManager::ReturnDocMutex(long sfType)
{
	hSettingsDocMutex[sfType].unlock();
	return true;
}

