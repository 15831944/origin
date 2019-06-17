#pragma once
#include "FailedFileLog.h"
#include "CharAssign.h"
#include "CreateProccess.h"
#include "Ado.h"
//#include "VdiDemo.h"
#include "CreateThread.h"
#include "Readfile.h"
#include "GetFileInfo.h"

#define MAX_SEM_COUNT 10
#define THREADCOUNT 50

class Test_Class
{
public:
    Test_Class(void);
    ~Test_Class(void);

    void test_FailedFileLog();
    void Test_CharAssign();
    int Test_Class:: GetBinaryKey_test(int nCurBSId, int nResHisId, int nResBsId, 
        unsigned char *pszBinary, int nSize);
    int Time_Test();
    int ConstCharMalloc();
    void Test_atoll();
    void Stdlib_atoll();

    void Test_Sprintf();

    void Test_Class::Ado_ShrinkDBTest();
    void Ado_LogBackupTest();
    void Ado_MemoryTest();

    void Test_Class::Ado_GetDBVersionTest();

    bool GetFlag() const;
    void unsingedCharTest();

    void Test_createProcess();
    int Test_Class::memTest( void );
    int Test_Class::MymemTest( void );
    void Test_Class::Ado_ScriptTest();

    void Test_Class::ReadfileTest();
    void Test_Class::test_and_or();
    void TestParam(int a,int b,int c,int d,int e,int f, int g,int a2,int b2,int c2,int d2,int e2,int f2, int g2,int g3);

    int Test_Class::ghSemaphoreTest( void );
    void Test_Class::GetFullNamePathTest();
    int Test_Class::Ado_GetStandByPath();
    void Test_Class::GetOsVersion();

    LONGLONG Test_Class::BkGetFilesTotalSize(string pszBackupFiles, LONGLONG &nTotalSize, LONGLONG &nFileNum);
    void Test_Class::BkGetFilesTotalSizeTest();

    DWORD WINAPI ThreadProc( LPVOID lpParam );

    void PrintfSysName();
    int Test_Class::GetHostNameTest();

private:
    mutable int m_AccessCount;
    bool m_flag;
    LONGLONG currenttime;
    HANDLE ghSemaphore;

};

void BkGetFilesTotalSizeTest();

