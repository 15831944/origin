/** =============================================================================
 ** Copyright (c) 2003 WaveTop Information Corp. All rights reserved.
 **
 ** The Mirror system
 **
 ** =============================================================================
 */

#ifndef _LIBMIORL_H_
#define _LIBMIORL_H_ 1

#include "oci.h"

/**
 * @defgroup oracle export/import functions
 * @{
 */

#define WTMI_ORA_SRV_MIRROR_MODE_EXP_IMP_FULL 2

int MiOrlExp(MiOrlService *pService, MiOrlExpOpt *pExpOpt);

int MiOrlImp(MiOrlService *pService, MiOrlImpOpt *pImpOpt);

/** @} */

/**
 * @defgroup oci middleman functions
 * @{
 */

/**
 * @desc Oracle oci library initialize
 * @file mioci_func.cpp
 * @note the ::MiOrlInit() function must be the first oci function 
 *       called by an application or DLL.
 * @fixme current library version not transfer oci initialize parameter.
 *        current ::MiOrlInit() function using OCI_OBJECT initialize parameter.
 *
 * @ret if the function succeeds, the return value is WAVETOP_BACKUP_OK,
 *      otherwise return a mirror status code, see mirror.h
 */
int MiOrlInit(void);

/**
 * @desc Oracle oci library terminate
 * @file mioci_func.cpp
 * @note the ::MiOrlInit() function must be the first oci function 
 *       called by an application or DLL.
 *
 * @ret if the function succeeds, the return value is WAVETOP_BACKUP_OK,
 *      otherwise return a mirror status code, see mirror.h
 */
int MiOrlUnInit(void);

int MiOrlOpenHandle(MiOrl **pOrl);

int MiOrlCloseHandle(MiOrl *pOrl);

int MiOrlLogon(MiOrl *pOrl, MiOrlService *pSession);

int MiOrlLogout(MiOrl *pOrl);

int MiOrlNoResultSQLExec(MiOrl *pOrl, int nZeroIsCommit, char *pszSqlCmd);

int MiOrlGetOCISvcCtx(MiOrl *pOrl, OCISvcCtx **pOCISvcCtx);

int MiOrlGetOCIEnv(MiOrl *pOrl, OCIEnv **pOCIEnv);

int MiOrlGetOCIError(MiOrl *pOrl, OCIError **pOCIError);

int MiOrlGetOCILastError(MiOrl *pOrl, sword *pnOCIError, 
                         char *pszErrorDesc, unsigned long nBufSize);

int MiOrlGetOCISetError(MiOrl *pOrl, sword nOCIError);

int MiOrlStrToOCINum(MiOrl *pOrl, char *pszNumStr, OCINumber *pOCINum);

int MiOrlOCINumToStr(MiOrl *pOrl, OCINumber *pOCINum, 
                     char *pszNumStr, unsigned long nNumStrBufSize);


int MiOrlLogon2(MiOrl *pOrl, MiOrlService *pSession);
int MiOrlLogout2(MiOrl *pOrl);

int MiOrlConnection(MiOrl *pOrl, MiOrlService *pSession);
int MiOrlDisconnection(MiOrl *pOrl);

void MiOrlExpExceptionCallback(MiOrlExpOpt *pExpOpt, int nStatus, 
                                   char *pszMsgFmt, ...);
/** @} */

/**
 * @defgroup oracle LogMnr middleman functions
 * @{
 */

/**
 * @desc get logmnr condtion from pCond.szTable
 * 
 */
int MiOrlGetLogMnrCond(MiOrlService *pSession, MiOrl *pOra, MiOrlLogMnrCond *pCond);

/**
 * @desc oracle redo log dump method entry
 * @file miorl_redo.cpp
 *
 * @param pSession [in]
 *          connection oracle session for write error log
 * @param pOra [in]
 *          connection oracle handle
 * @param pOpt [in]
 *
 * @param pDumpResult [in/out]
 *          dump redo log result description
 *
 * @ret if the function succeeds, the return value is WAVETOP_BACKUP_OK,
 *      otherwise return a mirror status code, see mirror.h
 */
int MiOrlRedoLogDump(MiOrlService *pSession, MiOrl *pOra, MiOrlRedoLogDumpOpt *pOpt,
                     MiOrlRedoLogDumpResult **pDumpResult);

int MiOrlRedoLogDumpEx(MiOrlService *pSession, MiOrl *pOra, MiOrlRedoLogDumpOpt *pOpt,
                     MiOrlRedoLogDumpResult **pDumpResult);

/**
 * @desc free oracle dump method result
 * @file miorl_redo.cpp
 *
 * @param pDumpResult [in/out]
 *          dump redo log result description
 *
 * @ret if the function succeeds, the return value is WAVETOP_BACKUP_OK,
 *      otherwise return a mirror status code, see mirror.h
 */
int MiOrlRedoLogDumpResultFree(MiOrlRedoLogDumpResult *pDumpResult);

/** @} */

/**
 * @defgroup other oracle middleman functions
 * @{
 */

/**
 * @desc create Oracle database dictionary file
 * @file miorl_redo.cpp
 */
int MiOrlRedoLogOracleDictFileCreate(MiOrl *pOra, char *pszDictFilename);

/**
 * @desc oracle utl_file_dir query
 * @file miorl_redo.cpp
 */
int MiOrlRedoLogOracleUtlDirGet(MiOrl *pOra, MiOrlOCIFailedCallBack pOCIFailedDump, 
                                char *pszBuf, unsigned long nBufSize);

/** 
 * @desc get all user and table in database
 * @res username:table
 */
int MiOrlGetTableAndOwners(MiOrl *pOra, MiOraDBTables **pDBTables);
int MiOrlGetDBOwners(MiOrl *pMiOra, MiOraDBTables **pDBTables);
int MiOrlGetDBOwnersTables(MiOrl *pMiOra, MiOraDBTables **pDBTables, char *pszOwner);
int MiOrlGetDBInfo(MiOrl *pMiOra, MiOraDBInfo *pDBInfo);
/** @}*/

/**
 * @desc oracle mangment funcs
 * @
 */

int MiOraAlertDBToArchivelog(char *pszSysName, char *pszPsw, char *pszDBServer);

int MiOraAlertSPFILEForDic(char *pszSysName, char *pszPsw, char *pszDBServer, char *pszDicFilePath);

int MiOraStartup(MiOrl *pOra);
/** @} */

/** @desc query database marker from database view and oci interface */
int MiOrlRedoLogOracleDBMarkerQuery(MiOrl *pOra, MiOraDBMarker *pDBMarker);

int MiOrlRedoLogOracleCurSCNQuery(MiOrl *pOra, char *pszSCN, 
                                  unsigned long nBufSize);

/** @} */

#endif /* !defined(_LIBMIORL_H_) */
