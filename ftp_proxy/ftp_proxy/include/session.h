#ifndef __STRUCT_
#define __STRUCT_
#include"str.h"
#include <sys/types.h>
#define BUF_SIZE  1024
#define NUM       10

//临时packet_buf
struct packet_buf {
	struct mystr*      string;                        // 字符串
	int                is_complete;                   // 是否完整
	int                is_signal;                     // 是否是单个报文
};

struct {
	int                child_keep_on;                 // 子进程while标志
	int                keep_on;                       // while标志 
	int                user_flag;                     // 接收到user标志
	int                server_socket;                 // 代理监听socket
	pid_t              father_pid;                    // 父进程pid
	pid_t              child_pids[BUF_SIZE];          // 子进程pids

	char               local_ip[16];                  // 本地ip
	char               server_ip[16];                 // 服务端ip
	struct packet_buf* packet_buf;                    // FTP临时保存包结构体

}global_state;

//FTP命令报文
struct command_packet {
	char               command[4];                    // 命令类型
	char               space;                         // 空格
	struct mystr*      arguements;                    // 参数内容

	void*              father;                        // 父结构体
};

//FTP相应报文
struct response_packet {
	char               code[3];                       // 响应码
	char               space;                         // 空格
	struct mystr*      message;                       // 响应消息

	void*              father;                        // 父结构体
};

//收发数据结构体
struct socket_data {
	char               trans_buf[BUF_SIZE * 8 + 1];   // 收发buf
	unsigned int       trans_length;                  // 收发长度
	unsigned int       index_begin;                   // 开始索引
	unsigned int       index_end;                     // 结束索引
};

//服务端结构体
struct server {
	int                 proxy_socket;                 // 连接客户端socket
	struct socket_data* socket_data;                  // 收发buf结构体
	struct command_packet* packet;                    // 命令报文 
	void*               father;                       // 父结构体

};

//客户端结构体
struct client {
	int                 connect_socket;               // 连接服务端socket
	int                 accept_socket;                // 等待连接socket
	char                server_ip[16];                // 服务端ip
	unsigned int        server_port;                  // 服务端端口
	unsigned int        listen_port;                  // 正在监听的端口

	struct socket_data* socket_data;                  // 收发buf结构体
	struct response_packet* packet;                   // 响应结构体
	void*               father;                       // 父结构体

};

//连接结构体
struct link {
	struct server* server;                            // 服务端信息                 
	struct client* client;                            // 客户端信息
	void*          father;                            // 父结构体
};

//用户信息结构体
struct session_info {
	char                user_name[40];                // 用户名
	long int            get_bytes;                    // 接收字节数
	int                 get_file_nums;                // 接收文件数量

	long int            put_bytes;                    // 上传字节数
	int                 put_file_nums;                // 上传文件数量
};

//会话状态信息结构体
struct session_status {
	int                 first_login;                   // 第一次登录
	int                 get_flag;                      // 下载标志
	int                 put_flag;                      // 上传标志
};

//会话结构体
struct session {
	struct link* cmd_link;                            // 控制连接
	struct link* data_link;                           // 传输连接
	struct session_info* session_info;                // 用户信息结构体
	struct session_status* session_status;            // 会话状态结构体
};

#endif
