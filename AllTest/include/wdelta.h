/** =============================================================================
 ** Copyright (c) 2003 WaveTop Information Corp. All rights reserved.
 **
 ** The backup system
 **
 ** =============================================================================
 */

/* Binary diffenrence comparasion interface definition */
#ifndef __BACKUP_WDELTA_H_
#define __BACKUP_WDELTA_H_ 1

#include "nspr.h"

/* The delta storing version */
#define WAVETOP_BACKUP_DELTA_VERSION   1000

#define WDELTA_OK                   0
#define WDELTA_INVALID_SYNTAX       2
#define WDELTA_NO_MEMORY            3
#define WDELTA_NOT_INTEGRITY        4
#define WDELTA_FILE_NOT_EXIST       5
#define WDELTA_SAVE_DELTA_FAILED    6

#define WDELTA_INCREASE_BLOCK_COUNT  (1<<10)

#define WDELTA_HEADER_SIZE          16

typedef struct BKWDELTAHANDLEST {
    PRFileDesc *pOSFd;
    int nEQBlockCount;
    int nEQBlockSum;
    long *nEQBlockIndex;
    int nStatus;
    int nVersion;
    int nFlags;
    int nMode;
    long nBlockSize;
    long nBlockCount;
    long nRemainder;
    int nInCount;
    int nInPos;
    char szInBuf[8192];
} BkWDeltaHandleSt, *BkWDeltaHandle;


int  BkWDeltaOpen(const char *pszDeltaFile, 
                int nOpenFlags,
                long nBlockCount,
                long nBlockSize,
                long nRemainder,
                BkWDeltaHandle *pDelta);
int  BkWDeltaWriteEQIndex(BkWDeltaHandle pDelta,
                long nEQIndex);
int  BkWDeltaWriteNEQBlock(BkWDeltaHandle pDelta,
                char *pszBlock,
                long nBlockSize);
int  BkWDeltaWriteSum(BkWDeltaHandle pDelta,
                char *pszMD4,
                long nSize);
int  BkWDeltaReadIndex(BkWDeltaHandle pDelta,
                long *pIndex);
int  BkWDeltaReadBlock(BkWDeltaHandle pDelta,
                char *pszBlock,
                long nBlockSize);
int  BkWDeltaReadSum(BkWDeltaHandle pDelta,
                char *pszMD4,
                long nSize);
void BkWDeltaCancel(BkWDeltaHandle pDelta);
int  BkWDeltaClose(BkWDeltaHandle pDelta);

int  BkWDeltaPatch(const char *pszSrcFile,
                const char *pszDeltaFile,
                const char *pszDstFile);
int  BkWDeltaPatch2(PRFileDesc *pSrcFD,
                unsigned long nSrcSize,
                const char *pszDeltaFile,
                const char *pszDstFile);


#endif /* __BACKUP_DIFF_H_ 1 */
