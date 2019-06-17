#include <iostream>
#include "nspr.h"
#include <Windows.h>
#include <WinSock2.h>

using namespace std;
#include <string.h>
#include <string>

#define LOG(format, ...)\
    printf("[%s, %d]"format, __FUNCTION__, __LINE__, ##__VA_ARGS__)
int value = 0;

void String_test()
{
    string strtest;
    strtest = string("ABC") + string("  hello");
    cout<<strtest.length()<<endl;
    cout<<sizeof(strtest.c_str())<<endl;
    char sztemp[256] = {0};
    _snprintf(sztemp,sizeof(sztemp),strtest.c_str());
    cout<<"sztemp 长度"<<strlen(sztemp)<<"; sztemp: "<<sztemp<<endl;
    cout<<strtest<<endl;
}

void equ_test()
{
    int a = 10;
    if(a == (0 - 258) || a == 0)
        cout<<"a: "<<a<<endl;

    cout<<( 3 & 7)<<endl;
    cout<<( 3 | 7)<<endl;
    cout<<( 3 & 4)<<endl;
    cout<<( 3 | 4)<<endl;
}

void if_test()
{
    int a = (16 == 16);
    cout<<"a: "<<a<<endl;
    char szHMACServer[128];
    char szHMACClient[128];
    memcpy(szHMACServer, "\0", sizeof(szHMACServer));
    memcpy(szHMACClient, "\0", sizeof(szHMACClient));
    int b = (memcmp(szHMACServer,szHMACServer,16) != 0);
    cout<<"b: "<<b<<endl;

}

void short_test()
{
    short a = 549629;
    cout<<a<<endl;

    short b = 332652;
    cout<<b<<endl;

    int c = strcmp("111","222");
    cout<<"c: "<<c<<endl;
    
    int d = strcmp("222","111");
    cout<<"d: "<<d<<endl;

}

int Convert(char *ppsztemp ,int  nsize)
{

    PR_snprintf(ppsztemp,nsize,"hello");
    return 0;
}
void malloc_test(){
    char sz[120] = {0};
    Convert(sz , sizeof(sz));
    if(sz)
        cout<<sz<<endl;
}

typedef enum PRFileType1
    {
    PR_FILE_FILE1,
    PR_FILE_DIRECTORY1,
    PR_FILE_OTHER1
    } PRFileType1;

struct test{
    PRFileType1 a;
    int b;
    int c;
    char *sz;
    char sz111[2048];
    };

void InitStruct(){
    test st_test;
    PRFileType1 bbb = PR_FILE_DIRECTORY1;
    }
void atio_test()
{
    char *pszstr1 = NULL;
    int a = 2;
    if((a = 1) == 1){
        cout<<"a=1;a: "<<a<<endl;
    }else
    {
        cout<<"a 不等于 1\n";
    }
    int nNum = atoi(pszstr1);
    cout<<nNum<<endl;
}
void test1()
{
    char szBuff[512] = {0};
    WritePrivateProfileString("server", "Spantime", "15432", 
        "D:\\test.conf");
    WritePrivateProfileString("server", "spantime", "15433", 
        "D:\\test.conf");

    GetPrivateProfileString("server","Spantime","",szBuff,sizeof(szBuff),"D:\\test.conf");
    cout<<"Get Spantime: "<<szBuff<<endl;
    GetPrivateProfileString("server","spantime","",szBuff,sizeof(szBuff),"D:\\test.conf");
    cout<<"Get spantime: "<<szBuff<<endl;



}

void test2()
{
    int nLogFlag = 0;
    int a = nLogFlag%30;
    for(int i = 0;i < 500;i++){
        if(i % 30 == 0)
            cout<<"nLogFlag"<< i<<" 是30 的倍数\n";

    }

}

void test3()
{
    if(PR_SUCCESS == PR_Delete("F:\\test\\data_MSSQLSERVER_db1019.000000")){
        cout<<"删除成功\n";
    }
};

void main()
{
    //test3();
    char hostName[260] ;
    int nRC = gethostname(hostName, sizeof(hostName));

    //atio_test();
    //short_test();
    //InitStruct();
    //malloc_test();
   // String_test();
    system("pause");
}

void RP_LOCK_TEST()
{
    //LOG("%s,%s","hello","peter\n");
    PRLock *lock = PR_NewLock();
    PR_Lock(lock);
    int a = 0;
    PR_Unlock(lock);
    system("pause");
}
void test()
{
	int total;
	int index;
	total = 0;
	for(index = 0; index < 100; index ++)
		total += index * index;
	value = total;
	return ;
}
int main12()
{
	//test();
	char sz[50] = "12345";
	char sz2[50] = {0};
	PR_snprintf(sz,sizeof(sz),"%s.bak",sz);
	cout<<sz<<endl;
	system("pause");
	return 1;
}

void main13()
{
	char *szPath = "D:\\Program Files\\Microsoft SQL Server\\MSSQL10.MSSQLSERVER\MSSQL\\DATA\\kingmeddb_log.ldf";
	char *pszTempPath = NULL;
	pszTempPath = strrchr(szPath, '\\');
	if(pszTempPath == NULL){
		cout<<"szTemp is NULL"<<endl;
		pszTempPath = strrchr(szPath, '/');
		if (pszTempPath == NULL) {
			cout<<"szTemp strrchr '/' is NULL"<<endl;
		}
	}
	system("pause");
}


struct test_ab2{
	int a;
	int b;

};
typedef struct test_ab2 test_ab2;


struct test_ab{
	int a;
	int b;
	test_ab2 *c;

};
typedef struct test_ab test_ab;

#include<stdio.h>

int square(int a,int b){

	int ret;
	int *p = NULL;
	*p = 666;
	return ret;
}

int doCalc(int num1, int num2){
	int ret = square(num1,num2);
	return ret;
}

int main14(){
	int param1 = 1;
	int param2 = 2;
	//int result = doCalc(param1,param2);
	test_ab  pTest = {0};
	/*if(pTest.c->a == 3)
	{
		cout<<"pTest"<<endl;
	}*/
	printf("result is %d\n");
	return 0;

}

char *unicodeToUtf8(const WCHAR *zWideFilename)   
{   
	int nByte;   
	char *zFilename;   

	nByte = WideCharToMultiByte(CP_UTF8, 0, zWideFilename, -1, 0, 0, 0, 0);   
	zFilename = (char *)malloc(nByte);   
	if(zFilename == 0) {   
		return 0;   
	}  

	nByte = WideCharToMultiByte(CP_UTF8, 0, zWideFilename, -1, zFilename, nByte, 0, 0);   
	if( nByte == 0 ) {   
		free(zFilename);   
		zFilename = 0;   
	}   
	return zFilename;   
}   

WCHAR *mbcsToUnicode(const char *zFilename)   
{   
	int nByte;   
	WCHAR *zMbcsFilename;   
	int codepage = AreFileApisANSI() ? CP_ACP : CP_OEMCP;   

	nByte = MultiByteToWideChar(codepage, 0, zFilename, -1, NULL,0)*sizeof(WCHAR);   
	zMbcsFilename = (WCHAR *)malloc(nByte*sizeof(zMbcsFilename[0]));   
	if( zMbcsFilename==0 ) {   
		return 0;   
	}  

	nByte = MultiByteToWideChar(codepage, 0, zFilename, -1,   zMbcsFilename, nByte);   
	if(nByte == 0) {   
		free(zMbcsFilename);   
		zMbcsFilename = 0;   
	}   
	return zMbcsFilename;   
}

char* ConvertToUTF(char *pszFile)
{
	unsigned short *wcPath;

	if (pszFile == NULL) {
		return NULL;
	}
	wcPath = (unsigned short *)mbcsToUnicode(pszFile);   
	pszFile = unicodeToUtf8((const WCHAR *)wcPath);
	free(wcPath);
	return pszFile;
}

void FreeChar(char *pszStr)
{
	if(!pszStr){
		free(pszStr);
		pszStr = NULL;
	}
	return;
}

void main45()
{
	/*unsigned long long a = 4936;
	a = a /1024*1024;*/

	/*char sz[] = "123";
	strcat(sz,".");
	cout<<sz<<endl;*/

	//char *pszStr = (char *)malloc(1000);
	
	//FreeChar(pszStr);
	/*for(int i = 0;i<= 2;i++){
		cout<<"I: "<<i<<endl;
	}*/
/*
	char *psz1="新建文件夹33";
	cout<<strlen(psz1)<<endl;
	char *psz = ConvertToUTF(psz1);

	cout<<"  长度："<<strlen(psz)<<endl;
	cout<<"end"<<endl;*/

	/*char *sz = "hello''tony''";
	cout<<sz<<endl;*/

   // cout<<hex<<16<<16<<endl;
    
	system("pause");
}
