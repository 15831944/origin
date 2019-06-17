/** ========================================================================
 ** Copyright (c) 2003 WaveTop Information Corp. All rights reserved.
 **
 ** The Mirror system
 **
 ** ========================================================================
 */

#ifndef _MIRROR_LIBRARY_UTIL_H_
#define _MIRROR_LIBRARY_UTIL_H_ 1



/**
 * @defgroup windows services control functions
 * @{
 */

#if defined(WIN32)

#ifdef __cplusplus
extern "C" {
#endif /* defined(__cplusplus) */


typedef struct _MiService {     /**< service description */
    char *pszServiceName;       /**< service name */
    char *pszDisplayName;       /**< service display name */
    char *pszServiceCmd;        /**< service startup command */
    char *pszBinPath;           /**< service binary path */
    unsigned long nStartType;   /**< service start type (see MSDN):
                                    
                                     SERVICE_BOOT_START
                                     SERVICE_SYSTEM_START
                                     SERVICE_AUTO_START
                                     SERVICE_DEMAND_START
                                     SERVICE_DISABLED

                                   */
} MiService;

/**
 * @desc create/modify windows services
 * @file libs/libmirror/miwin32.cpp
 * 
 * @param pService  [in]
 *          windows services description see ::MiService
 *
 * @param nZeroIsReconfig [in]
 *          if 0 == nZeroIsReconfig then modify a existing windows service
 *          else create a new windows service
 *
 * @ret if the function succeeds, the return value is WAVETOP_BACKUP_OK,
 *      otherwise return a mirror status code, see mirror.h
 */
WTMI_LIBMI_EXPORT_(int) MiServCreateServiceWin32(MiService *pService, 
                                                 long nZeroIsReconfig);

/**
 * @desc set windows service description string
 * @file libs/libmirror/miwin32.cpp
 * 
 * @param pszServicesName [in]
 *          service name
 *
 * @param pszDescStr [in]
 *          service description string
 *
 * @ret if the function succeeds, the return value is WAVETOP_BACKUP_OK,
 *      otherwise return a mirror status code, see mirror.h
 */
WTMI_LIBMI_EXPORT_(int) MiServSetDescription(char *pszServicesName, 
                                             char *pszDescStr);

/**
 * @desc startup windows service
 * @file libs/libmirror/miwin32.cpp
 *
 * @param pszServiceName  [in]
 *          windows services name
 *
 * @ret if the function succeeds, the return value is WAVETOP_BACKUP_OK,
 *      otherwise return a mirror status code, see mirror.h
 */
WTMI_LIBMI_EXPORT_(int) MiServServiceStart(char *pszServiceName);

/* service app real entry */
typedef int  (*SysMainFunction)(int argc, char **argv, char **env);

/* service app exit callback function */
typedef void (*SysExitFunction)(void);

/**
 * @desc windows service app entry
 * @file libs/libmirror/miwin32.cpp
 *
 * @param pszServiceName [in]
 *          service name
 *
 * @param pMainFunction [in]
 *          service app real entry
 *
 * @param pExitFunction [in]
 *          service app exit callback function
 *
 * @ret if the function succeeds, the return value is WAVETOP_BACKUP_OK,
 *      otherwise return a mirror status code, see mirror.h
 */
WTMI_LIBMI_EXPORT_(int) MiServServiceStartup(char *pszServiceName, 
                                             SysMainFunction pMainFunction, 
                                             SysExitFunction pExitFunction);

/**
 * @desc query service status
 * @file libs/libmirror/miwin32.cpp
 *
 * @param  pServiceName [in]
 *          service name
 * @param pStatus [in/out]
 *          for receive service status
 * 
 * @ret if the function succeeds, the return value is WAVETOP_BACKUP_OK,
 *      otherwise return a mirror status code, see mirror.h
 */
WTMI_LIBMI_EXPORT_(int) MiServQueryServiceStatus(char *pServiceName, 
                                                 SERVICE_STATUS *pStatus);

/**
 * @desc stop a running services
 * @file libs/libmirror/miwin32.cpp
 *
 * @param pszServiceName
 *          service name
 *
 * @ret if the function succeeds, the return value is WAVETOP_BACKUP_OK,
 *      otherwise return a mirror status code, see mirror.h
 */
WTMI_LIBMI_EXPORT_(int) MiServStopService(char *pszServiceName);

/**
 * @desc delete a services
 * @file libs/libmirror/miwin32.cpp
 *
 * @param pszServiceName
 *          service name
 *
 * @ret if the function succeeds, the return value is WAVETOP_BACKUP_OK,
 *      otherwise return a mirror status code, see mirror.h
 */
WTMI_LIBMI_EXPORT_(int) MiServDelService(char *pszServiceName);

#ifdef __cplusplus
}
#endif /* defined(__cplusplus) */

#endif /* defined(WIN32) */

/** @} */

#endif /* !defined(_MIRROR_LIBRARY_UTIL_H_) */
