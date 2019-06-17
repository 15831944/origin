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
* ��ʼдarchive�ļ�ͷ��Ϣ
* @[in]
* pszFilename �ļ���
* nFileSize �ļ���С
* @[out]
* �ɹ�����WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkDStoreFileBegin(request_rec *pReq, StoreHandle *pStore,
                                  char *pszFileName, char *pszArcFileName, PRInt32 nBuffSize);

/* 
* ��ȡarchive����
* @[in]
* pNode archive no��ƫ����
* nType = 0 ��һ�δ�archive���
* @[out]
* pBuff ��ȡ�����ݡ����С��
* �ɹ�����WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkDStoreFileRead(request_rec *pReq, StoreHandle *pStore, 
                                 ArchiveBlockBuff *pBuff, BkIndexNodeSt *pNode, PRInt32 *nType);

/* 
* д���ݿ��ļ�����
* @[in]
* pszFileName �ļ���
* @[out]
* �ɹ�����WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkDStoreDbFileWrite(request_rec *pReq, char *pszFileName, StoreHandle *pStore, char *pszDir=NULL);


/* ================================== */
/* archive W/R API*/
/* ================================== */

/* 
* ��ʼ��store��� 
* pReq
* @[out]
* pStore
* �ɹ�����WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkStoreHandleInit(request_rec *pReq, StoreHandle **pStore);

/* 
* ��store�����sqlite����������ļ����
* archive�ļ����
* pReq
* @[in]
* nType = 1 ����
* @[out]
* pStore
* �ɹ�����WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkStoreHandleOpen(request_rec *pReq, StoreHandle *pStore, PRInt32 nType);

API_EXPORT(int) BkStoreHandleOpen2(request_rec *pReq, StoreHandle *pStore, int nType, char *pszPath);

/* open tape archive db handle*/
API_EXPORT(int) BkTapeHandleOpen(request_rec *pReq, const char *pszTablePath, sqlite3 **pDataBase);
/* 
* �ر�store���
* @[in]
* pStore
* @[out]
* �ɹ�����WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkStoreHandleClose(StoreHandle *pStore);

/* 
* ��ʼдarchive�ļ�ͷ��Ϣ
* @[in]
* pszFilename �ļ���
* nFileSize �ļ���С
* @[out]
* �ɹ�����WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkStoreFileBegin(request_rec *pReq, StoreHandle *pStore, 
                                 char *pszFilename, PRInt64 nFileSize);

/* 
* дindex�ļ���sqlite��
* @[in]
* pStore
* @[out]
* �ɹ�����WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkStoreFileEnd(request_rec *pReq, StoreHandle *pStore);

/* ��ʱ������������ʹ�� 
   1.���Ը����ݿ�ʹ�ã�BkStoreFileEnd��
   2.���Ը���ʱ�ļ�ʹ��*/
API_EXPORT(int) BkStoreFileEnd2(request_rec *pReq, StoreHandle *pStore);

/* 
* дdata sqlite��
* @[in]
* pStore
* @[out]
* �ɹ�����WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkStoreWriteDataInfo(request_rec *pReq, char *pszPath, char *pszModeType, unsigned long nTime);

/* 
* дarchive����
* @[in]
* pszData ���ݿ�
* nLength ���ݿ��С
* nArcSize дarchive֮ǰarchive��С
* nLoopSize ѭ������ܴ�С
* @[out]
* �ɹ�����WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkStoreFileWrite(request_rec *pReq, StoreHandle *pStore, char *pszData, 
                                 unsigned long nLength, PRInt64 nLoopSize, PRInt64 nArcSize);
/* 
* ��ȡarchive����
* @[in]
* pNode archive no��ƫ����
* nType = 0 ��һ�δ�archive���
* @[out]
* pBuff ��ȡ�����ݡ����С��
* �ɹ�����WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkStoreFileRead(request_rec *pReq, StoreHandle *pStore, 
                                ArchiveBlockBuff *pBuff, BkIndexNodeSt *pNode, PRInt32 *nType);

API_EXPORT(int) BkStoreFileRead2(request_rec *pReq, StoreHandle *pStore, ArchiveBlockBuff *pBuff, 
	BkIndexNodeSt *pNode, PRInt32 *nType, char *szArchvieFile);

/* 
* д���ݿ��ļ�����
* @[in]
* pszFileName �ļ���
* @[out]
* �ɹ�����WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkStoreDbFileWrite(request_rec *pReq, char *pszFileName, StoreHandle *pStore);

/* ================================================= */
/* store query API*/
/* ================================================= */

/* 
* ��ѯ�û������в�����Ϣ 
* @[in]
* pszTaskName - ������
* ������Ϊ��ʱ���Բ�ѯ���û������в���
* @[out]
* pNode - ���Խڵ㣻���ж��
* �ɹ�����WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkGetStoreSchedule(request_rec *pReq, StoreHandle *pStore, 
                                   char *pszSchName, SchFileHeaderSt **pNode);

/* 
* ��ѯ���Ե����а汾��Ϣ-������
* @[in]
* pReq 
* pszSchName - ������
* nVer - ��Ҫ��ѯ��ĳ���汾��Ϣ��Ϊ0ʱ����ʾ��ѯ���а汾��
* @[out]
* pNode - ����ڵ㡣
* �ɹ�����WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkGetStoreTask(request_rec *pReq, StoreHandle *pStore,
                               char *pszSchName, int nVer, SchTaskNodeSt **pNode);

/* 
* ��ѯ���Ե����а汾��Ϣ-������
* @[in]
* pReq 
* nType - 1 = max idx . 2= max archive
* @[out]
* nMaxId - max id
* �ɹ�����WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkGetTaskMaxId(request_rec *pReq, StoreHandle *pStore, unsigned long *nMaxId, 
                               PRInt32 nType);

API_EXPORT(int) BkGetTaskArcNo(request_rec *pReq, StoreHandle *pStore, char *pszTaskName,
                                  unsigned long nVersion, char **pszArcGroup);

/* 
* ��ѯarchive�Ĵ�С
* @[in]
* pReq 
* pszArcNo Ϊ�ղ�ѯ����archive�Ĵ�С
* pszArcNo ��Ϊ�� ��ѯĳ��archive�Ĵ�С
* @[out]
* pPosList - archive�Ĵ�С
* �ɹ�����WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkGetAllArcSize(request_rec *pReq, StoreHandle *pStore, char *pszArcNo, 
                                        SchArcPosSt **pPosList);

/* 
* ��ѯ�汾�ľ����ļ���Ϣ
 -�������ļ��е����ṹ
* @[in]
* pReq 
* pNode - ����ڵ㡣
* @[out]
* pRoot - Ŀ¼���ṹ��
* �ɹ�����WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkGetStoreIndex(request_rec *pReq, StoreHandle *pStore, SchTaskNodeSt *pNode, 
                                BkIndexNodeSt **pRoot, PRInt32 nType);

API_EXPORT(int) BkGetStoreIndex2(request_rec *pReq, StoreHandle *pStore, 
	SchTaskNodeSt *pCurNode, BkIndexNodeSt **pHead, PRInt32 nType, char *szIndexName);


/* 
* ��ѯ�����ļ������а汾��Ϣ. Ŀǰʵ�ָ�API�ѶȺܴ��ݲ�ʵ��.
* @[in] 
* pReq
* pszFile - �ļ���
* @[out]
* pNode - �ļ��汾��Ϣ��
* �ɹ�����WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkGetFileVersion(request_rec *pReq, StoreHandle *pStore,
                                 const char *pszFile, SchTaskNodeSt *pNode);

/* 
* ��ѯ���Ե�����
* @[in] 
* pReq
* pszFileName - ������
* @[out]
* stFileVer - ��������
* BkFileBaseSt -��������
* �ɹ�����WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkGetFileAttribate(request_rec *pReq, StoreHandle *pStore, 
                                    const char *pszFileName, BkFileVersionSt **stFileVer);

API_EXPORT(int) BkGetFileBase(request_rec *pReq, StoreHandle *pStore, 
                                    BkFileBaseSt **pFile);

/* 
* ��ѯ���Ե�ǰ���İ汾��
* @[in] 
* pReq
* pszSchName - ������
* @[out]
* pnVersion - DB���д˲��Ե����汾��
* �ɹ�����WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkGetStoreCurrVer(request_rec *pReq, StoreHandle *pStore, char *pszSchName, 
                                  unsigned long *pnVersion);
/* store query END */
/* ================================================= */


/* ================================================= */
/* store delete API */
/* ================================================= */
/* 
* ɾ�����԰汾
* @[in] 
* pszSchName - ������
* nVer - Ҫɾ���ڼ����汾;Ϊ0ʱ,��ʾɾ������
* @[out]
* pnVersion - ���ε���ɾ������������
* pnDelArcNum - ���ε���ɾ����Archvie����
* �ɹ�����WAVETOP_BACKUP_OK
* ע���˷����ض�ɾ��һ�����������汾����
* ĳһ���������������У���ɾ���˱�������
 */
API_EXPORT(int) BkStoreDelSch(request_rec *pReq, StoreHandle *pStore, char *pszPolicyName, int nDeleteVer,
                              unsigned long *pnDelTaskNum, unsigned long *pnDelArcNum, 
                              PRInt64 *pnCleanSize);

/* 
* [�����Ժ�ʵ��]. Ŀǰʵ�ָ�API�ѶȺܴ��ݲ�ʵ��.
* ɾ���ļ���
* �ٶ�һ���ļ���10���汾�ж����ڣ�
* ��δ�����á��Ժ���ʵ�֡�
* @[in] 
* @[out]
* �ɹ�����WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkStoreDelFile();

/* ================================================= */
/* store delete END */
/* ================================================= */

/* 
* ���ļ����
* @[in] 
* pszFile �ļ���
* nFlags ���ļ���ʽ
* @[out]
* pArchiveFD �ļ����
* �ɹ�����WAVETOP_BACKUP_OK
 */
API_EXPORT(int)  BkFileOpen(const char *pszFile, unsigned long nFlags, 
                            PRFileDesc **pArchiveFD);
/* 
* �ر��ļ����
 */
API_EXPORT(int)  BkFileClose(PRFileDesc *pArchiveFD);
/* 
* һ���ļ�������д����һ���ļ�
* @[in] 
* pReadDesc Դ�ļ����
* pWriteDesc Ŀ���ļ����
* nFileSize Դ�ļ���С
* @[out]
* �ɹ�����WAVETOP_BACKUP_OK
 */
API_EXPORT(int)  BkHandleWriteHandle(PRFileDesc *pReadDesc, PRFileDesc *pWriteDesc,
                                    PRInt64 nFileSize);
/* 
* �����ļ���archive����
* @[in] 
* pReq
* file �ļ�����
* nArcNo �ļ��洢��archive num
* nArcStartPos archive ��ʼƫ����
* nArcEndPos archive ĩβƫ����
* @[out]
* pNode - ���ṹ
* �ɹ�����WAVETOP_BACKUP_OK
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
* �û�store�ļ���
* @[in] 
* pReq
* @[out]
* �ɹ�����WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkBackupFileLock(request_rec *pReq, BackupFileLockSt **pLock);

API_EXPORT(int) BkBackupFileUnLock(request_rec *pReq, BackupFileLockSt *pLock);


/* 
* �Զ�ɾ������/ɾ��logdata�ļ�
* @[in] 
* pszFilename ����FILE/MSSQL/ORACLE
* nDeleteTime ɾ��ʱ��
* @[out]
* pnCleanSize ɾ���Ĵ�С
* �ɹ�����WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkAutoDeleteTask(request_rec *pReq, StoreHandle *pStore, char *pszFilename,
                     int nDeleteTime, PRInt64 *pnCleanSize);

API_EXPORT(int) BkDeleteLodataFile(request_rec *pReq, PRInt32 nCurNum, PRInt32 nDeleteTime);

/*
* �ƶ��������˵�logdata����ʱ·����
* @[in]
* pszLastNmOfDestDir  ʵ���������ݿ�����ʵ������Ϊ�գ�
* nCurrNum            �ļ�ĩβ����
* pszSourDir          Դ�ļ�·��
* pszDestDir          Ŀ���ļ�·��
* @[out]
* �ɹ�����WAVETOP_BACKUP_OK
*/
API_EXPORT(int) BkSaveLodataFileToTmp(char *pszLastNmOfDestDir, PRInt32 nCurrNum, 
									  char *pszLogPath, char *pszDestDir);

/* 
* д����ϵͳ���ݲ��Ա�
* @[in] 
* nTibNum TIB���
* pszFileName ������
* nTotalSize ���ݴ�С
* @[out]
* �ɹ�����WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkStoreHandleDriveWrite(request_rec *pReq, PRInt32 nTibNum, char *pszFileName,
										PRInt64 *nTotalSize);

/* 
* ɾ������ϵͳ����TIB�ļ�������ARCHIVE�ļ�
* @[in] 
* pszDir �û�OSĿ¼
* pszTibname Ҫɾ����TIB����
* @[out]
* �ɹ�����WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BackupDeleteTibFile(char *pszDir, char *pszTibname);

/* 
 * ��ȡ��ʱ���ָ�ʱ��������С�汾
 * @[in] 
 * nRestoreTime - ��ԭʱ��
 * @[out]
 * pnMin        - ��С�汾
 * pnMax        - ���汾
 * @[return]
 * �ɹ�����WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkGetRestoreTimeVersion(request_rec *pReq, 
                                        StoreHandle *pStore, 
                                        unsigned long nRestoreTime, 
                                        unsigned long *pnMin, 
                                        unsigned long *pnMax);

/* 
 * ��ȡ���汾�ָ�ʱ����С�汾
 * @[in] 
 * nCurrVer     - ��ǰ�汾
 * @[out]
 * pnMin        - ���汾
 * @[return]
 * �ɹ�����WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkGetMaxRestoreVersion(request_rec *pReq, 
                                       StoreHandle *pStore,
                                       unsigned long nCurrVer, 
                                       unsigned long *nMax);

/* 
 * ��ȡ���汾�ָ�ʱ����С�汾
 * @[in] 
 * nCurrVer     - ��ǰ�汾
 * @[out]
 * pnMin        - ��С�汾
 * @[return]
 * �ɹ�����WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkGetMinRestoreVersion(request_rec *pReq, 
                                       StoreHandle *pStore,
                                       unsigned long nCurrVer, 
                                       unsigned long *nMin);

/* 
* �ֶ�ɾ������ϵͳ����TIB�ļ�������ARCHIVE�ļ�
* @[in] 
* nDeleteVer ɾ���汾
* pszPolicyName Ҫɾ����TIB����
* @[out]
* �ɹ�����WAVETOP_BACKUP_OK
 */
API_EXPORT(int) BkStoreDeleteDrive(request_rec *pReq, StoreHandle *pStore, char *pszPolicyName,
                        unsigned long nDeleteVer);
API_EXPORT(int) BkAutoDeleteDriveTask(request_rec *pReq, StoreHandle *pStore, char *pszFilename,
                     int nDeleteTime, PRInt64 *pnCleanSize);

API_EXPORT(int) BkStoreOneFileRead(request_rec *pReq, StoreHandle *pStore, char *pszArchvieFile, 
    ArchiveBlockBuff *pBuff, PRInt32 *nType);

API_EXPORT(int) BkGetFullName(request_rec *pReq, StoreHandle *pStore, char *pszDiffFile, char **pszFullName);

/* ƴװtemp�ļ�·��
 * @[in]
 * pszSrcFilePath - Դ·��
 * @[out] 
 * pszTargetFilePath - ��tempĿ¼�µ�·
 */
API_EXPORT(int) BkMontageTempPath(request_rec *pReq, char *pszSrcFilePath, char *pszTargetFilePath);

API_EXPORT(int) BkGetSingleIndexNode(BkIndexNodeSt *pNode, char *pszFileName, char *pszName, BkIndexNodeSt** pSearchResult);

API_EXPORT(bool) BkSerQueryNode(BkIndexNodeSt *pNode, struct file_struct *file, char *pszFileName,char* pszName, BkIndexNodeSt **stIndex);
API_EXPORT(int) travalIndexTree(ap_pool *pool,BkIndexNodeSt* pHead,BkIndexNodeSt** pResult, char* pszName, PRInt64 *nTotalSize);



#ifdef __cplusplus
}
#endif

#endif