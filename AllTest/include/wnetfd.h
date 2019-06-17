/** =============================================================================
 ** Copyright (c) 2003 WaveTop Information Corp. All rights reserved.
 **
 ** The backup system
 **
 ** =============================================================================
 */

#ifndef __WAVETOP_BACKUP_NETFD_H_
#define __WAVETOP_BACKUP_NETFD_H_  1

#include "nspr.h"
#include "wconfig.h"


/* The I/O buffer size */
#define BACKUP_IOBUFFER_SIZE            65536

class FTransport;
class openssl_pack;
typedef void* HANDLE;
/* The stream buffer */
struct sbuff {
	int fd;
	char inptr[BACKUP_IOBUFFER_SIZE];
	int incnt;
	int inpos;
	int insize;
	char outptr[BACKUP_IOBUFFER_SIZE];
	int outcnt;
	int outsize;
	int nInTimeOut;
	int nOutTimeOut;
#if defined(WIN32)
	HANDLE hEvent;
#endif
	FTransport* ft;
	openssl_pack*     ssl;

	time_t lastReadTimeStamp;
};
typedef struct sbuff sbuff;

/* IP address filtering (from mod_access.c in apache) */
typedef struct {
	char *from;
	struct {
		struct in_addr net;
		struct in_addr mask;
	} ip;
} allowdeny;

#ifdef __cplusplus
extern "C" {
#endif

	/* The network connection I/O functions */
	API_EXPORT(PRInt32)       NewNetfd(const char *pszSrvIp, int nPort, int nFlag);
	API_EXPORT(void)          CloseConnection(int nFD);
	API_EXPORT(sbuff *)       NewSBuff(int fd, int nInTimeOut, int nOutTimeOut);
	API_EXPORT(void)          CloseSBuff(sbuff *pSBuff);
	API_EXPORT(PRInt32)       FlushSBuff(sbuff *pSBuff);
	API_EXPORT(PRInt32)       WriteByte(sbuff *pSBuff, PRInt8 n);
	API_EXPORT(PRInt32)       WriteShort(sbuff *pSBuff, PRInt16 n);
	API_EXPORT(PRInt32)       WriteInt32(sbuff *pSBuff, PRInt32 n);
	API_EXPORT(PRInt32)       WriteInt64(sbuff *pSBuff, PRInt64 n);
	API_EXPORT(PRInt32)       WriteString(sbuff *pSBuff, const char *pszData);
	API_EXPORT(PRInt32)       WriteBinary(sbuff *pSBuff, const unsigned char *pszData, 
		PRInt32 nBytes);
	API_EXPORT(PRInt32)       ReadData(sbuff *pSBuff, PRInt32 nBytes);
	API_EXPORT(PRInt32)       ReadData2(sbuff *pSBuff);
	API_EXPORT(PRInt32)       ReadDataNoBuff(sbuff *pSBuff, char *pszBuf, PRInt32 nNeedBytes, 
		PRInt32 *nReadBytes);
	API_EXPORT(PRInt32)       ReadByte(sbuff *pSBuff, PRInt8 *n);
	API_EXPORT(PRInt32)       ReadShort(sbuff *pSBuff, PRInt16 *n);
	API_EXPORT(PRInt32)       ReadInt32(sbuff *pSBuff, PRInt32 *n);
	API_EXPORT(PRInt32)       ReadInt64(sbuff *pSBuff, PRInt64 *n);
	API_EXPORT(PRInt32)       ReadString(sbuff *pSBuff, char *pszBuf, PRInt32 
		nNeedBytes, PRInt32 *nReadBytes);
	API_EXPORT(PRInt32)       ReadBinary(sbuff *pSBuff, unsigned char *pszBuf, 
		PRInt32 nNeedBytes,
		PRInt32 *nReadBytes);
	// 流量控制
	API_EXPORT(int)   		 GetNetLimit(void);
 	API_EXPORT(int)      	 SetNetLimit(int nNetSpeed);
	API_EXPORT(int)          SetNetLimitLockForOnce(PRLock *pLock);


	/* Allow ip access functions. in the sgwork.cpp */
	API_EXPORT(int)          JudgeIp(const char *host);
	API_EXPORT(int)          AllowIpParse(allowdeny *a, char *where);
	API_EXPORT(int)          find_allow(sockaddr *pPeerAddr, allowdeny *ap[]);

	/*
       初始化 FC 传输
	   @pSBuff  ： 网络传输缓冲
	   @eEnable ： 0 -- 不启用FC通道传输数据
				   1 -- 备端采用非阻塞IO模式,小文件传输有优势
                   2 -- 备端采用阻塞IO模式，大文件传输有优势
	   @isServer ：0 -- 客户端
				   1 -- 服务端
	*/
    API_EXPORT(int)          FTInit(sbuff *pSBuff,int nEnable,int isServer);
    API_EXPORT(int)          FTUnInit(sbuff *pSBuff);

	

	API_EXPORT(int)          SSLInitAccept(sbuff *pSBuff);
	API_EXPORT(int)          SSLInitConnect(sbuff *pSBuff);
	API_EXPORT(int)          SSLUnInit(sbuff *pSBuff);

#ifdef __cplusplus
}
#endif

#endif /* __WAVETOP_BACKUP_NETFD_H_ */
