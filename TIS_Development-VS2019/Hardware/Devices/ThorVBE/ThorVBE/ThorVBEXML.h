#pragma once

class ThorVBEXML
{
private:
	ticpp::Document * _xmlObj;
	long SetAttribute(string tagName, string attribute, string attributeValue);
	long OpenConfigFile();
	long SaveConfigFile();	
public:
	 enum {NUM_CONNECTION_ATTRIBUTES = 3};
	 static const char * const CONNECTION_ATTR[NUM_CONNECTION_ATTRIBUTES];
	 
	 enum {NUM_CONFIGURATION_ATTRIBUTES = 4};
	 static const char * const CONFIGURATION_ATTR[NUM_CONFIGURATION_ATTRIBUTES];
	 static const char * const CONFIGURATION;

	ThorVBEXML();
	~ThorVBEXML();

	//long SetPortId
	long GetConnection(const string signature, long &portID,long &baudRate, string &serialNumber);
	long SetPortID(const string signature, const long portID);

	long GetConfiguration(long &magMin, long &magMax, long &wavelengthMin, long &wavelengthMax);	
};