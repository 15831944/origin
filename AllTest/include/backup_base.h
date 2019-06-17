/** =============================================================================
 ** Copyright (c) 2003-2005 WaveTop Information Corp. All rights reserved.
 **
 ** The Backup system
 **
 ** =============================================================================
 */

#include <time.h>
#include "nspr.h"
#include "backup_proto.h"

/* The base functions. */

#ifndef WAVETOP_BACKUP_BASE_FUNCTION_H
#define WAVETOP_BACKUP_BASE_FUNCTION_H 1

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 从指定的字符串中获取语意段.
 * 语意段是以空格隔开. 若语意段中有空格则把其两端加 escape 字符。
 * For Example　 : 　　　　abcd def "efax  das" ddk   uer
 * 其中共有五个语意段：1. abcd
 *               　    2. def 
 *                　   3. efax  das
 *              　　　 4. ddk
 *                  　 5. uer
 * 起始和终结escape 字符均为 '"'.
 *
 * @[in]
 * cSeparatorStart, cSeparatorEnd - the escape letter.
 *
 */
API_EXPORT(char *) BackupGetWord2(char *result, int size, const char **line,
                    char cSeparatorStart, char cSeparatorEnd);
API_EXPORT(char *) BackupGetWord(char *result, int size, const char **line);


/* The base functions (in function.cpp)
 * Get the server OS type 
 * @[out]
 * BACKUP_OS_TYPE_WINNT     1   // Windows NT 
 * BACKUP_OS_TYPE_UNIX      3   // Unix 
 */
API_EXPORT(int)    GetCurrentOSType();

/* Get Current Operating System error num */
API_EXPORT(int) GetCurrentOSError();

/**
 * Perform canonicalization with the exception that the
 * input case is preserved.
 * @[out]
 * return value - the canonical file name
 */
API_EXPORT(char *) BackupCanonicalFilename(char *pszFilename);

/**
 * Statistics a file information.
 * @[in]
 * pszFilename - the file name.
 * @[out]
 * Return WAVETOP_BACKUP_OK, when exists. Otherwise, return error code.
 * nType - the file type. BACKUP_FILE_FILE, BACKUP_FILE_DIRECTORY.
 * nSize - the file size.
 */
API_EXPORT(int)    BackupStat(const char *pszFilename, PRInt32 *nType, PRInt64 *nSize);

/**
 * Set the file normal attribytes to delete
 * @[out]
 * return value - WAVETOP_BACKUP_OK is successful
 */
API_EXPORT(int)    SetFileNormal(const char *pszFilename);

/**
 * Create a file.
 * @[out]
 * Try to create parent path of this file.
 * Return the file descriptor, when successful.
 */
API_EXPORT(PRFileDesc *) BackupOpenFile(const char *pszFilename, PRIntn nFlags, 
                                         PRIntn nMode);
API_EXPORT(int)    BackupDelete(char *pszFilename);
API_EXPORT(int)    BackupDeleteFiles(char *pszDir);
API_EXPORT(int)    BackupCreateParentDir(const char *pszFile);
API_EXPORT(int)    BackupCreateDir(const char *pszDir);
API_EXPORT(int)    BackupRemoveDir(char *pszDir);

/**
 * Is an absolute path.
 * @[out]
 * Return WAVETOP_BACKUP_OK, if a absolute path as '/usr/wavetop/backup'.
 * Return Other code.
 */
API_EXPORT(int)    BackupPathIsAbsolute(char *pszPath);
/**********************************************************************************
 * The directory and file utilities end
 **********************************************************************************/

/** 
 * BackupStrncpy - The secure string copy function.
 */
API_EXPORT(int)    BackupStrncpy(char *pszDest, char *pszSrc, int nDestsize);

/*
 * Parses an HTTP date in one of three standard forms:
 *
 *     Sun, 06 Nov 1994 08:49:37 GMT  ; RFC 822, updated by RFC 1123
 *     Sunday, 06-Nov-94 08:49:37 GMT ; RFC 850, obsoleted by RFC 1036
 *     Sun Nov  6 08:49:37 1994       ; ANSI C's asctime() format
 *
 * and returns the time_t number of seconds since 1 Jan 1970 GMT, or
 * 0 if this would be out of range or if the date is invalid.
 *
 * The restricted HTTP syntax is
 * 
 *     HTTP-date    = rfc1123-date | rfc850-date | asctime-date
 *
 *     rfc1123-date = wkday "," SP date1 SP time SP "GMT"
 *     rfc850-date  = weekday "," SP date2 SP time SP "GMT"
 *     asctime-date = wkday SP date3 SP time SP 4DIGIT
 *
 *     date1        = 2DIGIT SP month SP 4DIGIT
 *                    ; day month year (e.g., 02 Jun 1982)
 *     date2        = 2DIGIT "-" month "-" 2DIGIT
 *                    ; day-month-year (e.g., 02-Jun-82)
 *     date3        = month SP ( 2DIGIT | ( SP 1DIGIT ))
 *                    ; month day (e.g., Jun  2)
 *
 *     time         = 2DIGIT ":" 2DIGIT ":" 2DIGIT
 *                    ; 00:00:00 - 23:59:59
 *
 *     wkday        = "Mon" | "Tue" | "Wed"
 *                  | "Thu" | "Fri" | "Sat" | "Sun"
 *
 *     weekday      = "Monday" | "Tuesday" | "Wednesday"
 *                  | "Thursday" | "Friday" | "Saturday" | "Sunday"
 *
 *     month        = "Jan" | "Feb" | "Mar" | "Apr"
 *                  | "May" | "Jun" | "Jul" | "Aug"
 *                  | "Sep" | "Oct" | "Nov" | "Dec"
 *
 * However, for the sake of robustness (and Netscapeness), we ignore the
 * weekday and anything after the time field (including the timezone).
 *
 * This routine is intended to be very fast; 10x faster than using sscanf.
 *
 * Originally from Andrew Daviel <andrew@vancouver-webpages.com>, 29 Jul 96
 * but many changes since then.
 *
 */
#ifndef BAD_DATE
#define BAD_DATE 0
#endif
API_EXPORT(time_t) BackupparseHTTPdate(const char *pszDate);

/**
 * For example  :
 * pszTimeString    :   "2003-12-23 12:32:59"
 */
API_EXPORT(long) BackupParseTimeFromString(char *pszTimeString);

/**
 * GetErrorMessage - get error msg of this error num.
 *
 * Called by agent staging server.
 */
API_EXPORT(const char *) BackupGetErrorMessage(int nError);

/* Hash function from Chris Torek. */
API_EXPORT(unsigned long) BackupStringHash(const void *keyarg, register unsigned long len);

#ifndef WIN32
API_EXPORT(char *)itoa (int val, char *buf, unsigned radix);
#endif
API_EXPORT(char *)i64toa (int64 val, char *buf, unsigned radix);

#ifdef __cplusplus
}
#endif

#endif // ( WAVETOP_BACKUP_BASE_FUNCTION_H )