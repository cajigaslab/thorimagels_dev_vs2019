#include "stdafx.h"
#include "ZPiezoSetupXML.h"

ZPiezoXML::ZPiezoXML()
{
	_xmlObj = NULL;
	_currentPathAndFile;
}

ZPiezoXML::~ZPiezoXML()
{
	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}
}

const char * const ZPiezoXML::CONVERSION = "Conversion";

const char * const ZPiezoXML::CONVERSION_ATTR[NUM_CONVERSION_ATTRIBUTES] = {"volts2mm","offsetmm","mm_min", "mm_max", "stairCaseDelayPercentage", "pockelsReferenceThreshold"};

long ZPiezoXML::GetConversion(double &volt2mm, double &mmoffset_0v, double &mm_min, double &mm_max, long &stairCaseDelayPercentage, double &pockelsRefThreshold)
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
				case 1: ss>>mmoffset_0v; break;
				case 2: ss>>mm_min; break;
				case 3: ss>>mm_max; break;
				case 4: ss>>stairCaseDelayPercentage; break;
				case 5: ss>>pockelsRefThreshold; break;
				}
			}
		}
	}
	return TRUE;
}

const char * const ZPiezoXML::DMA = "Dma";

const char * const ZPiezoXML::DMA_ATTR[NUM_DMA_ATTRIBUTES] = {"piezoActiveLoadMS", "preLoadCount"};

long ZPiezoXML::GetDMA(long &activeLoadMS, long &preLoadCount)
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
			ticpp::Iterator< ticpp::Element > child(configObj,DMA);

			for ( child = child.begin( configObj ); child != child.end(); child++)
			{
				for(long attCount=0; attCount<NUM_DMA_ATTRIBUTES; attCount++)
				{
					string str;
					child->GetAttribute(DMA_ATTR[attCount],&str);
					stringstream ss(str);
					switch(attCount)
					{
					case 0: ss>>activeLoadMS; break;
					case 1: ss>>preLoadCount; break;
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

const char * const ZPiezoXML::IO = "IO";

const char * const ZPiezoXML::IO_ATTR[NUM_IO_ATTRIBUTES] = {"piezoLine","analogLine","triggerLine","pockelsReferenceAnalogLineOut"};

long ZPiezoXML::GetIO(string &piezoLine, string &analogLine, string &triggerLine, string &pockelsReferenceAnalogLineOut)
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
					case 0: ss>>piezoLine; break;
					case 1: ss>>analogLine; break;
					case 2: ss>>triggerLine; break;
					case 3: ss>>pockelsReferenceAnalogLineOut; break;
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


long ZPiezoXML::OpenConfigFile()
{
	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}

	wsprintf(_currentPathAndFile,L"ThorZPiezoSettings.xml");

	wstring ws = _currentPathAndFile;
	string s = ConvertWStringToString(ws);

	_xmlObj = new ticpp::Document(s);
	_xmlObj->LoadFile();

	return TRUE;
}


long ZPiezoXML::SaveConfigFile()
{
	if(_xmlObj != NULL)
	{
		wsprintf(_currentPathAndFile,L"ThorZPiezoSettings.xml");
		wstring ws = _currentPathAndFile;
		string s = ConvertWStringToString(ws);

		_xmlObj->SaveFile(s);
		return TRUE;
	}
	return FALSE;
}