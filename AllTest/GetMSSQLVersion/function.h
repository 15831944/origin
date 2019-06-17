#include <iostream>
#include <Windows.h>
#include "nspr.h"






#import "c:\Program Files\Common Files\System\ADO\msado15.dll" \
    no_namespace rename("EOF", "adoEOF")

#define MSSQL_LOG_ERR(format,...) printf(format,##__VA_ARGS__)
#define MSSQL_LOG_INFO(format,...) printf(format,##__VA_ARGS__)


#define WAVETOP_BACKUP_OK 0
#define WAVETOP_BACKUP_INTERNAL_ERROR 3
#define WAVETOP_BACKUP_FILE_TOO_LARGE 5

//数据库版本 
#define MI_MSSQL2_2K0_VERSION    8   //2000版本
#define MI_MSSQL2_2K5_VERSION    9   //2005版本
#define MI_MSSQL2_2K8_VERSION    10  //2008版本
#define MI_MSSQL2_2K12_VERSION   11  //2012版本及以上


int MiMSSqlGetVersion();

//ADO 独占数据库
int MiMSSqlGetDBModeShareExclusive();