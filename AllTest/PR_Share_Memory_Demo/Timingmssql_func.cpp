#include "Timingmssql_func.h"
#include "netfd.h"

/* 共享内存头上获取线程个数 */
int test(char *pszSharName)
    {
    PRSharedMemory *pShare        = NULL;
    ShareMasterSt *pShareMemHead  = NULL;
    char *pszAddr                 = NULL;
    int rv = WAVETOP_BACKUP_OK;

    if (pszSharName == NULL) {
        return WAVETOP_BACKUP_INTERNAL_ERROR;
        }
    //printf("pszSharName=%s",pszSharName);
    /* 读取共享内存头信息获取进程和线程数量 */
    pShare = PR_OpenSharedMemory(WAVETOP_SHARE_MEMORY,10, 
        PR_SHM_EXCL|PR_SHM_CREATE, 0666);
    if (pShare == NULL) {
        return WAVETOP_BACKUP_FILENAME_TOO_LONG;
        }
    pszAddr = (char *)PR_AttachSharedMemory(pShare, 0);
    if (pszAddr == NULL) {
        rv = WAVETOP_BACKUP_INTERNAL_ERROR;
        goto FAILED;
        }
   // cout<<"pszAddr: "<<pszAddr<<endl;
/*
    pShareMemHead = (ShareMasterSt *)pszAddr;
    if (   pShareMemHead->nThreadNum <= 0 
        || pShareMemHead->nProcNum <= 0) {
            rv = WAVETOP_BACKUP_INTERNAL_ERROR;
        }
    *pnThrdCount = pShareMemHead->nThreadNum;*/
FAILED:
    /* 释放pszaddr */
    if (pszAddr != NULL) {
        PR_DetachSharedMemory(pShare, pszAddr);
        }
    if (pShare != NULL) {
        PR_CloseSharedMemory(pShare);
        }

    return rv;
    }



/* 共享内存头上获取线程个数 */
int BkGetThreadNum(int *pnThrdCount, char *pszSharName)
    {
    PRSharedMemory *pShare        = NULL;
    ShareMasterSt *pShareMemHead  = NULL;
    char *pszAddr                 = NULL;
    int rv = WAVETOP_BACKUP_OK;

    if (pszSharName == NULL) {
        return WAVETOP_BACKUP_INTERNAL_ERROR;
        }
    //printf("pszSharName=%s",pszSharName);
    /* 读取共享内存头信息获取进程和线程数量 */
    pShare = PR_OpenSharedMemory(pszSharName, sizeof(ShareMasterSt), 
        PR_SHM_CREATE, 0664);
    if (pShare == NULL) {
        return WAVETOP_BACKUP_FILENAME_TOO_LONG;
        }
    pszAddr = (char *)PR_AttachSharedMemory(pShare, 0);
    if (pszAddr == NULL) {
        rv = WAVETOP_BACKUP_INTERNAL_ERROR;
        goto FAILED;
        }
    pShareMemHead = (ShareMasterSt *)pszAddr;
    if (   pShareMemHead->nThreadNum <= 0 
        || pShareMemHead->nProcNum <= 0) {
            rv = WAVETOP_BACKUP_INTERNAL_ERROR;
        }
    *pnThrdCount = pShareMemHead->nThreadNum;
FAILED:
    /* 释放pszaddr */
    if (pszAddr != NULL) {
        PR_DetachSharedMemory(pShare, pszAddr);
        }
    if (pShare != NULL) {
        PR_CloseSharedMemory(pShare);
        }

    return rv;
    }

int BKSetDBStart(char *pszModPath, char *pszUser, int nStatus, char *pszSharName)
{
    ShareThreadTaskSt *pstShareThrdNode = NULL;
    ShareMemorySt *pstShareNode = NULL;
    PRSharedMemory *pShare = NULL;
    char *pszSharHead = NULL;
    char *pszShareAddr = NULL;
    int nSharSize = 0;
    int nThrdCount = 0;
    int nPorcNum = 0;
    int nThreNum = 0;
    int rv = WAVETOP_BACKUP_OK;
    bool bFind = false;

    /* 共享内存头结构上获取线程个数 */
    BkGetThreadNum(&nThrdCount, pszSharName);
    cout<<"nThrdCount : "<<nThrdCount<<endl;
    if (nThrdCount > WAVETOP_BACKUP6_MAX_THREAD || nThrdCount < 0 || pszSharName == NULL)
    {
        rv = WAVETOP_BACKUP_INTERNAL_ERROR;
        goto FAILED;
    }

    /* 根据头结构信息获取整个共享内存 */
    nSharSize = sizeof(ShareMasterSt) +
                ((sizeof(ShareMemorySt) + (sizeof(ShareThreadTaskSt) * nThrdCount)) *
                 WAVETOP_BACKUP6_MAX_PROCESS);

    pShare = PR_OpenSharedMemory(pszSharName, nSharSize,
                                 PR_SHM_READONLY, 0664);
    if (pShare == NULL)
    {
        rv = WAVETOP_BACKUP_FILENAME_TOO_LONG;
        goto FAILED;
    }

    pszShareAddr = (char *)PR_AttachSharedMemory(pShare, 0);
    if (pszShareAddr == NULL)
    {
        rv = WAVETOP_BACKUP_INTERNAL_ERROR;
        goto FAILED;
    }
    pszSharHead = pszShareAddr;

    /* XXX: 整型赋值, 没加互斥锁. 子进程共享内存插槽信息 */
    pszShareAddr += sizeof(ShareMasterSt);
    for (nPorcNum = 0; nPorcNum < WAVETOP_BACKUP6_MAX_PROCESS; nPorcNum++)
    {
        pstShareNode = (ShareMemorySt *)pszShareAddr;
        if (pstShareNode->nStatus == WAVETOP_BACKUP6_CHILD_PROC_SLOT_BUSY)
        {
            for (nThreNum = 0; nThreNum < nThrdCount; nThreNum++)
            {
                pstShareThrdNode = (ShareThreadTaskSt *)(pszShareAddr + sizeof(ShareMemorySt) + (sizeof(ShareThreadTaskSt) * nThreNum));
                if (pstShareThrdNode->nTaskStatus == WAVETOP_BACKUP6_THREAD_TASK_BUSY)
                {
                    if (strcmp(pstShareThrdNode->szModulePath, pszModPath) == 0 && strcmp(pstShareThrdNode->szUser, pszUser) == 0)
                    {
                        pstShareThrdNode->nStartDBFlag = nStatus;
                        bFind = true;
                        break;
                    }
                }
            }
            if (bFind)
            {
                break;
            }
        }

        pszShareAddr += (sizeof(ShareMemorySt) + (sizeof(ShareThreadTaskSt) * nThrdCount));
    }

    PR_DetachSharedMemory(pShare, pszSharHead);
    PR_CloseSharedMemory(pShare);

    if (!bFind)
        rv = WAVETOP_BACKUP_INTERNAL_ERROR;

FAILED:

    return rv;
}

/* create a connection to a address */
int bkCreateNetSBuff(const char *pszAddr, int nPort, int nTimeout, sbuff **pServStream) 
    {
    PRSocketOptionData SockOption;
    PRFileDesc *pFileDesc;
    PRNetAddr NetAddr;
    PRHostEnt HostEntry;
    PRStatus nPrStatus;
    PRIntn nRetVal;
    sbuff *pNetStream;
    char szHostBuf[256];
    int i;

    for (i = 0; '\0' != pszAddr[i]; i++) {
        if (!ap_isdigit(pszAddr[i]) && '.' != pszAddr[i]) {
            break;
            }
        }

    if ('\0' != pszAddr[i]) {
        /* Setup network connection. */
        nPrStatus = PR_GetHostByName(pszAddr, szHostBuf, 
            sizeof(szHostBuf), &HostEntry);
        if (PR_SUCCESS != nPrStatus) {
            fprintf(stderr, "PR_GetHostByName failed\n");
            return WAVETOP_BACKUP_CONNECT_DOWN;
            }

        nRetVal = PR_EnumerateHostEnt(0, &HostEntry, nPort, &NetAddr);
        if (0 > nRetVal) {
            fprintf(stderr, "PR_EnumerateHostEnt failed\n");
            return WAVETOP_BACKUP_CONNECT_DOWN;
            }
        }
    else { /* IP address */
        PR_InitializeNetAddr(PR_IpAddrAny, nPort, &NetAddr);
        PR_StringToNetAddr(pszAddr, &NetAddr);
        }

    pFileDesc = PR_NewTCPSocket();
    if (NULL == pFileDesc) {
        fprintf(stderr, "PR_NewTCPSocket failed\n");
        return WAVETOP_BACKUP_CONNECT_DOWN;
        }

    /* Make the socket blocking. */
    SockOption.option = PR_SockOpt_Nonblocking;
    SockOption.value.non_blocking = PR_FALSE;

    nPrStatus = PR_SetSocketOption(pFileDesc, &SockOption);
    if (PR_SUCCESS != nPrStatus) {
        fprintf(stderr, "PR_SetSocketOption failed\n");

        PR_Close(pFileDesc);
        return WAVETOP_BACKUP_CONNECT_DOWN;
        }

    /* Verify that a connection can be made to the socket. */
    nPrStatus = PR_Connect(pFileDesc, &NetAddr, PR_INTERVAL_NO_TIMEOUT);
    if (PR_SUCCESS != nPrStatus) {
        fprintf(stderr, "PR_Connect (%s:%d) failed\n", pszAddr, nPort);

        PR_Close(pFileDesc);
        return WAVETOP_BACKUP_CONNECT_DOWN;
        }

    /* create sbuff */
    pNetStream = NewSBuff(pFileDesc, nTimeout, nTimeout);

    *pServStream = pNetStream;
    return WAVETOP_BACKUP_OK;
    }

int BkFreeSBuff(sbuff *pStream)
    {
    if (pStream->fd != NULL) {
        PR_Close(pStream->fd);
        }
    CloseSBuff(pStream);
    return WAVETOP_BACKUP_OK;
    }


int UinSendRecoverProtocol(const char * pszObject, const char *pszInstance, 
    const char* pszServer, char *szDbName, int nPort, 
    char *pszModid, char *pszMsg, PRInt32 nSize)
    {
    sbuff *pSBuff       = NULL;
    char szBuffer[1024] = { 0 };   
    char szMsg[1024]    = { 0 }; 
    int nResult         = 0;        
    int nStartRespone   = 0;
    int nProtocol       = 0;
    int nNetFD;

    if (szDbName == NULL || pszServer == NULL) {
        PR_snprintf(szMsg, sizeof(szMsg),
            "UinSendRecoverProtocol().failed, szDbName或pszServer为空");
        goto END;
        }

    nResult = bkCreateNetSBuff(pszServer, nPort, 600, &pSBuff);
    if (nResult != WAVETOP_BACKUP_OK) {
        PR_snprintf(szMsg, sizeof(szMsg), "连接IP:%s,端口:%d失败.", pszServer, nPort);
        goto END;
        }

    sprintf(szBuffer, "Proto-Version:%d\r\n", WAVETOP_BACKUP_VERSION_4);
    WriteString(pSBuff, szBuffer); 

    /* Protocl method (head) */
    sprintf(szBuffer, "Proto-Method:%d\r\n", BACKUP_PROTO_RECOVER);
    WriteString(pSBuff, szBuffer);

    /* OS Type (head) */
    //sprintf(szBuffer, "Proto-OS:%d\r\n", GetCurrentOSType());
    sprintf(szBuffer, "Proto-OS:%d\r\n", 2);
    WriteString(pSBuff, szBuffer);

    /* Mod ID */
    sprintf(szBuffer, "Mod-Id:%s\r\n", pszModid);
    WriteString(pSBuff, szBuffer);

    /* DBname */
    sprintf(szBuffer, "Db-Name:%s\r\n", szDbName);
    WriteString(pSBuff, szBuffer);

    sprintf(szBuffer, "Object-Name:%s\r\n", pszObject);
    WriteString(pSBuff, szBuffer); 

    if (pszInstance != NULL && strcasecmp(pszInstance, "") != 0) {
        sprintf(szBuffer, "Instance-Name:%s\r\n", pszInstance);
        WriteString(pSBuff, szBuffer);
        }   

    WriteString(pSBuff, "\r\n");

    nResult = FlushSBuff(pSBuff);
    if (nResult != WAVETOP_BACKUP_OK) {
        PR_snprintf(szMsg, sizeof(szMsg), "发送协议失败,IP:%s,端口:%d", pszServer, nPort);
        goto END;
        }

    /* Protocl version code */
    nResult = ReadInt32(pSBuff, &nProtocol);
    if (nResult != WAVETOP_BACKUP_OK) {
        PR_snprintf(szMsg, sizeof(szMsg), "接收协议失败,IP:%s,端口:%d", pszServer, nPort);
        goto END;
        }

    /* Status code */
    nResult = ReadInt32(pSBuff, &nStartRespone);
    if (nResult != WAVETOP_BACKUP_OK) {
        PR_snprintf(szMsg, sizeof(szMsg), "接收协议失败,IP:%s,端口:%d", pszServer, nPort);
        goto END;
        }

    if (nStartRespone != WAVETOP_BACKUP_OK) {
        PR_snprintf(szMsg, sizeof(szMsg), "恢复端响应异常,IP:%s,端口:%d", pszServer, nPort);
        nResult = WAVETOP_BACKUP_INTERNAL_ERROR;
        goto END;
        }

END:
    if (pSBuff != NULL) {
        BkFreeSBuff(pSBuff);
        pSBuff = NULL;
        }

    if (nResult == WAVETOP_BACKUP_OK) {
        PR_snprintf(szMsg, sizeof(szMsg), "启动实时恢复成功.");
        }

    MiSafeStrncpy(pszMsg, szMsg, nSize);

    return nResult;
    }
