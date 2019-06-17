/** ========================================================================
 ** Copyright (c) 2003 WaveTop Information Corp. All rights reserved.
 **
 ** The Mirror system
 **
 ** ========================================================================
 */

/**
 * @brief for load mirror library export functions
 * @file libs/include/libmirror.h
 * @author lexiongjia
 */

#ifndef _LIBMIRROR_H_
#define _LIBMIRROR_H_ 1

#if defined(WIN32)
#include <windows.h>
#else
#include <unistd.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif /* defined(WIN32) */

#if defined(AIX)
#include <strings.h>
#endif

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

#include "libmirror_types.h"
#include "mifilelock.h"
#include "mistring.h"
#include "mitime.h"
#include "mialg.h"
#include "wmd5.h"
#include "miutil.h"

#endif /* !defined(_LIBMIRROR_H_) */
