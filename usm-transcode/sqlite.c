#include "sqlite.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int get_info(sqlite3** dbFile,struct session_data* data)
{
    char sql[SQL_BUFFER_SIZE];
    memset(sql,0,SQL_BUFFER_SIZE);
    sprintf(sql,"select * from v where name = 'Display';");
    return DB_sqlexer(*dbFile,sql,get_info_callback,data,NULL);
}

sqlite3_stmt* get_pdu(sqlite3* dbFile,int type,int offset)
{
    int rc;
    sqlite3_stmt* stmt = NULL;
    char sql[SQL_BUFFER_SIZE];
    memset(sql,0,SQL_BUFFER_SIZE);
    if(0 == type){
        sprintf(sql,"select * from d where pdutyp = 1025;");
    }else if(1 == type){
        sprintf(sql,"select rowid,pdutst,pdulen,chunk,offset from e where pdutyp = 1025 and rowid > %d;",offset);
    }
    rc = sqlite3_prepare_v2(dbFile, sql, -1, &stmt, NULL);
    if(rc != SQLITE_OK){
        return NULL;
    }

    return stmt;
}

sqlite3_stmt* get_keyframe(sqlite3* dbFile,int offset)
{
    int rc = 0;
    sqlite3_stmt* stmt = NULL;
    char sql[SQL_BUFFER_SIZE];
    memset(sql,0,SQL_BUFFER_SIZE);
    sprintf(sql,"select * from i where pdutyp = 1026 and pduseq <= %d order by pduseq DESC limit 1;",offset);
    rc = sqlite3_prepare_v2(dbFile, sql, -1, &stmt, NULL);
    if(rc != SQLITE_OK){
        return NULL;
    }
    
    return stmt;
}

int get_info_callback(void* data,int argc,char* argv[],char* col_name)
{
    struct session_data* v_data = (struct session_data*)data;
    //分辨率格式为："width x height x bitsPerPixel"
    if(!strcmp(argv[1],"Display")){
        char* p = argv[2];
        char buf[8] = {0};

        //获取宽度
        for(int i = 0;*p!= 'x';i++,p++){
            buf[i] = *p;
        }
        v_data->width = atoi(buf);
        memset(buf,0,8);
        p++;

        //获取高度
        for(int i = 0;*p!= 'x';i++,p++){
            buf[i] = *p;
        }
        v_data->height = atoi(buf);
        memset(buf,0,8);
        p++;

        //获取颜色深度
        for (int i = 0; *p != '\0'; i ++, p ++)
        {
            buf[i] = *p;
        }
        v_data->bits = atoi(buf);
        v_data->bits = 8;
    }
    return 0;
}

int get_pdu_callback(void* data,int argc,char* argv[],char* col_name)
{
    struct pdu_data** pdu_data = (struct pdu_data**)data;
    (*pdu_data)->pdu_id = atoi(argv[0]);
    (*pdu_data)->pdu_time = atoi(argv[1]);
    (*pdu_data)->pdu_type = atoi(argv[2]);
    (*pdu_data)->pdu_length = atoi(argv[4]);
    if((*pdu_data)->memory_length == 0){
        //开辟空间
        (*pdu_data)->data = calloc(1,(*pdu_data)->pdu_length);
        if(!(*pdu_data)->data){
            return -1;
        }
    }else if((*pdu_data)->memory_length < (*pdu_data)->pdu_length){
        //追加空间
        (*pdu_data)->data = realloc((*pdu_data)->data,(*pdu_data)->pdu_length);
        if(!(*pdu_data)->data){
            return -1;
        }
    }

    //拷贝数据
    memcpy((*pdu_data)->data,argv[5],(*pdu_data)->pdu_length);
    return 0;
}

int DB_opener(sqlite3 **dbFile, const char *dbName)
{
    int rc;
    rc = sqlite3_open(dbName, dbFile);

    if (rc != SQLITE_OK)
    {
        return -1;
    }
    return 0;
}

int DB_sqlexer(sqlite3 *db, const char *sql, int (*callback)(void *, int, char **, char **), void *data, char **errMsg)
{
    int rc;
    rc = sqlite3_exec(db, sql, callback, data, errMsg);
    if (rc != SQLITE_OK)
    {
        sqlite3_free(*errMsg);
        return -1;
    }
    return 0;
}

int DB_closer(sqlite3 *dbFile)
{
    sqlite3_close(dbFile);
    return 0;
}

