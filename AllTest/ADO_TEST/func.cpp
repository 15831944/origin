#include "func.h"


int MiGetCurResHisId()
{
    _ConnectionPtr m_pConnection = NULL;
    _RecordsetPtr m_pRecordset = NULL;
	_variant_t var;
    _variant_t vRes;
    HRESULT hr;
    char szConnStr[128];
	char szSQL[1024];
    char szMessage[1024];
    char *pszConver;
    int nBackupSetId;
    int nResHisId;
    int nResult;
    int i = 0;

    /* init com in agent  
     * hr = CoInitialize(NULL);
     * if (FAILED(hr)) {
	 *	return WAVETOP_BACKUP_INTERNAL_ERROR;
     * }
     */

    nResult = WAVETOP_BACKUP_OK;
    try {
        hr = m_pConnection.CreateInstance("ADODB.Connection");
        if(SUCCEEDED(hr)) {

			PR_snprintf(szConnStr, sizeof(szConnStr), "Provider=SQLOLEDB.1;Persist Security" 
				"Info=False;Integrated Security=SSPI;Data Source=%s;Initial Catalog=msdb", 
				"2.2.2.40,1433");
			hr = m_pConnection->Open(szConnStr, "", "", adModeUnknown);
		}


    }
    catch (_com_error e) {
        strncpy(szMessage, e.Description(), sizeof(szMessage));
        return WAVETOP_BACKUP_INTERNAL_ERROR;
    } 

	PR_snprintf(szSQL, sizeof(szSQL),
        "select restore_history_id,backup_set_id from restorehistory "
        "where restore_history_id in (select max(restore_history_id) from restorehistory "
        "where destination_database_name = \'%s\')", "db1000");

    m_pRecordset.CreateInstance(__uuidof(Recordset));

    try {
        hr = m_pRecordset->Open(szSQL,
                            _variant_t((IDispatch*)m_pConnection,true),
                            adOpenDynamic,
                            adLockOptimistic,
                            adCmdText);
    }
    catch(_com_error e) {
		strncpy(szMessage, e.Description(), sizeof(szMessage));
		m_pConnection->Close();
        return WAVETOP_BACKUP_INTERNAL_ERROR;
    } 
    
    try { 
        while (!m_pRecordset->EndOfFile) {
            var = m_pRecordset->GetFields()->GetItem("backup_set_id")->GetValue();
            vRes = m_pRecordset->GetFields()->GetItem("restore_history_id")->GetValue();
            if (var.vt != VT_NULL) {
                nBackupSetId = var.iVal;
            }
            if (vRes.vt != VT_NULL) {
                nResHisId = vRes.iVal;
            }
            //PR_snprintf(pszCurResHisId, nSize, "%d", nResHisId);
           // PR_snprintf(pszResBSID, nResSize, "%d", nBackupSetId);
			cout<<"nResHisId: "<<nResHisId<<endl;
			cout<<"nBackupSetId: "<<nBackupSetId<<endl;
            m_pRecordset->MoveNext();
        }
    }

    catch(_com_error e)	{
 		strncpy(szMessage, e.Description(), sizeof(szMessage));
		nResult = WAVETOP_BACKUP_INTERNAL_ERROR;
        goto EXIT;
	}

EXIT:

    m_pRecordset->Close();
	if(m_pConnection->State)
		 m_pConnection->Close(); 
    m_pConnection = NULL;

    /* uninit com in modmssql dllmain() 
     * CoUninitialize();
     */

    return nResult;
}

#ifdef WIN32
char *unicodeToUtf8(const WCHAR *zWideFilename,size_t m_encode = CP_UTF8)   
{   
    int nByte;   
    char *zFilename;   

    nByte = WideCharToMultiByte(m_encode, 0, zWideFilename, -1, 0, 0, 0, 0);   
    zFilename = (char *)malloc(nByte);   
    if(zFilename == 0) {   
        return 0;   
    }   

    nByte = WideCharToMultiByte(m_encode, 0, zWideFilename, -1, zFilename, nByte, 0, 0);   
    if( nByte == 0 ) {   
        free(zFilename);   
        zFilename = 0;   
    }   
    cout<<"       "<<zFilename<<endl;
    return zFilename;   
}

WCHAR *mbcsToUnicode(const char *zFilename)   
{   
    int nByte;   
    WCHAR *zMbcsFilename;   
    int codepage = AreFileApisANSI() ? CP_ACP : CP_OEMCP;   

    nByte = MultiByteToWideChar(codepage, 0, zFilename, -1, NULL,0)*sizeof(WCHAR);   
    zMbcsFilename = (WCHAR *)malloc(nByte*sizeof(zMbcsFilename[0]));   
    if( zMbcsFilename==0 ) {   
        return 0;   
    }   

    nByte = MultiByteToWideChar(codepage, 0, zFilename, -1,   zMbcsFilename, nByte);   
    if(nByte == 0) {   
        free(zMbcsFilename);   
        zMbcsFilename = 0;   
    }   
    return zMbcsFilename;   
}
#endif

char* ConvertToUTF(char *pszFile)
{
    unsigned short *wcPath;

    if (pszFile == NULL) {
        return NULL;
    }

#if WIN32
    wcPath = (unsigned short *)mbcsToUnicode(pszFile);   
    pszFile = unicodeToUtf8((const WCHAR *)wcPath);
#endif

    return pszFile;
}

int MiMSSqlFreeStDbString(MiMSSqlstDbString *pstString)
{
    MiMSSqlstDbString *pstStringNode;

    if (NULL == pstString) {
        return WAVETOP_BACKUP_OK;
    }

    while (pstString) {
        pstStringNode = pstString;
        pstString = pstString->pNext;
        free(pstStringNode);
        if (NULL == pstString) {
            break;
        }
    }
    return WAVETOP_BACKUP_OK;
}

//ADO 获取bak信息
int MSSqlGetNameFromDeviceFromADO(char *pszIP, int nPort, char *pszUser, 
    char *pszPwd, char *pszDeviceName, MiMSSqlstDbString **pstDbFileName, 
    MiMSSqlstDbString **pstDblogName)
{
    MiMSSqlstDbString *pstDbFileNameNode = NULL; 
    MiMSSqlstDbString *pstDblogNameNode  = NULL;
    _ConnectionPtr m_pConnection         = NULL;
    _RecordsetPtr  m_pRecordset          = NULL;
    HRESULT hr                           = S_OK;
    char szConnStr[512]                  = {0};
    char szSql[2048]                     = {0};
    char szType[2048]                    = {0};
    int nResult                          = WAVETOP_BACKUP_OK;
    int nLen                             = 0;
    _bstr_t bstrRowLogicalInfo;
    _bstr_t bstrRowPhysicalInfo;
    _variant_t vName;
    _variant_t vType;

    if (NULL == pszDeviceName) {
        return WAVETOP_BACKUP_FILE_TOO_LARGE;
    }
    *pstDbFileName = NULL;
    *pstDblogName = NULL;

    try {
        hr = m_pConnection.CreateInstance("ADODB.Connection");
        if(SUCCEEDED(hr)) {
            PR_snprintf(szConnStr, sizeof(szConnStr), "Provider=SQLOLEDB.1;Data Source=%s,"
                "%d;Initial Catalog=msdb",pszIP,nPort);
            hr = m_pConnection->Open(szConnStr , pszUser, 
                pszPwd, adModeUnknown);
        }
    }
    catch (_com_error e) {
        MSSQL_LOG_ERR("MiMSSqlGetNameFromDeviceFromADO: m_pConnection Open error: %s",
            e.Description());
        return WAVETOP_BACKUP_INTERNAL_ERROR;
    } 

    PR_snprintf(szSql, sizeof(szSql), 
        "RESTORE FILELISTONLY FROM DISK = '%s'",
        pszDeviceName);
    m_pRecordset.CreateInstance(__uuidof(Recordset));
    try {
        hr = m_pRecordset->Open(szSql,
            _variant_t((IDispatch*)m_pConnection,true),
            adOpenDynamic,adLockOptimistic,adCmdText);
    }
    catch(_com_error e) {
        MSSQL_LOG_ERR("MiMSSqlGetNameFromDeviceFromADO: exec sql  error: %s",
            e.Description());
        if (m_pConnection)
            if (m_pConnection->State == adStateOpen)
                m_pConnection->Close();
        return WAVETOP_BACKUP_INTERNAL_ERROR;
    } 

    try { 
        while (!m_pRecordset->EndOfFile) {
            bstrRowLogicalInfo = m_pRecordset->GetFields()->GetItem("Type")->GetValue();
            strncpy(szType, (char *)bstrRowLogicalInfo, sizeof(szType));

            /* Database name*/
            if (!strcmp(szType,"D")) {
                bstrRowLogicalInfo = m_pRecordset->GetFields()->GetItem("LogicalName")->GetValue();
                bstrRowPhysicalInfo = m_pRecordset->Fields->GetItem("PhysicalName")->Value;
                /* judge the length of sqlquery */
                if (strlen((char *)bstrRowLogicalInfo) > 1024) {
                    nResult = WAVETOP_BACKUP_FILE_TOO_LARGE;
                    break;
                }

                char *psztemp = strrchr((char*)bstrRowPhysicalInfo, '\\');
                if(!psztemp || (psztemp && strlen(psztemp) > 1024)){
                    nResult = WAVETOP_BACKUP_FILE_TOO_LARGE;
                    break;
                }
                psztemp++;

                pstDbFileNameNode = (MiMSSqlstDbString *)malloc(sizeof(MiMSSqlstDbString));
                if (!pstDbFileNameNode) {
                    nResult = WAVETOP_BACKUP_FILE_TOO_LARGE;
                    break;
                }

                strcpy(pstDbFileNameNode->szDbString, (char *)bstrRowLogicalInfo);
                strcpy(pstDbFileNameNode->szPhysicalStr, psztemp);

                pstDbFileNameNode->pNext = *pstDbFileName;
                *pstDbFileName = pstDbFileNameNode; 
            }
            /* log name */
            else if (!strcmp(szType,"L")) {
                /* get db logical name */
                bstrRowLogicalInfo = m_pRecordset->GetFields()->GetItem("LogicalName")->GetValue();
                bstrRowPhysicalInfo = m_pRecordset->Fields->GetItem("PhysicalName")->Value;

                /* judge the length of sqlquery */
                if (strlen((char *)bstrRowLogicalInfo) > 1024) {
                    nResult = WAVETOP_BACKUP_FILE_TOO_LARGE;
                    break; 
                }

                char *psztemp = strrchr((char*)bstrRowPhysicalInfo, '\\');
                if(!psztemp || (psztemp && strlen(psztemp) > 1024)){
                    nResult = WAVETOP_BACKUP_FILE_TOO_LARGE;
                    break;
                }
                psztemp++;

                pstDblogNameNode = (MiMSSqlstDbString *)malloc(sizeof(MiMSSqlstDbString));
                if (!pstDblogNameNode) {
                    nResult = WAVETOP_BACKUP_FILE_TOO_LARGE;
                    break;
                }

                strcpy(pstDblogNameNode->szDbString, (char *)bstrRowLogicalInfo);
                strcpy(pstDblogNameNode->szPhysicalStr, psztemp);

                pstDblogNameNode->pNext = *pstDblogName;
                *pstDblogName = pstDblogNameNode;
            }
            else {
                nResult = WAVETOP_BACKUP_FILE_TOO_LARGE;
                break; 
            }

            m_pRecordset->MoveNext();
        }
    }

    catch(_com_error e)	{
        MSSQL_LOG_ERR("MiMSSqlGetNameFromDeviceFromADO: m_pRecordset GetData error: %s",
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


    /* if error free memory */
    if (nResult != WAVETOP_BACKUP_OK) {
        MiMSSqlFreeStDbString(*pstDbFileName);
        *pstDbFileName = NULL;
        MiMSSqlFreeStDbString(*pstDblogName);
        *pstDblogName = NULL;
    }

    return nResult;
}


//ADO 获取bak信息
int MemoryConnect(char *pszDeviceName, MiMSSqlstDbString **pstDbFileName, 
    MiMSSqlstDbString **pstDblogName)
{
    MiMSSqlstDbString *pstDbFileNameNode = NULL; 
    MiMSSqlstDbString *pstDblogNameNode  = NULL;
    _ConnectionPtr m_pConnection         = NULL;
    _RecordsetPtr  m_pRecordset          = NULL;
    HRESULT hr                           = S_OK;
    char szConnStr[512]                  = {0};
    char szSql[2048]                     = {0};
    char szType[2048]                    = {0};
    int nResult                          = WAVETOP_BACKUP_OK;
    int nLen                             = 0;
    _bstr_t bstrRowLogicalInfo;
    _bstr_t bstrRowPhysicalInfo;
    _variant_t vName;
    _variant_t vType;

    if (NULL == pszDeviceName) {
        return WAVETOP_BACKUP_FILE_TOO_LARGE;
    }
    *pstDbFileName = NULL;
    *pstDblogName = NULL;

    try {
        hr = m_pConnection.CreateInstance("ADODB.Connection");
        if(SUCCEEDED(hr)) {
            PR_snprintf(szConnStr, sizeof(szConnStr), "Provider=SQLOLEDB.1;Initial Catalog=msdb;Data Source=WIN-48D3DIUHI4J;Integrated Security=SSPI;");
            hr = m_pConnection->Open(szConnStr , "", 
                "", adModeUnknown);
        }
    }
    catch (_com_error e) {
        MSSQL_LOG_ERR("MiMSSqlGetNameFromDeviceFromADO: m_pConnection Open error: %s",
            (char*)e.Description());
        return WAVETOP_BACKUP_INTERNAL_ERROR;
    } 

    PR_snprintf(szSql, sizeof(szSql), 
        "RESTORE FILELISTONLY FROM DISK = '%s'",
        pszDeviceName);
    m_pRecordset.CreateInstance(__uuidof(Recordset));
    try {
        hr = m_pRecordset->Open(szSql,
            _variant_t((IDispatch*)m_pConnection,true),
            adOpenDynamic,adLockOptimistic,adCmdText);
    }
    catch(_com_error e) {
        MSSQL_LOG_ERR("MiMSSqlGetNameFromDeviceFromADO: exec sql  error: %s",
            (char*)e.Description());
        if (m_pConnection)
            if (m_pConnection->State == adStateOpen)
                m_pConnection->Close();
        return WAVETOP_BACKUP_INTERNAL_ERROR;
    } 

    try { 
        while (!m_pRecordset->EndOfFile) {
            bstrRowLogicalInfo = m_pRecordset->GetFields()->GetItem("Type")->GetValue();
            strncpy(szType, (char *)bstrRowLogicalInfo, sizeof(szType));

            /* Database name*/
            if (!strcmp(szType,"D")) {
                bstrRowLogicalInfo = m_pRecordset->GetFields()->GetItem("LogicalName")->GetValue();
                bstrRowPhysicalInfo = m_pRecordset->Fields->GetItem("PhysicalName")->Value;
                /* judge the length of sqlquery */
                if (strlen((char *)bstrRowLogicalInfo) > 1024) {
                    nResult = WAVETOP_BACKUP_FILE_TOO_LARGE;
                    break;
                }

                char *psztemp = strrchr((char*)bstrRowPhysicalInfo, '\\');
                if(!psztemp || (psztemp && strlen(psztemp) > 1024)){
                    nResult = WAVETOP_BACKUP_FILE_TOO_LARGE;
                    break;
                }
                psztemp++;

                pstDbFileNameNode = (MiMSSqlstDbString *)malloc(sizeof(MiMSSqlstDbString));
                if (!pstDbFileNameNode) {
                    nResult = WAVETOP_BACKUP_FILE_TOO_LARGE;
                    break;
                }

                strcpy(pstDbFileNameNode->szDbString, (char *)bstrRowLogicalInfo);
                strcpy(pstDbFileNameNode->szPhysicalStr, psztemp);

                pstDbFileNameNode->pNext = *pstDbFileName;
                *pstDbFileName = pstDbFileNameNode; 
            }
            /* log name */
            else if (!strcmp(szType,"L")) {
                /* get db logical name */
                bstrRowLogicalInfo = m_pRecordset->GetFields()->GetItem("LogicalName")->GetValue();
                bstrRowPhysicalInfo = m_pRecordset->Fields->GetItem("PhysicalName")->Value;

                /* judge the length of sqlquery */
                if (strlen((char *)bstrRowLogicalInfo) > 1024) {
                    nResult = WAVETOP_BACKUP_FILE_TOO_LARGE;
                    break; 
                }

                char *psztemp = strrchr((char*)bstrRowPhysicalInfo, '\\');
                if(!psztemp || (psztemp && strlen(psztemp) > 1024)){
                    nResult = WAVETOP_BACKUP_FILE_TOO_LARGE;
                    break;
                }
                psztemp++;

                pstDblogNameNode = (MiMSSqlstDbString *)malloc(sizeof(MiMSSqlstDbString));
                if (!pstDblogNameNode) {
                    nResult = WAVETOP_BACKUP_FILE_TOO_LARGE;
                    break;
                }

                strcpy(pstDblogNameNode->szDbString, (char *)bstrRowLogicalInfo);
                strcpy(pstDblogNameNode->szPhysicalStr, psztemp);

                pstDblogNameNode->pNext = *pstDblogName;
                *pstDblogName = pstDblogNameNode;
            }
            else {
                nResult = WAVETOP_BACKUP_FILE_TOO_LARGE;
                break; 
            }

            m_pRecordset->MoveNext();
        }
    }

    catch(_com_error e)	{
        MSSQL_LOG_ERR("MiMSSqlGetNameFromDeviceFromADO: m_pRecordset GetData error: %s",
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


    /* if error free memory */
    if (nResult != WAVETOP_BACKUP_OK) {
        MiMSSqlFreeStDbString(*pstDbFileName);
        *pstDbFileName = NULL;
        MiMSSqlFreeStDbString(*pstDblogName);
        *pstDblogName = NULL;
    }

    return nResult;
}

int CreateSql(MiMSSqlstDbString *pstDbFileName, MiMSSqlstDbString *pstDblogName,
    char *pszDbName, char *pszDeviceName, char *pszPath, char **ppstrSql)
{
    int nRC = 0;
    string  strSql;
    MiMSSqlstDbString *pszDbFiletemp = NULL;
    MiMSSqlstDbString *pszDbLogtemp = NULL;
    char sztemp[512] = {0};
    /*
        USE MASTER;
        IF EXISTS (SELECT * FROM SYS.DATABASES WHERE NAME ='ZY' and state = 0) 
        Begin;
        EXEC('ALTER DATABASE [ZY] SET SINGLE_USER WITH ROLLBACK IMMEDIATE');
        EXEC('ALTER DATABASE [ZY] SET MULTI_USER');
        end;
        RESTORE DATABASE [ZY] FROM DISK = 'C:\syn_ZY_full.bak' WITH FILE = 1, STATS = 10, NOUNLOAD, BUFFERCOUNT = 6,maxtransfersize = 419304, REPLACE , 
        move 'ZY' to 'C:\DBData\ZY_data.mdf', move 'ZY_log' to 'C:\DBData\ZY_log_0.ldf'
        */

    strSql = "USE MASTER;IF EXISTS (SELECT * FROM SYS.DATABASES WHERE NAME ='";
    strSql += string(pszDbName) + string("' and state = 0) ");
    strSql += string("Begin;");
    strSql += string("EXEC('ALTER DATABASE [") + string(pszDbName) + string("] SET SINGLE_USER WITH ROLLBACK IMMEDIATE');");
    strSql += string("EXEC('ALTER DATABASE [") + string(pszDbName) + string("] SET MULTI_USER');");
    strSql += string("end;");
    strSql += string("RESTORE DATABASE [") + string(pszDbName) + string("] FROM DISK = '");
    strSql += string(pszDeviceName) + string("' WITH FILE = 1, STATS = 10, NOUNLOAD, BUFFERCOUNT = 6,");
    strSql += string("maxtransfersize = 419304, REPLACE ");


    pszDbFiletemp = pstDbFileName;
    pszDbLogtemp = pstDblogName;
    for (int i = 0; ; i ++) {
        if (0 == i) {
            nRC = PR_snprintf(sztemp, sizeof(sztemp), 
                "%s\\%s.mdf",
                pszPath,
                pszDbFiletemp->szDbString);
        }
        else if (i > 0){
            nRC = PR_snprintf(sztemp, sizeof(sztemp), 
                "%s\\%s.ndf",
                pszPath,
                pszDbFiletemp->szDbString);
        }
        if (-1 == nRC) {
            MSSQL_LOG_ERR("赋值出错");
            return -1;
        }
        strSql += string(", move '") + string(pszDbFiletemp->szDbString) + 
            string("' to N'") + string(sztemp) + string("'");
        pszDbFiletemp = pszDbFiletemp->pNext;
        if (NULL == pszDbFiletemp) {
            break;
        }
    }
    for (int i = 0; ; i ++) {
        nRC = PR_snprintf(sztemp, sizeof(sztemp),
            "%s\\%s.ldf", pszPath, pszDbLogtemp->szDbString);
        if (-1 == nRC) {
            MSSQL_LOG_ERR("赋值出错");
            return -1;
        }
        strSql += string(", move '") + string(pszDbLogtemp->szDbString) + 
            string("' to N'") + string(sztemp) + string("'");
        pszDbLogtemp = pszDbLogtemp->pNext;
        if (NULL == pszDbLogtemp) {
            break;
        }
    }

    *ppstrSql = strdup(strSql.c_str());
    if(*ppstrSql == NULL){
        MSSQL_LOG_ERR("内存分配失败");
        return -1;
    }

    return 0;
}
void PrintProviderError(_ConnectionPtr pConnection) {
    // Print Provider Errors from Connection object.
    // pErr is a record object in the Connection's Error collection.
    ErrorPtr pErr = NULL;

    if ( (pConnection->Errors->Count) > 0 ) {
        long nCount = pConnection->Errors->Count;
        // Collection ranges from 0 to nCount -1.
        for ( long i = 0 ; i < nCount ; i++ ) {
            pErr = pConnection->Errors->GetItem(i);
            printf("\t Error number: %x\t%s", pErr->Number, pErr->Description);
        }
    }
}

void PrintComError(_com_error &e) {
    _bstr_t bstrSource(e.Source());
    _bstr_t bstrDescription(e.Description());

    // Print Com errors.
    printf("Error\n");
    printf("\tCode = %08lx\n", e.Error());
    printf("\tCode meaning = %s\n", e.ErrorMessage());
    printf("\tSource = %s\n", (LPCSTR) bstrSource);
    printf("\tDescription = %s\n", (LPCSTR) bstrDescription);
}

void ofstreamTxt(char *pszConent)
{
    char szPath[256] = {0};
    GetModuleFileName(NULL, szPath , sizeof(szPath));
    char *pstr = strrchr(szPath,'\\');
    *pstr = '\0';
    strcat(szPath,"\\sql.txt");
    ofstream mycout(szPath);
    mycout<<pszConent<<endl;
    mycout.close();
}

/* Get backup_set_id from msdb database */
int MiMSSqlExecSqlADO(char *pszIP, int nPort, char *pszUser, 
    char *pszPwd, char *pszDbName, char *pszDeviceName, char *pszPath)
{
    _ConnectionPtr m_pConnection   = NULL;
    MiMSSqlstDbString *pstDbFileName = NULL;
    MiMSSqlstDbString *pstDblogName = NULL;
	char szConnStr[512]             = {0};
	char *pszSQL                    = NULL;
	char szMessage[1024]            = {0};
	char *szErrorMessage            = NULL;
    int nBackupSetId                = 0;
    int nResult                     = WAVETOP_BACKUP_OK;
    HRESULT hr                      = 0;
    _RecordsetPtr  m_pRecordset     = NULL;

    try {
        hr = m_pConnection.CreateInstance("ADODB.Connection");
        if(SUCCEEDED(hr)) {
            //1 
            PR_snprintf(szConnStr, sizeof(szConnStr), "Provider=SQLOLEDB.1;Data Source=%s,"
                "%d;User ID=%s;Password=%s;Initial Catalog=msdb",pszIP,nPort,pszUser,pszPwd);
            //2
            /*PR_snprintf(szConnStr, sizeof(szConnStr), "Provider=SQLOLEDB.1;Data Source=%s\\%s,"
                "%d;Initial Catalog=msdb",pszIP, "mssqlserver", nPort);*/

            //3
            /*PR_snprintf(szConnStr, sizeof(szConnStr), "Provider=SQLOLEDB.1;Data Source=%s\\%s,"
                "%d;Initial Catalog=msdb","127.0.0.1", "mssqlserver", nPort);*/

            //4
            //PR_snprintf(szConnStr, sizeof(szConnStr), "Provider=SQLOLEDB.1;Data Source=%s;Initial Catalog=msdb","DESKTOP-O151T26");
            

            //5
            //PR_snprintf(szConnStr, sizeof(szConnStr), "Provider=SQLOLEDB.1;Data Source=%s;Initial Catalog=msdb;User ID=%s;Password=%s;Persist Security Info=false;","DESKTOP-O151T26",pszUser,pszPwd);

            //6
            //PR_snprintf(szConnStr, sizeof(szConnStr), "Provider=SQLOLEDB.1;Data Source=%s;Initial Catalog=msdb;Integrated Security=SSPI;Persist Security Info=false;","DESKTOP-O151T26");
           // PR_snprintf(szConnStr, sizeof(szConnStr), "DRIVER={SQL Server};server=%s,%d;DATABASE=msdb;",pszIP,nPort);
            m_pConnection->CommandTimeout = 0;
            hr = m_pConnection->Open(szConnStr , "", 
                "", adModeUnknown);
        }
    }
    catch (_com_error e) {
        MSSQL_LOG_ERR("MiMSSqlExecSqlADO: m_pConnection Open error: %s.连接字符串： %s",
            (char *)e.Description(),szConnStr);
        nResult = WAVETOP_BACKUP_INTERNAL_ERROR;
        goto EXIT;
    } 

    nResult =  MSSqlGetNameFromDeviceFromADO(pszIP, nPort, pszUser, 
        pszPwd, pszDeviceName, &pstDbFileName, &pstDblogName);
    if(nResult != 0){
        MSSQL_LOG_ERR("MSSqlGetNameFromDeviceFromADO failed.errorcode:  %d.",
            nResult);
        nResult = WAVETOP_BACKUP_INTERNAL_ERROR;
        goto EXIT;
    }

	//拼装sql语句
    nResult = CreateSql(pstDbFileName, pstDblogName, pszDbName, pszDeviceName, pszPath, &pszSQL);
	if(nResult != WAVETOP_BACKUP_OK){
		MSSQL_LOG_ERR("MiCreateImportSqlFromODBCorADO failed.errorcode:  %d.DB: %s",
			nResult,pszDbName);
        nResult = WAVETOP_BACKUP_INTERNAL_ERROR;
		goto EXIT;
	}

    cout<<"sql语句长度为： "<<strlen(pszSQL)<<endl;
    ofstreamTxt(pszSQL);
    try {
       hr = m_pConnection->Execute(_bstr_t(pszSQL),NULL,adExecuteNoRecords);
       if(FAILED(hr))
        PrintProviderError(m_pConnection);
    }
    catch(_com_error &e) {
        MSSQL_LOG_ERR("%s\n",(char*)e.Description());
		nResult = WAVETOP_BACKUP_INTERNAL_ERROR;
		goto EXIT;
    } 

EXIT:
    if (m_pRecordset && m_pRecordset->State == adStateOpen)
        m_pRecordset->Close();
    if (m_pConnection && m_pConnection->State == adStateOpen)
        m_pConnection->Close();
	if(pszSQL){free(pszSQL);pszSQL = NULL;}
    MiMSSqlFreeStDbString(pstDbFileName);
    pstDbFileName = NULL;
    MiMSSqlFreeStDbString(pstDblogName);
    pstDblogName = NULL;
    return nResult;
}


//ADO 获取bak信息
int GetDBState(char *pszIP, int nPort, char *pszUser, 
    char *pszPwd,char *pszDBName)
{
    MiMSSqlstDbString *pstDbFileNameNode = NULL; 
    MiMSSqlstDbString *pstDblogNameNode  = NULL;
    _ConnectionPtr m_pConnection         = NULL;
    _RecordsetPtr  m_pRecordset          = NULL;
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

    try {
        hr = m_pConnection.CreateInstance("ADODB.Connection");
        if(SUCCEEDED(hr)) {
            PR_snprintf(szConnStr, sizeof(szConnStr), "Provider=SQLOLEDB.1;Data Source=%s,"
                "%d;Initial Catalog=msdb",pszIP,nPort);
            hr = m_pConnection->Open(szConnStr , pszUser, 
                pszPwd, adModeUnknown);
        }
    }
    catch (_com_error e) {
        MSSQL_LOG_ERR("GetDBState: m_pConnection Open error: %s",
            e.Description());
        return WAVETOP_BACKUP_INTERNAL_ERROR;
    } 

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
