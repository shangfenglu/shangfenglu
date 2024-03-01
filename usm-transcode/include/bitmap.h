#ifndef __BITMAP_
#define __BITMAP_

#include <stdint.h>

//记录分辨率结构体
struct resolution {
    int width;                // 记录屏幕宽度
    int height;               // 记录屏幕高度
    int bit_per_pixel;        // 记录像素位数
};


//位图数据
struct bitmap {
    struct resolution* resolution;
    int64_t      time;             // 时间戳
	uint8_t* buffer;           // 内容
	uint64_t bufflen;          // 内容长度
};

//初始化位图
struct bitmap* bitmap_init(uint64_t bufflen);

//销毁位图
int bitmap_free(struct bitmap* bitmap);

#endif