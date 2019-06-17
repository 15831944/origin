/** =============================================================================
 ** Copyright (c) 2003 WaveTop Information Corp. All rights reserved.
 **
 ** The Mirror system
 **
 ** =============================================================================
 */

#ifndef _LIBMIRORL_TYPES_H_
#define _LIBMIRORL_TYPES_H_ 1

#include "oci.h"

typedef struct _MiOraDBTables {
    char szTableName[36];
    char szOwner[36];

    _MiOraDBTables *next;
} MiOraDBTables;

typedef struct _MiOraDBMarker {
    char szOraVer[512];
    char szDBID[128];
    char szDBName[32];
} MiOraDBMarker;

typedef struct _MiOraDBInfo {
    char szDicFilePath[1024];

    char szZeroIsAutoLog[1024];
    char szZeroIsArchiveMod[1024];
} MiOraDBInfo;

typedef struct _MiOraDBMarkerList {
    MiOraDBMarker DBMark;
    struct _MiOraDBMarkerList *pNext;
} MiOraDBMarkerList;

/* oracle access handle */
typedef struct MiOrl {
    OCIEnv *pEnv;
    OCIError *pError;
    OCIServer *pServer;
    OCISvcCtx *pSvcCtx;
    OCISession *pSession;
    int nZeroIsNeedLogon;
    /* last oci error code */
    sword nOCILastError;
} MiOrl;

typedef struct _MiOrlStrList {
    char *pszData;
    struct _MiOrlStrList *pNext;
} MiOrlStrList;

typedef struct _MiOrlTableList {
    char *pszTableName;
    char *pszTableOwner;
    struct _MiOrlTableList *pNext;
} MiOrlTableList;

typedef int MiOrlLoginType;
#define WT_MI_ORL_LOG_NORMAL        0
#define WT_MI_ORL_LOG_SYSDBA        1
#define WT_MI_ORL_LOG_SYSOPER       2

typedef struct _MiOrlService {
    char szService[512];
    char szUser[512];
    char szPsw[512];
    MiOrlLoginType nLoginType;
} MiOrlService;

typedef struct _MiOrlSessionList {
    MiOrlService OrlSession;
    struct _MiOrlSessionList *pNext;
} MiOrlSessionList;

typedef struct _MiOrlSessionListEx {
    MiOrlService OrlSession;
    MiOraDBMarker DBMark;
    struct _MiOrlSessionListEx *pNext;
} MiOrlSessionListEx;

/**
 * @defgroup export functions type
 * @{
 */

typedef struct _MiOrlExpOpt MiOrlExpOpt;

typedef void (*MiOrlExpException)(MiOrlExpOpt *pExpOpt, int nStatus, char *pszMsg);

struct _MiOrlExpOpt {
    char szExpToInfoFile[512];
    char szExpToDataFile[512];
    char szExpTempFile[512];
    char szExpLogFile[512];
	char szTables[512];
	char szOwners[512];
	char szToUser[128];
	char szCharSet[64];
    char *pszQuery;
	char *pOwner;
    PRInt16 nIsFull;  
    MiOrlStrList *pTablespaces; 
	MiOrlStrList *pTables;
    PRInt16 nIsCompress;
    PRInt16 nIsDirect;
    PRInt16 nIsConGrants;
    PRInt16 nIsConIndexes;
	PRInt16 nIsIgnore;
	PRInt16 nIsCommit;
	PRInt16 nIsDestroy;
	PRInt16 nIsRowsConstraints;
	PRInt16 nIsRows;
    unsigned long nBufSize;   
    long nExitCode; 
    long nMioraModeType;  
    MiOrlExpException pException;
};

/** @} */

/**
 * @defgroup import functions type
 * @{
 */
typedef struct _MiOrlImpOpt MiOrlImpOpt;

typedef void (*MiOrlImpException)(MiOrlImpOpt *pImpOpt, int nStatus, char *pszMsg);  

struct _MiOrlImpOpt {
    char szImpFromInfoFile[512];
    char szImpFromDataFile[512];
    char szImpTempFile[512];
    char szFromUser[128];
    char szToUser[128];
    char szLogFile[512];
    char szCharSet[512];
	char szTables[1024];
    MiOrlTableList *pTablesInfo;
    MiOrlStrList *pUserName;
    PRInt16 nMioraModeType;
	PRInt16 nIsFull;
    PRInt16 nIsIgnore;
	PRInt16 nIsCommit;
	PRInt16 nIsDestroy;
    PRInt16 nIsConGrants;
    PRInt16 nIsConIndexes;
    PRInt16 nIsRowsInput;
    int nZeroIsCommitImmediate;
    unsigned long nBufSize;
    long nExitCode;
    MiOrlImpException pException;
};

/** @} */

/**
 * @defgroup redo log dump data struct
 * @{
 */

typedef struct _MiOrlRedoLogDumpOpt MiOrlRedoLogDumpOpt;
typedef struct _MiOrlRedoFileDump MiOrlRedoFileDump;

/* oracle redo log type */
typedef long MiOrlRedoLogTypes;
/* oracle archive redo log */
#define WT_MI_ORL_LOGMNR_ARCH_REDOLOG       (1<<0)
/* oracle onlien redo log */
#define WT_MI_ORL_LOGMNR_ONLINE_REDOLOG     (1<<1)

/* redo log dump status */
typedef long MiOrlRedoLogStatus;
#define WT_MI_REDO_LOG_STATUS_INIT          0
#define WT_MI_REDO_LOG_STATUS_DUMP          1
#define WT_MI_REDO_LOG_STATUS_FAILED        2

/* online log group description */
typedef long MiOrlRedoLogOnlineLogGroup;
#define WT_MI_REDO_LOG_ONLINE_GROUP_ALL         0
#define WT_MI_REDO_LOG_ONLINE_GROUP_ACTIVE      (1<<30)
#define WT_MI_REDO_LOG_ONLINE_GROUP_INACTIVE    (1<<29)

/* start logmnr opt 
 * @see $(ORACLE_HOME)/rdbms/admin/dbmslm.sql
 */
typedef long MiOrlRedoLogStartLogMnrOpt;
#define WT_MI_REDO_LOG_STARTLOGMNROPT_DEFAULT                       0
#define WT_MI_REDO_LOG_STARTLOGMNROPT_NO_DICT_RESET_ONSELECT        (1<<0)
#define WT_MI_REDO_LOG_STARTLOGMNROPT_COMMITTED_DATA_ONLY           (1<<1)
#define WT_MI_REDO_LOG_STARTLOGMNROPT_SKIP_CORRUPTION               (1<<2)
#define WT_MI_REDO_LOG_STARTLOGMNROPT_DDL_DICT_TRACKING             (1<<3)
#define WT_MI_REDO_LOG_STARTLOGMNROPT_DICT_FROM_ONLINE_CATALOG      (1<<4)
#define WT_MI_REDO_LOG_STARTLOGMNROPT_DICT_FROM_REDO_LOGS           (1<<5)
#define WT_MI_REDO_LOG_STARTLOGMNROPT_NO_SQL_DELIMITER              (1<<6)

typedef void (*MiOrlOCIFailedCallBack)(MiOrlService *pOrlSession, 
                                       sword nOCIStatus, char *pszOCIErrorMsg);

typedef struct MiOrlRedoLogLCRResult {
    char *pszUndo;
} MiOrlRedoLogLCRResult;

typedef int (*MiOrlRedoLogLCR)(MiOrlRedoLogDumpOpt *pOpt, 
                               MiOrlRedoFileDump *pRedoFileDump, 
                               MiOrlRedoLogLCRResult *pLCRResult);

typedef int (*MiOrlRedoLogLCRFree)(MiOrlRedoLogDumpOpt *pOpt, 
                                   MiOrlRedoFileDump *pRedoFileDump, 
                                   MiOrlRedoLogLCRResult *pLCRResult);

struct _MiOrlRedoFileDump {    
    /* query control option */
    long nZeroIsContinue;
    long nZeroIsSCNSwitch;
    void *pCallbackData;
    unsigned long nSelectRowId;

    /* analysis redo log file name */
    char *pszRedoLogFilename;

    /* analysis redo log file type (online or archive) */
    MiOrlRedoLogTypes nLogType;

    /* redo sql command */
    char *pszSqlRedo;

    /* undo sql command 
     *
     * FIXME current version library not 
     *       set pszSqlUndo SQL to caller
     *
     */
    char *pszSqlUndo;

    /* the system change number */
    OCINumber *pOCINumSCN;

    /* the system change number string */
    char szOCINumSCN[128];

    /* redo log time stamp */
    OCIDate *pOCITimeStamp;

    /* redo log time stamp string */
    char szTimeStamp[512];

    /* rollback flag */
    unsigned long nRollback;

    /* user name */
    char *pszUserName;

    /* SEG owner condition string */
    char *pszSEGOwner;

    /* SEG name condition string */
    char *pszSEGName;

    /* SEG type condition string */
    char *pszSEGType;

    /* SEG type condition code */
    unsigned long nSEGType;

    /* sql operation string */
    char szOperation[64];

    /* sql operation code */
    unsigned long nOperationCode;

    /* affect database row id */
    char szROW_ID[32];

    /* The transaction ID slot number */
    unsigned long nXIDSLT;

    /* The transaction ID log sequence number */
    unsigned long nXIDSQN;

    char szTaskName[256];
};

typedef struct _MiOrlLogMnrCond {
    char szCurrentSCN[256];
    char szCurrentTime[256];
    char szTaskName[256];
    char szMirSCNTabName[256];
    char szTabOwner[256];
} MiOrlLogMnrCond;

typedef struct _MiOrlExpDump {
    long nZeroIsContinue;
    void *pCallbackData;
    char *pszExpFilename;
    unsigned long nSelectRowId;
    unsigned long nRollback;
    char szROW_ID[32];
} MiOrlExpFileDump;

/* LogMnr dump callback function define */
typedef int (*MiOrlRedoLogDumpFunction)(MiOrlRedoLogDumpOpt *pOpt,
                                        MiOrlRedoFileDump *pDump);

/* archive redo log file description */
typedef struct _MI_ORL_REDO_LOG_FILE {
    /* redo log file name */
    char szLogFilename[512];

    /* log first time */
    OCIDate OCIFirstTime;

    /* log next time */
    OCIDate OCINextTime;

    /* log status 
     * WT_MI_REDO_LOG_STATUS_INIT or WT_MI_REDO_LOG_STATUS_DUMP or WT_MI_REDO_LOG_STATUS_FAILED
     */
    MiOrlRedoLogStatus nStatus;

    /* log first scn */
    OCINumber OCINumSCNFirst;
    double nSCNFirst;

    /* log next scn */
    OCINumber OCINumSCNNext;
    double nSCNNext;

    /* log type archive or online */
    MiOrlRedoLogTypes nLogType;

    union {
        struct {
            /* online log grop id */
            unsigned long nGroupId;
        } Online;
        struct {
            /* archive log dump dest id 
             *
             * FIXME the value can't parse in oracle8i
             */
            unsigned long nDestId;
        } Archive;
    } RedoLogField;
} MiOrlRedoLogFile;

/* archive redo log file description list */
typedef struct _MI_ORL_REDO_LOG_FILES {
    MiOrlRedoLogFile RedoLogFileDesc;
    struct _MI_ORL_REDO_LOG_FILES *pNext;
} MiOrlRedoLogFiles;

/* oracle redo log dump options */
struct _MiOrlRedoLogDumpOpt {
    /* oracle major version */
    unsigned long nOraMajorVer;

    /* using malloc and realloc for deal sql ring  */
    unsigned long nZeroIsAllocRedo;

    /* database dict file name */
    char szDictFilename[512];

    /* if nZeroIsNeedReCreateDbDict == 0 then recreate database dict file */
    long nZeroIsNeedReCreateDbDict;

    /* start parse scn number string */
    char szOCISCNNumStart[128];

    /* end parse scn number string */
    char szOCISCNNumEnd[128];

    /* select sql_redo from v$logmnr_contents Condition string 
     * example:
     *      szQueryCondition = "LOWER(username)='sys'"
     * sql command == "select sql_redo ... from v$logmnr_contents where username='sys'"
     */
    char szQueryCondition[4096];

    /* parse oracle redo log types see MiOrlRedoLogTypes */
    MiOrlRedoLogTypes nParseRedoLogTypes;

    /* LogMnr start Options */
    MiOrlRedoLogStartLogMnrOpt nLogMnrOpts;

    /* if nZeroIsFailedContinue != 0 one log file failed 
     * then this functions terminate 
     */
    long nZeroIsFailedContinue;

    /* oci failed callback function */
    MiOrlOCIFailedCallBack pOCIFailedFunc;

    /* file dump callback */
    MiOrlRedoLogDumpFunction pRedoLogDumpFunc;

    /* file dump callback data */
    void *pFileDumpCallBackData;

    /* dump online log groups */
    MiOrlRedoLogOnlineLogGroup nDumpOnlineLogGroups;

    /* must be NULL, this field is for 
     * internal transfer data
     */
    MiOrlService *pOrlSession;

    /* zero is no terminate */
    long nZeroIsNoTerminate;

    MiOrlLogMnrCond LogMnrCond;

    char szTaskName[256];
};

typedef struct _MiOrlRedoLogDumpResult {
    /* parse archive redo log files */
    MiOrlRedoLogFiles *pRedoLogs;

    /* log first time */
    char szRealOCIFirstTime[128];
    /* log next time */
    char szRealOCINextTime[128];

} MiOrlRedoLogDumpResult;

/** @} */

#endif /* !defined(_LIBMIRORL_TYPES_H_) */
