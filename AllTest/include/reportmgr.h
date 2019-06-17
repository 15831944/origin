/** =============================================================================
 ** Copyright (c) 2005 WaveTop Information Corp. All rights reserved.
 **
 ** The Backup system
 **
 ** =============================================================================
 */

#ifndef __BACKUP_REPORT_MANAGER_H_
#define __BACKUP_REPORT_MANAGER_H_ 1

#include "nspr.h"
#include "wmain.h"

/** Report table leave **/
#define WAVETOP_BACKUP_REPT_ERR             1
#define WAVETOP_BACKUP_REPT_INFO            2

#ifdef __cplusplus
extern "C" {
#endif

/** ReptAddJob - add a report into report queue **/
API_EXPORT(int) ReptAddJob(ReportSlotst *pReportSlotst);

/* Get a new report job. Does not release this report. */
API_EXPORT(ReportSlotst *) ReptNewReport(request_rec *pReq);

/* Write the Server Report Table Info */
API_EXPORT(int)     ReptFillReportList(ReportSlotst *pReportSlotst, int nType, 
                           char *pszField, PRInt64 nNum, char *pszString);

/* write the report table into XML file */
API_EXPORT(int)     ReptFlushReport(ReportSlotst *pReportSlotst);

/* Get the task userd time */
API_EXPORT(int)     ReptGetTaskRunTime(ReportSlotst *stReportInfo, 
                           char **pszUsedTime);

/* Get the server filter file and transform to string */
API_EXPORT(int)     ReptGetServerFilter(pool *pool, 
                           char **pszServerFilter);

/* Get the rate of backup files (M/S) */
API_EXPORT(int)     ReptGetRateOfTransfers(ReportSlotst *stReportInfo, 
                           char **pszRate);

/* format time from unsigned long to string */
API_EXPORT(int)     ReptFormatTimeToString(pool *pool, unsigned long nTime, 
                               char **pszString);

/* Set Report End Flag */
API_EXPORT(int)     ReptSetEndFlag(ReportSlotst *pReport, int nTaskState);

/*
 * Add a file status into report list:
 * stReportInfo - the report list.
 * nEndCode - Backuo or Restore a file last exit code.
 * nType - the type of backup or restore.
 */
API_EXPORT(int)     ReptAddFileReportNode(ReportSlotst *stReportInfo, char *pszFile, 
                                int nEndCode, int nType);

#ifdef __cplusplus
}
#endif

PRInt32 ReptWriteSqliteDB(const char *pszTemplatePath, const char *pszSavePath, ReportInfoSt *pInfoSt);

#endif /* __BACKUP_REPORT_MANAGER_H_ */