#include <Windows.h>
#include <iostream>




struct test {
    union {
        struct  {
            VARTYPE vt;
            WORD    wReserved1;
            WORD    wReserved2;
            WORD    wReserved3;
            union {
                ULONGLONG     ullVal;       /* VT_UI8               */
                LONGLONG      llVal;        /* VT_I8                */
                LONG          lVal;         /* VT_I4                */
                BYTE          bVal;         /* VT_UI1               */
                SHORT         iVal;         /* VT_I2                */
                struct  {
                    int           n4Val;
                    PVOID         pvRecord;
                    IRecordInfo * pRecInfo;
                } __VARIANT_NAME_4;         /* VT_RECORD            */
            } __VARIANT_NAME_3;
        } __VARIANT_NAME_2;

        int decVal;
    } __VARIANT_NAME_1;
};


void test1();

//字符都转成小写
char* toLowerCase(char* str); 
void ofstreamTxt(char *pszConent);


