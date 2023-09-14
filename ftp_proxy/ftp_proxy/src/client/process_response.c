#include "process_response.h"
#include "session.h"
#include <stdio.h>
#include <string.h>

int process_response(struct response_packet* packet, struct socket_data* socket_data)
{
	int res = 0;
	struct session* session = (struct session*)(packet->father);
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
			socket_data = link->client->socket_data;
			sprintf(socket_data->trans_buf, "USER %s\r\n", session->session_info->user_name);
			socket_data->trans_length = 7 + strlen(session->session_info->user_name);
		}
		break;
	case FTP_530_RESPONSE:
		break;
	default:
		break;
	}
	if (-1 != res)
		pack_response_to_buf(socket_data, session->cmd_link->client->packet);

	return res;
}

int pack_response_to_buf(struct socket_data* socket_data, struct response_packet* packet)
{
	if ((packet->message->data_length + 6) >= (BUF_SIZE * 6)) {
		printf("%s:packet is to big\n", __FUNCTION__);
		return -1;
	}
	sprintf(socket_data->trans_buf, "%c%c%c%c%s\r\n",
		packet->code[0], packet->code[1], packet->code[2], packet->space, packet->message->data);
	socket_data->trans_length = packet->message->data_length + 6;

	return 0;
}

static enum response_type get_response_type(struct response_packet* packet)
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

static int process_227_response(struct link* cmd_link, struct link* data_link)
{
	char server_ip[16] = {};
	int server_port = 0;
	int ip[4] = {};
	int port[2] = {};
	struct response_packet* packet = cmd_link->client->packet;
	sscanf(packet->message->data, "Entering Passive Mode (%d,%d,%d,%d,%d,%d).",
		&ip[0], &ip[1], &ip[2], &ip[3], &port[0], &port[1]);
	sprintf(server_ip, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
	server_port = port[0] * 256 + port[1];

	data_link->client->listen_port = server_port;
	strcpy(data_link->client->server_ip, server_ip);
	data_link->client->server_port = server_port;

	sprintf(packet->message->data, "Entering Passive Mode (%s,%d,%d).",
		global_state.local_ip, port[0], port[1]);
	packet->message->data_length = strlen(packet->message->data);

	return 0;
}