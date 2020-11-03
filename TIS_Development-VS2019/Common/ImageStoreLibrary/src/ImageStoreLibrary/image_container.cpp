#include "scan.h"
#include "jpeg_handler.h"
#include "TiffData.h"
#include <fstream>
#include <omp.h>
#include ".\include\ImageProcessLibraryBasic.h"
#include "image_container.h"

void image_container::TIFFErrorProcExt(thandle_t hdl, const char* pModule, const char* pFormat, va_list pArg)
{
	char szMsg[512];
	vsprintf_s(szMsg, pFormat, pArg);
	printf("tiffErr-%s: %s\n", pModule, szMsg);
}


image_container::image_container(uint32_t id, bool isThumbnail, region* parentRegion, OpenMode mode, const char* fileName)
{
	_id = id;
	_isThumbnail = isThumbnail;
	_parentRegion = parentRegion;
	_mode = mode;
	if (!isThumbnail) {
		_tileWidth = _parentRegion->_parentScan->TileWidth;
		_tileHeight = _parentRegion->_parentScan->TileHeight;
		_imageWidth = _parentRegion->SizeX;
		_imageHeight = _parentRegion->SizeY;
	}

	Type = _parentRegion->_parentScan->Type;
	SignificantBits = _parentRegion->_parentScan->SignificantBits;
	_capacity = 65535;
	char buff[STRING_BUFFER_SIZE];

	if (fileName == nullptr)
	{
		sprintf_s(buff, "%s\\%s_scan_%d_region_%d_%d.tif",
			_parentRegion->_parentScan->parent_tiff->tiff_file_dir,
			_parentRegion->_parentScan->parent_tiff->tiff_file_name,
			_parentRegion->_parentScan->ScanID,
			_parentRegion->RegionID,
			id);
	}
	else
	{
		sprintf_s(buff, "%s\\%s",
			_parentRegion->_parentScan->parent_tiff->tiff_file_dir,
			fileName);
	}
	file_name = buff;

	if (OpenFile(mode) == SUCCESS)
	{
		TIFFSetErrorHandlerExt(TIFFErrorProcExt);
	}
}

image_container::~image_container()
{
	CloseFile();
	_parentRegion = nullptr;
}

long image_container::OpenFile(OpenMode openMode)
{
	if (tiff != NULL)
		return FILE_HAS_BEEN_OPENED;

	if (openMode == OpenMode::CREATE_MODE)
	{
		tiff = TIFFOpen(file_name.c_str(), "w8");
		if (tiff == NULL) return FAILED_FILE_NAME;
	}
	else if (openMode == OpenMode::READ_WRITE_MODE)
	{
		tiff = TIFFOpen(file_name.c_str(), "r+8");
		if (tiff == NULL) return FAILED_FILE_NAME;
	}
	else if (openMode == OpenMode::READ_ONLY_MODE)
	{
		tiff = TIFFOpen(file_name.c_str(), "r8");
		if (tiff == NULL) return FAILED_FILE_NAME;
	}
	else
	{
		return UNKNOWN_ERROR;
	}
	return SUCCESS;
}

long image_container::CloseFile()
{
	TIFFClose(tiff);
	tiff = nullptr;
	return SUCCESS;
}

interect_info* image_container::calculateInterectInfo(uint32 imageLength, uint32 tileLength, uint32 roiStart, uint32 roiLength, double scale, uint16* tileCount, uint32* dstImageLength)
{
	uint16_t tile_start = roiStart / tileLength;
	uint16_t tile_end = (roiStart + roiLength - 1) / tileLength;
	*dstImageLength = 0;
	*tileCount = (tile_end - tile_start + 1);
	interect_info*  infos = (interect_info*)malloc(*tileCount * sizeof(interect_info));
	for (uint16_t idx = tile_start; idx <= tile_end; idx++)
	{
		interect_info info;
		info.tile_index = idx;
		info.tile_start = roiStart <= idx * tileLength ? 0 : roiStart - idx * tileLength;
		info.tile_length = tileLength - info.tile_start - ((idx + 1)*tileLength <= (roiStart + roiLength) ? 0 : (idx + 1)*tileLength - (roiStart + roiLength));
		info.image_start = *dstImageLength;
		*dstImageLength += (uint32_t)(info.tile_length*scale);
		infos[idx - tile_start] = info;
	}
	return infos;
}

long image_container::SetupFramesIFD(std::vector<frame_info_ext> frames)
{
	if(_isThumbnail)
	{
		for (auto iframe = frames.begin(); iframe != frames.end(); ++iframe)
		{
			if (iframe->scaleLevel >= _parentRegion->ThumbImageSize.size()) {
				continue;
			}
			auto imageSizeRect = _parentRegion->ThumbImageSize[iframe->scaleLevel];
			

			TIFF_SET_FIELD_OR_FAIL(tiff, TIFFTAG_IMAGEWIDTH, static_cast<uint64_t>(imageSizeRect.width));
			TIFF_SET_FIELD_OR_FAIL(tiff, TIFFTAG_IMAGELENGTH, static_cast<uint64_t>(imageSizeRect.height));
			TIFF_SET_FIELD_OR_FAIL(tiff, TIFFTAG_TILEWIDTH, static_cast<uint64_t>(thumbnailTileSize));
			TIFF_SET_FIELD_OR_FAIL(tiff, TIFFTAG_TILELENGTH, static_cast<uint64_t>(thumbnailTileSize));
			TIFF_SET_FIELD_OR_FAIL(tiff, TIFFTAG_BITSPERSAMPLE, 8);
			TIFF_SET_FIELD_OR_FAIL(tiff, TIFFTAG_SAMPLESPERPIXEL, 1);
			TIFF_SET_FIELD_OR_FAIL(tiff, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
			TIFF_SET_FIELD_OR_FAIL(tiff, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
			TIFFCheckpointDirectory(tiff);
			TIFFWriteDirectory(tiff);
			_frameIfds.insert(make_pair(*iframe, _frameIfds.size()));
		}
	}
	else
	{
		for (auto iframe = frames.begin(); iframe != frames.end(); ++iframe)
		{
			TIFF_SET_FIELD_OR_FAIL(tiff, TIFFTAG_IMAGEWIDTH, _parentRegion->SizeX);
			TIFF_SET_FIELD_OR_FAIL(tiff, TIFFTAG_IMAGELENGTH, _parentRegion->SizeY);
			if (_parentRegion->SizeX != _tileWidth || _parentRegion->SizeY != _tileHeight)
			{
				TIFF_SET_FIELD_OR_FAIL(tiff, TIFFTAG_TILEWIDTH, _tileWidth);
				TIFF_SET_FIELD_OR_FAIL(tiff, TIFFTAG_TILELENGTH, _tileHeight);
			}
			switch (Type)
			{
			case PixelType_UINT8:
				TIFF_SET_FIELD_OR_FAIL(tiff, TIFFTAG_BITSPERSAMPLE, 8);
				break;
			case PixelType_INT16:
			case PixelType_UINT16:
				TIFF_SET_FIELD_OR_FAIL(tiff, TIFFTAG_BITSPERSAMPLE, 16);
				break;
			default:
				break;
			}
			TIFF_SET_FIELD_OR_FAIL(tiff, TIFFTAG_SAMPLESPERPIXEL, 1);
			TIFF_SET_FIELD_OR_FAIL(tiff, TIFFTAG_COMPRESSION, _parentRegion->_parentScan->parent_tiff->tag_compression);
			TIFF_SET_FIELD_OR_FAIL(tiff, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
			TIFF_SET_FIELD_OR_FAIL(tiff, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
			TIFFCheckpointDirectory(tiff);
			TIFFWriteDirectory(tiff);
			_frameIfds.insert(make_pair(*iframe, _frameIfds.size()));
		}
	}
	return SUCCESS;
}

long image_container::SaveTileData(void * image_data, uint32_t stride, frame_info_ext frame, unsigned int tile_row, unsigned int tile_column)
{
	int directoryID = _frameIfds[frame];
	if (directoryID < 0) return FRAMEINFO_IS_NOT_CORRECT;

	if (!TIFFSetDirectory(tiff, directoryID)) return UNKNOWN_ERROR;
	uint32_t imageWidth, imageHeight, tileWidth, tileHeight;
	uint16_t bits, channels;
	tmsize_t size = 0;
	TIFF_GET_FIELD_OR_FAIL(tiff, TIFFTAG_IMAGEWIDTH, imageWidth);
	TIFF_GET_FIELD_OR_FAIL(tiff, TIFFTAG_IMAGELENGTH, imageHeight);
	int isTiled = TIFFIsTiled(tiff);
	if (isTiled)
	{
		TIFF_GET_FIELD_OR_FAIL(tiff, TIFFTAG_TILEWIDTH, tileWidth);
		TIFF_GET_FIELD_OR_FAIL(tiff, TIFFTAG_TILELENGTH, tileHeight);
		TIFF_GET_FIELD_OR_FAIL(tiff, TIFFTAG_BITSPERSAMPLE, bits);
		TIFF_GET_FIELD_OR_FAIL(tiff, TIFFTAG_SAMPLESPERPIXEL, channels);
		uint32_t tile_size = tileWidth * tileHeight*(bits / 8)*channels;

		if (_isThumbnail) {
			unsigned char* dstBuf;
			unsigned long dstSize = 0;
			auto realHeight = min(imageHeight - tile_row * tileHeight, tileHeight);
			jpeg_compress((unsigned char*)image_data, &dstBuf, &dstSize, stride, realHeight);
			uint32_t tile_no = TIFFComputeTile(tiff, tileWidth*tile_column, tileHeight*tile_row, 0, 0);
			size = TIFFWriteRawTile(tiff, tile_no, dstBuf, dstSize);
			free(dstBuf);
		}
		else if (stride == tileWidth)
		{
			uint32_t tile_no = TIFFComputeTile(tiff, tileWidth*tile_column, tileHeight*tile_row, 0, 0);
			size = TIFFWriteEncodedTile(tiff, tile_no, (tdata_t)image_data, tile_size);
		}
		else
		{
			void* buf = malloc(tile_size);
			for (uint32_t row = 0; row < tileHeight; row++)
			{
				memcpy((char*)buf + row * tileWidth*(bits / 8)*channels, (char*)image_data + row * stride*(bits / 8)*channels, tileWidth*(bits / 8)*channels);
			}
			size = TIFFWriteTile(tiff, (tdata_t)buf, tileWidth*tile_column, tileHeight*tile_row, 0, 0);
			free(buf);
		}
	}
	else
	{
		size = TIFFWriteEncodedStrip(tiff, 0, (tdata_t)image_data, imageWidth*imageHeight * 2);
	}
	if (size <= 0)
		return UNKNOWN_ERROR;
	TIFFWriteDirectory(tiff);
	//TIFFReadDirectory(tiff);
	return SUCCESS;
}

long image_container::LoadRawRectData(frame_info frame, IplRect src_rect, void* buffer)
{
	int directoryID = _frameIfds[frame_info_ext(frame)];
	if (directoryID < 0) return FRAMEINFO_IS_NOT_CORRECT;
	lock_guard<mutex> lck(_readmt);
	if (!TIFFSetDirectory(tiff, directoryID)) return UNKNOWN_ERROR;
	uint32_t imageWidth, imageHeight, tileWidth, tileHeight;
	uint16_t bitsPerSample, samplesPerPixel;
	toff_t *sizes;
	int isTiled = TIFFIsTiled(tiff);
	imageWidth = _imageWidth;
	imageHeight = _imageHeight;
	TIFF_GET_FIELD_OR_FAIL(tiff, TIFFTAG_BITSPERSAMPLE, bitsPerSample);
	TIFF_GET_FIELD_OR_FAIL(tiff, TIFFTAG_SAMPLESPERPIXEL, samplesPerPixel);
	TIFF_GET_FIELD_OR_FAIL(tiff, isTiled ? TIFFTAG_TILEBYTECOUNTS : TIFFTAG_STRIPBYTECOUNTS, sizes);
	if (src_rect.x<0 || src_rect.y<0 || src_rect.x + src_rect.width>imageWidth || src_rect.y + src_rect.height>imageHeight)
		return ROI_IS_OUTOF_RANGE;
	if (isTiled)//tile data
	{
		TIFF_GET_FIELD_OR_FAIL(tiff, TIFFTAG_TILEWIDTH, tileWidth);
		TIFF_GET_FIELD_OR_FAIL(tiff, TIFFTAG_TILELENGTH, tileHeight);
		uint32 dstImageWidth, dstImageHeight;
		uint16 tileCountX, tileCountY;
		interect_info*  x_infos = calculateInterectInfo(imageWidth, tileWidth, src_rect.x, src_rect.width, 1.0, &tileCountX, &dstImageWidth);
		interect_info*  y_infos = calculateInterectInfo(imageHeight, tileHeight, src_rect.y, src_rect.height, 1.0, &tileCountY, &dstImageHeight);
		tsize_t tile_size = tileWidth * tileHeight * (bitsPerSample / 8)*samplesPerPixel;
		int threads = (int)ceil(omp_get_num_procs() / 2.0);
		omp_set_num_threads(threads);
		tdata_t buf = malloc(tile_size*threads);
		omp_lock_t* lock = new omp_lock_t;
		omp_init_lock(lock);
#pragma omp parallel for
		for (int n = 0; n < tileCountX*tileCountY; n++)
		{
			uint16_t i = n % tileCountX;
			uint16_t j = n / tileCountX;
			int thread_num = omp_get_thread_num();
			void* thread_buf = (uint8_t*)buf + tile_size * thread_num;
			memset(thread_buf, 0, tile_size);
			ttile_t tile_no = TIFFComputeTile(tiff, x_infos[i].tile_index *tileWidth, y_infos[j].tile_index*tileHeight, 0, 0);
			if (tile_no < 0 || sizes[tile_no] <= 0)
				continue;
			omp_set_lock(lock);
			tsize_t size = TIFFReadEncodedTile(tiff, tile_no, thread_buf, tile_size);
			omp_unset_lock(lock);
			if (size <= 0)
				continue;
			if (Type == PixelType_INT16 || Type == PixelType_UINT16)
				for (uint32_t line = 0; line < y_infos[j].tile_length; line++)
				{
					uint64_t src_offset = (y_infos[j].tile_start + line)*tileWidth + x_infos[i].tile_start;
					uint64_t dst_offset = (y_infos[j].image_start + line)*dstImageWidth + x_infos[i].image_start;
					memcpy((uint16_t*)buffer + dst_offset, (uint16_t*)thread_buf + src_offset, x_infos[i].tile_length*(bitsPerSample / 8)*samplesPerPixel);
				}
			else if (Type == PixelType_UINT8)
				for (uint32_t line = 0; line < y_infos[j].tile_length; line++)
				{
					uint64_t src_offset = (y_infos[j].tile_start + line)*tileWidth + x_infos[i].tile_start;
					uint64_t dst_offset = (y_infos[j].image_start + line)*dstImageWidth + x_infos[i].image_start;
					memcpy((uint8_t*)buffer + dst_offset, (uint8_t*)thread_buf + src_offset, x_infos[i].tile_length*(bitsPerSample / 8)*samplesPerPixel);
				}
		}
		free(buf);
	}
	else//strip data
	{
		if (sizes[0] <= 0)return UNKNOWN_ERROR;
		uint32_t dstImageWidth = (int)(src_rect.width);
		uint32_t dstImageHeight = (int)(src_rect.height);
		tsize_t image_size = imageWidth * imageHeight * (bitsPerSample / 8)*samplesPerPixel;

		if (src_rect.x == 0 && src_rect.y == 0 && src_rect.width == imageWidth && src_rect.height == imageHeight)
		{
			tsize_t size = TIFFReadEncodedStrip(tiff, 0, buffer, imageWidth*imageHeight * (bitsPerSample / 8)*samplesPerPixel);
			if (size < 0)
				return UNKNOWN_ERROR;
		}
		else
		{
			void* buf = malloc(image_size);
			memset(buf, 0, image_size);
			tsize_t size = TIFFReadEncodedStrip(tiff, 0, buf, imageWidth*imageHeight * (bitsPerSample / 8)*samplesPerPixel);
			if (size < 0)
			{
				free(buf);
				return UNKNOWN_ERROR;
			}
			long ret;
			IplRect dstRect = { 0,0 ,dstImageWidth,dstImageHeight };
			if (Type == PixelType_INT16)
				ret = fnImageProcess_resize_16s_C1R((int16_t*)buf, src_rect, (int)imageWidth, (int16_t*)buffer, dstRect, (int)(dstImageWidth));
			else if (Type == PixelType_UINT16)
				ret = fnImageProcess_resize_16u_C1R((uint16_t*)buf, src_rect, (int)imageWidth, (uint16_t*)buffer, dstRect, (int)(dstImageWidth));
			else if (Type == PixelType_UINT8)
				ret = fnImageProcess_resize_8u_C1R((uint8_t*)buf, src_rect, (int)imageWidth, (uint8_t*)buffer, dstRect, (int)(dstImageWidth));

			free(buf);
			if (ret < 0)
				return UNKNOWN_ERROR;
		}
	}
	return SUCCESS;
}

long image_container::Remove()
{
	if (!tiff)
		return UNKNOWN_ERROR;
	TIFFClose(tiff);
	tiff = NULL;
	remove(file_name.c_str());
	return SUCCESS;
}

long image_container::loadPyramidalRectData(frame_info_ext frame, IplSize dst_size, IplRect src_rect, void* buffer)
{
	int directoryID = _frameIfds[frame];
	if (directoryID < 0) return FRAMEINFO_IS_NOT_CORRECT;

	lock_guard<mutex> lck(_readmt);

	if (!TIFFSetDirectory(tiff, directoryID)) return UNKNOWN_ERROR;
	uint32_t imageWidth, imageHeight, tileWidth, tileHeight;
	uint16_t bitsPerSample, samplesPerPixel;
	toff_t *sizes;
	if (!TIFFIsTiled(tiff))
		return UNKNOWN_ERROR;
	TIFF_GET_FIELD_OR_FAIL(tiff, TIFFTAG_IMAGEWIDTH, imageWidth);
	TIFF_GET_FIELD_OR_FAIL(tiff, TIFFTAG_IMAGELENGTH, imageHeight);
	TIFF_GET_FIELD_OR_FAIL(tiff, TIFFTAG_BITSPERSAMPLE, bitsPerSample);
	TIFF_GET_FIELD_OR_FAIL(tiff, TIFFTAG_SAMPLESPERPIXEL, samplesPerPixel);
	TIFF_GET_FIELD_OR_FAIL(tiff, TIFFTAG_TILEBYTECOUNTS, sizes);
	TIFF_GET_FIELD_OR_FAIL(tiff, TIFFTAG_TILEWIDTH, tileWidth);
	TIFF_GET_FIELD_OR_FAIL(tiff, TIFFTAG_TILELENGTH, tileHeight);

	if (src_rect.x<0 || src_rect.y<0 || src_rect.x + src_rect.width>imageWidth || src_rect.y + src_rect.height>imageHeight)
		return UNKNOWN_ERROR;
	double scale_h = (double)dst_size.width / (double)src_rect.width;
	double scale_v = (double)dst_size.height / (double)src_rect.height;

	uint32 dstImageWidth, dstImageHeight;
	uint16 tileCountX, tileCountY;
	interect_info*  x_infos = calculateInterectInfo(imageWidth, tileWidth, src_rect.x, src_rect.width, scale_h, &tileCountX, &dstImageWidth);
	interect_info*  y_infos = calculateInterectInfo(imageHeight, tileHeight, src_rect.y, src_rect.height, scale_v, &tileCountY, &dstImageHeight);

	int threads = min((int)ceil(omp_get_num_procs() / 2.0), tileCountX*tileCountY);
	omp_set_num_threads(threads);
	omp_lock_t* lock = new omp_lock_t;
	omp_init_lock(lock);
	tdata_t buf = malloc(tileWidth*tileHeight*threads);
	tdata_t encoded_buf = malloc(tileWidth*tileHeight*threads);
	if (src_rect.width == dst_size.width&&src_rect.height == dst_size.height)
	{
#pragma omp parallel for
		for (int n = 0; n < tileCountX*tileCountY; n++)
		{
			uint16_t i = n % tileCountX;
			uint16_t j = n / tileCountX;
			int thread_num = omp_get_thread_num();
			void* thread_buf = (uint8_t*)buf + tileWidth * tileHeight*thread_num;
			void* thread_encoded_buf = (uint8_t*)encoded_buf + tileWidth * tileHeight*thread_num;
			memset(thread_buf, 0, tileWidth*tileHeight);
			memset(thread_encoded_buf, 0, tileWidth*tileHeight);
			ttile_t tile_no = TIFFComputeTile(tiff, x_infos[i].tile_index *tileWidth, y_infos[j].tile_index*tileHeight, 0, 0);
			uint32_t tile_actual_width = min(imageWidth - x_infos[i].tile_index *tileWidth, tileWidth);
			//uint32_t tile_actual_height = min(imageHeight - y_infos[j].tile_index *tileHeight, tileHeight);
			if (tile_no < 0)
				continue;
			omp_set_lock(lock);
			tsize_t size = TIFFReadRawTile(tiff, tile_no, thread_encoded_buf, sizes[tile_no]);
			omp_unset_lock(lock);
			if (size > 0)
			{
				if (jpeg_decompress((unsigned char*)thread_buf, (unsigned char*)thread_encoded_buf, (unsigned long)size) == 0)
				{
					for (uint32_t line = 0; line < y_infos[j].tile_length; line++)
					{
						uint64_t src_offset = (y_infos[j].tile_start + line)*tile_actual_width + x_infos[i].tile_start;
						uint64_t dst_offset = (y_infos[j].image_start + line)*dstImageWidth + x_infos[i].image_start;
						memcpy((uint8_t*)buffer + dst_offset, (uint8_t*)thread_buf + src_offset, x_infos[i].tile_length);
					}
				}
			}
		}
	}
	else
	{
		void* dstImage8u = NULL;
		if (dst_size.width != dstImageWidth || dst_size.height != dstImageHeight)
		{
			dstImage8u = malloc(dstImageWidth*dstImageHeight * (bitsPerSample / 8)*samplesPerPixel);
			memset(dstImage8u, 0, dstImageWidth*dstImageHeight * (bitsPerSample / 8)*samplesPerPixel);
		}
#pragma omp parallel for
		for (int n = 0; n < tileCountX*tileCountY; n++)
		{
			uint16_t i = n % tileCountX;
			uint16_t j = n / tileCountX;
			int thread_num = omp_get_thread_num();
			void* thread_buf = (uint8_t*)buf + tileWidth * tileHeight*thread_num;
			void* thread_encoded_buf = (uint8_t*)encoded_buf + tileWidth * tileHeight*thread_num;
			ttile_t tile_no = TIFFComputeTile(tiff, x_infos[i].tile_index *tileWidth, y_infos[j].tile_index*tileHeight, 0, 0);
			uint32_t tile_actual_width = min(imageWidth - x_infos[i].tile_index *tileWidth, tileWidth);
			if (tile_no < 0)
				continue;
			omp_set_lock(lock);
			tsize_t size = TIFFReadRawTile(tiff, tile_no, thread_encoded_buf, sizes[tile_no]);
			omp_unset_lock(lock);
			if (size > 0)
			{
				if (jpeg_decompress((unsigned char*)thread_buf, (unsigned char*)thread_encoded_buf, (unsigned long)size) == 0)
				{
					IplRect srcRect = { x_infos[i].tile_start,y_infos[j].tile_start ,x_infos[i].tile_length,y_infos[j].tile_length };
					IplRect dstRect = { x_infos[i].image_start,y_infos[j].image_start,(uint32_t)(x_infos[i].tile_length*scale_h),(uint32_t)(y_infos[j].tile_length*scale_v) };
					if (dst_size.width == dstImageWidth && dst_size.height == dstImageHeight)
					{
						fnImageProcess_resize_8u_C1R((uint8_t*)thread_buf, srcRect, tile_actual_width, (uint8_t*)buffer, dstRect, dstImageWidth);
					}
					else
						fnImageProcess_resize_8u_C1R((uint8_t*)thread_buf, srcRect, tile_actual_width, (uint8_t*)dstImage8u, dstRect, dstImageWidth);
				}
			}
		}
		if (dst_size.width != dstImageWidth || dst_size.height != dstImageHeight)
		{
			IplRect srcRect = { 0,0 ,dstImageWidth,dstImageHeight };
			IplRect dstRect = { 0,0 ,dst_size.width,dst_size.height };
			fnImageProcess_resize_8u_C1R((uint8_t*)dstImage8u, srcRect, (int)dstImageWidth, (uint8_t*)buffer, dstRect, (int)(dst_size.width));
			free(dstImage8u);
		}
	}


	free(buf);
	free(encoded_buf);
	free(x_infos);
	free(y_infos);
	return SUCCESS;
}

long image_container::loadScaledRawRectData(frame_info frame, IplSize dst_size, IplRect src_rect, void* buffer)
{
	long ret;
	if (Type == PixelType_UINT8)
	{
		if (dst_size.width == src_rect.width&&dst_size.height == src_rect.height)
		{
			return LoadRawRectData(frame, src_rect, buffer);
		}
		else
		{
			void* buf_raw = malloc((uint64_t)src_rect.width*(uint64_t)src_rect.height);
			memset(buf_raw, 0, (uint64_t)src_rect.width*(uint64_t)src_rect.height);
			ret = LoadRawRectData(frame, src_rect, buf_raw);
			if (ret != SUCCESS)
			{
				free(buf_raw);
				return ret;
			}
			IplRect srcRect = { 0,0 ,src_rect.width,src_rect.height };
			IplRect dstRect = { 0,0 ,dst_size.width,dst_size.height };
			fnImageProcess_resize_8u_C1R((uint8_t*)buf_raw, srcRect, src_rect.width, (uint8_t*)buffer, dstRect, dst_size.width);
			free(buf_raw);
			if (ret < 0)
				return UNKNOWN_ERROR;
		}
	}
	if (Type == PixelType_INT16 || Type == PixelType_UINT16)
	{
		void* buf_raw = malloc((uint64_t)src_rect.width*(uint64_t)src_rect.height * 2);
		memset(buf_raw, 0, (uint64_t)src_rect.width*(uint64_t)src_rect.height * 2);
		ret = LoadRawRectData(frame, src_rect, buf_raw);
		if (ret != SUCCESS)
		{
			free(buf_raw);
			return ret;
		}
		if (dst_size.width == src_rect.width&&dst_size.height == src_rect.height)
		{
			fnImageProcess_shift_buffer(buf_raw, SignificantBits, buffer, 8, (uint64_t)src_rect.width*(uint64_t)src_rect.height);
		}
		else
		{
			void* buf_raw_8u = malloc((uint64_t)src_rect.width*(uint64_t)src_rect.height);
			memset(buf_raw_8u, 0, (uint64_t)src_rect.width*(uint64_t)src_rect.height);
			fnImageProcess_shift_buffer(buf_raw, SignificantBits, buf_raw_8u, 8, (uint64_t)src_rect.width*(uint64_t)src_rect.height);
			IplRect srcRect = { 0,0 ,src_rect.width,src_rect.height };
			IplRect dstRect = { 0,0 ,dst_size.width,dst_size.height };
			long ret = fnImageProcess_resize_8u_C1R((uint8_t*)buf_raw_8u, srcRect, (int)src_rect.width, (uint8_t*)buffer, dstRect, (int)(dst_size.width));
			free(buf_raw_8u);
			if (ret <= 0)
			{
				free(buf_raw);
				return UNKNOWN_ERROR;
			}
		}
		free(buf_raw);
	}
	return SUCCESS;
}

long image_container::LoadTileData(frame_info_ext frame, uint16_t row, uint16_t column, void* buffer) {
	uint32_t imageWidth, imageHeight, tileWidth, tileHeight;
	int directoryID = _frameIfds[frame];
	if (directoryID < 0) return FRAMEINFO_IS_NOT_CORRECT;

	lock_guard<mutex>* lck = new lock_guard<mutex>(_readmt);

	if (!TIFFSetDirectory(tiff, directoryID)) return UNKNOWN_ERROR;

	uint16_t bitsPerSample, samplesPerPixel;
	toff_t *sizes;
	if (!TIFFIsTiled(tiff))
		return UNKNOWN_ERROR;
	TIFF_GET_FIELD_OR_FAIL(tiff, TIFFTAG_IMAGEWIDTH, imageWidth);
	TIFF_GET_FIELD_OR_FAIL(tiff, TIFFTAG_IMAGELENGTH, imageHeight);
	TIFF_GET_FIELD_OR_FAIL(tiff, TIFFTAG_BITSPERSAMPLE, bitsPerSample);
	TIFF_GET_FIELD_OR_FAIL(tiff, TIFFTAG_SAMPLESPERPIXEL, samplesPerPixel);
	TIFF_GET_FIELD_OR_FAIL(tiff, TIFFTAG_TILEBYTECOUNTS, sizes);
	TIFF_GET_FIELD_OR_FAIL(tiff, TIFFTAG_TILEWIDTH, tileWidth);
	TIFF_GET_FIELD_OR_FAIL(tiff, TIFFTAG_TILELENGTH, tileHeight);

	//Row/Col max start from zero;
	uint16 tileRowMax, tileColumnMax;
	tileColumnMax = imageWidth % tileWidth == 0 ? imageWidth / tileWidth - 1 : imageWidth / tileWidth;
	tileRowMax = imageHeight % tileHeight == 0 ? imageHeight / tileHeight - 1 : imageHeight / tileHeight;

	if (row < 0 || row > tileRowMax) return TILE_ROW_IS_NOT_CORRECT;
	if (column < 0 || column > tileColumnMax) return TILE_COLUMN_IS_NOT_CORRECT;

	ttile_t tile_no = TIFFComputeTile(tiff, column*tileWidth, row*tileHeight, 0, 0);

	size_t thumbImgSize = tileWidth * tileHeight*bitsPerSample / 8;
	tdata_t buf;
	tdata_t encoded_buf = malloc(thumbImgSize);
	memset(encoded_buf, 0, thumbImgSize);

	tsize_t size;
	if (_isThumbnail) {
		size = TIFFReadRawTile(tiff, tile_no, encoded_buf, sizes[tile_no]);
	}
	else {
		size = TIFFReadEncodedTile(tiff, tile_no, encoded_buf, sizes[tile_no]);
	}

	delete lck;

	if (size > 0)
	{
		bool flag = true;
		if (_isThumbnail) {
			buf = malloc(thumbImgSize);
			memset(buf, 0, thumbImgSize);
			flag = jpeg_decompress((unsigned char*)buf, (unsigned char*)encoded_buf, (unsigned long)size) == 0;
			free(encoded_buf);
		}
		else {
			buf = encoded_buf;
		}
		if (flag) {
			
			if (row < tileRowMax && column < tileColumnMax)
			{
				memcpy_s(buffer, thumbImgSize, buf, thumbImgSize);
			}
			else
			{
				// Not a full tile image.
				uint32_t rightWidth = imageWidth - column * tileWidth;
				uint32_t bottomHeight = imageHeight - row * tileHeight;
				uint32_t copyWidth = rightWidth > tileWidth ? tileWidth : rightWidth;
				uint32_t copyHeight = bottomHeight > tileHeight ? tileHeight : bottomHeight;

				memset(buffer, 0, copyWidth* copyHeight*bitsPerSample / 8);
				memcpy_s(buffer, copyWidth* copyHeight*bitsPerSample / 8, buf, copyWidth* copyHeight*bitsPerSample / 8);
			}
			free(buf);
			return SUCCESS;
		}
	}
	free(encoded_buf);
	return UNKNOWN_ERROR;
}

long image_container::LoadPyramidalRectData(frame_info_ext frame, uint16_t row, uint16_t column, void* buffer)
{
	int directoryID = _frameIfds[frame];
	if (directoryID < 0) return FRAMEINFO_IS_NOT_CORRECT;
	if (tiff == NULL) return UNKNOWN_ERROR;

	lock_guard<mutex>* lck = new lock_guard<mutex>(_readmt);
	if (!TIFFSetDirectory(tiff, directoryID)) return UNKNOWN_ERROR;
	uint32_t imageWidth, imageHeight, tileWidth, tileHeight;
	uint16_t bitsPerSample, samplesPerPixel;
	toff_t *sizes;
	if (!TIFFIsTiled(tiff))
		return UNKNOWN_ERROR;
	TIFF_GET_FIELD_OR_FAIL(tiff, TIFFTAG_IMAGEWIDTH, imageWidth);
	TIFF_GET_FIELD_OR_FAIL(tiff, TIFFTAG_IMAGELENGTH, imageHeight);
	TIFF_GET_FIELD_OR_FAIL(tiff, TIFFTAG_BITSPERSAMPLE, bitsPerSample);
	TIFF_GET_FIELD_OR_FAIL(tiff, TIFFTAG_SAMPLESPERPIXEL, samplesPerPixel);
	TIFF_GET_FIELD_OR_FAIL(tiff, TIFFTAG_TILEBYTECOUNTS, sizes);
	TIFF_GET_FIELD_OR_FAIL(tiff, TIFFTAG_TILEWIDTH, tileWidth);
	TIFF_GET_FIELD_OR_FAIL(tiff, TIFFTAG_TILELENGTH, tileHeight);

	//Row/Col max start from zero;
	uint16 tileRowMax, tileColumnMax;
	tileColumnMax = imageWidth % tileWidth == 0 ? imageWidth / tileWidth - 1 : imageWidth / tileWidth;
	tileRowMax = imageHeight % tileHeight == 0 ? imageHeight / tileHeight - 1 : imageHeight / tileHeight;

	if (row < 0 || row > tileRowMax) return TILE_ROW_IS_NOT_CORRECT;
	if (column < 0 || column > tileColumnMax) return TILE_COLUMN_IS_NOT_CORRECT;

	ttile_t tile_no = TIFFComputeTile(tiff, column*tileWidth, row*tileHeight, 0, 0);

	size_t thumbImgSize = tileWidth * tileHeight;
	tdata_t buf = malloc(thumbImgSize);
	memset(buf, 0, thumbImgSize);
	tdata_t encoded_buf = malloc(sizes[tile_no]);
	memset(encoded_buf, 0, sizes[tile_no]);
	tsize_t size = TIFFReadRawTile(tiff, tile_no, encoded_buf, sizes[tile_no]);



	delete lck;

	if (size > 0)
	{
		if (jpeg_decompress((unsigned char*)buf, (unsigned char*)encoded_buf, (unsigned long)size) == 0)
		{
			if (row < tileRowMax && column < tileColumnMax)
			{
				memcpy_s(buffer, thumbImgSize, buf, thumbImgSize);
			}
			else
			{
				// Not a full tile image.
				uint32_t rightWidth = imageWidth - column * tileWidth;
				uint32_t bottomHeight = imageHeight - row * tileHeight;
				uint32_t copyWidth = rightWidth > tileWidth ? tileWidth : rightWidth;
				uint32_t copyHeight = bottomHeight > tileHeight ? tileHeight : bottomHeight;

				memset(buffer, 0, copyWidth*copyHeight);
				memcpy_s(buffer, copyWidth*copyHeight, buf, copyWidth*copyHeight);
			}

			free(buf);
			free(encoded_buf);
			return SUCCESS;
		}
	}
	free(buf);
	free(encoded_buf);
	return UNKNOWN_ERROR;
}

bool image_container::CanCreateIFD() {
	return _frameIfds.size() < _capacity;
}

string image_container::GetFileName()
{
	char* fileName= strrchr((char*)file_name.c_str(), '\\');
	fileName++;
	return fileName;
}

bool image_container::ContainsFrame(frame_info_ext frame) {
	return _frameIfds.find(frame) != _frameIfds.end();
}