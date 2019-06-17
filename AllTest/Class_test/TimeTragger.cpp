#include <Windows.h>
#include <iostream>
#include <time.h>
using namespace std;

long long time1 =  time(NULL);


int TimeTrigger(HANDLE *phTimer, LONGLONG lBeginTime, LONG lInterval, bool bAutoReset)
{
    LARGE_INTEGER 	liDueTime;	
    liDueTime.QuadPart = -1000*1000*100; //10��
    //������ʱ��
    if(*phTimer == NULL){
        *phTimer = CreateWaitableTimer(NULL, !bAutoReset, NULL);
        if (!(*phTimer)) {
            cout<<"������ʱ��ʧ��"<<endl;
        }
    }

    //���ö�ʱ������
    if (!SetWaitableTimer(*phTimer, &liDueTime, lInterval, NULL, NULL, 0)) {
        cout<<"���ö�ʱ��ʧ��"<<endl;
    }
    return 0;
}
void test()
{
    HANDLE 			 hCheckDiskTimer = NULL;
    int nRet = 0;
    TimeTrigger(&hCheckDiskTimer ,1 ,60*1000,false);
    while(1){
        nRet = WaitForSingleObject(hCheckDiskTimer, 120*1000);
        if(nRet == WAIT_OBJECT_0){
            cout<<time(NULL)- time1<<endl;
        }else if(nRet != WAIT_OBJECT_0){
            cout<<"�ȴ���ʱ��ʧ��\n";
        }	
        Sleep(1000);
    }



}

#ifdef TIMETRAGGERTEST
void main()
{
    HANDLE hTimer = CreateWaitableTimer( NULL,true,NULL );  

    LARGE_INTEGER li;  

    li.QuadPart = -1000*1000*10;   
    LARGE_INTEGER li2;  

    li2.QuadPart = -1000*1000*100;   
    LARGE_INTEGER li3;  

    li3.QuadPart = time(NULL)+ 30*1000;

    if( !SetWaitableTimer( hTimer,&li3,5*1000,NULL,NULL,FALSE ))  

    {  

        CloseHandle( hTimer );  

        return;  

    }  

    while ( 1 )  

    {  

        //clock_t c_beg = clock();  

        int nRet = WaitForSingleObject( hTimer,0 );
        if(nRet == WAIT_OBJECT_0){
            cout<<"��ʱ��������...\n"; 
            SetWaitableTimer( hTimer,&li2,0,NULL,NULL,FALSE );
        }


        cout<<"ѭ��һ��"<<endl;
        Sleep(1000);

        //clock_t end = clock() - c_beg;  
        //cout<<"time:"<<end<<endl;  
    }  
    CloseHandle(hTimer); 
    system("pause");
}

#endif

VOID APIENTRY TimerAPCRoutine( PVOID pvArgToCompletionRoutine,DWORD dwTimerLowValue,DWORD dwTimerHighValue)

{

    cout<<"high:"<<dwTimerHighValue<<"      low"<<dwTimerLowValue<<endl;

}


