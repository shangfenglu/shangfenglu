#include "send_server.h"
#include "session.h"
#include "transport.h"

int send_server(struct server* server)
{
	ftp_send_data(server->socket_data, server->proxy_socket);
	return 0;
}
