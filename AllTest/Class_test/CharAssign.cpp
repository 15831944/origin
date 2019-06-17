#include "CharAssign.h"


CharAssign::CharAssign(void)
{
}


CharAssign::~CharAssign(void)
{
}


 char *CharAssign::Assign(char *&input)
 {
     input = (char*)malloc(128);
     _snprintf_s(input, 128, 127, "%s","hello");
     return input;
     
 }

 int CharAssign::myAtoi(char* str) {
     if(!str)
         return -1;

     while(*str){
         if(*str != ' ')
             break;
         str++;
     }

     return 0;
 }  

 long long CharAssign::myatoll(const char *p)
 {
     long long n;
     int c, neg = 0;
     unsigned char   *up = (unsigned char *)p;

     if (!isdigit(c = *up)) {
         while (isspace(c))
             c = *++up;
         switch (c) {
         case '-':
             neg++;
             /* FALLTHROUGH */
         case '+':
             c = *++up;
         }
         if (!isdigit(c))
             return (0);
     }

     for (n = '0' - c; isdigit(c = *++up); ) {
         n *= 10; /* two steps to avoid unnecessary overflow */
         n += '0' - c; /* accum neg to avoid surprises at MAX */
     }

     return (neg ? n : -n);
 }

 char *CharAssign::Mystrdup(const char *s)
 {
     assert(s != NULL);
     size_t len = strlen(s) + 1;
     void *new1 = malloc(len);
     if(new1 == NULL)
         return NULL;
     return (char *) memcpy(new1, s, len);
 }

 void CharAssign::strdupTest()
 {
     char *pszstr = "helloworld";
     char *pszstr2 = Mystrdup(pszstr);
     cout<<pszstr2<<endl;
 }


 //实现memcpy库函数
 void* CharAssign::memcpy(void *dst, const void *src, size_t count){
     // 容错处理
     if(dst == NULL || src == NULL){
         return NULL;
     }
     unsigned char *pdst = (unsigned char *)dst;
     const unsigned char *psrc = (const unsigned char *)src;
     //判断内存是否重叠
     bool flag1 = (pdst >= psrc && pdst < psrc + count);
     bool flag2 = (psrc >= pdst && psrc < pdst + count);
     if(flag1 || flag2){
         cout<<"内存重叠"<<endl;
         return NULL;
     }
     // 拷贝
     while(count--){
         *pdst++ = *psrc++;
     }
     return dst;
 }


 void* CharAssign::memmove(void *dst, const void *src, size_t count){
     // 容错处理
     if(dst == NULL || src == NULL){
         return NULL;
     }
     unsigned char *pdst = (unsigned char *)dst;
     const unsigned char *psrc = (const unsigned char *)src;
     //判断内存是否重叠
     bool flag1 = (pdst >= psrc && pdst < psrc + count);
     bool flag2 = (psrc >= pdst && psrc < pdst + count);
     if(flag1 || flag2){
         // 倒序拷贝
         while(count){
             *(pdst+count-1) = *(psrc+count-1);
             count--;
         }//while
     }
     else{
         // 拷贝
         while(count--){
             *pdst++ = *psrc++;
         }//while
     }
     return dst;
 }



 void CharAssign::Myunsignedchar(unsigned char v)

 {

     char c = v;

     unsigned char uc = v;

     unsigned int a = c, b = uc;

     int i = c, j = uc;

     printf("----------------\n");

     printf("%%c: %c, %c\n", c, uc);

     printf("%%X: %X, %X\n", c, uc);

     printf("%%u: %u, %u\n", a, b);

     printf("%%d: %d, %d\n", i, j);

 }


 void CharAssign::bigEndianTest(void)
 {
    typedef union _u{
        int n;
        char c;
     }u;

    u a;
    a.n = 1;
    if(a.c == 0){
        cout<<"此系统为大端"<<endl;
    } 
    else if(a.c == 1){
        cout<<"此系统为小端"<<endl;
    }
 }

 void CharAssign::XINT32Test()
 {
     unsigned char *pszTemp1 = (unsigned char *)malloc(1024);
     unsigned int nNum1 = 0;
     unsigned char *pszTemp2 = (unsigned char *)malloc(1024);
     unsigned int nNum2 = 0;

     nNum1= nNum2 = 15000;

     memmove(pszTemp1, &nNum1, sizeof(nNum1));
     XINT32(nNum1, pszTemp2);

     cout<<pszTemp1<<endl;
     cout<<pszTemp2<<endl;



     free(pszTemp1);
     free(pszTemp2);
 }

 char * CharAssign::PathHandle(char *pszPath)
 {
     if(pszPath == NULL)
         return pszPath;

     char *pszTemp = NULL;
     char *pszHead = pszPath;
     

     pszTemp = strstr(pszPath,".\\");
     return pszPath;

 }

 char *BackupSQLFilename(char *pszFilename)
 {
     char cPathSepa = '\\';

     if (pszFilename == NULL)
         return NULL;

     char *pszOld = pszFilename;
     for (; *pszFilename; pszFilename++) {
         if (*pszFilename == '/')
             *pszFilename = cPathSepa;
         else if (*pszFilename == '\\')
             *pszFilename = cPathSepa;
     }
     return pszOld;
 }

 string PathFilter(string &str)
 {    
     char cPathSepa = '\\';
     int nLen = str.length();

     for(int i = 0; i< nLen;i++)
     {
         if(str[i] == '/')
             str[i] = cPathSepa;
     }

     return str;
 }

 void PathFilterTest()
 {
     string str("H:\\1/2/3/4/5");
     cout<<str<<endl;
     str = PathFilter(str);

     cout<<str<<endl;
 }

 void StringNULL()
 {
     string str = NULL;
     cout<<"success"<<endl;
 }

 void StringPopBack()
 {
/*
     string str;
     str = "123456";
     cout << str << endl;

     //方法一：使用substr()
     str = str.substr(0, str.length() - 1);
     cout << str << endl;

     //方法二：使用erase()
     str.erase(str.end() - 1);
     cout << str << endl;

     //方法三：使用pop_back()
     str.pop_back();
     cout << str << endl;*/

     string strSql;
     strSql = "123456789";
     cout<<strSql<<endl;

     strSql.pop_back();
     cout<<"After popback: "<<strSql<<endl;

     strSql.substr(0, strSql.length() -1);
     cout<<"After substr: "<<strSql<<endl;

     strSql.erase(strSql.end() -1);
     cout<<"After erase: "<<strSql<<endl;
 }

 void OutPutfile()
 {
	ofstream file("C:\\11\\log.txt",iostream::app);
	double * p3 = new double [3];
	p3[0] = 0.2;
	p3[1] = 0.5;
	p3[2] = 0.8;
	file << "p3[1] is " << p3[1] << ".\n";
	p3 = p3 + 1;
	file << "Now p3[0] is " << p3[0] << " and ";
	file << "p3[1] is " << p3[1] << ".\n";
	p3 = p3 - 1;
	delete [] p3;
	char sz[512] = {0};
	file << sz<<"\n";
	file.close();
 } 


 string Utf8ToGbk(const char *src_str)
 {
     int len = MultiByteToWideChar(CP_UTF8, 0, src_str, -1, NULL, 0);
     wchar_t* wszGBK = new wchar_t[len + 1];
     memset(wszGBK, 0, len * 2 + 2);
     MultiByteToWideChar(CP_UTF8, 0, src_str, -1, wszGBK, len);
     len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, NULL, 0, NULL, NULL);
     char* szGBK = new char[len + 1];
     memset(szGBK, 0, len + 1);
     WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, szGBK, len, NULL, NULL);
     string strTemp(szGBK);
     if (wszGBK) delete[] wszGBK;
     if (szGBK) delete[] szGBK;
     return strTemp;
 }

 void StringAssignTest()
 {
     std::string s;
     // assign(size_type count, CharT ch)
     s.assign(4, '=');
     std::cout << s << '\n'; // "===="

     std::string const c("Exemplary");
     // assign(basic_string const& str)
     s.assign(c);
     std::cout << c << "==" << s <<'\n'; // "Exemplary == Exemplary"

     // assign(basic_string const& str, size_type pos, size_type count)
     s.assign(c, 0, c.length()-1);
     std::cout << s << '\n'; // "Exemplar";

     // assign(basic_string&& str)
     s.assign(std::string("C++ by ") + "example");
     std::cout << s << '\n'; // "C++ by example"

     // assign(charT const* s, size_type count)
     s.assign("C-style string", 7);
     std::cout << s << '\n'; // "C-style"

     // assign(charT const* s)
     s.assign("C-style\0string");
     std::cout << s << '\n'; // "C-style"

     char mutable_c_str[] = "C-style string";
     // assign(InputIt first, InputIt last)
     s.assign(std::begin(mutable_c_str), std::end(mutable_c_str));
     std::cout << s << '\n'; // "C-style string"

     // assign(std::initializer_list<charT> ilist)
     //s.assign({ 'C', '-', 's', 't', 'y', 'l', 'e' });
     //std::cout << s << '\n'; // "C-style"
 }