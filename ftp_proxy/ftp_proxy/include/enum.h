#ifndef __ENUM_
#define __ENUM_

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

//响文枚举
enum response_type {

	RERROR = 0,
	INCOMPLETE,                                // 当前响文还有后续响文
	FTP_227_RESPONSE,                          // 被动模式的响应包
	FTP_220_RESPONSE,                          // 220响应包
	FTP_530_RESPONSE                           // 发送登录响应包
};

#endif