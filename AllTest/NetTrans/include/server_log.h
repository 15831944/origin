/** =============================================================================
 ** Copyright (c) 2003 WaveTop Information Corp. All rights reserved.
 **
 ** The backup system
 **
 ** =============================================================================
 */

/** For the server log system **/

#ifndef _BACKUP_SERVER_LOG_H_
#define _BACKUP_SERVER_LOG_H_ 1

#include "wconfig.h"
#include <stdarg.h>

#ifdef HAVE_SYSLOG
#include <syslog.h>

#define APLOG_EMERG     LOG_EMERG     /* system is unusable */
#define APLOG_ALERT     LOG_ALERT     /* action must be taken immediately */
#define APLOG_CRIT      LOG_CRIT      /* critical conditions */
#define APLOG_ERR       LOG_ERR       /* error conditions */
#define APLOG_WARNING   LOG_WARNING   /* warning conditions */
#define APLOG_NOTICE    LOG_NOTICE    /* normal but significant condition */
#define APLOG_INFO      LOG_INFO      /* informational */
#define APLOG_DEBUG     LOG_DEBUG     /* debug-level messages */

#define APLOG_LEVELMASK LOG_PRIMASK   /* mask off the level value */

#else

#define APLOG_EMERG 0   /* system is unusable */
#define APLOG_ALERT 1   /* action must be taken immediately */
#define APLOG_CRIT  2   /* critical conditions */
#define APLOG_ERR   3   /* error conditions */
#define APLOG_WARNING   4   /* warning conditions */
#define APLOG_NOTICE    5   /* normal but significant condition */
#define APLOG_INFO  6   /* informational */
#define APLOG_DEBUG 7   /* debug-level messages */

#define APLOG_LEVELMASK 7   /* mask off the level value */
#define APLOG_MAX_NAME  1024

#endif

#define LOGDIR_MAX_SIZE                 (2.00 * (1 << 30))    //单位:GB
#define LOG_MAX_SIZE                    1024 * 1024 * 50    //单位:MB

#define APLOG_NOERRNO       (APLOG_LEVELMASK + 1)
#ifdef WIN32
/* Set to indicate that error msg should come from Win32's GetLastError(),
 * not errno. 
 */
#define APLOG_WIN32ERROR    ((APLOG_LEVELMASK+1) * 2)
#endif

#ifndef DEFAULT_LOGLEVEL
#define DEFAULT_LOGLEVEL    APLOG_WARNING
#endif

#define APLOG_MARK  __FILE__,__LINE__


/* The path to the shell interpreter, for parsed docs */
#ifndef SHELL_PATH
#if defined(OS2) || defined(WIN32)
/* Set default for OS/2 and Windows file system */
#define SHELL_PATH "CMD.EXE"
#else
#define SHELL_PATH "/bin/sh"
#endif
#endif /* SHELL_PATH */

/* ... even child processes (which we may want to wait for,
 * or to kill outright, on unexpected termination).
 *
 * ap_spawn_child is a utility routine which handles an awful lot of
 * the rigamarole associated with spawning a child --- it arranges
 * for pipes to the child's stdin and stdout, if desired (if not,
 * set the associated args to NULL).  It takes as args a function
 * to call in the child, and an argument to be passed to the function.
 */

struct BackupChildInfoSt {
#ifdef WIN32
    /*
     *  These handles are used by ap_call_exec to call 
     *  create process with pipe handles.
     */
    HANDLE hPipeInputRead;
    HANDLE hPipeOutputWrite;
    HANDLE hPipeErrorWrite;
#else
    /* 
     * We need to put a dummy member in here to avoid compilation
     * errors under certain Unix compilers, like SGI's and HPUX's,
     * which fail to compile a zero-sized struct.  Of course
     * it would be much nicer if there was actually a use for this
     * structure under Unix.  Aah the joys of x-platform code.
     */
    int dummy;
#endif
};

#ifdef __cplusplus
extern "C" {
#endif

API_EXPORT(int)  SLogInit(const char *error_fname, int s_loglevel);

/* The two primary logging functions, ap_log_error and ap_log_rerror,
 * use a printf style format string to build the log message.  It is
 * VERY IMPORTANT that you not include any raw data from the network,
 * such as the request-URI or request header fields, within the format
 * string.  Doing so makes the server vulnerable to a denial-of-service
 * attack and other messy behavior.  Instead, use a simple format string
 * like "%s", followed by the string containing the untrusted data.
 */
API_EXPORT(void) SLogErrorWrite(const char *file, int line, int level,
                 const char *user, const char *fmt, ...);
API_EXPORT(void) SLogErrorWrite2(const char *file, int line, int level,
                 const char *user, int id, const char *fmt, ...);

API_EXPORT(void) SLogErrorWrite3(const char *file, int line, int level,
    const char *user, const char *fmt, va_list args);

API_EXPORT(int)  SLogClose();

API_EXPORT(void) SLogSetDump(int dumprows);

#ifdef __cplusplus
}
#endif

#endif /* _BACKUP_SERVER_LOG_H_ 1 */
