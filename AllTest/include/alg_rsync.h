/** =============================================================================
 ** Copyright (c) 2003 WaveTop Information Corp. All rights reserved.
 **
 ** The backup system
 **
 ** =============================================================================
 */

#ifndef __RSYNC_ALGORITHM_H_
#define __RSYNC_ALGORITHM_H_ 1

#include <time.h>
#include "nspr.h"
#include "archive.h"

#include <sys/stat.h>
#include <errno.h>

/* support over 2GB file */
#define HAVE_OFF64_T 1

#define INO64_T PRInt64
#define DEV64_T PRInt64

#if HAVE_OFF64_T
#ifndef MIN
#define MIN(a,b) (LL_CMP(a, <, b) ? a : b)
#endif

#ifndef MAX
#define MAX(a,b) (LL_CMP(a, >, b) ? a : b)
#endif
#else
#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif
#endif

#define uchar unsigned char
#define schar char

#if HAVE_OFF64_T
#ifdef WIN32
#define OFF_T __int64
#define STRUCT_STAT struct _stati64
#else
#define OFF_T PRInt64
#define STRUCT_STAT struct stat64
#endif
#else
#define OFF_T off_t
#define STRUCT_STAT struct stat
#endif

/* a non-zero CHAR_OFFSET makes the rolling sum stronger, but is
 * incompatible with older versions :-( 
 */
#define CHAR_OFFSET 0


/* the length of the md4 checksum */
#define MD4_SUM_LENGTH 16
#define SUM_LENGTH 16

#define SPARSE_WRITE_SIZE (1024)
#define WRITE_SIZE (32*1024)
#define CHUNK_SIZE (32*1024)
#define MAX_MAP_SIZE (256*1024)
#define IO_BUFFER_SIZE (4092)

#define BLOCK_SIZE (700)

#define CVAL(buf,pos) (((unsigned char *)(buf))[pos])
#define PVAL(buf,pos) ((unsigned)CVAL(buf,pos))
#define SCVAL(buf,pos,val) (CVAL(buf,pos) = (val))

/* we know that the x86 can handle misalignment and has the "right" 
 * byteorder 
 */
#define SVAL(buf,pos) (PVAL(buf,pos)|PVAL(buf,(pos)+1)<<8)
#define IVAL(buf,pos) (SVAL(buf,pos)|SVAL(buf,(pos)+2)<<16)
#define SSVALX(buf,pos,val) (CVAL(buf,pos)=(val)&0xFF,CVAL(buf,pos+1)=(val)>>8)
#define SIVALX(buf,pos,val) (SSVALX(buf,pos,val&0xFFFF),SSVALX(buf,pos+2,val>>16))
#define SIVAL(buf,pos,val) SIVALX((buf),(pos),((uint32)(val)))

/* The file entry flags */
#define FILE_FILE (1<<0)
#define FILE_DIR (1<<1)
#define FILE_LNK (1<<2)
#define OS_FLAG_WIN32 (1<<3)
#define OS_FLAG_UNIX (1<<4)
#define SAME_MODE (1<<5)
#define SAME_RDEV (1<<6)
#define SAME_UID (1<<7)
#define SAME_GID (1<<8)
#define SAME_DIR (1<<9)
#define SAME_NAME SAME_DIR

#ifndef BLOCK_MINFREE
#define BLOCK_MINFREE 4096
#endif
#ifndef BLOCK_MINALLOC
#define BLOCK_MINALLOC 8192
#endif

/* the length of the md4 checksum */
#define MD4_SUM_LENGTH 16
#define SUM_LENGTH 16

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

#ifndef ACCESSPERMS
#define ACCESSPERMS 0777
#endif

/* Initial mask on permissions given to temporary files.  Mask off setuid
 * bits and group access because of potential race-condition security
 * holes, and mask other access because mode 707 is bizarre 
 */
#define INITACCESSPERMS 0700

#define WAVETOP_BACKUP_HAVE_BACKUP       10000

/* The rsync file node */
struct file_struct {
    /* The file exist or not exist for algorithm optimized.
     */
    int sum_count;

    /* The new version storing file name */
    char *newstore;
    unsigned int flags;
    time_t createtime;
    time_t modtime;
    PRInt64 length;
    mode_t mode;
    long attribute;

    INO64_T inode;

    /* Device this file lives upon */
    DEV64_T dev;

    /* If this is a device node, the device number. */
    DEV64_T rdev;
    uid_t uid;
    gid_t gid;
    char *basename;
    char *dirname;
    char *link;
    char *sum;

    /* Magzine */
    long magid;
    long magperiod;

    int backuptype;
    char *message;

    /* The file backup status
     * WAVETOP_BACKUP_HAVE_BACKUP -- have been backup
     * WAVETOP_BACKUP_FILE_NO_CHANGE - no change(does not backup)
     * Other the file status reference the backup_proto.h
     */
    int backup;
    
    /* The current version information 
     * curstore -- the current version store file
     * curlen -- the current version file length
     * curver -- the current version number
     */
    char *curstore;
    PRInt64 curlen;
    int curbackuptype;

    /* When a file no backup, its value is BACKUP_NO_VERSION */
    unsigned long curver;

    /* save delta file length */
    PRInt64 deltalen;
    
    /* The request handle */
    void *request;
    struct file_struct *next;
};

struct backup_info {
    PRInt64 nTotalFiles;
    PRInt64 nTotalFailed;
    PRInt64 nTotalSize;
    PRInt64 nRealFileLen;
    PRInt64 nTotalDirectory;
};

struct file_list {
    ap_pool *pool;
    request_rec *request;
    int count;
    int curpos;
    int malloced;
    struct file_struct **files;
};

struct sum_buf {
    OFF_T offset;           /* offset in file of this chunk */
    int len;                /* length of chunk of file */
    int i;                  /* index of this chunk */
    PRUint32 sum1;          /* simple checksum */
    char sum2[SUM_LENGTH];  /* checksum  */
};

struct sum_struct {
    PRInt64 flength;        /* total file length */
    PRInt32 count;          /* how many chunks */
    PRInt32 remainder;      /* flength % block_length */
    PRInt32 n;              /* block_length */
    struct sum_buf *sums;   /* points to info for each chunk */
};

#define CSUM_CHUNK 64

/* The rsync MD4 checksum session used by sum_xxx */
struct rsync_mdfour {
    struct mdfour md;
    int checksum_seed;
    char seed[4];      /* the checksum_seed byteorder */
    int sumresidue;
    char sumrbuf[CSUM_CHUNK];
};

/* The match session and targets used in the matching */
typedef unsigned short tag;
struct target {
    tag t;
    int i;
};
struct match_session {
    OFF_T last_match;
    struct target *targets;
    int *tag_table;
};

/* The file mapping structure */
struct map_struct {
    char *p;
    PRFileDesc *fd;
    int p_size;
    int p_len;
    OFF_T file_size;
    OFF_T p_offset;
    OFF_T p_fd_offset;
};

///////////////////////////////////////////////////////////////////////////
// The internal call API                                                 //
///////////////////////////////////////////////////////////////////////////
/* file I/O */
int                 sparse_end(PRFileDesc *fd);
int                 write_file(PRFileDesc *fd, char *buf, size_t len);
struct map_struct  *map_file(pool *p, PRFileDesc *fd, OFF_T len);
char               *map_ptr(struct map_struct *map, OFF_T offset, int len);
void                unmap_file(struct map_struct *map);

/* Checksum */
uint32              get_checksum1(char *buf1,int len);
int                 get_checksum2(pool *p, char *buf, int len, char *sum);
void                sum_init(struct rsync_mdfour *rmd);
void                sum_update(struct rsync_mdfour *rmd, char *p, int len);
void                sum_end(struct rsync_mdfour *rmd, char *sum);

/* Token */
void                send_token(sbuff *sb, int token, struct map_struct *buf, 
                               OFF_T offset,
                               int n,int toklen);
int                 recv_token(sbuff *sb, char **data);

/* flist */
struct file_list   *flist_new(pool *p);
void                match_sums(pool *p, struct match_session *ms, 
                               sbuff *sb, struct sum_struct *s, 
                               struct map_struct *buf, OFF_T len);

struct file_list   *recv_file_list(request_rec *r, sbuff *sb);
int backup_file(request_rec *r, sbuff *sb, StoreHandle *pStore, struct backup_info **pstBckFileInfo);
struct file_list   *send_file_list(pool *p, sbuff *sb, int filenum, char **fileargv);

///////////////////////////////////////////////////////////////////////////
// The standard call API                                                 //
///////////////////////////////////////////////////////////////////////////
/* Defined in the rsync library */
int                 do_rsync_recv(request_rec *r, const char *save_path, void *pStore);
int                 do_rsync_send(request_rec *r, int file_num, 
                                  const char **file_argv);

/* ap_stat supports 32 and 64 */ 
int                ap_stat(const char *fname, STRUCT_STAT *st);

/* get the temporary file name */
int                ap_get_tmpname(char *fnametmp, char *fname);

/**
 * Restore a file to a version, TODO:
 * 1) calculate the versions (from xxx to xxx)
 * 2) extract archive file
 * 3) patch it
 * 4) delete this patched file and dir
 * @[in]
 * pszFile is a restoring file name.
 * nRestoreVer is the version number.
 * @[out]
 * Return WAVETOP_BACKUP_OK, when successfully. Otherwise, return error code.
 * pszStoreFile is restored file name.
 * pFileVersion is restored file version information.
 */
int BkRestoreFileToVersion(request_rec *pReq, const char *pszFile,
                    int nRestoreVer, char **pszStoreFile, 
                    void *pFileVersion);

#endif /* __RSYNC_ALGORITHM_H_ */
