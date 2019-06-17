#ifndef _VDI_FUNCTION_H_ 
#define _VDI_FUNCTION_H_ 1

#include <iostream>
#include <Windows.h>

#include "nspr.h"
#include "vdi.h"
#include "vdierror.h"
#include "vdiguid.h"

using namespace std;

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
    struct _MiMSSqlstDbString *pNext;
}MiMSSqlstDbString;

#import "c:\Program Files\Common Files\System\ADO\msado15.dll" \
    no_namespace rename("EOF", "EndOfFile")

#define  WAVETOP_BACKUP_OK 0















#endif /* !define(_VDI_FUNCTION_H_) */