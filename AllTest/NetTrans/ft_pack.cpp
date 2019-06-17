/*** =============================================================================
** Copyright (c) 2003 WaveTop Information Corp. All rights reserved.
**
** The lanfree transport class
**
** =============================================================================
*/

#ifdef WIN32
#include <winsock2.h>
#include <strsafe.h>
#include <WinIoCtl.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>
#include <poll.h>
#include <stddef.h>
#include <assert.h>

#ifndef AIX
#include <linux/fs.h>
#include <sys/epoll.h>
#endif

#endif

#if defined(AIX)
#include <aio.h>
#include <sys/devinfo.h>
#endif

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include "wnetfd.h"
#include "ft_config.h"
#include "ft_pack.h"

#define FT_MAX_CONTENT (FT_BLOCK_SIZE - sizeof(FTDataHeaderSt))
#define INT_MAX_SIZE ((unsigned int)~0)
#define BEGIN_OFFSET 4096
#define FT_CHNANEL_SPACE_LEFT(m) (m.block_size - sizeof(FTDataHeaderSt) - m.nTotal)
#define FT_CHNANEL_SPACE_LEFT_NO_HEADER(m) (m.block_size  - m.nTotal)

#define FT_NONE_INIT   0
#define FT_INIT_OK     1
#define FT_CLOSE_WAIT  2
#define FT_CLOSE_WAIT1 3



#define XINT32(n,p) {\
    (p)[0]=(unsigned char)(((n)>>24)&0xff);\
    (p)[1]=(unsigned char)(((n)>>16)&0xff);\
    (p)[2]=(unsigned char)(((n)>>8)&0xff);\
    (p)[3]=(unsigned char)( (n)&0xff);\
}
#define VINT32(n,p) {\
    n= (((unsigned char)((p)[0])) << 24) + \
    (((unsigned char)((p)[1])) << 16) + \
    (((unsigned char)((p)[2])) << 8 ) + \
    (((unsigned char)((p)[3]))      ) ; \
}

#define XINT16(n,p) {\
    (p)[0]=(unsigned char)(((n)>>8)&0xff);\
    (p)[1]=(unsigned char)( (n)&0xff);\
}

#define VINT16(n,p) {\
    n= (((unsigned char)((p)[0])) << 8) + \
    (((unsigned char)((p)[1]))     ) ; \
}

#ifdef WIN32
#define snprintf _snprintf
#define FT_FREE_BUF(b) _aligned_free(b)
#else 
#define DWORD PRInt32
#define FT_FREE_BUF(b) free(b)
#define NETLINK_USER 31
#define MAX_PAYLOAD 1024
#define USE_MMAP 1
#endif

#ifdef WIN32
void LogErrorMsg(LPTSTR lpszFunction) 
{ 

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError(); 

    // Retrieve the system error message for the last-error code
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,0, NULL );

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
        (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR)); 

    StringCchPrintf((LPTSTR)lpDisplayBuf,LocalSize(lpDisplayBuf) / sizeof(TCHAR),TEXT("%s failed with error			%d: %s"),lpszFunction, dw, lpMsgBuf);

    /*	int iBuffSize = WideCharToMultiByte(CP_ACP, 0, (LPCWCH)lpDisplayBuf, -1, NULL, 0, NULL, false);  
    if (iBuffSize > 0 )  
    {  
    char* m_pString = (char*) new char[iBuffSize];  
    WideCharToMultiByte(CP_ACP, 0, (LPCWCH)lpDisplayBuf, -1, m_pString, iBuffSize, NULL, false);
    FT_LOG(FT_LOG_ERR,"%s", m_pString);
    delete[] m_pString;
    }  */
    FT_LOG(FT_LOG_ERR,"%s", lpDisplayBuf);
    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
}

#else 
#define TEXT(d) (d)
void LogErrorMsg(const char* lpszFunction) 

{
    FT_LOG(FT_LOG_ERR,"%s failed with error %d: %s",lpszFunction,errno,strerror(errno));
}

#ifndef AIX 
#include <sys/syscall.h>
#include <linux/aio_abi.h>

typedef unsigned int u_int;

static int
    io_submit(ft_aio_context_t ctx, long n, struct iocb **paiocb)
{
    return syscall(SYS_io_submit, ctx, n, paiocb);
}

static int
    io_setup(u_int nr_reqs, ft_aio_context_t *ctx)
{
    return syscall(SYS_io_setup, nr_reqs, ctx);
}

static int
    io_destroy(ft_aio_context_t ctx)
{
    return syscall(SYS_io_destroy, ctx);
}

static int
    io_getevents(ft_aio_context_t ctx, long min_nr, long nr, struct io_event *events,
struct timespec *tmo)
{
    return syscall(SYS_io_getevents, ctx, min_nr, nr, events, tmo);
}

#endif

#endif

static inline void *
    ptr_align (void const *ptr, size_t alignment)
{
    char const *p0 =(char const *) ptr;
    char const *p1 = p0 + alignment - 1;
    return (void *) (p1 - (size_t) p1 % alignment);
}

static inline unsigned short checksum(char* buff, int len) {
    int FirstByte = 1;
    int SecondByte = 1;
    char sum[2];
    for (int i = 0; i < len; ++i) {
        FirstByte += buff[i]; SecondByte += FirstByte;
    }
    sum[0] = FirstByte % 251;
    sum[1] = SecondByte % 251;
    return *(unsigned short*)sum;
}

static inline void 
    unpack_ctrlheader(FTControlPackHeaderSt* pack, FTCtrlHeaderSt* unpack)
{
    VINT16(unpack->code, pack->szCode)
    VINT16(unpack->type, pack->szType);
    VINT32(unpack->nlen, pack->szLen);
    VINT32(unpack->nDataLen, pack->szDataLen);
}

static inline void 
    pack_ctrlheader(FTControlPackHeaderSt* pack, FTCtrlHeaderSt* unpack)
{
    XINT16(unpack->code, pack->szCode)
    XINT16(unpack->type, pack->szType);
    XINT32(unpack->nlen, pack->szLen);
    XINT32(unpack->nDataLen, pack->szDataLen);
}

static void set_tcp_nodelay(int fd) {
    int enable = 1;
#ifdef WIN32
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&enable, sizeof(enable));
#else
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void*)&enable, sizeof(enable));
#endif
}


#if !(defined(WIN32) || defined(AIX))

int FTransport::netlink_get_msg(void* msg) {

    struct msghdr* pmsg = (struct msghdr*)msg ;

    struct pollfd rfds[1];
    long long size = 0;
    rfds[0].fd = m_netlink_fd;
    rfds[0].events = POLLIN;
    while (1)
    {
        errno = 0;
        int ret = poll(rfds, sizeof(rfds) / sizeof(rfds[0]), 500);
        if (ret < 0) {
            FT_LOG(FT_LOG_ERR,"poll failed ,code %d,msg:%s",errno,strerror(errno));
            return FT_ERR_SOCK;
        }
        else if (!ret) {
            return FT_ERR_TIMEOUT;
        }
        else {
            recvmsg(m_netlink_fd, pmsg, 0);
            break;
        }

    }
    return FT_OK;

}

int FTransport::netlink_send_msg(void* msg) {
    return FT_OK;
}

int FTransport::netlink_init() {

    struct sockaddr_nl src_addr;
    struct nlmsghdr *nlh = NULL;
    if (m_netlink_fd == -1) {
        m_netlink_pid = time(NULL);
        m_netlink_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);
        if (m_netlink_fd < 0)
            return -1;

        memset(&src_addr, 0, sizeof(src_addr));
        src_addr.nl_family = AF_NETLINK;
        src_addr.nl_pid = m_netlink_pid; /* self pid */

        if (0 != bind(m_netlink_fd, (struct sockaddr *)&src_addr, sizeof(src_addr))) {
            perror("bind");
            close(m_netlink_fd);
            m_netlink_fd = -1;
            return -1;
        }

        memset(&m_dest_addr, 0, sizeof(m_dest_addr));
        m_dest_addr.nl_family = AF_NETLINK;
        m_dest_addr.nl_pid = 0; /* For Linux Kernel */
        m_dest_addr.nl_groups = 0; /* unicast */

        struct iovec iov = { 0 };
        struct msghdr msg = { 0 };

        nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
        if (nlh == NULL)
            return -1;

        if (m_wdisk.fd != NULL) {
            memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
            nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
            nlh->nlmsg_pid = m_netlink_pid;
            nlh->nlmsg_flags = 0;
            snprintf((char*)NLMSG_DATA(nlh), MAX_PAYLOAD, "PUTW\nINO:%ld\nDEV:%ld\n", m_wdisk.ino, m_wdisk.idev);

            iov.iov_base = (void *)nlh;
            iov.iov_len = nlh->nlmsg_len;
            msg.msg_name = (void *)&m_dest_addr;
            msg.msg_namelen = sizeof(m_dest_addr);
            msg.msg_iov = &iov;
            msg.msg_iovlen = 1;
            sendmsg(m_netlink_fd, &msg, 0);
            int nRc =  netlink_get_msg(&msg);
            if (nRc != FT_OK ) {
                free(nlh);
                return nRc;
            }
        }

        if (m_rdisk.fd != NULL) {
            memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
            nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
            nlh->nlmsg_pid = m_netlink_pid;
            nlh->nlmsg_flags = 0;
            snprintf((char*)NLMSG_DATA(nlh), MAX_PAYLOAD, "PUTR\nINO:%ld\nDEV:%ld\n", m_rdisk.ino, m_rdisk.idev);
            iov.iov_base = (void *)nlh;
            iov.iov_len = nlh->nlmsg_len;
            msg.msg_name = (void *)&m_dest_addr;
            msg.msg_namelen = sizeof(m_dest_addr);
            msg.msg_iov = &iov;
            msg.msg_iovlen = 1;
            sendmsg(m_netlink_fd, &msg, 0);

            int nRc =  netlink_get_msg(&msg);
            if (nRc != FT_OK && nRc != FT_ERR_TIMEOUT) {
                free(nlh);
                return nRc;
            }
        }
        free(nlh);
    }
    return 0;
}

unsigned long FTransport::netlink_get_recv_size()
{
    struct iovec iov = { 0 };
    struct msghdr msg = { 0 };
    struct nlmsghdr *nlh = NULL;

    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    if (!nlh) {
        return -1;
    }
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = m_netlink_pid;
    nlh->nlmsg_flags = 0;

    snprintf((char*)NLMSG_DATA(nlh), MAX_PAYLOAD, "GETW\nINO:%ld\nDEV:%ld\n", m_rdisk.ino, m_rdisk.idev);
    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;
    msg.msg_name = (void *)&m_dest_addr;
    msg.msg_namelen = sizeof(m_dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    if (-1 == sendmsg(m_netlink_fd, &msg, 0) ) {
        LogErrorMsg("sendmsg");
        return -1;
    }

    long long size = 0;
    int nRc =  netlink_get_msg(&msg);
    if (nRc != FT_OK ) {
        free(nlh);
        return 0;
    }
    /* Read message from kernel */
    memcpy((void*)&size, NLMSG_DATA(nlh), sizeof(size));
    if (nlh)
        free(nlh);
    return size < 0 ? 0 : size;
}


unsigned long FTransport::netlink_get_send_ack()
{

    struct iovec iov = { 0 };
    struct msghdr msg = { 0 };
    struct nlmsghdr *nlh = NULL;

    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    if (!nlh) {
        return -1;
    }
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = m_netlink_pid;
    nlh->nlmsg_flags = 0;

    snprintf((char*)NLMSG_DATA(nlh), MAX_PAYLOAD, "GETR\nINO:%ld\nDEV:%ld\n", m_wdisk.ino, m_wdisk.idev);
    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;
    msg.msg_name = (void *)&m_dest_addr;
    msg.msg_namelen = sizeof(m_dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    if (-1 == sendmsg(m_netlink_fd, &msg, 0) ) {
        LogErrorMsg("recvmsg");
        return -1;
    }

    long long size = 0;
    int nRc =  netlink_get_msg(&msg);
    if (nRc != FT_OK && nRc != FT_ERR_TIMEOUT) {
        free(nlh);
        return 0;
    }

    /* Read message from kernel */
    memcpy((void*)&size, NLMSG_DATA(nlh), sizeof(size));
    if (nlh)
        free(nlh);
    return size < 0 ? 0 : size;
}
#endif

FTransport::FTransport() {
    memset(&m_wbuff,0,sizeof(struct stFTChannel));
    memset(&m_rbuff,0,sizeof(struct stFTChannel));
    memset(&m_wdisk,0,sizeof(struct stFTDisk));
    memset(&m_rdisk,0,sizeof(struct stFTDisk));
    memset(&m_last_read,0,sizeof(FTCtrlHeaderSt));
    memset(&m_last_send,0,sizeof(FTCtrlHeaderSt));

    m_total_send = 0;
    m_send_ack = 0;
    m_total_recv = 0;
    m_recv_ack = 0;
    m_tcpsbuf = NULL;
    m_is_noblock = 0;
    m_is_server = 0;

    m_send_ctrl_cnt = 0;
    m_recv_ctrl_cnt = 0;

#if !(defined(WIN32) || defined(AIX))
    m_left_to_read = 0;
    m_netlink_fd = -1;
    m_netlink_pid = 0;
    m_epoll_in_fd = -1;
    m_epoll_out_fd = -1;
#endif
}

FTransport::~FTransport() {

    FT_LOG(FT_LOG_INFO,"FT transport END IN state :%d",m_init_state);

    if (!m_is_server)
        putFTDevPair(m_devs);

#ifdef WIN32
    if (m_wdisk.overlap.hEvent)
        ftclose(m_wdisk.overlap.hEvent);
    if (m_rdisk.overlap.hEvent)
        ftclose(m_rdisk.overlap.hEvent);
#else

#if !(defined(AIX))
    if (m_netlink_fd != -1)
        ftclose((void*)m_netlink_fd);

    if (m_wdisk.ctx != 0) {
        io_destroy(m_wdisk.ctx);
    }
    if (m_rdisk.ctx != 0) {
        io_destroy(m_rdisk.ctx);
    }
#endif  
    if (m_wdisk.iocb != NULL) {
        free(m_wdisk.iocb);
    }
    if (m_rdisk.iocb != NULL) {
        free(m_rdisk.iocb);
    }

    if (m_wdisk.addr)
        munmap(m_wdisk.addr,m_wdisk.device_size);

    if (m_rdisk.addr)
        munmap(m_rdisk.addr,m_rdisk.device_size);
#endif

    if (m_wdisk.devpath)
        free(m_wdisk.devpath);

    if (m_rdisk.devpath)
        free(m_rdisk.devpath);


    if (m_wdisk.fd) {
        ftclose(m_wdisk.fd);
    }

    if (m_rdisk.fd) {
        ftclose(m_rdisk.fd);
    }

    if (m_wbuff.interlbuff){
        FT_FREE_BUF(m_wbuff.interlbuff);	
    }

    if (m_wbuff.interlbuff1){
        FT_FREE_BUF(m_wbuff.interlbuff1);
    }
    if (m_rbuff.interlbuff){
        FT_FREE_BUF(m_rbuff.interlbuff);	
    }

    if (m_rbuff.interlbuff1){
        FT_FREE_BUF(m_rbuff.interlbuff1);
    }

    if (m_init_state)
        UnInit(1);

    if (m_tcpsbuf) {
#ifdef WIN32
        if (m_is_noblock == 1) {
            WSAEventSelect(m_tcpsbuf->fd,m_tcpsbuf->hEvent,0 );
            unsigned long enable = 0;
            if (SOCKET_ERROR == ioctlsocket(m_tcpsbuf->fd, FIONBIO, &enable)){
                FT_LOG(FT_LOG_ERR,"ioctlsocket FIONBIO");
                LogErrorMsg("");
            }	

        }
        WSACloseEvent (m_tcpsbuf->hEvent);
#else      
        ioctl(m_tcpsbuf->fd, FIONBIO, 0);
#ifndef AIX
        if (m_epoll_in_fd != -1) {
            close(m_epoll_in_fd);
        }
        if (m_epoll_out_fd != -1) {
            close(m_epoll_out_fd);
        }
#endif
#endif
        free((void*)m_tcpsbuf);
    }

    FT_LOG(FT_LOG_INFO,"FT transport End send ctrl :%lld ,recv ctrl %lld",m_send_ctrl_cnt,m_recv_ctrl_cnt);

}

int FTransport::open_disk(struct stFTDisk* disk,struct stFTChannel* channel, unsigned long flag) {
#ifdef WIN32
    char szDiskName[512];
    GET_LENGTH_INFORMATION pds = { 0 };
    DWORD junk = 0;
    BOOL bResult = FALSE;
    LARGE_INTEGER offset;

    if (disk->devpath != NULL) {
        snprintf(szDiskName, sizeof(szDiskName) / sizeof(szDiskName[0]) - 1, "%s", disk->devpath);
        HANDLE hDisk = CreateFileA(szDiskName, GENERIC_READ | GENERIC_WRITE ,
            FILE_SHARE_READ |FILE_SHARE_WRITE, NULL, OPEN_EXISTING, flag , 0);
        if (hDisk == INVALID_HANDLE_VALUE) {
            return FT_ERR_OPEN;

        }
        disk->fd = hDisk;
        bResult = DeviceIoControl(
            (HANDLE)hDisk,              // handle to device
            IOCTL_DISK_GET_LENGTH_INFO,    // dwIoControlCode
            NULL, 0,                          // lpInBuffer
            &pds, sizeof(pds)                           // nInBufferSize
            , &junk,     // number of bytes returned
            NULL    // OVERLAPPED structure
            );

        if (!bResult)
            return FT_ERR_OPEN;
        disk->device_size = pds.Length.QuadPart;

        offset.QuadPart = channel->block_size;
        DWORD nResult = SetFilePointer(hDisk, offset.LowPart, &offset.HighPart, FILE_BEGIN);
        if (nResult == INVALID_SET_FILE_POINTER) {
            return FT_ERR_SEEK;
        }
        disk->offset = channel->block_size;
        DWORD dwIOCount = 0;
        if (!DeviceIoControl(hDisk, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &dwIOCount, NULL))
        {
            LogErrorMsg(TEXT("DeviceIoControl"));
            return FALSE;
        }
    }
#else 
    char szDiskName[512];

    snprintf(szDiskName, sizeof(szDiskName) / sizeof(szDiskName[0]) - 1, "%s", disk->devpath);
    int hDisk = open(szDiskName, O_DIRECT | O_RDWR | O_SYNC);
    if (hDisk == -1)
        return FT_ERR_OPEN;
    disk->fd = (void*)hDisk;
    /*	PRUint64 sector_size, total_sectors;
    if (-1 == ioctl(hDisk, BLKSSZGET, &sector_size))
    {
    return FT_ERR_UNKNOWN;
    }

    if (-1 == ioctl(hDisk, BLKGETSIZE, &total_sectors)) {
    LogErrorMsg("ioctl BLKGETSIZE");
    return false;
    }
    //	FT_LOG(FT_LOG_DEBUG, "sector size:%lu, total sector:%lu \n", sector_size, total_sectors);
    */
    PRInt64 disk_size;

#ifdef AIX
    struct devinfo stDev = {0};
    if (-1 == ioctl (hDisk, IOCINFO, &stDev)) {
        LogErrorMsg("ioctl(hDisk,IOCINFO,) ");
        return FT_ERR_UNKNOWN;
    }
    disk_size = ((PRInt64)stDev.un.scdk64.lo_numblks) * stDev.un.scdk64.blksize;
    FT_LOG(FT_LOG_DEBUG,"disk blocks %d ,bytes %d ,device size:%lld",stDev.un.scdk64.lo_numblks,stDev.un.scdk64.blksize,disk_size);
#else
    disk_size = lseek(hDisk, 0, SEEK_END);
#endif

    if (disk_size == -1 || disk_size == 0 ) {
        LogErrorMsg("lseek64");
        return FT_ERR_SEEK;
    }

    disk->device_size = disk_size;

    off_t offset;
    offset = channel->block_size;
    int nResult = lseek(hDisk, offset, SEEK_SET);
    if (nResult == -1) {
        return FT_ERR_SEEK;
    }
    disk->offset =  channel->block_size;

#endif

    return FT_OK;
}

#if !(defined(AIX) || defined(WIN32))

int FTransport::mmap_disk(struct stFTDisk* disk, struct stFTChannel* channel) {
    char szDiskName[512];
    snprintf(szDiskName, sizeof(szDiskName) / sizeof(szDiskName[0]) - 1, "%s", disk->devpath);
    int hDisk = open(szDiskName, O_RDWR);
    if (hDisk == -1)
        return FT_ERR_OPEN;
    disk->fd = (void*)hDisk;
    struct stat sb;
    if (fstat((long)disk->fd, &sb) == -1)
    {
        LogErrorMsg("fstat");
        return FT_ERR_OPEN;
    }
#ifdef USE_MMAP
    void* addr = mmap(NULL, sb.st_size, PROT_WRITE| PROT_READ,
        MAP_SHARED, (long)disk->fd, 0);
    if (addr == MAP_FAILED) {
        return FT_ERR_NO_MEM;
    }
    disk->addr = addr;
#endif
    disk->ino = sb.st_ino;
    disk->idev = (major(sb.st_dev) << 20) + minor(sb.st_dev);

    disk->offset = channel->block_size ;
    disk->device_size = sb.st_size;
#ifndef USE_MMAP
    ftseek(disk->fd, channel->block_size);
#endif
    return FT_OK;
}
#endif

int FTransport::open_device() {

    if (!m_is_server) {

        if (m_wdisk.devpath) {
            unsigned long flag = 0;
#ifdef WIN32
            flag |= FILE_FLAG_OVERLAPPED;
            flag |= FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH;
#endif
            if (open_disk(&m_wdisk,&m_wbuff, flag) != FT_OK) {
                return FT_ERR_OPEN;
            }
            if (m_wdisk.device_size != m_devs[0].disksize) {
                FT_LOG(FT_LOG_ERR,"m_wdisk:%s, actual size:%lld not equal:%lld",m_wdisk.devpath,m_wdisk.device_size,m_devs[0].disksize);
                return FT_ERR_UNKNOWN;
            }
        }
        if (m_rdisk.devpath) {
            unsigned long flag = 0;
#ifdef WIN32
            flag |= FILE_FLAG_NO_BUFFERING;
            flag |= FILE_FLAG_WRITE_THROUGH;
            flag |= FILE_FLAG_OVERLAPPED;
#endif
            if (open_disk(&m_rdisk,&m_rbuff,flag) != FT_OK) {
                return FT_ERR_OPEN;
            }
            if (m_rdisk.device_size != m_devs[1].disksize) {
                FT_LOG(FT_LOG_ERR,"m_rdisk:%s, actual size:%lld not equal:%lld",m_rdisk.devpath,m_rdisk.device_size,m_devs[1].disksize);
                return FT_ERR_UNKNOWN;
            }
        }

    }
    else {
#if !(defined(AIX) || defined(WIN32))
        if (m_wdisk.devpath) {
            if (mmap_disk(&m_wdisk, &m_wbuff) != FT_OK){
                return FT_ERR_OPEN;
            }
            if (m_wdisk.device_size != m_devs[1].disksize) {
                FT_LOG(FT_LOG_ERR,"m_wdisk:%s, actual size:%lld not equal:%lld",m_wdisk.devpath,m_wdisk.device_size,m_devs[0].disksize);
                return FT_ERR_UNKNOWN;
            }
        }
        if (m_rdisk.devpath) {
            if (mmap_disk(&m_rdisk, &m_rbuff) != FT_OK) {
                return FT_ERR_OPEN;
            }
            if (m_rdisk.device_size != m_devs[0].disksize) {
                FT_LOG(FT_LOG_ERR,"m_rdisk:%s, actual size:%lld not equal:%lld",m_rdisk.devpath,m_rdisk.device_size,m_devs[1].disksize);
                return FT_ERR_UNKNOWN;
            }
        }	
#endif
    }
    return FT_OK;
}


int FTransport::cs_sync(int state) {
    PRInt32 nSync = 0;
    int nRc = FT_OK;
    if (!m_tcpsbuf) {
        return FT_ERR_NO_MEM;
    }
    if (m_is_server) {
        nRc = ReadInt32(m_tcpsbuf,&nSync);
        if (nRc != FT_OK || nSync != 0xFF ) 
        {
            return FT_ERR_UNKNOWN;
        }
        nSync = state == FT_OK ? 0xFF : 0;
        WriteInt32(m_tcpsbuf,nSync);
        if (FlushSBuff(m_tcpsbuf) != FT_OK) {
            return FT_ERR_SOCK;
        }
    }
    else {
        nSync = state == FT_OK ? 0xFF : 0;
        WriteInt32(m_tcpsbuf,nSync);
        if (FlushSBuff(m_tcpsbuf) != FT_OK) {
            return FT_ERR_SOCK;
        }
        if (state == FT_OK){
            unsigned char szBuff[4] = {0};
            PRInt32 nRead; 
            nRc = ReadDataNoBuff(m_tcpsbuf,(char*)szBuff,4,&nRead);
            VINT32(nSync,szBuff);
            if (nRc != FT_OK || nSync != 0xFF ) 
            {
                return FT_ERR_UNKNOWN;
            }
        }
    }

    return FT_OK;

}


int FTransport::Init(sbuff *tcp,unsigned long flag ){

    int nRc = 0 ,block_size = 0,nNetStatus = 0;
    m_is_server  =  (flag & FT_CLIENT) ? 0 : 1;
    if (flag & FT_USE_NO_BLOCK) 
        m_is_noblock = 1;

    if (!m_is_server) {
        char szIp[16] = { 0 };
        nRc = getSockIp(tcp->fd, szIp, 16);
        if (nRc != 0) {
            FT_LOG(FT_LOG_ERR,"getSockIp failed!");
            return FT_ERR_SOCK;
        }

            nRc = getFTDevPair(szIp,m_devs,&block_size);
            WriteInt32(tcp, nRc);
            nNetStatus = FlushSBuff(tcp);
            if (nRc  != 0 || nNetStatus != 0) {
                FT_LOG(FT_LOG_DEBUG,"getFTDevPair failed!");
                return nRc ? nRc : nNetStatus;
            }

            nRc = sendFTDevPair(tcp, m_devs,block_size);
            if (nRc != 0) {
                FT_LOG(FT_LOG_ERR,"sendFTDevPair failed!");
                return nRc;
            }
            int nStatus;
            nNetStatus = ReadInt32(tcp, &nStatus);
            if (nNetStatus != 0 || nStatus != 0) {
                return nNetStatus ? nNetStatus : nStatus;
            }

            nRc = checkCltDev(m_devs);
            if (nRc != 0) {
                FT_LOG(FT_LOG_ERR,"checkCltDev failed!");
            }

            /* 发送客户端CheckDev状态 */
            WriteInt32(tcp, nRc);
            nNetStatus = FlushSBuff(tcp);
        if (nNetStatus != 0 || nRc != 0) {
            return nNetStatus ? nNetStatus : nRc;
        }
        m_rdisk.devpath = strdup(m_devs[1].diskname);
        m_wdisk.devpath = strdup(m_devs[0].diskname);
    }
    else {

        PRInt32 nStatus = 0;
        nNetStatus = ReadInt32(tcp, &nStatus);
        if (nNetStatus != 0 || nStatus != 0) {
            return nNetStatus ? nNetStatus : nStatus;
        }

        nRc = recvFTDevPair(tcp,m_devs,&block_size);
        if (nRc != 0) {
            FT_LOG(FT_LOG_ERR,"recvFTDevPair failed!");
            return nRc;
        }

        nRc = checkSrvDev(m_devs);
        if (nRc != 0) {
            FT_LOG(FT_LOG_ERR,"checkSrvDev failed!");
        }

        /* 发送服务端CheckDev状态 */
        WriteInt32(tcp, nRc);
        nNetStatus = FlushSBuff(tcp);
        if (nRc != 0 || nNetStatus != 0) {
            return nRc ? nRc : nNetStatus;
        }

        /* 获取客户端CheckDev状态 */
        nNetStatus = ReadInt32(tcp, &nStatus);
        if (nNetStatus != 0 || nStatus != 0) {
            return nNetStatus ? nNetStatus : nStatus;
        }
        m_rdisk.devpath = strdup(m_devs[0].diskname);
        m_wdisk.devpath = strdup(m_devs[1].diskname);
    }

    m_tcpsbuf = NewSBuff(tcp->fd,1800,1800);
    if (!m_tcpsbuf) {
        FT_LOG(FT_LOG_ERR, "NewSBuff error");
        goto END;
    }

    nRc = ft_alloc_mem(block_size);
    if (nRc != FT_OK) {
        FT_LOG(FT_LOG_ERR, "ft_alloc_mem error");
        goto END;
    }

    nRc =  open_device();
    if (nRc != FT_OK) {
        FT_LOG(FT_LOG_ERR, "open_device error");
        goto END;
    }

#ifdef WIN32
    m_wdisk.overlap.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (m_wdisk.overlap.hEvent == NULL) {
        nRc = FT_ERR_UNKNOWN;
        goto END;
    }
    m_rdisk.overlap.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (m_rdisk.overlap.hEvent == NULL) {
        nRc = FT_ERR_UNKNOWN;
        goto END;
    }
#elif !(defined(AIX) || defined(WIN32))
    if (m_wdisk.addr && !m_is_noblock ) {
        memset((char*)m_wdisk.addr + m_wdisk.offset, 0, m_wdisk.device_size - m_wdisk.offset);
    }

    if (m_rdisk.addr && !m_is_noblock) {
        memset((char*)m_rdisk.addr + m_rdisk.offset, 0, m_rdisk.device_size - m_rdisk.offset);
    }
    if ( m_is_server  && !m_is_noblock ) {    
        FT_LOG(FT_LOG_INFO,"In netlink_init");

        if ((nRc = netlink_init()) != FT_OK) {
            FT_LOG(FT_LOG_ERR,"In netlink_init error");
            goto END;
        }
        unsigned long size = netlink_get_recv_size();
        if (size > 0) {
            FT_LOG(FT_LOG_DEBUG, "netlink_init get recv size :%ld",size);
        }
        size = netlink_get_send_ack();
        if (size > 0) {
            FT_LOG(FT_LOG_DEBUG,"netlink_init get send ack :%ld",size);
        }
    }
#endif
END:
    if (FT_OK != cs_sync(nRc)) {
        return FT_ERR_UNKNOWN;
    }
    if (nRc == 0){
        if (m_is_noblock == 1 ) {
            nRc = set_sock_noblock();
            if (nRc != 0){
                return nRc;
            }
        }
        tcp->ft = this;
        m_init_state = FT_INIT_OK;
        FT_LOG(FT_LOG_INFO,"Init FT success! m_wdisk.devpath:%s,m_rdisk.devpath:%s",m_wdisk.devpath,m_rdisk.devpath);
    }
    return nRc;
}

int FTransport::set_sock_noblock() {
    int nRc = FT_OK;
    set_tcp_nodelay(m_tcpsbuf->fd);
#ifdef WIN32
    nRc = WSAEventSelect(m_tcpsbuf->fd,m_tcpsbuf->hEvent,FD_READ | FD_CLOSE );
    if (nRc != 0) {
        FT_LOG(FT_LOG_ERR, "WSAEventSelect failed !");
        LogErrorMsg("");
    }

#elif defined(AIX)
    int b_on = 1;
    if ( 0 != ioctl(m_tcpsbuf->fd, FIONBIO, &b_on))
    {
        FT_LOG(FT_LOG_ERR, "ioctl");
        LogErrorMsg("");
        return FT_ERR_SOCK;
    }
#else
    nRc = epoll_init();
    if (nRc != 0) {
        FT_LOG(FT_LOG_ERR, "epoll_init failed !");
    }

#endif
    return nRc;
}

int FTransport::Send(void* content, unsigned int size) {
    if (m_is_noblock) {
        return ft_send_noblock(content, size);
    }

    if (m_is_server) {
        return	ft_srv_send(content, size);
    }
    else {
        return ft_clt_send(content,size);
    }
}


int FTransport::check_seq(unsigned int nSeq,unsigned int nSubSeq ,struct stFTChannel * channel){
    if ((nSeq == 0 && nSubSeq == 0 )
        || ( channel->subseq == INT_MAX_SIZE && (nSubSeq != 1 || nSeq != channel->seq +1) )
        || (channel->subseq != INT_MAX_SIZE && (channel->subseq +1 != nSubSeq || nSeq != channel->seq))){
            return FT_ERR_UNKNOWN;
    }

    return FT_OK;
}


int FTransport::Recv(void* content, unsigned int buffsize, unsigned int* nRecved)
{
    if (m_is_noblock) {
        //return ft_recv_noblock(content, buffsize,nRecved);
        return ft_recv_noblock_async(content, buffsize,nRecved);
    }

    if (m_is_server) {
        return ft_srv_recv(content, buffsize, nRecved);
    }
    else
    {
        return ft_clt_recv(content, buffsize, nRecved);
    }

}


int FTransport::send_ctrdata(){

    int nRC = FT_OK;
    FTCtrlHeaderSt header = { 0 };
    FTControlPackHeaderSt stHeader = { 0 };

    if (m_total_recv > 0) {
        header.type = FT_CTL_RECV_AMOUNT;
        header.nlen = (PRUint32)((PRUint64)m_total_recv >> 32);
        header.nDataLen = (PRUint32)(m_total_recv & (PRUint64)0xffffffff);
        pack_ctrlheader(&stHeader, &header);
        FT_LOG(FT_LOG_DEBUG,"send ctl total recv :%lld \n", m_total_recv);
        int nRC =	WriteBinary(m_tcpsbuf, (const unsigned char*)&stHeader, sizeof(FTControlPackHeaderSt));
        m_send_ctrl_cnt++;
    }

    if ( m_total_send > 0) {
        header.type = FT_CTL_SEND_AMOUNT;
        header.nlen = (PRUint32)((PRUint64)m_total_send >> 32);
        header.nDataLen = (PRUint32)(m_total_send & (PRUint64)0xffffffff);
        pack_ctrlheader(&stHeader, &header);
        FT_LOG(FT_LOG_DEBUG,"send ctl total send :%lld \n", m_total_send);
        nRC = WriteBinary(m_tcpsbuf, (const unsigned char*)&stHeader, sizeof(FTControlPackHeaderSt));
        m_send_ctrl_cnt++;
    }
    nRC = FlushSBuff(m_tcpsbuf);
    if (nRC != 0) {
        m_init_state = FT_NONE_INIT;
        return FT_ERR_SOCK;
    }
    return FT_OK;
}

#if 0 
int FTransport::recv_ctrdata(){

    PRInt32 nRead = 0;
    int nRC = FT_OK;
    unsigned long bytes_available = 0;

#ifdef WIN32
    nRC = ioctlsocket(m_tcpsbuf->fd,FIONREAD,&bytes_available);

#else
    ioctl(m_tcpsbuf->fd,FIONREAD,&bytes_available);
#endif

    if (bytes_available == 0 && nRC == 0)
        return FT_OK;
    else if (nRC != 0) {
        return FT_ERR_SOCK;
    }
    FTControlPackHeaderSt* header = new FTControlPackHeaderSt();
    m_tcpsbuf->ft = NULL;

    while (bytes_available > 0) {
        int nRc = ReadBinary(m_tcpsbuf, (unsigned char*)header, sizeof(FTControlPackHeaderSt), &nRead);
        if (nRc != WAVETOP_BACKUP_OK) {
            nRC = WAVETOP_BACKUP_CONNECT_DOWN;
            break;
        }
        bytes_available -= sizeof(FTControlPackHeaderSt);

        short type, code;
        unsigned int len;
        VINT16(type, header->szType);
        VINT16(code, header->szCode);
        VINT32(len, header->szLen);
        PRInt64 offset = 0;
        if (type == FT_CTL_RECV_AMOUNT) {
            nRc = ReadInt64(m_tcpsbuf, &offset);
            if (nRc != WAVETOP_BACKUP_OK) {
                break;
            }
            if (offset > m_send_ack)
                m_send_ack = offset;
        }
        else if (type == FT_CTL_SEND_AMOUNT) {
            nRc = ReadInt64(m_tcpsbuf, &offset);
            if (nRc != WAVETOP_BACKUP_OK) {
                break;
            }

            if (offset > m_total_recv){
                m_total_recv = offset;

            }
        }
        bytes_available -=  8;
    }


EXIT:	
    if (header)
        delete header;
    m_tcpsbuf->ft = this;
    return nRC;
}
#endif


int FTransport::recv_ctrdata() {

    PRInt32 nRead = 0;
    int nRC = FT_OK;

    FTControlPackHeaderSt header = { 0 };
    FTCtrlHeaderSt   ctr = { 0 };

    int nCnt = 1;
    for (; nCnt > 0; nCnt--) {
        int nRc = ReadBinary(m_tcpsbuf, (unsigned char*)&header, sizeof(FTControlPackHeaderSt), &nRead);
        if (nRc != 0) {
            nRC = FT_ERR_SOCK;
            break;
        }
        m_recv_ctrl_cnt++;
        unpack_ctrlheader(&header, &ctr);
        PRUint64 offset = 0;
        offset = (PRUint64)(((PRUint64)ctr.nlen) << 32) + ctr.nDataLen;
        if (ctr.type == FT_CTL_RECV_AMOUNT) {

            if (offset > m_send_ack)
                m_send_ack = offset;
        }
        else if (ctr.type == FT_CTL_SEND_AMOUNT) {
            if (offset > m_total_recv) {
                m_total_recv = offset;

            }
        }
    }

    return nRC;
}

int FTransport::ftread(struct stFTDisk* disk, char* buff, int buff_size) {

#ifndef WIN32
    int noff = 0;
    int nTotal = 0;
    while (buff_size > 0) {
        int nRead = read((long)disk->fd, buff + noff, buff_size);
        if (nRead == -1) {
            LogErrorMsg(TEXT("ftread() read:"));
            return -1;
        }
        if (nRead == 0)
            break;
        nTotal += nRead;
        noff += nRead;
        buff_size -= nRead;
    }
    return nTotal;
#else
    DWORD nRead = 0;
    if (!ReadFile(disk->fd, buff, buff_size, &nRead, NULL))
    {
        LogErrorMsg(TEXT("ftread() ReadFile:"));
        FT_LOG(FT_LOG_ERR, "offset :%ld, size :%d", disk->offset, buff_size);
        return -1;
    }
    return nRead;
#endif

}

int FTransport::ftread(struct stFTDisk* disk, char** buff, unsigned int buff_size, PRUint64 offset) {
    if (offset + buff_size > disk->device_size)
        return FT_OK;
#ifndef WIN32
    *buff = (char*)disk->addr + offset;
#endif
    return buff_size;
}

/* 返回实际写入的大小 */
int FTransport::ftwrite(void* fd, char* buff, int buff_size) {
#ifndef WIN32
#ifndef USE_MMAP
    return write((long)fd, buff, buff_size);
#else
    memcpy(fd, buff, buff_size);
#endif
    return buff_size;
#else

#endif
    return FT_OK;
}

int FTransport::ft_asyncwrite(struct stFTDisk* disk, char* buff, int buff_size) {
#ifdef WIN32
    DWORD nWrite;
    if (disk->wait) {  /* 等待上一次写入完成 */

        if (WaitForSingleObject(disk->overlap.hEvent, INFINITE)) {
            LogErrorMsg("WaitForSingleObject:");
            return FT_ERR_UNKNOWN;	
        }      

        if (! GetOverlappedResult(disk->fd,&(disk->overlap),&nWrite,FALSE) ) {
            LogErrorMsg(TEXT("GetOverlappedResult Error"));
            return FT_ERR_UNKNOWN;
        }
        ResetEvent(disk->overlap.hEvent);
        disk->wait = 0;
        if (0 != send_ctrdata(&m_last_send)) {
            FT_LOG(FT_LOG_ERR,"ft_asyncwrite::send_ctrdata failed");
            return FT_ERR_SOCK;
        }
    }
    //	disk->overlap.Offset = disk->offset;
    disk->overlap.Offset = (PRUint32)(disk->offset & 0xFFFFFFFF);
    disk->overlap.OffsetHigh = (PRUint32)(disk->offset >> 32);

    BOOL nResult = WriteFile(disk->fd, buff, buff_size, &nWrite, &(disk->overlap));
    if (!nResult) {
        if (GetLastError() == ERROR_IO_PENDING)
            disk->wait = 1;
        else {
            LogErrorMsg(TEXT("WriteFile Error :"));
            return FT_ERR_UNKNOWN;
        }
    }
    else {
        FT_LOG(FT_LOG_DEBUG,"WriteFile do not wait");
        disk->wait = 0;
        if (0 != send_ctrdata(&m_last_send))
        {
            FT_LOG(FT_LOG_ERR,"ft_asyncwrite::send_ctrdata failed");
        }
    }

#else
    if (m_is_server) {
        memcpy((char*)disk->addr + disk->offset,buff,buff_size);
        return FT_OK;
    }
#if HAVE_FILE_AIO
#if defined(AIX)
    if (!disk->iocb) {
        disk->iocb = malloc(sizeof(struct aiocb64));
        if (disk->iocb == NULL) {
            return FT_ERR_UNKNOWN;
        }
        memset(disk->iocb, 0, sizeof(struct aiocb64));
        ((struct aiocb64*)(disk->iocb))->aio_fildes =(int)(long)disk->fd;
        ((struct aiocb64*)(disk->iocb))->aio_lio_opcode = LIO_WRITE;
    }

    if (disk->wait) {
        struct aiocb64* list[1];
        struct timespec timeout; 
        timeout.tv_sec = 300;
        timeout.tv_nsec = 0;
        int nent = 1, n = 0,nRt;
        list[0] = (struct aiocb64*)disk->iocb;
        do {
            n = aio_suspend64(list,nent,&timeout);
            if (n == -1 && errno != EAGAIN) {
                FT_LOG(FT_LOG_ERR,"aio_suspend failed with error code=%d",errno);
                FT_LOG(FT_LOG_ERR,"aio_suspend iocb offset%lld,bytes:%d",list[0]->aio_offset,list[0]->aio_nbytes);
                return FT_ERR_WRITE;
            }
        } while (n == -1);

        if ((nRt = aio_error64(list[0])) != 0 ) {
            FT_LOG(FT_LOG_ERR,"aio_error failed with error code=%d",nRt);
            return FT_ERR_WRITE;
        }
        if ((nRt = aio_return64(list[0])) != m_last_send.nlen ) {
            FT_LOG(FT_LOG_ERR,"aio_return64 failed with error return=%d",nRt);
            return FT_ERR_WRITE;
        }

        disk->wait = 0;
        if (0 != send_ctrdata(&m_last_send)) {
            return FT_ERR_SOCK;
        }
    }

    struct aiocb64* pstiocb = (struct aiocb64*)disk->iocb;
    pstiocb->aio_buf = buff;
    pstiocb->aio_nbytes = buff_size;
    pstiocb->aio_offset = disk->offset;
    int ft_err;
    do {
        if ((ft_err = aio_write64(pstiocb)) == 0 ){
            disk->wait = 1;
        }
        else {
            int nerr = errno;
            if (nerr != EAGAIN) {
                FT_LOG(FT_LOG_ERR, "aio_write64");
                LogErrorMsg("");
                return FT_ERR_WRITE;
            }
            FT_LOG(FT_LOG_ERR, "aio_write64 : %s",strerror(nerr));
        }
    } while (ft_err != 0);
#else
    if (!disk->iocb) {
        disk->ctx = NULL;
        if (0 != io_setup(1, &disk->ctx)) {
            return FT_ERR_UNKNOWN;
        }
        disk->iocb = malloc(sizeof(struct iocb));
        if (disk->iocb == NULL) {
            return FT_ERR_UNKNOWN;
        }
        memset(disk->iocb, 0, sizeof(struct iocb));
    }

    if (disk->wait) {
        struct io_event events[1];
        int n;
        do {
            n = io_getevents(disk->ctx, 1, 1, events, NULL);
            if (n != 1 && errno != EINTR ) {
                return FT_ERR_WRITE;
            }
        } while(n != 1);

        disk->wait = 0;
        if (0 != send_ctrdata(&m_last_send)) {
            return FT_ERR_SOCK;
        }
    }

    struct iocb* pstiocb = (struct iocb*)disk->iocb;
    struct iocb  *piocb[1];

    pstiocb->aio_data = (uint64_t)(uintptr_t)buff;
    pstiocb->aio_lio_opcode = IOCB_CMD_PWRITE;
    pstiocb->aio_fildes =(int)(long)disk->fd;
    pstiocb->aio_buf = (uint64_t)(uintptr_t)buff;
    pstiocb->aio_nbytes = buff_size;
    pstiocb->aio_offset = disk->offset;
    piocb[0] = pstiocb;
    int ft_err = 0;
    do {
        if ((ft_err = io_submit(disk->ctx, 1, piocb)) == 1 ){
            disk->wait = 1;
        }
        else {
            int nerr = errno;
            if (nerr == EAGAIN || nerr == ENOSYS || nerr != EINTR) {
                FT_LOG(FT_LOG_ERR, "io_submit");
                return FT_ERR_WRITE;
            }
            FT_LOG(FT_LOG_ERR, "io_submit : %s",strerror(nerr));
        }
    } while (ft_err != 1);


#endif  //AIX
#else
    ssize_t nWrite;
    nWrite = write((long)disk->fd, buff, buff_size);
    if (nWrite == -1) {
        return FT_ERR_WRITE;
    }

#endif  // HAVE_FILE_AIO
#endif // WIN32
    return FT_OK;
}

int FTransport::ft_asyncread(struct stFTDisk* disk,struct stFTChannel* channel,int* nRead) {

    int readed_cnt = 0;  /* 已经读取的次数 */
    *nRead = 0;
    char* pszCur = (char*) channel->buff;
    int read_cnt = 0;
    while (m_ctrl_queue.size() > 0) {

#ifdef WIN32
        DWORD nReaded;
        if (disk->wait) {  /* 等待上一次读取完成 */    
            if (disk->wait == 1) {
                if (! GetOverlappedResult(disk->fd,&(disk->overlap),&nReaded,TRUE) ) {
                    LogErrorMsg(TEXT("GetOverlappedResult Error"));
                    return FT_ERR_UNKNOWN;
                }
                ResetEvent(disk->overlap.hEvent);
                FT_LOG(FT_LOG_DEBUG,"GetOverlappedResult offset:%u",disk->overlap.Offset);
            }
            disk->wait = 0;
            m_last_read.type = FT_CTL_RECV_ACK;
            if (0 != send_ctrdata(&m_last_read)) {
                return FT_ERR_SOCK;
            }
            readed_cnt ++;
            /* 交换到上次读取的buff */
            if (read_cnt == 0)
                swap(channel);
            channel->nTotal = m_last_read.nDataLen;
            m_total_recv += m_last_read.nlen;
            m_ctrl_queue.pop();
            continue;
        }

        FTCtrlHeaderSt& data_header = m_ctrl_queue.front();
        if (channel->block_size < data_header.nlen) {
            FT_LOG(FT_LOG_ERR,"invalid header len!");
            return -1;
        }
        if (disk->offset + data_header.nlen > disk->device_size) {
            disk->offset = channel->block_size;
        }

        disk->overlap.Offset = (PRUint32)(disk->offset & 0xFFFFFFFF);
        disk->overlap.OffsetHigh = (PRUint32)(disk->offset >> 32);

        if (++read_cnt == 2) {
            pszCur = (char*) (channel->buff == channel->interlbuff ? channel->interlbuff1 : channel->interlbuff) ; 
        }

        BOOL nResult = ReadFile(disk->fd, pszCur,  data_header.nlen, &nReaded , &(disk->overlap));
        if (!nResult) {
            if (GetLastError() == ERROR_IO_PENDING){
                disk->wait = 1;
            }
            else {
                LogErrorMsg(TEXT("WriteFile Error :"));
                return FT_ERR_UNKNOWN;
            }
        }
        else {
            disk->wait = 2;
            FT_LOG(FT_LOG_DEBUG,"WriteFile do not wait");
        }
        memcpy(&m_last_read,&data_header,sizeof(FTCtrlHeaderSt));
        disk->offset += data_header.nlen;

#else 
        if (m_is_server) {
            FTCtrlHeaderSt& data_header = m_ctrl_queue.front();
            if (channel->block_size < data_header.nlen) {
                FT_LOG(FT_LOG_ERR,"invalid header len!");
                return -1;
            }
            if (disk->offset + data_header.nlen > disk->device_size) {
                disk->offset = channel->block_size;
            }
            memcpy(pszCur,(char*)disk->addr + disk->offset,data_header.nDataLen);
            channel->nTotal = data_header.nDataLen;
            *nRead = data_header.nDataLen;

            data_header.type = FT_CTL_RECV_ACK;
            m_total_recv += data_header.nlen;
            disk->offset += data_header.nlen;
            if (0 != send_ctrdata(&data_header)) {
                return FT_ERR_SOCK;
            }

            m_ctrl_queue.pop();
            return FT_OK;
        }
#if HAVE_FILE_AIO
#if defined(AIX)
        if (!disk->iocb) {
            disk->iocb = malloc(sizeof(struct aiocb64));
            if (disk->iocb == NULL) {
                return FT_ERR_UNKNOWN;
            }
            memset(disk->iocb, 0, sizeof(struct aiocb64));
            ((struct aiocb64*)(disk->iocb))->aio_fildes =(int)(long)disk->fd;
            ((struct aiocb64*)(disk->iocb))->aio_lio_opcode = LIO_READ;
        }

        if (disk->wait) {
            struct aiocb64 * list[1];
            struct timespec timeout; 
            timeout.tv_sec = 300;
            timeout.tv_nsec = 0;
            int nent = 1, n = 0,nRt;
            list[0] = (struct aiocb64*)disk->iocb;
            do {
                n = aio_suspend64(list,nent,&timeout);
                if (n == -1 && errno != EINTR) {
                    FT_LOG(FT_LOG_ERR,"aio_suspend failed with error code=%d,ret=%d",errno,n);
                    FT_LOG(FT_LOG_ERR,"aio_suspend iocb offset%lld,bytes:%d",list[0]->aio_offset,list[0]->aio_nbytes);
                    return FT_ERR_READ;
                }
            } while (n == -1);

            if ((nRt = aio_error64(list[0])) != 0 ) {
                FT_LOG(FT_LOG_ERR,"aio_error failed with error code=%d",errno);
                return FT_ERR_READ;
            }
            if ((nRt = aio_return64(list[0])) != m_last_read.nlen ) {
                FT_LOG(FT_LOG_ERR,"aio_return64 failed with error code=%d",errno);
                return FT_ERR_READ;
            }

            disk->wait = 0;
            if (read_cnt == 0)
                swap(channel);
            channel->nTotal = m_last_read.nDataLen;
            m_total_recv += m_last_read.nlen;
            m_last_read.type = FT_CTL_RECV_ACK;
            if ( 0 != send_ctrdata(&m_last_read)) {
                return FT_ERR_SOCK;
            }
            readed_cnt++;
            m_ctrl_queue.pop();
            continue;
        }

        if (++read_cnt == 2) {
            pszCur = (char*) (channel->buff == channel->interlbuff ? channel->interlbuff1 : channel->interlbuff) ; 
        }
        struct aiocb64* pstiocb = (struct aiocb64*)disk->iocb;
        FTCtrlHeaderSt& data_header = m_ctrl_queue.front();
        if (channel->block_size < data_header.nlen) {
            FT_LOG(FT_LOG_ERR,"invalid header len!");
            return -1;
        }
        if (disk->offset + data_header.nlen > disk->device_size) {
            disk->offset = channel->block_size;
        }
        pstiocb->aio_buf = pszCur;
        pstiocb->aio_nbytes = data_header.nlen;
        pstiocb->aio_offset = disk->offset;
        int ft_err;
        do {
            if ((ft_err = aio_read64(pstiocb)) == 0 ){
                disk->wait = 1;
                memcpy(&m_last_read,&data_header,sizeof(FTCtrlHeaderSt));
                disk->offset += data_header.nlen;
            }
            else {
                int nerr = errno;
                if ( nerr != EAGAIN ) {
                    FT_LOG(FT_LOG_ERR, "aio_read64");
                    LogErrorMsg("");
                    return FT_ERR_READ;
                }
            }
        } while (ft_err != 0);
#else

        if (!disk->iocb) {
            disk->ctx = NULL;
            if (0 != io_setup(1, &disk->ctx)) {
                return FT_ERR_UNKNOWN;
            }
            disk->iocb = malloc(sizeof(struct iocb));
            if (disk->iocb == NULL) {
                return FT_ERR_UNKNOWN;
            }
            memset(disk->iocb, 0, sizeof(struct iocb));
        }

        if (disk->wait) {
            struct io_event events[1];
            int n;
            do {
                n = io_getevents(disk->ctx, 1, 1, events, NULL);
                if (n != 1 && errno != EINTR) {
                    return FT_ERR_READ;
                }
            } while (n != 1);

            disk->wait = 0;
            if (read_cnt == 0)
                swap(channel);
            channel->nTotal = m_last_read.nDataLen;
            m_total_recv += m_last_read.nlen;
            m_last_read.type = FT_CTL_RECV_ACK;
            if ( 0 != send_ctrdata(&m_last_read)) {
                return FT_ERR_SOCK;
            }
            readed_cnt++;
            m_ctrl_queue.pop();
            continue;
        }

        if (++read_cnt == 2) {
            pszCur = (char*) (channel->buff == channel->interlbuff ? channel->interlbuff1 : channel->interlbuff) ; 
        }
        struct iocb* pstiocb = (struct iocb*)disk->iocb;
        struct iocb  *piocb[1];
        FTCtrlHeaderSt& data_header = m_ctrl_queue.front();
        if (channel->block_size < data_header.nlen) {
            FT_LOG(FT_LOG_ERR,"invalid header len!");
            return -1;
        }
        if (disk->offset + data_header.nlen > disk->device_size) {
            disk->offset = channel->block_size;
        }

        pstiocb->aio_data = (uint64_t)(uintptr_t)pszCur;
        pstiocb->aio_lio_opcode = IOCB_CMD_PREAD;
        pstiocb->aio_fildes =(int)(long)disk->fd;
        pstiocb->aio_buf = (uint64_t)(uintptr_t)pszCur;
        pstiocb->aio_nbytes = data_header.nlen;
        pstiocb->aio_offset = disk->offset;
        piocb[0] = pstiocb;
        int ft_err = 0;
        do {
            if ((ft_err = io_submit(disk->ctx, 1, piocb)) == 1 ){
                disk->wait = 1;
                memcpy(&m_last_read,&data_header,sizeof(FTCtrlHeaderSt));
                disk->offset += data_header.nlen;
            }
            else {
                int nerr = errno;
                if (nerr == EAGAIN || nerr == ENOSYS || nerr != EINTR) {
                    FT_LOG(FT_LOG_ERR, "io_submit");
                    LogErrorMsg("");
                    return FT_ERR_READ;
                }
            }
        } while (ft_err != 1);
#endif //AIX
#else
        ssize_t nRead;
        if (disk->offset + data_header.nlen > disk->device_size) {
            if (0 != ftseek(disk->fd,channel->block_size))
                return FT_ERR_SEEK;
        }
        nRead = read((long)disk->fd), pszCur, data_header.nlen);
        if (nRead == -1) {
            return FT_ERR_READ;
        }
        readed_cnt++;
#endif // AIO
#endif //WIN32
        if (readed_cnt)
            break;
    }
    if (readed_cnt != 0) {
        *nRead = channel->nTotal;
    }
    return FT_OK;	

}


void FTransport::swap(stFTChannel* channel){
    if (channel->buff == channel->interlbuff) {
        channel->buff = channel->interlbuff1;
    }
    else {
        channel->buff = channel->interlbuff;
    }

}


int  FTransport::ftseek(void* fd, unsigned long offset) {
#ifdef WIN32
    LARGE_INTEGER off;
    off.QuadPart = offset;
    DWORD nResult = SetFilePointer(fd, off.LowPart, &off.HighPart, FILE_BEGIN);
    if (nResult == INVALID_SET_FILE_POINTER) {
        return FT_ERR_SEEK;
    }
#else
    if (-1 == lseek((long)fd, offset, SEEK_SET)) {
        return FT_ERR_SEEK;
    }
#endif
    return FT_OK;
}

int FTransport::wait_ch_writable() {
    if (m_is_server) {
#if !(defined(AIX) || defined(WIN32))
        int nretry = FT_RETRY_TIMES;
        while (true)
        {
            unsigned long size = netlink_get_send_ack();
            if (size > 0) {
                m_send_ack += size;
                FT_LOG(FT_LOG_DEBUG, "clt read %u \n", size);
            }
            if (m_total_send - m_send_ack <= (m_wdisk.device_size - (m_wbuff.block_size * 2) ))
                break;
            else {
                nretry--;
                if (nretry == 0)
                    return FT_ERR_TIMEOUT;
                ft_sleep(FT_TIME_SLEEP);
            }
        }
        if (m_wdisk.offset + m_wbuff.block_size > m_wdisk.device_size)
        {
            m_wdisk.offset = m_wbuff.block_size;
#ifndef USE_MMAP
            ftseek(m_wdisk.fd, m_wbuff.block_size);
#endif

        }

#endif
    }
    else {
        /*if (!(m_wbuff.subseq % ((m_wdisk.device_size / m_wbuff.block_size / 2) + 1))) {
        recv_ctrdata();
        } */
        int nretry = 3;
        while (nretry-- > 0)
        {

            if (m_total_send - m_send_ack <= (m_wdisk.device_size - (m_wbuff.block_size * 2)))
                break;
            else {
                recv_ctrdata();
                ft_sleep(FT_TIME_SLEEP);
            }
        }

        if (m_wdisk.offset + m_wbuff.block_size > m_wdisk.device_size)
        {
            ftseek(m_wdisk.fd, m_wbuff.block_size);
            m_wdisk.offset = m_wbuff.block_size;
        }
    }
    return FT_OK;

}
int FTransport::wait_ch_readable() {

    int nRetry = 0;
    if (m_is_server) {
#if !(defined(AIX) || defined(WIN32))
        while (m_left_to_read < m_rbuff.block_size) {
            unsigned long nrecved = netlink_get_recv_size();
            if (nrecved > 0)
                m_left_to_read += nrecved;

            if (m_left_to_read < m_rbuff.block_size) {
                FT_LOG(FT_LOG_DEBUG, "wait for clt send \n ");
                ft_sleep(FT_TIME_SLEEP);
                nRetry += 1;
                if (nRetry > FT_RETRY_TIMES)
                    return FT_ERR_READ;
            }
            else {
                nRetry = 0;
                break;
            }

        }
        if (m_rdisk.offset + m_rbuff.block_size > m_rdisk.device_size) {
            m_rdisk.offset = m_rbuff.block_size;
        }
#endif
    }
    else {
        if ((m_rdisk.offset + m_rbuff.block_size)  > m_rdisk.device_size) {
            m_rdisk.offset = m_rbuff.block_size;
            if (FT_OK != ftseek(m_rdisk.fd, m_rdisk.offset)) {
                FT_LOG(FT_LOG_ERR, "ftseek disk:%s offset %ld,failed !", m_rdisk.devpath, m_rdisk.offset);
                FT_LOG(FT_LOG_ERR, "\n");
            }

        }

        while (true) {

            if (m_total_recv - m_recv_ack >= m_rbuff.block_size) {
                nRetry = 0;
                break;
            }

            if (FT_OK != recv_ctrdata()) {
                return FT_ERR_SOCK;
            }

            if (nRetry >= FT_RETRY_TIMES) {
                return FT_ERR_TIMEOUT;
            }
            else {
                nRetry++;
                ft_sleep(FT_TIME_SLEEP);
            }
        }
    }
    return FT_OK;
}

int FTransport::ft_clt_send(void* content, unsigned int size) {
    int nRC = FT_OK;
    unsigned int pack_size = 0;
    DWORD nWrite = 0;
    int left = 0;
    FTDataHeaderSt stHeader = { 0 };
    char* buff = (char*)content;

    if ((size == 0  ||  content == NULL) && m_wbuff.nFlush != 1) {
        return FT_ERR_UNKNOWN;
    }

    if (m_wbuff.nInpos == 0)
        m_wbuff.nInpos += sizeof(FTDataHeaderSt);

    if (  size < FT_CHNANEL_SPACE_LEFT(m_wbuff) && m_wbuff.nFlush != 1) {	
        memcpy((char*)m_wbuff.buff + m_wbuff.nInpos,content,size);
        m_wbuff.nTotal += size;
        m_wbuff.nInpos += size;
        return FT_OK;
    }
    else if (m_wbuff.nFlush != 1){
        left = size - (FT_CHNANEL_SPACE_LEFT(m_wbuff));
        if (FT_CHNANEL_SPACE_LEFT(m_wbuff) > 0)
            memcpy((char*)m_wbuff.buff + m_wbuff.nInpos,content,(FT_CHNANEL_SPACE_LEFT(m_wbuff)));
        content = (char*)content + FT_CHNANEL_SPACE_LEFT(m_wbuff); /* 指针偏移到下次位置 */
        m_wbuff.nTotal = m_wbuff.block_size - sizeof(FTDataHeaderSt); 
    }
    else {
        m_wbuff.nFlush = 0;
        if (m_wbuff.nTotal == 0)
            return FT_OK;
    }

    do {

        if (wait_ch_writable() != FT_OK) {
            return FT_ERR_WRITE;
        }

        char* pszBlock = (char*)m_wbuff.buff;
        DWORD nWrite = 0;	
        pack_size = sizeof(FTDataHeaderSt) + m_wbuff.nTotal;
        XINT32(m_wbuff.subseq, stHeader.szSubSeq);
        XINT32(pack_size, stHeader.szTotal);
        XINT32(m_wbuff.seq, stHeader.szSeq);

        memcpy(pszBlock, &stHeader, sizeof(FTDataHeaderSt));

        if (FT_CHNANEL_SPACE_LEFT(m_wbuff) > 0)
            memset(pszBlock + sizeof(FTDataHeaderSt) + m_wbuff.nTotal, 0, FT_CHNANEL_SPACE_LEFT(m_wbuff));

        if (FT_OK != ft_asyncwrite(&m_wdisk, pszBlock, m_wbuff.block_size) ) {
            return FT_ERR_WRITE;
        }

        FT_LOG(FT_LOG_DEBUG,"send seq:%d,nTotal:%d offset:%ld\n", m_wbuff.subseq, m_wbuff.nTotal,m_wdisk.offset);

        m_wdisk.offset += m_wbuff.block_size;
        m_total_send += m_wbuff.block_size;

        if (m_wbuff.subseq == INT_MAX_SIZE) {
            m_wbuff.seq++;
            m_wbuff.subseq = 1;
        }
        else {
            m_wbuff.subseq++;
        }

        if (m_wbuff.buff == m_wbuff.interlbuff) {
            m_wbuff.buff = m_wbuff.interlbuff1;
        }
        else {
            m_wbuff.buff = m_wbuff.interlbuff;
        }

        m_wbuff.nInpos = sizeof(FTDataHeaderSt);
        m_wbuff.nTotal = 0;

        if (left > 0 && left < FT_CHNANEL_SPACE_LEFT(m_wbuff)) {
            memcpy((char*)m_wbuff.buff + m_wbuff.nInpos,(char*)content,left);		
            //m_wbuff.nInpos = sizeof(FTDataHeaderSt);
            m_wbuff.nInpos += left;
            m_wbuff.nTotal = left;
            left = 0;
        }
        else if(left > 0) { /* 剩余大小 >= 通道总容量 */
            memcpy((char*)m_wbuff.buff + m_wbuff.nInpos,(char*)content ,FT_CHNANEL_SPACE_LEFT(m_wbuff));		
            left -= FT_CHNANEL_SPACE_LEFT(m_wbuff);
            content = (char*)content + FT_CHNANEL_SPACE_LEFT(m_wbuff);
            m_wbuff.nTotal = FT_CHNANEL_SPACE_LEFT(m_wbuff);
            continue;

        }

    }
    while (left > 0);

    return nRC;
}
int FTransport::ft_srv_send(void* content, unsigned int size) {
#ifndef WIN32

    FTDataHeaderSt stHeader      = { 0 };
    DWORD          nWrite        = 0;
    unsigned int   pack_size     = 0;
    int left = 0;
    int nRC = FT_OK;

    if ((size == 0 || content == NULL ) && m_wbuff.nFlush != 1) {
        return FT_ERR_UNKNOWN;
    }

    if (m_wbuff.nInpos == 0)
        m_wbuff.nInpos += sizeof(FTDataHeaderSt);

    if (  size < FT_CHNANEL_SPACE_LEFT(m_wbuff) && m_wbuff.nFlush != 1) {	
        memcpy((char*)m_wbuff.buff + m_wbuff.nInpos,content,size);
        m_wbuff.nTotal += size;
        m_wbuff.nInpos += size;
        return FT_OK;
    }
    else if (m_wbuff.nFlush != 1){
        left = size - (FT_CHNANEL_SPACE_LEFT(m_wbuff));
        if (FT_CHNANEL_SPACE_LEFT(m_wbuff) > 0)
            memcpy((char*)m_wbuff.buff + m_wbuff.nInpos,content,(FT_CHNANEL_SPACE_LEFT(m_wbuff)));
        content = (char*)content + FT_CHNANEL_SPACE_LEFT(m_wbuff); /* 指针偏移到下次位置 */
        m_wbuff.nTotal = m_wbuff.block_size - sizeof(FTDataHeaderSt); 
    }
    else {
        m_wbuff.nFlush = 0;
        if (m_wbuff.nTotal == 0)
            return FT_OK;
    }

    do {

        char* pszBlock = (char*)m_wbuff.buff;

        if (wait_ch_writable() != FT_OK) {
            return FT_ERR_WRITE;
        }

        XINT32(m_wbuff.subseq, stHeader.szSubSeq);
        XINT32(m_wbuff.seq, stHeader.szSeq);

        pack_size = sizeof(FTDataHeaderSt) + m_wbuff.nTotal;
        XINT32(pack_size, stHeader.szTotal);
        memcpy(pszBlock, &stHeader, sizeof(FTDataHeaderSt));

        if (m_wbuff.block_size - pack_size > 0)
            memset(pszBlock + pack_size, 0, m_wbuff.block_size - pack_size);
#ifdef USE_MMAP			
        int nResult = ftwrite((char*)m_wdisk.addr + m_wdisk.offset,pszBlock, m_wbuff.block_size );
#else
        int nResult = ftwrite(m_wdisk.fd,pszBlock, m_wbuff.block_size );
#endif
        if (nResult <= 0) {
            FT_LOG(FT_LOG_ERR, "ftwrite :%s failed", m_wdisk.devpath);
            return FT_ERR_WRITE;
        }

        m_wdisk.offset += m_wbuff.block_size;
        m_total_send += m_wbuff.block_size;

        if (FT_OK != send_ctrdata()) {
            FT_LOG(FT_LOG_ERR, "send_ctrdata :%s failed", m_wdisk.devpath);
            return FT_ERR_WRITE;
        }

        if (m_wbuff.subseq == INT_MAX_SIZE) {
            m_wbuff.seq++;
            m_wbuff.subseq = 1;
        }
        else {
            m_wbuff.subseq++;		
        }

        swap(&m_wbuff);

        m_wbuff.nInpos = sizeof(FTDataHeaderSt);
        m_wbuff.nTotal = 0;

        if (left > 0 && left < FT_CHNANEL_SPACE_LEFT(m_wbuff)) {
            memcpy((char*)m_wbuff.buff + m_wbuff.nInpos,(char*)content,left);		
            //m_wbuff.nInpos = sizeof(FTDataHeaderSt);
            m_wbuff.nInpos += left;
            m_wbuff.nTotal = left;
            left = 0;
        }
        else if(left > 0) { /* 剩余大小 >= 通道总容量 */
            memcpy((char*)m_wbuff.buff + m_wbuff.nInpos,(char*)content ,FT_CHNANEL_SPACE_LEFT(m_wbuff));		
            left -= FT_CHNANEL_SPACE_LEFT(m_wbuff);
            content = (char*)content + FT_CHNANEL_SPACE_LEFT(m_wbuff);
            m_wbuff.nTotal = FT_CHNANEL_SPACE_LEFT(m_wbuff);
            continue;
        }
    }
    while (left > 0);

    return nRC;
#else
    return FT_OK;
#endif

}

int FTransport::ft_clt_recv(void* content, unsigned int buffsize, unsigned int* nRecved) {

    int nRetry = 0;
    *nRecved = 0;
    int nleft = 0;
    FTDataHeaderSt stHeader = { 0 };

    if (buffsize == 0 || content == NULL) {
        return FT_ERR_UNKNOWN;
    }

    if (buffsize <= m_rbuff.nTotal ) {
        memcpy(content,(char*)m_rbuff.buff + m_rbuff.nInpos, buffsize);
        m_rbuff.nTotal -= buffsize;
        m_rbuff.nInpos += buffsize;
        *nRecved = buffsize;
        return FT_OK;
    }
    else if (m_rbuff.nTotal > 0) {
        memcpy(content, (char*)m_rbuff.buff + m_rbuff.nInpos, m_rbuff.nTotal);
        content = (void*)((char*)content + m_rbuff.nTotal);
        *nRecved += m_rbuff.nTotal;
    }
    nleft = buffsize - m_rbuff.nTotal;
    m_rbuff.nTotal = 0;
    m_rbuff.nInpos = sizeof(FTDataHeaderSt);

    do 
    {

        int nRecv = 0;
        if (wait_ch_readable() != FT_OK) {
            return FT_ERR_READ;
        }
        unsigned int nSeq, nSubSeq, nTotal;
        int nRead = ftread(&m_rdisk,(char*)m_rbuff.buff, m_rbuff.block_size);
        if (nRead != m_rbuff.block_size) {
            unsigned long off = 0;
#ifndef WIN32
            off = lseek((long)m_rdisk.fd, 0, SEEK_CUR);
#endif
            FT_LOG(FT_LOG_ERR, "ftread %s Error need read %d ,real read %d ,offset :%ld ,m_rdisk offset :%ld", m_rdisk.devpath, m_rbuff.block_size, nRead,off,m_rdisk.offset);
            FT_LOG(FT_LOG_ERR, " m_rdisk device_size %ld", m_rdisk.device_size);
            FT_LOG(FT_LOG_ERR, "\n");
            return FT_ERR_UNKNOWN;
        }

        memcpy(&stHeader, m_rbuff.buff, sizeof(FTDataHeaderSt));
        VINT32(nSeq, stHeader.szSeq);
        VINT32(nSubSeq, stHeader.szSubSeq)
            VINT32(nTotal, stHeader.szTotal);

        if (check_seq(nSeq, nSubSeq,&m_rbuff) != FT_OK || nTotal == 0) {
            if (nRetry > FT_RETRY_TIMES)
                return FT_ERR_UNKNOWN;

            DWORD offset = 0;
            offset = ftseek(m_rdisk.fd, m_rdisk.offset);
            if (offset != FT_OK) {
                FT_LOG(FT_LOG_ERR, "lseek:%s Error",m_rdisk.devpath);
                return FT_ERR_SEEK;
            }
            FT_LOG(FT_LOG_ERR,"\nwait for new data, offset subseq:%d,m_rbuff.subseq:%d,  %ld\n ", nSubSeq, m_rbuff.subseq, m_rdisk.offset);		
            ft_sleep(FT_TIME_SLEEP);
            nRetry += 1;
            continue;
        }

        m_rbuff.seq = nSeq;
        m_rbuff.subseq = nSubSeq;
        m_rdisk.offset += m_rbuff.block_size;		
        m_recv_ack += m_rbuff.block_size;

        nRecv = nTotal - sizeof(FTDataHeaderSt);
        m_rbuff.nInpos = sizeof(FTDataHeaderSt);
        m_rbuff.nTotal = nRecv;

        if (nleft > 0 && nleft <= nRecv) {
            memcpy(content,(char*)m_rbuff.buff + m_rbuff.nInpos,nleft);
            m_rbuff.nInpos += nleft;
            m_rbuff.nTotal -= nleft;
            *nRecved += nleft;
            nleft  = 0 ;
        }
        else if (nleft > 0) {
            memcpy(content,(char*)m_rbuff.buff + m_rbuff.nInpos,nRecv);
            content = (void*)( (char*)content + nRecv);
            nleft -= nRecv;
        }

    }
    while( nleft > 0);
    *nRecved = buffsize;
    return FT_OK;
}

int FTransport::ft_srv_recv(void* content, unsigned int buffsize, unsigned int* nRecved) {
#if !(defined(AIX) || defined(WIN32))

    FTDataHeaderSt stHeader = { 0 };
    unsigned int nRecvSize = buffsize;
    int nRetry = 0;
    int nleft = 0;
    *nRecved = 0;

    if (content == NULL || buffsize == 0) {
        return FT_OK;
    }

    if (buffsize <= m_rbuff.nTotal ) {
        memcpy(content,(char*)m_rbuff.buff + m_rbuff.nInpos, buffsize);
        m_rbuff.nTotal -= buffsize;
        m_rbuff.nInpos += buffsize;
        *nRecved = buffsize;
        return FT_OK;
    }
    else if (m_rbuff.nTotal > 0) {
        memcpy(content, (char*)m_rbuff.buff + m_rbuff.nInpos, m_rbuff.nTotal);
        content = (void*)((char*)content + m_rbuff.nTotal);
        *nRecved += m_rbuff.nTotal;
    }
    nleft = buffsize - m_rbuff.nTotal;
    m_rbuff.nTotal = 0;
    m_rbuff.nInpos = 0;

    do {	

        if (wait_ch_readable() != FT_OK) {
            return FT_ERR_READ;
        }

        int nRecv = 0;
        char* pos = NULL;
        unsigned int nSeq, nSubSeq, nTotal;
        int nRead = ftread(&m_rdisk, &pos, m_rbuff.block_size, m_rdisk.offset);
        if (nRead != m_rbuff.block_size) {
            return FT_ERR_UNKNOWN;
        }

        memcpy(&stHeader, pos, sizeof(FTDataHeaderSt));
        VINT32(nSeq, stHeader.szSeq);
        VINT32(nSubSeq, stHeader.szSubSeq)
            VINT32(nTotal, stHeader.szTotal);

        if (check_seq(nSeq, nSubSeq,&m_rbuff) != FT_OK || nTotal == 0) {
            if (nRetry > 1000)
                return FT_ERR_UNKNOWN;

            FT_LOG(FT_LOG_ALERT,"wait for new data, offset subseq:%d,m_rbuff.subseq:%d,  %ld\n ", nSubSeq, m_rbuff.subseq, m_rdisk.offset);
            usleep(1000 * 100);
            nRetry += 1;
            continue;
        } 

        m_left_to_read -= m_rbuff.block_size;
        m_rbuff.seq = nSeq;
        m_rbuff.subseq = nSubSeq;
        m_total_recv += nRead;
        m_rdisk.offset += m_rbuff.block_size;

        nRecv = nTotal - sizeof(FTDataHeaderSt);
        m_rbuff.nInpos = 0;
        m_rbuff.nTotal = nRecv;

        pos += sizeof(FTDataHeaderSt);
        memcpy(m_rbuff.buff, pos, nRecv);
        pos += nRecv;

        if (nleft > 0 && nleft <= nRecv) {
            memcpy(content,m_rbuff.buff,nleft);
            m_rbuff.nInpos += nleft;
            m_rbuff.nTotal -= nleft;
            nleft  = 0 ;
        }
        else if (nleft > 0) {
            memcpy(content,m_rbuff.buff,nRecv);
            content =(void*)( (char*)content + nRecv);
            nleft -= nRecv;
        }

        if (! (m_rbuff.subseq % ((m_rdisk.device_size / m_rbuff.block_size / 2) + 1))) {
            if (FT_OK != send_ctrdata() ) {
                return FT_ERR_UNKNOWN;
            }
        }

    }	
    while (nleft > 0);

#endif
    *nRecved = buffsize;
    return FT_OK;
}

int FTransport::ft_sleep(unsigned int usec) {
#ifdef WIN32
    Sleep(usec/1000);
#else
    usleep(usec);
#endif
    return FT_OK;
}

int FTransport::ftread(void* fd, char* buff, int buff_size) {
#ifdef WIN32
    DWORD nReaded;
    ReadFile(fd, buff, buff_size, &nReaded, NULL);
    return nReaded;
#else
    return read((long)fd, buff, buff_size); 
#endif

}

int FTransport::ftclose(void* fd) {
#ifdef WIN32
    CloseHandle(fd);
#else
    close((long)fd);
#endif
    return FT_OK;
}

int FTransport::Flush() {

    m_wbuff.nFlush = 1;

    if (FT_OK != Send(NULL, 0)) {
        return FT_ERR_WRITE;
    }

    if (m_wdisk.wait == 0) {
        return FT_OK;
    }

#ifdef WIN32	

    DWORD nWrite;
    if (GetOverlappedResult(m_wdisk.fd,&m_wdisk.overlap,&nWrite,TRUE) != true)
        return FT_ERR_UNKNOWN;
    ResetEvent(m_wdisk.overlap.hEvent);

#elif defined(AIX)
    struct aiocb64* list[1];
    struct timespec timeout; 
    timeout.tv_sec = 300;
    timeout.tv_nsec = 0;
    int nent = 1, n = 0,nRt;
    list[0] = (struct aiocb64*)m_wdisk.iocb;
    do {
        n = aio_suspend64(list,nent,&timeout);
        if (n == -1 && errno != EAGAIN) {
            FT_LOG(FT_LOG_ERR,"aio_suspend failed with error code=%d",errno);
            return FT_ERR_WRITE;
        }
    } while (n == -1);

    if ((nRt = aio_error64(list[0])) != 0 ) {
        FT_LOG(FT_LOG_ERR,"aio_error failed with error code=%d",errno);
        return FT_ERR_WRITE;
    }
    if ((nRt = aio_return64(list[0])) != m_last_send.nlen ) {
        FT_LOG(FT_LOG_ERR,"aio_return64 failed with error code=%d",errno);
        return FT_ERR_WRITE;
    }
#else
    struct io_event events[1];

    int n;
    do {
        n = io_getevents(m_wdisk.ctx, 1, 1, events, NULL);
        if (n != 1 && errno != EINTR ) {
            return FT_ERR_WRITE;
        }
    } while(n != 1);
#endif
    m_wdisk.wait = 0;	

#if HAVE_FILE_AIO
    if (FT_OK != send_ctrdata(&m_last_send)) {
        return FT_ERR_WRITE;
    }
#endif

    return FT_OK;
}

void FTransport::UnInit (int flag) {

    if (flag == 0 && m_init_state > 0) {
        sync_close();
    }
    m_init_state = FT_NONE_INIT;
}

void FTransport::sync_close() {

    FT_LOG(FT_LOG_DEBUG,"FT transport UnInit sync_close begin :%d",m_init_state);

    int nRC = FT_OK;
    FTControlPackHeaderSt header = { 0 };
    FTCtrlHeaderSt ctrl_header = { 0 };
    if (!m_tcpsbuf) {
        return ;
    }

    int nWaitCnt = 36000; /* 超时1小时 */
    if (m_is_noblock) {
        if (m_is_server) {
            ctrl_header.type = FT_DATA_END;
            send_ctrdata(&ctrl_header);
            while (m_init_state != FT_CLOSE_WAIT && nWaitCnt > 0){
                int rt =  recv_ctrdata(1);
                if (FT_ERR_TIMEOUT == rt)
                    nWaitCnt--;
                else if (rt != 0 )
                    break;
            }	
            send_ctrdata(&ctrl_header);

        }
        else {
            while (m_init_state != FT_CLOSE_WAIT &&  nWaitCnt > 0) {
                int rt =  recv_ctrdata(1);
                if (FT_ERR_TIMEOUT == rt)
                    nWaitCnt--;
                else if (rt != 0 )
                    break;
            }	

            ctrl_header.type = FT_DATA_END;
            send_ctrdata(&ctrl_header);
            recv_ctrdata(1000);
        }
        return ;
    }

    if (m_is_server) {
        XINT16(FT_DATA_END, header.szType);
        WriteBinary(m_tcpsbuf, (const unsigned char*)&header, sizeof(FTControlPackHeaderSt));
        nRC = FlushSBuff(m_tcpsbuf);
        m_send_ctrl_cnt++;
    }
    else {
        while (true) {
            int nRead;
            int nRc = ReadBinary(m_tcpsbuf, (unsigned char*)&header, sizeof(FTControlPackHeaderSt), &nRead);
            if (nRc != 0) {
                nRC = FT_ERR_SOCK;
                break;
            }
            unpack_ctrlheader(&header, &ctrl_header);
            if (ctrl_header.type == FT_DATA_END) {
                break;
            }
            m_recv_ctrl_cnt++;

        }
    }

}


int FTransport::wait_ch_writable(FTCtrlHeaderSt* header) {
    int nretry = FT_RETRY_TIMES;
    int nRc = 0;
    while (nretry-- > 0)
    {
        if (m_total_send - m_send_ack <= (m_wdisk.device_size - (m_wbuff.block_size * 2) - header->nlen )){
            if (m_send_ctrl_cnt && !(m_send_ctrl_cnt % 100)) { /* 每发送100次接收一次ack，防止接受缓冲区满 */
                nRc = recv_ctrdata(1);
                if (FT_OK != nRc && FT_ERR_TIMEOUT != nRc) {
                    m_init_state = 0;
                    return FT_ERR_SOCK;
                }
            }

            if (m_wdisk.offset + header->nlen > m_wdisk.device_size)
            {
#if !HAVE_FILE_AIO
                if ( FT_OK != ftseek(m_wdisk.fd,m_wbuff.block_size)) {
                    LogErrorMsg("ftseek");
                    return FT_ERR_SEEK;
                }
#endif
                m_wdisk.offset = m_wbuff.block_size;
            }
            return FT_OK;
        }

        else {
            nRc = recv_ctrdata(1);
            if (FT_OK != nRc && FT_ERR_TIMEOUT != nRc) {
                m_init_state = 0;
                return FT_ERR_SOCK;
            }
        }
    }

    return FT_ERR_TIMEOUT;

}

int FTransport::wait_ch_readable(int noblock) {

    int nRetry = 0;
    while (nRetry++ < FT_RETRY_TIMES) {

        if (m_ctrl_queue.size() > 0) {
            return FT_OK;
        }

        if (m_init_state == 2) {
            return FT_ERR_UNKNOWN;
        }

        int nRc = recv_ctrdata(1);
        if (FT_OK != nRc && FT_ERR_TIMEOUT != nRc) {
            m_init_state = 2;
            return FT_ERR_SOCK;
        }

    }
    return FT_ERR_SOCK;
}

int FTransport::send_ctrdata(FTCtrlHeaderSt* data) {
    int sockfd = m_tcpsbuf->fd;
    int nRetry = FT_RETRY_TIMES;

    FTControlPackHeaderSt stHeader;
    pack_ctrlheader(&stHeader, data);

#ifdef WIN32
    WSABUF saBuf;
    saBuf.buf = (char *)&stHeader;
    saBuf.len = sizeof(FTControlPackHeaderSt);
    DWORD writBytes = 0;
    do {
        if (WSASend(sockfd, &saBuf, 1, &writBytes, 0, NULL, NULL) == SOCKET_ERROR) {
            int rc = WSAGetLastError();
            if (rc != WSAEWOULDBLOCK) {
                FT_LOG(FT_LOG_ERR,"WSASend()");
                LogErrorMsg("");
                return FT_ERR_SOCK;
            }
            else {
                ft_sleep(FT_TIME_SLEEP);
            }
        }
        else {
            m_send_ctrl_cnt++;
            return FT_OK;
        }

    } while (nRetry-- > 0);

#elif defined AIX
    struct timeval tv;
    fd_set lfds;
    int rv;
    do {
        do {
            tv.tv_sec = 0;
            tv.tv_usec = 100000;
            FD_ZERO(&lfds);
            FD_SET(sockfd, &lfds);
            rv = select(sockfd + 1, NULL,&lfds, NULL, &tv);
        } while (rv < 0 && errno == EINTR);

        if (rv == 0) {
            FT_LOG(FT_LOG_DEBUG, "select timeout");
            continue;
        }
        else
        {
            int nwrite = write(sockfd, (void*)&stHeader, sizeof(stHeader));
            if (nwrite != sizeof(stHeader) ){
                if (errno != EAGAIN || errno || errno != EINTR) {
                    FT_LOG(FT_LOG_ERR,"write failed ret:%d",nwrite);
                    LogErrorMsg("");
                    return FT_ERR_SOCK;
                }
                else {
                    continue;
                }
            }
            m_send_ctrl_cnt++;
            return FT_OK;
        }

    }while (nRetry-- > 0);

#else

    /* 超时3600s */
    struct epoll_event events[1];
    do {
        int nfds = epoll_wait(m_epoll_out_fd, events, 1, 100);
        if (nfds == -1) {
            if (errno != EINTR){
                FT_LOG(FT_LOG_ERR, "epoll_wait error");
                LogErrorMsg("");
                m_init_state = FT_NONE_INIT;
                return FT_ERR_SOCK;
            }
            else {
                continue;
            }
        }
        if (nfds == 0) {
            FT_LOG(FT_LOG_DEBUG, "epoll_wait timeout");
            continue;
        }
        if (events[0].events & EPOLLERR) {
            FT_LOG(FT_LOG_ERR, "epoll_wait EPOLLERR");
            LogErrorMsg("");
            return FT_ERR_SOCK;
        }
        else {
            int nwrite = write(sockfd, (void*)&stHeader, sizeof(stHeader));
            if (nwrite != sizeof(stHeader) ){
                if (errno != EAGAIN || errno != EINTR) {
                    FT_LOG(FT_LOG_ERR,"write failed");
                    LogErrorMsg("");
                    return FT_ERR_SOCK;
                }
                else {
                    continue;
                }
            }
            m_send_ctrl_cnt++;
            return FT_OK;
        }
    }while (nRetry-- > 0);
#endif
    return FT_ERR_SOCK;
}

int FTransport::recv_ctrdata(int timeout) {
    int sockfd = m_tcpsbuf->fd;
    FTControlPackHeaderSt vec[100];
    FTCtrlHeaderSt stHeader;
    int nNeed,nTotal;
    unsigned long nRead;
    nNeed = sizeof(vec);
    nTotal = 0;

    char* pszbuff = (char*)vec;
    int nRetry = 1;
    if (timeout > 1){  /* 最后关闭通道时 */
        nRetry = timeout;
        nNeed = sizeof(FTControlPackHeaderSt);
    }

#ifdef WIN32
    WSAEVENT evtArray[1];
    evtArray[0] = m_tcpsbuf->hEvent;
    WSABUF saBuf;
#elif !defined(AIX)
    struct epoll_event events[1];
#else

#endif

    while (true) {
#ifdef WIN32
        int nResult = WSAWaitForMultipleEvents(1, evtArray, FALSE, 100, FALSE);
        if (nResult == WSA_WAIT_TIMEOUT) {
            if (--nRetry <= 0 ){
                FT_LOG(FT_LOG_DEBUG, "WSAWaitForMultipleEvents timeout");
                return FT_ERR_TIMEOUT;
            }
            else 
                continue;
        }
        nRead = 0;
        WSANETWORKEVENTS event;
        WSAEnumNetworkEvents(sockfd,NULL,&event);  
        WSAResetEvent(evtArray[0]);  

        if (event.lNetworkEvents & FD_CLOSE) {
            m_init_state = FT_NONE_INIT;
            FT_LOG(FT_LOG_ALERT, "WSAWaitForMultipleEvents FD_CLOSE");
            return FT_ERR_SOCK;
        }

        else if (event.lNetworkEvents & FD_READ) {
            DWORD opt = 0;
            saBuf.buf =(char*) pszbuff;
            saBuf.len = nNeed;
            if ( SOCKET_ERROR == WSARecv(sockfd,&saBuf,1,&nRead,&opt,NULL,NULL)){
                if (WSAGetLastError() != WSAEWOULDBLOCK ){
                    LogErrorMsg("recv_ctrdata() WSARecv() ");
                    return FT_ERR_SOCK;
                }
                else 
                {
                    nRetry--;
                    continue;
                }
            }
        }
        else {
            FT_LOG(FT_LOG_ALERT, "WSAWaitForMultipleEvents unknown error");
            break;
        }

#elif defined(AIX)
        struct timeval tv;
        fd_set lfds;
        int rv;
        do {
            tv.tv_sec = 0;
            tv.tv_usec = 100000;
            FD_ZERO(&lfds);
            FD_SET(sockfd, &lfds);
            rv = select(sockfd + 1, &lfds, NULL, NULL, &tv);
        } while (rv < 0 && errno == EINTR);

        if (rv == 0 ) {
            if (--nRetry <= 0 ){
                FT_LOG(FT_LOG_DEBUG, "select timeout");
                return FT_ERR_TIMEOUT;
            }
            else 
                continue;
        }
        nRead = read(sockfd, pszbuff, nNeed);
        if (nRead == -1 ) {
            return FT_ERR_SOCK;
        }

#else
        int nfds = epoll_wait(m_epoll_in_fd, events, 1, 100);
        if (nfds == -1 ) {
            if (errno != EINTR){
                FT_LOG(FT_LOG_ERR, "epoll_wait error");
                LogErrorMsg("");
                m_init_state = FT_NONE_INIT;
                return FT_ERR_SOCK;
            }
            else {
                continue;
            }
        }
        if (nfds == 0 ) {

            if (--nRetry <= 0 ){
                FT_LOG(FT_LOG_DEBUG, "epoll_wait timeout");
                return FT_ERR_TIMEOUT;
            }
            else 
                continue;

        }
        /* socket发生错误 */
        if (events[0].events & EPOLLERR) {
            FT_LOG(FT_LOG_ERR, "epoll_wait EPOLLERR");
            m_init_state = FT_NONE_INIT;
            return FT_ERR_SOCK;
        }
        nRead = read(sockfd, pszbuff, nNeed);
        if (nRead == -1 ) {
            return FT_ERR_SOCK;
        }
#endif		

        nTotal += nRead;
        if (nTotal % sizeof(FTControlPackHeaderSt)) {	
            FT_LOG(FT_LOG_ERR, "read imperfect :%d ",nTotal);
            nNeed = sizeof(FTControlPackHeaderSt) - (nTotal % sizeof(FTControlPackHeaderSt));
            pszbuff = (char*)vec + nTotal;  
            continue;
        }
        else if(nTotal > 0) {
            for (int i = 0; i < (nTotal / sizeof(FTControlPackHeaderSt)); i++) {
                unpack_ctrlheader(&vec[i] ,&stHeader);
                m_recv_ctrl_cnt++;
                if (stHeader.type == FT_CTL_SEND_PACK) {
                    /*	if (m_is_server) {
                    while (m_left_to_read < stHeader.nlen) {
                    int size = netlink_get_recv_size();
                    if (size > 0)
                    m_left_to_read += size;
                    else {
                    ft_sleep(FT_TIME_SLEEP);
                    }
                    }
                    m_left_to_read -= stHeader.nlen;
                    } */ 
                    m_ctrl_queue.push(stHeader);
                }
                else if (stHeader.type == FT_CTL_RECV_ACK) {
                    m_send_ack += stHeader.nlen;
                }
                else if (stHeader.type == FT_DATA_END) {
                    FT_LOG(FT_LOG_INFO, "read transport end flag !");
                    /* 初始化状态置为2,收到结束确认包了 */
                    m_init_state = FT_CLOSE_WAIT;
                    return FT_OK;
                }
            }
        }
        if (nTotal < sizeof(vec))
            break;
        else {
            nTotal = 0;
        }
    }

    return FT_OK;
}


int FTransport::ft_send_noblock(void* content, unsigned int size) {
    int nRC = FT_OK;
    unsigned int pack_size = 0;
    DWORD nWrite = 0;
    int left = 0;
    FTDataHeaderSt stHeader = { 0 };
    char* buff = (char*)content;

    if ((size == 0 || content == NULL) && m_wbuff.nFlush != 1) {
        return FT_ERR_UNKNOWN;
    }

    if (size < FT_CHNANEL_SPACE_LEFT_NO_HEADER(m_wbuff) && m_wbuff.nFlush != 1) {
        memcpy((char*)m_wbuff.buff + m_wbuff.nInpos, content, size);
        m_wbuff.nTotal += size;
        m_wbuff.nInpos += size;
        return FT_OK;
    }
    else if (m_wbuff.nFlush != 1) {
        left = size - (FT_CHNANEL_SPACE_LEFT_NO_HEADER(m_wbuff));
        if (FT_CHNANEL_SPACE_LEFT_NO_HEADER(m_wbuff) > 0)
            memcpy((char*)m_wbuff.buff + m_wbuff.nInpos, content, (FT_CHNANEL_SPACE_LEFT_NO_HEADER(m_wbuff)));
        content = (char*)content + FT_CHNANEL_SPACE_LEFT_NO_HEADER(m_wbuff); /* 指针偏移到下次位置 */
        m_wbuff.nTotal = m_wbuff.block_size;
    }
    else {
        m_wbuff.nFlush = 0;
        if (m_wbuff.nTotal == 0)
            return FT_OK;
    }

    do {

        char* pszBlock = (char*)m_wbuff.buff;
        FTCtrlHeaderSt header ;
        DWORD nWrite = 0;
        pack_size =  m_wbuff.nTotal;
        header.type = FT_CTL_SEND_PACK;
        header.code = 0;
        header.nDataLen = pack_size;
        header.nlen = (pack_size % FT_MIN_BLOCK_SIZE) ? ((pack_size / FT_MIN_BLOCK_SIZE + 1) * FT_MIN_BLOCK_SIZE) : pack_size;

        if (wait_ch_writable(&header) != FT_OK) {
            return FT_ERR_WRITE;
        }
        if (FT_OK != ft_asyncwrite(&m_wdisk, pszBlock, header.nlen)) {
            FT_LOG(FT_LOG_ERR,"ft_asyncwrite() error");
            LogErrorMsg("");
            return FT_ERR_WRITE;
        }
        memcpy(&m_last_send, &header, sizeof(header));

#if !HAVE_FILE_AIO
        if (0 != send_ctrdata(&m_last_send)) {
            return FT_ERR_SOCK;
        }
#endif
        if (m_is_server) {
            if (0 != send_ctrdata(&m_last_send)) {
                return FT_ERR_SOCK;
            }
        }

        FT_LOG(FT_LOG_DEBUG, "nTotal:%d offset:%ld\n",
            m_wbuff.nTotal, m_wdisk.offset);

        m_wdisk.offset += header.nlen;
        m_total_send += header.nlen;
        swap(&m_wbuff);

        m_wbuff.nInpos = 0;
        m_wbuff.nTotal = 0;

        if (left > 0 && left < FT_CHNANEL_SPACE_LEFT_NO_HEADER(m_wbuff)) {
            memcpy((char*)m_wbuff.buff + m_wbuff.nInpos, (char*)content, left);
            m_wbuff.nInpos += left;
            m_wbuff.nTotal += left;
            left = 0;
        }
        else if (left > 0) { /* 剩余大小 >= 通道总容量 */
            memcpy((char*)m_wbuff.buff + m_wbuff.nInpos, (char*)content, 
                FT_CHNANEL_SPACE_LEFT_NO_HEADER(m_wbuff));
            left -= FT_CHNANEL_SPACE_LEFT_NO_HEADER(m_wbuff);
            content = (char*)content + FT_CHNANEL_SPACE_LEFT_NO_HEADER(m_wbuff);
            m_wbuff.nTotal = FT_CHNANEL_SPACE_LEFT_NO_HEADER(m_wbuff);
            continue;
        }

    } while (left > 0);
    return nRC;
}

int FTransport::ft_recv_noblock(void* content, unsigned int buffsize, unsigned int* nRecved) {
    FTDataHeaderSt stHeader = { 0 };
    unsigned int nRecvSize = buffsize;
    int nRetry = 0;
    int nleft = 0;
    *nRecved = 0;

    if (content == NULL || buffsize == 0) {
        return FT_OK;
    }
    if (buffsize <= m_rbuff.nTotal) {
        memcpy(content, (char*)m_rbuff.buff + m_rbuff.nInpos, buffsize);
        m_rbuff.nTotal -= buffsize;
        m_rbuff.nInpos += buffsize;
        *nRecved = buffsize;
        return FT_OK;
    }
    else if (m_rbuff.nTotal > 0) {
        memcpy(content, (char*)m_rbuff.buff + m_rbuff.nInpos, m_rbuff.nTotal);

        content = (void*)((char*)content + m_rbuff.nTotal);
        *nRecved += m_rbuff.nTotal;
    }
    nleft = buffsize - m_rbuff.nTotal;
    m_rbuff.nTotal = 0;
    m_rbuff.nInpos = 0;

    do {

        if (wait_ch_readable(1) != FT_OK) {
            return FT_ERR_READ;
        }

        int nRecv = 0;
        char* pos = NULL;
        int offset = m_rbuff.nInpos;
        unsigned int ndataLen, nTotal;
        int nRead;
        while (m_ctrl_queue.size() > 0) {
            FTCtrlHeaderSt& data_header = m_ctrl_queue.front();
            if (FT_CHNANEL_SPACE_LEFT_NO_HEADER(m_rbuff) < data_header.nlen ) {
                break;
            }

            if (m_is_server) {
                if (m_rdisk.offset + data_header.nlen > m_rdisk.device_size){
                    m_rdisk.offset = m_rbuff.block_size;
                }

                nRead = ftread(&m_rdisk, &pos, data_header.nlen, m_rdisk.offset);
                if (nRead == 0) {
                    return FT_ERR_UNKNOWN;
                }
                memcpy((char*)m_rbuff.buff + offset, pos, data_header.nDataLen);

            }
            else {
                if (m_rdisk.offset + data_header.nlen > m_rdisk.device_size) {
                    ftseek(m_rdisk.fd, m_rbuff.block_size);
                    m_rdisk.offset = m_rbuff.block_size;
                }
                nRead = ftread(&m_rdisk, (char*)m_rbuff.buff + offset, data_header.nlen);

            }
            if (nRead != data_header.nlen) {
                return FT_ERR_READ;
            }
            data_header.type = FT_CTL_RECV_ACK;
            if (FT_OK != send_ctrdata(&data_header)) {
                return FT_ERR_UNKNOWN;
            }

            m_rbuff.nTotal += data_header.nDataLen;
            offset += data_header.nDataLen;
            m_total_recv += nRead;
            m_rdisk.offset += nRead;

            if (data_header.nlen != data_header.nDataLen) {
                m_ctrl_queue.pop();
                break;
            }
            m_ctrl_queue.pop();
        }

        nRecv = m_rbuff.nTotal;

        if (nleft > 0 && nleft <= nRecv) {
            memcpy(content, m_rbuff.buff, nleft);
            m_rbuff.nInpos += nleft;
            m_rbuff.nTotal -= nleft;
            nleft = 0;
        }
        else if (nleft > 0) {
            memcpy(content, m_rbuff.buff, nRecv);
            content = (void*)((char*)content + nRecv);
            nleft -= nRecv;
            m_rbuff.nTotal = 0;
            offset = 0;
        }

    } while (nleft > 0);
    *nRecved = buffsize;
    return FT_OK;
}

int FTransport::ft_recv_noblock_async(void* content, unsigned int buffsize, unsigned int* nRecved) {
    FTDataHeaderSt stHeader = { 0 };
    unsigned int nRecvSize = buffsize;
    int nRetry = 0;
    int nleft = 0;
    *nRecved = 0;

    if (content == NULL || buffsize == 0) {
        return FT_OK;
    }
    if (buffsize <= m_rbuff.nTotal) {
        memcpy(content, (char*)m_rbuff.buff + m_rbuff.nInpos, buffsize);
        m_rbuff.nTotal -= buffsize;
        m_rbuff.nInpos += buffsize;
        *nRecved = buffsize;
        return FT_OK;
    }
    else if (m_rbuff.nTotal > 0) {
        memcpy(content, (char*)m_rbuff.buff + m_rbuff.nInpos, m_rbuff.nTotal);

        content = (void*)((char*)content + m_rbuff.nTotal);
        *nRecved += m_rbuff.nTotal;
    }
    nleft = buffsize - m_rbuff.nTotal;
    m_rbuff.nTotal = 0;
    m_rbuff.nInpos = 0;

    do {
        int nRecv = 0,nRt = 0;

        if ((nRt = wait_ch_readable(1)) != FT_OK) {
            FT_LOG(FT_LOG_ERR,"wait_ch_readable failed! code=%d",nRt);
            return FT_ERR_READ;
        }

        if ((nRt  = ft_asyncread(&m_rdisk, &m_rbuff,&nRecv) )!= FT_OK) {
            FT_LOG(FT_LOG_ERR,"ft_asyncread failed! code=%d",nRt);
            return FT_ERR_READ;
        }
        if (nRecv == 0)
            continue;

        FT_LOG(FT_LOG_DEBUG,"ft_asyncread () offset:%lld recv size:%d",m_rdisk.offset,nRecv);
        if (nleft > 0 && nleft <= nRecv) {
            memcpy(content, m_rbuff.buff, nleft);
            m_rbuff.nInpos += nleft;
            m_rbuff.nTotal -= nleft;
            nleft = 0;
        }
        else if (nleft > 0) {
            memcpy(content, m_rbuff.buff, nRecv);
            content = (void*)((char*)content + nRecv);
            nleft -= nRecv;
            m_rbuff.nTotal = 0;
        }

    } while (nleft > 0);
    *nRecved = buffsize;
    return FT_OK;
}


#if !(defined(WIN32) || defined(AIX))
int FTransport::epoll_init() {
    struct epoll_event ev;
    int b_on = 1;

    if ( 0 != ioctl(m_tcpsbuf->fd, FIONBIO, &b_on))
    {
        FT_LOG(FT_LOG_ERR, "ioctl");
        LogErrorMsg("");
        return FT_ERR_SOCK;
    }

    int epollfd = epoll_create(1);
    if (epollfd == -1) {
        FT_LOG(FT_LOG_ERR, "epoll_create");
        return FT_ERR_SOCK;
    }
    ev.events = EPOLLIN | EPOLLERR;
    ev.data.fd = m_tcpsbuf->fd;
    int sock = m_tcpsbuf->fd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sock, &ev) == -1) {
        FT_LOG(FT_LOG_ERR,"epoll_ctl:error ");
        return FT_ERR_SOCK;
    }
    m_epoll_in_fd = epollfd;
    epollfd = epoll_create(1);
    if (epollfd == -1) {
        FT_LOG(FT_LOG_ERR, "epoll_create");
        return FT_ERR_SOCK;
    }
    ev.events = EPOLLOUT | EPOLLERR;
    ev.data.fd = m_tcpsbuf->fd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sock, &ev) == -1) {
        FT_LOG(FT_LOG_ERR, "epoll_ctl: error");
        return FT_ERR_SOCK;
    }
    m_epoll_out_fd = epollfd;
    return FT_OK;
}

#endif

int FTransport::ft_alloc_mem(int block_size) {
    int nRc = FT_OK;
    if (block_size > 0 && block_size <= 64) {
        block_size = block_size * 1024 * 1024;
    }
    else {
        block_size = FT_BLOCK_SIZE;
    }

    m_buff_size = block_size;
    m_rbuff.block_size = m_buff_size;
    m_wbuff.block_size = m_buff_size;
#ifdef WIN32
    m_wbuff.interlbuff = _aligned_malloc(m_buff_size,512);
    m_wbuff.interlbuff1 = _aligned_malloc(m_buff_size,512);
    m_rbuff.interlbuff = _aligned_malloc(m_buff_size,512);
    m_rbuff.interlbuff1 = _aligned_malloc(m_buff_size,512);
#else
    posix_memalign(&m_wbuff.interlbuff, sysconf(_SC_PAGESIZE), m_buff_size);
    posix_memalign(&m_wbuff.interlbuff1, sysconf(_SC_PAGESIZE), m_buff_size);
    posix_memalign(&m_rbuff.interlbuff, sysconf(_SC_PAGESIZE), m_buff_size);
    posix_memalign(&m_rbuff.interlbuff1, sysconf(_SC_PAGESIZE), m_buff_size);
#ifndef AIX
    m_wdisk.ctx = 2;
#endif
#endif
    m_wbuff.buff = m_wbuff.interlbuff;
    m_rbuff.buff = m_rbuff.interlbuff;

    m_wbuff.subseq = 1;

    if (m_wbuff.interlbuff == NULL || m_wbuff.interlbuff1 == NULL || m_rbuff.buff == NULL || m_tcpsbuf == NULL) {
        nRc = FT_ERR_NO_MEM;
    }
    return nRc;
}
