//////////////////////////////////////////////////////////////////////////////
//	Copyright 2006 WaveTop Information Corp.
//
//	Capture socket i/o(Winsock) APIs of mssql process
//
//  

#ifndef __WAVETOP_SQLSERV_CAPTURE_H_
#define __WAVETOP_SQLSERV_CAPTURE_H_ 1

#include "capture.h"
#include <windows.h>
#include <stdio.h>
#include "libmssqllog.h"

class CCaptureSession {
public:
    CCaptureSession();
    ~CCaptureSession();

public:
    MiLogHandle m_LogHandle;
    UINT64 m_nCurNum;
    HANDLE m_hWriteHandle;
    HANDLE m_hClientHandle;

    //
    // Init wavetop log library.
    //
    int LogInit(void);

    //
    // Close wavetop log library.
    //
    int LogClose(void);

    //
    // Create a connection.
    // @[out]
    // Return 0, if successful. Otherwise, nonzero.
    //
    int CreateConnection(SOCKET sSock, int nType);

    //
    // Test a connection status.
    //
    int CCaptureSession::TestConnectionStatus(SOCKET sSock, LPWSABUF lpBuffers, 
                                    DWORD dwBufferCount, LPVOID *lpWTOA, DWORD *pnStatus);

    // 
    // Add a connection event, but no data.
    // @[out]
    // Return 0, if successful. Otherwise, nonzero.
    // lpWTOA - return the WaveTop overlapped.
    //
    int AddConnectionEvent(SOCKET sSock,
                           int nType,
                           unsigned char *pszBuf,
                           DWORD dwBufSize,
                           LPWSAOVERLAPPED lpOverlapped,
                           LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
                           LPVOID *lpWTOA,
                           DWORD dwID);

    // Get a overlapped result.
    // @[out]
    // Return 0, if successful. Otherwise, nonzero.
    // ppNode - return IO buffer slot.
    //
    int PostResultByOverlapped(LPVOID lpWTOA,
                               DWORD dwCompletionKey,
                               DWORD dwStatus, 
                               DWORD *pnTransfer,
                               DWORD dwID,
                               IONodeSt **ppNode);

    //
    // Close a connection when closesocket.
    //
    int CloseConnetion(SOCKET sSock, int nType);

    // 
    // Save a trampoline PROC.
    // @[in]
    // pProc -- the trampoline PROC.
    // @[out]
    // Return 0, if successful. Otherwise, nonzero.
    // 
    int AddTrampolineProc(char *pszModuleName, char *pszImpModuleName, char *pszFuncName,
            PROC *pTrampAddr, PROC *pFakeAddr);

    //
    // Travels all PROCs.
    // @[out]
    // pProc -- the trampoline PROC.
    // Return 0, if successful. Otherwise, nonzero.
    //
    int TravelFirst(TrampolineProcSt *pProc);
    int TravelNext(TrampolineProcSt *pProc);

private:
    int RecvOtherData(SOCKET sSock, char *pszBuf, int nSize);
    int CaptureSendData(HANDLE hIO, SOCKET sSock, char *pszIP, DWORD dwStatus,
        DWORD dwSequ, unsigned char *pszBuf, int nType, DWORD dwNumberOfBytesRecvd);
    PSECURITY_ATTRIBUTES GetNullACL(void);
    void CleanNullACL(void *sa);

private:
    bool m_bIsClose;
    PSECURITY_ATTRIBUTES m_sa;
    ConnSessionSt m_arSesions[WAVETOP_CAPTURE_MAX_SESSION];

    TrampolineProcSt m_arProcs[32];
    int m_nProcCnt;
    int m_nProcPos;
    int m_nProcCursor;
};

extern CCaptureSession g_CaptureSession;

/////////////////////////////////////////////////////////////////////////////
//
//  The export interfaces
//
/////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

/////////////////////////////////////////////////////////////////////////////
// Capture SQLServer process entry.
// @[in]
//
// @[out]
//   Return 0, if successful. Otherwise, return error code(windows).
//
__declspec(dllexport) DWORD WaveTopSQLServEntry(void);

/////////////////////////////////////////////////////////////////////////////
// Capture SQLServer process detach.
// @[in]
//
// @[out]
//   Return 0, if successful. Otherwise, return error code(windows).
//
__declspec(dllexport) DWORD WaveTopSQLServDetach(void);


#ifdef __cplusplus
}
#endif

#endif // __WAVETOP_SQLSERV_CAPTURE_H_ 1