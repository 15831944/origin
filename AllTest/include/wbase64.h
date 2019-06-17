/** =============================================================================
 ** Copyright (c) 2003 WaveTop Information Corp. All rights reserved.
 **
 ** The backup system
 **
 ** =============================================================================
 */

#ifndef __BACKUP_BASE64_H_
#define __BACKUP_BASE64_H_ 1

#include <string.h>
#include "backup_proto.h"

#ifndef HUGE_STRING_LEN
#define HUGE_STRING_LEN  4096
#endif


#ifdef __cplusplus
extern "C" {
#endif


/* Base64 operations */
API_EXPORT(int)      BackupBase64DecodeLen(const char *bufcoded);
API_EXPORT(int)      BackupBase64Decode(char *bufplain, const char *bufcoded);
API_EXPORT(int)      BackupBase64DecodeBinary(unsigned char *bufplain,
                                  const char *bufcoded);
API_EXPORT(int)      BackupBase64EncodeLen(int len);
API_EXPORT(int)      BackupBase64Encode(char *encoded, const char *string, int len);
API_EXPORT(int)      BackupBase64EncodeBinary(char *encoded,
                                  const unsigned char *string, int len);

#ifdef __cplusplus
}
#endif


#endif /* __BACKUP_FUNCTION_H_ 1 */
