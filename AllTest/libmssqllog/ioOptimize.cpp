/** =============================================================================
 ** Copyright (c) 2011 WaveTop Information Corp. All rights reserved.
 **
 ** The A-CDP system
 **
 ** =============================================================================
 */

#if defined(WIN32) || defined(WINDOWS)
#include <windows.h>
#else
#include <errno.h>
#include <sys/mman.h>
#endif
#if defined(LINUX)
#include <unistd.h>
#endif
#include "ioOptimize.h"

CIoOptimizeObject::CIoOptimizeObject()
{
    m_nBaseBucketNum     = 1024;
    m_DBtype             = 0;
    m_pIoPool            = NULL;
    m_hSrcLog            = NULL;
    m_hTargLog           = NULL;
    m_pIoFileNameList    = NULL;
    m_pIoChuckListSt     = NULL;

} 

CIoOptimizeObject::~CIoOptimizeObject()
{
    UnInit();     
}

int CIoOptimizeObject::Init(char *pIoDataSrcDir, char *pIoDataTargDir, char *pIoDataName, int nDBtype)
{
    int nRC = WAVETOP_BACKUP_OK;

    m_pIoPool = ap_make_sub_pool(NULL);
    if (m_pIoPool == NULL) {
        SLogErrorWrite(APLOG_MARK, APLOG_ERR | APLOG_NOERRNO, NULL,
            "CIoOptimizeObject::Init(): ap_make_sub_pool() failed");
        nRC = WAVETOP_BACKUP_NO_MEMORY;
        goto FAILED;
    }
    m_DBtype = nDBtype;
    
    if (pIoDataSrcDir  == NULL || 
        pIoDataTargDir == NULL ||
        pIoDataName    == NULL) {
        SLogErrorWrite(APLOG_MARK, APLOG_ERR | APLOG_NOERRNO, NULL,
            "Init(): LogFileDir is NULL.");
        nRC = WAVETOP_BACKUP_INVALID_SYNTAX;
        goto FAILED;
    }
    
    /*  初始化源日志文件句柄 */
    nRC = MiMSSqlLogStartEx(&m_hSrcLog, pIoDataSrcDir, 1, pIoDataName);
    if (nRC != WAVETOP_BACKUP_OK) {
        SLogErrorWrite(APLOG_MARK, APLOG_ERR | APLOG_NOERRNO, NULL,
            "Init(): MiMSSqlLogStartEx() failed.");
        goto FAILED;
    }
    
    /* 初始化目标日志文件句柄 */
    nRC = MiMSSqlLogStartEx(&m_hTargLog, pIoDataTargDir, 2, pIoDataName);
    if (nRC != WAVETOP_BACKUP_OK) {
        SLogErrorWrite(APLOG_MARK, APLOG_ERR | APLOG_NOERRNO, NULL,
            "Init(): MiMSSqlLogStartEx() failed.");
        goto FAILED;
    }


FAILED:

    if (m_pIoPool != NULL && nRC != WAVETOP_BACKUP_OK)
        ap_destroy_pool(m_pIoPool);
    
    return nRC;

}

int CIoOptimizeObject::Hash(const void *pKey)
{
    PRInt64 nOffset = 0;
    int nHash       = 0;

    nOffset = *((PRInt64*)pKey);
    
    nHash = nOffset % (m_nBaseBucketNum - 1);
    
    return nHash;
}

/* 关键字冲突比较算法 */
int CIoOptimizeObject::Compare(const void *pKey1, const void *pKey2)
{
    IoDataNodeSt *pIoDataNode1 = (IoDataNodeSt *)pKey1;
    IoDataNodeSt *pIoDataNode2 = (IoDataNodeSt *)pKey2;

    if (pIoDataNode1->nOffset == pIoDataNode2->nOffset &&
        pIoDataNode1->nSize >= pIoDataNode2->nSize)
        return 1;
    else
        return 0;
}


int CIoOptimizeObject::IoNewAndSelectRow(pool *pPool, char*pszFileName, IoDataInfoSt **ppIoDataInfoList)
{
    int nRC   = WAVETOP_BACKUP_OK;
    int nFind = 0;
    IoDataInfoSt *pIoDataInfoList = NULL;
    IoDataInfoSt *pIoDataInfoTail = NULL;
    IoDataInfoSt *pIoDataTemp     = NULL;

    pIoDataTemp = m_pIoFileNameList;
    for (; pIoDataTemp != NULL; pIoDataTemp = pIoDataTemp->pNext) {
        pIoDataInfoTail = pIoDataTemp;
        if (strcmp(pIoDataTemp->pszFileName, pszFileName) == 0) {
            nFind = 1;
            *ppIoDataInfoList = pIoDataInfoTail;
            nRC = WAVETOP_BACKUP_OK;
            goto FAILED;
        }
    }
    
    pIoDataInfoList = (IoDataInfoSt *)ap_palloc(pPool, sizeof(IoDataInfoSt));
    if (pIoDataInfoList == NULL) {
        SLogErrorWrite(APLOG_MARK, APLOG_ERR | APLOG_NOERRNO, NULL, 
            "IoNewAndSelectRow(): ap_palloc() failed.");
        nRC = WAVETOP_BACKUP_NO_MEMORY;
        goto FAILED;    
    }
    memset(pIoDataInfoList, 0, sizeof(IoDataInfoSt));
    
    pIoDataInfoList->ppQueue = (IoDataNodeSt **)ap_palloc(pPool, (sizeof(IoDataNodeSt*)*m_nBaseBucketNum));
    if (pIoDataInfoList->ppQueue == NULL) {
        SLogErrorWrite(APLOG_MARK, APLOG_ERR | APLOG_NOERRNO, NULL, 
            "IoNewAndSelectRow(): ap_palloc() failed.");
        nRC = WAVETOP_BACKUP_NO_MEMORY;
        goto FAILED;     
    }
    memset(pIoDataInfoList->ppQueue, 0, (sizeof(IoDataInfoSt*)*m_nBaseBucketNum));
    
    pIoDataInfoList->pszFileName = ap_pstrdup(pPool, pszFileName);

    if (m_pIoFileNameList == NULL && nFind == 0) {

        m_pIoFileNameList = pIoDataInfoList;
        pIoDataInfoList->pNext = NULL;
        *ppIoDataInfoList = m_pIoFileNameList;
     
    }
    else if (pIoDataInfoTail != NULL && nFind == 0) {

        pIoDataInfoTail->pNext = pIoDataInfoList;
        pIoDataInfoList->pNext = NULL;
        *ppIoDataInfoList = pIoDataInfoList;

    }
    

FAILED:    
    return nRC;        
}


int CIoOptimizeObject::IoAddAndCheckNode(IoDataNodeSt *pIoDataNode, 
                                         IoDataInfoSt *pIoDataInfoList, 
                                         IoChuckListSt **ppIoChuckList)
{
    int nIdx  = 0;
    int nFind = 0;
    int nRC   = WAVETOP_BACKUP_OK;
    IoDataNodeSt *pIoDataNodeList  = NULL;
    IoDataNodeSt *pIoDataNodeTail  = NULL;
    IoChuckListSt *pIoChuckList    = NULL;
    
    nIdx = Hash(&(pIoDataNode->nOffset));

    if (pIoDataInfoList->ppQueue[nIdx] == NULL) {
        pIoDataInfoList->ppQueue[nIdx]  = pIoDataNode;
        pIoDataNode->pNext = NULL;
    }
    else {
        pIoDataNodeList = pIoDataInfoList->ppQueue[nIdx];
        for (; pIoDataNodeList != NULL; pIoDataNodeList = pIoDataNodeList->pNext) {
            
            pIoDataNodeTail = pIoDataNodeList;
            /* 重复的io块节点添加到丢弃链表 */
            if (Compare(pIoDataNodeList, pIoDataNode) == 1) {
                nFind = 1;
                pIoChuckList = (IoChuckListSt *)ap_palloc(m_pIoPool, sizeof(IoChuckListSt));
                if (pIoChuckList == NULL) {
                    SLogErrorWrite(APLOG_MARK, APLOG_ERR | APLOG_NOERRNO, NULL, 
                        "IoAddAndCheckNode(): ap_palloc failed.");
                    nRC = WAVETOP_BACKUP_NO_MEMORY;
                    goto FAILED;
                }
                memset(pIoChuckList, 0, sizeof(IoChuckListSt));

                pIoChuckList->nSeqNum = pIoDataNode->nSeqNum;
                pIoChuckList->pNext = NULL;

                *ppIoChuckList = pIoChuckList;
            }
        }
    }
    
    /* 无重复的io块节点添加到hash数组 */
    if (nFind == 0 && pIoDataNodeTail != NULL) {
        pIoDataNodeTail->pNext = pIoDataNode;
        pIoDataNode->pNext = NULL;
    }

FAILED:

    return nRC;
}

int CIoOptimizeObject::IoNewChuckList(IoChuckListSt *pIoChuckList)
{
    int nRC = WAVETOP_BACKUP_OK;
    
    if (m_pIoChuckListSt == NULL) {
        m_pIoChuckListSt = pIoChuckList;
        pIoChuckList->pNext = NULL;
    }
    else {
        pIoChuckList->pNext = m_pIoChuckListSt;
        m_pIoChuckListSt = pIoChuckList;
    }

    return nRC;
}

int CIoOptimizeObject::IoWriteLogDataFile(PRUint64 nMinNum, PRInt64 nMaxNum, PRInt64 nWriteBegNum)
{
    int nRC = WAVETOP_BACKUP_OK;
    PRUint64 nLastSeqNum           = 0;
    PRInt64 nHighOffset64          = 0;
    PRInt32  nDataSize             = 0;
    PRInt32  nPos                  = 0;
    PRInt64  nCurSeq               = 0;
    unsigned long nReadBytes       = 0;
    char *pTemp                    = NULL;
    char *pszData                  = NULL;
    unsigned char *pszBuff         = NULL;
    unsigned char *pszDataBuff     = NULL;
    unsigned char *pszIOBuff       = NULL;
    unsigned char *pszIO           = NULL;
    bool bFinish                   = true;
    WTBUF stBuf[2]                 = { 0 };
    ExIndexSt IndexReadSt          = { 0 };
    ExIndexSt IndexWriteSt         = { 0 };
    IoDataHeadSt dataNode          = { 0 };
    IoDataHeadSt dataOptimize      = { 0 };

    nCurSeq = nWriteBegNum;

    nRC = MiMSSqlReadBufferFromLogStart(m_hSrcLog, nMinNum, 1);
    if (nRC != WAVETOP_BACKUP_OK) {
        SLogErrorWrite(APLOG_MARK, APLOG_ERR | APLOG_NOERRNO, NULL, 
            "IoWriteLogDataFile(): MiMSSqlReadBufferFromLogStart() failed. code %d", nRC); 
        goto FAILED;
    }

    stBuf[0].pszBuf = (unsigned char *)ap_palloc(m_pIoPool, WAVETOP_CDP_ORA_MAX_FILE_NAME);;
    if (stBuf[0].pszBuf == NULL) {
        SLogErrorWrite(APLOG_MARK, APLOG_ERR | APLOG_NOERRNO, NULL,
            "IoWriteLogDataFile() ap_palloc failed.");
        nRC = WAVETOP_BACKUP_NO_MEMORY;
        goto FAILED;
    }
    memset(stBuf[0].pszBuf, 0, WAVETOP_CDP_ORA_MAX_FILE_NAME);

    pszData = (char *)ap_palloc(m_pIoPool, WAVETP_CDP_ORA_BLOCK_SIZE);;
    if (pszData == NULL) {
        SLogErrorWrite(APLOG_MARK, APLOG_ERR | APLOG_NOERRNO, NULL,
            "IoWriteLogDataFile() ap_palloc failed.");
        nRC = WAVETOP_BACKUP_NO_MEMORY;
        goto FAILED;
    }
    memset(pszData, 0, WAVETP_CDP_ORA_BLOCK_SIZE);

    pszBuff = (unsigned char *)malloc(WAVETP_CDP_ORA_BLOCK_SIZE);
    if (pszBuff == NULL) {
        SLogErrorWrite(APLOG_MARK, APLOG_ERR | APLOG_NOERRNO, NULL,
            "IoWriteLogDataFile() ap_palloc failed.");
        nRC = WAVETOP_BACKUP_NO_MEMORY;
        goto FAILED;
    }
    memset(pszBuff, 0, WAVETP_CDP_ORA_BLOCK_SIZE);

    pszIOBuff = (unsigned char *)malloc(WAVETOP_IO_OPTIMIZE_BLOCK_SIZE);
    if (pszIOBuff == NULL) {
        SLogErrorWrite(APLOG_MARK, APLOG_ERR | APLOG_NOERRNO, NULL,
            "IoWriteLogDataFile() malloc failed.");
        nRC = WAVETOP_BACKUP_NO_MEMORY;
        goto FAILED;
    }
    memset(pszIOBuff, 0, WAVETOP_IO_OPTIMIZE_BLOCK_SIZE);
    
    while (true) {
        memset(&dataNode, 0, sizeof(IoDataHeadSt));
        memset(pszData, 0, WAVETP_CDP_ORA_BLOCK_SIZE);
        do {
            /* 读取数据 */
            nRC = MiMSSqlReadBufferFromLogNext(m_hSrcLog, pszData, WAVETP_CDP_ORA_BLOCK_SIZE, 
                                               &nReadBytes, 0, &IndexReadSt);
            switch(nRC) {
            case WAVETOP_BACKUP_OK:
                bFinish = true;
                break;
            case WAVETOP_MIRROR_MSSQL2_LOG_MORE_WITH_LINE:
                bFinish = false;
                break;
            case WAVETOP_BACKUP_END:
                bFinish = true;
#if defined(WIN32)
                Sleep(500);
#else
                sleep(1);
#endif
                continue;    
            default:
                SLogErrorWrite(APLOG_MARK, APLOG_ERR | APLOG_NOERRNO, NULL, 
                    "IoWriteLogDataFile(): MiMSSqlReadBufferFromLogNext() failed. code %d", nRC);
                goto FAILED;
            }
            
            if (nLastSeqNum != IndexReadSt.nSeqNum) {
                pTemp = pszData;
                dataNode.nSize = 0;
                /* 获取文件名长度 */
                VINT32(dataNode.nFileNameLen, pTemp);
                pTemp += sizeof(dataNode.nFileNameLen);
                dataNode.nSize += sizeof(dataNode.nFileNameLen);
                if (dataNode.nFileNameLen < 0) {
                    SLogErrorWrite(APLOG_MARK, APLOG_ERR | APLOG_NOERRNO, NULL, 
                        "IoWriteLogDataFile(): Bad file name.");
                    nRC = WAVETOP_BACKUP_INVALID_SYNTAX;
                    goto FAILED;
                }
                else if (dataNode.nFileNameLen >= WAVETOP_CDP_ORA_MAX_FILE_NAME) {
                    SLogErrorWrite(APLOG_MARK, APLOG_ERR | APLOG_NOERRNO, NULL, 
                        "IoWriteLogDataFile: no memory.");
                    nRC = WAVETOP_BACKUP_NO_MEMORY;
                    goto FAILED;    
                }
                
                /* 获取文件名 */
                memcpy(dataNode.szFileName, pTemp, dataNode.nFileNameLen);
                dataNode.szFileName[dataNode.nFileNameLen] = '\0';
                pTemp += dataNode.nFileNameLen;
                dataNode.nSize += dataNode.nFileNameLen;
                
                /* 文件编号 */
                VINT16(dataNode.nFileNO, pTemp);
                pTemp += sizeof(dataNode.nFileNO);
                dataNode.nSize += sizeof(dataNode.nFileNO);
                
                /* 文件类型  */
                VINT16(dataNode.nFileType, pTemp);
                pTemp += sizeof(dataNode.nFileType);
                dataNode.nSize += sizeof(dataNode.nFileType);
                
                /* nAction */
                VINT16(dataNode.nAction, pTemp);
                pTemp += sizeof(dataNode.nAction);
                dataNode.nSize += sizeof(dataNode.nAction);
                
                /* highOffset */
                VINT32(dataNode.nHighOffset32, pTemp);
                pTemp += sizeof(dataNode.nHighOffset32);
                dataNode.nSize += sizeof(dataNode.nHighOffset32);
                
                /* lowOffset*/
                VINT32(dataNode.nLowOffset32, pTemp);
                pTemp += sizeof(dataNode.nLowOffset32);
                dataNode.nSize += sizeof(dataNode.nLowOffset32);
                
                nHighOffset64 = dataNode.nHighOffset32;
                dataNode.nOffSet = (nHighOffset64 << 32) + dataNode.nLowOffset32;
                
                /*  获取数据长度 */
                VINT32(dataNode.nDataLen, pTemp)
                pTemp += sizeof(dataNode.nDataLen);
                dataNode.nSize += sizeof(dataNode.nDataLen); 
                
                nDataSize = nReadBytes - (pTemp - pszData);
                if (dataNode.nDataLen > WAVETP_CDP_ORA_BLOCK_SIZE) {
                    pszDataBuff = (unsigned char *)malloc(dataNode.nDataLen + 1);
                    if (pszDataBuff == NULL) {
                        SLogErrorWrite(APLOG_MARK, APLOG_ERR | APLOG_NOERRNO, NULL,
                            "IoWriteLogDataFile() ap_palloc failed.");
                        nRC = WAVETOP_BACKUP_NO_MEMORY;
                        goto FAILED;
                    }
                    memset(pszDataBuff, 0, dataNode.nDataLen + 1);
                    memcpy(pszDataBuff, pTemp, nDataSize);
                    pszIO = pszDataBuff;
                }
                else {
                    memcpy(pszBuff, pTemp, nDataSize);
                    pszIO = pszBuff;      
                }
                
                nPos = nDataSize;
            }
            else {
                memcpy(pszIO+nPos, pszData, nReadBytes);
                nPos += nReadBytes;
            }

            nLastSeqNum = IndexReadSt.nSeqNum; 

        } while (bFinish != true);

        nPos = 0;
        SLogErrorWrite(APLOG_MARK, APLOG_DEBUG, NULL, 
            "原IO:fileName:%s, Seq:%lld, offset:%lld, DataLen:%d", 
            dataNode.szFileName, IndexReadSt.nSeqNum, dataNode.nOffSet, dataNode.nDataLen);

        /* 重复IO优化 */
        if (m_pIoChuckListSt != NULL) {
            if (IndexReadSt.nSeqNum == m_pIoChuckListSt->nSeqNum){
                m_pIoChuckListSt = m_pIoChuckListSt->pNext;  
                SLogErrorWrite(APLOG_MARK, APLOG_DEBUG, NULL, 
                    "重复IO被优化的IO:%lld", IndexReadSt.nSeqNum);
                if (pszDataBuff != NULL) {
                    free(pszDataBuff);
                    pszDataBuff = NULL;
                }
                continue;
            }
        }

        /* 第一个点 */
        if (strlen(dataOptimize.szFileName) <= 0) {
            memcpy(&dataOptimize, &dataNode, sizeof(IoDataHeadSt));
            memcpy(pszIOBuff, pszIO, dataNode.nDataLen);
            if (pszDataBuff != NULL) {
                free(pszDataBuff);
                pszDataBuff = NULL;
            }
            continue;
        }
        /* 线性IO优化 */
        else if (strncmp(dataOptimize.szFileName, dataNode.szFileName, dataOptimize.nFileNameLen) == 0 
            && dataNode.nOffSet == (dataOptimize.nOffSet + (PRInt64)dataOptimize.nDataLen)
            && dataNode.nDataLen + dataOptimize.nDataLen < WAVETOP_IO_OPTIMIZE_BLOCK_SIZE) {
            SLogErrorWrite(APLOG_MARK, APLOG_DEBUG, NULL,
                "线性被优化的IO:Seq:%lld.", IndexReadSt.nSeqNum);
            memcpy(pszIOBuff + dataOptimize.nDataLen, pszIO, dataNode.nDataLen);
            dataOptimize.nDataLen += dataNode.nDataLen;
            if (pszDataBuff != NULL) {
                free(pszDataBuff);
                pszDataBuff = NULL;
            }
            if (IndexReadSt.nSeqNum == nMaxNum) {
                memset(&dataNode, 0, sizeof(IoDataHeadSt));
            }
            else {
                continue;
            }
        }
        
TASKEND:
        /* 合并连续的IO块后重新写IO块头 */
        pTemp = (char *)stBuf[0].pszBuf;
        /* 文件长度 */
        XINT32(dataOptimize.nFileNameLen, pTemp);
        pTemp += sizeof(dataOptimize.nFileNameLen);
        
        /* 文件名 */
        memcpy(pTemp, dataOptimize.szFileName, dataOptimize.nFileNameLen);
        pTemp += dataOptimize.nFileNameLen;
        
        /* 文件编号 */
        XINT16(dataOptimize.nFileNO, pTemp);
        pTemp += sizeof(dataOptimize.nFileNO);
        
        /* 文件类型 */
        XINT16(dataOptimize.nFileType, pTemp);
        pTemp += sizeof(dataOptimize.nFileType);
        
        /* nAtion */
        XINT16(dataOptimize.nAction, pTemp);
        pTemp += sizeof(dataOptimize.nAction);
        
        /* highOffset */
        XINT32(dataOptimize.nHighOffset32, pTemp);
        pTemp += sizeof(dataOptimize.nHighOffset32);
        
        /* lowOffset */
        XINT32(dataOptimize.nLowOffset32, pTemp);
        pTemp += sizeof(dataOptimize.nLowOffset32);
        
        /* 数据长度 */
        XINT32(dataOptimize.nDataLen, pTemp);
        pTemp += sizeof(dataOptimize.nDataLen);
        
        stBuf[0].dwBufSize = dataOptimize.nSize;
        stBuf[1].pszBuf = pszIOBuff;
        stBuf[1].dwBufSize = dataOptimize.nDataLen;
        pTemp = NULL;
        nCurSeq++;
        IndexWriteSt.nSeqNum = nCurSeq;
        nRC = MiMSSqlWriteBufferToLog(m_hTargLog, stBuf, 2, &IndexWriteSt);
        if (nRC != WAVETOP_BACKUP_OK) {
           /* SLogErrorWrite(APLOG_MARK, APLOG_ERR | APLOG_NOERRNO, NULL,
                "IoWriteLogDataFile(): MiMSSqlWriteBufferToLog() failed."); */
            goto FAILED;
        }
       /* SLogErrorWrite(APLOG_MARK, APLOG_DEBUG, NULL, 
            "优化后的IO:filename:%s, Seq:%lld, offset:%lld, dataLen:%d",
            dataOptimize.szFileName, nCurSeq, dataOptimize.nOffSet, dataOptimize.nDataLen);*/
        
        memcpy(&dataOptimize, &dataNode, sizeof(IoDataHeadSt));
        if (dataNode.nDataLen > 0) {
            memset(pszIOBuff, 0, WAVETOP_IO_OPTIMIZE_BLOCK_SIZE);
            memcpy(pszIOBuff, pszIO, dataNode.nDataLen);    
        }
        
        if (pszDataBuff != NULL) {
            free(pszDataBuff);
            pszDataBuff = NULL;
        }
        
        /* 1)优化的最后一个IO块为前面的IO块不连续时 */
        if (   IndexReadSt.nSeqNum == nMaxNum 
            && dataNode.nDataLen > 0) {
           /* SLogErrorWrite(APLOG_MARK, APLOG_DEBUG, NULL, 
                "优化IO块结束,最后一个IO块为前面的IO块不连续时 Seq[%lld]", IndexReadSt.nSeqNum);*/
            IndexReadSt.nSeqNum++;
            /* 2)最后一个块为不是连续的块时先写入连续的块，在写入最后一个块 */
            goto TASKEND;
        }
        /* 3)优化的最后一个IO块和前一个块连续时只写入一次 */
        else if (   IndexReadSt.nSeqNum == nMaxNum
            && dataNode.nDataLen == 0) {
            /*SLogErrorWrite(APLOG_MARK, APLOG_DEBUG, NULL, 
                "优化IO块结束, 最后一个IO块为前面的IO块连续时 Seq[%lld]", IndexReadSt.nSeqNum);*/
            break;
        }
        else if (IndexReadSt.nSeqNum > nMaxNum) {
            break;
        }
    }
    
FAILED:
    if (pszIOBuff != NULL) {
        free(pszIOBuff);
        pszIOBuff = NULL;
    }

    if (pszBuff != NULL) {
        free(pszBuff);
        pszBuff = NULL;
    }

    if (pszDataBuff != NULL) {
        free(pszDataBuff);
        pszDataBuff = NULL;
    }
    return nRC;
    
}

int CIoOptimizeObject::IoDataOptimize(PRUint64 nMinNum, PRInt64 nMaxNum, PRInt64 nWriteBegNum)
{
    unsigned long nReadBytes       = 0;
    PRInt16 nFileNo                = 0;
    PRInt32 nStrLen                = 0;
    PRUint32 nHighOffset32         = 0;
    PRUint32 nLowOffset32          = 0;
    PRInt32 nDataLen               = 0; 
    PRInt64 nHighOffset64          = 0; 
    PRUint64 nLastSeqNum           = 0;
    bool bFinish                   = true;
    char *pTemp                    = NULL;
    char *pszFileName              = NULL;
    char *pszData                  = NULL;
    ExIndexSt IndexSt;
    IoDataNodeSt  *pIoDataNode     = NULL;
    IoDataInfoSt  *pIoFileNameList = NULL;
    IoChuckListSt *pIoChuckList    = NULL;
    pool *pPool                    = NULL;
    int nRC                        = WAVETOP_BACKUP_OK;
    int nMemLimitByte              = 0;
    
    nRC = MiMSSqlReadBufferFromLogStart(m_hSrcLog, nMaxNum, 1);
    if (nRC != WAVETOP_BACKUP_OK) {
        SLogErrorWrite(APLOG_MARK, APLOG_ERR | APLOG_NOERRNO, NULL, 
            "IoDataOptimize(): MiMSSqlReadBufferFromLogStart() failed. code %d", nRC); 
        goto FAILED;
    }

    pPool = ap_make_sub_pool(NULL);
    if (pPool == NULL) {
        SLogErrorWrite(APLOG_MARK, APLOG_ERR | APLOG_NOERRNO, NULL,
            "IoDataOptimize(): ap_make_sub_pool() failed");
        nRC = WAVETOP_BACKUP_NO_MEMORY;
        goto FAILED;
    }

    pszFileName = (char *)ap_palloc(pPool, WAVETOP_CDP_ORA_MAX_FILE_NAME);
    if (pszFileName == NULL) {
        SLogErrorWrite(APLOG_MARK, APLOG_ERR | APLOG_NOERRNO, NULL,
            "IoDataOptimize(): ap_palloc() failed");
        nRC = WAVETOP_BACKUP_NO_MEMORY;
        goto FAILED;
    }
    memset(pszFileName, 0, WAVETOP_CDP_ORA_MAX_FILE_NAME);
    
    pszData = (char *)ap_palloc(pPool, WAVETP_CDP_ORA_BLOCK_SIZE);
    if (pszData == NULL) {
        SLogErrorWrite(APLOG_MARK, APLOG_ERR | APLOG_NOERRNO, NULL,
            "IoDataOptimize(): ap_palloc() failed");
        nRC = WAVETOP_BACKUP_NO_MEMORY;
        goto FAILED;        
    }
    memset(pszData, 0, WAVETP_CDP_ORA_BLOCK_SIZE);


    while (true) {
        /* 
         * MSSQL 数据库时数据文件和日志文件时连续写的，因此无需合并重复的IO块 
         * ORACLE数据库的控制文件和日志是重复写的，因此合并重复的IO块
         * XXX: 目前只考虑了MSSQL和ORACLE数据库
         */
        if (m_DBtype == WAVETOP_MSSQL_DB) {
            break;
        }
        pIoDataNode = (IoDataNodeSt*)ap_palloc(pPool, sizeof(IoDataNodeSt));
        if (pIoDataNode == NULL) {
            SLogErrorWrite(APLOG_MARK, APLOG_ERR | APLOG_NOERRNO, NULL, 
                "IoDataOptimize(): ap_palloc failed.");
            nRC = WAVETOP_BACKUP_NO_MEMORY;
            goto FAILED;
        }
        memset(pIoDataNode, 0, sizeof(IoDataNodeSt));

        do {
            /* 逆序读取数据 */
            nRC = MiMSSqlReadBufferFromLogPrev(m_hSrcLog, pszData, WAVETP_CDP_ORA_BLOCK_SIZE, 
                &nReadBytes, 0, &IndexSt);        
            switch(nRC) {
            case WAVETOP_BACKUP_OK:
                bFinish = true;
                break;
            case WAVETOP_MIRROR_MSSQL2_LOG_MORE_WITH_LINE:
                bFinish = false;
                break;
            case WAVETOP_BACKUP_END:
                bFinish = true;
                break;
            default:
                SLogErrorWrite(APLOG_MARK, APLOG_ERR | APLOG_NOERRNO, NULL, 
                    "IoDataOptimize(): MiMSSqlReadBufferFromLogPrev() failed. code %d", nRC);
                goto FAILED;
            }

            if (nLastSeqNum != IndexSt.nSeqNum) {
                
                pTemp = pszData;

                /* 获取文件名长度 */
                VINT32(nStrLen, pTemp);
                pTemp += sizeof(nStrLen);
                if (nStrLen < 0) {
                    SLogErrorWrite(APLOG_MARK, APLOG_ERR | APLOG_NOERRNO, NULL, 
                        "IoDataOptimize(): Bad file name.");
                    nRC = WAVETOP_BACKUP_INVALID_SYNTAX;
                    goto FAILED;
                }
                else if (nStrLen >= WAVETOP_CDP_ORA_MAX_FILE_NAME) {
                    SLogErrorWrite(APLOG_MARK, APLOG_ERR | APLOG_NOERRNO, NULL, 
                        "IoDataOptimize(): no memory.");
                    nRC = WAVETOP_BACKUP_NO_MEMORY;
                    goto FAILED;    
                }
                
                 /* 获取文件名 */
                 memcpy(pszFileName, pTemp, nStrLen);
                 pszFileName[nStrLen] = '\0';
                 pTemp += nStrLen;

                 /* 文件编号 */
                 VINT16(nFileNo, pTemp);
                 pTemp += sizeof(nFileNo);

                 /* 文件类型  */
                 pTemp += sizeof(nFileNo);

                 /* nAction */
                 pTemp += sizeof(nFileNo);

                 /* highOffset */
                 VINT32(nHighOffset32, pTemp);
                 pTemp += sizeof(nHighOffset32);

                 /* lowOffset*/
                 VINT32(nLowOffset32, pTemp);
                 pTemp += sizeof(nLowOffset32);

                 nHighOffset64 = nHighOffset32;

                 pIoDataNode->nOffset = (nHighOffset64 << 32) + nLowOffset32;

                 /*  获取数据长度 */
                 VINT32(nDataLen, pTemp);

                 pIoDataNode->nSize = nDataLen;
                 pIoDataNode->nSeqNum = IndexSt.nSeqNum;

            }

            nLastSeqNum = IndexSt.nSeqNum; 

        } while (bFinish != true);

        if (IndexSt.nSeqNum <= nMinNum)
            break;
        
        /* 只优化控制文件和在线日志文件，其他文件过滤 */
        if (m_DBtype == WAVETOP_ORACLE_DB) {
            if (nFileNo != WAVETOP_CDP_ORA_CONTROL_FILE &&
                nFileNo != WAVETOP_CDP_ORA_REDOLOG_FILE) {
                 continue;
            } 
        }
        
        /* 1) io 块对应的文件名查找行和添加行 */
        nRC = IoNewAndSelectRow(pPool, pszFileName, &pIoFileNameList);
        if (nRC != WAVETOP_BACKUP_OK) {
            SLogErrorWrite(APLOG_MARK, APLOG_ERR | APLOG_NOERRNO, NULL,
                "IoDataOptimize(): IoNewAndSelectRow() failed.");
            goto FAILED;
        }
        
        /* 2) 从hash数组里检查io块是否重复的, 重复的io块丢弃，否则添加到hash数组 */
        nRC = IoAddAndCheckNode(pIoDataNode, pIoFileNameList, &pIoChuckList);
        if (nRC != WAVETOP_BACKUP_OK) {
            SLogErrorWrite(APLOG_MARK, APLOG_ERR | APLOG_NOERRNO, NULL,
                "IoDataOptimize():IoAddAndCheckNode() failed.");
            goto FAILED;
        }
        
        /* 3) 丢弃的节点添加到丢弃链表 */
        if(pIoChuckList != NULL) {
            nRC = IoNewChuckList(pIoChuckList);
            if (nRC != WAVETOP_BACKUP_OK) {
                SLogErrorWrite(APLOG_MARK, APLOG_ERR | APLOG_NOERRNO, NULL,
                    "IoDataOptimize():IoNewChuckList() failed.");
                goto FAILED;
            }
            pIoChuckList = NULL;
            nMemLimitByte += sizeof(IoDataNodeSt) + sizeof(IoChuckListSt);
        }
        else {
            nMemLimitByte += sizeof(IoDataNodeSt);
        }

        if (nMemLimitByte > WAVETOP_IO_MEMLIMIT) {
            SLogErrorWrite(APLOG_MARK, APLOG_ERR | APLOG_NOERRNO, NULL,
                "IoDataOptimize():内存超过制定的伐值, MEM[%d]", nMemLimitByte);
            break;
        }
        
    }

    /* 4) 丢弃链表构造完成后释放hash数组 */
    if (pPool != NULL) {
         ap_destroy_pool(pPool);
         pPool = NULL;
    }

    /* 5) 根据丢弃链表重新写logdata文件 */
    nRC = IoWriteLogDataFile(nMinNum, nMaxNum, nWriteBegNum);
    if (nRC != WAVETOP_BACKUP_OK) {
        SLogErrorWrite(APLOG_MARK, APLOG_ERR | APLOG_NOERRNO, NULL,
            "IoDataOptimize():IoWriteLogDataFile() failed.");
    }
    
FAILED:
    if (pPool != NULL) {
        ap_destroy_pool(pPool);
        pPool = NULL;
    }    

    return nRC;

}

int CIoOptimizeObject::UnInit()
{
    if (m_pIoPool != NULL) {
        ap_destroy_pool(m_pIoPool);
        m_pIoPool = NULL;
    }

    if (m_hSrcLog != NULL) {
        MiMSSqlReadBufferEnd(m_hSrcLog);
        m_hSrcLog = NULL;
    }

    if (m_hTargLog != NULL) {
        MiMSSqlReadBufferEnd(m_hTargLog);
        m_hTargLog = NULL;
    }
    
    return WAVETOP_BACKUP_OK;
}


