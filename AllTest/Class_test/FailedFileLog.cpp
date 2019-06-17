#include "FailedFileLog.h"

FailedFileLog::FailedFileLog(const char *serverdir){

    log = string(serverdir)+"/logs/FailedFile.log";
}

FailedFileLog::~FailedFileLog()
{
    Flush();
}

int FailedFileLog::Init()
{
    if(PR_Access(log.c_str(),PR_ACCESS_EXISTS)==PR_SUCCESS){
        PR_Delete(log.c_str());
    }
    fd = PR_Open(log.c_str(),PR_CREATE_FILE|PR_RDWR,0666);
    if(fd == NULL){
        return 0;
    }else{
        return 0;
    }
}

int FailedFileLog::Write(const char *filename)
{
    memset(buff,0,4096);
    PR_snprintf(buff,4096,"%s\n",filename);
    return PR_Write(fd,buff,strlen(filename)+1);
}

void FailedFileLog::Flush()
{
    if(fd!=NULL){
        PR_Close(fd);
        fd = NULL;
    }
}