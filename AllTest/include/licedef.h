/** =============================================================================
 ** Copyright (c) 2007 WaveTop Information Corp. All rights reserved.
 **
 ** The License control system
 **
 ** =============================================================================
 */

#ifndef __WAVETOP_LICENSE_DEF_H_
#define __WAVETOP_LICENSE_DEF_H_ 1

#include "license.h"

/********************************************************************
 *
 * The license error code (defined in license.h)
 ** Okay **
 * #define LICENSE_OK                  0
 *
 ** Invalid syntax or wrong arguments 
 * #define LICENSE_INVALID_SYNTAX      1
 *
 ** The license has beed destoryed **
 * #define LICENSE_MALFOMED            2
 *
 ** No license in DB **
 * #define LICENSE_NO_LICENSE          3
 *
 ** Out of memory **
 * #define LICENSE_NO_MEMORY           4
 *
 *
 ********************************************************************
 * The extended message code(扩展消息码)
 ********************************************************************/

/** Over the quota of license **/
#define LICENSE_OVER_QUOTA          5

/** Invalid syntax or wrong arguments **/
#define LICENSE_BAD_REQUEST         LICENSE_INVALID_SYNTAX

/** Connection Down **/
#define LICENSE_CONNECTION_DOWN     10

/** No support **/
#define LICENSE_NOT_IMPLEMENTED     11

/** Open file failed **/
#define LICENSE_OPEN_FILE_ERROR     12

/** Not registered **/
#define LICENSE_NO_REGISTERED       13

/** The license of the prodtype has eixst **/
#define LICENSE_HAVE_EXIST          14

/********************************************************************
 * End of message code
 ********************************************************************/

/** 定义协议版本 **/
#define LICENSE_VERSION_1000          1000

/** 定义协议方法 **/
#define LICENSE_PROTO_LOGIN           1  /** 登陆 **/
#define LICENSE_PROTO_ADD             2  /** 增加License **/
#define LICENSE_PROTO_DEL             3  /** 删除License **/
#define LICENSE_PROTO_QUERY           4  /** 查询License **/
#define LICENSE_PROTO_REGISTER        5  /** 注册该主机  **/
#define LICENSE_PROTO_CHECK           6  /** 校验该主机  **/
#define LICENSE_MACA_CHECK            7  /** 校验用户ID  **/
#define LICENSE_PROTO_SENDREPORT      8
#define LICENSE_PROTO_SENDSCHEDULE    9
#define LICENSE_PROTO_GETSCHEDULE     10

/** 协议参数 **/
#define LICENSE_OPTIONS_REGISTER      (1<<1) /** 注册该主机 **/
#define LICENSE_OPTIONS_UNREGISTER    (1<<2) /** 注销该主机 **/
#define LICENSE_OPTIONS_IDENTITY      (1<<3) /** 查询License时连带(主机)Identity一起获取 **/
#define LICENSE_OPTIONS_LICENSE       (1<<4) /** 获取License **/
#define LICENSE_OPTIONS_UNREGBYHOST   (1<<5) /** 注销相应主机名下的所有identity表中的信息**/
#define LICENSE_OPTIONS_LICEDELALL    (1<<6) /** 删除所有License **/
#define LICENSE_OPTIONS_REGISTERBC    (1<<7) /** 通过界面进行注册 **/

/********************************************************************
 *
 * 定义产品和其功能组件代码, 每个产品分配一段代码区域.
 *
 ********************************************************************/

/** ACDP产品定义 **/
#define LICENSE_PROD_ACDP                   (unsigned short)0x0001  /* 产品名:ACDP */
#define LICENSE_PROD_DCDP                   (unsigned short)0x0002  /* 产品名:DCDP */
#define LICENSE_PROD_BACKUP6                (unsigned short)0x0003	/* 产品名:实时备份 */
#define LICENSE_PROD_MIRROR                 (unsigned short)0x0004  /* 产品名:镜像*/

/*模块定义*/
#define LICENSE_PROD_MOD_SERVER             (unsigned short)0x0001  /* 服务器 */
#define LICENSE_PROD_MOD_SQLSERVER          (unsigned short)0x0002  /* SQL模块 */
#define LICENSE_PROD_MOD_ORACLE             (unsigned short)0x0003  /* ORACLE模块 */
#define LICENSE_PROD_MOD_MYSQL              (unsigned short)0x0004	/* MYSQL 模块 */
#define LICENSE_PROD_MOD_DB2                (unsigned short)0x0005	/* DB2模块 */
#define LICENSE_PROD_MOD_FILE               (unsigned short)0x0006	/* 文件模块 */
#define LICENSE_PROD_MOD_OS                 (unsigned short)0x0007  /* 操作系统模块 */
#define LICENSE_PROD_MOD_STREAMIO           (unsigned short)0x0008	/* 数据流-万能模块 */


/** 终端备份产品定义 **/
#define LICENSE_PROD_BACKUP_P               (unsigned short)0x0001  /* 专业版     */
#define LICENSE_PROD_BACKUP_S               (unsigned short)0x0002  /* 中小企业版 */
#define LICENSE_PROD_BACKUP_P_SERV          (unsigned short)0x0001  /* 服务器     */
#define LICENSE_PROD_BACKUP_P_FILE          (unsigned short)0x0006  /* 文件备份   */
//#define LICENSE_PROD_BACKUP_P_SHARE         (unsigned short)0x0004  /* 共享       */
//#define LICENSE_PROD_BACKUP_P_MAIL          (unsigned short)0x0010  /* 邮件备份   */
#define LICENSE_PROD_BACKUP_S_SERV          (unsigned short)0x0001  /* 服务器     */
#define LICENSE_PROD_BACKUP_S_FILE          (unsigned short)0x0006  /* 文件备份   */
//#define LICENSE_PROD_BACKUP_S_SHARE         (unsigned short)0x0004  /* 共享       */
//#define LICENSE_PROD_BACKUP_S_SCHED         (unsigned short)0x0008  /* 强制备份   */

/** 服务器备份产品定义 **/
#define LICENSE_PROD_BACKUP2                (unsigned short)0x0001  /* 服务器备份 */
#define LICENSE_PROD_BACKUP2_SERV           (unsigned short)0x0001  /* 服务器     */
#define LICENSE_PROD_BACKUP2_SQLSERVER      (unsigned short)0x0002  /* SQLServ备份 */
#define LICENSE_PROD_BACKUP2_ORACLE         (unsigned short)0x0003  /* Oracle备份 */
#define LICENSE_PROD_BACKUP2_FILE           (unsigned short)0x0006  /* 文件备份   */



/** 镜像产品定义 **/
//#define LICENSE_PROD_MIRROR                 (unsigned short)0x0004    /* 镜像    */
#define LICENSE_PROD_MIRROR_SERV            (unsigned short)0x0001  /* 服务器     */
#define LICENSE_PROD_MIRROR_SQLSERVER       (unsigned short)0x0002  /* SQLServ镜像 */
#define LICENSE_PROD_MIRROR_ORACLE          (unsigned short)0x0003  /* Oracle镜像 */
#define LICENSE_PROD_MIRROR_FILE            (unsigned short)0x0006  /* 文件镜像   */


/** 产品或组件的定义代码映射到实际存储代码 **/
#define LICENSE_MAP_DEFINETOREAL(MM) (MM)

/** 产品或组件的实际存储代码映射到定义代码 **/
#define LICENSE_MAP_REALTODEFINE(mm) (mm)


#endif /* __WAVETOP_LICENSE_DEF_H_ 1 */


