#include <windows.h>
#include <iostream>
#include <string>
#include <cstring>
using namespace std;
#include "nspr.h"

int main()
    {
    string strMapName("_wavetop_share_d_memory_");                // �ڴ�ӳ���������
    string strComData("This is common data!");        // �����ڴ��е�����
    char *pBuffer = NULL;                                    // �����ڴ�ָ��
    PRSharedMemory *share = NULL;
    

    share = PR_OpenSharedMemory(strMapName.c_str(),200,PR_SHM_EXCL|PR_SHM_CREATE,0666);
    if(share == NULL){
        cout<<"Sharememory is existed \n";
        share = PR_OpenSharedMemory(strMapName.c_str(),200,PR_SHM_CREATE,0664);
        if(share == NULL){
            cout<<"PR_OpenShareMemory failed"<<endl;
            return 0;
            }
        pBuffer = (char *)PR_AttachSharedMemory(share,0);
        if(pBuffer == NULL){
            cout<<"PR_AttachShareMemory failed"<<endl;
            return -1;
            }
        cout<<"��ȡ�����ڴ�����: "<<pBuffer<<endl;
        }
    else{
        pBuffer =(char *)PR_AttachSharedMemory(share,0);
        if(pBuffer == NULL){
            cout<<"PR_AttachShareMemory failed"<<endl;
            if(!share)
                PR_CloseSharedMemory(share);
            return 0;
            }
        strcpy(pBuffer,strComData.c_str());
        cout<<"д�빲���ڴ����ݣ� "<<pBuffer<<endl;
        }

    system("pause");

    if(!pBuffer)
        PR_DetachSharedMemory(share,(void *)pBuffer);
    if(!share)
        PR_CloseSharedMemory(share);
    return 0;
    }

int main12()
    {
    string strMapName("ShareMemory");                // �ڴ�ӳ���������
    string strComData("This is common data!");        // �����ڴ��е�����
    LPVOID pBuffer;                                    // �����ڴ�ָ��

    // ������ͼ��һ���������ڴ�ӳ���ļ�����  
    HANDLE hMap = ::OpenFileMapping(FILE_MAP_ALL_ACCESS, 0, strMapName.c_str());
    if (NULL == hMap)
        {    // ��ʧ�ܣ�����֮
        hMap = ::CreateFileMapping(INVALID_HANDLE_VALUE,
            NULL,
            PAGE_READWRITE,
            0,
            strComData.length()+1,
            strMapName.c_str());
        // ӳ������һ����ͼ���õ�ָ�����ڴ��ָ�룬�������������
        pBuffer = ::MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
        strcpy((char*)pBuffer, strComData.c_str());
        cout << "д�빲���ڴ����ݣ�" << (char *)pBuffer << endl;
        }
    else
        {    // �򿪳ɹ���ӳ������һ����ͼ���õ�ָ�����ڴ��ָ�룬��ʾ�����������
        pBuffer = ::MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
        cout << "��ȡ�����ڴ����ݣ�" << (char *)pBuffer << endl;
        }

    getchar();            // ע�⣬���̹رպ����о���Զ��رգ�����Ҫ��������ͣ

    // ����ļ�ӳ�䣬�ر��ڴ�ӳ���ļ�������
    ::UnmapViewOfFile(pBuffer);
    ::CloseHandle(hMap);
    system("pause");        
    return 0;
    }