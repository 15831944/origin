#include "Readfile.h"
#include "public.h"

CReadfile::CReadfile(char *pszPath)
{
    _snprintf_s(m_pszPath, sizeof(m_pszPath), pszPath);
}


CReadfile::~CReadfile(void)
{
}


int CReadfile::readbuffer()
{
    char buffer[256];  
    string str;
    ifstream in(m_pszPath, ios::in|ios::binary|ios::ate);  
    if (! in.is_open())  
    { cout << "Error opening file"; exit (1); }  
    while (!in.eof() )  
    {  
        unsigned int len = in.tellg();
        cout<<len<<endl;
        char *pszBuffer = new char[len];

        in.seekg (0, ios::beg);
        in.read(pszBuffer, len);
        str = pszBuffer;
        cout<<str.c_str()<<endl;
        /*in.getline (buffer,100);  
        cout << buffer << endl;  */
    }  
    in.close();
    return 0; 
}

/*
    CreateSemaphore
    OpenSemaphore
    ReleaseSemaphore

    CreateMutex
    OpenMutex
    ReleaseMutex

    InitializeCriticalSection
    EnterCriticalSection
    LeaveCriticalSection
    DeleteCriticalSection

    CreateEvent
    OpenEvent
    PulseEvent
    ResetEvent
    SetEvent
*/


