/** =============================================================================
 ** Copyright (c) 2003 WaveTop Information Corp. All rights reserved.
 **
 ** The backup system
 **
 ** =============================================================================
 */

#ifndef _SEM_LIB_H_
#define _SEM_LIB_H_

#if !defined(WIN32) /* semaphore for unix */

#ifndef API_EXPORT
#define API_EXPORT(p) extern p
#endif

#define LIB_SEM_OPEN_CREATE     (1 << 0)
#define LIB_SEM_OPEN_EXCL       (1 << 1)

#define LIB_SEM_UNDO            (1 << 0)
#define LIB_SEM_NOWAIT          (1 << 1)

union semun {
    int val;                    /* value for SETVAL */
    struct semid_ds *buf;       /* buffer for IPC_STAT, IPC_SET */
    unsigned short int *array;  /* array for GETALL, SETALL */
    struct seminfo *__buf;      /* buffer for IPC_INFO */
};

API_EXPORT(int) open_sem(char *name, char proj, int flag,
                         int mode, int val, int *pnSemid);

API_EXPORT(int) delete_sem(char *name, char proj);

API_EXPORT(int) release_sem(int semid);

API_EXPORT(int) wait_sem(int semid, int flag);

#endif /* !defined(WIN32) */

#endif /* #define _ORASEM_H_ */
