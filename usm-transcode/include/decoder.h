#ifndef __DECODER_
#define __DECODER_

#include <sqlite3.h>
#include <stdint.h>
#include <stdio.h>

#define REFRESH_FRAME 1025

struct bitmap;
//V表信息结构体
struct session_data{
    int width;
    int height;
    int bits;
};

//D表数据结构体
struct pdu_data{
    int pdu_id;
    int64_t pdu_time;
    int pdu_type;
    int pdu_length;
    int memory_length;
    void* data;
};

//更新区域结构体
struct rect{
    unsigned short x;
    unsigned short y;
    unsigned short width;
    unsigned short height;
};

struct rgb_data{
    struct rect rect;                    // 更新区域
    uint64_t data_length;                // 数据长度
    uint64_t jpeg_length;                // jpeg长度
    int memory_length;                   // 内存长度
    uint8_t*    data;                    // 数据
};

//分片文件结构体
struct share_file{
    char* folder;                // 文件目录
    FILE* fp;                    // 文件句柄
    int   chunk;                 // 分片数
};

//解码器结构体
struct decoder{
    sqlite3_stmt* stmt;                  // 记录游标对象
    sqlite3_stmt* keyframe_stmt;         // 关键帧游标对象
    struct session_data* session_data;   // 会话数据
    struct pdu_data* pdu_data;           // pdu数据
    struct rgb_data* rgb_data;           // rgb数据
    struct share_file* share_file;       // 分片文件
};

//获取分片文件结构体
struct share_file* get_share_file(const char* filename);

//获取V表信息结构体
struct session_data* get_session_data();

//销毁V表信息结构体
int free_session_data(struct session_data* session_data);

//获取D表数据结构体
struct pdu_data* get_pdu_data();

//销毁D表数据结构体
int free_pdu_data(struct pdu_data* pdu_data);

//获取jpg数据结构体
struct rgb_data* get_rgb_data();

//销毁jpg数据结构体
int free_rgb_data(struct rgb_data* rgb_data);

//打开解码器
struct decoder* decoder_open();

//关闭解码器
int decoder_close(struct decoder* decoder);

#endif //__DECODER_