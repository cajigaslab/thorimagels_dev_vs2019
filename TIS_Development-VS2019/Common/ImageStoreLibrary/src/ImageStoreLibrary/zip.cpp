#include <fstream>
#include ".\include\compat.h"
#include ".\include\libzip.h"
#include "zip.h"
#include "ImageStoreLibrary.h"

long zip_get_file_size(const char* file, const char* name)
{
	int errorp;
	zip_stat_t zst;
	zip_t* zip_t = zip_open(file, ZIP_RDONLY, &errorp);
	if (zip_t == NULL)		return ADDITIONAL_DATA_IS_NOT_EXIST;
	if (zip_stat(zip_t, name, ZIP_FL_ENC_GUESS, &zst) < 0)		return ADDITIONAL_DATA_IS_NOT_EXIST;
	zip_close(zip_t);
	return  (long)zst.size;
}

long zip_read_file(const char* file, const char* name, void* buffer, uint64_t size)
{
	int errorp;
	zip_stat_t zst;
	zip_t* zip_t = zip_open(file, ZIP_RDONLY, &errorp);
	if (zip_t == NULL)		return ADDITIONAL_DATA_IS_NOT_EXIST;
	if (zip_stat(zip_t, name, ZIP_FL_ENC_GUESS, &zst) < 0)		return ADDITIONAL_DATA_IS_NOT_EXIST;
	if (zst.size != size) return -1;
	zip_file_t* zip_file = zip_fopen(zip_t, name, ZIP_FL_COMPRESSED);
	if (zip_file == NULL)		return ADDITIONAL_DATA_IS_NOT_EXIST;
	long ret=(long)zip_fread(zip_file, buffer, zst.size);	
	zip_fclose(zip_file);
	zip_close(zip_t);
	return ret;
}

long zip_write_file(const char* file, const char* name, void* buffer, uint64_t size)
{
	zip_error_t error;
	int errorp;
	zip_t* zip_t = zip_open(file, ZIP_CREATE, &errorp);
	if (zip_t == NULL)		return ADDITIONAL_DATA_IS_NOT_EXIST;
	zip_source_t* zip_source = zip_source_buffer_create(buffer, size, 0, &error);
	if (zip_source == NULL)		return ADDITIONAL_DATA_IS_NOT_EXIST;
	long ret = (long)zip_file_add(zip_t, name, zip_source, ZIP_FL_OVERWRITE);
	zip_source_close(zip_source);
	zip_close(zip_t);	
	return ret == ZIP_ER_OK ? SUCCESS : UNKNOWN_ERROR;
}

long zip_delete_file(const char* file, const char* name)
{
	int errorp;
	zip_stat_t zst;
	zip_t* zip_t = zip_open(file, ZIP_RDONLY, &errorp);
	if (zip_t == NULL)		return ADDITIONAL_DATA_IS_NOT_EXIST;
	if (zip_stat(zip_t, name, ZIP_FL_ENC_GUESS, &zst) < 0)		return ADDITIONAL_DATA_IS_NOT_EXIST;
	long ret = zip_delete(zip_t, zst.index);
	zip_close(zip_t);
	return ret == ZIP_ER_OK ? SUCCESS : UNKNOWN_ERROR;
}
