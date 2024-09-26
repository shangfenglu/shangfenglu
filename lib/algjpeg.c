#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <setjmp.h>
#include <jerror.h>
#include <jpeglib.h>

#include "algjpeg.h"

#define OUTPUT_BUF_SIZE 4096

typedef struct{
    struct jpeg_destination_mgr pub;
    unsigned char **outbuffer;
    unsigned long *outsize;
    unsigned char *newbuffer;
    JOCTET *buffer;
    size_t bufsize;
}my_mem_destination_mgr;

typedef my_mem_destination_mgr *my_mem_dest_ptr;

static int rowbpp32tobpp24(char *src,int src_len,char *dst,int dst_len)
{
    int i;
    int index = 0;
    char r = 0;
    char g = 0;
    char b = 0;

    for(i=0; i < src_len; ){
        if(index +3 > dst_len) return -1;

        b = src[i++];
        g = src[i++];
        r = src[i++];

        dst[index++] = r;
        dst[index++] = g;
        dst[index++] = b;
        i++;
    }
    return index;
}

/* static int rowbpp16tobpp24(char * src,int src_len,char *dst,int dst_len,int depth)
{
    int i = 0;
    int index = 0;
    uint16_t bpp16 = 0;
    int r = 0;
    int g = 0;
    int b = 0;
    uint8_t first_byte = 0;
    uint8_t second_byte = 0;

    for(i=0; i< src_len;){
        first_byte = src[i];
        i++;
        second_byte = src[i];
        i++;

        bpp16 = (second_byte << 8| first_byte);
        if(depth == 16){
            r = (bpp16 & 0xf800) >> 11;
            g = (bpp16 & 0x07e0) >> 5;
            b = (bpp16 & 0x001f);

            dst[index++] = (r << 3);
            dst[index++] = (g << 2);
            dst[index++] = (b << 3);
        }else if(depth == 15){
            r = (bpp16 & 0xf800) >> 11;
            g = (bpp16 & 0x07c0) >> 5;
            b = (bpp16 & 0x003e);

            dst[index++] = (r << 3);
            dst[index++] = (g << 3);
            dst[index++] = (b << 3);
        }
    }
    return index;
} */

static int rowbpp16tobpp24(const char* src, int src_len, char* dst, int dst_len, int depth) {
    int i = 0;
    int index = 0;
    uint16_t bpp16 = 0;
    unsigned int r = 0;
    unsigned int g = 0;
    unsigned int b = 0;

    for (i = 0; i < src_len; i += 2) {
        uint8_t first_byte = src[i];
        uint8_t second_byte = src[i + 1];
        bpp16 = (second_byte << 8) | first_byte;

        if (depth == 16) {
            r = (bpp16 & 0xF800) >> 11;
            g = (bpp16 & 0x07E0) >> 5;
            b = bpp16 & 0x001F;
        } else if (depth == 15) {
            r = (bpp16 & 0x7C00) >> 10;
            g = (bpp16 & 0x03E0) >> 5;
            b = bpp16 & 0x001F;
        }

        dst[index++] = (r << 3);
        dst[index++] = (g << 3);
        dst[index++] = (b << 3);
    }

    return index;
}

static void init_mem_destionation(j_compress_ptr cinfo){
    /*no work necessary here*/
}

/* static boolean empty_mem_output_buffer(j_compress_ptr cinfo)
{
    size_t nextsize;
    JOCTET *nextbuffer;
    my_mem_dest_ptr dest = (my_mem_dest_ptr)cinfo->dest;

    nextsize = dest->bufsize * 2;
    nextbuffer = (JOCTET *)calloc(1,nextsize);

    if (nextbuffer == NULL){
        return FALSE;
    }

    memcpy(nextbuffer, dest->buffer, dest->buffer);

    if(dest->buffer != NULL)
        free(dest->buffer);

    dest->newbuffer = nextbuffer;

    dest->pub.next_output_byte = nextbuffer + dest->bufsize;

    dest->buffer = nextbuffer;
    dest->bufsize = nextsize;

    return TRUE;
} */

static boolean empty_mem_output_buffer(j_compress_ptr cinfo)
{
    size_t nextsize;
    my_mem_dest_ptr dest = (my_mem_dest_ptr)cinfo->dest;

    nextsize = dest->bufsize * 2;

    // 分配新的缓冲区
    JOCTET* newbuffer = (JOCTET*)calloc(nextsize, sizeof(JOCTET));
    if (newbuffer == NULL) {
        return FALSE;
    }

    // 释放旧的缓冲区
    if (dest->buffer != NULL) {
        free(dest->buffer);
    }

    // 更新缓冲区指针和大小
    dest->buffer = newbuffer;
    dest->bufsize = nextsize;
    dest->newbuffer = newbuffer;
    dest->pub.next_output_byte = newbuffer + dest->bufsize;

    return TRUE;
}

static void term_mem_destination(j_compress_ptr cinfo)
{
    my_mem_dest_ptr dest = (my_mem_dest_ptr)cinfo->dest;

    *dest->outbuffer = dest->buffer;
    *dest->outsize = dest->bufsize - dest->pub.free_in_buffer;
}

static int jpeg_memory_dest(j_compress_ptr cinfo, unsigned char** outbuffer, unsigned long* outsize) {
    my_mem_dest_ptr dest;

    if (outbuffer == NULL || outsize == NULL) {
        return -1;  // 检查输入参数是否为空
    }

    if (cinfo->dest == NULL) {
        cinfo->dest = (struct jpeg_destination_mgr*)(*cinfo->mem->alloc_small)((j_common_ptr)cinfo, JPOOL_PERMANENT, sizeof(my_mem_destination_mgr));
    }

    dest = (my_mem_dest_ptr)cinfo->dest;
    dest->pub.init_destination = init_mem_destionation;
    dest->pub.empty_output_buffer = empty_mem_output_buffer;  
    dest->pub.term_destination = term_mem_destination;
    dest->outbuffer = outbuffer;
    dest->outsize = outsize;
    dest->newbuffer = NULL;

    if (*outbuffer == NULL || *outsize == 0) {
        dest->newbuffer = *outbuffer = (unsigned char*)malloc(OUTPUT_BUF_SIZE);
        if (dest->newbuffer == NULL) {
            return -1;  // 内存分配失败的错误处理
        }
        *outsize = OUTPUT_BUF_SIZE;
    }

    dest->pub.next_output_byte = dest->buffer = *outbuffer;
    dest->pub.free_in_buffer = dest->bufsize = *outsize;
    return 0;
}

static int jpeg_init(struct jpeg_compress_struct* jcs, int quality, int width, int height, unsigned char** jpeg_buffer, unsigned long* jpeg_len) {
    struct jpeg_error_mgr jem;

    // 检查输入参数的有效性
    if (jcs == NULL || jpeg_buffer == NULL || jpeg_len == NULL) {
        printf("Invalid input parameters.\n");
        return -1;
    }

    jcs->err = jpeg_std_error(&jem);
    jpeg_create_compress(jcs);

    if (jpeg_memory_dest(jcs, jpeg_buffer, jpeg_len) < 0) {
        // 错误处理和错误消息输出
        fprintf(stderr, "Failed to set memory destination for JPEG compression.\n");
        jpeg_destroy_compress(jcs);
        return -1;
    }

    jcs->image_width = width;
    jcs->image_height = height;
    jcs->input_components = 3;
    jcs->in_color_space = JCS_RGB;

    jpeg_set_defaults(jcs);
    jpeg_set_quality(jcs, quality, TRUE);
    jpeg_start_compress(jcs, TRUE);
    return 0;
}

static void jpeg_end(struct jpeg_commpress_struct *jcs)
{
    jpeg_finish_compress(jcs);
    jpeg_destroy_compress(jcs);
}

int jpeg_encode(unsigned char **jpeg_data,unsigned char *src_data,int length,
        int width,int height,int bpp,int depth,int quality)
{
    struct jpeg_compress_struct jcs;
    JSAMPROW row_pointer[1];
    int j=0;
    int rect_row_stribe = 0;
    int bpp24_row_stribe = 0;
    char *bpp24rowbuf = NULL;
    char *p = NULL;
    unsigned long jpeg_len = 0;

    if(width == 0|| height == 0) {
        printf("encode error: width == 0 or height == 0\n");
        return -1;
    }

    if((bpp !=4 && bpp !=2 && bpp!=3) && (depth !=24 && depth !=16 && depth !=15)){
        printf("bpp = %d or depth ==  %d does not support\n",bpp,depth);
        return -1;
    }

    rect_row_stribe = width * bpp;
    bpp24_row_stribe = width * 3;
    bpp24rowbuf = calloc(1,bpp24_row_stribe);

    if(jpeg_init(&jcs, quality, width, height, jpeg_data, & jpeg_len) < 0){
        printf("jpeg init error");
        return -1;
    }

    for(j==0;j< length; j+=rect_row_stribe){
        p = (char *)(src_data +j);
        memset(bpp24rowbuf,0,bpp24_row_stribe);
        if(bpp == 4){
            rowbpp32tobpp24(p, rect_row_stribe,bpp24rowbuf, bpp24_row_stribe);
            row_pointer[0] = (JSAMPROW)bpp24_row_stribe;
        }else if(bpp ==3){
            row_pointer[0] = (JSAMPROW)p;
        }else if(bpp ==2){
            rowbpp16tobpp24(p, rect_row_stribe,bpp24rowbuf, bpp24_row_stribe,depth);
            row_pointer[0] = (JSAMPROW)bpp24_row_stribe;
        }else if(bpp == 1){
            row_pointer[0] = (JSAMPROW)bpp24rowbuf;
        }
        jpeg_write_scanlines(&jcs, row_pointer,1);
    }

   jpeg_end(&jcs);
   free(bpp24rowbuf);
   return jpeg_len;
}

static jmp_buf jump_buffer;

int jpeg_decode(uint8_t** buffer, int* buffer_memory_size, uint8_t* jpeg_data, uint32_t length) {
    int width = 0;
    int height = 0;
    int numComponents = 0;
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    JSAMPROW row_pointer[1];
    size_t rgbLength = 0;

    // 初始化JPEG解码器
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    // 设置输入数据源
    jpeg_mem_src(&cinfo, jpeg_data, length);

    // 读取JPEG文件头
    if (jpeg_read_header(&cinfo, TRUE) != JPEG_HEADER_OK) {
        jpeg_destroy_decompress(&cinfo);
        return -1;
    }

    // 开始解码
    jpeg_start_decompress(&cinfo);

    // 获取图像的宽度和高度
    width = cinfo.output_width;
    height = cinfo.output_height;
    numComponents = cinfo.output_components;


    // 计算RGB数据的长度
    rgbLength = width * height * numComponents;

    // 看是否需要扩充内存
    if (*buffer_memory_size < rgbLength) {
        if (*buffer != NULL) {
            free(*buffer);
            *buffer = NULL;
        }
        *buffer = (uint8_t*)malloc(rgbLength);
        if (*buffer == NULL) {
            jpeg_destroy_decompress(&cinfo);
            return -1;
        }
        *buffer_memory_size = rgbLength;
    }

    // 逐行读取RGB数据
    while (cinfo.output_scanline < height) {
        row_pointer[0] = &((*buffer)[cinfo.output_scanline * width * numComponents]);
        jpeg_read_scanlines(&cinfo, row_pointer, 1);
    }
    
    // 完成解码
    jpeg_finish_decompress(&cinfo);

    // 销毁JPEG解码器
    jpeg_destroy_decompress(&cinfo);

    return rgbLength;
}

