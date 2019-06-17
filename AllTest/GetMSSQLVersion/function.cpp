#include "function.h"



//ADO 获取版本信息
int MiMSSqlGetVersion()
    {
    _ConnectionPtr m_pConnection         = NULL;
    _RecordsetPtr  m_pRecordset          = NULL;
    HRESULT hr                           = S_OK;
    char szConnStr[512]                  = {0};
    char szSql[2048]                     = {0};
    char szType[2048]                    = {0};
    int nResult                          = 0;
    int nLen                             = 0;
    int nVersion = 0;
    _bstr_t bstrRowInfo;
    _variant_t vName;
    _variant_t vType;

    try {
        hr = m_pConnection.CreateInstance(__uuidof(Connection));
        if(SUCCEEDED(hr)) {
            PR_snprintf(szConnStr, sizeof(szConnStr), "Provider=SQLOLEDB.1;Data Source=%s,"
                "%s;Initial Catalog=msdb","60.60.60.15","1433");
            hr = m_pConnection->Open(szConnStr , "sa", 
                "Root@123", adModeUnknown);
            }
        }
    catch (_com_error e) {
        MSSQL_LOG_ERR(" m_pConnection Open error: %s",
            e.Description());
        return WAVETOP_BACKUP_INTERNAL_ERROR;
        } 

    PR_snprintf(szSql, sizeof(szSql), 
        "SELECT SERVERPROPERTY('productversion')");
    m_pRecordset.CreateInstance(__uuidof(Recordset));
    try {
        hr = m_pRecordset->Open(szSql,
            _variant_t((IDispatch*)m_pConnection,true),
            adOpenDynamic,adLockOptimistic,adCmdText);
        }
    catch(_com_error e) {
        MSSQL_LOG_ERR(" exec sql  error: %s",
            e.Description());
        if (m_pConnection && m_pConnection->State == adStateOpen)
                m_pConnection->Close();
        return WAVETOP_BACKUP_INTERNAL_ERROR;
        } 

    try { 
        while (!m_pRecordset->adoEOF) {
            bstrRowInfo = m_pRecordset->GetFields()->GetItem("")->GetValue();
            strncpy(szType, (char *)bstrRowInfo, sizeof(szType));
            MSSQL_LOG_INFO("%s\n",szType);

            /* Database name*/
            if (   strncmp(szType, "8", 1) == 0 ) {
                nVersion = MI_MSSQL2_2K0_VERSION;
            }
            else if (strncmp(szType, "9", 1) == 0) {
                nVersion =  MI_MSSQL2_2K5_VERSION;
            }
            else if (strncmp(szType, "10", 2) == 0) {
                nVersion =  MI_MSSQL2_2K8_VERSION;
            }
            else if (strncmp(szType, "11", 2) == 0 || strncmp(szType, "12", 2) == 0 || strncmp(szType, "13", 2) == 0) {
                nVersion =  MI_MSSQL2_2K12_VERSION;
            }
            else {
                nResult = WAVETOP_BACKUP_INTERNAL_ERROR;
            }
            m_pRecordset->MoveNext();
        }
    }

    catch(_com_error e)	{
        MSSQL_LOG_ERR(": m_pRecordset GetData error: %s",
            e.Description());
        nResult = WAVETOP_BACKUP_INTERNAL_ERROR;
        goto EXIT;
        }

EXIT:
    try
        {
        if (m_pRecordset && m_pRecordset->State == adStateOpen)
            m_pRecordset->Close();
        if (m_pConnection && m_pConnection->State == adStateOpen)
            m_pConnection->Close();
        }
    catch(_com_error e){
        MSSQL_LOG_ERR("MiMSSqlGetNameFromDeviceFromADO: close m_Recordset"
            " or m_pConnection failed. error: %s",
            e.Description());
        }


    return nResult;
    }


//ADO 独占数据库
int MiMSSqlGetDBModeShareExclusive()
{
    _ConnectionPtr m_pConnection         = NULL;
    _RecordsetPtr  m_pRecordset          = NULL;
    HRESULT hr                           = S_OK;
    char szConnStr[512]                  = {0};
    char szSql[2048]                     = {0};
    char szType[2048]                    = {0};
    int nResult                          = 0;
    int nLen                             = 0;
    int nVersion = 0;
    _bstr_t bstrRowInfo;
    _variant_t vName;
    _variant_t vType;

    try {
        hr = m_pConnection.CreateInstance(__uuidof(Connection));
        if(SUCCEEDED(hr)) {
            PR_snprintf(szConnStr, sizeof(szConnStr), "Provider=SQLOLEDB.1;Data Source=%s,"
                "%s;Initial Catalog=mssql1010","60.60.60.15","1433");
            hr = m_pConnection->Open(szConnStr , "sa", 
                "Root@123", adModeRead);
        }
    }
    catch (_com_error e) {
        MSSQL_LOG_ERR(" m_pConnection Open error: %s",
            e.Description());
        return WAVETOP_BACKUP_INTERNAL_ERROR;
    } 

    PR_snprintf(szSql, sizeof(szSql), 
        "SELECT SERVERPROPERTY('productversion')");
    m_pRecordset.CreateInstance(__uuidof(Recordset));
    try {
        hr = m_pRecordset->Open(szSql,
            _variant_t((IDispatch*)m_pConnection,true),
            adOpenDynamic,adLockOptimistic,adCmdText);
    }
    catch(_com_error e) {
        MSSQL_LOG_ERR(" exec sql  error: %s",
            e.Description());
        if (m_pConnection && m_pConnection->State == adStateOpen)
            m_pConnection->Close();
        return WAVETOP_BACKUP_INTERNAL_ERROR;
    } 

    try { 
        while (!m_pRecordset->adoEOF) {
            bstrRowInfo = m_pRecordset->GetFields()->GetItem("")->GetValue();
            strncpy(szType, (char *)bstrRowInfo, sizeof(szType));
            MSSQL_LOG_INFO("%s\n",szType);

            /* Database name*/
            if (   strncmp(szType, "8", 1) == 0 ) {
                nVersion = MI_MSSQL2_2K0_VERSION;
            }
            else if (strncmp(szType, "9", 1) == 0) {
                nVersion =  MI_MSSQL2_2K5_VERSION;
            }
            else if (strncmp(szType, "10", 2) == 0) {
                nVersion =  MI_MSSQL2_2K8_VERSION;
            }
            else if (strncmp(szType, "11", 2) == 0 || strncmp(szType, "12", 2) == 0 || strncmp(szType, "13", 2) == 0) {
                nVersion =  MI_MSSQL2_2K12_VERSION;
            }
            else {
                nResult = WAVETOP_BACKUP_INTERNAL_ERROR;
            }
            m_pRecordset->MoveNext();
        }
    }

    catch(_com_error e)	{
        MSSQL_LOG_ERR(": m_pRecordset GetData error: %s",
            e.Description());
        nResult = WAVETOP_BACKUP_INTERNAL_ERROR;
        goto EXIT;
    }

EXIT:
    try
    {
        if (m_pRecordset && m_pRecordset->State == adStateOpen)
            m_pRecordset->Close();
        if (m_pConnection && m_pConnection->State == adStateOpen)
            m_pConnection->Close();
    }
    catch(_com_error e){
        MSSQL_LOG_ERR("MiMSSqlGetNameFromDeviceFromADO: close m_Recordset"
            " or m_pConnection failed. error: %s",
            e.Description());
    }


    return nResult;
}