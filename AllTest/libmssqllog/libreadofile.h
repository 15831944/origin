

#define MAX_DISK_NUMBER                 4
#define MAX_ORACLE_FILE_NUMBER          10000
#define MAX_ARCHIVE_FILE_NUMBER         20000
#define WAVETOP_ORACLE_FILE_ERROR       -1
#define WAVETOP_ORACLE_FILE_OK          0
#define TRAN_REDO_VALUE_MAX            (30 * 1024)
#define TRAN_CONCTL_VALUE_MAX          (16 * 1024)

#define INVALID_ORACLE_FILE_FD        -101
#define ORACLE_FILE_NUMBER_BEGIN         88

#define IS_ORACLE_FILE(fd)         (((fd) >= ORACLE_FILE_NUMBER_BEGIN) && ((fd) < MAX_ORACLE_FILE_NUMBER ))
#define IS_ARCHIVE_FILE(fd)        (((fd) >= MAX_ORACLE_FILE_NUMBER)   && ((fd) < MAX_ARCHIVE_FILE_NUMBER))
#define IS_ASM_FILENAME(name)      (name[0] == '+')


//////////////////////////////////////////////////////////////////////////////////////
/////////////////////////         文件句柄链表       /////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
typedef struct File_Handle_Info {
    short nIsNull;
    int nFileType;
    void *pHandle;
	char filename[240];
}stFile_Handle_Info;


//////////////////////////////////////////////////////////////////////////////////////
/////////////////////////      ASM磁盘组信息结构体    ////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
typedef struct Oracle_Disk_Info {
    unsigned int nGroupNO;
    unsigned int nDiskNO;
    char szPath[256];
    char szDeviceName[256];
    void *pDeviceFD;
    struct Oracle_Disk_Info *pNext;
}stOracle_Disk_Info;

//////////////////////////////////////////////////////////////////////////////////////
/////////////////////////    AMS分配单元信息结构体    ////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
typedef struct {
    unsigned short nDiskCount;
    unsigned int nAuNO[MAX_DISK_NUMBER];
    stOracle_Disk_Info *pDisk[MAX_DISK_NUMBER];    
}stAllocation_Unit;

//////////////////////////////////////////////////////////////////////////////////////
/////////////////////////    oracle文件信息结构体    /////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
typedef struct Oracle_File_Info {
    unsigned int nIsNull;
    unsigned int nGroupNO;
    unsigned int nAUSize;
    unsigned int nFileNO;
    unsigned int nBlockSize;
    unsigned int nBlockCount;
    unsigned int nFileSize;
    unsigned int nAUCount;
    unsigned int nStrpwd;
    unsigned int nStrpSize;
    unsigned long long nOffset;
    stAllocation_Unit *pstAU;
    unsigned int nHandle;
    char *pszFileName;
    char szFileType[64];
    void *stmt;
}stOracle_File_Info;

//////////////////////////////////////////////////////////////////////////////////////
/////////////////////////     读取oracle文件抽象类      //////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
class COracleFile 
{
    public:

        virtual ~COracleFile() = 0;
        virtual int Open(char *pszFilename) = 0;
        virtual int Read(int nFd, char *pData, unsigned int nSize) = 0;
        virtual int Seek(int nFd, unsigned long long nOffset) = 0;
        virtual int Write(int nFd, char *pData, unsigned int nSize) = 0;
        virtual void Close(int nFd) =0;
};

//////////////////////////////////////////////////////////////////////////////////////
/////////////////////////    从普通文件系统中读取    /////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
class CNormalFs 
{
    public:
        CNormalFs();
        ~CNormalFs();
		int Init(void);
        int Open(char *pszFilename);
        int Read(int nFd, char *pData, unsigned int nSize);
        int Seek(int nFd, unsigned long long nOffset);
        int Write(int nFd, char *pData, unsigned int nSize);
        void Close(int nFd);

    private:

        stFile_Handle_Info *m_stFiles;
		void *m_pNormalLck;
		int m_inited_flag;
};

//////////////////////////////////////////////////////////////////////////////////////
/////////////////////////    从裸设备中读取      /////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
class CRawAsm: public COracleFile 
{
    public:

        CRawAsm(void *pOracleHandle);
        ~CRawAsm();
        int Open(char *pszFilename);
        int Read(int nFd, char *pData, unsigned int nSize);
        int Seek(int nFd, unsigned long long nOffset);
        int Write(int nFd, char *pData, unsigned int nSize);
        void Close(int nFd);

    private:

        int GetAsmDiskNo();
        void GetAsmDevice(char *pszKey, char *pszValue);
        int GetAsmFileNo(char *pFileName, int *nGroupNO, int *nFileNO);
        void GetAsmFileOffset(stOracle_File_Info *pFile, unsigned long long nOffset, int *nAuNumber, int *nAuOffset, int *nSize);
        int GetAsmAuList(char *pFileName, int nGroupNO, int nFileNO);
        int ReadDevice(int nFD, stOracle_Disk_Info *pDisk, unsigned int au_size, unsigned long long nOffset, char *pData, unsigned int nSize);

    private:

        void *m_pOracleHandle;
        void *m_pLock;
        int m_nGroupCount;
        CNormalFs *m_pCNFS;
        stOracle_Disk_Info *m_pDiskInfo;
        stOracle_Disk_Info *m_pDevList;
        stOracle_File_Info *m_stFiles;

};

//////////////////////////////////////////////////////////////////////////////////////
/////////////////////////    从数据库中读取    ///////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
class CDbAsm: public COracleFile 
//class CDbAsm
{
    public:
        CDbAsm(void *pOracleHandle);
        ~CDbAsm();
		int Init();
        int Open(char *pszFilename);
        int Read(int nFd, char *pData, unsigned int nSize);
        int Seek(int nFd, unsigned long long nOffset);
		int SeekBlock(int nFd, unsigned long long blockId);
		int ReadBlock(int nFd, char *pData, unsigned int nBlockNum);
        int Write(int nFd, char *pData, unsigned int nSize);
        void Close(int nFd);
    private:
                                
        /*获取文件属性*/
        int DbAsmGetFileAttr(char *pszFilename, char *szfType, int *pszBlockCount, int *pszBlockSize );

        /*保存文件信息*/
        int DbAsmSaveFileAttr(stOracle_File_Info  *szFileInfo);

        /*读数据库文件信息*/
        int DbAsmReadFile(stOracle_File_Info  *szFileInfo,char *pData, unsigned int nSize);

        /*设置文件头信息*/
        int SetAsmFileHeadInfo(stOracle_File_Info  *szFileInfo, char* pszInfo);

        void *m_OracleHandle;       /* Oracle Handle for Get Data */

        void *m_pLck;
        int m_inited_flag;
        stOracle_File_Info m_szFileAsm[MAX_ORACLE_FILE_NUMBER];
		CNormalFs *m_NormalFs;
};


//////////////////////////////////////////////////
////////// TEMP INFO , NEED TO READ CONF FILE ////
//////////////////////////////////////////////////

#define ASM_NAME  "asmasm"
#define ASM_USER  "sys"
#define ASM_PSWD  "HUbin1122"
#define ASM_MODE  OCI_SYSASM

