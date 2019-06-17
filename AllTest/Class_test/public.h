#pragma once
#include <Windows.h>
#include <iostream>
#include <string>
#include "time.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <assert.h>
#include <process.h>
#include <memory.h>
#include <fstream>
#include <WinSock2.h>
#include <Strsafe.h>
#include <map>
#include <io.h>
#include <sys/stat.h>
//#include <WinSock2.h

#include "nspr.h"
using namespace std;


typedef struct _ADO_PARAM_ST{
    char   szIP[260];
    int    nPort;
    char   szUser[260]; 
    char   szPwd[260];
    char   szDbName[260];
    char   szDeviceName[260];
    char   szInstance[260];
    int    nDBNativeError;
}AdoParamSt;
