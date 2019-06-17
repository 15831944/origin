/** ===========================================================================
 ** Copyright (c) 2007 WaveTop Information Corp. All rights reserved.
 **
 ** The mirror system
 **
 ** ===========================================================================
 */
/** 
 * File:         modules\server2\mioratrans.h
 * Description:  Description: Implemention file write buff
 */

#ifndef _ORACLE_FD_BUFF_H_
#define _ORACLE_FD_BUFF_H_ 1

/* The stream buffer */
struct fdbuff {
    PRFileDesc* pFD;
    char *outptr;
    int outcnt;
    int outsize;
};

/**
  * [new fdbuff FUNCTION] new fdbuff
  *
  * pFD          [IN] A file descriptor
  * buffsize     [IN] buff size
  *
  * [RETURN] If new successful return a pointer to a dynamically allocated fdbuff,
  *          otherwise return NULL.
  */
fdbuff * NewFDBuff(char *pszBuff, int buffsize);

/**
  * [close fdbuff FUNCTION] free memory
  *
  * pFDBuff          [IN] A fdbuff descriptor handle
  *
  * [RETURN] void.
  */
void    CloseFDBuff(fdbuff *pFDBuff);

/**
  * [new fdbuff FUNCTION] new fdbuff
  *
  * pFD          [IN] A file descriptor
  * pFDBuff          [IN] A fdbuff descriptor handle
  *
  * [RETURN] If flush successful return WAVETOP_BACKUP_OK,
  *          otherwise return error code.
  */
PRInt32 SetFDBuff(fdbuff *pFDBuff, PRFileDesc* pFD);

/**
  * [flush fdbuff FUNCTION] write memory data to file
  *
  * pFDBuff          [IN] A fdbuff descriptor handle
  *
  * [RETURN] If flush successful return WAVETOP_BACKUP_OK,
  *          otherwise return error code.
  */
PRInt32 FlushFDBuff(fdbuff *pFDBuff);

/**
  * [write data to fdbuff FUNCTION] write data to fdbuff
  *
  * pFDBuff          [IN] A fdbuff descriptor handle
  * pszData          [IN] string data buff
  *
  * [RETURN] If write successful return WAVETOP_BACKUP_OK,
  *          otherwise return error code.
  */
PRInt32 WriteFDString(fdbuff *pFDBuff, const char *pszData);

/**
  * [write data to fdbuff FUNCTION] write data to fdbuff
  *
  * pFDBuff          [IN] A fdbuff descriptor handle
  * pszData          [IN] data buff
  * nBytes           [IN] data len
  *
  * [RETURN] If write successful return WAVETOP_BACKUP_OK,
  *          otherwise return error code.
  */
PRInt32 WriteFDBinary(fdbuff *pFDBuff, const unsigned char *pszData, PRInt32 nBytes);

/**
  * 需要获取现在缓冲区中需要写出去的字节数
  */
PRInt32 GetOutBytesOfFDBuff(fdbuff *pFDBuff);

#endif

