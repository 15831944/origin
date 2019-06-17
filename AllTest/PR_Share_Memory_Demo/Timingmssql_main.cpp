#include "Timingmssql_func.h"

void Recover()
{   int nRC = 0;

    char *pszModType = "CLIENTMSSQLRECOVER";
    char *pszInstance = "MSSQLSERVER";
    char *pszReIP = "2.2.2.10";
    char *pszDBName = "db1003";
    char *pszModId = "0";
    char szMsg[1024] = {0};
    nRC = UinSendRecoverProtocol(pszModType, pszInstance, pszReIP,
        pszDBName, BACKUP_SERVER_PORT, pszModId, szMsg, sizeof(szMsg));
    cout<<"UinSendRecoverProtocol result : "<<nRC<<endl;

}
void StartDB()
{  
    PR_Init(PR_SYSTEM_THREAD, PR_PRIORITY_NORMAL, 1);
    int Threadcount = 0;
    int nRC = 0;
    char *pszShareName = WAVETOP_SHARE_MEMORY;
    nRC = BkGetThreadNum(&Threadcount,pszShareName);
    cout<<"Treadcount: "<<Threadcount<<endl;
    cout<<"BKGetThreadNum result: "<<nRC<<endl;
}

void test1()
{   
    test("1");


}
void ConfRead()
    {
    int nstart = GetPrivateProfileInt("TT","bstart",0,"D:\\1.conf");
    cout<<"开始 :nstart : "<<nstart<<endl;

    WritePrivateProfileString("TT","bstart","200","D:\\1.conf");
    nstart = GetPrivateProfileInt("TT","bstart",0,"D:\\1.conf");
    cout<<"写入后 :nstart : "<<nstart<<endl;
    }


void Getenv()
{   
    char *pszWorkPath = getenv("BACKUP_DIR");
    if(pszWorkPath != NULL)
    cout<<"BACKUP_DIR 变量是： "<<pszWorkPath<<endl;
}

void int2char()
{
    int count = 100;
    char num[10] = {0};
    char num2[10] = {0};

   
    cout<<"num: "<<num<<endl;
    cout<<"num2: "<<num2<<endl;
}
void main()
{
    //ConfRead();
    //int2char();
    //test1();
    //StartDB();
    //Recover();
    int a = time(NULL);
    cout<<a<<endl;
    system("pause");
    
}

