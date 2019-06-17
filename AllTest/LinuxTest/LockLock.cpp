#include <unistd.h>
#include <fcntl.h>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>
#include "stdio.h"

#define ERR_EXIT(msg) \
    do \
  { \
  perror(msg); \
  exit(-1); \
  } while(0)

int BackupFileLockLock(unsigned long fd, int rw);
int BackupFileLockUnlock(unsigned long fd, int rw);

int main()
{
    char buf[1024];
    int nCount = 0;
    int nRet   = 0;
    char pszName[] = "/home/yq/storle.lo";
    unsigned long nFileHandle;
    nFileHandle = open(pszName, O_CREAT|O_RDWR, 0666);
    if(nFileHandle == -1){
        ERR_EXIT("open failed\n");
    }else{
        printf("open success\n");
    }

    nRet = BackupFileLockLock(nFileHandle, 2);
    if(nRet != 0){
        printf("BackupFileLockLock failed\n");
    }else{
        printf("BackupFileLockLock success\n");
    }
    while(1){
        printf("µ⁄ %d √Î\n", nCount);
        nCount++;
        if(nCount > 30){
            printf("∂¡√ÎΩ· ¯\n");
        }
        sleep(1000);
    }

    BackupFileLockUnlock(nFileHandle, 2);
    if(nRet != 0){
        printf("BackupFileLockUnlock failed\n");
    }else{
        printf("BackupFileLockUnlock success\n");
    }

    getchar();
    return 0;
}

int BackupFileLockLock(unsigned long fd, int rw)
{
    struct flock lock_it;
    int cmd;
    int ret;

    lock_it.l_whence = SEEK_SET;
    lock_it.l_start = 0;
    lock_it.l_len = 0;
    lock_it.l_type = (rw == 2 ? F_WRLCK : F_RDLCK);
    lock_it.l_pid = 0;  /* pid not actually interesting */
    cmd = (rw == 2 ? F_SETLKW : F_SETLK);
    while ((ret = fcntl(fd, cmd, &lock_it)) < 0 
        && errno == EINTR); /* nop */
    return (ret < 0 ? 1 : 0);
}

int BackupFileLockUnlock(unsigned long fd, int rw)
{
    struct flock unlock_it;
    int cmd;
    int ret;

    unlock_it.l_whence = SEEK_SET;  /* from current point */
    unlock_it.l_start = 0;      /* -"- */
    unlock_it.l_len = 0;        /* until end of file */
    unlock_it.l_type = F_UNLCK;     /* set exclusive/write lock */
    unlock_it.l_pid = 0;        /* pid not actually interesting */
    cmd = (rw == 2 ? F_SETLKW : F_SETLK);
    while ((ret = fcntl(fd, cmd, &unlock_it)) < 0 
        && errno == EINTR); /* nop */
    return (ret < 0 ? 1 : 0);
}