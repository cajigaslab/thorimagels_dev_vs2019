#include <stdexcept>
#include <condition_variable>
#include <chrono>
#include <cassert>
#include <fstream> 
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include "RawDataReader.h"
#include "ClassicTiffConverter.h"

bool RawDataReader::_instanceFlag = false;
RawDataReader* RawDataReader::_single;

using namespace std;

ifstream rawFileStream;

RawDataReader::RawDataReader()
{
}

RawDataReader::~RawDataReader()
{
	_instanceFlag = false;
}

RawDataReader* RawDataReader::getInstance()
{
	if (!_instanceFlag)
	{
		_single = new RawDataReader();
		_instanceFlag = true;
	}
	return _single;
}

int RawDataReader::LoadRawDataFile(const wchar_t* file_name, int* image_handle)
{
	rawFileStream.open(file_name, ios::binary);
	if (!rawFileStream)
	{
		return -1;
	}

	return 0;
}

int RawDataReader::GetRawData(int image_handle, int offset, int size, void* image_data)
{
	rawFileStream.clear();
	rawFileStream.seekg(offset, ios::beg);
	if (!rawFileStream.eof()) {
		char* data = new char[size];
		memset(data, 0, size);
		rawFileStream.read(data, size);
		memcpy(image_data, data, size);
		delete[] data;
	}
	return 0;
}

int RawDataReader::CloseImage(int image_handle)
{
	rawFileStream.close();
	return 0;
}