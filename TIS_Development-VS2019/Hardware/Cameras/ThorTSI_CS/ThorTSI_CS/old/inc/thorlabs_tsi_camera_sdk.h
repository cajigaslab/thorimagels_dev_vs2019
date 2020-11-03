/*
* Copyright 2017 by Thorlabs, Inc.  All rights reserved.  Unauthorized use (including,
* without limitation, distribution and copying) is strictly prohibited.  All use requires, and is
* subject to, explicit written authorization and nondisclosure agreements with Thorlabs, Inc.
*/

#pragma once

enum TRIGGER_TYPE { NONE, STANDARD, BULB, TRIGGER_TYPE_MAX };
enum TRIGGER_POLARITY { ACTIVE_HIGH, ACTIVE_LOW, TRIGGER_POLARITY_MAX };
enum EEP_STATUS { OFF, ON_ACTIVE, ON_INACTIVE, ON_BULB, EEP_STATUS_MAX };
enum DATA_RATE { RESERVED1, RESERVED2, FPS_30, FPS_50, DATA_RATE_MAX };
enum USB_BUS_SPEED {USB1_0, USB2_0, USB3_0, USB_BUS_SPEED_MAX};

typedef void(*TL_CAMERA_FRAME_AVAILABLE_CALLBACK)(void* sender, unsigned short* image_buffer, int image_width, int image_height, int bit_depth, int number_of_color_channels, int frame_count, void* context);
typedef void(*TL_CAMERA_CAMERA_CONNECT_CALLBACK)(char* cameraSerialNumber, enum USB_BUS_SPEED usb_bus_speed, void* context);
typedef void(*TL_CAMERA_CAMERA_DISCONNECT_CALLBACK)(char* cameraSerialNumber, void* context);

typedef void(*TL_CAMERA_SET_FRAME_AVAILABLE_CALLBACK)(void* tl_camera_handle, TL_CAMERA_FRAME_AVAILABLE_CALLBACK handler, void* context);
typedef int(*TL_CAMERA_SET_CAMERA_CONNECT_CALLBACK)(TL_CAMERA_CAMERA_CONNECT_CALLBACK handler, void* context);
typedef int(*TL_CAMERA_SET_CAMERA_DISCONNECT_CALLBACK)(TL_CAMERA_CAMERA_DISCONNECT_CALLBACK handler, void* context);
typedef int(*TL_CAMERA_OPEN_SDK)();
typedef int(*TL_CAMERA_CLOSE_SDK)();
typedef char* (*TL_CAMERA_GET_LAST_ERROR)();
typedef int(*TL_CAMERA_GET_AVAILABLE_CAMERAS)(char* serial_numbers, int str_length);
typedef int(*TL_CAMERA_INTERNAL_COMMAND)(void* tl_camera_handle, char* data, char* response, int response_size);
typedef int(*TL_CAMERA_GET_EXPOSURE_US)(void* tl_camera_handle, int* exposure);
typedef int(*TL_CAMERA_SET_EXPOSURE_US)(void* tl_camera_handle, int exposure);
typedef int(*TL_CAMERA_GET_EXPOSURE_RANGE_US)(void* tl_camera_handle, int* exposure_min, int* exposure_max);
typedef int(*TL_CAMERA_GET_FIRMWARE_VERSION)(void* tl_camera_handle, char* firmware_version, int str_length);
typedef int(*TL_CAMERA_GET_FRAME_TIME_US)(void* tl_camera_handle, int* frame_time);
typedef int(*TL_CAMERA_GET_MEASURED_FRAMES_PER_SECOND)(void* tl_camera_handle, double* frame_rate_param);
typedef int(*TL_CAMERA_GET_HARDWARE_TRIGGER_MODE)(void* tl_camera_handle, enum TRIGGER_TYPE* trigger_type_enum, enum TRIGGER_POLARITY* trigger_polarity_enum);
typedef int(*TL_CAMERA_SET_HARDWARE_TRIGGER_MODE)(void* tl_camera_handle, enum TRIGGER_TYPE trigger_type_enum, enum TRIGGER_POLARITY trigger_polarity_enum);
typedef int(*TL_CAMERA_GET_HBIN)(void* tl_camera_handle, int* hbin);
typedef int(*TL_CAMERA_SET_HBIN)(void* tl_camera_handle, int hbin);
typedef int(*TL_CAMERA_GET_HBIN_RANGE)(void* tl_camera_handle, int* hbin_min, int* hbin_max);
typedef int(*TL_CAMERA_GET_HOT_PIXEL_CORRECTION)(void* tl_camera_handle, int* hot_pixel_correction);
typedef int(*TL_CAMERA_SET_HOT_PIXEL_CORRECTION)(void* tl_camera_handle, int hot_pixel_correction);
typedef int(*TL_CAMERA_GET_HOT_PIXEL_CORRECTION_THRESHOLD)(void* tl_camera_handle, int* hot_pixel_correction_threshold);
typedef int(*TL_CAMERA_SET_HOT_PIXEL_CORRECTION_THRESHOLD)(void* tl_camera_handle, int hot_pixel_correction_threshold);
typedef int(*TL_CAMERA_GET_HOT_PIXEL_CORRECTION_THRESHOLD_RANGE)(void* tl_camera_handle, int* hot_pixel_correction_threshold_min, int* hot_pixel_correction_threshold_max);
typedef int(*TL_CAMERA_GET_IMAGE_WIDTH_PIXELS)(void* tl_camera_handle, int* width);
typedef int(*TL_CAMERA_GET_IMAGE_WIDTH_RANGE_PIXELS)(void* tl_camera_handle, int* image_width_min, int* image_width_max);
typedef int(*TL_CAMERA_GET_IMAGE_HEIGHT_PIXELS)(void* tl_camera_handle, int* height);
typedef int(*TL_CAMERA_GET_IMAGE_HEIGHT_RANGE_PIXELS)(void* tl_camera_handle, int* image_height_min, int* image_height_max);
typedef int(*TL_CAMERA_GET_MODEL)(void* tl_camera_handle, char* model, int str_length);
typedef int(*TL_CAMERA_GET_MODEL_STRING_LENGTH_RANGE)(void* tl_camera_handle, int* model_min, int* model_max);
typedef int(*TL_CAMERA_GET_NAME)(void* tl_camera_handle, char* name, int str_length);
typedef int(*TL_CAMERA_SET_NAME)(void* tl_camera_handle, char* name);
typedef int(*TL_CAMERA_GET_NAME_STRING_LENGTH_RANGE)(void* tl_camera_handle, int* name_min, int* name_max);
typedef int(*TL_CAMERA_GET_NUMBER_OF_FRAMES_PER_TRIGGER)(void* tl_camera_handle, long* number_of_frames);
typedef int(*TL_CAMERA_SET_NUMBER_OF_FRAMES_PER_TRIGGER)(void* tl_camera_handle, long number_of_frames);
typedef int(*TL_CAMERA_GET_NUMBER_OF_FRAMES_PER_TRIGGER_RANGE)(void* tl_camera_handle, long* nframes_min, long* nframes_max);
typedef int(*TL_CAMERA_GET_USB_BUS_SPEED)(void* tl_camera_handle, int* usb_bus_speed);
typedef int(*TL_CAMERA_GET_DATA_RATE_SUPPORTED)(void* tl_camera_handle, enum DATA_RATE data_rate, int* is_supported);
typedef int(*TL_CAMERA_GET_DATA_RATE)(void* tl_camera_handle, enum DATA_RATE* data_rate);
typedef int(*TL_CAMERA_SET_DATA_RATE)(void* tl_camera_handle, enum DATA_RATE data_rate);
typedef int(*TL_CAMERA_GET_PIXEL_SIZE_BYTES)(void* tl_camera_handle, int* pixel_size_bytes);
typedef int(*TL_CAMERA_GET_PIXEL_WIDTH_UM)(void* tl_camera_handle, double* pixel_width_um);
typedef int(*TL_CAMERA_GET_PIXEL_HEIGHT_UM)(void* tl_camera_handle, double* pixel_height_um);
typedef int(*TL_CAMERA_GET_PIXEL_BIT_DEPTH)(void* tl_camera_handle, int* pixel_bit_depth);
typedef int(*TL_CAMERA_GET_ROI)(void* tl_camera_handle, int* upper_left_x, int* upper_left_y, int* lower_right_x, int* lower_right_y);
typedef int(*TL_CAMERA_SET_ROI)(void* tl_camera_handle, int upper_left_x, int upper_left_y, int lower_right_x, int lower_right_y);
typedef int(*TL_CAMERA_GET_ROI_RANGE)(void* tl_camera_handle, int* upper_left_x_min, int* upper_left_y_min, int* lower_right_x_min, int* lower_right_y_min, int* upper_left_x_max, int*  upper_left_y_max, int* lower_right_x_max, int* lower_right_y_max);
typedef int(*TL_CAMERA_GET_SERIAL_NUMBER)(void* tl_camera_handle, char* serial_number, int str_length);
typedef int(*TL_CAMERA_GET_SERIAL_NUMBER_STRING_LENGTH_RANGE)(void* tl_camera_handle, int* serial_number_min, int* serial_number_max);
typedef int(*TL_CAMERA_GET_STATUS_LED)(void* tl_camera_handle, int* status_led);
typedef int(*TL_CAMERA_SET_STATUS_LED)(void* tl_camera_handle, int status_led);
typedef int(*TL_CAMERA_GET_EEP_STATUS)(void* tl_camera_handle, enum EEP_STATUS* eep_status_enum);
typedef int(*TL_CAMERA_SET_EEP_ENABLED)(void* tl_camera_handle, int eep_enabled);
typedef int(*TL_CAMERA_GET_TEMPERATURE_DEGREES_C)(void* tl_camera_handle, int *temperature);
typedef int(*TL_CAMERA_GET_VBIN)(void* tl_camera_handle, int* vbin);
typedef int(*TL_CAMERA_SET_VBIN)(void* tl_camera_handle, int vbin);
typedef int(*TL_CAMERA_GET_VBIN_RANGE)(void* tl_camera_handle, int* vbin_min, int* vbin_max);
typedef int(*TL_CAMERA_OPEN_CAMERA)(char* camera_serial_number, void** tl_camera_handle);
typedef int(*TL_CAMERA_CLOSE_CAMERA)(void* tl_camera_handle);
typedef int(*TL_CAMERA_ARM)(void* tl_camera_handle, int number_of_frames_in_buffer);
typedef int(*TL_CAMERA_ISSUE_SOFTWARE_TRIGGER)(void* tl_camera_handle);
typedef int(*TL_CAMERA_DISARM)(void* tl_camera_handle);

#ifndef THORLABS_TSI_BUILD_DLL

#ifdef __cplusplus
extern "C"
{
#endif

extern TL_CAMERA_SET_FRAME_AVAILABLE_CALLBACK tl_camera_set_frame_available_callback;
extern TL_CAMERA_SET_CAMERA_CONNECT_CALLBACK tl_camera_set_camera_connect_callback;
extern TL_CAMERA_SET_CAMERA_DISCONNECT_CALLBACK tl_camera_set_camera_disconnect_callback;
extern TL_CAMERA_OPEN_SDK tl_camera_open_sdk;
extern TL_CAMERA_CLOSE_SDK tl_camera_close_sdk;
extern TL_CAMERA_GET_LAST_ERROR tl_camera_get_last_error;
extern TL_CAMERA_GET_AVAILABLE_CAMERAS tl_camera_get_available_cameras;
extern TL_CAMERA_INTERNAL_COMMAND tl_camera_internal_command;
extern TL_CAMERA_GET_EXPOSURE_US tl_camera_get_exposure_us;
extern TL_CAMERA_SET_EXPOSURE_US tl_camera_set_exposure_us;
extern TL_CAMERA_GET_EXPOSURE_RANGE_US tl_camera_get_exposure_range_us;
extern TL_CAMERA_GET_FIRMWARE_VERSION tl_camera_get_firmware_version;
extern TL_CAMERA_GET_FRAME_TIME_US tl_camera_get_frame_time_us;
extern TL_CAMERA_GET_MEASURED_FRAMES_PER_SECOND tl_camera_get_measured_frames_per_second;
extern TL_CAMERA_GET_HARDWARE_TRIGGER_MODE tl_camera_get_hardware_trigger_mode;
extern TL_CAMERA_SET_HARDWARE_TRIGGER_MODE tl_camera_set_hardware_trigger_mode;
extern TL_CAMERA_GET_HBIN tl_camera_get_hbin;
extern TL_CAMERA_SET_HBIN tl_camera_set_hbin;
extern TL_CAMERA_GET_HBIN_RANGE tl_camera_get_hbin_range;
extern TL_CAMERA_GET_HOT_PIXEL_CORRECTION tl_camera_get_hot_pixel_correction;
extern TL_CAMERA_SET_HOT_PIXEL_CORRECTION tl_camera_set_hot_pixel_correction;
extern TL_CAMERA_GET_HOT_PIXEL_CORRECTION_THRESHOLD tl_camera_get_hot_pixel_correction_threshold;
extern TL_CAMERA_SET_HOT_PIXEL_CORRECTION_THRESHOLD tl_camera_set_hot_pixel_correction_threshold;
extern TL_CAMERA_GET_HOT_PIXEL_CORRECTION_THRESHOLD_RANGE tl_camera_get_hot_pixel_correction_threshold_range;
extern TL_CAMERA_GET_IMAGE_WIDTH_PIXELS tl_camera_get_image_width_pixels;
extern TL_CAMERA_GET_IMAGE_WIDTH_RANGE_PIXELS tl_camera_get_image_width_range_pixels;
extern TL_CAMERA_GET_IMAGE_HEIGHT_PIXELS tl_camera_get_image_height_pixels;
extern TL_CAMERA_GET_IMAGE_HEIGHT_RANGE_PIXELS tl_camera_get_image_height_range_pixels;
extern TL_CAMERA_GET_MODEL tl_camera_get_model;
extern TL_CAMERA_GET_MODEL_STRING_LENGTH_RANGE tl_camera_get_model_string_length_range;
extern TL_CAMERA_GET_NAME tl_camera_get_name;
extern TL_CAMERA_SET_NAME tl_camera_set_name;
extern TL_CAMERA_GET_NAME_STRING_LENGTH_RANGE tl_camera_get_name_string_length_range;
extern TL_CAMERA_GET_NUMBER_OF_FRAMES_PER_TRIGGER tl_camera_get_number_of_frames_per_trigger;
extern TL_CAMERA_SET_NUMBER_OF_FRAMES_PER_TRIGGER tl_camera_set_number_of_frames_per_trigger;
extern TL_CAMERA_GET_NUMBER_OF_FRAMES_PER_TRIGGER_RANGE tl_camera_get_number_of_frames_per_trigger_range;
extern TL_CAMERA_GET_USB_BUS_SPEED tl_camera_get_usb_bus_speed;
extern TL_CAMERA_GET_DATA_RATE_SUPPORTED tl_camera_get_data_rate_supported;
extern TL_CAMERA_GET_DATA_RATE tl_camera_get_data_rate;
extern TL_CAMERA_SET_DATA_RATE tl_camera_set_data_rate;
extern TL_CAMERA_GET_PIXEL_SIZE_BYTES tl_camera_get_pixel_size_bytes;
extern TL_CAMERA_GET_PIXEL_WIDTH_UM tl_camera_get_pixel_width_um;
extern TL_CAMERA_GET_PIXEL_HEIGHT_UM tl_camera_get_pixel_height_um;
extern TL_CAMERA_GET_PIXEL_BIT_DEPTH tl_camera_get_pixel_bit_depth;
extern TL_CAMERA_GET_ROI tl_camera_get_roi;
extern TL_CAMERA_SET_ROI tl_camera_set_roi;
extern TL_CAMERA_GET_ROI_RANGE tl_camera_get_roi_range;
extern TL_CAMERA_GET_SERIAL_NUMBER tl_camera_get_serial_number;
extern TL_CAMERA_GET_SERIAL_NUMBER_STRING_LENGTH_RANGE tl_camera_get_serial_number_string_length_range;
extern TL_CAMERA_GET_STATUS_LED tl_camera_get_status_led;
extern TL_CAMERA_SET_STATUS_LED tl_camera_set_status_led;
extern TL_CAMERA_GET_EEP_STATUS tl_camera_get_eep_status;
extern TL_CAMERA_SET_EEP_ENABLED tl_camera_set_eep_enabled;
extern TL_CAMERA_GET_TEMPERATURE_DEGREES_C tl_camera_get_temperature_degrees_c;
extern TL_CAMERA_GET_VBIN tl_camera_get_vbin;
extern TL_CAMERA_SET_VBIN tl_camera_set_vbin;
extern TL_CAMERA_GET_VBIN_RANGE tl_camera_get_vbin_range;
extern TL_CAMERA_OPEN_CAMERA tl_camera_open_camera;
extern TL_CAMERA_CLOSE_CAMERA tl_camera_close_camera;
extern TL_CAMERA_ARM tl_camera_arm;
extern TL_CAMERA_ISSUE_SOFTWARE_TRIGGER tl_camera_issue_software_trigger;
extern TL_CAMERA_DISARM tl_camera_disarm;

int init_camera_sdk_dll();
int free_camera_sdk_dll();

#ifdef __cplusplus
}
#endif

#endif