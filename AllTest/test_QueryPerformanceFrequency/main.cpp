#include <iostream>
#include <Windows.h>

#include "func.h"

using namespace std;
#define VINT32(n,p) {\
	n= (((unsigned char)((p)[0])) << 24) + \
	(((unsigned char)((p)[1])) << 16) + \
	(((unsigned char)((p)[2])) << 8 ) + \
	(((unsigned char)((p)[3]))      ) ; \

void main()
{
	
	time_t a = time(NULL);

	Sleep(1000);
	time_t b = time(NULL);

	cout<<b -a <<endl;
    
	
	system("pause");
	
}