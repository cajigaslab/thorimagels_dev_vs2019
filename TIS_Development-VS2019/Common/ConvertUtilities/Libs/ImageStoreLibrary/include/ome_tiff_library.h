#pragma once
#ifdef OME_TIFF_LIBRARY_EXPORTS
#define OME_TIFF_LIBRARY_API extern "C" __declspec(dllexport)
#else
#define OME_TIFF_LIBRARY_API extern "C" __declspec(dllimport)
#endif
#include "ome_struct.h"
/**********************************************************************************
Important Tips:

<1> Don't forget using namespace "ome".
<2> In this library, "Stride" means data's byte width(width * byte_count).

**********************************************************************************/



/**********************************************************************************
Function:      ome_open_file [Write\Read] [Required]

Description:   Open an OME-TIFF file for write or read.

Input:         "file_name" --> Full path with file name，e.g: c:\\test.tif
			   "mode"      --> Create mode,read write mode or read mode.
			   "cm"        --> Use LZ4 compression or none.An fast lossless compression method.

Return:        Return value >=0 --> Handle of Tiff file
			   Return value < 0 --> Error code defines by "ErrorCode" in "common_def.h".

Notes:         "cm" is useful in write or create mode and only for raw data saved by function ome_save_tile_data.
			   The pyramidal data's compression is set by ome_generate_pyramidal_data when them generate.
			   It need more CPU resource if you choise LZ4 compression mode.You shold take care about that. In some extreme case,
			   you can not save disk space when you choise an compression method.
**********************************************************************************/
OME_TIFF_LIBRARY_API long ome_open_file(const wchar_t* file_name, ome::OpenMode mode, ome::CompressionMode cm = ome::CompressionMode::COMPRESSIONMODE_NONE);


/**********************************************************************************
Function:      ome_close_file [Write\Read] [Required]

Description:   Close an OME-TIFF file has been opened.

Input:         "handle" --> The handle of an OME-TIFF file which has been opened.

Return:        Status code defines by "ErrorCode" in "common_def.h".

Notes:         Some works is been done when close TIFF file when save image.Don't forget to call it when finish saving.
**********************************************************************************/
OME_TIFF_LIBRARY_API long ome_close_file(int handle);

/**********************************************************************************
Function:      ome_set_plates [Write] [Optional]

Description:   Set plates info to TIFF.Plates tells the total size of the experiment and it include one or more Well inside.
			   The complete meaning of Plates info you can see in the definition of it.
			   Only useful in create mode and must be set before save image by ome_save_tile_data.

Input:         "handle"      --> The handle of an OME-TIFF file which has been opened.
			   "plates_info" --> The content of plates which is set.

Return:        Status code defines by "ErrorCode" in "common_def.h".

Notes:         Only can be set for once.It is useful in create mode only.
**********************************************************************************/
OME_TIFF_LIBRARY_API long ome_set_plates(int handle, ome::Plates* plates_info);


/**********************************************************************************
Function:      ome_add_scan [Write] [Required]

Description:   Add scan info to TIFF.Scans tells the image structure and it include one or more Regions and Channels.
			   The complete meaning of Scans info you can see in the definition of it.
			   Must be set before save image by ome_save_tile_data.

Input:         "handle"    --> The handle of an OME-TIFF file which has been opened.
			   "scan_info" --> The content of plates which is added.

Return:        Status code defines by "ErrorCode" in "common_def.h".
**********************************************************************************/
OME_TIFF_LIBRARY_API long ome_add_scan(int handle, ome::Scan* scan_info);


/**********************************************************************************
Function:      ome_save_tile_data [Write] [Required]

Description:   Save raw data to Tiff.Only save a tile's data once.Tile size is defined by "tile_pixel_size_width" and
			   "tile_pixel_size_height" in Scan.In tiff file,a large image is not stored by a whole file stream but separate
			   several tile.You need to assign the index of row and column when you save it.

			   For example, yot want to save a 512*512 image and you hope the tile_pixel_size_width and tile_pixel_size_height
			   are 256.In this case,you need to call ome_save_tile_data four times.The "tile_row" and "tile_column" set to (0,0) (0,1)
			   (1,0) (1,1) in four different calls and the buffer "image_data" carry a different 256*256 image everytime.

Input:         "handle"     --> The handle of an OME-TIFF file which has been opened.
			   "image_data" --> The buffer which carrys the image.Its size should equal or less than tile width* tile heigh.
			   "frame"      --> The message of current image.It means which scan,region,channel,z_position,stream_series,time_series it is.
								OME-TIFF contains region,channel,z_position,time_series to describe an microscopy experiment.
								you can find more infomation in https://www-legacy.openmicroscopy.org/.
			   "tile_row"   --> The row index located.Please see Description to get its mean.
			   "tile_column"--> The column index located.Please see Description to get its mean.
			   "stride"     --> The buffer stride of "image_data". Default parameter "0" means stride is equal with tile's width byte size.

Return:        Status code defines by "ErrorCode" in "common_def.h".

Notes:         Open file , set plates and add scan before save tile. If you want to save image easier, you can set "tile_pixel_size_width" and
			   "tile_pixel_size_height" in Scan same as image width and height.Then you can save a image once by set "tile_row" and "tile_column".
			   It could affect performance when you read a small area in a large image. You can do it like this if you don't care about that.
			   You don't need to care about when a whole image is all saved.
			   Take care about the "pixel_type".
**********************************************************************************/
OME_TIFF_LIBRARY_API long ome_save_tile_data(int handle, void* image_data, ome::FrameInfo frame, unsigned int tile_row, unsigned int tile_column, unsigned int stride = 0);

/**********************************************************************************
Function:      ome_purge_frame [Write] [Required]

Description:   When all tile data have been saved, you must call this function to save ifd of this frame.

Input:         "handle"      --> The handle of an OME-TIFF file which has been opened.
			   "frame"      --> The message of current image.It means which scan,region,channel,z_position,stream_series,time_series it is.
								OME-TIFF contains region,channel,z_position,time_series to describe an microscopy experiment.
								you can find more infomation in https://www-legacy.openmicroscopy.org/.

Return:        Status code defines by "ErrorCode" in "common_def.h".

Notes:         If you call this function before all tile data saved, you can not save remaining tile data.
**********************************************************************************/
OME_TIFF_LIBRARY_API long ome_purge_frame(int handle, ome::FrameInfo frame);

/**********************************************************************************
Function:      ome_get_plates [Read] [Optional]

Description:   Get plates infomation in an OME-TIFF file.If you don't need plates info you can get images without calling it.

Input:         "handle"      --> The handle of an OME-TIFF file which has been opened.

Output:        "plates_info" --> Get the plates info. You need to initialize the object before use the function.It return a copy of object in
								 using file.So, modifying it is useless.

Return:        Status code defines by "ErrorCode" in "common_def.h".

Notes:         Take care! If you want to use C++ / MT / MTD runtime library, don't call it with a reference objct in stack.Or it
			   will raise an error when the object release automatically.
**********************************************************************************/
OME_TIFF_LIBRARY_API long ome_get_plates(int handle, ome::Plates* plates_info);


/**********************************************************************************
Function:      ome_get_scans [Read] [Required]

Description:   Get scans infomation in an OME-TIFF file.Call it before loading image only in the case that you don't need the Scans infomation
			   to load image.

Input:         "handle"      --> The handle of an OME-TIFF file which has been opened.

Output:        "scans_info" --> Get the scans info. You need to initialize the object before use the function.It return a copy of object in
			   using file.So, modifying it is useless.

Return:        Status code defines by "ErrorCode" in "common_def.h".

Notes:         Take care! If you want to use C++ / MT / MTD runtime library, don't call it with a reference objct in stack.Or it
			   will raise an error when the object release automatically.
**********************************************************************************/
OME_TIFF_LIBRARY_API long ome_get_scans(int handle, ome::Scans* scans_info);


/**********************************************************************************
Function:      ome_get_raw_data [Read] [Required]

Description:   Load the raw data from OME-TIFF file.

Input:         "handle"    --> The handle of an OME-TIFF file which has been opened.
			   "frame"     --> The message of current image.
			   "src_rect"  --> The area you want to get in an image.
			   "stride"    --> The buffer stride of "image_data". Default parameter "0" means stride is equal with "src_rect" 's width byte size.

Output:        "image_data" --> Buffer to get the specific area of an image. You need to initialize the buffer before use the function.

Return:        Status code defines by "ErrorCode" in "common_def.h".

Notes:         Raw data is stored in the file like "test_0.tid".Data generates by pyramidal data if the raw data file is not exist.You can
			   delete the raw data to save disk if you really don't care the precise pixel value.
**********************************************************************************/
OME_TIFF_LIBRARY_API long ome_get_raw_data(int handle, ome::FrameInfo frame, ome::OmeRect src_rect, void * image_data, unsigned int stride = 0);

/**********************************************************************************
Function:      ome_get_scaled_raw_data [Read] [Optional]

Description:   Load the scale pyramidal data.Its pixelType is same as raw data. It get data by rect. It is different with
			   ome_get_scaled_raw_data(Get data by tile), ome_get_scaled_data and ome_get_scaled_tileData(Get data from 8bit JPEG).
			   You need to call ome_generate_pyramidal_data generating scale data before use this function.

Input:         "handle"    --> The handle of an OME-TIFF file which has been opened.
			   "frame"     --> The message of current image.
			   "dst_size"  --> The final size you want to get.
			   "src_rect"  --> The area you want to get in an image(For the whole raw data size).ImageStoreLibray will choise suitable
							   level of data by "dst_size" and "src_rect".It will generate by other kind of data if some kind of scale data
							   is not exist.It is same with ome_get_raw_data if "dst_size" equal to "src_rect".
			   "stride"    --> The buffer stride of "image_data". Default parameter "0" means stride is equal with "dst_size" 's width byte size.

Output:        "image_data" --> Buffer to get the specific area of an image. You need to initialize the buffer before use the function.Its size
								should be decided by "dst_size" and raw data's "pixelType".

Return:        Status code defines by "ErrorCode" in "common_def.h".

Notes:         Scale pyramidal data is stored in the file like "test_0_rsRaw.tid".Data generates by other data if the scale data file is not exist.
			   You can get scale data type by ome_get_image_file_type.
**********************************************************************************/
OME_TIFF_LIBRARY_API long ome_get_scaled_raw_data(int handle, ome::FrameInfo frame, ome::OmeSize dst_size, ome::OmeRect src_rect, void * image_data, unsigned int stride = 0);

/**********************************************************************************
Function:      ome_get_scaled_raw_tileData [Read] [Optional]

Description:   Load the scale pyramidal data.Its pixelType is same as raw data. It get data by tile and scaleLevel. It is different with
			   ome_get_scaled_raw_data(Get data by rect), ome_get_scaled_data and ome_get_scaled_tileData(Get data from 8bit JPEG).
			   You need to call ome_generate_pyramidal_data generating scale data before use this function.

Input:         "handle"     --> The handle of an OME-TIFF file which has been opened.
			   "frame"      --> The message of current image.
			   "scaleLevel" --> The scaleLevel you want to get.You can get all useable level by "scale_levels" in "ScanRegion".
			   "row"        --> The row index you want to get
			   "column"     --> The column index you want to get.
			   "stride"     --> The buffer stride of "image_data". Default parameter "0" means stride is equal with scale date tile's width byte size.


Output:        "image_data" --> Buffer to get the specific tile of an image. You need to initialize the buffer before use the function.
								Its width and height should be "thumbnail_tile_pixel_size" both.

Return:        Status code defines by "ErrorCode" in "common_def.h".

Notes:         Scale pyramidal data is stored in the file like "test_0_rsRaw.tid".Data generates by other data if the scale data file is not exist.
			   You can get scale data type by ome_get_image_file_type.
**********************************************************************************/
OME_TIFF_LIBRARY_API long ome_get_scaled_raw_tileData(int handle, ome::FrameInfo frame, unsigned short scaleLevel, unsigned short row, unsigned short column, void * image_data, unsigned int stride = 0);

/**********************************************************************************
Function:      ome_get_scaled_data [Read] [Optional]

Description:   Load the 8bit's JPEG scale pyramidal data. It get data by rect. It is different with
			   ome_get_scaled_tileData(Get 8bit data JPEG scale pyramidal by tile).
			   You need to call ome_generate_pyramidal_data generating scale data before use this function.

Input:         "handle"    --> The handle of an OME-TIFF file which has been opened.
			   "frame"     --> The message of current image.
			   "dst_size"  --> The final size you want to get.
			   "src_rect"  --> The area you want to get in an image(For the whole raw data size).ImageStoreLibray will choise suitable
							   level of data by "dst_size" and "src_rect".It will generate by other kind of data if some kind of scale data
							   is not exist.
			   "stride"    --> The buffer stride of "image_data". Default parameter "0" means stride is equal with "dst_size" 's width byte size.

Output:        "image_data" --> Buffer to get the specific area of an image. You need to initialize the buffer before use the function.Its size
								should be decided by "dst_size".Its "pixelType" is "PIXEL_UINT8".

Return:        Status code defines by "ErrorCode" in "common_def.h".

Notes:         8bit's JPEG scale pyramidal data is stored in the file like "test_0_jpg.tid".Data generates by other data if the scale data file is not exist.
			   You can get scale data type by ome_get_image_file_type.
**********************************************************************************/
OME_TIFF_LIBRARY_API long ome_get_scaled_data(int handle, ome::FrameInfo frame, ome::OmeSize dst_size, ome::OmeRect src_rect, void * image_data, unsigned int stride = 0);

/**********************************************************************************
Function:      ome_get_scaled_tileData [Read] [Optional]

Description:   Load the 8bit's JPEG scale pyramidal data. It get data by rect. It is different with
			   ome_get_scaled_raw_data(Get 8bit data JPEG scale pyramidal by rect).
			   You need to call ome_generate_pyramidal_data generating scale data before use this function.

Input:         "handle"     --> The handle of an OME-TIFF file which has been opened.
			   "frame"      --> The message of current image.
			   "scaleLevel" --> The scaleLevel you want to get.You can get all useable level by "scale_levels" in "ScanRegion".
			   "row"        --> The row index you want to get
			   "column"     --> The column index you want to get.
			   "stride"     --> The buffer stride of "image_data". Default parameter "0" means stride is equal with scale date tile's width byte size.

Output:        "image_data" --> Buffer to get the specific tile of an image. You need to initialize the buffer before use the function.
								Its width and height should be "thumbnail_tile_pixel_size" both.Its "pixelType" is "PIXEL_UINT8".

Return:        Status code defines by "ErrorCode" in "common_def.h".

Notes:         8bit's JPEG scale pyramidal data is stored in the file like "test_0_jpg.tid".Data generates by other data if the scale data file is not exist.
			   You can get scale data type by ome_get_image_file_type.
**********************************************************************************/
OME_TIFF_LIBRARY_API long ome_get_scaled_tileData(int handle, ome::FrameInfo frame, unsigned short scaleLevel, unsigned short row, unsigned short column, void * image_data, unsigned int stride = 0);


/**********************************************************************************
Function:      ome_get_image_file_type [Read] [Optional]

Description:   It tells whether the OME-TIFF file contains raw data,scale pyramidal data and 8bit's JPEG scale pyramidal data.

Input:         "handle" --> The handle of an OME-TIFF file which has been opened.

Output:        "image_file_type" --> It carry infomation by bit map with macro definition:
									 RAW_DATA             0x01 //"TRUE" means raw data exists
									 PYRAMIDAL_JPEG       0x02 //"TRUE" means 8bit's JPEG scale pyramidal data exists.

Return:        Status code defines by "ErrorCode" in "common_def.h".

Notes:         Raw data is stored in the file like "test_0.tid".Scale pyramidal data is stored in the file like "test_0_rsRaw.tid".
			   8bit's JPEG scale pyramidal data is stored in the file like "test_0_jpg.tid".You can use
			   ome_generate_pyramidal_data generating scale data.
**********************************************************************************/
OME_TIFF_LIBRARY_API long ome_get_image_file_type(int handle, unsigned char* image_file_type);


/**********************************************************************************
Function:      ome_generate_pyramidal_data [Write] [Optional]

Description:   Create scale pyramidal data or 8bit's JPEG scale pyramidal data.

Input:         "handle"           --> The handle of an OME-TIFF file which has been opened.			   
			   "progress_callback"-->Save pyramidal progress callback.If you don't need it,set it to nullptr.

Return:        Status code defines by "ErrorCode" in "common_def.h".

Notes:         Raw data is stored in the file like "test_0.tid".Scale pyramidal data is stored in the file like "test_0_rsRaw.tid".
			   8bit's JPEG scale pyramidal data is stored in the file like "test_0_jpg.tid".You can use
			   ome_get_image_file_type to get what kind of data it includes.
**********************************************************************************/
OME_TIFF_LIBRARY_API long ome_generate_pyramidal_data(int handle, ome::ProgressCallback progress_callback);


/**********************************************************************************
Function:      ome_save_additional_data [Write] [Optional]

Description:   Save additional infomation like experiment setting and so on.It will store in a zip package which is same name with TIFF file.

Input:         "handle"           --> The handle of an OME-TIFF file which has been opened.
			   "additional_data"  --> The buffer of infomation.
			   "size"             --> Buffer size of "additional_data".
			   "name"             --> File name in zip packeage.Not the name of zip.

Return:        Status code defines by "ErrorCode" in "common_def.h".
**********************************************************************************/
OME_TIFF_LIBRARY_API long ome_save_additional_data(int handle, void *additional_data, unsigned int size, char* name);


/**********************************************************************************
Function:      ome_get_additional_data_size [Read] [Optional]

Description:   Get additional infomation size saved by ome_save_additional_data.

Input:         "handle" --> The handle of an OME-TIFF file which has been opened.
			   "name"   --> File name in zip packeage.Not the name of zip.

Output:        "size" --> additional infomation size.

Return:        Status code defines by "ErrorCode" in "common_def.h".
**********************************************************************************/
OME_TIFF_LIBRARY_API long ome_get_additional_data_size(int handle, char* name, unsigned int * size);


/**********************************************************************************
Function:      ome_get_additional_data [Read] [Optional]

Description:   Get additional infomation saved by ome_save_additional_data.

Input:         "handle" --> The handle of an OME-TIFF file which has been opened.
			   "name"   --> File name in zip packeage.Not the name of zip.
			   "size"   --> Buffer size of "additional_data".

Output:        "additional_data" --> Get the additional data.Should malloc before use this function.

Return:        Status code defines by "ErrorCode" in "common_def.h".
**********************************************************************************/
OME_TIFF_LIBRARY_API long ome_get_additional_data(int handle, char* name, void *additional_data, unsigned int size);


/**********************************************************************************
Function:      ome_set_plates_xml [Write] [Optional]

Description:   Set plates info to TIFF for other lanuage which not support C++ struct like C#.Use xml instead of struct.The xml format like this:

			   <Plates>
				 <Plate ID="1" Name="Custom Carrier" Width="5000" Height="5000" PhysicalSizeXUnit="4" PhysicalSizeYUnit="4" Rows="2" Columns="1">
				   <Well ID="1" PositionX="0" PositionY="0" Width="500" Height="500" Shape="1" Row="0" Column="0"/>
				   <Well ID="2" PositionX="500" PositionY="0" Width="500" Height="500" Shape="1" Row="1" Column="0"/>
				 </Plate>
			   </Plates>

			   You can see the xml detail in ImageStoreLibrary.cs and call its set_plates function to set plates like ome_set_plates.
			   Plates tells the total size of the experiment and it include one or more Well inside.
			   The complete meaning of Plates info you can see in the definition of it.
			   Only useful in create mode and must be set before save image by ome_save_tile_data.

Input:         "handle"      --> The handle of an OME-TIFF file which has been opened.
			   "plates_info" --> The xml content of plates which is set.
			   "size"        --> Xml's size.

Return:        Status code defines by "ErrorCode" in "common_def.h".

Notes:         Only can be set for once.It is useful in create mode only.
**********************************************************************************/
OME_TIFF_LIBRARY_API long ome_set_plates_xml(int handle, void * plates_info, unsigned int size);


/**********************************************************************************
Function:      ome_add_scan_xml [Write] [Required]

Description:   Add scan info to TIFF for other lanuage which not support C++ struct like C#.Use xml instead of struct.The xml format like this:

			   <Scan ID="1" PlateID="1" PhysicalSizeX="2.3440001" PhysicalSizeY="2.3440001" PhysicalSizeZ="0.5" PhysicalSizeXUnit="4" PhysicalSizeYUnit="4"
									 PhysicalSizeZUnit="4" TimeIncrement="500" TimeIncrementUnit="2" DimensionOrder="XYCZT" TileWidth="256" TileHeight="256"
									 ThumbnailTileSize="512" Type="1" SignificantBits="14">
				 <Channels>
				   <Channel ID="0" Name="Chan A"/>
				   <Channel ID="1" Name="Chan B"/>
				</Channels>
				<ScanRegions>
				   <ScanRegion ID="0" WellID="1" SizeX="1024" SizeY="1024" SizeZ="2" SizeT="2" SizeS="1">
					  <WellSample PositionX="0" PositionY="0" PositionZ="0" SizeX="600" SizeY="600" SizeZ="0.5"
									 PhysicalSizeXUnit="4" PhysicalSizeYUnit="4" PhysicalSizeZUnit="4"/>
				   </ScanRegion>
				</ScanRegions>
			   </Scan>

			   You can see the xml detail in ImageStoreLibrary.cs and call its add_scan function to add scan like ome_add_scan.
			   Scans tells the image structure and it include one or more Regions and Channels.
			   The complete meaning of Scans info you can see in the definition of it.
			   Must be set before save image by ome_save_tile_data.

Input:         "handle"    --> The handle of an OME-TIFF file which has been opened.
			   "scan_info" --> The xml content of plates which is added.
			   "size"      --> Xml's size.

Return:        Status code defines by "ErrorCode" in "common_def.h".
**********************************************************************************/
OME_TIFF_LIBRARY_API long ome_add_scan_xml(int handle, void * scan_info, unsigned int size);


/**********************************************************************************
Function:      ome_get_plates_xml_size [Read] [Optional]

Description:   Get plates xml's size for other lanuage which not support C++ struct like C#.
			   If you don't need plates info you can get images without calling this.

Input:         "handle"      --> The handle of an OME-TIFF file which has been opened.

Output:        "size" --> Get the plates xml buffer's size.

Return:        Status code defines by "ErrorCode" in "common_def.h".
**********************************************************************************/
OME_TIFF_LIBRARY_API long ome_get_plates_xml_size(int handle, unsigned int* size);


/**********************************************************************************
Function:      ome_get_plates_xml [Read] [Optional]

Description:   Get plates infomation for other lanuage which not support C++ struct like C#.
			   If you don't need plates info you can get images without calling it.

Input:         "handle"--> The handle of an OME-TIFF file which has been opened.
			   "size"  --> Plates xml buffer's size.

Output:        "plates_info" --> Get the plates info. You need to initialize the xml buffer before use the function.

Return:        Status code defines by "ErrorCode" in "common_def.h".
**********************************************************************************/
OME_TIFF_LIBRARY_API long ome_get_plates_xml(int handle, void * plate_info, unsigned int size);


/**********************************************************************************
Function:      ome_get_scans_xml_size [Read] [Required]

Description:   Get scans xml'size for other lanuage which not support C++ struct like C#.
			   Call it before loading image only in the case that you don't need the Scans infomation to load image.

Input:         "handle" --> The handle of an OME-TIFF file which has been opened.

Output:        "size"  --> Get the scans xml buffer's size.

Return:        Status code defines by "ErrorCode" in "common_def.h".
**********************************************************************************/
OME_TIFF_LIBRARY_API long ome_get_scans_xml_size(int handle, unsigned int* size);


/**********************************************************************************
Function:      ome_get_scans [Read] [Required]

Description:   Get scans infomation in an OME-TIFF file.Call it before loading image only in the case that you don't need the Scans infomation
			   to load image.

Input:         "handle" --> The handle of an OME-TIFF file which has been opened.
			   "size"  --> Scans xml buffer's size.

Output:        "scans_info" --> Get the scans xml. You need to initialize the buffer before use the function.

Return:        Status code defines by "ErrorCode" in "common_def.h".
**********************************************************************************/
OME_TIFF_LIBRARY_API long ome_get_scans_xml(int handle, void * scan_infos, unsigned int size);


/**********************************************************************************
Function:      ome_get_histogram [Read] [Optional]

Description:   Get the histogram data saved in tiff file.The data in will be created automatical in save data.Raw data's histogram is different
			   with scales' data.It is a 3d volume's histogram, not a frame's.It mean that every C,T,S has a histogram data in a .

Input:         "handle"     --> The handle of an OME-TIFF file which has been opened.
			   "frame"      --> The message of current image. z_id is useless.
			   "scaleLevel" --> The scaleLevel you want to get histogram.You can get all useable level by "scale_levels" in "ScanRegion".
			   "histSize"   --> The histogram's size.You can get it in "Scan".

Output:        "pHist" --> The buffer to get data. Must malloc before call this function.

Return:        Status code defines by "ErrorCode" in "common_def.h".
**********************************************************************************/
OME_TIFF_LIBRARY_API long ome_get_histogram(int handle, ome::FrameInfo frame, unsigned short scaleLevel, unsigned int* pHist, unsigned int histSize);


/**********************************************************************************
Function:      ome_get_limit [Read] [Optional]

Description:   Get minimum and maxium value of raw data of a single frame saved in tiff file.

Input:         "handle"     --> The handle of an OME-TIFF file which has been opened.
			   "frame"      --> The message of current image.
			   "scaleLevel" --> The scaleLevel you want to get.You can get all useable level by "scale_levels" in "ScanRegion".
								scaleLevel == 0 will get RawData min and max; other will get JPEG pyramidal data min and max value.

Output:        "min"		--> The min for whole image.
			   "max"		--> The max for whole image.

Return:        Status code defines by "ErrorCode" in "common_def.h".
**********************************************************************************/
OME_TIFF_LIBRARY_API long ome_get_min_max(int handle, ome::FrameInfo frame, unsigned short scaleLevel, void* min, void* max);

/**********************************************************************************
Function:      ome_get_custom_tag [Read] [Optional]

Description:   Get minimum and maxium value of raw data of a single frame saved in tiff file.

Input:         "handle"     --> The handle of an OME-TIFF file which has been opened.
			   "frame"      --> The message of current image.
			   "tag_id"		--> Custom tag id, which you want to store value with(tag_id between(CustomTag_First, CustomTag_Last)).
			   "tag_size"	--> How many memory space you have allocated for the "tag_value"(size in byte).

Output:
			   "tag_value"  --> Value stored with tag_id.

Return:        Status code defines by "ErrorCode" in "common_def.h".
**********************************************************************************/
OME_TIFF_LIBRARY_API long ome_get_custom_tag(int handle, ome::FrameInfo frame, unsigned short tag_id, unsigned int tag_size, void* tag_value);

/**********************************************************************************
Function:      ome_set_custom_tag [Write] [Optional]

Description:   Get minimum and maxium value of raw data of a single frame saved in tiff file.

Input:         "handle"     --> The handle of an OME-TIFF file which has been opened.
			   "frame"      --> The message of current image.
			   "tag_id"		--> Custom tag id, which you want to get its value(tag_id between(CustomTag_First, CustomTag_Last) defined in "common_def.h").
			   "tag_type"	--> Element type of "tag_value".
			   "tag_count"	--> Element count of "tag_value".
			   "tag_value"  --> Value need to stored with tag_id.


Return:        Status code defines by "ErrorCode" in "common_def.h".
**********************************************************************************/
OME_TIFF_LIBRARY_API long ome_set_custom_tag(int handle, ome::FrameInfo frame, unsigned short tag_id, ome::TiffTagDataType tag_type, unsigned int tag_count, void* tag_value);