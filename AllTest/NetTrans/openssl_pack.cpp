#include "openssl_pack.h"
#include "server_log.h"

openssl_pack::openssl_pack(void)
{
	ctx = NULL;
	ssl =NULL;
	meth = NULL;

}


openssl_pack::~openssl_pack(void)
{

}

int openssl_pack::InitConnect(sbuff *sb){
	
	int code,ret;
	unsigned long ulErr=0;
	char szErrMsg[1024] = {0};

	SLogErrorWrite(APLOG_MARK, APLOG_INFO, NULL,"InitConnect Begin");
	SSL_load_error_strings();     /*为打印调试信息作准备*/
#ifndef _WIN32
	SSL_library_init();
	if((ret = ERR_get_error()) !=0) goto FAIL;
#endif
	OpenSSL_add_ssl_algorithms(); /*初始化*/
	if((ret = ERR_get_error()) !=0) goto FAIL;

	meth = (SSL_METHOD *)TLSv1_client_method(); /*采用什么协议(SSLv2/SSLv3/TLSv1)在此指定*/
	if((ret = ERR_get_error()) !=0) goto FAIL;

	ctx = SSL_CTX_new (meth); 
	if((ret = ERR_get_error()) !=0) goto FAIL;


	/*构建随机数生成机制,WIN32平台必需*/
	srand( (unsigned)time( NULL ) );
	for( int i = 0;   i < 100;i++ )
		seed_int[i] = rand();
	RAND_seed(seed_int, sizeof(seed_int));

	ssl = SSL_new (ctx); 
	if((ret = ERR_get_error()) !=0) goto FAIL;

	code = SSL_set_fd (ssl, sb->fd);
	if((ret = ERR_get_error()) !=0) goto FAIL;

	for (int i=0;i<6;i++)
	{
		if ((code = SSL_connect (ssl)) == 1)
		{
			break;
		}
		if((ret = ERR_get_error()) !=0) goto FAIL;
#if defined(_WIN32)
		Sleep(400);
#else
		usleep(400000);
#endif
	}
	
	return (code==-1)?code:0;
FAIL:
	do 
	{
		ERR_error_string(ret ,szErrMsg);
		SLogErrorWrite(APLOG_MARK, APLOG_ERR, NULL,"[%s]",szErrMsg);
	} while ((ret = ERR_get_error()) != 0);
	return -1;
}

int openssl_pack::InitAccept(sbuff *sb){

	int code,ret;
	unsigned long ulErr=0;
	char szErrMsg[1024] = {0};

	SLogErrorWrite(APLOG_MARK, APLOG_INFO, NULL,"InitAccept begin");
	SSL_load_error_strings();            /*为打印调试信息作准备*/
#ifndef _WIN32
	SSL_library_init();
	if((ret = ERR_get_error()) !=0) goto FAIL;
#endif
	
#ifdef _WIN32
	/*windows环境下获取证书路径*/
	char CERTF[100]   = {0};
	char KEYF[100]    = {0};
	char CACERT[100]  = {0};
	char ExeFile[100] = {0};
	char CurrentPath[100] = {0};
	char CurrentPathTemp[100] = {0};

	GetModuleFileName(NULL,ExeFile,100);
	strncpy(CurrentPath,ExeFile,strlen(ExeFile)-15);//获取server所在路径	
	strcpy(CurrentPathTemp,CurrentPath);

	strcat(CurrentPathTemp,"cacert.pem");//证书与CA路径
	strcpy(CERTF,CurrentPathTemp);
	strcpy(CACERT,CERTF);

	strcat(CurrentPath,"privkey.pem");//私钥路径
	strcpy(KEYF,CurrentPath);
#else
#define CERTF   "cacert.pem" /*服务端的证书(需经CA签名)*/
#define KEYF   "privkey.pem"  /*服务端的私钥(建议加密存储)*/
#define CACERT "cacert.pem" /*CA 的证书*/ 
#endif

	OpenSSL_add_ssl_algorithms();        /*初始化*/
	if((ret = ERR_get_error()) !=0) goto FAIL;

	meth = (SSL_METHOD *)TLSv1_server_method();  /*采用什么协议(SSLv2/SSLv3/TLSv1)在此指定*/
	if((ret = ERR_get_error()) !=0) goto FAIL;

	ctx = SSL_CTX_new (meth);
	if((ret = ERR_get_error()) !=0) goto FAIL;

	SSL_CTX_set_verify(ctx,SSL_VERIFY_NONE,NULL);   /*验证与否*/
	if((ret = ERR_get_error()) !=0) goto FAIL;

	SSL_CTX_use_certificate_file(ctx, CERTF, SSL_FILETYPE_PEM);
	if((ret = ERR_get_error()) !=0) goto FAIL;

	SSL_CTX_use_PrivateKey_file(ctx, KEYF, SSL_FILETYPE_PEM);
	if((ret = ERR_get_error()) !=0) goto FAIL;

	SSL_CTX_check_private_key(ctx);
	if((ret = ERR_get_error()) !=0) goto FAIL;

	ssl = SSL_new (ctx);    
	if((ret = ERR_get_error()) !=0) goto FAIL;

	code = SSL_set_fd (ssl, sb->fd);
	if((ret = ERR_get_error()) !=0) goto FAIL;

	for (int i=0;i<6;i++)
	{
		if ((code=SSL_accept (ssl))==1)
		{
			break;
		}
		if((ret = ERR_get_error()) !=0) goto FAIL;
#if defined(_WIN32)
		Sleep(400);
#else
		usleep(400000);
#endif
	}
	return code<0?-1:0;
FAIL:
	do 
	{
		ERR_error_string(ret ,szErrMsg);
		SLogErrorWrite(APLOG_MARK, APLOG_ERR, NULL,"[%s]",szErrMsg);
	} while ((ret = ERR_get_error()) != 0);
	return -1;
}


int openssl_pack::Send(void* content,unsigned int size){

	//split to block (1024bytes)
	int blockSize=1024;
	int blockCount= size/blockSize+1;
	int lastBlockSize = size%1024;
	int SndCount=0;
	unsigned long ulErr=0;
	char szErrMsg[1024] = {0};
	int ret=0;

	for(;SndCount<blockCount;SndCount++){
		ret = SSL_write (ssl, (char *)content+blockSize*SndCount,SndCount==(blockCount-1)?lastBlockSize:blockSize);
		if (ret !=  (SndCount==(blockCount-1)?lastBlockSize:blockSize))
		{
			if((ret = ERR_get_error()) !=0) goto FAIL;
		}
	}
	return 0;
FAIL:
	do 
	{
		ERR_error_string(ret ,szErrMsg);
		SLogErrorWrite(APLOG_MARK, APLOG_ERR, NULL,"[%s]",szErrMsg);
	} while ((ret = ERR_get_error()) != 0);
	return -1;
}


int openssl_pack::Recv(void* content,unsigned int buffsize,unsigned int* nRecved){

	//split to block (1024bytes)
	int blockSize=1024;
	int blockCount= buffsize/blockSize+1;
	int lastBlockSize = buffsize%1024;
	int RevCount=0;

	unsigned long ulErr=0;
	char szErrMsg[1024] = {0};
	int ret=0;

	*nRecved=0;
	for(;RevCount<blockCount;RevCount++){
		ret = SSL_read (ssl, (char *)content+blockSize*RevCount,RevCount==(blockCount-1)?lastBlockSize:blockSize);
			//*nRecved += (RevCount==(blockCount-1))?lastBlockSize:blockSize;
		if (ret != (RevCount==(blockCount-1)?lastBlockSize:blockSize))
		{
			if((ret = ERR_get_error()) !=0) goto FAIL;
		}
		*nRecved+=ret;
	}
	return 0;
FAIL:
	do 
	{
		ERR_error_string(ret ,szErrMsg);
		SLogErrorWrite(APLOG_MARK, APLOG_ERR, NULL,"[%s]",szErrMsg);
	} while ((ret = ERR_get_error()) != 0);
	return -1;
}


void openssl_pack::UnInit (int flag){
	if (ssl!=NULL)
	{
		SSL_free (ssl);
	}
	if (ctx != NULL)
	{
		SSL_CTX_free (ctx);
	}
	
#ifndef _WIN32
	EVP_cleanup();                 //For EVP
	CRYPTO_cleanup_all_ex_data();  //generic
	ERR_remove_state(0);           //for ERR
	ERR_free_strings();            //for ERR
#endif
}

int openssl_pack::Flush(){
	return 0;
}

void openssl_pack::log(char *filename,char *format,...){
	char buffer[1024];
	memset(buffer,0,sizeof(buffer));
	FILE *pfile=fopen(filename,"a+");

	va_list argptr;
	va_start(argptr, format);

#if defined(_WIN32)
	_vsnprintf(buffer, 1024, format, argptr);//widows	
#else
	vsnprintf(buffer, 1024, format, argptr);//linux
#endif
	va_end(argptr);

	//fprintf(pfile,"[%s %s] [%s:%s:%d] %s",__DATE__,__TIME__,__FILE__,__FUNCTION__,__LINE__,buffer);
	fprintf(pfile,"[%s %s]%s\n",__DATE__,__TIME__,buffer);
	fclose(pfile);
}
void openssl_pack::logclear(char *filename){
	FILE *pfile=fopen(filename,"w+");
	fclose(pfile);
}
