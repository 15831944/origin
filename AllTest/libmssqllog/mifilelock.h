/** ========================================================================
 ** Copyright (c) 2003 WaveTop Information Corp. All rights reserved.
 **
 ** The Mirror system
 **
 ** ========================================================================
 */

#ifndef _MIFILELOCK_H_
#define _MIFILELOCK_H_ 1

/**
 * @defgroup file lock data struct and interface
 * @{
 */

#define WSTORE_RD_LOCK 1
#define WSTORE_WR_LOCK 2

#if defined (WIN32)
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
#else
#include "sem.h"
#endif
WTMI_BEGIN_DECLS

/**
 * @desc file lock initialize
 * @file mifilelock.cpp
 *
 * @param [in] pszName
 *          lock file name
 * @param [in/out] pLock
 *          file lock handle
 *
 * @ret if function success return WAVETOP_BACKUP_OK, otherwise 
 *      return a status code
 */
WTMI_LIBMI_EXPORT_(int) MiBackupFileLockInit(char *pszName, 
                                             BackupFileLockSt *pLock);

/**
 * @desc destory a file lock
 * @see ::MiBackupFileLockInit()
 * @file mifilelock.cpp
 *
 * @param [in] pLock
 *          file lock handle
 */
WTMI_LIBMI_EXPORT_(void) MiBackupFileLockDestroy(BackupFileLockSt *pLock);

/**
 * @desc enter critical section
 * @see ::MiBackupFileLockInit()
 * @file mifilelock.cpp
 *
 * @param [in] pLock
 *          file lock handle
 * @param [in] pLock
 *          lock option WSTORE_RD_LOCK or WSTORE_WR_LOCK
 *
 * @ret if function success return WAVETOP_BACKUP_OK, otherwise 
 *      return a status code
 */
WTMI_LIBMI_EXPORT_(int) MiBackupFileLockLock(BackupFileLockSt *pLock, 
                                             int nOption);

/**
 * @desc enter critical section 
 * @see ::MiBackupFileLockInit()
 * @file mifilelock.cpp
 *
 * @param [in] pLock
 *          file lock handle
 * @param [in] pLock
 *          lock option WSTORE_RD_LOCK or WSTORE_WR_LOCK
 *
 * @ret if function success return WAVETOP_BACKUP_OK, otherwise 
 *      return a status code
 */
WTMI_LIBMI_EXPORT_(int) MiBackupFileLockUnlock(BackupFileLockSt *pLock, 
                                               int nOption);

WTMI_LIBMI_EXPORT_(int) MiUnloadLibrary(void *pLib);
WTMI_LIBMI_EXPORT_(void *) MiGetLibFuncAdrr(void *pLib, char *pszName);
WTMI_LIBMI_EXPORT_(void *) MiLoadLibrary(char *pszLibName);

WTMI_LIBMI_EXPORT_(int) MiBackupPathIsAbsolute(const char *pszFile);

WTMI_LIBMI_EXPORT_(int) MiGetCurComputerHostName(char *pszHostName, 
                                                 unsigned long nBufSize);

/**
 * 获取UNC路径的剩余路径. 例如：
 * 
 * @[in]
 * pszUNCPath - UNC路径
 * @[out]
 * 仅当合法的UNC路径，返回WAVETOP_BACKUP_OK; 否则
 * 返回WAVETOP_BACKUP_INVALID_SYNTAX.
 * pszPath - 返回指针指向剩余路径。只有基本路径，则返回指针指向末尾'\0', 如: \\dell\test。
 * 如果存在多余路径, 如\\dell\test\readme.txt, 则返回指针指向"\readme.txt"。
 *
 */
WTMI_LIBMI_EXPORT_(int) MiGetUNCPath(char *pszUNCPath, char **pszPath);

WTMI_END_DECLS

/** @} */

#endif /* !defined(_MIFILELOCK_H_) */
