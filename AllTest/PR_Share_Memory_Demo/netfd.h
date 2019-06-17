#ifndef _NETFD_H_
#define _NETFD_H_

#include "public.h"


#ifdef WIN32
#ifndef API_EXPORT
#define API_EXPORT(p)  p
#endif
#else
#ifndef API_EXPORT 
#define API_EXPORT(p) extern p
#endif
#endif

#define WAVETOP_BACKUP_IOBUFFER_SIZE        8192


/* The stream buffer */
struct sbuff {
    PRFileDesc *fd;
    char inptr[WAVETOP_BACKUP_IOBUFFER_SIZE];
    int incnt;
    int inpos;
    int insize;
    char outptr[WAVETOP_BACKUP_IOBUFFER_SIZE];
    int outcnt;
    int outsize;
    int nInTimeOut;
    int nOutTimeOut;
    };
typedef struct sbuff sbuff;

int MiSafeStrncpy(char *pszDest, char *pszSrc, 
    unsigned long nDestsize);
API_EXPORT(sbuff *)     NewSBuff(PRFileDesc *fd, int nInTimeOut, int nOutTimeOut);
API_EXPORT(void)        CloseSBuff(sbuff *pSBuff);

API_EXPORT(PRInt32)     FlushSBuff(sbuff *pSBuff);
API_EXPORT(PRInt32)     WriteByte(sbuff *pSBuff, PRInt8 n);
API_EXPORT(PRInt32)     WriteShort(sbuff *pSBuff, PRInt16 n);
API_EXPORT(PRInt32)     WriteInt32(sbuff *pSBuff, PRInt32 n);
API_EXPORT(PRInt32)     WriteInt64(sbuff *pSBuff, PRInt64 n);
API_EXPORT(PRInt32)     WriteString(sbuff *pSBuff, const char *pszData);
API_EXPORT(PRInt32)     WriteBinary(sbuff *pSBuff, const unsigned char *pszData, 
    PRInt32 nBytes);
API_EXPORT(PRInt32)     WriteFormatData(sbuff *pSBuff, const char *pszFmt, ...);

API_EXPORT(PRInt32)     ReadByte(sbuff *pSBuff, PRInt8 *n);
API_EXPORT(PRInt32)     ReadShort(sbuff *pSBuff, PRInt16 *n);
API_EXPORT(PRInt32)     ReadInt32(sbuff *pSBuff, PRInt32 *n);
API_EXPORT(PRInt32)     ReadInt64(sbuff *pSBuff, PRInt64 *n);
API_EXPORT(PRInt32)     ReadData(sbuff *pSBuff, PRInt32 nBytes);
API_EXPORT(PRInt32)     ReadData2(sbuff *pSBuff);
API_EXPORT(PRInt32)     ReadDataNoBuff(sbuff *pSBuff, char *pszBuf, 
    PRInt32 nNeedBytes, PRInt32 *nReadBytes);
API_EXPORT(PRInt32)     ReadString(sbuff *pSBuff, char *pszBuf, 
    PRInt32 nNeedBytes, PRInt32 *nReadBytes);
API_EXPORT(PRInt32)     ReadBinary(sbuff *pSBuff, unsigned char *pszBuf, 
    PRInt32 nNeedBytes, PRInt32 *nReadBytes);
API_EXPORT(int) GetCurrentOSType(void);


#endif