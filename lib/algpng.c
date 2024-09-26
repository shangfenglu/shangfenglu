#include <stdlib.h>
#include <png.h>
#include "algpng.h"

struct img_buffer
{
	char *data;
	int size;
	int off;
};

static int buffer_write(struct img_buffer *buff, char *data, int length)
{
	char *tmp = NULL;
	int size = 0;

	if(buff->size - buff->off < length){
		size = buff->size * 2 + length;
		tmp = calloc(1, size);
		memcpy(tmp, buff->data, buff->off);
		free(buff->data);
		buff->data = tmp;
		buff->size = size;
	}

	memcpy(buff->data + buff->off, data, length);
	buff->off += length;
	return 0;
}
#define HAVE_PNG_GET_IO_PTR
static void png_write_handler(png_structp png, png_bytep data, png_size_t length) {
	/* Get png buffer structure */
	struct img_buffer *buff = NULL;

#ifdef HAVE_PNG_GET_IO_PTR
	buff = (struct img_buffer*) png_get_io_ptr(png);
#else
	buff = (struct img_buffer*) png->io_ptr;
#endif

	/* Append data to buffer, writing as necessary */
	buffer_write(buff, (char*)data, length);
}

static void png_flush_handler(png_structp png) {

	/* Get png buffer structure */
	struct img_buffer *buff = NULL;

#ifdef HAVE_PNG_GET_IO_PTR
	buff = (struct img_buffer*) png_get_io_ptr(png);
#else
	buff = (struct img_buffer*) png->io_ptr;
#endif

	/* Flush buffer */
}

int png_encode(char **out, char *img, int size, int width, int height, int bpp)
{
	int i = 0;
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	struct img_buffer buff = {0};
	
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png_ptr){
		return -1;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if(!info_ptr){
		 png_destroy_write_struct(&png_ptr, NULL);
		return -1;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		 png_destroy_write_struct(&png_ptr, &info_ptr);
		return -1;
	}

	buff.size = 1024;
	buff.data = calloc(1, buff.size);

	png_set_write_fn(png_ptr, &buff,
			png_write_handler,
			png_flush_handler);

	if(bpp == 4) {
		png_set_IHDR(
				png_ptr,
				info_ptr,
				width,
				height,
				8,
				PNG_COLOR_TYPE_RGBA,
				PNG_INTERLACE_NONE,
				PNG_COMPRESSION_TYPE_DEFAULT,
				PNG_FILTER_TYPE_DEFAULT
			    );
	}else if(bpp == 3){
		png_set_IHDR(
				png_ptr,
				info_ptr,
				width,
				height,
				8,
				PNG_COLOR_TYPE_RGB,
				PNG_INTERLACE_NONE,
				PNG_COMPRESSION_TYPE_DEFAULT,
				PNG_FILTER_TYPE_DEFAULT
			    );
	}else{
		free(buff.data);
		return -1;
	}

	png_write_info(png_ptr, info_ptr);

	for(i = 0; i < height; i++){
		png_bytep r = (png_bytep)img + i * width * bpp;
		png_write_row(png_ptr, r);
	}

	/* Finish write */
	png_destroy_write_struct(&png_ptr, &info_ptr);

	*out = buff.data;

	return buff.off;
}
