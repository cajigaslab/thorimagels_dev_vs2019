#include "xml_handler.h"
#include ".\include\tinyxml2.h"
#include <iostream>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <exception>
#include <minwindef.h>
#include <cmath>
#define XML_SCANF(buffer,format, ...)			\
	if(buffer==NULL)	throw new exception;		\
	sscanf_s(buffer,format,__VA_ARGS__);	

typedef region Region;

using namespace tinyxml2;
const char um[4] = { (char)194,(char)181,(char)109,(char)0 };

char* convertDouble(double value, char* str)
{
	char str2[STRING_BUFFER_SIZE] = { 0 };
	sprintf_s(str2, "%.3f", value);
	strcpy_s(str, STRING_BUFFER_SIZE, str2);
	return str;
}

PixelType convertPixelType(const char* type)
{
	if (strcmp(type, "uint16") == 0 || strcmp(type, "UINT16") == 0)
		return PixelType_UINT16;
	else	if (strcmp(type, "int16") == 0 || strcmp(type, "INT16") == 0)
		return PixelType_INT16;
	else if (strcmp(type, "uint8") == 0 || strcmp(type, "UINT8") == 0)
		return PixelType_UINT8;
	else
		return static_cast <PixelType>(-1);
}

const char* convertResUnit2String(ResUnit unit)
{
	//const char um[4] = { 194,181,109,0 };
	switch (unit)
	{
	case ResUnit::None:
		return "mm";
	case ResUnit::Inch:
		return "inch";
	case ResUnit::Centimeter:
		return "cm";
	case ResUnit::Millimetre:
		return "mm";
	case ResUnit::Micron:
		return "um";
	case ResUnit::Nanometer:
		return "nm";
	case ResUnit::Picometer:
		return "pm";
	default:
		return "mm";
	}
}

ResUnit convertString2ResUnit(const char* str)
{
	if (str == NULL) return ResUnit::Micron;
	//char um[4] = { 194,181,109,0 };
	if (memcmp(str, "inch", strlen(str)) == 0)
		return ResUnit::Inch;
	else if (memcmp(str, "cm", strlen(str)) == 0)
		return	ResUnit::Centimeter;
	else if (memcmp(str, "mm", strlen(str)) == 0)
		return ResUnit::Millimetre;
	else if (memcmp(str, "um", strlen(str)) == 0)
		return	ResUnit::Micron;
	else if (memcmp(str, "nm", strlen(str)) == 0)
		return	ResUnit::Nanometer;
	else if (memcmp(str, "pm", strlen(str)) == 0)
		return	ResUnit::Picometer;
	else
		return	ResUnit::None;
}

long xml_set_plate_info(char* plate_info, map<uint16_t, Plate*>* plates)
{
	tinyxml2::XMLDocument doc;
	if (doc.Parse(plate_info) != XMLError::XML_SUCCESS)
		return UNKNOWN_ERROR;
	try
	{
		const XMLElement* elementPlates = doc.FirstChildElement("Plates");
		if (elementPlates == NULL)	throw new exception("Plates");
		for (const XMLElement* elementPlate = elementPlates->FirstChildElement("Plate"); elementPlate; elementPlate = elementPlate->NextSiblingElement("Plate"))
		{
			Plate* plate = new Plate();
			XML_SCANF(elementPlate->Attribute("ID"), "Plate:%hd", &plate->PlateID);
			const char* name = elementPlate->Attribute("Name");
			strcpy_s(plate->Name, strlen(name) + 1, name);
			XML_SCANF(elementPlate->Attribute("Width"), "%lf", &plate->Width);
			XML_SCANF(elementPlate->Attribute("Height"), "%lf", &plate->Height);
			XML_SCANF(elementPlate->Attribute("PhysicalSizeXUnit"), "%d", &plate->PhysicalSizeXUnit);
			XML_SCANF(elementPlate->Attribute("PhysicalSizeYUnit"), "%d", &plate->PhysicalSizeYUnit);
			XML_SCANF(elementPlate->Attribute("Rows"), "%d", &plate->Rows);
			XML_SCANF(elementPlate->Attribute("Columns"), "%d", &plate->Columns);

			for (const XMLElement* elementWell = elementPlate->FirstChildElement("Well"); elementWell; elementWell = elementWell->NextSiblingElement("Well"))
			{
				Well* well = new Well();
				XML_SCANF(elementWell->Attribute("ID"), "Well:%hd.%hd",  &well->PlateID, &well->WellID);
				XML_SCANF(elementWell->Attribute("PositionX"), "%lf", &well->PositionX);
				XML_SCANF(elementWell->Attribute("PositionY"), "%lf", &well->PositionY);
				XML_SCANF(elementWell->Attribute("Width"), "%lf", &well->Width);
				XML_SCANF(elementWell->Attribute("Height"), "%lf", &well->Height);
				const char* shape = elementWell->Attribute("Shape");
				strcpy_s(well->Shape, strlen(shape) + 1, shape);
				XML_SCANF(elementWell->Attribute("Row"), "%d", &well->Row);
				XML_SCANF(elementWell->Attribute("Column"), "%d", &well->Column);
				plate->Wells.insert(make_pair(well->WellID, well));

				for (const XMLElement* elementWellSample = elementWell->FirstChildElement("WellSample"); elementWellSample; elementWellSample = elementWellSample->NextSiblingElement("WellSample"))
				{
					WellSample *wellSample = new WellSample();
					XML_SCANF(elementWellSample->Attribute("ID"), "WellSample:%hd.%hd.%hd", &wellSample->PlateID, &wellSample->WellID, &wellSample->WellSampleID);
					XML_SCANF(elementWellSample->Attribute("ImageID"), "%hd", &wellSample->ImageID);
					XML_SCANF(elementWellSample->Attribute("ScanID"), "%hd", &wellSample->ScanID);
					well->WellSamples.insert(make_pair(wellSample->WellSampleID, wellSample));

					for (const XMLElement* elementSampleRegion = elementWellSample->FirstChildElement("SampleRegion"); elementSampleRegion; elementSampleRegion = elementSampleRegion->NextSiblingElement("SampleRegion"))
					{
						SampleRegion *sampleRegion = new SampleRegion();
						XML_SCANF(elementSampleRegion->Attribute("ID"), "SampleRegion:%hd.%hd.%hd.%hd",  &wellSample->PlateID, &wellSample->WellID, &sampleRegion->WellSampleID, &sampleRegion->RegionID);
						XML_SCANF(elementSampleRegion->Attribute("PositionX"), "%lf", &sampleRegion->PositionX);
						XML_SCANF(elementSampleRegion->Attribute("PositionY"), "%lf", &sampleRegion->PositionY);
						XML_SCANF(elementSampleRegion->Attribute("PositionZ"), "%lf", &sampleRegion->PositionZ);
						XML_SCANF(elementSampleRegion->Attribute("SizeX"), "%lf", &sampleRegion->SizeX);
						XML_SCANF(elementSampleRegion->Attribute("SizeY"), "%lf", &sampleRegion->SizeY);
						XML_SCANF(elementSampleRegion->Attribute("SizeZ"), "%lf", &sampleRegion->SizeZ);
						XML_SCANF(elementSampleRegion->Attribute("PhysicalSizeXUnit"), "%hd", &sampleRegion->PhysicalSizeXUnit);
						XML_SCANF(elementSampleRegion->Attribute("PhysicalSizeYUnit"), "%hd", &sampleRegion->PhysicalSizeYUnit);
						XML_SCANF(elementSampleRegion->Attribute("PhysicalSizeZUnit"), "%hd", &sampleRegion->PhysicalSizeZUnit);
						wellSample->SampleRegions.insert(make_pair(sampleRegion->RegionID, sampleRegion));
					}
				}
			}
			(*plates).insert(make_pair(plate->PlateID, plate));
		}
		return SUCCESS;
	}
	catch (...)
	{
		return UNKNOWN_ERROR;
	}
}

long xml_add_scan_info(char* scan_info, map<uint16_t, Plate*> plates, Scan** scann)
{
	Scan* scan = *scann;
	if (scan == NULL) return FALSE;
	tinyxml2::XMLDocument doc;
	if (doc.Parse(scan_info) != XMLError::XML_SUCCESS)
		return FALSE;
	try
	{
		const XMLElement* elementScan = doc.FirstChildElement("Scan");
		if (elementScan == NULL)	throw new exception("Scan");
		XML_SCANF(elementScan->Attribute("ID"), "%hhd", &scan->ScanID);
		XML_SCANF(elementScan->Attribute("PlateID"), "%hd", &scan->PlateID);
		XML_SCANF(elementScan->Attribute("PhysicalSizeX"), "%lf", &scan->PhysicalSizeX);
		XML_SCANF(elementScan->Attribute("PhysicalSizeY"), "%lf", &scan->PhysicalSizeY);
		XML_SCANF(elementScan->Attribute("PhysicalSizeZ"), "%lf", &scan->PhysicalSizeZ);
		XML_SCANF(elementScan->Attribute("TimeIncrement"), "%hd", &scan->TimeIncrement);
		XML_SCANF(elementScan->Attribute("PhysicalSizeXUnit"), "%d", &scan->PhysicalSizeXUnit);
		XML_SCANF(elementScan->Attribute("PhysicalSizeYUnit"), "%d", &scan->PhysicalSizeYUnit);
		XML_SCANF(elementScan->Attribute("PhysicalSizeZUnit"), "%d", &scan->PhysicalSizeZUnit);
		const char* TimeIncrementUnit = elementScan->Attribute("TimeIncrementUnit");
		strcpy_s(scan->TimeIncrementUnit, strlen(TimeIncrementUnit) + 1, TimeIncrementUnit);
		const char* DimensionOrder = elementScan->Attribute("DimensionOrder");
		strcpy_s(scan->DimensionOrder, strlen(DimensionOrder) + 1, DimensionOrder);
		XML_SCANF(elementScan->Attribute("TileWidth"), "%hd", &scan->TileWidth);
		XML_SCANF(elementScan->Attribute("TileHeight"), "%hd", &scan->TileHeight);

		const char* type = elementScan->Attribute("Type");
		scan->Type = convertPixelType(type);
		XML_SCANF(elementScan->Attribute("SignificantBits"), "%hd", &scan->SignificantBits);
		const XMLElement* elementChannels = elementScan->FirstChildElement("Channels");
		if (elementChannels == NULL)	throw new exception("Channels");
		for (const XMLElement* elementChannel = elementChannels->FirstChildElement("Channel"); elementChannel; elementChannel = elementChannel->NextSiblingElement("Channel"))
		{
			Channel* channel = new Channel();
			XML_SCANF(elementChannel->Attribute("ID"), "%hd", &channel->ChannelID);
			XML_SCANF(elementChannel->Attribute("RefID"), "%hd", &channel->ChannelRefID);
			const char* name = elementChannel->Attribute("Name");
			strcpy_s(channel->Name, strlen(name) + 1, name);
			scan->Channels.insert(make_pair(channel->ChannelID, channel));
		}
		const XMLElement* elementScanRegions = elementScan->FirstChildElement("ScanRegions");
		if (elementScanRegions == NULL)	throw new exception("ScanRegions");
		for (const XMLElement* elementScanRegion = elementScanRegions->FirstChildElement("ScanRegion"); elementScanRegion; elementScanRegion = elementScanRegion->NextSiblingElement("ScanRegion"))
		{
			region* region = new Region();
			region->_parentScan = scan;
			region->ScanID = scan->ScanID;
			XML_SCANF(elementScanRegion->Attribute("ID"), "%hd", &region->RegionID);
			XML_SCANF(elementScanRegion->Attribute("SizeX"), "%d", &region->SizeX);
			XML_SCANF(elementScanRegion->Attribute("SizeY"), "%d", &region->SizeY);
			XML_SCANF(elementScanRegion->Attribute("SizeZ"), "%d", &region->SizeZ);
			XML_SCANF(elementScanRegion->Attribute("SizeT"), "%d", &region->SizeT);
			XML_SCANF(elementScanRegion->Attribute("SizeS"), "%d", &region->SizeS);
			region->Initialize();
			scan->Regions.insert(make_pair(region->RegionID, region));
			const XMLElement* elementSampleRegion = elementScanRegion->FirstChildElement("SampleRegion");
			if (elementSampleRegion == NULL)	throw new exception("SampleRegion");
			XML_SCANF(elementSampleRegion->Attribute("RegionID"), "RegionID:%hd.%hd.%hd.%hd", &region->PlateID, &region->WellID, &region->WellSampleID, &region->RegionID);
			Plate* plate = plates[region->PlateID];
			if (plate == NULL)	throw new exception("plate");
			Well* well = plate->Wells[region->WellID];
			if (well == NULL)	throw new exception("well");
			WellSample* wellSample = (well->WellSamples.find(region->WellSampleID) == well->WellSamples.end()) ? new WellSample() : well->WellSamples[region->WellSampleID];
			if (wellSample == NULL) throw new exception("WellSample");

			//if (well->WellSamples.size() == 0)
			//	wellSample->WellSampleID = 1;
			//else
			//	wellSample->WellSampleID = well->WellSamples.rbegin()->second->WellSampleID + 1;
			wellSample->PlateID = region->PlateID;
			wellSample->WellID = region->WellID;
			wellSample->ScanID = region->ScanID;

			SampleRegion* sampleRegion = new SampleRegion();
			sampleRegion->RegionID = region->RegionID;
			sampleRegion->WellSampleID = region->WellSampleID = wellSample->WellSampleID;
			XML_SCANF(elementSampleRegion->Attribute("PositionX"), "%lf", &sampleRegion->PositionX);
			XML_SCANF(elementSampleRegion->Attribute("PositionY"), "%lf", &sampleRegion->PositionY);
			XML_SCANF(elementSampleRegion->Attribute("PositionZ"), "%lf", &sampleRegion->PositionZ);
			XML_SCANF(elementSampleRegion->Attribute("SizeX"), "%lf", &sampleRegion->SizeX);
			XML_SCANF(elementSampleRegion->Attribute("SizeY"), "%lf", &sampleRegion->SizeY);
			XML_SCANF(elementSampleRegion->Attribute("SizeZ"), "%lf", &sampleRegion->SizeZ);
			sampleRegion->PhysicalSizeXUnit = scan->PhysicalSizeXUnit;
			sampleRegion->PhysicalSizeYUnit = scan->PhysicalSizeYUnit;
			sampleRegion->PhysicalSizeZUnit = scan->PhysicalSizeZUnit;
			if (wellSample->SampleRegions.find(region->RegionID) == wellSample->SampleRegions.end())
				wellSample->SampleRegions.insert(make_pair(region->RegionID, sampleRegion));

			if (well->WellSamples.find(region->WellSampleID) == well->WellSamples.end())
				well->WellSamples.insert(make_pair(wellSample->WellSampleID, wellSample));

		}
		return TRUE;
	}
	catch (exception&)
	{
		return FALSE;
	}
}

long xml_get_plate_info(map<uint16_t, Plate*> plates, char** xml)
{
	char tmp[STRING_BUFFER_SIZE] = { 0 };
	tinyxml2::XMLDocument doc;
	multimap<uint16_t, XMLElement*> map_PlateAcquisition;
	XMLElement *elementPlates = doc.NewElement("Plates");
	doc.InsertEndChild(elementPlates);

	for (map<uint16_t, Plate*>::iterator plateit = plates.begin(); plateit != plates.end(); plateit++)
	{
		XMLElement *elementPlate = doc.NewElement("Plate");
		elementPlates->InsertEndChild(elementPlate);
		memset(tmp, 0, STRING_BUFFER_SIZE);
		sprintf_s(tmp, "Plate:%hd", plateit->second->PlateID);
		elementPlate->SetAttribute("ID", tmp);
		elementPlate->SetAttribute("Name", plateit->second->Name);
		elementPlate->SetAttribute("Width", convertDouble(plateit->second->Width, tmp));
		elementPlate->SetAttribute("Height", convertDouble(plateit->second->Height, tmp));
		elementPlate->SetAttribute("PhysicalSizeXUnit", plateit->second->PhysicalSizeXUnit);
		elementPlate->SetAttribute("PhysicalSizeYUnit", plateit->second->PhysicalSizeYUnit);
		elementPlate->SetAttribute("Rows", plateit->second->Rows);
		elementPlate->SetAttribute("Columns", plateit->second->Columns);

		for (map<uint16_t, Well*>::iterator wellit = plateit->second->Wells.begin(); wellit != plateit->second->Wells.end(); wellit++)
		{
			XMLElement *elementWell = doc.NewElement("Well");
			elementPlate->InsertEndChild(elementWell);
			memset(tmp, 0, STRING_BUFFER_SIZE);
			sprintf_s(tmp, "Well:%hd.%hd", wellit->second->WellID, plateit->second->PlateID);
			elementWell->SetAttribute("ID", tmp);
			memset(tmp, 0, STRING_BUFFER_SIZE);

			elementWell->SetAttribute("PositionX", convertDouble(wellit->second->PositionX, tmp));
			elementWell->SetAttribute("PositionY", convertDouble(wellit->second->PositionY, tmp));
			elementWell->SetAttribute("Width", convertDouble(wellit->second->Width, tmp));
			elementWell->SetAttribute("Height", convertDouble(wellit->second->Height, tmp));
			elementWell->SetAttribute("Shape", wellit->second->Shape);
			elementWell->SetAttribute("Row", wellit->second->Row);
			elementWell->SetAttribute("Column", wellit->second->Column);

			for (map<uint16_t, WellSample*>::iterator wellSampleit = wellit->second->WellSamples.begin(); wellSampleit != wellit->second->WellSamples.end(); wellSampleit++)
			{
				XMLElement *elementWellSample = doc.NewElement("WellSample");
				elementWell->InsertEndChild(elementWellSample);
				memset(tmp, 0, STRING_BUFFER_SIZE);
				sprintf_s(tmp, "WellSample:%hd.%hd.%hd", wellSampleit->second->WellSampleID, wellit->second->WellID, plateit->second->PlateID);
				elementWellSample->SetAttribute("ID", tmp);
				memset(tmp, 0, STRING_BUFFER_SIZE);

				elementWellSample->SetAttribute("ScanID", wellSampleit->second->ScanID);
				elementWellSample->SetAttribute("ImageID", wellSampleit->second->ImageID);

				for (map<uint16_t, SampleRegion*>::iterator sampleRegionit = wellSampleit->second->SampleRegions.begin(); sampleRegionit != wellSampleit->second->SampleRegions.end(); sampleRegionit++)
				{
					XMLElement *elementSampleRegion = doc.NewElement("SampleRegion");
					elementWellSample->InsertEndChild(elementSampleRegion);
					memset(tmp, 0, STRING_BUFFER_SIZE);
					sprintf_s(tmp, "SampleRegion:%hd.%hd.%hd.%hd", sampleRegionit->second->RegionID, sampleRegionit->second->WellSampleID, wellit->second->WellID, plateit->second->PlateID);
					elementSampleRegion->SetAttribute("ID", tmp);
					memset(tmp, 0, STRING_BUFFER_SIZE);

					elementSampleRegion->SetAttribute("PositionX", convertDouble(sampleRegionit->second->PositionX, tmp));
					elementSampleRegion->SetAttribute("PositionY", convertDouble(sampleRegionit->second->PositionY, tmp));
					elementSampleRegion->SetAttribute("PositionZ", convertDouble(sampleRegionit->second->PositionZ, tmp));
					elementSampleRegion->SetAttribute("SizeX", convertDouble(sampleRegionit->second->SizeX, tmp));
					elementSampleRegion->SetAttribute("SizeY", convertDouble(sampleRegionit->second->SizeY, tmp));
					elementSampleRegion->SetAttribute("SizeZ", convertDouble(sampleRegionit->second->SizeZ, tmp));
					elementSampleRegion->SetAttribute("PhysicalSizeXUnit", sampleRegionit->second->PhysicalSizeXUnit);
					elementSampleRegion->SetAttribute("PhysicalSizeYUnit", sampleRegionit->second->PhysicalSizeYUnit);
					elementSampleRegion->SetAttribute("PhysicalSizeZUnit", sampleRegionit->second->PhysicalSizeZUnit);
				}
			}
		}
	}
	XMLPrinter printer;
	doc.Print(&printer);
	*xml = new char[printer.CStrSize()];
	memcpy_s(*xml, printer.CStrSize(), printer.CStr(), printer.CStrSize());
	return SUCCESS;
}

long xml_get_scan_infos(map<uint16_t, Plate*> plates, map<uint8_t, Scan*> scans, char** xml)
{
	char tmp[STRING_BUFFER_SIZE] = { 0 };
	tinyxml2::XMLDocument doc;
	try
	{
		XMLElement *elementScans = doc.NewElement("Scans");
		doc.InsertEndChild(elementScans);
		for (map<uint8_t, Scan*>::iterator scanit = scans.begin(); scanit != scans.end(); scanit++)
		{
			XMLElement *elementScan = doc.NewElement("Scan");
			elementScans->InsertEndChild(elementScan);
			elementScan->SetAttribute("ID", scanit->second->ScanID);
			elementScan->SetAttribute("PlateID", scanit->second->PlateID);
			elementScan->SetAttribute("PhysicalSizeX", convertDouble(scanit->second->PhysicalSizeX, tmp));
			elementScan->SetAttribute("PhysicalSizeY", convertDouble(scanit->second->PhysicalSizeY, tmp));
			elementScan->SetAttribute("PhysicalSizeZ", convertDouble(scanit->second->PhysicalSizeZ, tmp));
			elementScan->SetAttribute("PhysicalSizeXUnit", scanit->second->PhysicalSizeXUnit);
			elementScan->SetAttribute("PhysicalSizeYUnit", scanit->second->PhysicalSizeYUnit);
			elementScan->SetAttribute("PhysicalSizeZUnit", scanit->second->PhysicalSizeZUnit);
			elementScan->SetAttribute("TimeIncrement", scanit->second->TimeIncrement);
			elementScan->SetAttribute("TimeIncrementUnit", scanit->second->TimeIncrementUnit);
			elementScan->SetAttribute("DimensionOrder", scanit->second->DimensionOrder);
			elementScan->SetAttribute("TileWidth", scanit->second->TileWidth);
			elementScan->SetAttribute("TileHeight", scanit->second->TileHeight);
			elementScan->SetAttribute("ThumbnailTileSize", thumbnailTileSize);

			elementScan->SetAttribute("Type", PixelTypeString[scanit->second->Type].c_str());
			elementScan->SetAttribute("SignificantBits", scanit->second->SignificantBits);
			XMLElement *elementChannels = doc.NewElement("Channels");
			elementScan->InsertEndChild(elementChannels);
			for (map<uint16_t, Channel*>::iterator channelit = scanit->second->Channels.begin(); channelit != scanit->second->Channels.end(); channelit++)
			{
				XMLElement *elementChannel = doc.NewElement("Channel");
				elementChannels->InsertEndChild(elementChannel);
				elementChannel->SetAttribute("ID", channelit->second->ChannelID);
				elementChannel->SetAttribute("RefID", channelit->second->ChannelRefID);
				elementChannel->SetAttribute("Name", channelit->second->Name);
			}
			XMLElement *elementScanRegions = doc.NewElement("ScanRegions");
			elementScan->InsertEndChild(elementScanRegions);
			for (map<uint16_t, region*>::iterator regionit = scanit->second->Regions.begin(); regionit != scanit->second->Regions.end(); regionit++)
			{
				XMLElement *elementScanRegion = doc.NewElement("ScanRegion");
				elementScanRegions->InsertEndChild(elementScanRegion);
				elementScanRegion->SetAttribute("ID", regionit->second->RegionID);
				elementScanRegion->SetAttribute("SizeX", regionit->second->SizeX);
				elementScanRegion->SetAttribute("SizeY", regionit->second->SizeY);
				elementScanRegion->SetAttribute("SizeZ", regionit->second->SizeZ);
				elementScanRegion->SetAttribute("SizeT", regionit->second->SizeT);
				elementScanRegion->SetAttribute("SizeS", regionit->second->SizeS);
				elementScanRegion->SetAttribute("MaxScaleLevel", regionit->second->MaxScaleLevel);

				Plate* plate = plates[regionit->second->PlateID];
				if (plate == NULL)	throw new exception("plate");
				Well* well = plate->Wells[regionit->second->WellID];
				if (well == NULL)	throw new exception("well");
				WellSample* wellSample = well->WellSamples.find(regionit->second->WellSampleID) != well->WellSamples.end() ? well->WellSamples[regionit->second->WellSampleID] : NULL;
				if (wellSample == NULL)	throw new exception("wellSample");
				SampleRegion* sampleRegion = wellSample->SampleRegions.find(regionit->second->RegionID) != wellSample->SampleRegions.end() ? wellSample->SampleRegions[regionit->second->RegionID] : NULL;
				if (sampleRegion == NULL) continue;
				XMLElement *elementSampleRegion = doc.NewElement("SampleRegion");
				elementScanRegion->InsertEndChild(elementSampleRegion);
				memset(tmp, 0, STRING_BUFFER_SIZE);
				sprintf_s(tmp, "RegionID:%hd.%hd.%hd.%hd", regionit->second->PlateID, regionit->second->WellID, regionit->second->WellSampleID, regionit->second->RegionID);
				elementSampleRegion->SetAttribute("RegionID", tmp);
				elementSampleRegion->SetAttribute("PositionX", convertDouble(sampleRegion->PositionX, tmp));
				elementSampleRegion->SetAttribute("PositionY", convertDouble(sampleRegion->PositionY, tmp));
				elementSampleRegion->SetAttribute("PositionZ", convertDouble(sampleRegion->PositionZ, tmp));
				elementSampleRegion->SetAttribute("SizeX", convertDouble(sampleRegion->SizeX, tmp));
				elementSampleRegion->SetAttribute("SizeY", convertDouble(sampleRegion->SizeY, tmp));
				elementSampleRegion->SetAttribute("SizeZ", convertDouble(sampleRegion->SizeZ, tmp));
			}
		}
		XMLPrinter printer;
		doc.Print(&printer);
		*xml = new char[printer.CStrSize()];
		memcpy_s(*xml, printer.CStrSize(), printer.CStr(), printer.CStrSize());
		return SUCCESS;

	}
	catch (...)
	{
		return UNKNOWN_ERROR;
	}
}

XMLElement* createTiffDataElement(tinyxml2::XMLDocument* doc, image_container* pContainer, frame_info_ext frameExt) {
	XMLElement* elementTiffData = doc->NewElement("TiffData");
	elementTiffData->SetAttribute("FirstC", frameExt.frame.channel_id);
	elementTiffData->SetAttribute("FirstT", frameExt.frame.time_id - 1);
	elementTiffData->SetAttribute("FirstZ", frameExt.frame.z_id - 1);
	elementTiffData->SetAttribute("FirstS", frameExt.frame.s_id - 1);
	elementTiffData->SetAttribute("ContainerID", pContainer->_id);
	XMLElement* elementUUID = doc->NewElement("UUID");

	elementTiffData->SetAttribute("IFD", pContainer->_frameIfds[frameExt]);
	elementUUID->SetAttribute("FileName", pContainer->GetFileName().c_str());
	elementTiffData->InsertEndChild(elementUUID);
	return elementTiffData;
}

long xml_write_ome(map<uint16_t, Plate*> plates, map<uint8_t, Scan*> scans, char** xml, char* pre_file_name, bool is_create_thumbnailIFD)
{
	char tmp[STRING_BUFFER_SIZE] = { 0 };
	uint16_t imageID = 0;
	try
	{
		tinyxml2::XMLDocument doc;
		multimap<uint16_t, XMLElement*> map_PlateAcquisition;
		XMLElement *elementOME = doc.NewElement("OME");
		elementOME->SetAttribute("xmlns", "http://www.openmicroscopy.org/Schemas/OME/2016-06");
		elementOME->SetAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
		elementOME->SetAttribute("xsi:schemaLocation", "http://www.openmicroscopy.org/Schemas/OME/2016-06 http://www.openmicroscopy.org/Schemas/OME/2016-06/ome.xsd");

		doc.InsertEndChild(elementOME);
		XMLElement *elementMainImage = doc.NewElement("Image");
		elementOME->InsertEndChild(elementMainImage);
		elementMainImage->SetAttribute("ID", "Image:0");
		XMLElement *elementPixels = doc.NewElement("Pixels");
		elementMainImage->InsertEndChild(elementPixels);
		elementPixels->SetAttribute("ID", "Pixels:0");
		elementPixels->SetAttribute("DimensionOrder", "XYCZT");
		elementPixels->SetAttribute("Type", "uint8");
		elementPixels->SetAttribute("SizeC", 1);
		elementPixels->SetAttribute("SizeT", 1);
		elementPixels->SetAttribute("SizeX", 256);
		elementPixels->SetAttribute("SizeY", 128);
		elementPixels->SetAttribute("SizeZ", 1);
		elementPixels->SetAttribute("SizeS", 1);
		elementPixels->SetAttribute("TileWidth", 256);
		elementPixels->SetAttribute("TileHeight", 128);
		XMLElement* elementTiffData = doc.NewElement("TiffData");
		elementPixels->InsertEndChild(elementTiffData);
		elementTiffData->SetAttribute("FirstC", 0);
		elementTiffData->SetAttribute("FirstT", 0);
		elementTiffData->SetAttribute("FirstZ", 0);
		elementTiffData->SetAttribute("FirstS", 0);
		elementTiffData->SetAttribute("IFD", 0);
		XMLElement* elementUUID = doc.NewElement("UUID");
		memset(tmp, 0, STRING_BUFFER_SIZE);
		sprintf_s(tmp, "%s.tif", pre_file_name);
		elementUUID->SetAttribute("FileName", tmp);
		elementTiffData->InsertEndChild(elementUUID);
		imageID++;
		for (map<uint16_t, Plate*>::iterator plateit = plates.begin(); plateit != plates.end(); plateit++)
		{
			XMLElement *elementPlate = doc.NewElement("Plate");
			elementOME->InsertEndChild(elementPlate);
			memset(tmp, 0, STRING_BUFFER_SIZE);
			sprintf_s(tmp, "Plate:%hd", plateit->second->PlateID);
			elementPlate->SetAttribute("ID", tmp);
			elementPlate->SetAttribute("Name", plateit->second->Name);
			elementPlate->SetAttribute("Width", convertDouble(plateit->second->Width, tmp));
			elementPlate->SetAttribute("Height", convertDouble(plateit->second->Height, tmp));
			elementPlate->SetAttribute("PhysicalSizeXUnit", convertResUnit2String(plateit->second->PhysicalSizeXUnit));
			elementPlate->SetAttribute("PhysicalSizeYUnit", convertResUnit2String(plateit->second->PhysicalSizeYUnit));
			elementPlate->SetAttribute("Rows", plateit->second->Rows);
			elementPlate->SetAttribute("Columns", plateit->second->Columns);

			for (map<uint16_t, Well*>::iterator wellit = plateit->second->Wells.begin(); wellit != plateit->second->Wells.end(); wellit++)
			{
				XMLElement *elementWell = doc.NewElement("Well");
				elementPlate->InsertEndChild(elementWell);
				memset(tmp, 0, STRING_BUFFER_SIZE);
				sprintf_s(tmp, "Well:%hd.%hd", plateit->second->PlateID, wellit->second->WellID);
				elementWell->SetAttribute("ID", tmp);
				elementWell->SetAttribute("PositionX", convertDouble(wellit->second->PositionX, tmp));
				elementWell->SetAttribute("PositionY", convertDouble(wellit->second->PositionY, tmp));
				elementWell->SetAttribute("Width", convertDouble(wellit->second->Width, tmp));
				elementWell->SetAttribute("Height", convertDouble(wellit->second->Height, tmp));
				elementWell->SetAttribute("Shape", wellit->second->Shape);
				elementWell->SetAttribute("Row", wellit->second->Row);
				elementWell->SetAttribute("Column", wellit->second->Column);

				//uint16_t wellSampleID = 1;
				for (map<uint16_t, WellSample*>::iterator wellSampleit = wellit->second->WellSamples.begin(); wellSampleit != wellit->second->WellSamples.end(); wellSampleit++)
				{
					Scan* scan = scans[wellSampleit->second->ScanID];
					if (scan == NULL) throw new exception("scan");
					XMLElement *elementWellSample = doc.NewElement("WellSample");
					elementWell->InsertEndChild(elementWellSample);
					memset(tmp, 0, STRING_BUFFER_SIZE);
					sprintf_s(tmp, "WellSample:%hd.%hd.%hd", plateit->second->PlateID, wellit->second->WellID, wellSampleit->second->WellSampleID);	//wellSampleID
					elementWellSample->SetAttribute("ID", tmp);

					XMLElement *elementImageRef = doc.NewElement("ImageRef");
					elementWellSample->InsertEndChild(elementImageRef);
					memset(tmp, 0, STRING_BUFFER_SIZE);
					sprintf_s(tmp, "Image:%hd", imageID);
					elementImageRef->SetAttribute("ID", tmp);

					XMLElement *elementImage = doc.NewElement("Image");
					elementOME->InsertEndChild(elementImage);
					elementImage->SetAttribute("ID", tmp);

					for (map<uint16_t, SampleRegion*>::iterator sampleRegionit = wellSampleit->second->SampleRegions.begin(); sampleRegionit != wellSampleit->second->SampleRegions.end(); sampleRegionit++)
					{
						region* region = scan->Regions[sampleRegionit->second->RegionID];
						if (region == NULL) throw new exception("region");

						XMLElement *elementSampleRegion = doc.NewElement("Region");
						elementWellSample->InsertEndChild(elementSampleRegion);
						memset(tmp, 0, STRING_BUFFER_SIZE);
						sprintf_s(tmp, "Region:%hd.%hd.%hd.%hd", plateit->second->PlateID, wellit->second->WellID, sampleRegionit->second->WellSampleID, sampleRegionit->second->RegionID);
						elementSampleRegion->SetAttribute("ID", tmp);

						XMLElement *elementWellSampleRef = doc.NewElement("WellSampleRef");
						map_PlateAcquisition.insert(make_pair(scan->ScanID, elementWellSampleRef));
						memset(tmp, 0, STRING_BUFFER_SIZE);
						sprintf_s(tmp, "WellSample:%hd.%hd.%hd", plateit->second->PlateID, wellit->second->WellID, sampleRegionit->second->WellSampleID);
						elementWellSampleRef->SetAttribute("ID", tmp);
						elementWellSampleRef->SetAttribute("RegionID", region->RegionID);
						elementWellSampleRef->SetAttribute("PositionX", convertDouble(sampleRegionit->second->PositionX, tmp));
						elementWellSampleRef->SetAttribute("PositionY", convertDouble(sampleRegionit->second->PositionY, tmp));
						elementWellSampleRef->SetAttribute("PositionZ", convertDouble(sampleRegionit->second->PositionZ, tmp));
						elementWellSampleRef->SetAttribute("PositionXUnit", convertResUnit2String(sampleRegionit->second->PhysicalSizeXUnit));
						elementWellSampleRef->SetAttribute("PositionYUnit", convertResUnit2String(sampleRegionit->second->PhysicalSizeYUnit));
						elementWellSampleRef->SetAttribute("PositionZUnit", convertResUnit2String(sampleRegionit->second->PhysicalSizeZUnit));
						elementWellSampleRef->SetAttribute("SizeX", convertDouble(sampleRegionit->second->SizeX, tmp));
						elementWellSampleRef->SetAttribute("SizeY", convertDouble(sampleRegionit->second->SizeY, tmp));
						elementWellSampleRef->SetAttribute("SizeZ", convertDouble(sampleRegionit->second->SizeZ, tmp));

						XMLElement *elementPixels = doc.NewElement("Pixels");
						elementImage->InsertEndChild(elementPixels);
						elementPixels->SetAttribute("ID", "Pixels:0");
						elementPixels->SetAttribute("DimensionOrder", scan->DimensionOrder);
						elementPixels->SetAttribute("PhysicalSizeX", convertDouble(scan->PhysicalSizeX, tmp));
						elementPixels->SetAttribute("PhysicalSizeY", convertDouble(scan->PhysicalSizeY, tmp));
						elementPixels->SetAttribute("PhysicalSizeZ", convertDouble(scan->PhysicalSizeZ, tmp));
						elementPixels->SetAttribute("PhysicalSizeXUnit", convertResUnit2String(scan->PhysicalSizeXUnit));
						elementPixels->SetAttribute("PhysicalSizeYUnit", convertResUnit2String(scan->PhysicalSizeYUnit));
						elementPixels->SetAttribute("PhysicalSizeZUnit", convertResUnit2String(scan->PhysicalSizeZUnit));
						elementPixels->SetAttribute("TimeIncrement", scan->TimeIncrement);
						elementPixels->SetAttribute("TimeIncrementUnit", scan->TimeIncrementUnit);
						elementPixels->SetAttribute("Type", PixelTypeString[scan->Type].c_str());
						elementPixels->SetAttribute("SizeC", (int)scan->Channels.size());
						elementPixels->SetAttribute("TileWidth", scan->TileWidth);
						elementPixels->SetAttribute("TileHeight", scan->TileHeight);
						elementPixels->SetAttribute("SignificantBits", scan->SignificantBits);
						elementPixels->SetAttribute("SizeT", region->SizeT);
						elementPixels->SetAttribute("SizeX", region->SizeX);
						elementPixels->SetAttribute("SizeY", region->SizeY);
						elementPixels->SetAttribute("SizeZ", region->SizeZ);
						elementPixels->SetAttribute("SizeS", region->SizeS);
						elementPixels->SetAttribute("MaxScaleLevel", region->MaxScaleLevel);

						map<uint16_t, image_container*>* rawContainers = &region->RawContainers;
						for(map<uint16_t, image_container*>::iterator containerIt = (*rawContainers).begin();containerIt!=(*rawContainers).end();containerIt++)
						{
							auto pContainer = (*containerIt).second;
							for(map<frame_info_ext, uint16_t>::iterator frameIt = pContainer->_frameIfds.begin();frameIt!= pContainer->_frameIfds.end();frameIt++)
							{
								auto elementTiffData = createTiffDataElement(&doc, pContainer, (*frameIt).first);
								elementPixels->InsertEndChild(elementTiffData);
							}
						}
					}

					for (map<uint16_t, Channel*>::iterator channelit = scan->Channels.begin(); channelit != scan->Channels.end(); channelit++)
					{
						XMLElement *elementChannel = doc.NewElement("Channel");
						elementPixels->InsertEndChild(elementChannel);
						memset(tmp, 0, STRING_BUFFER_SIZE);
						sprintf_s(tmp, "Channel:%hd", channelit->second->ChannelID);
						elementChannel->SetAttribute("ID", tmp);
						sprintf_s(tmp, "%hd", channelit->second->ChannelRefID);
						elementChannel->SetAttribute("RefID", tmp);
						elementChannel->SetAttribute("Name", channelit->second->Name);
						elementChannel->SetAttribute("SamplesPerPixel", 1);
					}

					imageID++;
					//wellSampleID++;
				}
			}
			for (map<uint8_t, Scan*>::iterator scanit = scans.begin(); scanit != scans.end(); scanit++)
			{
				XMLElement *elementPlateAcquisition = doc.NewElement("PlateAcquisition");
				elementPlate->InsertEndChild(elementPlateAcquisition);
				memset(tmp, 0, STRING_BUFFER_SIZE);
				sprintf_s(tmp, "PlateAcquisition:%hd.%hhd", plateit->second->PlateID, scanit->second->ScanID);
				elementPlateAcquisition->SetAttribute("ID", tmp);
				for (multimap<uint16_t, XMLElement*>::iterator plateAcquisitionit = map_PlateAcquisition.find(scanit->second->ScanID); plateAcquisitionit != map_PlateAcquisition.end(); plateAcquisitionit++)
				{
					elementPlateAcquisition->InsertEndChild(plateAcquisitionit->second);
				}
			}
		}
		if (is_create_thumbnailIFD)
		{
			XMLElement* elementPyramidals = doc.NewElement("Pyramidals");
			elementPyramidals->SetAttribute("ThumbnailTileSize", thumbnailTileSize);
			elementOME->InsertEndChild(elementPyramidals);
			for each (double scale in pyramidal_scale_list)
			{
				XMLElement* elementPyramidal = doc.NewElement("Pyramidal");
				memset(tmp, 0, STRING_BUFFER_SIZE);
				sprintf_s(tmp, "%f", scale);
				elementPyramidal->SetAttribute("Scale", tmp);
				bool hasChildren = false;
				for (map<uint8_t, Scan*>::iterator scanit = scans.begin(); scanit != scans.end(); scanit++)
				{
					for (map<uint16_t, region*>::iterator regionit = scanit->second->Regions.begin(); regionit != scanit->second->Regions.end(); regionit++)
					{
						uint32_t pyramidalWidth, subPyamidalWidth, subPyamidalCountX, pyramidalHeight, subPyamidalHeight, subPyamidalCountY;
						bool isPyramidalX = region::GetPyramidalLength(regionit->second->SizeX, scale, &pyramidalWidth, &subPyamidalWidth, &subPyamidalCountX);
						bool isPyramidalY = region::GetPyramidalLength(regionit->second->SizeY, scale, &pyramidalHeight, &subPyamidalHeight, &subPyamidalCountY);
						if (!isPyramidalX && !isPyramidalY && scale != 1)
							continue;
						XMLElement* elementImage = doc.NewElement("Image");
						elementImage->SetAttribute("ScanID", scanit->second->ScanID);
						elementImage->SetAttribute("RegionID", regionit->second->RegionID);


						image_container *pContainer = regionit->second->ThumbnailContainers.begin()->second;

						for (map<uint16_t, Channel*>::iterator channelit = scanit->second->Channels.begin(); channelit != scanit->second->Channels.end(); channelit++)
						{
							for (uint32_t s = 0; s < regionit->second->SizeS; s++)
							{
								for (uint32_t t = 0; t < regionit->second->SizeT; t++)
								{
									for (uint32_t z = 0; z < regionit->second->SizeZ; z++)
									{
										frame_info frame;
										frame.scan_id = scanit->second->ScanID;
										frame.region_id = regionit->second->RegionID;
										frame.channel_id = channelit->second->ChannelID;
										frame.s_id = s + 1;   // ThorAnalysis start from 1; ome-tiff start from 0
										frame.time_id = t + 1;
										frame.z_id = z + 1;
										frame_info_ext scaledFrame(frame);
										scaledFrame.scaleLevel = (uint16_t)(-log(scale) / log(2));
										pContainer = regionit->second->findImageContainer(scaledFrame, true);

										XMLElement* elementTiffData = createTiffDataElement(&doc, pContainer, scaledFrame);

										elementImage->InsertEndChild(elementTiffData);
										elementPyramidal->InsertEndChild(elementImage);
										hasChildren = true;
									}
								}
							}
						}
					}
				}
				if (hasChildren)
				{
					elementPyramidals->InsertEndChild(elementPyramidal);
				}
			}
		}
		XMLPrinter printer;
		doc.Print(&printer);
		*xml = new char[printer.CStrSize()];
		memcpy_s(*xml, printer.CStrSize(), printer.CStr(), printer.CStrSize());
		return SUCCESS;

	}
	catch (...)
	{
		return UNKNOWN_ERROR;
	}
}

long xml_read_ome(char* xml, map<uint16_t, Plate*>* plates, map<uint8_t, Scan*>* scans, TiffData* parent_tiff, OpenMode openMode)
{
	map<int, WellSample*> map_image;
	tinyxml2::XMLDocument doc;
	if (doc.Parse(xml) != XMLError::XML_SUCCESS)
		return UNKNOWN_ERROR;
	try
	{
		const XMLElement* elementOME = doc.FirstChildElement("OME");
		if (elementOME == NULL)	throw new exception("OME");
		for (const XMLElement* elementPlate = elementOME->FirstChildElement("Plate"); elementPlate; elementPlate = elementPlate->NextSiblingElement("Plate"))
		{
			Plate* plate = new Plate;
			XML_SCANF(elementPlate->Attribute("ID"), "Plate:%hd", &plate->PlateID);
			(*plates).insert(make_pair(plate->PlateID, plate));
			const char* name = elementPlate->Attribute("Name");
			strcpy_s(plate->Name, strlen(name) + 1, name);
			XML_SCANF(elementPlate->Attribute("Width"), "%lf", &plate->Width);
			XML_SCANF(elementPlate->Attribute("Height"), "%lf", &plate->Height);
			plate->PhysicalSizeXUnit = (ResUnit)convertString2ResUnit(elementPlate->Attribute("PhysicalSizeXUnit"));
			plate->PhysicalSizeYUnit = (ResUnit)convertString2ResUnit(elementPlate->Attribute("PhysicalSizeYUnit"));
			XML_SCANF(elementPlate->Attribute("Rows"), "%d", &plate->Rows);
			XML_SCANF(elementPlate->Attribute("Columns"), "%d", &plate->Columns);


			for (const XMLElement* elementWell = elementPlate->FirstChildElement("Well"); elementWell; elementWell = elementWell->NextSiblingElement("Well"))
			{
				Well* well = new Well();
				XML_SCANF(elementWell->Attribute("ID"), "Well:%hd.%hd", &plate->PlateID, &well->WellID);
				XML_SCANF(elementWell->Attribute("PositionX"), "%lf", &well->PositionX);
				XML_SCANF(elementWell->Attribute("PositionY"), "%lf", &well->PositionY);
				XML_SCANF(elementWell->Attribute("Width"), "%lf", &well->Width);
				XML_SCANF(elementWell->Attribute("Height"), "%lf", &well->Height);
				const char* shape = elementWell->Attribute("Shape");
				strcpy_s(well->Shape, strlen(shape) + 1, shape);
				XML_SCANF(elementWell->Attribute("Row"), "%d", &well->Row);
				XML_SCANF(elementWell->Attribute("Column"), "%d", &well->Column);

				plate->Wells.insert(make_pair(well->WellID, well));

				for (const XMLElement* elementWellSample = elementWell->FirstChildElement("WellSample"); elementWellSample; elementWellSample = elementWellSample->NextSiblingElement("WellSample"))
				{
					WellSample* wellSample = new WellSample;
					wellSample->PlateID = plate->PlateID;
					wellSample->WellID = well->WellID;
					//wellSample->ScanID = scanRegion->ScanID;
					//wellSample->RegionID = scanRegion->ScanRegionID;
					XML_SCANF(elementWellSample->Attribute("ID"), "WellSample:%hd.%hd.%hd", &plate->PlateID, &well->WellID, &wellSample->WellSampleID);
					const XMLElement* elementImageRef = elementWellSample->FirstChildElement("ImageRef");
					if (elementImageRef == NULL)	throw new exception("ImageRef");
					XML_SCANF(elementImageRef->Attribute("ID"), "Image:%hd", &wellSample->ImageID);
					map_image.insert(make_pair(wellSample->ImageID, wellSample));
					well->WellSamples.insert(make_pair(wellSample->WellSampleID, wellSample));
					for (const XMLElement* elementRegion = elementWellSample->FirstChildElement("Region"); elementRegion; elementRegion = elementRegion->NextSiblingElement("Region"))
					{
						SampleRegion* sampleRegion = new SampleRegion();
						sampleRegion->WellSampleID = wellSample->WellSampleID;
						XML_SCANF(elementRegion->Attribute("ID"), "Region:%hd.%hd.%hd.%hd", &plate->PlateID, &well->WellID, &wellSample->WellSampleID, &sampleRegion->RegionID);
						wellSample->SampleRegions.insert(make_pair(sampleRegion->RegionID, sampleRegion));
					}
				}
			}
			for (const XMLElement* elementPlateAcquisition = elementPlate->FirstChildElement("PlateAcquisition"); elementPlateAcquisition; elementPlateAcquisition = elementPlateAcquisition->NextSiblingElement("PlateAcquisition"))
			{
				Scan* scan = new Scan(parent_tiff);
				XML_SCANF(elementPlateAcquisition->Attribute("ID"), "PlateAcquisition:%hd.%hhd", &scan->PlateID, &scan->ScanID);
				(*scans).insert(make_pair(scan->ScanID, scan));
				for (const XMLElement* elementWellSampleRef = elementPlateAcquisition->FirstChildElement("WellSampleRef"); elementWellSampleRef; elementWellSampleRef = elementWellSampleRef->NextSiblingElement("WellSampleRef"))
				{
					region* region = new Region;
					XML_SCANF(elementWellSampleRef->Attribute("ID"), "WellSample:%hd.%hd.%hd", &region->PlateID, &region->WellID, &region->WellSampleID);
					XML_SCANF(elementWellSampleRef->Attribute("RegionID"), "%hd", &region->RegionID);
					Well* well = plate->Wells[region->WellID];
					if (well == NULL) throw new exception("well");
					WellSample* wellSample = well->WellSamples[region->WellSampleID];
					if (wellSample == NULL) throw new exception("wellSample");
					wellSample->ScanID = scan->ScanID;
					SampleRegion* sampleRegion = new SampleRegion();
					sampleRegion->RegionID = region->RegionID;
					sampleRegion->WellSampleID = wellSample->WellSampleID;
					XML_SCANF(elementWellSampleRef->Attribute("PositionX"), "%lf", &sampleRegion->PositionX);
					XML_SCANF(elementWellSampleRef->Attribute("PositionY"), "%lf", &sampleRegion->PositionY);
					XML_SCANF(elementWellSampleRef->Attribute("PositionZ"), "%lf", &sampleRegion->PositionZ);
					sampleRegion->PhysicalSizeXUnit = (ResUnit)convertString2ResUnit(elementWellSampleRef->Attribute("PositionXUnit"));
					sampleRegion->PhysicalSizeYUnit = (ResUnit)convertString2ResUnit(elementWellSampleRef->Attribute("PositionYUnit"));
					sampleRegion->PhysicalSizeZUnit = (ResUnit)convertString2ResUnit(elementWellSampleRef->Attribute("PositionZUnit"));
					XML_SCANF(elementWellSampleRef->Attribute("SizeX"), "%lf", &sampleRegion->SizeX);
					XML_SCANF(elementWellSampleRef->Attribute("SizeY"), "%lf", &sampleRegion->SizeY);
					XML_SCANF(elementWellSampleRef->Attribute("SizeZ"), "%lf", &sampleRegion->SizeZ);
					region->_parentScan = scan;
					scan->Regions.insert(make_pair(region->RegionID, region));
					map_image[wellSample->ImageID]->SampleRegions.insert(make_pair(region->RegionID, sampleRegion));
				}
			}
		}
		map<uint16_t, Channel*> Channels;
		for (const XMLElement* elementImage = elementOME->FirstChildElement("Image"); elementImage; elementImage = elementImage->NextSiblingElement("Image"))
		{
			uint16_t image_id = 0;
			XML_SCANF(elementImage->Attribute("ID"), "Image:%hd", &image_id);
			const XMLElement* elementPixels = elementImage->FirstChildElement("Pixels");
			//get scan channels
			if (image_id == 0 && elementPixels != NULL)
			{
				for (const XMLElement* elementChannel = elementPixels->FirstChildElement("Channel"); elementChannel; elementChannel = elementChannel->NextSiblingElement("Channel"))
				{
					Channel* channel = new Channel;
					XML_SCANF(elementChannel->Attribute("ID"), "Channel:%hd", &channel->ChannelID);
					XML_SCANF(elementChannel->Attribute("RefID"), "%hd", &channel->ChannelRefID);
					const char* name = elementChannel->Attribute("Name");
					strcpy_s(channel->Name, strlen(name) + 1, name);
					Channels.insert(make_pair(channel->ChannelID, channel));
				}
				continue;
			}

			WellSample* wellSample = map_image[image_id];
			if (wellSample == NULL) throw new exception("wellSample");
			Scan* scan = (*scans)[wellSample->ScanID];
			if (scan == NULL) throw new exception("scan");
			scan->Channels = Channels;
			for (map<uint16_t, SampleRegion*>::iterator sampleRegionit = wellSample->SampleRegions.begin(); sampleRegionit != wellSample->SampleRegions.end(); sampleRegionit++)
			{
				if (elementPixels == NULL)	continue;

				region* sregion = (scan->Regions.find(sampleRegionit->first) != scan->Regions.end()) ? scan->Regions[sampleRegionit->first] : NULL;
				if (sregion == NULL) 
					continue;

				XML_SCANF(elementPixels->Attribute("SizeS"), "%d", &sregion->SizeS);
				XML_SCANF(elementPixels->Attribute("SizeT"), "%d", &sregion->SizeT);
				XML_SCANF(elementPixels->Attribute("SizeX"), "%d", &sregion->SizeX);
				XML_SCANF(elementPixels->Attribute("SizeY"), "%d", &sregion->SizeY);
				XML_SCANF(elementPixels->Attribute("SizeZ"), "%d", &sregion->SizeZ);
				sregion->Initialize();
				XML_SCANF(elementPixels->Attribute("MaxScaleLevel"), "%hd", &sregion->MaxScaleLevel);

				if (scan->PhysicalSizeX == 0)
				{
					XML_SCANF(elementPixels->Attribute("PhysicalSizeX"), "%lf", &scan->PhysicalSizeX);
					XML_SCANF(elementPixels->Attribute("PhysicalSizeY"), "%lf", &scan->PhysicalSizeY);
					XML_SCANF(elementPixels->Attribute("PhysicalSizeZ"), "%lf", &scan->PhysicalSizeZ);
					XML_SCANF(elementPixels->Attribute("TimeIncrement"), "%hd", &scan->TimeIncrement);
					scan->PhysicalSizeXUnit = (ResUnit)convertString2ResUnit(elementPixels->Attribute("PhysicalSizeXUnit"));
					scan->PhysicalSizeYUnit = (ResUnit)convertString2ResUnit(elementPixels->Attribute("PhysicalSizeYUnit"));
					scan->PhysicalSizeZUnit = (ResUnit)convertString2ResUnit(elementPixels->Attribute("PhysicalSizeZUnit"));
					const char* TimeIncrementUnit = elementPixels->Attribute("TimeIncrementUnit");
					strcpy_s(scan->TimeIncrementUnit, strlen(TimeIncrementUnit) + 1, TimeIncrementUnit);
					const char* DimensionOrder = elementPixels->Attribute("DimensionOrder");
					strcpy_s(scan->DimensionOrder, strlen(DimensionOrder) + 1, DimensionOrder);
					XML_SCANF(elementPixels->Attribute("TileWidth"), "%hd", &scan->TileWidth);
					XML_SCANF(elementPixels->Attribute("TileHeight"), "%hd", &scan->TileHeight);
					const char* type = elementPixels->Attribute("Type");
					scan->Type = convertPixelType(type);
					XML_SCANF(elementPixels->Attribute("SignificantBits"), "%hd", &scan->SignificantBits);

					for (const XMLElement* elementChannel = elementPixels->FirstChildElement("Channel"); elementChannel; elementChannel = elementChannel->NextSiblingElement("Channel"))
					{
						Channel* channel = new Channel;
						XML_SCANF(elementChannel->Attribute("ID"), "Channel:%hd", &channel->ChannelID);
						XML_SCANF(elementChannel->Attribute("RefID"), "%hd", &channel->ChannelRefID);
						const char* name = elementChannel->Attribute("Name");
						strcpy_s(channel->Name, strlen(name) + 1, name);
						scan->Channels.insert(make_pair(channel->ChannelID, channel));
					}
				}
				for (const XMLElement* elementTiffData = elementPixels->FirstChildElement("TiffData"); elementTiffData; elementTiffData = elementTiffData->NextSiblingElement("TiffData"))
				{
					frame_info frame;
					frame.scan_id = scan->ScanID;
					frame.region_id = sregion->RegionID;
					frame.channel_id = atoi(elementTiffData->Attribute("FirstC"));
					frame.time_id = atoi(elementTiffData->Attribute("FirstT")) + 1;
					frame.z_id = atoi(elementTiffData->Attribute("FirstZ")) + 1;
					frame.s_id = atoi(elementTiffData->Attribute("FirstS")) + 1;
					int ifd = atoi(elementTiffData->Attribute("IFD"));
					uint16_t containerId=0;
					if(elementTiffData->Attribute("ContainerID") != NULL)
						containerId = atoi(elementTiffData->Attribute("ContainerID"));
					const XMLElement* elementUUID = elementTiffData->FirstChildElement("UUID");
					auto fileName = elementUUID->Attribute("FileName");

					image_container* pContainer;
					auto containerIt = sregion->RawContainers.find(containerId);
					if (containerIt == sregion->RawContainers.end()) {
						pContainer = new image_container(containerId, false, sregion, openMode, fileName);
						sregion->RawContainers.insert(make_pair(containerId, pContainer));
					}
					else {
						pContainer = containerIt->second;
					}
					pContainer->_frameIfds.insert(make_pair(frame_info_ext(frame), ifd));
				}
				elementPixels = elementPixels->NextSiblingElement("Pixels");
			}
		}
		const XMLElement* elementPyramidals = elementOME->FirstChildElement("Pyramidals");
		if (elementPyramidals == NULL)	return SUCCESS;//throw new exception("Pyramidals");
		XML_SCANF(elementPyramidals->Attribute("ThumbnailTileSize"), "%hd", &thumbnailTileSize);

		for (const XMLElement* elementPyramidal = elementPyramidals->FirstChildElement("Pyramidal"); elementPyramidal; elementPyramidal = elementPyramidal->NextSiblingElement("Pyramidal"))
		{
			double scale;
			uint16_t scanID;
			uint16_t regionID;
			XML_SCANF(elementPyramidal->Attribute("Scale"), "%lf", &scale);
			for (const XMLElement* elementImage = elementPyramidal->FirstChildElement("Image"); elementImage; elementImage = elementImage->NextSiblingElement("Image"))
			{
				XML_SCANF(elementImage->Attribute("ScanID"), "%hd", &scanID);
				XML_SCANF(elementImage->Attribute("RegionID"), "%hd", &regionID);
				Scan* scan = scans->at((unsigned char)scanID);
				region* region = scan->Regions[regionID];
				for (const XMLElement* elementTiffData = elementImage->FirstChildElement("TiffData"); elementTiffData; elementTiffData = elementTiffData->NextSiblingElement("TiffData"))
				{
					frame_info frame;
					frame.scan_id = static_cast<uint8_t>(scanID);
					frame.region_id = regionID;
					frame.channel_id = atoi(elementTiffData->Attribute("FirstC"));
					frame.time_id = atoi(elementTiffData->Attribute("FirstT")) + 1;
					frame.z_id = atoi(elementTiffData->Attribute("FirstZ")) + 1;
					frame.s_id = atoi(elementTiffData->Attribute("FirstS")) + 1;
					int ifd = atoi(elementTiffData->Attribute("IFD"));
					uint16_t containerId = 0;
					if (elementTiffData->Attribute("ContainerID") != NULL)
						containerId = atoi(elementTiffData->Attribute("ContainerID"));
					const XMLElement* elementUUID = elementTiffData->FirstChildElement("UUID");
					if (elementUUID != NULL)
					{
						auto fileName = elementUUID->Attribute("FileName");
						if (fileName == nullptr)continue;
					}
					else
						continue;

					image_container* pContainer;
					auto containerIt = region->ThumbnailContainers.find(containerId);
					if (containerIt == region->ThumbnailContainers.end()) {
						pContainer = new image_container(containerId, true, region, openMode);
						region->ThumbnailContainers.insert(make_pair(containerId, pContainer));
					}
					else {
						pContainer = containerIt->second;
					}
					frame_info_ext scaledFrame(frame);
					scaledFrame.scaleLevel = (uint16_t)(-log(scale) / log(2));
					pContainer->_frameIfds.insert(make_pair(scaledFrame, ifd));
				}
			}
		}
		return SUCCESS;
	}
	catch (...)
	{
		return UNKNOWN_ERROR;
	}
}

///**************************		XML FILE		**************************///

long ResolvePlatesXML(char* plates_xml, std::map<uint16_t, Plate*>* plates)
{
	tinyxml2::XMLDocument doc;
	if (doc.Parse(plates_xml) != XMLError::XML_SUCCESS)
		return 0;
	try
	{
		const XMLElement* elementPlates = doc.FirstChildElement("Plates");
		if (elementPlates == NULL)	throw new std::exception("Plates");
		for (const XMLElement* elementPlate = elementPlates->FirstChildElement("Plate"); elementPlate; elementPlate = elementPlate->NextSiblingElement("Plate"))
		{
			Plate* plate = new Plate();
			XML_SCANF(elementPlate->Attribute("ID"), "Plate:%hd", &plate->PlateID);
			const char* name = elementPlate->Attribute("Name");
			strcpy_s(plate->Name, strlen(name) + 1, name);
			XML_SCANF(elementPlate->Attribute("Width"), "%lf", &plate->Width);
			XML_SCANF(elementPlate->Attribute("Height"), "%lf", &plate->Height);
			XML_SCANF(elementPlate->Attribute("PhysicalSizeXUnit"), "%d", &plate->PhysicalSizeXUnit);
			XML_SCANF(elementPlate->Attribute("PhysicalSizeYUnit"), "%d", &plate->PhysicalSizeYUnit);
			XML_SCANF(elementPlate->Attribute("Rows"), "%d", &plate->Rows);
			XML_SCANF(elementPlate->Attribute("Columns"), "%d", &plate->Columns);
			for (const XMLElement* elementWell = elementPlate->FirstChildElement("Well"); elementWell; elementWell = elementWell->NextSiblingElement("Well"))
			{
				Well* well = new Well();
				XML_SCANF(elementWell->Attribute("ID"), "Well:%hd.%hd", &well->PlateID, &well->WellID);
				XML_SCANF(elementWell->Attribute("PositionX"), "%lf", &well->PositionX);
				XML_SCANF(elementWell->Attribute("PositionY"), "%lf", &well->PositionY);
				XML_SCANF(elementWell->Attribute("Width"), "%lf", &well->Width);
				XML_SCANF(elementWell->Attribute("Height"), "%lf", &well->Height);
				const char* shape = elementWell->Attribute("Shape");
				strcpy_s(well->Shape, strlen(shape) + 1, shape);
				XML_SCANF(elementWell->Attribute("Row"), "%d", &well->Row);
				XML_SCANF(elementWell->Attribute("Column"), "%d", &well->Column);
				for (const XMLElement* elementWellSample = elementWell->FirstChildElement("WellSample"); elementWellSample; elementWellSample = elementWellSample->NextSiblingElement("WellSample"))
				{
					WellSample* wellSample = new WellSample();
					XML_SCANF(elementWellSample->Attribute("ID"), "WellSample:%hd.%hd.%hd", &wellSample->PlateID, &wellSample->WellID, &wellSample->WellSampleID);
					XML_SCANF(elementWellSample->Attribute("ScanID"), "%hd", &wellSample->ScanID);
					XML_SCANF(elementWellSample->Attribute("ImageID"), "%hd", &wellSample->ImageID);
					for (const XMLElement* elementSampleRegion = elementWellSample->FirstChildElement("SampleRegion"); elementSampleRegion; elementSampleRegion = elementSampleRegion->NextSiblingElement("SampleRegion"))
					{
						SampleRegion* sampleRegion = new SampleRegion();
						XML_SCANF(elementSampleRegion->Attribute("ID"), "SampleRegion:%hd.%hd.%hd.%hd", &wellSample->PlateID, &wellSample->WellID, &sampleRegion->WellSampleID, &sampleRegion->RegionID);
						XML_SCANF(elementSampleRegion->Attribute("PositionX"), "%lf", &sampleRegion->PositionX);
						XML_SCANF(elementSampleRegion->Attribute("PositionY"), "%lf", &sampleRegion->PositionY);
						XML_SCANF(elementSampleRegion->Attribute("PositionZ"), "%lf", &sampleRegion->PositionZ);
						XML_SCANF(elementSampleRegion->Attribute("SizeX"), "%lf", &sampleRegion->SizeX);
						XML_SCANF(elementSampleRegion->Attribute("SizeY"), "%lf", &sampleRegion->SizeY);
						XML_SCANF(elementSampleRegion->Attribute("SizeZ"), "%lf", &sampleRegion->SizeZ);
						XML_SCANF(elementSampleRegion->Attribute("PhysicalSizeXUnit"), "%d", &sampleRegion->PhysicalSizeXUnit);
						XML_SCANF(elementSampleRegion->Attribute("PhysicalSizeYUnit"), "%d", &sampleRegion->PhysicalSizeYUnit);
						XML_SCANF(elementSampleRegion->Attribute("PhysicalSizeZUnit"), "%d", &sampleRegion->PhysicalSizeZUnit);
					}
				}
				plate->Wells.insert(std::make_pair(well->WellID, well));
			}
			(*plates).insert(std::make_pair(plate->PlateID, plate));
		}
		return 1;
	}
	catch (...)
	{
		return 0;
	}

}

long ResolveScansXML(char* scans_xml, std::map<uint16_t, Plate*>* plates, std::map<uint8_t, Scan*>* scans)
{
	tinyxml2::XMLDocument doc;
	if (doc.Parse(scans_xml) != XMLError::XML_SUCCESS)
		return NULL;
	try
	{
		const XMLElement* elementScans = doc.FirstChildElement("Scans");
		if (elementScans == NULL)	throw new std::exception("Scans");
		for (const XMLElement* elementScan = elementScans->FirstChildElement("Scan"); elementScan; elementScan = elementScan->NextSiblingElement("Scan"))
		{
			Scan* scan = new Scan();
			XML_SCANF(elementScan->Attribute("ID"), "%hhd", &scan->ScanID);
			XML_SCANF(elementScan->Attribute("PlateID"), "%hd", &scan->PlateID);
			XML_SCANF(elementScan->Attribute("PhysicalSizeX"), "%lf", &scan->PhysicalSizeX);
			XML_SCANF(elementScan->Attribute("PhysicalSizeY"), "%lf", &scan->PhysicalSizeY);
			XML_SCANF(elementScan->Attribute("PhysicalSizeZ"), "%lf", &scan->PhysicalSizeZ);
			XML_SCANF(elementScan->Attribute("TimeIncrement"), "%hd", &scan->TimeIncrement);
			XML_SCANF(elementScan->Attribute("PhysicalSizeXUnit"), "%d", &scan->PhysicalSizeXUnit);
			XML_SCANF(elementScan->Attribute("PhysicalSizeYUnit"), "%d", &scan->PhysicalSizeYUnit);
			XML_SCANF(elementScan->Attribute("PhysicalSizeZUnit"), "%d", &scan->PhysicalSizeZUnit);
			const char* TimeIncrementUnit = elementScan->Attribute("TimeIncrementUnit");
			strcpy_s(scan->TimeIncrementUnit, strlen(TimeIncrementUnit) + 1, TimeIncrementUnit);
			const char* DimensionOrder = elementScan->Attribute("DimensionOrder");
			strcpy_s(scan->DimensionOrder, strlen(DimensionOrder) + 1, DimensionOrder);
			XML_SCANF(elementScan->Attribute("TileWidth"), "%hd", &scan->TileWidth);
			XML_SCANF(elementScan->Attribute("TileHeight"), "%hd", &scan->TileHeight);
			const char* type = elementScan->Attribute("Type");
			scan->Type = convertPixelType(type);
			XML_SCANF(elementScan->Attribute("SignificantBits"), "%hd", &scan->SignificantBits);
			const XMLElement* elementChannels = elementScan->FirstChildElement("Channels");
			if (elementChannels == NULL)	throw new std::exception("Channels");
			for (const XMLElement* elementChannel = elementChannels->FirstChildElement("Channel"); elementChannel; elementChannel = elementChannel->NextSiblingElement("Channel"))
			{
				Channel* channel = new Channel();
				XML_SCANF(elementChannel->Attribute("ID"), "%hd", &channel->ChannelID);
				XML_SCANF(elementChannel->Attribute("RefID"), "%hd", &channel->ChannelRefID);
				const char* name = elementChannel->Attribute("Name");
				strcpy_s(channel->Name, strlen(name) + 1, name);
				scan->Channels.insert(std::make_pair(channel->ChannelID, channel));
			}
			const XMLElement* elementScanRegions = elementScan->FirstChildElement("ScanRegions");
			if (elementScanRegions == NULL)	throw new std::exception("ScanRegions");
			for (const XMLElement* elementScanRegion = elementScanRegions->FirstChildElement("ScanRegion"); elementScanRegion; elementScanRegion = elementScanRegion->NextSiblingElement("ScanRegion"))
			{
				region* tRegion = new region();
				tRegion->ScanID = scan->ScanID;
				XML_SCANF(elementScanRegion->Attribute("ID"), "%hd", &tRegion->RegionID);
				XML_SCANF(elementScanRegion->Attribute("SizeX"), "%d", &tRegion->SizeX);
				XML_SCANF(elementScanRegion->Attribute("SizeY"), "%d", &tRegion->SizeY);
				XML_SCANF(elementScanRegion->Attribute("SizeZ"), "%d", &tRegion->SizeZ);
				XML_SCANF(elementScanRegion->Attribute("SizeT"), "%d", &tRegion->SizeT);
				XML_SCANF(elementScanRegion->Attribute("SizeS"), "%d", &tRegion->SizeS);
				scan->Regions.insert(std::make_pair(tRegion->RegionID, tRegion));
				const XMLElement* elementSampleRegion = elementScanRegion->FirstChildElement("SampleRegion");
				if (elementSampleRegion == NULL)	throw new std::exception("SampleRegion");
				XML_SCANF(elementSampleRegion->Attribute("RegionID"), "RegionID:%hd.%hd.%hd.%hd", &tRegion->PlateID, &tRegion->WellID, &tRegion->WellSampleID, &tRegion->RegionID);
				Plate* plate = (*plates)[tRegion->PlateID];
				if (plate == NULL)	throw new std::exception("plate");
				Well* well = plate->Wells[tRegion->WellID];
				if (well == NULL)	throw new std::exception("well");
				WellSample* wellSample = (well->WellSamples.find(tRegion->WellSampleID) == well->WellSamples.end()) ? new WellSample() : well->WellSamples[tRegion->WellSampleID];
				if (wellSample == NULL) throw new exception("wellSample");

				//if (well->WellSamples.size() == 0)
				//	wellSample->WellSampleID = 1;
				//else
				//	wellSample->WellSampleID = well->WellSamples.rbegin()->second->WellSampleID + 1;
				wellSample->PlateID = tRegion->PlateID;
				wellSample->WellID = tRegion->WellID;
				wellSample->ScanID = tRegion->ScanID;

				SampleRegion* sampleRegion = new SampleRegion();
				sampleRegion->RegionID = tRegion->RegionID;
				sampleRegion->WellSampleID = tRegion->WellSampleID = wellSample->WellSampleID;
				XML_SCANF(elementSampleRegion->Attribute("PositionX"), "%lf", &sampleRegion->PositionX);
				XML_SCANF(elementSampleRegion->Attribute("PositionY"), "%lf", &sampleRegion->PositionY);
				XML_SCANF(elementSampleRegion->Attribute("PositionZ"), "%lf", &sampleRegion->PositionZ);
				XML_SCANF(elementSampleRegion->Attribute("SizeX"), "%lf", &sampleRegion->SizeX);
				XML_SCANF(elementSampleRegion->Attribute("SizeY"), "%lf", &sampleRegion->SizeY);
				XML_SCANF(elementSampleRegion->Attribute("SizeZ"), "%lf", &sampleRegion->SizeZ);
				sampleRegion->PhysicalSizeXUnit = scan->PhysicalSizeXUnit;
				sampleRegion->PhysicalSizeYUnit = scan->PhysicalSizeYUnit;
				sampleRegion->PhysicalSizeZUnit = scan->PhysicalSizeZUnit;
				if (wellSample->SampleRegions.find(tRegion->RegionID) == wellSample->SampleRegions.end())
					wellSample->SampleRegions.insert(make_pair(tRegion->RegionID, sampleRegion));

				if (well->WellSamples.find(tRegion->WellSampleID) == well->WellSamples.end())
					well->WellSamples.insert(make_pair(wellSample->WellSampleID, wellSample));

			}
			(*scans).insert(std::make_pair(scan->ScanID, scan));
		}
		return 1;
	}
	catch (...)
	{
		return NULL;
	}

}

long GeneratePlatesXML(int fileHandle, std::map<uint16_t, Plate*> plates)
{
	long ret = TRUE;
	char tmp[_MAX_FNAME] = { 0 };
	tinyxml2::XMLDocument doc;
	char* dst_plates_info = NULL;
	try
	{
		std::multimap<uint16_t, XMLElement*> map_PlateAcquisition;
		XMLElement *elementPlates = doc.NewElement("Plates");
		doc.InsertEndChild(elementPlates);
		if(0 < plates.size())
		{
			for (std::map<uint16_t, Plate*>::iterator plateit = plates.begin(); plateit != plates.end(); plateit++)
			{
				XMLElement *elementPlate = doc.NewElement("Plate");
				elementPlates->InsertEndChild(elementPlate);
				memset(tmp, 0, _MAX_FNAME);
				sprintf_s(tmp, "Plate:%hd", plateit->second->PlateID);
				elementPlate->SetAttribute("ID", tmp);
				elementPlate->SetAttribute("Name", plateit->second->Name);
				elementPlate->SetAttribute("Width", convertDouble(plateit->second->Width, tmp));
				elementPlate->SetAttribute("Height", convertDouble(plateit->second->Height, tmp));
				elementPlate->SetAttribute("PhysicalSizeXUnit", plateit->second->PhysicalSizeXUnit);
				elementPlate->SetAttribute("PhysicalSizeYUnit", plateit->second->PhysicalSizeYUnit);
				elementPlate->SetAttribute("Rows", plateit->second->Rows);
				elementPlate->SetAttribute("Columns", plateit->second->Columns);
				for (std::map<uint16_t, Well*>::iterator wellit = plateit->second->Wells.begin(); wellit != plateit->second->Wells.end(); wellit++)
				{
					XMLElement *elementWell = doc.NewElement("Well");
					elementPlate->InsertEndChild(elementWell);
					memset(tmp, 0, _MAX_FNAME);
					sprintf_s(tmp, "Well:%hd.%hd", plateit->second->PlateID, wellit->second->WellID);
					elementWell->SetAttribute("ID", tmp);

					elementWell->SetAttribute("PositionX", convertDouble(wellit->second->PositionX, tmp));
					elementWell->SetAttribute("PositionY", convertDouble(wellit->second->PositionY, tmp));
					elementWell->SetAttribute("Width", convertDouble(wellit->second->Width, tmp));
					elementWell->SetAttribute("Height", convertDouble(wellit->second->Height, tmp));
					elementWell->SetAttribute("Shape", wellit->second->Shape);
					elementWell->SetAttribute("Row", wellit->second->Row);
					elementWell->SetAttribute("Column", wellit->second->Column);

					for (std::map<uint16_t, WellSample*>::iterator wellSampleit = wellit->second->WellSamples.begin(); wellSampleit != wellit->second->WellSamples.end(); wellSampleit++)
					{
						XMLElement *elementWellSample = doc.NewElement("WellSample");
						elementWell->InsertEndChild(elementWellSample);
						memset(tmp, 0, _MAX_FNAME);
						sprintf_s(tmp, "WellSample:%hd.%hd.%hd", plateit->second->PlateID, wellit->second->WellID, wellSampleit->second->WellSampleID);
						elementWellSample->SetAttribute("ID", tmp);

						elementWellSample->SetAttribute("ImageID", convertDouble(wellSampleit->second->ImageID, tmp));
						elementWellSample->SetAttribute("ScanID", convertDouble(wellSampleit->second->ScanID, tmp));

						for (std::map<uint16_t, SampleRegion*>::iterator sampleRegionit = wellSampleit->second->SampleRegions.begin(); sampleRegionit != wellSampleit->second->SampleRegions.end(); sampleRegionit++)
						{
							XMLElement *elementSampleRegion = doc.NewElement("SampleRegion");
							elementWellSample->InsertEndChild(elementSampleRegion);
							memset(tmp, 0, _MAX_FNAME);
							sprintf_s(tmp, "SampleRegion:%hd.%hd.%hd.%hd", plateit->second->PlateID, wellit->second->WellID, wellSampleit->second->WellSampleID, sampleRegionit->second->RegionID);
							elementSampleRegion->SetAttribute("ID", tmp);

							elementSampleRegion->SetAttribute("PositionX", convertDouble(sampleRegionit->second->PositionX, tmp));
							elementSampleRegion->SetAttribute("PositionY", convertDouble(sampleRegionit->second->PositionY, tmp));
							elementSampleRegion->SetAttribute("PositionZ", convertDouble(sampleRegionit->second->PositionZ, tmp));
							elementSampleRegion->SetAttribute("SizeX", convertDouble(sampleRegionit->second->SizeX, tmp));
							elementSampleRegion->SetAttribute("SizeY", convertDouble(sampleRegionit->second->SizeY, tmp));
							elementSampleRegion->SetAttribute("SizeZ", convertDouble(sampleRegionit->second->SizeZ, tmp));
							elementSampleRegion->SetAttribute("PhysicalSizeXUnit", sampleRegionit->second->PhysicalSizeXUnit);
							elementSampleRegion->SetAttribute("PhysicalSizeYUnit", sampleRegionit->second->PhysicalSizeYUnit);
							elementSampleRegion->SetAttribute("PhysicalSizeZUnit", sampleRegionit->second->PhysicalSizeZUnit);
						}
					}
				}
			}
		}

		XMLPrinter printer;
		doc.Print(&printer);
		dst_plates_info = new char[printer.CStrSize()];
		memcpy_s(dst_plates_info, printer.CStrSize(), printer.CStr(), printer.CStrSize());
		if (SUCCESS != fnAISS_set_plates_info(fileHandle, dst_plates_info, static_cast<uint32_t>(strlen(dst_plates_info))))
		{
			ret = FALSE;
		}
		delete(dst_plates_info);
		dst_plates_info = NULL;
		return ret;
	}
	catch (...)
	{
		if(NULL != dst_plates_info)
		{
			delete(dst_plates_info);
			dst_plates_info = NULL;
		}
		return FALSE;
	}
}

long GenerateScanXML(int fileHandle, std::map<uint16_t, Plate*> plates, Scan* scan)
{
	long ret = TRUE;
	char tmp[_MAX_FNAME] = { 0 };
	tinyxml2::XMLDocument doc;
	char* dst_scan_info = NULL;
	try
	{
		XMLElement *elementScan = doc.NewElement("Scan");
		doc.InsertEndChild(elementScan);

		if(NULL != scan)
		{
			elementScan->SetAttribute("ID", scan->ScanID);
			elementScan->SetAttribute("PlateID", scan->PlateID);
			elementScan->SetAttribute("PhysicalSizeX", convertDouble(scan->PhysicalSizeX, tmp));
			elementScan->SetAttribute("PhysicalSizeY", convertDouble(scan->PhysicalSizeY, tmp));
			elementScan->SetAttribute("PhysicalSizeZ", convertDouble(scan->PhysicalSizeZ, tmp));
			elementScan->SetAttribute("PhysicalSizeXUnit", scan->PhysicalSizeXUnit);
			elementScan->SetAttribute("PhysicalSizeYUnit", scan->PhysicalSizeYUnit);
			elementScan->SetAttribute("PhysicalSizeZUnit", scan->PhysicalSizeZUnit);
			elementScan->SetAttribute("TimeIncrement", scan->TimeIncrement);
			elementScan->SetAttribute("TimeIncrementUnit", scan->TimeIncrementUnit);
			elementScan->SetAttribute("DimensionOrder", scan->DimensionOrder);
			elementScan->SetAttribute("TileWidth", scan->TileWidth);
			elementScan->SetAttribute("TileHeight", scan->TileHeight);
			elementScan->SetAttribute("Type", PixelTypeString[scan->Type].c_str());
			elementScan->SetAttribute("SignificantBits", scan->SignificantBits);
			XMLElement *elementChannels = doc.NewElement("Channels");
			elementScan->InsertEndChild(elementChannels);
			for (std::map<uint16_t, Channel*>::iterator channelit = scan->Channels.begin(); channelit != scan->Channels.end(); channelit++)
			{
				XMLElement *elementChannel = doc.NewElement("Channel");
				elementChannels->InsertEndChild(elementChannel);
				elementChannel->SetAttribute("ID", channelit->second->ChannelID);
				elementChannel->SetAttribute("RefID", channelit->second->ChannelRefID);
				elementChannel->SetAttribute("Name", channelit->second->Name);
			}
			XMLElement *elementScanRegions = doc.NewElement("ScanRegions");
			elementScan->InsertEndChild(elementScanRegions);
			for (std::map<uint16_t, region*>::iterator regionit = scan->Regions.begin(); regionit != scan->Regions.end(); regionit++)
			{
				XMLElement *elementScanRegion = doc.NewElement("ScanRegion");
				elementScanRegions->InsertEndChild(elementScanRegion);
				elementScanRegion->SetAttribute("ID", regionit->second->RegionID);
				elementScanRegion->SetAttribute("SizeX", regionit->second->SizeX);
				elementScanRegion->SetAttribute("SizeY", regionit->second->SizeY);
				elementScanRegion->SetAttribute("SizeZ", regionit->second->SizeZ);
				elementScanRegion->SetAttribute("SizeT", regionit->second->SizeT);
				elementScanRegion->SetAttribute("SizeS", regionit->second->SizeS);
				Plate* plate = plates[regionit->second->PlateID];
				if (plate == NULL)	throw new std::exception("plate");
				Well* well = plate->Wells[regionit->second->WellID];
				if (well == NULL)	throw new std::exception("well");
				WellSample* wellSample = well->WellSamples[regionit->second->WellSampleID];
				if (wellSample == NULL)	throw new std::exception("wellSample");
				SampleRegion* sampleRegion = wellSample->SampleRegions[regionit->second->RegionID];
				if (sampleRegion == NULL)	throw new std::exception("SampleRegion");
				XMLElement *elementSampleRegion = doc.NewElement("SampleRegion");
				elementScanRegion->InsertEndChild(elementSampleRegion);
				memset(tmp, 0, _MAX_FNAME);
				sprintf_s(tmp, "RegionID:%hd.%hd.%hd.%hd", regionit->second->PlateID, regionit->second->WellID, regionit->second->WellSampleID, regionit->second->RegionID);
				elementSampleRegion->SetAttribute("RegionID", tmp);
				elementSampleRegion->SetAttribute("PositionX", convertDouble(sampleRegion->PositionX, tmp));
				elementSampleRegion->SetAttribute("PositionY", convertDouble(sampleRegion->PositionY, tmp));
				elementSampleRegion->SetAttribute("PositionZ", convertDouble(sampleRegion->PositionZ, tmp));
				elementSampleRegion->SetAttribute("SizeX", convertDouble(sampleRegion->SizeX, tmp));
				elementSampleRegion->SetAttribute("SizeY", convertDouble(sampleRegion->SizeY, tmp));
				elementSampleRegion->SetAttribute("SizeZ", convertDouble(sampleRegion->SizeZ, tmp));				
				elementSampleRegion->SetAttribute("PhysicalSizeXUnit", sampleRegion->PhysicalSizeXUnit);
				elementSampleRegion->SetAttribute("PhysicalSizeYUnit", sampleRegion->PhysicalSizeYUnit);
				elementSampleRegion->SetAttribute("PhysicalSizeZUnit", sampleRegion->PhysicalSizeZUnit);
			}
		}
		XMLPrinter printer;
		doc.Print(&printer);
		dst_scan_info = new char[printer.CStrSize()];
		memcpy_s(dst_scan_info, printer.CStrSize(), printer.CStr(), printer.CStrSize());
		if (SUCCESS != fnAISS_add_scan_info(fileHandle, dst_scan_info, static_cast<uint32_t>(strlen(dst_scan_info))))
		{
			ret = FALSE;
		}
		delete(dst_scan_info);
		dst_scan_info = NULL;
		return ret;
	}
	catch (...)
	{
		if(NULL != dst_scan_info)
		{
			delete(dst_scan_info);
			dst_scan_info = NULL;
		}
		return FALSE;
	}
}
