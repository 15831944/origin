/** =============================================================================
 ** Copyright (c) 2003 WaveTop Information Corp. All rights reserved.
 **
 ** The Mirror system
 **
 ** =============================================================================
 */

/**
 * @desc ms sql mirror library data struct
 * @author lexiongjia
 * @file libmimssql_types.h
 */

#ifndef _LIBMIMSSQL_TYPES_H_ 
#define _LIBMIMSSQL_TYPES_H_ 1

#include "nspr.h"

/**
 * mssql library operation
 */
typedef int MiMSSqlOperate;
#define WT_MI_MSSQL_OPT_EXPORT      1
#define WT_MI_MSSQL_OPT_IMPORT      2
#define WT_MI_MSSQL_OPT_UNKNOWN     3

/**
 * log callback function define
 */
typedef void (*MiMSSqlLogCallback)(void *pCallbackReserver, 
                                   MiMSSqlOperate nOpt, 
                                   int nStatus, 
                                   char *pszMsg, 
                                   char *pszSrcFile, 
                                   int nSrcLine);

/**
 * run status callback function define
 */
typedef int MiMSSqlProgress;
#define WT_MI_MSSQL_PROGRESS_PREPARE            (1<< 0)
#define WT_MI_MSSQL_PROGRESS_RUNING             (1<< 1)
#define WT_MI_MSSQL_PROGRESS_PART_COMPLETE      (1<< 2)
#define WT_MI_MSSQL_PROGRESS_FULL_COMPLETE      (1<< 3)

#define WT_MI_MSSQL_PROGRESS_SUCCESS            (1<< 29)
#define WT_MI_MSSQL_PROGRESS_EXCEPTION          (1<< 30)
#define WT_MI_MSSQL_PROGRESS_ERROR              (1<< 31)

typedef struct MiMSSqlRunStatus MiMSSqlRunStatus;
struct MiMSSqlRunStatus
{
    MiMSSqlOperate nOpt;
    MiMSSqlProgress nProgress;
    int nStatus;
    int nStreamId;
};
typedef void (*MiMSSqlStatusCallback)(void *pCallbackReserver, 
                                      MiMSSqlRunStatus *pStatus);

typedef int MiMSSqlTerminate;
#define WT_MI_MSSQL_TERM_STILL_RUN      0
#define WT_MI_MSSQL_TERM_TERMINATE      1
typedef void (*MiMSSqlTerminateCallback)(void *pCallbackReserver, 
                                         MiMSSqlTerminate *pnTerminate);

/**
 * mssql library run mode 
 */
typedef int MiMSSqlOptRunMode;

/**< multi-threads run mode */
#define WT_MI_MSSQL_MULTI_THREADS       1

/**< single thread run mode  */
#define WT_MI_MSSQL_SINGLE_THREADS      2

/**
 * database export mode 
 */
typedef int MiMSSqlExportMode;

/**< snapshot export mode */
#define WT_MI_MSSQL_EXPORT_SNAPSHOT            1

/**< database full export mode */
#define WT_MI_MSSQL_EXPORT_DATABASE            2

/**< log export mode */
#define WT_MI_MSSQL_EXPORT_LOG                 3

/**< database diff export mode */
#define WT_MI_MSSQL_EXPORT_DIFF                4

/**< file filegroup export mode */
#define WT_MI_MSSQL_EXPORT_FILE_GROUP          5

/**< database full, log export mode */
#define WT_MI_MSSQL_EXPORT_DATALOG             6

/**
 * mssql authentication mode
 */
typedef int MiMSSqlAuthMode;

/**< windows authentication mode */
#define WT_MI_MSSQL_AUTH_WIN      1

/**< mssql authentication mode */
#define WT_MI_MSSQL_AUTH_SQL      2

/**
 * mssql bkkey type
 */
typedef int MiMSSqlBKKeyType;

/**< mssql backup set id */
#define WT_MI_MSSQL_BAKSET_ID                     1  

/**< mssql restore history id */
#define WT_MI_MSSQL_RESHIS_ID                     2   

/**< mssql backup set id of once restore */
#define WT_MI_MSSQL_RESBS_ID                      3   

typedef struct MSSqlOptFileList MSSqlOptFileList;
struct MSSqlOptFileList {
    char *pszOptFile;
    MSSqlOptFileList *pNext;
};

/**< mssql restore mode */
typedef int MiMSSqlRestoreMode;
#define WT_MI_MSSQL_RESTORE_NORMAL          0
#define WT_MI_MSSQL_RESTORE_NORECOVERY      1
#define WT_MI_MSSQL_RESTORE_RECOVERY        2
#define WT_MI_MSSQL_RESTORE_STANDBY         3

typedef long MiMSSqlRestoreOpt;
#define WT_MI_MSSQL_RESTORE_REPLACE         (1<<0)
#define WT_MI_MSSQL_RESTORE_MOVE            (1<<1)

#define WT_MI_MSSQL_BACKUP_LOG_NO_TRUNCATE   0
#define WT_MI_MSSQL_BACKUP_LOG_TRUNCATE      1   

#define WT_MI_MSSQL_BACKUP_NO_DBCC_CHECKDB   0      
#define WT_MI_MSSQL_BACKUP_DBCC_CHECKDB      1   

#define WT_MI_MSSQL_MAX_NAME        32
#define WT_MI_MSSQL_MAX_DESC        128
#define WT_MI_MSSQL_MAX_PREFIX      128
#define WT_MI_MSSQL_MAX_DBNAME      128
#define WT_MI_MSSQL_MAX_INSTNAME    128
#define WT_MI_MSSQL_MAX_EXP_DIR     260

/* the Structure of exported file */
typedef struct _MiMSSqlstDbString {
    char szDBVName[512] ;
    char szDbString[512];
    struct _MiMSSqlstDbString *pNext;
}MiMSSqlstDbString;

/* The extension structure of exported file */
typedef struct _MssqlExpFile_St {
    char szFileName[256];
    long nFlag;
    unsigned long nVersion;
    unsigned long nStreamCount;
    unsigned long nReserve1;
    char szOptDBName[WT_MI_MSSQL_MAX_DBNAME];
    char szBakSetID[32];
    char szResHisID[32];
    char szResBSID[32];
    struct _MssqlExpFile_St *pNext;
} MssqlExpFileSt;

/* Backup File Histroy Version Information Structure */
typedef struct _Bak_History_Info_St {
    unsigned long nVersion;
    unsigned long nBackupTime;
    PRInt64 nFilesize;
    int nBackupType;
    int nFileType;
    char pszMessage[1024];
    unsigned char szBinary[80];
    struct _Bak_History_Info_St *next;
} BakHistroyInfoSt;

typedef struct _MSSqlServ
{
    /**< operation ms sql server name */
    char szDBServ[512];

    /**< operation ms sql user name */
    char szUser[512];

    /**< operation ms sql password */
    char szPasswd[128];

	/**< operation ms sql port */
    int port;

    /**< sql server authentication mode */
    MiMSSqlAuthMode nAuth;
} MSSqlServ;

typedef struct MssqlSettingSt MssqlSettingSt;
struct MssqlSettingSt {
    /* mssql serv */
    MSSqlServ stMSSqlServ;

    /**< max stream count */
    int nMaxStream;

    /**< initialize operation time out */    
    DWORD nInitTimeOut;

    /**< wait virtual device time */
    DWORD nVirDevTimeOut;

    /**< isql.exe directory, if pszISQLDir 
         equal NULL then use "" */
    char szISQLDir[MAX_PATH];

    /**< isql.exe relative file name, if pszISQLRelFile 
         equal NULL then use default file name isql.exe */
    char szISQLRelFile[MAX_PATH];
};

typedef struct MSSqlExportOpt MSSqlExportOpt;
struct  MSSqlExportOpt {
    /**< ms sql server setting */
    MssqlSettingSt stMSSql;

    /**< operation Instance name */
    WCHAR szOptInstName[WT_MI_MSSQL_MAX_INSTNAME];

    /**< operation database name */
    char szOptDBName[WT_MI_MSSQL_MAX_DBNAME];

    /**< database backup mode */
    MiMSSqlExportMode nBkMode;

    /**< run mode multi-thread or single */
    MiMSSqlOptRunMode nRunMode;

    /**< stream count */
    int nDataStream;

    /**< virtual device prefix name */
    char szVirDevPrefix[WT_MI_MSSQL_MAX_PREFIX];

    /**< verify export data */
    int nVerify;

    /**< delete noactivelog */
    int nTruncateLog;

    /**< dbcc checkdb */
    int nDBCCCheckdb;

    /**< storage block size */
    int nBlocakSize;

    /**< export directory */
    char szExpDir[WT_MI_MSSQL_MAX_EXP_DIR];

    /**< operation descripton */
    char szDesc[WT_MI_MSSQL_MAX_DESC];

    /**< operation name */
    char szName[WT_MI_MSSQL_MAX_NAME];

    int nBufferCount;
    int nTransferSize;
    int nFetchMode;

    /* isql process exit code */
    unsigned int nISqlExitCode;

    /* isql output message */
    char szISQLCmdStdOut[2048];
    char szISQLCmdStdErr[2048];

    void *pConn;
};

typedef struct MSSqlImportOpt MSSqlImportOpt;
struct  MSSqlImportOpt {
    /**< ms sql server setting */
    MssqlSettingSt stMSSql;

    /**< operation Instance name */
    WCHAR szOptInstName[WT_MI_MSSQL_MAX_INSTNAME];

    /**< operation database name */
    char szOptDBName[WT_MI_MSSQL_MAX_DBNAME];

    /**< database backup mode */
    MiMSSqlExportMode nBkMode;

    /**< run mode multi-thread or single */
    MiMSSqlOptRunMode nRunMode;

    /**< virtual device prefix name */
    char szVirDevPrefix[WT_MI_MSSQL_MAX_PREFIX];

    /**< stream count */
    int nDataStream;

	/**< Compress flag */
    int nCompress;

    /**< import directory */
    char szImpDir[WT_MI_MSSQL_MAX_EXP_DIR];

    MiMSSqlRestoreMode nRestoreMode;

    char szStandByUndoFile[512];

    MiMSSqlRestoreOpt nRestoreOpt;


    /* isql process exit code */
    unsigned int nISqlExitCode;

    /* isql output message */
    char szISQLCmdStdOut[2048];
    char szISQLCmdStdErr[2048];

	/* move string */
	MiMSSqlstDbString *pstDbFileName;
	MiMSSqlstDbString *pstDblogName;
	// XXX del
	/*
	char szSrcFilename[2048];
	char szSrcLogName[2048];
	*/

	char szDirector[2048];

    /* redirect path */
    char szDataRedirectPath[WT_MI_MSSQL_MAX_EXP_DIR];
    char szLogRedirectPath[WT_MI_MSSQL_MAX_EXP_DIR];
};

#endif /* !defined(_LIBMIMSSQL_TYPES_H_)  */
