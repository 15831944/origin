#include <Windows.h>
#include <stdio.h>
#include <tchar.h>
#include <string>
#include "CmdHandler.h"

#define SYSTEM_PAUSE system("pause")

void OnCommandEvent(const CHCmdParam* pParam, HRESULT hResultCode, char* szResult);

int main(int argc, TCHAR** argv)
{
	CHCmdParam cmdParam;
	CCmdHandler cmdHandler;
	HRESULT cmdResult = S_OK;

	ZeroMemory(&cmdParam, sizeof(cmdParam));
	cmdParam.iSize = sizeof(CHCmdParam);
	// 这里测试F磁盘Test目录下，命令格式为 cmd.exe /C + 命令
	TCHAR* szCmd = _T("cmd.exe /C dir F:\\Test&& echo S_OK || echo E_FAIL");
	_tcscpy_s(cmdParam.szCommand, szCmd);
	cmdParam.OnCmdEvent = OnCommandEvent;
	cmdParam.iTimeOut = 3000;

	cmdResult = cmdHandler.Initalize();
	if (cmdResult != S_OK)
	{
		printf("cmd handler 初始化失败\n");
		SYSTEM_PAUSE;
		return 0;
	}
	cmdResult = cmdHandler.HandleCommand(&cmdParam);
	if (cmdResult != S_OK)
	{
		printf("cmd handler 执行命令接口调用失败\n");
		cmdHandler.Finish();
		SYSTEM_PAUSE;
		return 0;
	}
	system("pause");
	return 0;
}

void OnCommandEvent(const CHCmdParam* pParam, HRESULT hResultCode, char* szResult)
{
	if (!szResult || !szResult[0]) return;
	if (!pParam || hResultCode != S_OK) return;
	printf("============== 回调 ==============\n");
	std::string echo_data(szResult);
	std::string s_ok("S_OK");
	std::string::size_type pos = echo_data.find(s_ok);
	if (pos != std::string::npos)
		printf("命令执行成功\n");
	else
		printf("命令执行失败\n");
	printf("执行返回的结构:\n");
	printf("========================================\n");
	printf("%s\n", szResult);
}
