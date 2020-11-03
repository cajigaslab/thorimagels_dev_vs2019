#include "TiffData.h"
#include "zip.h"
//#include <corecrt_io.h>
#include <fcntl.h>
#include<fstream>

TiffData::TiffData()
{
	tag_compression = IMAGESTORE_COMPRESSION_NONE;
}

TiffData::~TiffData()
{
	if (tiff != NULL)
		TIFFClose(tiff);
	if (_is_file_using = true) {
		remove(tiff_tmp_file);
	}
	map<uint8_t, Scan*>::iterator scanit = scans.begin();
	while (scanit != scans.end())
	{
		delete scanit->second;
		scanit->second = NULL;
		scans.erase(scanit++);
	}
	map<uint16_t, Plate*>::iterator plateit = plates.begin();
	while (plateit != plates.end())
	{
		delete plateit->second;
		plateit->second = NULL;
		plates.erase(plateit++);
	}
}

long TiffData::Init(char* file_name, OpenMode openMode, bool is_create_pyramidal_data)
{
	if (tiff != NULL)
		return FILE_HAS_BEEN_OPENED;
	_is_create_pyramidal_data = is_create_pyramidal_data;
	memset(tiff_file_full_name, 0, STRING_BUFFER_SIZE);
	memset(tiff_tmp_file, 0, STRING_BUFFER_SIZE);
	memset(tiff_file_extension, 0, STRING_BUFFER_SIZE);
	memset(tiff_file_name, 0, STRING_BUFFER_SIZE);
	memset(tiff_file_dir, 0, STRING_BUFFER_SIZE);
	memset(zip_file_full_name, 0, STRING_BUFFER_SIZE);
	char* name = strrchr(file_name, '\\');
	if (name)
	{
		name++;
		memcpy(tiff_file_dir, file_name, name - file_name - 1);
	}
	else
	{
		name = file_name;
		TCHAR szDir[MAX_PATH] = { 0 };
		GetCurrentDirectory(MAX_PATH, szDir);
		int iLength = WideCharToMultiByte(CP_ACP, 0, szDir, -1, NULL, 0, NULL, NULL);
		WideCharToMultiByte(CP_ACP, 0, szDir, -1, tiff_file_dir, iLength, NULL, NULL);
	}
	char* extension = strrchr(name, '.');
	if (!extension) return FAILED_FILE_NAME;
	memcpy(tiff_file_name, name, extension - name);
	memcpy(tiff_file_extension, extension + 1, strlen(extension));
	sprintf_s(tiff_file_full_name, "%s\\%s", tiff_file_dir, name);
	sprintf_s(tiff_tmp_file, "%s\\~%s.tmp", tiff_file_dir, name);
	sprintf_s(zip_file_full_name, "%s\\%s.zip", tiff_file_dir, tiff_file_name);
	if (openMode == OpenMode::CREATE_MODE)
	{
		ofstream f(tiff_tmp_file);
		if(!f)return FAILED_FILE_NAME;
		
		//auto err_no = _sopen_s(&tmp_fd, tiff_tmp_file, O_RDWR | O_CREAT, _SH_DENYWR, _S_IREAD | _S_IWRITE);
		//if(tmp_fd<0)return FAILED_FILE_NAME;
		tiff = TIFFOpen(tiff_file_full_name, "w");
		if (tiff == NULL) return FAILED_FILE_NAME;
	}
	else if (openMode == OpenMode::READ_WRITE_MODE)
	{
		ofstream f(tiff_tmp_file);
		if(!f)return FAILED_FILE_NAME;
		//auto err_no = _sopen_s(&tmp_fd, tiff_tmp_file, O_RDWR | O_CREAT, _SH_DENYWR, _S_IREAD | _S_IWRITE);
		//if (tmp_fd<0)return FAILED_FILE_NAME;

		tiff = TIFFOpen(tiff_file_full_name, "r+");
		if (tiff == NULL) return FAILED_FILE_NAME;
		char* xml;
		if (!TIFFGetField(tiff, TIFFTAG_IMAGEDESCRIPTION, &xml))
			return UNKNOWN_ERROR;
		if (xml_read_ome(xml, &plates, &scans, this, openMode) < 0)
			return UNKNOWN_ERROR;
	}
	else if (openMode == OpenMode::READ_ONLY_MODE)
	{
		tiff = TIFFOpen(tiff_file_full_name, "r");
		if (tiff == NULL) return UNKNOWN_ERROR;
		char* xml;
		if (!TIFFGetField(tiff, TIFFTAG_IMAGEDESCRIPTION, &xml))
			return UNKNOWN_ERROR;
		if (xml_read_ome(xml, &plates, &scans, this, openMode) < 0)
			return UNKNOWN_ERROR;
	}
	else
	{
		return UNKNOWN_ERROR;
	}
	return SUCCESS;
}

long TiffData::SaveTileData(void * image_data, uint32_t stride, frame_info frame, unsigned int tile_row, unsigned int tile_column)
{
	map<uint8_t, Scan*>::iterator scanit = scans.find(frame.scan_id);
	if (scanit == scans.end())	return FRAMEINFO_IS_NOT_CORRECT;
	auto regionit = scanit->second->Regions.find(frame.region_id);
	if (regionit == scanit->second->Regions.end())	return FRAMEINFO_IS_NOT_CORRECT;
	return regionit->second->SaveTileData(image_data, stride, frame, tile_row, tile_column);
}

long TiffData::GeneratePyramidalData(uint8_t scan_id)
{
	if (!_is_create_pyramidal_data)return FALSE;
	map<uint8_t, Scan*>::iterator scanit = scans.find(scan_id);
	if (scanit == scans.end()) 		return FRAMEINFO_IS_NOT_CORRECT;
	map<uint16_t, region*>* regions = &(*scanit).second->Regions;
	for(map<uint16_t, region*>::iterator regionit = regions->begin();regionit!=regions->end();regionit++)
	{
		auto result = regionit->second->GeneratePyramidalData();
		if (result != SUCCESS) {
			return result;
		}
	}
	return SUCCESS;
}

long TiffData::GeneratePyramidalData(frame_info frame, uint8_t* image, uint32_t image_width, uint32_t image_height)
{
	if (!_is_create_pyramidal_data)return FALSE;
	map<uint8_t, Scan*>::iterator scanit = scans.find(frame.scan_id);
	if (scanit == scans.end()) 		return FRAMEINFO_IS_NOT_CORRECT;
	auto regionit = scanit->second->Regions.find(frame.region_id);
	if(regionit== scanit->second->Regions.end()) return FRAMEINFO_IS_NOT_CORRECT;
	return regionit->second->GeneratePyramidalData(frame);
}

long TiffData::LoadRawData(frame_info frame, IplRect src_rect, void* buffer)
{
	map<uint8_t, Scan*>::iterator scanit = scans.find(frame.scan_id);
	if (scanit == scans.end()) 		return FRAMEINFO_IS_NOT_CORRECT;
	auto regionit = scanit->second->Regions.find(frame.region_id);
	if (regionit == scanit->second->Regions.end()) return FRAMEINFO_IS_NOT_CORRECT;
	return regionit->second->LoadRawRectData(frame, src_rect, buffer);
}

long TiffData::LoadScaledData(frame_info frame, IplSize dst_size, IplRect src_rect, void* buffer)
{
	map<uint8_t, Scan*>::iterator scanit = scans.find(frame.scan_id);
	if (scanit == scans.end()) 		return FRAMEINFO_IS_NOT_CORRECT;
	auto regionit = scanit->second->Regions.find(frame.region_id);
	if (regionit == scanit->second->Regions.end()) return FRAMEINFO_IS_NOT_CORRECT;
	return regionit->second->LoadPyramidalRectData(frame, dst_size, src_rect, buffer);
}

long TiffData::LoadScaledData(frame_info frame, uint16_t scaleLevel, uint16_t row, uint16_t column, void* buffer)
{
	map<uint8_t, Scan*>::iterator scanit = scans.find(frame.scan_id);
	if (scanit == scans.end()) 	return FRAMEINFO_IS_NOT_CORRECT; 
	auto regionit = scanit->second->Regions.find(frame.region_id);
	if (regionit == scanit->second->Regions.end()) return FRAMEINFO_IS_NOT_CORRECT;
	return regionit->second->LoadPyramidalRectData(frame, scaleLevel, row, column, buffer);
}

long TiffData::SaveAdditionalData(void* additional_data, uint32_t size, char* name)
{
	return zip_write_file(zip_file_full_name, name, additional_data, size);
}

long TiffData::GetAdditionalDataSize(char* name, uint32_t* size)
{
	long ret = zip_get_file_size(zip_file_full_name, name);
	if (ret >= 0)
		*size = ret;
	return SUCCESS;
}

long TiffData::GetAdditionalData(char* name, void* additional_data, uint32_t size)
{
	long ret = zip_read_file(zip_file_full_name, name, additional_data, size);
	if (ret != size)
		return BUFFER_SIZE_IS_NOT_CORRECT;
	return SUCCESS;
}

long TiffData::DeleteAdditionalData(char* name)
{
	return zip_delete_file(zip_file_full_name, name);
}

long TiffData::CleanData()
{
	map<uint8_t, Scan*>::iterator scanit = scans.begin();
	while (scanit != scans.end())
	{
		scanit = removeScan(scanit);
	}
	map<uint16_t, Plate*>::iterator plateit = plates.begin();
	while (plateit != plates.end())
	{
		delete plateit->second;
		plateit->second = NULL;
		plateit = plates.erase(plateit);
	}
	return rewriteOMEHeader();
}

long TiffData::SetPlateInfo(void* plate_info, uint32_t size)
{
	if (plates.size() != 0)
		return PLATE_HAS_SET;
	char* plate_info_string = (char*)plate_info;
	if (strlen(plate_info_string) != size)
		return BUFFER_SIZE_IS_NOT_CORRECT;
	return xml_set_plate_info(plate_info_string, &plates);

}

long TiffData::GetPlateInfoSize(uint32_t* size)
{
	if (plates.size() == 0)
		return PLATE_INFO_IS_NOT_EXIT;
	char* xml;
	xml_get_plate_info(plates, &xml);
	if (xml == NULL)
		return UNKNOWN_ERROR;
	*size = (uint32_t)(strlen(xml)) + 1;
	free(xml);
	return SUCCESS;

}

long TiffData::GetPlateInfo(void* plate_info, uint32_t size)
{
	if (plates.size() == 0)
		return PLATE_INFO_IS_NOT_EXIT;
	char* xml;
	xml_get_plate_info(plates, &xml);
	if (xml == NULL)
		return UNKNOWN_ERROR;
	size_t xml_size = strlen(xml);
	if (xml_size + 1 != size)
	{
		free(xml);
		return BUFFER_SIZE_IS_NOT_CORRECT;
	}
	memset(plate_info, 0, xml_size + 1);
	memcpy_s(plate_info, xml_size, xml, xml_size);
	free(xml);
	return SUCCESS;

}

long TiffData::AddScanInfo(void* scan_info, uint32_t size)
{
	char* scan_info_string = (char*)scan_info;
	if (strlen(scan_info_string) != size)
		return BUFFER_SIZE_IS_NOT_CORRECT;
	Scan* scan = new Scan(this);
	if (!xml_add_scan_info(scan_info_string, plates, &scan))
	{
		delete scan;
		return UNKNOWN_ERROR;
	}
	map<uint8_t, Scan*>::iterator scanit = scans.find(scan->ScanID);
	if (scanit != scans.end())
	{
		removeScan(scanit);
	}
	scans.insert(make_pair(scan->ScanID, scan));
	
	map<uint16_t, region*>* regions = &scan->Regions;
	for(map<uint16_t, region*>::iterator regionit = regions->begin();regionit!=regions->end();regionit++)
	{
		if (!regionit->second->CreateContainers(_is_create_pyramidal_data)) {
			return UNKNOWN_ERROR;
		}
	}
	return rewriteOMEHeader();
}

long TiffData::GetScanInfosSize(uint32_t* size)
{
	char* xml;
	xml_get_scan_infos(plates, scans, &xml);
	if (xml == NULL)
		return UNKNOWN_ERROR;
	*size = (uint32_t)(strlen(xml)+1);
	free(xml);
	return SUCCESS;
}

long TiffData::GetScanInfos(void* scan_infos, uint32_t size)
{
	char* xml;
	xml_get_scan_infos(plates, scans, &xml);
	if (xml == NULL)
		return UNKNOWN_ERROR;
	size_t xml_size = strlen(xml);
	if (xml_size +1 != size)
	{
		free(xml);
		return BUFFER_SIZE_IS_NOT_CORRECT;
	}
	memset(scan_infos, 0, xml_size+1);
	memcpy_s(scan_infos, xml_size, xml, xml_size);
	free(xml);
	return SUCCESS;
}

long TiffData::RemoveScan(uint32_t scan_id)
{
	map<uint8_t, Scan*>::iterator scanit = scans.find(scan_id);
	if (scanit == scans.end()) 		return SCAN_ID_IS_NOT_EXIST;
	removeScan(scanit);
	return rewriteOMEHeader();
}

long TiffData::SetField(uint32_t tag, uint32_t v)
{
	switch (tag)
	{
	case IMAGESTORETAG_COMPRESSION:
	{
		for each(uint32_t codec in RegisterCODEC)
		{
			if (codec == v)
			{
				tag_compression = v;
				return SUCCESS;
			}
		}
		return UNKNOWN_ERROR;
	}
	default:
		break;
	}
	return UNKNOWN_ERROR;
}

map<uint8_t, Scan*>::iterator TiffData::removeScan(map<uint8_t, Scan*>::iterator scanit)
{
	for (map<uint16_t, region*>::iterator regionit = scanit->second->Regions.begin(); regionit != scanit->second->Regions.end(); regionit++)
	{
		WellSample* wellSample = plates[scanit->second->PlateID]->Wells[regionit->second->WellID]->WellSamples[regionit->second->WellSampleID];
		plates[scanit->second->PlateID]->Wells[regionit->second->WellID]->WellSamples.erase(regionit->second->WellSampleID);
		delete wellSample;
		regionit->second->Remove();
	}
	delete scanit->second;
	return scans.erase(scanit);
}

long TiffData::rewriteOMEHeader()
{
	if (tiff != NULL)
		TIFFClose(tiff);
	tiff = NULL;
	tiff = TIFFOpen(tiff_file_full_name, "w");
	if (tiff == NULL) return UNKNOWN_ERROR;
	char* xml;
	xml_write_ome(plates, scans, &xml, tiff_file_name,_is_create_pyramidal_data);
	//TIFFSetDirectory(tiff, 0);
	TIFFFreeDirectory(tiff);
	if (!TIFFSetField(tiff, TIFFTAG_IMAGEDESCRIPTION, xml) ||
		!TIFFSetField(tiff, TIFFTAG_IMAGEWIDTH, 256) ||
		!TIFFSetField(tiff, TIFFTAG_IMAGELENGTH, 128) ||
		!TIFFSetField(tiff, TIFFTAG_BITSPERSAMPLE, 8) ||
		!TIFFSetField(tiff, TIFFTAG_SAMPLESPERPIXEL, 1) ||
		!TIFFSetField(tiff, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK) ||
		!TIFFSetField(tiff, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG))
		return UNKNOWN_ERROR;
	void* buf = malloc(256 * 128);
	memset(buf, 0, 256 * 128);
	TIFFWriteRawStrip(tiff, 0, buf, 256 * 128);
	TIFFWriteDirectory(tiff);
	free(xml);
	return SUCCESS;
}