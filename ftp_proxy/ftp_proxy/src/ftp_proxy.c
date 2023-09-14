#include "ftp_proxy.h"
#include "connect.h"
#include "session.h"
#include "process_service.h"
#include "process_client.h"
#include "signal.h"
#include "utils.h"
#include "transport.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/tcp.h>

int init_global_state(char* argv[])
{
	//初始化变量
	global_state.child_keep_on = 1;
	global_state.keep_on = 1;
	global_state.user_flag = 0;
	global_state.server_socket = -1;
	global_state.father_pid = getpid();
	int i = 0;
	for (i = 0; i < BUF_SIZE; i++)
		global_state.child_pids[i] = 0;

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
	int proxy_socket = 0;
	int i = 0;
	global_state.server_socket = init_socket_listen(21, 0);
	printf("%s:begin to accept the user\n", __FUNCTION__);
	do {
		//等待连接
		proxy_socket = accept_socket(global_state.server_socket);
		if (( - 1 == proxy_socket) && (global_state.keep_on)) {
			continue;
		}

		if (global_state.keep_on) {
			pid_t pid = fork();
			if (0 == pid) {
				start_user_session(proxy_socket);
				exit(0);
			}
			else
				add_child_pid(global_state.child_pids, BUF_SIZE, pid);
		}

	} while (global_state.keep_on);

	//开始清理用户
	for (i = 0; i < BUF_SIZE; i++) {
		if (0 != global_state.child_pids[i]) {
			kill(global_state.child_pids[i], CLEAR_USER);
			del_child_pid(global_state.child_pids, BUF_SIZE, global_state.child_pids[i]);
		}
	}
	return 0;
}

static int start_user_session(const int proxy_socket)
{
	int res = -1;
	int flag = 1;
	int i = 0;
	struct pollfd fds[5];
	struct session* session;
	
	init_pollfds(fds, 5);
	session = init_session();
	session->cmd_link->server->proxy_socket = proxy_socket;

	user_login(session);

	add_pollfds(fds, 5, session->cmd_link->server->proxy_socket);
	add_pollfds(fds, 5, session->cmd_link->client->connect_socket);

	do {
		res = poll(fds, 5, 2000);
		if (res < 0) {
			printf("%s:poll error\n", __FUNCTION__);
			break;
		}
		else if (0 == res)
			continue;

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
						global_state.child_keep_on = 0;
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

	} while (global_state.child_keep_on);
	
	//清理session结构体
	destory_session(session);

	//写入信息
	FILE* fd = fopen(FILE_PATH, "a");
	if (NULL == fd) {
		printf("%s:文件打开失败\n", __FUNCTION__);
		return -1;
	}
	fprintf(fd, "%s:put_file_nums:%d, put_bytes:%d\n", session->session_info->user_name, session->session_info->put_file_nums, session->session_info->put_bytes);
	fprintf(fd, "%s:get_file_nums:%d, get_byts:%d\n", session->session_info->user_name, session->session_info->get_file_nums, session->session_info->get_bytes);
	fclose(fd);

	kill(global_state.father_pid,CLEAR_USER);
	return 0;
}

static int data_trans(struct session* session, int socket)
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

		res = safe_recv(socket, socket_data->trans_buf, BUF_SIZE * 8, &socket_data->trans_length);
		if (0 == socket_data->trans_length)
			break;

		//数据统计
		if (1 == status->get_flag)
			info->get_bytes += socket_data->trans_length;
		else if (1 == status->put_flag)
			info->put_bytes += socket_data->trans_length;

		//数据发送
		res = safe_send(send_socket, socket_data->trans_buf, socket_data->trans_length, &socket_data->trans_length);
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

static int cmd_trans(struct session* session, int socket)
{
	int res = -1;
	int flag = (socket == session->cmd_link->server->proxy_socket) ? 1 : 0;

	if (1 == flag)
		res = process_server(session->cmd_link->server);
	else
		res = process_client(session->cmd_link->client);

	return res;
}

int clear_user()
{

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
	res = safe_send(link->server->proxy_socket, link->server->socket_data->trans_buf, link->server->socket_data->trans_length, &send_length);
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
	//cmd_trans(session, session->cmd_link->client->connect_socket);
	return 0;
}
