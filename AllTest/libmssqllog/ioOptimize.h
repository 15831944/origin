/** =============================================================================
 ** Copyright (c) 2011 WaveTop Information Corp. All rights reserved.
 **
 ** The A-CDP system
 **
 ** =============================================================================
 */

#ifndef _WAVETOP_IO_OPTIMIZE_H_
#define _WAVETOP_IO_OPTIMIZE_H_

#include <string.h>
#include "nspr.h"
#include "server_log.h"
#include "libmssqllog.h"
#include "logfile.h"
#include "walloc.h"

#define WAVETP_CDP_ORA_BLOCK_SIZE        (256 * 1024)
#define WAVETOP_IO_OPTIMIZE_BLOCK_SIZE   (16 * 1024 * 1024)
#define WAVETOP_CDP_ORA_MAX_FILE_NAME    8192
#define WAVETOP_CDP_ORA_CONTROL_FILE     (1 << 0)
#define WAVETOP_CDP_ORA_REDOLOG_FILE     (1 << 1)
#define WAVETOP_ORACLE_DB                1
#define WAVETOP_MSSQL_DB                 2
#define WAVETOP_IO_MEMLIMIT              (1024*1024*1024)

/* ��ǰ�ڵ�ͷ��Ϣ�ṹ */
typedef struct _IO_DATA_HEAD_ST {
    int nFileNameLen;
    char szFileName[1024];
    PRInt16 nFileNO;
    PRInt16 nFileType;
    PRInt16 nAction;
    PRUint32 nHighOffset32;
    PRUint32 nLowOffset32;
    PRInt64 nOffSet;
    PRInt32 nDataLen;
	
    int nSize;
} IoDataHeadSt;


/* IO���ϣ��ڵ�ṹ */
typedef struct  _IO_DATA_NODE_ST {
	
    /* IO������ */
    PRUint64 nSeqNum;
    
    /* ƫ���� */
    PRInt64 nOffset;
	
    /* ���ݳ��� */
    long nSize;
	
	_IO_DATA_NODE_ST *pNext;
	
} IoDataNodeSt;

/* �ļ��������Ĺ�ϣ�������� */
typedef struct  _IO_DATA_INFO_ST {
    
	/*  IO���Ӧ�������ļ��� */
    char *pszFileName;
    
    /* ��ϣ���� */
    IoDataNodeSt **ppQueue;
	
    _IO_DATA_INFO_ST *pNext;
	
} IoDataInfoSt;

/* ����IO������ */
typedef struct _IO_CHUCK_LIST_ST {
    
    /* IO������ */
    PRUint64 nSeqNum;
	
	_IO_CHUCK_LIST_ST *pNext; 
	
} IoChuckListSt;


class CIoOptimizeObject {

public:    
    CIoOptimizeObject();
    virtual ~CIoOptimizeObject();

public:

    /* 
     * ͳһ��ʼ����������ʼ��pool�ڴ�����Ա����
     * @[in]
     * pIoDataSrcDir - Ҫ�Ż���IO����·��
     * pIoDataTargDir - �Ż���ɺ�IO����·��
     * pIoDataName - LogData �ļ���
	 * nDBtype - ��������,nDBtype == 1 ORACLE���ݿ�, nDBtype == 2 MSSQL���ݿ� 
     * @[out]
     * ��ʼ���ɹ�����WAVETOP_BACKUP_OK, ���򷵻�ʧ��״̬�롣
     */
    int Init(char *pIoDataSrcDir, char *pIoDataTargDir, char *pIoDataName, int nDBtype);
    
    /* 
     * �ϲ��ظ���IO����������LogData�ļ�
     * @[in]
     * nMinNum - IO���Ż���ʼ���к�
     * nMaxNum - IO���Ż��������к�
     * nWriteBegNum - IO���Ż���ɺ�ʼд�����к�
     * @[out]
     * �Ż��ɹ�����WAVETOP_BACKUP_OK, ���򷵻�ʧ��״̬�롣
     */
    int IoDataOptimize(PRUint64 nMinNum, PRInt64 nMaxNum, PRInt64 nWriteBegNum); 
    
    
    /* �ͷ���Դ */
    int UnInit();

private:

   /* ���¶���Hash�ķ���*/
   int Hash(const void *pKey);
   
   /* Hash �ȽϷ��� */
   int Compare(const void *pKey1, const void *pKey2);
   
   /* 
    * ��ȡ��IO����ļ�������λ�в����ع�ϣ�������û����
    * ������в����ع�ϣ����    
    * @[in]
    * pszFileName - IO���Ӧ���ļ���
    * ppIoDataIndexInfoSt - �ļ�����Ӧ�Ĺ�ϣ����
    * @[out]
    * �ɹ�����WAVETOP_BACKUP_OK, ���򷵻�ʧ��״̬��
    */
   int IoNewAndSelectRow(pool *pPool, char*pszFileName, IoDataInfoSt **ppIoDataInfoList);
   
   /*  
    * ��ϣ����������IO��Ľڵ�
    * �ж�IO���ǲ����ظ���IO�� 
    * ������ظ���IO����Ϊ�������
    * ���������������ӵ���ϣ����ڵ�Ͷ�������
    * @[in]
    * pIoDataNodeList - io��ṹ
    * pIoDataInfoList - io���ļ�����Ӧ��hash����
    * @[out]
    * ppIoChuckList - Ҫ���붪������Ľڵ�
    * �ɹ�����WAVETOP_BACKUP_OK, ���򷵻�ʧ��״̬��
    */
   int IoAddAndCheckNode(IoDataNodeSt *pIoDataNode, IoDataInfoSt *pIoDataInfoList, 
	                     IoChuckListSt **ppIoChuckList);
    
   /* ���춪������ */
   int IoNewChuckList(IoChuckListSt *pIoChuckList);

   /* ���ݶ�����������дlogdata�ļ� */
   int IoWriteLogDataFile(PRUint64 nMinNum, PRInt64 nMaxNum, PRInt64 nWriteBegNum);
  
private:
     ap_pool *m_pIoPool;

     int m_nBaseBucketNum;

     /* Ҫ�Ż�����־�ļ���� */
     MiLogHandle m_hSrcLog;

     /* Ŀ����־�ļ���� */
     MiLogHandle m_hTargLog;

     /* IO���Ż���ʼ���к�*/
     int m_nBeginNum;

     /* IO���Ż��������к� */
     int m_nEndNum;
     
	 /* ���ݿ����� m_DBtype == 1 ORACLE���ݿ�, m_DBtype == 2 MSSQL���ݿ�*/
	 int m_DBtype;
	 
     IoDataInfoSt *m_pIoFileNameList;

     /* ���������б� */
     IoChuckListSt *m_pIoChuckListSt;    
};


#endif /* _WAVETOP_IO_OPTIMIZE_H_ */