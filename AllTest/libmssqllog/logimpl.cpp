/** =============================================================================
 ** Copyright (c) 2006 WaveTop Information Corp. All rights reserved.
 **
 ** The Mirror system
 **
 ** =============================================================================
 */

#if defined(WIN32) || defined(WINDOWS)
#include <windows.h>
#else
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include "configfile.h"
#include "sem.h"
#endif
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "libmssqllog.h"
#include "logfile.h"
#include "ioOptimize.h"

#ifdef WIN32
 /* To share the Mutex with other processes, we need a NULL ACL
 * Code from MS KB Q106387
*/
static PSECURITY_ATTRIBUTES GetNullACL(void)
{
    PSECURITY_DESCRIPTOR pSD;
    PSECURITY_ATTRIBUTES sa;
    
    sa  = (PSECURITY_ATTRIBUTES)LocalAlloc(LPTR, sizeof(SECURITY_ATTRIBUTES));
    pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
    if (pSD == NULL || sa == NULL) {
        return NULL;
    }
    
    /*
    * Win98 returns nonzero on failure; check LastError to make sure.
    */
    SetLastError(0);
    if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION)
        || GetLastError()) {
        LocalFree(pSD);
        LocalFree(sa);
        return NULL;
    }
    
    if (!SetSecurityDescriptorDacl(pSD, TRUE, (PACL) NULL, FALSE)
        || GetLastError()) {
        LocalFree(pSD);
        LocalFree(sa);
        return NULL;
    }
    
    sa->nLength = sizeof(SECURITY_DESCRIPTOR);
    sa->lpSecurityDescriptor = pSD;
    sa->bInheritHandle = FALSE;
    return sa;
}

static void CleanNullACL(void *sa)
{
    if (sa != NULL) {
        LocalFree(((PSECURITY_ATTRIBUTES)sa)->lpSecurityDescriptor);
        LocalFree(sa);
    }
}
#endif

#if defined(WIN32) || defined(WINDOWS)
static int MiMSSqlDeleteFile(LogHandleSt *pHandle, 
                             int nMaxSeqNum)
{
    char szDataPath[MAX_PATH];
    char szIdxPath[MAX_PATH];
    int nSeqNum = 0;

    for (; nSeqNum < nMaxSeqNum; nSeqNum++) {
        if (pHandle->szSuff[0] != '\0') {
            sprintf(szDataPath, "%s%s_%s.%06d", pHandle->szFileDir, 
                WAVETOP_LOGFILE_DATA, pHandle->szSuff, nSeqNum);
            sprintf(szIdxPath, "%s%s_%s.%06d", pHandle->szFileDir, 
                WAVETOP_LOGFILE_INDEX, pHandle->szSuff, nSeqNum);
        }
        else {
            sprintf(szDataPath, "%s%s_0.%06d", pHandle->szFileDir, 
                WAVETOP_LOGFILE_DATA, nSeqNum);
            sprintf(szIdxPath, "%s%s_0.%06d", pHandle->szFileDir, 
                WAVETOP_LOGFILE_INDEX, nSeqNum);
        }
        DeleteFile(szDataPath);
        DeleteFile(szIdxPath);
    }

    return WAVETOP_BACKUP_OK;
}



static int MiMSSqlCheckAndDeleteFile(LogHandleSt *pHandle, 
                                     unsigned char *pMMFView)
{
    int                nMinSeqNum = -1;
    int                nCount;
    int                nNodeLimit;
    int                nResult;
    time_t             nCurTime;
    bool               bForward = false;
    bool               bBackward = false;
    LogMMShareNodeSt  *pMMShareNode;

    nNodeLimit = WAVETOP_LOGFILE_MAP_SIZE / sizeof(LogMMShareNodeSt);
    nCurTime = time(0);
    for (nCount = 0; nCount < nNodeLimit; nCount++) {
        pMMShareNode = (LogMMShareNodeSt *)(pMMFView + 
            nCount * sizeof(LogMMShareNodeSt));
        if (pMMShareNode->bIsUsed == 0)
            continue;

        if ((nCurTime - pMMShareNode->nLastTime) > 15) {
            pMMShareNode->bIsUsed = 0;
            continue;
        }

        if (!bForward && pMMShareNode->nWay == 1) {
            bForward = true;
        }

        if (!bBackward && pMMShareNode->nWay == 2) {
            bBackward = true;
            break;
        }

        if (nMinSeqNum == -1) {
            nMinSeqNum = pMMShareNode->nCurNum;
            continue;
        }
        nMinSeqNum = (pMMShareNode->nCurNum < nMinSeqNum) ? 
            pMMShareNode->nCurNum : nMinSeqNum;
    }

    if (bBackward) {
        nResult = WAVETOP_BACKUP_OK;
    }
    else if (bForward) {
        nResult = MiMSSqlDeleteFile(pHandle, nMinSeqNum);
    }
    else {
        nResult = WAVETOP_BACKUP_INTERNAL_ERROR;
    }

    return nResult;
}

static void MiMSSqlGetNames(LogHandleSt *pHandle, 
                            char *pszMMSharefile,
                            char *pszMutexName)
{
    if (pHandle->szSuff[0] != '\0') {
        sprintf(pszMMSharefile, "%s_%s", WAVETOP_LOGFILE_SHARE, pHandle->szSuff);
        sprintf(pszMutexName, "%s_%s", WAVETOP_LOGFILE_MUTEX, pHandle->szSuff);
    }
    else {
        sprintf(pszMMSharefile, "%s_0", WAVETOP_LOGFILE_SHARE);
        sprintf(pszMutexName, "%s_0", WAVETOP_LOGFILE_MUTEX);
    }
}

DWORD WINAPI MiMSSqlMMShareThread(LPVOID lpParam)
{
    LogHandleSt          *pHandle;
    LogMMShareNodeSt     *pMMShareNode;
    bool                  bIsFirst = true;
    int                   nCount;
    int                   nNodeLimit;
    char                  szMMSharefile[256] = { 0 };
    char                  szMutexName[256] = { 0 };
    HANDLE                hFileMap = NULL;
    HANDLE                hMutex   = NULL;
    PVOID                 pView    = NULL;
    DWORD                 dwResult;
    PSECURITY_ATTRIBUTES  sa;

    pHandle = (LogHandleSt*)lpParam;
    pHandle->nDelete = 0;

    MiMSSqlGetNames(pHandle, szMMSharefile, szMutexName);

    /* 创建共享内存 */
    sa = GetNullACL();
    hMutex = CreateMutex(sa, FALSE, szMutexName);
    if (NULL == hMutex) {
        LOG_TRACE1("MiMSSqlMMShareThread : CreateMutex failed, code: %d", 
            GetLastError());
        goto END;
    }

    while (pHandle->nClose != 1) {
        dwResult = WaitForSingleObject(hMutex, 3000);
        if (dwResult == WAIT_TIMEOUT) {
            continue;
        }
        else if (dwResult == WAIT_OBJECT_0) {
            break;
        }
        else if (dwResult == WAIT_FAILED) {
            LOG_TRACE1("MiMSSqlMMShareThread : WaitForSingleObject failed, code: %d", 
                GetLastError());
            goto END;
        }
    }
    
    if (pHandle->nClose == 1) {
        goto END;
    }

    hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE, sa, PAGE_READWRITE, 0,
        WAVETOP_LOGFILE_MAP_SIZE, TEXT(szMMSharefile));
    if (hFileMap == NULL) {
        LOG_TRACE1("MiMSSqlMMShareThread : CreateFileMapping failed, code: %d",
            GetLastError());
        goto END;
    }

    /* 判断是否是第一个使用者 */
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        bIsFirst = false;
    }
 
    /* 将共享内存映射到当前进程 */
    pView = MapViewOfFile(hFileMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0,
        WAVETOP_LOGFILE_MAP_SIZE);
    if (pView == NULL) {
        LOG_TRACE1("MiMSSqlMMShareThread : MapViewOfFile failed, code: %d", 
            GetLastError());
        goto END;
    }

    nNodeLimit = WAVETOP_LOGFILE_MAP_SIZE / sizeof(LogMMShareNodeSt);
    if (bIsFirst)
        memset(pView, 0, WAVETOP_LOGFILE_MAP_SIZE);

    /* 传出去给别的接口使用这块共享内存 */
    pHandle->pView = (unsigned char *)pView;

    /* 寻找未使用的节点 */
    for (nCount = 0; nCount < nNodeLimit; nCount++) {
        pMMShareNode = (LogMMShareNodeSt *)((unsigned char*)pView + 
            nCount * sizeof(LogMMShareNodeSt));
        if (pMMShareNode->bIsUsed == 0) {
            pMMShareNode->bIsUsed = 1;
            pMMShareNode->nLastTime = time(NULL);
            pMMShareNode->nWay= pHandle->nWay;
            pMMShareNode->nCurNum = pHandle->nCurNum;
            break;
        }
    }
    ReleaseMutex(hMutex);

    /* 共享内存已用完 */
    if (nCount == nNodeLimit)
        goto END;

    try {    
        while (pHandle->nClose != 1) {
            dwResult = WaitForSingleObject(hMutex, 3000);
            if (dwResult == WAIT_TIMEOUT) {
                continue;
            }
            else if (dwResult == WAIT_FAILED) {
                LOG_TRACE1("MiMSSqlMMShareThread : WaitForSingleObject failed, code: %d", 
                    GetLastError());
                goto END;
            }
            pMMShareNode->bIsUsed = 1;
            pMMShareNode->nLastTime = time(NULL);
            pMMShareNode->nWay= pHandle->nWay;
            pMMShareNode->nCurNum = pHandle->nCurNum;
        
            /* 尝试删除文件 */
            if (pHandle->nDelete == 1) {
                MiMSSqlCheckAndDeleteFile(pHandle, (unsigned char *)pView);
                pHandle->nDelete = 0;
            }

            ReleaseMutex(hMutex);
            Sleep(500);
        }
    }
    catch (...) {
        LOG_TRACE1("MiMSSqlMMShareThread : Exception has been thrown, code: %d", 
            GetLastError());
    }

    dwResult = WaitForSingleObject(hMutex, 3000);
    if (dwResult != WAIT_OBJECT_0) 
        goto END;

    pMMShareNode->bIsUsed = 0;
    ReleaseMutex(hMutex);

END:
    CleanNullACL(sa);
    if (pView != NULL)
        UnmapViewOfFile(pView);
    if (hFileMap != NULL)
        CloseHandle(hFileMap);
    if (hMutex != NULL)
        CloseHandle(hMutex);

    return WAVETOP_BACKUP_OK;
}
#endif

static int BkMSSqlDeleteFile(LogHandleSt *pHandle, 
                             int nMaxSeqNum)
{
    char szDataPath[MAX_PATH];
    char szIdxPath[MAX_PATH];
    int nSeqNum = 0;
    int nRC = WAVETOP_BACKUP_OK;
    struct stat statBuff;
#if defined(WIN32) || defined(WINDOWS)
    BOOL bStatus = FALSE; 
#else
    int nStatus = -1;
#endif
    
    for (; nSeqNum < nMaxSeqNum; nSeqNum++) {
        if (pHandle->szSuff[0] != '\0') {
            sprintf(szDataPath, "%s%s_%s.%06d", pHandle->szFileDir, 
                WAVETOP_LOGFILE_DATA, pHandle->szSuff, nSeqNum);
            sprintf(szIdxPath, "%s%s_%s.%06d", pHandle->szFileDir, 
                WAVETOP_LOGFILE_INDEX, pHandle->szSuff, nSeqNum);
        }
        else {
            sprintf(szDataPath, "%s%s_0.%06d", pHandle->szFileDir, 
                WAVETOP_LOGFILE_DATA, nSeqNum);
            sprintf(szIdxPath, "%s%s_0.%06d", pHandle->szFileDir, 
                WAVETOP_LOGFILE_INDEX, nSeqNum);
        }
        
        nRC = stat(szDataPath, &statBuff);
        if (nRC == -1) 
            continue;

#if defined(WIN32) || defined(WINDOWS)
        bStatus = FALSE;
        bStatus = DeleteFile(szDataPath);
        if (!bStatus) 
            break;
        DeleteFile(szIdxPath);
#else
        nStatus = -1;
        nStatus = remove(szDataPath);
        if (nStatus == -1) 
            break;
        remove(szIdxPath);
#endif
    }
    
    return WAVETOP_BACKUP_OK;
}


static int MiMSSqlCreateMutex(LogHandleSt *pHandle)
{
#if defined(WIN32) || defined(WINDOWS)
    PSECURITY_ATTRIBUTES  sa;
    char szMutexName[1024] = { 0 };
    
    if (pHandle->szSuff[0] != '\0') {
        sprintf(szMutexName, "%s-%s", WAVETOP_LOGFILE_READWRITE_MUTEX, pHandle->szSuff);
    }
    else {
        sprintf(szMutexName, "%s-0", WAVETOP_LOGFILE_READWRITE_MUTEX);
    }
    sa = GetNullACL();
    pHandle->hMutex = CreateMutex(sa, FALSE, szMutexName);
    if (NULL == pHandle->hMutex) {
        LOG_TRACE1("CreateMutex failed, code: %d", GetLastError());
        CleanNullACL(sa);
        return GetLastError();
    }
    CleanNullACL(sa);

    return WAVETOP_BACKUP_OK;
#else
    pHandle->hMutex = (BackupFileLockSt *)malloc(sizeof(BackupFileLockSt));
    return BackupFileLockInit(pHandle->szLockFile, pHandle->hMutex);
#endif
}

static int MiMSSqlWaitForMutex(LogHandleSt *pHandle)
{
#if defined(WIN32) || defined(WINDOWS)
    unsigned long dwResult;
    
    do {
        dwResult = WaitForSingleObject(pHandle->hMutex, 3000);
        if (dwResult == WAIT_TIMEOUT)
            continue;
        else if (dwResult == WAIT_FAILED)
            return WAVETOP_BACKUP_INTERNAL_ERROR;
        else
            break;
    } while(TRUE);
    return WAVETOP_BACKUP_OK;
#else
    return BackupFileLockLock(pHandle->hMutex, WSTORE_WR_LOCK);
#endif
}

static void MiMSSqlReleseMutex(LogHandleSt *pHandle)
{
#if defined(WIN32) || defined(WINDOWS)
    ReleaseMutex(pHandle->hMutex);
#else
    BackupFileLockUnlock(pHandle->hMutex, WSTORE_WR_LOCK);
#endif
}

static void MiMSSqlDestroyMutex(LogHandleSt *pHandle)
{
#if defined(WIN32) || defined(WINDOWS)
    CloseHandle(pHandle->hMutex);
#else
    BackupFileLockDestroy(pHandle->hMutex);
    free(pHandle->hMutex);
#endif
}

static int MiMSSqlRollBack(LogHandleSt *pHandle)
{
    IndexSt stIndex;
    unsigned long nPos;
    unsigned long nTailPos;
    unsigned long nSize;
    int nResult = WAVETOP_BACKUP_OK;

    pHandle->hIndxFile->Last();
    nResult = pHandle->hIndxFile->Next(stIndex);
    if (nResult != WAVETOP_BACKUP_OK)
        return nResult;

    VINT32(nPos, stIndex.szPos);
    VINT32(nSize, stIndex.szSize);
    nTailPos = nPos + sizeof(unsigned long) + nSize;
    nResult = pHandle->hFile->Seek(WAVETOP_LOGFILE_BEGIN, nTailPos);
    if (nResult != WAVETOP_BACKUP_OK)
        return nResult;
    nResult = pHandle->hFile->SetEndOfFile(nTailPos);
    if (nResult != WAVETOP_BACKUP_OK)
        return nResult;
    pHandle->hFile->SetTailPos(nTailPos);

    return nResult;
}

/**
 * 初始化，使用共享读写文件方式
 * @[in]
 * pszLogPath - 指定的日志文件组存储路径
 * @[out]
 * pHandle - 返回打开的文件句柄
 * 成功返回WAVETOP_BACKUP_OK
 * 失败返回WAVETOP_BACKUP_INTERNAL_ERROR
 */
static int LogStart(MiLogHandle *ppHandle, 
                    char *pszLogPath, char *pszSuff, 
                    int nOperation)      
{
    LogHandleSt         *pHandle;
    LogHandleSt         *pRollBack;
    int                  nRet;
    char                 szDataPath[1024] = { 0 };
    char                 szIdxPath[1024] = { 0 };
    char                 szPathTemp[1024] = { 0 };
    char                *pszPtr;
    int                  nOptions;
#if defined(WIN32) || defined(WINDOWS)
    DWORD                dwThreadId;
#endif
    long                nPos = -1;

    assert((unsigned long)WAVETOP_LOGFILE_LIMITED_SIZE < ((unsigned long)1<<31));

    if (pszLogPath == NULL || *pszLogPath == '\0')
        return WAVETOP_BACKUP_FILE_NOT_EXIST;

    if (strlen(pszLogPath) >= 1000)
        return WAVETOP_BACKUP_FILENAME_TOO_LONG;

    pHandle = (LogHandleSt *)malloc(sizeof(LogHandleSt));
    if (pHandle == NULL)
        return WAVETOP_BACKUP_NO_MEMORY;

    memset(pHandle, 0, sizeof(LogHandleSt));

    if (pszSuff == NULL ||
        snprintf(pHandle->szSuff, sizeof(pHandle->szSuff), "%s", pszSuff) == -1) {
        free(pHandle);
        return WAVETOP_BACKUP_INTERNAL_ERROR;
    }
    
#if !defined(WIN32)
    sprintf(pHandle->szLockFile, "%s%s", pszLogPath, "/wt_sem");
#endif
    nRet = MiMSSqlCreateMutex(pHandle);
    if (nRet != WAVETOP_BACKUP_OK) {
        free(pHandle);/* 内存泄露 */
        return nRet;
    }

    nRet = MiMSSqlWaitForMutex(pHandle);
    if (nRet != WAVETOP_BACKUP_OK) {
        free(pHandle);/* 内存泄露 */
        return nRet;
    }

    pHandle->nOperation       = nOperation;
    pHandle->nLimitRecord     = WAVETOP_LOGFILE_LIMITED_RECORD;
    pHandle->nLimitSize       = WAVETOP_LOGFILE_LIMITED_SIZE;
    pHandle->nWay             = 1;

    /* Linear */
    pHandle->nLoopIndexCount  = 0xFFFFFFFF;

    pHandle->hFile            = NULL;
    pHandle->hIndxFile        = NULL;
    pHandle->nStatus          = WAVETOP_BACKUP_OK;

    strncpy((char*)(pHandle->szFileDir), pszLogPath, sizeof(pHandle->szFileDir));

    for (pszPtr = pHandle->szFileDir; *pszPtr != '\0'; pszPtr++) {
        if (*pszPtr == '\\')
            *pszPtr = '/';
    }

    if (pHandle->szSuff[0] != '\0') {
        sprintf(szPathTemp, "%s%s_%s.bin", pHandle->szFileDir, 
            WAVETOP_LOGFILE_CONF, pHandle->szSuff);
    }
    else {
        sprintf(szPathTemp, "%s%s_0.bin", pHandle->szFileDir, 
            WAVETOP_LOGFILE_CONF);
    }

    /* 如果是正常打开,则要读取文件序号 */
#if defined(WIN32) || defined(WINDOWS)
    pHandle->nCurNum = GetPrivateProfileInt("mssql2",
                                            "FileNum",
                                            0,
                                            szPathTemp);
#else
    CConfigfile *pCfg;
    pCfg = new CConfigfile(szPathTemp);
    if(pCfg != NULL) {
        pHandle->nCurNum = pCfg->ReadInt("mssql2", "FileNum", 0);
        delete pCfg;
    }
    else {
        LOG_TRACE1("The log.bin file %s open failed\n", szPathTemp);
        pHandle->nCurNum = 0;
    }
#endif
    pHandle->nFirstNo = pHandle->nCurNum;
   int nLastnum =0 ;
	BkMSSqlGetFileMaxNum((MiLogHandle*)pHandle,&nLastnum);
	printf("BkMSSqlGetFileMaxNum:  nLastnum: %d", nLastnum);
    if (nOperation == 1)
        nOptions = WAVETOP_LOGFILE_CREATE_FILE | WAVETOP_LOGFILE_RDONLY;
    else
        nOptions = WAVETOP_LOGFILE_CREATE_FILE | WAVETOP_LOGFILE_RDWR;

    if (pHandle->szSuff[0] != '\0') {
        sprintf(szIdxPath, "%s%s_%s.%06d", pHandle->szFileDir, 
            WAVETOP_LOGFILE_INDEX, pHandle->szSuff, pHandle->nCurNum);
    }
    else {
        sprintf(szIdxPath, "%s%s_0.%06d", pHandle->szFileDir, 
            WAVETOP_LOGFILE_INDEX, pHandle->nCurNum);
    }

    pHandle->hIndxFile = new CIndexLogFile();
    pHandle->hIndxFile->SetNo(pHandle->nCurNum);
    nRet = pHandle->hIndxFile->Open(szIdxPath, 
        nOptions,
        0644,
        &nPos);
    if (nRet == WAVETOP_BACKUP_OPEN_FILE_ERROR)
        goto FAILED;

    if (pHandle->szSuff[0] != '\0') {
        sprintf(szDataPath, "%s%s_%s.%06d", pHandle->szFileDir, 
            WAVETOP_LOGFILE_DATA, pHandle->szSuff, pHandle->nCurNum);
    }
    else {
        sprintf(szDataPath, "%s%s_0.%06d", pHandle->szFileDir, 
            WAVETOP_LOGFILE_DATA, pHandle->nCurNum);
    }

    pHandle->hFile = new CDataLogFile();
    pHandle->hFile->SetNo(pHandle->nCurNum);
    nRet = pHandle->hFile->Open(szDataPath, 
        nOptions,
        0644,
        nPos);
    if (nRet != WAVETOP_BACKUP_OK)
        goto FAILED;

    if (pHandle->hFile->GetRecordCount() != pHandle->hIndxFile->GetRecordCount()) {
        LOG_TRACE1("The index file %s has no matched data file. Rollback data file.\n",
            pHandle->hIndxFile->GetName());

        if (nOperation == 1) {
            LogHandleSt *pTmpHandle;
            int nOption;

            pTmpHandle = (LogHandleSt *)malloc(sizeof(LogHandleSt));
            if (pTmpHandle == NULL) {
                nRet = WAVETOP_BACKUP_NO_MEMORY;
                goto FAILED;
            }

            nOption = WAVETOP_LOGFILE_CREATE_FILE |
                WAVETOP_LOGFILE_RDWR | WAVETOP_LOGFILE_WITHOUT_VALIDATE;

            pTmpHandle->hIndxFile = new CIndexLogFile();
            pTmpHandle->hIndxFile->SetNo((pHandle)->nCurNum);
            nRet = pTmpHandle->hIndxFile->Open(szIdxPath, nOption, 0644);
            if (nRet != WAVETOP_BACKUP_OK) {
                delete pTmpHandle->hIndxFile;
                free(pTmpHandle);

                goto FAILED;
            }
            pTmpHandle->hIndxFile->SetRecordCount(pHandle->hIndxFile->GetRecordCount());

            pTmpHandle->hFile = new CDataLogFile();
            pTmpHandle->hFile->SetNo(pHandle->nCurNum);
            nRet = pTmpHandle->hFile->Open(szDataPath, nOption, 0644);
            if (nRet != WAVETOP_BACKUP_OK) {
                delete pTmpHandle->hIndxFile;
                delete pTmpHandle->hFile;
                free(pTmpHandle);

                goto FAILED;
            }

            pRollBack = pTmpHandle;
        }
        else {
            pRollBack = pHandle;
        }

        nRet = MiMSSqlRollBack(pRollBack);
        pHandle->hFile->SetRecordCount(pRollBack->hIndxFile->GetRecordCount());
        pHandle->hFile->SetTailPos(pRollBack->hFile->GetTailPos());
        if (nOperation == 1) {
            delete pRollBack->hIndxFile;
            delete pRollBack->hFile;
            free(pRollBack);
        }
        if (nRet != WAVETOP_BACKUP_OK) {
            LOG_TRACE("Rollback data file failed.\n");

            goto FAILED;
        }
    }

#if defined(WIN32) || defined(WINDOWS)
    pHandle->hThread = CreateThread(NULL, 0, MiMSSqlMMShareThread, (void *)pHandle, 
        0, &dwThreadId);
    if (pHandle->hThread == NULL) {
        nRet = WAVETOP_BACKUP_INTERNAL_ERROR;
        LOG_TRACE1("LogStart() failed in CreateThread(), error: %d", GetLastError());
        goto FAILED;
    }
#endif

    nRet = WAVETOP_BACKUP_OK;
    *ppHandle = pHandle;

FAILED: 
    MiMSSqlReleseMutex(pHandle);

    if (nRet != WAVETOP_BACKUP_OK) {
        if (pHandle != NULL) {
            if (pHandle->hFile != NULL)
                delete pHandle->hFile;
            if (pHandle->hIndxFile != NULL)
                delete pHandle->hIndxFile;
            if (pHandle->hMutex != NULL)
                MiMSSqlDestroyMutex(pHandle);
            free(pHandle);
        }
    }

    return nRet;
}

/**
 * 初始化，使用共享读写文件方式
 * @[in]
 * pszLogPath - 指定的日志文件组存储路径
 * @[out]
 * pHandle - 返回打开的文件句柄
 * 成功返回WAVETOP_BACKUP_OK
 * 失败返回WAVETOP_BACKUP_INTERNAL_ERROR
 */
MILOG_EXPORT_(int) MiMSSqlLogStart(MiLogHandle *ppHandle, 
                                   char *pszLogPath, 
                                   int nOperation,
                                   int nPort)
{
    int nRet;
    char szSuff[16];
    MiLogHandle pHandle;

    if (sprintf(szSuff, "%d", nPort) == -1) {
        return WAVETOP_BACKUP_INVALID_SYNTAX;
    }

    nRet = LogStart(&pHandle, pszLogPath, szSuff, nOperation);
    if (nRet == WAVETOP_BACKUP_OK) {
        *ppHandle = pHandle;
    }

    return nRet;
}

/**
 * 初始化，使用共享读写文件方式
 * @[in]
 * pszLogPath - 指定的日志文件组存储路径
 * @[out]
 * pHandle - 返回打开的文件句柄
 * 成功返回WAVETOP_BACKUP_OK
 * 失败返回WAVETOP_BACKUP_INTERNAL_ERROR
 */
MILOG_EXPORT_(int) MiMSSqlLogStartEx(MiLogHandle *ppHandle, 
                                     char *pszLogPath, 
                                     int nOperation,
                                     char *pszSuff)
{
    int nRet;
    MiLogHandle pHandle;

    if (pszSuff == NULL) {
        return WAVETOP_BACKUP_INTERNAL_ERROR;
    }

    nRet = LogStart(&pHandle, pszLogPath, pszSuff, nOperation);
    if (nRet == WAVETOP_BACKUP_OK) {
        *ppHandle = pHandle;       
    }

    return nRet;
}

/**
 * 关闭.
 * pHandle - 打开的日志文件的句柄
 * @[out]
 * 成功返回WAVETOP_BACKUP_OK
 * 失败返回WAVETOP_BACKUP_INTERNAL_ERROR
 */
MILOG_EXPORT_(int) MiMSSqlReadBufferEnd(MiLogHandle hHandle)
{
    LogHandleSt *pHandle = (LogHandleSt *)hHandle;
    int nRet = WAVETOP_BACKUP_OK;
    long dwResult;

    if (pHandle == NULL)
        return WAVETOP_BACKUP_INTERNAL_ERROR;

    MiMSSqlWaitForMutex(pHandle);

    if (pHandle->hFile != NULL)
        delete pHandle->hFile;
    if (pHandle->hIndxFile != NULL)
        delete pHandle->hIndxFile;

    MiMSSqlReleseMutex(pHandle);
    
#if defined(WIN32) || defined(WINDOWS)
    pHandle->nClose = 1;
    dwResult = WaitForSingleObject(pHandle->hThread, 15000);
#endif

    if (pHandle->hMutex != NULL)
        MiMSSqlDestroyMutex(pHandle);
    free(pHandle);

    return nRet;
}

/** 
 * 切换索引和数据文件句柄
 * @[in]
 * nDirect - 切换方向. 1 -- 顺序; 2 -- 反向.
 **/
static int MiMSSqlSwitchIndex(LogHandleSt *pHandle, int nDirect)
{
    char            szPath[1024];
    char            szFileNo[16];
    CDataLogFile   *pOldData;
    CIndexLogFile  *pOldIndx;
    int             nRet;
    int             nOptions;
    int             nOldNum;

    assert(pHandle->nLoopIndexCount >= 0);

    pOldData            = pHandle->hFile;
    pOldIndx            = pHandle->hIndxFile;
    pHandle->hFile      = NULL;
    pHandle->hIndxFile  = NULL;

    nOldNum = pHandle->nCurNum;

    if (nDirect == 1)
        pHandle->nCurNum++;
    else if (nDirect == 2)
        pHandle->nCurNum--;

    if (pHandle->nCurNum < 0) {
        pHandle->nCurNum = 0;
        nRet = WAVETOP_BACKUP_FILE_NOT_EXIST;
        
        goto FAILED;
    }
    if (pHandle->nCurNum >= pHandle->nLoopIndexCount)
        pHandle->nCurNum = 0;

    if (pHandle->nOperation == 1)
        nOptions = WAVETOP_LOGFILE_RDONLY;
    else
        nOptions = WAVETOP_LOGFILE_CREATE_FILE | WAVETOP_LOGFILE_RDWR;

    if (pHandle->szSuff[0] != '\0') {
        sprintf(szPath, "%s%s_%s.%06d", pHandle->szFileDir, 
            WAVETOP_LOGFILE_DATA, pHandle->szSuff, pHandle->nCurNum);
    }
    else {
        sprintf(szPath, "%s%s_0.%06d", pHandle->szFileDir, 
        WAVETOP_LOGFILE_DATA, pHandle->nCurNum);
    }

    if (pHandle->hFile != NULL) {
        pHandle->hFile->Close();
    }

    pHandle->hFile = new CDataLogFile();
    pHandle->hFile->SetNo(pHandle->nCurNum);
    nRet = pHandle->hFile->Open(szPath, 
        nOptions,
        0644);
    if (nRet != WAVETOP_BACKUP_OK)
        goto FAILED;

    if (pHandle->szSuff[0] != '\0') {
        sprintf(szPath, "%s%s_%s.%06d", pHandle->szFileDir, 
            WAVETOP_LOGFILE_INDEX, pHandle->szSuff, pHandle->nCurNum);
    }
    else {
        sprintf(szPath, "%s%s_0.%06d", pHandle->szFileDir, 
            WAVETOP_LOGFILE_INDEX, pHandle->nCurNum);
    }

    if (pHandle->hIndxFile != NULL) {
        pHandle->hIndxFile->Close();
    }

    pHandle->hIndxFile = new CIndexLogFile();
    pHandle->hIndxFile->SetNo(pHandle->nCurNum);
    nRet = pHandle->hIndxFile->Open(szPath, 
        nOptions,
        0644);
    if (nRet != WAVETOP_BACKUP_OK)
        goto FAILED;

    /** 仅当写方式才保存文件序号 **/
    if (pHandle->nOperation == 2) {
        sprintf(szFileNo, "%d",   pHandle->nCurNum);

        if (pHandle->szSuff[0] != '\0') {
            sprintf(szPath, "%s%s_%s.bin", pHandle->szFileDir, 
                WAVETOP_LOGFILE_CONF, pHandle->szSuff);
        }
        else {
            sprintf(szPath, "%s%s_0.bin", pHandle->szFileDir, 
                WAVETOP_LOGFILE_CONF);
        }

#if defined(WIN32) || defined(WINDOWS)
        WritePrivateProfileString("mssql2", "FileNum", szFileNo, szPath);
#else
        CConfigfile *pCfg;
        pCfg = new CConfigfile(szPath);
        pCfg->WriteString("mssql2", "FileNum", szFileNo);
        delete pCfg;
#endif
    }
    
FAILED:
    if (nRet != WAVETOP_BACKUP_OK) {
#if defined(WIN32) || defined(WINDOWS)
#ifdef _DEBUG
        printf("[Thd: %5d] Switch to index file %d failed!!!.\n", 
            GetCurrentThreadId(), pHandle->nCurNum);
#endif
#endif
        pHandle->nCurNum = nOldNum;

        if (pHandle->hFile != NULL)
            delete pHandle->hFile;
        if (pHandle->hIndxFile != NULL)
            delete pHandle->hIndxFile;

        pHandle->hFile     = pOldData;
        pHandle->hIndxFile = pOldIndx;
    }
    else {
        delete pOldIndx;
        delete pOldData;
    }

    return nRet;
}

/**
 * 将缓冲区中的数据写入指定日志文件
 * @[in]
 * pHandle - 打开的日志文件的句柄
 * pszBuffer - 待写入日志文件的数据
 * nBuffCount - 待写入日志文件数据的长度
 * nSocket - 待写入日志文件的套接字
 * pszIP - 待写入日志文件的IP地址
 * nType - 写入类型
 * nNum - 指定写入的序号
 * @[out]
 * 成功返回WAVETOP_BACKUP_OK
 * 失败返回WAVETOP_BACKUP_INTERNAL_ERROR
 */
MILOG_EXPORT_(int) MiMSSqlWriteBufferToLog(MiLogHandle hHandle,
                                           WTBUF *pBuffers,
                                           unsigned long nBuffCount,
                                           ExIndexSt *pIndex)
{
    LogHandleSt    *pHandle = (LogHandleSt *)hHandle;
    CIndexLogFile  *pIndexFile;
    CDataLogFile   *pDataFile;
    int             nResult;
    IndexSt         index;
    unsigned long   nTotalSize;
    unsigned long   nFileSize;
    unsigned long   nDataEndPos;
    unsigned long   nItem;
    PRInt32         nStatus = 0;

    assert(pHandle != NULL);
    assert(pHandle->nOperation == 2);

    if (pHandle->nOperation == 1)
        return WAVETOP_BACKUP_INVALID_SYNTAX;

    /* 检测共享内存，如果nWay=4，需要转换文件 */
    BkMSSqlOpenShareMM(hHandle, WAVETOP_MIRROR_MSSQL2_LOG_BROKEN, &nStatus);

    nResult = MiMSSqlWaitForMutex(pHandle);
    if (nResult != WAVETOP_BACKUP_OK)
        return WAVETOP_BACKUP_INTERNAL_ERROR;

    pIndexFile = pHandle->hIndxFile;
    pDataFile  = pHandle->hFile;

    /* 算出所有缓冲区大小 */
    nTotalSize = 0;
    for (nItem = 0; nItem < nBuffCount; nItem++)
        nTotalSize += pBuffers[nItem].dwBufSize;

    pDataFile->GetSize(&nFileSize);

    /** 当数据文件超过2GB时也进行转换 **/
    if (   (pIndexFile->GetRecordCount() >= pHandle->nLimitRecord)
        || ((nFileSize + nTotalSize) >= pHandle->nLimitSize)
        || nStatus == WAVETOP_MIRROR_MSSQL2_LOG_BROKEN) {
        nResult = MiMSSqlSwitchIndex(pHandle, 1);
#if defined(WIN32) || defined(WINDOWS)
        if (nResult != WAVETOP_BACKUP_OK) {
            LOG_TRACE1("MiMSSqlSwitchIndex() failed in MiMSSqlWriteBufferToLog(),"
                       "FAILED Code:%d\n", GetLastError());
            goto FAILED;
        }
#ifdef _DEBUG
        printf("[Thd: %5d] Switch to index file %d for Writing.\n", 
            GetCurrentThreadId(), pHandle->nCurNum);
#endif
#else
        if (nResult != WAVETOP_BACKUP_OK) {
            LOG_TRACE1("MiMSSqlSwitchIndex() failed in MiMSSqlWriteBufferToLog(),"
                       "FAILED Code:%d\n", errno);
            goto FAILED;
        }
#endif
        pIndexFile = pHandle->hIndxFile;
        pDataFile  = pHandle->hFile;
    }

    /* 首先, 写入数据文件 */
    nDataEndPos = pDataFile->GetTailPos();
    nResult = pDataFile->WriteRecord(pBuffers, nBuffCount);
    if (nResult != WAVETOP_BACKUP_OK) 
        goto FAILED;

    /* 第二步, 写入索引,序号,类型,SOCKET */
    XINT32(nDataEndPos,        index.szPos);
    XINT64(pIndex->nSeqNum,    index.szSeqNum);
    XINT32(pIndex->nSocket,    index.szSocket);
    XINT32(nTotalSize,         index.szSize);
    index.cType             =  pIndex->cType;
    strncpy(index.szIP, pIndex->szIP, sizeof(index.szIP));
    nResult = pIndexFile->AppendRecord(index);
    if (nResult != WAVETOP_BACKUP_OK) {
        /* Rollback to old position */
        pDataFile->Seek(WAVETOP_LOGFILE_BEGIN, nDataEndPos);
        goto FAILED;
    }

#if defined(WIN32) || defined(WINDOWS)
#ifdef _DEBUG
    printf("[Thd: %5d] AppdRecord seq=%d into index file %d success\n",
        GetCurrentThreadId(), (unsigned long)pIndex->nSeqNum, pHandle->nCurNum);
#endif
#endif

FAILED:
    MiMSSqlReleseMutex(pHandle);
    
    return nResult;
}

/**
 * 指定从某一序号开始读取数据
 * @[in]
 * Handle - 打开的日志文件的句柄
 * nNum - 起始序号。
 * nType - 如果是1，读取方式为从指定序号至末尾或头部。如果是2，读取方式为仅读取指定序号
 * @[out]
 * 成功返回WAVETOP_BACKUP_OK
 * 失败返回WAVETOP_BACKUP_INTERNAL_ERROR
 */
MILOG_EXPORT_(int) MiMSSqlReadBufferFromLogStart(MiLogHandle hHandle,
                                                 PRUint64 nNum,
                                                 int nType)
{
    LogHandleSt     *pHandle = (LogHandleSt *)hHandle;
    CIndexLogFile   *pIndexFile;
    int              nResult;
    bool             bFirstFound = false;
    unsigned long    nFile;
    char             szPath[1024];
    PRUint64         nStartSeq;
    PRUint64         nEndSeq;
    LogIndexHeaderSt inHeader;
    IndexSt          iIndex;
#if defined(WIN32) || defined(WINDOWS)
    struct stat      statBuf;
#else
    struct stat      statBuf;
#endif

    assert(pHandle != NULL);
    assert(pHandle->nOperation == 1);

    if (pHandle->nOperation == 2)
        return WAVETOP_BACKUP_INVALID_SYNTAX;

    nResult = MiMSSqlWaitForMutex(pHandle);
    if (nResult != WAVETOP_BACKUP_OK)
        return WAVETOP_BACKUP_INTERNAL_ERROR;

    /* 初始化状态 */
    pHandle->nStatus = WAVETOP_BACKUP_OK;
    pIndexFile = pHandle->hIndxFile;

    pIndexFile->GetHeader(inHeader);
    VINT64(nStartSeq,  inHeader.szStartSeq);
    VINT64(nEndSeq,    inHeader.szEndSeq);
    if (nNum == nEndSeq + 1) {
        MiMSSqlReleseMutex(pHandle);
        return WAVETOP_BACKUP_END;
    }

    /** 首先在当前索引文件查找 **/
    if (nNum >= nStartSeq && nNum <= nEndSeq) {
        nResult = pIndexFile->Search(nNum, iIndex);
        if (nResult == WAVETOP_BACKUP_OK) {
            MiMSSqlReleseMutex(pHandle);
            return WAVETOP_BACKUP_OK;
        }
    }

    /* 循环查找所有的文件 */
    for (nFile = 0; nFile < pHandle->nLoopIndexCount; nFile++) {
        if (pHandle->szSuff[0] != '\0') {
            sprintf(szPath, "%s%s_%s.%06d", 
                pHandle->szFileDir, WAVETOP_LOGFILE_INDEX, pHandle->szSuff, nFile);
        }
        else {
            sprintf(szPath, "%s%s_0.%06d", 
                pHandle->szFileDir, WAVETOP_LOGFILE_INDEX, nFile);
        }

        if (stat(szPath, &statBuf) == -1) {
            if (bFirstFound) {
                LOG_TRACE1("index file %s not exist\n", szPath);
                nResult = WAVETOP_BACKUP_FILE_NOT_EXIST;
                break;
            }
            continue;
        }

        bFirstFound = true;

        pIndexFile = new CIndexLogFile();
        nResult = pIndexFile->Open(szPath, WAVETOP_LOGFILE_RDONLY, 0644);
        if (nResult != WAVETOP_BACKUP_OK) {
            LOG_TRACE1("Open index file %s failed\n", szPath);
            delete pIndexFile;
            break;
        }

        pIndexFile->GetHeader(inHeader);
        VINT64(nStartSeq,  inHeader.szStartSeq);
        VINT64(nEndSeq,    inHeader.szEndSeq);

        if (nNum > nEndSeq) {
            /* Does not exist in this index file */
            delete pIndexFile;
            continue;
        }

        nResult = pIndexFile->Search(nNum, iIndex);
        if (nResult == WAVETOP_BACKUP_OK) {
            LOG_TRACE1("Search seq in index file %d.\n", nFile);
            goto END;
        }

        delete pIndexFile;
    }

    if (nFile >= pHandle->nLoopIndexCount)
        nResult = WAVETOP_BACKUP_FILE_NOT_EXIST;

END:
    if (nResult == WAVETOP_BACKUP_OK) {
        delete pHandle->hIndxFile;
        pHandle->hIndxFile = pIndexFile;
        pHandle->nCurNum   = nFile;
        pHandle->nFirstNo  = nFile;

        /* 切换数据文件 */
        delete pHandle->hFile;

        if (pHandle->szSuff[0] != '\0') {
            sprintf(szPath, "%s%s_%s.%06d", 
                pHandle->szFileDir, WAVETOP_LOGFILE_DATA, pHandle->szSuff, nFile);
        }
        else {
            sprintf(szPath, "%s%s_0.%06d", 
                pHandle->szFileDir, WAVETOP_LOGFILE_DATA, nFile);
        }

        LOG_TRACE1("Switch data file %d\n", nFile);

        pHandle->hFile = new CDataLogFile();
        nResult = pHandle->hFile->Open(szPath, WAVETOP_LOGFILE_RDONLY, 0644);
    }
    MiMSSqlReleseMutex(pHandle);

    return nResult;
}

/**
 * 循环调用该方法，获取从指定序号开始至末尾的数据
 * @[in]
 * pHandle - 打开的日志文件的句柄
 * pszBuffer - 待写入缓冲区
 * nSize - 待写入缓冲区长度
 * nSocket - 当不为0时读取指定套接字的下一条数据，当为0时忽略
 * @[out]
 * pnReadBytes - 返回已写入数据的长度
 * pIndex      - 返回索引节点.
 * 如果同一序号读取时缓冲区不够大，先将现有的缓冲区写满
 * 并返回WAVETOP_MIRROR_MORE_WITH_LINE，然后再次调用此方法接着读取
 * 该序号读取完成返回WAVETOP_BACKUP_OK
 * 已读取至末尾返回WAVETOP_MIRROR_LINE_END
 * 出现异常返回WAVETOP_BACKUP_INTERNAL_ERROR
 */
MILOG_EXPORT_(int) MiMSSqlReadBufferFromLogNext(MiLogHandle hHandle,
                                                char *pszBuffer,
                                                unsigned long nSize,
                                                unsigned long *pnReadBytes,
                                                unsigned long nSocket,
                                                ExIndexSt *pIndex)
{
    LogHandleSt    *pHandle = (LogHandleSt *)hHandle;
    CIndexLogFile  *pIndexFile;
    CDataLogFile   *pDataFile;
    unsigned long   nResult;
    unsigned long   nPos;
    unsigned long   nInSock;
    unsigned long   nBytes;
    unsigned long   nRecordSize;
    IndexSt         Index;
    char            szBuf[32];

    assert(pHandle != NULL);
    assert(pHandle->nOperation == 1);

    if (pHandle->nOperation == 2)
        return WAVETOP_BACKUP_INVALID_SYNTAX;

    if (pHandle->nStatus != WAVETOP_BACKUP_OK)
        return pHandle->nStatus;

    nResult = MiMSSqlWaitForMutex(pHandle);
    if (nResult != WAVETOP_BACKUP_OK)
        return WAVETOP_BACKUP_INVALID_SYNTAX;

    pHandle->nWay = 1;
    pIndexFile = pHandle->hIndxFile;
    pDataFile  = pHandle->hFile;

    if (pIndexFile->m_nMoreSize > 0) {
        nRecordSize = pIndexFile->m_nMoreSize > nSize ? nSize : pIndexFile->m_nMoreSize;
        if (pDataFile->Read(pszBuffer, nRecordSize, &nBytes) != WAVETOP_BACKUP_OK)
            goto FAILED;

        *pnReadBytes = nRecordSize;
        
        if (pIndexFile->m_nMoreSize > nSize) {
            pIndexFile->m_nMoreSize -= nRecordSize;
            nResult = WAVETOP_MIRROR_MSSQL2_LOG_MORE_WITH_LINE;
        }
        else {
            pIndexFile->m_nMoreSize = 0;
            nResult = WAVETOP_BACKUP_OK;
        }

        goto FAILED;
    }
    
    /** 找到正确的Index **/
    if (nSocket != 0) {
        for (;;) {
            nResult = pIndexFile->Next(Index);
            if (nResult == WAVETOP_BACKUP_END) {
                LOG_TRACE1("Switch to index file %d when Reading index.\n", 
                    pHandle->nCurNum);
                nResult = MiMSSqlSwitchIndex(pHandle, 1);
                if (nResult == WAVETOP_BACKUP_FILE_NOT_EXIST) {
#if defined(WIN32) || defined(WINDOWS)
#ifdef _DEBUG
                    printf("[Thd: %5d] ReadRecord index file %d to end. Wait...\n", 
                        GetCurrentThreadId(), pHandle->nCurNum);
#endif
#endif
                    nResult = WAVETOP_BACKUP_END;
                    goto FAILED;
                }
                else if (nResult != WAVETOP_BACKUP_OK)
                    goto FAILED;

#if defined(WIN32) || defined(WINDOWS)
#ifdef _DEBUG
                printf("[Thd: %5d] Switch to index file %d for Reading.\n", 
                    GetCurrentThreadId(), pHandle->nCurNum);
#endif
#endif

                pIndexFile = pHandle->hIndxFile;
                pDataFile  = pHandle->hFile;
                continue;
            }
            else if (nResult != WAVETOP_BACKUP_OK) {
                LOG_TRACE1("Read index record failed. FileNo: %d\n", 
                    pHandle->nCurNum);
                goto FAILED;
            }

            VINT32(nInSock, Index.szSocket);
            if (nInSock != nSocket)
                continue;
            else
                break;
        }
    }
    else {
READRECORD2:
        nResult = pIndexFile->Next(Index);
        if (nResult == WAVETOP_BACKUP_END) {
            nResult = MiMSSqlSwitchIndex(pHandle, 1);
            if (nResult == WAVETOP_BACKUP_FILE_NOT_EXIST) {
#if defined(WIN32) || defined(WINDOWS)
#ifdef _DEBUG
                printf("[Thd: %5d] ReadRecord index file %d to end. Wait...\n", 
                    GetCurrentThreadId(), pHandle->nCurNum);
#endif
#endif
                nResult = WAVETOP_BACKUP_END;
                goto FAILED;
            }
            else if (nResult != WAVETOP_BACKUP_OK)
                goto FAILED;
#if defined(WIN32) || defined(WINDOWS)
#ifdef _DEBUG
            printf("[Thd: %5d] Switch to index file %d for Reading.\n", 
                GetCurrentThreadId(), pHandle->nCurNum);
#endif
#endif
            pIndexFile = pHandle->hIndxFile;
            pDataFile  = pHandle->hFile;
            goto READRECORD2;
        }
        else if (nResult != WAVETOP_BACKUP_OK) {
            LOG_TRACE1("Read index record failed. FileNo: %d\n", 
                pHandle->nCurNum);
            goto FAILED;
        }

    }

    VINT32(nInSock,             Index.szSocket);
    VINT64(pIndex->nSeqNum,     Index.szSeqNum);
    VINT32(nPos,                Index.szPos);
    VINT32(nRecordSize,         Index.szSize);
    VINT32(pIndex->nRecordTime, Index.szTime);
    strcpy(pIndex->szIP,        Index.szIP);
    pIndex->cType             = Index.cType;
    pIndex->nSocket           = nInSock;
    pIndex->nDataSize         = nRecordSize;

#if defined(WIN32) || defined(WINDOWS)
#ifdef _DEBUG
    printf("[Thd: %5d] ReadRecord seq=%d from index file %d.\n", 
        GetCurrentThreadId(), (unsigned long)pIndex->nSeqNum, pHandle->nCurNum);
#endif
#endif

    /** 读取数据 **/
    nResult = pDataFile->Seek(WAVETOP_LOGFILE_BEGIN, nPos);
    if (nResult != WAVETOP_BACKUP_OK)
        goto FAILED;

    nResult = pDataFile->Read(szBuf, sizeof(long), &nBytes);
    if (nResult != WAVETOP_BACKUP_OK)
        goto FAILED;

    VINT32(nBytes, szBuf);

    assert(nBytes == nRecordSize);
    if (nBytes != nRecordSize) {
        nResult = WAVETOP_BACKUP_INVALID_SYNTAX;
        goto FAILED;
    }

    if (nRecordSize > nSize) {
        if (pDataFile->Read(pszBuffer, nSize, &nBytes) != WAVETOP_BACKUP_OK)
            goto FAILED;

        *pnReadBytes = nSize;
        nResult = WAVETOP_MIRROR_MSSQL2_LOG_MORE_WITH_LINE;

        pIndexFile->m_nMoreSize = nRecordSize - nSize;
    }
    else {
        if (pDataFile->Read(pszBuffer, nRecordSize, &nBytes) != WAVETOP_BACKUP_OK)
            goto FAILED;
        *pnReadBytes = nRecordSize;
        nResult = WAVETOP_BACKUP_OK;
    }

FAILED:
    MiMSSqlReleseMutex(pHandle);

    return nResult;
}

/** 
 * 切换索引和数据文件句柄,并且删除当前已经读完的前一个文件,只支持:1 -- 顺序 by:hugx
 * @[in]
 * nDirect - 切换方向. 1 -- 顺序; 2 -- 反向.
 **/
static int BkMSSqlSwitchIndex(LogHandleSt *pHandle, int nDirect)
{
    char            szPath[1024];
    char            szFileNo[16];
    CDataLogFile   *pOldData;
    CIndexLogFile  *pOldIndx;
    int             nRet;
    int             nOptions;
    int             nOldNum;
    int                nDeleteNum;

    assert(pHandle->nLoopIndexCount >= 0);

    pOldData            = pHandle->hFile;
    pOldIndx            = pHandle->hIndxFile;
    pHandle->hFile      = NULL;
    pHandle->hIndxFile  = NULL;

    nOldNum = pHandle->nCurNum;
    nDeleteNum = nOldNum;

    if (nDirect == 1)
        pHandle->nCurNum++;
    else if (nDirect == 2)
        pHandle->nCurNum--;

    if (pHandle->nCurNum < 0) {
        pHandle->nCurNum = 0;
        nRet = WAVETOP_BACKUP_FILE_NOT_EXIST;
        
        goto FAILED;
    }
    if (pHandle->nCurNum >= pHandle->nLoopIndexCount)
        pHandle->nCurNum = 0;

    /* 
     * if (pHandle->nFirstNo == pHandle->nCurNum) {
     *   LOG_TRACE1("Switch loop. No: %d.\n", 
     *       pHandle->nCurNum);
     *   return WAVETOP_BACKUP_FILE_NOT_EXIST;
     * }
     */

    if (pHandle->nOperation == 1)
        nOptions = WAVETOP_LOGFILE_RDONLY;
    else
        nOptions = WAVETOP_LOGFILE_CREATE_FILE | WAVETOP_LOGFILE_RDWR;

    if (pHandle->szSuff[0] != '\0') {
        sprintf(szPath, "%s%s_%s.%06d", pHandle->szFileDir, 
            WAVETOP_LOGFILE_DATA, pHandle->szSuff, pHandle->nCurNum);
    }
    else {
        sprintf(szPath, "%s%s_0.%06d", pHandle->szFileDir, 
        WAVETOP_LOGFILE_DATA, pHandle->nCurNum);
    }

    if (pHandle->hFile != NULL) {
        pHandle->hFile->Close();
    }

    pHandle->hFile = new CDataLogFile();
    pHandle->hFile->SetNo(pHandle->nCurNum);
    nRet = pHandle->hFile->Open(szPath, 
        nOptions,
        0644);
    if (nRet != WAVETOP_BACKUP_OK)
        goto FAILED;

    if (pHandle->szSuff[0] != '\0') {
        sprintf(szPath, "%s%s_%s.%06d", pHandle->szFileDir, 
            WAVETOP_LOGFILE_INDEX, pHandle->szSuff, pHandle->nCurNum);
    }
    else {
        sprintf(szPath, "%s%s_0.%06d", pHandle->szFileDir, 
            WAVETOP_LOGFILE_INDEX, pHandle->nCurNum);
    }

    if (pHandle->hIndxFile != NULL) {
        pHandle->hIndxFile->Close();
    }

    pHandle->hIndxFile = new CIndexLogFile();
    pHandle->hIndxFile->SetNo(pHandle->nCurNum);
    nRet = pHandle->hIndxFile->Open(szPath, 
        nOptions,
        0644);
    if (nRet != WAVETOP_BACKUP_OK)
        goto FAILED;

    /** 仅当写方式才保存文件序号 **/
    if (pHandle->nOperation == 2) {
        sprintf(szFileNo, "%d",   pHandle->nCurNum);

        if (pHandle->szSuff[0] != '\0') {
            sprintf(szPath, "%s%s_%s.bin", pHandle->szFileDir, 
                WAVETOP_LOGFILE_CONF, pHandle->szSuff);
        }
        else {
            sprintf(szPath, "%s%s_0.bin", pHandle->szFileDir, 
                WAVETOP_LOGFILE_CONF);
        }

#if defined(WIN32) || defined(WINDOWS)
        WritePrivateProfileString("mssql2", "FileNum", szFileNo, szPath);
#else
        CConfigfile *pCfg;
        pCfg = new CConfigfile(szPath);
        pCfg->WriteString("mssql2", "FileNum", szFileNo);
        delete pCfg;
#endif
    }
    
FAILED:
    if (nRet != WAVETOP_BACKUP_OK) {
#if defined(WIN32) || defined(WINDOWS)
#ifdef _DEBUG
        printf("[Thd: %5d] Switch to index file %d failed!!!.\n", 
            GetCurrentThreadId(), pHandle->nCurNum);
#endif
#endif
        pHandle->nCurNum = nOldNum;

        if (pHandle->hFile != NULL)
            delete pHandle->hFile;
        if (pHandle->hIndxFile != NULL)
            delete pHandle->hIndxFile;

        pHandle->hFile     = pOldData;
        pHandle->hIndxFile = pOldIndx;
    }
    else {
        delete pOldIndx;
        delete pOldData;
    }
    
    /* 删除当前已经读完的前一个文件,只支持:1 -- 顺序 by:hugx */
    if (nDirect == 1) {
        nDeleteNum--;
        try {
            if (nDeleteNum >= 0) {
                BkMSSqlDeleteFile(pHandle, nDeleteNum);
            }
        }
        catch (...) {
            /*
            LOG_TRACE1("BkMSSqlSwitchIndex : Exception has been thrown, code: %d", 
                GetLastError());
                */
        }
    }

    return nRet;
}

/**
 * 一次性加载指定8M对应数据文件到内存
 * pDataFile    -数据文件的句柄
 * nLoadPos     -加载文件的位置
 * pszIndx      -加载文件的缓冲区
 * 出现异常返回WAVETOP_BACKUP_INTERNAL_ERROR，否则返回WAVETOP_BACKUP_OK
 */

int LoadDataToInBuffer(CDataLogFile *pDataFile, unsigned long nLoadPos, char **pszData)
{
    unsigned long       nPos            = 0;
    unsigned long       nBytesWritten   = 0;
    int                 nRet            = WAVETOP_BACKUP_OK;

    /* free it when */
    if (!pDataFile->m_bLoadData) {
        *pszData = (char *)malloc(WAVETOP_MIRROR_INBUFFER_DATASIZE);
        if (NULL == *pszData) {
            nRet = WAVETOP_BACKUP_INTERNAL_ERROR;
            goto FAILED;
        }
    }
    memset(*pszData, '\0', WAVETOP_MIRROR_INBUFFER_DATASIZE);

    /* move to head */
    pDataFile->GetPosition(&nPos);
    pDataFile->Seek(WAVETOP_LOGFILE_BEGIN, nLoadPos);
    if (nRet != WAVETOP_BACKUP_OK) {
        nRet = WAVETOP_BACKUP_INTERNAL_ERROR;
        goto FAILED;
    }

    /* read data to buf */
    nRet = pDataFile->Read(*pszData, WAVETOP_MIRROR_INBUFFER_DATASIZE, &nBytesWritten);
    if (nRet != WAVETOP_BACKUP_OK && nRet != WAVETOP_BACKUP_END) {
        nRet = WAVETOP_BACKUP_INTERNAL_ERROR;
        goto FAILED;
    }
    pDataFile->m_nLoadEndPos = nBytesWritten;
    if (nRet == WAVETOP_BACKUP_END)
        nRet = WAVETOP_BACKUP_OK;

    pDataFile->Seek(WAVETOP_LOGFILE_BEGIN, nPos);
    return nRet;

FAILED:
    if (!*pszData) {
        free(*pszData);
        pszData = NULL;
    }
    pDataFile->Seek(WAVETOP_LOGFILE_BEGIN, nPos);

    return nRet;
}

/**
 * 改进：一次性加载数据和索引文件到内存，减少Io读操作
 * 然后循环调用该方法，获取从指定序号开始至末尾的数据
 * @[in]
 * pHandle - 打开的日志文件的句柄
 * pszBuffer - 待写入缓冲区
 * nSize - 待写入缓冲区长度
 * nSocket - 当不为0时读取指定套接字的下一条数据，当为0时忽略
 * @[out]
 * pnReadBytes - 返回已写入数据的长度
 * pIndex      - 返回索引节点.
 * 如果同一序号读取时缓冲区不够大，先将现有的缓冲区写满
 * 并返回WAVETOP_MIRROR_MORE_WITH_LINE，然后再次调用此方法接着读取
 * 该序号读取完成返回WAVETOP_BACKUP_OK
 * 已读取至末尾返回WAVETOP_MIRROR_LINE_END
 * 出现异常返回WAVETOP_BACKUP_INTERNAL_ERROR
 */
MILOG_EXPORT_(int) MiMSSqlReadBufferFromLogNextEx(MiLogHandle hHandle,
    char *pszBuffer,
    unsigned long nSize,
    unsigned long *pnReadBytes,
    unsigned long nSocket,
    ExIndexSt *pIndex,
    int nDelFalg)
{
    LogHandleSt    *pHandle = (LogHandleSt *)hHandle;
    CIndexLogFile  *pIndexFile;
    CDataLogFile   *pDataFile;
    unsigned long   nResult;
    unsigned long   nPos;
    unsigned long   nFHalf  = 0;
    unsigned long   nInSock;
    unsigned long   nBytes;
    unsigned long   nRecordSize;
    IndexSt         Index;
    char            szBuf[32];
    char           *pszTmp = NULL;
    bool            bReload = false;

    assert(pHandle != NULL);
    assert(pHandle->nOperation == 1);

    if (pHandle->nOperation == 2)
        return WAVETOP_BACKUP_INVALID_SYNTAX;

    if (pHandle->nStatus != WAVETOP_BACKUP_OK)
        return pHandle->nStatus;

    nResult = MiMSSqlWaitForMutex(pHandle);
    if (nResult != WAVETOP_BACKUP_OK)
        return WAVETOP_BACKUP_INVALID_SYNTAX;

    pHandle->nWay = 1;
    pIndexFile = pHandle->hIndxFile;
    pDataFile  = pHandle->hFile;

    if (pIndexFile->m_nMoreSize > 0) {
        if (pIndexFile->m_bOldType) {
            nRecordSize = pIndexFile->m_nMoreSize > nSize ? nSize : pIndexFile->m_nMoreSize;
            if (pDataFile->Read(pszBuffer, nRecordSize, &nBytes) != WAVETOP_BACKUP_OK)
                goto FAILED;

            *pnReadBytes = nRecordSize;

            if (pIndexFile->m_nMoreSize > nSize) {
                pIndexFile->m_nMoreSize -= nRecordSize;
                nResult = WAVETOP_MIRROR_MSSQL2_LOG_MORE_WITH_LINE;
            }
            else {
                pIndexFile->m_nMoreSize = 0;
                pIndexFile->m_bOldType = false;
                nResult = WAVETOP_BACKUP_OK;
            }
            goto FAILED;
        }
        else {
            nRecordSize = pIndexFile->m_nMoreSize > nSize ? nSize : pIndexFile->m_nMoreSize;
            *pnReadBytes = nRecordSize;

            pszTmp = pDataFile->m_pszDataBuf;
            pszTmp += pDataFile->m_nDataPos;
            memcpy(pszBuffer, pszTmp, nRecordSize);            
            pDataFile->m_nDataPos += nRecordSize;

            if (pIndexFile->m_nMoreSize > nSize) {
                pIndexFile->m_nMoreSize -= nRecordSize;
                nResult = WAVETOP_MIRROR_MSSQL2_LOG_MORE_WITH_LINE;
            }
            else {
                pIndexFile->m_nMoreSize = 0;
                pIndexFile->m_bOldType = false;
                nResult = WAVETOP_BACKUP_OK;
            }
            goto FAILED;
        }
    }

    /** 找到正确的Index **/
    if (nSocket != 0) {
        for (;;) {
            nResult = pIndexFile->Next(Index);
            if (nResult == WAVETOP_BACKUP_END) {
                LOG_TRACE1("Switch to index file %d when Reading index.\n",
                    pHandle->nCurNum);
                nResult = MiMSSqlSwitchIndex(pHandle, 1);
                if (nResult == WAVETOP_BACKUP_FILE_NOT_EXIST) {
#if defined(WIN32) || defined(WINDOWS)
#ifdef _DEBUG
                    printf("[Thd: %5d] ReadRecord index file %d to end. Wait...\n", 
                        GetCurrentThreadId(), pHandle->nCurNum);
#endif
#endif
                    nResult = WAVETOP_BACKUP_END;
                    goto FAILED;
                }
                else if (nResult != WAVETOP_BACKUP_OK)
                    goto FAILED;

#if defined(WIN32) || defined(WINDOWS)
#ifdef _DEBUG
                printf("[Thd: %5d] Switch to index file %d for Reading.\n", 
                    GetCurrentThreadId(), pHandle->nCurNum);
#endif
#endif

                pIndexFile = pHandle->hIndxFile;
                pDataFile  = pHandle->hFile;
                continue;
            }
            else if (nResult != WAVETOP_BACKUP_OK) {
                LOG_TRACE1("Read index record failed. FileNo: %d\n", 
                    pHandle->nCurNum);
                goto FAILED;
            }

            VINT32(nInSock, Index.szSocket);
            if (nInSock != nSocket)
                continue;
            else
                break;
        }
    }
    else {
READRECORD2:
        nResult = pIndexFile->Next(Index);
        if (nResult == WAVETOP_BACKUP_END) {    
            if (nDelFalg == 0) {
                nResult = MiMSSqlSwitchIndex(pHandle, 1);
            }
            else {
                nResult = BkMSSqlSwitchIndex(pHandle, 1);
            }
            if (nResult == WAVETOP_BACKUP_FILE_NOT_EXIST) {
#if defined(WIN32) || defined(WINDOWS)
#ifdef _DEBUG
                printf("[Thd: %5d] ReadRecord index file %d to end. Wait...\n", 
                    GetCurrentThreadId(), pHandle->nCurNum);
#endif
#endif
                nResult = WAVETOP_BACKUP_END;
                goto FAILED;
            }
            else if (nResult != WAVETOP_BACKUP_OK)
                goto FAILED;
#if defined(WIN32) || defined(WINDOWS)
#ifdef _DEBUG
            printf("[Thd: %5d] Switch to index file %d for Reading.\n", 
                GetCurrentThreadId(), pHandle->nCurNum);
#endif
#endif
            pIndexFile = pHandle->hIndxFile;
            pDataFile  = pHandle->hFile;
            goto READRECORD2;
        }
        else if (nResult != WAVETOP_BACKUP_OK) {
            LOG_TRACE1("Read index record failed. FileNo: %d\n", 
                pHandle->nCurNum);
            goto FAILED;
        }
    }

    VINT32(nInSock,             Index.szSocket);
    VINT64(pIndex->nSeqNum,     Index.szSeqNum);
    VINT32(nPos,                Index.szPos);
    VINT32(nRecordSize,         Index.szSize);
    VINT32(pIndex->nRecordTime, Index.szTime);
    strcpy(pIndex->szIP,        Index.szIP);
    pIndex->cType             = Index.cType;
    pIndex->nSocket           = nInSock;
    pIndex->nDataSize         = nRecordSize;

#if defined(WIN32) || defined(WINDOWS)
#ifdef _DEBUG
    printf("[Thd: %5d] ReadRecord seq=%d from index file %d.\n", 
        GetCurrentThreadId(), (unsigned long)pIndex->nSeqNum, pHandle->nCurNum);
#endif
#endif

    if (nRecordSize > WAVETOP_MIRROR_INBUFFER_DATASIZE)
        goto OLDWAY;
    
    /* first load */
    if (!pDataFile->m_bLoadData) {
        nResult = LoadDataToInBuffer(pDataFile, nPos, &pDataFile->m_pszDataBuf);
        if (WAVETOP_BACKUP_OK != nResult) {
            nResult = WAVETOP_BACKUP_INVALID_SYNTAX;
            goto FAILED;
        }
        pDataFile->m_bLoadData = true;
        pDataFile->m_nLoadPos = nPos;
    }

    /* make sure a full data */
    bReload = (nPos < pDataFile->m_nLoadPos)
        ||(nPos > (pDataFile->m_nLoadPos + pDataFile->m_nLoadEndPos))
        ||((pDataFile->m_nLoadPos + pDataFile->m_nLoadEndPos) < (nPos + nRecordSize));

    if (bReload) {
        nResult = LoadDataToInBuffer(pDataFile, nPos, &pDataFile->m_pszDataBuf);
        if (WAVETOP_BACKUP_OK != nResult) {
            nResult = WAVETOP_BACKUP_INVALID_SYNTAX;
            goto FAILED;
        }
        pDataFile->m_nLoadPos = nPos;
        pDataFile->m_nDataPos = 0;
    }

    pszTmp = pDataFile->m_pszDataBuf;
    pszTmp += (nPos - pDataFile->m_nLoadPos);

    memcpy(szBuf, pszTmp, sizeof(unsigned long));
    VINT32(nBytes, szBuf);
    assert(nBytes == nRecordSize);
    if (nBytes != nRecordSize) {
        nResult = WAVETOP_BACKUP_INVALID_SYNTAX;
        goto FAILED;
    }
    pszTmp += sizeof(unsigned long);
    pDataFile->m_nDataPos += 4;

    if (nRecordSize > nSize) {
        memcpy(pszBuffer, pszTmp, nSize);
        *pnReadBytes = nSize;
        nResult = WAVETOP_MIRROR_MSSQL2_LOG_MORE_WITH_LINE;
        pIndexFile->m_nMoreSize = nRecordSize - nSize;
        pDataFile->m_nDataPos += nSize;
    }
    else {
        memcpy(pszBuffer, pszTmp, nRecordSize);
        *pnReadBytes = nRecordSize;
        pDataFile->m_nDataPos += nRecordSize;
        nResult = WAVETOP_BACKUP_OK;
    }
    goto FAILED;

OLDWAY:
    /** 读取数据 **/
    nResult = pDataFile->Seek(WAVETOP_LOGFILE_BEGIN, nPos);
    if (nResult != WAVETOP_BACKUP_OK)
        goto FAILED;

    nResult = pDataFile->Read(szBuf, sizeof(unsigned long), &nBytes);
    if (nResult != WAVETOP_BACKUP_OK)
        goto FAILED;

    VINT32(nBytes, szBuf);

    assert(nBytes == nRecordSize);
    if (nBytes != nRecordSize) {
        nResult = WAVETOP_BACKUP_INVALID_SYNTAX;
        goto FAILED;
    }

    if (nRecordSize > nSize) {
        if (pDataFile->Read(pszBuffer, nSize, &nBytes) != WAVETOP_BACKUP_OK)
            goto FAILED;

        *pnReadBytes = nSize;
        nResult = WAVETOP_MIRROR_MSSQL2_LOG_MORE_WITH_LINE;
        pIndexFile->m_bOldType = true;
        pIndexFile->m_nMoreSize = nRecordSize - nSize;
    }
    else {
        if (pDataFile->Read(pszBuffer, nRecordSize, &nBytes) != WAVETOP_BACKUP_OK)
            goto FAILED;
        *pnReadBytes = nRecordSize;
        nResult = WAVETOP_BACKUP_OK;
    }

FAILED:
    MiMSSqlReleseMutex(pHandle);

    return nResult;
}


/**
 * 循环调用该方法，获取从指定序号开始至头部的数据
 * @[in]
 * pHandle - 打开的日志文件的句柄
 * pszBuffer - 待写入缓冲区
 * nSize - 待写入缓冲区长度
 * nSocket - 当不为0时读取指定套接字的上一条数据，当为0时忽略
 * @[out]
 * pnReadBytes - 返回已写入数据的长度
 * pIndex      - 返回索引节点.
 * 如果同一序号读取时缓冲区不够大，先将现有的缓冲区写满
 * 并返回WAVETOP_MIRROR_MORE_WITH_LINE，然后再次调用此方法接着读取。
 * 该序号读取完成返回WAVETOP_BACKUP_OK
 * 已读取至头部返回WAVETOP_MIRROR_LINE_END
 * 出现异常返回WAVETOP_BACKUP_INTERNAL_ERROR
 */
MILOG_EXPORT_(int) MiMSSqlReadBufferFromLogPrev(MiLogHandle hHandle,
                                                char *pszBuffer,
                                                unsigned long nSize,
                                                unsigned long *pnReadBytes,
                                                unsigned long nSocket,
                                                ExIndexSt *pIndex)
{
    LogHandleSt    *pHandle = (LogHandleSt *)hHandle;
    CIndexLogFile  *pIndexFile;
    CDataLogFile   *pDataFile;
    unsigned long   nPos;
    unsigned long   nInSock;
    unsigned long   nBytes;
    unsigned long   nRecordSize;
    IndexSt         Index;
    char            szBuf[32];
    int             nResult;
    int             nSwitch;

    assert(pHandle != NULL);
    assert(pHandle->nOperation == 1);

    if (pHandle->nOperation == 2)
        return WAVETOP_BACKUP_INVALID_SYNTAX;

    if (pHandle->nStatus != WAVETOP_BACKUP_OK)
        return pHandle->nStatus;

    nResult = MiMSSqlWaitForMutex(pHandle);
    if (nResult != WAVETOP_BACKUP_OK)
        return WAVETOP_BACKUP_INTERNAL_ERROR;

    pHandle->nWay = 2;
    pIndexFile = pHandle->hIndxFile;
    pDataFile  = pHandle->hFile;

    nSwitch    = 0;

    if (pIndexFile->m_nMoreSize > 0) {
        nRecordSize = pIndexFile->m_nMoreSize > nSize ? nSize : pIndexFile->m_nMoreSize;
        if (pDataFile->Read(pszBuffer, nRecordSize, &nBytes) != WAVETOP_BACKUP_OK)
            goto FAILED;

        *pnReadBytes = nRecordSize;
        
        if (pIndexFile->m_nMoreSize > nSize) {
            pIndexFile->m_nMoreSize -= nRecordSize;
            nResult = WAVETOP_MIRROR_MSSQL2_LOG_MORE_WITH_LINE;
        }
        else {
            pIndexFile->m_nMoreSize = 0;
            nResult = WAVETOP_BACKUP_OK;
        }

        goto FAILED;
    }
    
    /** 找到正确的Index **/
    if (nSocket != 0) {
        for (;;) {
            nResult = pIndexFile->Next2(Index);
            if (nResult == WAVETOP_BACKUP_END) {
                LOG_TRACE1("The index file %d pointer is wrong position.\n", 
                    pIndexFile->GetNo());
                goto FAILED;
            }
            else if (nResult != WAVETOP_BACKUP_OK) {
                LOG_TRACE1("Read index record failed. FileNo: %d\n", 
                    pHandle->nCurNum);
                goto FAILED;
            }

            nResult = pIndexFile->Prev();
            nResult = pIndexFile->Prev();
            if (nResult == WAVETOP_BACKUP_END) {
                nSwitch = 1;
                /* 如果直接goto FAILED则第一条记录不读 */
                VINT32(nInSock, Index.szSocket);
                if (nInSock != nSocket)
                    goto FAILED;
                else
                    break;
            }
            else {
                VINT32(nInSock, Index.szSocket);
                if (nInSock != nSocket)
                    continue;
                else
                    break;
            }    
        }
    }
    else {
        nResult = pIndexFile->Prev(Index);
        if (nResult == WAVETOP_BACKUP_END) {
            nSwitch = 1;
            nResult = WAVETOP_BACKUP_OK;
        }
        else if (nResult != WAVETOP_BACKUP_OK) {
            LOG_TRACE1("Read index record failed. FileNo: %d\n", 
                pHandle->nCurNum);
            goto FAILED;
        }
    }

    VINT32(nInSock,             Index.szSocket);
    VINT64(pIndex->nSeqNum,     Index.szSeqNum);
    VINT32(nPos,                Index.szPos);
    VINT32(nRecordSize,         Index.szSize);
    VINT32(pIndex->nRecordTime, Index.szTime);
    strcpy(pIndex->szIP,        Index.szIP);
    pIndex->cType             = Index.cType;
    pIndex->nSocket           = nInSock;
    pIndex->nDataSize         = nRecordSize;

    /** 读取数据 **/
    nResult = pDataFile->Seek(WAVETOP_LOGFILE_BEGIN, nPos);
    if (nResult != WAVETOP_BACKUP_OK)
        goto FAILED;

    nResult = pDataFile->Read(szBuf, sizeof(long), &nBytes);
    if (nResult != WAVETOP_BACKUP_OK)
        goto FAILED;

    VINT32(nBytes, szBuf);

    assert(nBytes == nRecordSize);
    if (nBytes != nRecordSize) {
        nResult = WAVETOP_BACKUP_INVALID_SYNTAX;
        goto FAILED;
    }

    if (nRecordSize > nSize) {
        if (pDataFile->Read(pszBuffer, nSize, &nBytes) != WAVETOP_BACKUP_OK)
            goto FAILED;

        *pnReadBytes = nSize;
        nResult = WAVETOP_MIRROR_MSSQL2_LOG_MORE_WITH_LINE;

        pIndexFile->m_nMoreSize = nRecordSize - nSize;
    }
    else {
        if (pDataFile->Read(pszBuffer, nRecordSize, &nBytes) != WAVETOP_BACKUP_OK)
            goto FAILED;
        *pnReadBytes = nRecordSize;
        nResult = WAVETOP_BACKUP_OK;
    }

FAILED:
    if (nSwitch == 1) {
        nResult = MiMSSqlSwitchIndex(pHandle, 2);
        if (nResult == WAVETOP_BACKUP_FILE_NOT_EXIST) {
#if defined(WIN32) || defined(WINDOWS)
#ifdef _DEBUG
            printf("[Thd: %5d] ReadRecord index file %d to first. Wait...\n", 
                GetCurrentThreadId(), pHandle->nCurNum);
#endif
#endif
            pHandle->nStatus = nResult;
            nResult = WAVETOP_BACKUP_END;
        }
        else if (nResult != WAVETOP_BACKUP_OK) {
            pHandle->nStatus = nResult;
            nResult = WAVETOP_BACKUP_INTERNAL_ERROR;
        }
        else {
#if defined(WIN32) || defined(WINDOWS)
#ifdef _DEBUG
            printf("[Thd: %5d] Switch to index file %d for Prev.\n", 
                GetCurrentThreadId(), pHandle->nCurNum);
#endif
#endif
            pHandle->hIndxFile->Last();
        }
        
    }

    MiMSSqlReleseMutex(pHandle);

    return nResult;
}

/**
 * 该方法可以获取日志文件中最大和最小的序号
 * @[in]
 * pHandle - 打开的日志文件的句柄
 * @[out]
 * pnMax - 存放取得的最大序号值
 * pnMin - 存放取得的最小序号值
 * 成功返回WAVETOP_BACKUP_OK
 * 失败返回WAVETOP_BACKUP_INTERNAL_FAILED
 */
MILOG_EXPORT_(int) MiMSSqlGetMaxAndMinNum(MiLogHandle hHandle,
                                          PRUint64 *pnMax,
                                          PRUint64 *pnMin)
{
    LogHandleSt    *pHandle = (LogHandleSt *)hHandle;
    CIndexLogFile  *pIndexFile = NULL;
    char            szPath[1024];
    char            szPathTemp[1024] = { 0 };
    struct stat     statBuf;
    PRUint64        nEndSeq = 0;
    PRUint64        nStartSeq = 0;
    int             nRet;
    int             nFile;
    int             nLastNum = 0;

    nRet = MiMSSqlWaitForMutex(pHandle);
    if (nRet != WAVETOP_BACKUP_OK)
        return WAVETOP_BACKUP_INTERNAL_ERROR;

    if (pHandle->szSuff[0] != '\0') {
        sprintf(szPathTemp, "%s%s_%s.bin", pHandle->szFileDir, 
            WAVETOP_LOGFILE_CONF, pHandle->szSuff);
    }
    else {
        sprintf(szPathTemp, "%s%s_0.bin", pHandle->szFileDir, 
            WAVETOP_LOGFILE_CONF);
    }
    
    /* 如果是正常打开,则要读取文件序号 */
#if defined(WIN32) || defined(WINDOWS)
    nLastNum = GetPrivateProfileInt("mssql2", "FileNum", 0, szPathTemp);
#else
    CConfigfile *pCfg;
    pCfg = new CConfigfile(szPathTemp);
    if(pCfg != NULL) {
        nLastNum = pCfg->ReadInt("mssql2", "FileNum", 0);
        delete pCfg;
    }
#endif

    if (pHandle->szSuff[0] != '\0') {
        sprintf(szPath, "%s%s_%s.%06d", pHandle->szFileDir, 
            WAVETOP_LOGFILE_INDEX, pHandle->szSuff, nLastNum);
    }
    else {
        sprintf(szPath, "%s%s_0.%06d", pHandle->szFileDir, 
            WAVETOP_LOGFILE_INDEX, nLastNum);
    }

    if (stat(szPath, &statBuf) == -1) {
        nRet = WAVETOP_BACKUP_INTERNAL_ERROR;
        goto End;
    }

    pIndexFile = new CIndexLogFile();
    if (pIndexFile == NULL) {
        nRet = WAVETOP_BACKUP_INTERNAL_ERROR;
        goto End;
    }

    nRet = pIndexFile->Open(szPath, WAVETOP_LOGFILE_RDONLY, 0644);
    if (nRet != WAVETOP_BACKUP_OK) {
        goto End;
    }

    nEndSeq = pIndexFile->GetEndSeq();
    if (nEndSeq < 0) {
        nRet = WAVETOP_BACKUP_INTERNAL_ERROR;
        goto End;
    }

    pIndexFile->Close();

    for (nFile = 0; nFile < pHandle->nLoopIndexCount; nFile++) {
        if (pHandle->szSuff[0] != '\0') {
            sprintf(szPath, "%s%s_%s.%06d", pHandle->szFileDir, 
                WAVETOP_LOGFILE_INDEX, pHandle->szSuff, nFile);
        }
        else {
            sprintf(szPath, "%s%s_0.%06d", pHandle->szFileDir, 
                WAVETOP_LOGFILE_INDEX, nFile);
        }

        if (stat(szPath, &statBuf) != -1) {
            break;
        }
    }

    nRet = pIndexFile->Open(szPath, WAVETOP_LOGFILE_RDONLY, 0644);
    if (nRet != WAVETOP_BACKUP_OK) {
        goto End;
    }

    nStartSeq = pIndexFile->GetStartSeq();
    if (nStartSeq < 0 || nStartSeq > nEndSeq) {
        nRet = WAVETOP_BACKUP_INTERNAL_ERROR;
        goto End;
    }

End:
    if (pIndexFile != NULL) {
        delete pIndexFile;
        pIndexFile = NULL;
    }

    if (nRet == WAVETOP_BACKUP_OK) {
        *pnMax = nEndSeq;
        *pnMin = nStartSeq;
    }

    MiMSSqlReleseMutex(pHandle);

    return nRet;
}


MILOG_EXPORT_(int) MiMSSqlDeleteLog(MiLogHandle hHandle)
{
#if defined(WIN32) || defined(WINDOWS)
    LogHandleSt *pHandle = (LogHandleSt *)hHandle;
    DWORD        dwResult;
    DWORD        dwThreadId;
    
    if (pHandle == NULL) {
        return WAVETOP_BACKUP_INTERNAL_ERROR;
    }
    
    dwResult = WaitForSingleObject(pHandle->hThread, 0);
    if (dwResult == WAIT_OBJECT_0) {
        pHandle->hThread = CreateThread(NULL, 0, MiMSSqlMMShareThread, 
            (void *)pHandle, 0, &dwThreadId);
        if (pHandle->hThread == NULL) {
            LOG_TRACE1("MiMSSqlAutoDeleteFile() failed in CreateThread(), "
                "error: %d", GetLastError());
            return WAVETOP_BACKUP_INTERNAL_ERROR;
        }
    }
    else if (dwResult == WAIT_FAILED) {
        return WAVETOP_BACKUP_INTERNAL_ERROR;
    }
    
    if (pHandle->nDelete != 1) {
        pHandle->nDelete = 1;
    }
#endif
    return WAVETOP_BACKUP_OK;
}

#if defined(WINDOWS) || defined(WIN32)
MILOG_EXPORT_(int) BkMSSqlWriteShareMM(MiLogHandle hHandle, PRInt32 nWay)
{
    LogHandleSt *pHandle = (LogHandleSt *)hHandle;
    LogMMShareNodeSt     *pMMShareNode;
    bool                  bIsFirst = true;
    int                   nCount;
    int                   nNodeLimit;
    int                   nResult = WAVETOP_BACKUP_OK;
    char                  szMMSharefile[64] = { 0 };
    char                  szMutexName[64] = { 0 };
    HANDLE                hFileMap = NULL;
    HANDLE                hMutex   = NULL;
    PVOID                 pView    = NULL;
    DWORD                 dwResult;
    PSECURITY_ATTRIBUTES  sa;
    
    if (pHandle == NULL) {
        return WAVETOP_BACKUP_INTERNAL_ERROR;
    }

    nResult = MiMSSqlWaitForMutex(pHandle);
    if (nResult != WAVETOP_BACKUP_OK)
        return WAVETOP_BACKUP_INTERNAL_ERROR;
    
    MiMSSqlGetNames(pHandle, szMMSharefile, szMutexName);
    
    /* 创建共享内存 */
    sa = GetNullACL();
    hMutex = CreateMutex(sa, FALSE, szMutexName);
    if (NULL == hMutex) {
        LOG_TRACE1("MiMSSqlMMShareThread : CreateMutex failed, code: %d", 
            GetLastError());
        goto END;
    }
    
    while (pHandle->nClose != 1) {
        dwResult = WaitForSingleObject(hMutex, 3000);
        if (dwResult == WAIT_TIMEOUT) {
            continue;
        }
        else if (dwResult == WAIT_OBJECT_0) {
            break;
        }
        else if (dwResult == WAIT_FAILED) {
            LOG_TRACE1("MiMSSqlMMShareThread : WaitForSingleObject failed, code: %d", 
                GetLastError());
            goto END;
        }
    }
    
    if (pHandle->nClose == 1) {
        goto END;
    }
        
    hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE, sa, PAGE_READWRITE, 0,
        WAVETOP_LOGFILE_MAP_SIZE, TEXT(szMMSharefile));
    if (hFileMap == NULL) {
        LOG_TRACE1("MiMSSqlMMShareThread : CreateFileMapping failed, code: %d",
            GetLastError());
        goto END;
    }
    
    /* 判断是否是第一个使用者 */
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        bIsFirst = false;
    }
    
    /* 将共享内存映射到当前进程 */
    pView = MapViewOfFile(hFileMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0,
        WAVETOP_LOGFILE_MAP_SIZE);
    if (pView == NULL) {
        LOG_TRACE1("MiMSSqlMMShareThread : MapViewOfFile failed, code: %d", 
            GetLastError());
        goto END;
    }
    
    nNodeLimit = WAVETOP_LOGFILE_MAP_SIZE / sizeof(LogMMShareNodeSt);
    
    /* 寻找未使用的节点 */
    for (nCount = 0; nCount < nNodeLimit; nCount++) {
        pMMShareNode = (LogMMShareNodeSt *)((unsigned char*)pView + 
            nCount * sizeof(LogMMShareNodeSt));
        if (pMMShareNode->bIsUsed == 0) {
            pMMShareNode->bIsUsed = 1;
            pMMShareNode->nWay = nWay;
            pMMShareNode->nLastTime = time(NULL);
            pMMShareNode->nCurNum = pHandle->nCurNum;
            break;
        }
    }

    ReleaseMutex(hMutex);

END: 
    MiMSSqlReleseMutex(pHandle);

    CleanNullACL(sa);
    if (pView != NULL)
        UnmapViewOfFile(pView);
    if (hFileMap != NULL)
        CloseHandle(hFileMap);
    if (hMutex != NULL)
        CloseHandle(hMutex);
    
    return WAVETOP_BACKUP_OK;
}
#endif

MILOG_EXPORT_(int) BkMSSqlOpenShareMM(MiLogHandle hHandle, PRInt32 nWay, PRInt32 *nStatus)
{
    LogHandleSt *pHandle = (LogHandleSt *)hHandle;
    LogMMShareNodeSt     *pMMShareNode;
    bool                  bIsFirst = true;
    int                   nCount;
    int                   nNodeLimit;
    int                   nResult = WAVETOP_BACKUP_OK;
    char                  szMMSharefile[64] = { 0 };
    char                  szMutexName[64] = { 0 };
#if defined(WINDOWS) || defined(WIN32)
    HANDLE                hFileMap = NULL;
    HANDLE                hMutex   = NULL;
    PVOID                 pView    = NULL;
#else
    unsigned char * pView = NULL;
#endif
    int                 dwResult;
    
    if (pHandle == NULL) {
        return WAVETOP_BACKUP_INTERNAL_ERROR;
    }

    nResult = MiMSSqlWaitForMutex(pHandle);
    if (nResult != WAVETOP_BACKUP_OK)
        return WAVETOP_BACKUP_INTERNAL_ERROR;
    
    if (pHandle->pView == NULL) {
        /* 初始化的函数内存线程可能还未创建 */
        nResult = WAVETOP_BACKUP_OK;
        goto END;
    }
#if defined(WINDOWS) || defined(WIN32)
    pView = (PVOID)pHandle->pView;
#else
    pView = pHandle->pView;
#endif
    
    nNodeLimit = WAVETOP_LOGFILE_MAP_SIZE / sizeof(LogMMShareNodeSt);
    
    /* 寻找未使用的节点 */
    for (nCount = 0; nCount < nNodeLimit; nCount++) {
        pMMShareNode = (LogMMShareNodeSt *)((unsigned char*)pView + 
            nCount * sizeof(LogMMShareNodeSt));
        if (pMMShareNode->bIsUsed == 1 && pMMShareNode->nWay == nWay) {
            /* 这块共享内存显示已经切换 */            
            if (nWay == WAVETOP_MIRROR_MSSQL2_LOG_BROKEN) {
                pMMShareNode->nCurNum = pHandle->nCurNum + 1;
                pMMShareNode->nWay = WAVETOP_MIRROR_MSSQL2_LOG_SWITCH;
                *nStatus = nWay;
                break;
            }
            
            if (nWay == WAVETOP_MIRROR_MSSQL2_LOG_SWITCH) {
                *nStatus = pMMShareNode->nCurNum;
            }
            else {
                *nStatus = nWay;
            }
            /* 这块共享内存置空 */
            pMMShareNode->bIsUsed = 0;
            pMMShareNode->nWay = 0;
            pMMShareNode->nLastTime = 0;
            pMMShareNode->nCurNum = 0;  
            break;
        }
    }   
    
END: 
    MiMSSqlReleseMutex(pHandle);

    return nResult;
}

/**
* 用于备份系统oracle服务器写入logdata
* 添加时间参数，时间是客户端传过来的
* 而不是现在的获取当前写入时间
* 将缓冲区中的数据写入指定日志文件
* @[in]
* pHandle - 打开的日志文件的句柄
* pszBuffer - 待写入日志文件的数据
* nBuffCount - 待写入日志文件数据的长度
* pIndex - index node，主要用于写备份时间
* @[out]
* 成功返回WAVETOP_BACKUP_OK
* 失败返回WAVETOP_BACKUP_INTERNAL_ERROR
*/
MILOG_EXPORT_(int) BkMSSqlWriteBufferToLog(MiLogHandle hHandle,
                                           WTBUF *pBuffers,
                                           unsigned long nBuffCount,
                                           ExIndexSt *pIndex)
{
    LogHandleSt    *pHandle = (LogHandleSt *)hHandle;
    CIndexLogFile  *pIndexFile;
    CDataLogFile   *pDataFile;
    int             nResult;
    IndexSt         index;
    unsigned long   nTotalSize;
    unsigned long   nFileSize;
    unsigned long   nDataEndPos;
    unsigned long   nItem;
    
    assert(pHandle != NULL);
    assert(pHandle->nOperation == 2);
    
    if (pHandle->nOperation == 1)
        return WAVETOP_BACKUP_INVALID_SYNTAX;
    
    nResult = MiMSSqlWaitForMutex(pHandle);
    if (nResult != WAVETOP_BACKUP_OK)
        return WAVETOP_BACKUP_INTERNAL_ERROR;
    
    pIndexFile = pHandle->hIndxFile;
    pDataFile  = pHandle->hFile;
    
    /* 算出所有缓冲区大小 */
    nTotalSize = 0;
    for (nItem = 0; nItem < nBuffCount; nItem++)
        nTotalSize += pBuffers[nItem].dwBufSize;
    
    pDataFile->GetSize(&nFileSize);
    
    /** 当数据文件超过2GB时也进行转换 **/
    if (   (pIndexFile->GetRecordCount() >= pHandle->nLimitRecord)
        || ((nFileSize + nTotalSize) >= pHandle->nLimitSize)) {
        nResult = MiMSSqlSwitchIndex(pHandle, 1);
#if defined(WIN32) || defined(WINDOWS)
        if (nResult != WAVETOP_BACKUP_OK) {
            LOG_TRACE1("MiMSSqlSwitchIndex() failed in MiMSSqlWriteBufferToLog(),"
                "FAILED Code:%d\n", GetLastError());
            goto FAILED;
        }
#ifdef _DEBUG
        printf("[Thd: %5d] Switch to index file %d for Writing.\n", 
            GetCurrentThreadId(), pHandle->nCurNum);
#endif
#else
        if (nResult != WAVETOP_BACKUP_OK) {
            LOG_TRACE1("MiMSSqlSwitchIndex() failed in MiMSSqlWriteBufferToLog(),"
                "FAILED Code:%d\n", errno);
            goto FAILED;
        }
#endif
        pIndexFile = pHandle->hIndxFile;
        pDataFile  = pHandle->hFile;
    }
    
    /* 首先, 写入数据文件 */
    nDataEndPos = pDataFile->GetTailPos();
    nResult = pDataFile->WriteRecord(pBuffers, nBuffCount);
    if (nResult != WAVETOP_BACKUP_OK) 
        goto FAILED;
    
    /* 第二步, 写入索引,序号,类型,SOCKET */
    XINT32(nDataEndPos,        index.szPos);
    XINT64(pIndex->nSeqNum,    index.szSeqNum);
    XINT32(pIndex->nSocket,    index.szSocket);
    XINT32(nTotalSize,         index.szSize);
    /* 备份时间指客户端发送过来的结构中的时间 */
    XINT32(pIndex->nRecordTime, index.szTime);
    index.cType             =  pIndex->cType;
    strncpy(index.szIP, pIndex->szIP, sizeof(index.szIP));
    nResult = pIndexFile->BkAppendRecord(index);
    if (nResult != WAVETOP_BACKUP_OK) {
        /* Rollback to old position */
        pDataFile->Seek(WAVETOP_LOGFILE_BEGIN, nDataEndPos);
        goto FAILED;
    }
    
#if defined(WIN32) || defined(WINDOWS)
#ifdef _DEBUG
    printf("[Thd: %5d] AppdRecord seq=%d into index file %d success\n",
        GetCurrentThreadId(), (unsigned long)pIndex->nSeqNum, pHandle->nCurNum);
#endif
#endif
    
FAILED:
    MiMSSqlReleseMutex(pHandle);
    
    return nResult;
}

/**
 * 该方法可以获取日志文件的文件名ID编号与最少Seq号
 * @[in]
 * pHandle - 打开的日志文件的句柄
 * nCurrSeq - 上一次全量版本的Seq 
 * @[out]
 * nBufLen - 取得该日志文件的ID编号
 * pnMin - 存放取得的最小序号值(Seq值)
 * 成功返回WAVETOP_BACKUP_OK
 * 失败返回WAVETOP_BACKUP_INTERNAL_FAILED
 */
MILOG_EXPORT_(int) BkMSSqlGetFileName(MiLogHandle hHandle, PRUint64 nCurrSeq, 
                                        int *nBufLen, PRUint64 *pnMin)
{
    LogHandleSt    *pHandle = (LogHandleSt *)hHandle;
    CIndexLogFile  *pIndexFile = NULL;
    char            szPath[1024];
    char            szPathTemp[1024] = { 0 };
    struct stat     statBuf;
    PRUint64        nEndSeq = 0;
    PRUint64        nStartSeq = 0;
    int             nRet;
    int             nFile;
    int             nLastNum = 0;

    nRet = MiMSSqlWaitForMutex(pHandle);
    if (nRet != WAVETOP_BACKUP_OK){
       /* SLogErrorWrite(APLOG_MARK, APLOG_ERR, NULL, 
            "BkMSSqlGetFileName(): Wait for mutex failed"); */
        return WAVETOP_BACKUP_INTERNAL_ERROR;
    }

    if (pHandle->szSuff[0] != '\0') {
        sprintf(szPathTemp, "%s%s_%s.bin", pHandle->szFileDir, 
            WAVETOP_LOGFILE_CONF, pHandle->szSuff);
    }
    else {
        sprintf(szPathTemp, "%s%s_0.bin", pHandle->szFileDir, 
            WAVETOP_LOGFILE_CONF);
    }
    
    /* 如果是正常打开,则要读取文件序号 */
#if defined(WIN32) || defined(WINDOWS)
    nLastNum = GetPrivateProfileInt("mssql2", "FileNum", 0, szPathTemp);
#else
    CConfigfile *pCfg;
    pCfg = new CConfigfile(szPathTemp);
    if(pCfg != NULL) {
        nLastNum = pCfg->ReadInt("mssql2", "FileNum", 0);
        delete pCfg;
    }
#endif

    pIndexFile = new CIndexLogFile();
    if (pIndexFile == NULL) {
         /*SLogErrorWrite(APLOG_MARK, APLOG_ERR, NULL, 
            "BkMSSqlGetFileName(): new CIndexLogFile failed!");*/
        nRet = WAVETOP_BACKUP_INTERNAL_ERROR;
        goto End;
    }

    for (nFile = 0; nFile <= nLastNum; nFile++) {
        if (pHandle->szSuff[0] != '\0') {
            sprintf(szPath, "%s%s_%s.%06d", pHandle->szFileDir, 
                WAVETOP_LOGFILE_INDEX, pHandle->szSuff, nFile);
        }
        else {
            sprintf(szPath, "%s%s_0.%06d", pHandle->szFileDir, 
                WAVETOP_LOGFILE_INDEX, nFile);
        }

        if (stat(szPath, &statBuf) == -1) {
            continue;
        }

        nRet = pIndexFile->Open(szPath, WAVETOP_LOGFILE_RDONLY, 0644);
        if (nRet != WAVETOP_BACKUP_OK) {
            /*SLogErrorWrite(APLOG_MARK, APLOG_ERR, NULL, 
            "BkMSSqlGetFileName(): nFile: %d, szPath: %s!",nFile,szPath);*/
            goto End;
        }
        
        nStartSeq = pIndexFile->GetStartSeq();
        nEndSeq = pIndexFile->GetEndSeq();
        if (nStartSeq < 0 || nStartSeq > nEndSeq) {
            nRet = WAVETOP_BACKUP_INTERNAL_ERROR;
            pIndexFile->Close();
            /*SLogErrorWrite(APLOG_MARK, APLOG_ERR, NULL, 
            "BkMSSqlGetFileName(): nStartSeq: %d, nEndSeq: %d!",nStartSeq,nEndSeq);*/
            goto End;
        }
        
        if (nCurrSeq >= nStartSeq && nCurrSeq <= nEndSeq) {
            *pnMin = nStartSeq;
            *nBufLen = nFile;
            pIndexFile->Close();
            break;
        }

        if (nStartSeq > nCurrSeq) {
            break;
        }

         pIndexFile->Close();
    }

End:
    if (pIndexFile != NULL) {
        delete pIndexFile;
        pIndexFile = NULL;
    }

    MiMSSqlReleseMutex(pHandle);

    return nRet;
}

MILOG_EXPORT_(int) BKMSSqlIoOptimizeBgein(MiLogHandle *ppHandle, char *pIoDataSrcDir, 
                                          char *pIoDataTargDir, char *pIoDataName, int nDBtype)
{
    /*CIoOptimizeObject *pIoOptimize = NULL;
    int nRC = WAVETOP_BACKUP_OK;
    
    //pIoOptimize = new CIoOptimizeObject;
    if (NULL == pIoOptimize) {
        / *SLogErrorWrite(APLOG_MARK, APLOG_ERR, NULL, 
            "BKMSSqlIoOptimizeBgein(): new CIoOptimizeObject failed");* /
        return WAVETOP_BACKUP_NO_MEMORY;
    }

    *ppHandle = pIoOptimize;
    
   // nRC = pIoOptimize->Init(pIoDataSrcDir, pIoDataTargDir, pIoDataName, nDBtype);
    if (nRC != WAVETOP_BACKUP_OK) {
        / *SLogErrorWrite(APLOG_MARK, APLOG_ERR, NULL, 
            "BKMSSqlIoOptimizeBgein(): Init() failed"); * /
    }
    
    return nRC; */   
	return 0;
} 

MILOG_EXPORT_(int) BKMSSqlIoOptimize(MiLogHandle pHandle, PRUint64 nMinNum, 
                                     PRInt64 nMaxNum, PRInt64 nWriteBegNum)
{
    /*CIoOptimizeObject *pIoOptimize = NULL;
    int nRC = WAVETOP_BACKUP_OK;
    
    if (NULL == pHandle) {
        return WAVETOP_BACKUP_BAD_REQUEST;
    }

    pIoOptimize = (CIoOptimizeObject *)pHandle;
    
    nRC = pIoOptimize->IoDataOptimize(nMinNum, nMaxNum, nWriteBegNum);
    if (nRC != WAVETOP_BACKUP_OK) {
        / *SLogErrorWrite(APLOG_MARK, APLOG_ERR | APLOG_NOERRNO, NULL, 
            "BKMSSqlIoOptimize(): IoDataOptimize() failed"); * /
    }*/

    return 0;
}

MILOG_EXPORT_(int) BKMSSqlIoOptimizeEnd(MiLogHandle pHandle)
{
    CIoOptimizeObject *pIoOptimize = NULL;
    
    if (NULL == pHandle) {
        return WAVETOP_BACKUP_BAD_REQUEST;
    }

    pIoOptimize = (CIoOptimizeObject *)pHandle;

    if (pIoOptimize != NULL) {
        delete pIoOptimize;
    }
    
    return WAVETOP_BACKUP_OK;
}

/**
 * 循环调用该方法，获取从指定序号开始至末尾的数据,并且删除当前已经读完的前一个文件
 * @[in]
 * pHandle - 打开的日志文件的句柄
 * pszBuffer - 待写入缓冲区
 * nSize - 待写入缓冲区长度
 * nSocket - 当不为0时读取指定套接字的下一条数据，当为0时忽略
 * @[out]
 * pnReadBytes - 返回已写入数据的长度
 * pIndex      - 返回索引节点.
 * 如果同一序号读取时缓冲区不够大，先将现有的缓冲区写满
 * 并返回WAVETOP_MIRROR_MORE_WITH_LINE，然后再次调用此方法接着读取
 * 该序号读取完成返回WAVETOP_BACKUP_OK
 * 已读取至末尾返回WAVETOP_MIRROR_LINE_END
 * 出现异常返回WAVETOP_BACKUP_INTERNAL_ERROR
 */
MILOG_EXPORT_(int) BkMSSqlReadBufferFromLogNext(MiLogHandle hHandle,
                                                char *pszBuffer,
                                                unsigned long nSize,
                                                unsigned long *pnReadBytes,
                                                unsigned long nSocket,
                                                ExIndexSt *pIndex)
{
    LogHandleSt    *pHandle = (LogHandleSt *)hHandle;
    CIndexLogFile  *pIndexFile;
    CDataLogFile   *pDataFile;
    unsigned long   nResult;
    unsigned long   nPos;
    unsigned long   nInSock;
    unsigned long   nBytes;
    unsigned long   nRecordSize;
    IndexSt         Index;
    char            szBuf[32];

    assert(pHandle != NULL);
    assert(pHandle->nOperation == 1);

    if (pHandle->nOperation == 2)
        return WAVETOP_BACKUP_INVALID_SYNTAX;

    if (pHandle->nStatus != WAVETOP_BACKUP_OK)
        return pHandle->nStatus;

    nResult = MiMSSqlWaitForMutex(pHandle);
    if (nResult != WAVETOP_BACKUP_OK)
        return WAVETOP_BACKUP_INVALID_SYNTAX;

    pIndexFile = pHandle->hIndxFile;
    pDataFile  = pHandle->hFile;

    if (pIndexFile->m_nMoreSize > 0) {
        nRecordSize = pIndexFile->m_nMoreSize > nSize ? nSize : pIndexFile->m_nMoreSize;
        if (pDataFile->Read(pszBuffer, nRecordSize, &nBytes) != WAVETOP_BACKUP_OK)
            goto FAILED;

        *pnReadBytes = nRecordSize;
        
        if (pIndexFile->m_nMoreSize > nSize) {
            pIndexFile->m_nMoreSize -= nRecordSize;
            nResult = WAVETOP_MIRROR_MSSQL2_LOG_MORE_WITH_LINE;
        }
        else {
            pIndexFile->m_nMoreSize = 0;
            nResult = WAVETOP_BACKUP_OK;
        }

        goto FAILED;
    }
    
    /** 找到正确的Index **/
    if (nSocket != 0) {
        for (;;) {
            nResult = pIndexFile->Next(Index);
            if (nResult == WAVETOP_BACKUP_END) {
                LOG_TRACE1("Switch to index file %d when Reading index.\n", 
                    pHandle->nCurNum);
                nResult = MiMSSqlSwitchIndex(pHandle, 1);
                if (nResult == WAVETOP_BACKUP_FILE_NOT_EXIST) {
#if defined(WIN32) || defined(WINDOWS)
#ifdef _DEBUG
                    printf("[Thd: %5d] ReadRecord index file %d to end. Wait...\n", 
                        GetCurrentThreadId(), pHandle->nCurNum);
#endif
#endif
                    nResult = WAVETOP_BACKUP_END;
                    goto FAILED;
                }
                else if (nResult != WAVETOP_BACKUP_OK)
                    goto FAILED;

#if defined(WIN32) || defined(WINDOWS)
#ifdef _DEBUG
                printf("[Thd: %5d] Switch to index file %d for Reading.\n", 
                    GetCurrentThreadId(), pHandle->nCurNum);
#endif
#endif

                pIndexFile = pHandle->hIndxFile;
                pDataFile  = pHandle->hFile;
                continue;
            }
            else if (nResult != WAVETOP_BACKUP_OK) {
                LOG_TRACE1("Read index record failed. FileNo: %d\n", 
                    pHandle->nCurNum);
                goto FAILED;
            }

            VINT32(nInSock, Index.szSocket);
            if (nInSock != nSocket)
                continue;
            else
                break;
        }
    }
    else {
READRECORD2:
        nResult = pIndexFile->Next(Index);
        if (nResult == WAVETOP_BACKUP_END) {
            nResult = BkMSSqlSwitchIndex(pHandle, 1);
            if (nResult == WAVETOP_BACKUP_FILE_NOT_EXIST) {
#if defined(WIN32) || defined(WINDOWS)
#ifdef _DEBUG
                printf("[Thd: %5d] ReadRecord index file %d to end. Wait...\n", 
                    GetCurrentThreadId(), pHandle->nCurNum);
#endif
#endif
                nResult = WAVETOP_BACKUP_END;
                goto FAILED;
            }
            else if (nResult != WAVETOP_BACKUP_OK)
                goto FAILED;
#if defined(WIN32) || defined(WINDOWS)
#ifdef _DEBUG
            printf("[Thd: %5d] Switch to index file %d for Reading.\n", 
                GetCurrentThreadId(), pHandle->nCurNum);
#endif
#endif
            pIndexFile = pHandle->hIndxFile;
            pDataFile  = pHandle->hFile;
            goto READRECORD2;
        }
        else if (nResult != WAVETOP_BACKUP_OK) {
            LOG_TRACE1("Read index record failed. FileNo: %d\n", 
                pHandle->nCurNum);
            goto FAILED;
        }

    }

    VINT32(nInSock,             Index.szSocket);
    VINT64(pIndex->nSeqNum,     Index.szSeqNum);
    VINT32(nPos,                Index.szPos);
    VINT32(nRecordSize,         Index.szSize);
    VINT32(pIndex->nRecordTime, Index.szTime);
    strcpy(pIndex->szIP,        Index.szIP);
    pIndex->cType             = Index.cType;
    pIndex->nSocket           = nInSock;
    pIndex->nDataSize         = nRecordSize;

#if defined(WIN32) || defined(WINDOWS)
#ifdef _DEBUG
    printf("[Thd: %5d] ReadRecord seq=%d from index file %d.\n", 
        GetCurrentThreadId(), (unsigned long)pIndex->nSeqNum, pHandle->nCurNum);
#endif
#endif

    /** 读取数据 **/
    nResult = pDataFile->Seek(WAVETOP_LOGFILE_BEGIN, nPos);
    if (nResult != WAVETOP_BACKUP_OK)
        goto FAILED;

    nResult = pDataFile->Read(szBuf, sizeof(long), &nBytes);
    if (nResult != WAVETOP_BACKUP_OK)
        goto FAILED;

    VINT32(nBytes, szBuf);

    assert(nBytes == nRecordSize);
    if (nBytes != nRecordSize) {
        nResult = WAVETOP_BACKUP_INVALID_SYNTAX;
        goto FAILED;
    }

    if (nRecordSize > nSize) {
        if (pDataFile->Read(pszBuffer, nSize, &nBytes) != WAVETOP_BACKUP_OK)
            goto FAILED;

        *pnReadBytes = nSize;
        nResult = WAVETOP_MIRROR_MSSQL2_LOG_MORE_WITH_LINE;

        pIndexFile->m_nMoreSize = nRecordSize - nSize;
    }
    else {
        if (pDataFile->Read(pszBuffer, nRecordSize, &nBytes) != WAVETOP_BACKUP_OK)
            goto FAILED;
        *pnReadBytes = nRecordSize;
        nResult = WAVETOP_BACKUP_OK;
    }

FAILED:
    MiMSSqlReleseMutex(pHandle);

    return nResult;
}

/**
 * 该方法可以获取日志文件的文件名ID编号
 * @[in]
 * pHandle - 打开的日志文件的句柄
 * @[out]
 * pnFileNum - 取得该日志文件的ID编号
 * 成功返回WAVETOP_BACKUP_OK
 * 失败返回WAVETOP_BACKUP_INTERNAL_FAILED
 */
MILOG_EXPORT_(int) BkMSSqlGetFileMinNum(MiLogHandle hHandle, PRInt32 *pnFileNum)
{
    LogHandleSt    *pHandle = (LogHandleSt *)hHandle;
    char            szPath[1024];
    char            szPathTemp[1024] = { 0 };
    struct stat     statBuf;
    int             nRet;
    int             nFile;
    int             nLastNum = 0;

    nRet = MiMSSqlWaitForMutex(pHandle);
    if (nRet != WAVETOP_BACKUP_OK)
        return WAVETOP_BACKUP_INTERNAL_ERROR;

    if (pHandle->szSuff[0] != '\0') {
        sprintf(szPathTemp, "%s%s_%s.bin", pHandle->szFileDir, 
            WAVETOP_LOGFILE_CONF, pHandle->szSuff);
    }
    else {
        sprintf(szPathTemp, "%s%s_0.bin", pHandle->szFileDir, 
            WAVETOP_LOGFILE_CONF);
    }
    
    /* 如果是正常打开,则要读取文件序号 */
#if defined(WIN32) || defined(WINDOWS)
    nLastNum = GetPrivateProfileInt("mssql2", "FileNum", 0, szPathTemp);
#else
    CConfigfile *pCfg;
    pCfg = new CConfigfile(szPathTemp);
    if(pCfg != NULL) {
        nLastNum = pCfg->ReadInt("mssql2", "FileNum", 0);
        delete pCfg;
    }
#endif

    for (nFile = 0; nFile <= nLastNum; nFile++) {
        if (pHandle->szSuff[0] != '\0') {
            sprintf(szPath, "%s%s_%s.%06d", pHandle->szFileDir, 
                WAVETOP_LOGFILE_INDEX, pHandle->szSuff, nFile);
        }
        else {
            sprintf(szPath, "%s%s_0.%06d", pHandle->szFileDir, 
                WAVETOP_LOGFILE_INDEX, nFile);
        }

        if (stat(szPath, &statBuf) == -1) {
            continue;
        }

        *pnFileNum = nFile;       
        
        break;
    }

End:
    MiMSSqlReleseMutex(pHandle);

    return nRet;
}

/**
 * 该方法可以获取日志文件的文件名ID编号
 * @[in]
 * pHandle - 打开的日志文件的句柄
 * @[out]
 * pnFileNum - 取得该日志文件的ID编号
 * 成功返回WAVETOP_BACKUP_OK
 * 失败返回WAVETOP_BACKUP_INTERNAL_FAILED
 */
MILOG_EXPORT_(int) BkMSSqlGetFileNumMinSeq(MiLogHandle hHandle, PRInt32 nFileNum, PRUint64 *nMinSeq)
{
    LogHandleSt    *pHandle = (LogHandleSt *)hHandle;
    CIndexLogFile  *pIndexFile = NULL;
    char            szPath[1024];
    PRUint64        nStartSeq = 0;
    int             nRet;

    nRet = MiMSSqlWaitForMutex(pHandle);
    if (nRet != WAVETOP_BACKUP_OK)
        return WAVETOP_BACKUP_INTERNAL_ERROR;

    if (pHandle->szSuff[0] != '\0') {
        sprintf(szPath, "%s%s_%s.%06d", pHandle->szFileDir, 
            WAVETOP_LOGFILE_INDEX, pHandle->szSuff, nFileNum);
    }
    else {
        sprintf(szPath, "%s%s_0.%06d", pHandle->szFileDir, 
            WAVETOP_LOGFILE_INDEX, nFileNum);
    }

    pIndexFile = new CIndexLogFile();
    if (pIndexFile == NULL) {
        nRet = WAVETOP_BACKUP_INTERNAL_ERROR;
        goto End;
    }

    nRet = pIndexFile->Open(szPath, WAVETOP_LOGFILE_RDONLY, 0644);
    if (nRet != WAVETOP_BACKUP_OK) {
        goto End;
    }

    nStartSeq = pIndexFile->GetStartSeq();
    if (nStartSeq < 0) {
        nRet = WAVETOP_BACKUP_INTERNAL_ERROR;
        goto End;
    }

    pIndexFile->Close();

End:
    if (pIndexFile != NULL) {
        delete pIndexFile;
        pIndexFile = NULL;
    }

    if (nRet == WAVETOP_BACKUP_OK) {
        *nMinSeq = nStartSeq;
    }

    MiMSSqlReleseMutex(pHandle);

    return nRet;
}

MILOG_EXPORT_(int) MiMSSqlDeleteSuperMaxFileImmediate(MiLogHandle hHandle, int nLastNum)
{
	LogHandleSt    *pHandle = (LogHandleSt *)hHandle;
	char            szPath[1024];
	char            szDatePath[1024];
	struct stat     statBuf;
	int             nRet = WAVETOP_BACKUP_OK;
	int             nFile;

	if (nLastNum <= 0)
		return WAVETOP_BACKUP_OK;

	nRet = MiMSSqlWaitForMutex(pHandle);
	if (nRet != WAVETOP_BACKUP_OK)
		return WAVETOP_BACKUP_INTERNAL_ERROR;

	for (nFile = nLastNum+1; nFile <= nLastNum+5; nFile++) {
		if (pHandle->szSuff[0] != '\0') {
			sprintf(szPath, "%s%s_%s.%06d", pHandle->szFileDir, 
				WAVETOP_LOGFILE_INDEX, pHandle->szSuff, nFile);
			sprintf(szDatePath, "%s%s_%s.%06d", pHandle->szFileDir, 
				WAVETOP_LOGFILE_DATA, pHandle->szSuff, nFile);
		}
		else {
			sprintf(szPath, "%s%s_0.%06d", pHandle->szFileDir, 
				WAVETOP_LOGFILE_INDEX, nFile);
			sprintf(szDatePath, "%s%s_0.%06d", pHandle->szFileDir, 
				WAVETOP_LOGFILE_DATA, nFile);
		}

		if (PR_SUCCESS == PR_Access(szPath, PR_ACCESS_EXISTS)) {
			if (PR_FAILURE == PR_Delete(szPath))
				break;
		}

		if (PR_SUCCESS == PR_Access(szDatePath, PR_ACCESS_EXISTS)) {
			if (PR_FAILURE == PR_Delete(szDatePath))
				break;
		}
	}

	MiMSSqlReleseMutex(pHandle);

	return nRet;
}


MILOG_EXPORT_(int) MiMSSqlGetLastNum(MiLogHandle hHandle, int *nLastNum)
{
	LogHandleSt    *pHandle = (LogHandleSt *)hHandle;
	int             nRet = WAVETOP_BACKUP_OK;
	char            szPathTemp[1024] = { 0 };

	*nLastNum = 0;

	nRet = MiMSSqlWaitForMutex(pHandle);
	if (nRet != WAVETOP_BACKUP_OK)
		return WAVETOP_BACKUP_INTERNAL_ERROR;

	if (pHandle->szSuff[0] != '\0') {
		sprintf(szPathTemp, "%s%s_%s.bin", pHandle->szFileDir, 
			WAVETOP_LOGFILE_CONF, pHandle->szSuff);
	}
	else {
		sprintf(szPathTemp, "%s%s_0.bin", pHandle->szFileDir, 
			WAVETOP_LOGFILE_CONF);
	}

	/* 如果是正常打开,则要读取文件序号 */
#if defined(WIN32) || defined(WINDOWS)
	*nLastNum = GetPrivateProfileInt("mssql2", "FileNum", 0, szPathTemp);
#else
	CConfigfile *pCfg;
	pCfg = new CConfigfile(szPathTemp);
	if(pCfg != NULL) {
		*nLastNum = pCfg->ReadInt("mssql2", "FileNum", 0);
		delete pCfg;
	}
#endif

	MiMSSqlReleseMutex(pHandle);

	return nRet;
}

MILOG_EXPORT_(int) MiMSSqlSetLastNum(MiLogHandle hHandle, int nLastNum)
{
	LogHandleSt    *pHandle = (LogHandleSt *)hHandle;
	int             nRet = WAVETOP_BACKUP_OK;
	char            szPathTemp[1024] = { 0 };
	char            szFileNo[16]     = {0};
	if(nLastNum <= 0){
		return nRet;
	}
	sprintf(szFileNo, "%d", nLastNum);

	nRet = MiMSSqlWaitForMutex(pHandle);
	if (nRet != WAVETOP_BACKUP_OK)
		return WAVETOP_BACKUP_INTERNAL_ERROR;
	if (pHandle->szSuff[0] != '\0') {
		sprintf(szPathTemp, "%s%s_%s.bin", pHandle->szFileDir, 
			WAVETOP_LOGFILE_CONF, pHandle->szSuff);
	}
	else {
		sprintf(szPathTemp, "%s%s_0.bin", pHandle->szFileDir, 
			WAVETOP_LOGFILE_CONF);
	}


	if(PR_Access(szPathTemp,PR_ACCESS_EXISTS) == PR_SUCCESS){

#if defined(WIN32) || defined(WINDOWS)
		WritePrivateProfileString("mssql2", "FileNum", szFileNo, szPathTemp);
#else
		CConfigfile *pCfg;
		pCfg = new CConfigfile(szPathTemp);
		pCfg->WriteString("mssql2", "FileNum", szFileNo);
		delete pCfg;
#endif

	}

	MiMSSqlReleseMutex(pHandle);

	return nRet;
}

MILOG_EXPORT_(int) BkMSSqlGetFileMaxNum(MiLogHandle hHandle, int *pnFileNum)
{
	LogHandleSt    *pHandle = (LogHandleSt *)hHandle;
	char            szPath[1024];
	char            szPathTemp[1024] = { 0 };
	struct stat     statBuf;
	int             nRet;	
	int             nLastNum = 0;
	int				nFile    = 0;
	int             nFileIndx = 0;
	int             nFileData = 0;
	BOOL            IsExist   = true;

	nRet = MiMSSqlWaitForMutex(pHandle);
	if (nRet != WAVETOP_BACKUP_OK)
		return WAVETOP_BACKUP_INTERNAL_ERROR;

	if (pHandle->szSuff[0] != '\0') {
		sprintf(szPathTemp, "%s%s_%s.bin", pHandle->szFileDir, 
			WAVETOP_LOGFILE_CONF, pHandle->szSuff);
	}
	else {
		sprintf(szPathTemp, "%s%s_0.bin", pHandle->szFileDir, 
			WAVETOP_LOGFILE_CONF);
	}

	/* 如果是正常打开,则要读取文件序号 */
#if defined(WIN32) || defined(WINDOWS)
	nLastNum = GetPrivateProfileInt("mssql2", "FileNum", 0, szPathTemp);
#else
	CConfigfile *pCfg;
	pCfg = new CConfigfile(szPathTemp);
	if(pCfg != NULL) {
		nLastNum = pCfg->ReadInt("mssql2", "FileNum", 0);
		delete pCfg;
	}
#endif

	for (nFile = nLastNum; IsExist; nFile++) {
		if (pHandle->szSuff[0] != '\0') {
			sprintf(szPath, "%s%s_%s.%06d", pHandle->szFileDir, 
				WAVETOP_LOGFILE_INDEX, pHandle->szSuff, nFile);
		}
		else {
			sprintf(szPath, "%s%s_0.%06d", pHandle->szFileDir, 
				WAVETOP_LOGFILE_INDEX, nFile);
		}

		if (stat(szPath, &statBuf) == -1) {
			IsExist = false;
		}else{
			continue;
		}

		nFileIndx = nFile;       

		break;
	}

	for (nFile = nLastNum; IsExist; nFile++) {
		if (pHandle->szSuff[0] != '\0') {
			sprintf(szPath, "%s%s_%s.%06d", pHandle->szFileDir, 
				WAVETOP_LOGFILE_DATA, pHandle->szSuff, nFile);
		}
		else {
			sprintf(szPath, "%s%s_0.%06d", pHandle->szFileDir, 
				WAVETOP_LOGFILE_DATA, nFile);
		}

		if (stat(szPath, &statBuf) == -1) {
			IsExist = false;
		}else{
			continue;
		}

		nFileData = nFile;       

		break;
	}

	*pnFileNum =nFileData > nFileIndx ? nFileData : nFileIndx;

End:
	MiMSSqlReleseMutex(pHandle);

	return nRet;
}

MILOG_EXPORT_(int) BkMSSqlSetFileMaxNum(MiLogHandle hHandle)
{
	int nRC = WAVETOP_BACKUP_OK;
	int nLastNum = 0;
	nRC = BkMSSqlGetFileMaxNum(hHandle,&nLastNum);
	if(nRC != WAVETOP_BACKUP_OK){
		return nRC;
	}
	nRC = MiMSSqlSetLastNum(hHandle,nLastNum);
	if(nRC != WAVETOP_BACKUP_OK){
		return nRC;
	}

	return WAVETOP_BACKUP_OK;

}

/*
MILOG_EXPORT_(int) MiMSSqlLogStartEx2(MiLogHandle *ppHandle, 
	char *pszLogPath, 
	int nOperation,
	char *pszSuff)
{

	//MiLogHandle          pHandle;
	LogHandleSt         *pHandle;
	LogHandleSt         *pRollBack;
	int                  nRet;
	char                 szDataPath[1024] = { 0 };
	char                 szIdxPath[1024] = { 0 };
	char                 szPathTemp[1024] = { 0 };
	char                *pszPtr;
	int                  nOptions;
#if defined(WIN32) || defined(WINDOWS)
	DWORD                dwThreadId;
#endif
	long                nPos = -1;

	//nRet = LogStart(&pHandle, pszLogPath, pszSuff, nOperation);
	if (pszSuff == NULL) {
		return WAVETOP_BACKUP_INTERNAL_ERROR;
	}

	assert((unsigned long)WAVETOP_LOGFILE_LIMITED_SIZE < ((unsigned long)1<<31));

	if (pszLogPath == NULL || *pszLogPath == '\0')
		return WAVETOP_BACKUP_FILE_NOT_EXIST;

	if (strlen(pszLogPath) >= 1000)
		return WAVETOP_BACKUP_FILENAME_TOO_LONG;

	pHandle = (LogHandleSt *)malloc(sizeof(LogHandleSt));
	if (pHandle == NULL)
		return WAVETOP_BACKUP_NO_MEMORY;

	memset(pHandle, 0, sizeof(LogHandleSt));

	if (pszSuff == NULL ||
		snprintf(pHandle->szSuff, sizeof(pHandle->szSuff), "%s", pszSuff) == -1) {
			free(pHandle);
			return WAVETOP_BACKUP_INTERNAL_ERROR;
	}

#if !defined(WIN32)
	sprintf(pHandle->szLockFile, "%s%s", pszLogPath, "/wt_sem");
#endif
	nRet = MiMSSqlCreateMutex(pHandle);
	if (nRet != WAVETOP_BACKUP_OK) {
		free(pHandle);/ * 内存泄露 * /
		return nRet;
	}

	nRet = MiMSSqlWaitForMutex(pHandle);
	if (nRet != WAVETOP_BACKUP_OK) {
		free(pHandle);/ * 内存泄露 * /
		return nRet;
	}

	pHandle->nOperation       = nOperation;
	pHandle->nLimitRecord     = WAVETOP_LOGFILE_LIMITED_RECORD;
	pHandle->nLimitSize       = WAVETOP_LOGFILE_LIMITED_SIZE;
	pHandle->nWay             = 1;

	/ * Linear * /
	pHandle->nLoopIndexCount  = 0xFFFFFFFF;

	pHandle->hFile            = NULL;
	pHandle->hIndxFile        = NULL;
	pHandle->nStatus          = WAVETOP_BACKUP_OK;

	strncpy((char*)(pHandle->szFileDir), pszLogPath, sizeof(pHandle->szFileDir));

	for (pszPtr = pHandle->szFileDir; *pszPtr != '\0'; pszPtr++) {
		if (*pszPtr == '\\')
			*pszPtr = '/';
	}

	nRet = BkMSSqlSetFileMaxNum((MiLogHandle*)pHandle,&(pHandle->nCurNum));
	if (nRet != WAVETOP_BACKUP_OK) {
		free(pHandle);/ * 内存泄露 * /
		return nRet;
	}

	pHandle->nFirstNo = pHandle->nCurNum;

	if (nOperation == 1)
		nOptions = WAVETOP_LOGFILE_CREATE_FILE | WAVETOP_LOGFILE_RDONLY;
	else
		nOptions = WAVETOP_LOGFILE_CREATE_FILE | WAVETOP_LOGFILE_RDWR;

	if (pHandle->szSuff[0] != '\0') {
		sprintf(szIdxPath, "%s%s_%s.%06d", pHandle->szFileDir, 
			WAVETOP_LOGFILE_INDEX, pHandle->szSuff, pHandle->nCurNum);
	}
	else {
		sprintf(szIdxPath, "%s%s_0.%06d", pHandle->szFileDir, 
			WAVETOP_LOGFILE_INDEX, pHandle->nCurNum);
	}

	pHandle->hIndxFile = new CIndexLogFile();
	pHandle->hIndxFile->SetNo(pHandle->nCurNum);
	nRet = pHandle->hIndxFile->Open(szIdxPath, nOptions,0644,&nPos);

	if (nRet == WAVETOP_BACKUP_OPEN_FILE_ERROR)
		goto FAILED;

	if (pHandle->szSuff[0] != '\0') {
		sprintf(szDataPath, "%s%s_%s.%06d", pHandle->szFileDir, 
			WAVETOP_LOGFILE_DATA, pHandle->szSuff, pHandle->nCurNum);
	}
	else {
		sprintf(szDataPath, "%s%s_0.%06d", pHandle->szFileDir, 
			WAVETOP_LOGFILE_DATA, pHandle->nCurNum);
	}

	pHandle->hFile = new CDataLogFile();
	pHandle->hFile->SetNo(pHandle->nCurNum);
	nRet = pHandle->hFile->Open(szDataPath, nOptions,0644,nPos);
	if (nRet != WAVETOP_BACKUP_OK)
		goto FAILED;

	if (pHandle->hFile->GetRecordCount() != pHandle->hIndxFile->GetRecordCount()) {
		LOG_TRACE1("The index file %s has no matched data file. Rollback data file.\n",
			pHandle->hIndxFile->GetName());

		if (nOperation == 1) {
			LogHandleSt *pTmpHandle;
			int nOption;

			pTmpHandle = (LogHandleSt *)malloc(sizeof(LogHandleSt));
			if (pTmpHandle == NULL) {
				nRet = WAVETOP_BACKUP_NO_MEMORY;
				goto FAILED;
			}

			nOption = WAVETOP_LOGFILE_CREATE_FILE |
				WAVETOP_LOGFILE_RDWR | WAVETOP_LOGFILE_WITHOUT_VALIDATE;

			pTmpHandle->hIndxFile = new CIndexLogFile();
			pTmpHandle->hIndxFile->SetNo((pHandle)->nCurNum);
			nRet = pTmpHandle->hIndxFile->Open(szIdxPath, nOption, 0644);
			if (nRet != WAVETOP_BACKUP_OK) {
				delete pTmpHandle->hIndxFile;
				free(pTmpHandle);

				goto FAILED;
			}
			pTmpHandle->hIndxFile->SetRecordCount(pHandle->hIndxFile->GetRecordCount());

			pTmpHandle->hFile = new CDataLogFile();
			pTmpHandle->hFile->SetNo(pHandle->nCurNum);
			nRet = pTmpHandle->hFile->Open(szDataPath, nOption, 0644);
			if (nRet != WAVETOP_BACKUP_OK) {
				delete pTmpHandle->hIndxFile;
				delete pTmpHandle->hFile;
				free(pTmpHandle);

				goto FAILED;
			}

			pRollBack = pTmpHandle;
		}
		else {
			pRollBack = pHandle;
		}

		nRet = MiMSSqlRollBack(pRollBack);
		pHandle->hFile->SetRecordCount(pRollBack->hIndxFile->GetRecordCount());
		pHandle->hFile->SetTailPos(pRollBack->hFile->GetTailPos());
		if (nOperation == 1) {
			delete pRollBack->hIndxFile;
			delete pRollBack->hFile;
			free(pRollBack);
		}
		if (nRet != WAVETOP_BACKUP_OK) {
			LOG_TRACE("Rollback data file failed.\n");

			goto FAILED;
		}
	}

#if defined(WIN32) || defined(WINDOWS)
	pHandle->hThread = CreateThread(NULL, 0, MiMSSqlMMShareThread, (void *)pHandle, 
		0, &dwThreadId);
	if (pHandle->hThread == NULL) {
		nRet = WAVETOP_BACKUP_INTERNAL_ERROR;
		LOG_TRACE1("LogStart() failed in CreateThread(), error: %d", GetLastError());
		goto FAILED;
	}
#endif

	nRet = WAVETOP_BACKUP_OK;
	*ppHandle = pHandle;

FAILED: 
	MiMSSqlReleseMutex(pHandle);

	if (nRet != WAVETOP_BACKUP_OK) {
		if (pHandle != NULL) {
			if (pHandle->hFile != NULL)
				delete pHandle->hFile;
			if (pHandle->hIndxFile != NULL)
				delete pHandle->hIndxFile;
			if (pHandle->hMutex != NULL)
				MiMSSqlDestroyMutex(pHandle);
			free(pHandle);
		}
	}

	if (nRet == WAVETOP_BACKUP_OK) {
		*ppHandle = pHandle;       
	}

	return nRet;
}

*/

/**
 * 该方法可以获取logdata文件夹中 指定数据库data文件最大编号和index文件最大编号中更大的编号
 * @[in]
 * pszLogPath - logdata文件夹路径
 * pszSuff - 实例名_数据库名  如 MSSQLSERVER_db1010
 * nLastNum - 指定的文件编号的指针
 * @[out]
 * pnFileNum-- 指定数据库data文件最大编号和index文件最大编号中更大的编号
 * 成功返回WAVETOP_BACKUP_OK
 * 失败返回WAVETOP_BACKUP_INTERNAL_FAILED
 */
MILOG_EXPORT_(int) BkMSSqlGetFileMaxNum2(char *pszLogPath,char *pszSuff, int *pnFileNum)
{
	char            szPath[1024];
	char            szPathTemp[1024] = { 0 };
	struct stat     statBuf;
	int             nRet = 0;	
	int             nLastNum = 0;
	int				nFile    = 0;
	int             nFileIndx = 0;
	int             nFileData = 0;
	BOOL            IsExist   = true;
	if (pszSuff[0] != '\0') {
		sprintf(szPathTemp, "%s%s_%s.bin", pszLogPath, 
			WAVETOP_LOGFILE_CONF, pszSuff);
	}
	else {
		sprintf(szPathTemp, "%s%s_0.bin", pszLogPath, 
			WAVETOP_LOGFILE_CONF);
	}

	/* 如果是正常打开,则要读取文件序号 */
#if defined(WIN32) || defined(WINDOWS)
	nLastNum = GetPrivateProfileInt("mssql2", "FileNum", 0, szPathTemp);
#else
	CConfigfile *pCfg;
	pCfg = new CConfigfile(szPathTemp);
	if(pCfg != NULL) {
		nLastNum = pCfg->ReadInt("mssql2", "FileNum", 0);
		delete pCfg;
	}
#endif

	for (nFile = nLastNum; IsExist; nFile++) {
		if (pszSuff[0] != '\0') {
			sprintf(szPath, "%s%s_%s.%06d", pszLogPath, 
				WAVETOP_LOGFILE_INDEX, pszSuff, nFile);
		}
		else {
			sprintf(szPath, "%s%s_0.%06d", pszLogPath, 
				WAVETOP_LOGFILE_INDEX, nFile);
		}

		if (stat(szPath, &statBuf) == -1) {
			IsExist = false;
		}else{
			continue;
		}

		nFileIndx = nFile;       

		break;
	}

	for (nFile = nLastNum; IsExist; nFile++) {
		if (pszSuff[0] != '\0') {
			sprintf(szPath, "%s%s_%s.%06d", pszLogPath, 
				WAVETOP_LOGFILE_DATA, pszSuff, nFile);
		}
		else {
			sprintf(szPath, "%s%s_0.%06d", pszLogPath, 
				WAVETOP_LOGFILE_DATA, nFile);
		}

		if (stat(szPath, &statBuf) == -1) {
			IsExist = false;
		}else{
			continue;
		}

		nFileData = nFile;       

		break;
	}

	*pnFileNum =nFileData > nFileIndx ? nFileData : nFileIndx;

End:
	return nRet;
}


/**
 * 该方法可以设置logdata文件夹中 bin文件记录的文件编号数字
 * @[in]
 * pszLogPath - logdata文件夹路径
 * pszSuff - 实例名_数据库名  如 MSSQLSERVER_db1010
 * nLastNum - 指定的文件编号的指针
 * @[out]
 * 更改 logdata文件夹 中指定数据库的 bin 文件中记录的文件编号
 * 成功返回WAVETOP_BACKUP_OK
 * 失败返回WAVETOP_BACKUP_INTERNAL_FAILED
 */
MILOG_EXPORT_(int) MiMSSqlSetLastNum2(char *pszLogPath,char *pszSuff, int nLastNum)
{
	int             nRet = WAVETOP_BACKUP_OK;
	char            szPathTemp[1024] = { 0 };
	char            szFileNo[16]     = {0};
	struct stat     statBuf;

	if(nLastNum <= 0){
		return nRet;
	}
	sprintf(szFileNo, "%d", nLastNum);

	if (pszSuff[0] != '\0') {
		sprintf(szPathTemp, "%s%s_%s.bin", pszLogPath, 
			WAVETOP_LOGFILE_CONF, pszSuff);
	}
	else {
		sprintf(szPathTemp, "%s%s_0.bin", pszLogPath, 
			WAVETOP_LOGFILE_CONF);
	}


#if defined(WIN32) || defined(WINDOWS)
	WritePrivateProfileString("mssql2", "FileNum", szFileNo, szPathTemp);
#else
	CConfigfile *pCfg;
	pCfg = new CConfigfile(szPathTemp);
	pCfg->WriteString("mssql2", "FileNum", szFileNo);
	delete pCfg;
#endif
	return nRet;
}

/**
 * 该方法可以删除 logdata文件夹中指定数据库 的所有data indx 文件（不包括iodaemon正在占用的文件）
 * @[in]
 * pszLogPath - logdata文件夹路径
 * pszSuff - 实例名_数据库名  如 MSSQLSERVER_db1010
 * nLastNum - 指定的文件编号的指针
 * 成功返回WAVETOP_BACKUP_OK
 * 失败返回WAVETOP_BACKUP_INTERNAL_FAILED
 */
MILOG_EXPORT_(int) MiMSSqlDeleteFileImmediate2(char *pszLogPath,char *pszSuff, int nLastNum)
{
	char            szPath[1024];
	char            szDatePath[1024];
	char			szBinPath[1024];
	int             nRet = WAVETOP_BACKUP_OK;
	int             nFile;

	if (nLastNum <= 0){
		/*SLogErrorWrite(APLOG_MARK, APLOG_ERR, NULL, 
			"MiMSSqlDeleteFileImmediate2():  param errro! nLastNum: %d",nLastNum);*/
		return WAVETOP_BACKUP_INTERNAL_ERROR;
	}
	for (nFile = 0; nFile <= nLastNum+10; nFile++) {
		if (pszSuff[0] != '\0') {
			sprintf(szPath, "%s%s_%s.%06d", pszLogPath, 
				WAVETOP_LOGFILE_INDEX, pszSuff, nFile);
			sprintf(szDatePath, "%s%s_%s.%06d", pszLogPath, 
				WAVETOP_LOGFILE_DATA, pszSuff, nFile);
		}
		else {
			sprintf(szPath, "%s%s_0.%06d", pszLogPath, 
				WAVETOP_LOGFILE_INDEX, nFile);
			sprintf(szDatePath, "%s%s_0.%06d", pszLogPath, 
				WAVETOP_LOGFILE_DATA, nFile);
		}

		if (PR_SUCCESS == PR_Access(szPath, PR_ACCESS_EXISTS)) {
			if (PR_FAILURE == PR_Delete(szPath))
				continue;
		}

		if (PR_SUCCESS == PR_Access(szDatePath, PR_ACCESS_EXISTS)) {
			if (PR_FAILURE == PR_Delete(szDatePath))
				continue;
		}
	}

	return nRet;
}

