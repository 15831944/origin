/** =============================================================================
 ** Copyright (c) 2003 WaveTop Information Corp. All rights reserved.
 **
 ** The Mirror system
 **
 ** =============================================================================
 */

/**
 * @desc ms sql mirror library internal functions
 * @author lexiongjia
 * @file mimssqlfunc.h
 */

#ifndef _LIBMIDATABASE_H_
#define _LIBMIDATABASE_H_ 1

#ifdef WIN32
#include "libmimssql_types.h"
#endif

typedef long MiDatabaseServType;
#define WT_MI_DBTYPE_MSSQL       1
#define WT_MI_DBTYPE_ORACLE      2
#define WT_MI_DBTYPE_UNKNOWN    -1

typedef int MiDbPolicy;
#define WT_MI_DB_POLICY_ALLFULL     1

/** database mirror base info */
typedef int MiDbStoreMode;
#define WT_MI_DB_STORE_MODE_FS       (1<<0)
#define WT_MI_DB_STORE_MODE_DBSERV   (1<<1)
typedef struct _MiDatabaseRecord {
    char szMachineName[255];            /**< mirror database server machine */
    MiDatabaseServType nDBServType;     /**< mirror database server type */
    char szDBServVersion[32];           /**< mirror database server version */
    char szDatabaseName[128];           /**< mirror database name */
    long nMirrorVersion;                /**< database mirror version */
    long nMirrorTime;                   /**< database mirror time */
    MiDbStoreMode nMirrorMode;          /**< database stroe mode */
    char szReserve[32];                 /**< reserve field */
} MiDatabaseRecord;

/** database mirror base info list */
typedef struct _MiDatabaseRecordList {
    MiDatabaseRecord stRecord;
    struct _MiDatabaseRecordList *pNext;
} MiDatabaseRecordList;

#ifdef WIN32
/** database mirror speciality info mssql */
typedef struct _MiDatabaseRecordInfoMSSql {
    unsigned short nStreamCount;        /**< mssql data stream count */
    char szOptDBName[128];              /**< mirror mssql database name */
    MiMSSqlExportMode nBkMode;          /**< mssql export mode */
    MiMSSqlOptRunMode nRunMode;         /**< mssql export run mode */
    char szVirDevPrefix[128];           /**< mssql data stream prefix string */    
    int nBlocakSize;                    /**< storage block size */
    int nBufferCount;
    int nTransferSize;
    int nFetchMode;
    char szReserve[32];                /**< reserve field */
} MiDatabaseRecordInfoMSSql;
#endif

typedef struct _MiDatabaseRecordInfoOracle {
    /* XXX oracle database mirror operation */
} MiDatabaseRecordInfoOracle;

/** total database mirror info */
typedef struct _MiDatabaseRecordInfo {
    MiDatabaseRecord stDbRecord;
    void *pData;                    /**< point to database mirror speciality info mssql
                                         MiDatabaseRecordInfoMSSql or 
                                         MiDatabaseRecordInfoOracle or other */
} MiDatabaseRecordInfo;

/** database mirror info list */
typedef struct _MiDatabaseRecordInfoList {
    MiDatabaseRecordInfo *pInfo;
    struct _MiDatabaseRecordInfoList *pNext;
} MiDatabaseRecordInfoList;

/** store file info list */
typedef struct _MiDatabaseStoreFileInfo {
    char szStoreFile[512];                  /**< physics file name on server */
    char szStreamFile[512];                 /**< stream file name from client */ 
	char szReserve[32];						/**< reserve field */
    struct _MiDatabaseStoreFileInfo *pNext;
} MiDatabaseStoreFileInfo;

/** store file recorder */
typedef struct _MiDatabaseRecordStoreFile {
    MiDatabaseRecord stDbRecord;
    MiDatabaseStoreFileInfo *pStoreFileInfo;
} MiDatabaseRecordStoreFile;

/** delete file recorder */
typedef struct _MirrorDbStoreDeleteInfo {   
	char *pszStoreDir;                          
    MiDatabaseRecordInfo *pAppendRecord;      /**< database record info */
} MirrorDbStoreDeleteInfo;

#endif /* !defined(_LIBMIDATABASE_H_) */
