/** =============================================================================
 ** Copyright (c) 2005 WaveTop Information Corp. All rights reserved.
 **
 ** The Backup system
 **
 ** =============================================================================
 */

#ifndef __WAVETOP_TASK_MONITOR_H_
#define __WAVETOP_TASK_MONITOR_H_ 1

#include "nspr.h"
#include "wmain.h"

/* default max task record and task time out */
#define WAVETOP_TASK_MONITOR_MAX  128
#define WAVETOP_TASK_TIME_OUT     3600

/* Task node state */
#define WAVETOP_TASK_NODE_FREE       0
#define WAVETOP_TASK_NODE_USED       1

/**
 * @task monitor functions
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize the monitor task */
int     MonTaskInit(pool *pPool);

/* Malloc the monitor task node in memory */
int     MakeMonTaskNode(pool *pPool);

/* Close the Monitor task */
int     MonTaskClose(void);

/* 
 * Get the this monitor task id and madk a task node in memory
 * If the id renturn -1 this monitor task is failed
 */
API_EXPORT(int)     MonTaskGetTaskID(request_rec *pReq, unsigned long *pnId);

/* Monify the task state by id */
API_EXPORT(int)     MonTaskSetTaskEndState(unsigned long nId, int nExitCode);

/* Set Task Data stream size */
API_EXPORT(int)     MonTaskSetTaskDataSize(unsigned long nId, PRInt64 nDataSize);   

/* Set report of this task */
API_EXPORT(int)     MonTaskSetTaskReport(unsigned long nId, ReportSlotst *pReport);

/* get task runing state */
API_EXPORT(int)     MonTaskGetTaskState(unsigned long nId, int *nState);

/* change task state by id*/
API_EXPORT(int)     MonTaskChangeTaskState(unsigned long nId, int nState);

/* change task user name by id*/
API_EXPORT(int)     MonTaskChangeTaskUserName(unsigned long nId, char *pszUser);

/* check task user name */
API_EXPORT(int)     MonTaskCheckUser(unsigned long nTaskId, const char *pszUser);

/* Get all monitor task id in memory by option*/
TaskInfoSt    *MonTaskGetAllTask(pool *pPool, GetTaskOption *pOption);

#ifdef __cplusplus
}
#endif

/** @}  */

#endif  /* __WAVETOP_TASK_MONITOR_H_ */
