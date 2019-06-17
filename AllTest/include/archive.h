/** =============================================================================
 ** Copyright (c) 2003 WaveTop Information Corp. All rights reserved.
 **
 ** The backup system
 **
 ** =============================================================================
 */

/* define Archive storing format */

#ifndef __BACKUP_ARCHIVE_H_
#define __BACKUP_ARCHIVE_H_ 1

#include "nspr.h"
#include "dbinfo.h"
#include "alg_mdfour.h"
#include "alg_rsync.h"

/* the new archive file version */
#define BACKUP_ARCHIVE_VERSION          1001
#define BACKUP_ARCHIVE_BLOCK_SIZE       1024
#define ARCHIVE_FILE_BLOCK_SIZE         (1<<13)

/* the new store file version */
#define BACKUP_STORE_FILE_VERSION       1001

/* The store index matrix column count */
/**
 * #define BACKUP_STORE_COLUMN_COUNT       32
 **/

/* The store verified node count */
#define BACKUP_STORE_VERIFY_NODE_COUNT  8

/* The archive file max number */
#define BACKUP_ARCHIVE_MAX_FILE         256

/* The store action */
#define BACKUP_STORE_ACTION_ADD         1
#define BACKUP_STORE_ACTION_MODIFY      2
#define BACKUP_STORE_ACTION_DELETE      3

#define BACKUP_STORE_SAME_FLAG          "0"
#define BACKUP_STORE_DIFF_FLAG          "1"

#undef MD5_SIZE
#ifndef MD5_SIZE
#define MD5_SIZE 16
#endif

#define BACKUP_STORE_FILE_NAME_LEN      1024

#define TMAGIC   "archive"  /* archive and a null */
#define TMAGLEN  8

/* Values used in typeflag field.  */
#define REGTYPE  '0'        /* regular file */
#define AREGTYPE '\0'       /* regular file */
#define LNKTYPE  '1'        /* link */
#define SYMTYPE  '2'        /* reserved */
#define CHRTYPE  '3'        /* character special */
#define BLKTYPE  '4'        /* block special */
#define DIRTYPE  '5'        /* directory */
#define FIFOTYPE '6'        /* FIFO special */
#define CONTTYPE '7'        /* reserved */

/* Bits used in the mode field, values in octal.  */
#define TSUID    04000      /* set UID on execution */
#define TSGID    02000      /* set GID on execution */
#define TSVTX    01000      /* reserved */
/* file permissions */
#define TUREAD   00400      /* read by owner */
#define TUWRITE  00200      /* write by owner */
#define TUEXEC   00100      /* execute/search by owner */
#define TGREAD   00040      /* read by group */
#define TGWRITE  00020      /* write by group */
#define TGEXEC   00010      /* execute/search by group */
#define TOREAD   00004      /* read by other */
#define TOWRITE  00002      /* write by other */
#define TOEXEC   00001      /* execute/search by other */

#define BLOCK_PAD     '\0'

/* the integer byte-order */
#define XINT64(n,p) {\
    (p)[0]=(unsigned char)(((n)>>56)&0xff);\
    (p)[1]=(unsigned char)(((n)>>48)&0xff);\
    (p)[2]=(unsigned char)(((n)>>40)&0xff);\
    (p)[3]=(unsigned char)(((n)>>32)&0xff);\
    (p)[4]=(unsigned char)(((n)>>24)&0xff);\
    (p)[5]=(unsigned char)(((n)>>16)&0xff);\
    (p)[6]=(unsigned char)(((n)>>8)&0xff);\
    (p)[7]=(unsigned char)( (n)&0xff);\
}
#define VINT64(n,p) {\
    PRUint32 h; \
    PRUint32 l; \
    PRInt64  u; \
    /* high 32-bit */ \
    h = (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3]; \
    /* low 32-bit */ \
    l = (p[4] << 24) + (p[5] << 16) + (p[6] << 8) + p[7]; \
    LL_UI2L(u, h); \
    LL_SHL(n, u, 32); \
    LL_UI2L(u, l); \
    LL_ADD(n, n, u); \
}
/* the integer byte-order */
#define XINT32(n,p) {\
    (p)[0]=(unsigned char)(((n)>>24)&0xff);\
    (p)[1]=(unsigned char)(((n)>>16)&0xff);\
    (p)[2]=(unsigned char)(((n)>>8)&0xff);\
    (p)[3]=(unsigned char)( (n)&0xff);\
}
#define VINT32(n,p) {\
    n= (((unsigned char)((p)[0])) << 24) + \
       (((unsigned char)((p)[1])) << 16) + \
       (((unsigned char)((p)[2])) << 8 ) + \
       (((unsigned char)((p)[3]))      ) ; \
}

#define XINT16(n,p) {\
    (p)[0]=(unsigned char)(((n)>>8)&0xff);\
    (p)[1]=(unsigned char)( (n)&0xff);\
}
#define VINT16(n,p) {\
    n= (((unsigned char)((p)[0])) << 8) + \
       (((unsigned char)((p)[1]))     ) ; \
}

typedef union _BK_CHECKSUM_ST {
    unsigned char szMD5[MD5_SIZE];
    unsigned char Buffer[MD5_SIZE];
} BkChecksumSt;

/* The index tree utilities, includes memory and file.
 * The structure BkIndexNodeSt is in the memory.
 * The structure BkIndexFileNodeSt is in the file.
 */
#define BACKUP_INDEX_STORE_VERSION_STR "1000"
#define BACKUP_INDEX_SAME_DIR    (1<<10)

/* The index node in the memory */
// typedef struct _BK_INDEX_NODE_ST {
//     char *pszFilename;
//     short nFlags;
//     struct _BK_INDEX_NODE_ST *pBrother;
//     struct _BK_INDEX_NODE_ST *pChild;
// } BkIndexNodeSt;

/* The index file storing header */
typedef struct _BK_INDEX_FILE_HEADER {
    unsigned char szMagic[8];
    char szVersion[8];
    char szNodeCount[4];
    unsigned szReserve[8];
} BkIndexFileHeader;

typedef struct _BK_ARCHIVE_END_ST {
    char szFileName[512];
    char szArcStartPos[8];
    char szArcEndPos[8];
    char szTotalSize[8];
}BkArchiveEndSt;

typedef struct _BK_ARCHIVE_ST {
    char szFileName[1024];
    unsigned long nArchiveNo;
    PRInt64 nArcStartPos;
    PRInt64 nArcEndPos;
    PRInt64 nTotalSize;
    struct _BK_ARCHIVE_ST *pNext;
}BkArchiveFileEndSt;

/* The base and version information store header */
typedef struct _BK_STORE_HEADER_ST {
    unsigned char MagicNumber[8];       /* 8  */
    unsigned char Version[4];           /* 12 */
    unsigned char FileCount[4];         /* 16 */

    /* no used 1.0 version here */
    unsigned char HeaderSize[2];        /* 18 */
    unsigned char BlockSize[2];         /* 20 */

    unsigned char ColCount[2];          /* 22 */
    unsigned char RowCount[4];          /* 26 */
    unsigned char MD5NodeCount[2];      /* 28 */
    unsigned char MD5Sum[MD5_SIZE];     /* 28 + 16  */
    char szArchivename[16];             /* 44 + 16  */
    char szUserTag[16];                 /* 60 + 16 */
    char szUnUse[52];                   /* 128 - 76 */
    struct {
        unsigned char ArchiveHighEnd[8];
        unsigned char ArchiveLowEnd[8];
    } ArchiveFile[BACKUP_ARCHIVE_MAX_FILE];
} BkStoreHeaderSt;

/* The base information structure of file node */
typedef struct BkArchiveBaseSt {
    /* The source file name before backup(on client).
     * To save memory, use the dynamic length.
     * char szSourceName[280];
     */
    char *szSourceName;
    /* The first backup time */
    unsigned char szCreateTime[4];
    /* The current file size */
    unsigned char szFileSize[8];
    /* The current file version */
    unsigned char szCurrentVer[4];
    /* The owner */
    char szUserID[4];
    /* The current backup time */
    unsigned char szBackupTime[4];
    char szFileType[4];
    unsigned char szFlags[4];
    char szReserve[16];
} BkArchiveBaseSt;

/* The Version information structure or file node */
typedef struct BkArchiveVersionSt {
    unsigned char szBackupTime[4];
    unsigned char szCreateTime[4];
    unsigned char szModifyTime[4];
    unsigned char szFileSize[8];
    unsigned char szVersion[4];

    /* The backup type */
    char szBackupType[4];

    /* The version file name.
     * To save memory, use the dynamic length.
     * char szBase64[280];
     */
    char *szBase64;
    unsigned char szDeltaSize[8];
    unsigned char szAttribute[4];
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   
    /* The check-in message. use the dynamic length.
     * char szMessage[33];
     */
    char *szMessage;

    /* This node has data relation or file relation 
     * If (nRelationType & BACKUP_DATA_RELATION_NODE)
     * the node has another data filesets belong to this file.
     * If (nRelationType & BACKUP_FILE_RELATION_NODE)
     * the node has another files belong to this backup.
     */
    char nRelationType;
    unsigned char szDataRelationRow[4];
    unsigned char szDataRelationCol[4];
    unsigned char szFileRelationRow[4];
    unsigned char szFileRelationCol[4];

    unsigned char szReserve1[4];
    char szBinary[80];
} BkArchiveVersionSt;

/* The microcopy storing in the archive
 * for the base file missing and damaged.
 */
typedef struct BkArchiveMicroCopySt {
    unsigned char szVersion[4];
    char szName[280];
    char nBackupType;
    char szBackupTime[16];
    unsigned char szIdxno[4];
    unsigned char szFileSize[8];
    unsigned char szMD5[16];
    unsigned char szBlockSize[4];
    /* Record the true file size */
    unsigned char szReserve[8];
} BkArchiveMicroCopySt;

/* The store node structure */
typedef struct BkArchiveStoreNodeSt {
    /* The use flags: free, used, unused */
    char nUseFlags;

    /* In the store file, row and col position */
    unsigned char szRow[4];
    unsigned char szCol[4];

    /* the store archive file name */
    unsigned char szArchiveNo[4];
    unsigned char szStartPos[8];
    unsigned char szEndPos[8];
    
    /* The store type: base or version */
    char nStoreType;

    union {
        BkArchiveVersionSt Version;
        BkArchiveBaseSt Base;
    } StoreInfo;

    /* Flags: encrypted, compressed */
    char nStoreFlags;

    /* Checksum */
    char nFileDigested;
    char szReserve[12];
} BkArchiveNodeSt;

typedef struct _BK_STORE_ARCHIVE_FILE {
    PRInt64 nArchiveHighEnd;
    PRInt64 nArchiveLowEnd;
} BkStoreArchiveFile;

typedef struct _BK_STORE_HANDLE_ST {
    unsigned char szMagic[16];
    unsigned long nVersion;
    unsigned short nColCount;
    unsigned long nRowCount;
    unsigned long nFileCount;
    unsigned long nOpenFlags;

    /**
     * Commented by liqy. no support global variables.
     * BkArchiveNodeSt CurrentNode;
     *
     * The archive file end position(8 Bytes)
     * unsigned long nArchiveNo;
     */

    /* The archive files array */
    BkStoreArchiveFile ArchiveFiles[BACKUP_ARCHIVE_MAX_FILE];
    char szArchivename[16];

    /* The store file md5 sum */
    unsigned short nMD5NodeCount;

    /* The store node matrix allocated in the pool */
    pool *pMatrixPool;

    /* User archive files */
    UserArchiveFileSt *pUserArchiveFiles;

    BkArchiveNodeSt **pMatrix;

    /* User Tag */
    char szUserTag[MD5_SIZE];
} BkStoreHandleSt;

/* The cache structure */
struct _BK_STORE_CACHE_ST {
    pool *pPool;

    /* The cache status. When it is dirty, then flush */
    int nDirty;

    /* The store cache handle */
    BkStoreHandleSt *pStoreHandle;

    /* The index tree handle */
    BkIndexNodeSt IndexRoot;

    /* The index tree file */
    char *pszIndexFile;

    /* The store file */
    char *pszStoreFile;

    /* The archive(data) file name */
    char *pszArchiveFile;

    /* The store file lock */
    BackupFileLockSt *pStoreLock;
};

typedef int (*BkStoreListFileCB)(BkArchiveNodeSt *pNode, void *pArg);

int  BkStoreFileClose(BkStoreHandleSt *pHandle,
                        const char *pszStoreFile);

/* The new Store functions */
int  BkStoreConstruct(const char *pszStoreFile,
                        BkStoreHandleSt *pStoreHandle,
                        request_rec *pReq);
int  BkStoreFlush(BkStoreHandleSt *pStoreHandle,
                        const char *pszStoreFile);

int  BkStoreFileSearchFile(BkStoreHandleSt *pHandle,
                        const char *pszFile,
                        int nStoreType,
                        BkArchiveNodeSt *pNode);
int  BkStoreFileAddFile(BkStoreHandleSt *pHandle,
                        BkArchiveNodeSt *pNode,
                        int nStoreMethod);
int  BkStoreFileAdd(request_rec *pReq,
                    BkStoreHandleSt *pHandle,
                    BkArchiveNodeSt *pNode,
                    char *pszPolicyName,
                    int nBackupFlag,
                    int nFlag = 0);
int  BkStoreAddNewFile(BkStoreHandleSt *pHandle,
                        BkArchiveBaseSt *pArchiveBase,
                        BkArchiveVersionSt *pArchiveVersion);
/* modify the whole node, set nWay 1
 * else, set nWay 0
 */
int  BkStoreFileModifyFile(BkStoreHandleSt *pHandle,
                        BkArchiveNodeSt *pNode,
                        int nWay);
/* delete a solo node
 * usually used at error handle, not need free user free space
 */
int  BkStoreFileDeleteFile(BkStoreHandleSt *pHandle,
                        BkArchiveNodeSt *pNode,
                        PRInt64 nArchiveSize);
/* delete base and all version node of a file
 * return the free space at *pnFree to caller
 */
int  BkStoreFileDeleteFile(BkStoreHandleSt *pHandle,
                           const char *pszFileName, 
                           PRInt64 *pnFree,
                           PRInt64 nArchiveSize);

/* clean store node */
int  BkStoreCleanVerNode(BkStoreHandleSt *pHandle, int *nDirty);
int  BkStoreCleanBaseNode(BkStoreHandleSt *pHandle, int *nDirty);

unsigned long BkStoreFileHash(BkStoreHandleSt *pHandle,
                        const char *pszFile,
                        register unsigned long nLen);

/* The new archive file functions */
int  BkArchiveOpenFile(request_rec *pReq, BkStoreHandleSt *pHandle,
                       int nArchiveNode, PRFileDesc **pArchiveHandle, 
                       int nFlags);

/* When append successs, the user cache has locked
 * caller must unlock the cache later
 */
int  BkArchiveWriteMicroCopy(BkArchiveNodeSt *pNode,
                        PRFileDesc *pArchiveFD);

int  BkArchiveRead(PRFileDesc *pArchiveFD,
                        char *pData,
                        unsigned long nSize,
                        unsigned long *pReadBytes);
int  BkArchiveSeek(PRFileDesc *pArchiveFD,
                        PRInt64 nOffset,
                        PRSeekWhence nDirect);


/* tools functions */
int  BkStoreGetFileSize(unsigned char *pszFileSize,
                        unsigned long *pnFileSizeHigh,
                        unsigned long *pnFileSizeLow);
int  BkStoreSetFileSize(unsigned char *pszFileSize,
                        unsigned long nFileSizeHigh,
                        unsigned long nFileSizeLow);
void FileTimeToStr(unsigned long nTime, unsigned char *pszBuf);
void StrToFileTime(unsigned long *pTime, unsigned char *pszBuf);
int  BkArchBase2FileBase(request_rec *pReq, BkArchiveBaseSt *pArchiveBase,
                        BkFileBaseSt *pFileBase);
int  BkArchVer2FileVer(request_rec *pReq, BkArchiveVersionSt *pArchiveVer,
                        BkFileVersionSt *pFileVer);
int  BkFileBase2ArchBase(request_rec *pReq, BkFileBaseSt *pFileBase,
                        BkArchiveBaseSt *pArchiveBase);
int  BkFileVer2ArchVer(request_rec *pReq, BkFileVersionSt *pFileVer,
                        BkArchiveVersionSt *pArchiveVer);
/*
 * lexiongjia add read the pReadDesc file write data to callback function
 */
int BkHandleWriteCb(request_rec *pReq, StoreHandle *pStore, void *pCtx, BkArcFileReadCallback pCbFunc, 
                        PRInt64 nFileSize);

/* The index tree APIs */
int  BkIndexNodeClose(BkIndexNodeSt *pRoot, const char *pszStoreFile);
int  BkIndexNodeAdd(pool *pPool, BkIndexNodeSt *pRoot, const char *pszFile,
                        BkIndexNodeSt **pNode);
int  BkIndexNodeDel(BkIndexNodeSt *pRoot, const char *pszFile);
int  BkIndexNodeSearch(BkIndexNodeSt *pRoot, const char *pszFile,
                        BkIndexNodeSt **pNode);
int  BkIndexNodeTravel(BkIndexNodeSt *pRoot, const char *pszFile,
                        BkIndexNodeSt **pList,
                        unsigned long *pCount);
int  BkIndexNodeFlush(BkIndexNodeSt *pRoot, const char *pszStoreFile);
void BkIndexNodeFreeNode(BkIndexNodeSt *pRoot);

int  BkIndexCreateFile(pool *pPool, request_rec *pReq, BkFileBaseSt *pBkFile,
                        BkAclInfoSt **pAcl, BkIndexNodeSt *pRoot);
int  BkIndexDeleteFile(request_rec *pReq, const char *pszSourceName,
                        BkAclInfoSt **pAcl, BkIndexNodeSt *pRoot);

/* travel delete empty directory index node */
int BkIndexTravelDelEmptyDir(BkIndexNodeSt *pRoot,
                             BkIndexNodeSt *pNode,
                             const char *pszSourceName);
/* delete empty directory index node */
int BkIndexDeleteEmptyDir(request_rec *pReq,
                          BkIndexNodeSt *pRoot, 
                          const char *pszDeleteFile);

/*
 * [out]:
 * pBaseFile struct point is a list
 * the first value is the information of the input file, contain the File-Flags
 * next pointer point it's children infomation list
 */
int  BkIndexStatFile(request_rec *pReq, char *pszSourceName,
                        BkFileBaseSt **pBaseFile, 
                        BkIndexNodeSt *pRoot);

int  BkIndexStatChild(request_rec *pReq, const char *pszSourceName,
                        BkFileBaseSt **pBaseFile, BkAclInfoSt **pAcl,
                        BkIndexNodeSt *pRoot);

/* The store cache functions */
int  BkStoreCacheGetHandle(request_rec *pReq, BkStoreCacheSt **pCache);

int  BkSetCacheDirty(BkStoreCacheSt *pCache);

/* Get the user archive file maxsize by number */
PRInt64 GetArchiveFileSizeById(UserArchiveFileSt 
                           *pArchiveFiles, int nNumber);

/* Get the user archive file name*/
int GetArchiveFileNameById(UserArchiveFileSt *pArchiveFiles, unsigned long nArchiveNo,
                              char *pszBuffer, int nBufLen);

/* Search a node */
BkArchiveNodeSt *BkStoreGetNode(BkStoreHandleSt *pHandle,
                    int nRow, int nCol);

/* Modify user archive file quota by id */
void BkStoreModifyArchiveQuotaById(UserArchiveFileSt *pArchiveFiles, 
                                   PRInt64 nSize, int nIncrease, int nID);

/* Construct the store file name */
void BkStoreGetIndexFileName(request_rec *pReq, char *pszBuffer, int nBufLen);
void BkStoreGetUserLockFileName(request_rec *pReq, char *pszBuffer, int nBufLen);
void BkStoreGetStoreFileName(request_rec *pReq, char *pszBuffer, int nBufLen);
int  BkStoreGetArchiveFileName(request_rec *pReq, unsigned long nArchiveN0,
                                char *pszBuffer, int nBufLen);

/* Get oracle node list */
int  BkOracleRestoreVerFileList(request_rec *pReq, 
        const char *pszVersionName, BkFileVersionSt **pFileVersion);

/** 
 * Internal function. Write a node into the buffer.
 * @[in]
 * pNode is the adding node.
 * nRow is the row number.
 * nCol is the column number.
 * nAction is write action, which includes three actions:
 * 1 - add. 
 * 2 - modify.
 * 3 - delete.
 */
int BkStoreWriteNodeIn(BkStoreHandleSt *pHandle,
                       BkArchiveNodeSt *pNode,
                       int nRow, int nCol, int nAction);
/* 每备份一次添加介质文件 */
int BkAddArchiveFile(const char *pszUsername, 
        PRInt64 nArchOffset, char *pszOccupantFile, UserArchiveFileSt *pszArchive);

#ifdef __cplusplus
extern "C" {
#endif

API_EXPORT(int)  BkArchiveWrite(PRFileDesc *pArchiveFD,
                        char *pData,
                        unsigned long nLength,
                        unsigned long *pWriteBytes);

API_EXPORT(int)  BkArchiveAppend(request_rec *pReq,
                        const char *pszFilename,
                        PRInt64 nFileSize,
                        PRFileDesc **pFD,
                        char *pszRelationName);

API_EXPORT(int)  BkStoreCacheReleaseHandle(request_rec *pReq);

/* Close the user store cache system */
API_EXPORT(int)  BkCloseUserStoreCache(request_rec *pReq);

#ifdef __cplusplus
}
#endif

#endif /* __BACKUP_ARCHIVE_H_ */
