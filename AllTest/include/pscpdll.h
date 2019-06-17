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
 * 通过ssh连接Esxi和Esx服务器上传文件方法
 * @[in]
 * pszHost         - Esxi 服务器IP地址
 * pUser           - Esxi 服务器用户名
 * pPassword       - Esxi 服务器密码
 * nPort           - Esxi 服务器ssh端口
 * pSourceFileName - 上传的源文件名(本地路径 E:\data\test.txt)
 * pTargetFileName - 上传的目标文件名(远程路径 root@192.168.1.160:/vmfs/volumes/datastore1/test)
 * @[out]
 * 上传文件成功返回 0, 否则返回错误码。
 */
API_EXPORT(int) BkVmSShUnloadFile(char *pszHost, char *pszUser, char *pszPassword, int nPort, 
                                  char *pszSourceFile, char *pszTargetFile);

/*
 * 通过ssh连接Esxi和Esx服务器下载文件方法
 * @[in]
 * pszHost         - Esxi 服务器IP地址
 * pUser           - Esxi 服务器用户名
 * pPassword       - Esxi 服务器密码
 * nPort           - Esxi 服务器ssh端口
 * pSourceFileName - 下载的源文件名(远程路径 root@192.168.1.160:/vmfs/volumes/datastore1/test)
 * pTargetFileName - 下载的目标文件名 (本地路径 E:\data\)
 * @[out]
 * 下载文件成功返回 0, 否则返回错误码。
 */
API_EXPORT(int) BkVmSShDownloadFile(char *pszHost, char *pszUser, char *pszPassword, int nPort,
                                    char *pszSourceFile, char *pszTargetFile);


#ifdef __cplusplus
}
#endif

#endif // __BACKUP_VM_PSCP_API_H_ 1