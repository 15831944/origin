/** =============================================================================
 ** Copyright (c) 2007 WaveTop Information Corp. All rights reserved.
 **
 ** The Mirror system
 **
 ** =============================================================================
 */

#ifndef __WAVETOP_MIRROR_ODBC_COMMON_HEADER_
#define __WAVETOP_MIRROR_ODBC_COMMON_HEADER_

#include <sql.h>
#include <sqlext.h>
#include <odbcss.h>

#define WAVETOP_SQL_HANDLE_HENV      0
#define WAVETOP_SQL_HANDLE_HDBC      1
#define WAVETOP_SQL_HANDLE_HSTMT     2

#define WAVETOP_SQL_COMMIT           0
#define WAVETOP_SQL_ROLLBACK         1

#define WAVETOP_BACKUP_OK 0
#define  WAVETOP_BACKUP_INTERNAL_ERROR 3;
#define  WB_USER_MESSAGE 0

/*  回调消息定义 */
#define WAVETOP_BACKUP_SYSTEM_BUSY          WB_USER_MESSAGE +   0
#define WAVETOP_BACKUP_CONNECT_DOWN         WB_USER_MESSAGE +   1
#define WAVETOP_BACKUP_NO_MEMORY            WB_USER_MESSAGE +   2
#define WAVETOP_BACKUP_FILE_NOT_INTGRETY    WB_USER_MESSAGE +   3
#define WAVETOP_BACKUP_FILTERED             WB_USER_MESSAGE +   4
#define WAVETOP_BACKUP_RATE                 WB_USER_MESSAGE +   5
#define WAVETOP_BACKUP_BEGIN                WB_USER_MESSAGE +   6
#define WAVETOP_BACKUP_END                  WB_USER_MESSAGE +   7
#define WAVETOP_BACKUP_OPEN_FILE_ERROR      WB_USER_MESSAGE +   8
#define WAVETOP_BACKUP_CREAT_FILE_ERROR     WB_USER_MESSAGE +   9
#define WAVETOP_BACKUP_OPEN_DIR_ERROR       WB_USER_MESSAGE +  10
#define WAVETOP_BACKUP_CREAT_DIR_ERROR      WB_USER_MESSAGE +  11
#define WAVETOP_BACKUP_FILE_NOT_EXIST       WB_USER_MESSAGE +  12
#define WAVETOP_BACKUP_INVALID_SYNTAX       WB_USER_MESSAGE +  13
#define WAVETOP_BACKUP_CONNECT_SUCCESS      WB_USER_MESSAGE +  14
#define WAVETOP_BACKUP_FSDETECT_FAILED      WB_USER_MESSAGE +  15
#define WAVETOP_BACKUP_DETECT_FILE          WB_USER_MESSAGE +  16
#define WAVETOP_BACKUP_RECEIVE_PULL         WB_USER_MESSAGE +  17
/* 多次都不能连接服务器：可能因为服务器down或网络故障 */
#define WAVETOP_BACKUP_CONNECT_DOWN_2       WB_USER_MESSAGE +  18

#define WAVETOP_BACKUP_NO_FREE_ROOM         WB_USER_MESSAGE +  19
#define WAVETOP_BACKUP_OVER_QUOTA           WB_USER_MESSAGE +  20
#define WAVETOP_BACKUP_OVER_REQ             WB_USER_MESSAGE +  25

#define WAVETOP_BACKUP_MODULE_NOT_LOAD      WB_USER_MESSAGE +  30

#define WAVETOP_BACKUP_QUIT                 WB_USER_MESSAGE +  31

#define WAVETOP_BACKUP_OCI_NO_DATA          WB_USER_MESSAGE +  32

typedef struct _CONNECTION_MSSQL_ST {
    char *pszInstance;
    char *pszIP;
    char *pszUser;
    char *pszPassword;
    int nPort;
} MiSQLConnectSt;

class CSqlConnect {
public:

    CSqlConnect();
    ~CSqlConnect();

public:

    int Connect();

    SQLHSTMT Allochstmt();

    void FreeHstmt(SQLHSTMT hStmt);

    int SetCommitAttribute(int nSwitch);

    int SetRowCount(int nRowCount);

    /* 重连SQL Server 服务器 */
    int ReConnect();

    void Close();

    int SetEnv(const char *pszInstance, const char *pszIP, int nPort,
        const char *pszUser, const char *pszPassword, const char *pszDBName = NULL);

    int GetODBCErr(int nHandleType, SQLHSTMT hStmt, SQLCHAR *Sqlstate, SQLINTEGER *NativeError, 
        SQLCHAR *MessageText, SQLSMALLINT BufferLength, SQLSMALLINT *TextLength);

    int EndTran(int nHandleType, int nOperation);

private:
    void Clear();

private:

    /* ODBC环境句柄 */
    SQLHENV m_hEnv;

    /* ODBC连接句柄 */
    SQLHDBC m_hDbc;

    char m_szInstance[256];

    char m_szIP[16];

    char m_szUser[256];

    char m_szPassword[256];

    char m_szDBName[256];

    int m_nPort;
};

#endif