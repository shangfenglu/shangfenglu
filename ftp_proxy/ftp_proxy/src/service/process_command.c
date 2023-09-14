#include "process_command.h"
#include "session.h"

#include <stdio.h>
#include <string.h>

int process_command(struct command_packet* packet, struct socket_data* socket_data)
{
	int res = 0;
	struct session* session = (struct session*)(packet->father);
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
			socket_data = link->server->socket_data;
			strcpy(socket_data->trans_buf, "530 Please login with USER and PASS.\r\n");
			socket_data->trans_length = 38;
			res = -1;
		}
	default:
		break;
	}

	if (-1 != res)
		pack_command_to_buf(socket_data, session->cmd_link->server->packet);

	return res;
}

int pack_command_to_buf(struct socket_data* socket_data, struct command_packet* packet)
{
	int length = (32 == packet->space) ? 1 : 0;
	int command_length = (strlen(packet->command) >= 4) ? 4 : strlen(packet->command);
	if ((packet->arguements->data_length + 5) >= (BUF_SIZE * 2)) {
		printf("%s:packet is to big\n", __FUNCTION__);
		return -1;
	}
	if (4 == command_length) {
		if (32 == packet->space)
			sprintf(socket_data->trans_buf, "%c%c%c%c%c%s\r\n", packet->command[0], packet->command[1],
				packet->command[2], packet->command[3], packet->space, packet->arguements->data);
		else
			sprintf(socket_data->trans_buf, "%c%c%c%c\r\n", packet->command[0], packet->command[1],
				packet->command[2], packet->command[3]);
	}
	else {
		if (32 == packet->space)
			sprintf(socket_data->trans_buf, "%c%c%c%c%s\r\n", packet->command[0], packet->command[1],
				packet->command[2], packet->space, packet->arguements->data);
		else
			sprintf(socket_data->trans_buf, "%c%c%c\r\n", packet->command[0], packet->command[1],
				packet->command[2]);

	}
	socket_data->trans_length = command_length + length + packet->arguements->data_length + 2;
	packet->command[3] = '\0';
	return 0;
}

static enum command_type get_command_type(struct command_packet* packet)
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

static int process_user_command(struct link* link, struct session_info* info)
{
	char ip[16] = {};
	char port[4] = {};
	char user_name[40] = {};
	char* ip_index = NULL;
	char* port_index = NULL;
	struct command_packet* packet = link->server->packet;

	ip_index = strstr(packet->arguements->data, "ip:");
	port_index = strstr(packet->arguements->data, "port:");
	if ((ip_index != NULL) && (port_index != NULL)) {
		sscanf(packet->arguements->data, "%s ip:%s port:%s", user_name, ip, port);
		sprintf(packet->arguements->data, "%s", user_name);
		packet->arguements->data_length = strlen(packet->arguements->data);
		link->client->server_port = atoi(port);
		strcpy(link->client->server_ip, ip);
	}
	else if ((ip_index != NULL) && (port_index == NULL)) {
		sscanf(packet->arguements->data, "%s ip:%s", user_name, ip);
		sprintf(packet->arguements->data, "%s", user_name);
		packet->arguements->data_length = strlen(packet->arguements->data);
		strcpy(link->client->server_ip, ip);
	}
	else if ((ip_index == NULL) && (port_index == NULL)) {
		sscanf(packet->arguements->data, "%s", user_name);
	}
	strcpy(info->user_name, user_name);

	return 0;
}

static int process_port_command(struct link* cmd_link, struct link* data_link)
{
	char server_ip[16] = {};
	int server_port = 0;
	int ip[4] = {};
	int port[2] = {};
	struct command_packet* packet = cmd_link->server->packet;

	sscanf(packet->arguements->data, "%d,%d,%d,%d,%d,%d",
		&ip[0], &ip[1], &ip[2], &ip[3], &port[0], &port[1]);
	sprintf(server_ip, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
	server_port = port[0] * 256 + port[1];

	data_link->client->listen_port = server_port;
	strcpy(data_link->client->server_ip, server_ip);
	data_link->client->server_port = server_port;

	sprintf(packet->arguements->data, "%s,%d,%d", global_state.local_ip, port[0], port[1]);
	packet->arguements->data_length = strlen(packet->arguements->data);

	return 0;
}
