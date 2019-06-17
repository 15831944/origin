#include "struct_function.h"
using namespace std;



void toLowerCase_test()
{
    char *psz = (char *)malloc(12);
    _snprintf(psz,12,"Hello");
    psz = toLowerCase(psz);
    cout<<psz<<endl;
}
    

void main()
{

    //test1();

   /* char a = 'H';
    a = a|0x20;
    toLowerCase_test();*/

    ofstreamTxt("hello");
    
    system("pause");
}