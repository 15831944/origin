/** =============================================================================
 ** Copyright (c) 2003 WaveTop Information Corp. All rights reserved.
 **
 ** The Mirror system
 **
 ** =============================================================================
 */

#ifndef __MIRROR_H_
#define __MIRROR_H_ 1

#include "mirror_ver.h"

#define WAVETOP_BACKUP_VERSION      1000

/* The backup server port(TCP) */
#define WAVETOP_BACKUP_PORT          57719
#define WAVETOP_BACKUP_PULL_PORT     57739
#define WAVETOP_CONSOL_LSNR_PORT     57710

/*  状态码定义
 *  0           成功
 *  1   - 100   回应方状态码
 *  101 - 200   请求方状态码
 *  201 - 300   协议错误
 */
#define WAVETOP_BACKUP_OK                       0        // 成功
#define WAVETOP_BACKUP_NOT_IMPLEMENTED          1        // 没有实现该方法
#define WAVETOP_BACKUP_FORBIDDEN                2        // 没有权限
#define WAVETOP_BACKUP_INTERNAL_ERROR           3        // 内部错误
#define WAVETOP_BACKUP_FILE_BUSY                4        // 服务器文件忙
#define WAVETOP_BACKUP_VERSION_NOT_SUPPORTED    5        // 不支持目前版本
#define WAVETOP_BACKUP_INDEX_DAMAGED            6        // 备份文件索引损坏
#define WAVETOP_BACKUP_AUTHORIZED_FAILED        7        // 用户认证失败
#define WAVETOP_BACKUP_BAD_REQUEST              101      // 错误请求
#define WAVETOP_BACKUP_FILENAME_TOO_LONG        102      // 文件名过长
#define WAVETOP_BACKUP_FILE_TOO_LARGE           103      // 文件太大
#define WAVETOP_BACKUP_REQUEST_ENTITY_TOO_LARGE 104      // 请求字段太长
#define WAVETOP_BACKUP_FILE_EMPTY               105      // 文件为空
#define WAVETOP_BACKUP_BAD_FILE_INDEX           201      // 错误文件索引
#define WAVETOP_BACKUP_FILE_NO_CHANGE           202      // 文件没有变化
#define WAVETOP_BACKUP_HAS_BEEN_WRITE_READ      203      // 数据库已经恢复为读写状态
#define WAVETOP_BACKUP_NULL_DATA                204      // 没有查找到数据
#define WAVETOP_BACKUP_SEND_STATUS_FAILED       205      // 服务器状态交互失败
#define WAVETOP_BACKUP_CHECK_PATH_AND_SPACE_FAILED  206      // 检查服务器路径和空间失败







/* 可自行定义的消息开始 */
#define WB_USER_MESSAGE         1001

/**
 * @defgroup protocol method code
 * @{
 */

#define BACKUP_PROTO_LOGIN              1   // login
#define BACKUP_PROTO_BACKUP             2   // file backup
#define BACKUP_PROTO_RESTORE            3   // file restore
#define BACKUP_PROTO_PULL               4

#define BACKUP_PROTO_DBMIRROR           5   // database mirror
#define BACKUP_PROTO_DBRESTORE          6   // database restore
#define BACKUP_PROTO_MIRRORDB_QUERY     7   // query mirror database
#define BACKUP_PROTO_MIRROR_MONITOR     8   // mirror monitor protocol method
#define BACKUP_PROTO_MIRRORDB_DELETE    9   // delete mirror database

#define BACKUP_PROTO_MIRROR_CONSOLE     10  // delete mirror database
#define BACKUP_PROTO_MIRROR_RESTORE_DB  11

#define BACKUP_PROTO_MIRROR_TRS         100 // trs mirror
#define BACKUP_PROTO_MIRROR_ORA         101 // oracle mirror
#define BACKUP_PROTO_MIRROR_MSSQL       102 // mssql mirror
#define BACKUP_PROTO_MIRROR_MYSQL       103 // mysql mirror
#define BACKUP_PROTO_MIRROR_MYSQL_EXP   104 // mysql export
#define BACKUP_PROTO_MIRROR_GBASE       106 // gbase mirror
#define BACKUP_PROTO_MIRROR_GBASE_EXP   107 // gbase export


#define BACKUP_PROTO_MIRROR_NEW_MONITOR 10101 // oracle monitor 
#define BACKUP_PROTO_MIRROR_ORAEXP      10102 // oracle export/import
#define BACKUP_PROTO_MIRROR_ORAIMP      10103

#define BACKUP_PROTO_MIRROR_SQL_MONITOR 10201 // Mssql monitor
#define BACKUP_PROTO_MIRROR_MYSQL_MONITOR 10301 // Mysql monitor
#define BACKUP_PROTO_MIRROR_GBASE_MONITOR 10601 // Gbase monitor

 
#define BACKUP_PROTO_ORA_AGENT_MGR      80
#define BACKUP_PROTO_ORA_SRV            81

/** @} */

/**
 * @defgroup mirror server monitor protocol
 * @{
 */

typedef int MiMonitorStatusFlage;
#define WTMI_MONITOR_STATUS_FLAGE_START     (1<<0)
#define WTMI_MONITOR_STATUS_FLAGE_RUN       (1<<1)
#define WTMI_MONITOR_STATUS_FLAGE_STOP      (1<<2)
#define WTMI_MONITOR_STATUS_SUCCESS         (1<<6)
#define WTMI_MONITOR_STATUS_FAILURE         (1<<7)

#define WTMI_MONITOR_TASK_TYPE_MYSQL_INC    17
#define WTMI_MONITOR_TASK_TYPE_GBASE_INC    19
#define WTMI_MONITOR_TASK_TYPE_GBASE        18


#define WTMI_MONITOR_STATUS_FLAGE_EXPDATA   21
#define WTMI_MONITOR_STATUS_FLAGE_SNDDATA   22
#define WTMI_MONITOR_STATUS_FLAGE_IMPDATA   23
#define WTMI_MONITOR_TASK_TYPE_MYSQL_FUL    105
#define WTMI_MONITOR_TASK_TYPE_GBASE_FUL    108


typedef int MiMonitorTaskType;
#define WTMI_MONITOR_TASK_TYPE_FS           (1<<0)
#define WTMI_MONITOR_TASK_TYPE_MSSQL        (1<<1)
#define WTMI_MONITOR_TASK_TYPE_ORACLE       (1<<2)
#define WTMI_MONITOR_TASK_RECO_ORACLE       (1<<3)
#define WTMI_MONITOR_TASK_TYPE_MYSQL        (1<<4)
#define WTMI_MONITOR_TASK_TYPE_MIRROR       (1<<8)
#define WTMI_MONITOR_TASK_TYPE_RESTORE      (1<<9)

/** @} */

/* Define the pull notify METHOD */
#define BACKUP_PROTO_PULL_NOTIFY      100001

/* 参数 - 传输该文件的参数 */
#define RESTORE_INHERIT_DIRNAME     (1<<7)  // 继承前一个目录名(可省略同名部分)

/* 参数 - 编码方式 */
#define BACKUP_PSW_TYPE_CLEAR       (1<<0)  // 明码方式传递
#define BACKUP_PSW_TYPE_BASE64      (1<<1)  // MD5-BASE64 加密方式传递
/* 认证方式 */
#define BACKUP_AUTHWAY_LOCAL_SYSTEM     (1<<2)  // 本系统认证
#define BACKUP_AUTHWAY_WIN_USER         (1<<3)  // Windows域用户认证

/* 操作系统类型 */
#define BACKUP_OS_TYPE_WINNT    1   // Windows NT 操作系统
#define BACKUP_OS_TYPE_WIN9X    2   // Windows Consumer 操作系统
#define BACKUP_OS_TYPE_UNIX     3   // Unix 操作系统

/* 文件类型 */
#define BACKUP_FILE_FILE         1
#define BACKUP_FILE_DIRECTORY    2
#define BACKUP_FILE_DETECT       3

/* console panle */
#define MIRROR_CONSOLE_TEST_CONNECT         1
#define MIRROR_CONSOLE_SEND_COMMAND_FILE    2


/* Delete user flag */
#define WACL_DELETE           (1<<0)

/* 还原参数 */
#define RESTORE_OPTION_RECURSE_SUBDIR     (1<<0)

/* 备份类型 */
#define BACKUP_TYPE_FULL               (1<<0)
#define BACKUP_TYPE_INCREASE           (1<<1)

/* 传输方式 */
#define BACKUP_TRANSFER_RSYNC           (1<<2)   // rsync 传输（增量传输） 
#define BACKUP_TRANSFER_SYNC            (1<<3)   // sync  传输
#define BACKUP_TRANSFER_LARGENUM        (1<<4)   // 大量文件传输方式
#define BACKUP_TRANSFER_COMPARE_REQ     (1<<5)   // 由sync 请求方进行比较
#define BACKUP_TRANSFER_ARCHIVE         (1<<6)   // sync传输算法ARCHIVE属性比较

#define BACKUP_OPTION_VERSION_INCREASE          (1<<10)   // 备份后版本递增1
#define BACKUP_OPTION_WAIT_FAILED_RESPONSE      (1<<11)   // 备份时客户端需等待服务器回应
#define BACKUP_OPTION_WAIT_SUCCESS_RESPONSE     (1<<12)   // 客户端需等待成功回应
#define BACKUP_OPTION_AUTO_MIRROR               (1<<13)   // 自动检测任务

/* 备份参数 */
#define BACKUP_OPTION_CHILD_DIR     (1<<20)         // 递归子目录
#define BACKUP_OPTION_CHECK_DATA    (1<<21)         // 备份后检查数据完整性
#define BACKUP_OPTION_FULL          BACKUP_TYPE_FULL        
#define BACKUP_OPTION_INCREASE      BACKUP_TYPE_INCREASE

/*  回调消息定义 */
#define WAVETOP_BACKUP_SYSTEM_BUSY          WB_USER_MESSAGE +   0
#define WAVETOP_BACKUP_CONNECT_DOWN         WB_USER_MESSAGE +   1
#define WAVETOP_BACKUP_NO_MEMORY            WB_USER_MESSAGE +   2
#define WAVETOP_BACKUP_FILE_NOT_INTGRETY    WB_USER_MESSAGE +   3
#define WAVETOP_BACKUP_FILTERED             WB_USER_MESSAGE +   4
#define WAVETOP_BACKUP_RATE                 WB_USER_MESSAGE +   5
#define WAVETOP_BACKUP_BEGIN                WB_USER_MESSAGE +   6
#define WAVETOP_BACKUP_END                  WB_USER_MESSAGE +   7
#define WAVETOP_BACKUP_OPEN_FILE_ERROR      WB_USER_MESSAGE +   8
#define WAVETOP_BACKUP_CREAT_FILE_ERROR     WB_USER_MESSAGE +   9
#define WAVETOP_BACKUP_OPEN_DIR_ERROR       WB_USER_MESSAGE +  10
#define WAVETOP_BACKUP_CREAT_DIR_ERROR      WB_USER_MESSAGE +  11
#define WAVETOP_BACKUP_FILE_NOT_EXIST       WB_USER_MESSAGE +  12
#define WAVETOP_BACKUP_INVALID_SYNTAX       WB_USER_MESSAGE +  13
#define WAVETOP_BACKUP_CONNECT_SUCCESS      WB_USER_MESSAGE +  14
#define WAVETOP_BACKUP_FSDETECT_FAILED      WB_USER_MESSAGE +  15
#define WAVETOP_BACKUP_DETECT_FILE          WB_USER_MESSAGE +  16
#define WAVETOP_BACKUP_RECEIVE_PULL         WB_USER_MESSAGE +  17
/* 多次都不能连接服务器：可能因为服务器down或网络故障 */
#define WAVETOP_BACKUP_CONNECT_DOWN_2       WB_USER_MESSAGE +  18

#define WAVETOP_BACKUP_NO_FREE_ROOM         WB_USER_MESSAGE +  19
#define WAVETOP_BACKUP_OVER_QUOTA           WB_USER_MESSAGE +  20
#define WAVETOP_BACKUP_OVER_REQ             WB_USER_MESSAGE +  25

#define WAVETOP_BACKUP_MODULE_NOT_LOAD      WB_USER_MESSAGE +  30

#define WAVETOP_BACKUP_QUIT                 WB_USER_MESSAGE +  31

#define WAVETOP_BACKUP_OCI_NO_DATA          WB_USER_MESSAGE +  32
                                           
#define WAVETOP_BACKUP_IOBUFFER_SIZE        8192
#define WAVETOP_BACKUP_FIRST_NOT_DO         WB_USER_MESSAGE +  288
/* The mirror file or directory or action. The options is used by 
 * Mirror-File-Way field.
 */

#define WAVETOP_MIRROR_FILE            1
#define WAVETOP_MIRROR_ACTION          2

/* the file modify action, which used by File-Action field. */
#define WAVETOP_MIRROR_ACTION_ADDED    (1<<1)
#define WAVETOP_MIRROR_ACTION_REMOVED  (1<<2)
#define WAVETOP_MIRROR_ACTION_MODIFIED (1<<3)
#define WAVETOP_MIRROR_ACTION_RENAMED  (1<<4)

/* The pull way (file or db) used Pull-Content field. */
#define WAVETOP_MIRROR_PULL_FILE         1
#define WAVETOP_MIRROR_PULL_DATABASE     2
#define WAVETOP_MIRROR_PULL_MSSQLBAK     3
#define WAVETOP_MIRROR_PULL_MSSQLMIDEL   4
#define WAVETOP_MIRROR_PULL_ORA_DB       5

#define WAVETOP_MIRROR_PUSH_ORA_DB       6
#define WAVETOP_MIRROR_PULL_MYSQL_DB     7
#define WAVETOP_MIRROR_PULL_GBASE_DB     8


#define WAVETOP_MIRROR_ORA_LOGMNR        1
#define WAVETOP_MIRROR_ORA_OBJ_SYNC      2
#define WAVETOP_MIRROR_ORA_DIC_CREATE    3

#define WAVETOP_MIRROR_ORA_SYN_BY_USER   1
#define WAVETOP_MIRROR_ORA_SYN_BY_TAB    2

/* The mirror way (auto or all), used by Mirror-Way field, 
 * in the PULL METHOD. 
 */
#define WAVETOP_MIRROR_AUTO            1
#define WAVETOP_MIRROR_ALL             2

/* The file system mirror options */
#define WAVETOP_MIRROR_FILE_NO_DEL     (1<<0)

/**
 * @desc server start mode
 * @{
 */

typedef int MiServStartMode;
#define WT_MI_SERV_STARTMODE_FS         (1<<0)
#define WT_MI_SERV_STARTMODE_MSSQL      (1<<1)
#define WT_MI_SERV_STARTMODE_ORACLE     (1<<2) 

/** @} */

/**
 * @mssql server schedule task bak mode
 * @{
 */

typedef int MiMssqlServerBakMode;
#define WT_MI_SERV_BAKMODE_CHECKDATA        (1<< 0)
#define WT_MI_SERV_BAKMODE_COMPRESSDATA     (1<< 1)
#define WT_MI_SERV_BAKMODE_COMBINEFILE      (1<< 2)
#define WT_MI_SERV_BAKMODE_WAITRUNNING      (1<< 3)

#define WT_MI_SERV_BAKMODE_NEEDLOG          (1<<10)


#define MI_LICENSE_TIME  2592000 /* 试用期30天 */
/** @} */

/* So that we can use inline on some critical functions, and use
 * GNUC attributes (such as to get -Wall warnings for printf-like
 * functions).  Only do this in gcc 2.7 or later ... it may work
 * on earlier stuff, but why chance it.
 *
 * We've since discovered that the gcc shipped with NeXT systems
 * as "cc" is completely broken.  It claims to be __GNUC__ and so
 * on, but it doesn't implement half of the things that __GNUC__
 * means.  In particular it's missing inline and the __attribute__
 * stuff.  So we hack around it.  PR#1613. -djg
 */
#if !defined(__GNUC__) || __GNUC__ < 2 || \
    (__GNUC__ == 2 && __GNUC_MINOR__ < 7) ||\
    defined(NEXT)
#define ap_inline
#define __attribute__(__x)
#define ENUM_BITFIELD(e,n,w)  signed int n : w
#else
#define ap_inline __inline__
#define USE_GNU_INLINE
#define ENUM_BITFIELD(e,n,w)  e n : w
#endif

/**
 * @defgroup win32 service naming define
 * @{
 */

#define WT_SERVICE_MISERV_DISPLAY_NAME          "WaveTop Mirror Server"
#define WT_SERVICE_MISERV_SERVICE_NAME          "WaveTop Mirror Server"
#define WT_SERVICE_MISERV_SERVICE_DESC          "WaveTop mirror system server daemon service"

#define WT_MI_CDAEMON_SERVICES_NAME             "Wavetop Mirror Client"
#define WT_MI_CDAEMON_SERVICES_DIS_NAME         "Wavetop Mirror Client"
#define WT_MI_CDAEMON_SERVICES_DESC             "WaveTop mirror system client daemon service"

#define WT_MI_LSNRCTL_SERVICES_NAME             "WavetopMirrorListen"
#define WT_MI_LSNRCTL_SERVICES_DIS_NAME         "WavetopMirrorListen"
#define WT_MI_LSNRCTL_SERVICES_DESC             "WavetopMirrorListen"

#define WT_SERVICE_MICDAEMON_FS_DISPLAY_NAME    "WaveTopMirrorCDaemonFS"
#define WT_SERVICE_MICDAEMON_FS_SERVICE_NAME    "WaveTopMirrorCDaemonFS"
#define WT_SERVICE_MICDAEMON_FS_SERVICE_DESC    "WaveTop mirror system client daemon service (File system mirror module)"

#define WT_SERVICE_MICDAEMON_MSSQL_DISPLAY_NAME    "WaveTopMirrorCDaemonMSSql"
#define WT_SERVICE_MICDAEMON_MSSQL_SERVICE_NAME    "WaveTopMirrorCDaemonMSSql"
#define WT_SERVICE_MICDAEMON_MSSQL_SERVICE_DESC    "WaveTop mirror system client daemon service (MSSQL mirror module)" 

/** @} */
//redo文件名称
#define WT_ORACLE_MIRROR_STANDBY_REDO           "DBRedo.log"
#define WT_ORACLE_MIRROR_STANDBY_QUICKCTRLFILE       "QuickwaveStandby.ctl"

/* 共享内存名 */
#define WT_MI_SHARE_MEMORY_NAME                 "wavetop_share"
#define WT_MI_SHARE_MEMORY_SIZE                 4

/**
 * @defgroup command file identified
 * @{
 */
/* command file format*/
#define WAVETOP_CONSOLE_UNKOWN_COMMANDFILE         8000
#define WAVETOP_CONSOLE_COMMANDFILE_TYPE_FS        1
#define WAVETOP_CONSOLE_COMMANDFILE_TYPE_ORA       2
#define WAVETOP_CONSOLE_COMMANDFILE_TYPE_MSSQL     3
#define WAVETOP_CONSOLE_COMMANDFILE_TYPE_TRS       4

#define WAVETOP_CONSOLE_ORA_CMD_GET_DB_USER_TAB    2001
#define WAVETOP_CONSOLE_ORA_CMD_GET_DB_USERS       2002
/** @} */

/**
 * @defgroup command file identified
 * @{
 */
/* command file format*/
#define WAVETOP_CONSOLE_UNKOWN_COMMANDFILE         8000
#define WAVETOP_CONSOLE_COMMANDFILE_TYPE_FS        1
#define WAVETOP_CONSOLE_COMMANDFILE_TYPE_ORA       2
#define WAVETOP_CONSOLE_COMMANDFILE_TYPE_MSSQL     3
#define WAVETOP_CONSOLE_COMMANDFILE_TYPE_TRS       4

#define WAVETOP_CONSOLE_ORA_CMD_GET_DB_USER_TAB    2001
/** @} */

#define WAVETOP_ORACLE_STORE_ASM                    1

#define WT_INCREMENT_ARC                            1
#define WT_INCREMENT_REDO                           2


#endif /* !defined(__MIRROR_H_) */
