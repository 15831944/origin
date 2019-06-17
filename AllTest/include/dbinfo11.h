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

#define BACKUP_IDX_NAME                         "index"
#define BACKUP_STORE_TABLE_INDEX_FILE			"schedule.db"
#define BACKUP_STORE_ARCHIVE_QUATO              20              
#define BACKUP_STORE_ARCHIVE_SIZE               1<<30               //单位:GB
#define BACKUP_STORE_FULL_BACKUP                7346181

#define TABLE_CREATE_SCHETABLE "create table scheduleinfo \
(schename char(256) PRIMARY KEY, \
tasktype char(16), \
sourcename char(64), \
username char(32), \
desc char(64))"

#define TABLE_CREATE_TASKTABLE "create table taskinfo \
(schename char(256), \
createtime char(32), \
backuptime char(32), \
filesize char(32), \
currentver char(16), \
backuptype char(16), \
idxname char(64), \
startpos char(260), \
endpos char(16), \
archiveno char(260))"

#define TABLE_INSERT_SCHETABLE "insert into scheduleinfo \
(schename, tasktype, sourcename, username, desc)"

#define TABLE_INSERT_TASKTABLE "insert into taskinfo\
(schename, createtime, backuptime, filesize, currentver, backuptype, \
idxname, startpos, endpos, archiveno)"

/* The respond code */
#define SCHEDULE_TABLE_OK                         0       // 成功
#define SCHEDULE_TABLE_INTERNAL_ERROR             1       // 内部错误
#define SCHEDULE_TABLE_OPEN_FILE_ERROR            2       // 打开文件失败
#define SCHEDULE_TABLE_WRITE_FILE_ERROR           3       // 写文件失败
#define SCHEDULE_TABLE_FILENAME_TOO_LONG          4       // 文件名过长
#define SCHEDULE_TABLE_CREAT_FILE_ERROR           5       // 创建文件失败
#define SCHEDULE_TABLE_FILE_NOT_FOUND             6       // 文件没有找到
#define SCHEDULE_TABLE_FILENAME_IS_EMPTY          7       // 文件名为空
#define SCHEDULE_TABLE_CONVERT_WCHAR_ERROR        8       // 转换为宽字符出错
#define SCHEDULE_TABLE_CONVERT_CHAR_ERROR         9       // 转换为窄字符出错
#define SCHEDULE_TABLE_LOAD_FILE_FAILURE          10      // 装载XML或XSL文件出错
#define SCHEDULE_TABLE_GET_NODE_FAILURE           11      // 获得XML树的结点出错
#define SCHEDULE_TABLE_NODE_NOT_FOUND             12      // 没有找到指定结点
#define SCHEDULE_TABLE_TRANSFORM_FAILURE          13      // 从XML转换成HTM出错
#define SCHEDULE_TABLE_NODE_IS_EMPTY              14      // 没有找到结点上的内容
#define SCHEDULE_TABLE_FILL_NODE_ERROR            15      // 填写结点出错
#define SCHEDULE_TABLE_SAVE_FILE_FAILURE          16      // 保存文件失败
#define SCHEDULE_TABLE_UNKNOWN_TYPE               17      // 无法识别的类型
#define SCHEDULE_TABLE_ALLOCATE_MEMORY_FAILURE    18      // 内存申请失败
#define SCHEDULE_TABLE_DELETE_FILE_FAILURE        19      // 删除文件失败
#define SCHEDULE_TABLE_LOST_FIELDS                20      // 缺少必须字段

#define SCHEDULE_TABLE_NOT_EXIST                  -1      //表不存在

typedef struct _BK_INDEX_NODE_ST_ {
    char *pszFilename;
    unsigned long nFlags;    /* DIR or FILE */
    
    unsigned long nCreateTime;
    PRInt64 nFileSize;
    unsigned long nCurrentVer;
    unsigned long nBackupTime;
    long nFileType;
    unsigned long nArchiveNo;
    PRInt64 nArcStartPos;
    PRInt64 nArcEndPos;
    
    struct _BK_INDEX_NODE_ST_ *pBrother;
    struct _BK_INDEX_NODE_ST_ *pChild;
} BkIndexNodeSt; 

typedef struct _BK_INDEX_FILE_ST_ {
    char *pszFilename;
    
    unsigned char szFlag[4]; /* DIR or FILE */
    unsigned char szCreateTime[4];
    unsigned char szFileSize[8];
    unsigned char szCurrentVer[4];
    unsigned char szBackupTime[4];
    unsigned char szFileType[4];
    
    unsigned char szArchiveNo[4];
    unsigned char szArcStartPos[8];
    unsigned char szArcEndPos[8];
} BkIndexFileSt;

typedef struct _BK_SCHEDULE_INFO_{
    char szSchName[256];
    char szSourceName[64];  /* 备份了什么，如e:\cvsroot或DB名*/
    char szDesc[64];
    char szUser[32];
    char szTaskType[16];   /* file,sql,oracle … */

    char szReserve1[32];
    char szReserve2[32];
	struct _BK_SCHEDULE_INFO_ *pNext;
} SchFileHeaderSt; 

/* 备份任务结构信息 */
typedef struct _BK_TASK_NODE_ST {
    char szSchName[256];
    char szCreateTime[32];  /*第一次备份时间*/
    char szBackupTime[32];  /*当前版本备份时间*/
    char szFileSize[32];
    char szCurrentVer[4];
    char szBakType[4];   /* 全量、增量 */
    /* 记录Idx文件名，与此版本在信息在哪个位置开始，结束 */
    char szIdxName[4];
    char szIdxStartPos[8];
    char szIdxEndPos[8];
    /* archive name */
    char szArchiveNo[4];

    char szReserve1[16];
    char szReserve2[16];
    struct _BK_TASK_NODE_ST *pNext;  /* 查询时，返回多条任务结构记录 */
} SchTaskNodeSt;

typedef struct _BK_FILE_NODE_ST {
    unsigned long nArchiveNo;
    PRInt64 nArcStartPos;
    PRInt64 nArcEndPos;

    BkFileBaseSt FileBase;
}BkIndexBaseSt;

/* Convert '\\' to '/' */
void ConvertToSlash(char *pStr);

char* ConvertToUTF(char *pszFile);

/* open database.if not exist,just create.
 * @[in]
 * pszTablePath - the schedule db path.
 * pDataBase - the handle of sqlite db.
 * nFind - open failed return 1,otherwise 0;
 *
 * @[out]
 * return SQLITE_OK, if successful.
 * Otherwise, return error code.
 */
int DataBaseOpen(const char *pszTablePath, sqlite3 **pDataBase, int *nFind);

/* execute sql语句.
 * @[in]
 * pDataBase - the handle of sqlite db.
 * pszSQL - sql 语句.
 * pszErrMsg - return log.
 *
 * @[out]
 * return SQLITE_OK, if successful.
 * Otherwise, return error code.
 */
int DataBaseExec(sqlite3 *pDataBase, char *pszSQL, char **pszErrMsg);

int DataBaseGetSchTable(sqlite3 *pDataBase, char *pszSQL, char **pszErrMsg, SchFileHeaderSt **pSchNode);
int DataBaseGetTaskTable(sqlite3 *pDataBase, char *pszSQL, char **pszErrMsg, SchTaskNodeSt **pSchNode);

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

#ifdef __cplusplus
extern "C" {
#endif
    
#ifdef WIN32
#include <windows.h>
    
#define API_EXPORT(p) __declspec(dllexport) p
#else
#ifndef API_EXPORT
#define API_EXPORT(p) extern p
#endif
#endif

/* open database.if not exist,just create.
 * @[in]
 * pTaskSt - the struct SchTaskNodeSt value.
 *
 * @[out]
 * return SCHEDULE_TABLE_OK, if successful.
 * Otherwise, return error code.
 */
API_EXPORT(int) BkSchFreeTaskJobList(SchTaskNodeSt *pTaskSt);

/* open database.if not exist,just create.
 * @[in]
 * pTaskSt - the struct SchFileHeaderSt value.
 *
 * @[out]
 * return SCHEDULE_TABLE_OK, if successful.
 * Otherwise, return error code.
 */
API_EXPORT(int) BkSchFreeScheList(SchFileHeaderSt *pScheSt);


/************************************************************************/
/* add / search / delete sch node function                              */
/************************************************************************/
API_EXPORT(int) BkSchAddNode(request_rec *pReq, SchFileHeaderSt *pSchList, 
                     SchTaskNodeSt  *pTaskList);
/* search sheduleinfo table file node */
API_EXPORT(int) BkSchSearchNode(request_rec *pReq, char *pszScheName, SchFileHeaderSt **pSchNode);

/* search taskinfo table file node */
API_EXPORT(int) BkTaskSearchNode(request_rec *pReq, char *pszTaskName, SchTaskNodeSt **pTaskNode);

API_EXPORT(int) BkTaskMaxTimeNode(request_rec *pReq, char *pszTaskName,
								SchTaskNodeSt **pTaskNode);

/* delete table taskinfo file */
API_EXPORT(int) BkSchDeleteNode(request_rec *pReq, char *pszScheName);
/* delete table scheduleinfo file */
API_EXPORT(int) BkTaskDeleteNode(request_rec *pReq, char *pszScheName, char *pszCurrVer);


/************************************************************************/
/* index file                                                            */
/************************************************************************/

API_EXPORT(int) BkWriteIndexFile(request_rec *pReq, struct file_struct *file, 
                                    BkIndexNodeSt **pNode, PRInt64 nTotalSize);

API_EXPORT(int) BkFlushIndexTree(request_rec *pReq, BkIndexNodeSt *pRoot);

API_EXPORT(int) BkIndexNodeTree(request_rec *pReq, BkIndexNodeSt *pRoot, 
                                        BkIndexBaseSt *pFile, short nFlags, BkIndexNodeSt **pNode);

API_EXPORT(int) BkGetIndexname(request_rec *pReq, unsigned long nIndexno, char *pszIdxName, int nBufLen);

API_EXPORT(int) BkWriteTaskInfo(request_rec *pReq, BkFileBaseSt BaseFile, 
                                struct file_struct *file, PRInt64 nTotalFileLen);

API_EXPORT(int)  BkWriteArchiveFile(request_rec *pReq, struct file_struct *pFileSt, 
                                    PRFileDesc **pFD, char *pszRelationName);

API_EXPORT(int) BkArchiveFileWrite(PRFileDesc *pArchiveFD, char *pData, unsigned long nLength,
                                        unsigned long *pWriteBytes);

API_EXPORT(int)  BkIndexFileConstruct(request_rec *pReq, const char *pszStoreFile, SchTaskNodeSt *pNode, 
                        BkIndexNodeSt *pRoot, unsigned long *pCount);

API_EXPORT(int) BkGetFileBase(request_rec *pReq, char *pszFileName, BkFileBaseSt **pBkFile); 

API_EXPORT(int)  BkArchiveDataExtract(request_rec *pReq, BkIndexBaseSt stBase,
                                            void *pCtx, BkArcFileReadCallback pCbFunc);

API_EXPORT(int) BkGetFileVersion(request_rec *pReq, const char *pszFileName,
                                 BkFileVersionSt *stFileVer);

API_EXPORT(int) BkGetALLFileVersion(request_rec *pReq, const char *pszFileName,
                                 BkFileVersionSt **stFileVer);

API_EXPORT(int) BkGetFileFullName(request_rec *pReq, char **pszDirName,
                                  BkIndexNodeSt *IndexRoot);

API_EXPORT(int) BkGetArchiveNo(request_rec *pReq, char *pszPolicyName);

API_EXPORT(int) BkLastIndexSize(request_rec *pReq, unsigned long nArchiveno, PRInt64 *nFileSize);

#ifdef __cplusplus
}
#endif

#endif