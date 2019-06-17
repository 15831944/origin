/** =============================================================================
 ** Copyright (c) 2006 WaveTop Information Corp. All rights reserved.
 **
 ** The Mirror system
 **
 ** =============================================================================
 */

#ifndef _LIBMI_LOGFILE_H_
#define _LIBMI_LOGFILE_H_ 1

#include "backup_proto.h"
#include "libmssqllog.h"
#if !defined(WIN32)
#include "sem.h"
#else
#include <time.h>
#endif

#include "nspr.h"

#define WAVETOP_LOGFILE_MAGIC              "WTMLOG"
#define WAVETOP_LOGFILE_VERSTR             "10"

#define WAVETOP_LOGFILE_LIMITED_RECORD     10000
#define WAVETOP_LOGFILE_FLUSH_COUNT        1000
#define WAVETOP_LOGFILE_LIMITED_SIZE       (1<<30)
#define WAVETOP_LOGFILE_LOOP_COUNT         (0xFFFFFFFF)
#define WAVETOP_LOGFILE_MAP_SIZE           4096

#define WAVETOP_LOGFILE_INDEX              "/indx"
#define WAVETOP_LOGFILE_DATA               "/data"
#define WAVETOP_LOGFILE_CONF               "/log"
#define WAVETOP_LOGFILE_FORMAT             "%s%s_%d.%06d"
#define WAVETOP_LOGFILE_SHARE              "WAVETOP_MMShare"
#define WAVETOP_LOGFILE_MUTEX              "WAVETOP_MMShare_MUTEX"

#define WAVETOP_LOGFILE_READWRITE_MUTEX    "{A18B0A27-F16C-4128-9A82-7AD4FD63CB16}"

/* the integer operation
 * XINT - int to char
 * VINT - char to int
 */
#define XINT64(n,p) {\
    (p)[0]=(unsigned char)(((n)>>56)&0xff);\
    (p)[1]=(unsigned char)(((n)>>48)&0xff);\
    (p)[2]=(unsigned char)(((n)>>40)&0xff);\
    (p)[3]=(unsigned char)(((n)>>32)&0xff);\
    (p)[4]=(unsigned char)(((n)>>24)&0xff);\
    (p)[5]=(unsigned char)(((n)>>16)&0xff);\
    (p)[6]=(unsigned char)(((n)>>8)&0xff);\
    (p)[7]=(unsigned char)( (n)&0xff);\
}
#define VINT64(n,p) {\
    unsigned long h; \
    unsigned long l; \
    PRUint64   u; \
    /* high 32-bit */ \
    h = (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3]; \
    /* low 32-bit */ \
    l = (p[4] << 24) + (p[5] << 16) + (p[6] << 8) + p[7]; \
    LL_UI2L(u, h); \
    LL_SHL(n, u, 32); \
    LL_UI2L(u, l); \
    LL_ADD(n, n, u); \
}

#define XINT32(n,p) {\
    (p)[0]=(unsigned char)(((n)>>24)&0xff);\
    (p)[1]=(unsigned char)(((n)>>16)&0xff);\
    (p)[2]=(unsigned char)(((n)>>8)&0xff);\
    (p)[3]=(unsigned char)( (n)&0xff);\
}
#define VINT32(n,p) {\
    n= (((unsigned char)((p)[0])) << 24) + \
       (((unsigned char)((p)[1])) << 16) + \
       (((unsigned char)((p)[2])) << 8 ) + \
       (((unsigned char)((p)[3]))      ) ; \
}

#define XINT16(n,p) {\
    (p)[0]=(unsigned char)(((n)>>8)&0xff);\
    (p)[1]=(unsigned char)( (n)&0xff);\
}
#define VINT16(n,p) {\
    n= (((unsigned char)((p)[0])) << 8) + \
	(((unsigned char)((p)[1]))     ) ; \
}

#ifdef _DEBUG
#define LOG_TRACE1(f, a) printf(f, a)
#define LOG_TRACE(f) printf(f)
#else
#define LOG_TRACE1(f, a)
#define LOG_TRACE(f)
#endif

/** The number of per index file **/
#define WAVETOP_MIRROR_MAX_RECORD                   1000000

/** The size of input buffer **/
#define WAVETOP_MIRROR_INBUFFER_SIZE                (1<<14)

#define WAVETOP_MIRROR_INBUFFER_DATASIZE            (1<<23)     //8M

#ifdef _DEBUG
#define LOG_TRACE1(f, a) printf(f, a)
#define LOG_TRACE(f) printf(f)
//#define LOG_TRACE1(format,a,... ) SLogErrorWrite(APLOG_MARK, APLOG_ERR, NULL, f,a,__VA_ARGS__)
//#define LOG_TRACE(format,...) SLogErrorWrite(APLOG_MARK, APLOG_INFO, NULL, format, __VA_ARGS__)
#else
#define LOG_TRACE1(f, a) 
#define LOG_TRACE(f)
//#define LOG_TRACE1(format,a,... ) SLogErrorWrite(APLOG_MARK, APLOG_ERR, NULL, f,a,__VA_ARGS__)
//#define LOG_TRACE(format,...) SLogErrorWrite(APLOG_MARK, APLOG_INFO, NULL, format, __VA_ARGS__)
#endif

/** 内部物理索引节点 **/
typedef struct INDEX_ST{
   unsigned char szPos[4];
   unsigned char szSeqNum[8];
   unsigned char cType;
   unsigned char szSocket[4];
   unsigned char szSize[4];
   char szIP[16];
   unsigned char szTime[4];
} IndexSt;

/** 索引文件头 **/
typedef struct _LOG_INDEX_HEADER_ST {
    unsigned char szRecordCount[4];
    unsigned char szFileSize[4];
    char szMagic[8];
    char szVersion[2];
    unsigned char szStartSeq[8];
    unsigned char szEndSeq[8];
    char szReserve[16];
    unsigned char szTime[4];
} LogIndexHeaderSt;


/** 
 * 打开文件选项摘自NSPR.
 *
 **************************************************************************
 * FUNCTION:    PR_Open
 * DESCRIPTION:    Open a file for reading, writing, or both.
 * INPUTS:
 *     const char *name
 *         The path name of the file to be opened
 *     PRIntn flags
 *         The file status flags.
 *         It is a bitwise OR of the following bit flags (only one of
 *         the first three flags below may be used):
 *		PR_RDONLY        Open for reading only.
 *		PR_WRONLY        Open for writing only.
 *		PR_RDWR          Open for reading and writing.
 *		PR_CREATE_FILE   If the file does not exist, the file is created
 *                              If the file exists, this flag has no effect.
 *      PR_SYNC          If set, each write will wait for both the file data
 *                              and file status to be physically updated.
 *		PR_APPEND        The file pointer is set to the end of
 *                              the file prior to each write.
 *		PR_TRUNCATE      If the file exists, its length is truncated to 0.
 *      PR_EXCL          With PR_CREATE_FILE, if the file does not exist,
 *                              the file is created. If the file already 
 *                              exists, no action and NULL is returned
 *
 *     PRIntn mode
 *         The access permission bits of the file mode, if the file is
 *         created when PR_CREATE_FILE is on.
 * OUTPUTS:    None
 * RETURNS:    PRFileDesc *
 *     If the file is successfully opened,
 *     returns a pointer to the PRFileDesc
 *     created for the newly opened file.
 *     Returns a NULL pointer if the open
 *     failed.
 * SIDE EFFECTS:
 * RESTRICTIONS:
 * MEMORY:
 *     The return value, if not NULL, points to a dynamically allocated
 *     PRFileDesc object.
 * ALGORITHM:
 **************************************************************************
 */

/* Open flags */
#define WAVETOP_LOGFILE_RDONLY           0x01
#define WAVETOP_LOGFILE_WRONLY           0x02
#define WAVETOP_LOGFILE_RDWR             0x04
#define WAVETOP_LOGFILE_CREATE_FILE      0x08
#define WAVETOP_LOGFILE_APPEND           0x10
#define WAVETOP_LOGFILE_TRUNCATE         0x20
#define WAVETOP_LOGFILE_SYNC             0x40
#define WAVETOP_LOGFILE_EXCL             0x80
#define WAVETOP_LOGFILE_WITHOUT_VALIDATE 0x90

/* Moving flags */
#define WAVETOP_LOGFILE_BEGIN        1
#define WAVETOP_LOGFILE_CURRENT      2
#define WAVETOP_LOGFILE_END          3

/*
** File modes ....
**
** CAVEAT: 'mode' is currently only applicable on UNIX platforms.
** The 'mode' argument may be ignored by PR_Open on other platforms.
**
**   00400   Read by owner.
**   00200   Write by owner.
**   00100   Execute (search if a directory) by owner.
**   00040   Read by group.
**   00020   Write by group.
**   00010   Execute by group.
**   00004   Read by others.
**   00002   Write by others
**   00001   Execute by others.
**
*/

/*
 **************************************************************************
 * Base LogFile class
 **************************************************************************
 */

class CLogFile {
public:
    CLogFile();
    ~CLogFile();

public:
    /** 
     * 打开文件.
     * @[in]
     * pszFileName  - 打开文件名.
     * nOptions     - 打开选项,如上定义. See open flags.
     * @[out]
     * 成功返回WAVETOP_BACKUP_OK；否则返回错误码. 以下方法同.
     **/
    int Open(const char *pszFileName, int nOptions, int nMode);

    /** 读文件 **/
    int Read(char *pszBuf, int nBufSize, unsigned long *pnBytesOfRead);

    /** 读文件 **/
    int ReadNoBuf(char *pszBuf, int nBufSize, unsigned long *pnBytesOfRead);

    /** 写文件 **/
    int Write(char *pszData, int nDataBytes);

    /** 写文件 **/
    int Write();

    /** 位置定位. 只用32位定义位移. See moving flags. **/
    int Seek(int nWay, long nPosition);

    /** 获取文件指针位置 **/
    int GetPosition(unsigned long *pnPos);

    /* 获取大小 */
    int GetSize(unsigned long *pnSize);

    /** 关闭文件 **/
    int Close();

    /** 刷新文件 **/
    bool FlushFile();

    /** 
     * 判断文件指针位置.
     * @[out]
     * 1     - 文件指针在末尾.
     * 0     - 文件指针不在末尾.
     **/
    int IsTail();

    /** 重置缓冲区指针为0 **/
    void ResetInBuf();

    /** 获取文件名称 **/
    const char *GetName();

    int SetEndOfFile(long nPosition);

private:
    /** The file name **/
    char   m_szFileName[1024];

#if defined(WIN32) || defined(WINDOWS)
    /** The file handle **/
    HANDLE m_hFile;
#else
    int    m_hFile;
#endif

    /** Open flags **/
    int    m_nOptions;

    /** The input buffer for reading **/
    char   m_szInBuf[WAVETOP_MIRROR_INBUFFER_SIZE];
    int    m_nInPos;
    int    m_nInSize;

    /** The output buffer for writing **/
    char   m_szOutBuf[WAVETOP_MIRROR_INBUFFER_SIZE];
    int    m_nOutPos;
    int    m_nOutSize;
};

/*
 **************************************************************************
 * IndexLogFile class
 **************************************************************************
 */

class CIndexLogFile : public CLogFile {
public:

    CIndexLogFile();
    ~CIndexLogFile();

public:

    unsigned long GetTailPos();
    PRUint64 GetStartSeq();
    PRUint64 GetEndSeq();
    unsigned long GetRecordCount();
    int GetNo();

    void SetNo(int nNo);
    void SetRecordCount(unsigned long nRecordCont);
    
public:

    /**
     * 重载Open方法，可以读取索引文件头.
     **/
    int Open(const char *pszFileName, int nOptions, int nMode, long *pPos = NULL);


    /** 
     * Create header of this index file.
     * @[out]
     * 成功返回WAVETOP_BACKUP_OK；否则返回错误码. 以下方法同.
     **/
    int CreateHeader(LogIndexHeaderSt &iHeader);

    /** 
     * Append a record.
     * @[out]
     * 成功返回WAVETOP_BACKUP_OK；否则返回错误码. 以下方法同.
     **/
    int AppendRecord(IndexSt &Index);

    /* lsc:备份 */
    int BkAppendRecord(IndexSt &index);

    /** Get header information **/
    int GetHeader(LogIndexHeaderSt &iHeader);

    /** 
     * 查找索引. 并且将文件指针移到该记录处.
     * @[in] 
     * nSeqNum   - 查找条件序号.
     * @[out]
     * 成功返回WAVETOP_BACKUP_OK；否则返回错误码.
     **/
    int Search(PRUint64 nSeqNum, IndexSt &Index);

    /**
     * 顺序读取下一个记录.
     * @[out]
     * Index   - 返回一个索引结构.
     * 成功返回WAVETOP_BACKUP_OK；否则返回错误码.
     */
    int Next(IndexSt &Index);

    /**
     * 顺序读取下一个记录, 不缓冲后续数据.
     * @[out]
     * 成功返回WAVETOP_BACKUP_OK；否则返回错误码.
     */
    int Next2(IndexSt &Index);

    /**
     * 移到上一个记录处，读取记录数据.
     * @[out]
     * Index   - 返回一个索引结构.
     * 成功返回WAVETOP_BACKUP_OK；否则返回错误码.
     */
    int Prev(IndexSt &Index);

    /**
     * 移到上一个记录处，不读取记录数据.
     * @[out]
     * 成功返回WAVETOP_BACKUP_OK；否则返回错误码.
     */
    int Prev();

    /** 指针移动 **/
    int First();
    int Last();

public:
    /**
     * 记录数据太大，一次不能读取完毕，因此需要多次读取。
     * 索引指针不移动，仅移动数据文件指针。
     *
     * 该字段标示还未读取的字节。
     **/
    unsigned long m_nMoreSize;

		bool m_bOldType;
		
    /** 对应的最后一包数据在数据文件中的索引 **/
    unsigned long m_nDataLastPos;

private:
    int WriteHeader(LogIndexHeaderSt &iHeader);

    int WriteSecondHeader(LogIndexHeaderSt &iHeader);

    int GetSecondHeader(LogIndexHeaderSt &iHeader);

private:

    /* The create time */
    unsigned long m_nCreateTime;

    /** The beginning sequeuce number. **/
    PRUint64 m_nStartSeq;

    /** The end sequeuce number. **/
    PRUint64 m_nEndSeq;

    /** The number of records */
    unsigned long m_nRecordCount;

    /** The number of current index **/
    int m_nNo;

    /** The last tail postion **/
    unsigned long m_nLastPos;
};

/*
 **************************************************************************
 * DataLogFile class
 **************************************************************************
 */
class CDataLogFile  : public CLogFile {
public:
    CDataLogFile();
    ~CDataLogFile();

public:
    /**
     * 重载Open方法，可以读取索引文件头.
     **/
    int Open(const char *pszFileName, int nOptions, int nMode, long nPos = -1);

    /** 
     * Write a record.
     * @[out]
     * 成功返回WAVETOP_BACKUP_OK；否则返回错误码. 以下方法同.
     **/
    int WriteRecord(WTBUF *pBuffers, int nBuffCount);

    unsigned long GetTailPos();
    unsigned long GetRecordCount();
    void SetRecordCount(unsigned long nRecordCount);
    void SetTailPos(unsigned long nPos);
    int GetNo();

    void SetNo(int nNo);

    /* 探测一个记录的长度. 不移动文件指针. */
    int  PeekRecordSize(unsigned long *pnRecordSize);

private:
    /** The last tail postion **/
    unsigned long m_nLastPos;

    /** The number of records */
    unsigned long m_nRecordCount;

    /** The number of current index **/
    int m_nNo;

public:
    bool     m_bLoadData;
    char    *m_pszDataBuf;
    unsigned long   m_nDataPos;
    unsigned long   m_nLoadPos;
    unsigned long   m_nLoadEndPos;
};

typedef struct LOG_INFO_ST {
    /* 保存日记文件路径 */
    char szFileDir[1024];

    /* 实例名_数据库名 */
    char szSuff[260];

    /* The index file */
    CIndexLogFile *hIndxFile;

    /* The data file */
    CDataLogFile  *hFile;

    /* The operation ways: 1 - reading. 2 - writing */
    int nOperation;
    
    /* 文件序号 */
    int nCurNum;

    /** 转储限定条件 **/
    unsigned long nLimitRecord;

    /** 转储大小限制 **/
    unsigned long nLimitSize;

    /** 共享内存结构 **/
    unsigned char *pView;

    /** 循环日志。索引文件个数. 当为0时则不循环. **/
    unsigned long nLoopIndexCount;

    /** 句柄状态 **/
    int nStatus;

    /** 初始化时的第一个索引文件编号,避免循环查找 **/
    int nFirstNo;

    /** 强制覆盖, 当循环写时. **/
    int nReplace;

    /** 读取方向 **/
    int nWay;

    /** 删除文件 **/
    int nDelete;

    /** 关闭句柄 **/
    int nClose;

    /** 端口号 **/
    int *pPort;
#if defined(WIN32) || defined(WINDOWS)
    /** 读写互斥量 **/
    HANDLE hMutex;

    HANDLE hThread;
#else
    char szLockFile[1024];
    BackupFileLockSt *hMutex;
#endif

} LogHandleSt;

typedef struct LOG_MMSHARE_NODE_ST {
    bool     bIsUsed;
    int      nWay;
    long     nLastTime;
    /* 文件序号 */
    int      nCurNum;
} LogMMShareNodeSt;


#endif /* _LIBMI_LOGFILE_H_ 1 */

