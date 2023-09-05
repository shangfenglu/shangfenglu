#include "../include/ftp_proxy.h"
#include "../include/nettool.h"
#include "../include/utils.h"
#include "../include/packettool.h"

#include <poll.h>
#include <unistd.h>

int init_global_state(char* argv[])
{
	//初始化变量
	global_state.keep_on = 1;
	global_state.user_flag = 0;
	
	//初始化临时pakcet
	global_state.packet_buf = init_packet_buf();
	
	//初始化ip地址
	strcpy(global_state.server_ip, argv[1]);
	get_local_ip(global_state.local_ip);
	
	//创建文件
	FILE* fd = fopen(FILE_PATH, "w");
	fclose(fd);

	return 0;
}

int start_proxy_service()
{
	int server_socket = 0;
	int proxy_socket = 0;
	int res = 0;
	struct session* session;
	session = init_session();
	server_socket = init_socket_listen(21,0);
	printf("%s:begin to accept the user\n", __FUNCTION__);
	do {
		proxy_socket = accept_socket(server_socket);
		if (-1 == proxy_socket) {
			continue;
		}
		session->cmd_link->server->proxy_socket = proxy_socket;
		if (0 == fork()) {
			start_user_session(session);
			break;
		}
	} while (global_state.keep_on);

	//清理资源SIGQUIT
	close(server_socket);
	printf("%s:over\n", __FUNCTION__);
	return 0;
}

int start_user_session(struct session* session)
{
	int res = -1;
	int flag = 1;
	int keep_on = 1;
	int i = 0;
	struct pollfd fds[5];
	init_pollfds(fds, 5);

	user_login(session);

	add_pollfds(fds, 5, session->cmd_link->server->proxy_socket);
	add_pollfds(fds, 5, session->cmd_link->client->connect_socket);
	
	do {
		res = poll(fds, 5, 1000);
		if (res < 0) {
			printf("%s:poll error\n", __FUNCTION__);
			break;
		}
		
		if (res > 0) {
			for (i = 0; i < 5; i++) {	
				if ((fds[i].fd != -1) && (fds[i].revents & POLLIN) && (fds[i].fd == session->data_link->client->accept_socket)) {
					//创建数据连接
					session->data_link->server->proxy_socket = accept_socket(session->data_link->client->accept_socket);
					connect_to_server(session->data_link);
					add_pollfds(fds, 5, session->data_link->server->proxy_socket);
					add_pollfds(fds, 5, session->data_link->client->connect_socket);
				}
				else if ((fds[i].fd != -1) && (fds[i].revents & POLLIN)) {
					
					if ((fds[i].fd == session->cmd_link->server->proxy_socket) ||
						(fds[i].fd == session->cmd_link->client->connect_socket)) {
						res = cmd_trans(session, fds[i].fd);
						if (-1 == res) {
							del_pollfds(fds, 5, session->cmd_link->server->proxy_socket);
							del_pollfds(fds, 5, session->cmd_link->client->connect_socket);
							//退出程序
							keep_on = 0;
							break;
						}
					}
					else {
						res = data_trans(session, fds[i].fd);
						if (-1 == res) {
							del_pollfds(fds, 5, session->data_link->server->proxy_socket);
							del_pollfds(fds, 5, session->data_link->client->connect_socket);
							del_pollfds(fds, 5, session->data_link->client->accept_socket);
							session->data_link->client->accept_socket = -1;
							session->session_status->get_flag = 0;
							session->session_status->put_flag = 0;
						}
					}

				}

				if ((1 == flag) && (session->data_link->client->accept_socket != -1)) {
					add_pollfds(fds, 5, session->data_link->client->accept_socket);
					flag = 0;
				}
				else if (-1 == session->data_link->client->accept_socket)
					flag = 1;
			}
		}
	
	} while (keep_on);
	
	//写入信息
	
	FILE* fd = fopen(FILE_PATH, "a");
	if (NULL == fd) {
		printf("%s:文件打开失败\n", __FUNCTION__);
		return -1;
	}
	fprintf(fd, "%s:put_file_nums:%d, put_bytes:%d\n", session->session_info->user_name, session->session_info->put_file_nums, session->session_info->put_bytes);
	fprintf(fd, "%s:get_file_nums:%d, get_byts:%d\n", session->session_info->user_name, session->session_info->get_file_nums, session->session_info->get_bytes);

	fclose(fd);
	return 0;
}

int user_login(struct session* session)
{
	process_client_login(session);
	process_server_login(session);
	return 0;
}

int process_client_login(struct session* session)
{
	int res = 0;
	struct link* link = session->cmd_link;
	int send_length = 0;
	strcpy(link->server->socket_data->trans_buf, FTP_WELOCME_STRING);
	link->server->socket_data->trans_length = 34;
	res = nSend(link->server->proxy_socket, link->server->socket_data->trans_buf, link->server->socket_data->trans_length,&send_length);
	if (0 == send_length) {
		printf("发送失败\n");
	}
	while (1) {
		usleep(1000);
		cmd_trans(session, link->server->proxy_socket);
		if (1 == global_state.user_flag) {
			break;
		}
	}
	global_state.user_flag = 0;
	
	return 0;
}

int process_server_login(struct session* session)
{
	connect_to_server(session->cmd_link);
	cmd_trans(session, session->cmd_link->client->connect_socket);
	return 0;
}

int cmd_trans(struct session* session, int socket)
{
	int res = -1;
	int keep_on = 1;
	struct link* link = session->cmd_link;
	int flag = (link->server->proxy_socket == socket) ? 1 : 0;                  // flag:1 表示发生动静的是server端
	int send_socket = (1 == flag) ? link->client->connect_socket : link->server->proxy_socket;
	struct socket_data* socket_data = (1 == flag) ? link->server->socket_data : link->client->socket_data;
	struct packet_buf* packet_buf = global_state.packet_buf;

	do {
		socket_data->index_begin = 0;
		socket_data->index_end = 0;
		socket_data->trans_length = 0;

		//接收数据
		res = nRecv(socket, socket_data->trans_buf, BUF_SIZE * 2, &socket_data->trans_length);
		if (0 == socket_data->trans_length) 
			break;
		socket_data->trans_buf[socket_data->trans_length] = '\0';
		
		while (1) {
			//提取报文
			res = extract_packet_buf(socket_data, packet_buf);
			if (0 == res)
				break;

			if (1 == flag) {
				struct client* client = link->client;
				//解包
				unpack_buf_to_command(packet_buf, link->server->packet);
				
				//处理
				res = process_command(session);
				
				//封包
				if (-1 == res)
					send_socket = link->server->proxy_socket;
				else
					pack_command_to_buf(client->socket_data, link->server->packet);
				
				//发送
				if (0 == global_state.user_flag)
					res = nSend(send_socket, client->socket_data->trans_buf, client->socket_data->trans_length, &client->socket_data->trans_length);
				client->socket_data->trans_length = 0;
			}
			else {
				struct server* server = link->server;
				unpack_buf_to_response(packet_buf, link->client->packet);
				res = process_response(session);
				if (-1 == res)
					send_socket = link->client->connect_socket;
				else
					pack_response_to_buf(server->socket_data, link->client->packet);
				res = nSend(send_socket, server->socket_data->trans_buf, server->socket_data->trans_length, &server->socket_data->trans_length);
				server->socket_data->trans_length = 0;
			}
			
			if (ENOTCONN == res) {
				keep_on = 0;
				break;
			}
		}

	} while (keep_on);

	//关闭连接
	if (ENOTCONN == res) {
		close(session->cmd_link->server->proxy_socket);
		close(session->cmd_link->client->connect_socket);
		return -1;
	}
	return 0;
}

int data_trans(struct session* session, int socket)
{
	int res = -1;
	struct link* link = session->data_link;
	int flag = (link->server->proxy_socket == socket) ? 1 : 0;
	int send_socket = (1 == flag) ? link->client->connect_socket : link->server->proxy_socket;
	struct socket_data* socket_data = (1 == flag) ? link->server->socket_data : link->client->socket_data;
	struct session_status* status = session->session_status;
	struct session_info* info = session->session_info;

	do {
		socket_data->trans_length = 0;
		
		res = nRecv(socket, socket_data->trans_buf, BUF_SIZE * 6, &socket_data->trans_length);
		if (0 == socket_data->trans_length)
			break;
		socket_data->trans_buf[socket_data->trans_length] = '\0';
		
		//数据统计
		if (1 == status->get_flag)
			info->get_bytes += socket_data->trans_length;
		else if (1 == status->put_flag)
			info->put_bytes += socket_data->trans_length;

		//数据发送
		res = nSend(send_socket, socket_data->trans_buf, socket_data->trans_length, &socket_data->trans_length);
		if (0 != res)
			break;
	} while (1);
	
	if (ENOTCONN == res) {
		close(link->client->connect_socket);
		close(link->client->accept_socket);
		close(link->server->proxy_socket);
		status->get_flag = 0;
		status->put_flag = 0;
		return -1;
	}
	return 0;
}

int extract_packet_buf(struct socket_data* socket_data, struct packet_buf* packet_buf)
{

	int ret = 0;
	int i = 0;
	char* buf = socket_data->trans_buf;

	for (i = socket_data->index_begin; i < socket_data->trans_length -1; i++) {
		if ((buf[i] == '\r') && (buf[i + 1] == '\n')) {
			
			//提取到一个完整的报文
			socket_data->index_end = i + 2;
			if (0 == packet_buf->is_complete) {
				append_buf(socket_data, packet_buf);
				packet_buf->is_complete = 1;
				
				//判断是否结束
				if (1 == packet_buf->is_signal) {
					ret = 0;
					socket_data->index_begin = socket_data->index_end;
					continue;
				}
				else
					ret = 1;
			}
			else {
				//判断是否是单个报文
				if (0 == packet_buf->is_signal) {
					append_buf(socket_data, packet_buf);
					if ((0 == compare_string(packet_buf->packet_buf, socket_data->trans_buf + socket_data->index_begin, 3)) &&
						(0 == is_complete(socket_data->trans_buf + socket_data->index_begin))) {
						packet_buf->is_signal = 1;
						ret = 1;
					}
					else {
						ret = 0;
						socket_data->index_begin = socket_data->index_end;
						continue;
					}
				}
				else {
					copy_buf(socket_data, packet_buf);
					if (-1 == is_complete(packet_buf->packet_buf)) {
						packet_buf->is_signal = 0;
						ret = 0;
						socket_data->index_begin = socket_data->index_end;
						continue;
					}
					else
						ret = 1;
				}
			}
			break;
		}

		if ((i+2) >= socket_data->trans_length) {
			socket_data->index_end = i + 2;
			if (1 == packet_buf->is_complete) {
				copy_buf(socket_data, packet_buf);
				packet_buf->is_complete = 0;
				//判断当前半截是否是响文结束
				if (0 == packet_buf->is_signal) {
					if ((0 == compare_string(packet_buf->packet_buf, socket_data->trans_buf + socket_data->index_begin, 3)) &&
						(0 == is_complete(socket_data->trans_buf + socket_data->index_begin))) {
						packet_buf->is_signal = 1;
					}
				}
			}
			else
				append_buf(socket_data, packet_buf);

			ret = 0;
			break;
		}
	}

	//位置更新
	socket_data->index_begin = socket_data->index_end;
	return ret;
}

int process_command(struct session* session)
{

	int res = 0;
	struct link* link = session->cmd_link;
	struct session_status* status = session->session_status;
	struct session_info* info = session->session_info;
	enum command_type type = get_command_type(link->server->packet);
	
	switch (type) {
	case CERROR:
		break;
	case PASV:
		break;
	case PORT:
	{
		process_port_command(session->cmd_link, session->data_link);
		session->data_link->client->accept_socket = init_socket_listen(session->data_link->client->listen_port, 1);
		accept_socket(session->data_link->client->accept_socket);
	}
		break;
	case USER:
		if (1 == status->first_login) {
			process_user_command(session->cmd_link, session->session_info);
			global_state.user_flag = 1;
		}
		break;
	case MODE:
		break;
	case PASS:
		status->first_login = 0;
		break;
	case RETR:
	{
		status->get_flag = 1;
		info->get_file_nums += 1;
	}
		break;
	case STOR:
	{
		status->put_flag = 1;
		info->put_file_nums += 1;
	}
		break;
	case STOU:
	{
		status->put_flag = 1;
		info->put_file_nums += 1;
	}
		break;
	case AUTH:
		if (1 == status->first_login) {
			strcpy(link->client->socket_data->trans_buf, "530 Please login with USER and PASS.\r\n");
			link->client->socket_data->trans_length = 38;
			res = -1;
		}
	default :
		break;
	}
	
	return res;
}

int process_response(struct session* session)
{
	int res = 0;
	struct link* link = session->cmd_link;
	struct session_status* status = session->session_status;
	enum response_type type = get_response_type(link->client->packet);

	switch (type)
	{
	case RERROR:
		break;
	case FTP_227_RESPONSE:
	{
		process_227_response(session->cmd_link, session->data_link);
		session->data_link->client->accept_socket = init_socket_listen(session->data_link->client->listen_port, 1);
		accept_socket(session->data_link->client->accept_socket);
	}
		break;
	case FTP_220_RESPONSE:
		if (1 == status->first_login) {
			res = -1;
			sprintf(session->cmd_link->server->socket_data->trans_buf, "USER %s\r\n", session->session_info->user_name);
			session->cmd_link->server->socket_data->trans_length = 7 + strlen(session->session_info->user_name);
		}
		break;
	case FTP_530_RESPONSE:
		break;
	default:
		break;
	}
	return res;
}

enum command_type get_command_type(struct command_packet* packet)
{
	enum command_type type = CERROR;
	if (0 == compare_string(packet->command, "PASV", 4)) {
		//PASV
		type = PASV;
	}
	else if (0 == compare_string(packet->command, "PORT", 4)) {
		//PORT
		type = PORT;
	}
	else if (0 == compare_string(packet->command, "USER", 4)) {
		//USER
		type = USER;
	}
	else if (0 == compare_string(packet->command, "PASS", 4)) {
		//PASS  
		type = PASS;
	}
	else if (0 == compare_string(packet->command, "RETR", 4)) {
		//RETR
		type = RETR;
	}
	else if (0 == compare_string(packet->command, "STOR", 4)) {
		//STOR
		type = STOR;
	}
	else if (0 == compare_string(packet->command, "STOU", 4)) {
		//STOU
		type = STOU;
	}
	else if (0 == compare_string(packet->command, "AUTH", 4)) {
		//AUTH
		type = AUTH;
	}

	return type;
}

enum response_type get_response_type(struct response_packet* packet)
{
	enum response_type type = RERROR;
	if (0 == compare_string(packet->code, "227 ", 3)) {
		//FTP_227_RESPONSE
		type = FTP_227_RESPONSE;
	}
	else if (0 == compare_string(packet->code, "220", 3)) {
		//FTP_220_RESPONSE
		type = FTP_220_RESPONSE;
	}
	else if (0 == compare_string(packet->code, "530", 3)) {
		type = FTP_530_RESPONSE;
	}

	return type;
}


