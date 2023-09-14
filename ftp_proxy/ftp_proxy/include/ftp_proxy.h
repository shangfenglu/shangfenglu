#ifndef __FTP_SERVICE_
#define __FTP_SERVICE_

#include "session.h"

#define FTP_WELOCME_STRING "220 Welcome to blah FTP service.\r\n"
#define FILE_PATH "user_info.txt"

//初始化全局变量
int init_global_state(char* argv[]);

//开启ftp_service服务
int start_proxy_service();

//用户会话
static int start_user_session(const int proxy_socket);

//数据传输
static int data_trans(struct session* session, int socket);

//控制传输
static int cmd_trans(struct session* session, int socket);

//清理用户
static int clear_user();

//用户登录处理
static int user_login(struct session* session);

//处理客户端登录流程
static int process_client_login(struct session* session);

//处理服务端登录流程
static int process_server_login(struct session* session);



#endif //__FTP_SERVICE_