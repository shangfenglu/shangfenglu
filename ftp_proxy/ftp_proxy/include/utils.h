#ifndef  __UTILS_
#define __UTILS_
#include <sys/types.h>

#define MEMORY_SIZE 200
struct link;
struct pollfd;
struct socket_data;
struct packet_buf;
//获得本机ip地址
int get_local_ip(char* local_ip);

//初始化用户结构体
struct session* init_session();

//初始化连接结构体
int init_link(struct link* link);

//初始化packet_buf结构体
struct packet_buf* init_packet_buf();

//清理session结构体
int destory_session(struct session* session);

//初始化poll数组
int init_pollfds(struct pollfd* fds, int num);

//向poll数组里添加socket
int add_pollfds(struct pollfd* fds, int num, const int socket);

//向poll数组里删除socket
int del_pollfds(struct pollfd* fds, int num, const int socket);

//向子进程数组中添加pid
int add_child_pid(pid_t* child_fds, int num, const pid_t child);

//向子进程数组中删除pid
int del_child_pid(pid_t* child_fds, int num, const pid_t child);

//比较字符串
int compare_string(const char* dst, const char* src, int length);

//获得index的字符
char get_char(const char* text, int index);

//拷贝packet_buf
int copy_packet_buf(struct socket_data* socket_data, struct packet_buf* packet_buf);

//追加packet_buf
int append_packet_buf(struct socket_data* socket_data, struct packet_buf* packet_buf);

#endif //__UTILS_
