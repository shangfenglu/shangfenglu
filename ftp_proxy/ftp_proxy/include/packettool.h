#ifndef __PACKETTOOL_
#define __PACKETTOOL_

#include "struct.h"

#define APPEND_SIZE  200
#define COMMAND_NUM  33

//FTP命令信息
struct command_info {
	char               command[5];                    // 命令类型
	int                space;                         // 是否存在空格
	int                length;                        // 命令长度
};

//拷贝buf
int copy_buf(struct socket_data* socket_data, struct packet_buf* packet_buf);

//追加buf
int append_buf(struct socket_data* socket_data, struct packet_buf* packet_buf);

//将buf解析到命令结构体
int unpack_buf_to_command(struct packet_buf* packet_buf, struct command_packet* packet);

//将buf解析到响文结构体
int unpack_buf_to_response(struct packet_buf* packet_buf, struct response_packet* packet);

//将命令结构体打包成buf
int pack_command_to_buf(struct socket_data* socket_data, struct command_packet* packet);

//将响文结构体打包成buf
int pack_response_to_buf(struct socket_data* socket_data, struct response_packet* packet);

//处理USER命令
int process_user_command(struct link* link,struct session_info* info);

//处理PORT命令
int process_port_command(struct link* cmd_link, struct link* data_link);

//处理227响文
int process_227_response(struct link* cmd_link, struct link* data_link);

#endif   //__PACKETTOOL_