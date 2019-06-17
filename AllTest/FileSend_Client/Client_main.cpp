#define _WINSOCK_DEPRECATED_NO_WARNINGS 1
#define _CRT_SECURE_NO_WARNINGS 1
#include<winsock2.h>
#include<stdlib.h>
#include<iostream>
#include <time.h>
#pragma comment(lib, "ws2_32.lib")
using namespace std;


#define  BUFFERSIZE 1048576


struct FileName {//存储形式："文件名.扩展名"
	char Fname[64];
	int len;
};
class FileSender
{
private:
	FILE * fp;
	SOCKET sock;
	sockaddr_in addr;
	FileName fn;
	char *temp;
public:
	FileSender()
	{
		sock = socket(AF_INET, SOCK_STREAM, 0);
		temp = (char *)malloc(BUFFERSIZE);
	}
	int Connect(const char *ip, int port)
	{
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.S_un.S_addr = inet_addr(ip);

		if (connect(sock, (sockaddr*)&addr, sizeof(addr)) != 0)
		{
			cout << "failed.." << endl;
		}
		return 1;
	}
	int openFile(const char *path)
	{
		char name[32], ext[16];
		_splitpath(path, NULL, NULL, name, ext);
		strcat_s(name, 32, ext);
		strcpy_s(fn.Fname, 32, name);
		fn.len = strlen(fn.Fname);
		fp = fopen(path, "rb");
		return 0;
	}
	int SendFile(const char *path)
	{
		openFile(path);
		send(sock, (char*)&fn, sizeof(FileName),0);
		fseek(fp, 0, SEEK_END);
		unsigned long long siz = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		send(sock, (CHAR*)&siz, sizeof(siz),0);
		siz = siz / BUFFERSIZE;
		long long index=0;
		int num;
		while (1)
		{
			num=fread(temp, 1, BUFFERSIZE, fp);
			if (num == 0)
				break;
			send(sock, temp, num, 0);
			index++;
			if(siz != 0)
			cout << (int)index*100/siz<<"%" << num << endl;
		}
		cout << "Successfully send" << endl;
		return 0;
	}

	int ShutdownSocket()
	{
		closesocket(sock);
		return 0;
	}

};

void SendAllFileList(const char *dir)
{
	HANDLE hFind;
	WIN32_FIND_DATA findData;
	//LARGE_INTEGER size;
	hFind = FindFirstFile(dir, &findData);
	char szTemp[260] = {0};
	FileSender fs;
	if(!fs.Connect("127.0.0.1", 3000)){
		cout<<"connect failed"<<endl;
	}
	do 
	{
		if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0)
			continue;
		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)    // 是否是目录 
		{
			cout << findData.cFileName << "\t<dir>\n";
			continue;
		}
		cout<<findData.cFileName<<endl;  //发送文件的路径
		_snprintf(szTemp,sizeof(szTemp),"D:\\1\\%s",findData.cFileName);


		fs.SendFile(szTemp);

	} while (FindNextFile(hFind, &findData));

}
int main()
{ 
	time_t starttime= time(NULL);
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	char dir[100] = "D:\\1\\";
	strcat(dir, "*.*");    // 需要在目录后面加上*.*表示所有文件/目录
	//SendFile(dir);
	SendAllFileList(dir);
	cout<<"用时： "<<time(NULL) - starttime<<endl;
	system("pause");
}
