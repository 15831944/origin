#pragma once
#include "public.h"
#import "c:\Program Files\Common Files\System\ADO\msado15.dll" \
    no_namespace rename("EOF", "EndOfFile")

#define WAVETOP_BACKUP_OK 0
#define WAVETOP_BACKUP_INTERNAL_ERROR 3
#define WAVETOP_BACKUP_FILE_TOO_LARGE 5

//数据库版本 
#define MI_MSSQL2_2K0_VERSION    8   //2000版本
#define MI_MSSQL2_2K5_VERSION    9   //2005版本
#define MI_MSSQL2_2K8_VERSION    10  //2008版本
#define MI_MSSQL2_2K12_VERSION   11  //2012版本及以上

#define MSSQL_LOG_INFO(format, ...)\
    printf(format,##__VA_ARGS__)
#define MSSQL_LOG_ERR(format, ...)\
    printf(format,##__VA_ARGS__)

/* the Structure of exported file */
typedef struct _MiMSSqlstDbString {
    char szDbString[1024];
    char szPhysicalStr[1024];
    struct _MiMSSqlstDbString *pNext;
}MiMSSqlstDbString;

class CAdo
{
public:
    CAdo(void);
    ~CAdo(void);
    int CAdo::Init(char *pszIP, int nPort, char *pszUser, 
        char *pszPwd);
    int ConnectDB();
    int ExecSqlNorecord();
    int ExecSqlWithRecord();
    void PrintProviderError(_ConnectionPtr pConnection);
    void CAdo::PrintComError(_com_error &e);

    int CAdo::LogBackup(const char *pszDBName, const char *pszPath);
    int CAdo::BackupDBVDI(const char *pszDBName, const char *pszDeviceName);
    int CAdo::BackupLogVDI(const char *pszDBName, 
        const char *pszDeviceName, int &nNativeError);

    int CAdo::GetDBVersion();
    int CAdo::GetDBState(char *pszDBName);

    int CAdo::EXECscript();
    int CAdo::ScriptTest();

    int CAdo::ShrinkDB(const char *pszDBName);

public:
    _bstr_t m_bstrSql;
    _RecordsetPtr  m_pRecordset;

private:
    _ConnectionPtr m_pConnection;
    
    //_CommandPtr m_pCmdByRoyalty;
    char*   m_pszIP;
    int     m_nPort;
    char*   m_pszUser; 
    char*   m_pszPwd;
    
    int     m_NativeError;
};

