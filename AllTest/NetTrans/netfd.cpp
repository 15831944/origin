/** =============================================================================
** Copyright (c) 2003 WaveTop Information Corp. All rights reserved.
**
** The backup system
**
** =============================================================================
*/

/** 
* File:        librsync/netfd.cpp
* Description: 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <time.h>
#include <winsock2.h>
#include <MSTcpIP.h> //LW add
#else
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <sys/timeb.h>
#include <sys/time.h>
#include <unistd.h>
#include <ctype.h>
#ifdef SOLARIS
#include <sys/filio.h>
#else   /* #ifdef SOLARIS */
#include <sys/ioctl.h>
#endif  /* #ifdef SOLARIS */
#endif  /* #ifdef WIN32 */
#include "wconfig.h"
#include "backup_proto.h"
#include "wnetfd.h"
#include "util.h"
#include "ft_pack.h"
#include "ft_config.h"
#include "openssl_pack.h"


#define IF_FT(p)  if ( p->ft != NULL ) {
#define END_FT   }

#if defined(WIN32)

#ifdef WIN32
#define HAVE_IO_OVERLAPPED 1
#endif

#if defined(HAVE_IO_COMP)
extern HANDLE g_hIOComp;
#endif


#ifdef WIN32
/* judge ip */
API_EXPORT(int) JudgeIp(const char *host)
{
	while ((*host == '.') || isdigit(*host))
		host++;

	return (*host == '\0');
}

/* IP rule parse. */
API_EXPORT(int) AllowIpParse(allowdeny *a, char *where)
{
	char *s;

	a->from = where;
	if ((s = strchr(where, '/'))) {
		struct in_addr mask;

		/* trample on where, we won't be using it any more */
		*s++ = '\0';
		if (!JudgeIp(where)
			|| (a->ip.net.s_addr = inet_addr(where)) == INADDR_NONE)
			goto FAILED;

		/* is_ip just tests if it matches [\d.]+ */
		if (!JudgeIp(s))
			goto FAILED;

		/* is it in /a.b.c.d form? */
		if (strchr(s, '.')) {
			mask.s_addr = inet_addr(s);
			if (mask.s_addr == INADDR_NONE)
				goto FAILED;
		}

		else {
			int i;
			/* assume it's in /nnn form */
			i = atoi(s);
			if (i > 32 || i <= 0)
				goto FAILED;
			mask.s_addr = 0xFFFFFFFFUL << (32 - i);
			mask.s_addr = htonl(mask.s_addr);
		}
		a->ip.mask = mask;
		a->ip.net.s_addr  = (a->ip.net.s_addr & mask.s_addr);   /* pjr - This fixes PR 4770 */
	}
	else if (isdigit(*where) && JudgeIp(where)) {
		/* legacy syntax for ip addrs: a.b.c. ==> a.b.c.0/24 for example */
		int shift;
		char *t;
		int octet;
		/* parse components */
		s = where;
		a->ip.net.s_addr = 0;
		a->ip.mask.s_addr = 0;
		shift = 24;
		while (*s) {
			t = s;
			if (!isdigit(*t))
				goto FAILED;

			while (isdigit(*t))
				++t;

			if (*t == '.')
				*t++ = 0;

			else if (*t)
				goto FAILED;

			if (shift < 0)
				goto FAILED;

			octet = atoi(s);
			if (octet < 0 || octet > 255)
				goto FAILED;

			a->ip.net.s_addr |= (unsigned int)octet << shift;
			a->ip.mask.s_addr |= 0xFFUL << shift;
			s = t;
			shift -= 8;
		}
		a->ip.net.s_addr = ntohl(a->ip.net.s_addr);
		a->ip.mask.s_addr = ntohl(a->ip.mask.s_addr);
	}
	else
		goto FAILED;

	return WAVETOP_BACKUP_OK;
FAILED:
	return WAVETOP_BACKUP_OPEN_FILE_ERROR;
}
#endif

/* compare ip */
API_EXPORT(int) find_allow(sockaddr *pPeerAddr, allowdeny *ap[])
{
	int i;

	for (i = 0; ap[i] != NULL; ++i) {

		if (ap[i]->ip.net.s_addr != INADDR_NONE
			&& (((struct sockaddr_in *)pPeerAddr)->sin_addr.s_addr
			& ap[i]->ip.mask.s_addr) == ap[i]->ip.net.s_addr)
			return WAVETOP_BACKUP_OK;
	}

	return WAVETOP_BACKUP_CONNECT_DOWN;
}

/*
* select() sometimes returns 1 even though the write will block. We must work around this.
*/

static int ap_sendwithtimeout(sbuff *sb, const char *buf, int len, int flags)
{
	int sock;
	int rc = WSAEWOULDBLOCK;
	WSAOVERLAPPED stOver;
	WSAEVENT evtArray[1];
	WSABUF saBuf;
	DWORD writBytes;
	DWORD transferBytes;
	DWORD opt;
	DWORD dwkey;

	IF_FT(sb)
		return sb->ft->Send((void*)buf, len);
	END_FT
		if (sb->ssl != NULL)
		{
			return sb->ssl->Send((void*)buf, len);
		}

		sock = sb->fd;

		/* If ap_sendwithtimeout is called with an invalid timeout
		* set a default timeout of 300 seconds. This hack is needed
		* to emulate the non-blocking send() that was removed in 
		* the previous patch to this function. Network servers
		* should never make network i/o calls w/o setting a timeout.
		* (doing otherwise opens a DoS attack exposure)
		*/
#if defined(HAVE_IO_COMP)    
		ZeroMemory(&stOver, sizeof(stOver));

		saBuf.buf = (char *)buf;
		saBuf.len = len;
		opt = 0; 
		writBytes = 0;

		if (WSASend(sock, &saBuf, 1, &writBytes, opt, &stOver, NULL) == SOCKET_ERROR) {
			rc = WSAGetLastError();
			if (rc != WSA_IO_PENDING) {
				rc = 0 - rc;
				goto FAILED;
			}
		}

		if (!GetQueuedCompletionStatus(g_hIOComp,
			&transferBytes, (LPDWORD)&dwkey, &pOver, (sb->nOutTimeOut * 1000))) {
				rc = WSAGetLastError();
				rc = 0 - rc;
				goto FAILED;
		}

		rc = transferBytes;

#elif defined(HAVE_IO_OVERLAPPED)
		ZeroMemory(&stOver, sizeof(stOver));
		stOver.hEvent = sb->hEvent;

		saBuf.buf = (char *)buf;
		saBuf.len = len;
		opt = 0; 
		writBytes = 0;

		if (WSASend(sock, &saBuf, 1, &writBytes, opt, &stOver, NULL) == SOCKET_ERROR) {
			rc = WSAGetLastError();
			if (rc != WSA_IO_PENDING) {
				rc = 0 - rc;
				goto FAILED;
			}
		}

		evtArray[0] = sb->hEvent;

		/* Wait for the overlapped I/O call to complete */
		if (WSAWaitForMultipleEvents(1, evtArray, FALSE, 
			(sb->nOutTimeOut * 1000), FALSE) == WAIT_TIMEOUT) {
				rc = 0 - WSA_WAIT_TIMEOUT;
				goto FAILED;
		}

		WSAGetOverlappedResult(sock, &stOver, &transferBytes, TRUE, &dwkey);
		rc = transferBytes;
#endif

		/* ioctlsocket(sock, FIONBIO, (u_long*)&iostate); */

FAILED:
		return rc;

}

static int ap_recvwithtimeout(sbuff *sb, char *buf, int len, int flags)
{

	int sock;
	int rc = WSAEWOULDBLOCK;
	WSAOVERLAPPED stOver;
	WSAEVENT evtArray[1];
	WSABUF saBuf;
	DWORD readBytes;
	DWORD transferBytes;
	DWORD dwkey;
	DWORD opt;

	IF_FT(sb)
		sb->ft->Recv(buf, len,(unsigned int*)&transferBytes);
	return transferBytes;
	END_FT
		if (sb->ssl != NULL)
		{
			sb->ssl->Recv(buf, len,(unsigned int*)&transferBytes);
			return transferBytes;
		}

		sock = sb->fd;

		/* If ap_recvwithtimeout is called with an invalid timeout
		* set a default timeout of 300 seconds. This hack is needed
		* to emulate the non-blocking recv() that was removed in 
		* the previous patch to this function. Network servers
		* should never make network i/o calls w/o setting a timeout.
		* (doing otherwise opens a DoS attack exposure)
		*/
#if defined(HAVE_IO_COMP)
		ZeroMemory(&stOver, sizeof(stOver));

		opt = 0; 
		readBytes = 0;
		saBuf.buf = buf;
		saBuf.len = len;

		if (WSARecv(sock, &saBuf, 1, &readBytes, &opt, &stOver, NULL) == SOCKET_ERROR) {
			rc = WSAGetLastError();
			if (rc != WSA_IO_PENDING) {
				rc = 0 - rc;
				goto FAILED;
			}
		}

		if (!GetQueuedCompletionStatus(g_hIOComp,
			&transferBytes, (LPDWORD)&dwkey, &pOver, (sb->nInTimeOut * 1000))) {
				rc = WSAGetLastError();
				rc = 0 - rc;
				goto FAILED;
		}

		rc = transferBytes;

#elif defined(HAVE_IO_OVERLAPPED)
		ZeroMemory(&stOver, sizeof(stOver));
		stOver.hEvent = sb->hEvent;
		saBuf.buf = buf;
		saBuf.len = len;

		opt = 0;
		readBytes = 0;

		if (WSARecv(sock, &saBuf, 1, &readBytes, &opt, &stOver, NULL) == SOCKET_ERROR) {
			rc = WSAGetLastError();
			if (rc != WSA_IO_PENDING) {
				rc = 0 - rc;
				goto FAILED;
			}
		}

		evtArray[0] = sb->hEvent;

		/* Wait for the overlapped I/O call to complete */
		if (WSAWaitForMultipleEvents(1, evtArray, FALSE, 
			(sb->nInTimeOut * 1000), FALSE) == WSA_WAIT_TIMEOUT) {
				rc = 0 - WAIT_TIMEOUT;
				goto FAILED;
		}
		WSAGetOverlappedResult(sock, &stOver, &transferBytes, TRUE, &dwkey);
		rc = transferBytes;
		if (transferBytes > 0)
		{
			sb->lastReadTimeStamp = time(NULL);
		}
#endif

		/* ioctlsocket(sock, FIONBIO, (u_long*)&iostate); */

FAILED:
		return rc;
}

#else /* END of WIN32. On the UNIX platform */
static int ap_recvwithtimeout(sbuff *sb, char *buf, int len, int flags)
{
	struct timeval tv;
	fd_set lfds;
	int rv;
	int rbytes;
	int sock;

	IF_FT(sb)
		sb->ft->Recv(buf, len,(unsigned int*)&rbytes);
	return rbytes;
	END_FT

		sock = sb->fd;

	do {
		tv.tv_sec = sb->nInTimeOut;
		tv.tv_usec = 0;
		FD_ZERO(&lfds);
		FD_SET(sock, &lfds);
		rv = select(sock + 1, &lfds, NULL, NULL, &tv);
	} while (rv < 0 && errno == EINTR);

	if (rv > 0) {
		rbytes = recv(sock, buf, len, flags);
		if (rbytes > 0)
		{
			sb->lastReadTimeStamp = time(NULL);
		}
	}
	else {
		rbytes = 0 - errno;
	}

	return rbytes;
}

static int ap_sendwithtimeout(sbuff *sb, const char *buf, int len, int flags)
{
	struct timeval tv;
	fd_set lfds;
	int rv;
	int sbytes;
	int sock;

	IF_FT(sb)
		return sb->ft->Send((void*)buf, len);
	END_FT

		sock = sb->fd;

	do {
		FD_ZERO(&lfds);
		FD_SET(sock, &lfds);
		tv.tv_sec = sb->nOutTimeOut;
		tv.tv_usec = 0;
		rv = select(sock + 1, NULL, &lfds, NULL, &tv);
	} while (rv < 0 && errno == EINTR);

	if (rv > 0) {
		sbytes = send(sock, buf, len, flags);
	}
	else {
		sbytes = 0 - errno;
	}

	return sbytes;
}
#endif

/* I/O for the network conntection */
API_EXPORT(sbuff *) NewSBuff(int fd, int nInTimeOut, int nOutTimeOut)
{
	sbuff *pSBuff;

	pSBuff = (sbuff *)malloc(sizeof(sbuff));
	if (pSBuff == NULL)
		return NULL;

	pSBuff->fd = fd;
	pSBuff->incnt = 0;
	pSBuff->outcnt = 0;
	pSBuff->insize = BACKUP_IOBUFFER_SIZE;
	pSBuff->outsize = BACKUP_IOBUFFER_SIZE;
	pSBuff->inpos = 0;
	pSBuff->nInTimeOut = nInTimeOut;
	pSBuff->nOutTimeOut = nOutTimeOut;
	pSBuff->ft = NULL;
	pSBuff->ssl = NULL;

#if defined(WIN32)
#if defined(HAVE_IO_OVERLAPPED)
	pSBuff->hEvent = WSACreateEvent();
#endif

	//经测，在收到最后一个报文，经过sKA_Settings.keepalivetime 毫秒后，发送Keep-Alive
	//如果远端socket正常，则会发送 Keep-Alive Ack Reply
	//如果服务端没有收到回应，则经过sKA_Settings.keepaliveinterval 毫秒后再次发送Keep-Alive
	//如果连续10次没有回应，则会发送RST，关闭socket，这样 WSAWaitForMultipleEvents 就会退出等待，
	//然后WSAGetOverlappedResult获得transferBytes为0，上层应用就会关闭连接，退出事务
	const int keepalivetime = 300;
	//设计每300秒发送keepalive,当无回应，则间隔30s再发。这样重复10次，差不多10min后给出宕机反馈
	DWORD dwError = 0L ;
	DWORD dwBytes;
	tcp_keepalive sKA_Settings = {0}, sReturned = {0} ;
	sKA_Settings.onoff = 1 ;
	sKA_Settings.keepalivetime = keepalivetime * 1000 ;
	sKA_Settings.keepaliveinterval = 30000 ; //ms Resend if No-Reply
	if (WSAIoctl(pSBuff->fd, SIO_KEEPALIVE_VALS, &sKA_Settings,
		sizeof(sKA_Settings), &sReturned, sizeof(sReturned), &dwBytes,
		NULL, NULL) != 0)
	{
		dwError = WSAGetLastError() ;
	}

#else
	{
		int keepAlive = 1; // 开启keepalive属性
		int keepIdle = 300; // 如该连接在300秒内没有任何数据往来,则进行探测 
		int keepInterval = 30; // 探测时发包的时间间隔为30 秒
		int keepCount = 10; // 探测尝试的次数.如果第1次探测包就收到响应了,则后2次的不再发.

		setsockopt(pSBuff->fd, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepAlive, sizeof(keepAlive));
		setsockopt(pSBuff->fd, SOL_TCP, TCP_KEEPIDLE, (void*)&keepIdle, sizeof(keepIdle));
		setsockopt(pSBuff->fd, SOL_TCP, TCP_KEEPINTVL, (void *)&keepInterval, sizeof(keepInterval));
		setsockopt(pSBuff->fd, SOL_TCP, TCP_KEEPCNT, (void *)&keepCount, sizeof(keepCount));
	}
#endif

	return pSBuff;
}

API_EXPORT(void) CloseSBuff(sbuff *pSBuff)
{
	IF_FT(pSBuff)
		delete pSBuff->ft;
	pSBuff->ft = NULL;
	END_FT	

		if (pSBuff->ssl != NULL)
		{
			pSBuff->ssl->UnInit(1);
			delete pSBuff->ssl;
			pSBuff->ssl = NULL;
		}


#if defined(WIN32) && defined(HAVE_IO_OVERLAPPED)
		CloseHandle(pSBuff->hEvent);
#endif
		free(pSBuff);
}

#ifdef WIN32
static LARGE_INTEGER nFreq;
static LARGE_INTEGER nEndTime;
static LARGE_INTEGER nCurTime;
static LARGE_INTEGER nBeginTime;
static double durationTime;
#else
static long long startTime=0;
static long long durationTime=0;
struct timeval tv;
long long getSystemTime() {
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_sec*1000000+tv.tv_usec;
}
#endif

static int nNetSpeedByte=0;
static int nNetMaxSpeed=0;
static PRInt32 nSend=0;
static long long standardTime;
static long long sleepTime=0;
static PRLock *g_lock;

API_EXPORT(int)   GetNetLimit(void)
{
	return nNetMaxSpeed;
}
API_EXPORT(int)  SetNetLimit(int nNetSpeed){
	nNetMaxSpeed = nNetSpeed;
	nNetSpeedByte = nNetSpeed*1024;
#ifdef WIN32
	QueryPerformanceFrequency(&nFreq);
#endif
	return nNetSpeed;
}

API_EXPORT(int) SetNetLimitLockForOnce(PRLock *pLock)
{
	g_lock = pLock;
	static const bool g_lock_inited = true;
	return 0;
}

API_EXPORT(PRInt32) FlushSBuff(sbuff *pSBuff)
{
	PRInt32 n;
	PRInt32 pos = 0;

	IF_FT(pSBuff)	
		return pSBuff->ft->Flush();
	END_FT

		if (pSBuff->ssl != NULL)
		{
			return pSBuff->ssl->Flush();
		}


#ifndef WIN32
		tv.tv_sec=0;
#endif

		if (pSBuff->outcnt <= 0)
			return WAVETOP_BACKUP_OK;

		if(nNetMaxSpeed>0){
			PR_Lock(g_lock);
			for (;;) {
				if(nSend==0){
#ifdef WIN32
					QueryPerformanceCounter(&nBeginTime);
#else
					startTime=getSystemTime();
#endif
				}
				n = ap_sendwithtimeout(pSBuff, pSBuff->outptr + pos, pSBuff->outcnt - pos, 0);
				if (n < 0) {
					PR_Unlock(g_lock);
					return WAVETOP_BACKUP_CONNECT_DOWN;
				}
				nSend += n;
				if(nSend>=nNetSpeedByte){
#ifdef WIN32
					QueryPerformanceCounter(&nEndTime);
					durationTime=(double)(nEndTime.QuadPart-nBeginTime.QuadPart) * 1000/(double)nFreq.QuadPart;
					if(durationTime < 1000){
						Sleep((int)(1000 - durationTime));
					}
#else
					durationTime = getSystemTime()-startTime;
					if(durationTime<1000000){
						tv.tv_usec = 1000000-durationTime;
						select(0,NULL,NULL,NULL,&tv);
					}
#endif	
					nSend = 0;
				}

				if ((pSBuff->outcnt - pos - n) > 0) {
					pos += n;
				}
				else {
					break;
				}
			}
			PR_Unlock(g_lock);
		}else{
			for (;;) {
				n = ap_sendwithtimeout(pSBuff, pSBuff->outptr + pos, pSBuff->outcnt - pos, 0);
				if (n < 0) {
					return WAVETOP_BACKUP_CONNECT_DOWN;
				}

				if ((pSBuff->outcnt - pos - n) > 0) {
					pos += n;
				}
				else {
					break;
				}
			}
		}

		pSBuff->outcnt = 0;
		return 0;
}

API_EXPORT(PRInt32) WriteByte(sbuff *pSBuff, PRInt8 n)
{
	IF_FT(pSBuff)	
		return pSBuff->ft->Send((void *)&n, 1);
	END_FT
		if (pSBuff->ssl != NULL)
		{
			return pSBuff->ssl->Send((void *)&n, 1);
		}

		if ((pSBuff->outsize - pSBuff->outcnt ) < sizeof(PRInt8)) {
			/* flush */
			FlushSBuff(pSBuff);
		}

		return WriteBinary(pSBuff, (const unsigned char *)&n, 1);
}

API_EXPORT(PRInt32) WriteShort(sbuff *pSBuff, PRInt16 n)
{
	unsigned char p[2];



	if ((pSBuff->outsize - pSBuff->outcnt ) < sizeof(PRInt16)) {
		/* flush */
		FlushSBuff(pSBuff);
	}

	p[0] = (n >> 8) & 0xff;
	p[1] = n & 0xff;

	IF_FT(pSBuff)
		return pSBuff->ft->Send((void *)p, 2);
	END_FT

		if (pSBuff->ssl != NULL)
		{
			p[0] = (n >> 8) & 0xff;
			p[1] = n & 0xff;
			return pSBuff->ssl->Send((void *)p, 2);
		}

		return WriteBinary(pSBuff, (const unsigned char *)p, 2);
}

API_EXPORT(PRInt32) WriteInt32(sbuff *pSBuff, PRInt32 n)
{
	unsigned char p[4];
	p[0] = (n >> 24) & 0xff;
	p[1] = (n >> 16) & 0xff;
	p[2] = (n >> 8) & 0xff;
	p[3] = n & 0xff;

	IF_FT(pSBuff)
		return pSBuff->ft->Send((void *)p, 4);
	END_FT
		if (pSBuff->ssl != NULL)
		{
			return pSBuff->ssl->Send((void *)p, 4);
		}
		if ((pSBuff->outsize - pSBuff->outcnt ) < sizeof(PRInt32)) {
			/* flush */
			FlushSBuff(pSBuff);
		}

		return WriteBinary(pSBuff, (const unsigned char *)p, 4);
}

API_EXPORT(PRInt32) WriteInt64(sbuff *pSBuff, PRInt64 n)
{
	unsigned char p[8];

	p[0] = (unsigned char)((n >> 56) & 0xff);
	p[1] = (unsigned char)((n >> 48) & 0xff);
	p[2] = (unsigned char)((n >> 40) & 0xff);
	p[3] = (unsigned char)((n >> 32) & 0xff);
	p[4] = (unsigned char)((n >> 24) & 0xff);
	p[5] = (unsigned char)((n >> 16) & 0xff);
	p[6] = (unsigned char)((n >> 8) & 0xff);
	p[7] = (unsigned char)(n & 0xff);

	IF_FT(pSBuff)		
		return pSBuff->ft->Send((void *)p, 8);
	END_FT
		if (pSBuff->ssl != NULL)
		{
			return pSBuff->ssl->Send((void *)p, 8);
		}


		if ((pSBuff->outsize - pSBuff->outcnt ) < sizeof(PRInt64)) {
			/* flush */
			FlushSBuff(pSBuff);
		}

		return WriteBinary(pSBuff, (const unsigned char *)p, 8);
}

API_EXPORT(PRInt32) WriteString(sbuff *pSBuff, const char *pszData)
{
	return WriteBinary(pSBuff, (const unsigned char *)pszData, strlen(pszData));
}

API_EXPORT(PRInt32) WriteBinary(sbuff *pSBuff, const unsigned char *pszData, PRInt32 nBytes)
{
	int pos = 0;
	int cursend = 0;

	if (nBytes <= 0) {
		return WAVETOP_BACKUP_OK;
	}

	IF_FT(pSBuff)	
		return pSBuff->ft->Send((void *)pszData, nBytes);
	END_FT
		if (pSBuff->ssl != NULL)
		{
			return pSBuff->ssl->Send((void *)pszData, nBytes);
		}


		if (pSBuff->outsize - pSBuff->outcnt <= nBytes) {
			for (;;) {
				cursend = (pSBuff->outsize - pSBuff->outcnt < nBytes - pos) ?
					pSBuff->outsize - pSBuff->outcnt :
				nBytes - pos;
				memcpy(pSBuff->outptr + pSBuff->outcnt, pszData + pos, cursend);
				pSBuff->outcnt += cursend;

				if (pSBuff->outsize == pSBuff->outcnt) {
					if (FlushSBuff(pSBuff) == WAVETOP_BACKUP_CONNECT_DOWN)
						return WAVETOP_BACKUP_CONNECT_DOWN;
				}

				if ((nBytes - pos - cursend) > 0) {
					pos += cursend;
				}
				else {
					return WAVETOP_BACKUP_OK;
				}
			}
		}
		else {
			memcpy(pSBuff->outptr + pSBuff->outcnt, pszData, nBytes);
			pSBuff->outcnt += nBytes;
		}

		return WAVETOP_BACKUP_OK;
}

API_EXPORT(PRInt32) ReadData(sbuff *pSBuff, PRInt32 nBytes)
{
	int l;

	if (nBytes > sizeof(pSBuff->inptr))
		return WAVETOP_BACKUP_CONNECT_DOWN;
	if (pSBuff->incnt >= nBytes)
		return WAVETOP_BACKUP_OK;
	if ((pSBuff->inpos >= nBytes) && (pSBuff->incnt > 0)) {
		memcpy(pSBuff->inptr, pSBuff->inptr + pSBuff->inpos, pSBuff->incnt);
		pSBuff->inpos = 0;
	}
	else if (pSBuff->incnt == 0) {
		pSBuff->inpos = 0;
	}

	while (1) {
		l = ap_recvwithtimeout(pSBuff, pSBuff->inptr + pSBuff->inpos + pSBuff->incnt, 
			pSBuff->insize - pSBuff->inpos - pSBuff->incnt, 0);
		if (l > 0) {
			pSBuff->incnt += l;
			if (pSBuff->incnt >= nBytes)
				return WAVETOP_BACKUP_OK;
		}
		else if (l == 0) {
			if (pSBuff->incnt < nBytes)
				return WAVETOP_BACKUP_CONNECT_DOWN;
			return WAVETOP_BACKUP_OK;
		}
		else {
			l = PR_GetError();
			return WAVETOP_BACKUP_CONNECT_DOWN;
		}
	}

	return WAVETOP_BACKUP_OK;
}

API_EXPORT(PRInt32) ReadData2(sbuff *pSBuff)
{
	int l;

	if ((pSBuff->inpos > 0) && (pSBuff->incnt > 0)) {
		memcpy(pSBuff->inptr, pSBuff->inptr + pSBuff->inpos, pSBuff->incnt);
		pSBuff->inpos = 0;
	}
	else if (pSBuff->incnt == 0) {
		pSBuff->inpos = 0;
	}
	l = ap_recvwithtimeout(pSBuff, pSBuff->inptr + pSBuff->incnt, 
		pSBuff->insize - pSBuff->incnt, 0);
	if (l > 0) {
		pSBuff->incnt += l;
		return WAVETOP_BACKUP_OK;
	}
	else if (l == 0) {
		return WAVETOP_BACKUP_CONNECT_DOWN;
	}
	else {
		l = PR_GetError();
		return WAVETOP_BACKUP_CONNECT_DOWN;
	}
}

API_EXPORT(PRInt32) ReadDataNoBuff(sbuff *pSBuff, char *pszBuf, PRInt32 nNeedBytes, PRInt32 *nReadBytes)
{ 
	int l;
	int o;

	*nReadBytes = 0;
	IF_FT(pSBuff)	
		return pSBuff->ft->Recv((void *)pszBuf, nNeedBytes,(unsigned int*)nReadBytes);
	END_FT

		if (pSBuff->ssl != NULL)
		{
			return pSBuff->ssl->Recv((void *)pszBuf, nNeedBytes,(unsigned int*)nReadBytes);
		}


		if (pSBuff->incnt > 0) {
			if (nNeedBytes > pSBuff->incnt) {
				memcpy(pszBuf, pSBuff->inptr + pSBuff->inpos, pSBuff->incnt);
				o = pSBuff->incnt;
				pSBuff->inpos = 0;
				pSBuff->incnt = 0;
				nNeedBytes -= o;
				*nReadBytes = o;
			}
			else {
				memcpy(pszBuf, pSBuff->inptr + pSBuff->inpos, nNeedBytes);
				pSBuff->inpos += nNeedBytes;
				pSBuff->incnt -= nNeedBytes;
				if (pSBuff->incnt == 0)
					pSBuff->inpos = 0;
				*nReadBytes = nNeedBytes;
				return WAVETOP_BACKUP_OK;
			}
		}
		else {
			o = 0;
		}

		while (nNeedBytes > 0) {
			if (nNeedBytes > 8192) {
				/*
				* if ((l = PR_Read(pSBuff->fd, pszBuf + o, 8192)) <= 0)
				*
				* if ((l = PR_Recv(pSBuff->fd, pszBuf + o, 8192, 0,
				*   PR_SecondsToInterval(pSBuff->nInTimeOut))) <= 0)
				*   return WAVETOP_BACKUP_CONNECT_DOWN;
				*/
				l = ap_recvwithtimeout(pSBuff, pszBuf + o, 8192, 0);
			}
			else {
				/*
				* if ((l = PR_Read(pSBuff->fd, pszBuf + o, nNeedBytes)) <= 0)
				*
				* if ((l = PR_Recv(pSBuff->fd, pszBuf + o, nNeedBytes, 0,
				*   PR_SecondsToInterval(pSBuff->nInTimeOut))) <= 0)
				*   return WAVETOP_BACKUP_CONNECT_DOWN;
				*/
				l = ap_recvwithtimeout(pSBuff, pszBuf + o, nNeedBytes, 0);
			}
			if (l == (0 - 258) || l == 0 ){
				FT_LOG(FT_LOG_ERR, "ReadDataNoBuff ap_recvwithtimeout,return len:%d,last errno:%d,nNeedBytes:%d,offset:%d",l,errno,nNeedBytes,o);
				return FT_ERR_TIMEOUT;
			}
			else if (l < 0){
				FT_LOG(FT_LOG_ERR, "ReadDataNoBuff ap_recvwithtimeout,return len:%d,last errno:%d,nNeedBytes:%d,offset:%d",l,errno,nNeedBytes,o);
				return WAVETOP_BACKUP_CONNECT_DOWN;
			}
			nNeedBytes -= l;
			o += l;
			(*nReadBytes) += l;
		}
		return WAVETOP_BACKUP_OK;
}

API_EXPORT(PRInt32) ReadByte(sbuff *pSBuff, PRInt8 *n)
{

	IF_FT(pSBuff)
		unsigned char s[1];
	unsigned int recvd;
	if (pSBuff->ft->Recv(s,1,&recvd) == FT_OK){
		*n = s[0];
		return WAVETOP_BACKUP_OK;
	}
	else
		return WAVETOP_BACKUP_CONNECT_DOWN;
	END_FT

		if (pSBuff->ssl != NULL)
		{
			unsigned char s[1];
			unsigned int recvd;

			if(0 == pSBuff->ssl->Recv (s,1,&recvd)){
				*n = s[0];
				return WAVETOP_BACKUP_OK;
			}
			else{
				return WAVETOP_BACKUP_CONNECT_DOWN;
			}
		}

		if (pSBuff->incnt < sizeof(PRInt8)) {
			if (ReadData(pSBuff, sizeof(PRInt8)))
				return WAVETOP_BACKUP_CONNECT_DOWN;
		}

		unsigned char *p = ((unsigned char *)pSBuff->inptr) + pSBuff->inpos;
		pSBuff->inpos += sizeof(PRInt8);
		pSBuff->incnt -= sizeof(PRInt8);

		*n = *p;

		return WAVETOP_BACKUP_OK;
}

API_EXPORT(PRInt32) ReadShort(sbuff *pSBuff, PRInt16 *n)
{
	unsigned char *p;

	IF_FT(pSBuff)
		unsigned char s[2];
	unsigned int recvd;
	if (pSBuff->ft->Recv(s,2,&recvd) == FT_OK){
		*n = (s[0] << 8) + s[1];
		return WAVETOP_BACKUP_OK;
	}
	else
		return WAVETOP_BACKUP_CONNECT_DOWN;
	END_FT

		if (pSBuff->ssl != NULL)
		{
			unsigned char s[2];
			unsigned int recvd;

			if(0 == pSBuff->ssl->Recv ( s,2,&recvd)>0){
				*n = (s[0] << 8) + s[1];
				return WAVETOP_BACKUP_OK;
			}
			else{
				return WAVETOP_BACKUP_CONNECT_DOWN;
			}
		}


		if (pSBuff->incnt < sizeof(PRInt16)) {
			if (ReadData(pSBuff, sizeof(PRInt16)))
				return WAVETOP_BACKUP_CONNECT_DOWN;
		}

		p = ((unsigned char *)pSBuff->inptr) + pSBuff->inpos;
		pSBuff->inpos += sizeof(short);
		pSBuff->incnt -= sizeof(short);

		*n = (p[0] << 8) + p[1];
		return WAVETOP_BACKUP_OK;
}

API_EXPORT(PRInt32) ReadInt32(sbuff *pSBuff, PRInt32 *n)
{
	unsigned char *p;

	IF_FT(pSBuff)
		unsigned char s[4];
	unsigned int recvd;
	if (pSBuff->ft->Recv(s,4,&recvd) == FT_OK){
		*n = (s[0] << 24) + (s[1] << 16) + (s[2] << 8) + s[3];
		return WAVETOP_BACKUP_OK;
	}
	else
		return WAVETOP_BACKUP_CONNECT_DOWN;
	END_FT

	if (pSBuff->ssl != NULL)
	{
		unsigned char s[4];
		unsigned int recvd;

		if(0 == pSBuff->ssl->Recv (s,4,&recvd)){
			*n = (s[0] << 24) + (s[1] << 16) + (s[2] << 8) + s[3];
			return WAVETOP_BACKUP_OK;
		}
		else{
			return WAVETOP_BACKUP_CONNECT_DOWN + 1;
		}
	}

	if (pSBuff->incnt < sizeof(PRInt32)) {
		if (ReadData(pSBuff, sizeof(PRInt32)))
			return WAVETOP_BACKUP_CONNECT_DOWN + 2;
	}

	p = ((unsigned char *)pSBuff->inptr) + pSBuff->inpos;
	pSBuff->inpos += sizeof(PRInt32);
	pSBuff->incnt -= sizeof(PRInt32);

	*n = (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3];
	return WAVETOP_BACKUP_OK;
}

API_EXPORT(PRInt32) ReadInt64(sbuff *pSBuff, PRInt64 *n)
{
	unsigned char *p;
	PRUint32 h;
	PRUint32 l;
	PRInt64  u;

	IF_FT(pSBuff)
		unsigned char s[8];
	p = s;
	unsigned int recvd;
	if (pSBuff->ft->Recv(p,8,&recvd) == FT_OK){
		h = (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3];
		/* low 32-bit */
		l = (p[4] << 24) + (p[5] << 16) + (p[6] << 8) + p[7];
		LL_UI2L(u, h);
		LL_SHL(*n, u, 32);
		LL_UI2L(u, l);
		LL_ADD(*n, *n, u);
		return WAVETOP_BACKUP_OK;
	}
	else
		return WAVETOP_BACKUP_CONNECT_DOWN;
	END_FT
		if (pSBuff->ssl != NULL)
		{
			unsigned char s[8];
			p = s;
			unsigned int recvd;

			if(0== pSBuff->ssl->Recv (p,8,&recvd)){
				h = (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3];
				/* low 32-bit */
				l = (p[4] << 24) + (p[5] << 16) + (p[6] << 8) + p[7];
				LL_UI2L(u, h);
				LL_SHL(*n, u, 32);
				LL_UI2L(u, l);
				LL_ADD(*n, *n, u);
				return WAVETOP_BACKUP_OK;
			}
			else{
				return WAVETOP_BACKUP_CONNECT_DOWN;
			}
		}

		if (pSBuff->incnt < sizeof(PRInt64)) {
			if (ReadData(pSBuff, sizeof(PRInt64)))
				return WAVETOP_BACKUP_CONNECT_DOWN;
		}

		p = ((unsigned char *)pSBuff->inptr) + pSBuff->inpos;
		pSBuff->inpos += sizeof(PRInt64);
		pSBuff->incnt -= sizeof(PRInt64);

		/* high 32-bit */
		h = (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3];
		/* low 32-bit */
		l = (p[4] << 24) + (p[5] << 16) + (p[6] << 8) + p[7];
		LL_UI2L(u, h);
		LL_SHL(*n, u, 32);
		LL_UI2L(u, l);
		LL_ADD(*n, *n, u);
		return WAVETOP_BACKUP_OK;
}

API_EXPORT(PRInt32) ReadString(sbuff *pSBuff, char *pszBuf, PRInt32 nNeedBytes, PRInt32 *nReadBytes)
{
	return ReadDataNoBuff(pSBuff, pszBuf, nNeedBytes, nReadBytes);
}

API_EXPORT(PRInt32) ReadBinary(sbuff *pSBuff, unsigned char *pszBuf, PRInt32 nNeedBytes,
	PRInt32 *nReadBytes)
{
	return ReadDataNoBuff(pSBuff, (char *)pszBuf, nNeedBytes, nReadBytes);
}

/* Create a physical connection to the server */
API_EXPORT(PRInt32) NewNetfd(const char *pszSrvIp, int nPort, int nFlag)
{
	char *pszServer;
	SOCKET nSocket;
	hostent *pstHost;
	sockaddr_in saAddr;
	u_long nVal;
	int sendBufferSize;
	int recvBufferSize;
	int i;
#if defined(WIN32)
#if defined(HAVE_IO_COMP)
	HANDLE hIoComp;
#endif
	DWORD dwError;
#endif

	pszServer = (char *)pszSrvIp;
	if (!strcmp(pszServer, "localhost")) {
		pszServer = "127.0.0.1";    
	}

	for (i = 0; pszServer[i] != '\0'; i++) {
		if (!ap_isdigit(pszServer[i]) && pszServer[i] != '.')
			break;
	}

	/* domain name */
	if (pszServer[i] != '\0') {

		pstHost = gethostbyname(pszServer);
		/* Setup network connection. */
		if (pstHost == NULL || pstHost->h_addrtype != AF_INET 
			|| pstHost->h_addr_list[0] == NULL) {
				/* SLogErrorWrite(APLOG_MARK, APLOG_ERR, NULL, 
				*   "Create the listen socket (gethostbyname) %s failed",
				*   pszServer);
				*/
				return INVALID_SOCKET;
		}

		memcpy(&saAddr.sin_addr.s_addr, pstHost->h_addr_list[0], pstHost->h_length);
	}
	else { 
		/* IP address */
		saAddr.sin_addr.s_addr = inet_addr(pszServer);
	}
	saAddr.sin_port = htons(nPort);
	saAddr.sin_family = AF_INET;

	nSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (nSocket == INVALID_SOCKET) {
		/* SLogErrorWrite(APLOG_MARK, APLOG_ERR, 
		*   NULL, "Create socket address %s failed", pszServer);
		*/
		return INVALID_SOCKET;
	}

#ifdef WIN32
	nVal = 0;
	if (ioctlsocket(nSocket, FIONBIO, &nVal) == SOCKET_ERROR) {
		/* SLogErrorWrite(APLOG_MARK, APLOG_ERR, 
		*   NULL, "ioctlsocket (FIONBIO) failed");
		*/
		goto loser;
	}
#else
	nVal = 0;
	if (ioctl(nSocket, FIONBIO, &nVal) == SOCKET_ERROR) {
		goto loser;
	}
#endif

	sendBufferSize = 1<<16;
	if (setsockopt(nSocket, SOL_SOCKET, SO_SNDBUF, 
		(const char*)&sendBufferSize, sizeof(sendBufferSize)) != 0)
		goto loser;

	recvBufferSize = 1<<16;
	if (setsockopt(nSocket, SOL_SOCKET, SO_RCVBUF, 
		(const char*)&recvBufferSize, sizeof(recvBufferSize)) != 0)
		goto loser;

	if (connect(nSocket, (sockaddr*)&saAddr, sizeof(saAddr)) == SOCKET_ERROR) {
		/* SLogErrorWrite(APLOG_MARK, APLOG_ERR, 
		*   NULL, "connect address %s failed(%d)", pszServer, GetLastError());
		*/
#ifdef WIN32
		dwError = GetLastError();
#endif
		goto loser;
	}

#ifdef WIN32
	nVal = 0;
	if (ioctlsocket(nSocket, FIONBIO, &nVal) == SOCKET_ERROR) {
		/* SLogErrorWrite(APLOG_MARK, APLOG_ERR, 
		*   NULL, "ioctlsocket (FIONBIO) failed");
		*/
		goto loser;
	}
#else
	nVal = 0;
	if (ioctl(nSocket, FIONBIO, &nVal) == SOCKET_ERROR) {
		goto loser;
	}
#endif

#if defined(WIN32) && defined(HAVE_IO_COMP)
	/* Associate with completion port */
	hIoComp = CreateIoCompletionPort((HANDLE)nSocket, g_hIOComp, (DWORD)0, 0);
	if (hIoComp != g_hIOComp)
		goto loser;
#endif

	return nSocket;

loser:
#ifdef WIN32
	closesocket(nSocket);
#else
	close(nSocket);
#endif
	return INVALID_SOCKET;
}

API_EXPORT(void) CloseConnection(int nFD)
{
#ifdef WIN32
	//shutdown(nFD, SHUTDOWN_NORETRY);
	closesocket(nFD);
#else
	//shutdown(nFD, 2);
	close(nFD);
#endif

}

API_EXPORT(int)  FTInit(sbuff *pSBuff, int nEnable, int isServer) {

	const char* pszServerIP ;
	int nRc = 0;
	if (isServer) {
		FlushSBuff(pSBuff);
		nRc = ReadInt32(pSBuff, &nEnable);
	}
	else {
		WriteInt32(pSBuff, nEnable);
		nRc = FlushSBuff(pSBuff);
	}
	if (nRc)
		return nRc;

	if (nEnable) {
		FTransport* ft = new FTransport();
		int mode = isServer ? FT_SERVER : FT_CLIENT;
		if (nEnable == 1)
		{
			FT_LOG(FT_LOG_DEBUG,"FT init no block mode");
			mode |= FT_USE_NO_BLOCK;
		}
		if (FT_OK != ft->Init(pSBuff, mode)) {
			ft->UnInit(1);
			delete ft;
			//return FT_ERR_UNKNOWN;
		}
	}
	return FT_OK;
}

API_EXPORT(int)  FTUnInit(sbuff *pSBuff) {
	IF_FT(pSBuff)
		FT_LOG(FT_LOG_INFO,"FTUnInit In");
	pSBuff->ft->UnInit(0);
	delete pSBuff->ft;
	pSBuff->ft = NULL;
	END_FT
		return FT_OK;
}

API_EXPORT(int)          SSLInitAccept(sbuff *pSBuff){
	pSBuff->ssl = new openssl_pack();
	if (0 != pSBuff->ssl->InitAccept(pSBuff)) {
		pSBuff->ssl->UnInit(1);
		delete pSBuff->ssl;
		return -1;
	}
	return 0;
}
API_EXPORT(int)          SSLInitConnect(sbuff *pSBuff){
	pSBuff->ssl = new openssl_pack();
	if (0 != pSBuff->ssl->InitConnect(pSBuff)) {
		pSBuff->ssl->UnInit(1);
		delete pSBuff->ssl;
		return -1;
	}
	return 0;
}
API_EXPORT(int)          SSLUnInit(sbuff *pSBuff){
	if (pSBuff->ssl != NULL)
	{
		pSBuff->ssl->UnInit(1);
		delete pSBuff->ssl;
		pSBuff->ssl=NULL;
	}
	return 0;
}

