#include "process_service.h"
#include "session.h"
#include "transport.h"
#include "parse_command.h"
#include "process_command.h"
#include "send_client.h"
#include "send_server.h"
#include <errno.h>
#include <stdio.h>

int process_server(struct server* server)
{
	int res = -1;
	struct session* session = (struct session*)(server->father);
	struct link* link = session->cmd_link;
	int recv_socket = link->server->proxy_socket;
	int send_socket = link->client->connect_socket;
	struct socket_data* socket_data = link->server->socket_data;

	//接收数据
	res = ftp_recv_data(socket_data, recv_socket);
	if (ENOTCONN == res) {
		/*close(link->server->proxy_socket);
		close(link->client->connect_socket);*/
		return -1;
	}
	else if (EAGAIN == res)
		return 0;

	do {
		//解析命令
		res = parse_command(socket_data, link->server->packet);
		if (0 == res)
			break;

		//处理命令
		res = process_command(link->server->packet,link->client->socket_data);
		if (-1 == res)
			send_socket = recv_socket;

		//发送数据
		if (0 == global_state.user_flag)
			ftp_send_data(link->client->socket_data, send_socket);

	} while (1);
	
	return 0;
}
