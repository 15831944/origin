/** =============================================================================
 ** Copyright (c) 2012 WaveTop Information Corp. All rights reserved.
 **
 ** The VIXDISK DLL
 **
 ** =============================================================================
 */
#ifndef __LIB_VIX_DISK_H
#define __LIB_VIX_DISK_H
#ifndef WIN32
#include <pthread.h>
#endif
#include "wconfig.h"
class VIXLock {
public:
    VIXLock();
    void lock();
    void unlock();
    ~VIXLock();
private:
#ifdef _WIN32
    CRITICAL_SECTION cs;
#else
    pthread_mutex_t mutex;
#endif
};

API_EXPORT(int) InitVixDiskLib();
#endif 

