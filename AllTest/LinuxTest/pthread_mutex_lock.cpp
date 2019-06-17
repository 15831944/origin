#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>


//定义一个全局的互斥变量
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
//int pthcount = 0;

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


/* 定义线程pthread */
static void * pthread(void *arg)       
{
    char buf[1024];
    int nCount = 0;
    int nRet   = 0;
    char pszName[] = "/home/yq/storle.lo";
    unsigned long nFileHandle;
    nFileHandle = open(pszName, O_CREAT|O_RDWR, 0666);
    if(nFileHandle == -1){
        printf("child open failed\n");
        return NULL;
    }else{
        printf("child open success\n");
    }

    pthread_mutex_lock(&mutex);
    nRet = BackupFileLockLock(nFileHandle, 2);  
    if(nRet != 0){
        printf("child BackupFileLockLock failed\n");
    }else{
        printf("child BackupFileLockLock success\n");
    }

    while(1){
        printf("child a number %d s\n", nCount);
        nCount++;
        if(nCount > 15){
            printf("child a end\n");
            break;
        }
        sleep(1);
    }

    nRet = BackupFileLockUnlock(nFileHandle, 2);
    if(nRet != 0){
        printf("child BackupFileLockUnlock failed\n");
    }else{
        printf("child BackupFileLockUnlock success\n");
    }

    pthread_mutex_unlock(&mutex);

    

    return NULL;
}

/* 定义线程pthread */
static void * pthread2(void *arg)       
{
    char buf[1024];
    int nCount = 0;
    int nRet   = 0;
    char pszName[] = "/home/yq/storle.lo";
    unsigned long nFileHandle;
    nFileHandle = open(pszName, O_CREAT|O_RDWR, 0666);
    if(nFileHandle == -1){
        printf("child open failed\n");
        return NULL;
    }else{
        printf("child open success\n");
    }
    nRet = BackupFileLockLock(nFileHandle, 2);  
    pthread_mutex_lock(&mutex);
    
    if(nRet != 0){
        printf("child BackupFileLockLock failed\n");
    }else{
        printf("child BackupFileLockLock success\n");
    }

    while(1){
        printf("child b number %d s\n", nCount);
        nCount++;
        if(nCount > 15){
            printf("child b end\n");
            break;
        }
        sleep(1);
    }

    nRet = BackupFileLockUnlock(nFileHandle, 2); 
    if(nRet != 0){
        printf("child BackupFileLockUnlock failed\n");
    }else{
        printf("child BackupFileLockUnlock success\n");
    }

    pthread_mutex_unlock(&mutex);
    return NULL;
}

/* main函数 */
int main(int agrc,char* argv[])
{
    pthread_t tidp;
    pthread_t tidp2;
    char buf[1024];
    int nCount = 0;
    int nRet   = 0;

    /* 创建线程pthread */
    if (pthread_create(&tidp, NULL, pthread, NULL) == -1)
    {
        printf("create error!\n");
        return 1;
    }

    /* 创建线程pthread */
    if (pthread_create(&tidp2, NULL, pthread2, NULL) == -1)
    {
        printf("create error!\n");
        return 1;
    }

    /* 等待线程pthread释放 */
    if (pthread_join(tidp, NULL) && pthread_join(tidp2, NULL))                  
    {
        printf("thread is not exit...\n");
        return -2;
    }

    return 0;

}   