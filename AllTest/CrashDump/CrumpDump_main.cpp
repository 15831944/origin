#include <iostream>

using namespace std;
struct A{
    int a;
    int b;
    char sz[2048];
    };
typedef struct A A;
void main()
{   
    cout<<"����ʼ\n";
    A *sttest = NULL;
    cout<<sttest->a<<endl;
}