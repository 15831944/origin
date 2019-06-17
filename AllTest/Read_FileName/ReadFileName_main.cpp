#include <iostream>
#include <cstring>
#include <windows.h>
using namespace std;

void listFiles(const char * dir);
void ShowFileName(const char *dir);

int main()
{
	using namespace std;
	char dir[100] = "H:\\AllTest\\BreakPoint_test\\";
	//cout << "Enter a directory (ends with \'\\\'): ";
	//cin.getline(dir, 100);
	strcat(dir, "*.*");    // 需要在目录后面加上*.*表示所有文件/目录
	ShowFileName(dir);
	//listFiles(dir);
	system("pause");
	return 0;
}

void listFiles(const char * dir)
{
	
	HANDLE hFind;
	WIN32_FIND_DATA findData;
	LARGE_INTEGER size;
	hFind = FindFirstFile(dir, &findData);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		cout << "Failed to find first file!\n";
		return;
	}
	do
	{
		// 忽略"."和".."两个结果 
		if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0)
			continue;
		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)    // 是否是目录 
		{
			cout << findData.cFileName << "\t<dir>\n";
		}
		else
		{
			size.LowPart = findData.nFileSizeLow;
			size.HighPart = findData.nFileSizeHigh;
			cout << findData.cFileName << "\t" << size.QuadPart << " bytes\n";
		}
	} while (FindNextFile(hFind, &findData));
	cout << "Done!\n";
}

void ShowFileName(const char * dir)
{	

	HANDLE hFind;
	WIN32_FIND_DATA findData;
	LARGE_INTEGER size;
	hFind = FindFirstFile(dir, &findData);
	
	do 
	{
		cout<<findData.cFileName<<endl;

	} while (FindNextFile(hFind, &findData));
}