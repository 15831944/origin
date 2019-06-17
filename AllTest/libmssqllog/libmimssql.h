/** =============================================================================
 ** Copyright (c) 2003 WaveTop Information Corp. All rights reserved.
 **
 ** The Mirror system
 **
 ** =============================================================================
 */

/**
 * @desc ms sql mirror library export function
 * @author lexiongjia
 * @file libmimssql.h
 */

/**
 * @remark library compile
 *
 * To gain access to free threaded COM, you'll need to define _WIN32_DCOM
 * before the system headers, either in the source (as in this example),
 * or when invoking the compiler (by using /D "_WIN32_DCOM")
 *
 * example:
 *
 *  #define _WIN32_DCOM     // Note: _WIN32_DCOM must be defined during the compile
 *  #include <objbase.h>
 *  #include <windows.h>
 *  #include <stdio.h>
 *  #include <stdlib.h>
 *  #include <string.h>
 *  #include "libmimssql_types.h"
 *  #include "libmimssql.h"
 *  
 *  int main(int argc, char **argv)
 *  {
 *      HRESULT hResult;
 *
 *      hResult = CoInitializeEx(NULL, COINIT_MULTITHREADED);
 *      if (!SUCCEEDED(hResult)) {
 *          fprintf(stderr, "Coinit fails: x%X\n", hResult);
 *          exit(1);
 *      }
 *
 *      // codes ... 
 *
 *      CoUninitialize();
 *      return 0;
 *  }
 *
 */

#ifndef _LIBMIMSSQL_H_ 
#define _LIBMIMSSQL_H_ 1

typedef struct MI_INST_INFO {
    char *pszIP;
    char *pszInstName;
    char *pszDBName;
    int   nPort;
    char *pszUser;
    char *pszPSW;
}MiInstInfo;

/**
 * @defgroup interface layer functions
 * @{
 */

/**
 * @file libmimssql.cpp
 * @desc export mssql data to file
 *
 * @param pExport
 *      [in/out] pointer to export operations, see libmimssql_types.h
 * @param pCallbackReserver
 *      [in] for transfer data to callback functions
 *
 * @ret if the function succeeds, the return value is WAVETOP_BACKUP_OK,
 *      otherwise return a mirror status code, see mirror.h
 */
int MiMSSqlExport(MSSqlExportOpt *pExport, void *pCallbackReserver, void *pMssqlParame);

int MiMSSqlExportEx(CSqlConnect *pSqlConn, MSSqlExportOpt *pExport,
    void *pCallbackReserver, void *pMssqlParame);

void SetQuit(PRInt32 *nQuit);

typedef struct MIRROR_QUIT {
	PRInt32 *nQuit;
}MIRROR_QUIT;

typedef struct MI_INC_BAK_INFO {
    PRInt64 nBakSize;
    void *pBakAddr;
}MiIncBakInfo;

typedef struct MI_EXEC_PARAM {
    char            *pszSQL;
    void            *pCallbackReserver;
    MiInstInfo      *pInstinfo;
    CSqlConnect     *pSqlConn;
    MiMSSqlOperate   nOpt;
}MiExecParam;

extern MIRROR_QUIT g_stQuit;
/**
 * @file libmimssql.cpp
 * @desc import mssql data from file
 *
 * @param pImport
 *      [in/out] pointer to import operations, see libmimssql_types.h
 * @param pCallbackReserver
 *      [in] for transfer data to callback functions
 *
 * @ret if the function succeeds, the return value is WAVETOP_BACKUP_OK,
 *      otherwise return a mirror status code, see mirror.h
 */
int MiMSSqlImport(MSSqlImportOpt *pImport, MiIncBakInfo *pIncBakInfo, void *pCallbackReserver);

int MiMSSqlImportEx(MSSqlImportOpt *pImport, void *pCallbackReserver, MiIncBakInfo *pIncBakInfo,
    CSqlConnect *pSqlConn, MiInstInfo *pInstInfo);

void ProcessMessages (
    SQLSMALLINT     handle_type,    // ODBC handle type
    SQLHANDLE       handle,         // ODBC handle
    int             ConnInd,        // TRUE if sucessful connection made
    bool*            pBackupSuccess);
/**
 * @file libmimssql.cpp
 * @desc get library export/import files with 
 *       device prefix name and stream count
 * @remark using ::MiMSSqlFreeOptFileList() free result
 *
 * @param pszPrefixName
 *      [in] export/import function's virtual device prefix name
 * @param nStreamCount
 *      [in] export/import function's data stream count name
 * @param pOptFiles
 *      [out] operation files list, see libmimssql_types.h
 * @param pCallbackReserver
 *      [in] for transfer data to callback functions
 *
 * @ret if the function succeeds, the return value is WAVETOP_BACKUP_OK,
 *      otherwise return a mirror status code, see mirror.h
 */
int MiMSSqlGetOptFiles(char *pszPrefixName, 
                       int nStreamCount, 
                       MSSqlOptFileList **pOptFiles, 
                       void *pCallbackReserver);

/**
 * @file libmimssql.cpp
 * @desc release ::MiMSSqlGetOptFiles() function result
 *
 * @param pOptFiles
 *      [in] operation files list, 
 *      see ::MiMSSqlGetOptFiles() and libmimssql_types.h
 * @param pCallbackReserver
 *      [in] for transfer data to callback functions
 *
 * @ret if the function succeeds, the return value is WAVETOP_BACKUP_OK,
 *      otherwise return a mirror status code, see mirror.h
 */
int MiMSSqlFreeOptFileList(MSSqlOptFileList *pOptFiles, 
                           void *pCallbackReserver);

/**
 * @file libmimssql.cpp
 * @desc set mssql library callback functions
 *
 * @param pWriteLogFunc
 *      [in] pointer to write log function, see libmimssql.h
 * @param pStatus
 *      [in] pointer to status message function, see libmimssql.h
 * @param pTerminate
 *      [in] pointer to terminate callback function, see libmimssql.h
 *
 * @ret if the function succeeds, the return value is WAVETOP_BACKUP_OK,
 *      otherwise return a mirror status code, see mirror.h
 */
int MiMSSqlSetCallbackFunc(MiMSSqlLogCallback pWriteLogFunc, 
                           MiMSSqlStatusCallback pStatus, 
                           MiMSSqlTerminateCallback pTerminate,
                           MiMSSqlLogCallback pWriteErrLogFunc);

/** @} */

/**
 * @file libmimssql.cpp
 * @create the Db if it is not exit
 *
 * @[in] MSSqlServ contains (server name, user id, user pwd)
 * @[in] db name
 * @[in] time out
 *
 * @ret if the function succeeds, the return value is WAVETOP_BACKUP_OK,
 *      otherwise return a mirror status code, see mirror.h
 */

int MiMSSqlCreateDb(MSSqlServ *pSqlServ, char *pszDbName, unsigned long nTimeOut);

/**
 * @file libmimssql.cpp
 * @delete the Db if it is not exit
 *
 * @[in] MSSqlServ contains (server name, user id, user pwd)
 * @[in] db name
 * @[in] time out
 *
 * @ret if the function succeeds, the return value is WAVETOP_BACKUP_OK,
 *      otherwise return a mirror status code, see mirror.h
 */

int MiMSSqlDeleteDb(MSSqlServ *pSqlServ, char *pszDbName, unsigned long nTimeOut);

int MiMSSqlGetNameFromDevice(MSSqlServ *pSqlServ, char *pszDeviceName, unsigned long nTimeOut,
							 MiMSSqlstDbString **pstDbFileName, MiMSSqlstDbString **pstDblogName);
int MiMSSqlGetNameFromDB(MSSqlServ *pSqlServ, char *pszDbName, unsigned long nTimeOut,
							 char *pszDbFileName, char *pszDblogName);
int MiMSSqlFreeStDbString(MiMSSqlstDbString *pstString);

/**
 * @file libmimssql.cpp
 * @create the Db if it is not exit
 *
 * @[in] MSSqlServ contains (server name, user id, user pwd)
 * @[in] time out
 * @[in] the size of pszDbVersion
 * @[out] the sql version
 * @ret if the function succeeds, the return value is WAVETOP_BACKUP_OK,
 *      otherwise return a mirror status code, see mirror.h
 */

int MiMSSqlDbVersion(MSSqlServ *pSqlServ, unsigned long nTimeOut, 
					 char *pszDbVersion, unsigned long nSize,
					 unsigned long *nVersionMajor,
					 unsigned long *nVersionMinor);

/**
 * @file libmimssql.cpp
 * @desc mssql server database do DBCC 
 *
 * @param pDBCCOpt
 *      [in/out] pointer to DBCC operations, see libmimssql_types.h
 * @param pCallbackReserver
 *      [in] for transfer data to callback functions
 *
 * @ret if the function succeeds, the return value is WAVETOP_BACKUP_OK,
 *      otherwise return a status code, see mirror.h
 */
int MiMSSqlDBCC(MSSqlExportOpt *pDBCCOpt, void *pCallbackReserver);

/**
 * @ file libmimssql.cpp   
 * @ desc Check backuped set of database 
 * @[in]
 * pExport - MSSqlExportOpt structure, see libmimssql_types.h
 * pList - The list of backuped files
 *
 * @[out]
 * pnRes - The result of check database integrity
 *
 * return WAVETOP_BACKUP_OK, if successful.
 * otherwise, return error code.
 */
int MssqlChkDBIntegrity(MSSqlExportOpt *pExport,
                                     BakHistroyInfoSt *pList,
                                     long *pnRes,
                                     char *pszInfo,
                                     int nSize);
/**
 * @ file libmimssql.cpp   
 * @ desc Check backuped set of database 
 * @[in]
 * pFileList - The list of backuped files
 *
 * @[out]
 * pszBinary - Get the Backup set key.
 *
 * return WAVETOP_BACKUP_OK, if successful.
 * otherwise, return error code.
 */
int MiMssqlGetBinaryKey(MSSqlExportOpt *pExport, 
                 unsigned char *pszBinary, 
                 int nSize);

/**
 * @ file mssqlimplement.cpp   
 * @ reconnect to database 
 * @[in]
 * connection handle
 *
 * @[out]
 *
 * return WAVETOP_BACKUP_OK, if successful.
 * otherwise, return error code.
 */

int MiMSSqlReconnDB(CSqlConnect *pSqlConn);

int MiMSSqlIsDBOkay(void *pCallbackReserver, MiMSSqlOperate  nOpt,
    MiInstInfo *pInstInfo, int *pnCtrl);

#define MI_MSSQLSERVER_EXECUTE          "OSQL.EXE"
#define MI_MSSQLSERVER_VDI_TIMEOUT      100
#define MI_MSSQLSERVER_VDI_SQLTOOLONG   101  //错误：SQL语句过长，超过1024字节
#endif /* !defined(_LIBMIMSSQL_H_)  */
