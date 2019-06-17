#include<iostream>  
#include<Windows.h>  
#include "nspr.h"
using namespace std;
#define COUNT 1

#define  SLogErrorWrite(b,c,d,f,a) cout<<f<<a<<endl
#define  SLogErrorWrite(b,c,d,f)  cout<<f<<endl
#define  WAVETOP_BACKUP_OK 0
#define  WAVETOP_BACKUP_INTERNAL_ERROR 2
#define  WT_BK_ORACLE_CLIENT_TIMEOUT  10







int BkSqlStopService(char *pszServerName);
char* GetExeDir();
int BkSqlStartService(char* pszServName);