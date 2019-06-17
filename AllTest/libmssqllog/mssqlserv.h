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
    /* ����ȫ����� */
    int nFullFlag;
    /* �����IO�� */
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


//��д�ϵ��ļ���Ϣ
#define nNeed_file          1120
#define	nNeed_name          1024
#define	nNeed_size          32
#define	nNeed_moditime      32
#define	nNeed_sendsize      32
//����ϵ��ļ���Ϣ
typedef struct _BP_FILES_ST {
    char pszFileName[nNeed_name];
    PRInt64 nSize;
    PRInt64 nModifyTime;
    PRInt32 noraFileType;
    PRInt64 nReadSize;
    
    _BP_FILES_ST *pNext;
    } BPFileSt;




typedef struct _MSSQL_FULL_REDIRECT_PATH_ST_ {
    int nRedirect; /* ���� 1 ʱ��һ���ض��򲻱�����ʷ���汾, 2 ʱ������ʷ�汾, 0 ʱ�����ض��� */
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

/* IO����ʱ�ļ��б� */
typedef struct BACKUP_SQL_LOGFILE_ST {
    
    char szLogDataPath[1024];          //��ʱ��־�ļ���·��
    PRInt64 nEndSeq;
    PRInt16 nTaskType;
    BACKUP_SQL_LOGFILE_ST *pNext;
    
}BkSqlLogFileSt;

typedef struct _BACKUP_MSSQL_RESTORE_CONF {
    sbuff *pBuff;
    request_rec *pReq;
    int nRecvQuit;
    int nWriteQuit;
    BkSqlLogFileSt *pLogDtaWriteQ;            // дio��������� 
    PRLock *LogTaskL;                         // дio��������   
    PRCondVar *LogTaskC;                      // дio�������������� 
    int nWriteLogTaskCount;                   // дio��������� 
    int nCount;
    char *pLogDataPath;
    MSSQLInfoSt *pSqlModInfo;                // ������Ϣ
    BkSqlFileSt *pSqlFileList;
    struct ThreadSlotst RestoreThreads[WAVETOP_CDP_RESTORE_THREAD_NUM];  /* �ָ��̳߳� */
    
} BkMSSQLConfSt;

extern BkMSSQLConfSt g_stBkMSSQLConf;

class CIOBlock {
public:
    /**
     * ��ʼ��
     * @[in]
     * pszLogPath - ָ������־�ļ���洢·��
     */
    CIOBlock(char *pszLogPath);
    ~CIOBlock();

    /**
     * ��ʼ��
     * @[in]
     * pszNodeFileName - ָ������־�ļ���洢·��
     * @[out]
     * �ɹ�����WAVETOP_BACKUP_OK, ���򷵻�ʧ��ԭ��.
     */
    int Init(MSSQLModCltConfSt *pstModConfInfo);

    /**
     * ��ʼ����־��
     * @[in]
     * pszLogPath - ��־�ļ��洢·��
     * nOperation - 1 - reading. 2 - writing
     * pszSuff - ��־�ļ���ƴ���ַ���
     * @[out]
     * �ɹ�����WAVETOP_BACKUP_OK��ʧ�ܷ���ԭ��
     */
    int Start(char *pszLogPath, PRInt32 nOperation, char *pszSuff);

    /**
     * �÷������Ի�ȡ��־�ļ���������С�����
     * @[in]
     * @[out]
     * pnMax - ���ȡ�õ�������ֵ
     * pnMin - ���ȡ�õ���С���ֵ
     * �ɹ�����WAVETOP_BACKUP_OK��ʧ�ܷ���ԭ��
     */
    int GetMaxAndMinNum(PRUint64 *pnMax, PRUint64 *pnMin);

    /**
     * �÷�����ȡ��ǰ�Ѿ����͵������������½ڵ�
     * @[in]
     * @[out]
     * pIndexNum - ���ȡ�õ��Ѿ�����������½ڵ�
     * �ɹ�����WAVETOP_BACKUP_OK, ʧ�ܷ���ԭ��
     */
    int GetCurrentIndex(PRUint64 *pIndexNum);

    /**
     * ָ����ĳһ��ſ�ʼ��ȡ����
     * @[in]
     * nSeqNum - ��ʼ��š�
     * nType - �����1����ȡ��ʽΪ��ָ�������ĩβ��ͷ���������2����ȡ��ʽΪ����ȡָ�����
     * @[out]
     * �ɹ�����WAVETOP_BACKUP_OK��ʧ�ܷ���ԭ��
     */
    int ReadBufferFromLogStart(PRUint64 nSeqNum, PRInt32 nType);

    /**
     * ָ����ĳһ��ſ�ʼ��ȡ����
     * @[in]
     * pszBuffer - ��д�뻺����
     * nSize - ��д�뻺��������
     * @[out]
     * pnReadBytes - ������д�����ݵĳ���
     * pIndex - ���������ڵ�.
     * ������WAVETOP_MIRROR_MSSQL2_LOG_MORE_WITH_LINE��Ȼ���ٴε��ô˷������Ŷ�ȡ��
     * ����Ŷ�ȡ��ɷ���WAVETOP_BACKUP_OK
     * �Ѷ�ȡ��ͷ������WAVETOP_BACKUP_END
     * �����쳣���ش�����
     */
    int ReadBufferFromLogNext(char *pszBuffer, unsigned long nSize,
                                   unsigned long *pnReadBytes, ExIndexSt *pIndex);

    /* дLOGDATA��Ĺ����ڴ� */
    int BkWriteShareMM(PRInt32 nWay);

    /* ��LOGDATA��Ĺ����ڴ� */
    int BkReadShareMM(PRInt32 nWay, PRInt32 *nStatus);

    /**
     * ����IO���ݿ鵽������
     * @[in]
     * pServerList - ��������Ϣ
     * @[out]
     * �ɹ�����WAVETOP_BACKUP_OK, ���򷵻�ʧ��ԭ��.
     */
    int SendIOBlock2Serv(sbuff *pSBuff, MSSQLModCltConfSt *pstModConfInfo, int nType);

    /**
    * ָ����ĳһ��Ŷ�ȡ���ݽ���
    * @[in]
    * @[out]
    * �ɹ�����WAVETOP_BACKUP_OK
    * ʧ�ܷ���ԭ��s
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
     * pszLogPath - ��־�ļ�·��
     * @[out]
     * ��.
     */
    CMSSQLGetChkPoint(int nVersion, char *pszLogPath);
    virtual ~CMSSQLGetChkPoint();

    /**
     * ��ʼ��
     * @[in]
     * nStartIndex - ��Ҫ�����Ŀ�ʼ�ڵ�
     * pszVersion - ���ݿ�汾
     * @[out];
     * ��;
     * return �ɹ�����WAVETOP_BACKUP_OK,ʧ�ܷ��ش������
     */
    int Init(MSSQLInfoSt *pstOraInfo);

    /**
     * ��ʼ����־��
     * @[in]
     * pszLogPath - ��־�ļ��洢·��
     * nOperation - 1 - reading. 2 - writing
     * pszSuff - ��־�ļ���ƴ���ַ���
     * @[out]
     * �ɹ�����WAVETOP_BACKUP_OK��ʧ�ܷ���ԭ��
     */
    int Start(char *pszLogPath, PRInt32 nOperation, char *pszSuff);

    int GetMaxAndMinNum(PRUint64 *pnMax, PRUint64 *pnMin);
    
    
    int ReadBufferFromLogNext(char *pszBuffer, unsigned long nSize,
        unsigned long *pnReadBytes, ExIndexSt *pIndex);
    
    
    int ReadBufferFromLogStart(PRUint64 nSeqNum, PRInt32 nType);
    
    
    /**
     * ����checkpoint
     * @[in]    
     * ��
     * @[out]
     * nCheckPointIndex - checkpoint���ڵ�IO������
     *return
     * �ɹ�����WAVETOP_BACKUP_OK��ʧ�ܷ���ԭ��
     */
    int Parse(PRUint64 *nCheckPointIndex, unsigned long *nCheckPointTime, char *pszTableName);

    int GetDBVersion();
    int GetMaxIndex();
    PRUint64 GetMaxSeq();
     /**
    * ָ����ĳһ��Ŷ�ȡ���ݽ���
    * @[in]
    * @[out]
    * �ɹ�����WAVETOP_BACKUP_OK
    * ʧ�ܷ���ԭ��
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
    
    /* ODBC������� */
    SQLHENV m_hEnv;
    
    /* ODBC���Ӿ�� */
    SQLHDBC m_hDbc;
    
    /* ODBC������Ϣ*/
    SQLCHAR MsgError[1024];
    
    char m_szInstance[256];
    
    char m_szIP[16];
    
    char m_szUser[256];
    
    char m_szPassword[256];
    
    int m_nPort;
};

/* ȫ��ͬ��ģ���ʼ�� */
int OraFullSyncModInit();


PRInt32 ReadLine(sbuff *pSBuff, char *pszLine, PRInt32 nLinesize, PRInt32 *nReadBytes);

/* ȫ�� */
int ModuleMSSQLHandleDBSync(request_rec *pReq, pool *pPool, MSSQLModCltConfSt *pstModConf);

/*int OraSendFullData(OraFileSt *pstFileList, sbuff *pSB, OracleInfoSt *pstOraInfo);*/

/* �ƻ����� */
int ModuleMSSQLIncrement(request_rec *pReq, pool *pPool, MSSQLModCltConfSt *pstModConf);

/* �����ļ��� */
int BkSqlRecvFileName(sbuff *pBuff, char *pszPath, int nLen, int *nRecvFlag);

/* ���������汾�ļ� */
int BkSqlRecIOFileList(pool *pPool, sbuff *psBuff, char *pszLogDataDir, int nType = 0);

/* ���ļ� */
int BkOpenFile(BkSqlFileSt *pstFullList, char *pszFileName, PRFileDesc **nFd);

/* �ر��ļ���� */
void BKClosefile(BkSqlFileSt *pstFileList);

/* �������ݿ� */
int BKSqlOperMSSQL(MSSQLConnSt *pUserInfo, const char *pszWorkPath, const char *pszSQLStmt);

/* �������ݿ� */
int BKSqlOperSybase(MSSQLConnSt *pUserInfo, const char *pszWorkPath, const char *pszSQLStmt);

/* ���ղ������ļ� */
int InSaveFile(PRFileDesc *pFd, sbuff *pBuff);

/* ��ȡģ�������ļ� */
int GetMSSQLModuleConf(pool *pPool, ModNode *pstModeNode, MSSQLModCltConfSt **pstModConf);

void BkSqlLogWrite(char *pszFmt, ...);
void SqlLogInfoWrite(char *pszFmt, ...);


/* IO buffer */
int BKMSSqlIOBuffer(MSSQLModCltConfSt *pstModConf, int nBackupType);

/* ��������� */
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

/* ';' ���ŷ������ݿ� */
int BkSplitDBName(pool *pPool, char **ppDbNameList, char *pszDbName, int *pnCount);

int BkGetMSSQLDBName(request_rec *pReq, char *pszDBNameVal, char **pszDBNameList, int nCount);

/** ������BACKUP.EXE�����ͻ��˵�ȫ������
 * ��������пͻ��˵����Լ�����ȫ�� 
 **/
int BkSparkFullBackup(MSSQLModCltConfSt *pstSqlInfo);

unsigned short BkChangeAuthoritychar(char const*file);

#endif /* !defined(__ORA_SERVICE_H_) */
