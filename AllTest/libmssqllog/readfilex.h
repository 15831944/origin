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

/* 判断文件句柄是否非法 */
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

/* 计算结构体t成员f的占用字节数 */
#ifndef	FIELD_SIZE
#define FIELD_SIZE(t, f) (sizeof(((t*)0)->f))
#endif

/* 计算结构体成员f相对该结构体起始位置的偏移量 */
#ifndef	FIELD_OFFSET
#define FIELD_OFFSET(t, f) ((DWORD)(DWORD_PTR)&(((t*)0)->f))
#endif

/* 定义文件系统类型枚举类型 */
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

/* 文件系统卷信息 */
typedef struct _WT_FS_VOLUME_INFO
{
    HANDLE     hVolume;                    // 卷句柄
    LPCTSTR    pszName;                    // 卷名
    LPTSTR     pszLabel;                   // 卷标
    LPCTSTR    pszPath;                    // 路径
    LONG       iDriveNumber;               // 盘符(0~25表示a~z，-1表示非法的)
    ULONG      uDriveType;                 // 磁盘类型
    DWORD      dwSerialNumber;             // 卷序列号
    QWORD      qwFirstSectorOffset;        // 第一块扇区偏移量
    DWORD      dwSectorPerCluster;         // 每个簇包含扇区数
    DWORD      dwBytesPerSector;           // 每个扇区含的字节数
    DWORD      dwBytesPerCluster;          // 每个簇含的字节数
    DWORD      dwTotalNumberOfClusters;    // 簇总数
    DWORD      dwNumberOfFreeClusters;     // 空余簇数
    DWORD      dwNumberOfUsedClusters;     // 已用簇数
    DWORD      dwFileSystemType;           // 文件系统类型
    DWORD      dwFileSystemFlags;          // 文件系统标志掩码    
}WT_FS_VOLUME_INFO, *PWT_FS_VOLUME_INFO, *LPWT_FS_VOLUME_INFO;

/* 文件信息 */
typedef struct _WT_FS_FILE_INFO_
{
    LPCTSTR          pszPath;                // 文件路径
    LPCTSTR          pszResolvedPath;        // 文件绝对路径
    HANDLE           hFile;                  // 文件句柄
    DWORD            dwFileAttributes;       // 文件属性
    LARGE_INTEGER    liFileSize;             // 文件大小    
    LARGE_INTEGER    liFileIndex;            // 文件索引
}WT_FS_FILE_INFO, *PWT_FS_FILE_INFO, *LPWT_FS_FILE_INFO;

// 文件延伸区块buff，一个延伸区块是连续存储的
typedef struct _WT_FS_FILE_EXTENTS
{
    DWORD dwExtentCount;               //文件存储延伸区块数
    struct {
       DWORD dwClusterCount;           // 簇数
        LARGE_INTEGER    liStartLcn;   //起始逻辑簇号
        } Extents[ ANYSIZE_ARRAY ];
}WT_FS_FILE_EXTENTS, *PWT_FS_FILE_EXTENTS, *LPWT_FS_FILE_EXTENTS;

/* 小文件,小于1KB的小文件存在MFT里 */
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

/* 打开文件
 * 应该先使用WTFsOpenFile来打开文件
 * 然后使用WTFsReadFile读取数据
 * 最后需要调用WTFsClose来关闭文件释放资源
 */
BOOL WTFsOpenFile(LPCTSTR pszSrcFileName);

/* 
 * 读文件 
 * @[in]
 * pszSrcFileName - 需要读取文件的物理绝对路径
 * lpPhyData - 读取到的物理数据
 * nByte - 读取到的物理数据长度
 * bFlag - 是否为最后一块 (0 - 不是最后一块, 1 - 是最后一块)
 * @[out]
 * 成功返回 true, 否则返回系统错误号.
 */
BOOL WTFsReadFile(LPCTSTR pszSrcFileName,
    LPVOID *lpPhyData,
    ULONG *nByte,
    BOOL *bFlag);
    
/* 关闭文件，释放所分配的空间 */
VOID WTFsClose(LPCTSTR pszSrcFileName);

/* 分析FAT文件 */
BOOL WTFsAnalyzeFAT(PWT_FS_VOLUME_INFO pFsVolInfo);

/* 获取卷信息 */
BOOL WTFsGetVolumeInfo(LPCTSTR pszPath, PWT_FS_VOLUME_INFO pFsVolInfo);

/* 释放卷信息 */
VOID WTFsFreeVolumeInfo(PWT_FS_VOLUME_INFO pFsVolInfo);

/* 获取文件信息 */
BOOL WTFsGetFileInfo(LPCTSTR pszPath, PWT_FS_FILE_INFO pFsFileInfo);

/* 释放文件信息 */
VOID WTFsFreeFileInfo(PWT_FS_FILE_INFO pFsFileInfo);

/* 获取文件延伸块 */
BOOL WTFsGetFileExtents(PWT_FS_VOLUME_INFO pFsVolInfo, 
    PWT_FS_FILE_INFO pFsFileInfo,
    PWT_FS_FILE_EXTENTS* ppFsFileExtents);

/* 读MFT */
BOOL WTFsReadFileMFT(PWT_FS_VOLUME_INFO pFsVolInfo,
    PWT_FS_FILE_INFO pFsFileInfo,
    PWT_FS_BLOB *ppFsFileData);

#endif // _WAVETOP_MIRROR_FS_IO_H_