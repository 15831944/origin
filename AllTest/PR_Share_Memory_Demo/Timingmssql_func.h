#ifndef _TIMINGMSSQL_FUN_H_
#define _TIMINGMSSQL_FUN_H_

#include "public.h"




typedef struct _ARGES_ST {
    /* Mirror Way: file or MSSQL or Oracle */
    int   nMirrorWay;
    char *pszAgentIP;
    char *pszServer;
    char *pszSrcDB;
    char *pszPullOutOpt;
    int   nfileMode;
    char *pszvirtdir;
    char *pszCliphydir;
    char *pszExcfilmat;
    char *pszFileTemp;
    char *pszPullOutContent;
    char *pszPort;
    char *pszPullOutParam;
    char *pszOcacleUser;
    char *pszOraclePwd;
    } TASKARGS;

/* �����ڴ�ͷ�ṹ */
typedef struct _BACKUP6_PROC_MASTER_ {
    int nProcNum;
    int nThreadNum;
    int nMasterPid;
    int nStatus;
    } ShareMasterSt;

typedef struct _BACKUP6_THREAD_TASK_ST_ {
    /* �ӽ��̵��߳���������״̬ */
    short nTaskStatus;
    /* �����̹�����ӽ���socket */
    int nChildFD;
    /* ������socket */
    int nParentFd;

    char szModulePath[512];
    /* �û��� */
    char szUser[32];
    /* ��������״̬ */
    int nStatus;
    /*  ���ݿ�������� */
    int nStartDBFlag;
    /* ���ݿ��С */
    PRInt64 nDBSize;
    /* �����ļ���С */
    PRInt64 nRecvSize;
    /* ��������ʱ�� */
    unsigned long nTime;
    /* ��¼ʵʱ�ָ������SEQ */
    PRInt64 nSeq;
    /* ����LOGDATA��� */
    int nCleanFlag;
#ifdef WIN32
    /* ����socket */
    WSAPROTOCOL_INFOA ProtocolInfo;
#endif

    } ShareThreadTaskSt;

/* �ӽ��̹����ڴ�ṹ */
typedef struct _BACKUP6_PROC_SHARE_ST_ {
    /* �ӽ���pid */
    int index;
    int nChildPid;
    int nStatus;
#ifdef WIN32
    HANDLE hShutdown;
    HANDLE hProcess;
#endif //WIN32

    } ShareMemorySt;

/* �����ڴ�ͷ�ϻ�ȡ�̸߳��� */
int BkGetThreadNum(int *pnThrdCount, char *pszSharName);

int BKSetDBStart(char *pszModPath, char *pszUser, int nStatus, char *pszSharName);

int test(char *pszSharName);

int UinSendRecoverProtocol(const char * pszObject, const char *pszInstance, 
    const char* pszServer, char *szDbName, int nPort, 
    char *pszModid, char *pszMsg, PRInt32 nSize);






#endif
