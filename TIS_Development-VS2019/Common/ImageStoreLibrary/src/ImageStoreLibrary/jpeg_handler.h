#pragma once
long jpeg_decompress(unsigned char* buf, unsigned char* encoded_buf, unsigned long encoded_size);
long jpeg_compress(unsigned char* buf, unsigned char** encoded_buf, unsigned long* encoded_size, unsigned long width, unsigned long height);
long jpeg_write_file(void* buf, char* file_name, unsigned long size);