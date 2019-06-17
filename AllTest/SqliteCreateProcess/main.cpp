#include<iostream>  
#include<Windows.h>  
#include "alarm.h"


using namespace std;  
#define COUNT 10
#ifdef SQLITE_ENABLE_IOTRACE
#define SQLITE_ENABLE_IOTRACE
#endif


int main14()  
{  
	STARTUPINFO si;  
	PROCESS_INFORMATION pi;  

	ZeroMemory(&si, sizeof(si));  
	ZeroMemory(&pi, sizeof(pi));  

	HANDLE thread[COUNT];

	char *pPath = GetExeDir();
	char szWrite[256] = {0};
	char szRead[256]  = {0};
	char szCreateDir[256] = {0};
	char sztest1[256] = {0};
	char sztest2[256] = {0};
	PR_snprintf(szWrite,256,"%s\\SqliteWrite.exe",pPath);
	PR_snprintf(szRead,256,"%s\\SqliteRead.exe",pPath);
	PR_snprintf(szCreateDir,256,"%s\\test_QueryPerformanceFrequency.exe",pPath);
	PR_snprintf(sztest1,256,"%s\\1.exe",pPath);
	PR_snprintf(sztest2,256,"%s\\CreateProccess_ONLY.exe",pPath);
	double Freq = 0.0;
	long long CounterStart = 0;
	long long  CounterEnd = 0;
	LARGE_INTEGER li ;
	if(!QueryPerformanceFrequency(&li))
		cout << "QueryPerformanceFrequency failed!\n";
	Freq = double(li.QuadPart)/1000;
	QueryPerformanceCounter(&li);
	CounterStart = li.QuadPart;

	for(int i= 0;i<COUNT;i++)
	{
		//����һ���½���  
		if(CreateProcess(  
			sztest2,   //  ָ��һ��NULL��β�ġ�����ָ����ִ��ģ��Ŀ��ֽ��ַ���  
			NULL, // �������ַ���  
			NULL, //    ָ��һ��SECURITY_ATTRIBUTES�ṹ�壬����ṹ������Ƿ񷵻صľ�����Ա��ӽ��̼̳С�  
			NULL, //    ���lpProcessAttributes����Ϊ�գ�NULL������ô������ܱ��̳С�<ͬ��>  
			false,//    ָʾ�½����Ƿ�ӵ��ý��̴��̳��˾����   
			CREATE_NEW_CONSOLE,  //  ָ�����ӵġ���������������ͽ��̵Ĵ����ı�  
			//  CREATE_NEW_CONSOLE  �¿���̨���ӽ���  
			//  CREATE_SUSPENDED    �ӽ��̴��������ֱ������ResumeThread����  
			NULL, //    ָ��һ���½��̵Ļ����顣����˲���Ϊ�գ��½���ʹ�õ��ý��̵Ļ���  
			NULL, //    ָ���ӽ��̵Ĺ���·��  
			&si, // �����½��̵������������ʾ��STARTUPINFO�ṹ��  
			&pi  // �����½��̵�ʶ����Ϣ��PROCESS_INFORMATION�ṹ��  
			))  
		{  
			CloseHandle(pi.hThread);
			thread[i] = pi.hProcess;
			cout << "create process success" << endl;  

			//�������йرվ������������̺��½��̵Ĺ�ϵ����Ȼ�п��ܲ�С�ĵ���TerminateProcess�����ص��ӽ���  
			//      CloseHandle(pi.hProcess);  
			//      CloseHandle(pi.hThread);  
		}  
		else{  
			cerr << "failed to create process" << endl;  
		} 
#if 0
		//����һ���½���  
		if(CreateProcess(  
			szRead,   //  ָ��һ��NULL��β�ġ�����ָ����ִ��ģ��Ŀ��ֽ��ַ���  
			NULL, // �������ַ���  
			NULL, //    ָ��һ��SECURITY_ATTRIBUTES�ṹ�壬����ṹ������Ƿ񷵻صľ�����Ա��ӽ��̼̳С�  
			NULL, //    ���lpProcessAttributes����Ϊ�գ�NULL������ô������ܱ��̳С�<ͬ��>  
			false,//    ָʾ�½����Ƿ�ӵ��ý��̴��̳��˾����   
			CREATE_NEW_CONSOLE,  //  ָ�����ӵġ���������������ͽ��̵Ĵ����ı�  
			//  CREATE_NEW_CONSOLE  �¿���̨���ӽ���  
			//  CREATE_SUSPENDED    �ӽ��̴��������ֱ������ResumeThread����  
			NULL, //    ָ��һ���½��̵Ļ����顣����˲���Ϊ�գ��½���ʹ�õ��ý��̵Ļ���  
			NULL, //    ָ���ӽ��̵Ĺ���·��  
			&si, // �����½��̵������������ʾ��STARTUPINFO�ṹ��  
			&pi  // �����½��̵�ʶ����Ϣ��PROCESS_INFORMATION�ṹ��  
			))  
		{  
			cout << "create process success" << endl;  

			//�������йرվ������������̺��½��̵Ĺ�ϵ����Ȼ�п��ܲ�С�ĵ���TerminateProcess�����ص��ӽ���  
			//      CloseHandle(pi.hProcess);  
			//      CloseHandle(pi.hThread);  
		}  
		else{  
			cerr << "failed to create process" << endl;  
		} 
#endif
	}
	WaitForMultipleObjects(COUNT,thread,TRUE,INFINITE);

	QueryPerformanceCounter(&li);
	CounterEnd = li.QuadPart;
	cout<<"Create Processִ��ʱ��Ϊ�� "<<double(CounterEnd-CounterStart)/Freq<<endl;
	system("pause");
	//Sleep(1000000);  

	//��ֹ�ӽ���  
	TerminateProcess(pi.hProcess, 300);  

	//��ֹ�����̣�״̬��  
	ExitProcess(1001);  
	return 0;  
}

int main12()  
{  
	STARTUPINFO si;  
	PROCESS_INFORMATION pi;  

	ZeroMemory(&si, sizeof(si));  
	ZeroMemory(&pi, sizeof(pi));  

	HANDLE thread[COUNT];

	char *pPath = GetExeDir();
	char szWrite[256] = {0};
	char szRead[256]  = {0};
	char szCreateDir[256] = {0};
	char sztest1[256] = {0};
	PR_snprintf(szWrite,256,"%s\\SqliteWrite.exe",pPath);
	PR_snprintf(szRead,256,"%s\\SqliteRead.exe",pPath);
	PR_snprintf(szCreateDir,256,"%s\\test_QueryPerformanceFrequency.exe",pPath);
	PR_snprintf(sztest1,256,"%s\\1.exe",pPath);
	double Freq = 0.0;
	long long CounterStart = 0;
	long long  CounterEnd = 0;
	LARGE_INTEGER li ;
	if(!QueryPerformanceFrequency(&li))
		cout << "QueryPerformanceFrequency failed!\n";
	Freq = double(li.QuadPart)/1000;
	QueryPerformanceCounter(&li);
	CounterStart = li.QuadPart;

	for(int i= 0;i<COUNT;i++)
	{
		//����һ���½���  
		if(CreateProcess(  
			NULL,   //  ָ��һ��NULL��β�ġ�����ָ����ִ��ģ��Ŀ��ֽ��ַ���  
			"net stop \"wavetop io daemon\"", // �������ַ���  
			NULL, //    ָ��һ��SECURITY_ATTRIBUTES�ṹ�壬����ṹ������Ƿ񷵻صľ�����Ա��ӽ��̼̳С�  
			NULL, //    ���lpProcessAttributes����Ϊ�գ�NULL������ô������ܱ��̳С�<ͬ��>  
			false,//    ָʾ�½����Ƿ�ӵ��ý��̴��̳��˾����   
			CREATE_NEW_CONSOLE,  //  ָ�����ӵġ���������������ͽ��̵Ĵ����ı�  
			//  CREATE_NEW_CONSOLE  �¿���̨���ӽ���  
			//  CREATE_SUSPENDED    �ӽ��̴��������ֱ������ResumeThread����  
			NULL, //    ָ��һ���½��̵Ļ����顣����˲���Ϊ�գ��½���ʹ�õ��ý��̵Ļ���  
			NULL, //    ָ���ӽ��̵Ĺ���·��  
			&si, // �����½��̵������������ʾ��STARTUPINFO�ṹ��  
			&pi  // �����½��̵�ʶ����Ϣ��PROCESS_INFORMATION�ṹ��  
			))  
		{  
			CloseHandle(pi.hThread);
			thread[i] = pi.hProcess;
			cout << "create process success" << endl;  

			//�������йرվ������������̺��½��̵Ĺ�ϵ����Ȼ�п��ܲ�С�ĵ���TerminateProcess�����ص��ӽ���  
			//      CloseHandle(pi.hProcess);  
			//      CloseHandle(pi.hThread);  
		}  
		else{  
			cerr << "failed to create process" << endl;  
		} 
#if 0
		//����һ���½���  
		if(CreateProcess(  
			szRead,   //  ָ��һ��NULL��β�ġ�����ָ����ִ��ģ��Ŀ��ֽ��ַ���  
			NULL, // �������ַ���  
			NULL, //    ָ��һ��SECURITY_ATTRIBUTES�ṹ�壬����ṹ������Ƿ񷵻صľ�����Ա��ӽ��̼̳С�  
			NULL, //    ���lpProcessAttributes����Ϊ�գ�NULL������ô������ܱ��̳С�<ͬ��>  
			false,//    ָʾ�½����Ƿ�ӵ��ý��̴��̳��˾����   
			CREATE_NEW_CONSOLE,  //  ָ�����ӵġ���������������ͽ��̵Ĵ����ı�  
			//  CREATE_NEW_CONSOLE  �¿���̨���ӽ���  
			//  CREATE_SUSPENDED    �ӽ��̴��������ֱ������ResumeThread����  
			NULL, //    ָ��һ���½��̵Ļ����顣����˲���Ϊ�գ��½���ʹ�õ��ý��̵Ļ���  
			NULL, //    ָ���ӽ��̵Ĺ���·��  
			&si, // �����½��̵������������ʾ��STARTUPINFO�ṹ��  
			&pi  // �����½��̵�ʶ����Ϣ��PROCESS_INFORMATION�ṹ��  
			))  
		{  
			cout << "create process success" << endl;  

			//�������йرվ������������̺��½��̵Ĺ�ϵ����Ȼ�п��ܲ�С�ĵ���TerminateProcess�����ص��ӽ���  
			//      CloseHandle(pi.hProcess);  
			//      CloseHandle(pi.hThread);  
		}  
		else{  
			cerr << "failed to create process" << endl;  
		} 
#endif
	}
	WaitForMultipleObjects(COUNT,thread,TRUE,INFINITE);

	QueryPerformanceCounter(&li);
	CounterEnd = li.QuadPart;
	cout<<"Create Processִ��ʱ��Ϊ�� "<<double(CounterEnd-CounterStart)/Freq<<endl;
	system("pause");
	//Sleep(1000000);  

	//��ֹ�ӽ���  
	TerminateProcess(pi.hProcess, 300);  

	//��ֹ�����̣�״̬��  
	ExitProcess(1001);  
	return 0;  
}

//char *pSuf = (char *)malloc(1000*1000*2000);
int Config_DB()
{
	sqlite3 *pDB = NULL;
	int nRC = 0;
	char *pPath = "D:\\sqlite_data\\1.db";
	char *errormsg = NULL;

	nRC = sqlite3_open(pPath,&pDB);
	if(nRC != SQLITE_OK){
		cout<<"sqlite3_open Config_DB error error: "<<sqlite3_errmsg(pDB)<<endl;
		sqlite3_close(pDB);
		cout<<"After sqlite3_open Config_DB failed !!sqlite3_close success"<<endl;
		return -1;
	}
	//sqlite3_db_config(pDB,SQLITE_DBCONFIG_LOOKASIDE,pSuf,65536,900);
	

	sqlite3_close(pDB);
	return 0;
}
int Create_DB()
{
	sqlite3 *pDB = NULL;
	int nRC = 0;
	char *pPath = "D:\\sqlite_data\\1.db";
	char *errormsg = NULL;

	nRC = sqlite3_open(pPath,&pDB);
	if(nRC != SQLITE_OK){
		cout<<"sqlite3_open create tablex error error: "<<sqlite3_errmsg(pDB)<<endl;
		sqlite3_close(pDB);
		cout<<"After sqlite3_open create tablex failed !!sqlite3_close success"<<endl;
		return -1;
	}

	nRC = sqlite3_exec(pDB,"create table tablex(a int);",NULL,NULL,&errormsg);
	if(nRC != SQLITE_OK){
		cout<<"sqlite3_exec create tablex failed!! error: "<<errormsg<<endl;
		sqlite3_free(errormsg);
		sqlite3_close(pDB);
		return -1;
	}

	sqlite3_close(pDB);

	return 0;
}

void MY_Thread(void *param)
{
	sqlite3 *pDB = NULL;
	int nRC = 0;
	char *pPath = "D:\\sqlite_data\\1.db";
	char *errormsg = NULL;


	nRC = sqlite3_open(pPath,&pDB);
	if(nRC != SQLITE_OK){
		cout<<"sqlite3_open error error: "<<sqlite3_errmsg(pDB)<<endl;
		sqlite3_close(pDB);
		return;
	}

	sqlite3_busy_timeout(pDB, 60*1000);

	nRC = sqlite3_exec(pDB,"insert into tablex values(1111);",NULL,NULL,&errormsg);
	if(nRC != SQLITE_OK){
		//if(nRC != 5)
			cout<<"sqlite3_exec failed!! error: "<<errormsg<<endl;
		
		sqlite3_free(errormsg);
		sqlite3_close(pDB);
		return;
	}
	cout<<"insert success"<<endl;
	sqlite3_close(pDB);
	return;
}
void MY_Thread2(void *param)
{
	cout<<"create a thread\n";
	return;
}
void MY_Thread_test(void *param)
    {
    WritePrivateProfileString("TT", "bstart","0","C:\\1.conf");
    while(1)
        {
            int nstart = GetPrivateProfileInt("TT", "bstart",0,"C:\\1.conf");
            if(nstart == 100){
                cout<<"nstart ����λ100\n";
                WritePrivateProfileString("TT", "bstart","0","C:\\1.conf");
                }

            
            Sleep(500);
        }
    cout<<"create a thread\n";
    return;
    }
#define MEMSIZE 1000*1000*200
int main13()
{
	int nRC = 0;
	char *pPath = "D:\\sqlite_data\\1.db";

	int threadmode = sqlite3_threadsafe();
	cout<<"�̰߳�ȫģʽ�� "<<threadmode<<endl;

	//sqlite3_config(SQLITE_CONFIG_MULTITHREAD);
	char *pBuf =(char*)malloc(MEMSIZE);
	memset(pBuf,'\0',sizeof(pBuf));
	sqlite3_config(SQLITE_CONFIG_HEAP, pBuf, MEMSIZE, 512);
	sqlite3_config(SQLITE_CONFIG_LOOKASIDE, 64, 64);

	PR_Delete(pPath);
	if(PR_Access(pPath,PR_ACCESS_EXISTS) != PR_SUCCESS){
		nRC = Create_DB();
		if(nRC != 0){
			cout<<"Create DB failed"<<endl;
			system("pause");
			return 0;
		}
		cout<<"create DB success\n";
	}
	//nRC = Config_DB();
	if(nRC != SQLITE_OK){
		cout<<"Config_DB failed \n";
		return 0;
	}
	cout<<"DB config success\n";
	
	for(int i = 0;i<800;i++){
		PR_CreateThread(PR_USER_THREAD,MY_Thread,NULL,PR_PRIORITY_NORMAL,PR_GLOBAL_THREAD,PR_JOINABLE_THREAD,1 << 21);
	}
	//PR_CreateThread(PR_USER_THREAD,MY_Thread,NULL,PR_PRIORITY_NORMAL,PR_GLOBAL_THREAD,PR_JOINABLE_THREAD,1 << 21);
	system("pause");
	return 0;
}

void test_3_12()
    {
        PR_CreateThread(PR_USER_THREAD,MY_Thread_test,NULL,PR_PRIORITY_NORMAL,PR_GLOBAL_THREAD,PR_JOINABLE_THREAD,0);
    }
int main()
    {
    int nRC = 0;

    test_3_12();
    system("pause");
    return 0;
    }
