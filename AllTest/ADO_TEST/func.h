


#include <Windows.h>
#include<iostream>
#include <iostream>
#include <fstream>
using namespace std;

//#include<iomanip>
#include <string>
//#include <icrsint.h>
#include "nspr.h"


using namespace std;

#import "c:\Program Files\Common Files\System\ADO\msado15.dll" \
    no_namespace rename("EOF", "EndOfFile")

#define WAVETOP_BACKUP_OK 0
#define WAVETOP_BACKUP_INTERNAL_ERROR 3
#define WAVETOP_BACKUP_FILE_TOO_LARGE 5

#define MSSQL_LOG_INFO(format, ...)\
    printf(format,##__VA_ARGS__)
#define MSSQL_LOG_ERR(format, ...)\
    printf(format,##__VA_ARGS__)




/* the Structure of exported file */
typedef struct _MiMSSqlstDbString {
    char szDbString[1024];
    char szPhysicalStr[1024];
    struct _MiMSSqlstDbString *pNext;
}MiMSSqlstDbString;





int MiGetCurResHisId();
char* ConvertToUTF(char *pszFile);
char *unicodeToUtf8(const WCHAR *zWideFilename,size_t m_encode);
char *unicodeTogbk(const WCHAR *zWideFilename);
int MiMSSqlFreeStDbString(MiMSSqlstDbString *pstString);

int CreateSql(MiMSSqlstDbString *pstDbFileName, MiMSSqlstDbString *pstDblogName,
    char *pszDbName, char *pszDeviceName, char *pszPath, char **ppstrSql);

int MiMSSqlExecSqlADO(char *pszIP, int nPort, char *pszUser, 
    char *pszPwd, char *pszDbName, char *pszDeviceName, char *pszPath);

int GetDBState(char *pszIP, int nPort, char *pszUser, 
    char *pszPwd,char *pszDBName);

//ADO 获取bak信息
int MemoryConnect(char *pszDeviceName, MiMSSqlstDbString **pstDbFileName, 
    MiMSSqlstDbString **pstDblogName);