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
 * The extended message code(��չ��Ϣ��)
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

/** ����Э��汾 **/
#define LICENSE_VERSION_1000          1000

/** ����Э�鷽�� **/
#define LICENSE_PROTO_LOGIN           1  /** ��½ **/
#define LICENSE_PROTO_ADD             2  /** ����License **/
#define LICENSE_PROTO_DEL             3  /** ɾ��License **/
#define LICENSE_PROTO_QUERY           4  /** ��ѯLicense **/
#define LICENSE_PROTO_REGISTER        5  /** ע�������  **/
#define LICENSE_PROTO_CHECK           6  /** У�������  **/
#define LICENSE_MACA_CHECK            7  /** У���û�ID  **/
#define LICENSE_PROTO_SENDREPORT      8
#define LICENSE_PROTO_SENDSCHEDULE    9
#define LICENSE_PROTO_GETSCHEDULE     10

/** Э����� **/
#define LICENSE_OPTIONS_REGISTER      (1<<1) /** ע������� **/
#define LICENSE_OPTIONS_UNREGISTER    (1<<2) /** ע�������� **/
#define LICENSE_OPTIONS_IDENTITY      (1<<3) /** ��ѯLicenseʱ����(����)Identityһ���ȡ **/
#define LICENSE_OPTIONS_LICENSE       (1<<4) /** ��ȡLicense **/
#define LICENSE_OPTIONS_UNREGBYHOST   (1<<5) /** ע����Ӧ�������µ�����identity���е���Ϣ**/
#define LICENSE_OPTIONS_LICEDELALL    (1<<6) /** ɾ������License **/
#define LICENSE_OPTIONS_REGISTERBC    (1<<7) /** ͨ���������ע�� **/

/********************************************************************
 *
 * �����Ʒ���书���������, ÿ����Ʒ����һ�δ�������.
 *
 ********************************************************************/

/** ACDP��Ʒ���� **/
#define LICENSE_PROD_ACDP                   (unsigned short)0x0001  /* ��Ʒ��:ACDP */
#define LICENSE_PROD_DCDP                   (unsigned short)0x0002  /* ��Ʒ��:DCDP */
#define LICENSE_PROD_BACKUP6                (unsigned short)0x0003	/* ��Ʒ��:ʵʱ���� */
#define LICENSE_PROD_MIRROR                 (unsigned short)0x0004  /* ��Ʒ��:����*/

/*ģ�鶨��*/
#define LICENSE_PROD_MOD_SERVER             (unsigned short)0x0001  /* ������ */
#define LICENSE_PROD_MOD_SQLSERVER          (unsigned short)0x0002  /* SQLģ�� */
#define LICENSE_PROD_MOD_ORACLE             (unsigned short)0x0003  /* ORACLEģ�� */
#define LICENSE_PROD_MOD_MYSQL              (unsigned short)0x0004	/* MYSQL ģ�� */
#define LICENSE_PROD_MOD_DB2                (unsigned short)0x0005	/* DB2ģ�� */
#define LICENSE_PROD_MOD_FILE               (unsigned short)0x0006	/* �ļ�ģ�� */
#define LICENSE_PROD_MOD_OS                 (unsigned short)0x0007  /* ����ϵͳģ�� */
#define LICENSE_PROD_MOD_STREAMIO           (unsigned short)0x0008	/* ������-����ģ�� */


/** �ն˱��ݲ�Ʒ���� **/
#define LICENSE_PROD_BACKUP_P               (unsigned short)0x0001  /* רҵ��     */
#define LICENSE_PROD_BACKUP_S               (unsigned short)0x0002  /* ��С��ҵ�� */
#define LICENSE_PROD_BACKUP_P_SERV          (unsigned short)0x0001  /* ������     */
#define LICENSE_PROD_BACKUP_P_FILE          (unsigned short)0x0006  /* �ļ�����   */
//#define LICENSE_PROD_BACKUP_P_SHARE         (unsigned short)0x0004  /* ����       */
//#define LICENSE_PROD_BACKUP_P_MAIL          (unsigned short)0x0010  /* �ʼ�����   */
#define LICENSE_PROD_BACKUP_S_SERV          (unsigned short)0x0001  /* ������     */
#define LICENSE_PROD_BACKUP_S_FILE          (unsigned short)0x0006  /* �ļ�����   */
//#define LICENSE_PROD_BACKUP_S_SHARE         (unsigned short)0x0004  /* ����       */
//#define LICENSE_PROD_BACKUP_S_SCHED         (unsigned short)0x0008  /* ǿ�Ʊ���   */

/** ���������ݲ�Ʒ���� **/
#define LICENSE_PROD_BACKUP2                (unsigned short)0x0001  /* ���������� */
#define LICENSE_PROD_BACKUP2_SERV           (unsigned short)0x0001  /* ������     */
#define LICENSE_PROD_BACKUP2_SQLSERVER      (unsigned short)0x0002  /* SQLServ���� */
#define LICENSE_PROD_BACKUP2_ORACLE         (unsigned short)0x0003  /* Oracle���� */
#define LICENSE_PROD_BACKUP2_FILE           (unsigned short)0x0006  /* �ļ�����   */



/** �����Ʒ���� **/
//#define LICENSE_PROD_MIRROR                 (unsigned short)0x0004    /* ����    */
#define LICENSE_PROD_MIRROR_SERV            (unsigned short)0x0001  /* ������     */
#define LICENSE_PROD_MIRROR_SQLSERVER       (unsigned short)0x0002  /* SQLServ���� */
#define LICENSE_PROD_MIRROR_ORACLE          (unsigned short)0x0003  /* Oracle���� */
#define LICENSE_PROD_MIRROR_FILE            (unsigned short)0x0006  /* �ļ�����   */


/** ��Ʒ������Ķ������ӳ�䵽ʵ�ʴ洢���� **/
#define LICENSE_MAP_DEFINETOREAL(MM) (MM)

/** ��Ʒ�������ʵ�ʴ洢����ӳ�䵽������� **/
#define LICENSE_MAP_REALTODEFINE(mm) (mm)


#endif /* __WAVETOP_LICENSE_DEF_H_ 1 */


