#include "stdafx.h"
#include "ThorBmExpanSetupXML.h"


ThorBmExpanXML::ThorBmExpanXML()
{
	_xmlObj = NULL;
	_currentPathAndFile[0] = NULL;
}

ThorBmExpanXML::~ThorBmExpanXML()
{
	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}
}

const char * const ThorBmExpanXML::CONNECTION = "Connection";
const char * const ThorBmExpanXML::CONNECTION_ATTR[NUM_CONNECTION_ATTRIBUTES] = {"portID0","baudRate0","address0","portID1","baudRate1","address1"};
const char * const ThorBmExpanXML::EXP_RATIOS = "EXP_RATIOS";
const char * const ThorBmExpanXML::EXP_RATIOS_ATTR[NUM_EXP_POS]={"RATIO0", "RATIO1", "RATIO2", "RATIO3", "RATIO4", "RATIO5", "RATIO6"};
const char * const ThorBmExpanXML::MOT0_POS = "MOT0_POS";
const char * const ThorBmExpanXML::MOT0_POS_ATTR[NUM_EXP_POS]={"POS0","POS1","POS2","POS3","POS4","POS5","POS6"};
const char * const ThorBmExpanXML::MOT1_POS = "MOT1_POS";
const char * const ThorBmExpanXML::MOT1_POS_ATTR[NUM_EXP_POS]={"POS0","POS1","POS2","POS3","POS4","POS5","POS6"};


long ThorBmExpanXML::GetConnection(long &portID0,long &portID1, long &baudRate0, long &baudRate1, long &address0, long &address1)
{	
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

      if ( configObj == NULL )
	  {
		  return FALSE;
	  }
	  else
	  {
		  ticpp::Iterator< ticpp::Element > child(configObj,CONNECTION);
		  for ( child = child.begin( configObj ); child != child.end(); child++)
		  {
			  for(long attCount = 0; attCount<NUM_CONNECTION_ATTRIBUTES; attCount++)
			  {
				  string str;
				  child->GetAttribute(CONNECTION_ATTR[attCount],&str);
				  stringstream ss(str);
				  switch(attCount)
				  {
				  case 0: ss>>portID0;		break;
				  case 1: ss>>baudRate0;	break;
				  case 2: ss>>address0;		break;
				  case 3: ss>>portID1;		break;
				  case 4: ss>>baudRate1;	break;
				  case 5: ss>>address1;		break;
				  }
			  }
		  }
	  }	
	  return TRUE;

}
long ThorBmExpanXML::GetExpRatios(long &Ratio0, long &Ratio1, long &Ratio2, long &Ratio3, long &Ratio4, long &Ratio5, long &Ratio6)
{
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);
    if ( configObj == NULL )
	{
	  return FALSE;
	}
	else
	{
		ticpp::Iterator< ticpp::Element > child(configObj,EXP_RATIOS);
		for ( child = child.begin( configObj ); child != child.end(); child++)
		{
			for(long attCount = 0; attCount<NUM_EXP_POS; attCount++)
			{
				string str;
				child->GetAttribute(EXP_RATIOS_ATTR[attCount],&str);
				stringstream ss(str);
				switch(attCount)
				{
				case 0: ss>>Ratio0; break;
				case 1: ss>>Ratio1; break;
				case 2: ss>>Ratio2; break;
				case 3: ss>>Ratio3; break;
				case 4: ss>>Ratio4; break;
				case 5: ss>>Ratio5; break;
				}
			}
		}
	}
	return TRUE;
}


long ThorBmExpanXML::GetMOT0_Pos(long &Pos0, long &Pos1, long &Pos2, long &Pos3, long &Pos4, long &Pos5, long &Pos6)
{
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);
    if ( configObj == NULL )
	{
	  return FALSE;
	}
	else
	{
		ticpp::Iterator< ticpp::Element > child(configObj,MOT0_POS);
		for ( child = child.begin( configObj ); child != child.end(); child++)
		{
			for(long attCount = 0; attCount<NUM_EXP_POS; attCount++)
			{
				string str;
				child->GetAttribute(MOT0_POS_ATTR[attCount],&str);
				stringstream ss(str);
				switch(attCount)
				{
				case 0: ss>>Pos0; break;
				case 1: ss>>Pos1; break;
				case 2: ss>>Pos2; break;
				case 3: ss>>Pos3; break;
				case 4: ss>>Pos4; break;
				case 5: ss>>Pos5; break;
				case 6: ss>>Pos6; break;


				}
			}
		}
	}
	return TRUE;
}

long ThorBmExpanXML::GetMOT1_Pos(long &Pos0, long &Pos1, long &Pos2, long &Pos3, long &Pos4, long &Pos5, long &Pos6)
{
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);
    if ( configObj == NULL )
	{
	  return FALSE;
	}
	else
	{
		ticpp::Iterator< ticpp::Element > child(configObj,MOT1_POS);
		for ( child = child.begin( configObj ); child != child.end(); child++)
		{
			for(long attCount = 0; attCount<NUM_EXP_POS; attCount++)
			{
				string str;
				child->GetAttribute(MOT1_POS_ATTR[attCount],&str);
				stringstream ss(str);
				switch(attCount)
				{
				case 0: ss>>Pos0; break;
				case 1: ss>>Pos1; break;
				case 2: ss>>Pos2; break;
				case 3: ss>>Pos3; break;
				case 4: ss>>Pos4; break;
				case 5: ss>>Pos5; break;
				case 6: ss>>Pos6; break;
				}
			}
		}
	}
	return TRUE;
}

long ThorBmExpanXML::OpenConfigFile()
{	
	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}

	wsprintf(_currentPathAndFile,L"ThorBmExpanSettings.xml");		

	string s = ConvertWStringToString(_currentPathAndFile);

	_xmlObj = new ticpp::Document(s);
	_xmlObj->LoadFile();
	
	return TRUE;
}
