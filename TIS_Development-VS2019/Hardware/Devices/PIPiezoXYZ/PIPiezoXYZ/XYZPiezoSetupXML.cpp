#include "stdafx.h"
#include "XYZPiezoSetupXML.h"

XYZPiezoXML::XYZPiezoXML()
{
	_xmlObj = NULL;
	_currentPathAndFile[0] = NULL;
}

XYZPiezoXML::~XYZPiezoXML()
{
	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}
}

const char * const XYZPiezoXML::CONVERSION = "Conversion";

const char * const XYZPiezoXML::CONVERSION_ATTR[NUM_CONVERSION_ATTRIBUTES] = {"volts2mm","xPos_min","xPos_max","yPos_min","yPos_max","zPos_min","zPos_max"};

long XYZPiezoXML::GetConversion(double &volt2mm,  double &xPos_min, double &xPos_max, double &yPos_min, double &yPos_max, double &zPos_min,double &zPos_max)
{
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	if(configObj == NULL )
	{
		return FALSE;
	}
	else
	{
		ticpp::Iterator< ticpp::Element > child(configObj,CONVERSION);

		for ( child = child.begin( configObj ); child != child.end(); child++)
		{
			for(long attCount=0; attCount<NUM_CONVERSION_ATTRIBUTES; attCount++)
			{
				string str;
				child->GetAttribute(CONVERSION_ATTR[attCount],&str);	
				stringstream ss(str);
				switch(attCount)
				{
				case 0: ss>>volt2mm; break;
				case 1: ss>>xPos_min; break;
				case 2: ss>>xPos_max; break;
				case 3: ss>>yPos_min; break;
				case 4: ss>>yPos_max; break;
				case 5: ss>>zPos_min; break;
				case 6: ss>>zPos_max; break;

				}
			}
		}
	}
	return TRUE;
}


const char * const XYZPiezoXML::IO = "IO";

const char * const XYZPiezoXML::IO_ATTR[NUM_IO_ATTRIBUTES] = {"piezoXLine","XanalogLine","piezoYLine","YanalogLine","piezoZLine","ZanalogLine"};

long XYZPiezoXML::GetIO(string &piezoXLine, string &XanalogLine, string &piezoYLine, string &YanalogLine,string &piezoZLine, string &ZanalogLine)
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
					case 0: ss>>piezoXLine; break;
					case 1: ss>>XanalogLine; break;
					case 2: ss>>piezoYLine; break;
					case 3: ss>>YanalogLine; break;
					case 4: ss>>piezoZLine; break;
					case 5: ss>>ZanalogLine; break;
					}
				}
			}
		}
	}
	catch(...)
	{
		return FALSE;
	}
	return TRUE;
}


long XYZPiezoXML::OpenConfigFile()
{
	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}
	
	wsprintf(_currentPathAndFile,L"PIPiezoSettings.xml");
	
	string s = ConvertWStringToString(_currentPathAndFile);
	
	_xmlObj = new ticpp::Document(s);
	_xmlObj->LoadFile();
	
	return TRUE;
}


long XYZPiezoXML::SaveConfigFile()
{
	if(_xmlObj != NULL)
	{
		wsprintf(_currentPathAndFile,L"PIPiezoSettings.xml");
		string s = ConvertWStringToString(_currentPathAndFile);
		_xmlObj->SaveFile(s);
		return TRUE;
	}
	return FALSE;
}


