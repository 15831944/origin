#include "struct_function.h"
#include <iostream>
#include <fstream>
#include <stdio.h>
using namespace std;

void test1()
{
    int a = (1<<32) - 1;
    int b = (-180) % 30; 
    cout<<a<<endl;
    cout<<b<<endl;
}


char* toLowerCase(char* str) 
{
    for(char * ptr = str; *ptr ;++ptr){
        if(*ptr >= 'A' && *ptr <= 'Z')
            *ptr |= 0x20; 
    }
    return str;
}

void ofstreamTxt(char *pszConent)
{
    char szPath[256] = {0};
    GetModuleFileName(NULL, szPath , sizeof(szPath));
    char *pstr = strrchr(szPath,'\\');
    *pstr = '\0';
    strcat(szPath,"\\sql.txt");
    ofstream mycout(szPath);
    mycout<<pszConent<<endl;
    mycout.close();
}



