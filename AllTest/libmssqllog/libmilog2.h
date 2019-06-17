/** =============================================================================
 ** Copyright (c) 2003 WaveTop Information Corp. All rights reserved.
 **
 ** The Mirror system
 **
 ** =============================================================================
 */

/**
 * @desc mirror log dll export functions and types
 * @author lexiongjia
 * @file libmilog.h
 */

#ifndef _LIBMI_LOG2_H_ 
#define _LIBMI_LOG2_H_ 1

#include <stdarg.h>

/* The export API definition */
#ifdef WIN32
#ifndef API_EXPORT
#define API_EXPORT(p) __declspec(dllexport) p
#endif
#else
#ifndef API_EXPORT 
#define API_EXPORT(p) extern p
#endif
#endif

#define APLOG_EMERG 0       /* system is unusable */
#define APLOG_ALERT 1       /* action must be taken immediately */
#define APLOG_CRIT  2       /* critical conditions */
#define APLOG_ERR   3       /* error conditions */
#define APLOG_WARNING   4   /* warning conditions */
#define APLOG_NOTICE    5   /* normal but significant condition */
#define APLOG_INFO  6       /* informational */
#define APLOG_DEBUG 7       /* debug-level messages */

#define APLOG_LEVELMASK 7   /* mask off the level value */
#define APLOG_MAX_NAME  1024

#define APLOG_NOERRNO       (APLOG_LEVELMASK + 1)
#ifdef WIN32
/* Set to indicate that error msg should come from Win32's GetLastError(),
 * not errno. 
 */
#endif

#define APLOG_WIN32ERROR    ((APLOG_LEVELMASK+1) * 2)

#ifndef DEFAULT_LOGLEVEL
#define DEFAULT_LOGLEVEL    APLOG_WARNING
#endif

#define APLOG_MARK  __FILE__,__LINE__

#ifdef __cplusplus
extern "C" {
#endif /* defined(__cplusplus) */

API_EXPORT(int)  SLogInit(const char *error_fname, int s_loglevel);
API_EXPORT(void) SLogSetDump(int dumprows);

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
                 const char *user, const char *user2, const char *fmt, va_list args);

API_EXPORT(int)  SLogClose();

#ifdef __cplusplus
}
#endif /* defined(__cplusplus) */

#endif /* !defined(_LIBMI_LOG_H_) */

