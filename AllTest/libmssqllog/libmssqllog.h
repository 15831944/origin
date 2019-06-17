/** =============================================================================
 ** Copyright (c) 2006 WaveTop Information Corp. All rights reserved.
 **
 ** The Mirror system
 **
 ** =============================================================================
 */

#ifndef _LIBMI_MSSQL_LOG_H_
#define _LIBMI_MSSQL_LOG_H_ 1

#include "backup_proto.h"
#include "nspr.h"
#include "walloc.h"

#define WAVETOP_MIRROR_MSSQL2_LOG_INTERNAL_FAILED   1
#define WAVETOP_MIRROR_MSSQL2_LOG_LINE_END          2
#define WAVETOP_MIRROR_MSSQL2_LOG_MORE_WITH_LINE    3

#define WAVETOP_MIRROR_MSSQL2_LOG_RESTARE           3
#define WAVETOP_MIRROR_MSSQL2_LOG_BROKEN            4
#define WAVETOP_MIRROR_MSSQL2_LOG_SWITCH            5

#define WAVETOP_MIRROR_MSSQL2_LOG_TYPE_TCP          10
#define WAVETOP_MIRROR_MSSQL2_LOG_TYPE_PIPE         11

/* The export API definition */
#ifdef WIN32
#ifndef MILOG_EXPORT
#define MILOG_EXPORT               __declspec(dllexport)
#endif
#ifndef MILOG_EXPORT_
#define MILOG_EXPORT_(__type)      __declspec(dllexport) __type
#endif
#else
#ifndef MILOG_EXPORT
#define MILOG_EXPORT               extern
#endif
#ifndef MILOG_EXPORT_
#define MILOG_EXPORT_(__type)      extern __type
#endif
#endif

typedef void * MiLogHandle;

/** Index node **/
typedef struct INDEX_EXNODE_ST{
    /* 序号 */
    PRUint64 nSeqNum;
    /* 类型 */
    unsigned char cType;
    /* socket */
    unsigned long nSocket;
    /* 数据大小 */
    unsigned long nDataSize;
    /* IP Address */
    char szIP[16];
    /* 记录时间. C语言格式. 秒为单位. 调用者不填写. */
    unsigned long nRecordTime;
} ExIndexSt;

//
// The buffer structure.
//
/*
typedef struct WTBUF {
    unsigned char *pszBuf;
    unsigned long dwBufSize;
    unsigned long dwPos;
} WTBUF, *LPWTBUF;*/

typedef struct WTBUF {
	ap_pool *pPool;
	PRUint32 dwBufSize;
	PRUint32 dwPos;
	PRUint8 *pszBuf;
	struct WTBUF *pNext;
	struct WTBUF *pTail;
} WTBUF, *LPWTBUF;
#ifdef __cplusplus
extern "C" {
#endif /* defined(__cplusplus) */

/**
 * 初始化，使用共享读写文件方式
 * @[in]
 * pszLogPath - 指定的日志文件组存储路径
 * nOperation - 1 - reading. 2 - writing.
 * @[out]
 * pHandle - 返回打开的文件句柄
 * 成功返回WAVETOP_BACKUP_OK
 * 失败返回WAVETOP_BACKUP_INTERNAL_ERROR
 */
MILOG_EXPORT_(int) MiMSSqlLogStart(MiLogHandle *pHandle,
                                   char *pszLogPath,
                                   int nOperation,
                                   int nPort = 0);
    
MILOG_EXPORT_(int) MiMSSqlLogStartEx(MiLogHandle *ppHandle, 
                                     char *pszLogPath, 
                                     int nOperation,
                                     char *pszSuff);

/**
 * 将缓冲区中的数据写入指定日志文件
 * @[in]
 * pHandle - 打开的日志文件的句柄
 * pszBuffer - 待写入日志文件的数据
 * nBuffCount - 待写入日志文件数据的长度
 * nSocket - 待写入日志文件的套接字
 * pszIP - 待写入日志文件的IP地址
 * nType - 写入类型
 * nNum - 指定写入的序号
 * @[out]
 * 成功返回WAVETOP_BACKUP_OK
 * 失败返回WAVETOP_BACKUP_INTERNAL_ERROR
 */
MILOG_EXPORT_(int) MiMSSqlWriteBufferToLog(MiLogHandle pHandle,
                                                  WTBUF *pBuffers,
                                                  unsigned long nBuffCount,
                                                  ExIndexSt *pIndex);

/**
 * 指定从某一序号开始读取数据
 * @[in]
 * Handle - 打开的日志文件的句柄
 * nSeqNum - 起始序号。
 * nType - 如果是1，读取方式为从指定序号至末尾或头部。如果是2，读取方式为仅读取指定序号
 * @[out]
 * 成功返回WAVETOP_BACKUP_OK
 * 失败返回WAVETOP_BACKUP_INTERNAL_ERROR
 */
MILOG_EXPORT_(int) MiMSSqlReadBufferFromLogStart(MiLogHandle hHandle,
                                                        PRUint64 nSeqNum,
                                                        int nType);

/**
 * 循环调用该方法，获取从指定序号开始至末尾的数据
 * @[in]
 * pHandle - 打开的日志文件的句柄
 * pszBuffer - 待写入缓冲区
 * nSize - 待写入缓冲区长度
 * nSocket - 当不为0时读取指定套接字的下一条数据，当为0时忽略
 * @[out]
 * pnReadBytes - 返回已写入数据的长度
 * pIndex      - 返回索引节点.
 * 如果同一序号读取时缓冲区不够大，先将现有的缓冲区写满
 * 并返回WAVETOP_MIRROR_MORE_WITH_LINE，然后再次调用此方法接着读取
 * 该序号读取完成返回WAVETOP_BACKUP_OK
 * 已读取至末尾返回WAVETOP_MIRROR_LINE_END
 * 出现异常返回WAVETOP_BACKUP_INTERNAL_ERROR
 */
MILOG_EXPORT_(int) MiMSSqlReadBufferFromLogNext(MiLogHandle hHandle,
                                                       char *pszBuffer,
                                                       unsigned long nSize,
                                                       unsigned long *pnReadBytes,
                                                       unsigned long nSocket,
                                                       ExIndexSt *pIndex);

/**
 * 改进：一次性加载数据和索引文件到内存，减少Io读操作
 * 然后循环调用该方法，获取从指定序号开始至末尾的数据
 * @[in]
 * pHandle - 打开的日志文件的句柄
 * pszBuffer - 待写入缓冲区
 * nSize - 待写入缓冲区长度
 * nSocket - 当不为0时读取指定套接字的下一条数据，当为0时忽略
 * @[out]
 * pnReadBytes - 返回已写入数据的长度
 * pIndex      - 返回索引节点.
 * nDelFalg    - 0是镜像的方式，1是acdp的方式
 * 如果同一序号读取时缓冲区不够大，先将现有的缓冲区写满
 * 并返回WAVETOP_MIRROR_MORE_WITH_LINE，然后再次调用此方法接着读取
 * 该序号读取完成返回WAVETOP_BACKUP_OK
 * 已读取至末尾返回WAVETOP_MIRROR_LINE_END
 * 出现异常返回WAVETOP_BACKUP_INTERNAL_ERROR
 */
MILOG_EXPORT_(int) MiMSSqlReadBufferFromLogNextEx(MiLogHandle hHandle,
    char *pszBuffer,
    unsigned long nSize,
    unsigned long *pnReadBytes,
    unsigned long nSocket,
    ExIndexSt *pIndex,
    int nDelFalg = 1);


/**
 * 循环调用该方法，获取从指定序号开始至头部的数据
 * @[in]
 * pHandle - 打开的日志文件的句柄
 * pszBuffer - 待写入缓冲区
 * nSize - 待写入缓冲区长度
 * nSocket - 当不为0时读取指定套接字的上一条数据，当为0时忽略
 * @[out]
 * pnReadBytes - 返回已写入数据的长度
 * pIndex      - 返回索引节点.
 * 如果同一序号读取时缓冲区不够大，先将现有的缓冲区写满
 * 并返回WAVETOP_MIRROR_MORE_WITH_LINE，然后再次调用此方法接着读取。
 * 该序号读取完成返回WAVETOP_BACKUP_OK
 * 已读取至头部返回WAVETOP_MIRROR_LINE_END
 * 出现异常返回WAVETOP_BACKUP_INTERNAL_ERROR
 */
MILOG_EXPORT_(int) MiMSSqlReadBufferFromLogPrev(MiLogHandle hHandle,
                                                       char *pszBuffer,
                                                       unsigned long nSize,
                                                       unsigned long *pnReadBytes,
                                                       unsigned long nSocket,
                                                       ExIndexSt *pIndex);

/**
 * 该方法可以获取日志文件中最大和最小的序号
 * @[in]
 * pHandle - 打开的日志文件的句柄
 * @[out]
 * pnMax - 存放取得的最大序号值
 * pnMin - 存放取得的最小序号值
 * 成功返回WAVETOP_BACKUP_OK
 * 失败返回WAVETOP_BACKUP_INTERNAL_FAILED
 */
MILOG_EXPORT_(int) MiMSSqlGetMaxAndMinNum(MiLogHandle hHandle,
                                                 PRUint64 *pnMax,
                                                 PRUint64 *pnMin);

/**
 * 指定从某一序号读取数据结束
 * pHandle - 打开的日志文件的句柄
 * @[out]
 * 成功返回WAVETOP_BACKUP_OK
 * 失败返回WAVETOP_BACKUP_INTERNAL_ERROR
 */
MILOG_EXPORT_(int) MiMSSqlReadBufferEnd(MiLogHandle hHandle);

/**
 * 自动删除没有被使用的日志文件
 * pHandle - 打开的日志文件的句柄
 * @[out]
 * 成功返回WAVETOP_BACKUP_OK
 * 失败返回WAVETOP_BACKUP_INTERNAL_ERROR
 */
MILOG_EXPORT_(int) MiMSSqlDeleteLog(MiLogHandle hHandle);

/**
 * 打开共享内存WAVETOP_MMShare_INSTANCE_DBNAME
 * 如果遇到共享内存块的nWay=WAVETOP_MIRROR_MSSQL2_LOG_BROKEN
 * 返回nStatus=WAVETOP_MIRROR_MSSQL2_LOG_BROKEN
 * 否则nStatus=0
 * @[out]
 * 成功返回WAVETOP_BACKUP_OK
 * 失败返回WAVETOP_BACKUP_INTERNAL_ERROR
 */
MILOG_EXPORT_(int) BkMSSqlOpenShareMM(MiLogHandle hHandle, PRInt32 nWay, PRInt32 *nStatus);

/**
 * 打开或者创建共享内存WAVETOP_MMShare_INSTANCE_DBNAME
 * 找到没有被使用的共享内存块，然后写入
 * nWay=WAVETOP_MIRROR_MSSQL2_LOG_RESTARE
 * @[out]
 * 成功返回WAVETOP_BACKUP_OK
 * 失败返回WAVETOP_BACKUP_INTERNAL_ERROR
 */
#ifdef WIN32
MILOG_EXPORT_(int) BkMSSqlWriteShareMM(MiLogHandle hHandle, PRInt32 nWay);
#endif
/*
 * 新增两个方法，供backup6使用.
 */
/**
* 用于备份系统oracle服务器写入logdata
* 添加时间参数，时间是客户端传过来的
* 而不是现在的获取当前写入时间
* 将缓冲区中的数据写入指定日志文件
* @[in]
* pHandle - 打开的日志文件的句柄
* pszBuffer - 待写入日志文件的数据
* nBuffCount - 待写入日志文件数据的长度
* pIndex - 添加时间参数的结构
* @[out]
* 成功返回WAVETOP_BACKUP_OK
* 失败返回WAVETOP_BACKUP_INTERNAL_ERROR
*/
MILOG_EXPORT_(int) BkMSSqlWriteBufferToLog(MiLogHandle hHandle,
                                           WTBUF *pBuffers,
                                           unsigned long nBuffCount,
                                           ExIndexSt *pIndex);

/**
 * 该方法可以获取日志文件的文件名ID编号与最少Seq号
 * @[in]
 * pHandle - 打开的日志文件的句柄
 * nCurrSeq - 上一次全量版本的Seq 
 * @[out]
 * nBufLen - 取得该日志文件的ID编号
 * pnMin - 存放取得的最小序号值(Seq值)
 * 成功返回WAVETOP_BACKUP_OK
 * 失败返回WAVETOP_BACKUP_INTERNAL_FAILED
 */
MILOG_EXPORT_(int) BkMSSqlGetFileName(MiLogHandle hHandle, PRUint64 nCurrSeq, 
                                      int *nBufLen, PRUint64 *pnMin);
/**
 * 循环调用该方法，获取从指定序号开始至末尾的数据,并且删除当前已经读完的前一个文件
 * @[in]
 * pHandle - 打开的日志文件的句柄
 * pszBuffer - 待写入缓冲区
 * nSize - 待写入缓冲区长度
 * nSocket - 当不为0时读取指定套接字的下一条数据，当为0时忽略
 * @[out]
 * pnReadBytes - 返回已写入数据的长度
 * pIndex      - 返回索引节点.
 * 如果同一序号读取时缓冲区不够大，先将现有的缓冲区写满
 * 并返回WAVETOP_MIRROR_MORE_WITH_LINE，然后再次调用此方法接着读取
 * 该序号读取完成返回WAVETOP_BACKUP_OK
 * 已读取至末尾返回WAVETOP_MIRROR_LINE_END
 * 出现异常返回WAVETOP_BACKUP_INTERNAL_ERROR
 */
MILOG_EXPORT_(int) BkMSSqlReadBufferFromLogNext(MiLogHandle hHandle,
                                                       char *pszBuffer,
                                                       unsigned long nSize,
                                                       unsigned long *pnReadBytes,
                                                       unsigned long nSocket,
                                                       ExIndexSt *pIndex);


/*------------------------------- IO块优化接口-----------------------------*/

/**
 * IO优化初始化
 * @[in]
 * pIoDataSrcDir  - 需要优化的指定日志文件组存储路径
 * pIoDataTargDir - 优化完成后的日志文件组存储路径
 * pIoDataName    - 日志文件名格式
 * nDBtype - nDBtype == 1 ORACLE 数据库, nDBtype == 2 MSSQL数据库
 * @[out]
 * ppHandle -  打开的IO块优化文件句柄
 * 成功返回 WAVETOP_BACKUP_OK 否则返回错误代码
 */
MILOG_EXPORT_(int) BKMSSqlIoOptimizeBgein(MiLogHandle *ppHandle, char *pIoDataSrcDir, 
                                          char *pIoDataTargDir, char *pIoDataName, int nDBtype);
/**
 * IO块优化
 * @[in]
 * pHandle - 打开的优化文件句柄
 * nMinNum - IO块优化开始序列号
 * nMaxNum - IO块优化结束序列号
 * nWriteBegNum - IO块优化完成后开始写入序列号
 * @[out]
 * 成功返回 WAVETOP_BACKUP_OK, 否则返回错误代码
 */

MILOG_EXPORT_(int) BKMSSqlIoOptimize(MiLogHandle pHandle, PRUint64 nMinNum, 
                                     PRInt64 nMaxNum, PRInt64 nWriteBegNum);

/**
 * IO块优化结束
 * pHandle - 打开的优化文件句柄
 * @[out]
 * 成功返回WAVETOP_BACKUP_OK 否则返回错误代码
 */
MILOG_EXPORT_(int) BKMSSqlIoOptimizeEnd(MiLogHandle pHandle);


/**
 * 该方法可以获取日志文件的文件名ID编号
 * @[in]
 * pHandle - 打开的日志文件的句柄
 * @[out]
 * pnFileNum - 取得该日志文件的ID编号
 * 成功返回WAVETOP_BACKUP_OK
 * 失败返回WAVETOP_BACKUP_INTERNAL_FAILED
 */
MILOG_EXPORT_(int) BkMSSqlGetFileMinNum(MiLogHandle hHandle, PRInt32 *pnFileNum);

/**
 * 该方法可以获取指定文件编号的最小SEQ
 * @[in]
 * pHandle - 打开的日志文件的句柄
 * nFileNum - 指定的文件编号
 * @[out]
 * nMinSeq - 指定文件编号的最小SEQ  
 * 成功返回WAVETOP_BACKUP_OK
 * 失败返回WAVETOP_BACKUP_INTERNAL_FAILED
 */
MILOG_EXPORT_(int) BkMSSqlGetFileNumMinSeq(MiLogHandle hHandle, PRInt32 nFileNum, PRUint64 *nMinSeq);

//删除 比 nlastNum大的 logdata文件
MILOG_EXPORT_(int) MiMSSqlDeleteSuperMaxFileImmediate(MiLogHandle hHandle, int nLastNum);

//获取 指定文件最大的编号
MILOG_EXPORT_(int) MiMSSqlGetLastNum(MiLogHandle hHandle, int *nLastNum);

MILOG_EXPORT_(int) MiMSSqlSetLastNum(MiLogHandle hHandle, int nLastNum);

MILOG_EXPORT_(int) BkMSSqlGetFileMaxNum(MiLogHandle hHandle, int *pnFileNum);


MILOG_EXPORT_(int) MiMSSqlSetLastNum2(char *pszLogPath,char *pszSuff, int nLastNum);
MILOG_EXPORT_(int) BkMSSqlGetFileMaxNum2(char *pszLogPath,char *pszSuff, int *pnFileNum);
MILOG_EXPORT_(int) MiMSSqlDeleteFileImmediate2(char *pszLogPath,char *pszSuff, int nLastNum);
#ifdef __cplusplus
}
#endif /* defined(__cplusplus) */

#endif /* !defined(_LIBMI_MSSQL_LOG_H_) */
