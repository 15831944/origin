#include "VdiDemo.h"


int nNativeError = 0;
int MiAnsiToWideStr(char *pszStr, WCHAR *pwszStr, unsigned long nBufSize);
CVdiDemo::CVdiDemo(void)
{
    m_pVds = NULL; 
    m_pVd = NULL; 
    m_pCmd = NULL;
    m_hFile = NULL;
    m_nVirDevTimeOut = 0;
}


CVdiDemo::~CVdiDemo(void)
{
    if(m_pVds != NULL){
        m_pVds->Close();
        m_pVds->Release();
    }

    if(m_hFile != NULL)
        CloseHandle(m_hFile);

}


int CVdiDemo::init(char *pszDeviceName, char *pszInstance, char *pszFileName)
{
    HRESULT hResult = 0;
    int nRetVal;
    DWORD dwOpenFileOpt = CREATE_ALWAYS;

    if(pszDeviceName == NULL || pszInstance == NULL || pszFileName == NULL)
        return -1;
    memset(&m_stConfig, 0, sizeof(m_stConfig));
    m_stConfig.deviceCount = 1;
    m_nVirDevTimeOut = 30*1000;

    _snprintf_s(m_szDeviceName, sizeof(m_szDeviceName), pszDeviceName);
    _snprintf_s(m_szInstanceName, sizeof(m_szInstanceName), pszInstance);

    //装换成宽字符集
    nRetVal = MiAnsiToWideStr(m_szDeviceName, m_wszDeviceName, sizeof(m_wszDeviceName));
    if (0 != nRetVal) {cout<<"MiAnsiToWideStr failed"<<endl;return -2;}

    nRetVal = MiAnsiToWideStr(m_szInstanceName, m_wszInstanceName, sizeof(m_wszInstanceName));
    if (0 != nRetVal) {cout<<"MiAnsiToWideStr failed"<<endl;return -3;}

    hResult = CoCreateInstance(CLSID_MSSQL_ClientVirtualDeviceSet, NULL, CLSCTX_INPROC_SERVER, IID_IClientVirtualDeviceSet2,(void**)&m_pVds);
    if (!SUCCEEDED(hResult)) {cout<<"::CoCreateInstance() failed\n";return -4;}

    hResult = m_pVds->CreateEx(m_wszInstanceName, m_wszDeviceName, &m_stConfig);
    if (!SUCCEEDED(hResult)) {cout<<"CreateEx 失败\n";return -5;}
    cout<<"CreateEx 成功\n";

    m_hFile = CreateFile(pszFileName, GENERIC_READ|FILE_SHARE_WRITE, FILE_SHARE_READ,NULL, dwOpenFileOpt, FILE_FLAG_BACKUP_SEMANTICS, 0);
    if (INVALID_HANDLE_VALUE == m_hFile) {cout<<"CreateFile failed"<<"errro: "<<GetLastError()<<endl;return -6;}

    return 0;
}

int CVdiDemo::WriteBak()
{
    HRESULT hResult = 0;
    BOOL bResult = false;
    DWORD dwCompletionCode = ERROR_SUCCESS; 
    DWORD dwBytesTransferred = 0;

    hResult = m_pVds->GetConfiguration(5*1000, &m_stConfig);
    if (!SUCCEEDED(hResult)) {
        cout<<"VDS::GetConfiguration() failed."<<hex<<hResult<<endl;return -1;}
    cout<< "GetConfiguration 成功"<<endl;

    //打开VDI
    hResult = m_pVds->OpenDevice(m_wszDeviceName, &m_pVd);
    if (!SUCCEEDED(hResult)) { cout<<"打开VDI 失败\n";return -2;}
    Sleep(1000);

    for (;;) {
        //获取 VDI 待执行命令
        hResult = m_pVd->GetCommand(m_nVirDevTimeOut, &m_pCmd);
        if (VD_E_TIMEOUT == hResult) {cout<<"getcommand time out\n";break;}
        else if (!SUCCEEDED(hResult)) { break;}

        switch (m_pCmd->commandCode) {
        case VDC_Read:
            bResult = ReadFile(m_hFile, m_pCmd->buffer, 
                m_pCmd->size, &dwBytesTransferred, 0);
            if (dwBytesTransferred == m_pCmd->size && bResult) {dwCompletionCode = ERROR_SUCCESS;}
            else {dwCompletionCode = ERROR_HANDLE_EOF;}
            break;

        case VDC_Write:
            bResult = WriteFile(m_hFile, m_pCmd->buffer, m_pCmd->size, &dwBytesTransferred, 0);
            if (dwBytesTransferred == m_pCmd->size && bResult) {
                dwCompletionCode = ERROR_SUCCESS;
            }
            else {
                cout<<"VDI::GetCommand() failed, stream id, can't write data to file\n";
                /* assume failure is disk full */
                dwCompletionCode = ERROR_DISK_FULL;
            }
            break;
        case VDC_Flush:
            break;
        case VDC_ClearError:
            dwCompletionCode = ERROR_SUCCESS;
            break;
        default:
            //cout<<"VDI::GetCommand() failed, stream id %d, unknown command\n";
            /*command is unknown... */
            dwCompletionCode = ERROR_NOT_SUPPORTED;
            break;
        }
        //通知 SQLServer服务器 命令执行完成
        hResult = m_pVd->CompleteCommand(m_pCmd, dwCompletionCode, dwBytesTransferred, 0);
        if (!SUCCEEDED(hResult)) {cout<<"VDI::CompleteCommand() failed\n";break;}
    }

    if (VD_E_CLOSE != hResult) {
        cout<<"VDI failed \n";
        return hResult;
    }else {
        cout<<"VDI success\n";
        return 0;
    }
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

void MY_thread(void *param)
{
    CAdo cado;
    AdoParamSt *pstPar = (AdoParamSt *)param;
    

    cado.BackupDBVDI(pstPar->szDbName, pstPar->szDeviceName);
    cout<<"线程结束\n";
    return;
}

unsigned __stdcall ADOThread( void* pArguments )
{
    printf( "In ADOThread...\n" );

    CAdo *cado = new CAdo;
    AdoParamSt *pstPar = (AdoParamSt *)pArguments;
    CoInitialize(NULL);
    cado->Init(pstPar->szIP, pstPar->nPort, pstPar->szUser, pstPar->szPwd);
    cado->BackupLogVDI(pstPar->szDbName, pstPar->szDeviceName, pstPar->nDBNativeError);
    delete cado;
    CoUninitialize();
    printf( "Out ADOThread...\n" );
    _endthreadex( 0 );
    return 0;
}

int CVdiDemo::VDI_ADO_Test()
{
    AdoParamSt *pstPar = (AdoParamSt*)malloc(sizeof(AdoParamSt));
    char szPath[260] = "H:\\1\\test_ndf.bak";

    HANDLE hThread;
    unsigned threadID;

    _snprintf_s(pstPar->szIP, sizeof(pstPar->szIP), "2.2.2.240");
    pstPar->nPort = 1433;
    pstPar->nDBNativeError = 0;
    _snprintf_s(pstPar->szUser, sizeof(pstPar->szUser), "sa");
    _snprintf_s(pstPar->szPwd, sizeof(pstPar->szPwd), "Root@123");
    _snprintf_s(pstPar->szDbName, sizeof(pstPar->szDbName), "test_ndf");
    _snprintf_s(pstPar->szDeviceName, sizeof(pstPar->szDeviceName), "test_ndf1557223733");
    _snprintf_s(pstPar->szInstance, sizeof(pstPar->szInstance), "MSSQLSERVER");

    int nRC = init(pstPar->szDeviceName, pstPar->szInstance, szPath);
    if(nRC != 0){
        cout<<"vdi init error\n";
        return -1;
    }
        
    //CreateThread
    printf( "Creating ADO thread...\n" );

    // Create the ADOThread
    hThread = (HANDLE)_beginthreadex( NULL, 0, &ADOThread, pstPar, 0, &threadID );
    if(hThread == 0){

    }

    nRC = WriteBak();
    if(nRC != 0){
        cout<<"WriteBak error\n";
    }

    if(pstPar->nDBNativeError == 4214)
        cout<<"\n-------------------------\n数据库执行失败 错误码为 4214\n-------------------\n";
    else
        cout<<"\n-------------------------\n数据库执行成功 错误码为"<<pstPar->nDBNativeError<<"\n-------------------\n";

   /* if(nNativeError == 4214){
        cout<<"\n-------------------------\n数据库执行失败 错误码为 4214\n-------------------\n";
    } else
        cout<<"\n-------------------------\n数据库执行失败 错误码为"<<pstPar->nDBNativeError<<"\n-------------------\n";*/

    return 0;

}
void main123()
{
    CVdiDemo cv1;

    CoInitialize(NULL);

    
    cv1.VDI_ADO_Test();

    CoUninitialize();
    system("pause");

}