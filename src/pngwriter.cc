// File: pngwriter.cc
// Date: Sat May 11 23:39:49 2013 +0800
// Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#include <png.h>
#include <iostream>
#include <cstdlib>
#include "pngwriter.h"
using namespace std;

void write_image(const char* file_name, short* img, int SIZE_X, int SIZE_Y){
	FILE *fp = fopen(file_name, "wb");
	png_structp png_ptr;
	png_infop info_ptr;
	png_bytep row;

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	info_ptr = png_create_info_struct(png_ptr);
	setjmp(png_jmpbuf(png_ptr));
	png_init_io(png_ptr, fp);
	png_set_IHDR(png_ptr, info_ptr, SIZE_X, SIZE_Y,
			8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	png_write_info(png_ptr, info_ptr);
	row = (png_bytep) malloc(3 * SIZE_X * sizeof(png_byte));
	for (int j = 0; j < SIZE_Y; j ++){
		for (int i = 0; i < SIZE_X; i ++)
			row[3 * i] = row[3 * i + 1] = row[3 * i + 2] = img[i * SIZE_Y + j];
		png_write_row(png_ptr, row);
	}
	png_write_end(png_ptr, NULL);

	if (fp != NULL) fclose(fp);
	if (info_ptr != NULL) png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
	if (png_ptr != NULL) png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
	if (row != NULL) free(row);

	cout << "Image Saved to " << file_name << endl;
	return ;
}

