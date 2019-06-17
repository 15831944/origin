/** =========================================================================
 ** Copyright (c) 2003 WaveTop Information Corp. All rights reserved.
 **
 ** The Mirror system
 **
 ** =========================================================================
 */

/**
 * @brief time library
 * @author lexiongjia
 * @file libs/include/mitime.h
 */

#ifndef _MITIME_H_
#define _MITIME_H_ 1

WTMI_BEGIN_DECLS

/*
 * @desc get CRun-Time lib time
 * @file mitime.cpp
 *
 * @param [in] unsigned long nYear
 *      year (1900 - 2XXX) 
 *
 * @param [in] unsigned long nMonth
 *      month (1 - 12)
 *
 * @param [in] unsigned long nDay
 *      day (1 - 31)
 *
 * @param [in] unsigned long nHour
 *      hour (0 - 23)
 *
 * @param [in] unsigned long nMinute
 *      minute (0 - 59)
 *
 * @param [in] unsigned long nSecond
 *      second (0 - 59)
 *
 * @param [out] time_t *pnResult
 *      CRun-Time library time
 * 
 * @ret if succeeds return WAVETOP_INDE_OK, else return status code
 *
 */
WTMI_LIBMI_EXPORT_(int) MiGetCTime32(unsigned long nYear, 
        unsigned long nMonth, unsigned long nDay, unsigned long nHour, 
        unsigned long  nMinute, unsigned long  nSecond, time_t *pnResult);

#ifdef WIN32
/**
 * @defgroup Win32 time <==> Unix time convert functions
 * @{
 */

/**
 * @desc convert windows system time => unix time
 * @file mitime.cpp
 *
 * @param pSysTime  [in]
 *          windows system time
 *
 * @ret unix time
 */
WTMI_LIBMI_EXPORT_(time_t) MiUnixTimeFromSystemTime(
                                const SYSTEMTIME *pSysTime);

/**
 * @desc convert unix time => windows system time
 * @file mitime.cpp
 *
 * @param nUnixTime [in]
 *          unix time
 *
 * @param pSysTime  [in/out]
 *          for receive windows system time
 *
 * @ret nothing
 */
WTMI_LIBMI_EXPORT_(void) MiUnixTimeToSystemTime(time_t nUnixTime, 
                                                SYSTEMTIME* pSysTime);

/**
 * @desc convert windows file time => unix time
 * @file mitime.cpp
 *
 * @param pFileTime [in]
 *          windows file time
 *
 * @ret nothing
 */
WTMI_LIBMI_EXPORT_(time_t) MiUnixTimeFromFileTime(const FILETIME *pFileTime);

/**
 * @desc convert unix time ==> windows file time
 * @file mitime.cpp
 *
 * @param pFileTime [in]
 *          windows file time
 *
 * @ret nothing
 */
WTMI_LIBMI_EXPORT_(void) MiUnixTimeToFileTime(time_t nUnixTime, 
                                              FILETIME *pFileTime);

/** @} */

#endif /* !defined(WIN32) */

WTMI_END_DECLS

#endif /* !defined(_MITIME_H_) */
