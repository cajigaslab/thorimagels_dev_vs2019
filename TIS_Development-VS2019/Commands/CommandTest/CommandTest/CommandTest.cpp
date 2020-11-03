#include "..\..\..\Tools\WinUnit\Include\WinUnit.h"
#include "..\..\..\Common\Command.h"

#include <stdlib.h>
#include <time.h>


namespace
{
	HANDLE hEvent = NULL;
	CommandDll * pCommand = NULL; 
	wchar_t commandPathandName[MAX_PATH] = _T("");
}


FIXTURE(CommandTestFixture);


SETUP(CommandTestFixture)
{
	WIN_ASSERT_TRUE(WinUnit::Environment::GetVariable(_T("CommandPathandName"),commandPathandName,ARRAYSIZE(commandPathandName)), _T("Environment variable CommandPathandName was not set.  Use --CommandPathandName option."));

	pCommand = new CommandDll(commandPathandName);
	
	WIN_ASSERT_TRUE(pCommand != NULL);

	WIN_ASSERT_TRUE(pCommand->Initialize(0)==TRUE);

	
}

TEARDOWN(CommandTestFixture)
{
	pCommand->Uninitialize();
	delete pCommand;
	pCommand = NULL;
}

#define	DBL_EPSILON 1e-10
#define	DOUBLE_EQ(x,v) (((v - DBL_EPSILON) < x) && (x <( v + DBL_EPSILON)))

//Read the command GUID
BEGIN_TESTF(GetGUID,CommandTestFixture)
{
	GUID guid;
	WIN_ASSERT_TRUE(pCommand->GetCommandGUID(&guid)==TRUE);
	WIN_TRACE("{%08x-%04x-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X}\n",guid.Data1, guid.Data2, guid.Data3,guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
}
END_TESTF


//Check the reading and writing of parameters for the command
BEGIN_TESTF(GetSetParamTest,CommandTestFixture)
{
	long paramID;
	long paramType;
	long paramAvailable;
	long paramReadOnly;
	double paramMin;
	double paramMax;
	double paramDefault;

	for(paramID=ICommand::PARAM_FIRST_PARAM; paramID < ICommand::PARAM_LAST_PARAM; paramID++)
	{
		WIN_ASSERT_TRUE(pCommand->GetParamInfo(paramID, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault)==TRUE);

		if(TRUE == paramAvailable)
		{
			double val;

			//if the parameter is read only make sure the get param function works
			if(TRUE == paramReadOnly)
			{
				WIN_ASSERT_TRUE(pCommand->GetParam(paramID,val)==TRUE,_T("paramID %d readonly value %f"),paramID,val);
			}
			else
			{
				//test cases for parameter values

				//min
				WIN_ASSERT_TRUE(pCommand->SetParam(paramID,paramMin)==TRUE);
				WIN_ASSERT_TRUE(pCommand->GetParam(paramID,val)==TRUE);
				WIN_ASSERT_TRUE(DOUBLE_EQ(paramMin,val)==TRUE);

				//max
				WIN_ASSERT_TRUE(pCommand->SetParam(paramID,paramMax)==TRUE);
				WIN_ASSERT_TRUE(pCommand->GetParam(paramID,val)==TRUE);
				WIN_ASSERT_TRUE(DOUBLE_EQ(paramMax,val)==TRUE);

				//default
				WIN_ASSERT_TRUE(pCommand->SetParam(paramID,paramDefault)==TRUE);
				WIN_ASSERT_TRUE(pCommand->GetParam(paramID,val)==TRUE);
				WIN_ASSERT_TRUE(DOUBLE_EQ(paramDefault,val)==TRUE);

				//if min max are not equal send a random value in between
				if(DOUBLE_EQ(paramMin,paramMax)==FALSE)
				{
					double newVal;
					double randDouble;

					srand(static_cast<unsigned int>(time(NULL)));

					randDouble = (   static_cast<double>(rand()) / (static_cast<double>(RAND_MAX)+static_cast<double>(1)) ); 

					newVal = randDouble * (paramMax - paramMin) + paramMin;

					//if long data type cast before setting the parameter
					if(paramType == ICommand::TYPE_LONG)
					{
						newVal = static_cast<long>(newVal);
					}

					//random value
					WIN_ASSERT_TRUE(pCommand->SetParam(paramID,newVal)==TRUE,_T("paramID %d newVal %f val %f"),paramID,newVal,val);
					WIN_ASSERT_TRUE(pCommand->GetParam(paramID,val)==TRUE,_T("paramID %d newVal %f val %f"),paramID,newVal,val);
					WIN_ASSERT_TRUE(DOUBLE_EQ(newVal,val)==TRUE,_T("paramID %d newVal %f val %f"),paramID,newVal,val);

				}

				//below min
				WIN_ASSERT_TRUE(pCommand->SetParam(paramID,paramMin - 1.0)==FALSE);
				WIN_ASSERT_TRUE(pCommand->GetParam(paramID,val)==TRUE);
				WIN_ASSERT_TRUE(DOUBLE_EQ(paramMin-1,val)==FALSE);

				//above max
				WIN_ASSERT_TRUE(pCommand->SetParam(paramID,paramMax + 1.0)==FALSE);
				WIN_ASSERT_TRUE(pCommand->GetParam(paramID,val)==TRUE);
				WIN_ASSERT_TRUE(DOUBLE_EQ(paramMax+1,val)==FALSE);		
			}
		}
		else
		{			
			//if the parameters is unavailable make sure the get/set fail
			double val;

			WIN_ASSERT_TRUE(pCommand->SetParam(paramID,paramDefault)==FALSE);
			WIN_ASSERT_TRUE(pCommand->GetParam(paramID,val)==FALSE);
		}
	}

}
END_TESTF

BEGIN_TESTF(SetupTeardown,CommandTestFixture)
{
	//fail if the command has not been setup
	WIN_ASSERT_TRUE(pCommand->TeardownCommand()==FALSE);

	//pass case
	WIN_ASSERT_TRUE(pCommand->SetupCommand() == TRUE);	
	WIN_ASSERT_TRUE(pCommand->TeardownCommand() == TRUE);

	WIN_ASSERT_TRUE(pCommand->SetupCommand() == TRUE);
	//fail if the command is already setup
	WIN_ASSERT_TRUE(pCommand->SetupCommand() == FALSE);
	WIN_ASSERT_TRUE(pCommand->TeardownCommand()==TRUE);
	//fail if the command has already been tore down
	WIN_ASSERT_TRUE(pCommand->TeardownCommand()==FALSE);

}
END_TESTF


UINT StatusThreadProc( LPVOID pParam )
{
	long status = ICommand::STATUS_BUSY;

	while(status == ICommand::STATUS_BUSY)
	{
		if(FALSE == pCommand->Status(status))
		{
			break;
		}
	}

	SetEvent( hEvent );

	return 0;
}

//Execute the command
BEGIN_TESTF(Execute,CommandTestFixture)
{
	GUID guid;

	WIN_ASSERT_TRUE(pCommand->GetCommandGUID(&guid)==TRUE);
	
	//setup of the command
	WIN_ASSERT_TRUE(pCommand->SetupCommand()==TRUE);

	char * pMemoryBuffer = NULL;

	//allocate a storage buffer for the parameters
	pMemoryBuffer = new char[ICommand::MAXIMUM_PARAMS_BINARY_BUFFER];

	WIN_ASSERT_TRUE(pMemoryBuffer != NULL);

	WIN_ASSERT_TRUE(pCommand->GetCustomParamsBinary(pMemoryBuffer));

	WIN_ASSERT_TRUE(pCommand->SetCustomParamsBinary(pMemoryBuffer));
/*
	FILE *file;
	
	char mode[4];

	mode[0]   = 'w';
	mode[1] = '+';
	mode[2] = 'b';
	mode[3] = '\0';

	file = fopen(".\\testSaveParam.str",mode);
 //TO DO
	//WIN_ASSERT_TRUE(pCommand->SaveCustomParamsXML(&file) == TRUE);

//	WIN_ASSERT_TRUE(pCommand->LoadCustomParamXML(&file) == TRUE);

	fclose(file);
*/
	WIN_ASSERT_TRUE(pCommand->Execute() == TRUE);

	hEvent = CreateEvent(0, FALSE, FALSE, 0);

	DWORD dwThreadId;

	HANDLE hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatusThreadProc, 0, 0, &dwThreadId );

	DWORD dwWait = WaitForMultipleObjects( 1, &hEvent, TRUE, INFINITE );

	WIN_ASSERT_TRUE(dwWait == WAIT_OBJECT_0);

	CloseHandle(hThread);

	//teardown of the command
	WIN_ASSERT_TRUE(pCommand->TeardownCommand()==TRUE);
}
END_TESTF
