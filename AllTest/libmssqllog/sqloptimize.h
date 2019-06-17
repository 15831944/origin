/** =============================================================================
 ** Copyright (c) 2011 WaveTop Information Corp. All rights reserved.
 **
 ** The A-CDP system
 **
 ** =============================================================================
 */

#ifndef _WAVETOP_SQL_OPTIMIZE_H_
#define _WAVETOP_SQL_OPTIMIZE_H_

class CSqlOptimize
{
public:
	CSqlOptimize();
	virtual ~CSqlOptimize();
public:

	/* 
     * 统一初始化方法：初始化pool内存和类成员变量
     * @[in]
     * pIoDataSrcDir - 要优化的IO块存放路径
     * pIoDataTargDir - 优化完成后IO块存放路径
     * pIoDataName - LogData 文件名
     * @[out]
     * 初始化成功返回WAVETOP_BACKUP_OK, 否则返回失败状态码。
     */
    int Init(char *pIoDataSrcDir, char *pIoDataTargDir, char *pIoDataName);

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

	ap_pool *m_pIoPool;

	/* 要优化的日志文件句柄 */
	MiLogHandle m_hSrcLog;
	
	/* 目标日志文件句柄 */
    MiLogHandle m_hTargLog;

	/* IO块优化开始序列号*/
	PRUint64 m_nBeginNum;
	
	/* IO块优化结束序列号 */
     PRUint64 m_nEndNum;

};


#endif /* _WAVETOP_IO_OPTIMIZE_H_ */