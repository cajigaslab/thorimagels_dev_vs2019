#include "Types.h"
#include "Logger.h"
#include "..\..\..\Common\StringCPP.h"
#include "..\..\..\Tools\tinyxml2\include\tinyxml2.h"
#include "..\..\..\Common\BinaryImageDataUtilities\GenericImage.h"
#include "..\..\..\Common\ResourceManager\ResourceManager\ResourceManager.h"
#include "..\..\..\Common\ThorSharedTypesCPP.h"

//load scan or necessary info from provided experiment path
long LoadMeso::LoadExperimentXML()
{
	if (0 >= ExpPath.size())
		return 0;

	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError error = tinyxml2::XMLError::XML_SUCCESS;
	if (0 == ExpPath.compare(ResourceManager::getInstance()->GetActiveSettingsFilePathAndName()))
	{
		ResourceManager::getInstance()->BorrowDocMutex(SettingsFileType::ACTIVE_EXPERIMENT_SETTINGS);
		error = doc.LoadFile(WStringToString(ExpPath).c_str());
		ResourceManager::getInstance()->ReturnDocMutex(SettingsFileType::ACTIVE_EXPERIMENT_SETTINGS);
		if (tinyxml2::XMLError::XML_SUCCESS != error)
		{
			Logger::getInstance().LogMessage(L"Load Experiment Failed!");
			return 0;
		}
	}
	else if (tinyxml2::XMLError::XML_SUCCESS != doc.LoadFile(WStringToString(ExpPath).c_str()))
	{
		Logger::getInstance().LogMessage(L"Load Experiment Failed!");
		return 0;
	}
	try
	{
		tinyxml2::XMLElement* root = doc.RootElement();
		tinyxml2::XMLElement* templateScans = root->FirstChildElement("TemplateScans");
		if (NULL == templateScans)
		{
			throw exception("Bad Format in Experiment.");
		}
		ReleaseScans();
		for (tinyxml2::XMLElement* child = templateScans->FirstChildElement("ScanInfo"); child != NULL; child = child->NextSiblingElement("ScanInfo"))
		{
			tinyxml2::XMLElement* scanAreas = child->FirstChildElement("ScanAreas");
			if (NULL == scanAreas)
				continue;

			//determine meso scan or micro scan, only one mode at a time
			std::string scanType = std::string(child->Attribute("Name"));
			if (0 == scanType.compare("Meso") && (0 < Scans.size()))
				break;		//single scan area in meso scan

			Scan* tScan = new Scan();
			tScan->Name = scanType;
			tScan->ScanID = child->IntAttribute("ScanID");
			tScan->SignificantBits = child->UnsignedAttribute("SignificantBits");
			tScan->TileHeight = child->UnsignedAttribute("TileHeight");
			tScan->TileWidth = child->UnsignedAttribute("TileWidth");
			tScan->TimeInterval = child->DoubleAttribute("TimeInterval");
			tScan->XPixelSize = child->DoubleAttribute("XPixelSize");
			tScan->YPixelSize = child->DoubleAttribute("YPixelSize");
			tScan->ZPixelSize = child->DoubleAttribute("ZPixelSize");
			EnumString<PixelType>::To(tScan->IPixelType, "PixelType_" + std::string(child->Attribute("IPixelType")));
			EnumString<ResUnit>::To(tScan->ResUnit, std::string(child->Attribute("ResUnit")));

			//scan configure
			tinyxml2::XMLElement* scanConfigure = child->FirstChildElement("ScanConfigure");
			if (NULL != scanConfigure)
			{
				tScan->ScanConfig.ScanMode = (ScanMode)scanConfigure->IntAttribute("ScanMode");
				tScan->ScanConfig.AverageMode = (AverageMode)scanConfigure->IntAttribute("AverageMode");
				tScan->ScanConfig.NumberOfAverageFrame = (AverageMode::NO_AVERAGE == tScan->ScanConfig.AverageMode) ? 1 : scanConfigure->UnsignedAttribute("NumberOfAverageFrame");
				tScan->ScanConfig.PhysicalFieldSize = scanConfigure->DoubleAttribute("PhysicalFieldSize");
				tScan->ScanConfig.StripLength = scanConfigure->UnsignedAttribute("StripLength");
				tScan->ScanConfig.CurrentPower = scanConfigure->DoubleAttribute("CurrentPower");
				tScan->ScanConfig.IsLivingMode = scanConfigure->UnsignedAttribute("IsLivingMode");
				tScan->ScanConfig.RemapShift = scanConfigure->IntAttribute("RemapShift");
			}

			//scan areas
			if (NULL != scanAreas)
			{
				for (tinyxml2::XMLElement* child1 = scanAreas->FirstChildElement("ScanArea"); child1 != NULL; child1 = child1->NextSiblingElement("ScanArea"))
				{
					if (!child1->BoolAttribute("IsEnable",false))
						continue;	//load enabled scan area only

					ScanArea* sArea = new ScanArea();
					sArea->ScanAreaID = child1->UnsignedAttribute("ScanAreaID");
					sArea->PhysicalSizeX = child1->DoubleAttribute("PhysicalSizeX");
					sArea->PhysicalSizeY = child1->DoubleAttribute("PhysicalSizeY");
					sArea->PhysicalSizeZ = child1->DoubleAttribute("PhysicalSizeZ");
					sArea->PositionX = child1->DoubleAttribute("PositionX");
					sArea->PositionY = child1->DoubleAttribute("PositionY");
					sArea->PositionZ = child1->DoubleAttribute("PositionZ");
					sArea->SizeX = child1->UnsignedAttribute("SizeX");
					sArea->SizeY = child1->UnsignedAttribute("SizeY");
					sArea->SizeZ = child1->UnsignedAttribute("SizeZ");
					sArea->SizeS = child1->UnsignedAttribute("SizeS");
					sArea->SizeT = child1->UnsignedAttribute("SizeT");

					tinyxml2::XMLElement* powerPoints = child1->FirstChildElement("PowerPoints");
					if (NULL != powerPoints)
					{
						for (tinyxml2::XMLElement* child2 = powerPoints->FirstChildElement("PowerPoint"); child2 != NULL; child2 = child2->NextSiblingElement("PowerPoint"))
						{
							PowerPoint* pPoint = new PowerPoint();
							pPoint->ZPosition = child2->DoubleAttribute("ZPosition");
							pPoint->PowerPercentage = child2->DoubleAttribute("PowerPercentage");

							sArea->PowerPoints.push_back(pPoint);
						}
					}

					tinyxml2::XMLElement* powerBoxs = child1->FirstChildElement("PowerBoxs");
					if (NULL != powerBoxs)
					{
						for (tinyxml2::XMLElement* child2 = powerBoxs->FirstChildElement("PowerBox"); child2 != NULL; child2 = child2->NextSiblingElement("PowerBox"))
						{
							PowerBox* pBox = new PowerBox();
							pBox->StartZ = child2->DoubleAttribute("StartZ");
							pBox->EndZ = child2->DoubleAttribute("EndZ");
							pBox->PowerPercentage = child2->DoubleAttribute("PowerPercentage");
							//pBox->PowerROI = ?;

							sArea->PowerBoxs.push_back(pBox);
						}
					}
					tScan->ScanAreas.push_back(sArea);
				}
			}
			if (0 >= tScan->ScanAreas.size())
			{
				delete tScan;
				continue;	//no scan area found in this scan
			}

			//channels
			tinyxml2::XMLElement* wavelengths = root->FirstChildElement("Wavelengths");
			vector<string> wavelength;
			if (NULL != wavelengths)
			{
				for (tinyxml2::XMLElement* child1 = wavelengths->FirstChildElement("Wavelength"); child1 != NULL; child1 = child1->NextSiblingElement("Wavelength"))
				{
					wavelength.push_back(child1->Attribute("name"));
				}
			}
			tinyxml2::XMLElement* channels = root->FirstChildElement("Wavelengths")->FirstChildElement("ChannelEnable");
			tScan->ScanConfig.ChannelCount = 0;
			if (NULL != channels)
			{
				vector<int> enabledChannel = ChannelManipulator<unsigned short>::getEnabledChannels(channels->IntAttribute("Set"));
				if (1 == enabledChannel.size())
				{
					Channel* chInfo = new Channel();
					chInfo->ChannelID = static_cast<uint16_t>(0);
					chInfo->ChannelRefID = static_cast<uint16_t>(enabledChannel.at(0));
					tScan->Channels.push_back(chInfo);
					tScan->ScanConfig.ChannelCount++;
				}
				else if (1 < enabledChannel.size())
				{
					//fixed 4 channels acquisition
					for (int i = 0; i < CHANNEL_COUNT; i++)
					{
						//no gap allowed in scan info channels, keep channel ID based on number of channels, channelID: 0-based
						//and use ref ID for actual enabled channel ID
						Channel* chInfo = new Channel();
						chInfo->ChannelID = static_cast<uint16_t>(i);
						chInfo->ChannelRefID = static_cast<uint16_t>(i);
						//if (i < wavelength.size())
						//	strcpy_s(chInfo->Name, wavelength[i].c_str());

						tScan->Channels.push_back(chInfo);
						tScan->ScanConfig.ChannelCount++;
					}
				}
			}
			Scans.push_back(tScan);
		}

	}
	catch(exception ex)
	{
		Logger::getInstance().LogMessage((wchar_t*)StringToWString(ex.what()).c_str());
		return 0;
	}
	return (0 < Scans.size());
}
