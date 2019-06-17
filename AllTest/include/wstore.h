/** =============================================================================
 ** Copyright (c) 2003 WaveTop Information Corp. All rights reserved.
 **
 ** The backup system
 **
 ** =============================================================================
 */

/* Storage interface definition */

#ifndef __BACKUP_STORE_H_
#define __BACKUP_STORE_H_ 1

/* the 4 GB - 1 MB Archive file size */
#define WSTORE_MAX_ARCHIVE_SIZE       9007199254740992
/* #define WSTORE_MAX_ARCHIVE_SIZE        (1024*1024*16) */

#define WSTORE_STORE_FILE           "store.sto"
#define WSTORE_ARCHIVE_FILE         "archive"
#define WSTORE_ARCHIVE_FILE_NAME    "datafile"
#define WSTORE_STORE_LOCK_FILE      "store.lo"
#define WSTORE_INDEX_DBM            "store.idx"

#define NAME_HANDLE_ARCHIVE         "ArchiveHandle"
#define NAME_HANDLE_STORE           "StoreHandle"
#define NAME_HANDLE_USER_LOCK       "UserLockHandle"

/* The store cache name */
#define WSTORE_NAME_HANDLE_STORE    "StoreCache"

/* Archive */
#define ARCHIVE_SHORT_FILENAME         (1<<0)

/* the dbm lock */
#define WSTORE_RD_LOCK 1
#define WSTORE_WR_LOCK 2

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

/* Archive file options */
#define BACKUP_ARCHIVE_APPEND2      1
#define BACKUP_ARCHIVE_NEW_BLOCK    2
#define BACKUP_ARCHIVE_WRITE_DATA   3
#define BACKUP_ARCHIVE_FLUSH        4
#define BACKUP_ARCHIVE_READ_BLOCK   5
#define BACKUP_ARCHIVE_READ_DATA    6

#define BACKUP_BASE_RECORD_SIZE (sizeof(short) + sizeof(unsigned long) + \
    sizeof(long) + sizeof(unsigned long) + sizeof(long) + sizeof(long) + \
    sizeof(long) + sizeof(unsigned long) + sizeof(char) + sizeof(long))

#define BACKUP_VER_RECORD_SIZE (sizeof(unsigned long) + sizeof(unsigned long) +\
    sizeof(unsigned long) + sizeof(long) + sizeof(char) + \
    sizeof(unsigned long) + sizeof(unsigned long) + sizeof(short))
    

/* The store type */
#define BACKUP_STORE_BASE      1
#define BACKUP_STORE_VERSION   2

/* The node use flags */
#define BACKUP_NODE_FREE       1
#define BACKUP_NODE_USED       2
#define BACKUP_NODE_UNUSED     3

/* The Relation node type */
#define BACKUP_DATA_RELATION_NODE     (1<<0)
#define BACKUP_FILE_RELATION_NODE     (1<<1)

/* The base information structure of file node */
typedef struct BkFileBaseSt {
    /* The source file name before backup(on client) */
    char *pszSourceName;
    /* The first backup time */
    unsigned long nCreateTime;
    /* The current file size */
    PRInt64 nFileSize;
    /* The current file version */
    unsigned long nCurrentVer;
    /* The owner */
    long nUserID;
    /* The current backup time */
    unsigned long nBackupTime;
    long nFileType;
    unsigned long nFlags;

    unsigned long nArchiveNo;
    PRInt64 nArcStartPos;
    PRInt64 nArcEndPos;
	mode_t mode;
	uid_t uid;
	gid_t gid;
    struct BkFileBaseSt *next;
} BkFileBaseSt;

/* The Version information structure or file node */
typedef struct BkFileVersionSt {
    unsigned long nBackupTime;
    unsigned long nCreateTime;
    unsigned long nModifyTime;
    PRInt64 nFileSize;
    unsigned long nVersion;

    /* The backup type */
    unsigned long nBackupType;

    /* The source file name before backup */
    char *pszSourceName;
    PRInt64 nDeltaSize;
    unsigned long nAttribute;
    /* The check-in message */
    char *pszMessage;

    /* This node has data relation or file relation 
     * If (nRelationType & BACKUP_DATA_RELATION_NODE)
     * the node has another data filesets belong to this file.
     * If (nRelationType & BACKUP_FILE_RELATION_NODE)
     * the node has another files belong to this backup.
     */
    char nRelationType;
    unsigned long nDataRelationRow;
    unsigned long nDataRelationCol;
    unsigned long nFileRelationRow;
    unsigned long nFileRelationCol;

    /* The MSSql database version number */
    unsigned long nReserve1;

    /* The reserve buffer */
    char szBinary[80];

    struct BkFileVersionSt *next;
} BkFileVersionSt;

typedef struct BkFileStoreNodeSt {
    /* The store type: base or version */
    char nStoreType;
    union StoreInfo {
        BkFileBaseSt Base;
        BkFileVersionSt Version;
    };

    /* The store archive file name */
    char *pszArchive;
    PRInt64 nStartPos;
    PRInt64 nEndPos;

    /* The use flags: free, used unused */
    char nUseFlags;

    /* Flags: encrypted, compressed */
    char nStoreFlags;
} BkFileStoreNodeSt;

typedef struct BkStoreFileNameSt {
    char *pszSourceName;
    char *pszStoreName;
    PRInt64 nFileSize;
    int nStatus;
    struct BkStoreFileNameSt *next;
} BkStoreFileNameSt;

/* typedef struct ArchiveHandleSt * ArchiveHandle; */

/* The store handle */
typedef struct _BK_STORE_HANDLE_ST BkStoreHandleSt;

/* The storing cache handle */
typedef struct _BK_STORE_CACHE_ST BkStoreCacheSt;

/* The Open archive flags */
#define BACKUP_OPEN_WITH_BACKUP     (1<<0)
#define BACKUP_RELATIVE_FILE_NAME   (1<<1)

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// The base and version information API                                       //
// All the pReq is the request of a ACTION.                                                                           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
 
/* Insert a new base file into the store file DB.
 * The version is always the first version. So, when do the first 
 * backup, Call this function to insert a backup file.
 * @[in] param
 * pBkFile is the base information of a file.
 * pBkVerFile is the version information of a file.
 * @[out] param
 * Returns WSTORE if insert is successful; otherwise return error code.
 */
API_EXPORT(int)  BkInsertBaseFile(request_rec *pReq, 
                      BkFileBaseSt *pBkFile,
                      BkFileVersionSt *pBkVerFile);

/* Modify the base information.
 * @[in] param
 * pBkFile is the base information of a file.
 * @[out] param
 * Returns WSTORE if modify is successful; otherwise return error code.
 */
API_EXPORT(int)  BkModifyBaseFile(request_rec *pReq, 
                      BkFileBaseSt *pBkFile);

/* Modify the version information.
 * @[in] param
 * pBkVerFile is the version information of a file.
 * @[out] param
 * Returns WSTORE if modify is successful; otherwise return error code.
 */
API_EXPORT(int)  BkModifyVerFile(request_rec *pReq,
                      BkFileVersionSt *pBkVerFile);

/* Stat the base information of a file .
 * @[in] param
 * pszFileName is the Statistics file.
 * @[out] param
 * pBkFile is statistics result.
 * Return value is WAVETOP_BACKUP_OK, when the base
 * file is exist. otherwise, return error code.
 */
API_EXPORT(int)  BkStatBaseFile(request_rec *pReq, 
                    const char *pszFileName, 
                    BkFileBaseSt *pBkFile);

/* Delete the file by the source file name.
 * @[in] param
 * pszFileName is the deleteing file.
 * @[out] param
 * Return value is WAVETOP_BACKUP_OK, when the base
 * file is successfully deleted. otherwise, return error code.
 */
API_EXPORT(int)  BkDeleteBaseFileBySource(request_rec *pReq, 
                              const char *pszFileName);

/* Delete store node by policy node list
 *
 * @[in] param
 * pReq - request
 * pszPolicyFile - policy file name (Policy/ComputerName/File)
 * pList - policy node list (nodes to delete) 
 *
 * @[out] param
 * pnFreeSize - point to free space
 *
 * Return value is WSTORE_OK, when the base
 * file is successfully deleted. otherwise, return error code.
 */
API_EXPORT(int) BkDeleteStoreNodeByPolicy(request_rec *pReq,
                              const char *pszPolicyFile,
                              PolicySt *pList,
                              PRInt64 *pnFreeSize);

/* Recover store node by policy node list
 *
 * @[in] param
 * pReq - request
 * pszPolicyFile - policy file name (Policy/ComputerName/File)
 * pList - policy node list (nodes to recover)
 *
 * @[out] param
 * pnFreeSize - point to free space (the space to recover)
 *
 * Return value is WSTORE_OK, when the base
 * file is successfully deleted. otherwise, return error code.
 */
API_EXPORT(int) BkRecoverStoreNodeByPolicy(request_rec *pReq,
                               const char *pszPolicyFile,
                               PolicySt *pList,
                               PRInt64 *pnFreeSize);

/* Delete file version information
 *
 * @[in] param
 * pReq - request
 * pszDeleteFile - policy file for delete
 * nDelVer - delete version (from 1 to nDelVer)
 *
 * @[out] param
 * pnFreeSize - size to free
 */
API_EXPORT(int) BkStoreDeleteVersion(request_rec *pReq,
                         const char *pszDeleteFile,
                         const unsigned long nDelVer,
                         PRInt64 *pnFreeSize);

/* Query base info node col and row
 *
 * @[in] param
 * pReq - request
 * pszFileName - file name
 * nSizeCol - length of pszCol
 * nSizeRow - length of pszRow
 * 
 * @[out]
 * pszCol - base node col
 * pszRow - base node row
 */
API_EXPORT(int) BkQueryFileBasePosition(request_rec *pReq, 
                            const char *pszFileName, 
                            char *pszCol,
                            int nSizeCol,
                            char *pszRow,
                            int nSizeRow);

/* Query file last version info node col and row
 *
 * @[in] param
 * pReq - request
 * pszFileName - file name
 * nLastVersion - file last version
 * nSizeCol - length of pszCol
 * nSizeRow - length of pszRow
 *
 * @[out] param
 * pszCol - last version node col
 * pszRow - last version node row
 */
API_EXPORT(int) BkQueryFileLastVerPos(request_rec *pReq, 
                          const char *pszFileName, 
                          unsigned long nLastVersion,
                          char *pszCol,
                          int nSizeCol,
                          char *pszRow,
                          int nSizeRow);

/* Below is the version information APIs.
 *
 * Insert a new version file. When backup a new version, call this function.
 * @[in]
 * pszFileName is the backup file.
 * pBkFileVer is the version information of a file.
 * @[out]
 * Return value is WAVETOP_BACKUP_OK, when the version information
 * is successfully inserted. otherwise, return error code.
 */
API_EXPORT(int)  BkInsertNewVerFile(request_rec *pReq,
                        const char *pszFileName,
                        BkFileVersionSt *pBkFileVer);

/* Query the all versions of a file.
 * @[in]
 * pszFileName is the backup file.
 * nVersionNum is the number of versions. If is a positive number, 
 * increasing order. If is a negative number, descending order.
 * If equal to 0, no limitation. 
 * @[out]
 * Return value is WAVETOP_BACKUP_OK, when the base
 * file is successfully deleted. otherwise, return error code.
 * When successful, call pResultCB to deliver the backup file information.
 */ 
API_EXPORT(int)  BkQueryVerFile(request_rec *pReq,
                    const char *pszFileName,
                    int nVersionNum,
                    BkFileVersionSt **pFileVer);

/* Stat the version inforation of a file.
 * @[in] param
 * pszVerFile is the Statistics file.
 * @[out] param
 * pBkFileVer is statistics result.
 * Return value is WAVETOP_BACKUP_OK, when the version
 * is exist. otherwise, return error code.
 */
API_EXPORT(int)  BkStatVerFile(request_rec *pReq,
                   const char *pszVerFile,
                   BkFileVersionSt *pBkFileVer);

/* Insert a directory.
 * @[in]
 * pBkDir is the directory.
 * @[out] param
 * Return value is WAVETOP_BACKUP_OK, when the directory
 * is successfully inserted. otherwise, return error code.
 */
API_EXPORT(int)  BkInsertDirectory(request_rec *pReq, BkFileBaseSt *pBkDir);

/* Delete a directory and all children.
 * @[in]
 * pszDirName is a deleting directory.
 * @[out] param
 * Return value is WAVETOP_BACKUP_OK, when the file is
 * successfully deleted. otherwise, return error code.
 */
API_EXPORT(int)  BkDeleteBackupDir(request_rec *pReq,
                      const char *pszDirName);

/* Travel the all sub-directories and files under
 * the specified directory.
 * @[in]
 * pszDirName is a traveling directory.
 * @[out] param
 * Return value is WAVETOP_BACKUP_OK, when the file is
 * successfully deleted. otherwise, return error code.
 */
API_EXPORT(int)  BkTravelBaseDir(request_rec *pReq,
                     const char *pszDirName,
                     BkFileBaseSt **pFileBase);

/* Get the user backuping root directory
 * @[in]
 * pResultCB is the callback function.
 * pArg is a favorite option.
 * @[out]
 * Return value is WAVETOP_BACKUP_OK, when the base
 * file is successfully deleted. otherwise, return error code.
 * When successful, call pResultCB to deliver the backup file information.
 */ 
API_EXPORT(int)  BkQueryUserRootDir(request_rec *pReq,
                    BkFileBaseSt **pFileBase);

/* Rollback the file when operation is failed.
 * @[in]
 * pszFile is the backuping file
 * nFiletype is this file type
 * nRolltype is the rollback type. 1 is that rollback the max version,
 * and 2 is that remove this file backup.
 * @[out]
 * Return value is WAVETOP_BACKUP_OK, when the base
 * file is successfully rollbacked. otherwise, return error code.
 */
API_EXPORT(int)  BkStoreRollBackFile(request_rec *pReq, const char *pszFile,
                                     int nFiletype, int nRolltype, PRInt64 nSize = 0);

/* Extract files from the store database.
 * @[in]
 * pNameList is the files list which need extracting.
 * @[out]
 * If a file has been appended failed, its status is error code.
 * Return value is WAVETOP_BACKUP_OK, when the base
 * file is successfully deleted. otherwise, return error code.
 */

/*
 * lexiongjia define for read archive data use callback 
 * nPiece == 0  the first read
 * nPiece == -1 the file read end
 */
typedef int (*BkArcFileReadCallback)(void *pCtx, char *pData, 
            unsigned long nDataOffset, unsigned long nDataLen, 
            int nPiece, int nStoreStatus);

API_EXPORT(int)  BkArchiveExtract(request_rec *pReq, 
                    BkStoreFileNameSt *pNameList,
                    const char *pszSavePath, void *pCtx, 
                    BkArcFileReadCallback pCbFunc);

/* Construct the user store cache system */
API_EXPORT(int)  BkContrustUserStoreCache(request_rec *pReq);

/* this method is only used for Policy restore */
API_EXPORT(int) BkQueryBaseFileByNode(request_rec *pReq,
                              BkFileBaseSt *pFileBase,
                              PolicySt *pNode,
                              unsigned long *nVer);

/* Clean the store node.
 * @[in]
 * StoreType The store type: base or version.
 * @[out]
 * Return value is WAVETOP_BACKUP_OK, when the store
 * node is successfully clean. otherwise, return error code.
 */
API_EXPORT(int) BkCleanStoreNode(request_rec *pReq, char StoreType);

/* Modify the store node.
 * @[in]
 * pszFileName : Modify node file name.
 * @[out]
 * Return value is WAVETOP_BACKUP_OK, when the store
 * node is successfully clean. otherwise, return error code.
 */
API_EXPORT(int) BkModifyStoreVersionNode(request_rec *pReq, char *pszFileName, 
                                         PRInt64 nStartPos);

/* Clear Store Medium when there is no enough space to use 
 * @[in]
 * ArchiveFileNo The ArchiveFile Num: 23
 * @[out]
 * Return value is WAVETOP_BACKUP_OK, when the store clean 
 * is successfully. otherwise, return error code.
 * for backup which use BACKUP_WEBUI_COVER_WHILE_NOSPACE method
 */
API_EXPORT(int) BKCleanStoreMedium(request_rec *pReq, int nArchiveFileNo, 
                                   PRInt64 *pnCleanSize);
API_EXPORT(int) BKCleanVersionArchive(request_rec *pReq, int nArchiveFileNo, 
                                      PRInt64 *pnCleanSize);

/* clean Archive Occupant */
API_EXPORT(int) BKCleanArchiveOccupant(request_rec *pReq, int nArchiveNo);
/* new add */
API_EXPORT(int) BkStoreFileSearchPolicy(request_rec *pReq, char *pszFilename, 
                                        unsigned long *nVersion);
API_EXPORT(int) BkGetArchiveFiles(request_rec *pReq, const char* pszSchedule, 
                                  unsigned long nDelVersion, unsigned long nObjectType, 
                                  unsigned long **ArchiveFilesId, int *nArchiveNumber);
API_EXPORT(int) BKModifyBaseNode(request_rec *pReq, char *pszFileName, 
                                 unsigned long nArchiveNo);
API_EXPORT(int) DelAllArchiveFiles(request_rec *pReq, char *pszFilename, 
                                   PRInt64 *pnCleanSize);
API_EXPORT(int) TerminalBackCleanStoreNode(request_rec *pReq, char *pszFileName,
                                           PRInt64 &nUserUsing, char StoreType);
API_EXPORT(int) TerminalBackCleanTree(request_rec *pReq, char *pszFileName);
API_EXPORT(int) BkPolicyBaseNodeAppend(request_rec *pReq, char *pszPolicyFile);

/* ftp */
API_EXPORT(int) BkFtpStoreFile(request_rec *pReq, const char *szFile);
API_EXPORT(int) CleanIndex(request_rec *pReq, char *pszFilename);

#ifdef __cplusplus
}
#endif

/* The acl information */
typedef struct BkFileBaseSt BkAclInfoSt;

#endif /* #define __BACKUP_STORE_H_ 1 */
