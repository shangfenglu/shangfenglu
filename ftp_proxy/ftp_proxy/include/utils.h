#ifndef  __UTILS_
#define __UTILS_

#include "struct.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <poll.h>

#define MEMORY_SIZE 200

//获得本机ip地址
int get_local_ip(char* local_ip);

//初始化用户结构体
struct session* init_session();

//初始化连接结构体
int init_link(struct link* link);

//初始化packet_buf结构体
struct packet_buf* init_packet_buf();

//初始化poll数组
int init_pollfds(struct pollfd* fds, int num);

//向poll数组里添加socket
int add_pollfds(struct pollfd* fds, int num, const int socket);

//向poll数组里删除socket
int del_pollfds(struct pollfd* fds, int num, const int socket);

//追加memory空间
void* append_memory(void* addr, unsigned int size, unsigned int append_length);

//比较字符串
int compare_string(const char* dst, const char* src, int length);

//检查是否完整
int is_complete(char* src);

//将字符串提升为大写
int string_upper(char* str, int length);

#endif  //__UTILS_