/** =============================================================================
** Copyright (c) 2010 WaveTop Information Corp. All rights reserved.
**
** The Mirror system
**
** =============================================================================
*/

#ifndef _WAVETOP_MIRROR_FS_IO_H_
#define _WAVETOP_MIRROR_FS_IO_H_

#include <windows.h>

#ifndef QWORD
typedef unsigned __int64 QWORD;
#endif

#ifndef LONG
typedef long LONG;
#endif

#ifndef ULONG
typedef unsigned long ULONG;
#endif

#ifndef VOID
typedef void VOID;
#endif

#ifndef ANYSIZE_ARRAY
#define ANYSIZE_ARRAY 1
#endif

#ifndef LODWORD
#define LODWORD(l) ((DWORD)((QWORD)(l)))
#endif

#ifndef HIDWORD
#define HIDWORD(l) ((DWORD)(((QWORD)(l) >> 32 ) & 0xFFFFFFFF))
#endif

#ifndef MAKEQWORD
#define MAKEQWORD(a,b) ((QWORD)(((DWORD)(a)) | \
    (((QWORD)((DWORD)(b))) << 32)))
#endif

/* �ж��ļ�����Ƿ�Ƿ� */
#ifndef IS_VALID_FILE_HANDLE
#define IS_VALID_FILE_HANDLE(h) \
    ((h) && (INVALID_HANDLE_VALUE != (h)))
#endif

#ifndef IS_VALID_FILE_ATTRIBUTES
#define IS_VALID_FILE_ATTRIBUTES(a) \
    ((a) && (INVALID_FILE_ATTRIBUTES != (a)))
#endif

#ifndef	IS_VALID_FILE_SIZE
#define IS_VALID_FILE_SIZE( s ) \
    ((s) && (INVALID_FILE_SIZE != (s)))
#endif

#ifndef	CALC_CLUSTER
#define CALC_CLUSTER(a, b) (((a) + ((b) - 1)) / (b))
#endif

/* ����ṹ��t��Աf��ռ���ֽ��� */
#ifndef	FIELD_SIZE
#define FIELD_SIZE(t, f) (sizeof(((t*)0)->f))
#endif

/* ����ṹ���Աf��Ըýṹ����ʼλ�õ�ƫ���� */
#ifndef	FIELD_OFFSET
#define FIELD_OFFSET(t, f) ((DWORD)(DWORD_PTR)&(((t*)0)->f))
#endif

/* �����ļ�ϵͳ����ö������ */
typedef enum _WT_MIRROR_FS_TYPE
{
    FS_TYPE_MIN        = 0,
    FS_TYPE_UNKNOWN    = FS_TYPE_MIN,
    FS_TYPE_NTFS       = 1,
    FS_TYPE_exFAT      = 2,    
    FS_TYPE_FAT12      = 3,
    FS_TYPE_FAT16      = 4,
    FS_TYPE_FAT32      = 5,
    FS_TYPE_HPFS       = 6,
    FS_TYPE_CDFS       = 7,
    FS_TYPE_MAX        = FS_TYPE_CDFS,
}WT_MIRROR_FS_TYPE;

/* �ļ�ϵͳ����Ϣ */
typedef struct _WT_FS_VOLUME_INFO
{
    HANDLE     hVolume;                    // ����
    LPCTSTR    pszName;                    // ����
    LPTSTR     pszLabel;                   // ���
    LPCTSTR    pszPath;                    // ·��
    LONG       iDriveNumber;               // �̷�(0~25��ʾa~z��-1��ʾ�Ƿ���)
    ULONG      uDriveType;                 // ��������
    DWORD      dwSerialNumber;             // �����к�
    QWORD      qwFirstSectorOffset;        // ��һ������ƫ����
    DWORD      dwSectorPerCluster;         // ÿ���ذ���������
    DWORD      dwBytesPerSector;           // ÿ�����������ֽ���
    DWORD      dwBytesPerCluster;          // ÿ���غ����ֽ���
    DWORD      dwTotalNumberOfClusters;    // ������
    DWORD      dwNumberOfFreeClusters;     // �������
    DWORD      dwNumberOfUsedClusters;     // ���ô���
    DWORD      dwFileSystemType;           // �ļ�ϵͳ����
    DWORD      dwFileSystemFlags;          // �ļ�ϵͳ��־����    
}WT_FS_VOLUME_INFO, *PWT_FS_VOLUME_INFO, *LPWT_FS_VOLUME_INFO;

/* �ļ���Ϣ */
typedef struct _WT_FS_FILE_INFO_
{
    LPCTSTR          pszPath;                // �ļ�·��
    LPCTSTR          pszResolvedPath;        // �ļ�����·��
    HANDLE           hFile;                  // �ļ����
    DWORD            dwFileAttributes;       // �ļ�����
    LARGE_INTEGER    liFileSize;             // �ļ���С    
    LARGE_INTEGER    liFileIndex;            // �ļ�����
}WT_FS_FILE_INFO, *PWT_FS_FILE_INFO, *LPWT_FS_FILE_INFO;

// �ļ���������buff��һ�����������������洢��
typedef struct _WT_FS_FILE_EXTENTS
{
    DWORD dwExtentCount;               //�ļ��洢����������
    struct {
       DWORD dwClusterCount;           // ����
        LARGE_INTEGER    liStartLcn;   //��ʼ�߼��غ�
        } Extents[ ANYSIZE_ARRAY ];
}WT_FS_FILE_EXTENTS, *PWT_FS_FILE_EXTENTS, *LPWT_FS_FILE_EXTENTS;

/* С�ļ�,С��1KB��С�ļ�����MFT�� */
typedef struct _WT_FS_BLOB
{
    DWORD   dwBlobSize;
    BYTE    pbyBlobData[ ANYSIZE_ARRAY ];

}WT_FS_BLOB, *PWT_FS_BLOB, *LPWT_FS_BLOB;

typedef struct _FAT_BPB
{
    WORD     wBytsPerSec;
    BYTE     bySecPerClus;
    WORD     wRsvdSecCnt;
    BYTE     byNumFATs;
    WORD     wRootEntCnt; 
    WORD     wTotSec16;
    BYTE     byMedia;
    WORD     wFATSz16;
    WORD     wSecPerTrk;
    WORD     wNumHeads;
    DWORD    dwHiddSec;
    DWORD    dwTotSec32;

}FAT_BPB, *PFAT_BPB, *LPFAT_BPB;

typedef struct _FAT_EBPB
{
    BYTE     byDrvNum; 
    BYTE     byReserved1; 
    BYTE     byBootSig;
    DWORD    dwVolID;
    BYTE     pbyVolLab[11];
    BYTE     pbyFilSysType[8];

}FAT_EBPB, *PFAT_EBPB, *LPFAT_EBPB;

typedef struct _FAT_EBPB32
{
    DWORD    dwFATSz32;
    WORD     wExtFlags;
    WORD     wFSVer;
    DWORD    dwRootClus;
    WORD     wFSInfo;
    WORD     wBkBootSec;
    BYTE     pbyReserved[12];
    FAT_EBPB ebpb;
}FAT_EBPB32, *PFAT_EBPB32, *LPFAT_EBPB32;

typedef struct _FAT_LBR
{
    BYTE       pbyJmpBoot[3];
    BYTE       pbyOEMName[8];
    FAT_BPB    bpb;
    union {
        FAT_EBPB     ebpb16; 
        FAT_EBPB32     ebpb32; 
    };

    BYTE      pbyBootCode[420];
    WORD      wTrailSig;
}FAT_LBR, *PFAT_LBR, *LPFAT_LBR;

typedef enum
{
    NtfsAttributeStandardInformation     = 0x10,
    NtfsAttributeAttributeList           = 0x20,
    NtfsAttributeFileName                = 0x30,
    NtfsAttributeObjectId                = 0x40,
    NtfsAttributeSecurityDescriptor      = 0x50,
    NtfsAttributeVolumeName              = 0x60,
    NtfsAttributeVolumeInformation       = 0x70,
    NtfsAttributeData                    = 0x80,
    NtfsAttributeIndexRoot               = 0x90,
    NtfsAttributeIndexAllocation         = 0xA0,
    NtfsAttributeBitmap                  = 0xB0,
    NtfsAttributeReparsePoint            = 0xC0,
    NtfsAttributeEAInformation           = 0xD0,
    NtfsAttributeEA                      = 0xE0,
    NtfsAttributePropertySet             = 0xF0,
    NtfsAttributeLoggedUtilityStream     = 0x100,
} NTFS_ATTRIBUTE_TYPE;

typedef struct _NTFS_RECORD_HEADER
{
    DWORD   dwType;
    WORD    wUsaOffset;
    WORD    wUsaCount;
    QWORD   qwLsn;

} NTFS_RECORD_HEADER, *PNTFS_RECORD_HEADER, *LPNTFS_RECORD_HEADER;

typedef struct _NTFS_FILE_RECORD_HEADER
{
    NTFS_RECORD_HEADER    ntfsRH;
    WORD                  wSequenceNumber; 
    WORD                  wLinkCount; 
    WORD                  wAttributeOffset;        
    WORD                  wFlags;                  
    DWORD                 dwBytesInUse;            
    DWORD                 dwBytesAllocated; 
    QWORD                 qwBaseFileRecord; 
    WORD                  wNextAttributeNumber;  
    WORD                  wPading;
    DWORD                 dwMFTRecordNumber; 
    WORD                  wUpdateSeqNum;
}NTFS_FILE_RECORD_HEADER, *PNTFS_FILE_RECORD_HEADER, *LPNTFS_FILE_RECORD_HEADER;

typedef struct _NTFS_ATTRIBUTE
{
    NTFS_ATTRIBUTE_TYPE eNtfsAttribType;
    DWORD               dwLength;
    BYTE                byNonresident;
    BYTE                byNameLength;
    WORD                wNameOffset;
    WORD                wFlags;
    WORD                wAttributeNumber;
} NTFS_ATTRIBUTE, *PNTFS_ATTRIBUTE, *LPNTFS_ATTRIBUTE;

typedef struct _NTFS_RESIDENT_ATTRIBUTE
{
    NTFS_ATTRIBUTE  ntfsAttribute;
    DWORD           dwValueLength;
    WORD            wValueOffset;
    BYTE            byFlags;

} NTFS_RESIDENT_ATTRIBUTE, *PNTFS_RESIDENT_ATTRIBUTE, *LPNTFS_RESIDENT_ATTRIBUTE;

typedef struct _WT_FS_DATA_
{
    DWORD dwExtentCount;
    DWORD dwClusterCount;
    char szFileName[1024];
    PVOID pData; 
    WT_FS_FILE_INFO   fsFileInfo;
    WT_FS_VOLUME_INFO fsVolInfo;
    PWT_FS_FILE_EXTENTS pFsFileExtents;
    PWT_FS_BLOB MFTBlob;
}WT_FS_DATA;

/* ���ļ�
 * Ӧ����ʹ��WTFsOpenFile�����ļ�
 * Ȼ��ʹ��WTFsReadFile��ȡ����
 * �����Ҫ����WTFsClose���ر��ļ��ͷ���Դ
 */
BOOL WTFsOpenFile(LPCTSTR pszSrcFileName);

/* 
 * ���ļ� 
 * @[in]
 * pszSrcFileName - ��Ҫ��ȡ�ļ����������·��
 * lpPhyData - ��ȡ������������
 * nByte - ��ȡ�����������ݳ���
 * bFlag - �Ƿ�Ϊ���һ�� (0 - �������һ��, 1 - �����һ��)
 * @[out]
 * �ɹ����� true, ���򷵻�ϵͳ�����.
 */
BOOL WTFsReadFile(LPCTSTR pszSrcFileName,
    LPVOID *lpPhyData,
    ULONG *nByte,
    BOOL *bFlag);
    
/* �ر��ļ����ͷ�������Ŀռ� */
VOID WTFsClose(LPCTSTR pszSrcFileName);

/* ����FAT�ļ� */
BOOL WTFsAnalyzeFAT(PWT_FS_VOLUME_INFO pFsVolInfo);

/* ��ȡ����Ϣ */
BOOL WTFsGetVolumeInfo(LPCTSTR pszPath, PWT_FS_VOLUME_INFO pFsVolInfo);

/* �ͷž���Ϣ */
VOID WTFsFreeVolumeInfo(PWT_FS_VOLUME_INFO pFsVolInfo);

/* ��ȡ�ļ���Ϣ */
BOOL WTFsGetFileInfo(LPCTSTR pszPath, PWT_FS_FILE_INFO pFsFileInfo);

/* �ͷ��ļ���Ϣ */
VOID WTFsFreeFileInfo(PWT_FS_FILE_INFO pFsFileInfo);

/* ��ȡ�ļ������ */
BOOL WTFsGetFileExtents(PWT_FS_VOLUME_INFO pFsVolInfo, 
    PWT_FS_FILE_INFO pFsFileInfo,
    PWT_FS_FILE_EXTENTS* ppFsFileExtents);

/* ��MFT */
BOOL WTFsReadFileMFT(PWT_FS_VOLUME_INFO pFsVolInfo,
    PWT_FS_FILE_INFO pFsFileInfo,
    PWT_FS_BLOB *ppFsFileData);

#endif // _WAVETOP_MIRROR_FS_IO_H_