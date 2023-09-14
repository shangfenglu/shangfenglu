#ifndef __PARSE_COMMAND_
#define __PARSE_COMMAND_

#define COMMAND_NUM  33
struct socket_data;
struct command_packet;
struct packet_buf;

//FTP命令信息
struct command_info {
	char               command[5];                    // 命令类型
	int                space;                         // 是否存在空格
	int                length;                        // 命令长度
};

//解析命令
int parse_command(struct socket_data* socket_data,struct command_packet* packet);

//提取命令报文
static int extract_command_packet(struct socket_data* socket_data, struct packet_buf* packet_buf);

//将buf解析到命令结构体
static int unpack_buf_to_command(struct packet_buf* packet_buf, struct command_packet* packet);


#endif // !__PARSE_COMMAND_
