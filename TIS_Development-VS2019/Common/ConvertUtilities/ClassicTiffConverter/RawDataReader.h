#pragma once

class RawDataReader
{
private:
	RawDataReader();
	static bool _instanceFlag;
	static RawDataReader* _single;

public:
	~RawDataReader();
	static RawDataReader* getInstance();

	static int LoadRawDataFile(const wchar_t* file_name, int* image_handle);
	static int GetRawData(int image_handle, int offset, int size, void* image_data);
	static int CloseImage(int image_handle);
};