#include "func.h"
void MY_Thread(void *param)
{
	int Num;
	while(1)
	{
		int nRC  = GetActiveAlarm(&Num);
		if(nRC != 0)
		{
			cout<<"GetActiveAlarm error"<<endl;
			break;
		}

		if(Num == 0){
			cout<<"["<<getpid()<<"] ["<<GetCurrentThreadId() <<"] "<<" continue"<<endl;
			Sleep(10);
			continue;
		}
		else if(Num == 1)
			break;

	}
	return;
}

void MY_Thread2(void *param)
{
	char *pszStr = "haha";
	while(1)
	{
		//pszStr = GetAlarmPath();
		pszStr = ConvertToUTF(pszStr);
		free(pszStr);
		Sleep(100);
	}
	return;
}
void main()
{

	for(int i = 0;i<200;i++){
		PR_CreateThread(PR_USER_THREAD,MY_Thread,NULL,PR_PRIORITY_NORMAL,PR_GLOBAL_THREAD,PR_JOINABLE_THREAD,1 << 21);
		cout<<"Create a THread\n";
	}


	system("pause");
}