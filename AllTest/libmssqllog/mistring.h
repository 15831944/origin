/** ========================================================================
 ** Copyright (c) 2003 WaveTop Information Corp. All rights reserved.
 **
 ** The Mirror system
 **
 ** ========================================================================
 */

#ifndef _MISTRING_H_
#define _MISTRING_H_ 1

#include <stdio.h>
#include <stdlib.h>

WTMI_BEGIN_DECLS

WTMI_LIBMI_EXPORT_(char*) MiSubString(char *result, int size, 
        const char *start, int len, char quote);

WTMI_LIBMI_EXPORT_(char*) MiGetWord(char *result, 
                                    int size, const char **line);

WTMI_LIBMI_EXPORT_(char **) MiSplitString(char *pszString, long *pnCount);
WTMI_LIBMI_EXPORT_(int) MiFreeSplitString(char **pszString, long nCount);


WTMI_LIBMI_EXPORT_(int) MiCfgGetline(char *buf, size_t bufsize, 
                                     int opt, FILE *file);

WTMI_LIBMI_EXPORT_(char) *MiCfgGetWord(char *result, int size, 
                                       const char **line);

WTMI_LIBMI_EXPORT_(int) MiSafeStrncpy(char *pszDest, char *pszSrc, 
                                      unsigned long nDestsize);

WTMI_LIBMI_EXPORT_(char *) MiCfgGetWord2(char *pszResultBuf, 
										 unsigned int nResultBufSize, 
										 const char **pszLine, 
								         char cCeparatorStart,
										 char cCeparatorEnd);

WTMI_END_DECLS

#endif /* !defined(_MISTRING_H_) */
