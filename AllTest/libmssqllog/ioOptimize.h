/** =============================================================================
 ** Copyright (c) 2011 WaveTop Information Corp. All rights reserved.
 **
 ** The A-CDP system
 **
 ** =============================================================================
 */

#ifndef _WAVETOP_IO_OPTIMIZE_H_
#define _WAVETOP_IO_OPTIMIZE_H_

#include <string.h>
#include "nspr.h"
#include "server_log.h"
#include "libmssqllog.h"
#include "logfile.h"
#include "walloc.h"

#define WAVETP_CDP_ORA_BLOCK_SIZE        (256 * 1024)
#define WAVETOP_IO_OPTIMIZE_BLOCK_SIZE   (16 * 1024 * 1024)
#define WAVETOP_CDP_ORA_MAX_FILE_NAME    8192
#define WAVETOP_CDP_ORA_CONTROL_FILE     (1 << 0)
#define WAVETOP_CDP_ORA_REDOLOG_FILE     (1 << 1)
#define WAVETOP_ORACLE_DB                1
#define WAVETOP_MSSQL_DB                 2
#define WAVETOP_IO_MEMLIMIT              (1024*1024*1024)

/* 当前节点头信息结构 */
typedef struct _IO_DATA_HEAD_ST {
    int nFileNameLen;
    char szFileName[1024];
    PRInt16 nFileNO;
    PRInt16 nFileType;
    PRInt16 nAction;
    PRUint32 nHighOffset32;
    PRUint32 nLowOffset32;
    PRInt64 nOffSet;
    PRInt32 nDataLen;
	
    int nSize;
} IoDataHeadSt;


/* IO块哈希表节点结构 */
typedef struct  _IO_DATA_NODE_ST {
	
    /* IO块的序号 */
    PRUint64 nSeqNum;
    
    /* 偏移量 */
    PRInt64 nOffset;
	
    /* 数据长度 */
    long nSize;
	
	_IO_DATA_NODE_ST *pNext;
	
} IoDataNodeSt;

/* 文件名关联的哈希数组链表 */
typedef struct  _IO_DATA_INFO_ST {
    
	/*  IO块对应的数据文件名 */
    char *pszFileName;
    
    /* 哈希数组 */
    IoDataNodeSt **ppQueue;
	
    _IO_DATA_INFO_ST *pNext;
	
} IoDataInfoSt;

/* 丢弃IO块链表 */
typedef struct _IO_CHUCK_LIST_ST {
    
    /* IO块的序号 */
    PRUint64 nSeqNum;
	
	_IO_CHUCK_LIST_ST *pNext; 
	
} IoChuckListSt;


class CIoOptimizeObject {

public:    
    CIoOptimizeObject();
    virtual ~CIoOptimizeObject();

public:

    /* 
     * 统一初始化方法：初始化pool内存和类成员变量
     * @[in]
     * pIoDataSrcDir - 要优化的IO块存放路径
     * pIoDataTargDir - 优化完成后IO块存放路径
     * pIoDataName - LogData 文件名
	 * nDBtype - 数据类型,nDBtype == 1 ORACLE数据库, nDBtype == 2 MSSQL数据库 
     * @[out]
     * 初始化成功返回WAVETOP_BACKUP_OK, 否则返回失败状态码。
     */
    int Init(char *pIoDataSrcDir, char *pIoDataTargDir, char *pIoDataName, int nDBtype);
    
    /* 
     * 合并重复的IO块重新生成LogData文件
     * @[in]
     * nMinNum - IO块优化开始序列号
     * nMaxNum - IO块优化结束序列号
     * nWriteBegNum - IO块优化完成后开始写入序列号
     * @[out]
     * 优化成功返回WAVETOP_BACKUP_OK, 否则返回失败状态码。
     */
    int IoDataOptimize(PRUint64 nMinNum, PRInt64 nMaxNum, PRInt64 nWriteBegNum); 
    
    
    /* 释放资源 */
    int UnInit();

private:

   /* 以下定义Hash的方法*/
   int Hash(const void *pKey);
   
   /* Hash 比较方法 */
   int Compare(const void *pKey1, const void *pKey2);
   
   /* 
    * 读取的IO块的文件名来定位行并返回哈希数组如果没有行
    * 新添加行并返回哈希数组    
    * @[in]
    * pszFileName - IO块对应的文件名
    * ppIoDataIndexInfoSt - 文件名对应的哈希数组
    * @[out]
    * 成功返回WAVETOP_BACKUP_OK, 否则返回失败状态码
    */
   int IoNewAndSelectRow(pool *pPool, char*pszFileName, IoDataInfoSt **ppIoDataInfoList);
   
   /*  
    * 哈希数组上搜索IO块的节点
    * 判断IO块是不是重复的IO块 
    * 如果是重复的IO块标记为丢弃添加
    * 到丢弃链表否则添加到哈希数组节点和丢弃链表
    * @[in]
    * pIoDataNodeList - io块结构
    * pIoDataInfoList - io块文件名对应的hash数组
    * @[out]
    * ppIoChuckList - 要加入丢弃链表的节点
    * 成功返回WAVETOP_BACKUP_OK, 否则返回失败状态码
    */
   int IoAddAndCheckNode(IoDataNodeSt *pIoDataNode, IoDataInfoSt *pIoDataInfoList, 
	                     IoChuckListSt **ppIoChuckList);
    
   /* 构造丢弃链表 */
   int IoNewChuckList(IoChuckListSt *pIoChuckList);

   /* 根据丢弃链表重新写logdata文件 */
   int IoWriteLogDataFile(PRUint64 nMinNum, PRInt64 nMaxNum, PRInt64 nWriteBegNum);
  
private:
     ap_pool *m_pIoPool;

     int m_nBaseBucketNum;

     /* 要优化的日志文件句柄 */
     MiLogHandle m_hSrcLog;

     /* 目标日志文件句柄 */
     MiLogHandle m_hTargLog;

     /* IO块优化开始序列号*/
     int m_nBeginNum;

     /* IO块优化结束序列号 */
     int m_nEndNum;
     
	 /* 数据库类型 m_DBtype == 1 ORACLE数据库, m_DBtype == 2 MSSQL数据库*/
	 int m_DBtype;
	 
     IoDataInfoSt *m_pIoFileNameList;

     /* 丢弃任务列表 */
     IoChuckListSt *m_pIoChuckListSt;    
};


#endif /* _WAVETOP_IO_OPTIMIZE_H_ */