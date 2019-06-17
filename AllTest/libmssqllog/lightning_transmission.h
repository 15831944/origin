/**================================================================
 ** Copyright (c) 2016 WaveTop Information Corp. All rights reserved.
 **
 ** ͨ�ô�����ͷ�ļ�
 **
 ** ==============================================================
 */
/* IODAEMONֹͣ */
#define WAVETOP_LT_IODAEMON_STOP        -2
/* δ֪ */
#define WAVETOP_LT_UNKNOW               -1
/* �ɹ� */
#define WAVETOP_LT_OK                   0
/* ��ʼ��ʧ�� */
#define WAVETOP_LT_INIT_ERROR           1
/* ���� */
#define WAVETOP_LT_CONTINUE             2
/* �ڵ���װ�� */
#define WAVETOP_LT_DATA_FULL            1
/* �ڵ�δװ�� */
#define WAVETOP_LT_DATA_EMPTY           0
/* oracle log ���� */
#define WAVETOP_LT_ORACLE_SEND_MODE     3
/* oracle log ���� */
#define WAVETOP_LT_ORACLE_RECV_MODE     4
/* �����ļ��ɹ� */
#define WAVETOP_LT_SEND_FILE_ERROR      5
/* �����ļ�ʧ�� */
#define WAVETOP_LT_RECV_FILE_ERROR      6
/* ���ļ�ʧ�� */
#define WAVETOP_LT_OPEN_FILE_ERROE      7
/* ��ȡ�ļ�ʧ�� */
#define WAVETOP_LT_READ_FILE_ERROE      8
/* д���ļ�ʧ�� */
#define WAVETOP_LT_WRITE_FILE_ERROE     9
/* �����ļ� */
#define WAVETOP_LT_INCR_FILE            10
/* ȫ���ļ� */
#define WAVETOP_LT_FULL_FILE            11
/* ������Ч */
#define WAVETOP_LT_INVALID_PARAMETER    12
/* ��������ʧ�� */
#define WAVETOP_LT_NEW_DATA_ERROR       13
/* �����ڴ�ʧ�� */
#define WAVETOP_LT_MALLOC_DATA_ERROR    14
/* ��ȡ�߳�ʧ�� */
#define WAVETOP_LT_READ_THREAD_ERROR    15
/* �����߳�ʧ�� */
#define WAVETOP_LT_SEND_THREAD_ERROR    16
/* �����߳�ʧ�� */
#define WAVETOP_LT_RECV_THREAD_ERROR    17
/* д���߳�ʧ�� */
#define WAVETOP_LT_WRITE_THREAD_ERROR   18
/* ����ģʽ */
#define WAVETOP_LT_SEND_MODE            19
/* ����ģʽ */
#define WAVETOP_LT_RECV_MODE            20
/* �ļ�ƫ��ʧ�� */
#define WAVETOP_LT_SEEK_ERROR           21
/* ���ӶϿ� */
#define WAVETOP_LT_CONNECT_DOWN         200
/* ���ӳɹ� */
#define WAVETOP_LT_CONNECT_SUCCESS      201
/* ����ɹ� */
#define WAVETOP_LT_JOB_SUCCESS          202
/* Э����� */
#define WAVETOP_LT_PROTOCOL_ALIGNMENT   "Protocol_alignment"

/**
 * @���� ����������нڵ�ṹ��
 *
 * @��Ա nID �ڵ����к�
 * @��Ա nType ��д��������WAVETOP_LT_DATA_FULL��WAVETOP_LT_DATA_EMPTY
 * @��Ա nSize ���ݿ鳤��
 * @��Ա pBuff ���ݿ�
 *
 */
typedef struct lt_data {
    unsigned int nID;
    unsigned int nType;
    int nSize;
    unsigned char *pBuff;
} LTDatast;

/**
 * @���� �����͵��ļ��б�ṹ��
 *
 * @��Ա szFileName ��Ҫ���͵��ļ�������
 * @��Ա szTime ��Ҫ���͵��ļ���ʱ���
 * @��Ա cMode �ļ������ͣ�WAVETOP_LT_INCR_FILE �����ļ� WAVETOP_LT_FULL_FILE ȫ���ļ�
 * @��Ա pNext ��һ���ڵ��ַ
 *
 */
typedef struct lt_filelist {
    char szFileName[1024];
    unsigned char szTime[8];
    unsigned char cMode;
    lt_filelist *pNext;
} LTFilelistst;

/**
 * @���� ���в����ṹ��
 *
 * @��Ա pNetBuff socket���Ӿ����Ϣ
 * @��Ա nNodeSize ������е��ڵ㳤��
 * @��Ա nNodeCount ������нڵ����
 * @��Ա nMode ���ͽ��ձ�ʶ��WAVETOP_LT_SEND_MODE ���� WAVETOP_LT_RECV_MODE ���� WAVETOP_LT_ORACLE_SEND_MODE ORACLE_LOG���� WAVETOP_LT_ORACLE_RECV_MODE ORACLE_LOG����
 * @��Ա nThreadID �ͻ��˷����߳�ID
 * @��Ա szIP �ͻ���IP
 * @��Ա szFileNumber �ѷ��͵��ļ����к�
 * @��Ա szCurrentOffset �ѷ��͵��ļ�ƫ����
 * @��Ա szWorkPath ��ǰ·��
 * @��Ա pFiles �跢�͵��ļ��б�
 *
 */
typedef struct lt_config {
    void *pNetBuff;
    unsigned int nNodeSize;
    unsigned int nNodeCount;
    unsigned int nMode;
    unsigned int nThreadID;
    char szIP[16];
    char szBeginFileNumber[12];
    char szFileCount[12];
    char szCurrentOffset[12];
    char szEndOffset[12];
    char szWorkPath[1024];
    LTFilelistst *pFiles;
} LTConfigst;

/**
 * @���� ����״̬�ṹ��
 *
 * @��Ա nReadThreadStatus ��ȡ�߳�״̬
 * @��Ա nSendThreadStatus �����߳�״̬
 * @��Ա nRecvThreadStatus �����߳�״̬
 * @��Ա nWriteThreadStatus д���߳�״̬
 * @��Ա nMessage ��Ϣ
 * @��Ա nAction ��Ϣ֪ͨ
 * @��Ա szFileNumber ��ǰ�ļ����
 * @��Ա szCurrentOffset ��ǰ�ļ�ƫ����
 *
 */
typedef struct lt_runstatus {
    int nReadThreadStatus;
    int nSendThreadStatus;
    int nRecvThreadStatus;
    int nWriteThreadStatus;
    int nMessage;
    int nAction;
    char szCompletedFile[12];
    char szFileNumber[12];
    char szCurrentOffset[32];
} LTRunStatusst;

class Clightning_transmission
{

public:

    Clightning_transmission();
    ~Clightning_transmission();

    /**
     * @���� ��ʼ��Clightning_transmission��ı�Ҫ�����в���
     *
     * @���� pstConfig
     *      [in] ���в�����Ϣ
     *
     * �ɹ����� WAVETOP_BACKUP_OK
     * ʧ�ܷ��� ����ͷ�ļ�����
     */
    int LTInit(LTConfigst *pstConfig);
    
    /**
     * @���� �����ʼ��ʱ��������в�����Ϣ
     *
     */
    void LTUnit();

    /**
     * @���� �ļ�����
     *
     * @���� pNetBuff
     *      [in] �������ӽṹ��netbuff
     *
     * �ɹ����� WAVETOP_BACKUP_OK
     * ʧ�ܷ��� ����ͷ�ļ�����
     */
    int LTContinue(void *pNetBuff);

    /**
     * @���� ѭ�����û�ȡͨ�ô����ļ��������״̬
     *
     * @���� pstRunStatus
     *      [out] ��������״̬
     */
    void LTGetStat(LTRunStatusst *pstRunStatus);

    /**
     * @���� �ͻ��˷����֮����Ϣ����
     *
     * @���� nAction
     *      [in] ��Ϣ
     */
    void LTMessage(int *nAction);

    /**
     * @���� oracle log �����������
     *
     * @���� pNode
     *      [in] �ڵ�
     */
    int LTInput(void *pNode);

    /**
     * @���� oracle log ȡ���������
     *
     * @���� pNode
     *      [in] �ڵ�
     */
    int LTOutput(void *pNode);

private:

    /**
     * @���� �������ݿ��߳�
     *
     * @���� pParm
     *      [in] ������thisָ��
     */
    static void LTSendThread(void *pParm);

    /**
     * @���� �������ݿ��߳�
     *
     * @���� pParm
     *      [in] ������thisָ��
     */
    static void LTRecvThread(void *pParm);

    /**
     * @���� ��ȡ�ļ��߳�
     *
     * @���� pParm
     *      [in] ������thisָ��
     * 
     * 
     */
    static void LTReadThread(void *pParm);
    
    /**
     * @���� д���ļ��߳�
     *
     * @���� pParm
     *      [in] ������thisָ��
     * 
     * 
     */
    static void LTWriteThread(void *pParm);

    /**
     * @���� ����ͨ�ļ�����ռ�ļ�
     *
     * @���� pszFileName
     *      [in] �����ļ���
     * 
     * 
     */
    int LTOpen(char *pszFileName);

    /**
     * @���� �ر��ļ����
     *
     * 
     * 
     */
    void LTClose();

    /**
     * @���� ƫ�Ƶ�ָ��ƫ����
     *
     * @���� nOffset
     *      [in] �����ļ�ƫ���� 
     * 
     */
    int LTSeek(void *nOffset, int nMode);

    /**
     * @���� д���ļ�
     *
     * @���� pszData
     *      [in] �����д���ڴ��ַ
     * @���� nDataBytes
     *      [in] �����д���ڴ泤��
     * 
     * 
     */
    int LTWrite(unsigned char *pszData, int nDataBytes);    

    /**
     * @���� ��ȡ�ļ�
     *
     * @���� pszData
     *      [in] �������ȡ���ڴ��ַ
     * @���� pnBytesOfRead
     *      [in] �������ȡ�ĳ��� 
     * 
     */
    int LTRead(unsigned char *pszData, void *pnBytesOfRead);

    /**
     * @���� ��ȡ�����ļ�
     *
     * @���� nNumber
     *      [in] �����ļ����к�
     * 
     * 
     */
    int LTSwitchIncFile(void *nNumber);

    /**
     * @���� д�������ļ�
     *
     * @���� nFileID
     *      [in] �����ļ����к�
     * @���� nFileOffset
     *      [in] �����ļ�ƫ����
     * @���� nMode
     *      [in] �����ļ����� ����/ȫ��
     * 
     * 
     */
    int LTWriteIndexFile(void *nFileID, void *nFileOffset, int nMode);

    /**
     * @���� ��ȡ�߳�ID
     * 
     * 
     */
    int LTGetThreadID();

    /**
     * @���� ��ȡ������Сlogdata�ļ����
     * 
     */
    void LTGetFileID();

private:

    /* �����߳̾�� */
    void *m_phSend;

    /* �����߳̾�� */
    void *m_phRecv;

    /* ��ȡ�߳̾�� */
    void *m_phRead;

    /* д���߳̾�� */
    void *m_phWrite;

    /* ��ǰ�򿪵��ļ���� */
    void *m_hFile;

    /* �߳��˳���ʶ */
    int m_nQuit;

    /* ���ӳɹ���ʶ */
    int m_nConnect;

    /* �߳����� */
    int m_nThreadCount;

    /* oracle log������м����� */
    int m_nOracleNum;

    /* ȫ�������ļ� */
    char m_szFullIndex[1024];

    /* ���������ļ� */
    char m_szIncIndex[1024];

    /* ���в�����Ϣ */
    LTConfigst *m_pstConfig;

    /* ������� */
    LTDatast *m_pstData;

    /* ����״̬ */
    LTRunStatusst m_stRunInfo;

};


