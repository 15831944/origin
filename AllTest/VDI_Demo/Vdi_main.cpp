
#include "vdi_function.h"



int MiPerform2();
int MiAnsiToWideStr(char *pszStr, WCHAR *pwszStr, unsigned long nBufSize);
int main()
{

	int nRC = MiPerform2();
	cout<<"MiPerform2 执行结果为 ："<<nRC<<endl;
	system("pause");

    return 0;
}
int MiAnsiToWideStr(char *pszStr, WCHAR *pwszStr, unsigned long nBufSize)
{
	int nRetVal;


	nRetVal = MultiByteToWideChar(CP_ACP, 0, pszStr, strlen(pszStr) + 1, 
		pwszStr, nBufSize / sizeof(WCHAR));
	if (0 == nRetVal) {
		cout<<"MultiByteToWideChar error"<<endl;
		return -1;
	}

	return 0;
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
void MY_Thread2(void *param)
{
    MiMSSqlstDbString*       pstDbFileNameNode          = NULL; 
    MiMSSqlstDbString*       pstDblogNameNode           = NULL;
    _ConnectionPtr           m_pConnection              = NULL;
    _RecordsetPtr            m_pRecordset               = NULL;
    HRESULT                  hr                         = S_OK;
    char                     szConnStr[512]             = {0};
    char                     szSql[2048]                = {0};
    char                     szType[2048]               = {0};
    int                      nResult                    = WAVETOP_BACKUP_OK;
    int                      nLen                       = 0;
    _bstr_t                  bstrRowInfo;
    _variant_t               vName;
    _variant_t               vType;
    MiMSSqlstDbString*      pstDbFileName = NULL;
    MiMSSqlstDbString*      pstDblogName = NULL;


    try {
        hr = m_pConnection.CreateInstance("ADODB.Connection");
        if(SUCCEEDED(hr)) {
            PR_snprintf(szConnStr, sizeof(szConnStr), "Provider=SQLOLEDB.1;Data Source=127.0.0.1,1433;Initial Catalog=msdb");
            hr = m_pConnection->Open(szConnStr ,"sa", 
                "Root@123", adModeUnknown);
        }
    }
    catch (_com_error e) {
        MSSQL_LOG_ERR("MiMSSqlGetNameFromDeviceFromADO: m_pConnection Open error: %s",
            e.Description());
        return ;
    } 

    PR_snprintf(szSql, sizeof(szSql),
        "if not exists(SELECT * FROM master..sysdatabases WHERE name = N'DB10031') CREATE DATABASE [DB10031]"
        "RESTORE DATABASE  [DB10031] FROM  VIRTUAL_DEVICE='syn_db1006_full' WITH REPLACE, RECOVERY,"
        "move 'db1003_1' to 'H:\\DBdata\\DB1003_data_0.mdf', move 'db1003' to 'H:\\DBdata\\DB1003_data_1.ndf', move 'db1003_log' to 'H:\\DBdata\\DB1003_log_0.ldf'");

    hr = m_pRecordset.CreateInstance(__uuidof(Recordset));
    if(!SUCCEEDED(hr)){
        cout<<"m_pRecordset createInstance error\n";
        if (m_pConnection)
            if (m_pConnection->State == adStateOpen)
                m_pConnection->Close();
        return;
    }
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
        return ;
    } 

    try { 
        while (!m_pRecordset->EndOfFile) {
            bstrRowInfo = m_pRecordset->GetFields()->GetItem("Type")->GetValue();
            strncpy(szType, (char *)bstrRowInfo, sizeof(szType));

            /* Database name*/
            if (!strcmp(szType,"D")) {
                bstrRowInfo = m_pRecordset->GetFields()->GetItem("LogicalName")->GetValue();

                /* judge the length of sqlquery */
                nLen = strlen((char *)bstrRowInfo);
                if (nLen > 2048) {
                    nResult = WAVETOP_BACKUP_FILE_TOO_LARGE;
                    break;
                }

                pstDbFileNameNode = (MiMSSqlstDbString *)malloc(sizeof(MiMSSqlstDbString));
                if (!pstDbFileNameNode) {
                    nResult = WAVETOP_BACKUP_FILE_TOO_LARGE;
                    break;
                }

                strcpy(pstDbFileNameNode->szDbString, (char *)bstrRowInfo);

                pstDbFileNameNode->pNext = pstDbFileName;
                pstDbFileName = pstDbFileNameNode; 
            }
            /* log name */
            else if (!strcmp(szType,"L")) {
                /* get db logical name */
                bstrRowInfo = m_pRecordset->GetFields()->GetItem("LogicalName")->GetValue();

                /* judge the length of sqlquery */
                nLen = strlen((char *)bstrRowInfo);
                if (nLen > 2048) {
                    nResult = WAVETOP_BACKUP_FILE_TOO_LARGE;
                    break; 
                }

                pstDblogNameNode = (MiMSSqlstDbString *)malloc(sizeof(MiMSSqlstDbString));
                if (!pstDblogNameNode) {
                    nResult = WAVETOP_BACKUP_FILE_TOO_LARGE;
                    break;
                }

                strcpy(pstDblogNameNode->szDbString, (char *)bstrRowInfo);

                pstDblogNameNode->pNext = pstDblogName;
                pstDblogName = pstDblogNameNode;
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
    if (1) {
        MiMSSqlFreeStDbString(pstDbFileName);
        pstDbFileName = NULL;
        MiMSSqlFreeStDbString(pstDblogName);
        pstDblogName = NULL;
    }
    if(nResult != 0){
        cout<<"函数执行不成功\n";
    }else{
        cout<<"函数执行成功\n";
    }
    return ;
}

void MY_adoexec(void *param)
{
    MiMSSqlstDbString*       pstDbFileNameNode          = NULL; 
    MiMSSqlstDbString*       pstDblogNameNode           = NULL;
    _ConnectionPtr           m_pConnection              = NULL;
    _RecordsetPtr            m_pRecordset               = NULL;
    HRESULT                  hr                         = S_OK;
    char                     szConnStr[512]             = {0};
    char                     szSql[2048]                = {0};
    char                     szType[2048]               = {0};
    int                      nResult                    = WAVETOP_BACKUP_OK;
    int                      nLen                       = 0;
    _bstr_t                  bstrRowInfo;
    _variant_t               vName;
    _variant_t               vType;
    MiMSSqlstDbString*      pstDbFileName = NULL;
    MiMSSqlstDbString*      pstDblogName = NULL;


    try {
        hr = m_pConnection.CreateInstance("ADODB.Connection");
        if(SUCCEEDED(hr)) {
            PR_snprintf(szConnStr, sizeof(szConnStr), "Provider=SQLOLEDB.1;Data Source=127.0.0.1,1433;Initial Catalog=msdb");
            hr = m_pConnection->Open(szConnStr ,"sa", 
                "Root@123", adModeUnknown);
        }
    }
    catch (_com_error e) {
        MSSQL_LOG_ERR("MiMSSqlGetNameFromDeviceFromADO: m_pConnection Open error: %s",
            e.Description());
        return ;
    } 

    PR_snprintf(szSql, sizeof(szSql),
        "RESTORE DATABASE [DB1003] FROM  VIRTUAL_DEVICE='db1003#' WITH REPLACE,"
        "RECOVERY,move 'db1003' to 'H:\\DBdata\\DB1003_data_0.mdf', move 'db1003_log' to 'H:\\DBdata\\DB1003_log_0.ldf'");

    hr = m_pRecordset.CreateInstance(__uuidof(Recordset));
    if(!SUCCEEDED(hr)){
        cout<<"m_pRecordset createInstance error\n";
        if (m_pConnection)
            if (m_pConnection->State == adStateOpen)
                m_pConnection->Close();
        return;
    }
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
        return ;
    } 

    try { 
        while (!m_pRecordset->EndOfFile) {
            bstrRowInfo = m_pRecordset->GetFields()->GetItem("Type")->GetValue();
            strncpy(szType, (char *)bstrRowInfo, sizeof(szType));

            /* Database name*/
            if (!strcmp(szType,"D")) {
                bstrRowInfo = m_pRecordset->GetFields()->GetItem("LogicalName")->GetValue();

                /* judge the length of sqlquery */
                nLen = strlen((char *)bstrRowInfo);
                if (nLen > 2048) {
                    nResult = WAVETOP_BACKUP_FILE_TOO_LARGE;
                    break;
                }

                pstDbFileNameNode = (MiMSSqlstDbString *)malloc(sizeof(MiMSSqlstDbString));
                if (!pstDbFileNameNode) {
                    nResult = WAVETOP_BACKUP_FILE_TOO_LARGE;
                    break;
                }

                strcpy(pstDbFileNameNode->szDbString, (char *)bstrRowInfo);

                pstDbFileNameNode->pNext = pstDbFileName;
                pstDbFileName = pstDbFileNameNode; 
            }
            /* log name */
            else if (!strcmp(szType,"L")) {
                /* get db logical name */
                bstrRowInfo = m_pRecordset->GetFields()->GetItem("LogicalName")->GetValue();

                /* judge the length of sqlquery */
                nLen = strlen((char *)bstrRowInfo);
                if (nLen > 2048) {
                    nResult = WAVETOP_BACKUP_FILE_TOO_LARGE;
                    break; 
                }

                pstDblogNameNode = (MiMSSqlstDbString *)malloc(sizeof(MiMSSqlstDbString));
                if (!pstDblogNameNode) {
                    nResult = WAVETOP_BACKUP_FILE_TOO_LARGE;
                    break;
                }

                strcpy(pstDblogNameNode->szDbString, (char *)bstrRowInfo);

                pstDblogNameNode->pNext = pstDblogName;
                pstDblogName = pstDblogNameNode;
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
    if (1) {
        MiMSSqlFreeStDbString(pstDbFileName);
        pstDbFileName = NULL;
        MiMSSqlFreeStDbString(pstDblogName);
        pstDblogName = NULL;
    }
    if(nResult != 0){
        cout<<"函数执行不成功\n";
    }else{
        cout<<"函数执行成功\n";
    }
    return ;
}

int Process_exec_isql()
{
    SECURITY_ATTRIBUTES SecurityAtt;
    STARTUPINFO ISQLStartupInfo;
    PROCESS_INFORMATION stProcInfo;
    DWORD dwBytes;
    BOOL bResult;
    int nRetVal;
    char szSql2[1024] = "\"isql.exe\" -S \"2.2.2.240\\MSSQLSERVER,1433\" -U \"sa\" -P \"Root@123\" -Q \"RESTORE DATABASE [DB1003] FROM  VIRTUAL_DEVICE='db1003#' WITH REPLACE,"
        "RECOVERY,move 'db1003' to 'H:\\DBdata\\DB1003_data_0.mdf', move 'db1003_log' to 'H:\\DBdata\\DB1003_log_0.ldf'\"";

    /* isql command start info */
    memset(&ISQLStartupInfo, 0, sizeof(ISQLStartupInfo));
    ISQLStartupInfo.cb = sizeof(ISQLStartupInfo);
    ISQLStartupInfo.dwFlags = STARTF_USESHOWWINDOW|STARTF_USESTDHANDLES;
    ISQLStartupInfo.wShowWindow = SW_SHOWNORMAL;

    if(CreateProcess(NULL,szSql2, NULL, NULL,FALSE, CREATE_NEW_CONSOLE,NULL, NULL,&ISQLStartupInfo,&stProcInfo)) {  
        CloseHandle(stProcInfo.hThread);
        cout << "create process success" << endl;  
    }else
        cout<<"CreateProcess failed"<<endl;
    return 0;
}

int MiPerform2()
{
	IClientVirtualDeviceSet2 *pVds = NULL; 
    IClientVirtualDevice *pVd = NULL; 
    VDC_Command *pCmd = NULL;
    HRESULT hResult;
    HANDLE hFile;
    DWORD dwCompletionCode = ERROR_SUCCESS; 
    DWORD dwBytesTransferred;
    DWORD dwOpenFileOpt = OPEN_EXISTING;
    WCHAR wszDeviceName[512];
    char szFilename[512] = "D:\\db1003#";
    char szDeviceName[512] = "db1003#";
    BOOL bResult;
    int nRetVal;
    int nQuit = 1;
    BOOL bSync = true;
    void *pTempAddr = NULL;
    PRUint64 nBakSize = 0;
    PRUint64 nTotalSize = 0;
	DWORD nVirDevTimeOut = 120000;
	WCHAR *pInstanceName = L"MSSQLSERVER";
	VDConfig stConfig = {0};

	CoInitialize(NULL);

    /* create virtual device */
	hResult = CoCreateInstance(CLSID_MSSQL_ClientVirtualDeviceSet, NULL, CLSCTX_INPROC_SERVER, IID_IClientVirtualDeviceSet2,(void**)&pVds);
	if (!SUCCEEDED(hResult)) {cout<<"::CoCreateInstance() failed\n";return -5;}

	//VDI名称 转换为 宽字符集
	nRetVal = MiAnsiToWideStr(szDeviceName, wszDeviceName, sizeof(wszDeviceName));
    if (0 != nRetVal) {cout<<"MiAnsiToWideStr failed"<<endl;return -1;}
	stConfig.deviceCount = 1;


    //stConfig需要置0，deviceCount = 1; 否则创建失败
    hResult = pVds->CreateEx(pInstanceName, wszDeviceName, &stConfig);
	if (!SUCCEEDED(hResult)) {cout<<"CreateEx 失败\n";return -1;}
	cout<<"CreateEx 成功\n";

    //执行sql命令
    Process_exec_isql();

	hResult = pVds->GetConfiguration(0 == nVirDevTimeOut ? 
		120000 : nVirDevTimeOut, &stConfig);
	if (!SUCCEEDED(hResult)) {
		cout<<"VDS::GetConfiguration() failed."<<hex<<hResult<<endl;;pVds->Close();pVds->Release();return -1;}
    cout<< "GetConfiguration 成功"<<endl;
    
	//创建备份文件
    hFile = CreateFile(szFilename, GENERIC_READ|FILE_SHARE_WRITE, FILE_SHARE_READ,NULL, dwOpenFileOpt, FILE_FLAG_BACKUP_SEMANTICS, 0);
    if (INVALID_HANDLE_VALUE == hFile) {cout<<"CreateFile failed"<<"errro: "<<GetLastError()<<endl;return -4;}

	//打开VDI
    hResult = pVds->OpenDevice(wszDeviceName, &pVd);
    if (!SUCCEEDED(hResult)) { cout<<"打开VDI 失败\n";return -2;}

    for (;;) {
		//获取 VDI 待执行命令
        hResult = pVd->GetCommand(nVirDevTimeOut, &pCmd);
        if (VD_E_TIMEOUT == hResult) {MSSQL_LOG_ERR("getcommand time out\n");break;}
        else if (!SUCCEEDED(hResult)) { break;}

        switch (pCmd->commandCode) {
        case VDC_Read:
            bResult = ReadFile(hFile, pCmd->buffer, 
                pCmd->size, &dwBytesTransferred, 0);
            if (dwBytesTransferred == pCmd->size && bResult) {dwCompletionCode = ERROR_SUCCESS;}
            else {dwCompletionCode = ERROR_HANDLE_EOF;}
            break;

        default:
            //cout<<"VDI::GetCommand() failed, stream id %d, unknown command\n";
            /*command is unknown... */
            dwCompletionCode = ERROR_NOT_SUPPORTED;
            break;
        }
		//通知 SQLServer服务器 命令执行完成
        hResult = pVd->CompleteCommand(pCmd, dwCompletionCode, dwBytesTransferred, 0);
        if (!SUCCEEDED(hResult)) {cout<<"VDI::CompleteCommand() failed\n";break;}
    }
    CloseHandle(hFile);
    if (VD_E_CLOSE != hResult) {
		cout<<"VDI failed \n";
        return hResult;
    }else {
        cout<<"VDI success\n";
        return 0;
    }
}
