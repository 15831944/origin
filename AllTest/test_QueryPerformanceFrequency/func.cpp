#include "func.h"
#include "nspr.h"


void time_test()
{
	LARGE_INTEGER starttime;
	LARGE_INTEGER endtime;
	LARGE_INTEGER nfreq;
	QueryPerformanceFrequency(&nfreq);

	QueryPerformanceCounter(&starttime);
	//Sleep(4000);
	PR_Sleep(1000);
	QueryPerformanceCounter(&endtime);
	cout<<(endtime.QuadPart-starttime.QuadPart)*1000/nfreq.QuadPart<<endl;
	if(((endtime.QuadPart-starttime.QuadPart)/nfreq.QuadPart)>3)
	{
		cout<<(endtime.QuadPart-starttime.QuadPart)*1000/nfreq.QuadPart<<endl;
	}
}

void PR_MKDIR_Test()
{
	if(PR_MkDir("D:\\test",0755) == PR_SUCCESS)
	cout<<"PR_MKdir success"<<endl;
	else
		cout<<"PR_MKdir failed"<<endl;
}

void CGI_Analysis()
{
	//char *pszArg = "-H \"-1\" -U \"2.2.2.40\" -O \"888888\" -Z \"2.2.2.40\" -C \"BACKUP\" -B \"401\"";
	//char *pszArg = "-H -1 -U \"2.2.2.40\" -O \"888888\" -Z \"2.2.2.40\" -C \"BACKUP\" -B \"401\"";
	//char *pszArg = "-H \"1\" -U \"2.2.2.40\" -O \"888888\" -Z \"2.2.2.40\" -C \"BACKUP\" -B \"401\"";
	char *pszArg = "-H \"1 -O \"123\" -A \"-1\" --H \"5\" ----------------------- -dafdf \"\"";
	//char *pszArg = "\0";
	char szKey[3];
	char szValue[4096];

	char *ppOut = NULL;
	char *pPos1;
	char *pPos2;

	while(pszArg = strchr(pszArg, '-')) {
		strncpy(szKey, pszArg, 2);
		szKey[2] = '\0';

		if (NULL == (pPos1 = strstr(pszArg, " \""))){
			pszArg += 1;
			continue;
		}
		pPos2 = strchr(pszArg + 2, '-');
		if (pPos2 != NULL) {
			if (pPos2 < pPos1) {
				//cJSON_AddStringToObject(pArg, szKey, "");
				cout<<"szKey: "<<szKey<<endl;
				cout<<"vlues: "<<""<<endl;
				pszArg = pPos2;
				continue;
			}
		}

		//if (*(pPos1 +=2) == '-')
		//	continue;
		pPos1 +=2;
		if (NULL == (pPos2 = strstr(pPos1, "\""))){
			pszArg += 2;
			continue;
		}
		strncpy(szValue, pPos1, pPos2 - pPos1);
		szValue[pPos2 - pPos1] = '\0';
		pszArg = pPos2;

		cout<<"szKey: "<<szKey<<endl;
		cout<<"szValue: "<<szValue<<endl;
		//cJSON_AddStringToObject(pArg, szKey, szValue);
	}
}


void CGI_Analysis2()
{
	char *pszArg = "-H \"1 -O \"123\" -A \"-1\" --H \"5\"";
	char szKey[3];
	char szValue[4096];

	char *ppOut = NULL;
	char *pPos1;
	char *pPos2;

	while(pszArg = strchr(pszArg, '-')) {
		strncpy(szKey, pszArg, 2);
		szKey[2] = '\0';

		if (NULL == (pPos1 = strstr(pszArg, " \"")))
			break;

		pPos2 = strchr(pszArg + 2, '-');
		if (pPos2 != NULL) {
			if (pPos2 < pPos1) {
				//cJSON_AddStringToObject(pArg, szKey, "");
				cout<<"szKey: "<<szKey<<endl;
				cout<<"vlues: "<<""<<endl;
				pszArg = pPos2;
				continue;
			}
		}

		if (*(pPos1 +=2) == '-')
			continue;

		if (NULL == (pPos2 = strstr(pPos1, "\"")))
			continue;

		strncpy(szValue, pPos1, pPos2 - pPos1);
		szValue[pPos2 - pPos1] = '\0';
		pszArg = pPos2;
		cout<<"szKey: "<<szKey<<endl;
		cout<<"vlues: "<<""<<endl;
		//cJSON_AddStringToObject(pArg, szKey, szValue);
	}
}

void time_test2()
{
	double starttime = time(NULL);
	Sleep(3000);
	double endtime = time(NULL);

	cout<<endtime - starttime<<endl;

}

void ThreeEleCalcu()
{
	int a =2;
	int b = 3;
	int c = a>b?a:b;
	cout<<c<<endl;
}

void test_PR_sn()
{
	char sz[1024] = "12345";
	char sz2[1024] = {0};
	PR_snprintf(sz,sizeof(sz),"123%s",sz);
	cout<<sz2<<endl;
}

void Printflld()
{
	time_t time_now = 0;

	time(&time_now);

	printf("现在的时间为： %lld, %s",time_now,"hello ,world");
}

void time_test4()
{
	time_t tick = 0;
	struct tm tm;
	char s[100];

	tick = time(NULL);
	tm = *localtime(&tick);
	strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S", &tm);
	printf("%d: %s\n", (int)tick, s);
}

void SQLASSEMBLE()
{

	char *pszSQLStmt            = NULL;
	pszSQLStmt = (char *) malloc(4096);
	 int nSQLLen = 4096;
	PR_snprintf(pszSQLStmt, 
		nSQLLen, "use [master]\r\n"
				"EXEC sp_detach_db '%s', 'true'\r\n"
				"CREATE DATABASE %s ON",
				 "DB1019",
				 "DB1019");
	for (int i = 0; i<20; i++) {
			PR_snprintf(pszSQLStmt, nSQLLen, "%s \(filename=N'%s'\),",
				pszSQLStmt,"C:\\DBdata\\db1019.ndf");
	}
	*(pszSQLStmt+strlen(pszSQLStmt)-1) = '\0';
	PR_snprintf(pszSQLStmt, nSQLLen, "%s\r\n for attach\r\n go\r\n", pszSQLStmt);
	cout<<"pszSQLStmt"<<pszSQLStmt<<"  "<<strlen(pszSQLStmt)<<endl;
	free(pszSQLStmt);

	
	char *psz ="";
		//cout<<"psz 不为空"<<endl;
	//cout<<strlen(psz);
	//*(psz-1) = '\0';


	char *sz1 = NULL;
    int a = 100;
	int b = 100;
	if((a = 10 <0)&&( b = 2>0 ))
		cout<<"sz \n";
	cout<<a<<endl;
	cout<<b<<endl;

}
void Setbin()
{
	char sztemp[16] = "50";
	WritePrivateProfileString("mssql2", "FileNum", sztemp, "D:\\11.bin");
}


int SetFileNormal(const char *pszFilename)
{
#ifdef WIN32
	SetFileAttributes(pszFilename, FILE_ATTRIBUTE_NORMAL);
#else
#endif
	return WAVETOP_BACKUP_OK;
}

int BackupStat(const char *pszFilename, PRInt32 *nType, PRInt64 *nSize)
{
	PRFileInfo64 fInfo;

	if (PR_GetFileInfo64(pszFilename, &fInfo) != PR_SUCCESS)
		return WAVETOP_BACKUP_FILE_NOT_EXIST;
	if (fInfo.type == PR_FILE_DIRECTORY) {
		*nType = BACKUP_FILE_DIRECTORY;
	}
	else  {
		*nType = BACKUP_FILE_FILE;
		*nSize = fInfo.size;
	}

	return WAVETOP_BACKUP_OK;
}
int BackupDelete(char *pszFilename)
{
#ifdef WIN32
	SetFileNormal(pszFilename);
	return (DeleteFile(pszFilename) ? 
WAVETOP_BACKUP_OK : WAVETOP_BACKUP_OPEN_FILE_ERROR);
#else
	return (0 == remove(pszFilename) ? 
WAVETOP_BACKUP_OK : WAVETOP_BACKUP_OPEN_FILE_ERROR);
#endif
}
#ifdef WIN32
int BackupCreateParentDir(const char *pszFile)
{
	char szDir[1024];
	char *pszFlag;
	PRInt32 nType;
	PRInt64 nSize;
	int nRC;
	int nEnd;
	int i;

	if (strlen(pszFile) >= sizeof(szDir))
		return WAVETOP_BACKUP_OPEN_FILE_ERROR;

	for (i = 0; *(pszFile + i); i++) {
		szDir[i] = (*(pszFile + i) == '/' ? '\\' : *(pszFile + i));
	}
	szDir[i] = '\0';
	/* get the parent directory */
	pszFlag = strrchr(szDir, '\\');
	if (pszFlag == NULL) {
		return 0;
	}
	*pszFlag = '\0';

	if (szDir[0] && (szDir[1] != ':'))
		return WAVETOP_BACKUP_INVALID_SYNTAX;

	pszFlag = &szDir[2];
	if (*pszFlag == '\0')
		return WAVETOP_BACKUP_OK;
	if (*pszFlag != '\\')
		return WAVETOP_BACKUP_INVALID_SYNTAX;
	pszFlag++;
	if (*pszFlag == '\0')
		return WAVETOP_BACKUP_OK;

	for (nEnd = 0;;) {
		for (; *pszFlag != '\0' && *pszFlag != '\\'; pszFlag++);
		if (*pszFlag == '\\')
			*pszFlag = '\0';
		else 
			nEnd = 1;

		nRC = BackupStat(szDir, &nType, &nSize);
		if (nRC == WAVETOP_BACKUP_FILE_NOT_EXIST) {
			if (CreateDirectory(szDir, NULL) == 0)
				return WAVETOP_BACKUP_CREAT_DIR_ERROR;
		}
		else if (nType == PR_FILE_FILE) {
			BackupDelete(szDir);
			if (CreateDirectory(szDir, NULL) == 0)
				return WAVETOP_BACKUP_CREAT_DIR_ERROR;
		}

		if (nEnd) {
			break;
		}
		else {
			*pszFlag = '\\';
			pszFlag++;
		}
	}

	return WAVETOP_BACKUP_OK;
}
#else
int BackupCreateParentDir(const char *pszFile)
{
	char szDir[1024];
	char *pszFlag;
	PRInt32 nType;
	PRInt64 nSize;
	int nRC;
	int nEnd;
	int i;

	if (strlen(pszFile) >= sizeof(szDir))
		return WAVETOP_BACKUP_OPEN_FILE_ERROR;

	for (i = 0; *(pszFile + i); i++) {
		szDir[i] = (*(pszFile + i) == '\\' ? '/' : *(pszFile + i));
	}
	szDir[i] = '\0';
	/* get the parent directory */
	pszFlag = strrchr(szDir, '/');
	if (pszFlag == NULL) {
		return 0;
	}
	*pszFlag = '\0';

	if (szDir[0] != '/')
		return WAVETOP_BACKUP_INVALID_SYNTAX;
	pszFlag = &szDir[1];
	if (*pszFlag == '\0')
		return WAVETOP_BACKUP_INVALID_SYNTAX;

	for (nEnd = 0;;) {
		for (; *pszFlag != '\0' && *pszFlag != '/'; pszFlag++);
		if (*pszFlag == '/')
			*pszFlag = '\0';
		else 
			nEnd = 1;

		nRC = BackupStat(szDir, &nType, &nSize);
		if (nRC == WAVETOP_BACKUP_FILE_NOT_EXIST) {
			if (PR_MkDir(szDir, 0755) != PR_SUCCESS)
				return WAVETOP_BACKUP_CREAT_DIR_ERROR;
		}
		else if (nType == PR_FILE_FILE) {
			BackupDelete(szDir);
			if (PR_MkDir(szDir, 0755) != PR_SUCCESS)
				return WAVETOP_BACKUP_CREAT_DIR_ERROR;
		}

		if (nEnd) {
			break;
		}
		else {
			*pszFlag = '/';
			pszFlag++;
		}
	}

	return WAVETOP_BACKUP_OK;
}
#endif

int BackupCreateDir(const char *pszDir)
{
	int nRC;

	if (PR_MkDir(pszDir, 0755) == PR_SUCCESS)
		return WAVETOP_BACKUP_OK;

	nRC = BackupCreateParentDir(pszDir);
	if (nRC != WAVETOP_BACKUP_OK)
		return nRC;

	if (PR_MkDir(pszDir, 0755) == PR_SUCCESS)
		return WAVETOP_BACKUP_OK;

	return WAVETOP_BACKUP_CREAT_DIR_ERROR;
}

int BkMssqlLogDataIsExist(char *pszFileName)
{
	PRFileInfo FileLogInfo;
	PRFileInfo FileTmpInfo;
	//char *pszFileName = NULL;
	char szLogData[MAXPATHLEN] = {0};
	char szTemp[MAXPATHLEN]    = {0};
	int nRC = WAVETOP_BACKUP_OK;

	PR_snprintf(szLogData, sizeof(szLogData), "%s/%s", pszFileName, BACKUP_LOG_DATA_FILE);
	PR_snprintf(szTemp, sizeof(szTemp), "%s/%s", pszFileName, BACKUP_DB_TEMP);

	if (PR_GetFileInfo(szLogData, &FileLogInfo) != PR_SUCCESS) {
		nRC = BackupCreateDir(szLogData);
		if (nRC != WAVETOP_BACKUP_OK) {
			/*SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR,
				GetRequestUser(pReq), "create dir failed.指定还原目录不存在");*/
			cout<<"create dir failed.指定还原目录不存在  11 "<<endl;
			return WAVETOP_BACKUP_CREAT_DIR_ERROR;
		}
	}
	cout<<"设置szLogData成功： "<<szLogData<<endl;
	/* 设置用户logdata路径 */
	//SetHeaderField(pReq, "Logdata-Dir", szLogData);    

	if (PR_GetFileInfo(szTemp, &FileTmpInfo) != PR_SUCCESS) {
		nRC = BackupCreateDir(szTemp);
		if (nRC != WAVETOP_BACKUP_OK) {
			/*SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR,
				GetRequestUser(pReq), "create dir failed.指定还原目录不存在");*/
			cout<<"create dir failed.指定还原目录不存在 22"<<endl;
			return WAVETOP_BACKUP_CREAT_DIR_ERROR;
		}
	}
	cout<<"设置szTemp成功： "<<szTemp<<endl;
	/* 设置用户logdata路径 */
	//SetHeaderField(pReq, "Temp-Dir", szTemp);    

	return nRC;
}


void BkFindMaxFileNum(char * pszDir, char *pszFileName, int *nFileNum){
	int 				nMaxNum = 0;
	int					nRC = WAVETOP_BACKUP_OK;

#if defined(WIN32) || defined(WINDOWS)
	struct 	_finddata_t	fileinfo;
	char 	pszPath[1024];

	PR_snprintf(pszPath, sizeof(pszPath), "%s/%s.*", pszDir, pszFileName);

	//查找第一个文件
	long handle = _findfirst(pszPath, &fileinfo);
	if (-1 == handle)
		return;

	string stTmp(fileinfo.name);
	string pszFileSuFixx(stTmp.substr(stTmp.length() - 6, stTmp.length() - 1));
	nMaxNum = atoi(pszFileSuFixx.c_str());

	//循环查找
	while (!_findnext(handle, &fileinfo))
	{
		string stTmp(fileinfo.name);
		string pszFileSuFixx(stTmp.substr(stTmp.length() - 6, stTmp.length() - 1));
		if (atoi(pszFileSuFixx.c_str()) > nMaxNum)
			nMaxNum = atoi(pszFileSuFixx.c_str());
	}

	_findclose(handle);
#else
	DIR *pDirHandle;
	struct dirent *stDirp;

	if((pDirHandle = opendir(pszDir))==NULL)
		return;

	while((stDirp=readdir(pDirHandle)) != NULL)
	{
		if(strcmp(stDirp->d_name,".") == 0 || strcmp(stDirp->d_name,"..") == 0)
			continue;

		string stTmp(stDirp->d_name);

		if(stTmp.find(pszFileName) == string::npos)
			continue;

		string pszFileSuFixx(stTmp.substr(stTmp.length() - 6, stTmp.length() - 1));
		if (atoi(pszFileSuFixx.c_str()) > nMaxNum)
			nMaxNum = atoi(pszFileSuFixx.c_str());
	}
#endif

	*nFileNum = nMaxNum;


}
/*
static int MiTestDBIsRecovering(MSSqlImportOpt *pImp,void *pCallbackReserver,
	MiMSSqlOperate nOpt, HANDLE handle)
{   
	
}*/
/*
void OSQL_TEST()
{
	char         szSQL[1024]        = { 0 };
	char         szSQL1[1024]       = { 0 };
	SQLHSTMT 	 hstmt              = SQL_NULL_HSTMT;
	SQLINTEGER 	 NativeError        = 0;
	int          nResult            = WAVETOP_BACKUP_OK;
	int          state              = 0;
	CSqlConnect  SqlConn;
	SQLRETURN   rc;  

	nResult = SqlConn.SetEnv((char* )pImp->szOptInstName,"127.0.0.1", 
		pImp->stMSSql.stMSSqlServ.port, pImp->stMSSql.stMSSqlServ.szUser, 
		pImp->stMSSql.stMSSqlServ.szPasswd);

	if (nResult != WAVETOP_BACKUP_OK) {
		MiMSSqlWriteLog(pCallbackReserver, nOpt, 
			nResult, WT_MI_MSSQL_LOG_DEBUG_MARK,
			"MiTestDBIsRecovering : SetEnv failed nResult=%d.",nResult);
		//PR_Unlock(g_MiGlobalConf.pGbaseLck);
		return nResult;
	}

	MiMSSqlWriteLog(pCallbackReserver, nOpt, 
		nResult, WT_MI_MSSQL_LOG_DEBUG_MARK,
		"MiTestDBIsRecovering begin!");

	nResult = SqlConn.Connect();
	if (nResult != WAVETOP_BACKUP_OK) {
		MiMSSqlWriteLog(pCallbackReserver, nOpt, 
			nResult, WT_MI_MSSQL_LOG_DEBUG_MARK,
			"MiTestDBIsRecovering : Connect failed. nResult=%d",nResult);
		//PR_Unlock(g_MiGlobalConf.pGbaseLck);
		return nResult;
	}

	PR_snprintf(szSQL, sizeof(szSQL),
		"select state from sys.databases "
		"where name = '%s'",
		pImp->szOptDBName);

	while(1){
		hstmt = SqlConn.Allochstmt();
		if (hstmt == SQL_NULL_HSTMT) {
			MiMSSqlWriteLog(pCallbackReserver, nOpt, 
				nResult, WT_MI_MSSQL_LOG_DEBUG_MARK,
				"MiTestDBIsRecovering : Allochstmt failed.");
			nResult = WAVETOP_BACKUP_INTERNAL_ERROR;
			Sleep(1000);
			return nResult;
		}

		rc = SQLExecDirect(hstmt, (SQLCHAR*)szSQL, SQL_NTS);
		if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO) {
			nResult = WAVETOP_BACKUP_INTERNAL_ERROR;
			MiMSSqlWriteLog(pCallbackReserver, nOpt, 
				nResult, WT_MI_MSSQL_LOG_DEBUG_MARK,
				"MiTestDBIsRecovering() SQLExecDirect() Failed.");
			SqlConn.FreeHstmt(hstmt);
			Sleep(1000);
			return nResult;
		}

		SQLBindCol(hstmt, 1, SQL_C_CHAR, szSQL1, sizeof(szSQL1), &NativeError);

		if (SQLFetch(hstmt) != SQL_SUCCESS) {
			MiMSSqlWriteLog(pCallbackReserver, nOpt, 
				nResult, WT_MI_MSSQL_LOG_DEBUG_MARK,
				"MiTestDBIsRecovering() SQLFetch() Failed.");
			nResult = WAVETOP_BACKUP_INTERNAL_ERROR;
			SqlConn.FreeHstmt(hstmt);
			Sleep(1000);
			return nResult;
		}			

		if(strcmp(szSQL1,"\0") != 0)
			state = atoi(szSQL1);
		else
			state = 3;  //如果数据库不存在，则返回 WAVETOP_BACKUP_INTERNAL_ERROR

		switch(state){
		case 0:  // ONLINE 则正常退出
			SqlConn.FreeHstmt(hstmt);
			MiMSSqlWriteLog(pCallbackReserver, nOpt, 
				nResult, WT_MI_MSSQL_LOG_DEBUG_MARK,
				"MiTestDBIsRecovering success!");
			SqlConn.FreeHstmt(hstmt);
			return nResult;
		case 1:  // RESTORING 
			SqlConn.FreeHstmt(hstmt);
			Sleep(10000);
			continue;
		case 2:  //正在恢复 或者 RECOVERING
			SqlConn.FreeHstmt(hstmt);
			Sleep(10000);
			continue;
		default: //其他状态 如 SUSPECT  ，OFFLINE等异常状态
			SqlConn.FreeHstmt(hstmt);
			nResult = WAVETOP_BACKUP_INTERNAL_ERROR;
			Sleep(1000);
			return nResult;
		}		
	}
	return nResult;
}*/