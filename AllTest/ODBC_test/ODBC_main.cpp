#include <windows.h>
#include <iostream>
#include "sqlcommhdr.h"
#include "nspr.h"
using namespace std;

int ODBCExec();
int ODBCExec2();
void main()
{
    int nRC = 0;
	for(int i = 0;i<1;i++){
		//int nRC = ODBCExec();
        nRC = ODBCExec2();
		cout<<"nRC: "<<nRC<<endl;
	}
	
	system("pause");
}

int ODBCExec()
{
	char         szSQL[1024]        = { 0 };
	char         szSQL1[1024]       = { 0 };
	char         szSQL2[1024]       = { 0 };
	char         szSQL3[1024]       = { 0 };
	char         szSQL4[1024]       = { 0 };
	char         szSQL5[1024]       = { 0 };
	SQLHSTMT 	 hstmt              = SQL_NULL_HSTMT;
	SQLINTEGER 	 NativeError        = 0;
	int          nResult            = 0;
	int          state              = 0;
	CSqlConnect  SqlConn;
	SQLRETURN   rc;  
	SQLCHAR     Sqlstate[6] = { 0 };
	SQLCHAR     MessageText[1024] = { 0 };
	SQLSMALLINT TextLength;
	char *pszDBname = "db10000";
	SQLINTEGER lrows = 0;
	long lcols = 0;

	/*nResult = SqlConn.SetEnv(szDBname,"2.2.2.40", 
		1433, "sa", "Root@123");*/
    nResult = SqlConn.SetEnv(pszDBname,"192.168.6.221", 
		42009, "sa", "Root@123");

	if (nResult != 0) {
		cout<<"setenv error"<<endl;
		system("pause");
		return -1;
	}

	nResult = SqlConn.Connect();
	if (nResult != 0) {
		cout<<"connect failed\n";
		return -1;
	}

/*
	PR_snprintf(szSQL, sizeof(szSQL),
		"select state from sys.databases "
		"where name = '%s'",
		szDBname);*/
	//PR_snprintf(szSQL,sizeof(szSQL),"RESTORE FILELISTONLY FROM DISK = 'D:/WaveTop/Backup6/server/MssqlExp/db1003_1551255595#'");
    /*_snprintf(szSQL, sizeof(szSQL), "USE [master] \n \
                                    EXEC('ALTER DATABASE [%s] SET SINGLE_USER WITH ROLLBACK IMMEDIATE') \n \
                                    EXEC('ALTER DATABASE [%s] SET SINGLE_USER') \n \
                                    EXEC('ALTER DATABASE [%s] SET RECOVERY FULL WITH NO_WAIT') \n \
                                    EXEC('ALTER DATABASE [%s] SET RECOVERY FULL')",
                                    pszDBname, pszDBname, pszDBname, pszDBname);*/
    /*_snprintf(szSQL, sizeof(szSQL), 
        "EXEC('ALTER DATABASE [%s] SET MULTI_USER')", pszDBname);*/
    _snprintf(szSQL, sizeof(szSQL), 
        "SELECT RTRIM(o.name),RTRIM(o.filename) FROM [DB20091001].dbo.sysfiles o WHERE (o.status & 0x40) <> 0");

	hstmt = SqlConn.Allochstmt();
	if (hstmt == SQL_NULL_HSTMT) {
		cout<<"Allochstmt failed"<<endl;
		return -1;
	} 

	rc = SQLExecDirect(hstmt, (SQLCHAR*)szSQL, SQL_NTS);
		if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO) {
			SqlConn.GetODBCErr(WAVETOP_SQL_HANDLE_HSTMT, hstmt, Sqlstate, &NativeError,
				MessageText, sizeof(MessageText), &TextLength);
			cout<<"SQLExecDirect failed. error： "<<MessageText<<endl;
			SqlConn.FreeHstmt(hstmt);
			return -1;
		}


	SQLRowCount(hstmt,&lrows);
	if(lrows < 2){
		//return -1;
	}



	SQLBindCol(hstmt, 1, SQL_C_CHAR, szSQL1, sizeof(szSQL1), &NativeError);
	SQLBindCol(hstmt, 3, SQL_C_CHAR, szSQL2, sizeof(szSQL2), &NativeError);


	if (SQLFetch(hstmt) != SQL_SUCCESS) {
		SqlConn.FreeHstmt(hstmt);
		cout<<"SQLFetch failed";
		SqlConn.Close();
		return -1;
	}

	if(!strcmp(szSQL2,"D")){
		cout<<"szSQL1 为 D"<<endl;
	}

	if(SQLFetch(hstmt) != SQL_SUCCESS){
		SqlConn.FreeHstmt(hstmt);
		cout<<"SQLFetch failed";
		SqlConn.Close();
		return -1;
	}

	if(!strcmp(szSQL2,"L")){
		cout<<"szSQL1 为 L"<<endl;
	}

	SqlConn.FreeHstmt(hstmt);
	SqlConn.Close();


	cout<<state<<endl;

	return 0;
}

int ODBCExec2()
{
	char         szSQL[1024]        = { 0 };
	char         szSQL1[1024]       = { 0 };
	char         szSQL2[1024]       = { 0 };
	char         szSQL3[1024]       = { 0 };
	char         szSQL4[1024]       = { 0 };
	char         szSQL5[1024]       = { 0 };
	SQLHSTMT 	 hstmt              = SQL_NULL_HSTMT;
	SQLINTEGER 	 NativeError        = 0;
	int          nResult            = 0;
	int          state              = 0;
	CSqlConnect  SqlConn;
	SQLRETURN   rc;  
	SQLCHAR     Sqlstate[6] = { 0 };
	SQLCHAR     MessageText[1024] = { 0 };
	SQLSMALLINT TextLength;
	char *pszDBname = "db10000";
	SQLINTEGER lrows = 0;
	long lcols = 0;

	/*nResult = SqlConn.SetEnv(szDBname,"2.2.2.40", 
		1433, "sa", "Root@123");*/
    nResult = SqlConn.SetEnv(pszDBname,"2.2.2.40", 
		1433, "sa", "Root@123");

	if (nResult != 0) {
		cout<<"setenv error"<<endl;
		system("pause");
		return -1;
	}

	nResult = SqlConn.Connect();
	if (nResult != 0) {
		cout<<"connect failed\n";
		return -1;
	}

/*
	PR_snprintf(szSQL, sizeof(szSQL),
		"select state from sys.databases "
		"where name = '%s'",
		szDBname);*/
	//PR_snprintf(szSQL,sizeof(szSQL),"RESTORE FILELISTONLY FROM DISK = 'D:/WaveTop/Backup6/server/MssqlExp/db1003_1551255595#'");
    /*_snprintf(szSQL, sizeof(szSQL), "USE [master] \n \
                                    EXEC('ALTER DATABASE [%s] SET SINGLE_USER WITH ROLLBACK IMMEDIATE') \n \
                                    EXEC('ALTER DATABASE [%s] SET SINGLE_USER') \n \
                                    EXEC('ALTER DATABASE [%s] SET RECOVERY FULL WITH NO_WAIT') \n \
                                    EXEC('ALTER DATABASE [%s] SET RECOVERY FULL')",
                                    pszDBname, pszDBname, pszDBname, pszDBname);*/
    /*_snprintf(szSQL, sizeof(szSQL), 
        "EXEC('ALTER DATABASE [%s] SET MULTI_USER')", pszDBname);*/
    _snprintf(szSQL, sizeof(szSQL), 
        "backup log db1001 to disk = 'C:\\bak\\db1001_log.bak'");

	hstmt = SqlConn.Allochstmt();
	if (hstmt == SQL_NULL_HSTMT) {
		cout<<"Allochstmt failed"<<endl;
		return -1;
	} 

	rc = SQLExecDirect(hstmt, (SQLCHAR*)szSQL, SQL_NTS);
		if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO) {
			SqlConn.GetODBCErr(WAVETOP_SQL_HANDLE_HSTMT, hstmt, Sqlstate, &NativeError,
				MessageText, sizeof(MessageText), &TextLength);
			cout<<"SQLExecDirect failed. error： "<<MessageText<<endl;
			SqlConn.FreeHstmt(hstmt);
			return -1;
		}


	SQLRowCount(hstmt,&lrows);
	if(lrows < 2){
		//return -1;
	}



	SQLBindCol(hstmt, 1, SQL_C_CHAR, szSQL1, sizeof(szSQL1), &NativeError);
	SQLBindCol(hstmt, 3, SQL_C_CHAR, szSQL2, sizeof(szSQL2), &NativeError);


	if (SQLFetch(hstmt) != SQL_SUCCESS) {
		SqlConn.FreeHstmt(hstmt);
		cout<<"SQLFetch failed";
		SqlConn.Close();
		return -1;
	}

	if(!strcmp(szSQL2,"D")){
		cout<<"szSQL1 为 D"<<endl;
	}

	if(SQLFetch(hstmt) != SQL_SUCCESS){
		SqlConn.FreeHstmt(hstmt);
		cout<<"SQLFetch failed";
		SqlConn.Close();
		return -1;
	}

	if(!strcmp(szSQL2,"L")){
		cout<<"szSQL1 为 L"<<endl;
	}

	SqlConn.FreeHstmt(hstmt);
	SqlConn.Close();


	cout<<state<<endl;

	return 0;
}