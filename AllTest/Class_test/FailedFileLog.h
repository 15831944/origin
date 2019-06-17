#pragma once
#include "public.h"


class FailedFileLog{
private:
    std::string log;
    PRFileDesc* fd;
    char buff[4096];

public:
    FailedFileLog(const char *serverdir);
    ~FailedFileLog();
    int Init();
    int Write(const char *filename);
    void Flush();
};
