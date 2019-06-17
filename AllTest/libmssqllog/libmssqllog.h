/** =============================================================================
 ** Copyright (c) 2006 WaveTop Information Corp. All rights reserved.
 **
 ** The Mirror system
 **
 ** =============================================================================
 */

#ifndef _LIBMI_MSSQL_LOG_H_
#define _LIBMI_MSSQL_LOG_H_ 1

#include "backup_proto.h"
#include "nspr.h"
#include "walloc.h"

#define WAVETOP_MIRROR_MSSQL2_LOG_INTERNAL_FAILED   1
#define WAVETOP_MIRROR_MSSQL2_LOG_LINE_END          2
#define WAVETOP_MIRROR_MSSQL2_LOG_MORE_WITH_LINE    3

#define WAVETOP_MIRROR_MSSQL2_LOG_RESTARE           3
#define WAVETOP_MIRROR_MSSQL2_LOG_BROKEN            4
#define WAVETOP_MIRROR_MSSQL2_LOG_SWITCH            5

#define WAVETOP_MIRROR_MSSQL2_LOG_TYPE_TCP          10
#define WAVETOP_MIRROR_MSSQL2_LOG_TYPE_PIPE         11

/* The export API definition */
#ifdef WIN32
#ifndef MILOG_EXPORT
#define MILOG_EXPORT               __declspec(dllexport)
#endif
#ifndef MILOG_EXPORT_
#define MILOG_EXPORT_(__type)      __declspec(dllexport) __type
#endif
#else
#ifndef MILOG_EXPORT
#define MILOG_EXPORT               extern
#endif
#ifndef MILOG_EXPORT_
#define MILOG_EXPORT_(__type)      extern __type
#endif
#endif

typedef void * MiLogHandle;

/** Index node **/
typedef struct INDEX_EXNODE_ST{
    /* ��� */
    PRUint64 nSeqNum;
    /* ���� */
    unsigned char cType;
    /* socket */
    unsigned long nSocket;
    /* ���ݴ�С */
    unsigned long nDataSize;
    /* IP Address */
    char szIP[16];
    /* ��¼ʱ��. C���Ը�ʽ. ��Ϊ��λ. �����߲���д. */
    unsigned long nRecordTime;
} ExIndexSt;

//
// The buffer structure.
//
/*
typedef struct WTBUF {
    unsigned char *pszBuf;
    unsigned long dwBufSize;
    unsigned long dwPos;
} WTBUF, *LPWTBUF;*/

typedef struct WTBUF {
	ap_pool *pPool;
	PRUint32 dwBufSize;
	PRUint32 dwPos;
	PRUint8 *pszBuf;
	struct WTBUF *pNext;
	struct WTBUF *pTail;
} WTBUF, *LPWTBUF;
#ifdef __cplusplus
extern "C" {
#endif /* defined(__cplusplus) */

/**
 * ��ʼ����ʹ�ù����д�ļ���ʽ
 * @[in]
 * pszLogPath - ָ������־�ļ���洢·��
 * nOperation - 1 - reading. 2 - writing.
 * @[out]
 * pHandle - ���ش򿪵��ļ����
 * �ɹ�����WAVETOP_BACKUP_OK
 * ʧ�ܷ���WAVETOP_BACKUP_INTERNAL_ERROR
 */
MILOG_EXPORT_(int) MiMSSqlLogStart(MiLogHandle *pHandle,
                                   char *pszLogPath,
                                   int nOperation,
                                   int nPort = 0);
    
MILOG_EXPORT_(int) MiMSSqlLogStartEx(MiLogHandle *ppHandle, 
                                     char *pszLogPath, 
                                     int nOperation,
                                     char *pszSuff);

/**
 * ���������е�����д��ָ����־�ļ�
 * @[in]
 * pHandle - �򿪵���־�ļ��ľ��
 * pszBuffer - ��д����־�ļ�������
 * nBuffCount - ��д����־�ļ����ݵĳ���
 * nSocket - ��д����־�ļ����׽���
 * pszIP - ��д����־�ļ���IP��ַ
 * nType - д������
 * nNum - ָ��д������
 * @[out]
 * �ɹ�����WAVETOP_BACKUP_OK
 * ʧ�ܷ���WAVETOP_BACKUP_INTERNAL_ERROR
 */
MILOG_EXPORT_(int) MiMSSqlWriteBufferToLog(MiLogHandle pHandle,
                                                  WTBUF *pBuffers,
                                                  unsigned long nBuffCount,
                                                  ExIndexSt *pIndex);

/**
 * ָ����ĳһ��ſ�ʼ��ȡ����
 * @[in]
 * Handle - �򿪵���־�ļ��ľ��
 * nSeqNum - ��ʼ��š�
 * nType - �����1����ȡ��ʽΪ��ָ�������ĩβ��ͷ���������2����ȡ��ʽΪ����ȡָ�����
 * @[out]
 * �ɹ�����WAVETOP_BACKUP_OK
 * ʧ�ܷ���WAVETOP_BACKUP_INTERNAL_ERROR
 */
MILOG_EXPORT_(int) MiMSSqlReadBufferFromLogStart(MiLogHandle hHandle,
                                                        PRUint64 nSeqNum,
                                                        int nType);

/**
 * ѭ�����ø÷�������ȡ��ָ����ſ�ʼ��ĩβ������
 * @[in]
 * pHandle - �򿪵���־�ļ��ľ��
 * pszBuffer - ��д�뻺����
 * nSize - ��д�뻺��������
 * nSocket - ����Ϊ0ʱ��ȡָ���׽��ֵ���һ�����ݣ���Ϊ0ʱ����
 * @[out]
 * pnReadBytes - ������д�����ݵĳ���
 * pIndex      - ���������ڵ�.
 * ���ͬһ��Ŷ�ȡʱ�������������Ƚ����еĻ�����д��
 * ������WAVETOP_MIRROR_MORE_WITH_LINE��Ȼ���ٴε��ô˷������Ŷ�ȡ
 * ����Ŷ�ȡ��ɷ���WAVETOP_BACKUP_OK
 * �Ѷ�ȡ��ĩβ����WAVETOP_MIRROR_LINE_END
 * �����쳣����WAVETOP_BACKUP_INTERNAL_ERROR
 */
MILOG_EXPORT_(int) MiMSSqlReadBufferFromLogNext(MiLogHandle hHandle,
                                                       char *pszBuffer,
                                                       unsigned long nSize,
                                                       unsigned long *pnReadBytes,
                                                       unsigned long nSocket,
                                                       ExIndexSt *pIndex);

/**
 * �Ľ���һ���Լ������ݺ������ļ����ڴ棬����Io������
 * Ȼ��ѭ�����ø÷�������ȡ��ָ����ſ�ʼ��ĩβ������
 * @[in]
 * pHandle - �򿪵���־�ļ��ľ��
 * pszBuffer - ��д�뻺����
 * nSize - ��д�뻺��������
 * nSocket - ����Ϊ0ʱ��ȡָ���׽��ֵ���һ�����ݣ���Ϊ0ʱ����
 * @[out]
 * pnReadBytes - ������д�����ݵĳ���
 * pIndex      - ���������ڵ�.
 * nDelFalg    - 0�Ǿ���ķ�ʽ��1��acdp�ķ�ʽ
 * ���ͬһ��Ŷ�ȡʱ�������������Ƚ����еĻ�����д��
 * ������WAVETOP_MIRROR_MORE_WITH_LINE��Ȼ���ٴε��ô˷������Ŷ�ȡ
 * ����Ŷ�ȡ��ɷ���WAVETOP_BACKUP_OK
 * �Ѷ�ȡ��ĩβ����WAVETOP_MIRROR_LINE_END
 * �����쳣����WAVETOP_BACKUP_INTERNAL_ERROR
 */
MILOG_EXPORT_(int) MiMSSqlReadBufferFromLogNextEx(MiLogHandle hHandle,
    char *pszBuffer,
    unsigned long nSize,
    unsigned long *pnReadBytes,
    unsigned long nSocket,
    ExIndexSt *pIndex,
    int nDelFalg = 1);


/**
 * ѭ�����ø÷�������ȡ��ָ����ſ�ʼ��ͷ��������
 * @[in]
 * pHandle - �򿪵���־�ļ��ľ��
 * pszBuffer - ��д�뻺����
 * nSize - ��д�뻺��������
 * nSocket - ����Ϊ0ʱ��ȡָ���׽��ֵ���һ�����ݣ���Ϊ0ʱ����
 * @[out]
 * pnReadBytes - ������д�����ݵĳ���
 * pIndex      - ���������ڵ�.
 * ���ͬһ��Ŷ�ȡʱ�������������Ƚ����еĻ�����д��
 * ������WAVETOP_MIRROR_MORE_WITH_LINE��Ȼ���ٴε��ô˷������Ŷ�ȡ��
 * ����Ŷ�ȡ��ɷ���WAVETOP_BACKUP_OK
 * �Ѷ�ȡ��ͷ������WAVETOP_MIRROR_LINE_END
 * �����쳣����WAVETOP_BACKUP_INTERNAL_ERROR
 */
MILOG_EXPORT_(int) MiMSSqlReadBufferFromLogPrev(MiLogHandle hHandle,
                                                       char *pszBuffer,
                                                       unsigned long nSize,
                                                       unsigned long *pnReadBytes,
                                                       unsigned long nSocket,
                                                       ExIndexSt *pIndex);

/**
 * �÷������Ի�ȡ��־�ļ���������С�����
 * @[in]
 * pHandle - �򿪵���־�ļ��ľ��
 * @[out]
 * pnMax - ���ȡ�õ�������ֵ
 * pnMin - ���ȡ�õ���С���ֵ
 * �ɹ�����WAVETOP_BACKUP_OK
 * ʧ�ܷ���WAVETOP_BACKUP_INTERNAL_FAILED
 */
MILOG_EXPORT_(int) MiMSSqlGetMaxAndMinNum(MiLogHandle hHandle,
                                                 PRUint64 *pnMax,
                                                 PRUint64 *pnMin);

/**
 * ָ����ĳһ��Ŷ�ȡ���ݽ���
 * pHandle - �򿪵���־�ļ��ľ��
 * @[out]
 * �ɹ�����WAVETOP_BACKUP_OK
 * ʧ�ܷ���WAVETOP_BACKUP_INTERNAL_ERROR
 */
MILOG_EXPORT_(int) MiMSSqlReadBufferEnd(MiLogHandle hHandle);

/**
 * �Զ�ɾ��û�б�ʹ�õ���־�ļ�
 * pHandle - �򿪵���־�ļ��ľ��
 * @[out]
 * �ɹ�����WAVETOP_BACKUP_OK
 * ʧ�ܷ���WAVETOP_BACKUP_INTERNAL_ERROR
 */
MILOG_EXPORT_(int) MiMSSqlDeleteLog(MiLogHandle hHandle);

/**
 * �򿪹����ڴ�WAVETOP_MMShare_INSTANCE_DBNAME
 * ������������ڴ���nWay=WAVETOP_MIRROR_MSSQL2_LOG_BROKEN
 * ����nStatus=WAVETOP_MIRROR_MSSQL2_LOG_BROKEN
 * ����nStatus=0
 * @[out]
 * �ɹ�����WAVETOP_BACKUP_OK
 * ʧ�ܷ���WAVETOP_BACKUP_INTERNAL_ERROR
 */
MILOG_EXPORT_(int) BkMSSqlOpenShareMM(MiLogHandle hHandle, PRInt32 nWay, PRInt32 *nStatus);

/**
 * �򿪻��ߴ��������ڴ�WAVETOP_MMShare_INSTANCE_DBNAME
 * �ҵ�û�б�ʹ�õĹ����ڴ�飬Ȼ��д��
 * nWay=WAVETOP_MIRROR_MSSQL2_LOG_RESTARE
 * @[out]
 * �ɹ�����WAVETOP_BACKUP_OK
 * ʧ�ܷ���WAVETOP_BACKUP_INTERNAL_ERROR
 */
#ifdef WIN32
MILOG_EXPORT_(int) BkMSSqlWriteShareMM(MiLogHandle hHandle, PRInt32 nWay);
#endif
/*
 * ����������������backup6ʹ��.
 */
/**
* ���ڱ���ϵͳoracle������д��logdata
* ���ʱ�������ʱ���ǿͻ��˴�������
* ���������ڵĻ�ȡ��ǰд��ʱ��
* ���������е�����д��ָ����־�ļ�
* @[in]
* pHandle - �򿪵���־�ļ��ľ��
* pszBuffer - ��д����־�ļ�������
* nBuffCount - ��д����־�ļ����ݵĳ���
* pIndex - ���ʱ������Ľṹ
* @[out]
* �ɹ�����WAVETOP_BACKUP_OK
* ʧ�ܷ���WAVETOP_BACKUP_INTERNAL_ERROR
*/
MILOG_EXPORT_(int) BkMSSqlWriteBufferToLog(MiLogHandle hHandle,
                                           WTBUF *pBuffers,
                                           unsigned long nBuffCount,
                                           ExIndexSt *pIndex);

/**
 * �÷������Ի�ȡ��־�ļ����ļ���ID���������Seq��
 * @[in]
 * pHandle - �򿪵���־�ļ��ľ��
 * nCurrSeq - ��һ��ȫ���汾��Seq 
 * @[out]
 * nBufLen - ȡ�ø���־�ļ���ID���
 * pnMin - ���ȡ�õ���С���ֵ(Seqֵ)
 * �ɹ�����WAVETOP_BACKUP_OK
 * ʧ�ܷ���WAVETOP_BACKUP_INTERNAL_FAILED
 */
MILOG_EXPORT_(int) BkMSSqlGetFileName(MiLogHandle hHandle, PRUint64 nCurrSeq, 
                                      int *nBufLen, PRUint64 *pnMin);
/**
 * ѭ�����ø÷�������ȡ��ָ����ſ�ʼ��ĩβ������,����ɾ����ǰ�Ѿ������ǰһ���ļ�
 * @[in]
 * pHandle - �򿪵���־�ļ��ľ��
 * pszBuffer - ��д�뻺����
 * nSize - ��д�뻺��������
 * nSocket - ����Ϊ0ʱ��ȡָ���׽��ֵ���һ�����ݣ���Ϊ0ʱ����
 * @[out]
 * pnReadBytes - ������д�����ݵĳ���
 * pIndex      - ���������ڵ�.
 * ���ͬһ��Ŷ�ȡʱ�������������Ƚ����еĻ�����д��
 * ������WAVETOP_MIRROR_MORE_WITH_LINE��Ȼ���ٴε��ô˷������Ŷ�ȡ
 * ����Ŷ�ȡ��ɷ���WAVETOP_BACKUP_OK
 * �Ѷ�ȡ��ĩβ����WAVETOP_MIRROR_LINE_END
 * �����쳣����WAVETOP_BACKUP_INTERNAL_ERROR
 */
MILOG_EXPORT_(int) BkMSSqlReadBufferFromLogNext(MiLogHandle hHandle,
                                                       char *pszBuffer,
                                                       unsigned long nSize,
                                                       unsigned long *pnReadBytes,
                                                       unsigned long nSocket,
                                                       ExIndexSt *pIndex);


/*------------------------------- IO���Ż��ӿ�-----------------------------*/

/**
 * IO�Ż���ʼ��
 * @[in]
 * pIoDataSrcDir  - ��Ҫ�Ż���ָ����־�ļ���洢·��
 * pIoDataTargDir - �Ż���ɺ����־�ļ���洢·��
 * pIoDataName    - ��־�ļ�����ʽ
 * nDBtype - nDBtype == 1 ORACLE ���ݿ�, nDBtype == 2 MSSQL���ݿ�
 * @[out]
 * ppHandle -  �򿪵�IO���Ż��ļ����
 * �ɹ����� WAVETOP_BACKUP_OK ���򷵻ش������
 */
MILOG_EXPORT_(int) BKMSSqlIoOptimizeBgein(MiLogHandle *ppHandle, char *pIoDataSrcDir, 
                                          char *pIoDataTargDir, char *pIoDataName, int nDBtype);
/**
 * IO���Ż�
 * @[in]
 * pHandle - �򿪵��Ż��ļ����
 * nMinNum - IO���Ż���ʼ���к�
 * nMaxNum - IO���Ż��������к�
 * nWriteBegNum - IO���Ż���ɺ�ʼд�����к�
 * @[out]
 * �ɹ����� WAVETOP_BACKUP_OK, ���򷵻ش������
 */

MILOG_EXPORT_(int) BKMSSqlIoOptimize(MiLogHandle pHandle, PRUint64 nMinNum, 
                                     PRInt64 nMaxNum, PRInt64 nWriteBegNum);

/**
 * IO���Ż�����
 * pHandle - �򿪵��Ż��ļ����
 * @[out]
 * �ɹ�����WAVETOP_BACKUP_OK ���򷵻ش������
 */
MILOG_EXPORT_(int) BKMSSqlIoOptimizeEnd(MiLogHandle pHandle);


/**
 * �÷������Ի�ȡ��־�ļ����ļ���ID���
 * @[in]
 * pHandle - �򿪵���־�ļ��ľ��
 * @[out]
 * pnFileNum - ȡ�ø���־�ļ���ID���
 * �ɹ�����WAVETOP_BACKUP_OK
 * ʧ�ܷ���WAVETOP_BACKUP_INTERNAL_FAILED
 */
MILOG_EXPORT_(int) BkMSSqlGetFileMinNum(MiLogHandle hHandle, PRInt32 *pnFileNum);

/**
 * �÷������Ի�ȡָ���ļ���ŵ���СSEQ
 * @[in]
 * pHandle - �򿪵���־�ļ��ľ��
 * nFileNum - ָ�����ļ����
 * @[out]
 * nMinSeq - ָ���ļ���ŵ���СSEQ  
 * �ɹ�����WAVETOP_BACKUP_OK
 * ʧ�ܷ���WAVETOP_BACKUP_INTERNAL_FAILED
 */
MILOG_EXPORT_(int) BkMSSqlGetFileNumMinSeq(MiLogHandle hHandle, PRInt32 nFileNum, PRUint64 *nMinSeq);

//ɾ�� �� nlastNum��� logdata�ļ�
MILOG_EXPORT_(int) MiMSSqlDeleteSuperMaxFileImmediate(MiLogHandle hHandle, int nLastNum);

//��ȡ ָ���ļ����ı��
MILOG_EXPORT_(int) MiMSSqlGetLastNum(MiLogHandle hHandle, int *nLastNum);

MILOG_EXPORT_(int) MiMSSqlSetLastNum(MiLogHandle hHandle, int nLastNum);

MILOG_EXPORT_(int) BkMSSqlGetFileMaxNum(MiLogHandle hHandle, int *pnFileNum);


MILOG_EXPORT_(int) MiMSSqlSetLastNum2(char *pszLogPath,char *pszSuff, int nLastNum);
MILOG_EXPORT_(int) BkMSSqlGetFileMaxNum2(char *pszLogPath,char *pszSuff, int *pnFileNum);
MILOG_EXPORT_(int) MiMSSqlDeleteFileImmediate2(char *pszLogPath,char *pszSuff, int nLastNum);
#ifdef __cplusplus
}
#endif /* defined(__cplusplus) */

#endif /* !defined(_LIBMI_MSSQL_LOG_H_) */
