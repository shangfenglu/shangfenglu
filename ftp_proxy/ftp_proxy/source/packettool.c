#include "../include/packettool.h"
#include "../include/nettool.h"
#include "../include/utils.h"

#include <stdio.h>
#include <errno.h>

struct command_info command_infos[] = {
	{"USER",1,4},
	{"PASS",1,4},
	{"ACCT",1,4},
	{"CWD",1,3},
	{"CDUP",0,4},
	{"SMNT",1,4},
	{"QUIT",0,4},
	{"REIN",0,4},
	{"PORT",1,4},
	{"PASV",0,4},
	{"TYPE",1,4},
	{"STRU",1,4},
	{"MODE",1,4},
	{"RETR",1,4},
	{"STOR",1,4},
	{"STOU",0,4},
	{"APPE",1,4},
	{"ALLO",1,4},
	{"REST",1,4},
	{"RNFR",1,4},
	{"RNTO",1,4},
	{"ABOR",0,4},
	{"DELE",1,4},
	{"RMD",1,3},
	{"MKD",1,3},
	{"PWD",0,3},
	{"LIST",0,4},
	{"NLST",0,4},
	{"SITE",1,4},
	{"SYST",0,4},
	{"STAT",1,4},
	{"HELP",1,4},
	{"NOOP",0,4},
};

int copy_buf(struct socket_data* socket_data, struct packet_buf* packet_buf)
{
	int index_end = socket_data->index_end;
	int index_begin = socket_data->index_begin;

	if ((index_end - index_begin) >= packet_buf->memory_size) {
		packet_buf->packet_buf = (char*)append_memory(packet_buf->packet_buf, packet_buf->memory_size, APPEND_SIZE);
		if (NULL == packet_buf->packet_buf) {
			printf("%s:append_memory %s\n", __FUNCTION__, strerror(errno));
			return -1;
		}
		packet_buf->memory_size += APPEND_SIZE;
	}

	memcpy(packet_buf->packet_buf, socket_data->trans_buf + index_begin,sizeof(char)*(index_end - index_begin));
	packet_buf->buf_length = index_end - index_begin;
	packet_buf->packet_buf[packet_buf->buf_length] = '\0';
	
	return 0;
}

int append_buf(struct socket_data* socket_data, struct packet_buf* packet_buf)
{
	int index_end = socket_data->index_end;
	int index_begin = socket_data->index_begin;

	if ((index_end - index_begin) >= (packet_buf->memory_size - packet_buf->buf_length)) {
		packet_buf->packet_buf = (char*)append_memory(packet_buf->packet_buf, packet_buf->memory_size, APPEND_SIZE);
		if (NULL == packet_buf->packet_buf) {
			printf("%s:append_memory %s\n", __FUNCTION__, strerror(errno));
			return -1;
		}
		packet_buf->memory_size += APPEND_SIZE;
	}

	memcpy(packet_buf->packet_buf + packet_buf->buf_length, socket_data->trans_buf + index_begin, 
		sizeof(char) * (index_end - index_begin));
	packet_buf->buf_length += index_end - index_begin;
	packet_buf->packet_buf[packet_buf->buf_length] = '\0';

	return 0;
}

int unpack_buf_to_command(struct packet_buf* packet_buf, struct command_packet* packet)
{
	int command_length = -1;
	int i = 0;
	//根据命令信息组装命名报文
	for (i = 0; i < COMMAND_NUM; i++) {
		command_length = command_infos[i].length;
		if (0 == compare_string(packet_buf->packet_buf, command_infos[i].command,command_length)) {
			if (1 == command_infos[i].space)
				packet->space = 32;
			else
				packet->space = '\0';
			break;
		}
	}
	memcpy(packet->command, packet_buf->packet_buf, sizeof(char) * command_length);
	if (32 == packet->space) {
		memcpy(packet->arguements, packet_buf->packet_buf + command_length + 1, sizeof(char) * packet_buf->buf_length - command_length - 3);
		packet->arguement_length = packet_buf->buf_length - command_length - 3;
		packet->arguements[packet->arguement_length] = '\0';
	}
	else {
		packet->arguement_length = 0;
		packet->arguements[0] = '\0';
	}
	strcpy(packet->split, "\r\n");
	
	return 0;
}

int unpack_buf_to_response(struct packet_buf* packet_buf, struct response_packet* packet)
{
	if ((packet_buf->buf_length - 4) >= packet->memory_size) {
		packet->message = (char*)append_memory(packet->message, packet->memory_size, APPEND_SIZE);
		if (NULL == packet->message) {
			printf("%s:append_memory %s\n", __FUNCTION__, strerror(errno));
			return -1;
		}
		packet->memory_size += APPEND_SIZE;
	}
	//开始组装包
	memcpy(packet->code, packet_buf->packet_buf,sizeof(char) * 3);
	packet->space = packet_buf->packet_buf[3];
	memcpy(packet->message, packet_buf->packet_buf + 4, sizeof(char) * (packet_buf->buf_length - 6));
	strcpy(packet->split,"\r\n");
	packet->message_length = packet_buf->buf_length - 6;
	packet->message[packet->message_length] = '\0';
	return 0;
}

int pack_command_to_buf(struct socket_data* socket_data, struct command_packet* packet)
{
	int length = (32 == packet->space) ? 1 : 0;
	int command_length = (strlen(packet->command) >= 4) ? 4 : strlen(packet->command);
	if ((packet->arguement_length + 5) >= (BUF_SIZE * 2)) {
		printf("%s:packet is to big\n", __FUNCTION__);
		return -1;
	}
	if (4 == command_length) {
		if (32 == packet->space) 
			sprintf(socket_data->trans_buf, "%c%c%c%c%c%s%c%c", packet->command[0], packet->command[1],
				packet->command[2], packet->command[3], packet->space, packet->arguements, packet->split[0], packet->split[1]);
		else 
			sprintf(socket_data->trans_buf, "%c%c%c%c%c%c", packet->command[0], packet->command[1],
				packet->command[2], packet->command[3],packet->split[0], packet->split[1]);
	}
	else {
		if(32 == packet->space)
			sprintf(socket_data->trans_buf, "%c%c%c%c%s%c%c", packet->command[0], packet->command[1],
				packet->command[2], packet->space, packet->arguements, packet->split[0], packet->split[1]);
		else
			sprintf(socket_data->trans_buf, "%c%c%c%c%c", packet->command[0], packet->command[1],
				packet->command[2], packet->split[0], packet->split[1]);
		
	}
	socket_data->trans_length = command_length + length + packet->arguement_length + 2;
	packet->command[3] = '\0';
	return 0;
}

int pack_response_to_buf(struct socket_data* socket_data, struct response_packet* packet)
{
	if ((packet->message_length + 6) >= (BUF_SIZE * 6)) {
		printf("%s:packet is to big\n", __FUNCTION__);
		return -1;
	}
	sprintf(socket_data->trans_buf, "%c%c%c%c%s%c%c", 
		packet->code[0], packet->code[1], packet->code[2], packet->space, packet->message, packet->split[0],packet->split[1]);
	socket_data->trans_length = packet->message_length + 6;
	
	return 0;
}

int process_user_command(struct link* link, struct session_info* info)
{
	int res = 0;
	char ip[16] = {};
	char port[4] = {};
	char user_name[40] = {};
	char* ip_index = NULL;
	char* port_index = NULL;
	struct command_packet* packet = link->server->packet;
	
	ip_index = strstr(packet->arguements, "ip:");
	port_index = strstr(packet->arguements, "port:");
	if ((ip_index != NULL) && (port_index != NULL)) {
		sscanf(packet->arguements, "%s ip:%s port:%s", user_name, ip, port);
		sprintf(packet->arguements, "%s", user_name);
		packet->arguement_length = strlen(packet->arguements);
		link->client->server_port = atoi(port);
		strcpy(link->client->server_ip, ip);
	}
	else if ((ip_index != NULL) && (port_index == NULL)) {
		sscanf(packet->arguements, "%s ip:%s", user_name, ip);
		sprintf(packet->arguements, "%s", user_name);
		packet->arguement_length = strlen(packet->arguements);
		strcpy(link->client->server_ip, ip);
	}
	else if ((ip_index == NULL) && (port_index == NULL)) {
		sscanf(packet->arguements, "%s", user_name);
	}
	strcpy(info->user_name, user_name);

	return 0;
}

int process_port_command(struct link* cmd_link, struct link* data_link)
{
	char server_ip[16] = {};
	int server_port = 0;
	int ip[4] = {};
	int port[2] = {};
	struct command_packet* packet = cmd_link->server->packet;

	sscanf(packet->arguements, "%d,%d,%d,%d,%d,%d", 
		&ip[0], &ip[1], &ip[2], &ip[3], &port[0], &port[1]);
	sprintf(server_ip, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
	server_port = port[0] * 256 + port[1];

	data_link->client->listen_port = server_port;
	strcpy(data_link->client->server_ip, server_ip);
	data_link->client->server_port = server_port;

	sprintf(packet->arguements, "%s,%d,%d", global_state.local_ip, port[0], port[1]);
	packet->arguement_length = strlen(packet->arguements);
	
	return 0;
}

int process_227_response(struct link* cmd_link, struct link* data_link)
{
	char server_ip[16] = {};
	int server_port = 0;
	int ip[4] = {};
	int port[2] = {};
	struct response_packet* packet = cmd_link->client->packet;
	sscanf(packet->message, "Entering Passive Mode (%d,%d,%d,%d,%d,%d).", 
		&ip[0], &ip[1], &ip[2], &ip[3], &port[0], &port[1]);
	sprintf(server_ip, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
	server_port = port[0] * 256 + port[1];

	data_link->client->listen_port = server_port;
	strcpy(data_link->client->server_ip, server_ip);
	data_link->client->server_port = server_port;

	sprintf(packet->message, "Entering Passive Mode (%s,%d,%d).",
		global_state.local_ip, port[0], port[1]);
	packet->message_length = strlen(packet->message);

	return 0;
}
