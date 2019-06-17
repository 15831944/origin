// configfile.cpp: implementation of the CConfigfile class.
//
//////////////////////////////////////////////////////////////////////
#include <string.h>
#include "configfile.h"
/*
*
* Revision 1.4  2004/11/29 07:25:43  fanmingyou
* m_nStatus
*
* Revision 1.3  2004/08/03 10:48:38  liqy
* When saving, does not to save the backup file
*
* Revision 1.2  2004/05/06 06:56:07  liqy
* 用NSPR实现
*
* Revision 1.1.1.1  2003/02/08 15:06:03  liqy
* WaveTop MenShen
*
* Revision 1.2  2003/02/08 13:59:47  liqy
* no message
*
*
* Revision 1.0 base
*/
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CConfigfile::CConfigfile()
{
    Init();
}

CConfigfile::CConfigfile(const char * szFilename)
{
    Init();
    Open(szFilename);
}

void CConfigfile::Init()
{
    /**
     * m_nStatus:
     *  0 -- normal, 
     * -1 -- closed, 
     *  1 -- data updated,should save before exit, 
     * <0 -- error 
     */
    m_nStatus = CFGSTA_CLOSED; 
    m_pszFilename = NULL;
    m_pfd = NULL;
    m_itemHead.m_nItemType = cfg_t_sectiohead;
    m_itemHead.m_pNext = NULL;
    m_itemHead.m_pszOrgLine = NULL;
    m_itemHead.m_nOrgLineLen = 0;
    m_pitemCurrent = NULL;
    m_pitemLast = &m_itemHead;
}

void CConfigfile::Clear()
{
    if(m_pszFilename){
        delete [] m_pszFilename;
        m_pszFilename = NULL;
    }
    
    if(m_pfd){
        PR_Close(m_pfd);
        m_pfd = NULL;
    }

    if(m_itemHead.m_pNext){
        CFG_ITEM *pNext = m_itemHead.m_pNext;
        CFG_ITEM *pItem;
        while(pNext){
            pItem = pNext;
            pNext = pNext->m_pNext;

            if(pItem->m_pszOrgLine){
                delete [] pItem->m_pszOrgLine;
            }
            delete pItem;
        }
        m_itemHead.m_pNext = NULL;
    }
    m_nStatus = CFGSTA_CLOSED;
    Init();
}

CConfigfile::~CConfigfile()
{
    Close();
}

#define PARSETEXT_BUFLEN 32768 //32*1024
int CConfigfile::Open(const char *szFilename)
{   
    Close();
    if(NULL == szFilename || 0 == szFilename[0])
        return CFGERR_INVALIDARG;
    
    int len = strlen(szFilename);
    m_pszFilename = new char[len+1];
    if(NULL == m_pszFilename)
        return CFGERR_OUTMEMORY;
    memcpy(m_pszFilename,szFilename,len);
    m_pszFilename[len] = 0;
        
    m_pfd = PR_Open(m_pszFilename,PR_RDONLY, 00666);
    if(NULL == m_pfd)
        return CFGERR_OPFILE;
    PR_Seek(m_pfd,0,PR_SEEK_SET);
    /////////////////////////////////////////////
    //read file
    char buf[PARSETEXT_BUFLEN];
    PRInt32 nData = 0;

    char * pszline = NULL; //terminated by '\0'
    PRInt32 nLineData = 0;
    
    PRInt32 i,iStart,nRead,nNeed,nDataCount;
    
    do{
        nNeed = PARSETEXT_BUFLEN;
        nRead = PR_Read(m_pfd,buf,nNeed);
        if(nRead > 0)
            nData = nRead;
        else
            nData = 0;

        //parse it
        i = 0;
        iStart = -1;
        while(i < nData){//parse data buf
            if(NULL == pszline){//new line
                //remove left blank
                while(i < nData) 
                    if(' ' == buf[i] || '\t' == buf[i]) 
                        i++;
                    else
                        break;
            }

            if(i >= nData)
                break;
            iStart = i;
            while(i < nData){//finding line end
                if('\n' == buf[i]){//found a line                   
                    //proc the line                 
                    nDataCount = i-iStart;
                    
                    char * pszMerge = MergeData(pszline,nLineData,buf+iStart,nDataCount,nLineData);
                    if(NULL == pszMerge && nLineData < 0){
                        if(pszline)
                            delete [] pszline;
                        return CFGERR_OUTMEMORY;
                    }
                    if(pszline)
                        delete [] pszline;
                    pszline = pszMerge;
                    
                    if(0 != PutLine(pszline,nLineData)){
                        if(pszline)
                            delete [] pszline;
                        return CFGERR_OUTMEMORY;
                    }

                    //reset
                    pszline = NULL;
                    nLineData = 0;
                    iStart = -1;
                    break;
                }
                i++;//jump data
            }//end while for finding line end
            i++;//jump '\n'
        }//end while for parsing data buf
        if(-1 != iStart){//valid data remained in buf, maybe get very long line or cross bufblocks line!
            nDataCount = nData-iStart;
            char * pszMerge = MergeData(pszline,nLineData,buf+iStart,nDataCount,nLineData);
            if(NULL == pszMerge && nLineData < 0){
                if(pszline)
                    delete [] pszline;
                return CFGERR_OUTMEMORY;
            }
            if(pszline){
                delete [] pszline;
                pszline = NULL;
            }
            pszline = pszMerge;
        }
        
    }while(nRead > 0);

    if(pszline){            
        if(0 != PutLine(pszline,nLineData)){
            if(pszline)
                delete [] pszline;
            return CFGERR_OUTMEMORY;
        }
        //reset
        pszline = NULL;
        nLineData = 0;
        iStart = -1;
    }           

    /////////////////////////////////////////////
    return 0;
}

/*
out [out] --  >=0 -- length of returned string, others -- failure
return value -- NULL -- failure, others -- success
*/
char * CConfigfile::MergeData(char *str0, int len0, char *str1, int len1, int &out)
{
    out = 0;//err or length of return string
    
    char *pszline = NULL;
    if(NULL == str0){
        if(NULL == str1){
            return NULL;
        }

        pszline = new char[len1+1];
        if(NULL == pszline){
            out = CFGERR_OUTMEMORY;
            return NULL;
        }

        memcpy(pszline,str1,len1);
        out = len1;
        pszline[out] = 0;
        return pszline;
    }
    else{
        if(NULL == str1){
            pszline = new char[len0+1];
            if(NULL == pszline){
                out = CFGERR_OUTMEMORY;
                return NULL;
            }

            memcpy(pszline,str0,len0);
            out = len0;
            pszline[out] = 0;
            return pszline;
        }

        pszline = new char[len0+len1+1];
        if(NULL == pszline){
            out = CFGERR_OUTMEMORY;
            return NULL;
        }
        memcpy(pszline,str0,len0);
        memcpy(pszline+len0,str1,len1);
        out = len0+len1;
        pszline[out] = 0;
        return pszline;
    }
    return NULL;
}

// 0 -- success; others -- failure
//pszline is created by method 'new', in other case it will bomb
int CConfigfile::PutLine(char *pszline,int len)
{
    //add line
    CFG_ITEM * pitem = new CFG_ITEM;
    if(NULL == pitem){
        return CFGERR_OUTMEMORY;
    }
    //adjust pszline data
    if(pszline && len > 0){
        if('\r' == pszline[len-1]){
            pszline[len-1] = 0;
            len--;
        }
        int i = len -1;
        while(i >= 0){
            if(' ' == pszline[i] || '\t' == pszline[i])
                pszline[i] = 0;
            else
                break;
            i--;
        }
        len = i+1;
    }
    pitem->m_nItemType = GetLineType(pszline);
    pitem->m_nOrgLineLen = len;
    pitem->m_pNext = NULL;
    pitem->m_pszOrgLine = pszline;

    m_pitemLast->m_pNext = pitem;
    m_pitemLast = pitem;

    return 0;
}

CConfigfile::CFG_ITEM_TYPE CConfigfile::GetLineType(char * pszline)
{
    if(NULL == pszline || 0 == pszline[0])
        return cfg_t_deaddata;
    switch(pszline[0]){
    case '#':
        return cfg_t_comment;
    case '[':
        {
            int i = 0;
            while(pszline[i]){
                if('[' == pszline[i]){
                    i++;
                    while(pszline[i]){
                        if(']' == pszline[i])
                            return cfg_t_sectiohead;
                        else
                            i++;
                    }
                    break;
                }
                else
                    i++;
            }
        }
        break;
    }
    return cfg_t_data;
}

int CConfigfile::Close()
{
    Save();
    Clear();
    return 0;
}

/*
return:
  >=0 -- length of returned buf; others -- failure; return buf terminated by '\0'
params:
  buf: pointer to output buf, if pointer to NULL, it returns the length of field value
  bufsize: size of buf;
*/
int  CConfigfile::ReadString(const char *szSection, const char *szFieldname, char * buf, int bufsize, const char * szDefault)
{
    CFG_ITEM * pSectionItem = GetSectionItem(szSection);
    const char * pszValue = GetFieldValue(pSectionItem,szFieldname);
    
    if(NULL == buf){//get length of value       
        if(pszValue && 0 != pszValue[0])
            return (strlen(pszValue)+1);        
        return 0;
    }

    buf[0] = 0;

    if(NULL == pszValue){
        //set default value
        if(NULL == szDefault)
            return 0;
        int len = strlen(szDefault);
        if(len < bufsize){
            memcpy(buf,szDefault,len);
            buf[len] = 0;
            return len;
        }
        else{
            return CFGERR_BUFSIZE;
        }
    }
    else{
        int len = strlen(pszValue);
        if(len < bufsize){
            memcpy(buf,pszValue,len);
            buf[len] = 0;
            return len;
        }
        else{
            return CFGERR_BUFSIZE;
        }
    }

    return CFGERR_UNKNOW;
}

/*
all data will be returned , include comment data line
return:
  >=0 -- length of returned buf; others -- failure; return buf terminated by '\0'
params:
  buf: pointer to output buf, if pointer to NULL, it returns the length of section data
  bufsize: size of buf;

  '\n' will be inserted bewteen lines
*/
int  CConfigfile::ReadSection(const char *szSection, char * buf, int bufsize)
{
    CFG_ITEM * pSectionItem = GetSectionItem(szSection);
    CFG_ITEM *pItem;
    if(NULL == pSectionItem)
        return 0;

    pItem = pSectionItem->m_pNext;
    
    if(NULL == buf){//get the size of section data      
        int nCount = 0;
        while(pItem && cfg_t_sectiohead !=pItem->m_nItemType){
            nCount += pItem->m_nOrgLineLen+1;//add a '\n'
            pItem = pItem->m_pNext;
        }
        if(nCount > 0)
            nCount++; //add a '\0'
        return nCount;
    }
    else{
        buf[0] = 0;
        int nCount = 0;
        while(pItem && cfg_t_sectiohead !=pItem->m_nItemType){
            if(bufsize-nCount > pItem->m_nOrgLineLen+1){// append '\n'
                memcpy(buf+nCount,pItem->m_pszOrgLine,pItem->m_nOrgLineLen);
                nCount += pItem->m_nOrgLineLen;
                buf[nCount] = '\n';
                buf[nCount+1] = 0;
                nCount += 1;
            }
            else
                return CFGERR_BUFSIZE;

            pItem = pItem->m_pNext;
        }

        return nCount;
    }
    
    return CFGERR_UNKNOW;
}

//return string can't be used during the operation on the same section+fieldname
const char * CConfigfile::ReadString(const char *szSection, const char *szFieldname)
{
    return GetFieldValue(GetSectionItem(szSection),szFieldname);
}

int CConfigfile::ReadInt(const char *szSection, const char *szFieldname, int iDefault)
{
    const char * pszValue = GetFieldValue(GetSectionItem(szSection),szFieldname);
    if(NULL == pszValue)
        return iDefault;
    
    return atoi(pszValue);  
}

long long CConfigfile::ReadInt64(const char *szSection, const char *szFieldname, long long iDefault)
{
    const char * pszValue = GetFieldValue(GetSectionItem(szSection),szFieldname);
    if(NULL == pszValue)
        return iDefault;
#ifdef WIN32
    return _strtoi64(pszValue, 0, 10);  
#else
    return strtoll(pszValue, 0, 10);  
#endif
}

/*
 return 0 -- success; others -- failure
*/
int CConfigfile::WriteString(const char *szSection, const char *szFieldname, char * szValue)
{
    if(NULL == szFieldname || 0 == szFieldname[0])
        return 0;

    CFG_ITEM * pSectionItem = GetSectionItem(szSection,true);
    if(NULL == pSectionItem){
        return CFGERR_OUTMEMORY;
    }

    int fieldlen = strlen(szFieldname);
    int valuelen = 0;
    if(szValue)
        valuelen = strlen(szValue);

    char * pNewLine = new char[fieldlen+valuelen+2];//add '='
    if(NULL == pNewLine)
        return CFGERR_OUTMEMORY;

    int nCount;
    memcpy(pNewLine,szFieldname,fieldlen);
    pNewLine[fieldlen] = '=';
    nCount = fieldlen+1;
    if(valuelen > 0){
        memcpy(pNewLine+nCount,szValue,valuelen);
        nCount += valuelen;
    }
    pNewLine[nCount] = 0;

    CFG_ITEM * pItem = GetFieldItem(pSectionItem,szFieldname);
    if(pItem){
        if(pItem->m_pszOrgLine)
            delete [] pItem->m_pszOrgLine;
        pItem->m_pszOrgLine = pNewLine;
        pItem->m_nOrgLineLen = nCount;
        m_nStatus = CFGSTA_UPDATE;
        return 0;
    }
    else{
        CFG_ITEM * pNewItem = new CFG_ITEM;
        if(NULL == pNewItem){
            delete [] pNewLine;
            return CFGERR_UNKNOW;
        }
        pNewItem->m_nItemType = cfg_t_data;
        pNewItem->m_nOrgLineLen = nCount;
        pNewItem->m_pszOrgLine = pNewLine;
        
        CFG_ITEM * pLastItemOfCurrSection = pSectionItem;
        while(pLastItemOfCurrSection->m_pNext){
            if(cfg_t_sectiohead == pLastItemOfCurrSection->m_pNext->m_nItemType)
                break;
            pLastItemOfCurrSection = pLastItemOfCurrSection->m_pNext;
        }

        //set last item
        if(m_pitemLast == pLastItemOfCurrSection)
            m_pitemLast = pNewItem;

        pNewItem->m_pNext = pLastItemOfCurrSection->m_pNext;
        pLastItemOfCurrSection->m_pNext = pNewItem;

        m_nStatus = CFGSTA_UPDATE;
        return 0;
    }
    return CFGERR_UNKNOW;
}


int CConfigfile::DeleteField(const char *szSection, const char *szFieldname)
{
    if(NULL == szFieldname || 0 == szFieldname[0])
        return 0;

    CFG_ITEM * pSectionItem = GetSectionItem(szSection,true);
    if(NULL == pSectionItem){
        return CFGERR_OUTMEMORY;
    }

    CFG_ITEM * pItem = GetFieldItem(pSectionItem,szFieldname);
    if(pItem){
        //获取当前节点的前一个节点
        CFG_ITEM * preItem = pSectionItem;
        for(CFG_ITEM * i = pSectionItem;i != NULL; i = i->m_pNext)
        {
            if(pItem == i->m_pNext)
            {
                preItem = i;
                break;
            }
        }
        preItem->m_pNext = preItem->m_pNext->m_pNext;//从链表中去掉当前节点
        //释放当前节点的空间
        if(pItem->m_pszOrgLine)
        {
            delete [] pItem->m_pszOrgLine;
        }
        delete [] pItem;

        m_nStatus = CFGSTA_UPDATE;
        return 0;
    }
    return CFGERR_UNKNOW;
}

//lines in szValue splited by '\n'
int CConfigfile::WriteSection(const char *szSection, char * szValue)
{   
    CFG_ITEM * pSectionItem = GetSectionItem(szSection,true);
    if(NULL == pSectionItem){
        return CFGERR_OUTMEMORY;
    }

    int nData;
    if(NULL == szValue || 0 == szValue[0])
        nData = 0;
    else
        nData = strlen(szValue);
    //parse it
    int i = 0;
    int iStart = -1;
    int nLineData;
    char *pszline = NULL;
    CFG_ITEM * pNewItemHead = NULL;
    CFG_ITEM * pNewItemLast = NULL;
    int err = 0;
    while(i < nData && 0 == err){//parse data buf
        //remove left blank
        while(i < nData){
            if(' ' == szValue[i] || '\t' == szValue[i]) 
                i++;
            else
                break;
        }

        if(i >= nData)
            break;

        iStart = i;
        while(i <= nData && 0 == err){//finding line end
            if('\n' == szValue[i] || 0 == szValue[i]){//found a line                    
                //proc the line                 
                nLineData = i-iStart;       
                
                pszline = new char[nLineData+1];
                if(NULL == pszline){
                    err = 1;
                    break;
                }
                
                memcpy(pszline,szValue+iStart,nLineData);
                pszline[nLineData] = 0;
                
                CFG_ITEM * pItem = new CFG_ITEM;
                if(NULL == pItem){
                    err = 1;
                    break;
                }
                pItem->m_nItemType = GetLineType(pszline);
                pItem->m_nOrgLineLen = nLineData;
                pItem->m_pszOrgLine = pszline;
                pItem->m_pNext = NULL;
                if(NULL == pNewItemHead){
                    pNewItemHead = pItem;
                    pNewItemLast = pItem;
                }
                else{
                    pNewItemLast->m_pNext = pItem;
                    pNewItemLast = pItem;
                }
                //reset
                pszline = NULL;
                break;
            }
            i++; //jump data
        }//end while for finding line end
        i++; //jump '\n'
    }//end while for parsing data buf

    if(0 != err){
        //free new data
        if(pszline)
            delete [] pszline;
        CFG_ITEM *pItem = pNewItemHead;
        while(pItem){
            CFG_ITEM *pNext = pItem->m_pNext;
            if(pItem->m_pszOrgLine)
                delete [] pItem->m_pszOrgLine;
            delete pItem;
            pItem = pNext;
        }
        return CFGERR_OUTMEMORY;
    }
    else{
        bool bNeedAdjustLastItem = false;
        //free old data and find next section head
        CFG_ITEM *pItem = pSectionItem->m_pNext;
        if(pSectionItem == m_pitemLast)
            bNeedAdjustLastItem = true;
        while(pItem){           
            if(cfg_t_sectiohead == pItem->m_nItemType)
                break;
            if(pItem == m_pitemLast)
                bNeedAdjustLastItem = true;
            CFG_ITEM *pNext = pItem->m_pNext;
            if(pItem->m_pszOrgLine)
                delete [] pItem->m_pszOrgLine;
            delete pItem;
            pItem = pNext;
        }
        //////////////////////////////////////////
        if(NULL == pNewItemHead){
            pSectionItem->m_pNext = pItem;
            if(bNeedAdjustLastItem){
                if(pItem)//in fact, if true == bNeedAdjustLastItem, pItem must be NULL, or the code is wrong
                    m_pitemLast = pItem;
                else
                    m_pitemLast = pSectionItem;
            }
        }
        else{//in this case, pNewItemLast must be !(NULL)
            pSectionItem->m_pNext = pNewItemHead;
            pNewItemLast->m_pNext = pItem;
            if(bNeedAdjustLastItem)
                m_pitemLast = pNewItemLast;
        }
        m_nStatus = CFGSTA_UPDATE;
        return 0;
    }

    return CFGERR_UNKNOW;
}

int CConfigfile::WriteInt(const char *szSection, const char *szFieldname, int iValue)
{
    char str[64];
    sprintf(str,"%d",iValue);
    return WriteString(szSection,szFieldname,str);
}

int CConfigfile::WriteInt64(const char *szSection, const char *szFieldname, long long iValue)
{
    char str[64];
    sprintf(str,"%lld",iValue);
    return WriteString(szSection,szFieldname,str);
}


// 0 -- success; 1 -- data is not changed; <0 -- failure
int CConfigfile::Save()
{
    if(m_pfd){
        PR_Close(m_pfd);
        m_pfd = NULL;
    }
    if(CFGSTA_UPDATE != m_nStatus || NULL == m_pszFilename)
        return 0;
    m_nStatus = CFGSTA_OPEN;
    PRFileInfo fi;
    if(PR_FAILURE == PR_GetFileInfo(m_pszFilename,&fi))
        fi.size = 0;
    if (0) {
        if(0 != fi.size){
            char bkfilename[512] = "";
            if(strlen(m_pszFilename)+5 > 512)
                return CFGERR_OUTMEMORY;
            strcpy(bkfilename,m_pszFilename);
            strcat(bkfilename,".bak");

            PR_Delete(bkfilename);
            if(PR_FAILURE == PR_Rename(m_pszFilename,bkfilename)){
                return CFGERR_BKFILE;
            }
        }
    }

    PRFileDesc *fd = PR_Open(m_pszFilename,PR_RDWR | PR_CREATE_FILE | PR_TRUNCATE, 00666);
    if(NULL == fd)
        return CFGERR_OPFILE;
    PR_Seek(fd,0,PR_SEEK_SET);

    CFG_ITEM * pItem = m_itemHead.m_pNext;
#ifdef WIN32
    const char lf[] = "\r\n";   
#else
    const char lf[] = "\n";
#endif
    int lflen = strlen(lf);
    while(pItem){
        if(pItem->m_pszOrgLine){
            PR_Write(fd,pItem->m_pszOrgLine,pItem->m_nOrgLineLen);
        }
        PR_Write(fd,lf,lflen);
        pItem = pItem->m_pNext;
    }

    PR_Close(fd);
    //////////////////
    return 0;
}

CConfigfile::CFG_ITEM * CConfigfile::GetSectionItem(const char * szSection,bool bCreate)
{
    if(NULL == szSection || 0 == szSection[0])
        return &m_itemHead;

    CFG_ITEM * pSectionItem = m_itemHead.m_pNext;

    while(pSectionItem){
        if(cfg_t_sectiohead == pSectionItem->m_nItemType){
            //check it
            char *pstart = NULL, *pend = NULL;
            char *pszline = pSectionItem->m_pszOrgLine;
            int i = 0;
            while(pszline[i]){
                if('[' == pszline[i]){
                    i++;
                    pstart = pszline+i;
                    while(pszline[i]){
                        if(']' == pszline[i]){
                            i--;
                            pend = pszline+i;
                            break;
                        }
                        else
                            i++;
                    }
                    break;
                }
                else
                    i++;
            }
            if(pstart && pend){
                while(pstart <= pend){
                    if(' ' == *pstart || '\t' == *pstart)
                        pstart++;
                    else
                        break;
                }
                while(pstart <= pend){
                    if(' ' == *pend || '\t' == *pend)
                        pend--;
                    else
                        break;
                }
                if(pstart <= pend){
                    //compare it with szSection
                    if(0 == iEqual(szSection,strlen(szSection),pstart,pend-pstart+1))
                        break;
                }
            }
        }
        pSectionItem = pSectionItem->m_pNext;
    }

    if(NULL == pSectionItem && bCreate){
        //add section
        int sectionlen = strlen(szSection);
        char *pszline = new char[sectionlen+3];     
        if(NULL == pszline)
            return NULL;
        int nLineData = 0;
        pszline[nLineData++] = '[';
        memcpy(pszline+nLineData,szSection,sectionlen);
        nLineData += sectionlen;
        pszline[nLineData++] = ']';
        pszline[nLineData] = 0;
        CFG_ITEM *pItem = new CFG_ITEM;
        if(NULL == pItem){
            delete [] pszline;
            return NULL;
        }
        pItem->m_nItemType = cfg_t_sectiohead;
        pItem->m_nOrgLineLen = nLineData;
        pItem->m_pNext = NULL;
        pItem->m_pszOrgLine = pszline;
        m_pitemLast->m_pNext = pItem;
        m_pitemLast = pItem;
        pSectionItem = pItem;
    }
    return pSectionItem;
}

CConfigfile::CFG_ITEM * CConfigfile::GetFieldItem(CConfigfile::CFG_ITEM * pSectionItem, 
                                                  const char * szFieldname)
{
    if(NULL == pSectionItem || NULL == szFieldname || 0 == szFieldname[0])
        return NULL;
    CFG_ITEM *pItem = pSectionItem->m_pNext;
    char *pstart,*pend;
    while(pItem && cfg_t_sectiohead != pItem->m_nItemType){
        if(cfg_t_data == pItem->m_nItemType){
            if(pItem->m_pszOrgLine && pItem->m_pszOrgLine[0]){              
                char *pszline = pItem->m_pszOrgLine;

                pstart = pItem->m_pszOrgLine;
                pend = NULL;
                //find '='
                while(*pszline){
                    if('=' == *pszline){
                        pend = pszline-1;
                        break;
                    }
                    else
                        pszline++;
                }
                if(pstart && pend){
                    //remove left blank
                    while(pstart <= pend){
                        if(' ' == *pstart || '\t' == *pstart)
                            pstart++;
                        else
                            break;
                    }
                    while(pstart <= pend){
                        if(' ' == *pend || '\t' == *pend)
                            pend--;
                        else
                            break;
                    }
                    if(pstart <= pend){
                        if(0 == iEqual(szFieldname,strlen(szFieldname),pstart,pend-pstart+1))
                            return pItem;
                    }
                }
            }
        }
        pItem = pItem->m_pNext;
    }
    return NULL;
}

const char * CConfigfile::GetFieldValue(CConfigfile::CFG_ITEM * pSectionItem, const char * szFieldname)
{
    CFG_ITEM *pItem = GetFieldItem(pSectionItem,szFieldname);
    if(pItem && pItem->m_pszOrgLine){
        char *pszline = pItem->m_pszOrgLine;
        while(*pszline){
            if('=' == *pszline){
                pszline++;
                while(*pszline){
                    if(' ' == *pszline || '\t' == *pszline)
                        pszline++;
                    else
                        return pszline;
                }
                break;
            }
            else
                pszline++;
        }
    }
    return NULL;
}

//if equal, then returns 0, else returns others
int CConfigfile::iEqual(const char *str0, int len0, const char *str1, int len1)
{
    if(NULL == str0 && NULL == str1)
        return 0;
    if(NULL != str0 && NULL != str1 && len0 == len1){
        int i = 0;
        while(i < len0){
            char c0,c1;
            c0 = str0[i];
            c1 = str1[i];
            //to lower case
            if(c0 <= 90 && c0 >= 65)
                c0 += 32;

            if(c1 <= 90 && c1 >= 65)
                c1 += 32;
            if(c0 != c1)
                return -1;
            i++;
        }
        return 0;
    }
    
    return -1;
}

//szSection[in] -- section name, if szSection == NULL or "", it will return the begin data
//if return NULL, found nothing; others -- found it
//don't modify/delete the return handle
CFG_SECTION_HANDLE CConfigfile::GetSectionHandle(const char *szSection)
{
    return (CFG_SECTION_HANDLE)GetSectionItem(szSection,false); 
}

//prev[in] -- get from FindFirstSection(...)
//buf[out] -- return the next section name from prev, terminated by '\0';
//bufsize[in] -- the length of szSection
//if success to find next section, it returns !(NULL) ;
//if failure, it returns NULL
//don't modify/delete the return handle
CFG_SECTION_HANDLE CConfigfile::FindNextSection(CFG_SECTION_HANDLE prev, char * buf, int bufsize)
{
    if(NULL == prev || NULL == buf || bufsize <= 0)
        return (CFG_SECTION_HANDLE)NULL;
    
    CFG_ITEM * pSectionItem = (CFG_ITEM * )prev;
    pSectionItem = pSectionItem->m_pNext;

    while(pSectionItem){
        if(cfg_t_sectiohead == pSectionItem->m_nItemType){
            //check it
            char *pstart = NULL, *pend = NULL;
            char *pszline = pSectionItem->m_pszOrgLine;
            int i = 0;
            while(pszline[i]){
                if('[' == pszline[i]){
                    i++;
                    pstart = pszline+i;
                    while(pszline[i]){
                        if(']' == pszline[i]){
                            i--;
                            pend = pszline+i;
                            break;
                        }
                        else
                            i++;
                    }
                    break;
                }
                else
                    i++;
            }
            if(pstart && pend){
                while(pstart <= pend){
                    if(' ' == *pstart || '\t' == *pstart)
                        pstart++;
                    else
                        break;
                }
                while(pstart <= pend){
                    if(' ' == *pend || '\t' == *pend)
                        pend--;
                    else
                        break;
                }
                int len = pend - pstart+1;
                if(len > 0 && len < bufsize){
                    //copy it
                    memcpy(buf,pstart,len);
                    buf[len] = 0;
                    return (CFG_SECTION_HANDLE)pSectionItem;
                }
            }
            break;
        }//found section
        pSectionItem = pSectionItem->m_pNext;
    }
    
    return (CFG_SECTION_HANDLE)NULL;
}

int CConfigfile::ReadStream(char *pszStream)
{
    char buf[8192];
    PRInt32 nData = 0;
    PRInt32 nTotalCount;

    char * pszline = NULL; //terminated by '\0'
    PRInt32 nLineData = 0;
    
    PRInt32 i,iStart,nRead,nNeed,nDataCount;
    
    strncpy(buf, pszStream, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    nTotalCount = strlen(buf);
    do{
        //fill data buf
        nNeed = nTotalCount-nData;
        if (nNeed > 0)
            nData = nNeed;
        else
            nData = 0;
        nRead = nData;
        //
        //modifed by liqy
        //nRead = PR_Read(m_pfd,buf+nData,nNeed);
        //if(nRead > 0)
        //    nData = nRead;
        //else
        //    nData = 0;
        //

        //parse it
        i = 0;
        iStart = -1;
        while(i < nData){//parse data buf
            if(NULL == pszline){//new line
                //remove left blank
                while(i < nData) 
                    if(' ' == buf[i] || '\t' == buf[i]) 
                        i++;
                    else
                        break;
            }

            if(i >= nData)
                break;
            iStart = i;
            while(i < nData){//finding line end
                if('\n' == buf[i]){//found a line                   
                    //proc the line                 
                    nDataCount = i-iStart;
                    
                    char * pszMerge = MergeData(pszline,nLineData,buf+iStart,nDataCount,nLineData);
                    if(NULL == pszMerge && nLineData < 0){
                        if(pszline)
                            delete [] pszline;
                        return CFGERR_OUTMEMORY;
                    }
                    if(pszline)
                        delete [] pszline;
                    pszline = pszMerge;
                    
                    if(0 != PutLine(pszline,nLineData)){
                        if(pszline)
                            delete [] pszline;
                        return CFGERR_OUTMEMORY;
                    }

                    //reset
                    pszline = NULL;
                    nLineData = 0;
                    iStart = -1;
                    break;
                }
                i++;//jump data
            }//end while for finding line end
            i++;//jump '\n'
        }//end while for parsing data buf
        if(-1 != iStart){//valid data remained in buf, maybe get very long line or cross bufblocks line!
            nDataCount = nData-iStart;
            char * pszMerge = MergeData(pszline,nLineData,buf+iStart,nDataCount,nLineData);
            if(NULL == pszMerge && nLineData < 0){
                if(pszline)
                    delete [] pszline;
                return CFGERR_OUTMEMORY;
            }
            if(pszline){
                delete [] pszline;
                pszline = NULL;
            }
            pszline = pszMerge;
        }
        
    }while(nRead > 0);

    if(pszline){            
        if(0 != PutLine(pszline,nLineData)){
            if(pszline)
                delete [] pszline;
            return CFGERR_OUTMEMORY;
        }
        //reset
        pszline = NULL;
        nLineData = 0;
        iStart = -1;
    }           

    /////////////////////////////////////////////
    return 0;
}
