#include "public.h"

#define  WAVETOP_BACKUP_OK 0
#define  WT_BK_ORACLE_CLIENT_TIMEOUT 1
#define WAVETOP_BACKUP_INTERNAL_ERROR 3


#define SLogErrorWrite(a,b,c,format,...) printf(format,##__VA_ARGS__)




//#define SERVICETEST


int BkSqlStartService(char* pszServName)
{
    SC_HANDLE schSCManager = NULL;
    SC_HANDLE schService   = NULL;
    SERVICE_STATUS status;
    DWORD dwOldCheckPoint  = NULL;
    DWORD dwStartTickCount = NULL;
    DWORD dwWaitTime = NULL;
    DWORD dwBytesNeeded = NULL;
    int   nError = WAVETOP_BACKUP_OK;
    
    schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (NULL == schSCManager) {
        nError = GetLastError();
        SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR, 
            NULL, "service manager open is failed. code:[%d]", nError);
        return nError;
    }
    
    schService = OpenService(schSCManager, pszServName, SC_MANAGER_ALL_ACCESS); 
    if (NULL == schService) {
        nError = GetLastError();
        if (nError == ERROR_SERVICE_DOES_NOT_EXIST) {                
            CloseServiceHandle(schService);
            SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR, 
                NULL, "open service is failed, service name:%s, : %d\n", pszServName, nError);        
        }
        else {
            SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR, 
                NULL, "open service is failed, schService");                    
        }  
        CloseServiceHandle(schSCManager);
        return nError;         
    } 
    
    /* Check the status in case the service is not stopped.*/
    if (!QueryServiceStatus(schService, &status)) {
        SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR,
            NULL, "QueryServiceStatus is failed");
        CloseServiceHandle(schSCManager);
        CloseServiceHandle(schService);
        return WAVETOP_BACKUP_INTERNAL_ERROR;
    }
    
    /* Check if the service is already running. It would be possible
    ** to stop the service here, but for simplicity this example just returns. 
    */
    if ((status.dwCurrentState != SERVICE_STOPPED ) && 
        (status.dwCurrentState != SERVICE_STOP_PENDING )) {
        CloseServiceHandle(schSCManager);
        CloseServiceHandle(schService);
        return WAVETOP_BACKUP_OK;
    }
    
    /* Wait for the service to stop before attempting to start it. */
    while (status.dwCurrentState == SERVICE_STOP_PENDING) {
        // Save the tick count and initial checkpoint.
        
        dwStartTickCount = GetTickCount();
        dwOldCheckPoint = status.dwCheckPoint;
        
        // Do not wait longer than the wait hint. A good interval is 
        // one-tenth of the wait hint but not less than 1 second  
        // and not more than 10 seconds. 
        dwWaitTime = status.dwWaitHint / 10;
        
        if( dwWaitTime < 1000 )
            dwWaitTime = 1000;
        else if ( dwWaitTime > 10000 )
            dwWaitTime = 10000;
        
        Sleep(dwWaitTime);
        // Check the status until the service is no longer stop pending. 
        
        if (!QueryServiceStatus(schService, &status)) {
            nError = GetLastError();
            SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR,
                NULL, "QueryServiceStatus failed (%d)\n", nError);
            CloseServiceHandle(schService); 
            CloseServiceHandle(schSCManager);
            return nError; 
        }
        
        if ( status.dwCheckPoint > dwOldCheckPoint ) {
            // Continue to wait and check.
            
            dwStartTickCount = GetTickCount();
            dwOldCheckPoint = status.dwCheckPoint;
        }
        else {
            if(GetTickCount()-dwStartTickCount > status.dwWaitHint) {
                printf("Timeout waiting for service to stop\n");
                CloseServiceHandle(schService); 
                CloseServiceHandle(schSCManager);
                return WAVETOP_BACKUP_INTERNAL_ERROR; 
            }
        }
    }
    
    /* Attempt to start the service. */
    if (!StartService(schService, 0, NULL) ) {
        /*SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR,
            NULL, "start service is failed");*/
        nError = WAVETOP_BACKUP_INTERNAL_ERROR;
    }

    /* Check the status until the service is no longer start pending. */
    if (!QueryServiceStatus(schService, &status)) {
        SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR,
            NULL, "QueryServiceStatus is failed");
        nError = WAVETOP_BACKUP_INTERNAL_ERROR;
    }
    
    /* Save the tick count and initial checkpoint. */
    dwStartTickCount  = GetTickCount();
    dwOldCheckPoint = status.dwCheckPoint;
    
    while (status.dwCurrentState == SERVICE_START_PENDING) {
        /* waiting time not less than 1 second and nor more than 10 seconds. */
        dwWaitTime = status.dwWaitHint / 10;
        
        if(dwWaitTime < 1000)
            dwWaitTime = 1000;
        if(dwWaitTime > 10000)
            dwWaitTime = 10000;
        
        Sleep(dwWaitTime);
        
        /*  Check the status again. */
        if (!QueryServiceStatus(schService, &status) ) {
            SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR,
                NULL, "QueryServiceStatus is failed");
            break;
        }
        if (status.dwCheckPoint > dwOldCheckPoint) {
            dwStartTickCount = GetTickCount();
            dwOldCheckPoint  = status.dwCheckPoint;
        }
        else {
            if (GetTickCount() - dwStartTickCount > status.dwWaitHint) {
                break;
            }
        }
    }
    /*  Determine whether the service is running. */
    if (status.dwCurrentState != SERVICE_RUNNING) {
        SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR, NULL, "Service not start");
        nError = WAVETOP_BACKUP_INTERNAL_ERROR;
    }else{
        nError = WAVETOP_BACKUP_OK;
    }
    
    CloseServiceHandle(schSCManager);
    CloseServiceHandle(schService);
    
    return nError;
}

int BkSqlStopService(char *pszServerName)
{
    SC_HANDLE schSCManager = NULL;
    SC_HANDLE schService   = NULL;
    SERVICE_STATUS status;
    DWORD dwOldCheckPoint = NULL;
    DWORD dwStartTickCount = NULL;
    DWORD dwWaitTime = NULL;
    DWORD dwBytesNeeded = NULL;
    DWORD dwTimeout  = WT_BK_ORACLE_CLIENT_TIMEOUT;
    char szSvcName[64] = {0};
    int   nError = WAVETOP_BACKUP_OK;
    
    schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (NULL == schSCManager) {
        nError = GetLastError();
        SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR, 
            NULL, "service manager open is failed, code:[%d].", nError);
        return nError;
    }      
    
    schService = OpenService(schSCManager, pszServerName, SC_MANAGER_ALL_ACCESS); 
    if(NULL == schService) {
        nError = GetLastError();
        if (nError == ERROR_SERVICE_DOES_NOT_EXIST) {   
            /* 服务不存在时,即认为服务停止 */
            CloseServiceHandle(schService);
            SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO|APLOG_ALERT, 
                NULL, "服务:%s不存在.", pszServerName);
            return WAVETOP_BACKUP_OK;          
        }
        else {
            SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR, 
                NULL, "open service is failed, schService:");
            CloseServiceHandle(schSCManager);
            return nError;              
        }        
    }
    
    if(!QueryServiceStatus(schService, &status)) {
        SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR,
            NULL, "QueryServiceStatus is failed", NULL);
        CloseServiceHandle(schSCManager);
        CloseServiceHandle(schService);
        return WAVETOP_BACKUP_INTERNAL_ERROR;
    }
    
    /* check the service if stopped */
    if (status.dwCurrentState == SERVICE_STOPPED) {  
        SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO|APLOG_INFO,
            NULL, "service is already stop", NULL);
        CloseServiceHandle(schSCManager);
        CloseServiceHandle(schService);
        return WAVETOP_BACKUP_OK;
    }
    
    /* If a stop is pending, wait for it. */
    while (status.dwCurrentState == SERVICE_STOP_PENDING) {
        SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO|APLOG_INFO,
            NULL, "service stop is pending...", NULL);
        
        Sleep(status.dwWaitHint);
        
        if(!QueryServiceStatus(schService, &status)) {
            SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR,
                NULL, "QueryServiceStatus is failed", NULL);
            CloseServiceHandle(schSCManager);
            CloseServiceHandle(schService);
            return WAVETOP_BACKUP_INTERNAL_ERROR;
        }
        
        /* if the service is stopped,return */
        if (status.dwCurrentState == SERVICE_STOPPED) {  
            SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO|APLOG_INFO,
                NULL, "service stopped is successfully", NULL);
            CloseServiceHandle(schSCManager);
            CloseServiceHandle(schService);
            return WAVETOP_BACKUP_OK;
        }
    }
    
    if (ControlService( 
        schService, 
        SERVICE_CONTROL_STOP, 
        (LPSERVICE_STATUS) &status) == FALSE ) {
        SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR,
            NULL, "ControlService failed %d", GetLastError());
        CloseServiceHandle(schSCManager);
        CloseServiceHandle(schService);
        return WAVETOP_BACKUP_INTERNAL_ERROR;
    }
    
    while (status.dwCurrentState != SERVICE_STOPPED) {
        Sleep(3000);
        dwStartTickCount = GetTickCount();
        /* Sleep(status.dwWaitHint); */
        
        if(!QueryServiceStatus(schService, &status)) {
            SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR,
                NULL, "QueryServiceStatus is failed", NULL);
            CloseServiceHandle(schSCManager);
            CloseServiceHandle(schService);
            return WAVETOP_BACKUP_INTERNAL_ERROR;
        }
        
        /* if service is stopped ,break while. */
        if (status.dwCurrentState == SERVICE_STOPPED) {
            break;
        }
        
        /* stop service is time out */
        if (GetTickCount() - dwStartTickCount > dwTimeout) {
            SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR,
                NULL, "waiting time is out", NULL);
            CloseServiceHandle(schSCManager);
            CloseServiceHandle(schService);
            return WAVETOP_BACKUP_INTERNAL_ERROR;
        }
    }    
    
    /* stop success */
    SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO|APLOG_INFO,
        NULL, "service stopped is successfully", NULL);
    
    return WAVETOP_BACKUP_OK;     
}


#ifdef SERVICETEST
void main()
{
    int nRC = BkSqlStartService("WaveTopBackup6Server");
    cout<<nRC<<endl;
}

#endif