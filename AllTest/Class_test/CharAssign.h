#pragma once
#include "public.h"
class CharAssign
{

public:
    CharAssign(void);
    ~CharAssign(void);
    char *Assign(char *&intput);
    int myAtoi(char* str);

    long long myatoll(const char *p);

    char *Mystrdup(const char *s);
    void strdupTest();
    void *CharAssign::memcpy(void *dst, const void *src, size_t len);
    void* CharAssign::memmove(void* str1,const void* str2,size_t n);

     void CharAssign::XINT32Test();
     void CharAssign::bigEndianTest(void);

     char * PathHandle(char *pszPath);
    void CharAssign::Myunsignedchar(unsigned char v);
};


void StringPopBack();
void PathFilterTest();

void StringNULL();

//输出日志到txt中
void OutPutfile();

/* the integer byte-order */
#define XINT32(n,p) {\
    (p)[0]=(unsigned char)(((n)>>24)&0xff);\
    (p)[1]=(unsigned char)(((n)>>16)&0xff);\
    (p)[2]=(unsigned char)(((n)>>8)&0xff);\
    (p)[3]=(unsigned char)( (n)&0xff);\
}
#define VINT32(n,p) {\
    n= (((unsigned char)((p)[0])) << 24) + \
    (((unsigned char)((p)[1])) << 16) + \
    (((unsigned char)((p)[2])) << 8 ) + \
    (((unsigned char)((p)[3]))      ) ; \
}

#define XINT16(n,p) {\
    (p)[0]=(unsigned char)(((n)>>8)&0xff);\
    (p)[1]=(unsigned char)( (n)&0xff);\
}
#define VINT16(n,p) {\
    n= (((unsigned char)((p)[0])) << 8) + \
    (((unsigned char)((p)[1]))     ) ; \
}