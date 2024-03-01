#include "transcoder.h"
#include "debug_msg.h"
#include "base64.h"
#include "cJSON.h"
#include "sqlite.h"
#include "bitmap.h"
#include "decoder.h"
#include "encoder.h"
#include "config.h"
#include "stream.h"
#include "algjpeg.h"
#include "config.h"
#include "ini.h"

#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <sys/time.h>

#define filename "../../conf/cfg.ini"
static struct xconfig config;

static void free_garg(struct garg *g) 
{
    if (g->input) {
        free(g->input);
        g->input = NULL;
    }
    if (g->output) {
        free(g->output);
        g->output = NULL;
    }
}

static void usage(const char *path) {
    const char *p = strrchr(path, '/');
    if (p) {
        p++;
    } else {
        p = "health_check";
    }

    printf("Usage %s [OPTIONS].\n", p);
    printf("Options:\n");
    printf("  -f: input file type.\n");
    printf("  -t: debug file path.\n");
    printf("  -d <level>: Special debug level, 0 to 7.\n");
    printf("  -i <input_file>: open file path.\n");
    printf("  -o <output_file>: output file path.\n");
    printf("  -b <base64>: serializational data base64.\n");
}

static int get_input_path(struct garg *g,char *input)
{
    cJSON  *json = NULL;
    cJSON  *file_type = NULL;
    cJSON *inputpath = NULL;
    cJSON *outputpath = NULL;
    cJSON *offset = NULL;
    char *originalString = input;
    originalString = base64_decode(originalString);
    if (originalString == NULL) {
        return -1;
    }

    json = cJSON_Parse(originalString);
    if (json == NULL) {
        free(originalString);
        return -1;
    }  
   
   //获取json路径
    inputpath = cJSON_GetObjectItemCaseSensitive(json,"input");
    if (cJSON_IsString(inputpath) && (inputpath->valuestring != NULL)) {
        if (g->input != NULL) {
            free(g->input); // 释放先前分配的内存
        }
        g->input = strdup(inputpath->valuestring);
    }

    //获取输出文件路径
    outputpath = cJSON_GetObjectItemCaseSensitive(json,"output");
    if (cJSON_IsString(outputpath) && (outputpath->valuestring != NULL)) {
        if (g->output != NULL) {
            free(g->output); // 释放先前分配的内存
        }
        g->output = strdup(outputpath->valuestring);
    }

    //获取文件类型
    file_type = cJSON_GetObjectItemCaseSensitive(json,"file_type");
    if (cJSON_IsString(file_type) && (file_type->valuestring != NULL)) {
        g->file_type = atoi(file_type->valuestring);
    }
    
    //获取偏移量
    offset = cJSON_GetObjectItemCaseSensitive(json,"offset");
    if (cJSON_IsString(offset) && (offset->valuestring != NULL)) {
        g->offset = atoi(offset->valuestring);
    }

    if(NULL == g->output){
        g->output = strdup("./output.mp4");
    }

    free(originalString);
    cJSON_Delete(json);
    return 0;
};

//解析函数
static int argv_parse(struct garg *g, int argc, char **argv) {
    int opt = 0;
    int i_flag = 0;
    int b_flag = 0;
    int h_glag = 0;
    static int debug = 1;
    g->file_type = 0;
    g->offset = 0;
    g->input = NULL;
    g->output = NULL;

    memset(g, 0, sizeof(struct garg));
    while ((opt = getopt(argc, argv, "f:d:ht:i:o:b:")) != -1) {
        switch (opt) {
            case 'f':
                //debug = 0;
                g->file_type = atoi(optarg);
                i_flag = 1;
                DB_INFO("file type:%d\n",g->file_type);
                break;
            case 'h':
                h_glag = 1;
                usage(argv[0]);
                break;
            case 'd':
                set_debug_level(atoi(optarg));
                break;
            case 't':
                if (-1 == open_debug_file(optarg)) {
                    DB_WAR("Open debug file:%s error", optarg);
                } else {
                    DB_INFO("Log to file:%s", optarg);
                }
                break;
            case 'i':
                i_flag = 1;
                if (g->input != NULL) {
                    free(g->input); 
                }
                g->input = strdup(optarg);
                DB_INFO("input path:%s\n",g->input);
                break;
            case 'o':
                i_flag = 1;
                if (g->output != NULL) {
                    free(g->output); 
                }
                g->output = strdup(optarg);
                break;
            case 'b':
                b_flag = 1;
                get_input_path(g,optarg);
                break;
            default:
                printf("invalid option:[%c]\n", opt);
                usage(argv[0]);
                return -1;
        }
    }
    if(i_flag && b_flag){
        printf("invalid option:please enter the '-b string' or '-i string -o string'\n");
        usage(argv[0]);
        return -1;
    }

    if((h_glag) && (i_flag || b_flag)){
        printf("invalid oprion\n");
        usage(argv[0]);
        return -1;
    }

    if(NULL == g->input){
        printf("invalid option:please enter the '-b string' or '-i string -o /string'\n");
        return -1;
    }

    if(NULL == g->output){
        g->output = strdup("./output.mp4");
    }else{
        size_t len = strlen(g->output);
        if (len >= 4 && strcmp(g->output + len - 4, ".mp4") != 0) {
            printf("invalid output file: %s, must be xxx.mp4\n", g->output);
            return -1; 
        }
    }
    

    return 0;
}
/*

{
    "input": "./share_file",
    "output": "./output.mp4",
    "file_type": "1",
    "offset": "2239"
}
base64 ewogICAgImlucHV0IjogIi4vc2hhcmVfZmlsZSIsCiAgICAib3V0cHV0IjogIi4vb3V0cHV0Lm1wNCIsCiAgICAiZmlsZV90eXBlIjogIjEiLAogICAgIm9mZnNldCI6ICIyMjM5Igp9

*/
static int pdu2rgba(STREAM* pdu_data,struct rgb_data* rgb_data)
{
    //读取rect
    stream_read_uint16(pdu_data, rgb_data->rect.x);
    stream_read_uint16(pdu_data, rgb_data->rect.y);
    stream_read_uint16(pdu_data, rgb_data->rect.width);
    stream_read_uint16(pdu_data, rgb_data->rect.height);

    //读取jpg的长度
    stream_read_uint32(pdu_data,rgb_data->jpeg_length);
    
    //将jpg数据解码为rgb数据
    rgb_data->data_length = jpeg_decode(&rgb_data->data,&rgb_data->memory_length,pdu_data->p,pdu_data->size);
    if(rgb_data->data_length < 0){
        DB_ERR("jpeg_decode failed");
        return -1;
    }
    stream_seek(pdu_data,rgb_data->jpeg_length);

    return 0;
}

static int rgba2bitmap(STREAM* bitmap_data,struct rgb_data* rgb_data,int width)
{
    void* ptr = NULL;            
    if(bitmap_data == NULL || rgb_data == NULL){
        DB_ERR("bitmap_data or rgb_data is null");
        return -1;
    }
    ptr = rgb_data->data;

    //1.将位图指针移动到指定位置
    bitmap_data->p = bitmap_data->data;
    stream_seek(bitmap_data,(3 * (rgb_data->rect.x + (rgb_data->rect.y * width))));

    //循环的将rgb_data数据写入到位图中
    for(int i = 0;i< rgb_data->rect.height;i++){
        stream_write(bitmap_data,ptr,rgb_data->rect.width * 3);
        ptr += rgb_data->rect.width * 3;
        stream_seek(bitmap_data,((3 * width)) - (3 * rgb_data->rect.width));
    }

    return 0;
}

static int read_pdu_data_sigle(struct transcode* transcode,int type)
{
   struct pdu_data* pdu_data = transcode->decoder->pdu_data;
    sqlite3_stmt* stmt = (0 == type) ? transcode->decoder->stmt : transcode->decoder->keyframe_stmt;

    pdu_data->pdu_id = sqlite3_column_int(stmt, 0);
    pdu_data->pdu_time = (uint64_t)sqlite3_column_int64(stmt, 1);
    pdu_data->pdu_type = sqlite3_column_int(stmt, 2);
    pdu_data->pdu_length = sqlite3_column_int(stmt, 4);

    if (pdu_data->data == NULL || pdu_data->memory_length < pdu_data->pdu_length) {
        void* new_data = realloc(pdu_data->data, pdu_data->pdu_length);
        if (new_data == NULL) {
            // 处理内存分配失败
            return -1;
        }
        pdu_data->data = new_data;
        pdu_data->memory_length = pdu_data->pdu_length;
    }

    int blobSize = sqlite3_column_bytes(stmt, 5);
    memcpy(pdu_data->data, sqlite3_column_blob(stmt, 5), blobSize);

    return 0;
}

static int read_pdu_data_share(struct transcode* transcode)
{
    int offset = 0;
    struct share_file* share_file = transcode->decoder->share_file;
    struct pdu_data* pdu_data = transcode->decoder->pdu_data;
    sqlite3_stmt* stmt = transcode->decoder->stmt;

    pdu_data->pdu_id = sqlite3_column_int(stmt, 0);
    pdu_data->pdu_time = (uint64_t)sqlite3_column_int64(stmt, 1);
    pdu_data->pdu_length = sqlite3_column_int(stmt, 2);
    pdu_data->pdu_type = sqlite3_column_int(stmt, 3);
    
    offset = sqlite3_column_int(stmt, 4);

    if (pdu_data->data == NULL || pdu_data->memory_length < pdu_data->pdu_length) {
        void* new_data = realloc(pdu_data->data, pdu_data->pdu_length);
        if (new_data == NULL) {
            return -1;
        }
        pdu_data->data = new_data;
        pdu_data->memory_length = pdu_data->pdu_length;
    }

    //判断打开文件
    if(share_file->chunk != pdu_data->pdu_type){
        share_file->chunk = pdu_data->pdu_type;
        if(share_file->fp != NULL){
            fclose(share_file->fp);
        }

        //拼接文件路径
        char path[256] = {0};
        sprintf(path,"%s/%d",share_file->folder,pdu_data->pdu_type);
        share_file->fp = fopen(path,"rb");
        if(share_file->fp == NULL){
            DB_ERR("fopen %s error",path);
            return -1;
        }
    }
    
    //将文件指针偏移
    fseek(share_file->fp,offset,SEEK_SET);

    //读取数据
    fread(pdu_data->data,1,pdu_data->pdu_length,share_file->fp);

    return 0;
}

static int update_bitmap(struct transcode* transcode,STREAM* bitmap_data,int* read_index)
{
    STREAM pdu_data;
    int rc = -1;

    pdu_data.data = transcode->decoder->pdu_data->data;
    pdu_data.p = transcode->decoder->pdu_data->data;
    pdu_data.size = transcode->decoder->pdu_data->pdu_length;
    stream_seek_uint32((&pdu_data));

    while(*read_index < transcode->decoder->pdu_data->pdu_length){
            rc = pdu2rgba(&pdu_data,transcode->decoder->rgb_data);
            if(rc < 0){
                DB_ERR("pdu2rgba error");
                return -1;
            }
            
            *read_index = *read_index + 12 + transcode->decoder->rgb_data->jpeg_length;
            rc = rgba2bitmap(bitmap_data,transcode->decoder->rgb_data,transcode->decoder->session_data->width);
            if(rc != 0){
                DB_ERR("rgba2bitmap error");
                return -1;
            }
        }

    return 0;
}

struct transcode* transcoder_open(struct garg* garg,struct xconfig* config)
{
    const char *suffix = "/db";
    int rc = 0;

    struct transcode* transcode = (struct transcode*)calloc(1,sizeof(struct transcode));
    if(transcode == NULL) {
        return NULL;
    }

    //打开解码器
    transcode->decoder = decoder_open();
    if(transcode->decoder == NULL) {
        free(transcode);
        DB_ERR("decoder_open error");
        return NULL;
    }

    //配置分片文件结构
    if(1 == garg->file_type){
        transcode->decoder->share_file = get_share_file(garg->input);
        garg->input = realloc(garg->input,strlen(garg->input) + strlen(suffix) + 1);
        strcat(garg->input,suffix);
    }

    //在获取数据库句柄
    transcode->db = NULL;
    rc = DB_opener(&transcode->db,garg->input);
    if(-1 == rc) {
        free(transcode);
        DB_ERR("DB_opener error");
        return NULL;
    }
    
    //执行V表获取分辨率等信息
    transcode->decoder->session_data = (struct session_data*)calloc(1,sizeof(struct session_data));
    if(transcode->decoder->session_data == NULL) {
        free(transcode);
        DB_ERR("calloc error");
        return NULL;
    }
    
    rc = get_info(&transcode->db,transcode->decoder->session_data);
    if(rc <  0) {
        DB_ERR("get_info error");
        free_session_data(transcode->decoder->session_data);
        free(transcode);
        return NULL;
    }
    
    if(transcode->decoder->session_data->width % 4){
        transcode->decoder->session_data->width += 4 - (transcode->decoder->session_data->width % 4);
    }
    if(transcode->decoder->session_data->height % 2){
        transcode->decoder->session_data->height += 2 - (transcode->decoder->session_data->height % 2);
    }

    //获取解码器游标
    transcode->decoder->keyframe_stmt = get_keyframe(transcode->db,garg->offset);
    transcode->decoder->stmt = get_pdu(transcode->db,garg->file_type,garg->offset);
    if((transcode->decoder->stmt == NULL) && (transcode->decoder->keyframe_stmt == NULL)) {
        DB_ERR("get stmt error");
        free_session_data(transcode->decoder->session_data);
        free(transcode);
        return NULL;
    }

    //打开位图
    transcode->bitmap = bitmap_init(transcode->decoder->session_data->width * transcode->decoder->session_data->height * 3);
    if(transcode->bitmap == NULL) {
        DB_ERR("bitmap_init error");
        free_session_data(transcode->decoder->session_data);
        free(transcode);
        return NULL;
    }
    transcode->bitmap->resolution->bit_per_pixel = 3;
    transcode->bitmap->resolution->height = transcode->decoder->session_data->height;
    transcode->bitmap->resolution->width = transcode->decoder->session_data->width;
    transcode->offset = garg->offset;

    //打开编码器
    transcode->encoder = encoder_open(transcode->decoder->session_data->width,transcode->decoder->session_data->height,config->fps,config->ffmpeg_threads,9,garg->output);
    if (transcode->encoder == NULL){
        DB_ERR("encoder_open error");
        decoder_close(transcode->decoder);
        bitmap_free(transcode->bitmap);
        free(transcode);
        return NULL;
    }
    
    return transcode;
}

int transcoder_transcode(struct transcode* transcode)
{
    int rc = 0;        
    int read_index = 0; 
    STREAM bitmap_data;

    bitmap_data.size = transcode->bitmap->bufflen;
    bitmap_data.data = transcode->bitmap->buffer;
    bitmap_data.p    = transcode->bitmap->buffer;
    
    //是否更新关键帧
    if((transcode->offset != 0) && (sqlite3_step(transcode->decoder->keyframe_stmt) == SQLITE_ROW))
    {
        read_index = 4;
        rc = read_pdu_data_sigle(transcode,1);
        if(rc != 0) {
            DB_ERR("read_pdu_data_sigle type 1 error");
            return -1;
        }
        
        rc = update_bitmap(transcode,&bitmap_data,&read_index);
        if(rc != 0) {
            DB_ERR("update_bitmap type 1 error");
            return -1;
        }
    }

    while ((rc = sqlite3_step(transcode->decoder->stmt)) == SQLITE_ROW){
        read_index = 4;
        bitmap_data.p = bitmap_data.data;

        //读取数据
        rc = (NULL == transcode->decoder->share_file) ? 
            read_pdu_data_sigle(transcode, 0) : read_pdu_data_share(transcode);
        if (rc != 0) {
            DB_ERR("read_pdu_data_share error");
            return -1;
        }

        //将读取到的数据更新到pdu数据流中
        rc = update_bitmap(transcode,&bitmap_data,&read_index);
        if(rc != 0) {
            DB_ERR("update_bitmap error");
            return -1;
        }

        //将位图数据传递给编码器
        transcode->bitmap->time = transcode->decoder->pdu_data->pdu_time;
        rc = encoder_encode(transcode->encoder,transcode->bitmap);
        if(-1 == rc) {
            DB_ERR("encoder_encode error");
            return -1;
        }else if (1 == rc){
            return transcode->decoder->pdu_data->pdu_id;
        }
    }

    return 0;
}

int transcoder_close(struct transcode* transcode)
{
    //释放资源
    if (transcode == NULL) {
        return -1;
    }

    // Close decoder
    if (transcode->decoder != NULL) {
        decoder_close(transcode->decoder);
        transcode->decoder = NULL;
    }

    // Close encoder
    if (transcode->encoder != NULL) {
        encoder_close(transcode->encoder,(RELEASE_LEVEL)6);
        transcode->encoder = NULL;
    }

    // Free bitmap
    if (transcode->bitmap != NULL) {
        bitmap_free(transcode->bitmap);
        transcode->bitmap = NULL;
    }

    // Close database
    if (transcode->db != NULL) {
        sqlite3_close(transcode->db);
        transcode->db = NULL;
    }

    // Free the transcode structure
    free(transcode);
    return 0;
}

int main(int argc,char* argv[])
{
    struct transcode* transcoder = NULL;
    struct ini_t* cfg_ini = NULL;
    int ret = -1;

    //解析参数
    if(-1 == argv_parse(&G, argc, argv)){
        return -1;
    }

    //读取配置文件
    if ((cfg_ini = ini_load(filename)) == NULL) {
        DB_INFO("ini_load failed");
        return -1;
	}
    g_rgbtrans_init(cfg_ini, &config);
    
    //打开transcoder
    transcoder = transcoder_open(&G, &config);
    if(transcoder == NULL) {
       DB_ERR("transcoder_open error");
       return -1;
    }

    //开始编码
    ret = transcoder_transcode(transcoder);
    if(ret == -1) {
        DB_ERR("transcoder_transcode error");
        return ret;
    }

    //关闭transcoder
    transcoder_close(transcoder);

    //释放资源
    free_garg(&G);

    return ret;
} 
