#ifndef __SQLITE_H__
#define __SQLITE_H__

#include <sqlite3.h>
#include "decoder.h"

#define SQL_BUFFER_SIZE 256
#define ITEMS_PER_ROW   1

//获取信息
int get_info(sqlite3** dbFile,struct session_data* data);

//获取数据
sqlite3_stmt* get_pdu(sqlite3* dbFile,int type,int offset);

//获取关键帧数据
sqlite3_stmt* get_keyframe(sqlite3* dbFile,int offset);

//用于执行v表的回调函数
int get_info_callback(void* data,int argc,char* argv[],char* col_name);

//用于执行d表的回调函数
int get_pdu_callback(void* data,int argc,char* argv[],char* col_name);

//打开数据库文件
int DB_opener(sqlite3 **dbFile, const char *dbName);

//执行sql语句
int DB_sqlexer(sqlite3 *db, const char *sql, int (*callback)(void *, int, char **, char **), void *data, char **errMsg);

//关闭数据库文件
int DB_closer(sqlite3 *dbFile);

#endif