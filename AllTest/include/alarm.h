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

/* �澯ϵͳ respond code */
#define ALARM_TABLE_OK                         0       // �ɹ�
#define ALARM_TABLE_INTERNAL_ERROR             1
#define ALARM_TABLE_OPEN_DB_ERROR              2       // ���ļ�ʧ��
#define ALARM_TABLE_FILEPATH_FAILURE           4       // ��ȡ�ļ�·��ʧ��
#define ALARM_FILE_NOEXIST					   5       //�ļ�������
#define ALARM_TABLE_EXEC_ERROR                 6       //ִ�����ʧ��
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

/*�澯ϵͳ�ṹ��*/
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

/*��ȡ��ǰexe�ļ�Ŀ¼*/
char* GetExeDir();

/*��ȡ��ǰexe�ļ�·��*/
char* GetExePath();

/* ��ʼ����־�ļ�*/
//int InitLog(int argc,char **argv);

/*���������в���*/
int getopt(int argc, char* const *argv, const char *optstr);

/*��ȡ��ǰ�豸ip*/
int GetServerNameIPs(MiAlarmCmd &AlarmCmd);

/*  д��澯���� */
int AlarmWriteSqliteDB(MiAlarmCmd &alarmsql);

//����1000������
int WriteDB_Multi();

//���뵥������
int WrieteDB_Single();

/* ��ӡδ������*/
int GetActiveAlarm();

/*��ӡ�Ѵ�����*/
int GetHistoryAlarm();

int GetCountDB();





#endif