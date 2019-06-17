#include "Ado.h"


CAdo::CAdo()
{
    m_pszIP = NULL;
    m_nPort = NULL;
    m_pszUser = NULL; 
    m_pszPwd  = NULL;
}

CAdo::~CAdo(void)
{
    if (m_pRecordset && m_pRecordset->State == adStateOpen)
        m_pRecordset->Close();
    if (m_pConnection && m_pConnection->State == adStateOpen)
        m_pConnection->Close();

    if(m_pszIP != NULL)
        free(m_pszIP);
    if(m_pszPwd != NULL)
        free(m_pszPwd);
    if(m_pszUser != NULL)
        free(m_pszUser);

    cout<<"CAdo  析构啦\n";
}

int CAdo::Init(char *pszIP, int nPort, char *pszUser, 
    char *pszPwd)
{
    size_t len = 0;
    HRESULT hr                           = S_OK;

    if(pszIP == NULL || pszUser == NULL || pszPwd ==NULL)
        return -1;

    len = strlen(pszIP) + 1;
    m_pszIP = (char *)malloc(len);
    if(m_pszIP == NULL)
        return -1;
    strcpy(m_pszIP, pszIP);

    m_nPort = nPort;

    len = strlen(pszUser) + 1;
    m_pszUser = (char *)malloc(len);
    if(m_pszUser == NULL)
        return -1;
    strcpy(m_pszUser, pszUser);

    len = strlen(pszPwd) + 1;
    m_pszPwd = (char *)malloc(len);
    if(m_pszPwd == NULL)
        return -1;
    strcpy(m_pszPwd, pszPwd);

    hr = m_pConnection.CreateInstance("ADODB.Connection");
    if(!SUCCEEDED(hr))
        return -2;
    hr = m_pRecordset.CreateInstance(__uuidof(Recordset));
    if(!SUCCEEDED(hr))
        return -2;
    
    return 0;
}

int CAdo::ConnectDB()
{
    char szConnStr[512]                  = {0};
    HRESULT hr                           = S_OK;

    try {
        if(SUCCEEDED(hr)) {
            _snprintf_s(szConnStr, sizeof(szConnStr), "Provider=SQLOLEDB.1;Data Source=%s,"
                "%d;Initial Catalog=msdb",m_pszIP,m_nPort);
            hr = m_pConnection->Open(szConnStr , m_pszUser, 
                m_pszPwd, adModeUnknown);
        }
    }
    catch (_com_error e) {
        MSSQL_LOG_ERR("GetDBState: m_pConnection Open error: %s",
            e.Description());
        return WAVETOP_BACKUP_INTERNAL_ERROR;
    }

    return 0;
}

int CAdo::ExecSqlNorecord()
{
    HRESULT hr = 0;
    try {
        cout<<(char*)m_bstrSql<<endl;
        hr = m_pConnection->Execute(m_bstrSql,NULL,adExecuteNoRecords);
        if(FAILED(hr))
            cout<<"执行语句错误\n";
    }
    catch(_com_error &e) {
        //MSSQL_LOG_ERR("%s;\n",(char*)e.Description());
        PrintProviderError(m_pConnection);
        if(m_pConnection->Errors->Count > 0)
            m_NativeError = m_pConnection->Errors->GetItem(0)->NativeError;
        else
            m_NativeError = 0;
        return WAVETOP_BACKUP_INTERNAL_ERROR;
    }

    return hr;
}

int CAdo::ExecSqlWithRecord()
{
    HRESULT hr = 0;

    try {
        hr = m_pRecordset->Open(m_bstrSql,
            _variant_t((IDispatch*)m_pConnection,true),
            adOpenDynamic,adLockOptimistic,adCmdText);
    }
    catch(_com_error e) {
        MSSQL_LOG_ERR("GetDBState: exec sql  error: %s",
            e.Description());
        return WAVETOP_BACKUP_INTERNAL_ERROR;
    }
    return 0;
}

int CAdo::LogBackup(const char *pszDBName, const char *pszPath)
{
    int nRC = 0;
    nRC = ConnectDB();
    if(nRC != 0)
        return nRC;
    m_bstrSql = "backup log [" + _bstr_t(pszDBName) + _bstr_t("] to disk = '");
    m_bstrSql += pszPath + _bstr_t("\\") + pszDBName + _bstr_t(".bak' with compression;"); 

    nRC = ExecSqlNorecord();
    if(nRC != 0)
        return nRC;

    return 0;
}

int CAdo::ShrinkDB(const char *pszDBName)
{
    int nRC = 0;
    nRC = ConnectDB();
    if(nRC != 0)
        return nRC;
   
    m_bstrSql = "USE MASTER\n";
    m_bstrSql += "ALTER DATABASE [" + _bstr_t(pszDBName) + "] SET RECOVERY SIMPLE WITH NO_WAIT\n";
    m_bstrSql += "ALTER DATABASE [" + _bstr_t(pszDBName) + "] SET TORN_PAGE_DETECTION ON WITH NO_WAIT\n";
    m_bstrSql += "ALTER DATABASE [" + _bstr_t(pszDBName) + "] SET RECOVERY SIMPLE\n";
    m_bstrSql += "ALTER DATABASE [" + _bstr_t(pszDBName) + "] SET TORN_PAGE_DETECTION ON\n";
    m_bstrSql += "use [" + _bstr_t(pszDBName) + "]\n";
    m_bstrSql += "declare @fileid int\n";
    m_bstrSql += "select @fileid = file_id from sys.database_files where type=1\n";
    m_bstrSql += "DBCC SHRINKFILE(@fileid, 10)\n";
    m_bstrSql += "use master\n";
    m_bstrSql += "ALTER DATABASE [" + _bstr_t(pszDBName) + "] SET RECOVERY FULL WITH NO_WAIT\n";
    m_bstrSql += "ALTER DATABASE [" + _bstr_t(pszDBName) + "] SET TORN_PAGE_DETECTION ON WITH NO_WAIT\n";
    m_bstrSql += "ALTER DATABASE [" + _bstr_t(pszDBName) + "] SET RECOVERY FULL\n";
    m_bstrSql += "ALTER DATABASE [" + _bstr_t(pszDBName) + "] SET TORN_PAGE_DETECTION ON\n";

    nRC = ExecSqlNorecord();
    if(nRC != 0)
        return nRC;

    return 0;
}



int CAdo::GetDBVersion()
{
    int nRC = 0;
    _bstr_t bstrRowInfo;
    char szType[2048] = {0};
    int nVersion = 0;
    int nTemp;
    char *pszstr;


    nRC = ConnectDB();
    if(nRC != 0)
        return nRC;
    m_bstrSql = "SELECT SERVERPROPERTY('productversion')";
    

    nRC = ExecSqlWithRecord();
    if(nRC != 0)
        return nRC;

    try { 
        while (!m_pRecordset->EndOfFile) {
            bstrRowInfo = m_pRecordset->GetFields()->GetItem("")->GetValue();
            strncpy(szType, (char *)bstrRowInfo, sizeof(szType));

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
            else if ( (szType[0] == '1') && ( (szType[1] >= '1') && (szType[1] <= '9') )) {
                nVersion =  MI_MSSQL2_2K12_VERSION;
            }
            else {
                nVersion = MI_MSSQL2_2K0_VERSION;
            }
            m_pRecordset->MoveNext();
        }
    }

    catch(_com_error e)	{
        MSSQL_LOG_ERR("MiMSSqlGetVersion():: m_pRecordset GetData error: %s",
            (char*)e.Description());
        nRC = WAVETOP_BACKUP_INTERNAL_ERROR;
    }

    return 0;
}

int CAdo::BackupDBVDI(const char *pszDBName, const char *pszDeviceName)
{
    int nRC = 0;
    nRC = ConnectDB();
    if(nRC != 0){
        cout<<"connect DB failed\n";
        return nRC;
    } else
        cout<<"connect DB success\n";


    m_bstrSql = "backup database [" + _bstr_t(pszDBName) + _bstr_t("] to VIRTUAL_DEVICE = '");
    m_bstrSql += pszDeviceName + _bstr_t("' with compression;"); 

    nRC = ExecSqlNorecord();
    if(nRC != 0){
        cout<<"execsqlNorecord failed\n";
        return nRC;
    }else{
        cout<<"execsqlNorecord success\n";
    }


    return 0;
}

int CAdo::BackupLogVDI(const char *pszDBName, const char *pszDeviceName, int &nNativeError)
{
    int nRC = 0;
    nRC = ConnectDB();
    if(nRC != 0){
        cout<<"connect DB failed\n";
        return nRC;
    } else
        cout<<"connect DB success\n";


    m_bstrSql = "backup database [" + _bstr_t(pszDBName) + _bstr_t("] to VIRTUAL_DEVICE = '");
    m_bstrSql += pszDeviceName + _bstr_t("' with compression;"); 

    nRC = ExecSqlNorecord();
    if(nRC != 0){
        cout<<"execsqlNorecord failed\n";
    }else{
        cout<<"execsqlNorecord success\n";
    }

    nNativeError = m_NativeError;

    return nRC;
}

//ADO 获取bak信息
int CAdo::GetDBState(char *pszDBName)
{
    MiMSSqlstDbString *pstDbFileNameNode = NULL; 
    MiMSSqlstDbString *pstDblogNameNode  = NULL;
    HRESULT hr                           = S_OK;
    char szConnStr[512]                  = {0};
    char szSql[2048]                     = {0};
    char szType[2048]                    = {0};
    int nResult                          = WAVETOP_BACKUP_OK;
    int nLen                             = 0;
    int a;

    _variant_t var;
    _bstr_t bstrRowLogicalInfo;
    _bstr_t bstrRowPhysicalInfo;
    _variant_t vName;
    _variant_t vType;

    nResult = ConnectDB();
    if(nResult != 0)
        return nResult;

    PR_snprintf(szSql, sizeof(szSql), 
        "select recovery_model,database_id,is_read_only from sys.databases where name = '%s'"
        ,pszDBName);
    /*PR_snprintf(szSql, sizeof(szSql), 
        "select id as is_read_only,remark from db1077.dbo.tablex1");*/
    m_pRecordset.CreateInstance(__uuidof(Recordset));
    try {
        hr = m_pRecordset->Open(szSql,
            _variant_t((IDispatch*)m_pConnection,true),
            adOpenDynamic,adLockOptimistic,adCmdText);
    }
    catch(_com_error e) {
        MSSQL_LOG_ERR("GetDBState: exec sql  error: %s",
            e.Description());
        if (m_pConnection)
            if (m_pConnection->State == adStateOpen)
                m_pConnection->Close();
        return WAVETOP_BACKUP_INTERNAL_ERROR;
    } 

    try { 
        while (!m_pRecordset->EndOfFile) {
            bstrRowLogicalInfo = m_pRecordset->GetFields()->GetItem("is_read_only")->GetValue();
            a = (bool)atoi((char*)bstrRowLogicalInfo);


            bstrRowLogicalInfo = m_pRecordset->GetFields()->GetItem("database_id")->GetValue();
            int b = atoi((char*)bstrRowLogicalInfo);

            bstrRowLogicalInfo = m_pRecordset->GetFields()->GetItem("recovery_model")->GetValue();
            int c = atoi((char*)bstrRowLogicalInfo);
             
            m_pRecordset->MoveNext();
        }
    }

    catch(_com_error e)	{
        MSSQL_LOG_ERR("GetDBState: m_pRecordset GetData error: %s",
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
        MSSQL_LOG_ERR("GetDBState: close m_Recordset"
            " or m_pConnection failed. error: %s",
            e.Description());
    }

    return nResult;
}

void CAdo::PrintProviderError(_ConnectionPtr pConnection) {
    // Print Provider Errors from Connection object.
    // pErr is a record object in the Connection's Error collection.
    ErrorPtr pErr = NULL;

    if ( (pConnection->Errors->Count) > 0 ) {
        long nCount = pConnection->Errors->Count;
        // Collection ranges from 0 to nCount -1.
        for ( long i = 0 ; i < nCount ; i++ ) {
            pErr = pConnection->Errors->GetItem(i);
            printf("\t NativeError: %d ;Error number: %x\t%s\n",pErr->NativeError, pErr->Number,(char*)(pErr->Description));
            //printf("%s;%s;%s\n", (char*)pErr->Source, (char *)pErr->HelpContext, (char *)pErr->SQLState);
            //printf("%s\n", (char*)pErr->GetSQLState());
        }
    }
    

    printf("\n");
}

void CAdo::PrintComError(_com_error &e) {
    _bstr_t bstrSource(e.Source());
    _bstr_t bstrDescription(e.Description());

    // Print Com errors.
    printf("Error\n");
    printf("\tCode = %08lx\n", e.Error());
    printf("\tCode meaning = %s\n", e.ErrorMessage());
    printf("\tSource = %s\n", (LPCSTR) bstrSource);
    printf("\tDescription = %s\n", (LPCSTR) bstrDescription);
}

int CAdo::EXECscript()
{
    _CommandPtr pCmdByRoyalty = NULL;
    _bstr_t bstrTemp;


    
    HRESULT hr = pCmdByRoyalty.CreateInstance(__uuidof(Command));
    if(!SUCCEEDED(hr)){
        cout<<"创建_CommandPtr 失败"<<endl;
        return -1;
    }
    pCmdByRoyalty->ActiveConnection = m_pConnection;
    pCmdByRoyalty->CommandText = "script.txt";
    pCmdByRoyalty->CommandType = adCmdText;
    pCmdByRoyalty->CommandTimeout = 0;

    

    try
    {
        m_pRecordset = pCmdByRoyalty->Execute(NULL,NULL,adCmdText);
        while (!(m_pRecordset->EndOfFile)) {
            bstrTemp = m_pRecordset->GetFields()->GetItem("name")->GetValue();
            cout<<(char *)bstrTemp<<endl;

            m_pRecordset->MoveNext(); 
        }
    }
    catch (_com_error e)
    {
        MSSQL_LOG_ERR("EXECscript():: m_pRecordset GetData error: %s",
            (char*)e.Description());
        return WAVETOP_BACKUP_INTERNAL_ERROR;
    }

    

    return 0;

}


int ChangeChar()
{
    _bstr_t bstr1 = "CREATE DATABASE [db1003] ON(filename=N'C:/DBdata2/db1003.mdf'),(filename=N'C:/DBdata2/db1003_log.ldf')"            "for attach";
    return 1;
    
}


