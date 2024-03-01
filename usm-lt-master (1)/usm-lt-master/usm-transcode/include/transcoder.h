#ifndef __TRANCODE_H__
#define __TRANCODE_H__

#include "sqlite3.h"
#include "bitmap.h"
#include "decoder.h"
#include "encoder.h"
#include "config.h"
#include "stream.h"

//参数配置结构体
struct garg {
    char *input;                 // 输入文件路径
    char *output;                // 输出文件路径
    int   file_type;             // 文件类型
    int   offset;                // 读取偏移                
} G;

//转码器结构体
struct transcode{
    sqlite3 *db;                  // 数据库句柄
    struct bitmap* bitmap;        // 位图结构体
    struct decoder* decoder;      // 解码器结构体
    struct encoder* encoder;      // 编码器结构体
    int             offset;       // 偏移量
};

//打开转码器
struct transcode* transcoder_open(struct garg* garg,struct xconfig* config);

//开始转码
int transcoder_transcode(struct transcode* transcode);

//关闭转码器
int transcoder_close(struct transcode* transcode);

//将pdu_data数据解码到rgba_data中
static int pdu2rgba(STREAM* pdu_data,struct rgb_data* rgb_data);

//将rgb_data数据更新到bitmap当中
static int rgba2bitmap(STREAM* bitmap_data,struct rgb_data* rgb_data,int width);

#endif //__TRANCODE_H__
