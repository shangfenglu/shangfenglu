#ifndef _SURFACE_RECT_H_
#define _SURFACE_RECT_H_
#include<stdint.h>
struct Session {
    int64_t KeyframeCount;
    unsigned char *SessionId;
    int SessionType;
    int SessionProto;
    int64_t SessionStart;              //起始时间
    int64_t SessionStop;               //结束时间
    unsigned char *SessionVersion;
    int64_t SessionFileSize;
    unsigned char *ClientUser;
    unsigned char *ClientUname;
    unsigned char *Display;             //分辨率
    unsigned char *ServerUser;
    unsigned char *ServerName;
    unsigned char *ServerMac;
    unsigned char *ServerIp;
    int ServerPort;
    unsigned char *RuleName;
    unsigned char *Command;
};


struct surface_rect
{
    int x;
    int y;
    int width;
    int height;
};

struct pud_data
{
    struct surface_rect rect;
    int size;
    void *data;
};

#endif