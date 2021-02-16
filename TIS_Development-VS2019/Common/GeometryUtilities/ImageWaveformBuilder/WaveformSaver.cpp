// WaveformSaver.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "WaveformSaver.h"
#include "..\..\ResourceManager\ResourceManager\ResourceManager.h"
#include "..\..\..\Tools\tinyxml2\include\tinyxml2.h"
#include "..\..\StringCPP.h"
#include "..\..\csvWriter.h"
#include "..\..\HDF5IOdll.h"
#include "..\..\ImageWaveformBuilderDll.h"

using namespace std;
using namespace tinyxml2;

///	***************************************** <summary>				WaveformSaver Constructor			</summary>	********************************************** ///

WaveformSaver::WaveformSaver()
{
}

///	***************************************** <summary> WaveformSaver static members & global variables	</summary>	********************************************** ///

bool WaveformSaver::_instanceFlag = false;
std::unique_ptr<WaveformSaver> WaveformSaver::_single;

std::unique_ptr<HDF5ioDLL> h5Loader(new HDF5ioDLL(L".\\Modules_Native\\HDF5IO.dll"));
std::unique_ptr<WaveformMemoryDLL> memLoader(new WaveformMemoryDLL(L".\\Modules_Native\\GeometryUtilitiesCPP.dll"));

///	***************************************** <summary>				WaveformSaver Functions				</summary>	********************************************** ///

// singleton
WaveformSaver* WaveformSaver::getInstance()
{
	if (!_instanceFlag)
	{
		try
		{
			_single.reset(new WaveformSaver());
			_instanceFlag = true;
			return _single.get();
		}
		catch (...)
		{
			throw;
		}
	}
	return _single.get();
}

///<summary>Save waveform to configured type, allow writing in order of [XY, POCKELS, DIGI, Z] </summary>
long WaveformSaver::SaveData(wstring outPath, SignalType stype, void* gparam, unsigned long long length)
{
	const wstring FNAME = L"\\Waveforms";
	const wstring FNAMEXY = L"\\WaveformXY";
	const wstring FNAMEZ = L"\\WaveformZ";
	const wstring FNAMEINFO = L"\\WaveformInfo";
	const string WAVEXY = "/WaveformXY";
	const string WAVEZ = "/WaveformZ";
	const string WAVEINFO = "/WaveformInfo";
	const string CLOCKR = "CLOCKRATE[Hz]";
	const string GALVOX = "GALVOX[V]";
	const string GALVOY = "GALVOY[V]";
	const string PIEZOZ = "PIEZOZ[V]";

	if (0 >= outPath.length() || NULL == gparam || 0 == length || SignalType::SIGNALTYPE_LAST <= stype)
		return FALSE;

	wchar_t fileName[MAX_PATH];
	wstring tempPath = ResourceManager::getInstance()->GetApplicationSettingsFilePathAndName();
	StringCbPrintfW(fileName, _MAX_PATH, tempPath.c_str());

	// load the ApplicationSettings.xml 
	tinyxml2::XMLDocument doc;
	if (doc.LoadFile(WStringToString(fileName).c_str()) != 0)
	{
		StringCbPrintfW(message, _MAX_PATH, L"%hs@%u: Application settings is not available. (%hs)", __FUNCTION__, __LINE__, __FILE__);
		LogMessage(message, VERBOSE_EVENT);
		return FALSE;
	}
	XMLElement* root = doc.RootElement();
	XMLElement* niSettingNode = root->FirstChildElement("ExperimentSaving");
	if (niSettingNode == NULL)
	{
		StringCbPrintfW(message, _MAX_PATH, L"%hs@%u: Node ExperimentSaving is not available in application settings. (%hs)", __FUNCTION__, __LINE__, __FILE__);
		LogMessage(message, VERBOSE_EVENT);
		return FALSE;
	}

	//parse for user selected type
	string saveType = niSettingNode->Attribute("saveWaveformRawH5Csv");
	std::transform(saveType.begin(), saveType.end(), saveType.begin(), [](unsigned char c) { return std::tolower(c); });

	//***************************************//
	// handle file saving to different types //
	//***************************************//
	double* pSrc = (double*)gparam;
	if (0 == saveType.compare("raw"))
	{
		char* ptr = NULL;
		long acctssAttribute = CREATE_ALWAYS;

		//open or create the same file if not exist
		if (FileExists(WStringToString(outPath + FNAME + L".raw")))
		{
			memLoader.get()->OpenMem(_gParams, wstring(outPath + FNAME + L".raw").c_str());
			acctssAttribute = OPEN_EXISTING;
		}
		else
		{
			_gParams.analogXYSize = _gParams.analogZSize = _gParams.analogPockelSize = _gParams.digitalSize = 0;
			_gParams.digitalLineCnt = 0;
			_gParams.driverType = _gParams.pockelsCount = 0;
			_gParams.stepVolt = 0.1; //to allow pass AllocateMem
			_gParams.ClockRate = 1;  //to allow pass AllocateMem
		}
		switch (stype)
		{
			case ANALOG_XY:
				_gParams.analogXYSize = length * 2;
				SAFE_MEMCPY(&_gParams.ClockRate, sizeof(double), pSrc);
				memLoader.get()->AllocateMem(_gParams, wstring(outPath + FNAME + L".raw").c_str(), acctssAttribute);
				ptr = memLoader.get()->GetMemMapPtr(ANALOG_XY, 0, _gParams.analogXYSize);
				SAFE_MEMCPY(ptr, length * sizeof(double), pSrc + 1);
				SAFE_MEMCPY(ptr + length * sizeof(double), length * sizeof(double), pSrc + 1 + length);
				memLoader.get()->UnlockMemMapPtr();
				memLoader.get()->CloseMem();
				break;
			case ANALOG_Z:
				_gParams.analogZSize = length;
				memLoader.get()->AllocateMem(_gParams, wstring(outPath + FNAME + L".raw").c_str(), acctssAttribute);
				ptr = memLoader.get()->GetMemMapPtr(ANALOG_Z, 0, _gParams.analogZSize);
				SAFE_MEMCPY(ptr, length * sizeof(double), pSrc);
				memLoader.get()->UnlockMemMapPtr();
				memLoader.get()->CloseMem();
				break;
			case ANALOG_POCKEL:
			case DIGITAL_LINES:
			case SIGNALTYPE_LAST:
			default:
				break;
		}

	}
	else if (0 == saveType.compare("csv"))
	{
		//separate files for easy access
		std::vector<std::string> names;
		csvWriter* csv = NULL;
		switch (stype)
		{
			case ANALOG_XY:
				names.push_back(CLOCKR);
				csv = new csvWriter(WStringToString(outPath + FNAMEINFO + L".csv"));
				csv->write(names, pSrc, 1);

				names.clear();
				names.push_back(GALVOX);
				names.push_back(GALVOY);
				csv = new csvWriter(WStringToString(outPath + FNAMEXY + L".csv"));
				csv->write(names, pSrc + 1, length);
				break;
			case ANALOG_Z:
				names.push_back(PIEZOZ);
				csv = new csvWriter(WStringToString(outPath + FNAMEZ + L".csv"));
				csv->write(names, pSrc, length);
				break;
			case ANALOG_POCKEL:
			case DIGITAL_LINES:
			case SIGNALTYPE_LAST:
			default:
				break;
		}
		SAFE_DELETE_PTR(csv);
	}
	else if (0 == saveType.compare("h5") || 0 == saveType.compare("hdf5"))
	{
		unsigned long long totalLength = (SignalType::ANALOG_XY == stype) ? length * 2 : length;

		//open or create the same file if not exist, but use different data groups
		if (FileExists(WStringToString(outPath + FNAME + L".h5").c_str()))
		{
			h5Loader.get()->OpenFileIO(wstring(outPath + FNAME + L".h5").c_str(), H5FileType::READWRITE);
		}
		else
		{
			h5Loader.get()->CreateFileIO(wstring(outPath + FNAME + L".h5").c_str(), H5FileType::OVERWRITE, totalLength);
		}

		std::vector<std::string> dataset;
		const char** datachar = NULL;
		switch (stype)
		{
			case ANALOG_XY:
				dataset.push_back("/" + CLOCKR);
				datachar = ConvertStrVec(dataset);
				h5Loader.get()->CreateGroupDatasets(WAVEINFO.c_str(), datachar, static_cast<long>(dataset.size()), H5DataType::DATA_DOUBLE);
				h5Loader.get()->ExtendData(WAVEINFO.c_str(), string("/" + CLOCKR).c_str(), pSrc, H5DataType::DATA_DOUBLE, false, 1);
				FreeCharVec(datachar, dataset.size());

				dataset.clear();
				dataset.push_back("/" + GALVOX);
				dataset.push_back("/" + GALVOY);
				datachar = ConvertStrVec(dataset);
				h5Loader.get()->CreateGroupDatasets(WAVEXY.c_str(), datachar, static_cast<long>(dataset.size()), H5DataType::DATA_DOUBLE);
				h5Loader.get()->ExtendData(WAVEXY.c_str(), string("/" + GALVOX).c_str(), pSrc + 1, H5DataType::DATA_DOUBLE, false, static_cast<unsigned long>(length));
				h5Loader.get()->ExtendData(WAVEXY.c_str(), string("/" + GALVOY).c_str(), pSrc + 1 + length, H5DataType::DATA_DOUBLE, false, static_cast<unsigned long>(length));
				FreeCharVec(datachar, dataset.size());
				break;
			case ANALOG_Z:
				dataset.push_back("/" + PIEZOZ);
				datachar = ConvertStrVec(dataset);
				h5Loader.get()->CreateGroupDatasets(WAVEZ.c_str(), datachar, static_cast<long>(dataset.size()), H5DataType::DATA_DOUBLE);
				h5Loader.get()->ExtendData(WAVEZ.c_str(), string("/" + PIEZOZ).c_str(), pSrc, H5DataType::DATA_DOUBLE, false, static_cast<unsigned long>(length));
				FreeCharVec(datachar, dataset.size());
				break;
			case ANALOG_POCKEL:
			case DIGITAL_LINES:
			case SIGNALTYPE_LAST:
			default:
				break;
		}
		h5Loader.get()->CloseFileIO();
	}
	return TRUE;
}