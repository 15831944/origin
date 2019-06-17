/*
** File:                configfile.h
** Description: read/write configfile; ~ vs Windows-ini
**
** revision 1.1
** add findfirstsection() and findnextsection() to enum the data list
** Open config file use PR_RDONLY
**
** version 1.0  
** base code
** ===================================
** config file format:
** [section]
** fieldname = value
** #-----------------------------------
** comment char ['#']
**/

#ifndef __READ_WRITE_CONFIG_FILE_H__
#define __READ_WRITE_CONFIG_FILE_H__

#include <nspr.h>

//errors: value < 0
#define CFGERR_UNKNOW       -1000
#define CFGERR_INVALIDARG   -1001
#define CFGERR_OUTMEMORY    -1002
#define CFGERR_NOEXISTFILE  -1003
#define CFGERR_OPFILE       -1004
#define CFGERR_BUFSIZE      -1004
#define CFGERR_BKFILE       -1005

//status
#define CFGSTA_CLOSED   0
#define CFGSTA_OPEN     1
#define CFGSTA_UPDATE   2
#define CFGSTA_ERROR    -1

typedef void * CFG_SECTION_HANDLE;

class CConfigfile  
{
public:
    enum CFG_ITEM_TYPE
    {
        cfg_t_comment = 0,
        cfg_t_deaddata,
        cfg_t_sectiohead,
        cfg_t_data
    };
    struct CFG_ITEM
    {
        struct CFG_ITEM * m_pNext;
        char * m_pszOrgLine;
        int m_nOrgLineLen;
        CFG_ITEM_TYPE m_nItemType;
    };

    CConfigfile();
    CConfigfile(const char * szFilename);
    ~CConfigfile();

    int Open(const char *szFilename);
    int Close();

    int ReadStream(char *pszStream);
    
    /*
    return:
      >=0 -- length of returned buf; others -- failure; returned buf terminated by '\0'
      !be sure the buf is enough to contain szDefault
    */
    int  ReadString(const char *szSection, const char *szFieldname, char * buf, int bufsize, const char * szDefault);
    int  ReadSection(const char *szSection, char * buf, int bufsize);
        
    //return string can't be used during the operation on the same section+fieldname
    const char * ReadString(const char *szSection, const char *szFieldname);
    
    int ReadInt(const char *szSection, const char *szFieldname, int iDefault);
    long long ReadInt64(const char *szSection, const char *szFieldname, long long iDefault);
    /*
     return 0 -- success; others -- failure
    */
    int WriteString(const char *szSection, const char *szFieldname, char * szValue);
    //lines in szValue splited by '\n'
    int WriteSection(const char *szSection, char * szValue);
    int WriteInt(const char *szSection, const char *szFieldname, int iValue);
    int WriteInt64(const char *szSection, const char *szFieldname, long long iValue);
    // 0 -- success; 1 -- data is not changed; <0 -- failure
    int Save();

    int DeleteField(const char *szSection, const char *szFieldname);

    // discard update data
    int DiscardUpdate() { if( CFGSTA_UPDATE == m_nStatus) m_nStatus = CFGSTA_OPEN; return m_nStatus; }

    //szSection[in] -- section name, if szSection == NULL or "", it will return the begin data
    //if return NULL, found nothing; others -- found it
    //don't modify/delete the return handle
    CFG_SECTION_HANDLE GetSectionHandle(const char *szSection);
    
    //prev[in] -- get from FindFirstSection(...)
    //buf[out] -- return the next section name from prev, terminated by '\0';
    //bufsize[in] -- the length of szSection
    //if success to find next section, it returns !(NULL) ;
    //if failure, it returns NULL
    //don't modify/delete the return handle
    CFG_SECTION_HANDLE FindNextSection(CFG_SECTION_HANDLE prev, char * buf, int bufsize);

protected:
    void Init();
    void Clear();
    CFG_ITEM_TYPE GetLineType(char * pszline);
    /*
    out [out] --  >=0 -- length of returned string, others -- failure
    return value -- NULL -- failure, others -- success
    */
    char * MergeData(char *str0, int len0, char *str1, int len1, int &out);

    // 0 -- success; others -- failure
    //pszline is created by method 'new', in other case func will bomb
    int PutLine(char *pszline,int len);

    //parse data
    CFG_ITEM * GetSectionItem(const char * szSection,bool bCreate = false);
    CFG_ITEM * GetFieldItem(CFG_ITEM * pSectionItem, const char * szFieldname);
    const char * GetFieldValue(CFG_ITEM * pSectionItem, const char * szFieldname);
    //if equal, then returns 0, else returns others
    int iEqual(const char *str0, int len0, const char *str1, int len1);
private:    
    int m_nStatus; // 0 -- normal,  -1 -- closed, 1 -- data updated,should save before exit, <0 -- error
    char * m_pszFilename;
    PRFileDesc * m_pfd;
    //assume the headitem has NULL section name, will be returned with GetSectionItem(NULL)
    CFG_ITEM m_itemHead;
    CFG_ITEM * m_pitemCurrent;
    CFG_ITEM * m_pitemLast;
};

#endif // __READ_WRITE_CONFIG_FILE_H__
