#include "parse_response.h"
#include "session.h"
#include "str.h"
#include "utils.h"
#include <string.h>
#include <stdio.h>


//解析响文
int parse_resonse(struct socket_data* data, struct response_packet* packet)
{
	int res = -1;
	struct packet_buf* packet_buf = global_state.packet_buf;
	
	//提取
	res = extract_response_packet(data, packet_buf);
	if (0 == res)
		return res;
	//解析
	unpack_buf_to_response(packet_buf, packet);
	return res;
}

int extract_response_packet(struct socket_data* socket_data, struct packet_buf* packet_buf)
{
	int ret = 0;
	int i = 0;
	char* buf = socket_data->trans_buf;

	for (i = socket_data->index_begin; i < socket_data->trans_length - 1; i++) {
		if ((buf[i] == '\r') && (buf[i + 1] == '\n')) {
			//提取到一个完整的报文
			socket_data->index_end = i + 2;
			if (0 == packet_buf->is_complete) {
				append_packet_buf(socket_data, packet_buf);
				packet_buf->is_complete = 1;

				//判断是否结束
				if (0 == packet_buf->is_signal) {
					ret = 0;
					socket_data->index_begin = socket_data->index_end;
					continue;
				}
				else
					ret = 1;
				break;
			}
			else {
				//判断是否是单个报文
				if (0 == packet_buf->is_signal) {
					append_packet_buf(socket_data, packet_buf);
					if ((0 == compare_string(packet_buf->string->data, socket_data->trans_buf + socket_data->index_begin, 3)) &&
						(' ' == get_char(socket_data->trans_buf + socket_data->index_begin, 3))) {
						packet_buf->is_signal = 1;
						ret = 1;
						break;
					}
					else {
						ret = 0;
						socket_data->index_begin = socket_data->index_end;
						continue;
					}
				}
				else {
					copy_packet_buf(socket_data, packet_buf);
					if ('-' == get_char(socket_data->trans_buf + socket_data->index_begin, 3)) {
						packet_buf->is_signal = 0;
						ret = 0;
						socket_data->index_begin = socket_data->index_end;
						continue;
					}
					else
						ret = 1;
					break;
				}
				break;
			}
		}

		if ((i + 2) >= socket_data->trans_length) {
			socket_data->index_end = i + 2;
			if (1 == packet_buf->is_complete) {
				copy_packet_buf(socket_data, packet_buf);
				packet_buf->is_complete = 0;
				//判断当前半截是否是响文结束
				if (0 == packet_buf->is_signal) {
					if ((0 == compare_string(packet_buf->string->data, socket_data->trans_buf + socket_data->index_begin, 3)) &&
						(' ' == get_char(socket_data->trans_buf + socket_data->index_begin, 3))) {
						packet_buf->is_signal = 1;
					}
				}
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

int unpack_buf_to_response(struct packet_buf* packet_buf, struct response_packet* packet)
{
	if ((packet_buf->string->data_length - 4) >= packet->message->alloc_length)
		str_append_alloc(packet->message, APPEND_SIZE);

	//开始组装包
	memcpy(packet->code, packet_buf->string->data, sizeof(char) * 3);
	packet->space = packet_buf->string->data[3];
	memcpy(packet->message->data, packet_buf->string->data + 4, sizeof(char) * (packet_buf->string->data_length - 6));
	//strcpy(packet->split, "\r\n");
	packet->message->data_length = packet_buf->string->data_length - 6;
	packet->message->data[packet->message->data_length] = '\0';
	return 0;
}

