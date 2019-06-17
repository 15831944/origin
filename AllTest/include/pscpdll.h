/** =============================================================================
 ** Copyright (c) 2012 WaveTop Information Corp. All rights reserved.
 **
 ** The VMware backup system
 **
 ** =============================================================================
 */

#ifndef __BACKUP_VM_PSCP_API_H_
#define __BACKUP_VM_PSCP_API_H_ 1

#ifdef __cplusplus
extern "C" {
#endif
    
#ifdef WIN32
#include <windows.h>
    
#define API_EXPORT(p) __declspec(dllexport) p
#else
#ifndef API_EXPORT
#define API_EXPORT(p) extern p
#endif
#endif


#ifdef __cplusplus
extern "C" {
#endif


/*
 * ͨ��ssh����Esxi��Esx�������ϴ��ļ�����
 * @[in]
 * pszHost         - Esxi ������IP��ַ
 * pUser           - Esxi �������û���
 * pPassword       - Esxi ����������
 * nPort           - Esxi ������ssh�˿�
 * pSourceFileName - �ϴ���Դ�ļ���(����·�� E:\data\test.txt)
 * pTargetFileName - �ϴ���Ŀ���ļ���(Զ��·�� root@192.168.1.160:/vmfs/volumes/datastore1/test)
 * @[out]
 * �ϴ��ļ��ɹ����� 0, ���򷵻ش����롣
 */
API_EXPORT(int) BkVmSShUnloadFile(char *pszHost, char *pszUser, char *pszPassword, int nPort, 
                                  char *pszSourceFile, char *pszTargetFile);

/*
 * ͨ��ssh����Esxi��Esx�����������ļ�����
 * @[in]
 * pszHost         - Esxi ������IP��ַ
 * pUser           - Esxi �������û���
 * pPassword       - Esxi ����������
 * nPort           - Esxi ������ssh�˿�
 * pSourceFileName - ���ص�Դ�ļ���(Զ��·�� root@192.168.1.160:/vmfs/volumes/datastore1/test)
 * pTargetFileName - ���ص�Ŀ���ļ��� (����·�� E:\data\)
 * @[out]
 * �����ļ��ɹ����� 0, ���򷵻ش����롣
 */
API_EXPORT(int) BkVmSShDownloadFile(char *pszHost, char *pszUser, char *pszPassword, int nPort,
                                    char *pszSourceFile, char *pszTargetFile);


#ifdef __cplusplus
}
#endif

#endif // __BACKUP_VM_PSCP_API_H_ 1