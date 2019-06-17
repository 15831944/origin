/** =============================================================================
 ** Copyright (c) 2006 WaveTop Information Corp. All rights reserved.
 **
 ** The Mirror system
 **
 ** =============================================================================
 */

#if defined(WIN32) || defined(WINDOWS)
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#endif
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "libmssqllog.h"
#include "logfile.h"

/*
 **************************************************************************
 * Base LogFile class
 **************************************************************************
 */

CLogFile::CLogFile() :
    m_nInPos(0),
    m_nInSize(0),
    m_nOutPos(0),
    m_nOutSize(0)
{
#if defined(WIN32) || defined(WINDOWS)
    m_hFile = INVALID_HANDLE_VALUE;
#else
    m_hFile = 0;
#endif
}

CLogFile::~CLogFile()
{
    Close();
}

int CLogFile::Open(const char *pszFileName, int nOptions, int nMode)
{
#if defined(WIN32) || defined(WINDOWS)
    int dwDesiredAccess;
    int dwCreationDisposition;
    WIN32_FIND_DATA stData;
    HANDLE hFind;

    if (strlen(pszFileName) >= sizeof(m_szFileName)) {
        LOG_TRACE1("The file name %s is over 1024!\n", pszFileName);
        return WAVETOP_BACKUP_FILENAME_TOO_LONG;
    }

    strcpy(m_szFileName, pszFileName);
    
    dwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
    if (nOptions & WAVETOP_LOGFILE_RDONLY)
        dwDesiredAccess = GENERIC_READ;
    else if (nOptions & WAVETOP_LOGFILE_WRONLY)
        dwDesiredAccess = GENERIC_WRITE;
    else if (nOptions & WAVETOP_LOGFILE_RDWR)
        dwDesiredAccess = GENERIC_WRITE | GENERIC_READ;

    if (nOptions & WAVETOP_LOGFILE_CREATE_FILE)
        dwCreationDisposition = OPEN_ALWAYS;
    else
        dwCreationDisposition = OPEN_EXISTING; 

    if (dwCreationDisposition == OPEN_EXISTING) {
        hFind = FindFirstFile(pszFileName, &stData);
        if (hFind == INVALID_HANDLE_VALUE)
            return WAVETOP_BACKUP_FILE_NOT_EXIST;
        FindClose(hFind);
    }

    m_hFile = CreateFile(pszFileName, 
        dwDesiredAccess, 
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        dwCreationDisposition,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
    if (m_hFile == INVALID_HANDLE_VALUE) {
        LOG_TRACE1("Open file %s failed.\n", pszFileName);
        return WAVETOP_BACKUP_OPEN_FILE_ERROR;
    }
    return WAVETOP_BACKUP_OK;
#else
    int flags;
    struct stat buf;

    flags = O_RDWR;
    if (nOptions & WAVETOP_LOGFILE_RDONLY)
        flags = O_RDONLY;
    else if (nOptions & WAVETOP_LOGFILE_WRONLY)
        flags = O_WRONLY;
    else if (nOptions & WAVETOP_LOGFILE_RDWR)
        flags = O_RDWR;

    if (nOptions & WAVETOP_LOGFILE_CREATE_FILE)
        flags |= O_CREAT;
    else if (stat(pszFileName, &buf) != 0){
        return WAVETOP_BACKUP_FILE_NOT_EXIST;
    }

    m_hFile = open(pszFileName, flags, 0666);
    if (m_hFile == -1) {
        LOG_TRACE1("Open file %s failed.\n", pszFileName);
        return WAVETOP_BACKUP_OPEN_FILE_ERROR;
    }

    return WAVETOP_BACKUP_OK;
#endif
}

int CLogFile::Read(char *pszBuf, int nBufSize, unsigned long *pnBytesOfRead)
{
#if defined(WIN32) || defined(WINDOWS)
    BOOL bResult;
    unsigned long dwTransferBytes;

    *pnBytesOfRead = 0;

START:
    if (m_nInSize >= nBufSize) {
        memcpy(pszBuf + (*pnBytesOfRead), (char *)m_szInBuf + m_nInPos, nBufSize);
        m_nInSize        -= nBufSize;
        m_nInPos          = (m_nInSize == 0 ? 0 : (m_nInPos + nBufSize));
        (*pnBytesOfRead) += nBufSize;
        return WAVETOP_BACKUP_OK;
    }
    else if (m_nInSize > 0) {
        memcpy(pszBuf + (*pnBytesOfRead), (char *)m_szInBuf + m_nInPos, m_nInSize);
        nBufSize         -= m_nInSize;
        (*pnBytesOfRead) += m_nInSize;
        m_nInPos          = 0;
        m_nInSize         = 0;
    }

    if (nBufSize > 0) {
        bResult = ReadFile(m_hFile,
            (char *)m_szInBuf + m_nInPos,
            (sizeof(m_szInBuf) - m_nInPos - m_nInSize),
            &dwTransferBytes, NULL);
        if (!bResult) {
            if (GetLastError() == ERROR_HANDLE_EOF) {
                LOG_TRACE1("Read File %s to end\n", m_szFileName);
                return WAVETOP_BACKUP_END;
            }
            LOG_TRACE1("Read File Failed, FAILED Code:%d\n", GetLastError());
            return WAVETOP_BACKUP_OPEN_FILE_ERROR;
        }

        if (dwTransferBytes == 0)
            return WAVETOP_BACKUP_END;

        m_nInSize  += dwTransferBytes;

        goto START;
    }
    
    return WAVETOP_BACKUP_OK;
#else
    int nResult = 0;
    int nTransferBytes;

    *pnBytesOfRead = 0;

    for (;;) {
        if (m_nInSize >= nBufSize) {
            memcpy(pszBuf + (*pnBytesOfRead), (char *)m_szInBuf + m_nInPos, nBufSize);
            m_nInSize        -= nBufSize;
            m_nInPos          = (m_nInSize == 0 ? 0 : (m_nInPos + nBufSize));
            (*pnBytesOfRead) += nBufSize;
            nResult = WAVETOP_BACKUP_OK;
            break;
        }

        if (m_nInSize > 0) {
            memcpy(pszBuf + (*pnBytesOfRead), (char *)m_szInBuf + m_nInPos, m_nInSize);
            nBufSize         -= m_nInSize;
            (*pnBytesOfRead) += m_nInSize;
            m_nInPos          = 0;
            m_nInSize         = 0;
        }

        if (nBufSize <= 0)
            break;

        nTransferBytes = read(m_hFile,
            (char *)m_szInBuf + m_nInPos,
            sizeof(m_szInBuf) - m_nInPos - m_nInSize);
        if (nTransferBytes == 0) {
            LOG_TRACE1("Read File %s to end\n", m_szFileName);
            nResult = WAVETOP_BACKUP_END;
            break;
        }
        m_nInSize  += nTransferBytes;
    }
    
    return nResult;
#endif
}

int CLogFile::ReadNoBuf(char *pszBuf, int nBufSize, unsigned long *pnBytesOfRead)
{
#if defined(WIN32) || defined(WINDOWS)
    BOOL  bResult;
    unsigned long dwPos = 0;
    unsigned long dwTransferBytes;
    int   nRet;

    *pnBytesOfRead = 0;
    nRet           = WAVETOP_BACKUP_OK;

    if (m_nInSize >= nBufSize) {
        memcpy(pszBuf, (char *)m_szInBuf + m_nInPos, nBufSize);
        m_nInSize        -= nBufSize;
        m_nInPos          = (m_nInSize == 0 ? 0 : (m_nInPos + nBufSize));
        return WAVETOP_BACKUP_OK;
    }
    else if (m_nInSize > 0) {
        memcpy(pszBuf, (char *)m_szInBuf + m_nInPos, m_nInSize);
        nBufSize         -= m_nInSize;
        dwPos             = m_nInSize;
        m_nInPos          = 0;
        m_nInSize         = 0;
    }

    while (nBufSize > 0) {
        bResult = ReadFile(m_hFile,
            (char *)pszBuf + dwPos,
            nBufSize,
            &dwTransferBytes, NULL);
        if (!bResult) {
            if (GetLastError() == ERROR_HANDLE_EOF) {
                LOG_TRACE1("Read File %s to end\n", m_szFileName);
                nRet = WAVETOP_BACKUP_END;
                break;
            }
            LOG_TRACE1("Read File Failed, FAILED Code:%d\n", GetLastError());
            nRet = WAVETOP_BACKUP_OPEN_FILE_ERROR;
            break;
        }

        if (dwTransferBytes == 0) {
            nRet = WAVETOP_BACKUP_END;
            break;
        }

        nBufSize -= dwTransferBytes;
        dwPos    += dwTransferBytes;
    }

    if (nRet == WAVETOP_BACKUP_OK)
        *pnBytesOfRead = dwPos;
    
    return nRet;
#else
    int  nResult = 0;
    int  nPos = 0;
    int  nTransferBytes;

    *pnBytesOfRead = 0;

    if (m_nInSize >= nBufSize) {
        memcpy(pszBuf, (char *)m_szInBuf + m_nInPos, nBufSize);
        m_nInSize        -= nBufSize;
        m_nInPos          = (m_nInSize == 0 ? 0 : (m_nInPos + nBufSize));
        return WAVETOP_BACKUP_OK;
    }
    else if (m_nInSize > 0) {
        memcpy(pszBuf, (char *)m_szInBuf + m_nInPos, m_nInSize);
        nBufSize         -= m_nInSize;
        nPos              = m_nInSize;
        m_nInPos          = 0;
        m_nInSize         = 0;
    }

    while (nBufSize > 0) {
        nTransferBytes = read(m_hFile,
            (char *)pszBuf + nPos, nBufSize);
        if (nTransferBytes == 0) {
            LOG_TRACE1("Read File %s to end\n", m_szFileName);
            nResult = WAVETOP_BACKUP_END;
            break;
        }
        nBufSize -= nTransferBytes;
        nPos    += nTransferBytes;
    }

    *pnBytesOfRead = nPos;
    
    return nResult;
#endif
}

int CLogFile::Write(char *pszData, int nDataBytes)
{
#if defined(WIN32) || defined(WINDOWS)
    BOOL  bResult;
    unsigned long dwPos;
    unsigned long dwTransferBytes;
    int   nRet;

    dwPos = 0;
    nRet  = WAVETOP_BACKUP_OK;

    while (nDataBytes > 0) {
        bResult = WriteFile(m_hFile, pszData + dwPos,
                            nDataBytes, &dwTransferBytes, NULL);
        if (!bResult) {
            LOG_TRACE1("Write File Failed," 
                "FAILED Code:%d\n", GetLastError());
            nRet = WAVETOP_BACKUP_OPEN_FILE_ERROR;
            break;
        }

        dwPos      += dwTransferBytes;
        nDataBytes -= dwTransferBytes;
    }
    return nRet;
#else
    int  nResult = 0;
    int  nPos = 0;
    int  nTransferBytes;

    while (nDataBytes > 0) {
        nTransferBytes = write(m_hFile, pszData + nPos, nDataBytes);
        if (nTransferBytes == -1) {
            LOG_TRACE1("Write File Failed, FAILED Code:%d\n", errno);
            nResult = WAVETOP_BACKUP_OPEN_FILE_ERROR;
            break;
        }

        nPos       += nTransferBytes;
        nDataBytes -= nTransferBytes;
    }
    return nResult;
#endif
}

int CLogFile::Write()
{
    int  nResult = 0;

    nResult = Write((char *)m_szOutBuf + m_nOutPos, m_nOutSize);
    if (nResult == WAVETOP_BACKUP_OK)
        m_nOutSize = 0;

    return nResult;
}

int CLogFile::SetEndOfFile(long nPosition)
{
#if defined(WIN32) || defined(WINDOWS)
    BOOL nResult;

    nResult = ::SetEndOfFile(m_hFile);
    if (nResult == 0)
        return WAVETOP_BACKUP_INTERNAL_ERROR;
#else
    int nResult;

    nResult = (m_hFile, nPosition);
    if (nResult == -1)
        return WAVETOP_BACKUP_INTERNAL_ERROR;
#endif

    return 0;
}

int CLogFile::Seek(int nWay, long nPosition)
{
#if defined(WIN32) || defined(WINDOWS)
    unsigned long dwPos;

    switch (nWay) {
    case WAVETOP_LOGFILE_BEGIN:
        dwPos = SetFilePointer(m_hFile, nPosition, NULL, FILE_BEGIN);
        break;
    case WAVETOP_LOGFILE_CURRENT:
        dwPos = SetFilePointer(m_hFile, nPosition, NULL, FILE_CURRENT);
        break;
    case WAVETOP_LOGFILE_END:
        dwPos = SetFilePointer(m_hFile, nPosition, NULL, FILE_END);
        break;
    default:
        return WAVETOP_BACKUP_INVALID_SYNTAX;
    }

    ResetInBuf();

    if (dwPos == 0xFFFFFFFF)
        return WAVETOP_BACKUP_OPEN_FILE_ERROR;
    return WAVETOP_BACKUP_OK;
#else
    int nPos;

    switch (nWay) {
    case WAVETOP_LOGFILE_BEGIN:
        nPos = lseek(m_hFile, nPosition, SEEK_SET);
        break;
    case WAVETOP_LOGFILE_CURRENT:
        nPos = lseek(m_hFile, nPosition, SEEK_CUR);
        break;
    case WAVETOP_LOGFILE_END:
        nPos = lseek(m_hFile, nPosition, SEEK_END);
        break;
    default:
        return WAVETOP_BACKUP_INVALID_SYNTAX;
    }

    ResetInBuf();

    if (nPos == -1)
        return errno;

    return WAVETOP_BACKUP_OK;
#endif
}

int CLogFile::GetSize(unsigned long *pnSize)
{
#if defined(WIN32) || defined(WINDOWS)
    *pnSize = GetFileSize(m_hFile, NULL);
    return WAVETOP_BACKUP_OK;
#else
    int nResult = 0;
    struct stat buf;

    nResult = fstat(m_hFile, &buf);
    if (nResult == -1)
        return errno;
    *pnSize = buf.st_size;

    return WAVETOP_BACKUP_OK;
#endif
}

int CLogFile::Close()
{
#if defined(WIN32) || defined(WINDOWS)
    if (m_nOutSize > 0 && Write() != WAVETOP_BACKUP_OK) {
        LOG_TRACE1("Write failed when closing, FAILED Code:%d\n", GetLastError());
        return WAVETOP_BACKUP_OPEN_FILE_ERROR;
    }
    if (m_hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(m_hFile);
        m_hFile = INVALID_HANDLE_VALUE;
    }
    return WAVETOP_BACKUP_OK;
#else
    if (m_nOutSize > 0 && Write() != WAVETOP_BACKUP_OK) {
        LOG_TRACE1("Write failed when closing, FAILED Code:%d\n", errno);
        return WAVETOP_BACKUP_OPEN_FILE_ERROR;
    }
    if (m_hFile != 0 && m_hFile != -1) {
        close(m_hFile);
        m_hFile = 0;
    }
    return WAVETOP_BACKUP_OK;
#endif
}

int CLogFile::GetPosition(unsigned long *pnPos)
{
#if defined(WIN32) || defined(WINDOWS)
    *pnPos = SetFilePointer(m_hFile, 0, NULL, FILE_CURRENT);
    return WAVETOP_BACKUP_OK;
#else
    *pnPos = lseek(m_hFile, 0, SEEK_CUR);
    if (*pnPos == -1)
        return errno;

    return WAVETOP_BACKUP_OK;
#endif
}

bool CLogFile::FlushFile()
{
#if defined(WIN32) || defined(WINDOWS)
    return FlushFileBuffers(m_hFile);
#else
    /* 待实现 */
    return WAVETOP_BACKUP_OK;
#endif
}

void CLogFile::ResetInBuf()
{
    m_nInPos  = 0;
    m_nInSize = 0;
}

const char *CLogFile::GetName()
{
    return (const char *)m_szFileName;
}

/*
 **************************************************************************
 * IndexLogFile class
 **************************************************************************
 */

CIndexLogFile::CIndexLogFile() :
    m_nMoreSize(0),
    m_nStartSeq(0),
    m_nEndSeq(0),
    m_nRecordCount(0),
    m_nLastPos(0),
    m_nDataLastPos(0),
    m_bOldType(false)
{
}

CIndexLogFile::~CIndexLogFile()
{
}

int CIndexLogFile::CreateHeader(LogIndexHeaderSt &iHeader)
{
    unsigned long nTime = 0;
    time_t nTime64 = 0;
    time(&nTime64);
    nTime = (unsigned long)nTime64;
    XINT32(nTime, iHeader.szTime);

    m_nCreateTime = nTime;

    /* 
     * XINT64(nStartSeq,   iHeader.szStartSeq);
     * XINT64(nEndSeq,     iHeader.szEndSeq);
     */
    XINT32(sizeof(LogIndexHeaderSt) * 2, iHeader.szFileSize);

    return WriteHeader(iHeader);
}

int CIndexLogFile::WriteHeader(LogIndexHeaderSt &iHeader)
{
    int nRet;

    strcpy(iHeader.szMagic,    WAVETOP_LOGFILE_MAGIC);
    strcpy(iHeader.szVersion,  WAVETOP_LOGFILE_VERSTR);

    nRet = Seek(WAVETOP_LOGFILE_BEGIN, 0);
    nRet = Write((char *)&iHeader, sizeof(iHeader));
    return nRet;
}

int CIndexLogFile::WriteSecondHeader(LogIndexHeaderSt &iHeader)
{
    int nRet;
    
    strcpy(iHeader.szMagic,    WAVETOP_LOGFILE_MAGIC);
    strcpy(iHeader.szVersion,  WAVETOP_LOGFILE_VERSTR);
    
    nRet = Seek(WAVETOP_LOGFILE_BEGIN, sizeof(LogIndexHeaderSt));
    nRet = Write((char *)&iHeader, sizeof(iHeader));
    return nRet;
}

int CIndexLogFile::BkAppendRecord(IndexSt &Index)
{
    LogIndexHeaderSt iHeader = {0};
    unsigned long    nPos;
    int              nRet;
    PRUint64           nStartSeq;
    PRUint64           nEndSeq;
    unsigned long    nFileSize;
    unsigned long    nRecordCount;
    
    nPos = (sizeof(Index) * m_nRecordCount) + sizeof(LogIndexHeaderSt) * 2;
    
    assert(nPos == m_nLastPos);
    
    /* Moves to tail */
    nRet = Seek(WAVETOP_LOGFILE_BEGIN, nPos);
    if (nRet != WAVETOP_BACKUP_OK) {
        LOG_TRACE("Seek to tail failed when appending a record\n");
        goto END;
    }
    
    /* lsc:备份从客户端传过来时间
     * time((time_t *)&nTime);
     * XINT32(nTime, Index.szTime);
     */

    nRet = Write((char *)&Index, sizeof(Index));
    if (nRet != WAVETOP_BACKUP_OK) {
        LOG_TRACE("Write index failed when appending a record\n");
        goto END;
    }
    
    /* Moves to head */
    Seek(WAVETOP_LOGFILE_BEGIN, 0);
    
    nRecordCount = m_nRecordCount + 1;
    VINT64(nEndSeq, Index.szSeqNum);
    
    if (nRecordCount == 1)
        nStartSeq = nEndSeq;
    else
        nStartSeq = m_nStartSeq;
    
    nFileSize = nPos + sizeof(Index);
    
    /** Writes into header **/
    XINT64(nStartSeq,    iHeader.szStartSeq);
    XINT64(nEndSeq,      iHeader.szEndSeq);
    XINT32(nRecordCount, iHeader.szRecordCount);
    XINT32(nFileSize,    iHeader.szFileSize);
    nRet =   WriteHeader(iHeader);
    nRet =   WriteSecondHeader(iHeader);
    
    if (!(nEndSeq % WAVETOP_LOGFILE_FLUSH_COUNT)) {
        FlushFile();
    }
    
END:
    if (nRet == WAVETOP_BACKUP_OK) {
        m_nRecordCount   = nRecordCount;
        m_nStartSeq      = nStartSeq;
        m_nEndSeq        = nEndSeq;
        m_nLastPos       = nFileSize;
    }
    else {
        Seek(WAVETOP_LOGFILE_BEGIN, 0);
    }
    
    return nRet;
}

int CIndexLogFile::AppendRecord(IndexSt &Index)
{
    LogIndexHeaderSt iHeader = {0};
    unsigned long    nPos;
    int              nRet;
    PRUint64           nStartSeq;
    PRUint64           nEndSeq;
    unsigned long    nFileSize;
    unsigned long    nRecordCount;
    unsigned long    nTime = 0;
    time_t nTime64 = 0;
    nPos = (sizeof(Index) * m_nRecordCount) + sizeof(LogIndexHeaderSt) * 2;

    assert(nPos == m_nLastPos);
    
    /* Moves to tail */
    nRet = Seek(WAVETOP_LOGFILE_BEGIN, nPos);
    if (nRet != WAVETOP_BACKUP_OK) {
        LOG_TRACE("Seek to tail failed when appending a record\n");
        goto END;
    }

    time(&nTime64);
    nTime = (unsigned long) nTime64;
    XINT32(nTime, Index.szTime);
    
    nRet = Write((char *)&Index, sizeof(Index));
    if (nRet != WAVETOP_BACKUP_OK) {
        LOG_TRACE("Write index failed when appending a record\n");
        goto END;
    }

    /* Moves to head */
    Seek(WAVETOP_LOGFILE_BEGIN, 0);

    nRecordCount = m_nRecordCount + 1;
    VINT64(nEndSeq, Index.szSeqNum);

    if (nRecordCount == 1)
        nStartSeq = nEndSeq;
    else
        nStartSeq = m_nStartSeq;

    nFileSize = nPos + sizeof(Index);

    /** Writes into header **/
    XINT64(nStartSeq,    iHeader.szStartSeq);
    XINT64(nEndSeq,      iHeader.szEndSeq);
    XINT32(nRecordCount, iHeader.szRecordCount);
    XINT32(nFileSize,    iHeader.szFileSize);
    nRet =   WriteHeader(iHeader);
    nRet =   WriteSecondHeader(iHeader);

    if (!(nEndSeq % WAVETOP_LOGFILE_FLUSH_COUNT)) {
        FlushFile();
    }

END:
    if (nRet == WAVETOP_BACKUP_OK) {
        m_nRecordCount   = nRecordCount;
        m_nStartSeq      = nStartSeq;
        m_nEndSeq        = nEndSeq;
        m_nLastPos       = nFileSize;
    }
    else {
        Seek(WAVETOP_LOGFILE_BEGIN, 0);
    }

    return nRet;
}

int CIndexLogFile::GetHeader(LogIndexHeaderSt &iHeader)
{
    int nRet;
    unsigned long nBytes;
    unsigned long nPos;
    
    GetPosition(&nPos);

    Seek(WAVETOP_LOGFILE_BEGIN, 0);
    nRet = Read((char *)&iHeader, sizeof(iHeader), &nBytes);
    Seek(WAVETOP_LOGFILE_BEGIN, nPos);

    VINT64(m_nStartSeq,       iHeader.szStartSeq);
    VINT64(m_nEndSeq,         iHeader.szEndSeq);
    VINT32(m_nRecordCount,    iHeader.szRecordCount);
    VINT32(m_nLastPos,        iHeader.szFileSize);
    VINT32(m_nCreateTime,     iHeader.szTime);

    return WAVETOP_BACKUP_OK;
}

int CIndexLogFile::GetSecondHeader(LogIndexHeaderSt &iHeader)
{
    int nRet;
    unsigned long nBytes;
    unsigned long nPos;

    GetPosition(&nPos);

    Seek(WAVETOP_LOGFILE_BEGIN, sizeof(LogIndexHeaderSt));
    nRet = Read((char *)&iHeader, sizeof(iHeader), &nBytes);
    Seek(WAVETOP_LOGFILE_BEGIN, nPos);

    VINT64(m_nStartSeq,       iHeader.szStartSeq);
    VINT64(m_nEndSeq,         iHeader.szEndSeq);
    VINT32(m_nRecordCount,    iHeader.szRecordCount);
    VINT32(m_nLastPos,        iHeader.szFileSize);
    VINT32(m_nCreateTime,     iHeader.szTime);

    return WAVETOP_BACKUP_OK;
}

int CIndexLogFile::Search(PRUint64 nSeqNum, IndexSt &Index)
{
    int           nRet;
    IndexSt       iNode;
    PRUint64        nNodeSeq;
    unsigned long nPos;
    
    if (nSeqNum > m_nEndSeq || nSeqNum < m_nStartSeq)
        return WAVETOP_BACKUP_FILE_NOT_EXIST;

    Seek(WAVETOP_LOGFILE_BEGIN, sizeof(LogIndexHeaderSt) * 2);
    nPos = sizeof(LogIndexHeaderSt) * 2;

    for (;;) {
        nRet = Next(iNode);
        if (nRet == WAVETOP_BACKUP_END) {
            nRet = WAVETOP_BACKUP_FILE_NOT_EXIST;
            break;
        }
        else if (nRet != WAVETOP_BACKUP_OK)
            break;

        VINT64(nNodeSeq, iNode.szSeqNum);
        if (nNodeSeq == nSeqNum)
            break;
        nPos += sizeof(iNode);
    }

    /**
     * 已经移动了文件指针和缓冲区指针，则需要将文件指针移到该记录处，
     * 重置缓冲区指针为0.
     *
     * 这样，下一次Next时则重新进行读文件，而不从缓冲区获取.
     **/
    if (nRet == WAVETOP_BACKUP_OK) {
        ResetInBuf();
        Seek(WAVETOP_LOGFILE_BEGIN, nPos);
        memcpy(&Index, &iNode, sizeof(IndexSt));
    }

    return nRet;
}

unsigned long CIndexLogFile::GetTailPos()
{
    return m_nLastPos;
}

PRUint64 CIndexLogFile::GetStartSeq()
{
    return m_nStartSeq;
}

PRUint64 CIndexLogFile::GetEndSeq()
{
    return m_nEndSeq;
}

unsigned long CIndexLogFile::GetRecordCount()
{
    return m_nRecordCount;
}

void CIndexLogFile::SetRecordCount(unsigned long nRecordCont)
{
    m_nRecordCount = nRecordCont;
}

int CIndexLogFile::GetNo()
{
    return m_nNo;
}

void CIndexLogFile::SetNo(int nNo)
{
    m_nNo = nNo;
}

int CIndexLogFile::Open(const char *pszFileName, int nOptions, int nMode, long *pnPos)
{
    LogIndexHeaderSt  iHeader = {0};
    int               nRet;
    unsigned long     nBytes;
    unsigned long     nPackPos;
    unsigned long     nRecordCount;
    unsigned long     nRecSize = 0;
    unsigned long     nIndxPos = 0;
    unsigned long     nIndxSize = 0;
    bool              bDataFlag = false;
    IndexSt           Index;

    nRet = CLogFile::Open(pszFileName, nOptions, nMode);

    if (nRet != WAVETOP_BACKUP_OK)
        return nRet;

    Seek(WAVETOP_LOGFILE_BEGIN, 0);

    nRet = Read((char *)&iHeader, sizeof(iHeader), &nBytes);

    if (nRet == WAVETOP_BACKUP_END) {
        m_nStartSeq    = 0;
        m_nEndSeq      = 0;
        m_nRecordCount = 0;
        m_nLastPos     = sizeof(LogIndexHeaderSt) * 2;
        memset(&iHeader, 0, sizeof(iHeader));
        CreateHeader(iHeader);
        WriteSecondHeader(iHeader);
        if (pnPos != NULL)
            *pnPos = 0;
        return WAVETOP_BACKUP_OK;
    }

    if (nRet != WAVETOP_BACKUP_OK)
        goto END;

    /* 跳过校验步骤 */
    if (nOptions & WAVETOP_LOGFILE_WITHOUT_VALIDATE)
        goto END;

    /** 校验完整性 **/
    nPackPos     = 0;
    nRecordCount = 0;

    Seek(WAVETOP_LOGFILE_BEGIN, sizeof(LogIndexHeaderSt) * 2);

    for (;;) {
        nRet = Next(Index);
        if (nRet != WAVETOP_BACKUP_OK)
            break;
        
        nPackPos += sizeof(Index);
        VINT32(m_nDataLastPos, Index.szPos);
        VINT32(nRecSize, Index.szSize);
        nRecordCount++;
    }

    if (nRet != WAVETOP_BACKUP_OK && nRet != WAVETOP_BACKUP_END
        && nRet != WAVETOP_BACKUP_FILE_NOT_INTGRETY)
        goto END;

    VINT64(m_nStartSeq,    iHeader.szStartSeq);
    VINT64(m_nEndSeq,      iHeader.szEndSeq);
    VINT32(m_nRecordCount, iHeader.szRecordCount);
    VINT32(m_nLastPos,     iHeader.szFileSize);

    /* used to regulate data file. 4:数据文件记录头 */
    if (NULL != pnPos && 0 != nRecordCount)
        *pnPos = m_nDataLastPos + nRecSize + 4;

    /* get nPos: place which need truncating in data file */
    if (nRet == WAVETOP_BACKUP_FILE_NOT_INTGRETY || nRecordCount > m_nRecordCount) {
        /* truncate index file and update index head */
        nIndxPos = sizeof(LogIndexHeaderSt) * 2 + sizeof(IndexSt) * nRecordCount;
        nRet = Seek(WAVETOP_LOGFILE_BEGIN, nIndxPos);
        if (nRet != WAVETOP_BACKUP_OK)
            goto END;
        
        GetSize(&nIndxSize);
        if (nIndxSize != nIndxPos) {
            nRet = SetEndOfFile(nIndxPos);
            if (nRet != WAVETOP_BACKUP_OK)
                goto END;
        }   
        bDataFlag = true;
        Seek(WAVETOP_LOGFILE_BEGIN, sizeof(LogIndexHeaderSt) * 2);
    }

    if ((nRecordCount != m_nRecordCount) || 
        ((nPackPos + sizeof(LogIndexHeaderSt) * 2) != m_nLastPos)) {

        GetSecondHeader(iHeader);

        /* 异常情况导致索引文件头没写进去，更新头 */
        if ((nRecordCount != m_nRecordCount) || 
            ((nPackPos + sizeof(LogIndexHeaderSt) * 2) != m_nLastPos)) {
                /* 更新索引头 */
                m_nRecordCount = nRecordCount;
                m_nEndSeq = m_nStartSeq + m_nRecordCount - 1;
                if (!m_nRecordCount)
                    m_nEndSeq = m_nStartSeq;
                m_nLastPos = nPackPos + sizeof(LogIndexHeaderSt) * 2;
            
                XINT64(m_nStartSeq, iHeader.szStartSeq);
                XINT64(m_nEndSeq, iHeader.szEndSeq);
                XINT64(m_nRecordCount, iHeader.szRecordCount);
                XINT64(m_nLastPos, iHeader.szFileSize);
                WriteSecondHeader(iHeader);
                nRet =  WAVETOP_BACKUP_OK;              
        }

        WriteHeader(iHeader);
    }

    if (bDataFlag)
        nRet = WAVETOP_BACKUP_FILE_NOT_INTGRETY;

    if (nRet == WAVETOP_BACKUP_END)
        nRet =  WAVETOP_BACKUP_OK;

END:

    Seek(WAVETOP_LOGFILE_BEGIN, sizeof(LogIndexHeaderSt) * 2);
    ResetInBuf();
    return nRet;
}

int CIndexLogFile::Next(IndexSt &Index)
{
    int nRet;
    unsigned long nBytes;

    nRet = Read((char *)&Index, sizeof(Index), &nBytes);
    if (nRet != WAVETOP_BACKUP_OK) {
        if (nRet == WAVETOP_BACKUP_END && nBytes && nBytes != sizeof(Index))
            nRet = WAVETOP_BACKUP_FILE_NOT_INTGRETY;
        return nRet;
    }
    if (nBytes != sizeof(Index))
        return WAVETOP_BACKUP_END;
    return WAVETOP_BACKUP_OK;
}

int CIndexLogFile::Next2(IndexSt &Index)
{
    int nRet;
    unsigned long nBytes;

    nRet = ReadNoBuf((char *)&Index, sizeof(Index), &nBytes);
    if (nRet != WAVETOP_BACKUP_OK)
        return nRet;
    if (nBytes != sizeof(Index))
        return WAVETOP_BACKUP_END;
    return WAVETOP_BACKUP_OK;
}

int CIndexLogFile::Prev(IndexSt &Index)
{
    unsigned long nPos;
    unsigned long nBytes;
    int nRet;

    GetPosition(&nPos);
    if (nPos < sizeof(LogIndexHeaderSt) * 2)
        return WAVETOP_BACKUP_END;

    /*
     * nRet = Seek(WAVETOP_LOGFILE_CURRENT, 0 - sizeof(IndexSt));
     * if (nRet != WAVETOP_BACKUP_OK)
     *   return nRet;
     */

    nRet = ReadNoBuf((char *)&Index, sizeof(Index), &nBytes);

    if (nBytes != sizeof(Index))
        return WAVETOP_BACKUP_FILE_NOT_INTGRETY;

    /** Moves file pointer **/
    Seek(WAVETOP_LOGFILE_CURRENT, 0 - sizeof(IndexSt));

    Seek(WAVETOP_LOGFILE_CURRENT, 0 - sizeof(IndexSt));
    GetPosition(&nPos);
    if (nPos < sizeof(LogIndexHeaderSt) * 2) {
        First();
        return WAVETOP_BACKUP_END;
    }

    /* 
     * nRet = Read((char *)&Index, sizeof(Index), &nBytes);
     * if (nRet != WAVETOP_BACKUP_OK)
     *   return nRet;
     * if (nBytes != sizeof(Index))
     *   return WAVETOP_BACKUP_END;
     * Seek(WAVETOP_LOGFILE_CURRENT, 0 - sizeof(Index));
     *
     * return WAVETOP_BACKUP_OK;
     */
    return nRet;
}

int CIndexLogFile::Prev()
{
    int nRet;
    unsigned long nPos;

    GetPosition(&nPos);
    if (nPos == sizeof(LogIndexHeaderSt) * 2)
        return WAVETOP_BACKUP_END;

    nRet = Seek(WAVETOP_LOGFILE_CURRENT, 0 - sizeof(IndexSt));
    if (nRet != WAVETOP_BACKUP_OK)
        return WAVETOP_BACKUP_END;

    return WAVETOP_BACKUP_OK;
}

int CIndexLogFile::First()
{
    unsigned long nFileSize;

    GetSize(&nFileSize);

    if (nFileSize == 0)
        return WAVETOP_BACKUP_FILE_EMPTY;

    ResetInBuf();
    return Seek(WAVETOP_LOGFILE_BEGIN, sizeof(LogIndexHeaderSt) * 2);
}

int CIndexLogFile::Last()
{
    unsigned long nFileSize;
    unsigned long nPos;

    GetSize(&nFileSize);

    if (nFileSize == 0)
        return WAVETOP_BACKUP_FILE_EMPTY;

    ResetInBuf();

    nPos = sizeof(LogIndexHeaderSt) * 2 + (sizeof(IndexSt) * (m_nRecordCount - 1));
    return Seek(WAVETOP_LOGFILE_BEGIN, nPos);
}

/*
 **************************************************************************
 * DataLogFile class
 **************************************************************************
 */

CDataLogFile::CDataLogFile() :
    m_nRecordCount(0),
    m_nLastPos(0),
    m_nNo(0),
    m_pszDataBuf(NULL),
    m_nDataPos(0),
    m_nLoadPos(0),
    m_nLoadEndPos(0),
    m_bLoadData(false)  
{
}

CDataLogFile::~CDataLogFile()
{
	if(NULL != m_pszDataBuf) {
		free(m_pszDataBuf);				//释放内部缓冲区
		m_pszDataBuf = NULL;
	}

}

void CDataLogFile::SetNo(int nNo)
{
    m_nNo = nNo;
}

/** 
 * Write a record.
 * @[out]
 * 成功返回WAVETOP_BACKUP_OK；否则返回错误码. 以下方法同.
 **/
int CDataLogFile::WriteRecord(WTBUF *pBuffers, int nBuffCount)
{
    int             nItem;
    unsigned long   nPos;
    int             nRet;
    unsigned long   nSize;
    unsigned char   szSize[8];

    nPos   = m_nLastPos;
    nSize  = 0;
    
    /* Write size */
    nRet = Seek(WAVETOP_LOGFILE_BEGIN, nPos);

    for (nItem = 0; nItem < nBuffCount; nItem++) {
        nSize += pBuffers[nItem].dwBufSize;
    }
    XINT32(nSize, szSize);
    nRet = Write((char *)szSize, sizeof(unsigned long));
    if (nRet != WAVETOP_BACKUP_OK) {
        LOG_TRACE("Write size of data record failed.\n");
        return nRet;
    }
    nPos += sizeof(unsigned long);

    if (nSize > 0) {
        for (nItem = 0; nItem < nBuffCount; nItem++) {
            nRet = Write((char *)pBuffers[nItem].pszBuf, pBuffers[nItem].dwBufSize);
            if (nRet != WAVETOP_BACKUP_OK)
                break;
            nPos += pBuffers[nItem].dwBufSize;
        }
    }

    if (nRet == WAVETOP_BACKUP_OK) {
        m_nLastPos = nPos;
        m_nRecordCount++;
    }
    return nRet;
}

unsigned long CDataLogFile::GetTailPos()
{
    return m_nLastPos;
}

unsigned long CDataLogFile::GetRecordCount()
{
    return m_nRecordCount;
}

void CDataLogFile::SetRecordCount(unsigned long nRecordCount)
{
    m_nRecordCount = nRecordCount;
}

void CDataLogFile::SetTailPos(unsigned long nPos)
{
    m_nLastPos = nPos;
}

int CDataLogFile::GetNo()
{
    return m_nNo;
}

/* 探测一个记录的长度. 不移动文件指针. */
int CDataLogFile::PeekRecordSize(unsigned long *pnRecordSize)
{
    char            szBuf[4];
    unsigned long   nBytes;
    unsigned long   nPos;
    int             nRet;

    GetPosition(&nPos);

    nRet = Read(szBuf, 4, &nBytes);
    if (nRet != WAVETOP_BACKUP_OK)
        return nRet;

    Seek(WAVETOP_LOGFILE_BEGIN, nPos);

    if (nBytes != 4)
        return WAVETOP_BACKUP_END;

    VINT32((*pnRecordSize), szBuf);

    return WAVETOP_BACKUP_NOT_IMPLEMENTED;
}

int CDataLogFile::Open(const char *pszFileName, int nOptions, int nMode, long nDataPos)
{
    int            nRet;
    unsigned long  nBytes;
    unsigned long  nPos;
    unsigned long  nDataSize;
    unsigned long  nRecrSize;
    unsigned long  nReadBytes;
    unsigned long  nRecordCount;
    char           szBuf[8192];

    nPos         = 0;
    nRecordCount = 0;

    nRet = CLogFile::Open(pszFileName, nOptions, nMode);
    if (nRet != WAVETOP_BACKUP_OK)
        return nRet;

    nRet = Seek(WAVETOP_LOGFILE_BEGIN, 0);
    if (nRet != WAVETOP_BACKUP_OK)
        return nRet;

    GetSize(&nDataSize);
    if (nDataSize == 0)
        return WAVETOP_BACKUP_OK;

    /* nDataPos */
    if (-1 != nDataPos) {
        if (nDataPos < nDataSize) {
            nRet = Seek(WAVETOP_LOGFILE_BEGIN, nDataPos);
            if (nRet != WAVETOP_BACKUP_OK)
                goto EXIT;
            
            nRet = SetEndOfFile(nDataPos);
            if (nRet != WAVETOP_BACKUP_OK)
                goto EXIT;;
            SetTailPos(nDataPos);
        }
    }    
    
    nRet = Seek(WAVETOP_LOGFILE_BEGIN, 0);
    if (nRet != WAVETOP_BACKUP_OK)
        goto EXIT;

    /* 跳过校验步骤 */
    if (nOptions & WAVETOP_LOGFILE_WITHOUT_VALIDATE)
        goto EXIT;

    /** 校验完整性 **/
    for (;;) {
        /* The size of package data */
        nRet = Read(szBuf, sizeof(unsigned long), &nBytes);
        if (nRet != WAVETOP_BACKUP_OK)
            break;

        if (nBytes > 0 && nBytes < sizeof(unsigned long)) {
            nRet = WAVETOP_BACKUP_FILE_NOT_INTGRETY;
            break;
        }
        else if (nBytes == 0) {
            break;
        }

        VINT32(nRecrSize, szBuf);
        nDataSize = nRecrSize;

        /* The package data */
        for (; nDataSize > 0;) {
            nReadBytes = (nDataSize > sizeof(szBuf) ? sizeof(szBuf) : nDataSize);
            nRet = Read(szBuf, nReadBytes, &nBytes);
            if (nRet != WAVETOP_BACKUP_OK && nRet != WAVETOP_BACKUP_END)
                break;

            if (nBytes == 0) {
                nRet = WAVETOP_BACKUP_END;
                break;
            }

            nDataSize -= nBytes;
        }

        /* 数据不完整 */
        if (nRet == WAVETOP_BACKUP_END && nDataSize > 0) {
            nRet = WAVETOP_BACKUP_FILE_NOT_INTGRETY;
            LOG_TRACE1("The data file %s is damaged!\n", GetName());
        }

        if (nRet != WAVETOP_BACKUP_OK && nRet != WAVETOP_BACKUP_END)
            break;

        nPos += nRecrSize + sizeof(unsigned long);
        nRecordCount++;

        if (nRet == WAVETOP_BACKUP_END) {
            nRet =  WAVETOP_BACKUP_OK;
            break;
        }
    }

    if (nRet == WAVETOP_BACKUP_OK || nRet == WAVETOP_BACKUP_END) {
        m_nLastPos = nPos;
        m_nRecordCount = nRecordCount;
        nRet = WAVETOP_BACKUP_OK;
    }

EXIT:
    Seek(WAVETOP_LOGFILE_BEGIN, 0);
    ResetInBuf();

    return nRet;
}

