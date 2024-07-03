#include "stdafx.h"
#include "ThorDAQIOXML.h"
#include "strsafe.h"
#include <iostream>     // std::cout, std::endl
#include <iomanip>      // std::setw
ThorDAQIOXML::ThorDAQIOXML()
{
	_xmlObj = NULL;
	_currentPathAndFile = "ThorDAQIOSettings.xml";
}

ThorDAQIOXML::~ThorDAQIOXML()
{
	if (_xmlObj != NULL)
	{
		delete _xmlObj;
	}
}

long ThorDAQIOXML::ConvertDBBSelectionToDIO(long lineIndex, THORDAQ_BOB_TYPE bobType)
{
	switch (bobType)
	{
	case TDQ_LEGACY_BOB:
		switch (lineIndex)
		{
		case 1: return 0;
		case 2: return 1;
		case 3: return 2;
		case 4: return 3;
		case 5: return 4;
		case 6: return 5;
		case 7: return 6;
		case 8: return 7;
		}
		break;
	case TDQ_3U_BOB:
		if (lineIndex >= 0 && lineIndex < 15)
		{
			return lineIndex;
		}
		break;
	default:
		break;
	}

	return -1;
}

UINT8 ThorDAQIOXML::GetDigLineSelection(UINT8 lineIndex, long dbb1Selection, THORDAQ_BOB_TYPE bobType)
{
	switch (lineIndex)
	{
	case DIO_LINES::DI_ImagingHardwareTrigger1: return DI_HW_Trigger_1;
	case DIO_LINES::DI_ImagingHardwareTrigger2: return DI_HW_Trigger_2;
	case DIO_LINES::DI_ImagingInternalDigitalTrigger:
	{
		//map to the internal digital line index
		return (UINT8)ConvertDBBSelectionToDIO(dbb1Selection, bobType); //TODO thordaq 2.0: what if it's negative
	}
	case DIO_LINES::DI_StimHardwareTrigger1: return DI_HW_Trigger_3;
	case DIO_LINES::DI_StimHardwareTrigger2: return DI_HW_Trigger_4;
	case DIO_LINES::DI_StimInternalDigitalTrigger:
	{
		//map to the internal digital line index
		return (UINT8)ConvertDBBSelectionToDIO(dbb1Selection, bobType);
	}
	case DIO_LINES::DI_ResonantScannerLineClock: return TD_DIO_MUXedSLAVE_PORTS::DI_ResonantScanner_Line_Feedback;
	case DIO_LINES::DO_GRFrameTrigger: return DO_Frame_Trigger_Out;
	case DIO_LINES::DO_GRPockelsDigitalLine: return DO_Pockels_Imaging_Dig_Out;
	case DIO_LINES::DO_GGFrameTrigger: return DO_Frame_Trigger_Out;
	case DIO_LINES::DO_GGLineTrigger: return DO_Line_Trigger_Out;
	case DIO_LINES::DO_GGPockelsDigitalLine: return DO_Pockels_Imaging_Dig_Out;
	case DIO_LINES::DO_CaptureActive: return DO_Capture_Active;
	case DIO_LINES::DO_BufferReady: return DO_Buffer_Ready;
	case DIO_LINES::DO_StimPockelsDigitalLine: return DO_StimPockelsDig_Out;
	case DIO_LINES::DO_StimActiveEnvelope: return DO_StimActiveEnvelope_Out;
	case DIO_LINES::DO_StimComplete: return DO_StimCycleComplete_Out;
	case DIO_LINES::DO_StimCycleEnvelope: return DO_StimCycleEnvelope_Out;
	case DIO_LINES::DO_StimIterationEnvelope: return DO_StimIterationEnvelope_Out;
	case DIO_LINES::DO_StimEpochEnvelope: return DO_StimEpochEnvelope_Out;
	case DIO_LINES::DO_StimPatterComplete: return DO_StimPatternComplete_Out;
	case DIO_LINES::DO_StimPatternEnvelope: return DO_StimPatternEnvelope_Out;
	case DIO_LINES::DO_InternalLinetrigger: return DO_ScanLine_Direction;
	case DIO_LINES::DO_InternalFrameTrigger: return DO_Internal_SOF_Trigger;
	case DIO_LINES::DO_InternalPixelClock: return DO_Pixel_Clock;
	case DIO_LINES::DO_InternalLineClock: return DO_Horizontal_Line_Trigger_Pulse;
		//TODO: add remaining thordaq internal lines
	}

	return DO_0_WaveMUX;
}


long ThorDAQIOXML::ConfigDigitalLines(std::map<UINT8, long> digitalIOSelection, THORDAQ_BOB_TYPE bobType, DIOSettingsType settingsType, std::vector<string>& dioConfig)
{
	std::map<UINT8, UINT8> tdqDigitalConfig;

	std::map<UINT8, long> digitalIOSelectionConversion;
	for (UINT8 i = 0; i < DIO_LINES::LAST_LINE; ++i)
	{
		switch (settingsType)
		{
		case DIOSettingsType::GR:
			if (DI_ImagingInternalDigitalTrigger == i || DI_StimInternalDigitalTrigger == i)
			{
				continue;
			}
			if (DIO_LINES::DO_GGPockelsDigitalLine == i)
			{
				if (digitalIOSelection.find(i) != digitalIOSelection.end())
				{
					if (digitalIOSelection[i] == digitalIOSelection[DO_GRPockelsDigitalLine])
					{
						continue;
					}
				}
			}
			if (DIO_LINES::DO_GGFrameTrigger == i)
			{
				if (digitalIOSelection.find(i) != digitalIOSelection.end())
				{
					if (digitalIOSelection[i] == digitalIOSelection[DO_GRFrameTrigger])
					{
						continue;
					}
				}
			}
			if (DIO_LINES::DO_StimPockelsDigitalLine == i)
			{
				if (digitalIOSelection.find(i) != digitalIOSelection.end())
				{
					if (digitalIOSelection[i] == digitalIOSelection[DO_GRPockelsDigitalLine])
					{
						continue;
					}
				}
			}
			break;
		case DIOSettingsType::GG:
			if (DI_ImagingInternalDigitalTrigger == i || DI_StimInternalDigitalTrigger == i)
			{
				continue;
			}
			if (DIO_LINES::DO_GGPockelsDigitalLine == i)
			{
				if (digitalIOSelection.find(i) != digitalIOSelection.end())
				{
					if (digitalIOSelection[i] == digitalIOSelection[DO_GRPockelsDigitalLine])
					{
						continue;
					}
				}
			}
			if (DIO_LINES::DO_GGFrameTrigger == i)
			{
				if (digitalIOSelection.find(i) != digitalIOSelection.end())
				{
					if (digitalIOSelection[i] == digitalIOSelection[DO_GRFrameTrigger])
					{
						continue;
					}
				}
			}
			if (DIO_LINES::DO_StimPockelsDigitalLine == i)
			{
				if (digitalIOSelection.find(i) != digitalIOSelection.end())
				{
					if (digitalIOSelection[i] == digitalIOSelection[DO_GRPockelsDigitalLine])
					{
						continue;
					}
				}
			}
			break;
		case DIOSettingsType::STIM:
			if (DIO_LINES::DO_GRPockelsDigitalLine == i)
			{
				if (digitalIOSelection.find(i) != digitalIOSelection.end())
				{
					if (digitalIOSelection[i] == digitalIOSelection[DO_GGPockelsDigitalLine] || digitalIOSelection[i] == digitalIOSelection[DO_StimPockelsDigitalLine])
					{
						continue;
					}
				}
			}
			if (DIO_LINES::DO_GGPockelsDigitalLine == i)
			{
				if (digitalIOSelection.find(i) != digitalIOSelection.end())
				{
					if (digitalIOSelection[i] == digitalIOSelection[DO_GGPockelsDigitalLine])
					{
						continue;
					}
				}
			}
			break;
		default:
			break;
		}
		
		digitalIOSelectionConversion[i] = ConvertDBBSelectionToDIO(digitalIOSelection[i], bobType);
	}


	for (UINT8 i = 0; i < DIO_LINES::LAST_LINE; ++i)
	{
		if (settingsType != DIOSettingsType::STIM)
		{
			if (DI_ImagingInternalDigitalTrigger == i || DI_StimInternalDigitalTrigger == i)
			{
				continue;
			}
		}
		//Need to add map of DIO to channel
		if (digitalIOSelectionConversion.find(i) != digitalIOSelectionConversion.end())
		{
			// we only have DIO_CHANNEL_COUNT available channels, so if the wrong setting was entered into _digitalIOSelection or it was empty (-1) then move to the next one
			if ((BOB_3U_DIO_AVAILABLE_BNCS <= digitalIOSelectionConversion[i] && THORDAQ_BOB_TYPE::TDQ_3U_BOB == bobType) ||
				(DIO_CHANNEL_COUNT <= digitalIOSelectionConversion[i] && THORDAQ_BOB_TYPE::TDQ_LEGACY_BOB == bobType)
				|| 0 > digitalIOSelectionConversion[i])
			{
				continue;
			}

			tdqDigitalConfig[(UINT8)digitalIOSelectionConversion[i]] = GetDigLineSelection(i, digitalIOSelectionConversion[i], bobType);
		}
	}

	if (tdqDigitalConfig.size() < DIO_CHANNEL_COUNT)
	{
		for (UINT8 i = 0; i < DIO_CHANNEL_COUNT; ++i)
		{
			if (tdqDigitalConfig.find(i) == tdqDigitalConfig.end())
			{
				tdqDigitalConfig[i] = DO_Pixel_Clock;
				if (tdqDigitalConfig.size() == DIO_CHANNEL_COUNT)
				{
					break;
				}
			}
		}
	}

	dioConfig = vector<string>();
	std::queue<int> unusedChan = std::queue<int>();
	int count = 0;

	for (int i = 0; i < DIO_CHANNEL_COUNT; ++i)
	{
		if (tdqDigitalConfig.find(i) == tdqDigitalConfig.end())
		{
			unusedChan.push(i);
		}
	}

	for (const auto& config : tdqDigitalConfig)
	{
		string io;
		string a = "A-";
		switch (config.second)
		{
		case TD_DIO_MUXedSLAVE_PORTS::DI_ResonantScanner_Line_Feedback:
		case TD_DIO_MUXedSLAVE_PORTS::DI_External_Line_Trigger:
		case TD_DIO_MUXedSLAVE_PORTS::DI_External_Pixel_Clock:
		case TD_DIO_MUXedSLAVE_PORTS::DI_External_Frame_Retrigger:
		case TD_DIO_MUXedSLAVE_PORTS::DI_External_SOF_Trigger:
			io = "I";
			a = "A-01";
			break;
		case TD_DIO_MUXedSLAVE_PORTS::DI_HW_Trigger_1:
		case TD_DIO_MUXedSLAVE_PORTS::DI_HW_Trigger_2:
		case TD_DIO_MUXedSLAVE_PORTS::DI_HW_Trigger_3:
		case TD_DIO_MUXedSLAVE_PORTS::DI_HW_Trigger_4:
			io = "I";
			a = "A-01";
			break;
		default:
			io = "O";
			a = "A-01";
			break;
		}

		std::stringstream ss1;
		ss1 << std::setw(2) << std::setfill('0') << (int)config.second;
		std::string m = ss1.str();

		int fpgaIndex = config.first;

		if (fpgaIndex >= DIO_CHANNEL_COUNT)
		{
			fpgaIndex = unusedChan.front();
			unusedChan.pop();
		}
		std::stringstream ss2;
		ss2 << std::setw(2) << std::setfill('0') << (int)config.first;
		std::string chan = ss2.str();

		std::stringstream ss3;
		ss3 << std::setw(2) << std::setfill('0') << (int)fpgaIndex;
		std::string fpgaIndexStr = ss3.str();

		//FORMAT: "D05I00M00A-01"; DigitalLine BNC# I/O FPGAIndex M Mux A-01

		string cmd = "D" + chan + io + fpgaIndexStr + "M" + m + a;
		dioConfig.push_back(cmd);
		++count;
		if (count >= DIO_CHANNEL_COUNT)
		{
			break;
		}
	}

	return TRUE;
}

const char* const ThorDAQIOXML::DIOSettings = "DIOSettings";

const char* const ThorDAQIOXML::DIOSETTINGS_ATTR[NUM_DIO_SETTINGS_ATTRIBUTES] = { "captureActiveInvert" };

long ThorDAQIOXML::GetDIOSettings(long& captureActiveInvert)
{
	if (_xmlObj != NULL)
	{
		delete _xmlObj;
	}

	_xmlObj = new ticpp::Document(_currentPathAndFile);
	_xmlObj->LoadFile();

	// make sure the top level root element exist
	ticpp::Element* configObj = _xmlObj->FirstChildElement(false);

	if (configObj == NULL)
	{
		return FALSE;
	}
	else
	{
		try
		{
			ticpp::Iterator< ticpp::Element > child(configObj, DIOSettings);

			for (child = child.begin(configObj); child != child.end(); child++)
			{
				for (UINT8 attIdx = 0; attIdx < NUM_DIO_SETTINGS_ATTRIBUTES; attIdx++)
				{
					std::string str;
					child->GetAttribute(DIOSETTINGS_ATTR[attIdx], &str);
					std::stringstream ss(str);
					switch (attIdx)
					{
					case 0:
					{
						ss >> captureActiveInvert;
					}
					break;
					}
				}
			}
		}
		catch (ticpp::Exception ex)
		{
			return FALSE;
		}
	}
	return TRUE;
}

const char* const ThorDAQIOXML::DIOCONFIG = "DIOLinesConfig";
const char* const ThorDAQIOXML::DIOCONFIG_ATTR[DIO_LINES::LAST_LINE] = { "DI_ImagingHardwareTrigger1", "DI_ImagingHardwareTrigger2", "DI_ImagingInternalDigitalTrigger", "DI_StimHardwareTrigger1", "DI_StimHardwareTrigger2", "DI_StimInternalDigitalTrigger", "DI_ResonantScannerLineClock","DO_GRFrameTrigger","DO_GRPockelsDigitalLine","DO_GGFrameTrigger","DO_GGLineTrigger", "DO_GGPockelsDigitalLine","DO_CaptureActive", "DO_BufferReady","DO_StimPockelsDigitalLine","DO_StimActiveEnvelope","DO_StimComplete","DO_StimCycleEnvelope","DO_StimIterationEnvelope","DO_StimEpochEnvelope","DO_StimPatternComplete","DO_StimPatternEnvelope", "DO_InternalLinetrigger", "DO_InternalFrameTrigger","DO_InternalPixelClock", "DO_InternalLineClock" };

long ThorDAQIOXML::GetDIOLinesConfiguration(std::map<UINT8, long>& digitalIOSelection)
{
	if (_xmlObj != NULL)
	{
		delete _xmlObj;
	}
	digitalIOSelection = std::map<UINT8, long>();
	_xmlObj = new ticpp::Document(_currentPathAndFile);
	_xmlObj->LoadFile();

	// make sure the top level root element exist
	ticpp::Element* configObj = _xmlObj->FirstChildElement(false);

	if (configObj == NULL)
	{
		return FALSE;
	}
	else
	{
		try
		{
			ticpp::Iterator< ticpp::Element > child(configObj, DIOCONFIG);

			for (child = child.begin(configObj); child != child.end(); child++)
			{
				for (UINT8 attIdx = 0; attIdx < DIO_LINES::LAST_LINE; attIdx++)
				{
					std::string str;
					child->GetAttribute(DIOCONFIG_ATTR[attIdx], &str);
					std::stringstream ss(str);
					long val;

					if (str.empty() || std::all_of(str.begin(), str.end(), [](char c) {return std::isspace(c); }))
					{
						val = -1;
					}
					else
					{
						ss >> val;
					}

					digitalIOSelection[attIdx] = val;
				}
			}
		}
		catch (ticpp::Exception ex)
		{
			return FALSE;
		}
	}
	return TRUE;
}


const char* const ThorDAQIOXML::AOCONFIG = "AOLinesConfig";
const char* const ThorDAQIOXML::AOCONFIG_ATTR[(int)AO::LAST_AO] = { "GR_Y", "GR_P0","GR_P1","GR_P2","GR_P3","GG0_X", "GG0_Y","GG0_P0", "GG0_P1","GG0_P2","GG0_P3","GG1_X","GG1_Y","GG1_P0","GG1_P1","GG1_P2", "GG1_P3", "Z", "RemoteFocus"};

long ThorDAQIOXML::GetAOLinesConfiguration(std::map<AO, long>& aoSelection)
{
	if (_xmlObj != NULL)
	{
		delete _xmlObj;
	}
	aoSelection = std::map<AO, long>();
	_xmlObj = new ticpp::Document(_currentPathAndFile);
	_xmlObj->LoadFile();

	// make sure the top level root element exist
	ticpp::Element* configObj = _xmlObj->FirstChildElement(false);

	if (configObj == NULL)
	{
		return FALSE;
	}
	else
	{
		try
		{
			ticpp::Iterator< ticpp::Element > child(configObj, AOCONFIG);

			for (child = child.begin(configObj); child != child.end(); child++)
			{
				for (UINT8 attIdx = 0; attIdx < (UINT8)AO::LAST_AO; attIdx++)
				{
					std::string str;
					child->GetAttribute(AOCONFIG_ATTR[attIdx], &str);
					std::stringstream ss(str);
					long val;

					if (str.empty() || std::all_of(str.begin(), str.end(), [](char c) {return std::isspace(c); }))
					{
						//if left empty or blank disable the output by setting it to -1
						val = -1;
					}
					else
					{
						ss >> val;

						if (val >= DAC_ANALOG_CHANNEL_COUNT)
						{
							//if outside of the available channels then the configuration is wrong and we disable the output by setting it to -1
							val = -1;
						}
					}

					aoSelection[(AO)attIdx] = val;
				}
			}
		}
		catch (ticpp::Exception ex)
		{
			return FALSE;
		}
	}
	return TRUE;
}