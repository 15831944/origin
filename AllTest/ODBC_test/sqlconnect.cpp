/** =============================================================================
 ** Copyright (c) 2008 WaveTop Information Corp. All rights reserved.
 **
 ** The Mirror system
 **
 ** =============================================================================
 */

/* ODBC连接类 */

#ifndef __WAVETOP_MIRROR_ODBC_CONECT_
#define __WAVETOP_MIRROR_ODBC_CONECT_

#include <windows.h>
#include <stdio.h>
#include "sqlcommhdr.h"
//#include "mirror.h"

CSqlConnect::CSqlConnect()
{
    m_hEnv = SQL_NULL_HENV;
    m_hDbc = SQL_NULL_HDBC;
    m_szInstance[0] = '\0';
    m_szIP[0] = '\0';
    m_szUser[0] = '\0';
    m_szPassword[0] = '\0';
}

CSqlConnect::~CSqlConnect()
{
    Close();
}

void CSqlConnect::Close()
{
    if (m_hDbc != SQL_NULL_HDBC) {
        SQLDisconnect(m_hDbc);
        SQLFreeHandle(SQL_HANDLE_DBC, m_hDbc);
        m_hDbc = SQL_NULL_HDBC;
    }

    if (m_hEnv != SQL_NULL_HENV) {
        SQLFreeHandle(SQL_HANDLE_ENV, m_hEnv);
        m_hEnv = SQL_NULL_HENV;
    }
}

void CSqlConnect::Clear()
{
    m_szInstance[0] = '\0';
    m_szIP[0] = '\0';
    m_szUser[0] = '\0';
    m_szPassword[0] = '\0';

    Close();
}

SQLHSTMT CSqlConnect::Allochstmt()
{
    SQLRETURN   hRecorde;
    SQLHSTMT hStmt;

    if (SQL_NULL_HDBC == m_hDbc || SQL_NULL_HENV == m_hEnv) {
        return SQL_NULL_HSTMT;
    }

    hRecorde = SQLAllocHandle(SQL_HANDLE_STMT, m_hDbc, &hStmt); 
    if (hRecorde != SQL_SUCCESS &&
        hRecorde != SQL_SUCCESS_WITH_INFO) {
        return SQL_NULL_HSTMT;
	}

    return hStmt;
}

void CSqlConnect::FreeHstmt(SQLHSTMT hStmt)
{
    if (hStmt != SQL_NULL_HSTMT) {
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        hStmt = SQL_NULL_HSTMT;
    }
}

int CSqlConnect::SetCommitAttribute(int nSwitch)
{
    if (SQL_NULL_HDBC == m_hDbc || SQL_NULL_HENV == m_hEnv) {
        return WAVETOP_BACKUP_INVALID_SYNTAX;
    }

    if (nSwitch) {
        SQLSetConnectAttr(m_hDbc, SQL_ATTR_AUTOCOMMIT, SQL_AUTOCOMMIT_OFF, 0);
    }
    else {
        SQLSetConnectAttr(m_hDbc, SQL_ATTR_AUTOCOMMIT, (void *)SQL_AUTOCOMMIT_ON, 0);
    }

    return WAVETOP_BACKUP_OK;
}

int CSqlConnect::SetRowCount(int nRowCount)
{
    SQLRETURN rc;
    SQLHSTMT hStmt;
    char szSQL[64];

    if (SQL_NULL_HDBC == m_hDbc || SQL_NULL_HENV == m_hEnv || nRowCount < 0) {
        return WAVETOP_BACKUP_INVALID_SYNTAX;
    }

    hStmt = Allochstmt();
    if (SQL_NULL_HSTMT == hStmt) {
        return WAVETOP_BACKUP_INVALID_SYNTAX;
    }

    sprintf(szSQL, "SET ROWCOUNT %d", nRowCount);

    rc = SQLExecDirect(hStmt, (SQLCHAR *)szSQL, SQL_NTS);
    FreeHstmt(hStmt);
    if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO) {
        return WAVETOP_BACKUP_INTERNAL_ERROR;
    }

    return WAVETOP_BACKUP_OK;
}

int CSqlConnect::SetEnv(const char *pszInstance, const char *pszIP, int nPort,
        const char *pszUser, const char *pszPassword, const char *pszDBName)
{
    if (NULL == pszInstance ||
        NULL == pszIP ||
        NULL == pszUser ||
        NULL == pszPassword) {
        return WAVETOP_BACKUP_INVALID_SYNTAX;
    }

    if ((strlen(pszInstance) >= sizeof(m_szInstance)) ||
        (strlen(pszIP) >= sizeof(m_szIP)) ||
        (strlen(pszUser) >= sizeof(m_szUser)) ||
        (strlen(pszPassword) >= sizeof(m_szPassword))) {
        return WAVETOP_BACKUP_INVALID_SYNTAX;
    }

    strcpy(m_szInstance, pszInstance);
    strcpy(m_szIP, pszIP);
    strcpy(m_szUser, pszUser);
    strcpy(m_szPassword, pszPassword);
    if (pszDBName == NULL)
        m_szDBName[0] = '\0';
    else
        strncpy(m_szDBName, pszDBName, sizeof(m_szDBName));
        
    m_nPort = nPort;

    return WAVETOP_BACKUP_OK;
}

int CSqlConnect::ReConnect()
{
    if ('\0' == m_szInstance[0] ||
        '\0' == m_szIP[0] ||
        '\0' == m_szUser[0] ||
        '\0' == m_szPassword[0]) {
        return WAVETOP_BACKUP_INVALID_SYNTAX;
    }

    Close();

    return Connect();
}

int CSqlConnect::GetODBCErr(int nHandleType, SQLHSTMT hStmt, SQLCHAR *Sqlstate, SQLINTEGER *NativeError, 
               SQLCHAR *MessageText, SQLSMALLINT BufferLength, SQLSMALLINT *TextLength)
{
    int nRet = WAVETOP_BACKUP_OK;

    if (SQL_NULL_HDBC == m_hDbc || 
        SQL_NULL_HENV == m_hEnv) {
        return WAVETOP_BACKUP_INVALID_SYNTAX;
    }

    switch (nHandleType) {
    case WAVETOP_SQL_HANDLE_HENV:
        SQLError(m_hEnv, NULL, NULL, Sqlstate, NativeError, MessageText, BufferLength, TextLength);
        break;
    case WAVETOP_SQL_HANDLE_HDBC:
        SQLError(NULL, m_hDbc, NULL, Sqlstate, NativeError, MessageText, BufferLength, TextLength);
        break;
    case WAVETOP_SQL_HANDLE_HSTMT:
        SQLError(NULL, NULL, hStmt, Sqlstate, NativeError, MessageText, BufferLength, TextLength);
        break;
    default:
        nRet = WAVETOP_BACKUP_INVALID_SYNTAX;
        break;
    }

    return nRet;
}

int CSqlConnect::EndTran(int nHandleType, int nOperation)
{
    SQLRETURN hRecorde;
    int nRet = WAVETOP_BACKUP_OK;
    SQLSMALLINT CompletionType;

    if (SQL_NULL_HDBC == m_hDbc || 
        SQL_NULL_HENV == m_hEnv) {
        return WAVETOP_BACKUP_INVALID_SYNTAX;
    }

    switch (nOperation) {
    case WAVETOP_SQL_COMMIT:
        CompletionType = SQL_COMMIT;
        break;
    case WAVETOP_SQL_ROLLBACK:
        CompletionType = SQL_ROLLBACK;
        break;
    default:
        return WAVETOP_BACKUP_INVALID_SYNTAX;
    }
    
    switch (nHandleType) {
    case WAVETOP_SQL_HANDLE_HENV:
        hRecorde = SQLEndTran(SQL_HANDLE_ENV, m_hEnv, CompletionType);
        break;
    case WAVETOP_SQL_HANDLE_HDBC:
        hRecorde = SQLEndTran(SQL_HANDLE_DBC, m_hDbc, CompletionType);
        break;
    default:
        nRet = WAVETOP_BACKUP_INVALID_SYNTAX;
        break;
    }

    if (nRet == WAVETOP_BACKUP_OK && 
        hRecorde != SQL_SUCCESS && 
        hRecorde != SQL_SUCCESS_WITH_INFO) {
        nRet = WAVETOP_BACKUP_INTERNAL_ERROR;
    }
    
    return nRet;
}

int CSqlConnect::Connect()
{
    SQLRETURN   hRecorde;
    char        szConnOption[2048];
    SQLCHAR     szConnStrOut[1024];	
    char        szPort[16];
    SQLSMALLINT cbConnStrOut = 0;
    int         nRet = WAVETOP_BACKUP_OK;

    /* 清空现有连接 */
    Close();

    /* Allocate environment handle */
    hRecorde = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_hEnv);	
    if (hRecorde != SQL_SUCCESS && hRecorde != SQL_SUCCESS_WITH_INFO) {
        nRet = WAVETOP_BACKUP_INTERNAL_ERROR;
        goto CLEANUP;
    }

    /* Set the ODBC version environment attribute */
    hRecorde = SQLSetEnvAttr(m_hEnv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);		
    if (hRecorde != SQL_SUCCESS &&
        hRecorde != SQL_SUCCESS_WITH_INFO) {
        nRet = WAVETOP_BACKUP_INTERNAL_ERROR;
        goto CLEANUP;
    }

    /* Allocate connection handle */
    hRecorde = SQLAllocHandle(SQL_HANDLE_DBC, m_hEnv, &m_hDbc);	
    if (hRecorde != SQL_SUCCESS && hRecorde != SQL_SUCCESS_WITH_INFO) {
        nRet = WAVETOP_BACKUP_INTERNAL_ERROR;
        goto CLEANUP;
    }

    /* Set login timeout to 10 seconds. */
    SQLSetConnectAttr(m_hDbc, SQL_LOGIN_TIMEOUT, (void*)10, 0);

    _snprintf(szPort, sizeof(szPort), "%d", m_nPort);

    _snprintf(szConnOption, sizeof(szConnOption),
        "DRIVER={SQL Server};server=%s%s%s%s%s%s%s;uid=%s;pwd=%s", m_szIP,
        (stricmp(m_szInstance, "MSSQLServer") == 0 ? "" : "\\"),
        (stricmp(m_szInstance, "MSSQLServer") == 0 ? "" : m_szInstance),
        (szPort[0] == '0' || szPort[0] == '-' ? "" : ","), 
        (szPort[0] == '0' || szPort[0] == '-' ? "" : szPort), 
        (m_szDBName[0] == '\0' ? "" : ";Database="),
        (m_szDBName[0] == '\0' ? "" : m_szDBName),
        m_szUser, m_szPassword);

    /* Connect to data source */
    hRecorde = SQLDriverConnect(m_hDbc,
        NULL,
        (unsigned char *)szConnOption,
        SQL_NTS,
        szConnStrOut,
        sizeof(szConnStrOut),
        &cbConnStrOut,
        SQL_DRIVER_NOPROMPT);

    if (hRecorde != SQL_SUCCESS && hRecorde != SQL_SUCCESS_WITH_INFO) {
        nRet = WAVETOP_BACKUP_INVALID_SYNTAX;
        goto CLEANUP;
    }

    return nRet;

CLEANUP: 
    Close();
    return nRet;
}

#endif