#include <iostream>
#include "configfile.h"
#include <Windows.h>


using namespace std;


void main()
{
	char *pszSrvIP_conf                = NULL;
	char szSection[64] = {0};
    char *pszPathName = "D:\\wbackup.conf";
    char *pAppName = "mode_0";
    if(PR_Access(pszPathName,PR_ACCESS_EXISTS) != PR_SUCCESS)
        WritePrivateProfileString(pAppName,"Username", "2.2.2.8", pszPathName);

	CConfigfile *pCfg             = NULL;
	pCfg = new CConfigfile("D:\\wbackup.conf");
	if (pCfg == NULL) {
		cout<<"pCfg Memory failed"<<"\n";

	}

    pCfg->WriteInt(pAppName,"nVersion", 1);
    pCfg->WriteInt(pAppName,"nLastVersion", 5);

   // pszSrvIP_conf = pCfg->ReadString(pAppName,"Username");

	pszSrvIP_conf = strdup(pCfg->ReadString("Mode_0", "Username"));

	if(pszSrvIP_conf != NULL && strcmp(pszSrvIP_conf,"\0")){
		cout<<"pszSrvIP_conf ²»Îª¿Õ"<<endl;
	}

	




	system("pause");
}