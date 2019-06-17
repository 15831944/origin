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
     * ͳһ��ʼ����������ʼ��pool�ڴ�����Ա����
     * @[in]
     * pIoDataSrcDir - Ҫ�Ż���IO����·��
     * pIoDataTargDir - �Ż���ɺ�IO����·��
     * pIoDataName - LogData �ļ���
     * @[out]
     * ��ʼ���ɹ�����WAVETOP_BACKUP_OK, ���򷵻�ʧ��״̬�롣
     */
    int Init(char *pIoDataSrcDir, char *pIoDataTargDir, char *pIoDataName);

	/* 
     * �ϲ��ظ���IO����������LogData�ļ�
     * @[in]
     * nMinNum - IO���Ż���ʼ���к�
     * nMaxNum - IO���Ż��������к�
     * nWriteBegNum - IO���Ż���ɺ�ʼд�����к�
     * @[out]
     * �Ż��ɹ�����WAVETOP_BACKUP_OK, ���򷵻�ʧ��״̬�롣
     */
    int IoDataOptimize(PRUint64 nMinNum, PRInt64 nMaxNum, PRInt64 nWriteBegNum); 

	/* �ͷ���Դ */
    int UnInit();

private:

	ap_pool *m_pIoPool;

	/* Ҫ�Ż�����־�ļ���� */
	MiLogHandle m_hSrcLog;
	
	/* Ŀ����־�ļ���� */
    MiLogHandle m_hTargLog;

	/* IO���Ż���ʼ���к�*/
	PRUint64 m_nBeginNum;
	
	/* IO���Ż��������к� */
     PRUint64 m_nEndNum;

};


#endif /* _WAVETOP_IO_OPTIMIZE_H_ */