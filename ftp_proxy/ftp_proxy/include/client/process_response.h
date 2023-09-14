#ifndef __PROCESS_RESPONSE_
#define __PROCESS_RESPONSE_
struct session;
struct socket_data;
struct response_packet;
struct link;

//响文枚举
enum response_type {

	RERROR = 0,
	INCOMPLETE,                                // 当前响文还有后续响文
	FTP_227_RESPONSE,                          // 被动模式的响应包
	FTP_220_RESPONSE,                          // 220响应包
	FTP_530_RESPONSE                           // 发送登录响应包
};


//处理FTP响文
int process_response(struct response_packet* packet,struct socket_data* socket_data);

//将响文结构体打包成buf
static int pack_response_to_buf(struct socket_data* socket_data, struct response_packet* packet);

//得到FTP响文类型
static enum response_type get_response_type(struct response_packet* packet);

//处理227响文
static int process_227_response(struct link* cmd_link, struct link* data_link);

#endif // !__PROCESS_RESPONSE_