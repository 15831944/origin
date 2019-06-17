#define _CRT_SECURE_NO_WARNINGS 1
#include <winsock2.h> 
#include <stdio.h> 
#include <windows.h>
#include <string.h>
#pragma comment(lib,"ws2_32.lib")
#include<iostream>
using namespace std;

#define  BUFFERSIZE 1048576


struct FileName {//用来存储和传输文件名扩展名的结构体
	char Fname[64];
	int len;
};
class FileReciever {
private:
	int server;
	int client;
	sockaddr_in sa;
	sockaddr_in ca;
	char *buff;
	FILE *fp;
	FileName fn;
public:
	FileReciever()
	{
		server = socket(AF_INET, SOCK_STREAM, 0);
		client = socket(AF_INET, SOCK_STREAM, 0);
		buff = (char *)malloc(BUFFERSIZE);
	}
	int Listen(int port)
	{
		sa.sin_family = AF_INET;
		sa.sin_port = htons(port);
		sa.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

		bind(server, (sockaddr*)&sa, sizeof(sa));
		listen(server, 5);
		int len = sizeof(ca);
		printf("waiting for connect\n");
		client = accept(server, (sockaddr*)&ca, &len);
		if (client == INVALID_SOCKET)
			cout << "failed" << endl;
		return 1;
	}
	int RecieveFile(const char *path)
	{
		char p[64];
		strcpy_s(p,64, path);
		int len = strlen(p);
		if (p[len - 1] != '\\')
		{
			p[len] = '\\';
			len++;
		}
		recv(client, (char*)&fn, sizeof(fn),0);
		strcat(p, fn.Fname);
		unsigned long long siz;
		recv(client, (char*)&siz, sizeof(siz), 0);
		siz = siz / BUFFERSIZE;
		fp = fopen(p, "wb+");
		long long int index = 0;
		int num;
		while (index<=siz)
		{
			num = recv(client, buff, BUFFERSIZE, 0);
			if (num <= 0)
				break;
			fwrite(buff, (int)num, 1, fp);
			index++;

			if(siz != 0)
			cout << (int)index * 100 / siz << num<<endl;
		}
		printf("seccess\n");
		return 0;
	}	
};
int main()
{
	WSADATA WSAData;
	char p[] = "D:\\2\\";
	WSAStartup(MAKEWORD(2, 2), &WSAData);

	FileReciever fr;
	fr.Listen(3000);
	while(1){
		fr.RecieveFile(p);
	}

	system("pause");
}
