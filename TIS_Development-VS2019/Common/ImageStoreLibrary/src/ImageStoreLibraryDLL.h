#pragma once

// This library is inherit from Thorlab China to support ThorImageLS on big OME tiff format
// Data structure or naming scheme is different in between and ThorImageLS only use a subset
// Plates and Wells are in physical dimension and Scans are in pixel image dimension
// Scan/ Plate/ Well / WellSample ID 1-based, channel / ScanRegion ID 0-based

#include <stdint.h>
#include "..\..\PDLL\pdll.h"
#include ".\ImageStoreLibrary\ImageStoreLibrary.h"

class ImageStoreLibraryDLL : public PDLL
{
#pragma warning(push)
#pragma warning(disable:26495)
#pragma warning(disable:26444)
	DECLARE_CLASS(ImageStoreLibraryDLL)
#pragma warning(pop)

	DECLARE_FUNCTION1(long, fnAISS_close_file, int)

	DECLARE_FUNCTION3(long, fnAISS_open_file, char*, OpenMode, bool)

	DECLARE_FUNCTION1(long, fnAISS_clean_data, int)

	DECLARE_FUNCTION3(long, fnAISS_set_plates_info, int, void *, unsigned int)

	DECLARE_FUNCTION2(long, fnAISS_get_plates_info_size, int, unsigned int *)

	DECLARE_FUNCTION3(long, fnAISS_get_plates_info, int, void *, unsigned int)

	DECLARE_FUNCTION3(long, fnAISS_add_scan_info, int, void *, unsigned int)

	DECLARE_FUNCTION2(long, fnAISS_get_scans_info_size, int, unsigned int *)

	DECLARE_FUNCTION3(long, fnAISS_get_scans_info, int, void *, unsigned int)

	DECLARE_FUNCTION2(long, fnAISS_remove_scan, int, unsigned int)

	DECLARE_FUNCTION6(long, fnAISS_save_tile_data, int, void*, unsigned int, frame_info, unsigned int, unsigned int)

	DECLARE_FUNCTION4(long, fnAISS_get_raw_data,int, frame_info, IplRect, void *)

	DECLARE_FUNCTION5(long, fnAISS_get_scaled_data, int, frame_info, IplSize, IplRect, void *)

	DECLARE_FUNCTION6(long, fnAISS_get_scaled_tileData, int, frame_info, uint16_t, uint16_t, uint16_t, void *)

	DECLARE_FUNCTION2(long, fnAISS_generate_pyramidal_data, int, unsigned char)

	DECLARE_FUNCTION5(long, fnAISS_generate_pyramidal_data_frame, int, frame_info, unsigned char*, unsigned int, unsigned int)

	DECLARE_FUNCTION4(long, fnAISS_save_additional_data, int, void *, unsigned int, char*)

	DECLARE_FUNCTION3(long, fnAISS_get_additional_data_size, int, char*, unsigned int *)

	DECLARE_FUNCTION4(long, fnAISS_get_additional_data, int, char*, void *, unsigned int)

	DECLARE_FUNCTION2(long, fnAISS_delete_additional_data, int, char*)

	DECLARE_FUNCTION3(long, fnAISS_set_field, int, unsigned int, unsigned int)

	/// ImageStoreWrapper Functions
	DECLARE_FUNCTION4(long, AddScan, double, double, double, long)

	DECLARE_FUNCTION0(long, ClearImageStore)

	DECLARE_FUNCTION8(long, GetImageStoreInfo, char*,long &, long &, long &, long &, long &, long &, long &)

	DECLARE_FUNCTION9(long, ReadImageStoreData, char*, long, long, long, long, long, long, long, long)

	DECLARE_FUNCTION3(long, SetupImageStore, wchar_t *, void*, long)

	DECLARE_FUNCTION1(long, SetScan, long)

	DECLARE_FUNCTION1(long, SetRegion, long)

	DECLARE_FUNCTION5(long, SaveData, void*, uint16_t, uint32_t, uint32_t, uint32_t)

	DECLARE_FUNCTION1(long, AdjustScanTCount, int)
};