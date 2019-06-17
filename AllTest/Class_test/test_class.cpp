#include "Test_Class.h"


Test_Class::Test_Class(void)
{
}


Test_Class::~Test_Class(void)
{
}

void Test_Class::test_FailedFileLog()
{
    FailedFileLog fl("D:/");
    fl.Init();
    fl.Write("sadf");
    fl.Flush();
}

void Test_Class::Test_CharAssign()
{
    CharAssign C1;
    char *input = NULL;
    C1.Assign(input);
    if(input)
        cout<<input<<endl;
}


int Test_Class:: GetBinaryKey_test(int nCurBSId, int nResHisId, int nResBsId, 
    unsigned char *pszBinary, int nSize)
{
    char szCurBSId[32];
    char szResHisId[32];
    char szResBsId[32];
    char cHead;
    char cLength;
    char cType;
    char cTotalLength = 1;
    int nResult = 0;
    int nIsSaved = 0;
    int nPos = 2;

    memset(pszBinary, 0, nSize);
    memset(szCurBSId, 0, sizeof(szCurBSId));
    memset(szResHisId, 0, sizeof(szResHisId));
    memset(szResBsId, 0, sizeof(szResBsId));
    PR_snprintf(szCurBSId, sizeof(szCurBSId), "%d", nCurBSId);
    PR_snprintf(szResHisId, sizeof(szResHisId), "%d", nResHisId);
    PR_snprintf(szResBsId, sizeof(szResBsId), "%d", nResBsId);

    /* binary key version 1 */
    pszBinary[0] = 1;
    cHead = 0;
    if (strlen(szCurBSId) > nSize - nPos) {
        return 3;
    }
    cType = 1;
    cLength = strlen(szCurBSId);
    cHead = ((int)cType << 5) | (int)cLength;
    pszBinary[nPos] = cHead;
    cTotalLength = (int)cTotalLength + 1;
    nPos++;
    memcpy(pszBinary + nPos, szCurBSId, (int)cLength);
    nPos += (int)cLength;
    cTotalLength += (int)cLength;

    if (nResHisId != 0 && nResBsId != 0) {
        cHead = 0;
        if (strlen(szResHisId) > nSize - nPos) {
            return 3;
        }
        cType = 2;
        cLength = strlen(szResHisId);
        cHead = ((int)cType << 5) | (int)cLength;
        pszBinary[nPos] = cHead;
        cTotalLength = (int)cTotalLength + 1;
        nPos++;
        memcpy(pszBinary + nPos, szResHisId, (int)cLength);
        nPos += (int)cLength;
        cTotalLength += (int)cLength;

        cHead = 0;
        if (strlen(szResBsId) > nSize - nPos) {
            return 3;
        }
        cType = 3;
        cLength = strlen(szResBsId);
        cHead = ((int)cType << 5) | (int)cLength;
        pszBinary[nPos] = cHead;
        cTotalLength = (int)cTotalLength + 1;
        nPos++;
        memcpy(pszBinary + nPos, szResBsId, (int)cLength);
        nPos += (int)cLength;
        cTotalLength += (int)cLength;
    }

    pszBinary[1] = cTotalLength;

    return nResult;
}

int Test_Class::Time_Test()
{
    char szBuff[216] = {0};
    int a = time(NULL);
    printf("现在的时间为： %d",time(NULL));
    int b = GetPrivateProfileInt("TT","nVsersion", 0 , "H:\\test_file\\1.conf");
    GetPrivateProfileInt("TT","nbackup_time", 0 , "H:\\test_file\\1.conf");
     //GetPrivateProfileString("server","Spantime","",szBuff,sizeof(szBuff),"D:\\test.conf");
    GetPrivateProfileString("TT","nVserion","",szBuff , sizeof(szBuff),"H:\\test_file\\1.conf");
    __int64 e = atoi(szBuff);
    return 0;
}

int Test_Class::ConstCharMalloc()
{
    const char *pszUser = NULL;
    pszUser = "hello";
    cout<<pszUser<<endl;
    pszUser = "Wei";
    cout<<pszUser;
    return 0;
}

void Test_Class::Test_atoll()
{
    char *pszTemp = " -   10373453454332";
    CharAssign c1;
    long long a = c1.myatoll(pszTemp);
    cout<<a<<endl;
}

void Test_Class::Stdlib_atoll()
{
    char *pszTemp = "3555990923";
    unsigned long a = _atoi64(pszTemp);
    unsigned long b = atoi(pszTemp);

    cout<<a<<endl;
}

void Test_Class::Test_Sprintf()
{
    char szSbuff[1024] = {0};
    char szLimited[5] = {0};
    char *pszName = NULL;
    sprintf(szSbuff, "hello%s",pszName);
    cout<<szSbuff<<endl;

    pszName = "helloeverybody";
    sprintf(szLimited,pszName);
    cout<<szLimited<<endl;
}

void Test_Class::Test_createProcess()
{
    char buffer[30000] = {0};
    int hResult = 0;
    DWORD readByte = 0;
    int len = 0;
    DWORD dwRet;
    bool bRetval = true;
    DWORD nISQLExitCode = 0;
    DWORD leftbyte = 0;
    DWORD readByte2 = 0;
    string strOpt;

    CCreateProccess c1;
    PR_snprintf(c1.szCmd,sizeof(c1.szCmd), 
        "\"isql.exe\" -S \"2.2.2.40\\MSSQLSERVER,1433\" -U \"sa\" -P \"Root@123\" -Q \"BACKUP  DATABASE  [db1019] TO disk='C:\\bak\\db1019_1556519202#' WITH COMPRESSION,BLOCKSIZE = 65536\"");
    c1.SetSecurityAtt();
    c1.EXECCmd();
    bRetval = GetExitCodeProcess(c1.pi.hProcess, &nISQLExitCode);
    if(!bRetval){
        nISQLExitCode = 3;
    }
    if(nISQLExitCode == STILL_ACTIVE){
        while(1){
            PeekNamedPipe(c1.hOutPut, buffer, 1024,&readByte, 0, &leftbyte);
            if(readByte != 0){
                hResult = ReadFile(c1.hOutPut,buffer,readByte,&readByte2,NULL);
                strOpt += buffer;
                if(hResult == TRUE && readByte != 0)
                    continue;
                else
                    break;
            }else
                break;

        } 

    }

    dwRet = WaitForSingleObject(c1.pi.hProcess,INFINITE);
    switch(dwRet)
    {
    case WAIT_TIMEOUT:
    case WAIT_FAILED:
        return ;
    case WAIT_OBJECT_0:
        //CloseHandle(c1.hInPut);//close hInPut handle ,make hte write pipe completes
        CloseHandle(c1.pi.hProcess);
        CloseHandle(c1.pi.hThread);
    }
   /* while(1){
        dwRet = WaitForSingleObject(c1.pi.hProcess,2*1000);
        if(dwRet == WAIT_TIMEOUT){

                hResult = ReadFile(c1.hOutPut,buffer+len,1024,&readByte,NULL);
                len += readByte;
                Sleep(200);
                if(hResult == TRUE && readByte != 0)
                    continue;
                else
                    break;
        }else if(dwRet == WAIT_FAILED){
            cout<<"waitforSingleObject failed"<<endl;
            break;
        }else if(dwRet == WAIT_OBJECT_0){
            cout<<"程序正常退出\n";
            CloseHandle(c1.hInPut);//close hInPut handle ,make hte write pipe completes
            CloseHandle(c1.pi.hProcess);
            CloseHandle(c1.pi.hThread);
            break;
        }
    }*/
    if (NULL != c1.hOutPut) {
        bRetval = PeekNamedPipe(c1.hOutPut, buffer, 
            sizeof(buffer), &readByte, NULL, NULL);
        if (!bRetval) {
            buffer[0] = '\0';
            cout<<"PeekNamedPipe() StdErrRead failed\n";
        }
        else {
            buffer[readByte] = '\0';
            if (buffer[0] != '\0') {
                cout<<buffer<<endl;
            }
        }

        CloseHandle(c1.hOutPut);
    }
    else {
        buffer[0] = '\0';
    }
    /*do
    {  
        //a write operation completes on the write end of the pipe,so ReadFile can begin
        hResult = ReadFile(c1.hOutPut,buffer+len,1024,&readByte,NULL);
        cout<<readByte<<endl;
        len += readByte;
        Sleep(200);
    }
    while(readByte!=0 && hResult);*/

    if(c1.hInPut)
        CloseHandle(c1.hInPut);
    cout<<"len : "<<len<<";\n buffer: "<<buffer<<endl;


    /*dwRet = WaitForSingleObject(c1.pi.hProcess,INFINITE);
    switch(dwRet)
    {
    case WAIT_TIMEOUT:
    case WAIT_FAILED:
        return ;
    case WAIT_OBJECT_0:
        CloseHandle(c1.hInPut);//close hInPut handle ,make hte write pipe completes
        CloseHandle(c1.pi.hProcess);
        CloseHandle(c1.pi.hThread);
    }*/

    
    /*do
    {  
        //a write operation completes on the write end of the pipe,so ReadFile can begin
        hResult = ReadFile(c1.hOutPut,buffer+len,1024,&readByte,NULL);
        cout<<readByte<<endl;
        len += readByte;
        Sleep(200);
    }
    while(readByte!=0 && hResult);

    cout<<buffer<<endl;*/

   // CloseHandle(c1.hOutPut);
    return ;
}

void Test_Class::Ado_LogBackupTest()
{
    char*   pszIP           = "2.2.2.40";
    int     nPort           = 1433;
    char*   pszUser         = "sa"; 
    char*   pszPwd          = "Root@123";
    char*   pszDbName       = "db1021";
    char*   pszPath   = "C:\\bak";
    CAdo c1;

    CoInitialize(NULL);

    c1.Init(pszIP, nPort, pszUser, pszPwd);
    c1.LogBackup(pszDbName, pszPath);

   CoUninitialize();
}

void Test_Class::Ado_GetDBVersionTest()
{
    char*   pszIP           = "2.2.2.40";
    int     nPort           = 1433;
    char*   pszUser         = "sa"; 
    char*   pszPwd          = "Root@123";
    char*   pszDbName       = "db1021";
    char*   pszPath   = "C:\\bak";
    CAdo c1;

    CoInitialize(NULL);

    c1.Init(pszIP, nPort, pszUser, pszPwd);
    c1.GetDBVersion();

    CoUninitialize();
}

void Test_Class::Ado_ScriptTest()
{
    char*   pszIP           = "2.2.2.40";
    int     nPort           = 1433;
    char*   pszUser         = "sa"; 
    char*   pszPwd          = "Root@123";
    char*   pszDbName       = "db1021";
    char*   pszPath   = "C:\\bak";
    CAdo c1;
    int nRC = 0;

    CoInitialize(NULL);

    nRC = c1.Init(pszIP, nPort, pszUser, pszPwd);
    if(nRC != 0){
        cout<<"init 失败\n";
        return ;
    }
        
    nRC = c1.ConnectDB();
    if(nRC != 0){
        cout<<"connectDB 失败\n";
        return ;
    }
    nRC = c1.EXECscript();
    if(nRC != 0){
        cout<<"EXECscript 失败\n";
        return ;
    }

    CoUninitialize();
}

void Test_Class::TestParam(int a,int b,int c,int d,int e,int f, int g,int a2,int b2,int c2,int d2,int e2,int f2, int g2,int g3)
{
    return;
}

bool Test_Class::GetFlag() const
{
    m_AccessCount++;
    return m_flag;
}


void Test_Class::Ado_MemoryTest()
{
    CAdo c1;
    cout<<sizeof(c1)<<endl;

}

void Test_Class::unsingedCharTest()
{
    CharAssign c1;
    c1.Myunsignedchar(0x80);
    c1.Myunsignedchar(0x7f);
}




int Test_Class::memTest( void )
{
    char str1[7] = "aabbcc";
    printf( "The string: %s\n", str1 );
    memcpy( str1 + 2, str1, 4 );
    printf( "New string: %s\n", str1 );

    strcpy_s( str1, sizeof(str1), "aabbcc" );   // reset string

    printf( "The string: %s\n", str1 );
    memmove( str1 + 2, str1, 4 );
    printf( "New string: %s\n", str1 );
    return 0;
}

int Test_Class::MymemTest( void )
{
    CharAssign c1;
    char str1[7] = "aabbcc";
    printf( "The string: %s\n", str1 );
    c1.memcpy( str1 + 2, str1, 4 );
    printf( "New string: %s\n", str1 );

    strcpy_s( str1, sizeof(str1), "aabbcc" );   // reset string

    printf( "The string: %s\n", str1 );
    c1.memmove( str1 + 2, str1, 4 );
    printf( "New string: %s\n", str1 );

    strcpy_s( str1, sizeof(str1), "aabbcc" );   // reset string
    char str2[50] = {0}; 
    printf( "The string: %s\n", str1 );
    c1.memcpy(str2, str1, sizeof(str1));
    printf( "New string: %s\n", str2 );
    return 0;
}

void Test_Class::ReadfileTest()
{
    CReadfile cread("H:\\AllTest\\dist\\script.txt");
    cread.readbuffer();
}

void Test_Class::test_and_or()
{
    int a,b,c,d,e,f;
    a= 0;
    a = 0xFFFFFFFF;
    b = a|0x01 ;
    cout<<"b:"<<b<<endl;

    c = a|0x02;
    cout<<"c: "<<c<<endl;

    d = a&0x01;
    cout<<"d: "<<d<<endl;

    e = a&0x02;
    cout<<"e: "<<e<<endl;

    f = 1|2;
    cout<<"f: "<<f<<endl;
}

void Test_Class::GetFullNamePathTest()
{
    char *pszTemp = NULL;
    char *pszTemp2 = NULL;
    char szstr[] = "C:\\.\\DBdata\\1\\..\\..\\DBdata\\db1.mdf";
    char szstr1[1024] = {};
    //StringCchPrintfA(szstr1,sizeof(szstr1),szstr);

    pszTemp = strstr(szstr1,"..\\");
    if(pszTemp != NULL){

    }
    int nRC = GetFullPathName(szstr1, sizeof(szstr1), szstr1, NULL);
    cout<<nRC<<endl;




    //pszTemp = strstr(szstr1, ".\\");
    // cout<<pszTemp<<endl;

    //memmove(pszTemp ,pszTemp +2, strlen(pszTemp+2) + 1);
    cout<<szstr1<<endl;
}

void Test_Class::Ado_ShrinkDBTest()
{
    char*   pszIP           = "2.2.2.40";
    int     nPort           = 1433;
    char*   pszUser         = "sa"; 
    char*   pszPwd          = "Root@123";
    char*   pszDbName       = "db1002";
    char*   pszPath   = "C:\\bak";
    time_t  pretime;
    CAdo c1;

    CoInitialize(NULL);

    c1.Init(pszIP, nPort, pszUser, pszPwd);
    pretime   = time(NULL);
    c1.ShrinkDB(pszDbName);
    cout<<"耗时： "<<time(NULL) - pretime<<endl;

    CoUninitialize();
}

int Test_Class::Ado_GetStandByPath()
{
    char*   pszIP           = "2.2.2.40";
    int     nPort           = 1433;
    char*   pszUser         = "sa"; 
    char*   pszPwd          = "Root@123";
    char*   pszDbName       = "db1002";
    char*   pszPath   = "C:\\bak";
    time_t  pretime;
    _bstr_t bstrTemp;
    char szstr[260] = {};
    int nRC = 0;
    CAdo c1;

    CoInitialize(NULL);

    nRC = c1.Init(pszIP, nPort, pszUser, pszPwd);
    if(nRC != 0){
        cout<<":: Ado_GetStandByPath init failed "<<endl;
        return nRC;
    }

    nRC = c1.ConnectDB();
    if(nRC != 0){
        cout<<":: Ado_GetStandByPath ConnectDB failed "<<endl;
        return nRC;
    }

    c1.m_bstrSql = "DECLARE  @backup_path NVARCHAR(255);";
    c1.m_bstrSql += _bstr_t("EXEC xp_instance_regread N'HKEY_LOCAL_MACHINE', N'Software\\Microsoft\\MSSQLServer\\MSSQLServer', N'BackupDirectory', @backup_path OUTPUT;");
    c1.m_bstrSql +=  _bstr_t(" select @backup_path");

    nRC = c1.ExecSqlWithRecord();
    if(nRC != 0){
        cout<<":: Ado_GetStandByPath ExecSqlWithRecord failed "<<endl;
        return nRC;
    }


    try { 
        while (!c1.m_pRecordset->EndOfFile) {
            bstrTemp = c1.m_pRecordset->GetFields()->GetItem("")->GetValue();
            StringCchPrintfA(szstr, sizeof(szstr), (char *)bstrTemp);
            cout<<szstr<<endl;
            c1.m_pRecordset->MoveNext();
        }
    }

    catch(_com_error e)	{
        MSSQL_LOG_ERR("m_pRecordset GetData error: %s",
            e.Description());
        return WAVETOP_BACKUP_INTERNAL_ERROR;
    }

}

void Test_Class::GetOsVersion()
{
    OSVERSIONINFO osvi;
    BOOL bIsWindowsXPorLater;

    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    GetVersionEx(&osvi);

    bIsWindowsXPorLater = 
        ( (osvi.dwMajorVersion > 5) ||
        ( (osvi.dwMajorVersion == 5) && (osvi.dwMinorVersion >= 1) ));

    if(bIsWindowsXPorLater)
        printf("The system meets the requirements.\n");
    else printf("The system does not meet the requirements.\n");

}

LONGLONG Test_Class::BkGetFilesTotalSize(string pszBackupFiles, LONGLONG &nTotalSize, LONGLONG &nFileNum)
{
    long hFile              = 0;
    string pathName         = "";
    string dirPathTemp      = "";
    string fileNameTemp     = "";
    struct _finddata_t fileInfo;
    struct _stat64 info;
    
    // 此处"/*"代表遍历所有的类型
    if((hFile = _findfirst(pathName.assign(pszBackupFiles).append("/*").c_str(),&fileInfo)) != -1) 
    { 
        do
        {  
            if((fileInfo.attrib & _A_SUBDIR)) 
            { 
                if(strcmp(fileInfo.name,".") != 0 && strcmp(fileInfo.name,"..") != 0) 
                {
                    dirPathTemp = pathName.assign(pszBackupFiles).append("/").append(fileInfo.name);
                    BkGetFilesTotalSize(dirPathTemp, nTotalSize, nFileNum);
                }
            }
            else
            { 
                fileNameTemp = pathName.assign(pszBackupFiles).append("/").append(fileInfo.name);
                if(-1 != ( _stat64(fileNameTemp.c_str(), &info) ) ){
                    nTotalSize +=  info.st_size;
                    nFileNum++;
                }
            }
        }while(_findnext(hFile, &fileInfo) == 0); 
        _findclose(hFile);
    } 
    if((nFileNum%1000) == 0)
        cout<<"文件数量"<<nFileNum<<endl;
    return nTotalSize;
}

void Test_Class::BkGetFilesTotalSizeTest()
{
    string pszDirPath = "H:\\SVN_Code";
    LONGLONG nSize = 0;
    LONGLONG nFileNum = 0;
    LONGLONG pretime = time(NULL);
    nSize = BkGetFilesTotalSize(pszDirPath, nSize, nFileNum);

    cout<<"文件夹大小："<<nSize<<endl;
    cout<<"文件夹数目："<<nFileNum<<endl;
    cout<<"耗时: "<<time(NULL) - pretime<<"秒"<<endl;
    system("pause");
}


void Test_Class::PrintfSysName()
{
    cout<<__FUNCTION__<<endl;
    cout<<__LINE__<<endl;
    cout<<__FILE__<<endl<<__TIME__<<endl<<__DATE__<<endl<<endl;
}

#pragma comment(lib,"ws2_32.lib")
int Test_Class::GetHostNameTest()
{
    char hostname[256];
    int iRet = 0;
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,1),&wsaData)) //调用Windows Sockets DLL
    {
        printf("Winsock无法初始化!\n");
        WSACleanup();
        return 0;
    }
    //memset(hostname, 0, 256);
    iRet = gethostname(hostname, sizeof(hostname));
    if(iRet != 0 )
    {
        printf( "get hostname error:%d\n", iRet);
    }
    printf("%s\n", hostname);
}

