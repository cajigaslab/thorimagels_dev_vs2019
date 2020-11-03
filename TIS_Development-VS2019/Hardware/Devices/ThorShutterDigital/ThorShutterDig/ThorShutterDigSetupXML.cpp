#include "stdafx.h"
#include "ThorShutterDigSetupXML.h"


/// <summary>
/// Initializes a new instance of the <see cref="ThorShutterDigXML" /> class.
/// </summary>
ThorShutterDigXML::ThorShutterDigXML()
{
	_xmlObj = NULL;
	_currentPathAndFile;
}

/// <summary>
/// Finalizes an instance of the <see cref="ThorShutterDigXML" /> class.
/// </summary>
ThorShutterDigXML::~ThorShutterDigXML()
{
	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}
}


const char * const ThorShutterDigXML::IO = "IO";

const char * const ThorShutterDigXML::IO_ATTR[NUM_IO_ATTRIBUTES] = {"shutterLine","shutterDelayMS"};

/// <summary>
/// Gets the io.
/// </summary>
/// <param name="shutterLine">The shutter line.</param>
/// <param name="shutterDelayMS">The shutter delay ms.</param>
/// <returns>long.</returns>
long ThorShutterDigXML::GetIO(string &shutterLine, long &shutterDelayMS)
{
	try
	{
		ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

		if ( configObj == NULL )
		{
			return FALSE;
		}
		else
		{
			ticpp::Iterator< ticpp::Element > child(configObj,IO);

			for ( child = child.begin( configObj ); child != child.end(); child++)
			{
				for(long attCount=0; attCount<NUM_IO_ATTRIBUTES; attCount++)
				{
					string str;		
					child->GetAttribute(IO_ATTR[attCount],&str);	
					stringstream ss(str);
					switch(attCount)
					{
					case 0: shutterLine = str; break;
					case 1: ss>>shutterDelayMS; break;
					}
				}
			}
			if(shutterLine=="") return FALSE;
		}		
	}
	catch(...)
	{
		return FALSE;
	}
	  return TRUE;
}


/// <summary>
/// Opens the configuration file.
/// </summary>
/// <returns>long.</returns>
long ThorShutterDigXML::OpenConfigFile()
{	
	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}

	wsprintf(_currentPathAndFile,L"ThorShutterDigSettings.xml");		

	wstring ws = _currentPathAndFile;
	string s = ConvertWStringToString(ws);

	_xmlObj = new ticpp::Document(s);
	_xmlObj->LoadFile();
	
	return TRUE;
}


/// <summary>
/// Saves the configuration file.
/// </summary>
/// <returns>long.</returns>
long ThorShutterDigXML::SaveConfigFile()
{	
	if(_xmlObj != NULL)
	{
		wsprintf(_currentPathAndFile,L"ThorShutterDigSettings.xml");		
		wstring ws = _currentPathAndFile;
		string s = ConvertWStringToString(ws);
		_xmlObj->SaveFile(s);
		return TRUE;
	}
	return FALSE;
			
}
