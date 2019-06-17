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
		//创建一个新进程  
		if(CreateProcess(  
			sztest2,   //  指向一个NULL结尾的、用来指定可执行模块的宽字节字符串  
			NULL, // 命令行字符串  
			NULL, //    指向一个SECURITY_ATTRIBUTES结构体，这个结构体决定是否返回的句柄可以被子进程继承。  
			NULL, //    如果lpProcessAttributes参数为空（NULL），那么句柄不能被继承。<同上>  
			false,//    指示新进程是否从调用进程处继承了句柄。   
			CREATE_NEW_CONSOLE,  //  指定附加的、用来控制优先类和进程的创建的标  
			//  CREATE_NEW_CONSOLE  新控制台打开子进程  
			//  CREATE_SUSPENDED    子进程创建后挂起，直到调用ResumeThread函数  
			NULL, //    指向一个新进程的环境块。如果此参数为空，新进程使用调用进程的环境  
			NULL, //    指定子进程的工作路径  
			&si, // 决定新进程的主窗体如何显示的STARTUPINFO结构体  
			&pi  // 接收新进程的识别信息的PROCESS_INFORMATION结构体  
			))  
		{  
			CloseHandle(pi.hThread);
			thread[i] = pi.hProcess;
			cout << "create process success" << endl;  

			//下面两行关闭句柄，解除本进程和新进程的关系，不然有可能不小心调用TerminateProcess函数关掉子进程  
			//      CloseHandle(pi.hProcess);  
			//      CloseHandle(pi.hThread);  
		}  
		else{  
			cerr << "failed to create process" << endl;  
		} 
#if 0
		//创建一个新进程  
		if(CreateProcess(  
			szRead,   //  指向一个NULL结尾的、用来指定可执行模块的宽字节字符串  
			NULL, // 命令行字符串  
			NULL, //    指向一个SECURITY_ATTRIBUTES结构体，这个结构体决定是否返回的句柄可以被子进程继承。  
			NULL, //    如果lpProcessAttributes参数为空（NULL），那么句柄不能被继承。<同上>  
			false,//    指示新进程是否从调用进程处继承了句柄。   
			CREATE_NEW_CONSOLE,  //  指定附加的、用来控制优先类和进程的创建的标  
			//  CREATE_NEW_CONSOLE  新控制台打开子进程  
			//  CREATE_SUSPENDED    子进程创建后挂起，直到调用ResumeThread函数  
			NULL, //    指向一个新进程的环境块。如果此参数为空，新进程使用调用进程的环境  
			NULL, //    指定子进程的工作路径  
			&si, // 决定新进程的主窗体如何显示的STARTUPINFO结构体  
			&pi  // 接收新进程的识别信息的PROCESS_INFORMATION结构体  
			))  
		{  
			cout << "create process success" << endl;  

			//下面两行关闭句柄，解除本进程和新进程的关系，不然有可能不小心调用TerminateProcess函数关掉子进程  
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
	cout<<"Create Process执行时间为： "<<double(CounterEnd-CounterStart)/Freq<<endl;
	system("pause");
	//Sleep(1000000);  

	//终止子进程  
	TerminateProcess(pi.hProcess, 300);  

	//终止本进程，状态码  
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
		//创建一个新进程  
		if(CreateProcess(  
			NULL,   //  指向一个NULL结尾的、用来指定可执行模块的宽字节字符串  
			"net stop \"wavetop io daemon\"", // 命令行字符串  
			NULL, //    指向一个SECURITY_ATTRIBUTES结构体，这个结构体决定是否返回的句柄可以被子进程继承。  
			NULL, //    如果lpProcessAttributes参数为空（NULL），那么句柄不能被继承。<同上>  
			false,//    指示新进程是否从调用进程处继承了句柄。   
			CREATE_NEW_CONSOLE,  //  指定附加的、用来控制优先类和进程的创建的标  
			//  CREATE_NEW_CONSOLE  新控制台打开子进程  
			//  CREATE_SUSPENDED    子进程创建后挂起，直到调用ResumeThread函数  
			NULL, //    指向一个新进程的环境块。如果此参数为空，新进程使用调用进程的环境  
			NULL, //    指定子进程的工作路径  
			&si, // 决定新进程的主窗体如何显示的STARTUPINFO结构体  
			&pi  // 接收新进程的识别信息的PROCESS_INFORMATION结构体  
			))  
		{  
			CloseHandle(pi.hThread);
			thread[i] = pi.hProcess;
			cout << "create process success" << endl;  

			//下面两行关闭句柄，解除本进程和新进程的关系，不然有可能不小心调用TerminateProcess函数关掉子进程  
			//      CloseHandle(pi.hProcess);  
			//      CloseHandle(pi.hThread);  
		}  
		else{  
			cerr << "failed to create process" << endl;  
		} 
#if 0
		//创建一个新进程  
		if(CreateProcess(  
			szRead,   //  指向一个NULL结尾的、用来指定可执行模块的宽字节字符串  
			NULL, // 命令行字符串  
			NULL, //    指向一个SECURITY_ATTRIBUTES结构体，这个结构体决定是否返回的句柄可以被子进程继承。  
			NULL, //    如果lpProcessAttributes参数为空（NULL），那么句柄不能被继承。<同上>  
			false,//    指示新进程是否从调用进程处继承了句柄。   
			CREATE_NEW_CONSOLE,  //  指定附加的、用来控制优先类和进程的创建的标  
			//  CREATE_NEW_CONSOLE  新控制台打开子进程  
			//  CREATE_SUSPENDED    子进程创建后挂起，直到调用ResumeThread函数  
			NULL, //    指向一个新进程的环境块。如果此参数为空，新进程使用调用进程的环境  
			NULL, //    指定子进程的工作路径  
			&si, // 决定新进程的主窗体如何显示的STARTUPINFO结构体  
			&pi  // 接收新进程的识别信息的PROCESS_INFORMATION结构体  
			))  
		{  
			cout << "create process success" << endl;  

			//下面两行关闭句柄，解除本进程和新进程的关系，不然有可能不小心调用TerminateProcess函数关掉子进程  
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
	cout<<"Create Process执行时间为： "<<double(CounterEnd-CounterStart)/Freq<<endl;
	system("pause");
	//Sleep(1000000);  

	//终止子进程  
	TerminateProcess(pi.hProcess, 300);  

	//终止本进程，状态码  
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
                cout<<"nstart 更改位100\n";
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
	cout<<"线程安全模式： "<<threadmode<<endl;

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
