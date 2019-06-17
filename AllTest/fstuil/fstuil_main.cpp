#include <Windows.h>
#include <iostream>
#include <string>
#include <fstream>

using namespace std;

bool IsExist(const char *name)
{
    fstream _file;
    _file.open(name,ios::in);
    if(!_file) return false;
    else       return true;
}
bool ExecCmd(char *pszstr)
{
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    si.dwFlags = STARTF_USESHOWWINDOW;  // ָ��wShowWindow��Ա��Ч
    si.wShowWindow = FALSE;      // �˳�Ա��ΪTRUE�Ļ�����ʾ�½����̵������ڣ�ΪFALSE�Ļ�����ʾ
    BOOL bRet = ::CreateProcess (
        NULL,               // ���ڴ�ָ����ִ���ļ����ļ���
        pszstr,                // �����в���
        NULL,               // Ĭ�Ͻ��̰�ȫ��
        NULL,               // Ĭ���̰߳�ȫ��
        FALSE,              // ָ����ǰ�����ڵľ�������Ա��ӽ��̼̳�
        CREATE_NEW_CONSOLE, // Ϊ�½��̴���һ���µĿ���̨����
        NULL,               // ʹ�ñ����̵Ļ�������
        NULL,               // ʹ�ñ����̵���������Ŀ¼
        &si,                // ָ���½����������ڵ�λ�á���С�ͱ�׼�����
        &pi
        );
    return bRet;
}
bool checkDiskSpace(const char *szDir,DWORD64 &nfreespace)
{
    DWORD64 qwFreeBytesToCaller, qwTotalBytes;
    bool bRet = GetDiskFreeSpaceEx(szDir, 
        (PULARGE_INTEGER)&qwFreeBytesToCaller, 
        (PULARGE_INTEGER)&qwTotalBytes, 
        (PULARGE_INTEGER)&nfreespace);
    return bRet;
}
void assembleCmd(char **pszCmd,char *szfilePtah,DWORD64 len)
{
    string str1;
    char *temp;
    str1 =  string("fsutil file createnew  ");
    str1 += szfilePtah+string(" ")+to_string(len);
    temp = (char*)malloc(str1.length()+1);
    strcpy(temp,str1.c_str());
    *pszCmd = temp;
}
int main()
{
    DWORD64 nFreeBytes              = 0;
    DWORD64	qwCreateBytes           = 0;
    DWORD64 TwoG                    = 2500;//2002MB
    char cmd[2048]                  = {0};
    bool bResult                    = true;
    char szDir[260]                 = "C:";
    char szfilePtah[260]            = "C:\\11\\1";
    char *pszCmd                    = NULL;
    TwoG = TwoG*1024*1024;
    bResult = checkDiskSpace(szDir,nFreeBytes);
    qwCreateBytes = nFreeBytes - TwoG;
    assembleCmd(&pszCmd,szfilePtah,qwCreateBytes);
    if(!IsExist(szfilePtah)&&nFreeBytes>TwoG)
    {
        ExecCmd(pszCmd);
    }else if(IsExist(szfilePtah)&&nFreeBytes>TwoG){
        DeleteFile(szfilePtah);
        ExecCmd(pszCmd);
    }else if(IsExist(szfilePtah)&&nFreeBytes<=TwoG){
        DeleteFile(szfilePtah);
    }
    return 0;
}
