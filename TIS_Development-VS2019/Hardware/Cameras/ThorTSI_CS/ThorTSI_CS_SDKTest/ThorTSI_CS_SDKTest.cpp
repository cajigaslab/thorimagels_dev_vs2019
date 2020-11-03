#include <iostream>
#include <string>
#include "windows.h"
#include "../ThorTSI_CS/inc/thorlabs_tsi_camera_sdk.h"

using namespace std;

namespace
{
	HANDLE frameAvailableEvent = CreateEvent(NULL, true, false, NULL);
	int numFrame = 1;

	void print_error(const char* msg)
	{
		cout << msg << " " << tl_camera_get_last_error() << endl;
	}

	void FrameAvailableCallback(void* sender, unsigned short* image_buffer, int image_width, int image_height, int bit_depth, int number_of_color_channels, int frame_count, void* context)
	{
		unsigned long frameSizeInBytes = image_width * image_height * (bit_depth / 8) * number_of_color_channels;
		void* tmpMem = malloc(frameSizeInBytes);
		memcpy_s(tmpMem, frameSizeInBytes, image_buffer, frameSizeInBytes);
		free(tmpMem);
		cout << "Copy frame # " << frame_count << endl;
		if(numFrame <= frame_count)
				SetEvent(frameAvailableEvent);
	}

	void camera_connect_callback(char* cameraSerialNumber, enum USB_BUS_SPEED usb_bus_speed, void* context)
	{
		cout << "camera " << cameraSerialNumber << " connected with bus speed = " << usb_bus_speed << endl;
	}

	void camera_disconnect_callback(char* cameraSerialNumber, void* context)
	{
		cout << "camera " << cameraSerialNumber << " disconnected" << endl;
	}

	void test_sdk(void* camera_handle)
	{
		if (!camera_handle) return;

		if (tl_camera_disarm(camera_handle)) print_error("Failed to stop the camera");
		else cout << "Successfully stopped the camera" << endl;

		// Test getting the current exposure.
		int exposure = 0;
		if (tl_camera_get_exposure_us(camera_handle, &exposure)) print_error("Failed to get exposure");
		else cout << "Exposure = " << exposure << endl;

		// Test setting a new exposure.
		exposure = 100000;
		if (tl_camera_set_exposure_us(camera_handle, exposure)) print_error("Failed to set exposure");

		// Read it back to make sure it succeeded.
		int exposure_confirmation = 0;
		if (tl_camera_get_exposure_us(camera_handle, &exposure_confirmation)) print_error("Failed to get exposure");

		else cout << "Exposure confirmation = " << exposure_confirmation << endl;

		// Test getting the exposure range.
		int exp_min = 0, exp_max = 0;
		if (tl_camera_get_exposure_range_us(camera_handle, &exp_min, &exp_max)) print_error("Failed to get exposure range");
		else cout << "exp min = " << exp_min << ", exp_max = " << exp_max << endl;

		// Test getting the firmware version.
		char firmware_version[1024];
		if (tl_camera_get_firmware_version(camera_handle, firmware_version, 1024)) print_error("Failed to get the firmware version");
		else cout << "firmware version = " << firmware_version << endl;

		// Test getting the fps.
		double fps_current = 0.0;
		if (tl_camera_get_measured_frames_per_second(camera_handle, &fps_current)) print_error("Failed to get the current frames per second");
		else cout << "fps = " << fps_current << endl; // Snhould be 0 since we are not acquiring images.

		// Test getting the hardware trigger mode.
		enum TRIGGER_TYPE current_mode;
		enum TRIGGER_POLARITY current_polarity;
		if (tl_camera_get_hardware_trigger_mode(camera_handle, &current_mode, &current_polarity)) print_error("Failed to get the hardware trigger mode");
		else cout << "current mode = " << current_mode << ", current polarity = " << current_polarity << endl;

		// Test setting the hardware trigger mode.
		enum TRIGGER_TYPE trigger_mode = STANDARD;
		enum TRIGGER_POLARITY trigger_polarity = ACTIVE_LOW;
		if (tl_camera_set_hardware_trigger_mode(camera_handle, trigger_mode, trigger_polarity)) print_error("Failed to set the hardware trigger mode");

		// Read the trigger mode back to make sure it took.
		if (tl_camera_get_hardware_trigger_mode(camera_handle, &current_mode, &current_polarity)) print_error("Failed to get the hardware trigger mode");
		else cout << "current mode = " << current_mode << ", current polarity = " << current_polarity << endl;

		// Test getting the hbin and vbin
		int current_hbin = 0;
		int current_vbin = 0;
		if (tl_camera_get_hbin(camera_handle, &current_hbin)) print_error("Failed to get the hbin");
		if (tl_camera_get_vbin(camera_handle, &current_vbin)) print_error("Failed to get the vgin");
		cout << "hbin = " << current_hbin << ", current_vbin = " << current_vbin << endl;

		// Test setting the hbin and vbin
		int hbin = 2, vbin = 2;
		if (tl_camera_set_hbin(camera_handle, hbin)) print_error("Failed to set hbin");
		if (tl_camera_set_vbin(camera_handle, vbin)) print_error("Failed to set vbin");

		// Read it back to make sure it succeeded.
		if (tl_camera_get_hbin(camera_handle, &current_hbin)) print_error("Failed to get the hbin");
		if (tl_camera_get_vbin(camera_handle, &current_vbin)) print_error("Failed to get the vgin");
		cout << "hbin = " << current_hbin << ", current_vbin = " << current_vbin << endl;

		// Test getting the hbin and vbin range.
		int hbin_min = 0, hbin_max = 0, vbin_min = 0, vbin_max = 0;
		if (tl_camera_get_hbin_range(camera_handle, &hbin_min, &hbin_max)) print_error("Failed to get the hbin range");
		if (tl_camera_get_vbin_range(camera_handle, &vbin_min, &vbin_max)) print_error("Failed to get the vbin range");
		cout << "hbin min = " << hbin_min << ", hbin max = " << hbin_max << ", vbin min = " << vbin_min << ", vbin max = " << vbin_max << endl;

		// Test hot pixel correction.
		int hot_pixel = 0;
		if (tl_camera_get_hot_pixel_correction(camera_handle, &hot_pixel)) print_error("Failed to get hot pixel correction");
		else cout << "hot pixel = " << hot_pixel << endl;
		int set_hot_pixel = 1;
		if (tl_camera_set_hot_pixel_correction(camera_handle, set_hot_pixel)) print_error("Failed to set hot pixel correction");
		if (tl_camera_get_hot_pixel_correction(camera_handle, &hot_pixel)) print_error("Failed to get hot pixel correction");
		else cout << "hot pixel = " << hot_pixel << endl;

		// Test hot pixel correction threshhold.
		int hot_pixel_threshold = 0;
		if (tl_camera_get_hot_pixel_correction_threshold(camera_handle, &hot_pixel_threshold)) print_error("Failed to get hot pixel correction threshhold");
		else cout << "hot pixel correction threshhold = " << hot_pixel_threshold << endl;
		int set_hot_pixel_threshhold = 2000;
		if (tl_camera_set_hot_pixel_correction_threshold(camera_handle, set_hot_pixel_threshhold)) print_error("Failed to set hot pixel correction threshhold");
		if (tl_camera_get_hot_pixel_correction_threshold(camera_handle, &hot_pixel_threshold)) print_error("Failed to get hot pixel correction threshhold");
		else cout << "hot pixel correction threshhold = " << hot_pixel_threshold << endl;
		int hot_pixel_threshold_min = 0, hot_pixel_threshold_max = 0;
		if (tl_camera_get_hot_pixel_correction_threshold_range(camera_handle, &hot_pixel_threshold_min, &hot_pixel_threshold_max)) print_error("Failed to get hot pixel correction threshold range");
		else cout << "hot pixel threshold min = " << hot_pixel_threshold_min << ", hot pixel threshold max = " << hot_pixel_threshold_max << endl;

		// Test image width and height.
		int image_width = 0, image_height = 0;
		if (tl_camera_get_image_width_pixels(camera_handle, &image_width)) print_error("Failed to get image width");
		else cout << "image width = " << image_width << endl;
		if (tl_camera_get_image_height_pixels(camera_handle, &image_height)) print_error("Failed to get image height");
		else cout << "image height = " << image_height << endl;
		int image_width_min = 0, image_width_max = 0, image_height_min = 0, image_height_max = 0;
		if (tl_camera_get_image_width_range_pixels(camera_handle, &image_width_min, &image_width_max)) print_error("Failed to get image width range");
		else cout << "image width min = " << image_width_min << ", image width max = " << image_width_max << endl;
		if (tl_camera_get_image_height_range_pixels(camera_handle, &image_height_min, &image_height_max)) print_error("Failed to get image height range");
		else cout << "image height min = " << image_height_min << ", image height max = " << image_height_max << endl;

		// Test model.
		char model[1024];
		if (tl_camera_get_model(camera_handle, model, 1024)) print_error("Failed to get model number");
		else cout << "model = " << model << endl;
		int model_min = 0, model_max = 0;
		if (tl_camera_get_model_string_length_range(camera_handle, &model_min, &model_max)) print_error("Failed to get model name length range");
		else cout << "model min = " << model_min << ", model_max = " << model_max << endl;

		// Test name.
		char name[1024];
		if (tl_camera_get_name(camera_handle, name, 1024)) print_error("Failed to get camera name");
		else cout << "camera name = " << name << endl;
		int name_min = 0, name_max = 0;
		if (tl_camera_get_name_string_length_range(camera_handle, &name_min, &name_max)) print_error("Failed to get name name length range");
		else cout << "name min = " << name_min << ", name max = " << name_max << endl;
		char new_name[1024];
		strcpy_s(new_name, "show620a");
		if (tl_camera_set_name(camera_handle, new_name)) print_error("Failed to set camera name");
		if (tl_camera_get_name(camera_handle, name, 1024)) print_error("Failed to get camera name");
		else cout << "camera name = " << name << endl;
		char short_name[5];
		if (tl_camera_get_name(camera_handle, short_name, 5)) print_error("Failed to get camera name");
		else cout << "camera name = " << short_name << endl;

		// Test frames per trigger.
		long frames_per_trigger = 0;
		if (tl_camera_get_number_of_frames_per_trigger(camera_handle, &frames_per_trigger)) print_error("Failed to get frames per trigger");
		else cout << "Frames per trigger = " << frames_per_trigger << endl;
		int set_frames_per_trigger = 1;
		if (tl_camera_set_number_of_frames_per_trigger(camera_handle, set_frames_per_trigger)) print_error("Failed to set the frames per trigger");
		if (tl_camera_get_number_of_frames_per_trigger(camera_handle, &frames_per_trigger)) print_error("Failed to get frames per trigger");
		else cout << "Frames per trigger = " << frames_per_trigger << endl;
		long fpt_min = 0, fpt_max = 0;
		if (tl_camera_get_number_of_frames_per_trigger_range(camera_handle, &fpt_min, &fpt_max)) print_error("Failed to get frames per trigger range");
		else cout << "fpt min = " << fpt_min << ", fpt_max = " << fpt_max << endl;

		// Test fps.
		enum DATA_RATE fps = RESERVED1;
		if (tl_camera_get_data_rate(camera_handle, &fps)) print_error("Failed to get FPS");
		else cout << "fps = " << fps << endl;
		enum DATA_RATE set_fps = FPS_50;
		if (tl_camera_set_data_rate(camera_handle, set_fps)) print_error("Failed to set FPS");
		if (tl_camera_get_data_rate(camera_handle, &fps)) print_error("Failed to get FPS");
		else cout << "fps = " << fps << endl;

		// Test pixel size.
		int pixel_size_bytes = 0;
		if (tl_camera_get_pixel_size_bytes(camera_handle, &pixel_size_bytes)) print_error("Failed to get pixel size bytes");
		else cout << "pixel size = " << pixel_size_bytes << " bytes" << endl;

		// Test ROI.
		int x1_min = 0, x1_max = 0, y1_min = 0, y1_max = 0, x2_min = 0, x2_max = 0, y2_min = 0, y2_max = 0;
		if (tl_camera_get_roi_range(camera_handle, &x1_min, &y1_min, &x2_min, &y2_min, &x1_max, &y1_max, &x2_max, &y2_max)) print_error("Failed to get ROI range");
		else cout << "x1_min = " << x1_min << ", x1_max = " << x1_max << ", y1_min = " << y1_min << ", y1_max = " << y1_max << ", x2_min = " << x2_min << ", x2_max = " << x2_max << ", y2_min = " << y2_min << ", y2_max = " << y2_max << endl;
		int x1 = 0, y1 = 0, x2 = 0, y2 = 0;
		if (tl_camera_get_roi(camera_handle, &x1, &y1, &x2, &y2)) print_error("Failed to get ROI");
		else cout << "roi_x_ul = " << x1 << ", roi_y_ul = " << y1 << ", roi_x_br = " << x2 << ", roi_y_br = " << y2 << endl;
		int s_x1 = 200, s_y1 = 200, s_x2 = 399, s_y2 = 399;
		if (tl_camera_set_roi(camera_handle, s_x1, s_y1, s_x2, s_y2)) print_error("Failed to set ROI");
		if (tl_camera_get_roi(camera_handle, &x1, &y1, &x2, &y2)) print_error("Failed to get ROI");
		else cout << "roi_x_ul = " << x1 << ", roi_y_ul = " << y1 << ", roi_x_br = " << x2 << ", roi_y_br = " << y2 << endl;

		// Test serial number.
		char sn[1024];
		if (tl_camera_get_serial_number(camera_handle, sn, 1024)) print_error("Failed to get the serial number");
		else cout << "serial number = " << sn << endl;
		int sn_min = 0, sn_max = 0;
		if (tl_camera_get_serial_number_string_length_range(camera_handle, &sn_min, &sn_max)) print_error("Failed to get serial number range");
		else cout << "sn min = " << sn_min << ", sn max = " << sn_max << endl;

		// Test status LED.
		int status_led = 0;
		if (tl_camera_get_status_led(camera_handle, &status_led)) print_error("Failed to get status LED");
		else cout << "status LED = " << status_led << endl;
		int set_status_led = 1;	//1 blue (usb3), 0 off
		if (tl_camera_set_status_led(camera_handle, set_status_led)) print_error("Failed to set status LED");
		if (tl_camera_get_status_led(camera_handle, &status_led)) print_error("Failed to get status LED");
		else cout << "status LED = " << status_led << endl;

		// Test EEP status.
		enum EEP_STATUS eep_status = OFF;
		if (tl_camera_get_eep_status(camera_handle, &eep_status)) print_error("Failed to get eep status");
		else cout << "EEP status = " << eep_status << endl;
		if (tl_camera_set_eep_enabled(camera_handle, true)) print_error("Failed to set eep enabled");
		enum EEP_STATUS new_eep_status = OFF;
		if (tl_camera_get_eep_status(camera_handle, &eep_status)) print_error("Failed to get eep status");
		else cout << "EEP status = " << eep_status << endl;

		// Test temperature.
		int temp = 0;
		if (tl_camera_get_temperature_degrees_c(camera_handle, &temp)) print_error("Failed to get the sensor temperature");
		else cout << "sensor temp = " << temp << " degrees C." << endl;

		// Test get pixel bit depth.
		int pixel_bit_depth = 0;
		if (tl_camera_get_pixel_bit_depth(camera_handle, &pixel_bit_depth)) print_error("Failed to get pixel bit depth");
		else cout << "pixel bit depth = " << pixel_bit_depth << endl;

		// Test acquire
		numFrame = 1;
		long numFrmInBuffer = 2;

		tl_camera_set_hardware_trigger_mode(camera_handle, TRIGGER_TYPE::NONE, TRIGGER_POLARITY::ACTIVE_HIGH);
		tl_camera_set_number_of_frames_per_trigger(camera_handle, numFrame);
		tl_camera_set_frame_available_callback(camera_handle, FrameAvailableCallback, NULL);

		ResetEvent(frameAvailableEvent);
		tl_camera_arm(camera_handle, numFrmInBuffer);
		tl_camera_issue_software_trigger(camera_handle);
		if(WAIT_OBJECT_0 == WaitForSingleObject(frameAvailableEvent, 5000))
		{
			cout << "frames acquired." << endl;
		}
		tl_camera_disarm(camera_handle);
	}
} // anonymous namespace


int main(void)
{
	if (init_camera_sdk_dll()) {
		cout << "Failed to initialize SDK" << endl;
		return 0;
	}

	if (tl_camera_open_sdk()) {
		cout << "Failed to open SDK" << endl;
		return 0;
	}

	char camera_ids[1024];

	if (tl_camera_set_camera_connect_callback(camera_connect_callback, nullptr))
	{
		cout << "Failed to set camera connect callback" << endl;
		return 0;
	}

	if (tl_camera_set_camera_disconnect_callback(camera_disconnect_callback, nullptr))
	{
		cout << "Failed to set camera disconnect callback" << endl;
		return 0;
	}

	if (tl_camera_get_available_cameras(camera_ids, 1024))
	{
		cout << "Failed to get available cameras" << endl;
		return 0;
	}
	else cout << camera_ids << endl;

	string s_camera_ids(camera_ids);
	string s_camera_id = s_camera_ids.substr(0, s_camera_ids.find(' '));

	char camera_id[128];
	strcpy_s(camera_id, s_camera_id.c_str());
	void* camera_handle = nullptr;

	if (tl_camera_open_camera(camera_id, &camera_handle))
	{
		cout << "Failed to open camera" << endl;
		return 0;
	}

	cout << "camera handle = " << reinterpret_cast <unsigned long long> (camera_handle) << endl;

	test_sdk(camera_handle);

	cout << "about to close camera" << endl;

	if (tl_camera_close_camera(camera_handle))
	{
		cout << "Failed to close camera" << endl;
		return 0;
	}
	camera_handle = nullptr;

	cout << "about to close sdk" << endl;

	if (tl_camera_close_sdk())
	{
		cout << "Failed to close SDK" << endl;
		return 0;
	}

	if (free_camera_sdk_dll())
	{
		cout << "Failed to destroy SDK" << endl;
		return 0;
	}

	return 0;
}

