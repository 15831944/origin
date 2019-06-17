#pragma once
#include "public.h"
#include "vdi.h"
#include "vdierror.h"
#include "vdiguid.h"

#include "Ado.h"

class CVdiDemo
{
public:
    CVdiDemo(void);
    ~CVdiDemo(void);

    int init(char *pszDeviceName, char *pszInstance, char *pszFileName);
    //int MiAnsiToWideStr(char *pszStr, WCHAR *pwszStr, unsigned long nBufSize);

    int WriteBak();

    int VDI_ADO_Test();
private:
    IClientVirtualDeviceSet2 *m_pVds; 
    IClientVirtualDevice *m_pVd; 
    VDC_Command *m_pCmd;
    HANDLE m_hFile;
    VDConfig m_stConfig;
    DWORD m_nVirDevTimeOut;
    char m_szDeviceName[512];
    WCHAR m_wszDeviceName[512];
    char m_szInstanceName[512];
    WCHAR m_wszInstanceName[512];
    
};

