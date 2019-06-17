#include <Windows.h>
#include <process.h>
#include <iostream>
#include "nspr.h"
#include "sqlite3.h"

using namespace std;

/* �澯ϵͳ respond code */
#define ALARM_TABLE_OK                         0       // �ɹ�
#define ALARM_TABLE_INTERNAL_ERROR             1
#define ALARM_TABLE_OPEN_DB_ERROR              2       // ���ļ�ʧ��
#define ALARM_TABLE_FILEPATH_FAILURE           4       // ��ȡ�ļ�·��ʧ��
#define ALARM_FILE_NOEXIST					   5       //�ļ�������
#define ALARM_TABLE_EXEC_ERROR                 6       //ִ�����ʧ��
#define ALARM_TABLE_CREATE_ERROR               7		
#define ALARM_TABLE_UPDATE_ERROR               8
#define ALARM_TABLE_DELETE_ERROR               9

#define BACKUP_ALARM_TABLE_FILE                     "alarm.db"

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

int GetActiveAlarm(int *pNum);

int GetActiveAlarm1(int *pNum);

char* ConvertToUTF(char *pszFile);

char* GetAlarmPath();