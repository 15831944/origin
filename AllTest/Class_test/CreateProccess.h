#pragma once
#include "public.h"
class CCreateProccess
{
public:
    CCreateProccess(void);
    ~CCreateProccess(void);
    int SetSecurityAtt();
    int EXECCmd();
public:
    char szCmd[1024];
    HANDLE hOutPut;
    HANDLE hInPut;
    SECURITY_ATTRIBUTES sa;
    STARTUPINFO st ;
    PROCESS_INFORMATION pi;
    BOOL hResult;
};

