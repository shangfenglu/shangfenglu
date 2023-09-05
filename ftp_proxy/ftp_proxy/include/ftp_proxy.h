#ifndef __FTP_SERVICE_
#define __FTP_SERVICE_

#include "enum.h"
#include "struct.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/time.h>
#include <netinet/tcp.h>

#define FTP_WELOCME_STRING "220 Welcome to blah FTP service.\r\n"
#define FILE_PATH "user_info.txt"

//初始化全局变量
int init_global_state(char* argv[]);

//开启ftp_service服务
int start_proxy_service();

//用户会话
int start_user_session(struct session* session);

//用户登录处理
int user_login(struct session* session);

//处理客户端登录流程
int process_client_login(struct session* session);

//处理服务端登录流程
int process_server_login(struct session* session);

//控制传输
int cmd_trans(struct session* session, int socket);

//数据传输
int data_trans(struct session* session, int socket);

//提取报文
int extract_packet_buf(struct socket_data* socket_data, struct packet_buf* packet_buf);

//处理FTP命令
int process_command(struct session* session);

//处理FTP响文
int process_response(struct session* session);

//得到FTP命令类型
enum command_type get_command_type(struct command_packet* packet);

//得到FTP响文类型
enum response_type get_response_type(struct response_packet* packet);


#endif  //__FTP_SERVICE_