#include "../include/utils.h"

#define MAX_IFACES 64
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdlib.h>
#include <unistd.h>

int get_local_ip(char* local_ip)
{
	int i = 0;
	struct ifaddrs* ifaddr, * ifa;
	int family, s;

	if (getifaddrs(&ifaddr) == -1)
	{
		perror("getifaddrs");
		exit(EXIT_FAILURE);
	}

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr == NULL)
			continue;

		family = ifa->ifa_addr->sa_family;

		if (family == AF_INET)
		{
			if (0 == strcmp(ifa->ifa_name, "ens33"))
				strcpy(local_ip, inet_ntoa(((struct sockaddr_in*)ifa->ifa_addr)->sin_addr));
		}
	}
	freeifaddrs(ifaddr);
	for (i = 0; i < 16; i++) {
		if (local_ip[i] == '.')
			local_ip[i] = ',';
	}
	return 0;
}

struct session* init_session()
{
	//会话结构体
	struct session* session = (struct session*)malloc(sizeof(struct session));
	if (NULL == session) {
		printf("%s:malloc session %s\n", __FUNCTION__, strerror(errno));
		return NULL;
	}

	//连接结构体
	session->cmd_link = (struct link*)malloc(sizeof(struct link));
	session->data_link = (struct link*)malloc(sizeof(struct link));
	session->session_info = (struct session_info*)malloc(sizeof(struct session_info));
	session->session_status = (struct session_status*)malloc(sizeof(struct  session_status));
	if ((NULL == session->cmd_link) || (NULL == session->data_link) || 
		(NULL == session->session_info) || (NULL == session->session_status)) {
		printf("%s:malloc link %s\n", __FUNCTION__, strerror(errno));
		return NULL;
	}

	//客户端，服务端结构体
	session->cmd_link->server = (struct server*)malloc(sizeof(struct server));
	session->cmd_link->client = (struct client*)malloc(sizeof(struct client));
	session->data_link->server = (struct server*)malloc(sizeof(struct server));
	session->data_link->client = (struct client*)malloc(sizeof(struct client));
	if ((NULL == session->cmd_link->server) || (NULL == session->cmd_link->client) ||
		(NULL == session->data_link->server) || (NULL == session->data_link->client)) {
		printf("%s:malloc server %s\n", __FUNCTION__, strerror(errno));
		return NULL;
	}

	//收发结构体
	session->cmd_link->server->socket_data = (struct socket_data*)malloc(sizeof(struct socket_data));
	session->cmd_link->server->packet = (struct command_packet*)malloc(sizeof(struct command_packet));
	session->cmd_link->client->socket_data = (struct socket_data*)malloc(sizeof(struct socket_data));
	session->cmd_link->client->packet = (struct response_packet*)malloc(sizeof(struct response_packet));
	if ((NULL == session->cmd_link->server->socket_data) || (NULL == session->cmd_link->server->packet) ||
		(NULL == session->cmd_link->client->socket_data) || (NULL == session->cmd_link->client->packet)) {
		printf("%s:malloc cmd_link socket_data %s\n", __FUNCTION__, strerror(errno));
		return NULL;
	}

	session->data_link->server->socket_data = (struct socket_data*)malloc(sizeof(struct socket_data));
	session->data_link->server->packet = (struct command_packet*)malloc(sizeof(struct command_packet));
	session->data_link->client->socket_data = (struct socket_data*)malloc(sizeof(struct socket_data));
	session->data_link->client->packet = (struct response_packet*)malloc(sizeof(struct response_packet));
	if ((NULL == session->data_link->server->socket_data) || (NULL == session->data_link->server->packet) ||
		(NULL == session->data_link->client->socket_data) || (NULL == session->data_link->client->packet)) {
		printf("%s:malloc data_link socket_data %s\n", __FUNCTION__, strerror(errno));
		return NULL;
	}

	init_link(session->cmd_link);
	init_link(session->data_link);
	session->session_status->first_login = 1;
	session->session_status->get_flag = 0;
	session->session_status->put_flag = 0;

	session->session_info->user_name[0] = '\0';
	session->session_info->get_bytes = 0;
	session->session_info->get_file_nums = 0;
	session->session_info->put_bytes = 0;
	session->session_info->put_file_nums = 0;

	return session;
}

int init_link(struct link* link)
{
	//server
	link->server->proxy_socket = -1;
	link->server->socket_data->trans_length = 0;
	link->server->socket_data->index_begin = 0;
	link->server->socket_data->index_end = 0;
	link->server->packet->command[3] = '\0';
	link->server->packet->space = '\0';
	link->server->packet->arguements = (char*)malloc(sizeof(char) * MEMORY_SIZE);
	link->server->packet->memory_size = MEMORY_SIZE;
	link->server->packet->arguement_length = 0;

	//client
	link->client->accept_socket = -1;
	link->client->connect_socket = -1;
	strcpy(link->client->server_ip, global_state.server_ip);
	link->client->server_port = 21;
	link->client->listen_port = 21;
	link->client->socket_data->trans_length = 0;
	link->client->socket_data->index_begin = 0;
	link->client->socket_data->index_end = 0;
	link->client->packet->code[0] = '\0';
	link->client->packet->space = ' ';
	link->client->packet->message = (char*)malloc(sizeof(char) * MEMORY_SIZE);
	link->client->packet->memory_size = MEMORY_SIZE;
	link->client->packet->message_length = 0;

	return 0;
}

struct packet_buf* init_packet_buf()
{
	struct packet_buf* packet_buf = (struct packet_buf*)malloc(sizeof(struct packet_buf));
	if (NULL == packet_buf) {
		printf("%s:malloc packet_buf %s\n", __FUNCTION__, strerror(errno));
		return NULL;
	}

	packet_buf->packet_buf = (char*)malloc(sizeof(char)* MEMORY_SIZE);
	packet_buf->memory_size = MEMORY_SIZE;
	packet_buf->is_complete = 1;
	packet_buf->buf_length = 0;
	packet_buf->is_signal = 1;

	return packet_buf;
}

int init_pollfds(struct pollfd* fds, int num)
{
	int i = 0;
	for (i = 0; i < num; i++) {
		fds[i].fd = -1;
		fds[i].events = POLLIN;
	}

	return 0;
}

int add_pollfds(struct pollfd* fds, int num, const int socket)
{
	int i = 0;
	for (i = 0; i < num; i++) {
		if (-1 == fds[i].fd) {
			fds[i].fd = socket;
			break;
		}
	}
	return 0;
}

int del_pollfds(struct pollfd* fds, int num, const int socket)
{
	int i = 0;
	for (i = 0; i < num; i++) {
		if (socket == fds[i].fd) {
			fds[i].fd = -1;
			break;
		}
	}
	return 0;
}

void* append_memory(void* addr, unsigned int size, unsigned int append_length)
{
	void* addr_ret = malloc(sizeof(char) * (size + append_length));
	memcpy(addr_ret, addr, sizeof(char) * size);
	free(addr);
	addr = NULL;
	return addr_ret; 
}

int compare_string(const char* dst, const char* src, int length)
{
	int i = 0;
	for (i = 0; i < length; i++) {
		if (dst[i] != src[i]) {
			return -1;
		}
	}
	return 0;
}

int is_complete(char* src)
{
	int res = -1;
	if ('-' == src[3])
		res = -1;
	else
		res = 0;
	return res;
}

int string_upper(char* str, int length)
{
	int i = 0;
	for (i = 0; i < length; i++) {
		str[i] = toupper(str[i]);
	}
	return 0;
}



