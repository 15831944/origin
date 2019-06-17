/* minigzip.c -- simulate gzip using the zlib compression library
 * Copyright (C) 1995-2005 Jean-loup Gailly.
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/*
 * minigzip is a minimal implementation of the gzip utility. This is
 * only an example of using zlib and isn't meant to replace the
 * full-featured gzip. No attempt is made to deal with file systems
 * limiting names to 14 or 8+3 characters, etc... Error checking is
 * very limited. So use minigzip only for testing; use gzip for the
 * real thing. On MSDOS, use only on file names without extension
 * or in pipe mode.
 */

/* @(#) $Id: minigzip.h,v 1.2 2009/05/26 02:58:12 huaizimin Exp $ */

#include <windows.h>
#include <stdio.h>
#include "zlib.h"
#include <assert.h>

#ifdef STDC
#  include <string.h>
#  include <stdlib.h>
#endif

#ifdef USE_MMAP
#  include <sys/types.h>
#  include <sys/mman.h>
#  include <sys/stat.h>
#endif

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

#ifdef VMS
#  define unlink delete
#  define GZ_SUFFIX "-gz"
#endif
#ifdef RISCOS
#  define unlink remove
#  define GZ_SUFFIX "-gz"
#  define fileno(file) file->__file
#endif
#if defined(__MWERKS__) && __dest_os != __be_os && __dest_os != __win32_os
#  include <unix.h> /* for fileno */
#endif

#ifndef WIN32 /* unlink already in stdio.h for WIN32 */
  extern int unlink OF((const char *));
#endif

#ifndef GZ_SUFFIX
#  define GZ_SUFFIX ".gz"
#endif
#define SUFFIX_LEN (sizeof(GZ_SUFFIX)-1)

#define BUFLEN      16384
#define MAX_NAME_LEN 1024

#ifdef MAXSEG_64K
#  define local static
   /* Needed for systems with limitation on stack size. */
#else
#  define local
#endif

char *prog;

void error            OF((const char *msg));
void gz_compress      OF((FILE   *in, gzFile out));
#ifdef USE_MMAP
int  gz_compress_mmap OF((FILE   *in, gzFile out));
#endif
void gz_uncompress    OF((gzFile in, FILE   *out));
void file_compress    OF((char  *file, char *mode));
void file_uncompress  OF((char  *file));
int  main             OF((int argc, char *argv[]));

/* ===========================================================================
 * Display error message and exit
 */
void error(const char *msg)
{
    fprintf(stderr, "%s: %s\n", prog, msg);
    exit(1);
}

/* ===========================================================================
 * Compress input to output then close both files.
 */

void gz_compress(FILE *in, gzFile out)
{
    local char buf[BUFLEN];
    int len;
    int err;

#ifdef USE_MMAP
    /* Try first compressing with mmap. If mmap fails (minigzip used in a
     * pipe), use the normal fread loop.
     */
    if (gz_compress_mmap(in, out) == Z_OK) return;
#endif
    for (;;) {
        len = (int)fread(buf, 1, sizeof(buf), in);
        if (ferror(in)) {
            perror("fread");
            exit(1);
        }
        if (len == 0) break;

        if (gzwrite(out, buf, (unsigned)len) != len) error(gzerror(out, &err));
    }
    fclose(in);
    if (gzclose(out) != Z_OK) error("failed gzclose");
}

#ifdef USE_MMAP /* MMAP version, Miguel Albrecht <malbrech@eso.org> */

/* Try compressing the input file at once using mmap. Return Z_OK if
 * if success, Z_ERRNO otherwise.
 */
int gz_compress_mmap(in, out)
    FILE   *in;
    gzFile out;
{
    int len;
    int err;
    int ifd = fileno(in);
    caddr_t buf;    /* mmap'ed buffer for the entire input file */
    off_t buf_len;  /* length of the input file */
    struct stat sb;

    /* Determine the size of the file, needed for mmap: */
    if (fstat(ifd, &sb) < 0) return Z_ERRNO;
    buf_len = sb.st_size;
    if (buf_len <= 0) return Z_ERRNO;

    /* Now do the actual mmap: */
    buf = mmap((caddr_t) 0, buf_len, PROT_READ, MAP_SHARED, ifd, (off_t)0);
    if (buf == (caddr_t)(-1)) return Z_ERRNO;

    /* Compress the whole file at once: */
    len = gzwrite(out, (char *)buf, (unsigned)buf_len);

    if (len != (int)buf_len) error(gzerror(out, &err));

    munmap(buf, buf_len);
    fclose(in);
    if (gzclose(out) != Z_OK) error("failed gzclose");
    return Z_OK;
}
#endif /* USE_MMAP */

/* ===========================================================================
 * Uncompress input to output then close both files.
 */
void gz_uncompress(gzFile in, FILE *out)
{
    local char buf[BUFLEN];
    int len;
    int err;

    for (;;) {
        len = gzread(in, buf, sizeof(buf));
        if (len < 0) error (gzerror(in, &err));
        if (len == 0) break;

        if ((int)fwrite(buf, 1, (unsigned)len, out) != len) {
            error("failed fwrite");
        }
    }
    if (fclose(out)) error("failed fclose");

    if (gzclose(in) != Z_OK) error("failed gzclose");
}


/* ===========================================================================
 * Compress the given file: create a corresponding .gz file and remove the
 * original.
 */
void file_compress(char *file, char *mode)
{
    local char outfile[MAX_NAME_LEN];
    FILE  *in;
    gzFile out;

    strcpy(outfile, file);
    strcat(outfile, GZ_SUFFIX);

    in = fopen(file, "rb");
    if (in == NULL) {
        perror(file);
        exit(1);
    }
    out = gzopen(outfile, mode);
    if (out == NULL) {
        fprintf(stderr, "%s: can't gzopen %s\n", prog, outfile);
        exit(1);
    }
    gz_compress(in, out);

    unlink(file);
}


/* ===========================================================================
 * Uncompress the given file and remove the original.
 */
void file_uncompress(char *file)
{
    local char buf[MAX_NAME_LEN];
    char *infile, *outfile;
    FILE  *out;
    gzFile in;
    uInt len = (uInt)strlen(file);

    strcpy(buf, file);

    if (len > SUFFIX_LEN && strcmp(file+len-SUFFIX_LEN, GZ_SUFFIX) == 0) {
        infile = file;
        outfile = buf;
        outfile[len-3] = '\0';
    } else {
        outfile = file;
        infile = buf;
        strcat(infile, GZ_SUFFIX);
    }
    in = gzopen(infile, "rb");
    if (in == NULL) {
        fprintf(stderr, "%s: can't gzopen %s\n", prog, infile);
        exit(1);
    }
    out = fopen(outfile, "wb");
    if (out == NULL) {
        perror(file);
        exit(1);
    }

    gz_uncompress(in, out);

    unlink(infile);
}

/*decompress from buffer source to buffer dest*/
int buff_decompress(char *source, char **dest, int *nBlockSize)						 
{
	int ret;
	int count = 0;
	int nResult = WAVETOP_BACKUP_OK;
	unsigned have;
	Bytef *out = NULL;
	char *pos = NULL;
	z_stream  strm;
	
	typedef struct Buff{
		char *block;
		int lenth;
		Buff *next;
	} Buff;
	
	Buff *pHead = NULL;
	Buff *pNode = NULL;
	Buff *pTemp = NULL;
	
	/*allocate inflate state */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	ret = inflateInit(&strm);
	if (ret != Z_OK) {
		printf("buff_decompress: deflateInit failed, code %d", ret);
		 nResult = WAVETOP_BACKUP_INTERNAL_ERROR;
		return nResult;
	}
	
	/* decompress until deflate stream ends */	
	strm.avail_in = *nBlockSize;
	strm.next_in = (Bytef *)source;	
	
	do {
		out = (Bytef *)malloc(BUFLEN);
		if (out == NULL) {
			printf("buff_decompress: memory allocate error");
			nResult = WAVETOP_BACKUP_INTERNAL_ERROR;
			goto EXIT;
		}

		strm.avail_out = BUFLEN;
		strm.next_out = out;
		
		ret = inflate(&strm, Z_NO_FLUSH);
		
		/* state not clobbered */
		assert(ret != Z_STREAM_ERROR);  
		switch (ret) {
		case Z_NEED_DICT:
			ret = Z_DATA_ERROR;     /* and fall through */
		case Z_DATA_ERROR:
		case Z_MEM_ERROR:
			(void)inflateEnd(&strm);
			return ret;
		}
		
		have = BUFLEN - strm.avail_out;
		count++;
		pNode = new Buff;

		if (NULL == pNode) {
			printf("buff_decompress: memory allocate error");
			nResult = WAVETOP_BACKUP_INTERNAL_ERROR;
			goto EXIT;
		}
		
		pNode->block = (char *)out;
		pNode->lenth = have;
		pNode->next = NULL; 

		if (pHead == NULL) {
			pHead = pNode;
		}
		else {
			pTemp = pHead;
			while (pTemp->next != NULL) {
				pTemp = pTemp->next;
			}

			pTemp->next = pNode;
		}
		
	} while(strm.avail_out == 0);

	/* clean up and return */
    (void)inflateEnd(&strm);
// 	if(ret != Z_STREAM_END) {
// 		printf("buff_decompress: data process cannot reach the end");
// 		nResult = WAVETOP_BACKUP_INTERNAL_ERROR;
// 		goto EXIT;
// 	}
	
	/* 将压缩后的数据写入传进来的内存块中*/
	*dest = (char *)malloc(count * BUFLEN);
	if (NULL == *dest) {
		printf("buff_decompress: memory allocate error");
		nResult = WAVETOP_BACKUP_INTERNAL_ERROR;
		goto EXIT;
	}

	memset(*dest, 0, count * BUFLEN);
	pNode = pHead;
	pos = *dest;
	*nBlockSize = 0;
	while (pNode != NULL) {
		pTemp = pNode;
		memcpy(pos, pNode->block, pNode->lenth);
		pos += pNode->lenth;
		*nBlockSize += pNode->lenth;
		pNode = pNode->next;
		delete pTemp->block;
		delete pTemp;
	}

EXIT:	
	return nResult;   
}