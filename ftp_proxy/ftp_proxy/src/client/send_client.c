#include "send_client.h"
#include "session.h"
#include <transport.h>

int send_client(struct client* client)
{
	ftp_send_data(client->socket_data, client->connect_socket);
	return 0;
}
