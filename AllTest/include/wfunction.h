/** =============================================================================
 ** Copyright (c) 2003 WaveTop Information Corp. All rights reserved.
 **
 ** The backup system
 **
 ** =============================================================================
 */

#ifndef __BACKUP_FUNCTION_H_
#define __BACKUP_FUNCTION_H_ 1
#include <time.h>
#include "nspr.h"
#include "walloc.h"
#include "wbase64.h"

#ifndef HUGE_STRING_LEN
#define HUGE_STRING_LEN  4096
#endif

struct request_rec {
    ap_pool *pool;
    int is_server;

    /* The protoco number of the request
     * Number version of protocol; 1.1 = 1001
     */
    int proto_num;

    /* The client os types:
     * BACKUP_OS_TYPE_WINNT
     * BACKUP_OS_TYPE_WIN9X
     * BACKUP_OS_TYPE_UNIX
     */
    int client_os;

    /* The authentication */
    int auth_way;
    const char *user;
    const char *passwd;

    /* The temporary storing directory */
    char *save_path;

    /* The connection sesession (for server) */
    void *connection;

    /* MIME header environments */
    table *headers_in;

    /* The protocol method */
    int proto_method;

    /* nTask id  if(id == -1) The task is not under monitor */
    unsigned long nTaskMonitorId;
    unsigned long nOldTaskId;

    /* In any case */
    int status; 
    /* The protocol number of the response */
    int response_proto;

    /* The backup task list for server */
    void *backup;

    /* The flag use in bit */
    int nbackupflag;

    PRLock *backlock;
};

/* 接管的配置文件中，记录的客户端的网络连接信息 */
typedef struct _BACKUP6_HOST_CONF_NODE_ {
	int nModeId;	
	char szCheckIP[16];	
	char szNetMask[16];
	char szNetGateway[128];

	char szCheckIP1[16];	
	char szNetMask1[16];
	char szNetGateway1[128];

	char szCheckUser[256];
	int nCheckPort;
	int nCheckCount;
	int nFailCount;
	/* true 表示已经接管 */
	bool IsSwitched;
	struct _BACKUP6_HOST_CONF_NODE_ *pNext;
} HostConfNode;

typedef struct request_rec request_rec;

/* The backup file lock */
typedef struct BACKUP_FILE_LOCK_ST {
    unsigned long nFileHandle;
    long nOptios;
#ifdef WIN32
    char szLockFile[MAX_PATH];
    OVERLAPPED OverLapped;
#else
    char szLockFile[MAX_PATH];
#endif
} BackupFileLockSt;

#ifdef __cplusplus
extern "C" {
#endif

/* a) remove ./ path segments
 * b) remove trailing . path, segment
 * c) remove all xx/../ segments. (including leading ../ and /../)
 * d) remove trailing xx/.. segment
 */
API_EXPORT(void)     BackupGetParents(char *pszName);

API_EXPORT(void)     BackupNo2Slash(char *pszName);

/* Get a line from file handle */
int      BackupGetLine(char *buf, size_t bufsize, int opt, FILE *file);

/* Backup file locak utilities */
API_EXPORT(int)      BackupFileLockInit(char *pszName, BackupFileLockSt *pLock);
API_EXPORT(void)     BackupFileLockDestroy(BackupFileLockSt *pLock);
API_EXPORT(int)      BackupFileLockLock(BackupFileLockSt *pLock, int nOption);
API_EXPORT(int)      BackupFileLockUnlock(BackupFileLockSt *pLock, int nOption);

API_EXPORT(char *)   BackupFetchType(char *pszFile);
API_EXPORT(int)      BackupFileFilter(char *pszFile);

char    *ap_getword_conf(char *result, int size, const char **line);

/* The secure string copy function */
char    *BackupGetwordNulls(pool *atrans, const char **line, char stop);
char    *BackupEscapeShellcmd(pool *pPool, const char *str);

/* Get The Space Search rate */
API_EXPORT(float)   GetSpaceSearchRate();

/* Load the CA Library */
int     LoadCALibrary();

/* Check the MAC address is correct */
int     CheckMACAddress();

int     BkGetFileHMAC(char *pszFilename, char *pszHMAC, unsigned long nBufSize);

/* Get The User Index And Store Backup Path */
int     GetIndexAndStoreBKPath(char *pszIdxStrPathBuf, int nBufSize);

/* Backup user index or store file before write into it */
API_EXPORT(int)     BackupUserIdxOrStrFile(const char *pszSourFile);

/*  Perform canonicalization with the exception that the
 *  input case is preserved.
 */
API_EXPORT(char *) BackupCanonicalFilename2(pool *pPool, const char *pszPath);

void BkCheckConnServer();
int BkSetSwitchIp(HostConfNode *pHostNode);


#ifdef __cplusplus
}
#endif

#endif /* __BACKUP_FUNCTION_H_ 1 */
