#ifndef __PROCESS_COMMAND_
#define __PROCESS_COMMAND_

struct session;
struct socket_data;
struct command_packet;
struct session_info;
struct link;

//命令枚举
enum command_type {
	CERROR = 0,
	PASV,                                      // 被动模式
	PORT,                                      // 主动模式
	USER,                                      // 指定用户
	MODE,                                      // 设置传输数据的模式。
	PASS,                                      // 指定登录密码。
	RETR,                                      // 从服务器下载（检索）文件。
	STOR,                                      // 将文件上传（存储）到服务器
	STOU,                                      // 将文件上传（存储），并在服务器上生成唯一的文件名
	AUTH                                       // 身份验证和安全机制
};

//处理FTP命令
int process_command(struct command_packet* packet,struct socket_data* socket_data);

//将命令结构体打包成buf
static int pack_command_to_buf(struct socket_data* socket_data, struct command_packet* packet);

//得到FTP命令类型
static enum command_type get_command_type(struct command_packet* packet);

//处理USER命令
static int process_user_command(struct link* link, struct session_info* info);

//处理PORT命令
static int process_port_command(struct link* cmd_link, struct link* data_link);


#endif // !__PROCESS_COMMAND_
