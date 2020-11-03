#include "..\..\..\Tools\WinUnit\Include\WinUnit.h"
#include "..\..\..\Common\Config.h"
#include "..\..\..\Tools\ticpp\ticpp.h"
#include "..\..\..\Tools\ticpp\tinyxml.h"
#include "..\..\..\Tools\ticpp\ticpprc.h"
#include "..\..\..\Common\Experiment.h"
#include "..\ExperimentManager\ExperimentManager.h"

#include <stdlib.h>
#include <time.h>
#include <sstream>

namespace
{
	ConfigDll* pConfig = NULL;
	wchar_t ConfigPathandName[MAX_PATH] = _T("");
}

FIXTURE(ConfigurationTestFixture);


SETUP(ConfigurationTestFixture)
{	
}


TEARDOWN(ConfigurationTestFixture)
{
}

BEGIN_TESTF(CreateExperimentTest,ConfigurationTestFixture)
{
	IExperiment * experiment=NULL;

	stringstream ss;
	wstringstream wss;

	wss << "ExperimentManagerTest.xml";

	experiment = ExperimentManager::getInstance()->CreateExperiment(wss.str());

	WIN_ASSERT_TRUE(experiment != NULL);

	string str;
	WIN_ASSERT_TRUE(experiment->GetName(str) == TRUE);
	WIN_ASSERT_STRING_EQUAL("Test",str.c_str());

	str = "ExpName";
	WIN_ASSERT_TRUE(experiment->SetName(str) == TRUE);
	WIN_ASSERT_TRUE(experiment->GetName(str) == TRUE);
	WIN_ASSERT_STRING_EQUAL("ExpName",str.c_str());


	WIN_ASSERT_TRUE(experiment->GetDate(str) == TRUE);
	WIN_ASSERT_STRING_EQUAL("7/8/2009",str.c_str());

	time_t rawtime; 
	struct tm * timeinfo;

	time(&rawtime);
	
#pragma warning( push )
#pragma warning( disable : 4996 )
	timeinfo = localtime(&rawtime);
#pragma warning( pop )

	ss.str("");

	ss << (timeinfo->tm_mon+1);
	ss << "/";
	ss << timeinfo->tm_mday;
	ss << "/";
	ss << (1900 + timeinfo->tm_year);

	str = ss.str();
	string str2;
	WIN_ASSERT_TRUE(experiment->SetDate(str) == TRUE);
	WIN_ASSERT_TRUE(experiment->GetDate(str2) == TRUE);
	WIN_ASSERT_STRING_EQUAL(str.c_str(),str2.c_str());

	WIN_ASSERT_TRUE(experiment->GetUser(str) == TRUE);
	WIN_ASSERT_STRING_EQUAL("Admin",str.c_str());

	WIN_ASSERT_TRUE(experiment->SetUser("Guest") == TRUE);
	WIN_ASSERT_TRUE(experiment->GetUser(str) == TRUE);
	WIN_ASSERT_STRING_EQUAL("Guest",str.c_str());
	
	WIN_ASSERT_TRUE(experiment->GetComputer(str) == TRUE);
	WIN_ASSERT_STRING_EQUAL("Dell-7400",str.c_str());

	WIN_ASSERT_TRUE(experiment->SetComputer("Dell-E5500") == TRUE);
	WIN_ASSERT_TRUE(experiment->GetComputer(str) == TRUE);
	WIN_ASSERT_STRING_EQUAL("Dell-E5500",str.c_str());
	
	double version;

	WIN_ASSERT_TRUE(experiment->GetSoftware(version) == TRUE);
	WIN_ASSERT_TRUE((version > (1.0 - .001))&&(version < (1.0 + .001)));

	WIN_ASSERT_TRUE(experiment->SetSoftware(2.0) == TRUE);
	WIN_ASSERT_TRUE(experiment->GetSoftware(version) == TRUE);
	WIN_ASSERT_TRUE((version > (2.0 - .001))&&(version < (2.0 + .001)));

	string camName;
	long camImageWidth;
	long camImageHeight;
	double camPixelSize;
	double camExposureTimeMS;		
	long gain, blackLevel, lightMode;				
	long left,top,right,bottom;
	long binningX, binningY;
	long tapsIndex, tapsBalance;
	long readoutSpeedIndex;
	long camAverageMode, camAverageNum;
	long camVericalFlip, camHorizontalFlip, imageAngle;

	//getting the values from the experiment setup XML files
	WIN_ASSERT_TRUE(experiment->GetCamera(camName,camImageWidth,camImageHeight,camPixelSize,camExposureTimeMS,gain,blackLevel,lightMode,left,top,right,bottom,binningX,binningY,tapsIndex,tapsBalance,readoutSpeedIndex,camAverageMode,camAverageNum,camVericalFlip,camHorizontalFlip,imageAngle) == TRUE);
	WIN_ASSERT_STRING_EQUAL("Basler",str.c_str());
	WIN_ASSERT_EQUAL(camImageWidth,1280);
	WIN_ASSERT_EQUAL(camImageHeight,1024);
	WIN_ASSERT_TRUE((camPixelSize > (6.25 - .001))&&(camPixelSize < (6.25 + .001)));
	WIN_ASSERT_EQUAL(gain,1);
	WIN_ASSERT_EQUAL(lightMode,0);

	WIN_ASSERT_TRUE(experiment->SetCamera("Hamamatsu",640,480,12.5,4,2,1) == TRUE);
	WIN_ASSERT_TRUE(experiment->GetCamera(camName,camImageWidth,camImageHeight,camPixelSize,camExposureTimeMS,gain,blackLevel,lightMode,left,top,right,bottom,binningX,binningY,tapsIndex,tapsBalance,readoutSpeedIndex,camAverageMode,camAverageNum,camVericalFlip,camHorizontalFlip,imageAngle) == TRUE);
	WIN_ASSERT_STRING_EQUAL("Hamamatsu",str.c_str());
	WIN_ASSERT_EQUAL(camImageWidth,640);
	WIN_ASSERT_EQUAL(camImageHeight,480);
	WIN_ASSERT_TRUE((camPixelSize > (12.5 - .001))&&(camPixelSize < (12.5 + .001)));
	WIN_ASSERT_EQUAL(gain,2);
	WIN_ASSERT_EQUAL(lightMode,1);
	
	double mag;
	string objName;

	WIN_ASSERT_TRUE(experiment->GetMagnification(mag,objName) == TRUE);
	WIN_ASSERT_TRUE((mag > (20.0 - .001))&&(mag < (20.0 + .001)));

	WIN_ASSERT_TRUE(experiment->SetMagnification(10.0,objName) == TRUE);
	WIN_ASSERT_TRUE(experiment->GetMagnification(mag,objName) == TRUE);
	WIN_ASSERT_TRUE((mag > (10.0 - .001))&&(mag < (10.0 + .001)));

	WIN_ASSERT_TRUE(experiment->GetNumberOfWavelengths() == 2);
	
	double exposureTimeMS;

	WIN_ASSERT_TRUE(experiment->GetWavelength(0,str,exposureTimeMS) == TRUE);
	WIN_ASSERT_STRING_EQUAL("DAPI",str.c_str());
	WIN_ASSERT_TRUE((exposureTimeMS > (100.0 - .001))&&(exposureTimeMS < (100.0 + .001)));

	WIN_ASSERT_TRUE(experiment->GetWavelength(1,str,exposureTimeMS) == TRUE);
	WIN_ASSERT_STRING_EQUAL("Brightfield",str.c_str());
	WIN_ASSERT_TRUE((exposureTimeMS > (5.0 - .001))&&(exposureTimeMS < (5.0 + .001)));

	WIN_ASSERT_TRUE(experiment->SetWavelength(0,"FITC",50.0) == TRUE);
	WIN_ASSERT_TRUE(experiment->GetWavelength(0,str,exposureTimeMS) == TRUE);
	WIN_ASSERT_STRING_EQUAL("FITC",str.c_str());
	WIN_ASSERT_TRUE((exposureTimeMS > (50.0 - .001))&&(exposureTimeMS < (50.0 + .001)));
	
	WIN_ASSERT_TRUE(experiment->SetWavelength(1,"TxRed",75.0) == TRUE);
	WIN_ASSERT_TRUE(experiment->GetWavelength(1,str,exposureTimeMS) == TRUE);
	WIN_ASSERT_STRING_EQUAL("TxRed",str.c_str());
	WIN_ASSERT_TRUE((exposureTimeMS > (75.0 - .001))&&(exposureTimeMS < (75.0 + .001)));

	WIN_ASSERT_TRUE(experiment->AddWavelength("YFP",90.0) == TRUE);
	WIN_ASSERT_TRUE(experiment->GetNumberOfWavelengths() == 3);

	WIN_ASSERT_TRUE(experiment->RemoveWavelength("FITC")==TRUE);
	WIN_ASSERT_TRUE(experiment->GetNumberOfWavelengths() == 2);
	
	long steps,enable;
	double stepSize;
	double startPos;
	long zStreamFrames,zStreamMode;

	WIN_ASSERT_TRUE(experiment->GetZStage(str,enable,steps,stepSize,startPos,zStreamFrames,zStreamMode) == TRUE);
	WIN_ASSERT_STRING_EQUAL("Thorlabs",str.c_str());
	WIN_ASSERT_TRUE(steps == 1);
	WIN_ASSERT_TRUE((stepSize > (.1 - .001))&&(stepSize < (.1 + .001)));

	WIN_ASSERT_TRUE(experiment->SetZStage("Ludl",1,5,.5,1.0,0,0) == TRUE);
	WIN_ASSERT_TRUE(experiment->GetZStage(str,enable,steps,stepSize,startPos,zStreamFrames,zStreamMode) == TRUE);
	WIN_ASSERT_STRING_EQUAL("Ludl",str.c_str());
	WIN_ASSERT_TRUE(steps == 5);
	WIN_ASSERT_TRUE((stepSize > (.5 - .001))&&(stepSize < (.5 + .001)));
	WIN_ASSERT_TRUE((startPos > (1 - .001))&&(startPos < (1 + .001)));

	long timepoints;
	double intervalSec;
	long triggerModeTimelapse;

	WIN_ASSERT_TRUE(experiment->GetTimelapse(timepoints,intervalSec,triggerModeTimelapse) == TRUE);
	WIN_ASSERT_TRUE(timepoints == 1);
	WIN_ASSERT_TRUE((intervalSec > (0 - .001))&&(intervalSec < (0 + .001)));

	WIN_ASSERT_TRUE(experiment->SetTimelapse(10,5.0,0) == TRUE);
	WIN_ASSERT_TRUE(experiment->GetTimelapse(timepoints,intervalSec,triggerModeTimelapse) == TRUE);
	WIN_ASSERT_TRUE(timepoints == 10);
	WIN_ASSERT_TRUE((intervalSec > (5.0 - .001))&&(intervalSec < (5.0 + .001)));
	
	IExperiment::SampleType type = IExperiment::WELLTYPE_SLIDE;
	//double offsetX,offsetY,offsetZ;

	//WIN_ASSERT_TRUE(experiment->GetSample(offsetX,offsetY,offsetZ) == TRUE);
	//WIN_ASSERT_TRUE(type == IExperiment::WELLTYPE_96);
	//WIN_ASSERT_TRUE((offsetX > (5.0 - .001))&&(offsetX < (5.0 + .001)));
	//WIN_ASSERT_TRUE((offsetY > (5.0 - .001))&&(offsetY < (5.0 + .001)));

	//WIN_ASSERT_TRUE(experiment->SetSample(IExperiment::WELLTYPE_384,11.0,12.0) == TRUE);
	//WIN_ASSERT_TRUE(experiment->GetSample(offsetX,offsetY,offsetZ) == TRUE);
	//WIN_ASSERT_TRUE(type == IExperiment::WELLTYPE_384);
	//WIN_ASSERT_TRUE((offsetX > (11.0 - .001))&&(offsetX < (11.0 + .001)));
	//WIN_ASSERT_TRUE((offsetY > (12.0 - .001))&&(offsetY < (12.0 + .001)));
	
	long startRow,startColumn;
	long rows,columns;
	double wellOffsetXMM,wellOffsetYMM;

	WIN_ASSERT_TRUE(experiment->GetWells(startRow,startColumn,rows,columns,wellOffsetXMM,wellOffsetYMM) == TRUE);
	WIN_ASSERT_TRUE(startRow == 1);
	WIN_ASSERT_TRUE(startColumn == 1);
	WIN_ASSERT_TRUE(rows == 8);
	WIN_ASSERT_TRUE(columns == 12);
	WIN_ASSERT_TRUE((wellOffsetXMM > (9.0 - .001))&&(wellOffsetXMM < (9.0 + .001)));
	WIN_ASSERT_TRUE((wellOffsetYMM > (9.0 - .001))&&(wellOffsetYMM < (9.0 + .001)));

	WIN_ASSERT_TRUE(experiment->SetWells(4,5,13,19,15.0,16.0) == TRUE);
	WIN_ASSERT_TRUE(experiment->GetWells(startRow,startColumn,rows,columns,wellOffsetXMM,wellOffsetYMM) == TRUE);
	WIN_ASSERT_TRUE(startRow == 4);
	WIN_ASSERT_TRUE(startColumn == 5);
	WIN_ASSERT_TRUE(rows == 13);
	WIN_ASSERT_TRUE(columns == 19);
	WIN_ASSERT_TRUE((wellOffsetXMM > (15.0 - .001))&&(wellOffsetXMM < (15.0 + .001)));
	WIN_ASSERT_TRUE((wellOffsetYMM > (16.0 - .001))&&(wellOffsetYMM < (16.0 + .001)));
	
	long subRows=1,subColumns=1;
	double subOffsetXMM=0,subOffsetYMM=0;
	double transOffsetXMM=0,transOffsetYMM=0;

	//WIN_ASSERT_TRUE(experiment->GetSubImages(subRows,subColumns,subOffsetXMM, subOffsetYMM,transOffsetXMM,transOffsetYMM) == TRUE);
	WIN_ASSERT_TRUE(subRows == 1);
	WIN_ASSERT_TRUE(subColumns == 1);
	WIN_ASSERT_TRUE((subOffsetXMM > (0.0 - .001))&&(subOffsetXMM < (0.0 + .001)));
	WIN_ASSERT_TRUE((subOffsetYMM > (0.0 - .001))&&(subOffsetYMM < (0.0 + .001)));
	WIN_ASSERT_TRUE((transOffsetXMM > (0.0 - .001))&&(transOffsetXMM < (0.0 + .001)));
	WIN_ASSERT_TRUE((transOffsetYMM > (0.0 - .001))&&(transOffsetYMM < (0.0 + .001)));

	WIN_ASSERT_TRUE(experiment->SetSubImages(3,3,2.0,3.0,5.0,-4.0) == TRUE);
	//WIN_ASSERT_TRUE(experiment->GetSubImages(subRows,subColumns,subOffsetXMM, subOffsetYMM,transOffsetXMM,transOffsetYMM) == TRUE);
	WIN_ASSERT_TRUE(subRows == 3);
	WIN_ASSERT_TRUE(subColumns == 3);
	WIN_ASSERT_TRUE((subOffsetXMM > (2.0 - .001))&&(subOffsetXMM < (2.0 + .001)));
	WIN_ASSERT_TRUE((subOffsetYMM > (3.0 - .001))&&(subOffsetYMM < (3.0 + .001)));
	WIN_ASSERT_TRUE((transOffsetXMM > (5.0 - .001))&&(transOffsetXMM < (5.0 + .001)));
	WIN_ASSERT_TRUE((transOffsetYMM > (-4.0 - .001))&&(transOffsetYMM < (-4.0 + .001)));
	
	WIN_ASSERT_TRUE(experiment->GetComments(str) == TRUE);
	WIN_ASSERT_STRING_EQUAL("",str.c_str());

	WIN_ASSERT_TRUE(experiment->SetComments("A Test Comment") == TRUE);
	WIN_ASSERT_TRUE(experiment->GetComments(str) == TRUE);
	WIN_ASSERT_STRING_EQUAL("A Test Comment",str.c_str());
	
	experiment->Update();

	WIN_ASSERT_TRUE(ExperimentManager::getInstance()->SetActiveExperiment(wss.str()) == TRUE);

	experiment = ExperimentManager::getInstance()->GetActiveExperiment();

	WIN_ASSERT_TRUE(experiment != NULL);

	startRow = 0;
	startColumn = 0;
	rows = 0;
	columns = 0;
	wellOffsetXMM = 0;
	wellOffsetYMM = 0;
	WIN_ASSERT_TRUE(experiment->GetWells(startRow,startColumn,rows,columns,wellOffsetXMM,wellOffsetYMM) == TRUE);
	WIN_ASSERT_TRUE(startRow == 4);
	WIN_ASSERT_TRUE(startColumn == 5);
	WIN_ASSERT_TRUE(rows == 13);
	WIN_ASSERT_TRUE(columns == 19);
	WIN_ASSERT_TRUE((wellOffsetXMM > (15.0 - .001))&&(wellOffsetXMM < (15.0 + .001)));
	WIN_ASSERT_TRUE((wellOffsetYMM > (16.0 - .001))&&(wellOffsetYMM < (16.0 + .001)));
}
END_TESTF



