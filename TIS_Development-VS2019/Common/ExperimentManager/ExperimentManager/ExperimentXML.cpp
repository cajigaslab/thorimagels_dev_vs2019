#include "stdafx.h"
#include "ExperimentXML.h"

#define VARIABLE_NAME(Variable) (#Variable)

ExperimentXML::ExperimentXML()
{
}

ExperimentXML::~ExperimentXML()
{
}

const char * const ExperimentXML::NAME = "Name";

const char * const ExperimentXML::NAME_ATTR[NUM_NAME_ATTRIBUTES] = {"name"};

wstring ExperimentXML::GetPathAndName()
{
	return StringToWString(_currentPathAndFile);
}

long ExperimentXML::GetName(string &name)
{	
	return GetAttribute(NAME, NAME_ATTR[0], name);
}

long ExperimentXML::SetName(string name)
{
	return SetAttribute(NAME, NAME_ATTR[0], name);
}

const char * const ExperimentXML::DATE = "Date";

const char * const ExperimentXML::DATE_ATTR[NUM_DATE_ATTRIBUTES] = {"date"};

long ExperimentXML::GetDate(string &date)
{
	return GetAttribute(DATE, DATE_ATTR[0], date);
}

long ExperimentXML::SetDate(string date)
{
	return SetAttribute(DATE, DATE_ATTR[0], date);
}

const char * const ExperimentXML::USER = "User";

const char * const ExperimentXML::USER_ATTR[NUM_USER_ATTRIBUTES] = {"name"};

long ExperimentXML::GetUser(string &name)
{
	return GetAttribute(USER, USER_ATTR[0], name);
}

long ExperimentXML::SetUser(string name)
{
	return SetAttribute(USER, USER_ATTR[0], name);
}

const char * const ExperimentXML::COMPUTER = "Computer";

const char * const ExperimentXML::COMPUTER_ATTR[NUM_COMPUTER_ATTRIBUTES] = {"name"};

long ExperimentXML::GetComputer(string &name)
{
	return GetAttribute(COMPUTER, COMPUTER_ATTR[0], name);
}

long ExperimentXML::SetComputer(string name)
{
	return SetAttribute(COMPUTER, COMPUTER_ATTR[0], name);
}

const char * const ExperimentXML::SOFTWARE = "Software";

const char * const ExperimentXML::SOFTWARE_ATTR[NUM_SOFTWARE_ATTRIBUTES] = {"version"};

long ExperimentXML::GetSoftware(double &version)
{
	long ret;
	string str;

	ret = GetAttribute(SOFTWARE, SOFTWARE_ATTR[0], str);

	istringstream is(str);

	is >> version;

	return ret;
}

long ExperimentXML::SetSoftware(double version)
{
	ostringstream os;
	os << version;

	return SetAttribute(SOFTWARE, SOFTWARE_ATTR[0], os.str());
}

const char * const ExperimentXML::MODALITY = "Modality";

const char * const ExperimentXML::MODALITY_ATTR[NUM_MODALITY_ATTRIBUTES] = {"primaryDetectorType", "name", "detectorLSMType"};

long ExperimentXML::GetModality(long& cameraType,string &modality, long& lsmType)
{
	string str;
	cameraType = ICamera::CameraType::LSM;
	lsmType = ICamera::LSMType::LSMTYPE_LAST;
	modality = "";
	try
	{
		if(NULL == _xmlObj.get())
		{
			logDll->TLTraceEvent(VERBOSE_EVENT,1,L"ExperimentManger  GetModality _xmlObj is null");
			return FALSE;
		}
		// make sure the top level root element exist
		ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

		if (NULL == configObj)
			return FALSE;

		ticpp::Element* parent = configObj->FirstChildElement(MODALITY, false);
		if (NULL == parent)
			return FALSE;

		for (int i = 0; i < NUM_MODALITY_ATTRIBUTES; i++)
		{
			parent->GetAttribute(MODALITY_ATTR[i], &str);
			stringstream ss(str);
			switch(i)
			{
			case 0:
				ss>>cameraType;
				break;
			case 1:
				modality = str;
				break;
			case 2:
				ss>>lsmType;
			}
		}
	}
	catch(...)
	{
		return FALSE;
	}
	return TRUE;
}

const char * const ExperimentXML::CAMERA = "Camera";

const char * const ExperimentXML::CAMERA_ATTR[NUM_CAMERA_ATTRIBUTES] = {"name","width","height","pixelSizeUM","exposureTimeMS","gain","blackLevel","lightmode","left","top","right","bottom","binningX","binningY","readoutTapIndex","tapBalance","readoutSpeedIndex","averageMode","averageNum","verticalFlip","horizontalFlip","imageAngle"};

long ExperimentXML::GetImageArea(long cameraType,long lsmtype,long &pixelX,long &pixelY,double &pixelSizeUM)
{
	if(_xmlObj.get() == NULL)
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"ExperimentManger  GetImageArea _xmlObj is null");
		return FALSE;
	}
	// make sure the top level root element exist
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	if ( configObj == NULL )
	{
		return FALSE;
	}
	else
	{
		string str = "";
		switch(cameraType)
		{
		case ICamera::CCD:
			{
				ticpp::Element* parent = configObj->FirstChildElement(CAMERA, false);
				if (parent)
				{
					const long NUM_CAM_SETTINGS = 3;
					char *cameraImageArea_Attr[NUM_CAM_SETTINGS]={"width","height","pixelSizeUM"};
					for (int i = 0; i < NUM_CAM_SETTINGS; i++)
					{
						parent->GetAttribute(cameraImageArea_Attr[i], &str);
						stringstream ss(str);
						switch(i)
						{
						case 0:
							ss>>pixelX;
							break;
						case 1:
							ss>>pixelY;
							break;
						case 2:
							ss>>pixelSizeUM;
							break;
						}
					}
				}else
				{
					return FALSE;
				}
			}
			break;
		case ICamera::LSM:
			{
				//consider meso scan in RGG if present
				if (ICamera::LSMType::RESONANCE_GALVO_GALVO == lsmtype)
				{
					long ScanID = 1;	//MesoScan(1),MicroScan(2)
					ticpp::Element *templateLevel1 = configObj->FirstChildElement(TEMPLATE_NODES[0]);
					if (NULL == templateLevel1)
						return FALSE;

					ticpp::Iterator<ticpp::Element> child1(templateLevel1, TEMPLATE_NODES[1]);
					for ( child1 = child1.begin(templateLevel1); child1 != child1.end(); child1++ )
					{
						child1->GetAttribute(VARIABLE_NAME(ScanID), &ScanID);
						if (1 == ScanID)
						{
							child1->GetAttribute(VARIABLE_NAME(XPixelSize), &pixelSizeUM);
							ticpp::Element *templateLevel2 = child1->FirstChildElement(TEMPLATE_NODES[2]);
							ticpp::Iterator<ticpp::Element> child2(templateLevel2, TEMPLATE_NODES[3]);

							//get view mode and all scan area ids when area is enabled
							for ( child2 = child2.begin(templateLevel2); child2 != child2.end(); child2++ )
							{
								child2->GetAttribute(VARIABLE_NAME(SizeX), &pixelX);
								child2->GetAttribute(VARIABLE_NAME(SizeY), &pixelY);
							}
						}
					}
				}
				else
				{
					ticpp::Element* parent = configObj->FirstChildElement(LSM, false);
					if (parent)
					{
						const long NUM_LSM_SETTINGS = 3;
						char *lsmScanArea_Attr[NUM_LSM_SETTINGS]={"pixelX","pixelY","pixelSizeUM"};

						for (int i = 0; i < NUM_LSM_SETTINGS; i++)
						{
							parent->GetAttribute(lsmScanArea_Attr[i], &str);
							stringstream ss(str);
							switch(i)
							{
							case 0:
								ss>>pixelX;
								break;
							case 1:
								ss>>pixelY;
								break;
							case 2:
								ss>>pixelSizeUM;
								break;
							}
						}
					}else
					{
						return FALSE;
					}
				}
			}
			break;		
		}
	}
	return TRUE;
}

long ExperimentXML::GetCamera(string &name,long &width,long &height,double &pixelSizeUM,double &exposureTimeMS, long &gain, long &blackLevel, long &lightMode, long &left, long &top, long &right, long &bottom, long &binningX, long &binningY, long &tapsIndex, long &tapsBalance, long &readOutSpeedIndex, long &averageMode, long &averageNum, long &verticalFlip, long &horizontalFlip, long &imageAngle)
{
	string str;

	long index;

	for(index=0; index<NUM_CAMERA_ATTRIBUTES; index++)
	{
		if(!GetAttribute(CAMERA, CAMERA_ATTR[index], str))
		{
			continue;
		}

		stringstream ss(str);

		switch(index)
		{
		case 0:
			name = str;
			break;
		case 1:
			ss>>width;
			break;
		case 2:
			ss>>height;
			break;
		case 3:
			ss>>pixelSizeUM;
			break;
		case 4:
			ss>>exposureTimeMS;
			break;
		case 5:
			ss>>gain;
			break;
		case 6:
			ss>>blackLevel;
			break;
		case 7:
			ss>>lightMode;
			break;
		case 8:
			ss>>left;
			break;
		case 9:
			ss>>top;
			break;
		case 10:
			ss>>right;
			break;
		case 11:
			ss>>bottom;
			break;
		case 12:
			ss>>binningX;
			break;
		case 13:
			ss>>binningY;
			break;
		case 14:
			ss>>tapsIndex;
			break;
		case 15:
			ss>>tapsBalance;
			break;
		case 16:
			ss>>readOutSpeedIndex;
			break;
		case 17:
			ss>>averageMode;
			break;
		case 18:
			ss>>averageNum;
			break;
		case 19:
			ss>>verticalFlip;
			break;
		case 20:
			ss>>horizontalFlip;
			break;
		case 21:
			ss>>imageAngle;
			break;
		}
	}
	return TRUE;
}

long ExperimentXML::SetCamera(string name, long width, long height, double pixelSizeUM,long binning, long gain, long lightMode)
{
	string str;
	stringstream ss;

	ss << name;
	ss << endl;
	ss << width;
	ss << endl;
	ss << height;
	ss << endl;
	ss << pixelSizeUM;
	ss << endl;
	ss << binning;
	ss << endl;
	ss << gain;
	ss << endl;
	ss << lightMode;
	ss << endl;

	long index;

	for(index=0; index<NUM_CAMERA_ATTRIBUTES; index++)
	{
		getline(ss,str);
		if(!SetAttribute(CAMERA, CAMERA_ATTR[index], str))
		{
			return FALSE;
		}
	}

	return TRUE;
}


const char * const ExperimentXML::LSM = "LSM";

const char * const ExperimentXML::LSM_ATTR[NUM_LSM_ATTRIBUTES] = {"areaMode","areaAngle","scanMode","interleave","pixelX","pixelY","channel","fieldSize","offsetX","offsetY","averageMode",
	"averageNum","clockSource","inputRange1","inputRange2","twoWayAlignment","extClockRate","dwellTime","flybackCycles","inputRange3","inputRange4", "minimizeFlybackCycles", "polarity1", "polarity2", "polarity3", "polarity4", "verticalFlip", "horizontalFlip", "crsFrequencyHz", "timeBasedLineScan", "tbLineScanTimeMS", "ThreePhotonEnable", "NumberOfPlanes"};

long ExperimentXML::GetLSM(long& areaMode, double& areaAngle, long& scanMode, long& interleave, long& pixelX, long& pixelY, long& channel, long& fieldSize, long& offsetX, long& offsetY,
	long& averageMode, long& averageNum, long& clockSource, long& inputRange1, long& inputRange2, long& twoWayAlignment, long& extClockRate,
	double& dwellTime, long& flybackCycles, long& inputRange3, long& inputRange4, long& minimizeFlybackCycles, long& polarity1, long& polarity2, long& polarity3, long& polarity4,
	long& verticalFlip, long& horizontalFlip, double& crsFrequencyHz, long& timeBasedLineScan, long& timeBasedLSTimeMS, long& threePhotonEnable, long& numberOfPlanes)
{
	string str;

	long index;

	for (index = 0; index < NUM_LSM_ATTRIBUTES; index++)
	{
		if (!GetAttribute(LSM, LSM_ATTR[index], str))
		{
			//if the attribute is not found just continue to the next attribute
			continue;
		}

		stringstream ss(str);

		switch (index)
		{
		case 0:
			ss >> areaMode;
			break;
		case 1:
			ss >> areaAngle;
			break;
		case 2:
			ss >> scanMode;
			break;
		case 3:
			ss >> interleave;
			break;
		case 4:
			ss >> pixelX;
			break;
		case 5:
			ss >> pixelY;
			break;
		case 6:
			ss >> channel;
			break;
		case 7:
			ss >> fieldSize;
			break;
		case 8:
			ss >> offsetX;
			break;
		case 9:
			ss >> offsetY;
			break;
		case 10:
			ss >> averageMode;
			break;
		case 11:
			ss >> averageNum;
			break;
		case 12:
			ss >> clockSource;
			break;
		case 13:
			ss >> inputRange1;
			break;
		case 14:
			ss >> inputRange2;
			break;
		case 15:
			ss >> twoWayAlignment;
			break;
		case 16:
			ss >> extClockRate;
			break;
		case 17:
			ss >> dwellTime;
			break;
		case 18:
			ss >> flybackCycles;
			break;
		case 19:
			ss >> inputRange3;
			break;
		case 20:
			ss >> inputRange4;
			break;
		case 21:
			ss >> minimizeFlybackCycles;
			break;
		case 22:
			ss >> polarity1;
			break;
		case 23:
			ss >> polarity2;
			break;
		case 24:
			ss >> polarity3;
			break;
		case 25:
			ss >> polarity4;
			break;
		case 26:
			ss >> verticalFlip;
			break;
		case 27:
			ss >> horizontalFlip;
			break;
		case 28:
			ss >> crsFrequencyHz;
			break;
		case 29:
			ss >> timeBasedLineScan;
			break;
		case 30:
			ss >> timeBasedLSTimeMS;
			break;
		case 31:
			ss >> threePhotonEnable;
			break;
		case 32:
			ss >> numberOfPlanes;
			break;
		}
	}
	return TRUE;
}

long ExperimentXML::SetLSM(long areaMode, double areaAngle,long scanMode,long interleave,long pixelX,long pixelY,long channel, long fieldSize, long offsetX, long offsetY,
						   long averageMode, long averageNum, long clockSource, long inputRange1, long inputRange2, long twoWayAlignment, long extClockRate,double dwellTime, long flybackCycles, long inputRange3, long inputRange4, long minimizeFlybackCycles, long polarity1, long polarity2, long polarity3, long polarity4, long verticalFlip, long horizontalFlip)
{
	string str;
	stringstream ss;
	ss << areaMode;
	ss << endl;
	ss << areaAngle;
	ss << endl;
	ss << scanMode;
	ss << endl;
	ss << interleave;
	ss << endl;
	ss << pixelX;
	ss << endl;
	ss << pixelY;
	ss << endl;
	ss << channel;
	ss << endl;
	ss << fieldSize;
	ss << endl;
	ss << offsetX;
	ss << endl;
	ss << offsetY;
	ss << endl;
	ss << averageMode;
	ss << endl;
	ss << averageNum;
	ss << endl;
	ss << clockSource;
	ss << endl;
	ss << inputRange1;
	ss << endl;
	ss << inputRange2;
	ss << endl;	
	ss << twoWayAlignment;
	ss << endl;	
	ss << extClockRate;
	ss << endl;
	ss << dwellTime;
	ss << endl;
	ss << flybackCycles;
	ss << endl;
	ss << inputRange3;
	ss << endl;
	ss << inputRange4;
	ss << endl;
	ss << minimizeFlybackCycles;
	ss << endl;
	ss << polarity1;
	ss << endl;
	ss << polarity2;
	ss << endl;
	ss << polarity3;
	ss << endl;
	ss << polarity4;
	ss << endl;
	ss << verticalFlip;
	ss << endl;
	ss << horizontalFlip;
	ss << endl;

	long index;

	for(index=0; index<NUM_LSM_ATTRIBUTES; index++)
	{
		getline(ss,str);
		if(!SetAttribute(LSM, LSM_ATTR[index], str))
		{
			return FALSE;
		}
	}

	return TRUE;
}


const char * const ExperimentXML::RAW = "RawData";

const char * const ExperimentXML::RAW_ATTR[NUM_RAW_ATTRIBUTES] = {"onlyEnabledChannels"};

long ExperimentXML::GetRaw(long &onlyEnabledChannels)
{
	string str;

	long index;

	for(index=0; index<NUM_RAW_ATTRIBUTES; index++)
	{
		if(!GetAttribute(RAW, RAW_ATTR[index], str))
		{
			return FALSE;
		}

		stringstream ss(str);

		switch(index)
		{
		case 0:
			ss>>onlyEnabledChannels;
			break;
		}
	}
	return TRUE;
}

const char * const ExperimentXML::STREAMING = "Streaming";

const char * const ExperimentXML::STREAMING_ATTR[NUM_STREAMING_ATTRIBUTES] = {"enable","frames","rawData","triggerMode","displayImage","storageMode", "zFastEnable", "zFastMode", "flybackFrames", "flybackLines","flybackTimeAdjustMS","volumeTimeAdjustMS","stepTimeAdjustMS", "stimulusTriggering","dmaFrames","stimulusMaxFrames","previewIndex","useReferenceVoltageForFastZPockels", "displayRollingAveragePreview"};

long ExperimentXML::GetStreaming(long& enable, long& frames, long& rawData, long& triggerMode, long& displayImage, long& storageMode, long& zFastEnable, long& zFastMode, long& flybackFrames, long& flybackLines, double& flybackTimeAdjustMS, double& volumeTimeAdjustMS, double& stepTimeAdjustMS, long& previewIndex, long& stimulusTriggering, long& dmaFrames, long& stimulusMaxFrames, long& useReferenceVoltageForFastZPockels, long& displayRollingAveragePreview)
{
	string str;

	long index;

	for (index = 0; index < NUM_STREAMING_ATTRIBUTES; index++)
	{
		if (!GetAttribute(STREAMING, STREAMING_ATTR[index], str))
		{
			return FALSE;
		}

		stringstream ss(str);

		switch (index)
		{
		case 0:
			ss >> enable;
			break;
		case 1:
			ss >> frames;
			break;
		case 2:
			ss >> rawData;
			break;
		case 3:
			ss >> triggerMode;
			break;
		case 4:
			ss >> displayImage;
			break;
		case 5:
			ss >> storageMode;
			break;
		case 6:
			ss >> zFastEnable;
			break;
		case 7:
			ss >> zFastMode;
			break;
		case 8:
			ss >> flybackFrames;
			break;
		case 9:
			ss >> flybackLines;
			break;
		case 10:
			ss >> flybackTimeAdjustMS;
			break;
		case 11:
			ss >> volumeTimeAdjustMS;
			break;
		case 12:
			ss >> stepTimeAdjustMS;
			break;
		case 13:
			ss >> stimulusTriggering;
			break;
		case 14:
			ss >> dmaFrames;
			break;
		case 15:
			ss >> stimulusMaxFrames;
			break;
		case 16:
			ss >> previewIndex;
			break;
		case 17:
			ss >> useReferenceVoltageForFastZPockels;
			break;
		case 18:
			ss >> displayRollingAveragePreview;
			break;
		}
	}
	return TRUE;
}

long ExperimentXML::SetStreaming(long enable,long frames, long rawData, long triggerMode, long displayImage, long storageMode, long zFastEnable, long flybackFrames, long stimulusTriggering,long dmaFrames,long stimulusMaxFrames, long previewIndex, long useReferenceVoltageForFastZPockels)
{
	string str;
	stringstream ss;

	ss << enable;
	ss << endl;
	ss << frames;
	ss << endl;
	ss << rawData;
	ss << endl;
	ss << triggerMode;
	ss << endl;
	ss <<storageMode;
	ss <<endl;
	ss <<zFastEnable;
	ss <<endl;
	ss <<flybackFrames;
	ss <<endl;
	ss <<stimulusTriggering;
	ss <<endl;
	ss <<dmaFrames;
	ss <<endl;
	ss <<stimulusMaxFrames;
	ss <<endl;
	ss <<previewIndex;
	ss <<endl;
	ss <<useReferenceVoltageForFastZPockels;
	ss <<endl;

	long index;

	for(index=0; index<NUM_STREAMING_ATTRIBUTES; index++)
	{
		getline(ss,str);
		if(!SetAttribute(STREAMING, STREAMING_ATTR[index], str))
		{
			return FALSE;
		}
	}

	return TRUE;
}


const char * const ExperimentXML::PMT = "PMT";

const char * const ExperimentXML::PMT_ATTR[NUM_PMT_ATTRIBUTES] = {"enableA","gainA","bandwidthAHz","offsetAVolts","enableB","gainB","bandwidthBHz","offsetBVolts","enableC","gainC","bandwidthCHz","offsetCVolts","enableD","gainD","bandwidthDHz","offsetDVolts"};

long ExperimentXML::GetPMT(long &enableA, long &gainA, long &bandwidthA, double &offsetA, long &enableB, long &gainB, long &bandwidthB, double &offsetB, long &enableC, long &gainC, long &bandwidthC, double &offsetC, long &enableD, long &gainD, long &bandwidthD, double &offsetD)
{
	string str;

	long index;

	for(index=0; index<NUM_PMT_ATTRIBUTES; index++)
	{
		if(!GetAttribute(PMT, PMT_ATTR[index], str))
		{
			return FALSE;
		}

		stringstream ss(str);

		switch(index)
		{
		case 0:
			ss>>enableA;
			break;
		case 1:
			ss>>gainA;
			break;
		case 2:
			ss>>bandwidthA;
			break;
		case 3:
			ss>>offsetA;
			break;
		case 4:
			ss>>enableB;
			break;
		case 5:
			ss>>gainB;
			break;
		case 6:
			ss>>bandwidthB;
			break;
		case 7:
			ss>>offsetB;
			break;
		case 8:
			ss>>enableC;
			break;
		case 9:
			ss>>gainC;
			break;
		case 10:
			ss>>bandwidthC;
			break;
		case 11:
			ss>>offsetC;
			break;
		case 12:
			ss>>enableD;
			break;
		case 13:
			ss>>gainD;
			break;
		case 14:
			ss>>bandwidthD;
			break;
		case 15:
			ss>>offsetD;
			break;
		}
	}
	return TRUE;
}

long ExperimentXML::SetPMT(long enableA, long gainA, long bandwidthA, double offsetA, long enableB, long gainB, long bandwidthB, double offsetB, long enableC, long gainC, long bandwidthC, double offsetC, long enableD, long gainD, long bandwidthD, double offsetD)
{
	string str;
	stringstream ss;

	ss << enableA;
	ss << endl;
	ss << gainA;
	ss << endl;
	ss << bandwidthA;
	ss << endl;
	ss << offsetA;
	ss << endl;
	ss << enableB;
	ss << endl;
	ss << gainB;
	ss << endl;
	ss << bandwidthB;
	ss << endl;
	ss << offsetB;
	ss << endl;
	ss << enableC;
	ss << endl;
	ss << gainC;
	ss << endl;
	ss << bandwidthC;
	ss << endl;
	ss << offsetC;
	ss << endl;
	ss << enableD;
	ss << endl;
	ss << gainD;
	ss << endl;
	ss << bandwidthD;
	ss << endl;
	ss << offsetD;
	ss << endl;

	long index;

	for(index=0; index<NUM_PMT_ATTRIBUTES; index++)
	{
		getline(ss,str);
		if(!SetAttribute(PMT, PMT_ATTR[index], str))
		{
			return FALSE;
		}
	}

	return TRUE;
}


const char * const ExperimentXML::MCLS = "MCLS";

const char * const ExperimentXML::MCLS_ATTR[NUM_MCLS_ATTRIBUTES] = {"enable1","power1","enable2","power2","enable3","power3","enable4","power4"};

long ExperimentXML::GetMCLS(long &enable1,double &power1,long &enable2,double &power2,long &enable3,double &power3,long &enable4,double &power4)
{
	string str;

	long index;

	for(index=0; index<NUM_MCLS_ATTRIBUTES; index++)
	{
		if(!GetAttribute(MCLS, MCLS_ATTR[index], str))
		{
			return FALSE;
		}

		stringstream ss(str);

		switch(index)
		{
		case 0:	ss>>enable1;break;
		case 1: ss>>power1;break;
		case 2:	ss>>enable2;break;
		case 3: ss>>power2;break;
		case 4:	ss>>enable3;break;
		case 5: ss>>power3;break;
		case 6:	ss>>enable4;break;
		case 7: ss>>power4;break;
		}
	}
	return TRUE;
}

long ExperimentXML::SetMCLS(long enable1,double power1,long enable2,double power2,long enable3,double power3,long enable4,double power4)
{
	string str;
	stringstream ss;

	ss << enable1;
	ss << endl;
	ss << power1;
	ss << endl;
	ss << enable2;
	ss << endl;
	ss << power2;
	ss << endl;
	ss << enable3;
	ss << endl;
	ss << power3;
	ss << endl;
	ss << enable4;
	ss << endl;
	ss << power4;
	ss << endl;

	long index;

	for(index=0; index<NUM_MCLS_ATTRIBUTES; index++)
	{
		getline(ss,str);
		if(!SetAttribute(MCLS, MCLS_ATTR[index], str))
		{
			return FALSE;
		}
	}

	return TRUE;
}

const char * const ExperimentXML::MAGNIFICATION = "Magnification";

const char * const ExperimentXML::MAGNIFICATION_ATTR[NUM_MAGNIFICATION_ATTRIBUTES] = {"mag","name"};

long ExperimentXML::GetMagnification(double &mag,string &name)
{
	string str;

	long index;

	for(index=0; index<NUM_MAGNIFICATION_ATTRIBUTES; index++)
	{
		if(!GetAttribute(MAGNIFICATION, MAGNIFICATION_ATTR[index], str))
		{
			return FALSE;
		}

		stringstream ss(str);

		switch(index)
		{
		case 0:	ss>>mag;break;
		case 1: name = str;break;
		}
	}
	return TRUE;
}

long ExperimentXML::SetMagnification(double mag, string name)
{
	string str;
	stringstream ss;

	ss << mag;
	ss << endl;
	ss << name;
	ss << endl;

	long index;

	for(index=0; index<NUM_MAGNIFICATION_ATTRIBUTES; index++)
	{
		getline(ss,str);
		if(!SetAttribute(MAGNIFICATION, MAGNIFICATION_ATTR[index], str))
		{
			return FALSE;
		}
	}

	return TRUE;
}

const char * const ExperimentXML::PINHOLEWHEEL = "PinholeWheel";

const char * const ExperimentXML::PINHOLEWHEEL_ATTR[NUM_PINHOLEWHEEL_ATTRIBUTES] = {"position"};

long ExperimentXML::GetPinholeWheel(long &position)
{
	long ret;
	string str;
	stringstream ss;

	ret = GetAttribute(PINHOLEWHEEL, PINHOLEWHEEL_ATTR[0], str);

	ss << str;
	ss >> position;

	return ret;
}

long ExperimentXML::SetPinholeWheel(long position)
{
	long ret;

	string str;
	stringstream ss;

	ss << position;

	ret = SetAttribute(PINHOLEWHEEL, PINHOLEWHEEL_ATTR[0], ss.str());

	return ret;
}

const char * const ExperimentXML::WAVELENGTHS = "Wavelengths";
const char * const ExperimentXML::WAVELENGTH = "Wavelength";
const char * const ExperimentXML::WAVELENGTH_ATTR[NUM_WAVELENGTH_ATTRIBUTES] = {"name","exposureTimeMS"};

long ExperimentXML::GetNumberOfWavelengths()
{
	long ret;
	string str;
	stringstream ss;
	long count;

	ret = GetTagCount(WAVELENGTHS, WAVELENGTH, count);

	if(TRUE == ret)
	{
		return count;
	}
	else
	{
		return 0;
	}
}

long ExperimentXML::GetChannelEnableSet()
{	
	long val = 0;
	if ( _xmlObj.get() == NULL )
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"ExperimentManger GetChannelEnableSet _xmlObj is null");
		return val;
	}

	// make sure the top level root element exist
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	if ( configObj == NULL )
	{
		return val;
	}
	else
	{
		ticpp::Element* parent = configObj->FirstChildElement(WAVELENGTHS);

		ticpp::Iterator< ticpp::Element > child(parent,"ChannelEnable");

		for ( child = child.begin( parent ); child != child.end(); child++)
		{
			string str;			  
			if(TRUE == GetAttributeWithChild(child,"ChannelEnable", "Set", str))
			{
				stringstream ss(str);
				ss>> val;
				return val;
			}
		}
	}
	return val;
}

long ExperimentXML::GetWavelength(long index,string &name,double &exposureTimeMS)
{	
	if ( _xmlObj.get() == NULL )
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"ExperimentManger  GetWavelength _xmlObj is null");
		return FALSE;
	}

	// make sure the top level root element exist
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	if ( configObj == NULL )
	{
		return FALSE;
	}
	else
	{
		ticpp::Element* parent = configObj->FirstChildElement(WAVELENGTHS);

		ticpp::Iterator< ticpp::Element > child(parent,WAVELENGTH);

		long count = 0;
		for ( child = child.begin( parent ); child != child.end(); child++ , count++)
		{
			if(count == index)
			{
				string str;			  

				long attCount;
				for(attCount = 0; attCount<NUM_WAVELENGTH_ATTRIBUTES; attCount++)
				{
					//child->GetAttribute(WAVELENGTH_ATTR[attCount],&str);
					GetAttributeWithChild(child,WAVELENGTH, WAVELENGTH_ATTR[attCount],str);
					stringstream ss(str);

					switch(attCount)
					{
					case 0:
						name = str;
						break;
					case 1:
						ss>>exposureTimeMS;
						break;				  
					}
				}

				return TRUE;
			}
		}
	}

	return FALSE;
}

long ExperimentXML::SetWavelength(long index,string name,double exposureTimeMS)
{
	if(_xmlObj.get() == NULL)
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"ExperimentManger  SetWavelength _xmlObj is null");
		return FALSE;
	}

	// make sure the top level root element exist
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	if ( configObj == NULL )
	{
		return FALSE;
	}
	else
	{
		ticpp::Element* parent = configObj->FirstChildElement(WAVELENGTHS);

		ticpp::Iterator< ticpp::Element > child(parent,WAVELENGTH);

		long count = 0;
		for ( child = child.begin( parent ); child != child.end(); child++ , count++)
		{
			if(count == index)
			{
				child->SetAttribute(WAVELENGTH_ATTR[0], name);
				child->SetAttribute(WAVELENGTH_ATTR[1], exposureTimeMS);
				return TRUE;
			}
		}
	}
	return FALSE;
}

long ExperimentXML::AddWavelength(string name,double exposureTimeMS)
{	
	if(_xmlObj.get() == NULL)
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"ExperimentManger  AddWavelength _xmlObj is null");
		return FALSE;
	}

	// make sure the top level root element exist
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	if ( configObj == NULL )
	{
		return FALSE;
	}
	else
	{
		ticpp::Element* parent = configObj->FirstChildElement(WAVELENGTHS);

		stringstream ss;

		ticpp::Element * element = new  ticpp::Element(WAVELENGTH);
		element->SetAttribute(WAVELENGTH_ATTR[0],name);
		ss << exposureTimeMS;
		element->SetAttribute(WAVELENGTH_ATTR[1],ss.str());
		parent->LinkEndChild(element);
	}

	return TRUE;
}

long ExperimentXML::RemoveWavelength(string name)
{
	if(_xmlObj.get() == NULL)
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"ExperimentManger  RemoveWavelength _xmlObj is null");
		return FALSE;
	}

	// make sure the top level root element exist
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	if ( configObj == NULL )
	{
		return FALSE;
	}
	else
	{
		ticpp::Element* parent = configObj->FirstChildElement(WAVELENGTHS);

		ticpp::Iterator< ticpp::Element > child(parent,WAVELENGTH);

		long count = 0;
		for ( child = child.begin( parent ); child != child.end(); child++ , count++)
		{
			string str;
			//child->GetAttribute(WAVELENGTH_ATTR[0],&str);
			GetAttributeWithChild(child, WAVELENGTH, WAVELENGTH_ATTR[0],str);
			if(str == name)
			{
				parent->RemoveChild(child.Get());

				return TRUE;
			}
		}
	}

	return FALSE;
}

long ExperimentXML::RemoveAllWavelengths()
{
	if(_xmlObj.get() == NULL)
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"ExperimentManger  RemoveWavelengths _xmlObj is null");
		return FALSE;
	}

	// make sure the top level root element exist
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	if ( configObj == NULL )
	{
		return FALSE;
	}
	else
	{
		ticpp::Element* parent = configObj->FirstChildElement(WAVELENGTHS);

		ticpp::Iterator< ticpp::Element > child(parent,WAVELENGTH);
		child->Clear();
		return TRUE;
		//long count = 0;
		//for ( child = child.begin( parent ); child != child.end(); child++ , count++)
		//{
		//	string str;
		//	//child->GetAttribute(WAVELENGTH_ATTR[0],&str);
		//	GetAttributeWithChild(child, WAVELENGTH, WAVELENGTH_ATTR[0],str);
		//	parent->RemoveChild(child.Get());
		//}
	}

	return FALSE;
}

const char * const ExperimentXML::ZSTAGE = "ZStage";

const char * const ExperimentXML::ZSTAGE_ATTR[NUM_ZSTAGE_ATTRIBUTES] = {"name","enable","steps","stepSizeUM","startPos","zStreamFrames","zStreamMode"};

long ExperimentXML::GetZStage(string &name, long &enable, long &steps, double &stepSize, double &startPos, long &zStreamFrames, long &zStreamMode )
{	
	string str;

	long index;

	for(index=0; index<NUM_ZSTAGE_ATTRIBUTES; index++)
	{
		if(!GetAttribute(ZSTAGE, ZSTAGE_ATTR[index], str))
		{
			return FALSE;
		}

		stringstream ss(str);

		switch(index)
		{
		case 0:
			name = str;
			break;		
		case 1:
			ss>>enable;
			break;
		case 2:
			ss>>steps;
			break;
		case 3:
			ss>>stepSize;
			break;
		case 4:
			ss>>startPos;
			break;
		case 5:
			ss>>zStreamFrames;
			break;
		case 6:
			ss>>zStreamMode;
			break;
		}
	}

	return TRUE;
}

long ExperimentXML::SetZStage(string name, long enable, long steps, double stepSize,double startPos, long zStreamFrames, long zStreamMode)
{
	string str;
	stringstream ss;

	ss << name;
	ss << endl;	
	ss << enable;
	ss << endl;	
	ss << steps;
	ss << endl;
	ss << stepSize;
	ss << endl;
	ss << startPos;
	ss << endl;
	ss << zStreamFrames;
	ss << endl;
	ss << zStreamMode;
	ss << endl;

	long index;

	for(index=0; index<NUM_ZSTAGE_ATTRIBUTES; index++)
	{
		getline(ss,str);
		if(!SetAttribute(ZSTAGE, ZSTAGE_ATTR[index], str))
		{
			return FALSE;
		}
	}

	return TRUE;
}

long ExperimentXML::GetZFileInfo(int& enable, double& scale)
{
	string str;

	if (!GetAttribute(ZSTAGE, "zFileEnable", str))
	{
		logDll->TLTraceEvent(VERBOSE_EVENT, 1, L"ExperimentManger  Could not find zFileEnable attribute");
		return FALSE;
	}
	enable = stoi(str);

	if (!GetAttribute(ZSTAGE, "zFilePosScale", str))
	{
		logDll->TLTraceEvent(VERBOSE_EVENT, 1, L"ExperimentManger  Could not find zFilePosScale attribute");
		return FALSE;
	}
	scale = stod(str);

	return TRUE;
}

long ExperimentXML::GetZ2LockInfo(int& lockEnable, int& mirrorEnable)
{
	string str;

	if (!GetAttribute(ZSTAGE, "z2StageLock", str))
	{
		logDll->TLTraceEvent(VERBOSE_EVENT, 1, L"ExperimentManger  Could not find z2StageLock attribute");
		return FALSE;
	}
	lockEnable = stoi(str);

	if (!GetAttribute(ZSTAGE, "z2StageMirror", str))
	{
		logDll->TLTraceEvent(VERBOSE_EVENT, 1, L"ExperimentManger  Could not find z2StageMirror attribute");
		return FALSE;
	}
	mirrorEnable = stoi(str);

	return TRUE;
}

const char * const ExperimentXML::TIMELAPSE = "Timelapse";

const char * const ExperimentXML::TIMELAPSE_ATTR[NUM_TIMELAPSE_ATTRIBUTES] = {"timepoints", "intervalSec", "triggerMode"};

long ExperimentXML::GetTimelapse(long &timepoints,double &intervalSec,long &triggerMode)
{	
	string str;

	long index;

	for(index=0; index<NUM_TIMELAPSE_ATTRIBUTES; index++)
	{
		if(!GetAttribute(TIMELAPSE, TIMELAPSE_ATTR[index], str))
		{
			return FALSE;
		}


		stringstream ss(str);

		switch(index)
		{
		case 0:
			ss>>timepoints;
			break;
		case 1:
			ss>>intervalSec;
			break;
		case 2:
			ss>>triggerMode;
			break;
		}
	}

	return TRUE;
}

long ExperimentXML::SetTimelapse(long timepoints, double intervalSec, long triggerMode)
{
	string str;
	stringstream ss;

	ss << timepoints;
	ss << endl;
	ss << intervalSec;
	ss << endl;
	ss << triggerMode;
	ss << endl;

	long index;

	for(index=0; index<NUM_TIMELAPSE_ATTRIBUTES; index++)
	{
		getline(ss,str);
		if(!SetAttribute(TIMELAPSE, TIMELAPSE_ATTR[index], str))
		{
			return FALSE;
		}
	}

	return TRUE;
}


const char * const ExperimentXML::TIMELAPSE_T_OFFSET_ATTR[NUM_TIMELAPSE_T_OFFSET_ATTRIBUTES] = {"tOffset"};

long ExperimentXML::GetTimelapseTOffset(long &t)
{	
	string str;

	long index;

	for(index=0; index<NUM_TIMELAPSE_T_OFFSET_ATTRIBUTES; index++)
	{
		if(!GetAttribute(TIMELAPSE, TIMELAPSE_T_OFFSET_ATTR[index], str))
		{
			return FALSE;
		}


		stringstream ss(str);

		switch(index)
		{
		case 0:
			ss>>t;
			break;
		}
	}

	return TRUE;
}

long ExperimentXML::SetTimelapseTOffset(long t)
{
	string str;
	stringstream ss;

	ss << t;
	ss << endl;

	long index;

	for(index=0; index<NUM_TIMELAPSE_T_OFFSET_ATTRIBUTES; index++)
	{
		getline(ss,str);
		if(!SetAttribute(TIMELAPSE, TIMELAPSE_T_OFFSET_ATTR[index], str))
		{
			return FALSE;
		}
	}

	return TRUE;
}


const char * const ExperimentXML::SAMPLE = "Sample";

const char * const ExperimentXML::SAMPLE_ATTR[NUM_SAMPLE_ATTRIBUTES] = {"homeOffsetX","homeOffsetY","homeOffsetZ","tiltAdjustment","fPt1XMM","fPt1YMM","fPt1ZMM","fPt2XMM","fPt2YMM","fPt2ZMM","fPt3XMM","fPt3YMM","fPt3ZMM"};

const char * const ExperimentXML::SAMPLEINFO_ATTR[NUM_SAMPLEINFO_ATTRIBUTES] = {"name","width","height","row","column","diameter","centerToCenterX","centerToCenterY","topLeftCenterOffsetX","topLeftCenterOffsetY","WellShape","WellWidth","WellHeight","initialStageLocationX", "initialStageLocationY" };

long ExperimentXML::GetSampleInfo(string &name, double &width, double &height, long &row, long &column, double &diameter, double &centerToCenterX, double &centerToCenterY, double &topLeftCenterOffsetX, double &topLeftCenterOffsetY, string &WellShape, double &WellWidth, double &WellHeight, double &initialStageLocationX, double &initialStageLocationY )
{
	string str;
	long index;

	for(index=0; index<NUM_SAMPLEINFO_ATTRIBUTES; index++)
	{
		if(!GetAttribute(SAMPLE, SAMPLEINFO_ATTR[index], str))
		{
			return FALSE;
		}
		stringstream ss(str);

		switch(index)
		{
		case 0:
			name = str;
			break;
		case 1:
			ss>>width;
			break;
		case 2:
			ss>>height;
			break;
		case 3:
			ss>>row;
			break;
		case 4:
			ss>>column;
			break;
		case 5:
			ss>>diameter;
			break;
		case 6:
			ss>>centerToCenterX;
			break;
		case 7:
			ss>>centerToCenterY;
			break;
		case 8:
			ss>>topLeftCenterOffsetX;
			break;
		case 9:
			ss>>topLeftCenterOffsetY;
			break;
		case 10:
			WellShape = str;
			break;
		case 11:
			ss>>WellWidth;
			break;
		case 12:
			ss>>WellHeight;
			break;
		case 13:
			ss>>initialStageLocationX;
			break;
		case 14:
			ss>>initialStageLocationY;
			break;
		}
	}
	return TRUE;
}

long ExperimentXML::GetSample(double &offsetX, double &offsetY, double &offsetZ, long &tiltAdjustment, double &fPt1X, double &fPt1Y, double &fPt1Z, double &fPt2X, double &fPt2Y, double &fPt2Z, double &fPt3X, double &fPt3Y, double &fPt3Z )
{
	string str;

	long index;

	for(index=0; index<NUM_SAMPLE_ATTRIBUTES; index++)
	{
		if(!GetAttribute(SAMPLE, SAMPLE_ATTR[index], str))
		{
			return FALSE;
		}
		stringstream ss(str);

		switch(index)
		{
		case 0:
			ss>>offsetX;
			break;
		case 1:
			ss>>offsetY;
			break;
		case 2:
			ss>>offsetZ;
			break;
		case 3:
			ss>>tiltAdjustment;
			break;
		case 4:
			ss>>fPt1X;
			break;
		case 5:
			ss>>fPt1Y;
			break;
		case 6:
			ss>>fPt1Z;
			break;
		case 7:
			ss>>fPt2X;
			break;
		case 8:
			ss>>fPt2Y;
			break;
		case 9:
			ss>>fPt2Z;
			break;
		case 10:
			ss>>fPt3X;
			break;
		case 11:
			ss>>fPt3Y;
			break;
		case 12:
			ss>>fPt3Z;
			break;
		}
	}

	return TRUE;
}

long ExperimentXML::SetSample(SampleType type, double offsetX, double offsetY, long tiltAdjustment, double fPt1X, double fPt1Y, double fPt1Z, double fPt2X, double fPt2Y, double fPt2Z, double fPt3X, double fPt3Y, double fPt3Z)
{
	string str;
	stringstream ss;

	ss << type;
	ss << endl;
	ss << offsetX;
	ss << endl;
	ss << offsetY;
	ss << endl;
	ss << tiltAdjustment;
	ss << endl;
	ss << fPt1X;
	ss << endl;
	ss << fPt1Y;
	ss << endl;
	ss << fPt1Z;
	ss << endl;
	ss << fPt2X;
	ss << endl;
	ss << fPt2Y;
	ss << endl;
	ss << fPt2Z;
	ss << endl;
	ss << fPt3X;
	ss << endl;
	ss << fPt3Y;
	ss << endl;
	ss << fPt3Z;
	ss << endl;

	long index;

	for(index=0; index<NUM_SAMPLE_ATTRIBUTES; index++)
	{
		getline(ss,str);
		if(!SetAttribute(SAMPLE, SAMPLE_ATTR[index], str))
		{
			return FALSE;
		}
	}

	return TRUE;

}


const char * const ExperimentXML::WELLS = "Wells";

const char * const ExperimentXML::WELLS_ATTR[NUM_WELLS_ATTRIBUTES] = {"startRow","startColumn","rows","columns","wellOffsetXMM","wellOffsetYMM"};

long ExperimentXML::GetWells(long &startRow, long &startColumn, long &rows, long &columns, double &wellOffsetXMM, double &wellOffsetYMM)
{
	if(_xmlObj.get() == NULL)
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"ExperimentManger  GetWells _xmlObj is null");
		return FALSE;
	}

	// make sure the top level root element exist
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	if ( configObj == NULL )
	{
		return FALSE;
	}
	else
	{
		ticpp::Element* parent = configObj->FirstChildElement(SAMPLE);
		ticpp::Element* element = parent->FirstChildElement(WELLS);

		string str;

		long index;
		for(index = 0; index<NUM_WELLS_ATTRIBUTES; index++)
		{
			try
			{
				element->GetAttribute(WELLS_ATTR[index],&str);	
				stringstream ss(str);

				switch(index)
				{
				case 0:
					ss>>startRow;
					break;
				case 1:
					ss>>startColumn;
					break;
				case 2:
					ss>>rows;
					break;
				case 3:
					ss>>columns;
					break;
				case 4:
					ss>>wellOffsetXMM;
					break;
				case 5:
					ss>>wellOffsetYMM;
					break;
				}
			}
			catch(ticpp::Exception ex)
			{
			}
		}
	}


	return TRUE;
}

long ExperimentXML::SetWells(long startRow, long startColumn, long rows, long columns, double wellOffsetXMM, double wellOffsetYMM)
{
	if(_xmlObj.get() == NULL)
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"ExperimentManger  SetWells _xmlObj is null");
		return FALSE;
	}
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	if ( configObj == NULL )
	{
		return FALSE;
	}
	else
	{
		string str;
		stringstream ss;

		ss << startRow;
		ss << endl;
		ss << startColumn;
		ss << endl;
		ss << rows;
		ss << endl;
		ss << columns;
		ss << endl;
		ss << wellOffsetXMM;
		ss << endl;
		ss << wellOffsetYMM;
		ss << endl;

		ticpp::Element* parent = configObj->FirstChildElement(SAMPLE);
		ticpp::Element* element = parent->FirstChildElement(WELLS);

		for(long index = 0; index< NUM_WELLS_ATTRIBUTES; index++)
		{
			getline(ss,str);

			element->SetAttribute(WELLS_ATTR[index], str);			  			  
		}
	}
	return TRUE;
}

const char * const ExperimentXML::SUBIMAGES = "SubImages";

const char * const ExperimentXML::SUBIMAGES_ATTR[NUM_SUBIMAGES_ATTRIBUTES] = {"isEnabled","subRows","subColumns","transOffsetXMM","transOffsetYMM","transOffsetZMM","overlapX","overlapY"};

long ExperimentXML::GetSubImages(vector<SubImage>& subImages, long cameraType, long lsmType)
{	
	if(_xmlObj.get() == NULL)
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"ExperimentManger  GetSubImages _xmlObj is null");
		return FALSE;
	}
	// make sure the top level root element exist
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	if ( configObj == NULL )
	{
		return FALSE;
	}
	else
	{
		double pixelSizeUM = 0;
		long scanAreaPixelX = 0, scanAreaPixelY = 0;
		if(FALSE == GetImageArea(cameraType, lsmType, scanAreaPixelX, scanAreaPixelY, pixelSizeUM))
			return FALSE;

		double scanAreaWidth = round(scanAreaPixelX * pixelSizeUM)/1000;
		double scanAreaHeight = round(scanAreaPixelY * pixelSizeUM)/1000;

		int wID = 0;
		ticpp::Element* parent = configObj->FirstChildElement(SAMPLE, false);
		if (parent)
		{
			for (ticpp::Element* well = parent->FirstChildElement(WELLS,false); well != 0; well = well->NextSiblingElement(WELLS,false))
			{
				wID++;
				for (ticpp::Element*  tile= well->FirstChildElement(SUBIMAGES,false); tile != 0; tile = tile->NextSiblingElement(SUBIMAGES,false))
				{
					SubImage subImage;
					subImage.wellID = wID;
					string str;
					long index;
					for(index=0; index<NUM_SUBIMAGES_ATTRIBUTES; index++)
					{
						try
						{
							tile->GetAttribute(SUBIMAGES_ATTR[index], &str);
							stringstream ss(str);

							switch(index)
							{
							case 0:
								subImage.isEnable = (str == "True") ? true : false;
								break;
							case 1:
								ss>>subImage.subRows;
								break;
							case 2:
								ss>>subImage.subColumns;
								break;
							case 3:
								ss>>subImage.transOffsetXMM;
								break;
							case 4:
								ss>>subImage.transOffsetYMM;
								break;
							case 5:
								ss>>subImage.transOffsetZMM;
								break;
							case 6:
								ss>>subImage.overlapX;
								break;
							case 7:
								ss>>subImage.overlapY;
								break;
							}
						}
						catch(ticpp::Exception ex)
						{
							char buf[512];
							WCHAR wbuf[512];
							const char* msg = ex.what();
							StringCbPrintfA(buf,512, "%s=> Tag: %s Attribute: %s", msg, SUBIMAGES, str.c_str());
							MultiByteToWideChar(0, 0, buf, -1, wbuf, 512);  
							logDll->TLTraceEvent(VERBOSE_EVENT, 1, wbuf);
						}
					}
					subImage.overlapX = subImage.overlapX/100;
					subImage.overlapY = subImage.overlapY/100;
					subImage.transOffsetXMM -=  (subImage.subColumns - (subImage.subColumns -1)*(subImage.overlapX) - 1) * scanAreaWidth / 2;
					subImage.transOffsetYMM +=  (subImage.subRows - (subImage.subRows - 1) * (subImage.overlapY) - 1) * scanAreaHeight / 2;
					subImage.scanAreaHeight = scanAreaHeight;
					subImage.scanAreaWidth = scanAreaWidth;
					if (subImage.isEnable == true)
					{
						subImages.push_back(subImage);
					}
				}
			}
		}
	}

	return TRUE;
}

long ExperimentXML::SetSubImages(long subRows,long subColumns, double subOffsetXMM, double subOffsetYMM, double transOffsetXMM, double transOffsetYMM)
{
	if(_xmlObj.get() == NULL)
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"ExperimentManger  SetSubImages _xmlObj is null");
		return FALSE;
	}
	// make sure the top level root element exist
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	if ( configObj == NULL )
	{
		return FALSE;
	}
	else
	{
		ticpp::Element* parent = configObj->FirstChildElement(SAMPLE);
		ticpp::Element* element = parent->FirstChildElement(WELLS);
		element = element->FirstChildElement(SUBIMAGES);

		string str;
		stringstream ss;

		ss << subRows;
		ss << endl;
		ss << subColumns;
		ss << endl;
		ss << subOffsetXMM;
		ss << endl;
		ss << subOffsetYMM;
		ss << endl;
		ss << transOffsetXMM;
		ss << endl;
		ss << transOffsetYMM;
		ss << endl;

		for(long index = 0; index< NUM_SUBIMAGES_ATTRIBUTES; index++)
		{
			getline(ss,str);

			element->SetAttribute(SUBIMAGES_ATTR[index], str);
		}
	}
	return TRUE;
}

const char * const ExperimentXML::COMMENTS = "Comments";

const char * const ExperimentXML::COMMENTS_ATTR[NUM_COMMENTS_ATTRIBUTES] = {"text"};

long ExperimentXML::GetComments(string &name)
{
	return GetAttribute(COMMENTS, COMMENTS_ATTR[0], name);
}

long ExperimentXML::SetComments(string name)
{
	return SetAttribute(COMMENTS, COMMENTS_ATTR[0], name);
}

const char * const ExperimentXML::AUTOFOCUS = "Autofocus";

const char * const ExperimentXML::AUTOFOCUS_ATTR[NUM_AUTOFOCUS_ATTRIBUTES] = {"type","repeat","exposureTimeMS","stepSizeUM","startPosMM","stopPosMM"};

long ExperimentXML::GetAutoFocus(long &type, long &repeat, double &expTimeMS, double &stepSizeUM, double &startPosMM, double &stopPosMM)
{
	string str;

	long index;

	for(index=0; index<NUM_AUTOFOCUS_ATTRIBUTES; index++)
	{
		if(!GetAttribute(AUTOFOCUS, AUTOFOCUS_ATTR[index], str))
		{
			return FALSE;
		}

		stringstream ss(str);

		switch(index)
		{
		case 0:
			ss>>type;
			break;
		case 1:
			ss>>repeat;
			break;
		case 2:
			ss>>expTimeMS;
			break;
		case 3:
			ss>>stepSizeUM;
			break;
		case 4:
			ss>>startPosMM;
			break;
		case 5:
			ss>>stopPosMM;
			break;
		}
	}


	return TRUE;
}

long ExperimentXML::SetAutoFocus(long type, long repeat, double expTimeMS, double stepSizeUM, double startPosMM, double stopPosMM)
{
	string str;
	stringstream ss;

	ss << type;
	ss << endl;
	ss << repeat;
	ss << endl;
	ss << expTimeMS;
	ss << endl;
	ss << stepSizeUM;
	ss << endl;
	ss << startPosMM;
	ss << endl;
	ss << stopPosMM;
	ss << endl;

	long index;

	for(index=0; index<NUM_AUTOFOCUS_ATTRIBUTES; index++)
	{
		getline(ss,str);
		if(!SetAttribute(AUTOFOCUS, AUTOFOCUS_ATTR[index], str))
		{
			return FALSE;
		}
	}

	return TRUE;
}

const char* const ExperimentXML::LAMP = "LAMP";

const char* const ExperimentXML::LAMP_ATTR[NUM_LAMP_ATTRIBUTES] = { "IsExternalTriggerTerm1","IsExternalTriggerTerm2","LampPosition1","LampPosition2","LampTerminal","led1enable","led1power","led2enable"
,"led2power","led3enable","led3power","led4enable","led4power","led5enable","led5power","led6enable","led6power","displayTemperatures","mainPower" };

long ExperimentXML::GetLampLED(long& led1enable, double& led1power, long& led2enable, double& led2power, long& led3enable, double& led3power, long& led4enable, double& led4power, long& led5enable, double& led5power, long& led6enable, double& led6power, double& mainPower)
{
	string str;

	long i;

	for (i = 0; i < NUM_LAMP_ATTRIBUTES; i++)
	{
		if (!GetAttribute(LAMP, LAMP_ATTR[i], str))
		{
			//if the attribute is not found just continue to the next attribute
			continue;
		}


		stringstream ss(str);

		switch (i)
		{
		case 5:
			ss >> led1enable;
			break;
		case 6:
			ss >> led1power;
			break;
		case 7:
			ss >> led2enable;
			break;
		case 8:
			ss >> led2power;
			break;
		case 9:
			ss >> led3enable;
			break;
		case 10:
			ss >> led3power;
			break;
		case 11:
			ss >> led4enable;
			break;
		case 12:
			ss >> led4power;
			break;
		case 13:
			ss >> led5enable;
			break;
		case 14:
			ss >> led5power;
			break;
		case 15:
			ss >> led6enable;
			break;
		case 16:
			ss >> led6power;
			break;
		case 18:
			ss >> mainPower;
			break;
		}
	}

	return TRUE;
}

const char * const ExperimentXML::POWER = "PowerRegulator";
const char * const ExperimentXML::POWER2 = "PowerRegulator2";

const char * const ExperimentXML::POWER_ATTR[NUM_POWER_ATTRIBUTES] = {"enable","type","start","stop","path","offset"};

long ExperimentXML::GetPower(long &enable, long &type, double &start, double &stop, string &path,double &zeroOffset, long index)
{
	string str;

	long i;

	for(i=0; i<NUM_POWER_ATTRIBUTES; i++)
	{
		switch(index)
		{
		case 0:
			{
				if(!GetAttribute(POWER, POWER_ATTR[i], str))
				{
					return FALSE;
				}
			}
			break;
		case 1:
			{
				if(!GetAttribute(POWER2, POWER_ATTR[i], str))
				{
					return FALSE;
				}
			}
			break;
		}

		stringstream ss(str);

		switch(i)
		{
		case 0:
			ss>>enable;
			break;
		case 1:
			ss>>type;
			break;
		case 2:
			ss>>start;
			break;
		case 3:
			ss>>stop;
			break;
		case 4:
			path = str;
			break;
		case 5:
			ss>>zeroOffset;
			break;
		}
	}


	return TRUE;
}

long ExperimentXML::SetPower(long enable, long type, double start, double stop, string path,double zeroOffset, long index)
{
	string str;
	stringstream ss;

	ss << enable;
	ss << endl;
	ss << type;
	ss << endl;
	ss << start;
	ss << endl;
	ss << stop;
	ss << endl;
	ss << path;
	ss << endl;
	ss << zeroOffset;
	ss << endl;

	long i;

	for(i=0; i<NUM_POWER_ATTRIBUTES; i++)
	{
		getline(ss,str);
		switch (index)
		{
		case 0:
			if(!SetAttribute(POWER, POWER_ATTR[i], str))
			{
				return FALSE;
			}
			break;
		case 1:
			if(!SetAttribute(POWER2, POWER_ATTR[i], str))
			{
				return FALSE;
			}
			break;
		}
	}

	return TRUE;
}


const char * const ExperimentXML::IMAGECORRECTION = "ImageCorrection";

const char * const ExperimentXML::IMAGECORRECTION_ATTR[NUM_IMAGECORRECTION_ATTRIBUTES] = {"enablePincushion","pinCoeff1","pinCoeff2","pinCoeff3","enableBackgroundSubtraction","pathBackgroundSubtraction","enableFlatField","pathFlatField"};

long ExperimentXML::GetImageCorrection(long &enablePincushion, double &pinCoeff1, double &pinCoeff2, double &pinCoeff3, long &enableBackgroundSubtration, string &pathBackgroundSubtration, long &enableFlatField, string &pathFlatField)
{
	string str;

	long index;

	for(index=0; index<NUM_IMAGECORRECTION_ATTRIBUTES; index++)
	{
		if(!GetAttribute(IMAGECORRECTION, IMAGECORRECTION_ATTR[index], str))
		{
			return FALSE;
		}

		stringstream ss(str);

		switch(index)
		{
		case 0:
			ss>>enablePincushion;
			break;
		case 1:
			ss>>pinCoeff1;
			break;
		case 2:
			ss>>pinCoeff2;
			break;
		case 3:
			ss>>pinCoeff3;
			break;
		case 4:
			ss>>enableBackgroundSubtration;
			break;
		case 5:
			pathBackgroundSubtration = str;
			break;
		case 6:
			ss>>enableFlatField;
			break;
		case 7:
			pathFlatField = str;
			break;

		}
	}


	return TRUE;
}

long ExperimentXML::SetImageCorrection(long enablePincushion, double pinCoeff1, double pinCoeff2, double pinCoeff3, long enableBackgroundSubtration, string pathBackgroundSubtration, long enableFlatField, string pathFlatField)
{
	string str;
	stringstream ss;

	ss << enablePincushion;
	ss << endl;
	ss << pinCoeff1;
	ss << endl;
	ss << pinCoeff2;
	ss << endl;
	ss << pinCoeff3;
	ss << endl;
	ss << enableBackgroundSubtration;
	ss << endl;
	ss << pathBackgroundSubtration;
	ss << endl;
	ss << enableFlatField;
	ss << endl;
	ss << pathFlatField;
	ss << endl;

	long index;

	for(index=0; index<NUM_IMAGECORRECTION_ATTRIBUTES; index++)
	{
		getline(ss,str);
		if(!SetAttribute(IMAGECORRECTION, IMAGECORRECTION_ATTR[index], str))
		{
			return FALSE;
		}
	}

	return TRUE;
}


const char * const ExperimentXML::MULTIPHOTONLASER = "MultiPhotonLaser";

const char * const ExperimentXML::MULTIPHOTONLASER_ATTR[NUM_MULTIPHOTONLASER_ATTRIBUTES] = {"enable","pos","seqEnable","seqPos1","seqPos2"};

long ExperimentXML::GetMultiPhotonLaser(long &enable, long &position, long &seqEnable, long &seqPos1, long &seqPos2)
{
	string str;

	long index;

	for(index=0; index<NUM_MULTIPHOTONLASER_ATTRIBUTES; index++)
	{
		try
		{
			if(!GetAttribute(MULTIPHOTONLASER, MULTIPHOTONLASER_ATTR[index], str))
			{
				return FALSE;
			}

			stringstream ss(str);

			switch(index)
			{
			case 0:
				ss>>enable;
				break;
			case 1:
				ss>>position;
				break;
			case 2:
				ss>>seqEnable;
				break;
			case 3:
				ss>>seqPos1;
				break;
			case 4:
				ss>>seqPos2;
				break;
			}
		}
		catch(ticpp::Exception ex)
		{
		}
	}


	return TRUE;
}

long ExperimentXML::SetMultiPhotonLaser(long enable, long position,long seqEnable,long seqPos1, long seqPos2)
{
	string str;
	stringstream ss;

	ss << enable;
	ss << endl;
	ss << position;
	ss << endl;
	ss << seqEnable;
	ss << endl;
	ss << seqPos1;
	ss << endl;
	ss << seqPos2;
	ss << endl;

	long index;

	for(index=0; index<NUM_MULTIPHOTONLASER_ATTRIBUTES; index++)
	{
		try
		{
			getline(ss,str);
			if(!SetAttribute(MULTIPHOTONLASER, MULTIPHOTONLASER_ATTR[index], str))
			{
				return FALSE;
			}
		}
		catch(ticpp::Exception ex)
		{
		}
	}

	return TRUE;
}


const char * const ExperimentXML::PHOTOBLEACHING = "Photobleaching";

const char * const ExperimentXML::PHOTOBLEACHING_ATTR[NUM_PHOTOBLEACHING_ATTRIBUTES] = {"enable",
	"laserPos",
	"durationMS",
	"powerPos",
	"width",
	"height",
	"offsetX",
	"offsetY",
	"bleachFrames",
	"fieldSize",
	"bleachTrigger",
	"preBleachFrames",
	"preBleachInterval",
	"preBleachStream",
	"postBleachFrames1",
	"postBleachInterval1",
	"postBleachStream1",
	"postBleachFrames2",
	"postBleachInterval2",
	"postBleachStream2",
	"powerEnable",
	"laserEnable",
	"bleachQuery",
	"bleachPostTrigger",
	"EnableSimultaneous",
	"pmt1EnableDuringBleach",
	"pmt2EnableDuringBleach",
	"pmt3EnableDuringBleach",
	"pmt4EnableDuringBleach"};

long ExperimentXML::GetPhotoBleachingAttr(char* attrName, double& val)
{
	long ret = FALSE;
	string str;
	if(GetAttribute(PHOTOBLEACHING, attrName, str))
	{
		stringstream ss(str);
		ss>>val;
		ret = TRUE;
	}
	return ret;
}

long ExperimentXML::GetPhotobleaching(long &enable, long &laserPosition, long &durationMS, double &powerPosition, long &width, long &height, long &offsetX, long &offsetY, long &bleachingFrames, long &fieldSize, long &bleachTrigger, long &preBleachingFrames, double &preBleachingInterval, long &preBleachingStream, long &postBleachingFrames1, double &postBleachingInterval1, long &postBleachingStream1, long &postBleachingFrames2, double &postBleachingInterval2, long &postBleachingStream2, long &powerEnable, long &laserEnable, long &bleachQuery,long &bleachPostTrigger, long &enableSimultaneous, long &pmt1EnableDuringBleach, long &pmt2EnableDuringBleach, long &pmt3EnableDuringBleach, long &pmt4EnableDuringBleach)
{
	string str;

	long index;

	for(index=0; index<NUM_PHOTOBLEACHING_ATTRIBUTES; index++)
	{
		if(!GetAttribute(PHOTOBLEACHING, PHOTOBLEACHING_ATTR[index], str))
		{
			continue;
		}

		stringstream ss(str);

		switch(index)
		{
		case 0:ss>>enable;break;
		case 1:ss>>laserPosition;break;
		case 2:ss>>durationMS;break;
		case 3:ss>>powerPosition;break;
		case 4:ss>>width;break;
		case 5:ss>>height;break;
		case 6:ss>>offsetX;break;
		case 7:ss>>offsetY;break;
		case 8:ss>>bleachingFrames;break;
		case 9:ss>>fieldSize;break;
		case 10:ss>>bleachTrigger;break;
		case 11:ss>>preBleachingFrames;break;
		case 12:ss>>preBleachingInterval;break;
		case 13:ss>>preBleachingStream;break;
		case 14:ss>>postBleachingFrames1;break;
		case 15:ss>>postBleachingInterval1;break;
		case 16:ss>>postBleachingStream1;break;
		case 17:ss>>postBleachingFrames2;break;
		case 18:ss>>postBleachingInterval2;break;
		case 19:ss>>postBleachingStream2;break;
		case 20:ss>>powerEnable;break;
		case 21:ss>>laserEnable;break;
		case 22:ss>>bleachQuery;break;
		case 23:ss>>bleachPostTrigger;break;
		case 24:ss>>enableSimultaneous;break;
		case 25:ss>>pmt1EnableDuringBleach;break;
		case 26:ss>>pmt2EnableDuringBleach;break;
		case 27:ss>>pmt3EnableDuringBleach;break;
		case 28:ss>>pmt4EnableDuringBleach;break;
		}
	}

	return TRUE;
}

long ExperimentXML::SetPhotobleaching(long enable, long laserPosition, long durationMS, double powerPosition, long width, long height, long offsetX, long offsetY, long bleachingFrames, long fieldSize, long bleachTrigger, long preBleachingFrames, double preBleachingInterval, long preBleachingStream, long postBleachingFrames1, double postBleachingInterval1, long postBleachingStream1, long postBleachingFrames2, double postBleachingInterval2, long postBleachingStream2, long powerEnable, long laserEnable, long bleachQuery,long bleachPostTrigger, long enableSimultaneous,long pmt1EnableDuringBleach, long pmt2EnableDuringBleach, long pmt3EnableDuringBleach, long pmt4EnableDuringBleach)
{
	string str;
	stringstream ss;

	ss << enable;
	ss << endl;
	ss << laserPosition;
	ss << endl;
	ss << durationMS;
	ss << endl;
	ss << powerPosition;
	ss << endl;
	ss << width;
	ss << endl;
	ss << height;
	ss << endl;
	ss << offsetX;
	ss << endl;
	ss << offsetY;
	ss << endl;
	ss << bleachingFrames;
	ss << endl;
	ss << fieldSize;
	ss << endl;
	ss << bleachTrigger;
	ss << endl;
	ss << preBleachingFrames;
	ss << endl;
	ss << preBleachingInterval;
	ss << endl;
	ss << postBleachingFrames1;
	ss << endl;
	ss << postBleachingInterval1;
	ss << endl;
	ss << postBleachingFrames2;
	ss << endl;
	ss << postBleachingInterval2;
	ss << endl;
	ss << powerEnable;
	ss << endl;
	ss << laserEnable;
	ss << endl;
	ss << bleachQuery;
	ss << endl;
	ss << bleachPostTrigger;
	ss << endl;
	ss << enableSimultaneous;
	ss << endl;
	ss << pmt1EnableDuringBleach;
	ss << endl;
	ss << pmt2EnableDuringBleach;
	ss << endl;
	ss << pmt3EnableDuringBleach;
	ss << endl;
	ss << pmt4EnableDuringBleach;
	ss << endl;

	long index;

	for(index=0; index<NUM_PHOTOBLEACHING_ATTRIBUTES; index++)
	{
		getline(ss,str);
		if(!SetAttribute(PHOTOBLEACHING, PHOTOBLEACHING_ATTR[index], str))
		{
			return FALSE;
		}
	}

	return TRUE;
}

const char * const ExperimentXML::CAPTUREMODE = "CaptureMode";

const char * const ExperimentXML::CAPTUREMODE_ATTR[NUM_CAPTUREMODE_ATTRIBUTES] = {"mode"};

long ExperimentXML::GetCaptureMode(long &mode)
{
	string str;

	long index;

	for(index=0; index<NUM_CAPTUREMODE_ATTRIBUTES; index++)
	{
		if(!GetAttribute(CAPTUREMODE, CAPTUREMODE_ATTR[index], str))
		{
			return FALSE;
		}

		stringstream ss(str);

		switch(index)
		{
		case 0:
			ss>>mode;
			break;
		}
	}


	return TRUE;
}

long ExperimentXML::SetCaptureMode(long mode)
{
	string str;
	stringstream ss;

	ss << mode;
	ss << endl;

	long index;

	for(index=0; index<NUM_CAPTUREMODE_ATTRIBUTES; index++)
	{
		getline(ss,str);
		if(!SetAttribute(CAPTUREMODE, CAPTUREMODE_ATTR[index], str))
		{
			return FALSE;
		}
	}

	return TRUE;
}


const char * const ExperimentXML::POCKELS = "Pockels";

const char * const ExperimentXML::POCKELS_ATTR[NUM_POCKELS_ATTRIBUTES] = {"type","start","stop","path","pockelsBlankPercentage"};

long ExperimentXML::GetPockels(long index, long &type, double &start, double &stop, string &path, long &maskPercent)
{

	if ( _xmlObj.get() == NULL )
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"ExperimentManger  GetPockels _xmlObj is null");
		return FALSE;
	}

	// make sure the top level root element exist
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	if ( configObj == NULL )
	{
		return FALSE;
	}
	else
	{
		ticpp::Iterator< ticpp::Element > child(configObj,POCKELS);

		long count = 0;
		for ( child = child.begin( configObj ); child != child.end(); child++ , count++)
		{
			if(count == index)
			{
				string str;			  

				long attCount;
				for(attCount = 0; attCount<NUM_POCKELS_ATTRIBUTES; attCount++)
				{
					try
					{
						child->GetAttribute(POCKELS_ATTR[attCount],&str);

						stringstream ss(str);

						switch(attCount)
						{
						case 0:
							ss>>type;
							break;
						case 1:
							ss>>start;
							break;
						case 2:
							ss>>stop;
							break;
						case 3:
							path = str;
							break;
						case 4:
							ss>>maskPercent;
							break;
						}
					}
					catch(ticpp::Exception ex)
					{
					}
				}

				return TRUE;
			}
		}
	}


	return TRUE;
}

long ExperimentXML::SetPockels(long index, long type, double start, double stop, string path, long maskPercent)
{
	if ( _xmlObj.get() == NULL )
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"ExperimentManger  SetPockels _xmlObj is null");
		return FALSE;
	}

	// make sure the top level root element exist
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	if ( configObj == NULL )
	{
		return FALSE;
	}
	else
	{
		ticpp::Iterator< ticpp::Element > child(configObj,POCKELS);

		long count = 0;
		for ( child = child.begin( configObj ); child != child.end(); child++ , count++)
		{
			if(count == index)
			{
				string str;
				stringstream ss;

				ss << type;
				ss << endl;
				ss << start;
				ss << endl;
				ss << stop;
				ss << endl;
				ss << path;
				ss << endl;
				ss << maskPercent;
				ss << endl;
				long index;

				for(index=0; index<NUM_POCKELS_ATTRIBUTES; index++)
				{
					getline(ss,str);
					child->SetAttribute(POCKELS_ATTR[index], str);				  
				}
				return TRUE;
			}
		}
	}

	return TRUE;
}

const char * const ExperimentXML::POCKELSMASK_ATTR[NUM_POCKELSMASK_ATTRIBUTES] = {"maskEnable","maskInvert","maskPath"};

long ExperimentXML::GetPockelsMask(long &enable, long &invert, string &path)
{

	if ( _xmlObj.get() == NULL )
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"ExperimentManger  GetPockelsMask _xmlObj is null");
		return FALSE;
	}

	// make sure the top level root element exist
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	if ( configObj == NULL )
	{
		return FALSE;
	}
	else
	{
		ticpp::Iterator< ticpp::Element > child(configObj,POCKELS);

		long count = 0;
		for ( child = child.begin( configObj ); child != child.end(); child++ , count++)
		{
			if(count == 0)
			{
				string str;			  

				long attCount;
				for(attCount = 0; attCount<NUM_POCKELSMASK_ATTRIBUTES; attCount++)
				{
					try
					{
						child->GetAttribute(POCKELSMASK_ATTR[attCount],&str);

						stringstream ss(str);

						switch(attCount)
						{
						case 0:
							ss>>enable;
							break;
						case 1:
							ss>>invert;
							break;
						case 2:
							path = str;
							break;
						}
					}
					catch(ticpp::Exception ex)
					{
					}
				}

				return TRUE;
			}
		}
	}

	return TRUE;
}

long ExperimentXML::SetPockelsMask(long enable, long invert, string path)
{
	if ( _xmlObj.get() == NULL )
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"ExperimentManger  SetPockels _xmlObj is null");
		return FALSE;
	}

	// make sure the top level root element exist
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	if ( configObj == NULL )
	{
		return FALSE;
	}
	else
	{
		ticpp::Iterator< ticpp::Element > child(configObj,POCKELS);

		long count = 0;
		for ( child = child.begin( configObj ); child != child.end(); child++ , count++)
		{
			if(count == 0)
			{
				string str;
				stringstream ss;

				ss << enable;
				ss << endl;
				ss << invert;
				ss << endl;
				ss << path;
				ss << endl;
				long index;

				for(index=0; index<NUM_POCKELSMASK_ATTRIBUTES; index++)
				{
					getline(ss,str);
					child->SetAttribute(POCKELSMASK_ATTR[index], str);				  
				}
				return TRUE;
			}
		}
	}

	return TRUE;
}


const char * const ExperimentXML::LIGHTPATH = "LightPath";

const char * const ExperimentXML::LIGHTPATH_ATTR[NUM_LIGHTPATH_ATTRIBUTES] = {"GalvoGalvo","GalvoResonance","Camera","InvertedLightPathPos","NDD"};

long ExperimentXML::GetLightPath(long &galvoGalvo, long &galvoRes, long &camera, long &invertedLightPathPos, long& ndd)
{
	string str;

	long index;

	for(index=0; index<NUM_LIGHTPATH_ATTRIBUTES; index++)
	{
		try
		{
			if(!GetAttribute(LIGHTPATH, LIGHTPATH_ATTR[index], str))
			{
				return FALSE;
			}

			stringstream ss(str);

			switch(index)
			{
			case 0:
				ss>>galvoGalvo;
				break;
			case 1:
				ss>>galvoRes;
				break;
			case 2:
				ss>>camera;
				break;
			case 3:
				ss>>invertedLightPathPos;
				break;
			case 4:
				ss >> ndd;
				break;
			}
		}
		catch(exception ex)
		{
			wchar_t errMsg[256];
			StringCbPrintfW(errMsg,256,L"ExperimentManager -> GetLightPath, could not read all elements from LightPath tag in Active.xml or LightPathList.xml.");
			logDll->TLTraceEvent(ERROR_EVENT, 1, errMsg);
			return FALSE;
		}
	}
	return TRUE;
}

long ExperimentXML::SetLightPath(long galvoGalvo, long galvoRes, long camera, long invertedLightPathPos, long ndd)
{
	string str;
	stringstream ss;

	ss << galvoGalvo;
	ss << endl;
	ss << galvoRes;
	ss << endl;
	ss << camera;
	ss << endl;
	ss << invertedLightPathPos;
	ss << endl;
	ss << ndd;
	ss << endl;

	long index;

	for(index=0; index<NUM_LIGHTPATH_ATTRIBUTES; index++)
	{
		getline(ss,str);
		if(!SetAttribute(LIGHTPATH, LIGHTPATH_ATTR[index], str))
		{
			return FALSE;
		}
	}

	return TRUE;

}


const char * const ExperimentXML::CAPTURESEQUENCE = "CaptureSequence";

const char * const ExperimentXML::CAPTURESEQUENCE_ATTR[NUM_CAPTURESEQUENCE_ATTRIBUTES] = {"enable"};

long ExperimentXML::GetCaptureSequence(long &enable)
{
	string str;

	long index;

	for(index=0; index<NUM_CAPTURESEQUENCE_ATTRIBUTES; index++)
	{
		if(!GetAttribute(CAPTURESEQUENCE, CAPTURESEQUENCE_ATTR[index], str))
		{
			return FALSE;
		}

		stringstream ss(str);

		switch(index)
		{
		case 0:
			ss>>enable;
			break;
		}
	}
	return TRUE;
}

const char * const ExperimentXML::SEQUENCESTEP = "LightPathSequenceStep";

const char * const ExperimentXML::SEQUENCESTEP_PMT_ATTR[NUM_SEQUENCESTEP_PMT_ATTRIBUTES] = {"enableA","gainA","enableB","gainB","enableC","gainC","enableD","gainD"};

const char * const ExperimentXML::SEQUENCESTEP_LSM_ATTR[NUM_SEQUENCESTEP_LSM_ATTRIBUTES] = {"channel"};

const char * const ExperimentXML::SEQUENCESTEP_MULTIPHOTON_ATTR[NUM_SEQUENCESTEP_MULTIPHOTON_ATTRIBUTES]  = {"pos"};

const char * const ExperimentXML::SEQUENCESTEP_MCLS_ATTR[NUM_SEQUENCESTEP_MCLS_ATTRIBUTES] = {"enable1","power1","enable2","power2","enable3","power3","enable4","power4"};

const char * const ExperimentXML::SEQUENCESTEP_PINHOLE_ATTR[NUM_SEQUENCESTEP_PINHOLE_ATTRIBUTES] = {"position"};

//const char * const ExperimentXML::SEQUENCESTEP_WAVELENGTHS = "Wavelengths";
const char * const ExperimentXML::SEQUENCESTEP_WAVELENGTH_ATTR[NUM_SEQUENCESTEP_WAVELENGTH_ATTRIBUTES] = {"name","exposureTimeMS"};

long ExperimentXML::GetSequenceSteps(vector<SequenceStep>& captureSequence)
{	
	if(_xmlObj.get() == NULL)
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"ExperimentManger  GetSequenceSteps _xmlObj is null");
		return FALSE;
	}
	// make sure the top level root element exist
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	if ( configObj == NULL )
	{
		return FALSE;
	}
	else
	{
		ticpp::Element* parent = configObj->FirstChildElement(CAPTURESEQUENCE, false);
		if (parent)
		{
			captureSequence.clear();
			for (ticpp::Element* step = parent->FirstChildElement(SEQUENCESTEP,false); step != 0; step = step->NextSiblingElement(SEQUENCESTEP,false))
			{
				SequenceStep sequenceStep;
				ticpp::Element*  pmt = step->FirstChildElement(PMT,false);
				string str;
				long index;

				//retrieve channel step PMT Settings
				for(index=0; index<NUM_SEQUENCESTEP_PMT_ATTRIBUTES; index++)
				{
					pmt->GetAttribute(SEQUENCESTEP_PMT_ATTR[index], &str);
					stringstream ss(str);

					switch(index)
					{
					case 0: ss>>sequenceStep.PMT1Enable; break;
					case 1: ss>>sequenceStep.PMT1Gain; break;
					case 2: ss>>sequenceStep.PMT2Enable; break;
					case 3: ss>>sequenceStep.PMT2Gain; break;
					case 4: ss>>sequenceStep.PMT3Enable; break;
					case 5: ss>>sequenceStep.PMT3Gain; break;
					case 6: ss>>sequenceStep.PMT4Enable; break;
					case 7: ss>>sequenceStep.PMT4Gain; break;
					}
				}

				//retrieve channel step Set MCLS Settings
				ticpp::Element*  mcls = step->FirstChildElement(MCLS,false);
				for(index=0; index<NUM_SEQUENCESTEP_MCLS_ATTRIBUTES; index++)
				{
					mcls->GetAttribute(SEQUENCESTEP_MCLS_ATTR[index], &str);
					stringstream ss(str);

					switch(index)
					{
					case 0:	ss>>sequenceStep.MCLSEnable1; break;
					case 1: ss>>sequenceStep.MCLSPower1; break;
					case 2:	ss>>sequenceStep.MCLSEnable2 ;break;
					case 3: ss>>sequenceStep.MCLSPower2; break;
					case 4:	ss>>sequenceStep.MCLSEnable3; break;
					case 5: ss>>sequenceStep.MCLSPower3; break;
					case 6:	ss>>sequenceStep.MCLSEnable4; break;
					case 7: ss>>sequenceStep.MCLSPower4; break;
					}
				}

				//retrieve channel step Multiphoton Settings
				ticpp::Element*  multiphton = step->FirstChildElement(MULTIPHOTONLASER,false);
				for(index=0; index<NUM_SEQUENCESTEP_MULTIPHOTON_ATTRIBUTES; index++)
				{
					multiphton->GetAttribute(SEQUENCESTEP_MULTIPHOTON_ATTR[index], &str);
					stringstream ss(str);

					switch(index)
					{
					case 0:	ss>>sequenceStep.MultiphotonPos; break;
					}
				}

				//retrieve channel step Pinhole Settings
				ticpp::Element*  pinhole = step->FirstChildElement(PINHOLEWHEEL,false);
				for(index=0; index<NUM_SEQUENCESTEP_PINHOLE_ATTRIBUTES; index++)
				{
					pinhole->GetAttribute(SEQUENCESTEP_PINHOLE_ATTR[index], &str);
					stringstream ss(str);

					switch(index)
					{
					case 0:	ss>>sequenceStep.PinholePos; break;
					}
				}

				//retrieve channel step Pinhole Settings
				ticpp::Element*  lsm = step->FirstChildElement(LSM,false);
				for(index=0; index<NUM_SEQUENCESTEP_LSM_ATTRIBUTES; index++)
				{
					lsm->GetAttribute(SEQUENCESTEP_LSM_ATTR[index], &str);
					stringstream ss(str);

					switch(index)
					{
					case 0:	ss>>sequenceStep.LSMChannel; break;
					}
				}

				ticpp::Element*  lightPath = step->FirstChildElement(LIGHTPATH,false);
				for(index=0; index<NUM_LIGHTPATH_ATTRIBUTES; index++)
				{
					try
					{
						lightPath->GetAttribute(LIGHTPATH_ATTR[index], &str);
						stringstream ss(str);

						switch(index)
						{
						case 0:
							ss>>sequenceStep.LightPathGGEnable;
							break;
						case 1:
							ss>>sequenceStep.LightPathGREnable;
							break;
						case 2:
							ss>>sequenceStep.LightPathCamEnable;
							break;
						case 3:
							ss>>sequenceStep.InvertedLightPathPosition;
							break;
						case 4:
							ss >> sequenceStep.LightPathNDDPosition;
							break;
						}
					}
					catch(exception ex)
					{
						logDll->TLTraceEvent(ERROR_EVENT,1,L"ExperimentManager could not read one of the LighPath tags from the sequence");
					}
				}

				//retrieve channel step Wavelength Settings
				ticpp::Element* waveLengthparent = step->FirstChildElement(WAVELENGTHS,false);
				ticpp::Iterator< ticpp::Element > wavelengthNode(waveLengthparent,WAVELENGTH);
				sequenceStep.Wavelength.clear();
				for ( wavelengthNode = wavelengthNode.begin( waveLengthparent ); wavelengthNode != wavelengthNode.end(); wavelengthNode++)
				{
					long attCount;
					Wavelength wavelength;
					for(attCount = 0; attCount<NUM_SEQUENCESTEP_WAVELENGTH_ATTRIBUTES; attCount++)
					{
						wavelengthNode->GetAttribute(SEQUENCESTEP_WAVELENGTH_ATTR[attCount],&str);
						stringstream ss(str);

						switch(attCount)
						{
						case 0:
							wavelength.name = str;
							break;
						case 1:
							ss>>wavelength.exposureTimeMS;
							break;
						}
					}
					sequenceStep.Wavelength.push_back(wavelength);
				}

				//retrieve channel step Pockels Settings
				ticpp::Iterator< ticpp::Element > pockelsNode(step,POCKELS);
				sequenceStep.Pockels.clear();
				for ( pockelsNode = pockelsNode.begin( step ); pockelsNode != pockelsNode.end(); pockelsNode++)
				{
					Pockels pockels;
					for(index = 0; index<NUM_POCKELS_ATTRIBUTES; index++)
					{
						pockelsNode->GetAttribute(POCKELS_ATTR[index],&str);

						stringstream ss(str);

						switch(index)
						{
						case 0:
							ss>>pockels.type;
							break;
						case 1:
							ss>>pockels.start;
							break;
						case 2:
							ss>>pockels.stop;
							break;
						case 3:
							pockels.path = str;
							break;
						}
					}
					sequenceStep.Pockels.push_back(pockels);
				}

				captureSequence.push_back(sequenceStep);
			}
		}
	}

	return TRUE;
}

const char * const ExperimentXML::SPECTRALFILTER = "SpectralFilter";

const char * const ExperimentXML::SPECTRALFILTER_ATTR[NUM_SPECTRALFILTER_ATTRIBUTES] = {"wavelengthStart", "wavelengthStop", "wavelengthStepSize", "bandwidthMode", "path"};

long ExperimentXML::GetSpectralFilter(long &startWavelength, long &stopWavelength, long &stepsSizeWavelength, long &bandwidthMode, string &path)
{
	for (long index=0; index<NUM_SPECTRALFILTER_ATTRIBUTES; index++)
	{
		string str;
		if (!GetAttribute(SPECTRALFILTER, SPECTRALFILTER_ATTR[index], str))
		{
			return FALSE;
		}

		stringstream ss(str);

		switch(index)
		{
		case 0:ss>>startWavelength;break;
		case 1:ss>>stopWavelength;break;
		case 2:ss>>stepsSizeWavelength;break;	
		case 3:ss>>bandwidthMode;break;
		case 4:path = str;break;
		}
	}

	return TRUE;
}

long ExperimentXML::SetSpectralFilter(long startWavelength, long stopWavelength, long stepsSizeWavelength, long bandwidthMode, string path)
{
	string str;
	stringstream ss;

	ss << startWavelength;
	ss << endl;
	ss << stopWavelength;
	ss << endl;
	ss << stepsSizeWavelength;
	ss << endl;
	ss << bandwidthMode;
	ss << endl;	
	ss << path;
	ss << endl;

	for (long index=0; index< NUM_SPECTRALFILTER_ATTRIBUTES; index++)
	{
		getline(ss,str);
		if (!SetAttribute(SPECTRALFILTER, SPECTRALFILTER_ATTR[index], str))
		{
			return FALSE;
		}
	}

	return TRUE;
}

const char* const ExperimentXML::EPITURRET = "EPITurret";

const char* const ExperimentXML::EPITURRET_ATTR[NUM_EPITURRET_ATTRIBUTES] = { "pos", "name" };

long ExperimentXML::GetEpiTurret(long& position, string& name)
{
	for (long index = 0; index < NUM_EPITURRET_ATTRIBUTES; index++)
	{
		string str;
		if (!GetAttribute(EPITURRET, EPITURRET_ATTR[index], str))
		{
			return FALSE;
		}

		stringstream ss(str);

		switch (index)
		{
		case 0:ss >> position; break;
		case 1:name = str; break;
		}
	}

	return TRUE;
}

long ExperimentXML::SetEpiTurret(long position, string name)
{
	string str;
	stringstream ss;

	ss << position;
	ss << endl;
	ss << name;
	ss << endl;

	for (long index = 0; index < NUM_EPITURRET_ATTRIBUTES; index++)
	{
		getline(ss, str);
		if (!SetAttribute(EPITURRET, EPITURRET_ATTR[index], str))
		{
			return FALSE;
		}
	}

	return TRUE;
}

const char * const ExperimentXML::TEMPLATE_NODES[NUM_TEMPLATE_LEVELS] = {"TemplateScans", "ScanInfo", "ScanAreas", "ScanArea"};

long ExperimentXML::GetSampleRegions(long &viewMode, vector<SampleRegion> &sregions)
{
	const int UM_TO_MM = 1000;

	long ScanID = 0;
	viewMode = 0;
	sregions.clear();

	if(_xmlObj.get() == NULL)
		return FALSE;

	try
	{
		// make sure the top level root element exist
		ticpp::Element *configObj = _xmlObj->FirstChildElement(false);
		if ( configObj == NULL )
			return FALSE;

		ticpp::Element *templateLevel1 = configObj->FirstChildElement(TEMPLATE_NODES[0]);
		ticpp::Iterator<ticpp::Element> child1(templateLevel1, TEMPLATE_NODES[1]);
		for ( child1 = child1.begin(templateLevel1); child1 != child1.end(); child1++ )
		{
			child1->GetAttribute(VARIABLE_NAME(ScanID), &ScanID);
			ticpp::Element *templateLevel2 = child1->FirstChildElement(TEMPLATE_NODES[2]);
			ticpp::Iterator<ticpp::Element> child2(templateLevel2, TEMPLATE_NODES[3]);

			//get view mode and all scan area ids when area is enabled
			bool IsEnable = false;
			string str = "";
			for ( child2 = child2.begin(templateLevel2); child2 != child2.end(); child2++ )
			{
				child2->GetAttribute(VARIABLE_NAME(IsEnable), &str);
				std::transform(str.begin(), str.end(), str.begin(), ::tolower);
				std::istringstream is(str);
				is >> std::boolalpha >> IsEnable;

				if(IsEnable)
				{
					viewMode = ScanID;
					SampleRegion region;
					child2->GetAttribute(VARIABLE_NAME(ScanAreaID), &region.RegionID);
					child2->GetAttribute(VARIABLE_NAME(PositionX), &region.PositionX);	region.PositionX /= UM_TO_MM;	//expect unit in mm in ImageStore
					child2->GetAttribute(VARIABLE_NAME(PositionY), &region.PositionY);	region.PositionY /= UM_TO_MM;
					child2->GetAttribute(VARIABLE_NAME(PositionZ), &region.PositionZ);	region.PositionZ /= UM_TO_MM;
					child2->GetAttribute(VARIABLE_NAME(PhysicalSizeX), &region.SizeX);	region.SizeX /= UM_TO_MM;
					child2->GetAttribute(VARIABLE_NAME(PhysicalSizeY), &region.SizeY);	region.SizeY /= UM_TO_MM;
					child2->GetAttribute(VARIABLE_NAME(PhysicalSizeZ), &region.SizeZ);	region.SizeZ /= UM_TO_MM;
					region.PhysicalSizeXUnit = region.PhysicalSizeYUnit = region.PhysicalSizeZUnit = ResUnit::Millimetre;
					sregions.push_back(region);
				}
				else
				{
					continue;
				}
			}
		}		
		return TRUE;
	}
	catch(ticpp::Exception ex)
	{
		WCHAR wbuf[_MAX_PATH];
		StringCbPrintfW(wbuf,_MAX_PATH, L"GetSampleRegions error: %s", StringToWString(ex.what()));
		logDll->TLTraceEvent(VERBOSE_EVENT, 1, wbuf);
		return FALSE;
	}
}

long ExperimentXML::GetScanRegions(long &viewMode, vector<ScanRegion> &sregions)
{
	const int UM_TO_MM = 1000;

	long ScanID = 0;
	viewMode = 0;
	sregions.clear();

	if(_xmlObj.get() == NULL)
		return FALSE;

	try
	{
		// make sure the top level root element exist
		ticpp::Element *configObj = _xmlObj->FirstChildElement(false);
		if ( configObj == NULL )
			return FALSE;

		ticpp::Element *templateLevel1 = configObj->FirstChildElement(TEMPLATE_NODES[0]);
		ticpp::Iterator<ticpp::Element> child1(templateLevel1, TEMPLATE_NODES[1]);
		for ( child1 = child1.begin(templateLevel1); child1 != child1.end(); child1++ )
		{
			child1->GetAttribute(VARIABLE_NAME(ScanID), &ScanID);
			ticpp::Element *templateLevel2 = child1->FirstChildElement(TEMPLATE_NODES[2]);
			ticpp::Iterator<ticpp::Element> child2(templateLevel2, TEMPLATE_NODES[3]);

			//get view mode and all scan area ids when area is enabled
			bool IsEnable = false;
			string str = "";
			for ( child2 = child2.begin(templateLevel2); child2 != child2.end(); child2++ )
			{
				child2->GetAttribute(VARIABLE_NAME(IsEnable), &str);
				std::transform(str.begin(), str.end(), str.begin(), ::tolower);
				std::istringstream is(str);
				is >> std::boolalpha >> IsEnable;

				if(IsEnable)
				{
					viewMode = ScanID;
					ScanRegion region;
					region.ScanID = static_cast<uint8_t>(ScanID);
					child2->GetAttribute(VARIABLE_NAME(ScanAreaID), &region.RegionID);
					child2->GetAttribute(VARIABLE_NAME(SizeX), &region.SizeX);
					child2->GetAttribute(VARIABLE_NAME(SizeY), &region.SizeY);
					child2->GetAttribute(VARIABLE_NAME(SizeZ), &region.SizeZ);
					child2->GetAttribute(VARIABLE_NAME(SizeT), &region.SizeT);
					child2->GetAttribute(VARIABLE_NAME(SizeS), &region.SizeS);
					region.BufferSize = 0;
					sregions.push_back(region);
				}
				else
				{
					continue;
				}
			}
		}		
		return TRUE;
	}
	catch(ticpp::Exception ex)
	{
		WCHAR wbuf[_MAX_PATH];
		StringCbPrintfW(wbuf,_MAX_PATH, L"GetScanRegions error: %s", StringToWString(ex.what()));
		logDll->TLTraceEvent(VERBOSE_EVENT, 1, wbuf);
		return FALSE;
	}
}

long ExperimentXML::GetScanAttribute(long viewMode, string attribute, double &value)
{
	long ScanID = 0;

	if(_xmlObj.get() == NULL)
		return FALSE;

	try
	{
		// make sure the top level root element exist
		ticpp::Element *configObj = _xmlObj->FirstChildElement(false);
		if ( configObj == NULL )
			return FALSE;

		ticpp::Element *templateLevel1 = configObj->FirstChildElement(TEMPLATE_NODES[0]);
		ticpp::Iterator<ticpp::Element> child1(templateLevel1, TEMPLATE_NODES[1]);
		for ( child1 = child1.begin(templateLevel1); child1 != child1.end(); child1++ )
		{
			child1->GetAttribute(VARIABLE_NAME(ScanID), &ScanID);
			if (viewMode == ScanID)
			{
				child1->GetAttribute(attribute, &value);
				return TRUE;
			}			
		}		
		return FALSE;
	}
	catch(ticpp::Exception ex)
	{
		WCHAR wbuf[_MAX_PATH];
		StringCbPrintfW(wbuf,_MAX_PATH, L"GetScanAttribute error: %s", StringToWString(ex.what()));
		logDll->TLTraceEvent(VERBOSE_EVENT, 1, wbuf);
		return FALSE;
	}
}

long ExperimentXML::SetAllScanAreas(long viewMode, long enable)
{
	long ScanID = 0, ScanAreaID = 0;
	string IsEnable = (0 == enable) ? "false" : "true";

	if(_xmlObj.get() == NULL)
		return FALSE;

	try
	{
		// make sure the top level root element exist
		ticpp::Element *configObj = _xmlObj->FirstChildElement(false);
		if ( configObj == NULL )
			return FALSE;

		ticpp::Element *templateLevel1 = configObj->FirstChildElement(TEMPLATE_NODES[0]);
		ticpp::Iterator<ticpp::Element> child1(templateLevel1, TEMPLATE_NODES[1]);
		for ( child1 = child1.begin(templateLevel1); child1 != child1.end(); child1++ )
		{
			child1->GetAttribute(VARIABLE_NAME(ScanID), &ScanID);

			//enable or disable all scan areas under view mode
			if (ScanID == viewMode)
			{
				ticpp::Element *templateLevel2 = child1->FirstChildElement(TEMPLATE_NODES[2]);
				ticpp::Iterator<ticpp::Element> child2(templateLevel2, TEMPLATE_NODES[3]);
				for ( child2 = child2.begin(templateLevel2); child2 != child2.end(); child2++ )
				{
					child2->SetAttribute(VARIABLE_NAME(IsEnable), IsEnable);
				}
			}
		}		
		return TRUE;
	}
	catch(ticpp::Exception ex)
	{
		WCHAR wbuf[_MAX_PATH];
		StringCbPrintfW(wbuf,_MAX_PATH, L"GetAreaViewMode error: %s", StringToWString(ex.what()));
		logDll->TLTraceEvent(VERBOSE_EVENT, 1, wbuf);
		return FALSE;
	}
}

const char* const ExperimentXML::ZFILEPOSTAG = "PosList";

long ExperimentXML::GetZPosList(vector<double>& vec)
{

	if (!GetElementList(ZFILEPOSTAG, "item", vec))
	{
		return FALSE;
	}

	return TRUE;
}

long ExperimentXML::GetAttribute(string tagName, string attribute, string &attributeValue)
{
	if(_xmlObj.get() == NULL)
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"ExperimentManger  GetAttribute _xmlObj is null");
		return FALSE;
	}
	// make sure the top level root element exist
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	if ( configObj == NULL )
	{
		return FALSE;
	}	  
	else
	{
		try
		{
			// iterate over to get the particular tag element specified as a parameter(tagName)
			ticpp::Iterator<ticpp::Element> child(configObj->FirstChildElement(tagName), tagName);
			//get the attribute value for the specified attribute name
			child->GetAttribute(attribute, &attributeValue);
		}
		catch(ticpp::Exception ex)
		{
			char buf[512];
			WCHAR wbuf[512];
			const char* msg = ex.what();
			StringCbPrintfA(buf,512, "%s=> Tag: %s Attribute: %s", msg, tagName.c_str(), attribute.c_str());
			MultiByteToWideChar(0, 0, buf, -1, wbuf, 512);  
			logDll->TLTraceEvent(VERBOSE_EVENT, 1, wbuf);
			return FALSE;
		}

		return TRUE;
	}
}

long ExperimentXML::GetAttributeWithChild(ticpp::Iterator<ticpp::Element> child, string tagName, string attribute, string &attributeValue)
{
	if(_xmlObj.get() == NULL)
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"HardwareSetupXML  GetAttribute _xmlObj is null");
		return FALSE;
	}
	// make sure the top level root element exist
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	if ( configObj == NULL )
	{
		return FALSE;
	}	  
	else
	{
		try
		{
			child->GetAttribute(attribute, &attributeValue);
		}
		catch(ticpp::Exception ex)
		{
			char buf[512];
			WCHAR wbuf[512];
			const char* msg = ex.what();
			StringCbPrintfA(buf,512, "%s=> Tag: %s Attribute: %s", msg, tagName.c_str(), attribute.c_str());
			MultiByteToWideChar(0, 0, buf, -1, wbuf, 512);  
			logDll->TLTraceEvent(VERBOSE_EVENT, 1, wbuf);
		}

		return TRUE;
	}
}

long ExperimentXML::SetAttribute(string tagName, string attribute, string attributeValue)
{
	if(_xmlObj.get() == NULL)
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"ExperimentManger  SetAttribute _xmlObj is null");
		return FALSE;
	}
	// make sure the top level root element exist
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	if ( configObj == NULL )
	{
		return FALSE;
	}
	else
	{
		// iterate over to get the particular tag element specified as a parameter(tagName)
		ticpp::Iterator<ticpp::Element> child(configObj->FirstChildElement(tagName), tagName);
		//get the attribute value for the specified attribute name
		child->SetAttribute(attribute, attributeValue);
		return TRUE;
	}
}

long ExperimentXML::SetAttribute(string tagParent, string tagName, string attribute, string attributeValue)
{
	if(_xmlObj.get() == NULL)
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"ExperimentManger  SetAttribute _xmlObj is null");
		return FALSE;
	}
	// make sure the top level root element exist
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	if ( configObj == NULL )
	{
		return FALSE;
	}
	else
	{
		ticpp::Element* parent = configObj->FirstChildElement(tagParent);

		// iterate over to get the particular tag element specified as a parameter(tagName)
		ticpp::Iterator<ticpp::Element> child(parent->FirstChildElement(tagName), tagName);
		//get the attribute value for the specified attribute name
		child->SetAttribute(attribute, attributeValue);
		return TRUE;
	}
}

long ExperimentXML::GetTagCount(string tagParent, string tagName, long &count)
{
	if(_xmlObj.get() == NULL)
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"ExperimentManger  GetTagCount _xmlObj is null");
		return FALSE;
	}
	// make sure the top level root element exist
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	if ( configObj == NULL )
	{
		return FALSE;
	}
	else
	{

		ticpp::Element* parent = configObj->FirstChildElement(tagParent);

		count = 0;

		ticpp::Iterator< ticpp::Element > child( parent, tagName);

		for ( child = child.begin( parent ); child != child.end(); child++ )
		{
			count++;
		}

		return TRUE;
	}
}

long ExperimentXML::GetElementList(string tagParent, string tagName, vector<double>& vect)
{
	if (_xmlObj.get() == NULL)
	{
		logDll->TLTraceEvent(VERBOSE_EVENT, 1, L"ExperimentManger  GetTagCount _xmlObj is null");
		return FALSE;
	}
	// make sure the top level root element exist
	ticpp::Element* configObj = _xmlObj->FirstChildElement(false);

	if (configObj == NULL)
	{
		return FALSE;
	}
	else
	{
		vect.clear(); // clear the vector first

		try
		{
			// find the element named 'tagParent' and iterate over its children of name 'tagName'
			ticpp::Element* parent = configObj->FirstChildElement(tagParent);
			ticpp::Iterator< ticpp::Element > child(parent, tagName);

			for (child = child.begin(parent); child != child.end(); child++)
			{
				string str = child->GetText();
				stringstream ss(str);
				double dVal;
				// convert to double
				ss >> dVal;
				vect.push_back(dVal);
			}
		}
		catch (...)
		{
			logDll->TLTraceEvent(WARNING_EVENT, 1, L"ExperimentManger  ElementList could not be read");
			return FALSE;
		}

		return TRUE;
	}
}

const char * const ExperimentXML::ROOT_TAG = "ThorImageExperiment";

long ExperimentXML::CreateConfigFile(string path)
{
	_xmlObj.reset(new ticpp::Document(path));

	ticpp::Declaration * decl = new ticpp::Declaration("1.0","","");

	ticpp::Element * element = new ticpp::Element(ROOT_TAG);

	_xmlObj->LinkEndChild(decl);
	_xmlObj->LinkEndChild(element);

	CreateTag(ROOT_TAG,NAME);
	SetAttribute(NAME,NAME_ATTR[0],"Test");

	CreateTag(ROOT_TAG,DATE);
	SetAttribute(DATE,DATE_ATTR[0],"7/8/2009");

	CreateTag(ROOT_TAG,USER);
	SetAttribute(USER,USER_ATTR[0],"Admin");

	CreateTag(ROOT_TAG,COMPUTER);
	SetAttribute(COMPUTER,COMPUTER_ATTR[0],"Dell-7400");

	CreateTag(ROOT_TAG,SOFTWARE);
	SetAttribute(SOFTWARE,SOFTWARE_ATTR[0],"1.0");

	CreateTag(ROOT_TAG,CAMERA);
	SetAttribute(CAMERA,CAMERA_ATTR[0],"Basler");
	SetAttribute(CAMERA,CAMERA_ATTR[1],"1280");
	SetAttribute(CAMERA,CAMERA_ATTR[2],"1024");
	SetAttribute(CAMERA,CAMERA_ATTR[3],"6.25");
	SetAttribute(CAMERA,CAMERA_ATTR[4],"1");
	SetAttribute(CAMERA,CAMERA_ATTR[5],"1");
	SetAttribute(CAMERA,CAMERA_ATTR[6],"0");

	CreateTag(ROOT_TAG,MAGNIFICATION);
	SetAttribute(MAGNIFICATION,MAGNIFICATION_ATTR[0],"20.0");

	CreateTag(ROOT_TAG,WAVELENGTHS);

	// make sure the top level root element exist
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	ticpp::Element * parentElement = configObj->FirstChildElement(WAVELENGTHS,false);

	element = new ticpp::Element(WAVELENGTH);
	element->SetAttribute(WAVELENGTH_ATTR[0],"DAPI");
	element->SetAttribute(WAVELENGTH_ATTR[1],"100.0");
	parentElement->LinkEndChild(element);

	element = new ticpp::Element(WAVELENGTH);
	element->SetAttribute(WAVELENGTH_ATTR[0],"Brightfield");
	element->SetAttribute(WAVELENGTH_ATTR[1],"5.0");
	parentElement->LinkEndChild(element);

	CreateTag(ROOT_TAG,ZSTAGE);
	SetAttribute(ZSTAGE,ZSTAGE_ATTR[0],"Thorlabs");	
	SetAttribute(ZSTAGE,ZSTAGE_ATTR[1],"1");
	SetAttribute(ZSTAGE,ZSTAGE_ATTR[2],".1");
	SetAttribute(ZSTAGE,ZSTAGE_ATTR[3],"0");
	SetAttribute(ZSTAGE,ZSTAGE_ATTR[4],"1");
	SetAttribute(ZSTAGE,ZSTAGE_ATTR[5],"1");

	CreateTag(ROOT_TAG,LSM);
	SetAttribute(SAMPLE,WELLS,WELLS_ATTR[0],"0");
	SetAttribute(SAMPLE,WELLS,WELLS_ATTR[1],"1024");
	SetAttribute(SAMPLE,WELLS,WELLS_ATTR[2],"1024");
	SetAttribute(SAMPLE,WELLS,WELLS_ATTR[3],"1");
	SetAttribute(SAMPLE,WELLS,WELLS_ATTR[4],"120");
	SetAttribute(SAMPLE,WELLS,WELLS_ATTR[5],"0");
	//SetAttribute(SAMPLE,WELLS,WELLS_ATTR[6],"0");
	//SetAttribute(SAMPLE,WELLS,WELLS_ATTR[7],"0");
	//SetAttribute(SAMPLE,WELLS,WELLS_ATTR[8],"0");
	//SetAttribute(SAMPLE,WELLS,WELLS_ATTR[9],"0");

	CreateTag(ROOT_TAG,TIMELAPSE);
	SetAttribute(TIMELAPSE,TIMELAPSE_ATTR[0],"1");
	SetAttribute(TIMELAPSE,TIMELAPSE_ATTR[1],"0.0");

	CreateTag(ROOT_TAG,SAMPLE);
	SetAttribute(SAMPLE,SAMPLE_ATTR[0],"0");//96 wellplate defaul
	SetAttribute(SAMPLE,SAMPLE_ATTR[1],"5.0");
	SetAttribute(SAMPLE,SAMPLE_ATTR[2],"5.0");

	CreateTag(SAMPLE,WELLS);
	SetAttribute(SAMPLE,WELLS,WELLS_ATTR[0],"1");
	SetAttribute(SAMPLE,WELLS,WELLS_ATTR[1],"1");
	SetAttribute(SAMPLE,WELLS,WELLS_ATTR[2],"8");
	SetAttribute(SAMPLE,WELLS,WELLS_ATTR[3],"12");
	SetAttribute(SAMPLE,WELLS,WELLS_ATTR[4],"9.0");
	SetAttribute(SAMPLE,WELLS,WELLS_ATTR[5],"9.0");

	parentElement = configObj->FirstChildElement(SAMPLE,false);	
	parentElement = parentElement->FirstChildElement(WELLS,false);

	element = new ticpp::Element(SUBIMAGES);
	element->SetAttribute(SUBIMAGES_ATTR[0],"1");
	element->SetAttribute(SUBIMAGES_ATTR[1],"1");
	element->SetAttribute(SUBIMAGES_ATTR[2],"0.0");
	element->SetAttribute(SUBIMAGES_ATTR[3],"0.0");
	element->SetAttribute(SUBIMAGES_ATTR[4],"0.0");
	element->SetAttribute(SUBIMAGES_ATTR[5],"0.0");

	parentElement->LinkEndChild(element);

	CreateTag(ROOT_TAG,COMMENTS);
	SetAttribute(COMMENTS,COMMENTS_ATTR[0],"");

	_xmlObj->SaveFile(path);

	_currentPathAndFile = path;

	return TRUE;
}

long ExperimentXML::CreateTag(string tagParent, string tag)
{
	if(_xmlObj.get() == NULL)
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"ExperimentManger  CreateTag _xmlObj is null");
		return FALSE;
	}
	// make sure the top level root element exist
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	if ( configObj == NULL )
	{
		return FALSE;
	}
	else
	{
		//get the attribute value for the specified attribute name
		ticpp::Element * element = new ticpp::Element(tag);

		string str(ROOT_TAG);

		if(tagParent == str)
		{
			configObj->LinkEndChild(element);
		}
		else
		{
			ticpp::Element * parentElement = configObj->FirstChildElement(tagParent,false);

			parentElement->LinkEndChild(element);
		}
		return TRUE;
	}
}

long ExperimentXML::Save()
{
	if(_xmlObj.get() == NULL)
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"ExperimentManger  Save _xmlObj is null");
		return FALSE;
	}
	_xmlObj->SaveFile(_currentPathAndFile);

	return TRUE;
}

long ExperimentXML::OpenConfigFile(string path)
{
	_xmlObj.reset(new ticpp::Document(path));

	try
	{
		_xmlObj->LoadFile();
	}
	catch(ticpp::Exception ex)
	{
		wstring ws;

		ws = StringToWString(path);
		const long MSG_SIZE = 256;
		wchar_t message[MSG_SIZE];

		StringCbPrintfW(message,MSG_SIZE,L"ExperimentXML OpenConfigFile failed (%s)",ws.c_str());

		logDll->TLTraceEvent(ERROR_EVENT,1,message);	
	}

	_currentPathAndFile = path;
	return TRUE;
}

void ExperimentXML::Update()
{
	Save();
}

string ExperimentXML::GetPath()
{

	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];

	_splitpath_s(_currentPathAndFile.c_str(),drive,_MAX_DRIVE,dir,_MAX_DIR,fname,_MAX_FNAME,ext,_MAX_EXT);

	stringstream ss;

	ss << drive << dir;

	return ss.str();
}
