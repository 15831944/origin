#include "CreateProccess.h"


CCreateProccess::CCreateProccess(void)
{
}


CCreateProccess::~CCreateProccess(void)
{
}

int CCreateProccess::SetSecurityAtt()
{
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle =TRUE;
    sa.lpSecurityDescriptor = NULL;
    if(!CreatePipe(&hOutPut,&hInPut,&sa,0))
    {
        cout<<"create pipe failed erorr="<<GetLastError()<<endl;
        return 1;
    }

    st.cb = sizeof(STARTUPINFO);
    GetStartupInfo(&st);
    st.hStdOutput = hInPut;
    st.hStdError = hInPut;
    st.wShowWindow = SW_HIDE;
    st.dwFlags = STARTF_USESHOWWINDOW |STARTF_USESTDHANDLES;
    return 0;
}

int CCreateProccess::EXECCmd()
{
    if(!CreateProcess(NULL,szCmd,NULL,
        NULL,TRUE,NULL,NULL,NULL,&st,&pi))
    {
        cout<<"failed create proccess,error="<<GetLastError()<<endl;
        return 1;
    }
    return 0;
}