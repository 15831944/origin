#include "sqlite3.h"
#include "ft_config.h"
#include <cstring>

#ifdef WIN32
#include <Windows.h>
#include <WinIoCtl.h>
#include <Winsock2.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

#ifndef WIN32
#ifdef AIX
#define UIX_DISK_DEV_PATH "/dev"
#else
#define UIX_DISK_DEV_PATH "/sys/block"
#endif
extern void LogErrorMsg(const char* lpszFunction);
#else
extern void LogErrorMsg(LPTSTR lpszFunction) ;
#endif

int getDBPath(char* pszPath) {
#ifdef WIN32
    /*	int nRc = GetCurrentDirectoryA(1024,  pszPath);
    if (nRc == 0)
    return -1; */

    GetModuleFileName(NULL, pszPath, _MAX_PATH);  
    *(strrchr( pszPath, '\\') + 1) = 0;  
#else
#if defined(_BSD_SOURCE) || _XOPEN_SOURCE >= 500

    if (!getcwd(pszPath, 1024)) {
        return -1;
    }
#elif defined(_GNU_SOURCE)
    char* pszCur = get_current_dir_name();
    if (pszCur) {
        strncpy(pszPath, pszCur, 1024);
        free(pszCur);
        return 0;
    }
#endif

#endif //WIN32
    return 0;
}

/*
* @devs : out	
*
*/

int getFTDevPair(const char* pszServer, ft_dev_vec& devs, int* block_size) {

    int nRC = 0;
    char szTmp[1024] = { 0 };
    sqlite3* db = NULL;

    if (0 != getDBPath(szTmp)) {
        FT_LOG(FT_LOG_ERR,"getDBPath failed!");
        return -1;
    }
    strcat(szTmp, "/data/");
    strcat(szTmp, FT_DB_NAME);
    nRC = openDB(szTmp, (void**)&db);
    if (nRC != 0) {
        FT_LOG(FT_LOG_ERR,"openDB failed!");
        /* log message */
        return FT_CONFIG_DB_ERR;
    }
    nRC = lockDB(db);
    if (nRC != 0) {
        /* log message */
        FT_LOG(FT_LOG_ERR,"lockDB failed!");
        closeDB(db);
        return FT_CONFIG_DB_ERR;
    }

    do {

        char *pszErrMsg = NULL;
        char **pResult = NULL;
        char *pszSQL = NULL;
        int nRow = 0;
        int nCol = 0;

        PR_snprintf(szTmp, sizeof(szTmp), "select id,objname,device,buffersize,size from fc_resource_pool where status = 0 and cli_ip='%s';", pszServer);
        nRC = sqlite3_get_table(db, szTmp, &pResult, &nRow, &nCol, &pszErrMsg);
        if (nRC != SQLITE_OK) {
            break;
        }
        if (nRow < 2)
        {
            FT_LOG(FT_LOG_DEBUG,"selected row < 2 ");
            nRC = -1;
            break;
        }

        int i = 5;
        PRInt32 buffersize = 0;
        PRInt64 size = 0;
        char devname[1024] = { 0 };
        int ids[2] = { 0 };
        int buffsize = 0;
        *block_size = 0;
        FtChannelInfo stChannel =  { 0 };
        for (int j = 0; j < 2; j++, i += 5) {

            sscanf(pResult[i],"%ld",&ids[j]);
            strncpy(stChannel.signature,pResult[i + 1],sizeof(stChannel.signature));
            strncpy(stChannel.diskname,pResult[i + 2],sizeof(stChannel.diskname));
            sscanf(pResult[i + 3],"%d",&buffersize);
            sscanf(pResult[i + 4],"%lld",&size);

            stChannel.buffsize = buffersize;
            stChannel.disksize = size;
            devs.push_back(stChannel);

            if (*block_size == 0)
                *block_size = buffersize;
            else if (*block_size > buffersize)
                *block_size = buffersize;
        }
        sqlite3_free_table(pResult);
        snprintf(szTmp, 1024, "update fc_resource_pool set status = 1 where id=%d or id=%d;", ids[0], ids[1]);
        nRC = sqlite3_exec((sqlite3*)db, szTmp, NULL, NULL, &pszErrMsg);
        if (nRC != SQLITE_OK) {
            FT_LOG(FT_LOG_DEBUG,"update fc_resource_pool status failed !");
            break;
        }
    } while (0);

    unLockDB(db);
    closeDB(db);

    return nRC;
}

int putFTDevPair(ft_dev_vec& devs) {
    int nRC = 0;
    char szTmpPath[1024] = { 0 };
    sqlite3* db = NULL;

    if (devs.size() < 2) {
        return 0;
    }

    if (0 != getDBPath(szTmpPath)) {
        return -1;
    }
    strcat(szTmpPath, "/data/");
    strcat(szTmpPath, FT_DB_NAME);

    nRC = openDB(szTmpPath, (void**)&db);
    if (nRC != 0) {
        /* log message */
        return FT_CONFIG_DB_ERR;
    }
    nRC = lockDB(db);
    if (nRC != 0) {
        /* log message */
        closeDB(db);
        return FT_CONFIG_DB_ERR;
    }

    char *pszErrMsg = NULL;

    snprintf(szTmpPath, 1024, "update fc_resource_pool set status = 0 where objname='%s' or objname='%s';", devs[0].signature, devs[1].signature);
    FT_LOG(FT_LOG_INFO," sql :%s",szTmpPath);

    nRC = sqlite3_exec((sqlite3*)db, szTmpPath, NULL, NULL, &pszErrMsg);
    if (nRC != SQLITE_OK) {
        FT_LOG(FT_LOG_ERR," sql :%s exec error",szTmpPath);
    }

    unLockDB(db);
    closeDB(db);

    return nRC;
}

int sendFTDevPair(sbuff* pSB, ft_dev_vec& devs, int block_size) {
    int nRc;
    for (int i = 0; i < 2; i++) {
        WriteInt32(pSB, strlen(devs[i].signature));
        WriteBinary(pSB, (const unsigned char*)devs[i].signature, strlen(devs[i].signature));
        WriteInt32(pSB, strlen(devs[i].diskname));
        WriteBinary(pSB, (const unsigned char*)devs[i].diskname, strlen(devs[i].diskname));

        WriteInt32(pSB, devs[i].buffsize);
        WriteInt64(pSB,devs[i].disksize);
    }
    WriteInt32(pSB, block_size);
    nRc = FlushSBuff(pSB);
    PRInt32 nStatus;
    nRc = ReadInt32(pSB, &nStatus);
    if (nRc != 0)
        return nRc;
    else
        return nStatus;

}

int recvFTDevPair(sbuff* pSB, ft_dev_vec& devs, int* block_size) {
    int nRc;
    FtChannelInfo stChannel = { 0 };
    PRInt32 nLen = 0;
    PRInt32 nRead = 0;
  
    for (int i = 0; i < 2; i++) {
        nRc = ReadInt32(pSB, &nLen);
        if (nRc != 0) {
            return -1;
        }
        nRc = ReadBinary(pSB, (unsigned char*)stChannel.signature, nLen,&nRead);
        if (nRc != 0) {
            return -1;
        }
        stChannel.signature[nRead] = '\0';
        nRc = ReadInt32(pSB, &nLen);
        if (nRc != 0) {
            return -1;
        }
        nRc = ReadBinary(pSB, (unsigned char*)stChannel.diskname, nLen,&nRead);
        if (nRc != 0) {
            return -1;
        }
        stChannel.diskname[nRead] = '\0';
        
        nRc = ReadInt32(pSB,&stChannel.buffsize);
        if (nRc != 0) {
            return -1;
        }
        
        nRc = ReadInt64(pSB,&stChannel.disksize);
        if (nRc != 0) {
            return -1;
        }

        devs.push_back(stChannel);
    }
    nRc = ReadInt32(pSB, block_size);
    if (nRc != 0) {
        return -1;
    }

    WriteInt32(pSB, nRc);
    nRc = FlushSBuff(pSB);
    return nRc;
}

int checkSrvDev(ft_dev_vec& devs) {
    PRFileDesc* fd;
    int nRc = 0;
    for (int i = 0; i < 2; i++) {
        if ((fd = PR_Open(devs[i].diskname, PR_WRONLY,00664) ) == NULL) {
            FT_LOG(FT_LOG_ERR,"PR_Open %s failed",devs[i].diskname);
            LogErrorMsg("");
            return -1;
        }
        if (4096 != PR_Seek(fd, 4096 , PR_SEEK_SET) )
        {
            nRc = -1;
            FT_LOG(FT_LOG_ERR,"PR_Seek %s failed",devs[i].diskname);
            LogErrorMsg("");
        }
        int nLen = strlen(devs[i].signature) + 1;
        if ( nLen != PR_Write(fd, devs[i].signature, nLen)) {
            nRc = -1;
            FT_LOG(FT_LOG_ERR,"PR_Write %s failed",devs[i].diskname);
            LogErrorMsg("");
        }
        PR_Close(fd);
    }
    return nRc != 0 ? -1 : 0;
}

int checkCltDev(ft_dev_vec& devs) {

    int nRc = 0;

    for (int i = 0; i < 2; i++) {
        char szDev[128] = { 0 };
        nRc = findDevice(devs[i].signature, szDev,128);
        if (nRc != 0) {
            FT_LOG(FT_LOG_ERR,"findDevice %s failed",devs[i].diskname);
            break;
        }
        strncpy(devs[i].diskname,szDev,sizeof(devs[i].diskname));
    }

    return nRc;

}

int openDB(const char* pszDb, void** db) {
    int nRC = 0;
    sqlite3 *pDB = NULL;
    nRC = sqlite3_open(pszDb, &pDB);
    if (nRC != SQLITE_OK) {
        nRC = -1;
        goto END;
    }
    *db = pDB;
END:
    return nRC;
}

int lockDB(void* db) {
    char* pszErrMsg;
    int nRC = 0;
    int nCnt = 0;
    while (nCnt < 100) {
        int nt = sqlite3_exec((sqlite3*)db, "begin exclusive transaction", NULL, NULL, &pszErrMsg);
        if (nt == SQLITE_OK)
            break;
        else {
            PR_Sleep(1000);
            nCnt++;
        }
    }
    if (nCnt == 100)
        return -1;
    return 0;
}

int unLockDB(void* db) {
    char* pszErrMsg;
    int nRC = 0;
    sqlite3_exec((sqlite3*)db, "commit transaction", NULL, NULL, &pszErrMsg);
    return 0;	
}

int closeDB(void* db) {
    int nResult = 0;

    nResult = sqlite3_close((sqlite3*) db);

    return nResult;
}
#ifdef WIN32
static int getDiskCnt() {

    char szReg[256]; 
    DWORD dwRegType1(REG_DWORD);
    LPBYTE lpRegDwordData(NULL); 
    DWORD dwDataSize = sizeof(DWORD);  
    DWORD dwCnt(0);
    long  lRet; 
    HKEY hKey; 

    sprintf(szReg, "SYSTEM\\CurrentControlSet\\services\\disk\\Enum");

    lRet = ::RegOpenKeyEx(  
        HKEY_LOCAL_MACHINE, // root key  
        szReg, // 要访问的键的位置  
        0,         //  
        KEY_READ,  // 以查询的方式访问注册表  
        &hKey);    // hKEY保存此函数所打开的键的句柄  
    if (lRet != ERROR_SUCCESS ) {
        FT_LOG(FT_LOG_ERR,"RegOpenKeyEx %s failed",szReg);
        return 0;
    }

    lRet = ::RegQueryValueEx(hKey, // 所打开的键的句柄  
        "Count",    // 要查询的键值名  
        NULL,  
        &dwRegType1,    // 查询数据的类型  
        (LPBYTE)&dwCnt,  // 保存所查询的数据  
        &dwDataSize);  // 预设置的数据长度  

    ::RegCloseKey(hKey);  
    if (lRet != ERROR_SUCCESS ) {
        FT_LOG(FT_LOG_ERR,"RegQueryValueEx  %s\\Count failed",szReg);
        return 0;
    }
    return dwCnt < 1024 ? dwCnt : 0;


}
#endif

int findDevice(const char* pszObjName, char* deviceName,int size) {
    char szDevName[128] = { 0 };
    int nRc = -1;
    char *pszBuff ;
#ifdef WIN32
    pszBuff	= (char*)malloc(512);
    if (pszBuff == NULL) {
        return -1;
    }
    int nDiskCnt =  getDiskCnt();
    FT_LOG(FT_LOG_DEBUG,"getDiskCnt %d ",nDiskCnt);
    int nErr = 0 , nDiskNum = 1;
    HANDLE hDisk;  

    /* 第一块硬盘是系统盘,跳过 */
    for (int i = 1; nDiskNum < nDiskCnt;i++ ) {  
        PR_snprintf(szDevName, 128, "\\\\.\\Physicaldrive%d", i);
        hDisk  = CreateFileA(szDevName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,  OPEN_EXISTING, NULL, 0);
        if (hDisk == INVALID_HANDLE_VALUE ) {
            /* 磁盘序号有可能不连续，导致打开错误 */
            if ( ERROR_FILE_NOT_FOUND == GetLastError())
            {
                if (++nErr > 10)
                    break;
            }
            else {
                FT_LOG(FT_LOG_ERR,"CreateFileA %s ",szDevName);
                LogErrorMsg(TEXT("msg :"));
                nDiskNum++;
            }
            continue;
        }
        nDiskNum++;
        DWORD dwIOCount = 0;
        if (!DeviceIoControl(hDisk, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &dwIOCount, NULL))
        {
            FT_LOG(FT_LOG_ALERT,"CreateFileA %s failed",szDevName);
            LogErrorMsg(TEXT("msg :"));
            CloseHandle(hDisk);
            hDisk = NULL;
            continue;
        }
        LARGE_INTEGER offset;
        offset.QuadPart = 4096;
        DWORD nResult = SetFilePointer(hDisk, offset.LowPart, &offset.HighPart, FILE_BEGIN);
        if (nResult == INVALID_SET_FILE_POINTER) {
            FT_LOG(FT_LOG_ALERT,"SetFilePointer %s filed",szDevName);
            LogErrorMsg(TEXT("msg :"));
            CloseHandle(hDisk);
            hDisk = NULL;
            continue;
        }
        DWORD dwRead = 0;
        if (!ReadFile(hDisk, pszBuff, 512, &dwRead, NULL)) {
            FT_LOG(FT_LOG_ALERT,"ReadFile %s filed",szDevName);
            LogErrorMsg(TEXT("msg :"));
            CloseHandle(hDisk);
            hDisk = NULL;
            continue;
        }

        CloseHandle(hDisk);
        hDisk = NULL;

        if (memcmp(pszObjName, pszBuff, 20) == 0) {
            nRc = 0;
            strcpy(deviceName, szDevName);
            break;
        }

    }
    if (hDisk && hDisk != INVALID_HANDLE_VALUE)
        CloseHandle(hDisk);
    free(pszBuff);

/*#elif defined(AIX)
    int fd;
    posix_memalign((void**)&pszBuff, sysconf(_SC_PAGESIZE), 512);
    if (pszBuff == NULL) {
        return -1;
    }
    FILE* fpipe = popen("ls /dev/hdisk*","r");
    if (fpipe == NULL) {
        return -1;
    }
    while (fgets(szDevName,255,fpipe) != NULL ) {
        if(szDevName[strlen(szDevName) -1] == '\n') {      
           szDevName[strlen(szDevName) -1] = '\0';   
        }
        fd = open(szDevName, O_DIRECT | O_RDONLY );
        if (fd == -1){
            FT_LOG(FT_LOG_ALERT,"open:%s failed!",szDevName);
            continue;
        }

        nRc = lseek(fd, 4096, SEEK_CUR);
        if (nRc == -1 ) {
            FT_LOG(FT_LOG_ALERT,"lseek:%s failed!",szDevName);
            goto CONTINUE;
        }
        nRc = read(fd, pszBuff, 512);
        if (nRc <= 0) {
            FT_LOG(FT_LOG_ALERT,"read:%s failed!",szDevName);
            goto CONTINUE;
        }
        if (memcmp(pszObjName, pszBuff, 20) == 0) {
            nRc = 0;
            strcpy(deviceName, szDevName);
            break;
        }
CONTINUE:	
        close(fd);
        fd = NULL;
    }
    if (fd)
        close(fd);
    if (fpipe)
        pclose(fpipe);
    free(pszBuff); */
#else
    const char* blk_dev_dir = UIX_DISK_DEV_PATH;
    int fd = 0;
    PRDir* fdir;
    PRDirEntry* pEntry;
    posix_memalign((void**)&pszBuff, sysconf(_SC_PAGESIZE), 512);
    if (pszBuff == NULL) {
        return -1;
    }

    if ( (fdir = PR_OpenDir(blk_dev_dir)) == NULL) {
        FT_LOG(FT_LOG_ERR,"PR_OpenDir:%s failed!",blk_dev_dir);
        return -1;
    }
    while ((pEntry = PR_ReadDir(fdir, PR_SKIP_BOTH)) != NULL) {

#if defined(AIX)
        if (strncmp(pEntry->name,"rhdisk",6) != 0)
            continue;
#endif
        PR_snprintf(szDevName, sizeof(szDevName), "/dev/%s", pEntry->name);
        FT_LOG(FT_LOG_DEBUG,"scan disk:%s begin",szDevName);
        fd = open(szDevName, O_DIRECT | O_RDONLY );
        if (fd == -1){
            FT_LOG(FT_LOG_ALERT,"open:%s failed!",szDevName);
            continue;
        }

        nRc = lseek(fd, 4096, SEEK_CUR);
        if (nRc == -1 ) {
            FT_LOG(FT_LOG_ALERT,"lseek:%s failed!",szDevName);
            goto CONTINUE;
        }
        nRc = read(fd, pszBuff, 512);
        if (nRc <= 0) {
            FT_LOG(FT_LOG_ALERT,"read:%s failed!",szDevName);
            goto CONTINUE;
        }
        if (memcmp(pszObjName, pszBuff, 20) == 0) {
            nRc = 0;
            strcpy(deviceName, szDevName);
            break;
        }
CONTINUE:	
        close(fd);
        fd = NULL;

    }
    if (fd )
        close(fd);
    if (fdir) {
        PR_CloseDir(fdir);
    }
    free(pszBuff);

#endif
    return nRc;
}

/* 根据FD获取IP */
int getSockIp(int fd, char *pszIp, int nlen)
{
    int nRC = 0;
    sockaddr_in sockAddr = { 0 };

#ifdef WIN32
    int nSockAddrLen = sizeof(sockaddr_in);
    nRC = getsockname(fd, (struct sockaddr *)&sockAddr, &nSockAddrLen);
#else
    socklen_t nSockAddrLen = sizeof(sockaddr_in);
    nRC = getsockname(fd, (sockaddr *)&sockAddr, &nSockAddrLen);
#endif
    if (nRC != 0) {

        return 1;
    }
    char* pszIpTmp = inet_ntoa(sockAddr.sin_addr);
    if (pszIpTmp == NULL) {
        return 1;
    }
    strncpy(pszIp, pszIpTmp,nlen);
    return nRC;
}