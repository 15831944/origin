/** =============================================================================
 ** Copyright (c) 2004 WaveTop Information Corp. All rights reserved.
 **
 ** The License control system
 **
 ** =============================================================================
 */

#ifndef __WAVETOP_LICENSE_H_
#define __WAVETOP_LICENSE_H_ 1

/* The license version */
#define LICENSE_VERSION    0x2

/* The license type */
#define LICENSE_TYPE_TRIAL     1
#define LICENSE_TYPE_NORMAL    2

/* The license error code */
#define LICENSE_OK                  0
#define LICENSE_INVALID_SYNTAX      1
#define LICENSE_MALFOMED            2
#define LICENSE_NO_LICENSE          3
#define LICENSE_NO_MEMORY           4          

/* the prodtype */
#define LICENSE_LICE_PROD_ACDP                          0x0001  /* ��Ʒ����ACDP */
#define LICENSE_LICE_PROD_DCDP                          0x0002  /* ��Ʒ����DCDP */
#define LICENSE_LICE__PROD_BACKUP6                      0x0003  /* ��Ʒ����ʵʱ���� */
#define LICENSE_LICE_PROD_MIRROR                        0x0004  /* ���� */
#define LICENSE_LICE_PROD_MOD_FILE                      0x0006  /* �ļ� �ͻ���ģ�� */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _LICENSE_TRIAL_ST_2 {
    short nDays;
    short nQuota;
} LicenseTrialSt2;

typedef long LicenseOfClients;


/**
 * ����license��Ϣ���ڲ��ṹ
 * �ɴ˽ṹ����license����
 * ��license������LicenseInfoSt֮��ת��
 *
 * ������룺�ڲ�ʵ�ּ��ܴ���
 **/
typedef struct _LICENSE_INFO {
    /* ��Ʒ���ͼ��������ϣ�ǰ4λΪ��Ʒ���ͣ�����λΪLicense���� */
    unsigned char nProdType;
    /* ģ�������� */
    unsigned short nCount;
    /* storesize */
    unsigned int nStoreSize;
    /* ģ��������������ÿ��ģ��ռλ12 */
    unsigned char nModInfo[9];
    
    unsigned long nChecksum;
       
} LicenseInfo;


/**
 * ����License��Ϣ�ⲿ�ṹ��
 * ��Ʒ��������붨����licedef.h�ļ��С�
 * 
 * ��������Ҫʹ��LICENSE_MAP_DEFINETOREAL��LICENSE_MAP_REALTODEFINE��
 * �ڶ�������ʵ�ʴ洢����֮�����ת����
 *
 * �������: ��۱��ִ��롣
 **/
typedef struct _LICENSE_INFO_ST {
    /* ��Ʒ���� */
    unsigned short nProdType;
    /* License�汾���� */
    unsigned short nLicenseType;
    /* ģ������ */
    unsigned short nModType;
    /* ��Ӧģ���������� */
    unsigned short nModNum;
    /* ģ�������� */
    unsigned short nCount;
    /* storesize */
    unsigned int nStoreSize;

    struct _LICENSE_INFO_ST *pNext;
}LicenseInfoSt;

/* The security functions in the security.c. The others, etc.
 * Encrypt the string.
 * @[in]
 * pszIn is the plain string.
 * nInsize: the plain string size(bytes).
 * @[out]
 * pszOut the secure string.
 * nOutsize: the secure string size(bytes).
 * On success, return 0. On error, not 0.
 */
int LicenseEncrypt(const char *pszIn, int nInsize, char *pszOut, int nOutsize,
                long *nOutbytes);
int LicenseDecrypt(const char *pszIn, int nInsize, char *pszOut, int nOutsize,
                long *nOutbytes);

/*
 * LicenseBase64Decode
 *
 * This routine encodes the data pointed to by the "pszIn" parameter using the
 * base64 algorithm, and returns status code.  If the "nInsize" parameter is 
 * not zero, it specifies the length of the source data.  If the "pszOut" parameter
 * is not null, it is assumed to point to a buffer of sufficient size (which may be
 * calculated: ((srclen + 2)/3)*4) into which the encoded data is placed 
 * (without any termination).  
 *
 * The nOutbytes is reserved.
 */
int LicenseBase64Encode(const char *pszIn, int nInsize, char *pszOut, int nOutsize,
                long *nOutbytes);

/*
 * LicenseBase64Decode
 *
 * This routine decodes the data pointed to by the "pszIn" parameter using
 * the base64 algorithm, and returns status code. The source may either 
 * include or exclude any trailing '=' characters. If the "nInsize" parameter
 * is not zero, it specifies the length of the source data. If the "pszOut" 
 * parameter is not null, it is assumed to point to a buffer of sufficient 
 * size (which may be calculated: (srclen * 3)/4 when srclen includes the 
 * '=' characters) into which the decoded data is placed (without any termination). 
 *
 * The nOutbytes is reserved.
 */

int LicenseBase64Decode(const char *pszIn, int nInsize, char *pszOut, int nOutsize,
                long *nOutbytes);

/* The nOutsize is not less than 16. */
int LicenseSecuDigest(const char *pszIn, int nInsize, char *pszOut, int nOutsize,
                long *nOutbytes);

/* The license structure functions in the license.c.
 * Convert a license string to a license structure.
 * @[in]
 * pszLicense is the license string.
 * @[out]
 * pLicense: the license structure 
 * On success, return 0. On error, not 0.
 */
int LicenseStrToLicense(const char *pszLicense, LicenseInfoSt *pLicense);

/* ��ȡָ��license��ָ����Ʒ��ģ�����Ϣ
 * @[in]
 * nProdtype is the Product type
 * nModtype is the module type
 * pszLicense is the license
 * @[out]
 * plicense - the module infomation of the prodtype and modtype��if the the prodtype is not
 *            the mirror and modtype is file, calculate the file ofclients.others find the module
 *            information from the license.
 *            
 * On success, return 0. On error, not 0.
 */
int LicenseGetModInfo(int nProdtype, int nModtype, char *pszLicense, LicenseInfoSt **plicense);

/* Convert a license structure to a license string.
 * @[in]
 * pLicense is a license structure.
 * pszLicense is the license string buffer.
 * nBufsize is the length of the license string buffer.
 * @[out]
 * pszLicense is the license string buffer.
 * pSize the length of the license string.
 * On success, return 0. On error, not 0.
 */
int LicenseToStr(const LicenseInfoSt *pLicense, char *pszLicense, 
                int nBufsize, long *pSize);

/* Load the license from a file.
 * @[in] 
 * pszFilename is the license file name.
 * @[out]
 * pLicense: the license structure.
 * pCount is the number of the licenses.
 * On success, return 0. On error, not 0.
 */
int LicenseLoad(const char *pszFilename, LicenseInfoSt **pLicense, long *pCount);

/* Create a license from a license information structure.
 * @[in]
 * pOrig is the license information. The nChecksum and szReserve are reserve.
 * @[out]
 * pLicense: the license created by core.
 * On success, return 0. On error, not 0.
 */
int LicenseCreate(const LicenseInfoSt *pOrig, LicenseInfo *pLicense);


/* Verify a license string.
 * @[in]
 * pszLicense is the license string.
 * @[out]
 * On success, return 0. On error, not 0.
 */
int LicenseVerifyFromStr(const char *pszLicense);

/* The store functions in the store.c.
 * Store a license structure into the license db.
 * @[in]
 * pLicense is the license structure.
 * nOverrite: 1 is overrite; 0 is insert.
 * @[out]
 * On success, return 0. On error, not 0.
 */
int LicenseStoreSave(const char *pszStoreFile,
                LicenseInfoSt *pLicense, int nOverrite);

/* the store function of license information
 * store a license structure into the license.db.
 * @[in]
 * pszStoreFile - the path of the store
 * pLicense - the structure of license 
 * overriter - 1 is overriter ,0 is insert.
 * [out]
 * On success, return 0. On error, not 0.
 */
int LicenseStoreProd(const char *pszStoreFile,
                LicenseInfoSt *pLicense, int overrite);

/* the store function of license information
 * store a Mac into the license.db.
 * @[in]
 * pszStoreFile - the path of the store
 * pszMac - the mac value
 * pszMacSerial - the serial of the mac 
 * overriter - 1 is overriter ,0 is insert.
 * [out]
 * On success, return 0. On error, not 0.
 */
int LicenseStoreMacSer(const char *pszStoreFile, const char *pszMac, 
                       const char *pszMacSerial, int overrite);


/* Accumulate the license, and create a new license structure.
 * The same license type has been only allowed to accumulate.
 * @[in]
 * pszLicense is the license string.
 * @[out]
 * pLicense: the license structure 
 * On success, return 0. On error, not 0.
 */
int LicenseStoreAccumulate(const char *pszStoreFile, 
                const char *pszLicense, LicenseInfoSt *pLicense);


/* Encode the passwd.
 * @[in]
 * const char *pszEncoded: the passwd.
 * char *pszOut: the encoded buffer.
 * int nOutSize: the encoded buffer size.
 * @[out]
 * long *pnOutBytes: the out bytes.
 * On success, return 0. On error, not 0.
 */
int LicenseEncodePasswd(const char *pszPasswd, char *pszOut, int nOutSize,
                        long *pnOutBytes);

/* Decode the passwd.
 * @[in]
 * const char *pszDecoded: the passwd.
 * char *pszOut: the decoded buffer.
 * int nOutSize: the decoded buffer size.
 * @[out]
 * long *pnOutBytes: the out bytes.
 * On success, return 0. On error, not 0.
 */
int LicenseDecodePasswd(const char *pszEncoded, char *pszOut, int nOutSize,
                        long *pnOutBytes);

/* Free the license memory pool */
void LicenseFree(void *pMem);


#ifdef __cplusplus
}
#endif

#endif // __WAVETOP_LICENSE_H_

