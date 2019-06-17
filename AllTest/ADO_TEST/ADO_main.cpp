
#include "func.h"


void RestoreBak()
{
    char*   pszIP           = "2.2.2.41";
    int     nPort           = 1433;
    char*   pszUser         = "sa"; 
    char*   pszPwd          = "Root@123";
    char*   pszDbName       = "db1005";
    char*   pszDeviceName   = "C:\\DBdata\\db1005.bak";
    //char*   pszDeviceName   = "C:\\11\\test1000.bak";
    char*   pszPath         = "C:\\DBdata";

    int nRC = MiMSSqlExecSqlADO(pszIP, nPort, pszUser, 
        pszPwd, pszDbName, pszDeviceName, pszPath);
    if(nRC != 0){
        MSSQL_LOG_ERR("\n还原数据库失败");
    }
}

void GetDBState2()
{
    char*   pszIP           = "2.2.2.10";
    int     nPort           = 1433;
    char*   pszUser         = "sa"; 
    char*   pszPwd          = "Root@123";
    char*   pszDbName       = "db1076";

    int nRC = GetDBState(pszIP, nPort, pszUser, 
        pszPwd, pszDbName);
    if(nRC != 0){
        MSSQL_LOG_ERR("\n还原数据库失败");
    }
}

void aaaa()
{
    char*   pszDbName       = "db1001";
    char*   pszDeviceName   = "C:\\bak\\db1001.bak";
    char*   pszPath         = "C:\\bak";
    MiMSSqlstDbString *pstDbFileName1;
    MiMSSqlstDbString *pstDbFileName2;
    int nRC = MemoryConnect(pszDeviceName, &pstDbFileName1, &pstDbFileName2);
    if(nRC != 0)
        MSSQL_LOG_ERR("\nerror");

    while(pstDbFileName1){
        cout<<pstDbFileName1->szDbString<<endl;
        cout<<pstDbFileName1->szPhysicalStr<<endl;
        pstDbFileName1 = pstDbFileName1->pNext;
    }

    while(pstDbFileName2){
        cout<<pstDbFileName2->szDbString<<endl;
        cout<<pstDbFileName2->szPhysicalStr<<endl;
        pstDbFileName2 = pstDbFileName2->pNext;
    }

    cout<<"end"<<endl;

}


void main()
{
    MSSQL_LOG_INFO("程序开始\n");
    CoInitialize(NULL);

    //GetDBState2();
    //RestoreBak();
    aaaa();

    CoUninitialize();
    system("Pause");
}

