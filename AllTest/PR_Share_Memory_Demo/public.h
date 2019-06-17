#ifndef _PUBLIC_H_
#define _PUBLIC_H_



#include <iostream>
#include <windows.h>
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "nspr.h"
using namespace std;


/* 子进程共享内存插槽状态 */
#define WAVETOP_BACKUP6_CHILD_PROC_SLOT_IDLE    0
#define WAVETOP_BACKUP6_CHILD_PROC_SLOT_INIT    1 
#define WAVETOP_BACKUP6_CHILD_PROC_SLOT_BUSY    2 

/* 子进程线程任务运行状态 */
#define WAVETOP_BACKUP6_THREAD_TASK_IDLE   0
#define WAVETOP_BACKUP6_THREAD_TASK_BUSY   2
#define WAVETOP_BACKUP6_THREAD_TASK_END    3 
#define WAVETOP_BACKUP6_THREAD_TASK_BEGIN  1



#define WAVETOP_BACKUP_OK                   0
#define WAVETOP_BACKUP_INTERNAL_ERROR       3
#define WAVETOP_BACKUP_FILENAME_TOO_LONG    5
#define WAVETOP_BACKUP_INVALID_SYNTAX       1
#define BACKUP_OS_TYPE_WINNT                2
#define BACKUP_OS_TYPE_WIN9X                4
#define WAVETOP_BACKUP_CONNECT_DOWN         1002
#define WAVETOP_BACKUP_VERSION_4            1004
#define BACKUP_PROTO_RECOVER          314     /* Recover */	

#define BACKUP_CLIENT_SERVER_CFG         "wbackup.conf"
#define BACKUP_SERVER_PORT               58831
#define WAVETOP_SHARE_MEMORY             "_wavetop_share_d_memory_"
#define WAVETOP_BACKUP6_MAX_PROCESS     62
#define WAVETOP_BACKUP6_MAX_THREAD      256    //支持Sqlserver备份200个数据库






#define ap_isalnum(c) (isalnum(((unsigned char)(c))))
#define ap_isalpha(c) (isalpha(((unsigned char)(c))))
#define ap_iscntrl(c) (iscntrl(((unsigned char)(c))))
#define ap_isdigit(c) (isdigit(((unsigned char)(c))))
#define ap_isgraph(c) (isgraph(((unsigned char)(c))))
#define ap_islower(c) (islower(((unsigned char)(c))))
#define ap_isprint(c) (isprint(((unsigned char)(c))))
#define ap_ispunct(c) (ispunct(((unsigned char)(c))))
#define ap_isspace(c) (isspace(((unsigned char)(c))))
#define ap_isupper(c) (isupper(((unsigned char)(c))))
#define ap_isxdigit(c) (isxdigit(((unsigned char)(c))))
#define ap_tolower(c) (tolower(((unsigned char)(c))))
#define ap_toupper(c) (toupper(((unsigned char)(c))))


#define strcasecmp(s1, s2) stricmp(s1, s2)
#define strncasecmp(s1, s2, n) strnicmp(s1, s2, n)





























#endif