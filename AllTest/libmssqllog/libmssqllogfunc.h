#include "ioOptimize.h"
#include "libmssqllog.h"
#include "logfile.h"
#include "sqloptimize.h"
#include <iostream>
#include "nspr.h"
#include "sqlite3.h"
#include "mssqlserv.h"
#include "backup_proto.h"
#include "walloc.h"

using namespace std;

#define MI_MSSQL_CONFIG_FILE   "sqlservmirror.conf"
#define MI_MSSQL_ERR_LOG       "err.log"
#define MI_MSSQL_HASH_ACTOR    256
#define MI_MSSQL_MEMLIMIT_DEFAULT 1024
#define MI_MSSQL_REPORT_OPEN    1
#define MI_MSSQL_RECORED_IN_MEMORY 5000
#define MI_MSSQL_REPT_DEF_INTERVAL 60 * 30

#define WTMI_MONITOR_STATUS_FLAGE_PAUSE 24

#define MI_MSSQL_REPT_INTERVAL "MsReptInterval"
#define MI_MSSQL_INSTANCE      "InstanceName"
#define MI_MSSQL_HOSTIP        "MssqlIP"
#define MI_MSSQL_PORT          "MssqlPort"
#define MI_MSSQL_MODPORT       "ModPort"
#define MI_MSSQL_USER          "MssqlUser"
#define MI_MSSQL_PASSWORD      "MssqlPassword"
#define MI_MSSQL_LOGPATH       "MssqlLogPath"
#define MI_MSSQL_DATAPATH      "MssqlDataPath"
#define MI_MSSQL_IMPPATH       "MssqlIMPPath"
#define MI_MSSQL_BAKPATH       "MssqlBAKPath"
#define MI_MSSQL_DBCOUNT       "DBCnts"
#define MI_MSSQL_TRANSTRACE    "TransTrace"
#define MI_MSSQL_PAUSERESTOR   "RestoreSeq"
#define MI_MSSQL_SELECTTIME    "SelectTime"
#define MI_MSSQL_RESTORETIME   "RestoreSeq"

#define MI_MSSQL_ERROR         "Execute error"
#define MI_MSSQL_ERROR_SQL     "SQL"
#define MI_MSSQL_ERROR_MESSAGE "Message"
#define MI_MSSQL_ERROR_STATE   "State"

#define MI_MSSQL_FULL_BAK       0
#define MI_MSSQL_LOG_BAK        1
#define MI_MSSQL_LOG_BAK2       2
#define MI_MSSQL_LOG_BAK3       3

#define MI_FROM_AGENT_EVENT     "Restore_From_Agent"
#define MI_TO_AGENT_EVENT       "Restore_To_Agent"
#define MI_MSSQL_SM_NAME        "Restore_ShareMemory"
#define MI_RECV_JOB_FROM_AGENT  "Recv_Job_From_Agent"

#define MI_MSSQL_BAK_SYN       "syn"
#define MI_MSSQL_BAK_INC       "inc"

#define MI_MSSQL_WORKING        0
#define MI_MSSQL_SWITCHING      1
#define MI_MSSQL_STOPPED        2
#define MI_MSSQL2_MAX_IO_SIZE   502400000
#define MI_MSSQL2_BLOCK_SIZE    5242880
#define MI_MSSQL2_LICENSE_TIME  1296000 /* ������15�� */

#define MI_MSSQL_LOGDATAPATH    "logdata"

static char *pszSuffix[6] = {"syn", "full.bak", "log.bak", "log2.bak", "log3.bak", "tran"};

/* 
 * �������ʹ�õĹ����ڴ湲��3��2048��С���ڴ档�μ�sqlproxy.h.
 * ��������������Web���湲ͬʹ�õ��ǵ�һ�鹲���ڴ棬��СΪ2048B.
 * ��һ��鹲���ڴ棬�����������:
 * 0 -- 7B, ǰ4���ֽڴ�����ݼ��ص�Ƶ�ʣ�����Ϊ��λ��Ĭ��Ϊ0��ʵʱ���ء����ĸ��ֽڱ�ʾ������ݿ������
 * 8 -- 1031B, ������ݿ��б����ڿ��Ƹ������ݿ��״̬��
 * 1032 -- 2047B, ����ݴ�ԭ�����ò���.
 * �ڶ���:
 * 2048 -- 4095B����Ų�ѯ����״̬����Ҫ�ɲ�ѯ����д. ������̶�״̬.
 * 2048B����ʾ��ѯ�߳�����. 2049��2051������
 * 2052��2055��ʾ��ѯ�����ӽ���ID.
 * ÿ����ѯ�߳�ռ��8�ֽڡ���һ�ֽڱ�ʾ״̬. 
 *
 * ������:
 * 4096 -- 6143B����ž���װ��״̬����Ҫ�ɽ��̽��̵�װ���߳�д����ѯ�����״̬.
 * 4096B����ʾװ���߳������� 4097��4099 ����.
 * ÿ��װ���߳�ռ��4�ֽڡ���һ�ֽڱ�ʾ״̬. 
 *
 */
#define MI_MSSQL_SM_SIZE           2048         /* �����ڴ������飬ÿ��2048B.  */
#define MI_MSSQL_SM_DB_SIZE        1024         /* DBName max length */
#define MI_MSSQL_BUFF_SIZE         1024*1024*4  /* �ݴ�ʱһ�δ��������ļ����ݵĴ�С���ô�СΪȫ��ʱ�����С*/

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
    unsigned long h; \
    unsigned long l; \
    PRUint64   u; \
    /* high 32-bit */ \
    h = (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3]; \
    /* low 32-bit */ \
    l = (p[4] << 24) + (p[5] << 16) + (p[6] << 8) + p[7]; \
    LL_UI2L(u, h); \
    LL_SHL(n, u, 32); \
    LL_UI2L(u, l); \
    LL_ADD(n, n, u); \
}

typedef struct MSSQL_MSTL {

	unsigned long nFlag;
	unsigned long nLSN;
	unsigned long nSectionSize;
	PRInt64 nOffset;
	PRInt64 nSectionBegin;
	PRInt16 nFileNo;

	struct MSSQL_MSTL *pNext;
} MiSqlMstlSt;
typedef struct _MI_MSSQL2_DB_NAME_ST {
    char szBDName[MI_MSSQL_SM_DB_SIZE];
    int  nStatus;                                    /* 0Ϊ��ԭ״̬��2��ͣ״̬��1Ϊ�м�״̬ */
    _MI_MSSQL2_DB_NAME_ST *pNext;
} MiMssql2DBNameSt;

/* define transaction struct */
typedef struct _MI_MSSQL_IO_ST {
    ap_pool *pPool;
    char        szInstance[MAX_PATH];
    char        szDBName[MAX_PATH];
} MiMSSqlIOSt;

#define MI_MISSQL_BAK_ST_SIZE  (280 + 8)
typedef struct _MI_MSSQL_BAK_ST_{
    ap_pool *pPool;
    ap_pool *pDataPool;             //�����ݵ��ڴ�
    char     szBakName[MAX_PATH];   //���ݼ��ļ�·��
    void    *pBakAddr;              //���ݼ��ڴ��ַ
    PRUint64 nBakSize;              //���ݼ���С
    PRInt64  nLastSeq;              //���� SEQ ��
    PRInt64  nNextOffset;           //���ݼ���ϢҪд�������ļ���
    PRInt64  nLastOffset;           //���ݼ���ϢҪд�������ļ���
    _MI_MSSQL_BAK_ST_    *pNext;
}MiMSSqlBakSt;

#define MI_MISSQL_BAK_SLOT_SIZE  (313 + 3)
typedef struct _MI_MSSQL_BAK_SLOT_{
    char    szInstName[128];
    char    szDBName[128];
    bool    bActived;       /* ��Ӧ�߳���ָ�� */
    bool    bUsed;
    bool    bSyned;
    bool    bRestartThr;
    bool    bStatus;        /* ��ͣ��ԭ��ʶ��true ������ͣ��ԭ��false ���������ԭ */
    PRInt32 nIncStatus;     /* ����ִ��״̬ */
    PRInt32 nSynStatus;     /* ȫ��ִ��״̬ */
    PRInt32 nIncCnt;        /* �ڴ����Ѿ�����ƴװ�õı��ݼ����� */
    PRLock *pLck;           /* ������ */
    PRCondVar *pCondVar;
    MiMSSqlBakSt *pBakHead; /* ��Ӧ���ݼ� */
    MiMSSqlBakSt *pBakTail;
    MiSqlMstlSt  *pMstl;
    char    szClientIp[16];
    bool bWillCreateNewBak; //ȫ������4��bak�ļ�ǰ��Ϊtrue����ֹƴװ�߳���ɾ���������ɵ�bak�ļ�
    _MI_MSSQL_BAK_SLOT_ *pNext;
}MiMSSqlBakSlot;

#define WAVETOP_MSSQL_MSCI         "MSCI"
#define WAVETOP_MSSQL_MSTL         "MSTL"
#define WAVETOP_MSSQL_MSLS         "MSLS"
#define WAVETOP_MSSQL_MQTL         "MQTL"
#define WAVETOP_MSSQL_SPAD         "SPAD"
#define WAVETOP_MSSQL_APAD         "APAD"
#define WAVETOP_MSSQL_SFIN         "SFIN"

#define WAVETOP_MSSQL_HEAD_SIZE    3584
#define WAVETOP_MSSQL_MSCI_SIZE    2048
#define WAVETOP_MSSQL_MSTL_SIZE    1024
#define WAVETOP_MSSQL_SPAD_SIZE    1024
#define WAVETOP_MSSQL_MSLS_SIZE    2048
#define WAVETOP_MSSQL_END_SIZE     4096

#define WAVETOP_MSSQL_2008_MSCI_SIZE    3072
#define WAVETOP_MSSQL_2008_MSLS_SIZE    3072

#define WAVETOP_MSSQL_2012_MSCI_SIZE    4096
#define WAVETOP_MSSQL_2012_MSLS_SIZE    4096

/* 3584 + 2048 + 1024 + 1024 + 2048 + 4096 + 1024(pMSTL_SPAD) + 1024(m_pTEMP) */
#define WAVETOP_MSSQL_2005_SIZENUM      15872
/* 3584 + 3072 + 1024 + 1024 + 3072 + 4096 + 1024(pMSTL_SPAD) + 1024(m_pTEMP) */
#define WAVETOP_MSSQL_2008_SIZENUM      17920
/* 3584 + 4096 + 1024 + 1024 + 4096 + 4096 + 1024(pMSTL_SPAD) + 1024(m_pTEMP) */
#define WAVETOP_MSSQL_2012_SIZENUM      19968

#define WAVETOP_MSSQL_2000_VERSION      8
#define WAVETOP_MSSQL_2005_VERSION      9
#define WAVETOP_MSSQL_2008_VERSION      10 
#define WAVETOP_MSSQL_2012_VERSION      11 

#define WAVETOP_MSSQL_FIRST_MSTL        1
#define WAVETOP_MSSQL_BACKUPSET_FILTER  2

#define WAVETOP_MSSQL_LOGDATA_BUF       8192



typedef struct MSSQL_BACPUPSET_INFO {

	unsigned char BeginLsn[12];
	unsigned char EndLsn[12];
	unsigned long nCurrSize;
	unsigned long nSequnce;
	unsigned long nMsciSize;
	unsigned long nMaxLsn;
	char szBakSetLsn[24];
	PRInt64 nLdfSize;
	PRInt64 nPageSize;
	PRInt64 nTotalSize;
	PRInt64 nCurrBegin;
	PRInt64 nLastOffset;
	PRInt64 nNextOffset;
	int nDatabaseVer;  

} MiSqlBackupSetInfoSt;

typedef struct MSSQL_BACPUPSET_MODULE {

	unsigned char *pHEAD;
	unsigned char *pMSCI;
	unsigned char *pMSTL;
	unsigned char *pMSLS;
	unsigned char *pSPAD;
	unsigned char *pMSTL_SPAD;
	unsigned char *pEND;
	unsigned char *pModuleMem;

} MiSqlBackupSetModuleSt;

typedef struct BACKUPSET {
	WTBUF *pIOQueue;
	PRInt64 nLastOffset;
	PRInt64 nCurrentSeq;
	unsigned long nFileSize;
	unsigned long nIOCount;
}BackUpSetSt;

typedef struct MSSQL_BACPUPLDF_INFO {

	/* ldf �ļ���С */
	PRInt64 nLDFSize;

	/* nLDFSize �ڱ��ݼ��е�λ�ã��� MSCI �У� */
	PRInt64 nLDFOff;

	/* �ļ���� */
	PRInt32 nNumb;

	MSSQL_BACPUPLDF_INFO *pNext;

} MiSqlBackupldfInfo;

/* ƴװ������־���ݼ��� */
class CAssembleTransLogBackupSet {

public:

	CAssembleTransLogBackupSet(char *pszBackUpFile, int nDatabaseVer);
	~CAssembleTransLogBackupSet();
	int Init(bool bFilterIO, unsigned long BackupSize);
	int Assembly(ap_pool *pPool, BackUpSetSt *pData, unsigned char **OutPut, unsigned long *nOutPutSize);
	void GetCurrLSN(char *pszCurLSN);
	void GetMstlInfo(MiSqlMstlSt **pMstl);

private:


	int Analyze();
	int GetLdfInfo();
	int CheckData(unsigned char *pBuff, char *pKey);
	int SetCurrMstlInfo(PRInt64 nOffset, PRInt16 nFileID);
	bool CheckMstlInfo(PRInt64 nOffset, PRInt16 nFileID);
	void SetFileSize(PRInt64 nFileSize, PRInt16 nFileID);
	void AssembleHead(unsigned char *InPut);
	void AssembleAllMstl(unsigned char *InPut);
	int AssembleMstl(unsigned char *InPut, BackUpSetSt *pData);
	void AssembleMsci(unsigned char *InPut);
	void AssembleMsls(unsigned char *InPut);
	void AssembleEnd(unsigned char *InPut);
	void StrReplace(unsigned char *pBuff);
	PRInt64 GetSum(unsigned char *pBuff, int nSize);
	unsigned short CheckSum_XOR(unsigned short *pBuffer, int nSize);

private:

	char *m_pszBackUpFile;
	MiSqlBackupSetModuleSt m_BackupSetModule;
	MiSqlBackupSetInfoSt m_BackupSetInfo;
	MiSqlBackupldfInfo *m_BackupLdfInfo;
	MiSqlMstlSt *m_MstlInfo;
	unsigned char *m_pTEMP;
	unsigned char *m_pMSCI;
	bool m_bFilterIO;
	bool m_bBlackBackup;

	/* for test */
	unsigned long m_BackupSetSize;
	PRInt64 m_nLDFSize;
};


int Init(char *pszInstance,char *pszDBName,MiLogHandle *hLog,char* pszLogPath);


int BkSqlStartService(char* pszServName);

int BkSqlStopService(char *pszServerName);

int MiSqlGetIO(unsigned char **pszData, PRInt64 *pnSize,
	PRUint64 *pnCurrSeq, MiLogHandle LogHandle,
	unsigned char *pBuf,  PRInt64 *pnMaxSize);


int GetIOQueue(ap_pool *pNewPool, MiLogHandle LogHandle, unsigned char *pBuf, 
	BackUpSetSt *pTemp, MiMSSqlBakSlot *pBakSlotGet, char *pszCurrLSN);