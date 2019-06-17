/** ========================================================================
 ** Copyright (c) 2003 WaveTop Information Corp. All rights reserved.
 **
 ** The Mirror system
 **
 ** ========================================================================
 */

#ifndef _MIALG_H_
#define _MIALG_H_ 1

#include "nspr.h"

#define WTMI_LIBALG_MAX_HMAC_LEN        128

WTMI_BEGIN_DECLS

WTMI_LIBMI_EXPORT_(int) MiGetFileHMAC(char *pszFilename, 
                                      char *pszHMAC, 
                                      unsigned long nBufSize);

WTMI_LIBMI_EXPORT_(PRInt64) MiBinSearch(void *ppBase[], 
        unsigned long nCount, void *pSearchNode,
        int ( *pFuncCompare )(const void *pArg1, 
        const void *pArg2, const void *pReserve));


WTMI_LIBMI_EXPORT_(void) MiHeapSort(void *ppBase[], 
        unsigned long nCount, int ( *pFuncCompare )(const void *pArg1, 
        const void *pArg2, const void *pReserve));

WTMI_END_DECLS

#endif /* !defined(_MIALG_H_) */
