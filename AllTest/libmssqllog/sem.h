/** =============================================================================
 ** Copyright (c) 2003 WaveTop Information Corp. All rights reserved.
 **
 ** The mirror system
 **
 ** =============================================================================
 */

#ifndef _ORASEM_H_
#define _ORASEM_H_


#if !defined(WIN32) /* semaphore for unix */

#include "mirror.h"

/* the dbm lock */
#define WSTORE_RD_LOCK 1
#define WSTORE_WR_LOCK 2

#ifndef MILOG_EXPORT
#define MILOG_EXPORT               extern
#endif
#ifndef MILOG_EXPORT_
#define MILOG_EXPORT_(__type)      extern __type
#endif

#define ORACLE_BIN_PATH_LEN     512
#define ORACLE_SCRIPT_NAME_LEN  512
#define ORACLE_INFO_LEN         8192

#define LIB_SEM_OPEN_CREATE     (1 << 0)
#define LIB_SEM_OPEN_EXCL       (1 << 1)

#define LIB_SEM_WAIT_NOWAIT     (1 << 0)

typedef struct _LIB_SEM {
    int semid;
} lib_sem;

typedef struct BACKUP_FILE_LOCK_ST {
    unsigned long nFileHandle;
    long nOptios;
#ifdef WIN32
char szLockFile[MAX_PATH];
    OVERLAPPED OverLapped;
#else
    char szLockFile[512];
#endif
} BackupFileLockSt;

union semun {
    int val;                    /* value for SETVAL */
    struct semid_ds *buf;       /* buffer for IPC_STAT, IPC_SET */
    unsigned short int *array;  /* array for GETALL, SETALL */
    struct seminfo *__buf;      /* buffer for IPC_INFO */
};

/* Backup file locak utilities */
int      BackupFileLockInit(char *pszName, BackupFileLockSt *pLock);
void     BackupFileLockDestroy(BackupFileLockSt *pLock);
int      BackupFileLockLock(BackupFileLockSt *pLock, int nOption);
int      BackupFileLockUnlock(BackupFileLockSt *pLock, int nOption);

MILOG_EXPORT_(lib_sem *) open_sem(char *name, char proj, int flag,
                               int mode, int init, int max);

MILOG_EXPORT_(int) close_sem(lib_sem *psem);

MILOG_EXPORT_(int) delete_sem(char *name, char proj);

MILOG_EXPORT_(int) release_sem(lib_sem *psem);

MILOG_EXPORT_(int) wait_sem(lib_sem *psem, int flag);

#endif /* !defined(WIN32) */

#endif /* #define _ORASEM_H_ */

