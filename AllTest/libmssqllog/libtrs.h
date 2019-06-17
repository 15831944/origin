/** ==========================================================================
 ** Copyright (c) 2003 WaveTop Information Corp. All rights reserved.
 **
 ** The Mirror system
 **
 ** ==========================================================================
 */

#ifndef _LIBTRS_H_
#define _LIBTRS_H_ 1

#include "libmirror_types.h"
#include "trs_def.h"

typedef struct _MI_TRS_CONN
{
    char szSrv[512];
    char szSrvPort[128];
    char szUserName[128];
    char szPsw[128];
} MiTrsConn;

typedef struct _MI_TRS_CLIENT
{
    MiTrsConn ConnDesc;
    HTRSCLIENT hTrsClient;
    INT nLastTrsErr;
} MiTrsClient;

typedef struct _MiTrsClientBkOpt
{
    const char *pszDataBase;    /* backup database name */
    const char *pszDevice;      /* backup to filename */
} MiTrsClientBkOpt;

typedef struct _MiTrsClientRvOpt
{
    const char *pszDatabase;
    const char *pszDevice;
    const char *pszDirectory;
    char szRvDbName[128];
} MiTrsClientRvOpt;

typedef struct _MiTrsClientRnOpt
{
    char *pszOldDatabase;
    char *pszNewDatabase;
} MiTrsClientRnOpt;

typedef struct _MiTrsClientDelOpt
{
    char *pszDatabase;
} MiTrsClientDelOpt;

WTMI_BEGIN_DECLS

WTMI_LIBMI_EXPORT_(MiTrsClient *) MiTrsClientStartup(MiTrsConn *pTrsConn);
WTMI_LIBMI_EXPORT_(void)          MiTrsClientCleanup(MiTrsClient *pTrsClient);


WTMI_LIBMI_EXPORT_(int)           MiTrsClientBackup(MiTrsClient *pTrsClient, 
                                                    MiTrsClientBkOpt *pBkOpt);


WTMI_LIBMI_EXPORT_(int)           MiTrsClienRecover(MiTrsClient *pTrsClient, 
                                                    MiTrsClientRvOpt *pRvOpt);


WTMI_LIBMI_EXPORT_(int)           MiTrsClienRename(MiTrsClient *pTrsClient,
                                                   MiTrsClientRnOpt *pRnOpt);


WTMI_LIBMI_EXPORT_(int)           MiTrsClienDelete(MiTrsClient *pTrsClient, 
                                                   MiTrsClientDelOpt *pDelOpt);

#define WT_TRSERR_MSG_LEN   512
WTMI_LIBMI_EXPORT_(int)           MiTrsClienErrStr(MiTrsClient *pTrsClient, 
                                                   char *pszErrStr, 
                                                   unsigned int nBufSize);

WTMI_END_DECLS

#endif /* !defined(_LIBTRS_H_) */
