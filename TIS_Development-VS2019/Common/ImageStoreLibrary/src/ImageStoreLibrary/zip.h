#pragma once

long zip_get_file_size(const char* file, const char* name);
long zip_read_file(const char* file, const char* name, void* buffer, uint64_t size);
long zip_write_file(const char* file, const char* name, void* buffer, uint64_t size);
long zip_delete_file(const char* file, const char* name);

