#include "stdafx.h"
#include "Strsafe.h"
#include "ThorSLMSetupXML.h"

ThorSLMXML::ThorSLMXML()
{
	_currentPathAndFile = "ThorSLMSettings.xml";
	_isLoad = false;
}

ThorSLMXML::~ThorSLMXML()
{
	_xmlObj.release();
}

const char* const ThorSLMXML::POSTTRANSFORM = "PostCalibration";
const char* const ThorSLMXML::POSTTRANSFORM2 = "PostCalibration2";

const char* const ThorSLMXML::POSTTRANSFORM_ATTR[NUM_POSTTRANSFORM_ATTRIBUTES] = { "verticalFlip","rotateAngle","scaleX","scaleY","offsetX","offsetY" };

long ThorSLMXML::GetPostTransform(int id, long& verticalFlip, double& rotateAngle, double& scaleX, double& scaleY, long& offsetX, long& offsetY)
{
	try
	{
		if (NULL == _xmlObj.get()) { OpenConfigFile(); }

		// make sure the top level root element exist
		ticpp::Element* configObj = _xmlObj.get()->FirstChildElement(false);

		if (configObj == NULL)
			return FALSE;

		ticpp::Iterator< ticpp::Element > child(configObj, 1 == id ? POSTTRANSFORM : POSTTRANSFORM2);

		for (child = child.begin(configObj); child != child.end(); child++)
		{
			bool resetNode = false;
			for (long attCount = 0; attCount < NUM_POSTTRANSFORM_ATTRIBUTES; attCount++)
			{
				if (!child->HasAttribute(POSTTRANSFORM_ATTR[attCount]))
				{
					resetNode = true;
					continue;
				}
				string str;
				child->GetAttribute(POSTTRANSFORM_ATTR[attCount], &str);
				stringstream ss(str);
				switch (attCount)
				{
				case 0:
				{
					ss >> verticalFlip;
				}
				break;
				case 1:
				{
					ss >> rotateAngle;
				}
				break;
				case 2:
				{
					ss >> scaleX;
				}
				break;
				case 3:
				{
					ss >> scaleY;
				}
				break;
				case 4:
				{
					ss >> offsetX;
				}
				break;
				case 5:
				{
					ss >> offsetY;
				}
				break;
				}
			}
			//reconstruct node with preferred attribute order
			if (resetNode)
			{
				UpdateNode(child, POSTTRANSFORM_ATTR, std::vector<std::string> { std::to_string(verticalFlip), std::to_string(rotateAngle), std::to_string(scaleX), std::to_string(scaleY), std::to_string(offsetX), std::to_string(offsetY) });

				SaveConfigFile();
			}
		}
		return TRUE;
	}
	catch (ticpp::Exception ex)
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorSLMXML GetPostTransform failed: %s", StringToWString(ex.what()).c_str());
		return FALSE;
	}
}

long ThorSLMXML::SetPostTransform(int id, long verticalFlip, double rotateAngle, double scaleX, double scaleY, long offsetX, long offsetY)
{
	try
	{
		if (NULL == _xmlObj.get()) { OpenConfigFile(); }

		ticpp::Element* configObj = _xmlObj.get()->FirstChildElement(false);

		if (configObj == NULL)
			return FALSE;

		string str;
		stringstream ss;

		ss << verticalFlip;
		ss << endl;
		ss << rotateAngle;
		ss << endl;
		ss << scaleX;
		ss << endl;
		ss << scaleY;
		ss << endl;
		ss << offsetX;
		ss << endl;
		ss << offsetY;
		ss << endl;

		long index;

		for (index = 0; index < NUM_POSTTRANSFORM_ATTRIBUTES; index++)
		{
			getline(ss, str);

			// iterate over to get the particular tag element specified as a parameter(tagName)
			ticpp::Iterator<ticpp::Element> child(configObj->FirstChildElement(1 == id ? POSTTRANSFORM : POSTTRANSFORM2), 1 == id ? POSTTRANSFORM : POSTTRANSFORM2);
			//get the attribute value for the specified attribute name
			child->SetAttribute(POSTTRANSFORM_ATTR[index], str);
		}

		SaveConfigFile();
		return TRUE;
	}
	catch (exception ex)
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorSLMXML SetPostTransform failed: %s", StringToWString(ex.what()).c_str());
		return FALSE;
	}
}

const char* const ThorSLMXML::CALIBRATION = "Calibration";
const char* const ThorSLMXML::CALIBRATION2 = "Calibration2";

const char* const ThorSLMXML::CALIBRATION_ATTR[NUM_CALIBRATION_ATTRIBUTES] = { "wavelengthNM","phaseMax","offsetZum","coeff1","coeff2","coeff3","coeff4","coeff5","coeff6", "coeff7", "coeff8" };

long ThorSLMXML::GetCalibration(int id, double& wavelengthNM, long& phaseMax, double& offsetZum, double& coeff1, double& coeff2, double& coeff3, double& coeff4, double& coeff5, double& coeff6, double& coeff7, double& coeff8)
{
	try
	{
		if (NULL == _xmlObj.get()) { OpenConfigFile(); }

		ticpp::Element* configObj = _xmlObj.get()->FirstChildElement(false);

		if (configObj == NULL)
			return FALSE;

		ticpp::Iterator< ticpp::Element > child(configObj, 1 == id ? CALIBRATION : CALIBRATION2);
		for (child = child.begin(configObj); child != child.end(); child++)
		{
			bool resetNode = false;
			for (long attCount = 0; attCount < NUM_CALIBRATION_ATTRIBUTES; attCount++)
			{
				if (!child->HasAttribute(CALIBRATION_ATTR[attCount]))
				{
					resetNode = true;
					continue;
				}
				string str;
				child->GetAttribute(CALIBRATION_ATTR[attCount], &str);
				stringstream ss(str);
				switch (attCount)
				{
				case 0:
					ss >> wavelengthNM;
					break;
				case 1:
					ss >> phaseMax;
					break;
				case 2:
					ss >> offsetZum;
					break;
				case 3:
					ss >> coeff1;
					break;
				case 4:
					ss >> coeff2;
					break;
				case 5:
					ss >> coeff3;
					break;
				case 6:
					ss >> coeff4;
					break;
				case 7:
					ss >> coeff5;
					break;
				case 8:
					ss >> coeff6;
					break;
				case 9:
					ss >> coeff7;
					break;
				case 10:
					ss >> coeff8;
					break;
				}
			}
			//reconstruct node with preferred attribute order
			if (resetNode)
			{
				UpdateNode(child, CALIBRATION_ATTR, std::vector<std::string> { std::to_string(wavelengthNM), std::to_string(phaseMax), std::to_string(offsetZum), std::to_string(coeff1), std::to_string(coeff2),
					std::to_string(coeff3), std::to_string(coeff4), std::to_string(coeff5), std::to_string(coeff6), std::to_string(coeff7), std::to_string(coeff8) });

				SaveConfigFile();
			}
		}

		return TRUE;
	}
	catch (exception ex)
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorSLMXML GetCalibration failed: %s", StringToWString(ex.what()).c_str());
		return FALSE;
	}
}

long ThorSLMXML::SetCalibration(int id, double coeff1, double coeff2, double coeff3, double coeff4, double coeff5, double coeff6, double coeff7, double coeff8)
{
	try
	{
		if (NULL == _xmlObj.get()) { OpenConfigFile(); }

		ticpp::Element* configObj = _xmlObj.get()->FirstChildElement(false);

		if (configObj == NULL)
			return FALSE;

		std::vector<std::string> attr = { std::to_string(coeff1), std::to_string(coeff2), std::to_string(coeff3), std::to_string(coeff4),
			std::to_string(coeff5), std::to_string(coeff6), std::to_string(coeff7), std::to_string(coeff8) };

		const int START_INDEX = 3;
		for (int index = START_INDEX; index < NUM_CALIBRATION_ATTRIBUTES; index++)
		{
			// iterate over to get the particular tag element specified as a parameter(tagName)
			ticpp::Iterator<ticpp::Element> child(configObj->FirstChildElement(1 == id ? CALIBRATION : CALIBRATION2), 1 == id ? CALIBRATION : CALIBRATION2);
			//get the attribute value for the specified attribute name
			child->SetAttribute(CALIBRATION_ATTR[index], attr[index - (int)START_INDEX]);
		}

		return SaveConfigFile();
	}
	catch (exception ex)
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorSLMXML SetCalibration failed: %s", StringToWString(ex.what()).c_str());
		return FALSE;
	}
}

long ThorSLMXML::SetDefocus(int id, double offsetZum)
{
	try
	{
		// make sure the top level root element exist
		ticpp::Element* configObj = _xmlObj.get()->FirstChildElement(false);

		if (configObj == NULL)
			return FALSE;

		ticpp::Iterator<ticpp::Element> child(configObj->FirstChildElement(1 == id ? CALIBRATION : CALIBRATION2), 1 == id ? CALIBRATION : CALIBRATION2);
		child->SetAttribute(CALIBRATION_ATTR[2], std::to_string(offsetZum));

		return SaveConfigFile();
	}
	catch (exception ex)
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorSLMXML SetDefocus failed: %s", StringToWString(ex.what()).c_str());
		return FALSE;
	}
}

const char* const ThorSLMXML::SPEC = "Spec";

const char* const ThorSLMXML::SPEC_ATTR[NUM_SPEC_ATTRIBUTES] = { "Name","dmdMode","overDrive","transientFrames","pitchUM","flatDiagRatio","flatPowerMinPercent","flatPowerMaxPercent","pixelX","pixelY","LUT","overDriveLUT","waveFront", "persistHologramZone1", "persistHologramZone2" };

long ThorSLMXML::GetSpec(string& name, long& dmdMode, long& overDrive, unsigned int& transientFrames, double& pitchUM, double& flatDiagRatio, double& flatPowerMinPercent, double& flatPowerMaxPercent, long& pixelX, long& pixelY, string& lut, string& odLUT, string& wavefront, long& persistHologramZone1, long& persistHologramZone2)
{
	try
	{
		if (NULL == _xmlObj.get()) { OpenConfigFile(); }

		ticpp::Element* configObj = _xmlObj.get()->FirstChildElement(false);

		if (configObj == NULL)
			return FALSE;

		ticpp::Iterator< ticpp::Element > child(configObj, SPEC);
		for (child = child.begin(configObj); child != child.end(); child++)
		{
			bool resetNode = false;
			for (long attCount = 0; attCount < NUM_SPEC_ATTRIBUTES; attCount++)
			{
				if (!child->HasAttribute(SPEC_ATTR[attCount]))
				{
					resetNode = true;
					continue;
				}
				string str;
				child->GetAttribute(SPEC_ATTR[attCount], &str);
				stringstream ss(str);
				switch (attCount)
				{
				case 0:
				{
					name = str;
				}
				break;
				case 1:
				{
					ss >> dmdMode;
				}
				break;
				case 2:
				{
					ss >> overDrive;
				}
				break;
				case 3:
				{
					ss >> transientFrames;
				}
				break;
				case 4:
				{
					ss >> pitchUM;
				}
				break;
				case 5:
				{
					ss >> flatDiagRatio;
				}
				break;
				case 6:
				{
					ss >> flatPowerMinPercent;
				}
				break;
				case 7:
				{
					ss >> flatPowerMaxPercent;
				}
				break;
				case 8:
				{
					ss >> pixelX;
				}
				break;
				case 9:
				{
					ss >> pixelY;
				}
				break;
				case 10:
				{
					lut = str;
				}
				break;
				case 11:
				{
					odLUT = str;
				}
				break;
				case 12:
				{
					wavefront = str;
				}
				break;
				case 13:
				{
					ss >> persistHologramZone1;
				}
				break;
				case 14:
				{
					ss >> persistHologramZone2;
				}
				break;
				}
			}
			//reconstruct node with preferred attribute order
			if (resetNode)
			{
				UpdateNode(child, SPEC_ATTR, std::vector<std::string> { name, std::to_string(dmdMode), std::to_string(overDrive), std::to_string(transientFrames), std::to_string(pitchUM), std::to_string(flatDiagRatio),
					std::to_string(flatPowerMinPercent), std::to_string(flatPowerMaxPercent), std::to_string(pixelX), std::to_string(pixelY), lut,
					odLUT, wavefront, std::to_string(persistHologramZone1), std::to_string(persistHologramZone2)  });

				SaveConfigFile();
			}
		}
		return TRUE;
	}
	catch (exception ex)
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorSLMXML GetSpec failed: %s", StringToWString(ex.what()).c_str());
		return FALSE;
	}
}

const char* const ThorSLMXML::TRIGGER = "Trigger";

const char* const ThorSLMXML::TRIGGER_ATTR[NUM_TRIGGER_ATTRIBUTES] = { "counterLine","triggerLine" };

long ThorSLMXML::GetTrigger(string& counterLine, string& triggerInput)
{
	try
	{
		if (NULL == _xmlObj.get()) { OpenConfigFile(); }

		ticpp::Element* configObj = _xmlObj.get()->FirstChildElement(false);

		if (configObj == NULL)
			return FALSE;

		ticpp::Iterator< ticpp::Element > child(configObj, TRIGGER);
		for (child = child.begin(configObj); child != child.end(); child++)
		{
			bool resetNode = false;
			for (long attCount = 0; attCount < NUM_TRIGGER_ATTRIBUTES; attCount++)
			{
				if (!child->HasAttribute(TRIGGER_ATTR[attCount]))
				{
					resetNode = true;
					continue;
				}
				string str;
				child->GetAttribute(TRIGGER_ATTR[attCount], &str);
				stringstream ss(str);
				switch (attCount)
				{
				case 0:
				{
					counterLine = str;
				}
				break;
				case 1:
				{
					triggerInput = str;
				}
				break;
				}
			}
			//reconstruct node with preferred attribute order
			if (resetNode)
			{
				UpdateNode(child, TRIGGER_ATTR, std::vector<std::string> { counterLine, triggerInput });
				SaveConfigFile();
			}
		}
		return TRUE;
	}
	catch (exception ex)
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorSLMXML GetTrigger failed: %s", StringToWString(ex.what()).c_str());
		return FALSE;
	}
}

const char* const ThorSLMXML::WINDVI = "WinDVI";

const char* const ThorSLMXML::WINDVI_ATTR[NUM_WINDVI_ATTRIBUTES] = { "monitorID" };

long ThorSLMXML::GetWinDVI(std::wstring& monitorID)
{
	try
	{
		if (NULL == _xmlObj.get()) { OpenConfigFile(); }

		ticpp::Element* configObj = _xmlObj.get()->FirstChildElement(false);

		if (configObj == NULL)
			return FALSE;

		ticpp::Iterator< ticpp::Element > child(configObj, WINDVI);

		for (child = child.begin(configObj); child != child.end(); child++)
		{
			for (long attCount = 0; attCount < NUM_WINDVI_ATTRIBUTES; attCount++)
			{
				string str;
				child->GetAttribute(WINDVI_ATTR[attCount], &str);
				switch (attCount)
				{
				case 0:
					monitorID = StringToWString(str);
					break;
				}
			}
		}
		return TRUE;
	}
	catch (...)
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorSLMXML GetWinDVI failed");
		return FALSE;
	}
}

long ThorSLMXML::GetLastErrorMsg(wchar_t* msg, long size)
{
	StringCbPrintfW(msg, size, _errMsg);
	return TRUE;

}

long ThorSLMXML::OpenConfigFile(long forceReload)
{
	try
	{
		errno_t err = 0;
		FILE* file = NULL;
		if (!_isLoad || forceReload)
		{
			if (err = fopen_s(&file, _currentPathAndFile.c_str(), "r") == 0)
			{
				fclose(file);
				//file exist, continue to load
			}
			else {
				//backward compatible, try find old name and have it renamed
				std::string sTemp = "ThorSLMPDM512Settings.xml";
				if (err = fopen_s(&file, sTemp.c_str(), "r") == 0)
				{
					fclose(file);
					err = rename(sTemp.c_str(), _currentPathAndFile.c_str());
				}
			}
			_xmlObj.reset(new ticpp::Document(_currentPathAndFile));
			_xmlObj.get()->LoadFile();
			_isLoad = true;
		}
		return TRUE;
	}
	catch (exception ex)
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorSLMXML OpenConfigFile failed: %s", StringToWString(ex.what()).c_str());
		return FALSE;
	}
}

long ThorSLMXML::SaveConfigFile()
{
	try
	{
		_xmlObj.get()->SaveFile();
		return TRUE;
	}
	catch (exception ex)
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorSLMXML SaveConfigFile failed: %s", StringToWString(ex.what()).c_str());
		return FALSE;
	}
}

//*************************************************** Private Functions ***************************************************//

void ThorSLMXML::UpdateNode(ticpp::Iterator<ticpp::Element> iElement, const char* const attributes[], std::vector<string> attriValues)
{
	ticpp::Attribute* att = iElement->FirstAttribute(false);
	do
	{
		iElement->RemoveAttribute(att->Name());
		att = iElement->FirstAttribute(false);
	} while (NULL != att);

	for (long attCount = 0; attCount < attriValues.size(); attCount++)
	{
		iElement->SetAttribute(attributes[attCount], attriValues[attCount]);
	}
}