/** =============================================================================
 ** Copyright (c) 2007 WaveTop Information Corp. All rights reserved.
 **
 ** The mirror system
 **
 ** =============================================================================
 */

#if !defined(WIN32)

#include <sys/types.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include "sem.h"

MILOG_EXPORT_(lib_sem *) open_sem(char *name, char proj, int flag,
                               int mode, int init, int max)
{
    struct sembuf sop;
    struct semid_ds seminfo;
    union semun arg;
    key_t key;
    lib_sem *psem = NULL;
    int fd;
    int rc;
    int i;

    if (flag & LIB_SEM_OPEN_CREATE) {
        fd = open(name, O_RDWR | O_CREAT, mode);
        if (fd == -1) {
            return NULL;
        }

        rc = close(fd);
        if (rc != 0) {
            return NULL;
        }
    }

    key = ftok(name, proj);
    if (key == (key_t)-1) {
        return NULL;
    }

    psem = (lib_sem *)malloc(sizeof(lib_sem));
    if (psem == NULL) {
        return NULL;
    }

    if (flag & LIB_SEM_OPEN_CREATE) {
        psem->semid = semget(key, 1, mode | IPC_CREAT | IPC_EXCL);
        if (psem->semid >= 0) {
            arg.val = init;
            if (semctl(psem->semid, 0, SETVAL, arg) == -1) {
                free(psem);
                return NULL;
            }

            return psem;
        }

        if (errno != EEXIST || flag & LIB_SEM_OPEN_EXCL) {
            free(psem);
            return NULL;
        }
    }

    psem->semid = semget(key, 0, 0);
    if (psem->semid == -1) {
        free (psem);
        return NULL;
    }

    for (i = 0; i < 60; i++) {
        arg.buf = &seminfo;
        semctl(psem->semid, 0, IPC_STAT, arg);
        if (seminfo.sem_otime != 0) {
            break;
        }

        sleep(1);
    }

    if (i == 60) {
        free(psem);
        return NULL;
    }

    return psem;
}

MILOG_EXPORT_(int) close_sem(lib_sem *psem)
{
    if (psem != NULL) {
        free(psem);
    }

    return WAVETOP_BACKUP_OK;
}

MILOG_EXPORT_(int) delete_sem(char *name, char proj)
{
    union semun unused;
    key_t key;
    int semid;
    int rc;

    key = ftok(name, proj);
    if ((key_t) -1 == key) {
        return WAVETOP_BACKUP_INTERNAL_ERROR;
    }

    semid = semget(key, 1, 0666);
    if (semid == -1) {
        return WAVETOP_BACKUP_INTERNAL_ERROR;
    }

    unused.val = 0;
    if (semctl(semid, 0, IPC_RMID, unused) == -1) {
        return WAVETOP_BACKUP_INTERNAL_ERROR;
    }

    return WAVETOP_BACKUP_OK;
}

MILOG_EXPORT_(int) release_sem(lib_sem *psem)
{
    struct sembuf sop;

    sop.sem_num = 0;
    sop.sem_op = 1;
    sop.sem_flg = 0;
    if (semop(psem->semid, &sop, 1) == -1) {
        return WAVETOP_BACKUP_INTERNAL_ERROR;
    }

    return WAVETOP_BACKUP_OK;
}

MILOG_EXPORT_(int) wait_sem(lib_sem *psem, int flag)
{
    struct sembuf sop;
    short semflag = 0;

    if (flag & LIB_SEM_WAIT_NOWAIT) {
        semflag = IPC_NOWAIT;
    } 

    sop.sem_num = 0;
    sop.sem_op = -1;
    sop.sem_flg = semflag;
    if (semop(psem->semid, &sop, 1) == -1) {
        return WAVETOP_BACKUP_INTERNAL_ERROR;
    }

    return WAVETOP_BACKUP_OK;
}


/* Backup file locak utilities */
int BackupFileLockInit(char *pszName, BackupFileLockSt *pLock)
{
    pLock->nFileHandle = open(pszName, O_CREAT|O_RDWR, 0664);
    if (pLock->nFileHandle == -1) {
        return WAVETOP_BACKUP_OPEN_FILE_ERROR;
    }

    strncpy(pLock->szLockFile, pszName, sizeof(pLock->szLockFile));

    return WAVETOP_BACKUP_OK;
}

void BackupFileLockDestroy(BackupFileLockSt *pLock)
{
    close(pLock->nFileHandle);
}

/* rw: 0 is read lock; 1 is write lock */
int BackupFileLockLock(BackupFileLockSt *pLock, int rw)
{
    struct flock lock_it;
    int cmd;
    int ret;

    lock_it.l_whence = SEEK_SET;
    lock_it.l_start = 0;
    lock_it.l_len = 0;
    lock_it.l_type = (rw == WSTORE_WR_LOCK ? F_WRLCK : F_RDLCK);
    lock_it.l_pid = 0;  /* pid not actually interesting */
    cmd = (rw == WSTORE_WR_LOCK ? F_SETLKW : F_SETLK);
    while ((ret = fcntl(pLock->nFileHandle, cmd, &lock_it)) < 0 
        && errno == EINTR); /* nop */
    return (ret < 0 ? 1 : 0);
}

int BackupFileLockUnlock(BackupFileLockSt *pLock, int rw)
{
    struct flock unlock_it;
    int cmd;
    int ret;
 
    unlock_it.l_whence = SEEK_SET;  /* from current point */
    unlock_it.l_start = 0;      /* -"- */
    unlock_it.l_len = 0;        /* until end of file */
    unlock_it.l_type = F_UNLCK;     /* set exclusive/write lock */
    unlock_it.l_pid = 0;        /* pid not actually interesting */
    cmd = (rw == WSTORE_WR_LOCK ? F_SETLKW : F_SETLK);
    while ((ret = fcntl(pLock->nFileHandle, cmd, &unlock_it)) < 0 
        && errno == EINTR); /* nop */
    return (ret < 0 ? 1 : 0);
}

#endif /* !defined(WIN32) */
