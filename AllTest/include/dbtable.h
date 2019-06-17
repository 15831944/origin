/** =============================================================================
** Copyright (c) 2004 WaveTop Information Corp. All rights reserved.
**
** The Report Table system
**
** =============================================================================
*/

#ifndef __BACKUP_DBTABLE_H_
#define __BACKUP_DBTABLE_H_ 1

#include <string>
#pragma warning(disable:4786) 
#include <map>
typedef struct _BK_SCHEDULE_INFO_{
    char szSchName[256];
    char szSourceName[4096];  /* 备份了什么，如e:\cvsroot或DB名*/
    char szDesc[64];
    char szUser[32];
    char szTaskType[16];   /* file,sql,oracle … */

    char szBlockSize[32];
    char szCtrlNum[32];
	struct _BK_SCHEDULE_INFO_ *pNext;
} SchFileHeaderSt; 

/* 备份任务结构信息 */
typedef struct _BK_TASK_NODE_ST {
    char szSchName[256];
    char szCreateTime[32];  /*第一次备份时间*/
    char szBackupTime[32];  /*当前版本备份时间*/
    char szFileSize[32];
    char szCurrentVer[16];
    char szBakType[4];   /* 全量、增量 */
    /* 记录Idx文件名，与此版本在信息在哪个位置开始，结束 */
    char szIdxName[16];
    char szIdxStartPos[32];
    char szIdxEndPos[32];
    /* archive name */
    char szArchiveNo[16];
	/* 数据库的checkpoint */
	char szCheckPoint[64];

    char szReserve1[16];
    char szReserve2[16];
	char szMode[8];
	char szUid[4];
	char szGid[4];
    struct _BK_TASK_NODE_ST *pNext;  /* 查询时，返回多条任务结构记录 */
} SchTaskNodeSt;

/* 备份任务结构信息 */
typedef struct _BK_DATA_INFO_{
    char szsource_ip[32];
    char szdbsize[64];
    char szmodtype[32];
    char szcreatedate[64];
} SchDataSt; 

/* archive 任务结束偏移位置表 */
typedef struct _BK_ARC_POS_ST {
    char szArchiveNo[8];
    char szArcEndPos[64];
    char szType[16];
    char szDesc[64];
} SchArcPosSt;

/* archive no 数量 */
typedef struct _BK_ARC_NO_ST {
    char szSchName[256];
    char szCurrentVer[16];
    char szArcNo[64];
    char szDesc[64];
} SchArcNoSt;

typedef struct wtStrLess {
public:
    bool operator()(const char* left,const char* right)const{
        return strcmp(left,right) < 0;
    }
}wtLess;

//typedef std::unordered_map<std::string,struct _BK_INDEX_NODE_ST_ *> HashTable;
typedef std::map <const char*,struct _BK_INDEX_NODE_ST_ *,wtLess> HashTable;
//typedef std::unordered_map<std::string,struct _BK_INDEX_NODE_ST_ *> HashTable;
typedef HashTable* PHashTable;

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
    bool bIsBackup;
	mode_t mode;
	uid_t  uid;
    gid_t  gid;
    PRInt32 nBackupType;
    struct _BK_INDEX_NODE_ST_ *pBrother;
	struct _BK_INDEX_NODE_ST_ *pChild;
	PHashTable pMap;
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
	unsigned char szMode[4];
	unsigned char szUid[4];
	unsigned char szGid[4];
} BkIndexFileSt;

typedef struct _BK_ARCHIVE_BUFF_ST {
    char *pszBuff;
    unsigned long nBlockSize;
    PRInt64 nInPut;
    PRInt64 nOutPut;
    PRInt64 nFileSize;
}ArchiveBlockBuff;

#endif