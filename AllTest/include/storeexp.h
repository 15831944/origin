/** =============================================================================
** Copyright (c) 2004 WaveTop Information Corp. All rights reserved.
**
** The Report Table system
**
** =============================================================================
*/

#ifndef __BACKUP_BK6_STORE_H_
#define __BACKUP_BK6_STORE_H_ 1

#include "dbtable.h"

typedef struct _BACKUP_STORE_HANDLE_ {
    ap_pool *pPool;
    
    int nArchiveNo;
    int nIndexNo;
    int nStatus;
    
    sqlite3 *pSchDbFd;
    PRFileDesc *pArchiveFD;
    PRFileDesc *pIndexFD;
    BkIndexNodeSt *pIndexRoot;
    SchTaskNodeSt *pTaskRoot;
    SchDataSt *pDataRoot;

    char *pszArcGroup;
    char *pszFileName;
    
    PRInt64 nCurrArcPos;
    PRInt64 nIndexPos;
    PRInt64 nTaskSize;
    PRInt64 nDBSeq;
    PRInt32 nSeqTime;
	PRInt32 nFileType;
    
    void *pReserve;
} StoreHandle;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
#include "windows.h"
    
#define API_EXPORT(p) __declspec(dllexport) p
#else
#ifndef API_EXPORT
#define API_EXPORT(p) extern p
#endif
#endif

/* ================================== */
/* direct W/R API*/
/* ================================== */


API_EXPORT(int) BkQueryMatchedFile(request_rec *pReq, 
								StoreHandle *pStore, 
                               const char *pszFilename,
							   const char *pszMatchedStr,
                               int nVer, 
							   BkFileBaseSt **pFileBase);

/* 
* 开始写archive文件头信息
* @[in]
* pszFilename 文件名
* nFileSize 文件大小
* @[out]
* 成功返回WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkDStoreFileBegin(request_rec *pReq, StoreHandle *pStore,
                                  char *pszFileName, char *pszArcFileName, PRInt32 nBuffSize);

/* 
* 读取archive数据
* @[in]
* pNode archive no和偏移量
* nType = 0 第一次打开archive句柄
* @[out]
* pBuff 读取的数据、块大小等
* 成功返回WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkDStoreFileRead(request_rec *pReq, StoreHandle *pStore, 
                                 ArchiveBlockBuff *pBuff, BkIndexNodeSt *pNode, PRInt32 *nType);

/* 
* 写数据库文件数据
* @[in]
* pszFileName 文件名
* @[out]
* 成功返回WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkDStoreDbFileWrite(request_rec *pReq, char *pszFileName, StoreHandle *pStore, char *pszDir=NULL);


/* ================================== */
/* archive W/R API*/
/* ================================== */

/* 
* 初始化store句柄 
* pReq
* @[out]
* pStore
* 成功返回WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkStoreHandleInit(request_rec *pReq, StoreHandle **pStore);

/* 
* 打开store句柄，sqlite句柄，索引文件句柄
* archive文件句柄
* pReq
* @[in]
* nType = 1 备份
* @[out]
* pStore
* 成功返回WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkStoreHandleOpen(request_rec *pReq, StoreHandle *pStore, PRInt32 nType);

API_EXPORT(int) BkStoreHandleOpen2(request_rec *pReq, StoreHandle *pStore, int nType, char *pszPath);

/* open tape archive db handle*/
API_EXPORT(int) BkTapeHandleOpen(request_rec *pReq, const char *pszTablePath, sqlite3 **pDataBase);
/* 
* 关闭store句柄
* @[in]
* pStore
* @[out]
* 成功返回WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkStoreHandleClose(StoreHandle *pStore);

/* 
* 开始写archive文件头信息
* @[in]
* pszFilename 文件名
* nFileSize 文件大小
* @[out]
* 成功返回WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkStoreFileBegin(request_rec *pReq, StoreHandle *pStore, 
                                 char *pszFilename, PRInt64 nFileSize);

/* 
* 写index文件和sqlite表
* @[in]
* pStore
* @[out]
* 成功返回WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkStoreFileEnd(request_rec *pReq, StoreHandle *pStore);

/* 定时备份增量内容使用 
   1.可以给数据库使用（BkStoreFileEnd）
   2.可以给定时文件使用*/
API_EXPORT(int) BkStoreFileEnd2(request_rec *pReq, StoreHandle *pStore);

/* 
* 写data sqlite表
* @[in]
* pStore
* @[out]
* 成功返回WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkStoreWriteDataInfo(request_rec *pReq, char *pszPath, char *pszModeType, unsigned long nTime);

/* 
* 写archive数据
* @[in]
* pszData 数据块
* nLength 数据块大小
* nArcSize 写archive之前archive大小
* nLoopSize 循环块的总大小
* @[out]
* 成功返回WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkStoreFileWrite(request_rec *pReq, StoreHandle *pStore, char *pszData, 
                                 unsigned long nLength, PRInt64 nLoopSize, PRInt64 nArcSize);
/* 
* 读取archive数据
* @[in]
* pNode archive no和偏移量
* nType = 0 第一次打开archive句柄
* @[out]
* pBuff 读取的数据、块大小等
* 成功返回WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkStoreFileRead(request_rec *pReq, StoreHandle *pStore, 
                                ArchiveBlockBuff *pBuff, BkIndexNodeSt *pNode, PRInt32 *nType);

API_EXPORT(int) BkStoreFileRead2(request_rec *pReq, StoreHandle *pStore, ArchiveBlockBuff *pBuff, 
	BkIndexNodeSt *pNode, PRInt32 *nType, char *szArchvieFile);

/* 
* 写数据库文件数据
* @[in]
* pszFileName 文件名
* @[out]
* 成功返回WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkStoreDbFileWrite(request_rec *pReq, char *pszFileName, StoreHandle *pStore);

/* ================================================= */
/* store query API*/
/* ================================================= */

/* 
* 查询用户的所有策略信息 
* @[in]
* pszTaskName - 策略名
* 策略名为空时可以查询该用户的所有策略
* @[out]
* pNode - 策略节点；可有多个
* 成功返回WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkGetStoreSchedule(request_rec *pReq, StoreHandle *pStore, 
                                   char *pszSchName, SchFileHeaderSt **pNode);

/* 
* 查询策略的所有版本信息-即任务
* @[in]
* pReq 
* pszSchName - 策略名
* nVer - 需要查询的某个版本信息；为0时，表示查询所有版本。
* @[out]
* pNode - 任务节点。
* 成功返回WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkGetStoreTask(request_rec *pReq, StoreHandle *pStore,
                               char *pszSchName, int nVer, SchTaskNodeSt **pNode);

/* 
* 查询策略的所有版本信息-即任务
* @[in]
* pReq 
* nType - 1 = max idx . 2= max archive
* @[out]
* nMaxId - max id
* 成功返回WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkGetTaskMaxId(request_rec *pReq, StoreHandle *pStore, unsigned long *nMaxId, 
                               PRInt32 nType);

API_EXPORT(int) BkGetTaskArcNo(request_rec *pReq, StoreHandle *pStore, char *pszTaskName,
                                  unsigned long nVersion, char **pszArcGroup);

/* 
* 查询archive的大小
* @[in]
* pReq 
* pszArcNo 为空查询所有archive的大小
* pszArcNo 不为空 查询某个archive的大小
* @[out]
* pPosList - archive的大小
* 成功返回WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkGetAllArcSize(request_rec *pReq, StoreHandle *pStore, char *pszArcNo, 
                                        SchArcPosSt **pPosList);

/* 
* 查询版本的具体文件信息
 -即索引文件中的树结构
* @[in]
* pReq 
* pNode - 任务节点。
* @[out]
* pRoot - 目录树结构。
* 成功返回WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkGetStoreIndex(request_rec *pReq, StoreHandle *pStore, SchTaskNodeSt *pNode, 
                                BkIndexNodeSt **pRoot, PRInt32 nType);

API_EXPORT(int) BkGetStoreIndex2(request_rec *pReq, StoreHandle *pStore, 
	SchTaskNodeSt *pCurNode, BkIndexNodeSt **pHead, PRInt32 nType, char *szIndexName);


/* 
* 查询单个文件的所有版本信息. 目前实现该API难度很大，暂不实现.
* @[in] 
* pReq
* pszFile - 文件名
* @[out]
* pNode - 文件版本信息。
* 成功返回WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkGetFileVersion(request_rec *pReq, StoreHandle *pStore,
                                 const char *pszFile, SchTaskNodeSt *pNode);

/* 
* 查询策略的属性
* @[in] 
* pReq
* pszFileName - 策略名
* @[out]
* stFileVer - 策略属性
* BkFileBaseSt -策略属性
* 成功返回WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkGetFileAttribate(request_rec *pReq, StoreHandle *pStore, 
                                    const char *pszFileName, BkFileVersionSt **stFileVer);

API_EXPORT(int) BkGetFileBase(request_rec *pReq, StoreHandle *pStore, 
                                    BkFileBaseSt **pFile);

/* 
* 查询策略当前最大的版本号
* @[in] 
* pReq
* pszSchName - 策略名
* @[out]
* pnVersion - DB库中此策略的最大版本号
* 成功返回WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkGetStoreCurrVer(request_rec *pReq, StoreHandle *pStore, char *pszSchName, 
                                  unsigned long *pnVersion);
/* store query END */
/* ================================================= */


/* ================================================= */
/* store delete API */
/* ================================================= */
/* 
* 删除策略版本
* @[in] 
* pszSchName - 策略名
* nVer - 要删除第几个版本;为0时,表示删除所有
* @[out]
* pnVersion - 本次调用删除的任务数量
* pnDelArcNum - 本次调用删除的Archvie数量
* 成功返回WAVETOP_BACKUP_OK
* 注：此方法必定删除一个备份链；版本号在
* 某一个备份链的序列中，就删除此备份链。
 */
API_EXPORT(int) BkStoreDelSch(request_rec *pReq, StoreHandle *pStore, char *pszPolicyName, int nDeleteVer,
                              unsigned long *pnDelTaskNum, unsigned long *pnDelArcNum, 
                              PRInt64 *pnCleanSize);

/* 
* [留着以后实现]. 目前实现该API难度很大，暂不实现.
* 删除文件；
* 假定一个文件在10个版本中都存在；
* 如何处理最好。以后在实现。
* @[in] 
* @[out]
* 成功返回WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkStoreDelFile();

/* ================================================= */
/* store delete END */
/* ================================================= */

/* 
* 打开文件句柄
* @[in] 
* pszFile 文件名
* nFlags 打开文件方式
* @[out]
* pArchiveFD 文件句柄
* 成功返回WAVETOP_BACKUP_OK
 */
API_EXPORT(int)  BkFileOpen(const char *pszFile, unsigned long nFlags, 
                            PRFileDesc **pArchiveFD);
/* 
* 关闭文件句柄
 */
API_EXPORT(int)  BkFileClose(PRFileDesc *pArchiveFD);
/* 
* 一个文件的数据写到另一个文件
* @[in] 
* pReadDesc 源文件句柄
* pWriteDesc 目标文件句柄
* nFileSize 源文件大小
* @[out]
* 成功返回WAVETOP_BACKUP_OK
 */
API_EXPORT(int)  BkHandleWriteHandle(PRFileDesc *pReadDesc, PRFileDesc *pWriteDesc,
                                    PRInt64 nFileSize);
/* 
* 单个文件的archive属性
* @[in] 
* pReq
* file 文件属性
* nArcNo 文件存储的archive num
* nArcStartPos archive 起始偏移量
* nArcEndPos archive 末尾偏移量
* @[out]
* pNode - 树结构
* 成功返回WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkWriteFileIndex(request_rec *pReq, struct file_struct *file, unsigned long nArcNo,
                                    BkIndexNodeSt **pNode, PRInt64 nArcEndPos, PRInt64 nArcStartPos);

/* Travel the first-level sub-directories and files under
 * the specified directory.
 * @[in]
 * The pszDirName is the client physical file name.
 * pResultCB is the callback function.
 * pArg is a favorite option.
 * @[out]
 * Return value is WAVETOP_BACKUP_OK, when the base
 * file is successfully deleted. otherwise, return error code.
 * When successful, call pResultCB to deliver the backup file information.
 */
API_EXPORT(int) BkQueryBaseDir(request_rec *pReq, StoreHandle *pStore, const char *pszFilename,
                               int nVer, BkFileBaseSt **pFileBase);

API_EXPORT(int) BkQueryBaseDir2(request_rec *pReq, StoreHandle *pStore, 
	                            const char *pszFilename,
	                            int nVer, BkFileBaseSt **pFileBase, char *indexFile);


/* 
* 用户store文件锁
* @[in] 
* pReq
* @[out]
* 成功返回WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkBackupFileLock(request_rec *pReq, BackupFileLockSt **pLock);

API_EXPORT(int) BkBackupFileUnLock(request_rec *pReq, BackupFileLockSt *pLock);


/* 
* 自动删除备份/删除logdata文件
* @[in] 
* pszFilename 类型FILE/MSSQL/ORACLE
* nDeleteTime 删除时间
* @[out]
* pnCleanSize 删除的大小
* 成功返回WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkAutoDeleteTask(request_rec *pReq, StoreHandle *pStore, char *pszFilename,
                     int nDeleteTime, PRInt64 *pnCleanSize);

API_EXPORT(int) BkDeleteLodataFile(request_rec *pReq, PRInt32 nCurNum, PRInt32 nDeleteTime);

/*
* 移动服务器端的logdata至临时路径下
* @[in]
* pszLastNmOfDestDir  实例名与数据库名（实例名可为空）
* nCurrNum            文件末尾序列
* pszSourDir          源文件路径
* pszDestDir          目标文件路径
* @[out]
* 成功返回WAVETOP_BACKUP_OK
*/
API_EXPORT(int) BkSaveLodataFileToTmp(char *pszLastNmOfDestDir, PRInt32 nCurrNum, 
									  char *pszLogPath, char *pszDestDir);

/* 
* 写操作系统备份策略表
* @[in] 
* nTibNum TIB编号
* pszFileName 策略名
* nTotalSize 备份大小
* @[out]
* 成功返回WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkStoreHandleDriveWrite(request_rec *pReq, PRInt32 nTibNum, char *pszFileName,
										PRInt64 *nTotalSize);

/* 
* 删除操作系统备份TIB文件，类似ARCHIVE文件
* @[in] 
* pszDir 用户OS目录
* pszTibname 要删除的TIB名字
* @[out]
* 成功返回WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BackupDeleteTibFile(char *pszDir, char *pszTibname);

/* 
 * 获取按时间点恢复时的最大和最小版本
 * @[in] 
 * nRestoreTime - 还原时间
 * @[out]
 * pnMin        - 最小版本
 * pnMax        - 最大版本
 * @[return]
 * 成功返回WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkGetRestoreTimeVersion(request_rec *pReq, 
                                        StoreHandle *pStore, 
                                        unsigned long nRestoreTime, 
                                        unsigned long *pnMin, 
                                        unsigned long *pnMax);

/* 
 * 获取按版本恢复时的最小版本
 * @[in] 
 * nCurrVer     - 当前版本
 * @[out]
 * pnMin        - 最大版本
 * @[return]
 * 成功返回WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkGetMaxRestoreVersion(request_rec *pReq, 
                                       StoreHandle *pStore,
                                       unsigned long nCurrVer, 
                                       unsigned long *nMax);

/* 
 * 获取按版本恢复时的最小版本
 * @[in] 
 * nCurrVer     - 当前版本
 * @[out]
 * pnMin        - 最小版本
 * @[return]
 * 成功返回WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkGetMinRestoreVersion(request_rec *pReq, 
                                       StoreHandle *pStore,
                                       unsigned long nCurrVer, 
                                       unsigned long *nMin);

/* 
* 手动删除操作系统备份TIB文件，类似ARCHIVE文件
* @[in] 
* nDeleteVer 删除版本
* pszPolicyName 要删除的TIB名字
* @[out]
* 成功返回WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkStoreDeleteDrive(request_rec *pReq, StoreHandle *pStore, char *pszPolicyName,
                        unsigned long nDeleteVer);
API_EXPORT(int) BkAutoDeleteDriveTask(request_rec *pReq, StoreHandle *pStore, char *pszFilename,
                     int nDeleteTime, PRInt64 *pnCleanSize);

API_EXPORT(int) BkStoreOneFileRead(request_rec *pReq, StoreHandle *pStore, char *pszArchvieFile, 
    ArchiveBlockBuff *pBuff, PRInt32 *nType);

API_EXPORT(int) BkGetFullName(request_rec *pReq, StoreHandle *pStore, char *pszDiffFile, char **pszFullName);

/* 拼装temp文件路径
 * @[in]
 * pszSrcFilePath - 源路径
 * @[out] 
 * pszTargetFilePath - 在temp目录下的路
 */
API_EXPORT(int) BkMontageTempPath(request_rec *pReq, char *pszSrcFilePath, char *pszTargetFilePath);

API_EXPORT(int) BkGetSingleIndexNode(BkIndexNodeSt *pNode, char *pszFileName, char *pszName, BkIndexNodeSt** pSearchResult);

API_EXPORT(bool) BkSerQueryNode(BkIndexNodeSt *pNode, struct file_struct *file, char *pszFileName,char* pszName, BkIndexNodeSt **stIndex);
API_EXPORT(int) travalIndexTree(ap_pool *pool,BkIndexNodeSt* pHead,BkIndexNodeSt** pResult, char* pszName, PRInt64 *nTotalSize);



#ifdef __cplusplus
}
#endif

#endif