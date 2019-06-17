#include <iostream>
#include <Windows.h>
using namespace std;

#import "..\dist\sqldmo.rll" no_namespace

int main()
{


	int CompressNum = GetPrivateProfileInt("server", "TimingMssqlBakcupCompress", 0, "D:\\wbackup.conf");
	cout<<CompressNum<<endl;
	system("pause");
	return 0;
}

