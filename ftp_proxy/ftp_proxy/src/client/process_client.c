#include "process_client.h"
#include "session.h"
#include "parse_response.h"
#include "process_response.h"
#include "send_client.h"
#include "send_server.h"
#include "transport.h"
#include <errno.h>

int process_client(struct client* client)
{
	int res = -1;
	struct session* session = (struct session*)(client->father);
	struct link* link = session->cmd_link;
	struct socket_data* socket_data = link->client->socket_data;

	//接收数据
	res = ftp_recv_data(socket_data, link->client->connect_socket);
	if (ENOTCONN == res) {
		/*close(link->server->proxy_socket);
		close(link->client->connect_socket);*/
		return -1;
	}
	else if (EAGAIN == res)
		return 0;
	
	do {
		//解析响应
		res = parse_resonse(socket_data, link->client->packet);
		if (0 == res)
			break;

		//处理响应
		res = process_response(link->client->packet,link->server->socket_data);

		//发送数据
		if (-1 == res)
			send_client(link->client);
		else
			send_server(link->server);
	} while (1);

	return 0;
}
