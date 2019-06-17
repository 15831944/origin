#pragma once
#include "public.h"

class CGetFileInfo
{
public:
    CGetFileInfo(void);
    ~CGetFileInfo(void);

    int GetFileSize();
    int CGetFileInfo::GetDirSize();
};

