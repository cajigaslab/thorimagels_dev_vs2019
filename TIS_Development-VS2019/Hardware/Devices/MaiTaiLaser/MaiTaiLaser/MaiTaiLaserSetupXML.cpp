#include "stdafx.h"
#include "MaiTaiLaserSetupXML.h"

#ifdef LOGGING_ENABLED
    extern auto_ptr<LogDll> logDll;
#endif

CMaiTaiLaserSetupXML::CMaiTaiLaserSetupXML ( void )
{
    _xmlObj = NULL;
    _currentPathAndFile[0] = NULL;
}


CMaiTaiLaserSetupXML::~CMaiTaiLaserSetupXML ( void )
{
    if ( _xmlObj != NULL )
    {
        delete _xmlObj;
    }
}

const char* const CMaiTaiLaserSetupXML::CONNECTION = "Connection";

const char* const CMaiTaiLaserSetupXML::CONNECTION_ATTR[NUM_CONNECTION_ATTRIBUTES] = {"portID", "baudRate"};

long CMaiTaiLaserSetupXML::GetConnection ( long& portID, long& baudRate )
{
    OpenConfigFile();
    // make sure the top level root element exist
    ticpp::Element* configObj = _xmlObj->FirstChildElement ( false );

    if ( configObj == NULL )
    {
        return FALSE;
    }

    else
    {
        ticpp::Iterator<ticpp::Element> child ( configObj, CONNECTION );

        for ( child = child.begin ( configObj ); child != child.end(); child++ )
        {
            for ( long attCount = 0; attCount < NUM_CONNECTION_ATTRIBUTES; attCount++ )
            {
                string str;
                child->GetAttribute ( CONNECTION_ATTR[attCount], &str );
                stringstream ss ( str );

                switch ( attCount )
                {
                    case 0:
                    {
                        ss >> portID;
                    }
                    break;

                    case 1:
                    {
                        ss >> baudRate;
                    }
                    break;
                }
            }
        }
    }

    return TRUE;
}

long CMaiTaiLaserSetupXML::OpenConfigFile()
{
    wsprintf ( _currentPathAndFile, L"MaiTaiLaserSettings.xml" );
    string s = ConvertWStringToString(_currentPathAndFile);

    if ( _xmlObj != NULL )
    {
        return TRUE;
    }

    _xmlObj = new ticpp::Document ( s );
    _xmlObj->LoadFile();
    return TRUE;
}

long CMaiTaiLaserSetupXML::SaveConfigFile()
{
    if ( _xmlObj != NULL )
    {
        _xmlObj->SaveFile();
    }

    return TRUE;
}