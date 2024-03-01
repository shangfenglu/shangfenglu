#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "ini.h"
struct xconfig
{
    int ffmpeg_threads; // ffmpeg 线程数
    int fps;            // 指定帧率
};


void g_rgbtrans_init(struct ini_t* cfg_ini, struct xconfig* cfg);

#endif // __XCONFIG_H__