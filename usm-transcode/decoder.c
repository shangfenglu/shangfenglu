#include "decoder.h"
#include "sqlite.h"
#include "debug_msg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct share_file* get_share_file(const char* filename)
{
    struct share_file* share_file = (struct share_file*)calloc(1,sizeof(struct share_file));
    if(share_file == NULL){
        DB_ERR("calloc share_file error");
        return NULL;
    }

    share_file->folder = strdup(filename);
    share_file->chunk = 0;
    share_file->fp = NULL;

    return share_file;
}

struct session_data* get_session_data()
{
    struct session_data* session_data = (struct session_data*)calloc(1,sizeof(struct session_data));
    if(session_data == NULL){
        DB_ERR("calloc session_data error");
        return NULL;
    }
    session_data->width = 0;
    session_data->height = 0;
    session_data->bits = 0;

    return session_data;
}

int free_session_data(struct session_data* session_data)
{
    int rc = -1;
    if(session_data){
        free(session_data);
        session_data = NULL;
        rc = 0;
    }

    return rc;
}

struct pdu_data* get_pdu_data()
{
    struct pdu_data* pdu_data = (struct pdu_data*)calloc(1,sizeof(struct pdu_data));
    if(pdu_data == NULL){
        DB_ERR("calloc pdu_data error");
        return NULL;
    }
    pdu_data->memory_length = 0;
    pdu_data->data = NULL;

    return pdu_data;
}

int free_pdu_data(struct pdu_data* pdu_data)
{
    int rc = -1;
    if(pdu_data){
        free(pdu_data);
        rc = 0;
    }
    return rc;
}

struct rgb_data* get_rgb_data()
{
	struct rgb_data* rgb_data = (struct rgb_data*)calloc(1,sizeof(struct rgb_data));
	if (!rgb_data) {
        DB_ERR("calloc rgb_data error");
		return NULL;
	}
    rgb_data->jpeg_length = 0;
	rgb_data->memory_length = 0;
	rgb_data->data = NULL;

	return rgb_data;
}

int free_rgb_data(struct rgb_data* rgb_data)
{
	if(rgb_data){
		free(rgb_data);
	}

	return 0;
}

struct decoder* decoder_open()
{
    struct decoder* decoder = (struct decoder*)calloc(1,sizeof(struct decoder));
    if(decoder == NULL){
        DB_ERR("calloc decoder error");
        return NULL;
    }
    
    decoder->stmt = NULL;
    decoder->share_file = NULL;

    //开辟pdu数据结构体的空间
    decoder->pdu_data = get_pdu_data();

    //开辟rgb数据结构体的空间
    decoder->rgb_data = get_rgb_data();
    if((decoder->rgb_data == NULL) || (decoder->pdu_data == NULL)){
        free_rgb_data(decoder->rgb_data);
        free_pdu_data(decoder->pdu_data);
        free(decoder);
        return NULL;
    }

    return decoder;
}

int decoder_close(struct decoder* decoder)
{
    if(decoder == NULL){
        return -1;
    }
    if(decoder->stmt){
        sqlite3_finalize(decoder->stmt);
    }
    free_session_data(decoder->session_data);
    free_pdu_data(decoder->pdu_data);
    free_rgb_data(decoder->rgb_data);
    free(decoder);

    return 0;
}