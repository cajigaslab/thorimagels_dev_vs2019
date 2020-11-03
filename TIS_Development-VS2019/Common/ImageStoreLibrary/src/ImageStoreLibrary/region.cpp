#include "region.h"
#include "jpeg_handler.h"
#include <omp.h>
#include "storeconf.h"
#include "TiffData.h"
#include ".\include\ImageProcessLibraryBasic.h"

region::region()
{
}


region::~region()
{
	for (map<uint16_t, image_container*>::iterator iter = ThumbnailContainers.begin(); iter != ThumbnailContainers.end();)
	{
		delete (*iter).second;
		iter = ThumbnailContainers.erase(iter);
	}

	for (map<uint16_t, image_container*>::iterator iter = RawContainers.begin(); iter != RawContainers.end();)
	{
		delete (*iter).second;
		iter = RawContainers.erase(iter);
	}
}

image_container* region::findImageContainer(frame_info_ext frame, bool isThumbnail)
{
	if (isThumbnail) {
		for each (auto c in ThumbnailContainers)
		{
			if (c.second->ContainsFrame(frame)) {
				return c.second;
			}
		}
	}
	else {
		for each (auto c in RawContainers)
		{
			if (c.second->ContainsFrame(frame)) {
				return c.second;
			}
		}
	}
	return nullptr;
}

long region::CreateContainers(bool is_create_pyramidal_data)
{
	image_container* pContainer = nullptr;
	vector<frame_info_ext> frames;
	for (map<uint16_t, Channel*>::iterator channelit = _parentScan->Channels.begin(); channelit != _parentScan->Channels.end(); channelit++)
	{
		for (uint32_t s = 0; s < SizeS; s++)
		{
			for (uint32_t t = 0; t < SizeT; t++)
			{
				for (uint32_t z = 0; z < SizeZ; z++)
				{
					frame_info frame;
					frame.channel_id = channelit->second->ChannelID;
					frame.region_id = this->RegionID;
					frame.scan_id = this->ScanID;
					frame.s_id = s + 1;
					frame.time_id = t + 1;
					frame.z_id = z + 1;
					frame_info_ext frameExt(frame);
					frames.push_back(frameExt);
					if (frames.size() > 0xffff) {
						pContainer = addContainer(false);
						pContainer->SetupFramesIFD(frames);
						frames.clear();
					}
				}
			}
		}
	}
	if (!frames.empty()) {
		pContainer = addContainer(false);
		pContainer->SetupFramesIFD(frames);
		frames.clear();
	}

	if (is_create_pyramidal_data)
	{
		for (size_t scaleLevel = 0; scaleLevel <= MaxScaleLevel; scaleLevel++) {
			for (map<uint16_t, Channel*>::iterator channelit = _parentScan->Channels.begin(); channelit != _parentScan->Channels.end(); channelit++)
			{
				for (uint32_t s = 0; s < SizeS; s++)
				{
					for (uint32_t t = 0; t < SizeT; t++)
					{
						for (uint32_t z = 0; z < SizeZ; z++)
						{
							frame_info frame;
							frame.channel_id = channelit->second->ChannelID;
							frame.region_id = this->RegionID;
							frame.scan_id = this->ScanID;
							frame.s_id = s + 1;
							frame.time_id = t + 1;
							frame.z_id = z + 1;
							frame_info_ext frameExt(frame);
							frameExt.scaleLevel = (uint16_t)scaleLevel;
							frames.push_back(frameExt);
							if (frames.size() > 0xffff) {
								pContainer = addContainer(true);
								pContainer->SetupFramesIFD(frames);
								frames.clear();
							}
						}
					}
				}
			}
		}
		if (!frames.empty()) {
			pContainer = addContainer(true);
			pContainer->SetupFramesIFD(frames);
			frames.clear();
		}
	}
	return SUCCESS;
}

bool region::GetPyramidalLength(uint32_t actualLength, double pyramidalScale, uint32_t* pyramidalLength, uint32_t* subPyamidalLength, uint32_t* subPyamidalCount)
{
	uint32_t length;
	length = (uint32_t)(actualLength* pyramidalScale);

	*pyramidalLength = length;
	*subPyamidalLength = thumbnailTileSize;
	*subPyamidalCount = length % thumbnailTileSize == 0 ? length / thumbnailTileSize : length / thumbnailTileSize + 1;
	return length >= thumbnailTileSize;
}

long region::GeneratePyramidalData() {
	for each (auto channel in _parentScan->Channels)
	{
		auto channelId = channel.first;
		for (uint32_t s = 0; s < SizeS; s++)
		{
			for (uint32_t t = 0; t < SizeT; t++)
			{
				for (uint32_t z = 0; z < SizeZ; z++)
				{
					frame_info frame;
					frame.channel_id = channelId;
					frame.region_id = this->RegionID;
					frame.scan_id = this->ScanID;
					frame.s_id = s + 1;
					frame.time_id = t + 1;
					frame.z_id = z + 1;
					auto result = GeneratePyramidalData(frame);
					if (result != SUCCESS) {
						return result;
					}
				}
			}
		}
	}
	return SUCCESS;
}

long region::GeneratePyramidalData(frame_info frame)
{
	frame_info_ext rawFrame(frame);
	for (size_t scaleLevel = 0; scaleLevel <= MaxScaleLevel; scaleLevel++)
	{
		double scale = pyramidal_scale_list[scaleLevel];
		uint32 pyramidalWidth, subPyamidalWidth, pyramidalHeight, subPyamidalHeight;
		pyramidalWidth = ThumbImageSize[scaleLevel].width;
		pyramidalHeight = ThumbImageSize[scaleLevel].height;
		subPyamidalWidth = thumbnailTileSize;
		subPyamidalHeight = thumbnailTileSize;

		//create container if not exists
		bool createIFD = false;
		auto scaledFrame = frame_info_ext(frame);
		scaledFrame.scaleLevel = (uint16_t)scaleLevel;
		auto pContainer = findImageContainer(scaledFrame, true);
		if (pContainer == nullptr) {
			createIFD = true;
			if (ThumbnailContainers.size() > 0) {
				auto pLastContainer = ThumbnailContainers.end();
				pLastContainer--;
				pContainer = pLastContainer->second;
			}
		}
		if (pContainer == nullptr 
			|| createIFD && !pContainer->CanCreateIFD()) {
				pContainer = addContainer(true);
		}
		if (createIFD) {
			vector<frame_info_ext> frames;
			frames.push_back(scaledFrame);
			pContainer->SetupFramesIFD(frames);
		}

		auto significantBits = _parentScan->SignificantBits;
		uint16_t bitsPerSample, samplesPerPixel;
		samplesPerPixel = 1;
		bitsPerSample = ((_parentScan->SignificantBits / 8) + 1) * 8;
		void* buf = NULL;
		if (scale == 1.0)
			buf = malloc(subPyamidalWidth*subPyamidalHeight * bitsPerSample / 8 * samplesPerPixel);

		void* buf_8u = malloc(subPyamidalWidth*subPyamidalHeight);
		auto row = 0;
		for (uint32 y = 0; y < pyramidalHeight; y += subPyamidalHeight)
		{
			auto column = 0;
			for (uint32 x = 0; x < pyramidalWidth; x += subPyamidalWidth)
			{
				uint32 width = x + subPyamidalWidth > pyramidalWidth ? pyramidalWidth - x : subPyamidalWidth;
				uint32 height = y + subPyamidalHeight > pyramidalHeight ? pyramidalHeight - y : subPyamidalHeight;
				if (scale == 1.0)
				{
					IplRect srcRect = { x,y,width, height };
					image_container* pFromContainer = findImageContainer(rawFrame, false);
					pFromContainer->LoadRawRectData(frame, srcRect, buf);
					fnImageProcess_shift_buffer(buf, significantBits, buf_8u, 8, width*height);
				}
				else
				{
					IplSize dstSize = { width,height };
					IplRect srcRect = { (uint32)((double)x / scale), (uint32)((double)y / scale), (uint32)((double)width / scale), (uint32)((double)height / scale) };
					image_container* pFromContainer = findImageContainer(rawFrame, true);
					pFromContainer->loadPyramidalRectData(rawFrame, dstSize, srcRect, buf_8u);
				}
				pContainer->SaveTileData(buf_8u, width, scaledFrame, row, column);
				column++;
			}
			row++;
		}
		free(buf_8u);
		if (scale == 1.0)
			free(buf);
	}
	return SUCCESS;
}

image_container* region::addContainer(bool isThumbnail) {
	auto id = RawContainers.size() + ThumbnailContainers.size();
	auto container = new image_container((uint32_t)id, isThumbnail, this,CREATE_MODE);
	if (isThumbnail) {
		ThumbnailContainers.insert(make_pair(id, container));
	}
	else {
		RawContainers.insert(make_pair(id, container));
	}
	return container;
}

long region::Remove()
{
	for each (auto container in RawContainers)
	{
		container.second->Remove();
	}
	for each (auto container in ThumbnailContainers)
	{
		container.second->Remove();
	}
	return SUCCESS;
}

long region::SaveTileData(void * image_data, uint32_t stride, frame_info frame, unsigned int tile_row, unsigned int tile_column) {
	auto frameExt = frame_info_ext(frame);
	auto pContainer = findImageContainer(frameExt, false);
	if (pContainer==nullptr) {
		//todo: allow append frame here...
		return FRAMEINFO_IS_NOT_CORRECT;
	}
	return pContainer->SaveTileData(image_data, stride, frameExt, tile_row, tile_column);
}

long region::LoadRawRectData(frame_info frame, IplRect src_rect, void* buffer) {
	auto frameExt = frame_info_ext(frame);
	auto pContainer = findImageContainer(frameExt, false);
	if (pContainer == nullptr) {
		return FRAMEINFO_IS_NOT_CORRECT;
	}
	return pContainer->LoadRawRectData(frame, src_rect, buffer);
}

long region::LoadPyramidalRectData(frame_info frame, IplSize dst_size, IplRect src_rect, void* buffer)
{
	double scale = (double)dst_size.width / (double)src_rect.width > (double)dst_size.height / (double)src_rect.height ? (double)dst_size.width / (double)src_rect.width : (double)dst_size.height / (double)src_rect.height;
	image_container *pContainer = nullptr;

	for (int i = NUM_PYRAMIDAL - 1; i >= 0; i--)
	{
		if (pyramidal_scale_list[i] < scale)
			continue;

		IplRect pyramidal_roi;
		pyramidal_roi.x = (uint32)(src_rect.x*pyramidal_scale_list[i]);
		pyramidal_roi.y = (uint32)(src_rect.y*pyramidal_scale_list[i]);
		pyramidal_roi.width = (uint32)(src_rect.width*pyramidal_scale_list[i]);
		pyramidal_roi.height = (uint32)(uint32)(src_rect.height*pyramidal_scale_list[i]);
		auto frameExt = frame_info_ext(frame);
		frameExt.scaleLevel = i;
		auto pContainer = findImageContainer(frameExt, true);
		return pContainer->loadPyramidalRectData(frameExt, dst_size, src_rect, buffer);
	}
	pContainer = findImageContainer(frame_info_ext(frame), false);
	return pContainer->loadScaledRawRectData(frame, dst_size, src_rect, buffer);
}

long region::LoadPyramidalRectData(frame_info frame, uint16_t scaleLevel, uint16_t row, uint16_t column, void* buffer)
{
	auto frameExt = frame_info_ext(frame);
	frameExt.scaleLevel = scaleLevel;
	auto pContainer = findImageContainer(frameExt, true);
	if (pContainer == NULL)
		return false;
	else
		return pContainer->LoadPyramidalRectData(frameExt, row, column, buffer);
}

void region::Initialize()
{
	for (size_t scaleLevel = 0; scaleLevel < NUM_PYRAMIDAL; scaleLevel++)
	{
		double scale = pyramidal_scale_list[scaleLevel];
		uint32 pyramidalWidth, subPyamidalWidth, subPyamidalCountX, pyramidalHeight, subPyamidalHeight, subPyamidalCountY;
		bool isPyramidalX = GetPyramidalLength(SizeX, scale, &pyramidalWidth, &subPyamidalWidth, &subPyamidalCountX);
		bool isPyramidalY = GetPyramidalLength(SizeY, scale, &pyramidalHeight, &subPyamidalHeight, &subPyamidalCountY);
		if (!isPyramidalX && !isPyramidalY && scaleLevel > 0)
		{
			continue;
		}
		auto thumbnailSize = IplSize();
		thumbnailSize.width = pyramidalWidth;
		thumbnailSize.height = pyramidalHeight;
		ThumbImageSize.push_back(thumbnailSize);
	}
	MaxScaleLevel = (uint16_t)ThumbImageSize.size() - 1;
	if (ThumbImageSize.size() == 0) {
		MaxScaleLevel = 0;
	}
	// min limit 1 to create region containers
	SizeZ = max(1, SizeZ);
	SizeT = max(1, SizeT);
	SizeS = max(1, SizeS);
}
