#include <stdio.h>
#include "tl_fenrir_sdk_load.h"
#include "tl_fenrir_sdk.h"

#ifdef _WIN32
#include "Windows.h"
HMODULE                        dll_handle = 0;
#else
#error // Not supported on Linux
#endif

TL_FENRIR_TIFF_OPEN                  tl_fenrir_tiff_open;
TL_FENRIR_TIFF_CLOSE                 tl_fenrir_tiff_close;
TL_FENRIR_TIFF_GET_NUM_IMAGES        tl_fenrir_tiff_get_num_images;
TL_FENRIR_TIFF_SET_IMAGE_INDEX       tl_fenrir_tiff_set_image_index;
TL_FENRIR_TIFF_GET_PIXEL_BUFFER      tl_fenrir_tiff_get_pixel_buffer;
TL_FENRIR_TIFF_GET_NUM_TAGS          tl_fenrir_tiff_get_num_tags;

TL_FENRIR_TIFF_GET_TAG_INFO          tl_fenrir_tiff_get_tag_info;
TL_FENRIR_TIFF_GET_TAG_ID            tl_fenrir_tiff_get_tag_id;
TL_FENRIR_TIFF_GET_TAG_TYPE          tl_fenrir_tiff_get_tag_type;
TL_FENRIR_TIFF_GET_TAG_SIZE_IN_BYTES tl_fenrir_tiff_get_tag_size_in_bytes;
TL_FENRIR_TIFF_GET_TAG_NUM_ELEMENTS  tl_fenrir_tiff_get_tag_num_elements;
TL_FENRIR_TIFF_GET_TAG_STR           tl_fenrir_tiff_get_tag_str;
TL_FENRIR_TIFF_GET_TAG_INT           tl_fenrir_tiff_get_tag_int;
TL_FENRIR_TIFF_GET_TAG_FLOAT         tl_fenrir_tiff_get_tag_float;
TL_FENRIR_TIFF_ADD_IMAGE             tl_fenrir_tiff_add_image;
TL_FENRIR_TIFF_SET_TAG               tl_fenrir_tiff_set_tag;

int tl_fenrir_initialize()
{
    if (dll_handle != NULL)
    {
        return TL_FENRIR_ERROR_ALREADY_INITIALIZED;
    }
#ifdef _WIN32
    dll_handle = LoadLibraryA("thorlabs_tsi_fenrir_sdk.dll");
#endif
    if (dll_handle == 0)
    {
        return TL_FENRIR_ERROR_UNABLE_TO_LOAD_LIBRARY;
    }
#ifdef _WIN32
    tl_fenrir_tiff_open = (TL_FENRIR_TIFF_OPEN)GetProcAddress(dll_handle, "tl_fenrir_tiff_open");
    tl_fenrir_tiff_close = (TL_FENRIR_TIFF_CLOSE)GetProcAddress(dll_handle, "tl_fenrir_tiff_close");
    tl_fenrir_tiff_get_num_images = (TL_FENRIR_TIFF_GET_NUM_IMAGES)GetProcAddress(dll_handle, "tl_fenrir_tiff_get_num_images");
    tl_fenrir_tiff_set_image_index = (TL_FENRIR_TIFF_SET_IMAGE_INDEX)GetProcAddress(dll_handle, "tl_fenrir_tiff_set_image_index");
    tl_fenrir_tiff_get_pixel_buffer = (TL_FENRIR_TIFF_GET_PIXEL_BUFFER)GetProcAddress(dll_handle, "tl_fenrir_tiff_get_pixel_buffer");
    tl_fenrir_tiff_get_num_tags = (TL_FENRIR_TIFF_GET_NUM_TAGS)GetProcAddress(dll_handle, "tl_fenrir_tiff_get_num_tags");
    tl_fenrir_tiff_get_tag_info = (TL_FENRIR_TIFF_GET_TAG_INFO)GetProcAddress(dll_handle, "tl_fenrir_tiff_get_tag_info");
    tl_fenrir_tiff_get_tag_id = (TL_FENRIR_TIFF_GET_TAG_ID)GetProcAddress(dll_handle, "tl_fenrir_tiff_get_tag_id");
    tl_fenrir_tiff_get_tag_type = (TL_FENRIR_TIFF_GET_TAG_TYPE)GetProcAddress(dll_handle, "tl_fenrir_tiff_get_tag_int");
    tl_fenrir_tiff_get_tag_size_in_bytes = (TL_FENRIR_TIFF_GET_TAG_SIZE_IN_BYTES)GetProcAddress(dll_handle, "tl_fenrir_tiff_get_tag_size_in_bytes");
    tl_fenrir_tiff_get_tag_num_elements = (TL_FENRIR_TIFF_GET_TAG_NUM_ELEMENTS)GetProcAddress(dll_handle, "tl_fenrir_tiff_get_tag_num_elements");
    tl_fenrir_tiff_get_tag_str = (TL_FENRIR_TIFF_GET_TAG_STR)GetProcAddress(dll_handle, "tl_fenrir_tiff_get_tag_str");
    tl_fenrir_tiff_get_tag_int = (TL_FENRIR_TIFF_GET_TAG_INT)GetProcAddress(dll_handle, "tl_fenrir_tiff_get_tag_int");
    tl_fenrir_tiff_get_tag_float = (TL_FENRIR_TIFF_GET_TAG_FLOAT)GetProcAddress(dll_handle, "tl_fenrir_tiff_get_tag_float");
    tl_fenrir_tiff_add_image = (TL_FENRIR_TIFF_ADD_IMAGE)GetProcAddress(dll_handle, "tl_fenrir_tiff_add_image");
    tl_fenrir_tiff_set_tag = (TL_FENRIR_TIFF_SET_TAG)GetProcAddress(dll_handle, "tl_fenrir_tiff_set_tag");
#endif

    if (!tl_fenrir_tiff_open)
        return TL_FENRIR_ERROR_FUNCTION_NOT_FOUND;
    if (!tl_fenrir_tiff_close)
        return TL_FENRIR_ERROR_FUNCTION_NOT_FOUND;
    if (!tl_fenrir_tiff_get_num_images)
        return TL_FENRIR_ERROR_FUNCTION_NOT_FOUND;
    if (!tl_fenrir_tiff_set_image_index)
        return TL_FENRIR_ERROR_FUNCTION_NOT_FOUND;
    if (!tl_fenrir_tiff_get_pixel_buffer)
        return TL_FENRIR_ERROR_FUNCTION_NOT_FOUND;
    if (!tl_fenrir_tiff_get_num_tags)
        return TL_FENRIR_ERROR_FUNCTION_NOT_FOUND;
    if (!tl_fenrir_tiff_get_tag_info)
        return TL_FENRIR_ERROR_FUNCTION_NOT_FOUND;
    if (!tl_fenrir_tiff_get_tag_id)
        return TL_FENRIR_ERROR_FUNCTION_NOT_FOUND;
    if (!tl_fenrir_tiff_get_tag_type)
        return TL_FENRIR_ERROR_FUNCTION_NOT_FOUND;
    if (!tl_fenrir_tiff_get_tag_size_in_bytes)
        return TL_FENRIR_ERROR_FUNCTION_NOT_FOUND;
    if (!tl_fenrir_tiff_get_tag_num_elements)
        return TL_FENRIR_ERROR_FUNCTION_NOT_FOUND;
    if (!tl_fenrir_tiff_get_tag_str)
        return TL_FENRIR_ERROR_FUNCTION_NOT_FOUND;
    if (!tl_fenrir_tiff_get_tag_int)
        return TL_FENRIR_ERROR_FUNCTION_NOT_FOUND;
    if (!tl_fenrir_tiff_get_tag_float)
        return TL_FENRIR_ERROR_FUNCTION_NOT_FOUND;
    if (!tl_fenrir_tiff_add_image)
        return TL_FENRIR_ERROR_FUNCTION_NOT_FOUND;
    if (!tl_fenrir_tiff_set_tag)
        return TL_FENRIR_ERROR_FUNCTION_NOT_FOUND;

    return TL_FENRIR_ERROR_NO_ERROR;
}

int tl_fenrir_terminate()
{
#ifdef _WIN32
    if (dll_handle != NULL)
    {
        FreeLibrary(dll_handle);
        dll_handle = NULL;
    }
#endif
    return TL_FENRIR_ERROR_NO_ERROR;
}
