#include <iostream>
#include "libmssqllogfunc.h"
using namespace std;



void main()
{
	char pszLogPath[256] = "C:\\Users\\yanqi\\Desktop\\c";
	char szInstance[256] = "MSSQLSERVER";
	char szDBName[256]   = "DB1019";
	MiLogHandle hLog;
	

	int nRC = WAVETOP_BACKUP_OK;
	int nLastnum = 0;
	PRUint64 nMax = 0;
	PRUint64 nMin = 0;

	char   szObjName[MAX_PATH]        = { 0 };
	char   szBAKName[MAX_PATH]        = { 0 };
	char   szCurrLSN[24]              = { 0 };
	//MiMSSqlBakSlot *pBakSlotGet       = NULL;
	MiMSSqlBakSt   *pBackSet          = NULL;
	ap_pool        *pParentPool       = NULL;
	ap_pool        *pDataPool         = NULL;
	ap_pool        *pPool             = NULL;
	ap_pool        *pAll              = NULL;
	ap_pool        *pBuffPool         = NULL;
	HANDLE          hTimer            = NULL;
	unsigned char  *pBackOut          = NULL;
	unsigned char  *pDataBuf          = NULL;
	unsigned long   nLenOut           = 0;
	MiLogHandle     LogHandle         = 0;
	PRInt64         nTempSeq          = 0;
	PRInt64         nLastSeq          = 0;
	int             nCount            = 0;
	bool			willRemind	      = true;
	bool            monitor_state     = true;   //增量监控状态,        true 为正常  ,false 为不正常
	int             nIsFull           = 0;
	int             nLastNum          = 0;
	CAssembleTransLogBackupSet *pBack = NULL;
	BackUpSetSt BackupInfo;
	MiMSSqlBakSlot *pBakSlotGet     = (MiMSSqlBakSlot *)malloc(sizeof(MiMSSqlBakSlot));
	unsigned short index;
    char szTemp[2048] = {0};

	BackupInfo.pIOQueue = NULL;
	BackupInfo.nCurrentSeq = 58;
	BackupInfo.nLastOffset = 14242304;
	BackupInfo.nFileSize = 0;
	BackupInfo.nIOCount = 0;


	//pBakSlotGet->szDBName = "db1017";
	PR_snprintf(pBakSlotGet->szDBName,sizeof(pBakSlotGet->szDBName),szDBName);
	PR_snprintf(pBakSlotGet->szInstName,sizeof(pBakSlotGet->szInstName),szInstance);
	pBakSlotGet->bActived = true;
	pBakSlotGet->bUsed = true;
	pBakSlotGet->bSyned = false;
	pBakSlotGet->bRestartThr = false;
	pBakSlotGet->bStatus = false;
	pBakSlotGet->nIncCnt = 0;
	pBakSlotGet->nSynStatus = 0;
	pBakSlotGet->nIncStatus = 0;
	pBakSlotGet->bWillCreateNewBak = true;
	pBakSlotGet->pNext	= NULL;

	PR_snprintf(pBakSlotGet->szClientIp,sizeof(pBakSlotGet->szClientIp),"2.2.2.40");





	pAll = ap_init_alloc();

	pBuffPool = ap_make_sub_pool(NULL);
	pDataBuf = (unsigned char *)ap_palloc(pBuffPool, MI_MSSQL2_BLOCK_SIZE);

	 pPool = ap_make_sub_pool(NULL);


	nRC = Init(szInstance,szDBName,&hLog,pszLogPath);
	snprintf(szTemp, sizeof(szTemp), "%s_%s", 
		szInstance, szDBName);

	nRC = MiMSSqlGetMaxAndMinNum(hLog,&nMax,&nMin);
	cout<<"MAx :  "<<nMax<<"  Min:  "<<nMin<<endl;
	//MiMS
	MiMSSqlDeleteFileImmediate2(pszLogPath,szTemp,nLastNum);

	nRC = GetIOQueue(pPool, hLog, pDataBuf, &BackupInfo, pBakSlotGet, szCurrLSN);

	//nRet = MiSqlGetIO(&pData, &nDataSize, &nSeqNum, hLog, pBuf, &nMaxSize);
	nRC = BkSqlStartService("Wavetop IO Daemon");
	cout<<nRC<<endl;

	//nRC = BkSqlStopService(servicename);


	
	system("pause");
}

/*
void main()
{
	int nRC = BkSqlStartService("LxpSvc");
	return;
}*/
