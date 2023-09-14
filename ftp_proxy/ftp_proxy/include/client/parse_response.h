#ifndef __PARSE_COMMAND_
#define __PARSE_COMMAND_

#define APPEND_SIZE  200

struct socket_data;
struct response_packet;
struct packet_buf;

//FTP命令信息
struct command_info {
	char               command[5];                    // 命令类型
	int                space;                         // 是否存在空格
	int                length;                        // 命令长度
};

//解析响文
int parse_resonse(struct socket_data* data,struct response_packet* packet);

//提取响文buf
static int extract_response_packet(struct socket_data* socket_data, struct packet_buf* packet_buf);

//将buf解析到响文结构体
static int unpack_buf_to_response(struct packet_buf* packet_buf, struct response_packet* packet);

#endif //__PARSE_COMMAND_