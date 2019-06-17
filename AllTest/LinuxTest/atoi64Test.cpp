#include <stdio.h>
#include <stdlib.h>
#include <iostream>
using namespace std;


/* mainº¯Êý */
int main(int agrc,char* argv[])
{
    char *pszTemp;
    pszTemp = "1234578910111213";
    long long a = _atoi64(pszTemp);
    cout<<"pszTemp: "<<pszTemp<<endl;
    cout<<"_atoi64(pszTemp): "<<a<<endl;

    return 0;
}