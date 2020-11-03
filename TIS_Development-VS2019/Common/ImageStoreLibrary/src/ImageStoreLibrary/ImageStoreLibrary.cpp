// ImageStoreLibrary.cpp : Defines the exported functions for the DLL application.
//

#include "ImageStoreLibrary.h"
#include "TiffData.h"

static vector<TiffData *> vecTiffData;

long fnAISS_open_file(char* file_name, OpenMode mode, bool is_create_pyramidal_data)
{
	TiffData* tiffData = new TiffData();
	long ret = tiffData->Init(file_name, mode, is_create_pyramidal_data);
	if (ret >= 0)
	{
		vecTiffData.push_back(tiffData);
		return (int)vecTiffData.size();
	}
	else
	{
		delete tiffData;
		return ret;
	}
}

long fnAISS_close_file(int handle)
{
	if (handle <= 0 || handle > (int)vecTiffData.size()) return HANDLE_IS_NOT_EXIST;
	delete(vecTiffData[handle - 1]);
	vecTiffData.erase(vecTiffData.begin() + handle - 1);
	return SUCCESS;
}

long fnAISS_clean_data(int handle)
{
	if (handle <= 0 || handle > (int)vecTiffData.size()) return HANDLE_IS_NOT_EXIST;
	return	vecTiffData[handle - 1]->CleanData();
}

long fnAISS_set_plates_info(int handle, void * plate_info, uint32_t size)
{
	if (plate_info == NULL) return INPUT_PTR_IS_NULL;
	if (handle <= 0 || handle > (int)vecTiffData.size()) return HANDLE_IS_NOT_EXIST;
	return	vecTiffData[handle - 1]->SetPlateInfo(plate_info, size);
}

long fnAISS_get_plates_info_size(int handle, uint32_t * size)
{
	if (size == NULL) return INPUT_PTR_IS_NULL;
	if (handle <= 0 || handle > (int)vecTiffData.size()) return HANDLE_IS_NOT_EXIST;
	return	vecTiffData[handle - 1]->GetPlateInfoSize(size);
}

long fnAISS_get_plates_info(int handle, void * plate_info, uint32_t size)
{
	if (plate_info == NULL) return INPUT_PTR_IS_NULL;
	if (handle <= 0 || handle > (int)vecTiffData.size()) return HANDLE_IS_NOT_EXIST;
	return	vecTiffData[handle - 1]->GetPlateInfo(plate_info, size);
}

long fnAISS_add_scan_info(int handle, void * scan_info, uint32_t size)
{
	if (scan_info == NULL) return INPUT_PTR_IS_NULL;
	if (size == 0)return BUFFER_SIZE_IS_NOT_CORRECT;
	if (handle <= 0 || handle > (int)vecTiffData.size()) return HANDLE_IS_NOT_EXIST;
	return	vecTiffData[handle - 1]->AddScanInfo(scan_info, size);
}

long fnAISS_get_scans_info_size(int handle, uint32_t * size)
{
	if (size == NULL) return INPUT_PTR_IS_NULL;
	if (handle <= 0 || handle > (int)vecTiffData.size()) return HANDLE_IS_NOT_EXIST;
	return	vecTiffData[handle - 1]->GetScanInfosSize(size);
}

long fnAISS_get_scans_info(int handle, void * scan_infos, uint32_t size)
{
	if (scan_infos == NULL) return INPUT_PTR_IS_NULL;
	if (handle <= 0 || handle > (int)vecTiffData.size()) return HANDLE_IS_NOT_EXIST;
	return	vecTiffData[handle - 1]->GetScanInfos(scan_infos, size);
}

long fnAISS_remove_scan(int handle, uint32_t scan_id)
{
	if (handle <= 0 || handle > (int)vecTiffData.size()) return HANDLE_IS_NOT_EXIST;
	return	vecTiffData[handle - 1]->RemoveScan(scan_id);
}

long fnAISS_save_tile_data(int handle, void* image_data, uint32_t stride, frame_info frame, uint32_t tile_row, uint32_t tile_column)
{
	if (image_data == NULL) return INPUT_PTR_IS_NULL;
	if (handle <= 0 || handle > (int)vecTiffData.size()) return HANDLE_IS_NOT_EXIST;
	return	vecTiffData[handle - 1]->SaveTileData(image_data, stride, frame, tile_row, tile_column);
}

long fnAISS_get_raw_data(int handle, frame_info frame, IplRect src_rect, void * image_data)
{
	if (image_data == NULL) return INPUT_PTR_IS_NULL;
	if (handle <= 0 || handle > (int)vecTiffData.size()) return HANDLE_IS_NOT_EXIST;
	return vecTiffData[handle - 1]->LoadRawData(frame, src_rect, image_data);
}

long fnAISS_get_scaled_data(int handle, frame_info frame, IplSize dst_size, IplRect src_rect, void * image_data)
{
	if (image_data == NULL) return INPUT_PTR_IS_NULL;
	if (handle <= 0 || handle > (int)vecTiffData.size()) return HANDLE_IS_NOT_EXIST;
	return vecTiffData[handle - 1]->LoadScaledData(frame, dst_size, src_rect, image_data);
}

long fnAISS_get_scaled_tileData(int handle, frame_info frame, uint16_t scaleLevel, uint16_t row, uint16_t column, void * image_data)
{
	if (image_data == NULL) return INPUT_PTR_IS_NULL;
	if (handle <= 0 || handle > (int)vecTiffData.size()) return HANDLE_IS_NOT_EXIST;
	return vecTiffData[handle - 1]->LoadScaledData(frame, scaleLevel, row, column, image_data);
}

long fnAISS_generate_pyramidal_data(int handle, uint8_t scan_id)
{
	if (handle <= 0 || handle > (int)vecTiffData.size()) return HANDLE_IS_NOT_EXIST;
	return	vecTiffData[handle - 1]->GeneratePyramidalData(scan_id);
}

long fnAISS_generate_pyramidal_data_frame(int handle, frame_info frame, uint8_t* image, uint32_t image_width, uint32_t image_height)
{
	if (handle <= 0 || handle > (int)vecTiffData.size()) return HANDLE_IS_NOT_EXIST;
	return	vecTiffData[handle - 1]->GeneratePyramidalData(frame, image, image_width, image_height);
}

long fnAISS_save_additional_data(int handle, void *additional_data, uint32_t size, char* name)
{
	if (additional_data == NULL || name == NULL) return INPUT_PTR_IS_NULL;
	if (handle <= 0 || handle > (int)vecTiffData.size()) return HANDLE_IS_NOT_EXIST;
	return	vecTiffData[handle - 1]->SaveAdditionalData(additional_data, size, name);
}

long fnAISS_get_additional_data_size(int handle, char* name, uint32_t * size)
{
	if (name == NULL || size == NULL) return INPUT_PTR_IS_NULL;
	if (handle <= 0 || handle > (int)vecTiffData.size()) return HANDLE_IS_NOT_EXIST;
	return	vecTiffData[handle - 1]->GetAdditionalDataSize(name, size);
}

long fnAISS_get_additional_data(int handle, char* name, void *additional_data, uint32_t size)
{
	if (name == NULL || additional_data == NULL) return INPUT_PTR_IS_NULL;
	if (handle <= 0 || handle > (int)vecTiffData.size()) return HANDLE_IS_NOT_EXIST;
	return	vecTiffData[handle - 1]->GetAdditionalData(name, additional_data, size);
}

long fnAISS_delete_additional_data(int handle, char* name)
{
	if (name == NULL) return INPUT_PTR_IS_NULL;
	if (handle <= 0 || handle > (int)vecTiffData.size()) return HANDLE_IS_NOT_EXIST;
	return	vecTiffData[handle - 1]->DeleteAdditionalData(name);
}

long fnAISS_set_field(int handle, uint32_t tag, uint32_t v)
{
	if (handle <= 0 || handle > (int)vecTiffData.size()) return HANDLE_IS_NOT_EXIST;
	return  vecTiffData[handle - 1]->SetField(tag, v);
}
