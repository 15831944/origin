/** =============================================================================
 ** Copyright (c) 2003 WaveTop Information Corp. All rights reserved.
 **
 ** The backup system
 **
 ** =============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nspr.h"


#include "netfd.h"

#ifdef WIN32
#include <process.h>

#include <windows.h>
#include <time.h> 
#else
#include <stdio.h>
#include <sys/timeb.h>
#include <sys/time.h>
#endif




/* I/O for the network conntection */
API_EXPORT(sbuff *) NewSBuff(PRFileDesc *fd, int nInTimeOut, int nOutTimeOut)
{
    sbuff *pSBuff;

    pSBuff = (sbuff *)malloc(sizeof(sbuff));
    if (pSBuff == NULL) {
        return NULL;
    }

    memset(pSBuff, 0, sizeof(sbuff));
    pSBuff->fd = fd;
    pSBuff->incnt = 0;
    pSBuff->outcnt = 0;
    pSBuff->insize = WAVETOP_BACKUP_IOBUFFER_SIZE;
    pSBuff->outsize = WAVETOP_BACKUP_IOBUFFER_SIZE;
    pSBuff->inpos = 0;

    pSBuff->nInTimeOut = nInTimeOut;
    pSBuff->nOutTimeOut = nOutTimeOut;

    return pSBuff;
}

API_EXPORT(void) CloseSBuff(sbuff *pSBuff)
{
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

static int nNetSpeedMaxByte=0;
static PRInt32 nSend=0;
static long long standardTime;
static long long sleepTime=0;
static PRLock *g_lock;

int MiSafeStrncpy(char *pszDest, char *pszSrc, 
    unsigned long nDestsize)
    {
    if (NULL == pszSrc) {
        pszDest[0] = '\0';
        return strlen(pszDest);
        }

    if (strlen(pszSrc) >= nDestsize) {
        strncpy(pszDest, pszSrc, nDestsize - 1);
        pszDest[nDestsize - 1] = '\0';
        }
    else {
        strcpy(pszDest, pszSrc);
        }

    return strlen(pszDest);
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
#ifndef WIN32
    tv.tv_sec=0;
#endif
	
    if (pSBuff->outcnt <= 0) {
        return WAVETOP_BACKUP_OK;
    }

    
    for (;;) {
        n = PR_Write(pSBuff->fd, pSBuff->outptr + pos, pSBuff->outcnt - pos);
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

    pSBuff->outcnt = 0;
    return 0;
}

API_EXPORT(PRInt32) WriteByte(sbuff *pSBuff, PRInt8 n)
{
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

    return WriteBinary(pSBuff, (const unsigned char *)p, 2);
}

API_EXPORT(PRInt32) WriteInt32(sbuff *pSBuff, PRInt32 n)
{
    unsigned char p[4];

    if ((pSBuff->outsize - pSBuff->outcnt ) < sizeof(PRInt32)) {
        /* flush */
        FlushSBuff(pSBuff);
    }

    p[0] = (n >> 24) & 0xff;
    p[1] = (n >> 16) & 0xff;
    p[2] = (n >> 8) & 0xff;
    p[3] = n & 0xff;

    return WriteBinary(pSBuff, (const unsigned char *)p, 4);
}

API_EXPORT(PRInt32) WriteInt64(sbuff *pSBuff, PRInt64 n)
{
    unsigned char p[8];

    if ((pSBuff->outsize - pSBuff->outcnt ) < sizeof(PRInt64)) {
        /* flush */
        FlushSBuff(pSBuff);
    }

    p[0] = (unsigned char)((n >> 56) & 0xff);
    p[1] = (unsigned char)((n >> 48) & 0xff);
    p[2] = (unsigned char)((n >> 40) & 0xff);
    p[3] = (unsigned char)((n >> 32) & 0xff);
    p[4] = (unsigned char)((n >> 24) & 0xff);
    p[5] = (unsigned char)((n >> 16) & 0xff);
    p[6] = (unsigned char)((n >> 8) & 0xff);
    p[7] = (unsigned char)(n & 0xff);

    return WriteBinary(pSBuff, (const unsigned char *)p, 8);
}

API_EXPORT(PRInt32) WriteString(sbuff *pSBuff, const char *pszData)
{
    return WriteBinary(pSBuff, (const unsigned char *)pszData, strlen(pszData));
}

API_EXPORT(PRInt32) WriteBinary(sbuff *pSBuff, const unsigned char *pszData, 
                                PRInt32 nBytes)
{
    int pos = 0;
    int cursend = 0;
    int n;
    
    if (nBytes <= 0) {
        return WAVETOP_BACKUP_OK;
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


API_EXPORT(PRInt32) WriteFormatData(sbuff *pSBuff, const char *pszFmt, ...)
{
    va_list args;
    const char *pszLst;
    char *pszData;
    char  cChar;
    PRInt32 nInt32;
    PRUint32 nUint32;
    PRInt64  nInt64;
    char szBuf[32];

    va_start(args, pszFmt);

    /* Check syntax */
    pszLst = pszFmt;
    while (*pszFmt) {
        if (*pszFmt == '%') {
            switch (*++pszFmt) {
            case 's': /* string */
                break;
            case 'c': /* a char */
                break;
            case 'd': /* 32 bits signed integer */
                break;
            case 'u': /* 32 bits unsigned integer */
                break;
            case 'I': /* 64 bits integer */
                break;
            default: /* Invalid format */
                return WAVETOP_BACKUP_INVALID_SYNTAX;
            }
        }
        pszFmt++;
    }
    
    pszFmt = pszLst;
    while (*pszFmt) {
        if (*pszFmt != '%') {
            WriteByte(pSBuff, (PRInt8)*pszFmt);
            pszFmt++;
        }
        else {
            switch (*++pszFmt) {
            case 's': /* string */
                pszData = va_arg(args, char *);
                WriteString(pSBuff, pszData);
                break;
            case 'c': /* a char */
                cChar = va_arg(args, char);
                WriteByte(pSBuff, (PRInt8)cChar);
                break;
            case 'd': /* 32 bits signed integer */
                nInt32 = va_arg(args, PRInt32);
                sprintf(szBuf, "%d", nInt32);
                WriteString(pSBuff, szBuf);
                break;
            case 'u': /* 32 bits unsigned integer */
                nUint32 = va_arg(args, PRUint32);
                sprintf(szBuf, "%u", nUint32);
                WriteString(pSBuff, szBuf);
                break;
            case 'I': /* XXX: 64 bits integer */
                nInt64 = va_arg(args, PRInt64);
                break;
            default: /* Invalid format */
                break;
            }
            pszFmt++;
        }        
    }

    va_end(args);
    return WAVETOP_BACKUP_OK;
}

API_EXPORT(PRInt32) ReadData(sbuff *pSBuff, PRInt32 nBytes)
{
    int l;

    if (nBytes > sizeof(pSBuff->inptr)) {
        return WAVETOP_BACKUP_CONNECT_DOWN;
    }

    if (pSBuff->incnt >= nBytes) {
        return WAVETOP_BACKUP_OK;
    }

    if ((pSBuff->inpos >= nBytes) && (pSBuff->incnt > 0)) {
        memcpy(pSBuff->inptr, pSBuff->inptr + pSBuff->inpos, pSBuff->incnt);
        pSBuff->inpos = 0;
    }
    else if (pSBuff->incnt == 0) {
        pSBuff->inpos = 0;
    }

    while (1) {
        l = PR_Recv(pSBuff->fd, pSBuff->inptr + pSBuff->inpos + pSBuff->incnt, 
             pSBuff->insize - pSBuff->inpos - pSBuff->incnt, 0, 
             PR_SecondsToInterval(pSBuff->nInTimeOut));
        /* 
         * l = PR_Read(pSBuff->fd, pSBuff->inptr + pSBuff->inpos + pSBuff->incnt, 
         *   pSBuff->insize - pSBuff->inpos - pSBuff->incnt);
         */
        if (l > 0) {
            pSBuff->incnt += l;
            if (pSBuff->incnt >= nBytes) {
                return WAVETOP_BACKUP_OK;
            }
        }
        else if (l == 0) {
            if (pSBuff->incnt < nBytes) {
                return WAVETOP_BACKUP_CONNECT_DOWN;
            }

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

    l = PR_Recv(pSBuff->fd, pSBuff->inptr + pSBuff->incnt, 
            pSBuff->insize - pSBuff->incnt, 0, 
            PR_SecondsToInterval(pSBuff->nInTimeOut));
    /*
     * l = PR_Read(pSBuff->fd, pSBuff->inptr + pSBuff->incnt, 
     *   pSBuff->insize - pSBuff->incnt);
     */
    if (l > 0) {
        pSBuff->incnt += l;
        return WAVETOP_BACKUP_OK;
    }
    else if (l == 0) {
        return WAVETOP_BACKUP_OK;
    }
    else {
        l = PR_GetError();
        return WAVETOP_BACKUP_CONNECT_DOWN;
    }
}

API_EXPORT(PRInt32) ReadDataNoBuff(sbuff *pSBuff, char *pszBuf, 
                                   PRInt32 nNeedBytes, PRInt32 *nReadBytes)
{
    int l;
    int o;

    *nReadBytes = 0;

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
            l = PR_Recv(pSBuff->fd, pszBuf + o, 8192, 0, 
                    PR_SecondsToInterval(pSBuff->nInTimeOut));
            if (l <= 0) {
                return WAVETOP_BACKUP_CONNECT_DOWN;
            }

            /*
             * if ((l = PR_Read(pSBuff->fd, pszBuf + o, 8192)) <= 0) {
             *   return WAVETOP_BACKUP_CONNECT_DOWN;
             * }
             */
        }
        else {
            l = PR_Recv(pSBuff->fd, pszBuf + o, nNeedBytes, 0, 
                    PR_SecondsToInterval(pSBuff->nInTimeOut));
            if (l <= 0) {
                return WAVETOP_BACKUP_CONNECT_DOWN;
            }

            /* 
             * if ((l = PR_Read(pSBuff->fd, pszBuf + o, nNeedBytes)) <= 0) {
             *   return WAVETOP_BACKUP_CONNECT_DOWN;
             * }
             */
        }

        nNeedBytes -= l;
        o += l;
        (*nReadBytes) += l;
    }
    return WAVETOP_BACKUP_OK;
}

API_EXPORT(PRInt32) ReadByte(sbuff *pSBuff, PRInt8 *n)
{
    if (pSBuff->incnt < sizeof(PRInt8)) {
        if (ReadData(pSBuff, sizeof(PRInt8))) {
            return WAVETOP_BACKUP_CONNECT_DOWN;
        }
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

    if (pSBuff->incnt < sizeof(PRInt16)) {
        if (ReadData(pSBuff, sizeof(PRInt16))) {
            return WAVETOP_BACKUP_CONNECT_DOWN;
        }
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

    if (pSBuff->incnt < sizeof(PRInt32)) {
        if (ReadData(pSBuff, sizeof(PRInt32))) {
            return WAVETOP_BACKUP_CONNECT_DOWN;
        }
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

    if (pSBuff->incnt < sizeof(PRInt64)) {
        if (ReadData(pSBuff, sizeof(PRInt64))) {
            return WAVETOP_BACKUP_CONNECT_DOWN;
        }
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


API_EXPORT(PRInt32) ReadString(sbuff *pSBuff, char *pszBuf, 
                               PRInt32 nNeedBytes, PRInt32 *nReadBytes)
{
    return ReadDataNoBuff(pSBuff, pszBuf, nNeedBytes, nReadBytes);
}

API_EXPORT(PRInt32) ReadBinary(sbuff *pSBuff, unsigned char *pszBuf, PRInt32 nNeedBytes,
                                   PRInt32 *nReadBytes)
{
    return ReadDataNoBuff(pSBuff, (char *)pszBuf, nNeedBytes, nReadBytes);
}

API_EXPORT(int) GetCurrentOSType(void)
{
#ifdef WIN32
    OSVERSIONINFO stOSInfo;

    stOSInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if (!GetVersionEx(&stOSInfo)) {
        if (stOSInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) {
            return BACKUP_OS_TYPE_WINNT;
        }
        else {
            return BACKUP_OS_TYPE_WIN9X;
        }
    }
    else {
        return BACKUP_OS_TYPE_WIN9X;
    }
#else
    return BACKUP_OS_TYPE_UNIX;
#endif  /* defined(WIN32) */
}

