/** =============================================================================
 ** Copyright (c) 2011 WaveTop Information Corp. All rights reserved.
 **
 ** The A-CDP system
 **
 ** =============================================================================
 */

#if defined(WIN32) || defined(WINDOWS)
#include <windows.h>
#else
#include <errno.h>
#include <sys/mman.h>
#endif
#if defined(LINUX)
#include <unistd.h>
#endif
#include "sqloptimize.h"
#include "libmssqllogfunc.h"

CSqlOptimize::CSqlOptimize()
{
	m_pIoPool            = NULL;
	m_hSrcLog            = NULL;
	m_hTargLog           = NULL;
} 

CSqlOptimize::CSqlOptimize()
{
	UnInit();
}

int CSqlOptimize::UnInit()
{
    if (m_pIoPool != NULL) {
        ap_destroy_pool(m_pIoPool);
        m_pIoPool = NULL;
    }
	
    if (m_hSrcLog != NULL) {
        MiMSSqlReadBufferEnd(m_hSrcLog);
        m_hSrcLog = NULL;
    }
	
    if (m_hTargLog != NULL) {
        MiMSSqlReadBufferEnd(m_hTargLog);
        m_hTargLog = NULL;
    }
    
    return WAVETOP_BACKUP_OK;
}

int CSqlOptimize::Init(char *pIoDataSrcDir, char *pIoDataTargDir, char *pIoDataName)
{
	int nRC = WAVETOP_BACKUP_OK;
	
    m_pIoPool = ap_make_sub_pool(NULL);
    if (m_pIoPool == NULL) {
        SLogErrorWrite(APLOG_MARK, APLOG_ERR | APLOG_NOERRNO, NULL,
            "CSqlOptimize::Init(): ap_make_sub_pool() failed");
        nRC = WAVETOP_BACKUP_NO_MEMORY;
        goto FAILED;
    }
      
    if (pIoDataSrcDir  == NULL || 
        pIoDataTargDir == NULL ||
        pIoDataName    == NULL) {
        SLogErrorWrite(APLOG_MARK, APLOG_ERR | APLOG_NOERRNO, NULL,
            "Init(): LogFileDir is NULL.");
        nRC = WAVETOP_BACKUP_INVALID_SYNTAX;
        goto FAILED;
    }
    
    /*  初始化源日志文件句柄 */
    nRC = MiMSSqlLogStartEx(&m_hSrcLog, pIoDataSrcDir, 1, pIoDataName);
    if (nRC != WAVETOP_BACKUP_OK) {
        SLogErrorWrite(APLOG_MARK, APLOG_ERR | APLOG_NOERRNO, NULL,
            "Init(): MiMSSqlLogStartEx() failed.");
        goto FAILED;
    }
    
    /* 初始化目标日志文件句柄 */
    nRC = MiMSSqlLogStartEx(&m_hTargLog, pIoDataTargDir, 2, pIoDataName);
    if (nRC != WAVETOP_BACKUP_OK) {
        SLogErrorWrite(APLOG_MARK, APLOG_ERR | APLOG_NOERRNO, NULL,
            "Init(): MiMSSqlLogStartEx() failed.");
        goto FAILED;
    }
	
	
FAILED:
	
    if (m_pIoPool != NULL && nRC != WAVETOP_BACKUP_OK)
        ap_destroy_pool(m_pIoPool);
    
    return nRC;
}

int CSqlOptimize::IoDataOptimize(PRUint64 nMinNum, PRInt64 nMaxNum, PRInt64 nWriteBegNum)
{
    int nRC = WAVETOP_BACKUP_OK;
    PRUint64 nLastSeqNum           = 0;
    PRInt16  nFileNo               = 0;
    PRInt32  nStrLen               = 0;
    PRInt32  nDataLen              = 0;
    PRInt32  nDataSize             = 0;
    PRInt32  nSize                 = 0;
    PRInt32  nCurSeq               = 0;
    unsigned long nReadBytes       = 0;

}



