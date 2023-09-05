#ifndef __NET_TOOLS_
#define __NET_TOOLS_

#include "struct.h"
#include <string.h>


//初始化socket属性
int init_socket(int socket);

//初始化监听socket
int init_socket_listen(int port, int init_type);

//等待连接socket
int accept_socket(int socket);

//连接到服务端
int connect_to_server(struct link* link);

//发送数据
int nSend(int fd, char* buf, int buf_size, int* sendLength);

//接收数据
int nRecv(int fd, char* buf, int buf_size, int* recvLength);

#endif //__NET_TOOLS_
