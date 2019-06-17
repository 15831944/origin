#pragma once
class CReadfile
{
public:
    CReadfile(char *pszPath);
    int readbuffer(); 
    ~CReadfile(void);
private:
    char m_pszPath[260];
};

