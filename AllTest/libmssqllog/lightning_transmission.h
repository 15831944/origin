/**================================================================
 ** Copyright (c) 2016 WaveTop Information Corp. All rights reserved.
 **
 ** 通用传输类头文件
 **
 ** ==============================================================
 */
/* IODAEMON停止 */
#define WAVETOP_LT_IODAEMON_STOP        -2
/* 未知 */
#define WAVETOP_LT_UNKNOW               -1
/* 成功 */
#define WAVETOP_LT_OK                   0
/* 初始化失败 */
#define WAVETOP_LT_INIT_ERROR           1
/* 继续 */
#define WAVETOP_LT_CONTINUE             2
/* 节点以装载 */
#define WAVETOP_LT_DATA_FULL            1
/* 节点未装载 */
#define WAVETOP_LT_DATA_EMPTY           0
/* oracle log 发送 */
#define WAVETOP_LT_ORACLE_SEND_MODE     3
/* oracle log 接收 */
#define WAVETOP_LT_ORACLE_RECV_MODE     4
/* 发送文件成功 */
#define WAVETOP_LT_SEND_FILE_ERROR      5
/* 接收文件失败 */
#define WAVETOP_LT_RECV_FILE_ERROR      6
/* 打开文件失败 */
#define WAVETOP_LT_OPEN_FILE_ERROE      7
/* 读取文件失败 */
#define WAVETOP_LT_READ_FILE_ERROE      8
/* 写入文件失败 */
#define WAVETOP_LT_WRITE_FILE_ERROE     9
/* 增量文件 */
#define WAVETOP_LT_INCR_FILE            10
/* 全量文件 */
#define WAVETOP_LT_FULL_FILE            11
/* 参数无效 */
#define WAVETOP_LT_INVALID_PARAMETER    12
/* 分配数组失败 */
#define WAVETOP_LT_NEW_DATA_ERROR       13
/* 分配内存失败 */
#define WAVETOP_LT_MALLOC_DATA_ERROR    14
/* 读取线程失败 */
#define WAVETOP_LT_READ_THREAD_ERROR    15
/* 发送线程失败 */
#define WAVETOP_LT_SEND_THREAD_ERROR    16
/* 接收线程失败 */
#define WAVETOP_LT_RECV_THREAD_ERROR    17
/* 写入线程失败 */
#define WAVETOP_LT_WRITE_THREAD_ERROR   18
/* 发送模式 */
#define WAVETOP_LT_SEND_MODE            19
/* 接收模式 */
#define WAVETOP_LT_RECV_MODE            20
/* 文件偏移失败 */
#define WAVETOP_LT_SEEK_ERROR           21
/* 连接断开 */
#define WAVETOP_LT_CONNECT_DOWN         200
/* 连接成功 */
#define WAVETOP_LT_CONNECT_SUCCESS      201
/* 任务成功 */
#define WAVETOP_LT_JOB_SUCCESS          202
/* 协议对齐 */
#define WAVETOP_LT_PROTOCOL_ALIGNMENT   "Protocol_alignment"

/**
 * @描述 无锁任务队列节点结构体
 *
 * @成员 nID 节点序列号
 * @成员 nType 读写互斥量：WAVETOP_LT_DATA_FULL、WAVETOP_LT_DATA_EMPTY
 * @成员 nSize 数据块长度
 * @成员 pBuff 数据块
 *
 */
typedef struct lt_data {
    unsigned int nID;
    unsigned int nType;
    int nSize;
    unsigned char *pBuff;
} LTDatast;

/**
 * @描述 待发送的文件列表结构体
 *
 * @成员 szFileName 需要发送的文件的名称
 * @成员 szTime 需要发送的文件的时间点
 * @成员 cMode 文件的类型：WAVETOP_LT_INCR_FILE 增量文件 WAVETOP_LT_FULL_FILE 全量文件
 * @成员 pNext 下一个节点地址
 *
 */
typedef struct lt_filelist {
    char szFileName[1024];
    unsigned char szTime[8];
    unsigned char cMode;
    lt_filelist *pNext;
} LTFilelistst;

/**
 * @描述 运行参数结构体
 *
 * @成员 pNetBuff socket连接句柄信息
 * @成员 nNodeSize 任务队列单节点长度
 * @成员 nNodeCount 任务队列节点个数
 * @成员 nMode 发送接收标识：WAVETOP_LT_SEND_MODE 发送 WAVETOP_LT_RECV_MODE 接收 WAVETOP_LT_ORACLE_SEND_MODE ORACLE_LOG发送 WAVETOP_LT_ORACLE_RECV_MODE ORACLE_LOG接收
 * @成员 nThreadID 客户端发送线程ID
 * @成员 szIP 客户端IP
 * @成员 szFileNumber 已发送的文件序列号
 * @成员 szCurrentOffset 已发送的文件偏移量
 * @成员 szWorkPath 当前路径
 * @成员 pFiles 需发送的文件列表
 *
 */
typedef struct lt_config {
    void *pNetBuff;
    unsigned int nNodeSize;
    unsigned int nNodeCount;
    unsigned int nMode;
    unsigned int nThreadID;
    char szIP[16];
    char szBeginFileNumber[12];
    char szFileCount[12];
    char szCurrentOffset[12];
    char szEndOffset[12];
    char szWorkPath[1024];
    LTFilelistst *pFiles;
} LTConfigst;

/**
 * @描述 运行状态结构体
 *
 * @成员 nReadThreadStatus 读取线程状态
 * @成员 nSendThreadStatus 发送线程状态
 * @成员 nRecvThreadStatus 接收线程状态
 * @成员 nWriteThreadStatus 写入线程状态
 * @成员 nMessage 消息
 * @成员 nAction 消息通知
 * @成员 szFileNumber 当前文件序号
 * @成员 szCurrentOffset 当前文件偏移量
 *
 */
typedef struct lt_runstatus {
    int nReadThreadStatus;
    int nSendThreadStatus;
    int nRecvThreadStatus;
    int nWriteThreadStatus;
    int nMessage;
    int nAction;
    char szCompletedFile[12];
    char szFileNumber[12];
    char szCurrentOffset[32];
} LTRunStatusst;

class Clightning_transmission
{

public:

    Clightning_transmission();
    ~Clightning_transmission();

    /**
     * @描述 初始化Clightning_transmission类的必要的运行参数
     *
     * @参数 pstConfig
     *      [in] 运行产生信息
     *
     * 成功返回 WAVETOP_BACKUP_OK
     * 失败返回 参照头文件定义
     */
    int LTInit(LTConfigst *pstConfig);
    
    /**
     * @描述 清理初始化时构造的运行参数信息
     *
     */
    void LTUnit();

    /**
     * @描述 文件续传
     *
     * @参数 pNetBuff
     *      [in] 网络连接结构体netbuff
     *
     * 成功返回 WAVETOP_BACKUP_OK
     * 失败返回 参照头文件定义
     */
    int LTContinue(void *pNetBuff);

    /**
     * @描述 循环调用获取通用传输文件类的运行状态
     *
     * @参数 pstRunStatus
     *      [out] 返回运行状态
     */
    void LTGetStat(LTRunStatusst *pstRunStatus);

    /**
     * @描述 客户端服务端之间消息传递
     *
     * @参数 nAction
     *      [in] 消息
     */
    void LTMessage(int *nAction);

    /**
     * @描述 oracle log 放入任务队列
     *
     * @参数 pNode
     *      [in] 节点
     */
    int LTInput(void *pNode);

    /**
     * @描述 oracle log 取出任务队列
     *
     * @参数 pNode
     *      [in] 节点
     */
    int LTOutput(void *pNode);

private:

    /**
     * @描述 发送数据块线程
     *
     * @参数 pParm
     *      [in] 传入类this指针
     */
    static void LTSendThread(void *pParm);

    /**
     * @描述 接收数据块线程
     *
     * @参数 pParm
     *      [in] 传入类this指针
     */
    static void LTRecvThread(void *pParm);

    /**
     * @描述 读取文件线程
     *
     * @参数 pParm
     *      [in] 传入类this指针
     * 
     * 
     */
    static void LTReadThread(void *pParm);
    
    /**
     * @描述 写入文件线程
     *
     * @参数 pParm
     *      [in] 传入类this指针
     * 
     * 
     */
    static void LTWriteThread(void *pParm);

    /**
     * @描述 打开普通文件、独占文件
     *
     * @参数 pszFileName
     *      [in] 传入文件名
     * 
     * 
     */
    int LTOpen(char *pszFileName);

    /**
     * @描述 关闭文件句柄
     *
     * 
     * 
     */
    void LTClose();

    /**
     * @描述 偏移到指定偏移量
     *
     * @参数 nOffset
     *      [in] 传入文件偏移量 
     * 
     */
    int LTSeek(void *nOffset, int nMode);

    /**
     * @描述 写入文件
     *
     * @参数 pszData
     *      [in] 传入待写入内存地址
     * @参数 nDataBytes
     *      [in] 传入待写入内存长度
     * 
     * 
     */
    int LTWrite(unsigned char *pszData, int nDataBytes);    

    /**
     * @描述 读取文件
     *
     * @参数 pszData
     *      [in] 传入待读取的内存地址
     * @参数 pnBytesOfRead
     *      [in] 传入待读取的长度 
     * 
     */
    int LTRead(unsigned char *pszData, void *pnBytesOfRead);

    /**
     * @描述 读取索引文件
     *
     * @参数 nNumber
     *      [in] 传入文件序列号
     * 
     * 
     */
    int LTSwitchIncFile(void *nNumber);

    /**
     * @描述 写入索引文件
     *
     * @参数 nFileID
     *      [in] 传入文件序列号
     * @参数 nFileOffset
     *      [in] 传入文件偏移量
     * @参数 nMode
     *      [in] 传入文件类型 增量/全量
     * 
     * 
     */
    int LTWriteIndexFile(void *nFileID, void *nFileOffset, int nMode);

    /**
     * @描述 获取线程ID
     * 
     * 
     */
    int LTGetThreadID();

    /**
     * @描述 获取本地最小logdata文件序号
     * 
     */
    void LTGetFileID();

private:

    /* 发送线程句柄 */
    void *m_phSend;

    /* 接收线程句柄 */
    void *m_phRecv;

    /* 读取线程句柄 */
    void *m_phRead;

    /* 写入线程句柄 */
    void *m_phWrite;

    /* 当前打开的文件句柄 */
    void *m_hFile;

    /* 线程退出标识 */
    int m_nQuit;

    /* 连接成功标识 */
    int m_nConnect;

    /* 线程总数 */
    int m_nThreadCount;

    /* oracle log任务队列计算器 */
    int m_nOracleNum;

    /* 全量索引文件 */
    char m_szFullIndex[1024];

    /* 增量索引文件 */
    char m_szIncIndex[1024];

    /* 运行参数信息 */
    LTConfigst *m_pstConfig;

    /* 任务队列 */
    LTDatast *m_pstData;

    /* 运行状态 */
    LTRunStatusst m_stRunInfo;

};


