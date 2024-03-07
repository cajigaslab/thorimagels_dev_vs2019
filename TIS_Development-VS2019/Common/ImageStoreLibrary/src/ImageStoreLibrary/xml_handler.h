#pragma once
#include "plate.h"
#include "scan.h"

long xml_set_plate_info(char* plate_info, map<uint16_t, Plate*>* plates);
long xml_add_scan_info(char* scan_info, map<uint16_t, Plate*> plates, Scan** scan);
long xml_get_plate_info(map<uint16_t, Plate*> plates, char** xml);
long xml_get_scan_infos(map<uint16_t, Plate*> plates, map<uint32_t, Scan*> scans,char** xml);
long xml_write_ome(map<uint16_t, Plate*> plates, map<uint32_t, Scan*> scans, char** xml, char* pre_file_name, bool is_create_pyramidal_data);
long xml_read_ome(char* xml, map<uint16_t, Plate*>* plates, map<uint32_t, Scan*>* scans, TiffData* parent_tiff, OpenMode openMode);

///**************************		XML FILE		**************************///

long ResolvePlatesXML(char* plates_xml, std::map<uint16_t, Plate*>* plates);
long ResolveScansXML(char* scans_xml, std::map<uint16_t, Plate*>* plates, std::map<uint32_t, Scan*>* scans);
long GeneratePlatesXML(int fileHandle, std::map<uint16_t, Plate*> plates);
long GenerateScanXML(int fileHandle, std::map<uint16_t, Plate*> plates, Scan* scan);