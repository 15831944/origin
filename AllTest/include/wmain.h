/** =============================================================================
 ** Copyright (c) 2003 WaveTop Information Corp. All rights reserved.
 **
 ** The backup system
 **
 ** =============================================================================
 */

#ifndef __BACKUP_MAIN_H_
#define __BACKUP_MAIN_H_ 1

#ifdef WIN32
#include <windows.h>
#include <winsock2.h>
#else
#include <unistd.h>
#endif

#include "nspr.h"
#include "wconfig.h"
#include "backup_ver.h"
#include "backup_proto.h"
#include "backup_base.h"
//#include "configfile.h"
#include "wsnprintf.h"
#include "wfunction.h"
#include "wuser.h"
#include "wca.h"
#include "license.h"
//#include "reportapi.h"
#include "wnetfd.h"
#include "servermodule.h"
#include "dbtable.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

#define BACKUP_SERVER_VERSION_NUMBER    1100
#define BACKUP_SERVER_VERSION           WAVETOP_BACKUP_PRODUCT_VERSION
#define BACKUP_CONFIG_FILE              "wbackup.conf"
#define BACKUP_TASK_ID_CONF             "taskid.conf"
#define BACKUP_ALARM_CONF               "alarm.conf"
#define BACKUP_DOMAINCONFIG_FILE        "domainlogin.conf"
#define BACKUP_FIRST_VERSION            1
#define BACKUP_NO_VERSION               (BACKUP_FIRST_VERSION - 1)
#define BACKUP_SERVER_BACKUP6_FILE      "backupserv6.conf"

/* The backup server configuration definition */
#define BACKUP_LOG_FILE                 "logs/wbackup.log"
#define BACKUP_UPDATE_LOG_FILE          "logs/wupdate.log"
#define BACKUP_PID_FILE                 "logs/backupserv.pid"
#define BACKUP_DATA_DIR                 "data"
#define BACKUP_WEBUI_DIR                "webui"
#define BACKUP_LICENSE_FILE             "license.conf"
#define BACKUP_INDEX_STORE_BACKUP_DIR   "idx_str_backup_dir"
#define BACKUP_LOG_DATA_FILE            "logdata"
#define BACKUP_DB_TEMP                  "temp"
#define BACKUP_MODULE_SECTION           "Mode_"
#define BACKUP_MODULE_OBJECT            "Object"
#define BACKUP_DRIVE_DIR				"OS_STORE"

/* Flag for the first file */
#define WAVETOP_OPTION_FIRST_FILE           (1<<31)  //首个文件标志
/* 区分策略分发和备份作业监控 */
#define WAVETOP_DISTINGUISH_POLICY_BACKUP   (1<<30)
/* Default space search rate */
#define BACKUP_SPACE_SEARCH_RATE    0.75

/* The max number of the directoies of the user */
#define BACKUP_MAX_DIRECTORY           32
/* The cache seconds from this time to the next flush */
#define BACKUP_CACHE_FLUSH_SECONDS     60
/* The cache idle seconds. over the number which will be closed */
#define BACKUP_CACHE_IDLE_SECONDS      300

/* The number of IP address filter */
#define WAVETOP_SERVER_RULE_NUM        32

/* 主进程创建的最多子进程数量 */
#define WAVETOP_BACKUP6_MAX_PROCESS          256
#define WAVETOP_BACKUP6_MAX_THREAD           64
#define WAVETOP_BACKUP6_DEFAULE_PROCESS      4
#define WAVETOP_CHAILD_PROC_DEFAULE_THREAD   16	
#define WAVETOP_SERVER_FD_NUMBER             8
#define WAVETOP_SHARE_MEMORY             "_wavetop_share_d_memory_"
#define WAVETOP_PROC_MUTEX_D             "_wavetop_proc_d_mutex_b6_"
#define WAVETOP_REPORT_MUTEX_D           "_wavetop_report_d_mutex_b6_"

/* 规则
 * 所有模块通用的状态1-100
 * modfs:       101--199
 * modsql:      201--299
 * modoracle:   301--399
 * modsybase:   401--499
 * MSSQL定时备份模块:  501--510
 * ....
*/
////////////////////////////
/* 备份状态 */
#define WAVETOP_BACKUP6_WAIT_BACKUP_TASK       101   /* 正在排队等待备份任务开始 */
#define WAVETOP_BACKUP6_INIT_TASK              102   /* 正在初始化任务 */
#define WAVETOP_BACKUP6_RECV_IO_DATA           103   /* 客户端正在传输IO，这里写的是服务器的状态，表示服务器在接收IO */
#define WAVETOP_BACKUP6_RECV_SYNC_DATA         104   /* 正在给服务器发送初始数据文件 */
#define WAVETOP_BACKUP6_WAIT_CHECKPOINT        105   /* 正在等待checkpoint-- 正在检查一致性点 */
#define WAVETOP_BACKUP6_UNITE_DATA             106   /* 正在合并IO与全量文件--正在优化数据 */
#define WAVETOP_BACKUP6_WRITE_STORE            107   /* 正在写ARC文件--正在写存储 */
#define WAVETOP_BACKUP6_RECV_INC_DATA          108   /* 正在执行文件增量备份 */
#define WAVETOP_BACKUP6_RMAN_EXPORTING		   109	 /* oralog备份导出rman数据 */
#define WAVETOP_BACKUP6_CLOUD_SNAPSHOT		   110	 /* 创建虚拟机快照 */
#define WAVETOP_BACKUP6_CLOUD_DEL_SNAPSHOT	   111

/* 还原状态 */
#define WAVETOP_BACKUP6_WAIT_RESTORE_TASK      201   /* 正在排队等待手动恢复任务开始 */
#define WAVETOP_BACKUP6_PREPRARE_RESTORE_TASK  202   /* 正在初始化还原任务 */
#define WAVETOP_BACKUP6_SEND_SYNC_DATA         203   /* 正在给客户端发送初始数据文件 */
#define WAVETOP_BACKUP6_SEND_IO_DATA           204   /* 正在给客户端发送IO数据 */
#define WAVETOP_BACKUP6_RMAN_IMPORTING		   205   /* RMAN 正在执行导入操作 */
#define WAVETOP_BACKUP6_REGISTER_ARCHIVE	   206	 /* 正在注册归档日志 */
#define WAVETOP_BACKUP6_STARTING_DB			   207	 /* 正在启动数据库 */

/* 实时恢复-服务器 状态 */
#define WAVETOP_BACKUP6_INIT_RECOVER_TASK      301   /* 服务器正在初始化恢复任务 */
#define WAVETOP_BACKUP6_S_RECOVER_FULL_DATA    302   /* 正在实时同步初始的完整数据给恢复端 */
#define WAVETOP_BACKUP6_S_RECOVER_IO_DATA      303   /* 正在实时同步最新的IO数据给恢复端 */
#define WAVETOP_BACKUP6_S_RECOVER_END_IODATA   304   /* 到了最后一个IO块,就一直等待最新IO块的状态 */
#define WAVETOP_BACKUP6_START_DB               310   /* BKCTRL写的共享内存检测到的数据库启动的状态 */
#define WAVETOP_BACKUP6_AUTO_START_DB          1001  /* 配置接管后，若检测到无法连接到主服务，则自动启动数据库的标记  */
#define WAVETOP_BACKUP6__START_DB_SUCCESS      1002  /* 配置接管后，若检测到无法连接到主服务，自动启动数据库成功的标记  */

/* 实时恢复-客户端 状态 */
#define WAVETOP_BACKUP6_RECOVER_RUNNING        401   /* 恢复任务正在执行 */
#define WAVETOP_BACKUP6_RECOVER_END            402   /* 恢复任务已结束 */
#define WAVETOP_BACKUP6_RECOVER_DB_START_BEGIN 403   /* 恢复启动数据库开始 */

/* 报警-服务器端 状态 */
#define WAVETOP_BACKUP6_ALARM_SERVER_RUNNING   501   /* 报警任务正在运行 */

/* 子进程线程任务运行状态 */
#define WAVETOP_BACKUP6_THREAD_TASK_IDLE   0
#define WAVETOP_BACKUP6_THREAD_TASK_BUSY   2
#define WAVETOP_BACKUP6_THREAD_TASK_END    3 
#define WAVETOP_BACKUP6_THREAD_TASK_BEGIN  1

/* 子进程共享内存插槽状态 */
#define WAVETOP_BACKUP6_CHILD_PROC_SLOT_IDLE    0
#define WAVETOP_BACKUP6_CHILD_PROC_SLOT_INIT    1  
#define WAVETOP_BACKUP6_CHILD_PROC_SLOT_BUSY    2  

/*
 * Special Backpserv error codes. These are basically used
 *  in servcore.cpp so we can keep track of various errors.
 *
 *   APEXIT_OK:
 *     A normal exit
 *   APEXIT_INIT:
 *     A fatal error arising during the server's init sequence
 *   APEXIT_CHILDINIT:
 *     The child died during it's init sequence
 *   APEXIT_CHILDFATAL:
 *     A fatal error, resulting in the whole server aborting.
 *     If a child exits with this error, the parent process
 *     considers this a server-wide fatal error and aborts.
 *                 
 */
#define APEXIT_OK       0x0
#define APEXIT_INIT     0x2
#define APEXIT_CHILDINIT    0x3
#define APEXIT_CHILDFATAL   0xf
typedef struct CharNodeEx {
    char *pszStr;
    PRInt64 nFileSize;
    int nFlat;
    CharNodeEx *next;
} CharNodeEx;

/* 共享内存头结构 */
typedef struct _BACKUP6_PROC_MASTER_ {
    int nProcNum;
    int nThreadNum;
    int nMasterPid;
	int nStatus;
} ShareMasterSt;
union DATA {
	PRInt8 n8;
	PRInt16 n16;
	PRInt32 n32;
	PRInt64 n64;
	char *pszStr;
};
typedef struct _REPORT_INFO {
	/* 所要替换的 key */
	char *pszFilter;
	/* 所要替换的 value */
	union DATA data;
	/* value 类型 */
	unsigned long datatype;
	struct _REPORT_INFO *next;
} ReportInfoSt;
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
    PRInt32 nTime;
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

/* The listen daemon thread job */
struct Jobst {
    struct Jobst *next;
    PRFileDesc *netfd;
};

struct ThreadSlotst {
    PRThread *thread;
    int flag;
};

/* The cache system */
typedef struct CacheNodeSt {
    char szUser[256];
    PRLock *pMutex;
    int nClose;
    int nIdle;
    int nInUse;
    int nIdleTime;
    int nUseTime;
    void *pData;
    int (*pFlushFunc) (void *pData);
    int (*pCloseFunc) (void *pData);
} CacheNodeSt;

/* The report slotst */
typedef struct ReportSlotst {
    pool *pPool;
    int  nFlag;
    int nCount;
    unsigned long nStartTime;
    unsigned long nTaskId;
    ReportInfoSt *pStReport;
    struct ReportSlotst *next;
} ReportSlotst;

typedef struct _TASK_INFO_ST {
    int nFlag;
    char szUserName[64];
    char szUserIP[32];

    /* task type : backup restore delete share */
    int nTaskType;

    /* object type : file, mssql, oracle */
    int nObjectType;

    /* task status : wait start run end */
    int nTaskStatus; 
    
    /* task exit code : succeed failed */
    int nTaskExitCode;
    unsigned long nTaskId;
    unsigned long nStartTime;
    unsigned long nEndTime;
    char *pReserve;

    /* The schedule name */
    char szSchName[64];

    /* The current bytes of transferring */
    PRInt64 nCurDataBytes;

    /* The report of this task */
    ReportSlotst *pReport;

    _TASK_INFO_ST *next;
} TaskInfoSt;

typedef struct _TASK_MONITOR_ST {
    PRLock *pCtrlLock;
    unsigned long nCurTaskId;
    unsigned long nLastPoint;
    unsigned long nTotalTasks;
    unsigned long nMaxTasks;
    unsigned long nTaskTimeOut;
    TaskInfoSt **pTaskList;
} TaskMonitorSt;

/* 用户可用磁盘空间链表 */
typedef struct _DISK_SPACEINFO_ST {
    unsigned long nFreeDiskSpace;
    unsigned long nTotalBackupSize;
    int      nFlag;
    char     szDevName[3];
    char     szUserName[64];
    char     szReserve[8];
    _DISK_SPACEINFO_ST *next;
} DiskSpaceInfoSt;

/* 备用设备列表 */
typedef struct _RESERVE_DEVICEINFO_ST {
    char szDevName[512];
    unsigned int uDevType;
    unsigned long nFreeSpace;
    int nFlag;
    _RESERVE_DEVICEINFO_ST *pNext;
}ReserveDeviceInfoSt;

/* 用户设备列表 */
typedef struct _USER_RESERVE_DEVICEINFO_ST {
    char szUserName[64];
    char szDevName[512];
    _USER_RESERVE_DEVICEINFO_ST *pNext;
} UserReserveDeviceInfoSt;

/* oracle 监控信息 */
typedef struct _BK_ORA_MONITOR_ST {
    /* monitor id */
    PRInt64 nTaskId;
    /* user name */
    char szTaskUser[256];
    /* total size */
    unsigned long nDataLen;
    /* database name */
    char szDbName[256];
    /* file name */
    char szFileName[1024];
    /* index num */
    PRUint64 nSeqNum;
    /* recv time */
    unsigned long nRecvTime;
    
    struct _BK_ORA_MONITOR_ST *pNext;
} OraMonitorInfoSt;


typedef struct _BACKUP6_MOD_NODE_ {
    char szKey[64];
    char szValue[2048];
    _BACKUP6_MOD_NODE_ *pNext;
} ModNode;

/* 配置文件，模块节点属性 */
typedef struct _BACKUP6_CONF_MOD_ST_ {
    char szModName[64];
    ModNode *pModNode;
    _BACKUP6_CONF_MOD_ST_ *pNext;
} ConfNode;



/* The global configuration information */
struct global_conf_st {
    pool *globalMem;

    /* The parent or child process */
    int childProcess;
    char *childKillEventName;

#ifdef WIN32
    /* The system quit event */
    HANDLE processShutdownEvent;

    /* The child quit event */
    HANDLE childKillEvent;

    /* For the Impersonating */
    char *pszImpersonateUser;
    char *pszImpersonatePasswd;
    HANDLE hAdminToken;

    char *pszDomainName;
    char *pszLocalName;

	/* license信息  */
	char *pszSerNumber;
	char *pszLicense;
    int nServiceFunc;
#else
    char *processFile;
	char *pszSerNumber;
    char *pszLicense;
    int nDemoUse;
#endif
    int cZeroIsDemoVer;
    /* One process for debuging */
    int oneProcess;

    char *serverdir;

    /* The configuration file */
    char *confile;
    char *initdir;
    char *logfile;
    char *updatedir;
    char *updatelogfile;
    char *pidfile;
    char *datadir;
    char *backupdir;
    char *idxstrbkdir;

    int loglevel;
    int reptlevel;

    /* The base bucket and overflow bucket. */
    int nBucketSize;
    int nBaseBucketNum;
    int nOverBucketNum;

    /* The flag of search a suitable node. */
    int nSearchRoomFlag;
    
    /* log rotate size */
    int nRotateLogRows;
    int port;
    int AgentPort;

	/* 区分实时备份或ACDP */
	int nBackupType;

    /* WSAduplicatSocket */
    int nFd;
    
    int serverfds[WAVETOP_SERVER_FD_NUMBER];
    /* 有效的socket数量 */
    int fdcount;
    
    /* 共享内存句柄 */
    PRSharedMemory *pShare;
	char *pszShareMemHead;

    /* 共享内存的模块编号 */
    int nModIdx;
    int ftpserverFd;
    
    /* The listen backlog */
    int backlog;

    int curThreadNum;
    int idleThreadNum;

    /* the handle request (working) thread
     * the curThreadNum is the current thread count
     */
    struct ThreadSlotst *threadSlots;
    
    char *wdeltaCmd;
    char *user;
    char *group;
    char *servername;
    //Iodaemon保存在本地模式还是直发模式.直发模式:1,保存本地:0
	int nIodaemonFlag;

    /* The working queue */
    struct Jobst *jobHead;
    struct Jobst *jobTail;
    PRLock *jobLock;
    PRLock *fulljobLock;
    PRLock *resjobLock;
    PRCondVar *jobVar;

    /* The report table info */
    ReportSlotst *pReportSlots;
    PRLock *pReportLock;

    /* the task monitor info */
    TaskMonitorSt *pMonitor; 

    /* The quitting flag */
    int quit;

    /* User slot */
    UserInfoSt *pUserInfoList;
    int nCacheSlotCount;
    CacheNodeSt *pCacheSlots;
    PRLock *pUserLock;
    int nDefaultIdle;
    int nDefaultFlush;
    int nIdleConnected;

    /* 流量限制大小 */
    int nFluxSize;

    /* FTP 备份端口 */
    int nFTPPort;

    /* 用户可用磁盘空间链表 */
    DiskSpaceInfoSt *pDiskSpaceInfoList[26];
    /* 用户备用设备列表 */
    UserReserveDeviceInfoSt *pUserReserveDeviceInfo;
    unsigned long nFreeSpace;

    /* the admin profile */
    char *pszEmail;
    char *pszAlertSubject;
    char *pszAlertMessage;
    char *pszSMTPServer;
    char *pszSMTPLoginUserName;
    char *pszSMTPLoginPasswd;
    int nSMTPPort;
    int nSMTPLogin;
    int nWarnFreeDiskSpace;
    int nTermFreeDiskSpace;
    int nTotalSendMailCount;
    int nCurrentSendMailCount;

    /* file name filter */
    CharNode *pFilter;

    /* The server socket options */
    int nSendBufsize;
    int nRecvBufsize;
    int nKeepalive;
    char *pszServaddr;
    char *pszServaddr2;
    char *pszServaddr3;
    char *pszServaddr4;

    /* The license information */
    LicenseInfoSt stLicenseInfo;
    char *pszLicenseFile;

    int nDemoUsedSeconds;
    int nDemoSeconds;
    char *pszDemofile;

    /* The html temp file path */
    char *pszReptHtmlPath;

    /* The XSL and XML templete file path */
    char *pszReptXSLXMLPath;

    /* The XML file to save path */
    char *pszReptXMLSavePath;

    /* The Ca Function Use Handle */
    WavetopCAModule *pCaFunHandle;
    char *pszCaOption;
    unsigned long nLibHandle;

    /* The MAC address serial number */
    char *pszMACAddress;

    /* Get The Space Search rate */
    float fSpaceSecrchRate;

    /* Policy files lock */
    PRLock *pPolicyLock;
    const char *pszPermissionUser;

    /* XXX: Maximum number of network cards (taolindun) */
    int nMaxNetworkCards;

    /* The max number of the increasing version */
    int nMaxNumIncreasingVer;

    /* The switch of increasingstore and completestore */
    int nIncreasingStoreFlag;

    /* Interval time for restart the child process */
    unsigned long nInterval;

    /**
     * IP address filtering from mod_access.c in apache
     * Save the ip parse result.
     */
    char *pszAllowFrom;
    allowdeny *ppAP[WAVETOP_SERVER_RULE_NUM];

    /* server run mode */
    char **pszModules;

    /* limit Archive file size */
    int nZeroIsNotLimitArcSize;
    int nArcLimitSize;
    int nLogdataSaveTime;

    /* defgroup modules define */
    BackupServerModulesSlot ServerModules[BACKUP_SERVER_MAX_MODULES];

    /* 初始创建子进程的数量，从配置文件读取 */
    int nProcessCount;
	/* 子进程线程数量 */
	int nChildThreadCount;
    
    /* oracle logdata */
    char *pszLogdata;
    /* oracle temp */
    char *pszTemp;

    /* monitor id */
    unsigned long nMonitorId;
    /* increase report total size */
    unsigned long nTotalSize;
    /* monitor info list */
    OraMonitorInfoSt *pOraMonitor;

    /* current mode */
    int nCurrentId;

    int nDbModCount;
    char *pIoLogdata;
    ConfNode *pConModInfo;
    
    /* 自动接管信息 */
    char *pszSernetName;
    char *pszSernetName1;

	int nCheckFlag;
	int nHostModeCount;
	HostConfNode *pHostHead;
#ifdef WIN32
	/* 共享内存读写互斥锁 */
	HANDLE hShareMutex;
#else
    int semid;
    int nSendMsgFd;
    int nRecvMsgFd;
#endif 

    /* 客户端还是服务器的标志 */
    int nIsServer;

    /* ORACLE是否执行checkpoint */
    int nIsChkPoint;
    /* 执行checkpoint的间隔时间 */
    PRInt32 nChkTime;

    /* 实时复制时客户端是否等待服务器写文件完成的协议，默认等待:0,不等待:1. */
    int nIsFastSendIO;

    /* 集群配置路径 */
    char szCdFile[MAXPATHLEN];

    char szMMSpath[MAXPATHLEN];
    PRInt32 nThreadCnt;
};

extern struct global_conf_st gGlobalConf;

/* The connection (session) */
typedef struct ConnectionSt {
    pool *mem;

    /* Information about the connection itself */
    sbuff *buff;

    /* remote address */
    struct sockaddr_in remote_addr;
    /* Client's IP address */
    char *remote_ip;

    /* If an authentication check was made,
     * this gets set to the user name.  We assume
     * that there's only one user per connection(!)
     */
    char *user;
    int userid;
    int groupid;
    char *passwd;
    int authway;
    char *userdir;
    long userflags;
    unsigned long nUserLoginTime;

    /* How many times have we used it? */
    int keepalives;

    /* Are we still talking? */
    unsigned aborted:1;
    /* Are we using HTTP Keep-Alive?
     * -1 fatal error, 0 undecided, 1 yes 
     */
    signed int keepalive:2;

    /* Cuurent user info */
    UserInfoSt *pCurUserInfo;
    UserItemSt *pCurUserItem;
} ConnectionSt;

#define POLICY_BUFFER_SIZE  24 * 128

typedef struct _POLICY_BK {
    /* In the store file, row and col position */
    unsigned char szRow[4];
    unsigned char szCol[4];
    unsigned char szBaseRow[4];
    unsigned char szBaseCol[4];

    char szReserve[8];
    _POLICY_BK *next;
} PolicySt;

#ifdef __cplusplus
extern "C" {
#endif

/* The protocol parser */
request_rec  *ReadRequestHeader(ConnectionSt *pConnection);
int           HandleRequest(request_rec *pReq);

/* Write the right response header */
API_EXPORT(int)           WriteResponseHeader(request_rec *pReq);

/* Write the error code response header and flush to the client */
API_EXPORT(int)           WriteErrorResponse(request_rec *pReq);

/* Get the request header field value
 * @[in]
 * pszField -- the request header field
 * @[out]
 * return value - the request header field value; if no
 *                this field, return NULL;
 */
API_EXPORT(const char *) GetHeaderField(request_rec *pReq, const char *pszField);

/* Set the request header field value
 * @[in]
 * pszField -- the field name
 * pszValue -- the field value
 * @[out]
 * return value - the status code;
 */
API_EXPORT(int)           SetHeaderField(request_rec *pReq, const char *pszField, char *pszValue);
API_EXPORT(int)           FlushConnection(ConnectionSt *pConnection);

/* Get the user of this request */
API_EXPORT(const char *) GetRequestUser(request_rec *pReq);

/* Get the glocalconf */
API_EXPORT(void *) GetGlobalConfVar(void);

/* 获取配置文件，模块节点属性 */
API_EXPORT(int) GetGlobalInSection(const char *pszSection, ModNode **pNode);

/* 获取配置文件，根据KEY,VALUE来查询 */
API_EXPORT(int) GetGlobalConfMod(const char *pszKey1, const char *pszValue1,
                                 const char *pszKey2, const char *pszValue2,
                                 const char *pszKey3, const char *pszValue3,
                                 ModNode **pNode);
API_EXPORT(int) GetGlobalConfModByName(const char *pszKey1, const char *pszValue1,
                                       const char *pszKey2, const char *pszValue2,
                                       const char *pszKey3, const char *pszValue3,
                                       const char *pszKey4, const char *pszValue4,
                                 ModNode **pNode);


/* Get the user item of this request */
API_EXPORT(const UserItemSt *) GetRequestUserItem(request_rec *pReq);

/* Get the user id of this request */
API_EXPORT(int)           GetRequestUserId(request_rec *pReq);

/* Get the task monitor id */
API_EXPORT(int)           GetRequestTaskId(request_rec *pReq);

/* Get the user nFlags */
API_EXPORT(unsigned long) GetRequestUserFlag(request_rec *pReq);
                               
/* Get the user login directory of this request */
API_EXPORT(const char *) GetRequestUserDir(request_rec *pReq);
API_EXPORT(const char *) GetUserDir(pool *pool, const char *pszUser);
API_EXPORT(const char *) GetRequestUserIP(request_rec *pReq);

/* Get the user archive files info */
API_EXPORT(UserArchiveFileSt *) GetReqUserArchiveFiles(request_rec *pReq);

API_EXPORT(sbuff *) GetRequestIo(request_rec *pReq);

/* Store algorithm
 * The string hash algorithm
 * @[out]
 * the hash value
 */
long          BackupHash(register char *pszStr, register int nLen);

/* Names the version file name
 * @[in]
 * pszFile - the storing file name
 * nVersion - the version number
 * @[out]
 * pszVersion - the version file name
 */
API_EXPORT(int)           BkNameVersionFile(pool *pPool, 
                                const char *pszFile, 
                                unsigned long nVersion,
                                char **pszVersion);
API_EXPORT(int)           BkNameDeltaFile(pool *pPool,
                              const char *pszFile,
                              char **pszDelta);

/* The storing file name algorithm
 * @[in]
 * pszSource - the source file name
 * @[out]
 * pnDirectory - the storing directory number
 * pszFile - the storing base file name
 */
API_EXPORT(int)           BkNameArchiveAlg(pool *pPool, 
                               const char *pszSource, 
                               char **pszFile);

/* Converts the source file name to the storing file name
 * @[in]
 * pszUserDir, the user directory
 * pszSource, the client source file name
 * @[out]
 * pszStore, the storing file name
 */
API_EXPORT(int)           BkNameStore(request_rec *pReq,
                          const char *pszSource,
                          char **pszResult);


/*
 * Magzine ID to Magzine name
 * @[in]
 * int nMagzineID  : MagzineID
 * int nMagzineSize: magzine name size
 * @[out]
 * char *szMagzine : Magzine name buffer
 */
int            MagzineIDToName(int nMagzineID, char *szMagzine, 
                                           int nMagzineSize);


/*
 * Magzine name to ID
 * @[in]
 * char *szMagzine  : Magzine name buffer
 * @[out]
 * int *pnMagzineID : Magzine ID value pointer
 */
int           MagzineNameToID(int *pnMagzineID, char *szMagzine);

/* The cache system manage APIs */
API_EXPORT(int)           BackupCacheInit();
API_EXPORT(int)           BackupCacheClose();
API_EXPORT(int)           BackupCacheSetIdle(request_rec *pReq, int nIdle);
API_EXPORT(int)           BackupCacheDaemonFunc(int nSeconds);
API_EXPORT(int)           BackupCacheGetByUsername(const char *pszUser,
                                    CacheNodeSt **pNode);
/* Get user cache, now this function not lock the user cache lock */
API_EXPORT(int)           BackupCacheGet(request_rec *pReq, CacheNodeSt **pNode);

/* now not unlock the user cache lock */
API_EXPORT(int)           BackupCacheRelease(request_rec *pReq);
int           BackupCacheDestroy(const char *pszUser);
API_EXPORT(int)           BackupCacheSetUser(request_rec *pReq, CacheNodeSt *pNode);
void          FlushUserInfoList(void);

int          BkGetUserDirSize(char *pszUserName, char *pszDir, PRInt64 *nUserSize);
/*
 * [In]
 *      nIncrease   :   1 - add user free space
 *                      0 - minor user free space
 *
 * [Out]
 *
 */

/* [in] nSize: by MB */
API_EXPORT(int)           CheckDiskSpace(PRInt32 nSize);
API_EXPORT(void)          BackupModifyUserQuota(const char *pszUser, char *pszDir);
API_EXPORT(int)           BackupCheckUserFreeSpaceFromFile(char *pszUserName,
                            PRInt64 nTotalKiloBytes, int nFlag);
API_EXPORT(int)           BackupCheckUserFreeSpaceFromDB(request_rec *pReq,
                                    PRInt64 nTotalKiloBytes);
API_EXPORT(int)           BackupCheckUserFreeSpaceFromPolicyFile(request_rec *pReq,
                                    PRInt64 nTotalKiloBytes);
API_EXPORT(int)           BackupFileCheckUserFreeSpace(request_rec *pReq, 
                                    PRInt64 nTotalFileBytes);

/* The demo version API in the demo.cpp */
int           BackupDemoInit();
int           MiSerialCheckWin32(void);
/* The API called by the cache daemon thread */
int           BackupDemoValidateTime(int nUseSeconds);

/* The report make thread */
void          ListenDaemon(void *pArg);

/* The report listen thread */
void          ReptListenReport(void *pArg);

/* Add new node of super user to global userlist */
int           AddSuperUserToBackup();

#ifdef WIN32
int SpecifyAccessRights(const char *pszFileName, 
                        const char *pszAccountName, 
                        unsigned long nAccessMask);
#endif

/**
 * Write updating log
 */
int          UpdateLogInit(const char *error_fname, int s_loglevel);
void         UpdateLogErrorWrite(const char *file, int line, int level,
                 const char *user, const char *fmt, ...);
int          UpdateLogClose();

/**
 * Read the base bucket and overflow bucket config.
 * @[out]
 * pnBaseBucketNum - The number of base bucket
 * pnOverBucketNum - The number of overflow bucket
 * pnBucketSize - The size of a bucket
 */
API_EXPORT(int)          BkGetBaseBucketConfig();
API_EXPORT(int)          BkGetOverBucketConfig();
API_EXPORT(int)          BkGetSizeOfBucketConfig();

/* Get the flag of search a suitable node. */
API_EXPORT(int)          BkGetFlagOfSearchRoom();

/* Get the max number of increasing version */
API_EXPORT(int)          BkGetInceasingVersionNumber();

/* Get the increasing store flag */
API_EXPORT(int)          BkGetIncreasingStoreFlag();

/* Get store node list from extracted file
 *
 * @[in] param
 * pReq - request
 * pszExtractFile - extracted file name
 *
 * @[out] param
 * pHead - pointer, point to node list head
 * pnFileCount - file node count
 */
API_EXPORT(int)    GetStoreNodeList(request_rec *pReq,
                         const char *pszExtractFile,
                         PolicySt **pHead,
                         PRInt32 *pnFileCount);
/* 备份完成后释放用户占用空间链表. */
API_EXPORT(void) FreeDiskSpaceUserInfoList(const char *pszUser);

/* 限制流量速度 */
API_EXPORT(void) SetFluxSize(const char *pszUser);


/* ------------------------------ 操作共享内存接口 ------------------------------- */

/* 共享内存里写入任务类型和运行状态 */
API_EXPORT(int) BkChangeProcStatus(ShareThreadTaskSt *pstThrdInfo, int nThrdId, 
								   int nStatus, int nTimeFlag = 0);

/* 设置子进程线程状态设置为忙碌或空闲, 被实时复制和恢复模块使用 */
API_EXPORT(int) BKSetThreadSlotStatus(int nStatus);

/* 共享内存里写入任务运行状态 */
API_EXPORT(int) BKSetDBStart(char *pszModPath, char *pszUser, int nStatus, char *pszSharName);

/* 共享内存里读取启动数据库状态 */
API_EXPORT(int) BKGetStartDBStatus(int *pnTaskStatus, int nThrdId);

/* XXX: 给实时恢复模块使用，记录传输的IO块的序列号 */
API_EXPORT(int) BkGetThrdSeqAndTimeAddr(int nThrdId, PRInt64 **pnSeq, PRInt32 **pnTime);

/* 统一消息引擎调用接口, 共享内存头上获取线程个数 */
API_EXPORT(int) BkGetThreadNum(int *pnThrdCount, char *pszSharName);

/* 给统一消息引擎使用接口，共享内存插槽上获取所有任务 */
API_EXPORT(int) BkGetShareMemoryStatus(ShareThreadTaskSt *pstTaskList, int nThrdCount, int *pnTaskCount, char *pszSharName);

API_EXPORT(int) BKSetLogdataFlag(char *pszModPath, char *pszUser);

API_EXPORT(int) BkGetLogdataFlag(PRInt32 nThrdId, PRInt32 *nCleanFlag);

/* 获取所有线程状态，给backup6模块调用 */
API_EXPORT(int) BkBackupGetShareMemoryStatus(ShareThreadTaskSt *pstTaskList, int *pnTaskCount);

/* 接管回切使用,启动该用户的所有数据库 */
/* 共享内存里写入启动某个生产机所对应的所有实时恢复任务数据库的标记 ,nType=1时写status，nType=2时写Dbreceivesize*/
API_EXPORT(int) BKSetRecoverTaskDBStatus(char *pszUserName,int nType, int nStatus, char *pszSharName);

/* 自动接管时等待某个生产机所对应的所有的数据库启动 */
API_EXPORT(int) BKWaitRecoverDBStart(char *pszUserName, char *pszSharName);

API_EXPORT(int) MiGetServicePath(char *pszServName, char *pszPath, int nLen);

int ParserSendOracle(request_rec *pReq, int nFlag);

/* The Mssql SEND of protocol method parser to AgentServer.
 * request_rec: the context.
 * @[in]
 * Return WAVETOP_BACKUP_OK, when finish successful. Otherwise, return 
 * error code.
 */
int ParserSendMssql(request_rec *pReq, int nFlag);


/* The Sybase SEND of protocol method parser to AgentServer.
 * request_rec: the context.
 * @[in]
 * Return WAVETOP_BACKUP_OK, when finish successful. Otherwise, return 
 * error code.
 */
int ParserSendSybase(request_rec *pReq, int nFlag);


/* The File SEND of protocol method parser to AgentServer.
 * request_rec: the context.
 * @[in]
 * Return WAVETOP_BACKUP_OK, when finish successful. Otherwise, return 
 * error code.
 */
int ParserSendFile(request_rec *pReq, int nFlag);


int TransmitPolicyProtocol(request_rec *pReq);

/* The protocol handle */
int           HandleQueryShare(request_rec *pReq);
int           HandleDownloadShare(request_rec *pReq);
int           HandleUploadShare(request_rec *pReq);
int           HandleModify(request_rec *pReq);

#ifdef __cplusplus
}
#endif

#endif /* __BACKUP_MAIN_H_ 1 */
