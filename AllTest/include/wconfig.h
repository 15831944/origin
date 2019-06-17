/** =============================================================================
 ** Copyright (c) 2003 WaveTop Information Corp. All rights reserved.
 **
 ** The backup system
 **
 ** =============================================================================
 */

#ifndef __WAVETOP_CONFIG_H_
#define __WAVETOP_CONFIG_H_ 1

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include "compoption.h"

#define SIZEOF_INT 4
#define SIZEOF_LONG 4
#define SIZEOF_SHORT 2
#define inline __inline

#ifndef MAX_PATH
#define MAX_PATH 1024
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 0
#endif

#ifdef WIN32

/* No use sigaction */
#define NO_USE_SIGACTION

typedef void RETSIGTYPE;

#if 0 /* C++ Builder defined */
typedef int uid_t;
typedef int gid_t;
typedef int pid_t;
typedef int tid_t;
typedef int mode_t;
#endif

#ifndef uid_t
#define uid_t int
#define gid_t int
#define pid_t int
#define tid_t int
#define mode_t int
#endif

typedef char * caddr_t;

#define GETGROUPS_T int
#define HAVE_STRUCT_STAT_ST_RDEV 1

#define strcasecmp(s1, s2) stricmp(s1, s2)
#define strncasecmp(s1, s2, n) strnicmp(s1, s2, n)

#define snprintf _snprintf
#define vsnprintf _vsnprintf

#else /* On UNIX platform */

#define stricmp(s1, s2) strcasecmp(s1, s2)
#define strnicmp(s1, s2, n) strncasecmp(s1, s2, n)

#endif

/* So that we can use inline on some critical functions, and use
 * GNUC attributes (such as to get -Wall warnings for printf-like
 * functions).  Only do this in gcc 2.7 or later ... it may work
 * on earlier stuff, but why chance it.
 *
 * We've since discovered that the gcc shipped with NeXT systems
 * as "cc" is completely broken.  It claims to be __GNUC__ and so
 * on, but it doesn't implement half of the things that __GNUC__
 * means.  In particular it's missing inline and the __attribute__
 * stuff.  So we hack around it.  PR#1613. -djg
 */
#if !defined(__GNUC__) || __GNUC__ < 2 || \
    (__GNUC__ == 2 && __GNUC_MINOR__ < 7) ||\
    defined(NEXT)
#define ap_inline
#define __attribute__(__x)
#define ENUM_BITFIELD(e,n,w)  signed int n : w
#else
#define ap_inline __inline__
#define USE_GNU_INLINE
#define ENUM_BITFIELD(e,n,w)  e n : w
#endif

#ifdef WIN32

#ifndef API_EXPORT
#define API_EXPORT(p) __declspec(dllexport) p
#endif
#else
#ifndef API_EXPORT
#define API_EXPORT(p) extern p
#endif
#endif


/* SOCKET type for UNIX */
#ifndef WIN32

#ifndef SOCKET
#define SOCKET int
#endif

#ifndef SOCKET_ERROR
#define SOCKET_ERROR -1
#endif

#ifndef SD_BOTH
#define SD_BOTH 2
#endif

#ifndef INVALID_SOCKET
#define INVALID_SOCKET (~0)
#endif

#define closesocket close

#endif

#if defined(AIX)
#if !defined(MAP_FAILED)
#define  MAP_FAILED ((void *) -1)
#endif 
#endif

#endif /* __WAVETOP_CONFIG_H_ */

