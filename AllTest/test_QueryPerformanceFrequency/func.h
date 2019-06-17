#include "iostream"
#include <io.h>
#include "Windows.h"
#include "time.h"
#include "backup_proto.h"
#include "wmain.h"

using namespace std;



void time_test();


void time_test2();

void ThreeEleCalcu();

void Printflld();

void PR_MKDIR_Test();

void SQLASSEMBLE();

void CGI_Analysis();

int BackupCreateDir(const char *pszDir);
int BackupCreateParentDir(const char *pszFile);
int BkMssqlLogDataIsExist(char *pszFileName);

void BkFindMaxFileNum(char * pszDir, char *pszFileName, int *nFileNum);