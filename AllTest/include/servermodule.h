/** =============================================================================
 ** Copyright (c) 2003-2005 WaveTop Information Corp. All rights reserved.
 **
 ** The Backup system
 **
 ** =============================================================================
 */

#ifndef _BACKUP_SERVER_MODULE_H_
#define _BACKUP_SERVER_MODULE_H_ 1

#include "nspr.h"
#include "backup_proto.h"
#include "wfunction.h"

/* The versionn of server module */
#define BACKUP_MODULE_SERVER_VERSION     1000
#define BACKUP_MODULE_SERVER_MAGIC       "BackupServer"

#define MAKE_TASK_POOL(p) ap_make_sub_pool(p)


/* The Initialization API.
 * @[in]
 * ap_pool *pPool: the pool of the taskctrl 
 * int nWay: 1. the first initialize. 2. restarting initialize.
 * void *pConfig: the module configuation data.
 * @[out]
 * Return WAVETOP_BACKUP_OK, when successful. Otherwise, return error code. 
 */
typedef int (*BCInitFunc) (pool *pPool, int nWay, void *pConfig);

/* Beginning of handling.
 * @[out]
 * void *pFileList: the DataBase Export file list.
 * Return WAVETOP_BACKUP_OK, when successful. Otherwise, return error code. 
 */
typedef int (*BCBeginHandleFunc) (request_rec *pReq);

/* The protocol handling API
 * ConnectionSt *pConn: the network connection.
 * TaskJob *pJob: the task object.
 * void *pFileList: the DataBase Export file list.
 * @[out]
 * Return WAVETOP_BACKUP_OK, when successful. Otherwise, return error code. 
 */
typedef int (*BCHandleFunc) (request_rec *pReq);

/* End of handling task job. 
 * ConnectionSt *pConn: the network connection.
 * TaskJob *pJob: the task object.
 * @[out]
 * Return WAVETOP_BACKUP_OK, when successful. Otherwise, return error code. 
 */
typedef void (*BCEndHandleFunc) (request_rec *pReq);

/* Release the module. */
typedef int (*BCCloseFunc) (void);

typedef struct _BACKUP_MODULE_SERVER_ST {
    char szModule[128];
    long nVersion;

    /* The protocol number that this module handles */
    short nProtoCode;

    /* The object type that this module handles. Only when 
     * nProtoCode == BACKUP_PROTO_BACKUP_OBJECT  OR 
     * nProtoCode == BACKUP_PROTO_RESTORE_OBJECT.
     */
    char szObjectType[128];
    unsigned long nReserve1;
    unsigned long nReserve2;
    unsigned long hModule;

    BCInitFunc pMCInitFunc;
    BCBeginHandleFunc pMCBeginHandleFunc;
    BCHandleFunc pMCHandleFunc;

    /* When this callbackup is NULL, then call the default END function. */
    BCEndHandleFunc pMCEndHandleFunc;
    BCCloseFunc pMCCloseFunc;
} ModuleServerSt;

#define BACKUP_SERVER_MAX_MODULES     256

/* backup system Server module type */
typedef char BackupServerModuleType;
#define BACKUP_SERVER_MODULE_TYPE_SELF    1
#define BACKUP_SERVER_MODULE_TYPE_LOAD    2

typedef struct _BACKUP_SERVER_MODULES_SLOT {
    char cZeroIsNotInUse;
    void *pModuleLib;
    BackupServerModuleType nModuleType;
    ModuleServerSt stModule;
} BackupServerModulesSlot;

/* Load basic server module */
int PreLoadServerBaseModules(pool *pPool);

/* Load the specified module into the backupcore modules queue, then
 * initialize the module with the configuration settting.
 */
int BackupLoadServerModule(const char *pszModule, void *pConfig);

/* call module func */
int HandleProtoModules(request_rec *pReq);

#endif /*  !defined(_BACKUP_SERVER_MODULE_H_) */
