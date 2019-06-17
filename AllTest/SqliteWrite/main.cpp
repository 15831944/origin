#include "alarm.h"
#include <iostream>
using namespace std;

int main(int argc,char **argv)
{
	cout<<"begin sqlitewrite"<<endl;
	char szAppPath[MAX_PATH] = {0};
	char *szDir              = NULL;
	int nResult;


	/*写入数据到alarm.db*/
	nResult = WriteDB_Multi();
	if(nResult != ALARM_TABLE_OK)
		cout<<"write error: "<<nResult<<endl;
	else
		cout<<"AlarmWriteSqliteDB succeed"<<endl;

	nResult = GetCountDB();
	if(nResult != ALARM_TABLE_OK)
		cout<<"getCount error: "<<nResult<<endl;

	system("pause");
	return 0;
}