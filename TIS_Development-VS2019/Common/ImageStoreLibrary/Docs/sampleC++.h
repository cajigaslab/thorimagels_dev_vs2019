//create an image data file get the handle
int data_handle = open_file("E:\\SampleData\\4um_spheres_hdr.tif", READ_WRITE_MODE);

//set plate information
set_plate_info(data_handle, plate_info_xml, size_plateinfo_xml);

//add one scan 
add_scan_info(data_handle, scan_info_xml, size_scaninfo_xml);

//save data one by one when capturing
//...
//frame_info(scan_id = 1, region_id = 0, channel_id = 0, z_id = 1, time_id = 1, s_id = 1)
//the stride is the width of a single row of pixels (a scan line)
save_tile_data(data_handle, image_data, 5000, frame_info,1,1);

//generate the pyramidal data
generate_Pyramidal_data(data_handle, 1);

//save additional data
save_additional_data(data_handle, additional_data, additional_data_size, "metadata");

//close file
close_file(data_handle);


// open file and read
int data_handle = open_file("E:\\SampleData\\4um_spheres_hdr.tif", READONLY_MODE);

//get metadata data
int size_a = 0;
get_additional_data_size(data_handle, "metadata", &size_a);
char* ad_buffer = new char[size_a];
get_additional_data(data_handle, "metadata",ad_buffer);

//get region image with desitination width and height and roi
get_data(data_handle, frame_info, roi, 1024, 768, image_buffer)

close_file(data_handle);
