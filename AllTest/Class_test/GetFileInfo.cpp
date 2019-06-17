#include "GetFileInfo.h"


CGetFileInfo::CGetFileInfo(void)
{
}


CGetFileInfo::~CGetFileInfo(void)
{
}


int CGetFileInfo::GetFileSize()
{
    HANDLE hfile = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAA stdata = {0};
    bool bStatus = false;
    LONGLONG nFileSize = 0;
    
    hfile = FindFirstFile("H:\\DBdata\\test1000.mdf", &stdata);
    if(hfile == INVALID_HANDLE_VALUE){
        cout<<"FindFirstFile failed\n";
        return -1;
    }

    bStatus = FindClose(hfile);
    if(!bStatus){
        cout<<"FindClose failed"<<endl;
        return -2;
    }
    hfile = INVALID_HANDLE_VALUE;
    nFileSize = ((LONGLONG)stdata.nFileSizeHigh << 32) + stdata.nFileSizeLow;
    cout<<"FileSize: "<<nFileSize<<endl;

    return 2;


}

int CGetFileInfo::GetDirSize()
{
    HANDLE hfile = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAA stdata = {0};
    bool bStatus = false;
    LONGLONG nFileSize = 0;

    hfile = FindFirstFile("H:\\DBdata\\test1000.mdf", &stdata);
    if(hfile == INVALID_HANDLE_VALUE){
        cout<<"FindFirstFile failed\n";
        return -1;
    }

    bStatus = FindClose(hfile);
    if(!bStatus){
        cout<<"FindClose failed"<<endl;
        return -2;
    }
    hfile = INVALID_HANDLE_VALUE;
    nFileSize = ((LONGLONG)stdata.nFileSizeHigh << 32) + stdata.nFileSizeLow;
    cout<<"FileSize: "<<nFileSize<<endl;

    return 2;

   // BkGetFilesTotalSize


}