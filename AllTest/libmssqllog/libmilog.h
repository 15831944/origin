/** =============================================================================
 ** Copyright (c) 2003 WaveTop Information Corp. All rights reserved.
 **
 ** The Mirror system
 **
 ** =============================================================================
 */

/**
 * @desc mirror log dll export functions and types
 * @author lexiongjia
 * @file libmilog.h
 */

#ifndef _LIBMI_LOG_H_ 
#define _LIBMI_LOG_H_ 1

/* log file max record */
#define WT_LOG_MAX_RECORD		5000

/* log file size limited define */
#define WT_LOG_LIMIT_NONE		0
#define WT_LOG_LIMIT_RESERVE	1

/* The client log level */
#define WT_LOG_LEVEL_SYSTEM		__FILE__,__LINE__, (1<<0)	// ϵͳ�������رա����á�ϵͳ���д���
#define WT_LOG_LEVEL_ERROR		__FILE__,__LINE__, (1<<1)	// ������־
#define WT_LOG_LEVEL_CONFIG		__FILE__,__LINE__, (1<<2)	// ������־
#define WT_LOG_LEVEL_BACKUP     __FILE__,__LINE__, (1<<3)  // ����
#define WT_LOG_LEVEL_RESTORE    __FILE__,__LINE__, (1<<4)  // ��ԭ 
#define WT_LOG_LEVEL_QUERY      __FILE__,__LINE__, (1<<5)  // ��ѯ
#define WT_LOG_LEVEL_DELETE     __FILE__,__LINE__, (1<<6)  // ɾ��
#define WT_LOG_LEVEL_MODIFY     __FILE__,__LINE__, (1<<7)  // �޸�
#define WT_LOG_LEVEL_LOGIN      __FILE__,__LINE__, (1<<8)  // ��¼
#define WT_LOG_LEVEL_UPDATE		__FILE__,__LINE__, (1<<9)	// �������
#define WT_LOG_LEVEL_SHARE		__FILE__,__LINE__, (1<<10)	// ����
#define WT_LOG_LEVEL_DEBUG		__FILE__,__LINE__, 7	    // ���Լ���

#define WT_LOG_LEVEL_SYSTEM_NEW		(1<<0)	// ϵͳ�������رա����á�ϵͳ���д���
#define WT_LOG_LEVEL_ERROR_NEW		(1<<1)	// ������־
#define WT_LOG_LEVEL_CONFIG_NEW		(1<<2)	// ������־
#define WT_LOG_LEVEL_BACKUP_NEW     (1<<3)  // ����
#define WT_LOG_LEVEL_RESTORE_NEW    (1<<4)  // ��ԭ 
#define WT_LOG_LEVEL_QUERY_NEW      (1<<5)  // ��ѯ
#define WT_LOG_LEVEL_DELETE_NEW     (1<<6)  // ɾ��
#define WT_LOG_LEVEL_MODIFY_NEW     (1<<7)  // �޸�
#define WT_LOG_LEVEL_LOGIN_NEW      (1<<8)  // ��¼
#define WT_LOG_LEVEL_UPDATE_NEW		(1<<9)	// �������
#define WT_LOG_LEVEL_SHARE_NEW		(1<<10)	// ����
#define WT_LOG_LEVEL_DEBUG_NEW		7	    // ���Լ���

/* Backup executing ways */
#define WT_LOG_EXECUTE_HAND     (1<<0)
#define WT_LOG_EXECUTE_AUTO     (1<<1)
#define WT_LOG_EXECUTE_SCHEDULE (1<<2)
#define WT_LOG_EXECUTE_EXPLORER (1<<3)

#define WT_LOG_EXECUTE_MSSQL    (1<<10)
#define WT_LOG_EXECUTE_ORACLE   (1<<11)
#define WT_LOG_EXECUTE_MYSQL    (1<<12)
#define WT_LOG_EXECUTE_GBASE    (1<<13)


/* The client result for query */
typedef struct ClientLogResult_ST {
	int level;
	char server[128];
	char descript[1024];
	char date[32];
} CLIENTLOG;

#ifdef __cplusplus
extern "C" {
#endif /* defined(__cplusplus) */

/**
 * ��־��ʼ��������
 * @[in]
 * file     - ��־�ļ�����
 * reserve  - ����־���������޶�ʱ����ת����1Ϊת����ʽ��
 * level    - ��־���ˡ��μ����϶��塣
 * options  - Ԥ��������
 **/
API_EXPORT(int) ClientLogInit(char *file, int reserve, 
                              int level, int options);

API_EXPORT(int) ClientLogWrite(int level, char *server, 
                               char *descript);

API_EXPORT(int) ClientLogWrite2(const char *file, int line, int level, char *server, 
                                char *fmt, ...);

API_EXPORT(ClientLogResult_ST **) ClientLogQuery(int level, int exec, 
                                                 char *server, PRTime starttime, 
                                                 PRTime endtime, char *descript, 
                                                 int limit);

API_EXPORT(void) ClientLogFree(ClientLogResult_ST **result);

API_EXPORT(int) ClientLogClean(PRTime endtime);
API_EXPORT(int) ClientLogClean2(const char *logfile);

API_EXPORT(int) ClientLogClose();

API_EXPORT(void) ClientSetLogLimitRows(long nLimitRows);
API_EXPORT(void) ClientSetModuleName(char *pszModName);


/* Extending Write log (include the backup executing way) */
API_EXPORT(int) ClientLogWriteEx(const char *file, int line, int level,  int exec, 
                                 char *server, char *descript);

API_EXPORT(int) ClientLogWrite2Ex(const char *file, int line, int level, int exec, 
                                  char *server, char *fmt, ...);

API_EXPORT(int) ClientLogWrite2ExSimple(const char *file, int line, int level, int exec, 
                                  char *server, char *fmt, ...);

#ifdef __cplusplus
}
#endif /* defined(__cplusplus) */

#endif /* !defined(_LIBMI_LOG_H_) */
