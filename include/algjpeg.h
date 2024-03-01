#ifndef JPEG_MEM_DST_
#define JPEG_MEM_DST_

#include <stdint.h>

//rgb2jpeg
int jpeg_encode(unsigned char **jpeg_data, unsigned char *src_data,int length,
            int width,int height,int  bpp,int depth,int quality);

//jpeg2rgb
int jpeg_decode(uint8_t** buffer, int* buffer_memory_size, uint8_t *jpeg_data, uint32_t length);

#endif