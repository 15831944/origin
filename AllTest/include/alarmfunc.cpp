#include "alarm.h"
#include <iostream>
using namespace std;

#define OPTERRCOLON (1)
#define OPTERRNF (2)
#define OPTERRARG (3)


char *optarg;
int optreset = 0;
int optind = 1;
int opterr = 1;
int optopt;


static int optiserr(int argc, char * const *argv, int oint,
	const char *optstr, int optchr, int err)
{
    if(opterr)
    {
        fprintf(stderr, "Error in argument %d, char %d: ", oint, optchr+1);
        switch(err)
        {
        case OPTERRCOLON:
            fprintf(stderr, ": in flags\n");
            break;
        case OPTERRNF:
            fprintf(stderr, "option not found %c\n", argv[oint][optchr]);
            break;
        case OPTERRARG:
            fprintf(stderr, "no argument for option %c\n", argv[oint][optchr]);
            break;
        default:
            fprintf(stderr, "unknown\n");
            break;
        }
    }
    optopt = argv[oint][optchr];
    return('?');
}

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

static char* ConvertToUTF(char *pszFile)
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

char* GetExeDir()
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
	szDir = ConvertToUTF(szDir);
	return szDir;
}

char* GetExePath()
{
	char *szDir = GetExeDir();
	PR_snprintf(szDir,MAX_PATH,"%s\\%s",szDir,BACKUP_ALARM_TABLE_FILE);
	szDir = ConvertToUTF(szDir);
	return szDir;
}

/* ´´½¨¸æ¾¯µÄ±í */
static PRInt32 BkCreatealarmTable(void)
{
	char *psztable        = NULL;
	char *pPath           = NULL;
	char *msg             = NULL;
	sqlite3 *db           = NULL;    
	PRInt32 nRC           = 0;

	/*»ñÈ¡exeÎÄ¼þÂ·¾¶*/
	pPath= GetExePath();
	if(pPath == NULL){
		throw ALARM_TABLE_FILEPATH_FAILURE;
	}

	psztable = "create table alarminfo(id integer primary key autoincrement,"
		"type integer,errorcode integer,IsSolved boolean,starttime timestamp,"
		"lasttime timestamp not null default (datetime('now','localtime')),"
		"ip varchar(256),moudle varchar(100))";

	try
	{
		/* ´ò¿ªÊý¾Ý¿â, ´´½¨Á¬½Ó */
		nRC = sqlite3_open(pPath, &db);
		if (nRC != SQLITE_OK) {
			free(pPath);
			return ALARM_TABLE_OPEN_DB_ERROR;
		}

		/* create table. alarm.db */        
		nRC=sqlite3_exec(db, psztable, 0, 0, &msg);
		if(nRC!=SQLITE_OK){
			throw nRC;
		}
	}
	catch (...)
	{
		nRC = ALARM_TABLE_CREATE_ERROR;
	}

	free(pPath);
	/* close database */
	sqlite3_close(db);
	return nRC;
}

/*  Ð´Èë¸æ¾¯ÄÚÈÝ */
int AlarmWriteSqliteDB(MiAlarmCmd &alarmsql)
{
	char szSqlBuffRept[2048] = { 0 };
	char *pPath              = NULL;
	char *pszErr             = NULL;
	char *pszErr2            = NULL;
	char *pos                = NULL; 
	PRInt32 nResult          = 0;
	sqlite3 *db              = NULL;
	char** pResult           = NULL;
	int nRow                 = 0;
	int nCol                 = 0;
	char szAlarmSqlTemp[1024]= {0};
	char *szAlarmSql         = NULL;
	char *error              = "database is locked";

	double Freq = 0.0;
	PRInt64 CounterStart = 0;
	PRInt64 CounterEnd = 0;
	LARGE_INTEGER li ;
	if(!QueryPerformanceFrequency(&li))
		cout << "QueryPerformanceFrequency failed!\n";
	Freq = double(li.QuadPart)/1000;
	QueryPerformanceCounter(&li);
	CounterStart = li.QuadPart;

	LARGE_INTEGER Starttime;
	LARGE_INTEGER Endtime;
	LARGE_INTEGER Freq;
	QueryPerformanceFrequency(&Freq);
	QueryPerformanceCounter(&Starttime);
	/*»ñÈ¡exeÎÄ¼þÂ·¾¶*/
	pPath= GetExeDir();
	if(pPath == NULL){
		throw ALARM_TABLE_FILEPATH_FAILURE;
	}

	try {
		/* create alarm.db if exist not create */
		if (PR_Access(pPath, PR_ACCESS_EXISTS) != PR_SUCCESS) {
			nResult=BkCreatealarmTable();
			if(nResult != ALARM_TABLE_OK){
				free(pPath);
				return nResult;
			}
		}


		/* ´ò¿ªÊý¾Ý¿â, ´´½¨Á¬½Ó */
		nResult = sqlite3_open(pPath, &db);
		if (nResult != SQLITE_OK) {
			free(pPath);
			return ALARM_TABLE_OPEN_DB_ERROR;
		}
		sqlite3_busy_timeout(db,60*1000);
#ifndef AP
		/*²éÑ¯ÊÇ·ñÒÑ¾­´æÔÚÒªµ¼ÈëµÄÄÚÈÝ*/
		PR_snprintf(szSqlBuffRept, sizeof(szSqlBuffRept),
			"select errorcode from alarminfo where errorcode = %d",alarmsql.errorcode);

		nResult = sqlite3_get_table(db,szSqlBuffRept,&pResult,&nRow,&nCol,&pszErr);
		if (nResult != SQLITE_OK){
			sqlite3_free(pszErr);
			throw nResult;
		}		
#endif
		
		/*ÈôÊý¾Ý¿âÖÐÒÑ¾­ÓÐÁË´Ë´ÎÐè²åÈëµÄÄÚÈÝ£¬ÔòÎÞÐè²åÈë*/
		if(nRow==0) {
			PR_snprintf(szSqlBuffRept, sizeof(szSqlBuffRept),          
				"insert into alarminfo(type,errorcode,IsSolved,starttime,ip,moudle) values( %d,%d,%d,datetime('now','localtime'),'%s', '%s');",alarmsql.type,
				alarmsql.errorcode,alarmsql.IsSolved,alarmsql.ip,alarmsql.moudle);
			szAlarmSql = ConvertToUTF(szSqlBuffRept);
			QueryPerformanceCounter(&li);
			CounterStart = li.QuadPart;

/*
			for(int i = 0;i<1200;i++){
				nResult=sqlite3_exec(db, szAlarmSql, 0, 0, &pszErr);
				if(nResult!=SQLITE_OK ){
					cout<<"exec error: "<<pszErr<<endl;
					if(strcmp(error,pszErr) == 0)
					{
						Sleep(50);
						continue;
					}
					sqlite3_free(pszErr);
					throw nResult;
				}
				break;
			}*/
			nResult=sqlite3_exec(db, szAlarmSql, 0, 0, &pszErr);
			if(nResult!=SQLITE_OK ){
				cout<<"exec error: "<<pszErr<<endl;
				sqlite3_free(pszErr);
				throw nResult;
			}

		}
		else{
			PR_snprintf(szSqlBuffRept, sizeof(szSqlBuffRept),          
				"update alarminfo set IsSolved = %d,lasttime = datetime('now','localtime') where errorcode = %d and IsSolved = 1;",alarmsql.IsSolved,alarmsql.errorcode);
			/*for(int i = 0;i<1200;i++){
				nResult = sqlite3_exec(db,szSqlBuffRept,0,0,&pszErr);
				if(nResult != SQLITE_OK){
					cout<<"exec error: "<<pszErr<<endl;
					if(strcmp(error,pszErr) == 0){
						Sleep(50);
						continue;
					}
					sqlite3_free(pszErr);
					throw nResult;
				}
				break;
			}*/
			nResult = sqlite3_exec(db,szSqlBuffRept,0,0,&pszErr);
			if(nResult != SQLITE_OK){
				cout<<"exec error: "<<pszErr<<endl;
				sqlite3_free(pszErr);
				throw nResult;
			}
		}
		nResult = ALARM_TABLE_OK;
	}
	catch (...) {
		nResult = ALARM_TABLE_EXEC_ERROR;
	}
	QueryPerformanceCounter(&Endtime);
	cout<<"sqlite3_execÖ´ÐÐÊ±¼äÎª£º "<<double((Starttime.QuadPart-Endtime.QuadPart)*1000)/Freq.QuadPart<<endl;
	free(pPath);
	sqlite3_close(db);
	return nResult;
}

/*É¾³ý¸æ¾¯ÄÚÈÝ*/
int AlarmUpdataSqliteDB(MiAlarmCmd *alarmsql)
{
	char szSqlBuffRept[2048] = { 0 };
	char *pPath              = NULL;
	char *pszErr             = NULL;
	PRInt32 nResult          = 0;
	sqlite3 *db              = NULL;	
	char *szAlarmSql         = NULL;
	/*»ñÈ¡exeÎÄ¼þÂ·¾¶*/
	pPath= GetAlarmPath();
	if(pPath == NULL){
		throw ALARM_TABLE_FILEPATH_FAILURE;
	}

	try {
		/* create alarm.db if exist not create */
		if (PR_Access(pPath, PR_ACCESS_EXISTS) != PR_SUCCESS) {
			return ALARM_TABLE_FILEPATH_FAILURE;
		}

		/* ´ò¿ªÊý¾Ý¿â, ´´½¨Á¬½Ó */
		nResult = sqlite3_open(pPath, &db);
		if (nResult != SQLITE_OK) {
			return 5;
		}
		/*´´½¨sqlÓï¾ä*/
		PR_snprintf(szSqlBuffRept, sizeof(szSqlBuffRept),          
			"delete from alarm where content = '%s';",alarmsql->content);
		/*×ª³ÉUTF-8¸ñÊ½*/
		szAlarmSql=ConvertToUTF(szSqlBuffRept);
		/*Ö´ÐÐsqlÉ¾³ý²Ù×÷*/
		nResult = sqlite3_exec(db,szAlarmSql,0,0,&pszErr);
		if(nResult!=SQLITE_OK){	
			sqlite3_free(pszErr);
			throw nResult;
		}
		nResult=ALARM_TABLE_OK;
	}
	catch (...) {
		if (nResult == ALARM_TABLE_OK) {
			nResult = ALARM_TABLE_INTERNAL_ERROR;
		}
	}
	/*¹Ø±ÕÊý¾Ý¿â*/
	sqlite3_close(db);
	return nResult;
}

/* ¶ÁÈ¡Î´´¦ÀíµÄ¸æ¾¯ÄÚÈÝ */
int GetActiveAlarm()
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

	/*»ñÈ¡exeÎÄ¼þÂ·¾¶*/
	pPath= GetAlarmPath();
	if(pPath == NULL){
		throw ALARM_TABLE_FILEPATH_FAILURE;
	}

	/* create alarm.db if exist not create */
	if (PR_Access(pPath, PR_ACCESS_EXISTS) != PR_SUCCESS) {
		if(nResult != ALARM_TABLE_OK){
			free(pPath);
			return ALARM_FILE_NOEXIST;
		}
	}

	try
	{
		/* ´ò¿ªÊý¾Ý¿â, ´´½¨Á¬½Ó */
		nResult = sqlite3_open(pPath, &db);
		if (nResult != SQLITE_OK) {
			free(pPath);
			return ALARM_TABLE_OPEN_DB_ERROR;
		}
		memset(szAlarmSqlTemp,0,sizeof(szAlarmSqlTemp));
		PR_snprintf(szAlarmSqlTemp, sizeof(szAlarmSqlTemp),
			"select type,errorcode,IsSolved,starttime,lasttime,ip,moudle from alarminfo where IsSolved = 1;");
		szAlarmSql=ConvertToUTF(szAlarmSqlTemp);



		nResult = sqlite3_get_table(db,szAlarmSql,&pResult,&nRow,&nCol,&pszErr);
		if(nResult != SQLITE_OK){
			sqlite3_free(pszErr);
			throw nResult;
		}

		for (int i = nCol ; i <(nRow+1) * nCol && nRow != 0; i++ ){
			cout<<"nResult[i]: "<<pResult[i]<<endl;
		}

		nResult = ALARM_TABLE_OK;
	}
	catch (...){
		nResult = ALARM_TABLE_INTERNAL_ERROR;
	}
	
	free(pPath);
	sqlite3_close(db);
	return nResult;

}

int GetHistoryAlarm()
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

	/*»ñÈ¡exeÎÄ¼þÂ·¾¶*/
	pPath= GetAlarmPath();
	if(pPath == NULL){
		throw ALARM_TABLE_FILEPATH_FAILURE;
	}

	/* create alarm.db if exist not create */
	if (PR_Access(pPath, PR_ACCESS_EXISTS) != PR_SUCCESS) {
		if(nResult != ALARM_TABLE_OK){
			free(pPath);
			return ALARM_FILE_NOEXIST;
		}
	}

	try
	{
		/* ´ò¿ªÊý¾Ý¿â, ´´½¨Á¬½Ó */
		nResult = sqlite3_open(pPath, &db);
		if (nResult != SQLITE_OK) {
			free(pPath);
			return ALARM_TABLE_OPEN_DB_ERROR;
		}

		sqlite3_busy_timeout(db,60*1000);

		memset(szAlarmSqlTemp,0,sizeof(szAlarmSqlTemp));
		PR_snprintf(szAlarmSqlTemp, sizeof(szAlarmSqlTemp),
			"select type,errorcode,IsSolved,starttime,lasttime,ip,moudle from alarminfo where IsSolved = 0;");
		szAlarmSql=ConvertToUTF(szAlarmSqlTemp);



		nResult = sqlite3_get_table(db,szAlarmSql,&pResult,&nRow,&nCol,&pszErr);
		if(nResult != SQLITE_OK){
			sqlite3_free(pszErr);
			throw nResult;
		}

		for (int i = nCol ; i <(nRow+1) * nCol && nRow != 0; i++ ){
			cout<<"nResult[i]: "<<pResult[i]<<endl;
		}

		nResult = ALARM_TABLE_OK;
	}
	catch (...){
		nResult = ALARM_TABLE_INTERNAL_ERROR;
	}

	free(pPath);
	sqlite3_close(db);
	return nResult;

}
int GetCountDB()
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

	/*»ñÈ¡exeÎÄ¼þÂ·¾¶*/
	pPath= GetAlarmPath();
	if(pPath == NULL){
		throw ALARM_TABLE_FILEPATH_FAILURE;
	}

	/* create alarm.db if exist not create */
	if (PR_Access(pPath, PR_ACCESS_EXISTS) != PR_SUCCESS) {
		if(nResult != ALARM_TABLE_OK){
			free(pPath);
			return ALARM_FILE_NOEXIST;
		}
	}

	try
	{
		/* ´ò¿ªÊý¾Ý¿â, ´´½¨Á¬½Ó */
		nResult = sqlite3_open(pPath, &db);
		if (nResult != SQLITE_OK) {
			free(pPath);
			return ALARM_TABLE_OPEN_DB_ERROR;
		}

		sqlite3_busy_timeout(db,60*1000);

		memset(szAlarmSqlTemp,0,sizeof(szAlarmSqlTemp));
		PR_snprintf(szAlarmSqlTemp, sizeof(szAlarmSqlTemp),
			"select count(1) from alarminfo ;");
		szAlarmSql=ConvertToUTF(szAlarmSqlTemp);



		nResult = sqlite3_get_table(db,szAlarmSql,&pResult,&nRow,&nCol,&pszErr);
		if(nResult != SQLITE_OK){
			sqlite3_free(pszErr);
			throw nResult;
		}

		for (int i = nCol ; i <(nRow+1) * nCol ; i++ ){
			cout<<"alarminfo count: "<<pResult[i]<<endl;
		}

		nResult = ALARM_TABLE_OK;
	}
	catch (...){
		nResult = ALARM_TABLE_INTERNAL_ERROR;
	}

	free(pPath);
	sqlite3_close(db);
	return nResult;

}
int getopt(int argc, char* const *argv, const char *optstr)
{
    static int optchr = 0;
    static int dash = 0; /* have already seen the - */
/*
	char *optarg;
	int optreset = 0;
	int optind = 1;
	int opterr = 1;
	int optopt;*/
    char *cp;

    if (optreset)
        optreset = optchr = dash = 0;
    if(optind >= argc)
        return(EOF);
    if(!dash && (argv[optind][0] !=  '-'))
        return(EOF);
    if(!dash && (argv[optind][0] ==  '-') && !argv[optind][1])
    {
        /*
         * use to specify stdin. Need to let pgm process this and
         * the following args
         */
        return(EOF);
    }
    if((argv[optind][0] == '-') && (argv[optind][1] == '-'))
    {
        /* -- indicates end of args */
        optind++;
        return(EOF);
    }
    if(!dash)
    {
        assert((argv[optind][0] == '-') && argv[optind][1]);
        dash = 1;
        optchr = 1;
    }

    /* Check if the guy tries to do a -: kind of flag */
    assert(dash);
    if(argv[optind][optchr] == ':')
    {
        dash = 0;
        optind++;
        return(optiserr(argc, argv, optind-1, optstr, optchr, OPTERRCOLON));
    }
    if(!(cp = strchr((char *)optstr, argv[optind][optchr])))
    {
        int errind = optind;
        int errchr = optchr;

        if(!argv[optind][optchr+1])
        {
            dash = 0;
            optind++;
        }
        else
            optchr++;
        return(optiserr(argc, argv, errind, optstr, errchr, OPTERRNF));
    }
    if(cp[1] == ':')
    {
        dash = 0;
        optind++;
        if(optind == argc)
            return(optiserr(argc, argv, optind-1, optstr, optchr, OPTERRARG));
        optarg = argv[optind++];
        return(*cp);
    }
    else
    {
        if(!argv[optind][optchr+1])
        {
            dash = 0;
            optind++;
        }
        else
            optchr++;
        return(*cp);
    }
    assert(0);
    return(0);
}

int GetServerNameIPs(MiAlarmCmd &AlarmCmd)
{
	/*struct hostent *hp;
	char szHostName[256];*/
	char szIPs[256] = {0};
	/*int len;
	int i;
	WSADATA wsaData;
	if(WSAStartup(MAKEWORD(2,2),&wsaData)==SOCKET_ERROR)
	{
		exit(0);
	}
	if (gethostname(szHostName, sizeof(szHostName)) == -1)
		return 1;

	if (NULL == (hp = gethostbyname(szHostName)))
		return 1;

	
	for (i = 0; hp->h_addr_list[i]!= NULL; i++) {
		if(len = strlen(szIPs)>239)
			break;
		in_addr *addr = (in_addr *)(hp->h_addr_list[i]);
		PR_snprintf(szIPs + strlen(szIPs), sizeof(szIPs) - strlen(szIPs), "%s;", 
			inet_ntoa(*addr));

	}*/
	PR_snprintf(szIPs,256, "%s;", 
		"192.168.0.1");
	AlarmCmd.ip = strdup(szIPs);
	return 0;
}

/* ³õÊ¼»¯ÈÕÖ¾ÎÄ¼þ*/
/*
int InitLog(int argc,char **argv)
{
	int nResult              = ALARM_TABLE_OK;
	char cmdPara[2048]       = {0};
	char logfile[512]        = {0};
	int i = 1;
	char *szDir = GetExeDir();
#ifndef AIX
	//-- start add printlog
	memset(logfile, 0, sizeof(logfile));
	PR_snprintf(logfile, sizeof(logfile), "%s%s", szDir, "/logs/pullsched.log");
	nResult = SLogInit(logfile, 6);
	if (0 != nResult) {
		fprintf(stderr, "Log (%s) initialized failed\n", logfile);
		//return -1;
	}
	//--end add printlog

	SLogErrorWrite(APLOG_MARK, APLOG_INFO, NULL, "pullsched.exe start!");

	//--start print cmd
	memset(cmdPara, 0, sizeof(cmdPara));
	for (i=1; i<argc; ++i){
		strcat(cmdPara, argv[i]);
		strcat(cmdPara, " ");
	}
	SLogErrorWrite(APLOG_MARK, APLOG_INFO, NULL, "cmd parameter[%s%c%s]", argv[0], ' ', cmdPara);
	//--end print cmd
#endif
	return 0;
}
*/

/*
int CopyCmdLine(int argc,char **argv,MiAlarmCmd &AlarmCmd)
{

	char *pszOptStr = "M:P:S:W:R:";
	int option				 = -1;
	for (;;) {   
		option = (char)getopt(argc, argv, pszOptStr);
		if ((char)EOF == option) {
			break;
		}

		switch (option) {
		case 'M':
			AlarmCmd.type  = atoi(optarg);
			break;
		case 'P':
			AlarmCmd.errorcode  = atoi(optarg);
			break;
		case 'S':
			AlarmCmd.IsSolved  = atoi(optarg);
			break;
		case 'W':
			AlarmCmd.moudle  = strdup(optarg);
			break;
		case 'R':
			AlarmCmd.operatortype = atoi(optarg);
			break;   
		default:
			cout<<"error"<<endl;
		}
	}
	return 0;
}
*/

void starttimecounter()
{
	double Freq = 0.0;
	PRInt64 CounterStart = 0;
	PRInt64 CounterEnd = 0;
	LARGE_INTEGER li ;
	if(!QueryPerformanceFrequency(&li))
		cout << "QueryPerformanceFrequency failed!\n";
	Freq = double(li.QuadPart)/1000;
	QueryPerformanceCounter(&li);
	CounterStart = li.QuadPart;
}

