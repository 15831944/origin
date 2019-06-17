#include "function.h"



int main()
{
    if ( FAILED(::CoInitialize(NULL)) )
        return -1;

   // MiMSSqlGetVersion();

    MiMSSqlGetDBModeShareExclusive();


    CoUninitialize();
    system("Pause");

}
