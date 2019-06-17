#include "func.h"



int main11()

{
	//SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR, NULL, "Service not start",1);
	int nRC =0;
	char *pServiceName = "W32Time";

	//nRC = BkSqlStopService(pServiceName);
	if(nRC != 0)
		cout<<"BkSqlStopService failed"<<endl;

	nRC = BkSqlStartService(pServiceName);
	if(nRC != 0)
		 cout<<"BkSqlStartService failed"<<endl;
	else 
		cout<<"BkSqlStartService success"<<endl;

	system("pause");
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
	char sztest2[256] = {0};
	PR_snprintf(szWrite,256,"%s\\SqliteWrite.exe",pPath);
	PR_snprintf(szRead,256,"%s\\SqliteRead.exe",pPath);
	PR_snprintf(szCreateDir,256,"%s\\test_QueryPerformanceFrequency.exe",pPath);
	PR_snprintf(sztest1,256,"%s\\1.exe",pPath);
	PR_snprintf(sztest2,256,"%s\\11.exe",pPath);
	//char szSql[1024] = "\"isql.exe\" -S \"2.2.2.9,1433\" -b -U \"sa\" -P \"Root@123\" -Q \"insert into db1019.dbo.tablex values(getdate(),'aaaaaaa');\"";
    char szSql[1024] = "\"isql.exe\" -S \"2.2.2.40\\MSSQLSERVER,1433\" -U \"sa\" -P \"Root@123\" -Q \"BACKUP  DATABASE  [db1019] TO disk='C:\\bak\\db1019_1556519202#' WITH COMPRESSION,BLOCKSIZE = 65536\"";
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
			szSql, // 命令行字符串  
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

int main()
{
	SECURITY_ATTRIBUTES SecurityAtt;
	STARTUPINFO ISQLStartupInfo;
	PROCESS_INFORMATION stProcInfo;
	HANDLE hStdOutRead = NULL;
	HANDLE hStdOutWrite = NULL;
	HANDLE hStdErrRead = NULL;
	HANDLE hStdErrWrite = NULL; 
	DWORD dwBytes;
	BOOL bResult;
	int nRetVal;
	char szISQLCmdStdOut[2048] = {0};
	char szISQLCmdStdErr[2048] = {0};
    char szSql[2048] = "\"isql.exe\" -S \"2.2.2.40\\MSSQLSERVER,1433\" -U \"sa\" -P \"Root@123\" -Q \"BACKUP  DATABASE  [db1019] TO disk='C:\\bak\\db1019_1556519202#' WITH COMPRESSION,BLOCKSIZE = 65536\"";
	//char szSql[1024] = "\"isql.exe\" -S 2.2.2.9,1433 -b -U \"sa\" -P \"Root@123\" -Q"
	//	"\" insert into db1019.dbo.tablex values(getdate(),'aaaaaaaaa')\"";
	//char szSql[1024] = "\"isql.exe\" -S 127.0.0.1,1433  -b -U \"sa\" -P \"Root@123\" -Q \"USE [master];RESTORE DATABASE [DB1015] FROM  DISK = N'C:/WaveTop/Backup6/server/MssqlExp/db1015_1547824831.bak' WITH  FILE = 1,REPLACE,NOUNLOAD,STATS = 5\"";


	/* stderr pipe */
	memset(&SecurityAtt, 0, sizeof(SecurityAtt));  
	SecurityAtt.nLength = sizeof(SECURITY_ATTRIBUTES); 
	SecurityAtt.bInheritHandle = TRUE; 
	SecurityAtt.lpSecurityDescriptor = NULL;
	bResult = CreatePipe(&hStdErrRead, &hStdErrWrite, &SecurityAtt, 0);
	if (!bResult) {
		hStdErrRead = NULL;
		hStdErrWrite = NULL;
	}

	/* stdout pipe */
	memset(&SecurityAtt, 0, sizeof(SecurityAtt));  
	SecurityAtt.nLength = sizeof(SECURITY_ATTRIBUTES); 
	SecurityAtt.bInheritHandle = TRUE; 
	SecurityAtt.lpSecurityDescriptor = NULL;
	bResult = CreatePipe(&hStdOutRead, &hStdOutWrite, &SecurityAtt, 0);
	if (!bResult) {
		hStdOutRead = NULL;
		hStdOutWrite = NULL;
	}

	/* isql command start info */
	memset(&ISQLStartupInfo, 0, sizeof(ISQLStartupInfo));
	ISQLStartupInfo.cb = sizeof(ISQLStartupInfo);
	ISQLStartupInfo.hStdOutput = hStdOutWrite;
	ISQLStartupInfo.hStdError = hStdErrWrite;
	ISQLStartupInfo.dwFlags = STARTF_USESHOWWINDOW|STARTF_USESTDHANDLES;
	ISQLStartupInfo.wShowWindow = SW_SHOW;


	if(CreateProcess(  
		NULL,   //  指向一个NULL结尾的、用来指定可执行模块的宽字节字符串  
		szSql, // 命令行字符串  
		NULL, //    指向一个SECURITY_ATTRIBUTES结构体，这个结构体决定是否返回的句柄可以被子进程继承。  
		NULL, //    如果lpProcessAttributes参数为空（NULL），那么句柄不能被继承。<同上>  
		TRUE,//    指示新进程是否从调用进程处继承了句柄。   
		0,  //  指定附加的、用来控制优先类和进程的创建的标  
		//  CREATE_NEW_CONSOLE  新控制台打开子进程  
		//  CREATE_SUSPENDED    子进程创建后挂起，直到调用ResumeThread函数  
		NULL, //    指向一个新进程的环境块。如果此参数为空，新进程使用调用进程的环境  
		NULL, //    指定子进程的工作路径  
		&ISQLStartupInfo, // 决定新进程的主窗体如何显示的STARTUPINFO结构体  
		&stProcInfo  // 接收新进程的识别信息的PROCESS_INFORMATION结构体  
		))  
	{  
		CloseHandle(stProcInfo.hThread);
		cout << "create process success" << endl;  

		//下面两行关闭句柄，解除本进程和新进程的关系，不然有可能不小心调用TerminateProcess函数关掉子进程  
		//      CloseHandle(pi.hProcess);  
		//      CloseHandle(pi.hThread);  
	}else
	   cout<<"CreateProcess failed"<<endl;
       for(int i = 0;i<100;i++){
           DWORD nResult = WaitForSingleObject(stProcInfo.hProcess,10*1000);
           if(nResult != WAIT_TIMEOUT)
               break;

           if (NULL != hStdOutRead) {
               bResult = PeekNamedPipe(hStdOutRead, szISQLCmdStdOut, 
                   sizeof(szISQLCmdStdOut), &dwBytes, NULL, NULL);
               if (!bResult) {
                   szISQLCmdStdOut[0] = '\0';
                   cout<<"PeekNamedPipe() StdOutRead failed"<<endl;
               }
               else {
                   szISQLCmdStdOut[dwBytes] = '\0';
                   cout<<szISQLCmdStdOut<<endl;
               }
           }
           cout<<"WaitForSingleObject 10s\n";
       }
	/* read isql stdout and stderr info  */
	if (NULL != hStdOutRead) {
		bResult = PeekNamedPipe(hStdOutRead, szISQLCmdStdOut, 
			sizeof(szISQLCmdStdOut), &dwBytes, NULL, NULL);
		if (!bResult) {
			szISQLCmdStdOut[0] = '\0';
			cout<<"PeekNamedPipe() StdOutRead failed"<<endl;
		}
		else {
			szISQLCmdStdOut[dwBytes] = '\0';
			cout<<szISQLCmdStdOut<<endl;
		}

		CloseHandle(hStdOutRead);
	}
	else {
		szISQLCmdStdOut[0] = '\0';
	}

	if (NULL != hStdErrRead) {
		bResult = PeekNamedPipe(hStdErrRead, szISQLCmdStdErr, 
			sizeof(szISQLCmdStdErr), &dwBytes, NULL, NULL);
		if (!bResult) {
			szISQLCmdStdErr[0] = '\0';
			cout<<"PeekNamedPipe() StdErrRead failed\n";
		}
		else {
			szISQLCmdStdErr[dwBytes] = '\0';
			if (szISQLCmdStdErr[0] != '\0') {
				cout<<szISQLCmdStdErr<<endl;
			}
		}

		CloseHandle(hStdErrRead);
	}
	else {
		szISQLCmdStdErr[0] = '\0';
	}

	if (NULL != hStdErrWrite) {
		CloseHandle(hStdErrWrite);
	}

	if (NULL != hStdOutWrite) {
		CloseHandle(hStdOutWrite);
	}
	system("pause");
	return 0;
}

void func()
{
	char sz[1024] = {0};
	memset(sz,0,sizeof(sz));
}
int main13()
{
	//PR_MKDir("D:\\123");
	for(int i = 0;i<10000000000000;i++)
		func();
    return 0;
}