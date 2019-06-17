/** =============================================================================
 ** Copyright (c) 2003-2005 WaveTop Information Corp. All rights reserved.
 **
 ** The Backup system
 **
 ** =============================================================================
 */

/* The protocol comments. */

#ifndef _BACKUP_PROTOCOL_COMMENTS_H_
#define _BACKUP_PROTOCOL_COMMENTS_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Get the comments of backup type.
 */
API_EXPORT(const char *) BackupGetBackupType(int nBackupType, int nReserve);

/**
 * Get the comments of file backup type
 */
API_EXPORT(const char *) BackupGetBackupFileType(int nBackupType, int nReserve);

/**
 * Get the authentication type
 */
API_EXPORT(const char *) BackupGetAuthType(int nAuthType, int nReserve);

/**
 * Get the Proto-Mehtod string
 */
API_EXPORT(const char *) BackupGetProtoMethodString(int nProtoMethod, int nReserve);

/**
 * Get the object type string
 */
API_EXPORT(const char *) BackupGetObjectType(int nObjectType, int nReserve);

#ifdef __cplusplus
}
#endif

#endif /* !defined(_BACKUP_PROTOCOL_COMMENTS_H_) */

