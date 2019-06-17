#include <Windows.h>
#include <string>
#include <map>
#include <iostream>
#include <io.h>
#include <sys\stat.h>
#include <time.h>
//#include "nspr.h"

using namespace std;
LONGLONG BkGetFilesTotalSize(string pszBackupFiles, LONGLONG &nTotalSize, LONGLONG &nFileNum);

void main()
{
    string pszDirPath = "H:\\SVN_Code";
    LONGLONG nSize = 0;
    LONGLONG nFileNum = 0;
    LONGLONG pretime = time(NULL);
    nSize = BkGetFilesTotalSize(pszDirPath, nSize, nFileNum);

    cout<<"�ļ��д�С��"<<nSize<<endl;
    cout<<"�ļ�����Ŀ��"<<nFileNum<<endl;
    cout<<"��ʱ: "<<time(NULL) - pretime<<"��"<<endl;
    system("pause");
}

LONGLONG BkGetFilesTotalSize(string pszBackupFiles, LONGLONG &nTotalSize, LONGLONG &nFileNum)
{
    long hFile              = 0;
    string pathName         = "";
    string dirPathTemp      = "";
    string fileNameTemp     = "";
    struct _finddata_t fileInfo;
    struct _stat64 info;
    std::map<int,char*>::iterator itrDir;
    std::map<int,char*>::iterator itrFile;
    
    // �˴�"/*"����������е�����
    if((hFile = _findfirst(pathName.assign(pszBackupFiles).append("/*").c_str(),&fileInfo)) != -1) 
    { 
        do
        {  
            if((fileInfo.attrib & _A_SUBDIR)) 
            { 
                if(strcmp(fileInfo.name,".") != 0 && strcmp(fileInfo.name,"..") != 0) 
                {
                    dirPathTemp = pathName.assign(pszBackupFiles).append("/").append(fileInfo.name);
                    BkGetFilesTotalSize(dirPathTemp, nTotalSize, nFileNum);
                }
            }
            else
            { 
                fileNameTemp = pathName.assign(pszBackupFiles).append("/").append(fileInfo.name);
                if(-1 != ( _stat64(fileNameTemp.c_str(), &info) ) ){
                    nTotalSize +=  info.st_size;
                    nFileNum++;
                }
            }
        }while(_findnext(hFile, &fileInfo) == 0); 
        _findclose(hFile);
    } 
    if((nFileNum%1000) == 0)
        cout<<"�ļ�����"<<nFileNum<<endl;
    return nTotalSize;
}