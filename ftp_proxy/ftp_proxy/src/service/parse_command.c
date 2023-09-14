#include "parse_command.h"
#include "session.h"
#include "utils.h"
#include "str.h"
#include <string.h>
#include <stdio.h>

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

int parse_command(struct socket_data* socket_data, struct command_packet* packet)
{
	int res = -1;
	struct packet_buf* packet_buf = global_state.packet_buf;
	//提取
	res = extract_command_packet(socket_data, packet_buf);
	if (0 == res)
		return res;
	//解包
	unpack_buf_to_command(packet_buf, packet);
	return res;
}

int extract_command_packet(struct socket_data* socket_data, struct packet_buf* packet_buf)
{
	int ret = 0;
	unsigned int i = 0;
	char* buf = socket_data->trans_buf;
	for (i = socket_data->index_begin; i < socket_data->trans_length - 1; i++) {
		if ((buf[i] == '\r') && (buf[i + 1] == '\n')) {
			//提取到一个完整的报文
			socket_data->index_end = i + 2;
			if (0 == packet_buf->is_complete) {
				append_packet_buf(socket_data, packet_buf);
				packet_buf->is_complete = 1;
			}
			else
				copy_packet_buf(socket_data, packet_buf);

			ret = 1;
			break;
		}

		if ((i + 2) >= socket_data->trans_length) {
			socket_data->index_end = i + 2;
			if (1 == packet_buf->is_complete) {
				copy_packet_buf(socket_data, packet_buf);
				packet_buf->is_complete = 0;
			}
			else
				append_packet_buf(socket_data, packet_buf);

			ret = 0;
			break;
		}
	}

	//位置更新
	socket_data->index_begin = socket_data->index_end;
	return ret;
}

static int unpack_buf_to_command(struct packet_buf* packet_buf, struct command_packet* packet)
{
	int command_length = -1;
	int i = 0;
	//根据命令信息组装命名报文
	for (i = 0; i < COMMAND_NUM; i++) {
		command_length = command_infos[i].length;
		if (0 == compare_string(packet_buf->string->data, command_infos[i].command, command_length)) {
			if (1 == command_infos[i].space)
				packet->space = 32;
			else
				packet->space = '\0';
			break;
		}
	}
	memcpy(packet->command, packet_buf->string->data, sizeof(char) * command_length);
	if (32 == packet->space) {
		memcpy(packet->arguements->data, packet_buf->string->data + command_length + 1, sizeof(char) * packet_buf->string->data_length - command_length - 3);
		packet->arguements->data_length = packet_buf->string->data_length - command_length - 3;
		packet->arguements->data[packet->arguements->data_length] = '\0';
	}
	else {
		packet->arguements->data_length = 0;
		packet->arguements->data[0] = '\0';
	}
	//strcpy(packet->split, "\r\n");
	return 0;
}
