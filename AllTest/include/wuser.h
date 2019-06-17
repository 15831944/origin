/** =============================================================================
 ** Copyright (c) 2003 WaveTop Information Corp. All rights reserved.
 **
 ** The backup system
 **
 ** =============================================================================
 */

/* User manager interface definition */

#ifndef __BACKUP_USER_H_
#define __BACKUP_USER_H_ 1

/* The error code */
#define WACL_OK                     0
#define WACL_RECORD_MALFORMED       1
#define WACL_DBM_OPEN_FAILED        2
#define WACL_DBM_OPERATION_FAILED   3
#define WACL_NO_MEMORY              4
#define WACL_INVALID_SYNTAX         5

#define ACL_USER_DBNAME             "wbkuser"

/* Typedef the user archive file structure */
typedef struct UserArchiveFileSt {
    
    struct UserArchiveFileSt *next;
    /* The User Archive File ID */    
    unsigned short nArchiveFileID;
    /* The Current User Archive File Path */
    char szArchiveFile[MAX_PATH];
    /* The Occupant which occupy the Archive File */
    char szOccupant[MAX_PATH];
    /* The Current User Archive File Max Size (Byte)*/
    PRInt64 nArchiveFileMaxSize;
    /* The Current User Archive File Used (Byte)*/
    PRInt64 nUsedSize;
    /* The User Archive File Once Increase Size (MB) */
    PRInt32 nIncreaseSize;
    /* The Archive File Type Flags:
     * WAVETOP_BACKUP_ARCHIVE_SHARE stat share Archive File
     * WAVETOP_BACKUP_ARCHIVE_SPECIAL stat special Archive File
     */
    unsigned long nArchiveFileType;
    /* The Reserve arg */
    char szReserve1[8];
    char szReserve2[8];
} UserArchiveFileSt;


/* Typedef the user structure */
typedef struct UserInfoSt {
    struct UserInfoSt *next;

    unsigned long nUserId;
    char szUser[255];
    char szPsw[64];
    char szLoginDir[255];
    char szDesc[255];
    PRInt64 nUserQuota;
    PRInt64 nUsedSize;
    short nArchiveFilesCount;

    /* nFlags 0 stat forbidden and 1 stat allowed */
    unsigned long nFlags;
    UserArchiveFileSt *pArchiveList;

    /* unsigned long nSuperiorId; */
    char szReserve1[8];
    char szReserve2[8];
} UserInfoSt;

/* Typedef the user item structure */
typedef struct UserItemSt {

    char szUnitName[255];
    char szLinkman[255];
    char szTel[16];
    char szEmail[255];
    char szIPAddr[32];
    /* unsigned long nSuperiorId; */
    char szReserve1[8];
    char szReserve2[8];
} UserItemSt;

/* Typedef the group structure */
typedef struct GroupInfoSt {
    struct GroupInfoSt *next;
    unsigned long nParentGroupId;
    unsigned long nGroupId;
    char szGroup[255];
    char szGroupDir[255];
    char szDesc[255];
    PRInt64 nGroupQuota;
    unsigned long nFlags;
    char szReserve1[8];
    char szReserve2[8];

    /* sub user and group list head node */
    UserInfoSt *pSubUserList;
    struct GroupInfoSt *pSubGroupList;

} GroupInfoSt;

/* Id list */
typedef struct IdNodeSt {
    struct IdNodeSt *pNext;
    unsigned long nUserId;
} IdNodeSt;

API_EXPORT(int)           ACLInit(const char *pszInitDir);
API_EXPORT(void)          ACLClose();
API_EXPORT(PRInt32)       ACLGetUserCount(PRInt32 *pnUserCount);
API_EXPORT(UserInfoSt *)  ACLGetAllUser();
API_EXPORT(void)          ACLFreeUser(UserInfoSt *pUserList);

/*
 * @[in] param
 * nAdded -- 0 is add; 1 is replace
 */
API_EXPORT(int)          ACLAddUser(UserInfoSt *pUserInfo, int nAdded);
API_EXPORT(int)          ACLDelUser(const char *pszUsername);
API_EXPORT(int)          ACLGetUserPerm(unsigned long nUserId, UserInfoSt *pUserInfo);
API_EXPORT(int)          ACLGetUserPerm(const char *pszUsername, UserInfoSt *pUserInfo);


/* Encode password */
int UserPswEncode(char *pszSrc, char *pszDest, unsigned long nDestBufSize);

/* Decode passwork */
int UserPswDecode(char *pszSrc, char *pszDest, unsigned long nDestBufSize);

#endif /* __BACKUP_USER_H_ 1 */
