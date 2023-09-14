#ifndef __CONNECT_
#define __CONNECT_
struct link;

//初始化socket属性
static int init_socket(int socket);

//初始化监听socket
int init_socket_listen(int port, int init_type);

//等待连接socket
int accept_socket(int socket);

//连接到服务端
int connect_to_server(struct link* link);

#endif //__CONNECT_