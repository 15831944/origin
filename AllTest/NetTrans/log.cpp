/** =============================================================================
 ** Copyright (c) 2003 WaveTop Information Corp. All rights reserved.
 **
 ** The backup system
 **
 ** =============================================================================
 */

/** 
 * File:        backupcore_s/log.cpp
 * Description: For the server log system
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#ifdef WIN32
#include <windows.h>
#include <process.h>
#include <io.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#ifndef AIX
#include <sys/syscall.h>
#endif
#include <sys/time.h>
#include <pthread.h>
#endif

#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>

#include "nspr.h"
#include "wconfig.h"
#include "backup_proto.h"

#ifndef WIN32
#include "configfile.h"
#endif

#include "server_log.h"

#ifndef LF
#define LF 10
#endif
#ifndef CR
#define CR 13
#endif

#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#ifdef WIN32
#define PATHMARK  '\\'
#else
#define PATHMARK  '/'
#endif

#define LOG_LEVE_ENV     "LOG_LEVEL"

#define localtime_r( _clock, _result ) \
    ( *(_result) = *localtime( (_clock) ), \
    (_result) )

#define ap_isspace(c) (isspace(((unsigned char)(c))))

typedef struct {
    char    *t_name;
    int t_val;
} TRANS;

#ifdef HAVE_SYSLOG

static const TRANS facilities[] = {
    {"auth",    LOG_AUTH},
#ifdef LOG_AUTHPRIV
    {"authpriv",LOG_AUTHPRIV},
#endif
#ifdef LOG_CRON
    {"cron",    LOG_CRON},
#endif
#ifdef LOG_DAEMON
    {"daemon",  LOG_DAEMON},
#endif
#ifdef LOG_FTP
    {"ftp", LOG_FTP},
#endif
#ifdef LOG_KERN
    {"kern",    LOG_KERN},
#endif
#ifdef LOG_LPR
    {"lpr", LOG_LPR},
#endif
#ifdef LOG_MAIL
    {"mail",    LOG_MAIL},
#endif
#ifdef LOG_NEWS
    {"news",    LOG_NEWS},
#endif
#ifdef LOG_SYSLOG
    {"syslog",  LOG_SYSLOG},
#endif
#ifdef LOG_USER
    {"user",    LOG_USER},
#endif
#ifdef LOG_UUCP
    {"uucp",    LOG_UUCP},
#endif
#ifdef LOG_LOCAL0
    {"local0",  LOG_LOCAL0},
#endif
#ifdef LOG_LOCAL1
    {"local1",  LOG_LOCAL1},
#endif
#ifdef LOG_LOCAL2
    {"local2",  LOG_LOCAL2},
#endif
#ifdef LOG_LOCAL3
    {"local3",  LOG_LOCAL3},
#endif
#ifdef LOG_LOCAL4
    {"local4",  LOG_LOCAL4},
#endif
#ifdef LOG_LOCAL5
    {"local5",  LOG_LOCAL5},
#endif
#ifdef LOG_LOCAL6
    {"local6",  LOG_LOCAL6},
#endif
#ifdef LOG_LOCAL7
    {"local7",  LOG_LOCAL7},
#endif
    {NULL,      -1},
};
#endif

static const TRANS priorities[] = {
    {"emerg",   APLOG_EMERG},
    {"alert",   APLOG_ALERT},
    {"crit",    APLOG_CRIT},
    {"error",   APLOG_ERR},
    {"warn",    APLOG_WARNING},
    {"notice",  APLOG_NOTICE},
    {"info",    APLOG_INFO},
    {"debug",   APLOG_DEBUG},
    {NULL,  -1},
};

/* error log */
static int      DumpLog();
static int      CopyLog();
static FILE    *error_log = NULL;
static int      loglevel = 0;
static int      logdump = 0;
static char     logfile[APLOG_MAX_NAME];
static int      log_cur_rows = 0;
static PRLock  *loglock = NULL;
static int      log_dump_rows = 0;
static PRStatus	nFlagDelLog = PR_FAILURE;

#if 0
static int SpawnLogChild(void *cmd, BackupChildInfoSt *pinfo)
{
    /* Child process code for 'ErrorLog "|..."';
     * may want a common framework for this, since I expect it will
     * be common for other foo-loggers to want this sort of thing...
     */
    int child_pid = 0;
#if defined(WIN32)
    char *shellcmd;
#endif

#ifdef SIGHUP
    /* No concept of a child process on Win32 */
    signal(SIGHUP, SIG_IGN);
#endif /* ndef SIGHUP */
#if defined(NETWARE)
    child_pid = spawnlp(P_NOWAIT, SHELL_PATH, (char *)cmd);
    return (child_pid);
#elif defined(WIN32)
    shellcmd = getenv("COMSPEC");
    if (!shellcmd)
        shellcmd = SHELL_PATH;
    child_pid = spawnl(_P_NOWAIT, shellcmd, shellcmd, "/c", (char *)cmd, NULL);
    return (child_pid);
#elif defined(OS2)
    /* For OS/2 we need to use a '/' and spawn the child rather than exec as
     * we haven't forked */
    child_pid = spawnl(P_NOWAIT, SHELL_PATH, SHELL_PATH, "/c", (char *)cmd, NULL);
    return (child_pid);
#else    
    execl(SHELL_PATH, SHELL_PATH, "-c", (char *)cmd, NULL);
#endif    
    exit(1);
    /* NOT REACHED */
    return (child_pid);
}

static int open_error_log(const char *error_fname)
{
    char *fname;

    if (*error_fname == '|') {
        logchildpid = BackupSpawnChild(SpawnLogChild, 
            (void *)(error_fname + 1), &error_log, NULL, NULL);
        if (logchildpid == 0) {
            perror("ap_spawn_child");
            fprintf(stderr, "Couldn't fork child for ErrorLog process\n");
            return 1;
        }
        logdump = 1;
    }

#ifdef HAVE_SYSLOG
    else if (!strncasecmp(error_fname, "syslog", 6)) {
        if ((fname = strchr(error_fname, ':'))) {
            const TRANS *fac;

            fname++;
            for (fac = facilities; fac->t_name; fac++) {
                if (!strcasecmp(fname, fac->t_name)) {
                    openlog(ap_server_argv0, LOG_NDELAY|LOG_CONS|LOG_PID,
                        fac->t_val);
                    error_log = NULL;
                    return 0;
                }
            }
        }
        else
            openlog(ap_server_argv0, LOG_NDELAY|LOG_CONS|LOG_PID, LOG_LOCAL7);

        error_log = NULL;
    }
#endif
    else {
        fname = (char *)error_fname;
        if (!(error_log = fopen(fname, "a"))) {
            perror("fopen");
            fprintf(stderr, "could not open error log file %s.\n", fname);
            return 1;
        }
    }
    return 0;
}
#endif

static int GetTid()
{
#ifdef WIN32
    DWORD tid = GetCurrentThreadId();
#elif AIX
    pthread_t tid = pthread_self();
#else
    pid_t tid = syscall(SYS_gettid);
#endif

    return tid;
}

#ifdef WIN32

bool bNotGetFrenquecy=true;
LARGE_INTEGER frenquecy;

static void gettimeofday(struct timeval *pt,void *ptr)
{
    LARGE_INTEGER perfCount;
    if (bNotGetFrenquecy)
    {
        QueryPerformanceFrequency(&frenquecy);
        bNotGetFrenquecy = false;
    }
    QueryPerformanceCounter(&perfCount);
    if (frenquecy.QuadPart>0)
    {
        double dd = (double)perfCount.QuadPart/frenquecy.QuadPart;
        pt->tv_sec = (long)dd;
        pt->tv_usec = ((long)((dd-pt->tv_sec)*1000000.))%1000000;
    }
    else
    {
        pt->tv_sec = perfCount.QuadPart/1000000;
        pt->tv_usec = perfCount.QuadPart%1000000;
    }	
}
#endif

#if defined (WIN32)
#else
PRInt32 HandleWriteHandle(PRFileDesc *pReadDesc,
    PRFileDesc *pWriteDesc,
    PRInt64 nFileSize)
{
    const PRInt32 nBlockSize = 8192;
    PRInt32 nWrite;
    PRInt32 nRead;
    PRInt64 nTotalRead;
    PRInt64 nTotalWrite;
    PRInt64 nTotalSize;
    char szIoBuf[nBlockSize];
    char *pszBuf;
    PRInt32 nRC = 0;

    nTotalRead = 0;
    nTotalWrite = 0;
    nTotalSize = nFileSize;
    pszBuf = (char *)szIoBuf;

    for (;;) {
        if (nTotalSize >= nBlockSize)
            nRead = PR_Read(pReadDesc, pszBuf, nBlockSize);
        else
            nRead = PR_Read(pReadDesc, pszBuf, (PRInt32)nTotalSize);
        if (nRead == -1) {
            nRC = -1;
            break;
        }

        nTotalRead += nRead;
        nTotalSize -= nRead;

        for (;;) {
            nWrite = PR_Write(pWriteDesc, pszBuf, nRead);
            if (nWrite == -1) {
                nRC = -1;
                break;
            }

            nTotalWrite += nWrite;

            if (nRead == nWrite) {
                nRC = 0;
                break;
            }
            else {
                nRead -= nWrite;
            }
        }

        if (   (nRC != 0)
            || (nRead == 0))
            break;


        if (   LL_EQ(nFileSize, nTotalRead)
            || LL_EQ(nFileSize, nTotalWrite)) {
                break;
        }
    }

    return nRC;
}

PRInt32 copy_file(char *pszSourceFile, char *pszDestinFile)
{
    PRFileDesc *pReadFileDesc   = NULL;
    PRFileDesc *pWriteFileDesc  = NULL;
    PRFileInfo64 sFileInfo;
    PRStatus nStatus;
    PRInt32 nRC  = 0;

    if (pszSourceFile == NULL || pszDestinFile == NULL) {
        return -1;
    }

    /* The Source file name */
    nStatus = PR_GetFileInfo64(pszSourceFile, &sFileInfo);
    if (nStatus != PR_SUCCESS) {
        nRC = nStatus;
        goto END;
    }

    /* The Destin file */
    pReadFileDesc = PR_Open(pszSourceFile, PR_RDONLY, 0664);
    if (NULL == pReadFileDesc) {
        nRC = -1;
        goto END;
    }

    /* Delete File */
    pWriteFileDesc = PR_Open(pszDestinFile, PR_CREATE_FILE|PR_RDWR, 0664);
    if (NULL == pWriteFileDesc) {
        nRC = -1;
        goto END;
    }

    nRC = HandleWriteHandle(pReadFileDesc, pWriteFileDesc, sFileInfo.size);
    if (nRC != 0) {
        goto END;
    }

END:
    if (pReadFileDesc != NULL)
        PR_Close(pReadFileDesc);

    if (pWriteFileDesc != NULL)
        PR_Close(pWriteFileDesc);

    return nRC;
}

#endif

static int ap_getch(FILE *f)
{
    return (fgetc(f));
}

static void *ap_getstr(void *buf, size_t bufsiz, FILE *f)
{
    return (fgets((char *)buf, bufsiz, f));
}

static int BackupCfgGetline(char *buf, size_t bufsize, 
				     int opt, FILE *file)
{
    /* If a "get string" function is defined, use it */
    if (buf != NULL) {
        char *src, *dst;
        char *cp;
        char *cbuf = buf;
        size_t cbufsize = bufsize;

        while (1) {
            if (ap_getstr(cbuf, cbufsize, file) == NULL)
                return 1;

            /*
             *  check for line continuation,
             *  i.e. match [^\\]\\[\r]\n only
             */
            cp = cbuf;
            while (cp < cbuf+cbufsize && *cp != '\0')
                cp++;
            if (cp > cbuf && cp[-1] == LF) {
                cp--;
                if (cp > cbuf && cp[-1] == CR)
                    cp--;
                if (cp > cbuf && cp[-1] == '\\') {
                    if (!opt) {
                        cp--;
                        if (!(cp > cbuf && cp[-1] == '\\')) {
                            /*
                             * line continuation requested -
                             * then remove backslash and continue
                             */
                            cbufsize -= (cp-cbuf);
                            cbuf = cp;
                            continue;
                        }
                        else {
                            /* 
                             * no real continuation because escaped -
                             * then just remove escape character
                             */
                            for ( ; cp < cbuf+cbufsize && *cp != '\0'; cp++)
                                cp[0] = cp[1];
                        }   
                    }
                    else {
                        cbufsize -= (cp-cbuf + 1);
                        cbuf = cp + 1;
                        continue;
                    }
                }
            }
            break;
        }

        /*
         * Leading and trailing white space is eliminated completely
         */
        src = buf;
        while (ap_isspace(*src))
            ++src;
        /* blast trailing whitespace */
        dst = &src[strlen(src)];
        while (--dst >= src && ap_isspace(*dst))
            *dst = '\0';
        /* Zap leading whitespace by shifting */
        if (src != buf)
            for (dst = buf; (*dst++ = *src++) != '\0'; )
                ;
            
            return 0;
    }
    else {
        /* No "get string" function defined; read character by character */
        register int c;
        register size_t i = 0;
        
        buf[0] = '\0';
        /* skip leading whitespace */
        do {
            c = ap_getch(file);
        } while (c == '\t' || c == ' ');
        
        if (c == EOF)
            return 1;
        
        if(bufsize < 2) {
            /* too small, assume caller is crazy */
            return 1;
        }

        while (1) {
            if ((c == '\t') || (c == ' ')) {
                buf[i++] = ' ';
                while ((c == '\t') || (c == ' '))
                    c = ap_getch(file);
            }
            if (c == CR) {
                /* silently ignore CR (_assume_ that a LF follows) */
                c = ap_getch(file);
            }
            if (c == LF) {
                /* increase line number and return on LF */
                /* ++cfp->line_number; */
            }
            if (c == EOF || c == 0x4 || c == LF || i >= (bufsize - 2)) {
            /* 
            *  check for line continuation
                */
                if (i > 0 && buf[i-1] == '\\') {
                    //i--;
                    if (!(i > 0 && buf[i-1] == '\\')) {
                        /* line is continued */
                        c = ap_getch(file);
                        continue;
                    }
                    /* else nothing needs be done because
                    * then the backslash is escaped and
                    * we just strip to a single one
                    */
                }
                /* blast trailing whitespace */
                while (i > 0 && ap_isspace(buf[i - 1]))
                    --i;
                buf[i] = '\0';
                return 0;
            }
            buf[i] = c;
            ++i;
            c = ap_getch(file);
        }
    }
}

static int open_error_log(const char *error_fname)
{
    char *fname;
    FILE *flog;
    char logbuf[8192];
    
    /* statistics the total rows of log file */
    log_cur_rows = 0;
    flog = fopen(error_fname, "r");
    if (flog != NULL) {
        /* Selecte the log rows */
        while (!BackupCfgGetline(logbuf, sizeof(logbuf), 0, flog)) {
            log_cur_rows++;
        }
        fclose(flog);
    }
    
#ifdef HAVE_SYSLOG
    if (!strncasecmp(error_fname, "syslog", 6)) {
        if ((fname = strchr(error_fname, ':'))) {
            const TRANS *fac;
            
            fname++;
            for (fac = facilities; fac->t_name; fac++) {
                if (!strcasecmp(fname, fac->t_name)) {
                    openlog(ap_server_argv0, LOG_NDELAY|LOG_CONS|LOG_PID,
                        fac->t_val);
                    error_log = NULL;
                    return 0;
                }
            }
        }
        else
            openlog(ap_server_argv0, LOG_NDELAY|LOG_CONS|LOG_PID, LOG_LOCAL7);
        
        error_log = NULL;
    }
    else {
#endif
        fname = (char *)error_fname;
        if (!(error_log = fopen(fname, "a"))) {
            perror("fopen");
            fprintf(stderr, "could not open error log file %s.\n", fname);
            return 1;
        }
#ifdef HAVE_SYSLOG
    }
#endif
    
    return 0;
}

/*
 * 遍历文件夹,删除500M大小的日志文件
 * [in]
 * pszLogDir:文件夹路径
 * nMinSize:删除文件总共的大小，默认为500M
 */
static void DelBigLog(char *pszLogDir, PRInt32 nMinSize) { 
    PRStatus           nClose = PR_SUCCESS; 
    PRDir             *pDir = NULL;
    PRDirEntry        *pDirName = NULL;
    PRFileInfo         info; 
    PRUint64           nLogSum = 0;
    char               szFilename[APLOG_MAX_NAME];
    PRStatus           nFlag = PR_SUCCESS;


    if (NULL == pszLogDir)
        return;
    /* 打开文件夹 */
    pDir = PR_OpenDir(pszLogDir);
    if (NULL == pDir) {
        return;
    }

    while (1) {
        pDirName = PR_ReadDir(pDir, PR_SKIP_BOTH);
        if (NULL == pDirName) {
            PR_CloseDir(pDir);
            return;
        }

        strcpy(szFilename, pszLogDir);
        strcat(szFilename, "/");
        strcat(szFilename, pDirName->name);

        if (PR_GetFileInfo(szFilename, &info) == PR_FAILURE)
            continue;

        /* nLogSum 是已删除的文件的总大小 */
        nLogSum += info.size; 
        nClose = PR_Delete(szFilename);
        if (PR_FAILURE == nClose)
            continue;
        /* 如果删除的文件大小超过规定大小，就返回 */
        if (nLogSum >= nMinSize) 
            nFlag = PR_FAILURE;

        if (PR_FAILURE == nFlag) {
            PR_CloseDir(pDir);
            return;
        }
    }
    return;
}

/*
 * 函数功能：按照指定条件删除模块日志文件夹中的日志文件.条件是：创建时间大于7天或文件夹中的文件总大小大于500M
 * [in]:
 * pszLogDir:模块文件夹的路径
 */
static void PR_CALLBACK DelLog(char *pszLogDir) {
    char               szFilename[APLOG_MAX_NAME];
    PRDir             *pDir = NULL;
    PRDirEntry        *pDirName = NULL;
    PRFileInfo         info;    	                  /*log folder size*/
    //PRTime             minus;                         /*logfile exist time*/
    //PRInt64            nFlagTime = 60 * 60 * 24 * 7;  /*clean logfile time,values is 7 days*/
    //PRInt64            nFlagBig = 1024 * 1024 * 1024 * 2;  /*control log folder size less than 512M*/
    PRUint64           nLogSum = 0;
    PRInt64            nMinusSize = 0;


    if (NULL == pszLogDir)
        return;
    /*open logfile folder and get logfile name*/
    pDir = PR_OpenDir(pszLogDir);
    if (NULL == pDir) {
        return;
    }

    while (1) {
        pDirName = PR_ReadDir(pDir, PR_SKIP_BOTH);
        int nLen = 0;
        const char* pPostfix = NULL;
        if (NULL == pDirName) {
            PR_CloseDir(pDir);
            return;
        }
        nLen = strlen(pDirName->name);
        if ((strlen(pszLogDir) + nLen + 1) 
            >= sizeof(szFilename)) {
                PR_CloseDir(pDir);
                return;
        }
#if 0
        /* wbackup.log 最少11个字符 */
        if (nLen < 11)
            continue;
#endif
        /* 取字符串后4位，比较 */
        pPostfix = pDirName->name + (nLen - 4);
        if (strcmp(pPostfix,".log") != 0)
            continue;

        strcpy(szFilename, pszLogDir);
        strcat(szFilename, "/");
        strcat(szFilename, pDirName->name);        

        if (PR_GetFileInfo(szFilename, &info) == PR_FAILURE)
            continue;
        
        /* 遇见文件夹跳过去 */
        if (info.type == PR_FILE_DIRECTORY)
            continue;
        nLogSum += info.size;

        /* 总大小超过2个G就清理一下，保持目录最大2个G */        
        if (nLogSum > LOGDIR_MAX_SIZE) {    
            nMinusSize = nLogSum - LOGDIR_MAX_SIZE;
            DelBigLog(pszLogDir, nMinusSize);
            nLogSum = nLogSum - nMinusSize;
        }
#if 0
        /* 不按时间删除日志 */
        /* PRTime 转换成 second */
        minus = (PR_Now() - info.creationTime);
        /* 删除创建时间过长的日志文件 */
        if (minus/1000000 >= nFlagTime)
            PR_Delete(szFilename);
        /* 日志文件大小超过限制，删除排序靠前的500M的日志文件 */
        else {

        }
#endif
    }
}

/*thread function:
*when nFlagDelLog is PR_SUCCESS,thread get busy.values is PR_FAILURE thread sleep.
*delete files unless the file exist time less than nFlagTime and size less than nFlagBig
*parameter is error_fname,it will be converted to folder pathname
*/
static void DelLogWeekly(void *arg) {
    char    *pszPath                   = NULL;
    char     szLogPath[1024];
    char     szLogFile[1024];
    char     szLogIni[1024];
    char    *pos;
    int      nLogLevel = 0;
    PRFileInfo info;
    int      tmpFileSize = 0;
#ifndef WIN32
    CConfigfile *pLog = NULL;    
#endif

    strncpy(szLogFile, logfile, strlen(logfile) + 1);
    strncpy(szLogPath, logfile, strlen(logfile) + 1);
    if ('\0' == szLogPath[0]) {
        return;
    }

    pos = szLogPath;
    while (*pos++ != '\0') {
        if (*pos ==  '\\')
            *pos = '/';

    }

    pszPath = strrchr(szLogPath, '/');
    if (NULL == pszPath) {
        return;
    }

    *pszPath = 0;

    strncpy(szLogIni, szLogPath, sizeof(szLogIni));
    strcat(szLogIni, "/log.ini");

    while(1) {
        
#ifdef WIN32
        nLogLevel = GetPrivateProfileInt("log", "loglevel", 0, szLogIni);
        if(nLogLevel > 0 && nLogLevel != loglevel) {
            loglevel = nLogLevel;
        }
#else
        if (PR_Access(szLogIni, PR_ACCESS_EXISTS) == PR_SUCCESS ) {
            pLog = new CConfigfile(szLogIni);
            if (pLog != NULL) {
                nLogLevel = pLog->ReadInt("log", "loglevel", 0);
                if(nLogLevel > 0 && nLogLevel != loglevel) {
                    loglevel = nLogLevel;
                }
                delete pLog;
                pLog = NULL;
            }
        }
#endif

        PR_Lock(loglock);
        if (PR_GetFileInfo(szLogFile, &info) == PR_SUCCESS) {
            tmpFileSize = info.size;
            if (tmpFileSize > LOG_MAX_SIZE) {            
                CopyLog();
            }
        }
        PR_Unlock(loglock);

        if (PR_FAILURE == nFlagDelLog) {
            PR_Sleep(PR_SecondsToInterval(10));
            continue;
        }
        DelLog(szLogPath);
        nFlagDelLog = PR_FAILURE;
    }

}

API_EXPORT(int) SLogInit(const char *error_fname, int s_loglevel)
{
    int replace_stderr;
    PRThread *pThread;
    
    nFlagDelLog = PR_FAILURE;

#ifdef OS390
    /*
     * Cause errno2 (reason code) information to be generated whenever
     * strerror(errno) is invoked.
     */
    setenv("_EDC_ADD_ERRNO2", "1", 1);
#endif

    loglevel |= s_loglevel;
    logdump = 0;

    if (open_error_log(error_fname))
        return 1;

    replace_stderr = 0;
#if 0
    if (error_log) {
        /* replace stderr with this new log */
        fflush(stderr);
        if (dup2(fileno(error_log), STDERR_FILENO) == -1) {
            SLogErrorWrite(APLOG_MARK, APLOG_CRIT, NULL,
                "unable to replace stderr with error_log");
            fclose(error_log);
            return 1;
        } 
        else {
            replace_stderr = 0;
        }
    }
#endif

    /* note that stderr may still need to be replaced with something
     * because it points to the old error log, or back to the tty
     * of the submitter.
     */
    if (replace_stderr && freopen("/dev/null", "w", stderr) == NULL) {
        SLogErrorWrite(APLOG_MARK, APLOG_CRIT, NULL,
            "unable to replace stderr with /dev/null");
        fclose(error_log);
        return 1;
    }

    loglock = PR_NewLock();
    if (loglock == NULL) {
        fclose(error_log);
        return 1;
    }
    
    strncpy(logfile, (char *)error_fname, sizeof(logfile));
    
    pThread = PR_CreateThread(PR_USER_THREAD, DelLogWeekly, (void *)error_fname, 
        PR_PRIORITY_NORMAL, PR_GLOBAL_THREAD, PR_JOINABLE_THREAD, 0);
    if(pThread == NULL) {
        return WAVETOP_BACKUP_NO_MEMORY;
    }

    return 0;
}

API_EXPORT(void) SLogSetDump(int dumprows)
{
    logdump = (dumprows == 0 ? 0 : 1);
    log_dump_rows = dumprows;
}

API_EXPORT(int) SLogClose()
{
    if (error_log != NULL)
        fclose(error_log);
    PR_DestroyLock(loglock);
    return WAVETOP_BACKUP_OK;
}

void ap_error_log2stderr()
{
    if (   error_log != NULL
        && fileno(error_log) != STDERR_FILENO)
        dup2(fileno(error_log), STDERR_FILENO);
}

static void log_error_core(const char *file, int line, int level,
               const char *user, const char *ip, const char *fmt, va_list args)
{
    char errstr[8192];
    size_t len;
    //int save_errno = errno;
    char *pszTmp = NULL;
    FILE *logf;
    time_t t;
    //tm *timestr;
    struct      tm tmloc;

    if (error_log) {
        /*
         * If we are doing normal logging, don't log messages that are
         * above the server log level unless it is a startup/shutdown notice
         */
        if (((level & APLOG_LEVELMASK) != APLOG_NOTICE) &&
            ((level & APLOG_LEVELMASK) > loglevel)) {
            if (logdump == 1) {
                //PR_Unlock(loglock);
            }
            return;
        }
        logf = error_log;
    }
    else {
        /*
         * If we are doing stderr logging (startup), don't log messages that are
         * above the default server log level unless it is a startup/shutdown
         * notice
         */
        if (((level & APLOG_LEVELMASK) != APLOG_NOTICE) &&
            ((level & APLOG_LEVELMASK) > DEFAULT_LOGLEVEL)) {
            if (logdump == 1) {
                //PR_Unlock(loglock);
            }
            return;
        }
        logf = stderr;
    }

    if (level & APLOG_INFO)
        level |= APLOG_NOERRNO;

    if (logf) {
        t = time(NULL);
#ifdef WIN32
        //localtime线程不安全
        PR_Lock(loglock);
        memcpy(&tmloc,localtime(&t),sizeof(tmloc));
        PR_Unlock(loglock);
#else
        localtime_r(&t, &tmloc);
#endif

        struct timeval CurrentTime;
        gettimeofday(&CurrentTime, NULL);

        len = PR_snprintf(errstr, sizeof(errstr), "[%d/%02d/%02d %02d:%02d:%02d:%06ld] ", 
            tmloc.tm_year + 1900, tmloc.tm_mon + 1, tmloc.tm_mday, 
            tmloc.tm_hour, tmloc.tm_min, tmloc.tm_sec, CurrentTime.tv_usec);
        /*
        timestr = ctime(&t);
        timestr[strlen(timestr) - 1] = '\0';
        len = PR_snprintf(errstr, sizeof(errstr), "[%s] ", timestr);
        */
    }
    else {
        len = 0;
    }
#if 0
    /* user field */
    len += PR_snprintf(errstr + len, sizeof(errstr) - len,
        "[%s] ", (user == NULL ? "" : user));

    /* client side ip */
    len += PR_snprintf(errstr + len, sizeof(errstr) - len,
        "[%s] ", (ip == NULL ? "" : ip));
#endif
    /* pid */
    len += PR_snprintf(errstr + len, sizeof(errstr) - len,
        "[%d] ", getpid());
    /* tid */
    len += PR_snprintf(errstr + len, sizeof(errstr) - len,
        "[%lu] ", GetTid());
    /* log level */
    len += PR_snprintf(errstr + len, sizeof(errstr) - len,
        "[%s] ", priorities[level & APLOG_LEVELMASK].t_name);

    /**- lexiongjia add for parse log format */
    len += PR_snprintf(errstr + len, sizeof(errstr) - len, "[");

    len += PR_vsnprintf(errstr + len, sizeof(errstr) - len, fmt, args);

    /**- lexiongjia add for parse log format */
    len += PR_snprintf(errstr + len, sizeof(errstr) - len, "] ");

    if (file != NULL && strlen(file) > 0) {
        if ((pszTmp = (char *)strrchr(file, PATHMARK)) != NULL)
        {
            file = pszTmp + 1;
        }
        len += PR_snprintf(errstr + len, sizeof(errstr) - len, "[%s:%d]\n", file, line);
    }
    else {
        len += PR_snprintf(errstr + len, sizeof(errstr) - len, "\n");
    }
    /* NULL if we are logging to syslog */
    if (logf) {
        fputs(errstr, logf);
        //fputc('\n', logf);
        fflush(logf);
    }
#ifdef HAVE_SYSLOG
    else {
        syslog(level & APLOG_LEVELMASK, "%s", errstr);
    }
#endif
}

API_EXPORT(void) SLogErrorWrite2(const char *file, int line, int level,
                                 const char *user, int id, const char *fmt, ...)
{
    va_list args;
    
    va_start(args, fmt);
    log_error_core(file, line, level, user, "", fmt, args);
    va_end(args);
}

API_EXPORT(void) SLogErrorWrite3(const char *file, int line, int level,
    const char *user, const char *fmt, va_list args)
{
   
    log_error_core(file, line, level, user, "", fmt, args);
}

API_EXPORT(void) SLogErrorWrite(const char *file, int line, int level,
                                const char *user, const char *fmt, ...)
{
    va_list args;
    
    va_start(args, fmt);
    log_error_core(file, line, level, user, "", fmt, args);
    va_end(args);
}

static int DumpLog()
{
    PRStatus nResult;
    char szDumpfile[1024];
    char szTime[128];
    char *pszFind;
    time_t now;
    tm *timestr;
    nFlagDelLog = PR_SUCCESS;
    
    strncpy(szDumpfile, logfile, sizeof(szDumpfile));
    pszFind = strrchr(szDumpfile, '.');
    if (pszFind == NULL) {
        return WAVETOP_BACKUP_OPEN_FILE_ERROR;
    }
    *pszFind = 0;
    
    pszFind = (char *)szDumpfile + strlen(szDumpfile);
    now = time(NULL);
    timestr = localtime(&now);
    PR_snprintf(szTime, sizeof(szTime), "%04d%02d%02d%02d%02d", 
        timestr->tm_year + 1900, timestr->tm_mon + 1, timestr->tm_mday, 
        timestr->tm_hour, timestr->tm_min);
    strcat(szDumpfile, szTime);
    if (szDumpfile[strlen(szDumpfile) - 1] == '\n')
        szDumpfile[strlen(szDumpfile) - 1] = '\0';
    /* skip driver ":" */
    for (; *pszFind; pszFind++) {
        if (*pszFind == ':')
            *pszFind = '_';
    }
    strcat(szDumpfile, ".log");
    
    /* close file handle */
    fclose(error_log);
    
    /* rename log file */
    nResult = PR_Rename(logfile, szDumpfile);
    if (nResult == PR_FAILURE) {
#ifdef WIN32
        rename((const char *)logfile, (const char *)szDumpfile);
#endif //WIN32
    }
    /* Open a new log file */
    strcpy(szDumpfile, logfile);
    
    log_cur_rows = 0;
    if (open_error_log(logfile)) {
        PR_Unlock(loglock);
        return WAVETOP_BACKUP_OPEN_FILE_ERROR;
    }
    return WAVETOP_BACKUP_OK;
}

static int CopyLog()
{
    char szDumpfile[1024];
    char szTime[128];
    char *pszFind;
    time_t now;
    tm *timestr;
    PRFileDesc *pFile = NULL;
    PRFileInfo info;

    nFlagDelLog = PR_SUCCESS;

    strncpy(szDumpfile, logfile, sizeof(szDumpfile));
    pszFind = strrchr(szDumpfile, '.');
    if (pszFind == NULL) {
        return WAVETOP_BACKUP_OPEN_FILE_ERROR;
    }
    *pszFind = 0;

    pszFind = (char *)szDumpfile + strlen(szDumpfile);
    now = time(NULL);
    timestr = localtime(&now);
    PR_snprintf(szTime, sizeof(szTime), "%04d%02d%02d%02d%02d", 
        timestr->tm_year + 1900, timestr->tm_mon + 1, timestr->tm_mday, 
        timestr->tm_hour, timestr->tm_min);
    strcat(szDumpfile, szTime);
    if (szDumpfile[strlen(szDumpfile) - 1] == '\n')
        szDumpfile[strlen(szDumpfile) - 1] = '\0';
    /* skip driver ":" */
    for (; *pszFind; pszFind++) {
        if (*pszFind == ':')
            *pszFind = '_';
    }
    strcat(szDumpfile, ".log");

    if (PR_Access(szDumpfile, PR_ACCESS_EXISTS) == PR_SUCCESS) {
        printf("exist:%s.\n", szDumpfile);
        PR_Sleep(PR_SecondsToInterval(10));
        return WAVETOP_BACKUP_OK;
    }

    if (PR_GetFileInfo(logfile, &info) == PR_SUCCESS && info.size < LOG_MAX_SIZE) {
        return WAVETOP_BACKUP_OK;
    }

    printf("S:%s.T:%s.\n", logfile, szDumpfile);
#ifdef WIN32
    if(CopyFile(logfile, szDumpfile, false)) {
        pFile = PR_Open(logfile, PR_TRUNCATE | PR_WRONLY, 0664);
    }
#else 
    if (copy_file(logfile, szDumpfile) == 0) {
        pFile = PR_Open(logfile, PR_TRUNCATE | PR_WRONLY, 0664);
    }
#endif

    if (pFile != NULL) {
        PR_Close(pFile);
        pFile = NULL;
    }
    return WAVETOP_BACKUP_OK;
}

