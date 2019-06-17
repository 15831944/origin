#ifndef __FT_CONFIG_H
#define __FT_CONFIG_H  1
#include "ft_pack.h"
#include <vector>
#include <utility>
#include <string>

#define FT_DEV_NO_LEFT 1
#define FT_CONFIG_DB_ERR 2
#define FT_CONFIG_DB_ERR_OPEN FT_CONFIG_DB_ERR + 1
#define FT_CONFIG_DB_ERR_LOCK FT_CONFIG_DB_ERR + 2
#define FT_CONFIG_DB_ERR_EXE FT_CONFIG_DB_ERR + 3
#define FT_CONFIG_DB_ERR_CLOSE FT_CONFIG_DB_ERR + 4

#define FT_DB_NAME "fc_resource_pool.db"

int getFTDevPair(const char* pszServer, ft_dev_vec& devs,int* block_size);

int putFTDevPair(ft_dev_vec& devs);

int sendFTDevPair(sbuff* pSB, ft_dev_vec& devs, int block_size);

int recvFTDevPair(sbuff* pSB, ft_dev_vec& devs, int* block_size);

int checkSrvDev(ft_dev_vec& devs);

int checkCltDev(ft_dev_vec& devs);

int getDBPath(char* pszPath);

int openDB(const char* pszDb,void** db);

int lockDB(void* db);

int unLockDB(void* db);

int closeDB(void* db);

int findDevice(const char* pszObjName,char* deviceName,int size);

int getSockIp(int fd, char *pszIp, int nlen);
#endif
