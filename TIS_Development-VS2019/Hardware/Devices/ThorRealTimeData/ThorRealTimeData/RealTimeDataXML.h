#pragma once
#include "AcquireData.h"

class XMLFile
{
public:
	ticpp::Document * _xmlObj;
	wchar_t _currentPathAndFile[_MAX_PATH];	

	XMLFile(){ _xmlObj = NULL; };
	~XMLFile(){ SAFE_DELETE_ARRAY(_xmlObj); };

	void GetPathFilename(wchar_t* pathfile);
	void SetPathFilename(const wchar_t* pathfile);

};

class RealTimeDataXML : public XMLFile
{
private:
	ticpp::Element * _boardElement;
	ticpp::Element * _spectralDomainElement;

	ticpp::Element * RealTimeDataXML::GetBoard();
	long PopulateVirtualChannels(ticpp::Iterator< ticpp::Element > itElement, std::vector<Channels>& channel);

public:
	static const char * const USERSETTINGS;
	static const char * const DAQDEVICE;
	static const char * const BOARDTYPE;
	enum {NUM_BOARDTYPE_ATTRIBUTES = 10};
	static const char * const BOARDTYPE_ATTR[NUM_BOARDTYPE_ATTRIBUTES];

	static const char * const CHANNELTYPE;
	enum {NUM_CHANNELTYPE_ATTRIBUTES = 7};
	static const char * const CHANNELTYPE_ATTR[NUM_CHANNELTYPE_ATTRIBUTES];
	static const char * const VIRTUALCHANNELTYPE;
	static const char * const SPECCHANNELTYPE;
	enum {NUM_VIRTUALCHANNELTYPE_ATTRIBUTES = 5};

	static const char * const SAMPLERATE;
	enum {NUM_SAMPLERATE_ATTRIBUTES = 3};
	static const char * const SAMPLERATE_ATTR[NUM_SAMPLERATE_ATTRIBUTES];

	static const char * const DISPLAY;
	enum {NUM_DISPLAY_ATTRIBUTES = 3};
	static const char * const DISPLAY_ATTR[NUM_DISPLAY_ATTRIBUTES];

	static const char * const BLEACHPARAM;
	enum {NUM_BLEACHPARAM_ATTRIBUTES = 12};
	static const char * const BLEACHPARAM_ATTR[NUM_BLEACHPARAM_ATTRIBUTES];

	static const char * const INVERTPARAM;
	enum {NUM_INVERTPARAM_ATTRIBUTES = 2};
	static const char * const INVERTPARAM_ATTR[NUM_INVERTPARAM_ATTRIBUTES];

	static const char * const SAVEPARAM;
	enum {NUM_SAVEPARAM_ATTRIBUTES = 1};
	static const char * const SAVEPARAM_ATTR[NUM_SAVEPARAM_ATTRIBUTES];

	static const char * const SPECTRALDOMAIN;
	enum {NUM_SPECTRALDOMAIN_ATTRIBUTES = 8};
	static const char * const SPECTRALDOMAIN_ATTR[NUM_SPECTRALDOMAIN_ATTRIBUTES];

	static const char * const VARIABLES;

	static const char * const VARIABLESETTINGS;
	enum {NUM_VARIABLE_ATTRIBUTES = 3};
	static const char * const VARIABLE_ATTR[NUM_VARIABLE_ATTRIBUTES];

	static const char * const FILEPATH;
	enum {NUM_FILEPATH_ATTRIBUTES = 2};
	static const char * const FILEPATH_ATTR[NUM_FILEPATH_ATTRIBUTES];

	RealTimeDataXML();
	RealTimeDataXML(wchar_t* pathfile);
	~RealTimeDataXML();

	long GetBoard(BoardInfo &board);
	long GetMode(Mode* mode);
	long GetDataChannel(std::vector<Channels>& channel);
	long GetVirtualChannel(std::vector<Channels>& channel);
	long GetAsyncMode(AsyncParams* aparam);
	long GetInvert(Invert* aparam);
	long GetSave(double &callbackTime);
	long GetSpectralDomain(Spectrum &sDomain);
	long GetSpecChannel(std::vector<Channels>& channel);
	long GetSpecVirtualChannel(std::vector<Channels>& channel);
	long GetVariables(std::map<int, GlobalVar>& vars);
	void SetVariables(std::map<int, GlobalVar>& vars);
	long GetFilePath(FilePathParam* filepath);

	long OpenConfigFile(bool reload = false);
	long SaveConfigFile();

};

class OTMDataXML : public XMLFile
{
private:
	//save current settings to program base as well if TRUE
	void SaveConfigFile(long saveGlobal);

public:
	static const char * const PARAMETERSETTINGS;
	enum {NUM_PARAMETER_ATTRIBUTES = 11};
	static const char * const PARAMETER_ATTR[NUM_PARAMETER_ATTRIBUTES];

	static const char * const FITTINGSETTINGS;
	enum {NUM_FITTING_ATTRIBUTES = 16};
	static const char * const FITTING_ATTR[NUM_FITTING_ATTRIBUTES];

	OTMDataXML();
	OTMDataXML(const wchar_t* pathfile);
	~OTMDataXML();

	long GetParameter(OTMParam* otmParam);
	void SetParameter(OTMParam* otmParam);

	long GetFittings(OTMFit* otmFit);
	void SetFittings(OTMFit* otmFit);

	long OpenConfigFile(bool reload = false);

};
