/** =============================================================================
 ** Copyright (c) 2003 WaveTop Information Corp. All rights reserved.
 **
 ** The backup system
 **
 ** =============================================================================
 */

/** 
 * File:        taskctrl/wca.h
 * Description: Certificate Authority ��֤��Ȩ
 */

#ifndef __WAVETOP_CA_H_
#define __WAVETOP_CA_H_

/* ֤����ܷ�ʽ */
#define CA_SIGN_MD2                1    // MD2���ܷ�ʽ
#define CA_SIGN_MD5                2    // MD5���ܷ�ʽ
#define CA_SIGN_SHA1               3    // SHA1���ܷ�ʽ

/* ������֤��������ʽ */
#define CA_VERSIGN_NO_BLACKLIST    0    // ��֤֤����Ч��ʱ�����CA�ĺ�������
#define CA_VERSIGN_BLACKLIST       1    // ��֤֤����Ч��ʱ���CA�ĺ�������

#define CA_HIDE_USER_INFODLG       0    // ����ʾ�û�֤��Ի���
#define CA_SHOW_USER_INFODLG       1    // ��ʾ�û�֤��Ի���

/* ���û�ȡ֤����豸���� */
#define CA_GET_CERT_FILE           2    // ֤���ȡΪ�ļ�
#define CA_GET_CERT_USB            9    // ֤���ȡΪUSB�� 

/* ���õ�¼��ʽ */
#define CA_LOGIN_ONCE              1    // �����¼

/* ��ȡ�û���Ϣ */
#define CA_GET_CERT_VERSION        1    // ֤��汾
#define CA_GET_CERT_ID             2    // ֤���ʶ��
#define CA_GET_USER_NAME           17   // �û���

/* ����USB������ */
#define CA_LOGIN_IN_SPAN           "CertChain.spc"    // ����
#define CA_LOGIN_OUT_SPAN          "gwwCertChain.spc"    // ������

typedef unsigned long WTHWIND;
typedef void *WTHCERT;

/* �����ʼ������ */
typedef struct _INITIAL_SHECA_SESSION {
    /* ����·�� */
    char *pszWorkDir;
    
    /* �û�֤���ȡ���� */
    unsigned long nUserCertdevicetype;

    /* �û�֤���ļ�·�� */
    char *pszUserCertparameter;

    /* �û�֤������ */
    char *pszUserCertpassword;

    /* �û���Կ��ȡ���� */
    unsigned long nUserkeydevicetype;
    
    /* �û���Կ�ļ�·�� */
    char *pszUserkeydeviceparameter;

    /* �û���Կ���� */
    char *pszUserkeypassword;

    /* ˽Կ��ʱʱ�� */
    unsigned long nPrivatekeytimeout;

    /* ��֤���ȡ���� */
    unsigned long nRootcertdevicetype;

    /* ��֤��·�� */
    char *pszRootcertdeviceparameter;
    
    /* ��֤������ */
    char *pszRootcertpassword;

    /* �洢֤�������豸���� */
    unsigned long nCertchaindevicetype;
    
    /* �洢֤�����豸�Ĳ��� */
    char *pszCertchaindeviceparameter;
    
    /* ֤�������� */
    char *pszCertchainpassword;

    void *pReserve;

    struct _INITIAL_SHECA_SESSION *next;
} InitialCaSession;

/* Initialize the sheca library.
 * @[in]
 * pSession - the initialize condition.
 * pCert - the reserve argument.
 * nMethod - if want show user info set it CA_SHOW_USER_INFODLG.
 * other set it CA_HIDE_USER_INFODLG.
 *
 * @[out]
 * return WAVETOP_BACKUP_OK, if successful, and return this pCent 
 * handle. otherwise, return error code.
 */
typedef int (*WTCAInit) (WTHCERT *pCert, 
                        InitialCaSession *pSession, int nMethod);

                        /* Initialize login .
                        * @[in]
                        * pCert - the reserve argument.
                        * nServerCertDevType - server cert deviece type.
                        * pszServerCertDevParam - server device parameter,if not need, input "".
                        * pszServerCertPassword- server cert password.
                        *
                        * @[out]
                        * return WAVETOP_BACKUP_OK, if successful, and return this pCent 
                        * handle. otherwise, return error code.
*/
typedef int (*WTCALoginInit) (WTHCERT *pCert, 
                              unsigned long nServerCertDevType,
                              char *pszServerCertDevParam, 
                              char *pszServerCertPassword);

/* Get the user Cert.
 * @[in]
 * WTHWIND - the Wnd handle.
 * pCert - the reserve argument.
 * pUserCertFile - the Cert file name.
 * pPassword - the Cert Password if no password set it "".
 * pCertBuf - Receive the Cert Content.
 * pCertlength - the Cert Content Lenth.
 *
 * @[out]
 * return WAVETOP_BACKUP_OK, if successful, and return this pCent 
 * handle pCertBuf and pCertlength. otherwise, return error code.
 */
typedef int (*WTCAGetCert) (WTHWIND, 
                        WTHCERT *pCert, 
                        unsigned long nGetMethod,
                        char *pszUserCertFile, 
                        char *pszPassword, 
                        char *pszCertBuf, 
                        int nCertBufLen,
                        unsigned long *pCertlength);

/* SHECA Sign.
 * @[in]
 * pCert - the reserve argument.
 * pszText - the data to sign.
 * nTextLen - the lenth of pazText.
 * signmethod - sign method .1: MD2   2: MD5   3: SHA1
 * pszSign - the signed data,
 * pnSignLen - the signed data lenth.
 *
 * @[out]
 * return WAVETOP_BACKUP_OK, if successful, and return this pCent 
 * handle, pszSign and pnSignLen. otherwise, return error code.
 */
typedef int (*WTCASign) (WTHCERT pCert, 
                        const char *pszText, 
                        int nTextLen, 
                        unsigned long signmethod, 
                        char *pszSign, 
                        unsigned long *pnSignLen);

/* SHECA Sign Verify.
 * @[in]
 * pCert - the reserve argument.
 * pszSourData - the data to sign.
 * nSourDataLen - the lenth of pazText.
 * pszSign - the signed data.
 * nSignLen - the signed data lenth.
 * nSignmethod - sign method .1: MD2   2: MD5   3: SHA1
 * pszCertContent - the Cert Content.
 * nCertContentLen - the Cert Content lenth.
 * pnResult - the Result.
 *
 * @[out]
 * return WAVETOP_BACKUP_OK, if successful. 
 * otherwise, return error code.
 */
typedef int (*WTCAVerifySign) (WTHCERT pCert, 
                        char *pszSourData, 
                        unsigned long nSourDataLen, 
                        char *pszSign, 
                        unsigned long nSignLen, 
                        unsigned long nSignmethod, 
                        char *pszCertContent, 
                        unsigned long nCertContentLen, 
                        unsigned long *pnResult);


/* Set the ACrlVerify method.
 * @[in]
 * pCert - the reserve argument.
 * nMode - Point to a server address.
 *
 * @[out]
 * return WAVETOP_BACKUP_OK, if successful. otherwise, return error code.
 */
typedef int (*WTCASetConfiguration) (WTHCERT *pCert, int nMode);

/* Crl Verify.
 * @[in]
 * pCert - the reserve argument.
 * pszServer - Point to a server address.
 * nPort - Point to connect port.
 * pszCertContent - the Cert Content.
 * nCertContentLen - the Cert Content lenth.
 * nReserve - the result.
 *
 * @[out]
 * return WAVETOP_BACKUP_OK, if successful. otherwise, return error code.
 */
typedef int (*WTCACrlVerify) (WTHCERT pCert, 
                        const char *pszServer, 
                        int nPort,  
                        char *pszCertContent,  
                        unsigned long nCertContentLen,
                        unsigned long *nReserve);

/* conversion the Cert Content to Base64 code.
 * @[in]
 * pCert - the reserve argument.
 * pszBase64 - the buf to receive the base64 code.
 * nBufsize - the buf lenth.
 *
 * @[out]
 * return WAVETOP_BACKUP_OK, if successful. otherwise, return error code.
 */
typedef int (*WTCAGetCertBase64) (WTHCERT pCert, 
                        char *pszBase64, 
                        int nBufsize,
                        unsigned long *pnBytes, 
                        unsigned long *pnCertCount);

/* conversion the Base64 code to Cert Content.
 * @[in]
 * pCert - the reserve argument.
 * pszBase64 - the buf to receive the base64 code.
 * nBufsize - the buf lenth.
 *
 * @[out]
 * return WAVETOP_BACKUP_OK, if successful. otherwise, return error code.
 */
typedef int (*WTCAConvertBase64ToCert) (WTHCERT *pCert, 
                        char *pszBase64, 
                        int nBufsize,
                        unsigned long *pnBytes, 
                        unsigned long *pnCertCount);

/* Login to the server.
 * @[in]
 * pCert - the reserve argument.
 * pszUser - the login name.
 * pszPassword - the login password.
 * nMethod - the login method.
 * pRand - the rand digital
 *
 * @[out]
 * return WAVETOP_BACKUP_OK, if successful. otherwise, return error code.
 */

typedef int (*WTCALogin) (WTHCERT *pCert, 
                          char *pszUser, 
                          char *pszPassword,
                          char *pszRandBuf,
                          int nRandLen,
                          int nMethod);
 


typedef int (*WTCTLoginVerify) (WTHCERT *pCert,
                                char *pszCurDir,
                                char *pszUserCert,
                                char *pszUser,
                                char *pszPassword,
                                char *pszRand,
                                int nMethod);

typedef int (*WTCTGetUserInfo) (WTHCERT *pCert,
                                char *pszUserCert,
                                char *pszBufInfo,
                                int nBufInfoLen,
                                int nMethod);

typedef int (*WTCTGetUserUUID) (WTHCERT *pCert, 
                                char *pszUserCert, 
                                char *pszBufInfo, 
                                int nBufInfoLen); 

/* Clean the Initialize session.
 * @[out]
 * return WAVETOP_BACKUP_OK, if successful. otherwise, return error code.
 */
typedef int (*WTCAClose) (WTHCERT pCert);

typedef struct _WAVETOP_CA_MODULE {
    unsigned long nVersion;
    unsigned long hModule;
    WTCAInit fInit;
    WTCALoginInit fLoginInit;
    WTCAGetCert fGetCert;
    WTCASign fSign;  
    WTCAVerifySign fVerifySign;
    WTCACrlVerify fAclVerify;
    WTCASetConfiguration fSetConfiguration;
    WTCAGetCertBase64 fGetCertBase64;
    WTCAConvertBase64ToCert fConvertBase64ToCert;
    WTCTGetUserInfo fGetUserInfo;
    WTCTGetUserUUID fGetUserUUID;
    WTCALogin fLogin;
    WTCTLoginVerify fLoginVerify;
    WTCAClose fClose;
    struct _WAVETOP_CA_MODULE *pNext;
} WavetopCAModule;

#endif // #ifndef __WAVETOP_CA_H_