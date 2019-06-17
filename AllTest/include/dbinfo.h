/** =============================================================================
** Copyright (c) 2004 WaveTop Information Corp. All rights reserved.
**
** The Report Table system
**
** =============================================================================
*/

#ifndef __BACKUP_DBINFO_H_
#define __BACKUP_DBINFO_H_ 1

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "wmain.h"
#include "wfunction.h"
#include "nspr.h"
#include "sqlite3.h"
#include "server_log.h"
#include "wstore.h"
#include "dbtable.h"
#include "storeexp.h"

#define BACKUP_IDX_NAME                         "index"
#define BACKUP_STORE_TABLE_INDEX_FILE			"schedule.db"
#define BACKUP_TAPE_TABLE_INDEX_FILE			"taperecord.db"
#define BACKUP_STORE_ARCHIVE_QUATO              20              
#define BACKUP_STORE_ARCHIVE_SIZE               (1.00 * 1024 * (1 << 30))    //��λ:GB
#define BACKUP_STORE_FULL_BACKUP                7346181
#define WAVETOP_ARCHIVE_BLOCK_SIZE              256 * 1024
#define WAVETOP_ARCHIVE_FILE_BLOCK              32 * 1024

#define TABLE_CREATE_DATATABLE "create table datainfo \
(source_ip char(32), \
dbsize char(64), \
modtype char(32), \
createdate char(64))"

#define TABLE_CREATE_SCHETABLE "create table scheduleinfo \
(schename char(256) PRIMARY KEY, \
tasktype char(16), \
sourcename char(64), \
username char(32), \
desc char(64), \
blocksize char(16), \
ctrlnum char(16))"

#define TABLE_CREATE_TASKTABLE "create table taskinfo \
(schename char(256), \
createtime char(32), \
backuptime char(32), \
filesize char(32), \
currentver int, \
backuptype char(16), \
idxname int, \
startpos char(260), \
endpos char(16), \
archiveno int, \
checkpoint char(64))"

#define TABLE_CREATE_ARCPOSTABLE "create table arcposinfo \
(arcno int, \
endpos char(64), \
type int, \
desc char(64))" 

#define TABLE_CREATE_ARCNOTABLE "create table arcnoinfo \
(schename char(256), \
ver int, \
arcno char(64), \
desc char(64))"

#define TABLE_INSERT_DATATABLE "insert into datainfo \
(source_ip, dbsize, modtype, createdate)"

#define TABLE_INSERT_ARCPOSTABLE "insert into arcposinfo \
(arcno, endpos, type, desc)"

#define TABLE_INSERT_ARCNOTABLE "insert into arcnoinfo \
(schename, ver, arcno, desc)"

#define TABLE_INSERT_SCHETABLE "insert into scheduleinfo \
(schename, tasktype, sourcename, username, desc, blocksize, ctrlnum)"

#define TABLE_INSERT_TASKTABLE "insert into taskinfo\
(schename, createtime, backuptime, filesize, currentver, backuptype, \
idxname, startpos, endpos, archiveno, checkpoint)"

/* The respond code */
#define SCHEDULE_TABLE_OK                         0       // �ɹ�
#define SCHEDULE_TABLE_INTERNAL_ERROR             1       // �ڲ�����
#define SCHEDULE_TABLE_OPEN_FILE_ERROR            2       // ���ļ�ʧ��
#define SCHEDULE_TABLE_WRITE_FILE_ERROR           3       // д�ļ�ʧ��
#define SCHEDULE_TABLE_FILENAME_TOO_LONG          4       // �ļ�������
#define SCHEDULE_TABLE_CREAT_FILE_ERROR           5       // �����ļ�ʧ��
#define SCHEDULE_TABLE_FILE_NOT_FOUND             6       // �ļ�û���ҵ�
#define SCHEDULE_TABLE_FILENAME_IS_EMPTY          7       // �ļ���Ϊ��
#define SCHEDULE_TABLE_CONVERT_WCHAR_ERROR        8       // ת��Ϊ���ַ�����
#define SCHEDULE_TABLE_CONVERT_CHAR_ERROR         9       // ת��Ϊխ�ַ�����
#define SCHEDULE_TABLE_LOAD_FILE_FAILURE          10      // װ��XML��XSL�ļ�����
#define SCHEDULE_TABLE_GET_NODE_FAILURE           11      // ���XML���Ľ�����
#define SCHEDULE_TABLE_NODE_NOT_FOUND             12      // û���ҵ�ָ�����
#define SCHEDULE_TABLE_TRANSFORM_FAILURE          13      // ��XMLת����HTM����
#define SCHEDULE_TABLE_NODE_IS_EMPTY              14      // û���ҵ�����ϵ�����
#define SCHEDULE_TABLE_FILL_NODE_ERROR            15      // ��д������
#define SCHEDULE_TABLE_SAVE_FILE_FAILURE          16      // �����ļ�ʧ��
#define SCHEDULE_TABLE_UNKNOWN_TYPE               17      // �޷�ʶ�������
#define SCHEDULE_TABLE_ALLOCATE_MEMORY_FAILURE    18      // �ڴ�����ʧ��
#define SCHEDULE_TABLE_DELETE_FILE_FAILURE        19      // ɾ���ļ�ʧ��
#define SCHEDULE_TABLE_LOST_FIELDS                20      // ȱ�ٱ����ֶ�

#define SCHEDULE_TABLE_NOT_EXIST                  -1      //������

/* Convert '\\' to '/' */
void ConvertToSlash(char *pStr);

char* ConvertToUTF(char *pszFile);

/* open database.if not exist,just create.
 * @[in]
 * pszTablePath - the schedule db path.
 * pDataBase - the handle of sqlite db.
 *
 * @[out]
 * return SQLITE_OK, if successful.
 * Otherwise, return error code.
 */
int DataBaseOpen(const char *pszTablePath, sqlite3 **pDataBased);

/* open Tape database.if not exist,just create.
 * @[in]
 * pszTablePath - the schedule db path.
 * pDataBase - the handle of sqlite db.
 *
 * @[out]
 * return SQLITE_OK, if successful.
 * Otherwise, return error code.
 */
int TapeDataBaseOpen(const char *pszTablePath, sqlite3 **pDataBased);
/* execute sql���.
 * @[in]
 * pDataBase - the handle of sqlite db.
 * pszSQL - sql ���.
 * pszErrMsg - return log.
 *
 * @[out]
 * return SQLITE_OK, if successful.
 * Otherwise, return error code.
 */
int DataBaseExec(sqlite3 *pDataBase, char *pszSQL, char **pszErrMsg);

int DataBaseGetSchTable(request_rec *pReq, sqlite3 *pDataBase, char *pszSQL, 
						char **pszErrMsg, SchFileHeaderSt **pSchNode);
int DataBaseGetTaskTable(request_rec *pReq, sqlite3 *pDataBase, 
						 char *pszSQL, char **pszErrMsg, SchTaskNodeSt **pSchNode);

int DataBaseGetMaxIdxAndArcId(request_rec *pReq, sqlite3 *pDataBase, 
						 char *pszSQL, char **pszErrMsg, unsigned long *nMaxId);

int DataBaseGetArcNo(request_rec *pReq, sqlite3 *pDataBase, char *pszSQL, 
                              char **pszErrMsg, char *pszArcNo);

int DataBaseGetArcPos(request_rec *pReq, sqlite3 *pDataBase, char *pszSQL, 
                              char **pszErrMsg, SchArcPosSt **pPosList);
/* close database.
 * @[in]
 * pDataBase - the handle of sqlite db.
 *
 * @[out]
 * return SQLITE_OK, if successful.
 * Otherwise, return error code.
 */
int DataBaseClose(sqlite3 *pDataBase);
void DataBaseFree(char *pszMessage);

/************************************************************************/
/* add / search / delete sch node function                              */
/************************************************************************/
int BkSchAddNode(request_rec *pReq, StoreHandle *pStore, SchFileHeaderSt *pSchList, 
                            SchTaskNodeSt *pTaskList);
int BkArcAddNode(request_rec *pReq, StoreHandle *pStore, SchArcPosSt *pPosList, 
                            SchArcNoSt *pNoList);
int BkDataAddNode(request_rec *pReq, StoreHandle *pStore, SchDataSt *pNoList);

int BkWriteDataInfo(request_rec *pReq, char *pszPath, char *pszModeType, unsigned long nRecvTime);

/* delete table taskinfo file */
int BkSchDeleteNode(request_rec *pReq, StoreHandle *pStore, char *pszScheName);

/* delete table scheduleinfo file */
int BkTaskDeleteNode(request_rec *pReq, StoreHandle *pStore, char *pszScheName, 
                        unsigned long nMinVer, unsigned long nMaxVer);
int BkTaskDeletebyVer(request_rec *pReq, StoreHandle *pStore, char *pszScheName, unsigned long nVer);
//

int BkDeleteArcNoNode(request_rec *pReq, StoreHandle *pStore, char *pszScheName, 
                        unsigned long nVer);

int BkDeleteArcPosNode(request_rec *pReq, StoreHandle *pStore, unsigned long nArcNo);

/* search taskinfo table file node */
int BkTaskSearchNode(request_rec *pReq, StoreHandle *pStore, char *pszTaskName, 
                                    SchTaskNodeSt **pTaskNode);

/* max backuptime search */
int BkTaskMaxTimeNode(request_rec *pReq, StoreHandle *pStore, char *pszTaskName,
                                SchTaskNodeSt **pTaskNode);

#endif