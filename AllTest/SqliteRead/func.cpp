#include "func.h"


#ifdef WIN32
static char *unicodeToUtf8(const WCHAR *zWideFilename)   
{   
	int nByte;   
	char *zFilename;   

	nByte = WideCharToMultiByte(CP_UTF8, 0, zWideFilename, -1, 0, 0, 0, 0);   
	zFilename = (char *)malloc(nByte);   
	if(zFilename == 0) {   
		return 0;   
	}   

	nByte = WideCharToMultiByte(CP_UTF8, 0, zWideFilename, -1, zFilename, nByte, 0, 0);   
	if( nByte == 0 ) {   
		free(zFilename);   
		zFilename = 0;   
	}   
	return zFilename;   
}   

static WCHAR *mbcsToUnicode(const char *zFilename)   
{   
	int nByte;   
	WCHAR *zMbcsFilename;   
	int codepage = AreFileApisANSI() ? CP_ACP : CP_OEMCP;   

	nByte = MultiByteToWideChar(codepage, 0, zFilename, -1, NULL,0)*sizeof(WCHAR);   
	zMbcsFilename = (WCHAR *)malloc(nByte*sizeof(zMbcsFilename[0]));   
	if( zMbcsFilename==0 ) {   
		return 0;   
	}   

	nByte = MultiByteToWideChar(codepage, 0, zFilename, -1,   zMbcsFilename, nByte);   
	if(nByte == 0) {   
		free(zMbcsFilename);   
		zMbcsFilename = 0;   
	}   
	return zMbcsFilename;   
}
#endif

char* ConvertToUTF(char *pszFile)
{
	unsigned short *wcPath;

	if (pszFile == NULL) {
		return NULL;
	}

#if WIN32
	wcPath = (unsigned short *)mbcsToUnicode(pszFile);   
	pszFile = unicodeToUtf8((const WCHAR *)wcPath);
#endif

	return pszFile;
}

char* GetAlarmPath()
{
	char *pszPos =  NULL;
	char *szDir =NULL;
	szDir = (char*)malloc(MAX_PATH);
#ifdef WIN32
	PRInt32 nRC = GetModuleFileName(NULL,szDir,MAX_PATH);
	if (nRC == sizeof(szDir)||nRC==0){
		free(szDir);
		return NULL;
	}
	pszPos = strrchr(szDir,'\\');
	if (pszPos == NULL) pszPos = strrchr(szDir, '/');
	if (pszPos == NULL){
		free(szDir);
		return NULL;
	}
	*pszPos = '\0';
#else
	if(!getcwd(szDir,sizeof(szDir))) return NULL;
	pszPos = strrchr(szDir,'\\');
	if (pszPos == NULL) pszPos = strrchr(szDir, '/');	
	if (pszPos == NULL)  return NULL;	
	*pszPos = '\0';
#endif
	PR_snprintf(szDir,MAX_PATH,"%s\\%s",szDir,BACKUP_ALARM_TABLE_FILE);
	szDir = ConvertToUTF(szDir);
	return szDir;
}

int GetActiveAlarm(int *pNum)
{
	char *pPath	             ="G:\\Mirror_Trunk_20170708\\mirror2\\dist\\Debug\\Win32\\alarm.db";
	char *pszErr             = NULL;
	int nResult              = ALARM_TABLE_OK;
	sqlite3 *db              = NULL;
	char** pResult           = NULL;
	char szAlarmSqlTemp[1024]= {0};
	char *szAlarmSql         = NULL;
	int nRow                 = 0;
	int nCol                 = 0;
	char szDir[260]  = "H:\\MultiThread.db";
	char *error              = "database is locked";
	*pNum = 0;

	/*获取exe文件路径*/
	/*pPath= GetAlarmPath();
	if(pPath == NULL){
		cout<<"pPath is NULL\n";
		return ALARM_TABLE_INTERNAL_ERROR;
	}*/
	//PR_snprintf(pPath,260,"G:\\Mirror_Trunk_20170708\\mirror2\\dist\\Debug\\Win32\\alarm.db");
	//pPath = ConvertToUTF(szDir);


	/* create alarm.db if exist not create */
	if (PR_Access(pPath, PR_ACCESS_EXISTS) != PR_SUCCESS) {
		if(nResult != ALARM_TABLE_OK){
			//free(pPath);
			return ALARM_FILE_NOEXIST;
		}
	}
	try
	{
		/* 打开数据库, 创建连接 */
		nResult = sqlite3_open(pPath, &db);
		if (nResult != SQLITE_OK) {
			//free(pPath);
			return ALARM_TABLE_OPEN_DB_ERROR;
		}

		sqlite3_busy_timeout(db,10*1000);

		memset(szAlarmSqlTemp,0,sizeof(szAlarmSqlTemp));
		PR_snprintf(szAlarmSqlTemp, sizeof(szAlarmSqlTemp),
			"select type,errorcode,IsSolved,starttime,lasttime,ip,moudle from alarminfo where IsSolved = 1;");
		//szAlarmSql=ConvertToUTF(szAlarmSqlTemp);

/*
		for(int i = 0;i<1200;i++){
			nResult = sqlite3_get_table(db,szAlarmSql,&pResult,&nRow,&nCol,&pszErr);
			if(nResult != SQLITE_OK){
				cout<<"exec error: "<<pszErr<<endl;
				if(strcmp(error,pszErr) == 0){
					Sleep(50);
					continue;
				}

				sqlite3_free(pszErr);
				throw nResult;
			}
		}*/
		nResult = sqlite3_get_table(db,szAlarmSqlTemp,&pResult,&nRow,&nCol,&pszErr);
		if(nResult != SQLITE_OK){
			if(pszErr)
				cout<<"exec error :"<<pszErr<<endl;
			else
				cout<<"exec error.pszErr is NULL\n";
			if (pResult != NULL) {
				sqlite3_free_table(pResult);
			}
			sqlite3_free(pszErr);
			sqlite3_close(db);
			return nResult;
		}
		

		//打印 所有 元素
		/*for (int i = nCol ; i <(nRow+1) * nCol && nRow != 0; i++ ){
			//cout<<"nResult["<<i<<"]: "<<pResult[i]<<endl;
		}*/

		nResult = ALARM_TABLE_OK;
	}
	catch (...){
		nResult = ALARM_TABLE_INTERNAL_ERROR;
	}
	if (pResult != NULL) {
		sqlite3_free_table(pResult);
	}
	if(pszErr)
	{
		sqlite3_free(pszErr);
	}
	//free(pPath);
	sqlite3_close(db);
	return nResult;

}

int GetActiveAlarm1(int *pNum)
{
	char *pPath	             = {0};
	char *pszErr             = NULL;
	int nResult              = ALARM_TABLE_OK;
	sqlite3 *db              = NULL;
	char** pResult           = NULL;
	char szAlarmSqlTemp[1024]= {0};
	char *szAlarmSql         = NULL;
	int nRow                 = 0;
	int nCol                 = 0;
	char szDir[260]  = "G:\\Mirror_Trunk_20170708\\mirror2\\dist\\Debug\\Win32\\alarm.db";
	char *error              = "database is locked";
	*pNum = 0;

	/*获取exe文件路径*/
	pPath= GetAlarmPath();
	if(pPath == NULL){
		throw ALARM_TABLE_FILEPATH_FAILURE;
	}
	//PR_snprintf(pPath,260,"G:\\Mirror_Trunk_20170708\\mirror2\\dist\\Debug\\Win32\\alarm.db");
	//pPath = ConvertToUTF(szDir);

	/* create alarm.db if exist not create */
	if (PR_Access(pPath, PR_ACCESS_EXISTS) != PR_SUCCESS) {
		if(nResult != ALARM_TABLE_OK){
			free(pPath);
			return ALARM_FILE_NOEXIST;
		}
	}

		/* 打开数据库, 创建连接 */
		nResult = sqlite3_open(pPath, &db);
		if (nResult != SQLITE_OK) {
			free(pPath);
			return ALARM_TABLE_OPEN_DB_ERROR;
		}

	sqlite3_close(db);
	return nResult;

}


