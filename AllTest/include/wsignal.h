/** =============================================================================
 ** Copyright (c) 2003 WaveTop Information Corp. All rights reserved.
 **
 ** The backup system
 **
 ** =============================================================================
 */

/* The signal handler */

#ifndef __BACKUP_SIGNAL_H_
#define __BACKUP_SIGNAL_H_ 1

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "nspr.h"
#include "wconfig.h"
#include "backup_proto.h"

void SignalTerminate(int nSig);
void SignalCoreDump(int nSig);


#endif /* __BACKUP_SIGNAL_H_ */
