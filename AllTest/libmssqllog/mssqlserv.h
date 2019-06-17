/** =============================================================================
 ** Copyright (c) 2011 WaveTop Information Corp. All rights reserved.
 **
 ** The mssql client of Backup system
 **
 ** =============================================================================
 */

#ifndef __MSSQL_SERVICE_H_
#define __MSSQL_SERVICE_H_ 1
//#define WAVETOP_ORA_IODATA_SEND_OK 1
/**
 * @defgroup windows services control functions
 * @{
 */
#ifdef WIN32
#include <windows.h>
#include <process.h>
#include <sql.h>
#include <sqlext.h>
#else
#include <unistd.h>
#endif


#include "wmain.h"
#include "server_log.h"
#include "wnetfd.h"

//#include "oraclient.h"
#include "libmssqllog.h"
//#include "readfile.h"
//#include "byteorder.h"
#include "wmd5.h"

#define WT_BK_ORACLE_CLIENT_TEMP            "temp"
#define WT_BK_ORACLE_CLIENT_BAK             "_bakwavetop"
#define WT_BK_ORACLE_CLIENT_TIMEOUT         30000

#define WAVETOP_SQL_HANDLE_HENV      0
#define WAVETOP_SQL_HANDLE_HDBC      1
#define WAVETOP_SQL_HANDLE_HSTMT     2

#define WAVETOP_SQL_COMMIT           0
#define WAVETOP_SQL_ROLLBACK         1

#define WAVETOP_MSSQL_LOG_FILE       1
#define WAVETOP_MSSQL_DATA_FILE      2

#define WAVETOP_UNI_GET_DB_INFO             "sp_helpdb"
#define WAVETOP_BACKUP6_DB_VERSION          "SELECT SERVERPROPERTY('productversion')"
#define WAVETOP_BACKUP6_DB_FILE             "select filename from sysfiles"

#define WAVETOP_LOP_END_CKPT              0x99
#define MI_MSSQL2_2K0_VERSION    8
#define MI_MSSQL2_2K5_VERSION    9
#define MI_MSSQL2_2K8_VERSION    10
#define WAVETOP_CDP_RESTORE_THREAD_NUM      2
#define WAVETOP_LOP_BEGIN_XACT            0x80
#define WAVETOP_BACKUP_SQL_CHECKPOINT       10
#define WAVETOP_BACKUP_SQL_FILTER           1
#define WAVETOP_BACKUP_MSSQL_DBCOUNT        128

#ifndef WAVETOP_BACKUP_SYBASE
#define WAVETOP_BACKUP_SYBASE               "SYBASE"
#endif

#ifndef WAVETOP_BACKUP_SYBASE_DB
#define WAVETOP_BACKUP_SYBASE_DB               "sybase"
#endif


extern struct global_conf_st * gCltMSSQLGlobalConf;
typedef void * CIOBlockHandle;
typedef void * pHandleSql; 

typedef struct _CONNECTION_MSSQL_ST_ {
    char szInstance[128];
    char szIP[32];
    char szUser[128];
    char szPassword[128];
    int nPort;
} MSSQLConnSt;

typedef struct _BACKUP6_MSSOL_FILE_ST_{
    char szFileName[MAXPATHLEN];
    _BACKUP6_MSSOL_FILE_ST_ *pNext;
} MSSQLDBFileSt;

typedef struct _SQL_DB_BASE_ST_ {
    char *pszDbName;
    char *pszDbSize;
    char *pszDbOwner;
    char *pszDbCreate;
    char *pszDbStatus;
    
    int nDbid;
    struct _SQL_DB_BASE_ST_ *pNext;
} MSSQLDbBaseSt;

typedef struct _BACKUP6_MSSQL_CONF_MODULELCT_ST_ {
    /* 触发全量标记 */
    int nFullFlag;
    /* 出错的IO块 */
    PRInt64 nErrSeq;
    int nModeId;
    char *pszUserName;
    char *pszUserPssWrd;
    char *pszSrvIP;
    int nSrvPort;
    int nDBPort;
    char *pszInstance;
    char *pszDBInstance;
    char *pszDBUser;
    char *pszDBPssWrd;
    char *pszDBName;
    char *pszObject;
    char *pszLogData;
    char *pszRedirctPath;
	char *pszCtrlPath;
	char *pszRedoPath;
    int  nIsSybase;
	int  nLanFree;
    int  nBPFlag;
    time_t time_now;
} MSSQLModCltConfSt;

typedef struct _MSSQL_INFO_ST {
    char szFullOraVersion[5];
    int nBlockSize;
    int nMSSQLVersion;
    int nDataFileCount;
    pHandleSql hMSSQL;
    MSSQLModCltConfSt *pModeCltConfInfo;
} MSSQLInfoSt;

typedef struct BkSqlFileSt {
    char szOldFileName[1024];
    char szFileName[1024];
    char szTempFileName[1024];
    PRFileDesc *nFd;
    /* if bStatus == ture, remane file name from szFileName to szTempFileName already. */
    bool bStatus;
    struct BkSqlFileSt *pNext;
}BkSqlFileSt;

struct ConnSt {
    unsigned long nStartTime;
    char *pszServer;
    int nPort;
    int nFlag;
    int nFD;
    int nInUse;
    struct ConnSt *next;
};

typedef struct _MSSQL_FILES_ST {
    char *pszFileName;
    PRFileDesc *fd;
    PRInt16 nFileNO;
    PRInt16 nFileType;
    PRInt32 nCompress;
    PRInt32 nOpenMod;
    PRInt64 nOffset;
    PRInt64 nSize;
    _MSSQL_FILES_ST *pNext;
} MSSQLFileSt;


//读写断点文件信息
#define nNeed_file          1120
#define	nNeed_name          1024
#define	nNeed_size          32
#define	nNeed_moditime      32
#define	nNeed_sendsize      32
//保存断点文件信息
typedef struct _BP_FILES_ST {
    char pszFileName[nNeed_name];
    PRInt64 nSize;
    PRInt64 nModifyTime;
    PRInt32 noraFileType;
    PRInt64 nReadSize;
    
    _BP_FILES_ST *pNext;
    } BPFileSt;




typedef struct _MSSQL_FULL_REDIRECT_PATH_ST_ {
    int nRedirect; /* 等于 1 时第一次重定向不保留历史留版本, 2 时保留历史版本, 0 时不做重定向 */
    char *pszOraCtrlFilePath;
    char *pszOraDataFilePath;
    char *pszOraRedoFilePath;
} MSSQLRedirectPathSt;

typedef struct BkMSSQLFileSt {
    char szFileName[1024];
    char szTempFileName[1024];
    PRFileDesc *nFd;
    /* if bStatus == ture, remane file name from szFileName to szTempFileName already. */
    bool bStatus;
    struct BkMSSQLFileSt *pNext;
}BkMSSQLFileSt;


typedef struct _MSSQL_FILE_BLOCK_ST {
    unsigned char *pszData;
    PRInt32 nDataSize;
    PRInt64 nOffset;
    _MSSQL_FILE_BLOCK_ST *pNext;
} MSSQLFileBlockSt;

typedef struct MSSQLFullFileListSt {
    char *pszFileName;
    char *pszTempFileNmae;
    PRInt16 nFileType;
    PRInt16 nRedirect;
    struct MSSQLFullFileListSt *next;
} MSSQLFullFileListSt;

/* IO块临时文件列表 */
typedef struct BACKUP_SQL_LOGFILE_ST {
    
    char szLogDataPath[1024];          //临时日志文件组路径
    PRInt64 nEndSeq;
    PRInt16 nTaskType;
    BACKUP_SQL_LOGFILE_ST *pNext;
    
}BkSqlLogFileSt;

typedef struct _BACKUP_MSSQL_RESTORE_CONF {
    sbuff *pBuff;
    request_rec *pReq;
    int nRecvQuit;
    int nWriteQuit;
    BkSqlLogFileSt *pLogDtaWriteQ;            // 写io块任务队列 
    PRLock *LogTaskL;                         // 写io块任务锁   
    PRCondVar *LogTaskC;                      // 写io块任务条件变量 
    int nWriteLogTaskCount;                   // 写io块任务个数 
    int nCount;
    char *pLogDataPath;
    MSSQLInfoSt *pSqlModInfo;                // 配置信息
    BkSqlFileSt *pSqlFileList;
    struct ThreadSlotst RestoreThreads[WAVETOP_CDP_RESTORE_THREAD_NUM];  /* 恢复线程池 */
    
} BkMSSQLConfSt;

extern BkMSSQLConfSt g_stBkMSSQLConf;

class CIOBlock {
public:
    /**
     * 初始化
     * @[in]
     * pszLogPath - 指定的日志文件组存储路径
     */
    CIOBlock(char *pszLogPath);
    ~CIOBlock();

    /**
     * 初始化
     * @[in]
     * pszNodeFileName - 指定的日志文件组存储路径
     * @[out]
     * 成功返回WAVETOP_BACKUP_OK, 否则返回失败原因.
     */
    int Init(MSSQLModCltConfSt *pstModConfInfo);

    /**
     * 初始化日志库
     * @[in]
     * pszLogPath - 日志文件存储路径
     * nOperation - 1 - reading. 2 - writing
     * pszSuff - 日志文件名拼接字符串
     * @[out]
     * 成功返回WAVETOP_BACKUP_OK，失败返回原因
     */
    int Start(char *pszLogPath, PRInt32 nOperation, char *pszSuff);

    /**
     * 该方法可以获取日志文件中最大和最小的序号
     * @[in]
     * @[out]
     * pnMax - 存放取得的最大序号值
     * pnMin - 存放取得的最小序号值
     * 成功返回WAVETOP_BACKUP_OK，失败返回原因
     */
    int GetMaxAndMinNum(PRUint64 *pnMax, PRUint64 *pnMin);

    /**
     * 该方法获取当前已经发送到服务器的最新节点
     * @[in]
     * @[out]
     * pIndexNum - 存放取得的已经发送完的最新节点
     * 成功返回WAVETOP_BACKUP_OK, 失败返回原因
     */
    int GetCurrentIndex(PRUint64 *pIndexNum);

    /**
     * 指定从某一序号开始读取数据
     * @[in]
     * nSeqNum - 起始序号。
     * nType - 如果是1，读取方式为从指定序号至末尾或头部。如果是2，读取方式为仅读取指定序号
     * @[out]
     * 成功返回WAVETOP_BACKUP_OK，失败返回原因
     */
    int ReadBufferFromLogStart(PRUint64 nSeqNum, PRInt32 nType);

    /**
     * 指定从某一序号开始读取数据
     * @[in]
     * pszBuffer - 待写入缓冲区
     * nSize - 待写入缓冲区长度
     * @[out]
     * pnReadBytes - 返回已写入数据的长度
     * pIndex - 返回索引节点.
     * 并返回WAVETOP_MIRROR_MSSQL2_LOG_MORE_WITH_LINE，然后再次调用此方法接着读取。
     * 该序号读取完成返回WAVETOP_BACKUP_OK
     * 已读取至头部返回WAVETOP_BACKUP_END
     * 出现异常返回错误码
     */
    int ReadBufferFromLogNext(char *pszBuffer, unsigned long nSize,
                                   unsigned long *pnReadBytes, ExIndexSt *pIndex);

    /* 写LOGDATA库的共享内存 */
    int BkWriteShareMM(PRInt32 nWay);

    /* 打开LOGDATA库的共享内存 */
    int BkReadShareMM(PRInt32 nWay, PRInt32 *nStatus);

    /**
     * 复制IO数据块到服务器
     * @[in]
     * pServerList - 服务器信息
     * @[out]
     * 成功返回WAVETOP_BACKUP_OK, 否则返回失败原因.
     */
    int SendIOBlock2Serv(sbuff *pSBuff, MSSQLModCltConfSt *pstModConfInfo, int nType);

    /**
    * 指定从某一序号读取数据结束
    * @[in]
    * @[out]
    * 成功返回WAVETOP_BACKUP_OK
    * 失败返回原因s
    */
    PRInt32 ReadBufferEnd();

    int BKGetServerSeq( sbuff *pSB, MSSQLModCltConfSt *pstModConfInfo, PRUint64 *nGetSeq);

    

private:
    char*            m_pszLogPath;
    MiLogHandle     m_hLog;
};


class CMSSQLGetChkPoint{
public:
    /**
     * 
     * @[in]
     * pszLogPath - 日志文件路径
     * @[out]
     * 无.
     */
    CMSSQLGetChkPoint(int nVersion, char *pszLogPath);
    virtual ~CMSSQLGetChkPoint();

    /**
     * 初始化
     * @[in]
     * nStartIndex - 需要解析的开始节点
     * pszVersion - 数据库版本
     * @[out];
     * 无;
     * return 成功返回WAVETOP_BACKUP_OK,失败返回错误代码
     */
    int Init(MSSQLInfoSt *pstOraInfo);

    /**
     * 初始化日志库
     * @[in]
     * pszLogPath - 日志文件存储路径
     * nOperation - 1 - reading. 2 - writing
     * pszSuff - 日志文件名拼接字符串
     * @[out]
     * 成功返回WAVETOP_BACKUP_OK，失败返回原因
     */
    int Start(char *pszLogPath, PRInt32 nOperation, char *pszSuff);

    int GetMaxAndMinNum(PRUint64 *pnMax, PRUint64 *pnMin);
    
    
    int ReadBufferFromLogNext(char *pszBuffer, unsigned long nSize,
        unsigned long *pnReadBytes, ExIndexSt *pIndex);
    
    
    int ReadBufferFromLogStart(PRUint64 nSeqNum, PRInt32 nType);
    
    
    /**
     * 解析checkpoint
     * @[in]    
     * 无
     * @[out]
     * nCheckPointIndex - checkpoint所在的IO块索引
     *return
     * 成功返回WAVETOP_BACKUP_OK，失败返回原因
     */
    int Parse(PRUint64 *nCheckPointIndex, unsigned long *nCheckPointTime, char *pszTableName);

    int GetDBVersion();
    int GetMaxIndex();
    PRUint64 GetMaxSeq();
     /**
    * 指定从某一序号读取数据结束
    * @[in]
    * @[out]
    * 成功返回WAVETOP_BACKUP_OK
    * 失败返回原因
    */
    PRInt32 ReadBufferEnd();

private:
    int m_nVersion;
    char* m_pszLogPath;
    MiLogHandle m_hLog;
    PRInt64 m_nIndexNum;
};

class CSqlConnect {
public:
    
    CSqlConnect();
    ~CSqlConnect();
    
public:
    
    int Connect();
    
    SQLHSTMT Allochstmt();
    
    void FreeHstmt(SQLHSTMT hStmt);
    
    int SetCommitAttribute(int nSwitch);
    
    int SetRowCount(int nRowCount);
    
    int ReConnect();
    
    void Close();
    
    SQLCHAR *GetMsgError();

    int RunCheckPoint(const char *pszCmd, SQLHSTMT hstmt);
    
    int GetDbVersion(PRInt32 *pnVersion, SQLHSTMT hstmt);
    
    int GetDbFile(pool *pMen, SQLHSTMT hstmt, MSSQLDBFileSt **pstFileList, char *pszSql);
    
    int GetTableInfo(const char *pszDbName, int nSysTable, MSSQLDbBaseSt **pData, SQLHSTMT hstmt);
    
    int GetInfoByCmd(const char *pszCmd, MSSQLDbBaseSt **pData, SQLHSTMT hstmt);
    
    int SetEnv(const char *pszInstance, const char *pszIP, int nPort,    
        const char *pszUser, const char *pszPassword);
    
    int GetODBCErr(int nHandleType, SQLHSTMT hStmt, SQLCHAR *Sqlstate, SQLINTEGER *NativeError, 
        SQLCHAR *MessageText, SQLSMALLINT BufferLength, SQLSMALLINT *TextLength);
    
    int EndTran(int nHandleType, int nOperation);

	int GetDbStatus(PRInt32 &status, SQLHSTMT hstmt,char *pszDbName);

	int CheckDbStatus(MSSQLConnSt *pstSqlConn,char *pszDbName);
    
    SQLHDBC GetHdbc();
    
private:
    void Clear();
    
private:
    
    /* ODBC环境句柄 */
    SQLHENV m_hEnv;
    
    /* ODBC连接句柄 */
    SQLHDBC m_hDbc;
    
    /* ODBC错误信息*/
    SQLCHAR MsgError[1024];
    
    char m_szInstance[256];
    
    char m_szIP[16];
    
    char m_szUser[256];
    
    char m_szPassword[256];
    
    int m_nPort;
};

/* 全量同步模块初始化 */
int OraFullSyncModInit();


PRInt32 ReadLine(sbuff *pSBuff, char *pszLine, PRInt32 nLinesize, PRInt32 *nReadBytes);

/* 全量 */
int ModuleMSSQLHandleDBSync(request_rec *pReq, pool *pPool, MSSQLModCltConfSt *pstModConf);

/*int OraSendFullData(OraFileSt *pstFileList, sbuff *pSB, OracleInfoSt *pstOraInfo);*/

/* 计划增量 */
int ModuleMSSQLIncrement(request_rec *pReq, pool *pPool, MSSQLModCltConfSt *pstModConf);

/* 接收文件名 */
int BkSqlRecvFileName(sbuff *pBuff, char *pszPath, int nLen, int *nRecvFlag);

/* 接收增量版本文件 */
int BkSqlRecIOFileList(pool *pPool, sbuff *psBuff, char *pszLogDataDir, int nType = 0);

/* 打开文件 */
int BkOpenFile(BkSqlFileSt *pstFullList, char *pszFileName, PRFileDesc **nFd);

/* 关闭文件句柄 */
void BKClosefile(BkSqlFileSt *pstFileList);

/* 操作数据库 */
int BKSqlOperMSSQL(MSSQLConnSt *pUserInfo, const char *pszWorkPath, const char *pszSQLStmt);

/* 操作数据库 */
int BKSqlOperSybase(MSSQLConnSt *pUserInfo, const char *pszWorkPath, const char *pszSQLStmt);

/* 接收并保存文件 */
int InSaveFile(PRFileDesc *pFd, sbuff *pBuff);

/* 获取模块配置文件 */
int GetMSSQLModuleConf(pool *pPool, ModNode *pstModeNode, MSSQLModCltConfSt **pstModConf);

void BkSqlLogWrite(char *pszFmt, ...);
void SqlLogInfoWrite(char *pszFmt, ...);


/* IO buffer */
int BKMSSqlIOBuffer(MSSQLModCltConfSt *pstModConf, int nBackupType);

/* 登入服务器 */
int BKMSSqlLoginServer(sbuff *pBuff, MSSQLModCltConfSt *pstModConfInfo);

/* get mssql info */
int InitSqlConnect(pool *pMem, MSSQLConnSt *pMsSql, pHandleSql *pHandle);
int GetSqlDbVersion(pool *pMem, pHandleSql pHandle, int *pnVersion);
int GetSqlDbFile(pool *pMen, pHandleSql pHandle, char *pszDBName, MSSQLDBFileSt **pstFileList);
int RunCheckPoint(pHandleSql pHandle, MSSQLInfoSt *pstSqlInfo, char *pszTableName, int nLen);
void UnInitSqlConnect(pHandleSql pHandle);
BOOL BKSqlFindCheckPoint(char *pszLogData, int nDataLen, int nDBVer, 
                         PRInt16 nFileNO, char *pszTableName, int *pnFind);
int BkSqlStartService(char* pszServName);

int BkSqlStopService(char *pszServerName);

/* ';' 符号分离数据库 */
int BkSplitDBName(pool *pPool, char **ppDbNameList, char *pszDbName, int *pnCount);

int BkGetMSSQLDBName(request_rec *pReq, char *pszDBNameVal, char **pszDBNameList, int nCount);

/** 类似于BACKUP.EXE发给客户端的全量任务
 * 这个方法有客户端调用自己触发全量 
 **/
int BkSparkFullBackup(MSSQLModCltConfSt *pstSqlInfo);

unsigned short BkChangeAuthoritychar(char const*file);

#endif /* !defined(__ORA_SERVICE_H_) */
