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

/* 共享内存头结构 */
typedef struct _BACKUP6_PROC_MASTER_ {
    int nProcNum;
    int nThreadNum;
    int nMasterPid;
    int nStatus;
    } ShareMasterSt;

typedef struct _BACKUP6_THREAD_TASK_ST_ {
    /* 子进程的线程任务运行状态 */
    short nTaskStatus;
    /* 父进程共享给子进程socket */
    int nChildFD;
    /* 父进程socket */
    int nParentFd;

    char szModulePath[512];
    /* 用户名 */
    char szUser[32];
    /* 任务运行状态 */
    int nStatus;
    /*  数据库启动标记 */
    int nStartDBFlag;
    /* 数据库大小 */
    PRInt64 nDBSize;
    /* 接收文件大小 */
    PRInt64 nRecvSize;
    /* 任务运行时间 */
    unsigned long nTime;
    /* 记录实时恢复任务的SEQ */
    PRInt64 nSeq;
    /* 清理LOGDATA标记 */
    int nCleanFlag;
#ifdef WIN32
    /* 传递socket */
    WSAPROTOCOL_INFOA ProtocolInfo;
#endif

    } ShareThreadTaskSt;

/* 子进程共享内存结构 */
typedef struct _BACKUP6_PROC_SHARE_ST_ {
    /* 子进程pid */
    int index;
    int nChildPid;
    int nStatus;
#ifdef WIN32
    HANDLE hShutdown;
    HANDLE hProcess;
#endif //WIN32

    } ShareMemorySt;

/* 共享内存头上获取线程个数 */
int BkGetThreadNum(int *pnThrdCount, char *pszSharName);

int BKSetDBStart(char *pszModPath, char *pszUser, int nStatus, char *pszSharName);

int test(char *pszSharName);

int UinSendRecoverProtocol(const char * pszObject, const char *pszInstance, 
    const char* pszServer, char *szDbName, int nPort, 
    char *pszModid, char *pszMsg, PRInt32 nSize);






#endif
