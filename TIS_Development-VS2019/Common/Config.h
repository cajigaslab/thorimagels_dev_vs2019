#pragma once

//Config.h
#include "..\Common\PDLL\pdll.h"
//dll wrapper class using the virtual class
#include <string>

using namespace std;

class IConfig
{
public:

	virtual long SetPath(string path) = 0;
	virtual long Save() = 0;
	virtual long GetRoot(string &root) = 0;
	virtual long GetAttribute(string tagName, string attribute, string &attributeValue) = 0;
	virtual long SetAttribute(string tagName, string attribute, string attributeValue) = 0;
	virtual long CreateTag(string parentTag, string tag, string attribute,string attributeValue) = 0;
};

class ConfigDll : public PDLL, public IConfig
{
	//call the macro and pass your class name
	DECLARE_CLASS(ConfigDll)
	//use DECLARE_FUNCTION4 since this function has 4 parameters
	
	DECLARE_FUNCTION1(long, SetPath, string)
	DECLARE_FUNCTION0(long, Save)
	DECLARE_FUNCTION1(long, GetRoot, string&)
	DECLARE_FUNCTION3(long, GetAttribute, string, string, string&)
	DECLARE_FUNCTION3(long, SetAttribute, string, string, string)
	DECLARE_FUNCTION4(long, CreateTag, string, string, string, string)

};

