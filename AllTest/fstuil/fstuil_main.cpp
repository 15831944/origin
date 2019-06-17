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
    si.dwFlags = STARTF_USESHOWWINDOW;  // 指定wShowWindow成员有效
    si.wShowWindow = FALSE;      // 此成员设为TRUE的话则显示新建进程的主窗口，为FALSE的话则不显示
    BOOL bRet = ::CreateProcess (
        NULL,               // 不在此指定可执行文件的文件名
        pszstr,                // 命令行参数
        NULL,               // 默认进程安全性
        NULL,               // 默认线程安全性
        FALSE,              // 指定当前进程内的句柄不可以被子进程继承
        CREATE_NEW_CONSOLE, // 为新进程创建一个新的控制台窗口
        NULL,               // 使用本进程的环境变量
        NULL,               // 使用本进程的驱动器和目录
        &si,                // 指定新进程中主窗口的位置、大小和标准句柄等
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
