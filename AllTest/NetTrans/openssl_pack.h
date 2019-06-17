#ifndef __OPENSSL_PACK__
#define __OPENSSL_PACK__ 1
#include "wnetfd.h"

#include "openssl/rsa.h"      
#include "openssl/crypto.h"
#include "openssl/x509.h"
#include "openssl/pem.h"
#include "openssl/ssl.h"
#include "openssl/err.h"
#include "openssl/rand.h"

class openssl_pack
{
public:
	openssl_pack(void);
	~openssl_pack(void);

	//************************************
	// Access:    public 
	// Returns:   int 
	// Qualifier:
	// Parameter: sbuff * sb
	// Parameter: unsigned long flag
	//description:
	//openssl 利用sbuff接头体里的套接字fd，初始化连接动作。
	//return value:if ==0 meams sucess ,if == -1 means fail.
	//befor use this function ,create socket fd first.
	//************************************
	int InitAccept(sbuff *sb);
	int InitConnect(sbuff *sb);

	//************************************
	// Access:    public 
	// Returns:   int
	// Qualifier:
	// Parameter: void * content
	// Parameter: unsigned int size
	//description: Send  Data (content),length of data is (size);
	//return value:if ==0 meams sucess ,if == -1 means fail.
	//befor use this function ,please InitAccept() or InitConnect();
	//************************************
	int Send(void* content,unsigned int size);

	//************************************
	// Access:    public 
	// Returns:   int
	// Qualifier:
	// Parameter: void * content
	// Parameter: unsigned int buffsize
	// Parameter: unsigned int * nRecved
	//description: Receive  Data (content),length of data is (buffsize). the length of actually Receive is (nRecved)
	//return value:if ==0 meams sucess ,if == -1 means fail.
	//befor use this function ,please InitAccept() or InitConnect();
	//************************************
	int Recv(void* content,unsigned int buffsize,unsigned int* nRecved);

	//************************************
	// Method:    Init
	// FullName:  openssl_pack::Init
	// Access:    public 
	// Returns:   void
	// Qualifier:
	// Parameter: int flag
	//************************************
	void UnInit (int flag);

	int Flush();

	static void log(char *filename,char *format,...);
	static void logclear(char *filename);
private:
	SSL_CTX* ctx;
	SSL*     ssl;
	SSL_METHOD *meth;

	int       seed_int[100]; /*存放随机序列*/

};
#endif

