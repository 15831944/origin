/** =============================================================================
 ** Copyright (c) 2003 WaveTop Information Corp. All rights reserved.
 **
 ** The backup system
 **
 ** =============================================================================
 */

/** 
 * File:        taskctrl/backup_proto.h
 * Description: backup Transport protocol and return state
 */
  
/* Backup Transport Protocols */

#ifndef __WAVETOP_BACKUP_PROTOCOL_H_
#define __WAVETOP_BACKUP_PROTOCOL_H_ 1

#ifdef WIN32
#include <windows.h>
#endif

#if defined(AIX)
#include <strings.h>
#endif

#include "wconfig.h"

/* The backserver port(TCP) */
#ifdef WAVETOP_BACKUP_BACKUP
#define WAVETOP_BACKUP_PORT          58861
#define WAVETOP_BACKUP_AGENT_PORT    58841
#else
#define WAVETOP_BACKUP_PORT          58831
#define WAVETOP_BACKUP_AGENT_PORT    58811
#define BACKUPSERVER_WEBUI_PORT      58881
#define BACKUP_STAGING_PORT          58888
#define BACKUP_STAGING_MON_PORT      58886
#define BACKUP_MEDSERVER_PORT        58885
#define BACKUP_MISERVER_PORT         58884
#define BACKUP_MNSERVER_PORT         58883
#define BACKUP_SERVER_FTP_PORT		 21
#endif

/* The server type */
#define WAVETOP_BACKUP_SERVER          1
#define WAVETOP_MEDIA_SERVER           2
#define WAVETOP_MIRROR_SERVER          3
#define WAVETOP_STAGING_SERVER         4
#define WAVETOP_AGENT_SERVER           5
#define WAVETOP_CONTROL_SERVER         6
#define WAVETOP_MANAGE_SERVER          7

///////////////////////////////////////////////////////////////////////////////
// Protocols definition(description) Begin                                   //
///////////////////////////////////////////////////////////////////////////////

/* Currently Protocol Version Number. See backup_protocol.doc */
#define WAVETOP_BACKUP_VERSION        1000
#define WAVETOP_BACKUP_VERSION_1      1001
#define WAVETOP_BACKUP_VERSION_2      1002
#define WAVETOP_BACKUP_VERSION_3      1003
#define WAVETOP_BACKUP_VERSION_4      1004

/* Protocol Define Method */
#define BACKUP_PROTO_LOGIN            1       /* Login */
#define BACKUP_PROTO_BACKUP           2       /* Backup */
#define BACKUP_PROTO_RESTORE          3       /* Restore */
#define BACKUP_PROTO_QUERY            4       /* Query Server Side Backup Files View */
#define BACKUP_PROTO_DELETE           5       /* Delete Files on the Server */
#define BACKUP_PROTO_USER_MANAGE      7       /* User Manage */
#define BACKUP_PROTO_GET_MTYPE        11      /* Get Magazine Type */
#define BACKUP_PROTO_BACKUP_OBJECT    BACKUP_PROTO_BACKUP
#define BACKUP_PROTO_RESTORE_OBJECT   BACKUP_PROTO_RESTORE
#define BACKUP_PROTO_MODIFY_SHARE     100     /* Modify Share */
#define BACKUP_PROTO_QUERY_SHARE      101     /* Query Share */
#define BACKUP_PROTO_DOWNLOAD_SHARE   102     /* Download Share */
#define BACKUP_PROTO_UPLOAD_SHARE     103     /* UpLoad Share */
#define BACKUP_PROTO_UPDATE           200     /* Client Auto Update */
#define BACKUP_PROTO_POLICY           300     /* Policy Dispense */
#define BACKUP_PROTO_TASK_MONITOR     301     /* Task Monitor */
#define BACKUP_PROTO_MOD_TASK_STATE   302     /* Modify Task end state */
#define BACKUP_PROTO_NET_CONFIG       303     /* Config stag server such as netflux control */
#define BACKUP_PROTO_CONFIG_MANAGE    304     /* Configfile Manage */
#define BACKUP_PROTO_SERVICES_CONTROL 305     /* services Control */
#define BACKUP_PROTO_QUERY_ROUTER     306     /* query router from mnserver */
#define BACKUP_PROTO_QUERY_REPORT     307     /* query report from mnserver */
#define BACKUP_PROTO_WRITE_REPORT     308     /* write report by mnserver */
#define BACKUP_PROTO_REWRITE_REPORT   309     /* mod report by mnserver */
#define BACKUP_PROTO_QUERY_USER       310     /* query user from mnserver */
#define BACKUP_PROTO_MODIFY_USER      311     /* modify user by mnserver */
#define BACKUP_PROTO_DELETE_REPORT	  312	  /* delete report by mnserver */
#define BACKUP_PROTO_QUERY_DETAIL     313     /* query detail of report from mnserver */

#define BACKUP_PROTO_RECOVER          314     /* Recover */	
#define WAVETOP_AGENT_BACKUP_STORE    434
/**
 * @defgroup agent push protocol parameter. For agents.
 * @{
 */

#define WAVETOP_AGENT_BACKUP_ORACLE    400     /* ORACLE push */
#define WAVETOP_AGENT_BACKUP_MSSQL     401     /* MSSQL server push */
#define WAVETOP_AGENT_BACKUP_FILE      402     /* File push */
#define WAVETOP_AGENT_BACKUP_DELETE    403     /* Weblogic push */
#define WAVETOP_AGENT_BACKUP_WEBSPHERE 404     /* WebSphere push */
#define WAVETOP_AGENT_BACKUP_APACHE    405     /* Apache push */
#define WAVETOP_AGENT_BACKUP_TOMCAT    406     /* Tomcat push */
#define WAVETOP_AGENT_BACKUP_SYSTEM    407     /* Operate System push */
#define WAVETOP_AGENT_BACKUP_SYBASE    408     /* SYBASE server push */
#define WAVETOP_AGENT_RESTORE_MSSQL    409
#define WAVETOP_AGENT_RESTORE_FILE     410
#define WAVETOP_AGENT_RESTORE_ORACLE   411
#define WAVETOP_AGENT_DELETE_MSSQL     412
#define WAVETOP_AGENT_DELETE_FILE      413
#define WAVETOP_AGENT_DELETE_ORACLE    415
#define WAVETOP_AGENT_QUERY_FILE       416
#define WAVETOP_AGENT_QUERY_MSSQL      417
#define WAVETOP_AGENT_QUERY_ORACLE     418
#define WAVETOP_AGENT_BACKUP_SQL_EXP   419
#define WAVETOP_AGENT_RESTORE_SQL_EXP  420
#define WAVETOP_AGENT_BACKUP_MYSQL     421
#define WAVETOP_AGENT_RESTORE_MYSQL    422
#define WAVETOP_AGENT_BACKUP_DB2       423
#define WAVETOP_AGENT_RESTORE_DB2      424
#define WAVETOP_AGENT_BACKUP_DRIVE     425    
#define WAVETOP_AGENT_RESTORE_DRIVE    426
#define WAVETOP_AGENT_BACKUP_VMWARE    430
#define WAVETOP_AGENT_RESTORE_VMWARE   431
#define WAVETOP_AGENT_BACKUP_UNIVERSAL 432
#define WAVETOP_AGENT_RESTORE_UNIVERSAL 433
#define WAVETOP_AGENT_BACKUP_ORACLE_LOG 434

#define WAVETOP_AGENT_BACKUP_V2P        435
#define WAVETOP_AGENT_BACKUP_P2V        436
#define WAVETOP_AGENT_BACKUP_CLOUD      437
#define WAVETOP_AGENT_BACKUP_ORCL_EXP   438
#define WAVETOP_AGENT_RESTORE_ORCL_EXP  439
#define WAVETOP_AGENT_BACKUP_TAPE	    440
#define WAVETOP_AGENT_BACKUP_D2C        442
#define WAVETOP_AGENT_BACKUP_BFILE      443


#define WAVETOP_AGENT_CDP_BACKUP_ORACLE     521    /* CDPADD */
#define WAVETOP_AGENT_CDP_BACKUP_MSSQL      522     /* CDPADD */
#define WAVETOP_AGENT_CDP_RESTORE_ORACLE    523
#define WAVETOP_AGENT_CDP_RESTORE_MSSQL     524
#define WAVETOP_AGENT_CDP_REALRESTORE       525
#define WAVETOP_AGENT_CDP_BACKUP_MYSQL      526
#define WAVETOP_AGENT_CDP_RESTORE_MYSQL     527
#define WAVETOP_AGENT_CDP_BACKUP_DB2        528
#define WAVETOP_AGENT_CDP_RESTORE_DB2       529
#define WAVETOP_AGENT_CDP_RESTORE_UNI       530
#define WAVETOP_AGENT_ALARM_START           540
/** @} */

/**
 * Begin of The Protocol Message Code
 * State Code definition
 *  0           Succeeded (SUCCEEDED)
 *  1   - 100   Response Side State Code
 *  101 - 200   Request  Side State Code
 *  201 - 300   Protocol Failed (FAILED)
 */
#define WAVETOP_BACKUP_OK                       0   /* Success (SUCCEEDED)*/
#define WAVETOP_BACKUP_NOT_IMPLEMENTED          1   /* the Method(Function) Not Implemented */
#define WAVETOP_BACKUP_FORBIDDEN                2   /* Doesnot Permission */
#define WAVETOP_BACKUP_INTERNAL_ERROR           3   /* Internal Error */
#define WAVETOP_BACKUP_FILE_BUSY                4   /* Server File Busy */
#define WAVETOP_BACKUP_VERSION_NOT_SUPPORTED    5   /* Current Version Does Not Support */
#define WAVETOP_BACKUP_INDEX_DAMAGED            6   /* Index Damaged of the Backup Files */
#define WAVETOP_BACKUP_AUTHORIZED_FAILED        7   /* User Authorized Failed */
#define WAVETOP_BACKUP_THREAD_EXHAUST           8   /* Server Work Thread Exhaust */
#define WAVETOP_BACKUP_OVER_QUOTA               9   /* Over storage Capability Quota */
#define WAVETOP_BACKUP_OVER_USER_LIMIT          10  /* Over The Users Amount Limit */
#define WAVETOP_BACKUP_ARCHIVE_CONVEYED         11  /* Server Data be Conveyed */
#define WAVETOP_BACKUP_HAVE_BROKEN_POINT        12   /* server has p2v broken point */

#define WAVETOP_BACKUP_BAD_REQUEST              101 /* Bad Request */
#define WAVETOP_BACKUP_FILENAME_TOO_LONG        102 /* Filename Too Long */
#define WAVETOP_BACKUP_FILE_TOO_LARGE           103 /* File Too Large */
#define WAVETOP_BACKUP_REQUEST_ENTITY_TOO_LARGE 104 /* Request  Entity Too Large */
#define WAVETOP_BACKUP_FILE_EMPTY               105 /* 文件为空 */
#define WAVETOP_BACKUP_BAD_FILE_INDEX           201 /* Bad File Index  */
#define WAVETOP_BACKUP_FILE_NO_CHANGE           202 /* File No change  */
#define WAVETOP_BACKUP_FILE_NO_MODIFY           203 /* File No modify  */
#define WAVETOP_BACKUP_FTP_REQUEST              204 /* FTP Request */
#define WAVETOP_BACKUP_WM_QUERYCB_OTHER			205 //unknown error where querey changed block


#define Byte_2_MB(a)    ((a)>>20)

/* user-defined Message Begin */
#define WB_USER_MESSAGE                         1001

/* Call-back Message Define */
#define WAVETOP_BACKUP_SYSTEM_BUSY          WB_USER_MESSAGE + 0
#define WAVETOP_BACKUP_CONNECT_DOWN         WB_USER_MESSAGE + 1
#define WAVETOP_BACKUP_NO_MEMORY            WB_USER_MESSAGE + 2
#define WAVETOP_BACKUP_FILE_NOT_INTGRETY    WB_USER_MESSAGE + 3
#define WAVETOP_BACKUP_FILTERED             WB_USER_MESSAGE + 4
#define WAVETOP_BACKUP_RATE                 WB_USER_MESSAGE + 5
#define WAVETOP_BACKUP_BEGIN                WB_USER_MESSAGE + 6
#define WAVETOP_BACKUP_END                  WB_USER_MESSAGE + 7
#define WAVETOP_BACKUP_OPEN_FILE_ERROR      WB_USER_MESSAGE + 8
#define WAVETOP_BACKUP_CREAT_FILE_ERROR     WB_USER_MESSAGE + 9
#define WAVETOP_BACKUP_OPEN_DIR_ERROR       WB_USER_MESSAGE + 10
#define WAVETOP_BACKUP_CREAT_DIR_ERROR      WB_USER_MESSAGE + 11
#define WAVETOP_BACKUP_FILE_NOT_EXIST       WB_USER_MESSAGE + 12
#define WAVETOP_BACKUP_INVALID_SYNTAX       WB_USER_MESSAGE + 13
#define WAVETOP_BACKUP_CONNECT_SUCCESS      WB_USER_MESSAGE + 14
#define WAVETOP_BACKUP_FSDETECT_FAILED      WB_USER_MESSAGE + 15
#define WAVETOP_BACKUP_INTERRUPT_TASK       WB_USER_MESSAGE + 16
#define WAVETOP_BACKUP_DEBUG_CALLBACK       WB_USER_MESSAGE + 17
#define WAVETOP_BACKUP_DEMO_EXPIRED         WB_USER_MESSAGE + 18
#define WAVETOP_BACKUP_ABSTRACT_OK          WB_USER_MESSAGE + 19
#define WAVETOP_BACKUP_ABSTRACT_ERROR       WB_USER_MESSAGE + 20
#define WAVETOP_BACKUP_IMPORTDB_ERROR       WB_USER_MESSAGE + 21
#define WAVETOP_BACKUP_EXPORTDB_ERROR       WB_USER_MESSAGE + 22
#define WAVETOP_BACKUP_EXPORTDB_SUCCESS     WB_USER_MESSAGE + 23
#define WAVETOP_BACKUP_USER_DOES_NOT_EXIST  WB_USER_MESSAGE + 24
#define WAVETOP_BACKUP_DIR_DOES_NOT_EXIST   WB_USER_MESSAGE + 25
#define WAVETOP_BACKUP_ARCHIVE_NO_FOUND     WB_USER_MESSAGE + 26
#define WAVETOP_BACKUP_DISK_DOES_NOT_NTFS   WB_USER_MESSAGE + 27
#define WAVETOP_BACKUP_NOT_ABSOLUTE_PATH    WB_USER_MESSAGE + 28
#define WAVETOP_BACKUP_BAD_PATHNAME         WB_USER_MESSAGE + 29
#define WAVETOP_BACKUP_SKIPPED              WB_USER_MESSAGE + 30
#define WAVETOP_BACKUP_LIMIT_BACKUP         WB_USER_MESSAGE + 31
#define WAVETOP_BACKUP_LIMIT_RESTORE        WB_USER_MESSAGE + 32
#define WAVETOP_BACKUP_LIMIT_DELETE         WB_USER_MESSAGE + 33
#define WAVETOP_BACKUP_SUCCESS_NOMATCH      WB_USER_MESSAGE + 34
#define WAVETOP_BACKUP_NO_SPACE             WB_USER_MESSAGE + 35


/* Server Doesnot Correspond Update Files */
#define WAVETOP_BACKUP_UPDATE_NO_FILES      WB_USER_MESSAGE + 100
#define WAVETOP_BACKUP_UPDATE_FILES         WB_USER_MESSAGE + 101
/* User need Update, But Not Send Files */
#define WAVETOP_BACKUP_UPDATE_YES           WB_USER_MESSAGE + 102
/* Policy Dispense */
#define WAVETOP_BACKUP_CONFIG_FILE          WB_USER_MESSAGE + 103

/* Store Method */
#define WSTORE_METHOD_APPEND        0
#define WSTORE_METHOD_COVER         1

#define WSTORE_MESSAGE                         2000
/* The error code */
#define WSTORE_OK                     WAVETOP_BACKUP_OK
#define WSTORE_RECORD_MALFORMED       WSTORE_MESSAGE + 1
#define WSTORE_DBM_OPEN_FAILED        WSTORE_MESSAGE + 2
#define WSTORE_DBM_OPERATION_FAILED   WSTORE_MESSAGE + 3
#define WSTORE_NO_MEMORY              WSTORE_MESSAGE + 4
#define WSTORE_INVALID_SYNTAX         WSTORE_MESSAGE + 5
#define WSTORE_ARCHIVE_OPEN_FAILED    WSTORE_MESSAGE + 100
#define WSTORE_NO_FILE                WSTORE_MESSAGE + 101
#define WSTORE_FILE_NAME_TOO_LONG     WSTORE_MESSAGE + 102
#define WSTORE_ARCHIVE_NO_FOUND       WSTORE_MESSAGE + 103
#define WSTORE_NO_ENOUGH_ROOM         WSTORE_MESSAGE + 104
#define WSTORE_ARCHIVE_NO_VER_FILE    WSTORE_NO_FILE

#define BACKUP_ORACLE_MESSAGE                       3000
#define BACKUP_ORACLE_DB_FAILURE                    BACKUP_ORACLE_MESSAGE + 3
#define BACKUP_ORACLE_TABLESPACE_IS_EXIST           BACKUP_ORACLE_MESSAGE + 101 /* Tablespace is exist */
#define BACKUP_ORACLE_RMAN_USER_IS_EXIST            BACKUP_ORACLE_MESSAGE + 102 /* RMAN user is exist */
#define BACKUP_ORACLE_AIMDB_HAS_REGISTERED          BACKUP_ORACLE_MESSAGE + 103 /* Aim database has registered */
#define BACKUP_ORACLE_BS_KEY_NOT_IN_EXPFILES        BACKUP_ORACLE_MESSAGE + 104 /* BS_KEY not in exported file list */
#define BACKUP_ORACLE_BS_KEY_NOT_IN_CATALOG         BACKUP_ORACLE_MESSAGE + 105 /* BS_KEY not in RMAN catalog */
#define BACKUP_ORACLE_DIFFERENT_INCARNTATION        BACKUP_ORACLE_MESSAGE + 106 /* Backuped files in different incarnation */
#define BACKUP_ORACLE_INVALID_BS_KEY                BACKUP_ORACLE_MESSAGE + 107 /* Invalid backup set key */
#define BACKUP_ORACLE_INVALID_INCARNATION           BACKUP_ORACLE_MESSAGE + 108 /* Invalid incarnation number */
#define BACKUP_ORACLE_INVALID_RESTORE_OPERATION     BACKUP_ORACLE_MESSAGE + 109 /* Restore operation is invalid */
#define BACKUP_ORACLE_INVALID_BINARY_FIELD          BACKUP_ORACLE_MESSAGE + 110 /* Invalid binary field */
#define BACKUP_ORACLE_BINARY_BUFFER_OVERFLOW        BACKUP_ORACLE_MESSAGE + 111 /* The binary buffer is overflow */
#define BACKUP_ORACLE_INCARNATION_KEY_NOT_FOUND     BACKUP_ORACLE_MESSAGE + 112 /* The incarnation key not found in binary */

#define WT_MSSQL_MESSAGE                            4000
#define WT_WILL_SEND_RECE_IO            WT_MSSQL_MESSAGE + 1
#define WT_WAIT_FOR_IO                  WT_MSSQL_MESSAGE + 2
#define WT_EXIST_NEW_IO                 WT_MSSQL_MESSAGE + 3
#define WT_SAVE_NO_IO                   WT_MSSQL_MESSAGE + 4
#define WT_INTERNAL_ERROR				WT_MSSQL_MESSAGE + 5


/****************************************************************************
 * End of The Protocol Message Code                                         *
 ***************************************************************************/

/* Parameter - Transport File Parameter */
#define RESTORE_INHERIT_DIRNAME       (1<<7)  /* Inherit Previous Directory Name omit same segment */

/* Parameter - Code Mode */
#define BACKUP_PSW_TYPE_CLEAR         (1<<0)  /* Clear Mode Transport */
#define BACKUP_PSW_TYPE_BASE64        (1<<1)  /* MD5-BASE64 ncrypt Mode Transport */

/* Parameter - Query-Option Field Define Query Parameter */
#define BACKUP_QUERY_BASE_INFO   1            /* Query Base Information */
#define BACKUP_QUERY_HISTROY     2            /* Query HIstroy Information */
#define BACKUP_QUERY_SHARE_ACL   3            /* Query Share Access Control List Permission */
#define BACKUP_QUERY_PATH        4            /* Query store path */
#define BACKUP_QUERY_FQ_FILE     5
/* Operating System Type */
#define BACKUP_OS_TYPE_WINNT     1            /* Windows NT Operating System */
#define BACKUP_OS_TYPE_WIN9X     2            /* Windows Consumer Operating System */
#define BACKUP_OS_TYPE_UNIX      3            /* Unix Operating System */

/* File Type */
#define BACKUP_FILE_FILE         1
#define BACKUP_FILE_DIRECTORY    2

/* Policy file type for flag field of policy base info */
#define BACKUP_POLICY_FILE       (1 << 31)
#define BACKUP_POLICY_MSSQL      (1 << 30)
#define BACKUP_POLICY_ORACLE     (1 << 29)

/* UserInfoSt flags */
#define WACL_DELETE                   (1<<0)
#define WACL_DOMAIN_USER              (1<<1)
#define WACL_SUPER_USER               (1<<2)
#define WACL_LIMIT_BACKUP             (1<<3)
#define WACL_LIMIT_RESTORE            (1<<4)
#define WACL_LIMIT_DELETE             (1<<5)
#define WACL_AUTHWAY_LOCAL_SYSTEM     (1<<31)    /* user auth way manage used by server */
#define WACL_AUTHWAY_WIN_USER         (1<<30) 
#define WACL_AUTHWAY_ANONYMOUS        (1<<29)  
#define WACL_AUTHWAY_CA_USER          (1<<28)
#define WACL_NO_RECORD_HOST           (1<<21)    /* 登陆时无需记录客户端IP(用于Web的备份视图控件) */
#define WACL_AUTHWAY_BAKEXE_LOGIN     (1<<20)    /* backup.exe登录，任意用户可以登录，在服务器验证。*/
/* Authentication Mode */
#define BACKUP_AUTHWAY_LOCAL_SYSTEM   WACL_AUTHWAY_LOCAL_SYSTEM     /* Local System Authentication Mode */
#define BACKUP_AUTHWAY_WIN_USER       WACL_AUTHWAY_WIN_USER  /* Windows Domain User Authentication Mode */
#define BACKUP_AUTHWAY_ANONYMOUS      WACL_AUTHWAY_ANONYMOUS                    /* Anonymous Login Mode */
#define BACKUP_AUTHWAY_CA_USER        WACL_AUTHWAY_CA_USER                    /* CA Authentication Mode */
#define BACKUP_AUTHWAY_RESTORE        (1<<6)  /* Authenticated-Restore */

/* Archive file Type */
#define WACL_TYPE_SHARE            0
#define WACL_TYPE_SPECIAL          1

/* Restore Parameter */
#define RESTORE_OPTION_RECURSE_SUBDIR     (1<<0)
#define RESTORE_OPTION_INTEGRITY_VERIFY   (1<<1)
#define RESTORE_OPTION_POLICY_RESTORE     (1<<2)

/* Backup Type */
#define BACKUP_TYPE_FULL                  (1<<0)
#define BACKUP_TYPE_INCREASE              (1<<1)

/* Transport Mode */
#define BACKUP_TRANSFER_RSYNC             (1<<2)  /* rsync Transport (increment Transport) */
#define BACKUP_TRANSFER_SYNC              (1<<3)  /* sync  Transport */
#define BACKUP_OPTION_VERSION_INCREASE    (1<<10) /* Version Increase 1 after Backup */
#define BACKUP_OPTION_BKTMP_DB            (1<<15) /* backup tmep database before restore*/
#define BACKUP_OPTION_CHECK_INTEGRITY     (1<<16) /* check integrity before backup database*/
#define BACKUP_OPTION_FILTER_SYSTEM_DISK  (1<<17) /* symbol whether to backup system disk*/

/* In Backup, Client Must Wait Server Failed Response */
#define BACKUP_OPTION_WAIT_FAILED_RESPONSE    (1<<11) 

/* In Backup, Client Must Wait Server Succeeded Response */
#define BACKUP_OPTION_WAIT_SUCCESS_RESPONSE   (1<<12)  

/* 全量和增量文件名不同 */
#define BACKUP_OPTION_OVERLOOK_FILENAME       (1<<13)
/* 还原增量时不要全量文件 */
#define BACKUP_OPTION_RESTORE_WITHOUT_FULL    2

/* Backup Parameter */
#define BACKUP_OPTION_CHILD_DIR         (1<<20)   /* Recursion Subdirectories */
#define BACKUP_OPTION_CHECK_DATA        (1<<21)   /* Check Data Integrity after Backup */
#define BACKUP_OPTION_ONLY_LAST_BACKUP  (1<<22)   /* Only backup files which modifed 
                                                   * from last backuping 
                                                   */
#define BACKUP_OPTION_FULL              BACKUP_TYPE_FULL
#define BACKUP_OPTION_INCREASE          BACKUP_TYPE_INCREASE

/* User Manage Method Parameter Define */
#define BACKUP_USER_MODIFY_PSW        1       /* Modify Password */
#define BACKUP_USER_DELETE_USER       2       /* Delete User */
#define BACKUP_USER_MODIFY_INFO       3       /* Modify User Information */
#define BACKUP_USER_GET_ALL_USER      4       /* Get All User And Group */
#define BACKUP_USER_ADD_USER          5       /* Add User */
#define BACKUP_USER_ADD_USER_ARCHIVE  6       /* Add User Archive File */
#define BACKUP_USER_MODIFY_USER_ARCHIVE  7    /* Modify user archive file */ 
#define BACKUP_USER_CLEAN_USER_STORE  8       /* Clean User Store File */
#define BACKUP_USER_MODIFY_USER_STORE   9
#define BACKUP_USER_MODIFY_ARC_OCCUPANT 10

/* backup22 */
#define DEVICE_MANAGE_ADD_DEVICE      1
#define DEVICE_MANAGE_MODIFY_DEVICE   2
#define DEVICE_MANAGE_DELETE_DEVICE   3
#define DEVICE_MANAGE_ADD_MEDIA       4
#define DEVICE_MANAGE_MODIFY_MEDIA    5
#define DEVICE_MANAGE_DELETE_MEDIA    6
#define DEVICE_MANAGE_QUERY_DEVICE    7
#define DEVICE_MANAGE_QUERY_MEDIA     8
#define DEVICE_MANAGE_CLEAN_MEDIA     9
#define DEVICE_MANAGE_DELETE_OWNER    10

/*Config Method Parameter Define */
#define CONFIG_MANAGE_READ_NORMAL_CONFIG       1
#define CONFIG_MANAGE_MODIFY_NORMAL_CONFIG     2
#define CONFIG_MANAGE_READ_ADVANCE_CONFIG      3
#define CONFIG_MANAGE_MODIFY_ADVANCE_CONFIG    4
#define CONFIG_MANAGE_READ_RUNNING_STATUS      5

/* Net Config Method Parameter Define */
#define BACKUP_NETCONFIG_FLUX_CONTROL 1       /* Modify Net Flux Control */
#define BACKUP_NETCONFIG_GET_FLUX     2       /* Get Net Flux Setting */

/* Share Type */
#define BACKUP_SHARE_SHARE            1       /* Share */
#define BACKUP_SHARE_UNSHARE          2       /* Not Share */

/* The file name type */
#define BACKUP_FILE_NAME_SHARE        1       /* Share File name Type */
#define BACKUP_FILE_NAME_PHYSICAL     2       /* Physical File name Type */

/* Share Access Authorization */
#define BACKUP_SHARE_READ             (1<<0)  /* ReadOnly Share */
#define BACKUP_SHARE_WRITE            (1<<1)  /* Read Write Share */

#ifdef WAVETOP_BACKUP_BACKUP
/* User And Group Type */
#define BACKUP_USER_USER              (1 <<  0)       /* User */
#define BACKUP_USER_GROUP             (1 <<  1)       /* Group */
#define BACKUP_USER_NO_GROUP          (1 <<  2)       /* Not Group, Not User */
#define BACKUP_USER_GROUP_ALL_USER    (1 <<  3)       /* whole Group */

#else
#define BACKUP_USER_USER              1       /* User */
#define BACKUP_USER_GROUP             2       /* Group */
#define BACKUP_USER_NO_GROUP          3       /* Not Group, Not User */
#define BACKUP_USER_GROUP_ALL_USER    4       /* whole Group */

#endif

/* Restore Parameter */
#define RESTORE_OPTION_FILE_TIME      (1<<20)
#define RESTORE_OPTION_FILE_ATTRIBUTE (1<<21)

/* task status */
#define WAVETOP_TASK_START           0
#define WAVETOP_TASK_WAIT            1
#define WAVETOP_TASK_END             2

/* Get the task option */
#define WAVETOP_GET_ALL_TASK         (1<<0)
#define WAVETOP_GET_TASK_TYPE        (1<<1)
#define WAVETOP_GET_TASK_STATUS      (1<<2)
#define WAVETOP_GET_TASK_USER        (1<<3)
#define WAVETOP_GET_OBJECT_TYPE      (1<<4)

/* Update Parameter */
#define BACKUP_UPDATE_QUERY           1       /* Query Only Update or not */
#define BACKUP_UPDATE_UPDATE          2       /* Request Update */

#ifdef WAVETOP_BACKUP_BACKUP
/* The service name of backup server for Windows (2000/XP/2003) */
#define WT_BK_SERVICE_SERVICE_NAME          "WaveTopBackupServer"
#define WT_BK_SERVICE_DISPLAY_NAME          "WaveTopBackupServer"
#define WT_BK_AGENT_SERVICE_NAME            "WaveTopBackupAgent"
#define WT_BK_AGENT_DISPLAY_NAME            "WaveTopBackupAgent"
#else
/* The service name of backup server for Windows (2000/XP/2003) */
#define WT_BK_SERVICE_SERVICE_NAME          "WaveTopBackup6Server"
#define WT_BK_SERVICE_DISPLAY_NAME          "WaveTopBackup6Server"
#define WT_BK_AGENT_SERVICE_NAME            "WaveTopBackup3Agent"
#define WT_BK_AGENT_DISPLAY_NAME            "WaveTopBackup3Agent"
#define WT_BK_STAGING_SERVICE_NAME          "WaveTopBackupStaging"
#define WT_BK_STAGING_DISPLAY_NAME          "WaveTopBackupStaging"
#endif
/* The service name of UME(the United Message Engine)*/
#define WT_UME_AGENT_SERVICE_NAME           "WaveTopUMEAgent"
#define WT_UME_AGENT_DISPLAY_NAME           "WaveTopUMEAgent"

/* Default Socket Time out (seconds) */
#define BACKUP_SOCKET_TIME_OUT        36000

/****************************************************************************
 * The backup Object Type:                                                  *
 * 1) Registry for windows                                                  *
 * 2) MSSQL server for windows                                              *
 ***************************************************************************/

/* backup object type */
#define BACKUP_OBJECT_REGISTER          1       /* Register */
#define BACKUP_OBJECT_MSSQL             2       /* MSSQL Database Object */
#define BACKUP_OBJECT_ORACLE            3       /* Oracle Database Object */
#define BACKUP_OBJECT_FILE              4       /* File */
#define BACKUP_OBJECT_POLICY            5       /* Policy Object */
#define BACKUP_OBJECT_SYSTEM            6       /* operate system */
#define BACKUP_OBJECT_SYBASE            7       /* sybase Database Object */
#define BACKUP_OBJECT_ORACLE_CLIENT     8       /* Oracle Database client Object */
#define BACKUP_OBJECT_MYSQL             9       /* Mysql Database Object */
#define BACKUP_OBJECT_DRIVE             10      /* Drive Object */
#define BACKUP_OBJECT_DB2               11      /* DB2 Database Object */
#define BACKUP_OBJECT_P2V               12      /* P2V */
#define BACKUP_OBJECT_VMWARE            13      /* VMWare Database Object */
#define BACKUP_OBJECT_UNI               14
#define BACKUP_OBJECT_DSSQL             15      /* DSSQL Database Object*/
#define BACKUP_OBJECT_ORALOG            16
#define BACKUP_OBJECT_CLOUD             17
#define BACKUP_OBJECT_ORASIGLE          18      /* orcl Database Object*/
#define BACKUP_OBJECT_CITRIX            19     
#define BACKUP_OBJECT_BFILE             20   


/* backup database object name */
#define BACKUP_OBJECT_BACKUPCORE            "BACKUPCORE"    /* Login Backupcore */    
#define BACKUP_OBJECT_CLIENT_FILE_NAME      "CLIENTFILE"    /* File Object Name */
#define BACKUP_OBJECT_FILE_NAME             "FILE"          /* File Object Name */
#define BACKUP_OBJECT_DIFFFILE_CLIENT_NAME  "CLIENTDIFFFILE"
#define BACKUP_OBJECT_DIFFFILE_NAME         "DIFFFILE"
#define BACKUP_OBJECT_MSSQL_NAME            "MSSQL"         /* MSSQL Database Object Name */
#define BACKUP_OBJECT_ORACLE_NAME           "ORACLE"        /* ORACLE Database Object Name */
#define BACKUP_OBJECT_REGISTER_NAME         "Register"      /* Register Object Name */
#define BACKUP_OBJECT_POLICY_NAME           "Policy"        /* Policy Object Name */
#define BACKUP_OBJECT_WEBLOGIC_NAME         "WEBLOGIC"      /* WebLogic Object Name */
#define BACKUP_OBJECT_WEBSPHERE_NAME        "WEBSPHERE"     /* WebSphere Object Name */
#define BACKUP_OBJECT_APACHE_NAME           "APACHE"        /* Apache Object Name */    
#define BACKUP_OBJECT_TOMCAT_NAME           "TOMCAT"        /* Apache Object Name */
#define BACKUP_OBJECT_SYSTEM_NAME           "SYSTEM"        /* Operate System Backup Name*/
#define BACKUP_OBJECT_SYBASE_NAME           "SYBASE"        /* Sybase Database Object Name */
#define BACKUP_OBJECT_ORACLE_CLIENT_NAME    "CLIENTORACLE"  /* Oracle Database client Object Name*/
#define BACKUP_OBJECT_MSSQL_CLIENT_NAME     "CLIENTMSSQL"   /* Oracle mssql client Object Name*/
#define BACKUP_OBJECT_DMSSQL_CLIENT_NAME    "CLIENTDMSSQL"  /* Dmssql client Object Name*/
#define BACKUP_OBJECT_FILE_CLIENT_NAME      "CLIENTFILE"    /* File client Object Name*/
#define BACKUP_OBJECT_DSSQL_NAME            "DSSQL"         /* mssql/sybase 定时备份 */
#define BACKUP_OBJECT_DB2_SERVER_NAME       "SERVERDB2"
#define BACKUP_OBJECT_DB2_CLIENT_NAME       "CLIENTDB2"
#define BACKUP_OBJECT_MYSQL_NAME            "MYSQL"
#define BACKUP_OBJECT_MYSQL_CLIENT_NAME     "CLIENTMYSQL"
#define BACKUP_OBJECT_DRIVE_SERVER          "DRIVE"  
#define BACKUP_OBJECT_ALARM_SERVER_NAME     "ALARMSERVER"   
#define BACKUP_OBJECT_VMWARE_NAME           "VMWARE" 
#define BACKUP_OBJECT_CLOUD_NAME            "CLOUD"
#define BACKUP_OBJECT_CITRIX_NAME           "CITRIX"
#define BACKUP_OBJECT_TAPE_NAME             "TAPE"
#define BACKUP_OBJECT_P2V_CLIENT_NAME		"CLIENTP2V"
#define BACKUP_OBJECT_P2V_NAME              "P2V"
#define BACKUP_OBJECT_V2V_NAME              "V2V"
#define BACKUP_OBJECT_UNIVERSAL_NAME		"UNIFILE"
#define BACKUP_OBJECT_UNIVERSAL_CLIENT_NAME "CLIENTUNI"
#define BACKUP_OBJECT_ORACLE_LOG_CLIENT_NAME "CLIENTORALOG"
#define BACKUP_OBJECT_ORACLE_LOG_NAME        "ORALOG"
#define BACKUP_OBJECT_ORACLE_SIGLE_NAME       "ORASIGLE"         /* oracle 单表备份 */
#define BACKUP_OBJECT_ORACLESIGLE_CLIENT_NAME "CLIENTORCLSIGLE"  /* oracle client Object Name*/
#define BACKUP_OBJECT_DB2_NAME		"DB2"
#define BACKUP_OBJECT_D2C_NAME      "D2C"
#define BACKUP_OBJECT_BFILE_CLIENT_NAME		"CLIENTBFILE"
#define BACKUP_OBJECT_BFILE_NAME		"BFILE"

#define BACKUP_OBJECT_ORACLE_RECOVER_NAME   "CLIENTORACLERECOVER" /* Oracle Database client recover Object Name */
#define BACKUP_OBJECT_MSSQL_RECOVER_NAME    "CLIENTMSSQLRECOVER"
#define BACKUP_OBJECT_MYSQL_RECOVER_NAME    "CLIENTMYSQLRECOVER"
#define BACKUP_OBJECT_FILE_RECOVER_NAME     "CLIENTFILERECOVER"
#define BACKUP_OBJECT_DMSSQL_RECOVER_NAME   "CLIENTDMSSQLRECOVER"
#define BACKUP_OBJECT_VMWARE_BACKUP_NAME    "VMWARESERVER"
#define BACKUP_OBJECT_P2V_BACKUP_NAME       "P2VSERVER"
#define BACKUP_OBJECT_UNIVERSAL_RECOVER_NAME "CLIENTUNIRECOVER"
#define BACKUP_OBJECT_ORALOG_RECOVER_NAME	"CLIENTORALOGRECOVER"
#define BACKUP_OBJECT_P2V_RECOVER_NAME    "CLIENTP2VRECOVER"

#define BACKUP_OBJECT_P2V_RESTORE_NAME    "CLIENTP2VRESTORE"
#define BACKUP_OBJECT_DB2_RECOVER_NAME "CLIENTDB2RECOVER"
#define BACKUP_OBJECT_BFILE_RESTORE_NAME    "CLIENTBFILERESTORE"
#define BACKUP_OBJECT_BFILE_RECOVER_NAME    "CLIENTBFILERECOVER"

/* MSSQL Backup Type */
#define BACKUP_OBJECT_DB_FILE_GROUP     2           /* file and filegroup Backup */
#define BACKUP_OBJECT_DB_FULL           3           /* Database Full Backup  */
#define BACKUP_OBJECT_DB_LOG            4           /* transaction Log Backup */
#define BACKUP_OBJECT_DB_DIFF           5           /* Differential Backup */

/* SYBASE Backup Type */
#define BACKUP_OBJECT_DB_FULL_SY        3           /* Database Full Backup  */
#define BACKUP_OBJECT_DB_LOG_SY         4           /* transaction Log Backup */

/* Oracle Backup Type */
#define BACKUP_OBJECT_DB_DIFF_0         10          /* Differential level 0 Backup */
#define BACKUP_OBJECT_DB_DIFF_1         12          /* Differential level 1 Backup */
#define BACKUP_OBJECT_DB_DIFF_2         14          /* Differential level 2 Backup */
#define BACKUP_OBJECT_DB_ARCHIVELOG     16          /* Backup archive log */
#define BACKUP_OBJECT_DB_TABLESPACE     18          /* Backup tablespace */
#define BACKUP_OBJECT_DB_CATALOG        20          /* Backup recovery catalog */

/* Operate system backup type */
#define BACKUP_OBJECT_OS_FULL           21          /* Operate system backup full */
#define BACKUP_OBJECT_OS_INCR           22          /* Operate system backup increase */
#define BACKUP_OBJECT_OS_DIFF           23          /* Operate system differential backup */

/* Backup option for file,mssql,oracle */
#define BACKUP_OPTION_ENCRYPT                  (1<<0)
#define BACKUP_OPTION_COMPRESS                 (1<<1)
#define BACKUP_OPTION_FRONT_ENCRYPT            (1<<2)

/****************************************************************************
 * End of Backup Object                                                     *
 ***************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Protocols definition(description) End                                     //
///////////////////////////////////////////////////////////////////////////////

typedef struct _CHAR_NODE {
    char *pszStr;
    long nFlag;
    unsigned long nVersion;
    unsigned long nReserve1;
    unsigned long nReserve2;
    struct _CHAR_NODE *next;
} CharNode;

/* The Client Get task monitor option */
typedef struct _CLIENT_GET_TASK_OPTION {
    int nOption;
    int nTaskType;
    int nObjectType;
    int nTaskStatus;
    char szUser[255];
} GetTaskOption;

/* The user and group information structure */
typedef struct _SHARE_GROUP_INFO_ST {
    unsigned long nId;
    unsigned long nParentId;

    /* user or group name */
    char szName[255];

    /* flag of user or group */
    int nFlag;

    /* flag of access control */
    int nAccess;

    /* the user list of group */
    struct _SHARE_GROUP_INFO_ST *pUserList;
 
    /* next item */
    struct _SHARE_GROUP_INFO_ST *next;
} ShareGroupSt;

/* The self definations: OBJECT TYPE */
#define BACKUP_OBJECT_TYPE_REGISTER    BACKUP_OBJECT_REGISTER
#define BACKUP_OBJECT_TYPE_MSSQL       BACKUP_OBJECT_MSSQL
#define BACKUP_OBJECT_TYPE_ORACLE      BACKUP_OBJECT_ORACLE
#define BACKUP_OBJECT_TYPE_FILE        BACKUP_OBJECT_FILE
#define BACKUP_OBJECT_TYPE_POLICY      BACKUP_OBJECT_POLICY
#define BACKUP_OBJECT_TYPE_SYSTEM      BACKUP_OBJECT_SYSTEM
#define BACKUP_OBJECT_TYPE_SYBASE      BACKUP_OBJECT_SYBASE
#define BACKUP_OBJECT_TYPE_ORA_CLIENT  BACKUP_OBJECT_ORACLE_CLIENT
#define BACKUP_OBJECT_TYPE_ORA_SIGLE   BACKUP_OBJECT_ORASIGLE
#define BACKUP_OBJECT_TYPE_DB2         BACKUP_OBJECT_DB2

//ing 续传,  仅 1 是可续传标志
#define BACKUP_BROKEN_ING	1
#define BACKUP_BROKEN_OK	2
#define BACKUP_BROKEN_ERR	3

//压缩词典重置,记录断点续传位置
//1024000*512Byte=500M
#define COMPRESS_DIC_RESET_NUM  1024000
//压缩分块大小，最好不要改
enum {
    BLOCK_BYTES = 1024 * 64,
};



#endif /* __WAVETOP_BACKUP_PROTOCOL_H_ */
