#include "libmssqllogfunc.h"


int Init(char *pszInstance,char *pszDBName,MiLogHandle *hLog,char* pszLogPath)
{
	int nRC                 = WAVETOP_BACKUP_OK;
	char szTemp[512]        = { 0 };

	snprintf(szTemp, sizeof(szTemp), "%s_%s", 
		pszInstance, pszDBName);

	nRC = MiMSSqlLogStartEx(hLog, pszLogPath, 1, szTemp);

	return nRC;
}


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
       /* SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR, 
            NULL, "service manager open is failed. code:[%d]", nError);*/
		cout<<"service manager open is failed. code"<<nError<<endl;
        return nError;
    }
    
    schService = OpenService(schSCManager, pszServName, SC_MANAGER_ALL_ACCESS); 
    if (NULL == schService) {
        nError = GetLastError();
        if (nError == ERROR_SERVICE_DOES_NOT_EXIST) {                
            CloseServiceHandle(schService);
           /* SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR, 
                NULL, "open service is failed, service name:%s, : %d\n", pszServName, nError);   */ 
			cout<<"open service is failed, service name: "<<pszServName<<"error"<<nError<<endl;
        }
        else {
            /*SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR, 
                NULL, "open service is failed, schService"); */  
			cout<<"open service is failed, schService "<<endl;
        }  
        CloseServiceHandle(schSCManager);
        return nError;         
    } 
    
    /* Check the status in case the service is not stopped.*/
    if (!QueryServiceStatus(schService, &status)) {
       /* SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR,
            NULL, "QueryServiceStatus is failed");*/
		cout<<"QueryServiceStatus is failed"<<endl;
        CloseServiceHandle(schSCManager);
        CloseServiceHandle(schService);
        return WAVETOP_BACKUP_INTERNAL_ERROR;
    }
    
    /* Check if the service is already running. It would be possible
    ** to stop the service here, but for simplicity this example just returns. 
    */
    if ((status.dwCurrentState != SERVICE_STOPPED ) && 
        (status.dwCurrentState != SERVICE_STOP_PENDING )) {
        /*SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO|APLOG_ALERT,
            NULL, "can not start the service,it is already running");*/
			cout<<"can not start the service,it is already running\n";
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
            /*SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR,
                NULL, "QueryServiceStatus failed (%d)\n", nError);*/
			cout<<"QueryServiceStatus failed: "<<nError<<endl;
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
       /* SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR,
            NULL, "start service is failed");*/
		cout<<"start service is failed"<<endl;
        nError = WAVETOP_BACKUP_INTERNAL_ERROR;
    }
    else
        /*SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO|APLOG_INFO,
        NULL, "service start pending.");*/
		cout<<"service start pending"<<endl;
    
    /* Check the status until the service is no longer start pending. */
    if (!QueryServiceStatus(schService, &status)) {
        /*SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR,
            NULL, "QueryServiceStatus is failed");*/
		cout<<"QueryServiceStatus is failed"<<endl;
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
            /*SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR,
                NULL, "QueryServiceStatus is failed");*/
			cout<<"QuerySercieStatus is failed"<<endl;
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
    if (status.dwCurrentState == SERVICE_RUNNING) {
        
        /*SLogErrorWrite(APLOG_MARK, APLOG_INFO,
            NULL, "Service started successfully");*/
		cout<<"Service started successfully"<<endl;
    }
    else {
        
        /*SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR,
            NULL, "Service not start");*/
		cout<<"Service not start"<<endl;
        nError = WAVETOP_BACKUP_INTERNAL_ERROR;
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
        /*SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR, 
            NULL, "service manager open is failed, code:[%d].", nError);*/
        return nError;
    }      
    
    schService = OpenService(schSCManager, pszServerName, SC_MANAGER_ALL_ACCESS); 
    if(NULL == schService) {
        nError = GetLastError();
        if (nError == ERROR_SERVICE_DOES_NOT_EXIST) {   
            /* 服务不存在时,即认为服务停止 */
            CloseServiceHandle(schService);
            /*SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO|APLOG_ALERT, 
                NULL, "服务:%s不存在.", pszServerName);*/
            return WAVETOP_BACKUP_OK;          
        }
        else {
            /*SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR, 
                NULL, "open service is failed, schService:");*/
            CloseServiceHandle(schSCManager);
            return nError;              
        }        
    }
    
    if(!QueryServiceStatus(schService, &status)) {
        /*SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR,
            NULL, "QueryServiceStatus is failed", NULL);*/
        CloseServiceHandle(schSCManager);
        CloseServiceHandle(schService);
        return WAVETOP_BACKUP_INTERNAL_ERROR;
    }
    
    /* check the service if stopped */
    if (status.dwCurrentState == SERVICE_STOPPED) {  
        /*SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO|APLOG_INFO,
            NULL, "service is already stop", NULL);*/
        CloseServiceHandle(schSCManager);
        CloseServiceHandle(schService);
        return WAVETOP_BACKUP_OK;
    }
    
    /* If a stop is pending, wait for it. */
    while (status.dwCurrentState == SERVICE_STOP_PENDING) {
        /*SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO|APLOG_INFO,
            NULL, "service stop is pending...", NULL);*/
        
        Sleep(status.dwWaitHint);
        
        if(!QueryServiceStatus(schService, &status)) {
            /*SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR,
                NULL, "QueryServiceStatus is failed", NULL);*/
            CloseServiceHandle(schSCManager);
            CloseServiceHandle(schService);
            return WAVETOP_BACKUP_INTERNAL_ERROR;
        }
        
        /* if the service is stopped,return */
        if (status.dwCurrentState == SERVICE_STOPPED) {  
            /*SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO|APLOG_INFO,
                NULL, "service stopped is successfully", NULL);*/
            CloseServiceHandle(schSCManager);
            CloseServiceHandle(schService);
            return WAVETOP_BACKUP_OK;
        }
    }
    
    if (ControlService( 
        schService, 
        SERVICE_CONTROL_STOP, 
        (LPSERVICE_STATUS) &status) == FALSE ) {
        /*SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR,
            NULL, "ControlService failed %d", GetLastError());*/
        CloseServiceHandle(schSCManager);
        CloseServiceHandle(schService);
        return WAVETOP_BACKUP_INTERNAL_ERROR;
    }
    
    while (status.dwCurrentState != SERVICE_STOPPED) {
        Sleep(3000);
        dwStartTickCount = GetTickCount();
        /* Sleep(status.dwWaitHint); */
        
        if(!QueryServiceStatus(schService, &status)) {
            /*SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR,
                NULL, "QueryServiceStatus is failed", NULL);*/
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
            /*SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR,
                NULL, "waiting time is out", NULL);*/
            CloseServiceHandle(schSCManager);
            CloseServiceHandle(schService);
            return WAVETOP_BACKUP_INTERNAL_ERROR;
        }
    }    
    
    /* stop success */
  /*  SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO|APLOG_INFO,
        NULL, "service stopped is successfully", NULL);*/
    
    return WAVETOP_BACKUP_OK;     
}


int MiSqlGetIO(unsigned char **pszData, PRInt64 *pnSize,
	PRUint64 *pnCurrSeq, MiLogHandle LogHandle,
	unsigned char *pBuf,  PRInt64 *pnMaxSize)
{
	int                nRC              = WAVETOP_BACKUP_OK;
	int                nCount           = 0;
	PRInt64            nLastSize        = 0;
	unsigned long      nDataLen         = 0;
	unsigned long      nReadBytes       = 0;
	PRInt64            nPos             = 0;
	char              *pszBuff          = NULL;
	char              *pTemp           = NULL;
	unsigned char     *pData           = NULL;
	bool               bFull           = false;
	ExIndexSt          IndexSt;
	pool               *pPool          = NULL;



	IndexSt.nSeqNum = 0;
	nRC = MiMSSqlReadBufferFromLogNext(LogHandle, (char *)pBuf,
		MI_MSSQL2_BLOCK_SIZE, &nReadBytes, 0, &IndexSt);
	if (nRC != WAVETOP_BACKUP_OK &&
		nRC != WAVETOP_MIRROR_MSSQL2_LOG_MORE_WITH_LINE)
		goto END;

	if (nReadBytes > *pnMaxSize) {
		free(*pszData);

		*pszData = (unsigned char *)malloc(nReadBytes);
		if (NULL == *pszData) {
			/*SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO | APLOG_ERR, NULL, 
				"MiSqlGetIO: No memory.");*/
			nRC = WAVETOP_BACKUP_NO_MEMORY;
			goto END;
		}
	}

	memcpy(*pszData, pBuf, nReadBytes);
	nDataLen += nReadBytes;

	if (nRC == WAVETOP_BACKUP_OK) {
		*pnSize = nReadBytes;
		goto END;
	}
	nPos = nReadBytes;

	pPool = ap_make_sub_pool(NULL);
	if (pPool == NULL) {
		/*SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO | APLOG_ERR, NULL, 
			"MiSqlGetIO: ap_make_sub_pool failed.");*/
		nRC = WAVETOP_BACKUP_NO_MEMORY;
		goto END;
	}

	while(1) {
		pszBuff = (char *)ap_palloc(pPool, MI_MSSQL2_BLOCK_SIZE);
		//pszBuff = (char *)malloc(MI_MSSQL2_BLOCK_SIZE);
		if (NULL == pszBuff) {
			/*SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO | APLOG_ERR, NULL, 
				"MiSqlGetIO: No memory.");*/
			nRC = WAVETOP_BACKUP_NO_MEMORY;
			goto END;
		}

		nRC = MiMSSqlReadBufferFromLogNext(LogHandle, pszBuff,
			MI_MSSQL2_BLOCK_SIZE, &nReadBytes, 0, &IndexSt);
		if (nRC == WAVETOP_BACKUP_OK ||
			nRC == WAVETOP_MIRROR_MSSQL2_LOG_MORE_WITH_LINE) {
				nDataLen += nReadBytes;
				if (nDataLen > *pnMaxSize) {
					pData = (unsigned char *)malloc(nDataLen);
					if (NULL == pData) {
						/*SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO | APLOG_ERR, NULL, 
							"MiSqlGetIO: No memory.");*/
						nRC = WAVETOP_BACKUP_NO_MEMORY;
						goto END;
					}
					*pszData = (unsigned char *)realloc(*pszData, nDataLen);
					memcpy(pData, *pszData, nPos);
					free(*pszData);
					*pszData = pData;
				}

				memcpy(*pszData + nPos, pszBuff, nReadBytes);
		}

		if (nRC == WAVETOP_BACKUP_OK) {
			break;
		}
		else if (nRC == WAVETOP_MIRROR_MSSQL2_LOG_MORE_WITH_LINE) {
			nPos += nReadBytes;
		}
		else
			goto END;
	}

	*pnSize = nDataLen;

END:
	if (pszBuff != NULL)
		free(pszBuff);

	if (nDataLen > *pnMaxSize)
		*pnMaxSize = nDataLen;
	*pnCurrSeq = IndexSt.nSeqNum;

	return nRC;
}
bool CheckMstlInfo(WTBUF *pList, INT64 nLastOffset, MiSqlMstlSt *pMstl)
{
	MiSqlMstlSt *pTempMstl = NULL;
	WTBUF *pTempList       = NULL;
	unsigned char *pTmp    = NULL;
	UINT32 nLowOffset    = 0;
	INT64 ncurroffset      = 0;

	for (pTempMstl = pMstl; pTempMstl != NULL; pTempMstl = pTempMstl->pNext) {
		if (nLastOffset == (pTempMstl->nSectionBegin + pTempMstl->nSectionSize))
			return true;
	}

	for (pTempList = pList; pTempList != NULL; pTempList = pTempList->pNext) {
		if (pTempList->dwPos == 1) {

			pTmp = pTempList->pszBuf;
			pTmp += 6;
			memcpy(&ncurroffset, pTmp, sizeof(UINT32));
			pTmp += 4;
			memcpy(&nLowOffset, pTmp, sizeof(UINT32));
			(ncurroffset <<= 32) += nLowOffset;

			if (nLastOffset == ncurroffset)
				return true;
		}
	}

	return false;
}

/* 获取 IO 队列 
 * 退出函数需满足：3s内无增量数据，或者拼装内存大小大于配置文件设置的阈值
 */
int GetIOQueue(ap_pool *pNewPool, MiLogHandle LogHandle, unsigned char *pBuf, 
    BackUpSetSt *pTemp, MiMSSqlBakSlot *pBakSlotGet, char *pszCurrLSN)
{
    unsigned char    *pTmp           = NULL;
    char              szCurrLsn[24]  = { 0 };
    PRUint64          nLastSeqNum    = 0;
    PRUint64          nSeqNum        = 0;
    PRInt64           nDataSize      = 0;
    PRInt64           nMaxSize       = WAVETOP_MSSQL_LOGDATA_BUF;
    int               nRet           = WAVETOP_BACKUP_OK;
    INT16             nAction        = 0;
    INT16             nCurrFileid    = 0;
    unsigned char    *pData          = NULL;
    unsigned long     nCurrSize      = 0;
    unsigned long     nIoSize        = 0;
    unsigned long     nLenNum        = 0;
    UINT32            nLowOffset   = 0;
    INT64             ncurroffset    = 0;
    INT64             nFilteroffset  = 0;
	INT64			  nLastOffsetInBuf = -1;
    WTBUF            *pList          = NULL;
    int               nCount         = 0;
    int               nContinue      = 0;
    int               nFlag          = 0;
    int               nQuit          = 0;
    PRInt64           nTotSize       = 0;
    bool              bTaxis         = false;
    bool              bSetFileSize   = false;
    bool              bFilterFlag    = false;
	time_t			  intime		 = time(NULL);
	const time_t	  WAITTIME		 = 180; //3min
	bool			  willRemind	 = true;
    int        		  nWaitNextIO    = 0;

    pTemp->pIOQueue = NULL;

    pData = (unsigned char *)malloc(WAVETOP_MSSQL_LOGDATA_BUF);
    if (pData == NULL) {
        /*SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO | APLOG_ERR, NULL, 
            "GetIOQueue: No memory. pData");*/
        return WAVETOP_BACKUP_NO_MEMORY;
    }

    while(1) {

        /*if (pBakSlotGet->nSynStatus == WAVETOP_BACKUP_SYSTEM_BUSY) {
            nRet = WAVETOP_BACKUP_SYSTEM_BUSY;
            return nRet;
        }*/

      //  MiMSSqlWaitMemLimit();

        /* 读数据 */
        nRet = MiSqlGetIO(&pData, &nDataSize, &nSeqNum, LogHandle, pBuf, &nMaxSize);
        switch (nRet) {
        case WAVETOP_BACKUP_OK:
            nContinue = 0;
			if (willRemind && time(NULL) - intime > WAITTIME){
				/*SLogErrorWrite(APLOG_MARK, APLOG_INFO, NULL, 
					"Wait```Although reading INC in seconds,but each INC size is small,total size of INCs is not big enough to assmble.[DB: %s]", pBakSlotGet->szDBName);*/
				willRemind = false;
				nContinue = 30;
			}
            break;
        case WAVETOP_BACKUP_END:
            if (nAction == 10200 || nFlag) {
               /* SLogErrorWrite(APLOG_MARK, APLOG_INFO, NULL, 
                    "GetIOQueue: Wait Next IO.Action: %d.Flag: %d.SEQ: %llu.[DB: %s]", nAction,nFlag,nSeqNum,pBakSlotGet->szDBName);*/
//				if(nFlag==1){
//					if(nWaitNextIO>=100){
//						nWaitNextIO =0 ;
//    				    return WAVETOP_BACKUP_INTERNAL_ERROR;
//                    }else{
//                        nWaitNextIO++;
//                        Sleep(100);
//					    continue;
//                    }
//				}else{
					Sleep(1000);
					continue;
//				}
            }

			//3s内无增量数据，退出函数从而拼装备份集
            if (nContinue == 30)
                goto END;
            else {
                nContinue++;
                Sleep(100);
                continue;
            }
        default:
            /*SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO | APLOG_ERR, NULL, 
                "GetIOQueue: ReadBufferFromLogNext failed.");*/
            nRet = WAVETOP_BACKUP_SYSTEM_BUSY;
            goto END;
        }

        pTmp = pData;
        pTmp += sizeof(INT16);
        pTmp += sizeof(INT16);
        memcpy(&nAction, pTmp, sizeof(nAction));
        pTmp += sizeof(INT16);
        memcpy(&ncurroffset, pTmp, sizeof(UINT32));
        pTmp += 4;
        memcpy(&nLowOffset, pTmp, sizeof(UINT32));
        pTmp += 4;

        (ncurroffset <<= 32) += nLowOffset;

        /* 过滤空的IO块 */
        if (nDataSize < 14 || (ncurroffset && nDataSize > 14 && pTmp[0] == 0x00)) {
            continue;
        }

		/* 若相邻Seq号的两个datablock具有相同offset，则过滤后一个,且不应改变相关变量*/
		if (nLastOffsetInBuf == ncurroffset){
			/*SLogErrorWrite(APLOG_MARK, APLOG_INFO, NULL, 
				"The block has similar offset compared with previous block.SEQ:%lld.", nSeqNum);*/
			continue;
		}

        /* 全量增量衔接 */
        if (pszCurrLSN[0] != '\0') {
            if (nAction == 10200) {
                bFilterFlag = true;
                goto CONTINUE;
            }

            if (ncurroffset == 0) {
                bFilterFlag = false;
                goto CONTINUE;
            }
            if (bFilterFlag && pTemp->nLastOffset != ncurroffset)
                goto CONTINUE;

            if (nDataSize > 14 && pData[14] != 0xAB) {
                _snprintf((char *)szCurrLsn, 24, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", pTmp[12], 
                    pTmp[13], pTmp[14], pTmp[15], pTmp[16], pTmp[17], pTmp[18], pTmp[19], pTmp[20], pTmp[21]);
            }

            if (strcmp((char *)pszCurrLSN, (char *)szCurrLsn) == 0) {
                pszCurrLSN[0] = '\0';
                pTemp->nLastOffset = ncurroffset;
				willRemind = true;
				/*SLogErrorWrite(APLOG_MARK, APLOG_INFO, NULL, 
					"Find same lsn.Start reading logdata.[DB: %s]", pBakSlotGet->szDBName);*/
				cout<<"Find same lsn.Start reading logdata "<<pBakSlotGet->szDBName<<endl;
            }
            else {
				if (willRemind && time(NULL) - intime > WAITTIME){
					/*SLogErrorWrite(APLOG_MARK, APLOG_INFO, NULL, 
						"Wait```Comparing lsn in INC with lsn in BAK now,to find same lsn for linking FULL and INC.[DB: %s]", pBakSlotGet->szDBName);*/
					willRemind = false;
				}
                continue;
            }
        }

CONTINUE:

        if (!ncurroffset) {
            if (bSetFileSize) {
                bSetFileSize = false;
                nFlag = WAVETOP_BACKUP_OK;
            }
            continue;
        }

		//程序为每一个数据块加了一些首部结构，这部分大小为14。当SQLSREVER一个数据块为4096，logdata中这个数据块大小实际为4110
        if (nFlag && pTemp->nLastOffset != ncurroffset && nDataSize != 4110) {
            continue;
        }

		//日志大小开始发生改变
        if (nAction == 10200)
            nFlag = 1;

        if (nFlag && nDataSize > 14 && pData[14] == 0xAB && pData[15] == 0x00) {
            nIoSize += 2048;
            bSetFileSize = true;
        }

        /* 排序IO， update delete 大事务时IO回出现不连续情况 */
        if (ncurroffset == nFilteroffset && nDataSize > 14 && 
            !(pData[14] == 0xAB && pData[15] == 0x00)) {
                nQuit--;
                if (!nQuit)
                    nFilteroffset = 0;

                /*SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO | APLOG_ERR, NULL, 
                    "Filter IO of Sequence: %llu.", nSeqNum);*/
                continue;
        }

        if (!nFlag && pTemp->nLastOffset != ncurroffset && pTemp->nLastOffset){
            if (pData[14] != 0xAB|| !CheckMstlInfo(pTemp->pIOQueue, pTemp->nLastOffset, pBakSlotGet->pMstl)){
                if (!nLastSeqNum)
                    nLastSeqNum = nSeqNum;
                bTaxis = true;
                continue;
            }                
        }
        else if (!nFlag && bTaxis && pTemp->nLastOffset == ncurroffset) {
            nFilteroffset = ncurroffset;
            bTaxis = false;
            /* LOGDATA 回滚到上一次偏移量不相同的SEQUNCE*/
            nRet = MiMSSqlReadBufferFromLogStart(LogHandle, nLastSeqNum, 1);

            /*SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO | APLOG_ERR, NULL, 
                "Roll Back IO to Sequence: %llu.", nLastSeqNum);*/
			cout<<"Rack IO to Sequence: "<<nLastSeqNum<<endl;

            nQuit++;
            nLastSeqNum = 0;
        }

        if (pList == NULL) {
            pList = (WTBUF *)ap_palloc(pNewPool, sizeof(WTBUF));
            pTemp->pIOQueue = pList;
        }
        else {
            pList->pNext = (WTBUF *)ap_palloc(pNewPool, sizeof(WTBUF));
            pList = pList->pNext;

        }
        if (pList == NULL) {
            /*SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO | APLOG_ERR, NULL, 
                "GetIOQueue: No memory. pData");*/
            goto END;
        }

        memset(pList, '\0', sizeof(WTBUF));

        pList->dwBufSize = nDataSize;
        pList->pszBuf = (unsigned char *)ap_palloc(pNewPool, nDataSize);
        if (pList->pszBuf == NULL) {
            /*SLogErrorWrite(APLOG_MARK, APLOG_NOERRNO | APLOG_ERR, NULL, 
                "GetIOQueue pszBuf NO memory.");*/
            nRet = WAVETOP_BACKUP_SYSTEM_BUSY;
            goto END;
        }

        memcpy(pList->pszBuf, pData, nDataSize);
        pTemp->nCurrentSeq = nSeqNum;
        nCount++;
        pTemp->pIOQueue->pTail = pList;
        nTotSize += nDataSize - 14;
		nLastOffsetInBuf = ncurroffset;

        /* 确保偏移量不连续的IO都存在同一链表中 */
        if (nDataSize > 14) {
            if (pData[14] != 0xAB && pData[15] == 0x00) {
                /* 确保文件自增长后续几块特定IO在一个链表里 */
                nCurrSize += nDataSize;
                pTemp->nLastOffset = ncurroffset + nDataSize - 14;
                nLenNum += nDataSize - 14;
            }
            else if (pData[14] == 0xAB && pData[15] != 0x00){
                pTemp->nLastOffset = ncurroffset + 8192;
                if (nLenNum % 65536 > 0)
                    nLenNum += 65536 - (nLenNum % 65536);
                nIoSize += nLenNum;
                nLenNum = 0;
            }
            else 
                pList->dwPos = 1;
        }
        else
            pList->dwPos = 1;


        /*  
		 *	第一个条件控制 待拼装内容是否大于配置文件中设置的阈值；
		 *  第二个条件控制 拼装内容中最后一条记录不是更改日志操作记录集中的某一条记录；
		 *  第三个条件控制 读取过程中所有乱序的记录已经排好序
		 *  第四个条件防止意外情况，即使前三条件不满足，只要拼装内存大小大于某个值，就退出进行拼装。
		 */
        if (/*nCurrSize > g_MiGlobalConf.nTotalSize && */!nFlag &&
            !nQuit || nTotSize >= MI_MSSQL2_MAX_IO_SIZE){
				/*SLogErrorWrite(APLOG_MARK, APLOG_ALERT, NULL, 
					"Accident has happend.nCurrSize: %lu.TotalSize: %d.nFlag[%d].nQuit[%d].[DB: %s]",
					nCurrSize,g_MiGlobalConf.nTotalSize,nFlag,nQuit,pBakSlotGet->szDBName);*/
				break;
		}
    }

END: 
    if (nLenNum != 0) {
        if (nLenNum % 65536 > 0)
            nLenNum += 65536 - (nLenNum % 65536);
        nIoSize += nLenNum;
    }

    pTemp->nIOCount = nCount;
    pTemp->nFileSize = nIoSize;

    if (pData != NULL)
        free(pData);

    if (nCount)
        return WAVETOP_BACKUP_OK;
    else
        return WAVETOP_BACKUP_END;
}
