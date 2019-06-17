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
	// �������F����TestĿ¼�£������ʽΪ cmd.exe /C + ����
	TCHAR* szCmd = _T("cmd.exe /C dir F:\\Test&& echo S_OK || echo E_FAIL");
	_tcscpy_s(cmdParam.szCommand, szCmd);
	cmdParam.OnCmdEvent = OnCommandEvent;
	cmdParam.iTimeOut = 3000;

	cmdResult = cmdHandler.Initalize();
	if (cmdResult != S_OK)
	{
		printf("cmd handler ��ʼ��ʧ��\n");
		SYSTEM_PAUSE;
		return 0;
	}
	cmdResult = cmdHandler.HandleCommand(&cmdParam);
	if (cmdResult != S_OK)
	{
		printf("cmd handler ִ������ӿڵ���ʧ��\n");
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
	printf("============== �ص� ==============\n");
	std::string echo_data(szResult);
	std::string s_ok("S_OK");
	std::string::size_type pos = echo_data.find(s_ok);
	if (pos != std::string::npos)
		printf("����ִ�гɹ�\n");
	else
		printf("����ִ��ʧ��\n");
	printf("ִ�з��صĽṹ:\n");
	printf("========================================\n");
	printf("%s\n", szResult);
}
