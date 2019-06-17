/** =============================================================================
** Copyright (c) 2003 WaveTop Information Corp. All rights reserved.
**
** The lanfree transport class
**
** =============================================================================
*/

#ifndef __FT_PACK__
#define __FT_PACK__ 1

#include "wnetfd.h"
#include <vector>
#include <queue>
#include <utility>
#include <string>

#if !( defined(AIX) || defined(WIN32))
#include <linux/netlink.h>
#endif

/* 错误码 */
#define FT_OK 0
#define FT_ERR_CODE 1
#define FT_ERR_WRITE FT_ERR_CODE + 1
#define FT_ERR_READ FT_ERR_CODE + 2
#define FT_ERR_OPEN FT_ERR_CODE + 3
#define FT_ERR_NO_MEM FT_ERR_CODE + 4
#define FT_ERR_SEEK FT_ERR_CODE + 5
#define FT_ERR_UNKNOWN FT_ERR_CODE + 6
#define FT_ERR_SOCK FT_ERR_CODE + 7
#define FT_ERR_TIMEOUT  FT_ERR_CODE + 8
#define FT_ERR_ASYNC_WRITE FT_ERR_CODE + 9

/* FC控制信息类型 */
#define FT_CTL_SEND_AMOUNT  1<<1
#define FT_CTL_RECV_AMOUNT  1<<2
#define FT_CTL_DATA    1<<3
#define FT_CTL_SEND_PACK  1<< 4
#define FT_CTL_RECV_ACK  1<< 5
#define FT_DATA_END    1<<14

#define FT_BLOCK_SIZE (2048 * 4096 * 4)
#define FT_TIME_SLEEP (100000)
#define FT_OUT_TIME    (3600 * 1000)
#define FT_RETRY_TIMES  (36000) 

#define FT_BACKEND_RAMDISK 1
#define FT_BACKEND_FILE    1<<1
#define FT_ASYNC_TRANSPORT 1<<2
#define FT_CLIENT          1<<3
#define FT_SERVER		   1<<4
#define FT_USE_NO_BLOCK	   1<<5
#define FT_MIN_BLOCK_SIZE  1024

/*  打印日志级别 */
#define FT_LOG_INFO  6
#define FT_LOG_ALERT 1
#define FT_LOG_ERR   3
#define FT_LOG_DEBUG  7

//#if !(defined(WIN32) /* || defined(AIX)*/)
#define HAVE_FILE_AIO 1
//#endif

typedef long  ft_aio_context_t;

#ifdef __cplusplus
extern "C" {
#endif
    extern void SLogErrorWrite(const char *file, int line, int level,const char *user, const char *fmt, ...);
#ifdef __cplusplus
}
#endif
#ifdef WIN32
void LogErrorMsg(LPTSTR lpszFunction) ;
#endif

#define FT_LOG(l,...)  SLogErrorWrite(__FILE__,__LINE__, l,NULL ,__VA_ARGS__)  

typedef struct _FT_DATA_PACK_HEADER {
    char szChkSum[2];
    char szSeq[4];
    char szSubSeq[4];
    char szType[2];
    char szTotal[4];
    char szReserve[4];
}FTDataHeaderSt;

typedef struct _FT_Control_Pack_Header {
    char szType[2];
    char szCode[2] ;
    char szLen[4];
    char szDataLen[4];
} FTControlPackHeaderSt;

typedef struct _FT_Control_Header {
    PRInt16  type;
    PRInt16  code;
    PRUint32 nlen;
    PRUint32 nDataLen;
} FTCtrlHeaderSt;

struct stFTChannel {
    unsigned int subseq;
    unsigned int seq;
    unsigned int nInpos;
    unsigned int nTotal;
    unsigned int timeout;
    int nFlush;
    int block_size;
    void* interlbuff;
    void* interlbuff1;
    void* buff;
};

struct stFTDisk {
    void* fd;
    char* devpath;
    PRUint64 device_size;
    PRUint64 offset;
    short  wait;
#ifdef WIN32
    OVERLAPPED overlap;
#else
#ifndef AIX
    ft_aio_context_t ctx;
    unsigned long ino;
    unsigned long idev;
#endif
    void* addr;
    void* iocb;
#endif
};

typedef struct _FT_CHANNEL_INFO {
    char signature[128];
    char diskname[1024];    
    PRInt64 disksize;
    PRInt32 buffsize;
} FtChannelInfo;


typedef std::vector< FtChannelInfo> ft_dev_vec;

class FTransport {
public:
    FTransport() ;
    int Init(sbuff *tcp,unsigned long flag);

    int Send(void* content,unsigned int size);

    int Recv(void* content,unsigned int buffsize,unsigned int* nRecved);

    int Flush();

    /* 0代表正常退出，其他表示异常退出 */
    void UnInit (int flag);

    ~FTransport() ;

private:
    int open_device();

    int cs_sync(int state);

    void sync_close();

    int open_disk(struct stFTDisk* disk,struct stFTChannel* channel, unsigned long flag = 0);

    int check_seq(unsigned int nSeq,unsigned int nSubSeq ,struct stFTChannel * channel);

    int wait_ch_writable();
    int wait_ch_readable();

    int wait_ch_writable(FTCtrlHeaderSt* header);
    int wait_ch_readable(int noblock);

    int send_ctrdata(FTCtrlHeaderSt* data);
    int recv_ctrdata( int timeout);

    int send_ctrdata();
    int recv_ctrdata();

    int ft_send_noblock(void* content, unsigned int size);

    int ft_recv_noblock(void* content, unsigned int buffsize, unsigned int* nRecved);

    int ft_clt_send(void* content, unsigned int size);

    int ft_srv_send(void* content, unsigned int size);

    int ft_clt_recv(void* content, unsigned int buffsize, unsigned int* nRecved);

    int ft_srv_recv(void* content, unsigned int buffsize, unsigned int* nRecved);

    int ft_sleep(unsigned int usec);

    /* 返回实际读到的大小 */
    int ftread(struct stFTDisk* disk, char* buff, int buff_size);

    int ftread(struct stFTDisk* disk, char** buff, unsigned int buff_size, PRUint64 offset);

    int ftread(void* fd,char* buff, int buff_size);

    /* 返回实际写入的大小 */
    int ftwrite(void* fd,char* buff, int buff_size);

    inline int ft_asyncwrite(struct stFTDisk* disk,char* buff, int buff_size);

    inline int ft_asyncread(struct stFTDisk* disk,struct stFTChannel* channel,int* nRead);

    inline int ft_recv_noblock_async(void* content, unsigned int buffsize,unsigned int* nRecved);

    int ftseek(void* fd, unsigned long offset);

    int ftclose(void* fd);

    int ft_alloc_mem(int block_size);

    void swap(stFTChannel* channel);

    int set_sock_noblock();

#if !(defined(AIX) || defined(WIN32))
    int mmap_disk(struct stFTDisk* disk, struct stFTChannel* channel);
    unsigned long  netlink_get_recv_size();
    unsigned long  netlink_get_send_ack();
    int netlink_get_msg(void* msg);
    int netlink_send_msg(void* msg);
    int netlink_init();
    int epoll_init();

    int m_netlink_fd;
    int m_netlink_pid;

    struct sockaddr_nl m_dest_addr;
    int m_epoll_in_fd;
    int m_epoll_out_fd;

    PRInt64 m_left_to_read;  /* 澶囩椹卞姩宸叉崟鑾峰埌鏁版嵁鐨勫墿浣欏ぇ灏?*/
#endif // WIN32

    stFTChannel m_rbuff; /* 读通道信息 */
    stFTChannel m_wbuff; /* 写通道信息 */
    stFTDisk m_wdisk;   /* 写缓存盘相关信息 */
    stFTDisk m_rdisk;   /* 读缓存盘相关信息 */
    sbuff* m_tcpsbuf;  /* tcp 缓存库 */

    PRUint32 m_buff_size;
    PRInt16 m_is_server;   /*  是否为服务端 */ 
    PRInt16 m_is_noblock;  /* 是否使用非阻塞IO传输控制信息 */
    PRInt16 m_init_state;  /*0 - 未初始化/发生错误 ，1 - 正常 ,2 - 已收到结束标志 */

    PRUint64 m_total_send;   /*  已发送数据总大小 */
    PRUint64 m_send_ack;     /*  已确认发送数据总大小 */
    PRUint64 m_total_recv;   /*  接受的数据总大小 */
    PRUint64 m_recv_ack;     /*  已确认接受的数据总大小 */	

    PRUint64 m_send_ctrl_cnt; /* 发送到控制数据包的数量 */
    PRUint64 m_recv_ctrl_cnt;

    ft_dev_vec m_devs; /* 通道设备在DB中的信息 */
    std::queue<FTCtrlHeaderSt> m_ctrl_queue;
    FTCtrlHeaderSt m_last_send;
    FTCtrlHeaderSt m_last_read;
};

#endif
