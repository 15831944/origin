#ifndef ___ALARM_H_
#define ___ALARM_H_ 1

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#if defined(WIN32)
#include <windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#endif /* defined(WIN32) */

//#include "libmilog2.h"
#include "sqlite3.h"
#include "nspr.h"

/* 告警系统 respond code */
#define ALARM_TABLE_OK                         0       // 成功
#define ALARM_TABLE_INTERNAL_ERROR             1
#define ALARM_TABLE_OPEN_DB_ERROR              2       // 打开文件失败
#define ALARM_TABLE_FILEPATH_FAILURE           4       // 获取文件路径失败
#define ALARM_FILE_NOEXIST					   5       //文件不存在
#define ALARM_TABLE_EXEC_ERROR                 6       //执行语句失败
#define ALARM_TABLE_CREATE_ERROR               6		
#define ALARM_TABLE_UPDATE_ERROR               7
#define ALARM_TABLE_DELETE_ERROR               8

#define BACKUP_ALARM_TABLE_FILE                     "alarm.db"
#define AP 1

extern char *optarg;
extern int optreset;
extern int optind;
extern int opterr;
extern int optopt;

/*告警系统结构体*/
typedef struct _ALARM_SQL_INFO_ {
	int         type;             //error type
	int         IsSolved;           //error is solved, 1 is not solved,0 is solved 
	int         errorcode;		
	char		*content;
	int			operatortype;
	char		*starttime;			
	char		*lasttime;
	char		*ip;
	char		*moudle;
}MiAlarmCmd;

/*获取当前exe文件目录*/
char* GetExeDir();

/*获取当前exe文件路径*/
char* GetExePath();

/* 初始化日志文件*/
//int InitLog(int argc,char **argv);

/*解析命令行参数*/
int getopt(int argc, char* const *argv, const char *optstr);

/*获取当前设备ip*/
int GetServerNameIPs(MiAlarmCmd &AlarmCmd);

/*  写入告警内容 */
int AlarmWriteSqliteDB(MiAlarmCmd &alarmsql);

//插入1000条数据
int WriteDB_Multi();

//插入单条数据
int WrieteDB_Single();

/* 打印未处理警告*/
int GetActiveAlarm();

/*打印已处理警告*/
int GetHistoryAlarm();

int GetCountDB();





#endif